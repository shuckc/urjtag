/*
 * $Id: vision_ep9307.c $
 *
 * Vision EP9307 SoM GPIO JTAG Cable Driver
 * Copyright (C) 2007, 2008 H Hartley Sweeten
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
 * Written by H Hartley Sweeten <hsweeten@visionengravers.com>, 2007, 2008.
 *
 */

#include "sysdep.h"

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "cable.h"
#include "chain.h"

#include "generic.h"

#include <cmd.h>

#define SYSCON_BASE		0x80930000
#define SYSCON_DEVICE_CONFIG	0x80
#define SYSCON_SWLOCK		0xC0

#define SYSCON_DEVCFG_HonIDE	(1 << 11)

#define GPIO_BASE		0x80840000
#define GPIO_PHDR		0x40
#define GPIO_PHDDR		0x44

#define TDO			4
#define TDI			6
#define TMS			5
#define TCK			7
#define TRST			3

#define HGPIO(b)		(1 << (b))
#define EP9307_TDO		HGPIO(TDO)
#define EP9307_TDI		HGPIO(TDI)
#define EP9307_TMS		HGPIO(TMS)
#define EP9307_TCK		HGPIO(TCK)
#define EP9307_TRST		HGPIO(TRST)

#define GPIO_INPUT_MASK		((EP9307_TCK)|(EP9307_TMS)|(EP9307_TDI)|(EP9307_TRST))
#define GPIO_OUTPUT_MASK	(~(EP9307_TDO))
#define GPIO_BITMASK		(~((EP9307_TDO)|(EP9307_TDI)|(EP9307_TMS)|(EP9307_TCK)|(EP9307_TRST)))

typedef struct {
	int		fd_dev_mem;
	void		*map_base;
	size_t		map_size;
	uint32_t	*gpio_PHDR;
	int		trst;
} ep9307_params_t;

static int
ep9307_gpio_open( cable_t *cable )
{
	ep9307_params_t *p = cable->params;
	off_t map_mask;
	uint32_t *syscon_devcfg;
	uint32_t *syscon_sysswlock;
	uint32_t *gpio_PHDDR;
	uint32_t tmp;

	/* Open the memory device so we can access the hardware registers */
	p->fd_dev_mem = open("/dev/mem", O_RDWR | O_SYNC);
	if (p->fd_dev_mem == -1) {
		printf( _("Error: unable to open /dev/mem\n") );
		return -1;
	}

	p->map_size = getpagesize();
	map_mask = p->map_size - 1;

	/* Map the System Controller registers */
	p->map_base = mmap(0, p->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, p->fd_dev_mem, SYSCON_BASE & ~map_mask);
	if (p->map_base == MAP_FAILED) {
		printf( _("Error: unable to mmap the System Control registers\n") );
		close (p->fd_dev_mem);
		return -1;
	}

	/* Create the pointers to access the DeviceCfg and SysSWLock registers */
	syscon_devcfg = (uint32_t*)(p->map_base + SYSCON_DEVICE_CONFIG);
	syscon_sysswlock = (uint32_t*)(p->map_base + SYSCON_SWLOCK);

	/* Set the HonIDE bit in the DeviceCfg register so we can use Port H as GPIO */
	tmp = *((uint32_t*)syscon_devcfg);
	tmp |= SYSCON_DEVCFG_HonIDE;

	/* The DeviceCfg register has a SoftwareLock; unlock it first */
	*((uint32_t*)syscon_sysswlock) = 0xAA;
	*((uint32_t*)syscon_devcfg) = tmp;

	/* Unmap the System Controller registers */
	if (munmap(p->map_base, p->map_size) == -1) {
		printf( _("Error: unable to munmap the System Controller registers\n"));
		close (p->fd_dev_mem);
		return -1;
	}

	/* Map the GPIO registers */
	p->map_base = mmap(0, p->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, p->fd_dev_mem, GPIO_BASE & ~map_mask);
	if (p->map_base == MAP_FAILED) {
		printf( _("Error: unable to mmap the GPIO registers\n") );
		close (p->fd_dev_mem);
		return -1;
	}

	/* Create the pointers to access the PHDR and PHDDR registers */
	p->gpio_PHDR = (uint32_t*)(p->map_base + GPIO_PHDR);
	gpio_PHDDR = (uint32_t*)(p->map_base + GPIO_PHDDR);

	/* Set the GPIO pins as inputs/outputs as needed for the JTAG interface */
	tmp = *((uint32_t*)gpio_PHDDR);
	tmp |= GPIO_INPUT_MASK;
	tmp &= GPIO_OUTPUT_MASK;
	*((uint32_t*)gpio_PHDDR) = tmp;

	return 0;
}

static int
ep9307_gpio_close( cable_t *cable )
{
	ep9307_params_t *p = cable->params;

	/* Unmap the GPIO registers */
	if (munmap(p->map_base, p->map_size) == -1) {
		printf( _("Error: unable to munmap the GPIO registers\n"));
	}
	close(p->fd_dev_mem);
	return 0;
}

static int
ep9307_gpio_write( cable_t *cable, uint8_t data )
{
	ep9307_params_t *p = cable->params;
	uint32_t tmp;

	tmp = *((uint32_t*)p->gpio_PHDR);
	tmp &= ~GPIO_OUTPUT_MASK;
	tmp |= data;
	*((uint32_t*)p->gpio_PHDR) = tmp;

	return 0;
}

static int
ep9307_gpio_read( cable_t *cable )
{
	ep9307_params_t *p = cable->params;
	uint32_t tmp;

	tmp = *((uint32_t*)p->gpio_PHDR);

	return tmp;
}

static int
ep9307_connect( char *params[], cable_t *cable )
{
	ep9307_params_t *cable_params;

	if ( cmd_params( params ) != 1) {
	  printf( _("Error: This cable type does not accept parameters!\n") );
	  return 1;
	}

	printf( _("Initializing Vision EP9307 SoM GPIO JTAG Cable\n") );

	cable_params = malloc( sizeof *cable_params );
	if (!cable_params) {
		printf( _("%s(%d) malloc failed!\n"), __FILE__, __LINE__);
		return 4;
	}

	cable->link.port = NULL;
	cable->params = cable_params;
	cable->chain = NULL;

	return 0;
}

static void
ep9307_cable_free( cable_t *cable )
{
	free( cable->params );
	free( cable );
}

static int
ep9307_init( cable_t *cable )
{
	ep9307_params_t *p = cable->params;

	if (ep9307_gpio_open( cable ))
		return -1;

	ep9307_gpio_write( cable, 1 << TRST );
	cable_wait( cable );
	p->trst = 1;

	return 0;
}

static void
ep9307_done( cable_t *cable )
{
	ep9307_gpio_close( cable );
}

static void
ep9307_clock( cable_t *cable, int tms, int tdi, int n )
{
	ep9307_params_t *p = cable->params;
	int bit_mask;
	int i;

	tms = tms ? 1 : 0;
	tdi = tdi ? 1 : 0;

	bit_mask = (tms << TMS) | (tdi << TDI) | (p->trst << TRST);

	for (i = 0; i < n; i++) {
		ep9307_gpio_write( cable, (0 << TCK) | bit_mask );
		cable_wait( cable );
		ep9307_gpio_write( cable, (1 << TCK) | bit_mask );
		cable_wait( cable );
	}
}

/**
 * NOTE: This also lowers the TDI and TMS lines; is this intended?
 */
static int
ep9307_get_tdo( cable_t *cable )
{
	ep9307_params_t *p = cable->params;

	ep9307_gpio_write( cable, (0 << TCK) | (p->trst << TRST) );
	cable_wait( cable );
	return (ep9307_gpio_read( cable ) >> TDO) & 1;
}

/**
 * NOTE: This also lowers the TCK, TDI, and TMS lines; is this intended?
 */
static int
ep9307_set_trst( cable_t *cable, int trst )
{
	ep9307_params_t *p = cable->params;

	p->trst = trst ? 1 : 0;

	ep9307_gpio_write( cable, p->trst << TRST );
	cable_wait( cable );
	return p->trst;
}

static int
ep9307_get_trst( cable_t *cable )
{
	return (ep9307_gpio_read( cable ) >> TRST) & 1;
}

static void
ep9307_help( const char *cablename )
{
	printf( _(
		"Usage: cable %s\n"
		"\n"
	), cablename );
}

cable_driver_t ep9307_cable_driver = {
	"EP9307",
	N_("Vision EP9307 SoM GPIO JTAG Cable"),
	ep9307_connect,
	generic_disconnect,
	ep9307_cable_free,
	ep9307_init,
	ep9307_done,
	generic_set_frequency,
	ep9307_clock,
	ep9307_get_tdo,
	generic_transfer,
	ep9307_set_trst,
	ep9307_get_trst,
	generic_flush_one_by_one,
	ep9307_help
};
