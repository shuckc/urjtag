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

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "part.h"
#include "register.h"

static char *
get_token( char *buf )
{
	char *t = strtok( buf, " \f\n\r\t\v" );
	if (t && (*t == '#'))
		return NULL;
	return t;
}

part_t *
read_part( FILE *f, tap_register *idr )
{
	int line = 0;
	part_t *part;

	if (!f)
		return NULL;

	part = part_alloc();
	if (!part) {
		printf( _("Out of memory\n") );
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

		/* signal */
		if (strcmp( t, "pin" ) == 0 || strcmp( t, "signal" ) == 0) {
			signal_t *s;

			t = get_token( NULL );
			if (!t) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}

			s = signal_alloc( t );
			if (!s) {
				printf( _("(%d) out of memory\n"), line );
				continue;
			}
			s->next = part->signals;
			part->signals = s;

			continue;
		}

		/* register */
		if (strcmp( t, "register" ) == 0) {
			char *n = get_token( NULL );	/* register name */
			int l;
			data_register *dr;

			t = get_token( NULL );		/* register length */
			if (!n || !t) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}

			l = strtol( t, &t, 10 );
			if ((t && *t) || (l < 1)) {
				printf( _("(%d) invalid register length\n"), line );
				continue;
			}

			dr = data_register_alloc( n, l );
			if (!dr) {
				printf( _("(%d) out of memory\n"), line );
				continue;
			}

			t = get_token( NULL );
			if (t) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}

			dr->next = part->data_registers;
			part->data_registers = dr;

			/* Boundary Scan Register */
			if (strcmp( dr->name, "BSR" ) == 0) {
				int i;

				part->boundary_length = l;
				part->bsbits = malloc( part->boundary_length * sizeof *part->bsbits );
				if (!part->bsbits) {
					printf( _("(%d) out of memory\n"), line );
					continue;
				}
				for (i = 0; i < part->boundary_length; i++)
					part->bsbits[i] = NULL;
			}

			/* Device Identification Register */
			if (strcmp( dr->name, "DIR" ) == 0)
				register_init( dr->out, register_get_string( idr ) );

			continue;
		}

		/* instruction */
		if (strcmp( t, "instruction" ) == 0) {
			t = get_token( NULL );		/* 'length' or instruction name */
			if (!t) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}
			/* we need 'length' first */
			if ((strcmp( t, "length" ) != 0) && (part->instruction_length == 0)) {
				printf( _("(%d) instruction length missing\n"), line );
				continue;
			}

			if (strcmp( t, "length" ) == 0) {
				t = get_token( NULL );
				if (!t) {
					printf( _("(%d) parse error\n"), line );
					continue;
				}
				part->instruction_length = strtol( t, &t, 10 );
				if ((t && *t) || (part->instruction_length < 1)) {
					printf( _("(%d) invalid instruction length\n"), line );
					continue;
				}
			} else {
				char *n = t;		/* save instruction name */
				instruction *i;

				t = get_token( NULL );	/* instruction bits */
				if (!t || (strlen( t ) != part->instruction_length)) {
					printf( _("(%d) parse error\n"), line );
					continue;
				}

				i = instruction_alloc( n, part->instruction_length, t );
				if (!i) {
					printf( _("(%d) out of memory\n"), line );
					continue;
				}

				i->next = part->instructions;
				part->instructions = i;

				t = get_token( NULL );	/* data register */
				if (!t) {
					printf( _("(%d) parse error\n"), line );
					continue;
				}
				i->data_register = part_find_data_register( part, t );
				if (!i->data_register) {
					printf( _("(%d) unknown data register\n"), line );
					continue;
				}
			}

			t = get_token( NULL );
			if (t) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}

			continue;
		}

		/* bit */
		if (strcmp( t, "bit" ) == 0) {
			int bit;
			int type;
			int safe;
			data_register *bsr = part_find_data_register( part, "BSR" );

			if (!bsr) {
				printf( _("(%d) missing Boundary Scan Register (BSR)\n"), line );
				continue;
			}

			/* get bit number */
			t = get_token( NULL );
			bit = strtol( t, &t, 10 );
			if ((t && *t) || (bit < 0) || (bit >= bsr->in->len)) {
				printf( _("(%d) invalid boundary bit number\n"), line );
				continue;
			}
			if (part->bsbits[bit]) {
				printf( _("(%d) duplicate bit declaration\n"), line );
				continue;
			}

			/* get bit type */
			t = get_token( NULL );
			if (!t || (strlen( t ) != 1)) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}
			switch (*t) {
				case 'I':
					type = BSBIT_INPUT;
					break;
				case 'O':
					type = BSBIT_OUTPUT;
					break;
				case 'B':
					type = BSBIT_BIDIR;
					break;
				case 'C':
					type = BSBIT_CONTROL;
					break;
				case 'X':
					type = BSBIT_INTERNAL;
					break;
				default:
					printf( _("(%d) parse error\n"), line );
					continue;
			}

			/* get safe value */
			t = get_token( NULL );
			if (!t || (strlen( t ) != 1)) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}
			safe = (*t == '1') ? 1 : 0;
			bsr->in->data[bit] = safe;

			/* get bit name */
			t = get_token( NULL );
			if (!t) {
				printf( _("(%d) parse error\n"), line );
				continue;
			}

			/* allocate bsbit */
			part->bsbits[bit] = bsbit_alloc( bit, t, type, part->signals, safe );
			if (!part->bsbits[bit]) {
				printf( _("(%d) out of memory\n"), line );
				continue;
			}

			/* we have control bit? */
			t = get_token( NULL );
			if (t) {
				int control;

				control = strtol( t, &t, 10 );
				if ((t && *t) || (control < 0)) {
					printf( _("(%d) invalid control bit number\n"), line );
					continue;
				}
				part->bsbits[bit]->control = control;

				/* control value */
				t = get_token( NULL );
				if (!t || (strlen( t ) != 1)) {
					printf( _("(%d) parse error\n"), line );
					continue;
				}
				part->bsbits[bit]->control_value = (*t == '1') ? 1 : 0;

				/* control state */
				t = get_token( NULL );
				if (!t || (strlen( t ) != 1)) {
					printf( _("(%d) parse error\n"), line );
					continue;
				}
				if (*t != 'Z') {
					printf( _("(%d) parse error\n"), line );
					continue;
				}
				part->bsbits[bit]->control_state = BSBIT_STATE_Z;

				t = get_token( NULL );
				if (t) {
					printf( _("(%d) parse error\n"), line );
					continue;
				}

			}

			continue;
		}

		printf( _("(%d) parse error\n"), line );
	}

	return part;
}
