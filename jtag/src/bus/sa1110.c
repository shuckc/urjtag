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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel StrongARM SA-1110 Microprocessor
 *     Developer's Manual", October 2001, Order Number: 278240-004
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
setup_address( part_t *p, uint32_t a )
{
	int i;
	char buff[10];

	for (i = 0; i < 26; i++) {
		sprintf( buff, "A%d", i );
		part_set_signal( p, buff, 1, (a >> i) & 1 );
	}
}

static void
set_data_in( part_t *p )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "D%d", i );
		part_set_signal( p, buff, 0, 0 );
	}
}

static void
setup_data( part_t *p, uint32_t d )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "D%d", i );
		part_set_signal( p, buff, 1, (d >> i) & 1 );
	}
}

static void
sa1110_bus_prepare( bus_t *bus )
{       
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
sa1110_bus_read_start( bus_t *bus, uint32_t adr )
{
	/* see Figure 10-12 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, "nCS0", 1, (adr >> 27) != 0 );
	part_set_signal( p, "nCS1", 1, (adr >> 27) != 1 );
	part_set_signal( p, "nCS2", 1, (adr >> 27) != 2 );
	part_set_signal( p, "nCS3", 1, (adr >> 27) != 3 );
	part_set_signal( p, "nCS4", 1, (adr >> 27) != 8 );
	part_set_signal( p, "nCS5", 1, (adr >> 27) != 9 );
	part_set_signal( p, "RD_nWR", 1, 1 );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nOE", 1, 0 );

	setup_address( p, adr );
	set_data_in( p );

	chain_shift_data_registers( chain );
}

static uint32_t
sa1110_bus_read_next( bus_t *bus, uint32_t adr )
{
	/* see Figure 10-12 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	setup_address( p, adr );
	chain_shift_data_registers( chain );

	{
		int i;
		char buff[10];
		uint32_t d = 0;

		for (i = 0; i < 32; i++) {
			sprintf( buff, "D%d", i );
			d |= (uint32_t) (part_get_signal( p, buff ) << i);
		}

		return d;
	}
}

static uint32_t
sa1110_bus_read_end( bus_t *bus )
{
	/* see Figure 10-12 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, "nCS0", 1, 1 );
	part_set_signal( p, "nCS1", 1, 1 );
	part_set_signal( p, "nCS2", 1, 1 );
	part_set_signal( p, "nCS3", 1, 1 );
	part_set_signal( p, "nCS4", 1, 1 );
	part_set_signal( p, "nCS5", 1, 1 );
	part_set_signal( p, "nOE", 1, 1 );
	chain_shift_data_registers( chain );

	{
		int i;
		char buff[10];
		uint32_t d = 0;

		for (i = 0; i < 32; i++) {
			sprintf( buff, "D%d", i );
			d |= (uint32_t) (part_get_signal( p, buff ) << i);
		}

		return d;
	}
}

static uint32_t
sa1110_bus_read( bus_t *bus, uint32_t adr )
{
	sa1110_bus_read_start( bus, adr );
	return sa1110_bus_read_end( bus );
}

static void
sa1110_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	/* see Figure 10-16 in [1] */
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, "nCS0", 1, (adr >> 27) != 0 );
	part_set_signal( p, "nCS1", 1, (adr >> 27) != 1 );
	part_set_signal( p, "nCS2", 1, (adr >> 27) != 2 );
	part_set_signal( p, "nCS3", 1, (adr >> 27) != 3 );
	part_set_signal( p, "nCS4", 1, (adr >> 27) != 8 );
	part_set_signal( p, "nCS5", 1, (adr >> 27) != 9 );
	part_set_signal( p, "RD_nWR", 1, 0 );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nOE", 1, 1 );

	setup_address( p, adr );
	setup_data( p, data );

	chain_shift_data_registers( chain );

	part_set_signal( p, "nWE", 1, 0 );
	chain_shift_data_registers( chain );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nCS0", 1, 1 );
	part_set_signal( p, "nCS1", 1, 1 );
	part_set_signal( p, "nCS2", 1, 1 );
	part_set_signal( p, "nCS3", 1, 1 );
	part_set_signal( p, "nCS4", 1, 1 );
	part_set_signal( p, "nCS5", 1, 1 );
	chain_shift_data_registers( chain );
}

static int
sa1110_bus_width( bus_t *bus )
{
	if (part_get_signal( PART, "ROM_SEL" )) {
		printf( "ROM_SEL: 32 bits\n" );
		return 32;
	} else {
		printf( "ROM_SEL: 16 bits\n" );
		return 16;
	}
}

static void
sa1110_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}

static const bus_t sa1110_bus = {
	NULL,
	sa1110_bus_prepare,
	sa1110_bus_width,
	sa1110_bus_read_start,
	sa1110_bus_read_next,
	sa1110_bus_read_end,
	sa1110_bus_read,
	sa1110_bus_write,
	sa1110_bus_free
};

bus_t *
new_sa1110_bus( chain_t *chain, int pn )
{
	bus_t *bus;

	if (!chain || !chain->parts || chain->parts->len <= pn || pn < 0)
		return NULL;

	bus = malloc( sizeof (bus_t) );
	if (!bus)
		return NULL;

	memcpy( bus, &sa1110_bus, sizeof (bus_t) );

	bus->params = malloc( sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[pn];

	return bus;
}

