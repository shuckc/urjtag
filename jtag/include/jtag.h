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

#include "chain.h"
#include "bus.h"
#include "part.h"

extern chain_t *chain;
extern bus_t *bus;
extern int big_endian;

int jtag_parse_file( const char *filename );

parts_t *detect_parts( chain_t *chain, char *db_path );
void discovery( chain_t *chain, const char *filename );

void readmem( bus_t *bus, FILE *f, uint32_t addr, uint32_t len );

void detectflash( bus_t *bus );

void flashmem( bus_t *bus, FILE *f, uint32_t addr );
void flashmsbin( bus_t *bus, FILE *f );

#endif /* JTAG_H */
