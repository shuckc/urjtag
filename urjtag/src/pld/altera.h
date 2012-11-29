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

#ifndef URJ_PLD_ALTERA_H
#define URJ_PLD_ALTERA_H

#include <urjtag/pld.h>

// JTAG instructions - not all are defined in the bsdl files, so use private constants
#define ALTERA_JI_EXTEST          0x000
#define ALTERA_JI_PROGRAM         0x002
#define ALTERA_JI_STARTUP         0x003
#define ALTERA_JI_CHECK_STATUS    0x004
#define ALTERA_JI_SAMPLE          0x005
#define ALTERA_JI_IDCODE          0x006
#define ALTERA_JI_USERCODE        0x007
#define ALTERA_JI_BYPASS          0x3FF
#define ALTERA_JI_PULSE_NCONFIG   0x001
#define ALTERA_JI_CONFIG_IO       0x00D
#define ALTERA_JI_HIGHZ           0x00B
#define ALTERA_JI_CLAMP           0x00A


#define ALTERA_IDCODE_MANUF      0x06E

extern const urj_pld_driver_t urj_pld_alt_driver;

typedef struct {
    char *design;
    char *part_name;
    char *date;
    char *time;
    uint32_t   length;
    uint8_t    *data;
} alt_bitstream_t;

typedef struct {
	char *family;
	char *device;
	uint32_t idcode;
	uint32_t jseq_max;
	uint32_t jseq_conf_done;
} alt_device_config_t;

// int alt_bitstream_load_bit (FILE *BIT_FILE, alt_bitstream_t *bs);
// alt_bitstream_t* alt_bitstream_alloc (void);
// void alt_bitstream_free (alt_bitstream_t *bs);

#endif /* URJ_PLD_ALTERA_H */
