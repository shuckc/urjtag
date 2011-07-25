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
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <urjtag/error.h>
#include <urjtag/bus.h>
#include <urjtag/flash.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_flashmem_run (urj_chain_t *chain, char *params[])
{
    int msbin;
    int noverify = 0;
    long unsigned adr = 0;
    FILE *f;
    int paramc = urj_cmd_params (params);
    int r;

    if (paramc < 3)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be >= %d, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (!urj_bus)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, _("Bus driver missing"));
        return URJ_STATUS_FAIL;
    }

    msbin = strcasecmp ("msbin", params[1]) == 0;
    if (!msbin && urj_cmd_get_number (params[1], &adr) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (paramc > 3)
        noverify = strcasecmp ("noverify", params[3]) == 0;
    else
        noverify = 0;

    f = fopen (params[2], FOPEN_R);
    if (!f)
    {
        urj_error_IO_set (_("Unable to open file `%s'"), params[2]);
        return URJ_STATUS_FAIL;
    }

    if (msbin)
        r = urj_flashmsbin (urj_bus, f, noverify);
    else
        r = urj_flashmem (urj_bus, f, adr, noverify);

    fclose (f);

    return r;
}

static void
cmd_flashmem_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s ADDR FILENAME [noverify]\n"
               "Usage: %s FILENAME [noverify]\n"
               "Program FILENAME content to flash memory.\n"
               "\n"
               "ADDR       target address for raw binary image\n"
               "FILENAME   name of the input file\n"
               "%-10s FILENAME is in MS .bin format (for WinCE)\n"
               "%-10s if specified, verification is skipped\n"
               "\n"
               "ADDR could be in decimal or hexadecimal (prefixed with 0x) form.\n"
               "\n"
               "Supported Flash Memories:\n"),
             "flashmem", "flashmem msbin", "msbin", "noverify");

    urj_cmd_show_list (urj_flash_flash_drivers);
}

static void
cmd_flashmem_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                       char * const *tokens, const char *text, size_t text_len,
                       size_t token_point)
{
    switch (token_point)
    {
    case 1: /* [addr|msbin] */
        break;

    case 2: /* filename */
        urj_completion_mayben_add_file (matches, match_cnt, text,
                                        text_len, false);
        break;

    case 3: /* [noverify] */
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "noverify");
        break;
    }
}

const urj_cmd_t urj_cmd_flashmem = {
    "flashmem",
    N_("burn flash memory with data from a file"),
    cmd_flashmem_help,
    cmd_flashmem_run,
    cmd_flashmem_complete,
};
