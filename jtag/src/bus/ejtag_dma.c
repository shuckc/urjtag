/*
 * $Id$
 *
 * EJTAG compatible bus driver via DMA
 * Copyright (C) 2008, Julien Aube
 * Credits goes to
 * - Marek Michalkiewicz (EJTAG Pracc driver),
 * - HairyDairyMaid for the HairyDairyMaid v48 utility,
 * - Florian Fanelli for the help on the flash interface infos,
 * - All others who contributed to the previous projetcs
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
 * Written by Marek Michalkiewicz <marekm@amelek.gda.pl>, 2005.
 *
 * Documentation:
 * [1] MIPS Licensees, "MIPS EJTAG Debug Solution", 980818 Rev. 2.0.0
 * [2] MIPS Technologies, Inc. "EJTAG Specification", 2001-02-15, Rev. 2.60
 * 
 *
 */

//#define PRINT_DATA_DEBUG 1
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
#include "state.h"

typedef struct {
	uint32_t impcode;  /* EJTAG Implementation Register */
} bus_params_t;

#define BP ((bus_params_t *) bus->params)

#define EJTAG_VER ((BP->impcode >> 29) & 7)
#define EJTAG_20	0
#define EJTAG_25	1
#define EJTAG_26	2

/* EJTAG control register bits */
#define PerRst		20
#define PRnW		19
#define PrAcc		18
#define PrRst		16
#define ProbEn		15
#define JtagBrk		12
#define BrkSt		 3
#define Rocc		31
#define ProbTrap	14

/* DMA */
#define DmaAcc      17
#define DstRt       11
#define DmaRwn       9 
#define Derr      10
// default : DMA tranfser size BYTE
#define DMA_HALFWORD 7
#define DMA_WORD     8
#define DMA_BYTE     0

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
ejtag_dma_bus_new( chain_t *chain, const bus_driver_t *driver, char *cmd_params[] )
{
	bus_t *bus;

	bus = calloc( 1, sizeof (bus_t) );
	if (!bus)
		return NULL;

	bus->driver = driver;
	bus->params = calloc( 1, sizeof (bus_params_t) );
	if (!bus->params) {
		free( bus );
		return NULL;
	}

	CHAIN = chain;
	PART = chain->parts->parts[chain->active_part];

	return bus;
}

/**
 * bus->driver->(*printinfo)
 *
 */
static void
ejtag_dma_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("EJTAG compatible bus driver via DMA (JTAG part No. %d)\n"), i );
}

/**
 * helper function
 *
 */
static uint32_t
reg_value( tap_register *reg )
{
	uint32_t retval = 0;
	int i;

	for (i = 0; i < reg->len; i++) {
		if (reg->data[i])
			retval |= (1 << i);
	}
	return retval;
}

#ifdef PRINT_DATA_DEBUG
/* Small debug helper */
char siz_(int sz)
{
	switch(sz) {
		case DMA_WORD: return 'w';
		case DMA_BYTE: return 'b';
		case DMA_HALFWORD: return 'h';
		default : return 'e';
	}
	return 'E';
}
#endif

/**
 * low-level dma write
 *
 */
static void ejtag_dma_write(bus_t *bus, unsigned int addr, unsigned int data,int sz)
{
	static data_register *ejctrl = NULL;
	static data_register *ejaddr = NULL;
	static data_register *ejdata = NULL;
	int i=0;
	int timeout=5;
	
	if (ejctrl == NULL)
		ejctrl = part_find_data_register( PART, "EJCONTROL" );
	if (ejaddr == NULL)
		ejaddr = part_find_data_register( PART, "EJADDRESS" );
	if (ejdata == NULL)
		ejdata = part_find_data_register( PART, "EJDATA" );

	switch (sz) { /* Fill the other bytes with copy of the current */
		case DMA_BYTE:
			data &= 0xff; data |= (data<<8) | (data << 16) | (data << 24);
			break;
		case DMA_HALFWORD:
			data &= 0xffff; data |= (data << 16);
			break;
		default: break;
	}	

	part_set_instruction( PART, "EJTAG_ADDRESS" );
	chain_shift_instructions( CHAIN );
	for (i = 0; i < 32; i++)
		ejaddr->in->data[i] = (addr >> i) & 1;	
	chain_shift_data_registers( CHAIN, 0 ); /* Push the address to write */
#ifdef PRINT_DATA_DEBUG
	printf("Wrote to ejaddr->in      =%s %08X\n",register_get_string( ejaddr->in),reg_value( ejaddr->in ) );
#endif
	part_set_instruction( PART, "EJTAG_DATA" );
	chain_shift_instructions( CHAIN );
	for (i = 0; i < 32; i++)
		ejdata->in->data[i] = (data >> i) & 1;	
	chain_shift_data_registers( CHAIN, 0 ); /* Push the data to write */
#ifdef PRINT_DATA_DEBUG
	printf("Wrote to edata->in(%c)    =%s %08X\n",siz_(sz),register_get_string( ejdata->in),reg_value( ejdata->in ) );
#endif
	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );
	register_fill( ejctrl->in, 0 );
	ejctrl->in->data[PrAcc]  = 1;  // Processor access 
	ejctrl->in->data[ProbEn] = 1; 
	ejctrl->in->data[DmaAcc] = 1; // DMA operation request */
	ejctrl->in->data[DstRt]  = 1;  
	if (sz)
		ejctrl->in->data[sz] = 1; // Size : can be WORD/HALFWORD or nothing for byte
	chain_shift_data_registers( CHAIN, 0 ); /* Do the operation */
	//printf("Wrote to ejctrl->in      =%s %08X\n",register_get_string( ejctrl->in),reg_value( ejctrl->in ) );
	
	do {
		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );
		register_fill( ejctrl->in, 0 );
		ejctrl->in->data[PrAcc] = 1;
		ejctrl->in->data[ProbEn] = 1;
		ejctrl->in->data[DmaAcc] = 1;
		chain_shift_data_registers( CHAIN, 1 );		
		timeout--;
		if ( ! timeout)
			break;
	} while(ejctrl->out->data[DstRt] == 1); // This flag tell us the processor has completed the op

	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );
	register_fill( ejctrl->in, 0 );
	ejctrl->in->data[PrAcc] = 1;
	ejctrl->in->data[ProbEn] = 1;
	chain_shift_data_registers( CHAIN, 1 );	  // Disable DMA, reset state to previous one.
	if (ejctrl->out->data[Derr] == 1) { // Check for DMA error, i.e. incorrect address
		printf( _("%s(%d) Error on dma write (dma transaction failed)\n"),
			__FILE__, __LINE__ );
	}
	return ;
}

/**
 * low level dma read operation
 *
 */
static unsigned int ejtag_dma_read(bus_t *bus, unsigned int addr,int sz)
{
	static data_register *ejctrl = NULL;
	static data_register *ejaddr = NULL;
	static data_register *ejdata = NULL;
	int i=0;
	int timeout=5;
	unsigned int ret;
	
	if (ejctrl == NULL)
		ejctrl = part_find_data_register( PART, "EJCONTROL" );
	if (ejaddr == NULL)
		ejaddr = part_find_data_register( PART, "EJADDRESS" );
	if (ejdata == NULL)
		ejdata = part_find_data_register( PART, "EJDATA" );

	part_set_instruction( PART, "EJTAG_ADDRESS" );
	chain_shift_instructions( CHAIN );
	for (i = 0; i < 32; i++)
		ejaddr->in->data[i] = (addr >> i) & 1;
	chain_shift_data_registers( CHAIN, 0 ); /* Push the address to read */
#ifdef PRINT_DATA_DEBUG
	printf("Wrote to ejaddr->in      =%s %08X\n",register_get_string( ejaddr->in),reg_value( ejaddr->in ) );
#endif
	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );
	register_fill( ejctrl->in, 0 );
	ejctrl->in->data[PrAcc]    = 1; // Processor access 
	ejctrl->in->data[ProbEn]   = 1;
	ejctrl->in->data[DmaAcc]   = 1; // DMA operation request */
	ejctrl->in->data[DstRt]    = 1;
	if (sz)
		ejctrl->in->data[sz]   = 1; // Size : can be WORD/HALFWORD or nothing for byte
	ejctrl->in->data[DmaRwn]   = 1; // This is a read
	chain_shift_data_registers( CHAIN, 0 ); /* Do the operation */
	//printf("Wrote to ejctrl->in      =%s %08X\n",register_get_string( ejctrl->in),reg_value( ejctrl->in ) );

	do {
		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );
		register_fill( ejctrl->in, 0 );
		ejctrl->in->data[PrAcc] = 1;
		ejctrl->in->data[ProbEn] = 1;
		ejctrl->in->data[DmaAcc] = 1;
		chain_shift_data_registers( CHAIN, 1 );
		
		//printf("Wrote to ejctrl->in   =%s %08X\n",register_get_string( ejctrl->in),reg_value( ejctrl->in ) );
		//printf("Read from ejctrl->out =%s %08X\n",register_get_string( ejctrl->out),reg_value( ejctrl->out ) );
		timeout--;
		if ( ! timeout)
			break;
	} while(ejctrl->out->data[DstRt] == 1); // This flag tell us the processor has completed the op

	part_set_instruction( PART, "EJTAG_DATA" );
	chain_shift_instructions( CHAIN );
	register_fill( ejdata->in,0);
	chain_shift_data_registers( CHAIN, 1 );
	ret = reg_value(ejdata->out);
#ifdef PRINT_DATA_DEBUG
	printf("Read from ejdata->out(%c) =%s %08X\n",siz_(sz),register_get_string( ejdata->out),reg_value( ejdata->out ) );
#endif
	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );
	register_fill( ejctrl->in, 0 );
	ejctrl->in->data[PrAcc] = 1;
	ejctrl->in->data[ProbEn] = 1;
	chain_shift_data_registers( CHAIN, 1 ); // Disable DMA, reset state to previous one.
	
//	printf("Wrote to ejctrl->in   =%s %08X\n",register_get_string( ejctrl->in),reg_value( ejctrl->in ) );
//	printf("Read from ejctrl->out =%s %08X\n",register_get_string( ejctrl->out),reg_value( ejctrl->out ) );
		
	if (ejctrl->out->data[Derr] == 1) { // Check for DMA error, i.e. incorrect address
		printf( _("%s(%d) Error on dma read (dma transaction failed)\n"),
			__FILE__, __LINE__ );
	}

	switch (sz) {
		case DMA_HALFWORD : ret &= ret & 0xffff; break;
		case DMA_BYTE     : ret &= ret & 0xff; break;
		case DMA_WORD:
		default:	break;
	}

	return ret;
}

/**
 * bus->driver->(*initbus)
 *
 */
int ejtag_dma_bus_init( bus_t *bus )
{
	data_register *ejctrl=NULL, *ejimpl=NULL, *ejaddr=NULL, *ejdata=NULL;
	int timeout=100;

	if (tap_state(CHAIN) != Run_Test_Idle) {
		/* silently skip initialization if TAP isn't in RUNTEST/IDLE state
		   this is required to avoid interfering with detect when initbus
		   is contained in the part description file
		   bus_init() will be called latest by bus_prepare() */
		return URJTAG_STATUS_OK;
	}

	ejctrl = part_find_data_register( PART, "EJCONTROL" );
	ejimpl = part_find_data_register( PART, "EJIMPCODE" );
	ejaddr = part_find_data_register( PART, "EJADDRESS" );
	ejdata = part_find_data_register( PART, "EJDATA" );
	
	if (!(ejctrl && ejimpl)) {
		printf( _("%s(%d) EJCONTROL or EJIMPCODE register not found\n"),
			__FILE__, __LINE__ );
		return URJTAG_STATUS_FAIL;
	}
	if (!(ejaddr && ejdata))  {
		printf( _("%s(%d) EJADDRESS of EJDATA register not found, DMA impossible\n"),
			__FILE__, __LINE__ );
		return URJTAG_STATUS_FAIL;
	}

	part_set_instruction( PART, "EJTAG_IMPCODE" );
	chain_shift_instructions( CHAIN );
	chain_shift_data_registers( CHAIN, 0 );
	chain_shift_data_registers( CHAIN, 1 );
	printf( "ImpCode=%s\n", register_get_string( ejimpl->out ) );
	BP->impcode = reg_value( ejimpl->out );

	switch (EJTAG_VER) {
		case EJTAG_20: printf( "EJTAG version: <= 2.0\n"); break;
		case EJTAG_25: printf( "EJTAG version: 2.5\n"); break;
		case EJTAG_26: printf( "EJTAG version: 2.6\n"); break;
	default:
		printf( "EJTAG version: unknown (%d)\n", EJTAG_VER );
	}
	printf( "EJTAG Implementation flags:%s%s%s%s%s%s%s\n",
		(BP->impcode & (1 << 28)) ? " R3k"	: " R4k",
		(BP->impcode & (1 << 24)) ? " DINTsup"	: "",
		(BP->impcode & (1 << 22)) ? " ASID_8"	: "",
		(BP->impcode & (1 << 21)) ? " ASID_6"	: "",
		(BP->impcode & (1 << 16)) ? " MIPS16"	: "",
		(BP->impcode & (1 << 14)) ? " NoDMA"	: " DMA",
		(BP->impcode & (1      )) ? " MIPS64"	: " MIPS32" );

	if (BP->impcode & (1 << 14)) {
		printf( "Warning, plateform claim there are no DMA support\n");
	}

	if (EJTAG_VER != EJTAG_20) {
		printf( "Warning, plateform has a version which is not supposed to have DMA\n");
	}

	// The purpose of this is to make the processor break into debug mode on
	// reset rather than execute the reset vector
	part_set_instruction( PART, "EJTAGBOOT" );
	chain_shift_instructions( CHAIN );
	
	// Prepare for following instructions
	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );
	register_fill( ejctrl->in, 0 );

	// Reset the processor
	ejctrl->in->data[PrRst] = 1;
	ejctrl->in->data[PerRst] = 1;
	chain_shift_data_registers( CHAIN, 0 );

	// Release reset
	ejctrl->in->data[PrRst] = 0;
	ejctrl->in->data[PerRst] = 0;
	chain_shift_data_registers( CHAIN, 0 );
	
	ejctrl->in->data[PrAcc] = 1;
	ejctrl->in->data[ProbEn] = 1;
	ejctrl->in->data[ProbTrap] = 1;
	ejctrl->in->data[JtagBrk] = 1;
	ejctrl->in->data[Rocc] = 1;
	chain_shift_data_registers( CHAIN, 0 );

	/* Wait until processor is in break */
	ejctrl->in->data[JtagBrk] = 0;
	do {
		chain_shift_data_registers( CHAIN, 1 );
		timeout--;
		if (!timeout) break; 
	} while ( ejctrl->out->data[BrkSt] == 1 );
	
	if (timeout == 0)
	{
		printf( _("%s(%d) Failed to enter debug mode, ctrl=%s\n"),
			__FILE__, __LINE__,
			register_get_string( ejctrl->out ) );
		return URJTAG_STATUS_FAIL;
	}

	// Handle the reset bit clear, if any
	if (ejctrl->out->data[Rocc]) {
		ejctrl->in->data[Rocc] = 0;
		chain_shift_data_registers( CHAIN, 0 );
		ejctrl->in->data[Rocc] = 1;
		chain_shift_data_registers( CHAIN, 1 );
	}
	
	
	// Clear Memory Protection Bit in DCR
	printf( _("Clear memory protection bit in DCR\n"));
	unsigned int val = ejtag_dma_read(bus, 0xff300000,DMA_WORD);
	ejtag_dma_write(bus, 0xff300000, val & ~(1<<2), DMA_WORD );
	
	// Clear watchdog, if any
	printf( _("Clear Watchdog\n"));
	ejtag_dma_write(bus, 0xb8000080,0,DMA_WORD);

	printf( _("Potential flash base address: [0x%x], [0x%x]\n"),
		ejtag_dma_read(bus, 0xfffe2000,DMA_WORD), ejtag_dma_read(bus, 0xfffe1000,DMA_WORD));
	
	printf( _("Processor successfully switched in debug mode.\n"));
	
	INITIALIZED = 1;
	return URJTAG_STATUS_OK;
}
/**
 * bus->driver->(*prepare)
 *
 */
void
ejtag_dma_bus_prepare( bus_t *bus )
{
	if (!INITIALIZED)
		bus_init( bus );
}

/**
 * bus->driver->(*area)
 *
 */
int
ejtag_dma_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{

	/* from MIPS.org datasheets */
	if (adr < UINT32_C(0x1E000000)) {
		area->description = "USEG : User addresses";
		area->start  = UINT32_C(0x00000000);
		area->length = UINT64_C(0x1E000000);
		area->width = 32;
	} else if (adr < UINT32_C(0x20000000)) {
		area->description = "FLASH : Addresses in flash (boot=0x1FC000000)";
		area->start  = UINT32_C(0x1E000000);
		area->length = UINT64_C(0x2000000);
		area->width = 16;
	} else if (adr < UINT32_C(0x80000000)) {
		area->description = "USEG : User addresses";
		area->start  = UINT32_C(0x20000000);
		area->length = UINT64_C(0x60000000);
		area->width = 32;
	} else if (adr < UINT32_C(0xA0000000)) {
		area->description = "KSEG0: Kernel Unmapped Cached";
		area->start  = UINT32_C(0x80000000);
		area->length = UINT64_C(0x20000000);
		area->width = 32;
	} else if (adr < UINT32_C(0xC0000000)) {
		area->description = "KSEG1: Kernel Unmapped Uncached";
		area->start  = UINT32_C(0xA0000000);
		area->length = UINT64_C(0x20000000);
		area->width = 32;
	} else if (adr < UINT32_C(0xE0000000)) {
		area->description = "SSEG : Supervisor Mapped";
		area->start  = UINT32_C(0xC0000000);
		area->length = UINT64_C(0x20000000);
		area->width = 32;
	} else {
		area->description = "KSEG3: Kernel Mapped";
		area->start  = UINT32_C(0xE0000000);
		area->length = UINT64_C(0x20000000);
		area->width = 32;
	}
	return URJTAG_STATUS_OK;
}

int get_sz(uint32_t adr)
{
	static bus_area_t area;
	static int initialized = 0;
	
	if (! initialized) {
		ejtag_dma_bus_area(NULL,adr,&area);
		initialized = 1;
	}
	switch (area.width)
	{
		case 32:	return DMA_WORD;
		case 16:	return DMA_HALFWORD;
		default:	break;
	}
	return DMA_BYTE;
}
/**
 * bus->driver->(*write)
 *
 */
void ejtag_dma_bus_write(bus_t *bus, uint32_t adr, uint32_t data)
{
	//printf("%s:adr=0x%x,data=0x%x\n",__FUNCTION__,adr,data);
	ejtag_dma_write(bus, adr,data, get_sz(adr));
}
/**
 * bus->driver->(*read)
 *
 */
unsigned int ejtag_dma_bus_read(bus_t *bus, uint32_t adr)
{
	int data = ejtag_dma_read(bus, adr, get_sz(adr));
	//printf("%s:adr=0x%x,got=0x%x\n",__FUNCTION__,adr,data);
	return data;
}

static unsigned int _data_read;
/**
 * bus->driver->(*read_start)
 *
 */
void ejtag_dma_bus_read_start(bus_t *bus, uint32_t adr)
{
	_data_read = ejtag_dma_read(bus, adr,get_sz(adr));
	//printf("%s:adr=0x%x, got=0x%x\n",__FUNCTION__,adr,_data_read);
	return;
}
/**
 * bus->driver->(*read_next)
 *
 */
unsigned int ejtag_dma_bus_read_next(bus_t *bus, uint32_t adr)
{
	unsigned int tmp_value = _data_read;
	_data_read = ejtag_dma_read(bus, adr,get_sz(adr));
	//printf("%s:adr=0x%x, got=0x%x\n",__FUNCTION__,adr,_data_read);
	return tmp_value;
}
/**
 * bus->driver->(*read_end)
 *
 */
unsigned int ejtag_dma_bus_read_end(bus_t *bus)
{
	return _data_read;
}

const bus_driver_t ejtag_dma_bus = {
	"ejtag_dma",
	N_("EJTAG compatible bus driver via DMA"),
	ejtag_dma_bus_new,
	generic_bus_free,
	ejtag_dma_bus_printinfo,
	ejtag_dma_bus_prepare,
	ejtag_dma_bus_area,
	ejtag_dma_bus_read_start,
	ejtag_dma_bus_read_next,
	ejtag_dma_bus_read_end,
	ejtag_dma_bus_read,
	ejtag_dma_bus_write,
	ejtag_dma_bus_init
};

