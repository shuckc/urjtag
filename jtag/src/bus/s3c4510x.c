/**
**  @file s3c4510x.c
**
**  $Id$
**
**  Copyright (C) 2003, All Rights Reserved
**
**  This program is free software; you can redistribute it and/or
**  modify it under the terms of the GNU General Public License
**  as published by the Free Software Foundation; either version 2
**  of the License, or (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
**  02111-1307, USA.
**
**  @author
**    Jiun-Shian Ho <asky@syncom.com.tw>,
**    Copy from other bus drivers written by Marcel Telka <marcel@telka.sk>
**
**    Krzysztof Blaszkowski <info@sysmikro.com.pl>
**    - fixed bug with driving nWBE, nECS, nSDCS (for SDRAM),
**    - fixed bug with preparing bus state after each chain_shift_data_registers().
**	tested on "peek" command only (2003/10/07).
**
**  @brief
**    Bus driver for Samsung S3C4510X (ARM7TDMI) micro controller.
**
**  @par Reference Documentations
**    - [1] Samsung Electronics Co., Ltd.,
**      "S3C4510B 32-Bit RISC Microcontroller User's Manual",
**      Revision 1, August 2000, Order Number: 21-S3-C4510B-082000
**
**  @note
**    - This bus driver is coded basing on S3C4510B.
**      However, Samsung do NOT giving a special JTAG ID-Code for this chip.
**    - Data Bus width is detected by B0SIZE[0:1];
**      the bus parameter is defined as 32-bit, but actually controlled by
**      @ref dbus_width. Make sure that B0SIZE[0:1] is welded correct.
**      Otherwise, you must modify @ref s3c4510_bus_width().
**    - ROM/Flash is selected by nRCS[5:0], now suppose only nRCS0.
**      So is nWBE[4:0], now suppose only nWBE0
**    - Unfortunately, B0SIZE isn't known before first SCAN/PRELOAD.
**      Is bus driver allowed to do JTAG activity during bus_area or bus_new?
**
=============================================================================*/


#include "sysdep.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"
#include "generic_bus.h"


#ifndef DEBUG_LVL2
#define DEBUG_LVL2(x)
#endif

/** @brief  Bus driver for Samsung S3C4510X */
typedef struct {
        signal_t *a[22];      /**< Only 22-bits addressing */
        signal_t *d[32];      /**< Data bus */
        signal_t *nrcs[6];    /**< not ROM/SRAM/Flash Chip Select;
                              ** Only using nRCS0. */
        signal_t *necs[4];
        signal_t *nsdcs[4];

        signal_t *nwbe[4];    /**< not Write Byte Enable */
        signal_t *noe;        /**< not Output Enable */
	int	 dbuswidth;
} bus_params_t;

#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define nRCS    ((bus_params_t *) bus->params)->nrcs
#define nECS    ((bus_params_t *) bus->params)->necs
#define nSDCS   ((bus_params_t *) bus->params)->nsdcs
#define nWBE    ((bus_params_t *) bus->params)->nwbe
#define nOE     ((bus_params_t *) bus->params)->noe

#define dbus_width ((bus_params_t *) bus->params)->dbuswidth
/** @brief  Width of Data Bus. Detected by B0SIZE[1:0]  */

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
s3c4510_bus_new( chain_t *chain, char *cmd_params[] )
{
        bus_t *bus;
        part_t *part;
        char buff[10];
        int i;
        int failed = 0;

        bus = calloc( 1, sizeof (bus_t) );
        if (!bus)
                return NULL;

	bus->driver = &s3c4510_bus;
	bus->params = calloc( 1, sizeof (bus_params_t) );
        if (!bus->params) {
                free( bus );
                return NULL;
        }


	dbus_width = 16;
        CHAIN = chain;
	PART = part = chain->parts->parts[chain->active_part];

        for (i = 0; i < 22; i++) {
                sprintf( buff, "ADDR%d", i );
		failed |= generic_bus_attach_sig( part, &(A[i]), buff );
        }

        for (i = 0; i < 32; i++) {
                sprintf( buff, "XDATA%d", i );
		failed |= generic_bus_attach_sig( part, &(D[i]), buff );
        }

        for (i = 0; i < 6; i++) {
                sprintf( buff, "nRCS%d", i );
		failed |= generic_bus_attach_sig( part, &(nRCS[i]), buff );
        }

	for (i = 0; i < 4; i++) {
                sprintf( buff, "nECS%d", i );
		failed |= generic_bus_attach_sig( part, &(nECS[i]), buff );
        }

	for (i = 0; i < 4; i++) {
                sprintf( buff, "nRAS%d", i );  /* those are nSDCS for SDRAMs only */
		failed |= generic_bus_attach_sig( part, &(nSDCS[i]), buff );
        }

        for (i = 0; i < 4; i++) {
                sprintf( buff, "nWBE%d", i );
		failed |= generic_bus_attach_sig( part, &(nWBE[i]), buff );
        }

	failed |= generic_bus_attach_sig( part, &(nOE), "nOE" );

        if (failed) {
                free( bus->params );
                free( bus );
                return NULL;
        }

        return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
s3c4510_bus_printinfo( bus_t *bus )
{
        int i;

        for (i = 0; i < CHAIN->parts->len; i++)
                if (PART == CHAIN->parts->parts[i])
                        break;
	printf( _("Samsung S3C4510B compatibile bus driver via BSR (JTAG part No. %d) RCS0=%ubit\n"), i ,dbus_width );
}

/**
 * bus->driver->(*init)
 *
 */
static int
s3c4510_bus_init( bus_t *bus )
{
		part_t *p = PART;
        chain_t *chain = CHAIN;

        part_set_instruction( p, "SAMPLE/PRELOAD" );
        chain_shift_instructions( chain );
        chain_shift_data_registers( chain, 0 );

	INITIALIZED = 1;

	return 0;
}

/**
 * bus->driver->(*area)
 *
 */
static int
s3c4510_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
        int b0size0, b0size1; // , endian;

	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);

	// endian = part_get_signal( PART, part_find_signal( PART, "LITTLE" ));
	b0size0 = part_get_signal( PART, part_find_signal( PART, "B0SIZE0" ));
	b0size1 = part_get_signal( PART, part_find_signal( PART, "B0SIZE1" ));

        switch ((b0size1 << 1) | b0size0) {
                case 1:
			area->width = dbus_width = 8;
			return 0;
                case 2:
			area->width = dbus_width = 16;
			return 0;
                case 3:
			area->width = dbus_width = 32;
			return 0;
                default:
                        printf( "B0SIZE[1:0]: Unknown\n" );
			area->width = 0;
			return -1;
        }
}

static void s3c4510_bus_setup_ctrl( bus_t *bus, int mode )
{
  int k;
  part_t *p = PART;

  for (k = 0; k < 6; k++)
	part_set_signal( p, nRCS[k], 1, (mode & (1 << k)) ? 1 : 0 );

  for (k = 0; k < 4; k++)
	part_set_signal( p, nECS[k], 1, 1 );

  for (k = 0; k < 4; k++)
	part_set_signal( p, nSDCS[k], 1, 1 );

  for (k = 0; k < 4; k++)
	part_set_signal( p, nWBE[k], 1, (mode & (1 << (k + 8))) ? 1 : 0 );

  part_set_signal( p, nOE, 1, (mode & (1 << 16)) ? 1 : 0 );
}

static void
setup_address( bus_t *bus, uint32_t a )
{
        int i,so;
        part_t *p = PART;

		switch(dbus_width)
		{
			case 32: so=2; break;
			case 16: so=1; break;
			default: so=0; break;
		}

        for (i = 0; i < 22; i++)
                part_set_signal( p, A[i], 1, (a >> (i+so)) & 1 );
}

static void
set_data_in( bus_t *bus )
{
        int i;
        part_t *p = PART;

        for (i = 0; i < dbus_width; i++)
                part_set_signal( p, D[i], 0, 0 );
}

static void
setup_data( bus_t *bus, uint32_t d )
{
        int i;
        part_t *p = PART;

        for (i = 0; i < dbus_width; i++)
                part_set_signal( p, D[i], 1, (d >> i) & 1 );
        /* Set other bits as 0 */
        for (i = dbus_width; i < 32; i++)
                part_set_signal( p, D[i], 1, 0 );
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
s3c4510_bus_read_start( bus_t *bus, uint32_t adr )
{
        /* see Figure 4-19 in [1] */
        chain_t *chain = CHAIN;

	s3c4510_bus_setup_ctrl( bus, 0x00fffe);  /* nOE=0, nRCS0 =0 */
        setup_address( bus, adr );
        set_data_in( bus );
        chain_shift_data_registers( chain, 0 );
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
s3c4510_bus_read_next( bus_t *bus, uint32_t adr )
{
        /* see Figure 4-20 in [1] */
        part_t *p = PART;
        chain_t *chain = CHAIN;
        int i;
        uint32_t d = 0;

	s3c4510_bus_setup_ctrl( bus, 0x00fffe);  /* nOE=0, nRCS0 =0 */
        setup_address( bus, adr );
	set_data_in( bus );
        chain_shift_data_registers( chain, 1 );

        for (i = 0; i < dbus_width; i++)
                d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

        return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
s3c4510_bus_read_end( bus_t *bus )
{
        /* see Figure 4-19 in [1] */
        part_t *p = PART;
        chain_t *chain = CHAIN;
        int i;
        uint32_t d = 0;

	s3c4510_bus_setup_ctrl( bus, 0x01ffff);  /* nOE=1, nRCS0 =1 */
        chain_shift_data_registers( chain, 1 );

        for (i = 0; i < dbus_width; i++)
                d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

        return d;
}

/**
 * bus->driver->(*write)
 *
 *  @brief
 *    ROM/SRAM/FlashPage Write Access Timing
 */
static void
s3c4510_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
        /* see Figure 4-21 in [1] */
        chain_t *chain = CHAIN;

	s3c4510_bus_setup_ctrl( bus, 0x01fffe);  /* nOE=1, nRCS0 =0 */
        setup_address( bus, adr );
        setup_data( bus, data );

        chain_shift_data_registers( chain, 0 );

	switch (dbus_width)
	{
	    default:
	    case 8:
		    s3c4510_bus_setup_ctrl( bus, 0x01fefe);  /* nOE=1, nRCS0 =0, nWBE0=0 */
		    break;
	    case 16:
		    s3c4510_bus_setup_ctrl( bus, 0x01fcfe);  /* nOE=1, nRCS0 =0, nWBE0-1=0 */
		    break;

	    case 32:
		    s3c4510_bus_setup_ctrl( bus, 0x01f0fe);  /* nOE=1, nRCS0 =0, nWBE0-3=0 */
		    break;
	}

        setup_address( bus, adr );
        setup_data( bus, data );

        chain_shift_data_registers( chain, 0 );

	s3c4510_bus_setup_ctrl( bus, 0x01ffff);  /* nOE=1, nRCS0 =1 */
        chain_shift_data_registers( chain, 0 );

	DEBUG_LVL2( printf("bus_write %08x @ %08x\n", data, adr); )
}

const bus_driver_t s3c4510_bus = {
	"s3c4510x",
	N_("Samsung S3C4510B compatible bus driver via BSR"),
	s3c4510_bus_new,
	generic_bus_free,
        s3c4510_bus_printinfo,
        generic_bus_prepare_extest,
        s3c4510_bus_area,
        s3c4510_bus_read_start,
        s3c4510_bus_read_next,
        s3c4510_bus_read_end,
        generic_bus_read,
	s3c4510_bus_write,
	s3c4510_bus_init
};



/*=============================================================================
**
**  CVS Log
**  $Log$
**  Revision 1.5  2003/09/10 12:38:01  telka
**  2003-09-10  Marcel Telka  <marcel@telka.sk>
**
**  	* src/bus/bcm1250.c (bcm1250_bus_printinfo, bcm1250_bus): Fixed typo (compatibile->compatible) in
**  		output message (thanks to Andreas Mohr).
**  	* src/bus/ixp425.c (ixp425_bus_printinfo, ixp425_bus): Ditto.
**  	* src/bus/pxa2x0.c (pxa2x0_bus_printinfo, pxa2x0_bus): Ditto.
**  	* src/bus/s3c4510x.c (s3c4510_bus_printinfo, s3c4510_bus): Ditto.
**  	* src/bus/sa1110.c (sa1110_bus_printinfo, sa1110_bus): Ditto.
**  	* src/bus/sh7727.c (sh7727_bus_printinfo, sh7727_bus): Ditto.
**  	* src/bus/sh7750r.c (sh7750r_bus_printinfo, sh7750r_bus): Ditto.
**  	* src/bus/sh7751r.c (sh7751r_bus_printinfo, sh7751r_bus): Ditto.
**
**  Revision 1.4  2003/09/05 21:09:14  telka
**  2003-09-05  Marcel Telka  <marcel@telka.sk>
**
**  	* include/bus.h (bus_drivers): Added constant declaration.
**  	(new_sa1110_bus, new_pxa250_bus, new_ixp425_bus, new_sh7727_bus, new_sh7750r_bus, new_sh7751r_bus)
**  	(new_bcm1250_bus): Function declarations removed.
**  	* src/bus/buses.c (bus_drivers): New constant definition.
**  	* src/bus/buses.h: New file.
**  	* src/bus/Makefile.am (libbus_a_SOURCES): Added buses.h.
**
**  	* src/bus/bcm1250.c (bcm1250_bus_printinfo): Added new function parameter 'bus'.
**  	(bcm1250_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_bcm1250_bus): Function renamed ...
**  	(bcm1250_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/ixp425.c (ixp425_bus_printinfo): Added new function parameter 'bus'.
**  	(ixp425_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_ixp425_bus): Function renamed ...
**  	(ixp425_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/pxa2x0.c (pxa2x0_bus_printinfo): Added new function parameter 'bus'.
**  	(pxa250_bus): Structure transformed ...
**  	(pxa2x0_bus): ... to this constant (changed type to bus_driver_t, changed members).
**  	(new_pxa250_bus): Function renamed ...
**  	(pxa2x0_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/s3c4510x.c (s3c4510_bus_printinfo): Added new function parameter 'bus'.
**  	(s3c4510_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_s3c4510_bus): Function renamed ...
**  	(s3c4510_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/sa1110.c (sa1110_bus_printinfo): Added new function parameter 'bus'.
**  	(sa1110_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_sa1110_bus): Function renamed ...
**  	(sa1110_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/sh7727.c (sh7727_bus_printinfo): Added new function parameter 'bus'.
**  	(sh7727_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_sh7727_bus): Function renamed ...
**  	(sh7727_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/sh7750r.c (sh7750r_bus_printinfo): Added new function parameter 'bus'.
**  	(sh7750r_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_sh7750r_bus): Function renamed ...
**  	(sh7750r_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**  	* src/bus/sh7751r.c (sh7751r_bus_printinfo): Added new function parameter 'bus'.
**  	(sh7751r_bus): Changed structure type to bus_driver_t. Changed members.
**  	(new_sh7751r_bus): Function renamed ...
**  	(sh7751r_bus_new): ... to this one. Changed parameter list to void (and function body updated).
**
**  	* src/cmd/cable.c (cmd_cable_run): Replaced bus->free() call with bus_free().
**  	* src/jtag.c (main): Ditto.
**
**  	* src/cmd/cmd.c (cmds): Added cmd_initbus.
**  	* src/cmd/detect.c (cmd_detect_run): Removed explicit bus driver detection.
**  	* src/cmd/initbus.c: New file.
**  	* src/cmd/Makefile.am (libcmd_a_SOURCES): Added initbus.c.
**
**  	* data/broadcom/bcm1250/bcm1250: Added 'initbus' command call.
**  	* data/hitachi/sh7727/sh7727: Ditto.
**  	* data/intel/ixp425/ixp425: Ditto.
**  	* data/intel/pxa250/pxa250: Ditto.
**  	* data/intel/pxa250/pxa250c0: Ditto.
**  	* data/intel/sa1110/sa1110: Ditto.
**  	* data/samsung/s3c4510b/s3c4510b: Ditto.
**
**  Revision 1.3  2003/08/28 07:26:02  telka
**  2003-08-28  Marcel Telka  <marcel@telka.sk>
**
**  	* src/readmem.c (readmem): Replaced bus_width macro with new bus_area.
**  	* src/bus/bcm1250.c (bcm1250_bus_width): Function removed.
**  	(bcm1250_bus_area): New function.
**  	* src/bus/ixp425.c (ixp425_bus_width): Function removed.
**  	(ixp425_bus_area): New function.
**  	* src/bus/pxa2x0.c (pxa250_bus_width): Function removed.
**  	(pxa2x0_bus_area): New function.
**  	* src/bus/s3c4510x.c (s3c4510_bus_width): Function removed.
**  	(s3c4510_bus_area): New function.
**  	* src/bus/sa1110.c (sa1110_bus_width: Function removed.
**  	(sa1110_bus_area): New function.
**  	* src/bus/sh7727.c (sh7727_bus_width): Function removed.
**  	(sh7727_bus_area): New function.
**  	* src/bus/sh7750r.c (sh7750r_bus_width): Function removed.
**  	(sh7750r_bus_area): New function.
**  	* src/bus/sh7751r.c (sh7751r_bus_width): Function removed.
**  	(sh7751r_bus_area): New function.
**  	* src/cmd/print.c (cmd_print_run): Added bus area printing.
**
**  Revision 1.2  2003/08/19 09:59:26  telka
**  2003-08-19  Marcel Telka  <marcel@telka.sk>
**
**  	* src/bus/s3c4510x.c: Changed comment.
**
**  Revision 1.1  2003/08/19 09:53:25  telka
**  2003-08-19  Marcel Telka  <marcel@telka.sk>
**
**  	* src/bus/Makefile.am (libbus_a_SOURCES): Added s3c4510x.c.
**  	* src/bus/s3c4510x.c: New file (Jiun-Shian Ho).
**
*/
