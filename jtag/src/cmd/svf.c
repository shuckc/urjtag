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
cmd_svf_run( char *params[] )
{
	FILE *SVF_FILE;
	int   num_params, result = -1;

	num_params = cmd_params( params );
	if (num_params == 2 || num_params == 3) {
		if ((SVF_FILE = fopen(params[1], "r")) != NULL) {

			if (num_params == 3) {
				if (strcmp(params[2], "stop") == 0) {
					svf_run(SVF_FILE, 1);
					result = 1;
				}
			} else {
				svf_run(SVF_FILE, 0);
				result = 1;
			}

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
		"Usage: %s FILE\n"
		"Usage: %s FILE stop\n"
		"Execute svf commands from FILE.\n"
		"Command execution stops upon TDO mismatch when 'stop' is specified.\n"
		"\n"
		"FILE file containing SVF commans\n"
	), "svf", "svf" );
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
