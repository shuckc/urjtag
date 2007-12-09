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


/*
 * Note: This lexer supports bison_locations if HAVE_MODERN_FLEX is
         defined. Apart from this, the option --bison-locations has
         to be specified on the command line.
         The command line option is controlled by the configure
         script that enables this feature based on the available
         version of flex. The #define is derived below from the version
         information that flex itself embeds in the *.c output.
 */


%{
#include <strings.h>
#include <ctype.h>

#include <sysdep.h>
#include "svf.h"
#include "svf_bison.h"

static int map_keyw_ident(YYSTYPE *, char *);
static void align_string(char *);

#ifdef YY_FLEX_MAJOR_VERSION
 #if (YY_FLEX_MAJOR_VERSION > 2) ||\
     ((YY_FLEX_MAJOR_VERSION == 2) && ((YY_FLEX_MINOR_VERSION > 5) \
        || ((YY_FLEX_MINOR_VERSION == 5) && (YY_FLEX_SUBMINOR_VERSION >= 31))))
  #undef HAVE_MODERN_FLEX
  #define HAVE_MODERN_FLEX 1
 #else
  #undef HAVE_MODERN_FLEX
 #endif
#endif

#ifdef HAVE_MODERN_FLEX
static void fix_yylloc(YYLTYPE *, char *);
static void fix_yylloc_nl(YYLTYPE *, char *);
#define FIX_YYLLOC(lloc, text) fix_yylloc(lloc, text);
#define FIX_YYLLOC_NL(lloc, text) fix_yylloc_nl(lloc, text);

#else

#define MAP_KEYW_IDENT(LVAL, TEXT) map_keyw_ident(LVAL, TEXT)
#define FIX_YYLLOC(lloc, text)
#define FIX_YYLLOC_NL(lloc, text)

#define YY_DECL int yylex (YYSTYPE *yylval)

#endif /* HAVE_MODERN_FLEX */

int yywrap(void)
{
  return(1);
}

#ifdef HAVE_MODERN_FLEX
#define YY_USER_INIT do { yylloc->first_line = yylloc->last_line = yylloc->first_column = yylloc->last_column = 0; } while (0)
#endif /* HAVE_MODERN_FLEX */
 
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

  FIX_YYLLOC(yylloc, yytext);
  keyw = map_keyw_ident(yylval, yytext);

  /* enable detection of VECTOR_STRING when this is a PIO command */
  if (keyw == PIO) {
    BEGIN(expect_vector);
  }

  return(keyw);
} /* end of keyword or identifier */


{DIGIT}+(\.{DIGIT}+)?([eE][-+]?{DIGIT}+)? {
  /* token is a real number */

  yylval->dvalue = strtod(yytext, (char **) NULL);
  FIX_YYLLOC(yylloc, yytext);

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

  FIX_YYLLOC_NL(yylloc, yytext);
  align_string(yytext);

  cstring = calloc(strlen(yytext) + 1, sizeof(char));
  strcpy(cstring, yytext);
  yylval->cvalue = cstring;
  return(VECTOR_STRING);
} /* end of vector string */


"("{WSPACE}*[\n\rA-Fa-f0-9 \t\r]+{WSPACE}*")" {
  /* token is a hexadecimal value (2) */
  char *cstring;

  FIX_YYLLOC_NL(yylloc, yytext);
  align_string(yytext);

  cstring = calloc(strlen(yytext) + 1, sizeof(char));
  strcpy(cstring, yytext);
  yylval->cvalue = cstring;
  return(HEXA_NUM);
} /* end of hexadecimal value */


{WSPACE}+ {
  /* token is a white space character */
  FIX_YYLLOC(yylloc, yytext);
} /* end of white space */


{COMMENT} {
  /* token is a comment */
  FIX_YYLLOC(yylloc, yytext);
} /* end of comment */


[()] {
  /* left or right parenthes */
  FIX_YYLLOC(yylloc, yytext);
  return(yytext[0]);
} /* end of left or right parenthesis */


\n {
  /* token is a new line character */
#ifdef HAVE_MODERN_FLEX
  yylloc->first_line = yylloc->last_line;
  yylloc->first_column = yylloc->last_column;
  ++yylloc->last_line;
  yylloc->last_column = 0;
#endif /* HAVE_MODERN_FLEX */
} /* end of new line */


; {
  /* token is end of statement character */

  /* release expect_vector */
  BEGIN(INITIAL);

  FIX_YYLLOC(yylloc, yytext);
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
} rwtable[] = { 
  {"ABSENT",    ABSENT},
  {"D",         D},
  {"DRCAPTURE", DRCAPTURE},
  {"DREXIT1",   DREXIT1},
  {"DREXIT2",   DREXIT2},
  {"DRPAUSE",   DRPAUSE},
  {"DRSELECT",  DRSELECT},
  {"DRSHIFT",   DRSHIFT},
  {"DRUPDATE",  DRUPDATE},
  {"EMPTY",     EMPTY},
  {"ENDDR",     ENDDR},
  {"ENDIR",     ENDIR},
  {"ENDSTATE",  ENDSTATE},
  {"FREQUENCY", FREQUENCY},
  {"H",         H},
  {"HDR",       HDR},
  {"HIR",       HIR},
  {"HZ",        HZ},
  {"IDLE",      IDLE},
  {"IN",        IN},
  {"INOUT",     INOUT},
  {"IRCAPTURE", IRCAPTURE},
  {"IREXIT1",   IREXIT1},
  {"IREXIT2",   IREXIT2},
  {"IRPAUSE",   IRPAUSE},
  {"IRSELECT",  IRSELECT},
  {"IRSHIFT",   IRSHIFT},
  {"IRUPDATE",  IRUPDATE},
  {"L",         L},
  {"MASK",      MASK},
  {"MAXIMUM",   MAXIMUM},
  {"OFF",       OFF},
  {"ON",        ON},
  {"OUT",       OUT},
  {"PIO",       PIO},
  {"PIOMAP",    PIOMAP},
  {"RESET",     RESET},
  {"RUNTEST",   RUNTEST},
  {"SCK",       SCK},
  {"SDR",       SDR},
  {"SEC",       SEC},
  {"SIR",       SIR},
  {"SMASK",     SMASK},
  {"STATE",     STATE},
  {"TCK",       TCK},
  {"TDI",       TDI},
  {"TDO",       TDO},
  {"TDR",       TDR},
  {"TIR",       TIR},
  {"TRST",      TRST},
  {"U",         U},
  {"X",         X},
  {"Z",         Z}
};//end of rwtable struct

#define END(v) (sizeof(v) / sizeof(v[0]) - 1)

static int
map_keyw_ident(YYSTYPE * yylval, char *str)
{
  int idx;
  int rw = IDENTIFIER;

  yylval->cvalue = str;

  for (idx = 0; idx <= END(rwtable); idx++) {
    if (strcasecmp(rwtable[idx].rw_name, str) == 0) {
      /* always return terminal value as semantic value */
      rw = rwtable[idx].rw_yylex;
      yylval->token = rw;
    }
  }

  return(rw);
}


static void
align_string(char *str)
{
  int src, dst, len;

  dst = 0;
  len = strlen(str);

  for (src = 0; src < len; src++) {
    if (isxdigit(str[src]))
      str[dst++] = str[src];
  }
  str[dst] = '\0';
}


#ifdef HAVE_MODERN_FLEX
static void
fix_yylloc(YYLTYPE * yylloc, char *str)
{
  yylloc->first_line = yylloc->last_line;
  yylloc->first_column = yylloc->last_column;
  yylloc->last_column += strlen(str);
}


static void
fix_yylloc_nl(YYLTYPE * yylloc, char *str)
{
  char *p;

  yylloc->first_line = yylloc->last_line;
  yylloc->first_column = yylloc->last_column;
  for (p = str; *p; ++p) {
    if (*p == '\n') {
      yylloc->last_column = 0;
      ++yylloc->last_line;
    } else {
      ++yylloc->last_column;
    }
  }
}
#endif /* HAVE_MODERN_FLEX */
