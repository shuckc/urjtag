/*
 * $Id$
 *
 * Flash Memory interface
 * Copyright (C) 2003 AH
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
 * Written by August Hörandl <august.hoerandl@gmx.at>
 *
 */

#ifndef FLASH_H
#define	FLASH_H

#include <stdint.h>
#include <flash/cfi.h>

#include "part.h"
#include "bus.h"

typedef struct {
	int buswidth;		/* supported bus width, 1/2/4 bytes */
	const char *name;
	const char *description;
	int (*flash_autodetect)( bus_t *bus, cfi_query_structure_t *cfi );
	void (*flash_print_info)( bus_t *bus );
	int (*flash_erase_block)( bus_t *bus, uint32_t adr );
	int (*flash_unlock_block)( bus_t *bus, uint32_t adr );
	int (*flash_program)( bus_t *bus, uint32_t adr, uint32_t data );
	void (*flash_readarray)( bus_t *bus );
} flash_driver_t;

extern flash_driver_t *flash_driver;
extern flash_driver_t *flash_drivers[];

/* #define flash_print_info      flash_driver->flash_print_info */
#define flash_erase_block     flash_driver->flash_erase_block
#define flash_unlock_block    flash_driver->flash_unlock_block
#define flash_program         flash_driver->flash_program
#define flash_readarray       flash_driver->flash_readarray

extern void set_flash_driver( bus_t *bus, cfi_query_structure_t *cfi );

#define	CFI_INTEL_ERROR_UNKNOWN				1
#define	CFI_INTEL_ERROR_UNSUPPORTED			2
#define	CFI_INTEL_ERROR_LOW_VPEN			3
#define	CFI_INTEL_ERROR_BLOCK_LOCKED			4
#define	CFI_INTEL_ERROR_INVALID_COMMAND_SEQUENCE	5

#endif /* FLASH_H */
