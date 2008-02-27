/*
 * $Id: debug.c,v 1.0 2005/10/10 00:00:0 DJF $
 *
 * Copyright (C) 2005 Protoparts
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
 * Written by David Farrell, 2005
 * based on templates by and portions  Written by Marcel Telka <marcel@telka.sk>, 2003.i
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
//#include "part.h"
//#include "bssignal.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_debug_run( chain_t *chain, char *params[] )
{
	unsigned int i;

	if (cmd_params( params ) != 2)
		return -1;

	if (cmd_get_number( params[1], &i ))
                return 1;

	debug_mode = i;
	return 1;
}

static void
cmd_debug_help( void )
{
	printf( _(
		"Usage: %s  n\n"
		"Enabled debugging.\n"
		"\n"
		"n =1 fileio, 2=tap commands, 4 =?\n"
	), "debug n" );
}

cmd_t cmd_debug = {
	"debug",
	N_("debug jtag program"),
	cmd_debug_help,
	cmd_debug_run
};

