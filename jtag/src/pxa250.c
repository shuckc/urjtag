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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#include <stdint.h>

#include "part.h"
#include "bus.h"

/* PXA250 must be at position 0 in JTAG chain */

static void
setup_address( part *p, uint32_t a )
{
	int i;
	char buff[10];

	for (i = 0; i < 26; i++) {
		sprintf( buff, "MA[%d]", i );
		part_set_signal( p, buff, 1, (a >> i) & 1 );
	}
}

static void
set_data_in( part *p )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "MD[%d]", i );
		part_set_signal( p, buff, 0, 0 );
	}
}

static void
setup_data( part *p, uint32_t d )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "MD[%d]", i );
		part_set_signal( p, buff, 1, (d >> i) & 1 );
	}
}

void
pxa250_bus_read_start( parts *ps, uint32_t adr )
{
	part *p = ps->parts[0];

	/* see Figure 6-13 in [1] */
	part_set_signal( p, "nCS[0]", 1, 0 );
	part_set_signal( p, "DQM[0]", 1, 0 );
	part_set_signal( p, "DQM[1]", 1, 0 );
	part_set_signal( p, "DQM[2]", 1, 0 );
	part_set_signal( p, "DQM[3]", 1, 0 );
	part_set_signal( p, "RDnWR", 1, 1 );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nOE", 1, 0 );
	part_set_signal( p, "nSDCAS", 1, 0 );

	setup_address( p, adr );
	set_data_in( p );

	parts_shift_data_registers( ps );
}

uint32_t
pxa250_bus_read_next( parts *ps, uint32_t adr )
{
	part *p = ps->parts[0];

	/* see Figure 6-13 in [1] */
	setup_address( p, adr );
	parts_shift_data_registers( ps );

	{
		int i;
		char buff[10];
		uint32_t d = 0;

		for (i = 0; i < 32; i++) {
			sprintf( buff, "MD[%d]", i );
			d |= (uint32_t) (part_get_signal( p, buff ) << i);
		}

		return d;
	}
}

uint32_t
pxa250_bus_read_end( parts *ps )
{
	part *p = ps->parts[0];

	/* see Figure 6-13 in [1] */
	part_set_signal( p, "nCS[0]", 1, 1 );
	part_set_signal( p, "nOE", 1, 1 );
	part_set_signal( p, "nSDCAS", 1, 1 );

	parts_shift_data_registers( ps );

	{
		int i;
		char buff[10];
		uint32_t d = 0;

		for (i = 0; i < 32; i++) {
			sprintf( buff, "MD[%d]", i );
			d |= (uint32_t) (part_get_signal( p, buff ) << i);
		}

		return d;
	}
}

uint32_t
pxa250_bus_read( parts *ps, uint32_t adr )
{
	pxa250_bus_read_start( ps, adr );
	return pxa250_bus_read_end( ps );
}

void
pxa250_bus_write( parts *ps, uint32_t adr, uint32_t data )
{
	/* see Figure 6-17 in [1] */
	part *p = ps->parts[0];

	part_set_signal( p, "nCS[0]", 1, 0 );
	part_set_signal( p, "DQM[0]", 1, 0 );
	part_set_signal( p, "DQM[1]", 1, 0 );
	part_set_signal( p, "DQM[2]", 1, 0 );
	part_set_signal( p, "DQM[3]", 1, 0 );
	part_set_signal( p, "RDnWR", 1, 0 );
	part_set_signal( p, "nWE", 1, 1 );
	part_set_signal( p, "nOE", 1, 1 );
	part_set_signal( p, "nSDCAS", 1, 0 );

	setup_address( p, adr );
	setup_data( p, data );

	parts_shift_data_registers( ps );

	part_set_signal( p, "nWE", 1, 0 );
	parts_shift_data_registers( ps );
	part_set_signal( p, "nWE", 1, 1 );
	parts_shift_data_registers( ps );
}

int
pxa250_bus_width( parts *ps )
{
	part *p = ps->parts[0];
	uint8_t boot_sel = (part_get_signal( p, "BOOT_SEL[2]" ) << 2)
			| (part_get_signal( p, "BOOT_SEL[1]" ) << 1)
			| part_get_signal( p, "BOOT_SEL[0]" );

	/* see Table 6-36. in [1] */
	switch (boot_sel) {
		case 0:
			printf( "BOOT_SEL: Asynchronous 32-bit ROM\n" );
			return 32;
		case 1:
			printf( "BOOT_SEL: Asynchronous 16-bit ROM\n" );
			return 16;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			printf( "TODO - BOOT_SEL: %d\n", boot_sel );
			return 0;
		default:
			printf( "BUG in code, file %s, line %d.\n", __FILE__, __LINE__ );
			return 0;
	}
}

bus_driver_t pxa250_bus_driver = {
	pxa250_bus_width,
	pxa250_bus_read_start,
	pxa250_bus_read_next,
	pxa250_bus_read_end,
	pxa250_bus_read,
	pxa250_bus_write
};
