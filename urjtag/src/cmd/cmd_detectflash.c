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

#include <urjtag/bus.h>
#include <urjtag/flash.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_detectflash_run (urj_chain_t *chain, char *params[])
{
    uint32_t adr;

    if (urj_cmd_params (params) != 2)
        return -1;

    if (!urj_bus)
    {
        printf (_("Error: Bus driver missing.\n"));
        return 1;
    }

    if (urj_cmd_get_number (params[1], &adr))
        return -1;

    urj_flash_detectflash (urj_bus, adr);

    return 1;
}

static void
cmd_detectflash_help (void)
{
    printf (_("Usage: %s ADDRESS\n"
              "Detect flash memory type connected to a part.\n"
              "\n"
              "ADDRESS    Base address for memory region\n"),
            "detectflash");
}

const urj_cmd_t urj_cmd_detectflash = {
    "detectflash",
    N_("detect parameters of flash chips attached to a part"),
    cmd_detectflash_help,
    cmd_detectflash_run
};
