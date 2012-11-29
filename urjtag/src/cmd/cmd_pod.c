/*
 * $Id$
 *
 * Copyright (C) 2008 K. Waschk
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
 * Written by Kolja Waschk, 2008
 *   based on idea and code by Sebastian Hesselbarth, 2008
 *   and code by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/cable.h>
#include <urjtag/pod.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_pod_run (urj_chain_t *chain, char *params[])
{
    int i, j;
    int mask = 0;
    int val = 0;

    if ((i = urj_cmd_params (params)) < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    for (j = 1; j < i; j++)
    {
        if (strncasecmp (params[j], "tdo", 3) == 0)
        {
            /* We only read this, so move on to the next param. */
            urj_log (URJ_LOG_LEVEL_NORMAL, "%d\n",
                     urj_tap_cable_get_tdo (chain->cable));
            continue;
        }

        char *eq = strrchr (params[j], '=');
        if (!eq)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           "parameter format should be 'SIGNAL=[0|1]', not '%s'",
                           params[j]);
            return URJ_STATUS_FAIL;
        }

        urj_pod_sigsel_t it = URJ_POD_CS_NONE;
        size_t n = strlen (params[j]);

        if (n > 4 && (strncasecmp (params[j], "tck", 3) == 0))
            it = URJ_POD_CS_TCK;
        else if (n > 4 && (strncasecmp (params[j], "tms", 3) == 0))
            it = URJ_POD_CS_TMS;
        else if (n > 4 && (strncasecmp (params[j], "tdi", 3) == 0))
            it = URJ_POD_CS_TDI;
        else if (n > 5 && (strncasecmp (params[j], "trst", 3) == 0))
            it = URJ_POD_CS_TRST;
        else if (n > 6 && (strncasecmp (params[j], "reset", 3) == 0))
            it = URJ_POD_CS_RESET;
        if (it == URJ_POD_CS_NONE)
        {
            urj_error_set (URJ_ERROR_SYNTAX, "illegal signal name in '%s'",
                           params[j]);
            return URJ_STATUS_FAIL;
        }

        mask |= it;
        if (atoi (eq + 1) != 0)
            val |= it;
    }

    if (mask)
    {
        /* If user just read TDO, then there's nothing to set. */
        if (urj_tap_chain_set_pod_signal (chain, mask, val) == -1)
            return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static void
cmd_pod_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                  char * const *tokens, const char *text, size_t text_len,
                  size_t token_point)
{
    static const char * const signals[] = {
        "TCK=",
        "TMS=",
        "TDI=",
        "TRST=",
        "RESET=",
        "TDO",
    };

    urj_completion_mayben_add_matches (matches, match_cnt, text,
                                       text_len, signals);
}

static void
cmd_pod_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s <TDO|SIGNAL=#> [<TDO|SIGNAL=#> ...]\n"
               "Set or read state of POD signal(s).\n"
               "TDO is the only signal which may be read.  All other signals,\n"
               "if specified, must be set.\n"
               "\n"
               "SIGNAL           TCK, TMS, TDI, TRST, or RESET\n"
               "#                0 or 1\n"),
             "pod");
}

const urj_cmd_t urj_cmd_pod = {
    "pod",
    N_("Set or read state of POD signal(s)"),
    cmd_pod_help,
    cmd_pod_run,
    cmd_pod_complete,
};
