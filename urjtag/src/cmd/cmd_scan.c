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
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>
#include <urjtag/bssignal.h>
#include <urjtag/bsbit.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_scan_run (urj_chain_t *chain, char *params[])
{
    urj_part_t *part;
    urj_data_register_t *bsr;
    urj_tap_register_t *obsr;
    int i;

    if ((i = urj_cmd_params (params)) < 1)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 1, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    part = urj_tap_chain_active_part (chain);
    if (part == NULL)
        return URJ_STATUS_FAIL;

    /* search for Boundary Scan Register */
    bsr = urj_part_find_data_register (part, "BSR");
    if (!bsr)
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("Boundary Scan Register (BSR) not found"));
        return URJ_STATUS_FAIL;
    }

    if (urj_part_find_instruction (part, "SAMPLE"))
    {
        urj_part_set_instruction (part, "SAMPLE");
    }
    else if (urj_part_find_instruction (part, "SAMPLE/PRELOAD"))
    {
        urj_part_set_instruction (part, "SAMPLE/PRELOAD");
    }
    else
    {
        urj_error_set (URJ_ERROR_UNSUPPORTED,_("Part can't SAMPLE"));
        return URJ_STATUS_FAIL;
    }

    urj_tap_chain_shift_instructions (chain);

    obsr = urj_tap_register_alloc (bsr->out->len);
    if (!obsr)
        return URJ_STATUS_FAIL;

    urj_tap_register_init (obsr, urj_tap_register_get_string (bsr->out));   // copy

    urj_tap_chain_shift_data_registers (chain, 1);

    urj_part_signal_t *s;
    for (s = part->signals; s; s = s->next)
    {
        if (s->input != NULL)
        {
            int old = obsr->data[s->input->bit];
            int new = bsr->out->data[s->input->bit];
            if (old != new)
            {
                urj_part_salias_t *a;
                urj_log (URJ_LOG_LEVEL_NORMAL, "%s", s->name);
                for (a = part->saliases; a; a = a->next)
                {
                    if (a->signal == s)
                        urj_log (URJ_LOG_LEVEL_NORMAL, ",%s", a->name);
                }
                urj_log (URJ_LOG_LEVEL_NORMAL, _(": %d > %d\n"), old, new);
            }
        }
    }

    urj_tap_register_free (obsr);

    return URJ_STATUS_OK;
}

static void
cmd_scan_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s [SIGNAL]* \n"
               "Read BSR and show changes since last scan.\n"),
             "scan");
}

const urj_cmd_t urj_cmd_scan = {
    "scan",
    N_("read BSR and show changes since last scan"),
    cmd_scan_help,
    cmd_scan_run
};
