/*
 * $Id$
 *
 * Keith & Koep JTAG Cable Driver
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
 * [1] ftp://www.keith-koep.com/pub/arm-tools/jtag/jtag05_sch.pdf
 *
 */

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"
#include "generic_parport.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define	TDI	0
#define	TCK	1
#define	TMS	2

/*
 * status
 * 
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO	5

/* 
 * control
 *
 * 0 - STROBE (pin 1)
 * 1 - AUTOFD (pin 14)
 * 2 - INIT (pin 16)
 * 3 - SELIN (pin 17)
 */
#define	TRST	0

static int
keithkoep_init( cable_t *cable )
{
	if (parport_open( cable->link.port ))
		return -1;

	parport_set_control( cable->link.port, 1 << TRST );
	PARAM_SIGNALS(cable) = CS_TRST;

	return 0;
}

static void
keithkoep_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->link.port, (0 << TCK) | (tms << TMS) | (tdi << TDI) );
		cable_wait( cable );
		parport_set_data( cable->link.port, (1 << TCK) | (tms << TMS) | (tdi << TDI) );
		cable_wait( cable );
	}

	PARAM_SIGNALS(cable) &= CS_TRST;
	PARAM_SIGNALS(cable) |= CS_TCK;
	PARAM_SIGNALS(cable) |= tms ? CS_TMS : 0;
	PARAM_SIGNALS(cable) |= tdi ? CS_TDI : 0;
}

static int
keithkoep_get_tdo( cable_t *cable )
{
	parport_set_data( cable->link.port, 0 << TCK );
	PARAM_SIGNALS(cable) &= ~(CS_TDI | CS_TCK | CS_TMS);

	cable_wait( cable );

	return (parport_get_status( cable->link.port ) >> TDO) & 1;
}

static int
keithkoep_set_signal( cable_t *cable, int mask, int val )
{
	int prev_sigs = PARAM_SIGNALS(cable);

	mask &= (CS_TDI | CS_TCK | CS_TMS | CS_TRST); // only these can be modified

	if (mask != 0)
	{
		int sigs = (prev_sigs & ~mask) | (val & mask);

		if ((mask & ~CS_TRST) != 0)
		{
			int data = 0;
			data |= (sigs & CS_TDI)  ? (1 << TDI)  : 0;
			data |= (sigs & CS_TCK)  ? (1 << TCK)  : 0;
			data |= (sigs & CS_TMS)  ? (1 << TMS)  : 0;
			parport_set_data( cable->link.port, data );
		}
		if ((mask & CS_TRST) != 0)
		{
			parport_set_control( cable->link.port, (sigs & CS_TRST) ? (1 << TRST) : 0 );
		}
		PARAM_SIGNALS(cable) = sigs;
	}

	return prev_sigs;
}

cable_driver_t keithkoep_cable_driver = {
	"KeithKoep",
	N_("Keith & Koep JTAG cable"),
	generic_parport_connect,
	generic_disconnect,
	generic_parport_free,
	keithkoep_init,
	generic_parport_done,
	generic_set_frequency,
	keithkoep_clock,
	keithkoep_get_tdo,
	generic_transfer,
	keithkoep_set_signal,
	generic_get_signal,
	generic_flush_one_by_one,
	generic_parport_help
};
