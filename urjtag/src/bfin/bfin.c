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
#include <unistd.h>

#include <urjtag/chain.h>
#include <urjtag/cable.h>
#include <urjtag/tap_state.h>
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/bfin.h>

const char * const scans[] = {
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
int bfin_wait_clocks = -1;

static const struct timespec bfin_emu_wait_ts = {0, 5000000};


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

/* Except IDCODE register, Blackfin data registers and instruction registers
   shift out MSB first, which does not conform to JTAG standard.  So it needs
   its own register_set_value and register_get_value functions.  */

static int
bfin_register_set_value (urj_tap_register_t *tr, uint64_t value)
{
    return urj_tap_register_set_value_bit_range (tr, value, 0, tr->len - 1);
}

static uint64_t
bfin_register_get_value (const urj_tap_register_t *tr)
{
    return urj_tap_register_get_value_bit_range (tr, 0, tr->len - 1);
}

static int
bfin_set_scan (urj_part_t *part, int scan)
{
    if (is_bfin_part (part))
    {
        if (BFIN_PART_SCAN (part) != scan)
        {
            urj_part_set_instruction (part, scans[scan]);
            if (part->active_instruction == NULL)
            {
                urj_log (URJ_LOG_LEVEL_ERROR,
                         _("%s: unable to load instruction '%s'\n"),
                         "bfin", scans[scan]);
                return -1;
            }
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

/* The helper functions for Blackfin DBGCTL and DBGSTAT operations.  */

static void
bfin_dbgctl_init (urj_part_t *part, uint16_t v)
{
    bfin_register_set_value (part->active_instruction->data_register->in, v);
}

static uint16_t
bfin_dbgstat_value (urj_part_t *part)
{
    return bfin_register_get_value (part->active_instruction->data_register->out);
}

#define PART_DBGCTL_CLEAR_OR_SET_BIT(name)                              \
    static void                                                         \
    part_dbgctl_bit_clear_or_set_##name (urj_chain_t *chain, int n, int set) \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        uint16_t dbgctl = BFIN_PART_DBGCTL (part);                      \
                                                                        \
        if (set)                                                        \
            dbgctl |= BFIN_PART_DATA (part)->dbgctl_##name;             \
        else                                                            \
            dbgctl &= ~BFIN_PART_DATA (part)->dbgctl_##name;            \
        bfin_dbgctl_init (part, dbgctl);                                \
        BFIN_PART_DBGCTL (part) = dbgctl;                               \
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
        uint16_t dbgctl = BFIN_PART_DBGCTL (part);                      \
        if (dbgctl & BFIN_PART_DATA (part)->dbgctl_##name)              \
            return 1;                                                   \
        else                                                            \
            return 0;                                                   \
    }
       
#define PART_DBGCTL_CLEAR_BIT(name)                                     \
    void                                                                \
    part_dbgctl_bit_clear_##name (urj_chain_t *chain, int n)            \
    {                                                                   \
        part_dbgctl_bit_clear_or_set_##name (chain, n, 0);              \
    }

#define DBGCTL_BIT_OP(name)                                             \
    PART_DBGCTL_CLEAR_OR_SET_BIT(name)                                  \
    PART_DBGCTL_SET_BIT(name)                                           \
    PART_DBGCTL_CLEAR_BIT(name)                                         \
    PART_DBGCTL_IS(name)

/* These functions check cached DBGSTAT. So before calling them,
   dbgstat_get or core_dbgstat_get has to be called to update cached
   DBGSTAT value.  */

#define PART_DBGSTAT_BIT_IS(name)                                       \
    int                                                                 \
    part_dbgstat_is_##name (urj_chain_t *chain, int n)                  \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        uint16_t dbgstat = BFIN_PART_DBGSTAT (part);                    \
        if (dbgstat & BFIN_PART_DATA (part)->dbgstat_##name)            \
            return 1;                                                   \
        else                                                            \
            return 0;                                                   \
    }

#define PART_DBGSTAT_CLEAR_BIT(name)                                    \
    static void                                                         \
    part_dbgstat_bit_clear_##name (urj_chain_t *chain, int n)           \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        urj_tap_register_t *r = part->active_instruction->data_register->in; \
        BFIN_PART_DBGSTAT (part) &= ~BFIN_PART_DATA (part)->dbgstat_##name; \
        bfin_register_set_value (r, BFIN_PART_DBGSTAT (part));          \
    }

#define PART_DBGSTAT_SET_BIT(name)                                      \
    static void                                                         \
    part_dbgstat_bit_set_##name (urj_chain_t *chain, int n)             \
    {                                                                   \
        urj_part_t *part = chain->parts->parts[n];                      \
        urj_tap_register_t *r = part->active_instruction->data_register->in; \
        BFIN_PART_DBGSTAT (part) |= BFIN_PART_DATA (part)->dbgstat_##name; \
        bfin_register_set_value (r, BFIN_PART_DBGSTAT (part));          \
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
    mask = BFIN_PART_DATA (part)->dbgstat_emucause_mask;
    emucause = BFIN_PART_DBGSTAT (part) & mask;

    while (!(mask & 0x1))
    {
        mask >>= 1;
        emucause >>= 1;
    }

    return emucause;
}

void
part_dbgstat_get (urj_chain_t *chain, int n)
{
    urj_part_t *part;

    assert (n >= 0 && n < chain->parts->len);

    part_scan_select (chain, n, DBGSTAT_SCAN);

    part = chain->parts->parts[n];

    urj_tap_chain_shift_data_registers_mode (chain, 1, 1, URJ_CHAIN_EXITMODE_UPDATE);

    BFIN_PART_DBGSTAT (part) = bfin_dbgstat_value (part);
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
    BFIN_PART_EMUPC (part) = bfin_register_get_value (r);
    if (save)
        BFIN_PART_EMUPC_ORIG (part) = BFIN_PART_EMUPC (part);

    return BFIN_PART_EMUPC (part);
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
part_wait_reset (urj_chain_t *chain, int n)
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
            bfin_register_set_value (r, insn << 16);
        else
            bfin_register_set_value (r, insn);
    }
    else
    {
        if ((insn & 0xffffffffffff0000ULL) == 0)
            bfin_register_set_value (r, insn << 48);
        else if ((insn & 0xffffffff00000000ULL) == 0)
            bfin_register_set_value (r, insn << 32);
        else
            bfin_register_set_value (r, insn);
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
        if (!part_is_bfin (chain, i))
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
        if (part_is_bfin (chain, i) && changed[i])
            scan_changed += bfin_set_scan (chain->parts->parts[i], emuir_scan);
        else
            scan_changed += bfin_set_scan (chain->parts->parts[i], BYPASS);
    }

    if (scan_changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (!part_is_bfin (chain, i))
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
        if (!part_is_bfin (chain, i))
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
        if (part_is_bfin (chain, i) && changed[i])
            scan_changed += bfin_set_scan (chain->parts->parts[i], emuir_scan);
        else
            scan_changed += bfin_set_scan (chain->parts->parts[i], BYPASS);
    }

    if (scan_changed)
        urj_tap_chain_shift_instructions_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    for (i = 0; i < chain->parts->len; i++)
    {
        if (!part_is_bfin (chain, i))
            continue;

        if (changed[i] && i == n)
        {
            part = chain->parts->parts[i];
            r = part->active_instruction->data_register->in;
            emuir_init_value (r, insn2);
            urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

            emuir_init_value (r, insn1);
        }
        else if (changed[i])
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

    value = bfin_register_get_value (r);
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

    bfin_register_set_value (r, v);
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
        urj_tap_chain_wait_ready (chain);
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
        urj_tap_chain_wait_ready (chain);
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
void part_register_set (urj_chain_t *chain, int n, enum core_regnum reg,
                        uint32_t value);

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
part_emulation_disable (urj_chain_t *chain, int n)
{
    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_clear_empwr (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
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
part_emulation_return (urj_chain_t *chain, int n)
{
    part_emuir_set (chain, n, INSN_RTE, URJ_CHAIN_EXITMODE_UPDATE);

    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_clear_emeen (chain, n);
    part_dbgctl_bit_clear_wakeup (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_IDLE);

    /* Get the RTE out of EMUIR so we don't execute it more than once.
       This is for working around an issue of ICE-100B.  */
    part_emuir_set (chain, n, INSN_NOP, URJ_CHAIN_EXITMODE_UPDATE);
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

    /*
     * Flush all system events like cache line fills.  Otherwise,
     * when we reset the system side, any events that the core was
     * waiting on no longer exist, and the core hangs.
     */
    part_emuir_set (chain, chain->main_part, INSN_SSYNC, URJ_CHAIN_EXITMODE_IDLE);

    /* Write 0x7 to SWRST to start system reset. */
    part_set_p0 (chain, chain->main_part, SWRST);
    part_set_r0 (chain, chain->main_part, 0x7);
    part_emuir_set (chain, chain->main_part, gen_store16_offset (REG_P0, 0, REG_R0), URJ_CHAIN_EXITMODE_IDLE);

    /*
     * Delay at least 10 SCLKs instead of doing an SSYNC insn.
     * Since the system is being reset, the sync signal might
     * not be asserted, and so the core hangs waiting for it.
     * The magic "10" number was given to us by ADI designers
     * who looked at the schematic and ran some simulations.
     */
    usleep (100);

    /* Write 0x0 to SWRST to stop system reset. */
    part_set_r0 (chain, chain->main_part, 0);
    part_emuir_set (chain, chain->main_part, gen_store16_offset (REG_P0, 0, REG_R0), URJ_CHAIN_EXITMODE_IDLE);

    /* Delay at least 1 SCLK; see comment above for more info. */
    usleep (100);

    part_set_p0 (chain, chain->main_part, p0);
    part_set_r0 (chain, chain->main_part, r0);
}

void
bfin_core_reset (urj_chain_t *chain, int n)
{
    part_emulation_disable (chain, n);

    part_emuir_set (chain, n, INSN_NOP, URJ_CHAIN_EXITMODE_UPDATE);

    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_set_sram_init (chain, n);
    part_dbgctl_bit_set_sysrst (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    part_wait_in_reset (chain, n);

    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_clear_sysrst (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);

    part_wait_reset (chain, n);

    part_emulation_enable (chain, n);
    part_emulation_trigger (chain, n);

    part_scan_select (chain, n, DBGCTL_SCAN);
    part_dbgctl_bit_clear_sram_init (chain, n);
    urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
}

void
software_reset (urj_chain_t *chain, int n)
{
    chain_system_reset (chain);
    bfin_core_reset (chain, n);
}

void
part_emupc_reset (urj_chain_t *chain, int n, uint32_t new_pc)
{
    urj_part_t *part = chain->parts->parts[n];
    uint32_t p0;

    p0 = part_register_get (chain, n, REG_P0);

    BFIN_PART_EMUPC (part) = new_pc;

    part_register_set (chain, n, REG_P0, new_pc);
    part_emuir_set (chain, n, gen_jump_reg (REG_P0), URJ_CHAIN_EXITMODE_IDLE);

    part_register_set (chain, n, REG_P0, p0);
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

struct bfin_part_data bfin_part_data_initializer =
{
    0, /* bypass */
    0, /* scan */

    0, /* dbgctl */
    0, /* dbgstat */

    0x1000, /* DBGCTL_SRAM_INIT */
    0x0800, /* DBGCTL_WAKEUP */
    0x0400, /* DBGCTL_SYSRST */
    0x0200, /* DBGCTL_ESSTEP */
    0x0000, /* DBGCTL_EMUDATSZ_32 */
    0x0080, /* DBGCTL_EMUDATSZ_40 */
    0x0100, /* DBGCTL_EMUDATSZ_48 */
    0x0180, /* DBGCTL_EMUDATSZ_MASK */
    0x0040, /* DBGCTL_EMUIRLPSZ_2 */
    0x0000, /* DBGCTL_EMUIRSZ_64 */
    0x0010, /* DBGCTL_EMUIRSZ_48 */
    0x0020, /* DBGCTL_EMUIRSZ_32 */
    0x0030, /* DBGCTL_EMUIRSZ_MASK */
    0x0008, /* DBGCTL_EMPEN */
    0x0004, /* DBGCTL_EMEEN */
    0x0002, /* DBGCTL_EMFEN */
    0x0001, /* DBGCTL_EMPWR */

    0x8000, /* DBGSTAT_LPDEC1 */
    0x0000, /* No DBGSTAT_IN_POWRGATE for bfin */
    0x4000, /* DBGSTAT_CORE_FAULT */
    0x2000, /* DBGSTAT_IDLE */
    0x1000, /* DBGSTAT_IN_RESET */
    0x0800, /* DBGSTAT_LPDEC0 */
    0x0400, /* DBGSTAT_BIST_DONE */
    0x03c0, /* DBGSTAT_EMUCAUSE_MASK */
    0x0020, /* DBGSTAT_EMUACK */
    0x0010, /* DBGSTAT_EMUREADY */
    0x0008, /* DBGSTAT_EMUDIOVF */
    0x0004, /* DBGSTAT_EMUDOOVF */
    0x0002, /* DBGSTAT_EMUDIF */
    0x0001, /* DBGSTAT_EMUDOF */

    INSN_ILLEGAL, /* emuir_a */
    INSN_ILLEGAL, /* emuir_b */

    0, /* emudat_out */
    0, /* emudat_in */

    -1, /* emupc */
    -1, /* emupc_orig */
};

static void
bfin_wait_ready (void *data)
{
    urj_chain_t *chain = (urj_chain_t *) data;

    /* The following default numbers of wait clock for various cables are
       tested on a BF537 stamp board, on which U-Boot is running.
       CCLK is set to 62MHz and SCLK is set to 31MHz, which is the lowest
       frequency I can set in BF537 stamp Linux kernel.

       The test is done by dumping memory from 0x20000000 to 0x20000010 using
       GDB and gdbproxy:

       (gdb) dump memory u-boot.bin 0x20000000 0x20000010
       (gdb) shell hexdump -C u-boot.bin

       With an incorrect number of wait clocks, the first 4 bytes will be
       duplicated by the second 4 bytes.  */

    if (bfin_wait_clocks == -1)
    {
        urj_cable_t *cable = chain->cable;
        uint32_t frequency = cable->frequency;
        const char *name = cable->driver->name;

        if (strcmp (name, "gnICE+") == 0)
        {
            if (frequency <= 6000000)
                bfin_wait_clocks = 5;
            else if (frequency <= 15000000)
                bfin_wait_clocks = 12;
            else /* <= 30MHz */
                bfin_wait_clocks = 21;
        }
        else if (strcmp (name, "gnICE") == 0)
            bfin_wait_clocks = 3;
        else if (strcmp (name, "ICE-100B") == 0)
        {
            if (frequency <= 5000000)
                bfin_wait_clocks = 5;
            else if (frequency <= 10000000)
                bfin_wait_clocks = 11;
            else if (frequency <= 17000000)
                bfin_wait_clocks = 19;
            else /* <= 25MHz */
                bfin_wait_clocks = 30;
        }

        if (bfin_wait_clocks == -1)
        {
            bfin_wait_clocks = 30;
            urj_warning (_("%s: untested cable, set wait_clocks to %d\n"),
                         name, bfin_wait_clocks);
        }
    }

    urj_tap_chain_defer_clock (chain, 0, 0, bfin_wait_clocks);
}

static void
bfin_part_init (urj_part_t *part)
{
    int i;

    if (!part || !part->params)
        goto error;

    part->params->free = free;
    part->params->wait_ready = bfin_wait_ready;
    part->params->data = malloc (sizeof (struct bfin_part_data));

    *BFIN_PART_DATA (part) = bfin_part_data_initializer;

    if (!part->active_instruction)
        goto error;
    for (i = 0; i < NUM_SCANS; i++)
        if (strcmp (part->active_instruction->name, scans[i]) == 0)
            break;

    if (i == NUM_SCANS)
        goto error;

    BFIN_PART_SCAN (part) = i;
    return;

 error:
    urj_warning (_("Blackfin part is missing instructions\n"));
}

void _bfin_part_init (void) __attribute__((constructor));

void
_bfin_part_init (void)
{
    /* Keep in sync with data/analog/PARTS */
    urj_part_init_register ("BF506", bfin_part_init);
    urj_part_init_register ("BF518", bfin_part_init);
    urj_part_init_register ("BF526", bfin_part_init);
    urj_part_init_register ("BF527", bfin_part_init);
    urj_part_init_register ("BF533", bfin_part_init);
    urj_part_init_register ("BF534", bfin_part_init);
    urj_part_init_register ("BF537", bfin_part_init);
    urj_part_init_register ("BF538", bfin_part_init);
    urj_part_init_register ("BF548", bfin_part_init);
    urj_part_init_register ("BF548M", bfin_part_init);
    urj_part_init_register ("BF561", bfin_part_init);
    urj_part_init_register ("BF592", bfin_part_init);
}
