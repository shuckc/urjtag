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

#include "register.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_dr_run( char *params[] )
{
	unsigned int n;
	int dir = 1;
	tap_register *r;

	if (cmd_params( params ) < 2 || cmd_params( params ) > 3)
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	if (cmd_get_number( params[1], &n ))
		return -1;

	if (n >= chain->parts->len) {
		printf( _("%s: invalid part number\n"), "dr" );
		return 1;
	}

	if (params[2]) {
		if (strcmp( params[2], "in" ) == 0)
			dir = 0;
		else if (strcmp( params[2], "out" ) == 0)
			dir = 1;
		else
			return -1;
	}

	if (dir)
		r = chain->parts->parts[n]->active_instruction->data_register->out;
	else
		r = chain->parts->parts[n]->active_instruction->data_register->in;
	printf( _("%s\n"), register_get_string( r ) );

	return 1;
}

static void
cmd_dr_help( void )
{
	printf( _(
		"Usage: %s PART [DIR]\n"
		"Display input or output data register content.\n"
		"\n"
		"PART          part number (see print command)\n"
		"DIR           requested data register; possible values: 'in' for\n"
		"                input and 'out' for output; default is 'out'\n"
	), "dr" );
}

cmd_t cmd_dr = {
	"dr",
	N_("display active data register for a part"),
	cmd_dr_help,
	cmd_dr_run
};
