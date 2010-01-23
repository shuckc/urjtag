/*
 * $Id: buses.h 1687 2009-12-29 23:53:09Z arniml $
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

#ifdef ENABLE_CABLE_ARCOM
_URJ_CABLE(arcom)
#endif
#ifdef ENABLE_CABLE_BYTEBLASTER
_URJ_CABLE(byteblaster)
#endif
#ifdef ENABLE_CABLE_USBBLASTER
_URJ_CABLE(usbblaster)
#endif
#ifdef ENABLE_CABLE_FT2232
_URJ_CABLE(ft2232)
_URJ_CABLE(ft2232_jtagkey)
_URJ_CABLE(ft2232_armusbocd)
_URJ_CABLE(ft2232_armusbtiny_h)
_URJ_CABLE(ft2232_gnice)
_URJ_CABLE(ft2232_gniceplus)
_URJ_CABLE(ft2232_oocdlinks)
_URJ_CABLE(ft2232_signalyzer)
_URJ_CABLE(ft2232_turtelizer2)
_URJ_CABLE(ft2232_usbtojtagif)
_URJ_CABLE(ft2232_flyswatter)
_URJ_CABLE(ft2232_usbscarab2)
#endif
#ifdef ENABLE_CABLE_DLC5
_URJ_CABLE(dlc5)
#endif
#ifdef ENABLE_CABLE_EA253
_URJ_CABLE(ea253)
#endif
#ifdef ENABLE_CABLE_EI012
_URJ_CABLE(ei012)
#endif
#ifdef ENABLE_CABLE_IGLOO
_URJ_CABLE(igloo)
#endif
#ifdef ENABLE_CABLE_KEITHKOEP
_URJ_CABLE(keithkoep)
#endif
#ifdef ENABLE_CABLE_LATTICE
_URJ_CABLE(lattice)
#endif
#ifdef ENABLE_CABLE_MPCBDM
_URJ_CABLE(mpcbdm)
#endif
#ifdef ENABLE_CABLE_TRITON
_URJ_CABLE(triton)
#endif
#ifdef ENABLE_JIM
_URJ_CABLE(jim)
#endif
#ifdef ENABLE_CABLE_WIGGLER
_URJ_CABLE(minimal)
_URJ_CABLE(wiggler)
_URJ_CABLE(wiggler2)
#endif
#ifdef ENABLE_CABLE_XPC
_URJ_CABLE(xpc_int)
_URJ_CABLE(xpc_ext)
#endif
#ifdef ENABLE_CABLE_JLINK
_URJ_CABLE(jlink)
#endif
#ifdef ENABLE_CABLE_EP9307
_URJ_CABLE(ep9307)
#endif
#ifdef ENABLE_CABLE_TS7800
_URJ_CABLE(ts7800)
#endif

#undef _URJ_CABLE
