/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#ifndef URJ_TAP_CABLE_GENERIC_H
#define URJ_TAP_CABLE_GENERIC_H

#include <urjtag/cable.h>
#include <urjtag/parport.h>

#include "../cable.h"

typedef struct
{
    int signals;
} urj_tap_cable_generic_params_t;

#define PARAM_SIGNALS(cable)    ((urj_tap_cable_generic_params_t *) (cable)->params)->signals

void urj_tap_cable_generic_disconnect (urj_cable_t *cable);
void urj_tap_cable_generic_set_frequency (urj_cable_t *cable,
                                          uint32_t new_freq);
/** @return number of clocks on success; -1 on error */
int urj_tap_cable_generic_transfer (urj_cable_t *cable, int len, const char *in,
                                    char *out);
int urj_tap_cable_generic_get_signal (urj_cable_t *cable,
                                      urj_pod_sigsel_t sig);
void urj_tap_cable_generic_flush_one_by_one (urj_cable_t *cable,
                                             urj_cable_flush_amount_t hm);
void urj_tap_cable_generic_flush_using_transfer (urj_cable_t *cable,
                                                 urj_cable_flush_amount_t hm);

#endif /* URJ_TAP_CABLE_GENERIC_H */
