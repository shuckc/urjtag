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
#include <string.h>

#include "parport.h"
#include "tap.h"
#include "cable.h"
#include "chain.h"
#include "jtag.h"
#include "bus.h"

#include "cmd.h"

static int
cmd_cable_run( char *params[] )
{
	int i;

	/* we need at least one parameter for 'cable' command */
	if (cmd_params( params ) < 2)
		return -1;

	/* search connection type driver */
	for (i = 0; parport_drivers[i]; i++)
		if (strcasecmp( params[1], parport_drivers[i]->type ) == 0)
			break;
	if (!parport_drivers[i]) {
		printf( _("Unknown connection type: %s\n"), params[1] );
		return 1;
	}

	if (bus) {
		bus_free( bus );
		bus = NULL;
	}
	chain_disconnect( chain );
	chain->cable = parport_drivers[i]->connect( (const char **) &params[2], cmd_params( params ) - 2 );
	if (!chain->cable) {
		printf( _("Error: Cable connection failed!\n") );
		return 1;
	}

	if (cable_init( chain->cable )) {
		printf( _("Error: Cable initialization failed!\n") );
		chain_disconnect( chain );
		return 1;
	}
	chain_set_trst( chain, 0 );
	chain_set_trst( chain, 1 );
	tap_reset( chain );

	return 1;
}

static void
cmd_cable_help( void )
{
	int i;

	printf( _(
		"Usage: %s PORTADDR CABLE\n"
		"Usage: %s DEV CABLE\n"
		"Usage: %s VID:PID:S/N CABLE\n"
		"Usage: %s VID:PID:S/N CABLE\n"
		"Select JTAG cable connected to parallel port.\n"
		"\n"
		"PORTADDR   parallel port address (e.g. 0x378)\n"
		"CABLE      cable type\n"
		"DEV        ppdev device (e.g. /dev/parport0)\n"
		"VID        empty or USB vendor ID, hex (e.g. 09FB)\n"
		"PID        empty or USB product ID, hex (e.g. 6001)\n"
		"S/N        empty or USB product serial number, ASCII\n"
		"\n"
		"List of supported cables:\n"
		"%-13s No cable connected\n"
	), "cable parallel", "cable ppdev", "cable ftdi", "cable xpcu", "none" );

	for (i = 0; cable_drivers[i]; i++)
		printf( _("%-13s %s\n"), cable_drivers[i]->name, _(cable_drivers[i]->description) );
}

cmd_t cmd_cable = {
	"cable",
	N_("select JTAG cable"),
	cmd_cable_help,
	cmd_cable_run
};
