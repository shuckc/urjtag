/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_include_run( char *params[] )
{
	int go;
	char *path;
	int len;

	if (cmd_params( params ) != 2)
		return -1;

	path = malloc( len = strlen( JTAG_DATA_DIR ) + strlen( params[1] ) + 2 );
	if (path == NULL) {
		printf( _("Out of memory\n") );
		return 1;
	}
	snprintf( path, len, "%s/%s", JTAG_DATA_DIR, params[1] );

	go = jtag_parse_file( path );
	if (go < 0)
		printf( _("Unable to open file `%s'!\n"), params[1] );

	free( path );

	return go ? 1 : 0;
}

static void
cmd_include_help( void )
{
	printf( _(
		"Usage: %s FILENAME\n"
		"Run command sequence from external FILENAME from the repository.\n"
		"\n"
		"FILENAME      Name of the file with commands\n"
	), "include" );
}

cmd_t cmd_include = {
	"include",
	N_("include command sequence from external repository"),
	cmd_include_help,
	cmd_include_run
};
