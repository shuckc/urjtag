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
%parse-param {urj_svf_parser_priv_t *priv_data}
%parse-param {urj_chain_t *chain}
/* See https://lists.gnu.org/archive/html/bug-bison/2014-02/msg00002.html */
%lex-param {void *HACK}
%name-prefix "urj_svf_"
%locations

%{
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <urjtag/log.h>

#include "svf.h"

/* interface to flex */
#include "svf_bison.h"
#define HACK priv_data->scanner
int yylex (YYSTYPE *, YYLTYPE *, void *);

// @@@@ RFHH need to define YYPRINTF in terms of urj_log()

#define YYERROR_VERBOSE


void yyerror(YYLTYPE *, urj_svf_parser_priv_t *priv_data, urj_chain_t *, const char *);

static void urj_svf_free_ths_params(struct ths_params *);
%}

%union {
  int    token;
  double dvalue;
  char  *cvalue;
  int    ivalue;
  struct hexa_frag hexa_frag;
  struct tdval tdval;
  struct tcval *tcval;
}


%token IDENTIFIER NUMBER HEXA_NUM_FRAGMENT VECTOR_STRING

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
%type <cvalue> HEXA_NUM_FRAGMENT
%type <tdval>  runtest_clk_count
%type <token>  runtest_run_state_opt
%type <token>  runtest_end_state_opt
%type <hexa_frag> hexa_num_sequence

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
      urj_svf_endxr(priv_data, generic_ir, $<token>2);
    }

    | ENDDR stable_state ';'
    {
      urj_svf_endxr(priv_data, generic_dr, $<token>2);
    }

    | FREQUENCY ';'
      {
        urj_svf_frequency(chain, 0.0);
      }

    | FREQUENCY NUMBER HZ ';'
      {
        urj_svf_frequency(chain, $2);
      }

    | HDR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &(priv_data->parser_params.ths_params);

        p->number = $2;
        urj_svf_hxr(generic_dr, p);
        urj_svf_free_ths_params(p);
      }

    | HIR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &(priv_data->parser_params.ths_params);

        p->number = $2;
        urj_svf_hxr(generic_ir, p);
        urj_svf_free_ths_params(p);
      }

    | PIOMAP '(' direction IDENTIFIER piomap_rec ')' ';'
      {
        urj_log (URJ_LOG_LEVEL_ERROR, "PIOMAP not implemented\n");
        yyerror(&@$, priv_data, chain, "PIOMAP");
        YYERROR;
      }

    | PIO VECTOR_STRING ';'
      {
        free($<cvalue>2);
        urj_log (URJ_LOG_LEVEL_ERROR, "PIO not implemented\n");
        yyerror(&@$, priv_data, chain, "PIO");
        YYERROR;
      }

    | RUNTEST runtest_run_state_opt runtest_clk_count runtest_time_opt runtest_end_state_opt ';'
      {
        struct runtest *rt = &(priv_data->parser_params.runtest);

        rt->run_state = $2;
        rt->run_count = $3.dvalue;
        rt->run_clk   = $3.token;
        rt->end_state = $5;

        if (urj_svf_runtest(chain, priv_data, rt) != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "RUNTEST");
          YYERROR;
        }
      }

    | RUNTEST runtest_run_state_opt runtest_time runtest_end_state_opt ';'
      {
        struct runtest *rt = &(priv_data->parser_params.runtest);

        rt->run_state = $2;
        rt->run_count = 0;
        rt->run_clk   = 0;
        rt->end_state = $4;

        if (urj_svf_runtest(chain, priv_data, rt) != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "RUNTEST");
          YYERROR;
        }
      }

    | SDR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &(priv_data->parser_params.ths_params);
        int result;

        p->number = $2;
        result = urj_svf_sxr(chain, priv_data, generic_dr, p, &@$);
        urj_svf_free_ths_params(p);

        if (result != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "SDR");
          YYERROR;
        }
      }

    | SIR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &(priv_data->parser_params.ths_params);
        int result;

        p->number = $2;
        result = urj_svf_sxr(chain, priv_data, generic_ir, p, &@$);
        urj_svf_free_ths_params(p);

        if (result != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "SIR");
          YYERROR;
        }
      }

    | STATE path_states stable_state ';'
      {
        if (urj_svf_state(chain, priv_data, &(priv_data->parser_params.path_states), $<token>3) != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "STATE");
          YYERROR;
        }
      }

    | TDR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &(priv_data->parser_params.ths_params);
        int result;

        p->number = $2;
        result = urj_svf_txr(generic_dr, p);
        urj_svf_free_ths_params(p);

        if (result != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "TDR");
          YYERROR;
        }
      }

    | TIR NUMBER ths_param_list ';'
      {
        struct ths_params *p = &(priv_data->parser_params.ths_params);
        int result;

        p->number = $2;
        result = urj_svf_txr(generic_ir, p);
        urj_svf_free_ths_params(p);

        if (result != URJ_STATUS_OK) {
          yyerror(&@$, priv_data, chain, "TIR");
          YYERROR;
        }
      }

    | TRST trst_mode ';'
    {
      if (urj_svf_trst(chain, priv_data, $<token>2) != URJ_STATUS_OK) {
        yyerror(&@$, priv_data, chain, "TRST");
        YYERROR;
      }
    }
;


ths_param_list
            : /* empty element */
            | ths_param_list ths_opt_param
;

ths_opt_param
            : TDI   '(' hexa_num_sequence ')'
              {
                priv_data->parser_params.ths_params.tdi = $3.buf;
              }

            | TDO   '(' hexa_num_sequence ')'
              {
                priv_data->parser_params.ths_params.tdo = $3.buf;
              }

            | MASK  '(' hexa_num_sequence ')'
              {
                priv_data->parser_params.ths_params.mask = $3.buf;
              }

            | SMASK '(' hexa_num_sequence ')'
              {
                priv_data->parser_params.ths_params.smask = $3.buf;
              }
;

hexa_num_sequence
           : HEXA_NUM_FRAGMENT
             {
                 $$.buf    = $1;
                 $$.strlen = strlen ($1);
                 $$.buflen = $$.strlen + 1;
             }
           | hexa_num_sequence HEXA_NUM_FRAGMENT
             {
#define REALLOC_STEP (1 << 16)
                 size_t frag_len = strlen ($2);
                 size_t req_len = $1.strlen + frag_len;
                 if ($1.buflen <= req_len) {
                     size_t newlen = req_len - $1.buflen < REALLOC_STEP ?
                         $1.buflen + REALLOC_STEP : req_len + 1;
                     $1.buf = (char *)realloc ($1.buf, newlen);
                     $1.buflen = newlen;
                 }
                 if ($1.buf != NULL) {
                     memcpy ($1.buf + $1.strlen, $2, frag_len + 1);
                     $1.strlen += frag_len;
                 }
                 free ($2);
                 $$ = $1;
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
                priv_data->parser_params.runtest.min_time = 0.0;
                priv_data->parser_params.runtest.max_time = 0.0;
              }

            | runtest_time
;

runtest_time
            : NUMBER SEC runtest_max_time_opt
              {
                priv_data->parser_params.runtest.min_time = $<dvalue>1;
              }
;

runtest_max_time_opt
            :
              {
                priv_data->parser_params.runtest.max_time = 0.0;
              }
            | MAXIMUM NUMBER SEC
              {
                priv_data->parser_params.runtest.max_time = $<dvalue>2;
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
                priv_data->parser_params.path_states.num_states = 0;
              }

            | path_states all_states
              {
                struct path_states *ps = &(priv_data->parser_params.path_states);

                if (ps->num_states < MAX_PATH_STATES) {
                  ps->states[ps->num_states] = $<token>2;
                  ps->num_states++;
                } else
                  urj_log (URJ_LOG_LEVEL_ERROR, "Error %s: maximum number of %d path states reached.\n",
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

%% void
yyerror (YYLTYPE *locp, urj_svf_parser_priv_t *priv_data, urj_chain_t *chain,
         const char *error_string)
{
    urj_log (URJ_LOG_LEVEL_ERROR, "Error occurred for SVF command, line %d, column %d-%d:\n %s.\n",
             locp->first_line, locp->first_column, locp->last_column, error_string);
}


static void
urj_svf_free_ths_params (struct ths_params *params)
{
    params->number = 0.0;

    if (params->tdi)
    {
        free (params->tdi);
        params->tdi = NULL;
    }
    if (params->tdo)
    {
        free (params->tdo);
        params->tdo = NULL;
    }
    if (params->mask)
    {
        free (params->mask);
        params->mask = NULL;
    }
    if (params->smask)
    {
        free (params->smask);
        params->smask = NULL;
    }
}


int
urj_svf_bison_init (urj_svf_parser_priv_t *priv_data, FILE *f, int num_lines)
{
    const struct svf_parser_params params = {
        {0.0, NULL, NULL, NULL, NULL},
        {{}, 0},
        {0, 0.0, 0, 0, 0, 0}
    };

    priv_data->parser_params = params;

    if ((priv_data->scanner =
         urj_svf_flex_init (f, num_lines)) == NULL)
        return 0;
    else
        return 1;
}


void
urj_svf_bison_deinit (urj_svf_parser_priv_t *priv_data)
{
    urj_svf_flex_deinit (priv_data->scanner);
}
