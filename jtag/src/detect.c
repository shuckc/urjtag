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
#include <ctype.h>
#include <string.h>

#include <jtag/register.h>
#include <jtag/tap.h>
#include <jtag/ctrl.h>
#include <jtag/part.h>

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
	if (!file)
		return 0;

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

parts *
detect_parts( char *db_path )
{
	tap_register *zeros;
	tap_register *id;
	char data_path[1024];

	parts *ps = parts_alloc();
	if (!ps) {
		printf( __FUNCTION__ ": out of memory\n" );
		return NULL;
	}

	zeros = register_fill( register_alloc( 32 ), 0 );
	id = register_alloc( 32 );

	tap_reset();

	tap_capture_dr();
	for (;;) {
		tap_register *key;
		struct id_record idr;
		char *p;

		tap_shift_register( zeros, id, 0 );
		if (!register_compare( id, zeros )) {
			tap_clock( 1, 0 );			/* Exit1-DR */
			tap_clock( 1, 0 );			/* Update-DR */
			break;
		}
		printf( "%s\n", register_get_string( id ) );

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

		/* part definition file */
		p = strrchr( data_path, '/' );
		if (p)
			p[1] = '\0';
		else
			data_path[0] = '\0';
		strcat( data_path, idr.name );

		printf( "Reading file: %s ... ", data_path );
		{
			FILE *f = fopen( data_path, "r" );
			part *p = read_part( f );
			p->active_instruction = part_find_instruction( p, "IDCODE" );
			parts_add_part( ps, p );
		}
		printf( "done\n" );
	}

	register_free( zeros );
	register_free( id );

	return ps;
}

void
setup_address( part *p, unsigned int a )
{
	int i;
	char buff[10];

	for (i = 0; i < 26; i++) {
		sprintf( buff, "MA[%d]", i );
		part_set_signal( p, buff, 1, (a >> i) & 1 );
	}
}

void
setup_data( part *p, unsigned int d )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "MD[%d]", i );
		part_set_signal( p, buff, 1, (d >> i) & 1 );
	}
}

unsigned int
get_data( part *p )
{
	int i;
	char buff[10];
	unsigned int d = 0;

	for (i = 0; i < 32; i++) {
		sprintf( buff, "MD[%d]", i );
		d |= (unsigned int) (part_get_signal( p, buff ) << i);
	}

	return d;
}

void set_data_in( part *p )
{
	int i;
	char buff[10];

	for (i = 0; i < 32; i++) {
		sprintf( buff, "MD[%d]", i );
		part_set_signal( p, buff, 0, 0 );
	}
}

#define	AB_READ		0
#define	AB_WRITE	1
#define	AB_SETUP	2
#define	AB_HOLD		3

unsigned int
access_bus( part *p, int type, unsigned int a, unsigned int d )
{
	part_set_signal( p, "nCS[0]", 1, 0 );
	setup_address( p, a );
	
	switch (type) {
		case AB_READ:
			part_set_signal( p, "nOE", 1, 0 );
			part_set_signal( p, "nWE", 1, 1 );
			set_data_in( p );
			break;
		case AB_WRITE:
			part_set_signal( p, "nOE", 1, 1 );
			part_set_signal( p, "nWE", 1, 0 );
			setup_data( p, d );
			break;
		case AB_SETUP:
		case AB_HOLD:
			part_set_signal( p, "nOE", 1, 1 );
			part_set_signal( p, "nWE", 1, 1 );
			setup_data( p, d );
			break;
		default:
			printf( "access_bus: invalid type\n" );
			return 0;
	}

	tap_capture_dr();
	tap_shift_register( p->bsr, p->prev_bsr, 1 );

	return get_data( p );
}

unsigned int
access_rom( part *p, int type, unsigned int a, unsigned int d )
{
	return access_bus( p, type, a << 2, d );
}

void
program_flash( part *p, unsigned int a, unsigned int d )
{
	
	access_bus( p, AB_SETUP, a, 0x00400040 );
	access_bus( p, AB_WRITE, a, 0x00400040 );
	access_bus( p, AB_HOLD, a, 0x00400040 );

	access_bus( p, AB_SETUP, a, d );
	access_bus( p, AB_WRITE, a, d );
	access_bus( p, AB_HOLD, a, d );

	access_bus( p, AB_READ, 0, 0 );
	printf( "pf: %08X\n", access_bus( p, AB_READ, 0, 0 ) );
}

void
unlock( part *p, unsigned int a )
{
	access_bus( p, AB_SETUP, a, 0x00600060 );
	access_bus( p, AB_WRITE, a, 0x00600060 );
	access_bus( p, AB_HOLD, a, 0x00600060 );

	access_bus( p, AB_SETUP, a, 0x00D000D0 );
	access_bus( p, AB_WRITE, a, 0x00D000D0 );
	access_bus( p, AB_HOLD, a, 0x00D000D0 );
}

void
erase( part *p, unsigned int a )
{
	printf( "erase\n" );

	access_bus( p, AB_SETUP, a, 0x00200020 );
	access_bus( p, AB_WRITE, a, 0x00200020 );
	access_bus( p, AB_HOLD, a, 0x00200020 );

	access_bus( p, AB_SETUP, a, 0x00D000D0 );
	access_bus( p, AB_WRITE, a, 0x00D000D0 );
	access_bus( p, AB_HOLD, a, 0x00D000D0 );

	access_bus( p, AB_READ, 0, 0 );
	printf( "pf: %08X\n", access_bus( p, AB_READ, 0, 0 ) );
	sleep( 4 );
	access_bus( p, AB_READ, 0, 0 );
	printf( "pf: %08X\n", access_bus( p, AB_READ, 0, 0 ) );
	printf( "pf: %08X\n", access_bus( p, AB_READ, 0, 0 ) );
}

int
main( void )
{
	parts *ps;
	part *p;

	unsigned int max_erase_time;
	unsigned int dsize;
	unsigned int ebri;

	tap_init();

	tap_set_trst( 0 );
	tap_set_trst( 1 );

	ps = detect_parts( "../data" );

	if (ps->len == 0) {
		printf( "Not detected!!!\n" );
		return 0;
	}

	p = ps->parts[0];

	printf( "Setting up safe default values\n" );
	parts_set_instruction( ps, "SAMPLE/PRELOAD" );
	tap_capture_dr();
	tap_shift_register( p->bsr, p->prev_bsr, 1 );

printf( "%s\n", register_get_string( p->bsr ) );
printf( "%s\n", register_get_string( p->prev_bsr ) );

	parts_set_instruction( ps, "EXTEST" );

	access_bus( p, AB_SETUP, 0x00000000, 0x00500050 );
	access_bus( p, AB_WRITE, 0x00000000, 0x00500050 );
	access_bus( p, AB_HOLD, 0x00000000, 0x00500050 );

	access_bus( p, AB_SETUP, 0x00000000, 0x00980098 );
	access_bus( p, AB_WRITE, 0x00000000, 0x00980098 );
	access_bus( p, AB_HOLD, 0x00000000, 0x00980098 );

	access_rom( p, AB_READ, 0x10, 0 );
	printf( "read 0x10: %08X\n", access_rom( p, AB_READ, 0x11, 0 ) );
	printf( "read 0x11: %08X\n", access_rom( p, AB_READ, 0x12, 0 ) );
	printf( "read 0x12: %08X\n", access_rom( p, AB_READ, 0x25, 0 ) );


	printf( "read max_erase_time: %08X\n", max_erase_time = access_rom( p, AB_READ, 0x27, 0 ) );
	dsize = 1 << (access_rom( p, AB_READ, 0x2D, 0 ) & 0xFFFF);
	printf( "device size: %08X\n", dsize );
	ebri = access_rom( p, AB_READ, 0x2D, 0 );
	printf( "ebri: %08X\n", ebri );

	unlock( p, 0 );
	erase( p, 0 );

	{
		FILE *f = fopen( "brux.b", "r" );
		unsigned int d;
		unsigned int a = 0;
		

		while (fread( &d, sizeof d, 1, f ) == 1) {
			printf( "adr: %08X\n", a );
			program_flash( p, a, d );
			a += 4;
		}
		fclose( f );
	}

	access_bus( p, AB_SETUP, 0, 0x00FF00FF );
	access_bus( p, AB_WRITE, 0, 0x00FF00FF );
	access_bus( p, AB_HOLD, 0, 0x00FF00FF );

	{
		FILE *f = fopen( "brux.b", "r" );
		unsigned int d;
		unsigned int a = 0;
		

		while (fread( &d, sizeof d, 1, f ) == 1) {
			unsigned int x;
			printf( "adr: %08X\n", a );
			access_bus( p, AB_READ, a, 0 );
			x = access_bus( p, AB_READ, a, 0 );
			if (x != d) {
				printf( "error read = %08X, expect = %08X\n", x, d );
				exit( 0 );
			}
			a += 4;
		}
		fclose( f );
	}
	exit( 0 );


	unlock( p, 0 );
	erase( p, 0 );

	{
		unsigned int i;
		for (i = 0; i < 256 * 1024; i += 4) {
			printf( "program: %08X\n", i );
			program_flash( p, i, i );
		}
	}

	access_bus( p, AB_SETUP, 0, 0x00FF00FF );
	access_bus( p, AB_WRITE, 0, 0x00FF00FF );
	access_bus( p, AB_HOLD, 0, 0x00FF00FF );

	{
		unsigned int i;
		unsigned int j;
		for (i = 0; i < 256 * 1024; i += 4) {
			printf( "read: %08X\n", i );
			access_bus( p, AB_READ, i, 0 );
			j = access_bus( p, AB_READ, i, 0 );
			if (i != j)
				printf( "error: a = %08X, d = %08X\n", i, j );
		}
	}


	{
		unsigned int b;
		for (b = 1; b < 128; b++) {
			unsigned int x;
			unsigned int d;

			access_bus( p, AB_SETUP, 0x00000000, 0x00500050 );
			access_bus( p, AB_WRITE, 0x00000000, 0x00500050 );
			access_bus( p, AB_HOLD, 0x00000000, 0x00500050 );

			access_bus( p, AB_SETUP, 0x00000000, 0x00980098 );
			access_bus( p, AB_WRITE, 0x00000000, 0x00980098 );
			access_bus( p, AB_HOLD, 0x00000000, 0x00980098 );

			unlock( p, b * 256 * 1024 );
			erase( p, b * 256 * 1024 );

			program_flash( p, b * 256 * 1024, 0x55AA55AA );

			access_bus( p, AB_SETUP, 0, 0x00FF00FF );
			access_bus( p, AB_WRITE, 0, 0x00FF00FF );
			access_bus( p, AB_HOLD, 0, 0x00FF00FF );

			printf( "Test: %08X\n", b * 256 * 1024 );

			for (x = 1; x < 128; x++) {
				access_bus( p, AB_READ, x * 256 * 1024, 0 );
				d = access_bus( p, AB_READ, x * 256 * 1024, 0 );
				if (d == 0x55AA55AA)
					printf( "  at %08X\n", x * 256 * 1024 );
			}

			access_bus( p, AB_SETUP, 0x00000000, 0x00500050 );
			access_bus( p, AB_WRITE, 0x00000000, 0x00500050 );
			access_bus( p, AB_HOLD, 0x00000000, 0x00500050 );

			access_bus( p, AB_SETUP, 0x00000000, 0x00980098 );
			access_bus( p, AB_WRITE, 0x00000000, 0x00980098 );
			access_bus( p, AB_HOLD, 0x00000000, 0x00980098 );

			erase( p, b * 256 * 1024 );	
		}
	}


	access_bus( p, AB_SETUP, 0, 0x00FF00FF );
	access_bus( p, AB_WRITE, 0, 0x00FF00FF );
	access_bus( p, AB_HOLD, 0, 0x00FF00FF );

	{
		unsigned int i;
		unsigned int j;
		for (i = 0; i < 256 * 1024; i += 4) {
			printf( "read: %08X\n", i );
			access_bus( p, AB_READ, i, 0 );
			j = access_bus( p, AB_READ, i, 0 );
			if (i != j)
				printf( "error: a = %08X, d = %08X\n", i, j );
		}
	}



#if 0
	program_flash( p, 0x0, 0xE1A00000 );
	program_flash( p, 0x4, 0xE1A00000 );
	program_flash( p, 0x8, 0xE1A00000 );
	program_flash( p, 0xC, 0xE1A00000 );
	program_flash( p, 0x10, 0xE1A00000 );
	program_flash( p, 0x14, 0xEAFFFFF9 );
#endif

	//erase( p, 0 );

	access_bus( p, AB_SETUP, 0, 0x00FF00FF );
	access_bus( p, AB_WRITE, 0, 0x00FF00FF );
	access_bus( p, AB_HOLD, 0, 0x00FF00FF );

	access_bus( p, AB_READ, 0, 0 );
	printf( "read data from 0x00: %08X\n", access_bus( p, AB_READ, 0x04, 0 ) );
	printf( "read data from 0x04: %08X\n", access_bus( p, AB_READ, 0x08, 0 ) );
	printf( "read data from 0x08: %08X\n", access_bus( p, AB_READ, 0x0C, 0 ) );
	printf( "read data from 0x0C: %08X\n", access_bus( p, AB_READ, 0x10, 0 ) );
	printf( "read data from 0x10: %08X\n", access_bus( p, AB_READ, 0x14, 0 ) );
	printf( "read data from 0x14: %08X\n", access_bus( p, AB_READ, 0x18, 0 ) );
	printf( "read data from 0x18: %08X\n", access_bus( p, AB_READ, 0x1C, 0 ) );
	printf( "read data from 0x1C: %08X\n", access_bus( p, AB_READ, 0x20, 0 ) );
	printf( "read data from 0x20: %08X\n", access_bus( p, AB_READ, 0x24, 0 ) );
	printf( "read data from 0x24: %08X\n", access_bus( p, AB_READ, 0x28, 0 ) );
	printf( "read data from 0x28: %08X\n", access_bus( p, AB_READ, 0x2C, 0 ) );
	printf( "read data from 0x2C: %08X\n", access_bus( p, AB_READ, 0x30, 0 ) );
	printf( "read data from 0x30: %08X\n", access_bus( p, AB_READ, 0x34, 0 ) );

#if 0
	tap_capture_dr();
a:
	printf( "Jedna\n" );
	part_set_signal( sa1110, "MA[24]", 1, 1 );
	tap_shift_register( sa1110->bsr, sa1110->prev_bsr, 1 );

	tap_capture_dr();
	sleep(5);
	printf( "Nula\n" );
	part_set_signal( sa1110, "MA[24]", 1, 0 );
	tap_shift_register( sa1110->bsr, sa1110->prev_bsr, 1 );

	tap_capture_dr();
	sleep(5);
	goto a;


	printf( "%s\n", register_get_string( sa1110->bsr ) );
	printf( "%s\n", register_get_string( sa1110->prev_bsr ) );
//	for (i = 0; i < s1110->boundary_length; i++) {
//		
//	}
#endif

	parts_free( ps );

	tap_reset();

	tap_done();

	return 0;
}
