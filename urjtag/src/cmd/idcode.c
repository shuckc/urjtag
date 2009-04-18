/*
 * $Id: idcode.c 1102 2008-02-27 03:38:31Z jiez $
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
 * Written by Uwe Bonnes <bon@elektron.ikp.physik.tu-darmstadt.de>, 2008.
 *
 */
#include "sysdep.h"

#include <stdio.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_idcode_run( chain_t *chain, char *params[] )
{
        unsigned int bytes=0;

        if (cmd_params( params ) == 1) {
                bytes = 0;
        }

        else if (cmd_params( params ) >2)
                return -1;

        else if (cmd_get_number( params[1], &bytes ))
                return -1;

        if (!cmd_test_cable( chain ))
                return 1;

        printf( _("Reading %d bytes if idcode\n"), bytes );
	idcode( chain , bytes);
        return 1;
}

static void
cmd_idcode_help( void )
{
        printf( _(
                "Usage: %s [BYTES]\n"
                "Read [BYTES]|all IDCODEs of all parts in a JTAG chain.\n"
                "\n"
                "BYTES must be an unsigned integer, Use 0 for BYTES to read all bytes\n"
        ), "idcode" );
}

cmd_t cmd_idcode = {
  "idcode",
  N_("Read IDCODEs of all parts in a JTAG chain"),
  cmd_idcode_help,
  cmd_idcode_run
};
