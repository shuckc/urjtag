/*
 * $Id: usbconn.c 851 2007-12-15 22:53:24Z kawk $
 *
 * Copyright (C) 2008 K. Waschk
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
 * Written by  Kolja Waschk <kawk>, 2008
 *
 */

#include "sysdep.h"

#include <strings.h>

#include "usbconn.h"

#ifdef HAVE_LIBUSB
extern usbconn_driver_t usbconn_libusb_driver;
#endif /* HAVE_LIBUSB */

usbconn_driver_t *usbconn_drivers[] = {
#ifdef HAVE_LIBUSB
	&usbconn_libusb_driver,
#endif /* HAVE_LIBUSB */
	NULL				/* last must be NULL */
};

int
usbconn_open( usbconn_t *conn )
{
	return conn->driver->open( conn );
}

int
usbconn_close( usbconn_t *conn )
{
	return conn->driver->close( conn );
}
