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

#include <sys/io.h>

#include "cable.h"
#include "state.h"

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

static unsigned int port;

static int
dlc5_init( unsigned int aport )
{
	tap_state_init();
	port = aport;
	return !ioperm( port, 2, 1 );
}

static void
dlc5_done( void )
{
	ioperm( port, 2, 0 );

	tap_state_done();
}

static void
dlc5_clock( int tms, int tdi )
{
	tms &= 1;
	tdi &= 1;

	outb( (1 << PROG) | (0 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();
	outb( (1 << PROG) | (1 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();

	tap_state_clock( tms );
}

static int
dlc5_get_tdo( void )
{
	outb( (1 << PROG) | (0 << TCK), port );
	cable_wait();
	return ((inb( port + 1 ) ^ 0x80) >> TDO) & 1;		/* BUSY is inverted */
}

static void
dlc5_set_trst( int new_trst )
{
}

cable_driver_t dlc5_cable_driver = {
	"DLC5",
	"Xilinx DLC5 JTAG Parallel Cable III",
	dlc5_init,
	dlc5_done,
	dlc5_clock,
	dlc5_get_tdo,
	dlc5_set_trst
};
