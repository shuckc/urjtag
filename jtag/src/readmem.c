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
#include <flash/cfi.h>
#include <flash/intel.h>
#include <std/mic.h>

#include "part.h"
#include "bus.h"

bus_driver_t *bus_driver = NULL;

void
readmem( parts *ps )
{
	part *p = ps->parts[0];
	uint8_t boot_sel;
	int o = 0;
	int d = 0;
#define	D_SA1110	1
#define	D_PXA250	2

	if (strcmp( p->part, "SA1110" ) == 0) {
		printf( "SA1110 detected\n" );
		d = D_SA1110;
		bus_driver = &sa1110_bus_driver;
	}
	if (strcmp( p->part, "PXA250" ) == 0) {
		printf( "PXA250 detected\n" );
		d = D_PXA250;
		bus_driver = &pxa250_bus_driver;
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

	/* detect CFI capable devices */
	bus_write( ps, 0x55 << o, 0x00980098 );		/* see 3.1 in [1] */
	if (bus_read( ps, 0x10 << o ) != (('Q' << 16) | 'Q')) {
		printf( "No CFI device detected (Q)!\n" );
		return;
	}
	if (bus_read( ps, 0x11 << o ) != (('R' << 16) | 'R')) {
		printf( "No CFI device detected (R)!\n" );
		return;
	}
	if (bus_read( ps, 0x12 << o ) != (('Y' << 16) | 'Y')) {
		printf( "No CFI device detected (Y)!\n" );
		return;
	}

	printf( "\n2 x 16 bit CFI devices detected (QRY ok)!\n\n" );

	/* TODO: Low chip only */
	/* see 3.3.2 in [1] */
	printf( "CFI Query Identification String:\n" );
	printf( "\tPrimary Vendor Command Set and Control Interface ID Code: 0x%02X%02X ", (uint8_t) bus_read( ps, 0x14 << o ), (uint8_t) bus_read( ps, 0x13 << o ) );
	{
		/* see Section 1. in [4] */
		uint16_t id = ((bus_read( ps, 0x14 << o ) & 0xFF) << 8) | (bus_read( ps, 0x13 << o ) & 0xFF);

		switch (id) {
			case CFI_VENDOR_NULL:
				printf( "(null)\n" );
				break;
			case CFI_VENDOR_INTEL_ECS:
				printf( "(Intel/Sharp Extended Command Set)\n" );
				break;
			case CFI_VENDOR_AMD_SCS:
				printf( "(AMD/Fujitsu Standard Commanf Set)\n" );
				break;
			case CFI_VENDOR_INTEL_SCS:
				printf( "(Intel Standard Command Set)\n" );
				break;
			case CFI_VENDOR_AMD_ECS:
				printf( "(AMD/Fujitsu Extended Command Set)\n" );
				break;
			case CFI_VENDOR_MITSUBISHI_SCS:
				printf( "(Mitsubishi Standard Command Set)\n" );
				break;
			case CFI_VENDOR_MITSUBISHI_ECS:
				printf( "(Mitsubishi Extended Command Set)\n" );
				break;
			case CFI_VENDOR_SST_PWCS:
				printf( "(Page Write Command Set)\n" );
				break;
			default:
				printf( "(unknown!!!)\n" );
				break;
		}
	}
	printf( "\tAddress of Primary Algorithm extended Query table: P = 0x%02X%02X\n", (uint8_t) bus_read( ps, 0x16 << o ), (uint8_t) bus_read( ps, 0x15 << o ) );
	printf( "\tAlternate Vendor Command Set and Control Interface ID Code: 0x%02X%02X (TODO)\n", (uint8_t) bus_read( ps, 0x18 << o ), (uint8_t) bus_read( ps, 0x17 << o ) );
	printf( "\tAddress of Alternate Algorithm extended Query table: A = 0x%02X%02X\n", (uint8_t) bus_read( ps, 0x1A << o ), (uint8_t) bus_read( ps, 0x19 << o ) );

	/* see 3.3.3 in [1] */
	printf( "CFI Query System Interface Information:\n" );
	printf( "\tVcc Logic Supply Minimum Write/Erase voltage: %d.%d V\n", (bus_read( ps, 0x1B << o ) >> 4) & 0xF, bus_read( ps, 0x1B << o ) & 0xF );
	printf( "\tVcc Logic Supply Maximum Write/Erase voltage: %d.%d V\n", (bus_read( ps, 0x1C << o ) >> 4) & 0xF, bus_read( ps, 0x1C << o ) & 0xF );
	printf( "\tVpp [Programming] Logic Supply Minimum Write/Erase voltage: %d.%d V\n", (bus_read( ps, 0x1D << o ) >> 4) & 0xF, bus_read( ps, 0x1D << o ) & 0xF );
	printf( "\tVpp [Programming] Logic Supply Maximum Write/Erase voltage: %d.%d V\n", (bus_read( ps, 0x1E << o ) >> 4) & 0xF, bus_read( ps, 0x1E << o ) & 0xF );
	printf( "\tTypical timeout per single byte/word write: %d us (0x%02X)\n", 1 << (bus_read( ps, 0x1F << o ) & 0xFF), bus_read( ps, 0x1F << o ) & 0xFF );
	printf( "\tTypical timeout for minimum-size buffer write: %d us (0x%02X)\n", 1 << (bus_read( ps, 0x20 << o ) & 0xFF), bus_read( ps, 0x20 << o ) & 0xFF );
	printf( "\tTypical timeout per individual block erase: %d ms (0x%02X)\n", 1 << (bus_read( ps, 0x21 << o ) & 0xFF), bus_read( ps, 0x21 << o ) & 0xFF );
	printf( "\tTypical timeout for full chip erase: %d ms (0x%02X)\n", 1 << (bus_read( ps, 0x22 << o ) & 0xFF), bus_read( ps, 0x22 << o ) & 0xFF );
	printf( "\tMaximum timeout for byte/word write: %d us (0x%02X)\n", 1 << ((bus_read( ps, 0x23 << o ) + bus_read( ps, 0x1F << o )) & 0xFF), bus_read( ps, 0x23 << o ) & 0xFF );
	printf( "\tMaximum timeout for buffer write: %d us (0x%02X)\n", 1 << ((bus_read( ps, 0x24 << o ) + bus_read( ps, 0x20 << o )) & 0xFF), bus_read( ps, 0x24 << o ) & 0xFF );
	printf( "\tMaximum timeout per individual block erase: %d ms (0x%02X)\n", 1 << ((bus_read( ps, 0x25 << o ) + bus_read( ps, 0x21 << o )) & 0xFF), bus_read( ps, 0x25 << o ) & 0xFF );
	printf( "\tMaximum timeout for chip erase: %d ms (0x%02X)\n", 1 << ((bus_read( ps, 0x26 << o ) + bus_read( ps, 0x22 << o )) & 0xFF), bus_read( ps, 0x26 << o ) & 0xFF );

	/* see 3.3.4 in [1] */
	printf( "Device Geometry Definition:\n" );
	printf( "\tDevice Size: %d B (0x%02X)\n", 1 << (bus_read( ps, 0x27 << o ) & 0xFF), bus_read( ps, 0x27 << o ) & 0xFF );
	printf( "\tFlash Device Interface description: 0x%02X%02X ", bus_read( ps, 0x29 << o ) & 0xFF, bus_read( ps, 0x28 << o ) & 0xFF );
	{
		/* see Section 2. in [4] */
		uint16_t id = ((bus_read( ps, 0x29 << o ) & 0xFF) << 8) | (bus_read( ps, 0x28 << o ) & 0xFF);

		switch (id) {
			case CFI_INTERFACE_X8:
				printf( "(x8)\n" );
				break;
			case CFI_INTERFACE_X16:
				printf( "(x16)\n" );
				break;
			case CFI_INTERFACE_X8_X16:
				printf( "(x8/x16)\n" );
				break;
			case CFI_INTERFACE_X32:
				printf( "(x32)\n" );
				break;
			case CFI_INTERFACE_X16_X32:
				printf( "(x16/x32)\n" );
				break;
			default:
				printf( "(unknown!!!)\n" );
				break;
		}
	}
	printf( "\tMaximum number of bytes in multi-byte write: %d\n", 1 << (((bus_read( ps, 0x2B << o ) & 0xFF) << 8) | (bus_read( ps, 0x2A << o ) & 0xFF)) );
	printf( "\tNumber of Erase Block Regions within device: %d\n", bus_read( ps, 0x2C << o ) & 0xFF );
	printf( "\tErase Block Region Information:\n" );
	
	{
		int a = 0x2D;
		int c = bus_read( ps, 0x2C << o ) & 0xFF;
		int i;

		for (i = 0; i < c; i++, a += 4) {
			uint32_t z = ((bus_read( ps, (a + 3) << o ) & 0xFF) << 16) | ((bus_read( ps, (a + 2) << o ) & 0xFF) << 8);
			uint32_t y = ((bus_read( ps, (a + 1) << o ) & 0xFF) << 8) | (bus_read( ps, a << o ) & 0xFF);

			printf( "\t\tRegion %d:\n", i );

			if (z == 0)
				z = 128;
			printf( "\t\t\tErase Block Size: %d\n", z );
			printf( "\t\t\tNumber of Erase Blocks: %d\n", y + 1 );
		}
	}

	/* Intel Primary Algorithm Extended Query Table - see Table 5. in [3] */
	/* TODO */

	/* Clear Status Register */
	bus_write( ps, 0 << o, 0x00500050 );

	/* Read Identifier Command */
	bus_write( ps, 0 << 0, 0x00900090 );

	switch (bus_read( ps, 0x00 << o ) & 0xFF) {
		case STD_MIC_INTEL:
			printf( "Manufacturer: %s\n", STD_MICN_INTEL );
			break;
		default:
			printf( "Unknown manufacturer!\n" );
			break;
	}

	printf( "Chip: " );
	switch (bus_read( ps, 0x01 << o ) & 0xFFFF) {
		case 0x0016:
			printf( "28F320J3A\n" );
			break;
		case 0x0017:
			printf( "28F640J3A\n" );
			break;
		case 0x0018:
			printf( "28F128J3A\n" );
			break;
		case 0x8801:
			printf( "28F640K3\n" );
			break;
		case 0x8802:
			printf( "28F128K3\n" );
			break;
		case 0x8803:
			printf( "28F256K3\n" );
			break;
		case 0x8805:
			printf( "28F640K18\n" );
			break;
		case 0x8806:
			printf( "28F128K18\n" );
			break;
		case 0x8807:
			printf( "28F256K18\n" );
			break;
		default:
			printf( "Unknown!\n" );
			break;
	}

	/* Read Array */
	bus_write( ps, 0 << o, 0x00FF00FF );
}
