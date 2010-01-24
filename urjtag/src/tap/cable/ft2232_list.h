/*
 * $Id$
 *
 * Generic cable driver for FTDI's FT2232C chip in MPSSE mode.
 * Copyright (C) 2007 A. Laeuger
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
 * Written by Arnim Laeuger, 2007-2008.
 * Support for JTAGkey submitted by Laurent Gauch, 2008.
 * Support for FT2232H written by Michael Hennerich, 2009; adapted
 *      for urjtag codebase and submitted by Adam Megacz, 2010.
 *
 */

_URJ_FTDI(ft2232)
_URJ_FTDI(armusbocd)
_URJ_FTDI(gnice)
_URJ_FTDI(gniceplus)
_URJ_FTDI(jtagkey)
_URJ_FTDI(oocdlinks)
_URJ_FTDI(turtelizer2)
_URJ_FTDI(usbtojtagif)
_URJ_FTDI(signalyzer)
_URJ_FTDI(flyswatter)
_URJ_FTDI(usbscarab2)

#undef _URJ_FTDI
