/*
 * $Id$
 *
 * Common Flash Memory Interface (CFI) Declarations
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
 * [1] Advanced Micro Devices, "Common Flash Memory Interface Specification Release 2.0",
 *     December 1, 2001
 * [2] Advanced Micro Devices, "Common Flash Memory Interface Publication 100 Vendor & Device
 *     ID Code Assignments", December 1, 2001, Volume Number: 96.1
 * [3] Intel Corporation, "Common Flash Interface (CFI) and Command Sets Application Note 646",
 *     April 2000, Order Number: 292204-004
 *
 */

#ifndef	FLASH_CFI_H
#define	FLASH_CFI_H

#include <common.h>

#if LANGUAGE == C
#include <stdint.h>
#endif

/* CFI Query Identification String - see Table 3.3.2 in [1] */

#define	CFI_QUERY_ID_OFFSET		0x10
#define	PRI_VENDOR_ID_OFFSET		0x13
#define	PRI_VENDOR_TABLE_ADR_OFFSET	0x15
#define	ALT_VENDOR_ID_OFFSET		0x17
#define	ALT_VENDOR_TABLE_ADR_OFFSET	0x19

/* Vendor Command Set & Control Interface ID Codes - see Section 1. in [2] */

#define	CFI_VENDOR_NULL			0x0000
#define	CFI_VENDOR_INTEL_SCS		0x0001		/* see 1.0 in [3] */
#define	CFI_VENDOR_AMD_SCS		0x0002
#define	CFI_VENDOR_INTEL_BCS		0x0003		/* see 1.0 in [3] */
#define	CFI_VENDOR_AMD_ESC		0x0004
#define	CFI_VENDOR_MITSUBISHI_SCS	0x0100
#define	CFI_VENDOR_MITSUBISHI_ECS	0x0101
#define	CFI_VENDOR_SST_PWCS		0x0102

/* CFI Query System Interface Information - see Table 3.3.3 in [1] */

#define	VCC_MIN_WEV_OFFSET		0x1B		/* Vcc Logic Supply Minimum Write/Erase voltage */
#define	VCC_MAX_WEV_OFFSET		0x1C		/* Vcc Logic Supply Maximum Write/Erase voltage */
#define	VPP_MIN_WEV_OFFSET		0x1D		/* Vpp [Programming] Supply Minimum Write/Erase voltage */
#define	VPP_MAX_WEV_OFFSET		0x1E		/* Vpp [Programming] Supply Maximum Write/Erase voltage */
#define	TYP_SINGLE_WRITE_TIMEOUT_OFFSET	0x1F		/* Typical timeout per single byte/word write */
#define	TYP_BUFFER_WRITE_TIMEOUT_OFFSET	0x20		/* Typical timeout for minimum-size buffer write */
#define	TYP_BLOCK_ERASE_TIMEOUT_OFFSET	0x21		/* Typical timeout per individual block erase */
#define	TYP_CHIP_ERASE_TIMEOUT_OFFSET	0x22		/* Typical timeout for full chip erase */
#define	MAX_SINGLE_WRITE_TIMEOUT_OFFSET	0x23		/* Maximum timeout for byte/word write */
#define	MAX_BUFFER_WRITE_TIMEOUT_OFFSET	0x24		/* Maximum timeout for buffer write */
#define	MAX_BLOCK_ERASE_TIMEOUT_OFFSET	0x25		/* Maximum timeout per individual block erase */
#define	MAX_CHIP_ERASE_TIMEOUT_OFFSET	0x26		/* Maximum timeout for chip erase */

#if LANGUAGE == C
typedef struct cfi_query_system_interface_information {
	uint16_t vcc_min_wev;				/* in mV */
	uint16_t vcc_max_wev;				/* in mV */
	uint16_t vpp_min_wev;				/* in mV, 0 - no Vpp pin is present */
	uint16_t vpp_max_wev;				/* in mV, 0 - no Vpp pin is present */
	uint32_t typ_single_write_timeout;		/* in us */
	uint32_t typ_buffer_write_timeout;		/* in us, 0 - not supported */
	uint32_t typ_block_erase_timeout;		/* in ms */
	uint32_t typ_chip_erase_timeout;		/* in ms, 0 - not supported */
	uint32_t max_single_write_timeout;		/* in us */
	uint32_t max_buffer_write_timeout;		/* in us, 0 - not supported */
	uint32_t max_block_erase_timeout;		/* in ms */
	uint32_t max_chip_erase_timeout;		/* in ms, 0 - not supported */
} cfi_query_system_interface_information;
#endif /* LANGUAGE == C */

/* Device Geometry Definition - see Table 3.3.4 in [1] */

#define	DEVICE_SIZE_OFFSET		0x27		/* Device Size */
#define	FLASH_DEVICE_INTERFACE_OFFSET	0x28		/* Flash Device Interface description */
#define	MAX_BYTES_WRITE_OFFSET		0x2A		/* Maximum number of bytes in multi-byte write */
#define	NUMBER_OF_ERASE_REGIONS_OFFSET	0x2C		/* Number of Erase Block Regions */
#define	ERASE_BLOCK_REGION_OFFSET	0x2D		/* Erase Block Region Information */

#if LANGUAGE == C
typedef struct cfi_erase_block_region cfi_erase_block_region;

typedef struct cfi_device_geometry {
	uint32_t device_size;				/* in B */
	uint16_t device_interface;			/* see Section 2. in [2] */
	uint32_t max_bytes_write;			/* in B */
	uint8_t number_of_erase_regions;
	cfi_erase_block_region *erase_block_regions;
} cfi_device_geometry;

struct cfi_erase_block_region {
	uint32_t erase_block_size;			/* in B */
	uint32_t erase_blocks_number;
};
#endif /* LANGUAGE == C */

/* Device Interface Code Assignments (for cfi_device_geometry.device_interface) - see Section 2. in [2] */

#define	CFI_INTERFACE_X8		0
#define CFI_INTERFACE_X16		1
#define	CFI_INTERFACE_X8_X16		2
#define	CFI_INTERFACE_X32		3
#define	CFI_INTERFACE_X16_X32		5

/* CFI Query Structure - see Table 3.3.1 in [1] */

#if LANGUAGE == C
typedef struct cfi_query_structure {
	cfi_query_system_interface_information system_interface_info;
	cfi_device_geometry device_geometry;
	void *pri_vendor_tbl;
	void *alt_vendor_tbl;
} cfi_query_structure;
#endif /* LANGUAGE == C */

#endif /* FLASH_CFI_H */
