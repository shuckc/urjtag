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

#include "sysdep.h"

#include <stdio.h>

#include "register.h"
#include "tap.h"
#include "state.h"
#include "chain.h"

void
tap_reset( chain_t *chain )
{
	tap_state_reset( chain );

	chain_defer_clock( chain, 1, 0, 5 );				/* Test-Logic-Reset */
	chain_defer_clock( chain, 0, 0, 1 );				/* Run-Test/Idle */

	parts_set_instruction( chain->parts, "BYPASS" );
}

void
tap_defer_shift_register( chain_t *chain, const tap_register *in, tap_register *out, int exit )
{
	int i;

	if (!(tap_state( chain ) & TAPSTAT_SHIFT))
		printf( _("%s: Invalid state: %2X\n"), "tap_shift_register", tap_state( chain ) );

	/* Capture-DR, Capture-IR, Shift-DR, Shift-IR, Exit2-DR or Exit2-IR state */
	if (tap_state( chain ) & TAPSTAT_CAPTURE)
		chain_defer_clock( chain, 0, 0, 1 );	/* save last TDO bit :-) */

	i = in->len;
	if(exit) i--;
	if(out && out->len < i) i = out->len;

	if(out)
		cable_defer_transfer( chain->cable, i, in->data, out->data );
	else
		cable_defer_transfer( chain->cable, i, in->data, NULL );

	for (; i < in->len; i++) {
		if (out != NULL && (i < out->len))
			out->data[i] = cable_defer_get_tdo( chain->cable );
		chain_defer_clock( chain, (exit != EXITMODE_SHIFT && ((i + 1) == in->len)) ? 1 : 0, in->data[i], 1 );	/* Shift (& Exit1) */
	}

	/* Shift-DR, Shift-IR, Exit1-DR or Exit1-IR state */
	if (exit == EXITMODE_IDLE) {
		chain_defer_clock( chain, 1, 0, 1 );	/* Update-DR or Update-IR */
		chain_defer_clock( chain, 0, 0, 1 );	/* Run-Test/Idle */
	}
}

void
tap_shift_register_output( chain_t *chain, const tap_register *in, tap_register *out, int exit )
{
	if(out != NULL)
	{
		int j;

		j = in->len;
		if(exit) j--;
		if(out && out->len < j) j = out->len;

		/* Asking for the result of the cable transfer
         * actually flushes the queue */

		(void)cable_transfer_late( chain->cable, out->data );
		for (; j < in->len && j < out->len; j++) 
				out->data[j] = cable_get_tdo_late( chain->cable );
	}
}

void
tap_shift_register( chain_t *chain, const tap_register *in, tap_register *out, int exit )
{
	tap_defer_shift_register( chain, in, out, exit );
	tap_shift_register_output( chain, in, out, exit );
}

void
tap_capture_dr( chain_t *chain )
{
	if ((tap_state( chain ) & (TAPSTAT_RESET | TAPSTAT_IDLE)) != TAPSTAT_IDLE)
		printf( _("%s: Invalid state: %2X\n"), "tap_capture_dr", tap_state( chain ) );

	/* Run-Test/Idle or Update-DR or Update-IR state */
	chain_defer_clock( chain, 1, 0, 1 );		/* Select-DR-Scan */
	chain_defer_clock( chain, 0, 0, 1 );		/* Capture-DR */
}

void
tap_capture_ir( chain_t *chain )
{
	if ((tap_state( chain ) & (TAPSTAT_RESET | TAPSTAT_IDLE)) != TAPSTAT_IDLE)
		printf( _("%s: Invalid state: %2X\n"), "tap_capture_ir", tap_state( chain ) );

	/* Run-Test/Idle or Update-DR or Update-IR state */
	chain_defer_clock( chain, 1, 0, 2 );		/* Select-DR-Scan, then Select-IR-Scan */
	chain_defer_clock( chain, 0, 0, 1 );		/* Capture-IR */
}
