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

#include <stdint.h>

#include "part.h"
#include "bus.h"

/* IXP425 must be at position 0 in JTAG chain */

static void 
select_flash( part *p) 
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
unselect_flash( part *p) 
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
setup_address( part *p, uint32_t a )
{
	int i;
	char buff[15];

	for (i = 0; i < 24; i++) {
		sprintf( buff, "EX_ADDR[%d]", i );
		part_set_signal( p, buff, 1, (a >> i) & 1 );
	}
}

static void
set_data_in( part *p )
{
	int i;
	char buff[15];

	for (i = 0; i < 16; i++) {
		sprintf( buff, "EX_DATA[%d]", i );
		part_set_signal( p, buff, 0, 0 );
	}
}

static void
setup_data( part *p, uint32_t d )
{
	int i;
	char buff[15];

	for (i = 0; i < 16; i++) {
		sprintf( buff, "EX_DATA[%d]", i );
		part_set_signal( p, buff, 1, (d >> i) & 1 );
	}
}

void
ixp425_bus_read_start( parts *ps, uint32_t adr )
{
	part *p = ps->parts[0];

	select_flash( p );
	part_set_signal( p, "EX_RD", 1, 0 );
	part_set_signal( p, "EX_WR", 1, 1 );

	setup_address( p, adr );
	set_data_in( p );

	parts_shift_data_registers( ps );
}

uint32_t
ixp425_bus_read_next( parts *ps, uint32_t adr )
{
	part *p = ps->parts[0];

	setup_address( p, adr );
	parts_shift_data_registers( ps );

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

uint32_t
ixp425_bus_read_end( parts *ps )
{
	part *p = ps->parts[0];

	unselect_flash( p );
	part_set_signal( p, "EX_RD", 1, 1 );
	part_set_signal( p, "EX_WR", 1, 1 );

	parts_shift_data_registers( ps );

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

uint32_t
ixp425_bus_read( parts *ps, uint32_t adr )
{
	ixp425_bus_read_start( ps, adr );
	return ixp425_bus_read_end( ps );
}

void
ixp425_bus_write( parts *ps, uint32_t adr, uint32_t data )
{
	part *p = ps->parts[0];

	select_flash( p );
	part_set_signal( p, "EX_RD", 1, 1 );

	setup_address( p, adr );
	setup_data( p, data );

	parts_shift_data_registers( ps );

	part_set_signal( p, "EX_WR", 1, 0 );
	parts_shift_data_registers( ps );
	part_set_signal( p, "EX_WR", 1, 1 );
	unselect_flash( p );
	parts_shift_data_registers( ps );
}

int
ixp425_bus_width( parts *ps )
{
	return 16;
}

bus_driver_t ixp425_bus_driver = {
	ixp425_bus_width,
	ixp425_bus_read_start,
	ixp425_bus_read_next,
	ixp425_bus_read_end,
	ixp425_bus_read,
	ixp425_bus_write
};
