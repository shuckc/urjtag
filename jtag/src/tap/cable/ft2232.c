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
 * Written by Arnim Laeuger, 2007.
 * Support for JTAGkey submitted by Laurent Gauch, 2008.
 *
 */

#include <stdlib.h>

#include "sysdep.h"

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"


/* Define MULTI_BYTE to activate block read transfer from parport
   no benefit over single byte read observed. */
#undef MULTI_BYTE

/* Maximum chunk to write to parport driver.
   Larger values might speed up comm, but there's an upper limit
   when too many bytes are sent and libftdi doesn't fetch the
   returned data in time -> deadlock */
#define MAXCHUNK (4 * 64)

/* Enable caching of last TDO to speed up things a bit.
	 Should be left define'd unless comm/sync problems occur. */
#define LAST_TDO_CACHE

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


#ifdef LAST_TDO_CACHE
/* global variables to save last TDO value
	 this acts as a cache to prevent multiple "Read Data Bits Low" transfer
	 over USB for ft2232_get_tdo */
static unsigned int last_tdo_valid;
static unsigned int last_tdo;
#endif

static uint32_t mpsse_frequency;

#ifdef MULTIBYTE
static uint8_t local_buffer[MAXCHUNK+16];
#endif


static void
update_frequency( cable_t *cable )
{
	parport_t *p = cable->port;
	uint32_t new_frequency = cable_get_frequency( cable );

	if (!new_frequency || new_frequency > FT2232_MAX_TCK_FREQ)
		new_frequency = FT2232_MAX_TCK_FREQ;

	/* update ft2232 frequency if cable setting changed */
	if (new_frequency != mpsse_frequency) {
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
		parport_set_data( p, TCK_DIVISOR );
		parport_set_data( p, div & 0xff );
		parport_set_data( p, (div >> 8) & 0xff );
		parport_set_control( p, 1 ); // flush
		parport_set_control( p, 0 ); // noflush

		mpsse_frequency = FT2232_MAX_TCK_FREQ / (div + 1);
	}
}


static int
ft2232_generic_init( cable_t *cable )
{
	parport_t *p = cable->port;

	if (parport_open( p ))
		return -1;

	/* Set Data Bits Low Byte
		 TCK = 0, TMS = 1, TDI = 0 */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_TMS );
	parport_set_data( p, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	/* Set TCK/SK Divisor */
	parport_set_data( p, TCK_DIVISOR );
	parport_set_data( p, 0 );
	parport_set_data( p, 0 );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	mpsse_frequency = FT2232_MAX_TCK_FREQ;

#ifdef LAST_TDO_CACHE
	last_tdo_valid = 0;
#endif

	return 0;
}

static int
ft2232_jtagkey_init( cable_t *cable )
{
	parport_t *p = cable->port;

	if (parport_open( p ))
		return -1;

	/* set loopback off */
	parport_set_data( p, LOOPBACK_END );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	/* Set Data Bits Low Byte
		 TCK = 0, TMS = 1, TDI = 0, nOE = 0 */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_TMS );
	parport_set_data( p, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS | BITMASK_JTAGKEY_nOE );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	/* Set TCK/SK Divisor */
	parport_set_data( p, TCK_DIVISOR );
	parport_set_data( p, 0 );
	parport_set_data( p, 0 );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	mpsse_frequency = FT2232_MAX_TCK_FREQ;

#ifdef LAST_TDO_CACHE
	last_tdo_valid = 0;
#endif

	return 0;
}

static int
ft2232_armusbocd_init( cable_t *cable )
{
	parport_t *p = cable->port;

	if (parport_open( p ))
		return -1;

	/* set loopback off */
	parport_set_data( p, LOOPBACK_END );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	/* Set Data Bits Low Byte
		 TCK = 0, TMS = 1, TDI = 0, nOE = 0 */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_TMS );
	parport_set_data( p, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS | BITMASK_ARMUSBOCD_nOE );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	/* Set TCK/SK Divisor */
	parport_set_data( p, TCK_DIVISOR );
	parport_set_data( p, 0 );
	parport_set_data( p, 0 );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	mpsse_frequency = FT2232_MAX_TCK_FREQ;

#ifdef LAST_TDO_CACHE
	last_tdo_valid = 0;
#endif

	return 0;
}

static void
ft2232_generic_done( cable_t *cable )
{
	parport_t *p = cable->port;

	/* Set Data Bits Low Byte
		 set all to input */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, 0 );
	parport_set_data( p, 0 );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	parport_close( p );
}

static void
ft2232_jtagkey_done( cable_t *cable )
{
	parport_t *p = cable->port;

	/* Set Data Bits Low Byte
		 disable output drivers */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_JTAGKEY_nOE );
	parport_set_data( p, BITMASK_JTAGKEY_nOE );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush
	/* Set Data Bits Low Byte
		 set all to input */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_JTAGKEY_nOE );
	parport_set_data( p, 0 );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	parport_close( p );
}

static void
ft2232_armusbocd_done( cable_t *cable )
{
	parport_t *p = cable->port;

	/* Set Data Bits Low Byte
		 disable output drivers */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_ARMUSBOCD_nOE );
	parport_set_data( p, BITMASK_ARMUSBOCD_nOE );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush
	/* Set Data Bits Low Byte
		 set all to input */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, BITMASK_ARMUSBOCD_nOE );
	parport_set_data( p, 0 );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush

	parport_close( p );
}

static void
ft2232_clock_defer( cable_t *cable, int defer, int tms, int tdi, int n )
{
	parport_t *p = cable->port;

	tms = tms ? 0x7f : 0;
	tdi = tdi ? 1 << 7 : 0;

	/* check for new frequency setting */
	update_frequency( cable );

	while (n > 0) {
		/* Clock Data to TMS/CS Pin (no Read) */
		parport_set_data( p, MPSSE_WRITE_TMS |
											MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG );
		if (n <= 7) {
			parport_set_data( p, n-1 );
			n = 0;
		} else {
			parport_set_data( p, 7-1 );
			n -= 7;
		}
		parport_set_data( p, tdi | tms );
	}
	if (!defer) {
		parport_set_control( p, 1 ); // flush
		parport_set_control( p, 0 ); // noflush

#ifdef LAST_TDO_CACHE
		last_tdo_valid = 0;
#endif
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
	parport_t *p = cable->port;

	/* Read Data Bits Low Byte */
	parport_set_data( p, GET_BITS_LOW );
}

static int
ft2232_get_tdo_finish( cable_t *cable )
{
	parport_t *p = cable->port;
	int value;

	value = ( parport_get_data( p ) & BITMASK_TDO) ? 1 : 0;

#ifdef LAST_TDO_CACHE
	last_tdo = value;
	last_tdo_valid = 1;
#endif

	return value;
}

static int
ft2232_get_tdo( cable_t *cable )
{
	parport_t *p = cable->port;

	ft2232_get_tdo_schedule( cable );
	parport_set_control( p, 1 ); // flush
	parport_set_control( p, 0 ); // noflush
	return ft2232_get_tdo_finish( cable );
}

static int
ft2232_set_trst( cable_t *cable, int trst )
{
	return 1;
}

static int
ft2232_transfer( cable_t *cable, int len, char *in, char *out )
{
	parport_t *p = cable->port;
	int in_offset = 0;
	int out_offset = 0;
	int bitwise_len;

	/* check for new frequency setting */
	update_frequency( cable );

	/* Set Data Bits Low Byte to lower TMS for transfer
		 TCK = 0, TMS = 0, TDI = 0, nOE = 0 */
	parport_set_data( p, SET_BITS_LOW );
	parport_set_data( p, 0 );
	parport_set_data( p, BITMASK_TCK | BITMASK_TDI | BITMASK_TMS | BITMASK_ARMUSBOCD_nOE );

	while (len - in_offset > 0) {
		int byte_idx;
		int chunkbytes = (len - in_offset) >> 3;

		if (chunkbytes > MAXCHUNK)
			chunkbytes = MAXCHUNK;

		if ((chunkbytes < MAXCHUNK) &&
				((len - in_offset) % 8 > 0))
			bitwise_len = (len - in_offset) % 8;
		else
			bitwise_len = 0;


		if (chunkbytes > 0) {
			/***********************************************************************
			 * Step 1:
			 * Determine data shifting command (bytewise).
			 * Either with or without read
			 ***********************************************************************/
			if (out)
				/* Clock Data Bytes In and Out LSB First
					 out on negative edge, in on positive edge */
				parport_set_data( p, MPSSE_DO_READ | MPSSE_DO_WRITE |
													MPSSE_LSB | MPSSE_WRITE_NEG );
			else
				/* Clock Data Bytes Out on -ve Clock Edge LSB First (no Read) */
				parport_set_data( p, MPSSE_DO_WRITE |
													MPSSE_LSB | MPSSE_WRITE_NEG );
			/* set byte count */
			parport_set_data( p, (chunkbytes - 1) & 0xff );
			parport_set_data( p, ((chunkbytes - 1) >> 8) & 0xff );


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
				parport_set_data( p, b );
			}
		}

		if (bitwise_len > 0) {
			/***********************************************************************
			 * Step 3:
			 * Determine data shifting command (bitwise).
			 * Either with or without read
			 ***********************************************************************/
			if (out)
				/* Clock Data Bytes In and Out LSB First
					 out on negative edge, in on positive edge */
				parport_set_data( p, MPSSE_DO_READ | MPSSE_DO_WRITE |
													MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG );
			else
				/* Clock Data Bytes Out on -ve Clock Edge LSB First (no Read) */
				parport_set_data( p, MPSSE_DO_WRITE |
													MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG );
			/* determine bit count */
			parport_set_data( p, bitwise_len - 1 );

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
				parport_set_data( p, b );
			}
		}

#ifdef LAST_TDO_CACHE
		if (out) {
			/* Read Data Bits Low Byte to get current TDO,
			   Do this only if we'll read out data nonetheless */
			parport_set_data( p, GET_BITS_LOW );
		}
#endif

		parport_set_control( p, 1 ); // flush
		parport_set_control( p, 0 ); // noflush
#ifdef LAST_TDO_CACHE
	/* invalidate TDO cache */
	last_tdo_valid = 0;
#endif


		if (out) {
			if (chunkbytes > 0) {
#ifdef MULTI_BYTE
				int buf_idx;
#endif
				uint32_t xferred;

				/*********************************************************************
				 * Step 5:
				 * Read TDO data in bundles of 8 bits if read is requested.
				 *********************************************************************/
#ifdef MULTI_BYTE
				parport_get_block( p, local_buffer, chunkbytes, &xferred );
				if (chunkbytes != xferred)
					printf("Bummer\n");
				buf_idx = 0;
#else
				xferred = chunkbytes;
#endif
				for (; xferred > 0; xferred--) {
					int bit_idx;
					unsigned char b;

#ifdef MULTI_BYTE
					b = local_buffer[buf_idx++];
#else
					b = parport_get_data( p );
#endif
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

				b = parport_get_data( p );

				for (bit_idx = (1 << (8 - bitwise_len)); bit_idx < 256; bit_idx <<= 1)
					out[out_offset++] = (b & bit_idx) ? 1 : 0;
			}
		}

#ifdef LAST_TDO_CACHE
		if (out) {
			/* gather current TDO */
			last_tdo = ( parport_get_data( p ) & BITMASK_TDO) ? 1 : 0;
			last_tdo_valid = 1;
		}
#endif
	}

	return 0;
}

#undef FLUSH_PUTS

static void
ft2232_flush( cable_t *cable )
{
#ifdef FLUSH_PUTS
	puts("flush()");
#endif
	while (cable->todo.num_items > 0)
	{
		int i, j, n, to_send = 0, to_rec = 0;
#ifdef LAST_TDO_CACHE
		int last_tdo_valid_schedule = last_tdo_valid;
		int last_tdo_valid_finish = last_tdo_valid;
#endif

		for(j=i=cable->todo.next_item, n=0; to_send < 64 && n<cable->todo.num_items; n++)
		{
			if(cable->todo.data[i].action == CABLE_TRANSFER) {
#ifdef FLUSH_PUTS
				puts("transfer");
#endif
				break;
			}

			switch(cable->todo.data[i].action) {
				case CABLE_CLOCK:
#ifdef FLUSH_PUTS
					puts("clock");
#endif
					ft2232_clock_defer( cable, 1,
					                    cable->todo.data[i].arg.clock.tms,
					                    cable->todo.data[i].arg.clock.tdi,
					                    cable->todo.data[i].arg.clock.n );
					to_send += 3;
#ifdef LAST_TDO_CACHE
					last_tdo_valid_schedule = 0;
#endif
					break;

				case CABLE_GET_TDO:
#ifdef LAST_TDO_CACHE
					if (!last_tdo_valid_schedule) {
#else
					{
#endif
						ft2232_get_tdo_schedule( cable );
#ifdef FLUSH_PUTS
						puts("get_tdo");
#endif
						to_send += 1;
						to_rec  += 1;
#ifdef LAST_TDO_CACHE
						last_tdo_valid_schedule = 1;
#endif
					}
					break;

				default:
					break;
			}

			i++;
			if (i >= cable->todo.max_items) i=0;
		}

		if(to_rec > 0)
		{
#ifdef FLUSH_PUTS
			puts("flush");
#endif
			parport_set_control( cable->port, 1 ); // flush
			parport_set_control( cable->port, 0 );
		}

		while(j!=i)
		{
			switch(cable->todo.data[j].action)
			{
				case CABLE_CLOCK:
#ifdef LAST_TDO_CACHE
					last_tdo_valid_finish = 0;
#endif
					break;
				case CABLE_GET_TDO:
				{
					int tdo;
					int m;
#ifdef LAST_TDO_CACHE
					if (last_tdo_valid_finish) {
						tdo = last_tdo;
#ifdef FLUSH_PUTS
						puts("tdo cached");
#endif
					} else
						tdo = ft2232_get_tdo_finish( cable );
					last_tdo_valid_finish = 1;
#else
					tdo = ft2232_get_tdo_finish( cable );
#endif
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
				default:
					break;
			};

			j++;
			if (j >= cable->todo.max_items) j=0;
			cable->todo.num_items --;
		};

		while(cable->todo.num_items > 0 && cable->todo.data[i].action == CABLE_TRANSFER)
		{
			int r = ft2232_transfer( cable,
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
ft2232_usbcable_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s ftdi-mpsse VID:PID\n"
		"\n"
		"VID        vendor ID (hex, e.g. 9FB, or empty)\n"
		"PID        product ID (hex, e.g. 6001, or empty)\n"
		"\n"
	), cablename );
}

cable_driver_t ft2232_cable_driver = {
	"FT2232",
	N_("Generic FTDI FT2232 Cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
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
	generic_connect,
	generic_disconnect,
	generic_cable_free,
	ft2232_armusbocd_init,
	ft2232_armusbocd_done,
	ft2232_clock,
	ft2232_get_tdo,
	ft2232_transfer,
	ft2232_set_trst,
	generic_get_trst,
//	generic_flush_using_transfer,
	ft2232_flush,
	ft2232_usbcable_help
};

cable_driver_t ft2232_jtagkey_cable_driver = {
	"JTAGkey",
	N_("Amontec JTAGkey (FT2232) Cable"),
	generic_connect,
	generic_disconnect,
	generic_cable_free,
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
