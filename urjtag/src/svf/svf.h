/*
 * $Id$
 *
 * Copyright (C) 2004, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2004.
 *
 */


#include <stdint.h>

#include "chain.h"

#define MAX_PATH_STATES 64

/* Coding for commands referring either to IR or DR */
enum generic_irdr_coding
{ generic_ir, generic_dr };


struct tdval
{
    int token;
    double dvalue;
};
struct tcval
{
    int token;
    char *cvalue;
    struct tcval *next;
};


struct ths_params
{
    double number;
    char *tdi;
    char *tdo;
    char *mask;
    char *smask;
};

struct path_states
{
    int states[MAX_PATH_STATES];
    int num_states;
};

struct runtest
{
    int run_state;
    uint32_t run_count;
    int run_clk;
    double min_time;
    double max_time;
    int end_state;
};

typedef struct
{
    struct ths_params params;
    int no_tdi;
    int no_tdo;
} sxr_t;


struct svf_parser_params
{
    struct ths_params ths_params;
    struct path_states path_states;
    struct runtest runtest;
};


/* private data of the bison parser
   used to store variables the would end up as globals otherwise */
struct parser_priv
{
    struct svf_parser_params parser_params;
    void *scanner;
    part_t *part;
    instruction *ir;
    data_register *dr;
    sxr_t sir_params;
    sxr_t sdr_params;
    int endir;
    int enddr;
    int runtest_run_state;
    int runtest_end_state;
    int svf_stop_on_mismatch;
    int svf_trst_absent;
    int svf_state_executed;
    uint32_t ref_freq;
    int mismatch_occurred;
    /* protocol issued warnings */
    int issued_runtest_maxtime;
};
typedef struct parser_priv parser_priv_t;

struct scanner_extra
{
    int num_lines;
    int print_progress;
    int planb;
    char decimal_point;
};
typedef struct scanner_extra scanner_extra_t;

struct YYLTYPE;

void *svf_flex_init (FILE *, int, int);
void svf_flex_deinit (void *);

int svf_bison_init (parser_priv_t *, FILE *, int, int);
void svf_bison_deinit (parser_priv_t *);

void svf_endxr (parser_priv_t *, enum generic_irdr_coding, int);
void svf_frequency (chain_t *, double);
int svf_hxr (enum generic_irdr_coding, struct ths_params *);
int svf_runtest (chain_t *, parser_priv_t *, struct runtest *);
int svf_state (chain_t *, parser_priv_t *, struct path_states *, int);
int svf_sxr (chain_t *, parser_priv_t *, enum generic_irdr_coding,
             struct ths_params *, struct YYLTYPE *);
int svf_trst (chain_t *, parser_priv_t *, int);
int svf_txr (enum generic_irdr_coding, struct ths_params *);
