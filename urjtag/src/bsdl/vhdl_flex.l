/*
 * $Id$
 *
 * Original Lex source by Ken Parker, 1990
 * Extensions and adaptions for UrJTAG by Arnim Laeuger, 2007
 *
 */

/*----------------------------------------------------------
 *
 *  Lex source for BSDL
 *
 *----------------------------------------------------------*/
/* Date:   901003 */

/*

Email header accompanying the original Yacc code:
  http://www.eda.org/vug_bbs/bsdl.parser

-----------------------------------8<--------------------------------------

Hello All,

This is this first mailing of the BSDL* Version 0.0 parser specifications
we are sending to people who request it from our publicized E-Mail address;

             bsdl%hpmtlx@hplabs.HP.com

You are free to redistribute this at will, but we feel that it would be
better if respondents asked for it directly so that their addresses can
be entered into our list for future mailings and updates.

It would be helpful if you could confirm receipt of this transmission.
We also would be very interested to hear about your experiences with this
information and what you are planning to do with BSDL.

Regards,

Ken Parker
Hewlett-Packard Company


*Boundary-Scan Description Language - as documented in:

"A Language for Describing Boundary-Scan Devices", K.P. Parker
and S. Oresjo, Proceedings 1990 International Test Conference,
Washington DC, pp 222-234


- -----------------cut here---------------------------------------------------


901004.0721                                  Hewlett-Packard Company
901016.1049                                  Manufacturing Test Division
                                             P.O. Box 301
                                             Loveland, Colorado  80537
                                             USA

                                                             October 1990
Hello BSDL Parser Requestor,

   This Electronic Mail reply contains the computer specifications for
Hewlett-Packard's Version 0.0 BSDL parser.  This section of the reply
explains the contents of the rest of this file.

This file is composed of seven (7) parts:

   1) How to use this file

   2) UNIX* Lex source  (lexicographical tokenizing rules)

   3) UNIX* Yacc source (BNF-like syntax description)

   4) A sample main program to recognize BSDL.

   5) A BSDL description of the Texas Instruments 74bct8374 that is
      recognized by the parser, for testing purposes.

   6) The VHDL package STD_1149_1_1990 needed by this parser.

   7) [added 901016] Porting experiences to other systems.


RECOMMENDATION: Save a copy of this file in archival storage before
                processing it via the instructions below.  This will
                allow you to recover from errors, and allow you to
                compare subsequently released data for changes.

DISCLAIMERS:

1.  The IEEE 1149.1 Working Group has not endorsed BSDL Version 0.0 and
    therefore no person may represent it as an IEEE standard or imply that
    a resulting IEEE standard will be identical to it.

2.  The IEEE 1149.1 Working Group recognizes that BSDL Version 0.0 is a
    well-conceived initiative that is likely to excelerate the creation
    of tools that support the 1149.1 standard.  As such, changes and
    enhancements will be carefully considered so as not to needlessly
    disrupt these development efforts.  The overriding goal is the
    ultimate success of the 1149.1 standard.

LEGAL NOTICES:

    Hewlett-Packard Company makes no warranty of any kind with regard to
    this information, including, but not limited to, the implied
    waranties of merchantability and fitness for a particular purpose.

    Hewlett-Packard Company shall not be liable for errors contained
    herein or direct, indirect, special, incidental, or consequential
    damages in connection with the furnishing, performance, or use of
    this material.


*UNIX is a trademark of AT&T in the USA and other countries.

*/

%option bison-bridge
%option reentrant
%option prefix="urj_vhdl_"
%option outfile="lex.yy.c"

%{
/* Begin lex input specifications */
#include <sysdep.h>
/* Undef TRUE and FALSE to avoid redefinition error
   when compiling for Windows host.  */
#undef TRUE
#undef FALSE

#include <stdlib.h>
#include <ctype.h>

#include <urjtag/jtag.h>

#include "bsdl_sysdep.h"

#include "bsdl_msg.h"

#include "vhdl_bison.h"
#include "vhdl_parser.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define YY_EXTRA_TYPE urj_bsdl_scan_extra_t *

static char *new_string (urj_bsdl_scan_extra_t *, const char *);

#define BINARY  0
#define DECIMAL 1
#define BIN_X   2

/* Fix up warnings from generated lex code */
#define lex_get_column yyget_column
#define lex_set_column yyset_column
#include "lex.h"

%}
%a  2800
%e  1200
%p  4700
%n   520
%k   140
%o  3000
%START RES BOU REG PAC
White          [ \t\b\r]*
VHDL_Comment   \-\-[^\n]*
Bin_X_Pattern  [0-1X]+
Digit          [0-9]
Exponent       [E][+-]?{Digit}+
Real_Number    {Digit}+[\.]{Digit}+{Exponent}
Decimal_Number {Digit}+
Identifier     [A-Z][A-Z0-9_]*
Quoted_String  \"[^\"\n]*\"
Concatenate    &
Single_Quote   [\']
Semicolon      ;
Eol            [\n]
Comma          [,]
Lparen         [\(]
Rparen         [\)]
Colon_Equal    \:\=
Colon          [\:]
Period         [\.]
Box            [\<][\>]
Illegal        [^A-Z0-9 \t\b\n\r\"\&\'\*\(\)\[\]\_\:\.\,\;\<\>]
Entity         ENTITY
Port           PORT
Generic        GENERIC
Use            USE
Attribute      ATTRIBUTE
Is             IS
Of             OF
Constant       CONSTANT
String         STRING
End            END
All            ALL
Physical_Pin_Map PHYSICAL_PIN_MAP
Pin_Map_String PIN_MAP_STRING
True           TRUE
False          FALSE
Signal         SIGNAL
Low            LOW
Both           BOTH
Out            OUT
Inout          INOUT
In             IN
Buffer         BUFFER
Linkage        LINKAGE
Bit_Vector     BIT_VECTOR
Bit            BIT
To             TO
Downto         DOWNTO
Package        PACKAGE
Body           BODY
Type           TYPE
Subtype        SUBTYPE
Record         RECORD
Array          ARRAY
Positive       POSITIVE
Range          RANGE
Cell_Info      CELL_INFO
Input          INPUT
Output2        OUTPUT2
Output3        OUTPUT3
Control        CONTROL
Controlr       CONTROLR
Internal       INTERNAL
Clock          CLOCK
Observe_Only   OBSERVE_ONLY
Bidir          BIDIR
Bidir_In       BIDIR_IN
Bidir_Out      BIDIR_OUT
Z              Z
Extest         EXTEST
Sample         SAMPLE
Intest         INTEST
Runbist        RUNBIST
Pi             PI
Po             PO
Upd            UPD
Cap            CAP
X              X
Zero           ZERO
One            ONE
Bsdl_Extension BSDL_EXTENSION
Std_1532_2001  STD_1532_2001
Std_1532_2002  STD_1532_2002
%%
{Entity}                {return (ENTITY); }
{Port}                  {yyextra->Base = DECIMAL;  /* Default number base */
                         return (PORT); }
{Generic}               {yyextra->Base = DECIMAL; return (GENERIC); }
{Use}                   {yyextra->Base = DECIMAL; return (USE); }
{Attribute}             {yyextra->Base = DECIMAL; return (ATTRIBUTE); }
{Is}                    {return (IS); }
{Constant}              {yyextra->Base = DECIMAL; return (CONSTANT); }
{String}                {return (STRING); }
{End}                   {yyextra->Base = DECIMAL;
                         BEGIN INITIAL; /* Turn off start conditions */
                         return (END); }
{All}                   {return (ALL); }
{Of}                    {return (OF); }
{Physical_Pin_Map}      {return (PHYSICAL_PIN_MAP); }
{Pin_Map_String}        {return (PIN_MAP_STRING); }
{True}                  {return (TRUE); }
{False}                 {return (FALSE); }
{Signal}                {return (SIGNAL); }
{Low}                   {return (LOW); }
{Both}                  {return (BOTH); }
{In}                    {return (IN); }
{Out}                   {return (OUT); }
{Inout}                 {return (INOUT); }
{Buffer}                {return (BUFFER); }
{Linkage}               {return (LINKAGE); }
{Bit}                   {return (BIT); }
{Bit_Vector}            {return (BIT_VECTOR); }
{To}                    {return (TO); }
{Downto}                {return (DOWNTO); }
{Package}               {return (PACKAGE); }
{Body}                  {return (BODY); }
{Type}                  {return (TYPE); }
{Subtype}               {return (SUBTYPE); }
{Record}                {return (RECORD); }
{Array}                 {yyextra->Base = DECIMAL; return (ARRAY); }
{Positive}              {return (POSITIVE); }
{Range}                 {return (RANGE); }
{Cell_Info}             {BEGIN PAC; return (CELL_INFO); }
{Bsdl_Extension}        {return (BSDL_EXTENSION); }
{Std_1532_2001}         {return (STD_1532_2001); }
{Std_1532_2002}         {return (STD_1532_2002); }
<BOU,PAC>{Input}        {return (INPUT); }
<BOU,PAC>{Output2}      {return (OUTPUT2); }
<BOU,PAC>{Output3}      {return (OUTPUT3); }
<BOU,PAC>{Controlr}     {return (CONTROLR); }
<BOU,PAC>{Control}      {return (CONTROL); }
<BOU,PAC>{Internal}     {return (INTERNAL); }
<BOU,PAC>{Clock}        {return (CLOCK); }
<BOU,PAC>{Observe_Only} {return (OBSERVE_ONLY); }
<BOU>{Bidir}            {return (BIDIR); }
<PAC>{Bidir_In}         {return (BIDIR_IN); }
<PAC>{Bidir_Out}        {return (BIDIR_OUT); }
<PAC>{Extest}           {return (EXTEST); }
<PAC>{Sample}           {return (SAMPLE); }
<PAC>{Intest}           {return (INTEST); }
<PAC>{Runbist}          {return (RUNBIST); }
<PAC>{Pi}               {return (PI); }
<PAC>{Po}               {return (PO); }
<PAC>{Upd}              {return (UPD); }
<PAC>{Cap}              {return (CAP); }
<PAC>{X}                {return (X); }
<PAC>{Zero}             {return (ZERO); }
<PAC>{One}              {return (ONE); }
<BOU>{Z}                {return (Z); }
{Eol}                   {yylineno++; /* Count lines */}
{Comma}                 {return (COMMA); }
{Lparen}                {return (LPAREN);}
{Rparen}                {return (RPAREN);}
{Period}                {return (PERIOD); }
{Colon}                 {return (COLON);  }
{Box}                   {return (BOX);  }
{Single_Quote}          {yyextra->Base = BIN_X; return (SINGLE_QUOTE); }
{Colon_Equal}           {return (COLON_EQUAL); }
{White}                 { /* Do Nothing on White Space */ }
{VHDL_Comment}          { /* Do Nothing on Comments    */ }
{Bin_X_Pattern}         {if (yyextra->Base != BIN_X) REJECT;
                         yylval->str = new_string (yyextra, yytext);
                         return (BIN_X_PATTERN);}
{Identifier}            {yylval->str = new_string (yyextra, yytext);
                         return (IDENTIFIER); }
{Quoted_String}         {yylval->str = new_string (yyextra, yytext);
                         return (QUOTED_STRING); }
{Decimal_Number}        {if (yyextra->Base != DECIMAL) REJECT;
                         yylval->integer = atoi ((char *)yytext);
                         return (DECIMAL_NUMBER);}
{Real_Number}           {yylval->str = new_string (yyextra, yytext);
                         return (REAL_NUMBER);}
{Concatenate}           {return (CONCATENATE);}
{Semicolon}             {return (SEMICOLON);}
{Illegal}               {urj_bsdl_err_set (yyextra->proc_mode, URJ_ERROR_BSDL_VHDL,
                                           "Illegal character %c (/%03o) at line %d:",
                                           (char)yytext[yyleng-1], (int)yytext[yyleng-1],
                                           yylineno);
                         yyextra->Compile_Errors++;
                         return (ILLEGAL);  /* Will cause syntax error */}
<<EOF>> {
  yypop_buffer_state (yyscanner);
  if (!YY_CURRENT_BUFFER)
    yyterminate();
}

%%

/*****************************************************************************
 * void *urj_vhdl_flex_init( FILE *f, int mode )
 *
 * Initializes the scanner and storage elements extra data structure.
 *
 * Parameters
 *   f         : descriptor of file for scanning
 *   proc_mode : processing mode, consisting of BSDL_MODE_* bits
 *
 * Returns
 *   pointer to newly initialized scanner structure
 ****************************************************************************/
void *
urj_vhdl_flex_init (FILE *f, int proc_mode)
{
    urj_bsdl_scan_extra_t *extra;
    yyscan_t scanner;

    /* get our scanner structure */
    if (yylex_init (&scanner) != 0)
    {
        urj_bsdl_ftl_set (proc_mode, URJ_ERROR_BSDL_VHDL,
                          "Scanner could not be initialized");
        return NULL;
    }
    yyset_in (f, scanner);

    if (!(extra = malloc (sizeof (urj_bsdl_scan_extra_t))))
    {
        urj_bsdl_ftl_set (proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
        yylex_destroy (scanner);
        return NULL;
    }

    extra->proc_mode = proc_mode;
    extra->Compile_Errors = 0;
    extra->Base = DECIMAL;

    yyset_extra (extra, scanner);

    return scanner;
}


/*****************************************************************************
 * void urj_vhdl_flex_deinit( void *scanner )
 *
 * Deinitializes the scanner and closes the input file.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_vhdl_flex_deinit (void *scanner)
{
    if (yyget_in (scanner))
    {
        /* file might still be open so close it in any case
           e.g. when a compile error occured and the parser didn't hit EOF/yywrap() */
        fclose (yyget_in (scanner));
        yyset_in (NULL, scanner);
    }
    free (yyget_extra (scanner));
    yylex_destroy (scanner);
}


/*****************************************************************************
 * void yywrap( yyscan_t scanner )
 *
 * Closes the input file upon EOF condition.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   1
 ****************************************************************************/
int
yywrap (yyscan_t scanner)
{
    if (yyget_in (scanner))
    {
        fclose (yyget_in (scanner));
        yyset_in (NULL, scanner);
    }
    return 1;
}


/*****************************************************************************
 * char *new_string( urj_bsdl_scan_extra_t * extra, const char *str )
 *
 * Allocates memory for a string and copies the contents of *str.
 *
 * Parameters
 *   extra : pointer to extra data structure
 *   str   : pointer to string to be duplicated
 *
 * Returns
 *   pointer to allocated and initialized string memory
 ****************************************************************************/
static char *
new_string (urj_bsdl_scan_extra_t *extra, const char *str)
{
    char *n_str;
    size_t n_str_size;

    n_str_size = strlen (str) + 1;
    if ((n_str = malloc (n_str_size)))
    {
        strncpy (n_str, str, n_str_size - 1);
        n_str[n_str_size - 1] = '\0';   /* set very last element to EOS */
    }
    else
        urj_bsdl_ftl_set (extra->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");

    return n_str;
}


/*****************************************************************************
 * void urj_vhdl_flex_switch_file( yyscan_t scanner, char *filename )
 *
 * Selects the specified file for further scanning.
 *
 * Parameters
 *   scanner  : pointer to scanner definition structure
 *   filename : name of the new file to be scanned
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_vhdl_flex_switch_file (yyscan_t scanner, char *filename)
{
    urj_bsdl_scan_extra_t *extra;
    FILE *f;
    char *s;

    /* convert filename to all upper case */
    s = filename;
    while (*s)
    {
        if (islower (*s))
            *s = toupper (*s);
        s++;
    }

    extra = yyget_extra (scanner);

    /* file in current directory has precedence */
    f = fopen (filename, FOPEN_R);
    if (!f)
    {
        const char *db_path = urj_get_data_dir ();
        char *db_file;

        /* @@@@ RFHH handle malloc failure */
        if ((db_file = malloc (strlen (db_path) + 1 +   /* "/"    */
                               4 +      /* "bsdl" */
                               1 +      /* "/"    */
                               strlen (filename) + 1)))
        {
            strcpy (db_file, db_path);
            strcat (db_file, "/");
            strcat (db_file, "bsdl");
            strcat (db_file, "/");
            strcat (db_file, filename);
            f = fopen (db_file, FOPEN_R);

            if (!f)
                urj_bsdl_ftl_set (extra->proc_mode, URJ_ERROR_IO,
                                  "Cannot open file %s or %s", filename,
                                  db_file);
            free (db_file);
            if (!f)
                return;
        }
    }

    yypush_buffer_state (yy_create_buffer (f, YY_BUF_SIZE, scanner), scanner);
    yyset_lineno (1, scanner);
}


/*****************************************************************************
 * int urj_vhdl_flex_get_compile_errors( yyscan_t scanner )
 *
 * Retrieves the accumulated number of compile errors encountered so far.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   number of compile errors
 ****************************************************************************/
int
urj_vhdl_flex_get_compile_errors (yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    return extra->Compile_Errors;
}


/*****************************************************************************
 * void urj_vhdl_flex_set_compile_errors( int n, yyscan_t scanner )
 *
 * Sets the number of compile errors to n.
 *
 * Parameters
 *   n       : preset number
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   void
 ****************************************************************************/
static void
urj_vhdl_flex_set_compile_errors (int n, yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    extra->Compile_Errors = n;
}


/*****************************************************************************
 * int urj_vhdl_flex_postinc_compile_errors( yyscan_t scanner )
 *
 * Increments the number of compile errors.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   number of compile errors before increment
 ****************************************************************************/
int
urj_vhdl_flex_postinc_compile_errors (yyscan_t scanner)
{
    int errors = urj_vhdl_flex_get_compile_errors (scanner);

    urj_vhdl_flex_set_compile_errors (errors + 1, scanner);
    return errors;
}


/*****************************************************************************
 * int urj_vhdl_flex_get_lineno( yyscan_t scanner )
 *
 * Returns the current line number in the scanned file.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   current line number
 ****************************************************************************/
int
urj_vhdl_flex_get_lineno (yyscan_t scanner)
{
    return yyget_lineno (scanner);
}


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
