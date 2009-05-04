/*
 * $Id$
 *
 * Copyright (C) 2003 Marcel Telka
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

#include "urjtag/chain.h"
#include "urjtag/part.h"
#include "urjtag/bssignal.h"

#include "urjtag/cmd.h"

#include "cmd.h"

static int
cmd_salias_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    urj_part_signal_t *s;
    urj_part_salias_t *sa;

    if (urj_cmd_params (params) != 3)
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return 1;

    if (urj_part_find_signal (part, params[1]) != NULL)
    {
        printf (_("Signal '%s' already defined\n"), params[1]);
        return 1;
    }

    s = urj_part_find_signal (part, params[2]);
    if (s == NULL)
    {
        printf (_("Signal '%s' not found\n"), params[2]);
        return 1;
    }

    sa = urj_part_salias_alloc (params[1], s);
    if (!sa)
    {
        printf (_("out of memory\n"));
        return 1;
    }

    sa->next = part->saliases;
    part->saliases = sa;

    return 1;
}

static void
cmd_salias_help (void)
{
    printf (_("Usage: %s ALIAS SIGNAL\n"
              "Define new signal ALIAS as alias for existing SIGNAL.\n"
              "\n"
              "ALIAS         New signal alias name\n"
              "SIGNAL        Existing signal name\n"), "signal");
}

const urj_cmd_t urj_cmd_salias = {
    "salias",
    N_("define an alias for a signal"),
    cmd_salias_help,
    cmd_salias_run
};
