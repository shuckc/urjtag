/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#ifndef	JTAG_STATE_H
#define	JTAG_STATE_H

#include <common.h>

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

#endif /* JTAG_STATE_H */
