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

#include <sysdep.h>

#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_help_run (urj_chain_t *chain, char *params[])
{
    int i;

    if (urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    /* short description generation */
    if (urj_cmd_params (params) == 1)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Command list:\n\n"));
        urj_cmd_show_list (urj_cmds);
        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("\nType \"help COMMAND\" for details about a particular command.\n"));
        return URJ_STATUS_OK;
    }

    /* search and print help for a particular command */
    for (i = 0; urj_cmds[i]; i++)
        if (strcasecmp (urj_cmds[i]->name, params[1]) == 0)
        {
            if (urj_cmds[i]->help)
                urj_cmds[i]->help ();
            return URJ_STATUS_OK;
        }

    urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: unknown command\n"), params[1]);

    return URJ_STATUS_OK;
}

static void
cmd_help_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s [COMMAND]\n"
               "Print short help for COMMAND, or list of available commands.\n"),
             "help");
}

static void
cmd_help_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                   char * const *tokens, const char *text, size_t text_len,
                   size_t token_point)
{
    size_t i;

    /* Completing the command itself will come here as token 0 */
    if (token_point > 1)
        return;

    for (i = 0; urj_cmds[i]; ++i)
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                         urj_cmds[i]->name);
}

const urj_cmd_t urj_cmd_help = {
    "help",
    N_("display this help"),
    cmd_help_help,
    cmd_help_run,
    cmd_help_complete,
};
