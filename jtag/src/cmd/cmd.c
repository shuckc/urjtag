/*
 * $Id$
 *
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <string.h>

#include "jtag.h"

#include "cmd.h"

extern cmd_t cmd_quit;
extern cmd_t cmd_help;
extern cmd_t cmd_frequency;
extern cmd_t cmd_cable;
extern cmd_t cmd_discovery;
extern cmd_t cmd_detect;
extern cmd_t cmd_signal;
extern cmd_t cmd_bit;
extern cmd_t cmd_register;
extern const cmd_t cmd_initbus;
extern cmd_t cmd_print;
extern cmd_t cmd_part;
extern cmd_t cmd_bus;
extern cmd_t cmd_instruction;
extern cmd_t cmd_shift;
extern cmd_t cmd_dr;
extern cmd_t cmd_get;
extern cmd_t cmd_set;
extern cmd_t cmd_endian;
extern cmd_t cmd_peek;
extern cmd_t cmd_poke;
extern cmd_t cmd_readmem;
extern cmd_t cmd_detectflash;
extern cmd_t cmd_flashmem;
extern cmd_t cmd_eraseflash;
extern cmd_t cmd_script;
extern cmd_t cmd_include;

const cmd_t *cmds[] = {
	&cmd_quit,
	&cmd_help,
	&cmd_frequency,
	&cmd_cable,
	&cmd_discovery,
	&cmd_detect,
	&cmd_signal,
	&cmd_bit,
	&cmd_register,
	&cmd_initbus,
	&cmd_print,
	&cmd_part,
	&cmd_bus,
	&cmd_instruction,
	&cmd_shift,
	&cmd_dr,
	&cmd_get,
	&cmd_set,
	&cmd_endian,
	&cmd_peek,
	&cmd_poke,
	&cmd_readmem,
	&cmd_detectflash,
	&cmd_flashmem,
	&cmd_eraseflash,
	&cmd_script,
	&cmd_include,
	NULL			/* last must be NULL */
};

int
cmd_test_cable( void )
{
	if (chain->cable)
		return 1;

	printf( _("Error: Cable not configured. Please use '%s' command first!\n"), "cable" );
	return 0;
}
