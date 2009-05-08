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

#include <urjtag/sysdep.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/parse.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>
#include <urjtag/bsdl.h>

#include "cmd.h"

static int
cmd_include_or_script_run (urj_chain_t *chain, int is_include, char *params[])
{
    int i;
    unsigned int j = 1;

    if (urj_cmd_params (params) < 2)
        return -1;

    if (!is_include)
    {
        printf (_("Please use the 'include' command instead of 'script'\n"));
    }

    if (urj_cmd_params (params) > 2)
    {
        /* loop n times option */
        if (urj_cmd_get_number (params[2], &j))
        {
            printf (_("%s: unable to get number from '%s'\n"),
                    "include/script", params[2]);
            return -1;
        }
    }

    for (i = 0; i < j; i++)
    {
        if (urj_parse_include (chain, params[1], ! is_include) != URJ_STATUS_OK)
        {
            printf ("error: %s\n", urj_error_describe ());
            urj_error_reset ();
            break;
        }
    }

    return 1;
}

static void
cmd_include_or_script_help (char *cmd)
{
    printf (_("Usage: %s FILENAME [n] \n"
              "Run command sequence n times from external FILENAME.\n"
              "\n" "FILENAME      Name of the file with commands\n"), cmd);
}

static int
cmd_include_run (urj_chain_t *chain, char *params[])
{
    return cmd_include_or_script_run (chain, 1, params);
}

static void
cmd_include_help (void)
{
    cmd_include_or_script_help ("include");
}

const urj_cmd_t urj_cmd_include = {
    "include",
    N_("include command sequence from external repository"),
    cmd_include_help,
    cmd_include_run
};

static int
cmd_script_run (urj_chain_t *chain, char *params[])
{
    return cmd_include_or_script_run (chain, 0, params);
}

static void
cmd_script_help (void)
{
    cmd_include_or_script_help ("script");
}

const urj_cmd_t urj_cmd_script = {
    "script",
    N_("run command sequence from external file"),
    cmd_script_help,
    cmd_script_run
};
