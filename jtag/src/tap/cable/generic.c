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

#include <stdlib.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

cable_t *
generic_connect( cable_driver_t *cable_driver, parport_t *port )
{
	generic_params_t *params = malloc( sizeof *params );
	cable_t *cable = malloc( sizeof *cable );
	if (!params || !cable) {
		free( params );
		free( cable );
		return NULL;
	}

	cable->driver = cable_driver;
	cable->port = port;
	cable->params = params;
	cable->chain = NULL;

	return cable;
}

void
generic_disconnect( cable_t *cable )
{
	cable_done( cable );
	chain_disconnect( cable->chain );
}

void
generic_cable_free( cable_t *cable )
{
	cable->port->driver->parport_free( cable->port );
	free( cable->params );
	free( cable );
}

void
generic_done( cable_t *cable )
{
	parport_close( cable->port );
}

int
generic_transfer( cable_t *cable, int len, char *in, char *out )
{
	int i;

	if(out)
		for(i=0; i<len; i++) {
			out[i] = cable_get_tdo( cable );
			cable_clock( cable, 0, in[i], 1 );
		}
	else
		for(i=0; i<len; i++) {
			cable_clock( cable, 0, in[i], 1 );
		}

	return i;
}

int
generic_get_trst( cable_t *cable )
{
	return PARAM_TRST(cable);
}
