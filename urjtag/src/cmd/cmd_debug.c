/*
 * $Id$
 *
 * Copyright (C) 2005 Protoparts
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
 * Written by David Farrell, 2005
 * based on templates by and portions  Written by Marcel Telka <marcel@telka.sk>, 2003.i
 *
 */

#include <sysdep.h>

#include <string.h>

#include <urjtag/error.h>
#include <urjtag/log.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static urj_log_level_t
string_to_log_level (const char *strlevel)
{
    if (0)
        ;
    else if (strcasecmp(strlevel, "all") == 0)
        return URJ_LOG_LEVEL_ALL;
    else if (strcasecmp(strlevel, "comm") == 0)
        return URJ_LOG_LEVEL_COMM;
    else if (strcasecmp(strlevel, "debug") == 0)
        return URJ_LOG_LEVEL_DEBUG;
    else if (strcasecmp(strlevel, "detail") == 0)
        return URJ_LOG_LEVEL_DETAIL;
    else if (strcasecmp(strlevel, "normal") == 0)
        return URJ_LOG_LEVEL_NORMAL;
    else if (strcasecmp(strlevel, "warning") == 0)
        return URJ_LOG_LEVEL_WARNING;
    else if (strcasecmp(strlevel, "error") == 0)
        return URJ_LOG_LEVEL_ERROR;
    else if (strcasecmp(strlevel, "silent") == 0)
        return URJ_LOG_LEVEL_SILENT;
    else
        return -1;
}

static const char *
log_level_to_string (urj_log_level_t level)
{
    /* sanity for string_to_log_level() return */
    if (level == -1)
        goto case_default;
    switch (level) {
    case URJ_LOG_LEVEL_ALL:     return "all";
    case URJ_LOG_LEVEL_COMM:    return "comm";
    case URJ_LOG_LEVEL_DEBUG:   return "debug";
    case URJ_LOG_LEVEL_DETAIL:  return "detail";
    case URJ_LOG_LEVEL_NORMAL:  return "normal";
    case URJ_LOG_LEVEL_WARNING: return "warning";
    case URJ_LOG_LEVEL_ERROR:   return "error";
    case URJ_LOG_LEVEL_SILENT:  return "silent";
    case_default:
    default: return "unknown";
    }
}

static int
cmd_debug_run (urj_chain_t *chain, char *params[])
{
    switch (urj_cmd_params (params)) {

    /* display current log level */
    case 1:
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Current log level is '%s'\n"),
                 log_level_to_string (urj_log_state.level));
        return URJ_STATUS_OK;

    /* set log level */
    case 2:
    {
        urj_log_level_t new_level = string_to_log_level (params[1]);
        if (new_level == -1)
        {
            urj_error_set (URJ_ERROR_SYNTAX, "unknown log level '%s'", params[1]);
            return URJ_STATUS_FAIL;
        }
        urj_log_state.level = new_level;

        return URJ_STATUS_OK;
    }

    /* fail! */
    default:
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }
}

static void
cmd_debug_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s LEVEL\n"
               "Set logging/debugging level.\n"
               "\n" "LEVEL:\n"
               "all       every single bit as it is transmitted\n"
               "comm      low level communication details\n"
               "debug     more details of interest for developers\n"
               "detail    verbose output\n"
               "normal    just noteworthy info\n"
               "warning   unmissable warnings\n"
               "error     only fatal errors\n"
               "silent    suppress logging output\n"),
             "debug");
}

static void
cmd_debug_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                    char * const *tokens, const char *text, size_t text_len,
                    size_t token_point)
{
    if (token_point != 1)
        return;

    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "all");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "comm");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "debug");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "detail");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "normal");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "warning");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "error");
    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "silent");
}

const urj_cmd_t urj_cmd_debug = {
    "debug",
    N_("set logging/debugging level"),
    cmd_debug_help,
    cmd_debug_run,
    cmd_debug_complete,
};
