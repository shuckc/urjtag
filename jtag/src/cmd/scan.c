/*
 * $Id: scan.c 733 2007-11-07 22:21:33Z arniml $
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
#include <stdlib.h>
#include <string.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_scan_run( chain_t *chain, char *params[] )
{
	part_t *part;
    data_register *bsr;
	tap_register *obsr;
	int i;

	if ((i = cmd_params( params )) < 1)
		return -1;

	if (!cmd_test_cable( chain ))
		return 1;

	if (!chain->parts) {
		printf( _("Run \"detect\" first.\n") );
		return 1;
	}

	if (chain->active_part >= chain->parts->len) {
		printf( _("%s: no active part\n"), "scan" );
		return 1;
	}

	part = chain->parts->parts[chain->active_part];

	/* search for Boundary Scan Register */
	bsr = part_find_data_register( part, "BSR" );
	if (!bsr) {
		printf( _("%s(%s:%d) Boundary Scan Register (BSR) not found\n"), __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	}

	if(part_find_instruction( part, "SAMPLE"))
	{
		part_set_instruction( part, "SAMPLE");
	}
	else if(part_find_instruction( part, "SAMPLE/PRELOAD"))
	{
		part_set_instruction( part, "SAMPLE/PRELOAD");
	}
	else
	{
		printf( _("%s(%s:%d) Part can't SAMPLE\n"), __FUNCTION__, __FILE__, __LINE__ );
		return 1;
	};

	chain_shift_instructions( chain );

	obsr = register_alloc( bsr->out->len );

	if(!obsr)
	{
		printf( _("Out of memory\n") );
		return 1;
	}

	{
		signal_t *s;

		register_init( obsr, register_get_string( bsr->out )); // copy

		chain_shift_data_registers( chain, 1 );

		for(s=part->signals; s; s=s->next)
		{
			if(s->input != NULL)
			{
				int old = obsr->data[s->input->bit];
				int new = bsr->out->data[s->input->bit];
				if( old != new ) 
				{
					salias_t *a;
					printf("%s", s->name);
					for(a = part->saliases; a; a=a->next)
					{
						if(a->signal == s) printf(",%s", a->name);
					}
					printf( _(": %d > %d\n"), old, new);
				}
			}
		}
	}

	register_free( obsr );

	return 1;
}

static void
cmd_scan_help( void )
{
	printf( _(
		"Usage: %s [SIGNAL]* \n"
		"Read BSR and show changes since last scan.\n"
	), "scan" );
}

cmd_t cmd_scan = {
	"scan",
	N_("read BSR and show changes since last scan"),
	cmd_scan_help,
	cmd_scan_run
};
