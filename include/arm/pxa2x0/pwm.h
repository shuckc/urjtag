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
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", May 2002, Order Number: 278534-005
 *
 */

#ifndef	PXA2X0_PWM_H
#define	PXA2X0_PWM_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* PWM0 and PWM1 Registers */

#define	PWM0_BASE	0x40B00000
#define	PWM1_BASE	0x40C00000

typedef volatile struct PWM_registers {
	uint32_t pwm_ctrl;
	uint32_t pwm_pwduty;
	uint32_t pwm_perval;
} PWM_registers;

#ifndef PWM0_pointer
#define	PWM0_pointer	((PWM_registers*) PWM0_BASE)
#endif
#ifndef PWM1_pointer
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

#endif	/* PXA2X0_PWM_H */
