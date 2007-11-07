/*
 * $Id$
 *
 * Copyright (C) 2002 by CSD at http://www-csd.ijs.si
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
 * Original parser skeleton by Robert Sedevici <robert.sedevcic@ijs.si>, 2002.
 *
 */

%pure-parser
%locations

%{
#include <stdio.h>
#include <stdlib.h>

#include "svf.h"

/* interface to flex */
#include "svf_bison.h"
int yylex (YYSTYPE *, YYLTYPE *);

#define YYERROR_VERBOSE

struct svf_parser_params {
    struct ths_params  ths_params;
    struct path_states path_states;
    struct runtest     runtest;
};


static struct svf_parser_params parser_params = {
  {0.0, NULL, NULL, NULL, NULL},
  {{}, 0},
  {0, 0.0, 0, 0, 0, 0}
};

void yyerror(const char *);

static void svf_free_ths_params(struct ths_params *);
%}

%union {
  int    token;
  double dvalue;
  char  *cvalue;
  int    ivalue;
  struct tdval tdval;
  struct tcval *tcval;
}


%token IDENTIFIER NUMBER HEXA_NUM VECTOR_STRING

%token EMPTY
%token ENDDR ENDIR 
%token FREQUENCY HZ
%token STATE RESET IDLE 
%token TDI TDO MASK SMASK
%token TRST ON OFF Z ABSENT
%token HDR HIR SDR SIR TDR TIR
%token PIO PIOMAP IN OUT INOUT H L U D X
%token RUNTEST MAXIMUM SEC TCK SCK ENDSTATE 
%token IRPAUSE IRSHIFT IRUPDATE IRSELECT IREXIT1 IREXIT2 IRCAPTURE
%token DRPAUSE DRSHIFT DRUPDATE DRSELECT DREXIT1 DREXIT2 DRCAPTURE
%token SVF_EOF 0    /* SVF_EOF must match bison's token YYEOF */

%type <dvalue> NUMBER
%type <tdval>  runtest_clk_count
%type <token>  runtest_run_state_opt
%type <token>  runtest_end_state_opt

%%

line
    : /* empty */
    | line svf_statement
    | error SVF_EOF
      /* Eat whole file in case of error.
       * This is necessary because the lexer will remember parts of the file
       * inside its input buffer.
       * In case errors do not driver the lexer to EOF then the next start
       * of yyparse() will read from this buffer, executing commands after the
       * previous error!
       */
;


svf_statement
    : ENDIR stable_state ';'
    {
      svf_endxr(generic_ir, $<token>2);
    }

    | ENDDR stable_state ';'
    {
      svf_endxr(generic_dr, $<token>2);
    }

    | FREQUENCY ';'
      {
        svf_frequency(0.0);
      }

    | FREQUENCY NUMBER HZ ';'
      {
        svf_frequency($2);
      }

    | HDR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &parser_params.ths_params;

        p->number = $2;
        svf_hxr(generic_dr, p);
        svf_free_ths_params(p);
      }

    | HIR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &parser_params.ths_params;

        p->number = $2;
        svf_hxr(generic_ir, p);
        svf_free_ths_params(p);
      }

    | PIOMAP '(' direction IDENTIFIER piomap_rec ')' ';'
      {
        printf("PIOMAP not implemented\n");
        yyerror("PIOMAP");
        YYERROR;
      }

    | PIO VECTOR_STRING ';'
      {
        free($<cvalue>2);
        printf("PIO not implemented\n");
        yyerror("PIO");
        YYERROR;
      }

    | RUNTEST runtest_run_state_opt runtest_clk_count runtest_time_opt runtest_end_state_opt ';'
      {
        struct runtest *rt = &parser_params.runtest;

        rt->run_state = $2;
        rt->run_count = $3.dvalue;
        rt->run_clk   = $3.token;
        rt->end_state = $5;

        if (!svf_runtest(rt)) {
          yyerror("RUNTEST");
          YYERROR;
        }
      }

    | RUNTEST runtest_run_state_opt runtest_time runtest_end_state_opt ';'
      {
        struct runtest *rt = &parser_params.runtest;

        rt->run_state = $2;
        rt->run_count = 0;
        rt->run_clk   = 0;
        rt->end_state = $4;

        if (!svf_runtest(rt)) {
          yyerror("RUNTEST");
          YYERROR;
        }
      }

    | SDR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &parser_params.ths_params;
        int result;

        p->number = $2;
        result = svf_sxr(generic_dr, p, &@$);
        svf_free_ths_params(p);

        if (!result) {
          yyerror("SDR");
          YYERROR;
        }
      }

    | SIR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &parser_params.ths_params;
        int result;

        p->number = $2;
        result = svf_sxr(generic_ir, p, &@$);
        svf_free_ths_params(p);

        if (!result) {
          yyerror("SIR");
          YYERROR;
        }
      }

    | STATE path_states stable_state ';'
      {
        if (!svf_state(&parser_params.path_states, $<token>3)) {
          yyerror("STATE");
          YYERROR;
        }
      }

    | TDR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &parser_params.ths_params;
        int result;

        p->number = $2;
        result = svf_txr(generic_dr, p);
        svf_free_ths_params(p);

        if (!result) {
          yyerror("TDR");
          YYERROR;
        }
      }

    | TIR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &parser_params.ths_params;
        int result;

        p->number = $2;
        result = svf_txr(generic_ir, p);
        svf_free_ths_params(p);

        if (!result) {
          yyerror("TIR");
          YYERROR;
        }
      }

    | TRST trst_mode ';'
    {
      if (!svf_trst($<token>2)) {
        yyerror("TRST");
        YYERROR;
      }
    }
;


ths_param_list
            : /* empty element */
            | ths_param_list ths_opt_param
; 

ths_opt_param
            : TDI   HEXA_NUM
              {
                parser_params.ths_params.tdi = $<cvalue>2;
              }

            | TDO   HEXA_NUM
              {
                parser_params.ths_params.tdo = $<cvalue>2;
              }

            | MASK  HEXA_NUM
              {
                parser_params.ths_params.mask = $<cvalue>2;
              }

            | SMASK HEXA_NUM
              {
                parser_params.ths_params.smask = $<cvalue>2;
              }
;

stable_state
            : RESET 
            | IDLE
            | DRPAUSE
            | IRPAUSE
;

runtest_run_state_opt
            : { $$ = 0; }       /* specify value for 'not existing' */
            | stable_state
              {
                $$ = $<token>1;
              }
;

runtest_clk_count
            : NUMBER TCK
              {
                $$.token  = $<token>2;
                $$.dvalue = $<dvalue>1;
              }

            | NUMBER SCK 
              {
                $$.token  = $<token>2;
                $$.dvalue = $<dvalue>1;
              }
;

runtest_time_opt
            :
              {
                parser_params.runtest.min_time = 0.0;
                parser_params.runtest.max_time = 0.0;
              }

            | runtest_time
;

runtest_time
            : NUMBER SEC runtest_max_time_opt
              {
                parser_params.runtest.min_time = $<dvalue>1;
              }
;

runtest_max_time_opt
            : 
            | MAXIMUM NUMBER SEC
              {
                parser_params.runtest.max_time = $<dvalue>2;
              }
;

runtest_end_state_opt
            : { $$ = 0; }           /* specify value for 'not existing' */
            | ENDSTATE stable_state
              {
                $$ = $<token>2;
              }
;

all_states
            : DRSELECT
            | DRCAPTURE
            | DRSHIFT
            | DREXIT1
            | DREXIT2
            | DRUPDATE
            | IRSELECT
            | IRCAPTURE
            | IRSHIFT
            | IREXIT1
            | IREXIT2
            | IRUPDATE
            | IRPAUSE
            | DRPAUSE
            | RESET
            | IDLE 
;

path_states
            : /* empty element, returns index 0 */
              {
                parser_params.path_states.num_states = 0;
              }

            | path_states all_states
              {
                struct path_states *ps = &parser_params.path_states;

                if (ps->num_states < MAX_PATH_STATES) {
                  ps->states[ps->num_states] = $<token>2;
                  ps->num_states++;
                } else
                  printf("Error %s: maximum number of %d path states reached.\n",
                        "svf", MAX_PATH_STATES);
              }
;

piomap_rec
            : 
            | piomap_rec direction IDENTIFIER
;

trst_mode
            : ON
            | OFF
            | Z
            | ABSENT
;

direction
            : IN
            | OUT
            | INOUT
;

%%


void
yyerror(const char *error_string)
{
  printf("Error occured for SVF command %s.\n", error_string);
}


static void
svf_free_ths_params(struct ths_params *params)
{
  params->number = 0.0;

  if (params->tdi) {
    free(params->tdi);
    params->tdi = NULL;
  }
  if (params->tdo) {
    free(params->tdo);
    params->tdo = NULL;
  }
  if (params->mask) {
    free(params->mask);
    params->mask = NULL;
  }
  if (params->smask) {
    free(params->smask);
    params->smask = NULL;
  }
}
