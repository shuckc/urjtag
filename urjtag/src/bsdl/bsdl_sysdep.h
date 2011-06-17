/*
 * $Id$
 *
 * Reduced version of the global sysdep.h that is suitable for the
 * BSDL subsystem components. config.h defines a number of macros
 * that collide with BSDL tokens.
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
 */

#ifndef BSDL_SYSDEP_H
#define BSDL_SYSDEP_H


#include "config.h"
/* Some of the config.h defines conflict with yacc output */
#undef PACKAGE

#include <urjtag/gettext.h>
#define _(s)            gettext(s)
#define N_(s)           gettext_noop(s)
#define P_(s,p,n)       ngettext(s,p,n)

#ifdef S_SPLINT_S
#  undef gettext
#  define gettext(s)      s
#  undef gettext_noop
#  define gettext_noop(s) s
#  undef ngettext
#  define ngettext(s,p,n) s
#endif

#endif /* BSDL_SYSDEP_H */
