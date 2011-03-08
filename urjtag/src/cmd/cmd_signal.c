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
#include <stdlib.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>
#include <urjtag/bssignal.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_signal_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    urj_part_signal_t *s;
    int i;

    if ((i = urj_cmd_params (params)) < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    if ((s = urj_part_find_signal (part, params[1])) != NULL)
    {
        if (i != 3)
        {
            urj_error_set (URJ_ERROR_ALREADY, _("Signal '%s' already defined"),
                           params[1]);
            return URJ_STATUS_FAIL;
        }

        urj_log (URJ_LOG_LEVEL_NORMAL, "Defining pin for signal %s\n",
                 s->name);

        return urj_part_signal_redefine_pin(chain, s, params[2]);
    }

    if (i == 3)
    {                           /* Add pin number */
        s = urj_part_signal_define_pin(chain, params[1], params[2]);
    }
    else
    {
        s = urj_part_signal_define(chain, params[1]);
    }
    if (s == NULL) {
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static void
cmd_signal_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s SIGNAL [PIN#]\n"
               "Define new signal with name SIGNAL for a part.\n"
               "\n"
               "SIGNAL           New signal name\n"
               "PIN#             List of pin # for a signal\n"),
             "signal");
}

/* This is used indirectly by other signal commands */
void
cmd_signal_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                     const char *text, size_t text_len)
{
    urj_part_t *part;
    urj_part_signal_t *s;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return;

    s = part->signals;
    while (s)
    {
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                         s->name);
        s = s->next;
    }
}

const urj_cmd_t urj_cmd_signal = {
    "signal",
    N_("define new signal for a part"),
    cmd_signal_help,
    cmd_signal_run
};
