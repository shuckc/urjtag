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
 * [2] JEDEC Solid State Technology Association, "Common Flash Interface (CFI) ID Codes",
 *     September 2001, Order Number: JEP137-A
 *
 */

#include "sysdep.h"

#include <stdint.h>
#include <string.h>
#include <flash/cfi.h>
#include <flash/intel.h>
#include <std/mic.h>

#include <brux/flash.h>
#include <brux/cfi.h>
#include <brux/bus.h>

int jedec_detect( bus_t *bus, uint32_t adr, cfi_array_t **cfi_array );

void
detectflash( bus_t *bus )
{
	cfi_array_t *cfi_array = NULL;
	cfi_query_structure_t *cfi;
	const char *s;

	if (!bus) {
		printf( _("Error: Missing bus driver!\n") );
		return;
	}

	bus_prepare( bus );

	if (cfi_detect( bus, 0, &cfi_array )) {
		cfi_array_free( cfi_array );
		cfi_array = NULL;
		if (jedec_detect( bus, 0, &cfi_array ) != 0) {
			cfi_array_free( cfi_array );
			printf( _("Flash not found!\n") );
			return;
		}
	}

	cfi = &cfi_array->cfi_chips[0]->cfi;

	/* detect CFI capable devices */
	/* TODO: Low chip only */
	/* see 4.3.2 in [1] */
	printf( _("Query identification string:\n") );
	/* see section 2 in [2] */
	switch (cfi->identification_string.pri_id_code) {
		case CFI_VENDOR_NULL:
			s = N_("null");
			break;
		case CFI_VENDOR_INTEL_ECS:
			s = N_("Intel/Sharp Extended Command Set");
			break;
		case CFI_VENDOR_AMD_SCS:
			s = N_("AMD/Fujitsu Standard Command Set");
			break;
		case CFI_VENDOR_INTEL_SCS:
			s = N_("Intel Standard Command Set");
			break;
		case CFI_VENDOR_AMD_ECS:
			s = N_("AMD/Fujitsu Extended Command Set");
			break;
		case CFI_VENDOR_MITSUBISHI_SCS:
			s = N_("Mitsubishi Standard Command Set");
			break;
		case CFI_VENDOR_MITSUBISHI_ECS:
			s = N_("Mitsubishi Extended Command Set");
			break;
		case CFI_VENDOR_SST_PWCS:
			s = N_("Page Write Command Set");
			break;
		default:
			s = N_("unknown!!!");
			break;
	}
	printf( _("\tPrimary Algorithm Command Set and Control Interface ID Code: 0x%04X (%s)\n"), cfi->identification_string.pri_id_code, _(s) );
	switch (cfi->identification_string.alt_id_code) {
		case CFI_VENDOR_NULL:
			s = N_("null");
			break;
		case CFI_VENDOR_INTEL_ECS:
			s = N_("Intel/Sharp Extended Command Set");
			break;
		case CFI_VENDOR_AMD_SCS:
			s = N_("AMD/Fujitsu Standard Command Set");
			break;
		case CFI_VENDOR_INTEL_SCS:
			s = N_("Intel Standard Command Set");
			break;
		case CFI_VENDOR_AMD_ECS:
			s = N_("AMD/Fujitsu Extended Command Set");
			break;
		case CFI_VENDOR_MITSUBISHI_SCS:
			s = N_("Mitsubishi Standard Command Set");
			break;
		case CFI_VENDOR_MITSUBISHI_ECS:
			s = N_("Mitsubishi Extended Command Set");
			break;
		case CFI_VENDOR_SST_PWCS:
			s = N_("Page Write Command Set");
			break;
		default:
			s = N_("unknown!!!");
			break;
	}
	printf( _("\tAlternate Algorithm Command Set and Control Interface ID Code: 0x%04X (%s)\n"), cfi->identification_string.alt_id_code, _(s) );

	/* see 4.3.3 in [1] */
	printf( _("Query system interface information:\n") );
	printf( _("\tVcc Logic Supply Minimum Write/Erase or Write voltage: %d mV\n"), cfi->system_interface_info.vcc_min_wev );
	printf( _("\tVcc Logic Supply Maximum Write/Erase or Write voltage: %d mV\n"), cfi->system_interface_info.vcc_max_wev );
	printf( _("\tVpp [Programming] Supply Minimum Write/Erase voltage: %d mV\n"), cfi->system_interface_info.vpp_min_wev );
	printf( _("\tVpp [Programming] Supply Maximum Write/Erase voltage: %d mV\n"), cfi->system_interface_info.vpp_max_wev );
	printf( _("\tTypical timeout per single byte/word program: %d us\n"), cfi->system_interface_info.typ_single_write_timeout );
	printf( _("\tTypical timeout for maximum-size multi-byte program: %d us\n"), cfi->system_interface_info.typ_buffer_write_timeout );
	printf( _("\tTypical timeout per individual block erase: %d ms\n"), cfi->system_interface_info.typ_block_erase_timeout );
	printf( _("\tTypical timeout for full chip erase: %d ms\n"), cfi->system_interface_info.typ_chip_erase_timeout );
	printf( _("\tMaximum timeout for byte/word program: %d us\n"), cfi->system_interface_info.max_single_write_timeout );
	printf( _("\tMaximum timeout for multi-byte program: %d us\n"), cfi->system_interface_info.max_buffer_write_timeout );
	printf( _("\tMaximum timeout per individual block erase: %d ms\n"), cfi->system_interface_info.max_block_erase_timeout );
	printf( _("\tMaximum timeout for chip erase: %d ms\n"), cfi->system_interface_info.max_chip_erase_timeout );

	/* see 4.3.4 in [1] */
	printf( _("Device geometry definition:\n") );
	printf( _("\tDevice Size: %d B (%d KiB, %d MiB)\n"), 
		cfi->device_geometry.device_size,
		cfi->device_geometry.device_size / 1024,
		cfi->device_geometry.device_size / (1024 * 1024) );
	/* see section 4 in [2] */
	switch (cfi->device_geometry.device_interface) {
		case CFI_INTERFACE_X8:
			s = N_("x8");
			break;
		case CFI_INTERFACE_X16:
			s = N_("x16");
			break;
		case CFI_INTERFACE_X8_X16:
			s = N_("x8/x16");
			break;
		case CFI_INTERFACE_X32:
			s = N_("x32");
			break;
		case CFI_INTERFACE_X16_X32:
			s = N_("x16/x32");
			break;
		default:
			s = N_("unknown!!!");
			break;
	}
	printf( _("\tFlash Device Interface Code description: 0x%04X (%s)\n"), cfi->device_geometry.device_interface, _(s) );
	printf( _("\tMaximum number of bytes in multi-byte program: %d\n"), cfi->device_geometry.max_bytes_write );
	printf( _("\tNumber of Erase Block Regions within device: %d\n"), cfi->device_geometry.number_of_erase_regions );
	printf( _("\tErase Block Region Information:\n") );
	{
		int i;

		for (i = 0; i < cfi->device_geometry.number_of_erase_regions; i++) {
			printf( _("\t\tRegion %d:\n"), i );
			printf( _("\t\t\tErase Block Size: %d B (%d KiB)\n"),
				cfi->device_geometry.erase_block_regions[i].erase_block_size,
				cfi->device_geometry.erase_block_regions[i].erase_block_size / 1024 );
			printf( _("\t\t\tNumber of Erase Blocks: %d\n"), cfi->device_geometry.erase_block_regions[i].number_of_erase_blocks );
		}
	}

	cfi_array_free( cfi_array );
}
