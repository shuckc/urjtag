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

#include <stdlib.h>
#include <string.h>

#include "chain.h"
#include "state.h"
#include "tap.h"

chain_t *
chain_alloc( void )
{
	chain_t *chain = malloc( sizeof (chain_t) );
	if (!chain)
		return NULL;

	chain->cable = NULL;
	chain->parts = NULL;
	chain->active_part = 0;
	tap_state_init( chain );

	return chain;
}

void
chain_free( chain_t *chain )
{
	if (!chain)
		return;

	chain_disconnect( chain );

	parts_free( chain->parts );
	free( chain );
}

void
chain_disconnect( chain_t *chain )
{
	if (!chain->cable)
		return;

	tap_state_done( chain );
	cable_done( chain->cable );
	cable_free( chain->cable );
	chain->cable = NULL;
}

void
chain_clock( chain_t *chain, int tms, int tdi, int n )
{
	int i;

	if (!chain || !chain->cable)
		return;

	cable_clock( chain->cable, tms, tdi, n );

        for (i = 0; i < n; i++)
		tap_state_clock( chain, tms );
}

void
chain_defer_clock( chain_t *chain, int tms, int tdi, int n )
{
	int i;

	if (!chain || !chain->cable)
		return;

	cable_defer_clock( chain->cable, tms, tdi, n );

        for (i = 0; i < n; i++)
		tap_state_clock( chain, tms );
}

int
chain_set_trst( chain_t *chain, int trst )
{
	int old_trst = cable_get_trst( chain->cable );
	trst = cable_set_trst( chain->cable, trst );
	tap_state_set_trst( chain, old_trst, trst );
	return trst;
}

int
chain_get_trst( chain_t *chain )
{
	return cable_get_trst( chain->cable );
}

void
chain_shift_instructions_mode( chain_t *chain, int capture_output, int capture, int exit )
{
	int i;
	parts_t *ps;

	if (!chain || !chain->parts)
		return;

	ps = chain->parts;

	for (i = 0; i < ps->len; i++) {
		if (ps->parts[i]->active_instruction == NULL) {
			printf( _("%s(%d) Part %d without active instruction\n"), __FILE__, __LINE__, i );
			return;
		}
	}

	if (capture)
		tap_capture_ir( chain );

	/* new implementation: split into defer + retrieve part
	   shift the data register of each part in the chain one by one */

	for (i = 0; i < ps->len; i++) {
		tap_defer_shift_register( chain, ps->parts[i]->active_instruction->value,
		                          capture_output ? ps->parts[i]->active_instruction->out : NULL,
		                          (i + 1) == ps->len ? exit : EXITMODE_SHIFT );
	}

	if(capture_output)
	{
		for (i = 0; i < ps->len; i++) {
			tap_shift_register_output( chain, ps->parts[i]->active_instruction->value,
			                           ps->parts[i]->active_instruction->out,
			                           (i + 1) == ps->len ? exit : EXITMODE_SHIFT );
		}
	}
	else
	{
		/* give the cable driver a chance to flush if it's considered useful */
		cable_flush( chain->cable, TO_OUTPUT );
	}
}

void
chain_shift_instructions( chain_t *chain )
{
	chain_shift_instructions_mode( chain, 0, 1, EXITMODE_IDLE );
}

void
chain_shift_data_registers_mode( chain_t *chain, int capture_output, int capture, int exit )
{
	int i;
	parts_t *ps;

	if (!chain || !chain->parts)
		return;

	ps = chain->parts;

	for (i = 0; i < ps->len; i++) {
		if (ps->parts[i]->active_instruction == NULL) {
			printf( _("%s(%d) Part %d without active instruction\n"), __FILE__, __LINE__, i );
			return;
		}
		if (ps->parts[i]->active_instruction->data_register == NULL) {
			printf( _("%s(%d) Part %d without data register\n"), __FILE__, __LINE__, i );
			return;
		}
	}

	if (capture)
		tap_capture_dr( chain );

	/* new implementation: split into defer + retrieve part
	   shift the data register of each part in the chain one by one */

	for (i = 0; i < ps->len; i++) {
		tap_defer_shift_register( chain, ps->parts[i]->active_instruction->data_register->in,
				capture_output ? ps->parts[i]->active_instruction->data_register->out : NULL,
				(i + 1) == ps->len ? exit : EXITMODE_SHIFT );
	}

	if(capture_output)
	{
		for (i = 0; i < ps->len; i++) {
			tap_shift_register_output( chain, ps->parts[i]->active_instruction->data_register->in,
				ps->parts[i]->active_instruction->data_register->out,
				(i + 1) == ps->len ? exit : EXITMODE_SHIFT );
		}
	}
	else
	{
		/* give the cable driver a chance to flush if it's considered useful */
		cable_flush( chain->cable, TO_OUTPUT );
	}
}

void
chain_shift_data_registers( chain_t *chain, int capture_output )
{
	chain_shift_data_registers_mode( chain, capture_output, 1, EXITMODE_IDLE );
}

void
chain_flush( chain_t *chain )
{
	if( chain->cable != NULL)
		cable_flush( chain->cable, COMPLETELY );
}

