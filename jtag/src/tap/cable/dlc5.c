/*
 * $Id$
 *
 * Xilinx DLC5 JTAG Parallel Cable III Driver
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
 * [1] Xilinx, Inc., "JTAG Programmer Guide",
 *     http://toolbox.xilinx.com/docsan/3_1i/pdf/docs/jtg/jtg.pdf
 *
 */

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"
#include "generic_parport.h"

/* see Figure B-1 in [1] */

/*
 * data D[7:0] (pins 9:2)
 */
#define	TDI	0
#define	TCK	1
#define	TMS	2
#define	CTRL	3
#define	PROG	4

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO	4

static int
dlc5_init( cable_t *cable )
{
	if (parport_open( cable->link.port ))
		return -1;

	PARAM_SIGNALS(cable) = CS_TRST;

	return 0;
}

static void
dlc5_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->link.port, (1 << PROG) | (0 << TCK) | (tms << TMS) | (tdi << TDI) );
		cable_wait( cable );
		parport_set_data( cable->link.port, (1 << PROG) | (1 << TCK) | (tms << TMS) | (tdi << TDI) );
		cable_wait( cable );
	}

	PARAM_SIGNALS(cable) &= CS_TRST;
	PARAM_SIGNALS(cable) |= CS_TCK;
	PARAM_SIGNALS(cable) |= tms ? CS_TMS : 0;
	PARAM_SIGNALS(cable) |= tdi ? CS_TDI : 0;
}

static int
dlc5_get_tdo( cable_t *cable )
{
	parport_set_data( cable->link.port, (1 << PROG) | ( 0 << TCK ) );
	PARAM_SIGNALS(cable) &= ~(CS_TDI | CS_TCK | CS_TMS);

	cable_wait( cable );

	return (parport_get_status( cable->link.port ) >> TDO) & 1;
}

static int
dlc5_set_signal( cable_t *cable, int mask, int val )
{
	int prev_sigs = PARAM_SIGNALS(cable);

	mask &= (CS_TDI | CS_TCK | CS_TMS); // only these can be modified

	if (mask != 0)
	{
		int data = 0;
		int sigs = (prev_sigs & ~mask) | (val & mask);
		data |= (sigs & CS_TDI)  ? (1 << TDI)  : 0;
		data |= (sigs & CS_TCK)  ? (1 << TCK)  : 0;
		data |= (sigs & CS_TMS)  ? (1 << TMS)  : 0;
		parport_set_data( cable->link.port, ( 1 << PROG ) | data );
		PARAM_SIGNALS(cable) = sigs;
	}

	return prev_sigs;
}

cable_driver_t dlc5_cable_driver = {
	"DLC5",
	N_("Xilinx DLC5 JTAG Parallel Cable III"),
	generic_parport_connect,
	generic_disconnect,
	generic_parport_free,
	dlc5_init,
	generic_parport_done,
	generic_set_frequency,
	dlc5_clock,
	dlc5_get_tdo,
	generic_transfer,
	dlc5_set_signal,
	generic_get_signal,
	generic_flush_one_by_one,
	generic_parport_help
};
