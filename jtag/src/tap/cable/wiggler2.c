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

	if ((data = parport_get_data( cable->link.port )) < 0) {
		if (parport_set_data( cable->link.port, (0 << TRST) | UNUSED_BITS ))
			return -1;
		PARAM_TRST(cable) = 1;
	} else
		PARAM_TRST(cable) = (data >> TRST) & 1;

	return 0;
}

static void
wiggler2_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->link.port, (PARAM_TRST(cable) << TRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI) | UNUSED_BITS );
		cable_wait( cable );
		parport_set_data( cable->link.port, (PARAM_TRST(cable) << TRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI) | UNUSED_BITS );
		cable_wait( cable );
	}
}

static int
wiggler2_get_tdo( cable_t *cable )
{
	parport_set_data( cable->link.port, (PARAM_TRST(cable) << TRST) | (0 << TCK) | UNUSED_BITS );
	cable_wait( cable );
	return (parport_get_status( cable->link.port ) >> TDO) & 1;
}

static int
wiggler2_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;

	parport_set_data( cable->link.port, (PARAM_TRST(cable) << TRST) | UNUSED_BITS );
	return PARAM_TRST(cable);
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
	wiggler2_set_trst,
	generic_get_trst,
	generic_flush_one_by_one,
	generic_parport_help
};
