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
#include <stddef.h>

#include <urjtag/error.h>
#include <urjtag/part.h>
#include <urjtag/part_instruction.h>
#include <urjtag/chain.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_instruction_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    long unsigned len;
    urj_part_instruction_t *i;

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    switch (urj_cmd_params (params))
    {
    case 2:
        urj_part_set_instruction (part, params[1]);
        if (part->active_instruction == NULL)
        {
            urj_error_set (URJ_ERROR_INVALID,
                           _("%s: unknown instruction '%s'"),
                           "instruction", params[1]);
            return URJ_STATUS_FAIL;
        }
        return URJ_STATUS_OK;

    case 3:
        if (strcasecmp (params[1], "length") != 0)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           "param 1 of 3 must be 'length', not '%s'",
                           params[1]);
            return URJ_STATUS_FAIL;
        }

        if (urj_cmd_get_number (params[2], &len) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

        return urj_part_instruction_length_set (part, len);

    case 4:
        i = urj_part_instruction_define (part, params[1], params[2], params[3]);
        if (!i)
            return URJ_STATUS_FAIL;

        return URJ_STATUS_OK;

    default:
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be 2, 3, or 4, not %d",
                       params[0], urj_cmd_params (params));
        break;
    }

    return URJ_STATUS_FAIL;
}

static void
cmd_instruction_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s INSTRUCTION\n"
               "Usage: %s length LENGTH\n"
               "Usage: %s INSTRUCTION CODE REGISTER\n"
               "Change active INSTRUCTION for a part or declare new instruction.\n"
               "\n"
               "INSTRUCTION   instruction name (e.g. BYPASS)\n"
               "LENGTH        common instruction length\n"
               "CODE          instruction code (e.g. 11111)\n"
               "REGISTER      default data register for instruction (e.g. BR)\n"),
             "instruction", "instruction", "instruction");
}

static void
cmd_instruction_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                          char * const *tokens, const char *text, size_t text_len,
                          size_t token_point)
{
    urj_part_t *part;
    urj_part_instruction_t *i;

    if (token_point != 1)
        return;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return;

    i = part->instructions;
    while (i)
    {
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                         i->name);
        i = i->next;
    }
}

const urj_cmd_t urj_cmd_instruction = {
    "instruction",
    N_("change active instruction for a part or declare new instruction"),
    cmd_instruction_help,
    cmd_instruction_run,
    cmd_instruction_complete,
};
