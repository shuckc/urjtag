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

#ifndef URJ_CABLE_CABLE_H
#define URJ_CABLE_CABLE_H

#define _URJ_LIST(item) extern const urj_parport_driver_t urj_tap_parport_##item##_driver;
#include "parport_list.h"

typedef struct port_node_t port_node_t;

struct port_node_t
{
    urj_parport_t *port;
    port_node_t *next;
};

#endif /* URJ_CABLE_CABLE_H */
