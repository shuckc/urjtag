/*
 * $Id$
 *
 * Copyright (C) 2011 Xiangfu Liu <xiangfu@sharism.cc>
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
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/bus.h>
#include <urjtag/flash.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_lockflash_run (urj_chain_t *chain, char *params[])
{
    long unsigned adr = 0;
    long unsigned number = 0;
    int unlock = 0;

    if (urj_cmd_params (params) != 3)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (!urj_bus)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, _("Bus driver missing"));
        return URJ_STATUS_FAIL;
    }
    if (urj_cmd_get_number (params[1], &adr) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;
    if (urj_cmd_get_number (params[2], &number) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (!strcmp(params[0], "unlockflash"))
        unlock = 1;

    return urj_flashlock (urj_bus, adr, number, unlock);
}

static void
cmd_lockflash_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s ADDR BLOCKS\n"
               "Lock flash memory from ADDR.\n"
               "\n"
               "ADDR       target address for erasing block\n"
               "BLOCKS     number of blocks to lock\n"
               "\n"
               "ADDR and BLOCKS could be in decimal or hexadecimal (prefixed with 0x) form.\n"
               "\n" "Supported Flash Memories:\n"),
             "lockflash");

    urj_cmd_show_list (urj_flash_flash_drivers);
}

const urj_cmd_t urj_cmd_lockflash = {
    "lockflash",
    N_("lock flash memory by number of blocks"),
    cmd_lockflash_help,
    cmd_lockflash_run,
};

static void
cmd_unlockflash_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s ADDR BLOCKS\n"
               "Unlock flash memory from ADDR.\n"
               "\n"
               "ADDR       target address for erasing block\n"
               "BLOCKS     number of blocks to lock\n"
               "\n"
               "ADDR and BLOCKS could be in decimal or hexadecimal (prefixed with 0x) form.\n"
               "\n" "Supported Flash Memories:\n"),
             "unlockflash");

    urj_cmd_show_list (urj_flash_flash_drivers);
}

const urj_cmd_t urj_cmd_unlockflash = {
    "unlockflash",
    N_("unlock flash memory by number of blocks"),
    cmd_unlockflash_help,
    cmd_lockflash_run,
};
