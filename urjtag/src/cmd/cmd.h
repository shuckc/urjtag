/*
 * $Id$
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

#ifndef URJ_SRC_CMD_H
#define URJ_SRC_CMD_H

#include <sysdep.h>

#include <urjtag/types.h>

typedef struct
{
    char *name;
    char *desc;
    void (*help) (void);
    /** @return URJ_STATUS_OK on success; URJ_STATUS_FAIL on error, both
     * syntax and library errors */
    int (*run) (urj_chain_t *chain, char *params[]);
} urj_cmd_t;

extern const urj_cmd_t urj_cmd_quit;
extern const urj_cmd_t urj_cmd_help;
extern const urj_cmd_t urj_cmd_frequency;
extern const urj_cmd_t urj_cmd_cable;
extern const urj_cmd_t urj_cmd_reset;
extern const urj_cmd_t urj_cmd_discovery;
extern const urj_cmd_t urj_cmd_idcode;
extern const urj_cmd_t urj_cmd_detect;
extern const urj_cmd_t urj_cmd_signal;
extern const urj_cmd_t urj_cmd_scan;
extern const urj_cmd_t urj_cmd_salias;
extern const urj_cmd_t urj_cmd_bit;
extern const urj_cmd_t urj_cmd_register;
extern const urj_cmd_t urj_cmd_initbus;
extern const urj_cmd_t urj_cmd_print;
extern const urj_cmd_t urj_cmd_part;
extern const urj_cmd_t urj_cmd_bus;
extern const urj_cmd_t urj_cmd_instruction;
extern const urj_cmd_t urj_cmd_shift;
extern const urj_cmd_t urj_cmd_dr;
extern const urj_cmd_t urj_cmd_get;
extern const urj_cmd_t urj_cmd_test;
extern const urj_cmd_t urj_cmd_shell;
extern const urj_cmd_t urj_cmd_set;
extern const urj_cmd_t urj_cmd_endian;
extern const urj_cmd_t urj_cmd_peek;
extern const urj_cmd_t urj_cmd_poke;
extern const urj_cmd_t urj_cmd_pod;
extern const urj_cmd_t urj_cmd_readmem;
extern const urj_cmd_t urj_cmd_writemem;
extern const urj_cmd_t urj_cmd_detectflash;
extern const urj_cmd_t urj_cmd_flashmem;
extern const urj_cmd_t urj_cmd_eraseflash;
extern const urj_cmd_t urj_cmd_script;
extern const urj_cmd_t urj_cmd_include;
extern const urj_cmd_t urj_cmd_addpart;
extern const urj_cmd_t urj_cmd_usleep;
#ifdef ENABLE_SVF
extern const urj_cmd_t urj_cmd_svf;
#endif
#ifdef ENABLE_BSDL
extern const urj_cmd_t urj_cmd_bsdl;
#endif
extern const urj_cmd_t urj_cmd_debug;

extern const urj_cmd_t *urj_cmds[];

/**
 * Tests if chain has a cable pointer
 *
 * @return URJ_STATUS_OK if success; URJ_STATUS_FAIL on error or failure
 */
int urj_cmd_test_cable (urj_chain_t *chain);

/**
 * Count the number of parameters in this NULL-terminated list
 */
int urj_cmd_params (char *params[]);
/**
 * Parse parameter as a long unsigned
 * @return URJ_STATUS_OK on success, URJ_STATUS_FAIL on error
 */
int urj_cmd_get_number (const char *s, long unsigned *i);

#endif /* URJ_CMD_H */
