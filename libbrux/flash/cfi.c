/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
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

#include <brux/cfi.h>
#include <brux/bus.h>

void
cfi_array_free( cfi_array_t *cfi_array )
{
	if (!cfi_array)
		return;

	if (cfi_array->cfi_chips) {
		int i;

		for (i = 0; i < cfi_array->bus_width; i++) {
			if (!cfi_array->cfi_chips[i])
				continue;

			free( cfi_array->cfi_chips[i]->cfi.device_geometry.erase_block_regions );
			free( cfi_array->cfi_chips[i] );
		}
		free( cfi_array->cfi_chips );
	}

	free( cfi_array );
}

int
cfi_detect( bus_t *bus, uint32_t adr, cfi_array_t **cfi_array )
{
	unsigned int bw;		/* bus width */
	unsigned int d;			/* data offset */
	int ba;				/* bus width address multiplier */
	int ma;				/* flash mode address multiplier */

	if (!cfi_array || !bus)
		return -1;		/* invalid parameters */

	*cfi_array = calloc( 1, sizeof (cfi_array_t) );
	if (!*cfi_array)
		return -2;		/* out of memory */

	(*cfi_array)->bus = bus;
	(*cfi_array)->address = adr;
	bw = bus_width( bus, adr );
	if (bw != 8 && bw != 16 && bw != 32)
		return -3;		/* invalid bus width */
	(*cfi_array)->bus_width = ba = bw / 8;
	(*cfi_array)->cfi_chips = calloc( ba, sizeof (cfi_chip_t *) );
	if (!(*cfi_array)->cfi_chips)
		return -2;		/* out of memory */

	for (d = 0; d < bw; d += 8) {
#define	A(off)			(adr + (off) * ba * ma)
#define	D(data)			((data) << d)
#define	gD(data)		(((data) >> d) & 0xFF)
#define	read1(off)		gD(bus_read( bus, A(off) ))
#define	read2(off)		(bus_read_start( bus, A(off) ), gD(bus_read_next( bus, A((off) + 1) )) | gD(bus_read_end( bus )) << 8)
#define	write1(off,data)	bus_write( bus, A(off), D(data) )

		cfi_query_structure_t *cfi;
		uint32_t tmp;
		int ret = -4;		/* CFI not detected (Q) */

		/* detect CFI capable devices - see Table 1 in [1] */
		for (ma = 1; ma <= 4; ma *= 2) {
			write1( CFI_CMD_QUERY_OFFSET, CFI_CMD_QUERY );

			if (read1(CFI_QUERY_ID_OFFSET) == 'Q') {
				ret = -5;	/* CFI not detected (R) */
				if (read1(CFI_QUERY_ID_OFFSET + 1) == 'R')
					break;
			}

			write1( 0, CFI_CMD_READ_ARRAY1 );
		}

		if (ma > 4)
			return ret;	/* CFI not detected (Q or R) */

		if (read1(CFI_QUERY_ID_OFFSET + 2) != 'Y') {
			write1( 0, CFI_CMD_READ_ARRAY1 );
			return -6;	/* CFI not detected (Y) */
		}

		(*cfi_array)->cfi_chips[d / 8] = calloc( 1, sizeof (cfi_chip_t) );
		if (!(*cfi_array)->cfi_chips[d / 8]) {
			write1( 0, CFI_CMD_READ_ARRAY1 );
			return -2;	/* out of memory */
		}
		cfi = &(*cfi_array)->cfi_chips[d / 8]->cfi;

		/* Identification string - see Table 6 in [1] */
		cfi->identification_string.pri_id_code = read2(PRI_VENDOR_ID_OFFSET);
		cfi->identification_string.pri_vendor_tbl = NULL;
		cfi->identification_string.alt_id_code = read2(ALT_VENDOR_ID_OFFSET);
		cfi->identification_string.alt_vendor_tbl = NULL;

		/* System interface information - see Table 7 in [1] */
		tmp = read1(VCC_MIN_WEV_OFFSET);
		cfi->system_interface_info.vcc_min_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
		tmp = read1(VCC_MAX_WEV_OFFSET);
		cfi->system_interface_info.vcc_max_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
		tmp = read1(VPP_MIN_WEV_OFFSET);
		cfi->system_interface_info.vpp_min_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;
		tmp = read1(VPP_MAX_WEV_OFFSET);
		cfi->system_interface_info.vpp_max_wev = ((tmp >> 4) & 0xF) * 1000 + (tmp & 0xF) * 100;

		/* TODO: Add out of range checks for timeouts */
		tmp = read1(TYP_SINGLE_WRITE_TIMEOUT_OFFSET);
		cfi->system_interface_info.typ_single_write_timeout = tmp ? (1 << tmp) : 0;

		tmp = read1(TYP_BUFFER_WRITE_TIMEOUT_OFFSET);
		cfi->system_interface_info.typ_buffer_write_timeout = tmp ? (1 << tmp) : 0;

		tmp = read1(TYP_BLOCK_ERASE_TIMEOUT_OFFSET);
		cfi->system_interface_info.typ_block_erase_timeout = tmp ? (1 << tmp) : 0;

		tmp = read1(TYP_CHIP_ERASE_TIMEOUT_OFFSET);
		cfi->system_interface_info.typ_chip_erase_timeout = tmp ? (1 << tmp) : 0;

		tmp = read1(MAX_SINGLE_WRITE_TIMEOUT_OFFSET);
		cfi->system_interface_info.max_single_write_timeout =
				(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_single_write_timeout;

		tmp = read1(MAX_BUFFER_WRITE_TIMEOUT_OFFSET);
		cfi->system_interface_info.max_buffer_write_timeout =
				(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_buffer_write_timeout;

		tmp = read1(MAX_BLOCK_ERASE_TIMEOUT_OFFSET);
		cfi->system_interface_info.max_block_erase_timeout =
				(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_block_erase_timeout;

		tmp = read1(MAX_CHIP_ERASE_TIMEOUT_OFFSET);
		cfi->system_interface_info.max_chip_erase_timeout =
				(tmp ? (1 << tmp) : 0) * cfi->system_interface_info.typ_chip_erase_timeout;

		/* Device geometry - see Table 8 in [1] */
		/* TODO: Add out of range check */
		cfi->device_geometry.device_size = 1 << read1(DEVICE_SIZE_OFFSET);

		cfi->device_geometry.device_interface = read2(FLASH_DEVICE_INTERFACE_OFFSET);

		/* TODO: Add out of range check */
		cfi->device_geometry.max_bytes_write = 1 << read2(MAX_BYTES_WRITE_OFFSET);

		tmp = cfi->device_geometry.number_of_erase_regions = read1(NUMBER_OF_ERASE_REGIONS_OFFSET);

		cfi->device_geometry.erase_block_regions = malloc( tmp * sizeof (cfi_erase_block_region_t) );
		if (!cfi->device_geometry.erase_block_regions) {
			write1( 0, CFI_CMD_READ_ARRAY1 );
			return -2;	/* out of memory */
		}
		
		{
			int a;
			int i;

			for (i = 0, a = ERASE_BLOCK_REGION_OFFSET; i < tmp; i++, a += 4) {
				uint32_t y = read2(a);
				uint32_t z = read2(a + 2) << 8;
				if (z == 0)
					z = 128;
				cfi->device_geometry.erase_block_regions[i].erase_block_size = z;
				cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks = y + 1;
			}
		}

		/* TODO: Intel Primary Algorithm Extended Query Table - see Table 5. in [2] */

		/* Read Array */
		write1( 0, CFI_CMD_READ_ARRAY1 );

#undef A
#undef D
#undef gD
#undef read1
#undef read2
#undef write1

		switch (cfi->device_geometry.device_interface) {
			case CFI_INTERFACE_X8:
				if (ma != 1)
					return -7;		/* error in device detection */
				(*cfi_array)->cfi_chips[d / 8]->width = 1;
				break;
			case CFI_INTERFACE_X16:
				if (ma != 1)
					return -7;		/* error in device detection */
				(*cfi_array)->cfi_chips[d / 8]->width = 2;
				d += 8;
				break;
			case CFI_INTERFACE_X8_X16:
				if (ma != 1 && ma != 2)
					return -7;		/* error in device detection */
				(*cfi_array)->cfi_chips[d / 8]->width = 2 / ma;
				if (ma == 1)
					d += 8;
				break;
			case CFI_INTERFACE_X32:
				if (ma != 1)
					return -7;		/* error in device detection */
				(*cfi_array)->cfi_chips[d / 8]->width = 4;
				d += 24;
				break;
			case CFI_INTERFACE_X16_X32:
				if (ma != 1 && ma != 2)
					return -7;		/* error in device detection */
				(*cfi_array)->cfi_chips[d / 8]->width = 4 / ma;
				if (ma == 1)
					d += 24;
				else
					d += 8;
				break;
			default:
				return -7;		/* error in device detection */
		}
	}

	return 0;
}
