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
#include <string.h>
#include <flash/intel.h>

#include "part.h"
#include "bus.h"

int flash_erase_block( parts *ps, uint32_t adr );
int flash_unlock_block( parts *ps, uint32_t adr );
int flash_program( parts *ps, uint32_t adr, uint32_t data );
int flash_erase_block32( parts *ps, uint32_t adr );
int flash_unlock_block32( parts *ps, uint32_t adr );
int flash_program32( parts *ps, uint32_t adr, uint32_t data );

void
flashmem( parts *ps, FILE *f )
{
	part *p = ps->parts[0];
	int o = 0;
	uint32_t adr;

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

	flash_unlock_block32( ps, 0 );
	printf( "block unlocked\n" );
	printf( "erasing block 0: %d\n", flash_erase_block32( ps, 0 ) );

	printf( "program:\n" );
	adr = 0;
	while (!feof( f )) {
		uint32_t data;
		printf( "addr: 0x%08X\r", adr );
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
	adr = 0;
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
