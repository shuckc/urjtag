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
 * Modified by Ville Voipio <vv@iki.fi>, 7-May-2008
 *
 */

#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "chain.h"
#include "cmd.h"
#include "jtag.h"

#define MAXINPUTLINE 100    /* Maximum input line length */



int
jtag_parse_line( chain_t *chain, char *line )
{
	int l, i, r, tcnt;
	char **a;
	char *c, *d;
	char *sline;
	
	if (line == NULL)
		return 1;
	l = strlen(line);
	if (l == 0)
		return 1;

	/* allocate as many chars as in the input line; this will be enough in all cases */
	sline = malloc((l+1) * sizeof(char));
	if (sline == NULL) {
		printf( _("Out of memory\n") );
		return 1;
	}

	/* count and copy the tokens */
	c = line;
	d = sline;
	tcnt = 0;
	while (1) {
		/* eat up leading spaces */
		while (isspace(*c))
			c++;
			
		/* if the command ends here (either by NUL or comment) */
		if (*c == '\0' || *c == '#')
			break;
			
		/* copy the meat (non-space, non-NUL) */
		while (!isspace(*c) && *c != '\0') {
			*d++ = *c++;
		}
		/* mark the end to the destination string */
		*d++ = '\0';
		tcnt++;
	}
	
	if (tcnt == 0) {
		free(sline);
		return 1;
	}
		
	/* allocate the token pointer table */
	a = malloc((tcnt + 1) * sizeof(char *));
	if (a == NULL) {
		fprintf(stderr, _("Out of memory\n"));
		return 1;
	}

	/* find the starting points for the tokens */
	d = sline;
	for (i = 0; i < tcnt; i++)
		{
		a[i] = d;
		while (*d++ != '\0')
			;	
		}
	a[tcnt] = NULL;

	r = cmd_run( chain, a );
	if(debug_mode & 1)printf("Return in jtag_parse_line r=%d\n",r);
	free( a );
	free(sline);
	return r;
}


int
jtag_parse_stream( chain_t *chain, FILE *f )
{
	char inputline[MAXINPUTLINE + 1];
	int go = 1, i, c, lnr, clip;

	/* read the stream line-by-line until EOF or "quit" */
	lnr = 0;
	do {
		i = 0;
		clip = 0;
		
		/* read stream until '\n' or EOF, copy at most MAXINPUTLINE-1 chars */
		while (1) {
			c = fgetc(f);
			if (c == EOF || c == '\n')
				break;
			if (i < sizeof(inputline) - 1)
				inputline[i++] = c;
			else 
				clip = 1;
			}
		inputline[i] = '\0';
		lnr++;
		if (clip)
			fprintf(stdout, "Warning: line %d exceeds %d characters, clipped\n", lnr, (int)sizeof(inputline) - 1);  
		go = jtag_parse_line(chain, inputline);
	}
	while (go && c != EOF);
	
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
