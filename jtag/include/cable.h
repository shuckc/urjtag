/*
 * $Id$
 *
 * Cable driver interface
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

#ifndef CABLE_H
#define	CABLE_H

#include <stdint.h>

typedef struct {
	const char *name;
	const char *description;
	int (*init)( unsigned int );
	void (*done)( void );
	void (*clock)( int, int );
	int (*get_tdo)( void );
	void (*set_trst)( int );
} cable_driver_t;

extern uint32_t frequency;
void cable_wait( void );

extern cable_driver_t *cable;
#define	tap_clock	cable->clock
#define	tap_get_tdo	cable->get_tdo
#define	tap_set_trst	cable->set_trst

extern cable_driver_t *cable_drivers[];

#endif /* CABLE_H */