/*
 * $Id: wiggler2.c,v 1.8 2003/09/11 16:45:15 telka Exp $
 *
 * Modified WIGGLER JTAG Cable Driver
 * Copyright (C) 2003 Ultra d.o.o.
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
 * Base on the code for the Macraigor WIGGLER code written by Marcel Telka.
 * Modified by Matej Kupljen <matej.kupljen@ultra.si> to support
 * the Modified WIGGLER JTAG cable. This has an additional pin, that is
 * used for CPU reset. The schematic is based on the source code for the
 * open source JTAG debugger for the PXA250 (255) processor, called Jelie
 * <www.jelie.org>.
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
#define	TDI		3
#define	TCK		2
#define	TMS		1
#define	TRST		4
#define CPU_RESET 	0

/* Certain Macraigor Wigglers appear to use one of the unused data lines as a
   power line so set all unused bits high. */
#define UNUSED_BITS (~((1 << TDI) | (1 << TCK) | (1 << TMS) | (1 << TRST) | (1 << CPU_RESET)) & 0xff)

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO 	7

static int
wiggler2_init( cable_t *cable )
{
	int data;

	if (parport_open( cable->link.port ))
		return -1;

	// TODO: CPU_RESET bit is set to zero here and can't be changed afterwards
	// If CPU_RESET=0 doesn't harm, it means it is an active high signal? - kawk

	if ((data = parport_get_data( cable->link.port )) < 0) {
		if (parport_set_data( cable->link.port, (1 << TRST) | UNUSED_BITS ))
			return -1;
		PARAM_SIGNALS(cable) = CS_TRST;
	} else
		PARAM_SIGNALS(cable) = ((data >> TRST) && 1) ? CS_TRST : 0;

	return 0;
}

static void
wiggler2_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;
	int trst = (PARAM_SIGNALS(cable) & CS_TRST) ? 1 : 0;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->link.port, (trst << TRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI) | UNUSED_BITS );
		cable_wait( cable );
		parport_set_data( cable->link.port, (trst << TRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI) | UNUSED_BITS );
		cable_wait( cable );
	}

	PARAM_SIGNALS(cable) &= CS_TRST;
	PARAM_SIGNALS(cable) |= CS_TCK;
	PARAM_SIGNALS(cable) |= tms ? CS_TMS : 0;
	PARAM_SIGNALS(cable) |= tdi ? CS_TDI : 0;
}

static int
wiggler2_get_tdo( cable_t *cable )
{
	int trst = (PARAM_SIGNALS(cable) & CS_TRST) ? 1 : 0;

	parport_set_data( cable->link.port, (trst << TRST) | (0 << TCK) | UNUSED_BITS );
	PARAM_SIGNALS(cable) &= ~(CS_TDI | CS_TCK | CS_TMS);

	cable_wait( cable );

	return (parport_get_status( cable->link.port ) >> TDO) & 1;
}

static int
wiggler2_set_signal( cable_t *cable, int mask, int val )
{
	int prev_sigs = PARAM_SIGNALS(cable);

	mask &= (CS_TDI | CS_TCK | CS_TMS | CS_TRST); // only these can be modified

	if (mask != 0)
	{
		int data = 0;
		int sigs = (prev_sigs & ~mask) | (val & mask);
		data |= (sigs & CS_TDI)  ? (1 << TDI)  : 0;
		data |= (sigs & CS_TCK)  ? (1 << TCK)  : 0;
		data |= (sigs & CS_TMS)  ? (1 << TMS)  : 0;
		data |= (sigs & CS_TRST) ? (1 << TRST) : 0;
		parport_set_data( cable->link.port, data | UNUSED_BITS );
		PARAM_SIGNALS(cable) = sigs;
	}

	return prev_sigs;
}

cable_driver_t wiggler2_cable_driver = {
	"WIGGLER2",
	N_("Modified (with CPU Reset) WIGGLER JTAG Cable"),
	generic_parport_connect,
	generic_disconnect,
	generic_parport_free,
	wiggler2_init,
	generic_parport_done,
	generic_set_frequency,
	wiggler2_clock,
	wiggler2_get_tdo,
	generic_transfer,
	wiggler2_set_signal,
	generic_get_signal,
	generic_flush_one_by_one,
	generic_parport_help
};
