/*
 * $Id$
 *
 * AC97 Registers
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
