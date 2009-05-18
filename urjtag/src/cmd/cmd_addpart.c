/*
 * $Id$
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
 * Written by Ville Voipio <ville.voipio@iki.fi>, 2008.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/tap.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_addpart_run (urj_chain_t *chain, char *params[])
{
    long unsigned len;

    if (urj_cmd_params (params) != 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_get_number (params[1], &len) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (urj_tap_manual_add (chain, len) == -1)
        return URJ_STATUS_FAIL;

    // @@@@ RFHH this cannot be
    if (chain->parts == NULL)
        return URJ_STATUS_FAIL;

    // @@@@ RFHH this cannot be
    if (chain->parts->len == 0)
    {
        urj_part_parts_free (chain->parts);
        chain->parts = NULL;
        return URJ_STATUS_FAIL;
    }

    urj_part_parts_set_instruction (chain->parts, "BYPASS");
    urj_tap_chain_shift_instructions (chain);

    return URJ_STATUS_OK;
}


static void
cmd_addpart_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s IRLENGTH\n"
               "Manually add a part to the end of the chain.\n"
               "\n"
               "IRLENGTH           instruction register length\n"),
             "addpart");
}


const urj_cmd_t urj_cmd_addpart = {
    "addpart",
    N_("manually adds parts on the JTAG chain"),
    cmd_addpart_help,
    cmd_addpart_run
};
