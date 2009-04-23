/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#ifndef URJ_JTAG_H
#define URJ_JTAG_H

#include <stdio.h>
#include <stdint.h>

#include <flash.h>

#include "chain.h"
#include "bus.h"
#include "part.h"

#define URJ_STATUS_OK             0
#define URJ_STATUS_FAIL           1
#define URJ_STATUS_SYNTAX_ERROR (-1)


extern urj_bus_t *bus;
extern int big_endian;
extern int debug_mode;

const char *urj_cmd_jtag_get_data_dir (void);

int urj_cmd_jtag_parse_file (urj_chain_t *chain, const char *filename);
int urj_cmd_jtag_parse_line (urj_chain_t *chain, char *line);
int urj_cmd_jtag_parse_stream (urj_chain_t *chain, FILE *f);

int urj_tap_detect_parts (urj_chain_t *chain, const char *db_path);
int urj_tap_manual_add (urj_chain_t *chain, int instr_len);
int urj_tap_detect_register_size (urj_chain_t *chain);
void urj_tap_discovery (urj_chain_t *chain);
void urj_tap_urj_tap_idcode (urj_chain_t *chain, unsigned int bytes);

void urj_bus_readmem (urj_bus_t *bus, FILE *f, uint32_t addr, uint32_t len);
void urj_bus_writemem (urj_bus_t *bus, FILE *f, uint32_t addr, uint32_t len);

void urj_flasherase (urj_bus_t *bus, uint32_t addr, int number);

#endif /* URJ_JTAG_H */
