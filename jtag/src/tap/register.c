/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "register.h"

tap_register *
register_alloc( int len )
{
	tap_register *tr;

	if (len < 1)
		return NULL;

	tr = malloc( sizeof (tap_register) );
	if (!tr)
		return NULL;
	
	tr->data = malloc( len );
	if (!tr->data) {
		free( tr );
		return NULL;
	}

	tr->string = malloc( len + 1 );
	if (!tr->string) {
		free( tr->data );
		free( tr );
		return NULL;
	}

	tr->len = len;
	tr->string[len] = '\0';

	return tr;
}

tap_register *
register_duplicate( const tap_register *tr )
{
 	if (!tr)
		return	NULL;

	return register_init( register_alloc( tr->len ), register_get_string( tr ) );
}

void
register_free( tap_register *tr )
{
	if (tr) {
		free( tr->data );
		free( tr->string );
	}
	free( tr );
}

tap_register *
register_fill( tap_register *tr, int val )
{
	if (tr)
		memset( tr->data, val & 1, tr->len );

	return tr;
}

const char *
register_get_string( const tap_register *tr )
{
	int i;

	if (!tr)
		return NULL;

	for (i = 0; i < tr->len; i++)
		tr->string[tr->len - 1 - i] = (tr->data[i] & 1) ? '1' : '0';

	return tr->string;
}

tap_register *
register_init( tap_register *tr, const char *value )
{
	int i;

	const char *p;

	if (!value || !tr)
		return tr;

	p = strchr( value, '\0' );

	for (i = 0; i < tr->len; i++) {
		if (p == value)
			tr->data[i] = 0;
		else {
			p--;
			tr->data[i] = (*p == '0') ? 0 : 1;
		}
	}

	return tr;
}

int
register_compare( const tap_register *tr, const tap_register *tr2 )
{
	int i;

	if (!tr && !tr2)
		return 0;
	
	if (!tr || !tr2)
		return 1;

	if (tr->len != tr2->len)
		return 1;

	for (i = 0; i < tr->len; i++)
		if (tr->data[i] != tr2->data[i])
			return 1;

	return 0;
}

int
register_match( const tap_register *tr, const char *expr )
{
	int i;
	const char *s;

	if (!tr || !expr || (tr->len != strlen( expr )))
		return 0;

	s = register_get_string( tr );

	for (i = 0; i < tr->len; i++)
		if ((expr[i] != '?') && (expr[i] != s[i]))
			return 0;

	return 1;
}

tap_register *
register_inc( tap_register *tr )
{
	int i;

	if (!tr)
		return NULL;

	for (i = 0; i < tr->len; i++) {
		tr->data[i] ^= 1;
		
		if (tr->data[i] == 1)
			break;
	}

	return tr;
}

tap_register *
register_dec( tap_register *tr )
{
	int i;

	if (!tr)
		return NULL;

	for (i = 0; i < tr->len; i++) {
		tr->data[i] ^= 1;
		
		if (tr->data[i] == 0)
			break;
	}

	return tr;
}

tap_register *
register_shift_right( tap_register *tr, int shift )
{
	int i;

	if (!tr)
		return NULL;

	if (shift < 1)
		return tr;

	for (i = 0; i < tr->len; i++) {
		if (i + shift < tr->len)
			tr->data[i] = tr->data[i + shift];
		else
			tr->data[i] = 0;
	}

	return tr;
}

tap_register *
register_shift_left( tap_register *tr, int shift )
{
	int i;

	if (!tr)
		return NULL;

	if (shift < 1)
		return tr;

	for (i = tr->len - 1; i >= 0; i--) {
		if (i - shift >= 0)
			tr->data[i] = tr->data[i - shift];
		else
			tr->data[i] = 0;
	}

	return tr;
}
