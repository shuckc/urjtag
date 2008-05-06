/*
 * $Id: amd_flash.c,v 1.0 20/09/2006 12:38:01  $
 *
 * AMD 8 bit flash driver for AM29F040B & AM29LV040B
 * Copyright (C) 2006 Kila Medical Systems.
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
 * Written by Ajith Kumar P.C <ajithpc@kila.com>
 *
 * Documentation:
 * [1] Spansion, Am29F040B Data Sheet
 * [2] Spansion, Am29LV040B Data Sheet
*/

#include "sysdep.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <flash/cfi.h>
#include <flash/intel.h>
#include <unistd.h>

#include <flash.h>
#include <bus.h>

#ifdef __MINGW32__
#include <windows.h>
#define usleep(x) Sleep(x/1E3)
#endif

//write specific
#define AMD_SECTOR_PROTECTED

//Read Specific
#define AMD_READ_IN_ERASE_SUSPENDED_SECTOR
#define AMD_READ_IN_NON_ERASE_SUSPENDED_SECTOR
#define AMD_NORMAL_READ
#define AMD_UNKNOWN_READ

//Mode
#define AMD_ERASE_SUSPEND_MODE
#define AMD_READING_MODE
#define AMD_EMBEDDED_ERASE_ALGORITHM
#define AMD_EMBEDDED_PROGRAM_ALGORITHM
#define AMD_UNDEFINED_MODE

#define FLASH_ERASE_ERROR			-5
#define ERASE_FLASH_SUCCESS			1

#define AMD_29xx040B	1

#define AMD_BYPASS_UNLOCK_ALGORITHM		1
#define AMD_STANDARD_WRITE_ALGORITHM		0
#define AMD_BYPASS_UNLOCK_MODE			1
#define AMD_STANDARD_MODE			0

struct
{
	unsigned long flash;
	unsigned short algorithm;
	unsigned short unlock_bypass;
}
var_forced_detection;

int amd_detect(bus_t *bus, uint32_t adr, cfi_array_t **cfi_array );
static int amd_29xx040_autodetect( cfi_array_t *cfi_array );
static int amd_29xx040_status( bus_t *bus, uint32_t adr, unsigned short data );
static void amd_29xx040_print_info( cfi_array_t *cfi_array );
static void amd_29xx040_read_array( cfi_array_t *cfi_array );
static int amd_29xx040_erase_block( cfi_array_t *cfi_array, uint32_t adr );
static int amd_29xx040_program( cfi_array_t *cfi_array, uint32_t adr, uint32_t data );
static int amd_29xx040_unlock_block( cfi_array_t *cfi_array, uint32_t adr );

int amd_detect(bus_t *bus, uint32_t adr, cfi_array_t **cfi_array )
{
	int mid;
	int did;
	bus_area_t area;
	cfi_query_structure_t *cfi ;

	if (!cfi_array || !bus)
		return -1;		/* invalid parameters */

	*cfi_array = calloc( 1, sizeof (cfi_array_t) );
	if (!*cfi_array)
		return -2;		/* out of memory */

	bus_write( bus, adr+0x0, 0xf0 );
	bus_write( bus, adr+0x555, 0xaa );
	bus_write( bus, adr+0x2AA, 0x55 );
	bus_write( bus, adr+0x555, 0x90 );
	mid = bus_read( bus, adr+0x0);
	did = bus_read( bus, adr+0x1);
	bus_write( bus, adr+0x0, 0xf0 );

	printf( "%s: mid %x, did %x\n", __FUNCTION__, mid, did );
	if (mid != 0x01)
		return -1;

	switch(did)
	{
		case 0xA4:
			var_forced_detection.flash = AMD_29xx040B;
			break;
		case 0x4F:
			var_forced_detection.flash = AMD_29xx040B;
			var_forced_detection.algorithm = AMD_BYPASS_UNLOCK_ALGORITHM;
			break;
		default:
			break;
	}

        (*cfi_array)->bus = bus;
        (*cfi_array)->address = 0;
        if (bus_area( bus, adr+0, &area ) != 0)
                return -8;              /* bus width detection failed */
        unsigned int bw = area.width;
	int ba,i;
        if (bw != 8 && bw != 16 && bw != 32)
                return -3;              /* invalid bus width */
        (*cfi_array)->bus_width = ba = bw / 8;
        (*cfi_array)->cfi_chips = calloc( ba, sizeof (cfi_chip_t *) );
        if (!(*cfi_array)->cfi_chips)
                return -2; 
	for ( i=0; i<ba; i++ )
	{
		(*cfi_array)->cfi_chips[i] = calloc( 1, sizeof (cfi_chip_t) );
		if (!(*cfi_array)->cfi_chips[i])
			return -2;	/* out of memory */
		(*cfi_array)->cfi_chips[i]->width = 1;		//ba;		
		cfi = &(*cfi_array)->cfi_chips[i]->cfi;

		cfi->identification_string.pri_id_code = CFI_VENDOR_NULL;
		cfi->identification_string.pri_vendor_tbl = NULL;
		cfi->identification_string.alt_id_code = 0;
		cfi->identification_string.alt_vendor_tbl = NULL;
			
		cfi->device_geometry.device_size = 512*1024;
		cfi->device_geometry.device_interface = 0;	// x 8
		cfi->device_geometry.max_bytes_write = 32;	//not used
		cfi->device_geometry.number_of_erase_regions = 1;
		cfi->device_geometry.erase_block_regions =
		malloc( cfi->device_geometry.number_of_erase_regions * sizeof (cfi_erase_block_region_t) );
		if (!cfi->device_geometry.erase_block_regions)
			return -2;	/* out of memory */

		cfi->device_geometry.erase_block_regions[i].erase_block_size = 64 * 1024;
		cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks = 8;
		//Add other details for info
	}
	return 0;
}


static int amd_29xx040_autodetect( cfi_array_t *cfi_array )
{
	return(var_forced_detection.flash == AMD_29xx040B); //Non-CFI Am29xx040B flash
}

static int amd_29xx040_status( bus_t *bus, uint32_t adr, unsigned short data )
{
	short timeout;
	unsigned short dq7bit,dq7mask,dq5mask;
	unsigned short data1;

	dq7mask = (1 << 7);
	dq5mask = (1 << 5);
	dq7bit = data & dq7mask;

	for (timeout = 0; timeout < 1000; timeout++)	//typical sector erase time = 0.7 sec
	{
		data1 = (unsigned short)(bus_read( bus, adr ) & 0xFF);
		if((data1 & dq7mask) == dq7bit)
			return 1;	//Success

		if((data1 & dq5mask) == dq5mask)
		{
			data1 = (unsigned short)(bus_read( bus, adr ) & 0xFF);
			if((data1 & dq7mask) == dq7bit)
			{
				return 1;	//Success
			}
			else
			{
				return 0; //Failure - Needs a reset command to return back to read array data
			}
		}
		usleep (50);
	}

	return 0;	//hardware failure
}



static void amd_29xx040_print_info( cfi_array_t *cfi_array )
{
	int mid, did, prot;
	bus_t *bus = cfi_array->bus;


	bus_write( bus, cfi_array->address + 0x0, 0xf0 );
	bus_write( bus, cfi_array->address + 0x555, 0xaa );
	bus_write( bus, cfi_array->address + 0x2AA, 0x55 );
	bus_write( bus, cfi_array->address + 0x555, 0x90 );
	mid  = bus_read( bus, cfi_array->address + 0x0);
	did  = bus_read( bus, cfi_array->address + 0x1);
	prot = bus_read( bus, cfi_array->address + 0x2);
	bus_write( bus, cfi_array->address + 0x0, 0xf0 );

	printf( "%s: mid %x, did %x\n", __FUNCTION__, mid, did );
//	amd_29xx040_read_array( cfi_array );		/* AMD reset */

	switch (mid)
	{
		case 0x01:
			printf( _("Chip: AMD Flash\n\tPartNumber: ") );
			break;
		default:
			printf( _("Unknown manufacturer (ID 0x%04x)"), mid );
			break;
	}
	printf( _("\n\tChip: ") );
	switch (did) {
		case 0xA4:
			printf( "Am29C040B\t-\t" );
			printf( _("5V Flash\n") );
			break;
		case 0x4F:
			printf( "Am29LV040B\t-\t" );
			printf( _("3V Flash\n") );
			break;
		default:
			printf ( _("Unknown (ID 0x%04x)"), did );
			break;
	}
	printf( _("\n\tProtected: %04x\n"), prot );
}

static void amd_29xx040_read_array( cfi_array_t *cfi_array )
{
	/* Read Array */
	if(var_forced_detection.unlock_bypass == AMD_BYPASS_UNLOCK_MODE)
	{
		bus_write( bus, cfi_array->address + 0x555, 0x90 );
		bus_write( bus, cfi_array->address + 0x2AA, 0x00 );
		usleep(100);
		var_forced_detection.unlock_bypass = AMD_STANDARD_MODE;
	}
	bus_write( cfi_array->bus, cfi_array->address + 0x0, 0x0F0 ); /* AMD reset */
}



static int amd_29xx040_erase_block( cfi_array_t *cfi_array, uint32_t adr )
{
	bus_t *bus = cfi_array->bus;

	printf("flash_erase_block 0x%08X\n", adr);

	/*	printf("protected: %d\n", amdisprotected(ps, adr)); */

	if(var_forced_detection.unlock_bypass == AMD_BYPASS_UNLOCK_MODE)
	{
		bus_write( bus, cfi_array->address + 0x555, 0x90 );
		bus_write( bus, cfi_array->address + 0x2AA, 0x00 );
		usleep(100);
		var_forced_detection.unlock_bypass = AMD_STANDARD_MODE;
	}

	bus_write( bus, cfi_array->address + 0x0, 0xf0 );
	bus_write( bus, cfi_array->address + 0x555, 0xaa );
	bus_write( bus, cfi_array->address + 0x2AA, 0x55 );
	bus_write( bus, cfi_array->address + 0x555, 0x80 );
	bus_write( bus, cfi_array->address + 0x555, 0xaa );
	bus_write( bus, cfi_array->address + 0x2AA, 0x55 );
//	bus_write( bus, cfi_array->address + 0x555, 0x10 );	//Chip Erase
	bus_write( bus, adr, 0x30 );	//Sector erase


	if (amd_29xx040_status( bus, adr, 0xff )) {
		printf( "flash_erase_block 0x%08X DONE\n", adr );
		amd_29xx040_read_array( cfi_array );	/* AMD reset */
		return ERASE_FLASH_SUCCESS;
	}
	printf( "flash_erase_block 0x%08X FAILED\n", adr );
	/* Read Array */
	amd_29xx040_read_array( cfi_array );		/* AMD reset */

	return FLASH_ERASE_ERROR;
}

static int amd_29xx040_program( cfi_array_t *cfi_array, uint32_t adr, uint32_t data )
{
	int status;
	bus_t *bus = cfi_array->bus;

	if (0)
		printf("\nflash_program 0x%08X = 0x%08X\n", adr, data);
	if(var_forced_detection.algorithm == AMD_BYPASS_UNLOCK_ALGORITHM)
	{
		if(var_forced_detection.unlock_bypass != AMD_BYPASS_UNLOCK_MODE)
		{
			bus_write( bus, cfi_array->address + 0x555, 0xaa );
			bus_write( bus, cfi_array->address + 0x2AA, 0x55 );
			bus_write( bus, cfi_array->address + 0x555, 0x20 );
			usleep(1000);
			var_forced_detection.unlock_bypass = AMD_BYPASS_UNLOCK_MODE;
		}
	}
	else
	{
		bus_write( bus, cfi_array->address + 0x555, 0xaa );
		bus_write( bus, cfi_array->address + 0x2AA, 0x55 );
	}

	bus_write( bus, cfi_array->address + 0x555, 0xA0 );
	bus_write( bus, adr, data );
	status = amd_29xx040_status( bus, adr, data );
	/*	amd_29xx040_read_array(cfi_array); */

	return !status;
}

static int amd_29xx040_unlock_block( cfi_array_t *cfi_array, uint32_t adr )
{
	printf( "flash_unlock_block 0x%08X IGNORE\n", adr );
	return 0;
}


flash_driver_t amd_29xx040_flash_driver = {
	1, /* buswidth */
	N_("AMD Standard Command Set"),
	N_("supported: AMD 29LV040B, 29C040B, 1x8 Bit"),
	amd_29xx040_autodetect,
	amd_29xx040_print_info,
	amd_29xx040_erase_block,
	amd_29xx040_unlock_block,
	amd_29xx040_program,
	amd_29xx040_read_array,
};
