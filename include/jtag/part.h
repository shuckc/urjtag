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
 */

#ifndef JTAG_PART_H
#define	JTAG_PART_H

#include <stdio.h>

#include <jtag/signal.h>
#include <jtag/instruction.h>
#include <jtag/data_register.h>
#include <jtag/bsbit.h>

typedef struct part part;

struct part {
	signal *signals;
	int instruction_length;
	instruction *instructions;
	instruction *active_instruction;
	data_register *data_registers;
	int boundary_length;
	bsbit **bsbits;
};

part *part_alloc( void );
void part_free( part *p );
part *read_part( FILE *f, tap_register *idr );
instruction *part_find_instruction( part *p, const char *iname );
data_register *part_find_data_register( part *p, const char *drname );
void part_shift_instruction( part *p, int exit );
void part_shift_data_register( part *p, int exit );
void part_set_signal( part *p, const char *pname, int out, int val );
int part_get_signal( part *p, const char *pname );

typedef struct parts parts;

struct parts {
	int len;
	part **parts;
};

parts *parts_alloc( void );
void parts_free( parts *ps );
int parts_add_part( parts *ps, part *p );
void parts_set_instruction( parts *ps, const char *iname );

#endif /* JTAG_PART_H */
