/*
 * $Id$
 *
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
 * Written by Bradley D. LaRonde <brad@ltc.com>, 2003.
 *
 */

#include "sysdep.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <flash/cfi.h>

#include <brux/cfi.h>
#include <brux/bus.h>

int
jedec_detect( bus_t *bus, uint32_t adr, cfi_array_t **cfi_array )
{
	unsigned int bw;		/* bus width */
	int ba;				/* bus width address multiplier */
	bus_area_t area;
	int mid;
	int did;
	cfi_query_structure_t *cfi;

	if (!cfi_array || !bus)
		return -1;		/* invalid parameters */

	*cfi_array = calloc( 1, sizeof (cfi_array_t) );
	if (!*cfi_array)
		return -2;		/* out of memory */

	(*cfi_array)->bus = bus;
	(*cfi_array)->address = adr;
	if (bus_area( bus, adr, &area ) != 0)
		return -8;		/* bus width detection failed */
	bw = area.width;
	if (bw != 8 && bw != 16 && bw != 32)
		return -3;		/* invalid bus width */
	(*cfi_array)->bus_width = ba = bw / 8;
	(*cfi_array)->cfi_chips = calloc( ba, sizeof (cfi_chip_t *) );
	if (!(*cfi_array)->cfi_chips)
		return -2;		/* out of memory */

	/* Query flash. */
	bus_write( bus, 0x0, 0xf0 );
	bus_write( bus, 0xaaa, 0xaa );
	bus_write( bus, 0x555, 0x55 );
	bus_write( bus, 0xaaa, 0x90 );
	mid = bus_read( bus, 0x0);
	did = bus_read( bus, 0x2);
	bus_write( bus, 0x0, 0xf0 );

	printf( "%s: mid %x, did %x\n", __FUNCTION__, mid, did );
	if (mid != 0x01)
		return -1;

	(*cfi_array)->cfi_chips[0] = calloc( 1, sizeof (cfi_chip_t) );
	if (!(*cfi_array)->cfi_chips[0])
		return -2;	/* out of memory */

	cfi = &(*cfi_array)->cfi_chips[0]->cfi;

	cfi->identification_string.pri_id_code = CFI_VENDOR_AMD_SCS;
	cfi->identification_string.pri_vendor_tbl = NULL;
	cfi->identification_string.alt_id_code = 0;
	cfi->identification_string.alt_vendor_tbl = NULL;

	cfi->device_geometry.number_of_erase_regions = 4;
	cfi->device_geometry.erase_block_regions =
		malloc( cfi->device_geometry.number_of_erase_regions * sizeof (cfi_erase_block_region_t) );
	if (!cfi->device_geometry.erase_block_regions)
		return -2;	/* out of memory */

	cfi->device_geometry.erase_block_regions[0].erase_block_size = 16 * 1024;
	cfi->device_geometry.erase_block_regions[0].number_of_erase_blocks = 1;
	cfi->device_geometry.erase_block_regions[1].erase_block_size = 8 * 1024;
	cfi->device_geometry.erase_block_regions[1].number_of_erase_blocks = 2;
	cfi->device_geometry.erase_block_regions[2].erase_block_size = 32 * 1024;
	cfi->device_geometry.erase_block_regions[2].number_of_erase_blocks = 1;
	cfi->device_geometry.erase_block_regions[3].erase_block_size = 64 * 1024;
	cfi->device_geometry.erase_block_regions[3].number_of_erase_blocks = 15;

	return 0;
}
