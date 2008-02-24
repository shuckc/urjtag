/*
 * $Id: generic.h 1002 2008-02-10 09:50:59Z kawk $
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

#ifndef GENERIC_USBCONN_H
#define	GENERIC_USBCONN_H

#include "cable.h"
#include "usbconn.h"

int generic_usbconn_connect( char *params[], cable_t *cable );
void generic_usbconn_done( cable_t *cable );
void generic_usbconn_help( const char *name );
void generic_usbconn_free( cable_t *cable );

#endif /* GENERIC_H */
