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

part *
part_alloc( void )
{
	part *p = malloc( sizeof *p );
	if (!p)
		return NULL;

	p->signals = NULL;
	p->instruction_length = 0;
	p->instructions = NULL;
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

	/* bsbits */
	for (i = 0; i < p->boundary_length; i++)
		bsbit_free( p->bsbits[i] );
	free( p->bsbits );

	free( p );
}

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
