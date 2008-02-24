/*
 * $Id$
 *
 * Macraigor Wiggler JTAG Cable Driver
 * Copyright (C) 2002, 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * Documentation:
 * [1] http://www.ocdemon.net/
 * [2] http://jtag-arm9.sourceforge.net/hardware.html
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cable.h"
#include "parport.h"
#include "chain.h"

#include "generic.h"
#include "generic_parport.h"

#include <cmd.h>

/*
 * Bit <-> pin mapping of an original Wiggler
 * This is the default when no mapping is specified for wiggler_connect( )
 *
 * data D[7:0] (pins 9:2)
 */
#define	nTRST	4	/* nTRST is not inverted in the cable */
#define	TDI	3
#define	TCK	2
#define	TMS	1
#define	nSRESET 0	/* sRESET is inverted in the cable */

/*
 * 7 - BUSY (pin 11)
 * 6 - ACK (pin 10)
 * 5 - PE (pin 12)
 * 4 - SEL (pin 13)
 * 3 - ERROR (pin 15)
 */
#define	TDO 	7	


/* macros used to stringify the defines above */
#define xstr(s) str(s)
#define str(s) #s
static const
char *std_wgl_map = xstr(TDO)   ","
                    xstr(nTRST) ","
                    xstr(TDI)   ","
                    xstr(TCK)   ","
                    xstr(TMS)   ","
                    "#" xstr(nSRESET);


/* private parameters of this cable driver */
typedef struct {
	int trst_lvl;
	int srst_act, srst_inact;
	int tms_act, tms_inact;
	int tck_act, tck_inact;
	int tdi_act, tdi_inact;
	int tdo_act, tdo_inact;
	int trst_act, trst_inact;
	int unused_bits;
} wiggler_params_t;


/* access macros for the parameters */
#define	PRM_TRST_LVL(cable)    ((wiggler_params_t *) cable->params)->trst_lvl
#define	PRM_SRST_ACT(cable)    ((wiggler_params_t *) cable->params)->srst_act
#define	PRM_SRST_INACT(cable)  ((wiggler_params_t *) cable->params)->srst_inact
#define	PRM_TMS_ACT(cable)     ((wiggler_params_t *) cable->params)->tms_act
#define	PRM_TMS_INACT(cable)   ((wiggler_params_t *) cable->params)->tms_inact
#define	PRM_TCK_ACT(cable)     ((wiggler_params_t *) cable->params)->tck_act
#define	PRM_TCK_INACT(cable)   ((wiggler_params_t *) cable->params)->tck_inact
#define	PRM_TDI_ACT(cable)     ((wiggler_params_t *) cable->params)->tdi_act
#define	PRM_TDI_INACT(cable)   ((wiggler_params_t *) cable->params)->tdi_inact
#define	PRM_TDO_ACT(cable)     ((wiggler_params_t *) cable->params)->tdo_act
#define	PRM_TDO_INACT(cable)   ((wiggler_params_t *) cable->params)->tdo_inact
#define	PRM_TRST_ACT(cable)    ((wiggler_params_t *) cable->params)->trst_act
#define	PRM_TRST_INACT(cable)  ((wiggler_params_t *) cable->params)->trst_inact
#define	PRM_UNUSED_BITS(cable) ((wiggler_params_t *) cable->params)->unused_bits



static int
map_pin( char *pin, int *act, int *inact )
{
	int bitnum;
	int inverted = 0;

	if ( *pin == '#' ) {
		inverted = 1;
		pin++;
	}

	if ( !isdigit( *pin ) )
		return -1;

	bitnum = atoi( pin ) % 8;

	bitnum = 1 << bitnum;

	*act   = inverted ? 0 : bitnum;
	*inact = inverted ? bitnum : 0;

	return 0;
}


static int
set_mapping( char *bitmap, cable_t *cable )
{
	const char delim = ',';
	int syntax = 0;
	char *tdo, *trst, *tdi, *tck, *tms, *srst;

	/* assign mappings for each pin */
	if ( ( tdo = bitmap ) )
		if ( ( trst = strchr( tdo,  delim ) ) ) {
			trst++;
			if ( ( tdi = strchr( trst, delim ) ) ) {
				tdi++;
				if ( ( tck = strchr( tdi,  delim ) ) ) {
					tck++;
					if ( ( tms = strchr( tck,  delim ) ) ) {
						tms++;
						if ( ( srst = strchr( tms,  delim ) ) ) {
							srst++;
							syntax = 1;
						} } } } }

	if ( !syntax )
		return -1;

	if ( map_pin( tdo,  &(PRM_TDO_ACT(cable)),  &(PRM_TDO_INACT(cable))  ) != 0 ) return -1;
	if ( map_pin( trst, &(PRM_TRST_ACT(cable)), &(PRM_TRST_INACT(cable)) ) != 0 ) return -1;
	if ( map_pin( tdi,  &(PRM_TDI_ACT(cable)),  &(PRM_TDI_INACT(cable))  ) != 0 ) return -1;
	if ( map_pin( tck,  &(PRM_TCK_ACT(cable)),  &(PRM_TCK_INACT(cable))  ) != 0 ) return -1;
	if ( map_pin( tms,  &(PRM_TMS_ACT(cable)),  &(PRM_TMS_INACT(cable))  ) != 0 ) return -1;
	if ( map_pin( srst, &(PRM_SRST_ACT(cable)), &(PRM_SRST_INACT(cable)) ) != 0 ) return -1;

	return 0;
}


static int
wiggler_connect( char *params[], cable_t *cable )
{
	int result;
	char *param_bitmap = NULL;
	wiggler_params_t *wiggler_params;

	if ( cmd_params ( params ) == 4 ) {
		/* acquire optional parameter for bit<->pin mapping */
		param_bitmap = params[3];
		/* generic_parport_connect() shouldn't see this parameter */
		params[3] = NULL;
	}

	if ( ( result = generic_parport_connect( params, cable ) ) != 0)
		return result;

	if ( param_bitmap )
		params[3] = param_bitmap;

	if ( ( wiggler_params = malloc( sizeof *wiggler_params ) ) == NULL )
		return 4;

	/* set new wiggler-specific params */
	free(cable->params);
	cable->params = wiggler_params;


	if ( ! param_bitmap )
		param_bitmap = (char *)std_wgl_map;

	if ( ( result = set_mapping( param_bitmap, cable ) ) != 0 )
		return result;


	/* Certain Macraigor Wigglers appear to use one of the unused data lines as a
	   power line so set all unused bits high. */
	PRM_UNUSED_BITS(cable) = ~( PRM_SRST_ACT(cable) | PRM_SRST_INACT(cable) |
				 PRM_TMS_ACT(cable)  | PRM_TMS_INACT(cable) |
				 PRM_TCK_ACT(cable)  | PRM_TCK_INACT(cable) |
				 PRM_TDI_ACT(cable)  | PRM_TDI_INACT(cable) |
				 PRM_TRST_ACT(cable) | PRM_TRST_INACT(cable) ) & 0xff;

	return 0;
}

static int
wiggler_init( cable_t *cable )
{
	int data;

	if (parport_open( cable->link.port ))
		return -1;

	if ((data = parport_get_data( cable->link.port )) < 0) {
		if (parport_set_data( cable->link.port, (PRM_TRST_ACT(cable) | PRM_TRST_INACT(cable)) | PRM_UNUSED_BITS(cable)))
			return -1;
		PRM_TRST_LVL(cable) = PRM_TRST_ACT(cable) | PRM_TRST_INACT(cable);
	} else
		PRM_TRST_LVL(cable) = data & (PRM_TRST_ACT(cable) | PRM_TRST_INACT(cable));

	return 0;
}

static void
wiggler_clock( cable_t *cable, int tms, int tdi, int n )
{
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	for (i = 0; i < n; i++) {
		parport_set_data( cable->link.port, PRM_TRST_LVL(cable) |
				PRM_TCK_INACT(cable) |
				(tms ? PRM_TMS_ACT(cable) : PRM_TMS_INACT(cable)) |
				(tdi ? PRM_TDI_ACT(cable) : PRM_TDI_INACT(cable)) |
				PRM_UNUSED_BITS(cable) );
		cable_wait( cable );
		parport_set_data( cable->link.port, PRM_TRST_LVL(cable) |
				PRM_TCK_ACT(cable) |
				(tms ? PRM_TMS_ACT(cable) : PRM_TMS_INACT(cable)) |
				(tdi ? PRM_TDI_ACT(cable) : PRM_TDI_INACT(cable)) |
				PRM_UNUSED_BITS(cable) );
		cable_wait( cable );
	}
}

static int
wiggler_get_tdo( cable_t *cable )
{
	parport_set_data( cable->link.port, PRM_TRST_LVL(cable) |
			PRM_TCK_INACT(cable) |
			PRM_UNUSED_BITS(cable) );
	cable_wait( cable );
	return (parport_get_status( cable->link.port ) & (PRM_TDO_ACT(cable) | PRM_TDO_INACT(cable))) ^
	PRM_TDO_ACT(cable) ? 0 : 1;
}

static int
wiggler_set_trst( cable_t *cable, int trst )
{
	PRM_TRST_LVL(cable) = trst ? PRM_TRST_ACT(cable) : PRM_TRST_INACT(cable);

	parport_set_data( cable->link.port, PRM_TRST_LVL(cable) |
			PRM_UNUSED_BITS(cable) );
	return PRM_TRST_LVL(cable) ^ PRM_TRST_ACT(cable) ? 0 : 1;
}

static int
wiggler_get_trst( cable_t *cable )
{
	return PRM_TRST_LVL(cable) ^ PRM_TRST_ACT(cable) ? 0 : 1;
}

static void
wiggler_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s parallel PORTADDR [TDO,TRST,TDI,TCK,TMS,SRESET]\n"
#if HAVE_LINUX_PPDEV_H
		"   or: cable %s ppdev PPDEV [TDO,TRST,TDI,TCK,TMS,SRESET]\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
		"   or: cable %s ppi PPIDEV [TDO,TRST,TDI,TCK,TMS,SRESET]\n"
#endif
		"\n"
		"PORTADDR   parallel port address (e.g. 0x378)\n"
#if HAVE_LINUX_PPDEV_H
		"PPDEF      ppdev device (e.g. /dev/parport0)\n"
#endif
#if HAVE_DEV_PPBUS_PPI_H
		"PPIDEF     ppi device (e.g. /dev/ppi0)\n"
#endif
		"TDO, ...   parallel port bit number, prepend '#' for inversion\n"
		"           default is '%s'\n"
		"\n"
	),
#if HAVE_LINUX_PPDEV_H
    cablename,
#endif
#if HAVE_DEV_PPBUS_PPI_H
    cablename,
#endif
    cablename,
    std_wgl_map
    );
}

cable_driver_t wiggler_cable_driver = {
	"WIGGLER",
	N_("Macraigor Wiggler JTAG Cable"),
	wiggler_connect,
	generic_disconnect,
	generic_parport_free,
	wiggler_init,
	generic_parport_done,
	generic_set_frequency,
	wiggler_clock,
	wiggler_get_tdo,
	generic_transfer,
	wiggler_set_trst,
	wiggler_get_trst,
	generic_flush_one_by_one,
	wiggler_help
};

cable_driver_t igloo_cable_driver = {
	"IGLOO",
	N_("Excelpoint IGLOO JTAG Cable"),
	wiggler_connect,
	generic_disconnect,
	generic_parport_free,
	wiggler_init,
	generic_parport_done,
	generic_set_frequency,
	wiggler_clock,
	wiggler_get_tdo,
	generic_transfer,
	wiggler_set_trst,
	wiggler_get_trst,
	generic_flush_one_by_one,
	wiggler_help
};

