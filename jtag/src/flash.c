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
 * [1] Advanced Micro Devices, "Common Flash Memory Interface Specification Release 2.0",
 *     December 1, 2001
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [3] Intel Corporation, "Common Flash Interface (CFI) and Command Sets
 *     Application Note 646", April 2000, Order Number: 292204-004
 * [4] Advanced Micro Devices, "Common Flash Memory Interface Publication 100 Vendor & Device
 *     ID Code Assignments", December 1, 2001, Volume Number: 96.1
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <flash/cfi.h>
#include <flash/intel.h>

#include <arpa/inet.h>
/* for ntohs */

#include "part.h"
#include "bus.h"
#include "flash.h"

extern flash_driver_t amd_32_flash_driver;
extern flash_driver_t intel_32_flash_driver;

flash_driver_t *flash_drivers[] = {
	&amd_32_flash_driver,
	&intel_32_flash_driver,
	NULL
};

flash_driver_t *flash_driver = NULL;

void
set_flash_driver( parts *ps, cfi_query_structure_t *cfi )
{
	int i;
	flash_driver = NULL;

	for (i = 0; flash_drivers[i] != NULL; i++)
		if (flash_drivers[i]->flash_autodetect( ps, cfi )) {
			flash_driver = flash_drivers[i];
			return;
		}

	printf( "unknown flash - vendor id: %d (0x%04x)\n",
		cfi->identification_string.pri_id_code,
		cfi->identification_string.pri_id_code );
}

/* check for flashmem - set driver */
void
flashcheck( parts *ps, cfi_query_structure_t **cfi )
{
	part *p = ps->parts[0];
	int o = 0;
	flash_driver = NULL;

	printf( "Note: Supported configuration is 2 x 16 bit or 1 x 16 bit only\n" );

	switch (bus_width( ps )) {
		case 16:
			o = 1;
			break;
		case 32:
			o = 2;
			break;
		default:
			printf( "Error: Unknown bus width!\n" );
			return;
	}

	/* EXTEST */
	part_set_instruction( p, "EXTEST" );
	parts_shift_instructions( ps );

	*cfi = detect_cfi( ps );
	if (!*cfi) {
		printf( "Flash not found!\n" );
		return;
	}
	set_flash_driver( ps, *cfi );
	if (!flash_driver) {
		printf( "Flash not supported!\n" );
		return;
	}
	flash_driver->flash_print_info( ps );
}

void
flashmsbin( parts *ps, FILE *f )
{
	uint32_t adr;
	cfi_query_structure_t *cfi = 0;

	flashcheck( ps, &cfi );
	if (!cfi || !flash_driver) {
		printf( "no flash driver found\n" );
		return;
	}

	/* test sync bytes */
	{
		char sync[8];
		fread( &sync, sizeof (char), 7, f );
		sync[7] = '\0';
		if (strcmp( "B000FF\n", sync ) != 0) {
			printf( "Invalid sync sequence!\n" );
			return;
		}
	}

	/* erase memory blocks */
	{
		uint32_t start;
		uint32_t len;
		int first, last;

		fread( &start, sizeof start, 1, f );
		fread( &len, sizeof len, 1, f );
		first = start / (cfi->device_geometry.erase_block_regions[0].erase_block_size * 2);
		last = (start + len - 1) / (cfi->device_geometry.erase_block_regions[0].erase_block_size * 2);
		for (; first <= last; first++) {
			adr = first * cfi->device_geometry.erase_block_regions[0].erase_block_size * 2;
			flash_unlock_block( ps, adr );
			printf( "block %d unlocked\n", first );
			printf( "erasing block %d: %d\n", first, flash_erase_block( ps, adr ) );
		}
	}

	printf( "program:\n" );
	for (;;) {
		uint32_t a, l, c;

		fread( &a, sizeof a, 1, f );
		fread( &l, sizeof l, 1, f );
		fread( &c, sizeof c, 1, f );
		if (feof( f )) {
			printf( "Error: premature end of file\n" );
			return;
		}
		printf( "record: start = 0x%08X, len = 0x%08X, checksum = 0x%08X\n", a, l, c );
		if ((a == 0) && (c == 0))
			break;
		if (l & 3) {
			printf( "Error: Invalid record length!\n" );
			return;
		}

		while (l) {
			uint32_t data;

			printf( "addr: 0x%08X\r", a );
			fflush(stdout);
			fread( &data, sizeof data, 1, f );
			if (flash_program( ps, a, data )) {
				printf( "\nflash error 1\n" );
				return;
			}
			a += 4;
			l -= 4;
		}
	}
	printf( "\n" );

	flash_readarray(ps);

	fseek( f, 15, SEEK_SET );
	printf( "verify:\n" );

	for (;;) {
		uint32_t a, l, c;

		fread( &a, sizeof a, 1, f );
		fread( &l, sizeof l, 1, f );
		fread( &c, sizeof c, 1, f );
		if (feof( f )) {
			printf( "Error: premature end of file\n" );
			return;
		}
		printf( "record: start = 0x%08X, len = 0x%08X, checksum = 0x%08X\n", a, l, c );
		if ((a == 0) && (c == 0))
			break;
		if (l & 3) {
			printf( "Error: Invalid record length!\n" );
			return;
		}

		while (l) {
			uint32_t data, readed;

			printf( "addr: 0x%08X\r", a );
			fflush( stdout );
			fread( &data, sizeof data, 1, f );
			readed = bus_read( ps, a );
			if (data != readed) {
				printf( "\nverify error: 0x%08X vs. 0x%08X at addr %08X\n", 
					readed, data, a );
				return;
			}
			a += 4;
			l -= 4;
		}
	}
	printf( "\n" );

	/* BYPASS */
	parts_set_instruction( ps, "BYPASS" );
	parts_shift_instructions( ps );

	printf( "Done.\n" );
}

void
flashmem( parts *ps, FILE *f, uint32_t addr )
{
	uint32_t adr;
	cfi_query_structure_t *cfi = NULL;
	int *erased;
	int i;

	flashcheck( ps, &cfi );
	if (!cfi || !flash_driver) {
		printf( "no flash driver found\n" );
		return;
	}

	erased = malloc( cfi->device_geometry.erase_block_regions[0].number_of_erase_blocks * sizeof *erased );
	if (!erased) {
		printf( "Out of memory!\n" );
		return;
	}
	for (i = 0; i < cfi->device_geometry.erase_block_regions[0].number_of_erase_blocks; i++)
		erased[i] = 0;

	printf( "program:\n" );
	adr = addr;
	while (!feof( f )) {
		uint32_t data;
#define BSIZE 4096
		char b[BSIZE];
		int bc = 0, bn = 0;
		int block_no = adr / (cfi->device_geometry.erase_block_regions[0].erase_block_size * flash_driver->buswidth / 2);
		printf( "addr: 0x%08X\r", adr );
		fflush( stdout );

		if (!erased[block_no]) {
			flash_unlock_block( ps, adr );
			printf( "block %d unlocked\n", block_no );
			printf( "erasing block %d: %d\n", block_no, flash_erase_block( ps, adr ) );
			erased[block_no] = 1;
		}

		bn = fread( b, 1, BSIZE, f );
		printf("addr 0x%08X (n is %d)\n", adr, bn);
		for (bc = 0; bc < bn; bc += flash_driver->buswidth) {
			if (flash_driver->buswidth == 2)
				data = htons( *((uint16_t *) &b[bc]) );
			else
				data = * ((uint32_t *) &b[bc]);
			if (flash_program( ps, adr, data )) {
				printf( "\nflash error 2\n" );
				return;
			}
			adr += flash_driver->buswidth;
		}
	}
	printf( "\n" );

	flash_readarray( ps );

	if (flash_driver->buswidth == 2) {			/* TODO: not available in 1 x 16 bit mode */
	fseek( f, 0, SEEK_SET );
	printf( "verify:\n" );
	adr = addr;
	while (!feof( f )) {
		uint32_t data;
		uint32_t readed;
		printf( "addr: 0x%08X\r", adr );
		fflush( stdout );
		fread( &data, flash_driver->buswidth, 1, f );
		readed = bus_read( ps, adr );
		if (data != readed) {
			printf( "\nverify error: 0x%08X vs. 0x%08X at addr %08X\n", readed, data, adr );
			return;
		}
		adr += flash_driver->buswidth;
	}
	printf( "\nDone.\n" );
	} else
		printf( "TODO: Verify is not available in 1 x 16 bit mode.\n" );

	/* BYPASS */
	parts_set_instruction( ps, "BYPASS" );
	parts_shift_instructions( ps );

	free( erased );
}




