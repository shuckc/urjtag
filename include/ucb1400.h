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

#endif /* UCB1400_H */
