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

#include <urjtag/chain.h>

#define MAX_PATH_STATES 64

/* Coding for commands referring either to IR or DR */
enum generic_irdr_coding
{
    generic_ir,
    generic_dr
};


struct hexa_frag
{
    char   *buf;
    size_t  buflen;
    size_t  strlen;
};
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
} urj_svf_sxr_t;


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
    urj_part_t *part;
    urj_part_instruction_t *ir;
    urj_data_register_t *dr;
    urj_svf_sxr_t sir_params;
    urj_svf_sxr_t sdr_params;
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
typedef struct parser_priv urj_svf_parser_priv_t;

struct scanner_extra
{
    int num_lines;
    int planb;
    char decimal_point;
};
typedef struct scanner_extra urj_svf_scanner_extra_t;

struct YYLTYPE;

void *urj_svf_flex_init (FILE *, int);
void urj_svf_flex_deinit (void *);

int urj_svf_bison_init (urj_svf_parser_priv_t *, FILE *, int);
void urj_svf_bison_deinit (urj_svf_parser_priv_t *);

void urj_svf_endxr (urj_svf_parser_priv_t *, enum generic_irdr_coding,
                    int);
void urj_svf_frequency (urj_chain_t *, double);
int urj_svf_hxr (enum generic_irdr_coding, struct ths_params *);
int urj_svf_runtest (urj_chain_t *, urj_svf_parser_priv_t *,
                     struct runtest *);
int urj_svf_state (urj_chain_t *, urj_svf_parser_priv_t *,
                   struct path_states *, int);
int urj_svf_sxr (urj_chain_t *, urj_svf_parser_priv_t *,
                 enum generic_irdr_coding, struct ths_params *,
                 struct YYLTYPE *);
int urj_svf_trst (urj_chain_t *, urj_svf_parser_priv_t *, int);
int urj_svf_txr (enum generic_irdr_coding, struct ths_params *);
