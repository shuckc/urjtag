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
 */

#include <stdint.h>

#include "part.h"
#include "bus.h"

/* SA1110 must be at position 0 in JTAG chain */

static void
setup_address( part *p, uint32_t a )
{
	int i;
	char buff[10];

	for (i = 0; i < 26; i++) {
		sprintf( buff, "A%d", i );
		part_set_signal( p, buff, 1, (a >> i) & 1 );
	}
}

static void
set_data_in( part *p )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "D%d", i );
		part_set_signal( p, buff, 0, 0 );
	}
}

static void
setup_data( part *p, uint32_t d )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "D%d", i );
		part_set_signal( p, buff, 1, (d >> i) & 1 );
	}
}

void
sa1110_bus_read_start( parts *ps, uint32_t adr )
{
	/* see Figure 10-12 in SA doc */
	part *p = ps->parts[0];

	part_set_signal( p, "nCS0", 1, 0 );
	part_set_signal( p, "nCS1", 1, 1 );
	part_set_signal( p, "nCS2", 1, 1 );
	part_set_signal( p, "nCS3", 1, 1 );
	part_set_signal( p, "nCS4", 1, 1 );
	part_set_signal( p, "nCS5", 1, 1 );
	part_set_signal( p, "RD_nWR", 1, 1 );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nOE", 1, 0 );

	setup_address( p, adr );
	set_data_in( p );

	parts_shift_data_registers( ps );
}

uint32_t
sa1110_bus_read_next( parts *ps, uint32_t adr )
{
	/* see Figure 10-12 in SA doc */
	part *p = ps->parts[0];

	setup_address( p, adr );
	parts_shift_data_registers( ps );

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

uint32_t
sa1110_bus_read_end( parts *ps )
{
	/* see Figure 10-12 in SA doc */
	part *p = ps->parts[0];

	part_set_signal( p, "nCS0", 1, 1 );
	part_set_signal( p, "nOE", 1, 1 );
	parts_shift_data_registers( ps );

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

uint32_t
sa1110_bus_read( parts *ps, uint32_t adr )
{
	sa1110_bus_read_start( ps, adr );
	return sa1110_bus_read_end( ps );
}

void
sa1110_bus_write( parts *ps, uint32_t adr, uint32_t data )
{
	/* see Figure 10-16 in SA doc */
	part *p = ps->parts[0];

	part_set_signal( p, "nCS0", 1, 0 );
	part_set_signal( p, "nCS1", 1, 1 );
	part_set_signal( p, "nCS2", 1, 1 );
	part_set_signal( p, "nCS3", 1, 1 );
	part_set_signal( p, "nCS4", 1, 1 );
	part_set_signal( p, "nCS5", 1, 1 );
	part_set_signal( p, "RD_nWR", 1, 0 );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nOE", 1, 1 );

	setup_address( p, adr );
	setup_data( p, data );

	parts_shift_data_registers( ps );

	part_set_signal( p, "nWE", 1, 0 );
	parts_shift_data_registers( ps );
	part_set_signal( p, "nWE", 1, 1 );
	parts_shift_data_registers( ps );
}

int
sa1110_bus_width( parts *ps )
{
	if (part_get_signal( ps->parts[0], "ROM_SEL" )) {
		printf( "ROM_SEL: 32 bits\n" );
		return 32;
	} else {
		printf( "ROM_SEL: 16 bits\n" );
		return 16;
	}
}

bus_driver_t sa1110_bus_driver = {
	sa1110_bus_width,
	sa1110_bus_read_start,
	sa1110_bus_read_next,
	sa1110_bus_read_end,
	sa1110_bus_read,
	sa1110_bus_write
};
