/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

void
help( const char *cmd )
{
	if (!cmd) {
		printf( "Command list:\n\n" );
		printf( "quit\t\texit from jtag\n" );
		printf( "help\t\tdisplay this help\n" );
		printf( "detect\t\tdetect parts on the JTAG chain\n" );
		printf( "print\t\tdisplay JTAG chain list/status\n" );
		printf( "instruction\tchange active instruction for a part\n" );
		printf( "shift\t\tshift data/instruction register through JTAG chain\n" );
		printf( "dr\t\tdisplay active data register for a part\n" );
		printf( "detectflash\tdetect parameters of flash chip attached to a part\n" );
		printf( "readmem\t\tread content of the memory and write it to file\n" );
		printf( "flashmem\tburn flash memory with data from a file\n" );
		printf( "set\t\tTODO\n" );
		printf( "\nType \"help <command>\" for details about particular command.\n" );
	} else {
		printf( "Not implemented. Sorry.\n" );
	}
}
