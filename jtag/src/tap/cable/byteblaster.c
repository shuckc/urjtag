/*
 * $Id$
 *
 * Altera ByteBlaster/ByteBlaster II/ByteBlasterMV Parallel Port Download Cable Driver
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
 * [1] Altera Corporation, "ByteBlaster Parallel Port Download Cable Data Sheet",
 *     February 1998, ver. 2.01, Order Number: A-DS-BYTE-02.01
 * [2] Altera Corporation, "ByteBlasterMV Parallel Port Download Cable Data Sheet",
 *     July 2002, Version 3.3, Order Number: DS-BYTBLMV-3.3
 * [3] Altera Corporation, "ByteBlaster II Parallel Port Download Cable Data Sheet",
 *     December 2002, Version 1.0, Order Number: DS-BYTEBLSTRII-1.0 L01-08739-00
 *
 */

#include <sys/io.h>

#include "cable.h"
#include "state.h"

/*
 * data D[7:0] (pins 9:2)
 */
#define	TDI	6
#define	TCK	0
#define	TMS	1

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO	7

static unsigned int port;

static int
byteblaster_init( unsigned int aport )
{
	tap_state_init();
	port = aport;
	return !(((port + 2 <= 0x400) && ioperm( port, 2, 1 )) || ((port + 2 > 0x400) && iopl( 3 )));
}

static void
byteblaster_done( void )
{
	if (port + 2 <= 0x400)
		ioperm( port, 2, 0 );
	else
		iopl( 0 );

	tap_state_done();
}

static void
byteblaster_clock( int tms, int tdi )
{
	tms &= 1;
	tdi &= 1;

	outb( (0 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();
	outb( (1 << TCK) | (tms << TMS) | (tdi << TDI), port );
	cable_wait();

	tap_state_clock( tms );
}

static int
byteblaster_get_tdo( void )
{
	outb( 0 << TCK, port );
	cable_wait();
	return ((inb( port + 1 ) ^ 0x80) >> TDO) & 1;		/* BUSY is inverted */
}

static void
byteblaster_set_trst( int new_trst )
{
}

cable_driver_t byteblaster_cable_driver = {
	"ByteBlaster",
	"Altera ByteBlaster/ByteBlaster II/ByteBlasterMV Parallel Port Download Cable",
	byteblaster_init,
	byteblaster_done,
	byteblaster_clock,
	byteblaster_get_tdo,
	byteblaster_set_trst
};
