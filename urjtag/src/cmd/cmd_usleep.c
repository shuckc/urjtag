/*
 * $Id$
 *
 * Copyright (C) 2008 Stanislav Sinyagin
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
 * Written by Stanislav Sinyagin <ssinyagin@k-open.com>, 2008.
 *
 */

#include <sysdep.h>

#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <urjtag/error.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_usleep_run (urj_chain_t *chain, char *params[])
{
    long unsigned usecs;

    if (urj_cmd_params (params) != 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_get_number (params[1], &usecs) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    usleep (usecs);

    return URJ_STATUS_OK;
}

static void
cmd_usleep_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s USECS\n"
               "Sleep some number of microseconds.\n"),
             "usleep");
}

const urj_cmd_t urj_cmd_usleep = {
    "usleep",
    N_("Sleep some number of microseconds"),
    cmd_usleep_help,
    cmd_usleep_run
};
