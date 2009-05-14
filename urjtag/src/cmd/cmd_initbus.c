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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
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
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (urj_tap_chain_active_part (chain) == NULL)
        return URJ_STATUS_FAIL;

    for (i = 0; urj_bus_drivers[i] != NULL; i++)
    {
        if (strcasecmp (urj_bus_drivers[i]->name, params[1]) == 0)
        {
            urj_bus_t *abus = urj_bus_drivers[i]->new_bus (chain,
                                                           urj_bus_drivers[i],
                                                           params);
            if (abus == NULL)
            {
                // @@@@ RFHH need to sanitize the bus module
                urj_error_set (URJ_ERROR_BUS, _("bus alloc/attach failed"));
                return URJ_STATUS_FAIL;
            }
            urj_bus_buses_add (abus);
            // @@@@ RFHH need to bail out on error ?
            if (URJ_BUS_INIT (abus) != URJ_STATUS_OK)
                printf (_("bus initialization failed!\n"));

            for (i = 0; i < urj_buses.len; i++)
                if (urj_buses.buses[i] == urj_bus)
                    break;
            // @@@@ RFHH no need to handle the case of bus not found ?
            if (i != urj_buses.len - 1)
                printf (_("Initialized bus %d, active bus %d\n"),
                        urj_buses.len - 1, i);

            return URJ_STATUS_OK;
        }
    }

    urj_error_set (URJ_ERROR_NOTFOUND, _("Unknown bus: %s"), params[1]);
    return URJ_STATUS_FAIL;
}

static void
cmd_initbus_help (void)
{
    int i;

    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s BUSNAME\n"
               "Initialize new bus driver for active part.\n"
               "\n"
               "BUSNAME       Name of the bus\n"
               "\n" "List of available buses:\n"),
             "initbus");

    for (i = 0; urj_bus_drivers[i] != NULL; i++)
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("%-10s %s\n"), urj_bus_drivers[i]->name,
                 urj_bus_drivers[i]->description);
}

const const urj_cmd_t urj_cmd_initbus = {
    "initbus",
    N_("initialize bus driver for active part"),
    cmd_initbus_help,
    cmd_initbus_run
};
