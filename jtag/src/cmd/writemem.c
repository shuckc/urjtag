/*
 * $Id$
 *
 * Written by Kent Palmkvist <kentp@isy.liu.se>, 2005
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
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdint.h>

#include "jtag.h"

#include "cmd.h"

static int
cmd_writemem_run( chain_t *chain, char *params[] )
{
	uint32_t adr;
	uint32_t len;
	FILE *f;

	if (cmd_params( params ) != 4)
		return -1;

	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}

	if (cmd_get_number( params[1], &adr) || cmd_get_number( params[2], &len))
		return -1;

	f = fopen( params[3], "r" );
	if (!f) {
		printf( _("Unable to open file `%s'!\n"), params[3] );
		return 1;
	}
	writemem( bus, f, adr, len );
	fclose( f );

	return 1;
}

static void
cmd_writemem_help( void )
{
	printf( _(
		"Usage: %s ADDR LEN FILENAME\n"
		"Write to device memory starting at ADDR the FILENAME file.\n"
		"\n"
		"ADDR       start address of the written memory area\n"
		"LEN        written memory length\n"
		"FILENAME   name of the input file\n"
		"\n"
		"ADDR and LEN could be in decimal or hexadecimal (prefixed with 0x) form.\n"
		"NOTE: This is NOT useful for FLASH programming!\n"
	), "writemem" );
}

cmd_t cmd_writemem = {
	"writemem",
	N_("write content of file to the memory"),
	cmd_writemem_help,
	cmd_writemem_run
};
