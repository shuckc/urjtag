/*
 * $Id$
 *
 * Macraigor Wiggler JTAG Cable Driver
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] http://www.ocdemon.net/
 * [2] http://jtag-arm9.sourceforge.net/hardware.html
 *
 */

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define	nTRST	4	/* nTRST is not inverted in the cable */
#define	TDI	3
#define	TCK	2
#define	TMS	1
#define	nSRESET 0	/* sRESET is inverted in the cable */

/* Certain Macraigor Wigglers appear to use one of the unused data lines as a
   power line so set all unused bits high. */
#define UNUSED_BITS (~((1 << nTRST) | (1 << TDI) | (1 << TCK) | (1 << TMS) | (1 << nSRESET)) & 0xff)

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO 	7	

static int
wiggler_init( cable_t *cable )
{
	int data;

	if (parport_open( cable->port ))
		return -1;

	if ((data = parport_get_data( cable->port )) < 0) {
		if (parport_set_data( cable->port, (1 << nTRST) | UNUSED_BITS))
			return -1;
		PARAM_TRST(cable) = 1;
	} else
		PARAM_TRST(cable) = (data >> nTRST) & 1;

	return 0;
}

static void
wiggler_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->port, (PARAM_TRST(cable) << nTRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI) | UNUSED_BITS );
		cable_wait();
		parport_set_data( cable->port, 0xe0 | (PARAM_TRST(cable) << nTRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI) | UNUSED_BITS );
		cable_wait();
	}
}

static int
wiggler_get_tdo( cable_t *cable )
{
	parport_set_data( cable->port, (PARAM_TRST(cable) << nTRST) | (0 << TCK) | UNUSED_BITS );
	cable_wait();
	return (parport_get_status( cable->port ) >> TDO) & 1;
}

static int
wiggler_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;

	parport_set_data( cable->port, (PARAM_TRST(cable) << nTRST) | UNUSED_BITS );
	return PARAM_TRST(cable);
}

cable_driver_t wiggler_cable_driver = {
	"WIGGLER",
	N_("Macraigor Wiggler JTAG Cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	wiggler_init,
	generic_done,
	wiggler_clock,
	wiggler_get_tdo,
	generic_transfer,
	wiggler_set_trst,
	generic_get_trst,
	generic_lptcable_help
};

cable_driver_t igloo_cable_driver = {
	"IGLOO",
	N_("Excelpoint IGLOO JTAG Cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	wiggler_init,
	generic_done,
	wiggler_clock,
	wiggler_get_tdo,
	generic_transfer,
	wiggler_set_trst,
	generic_get_trst,
	generic_lptcable_help
};

