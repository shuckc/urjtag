/*
 * $Id$
 *
 * Xilinx Platform Cable USB functions
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
 * Written by Kolja Waschk <kawk>, 2008
 *
 */

#ifndef XPCU_H
#define XPCU_H 1

#include <stdint.h>
#include <usb.h>

#define XPCU_VID 0x03FD
#define XPCU_PID 0x0008

struct usb_device *find_xpcu (void);
int xpcu_init ();
int xpcu_close (struct usb_dev_handle *xpcu);
int xpcu_request_28 (struct usb_dev_handle *xpcu, int value);
int xpcu_raise_ioa5 (struct usb_dev_handle *xpcu);
int xpcu_write_gpio (struct usb_dev_handle *xpcu, uint8_t bits);
int xpcu_read_gpio (struct usb_dev_handle *xpcu, uint8_t * bits);
int xpcu_bitrev_test (struct usb_dev_handle *xpcu);
int xpcu_select_gpio (struct usb_dev_handle *xpcu, int select);
int xpcu_open (struct usb_dev_handle **xpcu);
int xpcu_request_a6 (struct usb_dev_handle *xpcu, int nibbles, uint8_t * xmit,
                     int inlen, uint8_t * recv);


#endif /* XPCU_H */
