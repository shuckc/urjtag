/*
 * $Id$
 *
 * Mpcbdm JTAG Cable Driver
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
 * Modified for Mpcbdm by Christian Pellegrin <chri@ascensit.com>, 2003.
 *
 * Documentation:
 * [1] http://www.vas-gmbh.de/software/mpcbdm/VDB2.gif
 *
 */

#include <config.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

/*
 * data
 */
#define	TDI	1
#define	TCK	0
#define	TMS	2

/* 
 * control 
 */
#define	TRST	1
#define	TRST1	3

/* 
 * status
 */
#define	TDO 	5	

static int
mpcbdm_init( cable_t *cable )
{
	if (parport_open( cable->port ))
		return -1;

	parport_set_control( cable->port, (1 << TRST) | (1 << TRST1) );
	PARAM_TRST(cable) = 1;

	return 0;
}

static void
mpcbdm_clock( cable_t *cable, int tms, int tdi )
{
	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	parport_set_data( cable->port, (0 << TCK) | (tms << TMS) | (tdi << TDI) );
	parport_set_control( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_TRST(cable) << TRST1) );
	cable_wait();
	parport_set_data( cable->port, (1 << TCK) | (tms << TMS) | (tdi << TDI) );
	parport_set_control( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_TRST(cable) << TRST1) );
	cable_wait();
}

static int
mpcbdm_get_tdo( cable_t *cable )
{
	parport_set_data( cable->port, 0 << TCK );
	parport_set_control( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_TRST(cable) << TRST1) );
	cable_wait();
	return (parport_get_status( cable->port ) >> TDO) & 1;
}

static int
mpcbdm_set_trst( cable_t *cable, int trst )
{
	PARAM_TRST(cable) = trst ? 1 : 0;

	parport_set_control( cable->port, (PARAM_TRST(cable) << TRST) | (PARAM_TRST(cable) << TRST1) );
	return PARAM_TRST(cable);
}

cable_driver_t mpcbdm_cable_driver = {
	"MPCBDM",
	N_("Mpcbdm JTAG cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	mpcbdm_init,
	generic_done,
	mpcbdm_clock,
	mpcbdm_get_tdo,
	mpcbdm_set_trst,
	generic_get_trst
};
