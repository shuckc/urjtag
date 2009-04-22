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
#include <stdlib.h>
#include <string.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_signal_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    urj_part_signal_t *s;
    int i;

    if ((i = urj_cmd_params (params)) < 2)
        return -1;


    if (!urj_cmd_test_cable (chain))
        return 1;

    if (!chain->parts)
    {
        printf (_("Run \"detect\" first.\n"));
        return 1;
    }

    if (chain->active_part >= chain->parts->len)
    {
        printf (_("%s: no active part\n"), "signal");
        return 1;
    }

    part = chain->parts->parts[chain->active_part];
    if ((s = urj_part_find_signal (part, params[1])) != NULL)
    {
        if (i == 3)
        {
            printf ("Defining pin for signal %s\n", s->name);

            if (s->pin)
                free (s->pin);  /* erase old */

            /* Allocate the space for the pin number & copy it */
            s->pin = malloc (strlen (params[2]) + 1);
            strcpy (s->pin, params[2]);

            return 1;
        }
        else
        {
            printf (_("Signal '%s' already defined\n"), params[1]);
            return 1;
        }
    }

    s = urj_part_signal_alloc (params[1]);

    if (i == 3)
    {                           /* Add pin number */
        /* Allocate the space for the pin number & copy it */
        s->pin = malloc (strlen (params[2]) + 1);
        strcpy (s->pin, params[2]);

    }

    if (!s)
    {
        printf (_("out of memory\n"));
        return 1;
    }

    s->next = part->signals;
    part->signals = s;

    return 1;
}

static void
cmd_signal_help (void)
{
    printf (_("Usage: %s SIGNAL [PIN#]\n"
              "Define new signal with name SIGNAL for a part.\n"
              "\n"
              "SIGNAL		New signal name\n"
              "PIN#   	List of pin # for a signal\n"), "signal");
}

urj_cmd_t cmd_signal = {
    "signal",
    N_("define new signal for a part"),
    cmd_signal_help,
    cmd_signal_run
};
