/*
 * $Id$
 *
 * AVR32 multi-mode bus driver
 *
 * Copyright (c) 2008 Gabor Juhos <juhosg@openwrt.org>
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
 * Documentation:
 * [1] Atmel Corporation, "AT32AP7000 - High Performance, Low Power
 *     AVR(R)32 32-Bit Microcontroller", Rev. 32003K-AVR32-10/07
 */

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include <urjtag/part.h>
#include <urjtag/part_instruction.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>
#include <urjtag/data_register.h>
#include <urjtag/tap.h>
#include <urjtag/tap_register.h>

#include "buses.h"
#include "generic_bus.h"

typedef struct
{
    unsigned int mode;

    unsigned int slave;
    uint32_t addr_mask;

    uint32_t rwcs_rd;
    uint32_t rwcs_wr;
} bus_params_t;

#define BUS_MODE_OCD    0
#define BUS_MODE_HSBC   1
#define BUS_MODE_HSBU   2
#define BUS_MODE_x8     3
#define BUS_MODE_x16    4
#define BUS_MODE_x32    5
#define BUS_MODE_ERROR  UINT_MAX

#define BP              ((bus_params_t *) bus->params)
#define SLAVE           (BP->slave)
#define MODE            (BP->mode)
#define ADDR_MASK       (BP->addr_mask)
#define RWCS_RD         (BP->rwcs_rd)
#define RWCS_WR         (BP->rwcs_wr)

/* ------------------------------------------------------------------------- */
#define SAB_SLAVE_OCD           1
#define SAB_SLAVE_HSB_CACHED    4
#define SAB_SLAVE_HSB_UNCACHED  5

#define ACCESS_MODE_WRITE       0
#define ACCESS_MODE_READ        1

#define ACCESS_STATUS_OK        0
#define ACCESS_STATUS_ERR       -1

#define SAB_OCD_AREA_SIZE       UINT64_C(0x1000)
#define SAB_OCD_ADDR_MASK       0xfff
#define SAB_HSB_AREA_SIZE       UINT64_C(0x100000000)
#define SAB_HSB_ADDR_MASK       0xffffffff

/* OCD register addresses */
#define OCD_REG_RWCS            0x1c
#define OCD_REG_RWA             0x24
#define OCD_REG_RWD             0x28

/* OCD RWCS register definitions */
#define OCD_RWCS_AC             0x80000000      /* start access */
#define OCD_RWCS_SZ32           0x10000000      /* word access */
#define OCD_RWCS_SZ16           0x08000000      /* half-word access */
#define OCD_RWCS_SZ8            0x00000000      /* byte access */
#define OCD_RWCS_RW             0x40000000      /* access mode 0:read, 1: write */
#define OCD_RWCS_CNT_S          2
#define OCD_RWCS_ERR            0x00000002      /* last access generated and error */
#define OCD_RWCS_DV             0x00000001      /* data is valid */

/* shorthands */
#define OCD_RWCS_READONE        (OCD_RWCS_AC | (1 << OCD_RWCS_CNT_S))
#define OCD_RWCS_WRITEONE       (OCD_RWCS_READONE | OCD_RWCS_RW)
#define OCD_RWCS_READ8          (OCD_RWCS_READONE | OCD_RWCS_SZ8)
#define OCD_RWCS_WRITE8         (OCD_RWCS_WRITEONE | OCD_RWCS_SZ8)
#define OCD_RWCS_READ16         (OCD_RWCS_READONE | OCD_RWCS_SZ16)
#define OCD_RWCS_WRITE16        (OCD_RWCS_WRITEONE | OCD_RWCS_SZ16)
#define OCD_RWCS_READ32         (OCD_RWCS_READONE | OCD_RWCS_SZ32)
#define OCD_RWCS_WRITE32        (OCD_RWCS_WRITEONE | OCD_RWCS_SZ32)

#define DBG_BASIC       0x0001
#define DBG_SHIFT       0x0002
#define DBG_TRACE       0x8000

#define DBG_ALL         0xffff

#define DBG_LEVEL       0

#define DBG(t, f, ...)                                          \
        do {                                                    \
                if (DBG_LEVEL & (t))                            \
                        printf( f, ## __VA_ARGS__ );            \
        } while (0)

#define TRACE_ENTER()   DBG(DBG_TRACE, ">>> %s", __FUNCTION__ )
#define TRACE_EXIT()    DBG(DBG_TRACE, "<<< %s", __FUNCTION__ )

/* ------------------------------------------------------------------------- */

static inline void
register_set_bit (urj_tap_register_t *tr, unsigned int bitno,
                  unsigned int val)
{
    tr->data[bitno] = (val) ? 1 : 0;
}

static inline int
register_get_bit (urj_tap_register_t *tr, unsigned int bitno)
{
    return (tr->data[bitno] & 1) ? 1 : 0;
}

static inline void
shift_instr (urj_bus_t *bus, unsigned int bit)
{
    urj_tap_register_t *r = bus->part->active_instruction->out;

    do
    {
        DBG (DBG_SHIFT, _("%s: instr=%s\n"), __FUNCTION__,
             urj_tap_register_get_string (bus->part->active_instruction->
                                          value));
        urj_tap_chain_shift_instructions_mode (bus->chain, 1, 1,
                                               URJ_CHAIN_EXITMODE_IDLE);
        DBG (DBG_SHIFT, _("%s: ret=%s\n"), __FUNCTION__,
             urj_tap_register_get_string (r));
        /* TODO: add timeout checking */
    }
    while (register_get_bit (r, bit));
}

static inline void
shift_data (urj_bus_t *bus, unsigned int bit)
{
    urj_data_register_t *dr = bus->part->active_instruction->data_register;

    do
    {
        DBG (DBG_SHIFT, _("%s: data=%s\n"), __FUNCTION__,
             urj_tap_register_get_string (dr->in));
        urj_tap_chain_shift_data_registers (bus->chain, 1);
        DBG (DBG_SHIFT, _("%s: data out=%s\n"), __FUNCTION__,
             urj_tap_register_get_string (dr->out));
        /* TODO: add timeout checking */
    }
    while (register_get_bit (dr->out, bit));
}

/* ------------------------------------------------------------------------- */

static void
mwa_scan_in_instr (urj_bus_t *bus)
{
    shift_instr (bus, 2);
}

static void
mwa_scan_in_addr (urj_bus_t *bus, unsigned int slave, uint32_t addr, int mode)
{
    urj_tap_register_t *r = bus->part->active_instruction->data_register->in;
    int i;

    DBG (DBG_BASIC, _("%s: slave=%01x, addr=%08lx, %s\n"),
         __FUNCTION__, slave,
         (long unsigned) addr,
         (mode == ACCESS_MODE_READ) ? "READ" : "WRITE");

    /* set slave bits */
    for (i = 0; i < 4; i++)
        register_set_bit (r, 31 + i, slave & (1 << i));

    /* set address bits */
    addr >>= 2;
    for (i = 0; i < 30; i++)
        register_set_bit (r, 1 + i, addr & (1 << i));

    /* set access mode */
    register_set_bit (r, 0, mode);

    shift_data (bus, 32);
}

static void
mwa_scan_in_data (urj_bus_t *bus, uint32_t data)
{
    urj_tap_register_t *r = bus->part->active_instruction->data_register->in;
    int i;

    DBG (DBG_BASIC, _("%s: data=%08lx\n"), __FUNCTION__,
         (long unsigned) data);

    register_set_bit (r, 0, 0);
    register_set_bit (r, 1, 0);
    register_set_bit (r, 2, 0);

    for (i = 0; i < 32; i++)
        register_set_bit (r, 3 + i, data & (1 << i));

    shift_data (bus, 0);
}

static void
mwa_scan_out_data (urj_bus_t *bus, uint32_t *pdata)
{
    urj_tap_register_t *r = bus->part->active_instruction->data_register->out;
    uint32_t data;
    int i;

    shift_data (bus, 32);

    data = 0;
    for (i = 0; i < 32; i++)
        data |= register_get_bit (r, i) << i;

    DBG (DBG_BASIC, _("%s: data=%08lx\n"), __FUNCTION__,
         (long unsigned) data);

    *pdata = data;
}

static inline void
mwa_read_word (urj_bus_t *bus, unsigned int slave, uint32_t addr,
               uint32_t *data)
{
    mwa_scan_in_instr (bus);
    mwa_scan_in_addr (bus, slave, addr, ACCESS_MODE_READ);
    mwa_scan_out_data (bus, data);
}

static inline void
mwa_write_word (urj_bus_t *bus, unsigned int slave, uint32_t addr,
                uint32_t data)
{
    mwa_scan_in_instr (bus);
    mwa_scan_in_addr (bus, slave, addr, ACCESS_MODE_WRITE);
    mwa_scan_in_data (bus, data);
}

/* ------------------------------------------------------------------------- */

static void
nexus_access_start (urj_bus_t *bus)
{
    shift_instr (bus, 2);
}

static void
nexus_access_end (urj_bus_t *bus)
{
    urj_tap_reset_bypass (bus->chain);
}

static void
nexus_access_set_addr (urj_bus_t *bus, uint32_t addr, int mode)
{
    urj_tap_register_t *r = bus->part->active_instruction->data_register->in;
    int i;

    DBG (DBG_BASIC, _("%s: addr=%08lx, mode=%s\n"), __FUNCTION__,
         (long unsigned) addr,
         (mode == ACCESS_MODE_READ) ? "READ" : "WRITE");

    urj_tap_register_fill (r, 0);

    /* set address bits */
    addr >>= 2;
    for (i = 0; i < 7; i++)
        register_set_bit (r, 27 + i, addr & (1 << i));

    /* set access mode */
    register_set_bit (r, 26, mode);

    shift_data (bus, 32);
}

static void
nexus_access_read_data (urj_bus_t *bus, uint32_t *pdata)
{
    urj_tap_register_t *r = bus->part->active_instruction->data_register->out;
    uint32_t data;
    int i;

    shift_data (bus, 32);

    data = 0;
    for (i = 0; i < 32; i++)
        data |= register_get_bit (r, i) << i;

    DBG (DBG_BASIC, _("%s: data=%08lx\n"), __FUNCTION__,
         (long unsigned) data);

    *pdata = data;
}

static void
nexus_access_write_data (urj_bus_t *bus, uint32_t data)
{
    urj_tap_register_t *r = bus->part->active_instruction->data_register->in;
    int i;

    DBG (DBG_BASIC, _("%s: data=%08lx\n"), __FUNCTION__,
         (long unsigned) data);

    register_set_bit (r, 0, 0);
    register_set_bit (r, 1, 0);

    for (i = 0; i < 32; i++)
        register_set_bit (r, 2 + i, data & (1 << i));

    shift_data (bus, 0);
}

static inline void
nexus_reg_read (urj_bus_t *bus, uint32_t reg, uint32_t *data)
{
    nexus_access_set_addr (bus, reg, ACCESS_MODE_READ);
    nexus_access_read_data (bus, data);
}

static inline void
nexus_reg_write (urj_bus_t *bus, uint32_t reg, uint32_t data)
{
    nexus_access_set_addr (bus, reg, ACCESS_MODE_WRITE);
    nexus_access_write_data (bus, data);
}

/* ------------------------------------------------------------------------- */

static void
nexus_memacc_set_addr (urj_bus_t *bus, uint32_t addr, uint32_t rwcs)
{
    nexus_reg_write (bus, OCD_REG_RWA, addr);
    nexus_reg_write (bus, OCD_REG_RWCS, rwcs);
}

static int
nexus_memacc_read (urj_bus_t *bus, uint32_t *data)
{
    uint32_t status;
    int ret;

    do
    {
        nexus_reg_read (bus, OCD_REG_RWCS, &status);
        status &= (OCD_RWCS_ERR | OCD_RWCS_DV);
        /* TODO: add timeout checking */
    }
    while (status == 0);

    DBG (DBG_BASIC, _("%s: read status %08lx\n"), __FUNCTION__,
         (long unsigned) status);

    ret = ACCESS_STATUS_OK;
    switch (status)
    {
    case 1:
        nexus_reg_read (bus, OCD_REG_RWD, data);
        break;
    default:
        urj_error_set (URJ_ERROR_BUS, "read failed, status=%lu",
                       (long unsigned) status);
        *data = 0xffffffff;
        ret = ACCESS_STATUS_ERR;
        break;
    }

    return ret;
}

static int
nexus_memacc_write (urj_bus_t *bus, uint32_t addr, uint32_t data,
                    uint32_t rwcs)
{
    uint32_t status;
    int ret;

    nexus_reg_write (bus, OCD_REG_RWA, addr);
    nexus_reg_write (bus, OCD_REG_RWCS, rwcs);
    nexus_reg_write (bus, OCD_REG_RWD, data);

    nexus_reg_read (bus, OCD_REG_RWCS, &status);
    status &= (OCD_RWCS_ERR | OCD_RWCS_DV);

    DBG (DBG_BASIC, _("%s: status=%08lx\n"), __FUNCTION__,
         (long unsigned) status);

    ret = ACCESS_STATUS_OK;
    if (status)
    {
        urj_error_set (URJ_ERROR_BUS, "write failed, status=%lu",
                       (long unsigned) status);
        ret = ACCESS_STATUS_ERR;
    }

    return ret;
}

/* ------------------------------------------------------------------------- */

static void
avr32_bus_setup (urj_bus_t *bus, urj_chain_t *chain, urj_part_t *part,
                 unsigned int mode)
{
    bus->chain = chain;
    bus->part = part;
    MODE = mode;

    switch (mode)
    {
    case BUS_MODE_OCD:
        SLAVE = SAB_SLAVE_OCD;
        ADDR_MASK = SAB_OCD_ADDR_MASK & ~(3);
        break;

    case BUS_MODE_HSBC:
        SLAVE = SAB_SLAVE_HSB_CACHED;
        ADDR_MASK = SAB_HSB_ADDR_MASK & ~(3);
        break;

    case BUS_MODE_HSBU:
        SLAVE = SAB_SLAVE_HSB_UNCACHED;
        ADDR_MASK = SAB_HSB_ADDR_MASK & ~(3);
        break;

    case BUS_MODE_x8:
        ADDR_MASK = SAB_HSB_ADDR_MASK;
        RWCS_RD = OCD_RWCS_READ8;
        RWCS_WR = OCD_RWCS_WRITE8;
        break;

    case BUS_MODE_x16:
        ADDR_MASK = SAB_HSB_ADDR_MASK & ~(1);
        RWCS_RD = OCD_RWCS_READ16;
        RWCS_WR = OCD_RWCS_WRITE16;
        break;

    case BUS_MODE_x32:
        ADDR_MASK = SAB_HSB_ADDR_MASK & ~(3);
        RWCS_RD = OCD_RWCS_READ32;
        RWCS_WR = OCD_RWCS_WRITE32;
        break;
    }
}

static int
check_instruction (urj_part_t *part, const char *instr)
{
    int ret;

    ret = (urj_part_find_instruction (part, instr) == NULL);
    if (ret)
        urj_error_set (URJ_ERROR_NOTFOUND, "instruction %s not found", instr);

    return ret;
}

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
avr32_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
               const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    unsigned int mode = BUS_MODE_ERROR;

    if (cmd_params[0] == NULL)
    {
        urj_error_set (URJ_ERROR_SYNTAX, "no bus mode specified");
        return NULL;
    }

    if (cmd_params[1] != NULL)
    {
        urj_error_set (URJ_ERROR_SYNTAX, "invalid bus parameter: %s",
                       urj_param_string(&urj_bus_param_list, cmd_params[1]));
        return NULL;
    }

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    switch (cmd_params[0]->key)
    {
    case URJ_BUS_PARAM_KEY_OCD:
        mode = BUS_MODE_OCD;
        break;
    case URJ_BUS_PARAM_KEY_HSBC:
        mode = BUS_MODE_HSBC;
        break;
    case URJ_BUS_PARAM_KEY_HSBU:
        mode = BUS_MODE_HSBU;
        break;
    case URJ_BUS_PARAM_KEY_X8:          // see also: width=8
        mode = BUS_MODE_x8;
        break;
    case URJ_BUS_PARAM_KEY_X16:         // see also: width=16
        mode = BUS_MODE_x16;
        break;
    case URJ_BUS_PARAM_KEY_X32:         // see also: width=32
        mode = BUS_MODE_x32;
        break;

    // RFHH introduced 'width=8|16|32' as an alias for x8, x16, x32
    case URJ_BUS_PARAM_KEY_WIDTH:
        switch (cmd_params[0]->value.lu)
        {
        case 8:
            mode = BUS_MODE_x8;
            break;
        case 16:
            mode = BUS_MODE_x16;
            break;
        case 32:
            mode = BUS_MODE_x32;
            break;
        default:
            urj_bus_generic_free (bus);
            urj_error_set (URJ_ERROR_SYNTAX, "invalid bus width: %lu",
                           cmd_params[0]->value.lu);
            return NULL;
        }
        break;

    default:
        urj_bus_generic_free (bus);
        urj_error_set (URJ_ERROR_SYNTAX, "invalid bus mode: %s",
                       urj_param_string(&urj_bus_param_list, cmd_params[0]));
        return NULL;
    }

    switch (mode)
    {
    case BUS_MODE_OCD:
    case BUS_MODE_HSBC:
    case BUS_MODE_HSBU:
        if (check_instruction (part, "MEMORY_WORD_ACCESS"))
        {
            urj_bus_generic_free (bus);
            return NULL;
        }
        break;
    case BUS_MODE_x8:
    case BUS_MODE_x16:
    case BUS_MODE_x32:
        if (check_instruction (part, "NEXUS_ACCESS"))
        {
            urj_bus_generic_free (bus);
            return NULL;
        }
        break;
    default:
        urj_error_set (URJ_ERROR_INVALID, "need bus mode parameter");
        urj_bus_generic_free (bus);
        return NULL;
    }

    avr32_bus_setup (bus, chain, part, mode);

    return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
avr32_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;

    urj_log (ll, _("AVR32 multi-mode bus driver (JTAG part No. %d)\n"), i);
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
avr32_bus_prepare (urj_bus_t *bus)
{
    if (!bus->initialized)
        URJ_BUS_INIT (bus);
}

/**
 * bus->driver->(*area)
 *
 */
static int
avr32_bus_area (urj_bus_t *bus, uint32_t addr, urj_bus_area_t *area)
{
    switch (MODE)
    {
    case BUS_MODE_HSBC:
        area->description = "HSB memory space, cached";
        area->start = UINT32_C (0x00000000);
        area->length = SAB_HSB_AREA_SIZE;
        area->width = 32;
        break;
    case BUS_MODE_HSBU:
        area->description = "HSB memory space, uncached";
        area->start = UINT32_C (0x00000000);
        area->length = SAB_HSB_AREA_SIZE;
        area->width = 32;
        break;
    case BUS_MODE_x8:
        area->description = "HSB memory space, uncached";
        area->start = UINT32_C (0x00000000);
        area->length = SAB_HSB_AREA_SIZE;
        area->width = 8;
        break;
    case BUS_MODE_x16:
        area->description = "HSB memory space, uncached";
        area->start = UINT32_C (0x00000000);
        area->length = SAB_HSB_AREA_SIZE;
        area->width = 16;
        break;
    case BUS_MODE_x32:
        area->description = "HSB memory space, uncached";
        area->start = UINT32_C (0x00000000);
        area->length = SAB_HSB_AREA_SIZE;
        area->width = 32;
        break;
    case BUS_MODE_OCD:
        if (addr < SAB_OCD_AREA_SIZE)
        {
            area->description = "OCD registers";
            area->start = UINT32_C (0x00000000);
            area->length = SAB_OCD_AREA_SIZE;
            area->width = 32;
            break;
        }
        /* fallthrough */
    default:
        area->description = NULL;
        area->length = UINT64_C (0x100000000);
        area->width = 0;
        break;
    }

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
avr32_bus_read_start (urj_bus_t *bus, uint32_t addr)
{
    addr &= ADDR_MASK;

    DBG (DBG_BASIC, _("%s:addr=%08lx\n"), __FUNCTION__, (long unsigned) addr);

    switch (MODE)
    {
    case BUS_MODE_OCD:
    case BUS_MODE_HSBC:
    case BUS_MODE_HSBU:
        urj_part_set_instruction (bus->part, "MEMORY_WORD_ACCESS");
        mwa_scan_in_instr (bus);
        mwa_scan_in_addr (bus, SLAVE, addr, ACCESS_MODE_READ);
        break;

    case BUS_MODE_x8:
    case BUS_MODE_x16:
    case BUS_MODE_x32:
        urj_part_set_instruction (bus->part, "NEXUS_ACCESS");
        nexus_access_start (bus);
        nexus_memacc_set_addr (bus, addr, RWCS_RD);
        break;
    }

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
avr32_bus_read_end (urj_bus_t *bus)
{
    uint32_t data = 0;

    switch (MODE)
    {
    case BUS_MODE_OCD:
    case BUS_MODE_HSBC:
    case BUS_MODE_HSBU:
        mwa_scan_out_data (bus, &data);
        break;
    case BUS_MODE_x8:
    case BUS_MODE_x16:
    case BUS_MODE_x32:
        nexus_memacc_read (bus, &data);
        nexus_access_end (bus);
        break;
    }

    return data;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
avr32_bus_read_next (urj_bus_t *bus, uint32_t addr)
{
    uint32_t data = 0;

    addr &= ADDR_MASK;

    switch (MODE)
    {
    case BUS_MODE_OCD:
    case BUS_MODE_HSBC:
    case BUS_MODE_HSBU:
        data = avr32_bus_read_end (bus);
        avr32_bus_read_start (bus, addr);
        break;
    case BUS_MODE_x8:
    case BUS_MODE_x16:
    case BUS_MODE_x32:
        nexus_memacc_read (bus, &data);
        nexus_memacc_set_addr (bus, addr, RWCS_RD);
        break;
    }

    return data;
}

/**
 * bus->driver->(*write)
 *
 */
static void
avr32_bus_write (urj_bus_t *bus, uint32_t addr, uint32_t data)
{
    addr &= ADDR_MASK;

    switch (MODE)
    {
    case BUS_MODE_OCD:
    case BUS_MODE_HSBC:
    case BUS_MODE_HSBU:
        urj_part_set_instruction (bus->part, "MEMORY_WORD_ACCESS");
        mwa_write_word (bus, SLAVE, addr, data);
        break;
    case BUS_MODE_x8:
    case BUS_MODE_x16:
    case BUS_MODE_x32:
        urj_part_set_instruction (bus->part, "NEXUS_ACCESS");
        nexus_access_start (bus);
        nexus_memacc_write (bus, addr, data, RWCS_WR);
        nexus_access_end (bus);
        break;
    }
}

const urj_bus_driver_t urj_bus_avr32_bus = {
    "avr32",
    N_("Atmel AVR32 multi-mode bus driver, requires <mode> parameter\n"
       "           valid <mode> parameters:\n"
       "               x8:   8 bit bus for the uncached HSB area, via OCD registers\n"
       "               x16:  16 bit bus for the uncached HSB area, via OCD registers\n"
       "               x32:  32 bit bus for the uncached HSB area, via OCD registers\n"
       "               OCD : 32 bit bus for the OCD registers\n"
       "               HSBC: 32 bit bus for the cached HSB area, via SAB\n"
       "               HSBU: 32 bit bus for the uncached HSB area, via SAB"),
    avr32_bus_new,
    urj_bus_generic_free,
    avr32_bus_printinfo,
    avr32_bus_prepare,
    avr32_bus_area,
    avr32_bus_read_start,
    avr32_bus_read_next,
    avr32_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    avr32_bus_write,
    urj_bus_generic_no_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
