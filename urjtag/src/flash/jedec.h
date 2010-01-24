/*
 * $Id$
 *
 * Copyright (C) 2003 Matan Ziv-Av
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
 * Written by Matan Ziv-Av, 2003.
 *
 */

#ifndef URJ_FLASH_JEDEC_H
#define URJ_FLASH_JEDEC_H

#include <urjtag/types.h>
#include <urjtag/flash.h>

int urj_flash_jedec_detect (urj_bus_t *bus, uint32_t adr,
                            urj_flash_cfi_array_t **urj_flash_cfi_array);
#ifdef JEDEC_EXP
int urj_flash_jedec_exp_detect (urj_bus_t *bus, uint32_t adr,
                                urj_flash_cfi_array_t **urj_flash_cfi_array);
#endif

#endif /* ndef URJ_FLASH_JEDEC_H */
