/*
 * $Id$
 *
 * Arcom JTAG Cable Driver
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

/*
 * data D[7:0] (pins 9:2)
 */
#define	TDI	1
#define	TCK	0
#define	TMS	2
#define	TRST	3

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO 	7

static int
arcom_init( cable_t *cable )
{
	int data;

	if (parport_open( cable->port ))
		return -1;

	if ((data = parport_get_data( cable->port )) < 0) {
		if (parport_set_data( cable->port, 1 << TRST ))
			return -1;
		PARAM_TRST(cable) = 1;
	} else
		PARAM_TRST(cable) = (data >> TRST) & 1;

        return 0;
}

static void
arcom_clock( cable_t *cable, int tms, int tdi )
{
	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI) );
	cable_wait();
	parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI) );
	cable_wait();
}

static int
arcom_get_tdo( cable_t *cable )
{
	parport_set_data( cable->port, (PARAM_TRST(cable) << TRST) | (0 << TCK) );
	cable_wait();
	return (parport_get_status( cable->port ) >> TDO) & 1;
}

static int
arcom_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;

	parport_set_data( cable->port, PARAM_TRST(cable) << TRST );
	return PARAM_TRST(cable);
}

cable_driver_t arcom_cable_driver = {
	"ARCOM",
	N_("Arcom JTAG Cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	arcom_init,
	generic_done,
	arcom_clock,
	arcom_get_tdo,
	generic_transfer,
	arcom_set_trst,
	generic_get_trst
};
