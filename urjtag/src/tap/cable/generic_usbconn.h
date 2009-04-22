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

#ifndef URJ_TAP_CABLE_GENERIC_USBCONN_H
#define	URJ_TAP_CABLE_GENERIC_USBCONN_H

#include "cable.h"
#include "usbconn.h"

int urj_tap_cable_generic_usbconn_connect (char *params[], urj_cable_t *cable);
void urj_tap_cable_generic_usbconn_done (urj_cable_t *cable);
void urj_tap_cable_generic_usbconn_help (const char *name);
void urj_tap_cable_generic_usbconn_free (urj_cable_t *cable);

#endif /* URJ_TAP_CABLE_GENERIC_H */
