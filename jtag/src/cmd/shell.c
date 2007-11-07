/*
 * $Id: shell.c,v 1.6 2003/08/19 08:42:20 telka Exp $
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
 * shell.c added by djf
 */

#include "sysdep.h"

//#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "part.h"
//#include "bssignal.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_shell_run( char *params[] )
{
int n,l1,l2;
char *t;

	if((n=cmd_params( params )) == 1)
		return -1;

	/* I must apologize to everyone who knows what they are doing for
	* the following. If you can pass a shell argument past strtok the
	* please fix this.
	*/

	l1 = strlen(params[1]);
	l2 = strlen(params[2]);
	t = malloc(l1+l2+2); 	/* space + term */
	strcpy(t,params[1]);	/* main command */

	if(n == 3) {
		*(t+l1)= ' ';		/* add space */
		strcpy((t+l1+1),params[2]);
	}
	system(t);

	return 1;
}

static void
cmd_shell_help( void )
{
	printf( _(
		"Usage: %s cmmd\n"
		"Shell out to os for a command.\n"
		"\n"
		"CMMD OS Shell Command\n"
	), "shell cmmd" );
}

cmd_t cmd_shell = {
	"shell",
	N_("shell cmmd"),
	cmd_shell_help,
	cmd_shell_run
};
