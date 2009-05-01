/*
 * $Id$
 *
 * XScale PXA26x/PXA255/PXA250/PXA210 Memory Controller Registers
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 * [2] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Specification Update", February 2003, Order Number: 278534-012
 * [3] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     March 2003, Order Number: 278638-002
 * [4] Intel Corporation, "Intel PXA255 Processor Developer's Manual"
 *     March 2003, Order Number: 278693-001
 *
 */

#ifndef PXA2X0_MC_H
#define PXA2X0_MC_H

#include <urjtag/bitmask.h>

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

#if defined(PXA2X0_NOPXA250) && !defined(PXA2X0_NOPXA255)
#define PXA2X0_NOPXA255
#endif

#if defined(PXA2X0_NOPXA255) && !defined(PXA2X0_NOPXA260)
#define PXA2X0_NOPXA260
#endif

/* Memory Controller Registers */

#define MC_BASE         0x48000000

#ifndef __ASSEMBLY__
typedef volatile struct MC_registers
{
    uint32_t mdcnfg;
    uint32_t mdrefr;
    uint32_t msc0;
    uint32_t msc1;
    uint32_t msc2;
    uint32_t mecr;
    uint32_t __reserved1;
    uint32_t sxcnfg;
    uint32_t __reserved2;
    uint32_t sxmrs;
    uint32_t mcmem0;
    uint32_t mcmem1;
    uint32_t mcatt0;
    uint32_t mcatt1;
    uint32_t mcio0;
    uint32_t mcio1;
    uint32_t mdmrs;
    uint32_t boot_def;
#if !defined(PXA2X0_NOPXA255)
    uint32_t __reserved3[4];
    uint32_t mdmrslp;
#endif                          /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
    uint32_t __reserved4[2];
    uint32_t sa1111cr;
#endif                          /* PXA260 and above only */
} MC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define MC_pointer      ((MC_registers_t*) MC_BASE)
#endif

#define MDCNFG          MC_pointer->mdcnfg
#define MDREFR          MC_pointer->mdrefr
#define MSC0            MC_pointer->msc0
#define MSC1            MC_pointer->msc1
#define MSC2            MC_pointer->msc2
#define MECR            MC_pointer->mecr
#define SXCNFG          MC_pointer->sxcnfg
#define SXMRS           MC_pointer->sxmrs
#define MCMEM0          MC_pointer->mcmem0
#define MCMEM1          MC_pointer->mcmem1
#define MCATT0          MC_pointer->mcatt0
#define MCATT1          MC_pointer->mcatt1
#define MCIO0           MC_pointer->mcio0
#define MCIO1           MC_pointer->mcio1
#define MDMRS           MC_pointer->mdmrs
#define BOOT_DEF        MC_pointer->boot_def
#if !defined(PXA2X0_NOPXA255)
#define MDMRSLP         MC_pointer->mdmrslp
#endif /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
#define SA1111CR        MC_pointer->sa1111cr
#endif /* PXA260 and above only */
#endif /* __ASSEMBLY__ */

#define MDCNFG_OFFSET   0x00
#define MDREFR_OFFSET   0x04
#define MSC0_OFFSET     0x08
#define MSC1_OFFSET     0x0C
#define MSC2_OFFSET     0x10
#define MECR_OFFSET     0x14
#define SXCNFG_OFFSET   0x1C
#define SXMRS_OFFSET    0x24
#define MCMEM0_OFFSET   0x28
#define MCMEM1_OFFSET   0x2C
#define MCATT0_OFFSET   0x30
#define MCATT1_OFFSET   0x34
#define MCIO0_OFFSET    0x38
#define MCIO1_OFFSET    0x3C
#define MDMRS_OFFSET    0x40
#define BOOT_DEF_OFFSET 0x44
#if !defined(PXA2X0_NOPXA255)
#define MDMRSLP_OFFSET  0x58
#endif /* PXA255 and above only */
#if !defined(PXA2X0_NOPXA260)
#define SA1111CR_OFFSET 0x64
#endif /* PXA260 and above only */

/* MDCNFG bits - see Table 6-3 in [1] and D25 in [2], Table 6-3 in [3], Table 6-2 in [4] */

#define MDCNFG_DSA1111_2        URJ_BIT (28)
#define MDCNFG_DLATCH2          URJ_BIT (27)
#define MDCNFG_DTC2_MASK        URJ_BITS (25,24)
#define MDCNFG_DTC2(x)          URJ_BITS_VAL (25,24,x)
#define get_MDCNFG_DTC2(x)      URJ_BITS_GET (25,24,x)
#define MDCNFG_DNB2             URJ_BIT (23)
#define MDCNFG_DRAC2_MASK       URJ_BITS (22,21)
#define MDCNFG_DRAC2(x)         URJ_BITS_VAL (22,21,x)
#define get_MDCNFG_DRAC2(x)     URJ_BITS_GET (22,21,x)
#define MDCNFG_DCAC2_MASK       URJ_BITS (20,19)
#define MDCNFG_DCAC2(x)         URJ_BITS_VAL (20,19,x)
#define get_MDCNFG_DCAC2(x)     URJ_BITS_GET (20,19,x)
#define MDCNFG_DWID2            URJ_BIT (18)
#define MDCNFG_DE3              URJ_BIT (17)
#define MDCNFG_DE2              URJ_BIT (16)
#define MDCNFG_DSA1111_0        URJ_BIT (12)
#define MDCNFG_DLATCH0          URJ_BIT (11)
#define MDCNFG_DTC0_MASK        URJ_BITS (9,8)
#define MDCNFG_DTC0(x)          URJ_BITS_VAL (9,8,x)
#define get_MDCNFG_DTC0(x)      URJ_BITS_GET (9,8,x)
#define MDCNFG_DNB0             URJ_BIT (7)
#define MDCNFG_DRAC0_MASK       URJ_BITS (6,5)
#define MDCNFG_DRAC0(x)         URJ_BITS_VAL (6,5,x)
#define get_MDCNFG_DRAC0(x)     URJ_BITS_GET (6,5,x)
#define MDCNFG_DCAC0_MASK       URJ_BITS (4,3)
#define MDCNFG_DCAC0(x)         URJ_BITS_VAL (4,3,x)
#define get_MDCNFG_DCAC0(x)     URJ_BITS_GET (4,3,x)
#define MDCNFG_DWID0            URJ_BIT (2)
#define MDCNFG_DE1              URJ_BIT (1)
#define MDCNFG_DE0              URJ_BIT (0)

/* MDREFR bits - see Table 6-5 in [1], Table 6-6 in [3], Table 6-5 in [4] */

#define MDREFR_K2FREE           URJ_BIT (25)
#define MDREFR_K1FREE           URJ_BIT (24)
#define MDREFR_K0FREE           URJ_BIT (23)
#define MDREFR_SLFRSH           URJ_BIT (22)
#define MDREFR_APD              URJ_BIT (20)
#define MDREFR_K2DB2            URJ_BIT (19)
#define MDREFR_K2RUN            URJ_BIT (18)
#define MDREFR_K1DB2            URJ_BIT (17)
#define MDREFR_K1RUN            URJ_BIT (16)
#define MDREFR_E1PIN            URJ_BIT (15)
#define MDREFR_K0DB2            URJ_BIT (14)
#define MDREFR_K0RUN            URJ_BIT (13)
#define MDREFR_E0PIN            URJ_BIT (12)
#define MDREFR_DRI_MASK         URJ_BITS (11,0)
#define MDREFR_DRI(x)           URJ_BITS_VAL (11,0,x)
#define get_MDREFR_DRI(x)       URJ_BITS_GET (11,0,x)

/* MSC0 bits - see Table 6-21 in [1], Table 6-25 in [3], Table 6-21 in [4] */

#define MSC0_RBUFF1             URJ_BIT (31)
#define MSC0_RRR1_MASK          URJ_BITS (30,28)
#define MSC0_RRR1(x)            URJ_BITS_VAL (30,28,x)
#define get_MSC0_RRR1(x)        URJ_BITS_GET (30,28,x)
#define MSC0_RDN1_MASK          URJ_BITS (27,24)
#define MSC0_RDN1(x)            URJ_BITS_VAL (27,24,x)
#define get_MSC0_RDN1(x)        URJ_BITS_GET (27,24,x)
#define MSC0_RDF1_MASK          URJ_BITS (23,20)
#define MSC0_RDF1(x)            URJ_BITS_VAL (23,20,x)
#define get_MSC0_RDF1(x)        URJ_BITS_GET (23,20,x)
#define MSC0_RBW1               URJ_BIT (19)
#define MSC0_RT1_MASK           URJ_BITS (18,16)
#define MSC0_RT1(x)             URJ_BITS_VAL (18,16,x)
#define get_MSC0_RT1(x)         URJ_BITS_GET (18,16,x)
#define MSC0_RBUFF0             URJ_BIT (15)
#define MSC0_RRR0_MASK          URJ_BITS (14,12)
#define MSC0_RRR0(x)            URJ_BITS_VAL (14,12,x)
#define get_MSC0_RRR0(x)        URJ_BITS_GET (14,12,x)
#define MSC0_RDN0_MASK          URJ_BITS (11,9)
#define MSC0_RDN0(x)            URJ_BITS_VAL (11,8,x)
#define get_MSC0_RDN0(x)        URJ_BITS_GET (11,8,x)
#define MSC0_RDF0_MASK          URJ_BITS (7,4)
#define MSC0_RDF0(x)            URJ_BITS_VAL (7,4,x)
#define get_MSC0_RDF0(x)        URJ_BITS_GET (7,4,x)
#define MSC0_RBW0               URJ_BIT (3)
#define MSC0_RT0_MASK           URJ_BITS (2,0)
#define MSC0_RT0(x)             URJ_BITS_VAL (2,0,x)
#define get_MSC0_RT0(x)         URJ_BITS_GET (2,0,x)

/* MSC1 bits - see Table 6-21 in [1], Table 6-25 in [3], Table 6-21 in [4] */

#define MSC1_RBUFF3             URJ_BIT (31)
#define MSC1_RRR3_MASK          URJ_BITS (30,28)
#define MSC1_RRR3(x)            URJ_BITS_VAL (30,28,x)
#define get_MSC1_RRR3(x)        URJ_BITS_GET (30,28,x)
#define MSC1_RDN3_MASK          URJ_BITS (27,24)
#define MSC1_RDN3(x)            URJ_BITS_VAL (27,24,x)
#define get_MSC1_RDN3(x)        URJ_BITS_GET (27,24,x)
#define MSC1_RDF3_MASK          URJ_BITS (23,20)
#define MSC1_RDF3(x)            URJ_BITS_VAL (23,20,x)
#define get_MSC1_RDF3(x)        URJ_BITS_GET (23,20,x)
#define MSC1_RBW3               URJ_BIT (19)
#define MSC1_RT3_MASK           URJ_BITS (18,16)
#define MSC1_RT3(x)             URJ_BITS_VAL (18,16,x)
#define get_MSC1_RT3(x)         URJ_BITS_GET (18,16,x)
#define MSC1_RBUFF2             URJ_BIT (15)
#define MSC1_RRR2_MASK          URJ_BITS (14,12)
#define MSC1_RRR2(x)            URJ_BITS_VAL (14,12,x)
#define get_MSC1_RRR2(x)        URJ_BITS_GET (14,12,x)
#define MSC1_RDN2_MASK          URJ_BITS (11,9)
#define MSC1_RDN2(x)            URJ_BITS_VAL (11,8,x)
#define get_MSC1_RDN2(x)        URJ_BITS_GET (11,8,x)
#define MSC1_RDF2_MASK          URJ_BITS (7,4)
#define MSC1_RDF2(x)            URJ_BITS_VAL (7,4,x)
#define get_MSC1_RDF2(x)        URJ_BITS_GET (7,4,x)
#define MSC1_RBW2               URJ_BIT (3)
#define MSC1_RT2_MASK           URJ_BITS (2,0)
#define MSC1_RT2(x)             URJ_BITS_VAL (2,0,x)
#define get_MSC1_RT2(x)         URJ_BITS_GET (2,0,x)

/* MSC2 bits - see Table 6-21 in [1], Table 6-25 in [3], Table 6-21 in [4] */

#define MSC2_RBUFF5             URJ_BIT (31)
#define MSC2_RRR5_MASK          URJ_BITS (30,28)
#define MSC2_RRR5(x)            URJ_BITS_VAL (30,28,x)
#define get_MSC2_RRR5(x)        URJ_BITS_GET (30,28,x)
#define MSC2_RDN5_MASK          URJ_BITS (27,24)
#define MSC2_RDN5(x)            URJ_BITS_VAL (27,24,x)
#define get_MSC2_RDN5(x)        URJ_BITS_GET (27,24,x)
#define MSC2_RDF5_MASK          URJ_BITS (23,20)
#define MSC2_RDF5(x)            URJ_BITS_VAL (23,20,x)
#define get_MSC2_RDF5(x)        URJ_BITS_GET (23,20,x)
#define MSC2_RBW5               URJ_BIT (19)
#define MSC2_RT5_MASK           URJ_BITS (18,16)
#define MSC2_RT5(x)             URJ_BITS_VAL (18,16,x)
#define get_MSC2_RT5(x)         URJ_BITS_GET (18,16,x)
#define MSC2_RBUFF4             URJ_BIT (15)
#define MSC2_RRR4_MASK          URJ_BITS (14,12)
#define MSC2_RRR4(x)            URJ_BITS_VAL (14,12,x)
#define get_MSC2_RRR4(x)        URJ_BITS_GET (14,12,x)
#define MSC2_RDN4_MASK          URJ_BITS (11,9)
#define MSC2_RDN4(x)            URJ_BITS_VAL (11,8,x)
#define get_MSC2_RDN4(x)        URJ_BITS_GET (11,8,x)
#define MSC2_RDF4_MASK          URJ_BITS (7,4)
#define MSC2_RDF4(x)            URJ_BITS_VAL (7,4,x)
#define get_MSC2_RDF4(x)        URJ_BITS_GET (7,4,x)
#define MSC2_RBW4               URJ_BIT (3)
#define MSC2_RT4_MASK           URJ_BITS (2,0)
#define MSC2_RT4(x)             URJ_BITS_VAL (2,0,x)
#define get_MSC2_RT4(x)         URJ_BITS_GET (2,0,x)

/* MECR bits - see Table 6-27 in [1], Table 6-31 in [3], Table 6-27 in [4] */

#define MECR_CIT                URJ_BIT (1)
#define MECR_NOS                URJ_BIT (0)

/* SXCNFG bits - see Table 6-13 in [1], Table 6-14 in [3], Table 6-13 in [4] */

#define SXCNFG_SXLATCH2         URJ_BIT (30)
#define SXCNFG_SXTP2_MASK       URJ_BITS (29,28)
#define SXCNFG_SXTP2(x)         URJ_BITS_VAL (29,28,x)
#define get_SXCNFG_SXTP2(x)     URJ_BITS_GET (29,28,x)
#define SXCNFG_SXCA2_MASK       URJ_BITS (27,26)
#define SXCNFG_SXCA2(x)         URJ_BITS_VAL (27,26,x)
#define get_SXCNFG_SXCA2(x)     URJ_BITS_GET (27,26,x)
#define SXCNFG_SXRA2_MASK       URJ_BITS (25,24)
#define SXCNFG_SXRA2(x)         URJ_BITS_VAL (25,24,x)
#define get_SXCNFG_SXRA2(x)     URJ_BITS_GET (25,24,x)
#define SXCNFG_SXRL2_MASK       URJ_BITS (23,21)
#define SXCNFG_SXRL2(x)         URJ_BITS (23,21,x)
#define SXCNFG_SXCL2_MASK       URJ_BITS (20,18)
#define SXCNFG_SXCL2(x)         URJ_BITS_VAL (20,18,x)
#define get_SXCNFG_SXCL2(x)     URJ_BITS_GET (20,18,x)
#define SXCNFG_SXEN2_MASK       URJ_BITS (17,16)
#define SXCNFG_SXEN2(x)         URJ_BITS_VAL (17,16,x)
#define get_SXCNFG_SXEN2(x)     URJ_BITS_GET (17,16,x)
#define SXCNFG_SXLATCH0         URJ_BIT (14)
#define SXCNFG_SXTP0_MASK       URJ_BITS (13,12)
#define SXCNFG_SXTP0(x)         URJ_BITS_VAL (13,12,x)
#define get_SXCNFG_SXTP0(x)     URJ_BITS_GET (13,12,x)
#define SXCNFG_SXCA0_MASK       URJ_BITS (11,10)
#define SXCNFG_SXCA0(x)         URJ_BITS_VAL (11,10,x)
#define get_SXCNFG_SXCA0(x)     URJ_BITS_GET (11,10,x)
#define SXCNFG_SXRA0_MASK       URJ_BITS (9,8)
#define SXCNFG_SXRA0(x)         URJ_BITS_VAL (9,8,x)
#define get_SXCNFG_SXRA0(x)     URJ_BITS_GET (9,8,x)
#define SXCNFG_SXRL0_MASK       URJ_BITS (7,5)
#define SXCNFG_SXRL0(x)         URJ_BITS (7,5,x)
#define SXCNFG_SXCL0_MASK       URJ_BITS (4,2)
#define SXCNFG_SXCL0(x)         URJ_BITS_VAL (4,2,x)
#define get_SXCNFG_SXCL0(x)     URJ_BITS_GET (4,2,x)
#define SXCNFG_SXEN0_MASK       URJ_BITS (1,0)
#define SXCNFG_SXEN0(x)         URJ_BITS_VAL (1,0,x)
#define get_SXCNFG_SXEN0(x)     URJ_BITS_GET (1,0,x)

/* SXMRS bits - see Table 6-16 in [1], Table 6-17 in [3], Table 6-16 in [4] */

#define SXMRS_SXMRS2_MASK       URJ_BITS (30,16)
#define SXMRS_SXMRS2(x)         URJ_BITS_VAL (30,16,x)
#define get_SXMRS_SXMRS2(x)     URJ_BITS_GET (30,16,x)
#define SXMRS_SXMRS0_MASK       URJ_BITS (14,0)
#define SXMRS_SXMRS0(x)         URJ_BITS_VAL (14,0,x)
#define get_SXMRS_SXMRS0(x)     URJ_BITS_GET (14,0,x)

/* MCMEMx bits - see Table 6-23 in [1], Table 6-27 in [3], Table 6-23 in [4] */

#define MCMEM_HOLD_MASK         URJ_BITS (19,14)
#define MCMEM_HOLD(x)           URJ_BITS_VAL (19,14,x)
#define get_MCMEM_HOLD(x)       URJ_BITS_GET (19,14,x)
#define MCMEM_ASST_MASK         URJ_BITS (11,7)
#define MCMEM_ASST(x)           URJ_BITS_VAL (11,7,x)
#define get_MCMEM_ASST(x)       URJ_BITS_GET (11,7,x)
#define MCMEM_SET_MASK          URJ_BITS (6,0)
#define MCMEM_SET(x)            URJ_BITS_VAL (6,0,x)
#define get_MCMEM_SET(x)        URJ_BITS_GET (6,0,x)

/* MCATTx bits - see Table 6-24 in [1], Table 6-28 in [3], Table 6-24 in [4] */

#define MCATT_HOLD_MASK         URJ_BITS (19,14)
#define MCATT_HOLD(x)           URJ_BITS_VAL (19,14,x)
#define get_MCATT_HOLD(x)       URJ_BITS_GET (19,14,x)
#define MCATT_ASST_MASK         URJ_BITS (11,7)
#define MCATT_ASST(x)           URJ_BITS_VAL (11,7,x)
#define get_MCATT_ASST(x)       URJ_BITS_GET (11,7,x)
#define MCATT_SET_MASK          URJ_BITS (6,0)
#define MCATT_SET(x)            URJ_BITS_VAL (6,0,x)
#define get_MCATT_SET(x)        URJ_BITS_GET (6,0,x)

/* MCIOx bits - see Table 6-25 in [1], Table 6-29 in [3], Table 6-25 in [4] */

#define MCIO_HOLD_MASK          URJ_BITS (19,14)
#define MCIO_HOLD(x)            URJ_BITS_VAL (19,14,x)
#define get_MCIO_HOLD(x)        URJ_BITS_GET (19,14,x)
#define MCIO_ASST_MASK          URJ_BITS (11,7)
#define MCIO_ASST(x)            URJ_BITS_VAL (11,7,x)
#define get_MCIO_ASST(x)        URJ_BITS_GET (11,7,x)
#define MCIO_SET_MASK           URJ_BITS (6,0)
#define MCIO_SET(x)             URJ_BITS_VAL (6,0,x)
#define get_MCIO_SET(x)         URJ_BITS_GET (6,0,x)

/* MDMRS bits - see Table 6-4 in [1], Table 6-4 in [3], Table 6-3 in [4] */

#define MDMRS_MDMRS2_MASK       URJ_BITS (30,23)
#define MDMRS_MDMRS2(x)         URJ_BITS_VAL (30,23,x)
#define get_MDMRS_MDMRS2(x)     URJ_BITS_GET (30,23,x)
#define MDMRS_MDCL2_MASK        URJ_BITS (22,20)
#define MDMRS_MDCL2(x)          URJ_BITS_VAL (22,20,x)
#define get_MDMRS_MDCL2(x)      URJ_BITS_GET (22,20,x)
#define MDMRS_MDADD2            URJ_BIT (19)
#define MDMRS_MDBL2_MASK        URJ_BITS (18,16)
#define MDMRS_MDBL2(x)          URJ_BITS_VAL (18,16,x)
#define get_MDMRS_MDBL2(x)      URJ_BITS_GET (18,16,x)
#define MDMRS_MDMRS0_MASK       URJ_BITS (14,7)
#define MDMRS_MDMRS0(x)         URJ_BITS_VAL (14,7,x)
#define get_MDMRS_MDMRS0(x)     URJ_BITS_GET (14,7,x)
#define MDMRS_MDCL0_MASK        URJ_BITS (6,4)
#define MDMRS_MDCL0(x)          URJ_BITS_VAL (6,4,x)
#define get_MDMRS_MDCL0(x)      URJ_BITS_GET (6,4,x)
#define MDMRS_MDADD0            URJ_BIT (3)
#define MDMRS_MDBL0_MASK        URJ_BITS (2,0)
#define MDMRS_MDBL0(x)          URJ_BITS_VAL (2,0,x)
#define get_MDMRS_MDBL0(x)      URJ_BITS_GET (2,0,x)

/* BOOT_DEF bits - see Table 6-37 in [1], Table 6-40 in [3], Table 6-37 in [4] */

#define BOOT_DEF_PKG_TYPE       URJ_BIT (3)
#define BOOT_DEF_BOOT_SEL_MASK  URJ_BITS (2,0)
#define BOOT_DEF_BOOT_SEL(x)    URJ_BITS_VAL (2,0,x)
#define get_BOOT_DEF_BOOT_SEL(x)        URJ_BITS_GET (2,0,x)

#if !defined(PXA2X0_NOPXA255)
/* MDMRSLP bits - see Table 6-5 in [3], Table 6-4 in [4] */

#define MDMRSLP_MDLPEN2         URJ_BIT (31)
#define MDMRSLP_MDMRSLP2_MASK   URJ_BITS (30,16)
#define MDMRSLP_MDMRSLP2(x)     URJ_BITS_VAL (30,16,x)
#define get_MDMRSLP_MDMRSLP2(x) URJ_BITS_GET (30,16,x)
#define MDMRSLP_MDLPEN0         URJ_BIT (15)
#define MDMRSLP_MDMRSLP0_MASK   URJ_BITS (14,0)
#define MDMRSLP_MDMRSLP0(x)     URJ_BITS_VAL (14,0,x)
#define get_MDMRSLP_MDMRSLP0(x) URJ_BITS_GET (14,0,x)
#endif /* PXA255 and above only */

#if !defined(PXA2X0_NOPXA260)
/* SA1111CR bits - see Table 6-24 in [3] */

#define SA1111CR_SA1111_5       URJ_BIT (5)
#define SA1111CR_SA1111_4       URJ_BIT (4)
#define SA1111CR_SA1111_3       URJ_BIT (3)
#define SA1111CR_SA1111_2       URJ_BIT (2)
#define SA1111CR_SA1111_1       URJ_BIT (1)
#define SA1111CR_SA1111_0       URJ_BIT (0)
#endif /* PXA260 and above only */

#endif /* PXA2X0_MC_H */
