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

typedef enum
{
   OPTIONALLY,
   CONSERVATIVELY,
   TO_OUTPUT,
   COMPLETELY
}
cable_flush_amount_t;

struct cable_driver_t {
	const char *name;
	const char *description;
	int (*connect)( char *params[], cable_t *cable );
	void (*disconnect)( cable_t *cable );
	void (*cable_free)( cable_t *cable );
	int (*init)( cable_t * );
	void (*done)( cable_t * );
	void (*set_frequency)( cable_t *, uint32_t freq );
	void (*clock)( cable_t *, int, int, int );
	int (*get_tdo)( cable_t * );
	int (*transfer)( cable_t *, int, char *, char * );
	int (*set_trst)( cable_t *, int );
	int (*get_trst)( cable_t * );
	void (*flush)( cable_t *, cable_flush_amount_t );
	void (*help)( const char * );
};

typedef struct cable_queue_t cable_queue_t;

struct cable_queue_t {
	enum {
		CABLE_CLOCK,
		CABLE_GET_TDO,
		CABLE_TRANSFER,
		CABLE_SET_TRST,
		CABLE_GET_TRST
	} action;
	union {
		struct {
			int tms;
			int tdi;
			int n;
		} clock;
		struct {
			int tdo;
			int trst;
			int val;
		} value;
		struct {
			int len;
			char *in;
			char *out;
		} transfer;
		struct {
			int len;
			int res;
			char *out;
		} xferred;
	} arg;
};

typedef struct cable_queue_info_t cable_queue_info_t;

struct cable_queue_info_t {
	cable_queue_t *data;
	int	max_items;
	int num_items;
	int next_item;
	int next_free;
};

struct cable_t {
	cable_driver_t *driver;
	parport_t *port;
	void *params;
	chain_t *chain;
	cable_queue_info_t todo;
	cable_queue_info_t done;
	uint32_t delay;
	uint32_t frequency;
};

void cable_free( cable_t *cable );
int cable_init( cable_t *cable );
void cable_done( cable_t *cable );
void cable_flush( cable_t *cable, cable_flush_amount_t );
void cable_clock( cable_t *cable, int tms, int tdi, int n );
   int cable_defer_clock( cable_t *cable, int tms, int tdi, int n );
int cable_get_tdo( cable_t *cable );
   int cable_get_tdo_late( cable_t *cable );
   int cable_defer_get_tdo( cable_t *cable );
int cable_set_trst( cable_t *cable, int trst );
  int cable_defer_set_trst( cable_t *cable, int trst );
int cable_get_trst( cable_t *cable );
   int cable_get_trst_late( cable_t *cable );
   int cable_defer_get_trst( cable_t *cable );
int cable_transfer( cable_t *cable, int len, char *in, char *out );
   int cable_transfer_late( cable_t *cable, char *out );
   int cable_defer_transfer( cable_t *cable, int len, char *in, char *out );

void cable_set_frequency( cable_t *cable, uint32_t frequency );
uint32_t cable_get_frequency( cable_t *cable );
void cable_wait( cable_t *cable );
void cable_purge_queue( cable_queue_info_t *q, int io );
int cable_add_queue_item( cable_t *cable, cable_queue_info_t *q );
int cable_get_queue_item( cable_t *cable, cable_queue_info_t *q );

extern cable_driver_t *cable_drivers[];

#endif /* CABLE_H */
