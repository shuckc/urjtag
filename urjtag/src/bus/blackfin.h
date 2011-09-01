/*
 * $Id$
 *
 * Analog Devices unified Blackfin bus functions
 *
 * Copyright (C) 2008-2011 Analog Devices, Inc.
 * Licensed under the GPL-2 or later.
 */

#ifndef __BLACKFIN_BUS_H__
#define __BLACKFIN_BUS_H__

#include <sysdep.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <urjtag/part.h>
#include <urjtag/bus.h>
#include <urjtag/chain.h>
#include <urjtag/bssignal.h>

#include "buses.h"
#include "generic_bus.h"

typedef struct {
    const char *bus_name;
    const char *param;
} bfin_bus_default_t;

typedef struct {
    uint32_t async_base, async_size;

    int ams_cnt, data_cnt, addr_cnt, abe_cnt;
    urj_part_signal_t *ams[4], *data[32], *addr[32], *abe[4];
    urj_part_signal_t *aoe, *are, *awe;

    int sdram, sms_cnt;
    urj_part_signal_t *scas, *sras, *swe, *sms[4];

    urj_part_signal_t *hwait;
    int hwait_level;

    void (*select_flash) (urj_bus_t *bus, uint32_t adr);
    void (*unselect_flash) (urj_bus_t *bus);
} bfin_bus_params_t;

int bfin_bus_new (urj_bus_t *bus, const urj_param_t *cmd_params[],
                  const bfin_bus_default_t *defaults);

int bfin_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area);

void bfin_select_flash (urj_bus_t *bus, uint32_t adr);

void bfin_unselect_flash (urj_bus_t *bus);

void bfin_setup_address (urj_bus_t *bus, uint32_t adr);

void bfin_set_data_in (urj_bus_t *bus);

void bfin_setup_data (urj_bus_t *bus, uint32_t data);

int bfin_bus_read_start (urj_bus_t *bus, uint32_t adr);

uint32_t bfin_bus_read_end (urj_bus_t *bus);

uint32_t bfin_bus_read_next (urj_bus_t *bus, uint32_t adr);

void bfin_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data);

void bfin_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus);

#define _BFIN_BUS_DECLARE(board, funcs, desc) \
const urj_bus_driver_t urj_bus_##board##_bus = \
{ \
    #board, \
    N_("Blackfin " desc " bus driver via BSR\n" \
       "           hwait=[/]SIGNAL    Use specified SIGNAL as HWAIT"), \
    funcs##_bus_new, \
    urj_bus_generic_free, \
    bfin_bus_printinfo, \
    urj_bus_generic_prepare_extest, \
    bfin_bus_area, \
    bfin_bus_read_start, \
    bfin_bus_read_next, \
    bfin_bus_read_end, \
    urj_bus_generic_read, \
    urj_bus_generic_write_start, \
    /*funcs##_bus_write,*/ bfin_bus_write, \
    urj_bus_generic_no_init, \
    urj_bus_generic_no_enable, \
    urj_bus_generic_no_disable, \
    URJ_BUS_TYPE_PARALLEL, \
}
#define BFIN_BUS_DECLARE(board, desc) _BFIN_BUS_DECLARE(board, board, desc)

#endif
