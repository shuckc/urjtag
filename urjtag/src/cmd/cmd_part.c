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
#include <stdlib.h>

#include <urjtag/chain.h>
#include <urjtag/part.h>

#include <urjtag/cmd.h>

static int
cmd_part_run (urj_chain_t *chain, char *params[])
{
    unsigned int n;

/* part alias U1 (3 params) */
    if (urj_cmd_params (params) == 3)
    {
        if (strcasecmp (params[1], "alias") == 0)
        {
            urj_part_t *part = urj_tap_chain_active_part (chain);

            if (part == NULL)
                return 1;

            part->alias = malloc (strlen (params[2]) + 1);
            if (part->alias == NULL)
            {
                urj_error_set(URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                              strlen (params[2]) + 1);
                return -1;
            }

            strcpy (part->alias, params[2]);
            return 1;
        }
    }


    if (urj_cmd_params (params) != 2)
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    if (!chain->parts)
    {
        printf (_("Run \"detect\" first.\n"));
        return 1;
    }

/* Search for alias too djf */
    if (urj_cmd_get_number (params[1], &n))
    {

        /* Search all parts to check their aliases */
        int i;
        char *a;

        for (i = 0; i < chain->parts->len; i++)
        {
            a = chain->parts->parts[i]->alias;
            if (a && strcasecmp (a, params[1]) == 0)
                break;
        }
        if (i < chain->parts->len)
            n = i;


        else
            return -1;
    }

    if (n >= chain->parts->len)
    {
        printf (_("%s: invalid part number\n"), "part");
        return 1;
    }

    chain->active_part = n;

    return 1;
}

static void
cmd_part_help (void)
{
    printf (_("Usage: %s bus->part\n"
              "Change active part for current JTAG chain.\n"
              "\n" "bus->part          part number | alias\n"), "part");
}

urj_cmd_t urj_cmd_part = {
    "part",
    N_("change active part for current JTAG chain"),
    cmd_part_help,
    cmd_part_run
};
