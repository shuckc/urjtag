/*
 * $Id$
 *
 * TAP state handling
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

#include "state.h"

static int state = Unknown_State;
static int trst = 0;

int
tap_state( void )
{
	return state;
}

int
tap_state_init( void )
{
	return state = Unknown_State;
}

int
tap_state_done( void )
{
	return state = Unknown_State;
}

int
tap_state_set_trst( int new_trst )
{
	if (trst != (new_trst & 1)) {
		if (trst)
			state = Unknown_State;
		else
			state = Test_Logic_Reset;

		trst = new_trst & 1;
	}

	return state;
}

int
tap_state_get_trst( void )
{
	return trst;
}

int
tap_state_clock( int tms )
{
	if (tms & 1) {
		switch (state) {
			case Test_Logic_Reset:
				break;
			case Run_Test_Idle:
			case Update_DR:
			case Update_IR:
				state = Select_DR_Scan;
				break;
			case Select_DR_Scan:
				state = Select_IR_Scan;
				break;
			case Capture_DR:
			case Shift_DR:
				state = Exit1_DR;
				break;
			case Exit1_DR:
			case Exit2_DR:
				state = Update_DR;
				break;
			case Pause_DR:
				state = Exit2_DR;
				break;
			case Select_IR_Scan:
				state = Test_Logic_Reset;
				break;
			case Capture_IR:
			case Shift_IR:
				state = Exit1_IR;
				break;
			case Exit1_IR:
			case Exit2_IR:
				state = Update_IR;
				break;
			case Pause_IR:
				state = Exit2_IR;
				break;
			default:
				state = Unknown_State;
				break;
		}
	} else {
		switch (state) {
			case Test_Logic_Reset:
			case Run_Test_Idle:
			case Update_DR:
			case Update_IR:
				state = Run_Test_Idle;
				break;
			case Select_DR_Scan:
				state = Capture_DR;
				break;
			case Capture_DR:
			case Shift_DR:
			case Exit2_DR:
				state = Shift_DR;
				break;
			case Exit1_DR:
			case Pause_DR:
				state = Pause_DR;
				break;
			case Select_IR_Scan:
				state = Capture_IR;
				break;
			case Capture_IR:
			case Shift_IR:
			case Exit2_IR:
				state = Shift_IR;
				break;
			case Exit1_IR:
			case Pause_IR:
				state = Pause_IR;
				break;
			default:
				state = Unknown_State;
				break;
		}
	}

	return state;
}
