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

#include "part.h"
#include "bus.h"

int flash_erase_block( parts *ps, uint32_t adr );
int flash_unlock_block( parts *ps, uint32_t adr );
int flash_program( parts *ps, uint32_t adr, uint32_t data );
int flash_erase_block32( parts *ps, uint32_t adr );
int flash_unlock_block32( parts *ps, uint32_t adr );
int flash_program32( parts *ps, uint32_t adr, uint32_t data );

cfi_query_structure_t *detect_cfi( parts *ps );

void
flashmsbin( parts *ps, FILE *f )
{
	part *p = ps->parts[0];
	int o = 0;
	uint32_t adr;
	cfi_query_structure_t *cfi;

	printf( "Note: Supported configuration is 2 x 16 bit only\n" );

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

	cfi = detect_cfi( ps );

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
			flash_unlock_block32( ps, adr );
			printf( "block %d unlocked\n", first );
			printf( "erasing block %d: %d\n", first, flash_erase_block32( ps, adr ) );
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
			fread( &data, sizeof data, 1, f );
			if (flash_program32( ps, a, data )) {
				printf( "\nflash error\n" );
				return;
			}
			a += 4;
			l -= 4;
		}
	}
	printf( "\n" );

	/* Read Array */
	bus_write( ps, 0 << o, 0x00FF00FF );

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
			fread( &data, sizeof data, 1, f );
			readed = bus_read( ps, a );
			if (data != readed) {
				printf( "\nverify error: 0x%08X vs. 0x%08X\n", readed, data );
				return;
			}
			a += 4;
			l -= 4;
		}
	}
	printf( "\n" );

	printf( "Done.\n" );
}

void
flashmem( parts *ps, FILE *f, uint32_t addr )
{
	part *p = ps->parts[0];
	int o = 0;
	uint32_t adr;
	cfi_query_structure_t *cfi;
	int *erased;
	int i;

	printf( "Note: Supported configuration is 2 x 16 bit only\n" );

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

	cfi = detect_cfi( ps );
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
		int block_no = adr / (cfi->device_geometry.erase_block_regions[0].erase_block_size * 2);
		printf( "addr: 0x%08X\r", adr );

		if (!erased[block_no]) {
			flash_unlock_block32( ps, adr );
			printf( "block %d unlocked\n", block_no );
			printf( "erasing block %d: %d\n", block_no, flash_erase_block32( ps, adr ) );
			erased[block_no] = 1;
		}

		fread( &data, sizeof data, 1, f );
		if (flash_program32( ps, adr, data )) {
			printf( "\nflash error\n" );
			return;
		}
		adr += 4;
	}
	printf( "\n" );

	/* Read Array */
	bus_write( ps, 0 << o, 0x00FF00FF );

	fseek( f, 0, SEEK_SET );
	printf( "verify:\n" );
	adr = addr;
	while (!feof( f )) {
		uint32_t data;
		uint32_t readed;
		printf( "addr: 0x%08X\r", adr );
		fread( &data, sizeof data, 1, f );
		readed = bus_read( ps, adr );
		if (data != readed) {
			printf( "\nverify error: 0x%08X vs. 0x%08X\n", readed, data );
			return;
		}
		adr += 4;
	}
	printf( "\nDone.\n" );

	free( erased );
}

#define	CFI_INTEL_ERROR_UNKNOWN				1
#define	CFI_INTEL_ERROR_UNSUPPORTED			2
#define	CFI_INTEL_ERROR_LOW_VPEN			3
#define	CFI_INTEL_ERROR_BLOCK_LOCKED			4
#define	CFI_INTEL_ERROR_INVALID_COMMAND_SEQUENCE	5

int
flash_erase_block( parts *ps, uint32_t adr )
{
	uint16_t sr;

	bus_write( ps, 0, CFI_INTEL_CMD_CLEAR_STATUS_REGISTER );
	bus_write( ps, adr, CFI_INTEL_CMD_BLOCK_ERASE );
	bus_write( ps, adr, CFI_INTEL_CMD_CONFIRM );

	while (!((sr = bus_read( ps, 0 ) & 0xFE) & CFI_INTEL_SR_READY)) ; 		/* TODO: add timeout */

	switch (sr & ~CFI_INTEL_SR_READY) {
		case 0:
			return 0;
		case CFI_INTEL_SR_ERASE_ERROR | CFI_INTEL_SR_PROGRAM_ERROR:
			return CFI_INTEL_ERROR_INVALID_COMMAND_SEQUENCE;
		case CFI_INTEL_SR_ERASE_ERROR | CFI_INTEL_SR_VPEN_ERROR:
			return CFI_INTEL_ERROR_LOW_VPEN;
		case CFI_INTEL_SR_ERASE_ERROR | CFI_INTEL_SR_BLOCK_LOCKED:
			return CFI_INTEL_ERROR_BLOCK_LOCKED;
		default:
			break;
	}

	return CFI_INTEL_ERROR_UNKNOWN;
}

int
flash_unlock_block( parts *ps, uint32_t adr )
{
	uint16_t sr;

	bus_write( ps, 0, CFI_INTEL_CMD_CLEAR_STATUS_REGISTER );
	bus_write( ps, adr, CFI_INTEL_CMD_LOCK_SETUP );
	bus_write( ps, adr, CFI_INTEL_CMD_UNLOCK_BLOCK );

	while (!((sr = bus_read( ps, 0 ) & 0xFE) & CFI_INTEL_SR_READY)) ; 		/* TODO: add timeout */

	if (sr != CFI_INTEL_SR_READY)
		return CFI_INTEL_ERROR_UNKNOWN;
	else
		return 0;
}

int
flash_program( parts *ps, uint32_t adr, uint32_t data )
{
	uint16_t sr;

	bus_write( ps, 0, CFI_INTEL_CMD_CLEAR_STATUS_REGISTER );
	bus_write( ps, adr, CFI_INTEL_CMD_PROGRAM1 );
	bus_write( ps, adr, data );

	while (!((sr = bus_read( ps, 0 ) & 0xFE) & CFI_INTEL_SR_READY)) ; 		/* TODO: add timeout */

	if (sr != CFI_INTEL_SR_READY)
		return CFI_INTEL_ERROR_UNKNOWN;
	else
		return 0;
}
int
flash_erase_block32( parts *ps, uint32_t adr )
{
	uint32_t sr;

	bus_write( ps, 0, (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER << 16) | CFI_INTEL_CMD_CLEAR_STATUS_REGISTER );
	bus_write( ps, adr, (CFI_INTEL_CMD_BLOCK_ERASE << 16) | CFI_INTEL_CMD_BLOCK_ERASE );
	bus_write( ps, adr, (CFI_INTEL_CMD_CONFIRM << 16) | CFI_INTEL_CMD_CONFIRM );

	while (((sr = bus_read( ps, 0 ) & 0x00FE00FE) & ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) ; 		/* TODO: add timeout */

	if (sr != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY))
		return CFI_INTEL_ERROR_UNKNOWN;
	else
		return 0;
}

int
flash_unlock_block32( parts *ps, uint32_t adr )
{
	uint32_t sr;

	bus_write( ps, 0, (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER << 16) | CFI_INTEL_CMD_CLEAR_STATUS_REGISTER );
	bus_write( ps, adr, (CFI_INTEL_CMD_LOCK_SETUP << 16) | CFI_INTEL_CMD_LOCK_SETUP );
	bus_write( ps, adr, (CFI_INTEL_CMD_UNLOCK_BLOCK << 16) | CFI_INTEL_CMD_UNLOCK_BLOCK );

	while (((sr = bus_read( ps, 0 ) & 0x00FE00FE) & ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) ; 		/* TODO: add timeout */

	if (sr != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY))
		return CFI_INTEL_ERROR_UNKNOWN;
	else
		return 0;
}

int
flash_program32( parts *ps, uint32_t adr, uint32_t data )
{
	uint32_t sr;

	bus_write( ps, 0, (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER << 16) | CFI_INTEL_CMD_CLEAR_STATUS_REGISTER );
	bus_write( ps, adr, (CFI_INTEL_CMD_PROGRAM1 << 16) | CFI_INTEL_CMD_PROGRAM1 );
	bus_write( ps, adr, data );

	while (((sr = bus_read( ps, 0 ) & 0x00FE00FE) & ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY)) ; 		/* TODO: add timeout */

	if (sr != ((CFI_INTEL_SR_READY << 16) | CFI_INTEL_SR_READY))
		return CFI_INTEL_ERROR_UNKNOWN;
	else
		return 0;
}
