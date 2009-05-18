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

#ifndef URJ_CMD_H
#define URJ_CMD_H

#include "types.h"

/**
 * @return URJ_STATUS_OK on success; URJ_STATUS_FAIL on error. Consult
 *      urj_error for error details. Syntax errors in the input params are
 *      handled in the same way, urj_error is set to #URJ_ERROR_SYNTAX.
 */
int urj_cmd_run (urj_chain_t *chain, char *params[]);
/**
 * Search through registered commands
 *
 * @param text match commands whose prefix equals <code>text</code>. Rotates
 *      through the registered commands. The prefix length is set when
 *      the rotating state is reset.
 * @@@@ RFHH that is weird behaviour. Why not do the prefix length as strlen(text)?
 * @param state if 0, reset the rotating state to start from the beginning
 *
 * @return malloc'ed value. The caller is responsible for freeing it.
 *      NULL for malloc failure or end of command list.
 */
char *urj_cmd_find_next (const char *text, int state);

/* @@@@ RFHH candidate to become local in src/cmd/ after cable refactor */
int urj_cmd_params (char *params[]);
/* @@@@ RFHH candidate to become local in src/cmd/ after cable refactor */
int urj_cmd_get_number (const char *s, long unsigned *i);

#endif /* URJ_CMD_H */
