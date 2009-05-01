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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <urjtag/bsbit.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

static void
cmd_bit_print_params (char *params[], unsigned int parameters, char *command,
                      size_t command_size)
{
    unsigned int i;

    command[0] = '\0';
    strncat (command, params[0], command_size);
    for (i = 1; i < parameters; i++)
    {
        strncat (command, " ", command_size);
        strncat (command, params[i], command_size);
    }
}

static int
cmd_bit_run (urj_chain_t *chain, char *params[])
{
    unsigned int bit;
    int type;
    int safe;
    unsigned int control;
    unsigned int parameters = urj_cmd_params (params);
    urj_bsbit_t *bsbit;
    char command[1024];

    cmd_bit_print_params (params, parameters, command, sizeof command);

    if ((parameters != 5) && (parameters != 8))
    {
        printf (_("%s: invalid number of parameters (%d) for command '%s'\n"),
                "bit", parameters, command);
        return -1;
    }

    if (!urj_cmd_test_cable (chain))
    {
        printf (_("%s: cable test failed for command '%s'\n"), "bit",
                command);
        return 1;
    }

    /* bit number */
    if (urj_cmd_get_number (params[1], &bit))
    {
        printf (_("%s: unable to get boundary bit number for command '%s'\n"),
                "bit", command);
        return -1;
    }

    /* bit type */
    if (strlen (params[2]) != 1)
    {
        printf (_("%s: invalid bit type length for command '%s'\n"), "bit",
                command);
        return -1;
    }
    switch (params[2][0])
    {
    case 'I':
    case 'i':
        type = URJ_BSBIT_INPUT;
        break;
    case 'O':
    case 'o':
        type = URJ_BSBIT_OUTPUT;
        break;
    case 'B':
    case 'b':
        type = URJ_BSBIT_BIDIR;
        break;
    case 'C':
    case 'c':
        type = URJ_BSBIT_CONTROL;
        break;
    case 'X':
    case 'x':
        type = URJ_BSBIT_INTERNAL;
        break;
    default:
        printf (_("%s: invalid bit type for command '%s'\n"), "bit", command);
        return -1;
    }

    /* default (safe) value */
    if (strlen (params[3]) != 1)
    {
        printf (_("%s: invalid default value length for command '%s'\n"),
                "bit", command);
        return -1;
    }
    safe = (params[3][0] == '1');

    /* test for control bit */
    if (urj_cmd_params (params) == 5) {
        bsbit = urj_part_bsbit_alloc (chain, bit, params[4], type, safe);
        if (bsbit == NULL)
        {
            return -1;
        }

    } else {
        int control_value;
        int control_state;

        /* control bit number */
        if (urj_cmd_get_number (params[5], &control))
        {
            printf (_("%s: unable to get control bit number for command '%s'\n"),
                    "bit", command);
            return -1;
        }
        /* control value */
        if (strlen (params[6]) != 1)
        {
            printf (_("%s: invalid control value length for command '%s'\n"),
                    "bit", command);
            return -1;
        }
        control_value = (params[6][0] == '1');

        /* control state */
        if (strcasecmp (params[7], "Z"))
            return -1;

        control_state = URJ_BSBIT_STATE_Z;

        bsbit = urj_part_bsbit_alloc_control (chain, bit, params[4], type, safe,
                                              control, control_value,
                                              control_state);
        if (bsbit == NULL)
        {
            return -1;
        }
    }

    return 1;
}

static void
cmd_bit_help (void)
{
    printf (_("Usage: %s NUMBER TYPE DEFAULT SIGNAL [CBIT CVAL CSTATE]\n"
              "Define new BSR (Boundary Scan Register) bit for SIGNAL, with\n"
              "DEFAULT value.\n"
              "\n"
              "NUMBER        Bit number in the BSR\n"
              "TYPE          Bit type, valid values are I, O, B, C, and X\n"
              "DEFAULT       Default (safe) bit value, valid values are 1, 0, ?\n"
              "SIGNAL        Associated signal name\n"
              "CBIT          Control bit number\n"
              "CVAL          Control value\n"
              "CSTATE        Control state, valid state is only Z\n"), "bit");
}

urj_cmd_t urj_cmd_bit = {
    "bit",
    N_("define new BSR bit"),
    cmd_bit_help,
    cmd_bit_run
};
