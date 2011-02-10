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

#include <urjtag/cable.h>
#include <urjtag/chain.h>
#include "generic.h"
#include "generic_usbconn.h"

#include <urjtag/cmd.h>

static const urj_usbconn_cable_t * const urj_tap_cable_usbconn_cables[] = {
#define _URJ_USB(usb) &urj_tap_cable_usbconn_##usb,
#include "generic_usbconn_list.h"
    NULL
};

int
urj_tap_cable_generic_usbconn_connect (urj_cable_t *cable,
                                       const urj_param_t *params[])
{
    urj_usbconn_cable_t user_specified = {
        NULL,                   /* no name */
        NULL,                   /* no string pattern */
        NULL,                   /* no specific driver */
        -1,                     /* no VID */
        -1,                     /* no PID */
        0,                      /* default interface */
    };

    urj_tap_cable_generic_params_t *cable_params;
    urj_usbconn_t *conn = NULL;
    int i;

    if (strcasecmp (cable->driver->name, "usb") != 0)
    {
        user_specified.name = cable->driver->name;
    }

    if (params != NULL)
        /* parse arguments beyond the cable name */
        for (i = 0; params[i] != NULL; i++)
        {
            switch (params[i]->key)
            {
            case URJ_CABLE_PARAM_KEY_PID:
                user_specified.pid = params[i]->value.lu;
                break;
            case URJ_CABLE_PARAM_KEY_VID:
                user_specified.vid = params[i]->value.lu;
                break;
            case URJ_CABLE_PARAM_KEY_DESC:
                user_specified.desc = params[i]->value.string;
                break;
            case URJ_CABLE_PARAM_KEY_DRIVER:
                user_specified.driver = params[i]->value.string;
                break;
            case URJ_CABLE_PARAM_KEY_INTERFACE:
                user_specified.interface = params[i]->value.lu;
                break;
            default:
                // hand these to the driver connect()
                break;
            }
        }

    /* search usbconn driver list */
    for (i = 0; urj_tap_usbconn_drivers[i] && !conn; i++)
    {
        if ((user_specified.driver == NULL)
            || (strcasecmp (user_specified.driver,
                            urj_tap_usbconn_drivers[i]->type) == 0))
        {
            int j;

            /* search cable list */
            for (j = 0; urj_tap_cable_usbconn_cables[j] && !conn; j++)
            {
                if ((user_specified.name == NULL)
                    || (strcasecmp (user_specified.name,
                                    urj_tap_cable_usbconn_cables[j]->name) == 0))
                {
                    if (strcasecmp (urj_tap_cable_usbconn_cables[j]->driver,
                                    urj_tap_usbconn_drivers[i]->type) == 0)
                    {
                        urj_usbconn_cable_t cable_try =
                            *(urj_tap_cable_usbconn_cables[j]);

                        if (user_specified.vid >= 0)
                            cable_try.vid = user_specified.vid;
                        if (user_specified.pid >= 0)
                            cable_try.pid = user_specified.pid;
                        if (user_specified.desc != 0)
                            cable_try.desc = user_specified.desc;
                        if (user_specified.interface != 0)
                            cable_try.interface = user_specified.interface;

                        conn = urj_tap_usbconn_drivers[i]->connect (&cable_try,
                                                                    params);
                    }
                }
            }
        }
    }

    if (!conn)
    {
        // @@@@ RFHH make this into either the error from drivers->connect,
        // or urj_error_set (NOT_FOUND)
        urj_log (URJ_LOG_LEVEL_ERROR,
                 _("Couldn't connect to suitable USB device.\n"));
        return URJ_STATUS_FAIL;
    }

    cable_params = malloc (sizeof (urj_tap_cable_generic_params_t));
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (urj_tap_cable_generic_params_t));
        urj_tap_usbconn_drivers[i]->free (conn);
        return URJ_STATUS_FAIL;
    }

    cable->link.usb = conn;
    cable->params = cable_params;
    cable->chain = NULL;

    return URJ_STATUS_OK;
}

void
urj_tap_cable_generic_usbconn_free (urj_cable_t *cable)
{
    cable->link.usb->driver->free (cable->link.usb);
    free (cable->params);
    free (cable);
}

void
urj_tap_cable_generic_usbconn_done (urj_cable_t *cable)
{
    urj_tap_usbconn_close (cable->link.usb);
}

void
urj_tap_cable_generic_usbconn_help_ex (urj_log_level_t ll, const char *cablename,
                                       const char *ex_short, const char *ex_desc)
{
    int i;
    const urj_usbconn_cable_t *conn;

    for (i = 0; urj_tap_cable_usbconn_cables[i]; ++i)
    {
        conn = urj_tap_cable_usbconn_cables[i];
        if (strcasecmp (conn->name, cablename) == 0)
            break;
    }
    if (!urj_tap_cable_usbconn_cables[i])
    {
        urj_warning (_("Unable to locate cable %s"), cablename);
        return;
    }

    urj_log (ll,
             _("Usage: cable %s %s %s\n"
               "\n" "%s%s"
               "\n"
               "Default:   vid=%x pid=%x driver=%s\n"
               "\n"),
             cablename,
             URJ_TAP_CABLE_GENERIC_USBCONN_HELP_SHORT, ex_short,
             URJ_TAP_CABLE_GENERIC_USBCONN_HELP_DESC, ex_desc,
             conn->vid, conn->pid, conn->driver);
}

void
urj_tap_cable_generic_usbconn_help (urj_log_level_t ll, const char *cablename)
{
    urj_tap_cable_generic_usbconn_help_ex (ll, cablename, "", "");
}

int
urj_tap_cable_usb_probe (char *params[])
{
    int i,j;
    urj_usbconn_t *conn;

    urj_log_level_t old_level = urj_log_state.level;
    urj_log_state.level = URJ_LOG_LEVEL_SILENT;

    for (i = 0; urj_tap_usbconn_drivers[i]; ++i)
    {
        for (j = 0; urj_tap_cable_usbconn_cables[j]; ++j)
        {
            urj_usbconn_cable_t cable_try = *(urj_tap_cable_usbconn_cables[j]);
            conn = urj_tap_usbconn_drivers[i]->connect (&cable_try, NULL);
            if (conn)
            {
                urj_log_state.level = old_level;
                params[1] = (char *)urj_tap_cable_usbconn_cables[j]->name;
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         _("Found USB cable: %s\n"), params[1]);
                return URJ_STATUS_OK;
            }
        }
    }

    urj_log_state.level = old_level;
    return URJ_STATUS_FAIL;
}
