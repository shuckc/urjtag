/*
 * $Id$
 *
 * Copyright (C) 2011, Michael Vacek
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
 * Written by Michael Vacek <michael.vacek@gmail.com>, 2011.
 *
 */

#include <urjtag/error.h>
#include <urjtag/parse.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>
#include <urjtag/stapl.h>

#include "cmd.h"

static int
cmd_stapl_run (urj_chain_t *chain, char *params[])
{
    int num_params;
    int result = URJ_STATUS_OK;

    num_params = urj_cmd_params (params);
    if (num_params <= 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be = %d, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    result = urj_stapl_run (chain, params[1], params[2]);

    return result;
}

static void
cmd_stapl_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s FILE -aACTION\n"
               "Execute stapl commands from FILE.\n"
               "ACTION     : Name of stapl action"
               "to be executed from the FILE.\n"
               "\n" "FILE file containing stapl code\n"),
             "stapl");
}

static void
cmd_stapl_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                    char * const *tokens, const char *text, size_t text_len,
                    size_t token_point)
{
    urj_completion_mayben_add_file (matches, match_cnt, text, text_len, true);
}

const urj_cmd_t urj_cmd_stapl = {
    "stapl",
    N_("execute stapl commands from file"),
    cmd_stapl_help,
    cmd_stapl_run,
    cmd_stapl_complete,
};
