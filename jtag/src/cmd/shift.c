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

#include "chain.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_shift_run( char *params[] )
{
	if (cmd_params( params ) != 2)
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (strcmp( params[1], "ir" ) == 0) {
		chain_shift_instructions( chain );
		return 1;
	}
	if (strcmp( params[1], "dr" ) == 0) {
		chain_shift_data_registers( chain );
		return 1;
	}

	return -1;
}

static void
cmd_shift_help( void )
{
	printf( _(
		"Usage: %s\n"
		"Usage: %s\n"
		"Shift instruction or data register through JTAG chain.\n"
	), "shift ir", "shift dr" );
}

cmd_t cmd_shift = {
	"shift",
	N_("shift data/instruction registers through JTAG chain"),
	cmd_shift_help,
	cmd_shift_run
};
