/*
 * $Id$
 *
 * Bus driver interface
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#ifndef BUS_H
#define	BUS_H

#include <stdint.h>
#include "part.h"

typedef struct {
	void (*bus_read_start)( parts *, uint32_t );
	uint32_t (*bus_read_next)( parts *, uint32_t );
	uint32_t (*bus_read_end)( parts * );
	uint32_t (*bus_read)( parts *, uint32_t );
	void (*bus_write)( parts *, uint32_t, uint32_t );
} bus_driver_t;

extern bus_driver_t *bus_driver;
#define	bus_read_start	bus_driver->bus_read_start
#define	bus_read_next	bus_driver->bus_read_next
#define	bus_read_end	bus_driver->bus_read_end
#define	bus_read	bus_driver->bus_read
#define	bus_write	bus_driver->bus_write

extern bus_driver_t sa1110_bus_driver;
extern bus_driver_t pxa250_bus_driver;

#endif /* BUS_H */
