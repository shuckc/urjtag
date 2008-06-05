/*
 * $Id$
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
#include <string.h>

#include "cable.h"
#include "chain.h"
#include "cmd.h"

#include "generic.h"
#include "generic_usbconn.h"

#include "usbconn.h"
#include "usbconn/libftdx.h"

#include "cmd_xfer.h"


#define TCK    0
#define TMS    1
#define TDI    4
#define READ   6
#define SHMODE 7
#define OTHERS ((1<<2)|(1<<3)|(1<<5))

#define TDO    0

/* The default driver if not specified otherwise during connect */
#ifdef ENABLE_LOWLEVEL_FTD2XX
#define DEFAULT_DRIVER "ftd2xx"
#else
#define DEFAULT_DRIVER "ftdi"
#endif

typedef struct {
	cx_cmd_root_t  cmd_root;
} params_t;

static int
usbblaster_connect( char *params[], cable_t *cable )
{
	params_t *cable_params;
	int result;

	/* perform generic_usbconn_connect */
	if ( ( result = generic_usbconn_connect( params, cable ) ) != 0 )
		return result;

	cable_params = (params_t *)malloc( sizeof(params_t) );
	if (!cable_params)
	{
		printf( _("%s(%d) malloc failed!\n"), __FILE__, __LINE__);
		/* NOTE:
		 * Call the underlying usbport driver (*free) routine directly
		 * not generic_usbconn_free() since it also free's cable->params
		 * (which is not established) and cable (which the caller will do)
		 */
		cable->link.usb->driver->free( cable->link.usb );
		return 4;
	}

	cx_cmd_init( &(cable_params->cmd_root) );

	/* exchange generic cable parameters with our private parameter set */
	free( cable->params );
	cable->params = cable_params;

	return 0;
}

static int
usbblaster_init( cable_t *cable )
{
	int i;
	params_t *params = (params_t *)cable->params;
	cx_cmd_root_t *cmd_root = &(params->cmd_root);

	if (usbconn_open( cable->link.usb )) return -1;

	cx_cmd_queue( cmd_root, 0 );
	for(i=0;i<64;i++)
		cx_cmd_push( cmd_root, 0 );

	cx_xfer( cmd_root, NULL, cable, COMPLETELY );

	return 0;
}

static void
usbblaster_cable_free( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;

	cx_cmd_deinit( &(params->cmd_root) );

	generic_usbconn_free( cable );
}

static void
usbblaster_clock_schedule( cable_t *cable, int tms, int tdi, int n )
{
	params_t *params = (params_t *)cable->params;
	cx_cmd_root_t *cmd_root = &(params->cmd_root);
	int i, m;

	tms = tms ? (1<<TMS) : 0;
	tdi = tdi ? (1<<TDI) : 0;

	// printf("clock: %d %d %d\n", tms, tdi, n);

	m = n;
	if (tms == 0 && m >= 8)
	{
		unsigned char tdis = tdi ? 0xFF : 0;

		cx_cmd_queue( cmd_root, 0 );
		while (m >= 8)
		{
			int i;
			int chunkbytes = (m >> 3);
			if(chunkbytes > 63) chunkbytes = 63;

			cx_cmd_push( cmd_root, (1<<SHMODE)|(0<<READ)|chunkbytes );

			for (i=0; i<chunkbytes; i++)
			{
				cx_cmd_push( cmd_root, tdis );
			}

			m -= (chunkbytes << 3);
		}
	}

	for (i = 0; i < m; i++) {
		cx_cmd_queue( cmd_root, 0 );
		cx_cmd_push( cmd_root, OTHERS | (0 << TCK) | tms | tdi );
		cx_cmd_push( cmd_root, OTHERS | (1 << TCK) | tms | tdi );
	}
}

static void
usbblaster_clock( cable_t *cable, int tms, int tdi, int n )
{
	params_t *params = (params_t *)cable->params;

	usbblaster_clock_schedule( cable, tms, tdi, n );
	cx_xfer( &(params->cmd_root), NULL, cable, COMPLETELY );
}

static void
usbblaster_get_tdo_schedule( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;
	cx_cmd_root_t *cmd_root = &(params->cmd_root);

	cx_cmd_queue( cmd_root, 1 );
	cx_cmd_push( cmd_root, OTHERS ); /* TCK low */
	cx_cmd_push( cmd_root, OTHERS | (1 << READ) ); /* TCK low */
}

static int
usbblaster_get_tdo_finish( cable_t *cable )
{
#if 0
	char x = ( cx_xfer_recv( cable ) & (1 << TDO)) ? 1 : 0;
	printf("GetTDO %d\n", x);
	return x;
#else
	return ( cx_xfer_recv( cable ) & (1 << TDO)) ? 1 : 0;
#endif
}

static int
usbblaster_get_tdo( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;

	usbblaster_get_tdo_schedule( cable );
	cx_xfer( &(params->cmd_root), NULL, cable, COMPLETELY );
	return usbblaster_get_tdo_finish( cable );
}

static int
usbblaster_set_trst( cable_t *cable, int trst )
{
	return 1;
}

static void
usbblaster_transfer_schedule( cable_t *cable, int len, char *in, char *out )
{
	params_t *params = (params_t *)cable->params;
	cx_cmd_root_t *cmd_root = &(params->cmd_root);
	int in_offset = 0;

	cx_cmd_queue( cmd_root, 0 );
	cx_cmd_push( cmd_root, OTHERS ); /* TCK low */

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
		{
			cx_cmd_queue( cmd_root, chunkbytes );
			cx_cmd_push( cmd_root, (1<<SHMODE)|(1<<READ)|chunkbytes );
		}
		else {
			cx_cmd_queue( cmd_root, 0 );
			cx_cmd_push( cmd_root, (1<<SHMODE)|(0<<READ)|chunkbytes );
		}

		for(i=0; i<chunkbytes; i++)
		{
			int j;
			unsigned char b = 0;
			for(j=1; j<256; j<<=1) if(in[in_offset++]) b |= j;
			cx_cmd_push( cmd_root, b );
		}
	}

	while(len > in_offset)
	{
		char tdi = in[in_offset++] ? 1 : 0;

		cx_cmd_queue( cmd_root, out ? 1 : 0 );
		cx_cmd_push( cmd_root, OTHERS | (tdi << TDI));/* TCK low */
		cx_cmd_push( cmd_root, OTHERS | ((out)?(1 << READ):0) | (1 << TCK)  | (tdi << TDI));
	}
}

static int
usbblaster_transfer_finish( cable_t *cable, int len, char *out )
{
	params_t *params = (params_t *)cable->params;
	cx_cmd_root_t *cmd_root = &(params->cmd_root);
	int out_offset = 0;

	if (out == NULL)
		return 0;

	while(len - out_offset >= 8)
	{
		int i;
		int chunkbytes = ((len-out_offset)>>3);
		if(chunkbytes > 63) chunkbytes = 63;

		if(out) 
		{
			cx_xfer( cmd_root, NULL, cable, COMPLETELY );

			for(i=0; i<chunkbytes; i++)
			{
				int j;
				unsigned char b = cx_xfer_recv( cable );
#if 0
                printf("read byte: %02X\n", b);
#endif
                 
				for(j=1; j<256; j<<=1) out[out_offset++] = (b & j) ? 1:0;
			}
		}
	}

	while(len > out_offset)
		out[out_offset++] = ( cx_xfer_recv( cable ) & (1 << TDO)) ? 1 : 0;

#if 0
	{
		int o;
		printf("%d out: ", len);
		for(o=0;o<len;o++) printf("%c", out[o]?'1':'0');
		printf("\n");
	}
#endif

	return 0;
}

static int
usbblaster_transfer( cable_t *cable, int len, char *in, char *out )
{
  params_t *params = (params_t *)cable->params;

	usbblaster_transfer_schedule( cable, len, in, out );
	cx_xfer( &(params->cmd_root), NULL, cable, COMPLETELY );
	return usbblaster_transfer_finish( cable, len, out );
}

static void
usbblaster_flush( cable_t *cable, cable_flush_amount_t how_much )
{
	params_t *params = (params_t *)cable->params;

	if (how_much == OPTIONALLY) return;

	if (cable->todo.num_items == 0)
		cx_xfer( &(params->cmd_root), NULL, cable, how_much );

	while (cable->todo.num_items > 0)
	{
		int i, j, n;

		for (j = i = cable->todo.next_item, n = 0; n < cable->todo.num_items; n++)
		{

			switch (cable->todo.data[i].action)
			{
			case CABLE_CLOCK:
				usbblaster_clock_schedule( cable,
				                           cable->todo.data[i].arg.clock.tms,
				                           cable->todo.data[i].arg.clock.tdi,
				                           cable->todo.data[i].arg.clock.n );
				break;

			case CABLE_GET_TDO:
				usbblaster_get_tdo_schedule( cable );
        break;

			case CABLE_TRANSFER:
				usbblaster_transfer_schedule( cable,
				                              cable->todo.data[i].arg.transfer.len,
				                              cable->todo.data[i].arg.transfer.in,
				                              cable->todo.data[i].arg.transfer.out );
				break;

			default:
				break;
			}

			i++;
			if (i >= cable->todo.max_items)
				i = 0;
		}

		cx_xfer( &(params->cmd_root), NULL, cable, how_much );

		while (j != i)
		{
			switch (cable->todo.data[j].action)
			{
			case CABLE_GET_TDO:
				{
					int m;
					m = cable_add_queue_item( cable, &(cable->done) );
					cable->done.data[m].action = CABLE_GET_TDO;
					cable->done.data[m].arg.value.tdo = usbblaster_get_tdo_finish( cable );
					break;
				}
			case CABLE_GET_TRST:
				{
					int m = cable_add_queue_item( cable, &(cable->done) );
					cable->done.data[m].action = CABLE_GET_TRST;
					cable->done.data[m].arg.value.trst = 1;
					break;
        }
			case CABLE_TRANSFER:
				{
					int  r = usbblaster_transfer_finish( cable,
					                                     cable->todo.data[j].arg.transfer.len,
					                                     cable->todo.data[j].arg.transfer.out );
					free( cable->todo.data[j].arg.transfer.in );
					if (cable->todo.data[j].arg.transfer.out)
					{
						int m = cable_add_queue_item( cable, &(cable->done) );
						if (m < 0)
							printf("out of memory!\n");
						cable->done.data[m].action = CABLE_TRANSFER;
						cable->done.data[m].arg.xferred.len = cable->todo.data[j].arg.transfer.len;
						cable->done.data[m].arg.xferred.res = r;
						cable->done.data[m].arg.xferred.out = cable->todo.data[j].arg.transfer.out;
					}
				}
			default:
				break;
			}

			j++;
			if (j >= cable->todo.max_items)
				j = 0;
			cable->todo.num_items--;
		}

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
		"Usage: cable %s [vid=VID] [pid=PID] [desc=DESC] [driver=DRIVER]\n"
		"\n"
		"VID        vendor ID (hex, e.g. 0abc)\n"
		"PID        product ID (hex, e.g. 0abc)\n"
		"DESC       Some string to match in description or serial no.\n"
		"DRIVER     usbconn driver, either ftdi or ftd2xx\n"
		"           defaults to %s if not specified\n"
		"\n"
		),
		cablename,
		DEFAULT_DRIVER
		);
}

cable_driver_t usbblaster_cable_driver = {
	"UsbBlaster",
	N_("Altera USB-Blaster Cable"),
	usbblaster_connect,
	generic_disconnect,
	usbblaster_cable_free,
	usbblaster_init,
	generic_usbconn_done,
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
usbconn_cable_t usbconn_cable_usbblaster_ftdi = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftdi",             /* default usbconn driver */
  0x09FB,             /* VID */
  0x6001              /* PID */
};
usbconn_cable_t usbconn_cable_cubic_cyclonium_ftdi = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftdi",             /* default usbconn driver */
  0x09FB,             /* VID */
  0x6002              /* PID */
};
usbconn_cable_t usbconn_cable_nios_eval_ftdi = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftdi",             /* default usbconn driver */
  0x09FB,             /* VID */
  0x6003              /* PID */
};
usbconn_cable_t usbconn_cable_usb_jtag_ftdi = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftdi",             /* default usbconn driver */
  0x16C0,             /* VID */
  0x06AD              /* PID */
};
usbconn_cable_t usbconn_cable_usbblaster_ftd2xx = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftd2xx",           /* default usbconn driver */
  0x09FB,             /* VID */
  0x6001              /* PID */
};
usbconn_cable_t usbconn_cable_cubic_cyclonium_ftd2xx = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftd2xx",           /* default usbconn driver */
  0x09FB,             /* VID */
  0x6002              /* PID */
};
usbconn_cable_t usbconn_cable_nios_eval_ftd2xx = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftdi",             /* default usbconn driver */
  0x09FB,             /* VID */
  0x6003              /* PID */
};
usbconn_cable_t usbconn_cable_usb_jtag_ftd2xx = {
  "UsbBlaster",       /* cable name */
  NULL,               /* string pattern, not used */
  "ftd2xx",           /* default usbconn driver */
  0x16C0,             /* VID */
  0x06AD              /* PID */
};
