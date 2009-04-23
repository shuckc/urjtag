/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#ifndef URJ_BSBIT_BSBIT_H
#define URJ_BSBIT_BSBIT_H

typedef struct urj_bsbit urj_bsbit_t;

#include "bssignal.h"

#define URJ_BSBIT_INPUT         1
#define URJ_BSBIT_OUTPUT        2
#define URJ_BSBIT_CONTROL       3
#define URJ_BSBIT_INTERNAL      4
#define URJ_BSBIT_BIDIR         5

#define URJ_BSBIT_STATE_Z       (-1)

struct urj_bsbit
{
    int bit;
    char *name;
    int type;
    urj_part_signal_t *signal;
    int safe;                   /* safe value */
    int control;                /* -1 for none */
    int control_value;
    int control_state;
};

urj_bsbit_t *urj_part_bsbit_alloc (int bit, const char *name, int type,
                                   urj_part_signal_t *signal, int safe);
void urj_part_bsbit_free (urj_bsbit_t *b);

#endif /* URJ_BSBIT_BSBIT_H */
