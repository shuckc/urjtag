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
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static urj_endian_t current_file_endian = URJ_ENDIAN_LITTLE;

urj_endian_t urj_get_file_endian (void)
{
    return current_file_endian;
}

void urj_set_file_endian (urj_endian_t new_file_endian)
{
    current_file_endian = new_file_endian;
}

static const struct {
    const urj_endian_t endian;
    const char *name;
} endians[] = {
    { URJ_ENDIAN_LITTLE,  "little", },
    { URJ_ENDIAN_BIG,     "big",    },
    { URJ_ENDIAN_UNKNOWN, "unknown" },
};

const char *urj_endian_to_string (urj_endian_t endian)
{
    size_t idx;

    for (idx = 0; idx < ARRAY_SIZE(endians); ++idx)
        if (endian == endians[idx].endian)
            return endians[idx].name;

    /* last one is the "unknown" one */
    return endians[idx - 1].name;
}

urj_endian_t urj_endian_from_string (const char *strendian)
{
    size_t idx;

    for (idx = 0; idx < ARRAY_SIZE(endians); ++idx)
        if (!strcasecmp (endians[idx].name, strendian))
            return endians[idx].endian;

    /* last one is the "unknown" one */
    return endians[idx - 1].endian;
}

static int
cmd_endian_run (urj_chain_t *chain, char *params[])
{
    urj_endian_t new_endian;

    if (urj_cmd_params (params) > 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 2, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (!params[1])
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, _("Endianness for external files: %s\n"),
                 urj_endian_to_string (urj_get_file_endian ()));
        return URJ_STATUS_OK;
    }

    new_endian = urj_endian_from_string (params[1]);
    if (new_endian != URJ_ENDIAN_UNKNOWN)
    {
        urj_set_file_endian (new_endian);
        return URJ_STATUS_OK;
    }

    urj_error_set (URJ_ERROR_SYNTAX,
                   _("endianness must be 'little' or 'big', not '%s'"),
                   params[1]);
    return URJ_STATUS_FAIL;
}

static void
cmd_endian_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s\n"
               "Set or print endianness for external files.\n"),
             "endian [little|big]");
}

static void
cmd_endian_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                     char * const *tokens, const char *text, size_t text_len,
                     size_t token_point)
{
    size_t i;

    if (token_point != 1)
        return;

    for (i = 0; i < ARRAY_SIZE(endians); ++i)
        urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                         endians[i].name);
}

const urj_cmd_t urj_cmd_endian = {
    "endian",
    N_("set/print endianness"),
    cmd_endian_help,
    cmd_endian_run,
    cmd_endian_complete,
};
