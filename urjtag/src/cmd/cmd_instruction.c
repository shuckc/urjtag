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

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

static int
cmd_instruction_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;

    if (!urj_cmd_test_cable (chain))
        return 1;

    if (!chain->parts)
    {
        printf (_("Run \"detect\" first.\n"));
        return 1;
    }

    if (chain->active_part >= chain->parts->len)
    {
        printf (_("%s: no active part\n"), "instruction");
        return 1;
    }

    part = chain->parts->parts[chain->active_part];

    if (urj_cmd_params (params) == 2)
    {
        urj_part_set_instruction (part, params[1]);
        if (part->active_instruction == NULL)
            printf (_("%s: unknown instruction '%s'\n"), "instruction",
                    params[1]);
        return 1;
    }

    if (urj_cmd_params (params) == 3)
    {
        unsigned int len;

        if (strcasecmp (params[1], "length") != 0)
            return -1;

        if (urj_cmd_get_number (params[2], &len))
            return -1;

        return urj_part_instruction_length_set (part, len);
    }

    if (urj_cmd_params (params) == 4)
    {
        urj_part_instruction_t *i;

        i = urj_part_instruction_define (part, params[1], params[2], params[3]);
        if (!i)
        {
            return 1;
        }

        return 1;
    }

    return -1;
}

static void
cmd_instruction_help (void)
{
    printf (_("Usage: %s INSTRUCTION\n"
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

urj_cmd_t urj_cmd_instruction = {
    "instruction",
    N_("change active instruction for a part or declare new instruction"),
    cmd_instruction_help,
    cmd_instruction_run
};
