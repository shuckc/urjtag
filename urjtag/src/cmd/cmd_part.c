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
#include <stdlib.h>

#include <urjtag/error.h>
#include <urjtag/chain.h>
#include <urjtag/part.h>

#include <urjtag/cmd.h>

#include "cmd.h"

static int
cmd_part_run (urj_chain_t *chain, char *params[])
{
    long unsigned n;

    if (urj_cmd_params (params) > 3)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be <= %d, not %d",
                       params[0], 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

/* part alias U1 (3 params) */
    if (urj_cmd_params (params) == 3)
    {
        if (strcasecmp (params[1], "alias") == 0)
        {
            urj_part_t *part = urj_tap_chain_active_part (chain);

            if (part == NULL)
                return URJ_STATUS_FAIL;

            part->alias = strdup (params[2]);
            if (part->alias == NULL)
            {
                urj_error_set(URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails",
                              params[2]);
                return URJ_STATUS_FAIL;
            }

            return URJ_STATUS_OK;
        }
    }

    if (urj_cmd_params (params) != 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "%s: #parameters should be %d or %d, not %d",
                       params[0], 2, 3, urj_cmd_params (params));
        return URJ_STATUS_FAIL;
    }

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (!chain->parts)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, "Run \"detect\" first");
        return URJ_STATUS_FAIL;
    }

/* Search for alias too djf */
    if (urj_cmd_get_number (params[1], &n) != URJ_STATUS_OK)
    {

        /* Search all parts to check their aliases */
        int i;
        char *a;

        for (i = 0; i < chain->parts->len; i++)
        {
            a = chain->parts->parts[i]->alias;
            if (a && strcasecmp (a, params[1]) == 0)
                break;
        }
        if (i < chain->parts->len)
        {
            n = i;
        }
        else
        {
            urj_error_set (URJ_ERROR_NOTFOUND, "part '%s'", params[1]);
            return URJ_STATUS_FAIL;
        }
    }

    if (n >= chain->parts->len)
    {
        urj_error_set (URJ_ERROR_INVALID,
                       _("%s: invalid part number %lu, max %d"), "part",
                       n, chain->parts->len);
        return URJ_STATUS_FAIL;
    }

    chain->active_part = n;

    return URJ_STATUS_OK;
}

static void
cmd_part_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                   char * const *tokens, const char *text, size_t text_len,
                   size_t token_point)
{
    int i;

    if (token_point != 1)
        return;

    urj_completion_mayben_add_match (matches, match_cnt, text, text_len, "alias");

    for (i = 0; i < chain->parts->len; ++i)
    {
        /* We assume you'll never have more than 15*10 parts */
        char num[16];

        sprintf (num, "%i", i);
        urj_completion_mayben_add_match (matches, match_cnt, text,
                                         text_len, num);

        if (chain->parts->parts[i]->alias)
            urj_completion_mayben_add_match (matches, match_cnt, text, text_len,
                                             chain->parts->parts[i]->alias);
    }
}

static void
cmd_part_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s [PART|ALIAS]\n"
               "Change active part for current JTAG chain.\n\n"
               "Usage: %s ALIAS\n"
               "Assign an alias for the active part.\n"),
             "part", "part alias");
}

const urj_cmd_t urj_cmd_part = {
    "part",
    N_("change active part for current JTAG chain"),
    cmd_part_help,
    cmd_part_run,
    cmd_part_complete,
};
