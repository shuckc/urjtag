/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#ifndef	STATE_H
#define	STATE_H

#define	bit(b)		(1 << (b))

#define	TAPSTAT_DR	bit(0)
#define	TAPSTAT_IR	bit(1)
#define	TAPSTAT_SHIFT	bit(2)		/* register shift with TMS = 0 */
#define	TAPSTAT_IDLE	bit(3)		/* to Run-Test/Idle with TMS = 0 */
#define	TAPSTAT_CAPTURE	bit(4)		/* Capture state */
#define	TAPSTAT_UPDATE	bit(5)		/* to Update with TMS = 1 */
#define	TAPSTAT_PAUSE	bit(6)		/* to Pause with TMS = 0 */
#define	TAPSTAT_RESET	bit(7)		/* Test-Logic-Reset or unknown state */

#define	Unknown_State		(TAPSTAT_RESET)
#define	Test_Logic_Reset	(TAPSTAT_RESET | TAPSTAT_IDLE)
#define	Run_Test_Idle		(TAPSTAT_IDLE)
#define	Select_DR_Scan		(TAPSTAT_DR)
#define	Capture_DR		(TAPSTAT_DR | TAPSTAT_SHIFT | TAPSTAT_CAPTURE)
#define	Shift_DR		(TAPSTAT_DR | TAPSTAT_SHIFT)
#define	Exit1_DR		(TAPSTAT_DR | TAPSTAT_UPDATE | TAPSTAT_PAUSE)
#define	Pause_DR		(TAPSTAT_DR | TAPSTAT_PAUSE)
#define	Exit2_DR		(TAPSTAT_DR | TAPSTAT_SHIFT | TAPSTAT_UPDATE)
#define	Update_DR		(TAPSTAT_DR | TAPSTAT_IDLE)
#define	Select_IR_Scan		(TAPSTAT_IR)
#define	Capture_IR		(TAPSTAT_IR | TAPSTAT_SHIFT | TAPSTAT_CAPTURE)
#define	Shift_IR		(TAPSTAT_IR | TAPSTAT_SHIFT)
#define	Exit1_IR		(TAPSTAT_IR | TAPSTAT_UPDATE | TAPSTAT_PAUSE)
#define	Pause_IR		(TAPSTAT_IR | TAPSTAT_PAUSE)
#define	Exit2_IR		(TAPSTAT_IR | TAPSTAT_SHIFT | TAPSTAT_UPDATE)
#define	Update_IR		(TAPSTAT_IR | TAPSTAT_IDLE)

int tap_state( void );
int tap_state_init( void );
int tap_state_done( void );
int tap_state_set_trst( int trst );
int tap_state_get_trst( void );
int tap_state_clock( int tms );

#endif /* STATE_H */
