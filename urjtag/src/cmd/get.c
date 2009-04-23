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

#include "part.h"
#include "bssignal.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_get_run (urj_chain_t *chain, char *params[])
{
    int data;
    urj_part_signal_t *s;

    if (urj_cmd_params (params) != 3)
        return -1;

    if (strcasecmp (params[1], "signal") != 0)
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
        printf (_("%s: no active part\n"), "get");
        return 1;
    }

    s = urj_part_find_signal (chain->parts->parts[chain->active_part],
                              params[2]);
    if (!s)
    {
        printf (_("signal '%s' not found\n"), params[2]);
        return 1;
    }
    data = urj_part_get_signal (chain->parts->parts[chain->active_part], s);
    if (data != -1)
        printf (_("%s = %d\n"), params[2], data);

    return 1;
}

static void
cmd_get_help (void)
{
    printf (_("Usage: %s SIGNAL\n"
              "Get signal state from output BSR (Boundary Scan Register).\n"
              "\n"
              "SIGNAL        signal name (from JTAG declaration file)\n"),
            "get signal");
}

urj_cmd_t cmd_get = {
    "get",
    N_("get external signal value"),
    cmd_get_help,
    cmd_get_run
};
