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
cmd_set_run (urj_chain_t *chain, char *params[])
{
    int dir;
    long unsigned data = 0;
    urj_part_signal_t *s;
    urj_part_t *part;

    if (urj_cmd_params (params) < 4 || urj_cmd_params (params) > 5)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be 4 or 5, not %d",
                       params[0], urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (strcasecmp (params[1], "signal") != 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: second parameter must be '%s'",
                       params[0], params[1]);
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    /* direction */
    if (strcasecmp (params[3], "in") != 0
        && strcasecmp (params[3], "out") != 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: DIR parameter must be 'in' or 'out', not '%s'",
                       params[0], params[3]);
        return URJ_STATUS_FAIL;
    }

    dir = (strcasecmp (params[3], "in") == 0) ? 0 : 1;

    if (dir)
    {
        if (urj_cmd_get_number (params[4], &data) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;
        if (data > 1)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           "%s: DATA parameter must be '0' or '1', not '%s'",
                           params[0], params[4]);
            return URJ_STATUS_FAIL;
        }
    }

    s = urj_part_find_signal (part, params[2]);
    if (!s)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("signal '%s' not found"),
                       params[2]);
        return URJ_STATUS_FAIL;
    }

    return urj_part_set_signal (part, s, dir, data);
}

static void
cmd_set_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s signal SIGNAL DIR [DATA]\n"
               "Set signal state in input BSR (Boundary Scan Register).\n"
               "\n"
               "SIGNAL        signal name (from JTAG declaration file)\n"
               "DIR           requested signal direction; possible values: 'in' or 'out'\n"
               "DATA          desired output signal value ('0' or '1'); used only if DIR\n"
               "                is 'out'\n"),
             "set");
}

static void
cmd_set_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                  char * const *tokens, const char *text, size_t text_len,
                  size_t token_point)
{
    static const char * const dir[] = {
        "in", "out",
    };
    static const char * const data[] = {
        "0", "1",
    };

    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "signal");
        break;

    case 2:  /* name */
        cmd_signal_complete (chain, matches, match_cnt, text, text_len);
        break;

    case 3:  /* direction */
        urj_completion_mayben_add_matches (matches, match_cnt, text, text_len,
                                           dir);
        break;

    case 4:  /* value */
        if (!strcmp (tokens[3], "out"))
            urj_completion_mayben_add_matches (matches, match_cnt, text,
                                               text_len, data);
        break;
    }
}

const urj_cmd_t urj_cmd_set = {
    "set",
    N_("set external signal value"),
    cmd_set_help,
    cmd_set_run,
    cmd_set_complete,
};
