/*
 * $Id$
 *
 * XScale PXA26x/PXA250/PXA210 MMC Controller Registers
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
 * [2] Intel Corporation, "Intel PXA26x Processor Family Developer's Manual",
 *     October 2002, Order Number: 278638-001
 *
 */

#ifndef	PXA2X0_MMC_H
#define	PXA2X0_MMC_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* MMC Controller Registers */

#define	MMC_BASE	0x41100000

#if LANGUAGE == C
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
} MMC_registers_t;

#ifdef PXA2X0_UNMAPPED
#define	MMC_pointer	((MMC_registers_t*) MMC_BASE)
#endif

#define	MMC_STRPCL			MMC_pointer->mmc_strpcl
#define	MMC_STAT			MMC_pointer->mmc_stat
#define	MMC_CLKRT			MMC_pointer->mmc_clkrt
#define	MMC_SPI				MMC_pointer->mmc_spi
#define	MMC_CMDAT			MMC_pointer->mmc_cmdat
#define	MMC_RESTO			MMC_pointer->mmc_resto
#define	MMC_RDTO			MMC_pointer->mmc_rdto
#define	MMC_BLKLEN			MMC_pointer->mmc_blklen
#define	MMC_NOB				MMC_pointer->mmc_nob
#define	MMC_PRTBUF			MMC_pointer->mmc_prtbuf
#define	MMC_I_MASK			MMC_pointer->mmc_i_mask
#define	MMC_I_REG			MMC_pointer->mmc_i_reg
#define	MMC_CMD				MMC_pointer->mmc_cmd
#define	MMC_ARGH			MMC_pointer->mmc_argh
#define	MMC_ARGL			MMC_pointer->mmc_argl
#define	MMC_RES				MMC_pointer->mmc_res
#define	MMC_RXFIFO			MMC_pointer->mmc_rxfifo
#define	MMC_TXFIFO			MMC_pointer->mmc_txfifo
#endif /* LANGUAGE == C */

#define	MMC_STRPCL_OFFSET		0x00
#define	MMC_STAT_OFFSET			0x04
#define	MMC_CLKRT_OFFSET		0x08
#define	MMC_SPI_OFFSET			0x0C
#define	MMC_CMDAT_OFFSET		0x10
#define	MMC_RESTO_OFFSET		0x14
#define	MMC_RDTO_OFFSET			0x18
#define	MMC_BLKLEN_OFFSET		0x1C
#define	MMC_NOB_OFFSET			0x20
#define	MMC_PRTBUF_OFFSET		0x24
#define	MMC_I_MASK_OFFSET		0x28
#define	MMC_I_REG_OFFSET		0x2C
#define	MMC_CMD_OFFSET			0x30
#define	MMC_ARGH_OFFSET			0x34
#define	MMC_ARGL_OFFSET			0x38
#define	MMC_RES_OFFSET			0x3C
#define	MMC_RXFIFO_OFFSET		0x40
#define	MMC_TXFIFO_OFFSET		0x44

/* MMC_STRPCL bits - see Table 15-6 in [1], Table 15-6 in [2] */

#define	MMC_STRPCL_STRPCL_MASK		bits(1,0)
#define	MMC_STRPCL_STRPCL(x)		bits_val(1,0,x)
#define	get_MMC_STRPCL_STRPCL(x)	bits_get(1,0,x)

/* MMC_STAT bits - see Table 15-7 in [1], Table 15-7 in [2] */

#define	MMC_STAT_END_CMD_RES		bit(13)
#define	MMC_STAT_PRG_DONE		bit(12)
#define	MMC_STAT_DATA_TRAN_DONE		bit(11)
#define	MMC_STAT_CLK_EN			bit(8)
#define	MMC_STAT_RECV_FIFO_FULL		bit(7)
#define	MMC_STAT_XMIT_FIFO_EMPTY	bit(6)
#define	MMC_STAT_RES_CRC_ERR		bit(5)
#define	MMC_STAT_SPI_READ_ERROR_TOKEN	bit(4)
#define	MMC_STAT_CRC_READ_ERROR		bit(3)
#define	MMC_STAT_CRC_WRITE_ERROR	bit(2)
#define	MMC_STAT_TIME_OUT_RESPONSE	bit(1)
#define	MMC_STAT_READ_TIME_OUT		bit(0)

/* MMC_CLKRT bits - see Table 15-8 in [1], Table 15-8 in [2] */

#define	MMC_CLKRT_CLK_RATE_MASK		bits(2,0)
#define	MMC_CLKRT_CLK_RATE(x)		bits_val(2,0,x)
#define	get_MMC_CLKRT_CLK_RATE(x)	bits_get(2,0,x)

/* MMC_SPI bits - see Table 15-9 in [1], Table 15-9 in [2] */

#define	MMC_SPI_SPI_CS_ADDRESS		bit(3)
#define	MMC_SPI_SPI_CS_EN		bit(2)
#define	MMC_SPI_CRC_ON			bit(1)
#define	MMC_SPI_SPI_EN			bit(0)

/* MMC_CMDAT bits - see Table 15-10 in [1], Table 15-10 in [2] */

#define	MMC_CMDAT_MMC_DMA_EN		bit(7)
#define	MMC_CMDAT_INIT			bit(6)
#define	MMC_CMDAT_BUSY			bit(5)
#define	MMC_CMDAT_STREAM_BLOCK		bit(4)
#define	MMC_CMDAT_WRITE_READ		bit(3)
#define	MMC_CMDAT_DATA_EN		bit(2)
#define	MMC_CMDAT_RESPONSE_FORMAT_MASK	bits(1,0)
#define	MMC_CMDAT_RESPONSE_FORMAT(x)	bits_val(1,0,x)
#define	get_MMC_CMDAT_RESPONSE_FORMAT(x)	bits_get(1,0,x)

/* MMC_RESTO bits - see Table 15-11 in [1], Table 15-11 in [2] */

#define	MMC_RESTO_RES_TO_MASK		bits(6,0)
#define	MMC_RESTO_RES_TO(x)		bits_val(6,0,x)
#define	get_MMC_RESTO_RES_TO(x)		bits_get(6,0,x)

/* MMC_RDTO bits - see Table 15-12 in [1], Table 15-12 in [2] */

#define	MMC_RDTO_READ_TO_MASK		bits(15,0)
#define	MMC_RDTO_READ_TO(x)		bits_val(15,0,x)
#define	get_MMC_RDTO_READ_TO(x)		bits_get(15,0,x)

/* MMC_BLKLEN bits - see Table 15-13 in [1], Table 15-13 in [2] */

#define	MMC_BLKLEN_BLK_LEN_MASK		bits(9,0)
#define	MMC_BLKLEN_BLK_LEN(x)		bits_val(9,0,x)
#define	get_MMC_BLKLEN_BLK_LEN(x)	bits_get(9,0,x)

/* MMC_NOB bits - see Table 15-14 in [1], Table 15-14 in [2] */

#define	MMC_NOB_MMC_NOB_MASK		bits(15,0)
#define	MMC_NOB_MMC_NOB(x)		bits_val(15,0,x)
#define	get_MMC_NOB_MMC_NOB(x)		bits_get(15,0,x)

/* MMC_PRTBUF bits - see Table 15-15 in [1], Table 15-15 in [2] */

#define	MMC_PRTBUF_BUF_PART_FULL	bit(0)

/* MMC_I_MASK bits - see Table 15-16 in [1], Table 15-16 in [2] */

#define	MMC_I_MASK_TXFIFO_WR_REQ	bit(6)
#define	MMC_I_MASK_RXFIFO_RD_REQ	bit(5)
#define	MMC_I_MASK_CLK_IS_OFF		bit(4)
#define	MMC_I_MASK_STOP_CMD		bit(3)
#define	MMC_I_MASK_END_CMD_RES		bit(2)
#define	MMC_I_MASK_PRG_DONE		bit(1)
#define	MMC_I_MASK_DATA_TRAN_DONE	bit(0)

/* MMC_I_REG bits - see Table 15-17 in [1], Table 15-17 in [2] */

#define	MMC_I_REG_TXFIFO_WR_REQ		bit(6)
#define	MMC_I_REG_RXFIFO_RD_REQ		bit(5)
#define	MMC_I_REG_CLK_IS_OFF		bit(4)
#define	MMC_I_REG_STOP_CMD		bit(3)
#define	MMC_I_REG_END_CMD_RES		bit(2)
#define	MMC_I_REG_PRG_DONE		bit(1)
#define	MMC_I_REG_DATA_TRAN_DONE	bit(0)

/* MMC_CMD bits - see Table 15-18 in [1], Table 15-18 in [2] */

#define	MMC_CMD_CMD_INDEX_MASK		bits(5,0)
#define	MMC_CMD_CMD_INDEX(x)		bits_val(5,0,x)
#define	get_MMC_CMD_CMD_INDEX(x)	bits_get(5,0,x)

/* MMC commands (for MMC_CMD) - see Table 15-19 in [1], Table 15-19 in [2] */

#define MMC_CMD_GO_IDLE_STATE		MMC_CMD_CMD_INDEX(0)
#define MMC_CMD_SEND_OP_COND		MMC_CMD_CMD_INDEX(1)
#define MMC_CMD_ALL_SEND_CID		MMC_CMD_CMD_INDEX(2)
#define MMC_CMD_SET_RELATIVE_ADDR	MMC_CMD_CMD_INDEX(3)
#define MMC_CMD_SET_DSR			MMC_CMD_CMD_INDEX(4)
#define MMC_CMD_SELECT_DESELECT_CARD	MMC_CMD_CMD_INDEX(7)
#define MMC_CMD_SEND_CSD		MMC_CMD_CMD_INDEX(9)
#define MMC_CMD_SEND_CID		MMC_CMD_CMD_INDEX(10)
#define MMC_CMD_READ_DAT_UNTIL_STOP	MMC_CMD_CMD_INDEX(11)
#define MMC_CMD_STOP_TRANSMISSION	MMC_CMD_CMD_INDEX(12)
#define MMC_CMD_SEND_STATUS		MMC_CMD_CMD_INDEX(13)
#define MMC_CMD_GO_INACTIVE_STATE	MMC_CMD_CMD_INDEX(15)
#define MMC_CMD_SET_BLOCKLEN		MMC_CMD_CMD_INDEX(16)
#define MMC_CMD_READ_SINGLE_BLOCK	MMC_CMD_CMD_INDEX(17)
#define MMC_CMD_READ_MULTIPLE_BLOCK	MMC_CMD_CMD_INDEX(18)
#define MMC_CMD_WRITE_DAT_UNTIL_STOP	MMC_CMD_CMD_INDEX(20)
#define MMC_CMD_WRITE_BLOCK		MMC_CMD_CMD_INDEX(24)	
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	MMC_CMD_CMD_INDEX(25)
#define MMC_CMD_PROGRAM_CID		MMC_CMD_CMD_INDEX(26)
#define MMC_CMD_PROGRAM_CSD		MMC_CMD_CMD_INDEX(27)
#define MMC_CMD_SET_WRITE_PROT		MMC_CMD_CMD_INDEX(28)
#define MMC_CMD_CLR_WRITE_PROT		MMC_CMD_CMD_INDEX(29)
#define MMC_CMD_SEND_WRITE_PROT		MMC_CMD_CMD_INDEX(30)
#define MMC_CMD_TAG_SECTOR_START	MMC_CMD_CMD_INDEX(32)
#define MMC_CMD_TAG_SECTOR_END		MMC_CMD_CMD_INDEX(33)
#define MMC_CMD_UNTAG_SECTOR		MMC_CMD_CMD_INDEX(34)
#define MMC_CMD_TAG_ERASE_GROUP_START	MMC_CMD_CMD_INDEX(35)
#define MMC_CMD_TAG_ERASE_GROUP_END	MMC_CMD_CMD_INDEX(36)
#define MMC_CMD_UNTAG_ERASE_GROUP	MMC_CMD_CMD_INDEX(37)
#define MMC_CMD_ERASE			MMC_CMD_CMD_INDEX(38)
#define MMC_CMD_FAST_IO			MMC_CMD_CMD_INDEX(39)
#define MMC_CMD_GO_IRQ_STATE		MMC_CMD_CMD_INDEX(40)
#define MMC_CMD_LOCK_UNLOCK		MMC_CMD_CMD_INDEX(42)
#define MMC_CMD_APP_CMD			MMC_CMD_CMD_INDEX(55)
#define MMC_CMD_GEN_CMD			MMC_CMD_CMD_INDEX(56)
#define MMC_CMD_READ_OCR		MMC_CMD_CMD_INDEX(58)
#define MMC_CMD_CRC_ON_OFF		MMC_CMD_CMD_INDEX(59)

/* MMC_ARGH bits - see Table 15-20 in [1], Table 15-20 in [2] */

#define	MMC_ARGH_ARG_H_MASK		bits(15,0)
#define	MMC_ARGH_ARG_H(x)		bits_val(15,0,x)
#define	get_MMC_ARGH_ARG_H(x)		bits_get(15,0,x)

/* MMC_ARGL bits - see Table 15-21 in [1], Table 15-21 in [2] */

#define	MMC_ARGL_ARG_L_MASK		bits(15,0)
#define	MMC_ARGL_ARG_L(x)		bits_val(15,0,x)
#define	get_MMC_ARGL_ARG_L(x)		bits_get(15,0,x)

/* MMC_RES bits - see Table 15-22 in [1], Table 15-22 in [2] */

#define	MMC_RES_RESPONSE_DATA_MASK	bits(15,0)
#define	MMC_RES_RESPONSE_DATA(x)	bits_val(15,0,x)
#define	get_MMC_RES_RESPONSE_DATA(x)	bits_get(15,0,x)

/* MMC_RXFIFO bits - see Table 15-23 in [1], Table 15-23 in [2] */

#define	MMC_RXFIFO_READ_DATA_MASK	bits(7,0)
#define	MMC_RXFIFO_READ_DATA(x)		bits_val(7,0,x)
#define	get_MMC_RXFIFO_READ_DATA(x)	bits_get(7,0,x)

/* MMC_TXFIFO bits - see Table 15-24 in [1], Table 15-24 in [2] */

#define	MMC_TXFIFO_WRITE_DATA_MASK	bits(7,0)
#define	MMC_TXFIFO_WRITE_DATA(x)	bits_val(7,0,x)
#define	get_MMC_TXFIFO_WRITE_DATA(x)	bits_get(7,0,x)

#endif /* PXA2X0_MMC_H */
