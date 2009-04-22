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

#ifndef URJ_BUS_DRIVER_BRUX_BUS_H
#define	URJ_BUS_DRIVER_BRUX_BUS_H

#include <stdint.h>

#include "chain.h"

typedef struct
{
    const char *description;
    uint32_t start;
    uint64_t length;
    unsigned int width;
} urj_bus_area_t;

typedef struct urj_bus urj_bus_t;
typedef struct urj_bus_driver urj_bus_driver_t;

struct urj_bus_driver
{
    const char *name;
    const char *description;
    urj_bus_t *(*new_bus) (urj_chain_t *chain, const urj_bus_driver_t *driver,
                       char *cmd_params[]);
    void (*free_bus) (urj_bus_t *bus);
    void (*printinfo) (urj_bus_t *bus);
    void (*prepare) (urj_bus_t *bus);
    int (*area) (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area);
    void (*read_start) (urj_bus_t *bus, uint32_t adr);
    uint32_t (*read_next) (urj_bus_t *bus, uint32_t adr);
    uint32_t (*read_end) (urj_bus_t *bus);
    uint32_t (*read) (urj_bus_t *bus, uint32_t adr);
    void (*write) (urj_bus_t *bus, uint32_t adr, uint32_t data);
    int (*init) (urj_bus_t *bus);
};

struct urj_bus
{
    urj_chain_t *chain;
    urj_part_t *part;
    void *params;
    int initialized;
    const urj_bus_driver_t *driver;
};

extern urj_bus_t *bus;

#define	URJ_BUS_PRINTINFO(bus)	(bus)->driver->printinfo(bus)
#define	URJ_BUS_PREPARE(bus)	(bus)->driver->prepare(bus)
#define	URJ_BUS_AREA(bus,adr,a)	(bus)->driver->area(bus,adr,a)
#define	URJ_BUS_READ_START(bus,adr)	(bus)->driver->read_start(bus,adr)
#define	URJ_BUS_READ_NEXT(bus,adr)	(bus)->driver->read_next(bus,adr)
#define	URJ_BUS_READ_END(bus)	(bus)->driver->read_end(bus)
#define	URJ_BUS_READ(bus,adr)	(bus)->driver->read(bus,adr)
#define	URJ_BUS_WRITE(bus,adr,data)	(bus)->driver->write(bus,adr,data)
#define	URJ_BUS_FREE(bus)		(bus)->driver->free_bus(bus)
#define	URJ_BUS_INIT(bus)		(bus)->driver->init(bus)

#endif /* URJ_BUS_DRIVER_BRUX_BUS_H */
