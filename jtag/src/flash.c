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
#include <flash/intel.h>

#include "part.h"

#include "sa1110.h"
#include "pxa250.h"

void (*bus_read_start)( parts *, uint32_t );
uint32_t (*bus_read_next)( parts *, uint32_t );
uint32_t (*bus_read_end)( parts * );
uint32_t (*bus_read)( parts *, uint32_t );
void (*bus_write)( parts *, uint32_t, uint32_t );

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
	uint8_t boot_sel;
	int o = 0;
	int d = 0;
	uint32_t adr;
#define	D_SA1110	1
#define	D_PXA250	2

	if (strcmp( p->part, "SA1110" ) == 0) {
		printf( "SA1110 detected\n" );
		d = D_SA1110;
		bus_read_start = sa1110_bus_read_start;
		bus_read_next = sa1110_bus_read_next;
		bus_read_end = sa1110_bus_read_end;
		bus_read = sa1110_bus_read;
		bus_write = sa1110_bus_write;
	}
	if (strcmp( p->part, "PXA250" ) == 0) {
		printf( "PXA250 detected\n" );
		d = D_PXA250;
		bus_read_start = pxa250_bus_read_start;
		bus_read_next = pxa250_bus_read_next;
		bus_read_end = pxa250_bus_read_end;
		bus_read = pxa250_bus_read;
		bus_write = pxa250_bus_write;
	}

	if (!d) {
		printf( "Error: Only PXA250/SA1110 devices supported!\n" );
		return;
	}

	printf( "Note: Supported configuration is 2 x 16 bit only\n" );

	switch (d) {
		case D_SA1110:
			if (part_get_signal( p, "ROM_SEL" )) {
				printf( "ROM_SEL: 32 bits\n" );
				o = 2;
			} else {
				printf( "ROM_SEL: 16 bits\n" );
				o = 1;
			}
			break;
		case D_PXA250:
			boot_sel = (part_get_signal( p, "BOOT_SEL[2]" ) << 2) | (part_get_signal( p, "BOOT_SEL[1]" ) << 1) | part_get_signal( p, "BOOT_SEL[0]" );

			/* see Table 6-36. in [2] */
			switch (boot_sel) {
				case 0:
					printf( "BOOT_SEL: Asynchronous 32-bit ROM\n" );
					o = 2;
					break;
				case 1:
					printf( "BOOT_SEL: Asynchronous 16-bit ROM\n" );
					o = 1;
					break;
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					printf( "TODO - BOOT_SEL\n" );
					return;
				default:
					printf( "BUG in code, file %s, line %d.\n", __FILE__, __LINE__ );
					return;
			}
			break;
		default:
			printf( "Unknown device!!!\n" );
			break;
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
