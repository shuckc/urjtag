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

#include <sys/io.h>

#include "cable.h"

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

static int trst;
static unsigned int port;

static int
mpcbdm_init( unsigned int aport )
{
	port = aport;
	if (((port + 3 <= 0x400) && ioperm( port, 3, 1 )) || ((port + 3 > 0x400) && iopl( 3 )))
		return 0;

	outb( (1 << TRST) | (1 << TRST1), port + 2 );
	trst = 1;

	return 1;
}

static void
mpcbdm_done( void )
{
	if (port + 3 <= 0x400)
		ioperm( port, 3, 0 );
	else
		iopl( 0 );
}

static void
mpcbdm_clock( int tms, int tdi )
{
	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	outb( (0 << TCK) | (tms << TMS) | (tdi << TDI), port );
	outb( (trst << TRST) | (trst << TRST1), port + 2 );
	cable_wait();
	outb( (1 << TCK) | (tms << TMS) | (tdi << TDI), port );
	outb( (trst << TRST) | (trst << TRST1), port + 2 );
	cable_wait();
}

static int
mpcbdm_get_tdo( void )
{
	outb( (0 << TCK), port );
	outb( (trst << TRST) | (trst << TRST1), port + 2 );
	cable_wait();
	return ((inb( port + 1 ) ^ 0x80) >> TDO) & 1;		/* BUSY is inverted */
}

static int
mpcbdm_set_trst( int new_trst )
{
	trst = new_trst ? 1 : 0;

	outb( (trst << TRST) | (trst << TRST1), port + 2 );
	return trst;
}

static int
mpcdbm_get_trst( void )
{
	return trst;
}

cable_driver_t mpcbdm_cable_driver = {
	"MPCBDM",
	"Mpcbdm JTAG cable",
	mpcbdm_init,
	mpcbdm_done,
	mpcbdm_clock,
	mpcbdm_get_tdo,
	mpcbdm_set_trst,
	mpcdbm_get_trst
};
