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

#ifndef JTAG_REGISTER_H
#define	JTAG_REGISTER_H

typedef struct tap_register {
	char *data;		/* (public, r/w) register data */
	int len;		/* (public, r/o) register length */
	char *string;		/* (private) string representation of register data */
} tap_register;

tap_register *register_alloc( int len );
void register_free( tap_register *tr );
tap_register *register_fill( tap_register *tr, int val );
const char *register_get_string( tap_register *tr );
tap_register *register_init( tap_register *tr, const char *value );
int register_compare( const tap_register *tr, const tap_register *tr2 );

#endif /* JTAG_REGISTER_H */
