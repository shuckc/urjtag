/*
 * parse.c
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
 * Modified by Ajith Kumar P.C <ajithpc@kila.com>, 20/09/2006.
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chain.h"
#include "cmd.h"
#include "jtag.h"


static char *
get_token( char *buf )
{
	char *t = strtok( buf, " \f\n\r\t\v" );
	if (t && (*t == '#'))
		return NULL;
	return t;
}

int
jtag_parse_line( chain_t *chain, char *line )
{
	char *t;
	int l;
	int n;
	char **a;
	int r;

	if (!line || !(strlen( line ) > 0))
		return 1;

	t = get_token( line );
	if (!t)
		return 1;

	n = 0;
	l = 0;
	a = NULL;
	while (t) {
		if (n + 2 > l) {
			char **newa;
			l = (l < 16) ? 16 : (l * 2);
			newa = realloc( a, l * sizeof (char *) );
			if (!newa) {
				free( a );
				printf( _("Out of memory\n") );
				return 1;
			}
			a = newa;
		}
		a[n++] = t;
		a[n] = NULL;

		t = get_token( NULL );
	}

	r = cmd_run( chain, a );
	if(debug_mode & 1)printf("Return in jtag_parse_line r=%d\n",r);
	free( a );
	return r;
}

int
jtag_parse_stream( chain_t *chain, FILE *f )
{
	int go = 1;
	char *line = NULL;
	size_t n = 0;

	while (go && (getline( &line, &n, f ) != -1))
		if ((strlen(line) > 0) && (line[0] != '#'))
			go = jtag_parse_line(chain, line);

	free(line);

	return go;
}

int
jtag_parse_file( chain_t *chain, const char *filename )
{
	FILE *f;
	int go;

	f = fopen( filename, "r" );
	if (!f)
		return -1;

	go = jtag_parse_stream( chain, f );

	fclose(f);
	if(debug_mode & 1)printf("File Closed gp=%d\n",go);
	return go;
}
