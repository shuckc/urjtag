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

#include <jtag/part.h>
#include <jtag/tap.h>

/* part */

part *
part_alloc( void )
{
	part *p = malloc( sizeof *p );
	if (!p)
		return NULL;

	p->signals = NULL;
	p->instruction_length = 0;
	p->instructions = NULL;
	p->active_instruction = NULL;
	p->data_registers = NULL;
	p->boundary_length = 0;
	p->bsbits = NULL;
	p->idr = NULL;
	p->bsr = NULL;
	p->prev_bsr = NULL;

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

	while (p->data_registers) {
		data_register *dr = p->data_registers;
		p->data_registers = dr->next;
		data_register_free( dr );
	}

	/* bsbits */
	for (i = 0; i < p->boundary_length; i++)
		bsbit_free( p->bsbits[i] );
	free( p->bsbits );

	/* idr */
	register_free( p->idr );

	/* bsr */
	register_free( p->bsr );
	register_free( p->prev_bsr );

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
		dr = dr->next
	}

	return dr;
}

void
part_set_signal( part *p, const char *pname, int out, int val )
{
	/* search signal */
	signal *s = p->signals;
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
		p->bsr->data[s->output->bit] = val & 1;

		control = p->bsbits[s->output->bit]->control;
		if (control >= 0)
			p->bsr->data[control] = p->bsbits[s->output->bit]->control_value ^ 1;
	} else {
		if (!s->input) {
			printf( "signal %s cannot be set as input\n", pname );
			return;
		}
		if (s->output)
			p->bsr->data[s->output->control] = p->bsbits[s->output->control]->control_value;
	}
}

int
part_get_signal( part *p, const char *pname )
{
	/* search signal */
	signal *s = p->signals;
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

	return p->prev_bsr->data[s->input->bit];
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
	int j;

	tap_capture_ir();

	for (j = 0; j < ps->len; j++) {
		instruction *i = part_find_instruction( ps->parts[j], iname );
		if (!i) {
			printf( "Instruction '%s' not found\n", iname );
			return;
		}
		ps->parts[j]->active_instruction = i;
		tap_shift_register( i->value, NULL, (j + 1) == ps->len );
	}
}
