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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "part.h"
#include "ctrl.h"
#include "tap.h"

#include "detect.h"

static char *
get_token( char *buf )
{
	return strtok( buf, " \f\n\r\t\v" );
}

int
main( void )
{
	char *line = NULL;
	parts *ps = NULL;

	printf(
			"%s\n"
			"Copyright (C) 2002 ETC s.r.o.\n"
			"%s is free software, covered by the GNU General Public License, and you are\n"
			"welcome to change it and/or distribute copies of it under certain conditions.\n"
			"There is absolutely no warranty for %s.\n\n", PACKAGE_STRING, PACKAGE, PACKAGE
	);

	if (!tap_init()) {
		printf( "TAP initialization failed! Exiting.\n" );
		return 1;
	}

	tap_set_trst( 0 );
	tap_set_trst( 1 );

	for (;;) {
		char *t;

		free( line );
		line = readline( "jtag> " );

		if (!line || !*line)
			continue;
		add_history( line );

		t = get_token( line );
		if (!t)
			continue;

		if (strcmp( t, "quit" ) == 0)
			break;

		if (strcmp( t, "help" ) == 0) {
			printf( "help: Please read sources.\n" );
			continue;
		}

		if (strcmp( t, "detect" ) == 0) {
			t = get_token( NULL );
			if (ps)
				parts_free( ps );
			ps = detect_parts( "../data" );
			parts_set_instruction( ps, "SAMPLE/PRELOAD" );
			parts_shift_instructions( ps );
			parts_shift_data_registers( ps );
			parts_set_instruction( ps, "BYPASS" );
			parts_shift_instructions( ps );
			continue;
		}

		if (strcmp( t, "readmem" ) == 0) {
			readmem( ps );
			continue;
		}

		if (strcmp( t, "print" ) == 0) {
			parts_print( ps, 1 );
			continue;
		}

		if (strcmp( t, "instruction" ) == 0) {
			int n;

			t = get_token( NULL );
			if (!t) {
				printf( "instruction: syntax error\n" );
				continue;
			}

			n = strtol( t, &t, 10 );
			if (t && *t) {
				printf( "instruction: syntax error\n" );
				continue;
			}
			
			if ((n < 0) || (n >= ps->len)) {
				printf( "instruction: invalid part number\n" );
				continue;
			}

			t = get_token( NULL );
			if (!t) {
				printf( "Missing instruction name\n" );
				continue;
			}

			part_set_instruction( ps->parts[n], t );

			continue;
		}

		if (strcmp( t, "shift" ) == 0) {
			t = get_token( NULL );

			if (t && (strcmp( t, "ir" ) == 0)) {
				parts_shift_instructions( ps );
				continue;
			}

			if (t && (strcmp( t, "dr" ) == 0)) {
				parts_shift_data_registers( ps );
				continue;
			}

			printf( "shift: syntax error\n" );
			continue;
		}

		if (strcmp( t, "set" ) == 0) {
			t = get_token( NULL );
			if (!t) {
				printf( "set: syntax error\n" );
				continue;
			}
			
			continue;
		}

		printf( "%s: unknown command\n", t );
	}

	free( line );
	parts_free( ps );

	tap_reset();
	tap_done();

	return 0;
}
