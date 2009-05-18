/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Matan Ziv-Av <matan@svgalib.org>, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdint.h>

#include <urjtag/error.h>
#include <urjtag/bus.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_peek_run (urj_chain_t *chain, char *params[])
{
    long unsigned adr;
    uint32_t val;
    int pars, j = 1;
    urj_bus_area_t area;

    /* urj_bus_t *bus = part_get_active_bus(chain); */

    if ((pars = urj_cmd_params (params)) < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (!urj_bus)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, _("Bus missing"));
        return URJ_STATUS_FAIL;
    }
    if (!urj_bus->driver)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, _("Bus driver missing"));
        return URJ_STATUS_FAIL;
    }

    do
    {
        if (urj_cmd_get_number (params[j], &adr) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

        URJ_BUS_PREPARE (urj_bus);
        URJ_BUS_AREA (urj_bus, adr, &area);
        val = URJ_BUS_READ (urj_bus, adr);

        switch (area.width)
        {
        case 8:
            val &= 0xff;
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("URJ_BUS_READ(0x%08lx) = 0x%02lX (%li)\n"), adr,
                     (long unsigned) val, (long unsigned) val);
            break;
        case 16:
            val &= 0xffff;
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("URJ_BUS_READ(0x%08lx) = 0x%04lX (%li)\n"), adr,
                     (long unsigned) val, (long unsigned) val);
            break;
        default:
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("URJ_BUS_READ(0x%08lx) = 0x%08lX (%li)\n"), adr,
                     (long unsigned) val, (long unsigned) val);
        }
    }
    while (++j != pars);

    return URJ_STATUS_OK;
}

static void
cmd_peek_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s ADDR\n"
               "Read a single word (bus width size).\n"
               "\n"
               "ADDR       address to read from\n"
               "\n"
               "ADDR could be in decimal or hexadecimal (prefixed with 0x) form.\n"
               "\n"),
             "peek");
}

const urj_cmd_t urj_cmd_peek = {
    "peek",
    N_("read a single word"),
    cmd_peek_help,
    cmd_peek_run
};

static int
cmd_poke_run (urj_chain_t *chain, char *params[])
{
    long unsigned adr, val;
    urj_bus_area_t area;
    /*urj_bus_t *bus = part_get_active_bus(chain); */
    int k = 1, pars = urj_cmd_params (params);

    if (pars < 3 || !(pars & 1))
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d and odd-numbered, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (!urj_bus)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, _("Bus missing"));
        return URJ_STATUS_FAIL;
    }
    if (!urj_bus->driver)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, _("Bus driver missing"));
        return URJ_STATUS_FAIL;
    }

    URJ_BUS_PREPARE (urj_bus);

    while (k < pars)
    {
        if (urj_cmd_get_number (params[k], &adr) != URJ_STATUS_OK
            || urj_cmd_get_number (params[k + 1], &val) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;
        URJ_BUS_AREA (urj_bus, adr, &area);
        URJ_BUS_WRITE (urj_bus, adr, val);
        k += 2;
    }

    return URJ_STATUS_OK;
}

static void
cmd_poke_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s ADDR VAL [ADDR VAL] ... \n"
               "Write a single word (bus width size).\n"
               "\n"
               "ADDR       address to write\n"
               "VAL        value to write\n"
               "\n"
               "ADDR and VAL could be in decimal or hexadecimal (prefixed with 0x) form.\n"
               "\n"),
             "poke");
}

const urj_cmd_t urj_cmd_poke = {
    "poke",
    N_("write a single word"),
    cmd_poke_help,
    cmd_poke_run
};
