/*
 * $Id$
 *
 * XScale PXA250/PXA210 PWM0 and PWM1 Registers
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
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#ifndef	PXA2X0_PWM_H
#define	PXA2X0_PWM_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* PWM0 and PWM1 Registers */

#define	PWM0_BASE	0x40B00000
#define	PWM1_BASE	0x40C00000

#if LANGUAGE == C
typedef volatile struct PWM_registers {
	uint32_t pwm_ctrl;
	uint32_t pwm_pwduty;
	uint32_t pwm_perval;
} PWM_registers;

#ifdef PXA2X0_UNMAPPED
#define	PWM0_pointer	((PWM_registers*) PWM0_BASE)
#define	PWM1_pointer	((PWM_registers*) PWM1_BASE)
#endif

#define	PWM_CTRL	PWM_pointer->pwm_ctrl
#define	PWM_PWDUTY	PWM_pointer->pwm_pwduty
#define	PWM_PERVAL	PWM_pointer->pwm_perval

#define	PWM_CTRL0	PWM0_pointer->pwm_ctrl
#define	PWM_PWDUTY0	PWM0_pointer->pwm_pwduty
#define	PWM_PERVAL0	PWM0_pointer->pwm_perval

#define	PWM_CTRL1	PWM1_pointer->pwm_ctrl
#define	PWM_PWDUTY1	PWM1_pointer->pwm_pwduty
#define	PWM_PERVAL1	PWM1_pointer->pwm_perval
#endif /* LANGUAGE == C */

#define	PWM_CTRL_OFFSET		0x00
#define	PWM_PWDUTY_OFFSET	0x04
#define	PWM_PERVAL_OFFSET	0x08

/* PWM_CTRL bits - see Table 4-49 in [1] */

#define	PWM_CTRL_PWM_SD		bit(6)
#define	PWM_CTRL_PRESCALE_MASK	0x3F
#define	PWM_CTRL_PRESCALE(x)	(x & PWM_CTRL_PRESCALE_MASK)

/* PWM_PWDUTY bits - see Table 4-50 in [1] */

#define	PWM_PWDUTY_FDCYCLE	bit(10)
#define	PWM_PWDUTY_DCYCLE_MASK	0x3FF
#define	PWM_PWDUTY_DCYCLE(x)	(x & PWM_PWDUTY_DCYCLE_MASK)

/* PWM_PERVAL bits - see Table 4-51 in [1] */

#define	PWM_PERVAL_PV_MASK	0x3FF
#define	PWM_PERVAL_PV(x)	(x & PWM_PERVAL_PV_MASK)

#endif /* PXA2X0_PWM_H */
