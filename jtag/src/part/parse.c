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
#include <stdio.h>

#include <jtag/part.h>

static char *
get_token( char *buf )
{
	char *t = strtok( buf, " \f\n\r\t\v" );
	if (t && (*t == '#'))
		return NULL;
	return t;
}

part *
read_part( FILE *f )
{
	int line = 0;

	part *part = part_alloc();
	if (!part) {
		printf( "out of memory\n" );
		return NULL;
	}

	for (;;) {
		char *t;
		char buf[1024];

		if (fgets( buf, 1024, f ) == NULL)
			break;

		line++;

		t = get_token( buf );
		if (!t)
			continue;

		/* pin */
		if (strcmp( t, "pin" ) == 0) {
			signal *s;

			t = get_token( NULL );
			if (!t) {
				printf( "(%d) parse error\n", line );
				continue;
			}

			s = signal_alloc( t );
			if (!s) {
				printf( "(%d) out of memory\n", line );
				continue;
			}
			s->next = part->signals;
			part->signals = s;

			continue;
		}

		/* instruction */
		if (strcmp( t, "instruction" ) == 0) {
			t = get_token( NULL );		/* 'length' or instruction name */
			if (!t) {
				printf( "(%d) parse error\n", line );
				continue;
			}
			/* we need 'length' first */
			if ((strcmp( t, "length" ) != 0) && (part->instruction_length == 0)) {
				printf( "(%d) instruction length missing\n", line );
				continue;
			}

			if (strcmp( t, "length" ) == 0) {
				t = get_token( NULL );
				if (!t) {
					printf( "(%d) parse error\n", line );
					continue;
				}
				part->instruction_length = strtol( t, &t, 10 );
				if ((t && *t) || (part->instruction_length < 1)) {
					printf( "(%d) invalid instruction length\n", line );
					continue;
				}
			} else {
				char *n = t;		/* save instruction name */
				instruction *i;

				t = get_token( NULL );
				if (!t || (strlen( t ) != part->instruction_length)) {
					printf( "(%d) parse error\n", line );
					continue;
				}

				i = instruction_alloc( n, part->instruction_length, t );
				if (!i) {
					printf( "(%d) out of memory\n", line );
					continue;
				}

				i->next = part->instructions;
				part->instructions = i;
			}

			t = get_token( NULL );
			if (t) {
				printf( "(%d) parse error\n", line );
				continue;
			}

			continue;
		}

		/* boundary */
		if (strcmp( t, "boundary" ) == 0) {
			int i;

			char *l = get_token( NULL );	/* 1st token */
			t = get_token( NULL );		/* 2nd token */
			if (!t || !l || (strcmp( l, "length" ) != 0)) {
				printf( "(%d) parse error\n", line );
				continue;
			}
			
			part->boundary_length = strtol( t, &t, 10 );
			if ((t && *t) || (part->boundary_length < 1)) {
				printf( "(%d) invalid boundary length\n", line );
				continue;
			}
			part->bsbits = malloc( part->boundary_length * sizeof *part->bsbits );
			if (!part->bsbits) {
				printf( "(%d) out of memory\n", line );
				continue;
			}
			for (i = 0; i < part->boundary_length; i++)
				part->bsbits[i] = NULL;

			part->bsr = register_alloc( part->boundary_length );
			part->prev_bsr = register_alloc( part->boundary_length );
			if (!part->bsr || !part->prev_bsr) {
				printf( "(%d) out of memory\n", line );
				continue;
			}

			t = get_token( NULL );
			if (t) {
				printf( "(%d) parse error\n", line );
				continue;
			}


			continue;
		}

		/* bit */
		if (strcmp( t, "bit" ) == 0) {
			int bit;
			int type;
			int safe;

			/* get bit number */
			t = get_token( NULL );
			bit = strtol( t, &t, 10 );
			if ((t && *t) || (bit < 0)) {
				printf( "(%d) invalid boundary bit number\n", line );
				continue;
			}
			if (part->bsbits[bit]) {
				printf( "(%d) duplicate bit declaration\n", line );
				continue;
			}

			/* get bit type */
			t = get_token( NULL );
			if (!t || (strlen( t ) != 1)) {
				printf( "(%d) parse error\n", line );
				continue;
			}
			switch (*t) {
				case 'I':
					type = BSBIT_INPUT;
					break;
				case 'O':
					type = BSBIT_OUTPUT;
					break;
				case 'C':
					type = BSBIT_CONTROL;
					break;
				case 'X':
					type = BSBIT_INTERNAL;
					break;
				default:
					printf( "(%d) parse error\n", line );
					continue;
			}

			/* get safe value */
			t = get_token( NULL );
			if (!t || (strlen( t ) != 1)) {
				printf( "(%d) parse error\n", line );
				continue;
			}
			safe = (*t == '1') ? 1 : 0;
			part->bsr->data[bit] = safe;

			/* get bit name */
			t = get_token( NULL );
			if (!t) {
				printf( "(%d) parse error\n", line );
				continue;
			}

			/* allocate bsbit */
			part->bsbits[bit] = bsbit_alloc( bit, t, type, part->signals, safe );
			if (!part->bsbits[bit]) {
				printf( "(%d) out of memory\n", line );
				continue;
			}

			/* we have control bit? */
			t = get_token( NULL );
			if (t) {
				int control;

				control = strtol( t, &t, 10 );
				if ((t && *t) || (control < 0)) {
					printf( "(%d) invalid control bit number\n", line );
					continue;
				}
				part->bsbits[bit]->control = control;

				/* control value */
				t = get_token( NULL );
				if (!t || (strlen( t ) != 1)) {
					printf( "(%d) parse error\n", line );
					continue;
				}
				part->bsbits[bit]->control_value = (*t == '1') ? 1 : 0;

				/* control state */
				t = get_token( NULL );
				if (!t || (strlen( t ) != 1)) {
					printf( "(%d) parse error\n", line );
					continue;
				}
				if (*t != 'Z') {
					printf( "(%d) parse error\n", line );
					continue;
				}
				part->bsbits[bit]->control_state = BSBIT_STATE_Z;

				t = get_token( NULL );
				if (t) {
					printf( "(%d) parse error\n", line );
					continue;
				}

			}

			continue;
		}

		printf( "(%d) parse error\n", line );
	}

	return part;
}
