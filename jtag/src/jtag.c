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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "part.h"
#include "cable.h"
#include "tap.h"

#include "detect.h"
#include "bus.h"

#ifndef HAVE_GETLINE
ssize_t getline( char **lineptr, size_t *n, FILE *stream );
#endif

cable_driver_t *cable = NULL;
parts *ps = NULL;
bus_driver_t *bus_driver = NULL;

static char *
get_token( char *buf )
{
	return strtok( buf, " \f\n\r\t\v" );
}

void detectflash( parts *ps );
void readmem( parts *ps, FILE *f, uint32_t addr, uint32_t len );
void flashmem( parts *ps, FILE *f, uint32_t addr );
void flashmsbin( parts *ps, FILE *f );

void help( const char *cmd );

void discovery( const char *filename );

int jtag_parse_file( const char *filename );

#define	JTAGDIR		".jtag"
#define	HISTORYFILE	"history"
#define	RCFILE		"rc"

void
jtag_create_jtagdir( void )
{
	char *home = getenv( "HOME" );
	char *jdir;

	if (!home)
		return;

	jdir = malloc( strlen(home) + strlen(JTAGDIR) + 2 );	/* "/" and trailing \0 */
	if (!jdir)
		return;

	strcpy( jdir, home );
	strcat( jdir, "/" );
	strcat( jdir, JTAGDIR );

	/* Create the directory if it doesn't exists. */
	mkdir( jdir, 0755 );

	free( jdir );
}
						 
void
jtag_load_history( void )
{
	char *home = getenv( "HOME" );
	char *file;

	using_history();

	if (!home)
		return;

	file = malloc( strlen(home) + strlen(JTAGDIR) + strlen(HISTORYFILE) + 3 );	/* 2 x "/" and trailing \0 */
	if (!file)
		return;

	strcpy( file, home );
	strcat( file, "/" );
	strcat( file, JTAGDIR );
	strcat( file, "/" );
	strcat( file, HISTORYFILE );

	read_history( file );

	free( file );
}

void
jtag_save_history( void )
{
	char *home = getenv( "HOME" );
	char *file;

	if (!home)
		return;

	file = malloc( strlen(home) + strlen(JTAGDIR) + strlen(HISTORYFILE) + 3);	/* 2 x "/" and trailing \0 */
	if (!file)
		return;

	strcpy( file, home );
	strcat( file, "/" );
	strcat( file, JTAGDIR );
	strcat( file, "/");
	strcat( file, HISTORYFILE );

	write_history( file );

	free( file );
}

int
jtag_parse_line( char *line )
{
		char *t;

		if (!line || !(strlen( line ) > 0))
			return 1;

		t = get_token( line );
		if (!t)
			return 1;

		if (strcmp( t, "quit" ) == 0) {
			if (get_token( NULL )) {
				printf( "quit: syntax error\n\nType \"help\" for help.\n\n" );
				return 1;
			}
			return 0;
		}

		if (strcmp( t, "help" ) == 0) {
			t = get_token( NULL );
			if (get_token( NULL ))
				printf( "help: Syntax error!\n" );
			else
				help( t );
			return 1;
		}

		if (strcmp( t, "frequency" ) == 0) {
			uint32_t freq;

			t = get_token( NULL );
			if (!t) {
				printf( "Missing argument(s)\n" );
				return 1;
			}
			if ((sscanf( t, "0x%x", &freq ) != 1) && (sscanf( t, "%u", &freq ) != 1)) {
				printf( "syntax error\n" );
				return 1;
			}

			if (get_token( NULL )) {
				printf( "frequency: syntax error\n" );
				return 1;
			}

			printf( "Setting TCK frequency to %u Hz\n", freq );
			frequency = freq;

			return 1;
		}

		if (strcmp( t, "cable" ) == 0) {
			int i;
			unsigned int port;

			t = get_token( NULL );
			if (!t) {
				printf( "Missing argument(s)\n" );
				return 1;
			}
			if (strcmp( t, "parallel" ) != 0) {
				printf( "syntax error!\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "Missing argument(s)\n" );
				return 1;
			}
			if ((sscanf( t, "0x%x", &port ) != 1) && (sscanf( t, "%d", &port ) != 1)) {
				printf( "syntax error\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "Missing argument(s)\n" );
				return 1;
			}

			if (get_token( NULL )) {
				printf( "syntax error!\n" );
				return 1;
			}

			if (strcmp( t, "none" ) == 0) {
				printf( "Changed cable to 'none'\n" );
				cable = NULL;
				return 1;
			}
			
			for (i = 0; cable_drivers[i]; i++)
				if (strcmp( t, cable_drivers[i]->name ) == 0)
					break;

			if (!cable_drivers[i]) {
				printf( "Unknown cable: %s\n", t );
				return 1;
			}

			cable = cable_drivers[i];
			printf( "Initializing %s on parallel port at 0x%x\n", cable->description, port );
			if (!cable->init( port )) {
				printf( "Error: Cable driver initialization failed!\n" );
				cable = NULL;
				return 1;
			}

			cable->set_trst( 0 );
			cable->set_trst( 1 );
			tap_reset();

			return 1;
		}

		if (strcmp( t, "discovery" ) == 0) {
			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "discovery: missing filename\n" );
				return 1;
			}
			if (get_token( NULL )) {
				printf( "syntax error!\n" );
				return 1;
			}
			discovery( t );
			return 1;
		}

		if (strcmp( t, "detect" ) == 0) {
			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (get_token( NULL )) {
				printf( "detect: syntax error\n" );
				return 1;
			}

			parts_free( ps );
			ps = detect_parts( JTAG_DATA_DIR );
			if (!ps->len) {
				parts_free( ps );
				ps = NULL;
				return 1;
			}
			parts_set_instruction( ps, "SAMPLE/PRELOAD" );
			parts_shift_instructions( ps );
			parts_shift_data_registers( ps );
			parts_set_instruction( ps, "BYPASS" );
			parts_shift_instructions( ps );
			if (strcmp( ps->parts[0]->part, "SA1110" ) == 0)
				bus_driver = &sa1110_bus_driver;
			if (strcmp( ps->parts[0]->part, "PXA250" ) == 0)
				bus_driver = &pxa250_bus_driver;
			if (strcmp( ps->parts[0]->part, "IXP425" ) == 0)
				bus_driver = &ixp425_bus_driver;
			return 1;
		}

		if (strcmp( t, "flashmem" ) == 0) {
			FILE *f;
			int msbin = 0;
			uint32_t addr = 0;

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "flashmem: Missing argument(s)\n" );
				return 1;
			}
			if (strcmp( t, "msbin" ) != 0) {
				if ((sscanf( t, "0x%x", &addr ) != 1) && (sscanf( t, "%d", &addr ) != 1)) {
					printf( "error\n" );
					return 1;
				}
				printf( "0x%08X\n", addr );
			} else
				msbin = 1;
			/* filename */
			t = get_token( NULL );
			if (!t) {
				printf( "flashmem: missing filename\n" );
				return 1;
			}
			if (get_token( NULL )) {
				printf( "syntax error!\n" );
				return 1;
			}
			f = fopen( t, "r" );
			if (!f) {
				printf( "Unable to open file `%s'!\n", t );
				return 1;
			}
			if (msbin) 
				flashmsbin( ps, f );
			else
				flashmem( ps, f, addr );
			fclose( f );
			return 1;
		}

		if (strcmp( t, "readmem" ) == 0) {
			FILE *f;
			uint32_t addr = 0;
			uint32_t len = 0;

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "flashmem: Missing argument(s)\n" );
				return 1;
			}
			if ((sscanf( t, "0x%x", &addr ) != 1) && (sscanf( t, "%d", &addr ) != 1)) {
				printf( "syntax error\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "flashmem: Missing argument(s)\n" );
				return 1;
			}
			if ((sscanf( t, "0x%x", &len ) != 1) && (sscanf( t, "%d", &len ) != 1)) {
				printf( "syntax error\n" );
				return 1;
			}

			/* filename */
			t = get_token( NULL );
			if (!t) {
				printf( "flashmem: missing filename\n" );
				return 1;
			}
			if (get_token( NULL )) {
				printf( "syntax error!\n" );
				return 1;
			}

			f = fopen( t, "w" );
			if (!f) {
				printf( "Unable to create file `%s'!\n", t );
				return 1;
			}
			readmem( ps, f, addr, len );

			fclose( f );
			return 1;
		}

		if (strcmp( t, "detectflash" ) == 0) {
			if (get_token( NULL )) {
				printf( "detectflash: syntax error\n" );
				return 1;
			}

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			detectflash( ps );
			return 1;
		}

		if (strcmp( t, "print" ) == 0) {
			if (get_token( NULL )) {
				printf( "print: syntax error\n" );
				return 1;
			}

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			parts_print( ps, 1 );
			return 1;
		}

		if (strcmp( t, "instruction" ) == 0) {
			int n;

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "instruction: syntax error\n" );
				return 1;
			}

			n = strtol( t, &t, 10 );
			if (t && *t) {
				printf( "instruction: syntax error\n" );
				return 1;
			}

			if ((n < 0) || (n >= ps->len)) {
				printf( "instruction: invalid part number\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "instruction: missing instruction name\n" );
				return 1;
			}

			if (get_token( NULL )) {
				printf( "instruction: syntax error\n" );
				return 1;
			}

			part_set_instruction( ps->parts[n], t );
			if (ps->parts[n]->active_instruction == NULL)
				printf( "instruction: unknown instruction %s\n", t );

			return 1;
		}

		if (strcmp( t, "shift" ) == 0) {
			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			t = get_token( NULL );

			if (t && (strcmp( t, "ir" ) == 0)) {
				parts_shift_instructions( ps );
				return 1;
			}

			if (t && (strcmp( t, "dr" ) == 0)) {
				parts_shift_data_registers( ps );
				return 1;
			}

			printf( "shift: syntax error\n" );
			return 1;
		}

		if (strcmp( t, "dr" ) == 0) {
			int n;
			int dir;
			tap_register *r;

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "dr: syntax error\n" );
				return 1;
			}

			n = strtol( t, &t, 10 );
			if (t && *t) {
				printf( "dr: syntax error\n" );
				return 1;
			}
			
			if ((n < 0) || (n >= ps->len)) {
				printf( "dr: invalid part number\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t)
				dir = 1;
			else {
				if (strcmp( t, "in" ) == 0)
					dir = 0;
				else if (strcmp( t, "out" ) == 0)
					dir = 1;
				else {
					printf( "dr: syntax error\n" );
					return 1;
				}

				if (get_token( NULL )) {
					printf( "dr: syntax error\n" );
					return 1;
				}
			}

			if (dir)
				r = ps->parts[n]->active_instruction->data_register->out;
			else
				r = ps->parts[n]->active_instruction->data_register->in;
			printf( "%s\n", register_get_string( r ) );

			return 1;
		}

		if (strcmp( t, "set" ) == 0) {
			int n;
			int data;
			int dir;
			char *s;

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t || strcmp( t, "signal" ) != 0) {
				printf( "set: syntax error\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "set: syntax error\n" );
				return 1;
			}
			n = strtol( t, &t, 10 );
			if (t && *t) {
				printf( "set: syntax error\n" );
				return 1;
			}

			if ((n < 0) || (n >= ps->len)) {
				printf( "set: invalid part number\n" );
				return 1;
			}

			s = get_token( NULL );		/* signal name */
			if (!s) {
				printf( "set: syntax error\n" );
				return 1;
			}

			t = get_token( NULL );		/* direction */
			if (!t || (strcmp( t, "in" ) != 0 && strcmp( t, "out" ) != 0)) {
				printf( "set: syntax error\n" );
				return 1;
			}

			dir = (strcmp( t, "in" ) == 0) ? 0 : 1;
			if (dir) {
				t = get_token( NULL );
				data = strtol( t, &t, 10 );
				if (t && *t) {
					printf( "set: syntax error\n" );
					return 1;
				}

				if ((data < 0) || (data > 1)) {
					printf( "set: invalid data value\n" );
					return 1;
				}
			} else
				data = 0;

			if (get_token( NULL )) {
				printf( "set: syntax error\n" );
				return 1;
			}

			part_set_signal( ps->parts[n], s, dir, data );

			return 1;
		}

		if (strcmp( t, "get" ) == 0) {
			int n;
			int data;

			if (!cable) {
				printf( "Error: Cable not configured. Use 'cable' command first!\n" );
				return 1;
			}

			if (!ps) {
				printf( "Run \"detect\" first.\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t || strcmp( t, "signal" ) != 0) {
				printf( "get: syntax error\n" );
				return 1;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "get: syntax error\n" );
				return 1;
			}
			n = strtol( t, &t, 10 );
			if (t && *t) {
				printf( "get: syntax error\n" );
				return 1;
			}

			if ((n < 0) || (n >= ps->len)) {
				printf( "get: invalid part number\n" );
				return 1;
			}

			t = get_token( NULL );		/* signal name */
			if (!t || get_token( NULL )) {
				printf( "get: syntax error\n" );
				return 1;
			}

			data = part_get_signal( ps->parts[n], t );
			if (data != -1)
				printf( "%s = %d\n", t, data );

			return 1;
		}

		if (strcmp( t, "script" ) == 0) {
			t = get_token( NULL );          /* filename */
			if (!t) {
				printf( "script: missing filename\n" );
				return 1;
			}
			if (get_token( NULL )) {
				printf( "script: syntax error\n" );
				return 1;
			}

			jtag_parse_file( t );

			return 1;
		}

		printf( "%s: unknown command\n", t );

	return 1;
}

void
jtag_readline_loop( const char *prompt )
{
	char *line = NULL;

	/* Iterate */
	while (jtag_parse_line( line )) {
		free( line );

		/* Read a line from the terminal */
		line = readline( prompt );

		/* Check if we actually got something */
		if (line && (strlen( line ) > 0))
			add_history( line );
	}
	free( line );
}

int
jtag_parse_file( const char *filename )
{
	FILE *f;
	int go = 1;
	char *line = NULL;
	int n = 0;

	f = fopen( filename, "r" );
	if (!f) {
		printf( "Unable to open file `%s'!\n", filename);
		return 1;
	}

	while (getline( &line, &n, f ) != -1)
		if (strlen(line) > 0)
			go = jtag_parse_line(line);

	free(line);
	fclose(f);

	return go;
}

void
jtag_parse_rc( void )
{
	char *home = getenv( "HOME" );
	char *file;

	if (!home)
		return;

	file = malloc( strlen(home) + strlen(JTAGDIR) + strlen(RCFILE) + 3 );	/* 2 x "/" and trailing \0 */
	if (!file)
		return;

	strcpy( file, home );
	strcat( file, "/" );
	strcat( file, JTAGDIR );
	strcat( file, "/" );
	strcat( file, RCFILE );

	jtag_parse_file( file );

	free( file );
}

int
main( void )
{
	printf(
			"%s\n"
			"Copyright (C) 2002, 2003 ETC s.r.o.\n"
			"%s is free software, covered by the GNU General Public License, and you are\n"
			"welcome to change it and/or distribute copies of it under certain conditions.\n"
			"There is absolutely no warranty for %s.\n\n", PACKAGE_STRING, PACKAGE, PACKAGE
	);

	printf( "Warning: %s may damage your hardware! Type \"quit\" for exit!\n\n", PACKAGE );
	printf( "Type \"help\" for help.\n\n" );

	/* Create ~/.jtag */
	jtag_create_jtagdir();

	/* Parse and execute the RC file */
	jtag_parse_rc();

	/* Load history */
	jtag_load_history();

	/* main loop */
	jtag_readline_loop( "jtag> " );

	/* Save history */
	jtag_save_history();

	parts_free( ps );
	
	if (cable) {
		cable->set_trst( 0 );
		cable->set_trst( 1 );
		tap_reset();
		cable->done();

		cable = NULL;
	}

	return 0;
}
