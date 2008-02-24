/*
 * $Id: usbconn.h 809 2007-12-04 07:06:49Z kawk $
 *
 * USB Device Connection Driver Interface
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

#ifndef USBCONN_H
#define	USBCONN_H

#include <stdint.h>

typedef struct usbconn_t usbconn_t;

#include "cable.h"

typedef struct
{
	char *name;
	char *desc;
	char *driver;
	int32_t vid;
	int32_t pid;
} usbconn_cable_t;

typedef struct {
	const char *type;
	usbconn_t *(*connect)( const char **, int, usbconn_cable_t *);
	void (*free)( usbconn_t * );
	int (*open)( usbconn_t * );
	int (*close)( usbconn_t * );
} usbconn_driver_t;

struct usbconn_t {
	usbconn_driver_t *driver;
	void *params;
	cable_t *cable;
};

usbconn_t *usbconn_connect( const char **, int, usbconn_cable_t *);
int usbconn_free( usbconn_t *conn );
int usbconn_open( usbconn_t *conn );
int usbconn_close( usbconn_t *conn );
extern usbconn_driver_t *usbconn_drivers[];

#endif /* USBCONN_H */
