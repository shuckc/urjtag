/*
 * $Id$
 *
 * Copyright (C) 2004 Marcel Telka
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
 * Written by Marcel Telka <marcel@telka.sk>, 2004.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include "jtag.h"

#include "cmd.h"

void jtag_reset( chain_t *chain );

static int
cmd_reset_run( char *params[] )
{
	if (cmd_params( params ) > 1)
		return -1;

	if (!cmd_test_cable())
		return 1;

	jtag_reset( chain );

	return 1;
}

static void
cmd_reset_help( void )
{
	printf( _(
		"Usage: %s\n"
		"Reset current JTAG chain.\n"
	), "reset" );
}

cmd_t cmd_reset = {
	"reset",
	N_("reset JTAG chain"),
	cmd_reset_help,
	cmd_reset_run
};
