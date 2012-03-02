/*
 * $Id$
 *
 * Bus driver for the Zefant-XS3 Board manufactured by Simple Solutions.
 *
 *   http://www.zefant.de/
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2005.
 *
 * Notes:
 * ------
 *   This driver supports the Flash memory of the FPGA module, the
 *    optional SRAMs in the SO-DIMM socket and the serial EEPROM
 *    found on the mini ATX baseboard.
 *
 *   The external components are assigned different address ranges.
 *   These are arbtitrary but help to distinguish the devices.
 *   Please note that the address ranges reflect a maximum capacity
 *   situation for Flash and EEPROM. The actual chips might provide
 *   a smaller memory array.
 *
 *     FLASH:   0x00000000 - 0x001FFFFF
 *     RAM0:    0x00200000 - 0x0027FFFF
 *     RAM1:    0x00280000 - 0x002FFFFF
 *     EEPROM:  0x00300000 - 0x0030FFFF
 *      status: 0x00310000 - 0x0031FFFF
 *
 *   JTAG Tool generates byte addresses when accessing memories. Thus
 *   this driver discards the LSB when the RAM and flash ranges are
 *   addressed. readmem and writemem care for proper address increment
 *   based on the bus width.
 *   On the other hand, this driver reads and writes always one word
 *   (= 2 bytes) from/to the RAMs. It does not use the byte-enables.
 *   This is mainly due to the lack of byte-enable information in the
 *   bus-driver API.
 *
 *   Remember to clarify the endianness of your data when working with
 *   the memories.
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

#include "buses.h"
#include "generic_bus.h"

/* EEPROM commands */
#define EEPROM_CMD_WREN         0x06
#define EEPROM_CMD_WRDI         0x04
#define EEPROM_CMD_RDSR         0x05
#define EEPROM_CMD_WRSR         0x01
#define EEPROM_CMD_READ         0x03
#define EEPROM_CMD_WRITE        0x02

#define RAM_ADDR_WIDTH          18
#define RAM_DATA_WIDTH          16
#define FLASH_ADDR_WIDTH        25
#define FLASH_DATA_WIDTH        16
#define EEPROM_ADDR_WIDTH       16
#define EEPROM_DATA_WIDTH        8

/* length is in number of bytes
   the full address width is taken to build the power of 2 */
#define RAM_LENGTH              (1 << (RAM_ADDR_WIDTH + 1))
/* the flash component ignores A0, so address is not doubled here */
#define FLASH_LENGTH            (1 << FLASH_ADDR_WIDTH)
#define EEPROM_LENGTH           (1 << EEPROM_ADDR_WIDTH)
#define EEPROM_STATUS_LENGTH    EEPROM_LENGTH

#define FLASH_START             0
#define RAM0_START              FLASH_LENGTH
#define RAM1_START              (RAM0_START + RAM_LENGTH)
#define EEPROM_START            (RAM1_START + RAM_LENGTH)
#define EEPROM_STATUS_START     (EEPROM_START + EEPROM_LENGTH)

typedef enum
{ RAM, FLASH, EEPROM, EEPROM_STATUS } ctype_t;

typedef struct
{
    ctype_t ctype;
    char *cname;
    urj_part_signal_t *a[FLASH_ADDR_WIDTH];
    urj_part_signal_t *d[RAM_DATA_WIDTH];
    urj_part_signal_t *ncs;
    urj_part_signal_t *noe;
    urj_part_signal_t *nwe;
    urj_part_signal_t *nlb;
    urj_part_signal_t *nub;
    urj_part_signal_t *nbyte;
    urj_part_signal_t *sts;
    urj_part_signal_t *nrp;
    urj_part_signal_t *si;
    urj_part_signal_t *so;
    urj_part_signal_t *sck;
} component_t;

typedef struct
{
    uint32_t last_addr;         /* holds last address of read or write access */
    component_t flash;
    component_t ram0;
    component_t ram1;
    component_t eeprom;
    component_t eeprom_status;
} bus_params_t;

#define LAST_ADDR ((bus_params_t *) bus->params)->last_addr
#define A         comp->a
#define D         comp->d
#define nCS       comp->ncs
#define nOE       comp->noe
#define nWE       comp->nwe
#define nLB       comp->nlb
#define nUB       comp->nub
#define nBYTE     comp->nbyte
#define STS       comp->sts
#define nRP       comp->nrp
#define SI        comp->si
#define SO        comp->so
#define SCK       comp->sck

#define COMP_FLASH         &(((bus_params_t *) bus->params)->flash)
#define COMP_RAM0          &(((bus_params_t *) bus->params)->ram0)
#define COMP_RAM1          &(((bus_params_t *) bus->params)->ram1)
#define COMP_EEPROM        &(((bus_params_t *) bus->params)->eeprom)
#define COMP_EEPROM_STATUS &(((bus_params_t *) bus->params)->eeprom_status)

/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
zefant_xs3_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                    const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    int failed = 0;
    component_t *comp;
    int idx;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    /*
     * Setup FLASH
     */
    comp = COMP_FLASH;
    comp->ctype = FLASH;
    comp->cname = "FLASH";

    failed |= urj_bus_generic_attach_sig (part, &(A[0]), "IO_V9");
    failed |= urj_bus_generic_attach_sig (part, &(A[1]), "IO_U10");
    failed |= urj_bus_generic_attach_sig (part, &(A[2]), "IO_V10");
    failed |= urj_bus_generic_attach_sig (part, &(A[3]), "IO_W10");
    failed |= urj_bus_generic_attach_sig (part, &(A[4]), "IO_Y10");
    failed |= urj_bus_generic_attach_sig (part, &(A[5]), "IO_W8");
    failed |= urj_bus_generic_attach_sig (part, &(A[6]), "IO_W9");
    failed |= urj_bus_generic_attach_sig (part, &(A[7]), "IO_V8");
    failed |= urj_bus_generic_attach_sig (part, &(A[8]), "IO_V6");
    failed |= urj_bus_generic_attach_sig (part, &(A[9]), "IO_AA8");
    failed |= urj_bus_generic_attach_sig (part, &(A[10]), "IO_AB8");
    failed |= urj_bus_generic_attach_sig (part, &(A[11]), "IO_U7");
    failed |= urj_bus_generic_attach_sig (part, &(A[12]), "IO_V7");
    failed |= urj_bus_generic_attach_sig (part, &(A[13]), "IO_U6");
    failed |= urj_bus_generic_attach_sig (part, &(A[14]), "IO_Y6");
    failed |= urj_bus_generic_attach_sig (part, &(A[15]), "IO_AB11");
    failed |= urj_bus_generic_attach_sig (part, &(A[16]), "IO_AB10");
    failed |= urj_bus_generic_attach_sig (part, &(A[17]), "IO_AA10");
    failed |= urj_bus_generic_attach_sig (part, &(A[18]), "IO_W6");
    failed |= urj_bus_generic_attach_sig (part, &(A[19]), "IO_AA6");
    failed |= urj_bus_generic_attach_sig (part, &(A[20]), "IO_U11");
    failed |= urj_bus_generic_attach_sig (part, &(A[21]), "IO_Y13");
    failed |= urj_bus_generic_attach_sig (part, &(A[22]), "IO_AB13");
    failed |= urj_bus_generic_attach_sig (part, &(A[23]), "IO_U13");
    failed |= urj_bus_generic_attach_sig (part, &(A[24]), "IO_AA13");

    failed |= urj_bus_generic_attach_sig (part, &(D[0]), "IO_AA14");
    failed |= urj_bus_generic_attach_sig (part, &(D[1]), "IO_AB14");
    failed |= urj_bus_generic_attach_sig (part, &(D[2]), "IO_U12");
    failed |= urj_bus_generic_attach_sig (part, &(D[3]), "IO_V12");
    failed |= urj_bus_generic_attach_sig (part, &(D[4]), "IO_W11");
    failed |= urj_bus_generic_attach_sig (part, &(D[5]), "IO_V11");
    failed |= urj_bus_generic_attach_sig (part, &(D[6]), "IO_AB9");
    failed |= urj_bus_generic_attach_sig (part, &(D[7]), "IO_AA9");
    failed |= urj_bus_generic_attach_sig (part, &(D[8]), "IO_U16");
    failed |= urj_bus_generic_attach_sig (part, &(D[9]), "IO_AB15");
    failed |= urj_bus_generic_attach_sig (part, &(D[10]), "IO_AA15");
    failed |= urj_bus_generic_attach_sig (part, &(D[11]), "IO_W14");
    failed |= urj_bus_generic_attach_sig (part, &(D[12]), "IO_V14");
    failed |= urj_bus_generic_attach_sig (part, &(D[13]), "IO_U14");
    failed |= urj_bus_generic_attach_sig (part, &(D[14]), "IO_W13");
    failed |= urj_bus_generic_attach_sig (part, &(D[15]), "IO_V13");

    failed |= urj_bus_generic_attach_sig (part, &(nWE), "IO_Y17");
    failed |= urj_bus_generic_attach_sig (part, &(nOE), "IO_AA17");
    failed |= urj_bus_generic_attach_sig (part, &(nCS), "IO_U17");
    nLB = NULL;
    nUB = NULL;

    failed |= urj_bus_generic_attach_sig (part, &(nRP), "IO_V16");
    failed |= urj_bus_generic_attach_sig (part, &(nBYTE), "IO_Y16");
    failed |= urj_bus_generic_attach_sig (part, &(STS), "IO_W16");

    SI = NULL;
    SO = NULL;
    SCK = NULL;

    /*
     * Setup SO-DIMM SRAM0
     */
    comp = COMP_RAM0;
    comp->ctype = RAM;
    comp->cname = "RAM0";

    failed |= urj_bus_generic_attach_sig (part, &(A[0]), "IO_AA4");
    failed |= urj_bus_generic_attach_sig (part, &(A[1]), "IO_AB4");
    failed |= urj_bus_generic_attach_sig (part, &(A[2]), "IO_W5");
    failed |= urj_bus_generic_attach_sig (part, &(A[3]), "IO_Y3");
    failed |= urj_bus_generic_attach_sig (part, &(A[4]), "IO_Y1");
    failed |= urj_bus_generic_attach_sig (part, &(A[5]), "IO_M1");
    failed |= urj_bus_generic_attach_sig (part, &(A[6]), "IO_N2");
    failed |= urj_bus_generic_attach_sig (part, &(A[7]), "IO_L2");
    failed |= urj_bus_generic_attach_sig (part, &(A[8]), "IO_L1");
    failed |= urj_bus_generic_attach_sig (part, &(A[9]), "IO_K1");
    failed |= urj_bus_generic_attach_sig (part, &(A[10]), "IO_K3");
    failed |= urj_bus_generic_attach_sig (part, &(A[11]), "IO_L6");
    failed |= urj_bus_generic_attach_sig (part, &(A[12]), "IO_L4");
    failed |= urj_bus_generic_attach_sig (part, &(A[13]), "IO_L3");
    failed |= urj_bus_generic_attach_sig (part, &(A[14]), "IO_K4");
    failed |= urj_bus_generic_attach_sig (part, &(A[15]), "IO_AB5");
    failed |= urj_bus_generic_attach_sig (part, &(A[16]), "IO_AA5");
    failed |= urj_bus_generic_attach_sig (part, &(A[17]), "IO_Y5");
    A[18] = NULL;
    A[19] = NULL;
    A[20] = NULL;
    A[21] = NULL;
    A[22] = NULL;
    A[23] = NULL;
    A[24] = NULL;

    failed |= urj_bus_generic_attach_sig (part, &(D[0]), "IO_W1");
    failed |= urj_bus_generic_attach_sig (part, &(D[1]), "IO_V5");
    failed |= urj_bus_generic_attach_sig (part, &(D[2]), "IO_V3");
    failed |= urj_bus_generic_attach_sig (part, &(D[3]), "IO_V1");
    failed |= urj_bus_generic_attach_sig (part, &(D[4]), "IO_N1");
    failed |= urj_bus_generic_attach_sig (part, &(D[5]), "IO_N3");
    failed |= urj_bus_generic_attach_sig (part, &(D[6]), "IO_M2");
    failed |= urj_bus_generic_attach_sig (part, &(D[7]), "IO_M5");
    failed |= urj_bus_generic_attach_sig (part, &(D[8]), "IO_M4");
    failed |= urj_bus_generic_attach_sig (part, &(D[9]), "IO_M6");
    failed |= urj_bus_generic_attach_sig (part, &(D[10]), "IO_L5");
    failed |= urj_bus_generic_attach_sig (part, &(D[11]), "IO_N4");
    failed |= urj_bus_generic_attach_sig (part, &(D[12]), "IO_T6");
    failed |= urj_bus_generic_attach_sig (part, &(D[13]), "IO_V2");
    failed |= urj_bus_generic_attach_sig (part, &(D[14]), "IO_V4");
    failed |= urj_bus_generic_attach_sig (part, &(D[15]), "IO_U5");

    failed |= urj_bus_generic_attach_sig (part, &(nCS), "IO_W3");
    failed |= urj_bus_generic_attach_sig (part, &(nOE), "IO_Y2");
    failed |= urj_bus_generic_attach_sig (part, &(nWE), "IO_M3");
    failed |= urj_bus_generic_attach_sig (part, &(nLB), "IO_W2");
    failed |= urj_bus_generic_attach_sig (part, &(nUB), "IO_W4");
    nRP = NULL;
    nBYTE = NULL;
    STS = NULL;

    SI = NULL;
    SO = NULL;
    SCK = NULL;

    /*
     * Setup SO-DIMM SRAM1
     */
    comp = COMP_RAM1;
    comp->ctype = RAM;
    comp->cname = "RAM1";

    failed |= urj_bus_generic_attach_sig (part, &(A[0]), "IO_H5");
    failed |= urj_bus_generic_attach_sig (part, &(A[1]), "IO_F5");
    failed |= urj_bus_generic_attach_sig (part, &(A[2]), "IO_F2");
    failed |= urj_bus_generic_attach_sig (part, &(A[3]), "IO_D1");
    failed |= urj_bus_generic_attach_sig (part, &(A[4]), "IO_E1");
    failed |= urj_bus_generic_attach_sig (part, &(A[5]), "IO_F10");
    failed |= urj_bus_generic_attach_sig (part, &(A[6]), "IO_C7");
    failed |= urj_bus_generic_attach_sig (part, &(A[7]), "IO_C10");
    failed |= urj_bus_generic_attach_sig (part, &(A[8]), "IO_A10");
    failed |= urj_bus_generic_attach_sig (part, &(A[9]), "IO_B10");
    failed |= urj_bus_generic_attach_sig (part, &(A[10]), "IO_F11");
    failed |= urj_bus_generic_attach_sig (part, &(A[11]), "IO_A9");
    failed |= urj_bus_generic_attach_sig (part, &(A[12]), "IO_B9");
    failed |= urj_bus_generic_attach_sig (part, &(A[13]), "IO_B8");
    failed |= urj_bus_generic_attach_sig (part, &(A[14]), "IO_F9");
    failed |= urj_bus_generic_attach_sig (part, &(A[15]), "IO_F4");
    failed |= urj_bus_generic_attach_sig (part, &(A[16]), "IO_G6");
    failed |= urj_bus_generic_attach_sig (part, &(A[17]), "IO_G5");
    A[18] = NULL;
    A[19] = NULL;
    A[20] = NULL;
    A[21] = NULL;
    A[22] = NULL;
    A[23] = NULL;
    A[24] = NULL;

    failed |= urj_bus_generic_attach_sig (part, &(D[0]), "IO_C1");
    failed |= urj_bus_generic_attach_sig (part, &(D[1]), "IO_E2");
    failed |= urj_bus_generic_attach_sig (part, &(D[2]), "IO_C2");
    failed |= urj_bus_generic_attach_sig (part, &(D[3]), "IO_C3");
    failed |= urj_bus_generic_attach_sig (part, &(D[4]), "IO_B5");
    failed |= urj_bus_generic_attach_sig (part, &(D[5]), "IO_A5");
    failed |= urj_bus_generic_attach_sig (part, &(D[6]), "IO_B6");
    failed |= urj_bus_generic_attach_sig (part, &(D[7]), "IO_D7");
    failed |= urj_bus_generic_attach_sig (part, &(D[8]), "IO_D9");
    failed |= urj_bus_generic_attach_sig (part, &(D[9]), "IO_E9");
    failed |= urj_bus_generic_attach_sig (part, &(D[10]), "IO_F7");
    failed |= urj_bus_generic_attach_sig (part, &(D[11]), "IO_E7");
    failed |= urj_bus_generic_attach_sig (part, &(D[12]), "IO_D5");
    failed |= urj_bus_generic_attach_sig (part, &(D[13]), "IO_C4");
    failed |= urj_bus_generic_attach_sig (part, &(D[14]), "IO_D3");
    failed |= urj_bus_generic_attach_sig (part, &(D[15]), "IO_D4");

    failed |= urj_bus_generic_attach_sig (part, &(nCS), "IO_D2");
    failed |= urj_bus_generic_attach_sig (part, &(nOE), "IO_F3");
    failed |= urj_bus_generic_attach_sig (part, &(nWE), "IO_E10");
    failed |= urj_bus_generic_attach_sig (part, &(nLB), "IO_E4");
    failed |= urj_bus_generic_attach_sig (part, &(nUB), "IO_E3");
    nRP = NULL;
    nBYTE = NULL;
    STS = NULL;

    SI = NULL;
    SO = NULL;
    SCK = NULL;

    /*
     * Setup EEPROM
     */
    comp = COMP_EEPROM;
    comp->ctype = EEPROM;
    comp->cname = "EEPROM";

    failed |= urj_bus_generic_attach_sig (part, &(SI), "IO_H19");
    failed |= urj_bus_generic_attach_sig (part, &(SO), "IO_J21");
    failed |= urj_bus_generic_attach_sig (part, &(SCK), "IO_H21");
    failed |= urj_bus_generic_attach_sig (part, &(nCS), "IO_K22");

    for (idx = 0; idx < FLASH_ADDR_WIDTH; idx++)
        A[idx] = NULL;
    for (idx = 0; idx < RAM_DATA_WIDTH; idx++)
        D[idx] = NULL;
    nOE = NULL;
    nWE = NULL;
    nLB = NULL;
    nUB = NULL;
    nRP = NULL;
    nBYTE = NULL;
    STS = NULL;

    /*
     * Setup EEPROM Status
     * copy settings from EEPROM
     */
    ((bus_params_t *) bus->params)->eeprom_status =
        ((bus_params_t *) bus->params)->eeprom;
    comp = COMP_EEPROM_STATUS;
    comp->ctype = EEPROM_STATUS;
    comp->cname = "EEPROM Status";


    if (failed)
    {
        urj_bus_generic_free (bus);
        return NULL;
    }

    return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
zefant_xs3_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll, _("Simple Solutions Zefant-XS3 Board compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}

static void
setup_address (urj_bus_t *bus, uint32_t a, component_t *comp)
{
    int i;
    urj_part_t *p = bus->part;
    int addr_width;

    LAST_ADDR = a;

    switch (comp->ctype)
    {
    case FLASH:
        addr_width = FLASH_ADDR_WIDTH;
        /* address a is a byte address,
           A0 is ignored by the flash chip */
        break;
    case RAM:
        addr_width = RAM_ADDR_WIDTH;
        /* address a is a byte address so it is transferred into
           a word address here */
        a >>= 1;
        break;
    case EEPROM:
    case EEPROM_STATUS:
        addr_width = EEPROM_ADDR_WIDTH;
        break;
    default:
        addr_width = 0;
        break;
    }

    for (i = 0; i < addr_width; i++)
        urj_part_set_signal (p, A[i], 1, (a >> i) & 1);
}

static int
detect_data_width (component_t *comp)
{
    int width;

    switch (comp->ctype)
    {
    case RAM:
        width = RAM_DATA_WIDTH;
        break;
    case FLASH:
        width = FLASH_DATA_WIDTH;
        break;
    case EEPROM:
    case EEPROM_STATUS:
        width = EEPROM_DATA_WIDTH;
        break;
    default:
        width = 0;
        break;
    }

    return width;
}

static void
set_data_in (urj_bus_t *bus, component_t *comp)
{
    int i;
    urj_part_t *p = bus->part;
    int width;

    width = detect_data_width (comp);

    for (i = 0; i < width; i++)
        urj_part_set_signal_input (p, D[i]);
}

static void
setup_data (urj_bus_t *bus, uint32_t d, component_t *comp)
{
    int i;
    urj_part_t *p = bus->part;
    int width;

    width = detect_data_width (comp);

    for (i = 0; i < width; i++)
        urj_part_set_signal (p, D[i], 1, (d >> i) & 1);
}

static uint8_t
eeprom_shift_byte (urj_chain_t *chain, urj_part_t *p, component_t *comp,
                   uint8_t byte)
{
    int pos;
    uint8_t so_data = 0x00;

    for (pos = 7; pos >= 0; pos--)
    {
        /* set clock to 0 */
        urj_part_set_signal_low (p, SCK);
        /* apply data bit */
        urj_part_set_signal (p, SI, 1, (byte >> pos) & 0x01);
        /* commit signals */
        urj_tap_chain_shift_data_registers (chain, 1);

        /* set clock to 1 */
        urj_part_set_signal_high (p, SCK);
        /* commit signals */
        urj_tap_chain_shift_data_registers (chain, 1);

        /* read data on SO that was asserted by device after SCK went 0 */
        so_data |= (uint8_t) (urj_part_get_signal (p, SO) << pos);
    }

    return so_data;
}

static void
eeprom_disable_device (urj_chain_t *chain, urj_part_t *p, component_t *comp)
{
    /* ensure that SCK is low before disabling device */
    urj_part_set_signal_low (p, SCK);
    urj_tap_chain_shift_data_registers (chain, 0);

    /* finally disable device */
    urj_part_set_signal_high (p, nCS);
    urj_tap_chain_shift_data_registers (chain, 0);
}

/**
 * bus->driver->(*init)
 *
 */
static int
zefant_xs3_bus_init (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    component_t *comp;

    if (urj_tap_state (chain) != URJ_TAP_STATE_RUN_TEST_IDLE)
    {
        /* silently skip initialization if TAP isn't in RUNTEST/IDLE state
           this is required to avoid interfering with detect when initbus
           is contained in the part description file
           URJ_BUS_INIT() will be called latest by URJ_BUS_PREPARE() */
        return URJ_STATUS_OK;
    }

    /* Preload update registers */

    urj_part_set_instruction (p, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);

    /* FLASH */
    comp = COMP_FLASH;
    setup_data (bus, 0, comp);
    urj_part_set_signal_high (p, nCS);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);
    urj_part_set_signal_high (p, nRP);
    urj_part_set_signal_high (p, nBYTE);
    urj_part_set_signal_input (p, STS);

    /* RAM0 */
    comp = COMP_RAM0;
    setup_data (bus, 0, comp);
    urj_part_set_signal_high (p, nCS);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);
    urj_part_set_signal_high (p, nLB);
    urj_part_set_signal_high (p, nUB);

    /* RAM1 */
    comp = COMP_RAM1;
    setup_data (bus, 0, comp);
    urj_part_set_signal_high (p, nCS);
    urj_part_set_signal_high (p, nWE);
    urj_part_set_signal_high (p, nOE);
    urj_part_set_signal_high (p, nLB);
    urj_part_set_signal_high (p, nUB);

    /* EEPROM */
    comp = COMP_EEPROM;
    urj_part_set_signal_low (p, SI);
    urj_part_set_signal_input (p, SO);
    urj_part_set_signal_low (p, SCK);
    urj_part_set_signal_high (p, nCS);

    /* EEPROM Status */
    comp = COMP_EEPROM_STATUS;
    urj_part_set_signal_low (p, SI);
    urj_part_set_signal_input (p, SO);
    urj_part_set_signal_low (p, SCK);
    urj_part_set_signal_high (p, nCS);

    urj_tap_chain_shift_data_registers (chain, 0);

    bus->initialized = 1;

    return URJ_STATUS_OK;
}

static int
comp_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area,
               component_t **comp)
{
    if (adr < RAM0_START)
    {
        area->description = "FLASH Component";
        area->start = FLASH_START;
        area->length = FLASH_LENGTH;
        area->width = FLASH_DATA_WIDTH;
        *comp = COMP_FLASH;
    }
    else if (adr < RAM1_START)
    {
        area->description = "SO-DIMM RAM0 Component";
        area->start = RAM0_START;
        area->length = RAM_LENGTH;
        area->width = RAM_DATA_WIDTH;
        *comp = COMP_RAM0;
    }
    else if (adr < EEPROM_START)
    {
        area->description = "SO-DIMM RAM1 Component";
        area->start = RAM1_START;
        area->length = RAM_LENGTH;
        area->width = RAM_DATA_WIDTH;
        *comp = COMP_RAM1;
    }
    else if (adr < EEPROM_STATUS_START)
    {
        area->description = "EEPROM Component";
        area->start = EEPROM_START;
        area->length = EEPROM_LENGTH;
        area->width = EEPROM_DATA_WIDTH;
        *comp = COMP_EEPROM;
    }
    else if (adr < EEPROM_STATUS_START + EEPROM_STATUS)
    {
        area->description = "EEPROM Component Status";
        area->start = EEPROM_STATUS_START;
        area->length = EEPROM_LENGTH;
        area->width = EEPROM_DATA_WIDTH;
        *comp = COMP_EEPROM_STATUS;
    }
    else
    {
        area->description = "Dummy";
        area->start = FLASH_LENGTH + 2 * RAM_LENGTH + 2 * EEPROM_LENGTH;
        area->length = UINT64_C (0x100000000);
        area->width = 0;
        *comp = NULL;
    }

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*area)
 *
 */
static int
zefant_xs3_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    component_t *comp;

    return comp_bus_area (bus, adr, area, &comp);
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
zefant_xs3_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    urj_bus_area_t area;
    component_t *comp;
    uint8_t cmd = EEPROM_CMD_READ;

    comp_bus_area (bus, adr, &area, &comp);
    if (!comp)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        LAST_ADDR = adr;
        return URJ_STATUS_FAIL;
    }

    /* determine proper address setup strategy for component */
    switch (comp->ctype)
    {
    case FLASH:
    case RAM:
        urj_part_set_signal_low (p, nCS);
        urj_part_set_signal_high (p, nWE);
        urj_part_set_signal_low (p, nOE);
        if (comp->ctype == RAM)
        {
            urj_part_set_signal_low (p, nLB);
            urj_part_set_signal_low (p, nUB);
        }

        setup_address (bus, adr, comp);
        set_data_in (bus, comp);

        urj_tap_chain_shift_data_registers (chain, 0);

        break;

    case EEPROM_STATUS:
        cmd = EEPROM_CMD_RDSR;
        /* fall through */
    case EEPROM:
        /* enable device */
        urj_part_set_signal_low (p, nCS);

        /* shift command */
        eeprom_shift_byte (chain, p, comp, cmd);

        if (comp->ctype == EEPROM)
        {
            /* send address high part */
            eeprom_shift_byte (chain, p, comp, (adr >> 8) & 0xff);
            /* send address low part */
            eeprom_shift_byte (chain, p, comp, adr & 0xff);
        }

        LAST_ADDR = adr;
        break;

    default:
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("Component type not supported"));
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
zefant_xs3_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;
    component_t *comp;

    comp_bus_area (bus, adr, &area, &comp);
    if (!comp)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        LAST_ADDR = adr;
        return 0;
    }

    /* determine proper read strategy for component */
    switch (comp->ctype)
    {
    case FLASH:
    case RAM:
        setup_address (bus, adr, comp);
        urj_tap_chain_shift_data_registers (chain, 1);

        for (i = 0; i < area.width; i++)
            d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

        break;

    case EEPROM_STATUS:
    case EEPROM:
        /* read next byte */
        d = (uint32_t) eeprom_shift_byte (chain, p, comp, 0x00);
        break;

    default:
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("Component type not supported"));
        break;
    }

    return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
zefant_xs3_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;
    urj_bus_area_t area;
    component_t *comp;

    /* use last address of access to determine component */
    comp_bus_area (bus, LAST_ADDR, &area, &comp);
    if (!comp)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        return 0;
    }

    /* determine proper read strategy for component */
    switch (comp->ctype)
    {
    case FLASH:
    case RAM:
        urj_part_set_signal_high (p, nCS);
        urj_part_set_signal_high (p, nOE);
        if (comp->ctype == RAM)
        {
            urj_part_set_signal_high (p, nLB);
            urj_part_set_signal_high (p, nUB);
        }
        urj_tap_chain_shift_data_registers (chain, 1);

        for (i = 0; i < area.width; i++)
            d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

        break;

    case EEPROM_STATUS:
    case EEPROM:
        /* read final byte */
        d = (uint32_t) eeprom_shift_byte (chain, p, comp, 0x00);
        eeprom_disable_device (chain, p, comp);

        break;

    default:
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("Component type not supported"));
        break;
    }

    return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
zefant_xs3_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    urj_bus_area_t area;
    component_t *comp;
    uint8_t cmd = EEPROM_CMD_WRITE;

    comp_bus_area (bus, adr, &area, &comp);
    if (!comp)
    {
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS, _("Address out of range"));
        return;
    }

    switch (comp->ctype)
    {
    case FLASH:
    case RAM:
        urj_part_set_signal_low (p, nCS);
        urj_part_set_signal_high (p, nWE);
        urj_part_set_signal_high (p, nOE);
        if (comp->ctype == RAM)
        {
            urj_part_set_signal_low (p, nLB);
            urj_part_set_signal_low (p, nUB);
        }

        setup_address (bus, adr, comp);
        setup_data (bus, data, comp);

        urj_tap_chain_shift_data_registers (chain, 0);

        urj_part_set_signal_low (p, nWE);
        urj_tap_chain_shift_data_registers (chain, 0);
        urj_part_set_signal_high (p, nWE);
        urj_part_set_signal_high (p, nCS);
        if (comp->ctype == RAM)
        {
            urj_part_set_signal_high (p, nLB);
            urj_part_set_signal_high (p, nUB);
        }
        urj_tap_chain_shift_data_registers (chain, 0);

        break;

    case EEPROM_STATUS:
        cmd = EEPROM_CMD_WRSR;
        /* fall through */
    case EEPROM:
        /*
         * Step 1:
         * Poll status register and ensure that device is ready.
         */
        urj_part_set_signal_low (p, nCS);

        /* poll status register for nRDY */
        do
        {
            eeprom_shift_byte (chain, p, comp, EEPROM_CMD_RDSR);
        }
        while (eeprom_shift_byte (chain, p, comp, 0x00) & 0x01);

        eeprom_disable_device (chain, p, comp);


        /*
         * Step 2:
         * Enable writing.
         */
        urj_part_set_signal_low (p, nCS);

        /* enable writing */
        eeprom_shift_byte (chain, p, comp, EEPROM_CMD_WREN);

        eeprom_disable_device (chain, p, comp);


        /*
         * Step 3:
         * Write data to device.
         */
        urj_part_set_signal_low (p, nCS);

        /* send command
           command code has been determined by component type */
        eeprom_shift_byte (chain, p, comp, cmd);

        if (comp->ctype == EEPROM)
        {
            /* send address high part */
            eeprom_shift_byte (chain, p, comp, (adr >> 8) & 0xff);
            /* send address low part */
            eeprom_shift_byte (chain, p, comp, adr & 0xff);
        }

        /* send data to be written */
        eeprom_shift_byte (chain, p, comp, (uint8_t) (data & 0xff));

        eeprom_disable_device (chain, p, comp);

        break;

    default:
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       _("Component type not supported"));
        break;
    }
}

const urj_bus_driver_t urj_bus_zefant_xs3_bus = {
    "zefant-xs3",
    N_("Simple Solutions Zefant-XS3 Board compatible bus driver via BSR"),
    zefant_xs3_bus_new,
    urj_bus_generic_free,
    zefant_xs3_bus_printinfo,
    urj_bus_generic_prepare_extest,
    zefant_xs3_bus_area,
    zefant_xs3_bus_read_start,
    zefant_xs3_bus_read_next,
    zefant_xs3_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    zefant_xs3_bus_write,
    zefant_xs3_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};


/*
 Local Variables:
 mode:C
 tab-width:2
 indent-tabs-mode:t
 End:
*/
