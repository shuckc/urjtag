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


#include <sysdep.h>

#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/bsdl.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_bsdl_run (urj_chain_t *chain, char *params[])
{
    int num_params, result = -2;
    urj_bsdl_globs_t *globs = &(chain->bsdl);

    num_params = urj_cmd_params (params);
    if (num_params < 2 || num_params > 3)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d or %d, not %d",
                       params[0], 2, 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (strcmp (params[1], "test") == 0)
    {
        int debug_save;

        debug_save = globs->debug;
        globs->debug = 1;
        if (num_params == 3)
        {
            result = urj_bsdl_read_file (chain, params[2], URJ_BSDL_MODE_TEST,
                                         NULL) >= 0 ? 1 : -1;
        }
        else if (num_params == 2)
        {
            urj_bsdl_scan_files (chain, NULL, URJ_BSDL_MODE_TEST);
            result = 1;
        }
        globs->debug = debug_save;
    }

    if (strcmp (params[1], "dump") == 0)
    {
        if (num_params == 3)
        {
            result = urj_bsdl_read_file (chain, params[2], URJ_BSDL_MODE_DUMP,
                                         NULL) >= 0 ? 1 : -1;
        }
        else if (num_params == 2)
        {
            urj_bsdl_scan_files (chain, NULL, URJ_BSDL_MODE_DUMP);
            result = 1;
        }
    }

    if (num_params == 3)
    {
        if (strcmp (params[1], "path") == 0)
        {
            urj_bsdl_set_path (chain, params[2]);
            result = 1;
        }

        if (strcmp (params[1], "debug") == 0)
        {
            if (strcmp (params[2], "on") == 0)
            {
                globs->debug = 1;
                result = 1;
            }
            if (strcmp (params[2], "off") == 0)
            {
                globs->debug = 0;
                result = 1;
            }
        }
    }

    if (result == -2)
    {
        urj_error_set (URJ_ERROR_SYNTAX, "unknown/malformed bsdl command '%s'",
                       params[1]);
        return URJ_STATUS_FAIL;
    }

    return (result >= 0) ? URJ_STATUS_OK : URJ_STATUS_FAIL;
}

static void
cmd_bsdl_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                   char * const *tokens, const char *text, size_t text_len,
                   size_t token_point)
{
    static const char * const main_cmds[] = {
        "path",
        "test",
        "dump",
        "debug",
    };

    static const char * const debug_cmds[] = {
        "on", "off",
    };

    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_matches (matches, match_cnt, text, text_len,
                                           main_cmds);
        break;

    case 2:
        /* XXX: For "test" and "dump", we'll want to search the bsdl paths */
        if (!strcmp (tokens[1], "path"))
            urj_completion_mayben_add_file (matches, match_cnt, text,
                                            text_len, false);
        else if (!strcmp (tokens[1], "debug"))
            urj_completion_mayben_add_matches (matches, match_cnt, text,
                                               text_len, debug_cmds);
        break;
    }
}

static void
cmd_bsdl_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s path PATHLIST\n"
               "Usage: %s test [FILE]\n"
               "Usage: %s dump [FILE]\n"
               "Usage: %s debug on|off\n"
               "Manage BSDL files\n"
               "\n"
               "PATHLIST semicolon separated list of directory paths to search for BSDL files\n"
               "FILE file containing part description in BSDL format\n"),
            "bsdl", "bsdl", "bsdl", "bsdl");
}

const urj_cmd_t urj_cmd_bsdl = {
    "bsdl",
    N_("manage BSDL files"),
    cmd_bsdl_help,
    cmd_bsdl_run,
    cmd_bsdl_complete,
};


/* Emacs specific variables
;;; Local Variables: ***
;;; indent-tabs-mode:t ***
;;; tab-width:2 ***
;;; End: ***
*/
