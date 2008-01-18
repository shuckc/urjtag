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

#include <cmd.h>

static int
cmd_quit_run( char *params[] )
{
	if (params[1])
		return -1;

	return 0;
}

static void
cmd_quit_help( void )
{
	printf( _(
		"Usage: %s\n"
		"Exit from %s.\n"
	), "quit", PACKAGE );
}

cmd_t cmd_quit = {
	"quit",
	N_("exit and terminate this session"),
	cmd_quit_help,
	cmd_quit_run
};
