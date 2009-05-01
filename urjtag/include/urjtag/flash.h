/*
 * $Id: flash.h 1537 2009-04-27 12:59:18Z rfhh $
 *
 * Copyright (C) 2003 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#ifndef URJ_FLASH_H
#define URJ_FLASH_H

#include <stdio.h>
#include <stdint.h>

#include "types.h"

#if 0
/* Following moved here from brux/cfi.h */

#include <flash/cfi.h>

typedef struct
{
    int width;                  /* 1 for 8 bits, 2 for 16 bits, 4 for 32 bits, etc. */
    urj_flash_cfi_query_structure_t cfi;
} urj_flash_cfi_chip_t;
#endif

typedef struct urj_flash_cfi_chip urj_flash_cfi_chip_t;

typedef struct
{
    urj_bus_t *bus;
    uint32_t address;
    int bus_width;              /* in cfi_chips, e.g. 4 for 32 bits */
    urj_flash_cfi_chip_t **cfi_chips;
} urj_flash_cfi_array_t;

void urj_flash_cfi_array_free (urj_flash_cfi_array_t *urj_flash_cfi_array);
int urj_flash_cfi_detect (urj_bus_t *bus, uint32_t adr,
                          urj_flash_cfi_array_t **urj_flash_cfi_array);

/* End of brux/cfi.h */

typedef struct
{
    unsigned int bus_width;     /* 1 for 8 bits, 2 for 16 bits, 4 for 32 bits, etc. */
    const char *name;
    const char *description;
    int (*autodetect) (urj_flash_cfi_array_t *urj_flash_cfi_array);
    void (*print_info) (urj_flash_cfi_array_t *urj_flash_cfi_array);
    int (*erase_block) (urj_flash_cfi_array_t *urj_flash_cfi_array,
                        uint32_t adr);
    int (*unlock_block) (urj_flash_cfi_array_t *urj_flash_cfi_array,
                         uint32_t adr);
    int (*program) (urj_flash_cfi_array_t *urj_flash_cfi_array, uint32_t adr,
                    uint32_t *buffer, int count);
    void (*readarray) (urj_flash_cfi_array_t *urj_flash_cfi_array);
} urj_flash_driver_t;

#define URJ_FLASH_ERROR_NOERROR                         0
#define URJ_FLASH_ERROR_INVALID_COMMAND_SEQUENCE        1
#define URJ_FLASH_ERROR_LOW_VPEN                        2
#define URJ_FLASH_ERROR_BLOCK_LOCKED                    3
#define URJ_FLASH_ERROR_UNKNOWN                         99

extern urj_flash_cfi_array_t *urj_flash_cfi_array;

void urj_flash_detectflash (urj_bus_t *bus, uint32_t adr);

void urj_flashmem (urj_bus_t *bus, FILE * f, uint32_t addr, int);
void urj_flashmsbin (urj_bus_t *bus, FILE * f, int);

void urj_flasherase (urj_bus_t *bus, uint32_t addr, int number);

/* end of original brux/flash.h */

extern urj_flash_driver_t *urj_flash_flash_drivers[];

#endif /* URJ_FLASH_H */
