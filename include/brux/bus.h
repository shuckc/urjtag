/*
 * $Id$
 *
 * Bus driver interface
 * Copyright (C) 2002, 2003 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 */

#ifndef BRUX_BUS_H
#define	BRUX_BUS_H

#include <stdint.h>

typedef struct bus bus_t;

struct bus {
	void *params;
	void (*prepare)( bus_t *bus );
	int (*width)( bus_t *bus, uint32_t adr );
	void (*read_start)( bus_t *bus, uint32_t adr );
	uint32_t (*read_next)( bus_t *bus, uint32_t adr );
	uint32_t (*read_end)( bus_t *bus );
	uint32_t (*read)( bus_t *bus, uint32_t adr );
	void (*write)( bus_t *bus, uint32_t adr, uint32_t data );
	void (*free)( bus_t *bus );
};

#define	bus_prepare(bus)	bus->prepare(bus)
#define	bus_width(bus,adr)	bus->width(bus,adr)
#define	bus_read_start(bus,adr)	bus->read_start(bus,adr)
#define	bus_read_next(bus,adr)	bus->read_next(bus,adr)
#define	bus_read_end(bus)	bus->read_end(bus)
#define	bus_read(bus,adr)	bus->read(bus,adr)
#define	bus_write(bus,adr,data)	bus->write(bus,adr,data)
#define	bus_free(bus)		bus->free(bus)

#endif /* BRUX_BUS_H */
