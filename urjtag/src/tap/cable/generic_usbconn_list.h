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

#ifdef ENABLE_LOWLEVEL_FTDI
# define _URJ_USB_FTDI(x) _URJ_USB(x##_ftdi)
#else
# define _URJ_USB_FTDI(x)
#endif
#ifdef ENABLE_LOWLEVEL_FTD2XX
# define _URJ_USB_FTD2XX(x) _URJ_USB(x##_ftd2xx)
#else
# define _URJ_USB_FTD2XX(x)
#endif
#define _URJ_USB_FTDX(x) \
    _URJ_USB_FTDI(x) \
    _URJ_USB_FTD2XX(x)

#ifdef ENABLE_CABLE_XPC
_URJ_USB(xpc_int)
_URJ_USB(xpc_ext)
#endif
#ifdef ENABLE_CABLE_JLINK
_URJ_USB(jlink)
#endif
#ifdef ENABLE_CABLE_FT2232
_URJ_USB_FTDX(ft2232)
_URJ_USB_FTDX(armusbocd)
_URJ_USB_FTDX(armusbocdtiny)
_URJ_USB_FTDX(armusbtiny_h)
_URJ_USB_FTDX(flyswatter)
_URJ_USB_FTDX(gnice)
_URJ_USB_FTDX(gniceplus)
_URJ_USB_FTDX(jtagkey)
_URJ_USB_FTDX(milkymist)
_URJ_USB_FTDX(oocdlinks)
_URJ_USB_FTDX(signalyzer)
_URJ_USB_FTDX(turtelizer2)
_URJ_USB_FTDX(usbjtagrs232)
_URJ_USB_FTDX(usbscarab2)
_URJ_USB_FTDX(usbtojtagif)
#endif
#ifdef ENABLE_CABLE_USBBLASTER
_URJ_USB_FTDX(usbblaster)
_URJ_USB_FTDX(cubic_cyclonium)
_URJ_USB_FTDX(nios_eval)
_URJ_USB_FTDX(usb_jtag)
#endif
#ifdef ENABLE_CABLE_ICE100
_URJ_USB(ice100B)
_URJ_USB(ice100Bw)
#endif
#ifdef ENABLE_CABLE_VSLLINK
_URJ_USB(vsllink)
#endif

#undef _URJ_USB_FTDI
#undef _URJ_USB_FTD2XX
#undef _URJ_USB_FTDX
#undef _URJ_USB
