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

#ifndef SYSDEP_H
#define	SYSDEP_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <urjtag/gettext.h>
#define	_(s)		gettext(s)
#define	N_(s)		gettext_noop(s)
#define	P_(s,p,n)	ngettext(s,p,n)

#ifdef S_SPLINT_S
#undef gettext
#define	gettext(s)	s
#undef gettext_noop
#define	gettext_noop(s)	s
#undef ngettext
#define	ngettext(s,p,n)	s
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#ifdef __MINGW32__
/* Microsoft uses a different swprintf() than ISO C requires */
#include <stdio.h>
#define swprintf _snwprintf
/* No perms to give to mkdir */
#include <io.h>
#define mkdir(path, mode) mkdir(path)
#endif

/* Some Windows code likes to define this, so undo it here */
#ifdef interface
#undef interface
#endif

#ifndef HAVE_GETEUID
#define geteuid() 0
#endif

#ifndef HAVE_GETUID
#define getuid() 0
#endif

#ifndef HAVE_USLEEP
int usleep (long unsigned usec);
#endif

#ifndef HAVE_NANOSLEEP
#include <unistd.h>
#ifndef HAVE_STRUCT_TIMESPEC
struct timespec { unsigned long tv_sec, tv_nsec; };
#endif
#define nanosleep(req, rem) usleep((req)->tv_sec * 1000 * 1000 + (req)->tv_nsec / 1000)
#endif

#ifndef HAVE_GETLINE
#include <unistd.h>
#include <stdio.h>
extern ssize_t getline(char **line, size_t *len, FILE *f);
#endif

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

/* "b" is needed for Windows host.  "e" is a GLIBC extension, but Windows
   does not like it.  Most C libraries on UNIX-like systems hopefully
   implement the same extension or just ignore it without causing any trouble.
   So always appending "e" for them should be safe.  */
#if defined(_WIN32)
#define FOPEN_R  "rb"
#define FOPEN_W  "wb"
#else
#define FOPEN_R  "re"
#define FOPEN_W  "we"
#endif

#endif /* SYSDEP_H */
