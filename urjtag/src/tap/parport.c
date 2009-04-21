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

#include "parport.h"

extern parport_driver_t direct_parport_driver;
extern parport_driver_t ppdev_parport_driver;
extern parport_driver_t ppi_parport_driver;

parport_driver_t *parport_drivers[] = {
#ifdef ENABLE_LOWLEVEL_DIRECT
    &direct_parport_driver,
#endif /* ENABLE_LOWLEVEL_DIRECT */

#ifdef ENABLE_LOWLEVEL_PPDEV
    &ppdev_parport_driver,
#endif /* ENABLE_LOWLEVEL_PPDEV */

#ifdef ENABLE_LOWLEVEL_PPI
    &ppi_parport_driver,
#endif /* ENABLE_LOWLEVEL_PPI */
    NULL                        /* last must be NULL */
};


int
parport_open (parport_t * port)
{
    return port->driver->open (port);
}

int
parport_close (parport_t * port)
{
    return port->driver->close (port);
}

int
parport_set_data (parport_t * port, uint8_t data)
{
    return port->driver->set_data (port, data);
}

int
parport_get_data (parport_t * port)
{
    return port->driver->get_data (port);
}

int
parport_get_status (parport_t * port)
{
    return port->driver->get_status (port);
}

int
parport_set_control (parport_t * port, uint8_t data)
{
    return port->driver->set_control (port, data);
}
