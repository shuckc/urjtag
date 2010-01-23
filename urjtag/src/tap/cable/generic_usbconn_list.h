/*
 * $Id: generic_usbconn.c 1689 2010-01-08 17:06:27Z kawk $
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

#ifdef ENABLE_CABLE_XPC
_URJ_USB(xpc_int)
_URJ_USB(xpc_ext)
#endif
#ifdef ENABLE_CABLE_JLINK
_URJ_USB(jlink)
#endif
#ifdef ENABLE_CABLE_FT2232
#ifdef ENABLE_LOWLEVEL_FTD2XX
_URJ_USB(ft2232_ftd2xx)
_URJ_USB(armusbocd_ftd2xx)
_URJ_USB(armusbocdtiny_ftd2xx)
_URJ_USB(armusbtiny_h_ftd2xx)
_URJ_USB(gnice_ftd2xx)
_URJ_USB(gniceplus_ftd2xx)
_URJ_USB(jtagkey_ftd2xx)
_URJ_USB(oocdlinks_ftd2xx)
_URJ_USB(turtelizer2_ftd2xx)
_URJ_USB(usbtojtagif_ftd2xx)
_URJ_USB(signalyzer_ftd2xx)
_URJ_USB(flyswatter_ftd2xx)
_URJ_USB(usbscarab2_ftd2xx)
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
_URJ_USB(ft2232_ftdi)
_URJ_USB(armusbocd_ftdi)
_URJ_USB(armusbocdtiny_ftdi)
_URJ_USB(armusbtiny_h_ftdi)
_URJ_USB(gnice_ftdi)
_URJ_USB(gniceplus_ftdi)
_URJ_USB(jtagkey_ftdi)
_URJ_USB(oocdlinks_ftdi)
_URJ_USB(turtelizer2_ftdi)
_URJ_USB(usbtojtagif_ftdi)
_URJ_USB(signalyzer_ftdi)
_URJ_USB(flyswatter_ftdi)
_URJ_USB(usbscarab2_ftdi)
#endif
#endif
#ifdef ENABLE_CABLE_USBBLASTER
#ifdef ENABLE_LOWLEVEL_FTD2XX
_URJ_USB(usbblaster_ftd2xx)
_URJ_USB(cubic_cyclonium_ftd2xx)
_URJ_USB(nios_eval_ftd2xx)
_URJ_USB(usb_jtag_ftd2xx)
#endif
#ifdef ENABLE_LOWLEVEL_FTDI
_URJ_USB(usbblaster_ftdi)
_URJ_USB(cubic_cyclonium_ftdi)
_URJ_USB(nios_eval_ftdi)
_URJ_USB(usb_jtag_ftdi)
#endif
#endif

#undef _URJ_USB
