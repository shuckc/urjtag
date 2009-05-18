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

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/cable.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_frequency_run (urj_chain_t *chain, char *params[])
{
    long unsigned freq;

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_params (params) == 1)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Current TCK frequency is %lu Hz\n"),
                 (long unsigned) urj_tap_cable_get_frequency (chain->cable));
        return URJ_STATUS_OK;
    }

    if (urj_cmd_get_number (params[1], &freq) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Setting TCK frequency to %lu Hz\n"),
             freq);
    urj_tap_cable_set_frequency (chain->cable, freq);

    return URJ_STATUS_OK;
}

static void
cmd_frequency_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s [FREQ]\n"
               "Change TCK frequency to FREQ or print current TCK frequency.\n"
               "\n"
               "FREQ is in hertz. It's a maximum TCK frequency for JTAG interface.\n"
               "In some cases the TCK frequency is less than FREQ, but the frequency\n"
               "is never more than FREQ. Maximum supported frequency depends on JTAG\n"
               "adapter.\n"
               "\n"
               "FREQ must be an unsigned integer. Minimum allowed frequency is 1 Hz.\n"
               "Use 0 for FREQ to disable frequency limit.\n"),
             "frequency");
}

const urj_cmd_t urj_cmd_frequency = {
    "frequency",
    N_("setup JTAG frequency"),
    cmd_frequency_help,
    cmd_frequency_run
};
