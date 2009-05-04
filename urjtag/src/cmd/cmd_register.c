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
#include <stdlib.h>
#include <string.h>

#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/data_register.h>
#include <urjtag/tap_register.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_register_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    unsigned int len;
    urj_data_register_t *dr;

    if (urj_cmd_params (params) != 3)
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return 1;

    if (urj_cmd_get_number (params[2], &len))
        return -1;

    if (urj_part_find_data_register (part, params[1]) != NULL)
    {
        printf (_("Data register '%s' already defined\n"), params[1]);
        return 1;
    }

    dr = urj_part_data_register_alloc (params[1], len);
    if (!dr)
    {
        printf (_("out of memory\n"));
        return 1;
    }

    dr->next = part->data_registers;
    part->data_registers = dr;

    /* Boundary Scan Register */
    if (strcasecmp (dr->name, "BSR") == 0)
    {
        int i;

        part->boundary_length = len;
        part->bsbits = malloc (part->boundary_length * sizeof *part->bsbits);
        if (!part->bsbits)
        {
            printf (_("out of memory\n"));
            return 1;
        }
        for (i = 0; i < part->boundary_length; i++)
            part->bsbits[i] = NULL;
    }

    /* Device Identification Register */
    if (strcasecmp (dr->name, "DIR") == 0)
        urj_tap_register_init (dr->out,
                               urj_tap_register_get_string (part->id));

    return 1;
}

static void
cmd_register_help (void)
{
    printf (_("Usage: %s NAME LENGTH\n"
              "Define new data register with specified NAME and LENGTH.\n"
              "\n"
              "NAME          Data register name\n"
              "LENGTH        Data register length\n"), "register");
}

const urj_cmd_t urj_cmd_register = {
    "register",
    N_("define new data register for a part"),
    cmd_register_help,
    cmd_register_run
};
