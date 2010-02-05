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

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/bus.h>
#include <urjtag/flash.h>
#include <urjtag/jtag.h>

int
urj_bus_readmem (urj_bus_t *bus, FILE *f, uint32_t addr, uint32_t len)
{
    uint32_t step;
    uint64_t a;
    size_t bc = 0;
#define BSIZE 4096
    uint8_t b[BSIZE];
    urj_bus_area_t area;
    uint64_t end;

    if (!bus)
    {
        urj_error_set (URJ_ERROR_NO_BUS_DRIVER, _("Missing bus driver"));
        return URJ_STATUS_FAIL;
    }

    URJ_BUS_PREPARE (bus);

    if (URJ_BUS_AREA (bus, addr, &area) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    step = area.width / 8;

    if (step == 0)
    {
        urj_error_set (URJ_ERROR_INVALID,  _("Unknown bus width"));
        return URJ_STATUS_FAIL;
    }
    if (BSIZE % step != 0)
    {
        urj_error_set (URJ_ERROR_INVALID, "step %lu must divide BSIZE %d",
                       (long unsigned) step, BSIZE);
        return URJ_STATUS_FAIL;
    }

    addr = addr & (~(step - 1));
    len = (len + step - 1) & (~(step - 1));

    urj_log (URJ_LOG_LEVEL_NORMAL, _("address: 0x%08lX\n"),
             (long unsigned) addr);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("length:  0x%08lX\n"),
             (long unsigned) len);

    if (len == 0)
    {
        urj_error_set (URJ_ERROR_INVALID, _("length is 0"));
        return URJ_STATUS_FAIL;
    }

    a = addr;
    end = a + len;
    urj_log (URJ_LOG_LEVEL_NORMAL, _("reading:\n"));

    if (URJ_BUS_READ_START (bus, addr) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    for (a += step; a <= end; a += step)
    {
        uint32_t data;
        int j;

        if (a < end)
            data = URJ_BUS_READ_NEXT (bus, a);
        else
            data = URJ_BUS_READ_END (bus);

        for (j = step; j > 0; j--)
            if (urj_get_file_endian () == URJ_ENDIAN_BIG)
                b[bc++] = (data >> ((j - 1) * 8)) & 0xFF;
            else
            {
                b[bc++] = data & 0xFF;
                data >>= 8;
            }

        if ((bc >= BSIZE) || (a >= end))
        {
            urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08llX\r"),
                     (long long unsigned) a);
            if (fwrite (b, bc, 1, f) != 1)
            {
                urj_error_set (URJ_ERROR_FILEIO, "fwrite fails");
                urj_error_state.sys_errno = ferror(f);
                clearerr(f);
                return URJ_STATUS_FAIL;
            }
            bc = 0;
        }
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("\nDone.\n"));

    return URJ_STATUS_OK;
}
