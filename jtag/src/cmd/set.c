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
#include "bssignal.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_set_run( char *params[] )
{
	unsigned int n;
	int dir;
	unsigned int data = 0;
	signal_t *s;

	if (cmd_params( params ) < 5 || cmd_params( params ) > 6)
		return -1;

	if (strcmp( params[1], "signal" ) != 0)
		return -1;

	if (cmd_get_number( params[2], &n ))
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	if (n >= chain->parts->len) {
		printf( _("%s: invalid part number\n"), "set" );
		return 1;
	}

	/* direction */
	if (strcmp( params[4], "in" ) != 0 && strcmp( params[4], "out" ) != 0)
		return -1;

	dir = (strcmp( params[4], "in" ) == 0) ? 0 : 1;

	if (dir) {
		if (cmd_get_number( params[5], &data ))
			return -1;
		if (data > 1)
			return -1;
	}

	s = part_find_signal( chain->parts->parts[n], params[3] );
	if (!s) {
		printf( _("signal '%s' not found\n"), params[3] );
		return 1;
	}
	part_set_signal( chain->parts->parts[n], s, dir, data );

	return 1;
}

static void
cmd_set_help( void )
{
	printf( _(
		"Usage: %s PART SIGNAL DIR [DATA]\n"
		"Set signal state in input BSR (Boundary Scan Register).\n"
		"\n"
		"PART          part number (see print command)\n"
		"SIGNAL        signal name (from JTAG declaration file)\n"
		"DIR           requested signal direction; possible values: 'in' or 'out'\n"
		"DATA          desired output signal value ('0' or '1'); used only if DIR\n"
		"                is 'out'\n"
	), "set signal" );
}

cmd_t cmd_set = {
	"set",
	N_("set external signal value"),
	cmd_set_help,
	cmd_set_run
};
