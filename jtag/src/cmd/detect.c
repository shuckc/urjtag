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

#include <config.h>

#include <stdio.h>
#include <string.h>

#include "jtag.h"
#include "chain.h"
#include "bus.h"

#include "cmd.h"

static int
cmd_detect_run( char *params[] )
{
	if (cmd_params( params ) != 1)
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (bus) {
		bus->free( bus );
		bus = NULL;
	}
	parts_free( chain->parts );
	chain->parts = detect_parts( chain, JTAG_DATA_DIR );
	if (!chain->parts->len) {
		parts_free( chain->parts );
		chain->parts = NULL;
		return 1;
	}
	parts_set_instruction( chain->parts, "SAMPLE/PRELOAD" );
	chain_shift_instructions( chain );
	chain_shift_data_registers( chain, 1 );
	parts_set_instruction( chain->parts, "BYPASS" );
	chain_shift_instructions( chain );
	if (strcmp( chain->parts->parts[0]->part, "SA1110" ) == 0)
		bus = new_sa1110_bus( chain, 0 );
	if (strcmp( chain->parts->parts[0]->part, "PXA250" ) == 0)
		bus = new_pxa250_bus( chain, 0 );
	if (strcmp( chain->parts->parts[0]->part, "IXP425" ) == 0)
		bus = new_ixp425_bus( chain, 0 );
	if (strcmp( chain->parts->parts[0]->part, "SH7727" ) == 0)
		bus = new_sh7727_bus( chain, 0 );

	return 1;
}

static void
cmd_detect_help( void )
{
	printf( _(
		"Usage: %s\n"
		"Detect parts on the JTAG chain.\n"
		"\n"
		"Output from this command is a list of the detected parts.\n"
		"If no parts are detected other commands may not work properly.\n"
	), "detect" );
}

cmd_t cmd_detect = {
	"detect",
	N_("detect parts on the JTAG chain"),
	cmd_detect_help,
	cmd_detect_run
};
