/*
 * $Id$
 *
 * Copyright (C) 2008 K. Waschk
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
 * Written by Kolja Waschk, 2008
 *   based on idea and code by Sebastian Hesselbarth, 2008
 *   and code by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jtag.h"
#include <cable.h>

#include "cmd.h"

static int
cmd_pod_run (chain_t *chain, char *params[])
{
    int i, j;
    int mask = 0;
    int val = 0;

    if ((i = cmd_params (params)) < 2)
        return -1;

    if (!cmd_test_cable (chain))
        return -1;

    for (j = 1; j < i; j++)
    {
        char *eq = strrchr (params[j], '=');
        if (!eq)
            return -1;
        pod_sigsel_t it = CS_NONE;
        int n = strlen (params[j]);
        if (n > 4 && (strncasecmp (params[j], "tck", 3) == 0))
            it = CS_TCK;
        else if (n > 4 && (strncasecmp (params[j], "tms", 3) == 0))
            it = CS_TMS;
        else if (n > 4 && (strncasecmp (params[j], "tdi", 3) == 0))
            it = CS_TDI;
        else if (n > 5 && (strncasecmp (params[j], "trst", 3) == 0))
            it = CS_TRST;
        else if (n > 6 && (strncasecmp (params[j], "reset", 3) == 0))
            it = CS_RESET;
        if (it == CS_NONE)
            return -1;
        mask |= it;
        if (atoi (eq + 1) != 0)
            val |= it;
    }

    chain_set_pod_signal (chain, mask, val);

    return 1;
}

static void
cmd_pod_help (void)
{
    printf (_("Usage: %s SIGNAL=# [SIGNAL=# ...]\n"
              "Set state of POD signal(s) to 0 or 1.\n"
              "\n"
              "SIGNAL	    TCK,TMS, TDI, TRST, or RESET\n"
              "#          0 or 1\n"), "pod");
}

cmd_t cmd_pod = {
    "pod",
    N_("Set state of POD signal(s)"),
    cmd_pod_help,
    cmd_pod_run
};
