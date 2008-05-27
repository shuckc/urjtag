/*
 * $Id$
 *
 * Copyright (C) 2004, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2004.
 *
 */


#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include <svf.h>
#include <cmd.h>

static int
cmd_svf_run( chain_t *chain, char *params[] )
{
	FILE *SVF_FILE;
	int   num_params, i, result = -1;
	int   stop = 0;
	int   print_progress = 0;

	num_params = cmd_params( params );
	if (num_params > 1) {
		for (i = 2; i < num_params; i++) {
			if (strcasecmp(params[i], "stop") == 0)
				stop = 1;
			else if (strcasecmp(params[i], "progress") == 0)
				print_progress = 1;
			else
				return -1;
		}

		if ((SVF_FILE = fopen(params[1], "r")) != NULL) {
			svf_run(chain, SVF_FILE, stop, print_progress);
			result = 1;

			fclose(SVF_FILE);
		} else {
			printf( _("%s: cannot open file '%s' for reading\n"), "svf", params[1] );
		}

	}

	return result;
}


static void
cmd_svf_help( void )
{
	printf( _(
		"Usage: %s FILE [stop] [progress]\n"
		"Execute svf commands from FILE.\n"
		"stop     : Command execution stops upon TDO mismatch.\n"
		"progress : Continually displays progress status.\n"
		"\n"
		"FILE file containing SVF commans\n"
	), "svf" );
}

cmd_t cmd_svf = {
	"svf",
	N_("execute svf commands from file"),
	cmd_svf_help,
	cmd_svf_run
};


/* Emacs specific variables
;;; Local Variables: ***
;;; indent-tabs-mode:t ***
;;; tab-width:2 ***
;;; End: ***
*/
