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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gettext.h"
#define	_(s)		gettext(s)
#define	N_(s)		gettext_noop(s)
#define	P_(s,p,n)	ngettext(s,p,n)

#include <stdio.h>
#include <string.h>

#include "part.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_get_run( char *params[] )
{
	unsigned int n;
	int data;

	if (cmd_params( params ) != 4)
		return -1;

	if (strcmp( params[1], "signal") != 0)
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
		printf( _("%s: invalid part number\n"), "get" );
		return 1;
	}

	data = part_get_signal( chain->parts->parts[n], params[3] );
	if (data != -1)
		printf( _("%s = %d\n"), params[3], data );

	return 1;
}

static void
cmd_get_help( void )
{
	printf( _(
		"Usage: %s PART SIGNAL\n"
		"Get signal state from output BSR (Boundary Scan Register).\n"
		"\n"
		"PART          part number (see print command)\n"
		"SIGNAL        signal name (from JTAG declaration file)\n"
	), "get signal" );
}

cmd_t cmd_get = {
	"get",
	N_("get external signal value"),
	cmd_get_help,
	cmd_get_run
};
