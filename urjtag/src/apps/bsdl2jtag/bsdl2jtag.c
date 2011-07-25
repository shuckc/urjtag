/*
 * $Id$
 *
 * Copyright (C) 2009, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2009.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <urjtag/chain.h>
#include <urjtag/log.h>

#include <urjtag/bsdl_mode.h>
#include <urjtag/bsdl.h>


FILE *jtag_file;

static int
log_to_file (const char *fmt, va_list ap)
{
    return vfprintf (jtag_file, fmt, ap);
}


static void
cleanup (urj_chain_t *chain)
{
    urj_tap_chain_free (chain);
}


static void
usage (void)
{
    puts ("Usage:  bsdl2jtag <bsdl-file> <jtag-file>");
    puts ("Converts a BSDL file to a jtag part description.\n");
    puts ("Parameters");
    puts ("  bsdl-file : Name of BSDL file");
    puts ("  jtag-file : Name of converted jtag description file");
    puts ("");
}


int
main (int argc, char *const argv[])
{
    int result;
    urj_chain_t *chain = NULL;

    chain = urj_tap_chain_alloc ();
    if (chain == NULL)
    {
        urj_log_error_describe (URJ_LOG_LEVEL_ERROR);
        return 1;
    }

    if (argc != 3)
    {
        usage ();
        cleanup (chain);
        return 1;
    }

    jtag_file = fopen (argv[2], FOPEN_W);
    if (jtag_file == NULL)
    {
        printf ("Error: Can't open '%s' in write mode.\n", argv[2]);
        cleanup (chain);
        return 1;
    }

    /* log all messages to the jtag_file */
    urj_log_state.out_vprintf = log_to_file;
    result = urj_bsdl_read_file (chain, argv[1], URJ_BSDL_MODE_DUMP, NULL);
    if (result < 0)
        urj_log_error_describe (URJ_LOG_LEVEL_ERROR);

    fclose (jtag_file);
    cleanup (chain);
    return result < 0 ? 1 : 0;
}
