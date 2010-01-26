/* Copyright (C) 2008, 2009, 2010 Analog Devices, Inc.
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License as published by the Free Software
 * Foundation; either version 2, or (at your option) any later
 * version.  See the file COPYING for more details.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Non-GPL License is also available.  Please contact
 * <david.babicz@analog.com> for more information.
 *
 * Implementation of `Blackfin' target for the GDB proxy server.
 */


#ifndef _BLACKFIN_PART_H_
#define _BLACKFIN_PART_H_

#include <urjtag/part.h>

void _part_dbgctl_init (urj_part_t *, uint16_t);
uint16_t _part_dbgstat_value (urj_part_t *);
uint16_t _part_dbgstat_emucause_mask (urj_part_t *);

#define DECLARE__PART_DBGCTL_CLEAR_OR_SET_BIT(name)                     \
    uint16_t _part_dbgctl_bit_clear_or_set_##name (urj_part_t *, uint16_t, int);

#define DECLARE__PART_DBGCTL_IS(name)                                   \
    extern int _part_dbgctl_is_##name (urj_part_t *, uint16_t);

#define DECLARE__PART_DBGCTL(name)                                      \
    DECLARE__PART_DBGCTL_CLEAR_OR_SET_BIT(name)                         \
    DECLARE__PART_DBGCTL_IS(name)

DECLARE__PART_DBGCTL (sram_init)
DECLARE__PART_DBGCTL (wakeup)
DECLARE__PART_DBGCTL (sysrst)
DECLARE__PART_DBGCTL (esstep)
DECLARE__PART_DBGCTL (emudatsz_32)
DECLARE__PART_DBGCTL (emudatsz_40)
DECLARE__PART_DBGCTL (emudatsz_48)
DECLARE__PART_DBGCTL (emuirlpsz_2)
DECLARE__PART_DBGCTL (emuirsz_64)
DECLARE__PART_DBGCTL (emuirsz_48)
DECLARE__PART_DBGCTL (emuirsz_32)
DECLARE__PART_DBGCTL (empen)
DECLARE__PART_DBGCTL (emeen)
DECLARE__PART_DBGCTL (emfen)
DECLARE__PART_DBGCTL (empwr)

#define DECLARE__PART_DBGSTAT_BIT_IS(name)                              \
    int _part_dbgstat_is_##name (urj_part_t *, uint16_t);

#define DECLARE__PART_DBGSTAT_CLEAR_BIT(name)                           \
    uint16_t _part_dbgstat_bit_clear_##name (urj_part_t *, uint16_t);

#define DECLARE__PART_DBGSTAT_SET_BIT(name)                             \
    uint16_t _part_dbgstat_bit_set_##name (urj_part_t *, uint16_t);

DECLARE__PART_DBGSTAT_BIT_IS (lpdec1)
DECLARE__PART_DBGSTAT_BIT_IS (in_powrgate)
DECLARE__PART_DBGSTAT_BIT_IS (core_fault)
DECLARE__PART_DBGSTAT_BIT_IS (idle)
DECLARE__PART_DBGSTAT_BIT_IS (in_reset)
DECLARE__PART_DBGSTAT_BIT_IS (lpdec0)
DECLARE__PART_DBGSTAT_BIT_IS (bist_done)
DECLARE__PART_DBGSTAT_BIT_IS (emuack)
DECLARE__PART_DBGSTAT_BIT_IS (emuready)
DECLARE__PART_DBGSTAT_BIT_IS (emudiovf)
DECLARE__PART_DBGSTAT_BIT_IS (emudoovf)
DECLARE__PART_DBGSTAT_BIT_IS (emudif)
DECLARE__PART_DBGSTAT_BIT_IS (emudof)

DECLARE__PART_DBGSTAT_CLEAR_BIT (emudiovf)
DECLARE__PART_DBGSTAT_CLEAR_BIT (emudoovf)

DECLARE__PART_DBGSTAT_SET_BIT (emudiovf)
DECLARE__PART_DBGSTAT_SET_BIT (emudoovf)

int _part_sticky_in_reset (urj_part_t *);
int _part_dbgctl_dbgstat_in_one_chain (urj_part_t *);


#endif /* _BLACKFIN_PART_H_ */
