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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gettext.h"
#define	_(s)		gettext(s)
#define	N_(s)		gettext_noop(s)
#define	P_(s,p,n)	ngettext(s,p,n)

#include <stdlib.h>

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
	tap_state_init( chain );

	return chain;
}

void
chain_free( chain_t *chain )
{
	if (!chain)
		return;

	if (chain->cable) {
		chain->cable->set_trst( 0 );
		chain->cable->set_trst( 1 );
		tap_reset( chain );
		chain->cable->done();
	}
	tap_state_done( chain );
	/* cable_free( chain->cable ); */
	parts_free( chain->parts );
	free( chain );
}

int
chain_connect( chain_t *chain, cable_t *cable, unsigned int port )
{
	if (chain->cable) {
		tap_state_done( chain );
		chain->cable->done();
		chain->cable = NULL;
	}
	if (!cable || !cable->init( port ))
		return -1;

	chain->cable = cable;
	return 0;
}

void
chain_clock( chain_t *chain, int tms, int tdi )
{
	if (!chain || !chain->cable)
		return;

	chain->cable->clock( tms, tdi );
	tap_state_clock( chain, tms );
}

int
chain_set_trst( chain_t *chain, int trst )
{
	int old_trst = chain->cable->get_trst();
	trst = chain->cable->set_trst( trst );
	tap_state_set_trst( chain, old_trst, trst );
	return trst;
}

int
chain_get_trst( chain_t *chain )
{
	return chain->cable->get_trst();
}

void
chain_shift_instructions( chain_t *chain )
{
	int i;
	parts_t *ps;

	if (!chain || !chain->parts)
		return;

	ps = chain->parts;

	tap_capture_ir( chain );

	for (i = 0; i < ps->len; i++) {
		if (!ps->parts[i]->active_instruction) {
			printf( _("%s(%d) Part without active instruction\n"), __FILE__, __LINE__ );
			continue;
		}
		tap_shift_register( chain, ps->parts[i]->active_instruction->value, NULL, (i + 1) == ps->len );
	}
}

void
chain_shift_data_registers( chain_t *chain )
{
	int i;
	parts_t *ps;

	if (!chain || !chain->parts)
		return;

	ps = chain->parts;

	tap_capture_dr( chain );

	for (i = 0; i < ps->len; i++) {
		if (!ps->parts[i]->active_instruction) {
			printf( _("%s(%d) Part without active instruction\n"), __FILE__, __LINE__ );
			continue;
		}
		tap_shift_register( chain, ps->parts[i]->active_instruction->data_register->in,
				ps->parts[i]->active_instruction->data_register->out, (i + 1) == ps->len );
	}
}
