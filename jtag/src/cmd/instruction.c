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
cmd_instruction_run( char *params[] )
{
	part_t *part;

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

	part = chain->parts->parts[chain->active_part];

	if (cmd_params( params ) == 2) {
		part_set_instruction( part, params[1] );
		if (part->active_instruction == NULL)
			printf( _("%s: unknown instruction '%s'\n"), "instruction", params[1] );
		return 1;
	}
	
	if (cmd_params( params ) == 3) {
		unsigned int len;

		if (strcmp( params[1], "length" ) != 0)
			return -1;

		if (part->instructions != NULL) {
			printf( _("instruction length is already set and used\n") );
			return 1;
		}

		if (cmd_get_number( params[2], &len ))
			return -1;

		part->instruction_length = len;
		return 1;
	}

	if (cmd_params( params ) == 4) {
		instruction *i;

		if (strlen( params[2] ) != part->instruction_length) {
			printf( _("invalid instruction length\n") );
			return 1;
		}

		if (part_find_instruction( part, params[1] ) != NULL) {
			printf( _("Instruction '%s' already defined\n"), params[1] );
			return 1;
		}

		i = instruction_alloc( params[1], part->instruction_length, params[2] );
		if (!i) {
			printf( _("out of memory\n") );
			return 1;
		}

		i->next = part->instructions;
		part->instructions = i;

		i->data_register = part_find_data_register( part, params[3] );
		if (i->data_register == NULL) {
			printf( _("unknown data register '%s'\n"), params[3] );
			return 1;
		}

		return 1;
	}

	return -1;
}

static void
cmd_instruction_help( void )
{
	printf( _(
		"Usage: %s INSTRUCTION\n"
		"Usage: %s length LENGTH\n"
		"Usage: %s INSTRUCTION CODE REGISTER\n"
		"Change active INSTRUCTION for a part or declare new instruction.\n"
		"\n"
		"INSTRUCTION   instruction name (e.g. BYPASS)\n"
		"LENGTH        common instruction length\n"
		"CODE          instruction code (e.g. 11111)\n"
		"REGISTER      default data register for instruction (e.g. BR)\n"
	), "instruction", "instruction", "instruction" );
}

cmd_t cmd_instruction = {
	"instruction",
	N_("change active instruction for a part or declare new instruction"),
	cmd_instruction_help,
	cmd_instruction_run
};
