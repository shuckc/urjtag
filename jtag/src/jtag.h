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

#include <stdint.h>
#include <stdio.h>

#include <flash/cfi.h>

#include "part.h"
#include "chain.h"

parts_t *detect_parts( chain_t* chain, char *db_path );
void detectflash( chain_t *chain );
void readmem( chain_t *chain, FILE *f, uint32_t addr, uint32_t len );
void flashmem( chain_t *chain, FILE *f, uint32_t addr );
void flashmsbin( chain_t *chain, FILE *f );

void help( const char *cmd );

void discovery( chain_t *chain, const char *filename );

cfi_query_structure_t *detect_cfi( chain_t *chain );

#endif /* JTAG_H */
