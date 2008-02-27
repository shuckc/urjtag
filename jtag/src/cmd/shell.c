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

#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "part.h"
//#include "bssignal.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_shell_run( chain_t *chain, char *params[] )
{
	int i, len, n = cmd_params(params);
	char *shell_cmd;

	if((n=cmd_params( params )) == 1)
		return -1;

	/* I must apologize to everyone who knows what they are doing for
	* the following. If you can pass a shell argument past strtok the
	* please fix this.
	*/
	/* The problem is the parser which splits commands into params[]
	* and doesn't allow quoting. So we concatenate the params[] here
	* with single spaces, although the original might have different
	* whitespace (more than one space, tabs, ...) - kawk */

	for(i=1,len=0; i<n; i++) len += (1 + strlen(params[i]));

	shell_cmd = malloc(len);
	if(shell_cmd == NULL)
	{
		printf( _("Out of memory\n") );
		return -1;
	};

	strcpy(shell_cmd, params[1]);
	for(i=2; i<n; i++)
	{
		strcat(shell_cmd, " ");
		strcat(shell_cmd, params[i]);
	};
	printf("Executing '%s'\n", shell_cmd);

	system(shell_cmd);
	free(shell_cmd);

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
