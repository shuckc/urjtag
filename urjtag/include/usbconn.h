/*
 * $Id$
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

#ifndef URJ_USBCONN_H
#define	URJ_USBCONN_H

#include <stdint.h>
#include <stddef.h>

typedef struct urj_usbconn urj_usbconn_t;

#include "cable.h"

typedef struct
{
    char *name;
    char *desc;
    char *driver;
    int32_t vid;
    int32_t pid;
} urj_usbconn_cable_t;

typedef struct
{
    const char *type;
    urj_usbconn_t *(*connect) (const char **, int, urj_usbconn_cable_t *);
    void (*free) (urj_usbconn_t *);
    int (*open) (urj_usbconn_t *);
    int (*close) (urj_usbconn_t *);
    int (*read) (urj_usbconn_t *, uint8_t *, int);
    int (*write) (urj_usbconn_t *, uint8_t *, int, int);
} urj_usbconn_driver_t;

struct urj_usbconn
{
    urj_usbconn_driver_t *driver;
    void *params;
    urj_cable_t *cable;
};

urj_usbconn_t *usbconn_connect (const char **, int, urj_usbconn_cable_t *);
int usbconn_free (urj_usbconn_t *conn);
int urj_tap_usbconn_open (urj_usbconn_t *conn);
int urj_tap_usbconn_close (urj_usbconn_t *conn);
int urj_tap_usbconn_read (urj_usbconn_t *conn, uint8_t *buf, int len);
int urj_tap_usbconn_write (urj_usbconn_t *conn, uint8_t *buf, int len, int recv);
extern urj_usbconn_driver_t *usbconn_drivers[];

#endif /* URJ_USBCONN_H */
