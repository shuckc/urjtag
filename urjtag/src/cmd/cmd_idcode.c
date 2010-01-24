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
 * Written by Uwe Bonnes <bon@elektron.ikp.physik.tu-darmstadt.de>, 2008.
 *
 */
#include <sysdep.h>

#include <stdio.h>

#include <urjtag/error.h>
#include <urjtag/tap.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_idcode_run (urj_chain_t *chain, char *params[])
{
    long unsigned bytes = 0;

    if (urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_params (params) == 2)
        if (urj_cmd_get_number (params[1], &bytes) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Reading %lu bytes of idcode\n"), bytes);
    return urj_tap_idcode (chain, bytes);
}

static void
cmd_idcode_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s [BYTES]\n"
               "Read [BYTES] IDCODEs of all parts in a JTAG chain.\n"
               "\n"
               "BYTES must be an unsigned integer, and the default is 0.\n"
               "If BYTES is 0, IDCODEs will be read until 32 consecutive zeros are found.\n"),
             "idcode");
}

const urj_cmd_t urj_cmd_idcode = {
    "idcode",
    N_("Read IDCODEs of all parts in a JTAG chain"),
    cmd_idcode_help,
    cmd_idcode_run
};
