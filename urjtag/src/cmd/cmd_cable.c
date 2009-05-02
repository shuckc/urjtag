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

#include <urjtag/parport.h>
#include <urjtag/tap.h>
#include <urjtag/cable.h>
#include <urjtag/chain.h>
#include <urjtag/bus.h>

#include <urjtag/cmd.h>

static int
cmd_cable_run (urj_chain_t *chain, char *params[])
{
    urj_cable_t *cable;
    int i;
    int paramc = urj_cmd_params (params);

    /* we need at least one parameter for 'cable' command */
    if (paramc < 2)
        return -1;

    /* maybe old syntax was used?  search connection type driver */
    for (i = 0; urj_tap_parport_drivers[i]; i++)
        if (strcasecmp (params[1], urj_tap_parport_drivers[i]->type) == 0)
            break;

    if (urj_tap_parport_drivers[i] != 0)
    {
        /* Old syntax was used. Swap params. */
        printf (_
                ("Note: the 'cable' command syntax changed, please read the help text\n"));
        if (paramc >= 4)
        {
            char *tmparam;
            tmparam = params[3];
            params[3] = params[2];
            params[2] = params[1];
            params[1] = tmparam;
        }
        else
            return -1;
    }

    /* search cable driver list */
    for (i = 0; urj_tap_cable_drivers[i]; i++)
        if (strcasecmp (params[1], urj_tap_cable_drivers[i]->name) == 0)
            break;
    if (!urj_tap_cable_drivers[i])
    {
        printf (_("Unknown cable type: %s\n"), params[1]);
        return 1;
    }

    if (paramc >= 3)
    {
        if (strcasecmp (params[2], "help") == 0)
        {
            urj_tap_cable_drivers[i]->help (urj_tap_cable_drivers[i]->name);
            return 1;
        }
    }

    if (urj_bus)
    {
        URJ_BUS_FREE (urj_bus);
        urj_bus = NULL;
    }

    urj_tap_chain_disconnect (chain);

    cable = calloc (1, sizeof (urj_cable_t));
    if (!cable)
    {
        printf (_("%s(%d) calloc failed!\n"), __FILE__, __LINE__);
        return 1;
    }

    cable->driver = urj_tap_cable_drivers[i];

    if (cable->driver->connect (++params, cable))
    {
        printf (_("Error: Cable connection failed!\n"));
        free (cable);
        return 1;
    }

    chain->cable = cable;

    if (urj_tap_cable_init (chain->cable))
    {
        printf (_("Error: Cable initialization failed!\n"));
        urj_tap_chain_disconnect (chain);
        return 1;
    }
    urj_tap_chain_set_trst (chain, 0);
    urj_tap_chain_set_trst (chain, 1);
    urj_tap_reset (chain);

    return 1;
}

static void
cmd_cable_help (void)
{
    int i;

    printf (_("Usage: %s DRIVER [DRIVER_OPTS]\n"
              "Select JTAG cable type.\n"
              "\n"
              "DRIVER      name of cable\n"
              "DRIVER_OPTS options for the selected cable\n"
              "\n"
              "Type \"cable DRIVER help\" for info about options for cable DRIVER.\n"
              "\n" "List of supported cables:\n"), "cable");

    for (i = 0; urj_tap_cable_drivers[i]; i++)
        printf (_("%-13s %s\n"), urj_tap_cable_drivers[i]->name,
                _(urj_tap_cable_drivers[i]->description));
}

urj_cmd_t urj_cmd_cable = {
    "cable",
    N_("select JTAG cable"),
    cmd_cable_help,
    cmd_cable_run
};
