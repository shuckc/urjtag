/* 
 * Driver for Lattice FPGAs
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

#ifndef URJ_PLD_LATTICE_H
#define URJ_PLD_LATTICE_H

#include <urjtag/pld.h>

#define LATTICE_PART_ECP3_35EA      0x01012043
#define LSCC_PADDING_SZ   328

#define ECP_35EA_STATUS_    URJ_BIT(15)

extern const urj_pld_driver_t urj_pld_lat_ecp3_driver;

// Lattice have an arbitary number of null-terminated strings in the header, 
// convert this to a linked list of char*
typedef struct {
    char *text;
    lat_header_t *next;
} lat_header_t;

typedef struct {
    char *filename;
    lat_header_t *header;
    uint32_t   length;
    uint8_t    *data;
} lat_bitstream_t;

int lat_bitstream_load_bit (FILE *BIT_FILE, lat_bitstream_t *bs);
lat_bitstream_t* lat_bitstream_alloc (void);
void lat_bitstream_free (lat_bitstream_t *bs);

#endif /* URJ_PLD_LATTICE_H */
