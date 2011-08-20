/*
 * $Id$
 *
 * AMD 8 bit flash driver for AM29F040B & AM29LV040B
 * Copyright (C) 2006 Kila Medical Systems.
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
 * Written by Ajith Kumar P.C <ajithpc@kila.com>
 *
 * Documentation:
 * [1] Spansion, Am29F040B Data Sheet
 * [2] Spansion, Am29LV040B Data Sheet
*/

#include <sysdep.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>     /* usleep */

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>

#include "flash.h"
#include "amd.h"
#include "cfi.h"

//write specific
#define AMD_SECTOR_PROTECTED

//Read Specific
#define AMD_READ_IN_ERASE_SUSPENDED_SECTOR
#define AMD_READ_IN_NON_ERASE_SUSPENDED_SECTOR
#define AMD_NORMAL_READ
#define AMD_UNKNOWN_READ

//Mode
#define AMD_ERASE_SUSPEND_MODE
#define AMD_READING_MODE
#define AMD_EMBEDDED_ERASE_ALGORITHM
#define AMD_EMBEDDED_PROGRAM_ALGORITHM
#define AMD_UNDEFINED_MODE

#define FLASH_ERASE_ERROR                       (-5)
#define ERASE_FLASH_SUCCESS                     1

#define AMD_29xx040B    1

#define AMD_BYPASS_UNLOCK_ALGORITHM             1
#define AMD_STANDARD_WRITE_ALGORITHM            0
#define AMD_BYPASS_UNLOCK_MODE                  1
#define AMD_STANDARD_MODE                       0

static struct
{
    long unsigned flash;
    unsigned short algorithm;
    unsigned short unlock_bypass;
}
var_forced_detection;


int
urj_flash_amd_detect (urj_bus_t *bus, uint32_t adr,
                      urj_flash_cfi_array_t **cfi_array)
{
    int mid;
    int did;
    urj_bus_area_t area;
    urj_flash_cfi_query_structure_t *cfi;

    if (!cfi_array || !bus)
    {
        urj_error_set (URJ_ERROR_INVALID, "cfi_array or bus");
        return URJ_STATUS_FAIL;
    }

    *cfi_array = calloc (1, sizeof (urj_flash_cfi_array_t));
    if (!*cfi_array)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) failed",
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
    URJ_BUS_WRITE (bus, adr + 0x0, 0xf0);
    URJ_BUS_WRITE (bus, adr + 0x555, 0xaa);
    URJ_BUS_WRITE (bus, adr + 0x2AA, 0x55);
    URJ_BUS_WRITE (bus, adr + 0x555, 0x90);
    mid = URJ_BUS_READ (bus, adr + 0x0);
    did = URJ_BUS_READ (bus, adr + 0x1);
    URJ_BUS_WRITE (bus, adr + 0x0, 0xf0);

    urj_log (URJ_LOG_LEVEL_NORMAL, "%s: mid %x, did %x\n", __func__, mid, did);
    if (mid != 0x01)
    {
        urj_error_set (URJ_ERROR_FLASH, "mid != 0x01");
        return URJ_STATUS_FAIL;
    }

    switch (did)
    {
    case 0xA4:
        var_forced_detection.flash = AMD_29xx040B;
        break;
    case 0x4F:
        var_forced_detection.flash = AMD_29xx040B;
        var_forced_detection.algorithm = AMD_BYPASS_UNLOCK_ALGORITHM;
        break;
    default:
        break;
    }

    unsigned int bw = area.width;
    int ba, i;
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
                       (size_t) ba, sizeof (urj_flash_cfi_chip_t *));
        return URJ_STATUS_FAIL;
    }
    for (i = 0; i < ba; i++)
    {
        (*cfi_array)->cfi_chips[i] = calloc (1, sizeof (urj_flash_cfi_chip_t));
        if (!(*cfi_array)->cfi_chips[i])
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                           (size_t) 1, sizeof (urj_flash_cfi_chip_t));
            return URJ_STATUS_FAIL;
        }
        (*cfi_array)->cfi_chips[i]->width = 1;        //ba;
        cfi = &(*cfi_array)->cfi_chips[i]->cfi;

        cfi->identification_string.pri_id_code = CFI_VENDOR_NULL;
        cfi->identification_string.pri_vendor_tbl = NULL;
        cfi->identification_string.alt_id_code = 0;
        cfi->identification_string.alt_vendor_tbl = NULL;

        cfi->device_geometry.device_size = 512 * 1024;
        cfi->device_geometry.device_interface = 0;      // x 8
        cfi->device_geometry.max_bytes_write = 32;      //not used
        cfi->device_geometry.number_of_erase_regions = 1;
        cfi->device_geometry.erase_block_regions =
            malloc (cfi->device_geometry.number_of_erase_regions *
                    sizeof (urj_flash_cfi_erase_block_region_t));
        if (!cfi->device_geometry.erase_block_regions)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                           sizeof (urj_flash_cfi_erase_block_region_t));
            return URJ_STATUS_FAIL;
        }

        cfi->device_geometry.erase_block_regions[0].erase_block_size =
            64 * 1024;
        cfi->device_geometry.erase_block_regions[0].number_of_erase_blocks =
            8;
        //Add other details for info
    }

    return URJ_STATUS_OK;
}


static int
amd_29xx040_autodetect (urj_flash_cfi_array_t *cfi_array)
{
    return (var_forced_detection.flash == AMD_29xx040B);        //Non-CFI Am29xx040B flash
}

static int
amd_29xx040_status (urj_bus_t *bus, uint32_t adr, unsigned short data)
{
    short timeout;
    unsigned short dq7bit, dq7mask, dq5mask;
    unsigned short data1;

    dq7mask = (1 << 7);
    dq5mask = (1 << 5);
    dq7bit = data & dq7mask;

    for (timeout = 0; timeout < 1000; timeout++)        //typical sector erase time = 0.7 sec
    {
        data1 = (unsigned short) (URJ_BUS_READ (bus, adr) & 0xFF);
        if ((data1 & dq7mask) == dq7bit)
            return URJ_STATUS_OK;           //Success

        if ((data1 & dq5mask) == dq5mask)
        {
            data1 = (unsigned short) (URJ_BUS_READ (bus, adr) & 0xFF);
            if ((data1 & dq7mask) == dq7bit)
                return URJ_STATUS_OK;       //Success
            else
            {
                urj_error_set (URJ_ERROR_FLASH,
                               "status failure: needs a reset command to return back to read array data");
                return URJ_STATUS_FAIL;
            }
        }
        usleep (50);
    }

    urj_error_set (URJ_ERROR_FLASH, "hardware failure");
    return URJ_STATUS_FAIL;
}



static void
amd_29xx040_print_info (urj_log_level_t ll, urj_flash_cfi_array_t *cfi_array)
{
    int mid, did, prot;
    urj_bus_t *bus = cfi_array->bus;

    URJ_BUS_WRITE (bus, cfi_array->address + 0x0, 0xf0);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0xaa);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x2AA, 0x55);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0x90);
    mid = URJ_BUS_READ (bus, cfi_array->address + 0x0);
    did = URJ_BUS_READ (bus, cfi_array->address + 0x1);
    prot = URJ_BUS_READ (bus, cfi_array->address + 0x2);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x0, 0xf0);

    urj_log (ll, "%s: mid %x, did %x\n", __FUNCTION__, mid, did);
//      amd_29xx040_read_array( cfi_array );            /* AMD reset */

    switch (mid)
    {
    case 0x01:
        urj_log (ll, _("Chip: AMD Flash\n\tPartNumber: "));
        break;
    default:
        urj_log (ll, _("Unknown manufacturer (ID 0x%04x)"), mid);
        break;
    }
    urj_log (ll, _("\n\tChip: "));
    switch (did)
    {
    case 0xA4:
        urj_log (ll, "Am29C040B\t-\t");
        urj_log (ll, _("5V Flash\n"));
        break;
    case 0x4F:
        urj_log (ll, "Am29LV040B\t-\t");
        urj_log (ll, _("3V Flash\n"));
        break;
    default:
        urj_log (ll, _("Unknown (ID 0x%04x)"), did);
        break;
    }
    urj_log (ll, _("\n\tProtected: %04x\n"), prot);
}

static void
amd_29xx040_read_array (urj_flash_cfi_array_t *cfi_array)
{
    /* Read Array */
    if (var_forced_detection.unlock_bypass == AMD_BYPASS_UNLOCK_MODE)
    {
        /* @@@@ RFHH: changed this without understanding */
        URJ_BUS_WRITE (cfi_array->bus,
                       cfi_array->address + 0x555, 0x90);
        URJ_BUS_WRITE (cfi_array->bus,
                       cfi_array->address + 0x2AA, 0x00);
        usleep (100);
        var_forced_detection.unlock_bypass = AMD_STANDARD_MODE;
    }
    URJ_BUS_WRITE (cfi_array->bus, cfi_array->address + 0x0, 0x0F0);        /* AMD reset */
}



static int
amd_29xx040_erase_block (urj_flash_cfi_array_t *cfi_array,
                         uint32_t adr)
{
    urj_bus_t *bus = cfi_array->bus;

    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_erase_block 0x%08lX\n",
             (long unsigned) adr);

    /*      urj_log (URJ_LOG_LEVEL_NORMAL, "protected: %d\n", amdisprotected(ps, adr)); */

    if (var_forced_detection.unlock_bypass == AMD_BYPASS_UNLOCK_MODE)
    {
        URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0x90);
        URJ_BUS_WRITE (bus, cfi_array->address + 0x2AA, 0x00);
        usleep (100);
        var_forced_detection.unlock_bypass = AMD_STANDARD_MODE;
    }

    URJ_BUS_WRITE (bus, cfi_array->address + 0x0, 0xf0);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0xaa);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x2AA, 0x55);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0x80);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0xaa);
    URJ_BUS_WRITE (bus, cfi_array->address + 0x2AA, 0x55);
//      URJ_BUS_WRITE( bus, cfi_array->address + 0x555, 0x10 );     //Chip Erase
    URJ_BUS_WRITE (bus, adr, 0x30);     //Sector erase


    if (amd_29xx040_status (bus, adr, 0xff) == URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, "flash_erase_block 0x%08lX DONE\n",
                 (long unsigned) adr);
        amd_29xx040_read_array (cfi_array);   /* AMD reset */
        return URJ_STATUS_OK;
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_erase_block 0x%08lX FAILED\n",
             (long unsigned) adr);
    /* Read Array */
    amd_29xx040_read_array (cfi_array);       /* AMD reset */

    urj_error_set (URJ_ERROR_FLASH_ERASE, "erase block");
    return URJ_STATUS_FAIL;
}

static int
amd_29xx040_program_single (urj_flash_cfi_array_t *cfi_array,
                            uint32_t adr, uint32_t data)
{
    int status;
    urj_bus_t *bus = cfi_array->bus;

    urj_log (URJ_LOG_LEVEL_DETAIL, "\nflash_program 0x%08lX = 0x%08lX\n",
             (long unsigned) adr, (long unsigned) data);

    if (var_forced_detection.algorithm == AMD_BYPASS_UNLOCK_ALGORITHM)
    {
        if (var_forced_detection.unlock_bypass != AMD_BYPASS_UNLOCK_MODE)
        {
            URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0xaa);
            URJ_BUS_WRITE (bus, cfi_array->address + 0x2AA, 0x55);
            URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0x20);
            usleep (1000);
            var_forced_detection.unlock_bypass = AMD_BYPASS_UNLOCK_MODE;
        }
    }
    else
    {
        URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0xaa);
        URJ_BUS_WRITE (bus, cfi_array->address + 0x2AA, 0x55);
    }

    URJ_BUS_WRITE (bus, cfi_array->address + 0x555, 0xA0);
    URJ_BUS_WRITE (bus, adr, data);
    status = amd_29xx040_status (bus, adr, data);
    /*      amd_29xx040_read_array(cfi_array); */

    return status;
}

static int
amd_29xx040_program (urj_flash_cfi_array_t *cfi_array,
                     uint32_t adr, uint32_t *buffer, int count)
{
    int idx;

    /* unroll buffer to single writes */
    for (idx = 0; idx < count; idx++)
    {
        int status = amd_29xx040_program_single (cfi_array, adr,
                                                 buffer[idx]);
        if (status != URJ_STATUS_OK)
            return status;
        adr += cfi_array->bus_width;
    }

    return URJ_STATUS_OK;
}

static int
amd_29xx040_unlock_block (urj_flash_cfi_array_t *cfi_array,
                          uint32_t adr)
{
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_unlock_block 0x%08lX IGNORE\n",
             (long unsigned) adr);
    return URJ_STATUS_OK;
}

static int
amd_29xx040_lock_block (urj_flash_cfi_array_t *cfi_array,
                        uint32_t adr)
{
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_lock_block 0x%08lX IGNORE\n",
             (long unsigned) adr);
    return URJ_STATUS_OK;
}


const urj_flash_driver_t urj_flash_amd_29xx040_flash_driver = {
    N_("AMD Standard Command Set"),
    N_("supported: AMD 29LV040B, 29C040B, 1x8 Bit"),
    1,                          /* buswidth */
    amd_29xx040_autodetect,
    amd_29xx040_print_info,
    amd_29xx040_erase_block,
    amd_29xx040_lock_block,
    amd_29xx040_unlock_block,
    amd_29xx040_program,
    amd_29xx040_read_array,
};
