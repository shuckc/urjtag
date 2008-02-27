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

#ifndef JTAG_H
#define JTAG_H

#include <stdio.h>
#include <stdint.h>

#include <flash.h>

#include "chain.h"
#include "bus.h"
#include "part.h"

extern bus_t *bus;
extern int big_endian;
extern int debug_mode;

int jtag_parse_file( chain_t *chain, const char *filename );

int detect_parts( chain_t *chain, char *db_path );
int detect_register_size( chain_t *chain );
void discovery( chain_t *chain );

void readmem( bus_t *bus, FILE *f, uint32_t addr, uint32_t len );
void writemem( bus_t *bus, FILE *f, uint32_t addr, uint32_t len );

void flasherase( bus_t *bus, uint32_t addr, int number );

#endif /* JTAG_H */
