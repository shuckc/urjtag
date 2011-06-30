/*
 * $Id$
 *
 * Copyright (C) 2005 Protoparts
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
 * Written by David Farrell, 2005
 * based on templates by and portions  Written by Marcel Telka <marcel@telka.sk>, 2003.i
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <string.h>
//#include <stdlib.h>

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bssignal.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_test_run (urj_chain_t *chain, char *params[])
{
    int data;
    long unsigned i;
    urj_part_signal_t *s;
    urj_part_t *part;

    if (urj_cmd_params (params) != 4)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 4, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (strcasecmp (params[1], "signal") != 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: parameter[%d] should be '%s', not '%s'",
                       params[0], 1, "signal", params[1]);
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

    /* values 0,1,X since X is not a number, the following failure exits clean
     * and doesnt test anything, as it should.
     */
    if (urj_cmd_get_number (params[3], &i) != URJ_STATUS_OK)
        return URJ_STATUS_OK;

    data = urj_part_get_signal (part, s);
    if (data == -1)
        return URJ_STATUS_FAIL;

    if (data != i)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("<FAIL>%s = %d"), params[2], data);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static void
cmd_test_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s signal SIGNAL 0/1\n"
               "Test signal state from output BSR (Boundary Scan Register).\n"
               "\n"
               "SIGNAL        signal name (from JTAG declaration file)\n"),
             "test");
}

static void
cmd_test_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                   char * const *tokens, const char *text, size_t text_len,
                   size_t token_point)
{
    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "signal");
        break;

    case 2:  /* name */
        cmd_signal_complete (chain, matches, match_cnt, text, text_len);
        break;

    case 3:  /* value */
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "0");
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "1");
        break;
    }
}

const urj_cmd_t urj_cmd_test = {
    "test",
    N_("test external signal value"),
    cmd_test_help,
    cmd_test_run,
    cmd_test_complete,
};
