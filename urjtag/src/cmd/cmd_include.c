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

#include <urjtag/parse.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>
#include <urjtag/bsdl.h>

static int
cmd_include_or_script_run (urj_chain_t *chain, int is_include, char *params[])
{
    int go = 0, i, j = 1;
    char *path;
    int len;

    if (urj_cmd_params (params) < 2)
        return -1;

    if (!is_include)
    {
        printf (_("Please use the 'include' command instead of 'script'\n"));
    }

    /* If "params[1]" begins with a slash, or dots followed by a slash,
     * assume that user wants to ignore the search path */

    path = params[1];
#ifdef __MINGW32__
    if (isalpha (*path) && path[1] == ':')
        path += 2;
#endif
    while (*path == '.')
        path++;
    if (*path == '/' || *path == '\\' || !is_include)
    {
        path = strdup (params[1]);
    }
    else
    {
        const char *jtag_data_dir = urj_get_data_dir ();
        path = malloc (len = strlen (jtag_data_dir) + strlen (params[1]) + 2);
        if (path != NULL)
        {
            snprintf (path, len, "%s/%s", jtag_data_dir, params[1]);
        }
    }
    if (path == NULL)
    {
        printf (_("Out of memory\n"));
        return 1;
    }

#ifdef ENABLE_BSDL
    /* perform a test read to check for BSDL syntax */
    if (urj_bsdl_read_file (chain, path, URJ_BSDL_MODE_INCLUDE1, NULL) >= 0)
    {
        /* it seems to be a proper BSDL file, so re-read and execute */
        go = urj_bsdl_read_file (chain, path, URJ_BSDL_MODE_INCLUDE2, NULL);

        free (path);
        return 1;
    }
#endif

    if (urj_cmd_params (params) > 2)
    {
        sscanf (params[2], "%d", &j);   /* loop n times option */
    }

    for (i = 0; i < j; i++)
    {
        go = urj_parse_file (chain, path);

        if (go < 0)
        {
            if (go != -99)
                printf (_("Unable to open file `%s go=%d'!\n"), path, go);
            break;
        }
    }

    free (path);

    return go ? 1 : 0;
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

urj_cmd_t urj_cmd_include = {
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

urj_cmd_t urj_cmd_script = {
    "script",
    N_("run command sequence from external file"),
    cmd_script_help,
    cmd_script_run
};
