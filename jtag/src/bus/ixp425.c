/*
 * $Id$
 *
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
 * Written by Christian Pellegrin <chri@ascensit.com>, 2003.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"

typedef struct {
	chain_t *chain;
	part_t *part;
} bus_params_t;

#define	CHAIN	((bus_params_t *) bus->params)->chain
#define	PART	((bus_params_t *) bus->params)->part

static void 
select_flash( part_t *p) 
{
	part_set_signal( p, "EX_CS[0]", 1, 0 );
	part_set_signal( p, "EX_CS[1]", 1, 1 );
	part_set_signal( p, "EX_CS[2]", 1, 1 );
	part_set_signal( p, "EX_CS[3]", 1, 1 );
	part_set_signal( p, "EX_CS[4]", 1, 1 );
	part_set_signal( p, "EX_CS[5]", 1, 1 );
	part_set_signal( p, "EX_CS[6]", 1, 1 );
	part_set_signal( p, "EX_CS[7]", 1, 1 );
}

static void 
unselect_flash( part_t *p) 
{
	part_set_signal( p, "EX_CS[0]", 1, 1 );
	part_set_signal( p, "EX_CS[1]", 1, 1 );
	part_set_signal( p, "EX_CS[2]", 1, 1 );
	part_set_signal( p, "EX_CS[3]", 1, 1 );
	part_set_signal( p, "EX_CS[4]", 1, 1 );
	part_set_signal( p, "EX_CS[5]", 1, 1 );
	part_set_signal( p, "EX_CS[6]", 1, 1 );
	part_set_signal( p, "EX_CS[7]", 1, 1 );
}

static void
setup_address( part_t *p, uint32_t a )
{
	int i;
	char buff[15];

	for (i = 0; i < 24; i++) {
		sprintf( buff, "EX_ADDR[%d]", i );
		part_set_signal( p, buff, 1, (a >> i) & 1 );
	}
}

static void
set_data_in( part_t *p )
{
	int i;
	char buff[15];

	for (i = 0; i < 16; i++) {
		sprintf( buff, "EX_DATA[%d]", i );
		part_set_signal( p, buff, 0, 0 );
	}
}

static void
setup_data( part_t *p, uint32_t d )
{
	int i;
	char buff[15];

	for (i = 0; i < 16; i++) {
		sprintf( buff, "EX_DATA[%d]", i );
		part_set_signal( p, buff, 1, (d >> i) & 1 );
	}
}

static void
ixp425_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
ixp425_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	select_flash( p );
	part_set_signal( p, "EX_RD", 1, 0 );
	part_set_signal( p, "EX_WR", 1, 1 );

	setup_address( p, adr );
	set_data_in( p );

	chain_shift_data_registers( chain );
}

static uint32_t
ixp425_bus_read_next( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	setup_address( p, adr );
	chain_shift_data_registers( chain );

	{
		int i;
		char buff[15];
		uint32_t d = 0;

		for (i = 0; i < 16; i++) {
			sprintf( buff, "EX_DATA[%d]", i );
			d |= (uint32_t) (part_get_signal( p, buff ) << i);
		}

		return d;
	}
}

static uint32_t
ixp425_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	unselect_flash( p );
	part_set_signal( p, "EX_RD", 1, 1 );
	part_set_signal( p, "EX_WR", 1, 1 );

	chain_shift_data_registers( chain );

	{
		int i;
		char buff[15];
		uint32_t d = 0;

		for (i = 0; i < 16; i++) {
			sprintf( buff, "EX_DATA[%d]", i );
			d |= (uint32_t) (part_get_signal( p, buff ) << i);
		}

		return d;
	}
}

static uint32_t
ixp425_bus_read( bus_t *bus, uint32_t adr )
{
	ixp425_bus_read_start( bus, adr );
	return ixp425_bus_read_end( bus );
}

static void
ixp425_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	select_flash( p );
	part_set_signal( p, "EX_RD", 1, 1 );

	setup_address( p, adr );
	setup_data( p, data );

	chain_shift_data_registers( chain );

	part_set_signal( p, "EX_WR", 1, 0 );
	chain_shift_data_registers( chain );
	part_set_signal( p, "EX_WR", 1, 1 );
	unselect_flash( p );
	chain_shift_data_registers( chain );
}

static int
ixp425_bus_width( bus_t *bus, uint32_t adr )
{
	return 16;
}

static void
ixp425_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}


static const bus_t ixp425_bus = {
	NULL,
	ixp425_bus_prepare,
	ixp425_bus_width,
	ixp425_bus_read_start,
	ixp425_bus_read_next,
	ixp425_bus_read_end,
	ixp425_bus_read,
	ixp425_bus_write,
	ixp425_bus_free
};

bus_t *
new_ixp425_bus( chain_t *chain, int pn )
{
	bus_t *bus;

	if (!chain || !chain->parts || chain->parts->len <= pn || pn < 0)
		return NULL;

	bus = malloc( sizeof (bus_t) );
	if (!bus)
		return NULL;

	memcpy( bus, &ixp425_bus, sizeof (bus_t) );

	bus->params = malloc( sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[pn];

	return bus;
}
