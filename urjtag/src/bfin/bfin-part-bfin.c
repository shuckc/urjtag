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

static void
bfin_test_command_mmrs (urj_part_t *part, uint32_t addr, int icache,
                        uint32_t *command_addr,
                        uint32_t *data0_addr, uint32_t *data1_addr)
{
    if (icache) {
        *command_addr = ITEST_COMMAND;
        *data0_addr = ITEST_DATA0;
        *data1_addr = ITEST_DATA1;
    } else {
        *command_addr = DTEST_COMMAND;
        *data0_addr = DTEST_DATA0;
        *data1_addr = DTEST_DATA1;
    }
}

static void
bfin_test_command (urj_part_t *part, uint32_t addr, int w,
                   uint32_t command_addr, uint32_t *command_value)
{
    /* The shifting here is a bit funky, but should be straight forward and
       easier to maintain than hand coded masks.  So, start with the break
       down of the [DI]TEST_COMMAND MMR in the HRM and follow along:

       We need to put bit 11 of the address into bit 26 of the MMR.  So first
       we mask off ADDR[11] with:
       (addr & (1 << 11))

       Then we shift it from its current position (11) to its new one (26):
       ((...) << (26 - 11))
     */

    /* Start with the bits ITEST/DTEST share.  */
    *command_value =
        ((addr & (0x03 << 12)) << (16 - 12)) | /* ADDR[13:12] -> MMR[17:16] */
        ((addr & (0x01 << 14)) << (14 - 14)) | /* ADDR[14]    -> MMR[14]    */
        ((addr & (0xff <<  3)) << ( 3 -  3)) | /* ADDR[10:3]  -> MMR[10:3]  */
        (1 << 2)                             | /* 1 (data)    -> MMR[2]     */
        (w << 1);                              /* read/write  -> MMR[1]     */

    /* Now for the bits that aren't the same.  */
    if (command_addr == ITEST_COMMAND)
        *command_value |=
            ((addr & (0x03 << 10)) << (26 - 10));  /* ADDR[11:10] -> MMR[27:26] */
    else
        *command_value |=
            ((addr & (0x01 << 11)) << (26 - 11)) | /* ADDR[11] -> MMR[26] */
            ((addr & (0x01 << 21)) << (24 - 21));  /* ADDR[21] -> MMR[24] */

    /* Now, just for fun, some parts are slightly different.  */
    if (command_addr == DTEST_COMMAND)
    {
        /* BF50x has no additional needs.  */
        if (!strcmp (part->part, "BF518"))
        {
            /* MMR[23]:
               0 - Data Bank A (0xff800000) / Inst Bank A (0xffa00000)
               1 - Data Bank B (0xff900000) / Inst Bank B (0xffa04000)
             */
            if ((addr & 0xfff04000) == 0xffa04000 ||
                (addr & 0xfff00000) == 0xff900000)
                *command_value |= (1 << 23);
        }
        else if (!strcmp (part->part, "BF526") ||
                 !strcmp (part->part, "BF527") ||
                 !strcmp (part->part, "BF533") ||
                 !strcmp (part->part, "BF534") ||
                 !strcmp (part->part, "BF537") ||
                 !strcmp (part->part, "BF538") ||
                 !strcmp (part->part, "BF548") ||
                 !strcmp (part->part, "BF548M"))
        {
            /* MMR[23]:
               0 - Data Bank A (0xff800000) / Inst Bank A (0xffa00000)
               1 - Data Bank B (0xff900000) / Inst Bank B (0xffa08000)
             */
            if ((addr & 0xfff08000) == 0xffa08000 ||
                (addr & 0xfff00000) == 0xff900000)
                *command_value |= (1 << 23);
        }
        else if (!strcmp (part->part, "BF561"))
        {
            /* MMR[23]:
               0 - Data Bank A (Core A: 0xff800000 Core B: 0xff400000)
                   Inst Bank A (Core A: 0xffa00000 Core B: 0xff600000)
               1 - Data Bank B (Core A: 0xff900000 Core B: 0xff500000)
                   N/A for Inst (no Bank B)
             */
            uint32_t hi = (addr >> 20);
            if (hi == 0xff9 || hi == 0xff5)
                *command_value |= (1 << 23);
        }
        else if (!strcmp (part->part, "BF592"))
        {
            /* ADDR[15] -> MMR[15]
               MMR[22]:
               0 - L1 Inst (0xffa00000)
               1 - L1 ROM  (0xffa10000)
             */
            *command_value |= (addr & (1 << 15));
            if ((addr >> 16) == 0xffa1)
                *command_value |= (1 << 22);
        }
    }
}

static const struct emu_oab bfin_emu_oab =
{
    bfin_dbgctl_init,
    bfin_dbgstat_value,

    bfin_test_command_mmrs,
    bfin_test_command,

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
bfin_init (void)
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
