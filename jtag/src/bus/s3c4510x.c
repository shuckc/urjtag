/**
**  @file s3c4510.c
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
**      the bus parameter is defined as 32-bis, but actually controlled by
**      @ref dbus_width. Make sure that B0SIZE[0:1] is welded correct.
**      Otherwise, you must modify @ref s3c4510_bus_width().
**    - ROM/Flash is selected by nRCS[5:0], now suppose only nRCS0.
**      So is nWBE[4:0], now suppose only nWBE0
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


/** @brief  Bus driver for Samsung S3C4510X */
typedef struct {
        chain_t *chain;
        part_t *part;
        signal_t *a[22];      /**< Only 22-bits addressing */
        signal_t *d[32];      /**< Data bus */
        signal_t *nrcs[6];    /**< not ROM/SRAM/Flash Chip Select;
                              ** Only using nRCS0. */
        signal_t *nwbe[4];    /**< not Write Byte Enable */
        signal_t *noe;        /**< not Output Enable */
} bus_params_t;

#define CHAIN   ((bus_params_t *) bus->params)->chain
#define PART    ((bus_params_t *) bus->params)->part
#define A       ((bus_params_t *) bus->params)->a
#define D       ((bus_params_t *) bus->params)->d
#define nRCS    ((bus_params_t *) bus->params)->nrcs
#define nWBE    ((bus_params_t *) bus->params)->nwbe
#define nOE     ((bus_params_t *) bus->params)->noe


/** @brief  Width of Data Bus. Detected by B0SIZE[1:0] */
unsigned char dbus_width = 16;


static void
setup_address( bus_t *bus, uint32_t a )
{
        int i;
        part_t *p = PART;

        for (i = 0; i < 22; i++)
                part_set_signal( p, A[i], 1, (a >> i) & 1 );
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

static void
s3c4510_bus_printinfo( void )
{
        int i;

        for (i = 0; i < CHAIN->parts->len; i++)
                if (PART == CHAIN->parts->parts[i])
                        break;
        printf( _("Samsung S3C4510B compatibile bus driver via BSR (JTAG part No. %d)\n"), i );
}

static void
s3c4510_bus_prepare( bus_t *bus )
{
        part_set_instruction( PART, "EXTEST" );
        chain_shift_instructions( CHAIN );
}

static void
s3c4510_bus_read_start( bus_t *bus, uint32_t adr )
{
        /* see Figure 4-19 in [1] */
        part_t *p = PART;
        chain_t *chain = CHAIN;

        part_set_signal( p, nRCS[0], 1, 0 );
        part_set_signal( p, nRCS[1], 1, 1 );
        part_set_signal( p, nRCS[2], 1, 1 );
        part_set_signal( p, nRCS[3], 1, 1 );
        part_set_signal( p, nRCS[4], 1, 1 );
        part_set_signal( p, nRCS[5], 1, 1 );
        part_set_signal( p, nWBE[0], 1, 1 );
        part_set_signal( p, nWBE[1], 1, 1 );
        part_set_signal( p, nWBE[2], 1, 1 );
        part_set_signal( p, nWBE[3], 1, 1 );
        part_set_signal( p, nOE, 1, 0 );

        setup_address( bus, adr );
        set_data_in( bus );

        chain_shift_data_registers( chain, 0 );
}

static uint32_t
s3c4510_bus_read_next( bus_t *bus, uint32_t adr )
{
        /* see Figure 4-20 in [1] */
        part_t *p = PART;
        chain_t *chain = CHAIN;
        int i;
        uint32_t d = 0;

        setup_address( bus, adr );
        chain_shift_data_registers( chain, 1 );

        for (i = 0; i < dbus_width; i++)
                d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

        return d;
}

static uint32_t
s3c4510_bus_read_end( bus_t *bus )
{
        /* see Figure 4-19 in [1] */
        part_t *p = PART;
        chain_t *chain = CHAIN;
        int i;
        uint32_t d = 0;

        part_set_signal( p, nRCS[0], 1, 1 );
        part_set_signal( p, nRCS[1], 1, 1 );
        part_set_signal( p, nRCS[2], 1, 1 );
        part_set_signal( p, nRCS[3], 1, 1 );
        part_set_signal( p, nRCS[4], 1, 1 );
        part_set_signal( p, nRCS[5], 1, 1 );
        part_set_signal( p, nWBE[0], 1, 1 );
        part_set_signal( p, nWBE[1], 1, 1 );
        part_set_signal( p, nWBE[2], 1, 1 );
        part_set_signal( p, nWBE[3], 1, 1 );
        part_set_signal( p, nOE, 1, 1 );
        chain_shift_data_registers( chain, 1 );

        for (i = 0; i < dbus_width; i++)
                d |= (uint32_t) (part_get_signal( p, D[i] ) << i);

        return d;
}

static uint32_t
s3c4510_bus_read( bus_t *bus, uint32_t adr )
{
        s3c4510_bus_read_start( bus, adr );
        return s3c4510_bus_read_end( bus );
}


/**
**  @brief
**    ROM/SRAM/FlashPage Write Access Timing
*/
static void
s3c4510_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
        /* see Figure 4-21 in [1] */
        part_t *p = PART;
        chain_t *chain = CHAIN;

        part_set_signal( p, nRCS[0], 1, 0 );
        part_set_signal( p, nRCS[1], 1, 1 );
        part_set_signal( p, nRCS[2], 1, 1 );
        part_set_signal( p, nRCS[3], 1, 1 );
        part_set_signal( p, nRCS[4], 1, 1 );
        part_set_signal( p, nRCS[5], 1, 1 );
        part_set_signal( p, nWBE[0], 1, 1 );
        part_set_signal( p, nWBE[1], 1, 1 );
        part_set_signal( p, nWBE[2], 1, 1 );
        part_set_signal( p, nWBE[3], 1, 1 );
        part_set_signal( p, nOE, 1, 1 );

        setup_address( bus, adr );
        setup_data( bus, data );

        chain_shift_data_registers( chain, 0 );

        part_set_signal( p, nWBE[0], 1, 0 );
        chain_shift_data_registers( chain, 0 );
        part_set_signal( p, nWBE[0], 1, 1 );
        part_set_signal( p, nRCS[0], 1, 1 );
        part_set_signal( p, nRCS[1], 1, 1 );
        part_set_signal( p, nRCS[2], 1, 1 );
        part_set_signal( p, nRCS[3], 1, 1 );
        part_set_signal( p, nRCS[4], 1, 1 );
        part_set_signal( p, nRCS[5], 1, 1 );
        chain_shift_data_registers( chain, 0 );
}

static int
s3c4510_bus_width( bus_t *bus, uint32_t adr )
{
        int b0size0, b0size1;

        b0size0 = part_get_signal( PART, part_find_signal( PART, "B0SIZE0" ));
        b0size1 = part_get_signal( PART, part_find_signal( PART, "B0SIZE1" ));

        switch ((b0size1 << 1) | b0size0) {
                case 1:
                        printf( "B0SIZE[1:0]: 01, 8 bits\n" );
                        dbus_width = 8;
                        return 8;
                case 2:
                        printf( "B0SIZE[1:0]: 10, 16 bits\n" );
                        dbus_width = 16;
                        return 16;
                case 3:
                        printf( "B0SIZE[1:0]: 11, 32 bits\n" );
                        dbus_width = 32;
                        return 32;
                default:
                        printf( "B0SIZE[1:0]: Unknown\n" );
                        return 0;
        }
}

static void
s3c4510_bus_free( bus_t *bus )
{
        free( bus->params );
        free( bus );
}

static const bus_t s3c4510_bus = {
        NULL,
        s3c4510_bus_printinfo,
        s3c4510_bus_prepare,
        s3c4510_bus_width,
        s3c4510_bus_read_start,
        s3c4510_bus_read_next,
        s3c4510_bus_read_end,
        s3c4510_bus_read,
        s3c4510_bus_write,
        s3c4510_bus_free
};

bus_t *
new_s3c4510_bus( chain_t *chain, int pn )
{
        bus_t *bus;
        char buff[10];
        int i;
        int failed = 0;

        if (!chain || !chain->parts || chain->parts->len <= pn || pn < 0)
                return NULL;

        bus = malloc( sizeof (bus_t) );
        if (!bus)
                return NULL;

        memcpy( bus, &s3c4510_bus, sizeof (bus_t) );

        bus->params = malloc( sizeof (bus_params_t) );
        if (!bus->params) {
                free( bus );
                return NULL;
        }

        CHAIN = chain;
        PART = chain->parts->parts[pn];

        for (i = 0; i < 22; i++) {
                sprintf( buff, "ADDR%d", i );
                A[i] = part_find_signal( PART, buff );
                if (!A[i]) {
                        printf( _("signal '%s' not found\n"), buff );
                        failed = 1;
                        break;
                }
        }
        for (i = 0; i < 32; i++) {
                sprintf( buff, "XDATA%d", i );
                D[i] = part_find_signal( PART, buff );
                if (!D[i]) {
                        printf( _("signal '%s' not found\n"), buff );
                        failed = 1;
                        break;
                }
        }
        for (i = 0; i < 6; i++) {
                sprintf( buff, "nRCS%d", i );
                nRCS[i] = part_find_signal( PART, buff );
                if (!nRCS[i]) {
                        printf( _("signal '%s' not found\n"), buff );
                        failed = 1;
                        break;
                }
        }
        for (i = 0; i < 4; i++) {
                sprintf( buff, "nWBE%d", i );
                nWBE[i] = part_find_signal( PART, "nWBE" );
                if (!nWBE) {
                        printf( _("signal '%s' not found\n"), buff );
                        failed = 1;
                        break;
                }
        }
        nOE = part_find_signal( PART, "nOE" );
        if (!nOE) {
                printf( _("signal '%s' not found\n"), "nOE" );
                failed = 1;
        }

        if (failed) {
                free( bus->params );
                free( bus );
                return NULL;
        }

        return bus;
}


/*=============================================================================
**
**  CVS Log
**  $Log$
**  Revision 1.1  2003/08/19 09:53:25  telka
**  2003-08-19  Marcel Telka  <marcel@telka.sk>
**
**  	* src/bus/Makefile.am (libbus_a_SOURCES): Added s3c4510x.c.
**  	* src/bus/s3c4510x.c: New file (Jiun-Shian Ho).
**
*/
