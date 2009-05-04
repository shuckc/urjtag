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

#include <urjtag/sysdep.h>

#include <stdio.h>
#include <string.h>

#include <urjtag/chain.h>
#include <urjtag/tap.h>
#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_detect_run (urj_chain_t *chain, char *params[])
{
    int i;
    urj_bus_t *abus;

    if (urj_cmd_params (params) != 1)
        return -1;

    if (!urj_cmd_test_cable (chain))
        return 1;

    urj_bus_buses_free ();
    urj_part_parts_free (chain->parts);
    chain->parts = NULL;
    urj_tap_detect_parts (chain, urj_get_data_dir ());
    if (!chain->parts)
        return 1;
    if (!chain->parts->len)
    {
        urj_part_parts_free (chain->parts);
        chain->parts = NULL;
        return 1;
    }
    urj_part_parts_set_instruction (chain->parts, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);
    urj_tap_chain_shift_data_registers (chain, 1);
    urj_part_parts_set_instruction (chain->parts, "BYPASS");
    urj_tap_chain_shift_instructions (chain);

    // Initialize all the buses
    for (i = 0; i < urj_buses.len; i++)
    {
        abus = urj_buses.buses[i];
        if (abus->driver->init)
        {
            if (abus->driver->init (abus) != URJ_STATUS_OK)
                return -1;
        }
    }

    return 1;
}

static void
cmd_detect_help (void)
{
    printf (_("Usage: %s\n"
              "Detect parts on the JTAG chain.\n"
              "\n"
              "Output from this command is a list of the detected parts.\n"
              "If no parts are detected other commands may not work properly.\n"),
            "detect");
}

const urj_cmd_t urj_cmd_detect = {
    "detect",
    N_("detect parts on the JTAG chain"),
    cmd_detect_help,
    cmd_detect_run
};
