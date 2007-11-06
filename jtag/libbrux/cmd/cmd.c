/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include <brux/cmd.h>

int
cmd_run( char *params[] )
{
	int i;

	if (!params[0])
		return 1;

	for (i = 0; cmds[i]; i++)
		if (strcmp( cmds[i]->name, params[0] ) == 0) {
			int r = cmds[i]->run( params );
			if (r < 0)
				printf( _("%s: syntax error!\n"), params[0] );
			return r;
		}

	printf( _("%s: unknown command\n"), params[0] );
	return 1;
}

int
cmd_params( char *params[] )
{
	int i = 0;

	while (params[i])
		i++;

	return i;
}

int
cmd_get_number( char *s, unsigned int *i )
{
	int n;
	int r;
	size_t l;

	if (!s || !i)
		return -1;

	l = strlen( s );

	n = -1;
	r = sscanf( s, "0x%x%n", i, &n);
	if (r == 1 && n == l)
		return 0;

	n = -1;
	r = sscanf( s, "%u%n", i, &n );
	if (r == 1 && n == l)
		return 0;

	return -1;
}
