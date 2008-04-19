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
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#endif

#include "jtag.h"

#include "cmd.h"

extern cmd_t cmd_quit;
extern cmd_t cmd_help;
extern cmd_t cmd_frequency;
extern cmd_t cmd_cable;
extern cmd_t cmd_reset;
extern cmd_t cmd_discovery;
extern cmd_t cmd_idcode;
extern cmd_t cmd_detect;
extern cmd_t cmd_signal;
extern cmd_t cmd_scan;
extern const cmd_t cmd_salias;
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
extern cmd_t cmd_test;
extern cmd_t cmd_shell;
extern cmd_t cmd_set;
extern cmd_t cmd_endian;
extern cmd_t cmd_peek;
extern cmd_t cmd_poke;
extern cmd_t cmd_readmem;
extern cmd_t cmd_writemem;
extern cmd_t cmd_detectflash;
extern cmd_t cmd_flashmem;
extern cmd_t cmd_eraseflash;
extern cmd_t cmd_script;
extern cmd_t cmd_include;
#ifdef ENABLE_SVF
extern cmd_t cmd_svf;
#endif
#ifdef ENABLE_BSDL
extern cmd_t cmd_bsdl;
#endif
extern cmd_t cmd_debug;

const cmd_t *cmds[] = {
	&cmd_quit,
	&cmd_help,
	&cmd_frequency,
	&cmd_cable,
	&cmd_reset,
	&cmd_discovery,
	&cmd_idcode,
	&cmd_detect,
	&cmd_signal,
	&cmd_scan,
	&cmd_salias,
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
	&cmd_test,
	&cmd_shell,
	&cmd_set,
	&cmd_endian,
	&cmd_peek,
	&cmd_poke,
	&cmd_readmem,
	&cmd_writemem,
	&cmd_detectflash,
	&cmd_flashmem,
	&cmd_eraseflash,
	&cmd_script,
	&cmd_include,
#ifdef ENABLE_SVF
	&cmd_svf,
#endif
#ifdef ENABLE_BSDL
	&cmd_bsdl,
#endif
	&cmd_debug,
	NULL			/* last must be NULL */
};

#ifdef HAVE_LIBREADLINE
static char *
cmd_find_next( const char *text, int state )
{
	static size_t cmd_idx, len;

	if (!state) {
		cmd_idx = 0;
		len = strlen(text);
	}

	while (cmds[cmd_idx]) {
		char *name = cmds[cmd_idx++]->name;
		if (!strncmp(name, text, len))
			return strdup(name);
	}

	return NULL;
}

char **
cmd_completion( const char *text, int start, int end )
{
	char **ret = NULL;

	if (start == 0)
		ret = rl_completion_matches(text, cmd_find_next);

	return ret;
}
#endif

int
cmd_test_cable( chain_t *chain )
{
	if (chain->cable)
		return 1;

	printf( _("Error: Cable not configured. Please use '%s' command first!\n"), "cable" );
	return 0;
}

/* Remainder copied from libbrux/cmd/cmd.c */

int
cmd_run( chain_t *chain, char *params[] )
{
	int i, pidx;
	size_t len;

	if (!params[0])
		return 1;

	pidx = -1;
	len = strlen(params[0]);

	for (i = 0; cmds[i]; ++i) {
		if (strcasecmp( cmds[i]->name, params[0] ) == 0) {
			int r;
 run_cmd:
			r = cmds[i]->run( chain, params );
			if (r < 0)
				printf( _("%s: syntax error!\n"), params[0] );
			return r;
		} else if (strncasecmp( cmds[i]->name, params[0], len ) == 0) {
			if (pidx == -1)
				pidx = i;
			else
				pidx = -2;
		}
	}

	switch (pidx) {
		case -2: printf( _("%s: Ambiguous command\n"), params[0] ); break;
		case -1: printf( _("%s: unknown command\n"), params[0] ); break;
		default: i = pidx; goto run_cmd;
	}

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
	size_t l;

	if (!s || !i)
		return -1;

	l = strlen( s );

	n = -1;
	r = sscanf( s, "0x%x%n", i, &n);
	if (r == 1 && n == l)
		return 0;

	n = -1;
	r = sscanf( s, "%u%n", i, &n );
	if (r == 1 && n == l)
		return 0;

	return -1;
}
