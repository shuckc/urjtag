/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <string.h>

#include <urjtag/error.h>
#include <urjtag/chain.h>

#include <urjtag/cmd.h>

#include "cmd.h"

const urj_cmd_t *urj_cmds[] = {
#define _URJ_CMD(cmd) &urj_cmd_##cmd,
#include "cmd_list.h"
    NULL                        /* last must be NULL */
};

/*
 * @param text match commands whose prefix equals <code>text</code>. Rotates
 *      through the registered commands. The prefix length is set when
 *      the rotating state is reset.
 * @@@@ RFHH that is weird behaviour. Why not do the prefix length as strlen(text)?
 */
char *
urj_cmd_find_next (const char *text, int state)
{
    static size_t cmd_idx, len;
    char *next = NULL;

    if (!state)
    {
        cmd_idx = 0;
        len = strlen (text);
    }

    while (urj_cmds[cmd_idx])
    {
        char *name = urj_cmds[cmd_idx++]->name;
        if (!strncmp (name, text, len))
        {
            next = strdup (name);
            if (next == NULL)
                urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "strdup(%s) fails",
                               name);
            break;
        }
    }

    return next;
}

int
urj_cmd_test_cable (urj_chain_t *chain)
{
    if (chain->cable)
        return URJ_STATUS_OK;

    urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                   "Cable not configured. Please use '%s' command first!",
                   "cable");
    return URJ_STATUS_FAIL;
}

/* Remainder copied from libbrux/cmd/cmd.c */

int
urj_cmd_run (urj_chain_t *chain, char *params[])
{
    int i, pidx;
    size_t len;

    if (!params[0])
        return URJ_STATUS_OK;

    pidx = -1;
    len = strlen (params[0]);

    for (i = 0; urj_cmds[i]; ++i)
    {
        if (strcasecmp (urj_cmds[i]->name, params[0]) == 0)
        {
          run_cmd:
            return urj_cmds[i]->run (chain, params);
        }
        else if (strncasecmp (urj_cmds[i]->name, params[0], len) == 0)
        {
            if (pidx == -1)
                pidx = i;
            else
                pidx = -2;
        }
    }

    switch (pidx)
    {
    case -2:
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: Ambiguous command\n"), params[0]);
        break;
    case -1:
        urj_log (URJ_LOG_LEVEL_NORMAL, _("%s: unknown command\n"), params[0]);
        break;
    default:
        i = pidx;
        goto run_cmd;
    }

    return URJ_STATUS_OK;
}

int
urj_cmd_params (char *params[])
{
    int i = 0;

    while (params[i])
        i++;

    return i;
}

int
urj_cmd_get_number (const char *s, long unsigned *i)
{
    int n;
    int r;
    size_t l;

    if (!s || !i)
    {
        urj_error_set (URJ_ERROR_INVALID, "NULL string or int pointer");
        return URJ_STATUS_FAIL;
    }

    l = strlen (s);

    n = -1;
    r = sscanf (s, "0x%lx%n", i, &n);
    if (r == 1 && n == l)
        return URJ_STATUS_OK;

    n = -1;
    r = sscanf (s, "%lu%n", i, &n);
    if (r == 1 && n == l)
        return URJ_STATUS_OK;

    urj_error_set (URJ_ERROR_SYNTAX, "not a number: '%s'", s);

    return URJ_STATUS_FAIL;
}
