/*
 * $Id: libftdx.h 1594 2009-05-13 23:36:04Z rfhh $
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
 * Written by A. Laeuger, 2008
 *
 */

#ifndef URJ_USBCONN_LIBFTDX_H
#define URJ_USBCONN_LIBFTDX_H 1

#define URJ_USBCONN_FTDX_MAXSEND 4096
#define URJ_USBCONN_FTDX_MAXSEND_MPSSE (64 * 1024)

/* Maximum chunk to receive from ftdi/ftd2xx driver.
   Larger values might speed up comm, but there's an upper limit
   when too many bytes are sent and the underlying libftdi or libftd2xx
   don't fetch the returned data in time -> deadlock */
#define URJ_USBCONN_FTDI_MAXRECV   ( 4 * 64)
#define URJ_USBCONN_FTD2XX_MAXRECV (63 * 64)
#define URJ_USBCONN_FTDX_MAXRECV   (URJ_USBCONN_FTD2XX_MAXRECV < URJ_USBCONN_FTDI_MAXRECV ? URJ_USBCONN_FTD2XX_MAXRECV : URJ_USBCONN_FTDI_MAXRECV)

#endif
