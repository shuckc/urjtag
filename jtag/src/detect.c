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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "register.h"
#include "tap.h"
#include "cable.h"
#include "part.h"
#include "chain.h"

#include "jtag.h"

struct id_record {
	char name[20];
	char fullname[100];
};

static int
find_record( char *filename, tap_register *key, struct id_record *idr )
{
	FILE *file;
	tap_register *tr;
	int r = 0;

	file = fopen( filename, "r" );
	if (!file) {
		printf( "Cannot open %s\n", filename );
		return 0;
	}

	tr = register_alloc( key->len );

	for (;;) {
		char *p;
		char *s;
		char line[1024];

		if (fgets( line, 1024, file ) == NULL)
			break;

		/* remove comment and nl from the line */
		p = strpbrk( line, "#\n" );
		if (p)
			*p = '\0';

		p = line;

		/* skip whitespace */
		while (*p && isspace(*p))
			p++;

		/* remove ending whitespace */
		s = strchr( p, '\0' );
		while (s != p) {
			if (!isspace(*--s))
				break;
			*s = '\0';
		}	

		/* line is empty? */
		if (!*p)
			continue;

		/* find end of field */
		s = p;
		while (*s && !isspace(*s))
			s++;
		if (*s)
			*s++ = '\0';

		/* test field length */
		if (strlen( p ) != key->len)
			continue;

		/* match */
		register_init( tr, p );
		if (register_compare( tr, key ))
			continue;

		/* next field */
		p = s;

		/* skip whitespace */
		while (*p && isspace(*p))
			p++;

		/* line is empty? */
		if (!*p)
			continue;

		/* find end of field */
		s = p;
		while (*s && !isspace(*s))
			s++;
		if (*s)
			*s++ = '\0';

		/* test field length */
		if (strlen( p ) >= sizeof idr->name)
			continue;

		/* copy name */
		strcpy( idr->name, p );

		/* next field */
		p = s;

		/* skip whitespace */
		while (*p && isspace(*p))
			p++;

		/* line is empty? */
		if (!*p)
			continue;

		/* test field length */
		if (strlen( p ) >= sizeof idr->fullname)
			continue;

		/* copy fullname */
		strcpy( idr->fullname, p );

		r = 1;
		break;
	}

	fclose( file );

	register_free( tr );

	return r;
}

parts_t *
detect_parts( chain_t *chain, char *db_path )
{
	char data_path[1024];
	char manufacturer[MAXLEN_MANUFACTURER + 1];
	char partname[MAXLEN_PART + 1];
	char stepping[MAXLEN_STEPPING + 1];

	tap_register *zeros = register_fill( register_alloc( 32 ), 0 );
	tap_register *ones = register_fill( register_alloc( 32 ), 1 );
	tap_register *id = register_alloc( 32 );
	parts_t *ps = parts_alloc();

	if (!zeros || !ones || !id || !ps) {
		printf( "%s: out of memory\n", __FUNCTION__ );

		register_free( zeros );
		register_free( ones );
		register_free( id );
		parts_free( ps );
		return NULL;
	}

	tap_reset( chain );

	tap_capture_dr( chain );
	for (;;) {
		tap_register *key;
		struct id_record idr;
		char *p;
		FILE *f;
		part_t *part;

		tap_shift_register( chain, zeros, id, 0 );
		if (!register_compare( id, zeros ))
			break;				/* end of chain */

		if (!register_compare( ones, id )) {
			printf( "%s: bad JTAG connection (TDO is 1)\n", __FUNCTION__ );
			break;
		}

		printf( "Device Id: %s\n", register_get_string( id ) );

		strcpy( data_path, db_path );		/* FIXME: Buffer overrun */

		/* manufacturers */
		strcat( data_path, "/MANUFACTURERS" );

		key = register_alloc( 11 );
		memcpy( key->data, &id->data[1], key->len );
		if (!find_record( data_path, key, &idr )) {
			printf( "  Unknown manufacturer!\n" );
			register_free( key );
			continue;
		}
		register_free( key );

		printf( "  Manufacturer: %s\n", idr.fullname );
		if (strlen( idr.fullname ) > MAXLEN_MANUFACTURER)
			printf( "Warning: Manufacturer too long\n" );
		strncpy( manufacturer, idr.fullname, MAXLEN_MANUFACTURER );
		manufacturer[MAXLEN_MANUFACTURER] = '\0';

		/* parts */
		p = strrchr( data_path, '/' );
		if (p)
			p[1] = '\0';
		else
			data_path[0] = '\0';
		strcat( data_path, idr.name );
		strcat( data_path, "/PARTS" );

		key = register_alloc( 16 );
		memcpy( key->data, &id->data[12], key->len );
		if (!find_record( data_path, key, &idr )) {
			printf( "  Unknown part!\n" );
			register_free( key );
			continue;
		}
		register_free( key );

		printf( "  Part:         %s\n", idr.fullname );
		if (strlen( idr.fullname ) > MAXLEN_PART)
			printf( "Warning: Part too long\n" );
		strncpy( partname, idr.fullname, MAXLEN_PART );
		partname[MAXLEN_PART] = '\0';

		/* steppings */
		p = strrchr( data_path, '/' );
		if (p)
			p[1] = '\0';
		else
			data_path[0] = '\0';
		strcat( data_path, idr.name );
		strcat( data_path, "/STEPPINGS" );

		key = register_alloc( 4 );
		memcpy( key->data, &id->data[28], key->len );
		if (!find_record( data_path, key, &idr )) {
			printf( "  Unknown stepping!\n" );
			register_free( key );
			continue;
		}
		register_free( key );

		printf( "  Stepping:     %s\n", idr.fullname );
		if (strlen( idr.fullname ) > MAXLEN_STEPPING)
			printf( "Warning: Stepping too long\n" );
		strncpy( stepping, idr.fullname, MAXLEN_STEPPING );
		stepping[MAXLEN_STEPPING] = '\0';

		/* part definition file */
		p = strrchr( data_path, '/' );
		if (p)
			p[1] = '\0';
		else
			data_path[0] = '\0';
		strcat( data_path, idr.name );

		printf( "  Filename:     %s\n", data_path );
		f = fopen( data_path, "r" );
		part = read_part( f, id );
		if (part) {
			strcpy( part->manufacturer, manufacturer );
			strcpy( part->part, partname );
			strcpy( part->stepping, stepping );
			part->active_instruction = part_find_instruction( part, "IDCODE" );
			parts_add_part( ps, part );
		} else {
			printf( "%s(%s:%d) Error: part read failed\n", __FUNCTION__, __FILE__, __LINE__ );
			exit( 1 );
		}
		if (f)
			fclose( f );
	}

	chain_clock( chain, 1, 0 );		/* Exit1-DR */
	chain_clock( chain, 1, 0 );		/* Update-DR */

	register_free( zeros );
	register_free( ones );
	register_free( id );

	return ps;
}
