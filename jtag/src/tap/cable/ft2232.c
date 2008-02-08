/*
 * $Id$
 *
 * Generic cable driver for FTDI's FT2232C chip in MPSSE mode.
 * Copyright (C) 2007 A. Laeuger
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
 * Written by Arnim Laeuger, 2007-2008.
 * Support for JTAGkey submitted by Laurent Gauch, 2008.
 *
 */

#include <stdlib.h>
#include <string.h>

#include "sysdep.h"

#include <cable.h>
#include <parport.h>
#include <chain.h>
#include <cmd.h>

#include "generic.h"


/* Maximum chunk to write to parport driver.
   Larger values might speed up comm, but there's an upper limit
   when too many bytes are sent and the underlying libftdi or libftd2xx
   don't fetch the returned data in time -> deadlock */
#define MAXRECV_FTD2XX (63 * 64)
#define MAXRECV_FTDI   ( 4 * 64)

/* Maximum TCK frequency of FT2232 */
#define FT2232_MAX_TCK_FREQ 6000000


/* repeat the definitions for MPSSE command processor here
	 since we cannot rely on the existence of ftdih. even though
	 they're defined there */

/* Shifting commands IN MPSSE Mode*/
#define MPSSE_WRITE_NEG 0x01   /* Write TDI/DO on negative TCK/SK edge*/
#define MPSSE_BITMODE   0x02   /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04   /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08   /* LSB first */
#define MPSSE_DO_WRITE  0x10   /* Write TDI/DO */
#define MPSSE_DO_READ   0x20   /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40   /* Write TMS/CS */

/* FTDI MPSSE commands */
#define SET_BITS_LOW   0x80
/*BYTE DATA*/
/*BYTE Direction*/
#define SET_BITS_HIGH  0x82
/*BYTE DATA*/
/*BYTE Direction*/
#define GET_BITS_LOW   0x81
#define GET_BITS_HIGH  0x83
#define LOOPBACK_START 0x84
#define LOOPBACK_END   0x85
#define TCK_DIVISOR    0x86


/* bit and bitmask definitions for GPIO commands */
#define BIT_TCK 0
#define BIT_TDI 1
#define BIT_TDO 2
#define BIT_TMS 3
#define BIT_JTAGKEY_nOE 4
#define BIT_ARMUSBOCD_nOE 4
#define BITMASK_TDO (1 << BIT_TDO)
#define BITMASK_TDI (1 << BIT_TDI)
#define BITMASK_TCK (1 << BIT_TCK)
#define BITMASK_TMS (1 << BIT_TMS)
#define BITMASK_JTAGKEY_nOE (1 << BIT_JTAGKEY_nOE)
#define BITMASK_ARMUSBOCD_nOE (1 << BIT_ARMUSBOCD_nOE)


typedef struct {
	uint32_t mpsse_frequency;
	/* variables to save last TDO value
	   this acts as a cache to prevent multiple "Read Data Bits Low" transfer
	   over USB for ft2232_get_tdo */
	unsigned int last_tdo_valid;
	unsigned int last_tdo;

	/* queue buffers */
	uint32_t send_buffer_len;
	uint32_t to_send;
	uint16_t *send_buffer;
	uint32_t recv_buffer_len;
	uint32_t to_recv;
	uint32_t recv_idx;
	uint8_t *recv_buffer;
	uint16_t maxrecv;
} params_t;


static int
extend_send_buffer( params_t *params )
{
	/* check size of send_buffer and increase it if not sufficient */
	if (params->to_send >= params->send_buffer_len) {
		params->send_buffer_len *= 2;
		params->send_buffer = (uint16_t *)realloc( params->send_buffer, params->send_buffer_len * sizeof(uint16_t) );
	}

	return params->send_buffer ? 1 : 0;
}


static void
push_recv_cmd( params_t *params, uint16_t num_recv )
{
	extend_send_buffer( params );

	/* set MSB to flag this as a header of a receive command */
	params->send_buffer[params->to_send++] = (1 << 15) | num_recv;
	params->to_recv += num_recv;
}


static void
push_to_send( params_t *params, uint8_t d )
{
	extend_send_buffer( params );

	params->send_buffer[params->to_send++] = d;
}


static void
send_and_receive( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;
	uint32_t bytes_sent, bytes_to_recv, bytes_recvd;
	uint16_t *send_idx;
	uint8_t *recv_idx;

	/* TODO: move down before receive routine */
	if (params->to_recv > params->recv_buffer_len) {
		free( params->recv_buffer );
		params->recv_buffer = (uint8_t *)malloc( params->to_recv );
	}

	send_idx = params->send_buffer;
	bytes_sent = 0;
	recv_idx = params->recv_buffer;
	bytes_recvd = 0;
	bytes_to_recv = 0;

	while (bytes_sent < params->to_send) {

		/* Step 1: send scheduled bytes through the parport driver */
		while (bytes_sent < params->to_send) {
			/* check for receive command header */
			if (*send_idx & 0x8000) {
				uint16_t num_recv = *send_idx & 0x7fff;

				if (bytes_to_recv + num_recv > params->maxrecv) {
					/* suspend sending since we can't handle the receive data
					   of this command */
					break;
				}

				/* eat up entry */
				send_idx++;
				bytes_sent++;

				bytes_to_recv += num_recv;
			}

			parport_set_data( p, *send_idx & 0xff );
			send_idx++;
			bytes_sent++;
		}

		/* Step 2: flush parport */
		parport_set_control( p, 1 ); // flush
		parport_set_control( p, 0 ); // noflush

		/* Step 3: receive answers */
		while (bytes_to_recv) {
			*recv_idx = parport_get_data( p );
			recv_idx++;
			bytes_to_recv--;
			bytes_recvd++;
		}
	}

	params->recv_idx = 0;
	params->to_recv  = bytes_recvd;
	params->to_send  = 0;
}


static uint8_t
pop_to_recv( params_t *params )
{
	if (params->to_recv == 0) {
		printf( _("Error: Receive buffer underrun %s line %d\n"), __FILE__, __LINE__ );
		return 0;
	}

	params->to_recv--;
	return params->recv_buffer[params->recv_idx++];
}


static void
update_frequency( cable_t *cable )
{
	uint32_t new_frequency = cable_get_frequency( cable );
	params_t *params = (params_t *)cable->params;

	if (!new_frequency || new_frequency > FT2232_MAX_TCK_FREQ)
		new_frequency = FT2232_MAX_TCK_FREQ;

	/* update ft2232 frequency if cable setting changed */
	if (new_frequency != params->mpsse_frequency) {
		uint32_t div;

		div = FT2232_MAX_TCK_FREQ / new_frequency;
		if (FT2232_MAX_TCK_FREQ % new_frequency)
			div++;

		if (div >= (1 << 16)) {
			div = (1 << 16) - 1;
			printf( _("Warning: Setting lowest supported frequency for FT2232: %d\n"), FT2232_MAX_TCK_FREQ/div );
		}

		/* send new divisor to device */
		div -= 1;
		push_to_send( params, TCK_DIVISOR );
		push_to_send( params, div & 0xff );
		push_to_send( params, (div >> 8) & 0xff );

		send_and_receive( cable );

		params->mpsse_frequency = FT2232_MAX_TCK_FREQ / (div + 1);
	}
}


static int
ft2232_generic_init( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;

	if (parport_open( p ))
		return -1;

	/* Set Data Bits Low Byte
		 TCK = 0, TMS = 1, TDI = 0 */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_TMS );
	push_to_send( params, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS );
	send_and_receive( cable );

	/* Set TCK/SK Divisor */
	push_to_send( params, TCK_DIVISOR );
	push_to_send( params, 0 );
	push_to_send( params, 0 );
	send_and_receive( cable );

	params->mpsse_frequency = FT2232_MAX_TCK_FREQ;

	params->last_tdo_valid = 0;

	return 0;
}

static int
ft2232_jtagkey_init( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;

	if (parport_open( p ))
		return -1;

	/* set loopback off */
	push_to_send( params, LOOPBACK_END );
	send_and_receive( cable );

	/* Set Data Bits Low Byte
		 TCK = 0, TMS = 1, TDI = 0, nOE = 0 */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_TMS );
	push_to_send( params, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS | BITMASK_JTAGKEY_nOE );
	send_and_receive( cable );

	/* Set TCK/SK Divisor */
	push_to_send( params, TCK_DIVISOR );
	push_to_send( params, 0 );
	push_to_send( params, 0 );
	send_and_receive( cable );

	params->mpsse_frequency = FT2232_MAX_TCK_FREQ;

	params->last_tdo_valid = 0;

	return 0;
}


static int
ft2232_armusbocd_init( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;

	if (parport_open( p ))
		return -1;

	/* set loopback off */
	push_to_send( params, LOOPBACK_END );
	send_and_receive( cable );

	/* Set Data Bits Low Byte
		 TCK = 0, TMS = 1, TDI = 0, nOE = 0 */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_TMS );
	push_to_send( params, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS | BITMASK_ARMUSBOCD_nOE );
	send_and_receive( cable );

	/* Set TCK/SK Divisor */
	push_to_send( params, TCK_DIVISOR );
	push_to_send( params, 0 );
	push_to_send( params, 0 );
	send_and_receive( cable );

	params->mpsse_frequency = FT2232_MAX_TCK_FREQ;

	params->last_tdo_valid = 0;

	return 0;
}


static void
ft2232_generic_done( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;

	/* Set Data Bits Low Byte
		 set all to input */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, 0 );
	push_to_send( params, 0 );
	send_and_receive( cable );

	parport_close( p );
}


static void
ft2232_jtagkey_done( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;

	/* Set Data Bits Low Byte
		 disable output drivers */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_JTAGKEY_nOE );
	push_to_send( params, BITMASK_JTAGKEY_nOE );
	send_and_receive( cable );
	/* Set Data Bits Low Byte
		 set all to input */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_JTAGKEY_nOE );
	push_to_send( params, 0 );
	send_and_receive( cable );

	parport_close( p );
}


static void
ft2232_armusbocd_done( cable_t *cable )
{
	parport_t *p = cable->port;
	params_t *params = (params_t *)cable->params;

	/* Set Data Bits Low Byte
		 disable output drivers */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_ARMUSBOCD_nOE );
	push_to_send( params, BITMASK_ARMUSBOCD_nOE );
	send_and_receive( cable );
	/* Set Data Bits Low Byte
		 set all to input */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, BITMASK_ARMUSBOCD_nOE );
	push_to_send( params, 0 );
	send_and_receive( cable );

	parport_close( p );
}


static void
ft2232_clock_defer( cable_t *cable, int defer, int tms, int tdi, int n )
{
	params_t *params = (params_t *)cable->params;

	tms = tms ? 0x7f : 0;
	tdi = tdi ? 1 << 7 : 0;

	/* check for new frequency setting */
	update_frequency( cable );

	while (n > 0) {
		/* Clock Data to TMS/CS Pin (no Read) */
		push_to_send( params, MPSSE_WRITE_TMS |
		              MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG );
		if (n <= 7) {
			push_to_send( params, n-1 );
			n = 0;
		} else {
			push_to_send( params, 7-1 );
			n -= 7;
		}
		push_to_send( params, tdi | tms );
	}
	if (!defer) {
		send_and_receive( cable );

		params->last_tdo_valid = 0;
	}
}


static void
ft2232_clock( cable_t *cable, int tms, int tdi, int n )
{
	ft2232_clock_defer( cable, 0, tms, tdi, n );
}


static void
ft2232_get_tdo_schedule( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;

	/* Read Data Bits Low Byte */
	push_recv_cmd( params, 1 );
	push_to_send( params, GET_BITS_LOW );
}


static int
ft2232_get_tdo_finish( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;
	int value;

	value = ( pop_to_recv( params ) & BITMASK_TDO) ? 1 : 0;

	params->last_tdo = value;
	params->last_tdo_valid = 1;

	return value;
}


static int
ft2232_get_tdo( cable_t *cable )
{
	ft2232_get_tdo_schedule( cable );
	send_and_receive( cable );
	return ft2232_get_tdo_finish( cable );
}


static int
ft2232_set_trst( cable_t *cable, int trst )
{
	return 1;
}


static void
ft2232_transfer_schedule( cable_t *cable, int len, char *in, char *out )
{
	params_t *params = (params_t *)cable->params;
	int in_offset = 0;
	int bitwise_len;
	int chunkbytes;

	/* Set Data Bits Low Byte to lower TMS for transfer
		 TCK = 0, TMS = 0, TDI = 0, nOE = 0 */
	push_to_send( params, SET_BITS_LOW );
	push_to_send( params, 0 );
	push_to_send( params, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS | BITMASK_ARMUSBOCD_nOE );


	chunkbytes = len >> 3;
	while (chunkbytes > 0) {
		int byte_idx;

		/* reduce chunkbytes to the maximum amount we can receive in one step */
		if (out && chunkbytes > params->maxrecv)
			chunkbytes = params->maxrecv;
		/* restrict chunkbytes to the maximum amount that can be transferred
		   for one single operation */
		if (chunkbytes > (1 << 16))
			chunkbytes = 1 << 16;

		/***********************************************************************
		 * Step 1:
		 * Determine data shifting command (bytewise).
		 * Either with or without read
		 ***********************************************************************/
		if (out) {
			push_recv_cmd( params, chunkbytes );
			/* Clock Data Bytes In and Out LSB First
			   out on negative edge, in on positive edge */
			push_to_send( params, MPSSE_DO_READ | MPSSE_DO_WRITE |
			              MPSSE_LSB | MPSSE_WRITE_NEG );
		} else
			/* Clock Data Bytes Out on -ve Clock Edge LSB First (no Read) */
			push_to_send( params, MPSSE_DO_WRITE |
			              MPSSE_LSB | MPSSE_WRITE_NEG );
		/* set byte count */
		push_to_send( params, (chunkbytes - 1) & 0xff );
		push_to_send( params, ((chunkbytes - 1) >> 8) & 0xff );


		/*********************************************************************
		 * Step 2:
		 * Write TDI data in bundles of 8 bits.
		 *********************************************************************/
		for (byte_idx = 0; byte_idx < chunkbytes; byte_idx++) {
			int bit_idx;
			unsigned char b = 0;

			for (bit_idx = 1; bit_idx < 256; bit_idx <<= 1)
				if (in[in_offset++])
					b |= bit_idx;
			push_to_send( params, b );
		}

		/* recalc chunkbytes for next round */
		chunkbytes = (len - in_offset) >> 3;
	}

	/* determine bitwise shift amount */
	bitwise_len = (len - in_offset) % 8;
	if (bitwise_len > 0) {
		/***********************************************************************
		 * Step 3:
		 * Determine data shifting command (bitwise).
		 * Either with or without read
		 ***********************************************************************/
		if (out) {
			push_recv_cmd( params, 1 );
			/* Clock Data Bytes In and Out LSB First
			   out on negative edge, in on positive edge */
			push_to_send( params, MPSSE_DO_READ | MPSSE_DO_WRITE |
			              MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG );
		} else
			/* Clock Data Bytes Out on -ve Clock Edge LSB First (no Read) */
			push_to_send( params, MPSSE_DO_WRITE |
			              MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG );
		/* determine bit count */
		push_to_send( params, bitwise_len - 1 );

		/***********************************************************************
		 * Step 4:
		 * Write TDI data bitwise
		 ***********************************************************************/
		{
			int bit_idx;
			unsigned char b = 0;
			for (bit_idx = 1; bit_idx <= 1 << bitwise_len; bit_idx <<= 1) {
				if (in[in_offset++])
					b |= bit_idx;
			}
			push_to_send( params, b );
		}
	}

	if (out) {
		/* Read Data Bits Low Byte to get current TDO,
		   Do this only if we'll read out data nonetheless */
		push_recv_cmd( params, 1 );
		push_to_send( params, GET_BITS_LOW );
		params->last_tdo_valid = 1;
	} else
		params->last_tdo_valid = 0;
}


static int
ft2232_transfer_finish( cable_t *cable, int len, char *out )
{
	params_t *params = (params_t *)cable->params;
	int bitwise_len;
	int chunkbytes;
	int out_offset = 0;

	chunkbytes = len >> 3;
	bitwise_len = len % 8;

	if (out) {
		if (chunkbytes > 0) {
			uint32_t xferred;

			/*********************************************************************
			 * Step 5:
			 * Read TDO data in bundles of 8 bits if read is requested.
			 *********************************************************************/
			xferred = chunkbytes;
			for (; xferred > 0; xferred--) {
				int bit_idx;
				unsigned char b;

				b = pop_to_recv( params );
				for (bit_idx = 1; bit_idx < 256; bit_idx <<= 1)
					out[out_offset++] = (b & bit_idx) ? 1 : 0;
			}
		}

		if (bitwise_len > 0) {
			/***********************************************************************
			 * Step 6:
			 * Read TDO data bitwise if read is requested.
			 ***********************************************************************/
			int bit_idx;
			unsigned char b;

			b = pop_to_recv( params );

				for (bit_idx = (1 << (8 - bitwise_len)); bit_idx < 256; bit_idx <<= 1)
					out[out_offset++] = (b & bit_idx) ? 1 : 0;
		}

		/* gather current TDO */
		params->last_tdo = ( pop_to_recv( params ) & BITMASK_TDO) ? 1 : 0;
		params->last_tdo_valid = 1;
	} else
		params->last_tdo_valid = 0;

	return 0;
}


static int
ft2232_transfer( cable_t *cable, int len, char *in, char *out )
{
	/* check for new frequency setting */
	update_frequency( cable );
	ft2232_transfer_schedule( cable, len, in, out );
	send_and_receive( cable );
	return ft2232_transfer_finish( cable, len, out );
}


static void
ft2232_flush( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;

	while (cable->todo.num_items > 0)
	{
		int i, j, n;
		int last_tdo_valid_schedule = params->last_tdo_valid;
		int last_tdo_valid_finish = params->last_tdo_valid;

		for (j = i = cable->todo.next_item, n = 0; n < cable->todo.num_items; n++) {

			switch (cable->todo.data[i].action) {
				case CABLE_CLOCK:
					ft2232_clock_defer( cable, 1,
					                    cable->todo.data[i].arg.clock.tms,
					                    cable->todo.data[i].arg.clock.tdi,
					                    cable->todo.data[i].arg.clock.n );
					last_tdo_valid_schedule = 0;
					break;

				case CABLE_GET_TDO:
					if (!last_tdo_valid_schedule) {
						ft2232_get_tdo_schedule( cable );
						last_tdo_valid_schedule = 1;
					}
					break;

				case CABLE_TRANSFER:
					ft2232_transfer_schedule( cable,
					                          cable->todo.data[i].arg.transfer.len,
					                          cable->todo.data[i].arg.transfer.in,
					                          cable->todo.data[i].arg.transfer.out );
					last_tdo_valid_schedule = params->last_tdo_valid;
					break;

				default:
					break;
			}

			i++;
			if (i >= cable->todo.max_items)
				i = 0;
		}

		send_and_receive( cable );

		while (j != i) {
			switch (cable->todo.data[j].action) {
				case CABLE_CLOCK:
					last_tdo_valid_finish = 0;
					break;
				case CABLE_GET_TDO:
				{
					int tdo;
					int m;
					if (last_tdo_valid_finish)
						tdo = params->last_tdo;
					else
						tdo = ft2232_get_tdo_finish( cable );
					last_tdo_valid_finish = 1;
					m = cable_add_queue_item( cable, &(cable->done) );
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
				case CABLE_TRANSFER:
				{
					int  r = ft2232_transfer_finish( cable,
					                                 cable->todo.data[j].arg.transfer.len,
					                                 cable->todo.data[j].arg.transfer.out );
					last_tdo_valid_finish = params->last_tdo_valid;
					free( cable->todo.data[j].arg.transfer.in );
					if (cable->todo.data[j].arg.transfer.out) {
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


static int
ft2232_connect( char *params[], cable_t *cable )
{
	params_t *cable_params;
	parport_t *port;
	int i;
	uint16_t maxrecv;

	if ( cmd_params( params ) < 3 ) {
	  puts( _("not enough arguments!") );
	  return 1;
	}

	if (strcasecmp( params[1], "ftdi-mpsse" ) == 0) {
		maxrecv = MAXRECV_FTDI;
	} else if	(strcasecmp( params[1], "ftd2xx-mpsse" ) == 0) {
		maxrecv = MAXRECV_FTD2XX;
	} else {
		puts( _("Wrong parport driver selected!") );
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

	cable_params = (params_t *)malloc( sizeof(params_t) );
	if (!cable_params) {
		free( cable );
		return 4;
	}

	cable_params->last_tdo_valid  = 0;
	cable_params->send_buffer_len = 1024;
	cable_params->to_send         = 0;
	cable_params->send_buffer     = (uint16_t *)malloc( cable_params->send_buffer_len * sizeof(uint16_t) );
	cable_params->recv_buffer_len = 1024;
	cable_params->to_recv         = 0;
	cable_params->recv_idx        = 0;
	cable_params->recv_buffer     = (uint8_t *)malloc( cable_params->recv_buffer_len );
	cable_params->maxrecv         = maxrecv;

	cable->port = port;
	cable->params = cable_params;
	cable->chain = NULL;

	return 0;
}


static void
ft2232_cable_free( cable_t *cable )
{
	params_t *params = (params_t *)cable->params;

	cable->port->driver->parport_free( cable->port );
	free( params->send_buffer );
	free( params->recv_buffer );
	free( cable->params );
	free( cable );
}


static void
ft2232_usbcable_help( const char *cablename )
{
	printf( _(
#ifdef HAVE_LIBFTDI
		"Usage: cable %s ftdi-mpsse VID:PID\n"
#endif
#ifdef HAVE_LIBFTD2XX
		"Usage: cable %s ftd2xx-mpsse VID:PID\n"
#endif
		"\n"
		"VID        vendor ID (hex, e.g. 9FB, or empty)\n"
		"PID        product ID (hex, e.g. 6001, or empty)\n"
    "\n"
#ifdef HAVE_LIBFTDI
		"Expect suboptimal performance of ftdi-mpsse for large shifts with output capture.\n"
#endif
		"\n"
	),
#ifdef HAVE_LIBFTDI
		 cablename
#endif
#if defined HAVE_LIBFTDI && defined HAVE_LIBFTD2XX
		 ,
#endif
#ifdef HAVE_LIBFTD2XX
		 cablename
#endif
		);
}


cable_driver_t ft2232_cable_driver = {
	"FT2232",
	N_("Generic FTDI FT2232 Cable"),
	ft2232_connect,
	generic_disconnect,
	ft2232_cable_free,
	ft2232_generic_init,
	ft2232_generic_done,
	ft2232_clock,
	ft2232_get_tdo,
	ft2232_transfer,
	ft2232_set_trst,
	generic_get_trst,
	ft2232_flush,
	ft2232_usbcable_help
};

cable_driver_t ft2232_armusbocd_cable_driver = {
	"ARM-USB-OCD",
	N_("Olimex ARM-USB-OCD (FT2232) Cable"),
	ft2232_connect,
	generic_disconnect,
	ft2232_cable_free,
	ft2232_armusbocd_init,
	ft2232_armusbocd_done,
	ft2232_clock,
	ft2232_get_tdo,
	ft2232_transfer,
	ft2232_set_trst,
	generic_get_trst,
	ft2232_flush,
	ft2232_usbcable_help
};

cable_driver_t ft2232_jtagkey_cable_driver = {
	"JTAGkey",
	N_("Amontec JTAGkey (FT2232) Cable"),
	ft2232_connect,
	generic_disconnect,
	ft2232_cable_free,
	ft2232_jtagkey_init,
	ft2232_jtagkey_done,
	ft2232_clock,
	ft2232_get_tdo,
	ft2232_transfer,
	ft2232_set_trst,
	generic_get_trst,
	ft2232_flush,
	ft2232_usbcable_help
};



/*
 Local Variables:
 mode:C
 tab-width:2
 indent-tabs-mode:t
 End:
*/
