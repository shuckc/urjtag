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
#include <stdlib.h>
#include <string.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_signal_run( char *params[] )
{
	part_t *part;
	signal_t *s;

	if (cmd_params( params ) < 2)
		return -1;


	if (!cmd_test_cable())
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	if (chain->active_part >= chain->parts->len) {
		printf( _("%s: no active part\n"), "signal" );
		return 1;
	}

	part = chain->parts->parts[chain->active_part];
	if (part_find_signal( part, params[1] ) != NULL) {
		printf( _("Signal '%s' already defined\n"), params[1] );
		return 1;
	}

	s = signal_alloc( params[1] );
	if (!s) {
		printf( _("out of memory\n") );
		return 1;
	}

	s->next = part->signals;
	part->signals = s;

	return 1;
}

static void
cmd_signal_help( void )
{
	printf( _(
		"Usage: %s SIGNAL [PINLIST...]\n"
		"Define new signal with name SIGNAL for a part.\n"
		"\n"
		"SIGNAL        New signal name\n"
		"PINLIST       List of pins for a signal (not used)\n"
	), "signal" );
}

cmd_t cmd_signal = {
	"signal",
	N_("define new signal for a part"),
	cmd_signal_help,
	cmd_signal_run
};
