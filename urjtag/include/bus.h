/*
 * $Id$
 *
 * Bus driver interface
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

#ifndef URJ_BUS_H
#define	URJ_BUS_H

#include <stdint.h>

#include <bus_driver.h>

typedef struct
{
    int len;
    urj_bus_t **buses;
} urj_buses_t;

extern urj_buses_t buses;
extern const urj_bus_driver_t *bus_drivers[];

void urj_bus_buses_free (void);
void urj_bus_buses_add (urj_bus_t *abus);
void urj_bus_buses_delete (urj_bus_t *abus);

#endif /* URJ_BUS_H */
