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

#include <jtag/data_register.h>

data_register *
data_register_alloc( const char *name, int len )
{
	data_register *dr;

	if (!name)
		return NULL;

	dr = malloc( sizeof *dr );
	if (!dr)
		return NULL;

	dr->name = strdup( name );
	if (!dr->name) {
		free( dr );
		return NULL;
	}

	dr->value = register_alloc( len );
	dr->oldval = register_alloc( len );
	if (!dr->value || dr->oldval) {
		free( dr->value );
		free( dr->oldval );
		free( dr->name );
		free( dr );
		return NULL;
	}

	dr->next = NULL;

	return dr;
}

void
data_register_free( data_register *dr )
{
	if (!dr)
		return;

	free( dr->name );
	register_free( dr->value );
	register_free( dr->oldval );
	free( dr );
}
