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
 * Written by Matan Ziv-Av <matan@svgalib.org>, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdint.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_peek_run( char *params[] )
{
	uint32_t adr, val;

	if (cmd_params( params ) != 2)
		return -1;

	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}

	if (cmd_get_number( params[1], &adr ))
		return -1;

	bus_prepare( bus );
	val = bus_read( bus, adr );

	printf( _("bus_read(0x%08x) = 0x%08X (%i)\n"), adr, val, val );

	return 1;
}

static void
cmd_peek_help( void )
{
	printf( _(
		"Usage: %s ADDR\n"
		"Read a single word (bus width size).\n"
		"\n"
		"ADDR       address to read from\n"
		"\n"
		"ADDR could be in decimal or hexadecimal (prefixed with 0x) form.\n"
		"\n"
	), "peek" );
}

cmd_t cmd_peek = {
	"peek",
	N_("read a single word"),
	cmd_peek_help,
	cmd_peek_run
};

static int
cmd_poke_run( char *params[] )
{
	uint32_t adr, val;

	if (cmd_params( params ) != 3)
		return -1;

	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}

	if (cmd_get_number( params[1], &adr ) || cmd_get_number( params[2], &val ))
		return -1;

	bus_prepare( bus );
	bus_write( bus, adr, val );

	return 1;
}

static void
cmd_poke_help( void )
{
	printf( _(
		"Usage: %s ADDR VAL\n"
		"Write a single word (bus width size).\n"
		"\n"
		"ADDR       address to write\n"
		"VAL        value to write\n"
		"\n"
		"ADDR and VAL could be in decimal or hexadecimal (prefixed with 0x) form.\n"
		"\n"
	), "poke" );
}

cmd_t cmd_poke = {
	"poke",
	N_("write a single word"),
	cmd_poke_help,
	cmd_poke_run
};
