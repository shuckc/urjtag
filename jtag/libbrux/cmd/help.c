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

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include <brux/cmd.h>

static int
cmd_help_run( char *params[] )
{
	int i;

	/* short description generation */
	if (!params[1]) {
		printf( _("Command list:\n\n") );
		for (i = 0; cmds[i]; i++)
			printf( _("%-13s %s\n"), cmds[i]->name, cmds[i]->desc ? _(cmds[i]->desc) : _("(no description available)") );
		printf( _("\nType \"help COMMAND\" for details about a particular command.\n") );
		return 1;
	}

	if (params[2])
		return -1;

	/* search and print help for a particular command */
	for (i = 0; cmds[i]; i++)
		if (strcasecmp( cmds[i]->name, params[1] ) == 0) {
			if (cmds[i]->help)
				cmds[i]->help();
			return 1;
		}

	printf( _("%s: unknown command\n"), params[1] );

	return 1;
}

static void
cmd_help_help( void )
{
	printf( _(
		"Usage: %s [COMMAND]\n"
		"Print short help for COMMAND, or list of available commands.\n"
	), "help" );
}

cmd_t cmd_help = {
	"help",
	N_("display this help"),
	cmd_help_help,
	cmd_help_run
};
