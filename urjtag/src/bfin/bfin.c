/* Copyright (C) 2008, 2009, 2010 Analog Devices, Inc.
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

#include <sysdep.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <urjtag/chain.h>
#include <urjtag/tap_state.h>
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/bfin.h>
#include "bfin-part.h"

const char *scans[] = {
    "IDCODE",
    "DBGSTAT_SCAN",
    "DBGCTL_SCAN",
    "EMUIR_SCAN",
    "EMUDAT40_SCAN",
    "EMUPC_SCAN",
    "BYPASS",
    "EMUIR64_SCAN",
};

#define SWRST 0xffc00100


int bfin_check_emuready = 1;

static struct timespec bfin_emu_wait_ts = {0, 5000000};


static int
is_bfin_part (urj_part_t *part)
{
    /* FIXME: We now assume only Blackfin parts have initialized params.  */
    if (part->params && part->params->data)
        return 1;
    else
        return 0;
}

int
part_is_bfin (urj_chain_t *chain, int n)
{
    return is_bfin_part (chain->parts->parts[n]);
}

static int
part_is_bypassed (urj_chain_t *chain, int n)
{
    urj_part_t *part;

    part = chain->parts->parts[n];

    if (part_is_bfin (chain, n))
        return BFIN_PART_BYPASS (part);

    /* Other parts are all bypassed.  */
    else
        return 1;
}

void
part_bypass (urj_chain_t *chain, int n)
{
    urj_part_t *part;

    part = chain->parts->parts[n];
    if (is_bfin_part (part))
        BFIN_PART_BYPASS (part) = 1;
}

urj_tap_register_t *
register_init_value (urj_tap_register_t *tr, uint64_t value)
{
    int i;

    for (i = 0; i < tr->len; i++)
        tr->data[i] = (value >> (tr->len - i - 1)) & 1;

    return tr;
}

uint64_t
register_value (urj_tap_register_t *tr)
{
    uint64_t v = 0;
    int i;

    for (i = 0; i < tr->len; i++)
        v = (v << 1) | tr->data[i];

    return v;
}

static int
bfin_set_scan (urj_part_t *part, int scan)
{
    if (is_bfin_part (part))
    {
        if (BFIN_PART_SCAN (part) != scan)
        {
            urj_part_set_instruction (part, scans[scan]);
            assert (part->active_instruction != NULL);
            BFIN_PART_SCAN (part) = scan;
            return 1;
        }
        else
            return 0;
    }
    else
    {
        urj_part_set_instruction (part, scans[scan]);
        return 1;
    }
}

static void emuir_init_value (urj_tap_register_t *r, uint64_t insn);

#if 0
static int
chain_scan_select_array (urj_chain_t *chain, int *scan)
{
    int i;
    int changed;

    changed = 0;

    for (i = 0; i < chain->parts->len; i++)
    {
        urj_part_t *part = chain->parts->parts[i];

        if (part_is_bypassed (chain, i))
            changed += bfin_set_scan (part, BYPASS);
        else
            changed += bfin_set_scan (part, scan[i]);
    }

    if (changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    return 0;
}
#endif

int
chain_scan_select (urj_chain_t *chain, int scan)
{
    int i;
    int changed;

    changed = 0;

    for (i = 0; i < chain->parts->len; i++)
        if (part_is_bypassed (chain, i))
            changed += bfin_set_scan (chain->parts->parts[i], BYPASS);
        else
            changed += bfin_set_scan (chain->parts->parts[i], scan);

    if (changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    return 0;
}

int
part_scan_select (urj_chain_t *chain, int n, int scan)
{
    int i;
    int changed;
    urj_part_t *part;

    changed = 0;

    part = chain->parts->parts[n];

    changed += bfin_set_scan (part, scan);

    if (part->active_instruction == NULL)
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 _("%s: unknown instruction '%s'\n"), part->part, scans[scan]);
        return -1;
    }

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (i != n)
        {
            part = chain->parts->parts[i];
            changed += bfin_set_scan (part, BYPASS);
        }
    }

    if (changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    return 0;
}

#define PART_DBGCTL_CLEAR_OR_SET_BIT(name)                              \
    static void                                                         \
    part_dbgctl_bit_clear_or_set_##name (urj_chain_t *chain, int n, int set) \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        uint16_t dbgctl = BFIN_PART_DBGCTL (part);                      \
                                                                        \
        dbgctl = _part_dbgctl_bit_clear_or_set_##name (part, dbgctl, set); \
        _part_dbgctl_init (part, dbgctl);                               \
        BFIN_PART_DBGCTL (part) = dbgctl;                               \
    }

#define CHAIN_DBGCTL_CLEAR_OR_SET_BIT(name)                             \
    static void                                                         \
    chain_dbgctl_bit_clear_or_set_##name (urj_chain_t *chain, int set)  \
    {                                                                   \
        int i;                                                          \
                                                                        \
        for (i = 0; i < chain->parts->len; i++)                         \
        {                                                               \
            if (part_is_bypassed (chain, i))                            \
                continue;                                               \
                                                                        \
            part_dbgctl_bit_clear_or_set_##name (chain, i, set);        \
        }                                                               \
    }

#define PART_DBGCTL_SET_BIT(name)                                       \
    void                                                                \
    part_dbgctl_bit_set_##name (urj_chain_t *chain, int n)              \
    {                                                                   \
        part_dbgctl_bit_clear_or_set_##name (chain, n, 1);              \
    }

#define PART_DBGCTL_IS(name)                                            \
    int                                                                 \
    part_dbgctl_is_##name (urj_chain_t *chain, int n)                   \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        return _part_dbgctl_is_##name (part, BFIN_PART_DBGCTL (part));  \
    }
       
#define PART_DBGCTL_CLEAR_BIT(name)                                     \
    void                                                                \
    part_dbgctl_bit_clear_##name (urj_chain_t *chain, int n)            \
    {                                                                   \
        part_dbgctl_bit_clear_or_set_##name (chain, n, 0);              \
    }

#define CHAIN_DBGCTL_SET_BIT(name)                                      \
    void                                                                \
    chain_dbgctl_bit_set_##name (urj_chain_t *chain)                    \
    {                                                                   \
        chain_dbgctl_bit_clear_or_set_##name (chain, 1);                \
    }

#define CHAIN_DBGCTL_CLEAR_BIT(name)                                    \
    void                                                                \
    chain_dbgctl_bit_clear_##name (urj_chain_t *chain)                  \
    {                                                                   \
        chain_dbgctl_bit_clear_or_set_##name (chain, 0);                \
    }

#define DBGCTL_BIT_OP(name)                                             \
    PART_DBGCTL_CLEAR_OR_SET_BIT(name)                                  \
    PART_DBGCTL_SET_BIT(name)                                           \
    PART_DBGCTL_CLEAR_BIT(name)                                         \
    PART_DBGCTL_IS(name)                                                \
    CHAIN_DBGCTL_CLEAR_OR_SET_BIT(name)                                 \
    CHAIN_DBGCTL_SET_BIT(name)                                          \
    CHAIN_DBGCTL_CLEAR_BIT(name)


/* These functions check cached DBGSTAT. So before calling them,
   dbgstat_get or core_dbgstat_get has to be called to update cached
   DBGSTAT value.  */

#define PART_DBGSTAT_BIT_IS(name)                                       \
    int                                                                 \
    part_dbgstat_is_##name (urj_chain_t *chain, int n)                  \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        return _part_dbgstat_is_##name (part, BFIN_PART_DBGSTAT (part)); \
    }

#define PART_DBGSTAT_CLEAR_BIT(name)                                    \
    static void                                                         \
    part_dbgstat_bit_clear_##name (urj_chain_t *chain, int n)           \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        urj_tap_register_t *r = part->active_instruction->data_register->in; \
        BFIN_PART_DBGSTAT (part)                                        \
            = _part_dbgstat_bit_clear_##name (part, BFIN_PART_DBGSTAT (part)); \
        register_init_value (r, BFIN_PART_DBGSTAT (part));              \
    }

#define PART_DBGSTAT_SET_BIT(name)                                      \
    static void                                                         \
    part_dbgstat_bit_set_##name (urj_chain_t *chain, int n)             \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        urj_tap_register_t *r = part->active_instruction->data_register->in; \
        BFIN_PART_DBGSTAT (part)                                        \
            = _part_dbgstat_bit_set_##name (part, BFIN_PART_DBGSTAT (part)); \
        register_init_value (r, BFIN_PART_DBGSTAT (part));              \
    }

DBGCTL_BIT_OP (sram_init)
DBGCTL_BIT_OP (wakeup)
DBGCTL_BIT_OP (sysrst)
DBGCTL_BIT_OP (esstep)
DBGCTL_BIT_OP (emudatsz_32)
DBGCTL_BIT_OP (emudatsz_40)
DBGCTL_BIT_OP (emudatsz_48)
DBGCTL_BIT_OP (emuirlpsz_2)
DBGCTL_BIT_OP (emuirsz_64)
DBGCTL_BIT_OP (emuirsz_48)
DBGCTL_BIT_OP (emuirsz_32)
DBGCTL_BIT_OP (empen)
DBGCTL_BIT_OP (emeen)
DBGCTL_BIT_OP (emfen)
DBGCTL_BIT_OP (empwr)

PART_DBGSTAT_BIT_IS (lpdec1)
PART_DBGSTAT_BIT_IS (in_powrgate)
PART_DBGSTAT_BIT_IS (core_fault)
PART_DBGSTAT_BIT_IS (idle)
PART_DBGSTAT_BIT_IS (in_reset)
PART_DBGSTAT_BIT_IS (lpdec0)
PART_DBGSTAT_BIT_IS (bist_done)
PART_DBGSTAT_BIT_IS (emuack)
PART_DBGSTAT_BIT_IS (emuready)
PART_DBGSTAT_BIT_IS (emudiovf)
PART_DBGSTAT_BIT_IS (emudoovf)
PART_DBGSTAT_BIT_IS (emudif)
PART_DBGSTAT_BIT_IS (emudof)

PART_DBGSTAT_CLEAR_BIT (emudiovf)
PART_DBGSTAT_CLEAR_BIT (emudoovf)

PART_DBGSTAT_SET_BIT (emudiovf)
PART_DBGSTAT_SET_BIT (emudoovf)


uint16_t
part_dbgstat_emucause (urj_chain_t *chain, int n)
{
    urj_part_t *part;
    uint16_t mask;
    uint16_t emucause;

    part = chain->parts->parts[n];
    mask = _part_dbgstat_emucause_mask (part);
    emucause = BFIN_PART_DBGSTAT (part) & mask;

    while (!(mask & 0x1))
    {
        mask >>= 1;
        emucause >>= 1;
    }

    return emucause;
}

void
chain_dbgstat_get (urj_chain_t *chain)
{
    urj_part_t *part;
    int i;

    chain_scan_select (chain, DBGSTAT_SCAN);

    /* After doing a shiftDR you always must eventually do an
       update-DR. The dbgstat and dbgctl registers are in the same scan
       chain.  Therefore when you want to read dbgstat you have to be
       careful not to corrupt the dbgctl register in the process. So you
       have to shift out the value that is currently in the dbgctl and
       dbgstat registers, then shift back the same value into the dbgctl
       so that when you do an updateDR you will not change the dbgctl
       register when all you wanted to do is read the dbgstat value.  */

    for (i = 0; i < chain->parts->len; i++)
    {
        part = chain->parts->parts[i];

        if (part_is_bypassed (chain, i))
            continue;

        if (_part_dbgctl_dbgstat_in_one_chain (part))
            _part_dbgctl_init (part, BFIN_PART_DBGCTL (part));
    }

    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        part = chain->parts->parts[i];
        BFIN_PART_DBGSTAT (part) = _part_dbgstat_value (part);
    }
}

void
part_dbgstat_get (urj_chain_t *chain, int n)
{
    urj_part_t *part;

    assert (n >= 0 && n < chain->parts->len);

    part_scan_select (chain, n, DBGSTAT_SCAN);

    /* See above comments.  */

    part = chain->parts->parts[n];

    if (_part_dbgctl_dbgstat_in_one_chain (part))
        _part_dbgctl_init (part, BFIN_PART_DBGCTL (part));

    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);

    BFIN_PART_DBGSTAT (part) = _part_dbgstat_value (part);
}

void
chain_emupc_get (urj_chain_t *chain, int save)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    int i;

    chain_scan_select (chain, EMUPC_SCAN);

    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);
    for (i = 0; i < chain->parts->len; i++)
    {
        part = chain->parts->parts[i];

        if (part_is_bypassed (chain, i))
            continue;

        r = part->active_instruction->data_register->out;
        BFIN_PART_EMUPC (part) = register_value (r);
        if (save)
            BFIN_PART_EMUPC_ORIG (part) = BFIN_PART_EMUPC (part);
    }
}

uint32_t
part_emupc_get (urj_chain_t *chain, int n, int save)
{
    urj_part_t *part;
    urj_tap_register_t *r;

    assert (n >= 0 && n < chain->parts->len);

    part_scan_select (chain, n, EMUPC_SCAN);

    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);

    part = chain->parts->parts[n];
    r = part->active_instruction->data_register->out;
    BFIN_PART_EMUPC (part) = register_value (r);
    if (save)
        BFIN_PART_EMUPC_ORIG (part) = BFIN_PART_EMUPC (part);

    return BFIN_PART_EMUPC (part);
}

void
chain_dbgstat_clear_ovfs (urj_chain_t *chain)
{
    int i;

    chain_scan_select (chain, DBGSTAT_SCAN);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;
        part_dbgstat_bit_set_emudiovf (chain, i);
        part_dbgstat_bit_set_emudoovf (chain, i);
    }

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;
        part_dbgstat_bit_clear_emudiovf (chain, i);
        part_dbgstat_bit_clear_emudoovf (chain, i);
    }
}

void
part_dbgstat_clear_ovfs (urj_chain_t *chain, int n)
{
    part_scan_select (chain, n, DBGSTAT_SCAN);

    part_dbgstat_bit_set_emudiovf (chain, n);
    part_dbgstat_bit_set_emudoovf (chain, n);

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    part_dbgstat_bit_clear_emudiovf (chain, n);
    part_dbgstat_bit_clear_emudoovf (chain, n);
}

void
chain_check_emuready (urj_chain_t *chain)
{
    int emuready;
    int i;

    chain_dbgstat_get (chain);
    emuready = 1;
    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (!(part_dbgstat_is_emuready (chain, i)))
        {
            emuready = 0;
            break;
        }
    }

    assert (emuready);
}

void
part_check_emuready (urj_chain_t *chain, int n)
{
    int emuready;

    part_dbgstat_get (chain, n);
    if (part_dbgstat_is_emuready (chain, n))
        emuready = 1;
    else
        emuready = 0;

    assert (emuready);
}

int
part_sticky_in_reset (urj_chain_t *chain, int n)
{
    urj_part_t *part = chain->parts->parts[n];
    return _part_sticky_in_reset (part);
}

void
chain_wait_in_reset (urj_chain_t *chain)
{
    int in_reset;
    int i;
    int waited = 0;

  try_again:

    chain_dbgstat_get (chain);
    in_reset = 1;
    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (!(part_dbgstat_is_in_reset (chain, i)))
        {
            in_reset = 0;
            break;
        }
    }

    if (waited)
        assert (in_reset);

    if (!in_reset)
    {
        nanosleep (&bfin_emu_wait_ts, NULL);
        waited = 1;
        goto try_again;
    }
}

void
part_wait_in_reset (urj_chain_t *chain, int n)
{
    int in_reset;
    int waited = 0;

  try_again:

    part_dbgstat_get (chain, n);
    if (part_dbgstat_is_in_reset (chain, n))
        in_reset = 1;
    else
        in_reset = 0;

    if (waited)
        assert (in_reset);

    if (!in_reset)
    {
        nanosleep (&bfin_emu_wait_ts, NULL);
        waited = 1;
        goto try_again;
    }
}

void
chain_wait_reset (urj_chain_t *chain)
{
    int in_reset;
    int i;
    int waited = 0;

  try_again:

    chain_dbgstat_get (chain);
    in_reset = 0;
    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (part_dbgstat_is_in_reset (chain, i) && !part_sticky_in_reset (chain, i))
        {
            in_reset = 1;
            break;
        }
    }

    if (waited)
        assert (!in_reset);

    if (in_reset)
    {
        nanosleep (&bfin_emu_wait_ts, NULL);
        waited = 1;
        goto try_again;
    }
}

void
part_wait_reset (urj_chain_t *chain, int n)
{
    int in_reset;
    int waited = 0;

  try_again:

    part_dbgstat_get (chain, n);
    if (part_dbgstat_is_in_reset (chain, n) && !part_sticky_in_reset (chain, n))
        in_reset = 1;
    else
        in_reset = 0;

    if (waited)
        assert (!in_reset);

    if (in_reset)
    {
        nanosleep (&bfin_emu_wait_ts, NULL);
        waited = 1;
        goto try_again;
    }
}

static void
emuir_init_value (urj_tap_register_t *r, uint64_t insn)
{
    if (r->len == 32 || r->len == 34)
    {
        assert ((insn & 0xffffffff00000000ULL) == 0);

        if ((insn & 0xffffffffffff0000ULL) == 0)
            register_init_value (r, insn << 16);
        else
            register_init_value (r, insn);
    }
    else
    {
        if ((insn & 0xffffffffffff0000ULL) == 0)
            register_init_value (r, insn << 48);
        else if ((insn & 0xffffffff00000000ULL) == 0)
            register_init_value (r, insn << 32);
        else
            register_init_value (r, insn);
    }

    /* If EMUIR has two identify bits, set it properly.
       [len-1:len-2] is
       1 for 16-bit instruction.
       2 for 32-bit instruction.
       3 for 64-bit instruction.
       [len-1] is in data[0] and [len-2] is in data[1].  */

    if (r->len % 32 == 2)
    {
        if ((insn & 0xffffffffffff0000ULL) == 0)
        {
            r->data[0] = 0;
            r->data[1] = 1;
        }
        else if ((insn & 0xffffffff00000000ULL) == 0)
        {
            r->data[0] = 1;
            r->data[1] = 0;
        }
        else
            r->data[0] = r->data[1] = 1;
    }
}

void
chain_emuir_set_same (urj_chain_t *chain, uint64_t insn, int exit)
{
    int emuir_scan;
    urj_part_t *part;
    urj_tap_register_t *r;
    int i;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if ((insn & 0xffffffff00000000ULL) == 0)
    {
        emuir_scan = EMUIR_SCAN;

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_set_emuirsz_32 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }
    else
    {
        emuir_scan = EMUIR64_SCAN;

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_set_emuirsz_64 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    chain_scan_select (chain, emuir_scan);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        part = chain->parts->parts[i];
        r = part->active_instruction->data_register->in;
        emuir_init_value (r, insn);
        BFIN_PART_EMUIR_A (part) = insn;
    }

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, exit);

    if (exit == URJ_CHAIN_EXITMODE_IDLE && bfin_check_emuready)
        chain_check_emuready (chain);
}

void
part_emuir_set (urj_chain_t *chain, int n, uint64_t insn, int exit)
{
    int emuir_scan;
    urj_part_t *part;
    urj_tap_register_t *r;
    int *changed;
    int scan_changed;
    int i;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if ((insn & 0xffffffff00000000ULL) == 0)
    {
        emuir_scan = EMUIR_SCAN;

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirsz_32 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }
    else
    {
        emuir_scan = EMUIR64_SCAN;

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirsz_64 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    assert (n >= 0 && n < chain->parts->len);

    changed = (int *) malloc (chain->parts->len *sizeof (int));

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (i == n && BFIN_PART_EMUIR_A (chain->parts->parts[i]) != insn)
        {
            BFIN_PART_EMUIR_A (chain->parts->parts[i]) = insn;
            changed[i] = 1;
        }
        else if (i != n && BFIN_PART_EMUIR_A (chain->parts->parts[i]) != INSN_NOP)
        {
            BFIN_PART_EMUIR_A (chain->parts->parts[i]) = INSN_NOP;
            changed[i] = 1;
        }
        else
            changed[i] = 0;
    }

    scan_changed = 0;

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (changed[i])
            scan_changed += bfin_set_scan (chain->parts->parts[i], emuir_scan);
        else
            scan_changed += bfin_set_scan (chain->parts->parts[i], BYPASS);
    }

    if (scan_changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (changed[i])
        {
            part = chain->parts->parts[i];
            r = part->active_instruction->data_register->in;
            emuir_init_value (r, BFIN_PART_EMUIR_A (part));
        }
    }

    free (changed);

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, exit);

    if (exit == URJ_CHAIN_EXITMODE_IDLE && bfin_check_emuready)
        part_check_emuready (chain, n);
}

void
chain_emuir_set_same_2 (urj_chain_t *chain, uint64_t insn1, uint64_t insn2, int exit)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    int emuir_scan;
    int i;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if ((insn1 & 0xffffffff00000000ULL) == 0
        && (insn2 & 0xffffffff00000000ULL) == 0)
    {
        emuir_scan = EMUIR_SCAN;

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_set_emuirsz_32 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }
    else
    {
        emuir_scan = EMUIR64_SCAN;

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_set_emuirsz_64 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    chain_scan_select (chain, emuir_scan);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        part = chain->parts->parts[i];
        r = part->active_instruction->data_register->in;
        emuir_init_value (r, insn2);
        BFIN_PART_EMUIR_B (part) = insn2;
    }

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        part = chain->parts->parts[i];
        r = part->active_instruction->data_register->in;
        emuir_init_value (r, insn1);
        BFIN_PART_EMUIR_A (part) = insn1;
    }

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, exit);
    if (exit ==  URJ_CHAIN_EXITMODE_IDLE && bfin_check_emuready)
        chain_check_emuready (chain);
}

void
part_emuir_set_2 (urj_chain_t *chain, int n, uint64_t insn1, uint64_t insn2, int exit)
{
    int emuir_scan;
    urj_part_t *part;
    urj_tap_register_t *r;
    int *changed;
    int scan_changed;
    int i;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if ((insn1 & 0xffffffff00000000ULL) == 0
        && (insn2 & 0xffffffff00000000ULL) == 0)
    {
        emuir_scan = EMUIR_SCAN;

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirsz_32 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }
    else
    {
        emuir_scan = EMUIR64_SCAN;

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirsz_64 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    assert (n >= 0 && n < chain->parts->len);

    changed = (int *) malloc (chain->parts->len * sizeof (int));

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (i == n
            && (BFIN_PART_EMUIR_A (chain->parts->parts[i]) != insn1
                || BFIN_PART_EMUIR_B (chain->parts->parts[i]) != insn2))
        {
            BFIN_PART_EMUIR_A (chain->parts->parts[i]) = insn1;
            BFIN_PART_EMUIR_B (chain->parts->parts[i]) = insn2;
            changed[i] = 1;
        }
        else if (i != n
                 && BFIN_PART_EMUIR_A (chain->parts->parts[i]) != INSN_NOP)
        {
            BFIN_PART_EMUIR_A (chain->parts->parts[i]) = INSN_NOP;
            changed[i] = 1;
        }
        else
            changed[i] = 0;
    }

    scan_changed = 0;

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (changed[i])
            scan_changed += bfin_set_scan (chain->parts->parts[i], emuir_scan);
        else
            scan_changed += bfin_set_scan (chain->parts->parts[i], BYPASS);
    }

    if (scan_changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        if (changed[i] && i == n)
        {
            part = chain->parts->parts[i];
            r = part->active_instruction->data_register->in;
            emuir_init_value (r, insn2);
            urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

            emuir_init_value (r, insn1);
        }
        else if (changed[i] && i != chain->active_part)
        {
            part = chain->parts->parts[i];
            r = part->active_instruction->data_register->in;
            emuir_init_value (r, BFIN_PART_EMUIR_A (part));
        }
    }

    free (changed);

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, exit);

    if (exit == URJ_CHAIN_EXITMODE_IDLE && bfin_check_emuready)
        part_check_emuready (chain, n);
}

uint64_t
emudat_value (urj_tap_register_t *r)
{
    uint64_t value;

    value = register_value (r);
    value >>= (r->len - 32);

    return value;
}

void
emudat_init_value (urj_tap_register_t *r, uint32_t value)
{
    uint64_t v = value;

    v <<= (r->len - 32);
    /* If the register size is larger than 32 bits, set EMUDIF.  */
    if (r->len == 34 || r->len == 40 || r->len == 48)
        v |= 0x1 << (r->len - 34);

    register_init_value (r, v);
}

void
part_emudat_defer_get (urj_chain_t *chain, int n, int exit)
{
    int i;
    urj_parts_t *ps;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if (exit == URJ_CHAIN_EXITMODE_IDLE)
    {
        assert (urj_tap_state (chain) & URJ_TAP_STATE_IDLE);
        urj_tap_chain_defer_clock (chain, 0, 0, 1);
    }

    if (part_scan_select (chain, n, EMUDAT_SCAN) < 0)
        abort ();

    if (!chain || !chain->parts)
        return;

    ps = chain->parts;

    for (i = 0; i < ps->len; i++)
    {
        if (ps->parts[i]->active_instruction == NULL)
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     _("Part %d without active instruction\n"), i);
            return;
        }
        if (ps->parts[i]->active_instruction->data_register == NULL)
        {
            urj_log (URJ_LOG_LEVEL_ERROR,
                     _("Part %d without data register\n"), i);
            return;
        }
    }

    urj_tap_capture_dr (chain);

    /* new implementation: split into defer + retrieve part
       shift the data register of each part in the chain one by one */

    for (i = 0; i < ps->len; i++)
    {
        urj_tap_defer_shift_register (chain, ps->parts[i]->active_instruction->data_register->in,
                                      ps->parts[i]->active_instruction->data_register->out,
                                      (i + 1) == ps->len ? URJ_CHAIN_EXITMODE_UPDATE : URJ_CHAIN_EXITMODE_SHIFT);
    }
}

uint32_t
part_emudat_get_done (urj_chain_t *chain, int n, int exit)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    uint64_t value;
    int i;
    urj_parts_t *ps;

    ps = chain->parts;

    for (i = 0; i < ps->len; i++)
    {
        urj_tap_shift_register_output (chain, ps->parts[i]->active_instruction->data_register->in,
                                       ps->parts[i]->active_instruction->data_register->out,
                                       (i + 1) == ps->len ? URJ_CHAIN_EXITMODE_UPDATE : URJ_CHAIN_EXITMODE_SHIFT);
    }

    part = chain->parts->parts[n];
    r = part->active_instruction->data_register->out;
    value = emudat_value (r);

    /* TODO  Is it good to check EMUDOF here if it's available?  */

    return value;
}

/* These two emudat functions only care the payload data, which is the
   upper 32 bits.  Then follows EMUDOF and EMUDIF if the register size
   is larger than 32 bits.  Then the remaining is reserved or don't
   care bits.  */

uint32_t
part_emudat_get (urj_chain_t *chain, int n, int exit)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    uint64_t value;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if (exit == URJ_CHAIN_EXITMODE_IDLE)
    {
        assert (urj_tap_state (chain) & URJ_TAP_STATE_IDLE);
        urj_tap_chain_defer_clock (chain, 0, 0, 1);
    }

    if (part_scan_select (chain, n, EMUDAT_SCAN) < 0)
        return -1;

    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);
    part = chain->parts->parts[n];
    r = part->active_instruction->data_register->out;
    value = emudat_value (r);

    /* TODO  Is it good to check EMUDOF here if it's available?  */

    return value;
}

void
part_emudat_set (urj_chain_t *chain, int n, uint32_t value, int exit)
{
    urj_part_t *part;
    urj_tap_register_t *r;

    assert (exit == URJ_CHAIN_EXITMODE_UPDATE || exit == URJ_CHAIN_EXITMODE_IDLE);

    if (part_scan_select (chain, n, EMUDAT_SCAN) < 0)
        return;

    part = chain->parts->parts[n];
    r = part->active_instruction->data_register->in;
    emudat_init_value (r, value);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, exit);

    if (exit == URJ_CHAIN_EXITMODE_IDLE && bfin_check_emuready)
        part_check_emuready (chain, n);
}

/* Forward declarations */
void chain_register_set (urj_chain_t *chain, enum core_regnum reg, uint32_t *value);
void part_register_set (urj_chain_t *chain, int n, enum core_regnum reg,
                        uint32_t value);

void
chain_register_get (urj_chain_t *chain, enum core_regnum reg, uint32_t *value)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    int i;
    uint32_t *r0 = NULL;

    if (DREG_P (reg) || PREG_P (reg))
        chain_emuir_set_same (chain, gen_move (REG_EMUDAT, reg), URJ_CHAIN_EXITMODE_IDLE);
    else
    {
        r0 = (uint32_t *)malloc (chain->parts->len * sizeof (uint32_t));
        if (!r0)
            abort ();

        chain_register_get (chain, REG_R0, r0);

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_set_emuirlpsz_2 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        chain_emuir_set_same_2 (chain, gen_move (REG_R0, reg),
                                gen_move (REG_EMUDAT, REG_R0), URJ_CHAIN_EXITMODE_IDLE);

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_clear_emuirlpsz_2 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    chain_scan_select (chain, EMUDAT_SCAN);
    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);
    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        part = chain->parts->parts[i];
        r = part->active_instruction->data_register->out;
        value[i] = emudat_value (r);
    }

    if (!DREG_P (reg) && !PREG_P (reg))
    {
        chain_register_set (chain, REG_R0, r0);
        free (r0);
    }
}

uint32_t
part_register_get (urj_chain_t *chain, int n, enum core_regnum reg)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    uint32_t r0 = 0;

    if (DREG_P (reg) || PREG_P (reg))
        part_emuir_set (chain, n, gen_move (REG_EMUDAT, reg), URJ_CHAIN_EXITMODE_IDLE);
    else
    {
        r0 = part_register_get (chain, n, REG_R0);

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        part_emuir_set_2 (chain, n, gen_move (REG_R0, reg),
                          gen_move (REG_EMUDAT, REG_R0), URJ_CHAIN_EXITMODE_IDLE);

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_clear_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    part_scan_select (chain, n, EMUDAT_SCAN);
    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);
    part = chain->parts->parts[n];
    r = part->active_instruction->data_register->out;

    if (!DREG_P (reg) && !PREG_P (reg))
        part_register_set (chain, n, REG_R0, r0);

    return emudat_value (r);
}

static void
chain_register_set_1 (urj_chain_t *chain, enum core_regnum reg, uint32_t *value, bool array)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    int i;
    uint32_t *r0 = NULL;

    if (!DREG_P (reg) && !PREG_P (reg))
    {
        r0 = (uint32_t *)malloc (chain->parts->len * sizeof (uint32_t));
        if (!r0)
            abort ();

        chain_register_get (chain, REG_R0, r0);
    }

    chain_scan_select (chain, EMUDAT_SCAN);
    for (i = 0; i < chain->parts->len; i++)
    {
        if (part_is_bypassed (chain, i))
            continue;

        part = chain->parts->parts[i];
        r = part->active_instruction->data_register->in;
        emudat_init_value (r, array ? value[i] : *value);
    }
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    if (DREG_P (reg) || PREG_P (reg))
        chain_emuir_set_same (chain, gen_move (reg, REG_EMUDAT), URJ_CHAIN_EXITMODE_IDLE);
    else
    {
        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_set_emuirlpsz_2 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        chain_emuir_set_same_2 (chain, gen_move (REG_R0, REG_EMUDAT),
                                gen_move (reg, REG_R0), URJ_CHAIN_EXITMODE_IDLE);

        chain_scan_select (chain, DBGCTL_SCAN);
        chain_dbgctl_bit_clear_emuirlpsz_2 (chain);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        chain_register_set (chain, REG_R0, r0);
        free (r0);
    }
}

void
chain_register_set (urj_chain_t *chain, enum core_regnum reg, uint32_t *value)
{
    chain_register_set_1 (chain, reg, value, true);
}

void
chain_register_set_same (urj_chain_t *chain, enum core_regnum reg, uint32_t value)
{
    chain_register_set_1 (chain, reg, &value, false);
}

void
part_register_set (urj_chain_t *chain, int n, enum core_regnum reg, uint32_t value)
{
    urj_part_t *part;
    urj_tap_register_t *r;
    uint32_t r0 = 0;

    if (!DREG_P (reg) && !PREG_P (reg))
        r0 = part_register_get (chain, n, REG_R0);

    part_scan_select (chain, n, EMUDAT_SCAN);

    part = chain->parts->parts[n];
    r = part->active_instruction->data_register->in;
    BFIN_PART_EMUDAT_IN (part) = value;
    emudat_init_value (r, value);

    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    if (DREG_P (reg) || PREG_P (reg))
        part_emuir_set (chain, n, gen_move (reg, REG_EMUDAT), URJ_CHAIN_EXITMODE_IDLE);
    else
    {
        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        part_emuir_set_2 (chain, n, gen_move (REG_R0, REG_EMUDAT),
                          gen_move (reg, REG_R0), URJ_CHAIN_EXITMODE_IDLE);

        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_clear_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        part_register_set (chain, n, REG_R0, r0);
    }
}

uint32_t
part_get_r0 (urj_chain_t *chain, int n)
{
    return part_register_get (chain, n, REG_R0);
}

uint32_t
part_get_p0 (urj_chain_t *chain, int n)
{
    return part_register_get (chain, n, REG_P0);
}

void
part_set_r0 (urj_chain_t *chain, int n, uint32_t value)
{
    part_register_set (chain, n, REG_R0, value);
}

void
part_set_p0 (urj_chain_t *chain, int n, uint32_t value)
{
    part_register_set (chain, n, REG_P0, value);
}

void
chain_emulation_enable (urj_chain_t *chain)
{
    chain_scan_select (chain, DBGCTL_SCAN);

    chain_dbgctl_bit_set_empwr (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    chain_dbgctl_bit_set_emfen (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    chain_dbgctl_bit_set_emuirsz_32 (chain);
    chain_dbgctl_bit_set_emudatsz_40 (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
}

void
part_emulation_enable (urj_chain_t *chain, int n)
{
    part_scan_select (chain, n, DBGCTL_SCAN);

    part_dbgctl_bit_set_empwr (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    part_dbgctl_bit_set_emfen (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    part_dbgctl_bit_set_emuirsz_32 (chain, n);
    part_dbgctl_bit_set_emudatsz_40 (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
}

void
chain_emulation_disable (urj_chain_t *chain)
{
    chain_scan_select (chain, DBGCTL_SCAN);
    chain_dbgctl_bit_clear_empwr (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
}

void
part_emulation_disable (urj_chain_t *chain, int n)
{
    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_clear_empwr (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
}


void
chain_emulation_trigger (urj_chain_t *chain)
{
    chain_emuir_set_same (chain, INSN_NOP, URJ_CHAIN_EXITMODE_UPDATE);

    chain_scan_select (chain, DBGCTL_SCAN);
    chain_dbgctl_bit_set_wakeup (chain);
    chain_dbgctl_bit_set_emeen (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_IDLE);

    /* I don't know why, but the following code works.  */
    /* Enter the emulation mode */
    urj_tap_chain_defer_clock (chain, 1, 0, 1);
    /* Bring the TAP state to Update-DR */
    urj_tap_chain_defer_clock (chain, 0, 0, 1);
    urj_tap_chain_defer_clock (chain, 1, 0, 2);
}

void
part_emulation_trigger (urj_chain_t *chain, int n)
{
    part_emuir_set (chain, n, INSN_NOP, URJ_CHAIN_EXITMODE_UPDATE);

    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_set_wakeup (chain, n);
    part_dbgctl_bit_set_emeen (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_IDLE);

    /* I don't know why, but the following code works.  */
    /* Enter the emulation mode */
    urj_tap_chain_defer_clock (chain, 1, 0, 1);
    /* Bring the TAP state to Update-DR */
    urj_tap_chain_defer_clock (chain, 0, 0, 1);
    urj_tap_chain_defer_clock (chain, 1, 0, 2);
}

void
chain_emulation_return (urj_chain_t *chain)
{
    chain_emuir_set_same (chain, INSN_RTE, URJ_CHAIN_EXITMODE_UPDATE);

    chain_scan_select (chain, DBGCTL_SCAN);
    chain_dbgctl_bit_clear_emeen (chain);
    chain_dbgctl_bit_clear_wakeup (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_IDLE);
}

void
part_emulation_return (urj_chain_t *chain, int n)
{
    part_emuir_set (chain, n, INSN_RTE, URJ_CHAIN_EXITMODE_UPDATE);

    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_clear_emeen (chain, n);
    part_dbgctl_bit_clear_wakeup (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_IDLE);
}

void
part_execute_instructions (urj_chain_t *chain, int n, struct bfin_insn *insns)
{
    while (insns)
    {
        if (insns->type == BFIN_INSN_NORMAL)
            part_emuir_set (chain, n, insns->i, URJ_CHAIN_EXITMODE_IDLE);
        else /* insns->type == BFIN_INSN_SET_EMUDAT */
            part_emudat_set (chain, n, insns->i, URJ_CHAIN_EXITMODE_UPDATE);

        insns = insns->next;
    }

    return;
}

void
chain_system_reset (urj_chain_t *chain)
{
    uint32_t p0, r0;

    p0 = part_get_p0 (chain, chain->main_part);
    r0 = part_get_r0 (chain, chain->main_part);

    part_set_p0 (chain, chain->main_part, SWRST);
    part_set_r0 (chain, chain->main_part, 0x7);
    part_emuir_set (chain, chain->main_part, gen_store16_offset (REG_P0, 0, REG_R0), URJ_CHAIN_EXITMODE_IDLE);
    part_emuir_set (chain, chain->main_part, INSN_SSYNC, URJ_CHAIN_EXITMODE_IDLE);
    part_set_r0 (chain, chain->main_part, 0);
    part_emuir_set (chain, chain->main_part, gen_store16_offset (REG_P0, 0, REG_R0), URJ_CHAIN_EXITMODE_IDLE);
    part_emuir_set (chain, chain->main_part, INSN_SSYNC, URJ_CHAIN_EXITMODE_IDLE);

    part_set_p0 (chain, chain->main_part, p0);
    part_set_r0 (chain, chain->main_part, r0);
}

void
bfin_core_reset (urj_chain_t *chain)
{
    chain_emulation_disable (chain);

    part_emuir_set (chain, chain->main_part, INSN_NOP, URJ_CHAIN_EXITMODE_UPDATE);

    chain_scan_select (chain, DBGCTL_SCAN);
    chain_dbgctl_bit_set_sram_init (chain);
    part_dbgctl_bit_set_sysrst (chain, chain->main_part);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    chain_wait_in_reset (chain);

    chain_scan_select (chain, DBGCTL_SCAN);
    part_dbgctl_bit_clear_sysrst (chain, chain->main_part);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    chain_wait_reset (chain);

    chain_emulation_enable (chain);
    chain_emulation_trigger (chain);

    chain_scan_select (chain, DBGCTL_SCAN);
    chain_dbgctl_bit_clear_sram_init (chain);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
}

void
software_reset (urj_chain_t *chain)
{
    chain_system_reset (chain);
    bfin_core_reset (chain);
}

void
chain_emupc_reset (urj_chain_t *chain, uint32_t *new_pc)
{
    uint32_t p0[chain->parts->len];
    int i;

    chain_register_get (chain, REG_P0, p0);

    for (i = 0; i < chain->parts->len; i++)
    {
        urj_part_t *part = chain->parts->parts[i];

        if (part_is_bypassed (chain, i))
            continue;

        BFIN_PART_EMUPC (part) = new_pc[i];
    }

    chain_register_set (chain, REG_P0, new_pc);
    chain_emuir_set_same (chain, gen_jump_reg (REG_P0), URJ_CHAIN_EXITMODE_IDLE);

    chain_register_set (chain, REG_P0, p0);
}

uint32_t
part_mmr_read_clobber_r0 (urj_chain_t *chain, int n, int32_t offset, int size)
{
    uint32_t value;

    assert (size == 2 || size == 4);

    if (offset == 0)
    {
        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        if (size == 2)
            part_emuir_set_2 (chain, n,
                              gen_load16z (REG_R0, REG_P0),
                              gen_move (REG_EMUDAT, REG_R0),
                              URJ_CHAIN_EXITMODE_UPDATE);
        else
            part_emuir_set_2 (chain, n,
                              gen_load32 (REG_R0, REG_P0),
                              gen_move (REG_EMUDAT, REG_R0),
                              URJ_CHAIN_EXITMODE_UPDATE);
    }
    else
    {
        if (size == 2)
            part_emuir_set (chain, n, gen_load16z_offset (REG_R0, REG_P0, offset), URJ_CHAIN_EXITMODE_IDLE);
        else
            part_emuir_set (chain, n, gen_load32_offset (REG_R0, REG_P0, offset), URJ_CHAIN_EXITMODE_IDLE);
        part_emuir_set (chain, n, gen_move (REG_EMUDAT, REG_R0), URJ_CHAIN_EXITMODE_UPDATE);
    }
    value = part_emudat_get (chain, n, URJ_CHAIN_EXITMODE_IDLE);

    if (offset == 0)
    {
        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_clear_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }

    return value;
}

uint32_t
part_mmr_read (urj_chain_t *chain, int n, uint32_t addr, int size)
{
    uint32_t p0, r0;
    uint32_t value;

    p0 = part_register_get (chain, n, REG_P0);
    r0 = part_register_get (chain, n, REG_R0);

    part_register_set (chain, n, REG_P0, addr);
    value = part_mmr_read_clobber_r0 (chain, n, 0, size);

    part_register_set (chain, n, REG_P0, p0);
    part_register_set (chain, n, REG_R0, r0);

    return value;
}

void
part_mmr_write_clobber_r0 (urj_chain_t *chain, int n, int32_t offset, uint32_t data, int size)
{
    assert (size == 2 || size == 4);

    part_emudat_set (chain, n, data, URJ_CHAIN_EXITMODE_UPDATE);

    if (offset == 0)
    {
        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_set_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

        if (size == 2)
            part_emuir_set_2 (chain, n,
                              gen_move (REG_R0, REG_EMUDAT),
                              gen_store16 (REG_P0, REG_R0),
                              URJ_CHAIN_EXITMODE_IDLE);
        else
            part_emuir_set_2 (chain, n,
                              gen_move (REG_R0, REG_EMUDAT),
                              gen_store32 (REG_P0, REG_R0),
                              URJ_CHAIN_EXITMODE_IDLE);
    }
    else
    {
        part_emuir_set (chain, n, gen_move (REG_R0, REG_EMUDAT), URJ_CHAIN_EXITMODE_IDLE);
        if (size == 2)
            part_emuir_set (chain, n, gen_store16_offset (REG_P0, offset, REG_R0), URJ_CHAIN_EXITMODE_IDLE);
        else
            part_emuir_set (chain, n, gen_store32_offset (REG_P0, offset, REG_R0), URJ_CHAIN_EXITMODE_IDLE);
    }

    if (offset == 0)
    {
        part_scan_select (chain, n, DBGCTL_SCAN);
        part_dbgctl_bit_clear_emuirlpsz_2 (chain, n);
        urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
    }
}

void
part_mmr_write (urj_chain_t *chain, int n, uint32_t addr, uint32_t data, int size)
{
    uint32_t p0, r0;

    p0 = part_register_get (chain, n, REG_P0);
    r0 = part_register_get (chain, n, REG_R0);

    part_register_set (chain, n, REG_P0, addr);
    part_mmr_write_clobber_r0 (chain, n, 0, data, size);

    part_register_set (chain, n, REG_P0, p0);
    part_register_set (chain, n, REG_R0, r0);
}
