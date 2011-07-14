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
%option prefix="urj_bsdl_"
%option outfile="lex.yy.c"

%{
/* Begin lex input specifications */

#include <stdlib.h>
#include <ctype.h>

#include "bsdl_sysdep.h"

#include "bsdl_msg.h"

#include "bsdl_bison.h"
#include "bsdl_parser.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define YY_EXTRA_TYPE urj_bsdl_scan_extra_t *

static char *new_string (urj_bsdl_scan_extra_t *, const char *);

#define BINARY  0
#define DECIMAL 1
#define BIN_X   2
#define HEX     3

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
%START RES BOU REG PAC ISC
White          [ \t\b\r]*
VHDL_Comment   \-\-[^\n]*
Binary_Pattern [0-1]+
Bin_X_Pattern  [0-1X]+
Hex_String     [0-9A-F]+
Digit          [0-9]
Exponent       [E][+-]?{Digit}+
Real_Number    {Digit}+[\.]{Digit}+{Exponent}
Decimal_Number {Digit}+
Identifier     [A-Z][A-Z0-9_]*
Eol            [\n]
Comma          [,]
Lparen         [\(]
Rparen         [\)]
Lbracket       [\[]
Rbracket       [\]]
Colon          [\:]
Asterisk       [\*]
Dollar         [\$]
Equal          [\=]
Plus           [\+]
Minus          [\-]
Sh_Right       [\>][\>]
Sh_Left        [\<][\<]
Tilde          [\~]
Question_Exclamation [\?][\!]
Question       [\?]
Exclamation    [\!]
Illegal        [^A-Z0-9 \t\b\n\r\"\&\'\*\(\)\[\]\_\:\.\,\;\$\=\+\-\>\<\~\?\!]
Constant       CONSTANT
Physical_Pin_Map PHYSICAL_PIN_MAP
Pin_Map        PIN_MAP
Pin_Map_String PIN_MAP_STRING
Tap_Scan_In    TAP_SCAN_IN
Tap_Scan_Out   TAP_SCAN_OUT
Tap_Scan_Clock TAP_SCAN_CLOCK
Tap_Scan_Mode  TAP_SCAN_MODE
Tap_Scan_Reset TAP_SCAN_RESET
Instruction_Length   INSTRUCTION_LENGTH
Instruction_Opcode   INSTRUCTION_OPCODE
Instruction_Capture  INSTRUCTION_CAPTURE
Instruction_Disable  INSTRUCTION_DISABLE
Instruction_Guard    INSTRUCTION_GUARD
Instruction_Private  INSTRUCTION_PRIVATE
Register_Access      REGISTER_ACCESS
Boundary_Length      BOUNDARY_LENGTH
Boundary_Register    BOUNDARY_REGISTER
Idcode_Register      IDCODE_REGISTER
Usercode_Register    USERCODE_REGISTER
Boundary       BOUNDARY
Bypass         BYPASS
Clamp          CLAMP
Extest         EXTEST
Highz          HIGHZ
Idcode         IDCODE
Intest         INTEST
Preload        PRELOAD
Runbist        RUNBIST
Sample         SAMPLE
Usercode       USERCODE
Device_Id      DEVICE_ID
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
Compliance_Patterns   COMPLIANCE_PATTERNS
Component_Conformance COMPONENT_CONFORMANCE
Std_1149_1_1990       STD_1149_1_1990
Std_1149_1_1993       STD_1149_1_1993
Std_1149_1_2001       STD_1149_1_2001
ISC_Conformance       ISC_CONFORMANCE
Std_1532_2001         STD_1532_2001
Std_1532_2002         STD_1532_2002
ISC_Pin_Behavior      ISC_PIN_BEHAVIOR
ISC_Fixed_System_Pins ISC_FIXED_SYSTEM_PINS
ISC_Status            ISC_STATUS
Implemented           IMPLEMENTED
ISC_Blank_Usercode    ISC_BLANK_USERCODE
ISC_Security          ISC_SECURITY
ISC_Disable_Read      ISC_DISABLE_READ
ISC_Disable_Program   ISC_DISABLE_PROGRAM
ISC_Disable_Erase     ISC_DISABLE_ERASE
ISC_Disable_Key       ISC_DISABLE_KEY
ISC_Flow              ISC_FLOW
Unprocessed           UNPROCESSED
Exit_On_Error         EXIT_ON_ERROR
Array                 ARRAY
Security              SECURITY
Initialize            INITIALIZE
Repeat                REPEAT
Terminate             TERMINATE
Loop                  LOOP
Wait                  WAIT
Min                   MIN
Max                   MAX
Crc                   CRC
Ost                   OST
ISC_Procedure         ISC_PROCEDURE
ISC_Action            ISC_ACTION
Proprierary           PROPRIETARY
Optional              OPTIONAL
Recommended           RECOMMENDED
ISC_Illegal_Exit      ISC_ILLEGAL_EXIT
%%
{Constant}              {yyextra->Base = DECIMAL; return( CONSTANT ); }
{Pin_Map}               {return( PIN_MAP ); }
{Physical_Pin_Map}      {return( PHYSICAL_PIN_MAP ); }
{Pin_Map_String}        {return( PIN_MAP_STRING ); }
{Tap_Scan_In}           {return( TAP_SCAN_IN ); }
{Tap_Scan_Out}          {return( TAP_SCAN_OUT ); }
{Tap_Scan_Clock}        {return( TAP_SCAN_CLOCK ); }
{Tap_Scan_Mode}         {return( TAP_SCAN_MODE ); }
{Tap_Scan_Reset}        {return( TAP_SCAN_RESET ); }
{Instruction_Length}    {yyextra->Base = DECIMAL; return( INSTRUCTION_LENGTH ); }
{Instruction_Opcode}    {yyextra->Base = BINARY;  /* Enable Binary Patterns */
                         return( INSTRUCTION_OPCODE ); }
{Instruction_Capture}   {yyextra->Base = BIN_X;  /* Enable Bin_X Patterns */
                         return( INSTRUCTION_CAPTURE ); }
{Instruction_Disable}   {return( INSTRUCTION_DISABLE ); }
{Instruction_Guard}     {return( INSTRUCTION_GUARD ); }
{Instruction_Private}   {return( INSTRUCTION_PRIVATE ); }
{Register_Access}       {BEGIN(REG); yyextra->Base = DECIMAL;
                         return( REGISTER_ACCESS ); }
{Boundary_Length}       {yyextra->Base = DECIMAL; return( BOUNDARY_LENGTH ); }
{Boundary_Register}     {BEGIN(BOU); yyextra->Base = DECIMAL;
                         return( BOUNDARY_REGISTER ); }
{Idcode_Register}       {yyextra->Base = BIN_X; return( IDCODE_REGISTER ); }
{Usercode_Register}     {yyextra->Base = BIN_X; return( USERCODE_REGISTER ); }
{Compliance_Patterns}   {yyextra->Base = DECIMAL; return( COMPLIANCE_PATTERNS ); }
{Component_Conformance} {return( COMPONENT_CONFORMANCE ); }
{Std_1149_1_1990}       {return( STD_1149_1_1990 ); }
{Std_1149_1_1993}       {return( STD_1149_1_1993 ); }
{Std_1149_1_2001}       {return( STD_1149_1_2001 ); }
{ISC_Conformance}       {yyextra->Base = DECIMAL; return( ISC_CONFORMANCE ); }
{Std_1532_2001}         {return( STD_1532_2001 ); }
{Std_1532_2002}         {return( STD_1532_2002 ); }
{ISC_Pin_Behavior}      {BEGIN(ISC); yyextra->Base = DECIMAL; return( ISC_PIN_BEHAVIOR ); }
{ISC_Fixed_System_Pins} {yyextra->Base = DECIMAL; return( ISC_FIXED_SYSTEM_PINS ); }
{ISC_Status}            {yyextra->Base = DECIMAL; return( ISC_STATUS ); }
<ISC>{Implemented}      {return( IMPLEMENTED ); }
{ISC_Blank_Usercode}    {yyextra->Base = BIN_X; return( ISC_BLANK_USERCODE ); }
{ISC_Security}          {yyextra->Base = DECIMAL; return( ISC_SECURITY ); }
{ISC_Disable_Read}      {yyextra->Base = DECIMAL; return( ISC_DISABLE_READ ); }
{ISC_Disable_Program}   {yyextra->Base = DECIMAL; return( ISC_DISABLE_PROGRAM ); }
{ISC_Disable_Erase}     {yyextra->Base = DECIMAL; return( ISC_DISABLE_ERASE ); }
{ISC_Disable_Key}       {yyextra->Base = DECIMAL; return( ISC_DISABLE_KEY ); }
{ISC_Flow}              {yyextra->Base = DECIMAL; return( ISC_FLOW ); }
<ISC>{Unprocessed}      {return( UNPROCESSED ); }
<ISC>{Exit_On_Error}    {return( EXIT_ON_ERROR ); }
<ISC>{Array}            {return( ARRAY ); }
<ISC>{Security}         {return( SECURITY ); }
<ISC>{Preload}          {return( PRELOAD ); }
<ISC>{Initialize}       {return( INITIALIZE ); }
<ISC>{Repeat}           {yyextra->Base = DECIMAL; return( REPEAT ); }
<ISC>{Terminate}        {return( TERMINATE ); }
<ISC>{Loop}             {return( LOOP ); }
<ISC>{Wait}             {return( WAIT ); }
<ISC>{Min}              {return( MIN ); }
<ISC>{Max}              {return( MAX ); }
<ISC>{Crc}              {return( CRC ); }
<ISC>{Ost}              {return( OST ); }
<ISC>{Dollar}           {return( DOLLAR ); }
<ISC>{Equal}            {return( EQUAL ); }
<ISC>{Plus}             {return( PLUS ); }
<ISC>{Minus}            {return( MINUS ); }
<ISC>{Sh_Right}         {return( SH_RIGHT ); }
<ISC>{Sh_Left}          {return( SH_LEFT ); }
<ISC>{Tilde}            {return( TILDE ); }
<ISC>{Question_Exclamation}  {return( QUESTION_EXCLAMATION ); }
<ISC>{Question}         {return( QUESTION_MARK ); }
<ISC>{Exclamation}      {return( EXCLAMATION_MARK ); }
{ISC_Procedure}         {yyextra->Base = DECIMAL; return( ISC_PROCEDURE ); }
{ISC_Action}            {yyextra->Base = DECIMAL; return( ISC_ACTION ); }
<ISC>{Proprierary}      {return( PROPRIETARY ); }
<ISC>{Optional}         {return( OPTIONAL ); }
<ISC>{Recommended}      {return( RECOMMENDED ); }
{ISC_Illegal_Exit}      {return( ISC_ILLEGAL_EXIT ); }
<REG,ISC>{Bypass}       {return( BYPASS ); }
<REG,ISC>{Boundary}     {return( BOUNDARY ); }
<REG,ISC>{Clamp}        {return( CLAMP ); }
<REG,ISC>{Extest}       {return( EXTEST ); }
<REG,ISC>{Highz}        {return( HIGHZ ); }
<REG,ISC>{Idcode}       {return( IDCODE ); }
<REG,ISC>{Intest}       {return( INTEST ); }
<REG,ISC>{Preload}      {return( PRELOAD ); }
<REG,ISC>{Runbist}      {return( RUNBIST ); }
<REG,ISC>{Sample}       {return( SAMPLE ); }
<REG,ISC>{Usercode}     {return( USERCODE ); }
<REG,ISC>{Device_Id}    {return( DEVICE_ID ); }
<BOU,PAC>{Input}        {return( INPUT ); }
<BOU,PAC>{Output2}      {return( OUTPUT2 ); }
<BOU,PAC>{Output3}      {return( OUTPUT3 ); }
<BOU,PAC>{Controlr}     {return( CONTROLR ); }
<BOU,PAC>{Control}      {return( CONTROL ); }
<BOU,PAC>{Internal}     {return( INTERNAL ); }
<BOU,PAC>{Clock}        {return( CLOCK ); }
<BOU,PAC>{Observe_Only} {return( OBSERVE_ONLY ); }
<BOU>{Bidir}            {return( BIDIR ); }
<PAC>{Bidir_In}         {return( BIDIR_IN ); }
<PAC>{Bidir_Out}        {return( BIDIR_OUT ); }
<BOU>{Z}                {return( Z ); }
<BOU>{Weak0}            {return( WEAK0 ); }
<BOU>{Weak1}            {return( WEAK1 ); }
<BOU>{Pull0}            {return( PULL0 ); }
<BOU>{Pull1}            {return( PULL1 ); }
<BOU>{Keeper}           {return( KEEPER ); }
{Eol}                   {yylineno++; /* Count lines */}
{Comma}                 {return( COMMA ); }
{Lparen}                {return( LPAREN );}
{Rparen}                {return( RPAREN );}
{Lbracket}              {return( LBRACKET ); }
{Rbracket}              {return( RBRACKET ); }
{Colon}                 {return( COLON );  }
{Asterisk}              {return( ASTERISK );  }
{White}                 { /* Do Nothing on White Space */ }
{VHDL_Comment}          { /* Do Nothing on Comments    */ }
{Bin_X_Pattern}         {if (yyextra->Base != BIN_X) REJECT;
                         yylval->str = new_string( yyextra, yytext );
                         return( BIN_X_PATTERN );}
{Hex_String}            {if (yyextra->Base != HEX) REJECT;
                         yylval->str = new_string( yyextra, yytext );
                         return( HEX_STRING );}
{Identifier}            {yylval->str = new_string( yyextra, yytext );
                         return( IDENTIFIER ); }
{Binary_Pattern}        {if (yyextra->Base != BINARY) REJECT;
                         yylval->str = new_string( yyextra, yytext );
                         return( BINARY_PATTERN );}
{Decimal_Number}        {if (yyextra->Base != DECIMAL) REJECT;
                         yylval->integer = atoi( (char *)yytext );
                         return( DECIMAL_NUMBER );}
{Real_Number}           {yylval->str = new_string( yyextra, yytext );
                         return( REAL_NUMBER );}
{Illegal}               {urj_bsdl_err_set( yyextra->proc_mode, URJ_ERROR_BSDL_BSDL,
                                           "Illegal character %c (/%03o) at line %d:",
                                           (char)yytext[yyleng-1], (int)yytext[yyleng-1],
                                           yylineno );
                         yyextra->Compile_Errors++;
                         return( ILLEGAL );  /* Will cause syntax error */}
<<EOF>> {
  yypop_buffer_state( yyscanner );
  if ( !YY_CURRENT_BUFFER )
    yyterminate();
}

%%

/*****************************************************************************
 * void *urj_bsdl_flex_init( int proc_mode )
 *
 * Initializes the scanner and storage elements extra data structure.
 *
 * Parameters
 *   proc_mode : processing mode, consisting of BSDL_MODE_* bits
 *
 * Returns
 *   pointer to newly initialized scanner structure
 ****************************************************************************/
void *
urj_bsdl_flex_init (int proc_mode)
{
    urj_bsdl_scan_extra_t *extra;
    yyscan_t scanner;

    /* get our scanner structure */
    if (yylex_init (&scanner) != 0)
    {
        urj_bsdl_ftl_set (proc_mode, URJ_ERROR_BSDL_BSDL,
                          "Scanner could not be initialized");
        return NULL;
    }

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
 * void urj_bsdl_flex_deinit( void *scanner )
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
urj_bsdl_flex_deinit (void *scanner)
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
 * char *new_string( urj_bsdl_scan_extra_t *extra, const char *str )
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

    return (n_str);
}


/*****************************************************************************
 * void urj_bsdl_flex_switch_buffer( yyscan_t scanner, const char *buffer, int lineno )
 *
 * Selects the specified string buffer for further scanning.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *   buffer  : pointer to string buffer to be scanned
 *   lineno  : initial line number offset
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_flex_switch_buffer (yyscan_t scanner, const char *buffer, int lineno)
{
    /* ugly, ulgy, ugly
       prepare yyg for later use of YY_CURRENT_BUFFER */
    struct yyguts_t *yyg = (struct yyguts_t *) scanner;

    /* yy_scan_string() switches to the string buffer internally,
       so we must save the current buffer state explicitly by pushing the stack
       and setting top of stack to the current buffer state again.
       yy_scan_string() can then savely switch YY_CURRENT_BUFFER to the string buffer.
       yypop_buffer_state() will delete the string buffer afterwards and pop the saved
       current buffer state. */
    yypush_buffer_state (YY_CURRENT_BUFFER, scanner);
    yy_scan_string (buffer, scanner);
    yyset_lineno (lineno, scanner);
}


/*****************************************************************************
 * void urj_bsdl_flex_stop_buffer( yyscan_t scanner )
 *
 * Removes the current buffer from the scanner stack.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_flex_stop_buffer (yyscan_t scanner)
{
    yypop_buffer_state (scanner);
//  if ( !YY_CURRENT_BUFFER )
//    yyterminate();
}


/*****************************************************************************
 * int urj_bsdl_flex_get_compile_errors( yyscan_t scanner )
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
urj_bsdl_flex_get_compile_errors (yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    return extra->Compile_Errors;
}


/*****************************************************************************
 * void urj_bsdl_flex_set_compile_errors( int n, yyscan_t scanner )
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
urj_bsdl_flex_set_compile_errors (int n, yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    extra->Compile_Errors = n;
}


/*****************************************************************************
 * int urj_bsdl_flex_postinc_compile_errors( yyscan_t scanner )
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
urj_bsdl_flex_postinc_compile_errors (yyscan_t scanner)
{
    int errors = urj_bsdl_flex_get_compile_errors (scanner);

    urj_bsdl_flex_set_compile_errors (errors + 1, scanner);
    return errors;
}


/*****************************************************************************
 * void urj_bsdl_flex_set_bin_x( yyscan_t scanner )
 *
 * Selects BIN_X mode for pattern matching.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_flex_set_bin_x (yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    extra->Base = BIN_X;
}


/*****************************************************************************
 * void urj_bsdl_flex_set_hex( yyscan_t scanner )
 *
 * Selects HEX mode for pattern matching.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_flex_set_hex (yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    extra->Base = HEX;
}


/*****************************************************************************
 * void urj_bsdl_flex_set_decimal( yyscan_t scanner )
 *
 * Selects DECIMAL mode for pattern matching.
 *
 * Parameters
 *   scanner : pointer to scanner definition structure
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_flex_set_decimal (yyscan_t scanner)
{
    urj_bsdl_scan_extra_t *extra = yyget_extra (scanner);
    extra->Base = DECIMAL;
}


/*
 Local Variables:
 mode:C
 c-default-style:java
 indent-tabs-mode:nil
 End:
*/
