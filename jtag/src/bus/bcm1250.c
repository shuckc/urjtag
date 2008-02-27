//#define USE_BCM_EJTAG
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
 * Written by Matan Ziv-Av.
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#include "sysdep.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "part.h"
#include "bus.h"
#include "chain.h"
#include "bssignal.h"
#include "jtag.h"
#include "buses.h"

#ifdef USE_BCM_EJTAG
int bcm1250_ejtag_do(bus_t *, uint64_t, uint64_t, int, int, unsigned char *, int);
#endif
typedef struct {
	chain_t *chain;
	part_t *part;
	signal_t *io_ad[32];
	signal_t *io_cs_l[7];
	signal_t *io_rw;
	signal_t *io_wr_l;
	signal_t *io_oe_l;
} bus_params_t;

#define CHAIN   ((bus_params_t *) bus->params)->chain
#define PART    ((bus_params_t *) bus->params)->part

#define IO_AD ((bus_params_t *) bus->params)->io_ad
#define IO_CS_L ((bus_params_t *) bus->params)->io_cs_l
#define IO_RW ((bus_params_t *) bus->params)->io_rw
#define IO_WR_L ((bus_params_t *) bus->params)->io_wr_l
#define IO_OE_L ((bus_params_t *) bus->params)->io_oe_l

static void
setup_address( bus_t *bus, uint32_t a )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 24; i++) {
		part_set_signal( p, IO_AD[i], 1, (a >> i) & 1 );
	}
}

static void
set_data_in( bus_t *bus )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 8; i++) {
		part_set_signal( p, IO_AD[i+24], 0, 0 );
	}
}

static void
setup_data( bus_t *bus, uint32_t d )
{
	int i;
	part_t *p = PART;

	for (i = 0; i < 8; i++) {
		part_set_signal( p, IO_AD[i+24], 1, (d >> i) & 1 );
	}
}

#ifndef USE_BCM_EJTAG
static void
bcm1250_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("Broadcom BCM1250 compatible bus driver via BSR (JTAG part No. %d)\n"), i );
}

void
bcm1250_bus_read_start( bus_t *bus, uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, IO_CS_L[0], 1, 0 );
	part_set_signal( p, IO_CS_L[1], 1, 1 );
	part_set_signal( p, IO_CS_L[2], 1, 1 );
	part_set_signal( p, IO_CS_L[3], 1, 1 );
	part_set_signal( p, IO_CS_L[4], 1, 1 );
	part_set_signal( p, IO_CS_L[5], 1, 1 );
	part_set_signal( p, IO_CS_L[6], 1, 1 );
	part_set_signal( p, IO_CS_L[7], 1, 1 );
	part_set_signal( p, IO_RW, 1, 1 );
	part_set_signal( p, IO_WR_L, 1, 1 );
	part_set_signal( p, IO_OE_L, 1, 0 );

	setup_address( bus, adr );
	set_data_in( bus );

	chain_shift_data_registers( chain, 0 );
}

uint32_t
bcm1250_bus_read_next( bus_t *bus , uint32_t adr )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	setup_address( bus, adr );
	chain_shift_data_registers( chain, 1 );

	{
		int i;
		uint32_t d = 0;

		for (i = 0; i < 8; i++) {
			d |= (uint32_t) (part_get_signal( p, IO_AD[i+24] ) << i);
		}

		return d;
	}
}

uint32_t
bcm1250_bus_read_end( bus_t *bus )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, IO_CS_L[0], 1, 1 );
	part_set_signal( p, IO_OE_L, 1, 1 );
	chain_shift_data_registers( chain, 1 );

	{
		int i;
		uint32_t d = 0;

		for (i = 0; i < 8; i++) {
			d |= (uint32_t) (part_get_signal( p, IO_AD[i+24] ) << i);
		}

		return d;
	}
}

uint32_t
bcm1250_bus_read( bus_t *bus, uint32_t adr )
{
	uint32_t i;
	bcm1250_bus_read_start( bus, adr );
	i=bcm1250_bus_read_end( bus );
	return i;
}

void
bcm1250_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	part_t *p = PART;
	chain_t *chain = CHAIN;

	part_set_signal( p, IO_CS_L[0], 1, 0 );
	part_set_signal( p, IO_CS_L[1], 1, 1 );
	part_set_signal( p, IO_CS_L[2], 1, 1 );
	part_set_signal( p, IO_CS_L[3], 1, 1 );
	part_set_signal( p, IO_CS_L[4], 1, 1 );
	part_set_signal( p, IO_CS_L[5], 1, 1 );
	part_set_signal( p, IO_CS_L[6], 1, 1 );
	part_set_signal( p, IO_CS_L[7], 1, 1 );
	part_set_signal( p, IO_RW, 1, 0 );
	part_set_signal( p, IO_WR_L, 1, 1 );
	part_set_signal( p, IO_OE_L, 1, 1 );

	setup_address( bus, adr );
	setup_data( bus, data );

	chain_shift_data_registers( chain, 0 );

	part_set_signal( p, IO_WR_L, 1, 0 );
	chain_shift_data_registers( chain, 0 );
	
	part_set_signal( p, IO_WR_L, 1, 1 );
	chain_shift_data_registers( chain, 0 );
}

#else /* BCM_EJTAG */
int addr;
uint64_t base = 0x1fc00000;
uint32_t bcm1250_bus_read(bus_t *bus, uint32_t adr ) {
	unsigned char buf[32];
	bcm1250_ejtag_do(bus, adr+base, 0, 1, 0, buf, 0);
	return buf[adr&0x1f];
		
}

uint32_t bcm1250_bus_read_next(bus_t *bus, uint32_t adr ) {
	uint32_t t;
	t=bcm1250_bus_read(bus, addr);
	addr=adr;
	return t;
}

uint32_t bcm1250_bus_read_end(bus_t *bus) {
	return bcm1250_bus_read(bus, addr);
}

void bcm1250_bus_read_start(bus_t *bus, uint32_t adr ) {
addr=adr;
}

void
bcm1250_bus_write( bus_t *bus, uint32_t adr, uint32_t data ) {
	unsigned char buf[32];
	bcm1250_ejtag_do(bus, adr+base, data, 0, 0, buf, 0);
}

#endif
	
static int
bcm1250_bus_area( bus_t *bus, uint32_t addr, bus_area_t *area )
{
	area->description = NULL;
	area->start = UINT32_C(0x00000000);
	area->length = UINT64_C(0x100000000);
	area->width = 8;

	return 0;
}

static void
bcm1250_bus_prepare( bus_t *bus )
{
	part_set_instruction( PART, "EXTEST" );
	chain_shift_instructions( CHAIN );
}

static void
bcm1250_bus_free( bus_t *bus )
{
	free( bus->params );
	free( bus );
}

static bus_t *bcm1250_bus_new( chain_t *chain, char *cmd_params[] );

const bus_driver_t bcm1250_bus = {
	"bcm1250",
	N_("Broadcom BCM1250 compatible bus driver via BSR"),
	bcm1250_bus_new,
	bcm1250_bus_free,
	bcm1250_bus_printinfo,
	bcm1250_bus_prepare,
	bcm1250_bus_area,
	bcm1250_bus_read_start,
	bcm1250_bus_read_next,
	bcm1250_bus_read_end,
	bcm1250_bus_read,
	bcm1250_bus_write,
	NULL
};

static bus_t *
bcm1250_bus_new( chain_t *chain, char *cmd_params[] )
{
    bus_t *bus;
    char buff[10];
    int i;
    int failed = 0;

	if (!chain || !chain->parts || chain->parts->len <= chain->active_part || chain->active_part < 0)
		return NULL;

    bus = malloc( sizeof (bus_t) );
    if (!bus)
        return NULL;

	bus->driver = & bcm1250_bus;
    bus->params = calloc( 1, sizeof (bus_params_t) );
    if (!bus->params) {
        free( bus );
        return NULL;
    }

    CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

    for (i = 0; i < 32; i++) {
        sprintf( buff, "IO_AD%d", i );
        IO_AD[i] = part_find_signal( PART, buff );
        if (!IO_AD[i]) {
            printf( _("signal '%s' not found\n"), buff );
            failed = 1;
            break;
        }
    }
    for (i = 0; i < 8; i++) {
        sprintf( buff, "IO_CS_L%d", i );
        IO_CS_L[i] = part_find_signal( PART, buff );
        if (!IO_CS_L[i]) {
            printf( _("signal '%s' not found\n"), buff );
            failed = 1;
            break;
        }
    }
    IO_RW = part_find_signal( PART, "IO_RW" );
    if (!IO_RW) {
        printf( _("signal '%s' not found\n"), "IO_RW" );
        failed = 1;
    }
    IO_WR_L = part_find_signal( PART, "IO_WR_L" );
    if (!IO_WR_L) {
        printf( _("signal '%s' not found\n"), "IO_WR_L" );
        failed = 1;
    }
    IO_OE_L = part_find_signal( PART, "IO_OE_L" );
    if (!IO_OE_L) {
        printf( _("signal '%s' not found\n"), "IO_OE_L" );
        failed = 1;
    }

    if (failed) {
        free( bus->params );
        free( bus );
        return NULL;
    }

    return bus;
}


#ifdef USE_BCM_EJTAG
int bcm1250_ejtag_do(bus_t *bus, uint64_t ad, uint64_t da, int read, int type, 
		unsigned char *buf, int verbose) {
	
	part_t *p = PART;
	chain_t *chain = CHAIN;
	char ctrl[15]="010000000000";
	char addrr[80]="0000" "111" "000" 
		"11111111111111111111111111111111"
		"00000000" "00011111" "11000000" "00000000" "000";
	int j, k, n, m;
	uint64_t a;

	if(verbose)printf("BCM1250: ejtag_do(%08Lx, %08Lx, %i, %i)\n", ad, da, read, type);
	
	a=ad>>5;
	for(j=0;j<35;j++) {
		addrr[76-j]='0'+(a&1);
		a>>=1;
	}
	
	j=(1<<type)-1;
	for(m=10; m<42; m++) addrr[m]='0';
	n=ad&(~j&0x1f);
	for(m=n; m<n+(1<<type); m++) addrr[m+10]='1';	

	ctrl[2]='0';
	ctrl[3]='0';
	part_set_instruction( p, "CONTROLL" );
	chain_shift_instructions(chain);
	j=strlen(ctrl);
	k=0;
	while(j>0){
		j--;
		p->active_instruction->data_register->in->data[j]=ctrl[k++]&1;
	}
	chain_shift_data_registers( chain, 1 );


	if(read) {
		addrr[7]='0';
		addrr[8]='0';
		addrr[9]='0';
	} else {
		addrr[7]='0';
		addrr[8]='1';
		addrr[9]='0';
	}

	part_set_instruction( p, "ADDR" );
	chain_shift_instructions(chain);
	j=strlen(addrr);
	k=0;
	while(j>0){
		j--;
		p->active_instruction->data_register->in->data[j]=addrr[k++]&1;
	}
	chain_shift_data_registers( chain, 0 );

	if(!read) {
		part_set_instruction( p, "DATA");
		chain_shift_instructions(chain);
		for(j=0;j<277;j++)
			p->active_instruction->data_register->in->data[j]=j&1;
		p->active_instruction->data_register->in->data[259]=1;
		p->active_instruction->data_register->in->data[258]=0;
		p->active_instruction->data_register->in->data[257]=0;
		p->active_instruction->data_register->in->data[256]=1;
		j=0;
		if(type<5) {
			k=256-(n+(1<<type))*8;
			while(j<(8<<type)){
				p->active_instruction->data_register->in->data[k+j]=da&1;
				da>>=1;
				j++;
			}
		} else {
			int r;
			for(r=0; r<32; r++) {
				int s,t;
				t=buf[r];
				for(s=0;s<8;s++) {
					p->active_instruction->data_register->in->data[248-r*8+s]=t&1;
					t>>=1;
				}
			}
		}
		chain_shift_data_registers( chain, 0 );
	}


	ctrl[2]='1';
	if(!read)ctrl[3]='1';
	part_set_instruction( p, "CONTROLL" );
	chain_shift_instructions(chain);
	j=strlen(ctrl);
	k=0;
	while(j>0){
		j--;
		p->active_instruction->data_register->in->data[j]=ctrl[k++]&1;
	}
	chain_shift_data_registers( chain, 1 );
	if(verbose || read) {
		volatile int q;
		int to;
		
		to=5;
		for(q=0;q<100;q++);
		part_set_instruction( p, "DATA");
		chain_shift_instructions(chain);
		chain_shift_data_registers( chain, 1 );
		
		while((p->active_instruction->data_register->out->data[276-17]==0) && to--) {
			chain_shift_data_registers( chain, 1 );
		}
		for(j=n;j<n+(1<<type);j++) {
			buf[j]=0;
			for(m=0;m<8;m++) {
				buf[j]<<=1;
				buf[j]+=p->active_instruction->data_register->out->data[255-(j*8)-m]&1;
			}
			if(verbose)printf("%02x ",buf[j]);
		}
		if(verbose) {
			printf("\n");

			printf(" status:\n");
			for(j=0;j<21;j++) {
				printf("%c", '0'+p->active_instruction->data_register->out->data[276-j]);
				if((j==5) || (j==11) || (j==12) || (j==16) || (j==17))printf(" ");
			}
			printf("\n");
		}
	}
	return 0;
}
#endif
