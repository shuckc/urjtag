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

/* @@@@ RFHH put these in a .h file */
#ifdef ENABLE_CABLE_XPC
extern urj_usbconn_cable_t urj_tap_cable_usbconn_xpc_int;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_xpc_ext;
#endif
#ifdef ENABLE_CABLE_JLINK
extern urj_usbconn_cable_t urj_tap_cable_usbconn_jlink;
#endif
#ifdef ENABLE_CABLE_FT2232
#ifdef ENABLE_LOWLEVEL_FTD2XX
extern urj_usbconn_cable_t urj_tap_cable_usbconn_ft2232_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_armusbocd_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_armusbocdtiny_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_gnice_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_jtagkey_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_oocdlinks_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_turtelizer2_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usbtojtagif_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_signalyzer_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_flyswatter_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usbscarab2_ftd2xx;
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
extern urj_usbconn_cable_t urj_tap_cable_usbconn_ft2232_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_armusbocd_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_armusbocdtiny_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_gnice_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_jtagkey_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_oocdlinks_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_turtelizer2_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usbtojtagif_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_signalyzer_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_flyswatter_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usbscarab2_ftdi;
#endif
#endif
#ifdef ENABLE_CABLE_USBBLASTER
#ifdef ENABLE_LOWLEVEL_FTD2XX
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usbblaster_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_cubic_cyclonium_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_nios_eval_ftd2xx;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usb_jtag_ftd2xx;
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usbblaster_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_cubic_cyclonium_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_nios_eval_ftdi;
extern urj_usbconn_cable_t urj_tap_cable_usbconn_usb_jtag_ftdi;
#endif
#endif

urj_usbconn_cable_t *urj_tap_cable_usbconn_cables[] = {
#ifdef ENABLE_CABLE_XPC
    &urj_tap_cable_usbconn_xpc_int,
    &urj_tap_cable_usbconn_xpc_ext,
#endif
#ifdef ENABLE_CABLE_JLINK
    &urj_tap_cable_usbconn_jlink,
#endif
#ifdef ENABLE_CABLE_FT2232
#ifdef ENABLE_LOWLEVEL_FTD2XX
    &urj_tap_cable_usbconn_ft2232_ftd2xx,
    &urj_tap_cable_usbconn_armusbocd_ftd2xx,
    &urj_tap_cable_usbconn_armusbocdtiny_ftd2xx,
    &urj_tap_cable_usbconn_gnice_ftd2xx,
    &urj_tap_cable_usbconn_jtagkey_ftd2xx,
    &urj_tap_cable_usbconn_oocdlinks_ftd2xx,
    &urj_tap_cable_usbconn_turtelizer2_ftd2xx,
    &urj_tap_cable_usbconn_usbtojtagif_ftd2xx,
    &urj_tap_cable_usbconn_signalyzer_ftd2xx,
    &urj_tap_cable_usbconn_flyswatter_ftd2xx,
    &urj_tap_cable_usbconn_usbscarab2_ftd2xx,
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
    &urj_tap_cable_usbconn_ft2232_ftdi,
    &urj_tap_cable_usbconn_armusbocd_ftdi,
    &urj_tap_cable_usbconn_armusbocdtiny_ftdi,
    &urj_tap_cable_usbconn_gnice_ftdi,
    &urj_tap_cable_usbconn_jtagkey_ftdi,
    &urj_tap_cable_usbconn_oocdlinks_ftdi,
    &urj_tap_cable_usbconn_turtelizer2_ftdi,
    &urj_tap_cable_usbconn_usbtojtagif_ftdi,
    &urj_tap_cable_usbconn_signalyzer_ftdi,
    &urj_tap_cable_usbconn_flyswatter_ftdi,
    &urj_tap_cable_usbconn_usbscarab2_ftdi,
#endif
#endif
#ifdef ENABLE_CABLE_USBBLASTER
#ifdef ENABLE_LOWLEVEL_FTD2XX
    &urj_tap_cable_usbconn_usbblaster_ftd2xx,
    &urj_tap_cable_usbconn_cubic_cyclonium_ftd2xx,
    &urj_tap_cable_usbconn_nios_eval_ftd2xx,
    &urj_tap_cable_usbconn_usb_jtag_ftd2xx,
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
    &urj_tap_cable_usbconn_usbblaster_ftdi,
    &urj_tap_cable_usbconn_cubic_cyclonium_ftdi,
    &urj_tap_cable_usbconn_nios_eval_ftdi,
    &urj_tap_cable_usbconn_usb_jtag_ftdi,
#endif
#endif
    NULL
};

int
urj_tap_cable_generic_usbconn_connect (char *params[], urj_cable_t *cable)
{
    urj_usbconn_cable_t user_specified = {
        NULL,                   /* no name */
        NULL,                   /* no string pattern */
        NULL,                   /* no specific driver */
        -1,                     /* no VID */
        -1,                     /* no PID */
    };

    int paramc = urj_cmd_params (params);
    urj_tap_cable_generic_params_t *cable_params;
    urj_usbconn_t *conn = NULL;
    int i;

    if (strcasecmp (params[0], "usb") != 0)
    {
        user_specified.name = params[0];
    }

    /* parse arguments beyond the cable name */
    for (i = 1; i < paramc; i++)
    {
        if (strncasecmp ("pid=", params[i], 4) == 0)
        {
            user_specified.pid = strtol (params[i] + 4, NULL, 16);
        }
        else if (strncasecmp ("vid=", params[i], 4) == 0)
        {
            user_specified.vid = strtol (params[i] + 4, NULL, 16);
        }
        else if (strncasecmp ("desc=", params[i], 5) == 0)
        {
            user_specified.desc = params[i] + 5;
        }
        else if (strncasecmp ("driver=", params[i], 7) == 0)
        {
            user_specified.driver = params[i] + 7;
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

                        // @@@@ RFHH bail out on failure?
                        conn =
                            urj_tap_usbconn_drivers[i]->
                            connect ((const char **) &params[1], paramc - 1,
                                     &cable_try);
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
urj_tap_cable_generic_usbconn_help (urj_log_level_t ll, const char *cablename)
{
    urj_log (ll,
             _("Usage: cable %s [vid=VID] [pid=PID] [desc=DESC] [...]\n"
               "\n"
               "VID        USB Device Vendor ID (hex, e.g. 0abc)\n"
               "PID        USB Device Product ID (hex, e.g. 0abc)\n"
               "DESC       Some string to match in description or serial no.\n"
               "\n"), cablename);
}
