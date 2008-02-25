/*
 * $Id: usbconn/libusb.h,v 1.7 2003/08/19 09:05:25 telka Exp $
 *
 * Link driver for accessing USB devices via libusb
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
 * Written by Kolja Waschk, 2008
 *
 */

#ifndef _USBCONN_LIBUSB_H
#define _USBCONN_LIBUSB_H 1

#include <usb.h>

typedef struct {
	struct usb_device *dev;
	struct usb_dev_handle *handle;
	void *data;
} libusb_param_t;

#endif

