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
chain_shift_instructions_mode( chain_t *chain, int capture, int exit )
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
	for (i = 0; i < ps->len; i++)
		tap_shift_register( chain, ps->parts[i]->active_instruction->value, NULL,
                                    (i + 1) == ps->len ? exit : EXITMODE_SHIFT );
}

void
chain_shift_instructions( chain_t *chain )
{
	chain_shift_instructions_mode( chain, 1, EXITMODE_IDLE );
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

#if 1
	/* old implementation:
	   shift the data register of each part in the chain one by one */
	for (i = 0; i < ps->len; i++) {
		puts("tap_shift_register");
		tap_shift_register( chain, ps->parts[i]->active_instruction->data_register->in,
				capture_output ? ps->parts[i]->active_instruction->data_register->out : NULL,
				(i + 1) == ps->len ? exit : EXITMODE_SHIFT );
	}
#elif 1
	{
		/* new implementation:
		   combine the data registers of all parts in the chain into one temporary register,
		   shift once,
		   copy back the "out" data to the data registers of all parts */
		int total_length = 0;
		data_register *temp_reg;
		char *part_str, *temp_str;

		/* determine total length of all data registers for temporary register */
		for (i = 0; i < ps->len; i++)
			total_length += ps->parts[i]->active_instruction->data_register->in->len;
		temp_reg = data_register_alloc( "TEMP", total_length );

		/* combine "in" data of all registers */
		temp_str = register_get_string( temp_reg->in );
		temp_str[0] = '\0';
		for (i = ps->len - 1; i >= 0; i--) {
			part_str = register_get_string( ps->parts[i]->active_instruction->data_register->in );
			strcat( temp_str, part_str );
		}
		register_init( temp_reg->in, temp_str );

		/* shift once */
		tap_shift_register( chain, temp_reg->in, capture_output ? temp_reg->out : NULL, exit );
		if (capture_output) {
			char *idx_string;

			/* copy back the "out" data */
			temp_str = register_get_string( temp_reg->out );
			for (i = ps->len - 1, idx_string = temp_str; i >= 0; i--) {
				part_str = register_get_string( ps->parts[i]->active_instruction->data_register->out );
				strncpy( part_str, idx_string, ps->parts[i]->active_instruction->data_register->out->len );
				register_init( ps->parts[i]->active_instruction->data_register->out, part_str );
				idx_string += ps->parts[i]->active_instruction->data_register->out->len;
			}
		}

		data_register_free( temp_reg );
	}
#else
	/* new^2 implementation: split into defer + retrieve part
	   shift the data register of each part in the chain one by one */

	for (i = 0; i < ps->len; i++) {
		puts("tap_defer_shift_register");
		tap_defer_shift_register( chain, ps->parts[i]->active_instruction->data_register->in,
				capture_output ? ps->parts[i]->active_instruction->data_register->out : NULL,
				(i + 1) == ps->len ? exit : EXITMODE_SHIFT );
	}

	if(capture_output)
	{
		for (i = 0; i < ps->len; i++) {
			puts("tap_shift_register_output");
			tap_shift_register_output( chain, ps->parts[i]->active_instruction->data_register->in,
				ps->parts[i]->active_instruction->data_register->out,
				(i + 1) == ps->len ? exit : EXITMODE_SHIFT );
		}
	}
#endif
}

void
chain_shift_data_registers( chain_t *chain, int capture_output )
{
	chain_shift_data_registers_mode( chain, capture_output, 1, EXITMODE_IDLE );
}
