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

#ifndef GENERIC_H
#define	GENERIC_H

#include "cable.h"
#include "parport.h"

typedef struct
{
    int signals;
} generic_params_t;

#define	PARAM_SIGNALS(cable)	((generic_params_t *) cable->params)->signals

void generic_disconnect (cable_t * cable);
void generic_set_frequency (cable_t * cable, uint32_t new_freq);
int generic_transfer (cable_t * cable, int len, char *in, char *out);
int generic_get_signal (cable_t * cable, pod_sigsel_t sig);
void generic_flush_one_by_one (cable_t * cable, cable_flush_amount_t hm);
void generic_flush_using_transfer (cable_t * cable, cable_flush_amount_t hm);

#endif /* GENERIC_H */
