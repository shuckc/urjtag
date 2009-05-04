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

#include <urjtag/sysdep.h>

#include <strings.h>
#include <stddef.h>

#include <urjtag/usbconn.h>

#ifdef HAVE_LIBUSB
extern urj_usbconn_driver_t urj_tap_usbconn_libusb_driver;
#endif /* HAVE_LIBUSB */
#ifdef ENABLE_LOWLEVEL_FTD2XX
extern urj_usbconn_driver_t urj_tap_usbconn_ftd2xx_driver;
extern urj_usbconn_driver_t urj_tap_usbconn_ftd2xx_mpsse_driver;
#endif /* ENABLE_LOWLEVEL_FTD2XX */
#ifdef ENABLE_LOWLEVEL_FTDI
extern urj_usbconn_driver_t urj_tap_usbconn_ftdi_driver;
extern urj_usbconn_driver_t urj_tap_usbconn_ftdi_mpsse_driver;
#endif /* ENABLE_LOWLEVEL_FTDI */

urj_usbconn_driver_t *urj_tap_usbconn_drivers[] = {
#ifdef HAVE_LIBUSB
    &urj_tap_usbconn_libusb_driver,
#endif /* HAVE_LIBUSB */
#ifdef ENABLE_LOWLEVEL_FTD2XX
    &urj_tap_usbconn_ftd2xx_driver,
    &urj_tap_usbconn_ftd2xx_mpsse_driver,
#endif /* ENABLE_LOWLEVEL_FTD2XX */
#ifdef ENABLE_LOWLEVEL_FTDI
    &urj_tap_usbconn_ftdi_driver,
    &urj_tap_usbconn_ftdi_mpsse_driver,
#endif /* ENABLE_LOWLEVEL_FTDI */
    NULL                        /* last must be NULL */
};

int
urj_tap_usbconn_open (urj_usbconn_t *conn)
{
    return conn->driver->open (conn);
}

int
urj_tap_usbconn_close (urj_usbconn_t *conn)
{
    return conn->driver->close (conn);
}

int
urj_tap_usbconn_read (urj_usbconn_t *conn, uint8_t *buf, int len)
{
    if (conn->driver->read)
        return conn->driver->read (conn, buf, len);
    else
        return 0;
}

int
urj_tap_usbconn_write (urj_usbconn_t *conn, uint8_t *buf, int len, int recv)
{
    if (conn->driver->write)
        return conn->driver->write (conn, buf, len, recv);
    else
        return 0;
}
