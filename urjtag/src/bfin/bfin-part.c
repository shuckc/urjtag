/*
 * Copyright (C) 2008, 2009, 2010 Analog Devices, Inc.
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
 */

#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/bfin.h>
#include "bfin-part.h"

/* Wrappers of the helper functions.  */

void
_part_dbgctl_init (urj_part_t *part, uint16_t value)
{
    EMU_OAB (part)->dbgctl_init (part, value);
}

uint16_t
_part_dbgstat_value (urj_part_t *part)
{
    return EMU_OAB (part)->dbgstat_value (part);
}

/* Routines to access DBGCTL and DBGSTAT bits.  */

#define _PART_DBGCTL_CLEAR_OR_SET_BIT(name)                             \
    uint16_t                                                            \
    _part_dbgctl_bit_clear_or_set_##name (urj_part_t *part, uint16_t dbgctl, int set) \
    {                                                                   \
        if (set)                                                        \
            return dbgctl | EMU_OAB (part)->dbgctl_##name;              \
        else                                                            \
            return dbgctl & ~EMU_OAB (part)->dbgctl_##name;             \
    }

#define _PART_DBGCTL_BIT_IS(name)                                       \
    int                                                                 \
    _part_dbgctl_is_##name (urj_part_t *part, uint16_t dbgctl)          \
    {                                                                   \
        if (dbgctl & EMU_OAB (part)->dbgctl_##name)                     \
            return 1;                                                   \
        else                                                            \
            return 0;                                                   \
    }

#define _PART_DBGCTL(name)                                              \
    _PART_DBGCTL_CLEAR_OR_SET_BIT(name)                                 \
    _PART_DBGCTL_BIT_IS(name)

#define _PART_DBGCTL_BIT_IS_MASK(base, sfx)                             \
    int                                                                 \
    _part_dbgctl_is_##base##_##sfx (urj_part_t *part, uint16_t dbgctl)  \
    {                                                                   \
        if ((dbgctl & EMU_OAB (part)->dbgctl_##base##_mask) ==          \
            EMU_OAB (part)->dbgctl_##base##_##sfx)                      \
            return 1;                                                   \
        else                                                            \
            return 0;                                                   \
    }

#define _PART_DBGCTL_MASK(base, sfx)            \
    _PART_DBGCTL_CLEAR_OR_SET_BIT(base##_##sfx) \
    _PART_DBGCTL_BIT_IS_MASK(base, sfx)

_PART_DBGCTL (sram_init)
_PART_DBGCTL (wakeup)
_PART_DBGCTL (sysrst)
_PART_DBGCTL (esstep)
_PART_DBGCTL_MASK (emudatsz, 32)
_PART_DBGCTL_MASK (emudatsz, 40)
_PART_DBGCTL_MASK (emudatsz, 48)
_PART_DBGCTL (emuirlpsz_2)
_PART_DBGCTL_MASK (emuirsz, 64)
_PART_DBGCTL_MASK (emuirsz, 48)
_PART_DBGCTL_MASK (emuirsz, 32)
_PART_DBGCTL (empen)
_PART_DBGCTL (emeen)
_PART_DBGCTL (emfen)
_PART_DBGCTL (empwr)

#define _PART_DBGSTAT_BIT_IS(name)                                      \
    int                                                                 \
    _part_dbgstat_is_##name (urj_part_t *part, uint16_t dbgstat)        \
    {                                                                   \
        if (dbgstat & EMU_OAB (part)->dbgstat_##name)                   \
            return 1;                                                   \
        else                                                            \
            return 0;                                                   \
    }

#define _PART_DBGSTAT_CLEAR_BIT(name)                                   \
    uint16_t                                                            \
    _part_dbgstat_bit_clear_##name (urj_part_t *part, uint16_t dbgstat) \
    {                                                                   \
        return dbgstat & ~EMU_OAB (part)->dbgstat_##name;               \
    }

#define _PART_DBGSTAT_SET_BIT(name)                                     \
    uint16_t                                                            \
    _part_dbgstat_bit_set_##name (urj_part_t *part, uint16_t dbgstat)   \
    {                                                                   \
        return dbgstat | EMU_OAB (part)->dbgstat_##name;                \
    }

_PART_DBGSTAT_BIT_IS (lpdec1)
_PART_DBGSTAT_BIT_IS (in_powrgate)
_PART_DBGSTAT_BIT_IS (core_fault)
_PART_DBGSTAT_BIT_IS (idle)
_PART_DBGSTAT_BIT_IS (in_reset)
_PART_DBGSTAT_BIT_IS (lpdec0)
_PART_DBGSTAT_BIT_IS (bist_done)
_PART_DBGSTAT_BIT_IS (emuack)
_PART_DBGSTAT_BIT_IS (emuready)
_PART_DBGSTAT_BIT_IS (emudiovf)
_PART_DBGSTAT_BIT_IS (emudoovf)
_PART_DBGSTAT_BIT_IS (emudif)
_PART_DBGSTAT_BIT_IS (emudof)

_PART_DBGSTAT_CLEAR_BIT (emudiovf)
_PART_DBGSTAT_CLEAR_BIT (emudoovf)

_PART_DBGSTAT_SET_BIT (emudiovf)
_PART_DBGSTAT_SET_BIT (emudoovf)

int
_part_dbgctl_dbgstat_in_one_chain (urj_part_t *part)
{
    return EMU_OAB (part)->dbgctl_dbgstat_in_one_chain;
}

int
_part_sticky_in_reset (urj_part_t *part)
{
    return EMU_OAB (part)->sticky_in_reset;
}

uint16_t
_part_dbgstat_emucause_mask (urj_part_t *part)
{
    return EMU_OAB (part)->dbgstat_emucause_mask;
}

void _bfin_part_init (void) __attribute__((constructor));

extern void bfin_init (void);

void
_bfin_part_init ()
{
    bfin_init ();
}
