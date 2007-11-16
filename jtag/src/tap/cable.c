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

#include "sysdep.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include "fclock.h"
#include "jtag.h"
#include "cable.h"

extern cable_driver_t arcom_cable_driver;
extern cable_driver_t byteblaster_cable_driver;
#ifdef HAVE_LIBFTDI
extern cable_driver_t usbblaster_cable_driver;
extern cable_driver_t ft2232_cable_driver;
extern cable_driver_t ft2232_armusbocd_cable_driver;
#endif
extern cable_driver_t dlc5_cable_driver;
extern cable_driver_t ea253_cable_driver;
extern cable_driver_t ei012_cable_driver;
extern cable_driver_t igloo_cable_driver;
extern cable_driver_t keithkoep_cable_driver;
extern cable_driver_t lattice_cable_driver;
extern cable_driver_t mpcbdm_cable_driver;
extern cable_driver_t triton_cable_driver;
extern cable_driver_t wiggler_cable_driver;
extern cable_driver_t wiggler2_cable_driver;
extern cable_driver_t wiggler_cable_driver;
#ifdef HAVE_LIBUSB
extern cable_driver_t xpc_int_cable_driver;
extern cable_driver_t xpc_ext_cable_driver;
#endif

cable_driver_t *cable_drivers[] = {
	&arcom_cable_driver,
	&byteblaster_cable_driver,
#ifdef HAVE_LIBFTDI
	&usbblaster_cable_driver,
	&ft2232_cable_driver,
	&ft2232_armusbocd_cable_driver,
#endif
	&dlc5_cable_driver,
	&ea253_cable_driver,
	&ei012_cable_driver,
	&igloo_cable_driver,
	&keithkoep_cable_driver,
	&lattice_cable_driver,
	&mpcbdm_cable_driver,
	&triton_cable_driver,
	&wiggler_cable_driver,
	&wiggler2_cable_driver,	
#ifdef HAVE_LIBUSB
	&xpc_int_cable_driver,
	&xpc_ext_cable_driver,
#endif
	NULL				/* last must be NULL */
};

void
cable_free( cable_t *cable )
{
	cable->driver->cable_free( cable );
}

int
cable_init( cable_t *cable )
{
	return cable->driver->init( cable );
}

void
cable_done( cable_t *cable )
{
	return cable->driver->done( cable );
}

void
cable_clock( cable_t *cable, int tms, int tdi, int n )
{
	cable->driver->clock( cable, tms, tdi, n );
}

int
cable_get_tdo( cable_t *cable )
{
	return cable->driver->get_tdo( cable );
}

int
cable_set_trst( cable_t *cable, int trst )
{
	return cable->driver->set_trst( cable, trst );
}

int
cable_get_trst( cable_t *cable )
{
	return cable->driver->get_trst( cable );
}

static uint32_t delay = 0;
static uint32_t frequency = 0;

void
cable_set_frequency( cable_t *cable, uint32_t new_frequency )
{
	if (new_frequency == 0) {
		delay = 0;
		frequency = 0;
	} else {
		const double tolerance = 0.1;
		uint32_t loops;

		printf("requested frequency %u, now calibrating delay loop\n", new_frequency);

		if (delay == 0) {
			delay = 1000;
			loops = 10000;
		} else {
			loops = 3 * frequency;
		}

		while (1) {
			uint32_t i, new_delay;
			long double start, end, real_frequency;

			start = frealtime();	
			for (i = 0; i < loops; ++i) {
				chain_clock(chain, 0, 0, 1);
			}
			end = frealtime();

			assert(end > start);
			real_frequency = (long double)loops / (end - start);
			printf("new real frequency %Lg, delay %u\n", 
			       real_frequency, delay);

			loops = 3 * fmax(real_frequency, new_frequency);
			new_delay = (long double)delay * real_frequency / new_frequency;

			if (real_frequency >= (1.0 - tolerance)*new_frequency) {
				if (real_frequency <= (1.0 + tolerance)*new_frequency) {
					break;
				}
				if (new_delay > delay) {
					delay = new_delay;
				} else {
					delay++;
				}
			} else {
				if (new_delay < delay) {
					delay = new_delay;
				} else {
					delay--;
				}			
				if (delay == 0) {
					printf("operating without delay\n");
					break;
				}
			}
		}

		frequency = new_frequency;
	}
}

uint32_t
cable_get_frequency( cable_t *cable )
{
	return frequency;
}

int
cable_transfer( cable_t *cable, int len,  char *in, char *out )
{
	int r;
	r=cable->driver->transfer( cable, len, in, out );
	return r;
}

void
cable_wait( void )
{
	int i;
	volatile int j;

	if (delay == 0)
		return;

	for (i = 0; i < delay; ++i) {
		j = i;
	}
}
