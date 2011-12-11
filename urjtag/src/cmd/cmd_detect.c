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
#include <urjtag/tap.h>
#include <urjtag/error.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_detect_run (urj_chain_t *chain, char *params[])
{
    if (urj_cmd_params (params) != 1)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 1, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (urj_tap_detect (chain, 0) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    return URJ_STATUS_OK;
}

static void
cmd_detect_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s\n"
               "Detect parts on the JTAG chain.\n"
               "\n"
               "Output from this command is a list of the detected parts.\n"
               "If no parts are detected other commands may not work properly.\n"),
            "detect");
}

const urj_cmd_t urj_cmd_detect = {
    "detect",
    N_("detect parts on the JTAG chain"),
    cmd_detect_help,
    cmd_detect_run
};
