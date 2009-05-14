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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_endian_run (urj_chain_t *chain, char *params[])
{
    if (urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (!params[1])
    {
        if (urj_big_endian)
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("Endianess for external files: big\n"));
        else
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("Endianess for external files: little\n"));
        return URJ_STATUS_OK;
    }

    if (strcasecmp (params[1], "little") == 0)
    {
        urj_big_endian = 0;
        return URJ_STATUS_OK;
    }
    if (strcasecmp (params[1], "big") == 0)
    {
        urj_big_endian = 1;
        return URJ_STATUS_OK;
    }

    urj_error_set (URJ_ERROR_SYNTAX,
                   "endianness must be 'little' or 'big', not '%s'", params[1]);
    return URJ_STATUS_FAIL;
}

static void
cmd_endian_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s\n"
               "Set or print endianess for external files.\n"),
             "endian [little|big]");
}

const urj_cmd_t urj_cmd_endian = {
    "endian",
    N_("set/print endianess"),
    cmd_endian_help,
    cmd_endian_run
};
