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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"

#include <cmd.h>

#undef VERBOSE

#ifdef VERBOSE
void
print_vector(int len, char *vec)
{
	int i;
	for(i=0;i<len;i++) printf("%c",vec[i]?'1':'0');
}
#endif

int
generic_connect( char *params[], cable_t *cable )
{
	generic_params_t *cable_params = malloc( sizeof *cable_params );
	parport_t *port;
	int i;

	if ( cmd_params( params ) < 3 ) {
	  printf( _("not enough arguments!\n") );
	  return 1;
	}
	  
	/* search parport driver list */
	for (i = 0; parport_drivers[i]; i++)
		if (strcasecmp( params[1], parport_drivers[i]->type ) == 0)
			break;
	if (!parport_drivers[i]) {
		printf( _("Unknown port driver: %s\n"), params[1] );
		return 2;
	}

	/* set up parport driver */
	port = parport_drivers[i]->connect( (const char **) &params[2],
					    cmd_params( params ) - 2 );

        if (port == NULL) {
	  printf( _("Error: Cable connection failed!\n") );
	  return 3;
        }

	if (!cable_params) {
		free( cable_params );
		free( cable );
		return 4;
	}

	cable->port = port;
	cable->params = cable_params;
	cable->chain = NULL;

	return 0;
}

void
generic_disconnect( cable_t *cable )
{
	cable_done( cable );
	chain_disconnect( cable->chain );
}

void
generic_cable_free( cable_t *cable )
{
	cable->port->driver->parport_free( cable->port );
	free( cable->params );
	free( cable );
}

void
generic_done( cable_t *cable )
{
	parport_close( cable->port );
}

int
generic_transfer( cable_t *cable, int len, char *in, char *out )
{
	int i;

	if(out)
		for(i=0; i<len; i++) {
			out[i] = cable->driver->get_tdo( cable );
			cable->driver->clock( cable, 0, in[i], 1 );
		}
	else
		for(i=0; i<len; i++) {
			cable->driver->clock( cable, 0, in[i], 1 );
		}

	return i;
}

int
generic_get_trst( cable_t *cable )
{
	return PARAM_TRST(cable);
}

int
do_one_queued_action( cable_t *cable )
{
	int i;

#ifdef VERBOSE
	printf("do_one_queued\n");
#endif

	if ( (i = cable_get_queue_item( cable, &(cable->todo) )) >= 0 )
	{
		int j;

		if( cable->done.num_items >= cable->done.max_items )
		{
			if( cable->todo.data[i].action == CABLE_GET_TDO
				|| cable->todo.data[i].action == CABLE_GET_TRST
				|| cable->todo.data[i].action == CABLE_TRANSFER )
			{
				printf(_("No space in cable activity results queue.\n"));
				cable_purge_queue( &(cable->done), 1 );
			}
		};

		switch(cable->todo.data[i].action)
		{
			case CABLE_CLOCK:
				cable->driver->clock( cable,
					cable->todo.data[i].arg.clock.tms,
					cable->todo.data[i].arg.clock.tdi,
					cable->todo.data[i].arg.clock.n );
				break;
			case CABLE_SET_TRST:
				cable_set_trst( cable,
					cable->todo.data[i].arg.value.trst );
				break;
			case CABLE_TRANSFER:
			{
				int r = cable->driver->transfer( cable,
						cable->todo.data[i].arg.transfer.len,
						cable->todo.data[i].arg.transfer.in,
						cable->todo.data[i].arg.transfer.out);

				free(cable->todo.data[i].arg.transfer.in);
				if(cable->todo.data[i].arg.transfer.out != NULL)
				{
					j = cable_add_queue_item( cable, &(cable->done) );
#ifdef VERBOSE
					printf("add result from transfer to %p.%d (out=%p)\n", &(cable->done), j, cable->todo.data[i].arg.transfer.out);
#endif
					cable->done.data[j].action = CABLE_TRANSFER;
					cable->done.data[j].arg.xferred.len = cable->todo.data[i].arg.transfer.len;
					cable->done.data[j].arg.xferred.res = r;
					cable->done.data[j].arg.xferred.out = cable->todo.data[i].arg.transfer.out;
				};
				break;
			};
			case CABLE_GET_TDO:
				j = cable_add_queue_item( cable, &(cable->done) );
#ifdef VERBOSE
				printf("add result from get_tdo to %p.%d\n", &(cable->done), j);
#endif
				cable->done.data[j].action = CABLE_GET_TDO;
				cable->done.data[j].arg.value.tdo =
					cable->driver->get_tdo( cable );
				break;
			case CABLE_GET_TRST:
				j = cable_add_queue_item( cable, &(cable->done) );
#ifdef VERBOSE
				printf("add result from get_trst to %p.%d\n", &(cable->done), j);
#endif
				cable->done.data[j].action = CABLE_GET_TRST;
				cable->done.data[j].arg.value.trst =
					cable->driver->get_trst( cable );
				break;
		};
#ifdef VERBOSE
		printf("do_one_queued done\n");
#endif

		return 1;
	}
#ifdef VERBOSE
	printf("do_one_queued abort\n");
#endif

	return 0;
}

void
generic_flush_one_by_one( cable_t *cable, cable_flush_amount_t how_much )
{
    if( how_much == OPTIONALLY ) return;

	while( do_one_queued_action( cable ) );
}

void
generic_flush_using_transfer( cable_t *cable, cable_flush_amount_t how_much )
{
	int i, j, n;
	char *in, *out;

    if( how_much == OPTIONALLY ) return;

	if(cable->todo.num_items == 0) return;

	do
	{
		int r, bits = 0, tdo;

#ifdef VERBOSE
		printf("flush(%d)\n", cable->todo.num_items);
#endif

		/* Combine as much as possible into transfer() */

		/* Step 1: Count clocks. Can do only clock(TMS=0), get_tdo, transfer */

		for(i = cable->todo.next_item, n=0; n < cable->todo.num_items; n++)
		{
			if(cable->todo.data[i].action != CABLE_CLOCK
				&& cable->todo.data[i].action != CABLE_TRANSFER
				&& cable->todo.data[i].action != CABLE_GET_TDO)
			{
#ifdef VERBOSE
				printf("cutoff at n=%d because action unsuitable for transfer\n", n);
#endif
				break;
			}
			if(cable->todo.data[i].action == CABLE_CLOCK
				&& cable->todo.data[i].arg.clock.tms != 0)
			{
#ifdef VERBOSE
				printf("cutoff at n=%d because clock.tms=1 is unsuitable for transfer\n", n);
#endif
				break;
			}
			if(cable->todo.data[i].action == CABLE_CLOCK)
			{
				int k = cable->todo.data[i].arg.clock.n;
#ifdef VERBOSE
				printf("%d clock(s)\n", k);
#endif
				bits += k;
			}
			else if(cable->todo.data[i].action == CABLE_TRANSFER)
			{
				int k = cable->todo.data[i].arg.transfer.len;
#ifdef VERBOSE
	  			printf("%d transfer\n", k);
#endif
				bits += k;
			}
			i++;
			if(i >= cable->todo.max_items) i = 0;
		};

#ifdef VERBOSE
		printf("%d combined into one (%d bits)\n", n, bits);
#endif

		if(bits == 0 || n <= 1)
		{
			do_one_queued_action( cable );
		}
		else
		{
			/* Step 2: Combine into single transfer. */

			in = malloc(bits);
			out = malloc(bits);

			if(in == NULL || out == NULL)
			{
				if(in != NULL) free(in);
				if(out != NULL) free(out);
				generic_flush_one_by_one( cable, how_much );
				break; 
			};

			for(j=0, bits=0, i=cable->todo.next_item; j<n; j++)
			{
				if(cable->todo.data[i].action == CABLE_CLOCK)
				{
					int k;
					for(k=0;k<cable->todo.data[i].arg.clock.n;k++)
						in[bits++] = cable->todo.data[i].arg.clock.tdi;
				}
				else if(cable->todo.data[i].action == CABLE_TRANSFER)
				{
					int len = cable->todo.data[i].arg.transfer.len;
					if(len>0)
					{
						memcpy(in+bits, cable->todo.data[i].arg.transfer.in, len);
						bits += len;
					};
				};
				i++;
				if(i >= cable->todo.max_items) i = 0;
			};

			/* Step 3: Do the transfer */
	
			r = cable->driver->transfer( cable, bits, in, out );
#ifdef VERBOSE
			printf("in: "); print_vector(bits,in); printf("\n");
			if(out) { printf("out: "); print_vector(bits,out); printf("\n"); };
#endif
	
			/* Step 4: Pick results from transfer */ 
	
			for(j=0, bits=0, i=cable->todo.next_item; j<n; j++)
			{
				if(cable->todo.data[i].action == CABLE_CLOCK)
				{
					int k;
					for(k=0;k<cable->todo.data[i].arg.clock.n;k++)
						tdo = out[bits++];
				}
				else if(cable->todo.data[i].action == CABLE_GET_TDO)
				{
					int c = cable_add_queue_item( cable, &(cable->done) );
#ifdef VERBOSE
					printf("add result from transfer to %p.%d\n", &(cable->done), c);
#endif
					cable->done.data[c].action = CABLE_GET_TDO;
					cable->done.data[c].arg.value.tdo = tdo;
				}
				else if(cable->todo.data[i].action == CABLE_TRANSFER)
				{
					char *p = cable->todo.data[i].arg.transfer.out;
					int len = cable->todo.data[i].arg.transfer.len;
					free(cable->todo.data[i].arg.transfer.in);
					if(p != NULL)
					{
						int c = cable_add_queue_item( cable, &(cable->done) );
#ifdef VERBOSE
						printf("add result from transfer to %p.%d\n", &(cable->done), c);
#endif
						cable->done.data[c].action = CABLE_TRANSFER;
						cable->done.data[c].arg.xferred.len = len;
						cable->done.data[c].arg.xferred.res = r;
						cable->done.data[c].arg.xferred.out = p;
						if(len > 0) memcpy(p, out+bits, len);
					}
					if(len>0) bits += len;
					if(bits>0) tdo = out[bits-1];
				};
				i++;
				if(i >= cable->todo.max_items) i = 0;
			}

			cable->todo.next_item = i;
			cable->todo.num_items -= n;

			free(in);
			free(out);
		}
	}
	while(cable->todo.num_items > 0);
}

void
generic_lptcable_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s parallel PORTADDR\n"
#if HAVE_LINUX_PPDEV_H
		"   or: cable %s ppdev PPDEV\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
		"   or: cable %s ppi PPIDEV\n"
#endif
		"\n"
		"PORTADDR   parallel port address (e.g. 0x378)\n"
#if HAVE_LINUX_PPDEV_H
		"PPDEF      ppdev device (e.g. /dev/parport0)\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
		"PPIDEF     ppi device (e.g. /dev/ppi0)\n"
#endif
		"\n"
	),
#if HAVE_LINUX_PPDEV_H
    cablename,
#endif
#if HAVE_DEV_PPBUS_PPI_H
    cablename,
#endif
    cablename 
    );
}

