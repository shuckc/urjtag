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
 * [1] JEDEC Solid State Technology Association, "Common Flash Interface (CFI)",
 *     September 1999, Order Number: JESD68
 * [2] JEDEC Solid State Technology Association, "Common Flash Interface (CFI) ID Codes",
 *     September 2001, Order Number: JEP137-A
 * [3] AMD, "Common Flash Memory Interface Specification", Release 2.0
 *     December 1, 2001.
 * [4] SPANSION, "Common Flash Interface Version 1.4 Vendor Specific
 *     Extensions", March 22, 2004.
 *
 */

#ifndef FLASH_CFI_H
#define FLASH_CFI_H

#ifndef __ASSEMBLY__
#include <stdint.h>

#include <urjtag/types.h>
#include <urjtag/flash.h>
#endif

/* CFI commands - see Table 1 in [1] */

#define CFI_CMD_READ_ARRAY1             0xFF
#define CFI_CMD_READ_ARRAY2             0xF0
#define CFI_CMD_QUERY                   0x98
#define CFI_CMD_QUERY_OFFSET            0x55

/* Query identification string - see 4.3.2 in [1] */

#define CFI_QUERY_ID_OFFSET             0x10
#define PRI_VENDOR_ID_OFFSET            0x13
#define PRI_VENDOR_TABLE_ADR_OFFSET     0x15
#define ALT_VENDOR_ID_OFFSET            0x17
#define ALT_VENDOR_TABLE_ADR_OFFSET     0x19

#ifndef __ASSEMBLY__
typedef struct cfi_query_identification_string
{
    uint16_t pri_id_code;
    void *pri_vendor_tbl;
    uint16_t alt_id_code;
    void *alt_vendor_tbl;
} urj_flash_cfi_query_identification_string_t;
#endif /* __ASSEMBLY__ */

/* Algorithm command set & control interface ID codes - see Table 1 in [2] */

#define CFI_VENDOR_NULL                 0x0000
#define CFI_VENDOR_INTEL_ECS            0x0001
#define CFI_VENDOR_AMD_SCS              0x0002
#define CFI_VENDOR_INTEL_SCS            0x0003
#define CFI_VENDOR_AMD_ECS              0x0004
#define CFI_VENDOR_MITSUBISHI_SCS       0x0100
#define CFI_VENDOR_MITSUBISHI_ECS       0x0101
#define CFI_VENDOR_SST_PWCS             0x0102

/* Query system interface information - see 4.3.3 in [1] */

#define VCC_MIN_WEV_OFFSET              0x1B    /* Vcc Logic Supply Minimum Write/Erase voltage */
#define VCC_MAX_WEV_OFFSET              0x1C    /* Vcc Logic Supply Maximum Write/Erase voltage */
#define VPP_MIN_WEV_OFFSET              0x1D    /* Vpp [Programming] Supply Minimum Write/Erase voltage */
#define VPP_MAX_WEV_OFFSET              0x1E    /* Vpp [Programming] Supply Maximum Write/Erase voltage */
#define TYP_SINGLE_WRITE_TIMEOUT_OFFSET 0x1F    /* Typical timeout per single byte/word write */
#define TYP_BUFFER_WRITE_TIMEOUT_OFFSET 0x20    /* Typical timeout for minimum-size buffer write */
#define TYP_BLOCK_ERASE_TIMEOUT_OFFSET  0x21    /* Typical timeout per individual block erase */
#define TYP_CHIP_ERASE_TIMEOUT_OFFSET   0x22    /* Typical timeout for full chip erase */
#define MAX_SINGLE_WRITE_TIMEOUT_OFFSET 0x23    /* Maximum timeout for byte/word write */
#define MAX_BUFFER_WRITE_TIMEOUT_OFFSET 0x24    /* Maximum timeout for buffer write */
#define MAX_BLOCK_ERASE_TIMEOUT_OFFSET  0x25    /* Maximum timeout per individual block erase */
#define MAX_CHIP_ERASE_TIMEOUT_OFFSET   0x26    /* Maximum timeout for chip erase */

#ifndef __ASSEMBLY__
typedef struct cfi_query_system_interface_information
{
    uint16_t vcc_min_wev;       /* in mV */
    uint16_t vcc_max_wev;       /* in mV */
    uint16_t vpp_min_wev;       /* in mV, 0 - no Vpp pin is present */
    uint16_t vpp_max_wev;       /* in mV, 0 - no Vpp pin is present */
    uint32_t typ_single_write_timeout;  /* in us, 0 - not supported */
    uint32_t typ_buffer_write_timeout;  /* in us, 0 - not supported */
    uint32_t typ_block_erase_timeout;   /* in ms, 0 - not supported */
    uint32_t typ_chip_erase_timeout;    /* in ms, 0 - not supported */
    uint32_t max_single_write_timeout;  /* in us, 0 - not supported */
    uint32_t max_buffer_write_timeout;  /* in us, 0 - not supported */
    uint32_t max_block_erase_timeout;   /* in ms, 0 - not supported */
    uint32_t max_chip_erase_timeout;    /* in ms, 0 - not supported */
} urj_flash_cfi_query_system_interface_information_t;
#endif /* __ASSEMBLY__ */

/* Device geometry definition - see 4.3.4 in [1] */

#define DEVICE_SIZE_OFFSET              0x27    /* Device Size */
#define FLASH_DEVICE_INTERFACE_OFFSET   0x28    /* Flash Device Interface description */
#define MAX_BYTES_WRITE_OFFSET          0x2A    /* Maximum number of bytes in multi-byte write */
#define NUMBER_OF_ERASE_REGIONS_OFFSET  0x2C    /* Number of Erase Block Regions */
#define ERASE_BLOCK_REGION_OFFSET       0x2D    /* Erase Block Region Information */

#ifndef __ASSEMBLY__
typedef struct cfi_erase_block_region urj_flash_cfi_erase_block_region_t;

typedef struct cfi_device_geometry
{
    uint32_t device_size;       /* in B */
    uint16_t device_interface;  /* see Table 2 in [2] */
    uint32_t max_bytes_write;   /* in B */
    uint8_t number_of_erase_regions;
    urj_flash_cfi_erase_block_region_t *erase_block_regions;
} urj_flash_cfi_device_geometry_t;

struct cfi_erase_block_region
{
    uint32_t erase_block_size;  /* in B */
    uint32_t number_of_erase_blocks;
};
#endif /* __ASSEMBLY__ */

/* Device interface code assignments (for cfi_device_geometry.device_interface) - see Table 2 in [2] */

#define CFI_INTERFACE_X8                0
#define CFI_INTERFACE_X16               1
#define CFI_INTERFACE_X8_X16            2
#define CFI_INTERFACE_X32               3
#define CFI_INTERFACE_X16_X32           4

/* CFI Query structure - see 4.3.1 in [1] */

#ifndef __ASSEMBLY__
typedef struct cfi_query_structure
{
    urj_flash_cfi_query_identification_string_t identification_string;
    urj_flash_cfi_query_system_interface_information_t system_interface_info;
    urj_flash_cfi_device_geometry_t device_geometry;
} urj_flash_cfi_query_structure_t;

struct URJ_FLASH_CFI_CHIP
{
    int width;                  /* 1 for 8 bits, 2 for 16 bits, 4 for 32 bits, etc. */
    urj_flash_cfi_query_structure_t cfi;
};
#endif /* __ASSEMBLY__ */

/* AMD primary vendor-specific extended query structure - see [3] and [4] */
#define MAJOR_VERSION_OFFSET                    0x03
#define MINOR_VERSION_OFFSET                    0x04
#define ADDRESS_SENSITIVE_UNLOCK_OFFSET         0x05
#define ERASE_SUSPEND_OFFSET                    0x06
#define SECTOR_PROTECT_OFFSET                   0x07
#define SECTOR_TEMPORARY_UNPROTECT_OFFSET       0x08
#define SECTOR_PROTECT_SCHEME_OFFSET            0x09
#define SIMULTANEOUS_OPERATION_OFFSET           0x0A
#define BURST_MODE_TYPE_OFFSET                  0x0B
#define PAGE_MODE_TYPE_OFFSET                   0x0C
#define ACC_MIN_OFFSET                          0x0D
#define ACC_MAX_OFFSET                          0x0E
#define TOP_BOTTOM_SECTOR_FLAG_OFFSET           0x0F
#define PROGRAM_SUSPEND_OFFSET                  0x10
#define UNLOCK_BYPASS_OFFSET                    0x11
#define SECSI_SECTOR_SIZE_OFFSET                0x12
#define EMBEDDED_HWRST_TIMEOUT_MAX_OFFSET       0x13
#define NON_EMBEDDED_HWRST_TIMEOUT_MAX_OFFSET   0x14
#define ERASE_SUSPEND_TIMEOUT_MAX_OFFSET        0x15
#define PROGRAM_SUSPEND_TIMEOUT_MAX_OFFSET      0x16
#define BANK_ORGANIZATION_OFFSET                0x17
#define BANK_REGION_INFO_OFFSET                 0X18

#ifndef __ASSEMBLY__
typedef struct amd_pri_extened_query_structure
{
    uint8_t major_version;
    uint8_t minor_version;
    uint8_t address_sensitive_unlock;
    uint8_t erase_suspend;
    uint8_t sector_protect;
    uint8_t sector_temporary_unprotect;
    uint8_t sector_protect_scheme;
    uint8_t simultaneous_operation;
    uint8_t burst_mode_type;
    uint8_t page_mode_type;
    uint16_t acc_min;           /* in mV */
    uint16_t acc_max;           /* in mV */
    uint8_t top_bottom_sector_flag;
    uint8_t program_suspend;
    uint8_t unlock_bypass;
    uint8_t secsi_sector_size;
    uint8_t embedded_hwrst_timeout_max;
    uint8_t non_embedded_hwrst_timeout_max;     /* in ns */
    uint8_t erase_suspend_timeout_max;  /* in ns */
    uint8_t program_suspend_timeout_max;        /* in us */
    uint8_t bank_organization;  /* in us */
    uint8_t bank_region_info[0];
} urj_flash_cfi_amd_pri_extened_query_structure_t;

void urj_flash_cfi_array_free (urj_flash_cfi_array_t *cfi_array);
/** @return URJ_STATUS_OK on success; URJ_STATUS_FAIL on error */
int urj_flash_cfi_detect (urj_bus_t *bus, uint32_t adr,
                          urj_flash_cfi_array_t **cfi_array);
#endif /* __ASSEMBLY__ */

#endif /* FLASH_CFI_H */
