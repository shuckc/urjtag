/*
 * $Id$
 *
 * XScale PXA250/PXA210 MMC Controller Registers
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
 * [1] Intel Corporation, "Intel PXA250 and PXA210 Application Processors
 *     Developer's Manual", February 2002, Order Number: 278522-001
 *
 */

#ifndef	PXA2X0_MMC_H
#define	PXA2X0_MMC_H

#ifndef uint32_t
typedef	unsigned int	uint32_t;
#endif

/* MMC Controller Registers */

#define	MMC_BASE	0x41100000

typedef volatile struct MMC_registers {
	uint32_t mmc_strpcl;
	uint32_t mmc_stat;
	uint32_t mmc_clkrt;
	uint32_t mmc_spi;
	uint32_t mmc_cmdat;
	uint32_t mmc_resto;
	uint32_t mmc_rdto;
	uint32_t mmc_blklen;
	uint32_t mmc_nob;
	uint32_t mmc_prtbuf;
	uint32_t mmc_i_mask;
	uint32_t mmc_i_reg;
	uint32_t mmc_cmd;
	uint32_t mmc_argh;
	uint32_t mmc_argl;
	uint32_t mmc_res;
	uint32_t mmc_rxfifo;
	uint32_t mmc_txfifo;
} MMC_registers;

#ifndef MMC_pointer
#define	MMC_pointer	((MMC_registers*) MMC_BASE)
#endif

#define	MMC_STRPCL	MMC_pointer->mmc_strpcl
#define	MMC_STAT	MMC_pointer->mmc_stat
#define	MMC_CLKRT	MMC_pointer->mmc_clkrt
#define	MMC_SPI		MMC_pointer->mmc_spi
#define	MMC_CMDAT	MMC_pointer->mmc_cmdat
#define	MMC_RESTO	MMC_pointer->mmc_resto
#define	MMC_RDTO	MMC_pointer->mmc_rdto
#define	MMC_BLKLEN	MMC_pointer->mmc_blklen
#define	MMC_NOB		MMC_pointer->mmc_nob
#define	MMC_PRTBUF	MMC_pointer->mmc_prtbuf
#define	MMC_I_MASK	MMC_pointer->mmc_i_mask
#define	MMC_I_REG	MMC_pointer->mmc_i_reg
#define	MMC_CMD		MMC_pointer->mmc_cmd
#define	MMC_ARGH	MMC_pointer->mmc_argh
#define	MMC_ARGL	MMC_pointer->mmc_argl
#define	MMC_RES		MMC_pointer->mmc_res
#define	MMC_RXFIFO	MMC_pointer->mmc_rxfifo
#define	MMC_TXFIFO	MMC_pointer->mmc_txfifo

#endif	/* PXA2X0_MMC_H */
