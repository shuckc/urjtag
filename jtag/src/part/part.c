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

#include <stdlib.h>
#include <string.h>

#include "part.h"
#include "tap.h"

/* part */

part *
part_alloc( void )
{
	part *p = malloc( sizeof *p );
	if (!p)
		return NULL;

	p->manufacturer[0] = '\0';
	p->part[0] = '\0';
	p->stepping[0] = '\0';
	p->signals = NULL;
	p->instruction_length = 0;
	p->instructions = NULL;
	p->active_instruction = NULL;
	p->data_registers = NULL;
	p->boundary_length = 0;
	p->bsbits = NULL;

	return p;
}

void
part_free( part *p )
{
	int i;

	if (!p)
		return;

	/* sirnals */
	while (p->signals) {
		signal *s = p->signals;
		p->signals = s->next;
		signal_free( s );
	}

	/* instructions */
	while (p->instructions) {
		instruction *i = p->instructions;
		p->instructions = i->next;
		instruction_free( i );
	}

	/* data registers */
	while (p->data_registers) {
		data_register *dr = p->data_registers;
		p->data_registers = dr->next;
		data_register_free( dr );
	}

	/* bsbits */
	for (i = 0; i < p->boundary_length; i++)
		bsbit_free( p->bsbits[i] );
	free( p->bsbits );

	free( p );
}

instruction *
part_find_instruction( part *p, const char *iname )
{
	instruction *i;

	if (!p || !iname)
		return NULL;

	i = p->instructions;
	while (i) {
		if (strcmp( iname, i->name ) == 0)
			break;
		i = i->next;
	}

	return i;
}

data_register *
part_find_data_register( part *p, const char *drname )
{
	data_register *dr;

	if (!p || !drname)
		return NULL;

	dr = p->data_registers;
	while (dr) {
		if (strcmp( drname, dr->name ) == 0)
			break;
		dr = dr->next;
	}

	return dr;
}

void part_set_instruction( part *p, const char *iname )
{
	if (p)
		p->active_instruction = part_find_instruction( p, iname );
}

void
part_shift_instruction( part *p, int exit )
{
	if (!p || !p->active_instruction)
		return;

	tap_shift_register( p->active_instruction->value, NULL, exit );
}

void
part_shift_data_register( part *p, int exit )
{
	if (!p || !p->active_instruction || !p->active_instruction->data_register)
		return;
	
	tap_shift_register( p->active_instruction->data_register->in, p->active_instruction->data_register->out, exit );
}

void
part_set_signal( part *p, const char *pname, int out, int val )
{
	signal *s;

	/* search for Boundary Scan Register */
	data_register *bsr = part_find_data_register( p, "BSR" );
	if (!bsr) {
		printf( "%s(%s:%d) Boundary Scan Register (BSR) not found\n", __FUNCTION__, __FILE__, __LINE__ );
		return;
	}

	/* search signal */
	s = p->signals;
	while (s) {
		if (strcmp( pname, s->name ) == 0)
			break;
		s = s->next;
	}

	if (!s) {
		printf( "signal %s not found\n", pname );
		return;
	}

	/*setup signal */
	if (out) {
		int control;
		if (!s->output) {
			printf( "signal %s cannot be set as output\n", pname );
			return;
		}
		bsr->in->data[s->output->bit] = val & 1;

		control = p->bsbits[s->output->bit]->control;
		if (control >= 0)
			bsr->in->data[control] = p->bsbits[s->output->bit]->control_value ^ 1;
	} else {
		if (!s->input) {
			printf( "signal %s cannot be set as input\n", pname );
			return;
		}
		if (s->output)
			bsr->in->data[s->output->control] = p->bsbits[s->output->bit]->control_value;
	}
}

int
part_get_signal( part *p, const char *pname )
{
	signal *s;

	/* search for Boundary Scan Register */
	data_register *bsr = part_find_data_register( p, "BSR" );
	if (!bsr) {
		printf( "%s(%s:%d) Boundary Scan Register (BSR) not found\n", __FUNCTION__, __FILE__, __LINE__ );
		return -1;
	}

	/* search signal */
	s = p->signals;
	while (s) {
		if (strcmp( pname, s->name ) == 0)
			break;
		s = s->next;
	}

	if (!s) {
		printf( "signal %s not found\n", pname );
		return -1;
	}

	if (!s->input) {
		printf( "signal %s is not input signal\n", pname );
		return -1;
	}

	return bsr->out->data[s->input->bit];
}

/* parts */

parts *
parts_alloc( void )
{
	parts *ps = malloc( sizeof *ps );
	if (!ps)
		return NULL;

	ps->len = 0;
	ps->parts = NULL;

	return ps;
}

void
parts_free( parts *ps )
{
	int i;

	if (!ps)
		return;

	for (i = 0; i < ps->len; i++)
		part_free(ps->parts[i]);

	free( ps->parts );
	free( ps );
}

int
parts_add_part( parts *ps, part *p )
{
	part **np = realloc( ps->parts, (ps->len + 1) * sizeof *ps->parts );

	if (!np)
		return 0;

	ps->parts = np;
	ps->parts[ps->len++] = p;

	return 1;
}

void
parts_set_instruction( parts *ps, const char *iname )
{
	int i;

	for (i = 0; i < ps->len; i++)
		ps->parts[i]->active_instruction = part_find_instruction( ps->parts[i], iname );
}

void
parts_shift_instructions( parts *ps )
{
	int i;

	if (!ps)
		return;

	tap_capture_ir();

	for (i = 0; i < ps->len; i++) {
		if (!ps->parts[i]->active_instruction) {
			printf( "%s(%s:%d) Part without active instruction\n", __FUNCTION__, __FILE__, __LINE__ );
			continue;
		}
		tap_shift_register( ps->parts[i]->active_instruction->value, NULL, (i + 1) == ps->len );
	}
}

void
parts_shift_data_registers( parts *ps )
{
	int i;

	if (!ps)
		return;

	tap_capture_dr();

	for (i = 0; i < ps->len; i++) {
		if (!ps->parts[i]->active_instruction) {
			printf( "%s(%s:%d) Part without active instruction\n", __FUNCTION__, __FILE__, __LINE__ );
			continue;
		}
		tap_shift_register( ps->parts[i]->active_instruction->data_register->in,
				ps->parts[i]->active_instruction->data_register->out, (i + 1) == ps->len );
	}
}

void
parts_print( parts *ps, int header )
{
	int i;

	char format[100];
	snprintf( format, 100, " %%-%ds %%-%ds %%-%ds %%-%ds %%-%ds \n", MAXLEN_MANUFACTURER, MAXLEN_PART, MAXLEN_STEPPING,
			MAXLEN_INSTRUCTION, MAXLEN_DATA_REGISTER );

	if (header) {
		printf( " No." );
		printf( format, "Manufacturer", "Part", "Stepping", "Instruction", "Register" );
		for (i = 0; i < 10 + MAXLEN_MANUFACTURER + MAXLEN_PART + MAXLEN_STEPPING + MAXLEN_INSTRUCTION + MAXLEN_DATA_REGISTER; i++ )
			putchar( '-' );
		putchar( '\n' );
	}

	if (!ps)
		return;

	for (i = 0; i < ps->len; i++) {
		char *instruction = "(none)";
		char *dr = "(none)";
		part *p = ps->parts[i];

		if (p->active_instruction) {
			instruction = p->active_instruction->name;
			dr = p->active_instruction->data_register->name;
		}
		printf( " %3d", i );
		printf( format, p->manufacturer, p->part, p->stepping, instruction, dr );
	}
}
