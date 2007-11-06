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


#define MAX_PATH_STATES 64

/* Coding for commands referring either to IR or DR */
enum generic_irdr_coding { generic_ir, generic_dr };


struct tdval {
    int    token;
    double dvalue;
};
struct tcval {
    int   token;
    char *cvalue;
    struct tcval *next;
};


struct ths_params {
    double number;
    char  *tdi;
    char  *tdo;
    char  *mask;
    char  *smask;
};

struct path_states {
    int states[MAX_PATH_STATES];
    int num_states;
};

struct runtest {
    int    run_state;
    uint32_t run_count;
    int    run_clk;
    double min_time;
    double max_time;
    int    end_state;
};

struct YYLTYPE;

void svf_endxr(enum generic_irdr_coding, int);
void svf_frequency(double);
int  svf_hxr(enum generic_irdr_coding, struct ths_params *);
int  svf_runtest(struct runtest *);
int  svf_state(struct path_states *, int);
int  svf_sxr(enum generic_irdr_coding, struct ths_params *, struct YYLTYPE *);
int  svf_trst(int);
int  svf_txr(enum generic_irdr_coding, struct ths_params *);
