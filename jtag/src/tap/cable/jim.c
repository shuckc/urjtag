/*
 * $Id: $
 *
 * JTAG target simulator JIM "cable" driver
 *
 * Copyright (C) 2008 Kolja Waschk
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
 */

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

#include <cmd.h>

int
jim_cable_connect( char *params[], cable_t *cable )
{
	if ( cmd_params( params ) < 1 ) {
	  printf( _("not enough arguments!\n") );
	  return 1;
	}

	cable->chain = NULL;

	return 0;
}

void
jim_cable_disconnect( cable_t *cable )
{
	cable_done( cable );
	chain_disconnect( cable->chain );
}

void
jim_cable_free( cable_t *cable )
{
	free( cable );
}

void
jim_cable_done( cable_t *cable )
{
}

static int
jim_cable_init( cable_t *cable )
{
	printf( _("JTAG target simulator JIM - work in progress!\n"));
    return 0;
}

static void
jim_cable_clock( cable_t *cable, int tms, int tdi, int n )
{
}

static int
jim_cable_get_tdo( cable_t *cable )
{
    return 0;
}

static int
jim_cable_get_trst( cable_t *cable )
{
    return 0;
}

static int
jim_cable_set_trst( cable_t *cable, int trst )
{
    return jim_cable_get_trst( cable );
}

static void
jim_cable_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s\n"
	),
    cablename
    );
}

cable_driver_t jim_cable_driver = {
	"JIM",
	N_("JTAG target simulator JIM"),
	jim_cable_connect,
	jim_cable_disconnect,
	jim_cable_free,
	jim_cable_init,
	jim_cable_done,
	jim_cable_clock,
	jim_cable_get_tdo,
	generic_transfer,
	jim_cable_set_trst,
	jim_cable_get_trst,
	jim_cable_help
};

