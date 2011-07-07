/*
 * $Id$
 *
 * EJTAG compatible bus driver via PrAcc
 * Copyright (C) 2005, Marek Michalkiewicz
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
 * Written by Marek Michalkiewicz <marekm@amelek.gda.pl>, 2005.
 *
 * Documentation:
 * [1] MIPS Licensees, "MIPS EJTAG Debug Solution", 980818 Rev. 2.0.0
 * [2] MIPS Technologies, Inc. "EJTAG Specification", 2001-02-15, Rev. 2.60
 *
 */

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>
#include <urjtag/tap_state.h>
#include <urjtag/tap_register.h>
#include <urjtag/data_register.h>

#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    uint32_t impcode;           /* EJTAG Implementation Register */
    uint16_t adr_hi;            /* cached high bits of $3 */
} bus_params_t;

#define BP              ((bus_params_t *) bus->params)

#define EJTAG_VER       ((BP->impcode >> 29) & 7)

#define EJTAG_20        0
#define EJTAG_25        1
#define EJTAG_26        2
#define EJTAG_31        3

/* EJTAG 3.1 Control Register Bits */
#define VPED            23      /* R    */
/* EJTAG 2.6 Control Register Bits */
#define Rocc            31      /* R/W0 */
#define Psz1            30      /* R    */
#define Psz0            29      /* R    */
#define Doze            22      /* R    */
#define ProbTrap        14      /* R/W  */
#define DebugMode        3      /* R    */
/* EJTAG 1.5.3 Control Register Bits */
#define Dnm             28      /* */
#define Sync            23      /* R/W  */
#define Run             21      /* R    */
#define PerRst          20      /* R/W  */
#define PRnW            19      /* R    0 = Read, 1 = Write */
#define PrAcc           18      /* R/W0 */
#define DmaAcc          17      /* R/W  */
#define PrRst           16      /* R/W  */
#define ProbEn          15      /* R/W  */
#define SetDev          14      /* R    */
#define JtagBrk         12      /* R/W1 */
#define DStrt           11      /* R/W1 */
#define DeRR            10      /* R    */
#define DrWn             9      /* R/W  */
#define Dsz1             8      /* R/W  */
#define Dsz0             7      /* R/W  */
#define DLock            5      /* R/W  */
#define BrkSt            3      /* R    */
#define TIF              2      /* W0/R */
#define TOF              1      /* W0/R */
#define ClkEn            0      /* R/W  */

/* EJTAG 3.1 Debug Control Register at drseg 0xFF300000 */
#define PCS              9      /* R    */
#define PCR2             8      /* R/W  */
#define PCR1             7      /* R/W  */
#define PCR0             6      /* R/W  */
/* EJTAG 2.X Debug Control Register at drseg 0xFF300000 */
#define DataBrk         17      /* R    */
#define InstBrk         16      /* R    */
#define NMIPend          2      /* R    */
#define SRstE            1      /* R/W  */
#define DCRProbeEn       0      /* R    */
/* EJTAG 1.5.3 Debug Control Register at drseg 0xFF300000*/
#define HIS             30      /* R    */
#define ENM             29      /* R 0=Little End,1=Big Endian */
#define MIntE            4      /* R/W  */
#define MNmiE            3      /* R/W  */
#define MemProt          2      /* R/W 0=WriteOK,1=Protected */
#define MRst             1      /* R/W  */
#define TraceMode        0      /* R/W  */

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
ejtag_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
               const urj_param_t *cmd_params[])
{
    return urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
ejtag_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("EJTAG compatible bus driver via PrAcc (JTAG part No. %d)\n"),
             i);
}

static uint32_t
reg_value (urj_tap_register_t *reg)
{
    uint32_t retval = 0;
    int i;

    for (i = 0; i < reg->len; i++)
    {
        if (reg->data[i])
            retval |= (1 << i);
    }
    return retval;
}

static uint32_t
ejtag_run_pracc (urj_bus_t *bus, const uint32_t *code, unsigned int len)
{
    urj_data_register_t *ejaddr, *ejdata, *ejctrl;
    int i, pass;
    uint32_t addr, data, retval;

    ejaddr = urj_part_find_data_register (bus->part, "EJADDRESS");
    ejdata = urj_part_find_data_register (bus->part, "EJDATA");
    ejctrl = urj_part_find_data_register (bus->part, "EJCONTROL");
    if (!(ejaddr && ejdata && ejctrl))
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("EJADDRESS, EJDATA or EJCONTROL register not found"));
        return 0;
    }

    urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
    urj_tap_chain_shift_instructions (bus->chain);

    pass = 0;
    retval = 0;

    for (;;)
    {
        ejctrl->in->data[PrAcc] = 1;
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        urj_tap_chain_shift_data_registers (bus->chain, 1);

        urj_log (URJ_LOG_LEVEL_ALL,  "ctrl=%s\n",
                 urj_tap_register_get_string (ejctrl->out));

        if (ejctrl->out->data[Rocc])
        {
            urj_error_set (URJ_ERROR_BUS, _("Reset occurred, ctrl=%s"),
                           urj_tap_register_get_string (ejctrl->out));
            bus->initialized = 0;
            break;
        }
        if (!ejctrl->out->data[PrAcc])
        {
            urj_error_set (URJ_ERROR_BUS, _("No processor access, ctrl=%s"),
                           urj_tap_register_get_string (ejctrl->out));
            bus->initialized = 0;
            break;
        }

        urj_part_set_instruction (bus->part, "EJTAG_ADDRESS");
        urj_tap_chain_shift_instructions (bus->chain);

        urj_tap_chain_shift_data_registers (bus->chain, 1);
        addr = reg_value (ejaddr->out);
        if (addr & 3)
        {
            urj_error_set (URJ_ERROR_BUS,
                           _("PrAcc bad alignment: addr=0x%08lx"),
                           (long unsigned) addr);
            addr &= ~3;
        }

        urj_part_set_instruction (bus->part, "EJTAG_DATA");
        urj_tap_chain_shift_instructions (bus->chain);

        urj_tap_register_fill (ejdata->in, 0);

        if (ejctrl->out->data[PRnW])
        {
            urj_tap_chain_shift_data_registers (bus->chain, 1);
            data = reg_value (ejdata->out);
            urj_log (URJ_LOG_LEVEL_ALL,
                     _("%s(%d) PrAcc write: addr=0x%08lx data=0x%08lx\n"),
                     __FILE__, __LINE__,
                     (long unsigned) addr, (long unsigned) data);
            if (addr == UINT32_C (0xff200000))
            {
                /* Return value from the target CPU.  */
                retval = data;
            }
            else
            {
                urj_error_set (URJ_ERROR_BUS,
                               _("Unknown write addr=0x%08lx data=0x%08lx"),
                               (long unsigned) addr, (long unsigned) data);
            }
        }
        else
        {
            if (addr == UINT32_C (0xff200200) && pass++)
                break;

            data = 0;
            if (addr >= 0xff200200 && addr < 0xff200200 + (len << 2))
            {
                data = code[(addr - 0xff200200) >> 2];

                for (i = 0; i < 32; i++)
                    ejdata->in->data[i] = (data >> i) & 1;
            }
            urj_log (URJ_LOG_LEVEL_ALL,
                     "%s(%d) PrAcc read: addr=0x%08lx data=0x%08lx\n",
                     __FILE__, __LINE__,
                     (long unsigned) addr, (long unsigned) data);
            urj_tap_chain_shift_data_registers (bus->chain, 0);
        }

        urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
        urj_tap_chain_shift_instructions (bus->chain);

        ejctrl->in->data[PrAcc] = 0;
        urj_tap_chain_shift_data_registers (bus->chain, 0);
    }
    return retval;
}

static int
ejtag_bus_init (urj_bus_t *bus)
{
    urj_data_register_t *ejctrl, *ejimpl, *ejaddr, *ejdata;
    uint32_t code[4] = {
        0x3c04ff20,             // lui $4,0xff20
        0x349f0200,             // ori $31,$4,0x0200
        0x03e00008,             // jr $31
        0x3c030000              // lui $3,0
    };

    if (urj_tap_state (bus->chain) != URJ_TAP_STATE_RUN_TEST_IDLE)
    {
        /* silently skip initialization if TAP isn't in RUNTEST/IDLE state
           this is required to avoid interfering with detect when initbus
           is contained in the part description file
           URJ_BUS_INIT() will be called latest by URJ_BUS_PREPARE() */
        return URJ_STATUS_OK;
    }

    ejctrl = urj_part_find_data_register (bus->part, "EJCONTROL");
    ejimpl = urj_part_find_data_register (bus->part, "EJIMPCODE");
    ejaddr = urj_part_find_data_register (bus->part, "EJADDRESS");
    ejdata = urj_part_find_data_register (bus->part, "EJDATA");
    if (!(ejctrl && ejimpl))
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("EJCONTROL or EJIMPCODE register not found"));
        return URJ_STATUS_FAIL;
    }

    urj_part_set_instruction (bus->part, "EJTAG_IMPCODE");
    urj_tap_chain_shift_instructions (bus->chain);
    urj_tap_chain_shift_data_registers (bus->chain, 0); //Write
    urj_tap_chain_shift_data_registers (bus->chain, 1); //Read
    urj_log (URJ_LOG_LEVEL_NORMAL, "ImpCode=%s %08lX\n",
             urj_tap_register_get_string (ejimpl->out),
             (long unsigned) reg_value (ejimpl->out));
    BP->impcode = reg_value (ejimpl->out);

    switch (EJTAG_VER)
    {
    case EJTAG_20:
        urj_log (URJ_LOG_LEVEL_NORMAL, "EJTAG version: <= 2.0\n");
        break;
    case EJTAG_25:
        urj_log (URJ_LOG_LEVEL_NORMAL, "EJTAG version: 2.5\n");
        break;
    case EJTAG_26:
        urj_log (URJ_LOG_LEVEL_NORMAL, "EJTAG version: 2.6\n");
        break;
    case EJTAG_31:
        urj_log (URJ_LOG_LEVEL_NORMAL, "EJTAG version: 3.1\n");
        break;
    default:
        urj_log (URJ_LOG_LEVEL_NORMAL, "EJTAG version: unknown (%lu)\n",
                 (long unsigned) EJTAG_VER);
    }
    urj_log (URJ_LOG_LEVEL_NORMAL,
             "EJTAG Implementation flags:%s%s%s%s%s%s%s\n",
             (BP->impcode & (1 << 28)) ? " R3k" : " R4k",
             (BP->impcode & (1 << 24)) ? " DINTsup" : "",
             (BP->impcode & (1 << 22)) ? " ASID_8" : "",
             (BP->impcode & (1 << 21)) ? " ASID_6" : "",
             (BP->impcode & (1 << 16)) ? " MIPS16" : "",
             (BP->impcode & (1 << 14)) ? " NoDMA" : " DMA",
             (BP->impcode & (1)) ? " MIPS64" : " MIPS32");

    if (EJTAG_VER >= EJTAG_25)
    {
        urj_part_set_instruction (bus->part, "EJTAGBOOT");
        urj_tap_chain_shift_instructions (bus->chain);
    }
    urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
    urj_tap_chain_shift_instructions (bus->chain);
    //Reset
    urj_tap_register_fill (ejctrl->in, 0);
    ejctrl->in->data[PrRst] = 1;
    ejctrl->in->data[PerRst] = 1;
    urj_tap_chain_shift_data_registers (bus->chain, 0); //Write
    ejctrl->in->data[PrRst] = 0;
    ejctrl->in->data[PerRst] = 0;
    urj_tap_chain_shift_data_registers (bus->chain, 0); //Write
//
    if (EJTAG_VER == EJTAG_20)
    {
        if (!(ejaddr && ejdata))
        {
            urj_error_set (URJ_ERROR_NOTFOUND,
                           _("EJADDRESS or EJDATA register not found"));
            return URJ_STATUS_FAIL;
        }
        // Try enabling memory write on EJTAG_20 (BCM6348)
        // Badly Copied from HairyDairyMaid V4.8
        //ejtag_dma_write(0xff300000, (ejtag_dma_read(0xff300000) & ~(1<<2)) );
        urj_log (URJ_LOG_LEVEL_ALL, "Set Address to READ from\n");
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG ADDRESS Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_ADDRESS");
        urj_tap_chain_shift_instructions (bus->chain);
        //Set to Debug Control Register Address, 0xFF300000
        urj_tap_register_init (ejaddr->in,
                               "11111111001100000000000000000000");
        urj_log (URJ_LOG_LEVEL_ALL, "Write to ejaddr->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejaddr->in),
                 (unsigned long) reg_value (ejaddr->in));
        urj_tap_chain_shift_data_registers (bus->chain, 0);     //Write
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG CONTROL Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
        urj_tap_chain_shift_instructions (bus->chain);
        //Set some bits in CONTROL Register 0x00068B00
        urj_tap_register_fill (ejctrl->in, 0);  // Clear Register
        ejctrl->in->data[PrAcc] = 1;    // 18----|||
        ejctrl->in->data[DmaAcc] = 1;   // 17----|||
        ejctrl->in->data[ProbEn] = 1;   // 15-----||
        ejctrl->in->data[DStrt] = 1;    // 11------|
        ejctrl->in->data[DrWn] = 1;     // 9-------|
        ejctrl->in->data[Dsz1] = 1;     // 8-------| DMA_WORD = 0x00000100 = Bit8
        urj_tap_chain_shift_data_registers (bus->chain, 1);     //WriteRead
        urj_log (URJ_LOG_LEVEL_ALL, "Write To ejctrl->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->in),
                 (unsigned long) reg_value (ejctrl->in));
        urj_log (URJ_LOG_LEVEL_ALL, "Read From ejctrl->out   =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->out),
                 (unsigned long) reg_value (ejctrl->out));
        do
        {
            urj_log (URJ_LOG_LEVEL_ALL, "Wait for DStrt to clear\n");
            urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
            urj_tap_chain_shift_instructions (bus->chain);
            urj_tap_register_fill (ejctrl->in, 0);
            //Set some bits in CONTROL Register 0x00068000
            ejctrl->in->data[PrAcc] = 1;        // 18----||
            ejctrl->in->data[DmaAcc] = 1;       // 17----||
            ejctrl->in->data[ProbEn] = 1;       // 15-----|
            urj_tap_chain_shift_data_registers (bus->chain, 1); //WriteRead
            urj_log (URJ_LOG_LEVEL_ALL, "Write To ejctrl->in     =%s %08lX\n",
                     urj_tap_register_get_string (ejctrl->in),
                     (unsigned long) reg_value (ejctrl->in));
            urj_log (URJ_LOG_LEVEL_ALL, "Read From ejctrl->out   =%s %08lX\n",
                     urj_tap_register_get_string( ejctrl->out),
                     (unsigned long) reg_value (ejctrl->out));
        }
        while (ejctrl->out->data[DStrt] == 1);
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG DATA Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_DATA");
        urj_tap_chain_shift_instructions (bus->chain);
        urj_tap_register_fill (ejdata->in, 0);  // Clear Register
        urj_tap_chain_shift_data_registers (bus->chain, 1);     //WriteRead
        urj_log (URJ_LOG_LEVEL_ALL,  "Write To ejdata->in    =%s %08lX\n",
                 urj_tap_register_get_string (ejdata->in),
                 (unsigned long) reg_value (ejdata->in));
        urj_log (URJ_LOG_LEVEL_ALL,  "Read From ejdata->out  =%s %08lX\n",
                 urj_tap_register_get_string (ejdata->out),
                 (unsigned long) reg_value (ejdata->out));
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG CONTROL Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
        urj_tap_chain_shift_instructions (bus->chain);
        urj_tap_register_fill (ejctrl->in, 0);
        //Set some bits in CONTROL Register 0x00048000
        ejctrl->in->data[PrAcc] = 1;    // 18----||
        ejctrl->in->data[ProbEn] = 1;   // 15-----|
        urj_tap_chain_shift_data_registers (bus->chain, 1);     //WriteRead
        urj_log (URJ_LOG_LEVEL_ALL, "Write To ejctrl->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->in),
                 (unsigned long) reg_value (ejctrl->in));
        urj_log (URJ_LOG_LEVEL_ALL, "Read From ejctrl->out   =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->out),
                 (unsigned long) reg_value (ejctrl->out));
        if (ejctrl->out->data[DeRR] == 1)
        {
            urj_error_set (URJ_ERROR_BUS_DMA, "DMA READ ERROR");
        }
        //Now have data from DCR, need to reset the MP Bit (2) and write it back out
        urj_tap_register_init (ejdata->in,
                               urj_tap_register_get_string (ejdata->out));
        ejdata->in->data[MemProt] = 0;
        urj_log (URJ_LOG_LEVEL_ALL, "Need to Write ejdata-> =%s %08lX\n",
                 urj_tap_register_get_string (ejdata->in),
                 (unsigned long) reg_value (ejdata->in));

        // Now the Write
        urj_log (URJ_LOG_LEVEL_ALL, "Set Address To Write To\n");
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG ADDRESS Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_ADDRESS");
        urj_tap_chain_shift_instructions (bus->chain);
        urj_tap_register_init (ejaddr->in,
                               "11111111001100000000000000000000");
        urj_log (URJ_LOG_LEVEL_ALL, "Write to ejaddr->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejaddr->in),
                 (unsigned long) reg_value (ejaddr->in));
        //This appears to be a write with NO Read
        urj_tap_chain_shift_data_registers (bus->chain, 0);     //Write
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG DATA Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_DATA");
        urj_tap_chain_shift_instructions (bus->chain);
        //The value is already in ejdata->in, so write it
        urj_log (URJ_LOG_LEVEL_ALL, "Write To ejdata->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejdata->in),
                 (unsigned long) reg_value (ejdata->in));
        urj_tap_chain_shift_data_registers (bus->chain, 0);     //Write
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG CONTROL Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
        urj_tap_chain_shift_instructions (bus->chain);

        //Set some bits in CONTROL Register
        urj_tap_register_fill (ejctrl->in, 0);  // Clear Register
        ejctrl->in->data[DmaAcc] = 1;   // 17
        ejctrl->in->data[Dsz1] = 1;     // DMA_WORD = 0x00000100 = Bit8
        ejctrl->in->data[DStrt] = 1;    // 11
        ejctrl->in->data[ProbEn] = 1;   // 15
        ejctrl->in->data[PrAcc] = 1;    // 18
        urj_tap_chain_shift_data_registers (bus->chain, 1);     //Write/Read
        urj_log (URJ_LOG_LEVEL_ALL, "Write to ejctrl->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->in),
                 (unsigned long) reg_value (ejctrl->in));
        urj_log (URJ_LOG_LEVEL_ALL, "Read from ejctrl->out   =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->out),
                 (unsigned long) reg_value (ejctrl->out));
        do
        {
            urj_log (URJ_LOG_LEVEL_ALL, "Wait for DStrt to clear\n");
            //Might not need these 2 lines
            urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
            urj_tap_chain_shift_instructions (bus->chain);
            ejctrl->in->data[DmaAcc] = 1;       // 17
            ejctrl->in->data[ProbEn] = 1;       // 15
            ejctrl->in->data[PrAcc] = 1;        // 18
            urj_tap_chain_shift_data_registers (bus->chain, 1); //Write/Read
            urj_log (URJ_LOG_LEVEL_ALL, "Write to ejctrl->in     =%s %08lX\n",
                     urj_tap_register_get_string (ejctrl->in),
                     (unsigned long) reg_value (ejctrl->in));
            urj_log (URJ_LOG_LEVEL_ALL, "Read from ejctrl->out   =%s %08lX\n",
                     urj_tap_register_get_string (ejctrl->out),
                     (unsigned long) reg_value (ejctrl->out));
        }
        while (ejctrl->out->data[DStrt] == 1);
        urj_log (URJ_LOG_LEVEL_ALL, "Select EJTAG CONTROL Register\n");
        urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
        urj_tap_chain_shift_instructions (bus->chain);
        urj_tap_register_fill (ejctrl->in, 0);
        //Set some bits in CONTROL Register 0x00048000
        ejctrl->in->data[PrAcc] = 1;    // 18----||
        ejctrl->in->data[ProbEn] = 1;   // 15-----|
        urj_tap_chain_shift_data_registers (bus->chain, 1);     //Write/Read
        urj_log (URJ_LOG_LEVEL_ALL, "Write To ejctrl->in     =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->in),
                 (unsigned long) reg_value (ejctrl->in));
        urj_log (URJ_LOG_LEVEL_ALL, "Read From ejctrl->out   =%s %08lX\n",
                 urj_tap_register_get_string (ejctrl->out),
                 (unsigned long) reg_value (ejctrl->out));
        if (ejctrl->out->data[DeRR] == 1)
        {
            urj_error_set (URJ_ERROR_BUS_DMA, "DMA WRITE ERROR");
        }
    }

    urj_part_set_instruction (bus->part, "EJTAG_CONTROL");
    urj_tap_chain_shift_instructions (bus->chain);

    urj_tap_register_fill (ejctrl->in, 0);
    ejctrl->in->data[PrAcc] = 1;
    ejctrl->in->data[ProbEn] = 1;
    if (EJTAG_VER >= EJTAG_25)
    {
        ejctrl->in->data[ProbTrap] = 1;
        ejctrl->in->data[Rocc] = 1;
    }
    urj_tap_chain_shift_data_registers (bus->chain, 0);

    ejctrl->in->data[PrAcc] = 1;
    ejctrl->in->data[ProbEn] = 1;
    ejctrl->in->data[ProbTrap] = 1;
    ejctrl->in->data[JtagBrk] = 1;

    urj_tap_chain_shift_data_registers (bus->chain, 0);

    ejctrl->in->data[JtagBrk] = 0;
    urj_tap_chain_shift_data_registers (bus->chain, 1);

    if (!ejctrl->out->data[BrkSt])
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("Failed to enter debug mode, ctrl=%s"),
                       urj_tap_register_get_string (ejctrl->out));
        return URJ_STATUS_FAIL;
    }
    else
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, "Processor entered Debug Mode.\n");
    }
    if (ejctrl->out->data[Rocc])
    {
        ejctrl->in->data[Rocc] = 0;
        urj_tap_chain_shift_data_registers (bus->chain, 0);
        ejctrl->in->data[Rocc] = 1;
        urj_tap_chain_shift_data_registers (bus->chain, 1);
    }

    //HDM now Clears Watchdog


    ejtag_run_pracc (bus, code, 4);
    BP->adr_hi = 0;
    bus->initialized = 1;
    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
ejtag_bus_prepare (urj_bus_t *bus)
{
    if (!bus->initialized)
        URJ_BUS_INIT (bus);
}

/**
 * bus->driver->(*area)
 *
 */
static int
ejtag_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    if (adr < UINT32_C (0x20000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x00000000);
        area->length = UINT64_C (0x20000000);
        area->width = 8;
    }
    else if (adr < UINT32_C (0x40000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x20000000);
        area->length = UINT64_C (0x20000000);
        area->width = 16;
    }
    else if (adr < UINT32_C (0x60000000))
    {
        area->description = NULL;
        area->start = UINT32_C (0x40000000);
        area->length = UINT64_C (0x20000000);
        area->width = 32;
    }
    else
    {
        area->description = NULL;
        area->start = UINT32_C (0x60000000);
        area->length = UINT64_C (0xa0000000);
        area->width = 0;
    }
    return URJ_STATUS_OK;
}

static int
ejtag_gen_read (urj_bus_t *bus, uint32_t *code, uint32_t adr)
{
    uint16_t adr_hi, adr_lo;
    uint32_t *p = code;

    /* 16-bit signed offset, phys -> kseg1 */
    adr_lo = adr & 0xffff;
    adr_hi = ((adr >> 16) & 0x1fff);
    /* Increment adr_hi if adr_lo < 0 */
    adr_hi += (adr_lo >> 15);
    /* Bypass cache */
    adr_hi += 0xa000;

    if (BP->adr_hi != adr_hi)
    {
        BP->adr_hi = adr_hi;
        *p++ = 0x3c030000 | adr_hi;     // lui $3,adr_hi
    }
    switch (adr >> 29)
    {
    case 0:
        *p++ = 0x90620000 | adr_lo;     // lbu $2,adr_lo($3)
        break;
    case 1:
        *p++ = 0x94620000 | (adr_lo & ~1);      // lhu $2,adr_lo($3)
        break;
    case 2:
        *p++ = 0x8c620000 | (adr_lo & ~3);      // lw $2,adr_lo($3)
        break;
    default:                   /* unknown bus width */
        *p++ = 0x00001025;      // move $2,$0
        break;
    }
    *p++ = 0x03e00008;          // jr $31
    return p - code;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
ejtag_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    uint32_t code[3];

    ejtag_run_pracc (bus, code, ejtag_gen_read (bus, code, adr));
    urj_log (URJ_LOG_LEVEL_COMM, "URJ_BUS_READ_START: adr=0x%08lx\n",
             (long unsigned) adr);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
ejtag_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    uint32_t d;
    uint32_t code[4], *p = code;

    *p++ = 0xac820000;          // sw $2,0($4)
    p += ejtag_gen_read (bus, p, adr);

    d = ejtag_run_pracc (bus, code, p - code);

    urj_log (URJ_LOG_LEVEL_COMM,
             "URJ_BUS_READ_NEXT: adr=0x%08lx data=0x%08lx\n",
             (long unsigned) adr, (long unsigned) d);
    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
ejtag_bus_read_end (urj_bus_t *bus)
{
    uint32_t d;
    static const uint32_t code[2] = {
        0xac820000,             // sw $2,0($4)
        0x03e00008              // jr $31
    };

    d = ejtag_run_pracc (bus, code, 2);

    urj_log (URJ_LOG_LEVEL_COMM, "URJ_BUS_READ_END: data=0x%08lx\n",
             (long unsigned) d);
    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
ejtag_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    uint16_t adr_hi, adr_lo;
    uint32_t code[5], *p = code;

    /* 16-bit signed offset, phys -> kseg1 */
    adr_lo = adr & 0xffff;
    adr_hi = ((adr >> 16) & 0x1fff) + (adr_lo >> 15) + 0xa000;

    if (BP->adr_hi != adr_hi)
    {
        BP->adr_hi = adr_hi;
        *p++ = 0x3c030000 | adr_hi;     // lui $3,adr_hi
    }
    switch (adr >> 29)
    {
    case 0:
        *p++ = 0x34020000 | (data & 0xff);      // li $2,data
        *p++ = 0xa0620000 | adr_lo;     // sb $2,adr_lo($3)
        break;
    case 1:
        *p++ = 0x34020000 | (data & 0xffff);    // li $2,data
        *p++ = 0xa4620000 | (adr_lo & ~1);      // sh $2,adr_lo($3)
        break;
    case 2:
        *p++ = 0x3c020000 | (data >> 16);       // lui $2,data_hi
        *p++ = 0x34420000 | (data & 0xffff);    // ori $2,data_lo
        *p++ = 0xac620000 | (adr_lo & ~3);      // sw $2,adr_lo($3)
        break;
    }
    *p++ = 0x03e00008;          // jr $31

    ejtag_run_pracc (bus, code, p - code);

    urj_log (URJ_LOG_LEVEL_COMM,
             "URJ_BUS_WRITE: adr=0x%08lx data=0x%08lx\n",
             (long unsigned) adr, (long unsigned) data);
}

const urj_bus_driver_t urj_bus_ejtag_bus = {
    "ejtag",
    N_("EJTAG compatible bus driver via PrAcc"),
    ejtag_bus_new,
    urj_bus_generic_free,
    ejtag_bus_printinfo,
    ejtag_bus_prepare,
    ejtag_bus_area,
    ejtag_bus_read_start,
    ejtag_bus_read_next,
    ejtag_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    ejtag_bus_write,
    ejtag_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
