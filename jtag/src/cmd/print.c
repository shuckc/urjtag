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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
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

#include "part.h"
#include "jtag.h"

#include "cmd.h"

static int
cmd_print_run( char *params[] )
{
	if (cmd_params( params ) != 1)
		return -1;

	if (!cmd_test_cable())
		return 1;

	parts_print( chain->parts, 1 );

	return 1;
}

static void
cmd_print_help( void )
{
	printf( _(
		"Usage: %s\n"
		"Display JTAG chain status.\n"
		"\n"
		"Display list of the parts connected to the JTAG chain including\n"
		"part number and current (active) instruction and data register.\n"
	), "print" );
}

cmd_t cmd_print = {
	"print",
	N_("display JTAG chain list/status"),
	cmd_print_help,
	cmd_print_run
};
