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
#include <urjtag/parport.h>

#include <urjtag/cmd.h>

#include "generic.h"
#include "generic_parport.h"

#ifdef UNUSED   // @@@@ RFHH
static void
print_vector (urj_log_level_t ll, int len, char *vec)
{
    int i;
    for (i = 0; i < len; i++)
        urj_log (ll, "%c", vec[i] ? '1' : '0');
}
#endif

int
urj_tap_cable_generic_parport_connect (urj_cable_t *cable,
                                       urj_cable_parport_devtype_t devtype,
                                       const char *devname,
                                       const urj_param_t *params[])
{
    urj_tap_cable_generic_params_t *cable_params;
    urj_parport_t *port;
    int i;

    if (urj_param_num (params) > 0)
    {
        urj_error_set (URJ_ERROR_SYNTAX, _("extra arguments"));
        return URJ_STATUS_FAIL;
    }

    /* search parport driver list */
    for (i = 0; urj_tap_parport_drivers[i]; i++)
        if (devtype == urj_tap_parport_drivers[i]->type)
            break;
    if (!urj_tap_parport_drivers[i])
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("Unknown port type: %s"),
                       urj_cable_parport_devtype_string(devtype));
        return URJ_STATUS_FAIL;
    }

    /* set up parport driver */
    port = urj_tap_parport_drivers[i]->connect (devname);

    if (port == NULL)
    {
        // retain error state
        return URJ_STATUS_FAIL;
    }

    cable_params = malloc (sizeof *cable_params);
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof *cable_params);
        urj_tap_parport_drivers[i]->parport_free (port);
        return URJ_STATUS_FAIL;
    }

    cable->link.port = port;
    cable->params = cable_params;
    cable->chain = NULL;

    return URJ_STATUS_OK;
}

void
urj_tap_cable_generic_parport_free (urj_cable_t *cable)
{
    cable->link.port->driver->parport_free (cable->link.port);
    free (cable->params);
    free (cable);
}

void
urj_tap_cable_generic_parport_done (urj_cable_t *cable)
{
    urj_tap_parport_close (cable->link.port);
}

void
urj_tap_cable_generic_parport_help (urj_log_level_t ll, const char *cablename)
{
    urj_log (ll,
             _("Usage: cable %s parallel PORTADDR\n"
#if ENABLE_LOWLEVEL_PPDEV
               "   or: cable %s ppdev PPDEV\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
               "   or: cable %s ppi PPIDEV\n"
#endif
               "\n" "PORTADDR   parallel port address (e.g. 0x378)\n"
#if ENABLE_LOWLEVEL_PPDEV
               "PPDEV      ppdev device (e.g. /dev/parport0)\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
               "PPIDEF     ppi device (e.g. /dev/ppi0)\n"
#endif
               "\n"),
#if ENABLE_LOWLEVEL_PPDEV
             cablename,
#endif
#if HAVE_DEV_PPBUS_PPI_H
             cablename,
#endif
             cablename);
}
