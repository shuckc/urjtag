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

#include "jtag.h"

#include "cmd.h"

static int
cmd_bit_run( char *params[] )
{
	part_t *part;
	data_register *bsr;
	unsigned int bit;
	int type;
	int safe;
	unsigned int control;

	if ((cmd_params( params ) != 5) && (cmd_params( params ) != 8))
		return -1;


	if (!cmd_test_cable())
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	if (chain->active_part >= chain->parts->len) {
		printf( _("%s: no active part\n"), "bit" );
		return 1;
	}

	part = chain->parts->parts[chain->active_part];
	bsr = part_find_data_register( part, "BSR" );
	if (bsr == NULL) {
		printf( _("missing Boundary Scan Register (BSR)\n") );
		return 1;
	}

	/* bit number */
	if (cmd_get_number( params[1], &bit ))
		return -1;

	if (bit >= bsr->in->len) {
		printf( _("invalid boundary bit number\n") );
		return 1;
	}
	if (part->bsbits[bit] != NULL) {
		printf( _("duplicate bit declaration\n") );
		return 1;
	}

	/* bit type */
	if (strlen( params[2] ) != 1)
		return -1;
	switch (params[2][0]) {
		case 'I':
			type = BSBIT_INPUT;
			break;
		case 'O':
			type = BSBIT_OUTPUT;
			break;
		case 'B':
			type = BSBIT_BIDIR;
			break;
		case 'C':
			type = BSBIT_CONTROL;
			break;
		case 'X':
			type = BSBIT_INTERNAL;
			break;
		default:
			return -1;
	}

	/* default (safe) value */
	if (strlen( params[3] ) != 1)
		return -1;

	safe = (params[3][0] == '1') ? 1 : 0;
	bsr->in->data[bit] = safe;

	/* allocate bsbit */
	part->bsbits[bit] = bsbit_alloc( bit, params[4], type, part_find_signal( part, params[4] ), safe );
	if (part->bsbits[bit] == NULL) {
		printf( _("out of memory\n") );
		return 1;
	}

	/* test for control bit */
	if (cmd_params( params ) == 5)
		return 1;

	/* control bit number */
	if (cmd_get_number( params[5], &control ))
		return -1;
	if (control >= bsr->in->len) {
		printf( _("invalid control bit number\n") );
		return 1;
	}
	part->bsbits[bit]->control = control;

	/* control value */
	if (strlen( params[6] ) != 1)
		return -1;
	part->bsbits[bit]->control_value = (params[6][0] == '1') ? 1 : 0;

	/* control state */
	if ((strlen( params[7] ) != 1) || (params[7][0] != 'Z'))
		return -1;
	part->bsbits[bit]->control_state = BSBIT_STATE_Z;

	return 1;
}

static void
cmd_bit_help( void )
{
	printf( _(
		"Usage: %s NUMBER TYPE DEFAULT SIGNAL [CBIT CVAL CSTATE]\n"
		"Define new BSR (Boundary Scan Register) bit for SIGNAL, with\n"
		"DEFAULT value.\n"
		"\n"
		"NUMBER        Bit number in the BSR\n"
		"TYPE          Bit type, valid values are I, O, B, C, and X\n"
		"DEFAULT       Default (safe) bit value, valid values are 1, 0, ?\n"
		"SIGNAL        Associated signal name\n"
		"CBIT          Control bit number\n"
		"CVAL          Control value\n"
		"CSTATE        Control state, valid state is only Z\n"
	), "bit" );
}

cmd_t cmd_bit = {
	"bit",
	N_("define new BSR bit"),
	cmd_bit_help,
	cmd_bit_run
};
