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

#include <sysdep.h>

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include <urjtag/error.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>

#include "flash.h"

#include "jedec.h"
#include "amd.h"
#include "cfi.h"
#include "intel.h"

urj_flash_cfi_array_t *urj_flash_cfi_array = NULL;

static const urj_flash_detect_func_t urj_flash_detect_funcs[] = {
    &urj_flash_cfi_detect,
    &urj_flash_jedec_detect,
    &urj_flash_amd_detect,
#ifdef JEDEC_EXP
    &urj_flash_jedec_exp_detect,
#endif
};

void
urj_flash_cleanup (void)
{
    urj_flash_cfi_array_free (urj_flash_cfi_array);
    urj_flash_cfi_array = NULL;
}

int
urj_flash_detectflash (urj_log_level_t ll, urj_bus_t *bus, uint32_t adr)
{
    urj_flash_cfi_query_structure_t *cfi;
    const char *s;
    int i, ret;

    if (!bus)
    {
        urj_error_set (URJ_ERROR_INVALID, _("bus driver"));
        return URJ_STATUS_FAIL;
    }

    urj_error_reset ();

    urj_flash_cleanup();

    URJ_BUS_PREPARE (bus);

    for (i = 0; i < ARRAY_SIZE (urj_flash_detect_funcs); ++i)
    {
        ret = urj_flash_detect_funcs[i] (bus, adr, &urj_flash_cfi_array);
        if (ret == URJ_STATUS_OK)
            break;
        urj_flash_cleanup ();
    }

    if (urj_flash_cfi_array == NULL)
    {
        /* Preserve error from lower layers if they set one */
        if (urj_error_get () == URJ_ERROR_OK)
            urj_error_set (URJ_ERROR_NOTFOUND, _("Flash not found"));
        return URJ_STATUS_FAIL;
    }

    cfi = &urj_flash_cfi_array->cfi_chips[0]->cfi;

    /* detect CFI capable devices */
    /* TODO: Low chip only */
    /* see 4.3.2 in [1] */
    urj_log (ll, _("Query identification string:\n"));
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
    urj_log (ll, _("\tPrimary Algorithm Command Set and Control Interface ID Code: 0x%04X (%s)\n"),
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
    urj_log (ll, _("\tAlternate Algorithm Command Set and Control Interface ID Code: 0x%04X (%s)\n"),
             cfi->identification_string.alt_id_code, _(s));

    /* see 4.3.3 in [1] */
    urj_log (ll, _("Query system interface information:\n"));
    urj_log (ll, _("\tVcc Logic Supply Minimum Write/Erase or Write voltage: %d mV\n"),
             cfi->system_interface_info.vcc_min_wev);
    urj_log (ll, _("\tVcc Logic Supply Maximum Write/Erase or Write voltage: %d mV\n"),
             cfi->system_interface_info.vcc_max_wev);
    urj_log (ll, _("\tVpp [Programming] Supply Minimum Write/Erase voltage: %d mV\n"),
             cfi->system_interface_info.vpp_min_wev);
    urj_log (ll, _("\tVpp [Programming] Supply Maximum Write/Erase voltage: %d mV\n"),
             cfi->system_interface_info.vpp_max_wev);
    urj_log (ll, _("\tTypical timeout per single byte/word program: %d us\n"),
             (int) cfi->system_interface_info.typ_single_write_timeout);
    urj_log (ll, _("\tTypical timeout for maximum-size multi-byte program: %d us\n"),
             (int) cfi->system_interface_info.typ_buffer_write_timeout);
    urj_log (ll, _("\tTypical timeout per individual block erase: %d ms\n"),
             (int) cfi->system_interface_info.typ_block_erase_timeout);
    urj_log (ll, _("\tTypical timeout for full chip erase: %d ms\n"),
             (int) cfi->system_interface_info.typ_chip_erase_timeout);
    urj_log (ll, _("\tMaximum timeout for byte/word program: %d us\n"),
             (int) cfi->system_interface_info.max_single_write_timeout);
    urj_log (ll, _("\tMaximum timeout for multi-byte program: %d us\n"),
             (int) cfi->system_interface_info.max_buffer_write_timeout);
    urj_log (ll, _("\tMaximum timeout per individual block erase: %d ms\n"),
             (int) cfi->system_interface_info.max_block_erase_timeout);
    urj_log (ll, _("\tMaximum timeout for chip erase: %d ms\n"),
             (int) cfi->system_interface_info.max_chip_erase_timeout);

    /* see 4.3.4 in [1] */
    urj_log (ll, _("Device geometry definition:\n"));
    urj_log (ll, _("\tDevice Size: %d B (%d KiB, %d MiB)\n"),
             (int) cfi->device_geometry.device_size,
             (int) (cfi->device_geometry.device_size / 1024),
             (int) (cfi->device_geometry.device_size / (1024 * 1024)));
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
    urj_log (ll, _("\tFlash Device Interface Code description: 0x%04X (%s)\n"),
             cfi->device_geometry.device_interface, _(s));
    urj_log (ll, _("\tMaximum number of bytes in multi-byte program: %d\n"),
             (int) cfi->device_geometry.max_bytes_write);
    urj_log (ll, _("\tNumber of Erase Block Regions within device: %d\n"),
             cfi->device_geometry.number_of_erase_regions);
    urj_log (ll, _("\tErase Block Region Information:\n"));
    {
        int i;

        for (i = 0; i < cfi->device_geometry.number_of_erase_regions; i++)
        {
            urj_log (ll, _("\t\tRegion %d:\n"), i);
            urj_log (ll, _("\t\t\tErase Block Size: %d B (%d KiB)\n"),
                     (int) cfi->device_geometry.erase_block_regions[i].
                     erase_block_size,
                     (int) cfi->device_geometry.erase_block_regions[i].
                     erase_block_size / 1024);
            urj_log (ll, _("\t\t\tNumber of Erase Blocks: %d\n"),
                     (int) cfi->device_geometry.erase_block_regions[i].
                     number_of_erase_blocks);
        }
    }

    if (cfi->identification_string.pri_id_code == CFI_VENDOR_AMD_SCS
        && cfi->identification_string.pri_vendor_tbl != NULL)
    {
        urj_flash_cfi_amd_pri_extened_query_structure_t *pri_vendor_tbl;
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

        pri_vendor_tbl = cfi->identification_string.pri_vendor_tbl;
        major_version = pri_vendor_tbl->major_version;
        minor_version = pri_vendor_tbl->minor_version;

        urj_log (ll, _("Primary Vendor-Specific Extended Query:\n"));
        urj_log (ll, _("\tMajor version number: %c\n"),
                 pri_vendor_tbl->major_version);
        urj_log (ll, _("\tMinor version number: %c\n"),
                 pri_vendor_tbl->minor_version);
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '0'))
        {
            if ((pri_vendor_tbl->address_sensitive_unlock & 0x3) <
                ARRAY_SIZE (required_or_not))
                urj_log (ll, _("\tAddress Sensitive Unlock: %s\n"),
                         required_or_not[pri_vendor_tbl->
                                         address_sensitive_unlock & 0x3]);
            else
                urj_log (ll, _("\tAddress Sensitive Unlock: %s\n"), bad_value);

            if (major_version > '1'
                || (major_version == '1' && minor_version >= '4'))
            {
                if ((pri_vendor_tbl->address_sensitive_unlock >> 2) <
                    ARRAY_SIZE (process_technology))
                    urj_log (ll, _("\tProcess Technology: %s\n"),
                             process_technology[pri_vendor_tbl->
                                                address_sensitive_unlock >>
                                                2]);
                else
                    urj_log (ll, _("\tProcess Technology: %s\n"), bad_value);
            }
            else if (major_version == '1' && minor_version == '3')
            {
                if ((pri_vendor_tbl->address_sensitive_unlock >> 2) <
                    ARRAY_SIZE (process_technology_13))
                    urj_log (ll, _("\tProcess Technology: %s\n"),
                             process_technology_13[pri_vendor_tbl->
                                                   address_sensitive_unlock >>
                                                   2]);
                else
                    urj_log (ll, _("\tProcess Technology: %s\n"), bad_value);
            }
            if (pri_vendor_tbl->erase_suspend < ARRAY_SIZE (erase_suspend))
                urj_log (ll, _("\tErase Suspend: %s\n"),
                         erase_suspend[pri_vendor_tbl->erase_suspend]);
            if (pri_vendor_tbl->sector_protect == 0)
                urj_log (ll, _("\tSector Protect: Not supported\n"));
            else
                urj_log (ll, _("\tSector Protect: %d sectors per group\n"),
                         pri_vendor_tbl->sector_protect);
            if (pri_vendor_tbl->sector_temporary_unprotect <
                ARRAY_SIZE (supported_or_not))
                urj_log (ll, _("\tSector Temporary Unprotect: %s\n"),
                         supported_or_not[pri_vendor_tbl->
                                          sector_temporary_unprotect]);
            else
                urj_log (ll, _("\tSector Temporary Unprotect: %s\n"),
                         bad_value);
            if (pri_vendor_tbl->sector_protect_scheme <
                ARRAY_SIZE (sector_protect_scheme))
                urj_log (ll, _("\tSector Protect/Unprotect Scheme: %s\n"),
                         sector_protect_scheme[pri_vendor_tbl->
                                               sector_protect_scheme]);
            else
                urj_log (ll, _("\tSector Protect/Unprotect Scheme: %s\n"),
                         bad_value);
            if (pri_vendor_tbl->simultaneous_operation == 0)
                urj_log (ll, _("\tSimultaneous Operation: Not supported\n"));
            else
                urj_log (ll, _("\tSimultaneous Operation: %d sectors\n"),
                         pri_vendor_tbl->simultaneous_operation);
            if (pri_vendor_tbl->burst_mode_type <
                ARRAY_SIZE (supported_or_not))
                urj_log (ll, _("\tBurst Mode Type: %s\n"),
                         supported_or_not[pri_vendor_tbl->burst_mode_type]);
            else
                urj_log (ll, _("\tBurst Mode Type: %s\n"),
                         bad_value);
            if (pri_vendor_tbl->page_mode_type < ARRAY_SIZE (page_mode_type))
                urj_log (ll, _("\tPage Mode Type: %s\n"),
                         page_mode_type[pri_vendor_tbl->page_mode_type]);
            else
                urj_log (ll, _("\tPage Mode Type: %s\n"),
                         bad_value);
        }
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '1'))
        {
            urj_log (ll, _("\tACC (Acceleration) Supply Minimum: %d mV\n"),
                     pri_vendor_tbl->acc_min);
            urj_log (ll, _("\tACC (Acceleration) Supply Maximum: %d mV\n"),
                     pri_vendor_tbl->acc_max);
            if (pri_vendor_tbl->top_bottom_sector_flag <
                ARRAY_SIZE (top_bottom))
                urj_log (ll, _("\tTop/Bottom Sector Flag: %s\n"),
                         top_bottom[pri_vendor_tbl->top_bottom_sector_flag]);
            else
                urj_log (ll, _("\tTop/Bottom Sector Flag: %s\n"), bad_value);
        }
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '2'))
        {
            if (pri_vendor_tbl->program_suspend <
                ARRAY_SIZE (supported_or_not))
                urj_log (ll, _("\tProgram Suspend: %s\n"),
                         supported_or_not[pri_vendor_tbl->program_suspend]);
            else
                urj_log (ll, _("\tProgram Suspend: %s\n"),
                         bad_value);
        }
        if (major_version > '1'
            || (major_version == '1' && minor_version >= '4'))
        {
            if (pri_vendor_tbl->unlock_bypass < ARRAY_SIZE (supported_or_not))
                urj_log (ll, _("\tUnlock Bypass: %s\n"),
                         supported_or_not[pri_vendor_tbl->unlock_bypass]);
            else
                urj_log (ll, _("\tUnlock Bypass: %s\n"), bad_value);
            urj_log (ll, _("\tSecSi Sector (Customer OTP Area) Size: %d bytes\n"),
                     pri_vendor_tbl->secsi_sector_size);
            urj_log (ll, _("\tEmbedded Hardware Reset Timeout Maximum: %d ns\n"),
                     pri_vendor_tbl->embedded_hwrst_timeout_max);
            urj_log (ll, _("\tNon-Embedded Hardware Reset Timeout Maximum: %d ns\n"),
                     pri_vendor_tbl->non_embedded_hwrst_timeout_max);
            urj_log (ll, _("\tErase Suspend Timeout Maximum: %d us\n"),
                     pri_vendor_tbl->erase_suspend_timeout_max);
            urj_log (ll, _("\tProgram Suspend Timeout Maximum: %d us\n"),
                     pri_vendor_tbl->program_suspend_timeout_max);
        }
        if ((major_version > '1'
             || (major_version == '1' && minor_version >= '3'))
            && pri_vendor_tbl->bank_organization)
        {
            urj_log (ll, _("\tBank Organization:\n"));
            for (i = 0; i < pri_vendor_tbl->bank_organization; i++)
                urj_log (ll, _("\t\tBank%d: %d sectors\n"),
                         i + 1, pri_vendor_tbl->bank_region_info[i]);
        }
    }

    return URJ_STATUS_OK;
}
