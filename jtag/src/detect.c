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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <brux/cmd.h>

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
		printf( _("Cannot open %s\n"), filename );
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

int
detect_parts( chain_t *chain, char *db_path )
{
	int irlen;
	tap_register *ir;
	int chlen;
	tap_register *one;
	tap_register *ones;
	tap_register *br;
	tap_register *id;
	parts_t *ps;
	int i;

	char data_path[1024];
	char *cmd[3] = {"script", data_path, NULL};
	char manufacturer[MAXLEN_MANUFACTURER + 1];
	char partname[MAXLEN_PART + 1];
	char stepping[MAXLEN_STEPPING + 1];

	/* Detect IR length */
	tap_reset( chain );
	tap_capture_ir( chain );
	irlen = detect_register_size( chain );
	if (irlen < 1)
		return 0;

	printf( _("IR length: %d\n"), irlen );

	/* Allocate IR */
	ir = register_fill( register_alloc( irlen ), 1 );
	if (ir == NULL) {
		printf( _("out of memory\n") );
		return 0;
	}

	tap_shift_register( chain, ir, NULL, 1 );
	register_free( ir );

	/* Detect chain length */
	tap_capture_dr( chain );
	chlen = detect_register_size( chain );
	if (chlen < 1) {
		printf( _("Unable to detect JTAG chain length\n") );
		return 0;
	}
	printf( _("Chain length: %d\n"), chlen );

	/* Allocate registers and parts */
	one = register_fill( register_alloc( 1 ), 1 );
	ones = register_fill( register_alloc( 31 ), 1 );
	br = register_alloc( 1 );
	id = register_alloc( 32 );
	ps = parts_alloc();
	if (!one || !ones || !br || !id || !ps) {
		printf( _("out of memory\n") );

		register_free( one );
		register_free( ones );
		register_free( br );
		register_free( id );
		parts_free( ps );
		return 0;
	}
	chain->parts = ps;
	chain->active_part = 0;

	/* Detect parts */
	tap_reset( chain );
	tap_capture_dr( chain );

	for (i = 0; i < chlen; i++) {
		part_t *part;
		tap_register *did = br;		/* detected id (length is 1 or 32) */
		tap_register *key;
		struct id_record idr;
		char *p;

		tap_shift_register( chain, one, br, 0 );
		if (register_compare( one, br ) == 0) {
			/* part with id */
			tap_shift_register( chain, ones, id, 0 );
			register_shift_left( id, 1 );
			id->data[0] = 1;
			did = id;
		}

		printf( _("Device Id: %s\n"), register_get_string( did ) );
		part = part_alloc( did );
		if (part == NULL) {
			printf( _("Out of memory\n") );
			break;
		}
		parts_add_part( ps, part );

		if (did == br)
			continue;

		/* find JTAG declarations for a part with id */

		strcpy( data_path, db_path );		/* FIXME: Buffer overrun */

		/* manufacturers */
		strcat( data_path, "/MANUFACTURERS" );

		key = register_alloc( 11 );
		memcpy( key->data, &id->data[1], key->len );
		if (!find_record( data_path, key, &idr )) {
			printf( _("  Unknown manufacturer!\n") );
			register_free( key );
			continue;
		}
		register_free( key );

		printf( _("  Manufacturer: %s\n"), idr.fullname );
		if (strlen( idr.fullname ) > MAXLEN_MANUFACTURER)
			printf( _("Warning: Manufacturer too long\n") );
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
			printf( _("  Unknown part!\n") );
			register_free( key );
			continue;
		}
		register_free( key );

		printf( _("  Part:         %s\n"), idr.fullname );
		if (strlen( idr.fullname ) > MAXLEN_PART)
			printf( _("Warning: Part too long\n") );
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
			printf( _("  Unknown stepping!\n") );
			register_free( key );
			continue;
		}
		register_free( key );

		printf( _("  Stepping:     %s\n"), idr.fullname );
		if (strlen( idr.fullname ) > MAXLEN_STEPPING)
			printf( _("Warning: Stepping too long\n") );
		strncpy( stepping, idr.fullname, MAXLEN_STEPPING );
		stepping[MAXLEN_STEPPING] = '\0';

		/* part definition file */
		p = strrchr( data_path, '/' );
		if (p)
			p[1] = '\0';
		else
			data_path[0] = '\0';
		strcat( data_path, idr.name );

		printf( _("  Filename:     %s\n"), data_path );

		/* run JTAG declarations */
		chain->active_part = ps->len - 1;
		strcpy( part->manufacturer, manufacturer );
		strcpy( part->part, partname );
		strcpy( part->stepping, stepping );
		cmd_run( cmd );
		if (part->active_instruction == NULL)
			part->active_instruction = part_find_instruction( part, "IDCODE" );
	}

	for (i = 0; i < 32; i++) {
		tap_shift_register( chain, one, br, 0 );
		if (register_compare( one, br ) != 0) {
			printf( _("Error: Unable to detect JTAG chain end!\n") );
			break;
		}
	}
	tap_shift_register( chain, one, NULL, 1 );

	register_free( one );
	register_free( ones );
	register_free( br );
	register_free( id );

	return ps->len;
}
