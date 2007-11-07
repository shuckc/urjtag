/*
 * $Id$
 *
 * Copyright (C) 2003 TF
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
 * Written by Thomas Froehlich <t.froehlich@gmx.at>, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "jtag.h"
#include "flash.h"

#include "cmd.h"

static int
cmd_eraseflash_run( char *params[] )
{
	uint32_t adr = 0;
	unsigned int number = 0;

	if (cmd_params( params ) != 3)
		return -1;
	if (!cmd_test_cable())
		return 1;
	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}
	if (cmd_get_number( params[1], &adr ))
		return -1;
	if (cmd_get_number( params[2], &number ))
		return -1;
	flasherase( bus, adr, number );

	return 1;
}

static void
cmd_eraseflash_help( void )
{
	int i;

	printf( _(
		"Usage: %s ADDR BLOCKS\n"
		"Erase flash memory from ADDR.\n"
		"\n"
		"ADDR       target addres for erasing block\n"
		"BLOCKS     number of blocks to erase\n"
		"\n"
		"ADDR and BLOCKS could be in decimal or hexadecimal (prefixed with 0x) form.\n"
		"\n"
		"Supported Flash Memories:\n"
	), "eraseflash" );

	for (i = 0; flash_drivers[i]; i++)
		printf( _("%s\n     %s\n"), _(flash_drivers[i]->name), _(flash_drivers[i]->description) );
}

cmd_t cmd_eraseflash = {
	"eraseflash",
	N_("erase flash memory by number of blocks"),
	cmd_eraseflash_help,
	cmd_eraseflash_run
};
