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
#include <urjtag/chain.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_shift_run (urj_chain_t *chain, char *params[])
{
    if (urj_cmd_params (params) != 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (strcasecmp (params[1], "ir") == 0)
    {
        /* @@@@ RFHH check result */
        urj_tap_chain_shift_instructions (chain);
        return URJ_STATUS_OK;
    }
    if (strcasecmp (params[1], "dr") == 0)
    {
        /* @@@@ RFHH check result */
        urj_tap_chain_shift_data_registers (chain, 1);
        return URJ_STATUS_OK;
    }

    urj_error_set (URJ_ERROR_SYNTAX,
                   "%s parameter 2 must be 'ir' or 'dr', not '%s'",
                   params[0], params[1]);
    return URJ_STATUS_FAIL;
}

static void
cmd_shift_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s\n"
               "Usage: %s\n"
               "Shift instruction or data register through JTAG chain.\n"),
            "shift ir", "shift dr");
}

static void
cmd_shift_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                    char * const *tokens, const char *text, size_t text_len,
                    size_t token_point)
{
    if (token_point != 1)
        return;

    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "dr");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "ir");
}

const urj_cmd_t urj_cmd_shift = {
    "shift",
    N_("shift data/instruction registers through JTAG chain"),
    cmd_shift_help,
    cmd_shift_run,
    cmd_shift_complete,
};
