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
 * Modified by Ajith Kumar P.C <ajithpc@kila.com>, 20/09/2006
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
 *
 */

#include <sysdep.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/bus.h>
#include <urjtag/jtag.h>
#include <urjtag/flash.h>

#include "flash.h"
#include "cfi.h"
#include "intel.h"
#include "amd.h"

const urj_flash_driver_t * const urj_flash_flash_drivers[] = {
    &urj_flash_amd_32_flash_driver,
    &urj_flash_amd_16_flash_driver,
    &urj_flash_amd_8_flash_driver,
    &urj_flash_intel_32_flash_driver,
    &urj_flash_intel_16_flash_driver,
    &urj_flash_intel_8_flash_driver,
    &urj_flash_amd_29xx040_flash_driver,        //20/09/2006
    NULL
};

static const urj_flash_driver_t *flash_driver = NULL;

static int
set_flash_driver (void)
{
    int i;
    urj_flash_cfi_query_structure_t *cfi;

    flash_driver = NULL;
    if (urj_flash_cfi_array == NULL)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, "global cfi_array not set");
        return URJ_STATUS_FAIL;
    }

    cfi = &urj_flash_cfi_array->cfi_chips[0]->cfi;

    for (i = 0; urj_flash_flash_drivers[i] != NULL; i++)
        if (urj_flash_flash_drivers[i]->autodetect (urj_flash_cfi_array))
        {
            flash_driver = urj_flash_flash_drivers[i];
            flash_driver->print_info (URJ_LOG_LEVEL_NORMAL,
                                      urj_flash_cfi_array);
            return URJ_STATUS_OK;
        }

    urj_log (URJ_LOG_LEVEL_ERROR,
             _("unknown flash - vendor id: %d (0x%04x)\n"),
             cfi->identification_string.pri_id_code,
             cfi->identification_string.pri_id_code);

    urj_error_set (URJ_ERROR_UNSUPPORTED, _("Flash not supported"));

    return URJ_STATUS_FAIL;
}

#define fread_ret(ptr, size, nmemb, stream) \
do { \
    if (fread (ptr, size, nmemb, stream) != (nmemb)) \
    { \
        urj_error_IO_set (_("fread() was short")); \
        return URJ_STATUS_FAIL; \
    } \
} while (0)

int
urj_flashmsbin (urj_bus_t *bus, FILE *f, int noverify)
{
    uint32_t adr;
    urj_flash_cfi_query_structure_t *cfi;

    set_flash_driver ();
    if (!urj_flash_cfi_array || !flash_driver)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("no flash driver found"));
        return URJ_STATUS_FAIL;
    }

    cfi = &urj_flash_cfi_array->cfi_chips[0]->cfi;

    /* test sync bytes */
    {
        char sync[8];
        fread_ret (&sync, sizeof (char), 7, f);
        sync[7] = '\0';
        if (strcmp ("B000FF\n", sync) != 0)
        {
            urj_error_set (URJ_ERROR_INVALID, _("Invalid sync sequence"));
            return URJ_STATUS_FAIL;
        }
    }

    /* erase memory blocks */
    {
        uint32_t start;
        uint32_t len;
        int first, last;
        uint32_t block_size =
            cfi->device_geometry.erase_block_regions[0].erase_block_size;

        fread_ret (&start, sizeof start, 1, f);
        fread_ret (&len, sizeof len, 1, f);
        first = start / (block_size * 2);
        last = (start + len - 1) / (block_size * 2);
        for (; first <= last; first++)
        {
            int r;

            adr = first * block_size * 2;
            // @@@@ RFHH what about returning on error?
            (void) flash_driver->unlock_block (urj_flash_cfi_array, adr);
            urj_log (URJ_LOG_LEVEL_NORMAL, _("block %d unlocked\n"), first);
            // @@@@ RFHH what about returning on error?
            r = flash_driver->erase_block (urj_flash_cfi_array, adr);
            urj_log (URJ_LOG_LEVEL_NORMAL, _("erasing block %d: %d\n"),
                     first, r);
        }
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("program:\n"));
    for (;;)
    {
        uint32_t a, l, c;

        fread_ret (&a, sizeof a, 1, f);
        fread_ret (&l, sizeof l, 1, f);
        fread_ret (&c, sizeof c, 1, f);
        if (feof (f))
        {
            urj_error_IO_set (_("premature end of file"));
            return URJ_STATUS_FAIL;
        }
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("record: start = 0x%08lX, len = 0x%08lX, checksum = 0x%08lX\n"),
                 (long unsigned) a, (long unsigned) l, (long unsigned) c);
        if ((a == 0) && (c == 0))
            break;
        if (l & 3)
        {
            urj_error_set (URJ_ERROR_INVALID, _("Invalid record length"));
            return URJ_STATUS_FAIL;
        }

        while (l)
        {
            uint32_t data;

            urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08lX"),
                     (long unsigned) a);
            urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
            fread_ret (&data, sizeof data, 1, f);
            if (flash_driver->program (urj_flash_cfi_array, a, &data, 1)
                != URJ_STATUS_OK)
                // retain error state
                return URJ_STATUS_FAIL;
            a += 4;
            l -= 4;
        }
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, "\n");

    flash_driver->readarray (urj_flash_cfi_array);

    if (noverify)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, _("verify skipped\n"));
        return URJ_STATUS_OK;
    }

    fseek (f, 15, SEEK_SET);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("verify:\n"));

    for (;;)
    {
        uint32_t a, l, c;

        fread_ret (&a, sizeof a, 1, f);
        fread_ret (&l, sizeof l, 1, f);
        fread_ret (&c, sizeof c, 1, f);
        if (feof (f))
        {
            urj_error_IO_set (_("premature end of file"));
            return URJ_STATUS_FAIL;
        }
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("record: start = 0x%08lX, len = 0x%08lX, checksum = 0x%08lX\n"),
                 (long unsigned) a, (long unsigned) l, (long unsigned) c);
        if ((a == 0) && (c == 0))
            break;
        if (l & 3)
        {
            urj_error_set (URJ_ERROR_INVALID, _("Invalid record length"));
            return URJ_STATUS_FAIL;
        }

        while (l)
        {
            uint32_t data, readed;

            urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08lX"),
                     (long unsigned) a);
            urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
            fread_ret (&data, sizeof data, 1, f);
            readed = URJ_BUS_READ (bus, a);
            if (data != readed)
            {
                urj_error_set (URJ_ERROR_FLASH_PROGRAM,
                               _("verify error: 0x%08lX vs. 0x%08lX at addr %08lX"),
                               (long unsigned) readed, (long unsigned) data,
                               (long unsigned) a);
                return URJ_STATUS_FAIL;
            }
            a += 4;
            l -= 4;
        }
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("\nDone.\n"));

    return URJ_STATUS_OK;
}

static int
find_block (urj_flash_cfi_query_structure_t *cfi, int adr, int bus_width,
            int chip_width, int *bytes_until_next_block)
{
    int i;
    int b = 0;
    int bb = 0;

    for (i = 0; i < cfi->device_geometry.number_of_erase_regions; i++)
    {
        const int region_blocks =
            cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks;
        const int flash_block_size =
            cfi->device_geometry.erase_block_regions[i].erase_block_size;
        const int region_block_size =
            (bus_width / chip_width) * flash_block_size;
        const int region_size = region_blocks * region_block_size;

        if (adr < (bb + region_size))
        {
            int bir = (adr - bb) / region_block_size;
            *bytes_until_next_block = bb + (bir + 1) * region_block_size - adr;
            return b + bir;
        }
        b += region_blocks;
        bb += region_size;
    }
    return -1;
}

int
urj_flashmem (urj_bus_t *bus, FILE *f, uint32_t addr, int noverify)
{
    uint32_t adr;
    urj_flash_cfi_query_structure_t *cfi;
    int *erased;
    int i;
    int neb;
    int bus_width;
    int chip_width;
#define BSIZE (1 << 12)
    uint32_t write_buffer[BSIZE];
    int write_buffer_count;
    uint32_t write_buffer_adr;

    set_flash_driver ();
    if (!urj_flash_cfi_array || !flash_driver)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("no flash driver found"));
        return URJ_STATUS_FAIL;
    }
    cfi = &urj_flash_cfi_array->cfi_chips[0]->cfi;

    bus_width = urj_flash_cfi_array->bus_width;
    chip_width = urj_flash_cfi_array->cfi_chips[0]->width;

    for (i = 0, neb = 0; i < cfi->device_geometry.number_of_erase_regions;
         i++)
        neb +=
            cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks;

    erased = malloc (neb * sizeof *erased);
    if (!erased)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) failed"),
                       neb * sizeof *erased);
        return URJ_STATUS_FAIL;
    }
    for (i = 0; i < neb; i++)
        erased[i] = 0;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("program:\n"));
    adr = addr;
    while (!feof (f))
    {
        uint32_t data;
        uint8_t b[BSIZE];
        int bc = 0, bn = 0, btr = BSIZE;
        int block_no = find_block (cfi, adr - urj_flash_cfi_array->address,
                                   bus_width, chip_width, &btr);

        write_buffer_count = 0;
        write_buffer_adr = adr;

        if (btr > BSIZE)
            btr = BSIZE;
        // @@@@ RFHH check error state?
        bn = fread (b, 1, btr, f);

        if (bn > 0 && !erased[block_no])
        {
            int r;

            // @@@@ RFHH what about returning on error?
            (void) flash_driver->unlock_block (urj_flash_cfi_array, adr);
            urj_log (URJ_LOG_LEVEL_NORMAL, _("\nblock %d unlocked\n"),
                     block_no);
            // @@@@ RFHH what about returning on error?
            r = flash_driver->erase_block (urj_flash_cfi_array, adr);
            urj_log (URJ_LOG_LEVEL_NORMAL, _("erasing block %d: %d\n"),
                     block_no, r);
            erased[block_no] = 1;
        }

        for (bc = 0; bc < bn; bc += flash_driver->bus_width)
        {
            int j;
            if ((adr & (BSIZE - 1)) == 0)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08lX"),
                         (long unsigned) adr);
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
            }

            data = 0;
            for (j = 0; j < flash_driver->bus_width; j++)
                if (urj_get_file_endian () == URJ_ENDIAN_BIG)
                    data = (data << 8) | b[bc + j];
                else
                    data |= b[bc + j] << (j * 8);

            /* store data in write buffer, will be programmed to flash later */
            write_buffer[write_buffer_count++] = data;

            adr += flash_driver->bus_width;
        }

        if (write_buffer_count > 0)
            if (flash_driver->program (urj_flash_cfi_array, write_buffer_adr,
                                       write_buffer, write_buffer_count))
            {
                // retain error state
                return URJ_STATUS_FAIL;
            }

    }
    free (erased);

    urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08lX\n"),
             (long unsigned) adr - flash_driver->bus_width);

    flash_driver->readarray (urj_flash_cfi_array);

    if (noverify)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, _("verify skipped\n"));
        return URJ_STATUS_OK;
    }

    fseek (f, 0, SEEK_SET);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("verify:\n"));
    adr = addr;
    while (!feof (f))
    {
        uint32_t data, readed;
        uint8_t b[BSIZE];
        int bc = 0, bn = 0, btr = BSIZE;

        // @@@@ RFHH check error state?
        bn = fread (b, 1, btr, f);

        /* start consecutive read */
        URJ_BUS_READ_START (bus, adr);

        for (bc = 0; bc < bn; bc += flash_driver->bus_width)
        {
            int j;
            uint32_t next_adr = adr + flash_driver->bus_width;

            if ((adr & 0xFF) == 0)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08lX"),
                         (long unsigned) adr);
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
            }

            data = 0;
            for (j = 0; j < flash_driver->bus_width; j++)
                if (urj_get_file_endian () == URJ_ENDIAN_BIG)
                    data = (data << 8) | b[bc + j];
                else
                    data |= b[bc + j] << (j * 8);

            readed = URJ_BUS_READ_NEXT (bus, next_adr);
            if (data != readed)
            {
                /* end consecutive read */
                (void) URJ_BUS_READ_END (bus);

                urj_error_set (URJ_ERROR_FLASH_PROGRAM,
                               _("addr: 0x%08lX\n verify error:\nread: 0x%08lX\nexpected: 0x%08lX\n"),
                                 (long unsigned) adr, (long unsigned) readed,
                                 (long unsigned) data);
                return URJ_STATUS_FAIL;
            }
            adr = next_adr;
        }

        /* end consecutive read
           this wastes one read access but saves us from determining the for-loop
           finish condition twice within the loop */
        (void) URJ_BUS_READ_END (bus);
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08lX\nDone.\n"),
             (long unsigned) adr - flash_driver->bus_width);

    return URJ_STATUS_OK;
}

int
urj_flasherase (urj_bus_t *bus, uint32_t addr, uint32_t number)
{
    urj_flash_cfi_query_structure_t *cfi;
    uint32_t i;
    int status = URJ_STATUS_OK;
    int bus_width;
    int chip_width;

    set_flash_driver ();
    if (!urj_flash_cfi_array || !flash_driver)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("no flash driver found"));
        return URJ_STATUS_FAIL;
    }
    cfi = &urj_flash_cfi_array->cfi_chips[0]->cfi;

    bus_width = urj_flash_cfi_array->bus_width;
    chip_width = urj_flash_cfi_array->cfi_chips[0]->width;

    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("\nErasing %d Flash block%s from address 0x%lx\n"), number,
             number > 1 ? "s" : "", (long unsigned) addr);

    for (i = 1; i <= number; i++)
    {
        int r;
        int btr = 0;
        int block_no = find_block (cfi, addr - urj_flash_cfi_array->address,
                                   bus_width, chip_width, &btr);

        if (block_no < 0)
        {
            urj_error_set (URJ_ERROR_FLASH_ERASE, "Cannot find block");
            status = URJ_STATUS_FAIL;
            break;
        }

        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("(%d%% Completed) FLASH Block %d : Unlocking ... "),
                i * 100 / number, block_no);
        flash_driver->unlock_block (urj_flash_cfi_array, addr);
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Erasing ... "));
        r = flash_driver->erase_block (urj_flash_cfi_array, addr);
        if (r == URJ_STATUS_OK)
        {
            if (i == number)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         _("(100%% Completed) FLASH Block %d : Unlocking ... Erasing ... Ok.\n"),
                         block_no);
            }
            else
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, _("Ok."));
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
                urj_log (URJ_LOG_LEVEL_NORMAL, _("%78s"), "");
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
            }
        }
        else
        {
            urj_log (URJ_LOG_LEVEL_NORMAL, _("ERROR.\n"));
            status = r;
        }
        addr += btr;
    }

    if (status == URJ_STATUS_OK)
        urj_log (URJ_LOG_LEVEL_NORMAL, _("\nErasing Completed.\n"));
    else
        urj_log (URJ_LOG_LEVEL_NORMAL, _("\nErasing (partially) Failed.\n"));

    /* BYPASS */
    //       urj_part_parts_set_instruction( ps, "BYPASS" );
    //       urj_tap_chain_shift_instructions( chain );

    return status;
}

int
urj_flashlock (urj_bus_t *bus, uint32_t addr, uint32_t number, int unlock)
{
    urj_flash_cfi_query_structure_t *cfi;
    uint32_t i;
    int status = URJ_STATUS_OK;
    int bus_width;
    int chip_width;

    set_flash_driver ();
    if (!urj_flash_cfi_array || !flash_driver)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("no flash driver found"));
        return URJ_STATUS_FAIL;
    }
    cfi = &urj_flash_cfi_array->cfi_chips[0]->cfi;

    bus_width = urj_flash_cfi_array->bus_width;
    chip_width = urj_flash_cfi_array->cfi_chips[0]->width;

    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("\n%s %d Flash block%s from address 0x%lx\n"),
             unlock == 1 ? "Unlocking" : "Locking",
             number,
             number > 1 ? "s" : "", (long unsigned) addr);

    for (i = 1; i <= number; i++)
    {
        int r;
        int btr = 0;
        int block_no = find_block (cfi, addr - urj_flash_cfi_array->address,
                                   bus_width, chip_width, &btr);

        if (block_no < 0)
        {
            urj_error_set ((unlock == 1
                            ? URJ_ERROR_FLASH_UNLOCK : URJ_ERROR_FLASH_LOCK),
                           "Cannot find block");
            status = URJ_STATUS_FAIL;
            break;
        }

        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("(%d%% Completed) FLASH Block %d : %s ... "),
                 i * 100 / number, block_no,
                 unlock == 1 ? "unlocking" : "locking");

        if (unlock)
                r = flash_driver->unlock_block (urj_flash_cfi_array, addr);
        else
                r = flash_driver->lock_block (urj_flash_cfi_array, addr);

        if (r == URJ_STATUS_OK)
        {
            if (i == number)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         _("(100%% Completed) FLASH Block %d : %s ... Ok.\n"),
                         block_no,
                         unlock == 1 ? "unlocking" : "locking");

            }
            else
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, _("Ok."));
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
                urj_log (URJ_LOG_LEVEL_NORMAL, _("%78s"), "");
                urj_log (URJ_LOG_LEVEL_NORMAL, "\r");
            }
        }
        else
        {
            urj_log (URJ_LOG_LEVEL_NORMAL, _("ERROR.\n"));
            status = r;
        }
        addr += btr;
    }

    if (status == URJ_STATUS_OK)
        urj_log (URJ_LOG_LEVEL_NORMAL, _("\n%s Completed.\n"),
                 unlock == 1 ? "Unlocking" : "Locking");
    else
        urj_log (URJ_LOG_LEVEL_NORMAL, _("\n%s (partially) Failed.\n"),
                 unlock == 1 ? "Unlocking" : "Locking");

    return status;
}
