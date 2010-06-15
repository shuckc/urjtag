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

#include <stddef.h>

#include <urjtag/parport.h>

#include "parport.h"

const urj_parport_driver_t *const urj_tap_parport_drivers[] = {
#define _URJ_LIST(item) &urj_tap_parport_##item##_driver,
#include "parport_list.h"
    NULL                        /* last must be NULL */
};


int
urj_tap_parport_open (urj_parport_t *port)
{
    return port->driver->open (port);
}

int
urj_tap_parport_close (urj_parport_t *port)
{
    return port->driver->close (port);
}

int
urj_tap_parport_set_data (urj_parport_t *port, const unsigned char data)
{
    return port->driver->set_data (port, data);
}

int
urj_tap_parport_get_data (urj_parport_t *port)
{
    return port->driver->get_data (port);
}

int
urj_tap_parport_get_status (urj_parport_t *port)
{
    return port->driver->get_status (port);
}

int
urj_tap_parport_set_control (urj_parport_t *port, const unsigned char data)
{
    return port->driver->set_control (port, data);
}

const char *
urj_cable_parport_devtype_string(urj_cable_parport_devtype_t dt)
{
    switch (dt)
    {
    case URJ_CABLE_PARPORT_DEV_PARALLEL: return "parallel";
    case URJ_CABLE_PARPORT_DEV_PPDEV:    return "ppdev";
    case URJ_CABLE_PARPORT_DEV_PPI:      return "ppi";
    case URJ_CABLE_PARPORT_N_DEVS:       return "#devs";
    }
    return "<unknown parport devtype>";
}

