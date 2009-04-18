/*
 * $Id$
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
#ifdef ENABLE_LOWLEVEL_FTD2XX
extern usbconn_driver_t usbconn_ftd2xx_driver;
extern usbconn_driver_t usbconn_ftd2xx_mpsse_driver;
#endif /* ENABLE_LOWLEVEL_FTD2XX */
#ifdef ENABLE_LOWLEVEL_FTDI
extern usbconn_driver_t usbconn_ftdi_driver;
extern usbconn_driver_t usbconn_ftdi_mpsse_driver;
#endif /* ENABLE_LOWLEVEL_FTDI */

usbconn_driver_t *usbconn_drivers[] = {
#ifdef HAVE_LIBUSB
	&usbconn_libusb_driver,
#endif /* HAVE_LIBUSB */
#ifdef ENABLE_LOWLEVEL_FTD2XX
	&usbconn_ftd2xx_driver,
	&usbconn_ftd2xx_mpsse_driver,
#endif /* ENABLE_LOWLEVEL_FTD2XX */
#ifdef ENABLE_LOWLEVEL_FTDI
	&usbconn_ftdi_driver,
	&usbconn_ftdi_mpsse_driver,
#endif /* ENABLE_LOWLEVEL_FTDI */
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

int
usbconn_read( usbconn_t *conn, uint8_t *buf, int len )
{
	if (conn->driver->read)
		return conn->driver->read( conn, buf, len );
	else
		return 0;
}

int
usbconn_write( usbconn_t *conn, uint8_t *buf, int len, int recv )
{
	if (conn->driver->write)
		return conn->driver->write( conn, buf, len, recv );
	else
		return 0;
}
