/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] JEDEC Solid State Technology Association, "Common Flash Interface (CFI)",
 *     September 1999, Order Number: JESD68
 * [2] Intel Corporation, "Common Flash Interface (CFI) and Command Sets
 *     Application Note 646", April 2000, Order Number: 292204-004
 *
 */

#include <sysdep.h>

#include <stdint.h>
#include <stdlib.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>

#include "flash.h"

#include "cfi.h"

void
urj_flash_cfi_array_free (urj_flash_cfi_array_t *cfi_array)
{
    if (!cfi_array)
        return;

    if (cfi_array->cfi_chips)
    {
        int i;

        for (i = 0; i < cfi_array->bus_width; i++)
        {
            if (!cfi_array->cfi_chips[i])
                continue;

            free (cfi_array->cfi_chips[i]->cfi.device_geometry.
                  erase_block_regions);
            if (cfi_array->cfi_chips[i]->cfi.identification_string.
                pri_vendor_tbl)
                free (cfi_array->cfi_chips[i]->cfi.
                      identification_string.pri_vendor_tbl);
            free (cfi_array->cfi_chips[i]);
        }
        free (cfi_array->cfi_chips);
    }

    free (cfi_array);
}

int
urj_flash_cfi_detect (urj_bus_t *bus, uint32_t adr,
                      urj_flash_cfi_array_t **cfi_array)
{
    unsigned int bw;            /* bus width */
    unsigned int d;             /* data offset */
    size_t ba;                  /* bus width address multiplier */
    int ma;                     /* flash mode address multiplier */
    urj_bus_area_t area;

    if (!cfi_array || !bus)
    {
        urj_error_set (URJ_ERROR_INVALID, "cfi_array or bus");
        return URJ_STATUS_FAIL;
    }

    *cfi_array = calloc (1, sizeof (urj_flash_cfi_array_t));
    if (!*cfi_array)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof (urj_flash_cfi_array_t));
        return URJ_STATUS_FAIL;
    }

    (*cfi_array)->bus = bus;
    (*cfi_array)->address = adr;
    if (URJ_BUS_AREA (bus, adr, &area) != URJ_STATUS_OK)
        // retain error state
        return URJ_STATUS_FAIL;
    if (URJ_BUS_TYPE (bus) != URJ_BUS_TYPE_PARALLEL)
        return URJ_STATUS_FAIL;
    bw = area.width;
    if (bw != 8 && bw != 16 && bw != 32)
    {
        urj_error_set (URJ_ERROR_INVALID, "bus width = %d", bw);
        return URJ_STATUS_FAIL;
    }
    (*cfi_array)->bus_width = ba = bw / 8;
    (*cfi_array)->cfi_chips = calloc (ba, sizeof (urj_flash_cfi_chip_t *));
    if (!(*cfi_array)->cfi_chips)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       ba, sizeof (urj_flash_cfi_chip_t *));
        return URJ_STATUS_FAIL;
    }

    for (d = 0; d < bw; d += 8)
    {
#define A(off)                  (adr + (off) * ba * ma)
#define D(data)                 ((data) << d)
#define gD(data)                (((data) >> d) & 0xFF)
#define read1(off)              gD(URJ_BUS_READ( bus, A(off) ))
        // @@@@ RFHH check status of URJ_BUS_READ_START
#define read2(off)              (URJ_BUS_READ_START (bus, A(off)), gD (URJ_BUS_READ_NEXT (bus, A((off) + 1))) | gD (URJ_BUS_READ_END (bus)) << 8)
#define write1(off,data)        URJ_BUS_WRITE( bus, A(off), D(data) )

        urj_flash_cfi_query_structure_t *cfi;
        uint32_t tmp;
        int ret = -4;           /* CFI not detected (Q) */
        uint16_t pri_vendor_tbl_adr;

        /* detect CFI capable devices - see Table 1 in [1] */
        for (ma = 1; ma <= 4; ma *= 2)
        {
            write1 (CFI_CMD_QUERY_OFFSET, CFI_CMD_QUERY);

            if (read1 (CFI_QUERY_ID_OFFSET) == 'Q')
            {
                ret = -5;       /* CFI not detected (R) */
                if (read1 (CFI_QUERY_ID_OFFSET + 1) == 'R')
                    break;
            }

            write1 (0, CFI_CMD_READ_ARRAY1);
            write1 (0, CFI_CMD_READ_ARRAY2);
        }

        if (ma > 4)
        {
            if (ret == -4)
                urj_error_set (URJ_ERROR_FLASH, "CFI not detected (Q)");
            else
                urj_error_set (URJ_ERROR_FLASH, "CFI not detected (R)");
            return URJ_STATUS_FAIL;
        }

        if (read1 (CFI_QUERY_ID_OFFSET + 2) != 'Y')
        {
            write1 (0, CFI_CMD_READ_ARRAY1);
            write1 (0, CFI_CMD_READ_ARRAY2);
            urj_error_set (URJ_ERROR_FLASH, "CFI not detected (Y)");
            return URJ_STATUS_FAIL;
        }

        (*cfi_array)->cfi_chips[d / 8] = calloc (1,
                                                 sizeof (urj_flash_cfi_chip_t));
        if (!(*cfi_array)->cfi_chips[d / 8])
        {
            write1 (0, CFI_CMD_READ_ARRAY1);
            write1 (0, CFI_CMD_READ_ARRAY2);
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                           (size_t) 1, sizeof (urj_flash_cfi_chip_t));
            return URJ_STATUS_FAIL;
        }
        cfi = &(*cfi_array)->cfi_chips[d / 8]->cfi;

        /* Identification string - see Table 6 in [1] */
        cfi->identification_string.pri_id_code = read2 (PRI_VENDOR_ID_OFFSET);
        cfi->identification_string.pri_vendor_tbl = NULL;
        cfi->identification_string.alt_id_code = read2 (ALT_VENDOR_ID_OFFSET);
        cfi->identification_string.alt_vendor_tbl = NULL;

        /* System interface information - see Table 7 in [1] */
        tmp = read1 (VCC_MIN_WEV_OFFSET);
        cfi->system_interface_info.vcc_min_wev =
            ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
        tmp = read1 (VCC_MAX_WEV_OFFSET);
        cfi->system_interface_info.vcc_max_wev =
            ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
        tmp = read1 (VPP_MIN_WEV_OFFSET);
        cfi->system_interface_info.vpp_min_wev =
            ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
        tmp = read1 (VPP_MAX_WEV_OFFSET);
        cfi->system_interface_info.vpp_max_wev =
            ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;

        /* TODO: Add out of range checks for timeouts */
        tmp = read1 (TYP_SINGLE_WRITE_TIMEOUT_OFFSET);
        cfi->system_interface_info.typ_single_write_timeout =
            tmp ? (1 << tmp) : 0;

        tmp = read1 (TYP_BUFFER_WRITE_TIMEOUT_OFFSET);
        cfi->system_interface_info.typ_buffer_write_timeout =
            tmp ? (1 << tmp) : 0;

        tmp = read1 (TYP_BLOCK_ERASE_TIMEOUT_OFFSET);
        cfi->system_interface_info.typ_block_erase_timeout =
            tmp ? (1 << tmp) : 0;

        tmp = read1 (TYP_CHIP_ERASE_TIMEOUT_OFFSET);
        cfi->system_interface_info.typ_chip_erase_timeout =
            tmp ? (1 << tmp) : 0;

        tmp = read1 (MAX_SINGLE_WRITE_TIMEOUT_OFFSET);
        cfi->system_interface_info.max_single_write_timeout =
            (tmp ? (1 << tmp) : 0) *
            cfi->system_interface_info.typ_single_write_timeout;

        tmp = read1 (MAX_BUFFER_WRITE_TIMEOUT_OFFSET);
        cfi->system_interface_info.max_buffer_write_timeout =
            (tmp ? (1 << tmp) : 0) *
            cfi->system_interface_info.typ_buffer_write_timeout;

        tmp = read1 (MAX_BLOCK_ERASE_TIMEOUT_OFFSET);
        cfi->system_interface_info.max_block_erase_timeout =
            (tmp ? (1 << tmp) : 0) *
            cfi->system_interface_info.typ_block_erase_timeout;

        tmp = read1 (MAX_CHIP_ERASE_TIMEOUT_OFFSET);
        cfi->system_interface_info.max_chip_erase_timeout =
            (tmp ? (1 << tmp) : 0) *
            cfi->system_interface_info.typ_chip_erase_timeout;

        /* Device geometry - see Table 8 in [1] */
        /* TODO: Add out of range check */
        cfi->device_geometry.device_size = 1 << read1 (DEVICE_SIZE_OFFSET);

        cfi->device_geometry.device_interface =
            read2 (FLASH_DEVICE_INTERFACE_OFFSET);

        /* TODO: Add out of range check */
        cfi->device_geometry.max_bytes_write =
            1 << read2 (MAX_BYTES_WRITE_OFFSET);

        tmp = cfi->device_geometry.number_of_erase_regions =
            read1 (NUMBER_OF_ERASE_REGIONS_OFFSET);

        cfi->device_geometry.erase_block_regions =
            malloc (tmp * sizeof (urj_flash_cfi_erase_block_region_t));
        if (!cfi->device_geometry.erase_block_regions)
        {
            write1 (0, CFI_CMD_READ_ARRAY1);
            write1 (0, CFI_CMD_READ_ARRAY2);
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                           (size_t) tmp
                           * sizeof (urj_flash_cfi_erase_block_region_t));
            return URJ_STATUS_FAIL;
        }

        {
            int a;
            int i;

            for (i = 0, a = ERASE_BLOCK_REGION_OFFSET; i < tmp; i++, a += 4)
            {
                uint32_t y = read2 (a);
                uint32_t z = read2 (a + 2) << 8;
                if (z == 0)
                    z = 128;
                cfi->device_geometry.erase_block_regions[i].erase_block_size =
                    z;
                cfi->device_geometry.erase_block_regions[i].
                    number_of_erase_blocks = y + 1;
            }
        }

        pri_vendor_tbl_adr = read2 (PRI_VENDOR_TABLE_ADR_OFFSET);

        /* AMD CFI Primary Vendor-Specific Extended Query Table - see [3] and [4] */
        if (cfi->identification_string.pri_id_code == CFI_VENDOR_AMD_SCS
            && pri_vendor_tbl_adr != 0)
        {
            urj_flash_cfi_amd_pri_extened_query_structure_t *pri_vendor_tbl;
            uint8_t major_version;
            uint8_t minor_version;
            uint8_t num_of_banks;
            int i;
#undef A
#define A(off)                  (adr + (pri_vendor_tbl_adr + (off)) * ba * ma)

            if (read1 (0) != 'P' || read1 (1) != 'R' || read1 (2) != 'I')
            {
                write1 (0, CFI_CMD_READ_ARRAY1);
                write1 (0, CFI_CMD_READ_ARRAY2);
                urj_error_set (URJ_ERROR_FLASH,
                               "CFI primary vendor table not detected");
                return URJ_STATUS_FAIL;
            }

            major_version = read1 (MAJOR_VERSION_OFFSET);
            minor_version = read1 (MINOR_VERSION_OFFSET);
            if (major_version > '1'
                || (major_version == '1' && minor_version >= '3'))
                num_of_banks = read1 (BANK_ORGANIZATION_OFFSET);
            else
                num_of_banks = 0;
            pri_vendor_tbl = calloc (1,
                    sizeof (urj_flash_cfi_amd_pri_extened_query_structure_t)
                            + num_of_banks * sizeof (uint8_t));
            if (!pri_vendor_tbl)
            {
                write1 (0, CFI_CMD_READ_ARRAY1);
                write1 (0, CFI_CMD_READ_ARRAY2);
                urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                               (size_t) 1,
                               sizeof (urj_flash_cfi_amd_pri_extened_query_structure_t)
                                   + num_of_banks * sizeof (uint8_t));
                return URJ_STATUS_FAIL;
            }

            if (major_version > '1'
                || (major_version == '1' && minor_version >= '0'))
            {
                pri_vendor_tbl->major_version = major_version;
                pri_vendor_tbl->minor_version = minor_version;
                pri_vendor_tbl->address_sensitive_unlock =
                    read1 (ADDRESS_SENSITIVE_UNLOCK_OFFSET);
                pri_vendor_tbl->erase_suspend = read1 (ERASE_SUSPEND_OFFSET);
                pri_vendor_tbl->sector_protect = read1 (SECTOR_PROTECT_OFFSET);
                pri_vendor_tbl->sector_temporary_unprotect =
                    read1 (SECTOR_TEMPORARY_UNPROTECT_OFFSET);
                pri_vendor_tbl->sector_protect_scheme =
                    read1 (SECTOR_PROTECT_SCHEME_OFFSET);
                pri_vendor_tbl->simultaneous_operation =
                    read1 (SIMULTANEOUS_OPERATION_OFFSET);
                pri_vendor_tbl->burst_mode_type =
                    read1 (BURST_MODE_TYPE_OFFSET);
                pri_vendor_tbl->page_mode_type =
                    read1 (PAGE_MODE_TYPE_OFFSET);
            }
            if (major_version > '1'
                || (major_version == '1' && minor_version >= '1'))
            {
                tmp = read1 (ACC_MIN_OFFSET);
                pri_vendor_tbl->acc_min =
                    ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
                tmp = read1 (ACC_MAX_OFFSET);
                pri_vendor_tbl->acc_max =
                    ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
                pri_vendor_tbl->top_bottom_sector_flag =
                    read1 (TOP_BOTTOM_SECTOR_FLAG_OFFSET);
            }
            if (major_version > '1'
                || (major_version == '1' && minor_version >= '2'))
                pri_vendor_tbl->program_suspend =
                    read1 (PROGRAM_SUSPEND_OFFSET);
            if (major_version > '1'
                || (major_version == '1' && minor_version >= '3'))
            {
                if (pri_vendor_tbl->simultaneous_operation)
                    pri_vendor_tbl->bank_organization =
                        read1 (BANK_ORGANIZATION_OFFSET);
                else
                    pri_vendor_tbl->bank_organization = 0;
                for (i = 0; i < pri_vendor_tbl->bank_organization; i++)
                    pri_vendor_tbl->bank_region_info[i] =
                        read1 (BANK_REGION_INFO_OFFSET +
                               i * sizeof (uint8_t));
            }
            if (major_version > '1'
                || (major_version == '1' && minor_version >= '4'))
            {
                pri_vendor_tbl->unlock_bypass = read1 (UNLOCK_BYPASS_OFFSET);
                tmp = read1 (SECSI_SECTOR_SIZE_OFFSET);
                pri_vendor_tbl->secsi_sector_size = tmp ? (1 << tmp) : 0;
                tmp = read1 (EMBEDDED_HWRST_TIMEOUT_MAX_OFFSET);
                pri_vendor_tbl->embedded_hwrst_timeout_max =
                    tmp ? (1 << tmp) : 0;
                tmp = read1 (NON_EMBEDDED_HWRST_TIMEOUT_MAX_OFFSET);
                pri_vendor_tbl->non_embedded_hwrst_timeout_max =
                    tmp ? (1 << tmp) : 0;
                tmp = read1 (ERASE_SUSPEND_TIMEOUT_MAX_OFFSET);
                pri_vendor_tbl->erase_suspend_timeout_max =
                    tmp ? (1 << tmp) : 0;
                tmp = read1 (PROGRAM_SUSPEND_TIMEOUT_MAX_OFFSET);
                pri_vendor_tbl->program_suspend_timeout_max =
                    tmp ? (1 << tmp) : 0;
            }

            cfi->identification_string.pri_vendor_tbl = (void *) pri_vendor_tbl;

#undef A
#define A(off)                  (adr + (off) * ba * ma)

            /* Reverse the order of erase block region information for top boot devices.  */
            if ((major_version > '1'
                 || (major_version == '1' && minor_version >= '1'))
                && pri_vendor_tbl->top_bottom_sector_flag == 0x3)
            {
                uint32_t y, z;
                uint32_t n = cfi->device_geometry.number_of_erase_regions;

                for (i = 0; i < n / 2; i++)
                {
                    z = cfi->device_geometry.erase_block_regions[i].
                        erase_block_size;
                    y = cfi->device_geometry.erase_block_regions[i].
                        number_of_erase_blocks;
                    cfi->device_geometry.erase_block_regions[i].
                        erase_block_size =
                        cfi->device_geometry.erase_block_regions[n - i - 1].
                        erase_block_size;
                    cfi->device_geometry.erase_block_regions[i].
                        number_of_erase_blocks =
                        cfi->device_geometry.erase_block_regions[n - i - 1].
                        number_of_erase_blocks;
                    cfi->device_geometry.erase_block_regions[n - i - 1].
                        erase_block_size = z;
                    cfi->device_geometry.erase_block_regions[n - i - 1].
                        number_of_erase_blocks = y;
                }
            }
        }

        /* TODO: Intel Primary Algorithm Extended Query Table - see Table 5. in [2] */

        /* Read Array */
        write1 (0, CFI_CMD_READ_ARRAY1);
        write1 (0, CFI_CMD_READ_ARRAY2);

#undef A
#undef D
#undef gD
#undef read1
#undef read2
#undef write1

        switch (cfi->device_geometry.device_interface)
        {
        case CFI_INTERFACE_X8:
            if (ma != 1)
            {
                urj_error_set (URJ_ERROR_FLASH_DETECT, "device detection");
                return URJ_STATUS_FAIL;
            }
            (*cfi_array)->cfi_chips[d / 8]->width = 1;
            break;
        case CFI_INTERFACE_X16:
            if (ma != 1)
            {
                urj_error_set (URJ_ERROR_FLASH_DETECT, "device detection");
                return URJ_STATUS_FAIL;
            }
            (*cfi_array)->cfi_chips[d / 8]->width = 2;
            d += 8;
            break;
        case CFI_INTERFACE_X8_X16:
            if (ma != 1 && ma != 2)
            {
                urj_error_set (URJ_ERROR_FLASH_DETECT, "device detection");
                return URJ_STATUS_FAIL;
            }
            (*cfi_array)->cfi_chips[d / 8]->width = 2 / ma;
            if (ma == 1)
                d += 8;
            break;
        case CFI_INTERFACE_X32:
            if (ma != 1)
            {
                urj_error_set (URJ_ERROR_FLASH_DETECT, "device detection");
                return URJ_STATUS_FAIL;
            }
            (*cfi_array)->cfi_chips[d / 8]->width = 4;
            d += 24;
            break;
        case CFI_INTERFACE_X16_X32:
            if (ma != 1 && ma != 2)
            {
                urj_error_set (URJ_ERROR_FLASH_DETECT, "device detection");
                return URJ_STATUS_FAIL;
            }
            (*cfi_array)->cfi_chips[d / 8]->width = 4 / ma;
            if (ma == 1)
                d += 24;
            else
                d += 8;
            break;
        default:
            urj_error_set (URJ_ERROR_FLASH_DETECT, "device detection");
            return URJ_STATUS_FAIL;
        }
    }

    return URJ_STATUS_OK;
}
