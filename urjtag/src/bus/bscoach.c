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

#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"
#include "state.h"


typedef struct
{
    uint32_t last_adr;
    signal_t *adr[15];
    signal_t *d[8];
    signal_t *deca;
    signal_t *decb;
    signal_t *decc;
    signal_t *we_f;
    signal_t *oe_f;
} bus_params_t;

#define	LAST_ADR	((bus_params_t *) bus->params)->last_adr
#define ADR ((bus_params_t *) bus->params)->adr
#define D ((bus_params_t *) bus->params)->d
#define DECA ((bus_params_t *) bus->params)->deca
#define DECB ((bus_params_t *) bus->params)->decb
#define DECC ((bus_params_t *) bus->params)->decc
#define WE_F ((bus_params_t *) bus->params)->we_f
#define OE_F ((bus_params_t *) bus->params)->oe_f


/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
flashbscoach_bus_new (chain_t * chain, const bus_driver_t * driver,
                      char *cmd_params[])
{
    bus_t *bus;
    part_t *part;
    int failed = 0;

    bus = calloc (1, sizeof (bus_t));
    if (!bus)
        return NULL;

    bus->driver = driver;
    bus->params = calloc (1, sizeof (bus_params_t));
    if (!bus->params)
    {
        free (bus);
        return NULL;
    }

    CHAIN = chain;
    PART = part = chain->parts->parts[chain->active_part];
    //OE & WE
    failed |= generic_bus_attach_sig (part, &(OE_F), "PB02_00");
    failed |= generic_bus_attach_sig (part, &(WE_F), "PB02_08");
    //Decoder
    failed |= generic_bus_attach_sig (part, &(DECA), "PB02_04");
    failed |= generic_bus_attach_sig (part, &(DECB), "PB00_12");
    failed |= generic_bus_attach_sig (part, &(DECC), "PB02_07");
    //Adressbus
    failed |= generic_bus_attach_sig (part, &(ADR[0]), "PB01_09");
    failed |= generic_bus_attach_sig (part, &(ADR[1]), "PB01_06");
    failed |= generic_bus_attach_sig (part, &(ADR[2]), "PB01_10");
    failed |= generic_bus_attach_sig (part, &(ADR[3]), "PB01_11");
    failed |= generic_bus_attach_sig (part, &(ADR[4]), "PB01_12");
    failed |= generic_bus_attach_sig (part, &(ADR[5]), "PB01_13");
    failed |= generic_bus_attach_sig (part, &(ADR[6]), "PB01_15");
    failed |= generic_bus_attach_sig (part, &(ADR[7]), "PB01_14");
    failed |= generic_bus_attach_sig (part, &(ADR[8]), "PB01_16");
    failed |= generic_bus_attach_sig (part, &(ADR[9]), "PB00_01");
    failed |= generic_bus_attach_sig (part, &(ADR[10]), "PB00_04");
    failed |= generic_bus_attach_sig (part, &(ADR[11]), "PB00_05");
    failed |= generic_bus_attach_sig (part, &(ADR[12]), "PB00_00");
    failed |= generic_bus_attach_sig (part, &(ADR[13]), "PB00_07");
    failed |= generic_bus_attach_sig (part, &(ADR[14]), "PB00_02");
    //Datenbus
    failed |= generic_bus_attach_sig (part, &(D[0]), "PB00_10");
    failed |= generic_bus_attach_sig (part, &(D[1]), "PB00_06");
    failed |= generic_bus_attach_sig (part, &(D[2]), "PB00_13");
    failed |= generic_bus_attach_sig (part, &(D[3]), "PB00_09");
    failed |= generic_bus_attach_sig (part, &(D[4]), "PB00_14");
    failed |= generic_bus_attach_sig (part, &(D[5]), "PB00_16");
    failed |= generic_bus_attach_sig (part, &(D[6]), "PB02_01");
    failed |= generic_bus_attach_sig (part, &(D[7]), "PB00_11");




    if (failed)
    {
        free (bus->params);
        free (bus);
        return NULL;
    }


    return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
flashbscoach_bus_printinfo (bus_t * bus)
{
    int i;

    for (i = 0; i < CHAIN->parts->len; i++)
        if (PART == CHAIN->parts->parts[i])
            break;
    printf (_
            ("Goepel electronic Boundary Scan Coach compatible bus driver via BSR (JTAG part No. %d)\n"),
            i);
}







/**
 * bus->driver->(*init)
 *
 */
static int
flashbscoach_bus_init (bus_t * bus)
{
    part_t *p = PART;
    chain_t *chain = CHAIN;
    int i = 0;

    if (tap_state (chain) != Run_Test_Idle)
    {
        /* silently skip initialization if TAP isn't in RUNTEST/IDLE state
           this is required to avoid interfering with detect when initbus
           is contained in the part description file
           bus_init() will be called latest by bus_prepare() */
        return URJTAG_STATUS_OK;
    }


    part_set_instruction (p, "SAMPLE/PRELOAD");
    chain_shift_instructions (chain);

    part_set_signal (p, DECA, 1, 1);
    part_set_signal (p, DECB, 1, 1);
    part_set_signal (p, DECC, 1, 1);
    part_set_signal (p, OE_F, 1, 1);    //OE_F low aktiv
    part_set_signal (p, WE_F, 1, 1);    //WE_F low aktiv

    for (i = 0; i < 15; i++)
        part_set_signal (p, ADR[i], 1, 1);

    part_set_signal (p, D[0], 1, 0);
    part_set_signal (p, D[1], 1, 0);
    part_set_signal (p, D[2], 1, 0);
    part_set_signal (p, D[3], 1, 0);
    part_set_signal (p, D[4], 1, 0);
    part_set_signal (p, D[5], 1, 0);
    part_set_signal (p, D[6], 1, 0);
    part_set_signal (p, D[7], 1, 0);

    chain_shift_data_registers (chain, 0);

    INITIALIZED = 1;

    return URJTAG_STATUS_OK;
}


/**
 * bus->driver->(*area)
 *
 */
static int
flashbscoach_bus_area (bus_t * bus, uint32_t adr, bus_area_t * area)
{
    area->description = NULL;
    area->start = UINT32_C (0x00000000);
    area->length = UINT64_C (0x00100000000);
    area->width = 8;
//      area->width = part_get_signal( PART, part_find_signal( PART, "ROMSIZ" ) ) ? 16 : 32;


    return 0;


}
static void
setup_data (bus_t * bus, uint32_t d)
{
    int i;
    part_t *p = PART;
    bus_area_t area;

    flashbscoach_bus_area (bus, 0, &area);



    for (i = 0; i < area.width; i++)
        part_set_signal (p, D[i], 1, (d >> i) & 1);
}

static void
set_data_in (bus_t * bus)
{

    part_t *p = PART;
    bus_area_t area;

    flashbscoach_bus_area (bus, 0, &area);

    part_set_signal (p, D[0], 0, 0);
    part_set_signal (p, D[1], 0, 0);
    part_set_signal (p, D[2], 0, 0);
    part_set_signal (p, D[3], 0, 0);
    part_set_signal (p, D[4], 0, 0);
    part_set_signal (p, D[5], 0, 0);
    part_set_signal (p, D[6], 0, 0);
    part_set_signal (p, D[7], 0, 0);
}
static void
setup_address (bus_t * bus, uint32_t a)
{
    int i;
    part_t *p = PART;

    for (i = 0; i < 15; i++)
        part_set_signal (p, ADR[i], 1, (a >> i) & 1);
}

static uint32_t
get_data_out (bus_t * bus)
{
    int i;
    part_t *p = PART;
    bus_area_t area;
    uint32_t d = 0;

    flashbscoach_bus_area (bus, 0, &area);

    for (i = 0; i < area.width; i++)
        d |= (uint32_t) (part_get_signal (p, D[i]) << i);

    return d;
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
flashbscoach_bus_read_start (bus_t * bus, uint32_t adr)
{
    part_t *p = PART;
    chain_t *chain = CHAIN;

    LAST_ADR = adr;

    part_set_signal (p, DECA, 1, 0);
    part_set_signal (p, DECB, 1, 1);
    part_set_signal (p, DECC, 1, 1);
    part_set_signal (p, OE_F, 1, 0);    //OE_F low aktiv
    part_set_signal (p, WE_F, 1, 1);    //WE_F low aktiv

    setup_address (bus, adr);
    set_data_in (bus);

    chain_shift_data_registers (chain, 0);

}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
flashbscoach_bus_read_next (bus_t * bus, uint32_t adr)
{
//      part_t *p = PART;
    chain_t *chain = CHAIN;


    setup_address (bus, adr);
    chain_shift_data_registers (chain, 1);

//      d = get_data_out( bus );
//      LAST_ADR = adr;
    return get_data_out (bus);

}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
flashbscoach_bus_read_end (bus_t * bus)
{
    part_t *p = PART;
    chain_t *chain = CHAIN;



    part_set_signal (p, DECA, 1, 1);
    part_set_signal (p, DECB, 1, 1);
    part_set_signal (p, DECC, 1, 1);
    part_set_signal (p, OE_F, 1, 1);    //OE_F low aktiv
    part_set_signal (p, WE_F, 1, 1);    //WE_F low aktiv

    chain_shift_data_registers (chain, 1);

    return get_data_out (bus);

}

/**
 * bus->driver->(*write)
 *
 */
static void
flashbscoach_bus_write (bus_t * bus, uint32_t adr, uint32_t data)
{
    part_t *p = PART;
    chain_t *chain = CHAIN;
    part_set_signal (p, DECA, 1, 0);
    part_set_signal (p, DECB, 1, 1);
    part_set_signal (p, DECC, 1, 1);
    part_set_signal (p, OE_F, 1, 1);    //OE_F low aktiv
    part_set_signal (p, WE_F, 1, 1);    //WE_F low aktiv

    setup_address (bus, adr);
    setup_data (bus, data);

    chain_shift_data_registers (chain, 0);

    part_set_signal (p, WE_F, 1, 0);
    chain_shift_data_registers (chain, 0);

    part_set_signal (p, DECA, 1, 1);
    part_set_signal (p, DECB, 1, 1);
    part_set_signal (p, DECC, 1, 1);
    part_set_signal (p, OE_F, 1, 1);    //OE_F low aktiv
    part_set_signal (p, WE_F, 1, 1);    //WE_F low aktiv

    chain_shift_data_registers (chain, 0);
}

const bus_driver_t bscoach_bus = {
    "flashbscoach",
    N_("Goepel Boundary Scan Coach compatible bus driver for flash programming via BSR"),
    flashbscoach_bus_new,
    generic_bus_free,
    flashbscoach_bus_printinfo,
    generic_bus_prepare_extest,
    flashbscoach_bus_area,
    flashbscoach_bus_read_start,
    flashbscoach_bus_read_next,
    flashbscoach_bus_read_end,
    generic_bus_read,
    flashbscoach_bus_write,
    flashbscoach_bus_init
};
