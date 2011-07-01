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
#include <string.h>
#include <stdlib.h>

#include <urjtag/error.h>
#include <urjtag/parport.h>
#include <urjtag/tap.h>
#include <urjtag/cable.h>
#include <urjtag/chain.h>
#include <urjtag/bus.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cable_probe (char *params[])
{
    return urj_tap_cable_usb_probe (params);
}

static int
cmd_cable_run (urj_chain_t *chain, char *params[])
{
    int i;
    int paramc = urj_cmd_params (params);

    /* we need at least one parameter for 'cable' command */
    if (paramc < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, paramc);
        return URJ_STATUS_FAIL;
    }

    if (strcasecmp (params[1], "probe") == 0 && cable_probe (params))
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("%s: automatic probe found nothing"),
                       params[0]);
        return URJ_STATUS_FAIL;
    }

    /* maybe old syntax was used?  search connection type driver */
    for (i = 0; urj_tap_parport_drivers[i]; i++)
        if (strcasecmp (params[1],
                        urj_cable_parport_devtype_string(urj_tap_parport_drivers[i]->type)) == 0)
            break;

    if (urj_tap_parport_drivers[i] != 0)
    {
        /* Old syntax was used. Swap params. */
        urj_warning ("Note: the 'cable' command syntax changed, please read the help text\n");
        if (paramc >= 4)
        {
            char *tmparam;
            tmparam = params[3];
            params[3] = params[2];
            params[2] = params[1];
            params[1] = tmparam;
        }
        else
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           "old syntax requires >= %d params, not %d",
                           4, paramc);
            return URJ_STATUS_FAIL;
        }
    }

    if (paramc >= 3 && strcasecmp (params[2], "help") == 0)
    {
        const urj_cable_driver_t *driver;

        driver = urj_tap_cable_find (params[1]);
        if (!driver)
        {
            urj_error_set (URJ_ERROR_INVALID,
                           "unknown cable driver '%s'", params[1]);
            return URJ_STATUS_FAIL;
        }

        driver->help (URJ_LOG_LEVEL_NORMAL, driver->name);
        return URJ_STATUS_OK;
    }

    return urj_tap_chain_connect (chain, params[1], &params[2]);
}

static void
cmd_cable_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s DRIVER [DRIVER_OPTS]\n"
               "Select JTAG cable type.\n"
               "\n"
               "DRIVER      name of cable\n"
               "DRIVER_OPTS options for the selected cable\n"
               "\n"
               "Type \"cable DRIVER help\" for info about options for cable DRIVER.\n"
               "You can also use the driver \"probe\" to attempt autodetection.\n"
               "\n" "List of supported cables:\n"),
             "cable");

    urj_cmd_show_list (urj_tap_cable_drivers);
}

static void
cmd_cable_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                    char * const *tokens, const char *text, size_t text_len,
                    size_t token_point)
{
    size_t i;

    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "probe");

        for (i = 0; urj_tap_cable_drivers[i]; i++)
            urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                             urj_tap_cable_drivers[i]->name);
        break;
    case 2:
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "help");
        urj_completion_mayben_add_param_list (matches, match_cnt, text,
                                              text_len, urj_cable_param_list);
        break;
    }
}

const urj_cmd_t urj_cmd_cable = {
    "cable",
    N_("select JTAG cable"),
    cmd_cable_help,
    cmd_cable_run,
    cmd_cable_complete,
};
