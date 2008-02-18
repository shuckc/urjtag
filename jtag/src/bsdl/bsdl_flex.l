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
%option prefix="bsdl"
%option outfile="lex.yy.c"

%{
/* Begin lex input specifications */

#include <stdlib.h>
#include <ctype.h>

#include "bsdl_sysdep.h"

#include "bsdl_bison.h"
#include "bsdl.h"

#define YY_EXTRA_TYPE scan_extra_t *

static char *new_string(const char *);

#define BINARY 0
#define DECIMAL 1
#define BIN_X 2

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
Binary_Pattern [0-1]+
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
Lbracket       [\[]
Rbracket       [\]]
Colon_Equal    \:\=
Colon          [\:]
Period         [\.]
Asterisk       [\*]
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
Pin_Map        PIN_MAP
Pin_Map_String PIN_MAP_STRING
True           TRUE
False          FALSE
Signal         SIGNAL
Tap_Scan_In    TAP_SCAN_IN
Tap_Scan_Out   TAP_SCAN_OUT
Tap_Scan_Clock TAP_SCAN_CLOCK
Tap_Scan_Mode  TAP_SCAN_MODE
Tap_Scan_Reset TAP_SCAN_RESET
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
Instruction_Length   INSTRUCTION_LENGTH
Instruction_Opcode   INSTRUCTION_OPCODE
Instruction_Capture  INSTRUCTION_CAPTURE
Instruction_Disable  INSTRUCTION_DISABLE
Instruction_Guard    INSTRUCTION_GUARD
Instruction_Private  INSTRUCTION_PRIVATE
Instruction_Usage    INSTRUCTION_USAGE
Instruction_Sequence INSTRUCTION_SEQUENCE
Register_Access      REGISTER_ACCESS
Boundary_Cells       BOUNDARY_CELLS
Boundary_Length      BOUNDARY_LENGTH
Boundary_Register    BOUNDARY_REGISTER
Idcode_Register      IDCODE_REGISTER
Usercode_Register    USERCODE_REGISTER
Design_Warning       DESIGN_WARNING
Boundary       BOUNDARY
Bypass         BYPASS
Highz          HIGHZ
Idcode         IDCODE
Device_Id      DEVICE_ID
Usercode       USERCODE
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
Weak0          WEAK0
Weak1          WEAK1
Pull0          PULL0
Pull1          PULL1
Keeper         KEEPER
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
Component_Conformance COMPONENT_CONFORMANCE
Port_Grouping         PORT_GROUPING
Runbist_Execution     RUNBIST_EXECUTION
Intest_Execution      INTEST_EXECUTION
Bsdl_Extension        BSDL_EXTENSION
Compliance_Patterns   COMPLIANCE_PATTERNS
%%
{Entity}                {return(ENTITY); }
{Port}                  {yyextra->Base = DECIMAL;  /* Default number base */
                         return(PORT); }
{Generic}               {yyextra->Base = DECIMAL; return(GENERIC); }
{Use}                   {yyextra->Base = DECIMAL; return(USE); }
{Attribute}             {yyextra->Base = DECIMAL; return(ATTRIBUTE); }
{Is}                    {return(IS); }
{Constant}              {yyextra->Base = DECIMAL; return(CONSTANT); }
{String}                {return(STRING); }
{End}                   {yyextra->Base = DECIMAL;
                         BEGIN INITIAL; /* Turn off start conditions */
                         return(END); }
{All}                   {return(ALL); }
{Of}                    {return(OF); }
{Pin_Map}               {return(PIN_MAP); }
{Physical_Pin_Map}      {return(PHYSICAL_PIN_MAP); }
{Pin_Map_String}        {return(PIN_MAP_STRING); }
{True}                  {return(TRUE); }
{False}                 {return(FALSE); }
{Signal}                {return(SIGNAL); }
{Tap_Scan_In}           {return(TAP_SCAN_IN); }
{Tap_Scan_Out}          {return(TAP_SCAN_OUT); }
{Tap_Scan_Clock}        {return(TAP_SCAN_CLOCK); }
{Tap_Scan_Mode}         {return(TAP_SCAN_MODE); }
{Tap_Scan_Reset}        {return(TAP_SCAN_RESET); }
{Low}                   {return(LOW); }
{Both}                  {return(BOTH); }
{In}                    {return(IN); }
{Out}                   {return(OUT); }
{Inout}                 {return(INOUT); }
{Buffer}                {return(BUFFER); }
{Linkage}               {return(LINKAGE); }
{Bit}                   {return(BIT); }
{Bit_Vector}            {return(BIT_VECTOR); }
{To}                    {return(TO); }
{Downto}                {return(DOWNTO); }
{Package}               {return(PACKAGE); }
{Body}                  {return(BODY); }
{Type}                  {return(TYPE); }
{Subtype}               {return(SUBTYPE); }
{Record}                {return(RECORD); }
{Array}                 {yyextra->Base = DECIMAL; return(ARRAY); }
{Positive}              {return(POSITIVE); }
{Range}                 {return(RANGE); }
{Cell_Info}             {BEGIN PAC; return(CELL_INFO); }
{Instruction_Length}    {return(INSTRUCTION_LENGTH); }
{Instruction_Opcode}    {yyextra->Base = BINARY;  /* Enable Binary Patterns */
                         return(INSTRUCTION_OPCODE); }
{Instruction_Capture}   {yyextra->Base = BIN_X;  /* Enable Bin_X Patterns */
                         return(INSTRUCTION_CAPTURE); }
{Instruction_Disable}   {return(INSTRUCTION_DISABLE); }
{Instruction_Guard}     {return(INSTRUCTION_GUARD); }
{Instruction_Private}   {return(INSTRUCTION_PRIVATE); }
{Instruction_Usage}     {return(INSTRUCTION_USAGE); }
{Instruction_Sequence}  {return(INSTRUCTION_SEQUENCE); }
{Register_Access}       {BEGIN REG; return(REGISTER_ACCESS); }
{Boundary_Cells}        {return(BOUNDARY_CELLS); }
{Boundary_Length}       {return(BOUNDARY_LENGTH); }
{Boundary_Register}     {BEGIN BOU; return(BOUNDARY_REGISTER); }
{Idcode_Register}       {yyextra->Base = BIN_X; return(IDCODE_REGISTER); }
{Usercode_Register}     {yyextra->Base = BIN_X; return(USERCODE_REGISTER); }
{Design_Warning}        {return(DESIGN_WARNING); }
{Component_Conformance} {return(COMPONENT_CONFORMANCE); }
{Port_Grouping}         {return(PORT_GROUPING); }
{Runbist_Execution}     {return(RUNBIST_EXECUTION); }
{Intest_Execution}      {return(INTEST_EXECUTION); }
{Bsdl_Extension}        {return(BSDL_EXTENSION); }
{Compliance_Patterns}   {yyextra->Base = BIN_X; return(COMPLIANCE_PATTERNS); }
<REG>{Boundary}         {return(BOUNDARY); }
<REG>{Bypass}           {return(BYPASS); }
<REG>{Highz}            {return(HIGHZ); }
<REG>{Idcode}           {return(IDCODE); }
<REG>{Usercode}         {return(USERCODE); }
<REG>{Device_Id}        {return(DEVICE_ID); }
<BOU,PAC>{Input}        {return(INPUT); }
<BOU,PAC>{Output2}      {return(OUTPUT2); }
<BOU,PAC>{Output3}      {return(OUTPUT3); }
<BOU,PAC>{Controlr}     {return(CONTROLR); }
<BOU,PAC>{Control}      {return(CONTROL); }
<BOU,PAC>{Internal}     {return(INTERNAL); }
<BOU,PAC>{Clock}        {return(CLOCK); }
<BOU,PAC>{Observe_Only} {return(OBSERVE_ONLY); }
<BOU>{Bidir}            {return(BIDIR); }
<PAC>{Bidir_In}         {return(BIDIR_IN); }
<PAC>{Bidir_Out}        {return(BIDIR_OUT); }
<PAC>{Extest}           {return(EXTEST); }
<PAC>{Sample}           {return(SAMPLE); }
<PAC>{Intest}           {return(INTEST); }
<PAC>{Runbist}          {return(RUNBIST); }
<PAC>{Pi}               {return(PI); }
<PAC>{Po}               {return(PO); }
<PAC>{Upd}              {return(UPD); }
<PAC>{Cap}              {return(CAP); }
<PAC>{X}                {return(X); }
<PAC>{Zero}             {return(ZERO); }
<PAC>{One}              {return(ONE); }
<BOU>{Z}                {return(Z); }
<BOU>{Weak0}            {return(WEAK0); }
<BOU>{Weak1}            {return(WEAK1); }
<BOU>{Pull0}            {return(PULL0); }
<BOU>{Pull1}            {return(PULL1); }
<BOU>{Keeper}           {return(KEEPER); }
{Eol}                   {yylineno++; /* Count lines */}
{Comma}                 {return(COMMA); }
{Lparen}                {return(LPAREN);}
{Rparen}                {return(RPAREN);}
{Period}                {return(PERIOD); }
{Lbracket}              {return(LBRACKET); }
{Rbracket}              {return(RBRACKET); }
{Colon}                 {return(COLON);  }
{Asterisk}              {return(ASTERISK);  }
{Box}                   {return(BOX);  }
{Single_Quote}          {yyextra->Base = BIN_X; return(SINGLE_QUOTE); }
{Colon_Equal}           {return(COLON_EQUAL); }
{White}                 { /* Do Nothing on White Space */ }
{VHDL_Comment}          { /* Do Nothing on Comments    */ }
{Bin_X_Pattern}         {if (yyextra->Base != BIN_X) REJECT;
                         yylval->str = new_string(yytext);
                         return(BIN_X_PATTERN);}
{Identifier}            {yylval->str = new_string(yytext);
                         return(IDENTIFIER); }
{Quoted_String}         {yylval->str = new_string(yytext);
                         return(QUOTED_STRING); }
{Binary_Pattern}        {if (yyextra->Base != BINARY) REJECT;
                         yylval->str = new_string(yytext);
                         return(BINARY_PATTERN);}
{Decimal_Number}        {if (yyextra->Base != DECIMAL) REJECT;
                         yylval->integer = atoi((char *)yytext);
                         return(DECIMAL_NUMBER);}
{Real_Number}           {yylval->str = new_string(yytext);
                         return(REAL_NUMBER);}
{Concatenate}           {return(CONCATENATE);}
{Semicolon}             {return(SEMICOLON);}
{Illegal}               {bsdl_msg(BSDL_MSG_ERR,
                      _("Error: Illegal character %c (/%03o) at line %d:\n"),
                          (char)yytext[yyleng-1], (int)yytext[yyleng-1],
                          yylineno);
                          yyextra->Compile_Errors++;
                         return(ILLEGAL);  /* Will cause syntax error */}
<<EOF>> {
  yypop_buffer_state(yyscanner);
  if ( !YY_CURRENT_BUFFER )
    yyterminate();
}
%%
void *bsdl_flex_init(FILE *f)  /* Change input file from STDIN to BSDL file */
{
  scan_extra_t *extra;
  yyscan_t scanner;

  /* get our scanner structure */
  if (yylex_init(&scanner) != 0) {
    bsdl_msg(BSDL_MSG_FATAL, _("Scanner could not be initialized\n") );
    return NULL;
  }
  yyset_in(f, scanner);

  if (!(extra = (scan_extra_t *)malloc(sizeof(scan_extra_t)))) {
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);
    yylex_destroy(scanner);
    return NULL;
  }

  extra->Compile_Errors = 0;
  extra->Base = DECIMAL;

  yyset_extra(extra, scanner);

  return scanner;
}

void bsdl_flex_deinit(void *scanner)
{
  free(yyget_extra(scanner));
  yylex_destroy(scanner);
}


int yywrap(yyscan_t scanner)
{
  if (yyget_in(scanner)) {
    fclose(yyget_in(scanner));
    yyset_in(NULL, scanner);
  }
  return(1);
}


static char *new_string(const char *str)
{
  char   *n_str;
  size_t  n_str_size;

  n_str_size = strlen(str) + 1;
  if ((n_str = (char *)malloc(n_str_size))) {
    strncpy(n_str, str, n_str_size-1);
    n_str[n_str_size-1] = '\0';  /* set very last element to EOS */
  } else
    bsdl_msg(BSDL_MSG_FATAL, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);

  return(n_str);
}


void bsdl_flex_switch_file(yyscan_t scanner, char *filename)
{
  scan_extra_t *extra;
  FILE *f;
  char *s;

  /* convert filename to all upper case */
  s = filename;
  while (*s) {
    if (islower(*s))
      *s = toupper(*s);
    s++;
  }

  /* file in current directory has precedence */
  f = fopen(filename, "r");
  if (!f) {
    char db_path[] = JTAG_DATA_DIR;
    char *db_file;

    if ((db_file = (char *)malloc(strlen(db_path) +
                                  1 +  /* "/"    */
                                  4 +  /* "bsdl" */
                                  1 +  /* "/"    */
                                  strlen(filename) +
                                  1))) {
      strcpy(db_file, db_path);
      strcat(db_file, "/");
      strcat(db_file, "bsdl");
      strcat(db_file, "/");
      strcat(db_file, filename);
      f = fopen(db_file, "r");

      if (!f)
        bsdl_msg(BSDL_MSG_FATAL, _("Cannot open file %s or %s.\n"), filename, db_file);
      free(db_file);
      if (!f)
        return;
    }
  }

  yypush_buffer_state(yy_create_buffer(f, YY_BUF_SIZE, scanner), scanner);
  extra = yyget_extra(scanner);
  yyset_lineno(1, scanner);
}

void bsdl_flex_switch_buffer(yyscan_t scanner, const char *buffer)
{
  /* ugly, ulgy, ugly
     prepare yyg for later use of YY_CURRENT_BUFFER */
  struct yyguts_t * yyg = (struct yyguts_t*)scanner;
  int lineno;

  lineno = yyget_lineno(scanner);
  /* yy_scan_string() switches to the string buffer internally,
     so we must save the current buffer state explicitly by pushing the stack
     and setting top of stack to the current buffer state again.
     yy_scan_string() can then savely switch YY_CURRENT_BUFFER to the string buffer.
     yypop_buffer_state() will delete the string buffer afterwards and pop the saved
     current buffer state. */
  yypush_buffer_state(YY_CURRENT_BUFFER, scanner);
  yy_scan_string(buffer, scanner);
  yyset_lineno(lineno, scanner);
}

int bsdl_flex_get_compile_errors(yyscan_t scanner)
{
  scan_extra_t *extra = yyget_extra(scanner);
  return extra->Compile_Errors;
}

static void bsdl_flex_set_compile_errors(int n, yyscan_t scanner)
{
  scan_extra_t *extra = yyget_extra(scanner);
  extra->Compile_Errors = n;
}

int bsdl_flex_postinc_compile_errors(yyscan_t scanner)
{
  int errors = bsdl_flex_get_compile_errors(scanner);

  bsdl_flex_set_compile_errors(errors+1, scanner);
  return errors;
}

int bsdl_flex_get_lineno(yyscan_t scanner)
{
  return yyget_lineno(scanner);
}
