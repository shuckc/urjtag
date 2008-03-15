/*
 * $Id$
 *
 * Written by Kent Palmkvist (kentp@isy.liu.se>, 2005.
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
writemem( bus_t *bus, FILE *f, uint32_t addr, uint32_t len )
{
	uint32_t step;
	uint32_t a;
	int bc = 0;
	int bidx = 0;
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
	printf( _("writing:\n") );

	for (; a < end; a += step) {
		uint32_t data;
		int j;

		/* Read one block of data */
		if ( bc < step ) {
			printf( _("addr: 0x%08X"), a );
			printf( "\r" );
			fflush( stdout );
			if (bc != 0)
			  printf( _("Data not on word boundary, NOT SUPPORTED!"));
			if (feof(f)) {
			  printf( _("Unexpected end of file!\n"));
			  printf( _("Addr: 0x%08X\n"), a);
			  break;
			}
			bc = fread( b, 1, BSIZE, f );
			if (!bc) {
			  printf( _("Short read: bc=0x%X\n"), bc);
			}
			bidx = 0;

		}

		/* Write a word at  time */
		data = 0;
		for (j = step; j > 0; j--) {
			if (big_endian) {
			        data |= b[bidx++];
				data <<= 8;
				bc--;
			} else {
			        data |= (b[bidx++] << ((step - j) * 8));
				bc--;
			}
		}

		bus_write( bus, a, data );
	  
	}

	printf( _("\nDone.\n") );
}
