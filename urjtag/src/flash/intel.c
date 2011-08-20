/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 * Changed by August Hörandl, 2003
 *
 * Documentation:
 * [1] Advanced Micro Devices, "Common Flash Memory Interface Specification Release 2.0",
 *     December 1, 2001
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [3] Intel Corporation, "Common Flash Interface (CFI) and Command Sets
 *     Application Note 646", April 2000, Order Number: 292204-004
 * [4] Advanced Micro Devices, "Common Flash Memory Interface Publication 100 Vendor & Device
 *     ID Code Assignments", December 1, 2001, Volume Number: 96.1
 * [5] Micron Technology, Inc. "Q-Flash Memory MT28F123J3, MT28F640J3, MT28F320J3",
 *     MT28F640J3.fm - Rev. N 3/05 EN
 *
 */

#include <sysdep.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>

#include "flash.h"

#include "cfi.h"
#include "intel.h"
#include "mic.h"

/* autodetect, we can handle this chip */
static int
intel_flash_autodetect32 (urj_flash_cfi_array_t *cfi_array)
{
    urj_bus_area_t area;

    if (URJ_BUS_AREA (cfi_array->bus, cfi_array->address,
                      &area) != URJ_STATUS_OK)
        return 0;

    return ((cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
             == CFI_VENDOR_MITSUBISHI_SCS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_MITSUBISHI_ECS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_INTEL_ECS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_INTEL_SCS))
           && (area.width == 32);
}

static int
intel_flash_autodetect (urj_flash_cfi_array_t *cfi_array)
{
    urj_bus_area_t area;

    if (URJ_BUS_AREA (cfi_array->bus, cfi_array->address,
                      &area) != URJ_STATUS_OK)
        return 0;

    return ((cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
             == CFI_VENDOR_MITSUBISHI_SCS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_MITSUBISHI_ECS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_INTEL_ECS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_INTEL_SCS))
           && (area.width == 16);
}

static int
intel_flash_autodetect8 (urj_flash_cfi_array_t *cfi_array)
{
    urj_bus_area_t area;

    if (URJ_BUS_AREA (cfi_array->bus, cfi_array->address,
                      &area) != URJ_STATUS_OK)
        return 0;

    return ((cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
             == CFI_VENDOR_MITSUBISHI_SCS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_MITSUBISHI_ECS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_INTEL_ECS)
            || (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
                == CFI_VENDOR_INTEL_SCS))
           && (area.width == 8);
}

static void
_intel_flash_print_info (urj_log_level_t ll, urj_flash_cfi_array_t *cfi_array,
                        int o)
{
    uint32_t mid, cid;
    urj_bus_t *bus = cfi_array->bus;

    mid = (URJ_BUS_READ (bus, cfi_array->address + (0x00 << o)) & 0xFF);
    switch (mid)
    {
    case STD_MIC_INTEL:
        urj_log (ll, _("Manufacturer: %s\n"), STD_MICN_INTEL);
        break;
    case STD_MIC_MITSUBISHI:
        urj_log (ll, _("Manufacturer: %s\n"), STD_MICN_MITSUBISHI);
        break;
    case STD_MIC_MICRON_TECHNOLOGY:
        urj_log (ll, _("Manufacturer: %s\n"), STD_MICN_MICRON_TECHNOLOGY);
        break;
    default:
        urj_log (ll, _("Unknown manufacturer (0x%04lX)!\n"),
                 (long unsigned) mid);
        break;
    }

    urj_log (ll, _("Chip: "));
    cid = (URJ_BUS_READ (bus, cfi_array->address + (0x01 << o)) & 0xFFFF);
    switch (cid)
    {
    case 0x0016:
        urj_log (ll, "28F320J3A\n");
        break;
    case 0x0017:
        urj_log (ll, "28F640J3A\n");
        break;
    case 0x0018:
        urj_log (ll, "28F128J3A\n");
        break;
    case 0x001D:
        urj_log (ll, "28F256J3A\n");
        break;
    case 0x8801:
        urj_log (ll, "28F640K3\n");
        break;
    case 0x8802:
        urj_log (ll, "28F128K3\n");
        break;
    case 0x8803:
        urj_log (ll, "28F256K3\n");
        break;
    case 0x8805:
        urj_log (ll, "28F640K18\n");
        break;
    case 0x8806:
        urj_log (ll, "28F128K18\n");
        break;
    case 0x8807:
        urj_log (ll, "28F256K18\n");
        break;
    case 0x880B:
        urj_log (ll, "GE28F640L18T\n");
        break;
    case 0x880C:
        urj_log (ll, "GE28F128L18T\n");
        break;
    case 0x880D:
        urj_log (ll, "GE28F256L18T\n");
        break;
    case 0x880E:
        urj_log (ll, "GE28F640L18B\n");
        break;
    case 0x880F:
        urj_log (ll, "GE28F128L18B\n");
        break;
    case 0x8810:
        urj_log (ll, "GE28F256L18B\n");
        break;
    case 0x891F:
        urj_log (ll, "28F256P33\n");
        break;
    default:
        urj_log (ll, _("Unknown (0x%02lX)!\n"), (long unsigned) cid);
        break;
    }

    /* Read Array */
    URJ_BUS_WRITE (bus, cfi_array->address + (0 << o), 0x00FF00FF);
}

static void
intel_flash_print_info (urj_log_level_t ll, urj_flash_cfi_array_t *cfi_array)
{
    int o = 1;
    urj_bus_t *bus = cfi_array->bus;

    /* Intel Primary Algorithm Extended Query Table - see Table 5. in [3] */
    /* TODO */

    /* Clear Status Register */
    URJ_BUS_WRITE (bus, cfi_array->address + (0 << o), 0x0050);

    /* Read Identifier Command */
    URJ_BUS_WRITE (bus, cfi_array->address + (0 << 0), 0x0090);

    _intel_flash_print_info (ll, cfi_array, o);
}

static void
intel_flash_print_info32 (urj_log_level_t ll, urj_flash_cfi_array_t *cfi_array)
{
    int o = 2;
    urj_bus_t *bus = cfi_array->bus;
    /* Intel Primary Algorithm Extended Query Table - see Table 5. in [3] */
    /* TODO */

    /* Clear Status Register */
    URJ_BUS_WRITE (bus, cfi_array->address + (0 << o), 0x00500050);

    /* Read Identifier Command */
    URJ_BUS_WRITE (bus, cfi_array->address + (0 << 0), 0x00900090);

    _intel_flash_print_info (ll, cfi_array, o);
}

static int
intel_flash_erase_block (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    uint16_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_BLOCK_ERASE);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_CONFIRM);

    while (!((sr = URJ_BUS_READ (bus, cfi_array->address) & 0xFE) & CFI_INTEL_SR_READY));     /* TODO: add timeout */

    switch (sr & ~CFI_INTEL_SR_READY)
    {
    case 0:
        return URJ_STATUS_OK;
    case CFI_INTEL_SR_ERASE_ERROR | CFI_INTEL_SR_PROGRAM_ERROR:
        urj_error_set (URJ_ERROR_FLASH_ERASE, _("invalid command seq"));
        return URJ_STATUS_FAIL;
    case CFI_INTEL_SR_ERASE_ERROR | CFI_INTEL_SR_VPEN_ERROR:
        urj_error_set (URJ_ERROR_FLASH_ERASE, _("low vpen"));
        return URJ_STATUS_FAIL;
    case CFI_INTEL_SR_ERASE_ERROR | CFI_INTEL_SR_BLOCK_LOCKED:
        urj_error_set (URJ_ERROR_FLASH_ERASE, _("block locked"));
        return URJ_STATUS_FAIL;
    default:
        break;
    }

    urj_error_set (URJ_ERROR_FLASH, "unknown error");
    return URJ_STATUS_FAIL;
}

static int
intel_flash_unlock_block (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    uint16_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_LOCK_SETUP);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_UNLOCK_BLOCK);

    while (!((sr = URJ_BUS_READ (bus, cfi_array->address) & 0xFE) & CFI_INTEL_SR_READY));     /* TODO: add timeout */

    if (sr != CFI_INTEL_SR_READY)
    {
        urj_error_set (URJ_ERROR_FLASH_UNLOCK,
                       _("unknown error while unlocking block"));
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_lock_block (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    uint16_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_LOCK_SETUP);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_LOCK_BLOCK);

    while (!((sr = URJ_BUS_READ (bus, cfi_array->address) & 0xFE) & CFI_INTEL_SR_READY));     /* TODO: add timeout */

    if (sr != CFI_INTEL_SR_READY)
    {
        urj_error_set (URJ_ERROR_FLASH_LOCK,
                       _("unknown error while locking block"));
        return URJ_STATUS_FAIL;
    }

    URJ_BUS_WRITE (bus, adr + 0x02, CFI_INTEL_CMD_READ_IDENTIFIER);

    sr = URJ_BUS_READ (bus, cfi_array->address & 0x01);
    if (!sr)
    {
        urj_error_set (URJ_ERROR_FLASH_LOCK,
                       _("locking block failed"));
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_program_single (urj_flash_cfi_array_t *cfi_array,
                            uint32_t adr, uint32_t data)
{
    uint16_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_PROGRAM1);
    URJ_BUS_WRITE (bus, adr, data);

    while (!((sr = URJ_BUS_READ (bus, cfi_array->address) & 0xFE) & CFI_INTEL_SR_READY));     /* TODO: add timeout */

    if (sr != CFI_INTEL_SR_READY)
    {
        urj_error_set (URJ_ERROR_FLASH_PROGRAM,
                       _("unknown error while programming"));
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_program_buffer (urj_flash_cfi_array_t *cfi_array,
                            uint32_t adr, uint32_t *buffer, int count)
{
    /* NOTE: Write-to-buffer programming operation according to [5], Figure 9 */
    uint16_t sr;
    urj_bus_t *bus = cfi_array->bus;
    urj_flash_cfi_chip_t *cfi_chip = cfi_array->cfi_chips[0];
    int wb_bytes = cfi_chip->cfi.device_geometry.max_bytes_write;
    int chip_width = cfi_chip->width;
    int offset = 0;

    while (count > 0)
    {
        int wcount, idx;
        uint32_t block_adr = adr;

        /* determine length of next multi-byte write */
        wcount = wb_bytes - (adr % wb_bytes);
        wcount /= chip_width;
        if (wcount > count)
            wcount = count;

        /* issue command WRITE_TO_BUFFER */
        URJ_BUS_WRITE (bus, cfi_array->address,
                       CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
        /* poll XSR7 == 1 */
        do {
            URJ_BUS_WRITE (bus, adr, CFI_INTEL_CMD_WRITE_TO_BUFFER);
        } while (!((sr = URJ_BUS_READ (bus, cfi_array->address) & 0xFE) & CFI_INTEL_SR_READY)); /* TODO: add timeout */

        /* write count value (number of upcoming writes - 1) */
        URJ_BUS_WRITE (bus, adr, wcount - 1);

        /* write payload to buffer */
        for (idx = 0; idx < wcount; idx++)
        {
            URJ_BUS_WRITE (bus, adr, buffer[offset + idx]);
            adr += cfi_array->bus_width;
        }
        offset += wcount;

        /* issue command WRITE_CONFIRM */
        URJ_BUS_WRITE (bus, block_adr, CFI_INTEL_CMD_WRITE_CONFIRM);

        count -= wcount;
    }

    /* poll SR7 == 1 */
    while (!((sr = URJ_BUS_READ (bus, cfi_array->address) & 0xFE) & CFI_INTEL_SR_READY));     /* TODO: add timeout */
    if (sr != CFI_INTEL_SR_READY)
    {
        urj_error_set (URJ_ERROR_FLASH_PROGRAM,
                       _("unknown error while programming"));
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_program (urj_flash_cfi_array_t *cfi_array,
                     uint32_t adr, uint32_t *buffer, int count)
{
    urj_flash_cfi_query_structure_t *cfi = &(cfi_array->cfi_chips[0]->cfi);
    int max_bytes_write = cfi->device_geometry.max_bytes_write;

#ifndef FLASH_MULTI_BYTE
    max_bytes_write = 1;
#endif

    /* multi-byte writes supported? */
    if (max_bytes_write > 1)
        return intel_flash_program_buffer (cfi_array, adr, buffer, count);

    else
    {
        /* unroll buffer to single writes */
        int idx;

        for (idx = 0; idx < count; idx++)
        {
            int status = intel_flash_program_single (cfi_array, adr,
                                                     buffer[idx]);
            if (status != URJ_STATUS_OK)
                return status;
            adr += cfi_array->bus_width;
        }
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_erase_block32 (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    uint32_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER << 16) |
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr,
                   (CFI_INTEL_CMD_BLOCK_ERASE << 16) |
                   CFI_INTEL_CMD_BLOCK_ERASE);
    URJ_BUS_WRITE (bus, adr,
                   (CFI_INTEL_CMD_CONFIRM << 16) | CFI_INTEL_CMD_CONFIRM);

    while (((sr = URJ_BUS_READ (bus, cfi_array->address) & 0x00FE00FE) & ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY));    /* TODO: add timeout */

    if (sr != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY))
    {
        urj_error_set (URJ_ERROR_FLASH_ERASE, "sr = 0x%08lX",
                       (long unsigned) sr);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_unlock_block32 (urj_flash_cfi_array_t *cfi_array,
                            uint32_t adr)
{
    uint32_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER << 16) |
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr,
                   (CFI_INTEL_CMD_LOCK_SETUP << 16) |
                   CFI_INTEL_CMD_LOCK_SETUP);
    URJ_BUS_WRITE (bus, adr,
                   (CFI_INTEL_CMD_UNLOCK_BLOCK << 16) |
                   CFI_INTEL_CMD_UNLOCK_BLOCK);

    while (((sr = URJ_BUS_READ (bus, cfi_array->address) & 0x00FE00FE) & ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY));    /* TODO: add timeout */

    if (sr != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY))
    {
        urj_error_set (URJ_ERROR_FLASH_UNLOCK, "sr = 0x%08lX",
                       (long unsigned) sr);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_lock_block32 (urj_flash_cfi_array_t *cfi_array,
                          uint32_t adr)
{
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_lock_block32 0x%08lX IGNORE\n",
             (long unsigned) adr);
    return URJ_STATUS_OK;
}

static int
intel_flash_program32_single (urj_flash_cfi_array_t *cfi_array,
                              uint32_t adr, uint32_t data)
{
    uint32_t sr;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address,
                   (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER << 16) |
                   CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
    URJ_BUS_WRITE (bus, adr,
                   (CFI_INTEL_CMD_PROGRAM1 << 16) | CFI_INTEL_CMD_PROGRAM1);
    URJ_BUS_WRITE (bus, adr, data);

    while (((sr = URJ_BUS_READ (bus, cfi_array->address) & 0x00FE00FE) & ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY));    /* TODO: add timeout */

    if (sr != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY))
    {
        urj_error_set (URJ_ERROR_FLASH_PROGRAM, "sr = 0x%08lX",
                       (long unsigned) sr);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static int
intel_flash_program32 (urj_flash_cfi_array_t *cfi_array,
                       uint32_t adr, uint32_t *buffer, int count)
{
    /* Single byte programming is forced for 32 bit (2x16) flash configuration.
       a) lack of testing capbilities for 2x16 multi-byte write operation
       b) no implementation of intel_flash_program32_buffer() available
       Closing these issues will enable multi-byte for 2x16 as well. */
    int idx;

    /* unroll buffer to single writes */
    for (idx = 0; idx < count; idx++)
    {
        int status = intel_flash_program32_single (cfi_array, adr, buffer[idx]);
        if (status != URJ_STATUS_OK)
            return status;
        adr += cfi_array->bus_width;
    }

    return URJ_STATUS_OK;
}

static void
intel_flash_readarray32 (urj_flash_cfi_array_t *cfi_array)
{
    /* Read Array */
    URJ_BUS_WRITE (cfi_array->bus, cfi_array->address, 0x00FF00FF);
}

static void
intel_flash_readarray (urj_flash_cfi_array_t *cfi_array)
{
    /* Read Array */
    URJ_BUS_WRITE (cfi_array->bus, cfi_array->address, 0x00FF00FF);
}

const urj_flash_driver_t urj_flash_intel_32_flash_driver = {
    N_("Intel Standard Command Set"),
    N_("supported: 28Fxxxx, 2 x 16 bit"),
    4,                          /* buswidth */
    intel_flash_autodetect32,
    intel_flash_print_info32,
    intel_flash_erase_block32,
    intel_flash_lock_block32,
    intel_flash_unlock_block32,
    intel_flash_program32,
    intel_flash_readarray32,
};

const urj_flash_driver_t urj_flash_intel_16_flash_driver = {
    N_("Intel Standard Command Set"),
    N_("supported: 28Fxxxx, 1 x 16 bit"),
    2,                          /* buswidth */
    intel_flash_autodetect,
    intel_flash_print_info,
    intel_flash_erase_block,
    intel_flash_lock_block,
    intel_flash_unlock_block,
    intel_flash_program,
    intel_flash_readarray,
};

const urj_flash_driver_t urj_flash_intel_8_flash_driver = {
    N_("Intel Standard Command Set"),
    N_("supported: 28Fxxxx, 1 x 8 bit"),
    1,                          /* buswidth */
    intel_flash_autodetect8,
    intel_flash_print_info,
    intel_flash_erase_block,
    intel_flash_lock_block,
    intel_flash_unlock_block,
    intel_flash_program,
    intel_flash_readarray,
};
