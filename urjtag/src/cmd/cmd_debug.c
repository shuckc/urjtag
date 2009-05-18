/*
 * $Id: debug.c,v 1.0 2005/10/10 00:00:0 DJF $
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

static int
cmd_debug_run (urj_chain_t *chain, char *params[])
{
    if (urj_cmd_params (params) != 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (0)
        ;
    else if (strcasecmp(params[1], "all") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_ALL;
    else if (strcasecmp(params[1], "comm") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_COMM;
    else if (strcasecmp(params[1], "debug") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_DEBUG;
    else if (strcasecmp(params[1], "detail") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_DETAIL;
    else if (strcasecmp(params[1], "normal") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_NORMAL;
    else if (strcasecmp(params[1], "warning") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_WARNING;
    else if (strcasecmp(params[1], "error") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_ERROR;
    else if (strcasecmp(params[1], "silent") == 0)
        urj_log_state.level = URJ_LOG_LEVEL_SILENT;
    else
    {
        urj_error_set (URJ_ERROR_SYNTAX, "unknown log level '%s'", params[1]);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
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

const urj_cmd_t urj_cmd_debug = {
    "debug",
    N_("set logging/debugging level"),
    cmd_debug_help,
    cmd_debug_run
};
