/*
 * $Id$
 *
 * Copyright (C) 2007, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2007.
 *
 */


#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include <bsdl.h>
#include <cmd.h>

static int
cmd_bsdl_run( char *params[] )
{
	int num_params, result = -1;

	num_params = cmd_params(params);
	if (num_params >= 2) {
		if (strcmp(params[1], "test") == 0) {
			int debug_save;

			debug_save = bsdl_debug;
			bsdl_debug = 1;
			if (num_params == 3) {
				result = bsdl_read_file(params[2], -1, NULL) >= 0 ? 1 : -1;
			} else if (num_params == 2) {
				bsdl_scan_files(NULL, -1);
				result = 1;
			}
			bsdl_debug = debug_save;
		}

		if (strcmp(params[1], "dump") == 0) {
			if (num_params == 3) {
				result = bsdl_read_file(params[2], 0, NULL) >= 0 ? 1 : -1;
			} else if (num_params == 2) {
				bsdl_scan_files(NULL, 0);
				result = 1;
			}
		}

		if (num_params == 3) {
			if (strcmp(params[1], "path") == 0) {
				bsdl_set_path(params[2]);
				result = 1;
			}

			if (strcmp(params[1], "debug") == 0) {
				if (strcmp(params[2], "on") == 0) {
					bsdl_debug = 1;
					result = 1;
				}
				if (strcmp(params[2], "off") == 0) {
					bsdl_debug = 0;
					result = 1;
				}
			}
		}
	}

	return result;
}


static void
cmd_bsdl_help( void )
{
	printf( _(
		"Usage: %s path PATHLIST\n"
		"Usage: %s test [FILE]\n"
		"Usage: %s dump [FILE]\n"
		"Usage: %s debug on|off\n"
		"Manage BSDL files\n"
		"\n"
		"PATHLIST semicolon separated list of directory paths to search for BSDL files\n"
		"FILE file containing part description in BSDL format\n"
	), "bsdl", "bsdl", "bsdl", "bsdl" );
}

cmd_t cmd_bsdl = {
	"bsdl",
	N_("manage BSDL files"),
	cmd_bsdl_help,
	cmd_bsdl_run
};


/* Emacs specific variables
;;; Local Variables: ***
;;; indent-tabs-mode:t ***
;;; tab-width:2 ***
;;; End: ***
*/
