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
#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_get_run (urj_chain_t *chain, char *params[])
{
    int data;
    urj_part_signal_t *s;
    urj_part_t *part;

    if (urj_cmd_params (params) != 3)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (strcasecmp (params[1], "signal") != 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "params[1] must be 'signal', not '%s'", params[1]);
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    s = urj_part_find_signal (part, params[2]);
    if (!s)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("signal '%s' not found"),
                       params[2]);
        return URJ_STATUS_FAIL;
    }
    data = urj_part_get_signal (part, s);
    if (data == -1)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("%s = %d\n"), params[2], data);

    return URJ_STATUS_OK;
}

static void
cmd_get_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s signal SIGNAL\n"
               "Get signal state from output BSR (Boundary Scan Register).\n"
               "\n"
               "SIGNAL        signal name (from JTAG declaration file)\n"),
             "get");
}

static void
cmd_get_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                  char * const *tokens, const char *text, size_t text_len,
                  size_t token_point)
{
    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "signal");
        break;

    case 2:
        cmd_signal_complete (chain, matches, match_cnt, text, text_len);
        break;
    }
}

const urj_cmd_t urj_cmd_get = {
    "get",
    N_("get external signal value"),
    cmd_get_help,
    cmd_get_run,
    cmd_get_complete,
};
