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

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>

#include "part.h"

/* part */

part_t *
part_alloc( const tap_register *id )
{
	part_t *p = malloc( sizeof *p );
	if (!p)
		return NULL;

	p->id = register_duplicate( id );
	p->manufacturer[0] = '\0';
	p->part[0] = '\0';
	p->stepping[0] = '\0';
	p->signals = NULL;
	p->saliases = NULL;
	p->instruction_length = 0;
	p->instructions = NULL;
	p->active_instruction = NULL;
	p->data_registers = NULL;
	p->boundary_length = 0;
	p->bsbits = NULL;

	return p;
}

void
part_free( part_t *p )
{
	int i;

	if (!p)
		return;

	/* id */
	free( p->id );

	/* signals */
	while (p->signals) {
		signal_t *s = p->signals;
		p->signals = s->next;
		signal_free( s );
	}

	/* saliases */
	while (p->saliases) {
		salias_t *sa = p->saliases;
		p->saliases = sa->next;
		salias_free( sa );
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
part_find_instruction( part_t *p, const char *iname )
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
part_find_data_register( part_t *p, const char *drname )
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

signal_t *
part_find_signal( part_t *p, const char *signalname )
{
	signal_t *s;
	salias_t *sa;

	if (!p || !signalname)
		return NULL;

	s = p->signals;
	while (s) {
		if (strcmp( signalname, s->name ) == 0)
			return s;
		s = s->next;
	}

	sa = p->saliases;
	while (sa) {
		if (strcmp( signalname, sa->name ) == 0)
			return sa->signal;
		sa = sa->next;
	}

	return NULL;
}

void
part_set_instruction( part_t *p, const char *iname )
{
	if (p)
		p->active_instruction = part_find_instruction( p, iname );
}

void
part_set_signal( part_t *p, signal_t *s, int out, int val )
{
	data_register *bsr;

	if (!p || !s)
		return;

	/* search for Boundary Scan Register */
	bsr = part_find_data_register( p, "BSR" );
	if (!bsr) {
		printf( _("%s(%s:%d) Boundary Scan Register (BSR) not found\n"), __FUNCTION__, __FILE__, __LINE__ );
		return;
	}

	/* setup signal */
	if (out) {
		int control;
		if (!s->output) {
			printf( _("signal '%s' cannot be set as output\n"), s->name );
			return;
		}
		bsr->in->data[s->output->bit] = val & 1;

		control = p->bsbits[s->output->bit]->control;
		if (control >= 0)
			bsr->in->data[control] = p->bsbits[s->output->bit]->control_value ^ 1;
	} else {
		if (!s->input) {
			printf( _("signal '%s' cannot be set as input\n"), s->name );
			return;
		}
		if (s->output)
			bsr->in->data[s->output->control] = p->bsbits[s->output->bit]->control_value;
	}
}

int
part_get_signal( part_t *p, signal_t *s )
{
	data_register *bsr;

	if (!p || !s)
		return -1;

	/* search for Boundary Scan Register */
	bsr = part_find_data_register( p, "BSR" );
	if (!bsr) {
		printf( _("%s(%s:%d) Boundary Scan Register (BSR) not found\n"), __FUNCTION__, __FILE__, __LINE__ );
		return -1;
	}

	if (!s->input) {
		printf( _("signal '%s' is not input signal\n"), s->name );
		return -1;
	}

	return bsr->out->data[s->input->bit];
}

void
part_print( part_t *p )
{
	char *instruction = NULL;
	char *dr = NULL;
	char format[100];

	if (!p)
		return;

	snprintf( format, 100, _("%%-%ds %%-%ds %%-%ds %%-%ds %%-%ds\n"), MAXLEN_MANUFACTURER, MAXLEN_PART, MAXLEN_STEPPING,
			MAXLEN_INSTRUCTION, MAXLEN_DATA_REGISTER );

	if (p->active_instruction) {
		instruction = p->active_instruction->name;
		if (p->active_instruction->data_register != NULL)
			dr = p->active_instruction->data_register->name;
	}
	if (instruction == NULL)
		instruction = _("(none)");
	if (dr == NULL)
		dr = _("(none)");
	printf( format, p->manufacturer, p->part, p->stepping, instruction, dr );
}

/* parts */

parts_t *
parts_alloc( void )
{
	parts_t *ps = malloc( sizeof *ps );
	if (!ps)
		return NULL;

	ps->len = 0;
	ps->parts = NULL;

	return ps;
}

void
parts_free( parts_t *ps )
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
parts_add_part( parts_t *ps, part_t *p )
{
	part_t **np = realloc( ps->parts, (ps->len + 1) * sizeof *ps->parts );

	if (!np)
		return 0;

	ps->parts = np;
	ps->parts[ps->len++] = p;

	return 1;
}

void
parts_set_instruction( parts_t *ps, const char *iname )
{
	int i;

	for (i = 0; i < ps->len; i++)
		ps->parts[i]->active_instruction = part_find_instruction( ps->parts[i], iname );
}

void
parts_print( parts_t *ps )
{
	int i;

	if (!ps)
		return;

	for (i = 0; i < ps->len; i++) {
		part_t *p = ps->parts[i];

		if (!p)
			continue;

		printf( _(" %3d "), i );
		part_print( p );
	}
}
