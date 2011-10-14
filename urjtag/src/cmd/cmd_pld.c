/*
 * $Id$
 *
 * Support for Programmable Logic Devices (PLD)
 *
 * Copyright (C) 2010, Michael Walle
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
 * Written by Michael Walle <michael@walle.cc>, 2010
 *
 */

#include <sysdep.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <urjtag/error.h>
#include <urjtag/log.h>

#include <urjtag/pld.h>
#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_pld_run (urj_chain_t *chain, char *params[])
{
    FILE *pld_file;
    int num_params;
    int result = URJ_STATUS_OK;

    num_params = urj_cmd_params (params);
    if (num_params < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: #parameters should be >= %d, not %d"),
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (strcasecmp (params[1], "load") == 0)
    {
        if (num_params < 3)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           _("%s: no filename specified"),
                           params[0]);
            return URJ_STATUS_FAIL;
        }

        if ((pld_file = fopen (params[2], FOPEN_R)) != NULL)
        {
            result = urj_pld_configure (chain, pld_file);
            fclose (pld_file);
        }
        else
        {
            urj_error_IO_set (_("%s: cannot open file '%s'"),
                              params[0], params[1]);
            result = URJ_STATUS_FAIL;
        }
    }
    else if (strcasecmp (params[1], "status") == 0)
    {
        result = urj_pld_print_status (chain);
    }
    else if (strcasecmp (params[1], "readreg") == 0)
    {
        unsigned long reg;

        if (num_params < 3)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           _("%s: #parameters should be >= %d, not %d"),
                           params[0], 3, urj_cmd_params (params));
            return URJ_STATUS_FAIL;
        }

        if (urj_cmd_get_number (params[2], &reg) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

        result = urj_pld_read_register (chain, reg);
    }
    else if (strcasecmp (params[1], "writereg") == 0)
    {
        unsigned long reg;
        unsigned long value;

        if (num_params < 4)
        {
            urj_error_set (URJ_ERROR_SYNTAX,
                           _("%s: #parameters should be >= %d, not %d"),
                           params[0], 4, urj_cmd_params (params));
            return URJ_STATUS_FAIL;
        }

        if (urj_cmd_get_number (params[2], &reg) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

        if (urj_cmd_get_number (params[3], &value) != URJ_STATUS_OK)
            return URJ_STATUS_FAIL;

        result = urj_pld_write_register (chain, reg, value);
    }
    else if (strcasecmp (params[1], "reconfigure") == 0)
    {
        result = urj_pld_reconfigure (chain);
    }
    else
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       _("%s: unknown command"), params[0]);
        return URJ_STATUS_FAIL;
    }

    return result;
}


static void
cmd_pld_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s load PLDFILE\n"
               "Usage: %s reconfigure\n"
               "Usage: %s status\n"
               "Usage: %s readreg REG\n"
               "Usage: %s writereg REG VALUE\n"
               "Configure FPGA from PLDFILE, query status, read and write registers.\n"),
             "pld", "pld", "pld", "pld", "pld");
}

const urj_cmd_t urj_cmd_pld = {
    "pld",
    N_("configure a Programmable Logic Device from file"),
    cmd_pld_help,
    cmd_pld_run
};
