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


#include <sysdep.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <urjtag/error.h>
#include <urjtag/log.h>

#include <urjtag/svf.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_svf_run (urj_chain_t *chain, char *params[])
{
    FILE *SVF_FILE;
    int num_params, i;
    int stop = 0;
    int print_progress = 0;
    uint32_t ref_freq = 0;
    urj_log_level_t old_log_level = urj_log_state.level;
    int result = URJ_STATUS_OK;

    num_params = urj_cmd_params (params);
    if (num_params < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    for (i = 2; i < num_params; i++)
    {
        if (strcasecmp (params[i], "stop") == 0)
            stop = 1;
        else if (strcasecmp (params[i], "progress") == 0)
            print_progress = 1;
        else if (strncasecmp (params[i], "ref_freq=", 9) == 0)
            ref_freq = strtol (params[i] + 9, NULL, 10);
        else
        {
            urj_error_set (URJ_ERROR_SYNTAX, "%s: unknown command '%s'",
                           params[0], params[i]);
            return URJ_STATUS_FAIL;
        }
    }

    if (print_progress)
        urj_log_state.level = URJ_LOG_LEVEL_DETAIL;

    if ((SVF_FILE = fopen (params[1], FOPEN_R)) != NULL)
    {
        result = urj_svf_run (chain, SVF_FILE, stop, ref_freq);

        fclose (SVF_FILE);
    }
    else
    {
        urj_error_IO_set ("%s: cannot open file '%s'", params[0], params[1]);
        result = URJ_STATUS_FAIL;
    }

    urj_log_state.level = old_log_level;

    return result;
}

static void
cmd_svf_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                  char * const *tokens, const char *text, size_t text_len,
                  size_t token_point)
{
    static const char * const main_cmds[] = {
        "stop",
        "progress",
        "ref_freq=",
    };

    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_file (matches, match_cnt, text,
                                        text_len, false);
        break;

    default:
        urj_completion_mayben_add_matches (matches, match_cnt, text, text_len,
                                           main_cmds);
        break;
    }
}

static void
cmd_svf_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s FILE [stop] [progress] [ref_freq=<frequency>]\n"
               "Execute svf commands from FILE.\n"
               "stop     : Command execution stops upon TDO mismatch.\n"
               "progress : Continually displays progress status.\n"
               "ref_freq : Use <frequency> as the reference for 'RUNTEST xxx SEC' commands\n"
               "\n" "FILE file containing SVF commands\n"),
             "svf");
}

const urj_cmd_t urj_cmd_svf = {
    "svf",
    N_("execute svf commands from file"),
    cmd_svf_help,
    cmd_svf_run,
    cmd_svf_complete,
};
