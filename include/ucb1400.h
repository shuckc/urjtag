/*
 * $Id$
 *
 * Philips UCB1400 Registers
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
#define	UCB1400_TSC_TM_MASK			0x0300
#define	UCB1400_TSC_TM(x)			((x << 8) & UCB1400_TSC_TM_MASK)
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
#define	UCB1400_ADCC_AI_MASK			0x001C
#define	UCB1400_ADCC_AI(x)			((x << 2) & UCB1400_ADCC_AI_MASK)
#define	UCB1400_ADCC_VREFB			bit(1)
#define	UCB1400_ADCC_ASE			bit(0)

/* UCB1400_ADC_Data bits - see 12.18 in [1] */

#define	UCB1400_ADCD_ADV			bit(15)
#define	UCB1400_ADCD_AD_MASK			0x03FF
#define	UCB1400_ADCD_AD(x)			(x & UCB1400_ADCD_AD_MASK)

/* UCB1400_Feature_Control_Status_1 bits - see 12.19 in [1] */

#define	UCB1400_FCS1_BB_MASK			0x7800
#define	UCB1400_FCS1_BB(x)			((x << 11) & UCB1400_FCS1_BB_MASK)
#define	UCB1400_FCS1_TR_MASK			0x0600
#define	UCB1400_FCS1_TR(x)			((x << 9) & UCB1400_FCS1_TR_MASK)
#define	UCB1400_FCS1_M_MASK			0x0180
#define	UCB1400_FCS1_M(x)			((x << 7) & UCB1400_FCS1_M_MASK)
#define	UCB1400_FCS1_HPEN			bit(6)
#define	UCB1400_FCS1_DE				bit(5)
#define	UCB1400_FCS1_DC				bit(4)
#define	UCB1400_FCS1_HIPS			bit(3)
#define	UCB1400_FCS1_GIEN			bit(2)
#define	UCB1400_FCS1_OVFL			bit(0)

/* UCB1400_Feature_Control_Status_2 bits - see 12.20 in [1] */

#define	UCB1400_FCS2_SMT			bit(15)
#define	UCB1400_FCS2_SUEV_MASK			0x6000
#define	UCB1400_FCS2_SUEV(x)			((x << 13) & UCB1400_FCS2_SUEV_MASK)
#define	UCB1400_FCS2_AVE			bit(12)
#define	UCB1400_FCS2_AVEN_MASK			0x0C00
#define	UCB1400_FCS2_AVEN(x)			((x << 10) & UCB1400_FCS2_AVEN_MASK)
#define	UCB1400_FCS2_SLP_MASK			0x0030
#define	UCB1400_FCS2_SLP(x)			((x << 4) & UCB1400_FCS2_SLP_MASK)
#define	UCB1400_FCS2_EV_MASK			0x0007
#define	UCB1400_FCS2_EV(x)			(x & UCB1400_FCS2_EV_MASK)

/* UCB1400_Test_Control bits - see 12.21 in [1] */

#define	UCB1400_TC_TM_MASK			0x007F
#define	UCB1400_TC_TM(x)			(x & UCB1400_TC_TM_MASK)

/* UCB1400_Extra_Interrupt bits - see 12.22 in [1] */

#define	UCB1400_EI_CLPL				bit(15)
#define	UCB1400_EI_CLPR				bit(14)
#define	UCB1400_EI_CLPG				bit(13)

#endif /* UCB1400_H */
