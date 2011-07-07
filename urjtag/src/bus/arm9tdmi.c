/*
 * $Id$
 *
 * ARM9TDMI core bus driver
 * Copyright (C) 2003, Rongkai zhan <zhanrk@163.com>
 * Copyright (C) 2009, Jochen Friedrich <jochen@scram.de>
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
 */

#define ARM9DEBUG 0

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

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
    uint32_t chain;           /* Chain number */
} bus_params_t;

#define BP              ((bus_params_t *) bus->params)

#define ARM9TDMI_ICE_DBGCTL  0x00
#define ARM9TDMI_ICE_DBGSTAT 0x01

#define DEBUG_SPEED          0
#define SYSTEM_SPEED         1

#define ARM_NOP 0xE1A00000

static urj_data_register_t *scann = NULL;
static urj_data_register_t *scan1 = NULL;
static urj_data_register_t *scan2 = NULL;

static uint32_t _data_read;

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
arm9tdmi_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                  const urj_param_t *cmd_params[])
{
    return urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
arm9tdmi_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("ARM9TDMI compatible bus driver (JTAG part No. %d)\n"),
             i);
}

/**
 * helper functions
 *
 */

#if (ARM9DEBUG)
static void
arm9tdmi_debug_in_reg(urj_data_register_t *reg)
{
    int i;

    urj_log(URJ_LOG_LEVEL_ALL, "in  :");
    for (i = 0; i < reg->in->len; i++)
        urj_log(URJ_LOG_LEVEL_ALL, reg->in->data[i]?"1":"0");
    urj_log(URJ_LOG_LEVEL_ALL, "\n");
}

static void
arm9tdmi_debug_out_reg(urj_data_register_t *reg)
{
    int i;

    urj_log(URJ_LOG_LEVEL_ALL, "out :");
    for (i = 0; i < reg->out->len; i++)
        urj_log(URJ_LOG_LEVEL_ALL, reg->out->data[i]?"1":"0");
    urj_log(URJ_LOG_LEVEL_ALL, "\n");
}
#endif

static void
arm9tdmi_exec_instruction(urj_bus_t *bus, unsigned int c1_inst, unsigned int c1_data, unsigned int flags)
{
    int i;

    for (i = 0; i < 32; i++)
        scan1->in->data[66-i] = (c1_inst >> i) & 1;
    scan1->in->data[34] = flags;
    scan1->in->data[33] = 0;
    scan1->in->data[32] = 0;
    for (i = 0; i < 32; i++)
        scan1->in->data[i] = (c1_data >> i) & 1;
#if (ARM9DEBUG)
    arm9tdmi_debug_in_reg(scan1);
#endif
    urj_tap_chain_shift_data_registers (bus->chain, 1);
#if (ARM9DEBUG)
    arm9tdmi_debug_out_reg(scan1);
#endif
}

static void
arm9tdmi_select_scanchain(urj_bus_t *bus, unsigned int chain)
{
    int i;

    urj_part_set_instruction (bus->part, "SCAN_N");
    urj_tap_chain_shift_instructions (bus->chain);

    for (i = 0; i < scann->in->len; i++)
        scann->in->data[i] = (chain >> i) & 1;
    urj_tap_chain_shift_data_registers (bus->chain, 0);
}

static void
arm9tdmi_ice_read(urj_bus_t *bus, unsigned int reg_addr, unsigned int *reg_val)
{
    int i;

    for (i = 0; i < 32; i++)
        scan2->in->data[i] = 0;
    for (i = 0; i < 5; i++)
        scan2->in->data[i+32] = (reg_addr >> i) & 1;
    scan2->in->data[37] = 0;
    urj_tap_chain_shift_data_registers (bus->chain, 1);

    for (i = 0; i < 32; i++)
        if (scan2->out->data[i])
            *reg_val |= (1 << i);
}

static void
arm9tdmi_ice_write(urj_bus_t *bus, unsigned int reg_addr, unsigned int reg_val)
{
    int i;

    for (i = 0; i < 32; i++)
        scan2->in->data[i] = (reg_val >> i) & 1;
    for (i = 0; i < 5; i++)
        scan2->in->data[i+32] = (reg_addr >> i) & 1;
    scan2->in->data[37] = 1;
    urj_tap_chain_shift_data_registers (bus->chain, 0);
}

/**
 * low-level memory write
 *
 */

static void
arm9tdmi_write(urj_bus_t *bus, unsigned int addr, unsigned int data, unsigned int sz)
{
    unsigned int c1_inst, c1_data;

    /*
     * Load R0 with the address to write.
     * Load R1 with the data to write.
     */

    c1_inst = 0xE59F0000; /* LDR R0, [PC] */
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_inst = 0xE59F1000; /* LDR R1, [PC] */
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);

    /* Flush pipeline before SYSTEM_SPEED access */

    c1_inst = ARM_NOP;
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_data = addr;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_data = data;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);

    if (sz == 32)
        c1_inst = 0xE5801000; /* STR R1, [R0] */

    else if (sz == 16)
        c1_inst = 0xE1C010B0; /* STRH R1, [R0] */

    else if (sz == 8)
        c1_inst = 0xE5C01000; /* STRB R1, [R0] */

    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);

    /*
     * Execute instruction at SYSTEM_SPEED as we need to access memory
     */
    c1_inst = ARM_NOP;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, SYSTEM_SPEED);
    urj_tap_chain_flush(bus->chain);
    /* Write RESTART instruction into the TAP controller.
     * When the state machine enters the Run-Test/Idle state,
     * the ARM9TDMI core will revert back to system mode,
     * and it will resynchronize clock to MCLK.
     */
    urj_part_set_instruction (bus->part, "RESTART");
    urj_tap_chain_shift_instructions (bus->chain);

    /*
     * Now, the ARM9TDMI core re-entered the debug state.
     * Before the debug session continues, we must load the
     * TAP controller with the INTEST instruction. We can use
     * the instruction "STR R1, [R1]" running at debug-speed to
     * read out the contents of register R1.
     */

    urj_part_set_instruction (bus->part, "INTEST1");
    urj_tap_chain_shift_instructions_mode (bus->chain, 0, 1,
                                           URJ_CHAIN_EXITMODE_UPDATE);
}

/**
 * low level memory read
 *
 */
static unsigned int
arm9tdmi_read (urj_bus_t *bus, unsigned int addr, unsigned int sz)
{
    unsigned int c1_inst, c1_data, result, i;

    /*
     * Load R0 with the address to read
     */
    c1_inst = 0xE59F0000; /* LDR R0, [PC] */
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_inst = ARM_NOP;
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_data = addr;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);

    if (sz == 32)
        c1_inst = 0xE5901000; /* LDR R1, [R0] */

    else if (sz == 16)
        c1_inst = 0xE1D010B0; /* LDRH R1, [R0] */

    else if (sz == 8)
        c1_inst = 0xE5D01000; /* LDRB R1, [R0] */

    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);

    /*
     * Execute instruction at SYSTEM_SPEED as we need to access memory
     */
    c1_inst = ARM_NOP;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, SYSTEM_SPEED);
    urj_tap_chain_flush(bus->chain);

    /* Write RESTART instruction into the TAP controller.
     * When the state machine enters the Run-Test/Idle state,
     * the ARM9TDMI core will revert back to system mode,
     * and it will resynchronize clock to MCLK.
     */
    urj_part_set_instruction (bus->part, "RESTART");
    urj_tap_chain_shift_instructions (bus->chain);

    /*
     * Now, the ARM9TDMI core re-entered the debug state.
     * Before the debug session continues, we must load the
     * TAP controller with the INTEST instruction. We can use
     * the instruction "STR R1, [PC]" running at debug-speed to
     * read out the contents of register R1.
     */

    urj_part_set_instruction (bus->part, "INTEST1");
    urj_tap_chain_shift_instructions_mode (bus->chain, 0, 1,
                                           URJ_CHAIN_EXITMODE_UPDATE);

    c1_inst = 0xE58F1000; /* STR R1, [PC] */
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    c1_inst = ARM_NOP;
    c1_data = 0;
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);

    result = 0;
    for (i = 0; i < 32; i++)
    {
        if (scan1->out->data[i])
            result |= (1 << i);
    }
    arm9tdmi_exec_instruction(bus, c1_inst, c1_data, DEBUG_SPEED);
    return result;
}

/**
 * bus->driver->(*initbus)
 *
 */
static int
arm9tdmi_bus_init (urj_bus_t *bus)
{
    unsigned int i, status, success;

    if (urj_tap_state (bus->chain) != URJ_TAP_STATE_RUN_TEST_IDLE)
    {
        /* silently skip initialization if TAP isn't in RUNTEST/IDLE state
           this is required to avoid interfering with detect when initbus
           is contained in the part description file
           URJ_BUS_INIT() will be called latest by URJ_BUS_PREPARE() */
        return URJ_STATUS_OK;
    }

    if (scann == NULL)
        scann = urj_part_find_data_register (bus->part, "SCANN");
    if (scan1 == NULL)
        scan1 = urj_part_find_data_register (bus->part, "SCAN1");
    if (scan2 == NULL)
        scan2 = urj_part_find_data_register (bus->part, "SCAN2");

    if (!(scann))
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("SCANN register"));
        return URJ_STATUS_FAIL;
    }
    if (!(scan1))
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("SCAN1 register"));
        return URJ_STATUS_FAIL;
    }
    if (!(scan2))
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       _("SCAN2 register"));
        return URJ_STATUS_FAIL;
    }

    /*
     * select scan chain 2 -- EmbeddedICE-RT
     */
    arm9tdmi_select_scanchain(bus, 2);

    urj_part_set_instruction (bus->part, "INTEST2");
    urj_tap_chain_shift_instructions (bus->chain);

    arm9tdmi_ice_write(bus, ARM9TDMI_ICE_DBGCTL, 0x3);

    urj_part_set_instruction (bus->part, "RESTART");
    urj_tap_chain_shift_instructions (bus->chain);

    i = 0;
    success = 0;
    status = 0;

    while (i++ < 10) {
        urj_part_set_instruction (bus->part, "INTEST2");
        urj_tap_chain_shift_instructions (bus->chain);

        arm9tdmi_ice_read(bus, ARM9TDMI_ICE_DBGSTAT, &status);

        if (status & 0x01) {
            success = 1;
            break;
        }
        urj_part_set_instruction (bus->part, "RESTART");
        urj_tap_chain_shift_instructions (bus->chain);
        usleep(100);
    }

    if (!success)
    {
        urj_error_set (URJ_ERROR_TIMEOUT,
                       _("Failed to enter debug mode, ctrl=%s"),
                       urj_tap_register_get_string (scan2->out));
        return URJ_STATUS_FAIL;
    }

    arm9tdmi_ice_write(bus, ARM9TDMI_ICE_DBGCTL, 0x00);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("The target is halted in "));
    if (status & 0x10)
        urj_log (URJ_LOG_LEVEL_NORMAL, _("THUMB mode.\n"));
    else
        urj_log (URJ_LOG_LEVEL_NORMAL, _("ARM mode.\n"));

    /* select scan chain 1, and use INTEST instruction */
    arm9tdmi_select_scanchain(bus, 1);

    urj_part_set_instruction (bus->part, "INTEST1");
    urj_tap_chain_shift_instructions_mode (bus->chain, 0, 1,
                                           URJ_CHAIN_EXITMODE_UPDATE);

    bus->initialized = 1;
    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
arm9tdmi_bus_prepare (urj_bus_t *bus)
{
    if (!bus->initialized)
        URJ_BUS_INIT (bus);
}

/**
 * bus->driver->(*area)
 *
 */
static int
arm9tdmi_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{

    if (adr < UINT32_C (0xF0000000))
    {
        area->description = "USEG : User addresses";
        area->start = UINT32_C (0x00000000);
        area->length = UINT64_C (0xF0000000);
        area->width = 32;
    }
    else
    {
        area->description = "FLASH : Addresses in flash (boot=0xffff0000)";
        area->start = UINT32_C (0xF0000000);
        area->length = UINT64_C (0x10000000);
        area->width = 16;
    }
    return URJ_STATUS_OK;
}

static int
get_sz (uint32_t adr)
{
    urj_bus_area_t area;

    arm9tdmi_bus_area (NULL, adr, &area);

    return area.width;
}

/**
 * bus->driver->(*write)
 *
 */
static void
arm9tdmi_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_log (URJ_LOG_LEVEL_ALL, "%s:adr=0x%lx,got=0x%lx\n", __func__,
             (long unsigned) adr, (long unsigned) data);
    arm9tdmi_write (bus, adr, data, get_sz (adr));
}

/**
 * bus->driver->(*read)
 *
 */
static uint32_t
arm9tdmi_bus_read (urj_bus_t *bus, uint32_t adr)
{
    int data = arm9tdmi_read (bus, adr, get_sz (adr));
    urj_log (URJ_LOG_LEVEL_ALL, "%s:adr=0x%lx,got=0x%lx\n", __func__,
             (long unsigned) adr, (long unsigned) data);
    return data;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
arm9tdmi_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    _data_read = arm9tdmi_read (bus, adr, get_sz (adr));
    urj_log (URJ_LOG_LEVEL_ALL, "%s:adr=0x%lx, got=0x%lx\n", __func__,
             (long unsigned) adr, (long unsigned) _data_read);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
arm9tdmi_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    uint32_t tmp_value = _data_read;
    _data_read = arm9tdmi_read (bus, adr, get_sz (adr));
    urj_log (URJ_LOG_LEVEL_ALL, "%s:adr=0x%lx, got=0x%lx\n", __func__,
             (long unsigned) adr, (long unsigned) _data_read);
    return tmp_value;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
arm9tdmi_bus_read_end (urj_bus_t *bus)
{
    return _data_read;
}


const urj_bus_driver_t urj_bus_arm9tdmi_bus = {
    "arm9tdmi",
    N_("ARM9TDMI compatible bus driver"),
    arm9tdmi_bus_new,
    urj_bus_generic_free,
    arm9tdmi_bus_printinfo,
    arm9tdmi_bus_prepare,
    arm9tdmi_bus_area,
    arm9tdmi_bus_read_start,
    arm9tdmi_bus_read_next,
    arm9tdmi_bus_read_end,
    arm9tdmi_bus_read,
    urj_bus_generic_write_start,
    arm9tdmi_bus_write,
    arm9tdmi_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
