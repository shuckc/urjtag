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
 * [2] Intel Corporation, "Common Flash Interface (CFI) and Command Sets
 *     Application Note 646", April 2000, Order Number: 292204-004
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <flash/cfi.h>

#include "bus.h"

/* function to cover 2x16 and 1x16 modes */
#define BW16(x) ( (bus_width(ps) == 16) ? x : ( (x<<16) | x ) )

static uint16_t
read2( parts *ps, uint32_t adr, int o )
{
	uint16_t r;

	bus_read_start( ps, adr << o );
	r = bus_read_next( ps, (adr + 1) << o );
	return ((bus_read_end( ps ) & 0xFF) << 8) | (r & 0xFF);
}

cfi_query_structure_t *
detect_cfi( parts *ps )
{
	cfi_query_structure_t *cfi;
	int o = 2;
	uint32_t tmp;

	if (bus_width(ps) == 16)
		o = 1;

	/* detect CFI capable devices - see Table 1 in [1] */
	bus_write( ps, CFI_CMD_QUERY_OFFSET << o, BW16(CFI_CMD_QUERY) );
	if (bus_read( ps, CFI_QUERY_ID_OFFSET << o ) != BW16('Q')) {
		printf( "No CFI device detected (Q)!\n" );
		return NULL;
	}
	if (bus_read( ps, (CFI_QUERY_ID_OFFSET + 1) << o ) != BW16('R')) {
		printf( "No CFI device detected (R)!\n" );
		return NULL;
	}
	if (bus_read( ps, (CFI_QUERY_ID_OFFSET + 2) << o ) != BW16('Y')) {
		printf( "No CFI device detected (Y)!\n" );
		return NULL;
	}

	printf( "\n%d x 16 bit CFI devices detected (QRY ok)!\n\n", o);

	cfi = malloc( sizeof *cfi );
	if (!cfi)
		return NULL;

	/* TODO: Low chip only (bits 15:0) */

	/* Identification string - see Table 6 in [1] */
	cfi->identification_string.pri_id_code = read2( ps, PRI_VENDOR_ID_OFFSET, o );
	cfi->identification_string.pri_vendor_tbl = NULL;
	cfi->identification_string.alt_id_code = read2( ps, ALT_VENDOR_ID_OFFSET, o );
	cfi->identification_string.alt_vendor_tbl = NULL;

	/* System interface information - see Table 7 in [1] */
	tmp = bus_read( ps, VCC_MIN_WEV_OFFSET << o );
	cfi->system_interface_info.vcc_min_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
	tmp = bus_read( ps, VCC_MAX_WEV_OFFSET << o );
	cfi->system_interface_info.vcc_max_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
	tmp = bus_read( ps, VPP_MIN_WEV_OFFSET << o );
	cfi->system_interface_info.vpp_min_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
	tmp = bus_read( ps, VPP_MAX_WEV_OFFSET << o );
	cfi->system_interface_info.vpp_max_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;

	/* TODO: Add out of range checks for timeouts */
	tmp = bus_read( ps, TYP_SINGLE_WRITE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.typ_single_write_timeout = tmp ? (1 << tmp) : 0;

	tmp = bus_read( ps, TYP_BUFFER_WRITE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.typ_buffer_write_timeout = tmp ? (1 << tmp) : 0;

	tmp = bus_read( ps, TYP_BLOCK_ERASE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.typ_block_erase_timeout = tmp ? (1 << tmp) : 0;

	tmp = bus_read( ps, TYP_CHIP_ERASE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.typ_chip_erase_timeout = tmp ? (1 << tmp) : 0;

	tmp = bus_read( ps, MAX_SINGLE_WRITE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.max_single_write_timeout =
			(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_single_write_timeout;

	tmp = bus_read( ps, MAX_BUFFER_WRITE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.max_buffer_write_timeout =
			(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_buffer_write_timeout;

	tmp = bus_read( ps, MAX_BLOCK_ERASE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.max_block_erase_timeout =
			(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_block_erase_timeout;

	tmp = bus_read( ps, MAX_CHIP_ERASE_TIMEOUT_OFFSET << o ) & 0xFF;
	cfi->system_interface_info.max_chip_erase_timeout =
			(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_chip_erase_timeout;

	/* Device geometry - see Table 8 in [1] */
	/* TODO: Add out of range check */
	cfi->device_geometry.device_size = 1 << (bus_read( ps, DEVICE_SIZE_OFFSET << o ) & 0xFF);

	cfi->device_geometry.device_interface = read2( ps, FLASH_DEVICE_INTERFACE_OFFSET, o );

	/* TODO: Add out of range check */
	cfi->device_geometry.max_bytes_write = 1 << read2( ps, MAX_BYTES_WRITE_OFFSET, o );

	tmp = bus_read( ps, NUMBER_OF_ERASE_REGIONS_OFFSET << o ) & 0xFF;
	cfi->device_geometry.number_of_erase_regions = tmp;

	cfi->device_geometry.erase_block_regions = malloc( tmp * sizeof (cfi_erase_block_region_t) );
	if (!cfi->device_geometry.erase_block_regions) {
		free( cfi );
		return NULL;
	}
	
	{
		int a = ERASE_BLOCK_REGION_OFFSET;
		int i;

		for (i = 0, a = ERASE_BLOCK_REGION_OFFSET; i < tmp; i++, a += 4) {
			uint32_t y = read2( ps, a, o );
			uint32_t z = read2( ps, a + 2, o ) << 8;
			if (z == 0)
				z = 128;
			cfi->device_geometry.erase_block_regions[i].erase_block_size = z;
			cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks = y + 1;
		}
	}

	/* TODO: Intel Primary Algorithm Extended Query Table - see Table 5. in [2] */

	/* Read Array */
	bus_write( ps, 0, (CFI_CMD_READ_ARRAY1 << 16) | CFI_CMD_READ_ARRAY1 );

	return cfi;
}
