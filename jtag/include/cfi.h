/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#ifndef CFI_H
#define CFI_H

#include <stdint.h>
#include <flash/cfi.h>

#include "bus.h"

typedef struct {
	int width;		/* 1 for 8 bits, 2 for 16 bits, 4 for 32 bits, etc. */
	cfi_query_structure_t cfi;
} cfi_chip_t;

typedef struct {
	bus_t *bus;
	uint32_t address;
	int bus_width;		/* in cfi_chips, e.g. 4 for 32 bits */
	cfi_chip_t **cfi_chips;
} cfi_array_t;

void cfi_array_free( cfi_array_t *cfi_array );
int detect_cfi( bus_t *bus, uint32_t adr, cfi_array_t **cfi_array );

#endif /* CFI_H */
