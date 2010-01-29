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
#include <urjtag/params.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_initbus_run (urj_chain_t *chain, char *params[])
{
    int drv, i;
    const urj_param_t **bus_params;

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

    for (drv = 0; urj_bus_drivers[drv] != NULL; drv++)
        if (strcasecmp (urj_bus_drivers[drv]->name, params[1]) == 0)
            break;

    if (urj_bus_drivers[drv] == NULL)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("Unknown bus: %s"), params[1]);
        return URJ_STATUS_FAIL;
    }

    urj_param_init (&bus_params);
    for (i = 2; params[i] != NULL; i++)
        if (urj_param_push (&urj_bus_param_list, &bus_params,
                            params[i]) != URJ_STATUS_OK)
        {
            urj_param_clear (&bus_params);
            return URJ_STATUS_FAIL;
        }

    if (urj_bus_init_bus(chain, urj_bus_drivers[drv], bus_params) == NULL)
    {
        urj_param_clear (&bus_params);
        return URJ_STATUS_FAIL;
    }

    urj_param_clear (&bus_params);

    return URJ_STATUS_OK;
}

static void
cmd_initbus_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s BUSNAME\n"
               "Initialize new bus driver for active part.\n"
               "\n"
               "BUSNAME       Name of the bus\n"
               "\n" "List of available buses:\n"),
             "initbus");

    urj_cmd_show_list (urj_bus_drivers);
}

const const urj_cmd_t urj_cmd_initbus = {
    "initbus",
    N_("initialize bus driver for active part"),
    cmd_initbus_help,
    cmd_initbus_run
};
