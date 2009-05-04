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

#include <urjtag/sysdep.h>

#include <stdio.h>
#include <string.h>

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
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return 1;

    active_ir = part->active_instruction;
    if (active_ir == NULL)
    {
        printf (_("%s: part without active instruction\n"), "dr");
        return 1;
    }
    dr = active_ir->data_register;
    if (dr == NULL)
    {
        printf (_("%s: part without active data register\n"), "dr");
        return 1;
    }

    if (params[1])
    {
        if (strcasecmp (params[1], "in") == 0)
            dir = 0;
        else if (strcasecmp (params[1], "out") == 0)
            dir = 1;
        else
        {
            unsigned int bit;
            if (strspn (params[1], "01") != strlen (params[1]))
            {
                return -1;
            }

            r = dr->in;
            if (r->len != strlen (params[1]))
            {
                printf (_("%s: register length mismatch\n"), "dr");
                return 1;
            }
            for (bit = 0; params[1][bit]; bit++)
            {
                r->data[r->len - 1 - bit] = (params[1][bit] == '1');
            }

            dir = 0;
        }
    }

    if (dir)
        r = dr->out;
    else
        r = dr->in;
    printf (_("%s\n"), urj_tap_register_get_string (r));

    return 1;
}

static void
cmd_dr_help (void)
{
    printf (_("Usage: %s [DIR]\n"
              "Usage: %s BITSTRING\n"
              "Display input or output data register content or set current register.\n"
              "\n"
              "DIR           requested data register; possible values: 'in' for\n"
              "              input and 'out' for output; default is 'out'\n"
              "BITSTRING     set current data register with BITSTRING (e.g. 01010)\n"),
            "dr", "dr");
}

const urj_cmd_t urj_cmd_dr = {
    "dr",
    N_("display active data register for a part"),
    cmd_dr_help,
    cmd_dr_run
};
