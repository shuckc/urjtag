/*
 * $Id$
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
 * Written by Ville Voipio <ville.voipio@iki.fi>, 2008.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include "jtag.h"
#include "chain.h"
#include "bus.h"

#include "cmd.h"

static int
cmd_addpart_run( chain_t *chain, char *params[] )
{
	unsigned int len;
	
	if (cmd_params( params ) != 2)
		return -1;

	if (cmd_get_number( params[1], &len ))
		return -1;

	if (!cmd_test_cable( chain ))
		return 1;

	manual_add( chain, len );

	if (chain->parts == NULL)
		return 1;

	if (chain->parts->len == 0) {
		parts_free( chain->parts );
		chain->parts = NULL;
	}

	parts_set_instruction(chain->parts, "BYPASS");
	chain_shift_instructions(chain);

	return 1;
}


static void
cmd_addpart_help( void )
{
	printf( _(
		"Usage: %s IRLENGTH\n"
		"Manually add a part to the end of the chain.\n"
		"\n"
		"IRLENGTH           instruction register length\n"
	), "addpart" );
}


cmd_t cmd_addpart = {
	"addpart",
	N_("manually adds parts on the JTAG chain"),
	cmd_addpart_help,
	cmd_addpart_run
};
