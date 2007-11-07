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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#ifdef ENABLE_NLS
#include <locale.h>
#endif /* ENABLE_NLS */

#include "chain.h"
#include "bus.h"
#include "cmd.h"
#include "jtag.h"

#ifndef HAVE_GETLINE
ssize_t getline( char **lineptr, size_t *n, FILE *stream );
#endif

chain_t *chain = NULL;
int debug_mode = 0;
int big_endian = 0;
extern cfi_array_t *cfi_array;

static char *
get_token( char *buf )
{
	char *t = strtok( buf, " \f\n\r\t\v" );
	if (t && (*t == '#'))
		return NULL;
	return t;
}

#define	JTAGDIR		".jtag"
#define	HISTORYFILE	"history"
#define	RCFILE		"rc"

static void
jtag_create_jtagdir( void )
{
	char *home = getenv( "HOME" );
	char *jdir;

	if (!home)
		return;

	jdir = malloc( strlen(home) + strlen(JTAGDIR) + 2 );	/* "/" and trailing \0 */
	if (!jdir)
		return;

	strcpy( jdir, home );
	strcat( jdir, "/" );
	strcat( jdir, JTAGDIR );

	/* Create the directory if it doesn't exists. */
	mkdir( jdir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

	free( jdir );
}
						 
static void
jtag_load_history( void )
{
	char *home = getenv( "HOME" );
	char *file;

	using_history();

	if (!home)
		return;

	file = malloc( strlen(home) + strlen(JTAGDIR) + strlen(HISTORYFILE) + 3 );	/* 2 x "/" and trailing \0 */
	if (!file)
		return;

	strcpy( file, home );
	strcat( file, "/" );
	strcat( file, JTAGDIR );
	strcat( file, "/" );
	strcat( file, HISTORYFILE );

	read_history( file );

	free( file );
}

static void
jtag_save_history( void )
{
	char *home = getenv( "HOME" );
	char *file;

	if (!home)
		return;

	file = malloc( strlen(home) + strlen(JTAGDIR) + strlen(HISTORYFILE) + 3);	/* 2 x "/" and trailing \0 */
	if (!file)
		return;

	strcpy( file, home );
	strcat( file, "/" );
	strcat( file, JTAGDIR );
	strcat( file, "/");
	strcat( file, HISTORYFILE );

	write_history( file );

	free( file );
}

static int
jtag_parse_line( char *line )
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

	r = cmd_run( a );
	if(debug_mode & 1)printf("Return in jtag_parse_line r=%d\n",r);
	free( a );
	return r;
}


static int jtag_readline_multiple_commands_support(char * line) /* multiple commands should be separated with '::' */
{
  int 	r;
  char	*nextcmd = line;

  if (!line || !(strlen( line ) > 0))
		return 1;
  
  do {
  line = nextcmd;

  nextcmd = strstr(nextcmd, "::"); /* :: to not confuse ms-dos users ;-) */
  
  if (nextcmd) {  
    *nextcmd++ = 0;
     ++nextcmd;
     while (*line == ':') ++line;
  } 
  
  r = jtag_parse_line( line );
  
  } while (nextcmd && r);

  return r;
}

static void
jtag_readline_loop( const char *prompt )
{
	char *line = NULL;

	/* Iterate */
	while (jtag_readline_multiple_commands_support( line )) {
		free( line );

		/* Read a line from the terminal */
		line = readline( prompt );

		/* Check if we actually got something */
		if (line && (strlen( line ) > 0))
			add_history( line );
	}
	free( line );
}

static int
jtag_parse_stream( FILE *f )
{
	int go = 1;
	char *line = NULL;
	size_t n = 0;

	while (go && (getline( &line, &n, f ) != -1))
		if ((strlen(line) > 0) && (line[0] != '#'))
			go = jtag_parse_line(line);

	free(line);

	return go;
}

int
jtag_parse_file( const char *filename )
{
	FILE *f;
	int go;

	f = fopen( filename, "r" );
	if (!f)
		return -1;

	go = jtag_parse_stream( f );

	fclose(f);
	if(debug_mode & 1)printf("File Closed gp=%d\n",go);
	return go;
}

static int
jtag_parse_rc( void )
{
	char *home = getenv( "HOME" );
	char *file;
	int go;

	if (!home)
		return 1;

	file = malloc( strlen(home) + strlen(JTAGDIR) + strlen(RCFILE) + 3 );	/* 2 x "/" and trailing \0 */
	if (!file)
		return 1;

	strcpy( file, home );
	strcat( file, "/" );
	strcat( file, JTAGDIR );
	strcat( file, "/" );
	strcat( file, RCFILE );

	go = jtag_parse_file( file );

	free( file );

	return go;
}

static void
cleanup( void )
{
	cfi_array_free( cfi_array );
	cfi_array = NULL;

	if (bus) {
		bus_free( bus );
		bus = NULL;
	}
	chain_free( chain );
	chain = NULL;
}

int
main( int argc, const char **argv )
{
	int go;
	int i;

#ifdef ENABLE_NLS
	/* l10n support */
	setlocale( LC_ALL, "" );
	bindtextdomain( PACKAGE, LOCALEDIR );
	textdomain( PACKAGE );
#endif /* ENABLE_NLS */

	/* input from files */
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			chain = chain_alloc();
			if (!chain) {
				printf( _("Out of memory\n") );
				return -1;
			}

			go = jtag_parse_file( argv[i] );
			cleanup();
			if (go < 0) {
				printf( _("Unable to open file `%s'!\n"), argv[i] );
				break;
			}
		}
		return 0;
	}

	/* input from stdin */
	if (!isatty(0)) {
		chain = chain_alloc();
		if (!chain) {
			printf( _("Out of memory\n") );
			return -1;
		}

		jtag_parse_stream( stdin );

		cleanup();

		return 0;
	}

	/* interactive */
	printf(
			_("%s\n"
			"Copyright (C) 2002, 2003 ETC s.r.o.\n"
			"%s is free software, covered by the GNU General Public License, and you are\n"
			"welcome to change it and/or distribute copies of it under certain conditions.\n"
			"There is absolutely no warranty for %s.\n\n"), PACKAGE_STRING, PACKAGE_NAME, PACKAGE_NAME
	);

	chain = chain_alloc();
	if (!chain) {
		printf( _("Out of memory\n") );
		return -1;
	}

	printf( _("Warning: %s may damage your hardware! Type \"quit\" to exit!\n\n"), PACKAGE_NAME );
	printf( _("Type \"help\" for help.\n\n") );

	/* Create ~/.jtag */
	jtag_create_jtagdir();

	/* Parse and execute the RC file */
	go = jtag_parse_rc();

	if (go) {
		/* Load history */
		jtag_load_history();

		/* main loop */
		jtag_readline_loop( "jtag> " );

		/* Save history */
		jtag_save_history();
	}

	cleanup();

	return 0;
}
