/*
 * $Id$
 *
 * Ka-Ro TRITON Starterkit II (PXA255/250) JTAG Cable
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
 * Modified for TRITON by Andreas Mohr <andi@lisas.de>, 2003
 *
 */

/*
 * Ka-Ro electronics GmbH (http://www.karo-electronics.de)
 * TRITON Starterkit II (PXA255/250) JTAG Parallel Cable Driver
 * (boards probably produced by www.mite.cz)
 * Other vendors: www.strategic-test.com, www.fsforth.de (www.es-usa.com),
 *                www.directinsight.co.uk, www.quantum.com.pl, 
 *
 * This code has been verified to work with a Starterkit II,
 * but a Starterkit I might also work (however it has a differing JTAG cable
 * interface circuit, so all bets are off).
 */

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define	TDI	1
#define	TCK	0
#define	TMS	2
#define	TRST	3
#define SRESET	4
#define ENAB	5 /* not programmed, since it's always 0 */

/*
 * status
 *
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO 	7	

static int
triton_init( cable_t *cable )
{
	if (parport_open( cable->port ))
		return -1;

	PARAM_TRST(cable) = 1;
	PARAM_SRESET(cable) = 1;

	return 0;
}

static void
triton_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_SRESET(cable) << SRESET) | (0 << TCK) | (tms << TMS) | (tdi << TDI) );
		cable_wait();
		parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_SRESET(cable) << SRESET) | (1 << TCK) | (tms << TMS) | (tdi << TDI) );
		cable_wait();
	}
}

static int
triton_get_tdo( cable_t *cable )
{
	parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_SRESET(cable) << SRESET) | (0 << TCK) );
	cable_wait();
	return (parport_get_status( cable->port ) >> TDO) & 1;
}

static int
triton_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;

	parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_SRESET(cable) << SRESET) );
	return PARAM_TRST(cable);
}

cable_driver_t triton_cable_driver = {
	"TRITON",
	N_("Ka-Ro TRITON Starterkit II (PXA255/250) JTAG Cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	triton_init,
	generic_done,
	triton_clock,
	triton_get_tdo,
	generic_transfer,
	triton_set_trst,
	generic_get_trst,
	generic_lptcable_help
};
