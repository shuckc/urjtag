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
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/params.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_initbus_run (urj_chain_t *chain, char *params[])
{
    if (urj_cmd_params (params) < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    return urj_bus_init (chain, params[1], &params[2]);
}

static void
cmd_initbus_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s BUSNAME\n"
               "Initialize new bus driver for active part.\n"
               "\n"
               "BUSNAME       Name of the bus\n"
               "\n" "List of available buses:\n"),
             "initbus");

    urj_cmd_show_list (urj_bus_drivers);
}

static void
cmd_initbus_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                      char * const *tokens, const char *text, size_t text_len,
                      size_t token_point)
{
    size_t i;

    switch (token_point)
    {
    case 1:
        for (i = 0; urj_bus_drivers[i]; ++i)
            urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                             urj_bus_drivers[i]->name);
        break;
    default:
        urj_completion_mayben_add_param_list (matches, match_cnt, text,
                                              text_len, urj_bus_param_list);
        break;
    }
}

const urj_cmd_t urj_cmd_initbus = {
    "initbus",
    N_("initialize bus driver for active part"),
    cmd_initbus_help,
    cmd_initbus_run,
    cmd_initbus_complete,
};
