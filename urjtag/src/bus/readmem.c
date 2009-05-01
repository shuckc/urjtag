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

#include <urjtag/bus.h>
#include <urjtag/flash.h>
#include <urjtag/jtag.h>

void
urj_bus_readmem (urj_bus_t *bus, FILE * f, uint32_t addr, uint32_t len)
{
    uint32_t step;
    uint32_t a;
    int bc = 0;
#define BSIZE 4096
    uint8_t b[BSIZE];
    urj_bus_area_t area;
    uint64_t end;

    if (!bus)
    {
        printf (_("Error: Missing bus driver!\n"));
        return;
    }

    URJ_BUS_PREPARE (bus);

    if (URJ_BUS_AREA (bus, addr, &area) != URJ_STATUS_OK)
    {
        printf (_("Error: Bus width detection failed\n"));
        return;
    }
    step = area.width / 8;

    if (step == 0)
    {
        printf (_("Unknown bus width!\n"));
        return;
    }

    addr = addr & (~(step - 1));
    len = (len + step - 1) & (~(step - 1));

    printf (_("address: 0x%08X\n"), addr);
    printf (_("length:  0x%08X\n"), len);

    if (len == 0)
    {
        printf (_("length is 0.\n"));
        return;
    }

    a = addr;
    end = a + len;
    printf (_("reading:\n"));
    URJ_BUS_READ_START (bus, addr);
    for (a += step; a <= end; a += step)
    {
        uint32_t data;
        int j;

        if (a < addr + len)
            data = URJ_BUS_READ_NEXT (bus, a);
        else
            data = URJ_BUS_READ_END (bus);

        for (j = step; j > 0; j--)
            if (urj_big_endian)
                b[bc++] = (data >> ((j - 1) * 8)) & 0xFF;
            else
            {
                b[bc++] = data & 0xFF;
                data >>= 8;
            }

        if ((bc >= BSIZE) || (a >= end))
        {
            printf (_("addr: 0x%08X"), a);
            printf ("\r");
            fflush (stdout);
            fwrite (b, bc, 1, f);
            bc = 0;
        }
    }

    printf (_("\nDone.\n"));
}
