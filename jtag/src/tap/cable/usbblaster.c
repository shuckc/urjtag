/*
 * $Id: usbblaster.c,v 1.8 2003/08/22 22:42:02 telka Exp $
 *
 * Altera USB-Blaster<tm> Cable Driver
 * Copyright (C) 2006 K. Waschk
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
 * Written by Kolja Waschk, 2006; http://www.ixo.de
 *
 */

#include "sysdep.h"

#include <stdlib.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"
#include "generic_parport.h"

#define TCK    0
#define TMS    1
#define TDI    4
#define READ   6
#define SHMODE 7
#define OTHERS ((1<<2)|(1<<3)|(1<<5))

#define TDO    0

static int
usbblaster_init( cable_t *cable )
{
	int i;

	if (parport_open( cable->link.port ))
		return -1;

	for(i=0;i<64;i++)
		parport_set_data( cable->link.port, 0 );

	parport_set_control( cable->link.port, 1 ); // flush
	parport_set_control( cable->link.port, 0 ); // noflush

	return 0;
}

static void
usbblaster_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i, m;

	tms = tms ? (1<<TMS) : 0;
	tdi = tdi ? (1<<TDI) : 0;

	// printf("clock: %d %d %d\n", tms, tdi, n);

	m = n;
	if (tms == 0 && m >= 8)
	{
		unsigned char tdis = tdi ? 0xFF : 0;

		parport_set_control( cable->link.port, 0 ); // noflush

		while (m >= 8)
		{
			int i;
			int chunkbytes = (m >> 3);
			if(chunkbytes > 63) chunkbytes = 63;

			parport_set_data( cable->link.port,(1<<SHMODE)|(0<<READ)|chunkbytes);

			for (i=0; i<chunkbytes; i++)
			{
				parport_set_data( cable->link.port, tdis);
			}

			m -= (chunkbytes << 3);
		}
	}
			
	for (i = 0; i < m; i++) {
		parport_set_data( cable->link.port, OTHERS | (0 << TCK) | tms | tdi );
		parport_set_data( cable->link.port, OTHERS | (1 << TCK) | tms | tdi );
		parport_set_control( cable->link.port, 1 ); // flush
		parport_set_control( cable->link.port, 0 ); // noflush
	}
}

static int
usbblaster_get_tdo( cable_t *cable )
{
	parport_set_control( cable->link.port, 0 ); // noflush
	parport_set_data( cable->link.port, OTHERS ); /* TCK low */
	parport_set_data( cable->link.port, OTHERS | (1 << READ) ); /* TCK low */
	parport_set_control( cable->link.port, 1 ); // flush
	parport_set_control( cable->link.port, 0 ); // noflush
#if 0
    {
	  char x = ( parport_get_data( cable->link.port ) & (1 << TDO)) ? 1 : 0;
      printf("GetTDO %d\n", x);
      return x;
    }
#else
	return ( parport_get_data( cable->link.port ) & (1 << TDO)) ? 1 : 0;
#endif
}

static int
usbblaster_set_trst( cable_t *cable, int trst )
{
	return 1;
}

static int
usbblaster_transfer( cable_t *cable, int len, char *in, char *out )
{
	int in_offset = 0;
	int out_offset = 0;
	parport_set_control( cable->link.port, 0 );
	parport_set_data( cable->link.port, OTHERS ); /* TCK low */

#if 0
				{
					int o;
					printf("%d in: ", len);
					for(o=0;o<len;o++) printf("%c", in[o]?'1':'0');
					printf("\n");
				}
#endif

	while(len - in_offset >= 8)
	{
		int i;
		int chunkbytes = ((len-in_offset)>>3);
		if(chunkbytes > 63) chunkbytes = 63;

		if(out)
			parport_set_data( cable->link.port,(1<<SHMODE)|(1<<READ)|chunkbytes);
		else
			parport_set_data( cable->link.port,(1<<SHMODE)|(0<<READ)|chunkbytes);

		for(i=0; i<chunkbytes; i++)
		{
			int j;
			unsigned char b = 0;
			for(j=1; j<256; j<<=1) if(in[in_offset++]) b |= j;
			parport_set_data( cable->link.port, b );
		};

		if(out) 
		{
			parport_set_control( cable->link.port, 1 ); // flush
			parport_set_control( cable->link.port, 0 ); 

			for(i=0; i<chunkbytes; i++)
			{
				int j;
				unsigned char b = parport_get_data( cable->link.port );
#if 0
                printf("read byte: %02X\n", b);
#endif
                 
				for(j=1; j<256; j<<=1) out[out_offset++] = (b & j) ? 1:0;
			};
		};
	};

	while(len > in_offset)
	{
		char tdi = in[in_offset++] ? 1 : 0;
		parport_set_data( cable->link.port, OTHERS | ((out)?(1 << READ):0) | (tdi << TDI));/* TCK low */
		parport_set_data( cable->link.port, OTHERS | (1 << TCK)  | (tdi << TDI));
	}

	if(out)
	{
		parport_set_control( cable->link.port, 1 ); // flush
		parport_set_control( cable->link.port, 0 );

		while(len > out_offset)
			out[out_offset++] = ( parport_get_data( cable->link.port ) & (1 << TDO)) ? 1 : 0;

#if 0
				{
					int o;
					printf("%d out: ", len);
					for(o=0;o<len;o++) printf("%c", out[o]?'1':'0');
					printf("\n");
				}
#endif

	}

	return 0;
}

static void
usbblaster_flush( cable_t *cable, cable_flush_amount_t how_much )
{
	if( how_much == OPTIONALLY ) return;

	while (cable->todo.num_items > 0)
	{
		int i, j, n, to_send = 0;

		for(j=i=cable->todo.next_item, n=0; to_send < 64 && n<cable->todo.num_items; n++)
		{
			if(cable->todo.data[i].action == CABLE_TRANSFER) break;

			switch(cable->todo.data[i].action)
			{
				case CABLE_CLOCK:
				{
					int tms = cable->todo.data[i].arg.clock.tms ? (1<<TMS) : 0;
					int tdi = cable->todo.data[i].arg.clock.tdi ? (1<<TDI) : 0;
					int m   = cable->todo.data[i].arg.clock.n;
					// printf("clock: %d %d %d\n", tms, tdi, m);
					for(; m>0; m--)
					{
						parport_set_data( cable->link.port, OTHERS | tms | tdi );
						parport_set_data( cable->link.port, OTHERS | (1 << TCK) | tms | tdi );
						to_send += 2;
					}
					break;
				}
				case CABLE_GET_TDO:
				{
					parport_set_data( cable->link.port, OTHERS ); /* TCK low */
					parport_set_data( cable->link.port, OTHERS | (1 << READ) ); /* TCK low */
					// printf("get_tdo\n");
					to_send += 2;
					break;
				}
				default:
					break;
			};

			i++;
			if (i >= cable->todo.max_items) i=0;
		};

#if 0
		if(cable->todo.num_items > 0 && cable->todo.data[i].action == CABLE_TRANSFER)
		{
			parport_set_data( cable->link.port, OTHERS ); /* TCK low */
		};
#endif

		if(to_send > 0)
		{
			parport_set_control( cable->link.port, 1 ); // flush
			parport_set_control( cable->link.port, 0 );
		}

		while(j!=i)
		{
			switch(cable->todo.data[j].action)
			{
				case CABLE_GET_TDO:
				{
					int tdo = (parport_get_data( cable->link.port ) & (1<<TDO)) ? 1 : 0;
					int m = cable_add_queue_item( cable, &(cable->done) );
					cable->done.data[m].action = CABLE_GET_TDO;
					cable->done.data[m].arg.value.tdo = tdo;
					break;
				}
				case CABLE_GET_TRST:
				{
					int m = cable_add_queue_item( cable, &(cable->done) );
					cable->done.data[m].action = CABLE_GET_TRST;
					cable->done.data[m].arg.value.trst = 1;
					break;
				}
				default:
					break;
			};

			j++;
			if (j >= cable->todo.max_items) j=0;
			cable->todo.num_items --;
		};

		while(cable->todo.num_items > 0 && cable->todo.data[i].action == CABLE_TRANSFER)
		{
			int r = usbblaster_transfer( cable,
				cable->todo.data[i].arg.transfer.len,
				cable->todo.data[i].arg.transfer.in,
				cable->todo.data[i].arg.transfer.out);

			free(cable->todo.data[i].arg.transfer.in);
			if(cable->todo.data[i].arg.transfer.out != NULL)
			{
				int m = cable_add_queue_item( cable, &(cable->done) );
				if(m < 0) printf("out of memory!!\n");
				cable->done.data[m].action = CABLE_TRANSFER;
				cable->done.data[m].arg.xferred.len = cable->todo.data[i].arg.transfer.len;
				cable->done.data[m].arg.xferred.res = r;
				cable->done.data[m].arg.xferred.out = cable->todo.data[i].arg.transfer.out;
					
			};

			i++;
			if (i >= cable->todo.max_items) i=0;
			cable->todo.num_items --;
		};

		cable->todo.next_item = i;
	}
}

void
usbblaster_set_frequency( cable_t *cable, uint32_t new_frequency )
{
	cable->frequency = new_frequency;
}

void
usbblaster_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s ftdi VID:PID\n"
		"\n"
		"VID        vendor ID (hex, e.g. 9FB, or empty)\n"
		"PID        product ID (hex, e.g. 6001, or empty)\n"
		"\n"
	), cablename );
}

cable_driver_t usbblaster_cable_driver = {
	"UsbBlaster",
	N_("Altera USB-Blaster Cable"),
	generic_parport_connect,
	generic_disconnect,
	generic_parport_free,
	usbblaster_init,
	generic_parport_done,
	usbblaster_set_frequency,
	usbblaster_clock,
	usbblaster_get_tdo,
	usbblaster_transfer,
	usbblaster_set_trst,
	generic_get_trst,
//	generic_flush_one_by_one,
//	generic_flush_using_transfer,
	usbblaster_flush,
	usbblaster_help,
};
