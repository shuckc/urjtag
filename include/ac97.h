/*
 * $Id$
 *
 * AC97 Registers
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
 * [1] Intel Corporation, "AC'97 Component Specification Revision 2.3
 *     Rev 1.0", April 2002
 *
 */

#ifndef	AC97_H
#define	AC97_H

/* Baseline Audio Register Set - see 5.7 in [1] */

#define	AC97_Reset			0x00
#define	AC97_Master_Volume		0x02
#define	AC97_Aux_Out_Volume		0x04
#define	AC97_Mono_Volume		0x06
#define	AC97_Master_Tone		0x08
#define	AC97_PC_Beep_Volume		0x0A
#define	AC97_Phone_Volume		0x0C
#define	AC97_Mic_Volume			0x0E
#define	AC97_Line_In_Volume		0x10
#define	AC97_CD_Volume			0x12
#define	AC97_Video_Volume		0x14
#define	AC97_Aux_In_Volume		0x16
#define	AC97_PCM_Out_Volume		0x18
#define	AC97_Record_Select		0x1A
#define	AC97_Record_Gain		0x1C
#define	AC97_Record_Gain_Mic		0x1E
#define	AC97_General_Purpose		0x20
#define	AC97_3D_Control			0x22
#define	AC97_Audio_Int_and_Paging	0x24
#define	AC97_Powerdown_Ctrl_Stat	0x26

/* Extended Audio Register Set - see 5.8 in [1] */

#define	AC97_Extended_Audio_ID		0x28
#define	AC97_Extended_Audio_Stat_Ctrl	0x2A
#define	AC97_PCM_Front_DAC_Rate		0x2C
#define	AC97_PCM_Surr_DAC_Rate		0x2E
#define	AC97_PCM_LFE_DAC_Rate		0x30
#define	AC97_PCM_L_R_ADC_Rate		0x32
#define	AC97_Mic_ADC_Rate		0x34
#define	AC97_Center_LFE_Volume		0x36
#define	AC97_Surround_Volume		0x38
#define	AC97_S_PDIF_Control		0x3A

#define	AC97_Vendor_ID1			0x7C
#define	AC97_Vendor_ID2			0x7E

#endif /* AC97_H */
