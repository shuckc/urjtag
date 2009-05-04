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
#include <urjtag/bus.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_initbus_run (urj_chain_t *chain, char *params[])
{
    int i;

    if (urj_cmd_params (params) < 2)
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    if (urj_tap_chain_active_part (chain) == NULL)
        return 1;

    for (i = 0; urj_bus_drivers[i] != NULL; i++)
    {
        if (strcasecmp (urj_bus_drivers[i]->name, params[1]) == 0)
        {
            urj_bus_t *abus = urj_bus_drivers[i]->new_bus (chain,
                                                           urj_bus_drivers[i],
                                                           params);
            if (abus == NULL)
            {
                printf (_("bus alloc/attach failed!\n"));
                return 1;
            }
            urj_bus_buses_add (abus);
            if (URJ_BUS_INIT (abus) != URJ_STATUS_OK)
                printf (_("bus initialization failed!\n"));

            for (i = 0; i < urj_buses.len; i++)
                if (urj_buses.buses[i] == urj_bus)
                    break;
            if (i != urj_buses.len - 1)
                printf (_("Initialized bus %d, active bus %d\n"),
                        urj_buses.len - 1, i);

            return 1;
        }
    }

    printf (_("Unknown bus: %s\n"), params[1]);

    return 1;
}

static void
cmd_initbus_help (void)
{
    int i;

    printf (_("Usage: %s BUSNAME\n"
              "Initialize new bus driver for active part.\n"
              "\n"
              "BUSNAME       Name of the bus\n"
              "\n" "List of available buses:\n"), "initbus");

    for (i = 0; urj_bus_drivers[i] != NULL; i++)
        printf (_("%-10s %s\n"), urj_bus_drivers[i]->name,
                urj_bus_drivers[i]->description);
}

const const urj_cmd_t urj_cmd_initbus = {
    "initbus",
    N_("initialize bus driver for active part"),
    cmd_initbus_help,
    cmd_initbus_run
};
