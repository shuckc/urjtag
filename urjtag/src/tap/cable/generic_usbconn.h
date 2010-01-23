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
#define URJ_TAP_CABLE_GENERIC_USBCONN_H

#include <urjtag/cable.h>
#include <urjtag/usbconn.h>

#define _URJ_USB(usb) extern urj_usbconn_cable_t urj_tap_cable_usbconn_##usb;
#include "generic_usbconn_list.h"

/**
 * @return URJ_STATUS_OK on success, URJ_STATUS_FAIL and urj_error on error
 */
int urj_tap_cable_generic_usbconn_connect (urj_cable_t *cable,
                                           const urj_param_t *params[]);
void urj_tap_cable_generic_usbconn_done (urj_cable_t *cable);
void urj_tap_cable_generic_usbconn_help (urj_log_level_t ll, const char *name);
void urj_tap_cable_generic_usbconn_free (urj_cable_t *cable);

#endif /* URJ_TAP_CABLE_GENERIC_H */
