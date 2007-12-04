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

#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

#include <brux/cmd.h>

int
generic_connect( char *params[], cable_t *cable )
{
	generic_params_t *cable_params = malloc( sizeof *cable_params );
	parport_t *port;
	int i;

	if ( cmd_params( params ) < 3 ) {
	  printf( _("not enough arguments!\n") );
	  return 1;
	}
	  
	/* search parport driver list */
	for (i = 0; parport_drivers[i]; i++)
		if (strcasecmp( params[1], parport_drivers[i]->type ) == 0)
			break;
	if (!parport_drivers[i]) {
		printf( _("Unknown port driver: %s\n"), params[1] );
		return 2;
	}

	/* set up parport driver */
	port = parport_drivers[i]->connect( (const char **) &params[2],
					    cmd_params( params ) - 2 );

        if (port == NULL) {
	  printf( _("Error: Cable connection failed!\n") );
	  return 3;
        }

	if (!cable_params) {
		free( cable_params );
		free( cable );
		return 4;
	}

	cable->port = port;
	cable->params = cable_params;
	cable->chain = NULL;

	return 0;
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
