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

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "part.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_print_run( char *params[] )
{
	char format[100];
	char header[100];
	int i;

	if (cmd_params( params ) > 2)
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	snprintf( format, 100, _(" No. %%-%ds %%-%ds %%-%ds %%-%ds %%-%ds\n"), MAXLEN_MANUFACTURER, MAXLEN_PART, MAXLEN_STEPPING,
			MAXLEN_INSTRUCTION, MAXLEN_DATA_REGISTER );
	snprintf( header, 100, format, _("Manufacturer"), _("Part"), _("Stepping"), _("Instruction"), _("Register") );
	printf( header );

	for (i = 0; i < strlen( header ); i++ )
		putchar( '-' );
	putchar( '\n' );

	if (cmd_params( params ) == 1) {
		if (chain->parts->len > chain->active_part) {
			printf( _(" %3d "), chain->active_part );
			part_print( chain->parts->parts[chain->active_part] );
		}
		return 1;
	}

	if (strcmp( params[1], "chain" ) != 0)
		return -1;

	parts_print( chain->parts );

	return 1;
}

static void
cmd_print_help( void )
{
	printf( _(
		"Usage: %s [chain]\n"
		"Display JTAG chain status.\n"
		"\n"
		"Display list of the parts connected to the JTAG chain including\n"
		"part number and current (active) instruction and data register.\n"
	), "print" );
}

cmd_t cmd_print = {
	"print",
	N_("display JTAG chain list/status"),
	cmd_print_help,
	cmd_print_run
};
