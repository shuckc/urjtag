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

#ifndef PART_H
#define	PART_H

#include <stdio.h>

#include "bssignal.h"
#include "instruction.h"
#include "data_register.h"
#include "bsbit.h"

#define	MAXLEN_MANUFACTURER	25
#define	MAXLEN_PART		20
#define	MAXLEN_STEPPING		8

typedef struct part part_t;

struct part {
	tap_register *id;
	char *alias; /* djf refdes */
	char manufacturer[MAXLEN_MANUFACTURER + 1];
	char part[MAXLEN_PART + 1];
	char stepping[MAXLEN_STEPPING + 1];
	signal_t *signals;
	salias_t *saliases;
	int instruction_length;
	instruction *instructions;
	instruction *active_instruction;
	data_register *data_registers;
	int boundary_length;
	bsbit_t **bsbits;
};

part_t *part_alloc( const tap_register *id );
void part_free( part_t *p );
part_t *read_part( FILE *f, tap_register_t *idr );
instruction *part_find_instruction( part_t *p, const char *iname );
data_register *part_find_data_register( part_t *p, const char *drname );
signal_t *part_find_signal( part_t *p, const char *signalname );
void part_set_instruction( part_t *p, const char *iname );
void part_set_signal( part_t *p, signal_t *s, int out, int val );
int part_get_signal( part_t *p, signal_t *s );
void part_print( part_t *p );

typedef struct parts parts_t;

struct parts {
	int len;
	part_t **parts;
};

parts_t *parts_alloc( void );
void parts_free( parts_t *ps );
int parts_add_part( parts_t *ps, part_t *p );
void parts_set_instruction( parts_t *ps, const char *iname );
void parts_print( parts_t *ps );

#endif /* PART_H */
