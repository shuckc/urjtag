/*
 * $Id$
 *
 * Manufacturer's Identification Code declarations
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
 * [1] JEDEC Solid State Technology Association, "Standard Manufacturer's
 *     Identification Code", September 2001, Order Number: JEP106-K
 *
 */

#ifndef	STD_MIC_H
#define	STD_MIC_H

/* Manufacturer's Identification Code - see Table 1 in [1] */

#define	STD_MIC_AMD		0x01
#define	STD_MICN_AMD		"AMD"
#define	STD_MIC_AMI		0x02
#define	STD_MICN_AMI		"AMI"
#define	STD_MIC_FAIRCHILD	0x83
#define	STD_MICN_FAIRCHILD	"Fairchild"
#define	STD_MIC_FUJITSU		0x04
#define	STD_MICN_FUJITSU	"Fujitsu"
#define	STD_MIC_GTE		0x85
#define	STD_MICN_GTE		"GTE"
#define	STD_MIC_HARRIS		0x86
#define	STD_MICN_HARRIS		"Harris"
#define	STD_MIC_HITACHI		0x07
#define	STD_MICN_HITACHI	"Hitachi"
#define	STD_MIC_INMOS		0x08
#define	STD_MICN_INMOS		"Inmos"
#define	STD_MIC_INTEL		0x89
#define	STD_MICN_INTEL		"Intel"
/* TODO */

#endif /* STD_MIC_H */
