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
#include <string.h>

#include "fclock.h"
#include "jtag.h"
#include "cable.h"

extern cable_driver_t arcom_cable_driver;
extern cable_driver_t byteblaster_cable_driver;
#ifdef HAVE_LIBFTDI
extern cable_driver_t usbblaster_cable_driver;
extern cable_driver_t ft2232_cable_driver;
extern cable_driver_t ft2232_jtagkey_cable_driver;
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
#ifdef ENABLE_JIM
extern cable_driver_t jim_cable_driver;
#endif
extern cable_driver_t wiggler_cable_driver;
extern cable_driver_t wiggler2_cable_driver;
extern cable_driver_t wiggler_cable_driver;
#ifdef HAVE_LIBUSB
extern cable_driver_t xpc_int_cable_driver;
extern cable_driver_t xpc_ext_cable_driver;
#endif
#ifdef ENABLE_EP9307
extern cable_driver_t ep9307_cable_driver;
#endif

cable_driver_t *cable_drivers[] = {
	&arcom_cable_driver,
	&byteblaster_cable_driver,
#ifdef HAVE_LIBFTDI
	&usbblaster_cable_driver,
	&ft2232_cable_driver,
	&ft2232_jtagkey_cable_driver,
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
#ifdef ENABLE_JIM
	&jim_cable_driver,
#endif
	&wiggler_cable_driver,
	&wiggler2_cable_driver,	
#ifdef HAVE_LIBUSB
	&xpc_int_cable_driver,
	&xpc_ext_cable_driver,
#endif
#ifdef ENABLE_EP9307
	&ep9307_cable_driver,
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
	cable->delay = 0;
	cable->frequency = 0;

	cable->todo.max_items = 128;
	cable->todo.num_items = 0;
	cable->todo.next_item = 0;
	cable->todo.next_free = 0;
	cable->todo.data = malloc(cable->todo.max_items*sizeof(cable_queue_t));

	cable->done.max_items = 128;
	cable->done.num_items = 0;
	cable->done.next_item = 0;
	cable->done.next_free = 0;
	cable->done.data = malloc(cable->done.max_items*sizeof(cable_queue_t));

	if(cable->todo.data == NULL || cable->done.data == NULL)
	{
		printf(_("Failed to allocate memory for cable activity queue.\n"));
		if(cable->todo.data != NULL) free(cable->todo.data);
		if(cable->done.data != NULL) free(cable->done.data);
		return 1;
	};

	return cable->driver->init( cable );
}

void
cable_flush ( cable_t *cable )
{
	cable->driver->flush( cable );
}

void
cable_done( cable_t *cable )
{
	cable_flush( cable );
	if( cable->todo.data != NULL)
	{
		free( cable->todo.data );
		free( cable->done.data );
	}
	return cable->driver->done( cable );
}

int
cable_add_queue_item( cable_t *cable, cable_queue_info_t *q )
{
	int i,j;
	if( q->num_items >= q->max_items ) /* queue full? */
	{
		int new_max_items;
		cable_queue_t *resized;

        new_max_items = q->max_items + 128;
        resized = realloc(q->data, new_max_items * sizeof(cable_queue_t));
        if(resized == NULL)
        {
           printf(_("Out of memory: couldn't resize activity queue to %d\n"),
                new_max_items);
           return -1; /* report failure */
        }
        printf(_("(Resized JTAG activity queue to hold max %d items)\n"),
             new_max_items);
        q->data = resized;

        /* The queue was full. Except for the special case when next_item is 0,
		 * resizing just introduced a gap between old and new max, which has to
		 * be filled; either by moving data from next_item .. max_items, or
		 * from 0 .. next_free (whatever is smaller). */

#define CHOOSE_SMALLEST_AREA_TO_MOVE 1

        if(q->next_item != 0)
        {
          int added_space = new_max_items - q->max_items;
          int num_to_move = q->max_items - q->next_item;

#ifdef CHOOSE_SMALLEST_AREA_TO_MOVE
          if(num_to_move <= q->next_free)
#endif
          {
            /* Move queue items at end of old array
             * towards end of new array: 345612__ -> 3456__12 */

            memmove(&(q->data[q->max_items]), &(q->data[q->next_item]),
                    num_to_move * sizeof(cable_queue_t));

            q->next_item += num_to_move;
          }
#ifdef CHOOSE_SMALLEST_AREA_TO_MOVE
          else
          {
            if(q->next_free <= added_space)
            {
              /* Relocate queue items at beginning of old array
               * to end of new array: 561234__ -> __123456 */

              memcpy(&(q->data[q->max_items]), &(q->data[0]), 
                  q->next_free * sizeof(cable_queue_t));

              q->next_free += q->max_items;
            }
            else
            {
              /* Same as above, but for the case if new space 
               * isn't large enough to hold all relocated items */

              /* Step 1: 456123__ -> __612345 */

              memcpy(&(q->data[q->max_items]), &(q->data[0]), 
                  added_space * sizeof(cable_queue_t));

              /* Step 2: __612345 -> 6__12345 */

              memmove(&(q->data[0]), &(q->data[added_space]), 
                  (q->next_free - added_space) * sizeof(cable_queue_t));

              q->next_free -= added_space;
            }
          }
#endif
        }
        q->max_items = new_max_items;
	}

	i = q->next_free;
	j = i+1;
	if( j >= q->max_items ) j = 0;
	q->next_free = j;
	q->num_items ++;

	// printf("add_queue_item to %p: %d\n", q, i);
	return i;
}

int
cable_get_queue_item( cable_t *cable, cable_queue_info_t *q )
{
	if(q->num_items > 0)
	{
		int i = q->next_item;
		int j = i+1;
		if( j >= q->max_items ) j = 0;
		q->next_item = j;
		q->num_items --;
		// printf("get_queue_item from %p: %d\n", q, i);
		return i;
	}

	// printf("get_queue_item from %p: %d\n", q, -1);
    return -1;
}

void
cable_purge_queue( cable_queue_info_t *q, int io )
{
	while(q->num_items > 0)
	{
		int i = q->next_item;
		if(q->data[i].action == CABLE_TRANSFER)
		{
			if(io == 0) /* todo queue */
			{
				if(q->data[i].arg.transfer.in != NULL) free(q->data[i].arg.transfer.in);
				if(q->data[i].arg.transfer.out != NULL) free(q->data[i].arg.transfer.out);
			}
			else /* done queue */
			{
				if(q->data[i].arg.xferred.out != NULL) free(q->data[i].arg.xferred.out);
			}
		}

		i++;
		if(i >= q->max_items) i = 0;
		q->num_items--;
	};

	q->num_items = 0;
	q->next_item = 0;
	q->next_free = 0;
}

void
cable_clock( cable_t *cable, int tms, int tdi, int n )
{
	cable_flush( cable );
	cable->driver->clock( cable, tms, tdi, n );
}

int
cable_defer_clock ( cable_t *cable, int tms, int tdi, int n )
{
	int i = cable_add_queue_item( cable, &(cable->todo) );
	if( i < 0 ) return 1; /* report failure */
	cable->todo.data[i].action = CABLE_CLOCK;
	cable->todo.data[i].arg.clock.tms = tms;
	cable->todo.data[i].arg.clock.tdi = tdi;
	cable->todo.data[i].arg.clock.n   = n;
	return 0; /* success */
}

int
cable_get_tdo( cable_t *cable )
{
	cable_flush( cable );
	return cable->driver->get_tdo( cable );
}

int
cable_get_tdo_late( cable_t *cable )
{
	int i;
	cable_flush( cable );
	i = cable_get_queue_item( cable, &(cable->done) );
	if( i >= 0 )
	{
		if(cable->done.data[i].action != CABLE_GET_TDO)
		{
			printf(_("Internal error: Got wrong type of result from queue (%d? %p.%d)\n"),
				cable->done.data[i].action, &(cable->done), i);
			cable_purge_queue( &(cable->done), 1 );
		}
		else
		{
			return cable->done.data[i].arg.value.tdo;
		}
	};
	return cable->driver->get_tdo( cable );
}

int
cable_defer_get_tdo( cable_t *cable )
{
	int i = cable_add_queue_item( cable, &(cable->todo) );
	if( i < 0 ) return 1; /* report failure */
	cable->todo.data[i].action = CABLE_GET_TDO;
	return 0; /* success */
}

int
cable_set_trst( cable_t *cable, int trst )
{
    cable_flush( cable );
	return cable->driver->set_trst( cable, trst );
}

int
cable_defer_set_trst( cable_t *cable, int trst )
{
	int i = cable_add_queue_item( cable, &(cable->todo) );
	if( i < 0 ) return 1; /* report failure */
	cable->todo.data[i].action = CABLE_SET_TRST;
	cable->todo.data[i].arg.value.trst = trst;
	return 0; /* success */
}

int
cable_get_trst( cable_t *cable )
{
	cable_flush( cable );
	return cable->driver->get_trst( cable );
}

int
cable_get_trst_late( cable_t *cable )
{
	int i;
	cable_flush( cable );
	i = cable_get_queue_item( cable, &(cable->done) );
	if( i >= 0 )
	{
		if(cable->done.data[i].action != CABLE_GET_TRST)
		{
			printf(_("Internal error: Got wrong type of result from queue (%d? %p.%d)\n"),
				cable->done.data[i].action, &(cable->done), i);
			cable_purge_queue( &(cable->done), 1 );
		}
		else
		{
			return cable->done.data[i].arg.value.trst;
		}
	};
	return cable->driver->get_trst( cable );
}

int
cable_defer_get_trst( cable_t *cable )
{
	int i = cable_add_queue_item( cable, &(cable->todo) );
	if( i < 0 ) return 1; /* report failure */
	cable->todo.data[i].action = CABLE_GET_TRST;
	return 0; /* success */
}

int
cable_transfer( cable_t *cable, int len, char *in, char *out )
{
	cable_flush( cable );
	return cable->driver->transfer( cable, len, in, out );
}

int
cable_transfer_late( cable_t *cable, char *out )
{
	int i;
	cable_flush( cable );
	i = cable_get_queue_item( cable, &(cable->done) );

	if( i >= 0 && cable->done.data[i].action == CABLE_TRANSFER)
	{
#if 0
		printf("Got queue item (%p.%d) len=%d out=%p\n", 
			&(cable->done), i, 
				cable->done.data[i].arg.xferred.len,
				cable->done.data[i].arg.xferred.out);
#endif
		if(out) memcpy(out,
				cable->done.data[i].arg.xferred.out,
				cable->done.data[i].arg.xferred.len);
		free(cable->done.data[i].arg.xferred.out);
		return cable->done.data[i].arg.xferred.res;
	};
	
	if(cable->done.data[i].action != CABLE_TRANSFER)
	{
		printf(_("Internal error: Got wrong type of result from queue (#%d %p.%d)\n"),
			cable->done.data[i].action, &(cable->done), i);
		cable_purge_queue( &(cable->done), 1 );
	}
	else
	{
		printf(_("Internal error: Wanted transfer result but none was queued\n"));
	}
	return 0;
}

int
cable_defer_transfer( cable_t *cable, int len, char *in, char *out )
{
	char *dbuf;
	int i = cable_add_queue_item( cable, &(cable->todo) );
	if( i < 0 ) return 1; /* report failure */
	cable->todo.data[i].action = CABLE_TRANSFER;
	cable->todo.data[i].arg.transfer.len = len;

	dbuf = malloc(len);
	if(dbuf == NULL) return 1; /* report failure */
	if(in) memcpy(dbuf, in, len);
	cable->todo.data[i].arg.transfer.in  = dbuf;

	if(out)
	{
		dbuf = malloc(len);
		if(dbuf == NULL)
		{
			free(cable->todo.data[i].arg.transfer.in);
			return 1; /* report failure */
		}
		cable->todo.data[i].arg.transfer.out = dbuf;
	}
	else
	{
		cable->todo.data[i].arg.transfer.out = NULL;
	}
	return 0; /* success */
}

void
cable_set_frequency( cable_t *cable, uint32_t new_frequency )
{
	if (new_frequency == 0) {
		cable->delay = 0;
		cable-> frequency = 0;
	} else {
		const double tolerance = 0.1;
		uint32_t loops;
		uint32_t delay = cable->delay;
		uint32_t frequency = cable->frequency;

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

		cable->delay = delay;
		cable->frequency = frequency;
	}
}

uint32_t
cable_get_frequency( cable_t *cable )
{
	return cable->frequency;
}

void
cable_wait( cable_t *cable )
{
	int i;
	volatile int j;
	uint32_t delay = cable->delay;

	if (delay == 0)
		return;

	for (i = 0; i < delay; ++i) {
		j = i;
	}
}
