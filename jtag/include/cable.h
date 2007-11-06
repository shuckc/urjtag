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

typedef struct cable_t cable_t;

#include "parport.h"
#include "chain.h"

typedef struct cable_driver_t cable_driver_t;

struct cable_driver_t {
	const char *name;
	const char *description;
	cable_t *(*connect)( cable_driver_t *, parport_t * );
	void (*disconnect)( cable_t *cable );
	void (*cable_free)( cable_t *cable );
	int (*init)( cable_t * );
	void (*done)( cable_t * );
	void (*clock)( cable_t *, int, int );
	int (*get_tdo)( cable_t * );
	int (*set_trst)( cable_t *, int );
	int (*get_trst)( cable_t * );
};

struct cable_t {
	cable_driver_t *driver;
	parport_t *port;
	void *params;
	chain_t *chain;
};

void cable_free( cable_t *cable );
int cable_init( cable_t *cable );
void cable_done( cable_t *cable );
void cable_clock( cable_t *cable, int tms, int tdi );
int cable_get_tdo( cable_t *cable );
int cable_set_trst( cable_t *cable, int trst );
int cable_get_trst( cable_t *cable );

void cable_set_frequency( cable_t *cable, uint32_t frequency );
uint32_t cable_get_frequency( cable_t *cable );
void cable_wait( void );

extern cable_driver_t *cable_drivers[];

#endif /* CABLE_H */
