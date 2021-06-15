/* 
 * Driver for Altera FPGAs
 *
 * Copyright (C) 2012, Chris Shucksmith
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
 * Written by Chris Shucksmith <chris@shucksmith.co.uk>, 2012
 *
 */

#ifndef URJ_PLD_ALTERA_H
#define URJ_PLD_ALTERA_H

#include <urjtag/pld.h>

#define ALTERA_IDCODE_MANUF      0x06E

extern const urj_pld_driver_t urj_pld_alt_driver;

typedef struct {
	char *family;
	char *device;
	uint32_t idcode;
	uint32_t jseq_max;
	uint32_t jseq_conf_done;
} alt_device_config_t;

#endif /* URJ_PLD_ALTERA_H */
