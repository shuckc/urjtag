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

#include "jtag.h"

#include "cmd.h"

static int
cmd_detectflash_run( char *params[] )
{
	if (cmd_params( params ) != 1)
		return -1;

	if (!cmd_test_cable())
		return 1;

	if (!bus) {
		printf( _("Error: Bus driver missing.\n") );
		return 1;
	}

	detectflash( bus );

	return 1;
}

static void
cmd_detectflash_help( void )
{
	printf( _(
		"Usage: %s\n"
		"Detect flash memory type connected to part.\n"
		"\n"
		"Only detects flash connected to part 0. Part 0 must support\n"
		"bus operations.\n"
	), "detectflash" );
}

cmd_t cmd_detectflash = {
	"detectflash",
	N_("detect parameters of flash chips attached to a part"),
	cmd_detectflash_help,
	cmd_detectflash_run
};
