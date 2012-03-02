/*
 * $Id$
 *
 * Busdriver for flashaccess on the Goepel "Boundary Scan Coach" training board
 * www.goepel.com
 * The flash has 1Mbit but only 15 address lines are connected.
 * So only 4Kbit can be accessed.
 * Erfurt, Oct. 10th M. Schneider www.masla.de
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


typedef struct
{
    uint32_t last_adr;
    urj_part_signal_t *adr[15];
    urj_part_signal_t *d[8];
    urj_part_signal_t *deca;
    urj_part_signal_t *decb;
    urj_part_signal_t *decc;
    urj_part_signal_t *we_f;
    urj_part_signal_t *oe_f;
} bus_params_t;

#define LAST_ADR        ((bus_params_t *) bus->params)->last_adr
#define ADR             ((bus_params_t *) bus->params)->adr
#define D               ((bus_params_t *) bus->params)->d
#define DECA            ((bus_params_t *) bus->params)->deca
#define DECB            ((bus_params_t *) bus->params)->decb
#define DECC            ((bus_params_t *) bus->params)->decc
#define WE_F            ((bus_params_t *) bus->params)->we_f
#define OE_F            ((bus_params_t *) bus->params)->oe_f


/**
 * bus->driver->(*new_bus)
 *
 */
static urj_bus_t *
flashbscoach_bus_new (urj_chain_t *chain, const urj_bus_driver_t *driver,
                      const urj_param_t *cmd_params[])
{
    urj_bus_t *bus;
    urj_part_t *part;
    int failed = 0;

    bus = urj_bus_generic_new (chain, driver, sizeof (bus_params_t));
    if (bus == NULL)
        return NULL;
    part = bus->part;

    //OE & WE
    failed |= urj_bus_generic_attach_sig (part, &(OE_F), "PB02_00");
    failed |= urj_bus_generic_attach_sig (part, &(WE_F), "PB02_08");
    //Decoder
    failed |= urj_bus_generic_attach_sig (part, &(DECA), "PB02_04");
    failed |= urj_bus_generic_attach_sig (part, &(DECB), "PB00_12");
    failed |= urj_bus_generic_attach_sig (part, &(DECC), "PB02_07");
    //Adressbus
    failed |= urj_bus_generic_attach_sig (part, &(ADR[0]), "PB01_09");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[1]), "PB01_06");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[2]), "PB01_10");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[3]), "PB01_11");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[4]), "PB01_12");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[5]), "PB01_13");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[6]), "PB01_15");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[7]), "PB01_14");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[8]), "PB01_16");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[9]), "PB00_01");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[10]), "PB00_04");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[11]), "PB00_05");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[12]), "PB00_00");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[13]), "PB00_07");
    failed |= urj_bus_generic_attach_sig (part, &(ADR[14]), "PB00_02");
    //Datenbus
    failed |= urj_bus_generic_attach_sig (part, &(D[0]), "PB00_10");
    failed |= urj_bus_generic_attach_sig (part, &(D[1]), "PB00_06");
    failed |= urj_bus_generic_attach_sig (part, &(D[2]), "PB00_13");
    failed |= urj_bus_generic_attach_sig (part, &(D[3]), "PB00_09");
    failed |= urj_bus_generic_attach_sig (part, &(D[4]), "PB00_14");
    failed |= urj_bus_generic_attach_sig (part, &(D[5]), "PB00_16");
    failed |= urj_bus_generic_attach_sig (part, &(D[6]), "PB02_01");
    failed |= urj_bus_generic_attach_sig (part, &(D[7]), "PB00_11");

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
flashbscoach_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;
    urj_log (ll,
             _("Goepel electronic Boundary Scan Coach compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}







/**
 * bus->driver->(*init)
 *
 */
static int
flashbscoach_bus_init (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    int i = 0;

    if (urj_tap_state (chain) != URJ_TAP_STATE_RUN_TEST_IDLE)
    {
        /* silently skip initialization if TAP isn't in RUNTEST/IDLE state
           this is required to avoid interfering with detect when initbus
           is contained in the part description file
           URJ_BUS_INIT() will be called latest by URJ_BUS_PREPARE() */
        return URJ_STATUS_OK;
    }


    urj_part_set_instruction (p, "SAMPLE/PRELOAD");
    urj_tap_chain_shift_instructions (chain);

    urj_part_set_signal_high (p, DECA);
    urj_part_set_signal_high (p, DECB);
    urj_part_set_signal_high (p, DECC);
    urj_part_set_signal_high (p, OE_F);        //OE_F low aktiv
    urj_part_set_signal_high (p, WE_F);        //WE_F low aktiv

    for (i = 0; i < 15; i++)
        urj_part_set_signal_high (p, ADR[i]);

    urj_part_set_signal_low (p, D[0]);
    urj_part_set_signal_low (p, D[1]);
    urj_part_set_signal_low (p, D[2]);
    urj_part_set_signal_low (p, D[3]);
    urj_part_set_signal_low (p, D[4]);
    urj_part_set_signal_low (p, D[5]);
    urj_part_set_signal_low (p, D[6]);
    urj_part_set_signal_low (p, D[7]);

    urj_tap_chain_shift_data_registers (chain, 0);

    bus->initialized = 1;

    return URJ_STATUS_OK;
}


/**
 * bus->driver->(*area)
 *
 */
static int
flashbscoach_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x00100000000);
    area->width = 8;
//      area->width = urj_part_get_signal( bus->part, urj_part_find_signal( bus->part, "ROMSIZ" ) ) ? 16 : 32;


    return 0;


}
static void
setup_data (urj_bus_t *bus, uint32_t d)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    flashbscoach_bus_area (bus, 0, &area);



    for (i = 0; i < area.width; i++)
        urj_part_set_signal (p, D[i], 1, (d >> i) & 1);
}

static void
set_data_in (urj_bus_t *bus)
{

    urj_part_t *p = bus->part;
    urj_bus_area_t area;

    flashbscoach_bus_area (bus, 0, &area);

    urj_part_set_signal_input (p, D[0]);
    urj_part_set_signal_input (p, D[1]);
    urj_part_set_signal_input (p, D[2]);
    urj_part_set_signal_input (p, D[3]);
    urj_part_set_signal_input (p, D[4]);
    urj_part_set_signal_input (p, D[5]);
    urj_part_set_signal_input (p, D[6]);
    urj_part_set_signal_input (p, D[7]);
}
static void
setup_address (urj_bus_t *bus, uint32_t a)
{
    int i;
    urj_part_t *p = bus->part;

    for (i = 0; i < 15; i++)
        urj_part_set_signal (p, ADR[i], 1, (a >> i) & 1);
}

static uint32_t
get_data_out (urj_bus_t *bus)
{
    int i;
    urj_part_t *p = bus->part;
    urj_bus_area_t area;
    uint32_t d = 0;

    flashbscoach_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (urj_part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static int
flashbscoach_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;

    LAST_ADR = adr;

    urj_part_set_signal_low (p, DECA);
    urj_part_set_signal_high (p, DECB);
    urj_part_set_signal_high (p, DECC);
    urj_part_set_signal_low (p, OE_F);        //OE_F low aktiv
    urj_part_set_signal_high (p, WE_F);        //WE_F low aktiv

    setup_address (bus, adr);
    set_data_in (bus);

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
flashbscoach_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
//      urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;


    setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

//      d = get_data_out( bus );
//      LAST_ADR = adr;
    return get_data_out (bus);

}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
flashbscoach_bus_read_end (urj_bus_t *bus)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;



    urj_part_set_signal_high (p, DECA);
    urj_part_set_signal_high (p, DECB);
    urj_part_set_signal_high (p, DECC);
    urj_part_set_signal_high (p, OE_F);        //OE_F low aktiv
    urj_part_set_signal_high (p, WE_F);        //WE_F low aktiv

    urj_tap_chain_shift_data_registers (chain, 1);

    return get_data_out (bus);

}

/**
 * bus->driver->(*write)
 *
 */
static void
flashbscoach_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    urj_part_t *p = bus->part;
    urj_chain_t *chain = bus->chain;
    urj_part_set_signal_low (p, DECA);
    urj_part_set_signal_high (p, DECB);
    urj_part_set_signal_high (p, DECC);
    urj_part_set_signal_high (p, OE_F);        //OE_F low aktiv
    urj_part_set_signal_high (p, WE_F);        //WE_F low aktiv

    setup_address (bus, adr);
    setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (p, WE_F);
    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_high (p, DECA);
    urj_part_set_signal_high (p, DECB);
    urj_part_set_signal_high (p, DECC);
    urj_part_set_signal_high (p, OE_F);        //OE_F low aktiv
    urj_part_set_signal_high (p, WE_F);        //WE_F low aktiv

    urj_tap_chain_shift_data_registers (chain, 0);
}

const urj_bus_driver_t urj_bus_bscoach_bus = {
    "flashbscoach",
    N_("Goepel Boundary Scan Coach compatible bus driver for flash programming via BSR"),
    flashbscoach_bus_new,
    urj_bus_generic_free,
    flashbscoach_bus_printinfo,
    urj_bus_generic_prepare_extest,
    flashbscoach_bus_area,
    flashbscoach_bus_read_start,
    flashbscoach_bus_read_next,
    flashbscoach_bus_read_end,
    urj_bus_generic_read,
    urj_bus_generic_write_start,
    flashbscoach_bus_write,
    flashbscoach_bus_init,
    urj_bus_generic_no_enable,
    urj_bus_generic_no_disable,
    URJ_BUS_TYPE_PARALLEL,
};
