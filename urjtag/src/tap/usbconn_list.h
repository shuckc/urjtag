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


/*
 * Please keep this list sorted alphabetically
 */

#ifdef ENABLE_LOWLEVEL_FTD2XX
_URJ_LIST(ftd2xx)
_URJ_LIST(ftd2xx_mpsse)
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
_URJ_LIST(ftdi)
_URJ_LIST(ftdi_mpsse)
#endif
#ifdef HAVE_LIBUSB
_URJ_LIST(libusb)
#endif

#undef _URJ_LIST
