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

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include <urjtag/jtag.h>

#include <urjtag/cmd.h>

extern urj_cmd_t urj_cmd_quit;
extern urj_cmd_t urj_cmd_help;
extern urj_cmd_t urj_cmd_frequency;
extern urj_cmd_t urj_cmd_cable;
extern urj_cmd_t urj_cmd_reset;
extern urj_cmd_t urj_cmd_discovery;
extern urj_cmd_t urj_cmd_idcode;
extern urj_cmd_t urj_cmd_detect;
extern urj_cmd_t urj_cmd_signal;
extern urj_cmd_t urj_cmd_scan;
extern const urj_cmd_t urj_cmd_salias;
extern urj_cmd_t urj_cmd_bit;
extern urj_cmd_t urj_cmd_register;
extern const urj_cmd_t urj_cmd_initbus;
extern urj_cmd_t urj_cmd_print;
extern urj_cmd_t urj_cmd_part;
extern urj_cmd_t urj_cmd_bus;
extern urj_cmd_t urj_cmd_instruction;
extern urj_cmd_t urj_cmd_shift;
extern urj_cmd_t urj_cmd_dr;
extern urj_cmd_t urj_cmd_get;
extern urj_cmd_t urj_cmd_test;
extern urj_cmd_t urj_cmd_shell;
extern urj_cmd_t urj_cmd_set;
extern urj_cmd_t urj_cmd_endian;
extern urj_cmd_t urj_cmd_peek;
extern urj_cmd_t urj_cmd_poke;
extern urj_cmd_t urj_cmd_pod;
extern urj_cmd_t urj_cmd_readmem;
extern urj_cmd_t urj_cmd_writemem;
extern urj_cmd_t urj_cmd_detectflash;
extern urj_cmd_t urj_cmd_flashmem;
extern urj_cmd_t urj_cmd_eraseflash;
extern urj_cmd_t urj_cmd_script;
extern urj_cmd_t urj_cmd_include;
extern urj_cmd_t urj_cmd_addpart;
extern urj_cmd_t urj_cmd_usleep;
#ifdef ENABLE_SVF
extern urj_cmd_t urj_cmd_svf;
#endif
#ifdef ENABLE_BSDL
extern urj_cmd_t urj_cmd_bsdl;
#endif
extern urj_cmd_t urj_cmd_debug;

const urj_cmd_t *urj_cmds[] = {
    &urj_cmd_quit,
    &urj_cmd_help,
    &urj_cmd_frequency,
    &urj_cmd_cable,
    &urj_cmd_reset,
    &urj_cmd_discovery,
    &urj_cmd_idcode,
    &urj_cmd_detect,
    &urj_cmd_signal,
    &urj_cmd_scan,
    &urj_cmd_salias,
    &urj_cmd_bit,
    &urj_cmd_register,
    &urj_cmd_initbus,
    &urj_cmd_print,
    &urj_cmd_part,
    &urj_cmd_bus,
    &urj_cmd_instruction,
    &urj_cmd_shift,
    &urj_cmd_dr,
    &urj_cmd_get,
    &urj_cmd_test,
    &urj_cmd_shell,
    &urj_cmd_set,
    &urj_cmd_endian,
    &urj_cmd_peek,
    &urj_cmd_poke,
    &urj_cmd_pod,
    &urj_cmd_readmem,
    &urj_cmd_writemem,
    &urj_cmd_detectflash,
    &urj_cmd_flashmem,
    &urj_cmd_eraseflash,
    &urj_cmd_script,
    &urj_cmd_include,
    &urj_cmd_addpart,
    &urj_cmd_usleep,
#ifdef ENABLE_SVF
    &urj_cmd_svf,
#endif
#ifdef ENABLE_BSDL
    &urj_cmd_bsdl,
#endif
    &urj_cmd_debug,
    NULL                        /* last must be NULL */
};

int
urj_cmd_test_cable (urj_chain_t *chain)
{
    if (chain->cable)
        return 1;

    printf (_
            ("Error: Cable not configured. Please use '%s' command first!\n"),
            "cable");
    return 0;
}

/* Remainder copied from libbrux/cmd/cmd.c */

int
urj_cmd_run (urj_chain_t *chain, char *params[])
{
    int i, pidx;
    size_t len;

    if (!params[0])
        return 1;

    pidx = -1;
    len = strlen (params[0]);

    for (i = 0; urj_cmds[i]; ++i)
    {
        if (strcasecmp (urj_cmds[i]->name, params[0]) == 0)
        {
            int r;

          run_cmd:
            r = urj_cmds[i]->run (chain, params);
            if (r < 0)
                printf (_("%s: syntax error!\n"), params[0]);
            return r;
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
        printf (_("%s: Ambiguous command\n"), params[0]);
        break;
    case -1:
        printf (_("%s: unknown command\n"), params[0]);
        break;
    default:
        i = pidx;
        goto run_cmd;
    }

    return 1;
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
urj_cmd_get_number (char *s, unsigned int *i)
{
    int n;
    int r;
    size_t l;

    if (!s || !i)
        return -1;

    l = strlen (s);

    n = -1;
    r = sscanf (s, "0x%x%n", i, &n);
    if (r == 1 && n == l)
        return 0;

    n = -1;
    r = sscanf (s, "%u%n", i, &n);
    if (r == 1 && n == l)
        return 0;

    return -1;
}
