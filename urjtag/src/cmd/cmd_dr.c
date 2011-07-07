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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/part_instruction.h>
#include <urjtag/data_register.h>
#include <urjtag/tap_register.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_dr_run (urj_chain_t *chain, char *params[])
{
    int dir = 1;
    urj_part_t *part;
    urj_tap_register_t *r;
    urj_data_register_t *dr;
    urj_part_instruction_t *active_ir;

    if (urj_cmd_params (params) < 1 || urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= 1 and <= 2, not %d",
                       params[0], urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    active_ir = part->active_instruction;
    if (active_ir == NULL)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("%s: part without active instruction"), "dr");
        return URJ_STATUS_FAIL;
    }
    dr = active_ir->data_register;
    if (dr == NULL)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("%s: instruction without active data register"), "dr");
        return URJ_STATUS_FAIL;
    }

    if (params[1])
    {
        if (strcasecmp (params[1], "in") == 0)
            dir = 0;
        else if (strcasecmp (params[1], "out") == 0)
            dir = 1;
        else
        {
            int ret = urj_tap_register_set_string (dr->in, params[1]);
            if (ret != URJ_STATUS_OK)
                return ret;
            dir = 0;
        }
    }

    if (dir)
        r = dr->out;
    else
        r = dr->in;
    urj_log (URJ_LOG_LEVEL_NORMAL, "%s (0x%0*" PRIX64 ")\n",
             urj_tap_register_get_string (r), r->len / 4,
             urj_tap_register_get_value (r));

    return URJ_STATUS_OK;
}

static void
cmd_dr_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                 char * const *tokens, const char *text, size_t text_len,
                 size_t token_point)
{
    static const char * const dir[] = {
        "in",
        "out",
    };

    if (token_point != 1)
        return;

    urj_completion_mayben_add_matches (matches, match_cnt, text, text_len, dir);
}

static void
cmd_dr_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %1$s [DIR]\n"
               "Usage: %1$s BITSTRING\n"
               "Usage: %1$s HEXSTRING\n"
               "Display input or output data register content or set current register.\n"
               "\n"
               "DIR              requested data register; possible values: 'in' for\n"
               "                 input and 'out' for output; default is 'out'\n"
               "BITSTRING        set current data register with BITSTRING (e.g. 01010)\n"
               "HEXSTRING        set current data register with HEXSTRING (e.g. 0x123)\n"),
             "dr");
}

const urj_cmd_t urj_cmd_dr = {
    "dr",
    N_("display active data register for a part"),
    cmd_dr_help,
    cmd_dr_run,
    cmd_dr_complete,
};
