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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gettext.h"
#define	_(s)		gettext(s)
#define	N_(s)		gettext_noop(s)
#define	P_(s,p,n)	ngettext(s,p,n)

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
extern cmd_t cmd_print;
extern cmd_t cmd_instruction;
extern cmd_t cmd_shift;
extern cmd_t cmd_dr;
extern cmd_t cmd_get;
extern cmd_t cmd_set;
extern cmd_t cmd_endian;
extern cmd_t cmd_readmem;
extern cmd_t cmd_detectflash;
extern cmd_t cmd_flashmem;
extern cmd_t cmd_script;

const cmd_t *cmds[] = {
	&cmd_quit,
	&cmd_help,
	&cmd_frequency,
	&cmd_cable,
	&cmd_discovery,
	&cmd_detect,
	&cmd_print,
	&cmd_instruction,
	&cmd_shift,
	&cmd_dr,
	&cmd_get,
	&cmd_set,
	&cmd_endian,
	&cmd_readmem,
	&cmd_detectflash,
	&cmd_flashmem,
	&cmd_script,
	NULL			/* last must be NULL */
};

int
cmd_run( char *params[] )
{
	int i;

	if (!params[0])
		return 1;

	for (i = 0; cmds[i]; i++)
		if (strcmp( cmds[i]->name, params[0] ) == 0) {
			int r = cmds[i]->run( params );
			if (r < 0)
				printf( _("%s: syntax error!\n"), params[0] );
			return r;
		}

	printf( _("%s: unknown command\n"), params[0] );
	return 1;
}

int
cmd_params( char *params[] )
{
	int i = 0;

	while (params[i])
		i++;

	return i;
}

int
cmd_get_number( char *s, unsigned int *i )
{
	int n;
	int r;
	int l;

	if (!s || !i)
		return -1;

	l = strlen( s );
	r = sscanf( s, "0x%x%n", i, &n);
	if (r == 1 && n == l)
		return 0;
	r = sscanf( s, "%u%n", i, &n );
	if (r == 1 && n == l)
		return 0;

	return -1;
}

int
cmd_test_cable( void )
{
	if (chain->cable)
		return 1;

	printf( _("Error: Cable not configured. Please use '%s' command first!\n"), "cable" );
	return 0;
}
