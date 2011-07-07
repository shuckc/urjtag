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
 * shell.c added by djf
 */

#include <sysdep.h>

#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <urjtag/error.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_shell_run (urj_chain_t *chain, char *params[])
{
    int i, n = urj_cmd_params (params);
    size_t len;
    char *shell_cmd;

    if ((n = urj_cmd_params (params)) == 1)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    /* I must apologize to everyone who knows what they are doing for
     * the following. If you can pass a shell argument past strtok the
     * please fix this.
     */
    /* The problem is the parser which splits commands into params[]
     * and doesn't allow quoting. So we concatenate the params[] here
     * with single spaces, although the original might have different
     * whitespace (more than one space, tabs, ...) - kawk */

    for (i = 1, len = 0; i < n; i++)
        len += (1 + strlen (params[i]));

    shell_cmd = malloc (len);
    if (shell_cmd == NULL)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zu) fails",
                       len);
        return URJ_STATUS_FAIL;
    }

    strcpy (shell_cmd, params[1]);
    for (i = 2; i < n; i++)
    {
        strcat (shell_cmd, " ");
        strcat (shell_cmd, params[i]);
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, "Executing '%s'\n", shell_cmd);

    i = system (shell_cmd);
    free (shell_cmd);

    if (i)
        urj_log (URJ_LOG_LEVEL_NORMAL, "shell returned %i\n", i);

    return URJ_STATUS_OK;
}

static void
cmd_shell_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                    char * const *tokens, const char *text, size_t text_len,
                    size_t token_point)
{
    /* XXX: Should first token complete via $PATH ? */
    urj_completion_mayben_add_file (matches, match_cnt, text, text_len, false);
}

static void
cmd_shell_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s CMD\n"
               "Shell out to the OS for a command.\n"
               "\n" "CMD OS Shell Command\n"),
             "shell");
}

const urj_cmd_t urj_cmd_shell = {
    "shell",
    N_("run a shell command"),
    cmd_shell_help,
    cmd_shell_run,
    cmd_shell_complete,
};
