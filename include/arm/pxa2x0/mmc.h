/*
 * $Id$
 *
 * XScale PXA250/PXA210 MMC Controller Registers
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
