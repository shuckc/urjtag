/*
 * $Id$
 *
 * EJTAG compatible bus driver via PrAcc
 * Copyright (C) 2005, Marek Michalkiewicz
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
 */

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
	uint16_t adr_hi;   /* cached high bits of $3 */
} bus_params_t;

#define BP ((bus_params_t *) bus->params)

#define EJTAG_VER ((BP->impcode >> 29) & 7)

#define EJTAG_20	0
#define EJTAG_25	1
#define EJTAG_26	2
#define EJTAG_31	3

/* EJTAG 3.1 Control Register Bits */
#define VPED		23	/* R	*/
/* EJTAG 2.6 Control Register Bits */
#define Rocc		31	/* R/W0 */
#define Psz1		30	/* R    */
#define Psz0		29	/* R    */
#define Doze		22	/* R    */
#define ProbTrap	14	/* R/W  */
#define DebugMode	 3	/* R	*/
/* EJTAG 1.5.3 Control Register Bits */
#define Dnm		28 /* */
#define Sync		23	/* R/W  */
#define Run		21	/* R    */
#define PerRst		20	/* R/W  */
#define PRnW		19	/* R    0 = Read, 1 = Write */
#define PrAcc		18	/* R/W0 */
#define DmaAcc		17	/* R/W  */
#define PrRst		16	/* R/W  */
#define ProbEn		15	/* R/W  */
#define SetDev		14	/* R    */
#define JtagBrk		12	/* R/W1 */
#define DStrt		11	/* R/W1 */
#define DeRR		10	/* R    */
#define DrWn		 9	/* R/W  */
#define Dsz1		 8	/* R/W  */
#define Dsz0		 7	/* R/W  */
#define DLock		 5	/* R/W  */
#define BrkSt		 3	/* R    */
#define TIF		 2	/* W0/R	*/
#define TOF		 1	/* W0/R */
#define ClkEn		 0	/* R/W  */

/* EJTAG 3.1 Debug Control Register at drseg 0xFF300000 */
#define PCS			 9	/* R	*/
#define PCR2		 8	/* R/W	*/
#define PCR1		 7	/* R/W	*/
#define PCR0		 6	/* R/W	*/
/* EJTAG 2.X Debug Control Register at drseg 0xFF300000 */
#define	DataBrk		17	/* R	*/
#define	InstBrk		16	/* R	*/
#define	NMIPend		 2	/* R	*/
#define	SRstE		 1	/* R/W	*/
#define	DCRProbeEn	 0	/* R	*/
/* EJTAG 1.5.3 Debug Control Register at drseg 0xFF300000*/
#define HIS			30	/* R	*/
#define ENM			29	/* R 0=Little End,1=Big Endian */
#define MIntE		4	/* R/W	*/
#define MNmiE		3	/* R/W	*/
#define MemProt		2	/* R/W 0=WriteOK,1=Protected */
#define MRst		1	/* R/W	*/
#define TraceMode	0	/* R/W	*/

/**
 * bus->driver->(*new_bus)
 *
 */
static bus_t *
ejtag_bus_new( chain_t *chain, const bus_driver_t *driver, char *cmd_params[] )
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
ejtag_bus_printinfo( bus_t *bus )
{
	int i;

	for (i = 0; i < CHAIN->parts->len; i++)
		if (PART == CHAIN->parts->parts[i])
			break;
	printf( _("EJTAG compatible bus driver via PrAcc (JTAG part No. %d)\n"), i );
}

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

static uint32_t
ejtag_run_pracc( bus_t *bus, const uint32_t *code, unsigned int len )
{
	data_register *ejaddr, *ejdata, *ejctrl;
	int i, pass;
	uint32_t addr, data, retval;

	ejaddr = part_find_data_register( PART, "EJADDRESS" );
	ejdata = part_find_data_register( PART, "EJDATA" );
	ejctrl = part_find_data_register( PART, "EJCONTROL" );
	if (!(ejaddr && ejdata && ejctrl)) {
		printf( _("%s(%d) EJADDRESS, EJDATA or EJCONTROL register not found\n"),
			__FILE__, __LINE__ );
		return 0;
	}

	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );

	pass = 0;
	retval = 0;

	for (;;) {
		ejctrl->in->data[PrAcc] = 1;
		chain_shift_data_registers( CHAIN, 0 );
		chain_shift_data_registers( CHAIN, 1 );

//		printf( "ctrl=%s\n", register_get_string( ejctrl->out ) );

		if (ejctrl->out->data[Rocc]) {
			printf( _("%s(%d) Reset occurred, ctrl=%s\n"),
				__FILE__, __LINE__,
				register_get_string( ejctrl->out ) );
			INITIALIZED = 0;
			break;
		}
		if (! ejctrl->out->data[PrAcc]) {
			printf( _("%s(%d) No processor access, ctrl=%s\n"),
				__FILE__, __LINE__,
				register_get_string( ejctrl->out ) );
			INITIALIZED = 0;
			break;
		}

		part_set_instruction( PART, "EJTAG_ADDRESS" );
		chain_shift_instructions( CHAIN );

		chain_shift_data_registers( CHAIN, 1 );
		addr = reg_value( ejaddr->out );
		if (addr & 3) {
			printf( _("%s(%d) PrAcc bad alignment: addr=0x%08x\n"),
				__FILE__, __LINE__, addr );
			addr &= ~3;
		}

		part_set_instruction( PART, "EJTAG_DATA" );
		chain_shift_instructions( CHAIN );

		register_fill( ejdata->in, 0 );

		if (ejctrl->out->data[PRnW]) {
			chain_shift_data_registers( CHAIN, 1 );
			data = reg_value( ejdata->out );
#if 0
			printf( _("%s(%d) PrAcc write: addr=0x%08x data=0x%08x\n"),
				__FILE__, __LINE__, addr, data );
#endif
			if (addr == UINT32_C(0xff200000)) {
				/* Return value from the target CPU.  */
				retval = data;
			} else {
				printf( _("%s(%d) Unknown write addr=0x%08x data=0x%08x\n"),
					__FILE__, __LINE__, addr, data );
			}
		} else {
			if (addr == UINT32_C(0xff200200) && pass++)
				break;

			data = 0;
			if (addr >= 0xff200200 && addr < 0xff200200 + (len << 2)) {
				data = code[(addr - 0xff200200) >> 2];

				for (i = 0; i < 32; i++)
					ejdata->in->data[i] = (data >> i) & 1;
			}
#if 0
			printf( "%s(%d) PrAcc read: addr=0x%08x data=0x%08x\n",
				__FILE__, __LINE__, addr, data );
#endif
			chain_shift_data_registers( CHAIN, 0 );
		}

		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );

		ejctrl->in->data[PrAcc] = 0;
		chain_shift_data_registers( CHAIN, 0 );
	}
	return retval;
}

static int
ejtag_bus_init( bus_t *bus )
{
	data_register *ejctrl, *ejimpl, *ejaddr, *ejdata, *ejall;
	uint32_t code[4] = {
		0x3c04ff20,				// lui $4,0xff20
		0x349f0200,				// ori $31,$4,0x0200
		0x03e00008,				// jr $31
		0x3c030000				// lui $3,0
	};

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
	ejdata = part_find_data_register( PART, "EJDATA"    );
	ejall  = part_find_data_register( PART, "EJALL"     );
	if (!(ejctrl && ejimpl)) {
		printf( _("%s(%d) EJCONTROL or EJIMPCODE register not found\n"),
			__FILE__, __LINE__ );
		return URJTAG_STATUS_FAIL;
	}

	part_set_instruction( PART, "EJTAG_IMPCODE" );
	chain_shift_instructions( CHAIN );
	chain_shift_data_registers( CHAIN, 0 );//Write
	chain_shift_data_registers( CHAIN, 1 );//Read
	printf( "ImpCode=%s %08X\n", register_get_string( ejimpl->out ), reg_value( ejimpl->out ) );
	BP->impcode = reg_value( ejimpl->out );

	switch (EJTAG_VER) {
	case EJTAG_20: printf( "EJTAG version: <= 2.0\n"); break;
	case EJTAG_25: printf( "EJTAG version: 2.5\n"); break;
	case EJTAG_26: printf( "EJTAG version: 2.6\n"); break;
	case EJTAG_31: printf( "EJTAG version: 3.1\n"); break;
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

	if (EJTAG_VER >= EJTAG_25) {
		part_set_instruction( PART, "EJTAGBOOT" );
		chain_shift_instructions( CHAIN );
	}
	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );
	//Reset
	register_fill( ejctrl->in, 0 );
	ejctrl->in->data[PrRst] = 1;
	ejctrl->in->data[PerRst] = 1;
	chain_shift_data_registers( CHAIN, 0 );//Write
	ejctrl->in->data[PrRst] = 0;
	ejctrl->in->data[PerRst] = 0;
	chain_shift_data_registers( CHAIN, 0 );//Write
//
	if (EJTAG_VER == EJTAG_20)
	{
		// Try enabling memory write on EJTAG_20 (BCM6348)
		// Badly Copied from HairyDairyMaid V4.8
		//ejtag_dma_write(0xff300000, (ejtag_dma_read(0xff300000) & ~(1<<2)) );
//		printf("Set Address to READ from\n");
//		printf("Select EJTAG ADDRESS Register\n");
		part_set_instruction( PART, "EJTAG_ADDRESS" );
		chain_shift_instructions ( CHAIN );
		//Set to Debug Control Register Address, 0xFF300000
		register_init( ejaddr->in, "11111111001100000000000000000000");
//		printf("Write to ejaddr->in     =%s %08X\n",register_get_string( ejaddr->in),reg_value( ejaddr->in ) );
		chain_shift_data_registers (CHAIN, 0);//Write
//		printf("Select EJTAG CONTROL Register\n");
		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );
		//Set some bits in CONTROL Register 0x00068B00
		register_fill( ejctrl->in, 0 ); // Clear Register
		ejctrl->in->data[PrAcc]    = 1; // 18----|||
		ejctrl->in->data[DmaAcc]   = 1; // 17----|||
		ejctrl->in->data[ProbEn]   = 1; // 15-----||
		ejctrl->in->data[DStrt]    = 1; // 11------|
		ejctrl->in->data[DrWn]     = 1; // 9-------|
		ejctrl->in->data[Dsz1]     = 1; // 8-------| DMA_WORD = 0x00000100 = Bit8
		chain_shift_data_registers( CHAIN, 1 );//WriteRead
//		printf("Write To ejctrl->in     =%s %08X\n",register_get_string( ejctrl->in), reg_value( ejctrl->in ) );
//		printf("Read From ejctrl->out   =%s %08X\n",register_get_string( ejctrl->out),reg_value( ejctrl->out ) );
		do {
//		    printf("Wait for DStrt to clear\n");
		    part_set_instruction( PART, "EJTAG_CONTROL" );
		    chain_shift_instructions( CHAIN );
		    register_fill( ejctrl->in, 0 );
		    //Set some bits in CONTROL Register 0x00068000
		    ejctrl->in->data[PrAcc]    = 1; // 18----||
		    ejctrl->in->data[DmaAcc]   = 1; // 17----||
		    ejctrl->in->data[ProbEn]   = 1; // 15-----|
		    chain_shift_data_registers( CHAIN, 1 );//WriteRead
//			printf("Write To ejctrl->in     =%s %08X\n",register_get_string( ejctrl->in), reg_value( ejctrl->in ) );
//			printf("Read From ejctrl->out   =%s %08X\n",register_get_string( ejctrl->out),reg_value( ejctrl->out ) );
		} while ( ejctrl->out->data[DStrt]==1 );
//		printf("Select EJTAG DATA Register\n");
		part_set_instruction( PART, "EJTAG_DATA" );
		chain_shift_instructions (CHAIN );
		register_fill( ejdata->in, 0 ); // Clear Register
		chain_shift_data_registers( CHAIN, 1 );//WriteRead
//		printf( "Write To ejdata->in    =%s %08X\n", register_get_string( ejdata->in), reg_value( ejdata->in ) );
//		printf( "Read From ejdata->out  =%s %08X\n", register_get_string( ejdata->out),reg_value( ejdata->out ) );
//		printf("Select EJTAG CONTROL Register\n");
		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );
		register_fill( ejctrl->in, 0 );
		//Set some bits in CONTROL Register 0x00048000
		ejctrl->in->data[PrAcc]    = 1; // 18----||
		ejctrl->in->data[ProbEn]   = 1; // 15-----|
		chain_shift_data_registers( CHAIN, 1 );//WriteRead
//		printf("Write To ejctrl->in     =%s %08X\n",register_get_string( ejctrl->in), reg_value( ejctrl->in ) );
//		printf("Read From ejctrl->out   =%s %08X\n",register_get_string( ejctrl->out),reg_value( ejctrl->out ) );
		if (ejctrl->out->data[DeRR]==1)
		{
			printf("DMA READ ERROR\n");
		}
		//Now have data from DCR, need to reset the MP Bit (2) and write it back out
		register_init( ejdata->in, register_get_string( ejdata->out ) );
		ejdata->in->data[MemProt] = 0;
//		printf( "Need to Write ejdata-> =%s %08X\n", register_get_string( ejdata->in),reg_value( ejdata->in ) );

	// Now the Write
//		printf("Set Address To Write To\n");
//		printf("Select EJTAG ADDRESS Register\n");
		part_set_instruction( PART, "EJTAG_ADDRESS" );
		chain_shift_instructions ( CHAIN );
		register_init( ejaddr->in, "11111111001100000000000000000000" );
//		printf("Write to ejaddr->in     =%s %08X\n",register_get_string( ejaddr->in), reg_value( ejaddr->in ) );
		//This appears to be a write with NO Read
		chain_shift_data_registers ( CHAIN, 0 );//Write
//		printf("Select EJTAG DATA Register\n");
		part_set_instruction( PART, "EJTAG_DATA" );
		chain_shift_instructions ( CHAIN );
		//The value is already in ejdata->in, so write it
//		printf("Write To ejdata->in     =%s %08X\n", register_get_string( ejdata->in),reg_value( ejdata->in ) );
		chain_shift_data_registers( CHAIN, 0 );//Write
//		printf("Select EJTAG CONTROL Register\n");
		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );

		//Set some bits in CONTROL Register
		register_fill( ejctrl->in, 0 ); // Clear Register
		ejctrl->in->data[DmaAcc]   = 1; // 17
		ejctrl->in->data[Dsz1]     = 1; // DMA_WORD = 0x00000100 = Bit8
		ejctrl->in->data[DStrt]    = 1; // 11
		ejctrl->in->data[ProbEn]   = 1; // 15
		ejctrl->in->data[PrAcc]    = 1; // 18
		chain_shift_data_registers( CHAIN, 1 );//Write/Read
//		printf("Write to ejctrl->in     =%s %08X\n",register_get_string( ejctrl->in),  reg_value( ejctrl->in ) );
//		printf("Read from ejctrl->out   =%s %08X\n",register_get_string( ejctrl->out), reg_value( ejctrl->out ) );
		do {
//		    printf("Wait for DStrt to clear\n");
		    //Might not need these 2 lines
		    part_set_instruction( PART, "EJTAG_CONTROL" );
		    chain_shift_instructions( CHAIN );
		    ejctrl->in->data[DmaAcc]   = 1; // 17
		    ejctrl->in->data[ProbEn]   = 1; // 15
		    ejctrl->in->data[PrAcc]    = 1; // 18
		    chain_shift_data_registers( CHAIN, 1 );//Write/Read
//		    printf("Write to ejctrl->in     =%s %08X\n",register_get_string( ejctrl->in),  reg_value( ejctrl->in ) );
//		    printf("Read from ejctrl->out   =%s %08X\n",register_get_string( ejctrl->out), reg_value( ejctrl->out ) );
		} while ( ejctrl->out->data[DStrt]==1 );
//		printf("Select EJTAG CONTROL Register\n");
		part_set_instruction( PART, "EJTAG_CONTROL" );
		chain_shift_instructions( CHAIN );
		register_fill( ejctrl->in, 0 );
		//Set some bits in CONTROL Register 0x00048000
		ejctrl->in->data[PrAcc]    = 1; // 18----||
		ejctrl->in->data[ProbEn]   = 1; // 15-----|
		chain_shift_data_registers( CHAIN, 1 );//Write/Read
//		printf("Write To ejctrl->in     =%s %08X\n",register_get_string( ejctrl->in),reg_value( ejctrl->in ) );
//		printf("Read From ejctrl->out   =%s %08X\n",register_get_string( ejctrl->out),reg_value( ejctrl->out ) );
		if ( ejctrl->out->data[DeRR]==1 )
		{
			printf("DMA WRITE ERROR\n");
		}
	}

	part_set_instruction( PART, "EJTAG_CONTROL" );
	chain_shift_instructions( CHAIN );

	register_fill( ejctrl->in, 0 );
	ejctrl->in->data[PrAcc] = 1;
	ejctrl->in->data[ProbEn] = 1;
	if (EJTAG_VER >= EJTAG_25) {
		ejctrl->in->data[ProbTrap] = 1;
		ejctrl->in->data[Rocc] = 1;
	}
	chain_shift_data_registers( CHAIN, 0 );

	ejctrl->in->data[PrAcc] = 1;
	ejctrl->in->data[ProbEn] = 1;
	ejctrl->in->data[ProbTrap] = 1;
	ejctrl->in->data[JtagBrk] = 1;

	chain_shift_data_registers( CHAIN, 0 );

	ejctrl->in->data[JtagBrk] = 0;
	chain_shift_data_registers( CHAIN, 1 );

	if (! ejctrl->out->data[BrkSt]) {
		printf( _("%s(%d) Failed to enter debug mode, ctrl=%s\n"),
			__FILE__, __LINE__,
			register_get_string( ejctrl->out ) );
		return URJTAG_STATUS_FAIL;
	}
	else
	{
		printf("Processor entered Debug Mode.\n");
	}
	if (ejctrl->out->data[Rocc]) {
		ejctrl->in->data[Rocc] = 0;
		chain_shift_data_registers( CHAIN, 0 );
		ejctrl->in->data[Rocc] = 1;
		chain_shift_data_registers( CHAIN, 1 );
	}

	//HDM now Clears Watchdog


	ejtag_run_pracc( bus, code, 4 );
	BP->adr_hi = 0;
	INITIALIZED = 1;
	return URJTAG_STATUS_OK;
}

/**
 * bus->driver->(*prepare)
 *
 */
static void
ejtag_bus_prepare( bus_t *bus )
{
	if (!INITIALIZED)
		bus_init( bus );
}

/**
 * bus->driver->(*area)
 *
 */
static int
ejtag_bus_area( bus_t *bus, uint32_t adr, bus_area_t *area )
{
	if (adr < UINT32_C(0x20000000)) {
		area->description = NULL;
		area->start  = UINT32_C(0x00000000);
		area->length = UINT64_C(0x20000000);
		area->width = 8;
	} else if (adr < UINT32_C(0x40000000)) {
		area->description = NULL;
		area->start  = UINT32_C(0x20000000);
		area->length = UINT64_C(0x20000000);
		area->width = 16;
	} else if (adr < UINT32_C(0x60000000)) {
		area->description = NULL;
		area->start  = UINT32_C(0x40000000);
		area->length = UINT64_C(0x20000000);
		area->width = 32;
	} else {
		area->description = NULL;
		area->start  = UINT32_C(0x60000000);
		area->length = UINT64_C(0xa0000000);
		area->width = 0;
	}
	return URJTAG_STATUS_OK;
}

static int
ejtag_gen_read( uint32_t *code, uint32_t adr )
{
	uint16_t adr_hi, adr_lo;
	uint32_t *p = code;

	/* 16-bit signed offset, phys -> kseg1 */
	adr_lo = adr & 0xffff;
	adr_hi = ((adr >> 16) & 0x1fff);
	/* Increment adr_hi if adr_lo < 0 */
	adr_hi += (adr_lo >> 15);
	/* Bypass cache */
	adr_hi += 0xa000;

	if (BP->adr_hi != adr_hi) {
		BP->adr_hi = adr_hi;
		*p++ = 0x3c030000 | adr_hi;		// lui $3,adr_hi
	}
	switch (adr >> 29) {
	case 0:
		*p++ = 0x90620000 | adr_lo;		// lbu $2,adr_lo($3)
		break;
	case 1:
		*p++ = 0x94620000 | (adr_lo & ~1);	// lhu $2,adr_lo($3)
		break;
	case 2:
		*p++ = 0x8c620000 | (adr_lo & ~3);	// lw $2,adr_lo($3)
		break;
	default:  /* unknown bus width */
		*p++ = 0x00001025;			// move $2,$0
		break;
	}
	*p++ = 0x03e00008;				// jr $31
	return p - code;
}

/**
 * bus->driver->(*read_start)
 *
 */
static void
ejtag_bus_read_start( bus_t *bus, uint32_t adr )
{
	uint32_t code[3];

	ejtag_run_pracc( bus, code, ejtag_gen_read( code, adr ));
	// printf("bus_read_start: adr=0x%08x\n", adr);
}

/**
 * bus->driver->(*read_next)
 *
 */
static uint32_t
ejtag_bus_read_next( bus_t *bus, uint32_t adr )
{
	uint32_t d;
	uint32_t code[4], *p = code;

	*p++ = 0xac820000;				// sw $2,0($4)
	p += ejtag_gen_read( p, adr );

	d = ejtag_run_pracc( bus, code, p - code );

	// printf("bus_read_next: adr=0x%08x data=0x%08x\n", adr, d);
	return d;
}

/**
 * bus->driver->(*read_end)
 *
 */
static uint32_t
ejtag_bus_read_end( bus_t *bus )
{
	uint32_t d;
	static const uint32_t code[2] = {
		0xac820000,				// sw $2,0($4)
		0x03e00008				// jr $31
	};

	d = ejtag_run_pracc( bus, code, 2 );

	// printf("bus_read_end: data=0x%08x\n", d);
	return d;
}

/**
 * bus->driver->(*write)
 *
 */
static void
ejtag_bus_write( bus_t *bus, uint32_t adr, uint32_t data )
{
	uint16_t adr_hi, adr_lo;
	uint32_t code[5], *p = code;

	/* 16-bit signed offset, phys -> kseg1 */
	adr_lo = adr & 0xffff;
	adr_hi = ((adr >> 16) & 0x1fff) + (adr_lo >> 15) + 0xa000;

	if (BP->adr_hi != adr_hi) {
		BP->adr_hi = adr_hi;
		*p++ = 0x3c030000 | adr_hi;		// lui $3,adr_hi
	}
	switch (adr >> 29) {
	case 0:
		*p++ = 0x34020000 | (data & 0xff);	// li $2,data
		*p++ = 0xa0620000 | adr_lo;		// sb $2,adr_lo($3)
		break;
	case 1:
		*p++ = 0x34020000 | (data & 0xffff);	// li $2,data
		*p++ = 0xa4620000 | (adr_lo & ~1);	// sh $2,adr_lo($3)
		break;
	case 2:
		*p++ = 0x3c020000 | (data >> 16);	// lui $2,data_hi
		*p++ = 0x34420000 | (data & 0xffff);	// ori $2,data_lo
		*p++ = 0xac620000 | (adr_lo & ~3);	// sw $2,adr_lo($3)
		break;
	}
	*p++ = 0x03e00008;				// jr $31

	ejtag_run_pracc( bus, code, p - code );

	// printf("bus_write: adr=0x%08x data=0x%08x\n", adr, data);
}

const bus_driver_t ejtag_bus = {
	"ejtag",
	N_("EJTAG compatible bus driver via PrAcc"),
	ejtag_bus_new,
	generic_bus_free,
	ejtag_bus_printinfo,
	ejtag_bus_prepare,
	ejtag_bus_area,
	ejtag_bus_read_start,
	ejtag_bus_read_next,
	ejtag_bus_read_end,
	generic_bus_read,
	ejtag_bus_write,
	ejtag_bus_init
};
