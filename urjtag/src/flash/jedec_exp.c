/*
 * $Id$
 *
 * Copyright (C) 2008 Kolja Waschk
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
 * Written by Kolja Waschk, 2008,
 * partially based on snippets from jedec.c/amd_flash.c/cfi.c
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <urjtag/error.h>
#include <urjtag/log.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>
#include <urjtag/bitmask.h>

#include "flash.h"

#include "jedec.h"
#include "cfi.h"
#include "intel.h"

void
urj_flash_jedec_exp_read_id (urj_bus_t *bus, uint32_t adr, uint32_t dmask,
                             uint32_t pata, uint32_t patb, uint32_t dcmd,
                             int det_addroffset, int det_dataoffset,
                             uint32_t det_addrpat)
{
    int locofs;

    det_addrpat <<= det_addroffset;
    urj_log (URJ_LOG_LEVEL_NORMAL,
             "     trying with address pattern base %08x:", det_addrpat);
    URJ_BUS_WRITE (bus, adr + det_addrpat, pata);
    URJ_BUS_WRITE (bus, adr + (det_addrpat >> 1), patb);
    URJ_BUS_WRITE (bus, adr + det_addrpat, dcmd);

    for (locofs = 0; locofs <= 2; locofs++)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, " %08x",
                 (dmask & URJ_BUS_READ (bus, adr + (locofs << det_addroffset)))
                 >> det_dataoffset);
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, "\n");
}

int
urj_flash_jedec_exp_detect (urj_bus_t *bus, uint32_t adr,
                            urj_flash_cfi_array_t **cfi_array)
{
    /* Temporary containers for manufacturer and device id while
       probing with different Autoselect methods. */
    int ba, bw;
    int det_buswidth;
    urj_bus_area_t area;

    *cfi_array = calloc (1, sizeof (urj_flash_cfi_array_t));
    if (!*cfi_array)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       1, sizeof (urj_flash_cfi_array_t));
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

    if (bw == 0)
        bw = 32;                // autodetection!

    if (bw != 8 && bw != 16 && bw != 32)
    {
        urj_error_set (URJ_ERROR_INVALID, "bus width %d", bw);
        return URJ_STATUS_FAIL;
    }
    (*cfi_array)->bus_width = ba = bw / 8;

    (*cfi_array)->cfi_chips = calloc (1, sizeof (urj_flash_cfi_chip_t *));
    if (!(*cfi_array)->cfi_chips)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       1, sizeof (urj_flash_cfi_chip_t *));
        return URJ_STATUS_FAIL;
    }

    (*cfi_array)->cfi_chips[0] = calloc (1, sizeof (urj_flash_cfi_chip_t));
    if (!(*cfi_array)->cfi_chips[0])
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       1, sizeof (urj_flash_cfi_chip_t));
        return URJ_STATUS_FAIL;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL,
        "=== experimental extensive JEDEC brute-force autodetection ===\n");
    for (det_buswidth = bw; det_buswidth >= 8; det_buswidth >>= 1)
    {
        int det_datawidth;
        urj_log (URJ_LOG_LEVEL_NORMAL, "- trying with cpu buswidth %d\n",
                 det_buswidth);
        for (det_datawidth = det_buswidth; det_datawidth >= 8;
             det_datawidth >>= 1)
        {
            int det_dataoffset;
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     "-- trying with flash datawidth %d\n", det_datawidth);
            for (det_dataoffset = 0;
                 det_dataoffset + det_datawidth <= det_buswidth;
                 det_dataoffset += 8)
            {
                int det_addroffset;
                uint32_t dmask = URJ_BITS (det_dataoffset,
                                           det_datawidth + det_dataoffset -
                                           1);
                uint32_t pata = ~dmask | (0xAA << det_dataoffset);
                uint32_t patb = ~dmask | (0x55 << det_dataoffset);
                uint32_t dcmd = ~dmask | (0x90 << det_dataoffset);

                urj_log (URJ_LOG_LEVEL_NORMAL,
                         "--- trying with flash dataoffset %d", det_dataoffset);
                urj_log (URJ_LOG_LEVEL_NORMAL, " (using %08X, %08X and %08X)\n",
                         pata, patb, dcmd);

                for (det_addroffset = 0; det_addroffset <= 2;
                     det_addroffset++)
                {
                    urj_flash_jedec_exp_read_id (bus, adr, dmask, pata, patb,
                                                 dcmd, det_addroffset,
                                                 det_dataoffset, 0x5555);
                    urj_flash_jedec_exp_read_id (bus, adr, dmask, pata, patb,
                                                 dcmd, det_addroffset,
                                                 det_dataoffset, 0x0555);
                }
            }
        }
    }
    urj_log (URJ_LOG_LEVEL_NORMAL,
        "=== end of experimental extensive JEDEC brute-force autodetection ===\n");

    return URJ_STATUS_OK;
}
