/*
 * $Id$
 *
 * Lattice Parallel Port Download Cable Driver
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
 */

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"
#include "generic_parport.h"

/*
 * 0 - STROBE (pin 1)
 * 1 - AUTOFD (pin 14)
 * 2 - INIT (pin 16)
 * 3 - SELECT (pin 17)
 * data D[7:0] (pins 9:2)
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDI	0
#define	TCK	1
#define	TMS	2
#define	TRST	4
#define	TDO	6


static int
lattice_init( cable_t *cable )
{
	if (parport_open( cable->link.port ))
		return -1;

	PARAM_TRST(cable) = 1;
	
	return 0;
}

static void
lattice_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->link.port, (0 << TCK) | (tms << TMS) | (tdi << TDI) | (1 << TRST) );
		cable_wait( cable );
		parport_set_data( cable->link.port, (1 << TCK) | (tms << TMS) | (tdi << TDI) | (1 << TRST) );
		cable_wait( cable );
	}
}

static int
lattice_get_tdo( cable_t *cable )
{
	parport_set_data( cable->link.port, (0 << TCK) | (1 << TRST) );
	cable_wait( cable );
	return (parport_get_status( cable->link.port ) >> TDO) & 1;
}

static int
lattice_set_trst( cable_t *cable, int trst )
{
	return parport_set_data( cable->link.port, trst << TRST );
}

cable_driver_t lattice_cable_driver = {
	"Lattice",
	N_("Lattice Parallel Port JTAG Cable"),
	generic_parport_connect,
	generic_disconnect,
	generic_parport_free,
	lattice_init,
	generic_parport_done,
	generic_set_frequency,
	lattice_clock,
	lattice_get_tdo,
	generic_transfer,
	lattice_set_trst,
	generic_get_trst,
	generic_flush_one_by_one,
	generic_parport_help
};
