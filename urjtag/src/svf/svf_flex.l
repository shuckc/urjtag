/* $Id$
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
 * Original lexer by Robert Sedevici <robert.sedevcic@ijs.si>, 2002.
 * Modified by Arnim Laeuger <arniml@users.sourceforge.net>, 2004.
 *
 */


%option bison-bridge
%option reentrant
%option prefix="urj_svf_"
%option outfile="lex.yy.c"
%option bison-locations

%{
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include <urjtag/sysdep.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "svf.h"

#include "svf_bison.h"

#define YY_EXTRA_TYPE urj_svf_scanner_extra_t *

static int map_keyw_ident(YYSTYPE *, char *);
static void align_string(char *);

static void fix_yylloc(YYLTYPE *, char *);
static void fix_yylloc_nl(YYLTYPE *, char *, YY_EXTRA_TYPE);
static void progress_nl(YYLTYPE *, YY_EXTRA_TYPE);

int yywrap(yyscan_t scanner);
int yywrap(yyscan_t scanner)
{
  return(1);
}

#define YY_USER_INIT \
    do { \
        yylloc->first_line = yylloc->last_line = yylloc->first_column = yylloc->last_column = 0; \
    } while (0)

%}

%pointer

LETTER          [A-Za-z]
DIGIT           [0-9]
HEX_DIGIT       {DIGIT}|[a-fA-F]
WSPACE          [ \t\r]
COMMENT         (!.*)|("//".*)[^\n]

%s expect_vector

%%


{LETTER}+[0-9A-Za-z_]* {
  /* token is a keyword or identifier */
  int keyw;

  fix_yylloc(yylloc, yytext);
  keyw = map_keyw_ident(yylval, yytext);

  /* enable detection of VECTOR_STRING when this is a PIO command */
  if (keyw == PIO) {
    BEGIN(expect_vector);
  }

  return(keyw);
} /* end of keyword or identifier */


{DIGIT}+(\.{DIGIT}+)?([eE][-+]?{DIGIT}+)? {
  /* token is a real number */

  char *real_string = strdup(yytext);

  /* Note: We need to compensate the current locale's representation
           of the decimal point since strtod() functionality depends
           on the locale settings. */

  if (real_string) {
    char *point_pos = strchr(real_string, '.');
    YY_EXTRA_TYPE extra = yyget_extra(yyscanner);

    if (point_pos)
      /* convert decimal point into current locale's representation */
      *point_pos = extra->decimal_point;

    yylval->dvalue = strtod(real_string, (char **) NULL);
    fix_yylloc(yylloc, yytext);

    free(real_string);
  } else
    yylval->dvalue = 0.0;

  return(NUMBER);
} /* end of real number */


<expect_vector>"("{WSPACE}*[\n\rHhLlZzUuDdXx \t\r]+{WSPACE}*")" {
  /* There is an overlap of VECTOR_STRING and HEXA_NUM when the string
     contains only 'd' or 'D'. To prevent complicated parsing rules,
     the lexer is instructed to detect VECTOR_STRING only when a PIO
     command has been scanned previously.
     This is enabled with <expect_vector>. */
  /* token is a vector string */
  char *cstring;

  fix_yylloc_nl(yylloc, yytext, yyget_extra(yyscanner));
  align_string(yytext);

  cstring = calloc(strlen(yytext) + 1, sizeof(char));
  strcpy(cstring, yytext);
  yylval->cvalue = cstring;
  return(VECTOR_STRING);
} /* end of vector string */


"("{WSPACE}*[\n\rA-Fa-f0-9 \t\r]+{WSPACE}*")" {
  /* token is a hexadecimal value (2) */
  char *cstring;

  fix_yylloc_nl(yylloc, yytext, yyget_extra(yyscanner));
  align_string(yytext);

  cstring = calloc(strlen(yytext) + 1, sizeof(char));
  strcpy(cstring, yytext);
  yylval->cvalue = cstring;
  return(HEXA_NUM);
} /* end of hexadecimal value */


{WSPACE}+ {
  /* token is a white space character */
  fix_yylloc(yylloc, yytext);
} /* end of white space */


{COMMENT} {
  /* token is a comment */
  fix_yylloc(yylloc, yytext);
} /* end of comment */


[()] {
  /* left or right parenthes */
  fix_yylloc(yylloc, yytext);
  return(yytext[0]);
} /* end of left or right parenthesis */


\n {
  /* token is a new line character */
  yylloc->first_line = yylloc->last_line;
  yylloc->first_column = yylloc->last_column;
  ++yylloc->last_line;
  yylloc->last_column = 0;
  progress_nl(yylloc, yyget_extra(yyscanner));
} /* end of new line */


; {
  /* token is end of statement character */

  /* release expect_vector */
  BEGIN(INITIAL);

  fix_yylloc(yylloc, yytext);
  return(yytext[0]);
} /* end of statement character */


<<EOF>> {

  return(EOF);
} /* end of file token */


. {
  /* print token if interactive parsing enabled and yyin != stdin */

  fprintf(stderr, "Error: \"%s\" is not a legal SVF language token\n", yytext);

} /* end of any other character */

%%


/*=============================================================================
 * rwtable - reserve word table
 *===========================================================================*/
static struct rwtable
{
    char *rw_name;
    int rw_yylex;
} rwtable[] =
{
    { "ABSENT",         ABSENT },
    { "D",              D },
    { "DRCAPTURE",      DRCAPTURE },
    { "DREXIT1",        DREXIT1 },
    { "DREXIT2",        DREXIT2 },
    { "DRPAUSE",        DRPAUSE },
    { "DRSELECT",       DRSELECT },
    { "DRSHIFT",        DRSHIFT },
    { "DRUPDATE",       DRUPDATE },
    { "EMPTY",          EMPTY },
    { "ENDDR",          ENDDR },
    { "ENDIR",          ENDIR },
    { "ENDSTATE",       ENDSTATE },
    { "FREQUENCY",      FREQUENCY },
    { "H",              H },
    { "HDR",            HDR },
    { "HIR",            HIR },
    { "HZ",             HZ },
    { "IDLE",           IDLE },
    { "IN",             IN },
    { "INOUT",          INOUT },
    { "IRCAPTURE",      IRCAPTURE },
    { "IREXIT1",        IREXIT1 },
    { "IREXIT2",        IREXIT2 },
    { "IRPAUSE",        IRPAUSE },
    { "IRSELECT",       IRSELECT },
    { "IRSHIFT",        IRSHIFT },
    { "IRUPDATE",       IRUPDATE },
    { "L",              L },
    { "MASK",           MASK },
    { "MAXIMUM",        MAXIMUM },
    { "OFF",            OFF },
    { "ON",             ON },
    { "OUT",            OUT },
    { "PIO",            PIO },
    { "PIOMAP",         PIOMAP },
    { "RESET",          RESET },
    { "RUNTEST",        RUNTEST },
    { "SCK",            SCK },
    { "SDR",            SDR },
    { "SEC",            SEC },
    { "SIR",            SIR },
    { "SMASK",          SMASK },
    { "STATE",          STATE },
    { "TCK",            TCK },
    { "TDI",            TDI },
    { "TDO",            TDO },
    { "TDR",            TDR },
    { "TIR",            TIR },
    { "TRST",           TRST },
    { "U",              U },
    { "X",              X },
    { "Z",              Z }
};                              //end of rwtable struct

#define END(v) (sizeof(v) / sizeof((v)[0]) - 1)

static int
map_keyw_ident (YYSTYPE *mylval, char *str)
{
    int idx;
    int rw = IDENTIFIER;

    mylval->cvalue = str;

    for (idx = 0; idx <= END (rwtable); idx++)
    {
        if (strcasecmp (rwtable[idx].rw_name, str) == 0)
        {
            /* always return terminal value as semantic value */
            rw = rwtable[idx].rw_yylex;
            mylval->token = rw;
        }
    }

    return (rw);
}


static void
align_string (char *str)
{
    int src, dst, len;

    dst = 0;
    len = strlen (str);

    for (src = 0; src < len; src++)
    {
        if (isxdigit (str[src]))
            str[dst++] = str[src];
    }
    str[dst] = '\0';
}


static void
fix_yylloc (YYLTYPE *mylloc, char *str)
{
    mylloc->first_line = mylloc->last_line;
    mylloc->first_column = mylloc->last_column;
    mylloc->last_column += strlen (str);
}


static void
fix_yylloc_nl (YYLTYPE *mylloc, char *str, YY_EXTRA_TYPE extra)
{
    char *p;

    mylloc->first_line = mylloc->last_line;
    mylloc->first_column = mylloc->last_column;
    for (p = str; *p; ++p)
    {
        if (*p == '\n')
        {
            mylloc->last_column = 0;
            ++mylloc->last_line;
            progress_nl (mylloc, extra);
        }
        else
        {
            ++mylloc->last_column;
        }
    }
}


static void
progress_nl (YYLTYPE *mylloc, YY_EXTRA_TYPE extra)
{
    int percent;

    if ((extra->print_progress) && (mylloc->last_line % 10 == 0))
    {
        percent = ((mylloc->last_line * 100) + 1) / extra->num_lines;
        if (percent <= 1)
            return;             // dont bother printing < 1 %
        printf ("\r");
        printf (_("Parsing %6d/%d (%3.0d%%)"),
                mylloc->last_line, extra->num_lines, percent);
    }
}


void *
urj_svf_flex_init (FILE *f, int num_lines, int print_progress)
{
    YY_EXTRA_TYPE extra;
    yyscan_t scanner;

    /* get our scanner structure */
    if (yylex_init (&scanner) != 0)
        return NULL;

    yyset_in (f, scanner);

    if (!(extra = malloc (sizeof (urj_svf_scanner_extra_t))))
    {
        printf (_("Out of memory, %s line %i\n"), __FILE__, __LINE__);
        yylex_destroy (scanner);
        return NULL;
    }

    extra->num_lines = num_lines;
    extra->print_progress = print_progress;

#ifdef ENABLE_NLS
    {
        struct lconv *lc = localeconv ();
        extra->decimal_point = lc->decimal_point[0];
    }
#else
    extra->decimal_point = '.';
#endif

    yyset_extra (extra, scanner);

    return scanner;
}


void
urj_svf_flex_deinit (void *scanner)
{
    YY_EXTRA_TYPE extra = yyget_extra (scanner);
    if (extra->print_progress)
        printf ("\n");
    free (extra);
    yylex_destroy (scanner);
}
