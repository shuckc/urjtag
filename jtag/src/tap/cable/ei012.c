/*
 * $Id$
 *
 * ETC EI012 JTAG Cable Driver
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

#include <stdio.h>
#include <sys/io.h>

#include "cable.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define	TCK	0
#define	TDI	1
#define	TMS	2
#define	TRST	4

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO	7

static int trst;
static unsigned int port;

static int
ei012_init( unsigned int aport )
{
	port = aport;
	if (((port + 2 <= 0x400) && ioperm( port, 2, 1 )) || ((port + 2 > 0x400) && iopl( 3 )))
		return 0;
	trst = (inb( port ) >> TRST) & 1;

	return 1;
}

static void
ei012_done( void )
{
	if (port + 2 <= 0x400)
		ioperm( port, 2, 0 );
	else
		iopl( 0 );
}

static void
ei012_clock( int tms, int tdi )
{
	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	outb( (trst << TRST) | (0 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();
	outb( (trst << TRST) | (1 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();
}

static int
ei012_get_tdo( void )
{
	outb( (trst << TRST) | (0 << TCK), port );
	cable_wait();
	return ((inb( port + 1 ) ^ 0x80) >> TDO) & 1;		/* BUSY is inverted */
}

static int
ei012_set_trst( int new_trst )
{
	trst = new_trst ? 1 : 0;

	outb( trst << TRST, port );
	return trst;
}

static int
ei012_get_trst( void )
{
	return trst;
}

cable_driver_t ei012_cable_driver = {
	"EI012",
	"ETC EI012 JTAG Cable",
	ei012_init,
	ei012_done,
	ei012_clock,
	ei012_get_tdo,
	ei012_set_trst,
	ei012_get_trst
};
