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
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#ifndef URJ_CHAIN_H
#define	URJ_CHAIN_H

#include "part.h"
#include "pod.h"

typedef struct urj_chain urj_chain_t;

#include "cable.h"
#include "bsdl.h"

#define URJ_CHAIN_EXITMODE_SHIFT 0
#define URJ_CHAIN_EXITMODE_IDLE  1
#define URJ_CHAIN_EXITMODE_EXIT1 2
#define URJ_CHAIN_EXITMODE_UPDATE 3

struct urj_chain
{
    int state;
    urj_parts_t *parts;
    int total_instr_len;
    int active_part;
    urj_cable_t *cable;
    urj_bsdl_globs_t bsdl;
};

urj_chain_t *urj_tap_chain_alloc (void);
void urj_tap_chain_free (urj_chain_t *chain);
void urj_tap_chain_disconnect (urj_chain_t *chain);
void urj_tap_chain_clock (urj_chain_t *chain, int tms, int tdi, int n);
void urj_tap_chain_defer_clock (urj_chain_t *chain, int tms, int tdi, int n);
int urj_tap_chain_set_trst (urj_chain_t *chain, int trst);
int urj_tap_chain_get_trst (urj_chain_t *chain);
void urj_tap_chain_shift_instructions (urj_chain_t *chain);
void urj_tap_chain_shift_instructions_mode (urj_chain_t *chain, int capture_output,
                                    int capture, int chain_exit);
void urj_tap_chain_shift_data_registers (urj_chain_t *chain, int capture_output);
void urj_tap_chain_shift_data_registers_mode (urj_chain_t *chain, int capture_output,
                                      int capture, int chain_exit);
void urj_tap_chain_flush (urj_chain_t *chain);
int urj_tap_chain_set_pod_signal (urj_chain_t *chain, int mask, int val);
int urj_tap_chain_get_pod_signal (urj_chain_t *chain, urj_pod_sigsel_t sig);

typedef struct
{
    urj_chain_t **chains;
    int size;                   /* allocated chains array size */
} urj_chains_t;

#endif /* URJ_CHAIN_H */
