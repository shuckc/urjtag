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
#include "chain.h"

typedef struct bus bus_t;

struct bus {
	void *params;
	void (*prepare)( bus_t *bus );
	int (*width)( bus_t *bus );
	void (*read_start)( bus_t *bus, uint32_t adr );
	uint32_t (*read_next)( bus_t *bus, uint32_t adr );
	uint32_t (*read_end)( bus_t *bus );
	uint32_t (*read)( bus_t *bus, uint32_t adr );
	void (*write)( bus_t *bus, uint32_t adr, uint32_t data );
	void (*free)( bus_t *bus );
};

#define	bus_prepare(bus)	bus->prepare(bus)
#define	bus_width(bus)		bus->width(bus)
#define	bus_read_start(bus,adr)	bus->read_start(bus,adr)
#define	bus_read_next(bus,adr)	bus->read_next(bus,adr)
#define	bus_read_end(bus)	bus->read_end(bus)
#define	bus_read(bus,adr)	bus->read(bus,adr)
#define	bus_write(bus,adr,data)	bus->write(bus,adr,data)
#define	bus_free(bus)		bus->free(bus)

bus_t *new_sa1110_bus( chain_t *chain, int pn );
bus_t *new_pxa250_bus( chain_t *chain, int pn );
bus_t *new_ixp425_bus( chain_t *chain, int pn );

#endif /* BUS_H */
