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

#include "part.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_instruction_run( char *params[] )
{
	if (cmd_params( params ) != 2)
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	if (chain->active_part >= chain->parts->len) {
		printf( _("%s: no active part\n"), "instruction" );
		return 1;
	}

	part_set_instruction( chain->parts->parts[chain->active_part], params[1] );
	if (chain->parts->parts[chain->active_part]->active_instruction == NULL)
		printf( _("%s: unknown instruction '%s'\n"), "instruction", params[1] );

	return 1;
}

static void
cmd_instruction_help( void )
{
	printf( _(
		"Usage: %s INSTRUCTION\n"
		"Change active INSTRUCTION for a PART.\n"
		"\n"
		"INSTRUCTION   instruction name (e.g. BYPASS)\n"
	), "instruction" );
}

cmd_t cmd_instruction = {
	"instruction",
	N_("change active instruction for a part"),
	cmd_instruction_help,
	cmd_instruction_run
};
