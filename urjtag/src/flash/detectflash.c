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
 *
 * Documentation:
 * [1] JEDEC Solid State Technology Association, "Common Flash Interface (CFI)",
 *     September 1999, Order Number: JESD68
 * [2] JEDEC Solid State Technology Association, "Common Flash Interface (CFI) ID Codes",
 *     September 2001, Order Number: JEP137-A
 *
 */

#include "sysdep.h"

#include <stdint.h>
#include <string.h>
#include <flash/cfi.h>
#include <flash/intel.h>
#include <flash/mic.h>

#include <flash.h>
#include <bus.h>

cfi_array_t *cfi_array = NULL;

extern int jedec_exp_detect (bus_t *bus, uint32_t adr,
                             cfi_array_t **cfi_array);
extern int jedec_detect (bus_t *bus, uint32_t adr, cfi_array_t **cfi_array);

extern int amd_detect (bus_t *bus, uint32_t adr, cfi_array_t **cfi_array);      //Ajith

void
detectflash (bus_t *bus, uint32_t adr)
{
    cfi_query_structure_t *cfi;
    const char *s;

    if (!bus)
    {
        printf (_("Error: Missing bus driver!\n"));
        return;
    }

    cfi_array_free (cfi_array);
    cfi_array = NULL;

    bus_prepare (bus);

    if (cfi_detect (bus, adr, &cfi_array))
    {
        cfi_array_free (cfi_array);
        cfi_array = NULL;
        if (jedec_detect (bus, adr, &cfi_array) != 0)
        {
            cfi_array_free (cfi_array);
            cfi_array = NULL;
            if (amd_detect (bus, adr, &cfi_array) != 0)
            {
                cfi_array_free (cfi_array);
                cfi_array = NULL;
#ifdef JEDEC_EXP
                if (jedec_exp_detect (bus, adr, &cfi_array))
                {
                    cfi_array_free (cfi_array);
                    cfi_array = NULL;
                }
#endif
            }
        }
    }

    if (cfi_array == NULL)
    {
        printf (_("Flash not found!\n"));
        return;
    }

    cfi = &cfi_array->cfi_chips[0]->cfi;

    /* detect CFI capable devices */
    /* TODO: Low chip only */
    /* see 4.3.2 in [1] */
    printf (_("Query identification string:\n"));
    /* see section 2 in [2] */
    switch (cfi->identification_string.pri_id_code)
    {
    case CFI_VENDOR_NULL:
        s = N_("null");
        break;
    case CFI_VENDOR_INTEL_ECS:
        s = N_("Intel/Sharp Extended Command Set");
        break;
    case CFI_VENDOR_AMD_SCS:
        s = N_("AMD/Fujitsu Standard Command Set");
        break;
    case CFI_VENDOR_INTEL_SCS:
        s = N_("Intel Standard Command Set");
        break;
    case CFI_VENDOR_AMD_ECS:
        s = N_("AMD/Fujitsu Extended Command Set");
        break;
    case CFI_VENDOR_MITSUBISHI_SCS:
        s = N_("Mitsubishi Standard Command Set");
        break;
    case CFI_VENDOR_MITSUBISHI_ECS:
        s = N_("Mitsubishi Extended Command Set");
        break;
    case CFI_VENDOR_SST_PWCS:
        s = N_("Page Write Command Set");
        break;
    default:
        s = N_("unknown!!!");
        break;
    }
    printf (_
            ("\tPrimary Algorithm Command Set and Control Interface ID Code: 0x%04X (%s)\n"),
            cfi->identification_string.pri_id_code, _(s));
    switch (cfi->identification_string.alt_id_code)
    {
    case CFI_VENDOR_NULL:
        s = N_("null");
        break;
    case CFI_VENDOR_INTEL_ECS:
        s = N_("Intel/Sharp Extended Command Set");
        break;
    case CFI_VENDOR_AMD_SCS:
        s = N_("AMD/Fujitsu Standard Command Set");
        break;
    case CFI_VENDOR_INTEL_SCS:
        s = N_("Intel Standard Command Set");
        break;
    case CFI_VENDOR_AMD_ECS:
        s = N_("AMD/Fujitsu Extended Command Set");
        break;
    case CFI_VENDOR_MITSUBISHI_SCS:
        s = N_("Mitsubishi Standard Command Set");
        break;
    case CFI_VENDOR_MITSUBISHI_ECS:
        s = N_("Mitsubishi Extended Command Set");
        break;
    case CFI_VENDOR_SST_PWCS:
        s = N_("Page Write Command Set");
        break;
    default:
        s = N_("unknown!!!");
        break;
    }
    printf (_
            ("\tAlternate Algorithm Command Set and Control Interface ID Code: 0x%04X (%s)\n"),
            cfi->identification_string.alt_id_code, _(s));

    /* see 4.3.3 in [1] */
    printf (_("Query system interface information:\n"));
    printf (_
            ("\tVcc Logic Supply Minimum Write/Erase or Write voltage: %d mV\n"),
            cfi->system_interface_info.vcc_min_wev);
    printf (_
            ("\tVcc Logic Supply Maximum Write/Erase or Write voltage: %d mV\n"),
            cfi->system_interface_info.vcc_max_wev);
    printf (_
            ("\tVpp [Programming] Supply Minimum Write/Erase voltage: %d mV\n"),
            cfi->system_interface_info.vpp_min_wev);
    printf (_
            ("\tVpp [Programming] Supply Maximum Write/Erase voltage: %d mV\n"),
            cfi->system_interface_info.vpp_max_wev);
    printf (_("\tTypical timeout per single byte/word program: %d us\n"),
            cfi->system_interface_info.typ_single_write_timeout);
    printf (_
            ("\tTypical timeout for maximum-size multi-byte program: %d us\n"),
            cfi->system_interface_info.typ_buffer_write_timeout);
    printf (_("\tTypical timeout per individual block erase: %d ms\n"),
            cfi->system_interface_info.typ_block_erase_timeout);
    printf (_("\tTypical timeout for full chip erase: %d ms\n"),
            cfi->system_interface_info.typ_chip_erase_timeout);
    printf (_("\tMaximum timeout for byte/word program: %d us\n"),
            cfi->system_interface_info.max_single_write_timeout);
    printf (_("\tMaximum timeout for multi-byte program: %d us\n"),
            cfi->system_interface_info.max_buffer_write_timeout);
    printf (_("\tMaximum timeout per individual block erase: %d ms\n"),
            cfi->system_interface_info.max_block_erase_timeout);
    printf (_("\tMaximum timeout for chip erase: %d ms\n"),
            cfi->system_interface_info.max_chip_erase_timeout);

    /* see 4.3.4 in [1] */
    printf (_("Device geometry definition:\n"));
    printf (_("\tDevice Size: %d B (%d KiB, %d MiB)\n"),
            cfi->device_geometry.device_size,
            cfi->device_geometry.device_size / 1024,
            cfi->device_geometry.device_size / (1024 * 1024));
    /* see section 4 in [2] */
    switch (cfi->device_geometry.device_interface)
    {
    case CFI_INTERFACE_X8:
        s = N_("x8");
        break;
    case CFI_INTERFACE_X16:
        s = N_("x16");
        break;
    case CFI_INTERFACE_X8_X16:
        s = N_("x8/x16");
        break;
    case CFI_INTERFACE_X32:
        s = N_("x32");
        break;
    case CFI_INTERFACE_X16_X32:
        s = N_("x16/x32");
        break;
    default:
        s = N_("unknown!!!");
        break;
    }
    printf (_("\tFlash Device Interface Code description: 0x%04X (%s)\n"),
            cfi->device_geometry.device_interface, _(s));
    printf (_("\tMaximum number of bytes in multi-byte program: %d\n"),
            cfi->device_geometry.max_bytes_write);
    printf (_("\tNumber of Erase Block Regions within device: %d\n"),
            cfi->device_geometry.number_of_erase_regions);
    printf (_("\tErase Block Region Information:\n"));
    {
        int i;

        for (i = 0; i < cfi->device_geometry.number_of_erase_regions; i++)
        {
            printf (_("\t\tRegion %d:\n"), i);
            printf (_("\t\t\tErase Block Size: %d B (%d KiB)\n"),
                    cfi->device_geometry.erase_block_regions[i].
                    erase_block_size,
                    cfi->device_geometry.erase_block_regions[i].
                    erase_block_size / 1024);
            printf (_("\t\t\tNumber of Erase Blocks: %d\n"),
                    cfi->device_geometry.erase_block_regions[i].
                    number_of_erase_blocks);
        }
    }

    if (cfi->identification_string.pri_id_code == CFI_VENDOR_AMD_SCS
        && cfi->identification_string.pri_vendor_tbl != NULL)
    {
        amd_pri_extened_query_structure_t *pri_vendor_tbl;
        uint8_t major_version;
        uint8_t minor_version;
        int i;
        const char *required_or_not[2] = {
            N_("Required"), N_("Not required")
        };
        const char *supported_or_not[2] = {
            N_("Supported"), N_("Not supported")
        };
        const char *process_technology[6] = {
            N_("170-nm Floating Gate technology"),
            N_("230-nm MirrorBit(tm) technology"),
            N_("130-nm Floating Gate technology"),
            N_("110-nm MirrorBit(tm) technology"),
            N_("90-nm Floating Gate technology"),
            N_("90-nm MirrorBit(tm) technology")
        };
        const char *process_technology_13[3] = {
            N_("CS49"), N_("CS59"), N_("CS99")
        };
        const char *erase_suspend[3] = {
            N_("Not supported"), N_("Read only"), N_("Read/write")
        };
        const char *sector_protect_scheme[8] = {
            N_("29F040 mode"), N_("29F016 mode"), N_("29F400 mode"),
            N_("29LV800 mode"),
            N_("29BDS640 mode (Software Command Locking)"),
            N_("29BDD160 mode (New Sector Protect)"),
            N_("29PDL128 mode (New Sector Protect + 29LV800)"),
            N_("Advanced Sector Protect")
        };
        const char *page_mode_type[4] = {
            N_("Not supported"), N_("4 word Page"), N_("8 word Page"),
            N_("16 word Page")
        };

        const char *top_bottom[6] = {
            N_("No boot"),
            N_("8x8kb sectors at top and bottom with WP control"),
            N_("Bottom boot device"), N_("Top boot device"),
            N_("Uniform bottom boot device"), N_("Uniform top boot device")
        };
        const char *bad_value = N_("Bad value");

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

        pri_vendor_tbl = cfi->identification_string.pri_vendor_tbl;
        major_version = pri_vendor_tbl->major_version;
        minor_version = pri_vendor_tbl->minor_version;

        printf (_("Primary Vendor-Specific Extended Query:\n"));
        printf (_("\tMajor version number: %c\n"),
                pri_vendor_tbl->major_version);
        printf (_("\tMinor version number: %c\n"),
                pri_vendor_tbl->minor_version);
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '0'))
        {
            if ((pri_vendor_tbl->address_sensitive_unlock & 0x3) <
                ARRAY_SIZE (required_or_not))
                printf (_("\tAddress Sensitive Unlock: %s\n"),
                        required_or_not[pri_vendor_tbl->
                                        address_sensitive_unlock & 0x3]);
            else
                printf (_("\tAddress Sensitive Unlock: %s\n"), bad_value);

            if (major_version > '1'
                || (major_version == '1' && minor_version >= '4'))
            {
                if ((pri_vendor_tbl->address_sensitive_unlock >> 2) <
                    ARRAY_SIZE (process_technology))
                    printf (_("\tProcess Technology: %s\n"),
                            process_technology[pri_vendor_tbl->
                                               address_sensitive_unlock >>
                                               2]);
                else
                    printf (_("\tProcess Technology: %s\n"), bad_value);
            }
            else if (major_version == '1' && minor_version == '3')
            {
                if ((pri_vendor_tbl->address_sensitive_unlock >> 2) <
                    ARRAY_SIZE (process_technology_13))
                    printf (_("\tProcess Technology: %s\n"),
                            process_technology_13[pri_vendor_tbl->
                                                  address_sensitive_unlock >>
                                                  2]);
                else
                    printf (_("\tProcess Technology: %s\n"), bad_value);
            }
            if (pri_vendor_tbl->erase_suspend < ARRAY_SIZE (erase_suspend))
                printf (_("\tErase Suspend: %s\n"),
                        erase_suspend[pri_vendor_tbl->erase_suspend]);
            if (pri_vendor_tbl->sector_protect == 0)
                printf (_("\tSector Protect: Not supported\n"));
            else
                printf (_("\tSector Protect: %d sectors per group\n"),
                        pri_vendor_tbl->sector_protect);
            if (pri_vendor_tbl->sector_temporary_unprotect <
                ARRAY_SIZE (supported_or_not))
                printf (_("\tSector Temporary Unprotect: %s\n"),
                        supported_or_not[pri_vendor_tbl->
                                         sector_temporary_unprotect]);
            else
                printf (_("\tSector Temporary Unprotect: %s\n"), bad_value);
            if (pri_vendor_tbl->sector_protect_scheme <
                ARRAY_SIZE (sector_protect_scheme))
                printf (_("\tSector Protect/Unprotect Scheme: %s\n"),
                        sector_protect_scheme[pri_vendor_tbl->
                                              sector_protect_scheme]);
            else
                printf (_("\tSector Protect/Unprotect Scheme: %s\n"),
                        bad_value);
            if (pri_vendor_tbl->simultaneous_operation == 0)
                printf (_("\tSimultaneous Operation: Not supported\n"));
            else
                printf (_("\tSimultaneous Operation: %d sectors\n"),
                        pri_vendor_tbl->simultaneous_operation);
            if (pri_vendor_tbl->burst_mode_type <
                ARRAY_SIZE (supported_or_not))
                printf (_("\tBurst Mode Type: %s\n"),
                        supported_or_not[pri_vendor_tbl->burst_mode_type]);
            else
                printf (_("\tBurst Mode Type: %s\n"), bad_value);
            if (pri_vendor_tbl->page_mode_type < ARRAY_SIZE (page_mode_type))
                printf (_("\tPage Mode Type: %s\n"),
                        page_mode_type[pri_vendor_tbl->page_mode_type]);
            else
                printf (_("\tPage Mode Type: %s\n"), bad_value);
        }
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '1'))
        {
            printf (_("\tACC (Acceleration) Supply Minimum: %d mV\n"),
                    pri_vendor_tbl->acc_min);
            printf (_("\tACC (Acceleration) Supply Maximum: %d mV\n"),
                    pri_vendor_tbl->acc_max);
            if (pri_vendor_tbl->top_bottom_sector_flag <
                ARRAY_SIZE (top_bottom))
                printf (_("\tTop/Bottom Sector Flag: %s\n"),
                        top_bottom[pri_vendor_tbl->top_bottom_sector_flag]);
            else
                printf (_("\tTop/Bottom Sector Flag: %s\n"), bad_value);
        }
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '2'))
        {
            if (pri_vendor_tbl->program_suspend <
                ARRAY_SIZE (supported_or_not))
                printf (_("\tProgram Suspend: %s\n"),
                        supported_or_not[pri_vendor_tbl->program_suspend]);
            else
                printf (_("\tProgram Suspend: %s\n"), bad_value);
        }
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '4'))
        {
            if (pri_vendor_tbl->unlock_bypass < ARRAY_SIZE (supported_or_not))
                printf (_("\tUnlock Bypass: %s\n"),
                        supported_or_not[pri_vendor_tbl->unlock_bypass]);
            else
                printf (_("\tUnlock Bypass: %s\n"), bad_value);
            printf (_("\tSecSi Sector (Customer OTP Area) Size: %d bytes\n"),
                    pri_vendor_tbl->secsi_sector_size);
            printf (_("\tEmbedded Hardware Reset Timeout Maximum: %d ns\n"),
                    pri_vendor_tbl->embedded_hwrst_timeout_max);
            printf (_
                    ("\tNon-Embedded Hardware Reset Timeout Maximum: %d ns\n"),
                    pri_vendor_tbl->non_embedded_hwrst_timeout_max);
            printf (_("\tErase Suspend Timeout Maximum: %d us\n"),
                    pri_vendor_tbl->erase_suspend_timeout_max);
            printf (_("\tProgram Suspend Timeout Maximum: %d us\n"),
                    pri_vendor_tbl->program_suspend_timeout_max);
        }
        if ((major_version > '1'
             || (major_version == '1' && minor_version >= '3'))
            && pri_vendor_tbl->bank_organization)
        {
            printf (_("\tBank Organization:\n"));
            for (i = 0; i < pri_vendor_tbl->bank_organization; i++)
                printf (_("\t\tBank%d: %d sectors\n"), i + 1,
                        pri_vendor_tbl->bank_region_info[i]);
        }
    }
}
