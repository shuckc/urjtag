/*
 * $Id: xpc.c,v 1.8 2003/08/19 08:42:20 telka Exp $
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

static int
xpc_int_init( cable_t *cable )
{
	if (parport_open( cable->port )) return -1;
	if (parport_set_control( cable->port, 1 ) < 0) return -1;

	PARAM_TRST(cable) = 1;

	return 0;
}

static int
xpc_ext_init( cable_t *cable )
{
	if (parport_open( cable->port )) return -1;
	if (parport_set_control( cable->port, 0 ) < 0) return -1;

	PARAM_TRST(cable) = 1;

	return 0;
}


#define	PROG 3
#define	TCK	2
#define TMS 1
#define	TDI	0
#define	TDO	0

static void
xpc_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	parport_set_data( cable->port, (1 << PROG) | (0 << TCK) | (tms << TMS) | (tdi << TDI) );
	for (i = 0; i < n; i++) {
		parport_set_data( cable->port, (1 << PROG) | (1 << TCK) | (tms << TMS) | (tdi << TDI) );
		parport_set_data( cable->port, (1 << PROG) | (0 << TCK) | (tms << TMS) | (tdi << TDI) );
	}
}

static int
xpc_get_tdo( cable_t *cable )
{
	return (parport_get_data( cable->port ) >> TDO) & 1;
}

static int
xpc_set_trst( cable_t *cable, int trst )
{
	return 1;
}

void
xpc_set_frequency( cable_t *cable, uint32_t new_frequency )
{
	cable->frequency = new_frequency;
}


void
xpcu_usbcable_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s xpcu VID:PID\n"
		"\n"
		"VID        vendor ID (hex, e.g. 9FB, or empty)\n"
		"PID        product ID (hex, e.g. 6001, or empty)\n"
		"\n"
	), cablename );
}

cable_driver_t xpc_int_cable_driver = {
	"xpc_int",
	N_("Xilinx Platform Cable USB internal chain"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	xpc_int_init,
	generic_done,
	xpc_set_frequency,
	xpc_clock,
	xpc_get_tdo,
	generic_transfer,
	xpc_set_trst,
	generic_get_trst,
	generic_flush_using_transfer,
	xpcu_usbcable_help
};

cable_driver_t xpc_ext_cable_driver = {
	"xpc_ext",
	N_("Xilinx Platform Cable USB external chain"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	xpc_ext_init,
	generic_done,
	xpc_set_frequency,
	xpc_clock,
	xpc_get_tdo,
	generic_transfer,
	xpc_set_trst,
	generic_get_trst,
	generic_flush_using_transfer,
	xpcu_usbcable_help
};

