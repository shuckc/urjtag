/*
 * $Id$
 *
 * Philips UCB1400 Registers
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
 * Documentation:
 * [1] Philips Semiconductors, "UCB1400 Audio codec with touch screen
 *     controller and power management monitor Rev. 02", 21 June 2002,
 *     Order Number: 9397 750 0961 1
 *
 */

#ifndef	UCB1400_H
#define	UCB1400_H

#define	UCB1400_IO_Data				0x5A		/* see 12.11 in [1] */
#define	UCB1400_IO_Direction			0x5C		/* see 12.12 in [1] */
#define	UCB1400_Positive_INT_Enable		0x5E		/* see 12.13 in [1] */
#define	UCB1400_Negative_INT_Enable		0x60		/* see 12.14 in [1] */
#define	UCB1400_INT_Clear_Status		0x62		/* see 12.15 in [1] */
#define	UCB1400_Touch_Screen_Control		0x64		/* see 12.16 in [1] */
#define	UCB1400_ADC_Control			0x66		/* see 12.17 in [1] */
#define	UCB1400_ADC_Data			0x68		/* see 12.18 in [1] */
#define	UCB1400_Feature_Control_Status_1	0x6A		/* see 12.19 in [1] */
#define	UCB1400_Feature_Control_Status_2	0x6C		/* see 12.20 in [1] */
#define	UCB1400_Test_Control			0x6E		/* see 12.21 in [1] */
#define	UCB1400_Extra_Interrupt			0x70		/* see 12.22 in [1] */

/* UCB1400 data/interrupt bits - see 12.11 - 12.15 in [1] */

#define	UCB1400_IO9				bit(9)
#define	UCB1400_IO8				bit(8)
#define	UCB1400_IO7				bit(7)
#define	UCB1400_IO6				bit(6)
#define	UCB1400_IO5				bit(5)
#define	UCB1400_IO4				bit(4)
#define	UCB1400_IO3				bit(3)
#define	UCB1400_IO2				bit(2)
#define	UCB1400_IO1				bit(1)
#define	UCB1400_IO0				bit(0)

/* UCB1400 interrupt bits - see 12.13 - 12.15 in [1] */

#define	UCB1400_INT_OVL				bit(15)
#define	UCB1400_INT_CLP				bit(14)
#define	UCB1400_INT_TMX				bit(13)
#define	UCB1400_INT_TPX				bit(12)
#define	UCB1400_INT_ADC				bit(11)

/* UCB1400_Touch_Screen_Control bits - see 12.16 in [1] */

#define	UCB1400_TSC_MX				bit(13)
#define	UCB1400_TSC_PX				bit(12)
#define	UCB1400_TSC_BIAS			bit(11)
#define	UCB1400_TSC_HYSD			bit(10)
#define	UCB1400_TSC_TM_MASK			bits(9,8)
#define	UCB1400_TSC_TM(x)			bits_val(9,8,x)
#define	UCB1400_TSC_PYG				bit(7)
#define	UCB1400_TSC_MYG				bit(6)
#define	UCB1400_TSC_PXG				bit(5)
#define	UCB1400_TSC_MXG				bit(4)
#define	UCB1400_TSC_PYP				bit(3)
#define	UCB1400_TSC_MYP				bit(2)
#define	UCB1400_TSC_PXP				bit(1)
#define	UCB1400_TSC_MXP				bit(0)

/* UCB1400_ADC_Control bits - see 12.17 in [1] */

#define	UCB1400_ADCC_AE				bit(15)
#define	UCB1400_ADCC_AS				bit(7)
#define	UCB1400_ADCC_EXVEN			bit(5)
#define	UCB1400_ADCC_AI_MASK			bits(4,2)
#define	UCB1400_ADCC_AI(x)			bits_val(4,2,x)
#define	UCB1400_ADCC_VREFB			bit(1)
#define	UCB1400_ADCC_ASE			bit(0)

/* UCB1400_ADC_Data bits - see 12.18 in [1] */

#define	UCB1400_ADCD_ADV			bit(15)
#define	UCB1400_ADCD_AD_MASK			bits(9,0)
#define	UCB1400_ADCD_AD(x)			bits_val(9,0,x)

/* UCB1400_Feature_Control_Status_1 bits - see 12.19 in [1] */

#define	UCB1400_FCS1_BB_MASK			bits(14,11)
#define	UCB1400_FCS1_BB(x)			bits_val(14,11,x)
#define	UCB1400_FCS1_TR_MASK			bits(10,9)
#define	UCB1400_FCS1_TR(x)			bits_val(10,9,x)
#define	UCB1400_FCS1_M_MASK			bits(8,7)
#define	UCB1400_FCS1_M(x)			bits_val(8,7,x)
#define	UCB1400_FCS1_HPEN			bit(6)
#define	UCB1400_FCS1_DE				bit(5)
#define	UCB1400_FCS1_DC				bit(4)
#define	UCB1400_FCS1_HIPS			bit(3)
#define	UCB1400_FCS1_GIEN			bit(2)
#define	UCB1400_FCS1_OVFL			bit(0)

/* UCB1400_Feature_Control_Status_2 bits - see 12.20 in [1] */

#define	UCB1400_FCS2_SMT			bit(15)
#define	UCB1400_FCS2_SUEV_MASK			bits(14,13)
#define	UCB1400_FCS2_SUEV(x)			bits_val(14,13,x)
#define	UCB1400_FCS2_AVE			bit(12)
#define	UCB1400_FCS2_AVEN_MASK			bits(11,10)
#define	UCB1400_FCS2_AVEN(x)			bits_val(11,10,x)
#define	UCB1400_FCS2_SLP_MASK			bits(5,4)
#define	UCB1400_FCS2_SLP(x)			bits_val(5,4,x)
#define	UCB1400_FCS2_EV_MASK			bits(2,0)
#define	UCB1400_FCS2_EV(x)			bits_val(2,0,x)

/* UCB1400_Test_Control bits - see 12.21 in [1] */

#define	UCB1400_TC_TM_MASK			bits(6,0)
#define	UCB1400_TC_TM(x)			bits_val(6,0,x)

/* UCB1400_Extra_Interrupt bits - see 12.22 in [1] */

#define	UCB1400_EI_CLPL				bit(15)
#define	UCB1400_EI_CLPR				bit(14)
#define	UCB1400_EI_CLPG				bit(13)

#endif /* UCB1400_H */
