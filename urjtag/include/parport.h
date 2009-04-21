/*
 * $Id$
 *
 * Parallel Port Connection Driver Interface
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

#ifndef PARPORT_H
#define	PARPORT_H

#include <stdint.h>

typedef struct parport_t parport_t;

#include "cable.h"

typedef struct
{
    const char *type;
    parport_t *(*connect) (const char **, int);
    void (*parport_free) (parport_t *);
    int (*open) (parport_t *);
    int (*close) (parport_t *);
    int (*set_data) (parport_t *, uint8_t);
    int (*get_data) (parport_t *);
    int (*get_status) (parport_t *);
    int (*set_control) (parport_t *, uint8_t);
} parport_driver_t;

struct parport_t
{
    parport_driver_t *driver;
    void *params;
    cable_t *cable;
};

int parport_open (parport_t * port);
int parport_close (parport_t * port);
int parport_set_data (parport_t * port, uint8_t data);
int parport_get_data (parport_t * port);
int parport_get_status (parport_t * port);
int parport_set_control (parport_t * port, uint8_t data);

extern parport_driver_t *parport_drivers[];

#endif /* PARPORT_H */
