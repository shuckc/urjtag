/*
 * $Id$
 *
 * Copyright (C) 2003 Marcel Telka
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
cmd_salias_run( chain_t *chain, char *params[] )
{
	part_t *part;
	signal_t *s;
	salias_t *sa;

	if (cmd_params( params ) != 3)
		return -1;

	if (!cmd_test_cable( chain ))
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

	s = part_find_signal( part, params[2] );
	if (s == NULL) {
		printf( _("Signal '%s' not found\n"), params[2] );
		return 1;
	}

	sa = salias_alloc( params[1], s );
	if (!sa) {
		printf( _("out of memory\n") );
		return 1;
	}

	sa->next = part->saliases;
	part->saliases = sa;

	return 1;
}

static void
cmd_salias_help( void )
{
	printf( _(
		"Usage: %s ALIAS SIGNAL\n"
		"Define new signal ALIAS as alias for existing SIGNAL.\n"
		"\n"
		"ALIAS         New signal alias name\n"
		"SIGNAL        Existing signal name\n"
	), "signal" );
}

const cmd_t cmd_salias = {
	"salias",
	N_("define an alias for a signal"),
	cmd_salias_help,
	cmd_salias_run
};
