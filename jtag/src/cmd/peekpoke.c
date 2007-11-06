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
	int	pars, j = 1, bw;
	bus_area_t area;
	
	/* bus_t * bus = part_get_active_bus(chain); */

	if ( (pars = cmd_params( params )) < 2)
		return -1;

	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}
	do {
	if (cmd_get_number( params[j], &adr ))
		return -1;
	
	bus_prepare( bus );
	bus_area( bus, adr, &area );
	val = bus_read( bus, adr );

	    switch ( area.width ) 
	    {
	    case 8:
		    val &= 0xff;
		    printf( _("bus_read(0x%08x) = 0x%02X (%i)\n"), adr, val, val );
		    break;
	    case 16:
		    val &= 0xffff;
		    printf( _("bus_read(0x%08x) = 0x%04X (%i)\n"), adr, val, val );
		    break;
	    default:
	    	    printf( _("bus_read(0x%08x) = 0x%08X (%i)\n"), adr, val, val );
	    }	     
	} while (++j != pars);

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
	bus_area_t area;
	/*bus_t * bus = part_get_active_bus(chain);*/
	int	k = 1, pars = cmd_params( params );

	if ( pars < 3 || !(pars & 1))
		return -1;

	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}

	
	bus_prepare( bus );	
	
	while ( k < pars) {
	    if (cmd_get_number( params[k], &adr ) || cmd_get_number( params[k+1], &val ))
			return -1;	
	    bus_area( bus, adr, &area );
	    bus_write( bus, adr, val );
	    k += 2;
	}

	return 1;
}

static void
cmd_poke_help( void )
{
	printf( _(
		"Usage: %s ADDR VAL [ADDR VAL] ... \n"
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
