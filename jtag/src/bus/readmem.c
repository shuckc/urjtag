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
 * [1] JEDEC Solid State Technology Association, "Common Flash Interface (CFI)",
 *     September 1999, Order Number: JESD68
 * [2] JEDEC Solid State Technology Association, "Common Flash Interface (CFI) ID Codes",
 *     September 2001, Order Number: JEP137-A
 *
 */

#include "sysdep.h"

#include <stdint.h>
#include <string.h>
#include <flash/cfi.h>
#include <flash/intel.h>
#include <flash/mic.h>

#include "bus.h"
#include "flash.h"
#include "jtag.h"

void
readmem( bus_t *bus, FILE *f, uint32_t addr, uint32_t len )
{
	uint32_t step;
	uint32_t a;
	int bc = 0;
#define BSIZE 4096
	uint8_t b[BSIZE];
	bus_area_t area;
	uint64_t end;

	if (!bus) {
		printf( _("Error: Missing bus driver!\n") );
		return;
	}

	bus_prepare( bus );

	if (bus_area( bus, addr, &area ) != 0) {
		printf( _("Error: Bus width detection failed\n") );
		return;
	}
	step = area.width / 8;

	if (step == 0) {
		printf( _("Unknown bus width!\n") );
		return;
	}

	addr = addr & (~(step - 1));
	len = (len + step - 1) & (~(step - 1));

	printf( _("address: 0x%08X\n"), addr );
	printf( _("length:  0x%08X\n"), len );

	if (len == 0) {
		printf( _("length is 0.\n") );
		return;
	}

	a = addr;
	end = a + len;
	printf( _("reading:\n") );
	bus_read_start( bus, addr );
	for (a += step; a <= end; a += step) {
		uint32_t data;
		int j;

		if (a < addr + len)
			data = bus_read_next( bus, a );
		else
			data = bus_read_end( bus );

		for (j = step; j > 0; j--)
			if (big_endian)
				b[bc++] = (data >> ((j - 1) * 8)) & 0xFF;
			else {
				b[bc++] = data & 0xFF;
				data >>= 8;
			}

		if ((bc >= BSIZE) || (a >= end) ) {
			printf( _("addr: 0x%08X"), a );
			printf( "\r" );
			fflush( stdout );
			fwrite( b, bc, 1, f );
			bc = 0;
		}
	}

	printf( _("\nDone.\n") );
}
