/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#define	MAXLEN_MANUFACTURER	20
#define	MAXLEN_PART		20
#define	MAXLEN_STEPPING		8

typedef struct part part;

struct part {
	char manufacturer[MAXLEN_MANUFACTURER + 1];
	char part[MAXLEN_PART + 1];
	char stepping[MAXLEN_STEPPING + 1];
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
void part_set_instruction( part *p, const char *iname );
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
void parts_shift_instructions( parts *ps );
void parts_shift_data_registers( parts *ps );
void parts_print( parts *ps, int header );

#endif /* JTAG_PART_H */
