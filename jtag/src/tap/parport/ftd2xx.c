/*
 * $Id$
 *
 * libftd2xx Driver
 *
 * Based on libftdi driver
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
 * Written by Kolja Waschk, 2006.
 * Structure taken from ppdev.c, written by Marcel Telka, 2003.
 * Ported to libftd2xx by A. Laeuger, 2007.
 *
 */

#include "sysdep.h"

#include <fcntl.h>
#include <unistd.h>
#if __CYGWIN__ || __MINGW32__
#include <windows.h>
#endif
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <ftd2xx.h>

#include "parport.h"
#include "cable.h"

parport_driver_t ftd2xx_parport_driver;
parport_driver_t ftd2xx_mpsse_parport_driver;

typedef struct port_node_t port_node_t;

struct port_node_t {
	parport_t *port;
	port_node_t *next;
};

static port_node_t *ports = NULL;		/* devices */

#define OUTBUF_LEN_STD 64
#define OUTBUF_LEN_MPSSE 4096

typedef struct {
	char *serial;
	unsigned int vendor_id;
	unsigned int product_id;
	char autoflush;
	FT_HANDLE fc;
	int outcount;
	unsigned char *outbuf;
	int outbuf_len;
} ftd2xx_params_t;

static int ftd2xx_set_data ( parport_t *parport, uint8_t data );
static int ftd2xx_set_control ( parport_t *parport, uint8_t data );
static int ftd2xx_flush_output ( ftd2xx_params_t *p );

static parport_t *
ftd2xx_parport_alloc( const char *vidpid, parport_driver_t * parport_driver, size_t outbuf_len )
{
	ftd2xx_params_t *params = malloc( sizeof *params );
	parport_t *parport = malloc( sizeof *parport );
	port_node_t *node = malloc( sizeof *node );
	unsigned char *outbuf = malloc( outbuf_len );

	if (!node || !parport || !params || !outbuf)  {
		free( node );
		free( parport );
		free( params );
		return NULL;
	}

	params->outbuf = outbuf;
	params->outbuf_len = outbuf_len;
	params->outcount = 0;
	params->autoflush = 0;
	params->product_id = 0;
	params->vendor_id = 0;
	params->serial = NULL;

	{
		char *f = strchr(vidpid, ':');
		char *l = strrchr(vidpid, ':');
		if(f)
		{
			params->vendor_id = strtoul(vidpid, NULL, 16);
			params->product_id = strtoul(f+1, NULL, 16);
			if(l!=f) params->serial = strdup(l+1);
		};
	};

	parport->params = params;
	parport->driver = parport_driver;
	parport->cable = NULL;

	node->port = parport;
	node->next = ports;

	ports = node;

	return parport;
}

static void
ftd2xx_parport_free( parport_t *port )
{
	port_node_t **prev;

	for (prev = &ports; *prev; prev = &((*prev)->next))
		if ((*prev)->port == port)
			break;

	if (*prev) {
		port_node_t *pn = *prev;
		*prev = pn->next;
		free( pn );
	}

	free( ((ftd2xx_params_t *) port->params)->serial );
	free( ((ftd2xx_params_t *) port->params)->outbuf );
	free( port->params );
	free( port );
}


int
ftd2xx_pre_connect( const char **par, int parnum )
{
	port_node_t *pn;

	if (parnum != 1) {
		printf( _("Syntax error!\n") );
		return 0;
	}

	for (pn = ports; pn; pn = pn->next)
		if (strcmp( pn->port->params, par[0] ) == 0) {
			printf( _("Disconnecting %s from FTDI device %s\n"), _(pn->port->cable->driver->description), par[0] );
			pn->port->cable->driver->disconnect( pn->port->cable );
			break;
		}

#if TODO
	printf( _("Initializing %s on FTDI device %s\n"), _(cable_drivers[i]->description), par[0] );
#else
	printf( _("Initializing on FTDI device %s\n"), par[0] );
#endif

	return 1;
}


parport_t *
ftd2xx_std_connect( const char **par, int parnum )
{
	parport_t *parport;

	if (!ftd2xx_pre_connect(par, parnum))
		return NULL;

	parport = ftd2xx_parport_alloc( par[0], &ftd2xx_parport_driver, OUTBUF_LEN_STD );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	return parport;
}


parport_t *
ftd2xx_mpsse_connect( const char **par, int parnum )
{
	parport_t *parport;

	if (!ftd2xx_pre_connect(par, parnum))
		return NULL;

	parport = ftd2xx_parport_alloc( par[0], &ftd2xx_mpsse_parport_driver, OUTBUF_LEN_MPSSE );
	if (!parport) {
		printf( _("%s(%d) Out of memory.\n"), __FILE__, __LINE__ );
		return NULL;
	}

	return parport;
}


static int
ftd2xx_generic_open( parport_t *parport )
{
	ftd2xx_params_t *p = parport->params;
	FT_STATUS status;

#if !__CYGWIN__ && !__MINGW32__
	/* Add non-standard Vid/Pid to the linux driver */
	if ((status = FT_SetVIDPID(p->vendor_id, p->product_id)) != FT_OK)
		fprintf( stderr, "Warning: couldn't add %4.4x:%4.4x", p->vendor_id, p->product_id );
#endif

	if ((status = FT_Open(0, &(p->fc))) != FT_OK) {
		fprintf( stderr, "Error: unable to open FTDI device: %li\n", status);
		/* mark ftd2xx layer as not initialized */
		p->fc = NULL;
		return -1;
	}

	return 0;
}


static int
ftd2xx_std_open( parport_t *parport )
{
	int r;
	ftd2xx_params_t *p = parport->params;
	FT_HANDLE fc;
	FT_STATUS status;

	r = ftd2xx_generic_open(parport);
	if (r < 0)
		return r;

	fc = p->fc;

	if ((status = FT_SetBitMode( fc, 0x00, 0x00 )) != FT_OK) {
		fprintf(stderr, "Can't disable bitmode: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_SetLatencyTimer(fc, 2)) != FT_OK) {
		fprintf(stderr, "Can't set latency timer: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_SetBaudRate(fc, 3E6)) != FT_OK) {
		fprintf(stderr, "Can't set baudrate: %li\n", status);
		FT_Close(fc);
		return -1;
	}

	return 0;
}


static int
ftd2xx_mpsse_open( parport_t *parport )
{
	int r;
	ftd2xx_params_t *p = parport->params;
	FT_HANDLE fc;
	FT_STATUS status;

	r = ftd2xx_generic_open(parport);
	if (r < 0)
		return r;

	fc = p->fc;

	/* This sequence might seem weird and containing superfluous stuff.
	   However, it's built after the description of JTAG_InitDevice
	     Ref. FTCJTAGPG10.pdf
	   Intermittent problems will occur when certain steps are skipped. */
	if ((status = FT_ResetDevice(fc)) != FT_OK) {
		fprintf(stderr, "Can't reset device: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_Purge(fc, FT_PURGE_RX)) != FT_OK) {
		fprintf(stderr, "Can't purge buffers: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_SetChars(fc, 0, 0, 0, 0)) != FT_OK) {
		fprintf(stderr, "Can't set special characters: %li\n", status);
		FT_Close(fc);
		return -1;
	}
        /* set a reasonable latency timer value
           if this value is too low then the chip will send intermediate result data
           in short packets (suboptimal performance) */
	if ((status = FT_SetLatencyTimer(fc, 16)) != FT_OK) {
		fprintf(stderr, "Can't set target latency timer: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_SetBitMode( fc, 0x00, 0x00 )) != FT_OK) {
		fprintf(stderr, "Can't disable bitmode: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_SetBitMode( fc, 0x0b, 0x02 /*BITMODE_MPSSE*/ )) != FT_OK) {
		fprintf(stderr, "Can't set bitmode: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_ResetDevice(fc)) != FT_OK) {
		fprintf(stderr, "Can't reset device: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_Purge(fc, FT_PURGE_RX)) != FT_OK) {
		fprintf(stderr, "Can't purge buffers: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	/* set TCK Divisor */
	ftd2xx_set_data(parport, 0x86);
	ftd2xx_set_data(parport, 0x00);
	ftd2xx_set_data(parport, 0x00);
	ftd2xx_set_control(parport, 1);
	ftd2xx_set_control(parport, 0);
	/* switch off loopback */
	ftd2xx_set_data(parport, 0x85);
	ftd2xx_set_control(parport, 1);
	ftd2xx_set_control(parport, 0);
	if ((status = FT_ResetDevice(fc)) != FT_OK) {
		fprintf(stderr, "Can't reset device: %li\n", status);
		FT_Close(fc);
		return -1;
	}
	if ((status = FT_Purge(fc, FT_PURGE_RX)) != FT_OK) {
		fprintf(stderr, "Can't purge buffers: %li\n", status);
		FT_Close(fc);
		return -1;
	}

	return 0;
}


static int
ftd2xx_flush_output ( ftd2xx_params_t *p )
{
	DWORD xferred;
	FT_STATUS status;

	if ((status = FT_Write( p->fc, p->outbuf, (DWORD)(p->outcount), &xferred )) != FT_OK)
		fprintf( stderr, "Error: FT_Write() failed in %i\n", __LINE__ );

	if(xferred > 0 && xferred < p->outcount)
	{
		int offset = xferred;
		int remaining = p->outcount - xferred;

		while(remaining)
		{
			printf("W\n");
			if(xferred < 0) return xferred;
			if ((status = FT_Write( p->fc, p->outbuf + offset, (DWORD)remaining, &xferred)) != FT_OK)
				fprintf( stderr, "Error: FT_Write() failed in %i\n", __LINE__ );
            if(xferred < 0)
            {
              memmove(p->outbuf, p->outbuf + offset, remaining);
              p->outcount = remaining;
              return 0;
            }
            offset += xferred;
			remaining  -= xferred;
		}
	};
	p->outcount = 0;

	return 0;
}

static int
ftd2xx_close( parport_t *parport )
{
	ftd2xx_params_t *p = parport->params;

	if (p->fc) {
		if(p->outcount > 0) ftd2xx_flush_output( p );
		p->outcount = 0;

		FT_SetBitMode( p->fc, 0x00, 0x00 );
		FT_Close(p->fc);
	}

	return 0;
}

static int
ftd2xx_set_data( parport_t *parport, uint8_t data )
{
	ftd2xx_params_t *p = parport->params;
	DWORD xferred;
	FT_STATUS status;

	if (p->fc) {
		if(p->autoflush)
		{
			if ((status = FT_Write( p->fc, &data, 1 , &xferred)) != FT_OK)
				fprintf( stderr, "Error: FT_Write() failed in %i\n", __LINE__ );
			if (xferred != 1)
				printf("w\n");
		}
		else
		{
			p->outbuf[p->outcount++] = data;

			if(p->outcount >= p->outbuf_len)
				return ftd2xx_flush_output( p );
		}
	}
	   
	return 0;
}

static int
ftd2xx_get_data( parport_t *parport )
{
	DWORD xferred = 0;
	FT_STATUS status;
	unsigned char d;
	ftd2xx_params_t *p = parport->params;

	if (p->fc) {
		while (xferred == 0) {
			if ((status = FT_Read( p->fc, &d, 1, &xferred )) != FT_OK)
				printf( "Error: FT_Read() failed in %i\n", __LINE__ );
		}
		return d;
	} else
		return 0;
}

static int
ftd2xx_get_status( parport_t *parport )
{
	return 0;
}

static int
ftd2xx_set_control( parport_t *parport, uint8_t data )
{
	ftd2xx_params_t *p = parport->params;

	if (p->fc) {
		p->autoflush = data;
		if(p->autoflush) ftd2xx_flush_output( p );
	}
	
	return 0;
}

parport_driver_t ftd2xx_parport_driver = {
	"ftd2xx",
	ftd2xx_std_connect,
	ftd2xx_parport_free,
	ftd2xx_std_open,
	ftd2xx_close,
	ftd2xx_set_data,
	ftd2xx_get_data,
	ftd2xx_get_status,
	ftd2xx_set_control
};

parport_driver_t ftd2xx_mpsse_parport_driver = {
	"ftd2xx-mpsse",
	ftd2xx_mpsse_connect,
	ftd2xx_parport_free,
	ftd2xx_mpsse_open,
	ftd2xx_close,
	ftd2xx_set_data,
	ftd2xx_get_data,
	ftd2xx_get_status,
	ftd2xx_set_control
};
