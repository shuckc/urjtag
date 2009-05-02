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

#include "urjtag/gettext.h"
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

#ifdef __MINGW32__
#define _NO_W32_PSEUDO_MODIFIERS
#include <windows.h>
#define geteuid() 0
#define getuid() 0
/* Microsoft uses a different swprintf() than ISO C requires */
#include <stdio.h>
#define swprintf _snwprintf
#endif

#endif /* SYSDEP_H */
