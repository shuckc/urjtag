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

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/bsbit.h>
#include <urjtag/part.h>
#include <urjtag/chain.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static void
cmd_bit_print_params (char *params[], unsigned int parameters, char *command,
                      size_t command_size)
{
    unsigned int i;

    command[0] = '\0';
    command_size -= 1;
    strncat (command, params[0], command_size);
    command_size -= strlen (params[0]);
    for (i = 1; i < parameters; i++)
    {
        strncat (command, " ", command_size);
        command_size -= 1;
        strncat (command, params[i], command_size);
        command_size -= strlen (params[i]);
    }
}

static int
cmd_bit_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    long unsigned bit;
    int type;
    int safe;
    long unsigned control;
    unsigned int parameters = urj_cmd_params (params);
    char command[1024];

    cmd_bit_print_params (params, parameters, command, sizeof command);

    if ((parameters != 5) && (parameters != 8))
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #params should be 5 or 8, not %d",
                       "bus", parameters);
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    /* bit number */
    if (urj_cmd_get_number (params[1], &bit) != URJ_STATUS_OK)
    {
        // error state already set
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("%s: unable to get boundary bit number for command '%s'"),
                 "bus", command);
        return URJ_STATUS_FAIL;
    }

    /* bit type */
    if (strlen (params[2]) != 1)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: invalid bit type length for command '%s'"),
                      "bus", command);
        return URJ_STATUS_FAIL;
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
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: invalid bit type for command '%s'"),
                       "bus", command);
        return URJ_STATUS_FAIL;
    }

    /* default (safe) value */
    if (strlen (params[3]) != 1)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: invalid default value length for command '%s'"),
                       "bus", command);
        return URJ_STATUS_FAIL;
    }
    switch (params[3][0])
    {
    case '0':
    case '1':
        safe = params[3][0] - '0';
        break;
    case '?':
        safe = URJ_BSBIT_DONTCARE;
        break;
    default:
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: invalid default value '%s' for command '%s'"),
                       "bus", params[3], command);
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_params (params) == 5)
        // without control bit
        return urj_part_bsbit_alloc (part, bit, params[4], type, safe);

    /* with control bit */

    int control_value;
    int control_state;

    /* control bit number */
    if (urj_cmd_get_number (params[5], &control) != URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("%s: unable to get control bit number for command '%s'"),
                 "bit", command);
        return URJ_STATUS_FAIL;
    }
    /* control value */
    if (strlen (params[6]) != 1)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: invalid control value length for command '%s'"),
                       "bit", command);
        return URJ_STATUS_FAIL;
    }
    control_value = (params[6][0] == '1');

    /* control state */
    if (strcasecmp (params[7], "Z"))
    {
        urj_error_set (URJ_ERROR_SYNTAX, "control state '%s' must be 'Z'",
                       params[7]);
        return URJ_STATUS_FAIL;
    }

    control_state = URJ_BSBIT_STATE_Z;

    return urj_part_bsbit_alloc_control (part, bit, params[4], type, safe,
                                         control, control_value,
                                         control_state);
}

static void
cmd_bit_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s NUMBER TYPE DEFAULT SIGNAL [CBIT CVAL CSTATE]\n"
               "Define new BSR (Boundary Scan Register) bit for SIGNAL, with\n"
               "DEFAULT value.\n"
               "\n"
               "NUMBER        Bit number in the BSR\n"
               "TYPE          Bit type, valid values are I, O, B, C, and X\n"
               "DEFAULT       Default (safe) bit value, valid values are 1, 0, ?\n"
               "SIGNAL        Associated signal name\n"
               "CBIT          Control bit number\n"
               "CVAL          Control value\n"
               "CSTATE        Control state, valid state is only Z\n"),
             "bit");
}

const urj_cmd_t urj_cmd_bit = {
    "bit",
    N_("define new BSR bit"),
    cmd_bit_help,
    cmd_bit_run
};
