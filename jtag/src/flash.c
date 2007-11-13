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
 * Modified by Ajith Kumar P.C <ajithpc@kila.com>, 20/09/2006
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

#include "sysdep.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <flash/cfi.h>
#include <flash/intel.h>
#include <brux/cfi.h>

#include "bus.h"
#include "flash.h"
#include "jtag.h"

extern flash_driver_t amd_32_flash_driver;
extern flash_driver_t amd_16_flash_driver;
extern flash_driver_t amd_8_flash_driver;
extern flash_driver_t intel_32_flash_driver;
extern flash_driver_t intel_16_flash_driver;
extern flash_driver_t intel_8_flash_driver;
extern flash_driver_t amd_29xx040_flash_driver;	//20/09/2006

flash_driver_t *flash_drivers[] = {
	&amd_32_flash_driver,
	&amd_16_flash_driver,
	&amd_8_flash_driver,
	&intel_32_flash_driver,
	&intel_16_flash_driver,
	&intel_8_flash_driver,
	&amd_29xx040_flash_driver,	//20/09/2006
	NULL
};

extern cfi_array_t *cfi_array;
static flash_driver_t *flash_driver = NULL;

extern int amd_detect(bus_t *bus, cfi_array_t **cfi_array ); //Ajit

static void
set_flash_driver( void )
{
	int i;
	cfi_query_structure_t *cfi;

	flash_driver = NULL;
	if (cfi_array == NULL)
		return;
	cfi = &cfi_array->cfi_chips[0]->cfi;

	for (i = 0; flash_drivers[i] != NULL; i++)
		if (flash_drivers[i]->autodetect( cfi_array )) {
			flash_driver = flash_drivers[i];
			flash_driver->print_info( cfi_array );
			return;
		}

	printf( _("unknown flash - vendor id: %d (0x%04x)\n"),
		cfi->identification_string.pri_id_code,
		cfi->identification_string.pri_id_code );

	printf( _("Flash not supported!\n") );
}

void
flashmsbin( bus_t *bus, FILE *f )
{
	uint32_t adr;
	cfi_query_structure_t *cfi;

	set_flash_driver();
	if (!cfi_array || !flash_driver) {
		printf( _("no flash driver found\n") );
		return;
	}
	cfi = &cfi_array->cfi_chips[0]->cfi;

	/* test sync bytes */
	{
		char sync[8];
		fread( &sync, sizeof (char), 7, f );
		sync[7] = '\0';
		if (strcmp( "B000FF\n", sync ) != 0) {
			printf( _("Invalid sync sequence!\n") );
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
			flash_driver->unlock_block( cfi_array, adr );
			printf( _("block %d unlocked\n"), first );
			printf( _("erasing block %d: %d\n"), first, flash_driver->erase_block( cfi_array, adr ) );
		}
	}

	printf( _("program:\n") );
	for (;;) {
		uint32_t a, l, c;

		fread( &a, sizeof a, 1, f );
		fread( &l, sizeof l, 1, f );
		fread( &c, sizeof c, 1, f );
		if (feof( f )) {
			printf( _("Error: premature end of file\n") );
			return;
		}
		printf( _("record: start = 0x%08X, len = 0x%08X, checksum = 0x%08X\n"), a, l, c );
		if ((a == 0) && (c == 0))
			break;
		if (l & 3) {
			printf( _("Error: Invalid record length!\n") );
			return;
		}

		while (l) {
			uint32_t data;

			printf( _("addr: 0x%08X"), a );
			printf( "\r" );
			fflush(stdout);
			fread( &data, sizeof data, 1, f );
			if (flash_driver->program( cfi_array, a, data )) {
				printf( _("\nflash error\n") );
				return;
			}
			a += 4;
			l -= 4;
		}
	}
	printf( "\n" );

	flash_driver->readarray( cfi_array );

	fseek( f, 15, SEEK_SET );
	printf( _("verify:\n") );

	for (;;) {
		uint32_t a, l, c;

		fread( &a, sizeof a, 1, f );
		fread( &l, sizeof l, 1, f );
		fread( &c, sizeof c, 1, f );
		if (feof( f )) {
			printf( _("Error: premature end of file\n") );
			return;
		}
		printf( _("record: start = 0x%08X, len = 0x%08X, checksum = 0x%08X\n"), a, l, c );
		if ((a == 0) && (c == 0))
			break;
		if (l & 3) {
			printf( _("Error: Invalid record length!\n") );
			return;
		}

		while (l) {
			uint32_t data, readed;

			printf( _("addr: 0x%08X"), a );
			printf( "\r" );
			fflush( stdout );
			fread( &data, sizeof data, 1, f );
			readed = bus_read( bus, a );
			if (data != readed) {
				printf( _("\nverify error: 0x%08X vs. 0x%08X at addr %08X\n"),
					readed, data, a );
				return;
			}
			a += 4;
			l -= 4;
		}
	}

	printf( _("\nDone.\n") );
}

static int
find_block( cfi_query_structure_t *cfi, int adr, int bus_width, int chip_width )
{
	int i;
	int b = 0;
	int bb = 0;

	for (i = 0; i < cfi->device_geometry.number_of_erase_regions; i++) {
		const int region_blocks = cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks;
		const int flash_block_size = cfi->device_geometry.erase_block_regions[i].erase_block_size;
		const int region_block_size = (bus_width / chip_width) * flash_block_size;
		const int region_size = region_blocks * region_block_size;

		if (adr < (bb + region_size))
			return b + ((adr - bb) / region_block_size);
		b += region_blocks;
		bb += region_size;
	}
	return -1;
}

void
flashmem( bus_t *bus, FILE *f, uint32_t addr )
{
	uint32_t adr;
	cfi_query_structure_t *cfi;
	int *erased;
	int i;
	int neb;
	int bus_width;
	int chip_width;

	set_flash_driver();
	if (!cfi_array || !flash_driver) {
		printf( _("no flash driver found\n") );
		return;
	}
	cfi = &cfi_array->cfi_chips[0]->cfi;

	bus_width = cfi_array->bus_width;
	chip_width = cfi_array->cfi_chips[0]->width;

	for (i = 0, neb = 0; i < cfi->device_geometry.number_of_erase_regions; i++)
		neb += cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks;

	erased = malloc( neb * sizeof *erased );
	if (!erased) {
		printf( _("Out of memory!\n") );
		return;
	}
	for (i = 0; i < neb; i++)
		erased[i] = 0;

	printf( _("program:\n") );
	adr = addr;
	while (!feof( f )) {
		uint32_t data;
#define BSIZE 4096
		uint8_t b[BSIZE];
		int bc = 0, bn = 0;
//		int block_no = find_block( cfi, adr );
		int block_no = find_block( cfi, adr - cfi_array->address, bus_width, chip_width);
		if (!erased[block_no]) {
			flash_driver->unlock_block( cfi_array, adr );
			printf( _("\nblock %d unlocked\n"), block_no );
			printf( _("erasing block %d: %d\n"), block_no, flash_driver->erase_block( cfi_array, adr ) );
			erased[block_no] = 1;
		}

		bn = fread( b, 1, BSIZE, f );
		for (bc = 0; bc < bn; bc += flash_driver->bus_width) {
			int j;
			if ((adr & 0xFF) == 0) {
				printf( _("addr: 0x%08X\r"), adr );
				fflush( stdout );
			}

			data = 0;
			for (j = 0; j < flash_driver->bus_width; j++)
				if (big_endian)
					data = (data << 8) | b[bc + j];
				else
					data |= b[bc + j] << (j * 8);

			if (flash_driver->program( cfi_array, adr, data )) {
				printf( _("\nflash error\n") );
				return;
			}
			adr += flash_driver->bus_width;
		}
	}
	printf( "\n" );

	flash_driver->readarray( cfi_array );

	fseek( f, 0, SEEK_SET );
	printf( _("verify:\n") );
	fflush( stdout );
	adr = addr;
	while (!feof( f )) {
		uint8_t buf[16];
		uint32_t data;
		uint32_t readed;
		int j;

		if (fread( buf, flash_driver->bus_width, 1, f ) != 1) {
			if (feof(f))
				break;
			printf( _("Error during file read.\n") );
			return;
		}

		data = 0;
		for (j = 0; j < flash_driver->bus_width; j++)
			if (big_endian)
				data = (data << 8) | buf[j];
			else
				data |= buf[j] << (j * 8);

		if ((addr && 0xffffff00) == 0) {
			printf( _("addr: 0x%08X\r"), adr );
			fflush( stdout );
		}
		readed = bus_read( bus, adr );
		if (data != readed) {
			printf( _("\nverify error:\nreaded: 0x%08X\nexpected: 0x%08X\n"), readed, data );
			return;
		}
		adr += flash_driver->bus_width;
	}
	printf( _("\nDone.\n") );

	free( erased );
}

void
flasherase( bus_t *bus, uint32_t addr, int number )
{
	cfi_query_structure_t *cfi;
	int i;
	int status = 0;

	set_flash_driver();
	if (!cfi_array || !flash_driver) {
		printf( _("no flash driver found\n") );
		return;
	}
	cfi = &cfi_array->cfi_chips[0]->cfi;

	printf( _("\nErasing %d Flash block%s from address 0x%x\n"), number, number > 1 ? "s" : "", addr);

	for (i = 1; i <= number; i++) {
		int addr_block = (cfi->device_geometry.erase_block_regions[0].erase_block_size * flash_driver->bus_width / 2);
		int block_no = addr / addr_block;
		printf( _("(%d%% Completed) FLASH Block %d : Unlocking ... "), i*100/number, block_no);
		fflush(stdout);
		flash_driver->unlock_block( cfi_array, addr );
		printf( _("Erasing ... ") );
		fflush(stdout);
		status = flash_driver->erase_block( cfi_array, addr );
		if (status == 0) {
			if (i == number)
				printf( _("\r(100%% Completed) FLASH Block %d : Unlocking ... Erasing ... Ok.\n"), block_no );
			else
				printf( _("Ok.\r%78s\r"), "" );
		}
		else
			printf( _("ERROR.\n") );
		addr |= (addr_block - 1);
		addr += 1;
	}

	if (status == 0)
		printf( _("\nErasing Completed.\n") );
	else
		printf( _("\nErasing Failed.\n") );

	/* BYPASS */
	//       parts_set_instruction( ps, "BYPASS" );
	//       chain_shift_instructions( chain );
}
