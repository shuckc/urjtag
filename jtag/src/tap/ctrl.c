/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#include <sys/io.h>

#include "ctrl.h"
#include "state.h"

#define	TCK	0
#define	TDI	1
#define	TMS	2
#define	TRST	4

#define	TDO	7

static unsigned short int port = 0x378;

void
tap_init( void )
{
	tap_state_init();
	printf( "Initilizing parallel TAP on port 0x%x\n", port );
	if (ioperm( port, 2, 1 )) {
		printf( "Error: Initialization failed!\n" );
		return;
	}
	tap_state_set_trst( (inb( port ) >> TRST) & 1 );
}

void
tap_done( void )
{
	ioperm( port, 2, 0 );

	tap_state_done();
}

void
tap_clock( int tms, int tdi )
{
	int trst = tap_state_get_trst();

	tms &= 1;
	tdi &= 1;

	outb( (trst << TRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI), port );
	outb( (trst << TRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI), port );

	tap_state_clock( tms );
}

int
tap_get_tdo( void )
{
	outb( (tap_state_get_trst() << TRST) | (0 << TCK), port );
	return ((inb( port + 1 ) >> TDO) & 1) ^ 1;
}

void
tap_set_trst( int new_trst )
{
	tap_state_set_trst( new_trst );
	outb( (new_trst & 1) << TRST, port );
}
