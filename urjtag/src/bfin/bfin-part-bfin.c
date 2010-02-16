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


#include <sysdep.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/cable.h>
#include <urjtag/data_register.h>
#include <urjtag/part_instruction.h>
#include <urjtag/tap_register.h>
#include <urjtag/bfin.h>
#include <urjtag/log.h>

/* The helper functions for Blackfin DBGCTL and DBGSTAT operations.  */

static void
bfin_dbgctl_init (urj_part_t *part, uint16_t v)
{
    register_init_value (part->active_instruction->data_register->in, v);
}

static uint16_t
bfin_dbgstat_value (urj_part_t *part)
{
    return urj_tap_register_get_value (part->active_instruction->data_register->out);
}

static uint32_t
bfin_test_command (uint32_t addr, int w)
{
    uint32_t test_command;

    /* We can only access [15:0] range.  */
    if ((addr & 0xf0000) != 0)
        return 0;

    test_command =
        (addr & 0x0800) << 15   /* Address bit 11 */
        | (addr & 0x8000) << 8  /* Address bit 15 */
        | (addr & 0x3000) << 4  /* Address bits [13:12] */
        | (addr & 0x47f8)       /* Address bits 14 and [10:3] */
        | 0x1000000             /* Access instruction */
        | 0x4;                  /* Access data array */

    if (w)
        test_command |= 0x2;    /* Write */

    return test_command;
}

struct emu_oab bfin_emu_oab =
{
    bfin_dbgctl_init,
    bfin_dbgstat_value,

    bfin_test_command,

    DTEST_COMMAND,
    DTEST_DATA0,
    DTEST_DATA1,

    0, /* dbgctl_dbgstat_in_one_chain */
    0, /* sticky_in_reset */

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
};

static void
bfin_wait_ready (void *data)
{
    urj_chain_t *chain = (urj_chain_t *) data;

    /* When CCLK is 62MHz and SCLK is 31MHz, which is the lowest frequency I
       can set in bf537 stamp linux kernel, set the gnICE+ cable frequency to
       15MHz (current default), --wait-clocks >= 12 is required.

       When CCLK is 62MHz and SCLK is 31MHz, set the gnICE+ cable frequency to
       30MHz, --wait-clocks >= 21 is required.

       When CCLK is 62MHz and SCLK is 31MHz, set the gnICE+ cable frequency to
       6MHz (default), --wait-clocks >= 5 is required.

       When CCLK is 62MHz and SCLK is 31MHz, set the gnICE cable frequency to
       6MHz (default), --wait-clocks >= 2 is required.  */

    if (bfin_wait_clocks == -1)
    {
        urj_cable_t *cable = chain->cable;
        uint32_t frequency = cable->frequency;
        const char *name = cable->driver->name;

        if (strcmp (name, "gnICE+") == 0 && frequency == 15000000)
            bfin_wait_clocks = 12;
        else if (strcmp (name, "gnICE+") == 0 && frequency == 30000000)
            bfin_wait_clocks = 21;
        else if (strcmp (name, "gnICE+") == 0 && frequency == 6000000)
            bfin_wait_clocks = 5;
        else if (strcmp (name, "gnICE") == 0 && frequency == 6000000)
            bfin_wait_clocks = 2;
        else
        {
            bfin_wait_clocks = 21;
            urj_warning (_("untested cable or frequency, set wait_clocks to %d\n"), bfin_wait_clocks);
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
    EMU_OAB (part) = &bfin_emu_oab;

    BFIN_PART_BYPASS (part) = 0;

    if (!part->active_instruction)
        goto error;
    for (i = 0; i < NUM_SCANS; i++)
        if (strcmp (part->active_instruction->name, scans[i]) == 0)
            break;

    if (i == NUM_SCANS)
        goto error;

    BFIN_PART_SCAN (part) = i;
    BFIN_PART_DBGCTL (part) = 0;
    BFIN_PART_DBGSTAT (part) = 0;
    BFIN_PART_EMUIR_A (part) = INSN_ILLEGAL;
    BFIN_PART_EMUIR_B (part) = INSN_ILLEGAL;
    BFIN_PART_EMUDAT_OUT (part) = 0;
    BFIN_PART_EMUDAT_IN (part) = 0;
    BFIN_PART_EMUPC (part) = -1;
    return;

 error:
    urj_warning (_("Blackfin part is missing instructions\n"));
}

extern void bfin_init (void);

void
bfin_init ()
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
