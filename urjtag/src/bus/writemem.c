/*
 * $Id$
 *
 * Written by Kent Palmkvist (kentp@isy.liu.se>, 2005.
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
urj_bus_writemem (urj_bus_t *bus, FILE *f, uint32_t addr, uint32_t len)
{
    uint32_t step;
    uint64_t a;
    size_t bc = 0;
    int bidx = 0;
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
        urj_error_set (URJ_ERROR_INVALID, _("Unknown bus width"));
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
    urj_log (URJ_LOG_LEVEL_NORMAL, _("writing:\n"));

    for (; a < end; a += step)
    {
        uint32_t data;
        int j;

        /* Read one block of data */
        if (bc == 0)
        {
            urj_log (URJ_LOG_LEVEL_NORMAL, _("addr: 0x%08llX\r"),
                     (long long unsigned) a);
            bc = fread (b, 1, BSIZE, f);
            if (bc != BSIZE)
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, _("Short read: bc=0x%zX\n"), bc);
                if (bc < step)
                {
                    // Not even enough for one step. Something is wrong. Check
                    // the file state and bail out.
                    if (feof (f))
                        urj_error_set (URJ_ERROR_FILEIO,
                            _("Unexpected end of file; Addr: 0x%08llX\n"),
                            (long long unsigned) a);
                    else
                    {
                        urj_error_set (URJ_ERROR_FILEIO, "fread fails");
                        urj_error_state.sys_errno = ferror(f);
                        clearerr(f);
                    }

                    return URJ_STATUS_FAIL;
                }
                /* else, process what we have read, then return to fread() to
                 * meet the error condition (again) */
            }
            bidx = 0;
        }

        /* Write a word at a time */
        data = 0;
        for (j = step; j > 0 && bc > 0; j--)
        {
            if (urj_get_file_endian () == URJ_ENDIAN_BIG)
            {
                /* first shift doesn't matter: data = 0 */
                data <<= 8;
                data |= b[bidx++];
            }
            else
                data |= (b[bidx++] << ((step - j) * 8));
            bc--;
        }

        URJ_BUS_WRITE (bus, a, data);
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("\nDone.\n"));

    return URJ_STATUS_OK;
}
