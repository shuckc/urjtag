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

#include <stdio.h>

#include "register.h"
#include "tap.h"
#include "ctrl.h"
#include "state.h"

void
tap_reset( void )
{
	tap_clock( 1, 0 );
	tap_clock( 1, 0 );
	tap_clock( 1, 0 );
	tap_clock( 1, 0 );
	tap_clock( 1, 0 );			/* Test-Logic-Reset */

	tap_clock( 0, 0 );			/* Run-Test/Idle */
}

void
tap_shift_register( const tap_register *in, tap_register *out, int exit )
{
	int i;

	if (!(tap_state() & TAPSTAT_SHIFT))
		printf( "tap_shift_register: Invalid state: %2X\n", tap_state() );

	/* Capture-DR, Capture-IR, Shift-DR, Shift-IR, Exit2-DR or Exit2-IR state */
	if (tap_state() & TAPSTAT_CAPTURE)
		tap_clock( 0, 0 );		/* save last TDO bit :-) */
	for (i = 0; i < in->len; i++) {
		if (out && (i < out->len))
			out->data[i] = tap_get_tdo();
		tap_clock( (exit && ((i + 1) == in->len)) ? 1 : 0, in->data[i] );	/* Shift (& Exit1) */
	}
	/* Shift-DR, Shift-IR, Exit1-DR or Exit1-IR state */
	if (exit)
		tap_clock( 1, 0 );		/* Update-DR or Update-IR */
}

void
tap_capture_dr( void )
{
	if ((tap_state() & (TAPSTAT_RESET | TAPSTAT_IDLE)) != TAPSTAT_IDLE)
		printf( "tap_capture_dr: Invalid state: %2X\n", tap_state() );

	/* Run-Test/Idle or Update-DR or Update-IR state */
	tap_clock( 1, 0 );			/* Select-DR-Scan */
	tap_clock( 0, 0 );			/* Capture-DR */
}

void
tap_capture_ir( void )
{
	if ((tap_state() & (TAPSTAT_RESET | TAPSTAT_IDLE)) != TAPSTAT_IDLE)
		printf( "tap_capture_ir: Invalid state: %2X\n", tap_state() );

	/* Run-Test/Idle or Update-DR or Update-IR state */
	tap_clock( 1, 0 );			/* Select-DR-Scan */
	tap_clock( 1, 0 );			/* Select-IR-Scan */
	tap_clock( 0, 0 );			/* Capture-IR */
}

void
write_command( const tap_register *c, tap_register *cout, int len )
{
	tap_capture_ir();
	tap_shift_register( c, cout, 1 );
}
