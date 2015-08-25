/*
 * $Id$
 *
 * Original Yacc code by Ken Parker, 1990
 * Extensions and adaptions for UrJTAG by Arnim Laeuger, 2007
 *
 */

/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  Yacc code for BSDL                                                     */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/* Date:  901003 */

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


%pure-parser
%parse-param {urj_bsdl_parser_priv_t *priv_data}
/* See https://lists.gnu.org/archive/html/bug-bison/2014-02/msg00002.html */
%lex-param {void *HACK}
%defines
%name-prefix "urj_bsdl_"

%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "bsdl_sysdep.h"

#include "bsdl_types.h"
#include "bsdl_msg.h"

/* interface to flex */
#include "bsdl_bison.h"
#include "bsdl_parser.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define HACK priv_data->scanner
int yylex (YYSTYPE *, void *);

#if 1
#define ERROR_LIMIT 0
#define BUMP_ERROR \
    do { \
        if (urj_bsdl_flex_postinc_compile_errors (priv_data->scanner) > ERROR_LIMIT) \
        { \
            Give_Up_And_Quit (priv_data); \
            YYABORT; \
        } \
    } while (0)
#else
#define BUMP_ERROR \
    do { \
        Give_Up_And_Quit (priv_data); \
        YYABORT; \
    } while (0)
#endif

static void Print_Error (urj_bsdl_parser_priv_t *, const char *);
static void Print_Warning (urj_bsdl_parser_priv_t *, const char *);
static void Give_Up_And_Quit (urj_bsdl_parser_priv_t *);

/* semantic functions */
static void add_instruction (urj_bsdl_parser_priv_t *, char *, char *);
static void ac_set_register (urj_bsdl_parser_priv_t *, char *, int);
static void ac_add_instruction (urj_bsdl_parser_priv_t *, char *);
static void ac_apply_assoc (urj_bsdl_parser_priv_t *);
static void prt_add_name (urj_bsdl_parser_priv_t *, char *);
static void prt_add_bit (urj_bsdl_parser_priv_t *);
static void prt_add_range (urj_bsdl_parser_priv_t *, int, int);
static void ci_no_disable (urj_bsdl_parser_priv_t *);
static void ci_set_cell_spec_disable (urj_bsdl_parser_priv_t *, int, int,
                                      int);
static void ci_set_cell_spec (urj_bsdl_parser_priv_t *, int, char *);
static void ci_append_cell_info (urj_bsdl_parser_priv_t *, int);

void yyerror (urj_bsdl_parser_priv_t *, const char *);
%}

%union {
  int   integer;
  char *str;
}


%token CONSTANT PIN_MAP
%token PHYSICAL_PIN_MAP PIN_MAP_STRING
%token TAP_SCAN_IN TAP_SCAN_OUT TAP_SCAN_MODE TAP_SCAN_RESET
%token TAP_SCAN_CLOCK
%token INSTRUCTION_LENGTH INSTRUCTION_OPCODE INSTRUCTION_CAPTURE INSTRUCTION_DISABLE
%token INSTRUCTION_GUARD INSTRUCTION_PRIVATE
%token REGISTER_ACCESS
%token BOUNDARY_LENGTH BOUNDARY_REGISTER IDCODE_REGISTER
%token USERCODE_REGISTER BOUNDARY DEVICE_ID
%token INPUT OUTPUT2 OUTPUT3 CONTROL CONTROLR INTERNAL
%token CLOCK BIDIR BIDIR_IN BIDIR_OUT
%token Z WEAK0 WEAK1 IDENTIFIER
%token PULL0 PULL1 KEEPER
%token DECIMAL_NUMBER BINARY_PATTERN
%token BIN_X_PATTERN COMMA
%token LPAREN RPAREN LBRACKET RBRACKET COLON ASTERISK
%token COMPLIANCE_PATTERNS
%token OBSERVE_ONLY
%token BYPASS CLAMP EXTEST HIGHZ IDCODE INTEST PRELOAD RUNBIST SAMPLE USERCODE
%token COMPONENT_CONFORMANCE STD_1149_1_1990 STD_1149_1_1993 STD_1149_1_2001
%token ISC_CONFORMANCE STD_1532_2001 STD_1532_2002
%token ISC_PIN_BEHAVIOR
%token ISC_FIXED_SYSTEM_PINS
%token ISC_STATUS IMPLEMENTED
%token ISC_BLANK_USERCODE
%token ISC_SECURITY ISC_DISABLE_READ ISC_DISABLE_PROGRAM ISC_DISABLE_ERASE ISC_DISABLE_KEY
%token ISC_FLOW UNPROCESSED EXIT_ON_ERROR ARRAY SECURITY INITIALIZE REPEAT TERMINATE
%token LOOP MIN MAX DOLLAR EQUAL HEX_STRING WAIT REAL_NUMBER
%token PLUS MINUS SH_RIGHT SH_LEFT TILDE QUESTION_MARK EXCLAMATION_MARK QUESTION_EXCLAMATION
%token CRC OST
%token ISC_PROCEDURE
%token ISC_ACTION PROPRIETARY OPTIONAL RECOMMENDED
%token ISC_ILLEGAL_EXIT
%token ILLEGAL

%type <str> HEX_STRING
%type <str> BIN_X_PATTERN
%type <str> IDENTIFIER
%type <str> BINARY_PATTERN
%type <str> Binary_Pattern
%type <str> Binary_Pattern_List
%type <str> REAL_NUMBER
%type <integer> DECIMAL_NUMBER
%type <integer> Cell_Function
%type <str> Safe_Value
%type <integer> Disable_Value
%type <str> Standard_Reg
%type <str> Instruction_Name

%start BSDL_Statement

%%  /* End declarations, begin rules */

BSDL_Statement   : BSDL_Pin_Map
                 | BSDL_Map_String
                 | BSDL_Tap_Scan_In
                 | BSDL_Tap_Scan_Out
                 | BSDL_Tap_Scan_Mode
                 | BSDL_Tap_Scan_Reset
                 | BSDL_Tap_Scan_Clock
                 | BSDL_Inst_Length
                 | BSDL_Opcode
                 | BSDL_Inst_Capture
                 | BSDL_Inst_Disable
                 | BSDL_Inst_Guard
                 | BSDL_Inst_Private
                 | BSDL_Idcode_Register
                 | BSDL_Usercode_Register
                 | BSDL_Register_Access
                 | BSDL_Boundary_Length
                 | BSDL_Boundary_Register
                 | BSDL_Compliance_Patterns
                 | BSDL_Component_Conformance
                 | ISC_Extension
                 | error
                   {
                     Print_Error (priv_data, _("Unsupported BSDL construct found"));
                     BUMP_ERROR;
                     YYABORT;
                   }
;

/****************************************************************************/
BSDL_Pin_Map : PIN_MAP PHYSICAL_PIN_MAP
;

/****************************************************************************/
BSDL_Map_String   : PIN_MAP_STRING Pin_Mapping
                  | BSDL_Map_String COMMA Pin_Mapping
;
Pin_Mapping       : IDENTIFIER COLON Physical_Pin_Desc
                    { free ($1); }
;
Physical_Pin_Desc : Physical_Pin
                  | LPAREN Physical_Pin_List RPAREN
;
Physical_Pin_List : Physical_Pin
                  | Physical_Pin_List COMMA Physical_Pin
;
Physical_Pin      : IDENTIFIER
                    { free ($1); }
                  | IDENTIFIER LPAREN DECIMAL_NUMBER RPAREN
                    { free ($1); }
                  | DECIMAL_NUMBER
;

/****************************************************************************/
BSDL_Tap_Scan_In : TAP_SCAN_IN DECIMAL_NUMBER
;

/****************************************************************************/
BSDL_Tap_Scan_Out : TAP_SCAN_OUT DECIMAL_NUMBER
;

/****************************************************************************/
BSDL_Tap_Scan_Mode : TAP_SCAN_MODE DECIMAL_NUMBER
;

/****************************************************************************/
BSDL_Tap_Scan_Reset : TAP_SCAN_RESET DECIMAL_NUMBER
;

/****************************************************************************/
BSDL_Tap_Scan_Clock : TAP_SCAN_CLOCK DECIMAL_NUMBER
;

/****************************************************************************/
BSDL_Inst_Length : INSTRUCTION_LENGTH DECIMAL_NUMBER
                   { priv_data->jtag_ctrl->instr_len = $2; }
;

/****************************************************************************/
BSDL_Opcode         : INSTRUCTION_OPCODE BSDL_Opcode_Table
;
BSDL_Opcode_Table   : Opcode_Desc
                    | BSDL_Opcode_Table COMMA Opcode_Desc
                    | error
                      {
                        Print_Error (priv_data,
                                     _("Error in Instruction_Opcode attribute statement"));
                        BUMP_ERROR;
                        YYABORT;
                      }
;
Opcode_Desc         : IDENTIFIER LPAREN Binary_Pattern_List RPAREN
                      { add_instruction (priv_data, $1, $3); }
;
Binary_Pattern_List : Binary_Pattern
                      { $$ = $1; }
                    | Binary_Pattern_List COMMA Binary_Pattern
                      {
                        Print_Warning (priv_data,
                                       _("Multiple opcode patterns are not supported, first pattern will be used"));
                        $$ = $1;
                        free ($3);
                      }
;
Binary_Pattern      : BINARY_PATTERN
                      { $$ = $1; }
;

/****************************************************************************/
BSDL_Inst_Capture : INSTRUCTION_CAPTURE BIN_X_PATTERN
                    { free ($2); }
;

/****************************************************************************/
BSDL_Inst_Disable : INSTRUCTION_DISABLE IDENTIFIER
                    { free ($2); }
;

/****************************************************************************/
BSDL_Inst_Guard : INSTRUCTION_GUARD IDENTIFIER
                  { free ($2); }
;

/****************************************************************************/
BSDL_Inst_Private : INSTRUCTION_PRIVATE Private_Opcode_List
;
Private_Opcode_List : Private_Opcode
                    | Private_Opcode_List COMMA Private_Opcode
                    | error
                      {
                        Print_Error (priv_data, _("Error in Opcode List"));
                        BUMP_ERROR;
                        YYABORT;
                      }
;
Private_Opcode      : IDENTIFIER
                      { free ($1); }
;

/****************************************************************************/
BSDL_Idcode_Register : IDCODE_REGISTER BIN_X_PATTERN
                       { priv_data->jtag_ctrl->idcode = $2; }
;

/****************************************************************************/
BSDL_Usercode_Register : USERCODE_REGISTER BIN_X_PATTERN
                         { priv_data->jtag_ctrl->usercode = $2; }
;

/****************************************************************************/
BSDL_Register_Access : REGISTER_ACCESS Register_String
;
Register_String      : Register_Assoc
                     | Register_String COMMA Register_Assoc
;
Register_Assoc       : Register_Decl LPAREN Reg_Opcode_List RPAREN
                       { ac_apply_assoc (priv_data); }
;
Register_Decl        : Standard_Reg
                       { ac_set_register (priv_data, $1, 0); }
                     | IDENTIFIER LBRACKET DECIMAL_NUMBER RBRACKET
                       { ac_set_register (priv_data, $1, $3); }
;
Standard_Reg         : BOUNDARY
                       { $$ = strdup ("BOUNDARY"); }
                     | BYPASS
                       { $$ = strdup ("BYPASS"); }
                     | IDCODE
                       { $$ = strdup ("IDCODE"); }
                     | USERCODE
                       { $$ = strdup ("USERCODE"); }
                     | DEVICE_ID
                       { $$ = strdup ("DEVICE_ID"); }
;
Reg_Opcode_List      : Reg_Opcode
                     | Reg_Opcode_List COMMA Reg_Opcode
;
Instruction_Name     : BYPASS
                       { $$ = strdup ("BYPASS"); }
                     | CLAMP
                       { $$ = strdup ("CLAMP"); }
                     | EXTEST
                       { $$ = strdup ("EXTEST"); }
                     | HIGHZ
                       { $$ = strdup ("HIGHZ"); }
                     | IDCODE
                       { $$ = strdup ("IDCODE"); }
                     | INTEST
                       { $$ = strdup ("INTEST"); }
                     | PRELOAD
                       { $$ = strdup ("PRELOAD"); }
                     | RUNBIST
                       { $$ = strdup ("RUNBIST"); }
                     | SAMPLE
                       { $$ = strdup ("SAMPLE"); }
                     | USERCODE
                       { $$ = strdup ("USERCODE"); }
                     | IDENTIFIER
                       { $$ = $1; }
;
Reg_Opcode           : Instruction_Name
                       { ac_add_instruction (priv_data, $1); }
;


/****************************************************************************/
BSDL_Boundary_Length : BOUNDARY_LENGTH DECIMAL_NUMBER
                       { priv_data->jtag_ctrl->bsr_len = $2; }
;

/****************************************************************************/
BSDL_Boundary_Register : BOUNDARY_REGISTER BSDL_Cell_Table
;
BSDL_Cell_Table : Cell_Entry
                | BSDL_Cell_Table COMMA Cell_Entry
                | error
                  {Print_Error (priv_data, _("Error in Boundary Cell description"));
                   BUMP_ERROR; YYABORT; }
;
Cell_Entry      : DECIMAL_NUMBER LPAREN Cell_Info RPAREN
                  { ci_append_cell_info (priv_data, $1); }
;
Cell_Info       : Cell_Spec
                  { ci_no_disable (priv_data); }
                | Cell_Spec COMMA Disable_Spec
;
Cell_Spec       : IDENTIFIER COMMA Port_Name COMMA Cell_Function
                  COMMA Safe_Value
                  {
                    free ($1);
                    ci_set_cell_spec (priv_data, $5, $7);
                  }
;
Port_Name       : IDENTIFIER
                  {
                    prt_add_name (priv_data, $1);
                    prt_add_bit (priv_data);
                  }
                | IDENTIFIER LPAREN DECIMAL_NUMBER RPAREN
                  {
                    prt_add_name (priv_data, $1);
                    prt_add_range (priv_data, $3, $3);
                  }
                | ASTERISK
                  {
                    prt_add_name (priv_data, strdup ("*"));
                    prt_add_bit (priv_data);
                  }
;
Cell_Function   : INPUT
                  { $$ = INPUT; }
                | OUTPUT2
                  { $$ = OUTPUT2; }
                | OUTPUT3
                  { $$ = OUTPUT3; }
                | CONTROL
                  { $$ = CONTROL; }
                | CONTROLR
                  { $$ = CONTROLR; }
                | INTERNAL
                  { $$ = INTERNAL; }
                | CLOCK
                  { $$ = CLOCK; }
                | BIDIR
                  { $$ = BIDIR; }
                | OBSERVE_ONLY
                  { $$ = OBSERVE_ONLY; }
;
Safe_Value      : IDENTIFIER
                  { $$ = $1; }
                | DECIMAL_NUMBER
                  {
                    char *tmp;
                    /* @@@@ RFHH check malloc result */
                    tmp = malloc (2);
                    snprintf (tmp, 2, "%i", $1);
                    tmp[1] = '\0';
                    $$ = tmp;
                  }
;
Disable_Spec    : DECIMAL_NUMBER COMMA DECIMAL_NUMBER COMMA Disable_Value
                  { ci_set_cell_spec_disable (priv_data, $1, $3, $5); }
;
Disable_Value   : Z
                  { $$ = Z; }
                | WEAK0
                  { $$ = WEAK0; }
                | WEAK1
                  { $$ = WEAK1; }
                | PULL0
                  { $$ = PULL0; }
                | PULL1
                  { $$ = PULL1; }
                | KEEPER
                  { $$ = KEEPER; }
;

/****************************************************************************/
BSDL_Compliance_Patterns : COMPLIANCE_PATTERNS BSDL_Compliance_Pattern
;
BSDL_Compliance_Pattern : LPAREN Physical_Pin_List RPAREN
                          { urj_bsdl_flex_set_bin_x (priv_data->scanner); }
                          LPAREN Bin_X_Pattern_List RPAREN
;
Bin_X_Pattern_List : BIN_X_PATTERN
                     { free ($1); }
                   | Bin_X_Pattern_List COMMA BIN_X_PATTERN
                     { free ($3); }
;

/****************************************************************************/
BSDL_Component_Conformance : COMPONENT_CONFORMANCE STD_1149_1_1990
                             { priv_data->jtag_ctrl->conformance = URJ_BSDL_CONF_1990; }
                           | COMPONENT_CONFORMANCE STD_1149_1_1993
                             { priv_data->jtag_ctrl->conformance = URJ_BSDL_CONF_1993; }
                           | COMPONENT_CONFORMANCE STD_1149_1_2001
                             { priv_data->jtag_ctrl->conformance = URJ_BSDL_CONF_2001; }
;
/****************************************************************************/
ISC_Extension : ISC_Conformance
              | ISC_Pin_Behavior
              | ISC_Fixed_System_Pins
              | ISC_Status
              | ISC_Blank_Usercode
              | ISC_Security
              | ISC_Flow
              | ISC_Procedure
              | ISC_Action
              | ISC_Illegal_Exit
;
/****************************************************************************/
ISC_Conformance : ISC_CONFORMANCE STD_1532_2001
                | ISC_CONFORMANCE STD_1532_2002
;
/****************************************************************************/
ISC_Pin_Behavior    : ISC_PIN_BEHAVIOR Pin_Behavior_Option
;
Pin_Behavior_Option : HIGHZ
                    | CLAMP
                    | error
                      {
                        Print_Error (priv_data, _("Error in ISC_Pin_Behavior Definition"));
                        BUMP_ERROR;
                        YYABORT;
                      }
;
/****************************************************************************/
ISC_Fixed_System_Pins : ISC_FIXED_SYSTEM_PINS Fixed_Pin_List
;
Fixed_Pin_List        : Port_Id
                      | Fixed_Pin_List COMMA Port_Id
                      | error
                        {
                          Print_Error (priv_data, _("Error in ISC_Fixed_System_Pins Definition"));
                          BUMP_ERROR;
                          YYABORT;
                        }
;
Port_Id               : IDENTIFIER
                        { free ($1); }
                      | IDENTIFIER LPAREN DECIMAL_NUMBER RPAREN
                        { free ($1); }
;
/****************************************************************************/
ISC_Status      : ISC_STATUS Status_Modifier IMPLEMENTED
;
Status_Modifier : /* empty */
                | IDENTIFIER
                  { free ($1); }
;
/****************************************************************************/
ISC_Blank_Usercode : ISC_BLANK_USERCODE BIN_X_PATTERN
                     { free ($2); }
;
/****************************************************************************/
ISC_Security : ISC_SECURITY Protection_Spec
;
Protection_Spec : Read_Spec COMMA Program_Spec COMMA Erase_Spec COMMA Key_Spec
                | error
                  {
                    Print_Error (priv_data, _("Error in ISC_Security Definition"));
                    BUMP_ERROR;
                    YYABORT;
                  }
;
Read_Spec       : ISC_DISABLE_READ Bit_Spec
;
Program_Spec    : ISC_DISABLE_PROGRAM Bit_Spec
;
Erase_Spec      : ISC_DISABLE_ERASE Bit_Spec
;
Key_Spec        : ISC_DISABLE_KEY Bit_Range
;
Bit_Spec        : ASTERISK
                | DECIMAL_NUMBER
;
Bit_Range       : ASTERISK
                | DECIMAL_NUMBER MINUS DECIMAL_NUMBER
;
/****************************************************************************/
ISC_Flow               : ISC_FLOW Flow_Definition_List
;
Flow_Definition_List   : Flow_Definition
                       | Flow_Definition_List COMMA Flow_Definition
;
Flow_Definition        : Flow_Descriptor
                       | Flow_Descriptor Initialize_Block
                       | Flow_Descriptor Initialize_Block Repeat_Block
                       | Flow_Descriptor Initialize_Block Repeat_Block Terminate_Block
                       | Flow_Descriptor Repeat_Block
                       | Flow_Descriptor Repeat_Block Terminate_Block
                       | Flow_Descriptor Terminate_Block
                       | error
                         {
                           Print_Error (priv_data, _("Error in ISC_Flow Definition"));
                           BUMP_ERROR;
                           YYABORT;
                         }
;
Flow_Descriptor        : IDENTIFIER
                         { free ($1); }
                       | IDENTIFIER Data_Name
                         { free ($1); }
                       | IDENTIFIER Data_Name UNPROCESSED
                         { free ($1); }
                       | IDENTIFIER Data_Name UNPROCESSED EXIT_ON_ERROR
                         { free ($1); }
                       | IDENTIFIER UNPROCESSED
                         { free ($1); }
                       | IDENTIFIER UNPROCESSED EXIT_ON_ERROR
                         { free ($1); }
                       | IDENTIFIER EXIT_ON_ERROR
                         { free ($1); }
;
Data_Name              : LPAREN Standard_Data_Name RPAREN
                       | LPAREN IDENTIFIER RPAREN
                         { free ($2); }
;
Standard_Data_Name     : ARRAY | USERCODE | SECURITY | IDCODE | PRELOAD
;
Initialize_Block       : INITIALIZE Activity_List
;
Repeat_Block           : REPEAT DECIMAL_NUMBER Activity_List
;
Terminate_Block        : TERMINATE Activity_List
;
Activity_List          : Activity_Element
                       | Activity_List Activity_Element
;
Activity_Element       : Activity
                       | Loop_Block
;
Loop_Block             : LOOP Loop_Min_Spec Loop_Max_Spec LPAREN Loop_Activity_List RPAREN
;
Loop_Min_Spec          : /* empty */
                       | MIN DECIMAL_NUMBER
;
Loop_Max_Spec          : MAX DECIMAL_NUMBER
;
Loop_Activity_List     : Activity
                       | Loop_Activity_List Activity
;
Activity               : LPAREN Instruction_Name Wait_Specification RPAREN
                         { free ($2); }
                       | LPAREN Instruction_Name Update_Field_List Wait_Specification RPAREN
                         { free ($2); }
                       | LPAREN Instruction_Name Wait_Specification Capture_Field_List RPAREN
                         { free ($2); }
                       | LPAREN Instruction_Name Update_Field_List Wait_Specification Capture_Field_List RPAREN
                         { free ($2); }
;
Update_Field_List      : Update_Field
                       | Update_Field_List COMMA Update_Field
;
Update_Field           : DECIMAL_NUMBER
                       | DECIMAL_NUMBER COLON
                         { urj_bsdl_flex_set_hex (priv_data->scanner); }
                         Data_Expression
                         { urj_bsdl_flex_set_decimal (priv_data->scanner); }
;
Data_Expression        : HEX_STRING
                         { free ($1); }
                       | Input_Specifier
                       | Variable_Expression
;
Variable_Expression    : Variable
                       | Variable_Assignment
                       | Variable_Update
;
Variable_Assignment    : Variable EQUAL
                         { urj_bsdl_flex_set_hex (priv_data->scanner); }
                         HEX_STRING
                         {
                           free ($4);
                           urj_bsdl_flex_set_decimal (priv_data->scanner);
                         }
                       | Variable Input_Specifier
;
Variable_Update        : Variable Complement_Operator
                       | Variable Binary_Operator DECIMAL_NUMBER
;
Input_Specifier        : Input_Operator
                       | IO_Operator
;
Capture_Field_List     : Capture_Field
                       | Capture_Field_List COMMA Capture_Field
;
Capture_Field          : DECIMAL_NUMBER COLON
                         { urj_bsdl_flex_set_hex (priv_data->scanner); }
                         Capture_Field_Rest
                         { urj_bsdl_flex_set_decimal (priv_data->scanner); }
;
Capture_Field_Rest     : Capture_Specification
                       | Capture_Specification CRC_Tag
                       | Capture_Specification CRC_Tag OST_Tag
                       | Capture_Specification OST_Tag
;
Capture_Specification  : Expected_Data
                       | Expected_Data Compare_Mask
;
Expected_Data          : /* empty */
                       | Output_Operator
                       | Output_Operator Data_Expression
                       | Data_Expression
;
Compare_Mask           : ASTERISK
                       | ASTERISK Output_Operator
                       | ASTERISK Output_Operator Data_Expression
                       | ASTERISK Data_Expression
;
Wait_Specification     : WAIT Duration_Specification
                       | WAIT Duration_Specification MIN
                       | WAIT Duration_Specification MIN COLON Duration_Specification MAX
;
Duration_Specification : Clock_Cycles
                       | REAL_NUMBER
                         { free ($1); }
                       | Clock_Cycles COMMA REAL_NUMBER
                         { free ($3); }
;
Clock_Cycles           : Port_Id DECIMAL_NUMBER
;
Variable               : DOLLAR IDENTIFIER
                         { free ($2); }
;
Binary_Operator        : PLUS
                         { urj_bsdl_flex_set_decimal (priv_data->scanner); }
                       | MINUS
                         { urj_bsdl_flex_set_decimal (priv_data->scanner); }
                       | SH_RIGHT
                         { urj_bsdl_flex_set_decimal (priv_data->scanner); }
                       | SH_LEFT
                         { urj_bsdl_flex_set_decimal (priv_data->scanner); }
;
Complement_Operator    : TILDE
;
Input_Operator         : QUESTION_MARK
;
Output_Operator        : EXCLAMATION_MARK
;
IO_Operator            : QUESTION_EXCLAMATION
;
CRC_Tag                : COLON CRC
;
OST_Tag                : COLON OST
;
/****************************************************************************/
ISC_Procedure        : ISC_PROCEDURE Procedure_List
;
Procedure_List       : Procedure
                     | Procedure_List COMMA Procedure
;
Procedure            : IDENTIFIER EQUAL LPAREN Flow_Descriptor_List RPAREN
                       { free ($1); }
                     | IDENTIFIER Data_Name EQUAL LPAREN Flow_Descriptor_List RPAREN
                       { free ($1); }
                     | error
                       {
                         Print_Error (priv_data, _("Error in ISC_Procedure Definition"));
                         BUMP_ERROR;
                         YYABORT;
                       }
;
Flow_Descriptor_List : Flow_Descriptor
                     | Flow_Descriptor_List COMMA Flow_Descriptor
;
/****************************************************************************/
ISC_Action                : ISC_ACTION Action_List
;
Action_List               : Action
                          | Action_List COMMA Action
;
Action                    : IDENTIFIER EQUAL LPAREN Action_Specification_List RPAREN
                            { free ($1); }
                          | IDENTIFIER Data_Name EQUAL LPAREN Action_Specification_List RPAREN
                            { free ($1); }
                          | IDENTIFIER PROPRIETARY EQUAL LPAREN Action_Specification_List RPAREN
                            { free ($1); }
                          | IDENTIFIER Data_Name PROPRIETARY EQUAL LPAREN Action_Specification_List RPAREN
                            { free ($1); }
                          | error
                            {
                              Print_Error (priv_data, _("Error in ISC_Action Definition"));
                              BUMP_ERROR;
                              YYABORT;
                            }
;
Action_Specification_List : Action_Specification
                          | Action_Specification_List COMMA Action_Specification
;
Action_Specification      : IDENTIFIER
                            { free ($1); }
                          | IDENTIFIER Data_Name
                            { free ($1); }
                          | IDENTIFIER Data_Name PROPRIETARY
                            { free ($1); }
                          | IDENTIFIER Data_Name Option_Specification
                            { free ($1); }
                          | IDENTIFIER Data_Name PROPRIETARY Option_Specification
                            { free ($1); }
                          | IDENTIFIER PROPRIETARY
                            { free ($1); }
                          | IDENTIFIER PROPRIETARY Option_Specification
                            { free ($1); }
                          | IDENTIFIER Option_Specification
                            { free ($1); }
;
Option_Specification      : OPTIONAL | RECOMMENDED
;
/****************************************************************************/
ISC_Illegal_Exit : ISC_ILLEGAL_EXIT Exit_Instruction_List
;
Exit_Instruction_List : IDENTIFIER
                        { free ($1); }
                      | Exit_Instruction_List COMMA IDENTIFIER
                        { free ($3); }
;

%%  /* End rules, begin programs  */

/*----------------------------------------------------------------------*/
static void
Print_Error (urj_bsdl_parser_priv_t *priv_data, const char *Errmess)
{
    urj_bsdl_err (priv_data->jtag_ctrl->proc_mode,
                  _("Line %d, %s.\n"), priv_data->lineno, Errmess);

    /* set an error if nothing else is pending */
    if (urj_error_get () == URJ_ERROR_OK)
        urj_bsdl_err_set (priv_data->jtag_ctrl->proc_mode,
                          URJ_ERROR_BSDL_BSDL,
                          "Parser error, see log for details");
}

/*----------------------------------------------------------------------*/
static void
Print_Warning (urj_bsdl_parser_priv_t *priv_data, const char *Warnmess)
{
    urj_bsdl_warn (priv_data->jtag_ctrl->proc_mode,
                   _("Line %d, %s.\n"), priv_data->lineno, Warnmess);
}

/*----------------------------------------------------------------------*/
static void
Give_Up_And_Quit (urj_bsdl_parser_priv_t *priv_data)
{
    //Print_Error( priv_data, "Too many errors" );
    urj_bsdl_flex_stop_buffer (priv_data->scanner);
}

/*----------------------------------------------------------------------*/
void
yyerror (urj_bsdl_parser_priv_t *priv_data, const char *error_string)
{
}


/*****************************************************************************
 * void urj_bsdl_sem_init( urj_bsdl_parser_priv_t *priv )
 *
 * Initializes storage elements in the private parser and jtag control
 * structures that are used for semantic purposes.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
urj_bsdl_sem_init (urj_bsdl_parser_priv_t *priv)
{
    urj_bsdl_jtag_ctrl_t *jc = priv->jtag_ctrl;

    jc->instr_len = -1;
    jc->bsr_len = -1;
    jc->conformance = URJ_BSDL_CONF_UNKNOWN;
    jc->idcode = NULL;
    jc->usercode = NULL;

    jc->instr_list = NULL;

    priv->ainfo.next = NULL;
    priv->ainfo.reg = NULL;
    priv->ainfo.instr_list = NULL;
    jc->ainfo_list = NULL;

    priv->tmp_cell_info.next = NULL;
    priv->tmp_cell_info.port_name = NULL;
    priv->tmp_cell_info.basic_safe_value = NULL;
    jc->cell_info_first = NULL;
    jc->cell_info_last = NULL;

    priv->tmp_port_desc.names_list = NULL;
    priv->tmp_port_desc.next = NULL;
}


/*****************************************************************************
 * void free_instr_list( struct instr_elem *il )
 *
 * Deallocates the given list of instr_elem.
 *
 * Parameters
 *   il : first instr_elem to deallocate
 *
 * Returns
 *   void
 ****************************************************************************/
static void
free_instr_list (urj_bsdl_instr_elem_t *il)
{
    if (il)
    {
        if (il->instr)
            free (il->instr);
        if (il->opcode)
            free (il->opcode);
        free_instr_list (il->next);
        free (il);
    }
}


/*****************************************************************************
 * void free_ainfo_list( urj_bsdl_types_ainfo_elem_t *ai, int free_me )
 *
 * Deallocates the given list of ainfo_elem.
 *
 * Parameters
 *  ai      : first ainfo_elem to deallocate
 *  free_me : set to 1 to free memory for ai as well
 *
 * Returns
 *  void
 ****************************************************************************/
static void
free_ainfo_list (urj_bsdl_types_ainfo_elem_t *ai, int free_me)
{
    if (ai)
    {
        if (ai->reg)
            free (ai->reg);

        free_instr_list (ai->instr_list);
        free_ainfo_list (ai->next, 1);

        if (free_me)
            free (ai);
    }
}


/*****************************************************************************
 * void free_string_list( urj_bsdl_string_elem_t *sl )
 *
 * Deallocates the given list of string_elem items.
 *
 * Parameters
 *  sl : first string_elem to deallocate
 *
 * Returns
 *  void
 ****************************************************************************/
static void
free_string_list (urj_bsdl_string_elem_t *sl)
{
    if (sl)
    {
        if (sl->string)
            free (sl->string);
        free_string_list (sl->next);
        free (sl);
    }
}


/*****************************************************************************
 * void free_c_list( urj_bsdl_cell_info_t *ci, int free_me )
 *
 * Deallocates the given list of cell_info items.
 *
 * Parameters
 *  ci      : first cell_info item to deallocate
 *  free_me : 1 -> free memory for *ci as well
 *            0 -> don't free *ci memory
 *
 * Returns
 *  void
 ****************************************************************************/
static void
free_ci_list (urj_bsdl_cell_info_t *ci, int free_me)
{
    if (ci)
    {
        free_ci_list (ci->next, 1);

        if (ci->port_name)
            free (ci->port_name);

        if (ci->basic_safe_value)
            free (ci->basic_safe_value);

        if (free_me)
            free (ci);
    }
}


/*****************************************************************************
 * void urj_bsdl_sem_deinit( urj_bsdl_parser_priv_t *priv )
 *
 * Frees and deinitializes storage elements in the private parser and
 * jtag control structures that were filled by semantic rules.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
urj_bsdl_sem_deinit (urj_bsdl_parser_priv_t *priv)
{
    urj_bsdl_jtag_ctrl_t *jc = priv->jtag_ctrl;

    if (jc->idcode)
    {
        free (jc->idcode);
        jc->idcode = NULL;
    }

    if (jc->usercode)
    {
        free (jc->usercode);
        jc->usercode = NULL;
    }

    /* free cell_info list */
    free_ci_list (jc->cell_info_first, 1);
    jc->cell_info_first = jc->cell_info_last = NULL;
    free_ci_list (&(priv->tmp_cell_info), 0);

    /* free instr_list */
    free_instr_list (jc->instr_list);
    jc->instr_list = NULL;

    /* free ainfo_list */
    free_ainfo_list (jc->ainfo_list, 1);
    jc->ainfo_list = NULL;
    free_ainfo_list (&(priv->ainfo), 0);

    /* free string list in temporary port descritor */
    free_string_list (priv->tmp_port_desc.names_list);
    priv->tmp_port_desc.names_list = NULL;
}


/*****************************************************************************
 * urj_bsdl_parser_priv_t *urj_bsdl_parser_init( urj_bsdl_jtag_ctrl_t *jtag_ctrl )
 *
 * Initializes storage elements in the private parser structure that are
 * used for parser maintenance purposes.
 * Subsequently calls initializer functions for the scanner and the semantic
 * parts.
 *
 * Parameters
 *   jtag_ctrl : pointer to jtag control structure
 *
 * Returns
 *   pointer to private parser structure
 ****************************************************************************/
urj_bsdl_parser_priv_t *
urj_bsdl_parser_init (urj_bsdl_jtag_ctrl_t *jtag_ctrl)
{
    urj_bsdl_parser_priv_t *new_priv;

    if (!(new_priv = malloc (sizeof (urj_bsdl_parser_priv_t))))
    {
        urj_bsdl_ftl_set (jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
        return NULL;
    }

    new_priv->jtag_ctrl = jtag_ctrl;

    if (!(new_priv->scanner = urj_bsdl_flex_init (jtag_ctrl->proc_mode)))
    {
        free (new_priv);
        new_priv = NULL;
    }

    urj_bsdl_sem_init (new_priv);

    return new_priv;
}


/*****************************************************************************
 * void urj_bsdl_parser_deinit( urj_bsdl_parser_priv_t *priv )
 *
 * Frees storage elements in the private parser structure that are
 * used for parser maintenance purposes.
 * Subsequently calls deinitializer functions for the scanner and the semantic
 * parts.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
void
urj_bsdl_parser_deinit (urj_bsdl_parser_priv_t *priv_data)
{
    urj_bsdl_sem_deinit (priv_data);
    urj_bsdl_flex_deinit (priv_data->scanner);
    free (priv_data);
}


/*****************************************************************************
 * void add_instruction( urj_bsdl_parser_priv_t *priv, char *instr, char *opcode )
 *
 * Converts the instruction specification into a member of the main
 * list of instructions at priv->jtag_ctrl->instr_list.
 *
 * Parameters
 *   priv   : private data container for parser related tasks
 *   instr  : instruction name
 *   opcode : instruction opcode
 *
 * Returns
 *   void
 ****************************************************************************/
static void
add_instruction (urj_bsdl_parser_priv_t *priv, char *instr, char *opcode)
{
    urj_bsdl_instr_elem_t *new_instr;

    new_instr = malloc (sizeof (urj_bsdl_instr_elem_t));
    if (new_instr)
    {
        new_instr->next = priv->jtag_ctrl->instr_list;
        new_instr->instr = instr;
        new_instr->opcode = opcode;

        priv->jtag_ctrl->instr_list = new_instr;
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}


/*****************************************************************************
 * void ac_set_register( urj_bsdl_parser_priv_t *priv, char *reg, int reg_len )
 * Register Access management function
 *
 * Stores the register specification values for the current register access
 * specification in the temporary storage region for later usage.
 *
 * Parameters
 *   priv    : private data container for parser related tasks
 *   reg     : register name
 *   reg_len : optional register length
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ac_set_register (urj_bsdl_parser_priv_t *priv, char *reg, int reg_len)
{
    urj_bsdl_types_ainfo_elem_t *tmp_ai = &(priv->ainfo);

    tmp_ai->reg = reg;
    tmp_ai->reg_len = reg_len;
}


/*****************************************************************************
 * void ac_add_instruction( urj_bsdl_parser_priv_t *priv, char *instr )
 * Register Access management function
 *
 * Appends the specified instruction to the list of instructions for the
 * current register access specification in the temporary storage region
 * for later usage.
 *
 * Parameters
 *   priv  : private data container for parser related tasks
 *   instr : instruction name
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ac_add_instruction (urj_bsdl_parser_priv_t *priv, char *instr)
{
    urj_bsdl_types_ainfo_elem_t *tmp_ai = &(priv->ainfo);
    urj_bsdl_instr_elem_t *new_instr;

    new_instr = malloc (sizeof (urj_bsdl_instr_elem_t));
    if (new_instr)
    {
        new_instr->next = tmp_ai->instr_list;
        new_instr->instr = instr;
        new_instr->opcode = NULL;

        tmp_ai->instr_list = new_instr;
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}


/*****************************************************************************
 * void ac_apply_assoc( urj_bsdl_parser_priv_t *priv )
 * Register Access management function
 *
 * Appends the collected register access specification from the temporary
 * storage region to the main ainfo list.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ac_apply_assoc (urj_bsdl_parser_priv_t *priv)
{
    urj_bsdl_jtag_ctrl_t *jc = priv->jtag_ctrl;
    urj_bsdl_types_ainfo_elem_t *tmp_ai = &(priv->ainfo);
    urj_bsdl_types_ainfo_elem_t *new_ai;

    new_ai = malloc (sizeof (urj_bsdl_types_ainfo_elem_t));
    if (new_ai)
    {
        new_ai->next = jc->ainfo_list;
        new_ai->reg = tmp_ai->reg;
        new_ai->reg_len = tmp_ai->reg_len;
        new_ai->instr_list = tmp_ai->instr_list;

        jc->ainfo_list = new_ai;
    }
    else
        urj_bsdl_ftl_set (jc->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");

    /* clean up obsolete temporary entries */
    tmp_ai->reg = NULL;
    tmp_ai->reg_len = 0;
    tmp_ai->instr_list = NULL;
}


/*****************************************************************************
 * void prt_add_name( urj_bsdl_parser_priv_t *priv, char *name )
 * Port name management function
 *
 * Sets the name field of the temporary storage area for port description
 * (port_desc) to the parameter name.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   name : base name of the port, memory get's free'd lateron
 *
 * Returns
 *   void
 ****************************************************************************/
static void
prt_add_name (urj_bsdl_parser_priv_t *priv, char *name)
{
    urj_bsdl_port_desc_t *pd = &(priv->tmp_port_desc);
    urj_bsdl_string_elem_t *new_string;

    new_string = malloc (sizeof (urj_bsdl_string_elem_t));
    if (new_string)
    {
        new_string->next = pd->names_list;
        new_string->string = name;

        pd->names_list = new_string;
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}


/*****************************************************************************
 * void prt_add_bit( urj_bsdl_parser_priv_t *priv )
 * Port name management function
 *
 * Sets the vector and index fields of the temporary storage area for port
 * description (port_desc) to non-vector information. The low and high indice
 * are set to equal numbers (exact value is irrelevant).
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
prt_add_bit (urj_bsdl_parser_priv_t *priv)
{
    urj_bsdl_port_desc_t *pd = &(priv->tmp_port_desc);

    pd->is_vector = 0;
    pd->low_idx = 0;
    pd->high_idx = 0;
}


/*****************************************************************************
 * void prt_add_range( urj_bsdl_parser_priv_t *priv, int low, int high )
 * Port name management function
 *
 * Sets the vector and index fields of the temporary storage area for port
 * description (port_desc) to the specified vector information.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *   low  : low index of vector
 *   high : high index of vector
 *
 * Returns
 *   void
 ****************************************************************************/
static void
prt_add_range (urj_bsdl_parser_priv_t *priv, int low, int high)
{
    urj_bsdl_port_desc_t *pd = &(priv->tmp_port_desc);

    pd->is_vector = 1;
    pd->low_idx = low;
    pd->high_idx = high;
}


/*****************************************************************************
 * void ci_no_disable( urj_bsdl_parser_priv_t *priv )
 * Cell Info management function
 *
 * Tracks that there is no disable term for the current cell info.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ci_no_disable (urj_bsdl_parser_priv_t *priv)
{
    priv->tmp_cell_info.ctrl_bit_num = -1;
}


/*****************************************************************************
 * void ci_set_cell_spec_disable( urj_bsdl_parser_priv_t *priv, int ctrl_bit_num,
 *                                int safe_value, int disable_value )
 * Cell Info management function
 *
 * Applies the disable specification of the current cell spec to the variables
 * for temporary storage of these information elements.
 *
 * Parameters
 *   priv          : private data container for parser related tasks
 *   ctrl_bit_num  : bit number of related control cell
 *   safe_value    : safe value for initialization of this cell
 *   disable_value : currently ignored
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ci_set_cell_spec_disable (urj_bsdl_parser_priv_t *priv, int ctrl_bit_num,
                          int safe_value, int disable_value)
{
    urj_bsdl_cell_info_t *ci = &(priv->tmp_cell_info);

    ci->ctrl_bit_num = ctrl_bit_num;
    ci->disable_safe_value = safe_value;
    /* disable value is ignored at the moment */
}


/*****************************************************************************
 * void ci_set_cell_spec( urj_bsdl_parser_priv_t *priv,
 *                        int function, char *safe_value )
 * Cell Info management function
 *
 * Sets the specified values of the current cell_spec (without disable term)
 * to the variables for temporary storage of these information elements.
 * The name of the related port is taken from the port_desc structure that
 * was filled in previously by the rule Port_Name.
 *
 * Parameters
 *   priv       : private data container for parser related tasks
 *   function   : cell function indentificator
 *   safe_value : safe value for initialization of this cell
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ci_set_cell_spec (urj_bsdl_parser_priv_t *priv,
                  int function, char *safe_value)
{
    urj_bsdl_cell_info_t *ci = &(priv->tmp_cell_info);
    urj_bsdl_port_desc_t *pd = &(priv->tmp_port_desc);
    urj_bsdl_string_elem_t *name = priv->tmp_port_desc.names_list;
    char *port_string;
    size_t str_len, name_len;

    ci->cell_function = function;
    ci->basic_safe_value = safe_value;

    /* handle indexed port name:
       - names of scalar ports are simply copied from the port_desc structure
       to the final string that goes into ci
       - names of vectored ports are expanded with their decimal index as
       collected earlier earlier in rule Port_Name
     */
    name_len = strlen (name->string);
    str_len = name_len + 1 + 10 + 1 + 1;
    if ((port_string = malloc (str_len)) != NULL)
    {
        if (pd->is_vector)
            snprintf (port_string, str_len - 1, "%s(%d)", name->string,
                      pd->low_idx);
        else
            strncpy (port_string, name->string, str_len - 1);
        port_string[str_len - 1] = '\0';

        ci->port_name = port_string;
    }
    else
    {
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
        ci->port_name = NULL;
    }

    free_string_list (priv->tmp_port_desc.names_list);
    priv->tmp_port_desc.names_list = NULL;
}


/*****************************************************************************
 * void ci_append_cell_info( urj_bsdl_parser_priv_t *priv, int bit_num )
 * Cell Info management function
 *
 * Appends the temporary cell info to the global list of cell infos.
 *
 * Parameters
 *   priv    : private data container for parser related tasks
 *   bit_num : bit number of current cell
 *
 * Returns
 *   void
 ****************************************************************************/
static void
ci_append_cell_info (urj_bsdl_parser_priv_t *priv, int bit_num)
{
    urj_bsdl_cell_info_t *tmp_ci = &(priv->tmp_cell_info);
    urj_bsdl_cell_info_t *ci;
    urj_bsdl_jtag_ctrl_t *jc = priv->jtag_ctrl;

    ci = malloc (sizeof (urj_bsdl_cell_info_t));
    if (ci)
    {
        ci->next = NULL;
        if (jc->cell_info_last)
            jc->cell_info_last->next = ci;
        else
            jc->cell_info_first = ci;
        jc->cell_info_last = ci;

        ci->bit_num = bit_num;
        ci->port_name = tmp_ci->port_name;
        ci->cell_function = tmp_ci->cell_function;
        ci->basic_safe_value = tmp_ci->basic_safe_value;
        ci->ctrl_bit_num = tmp_ci->ctrl_bit_num;
        ci->disable_safe_value = tmp_ci->disable_safe_value;

        tmp_ci->port_name = NULL;
        tmp_ci->basic_safe_value = NULL;
    }
    else
        urj_bsdl_ftl_set (jc->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}


/*
 Local Variables:
 mode:C
 c-default-style:java
 indent-tabs-mode:nil
 End:
*/
