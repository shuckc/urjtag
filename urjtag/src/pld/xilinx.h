/* 
 * $Id$
 *
 * Driver for Xilinx FPGAs
 *
 * Copyright (C) 2010, Michael Walle
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
 * Written by Michael Walle <michael@walle.cc>, 2010
 *
 */

#ifndef URJ_PLD_XILINX_H
#define URJ_PLD_XILINX_H

#include <urjtag/pld.h>

#define XILINX_SR_DONE        URJ_BIT(5)
#define XILINX_SR_INIT        URJ_BIT(4)
#define XILINX_SR_ISC_ENABLED URJ_BIT(3)
#define XILINX_SR_ISC_DONE    URJ_BIT(2)

#define XILINX_XC3S_REG_STAT 8
#define XILINX_XC4V_REG_STAT 7
#define XILINX_XC6S_REG_STAT 8

#define XILINX_FAMILY_XC2V      0x08
#define XILINX_FAMILY_XC3S      0x0A
#define XILINX_FAMILY_XC4VLX    0x0B
#define XILINX_FAMILY_XC3SE     0x0E
#define XILINX_FAMILY_XC4VFX    0x0F
#define XILINX_FAMILY_XC4VSX    0x10
#define XILINX_FAMILY_XC3A      0x11
#define XILINX_FAMILY_XC3AN     0x13
#define XILINX_FAMILY_XC3SD     0x1C
#define XILINX_FAMILY_XC5VLX    0x14
#define XILINX_FAMILY_XC5VLXT   0x15
#define XILINX_FAMILY_XC5VSXT   0x17
#define XILINX_FAMILY_XC5CFXT   0x19
#define XILINX_FAMILY_XC5CTXT   0x22
#define XILINX_FAMILY_XC6S      0x20

#define XC3S_STATUS_SYNC_TIMEOUT    URJ_BIT(15)
#define XC3S_STATUS_SEU_ERR         URJ_BIT(14)
#define XC3S_STATUS_DONE            URJ_BIT(13)
#define XC3S_STATUS_INIT            URJ_BIT(12)
#define XC3S_STATUS_MODE_M2         URJ_BIT(11)
#define XC3S_STATUS_MODE_M1         URJ_BIT(10)
#define XC3S_STATUS_MODE_M0         URJ_BIT(9)
#define XC3S_STATUS_VSEL_VS2        URJ_BIT(8)
#define XC3S_STATUS_VSEL_VS1        URJ_BIT(7)
#define XC3S_STATUS_VSEL_VS0        URJ_BIT(6)
#define XC3S_STATUS_GHIGH_B         URJ_BIT(5)
#define XC3S_STATUS_GWE             URJ_BIT(4)
#define XC3S_STATUS_GTS_CFG_B       URJ_BIT(3)
#define XC3S_STATUS_DCM_LOCK        URJ_BIT(2)
#define XC3S_STATUS_ID_ERROR        URJ_BIT(1)
#define XC3S_STATUS_CRC_ERROR       URJ_BIT(0)

#define XC4V_STATUS_DEC_ERROR       URJ_BIT(16)
#define XC4V_STATUS_ID_ERROR        URJ_BIT(15)
#define XC4V_STATUS_DONE            URJ_BIT(14)
#define XC4V_STATUS_RELEASE_DONE    URJ_BIT(13)
#define XC4V_STATUS_INIT            URJ_BIT(12)
#define XC4V_STATUS_INIT_COMPLETE   URJ_BIT(12)
#define XC4V_STATUS_MODE_M2         URJ_BIT(10)
#define XC4V_STATUS_MODE_M1         URJ_BIT(9)
#define XC4V_STATUS_MODE_M0         URJ_BIT(8)
#define XC4V_STATUS_GHIGH_B         URJ_BIT(7)
#define XC4V_STATUS_GWE             URJ_BIT(6)
#define XC4V_STATUS_GTS_CFG_B       URJ_BIT(5)
#define XC4V_STATUS_EOS             URJ_BIT(4)
#define XC4V_STATUS_DCI_MATCH       URJ_BIT(3)
#define XC4V_STATUS_DCM_LOCK        URJ_BIT(2)
#define XC4V_STATUS_PART_SECURED    URJ_BIT(1)
#define XC4V_STATUS_CRC_ERROR       URJ_BIT(0)

#define XC6S_STATUS_SWWD            URJ_BIT(15)
#define XC6S_STATUS_IN_PWRDN        URJ_BIT(14)
#define XC6S_STATUS_DONE            URJ_BIT(13)
#define XC6S_STATUS_INIT_B          URJ_BIT(12)
#define XC6S_STATUS_MODE_M1         URJ_BIT(10)
#define XC6S_STATUS_MODE_M0         URJ_BIT(9)
#define XC6S_STATUS_HSWAPEN         URJ_BIT(8)
#define XC6S_STATUS_PART_SECURED    URJ_BIT(7)
#define XC6S_STATUS_DEC_ERROR       URJ_BIT(6)
#define XC6S_STATUS_GHIGH_B         URJ_BIT(5)
#define XC6S_STATUS_GWE             URJ_BIT(4)
#define XC6S_STATUS_GTS_CFG_B       URJ_BIT(3)
#define XC6S_STATUS_DCM_LOCK        URJ_BIT(2)
#define XC6S_STATUS_ID_ERROR        URJ_BIT(1)
#define XC6S_STATUS_CRC_ERROR       URJ_BIT(0)

extern const urj_pld_driver_t urj_pld_xc3s_driver;
extern const urj_pld_driver_t urj_pld_xc6s_driver;
extern const urj_pld_driver_t urj_pld_xc4v_driver;

typedef struct {
    char *design;
    char *part_name;
    char *date;
    char *time;
    uint32_t   length;
    uint8_t    *data;
} xlx_bitstream_t;

int xlx_bitstream_load_bit (FILE *BIT_FILE, xlx_bitstream_t *bs);
xlx_bitstream_t* xlx_bitstream_alloc (void);
void xlx_bitstream_free (xlx_bitstream_t *bs);

#endif /* URJ_PLD_XILINX_H */
