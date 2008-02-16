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

#include <jim.h>

/* private parameters of this cable driver */
typedef struct
{
    jim_state_t *s;
}
jim_cable_params_t;

int
jim_cable_connect( char *params[], cable_t *cable )
{
	if ( cmd_params( params ) < 1 ) {
	  printf( _("not enough arguments!\n") );
	  return 1;
	}

	printf( _("JTAG target simulator JIM - work in progress!\n"));

	cable->chain = NULL;
    cable->params = (jim_cable_params_t *)malloc(sizeof(jim_cable_params_t));

    if(cable->params != NULL)
    {
        jim_state_t *s;
        s = jim_init();
        if(s == NULL)
        {
            free(cable->params);
            cable->params = NULL;
        }
        else
        {
            ((jim_cable_params_t *)(cable->params))->s = s;
        }
    }

    if(cable->params == NULL)
    {
      printf(_("Initialization failed.\n"));
      return 1;
    };

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
    if(cable->params != NULL)
    {
        jim_free( ((jim_cable_params_t*)(cable->params))->s );
        free( cable->params );
    };
	free( cable );
}

void
jim_cable_done( cable_t *cable )
{
}

static int
jim_cable_init( cable_t *cable )
{
    return 0;
}

static void
jim_cable_clock( cable_t *cable, int tms, int tdi, int n )
{
    int i;
    jim_cable_params_t *jcp = (jim_cable_params_t*)(cable->params);

    for(i = 0; i < n; i++)
    {
        jim_tck_rise( jcp->s, tms, tdi );
        jim_tck_fall( jcp->s );
    }
}

static int
jim_cable_get_tdo( cable_t *cable )
{
    jim_cable_params_t *jcp = (jim_cable_params_t*)(cable->params);

    return jim_get_tdo( jcp->s );
}

static int
jim_cable_get_trst( cable_t *cable )
{
    jim_cable_params_t *jcp = (jim_cable_params_t*)(cable->params);

    return jim_get_trst( jcp->s );
}

static int
jim_cable_set_trst( cable_t *cable, int trst )
{
    jim_cable_params_t *jcp = (jim_cable_params_t*)(cable->params);

    jim_set_trst( jcp->s, trst );
    return jim_get_trst( jcp->s );
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
	generic_set_frequency,
	jim_cable_clock,
	jim_cable_get_tdo,
	generic_transfer,
	jim_cable_set_trst,
	jim_cable_get_trst,
	generic_flush_using_transfer,
	jim_cable_help
};

