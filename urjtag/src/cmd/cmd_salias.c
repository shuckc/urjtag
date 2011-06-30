/*
 * $Id$
 *
 * Copyright (C) 2003 Marcel Telka
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
cmd_salias_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    urj_part_signal_t *s;
    urj_part_salias_t *sa;

    if (urj_cmd_params (params) != 3)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    if (urj_part_find_signal (part, params[1]) != NULL)
    {
        return URJ_STATUS_FAIL;
    }

    s = urj_part_find_signal (part, params[2]);
    if (s == NULL)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, _("Signal '%s' not found"),
                       params[2]);
        return URJ_STATUS_FAIL;
    }

    sa = urj_part_salias_alloc (params[1], s);
    if (!sa)
        return URJ_STATUS_FAIL;

    sa->next = part->saliases;
    part->saliases = sa;

    return URJ_STATUS_OK;
}

static void
cmd_salias_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s ALIAS SIGNAL\n"
               "Define new signal ALIAS as alias for existing SIGNAL.\n"
               "\n"
               "ALIAS         New signal alias name\n"
               "SIGNAL        Existing signal name\n"),
             "signal");
}

static void
cmd_salias_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                     char * const *tokens, const char *text, size_t text_len,
                     size_t token_point)
{
    if (token_point != 2)
        return;

    cmd_signal_complete (chain, matches, match_cnt, text, text_len);
}

const urj_cmd_t urj_cmd_salias = {
    "salias",
    N_("define an alias for a signal"),
    cmd_salias_help,
    cmd_salias_run,
    cmd_salias_complete,
};
