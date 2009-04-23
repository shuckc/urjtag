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

#include "jtag.h"

#include "cmd.h"

static int
cmd_bus_run (urj_chain_t *chain, char *params[])
{
    unsigned int n;

    if (urj_cmd_params (params) != 2)
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    if (!chain->parts)
    {
        printf (_("Run \"detect\" first.\n"));
        return 1;
    }

    if (urj_cmd_get_number (params[1], &n))
        return -1;

    if (n >= urj_buses.len)
    {
        printf (_("%s: invalid bus number\n"), "bus");
        return 1;
    }

    urj_bus = urj_buses.buses[n];

    return 1;
}

static void
cmd_bus_help (void)
{
    printf (_("Usage: %s BUS\n"
              "Change active bus.\n"
              "\n" "BUS           bus number\n"), "bus");
}

urj_cmd_t urj_cmd_bus = {
    "bus",
    N_("change active bus"),
    cmd_bus_help,
    cmd_bus_run
};
