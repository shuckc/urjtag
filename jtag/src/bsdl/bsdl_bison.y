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
%parse-param {parser_priv_t *priv_data}
%defines
%name-prefix="bsdl"

%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "bsdl_sysdep.h"

/* interface to flex */
#include "bsdl_bison.h"
#include "bsdl.h"

#define YYLEX_PARAM priv_data->scanner
int yylex (YYSTYPE *, void *);

#if 1
#define ERROR_LIMIT 15
#define BUMP_ERROR if (bsdl_flex_postinc_compile_errors(priv_data->scanner)>ERROR_LIMIT) \
                          {Give_Up_And_Quit(priv_data);YYABORT;}
#else
#define BUMP_ERROR {Give_Up_And_Quit(priv_data);YYABORT;}
#endif

static void Init_Text(parser_priv_t *);
static void Store_Text(parser_priv_t *, char *);
static void Print_Error(parser_priv_t *, const char *);
static void Print_Warning(parser_priv_t *, const char *);
static void Give_Up_And_Quit(parser_priv_t *);

void yyerror(parser_priv_t *, const char *);
%}

%union {
    int   integer;
    char *str;
}


%token ENTITY  PORT  GENERIC  USE  ATTRIBUTE  IS
%token OF  CONSTANT  STRING  END  ALL  PIN_MAP
%token PHYSICAL_PIN_MAP  PIN_MAP_STRING  TRUE  FALSE  SIGNAL
%token TAP_SCAN_IN  TAP_SCAN_OUT  TAP_SCAN_MODE  TAP_SCAN_RESET
%token TAP_SCAN_CLOCK  LOW  BOTH  IN  OUT  INOUT
%token BUFFER  LINKAGE  BIT  BIT_VECTOR  TO  DOWNTO
%token PACKAGE  BODY  TYPE  SUBTYPE  RECORD  ARRAY
%token POSITIVE  RANGE  CELL_INFO  INSTRUCTION_LENGTH
%token INSTRUCTION_OPCODE  INSTRUCTION_CAPTURE  INSTRUCTION_DISABLE
%token INSTRUCTION_GUARD INSTRUCTION_PRIVATE  INSTRUCTION_USAGE
%token INSTRUCTION_SEQUENCE  REGISTER_ACCESS  BOUNDARY_CELLS
%token BOUNDARY_LENGTH  BOUNDARY_REGISTER  IDCODE_REGISTER
%token USERCODE_REGISTER  DESIGN_WARNING  BOUNDARY  BYPASS  HIGHZ  IDCODE  DEVICE_ID
%token USERCODE  INPUT  OUTPUT2  OUTPUT3  CONTROL  CONTROLR  INTERNAL
%token CLOCK  BIDIR  BIDIR_IN  BIDIR_OUT  EXTEST  SAMPLE
%token INTEST  RUNBIST  PI  PO  UPD  CAP  X
%token ZERO  ONE  Z  WEAK0  WEAK1 IDENTIFIER
%token PULL0 PULL1 KEEPER
%token SINGLE_QUOTE  QUOTED_STRING  DECIMAL_NUMBER  BINARY_PATTERN
%token BIN_X_PATTERN  REAL_NUMBER  CONCATENATE  SEMICOLON  COMMA
%token LPAREN  RPAREN  LBRACKET  RBRACKET  COLON  ASTERISK
%token BOX  COLON_EQUAL  PERIOD ILLEGAL
%token COMPONENT_CONFORMANCE PORT_GROUPING RUNBIST_EXECUTION
%token INTEST_EXECUTION BSDL_EXTENSION COMPLIANCE_PATTERNS
%token OBSERVE_ONLY

%type <str> BIN_X_PATTERN
%type <str> IDENTIFIER
%type <str> QUOTED_STRING
%type <str> BINARY_PATTERN
%type <str> Binary_Pattern
%type <str> Binary_Pattern_List
%type <integer> DECIMAL_NUMBER
%type <str> REAL_NUMBER
%type <integer> Cell_Function
%type <str> Safe_Value
%type <integer> Disable_Value
%type <str> Standard_Reg
%type <str> Standard_Inst

%start BSDL_Program

%%  /* End declarations, begin rules */

BSDL_Program     : Begin_BSDL Part_1 Part_2 End_BSDL
                 ;
Begin_BSDL       : ENTITY IDENTIFIER IS
                   { bsdl_set_entity(priv_data, $2); }
                 | error
                   {Print_Error(priv_data, _("Improper Entity declaration"));
                    Print_Error(priv_data, _("Check if source file is BSDL"));
                    BUMP_ERROR; YYABORT; /* Probably not a BSDL source file */
                   }
                 ;
Part_1           : VHDL_Generic        /* 1994 and later */
                   VHDL_Port
                   VHDL_Use_Part
                   VHDL_Component_Conformance
                   VHDL_Pin_Map
                   VHDL_Constant_List
                 | VHDL_Generic        /* 1990 */
                   VHDL_Port
                   VHDL_Use_Part
                   VHDL_Pin_Map
                   VHDL_Constant_List
                 | error
                   {Print_Error(priv_data, _("Syntax Error"));
                    BUMP_ERROR; YYABORT; }
                 ;
Part_2           : VHDL_Tap_Signals
                   VHDL_Compliance_Patterns
                   VHDL_Inst_Length
                   VHDL_Inst_Opcode
                   VHDL_Inst_Details
                   VHDL_Boundary_Details
                   VHDL_Boundary_Register
                 | VHDL_Tap_Signals
                   VHDL_Inst_Length
                   VHDL_Inst_Opcode
                   VHDL_Inst_Details
                   VHDL_Boundary_Details
                   VHDL_Boundary_Register
                 | error
                   {Print_Error(priv_data, _("Syntax Error"));
                    BUMP_ERROR; YYABORT; }
                 ;
End_BSDL         : VHDL_Design_Warning END IDENTIFIER SEMICOLON
                   { free($3); }
                 | error
                   {Print_Error(priv_data, _("Syntax Error"));
                    BUMP_ERROR; YYABORT; }
                 ;
VHDL_Generic     : GENERIC LPAREN PHYSICAL_PIN_MAP COLON STRING COLON_EQUAL
                   Quoted_String  RPAREN SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   IDENTIFIER
                   { free($11); }
                 ;
VHDL_Port        : PORT LPAREN Port_Specifier_List RPAREN SEMICOLON
                 | error
                   {Print_Error(priv_data, _("Improper Port declaration"));
                    BUMP_ERROR; YYABORT; }
                 ;
Port_Specifier_List : Port_Specifier
                 | Port_Specifier_List SEMICOLON Port_Specifier
                 ;
Port_Specifier   : Port_List COLON Function Scaler_Or_Vector
                   {
                     bsdl_prt_apply_port(priv_data);
                   }
                 ;
Port_List        : IDENTIFIER
                   { bsdl_prt_add_name(priv_data, $1); }
                 | Port_List COMMA IDENTIFIER
                   {
                     bsdl_prt_add_name(priv_data, $3);
                   }
                 ;
Function         : IN | OUT | INOUT | BUFFER | LINKAGE
                 ;
Scaler_Or_Vector : BIT
                   { bsdl_prt_add_bit(priv_data); }
                 | BIT_VECTOR LPAREN Vector_Range RPAREN
                 ;
Vector_Range     : DECIMAL_NUMBER TO DECIMAL_NUMBER
                   { bsdl_prt_add_range(priv_data, $1, $3); }
                 | DECIMAL_NUMBER DOWNTO DECIMAL_NUMBER
                   { bsdl_prt_add_range(priv_data, $3, $1); }
                 ;
VHDL_Use_Part    : Standard_Use
                 | Standard_Use VHDL_Use_List
                 | error
                   {Print_Error(priv_data, _("Error in Package declaration(s)"));
                    BUMP_ERROR; YYABORT; }
                 ;
Standard_Use     : USE IDENTIFIER
                   {/* Parse Standard 1149.1 Package */
                    strcpy(priv_data->Package_File_Name, $2);
                    free($2);
                   }
                   PERIOD ALL SEMICOLON
                   {
                     priv_data->Reading_Package = 1;
                     bsdl_flex_switch_file(priv_data->scanner,
                                           priv_data->Package_File_Name);
                   }
                   Standard_Package
                   {
                     priv_data->Reading_Package = 0;
                   }
                 ;
Standard_Package : PACKAGE IDENTIFIER IS Standard_Decls Defered_Constants
                   Standard_Decls END IDENTIFIER SEMICOLON Package_Body
                   { free($2); free($8); }
                 | error
                   {Print_Error(priv_data, _("Error in Standard Package"));
                    BUMP_ERROR; YYABORT; }
                 ;
Standard_Decls   : Standard_Decl
                 | Standard_Decls Standard_Decl
                 ;
Standard_Decl    : ATTRIBUTE Standard_Attributes COLON Attribute_Type SEMICOLON
                 | TYPE IDENTIFIER IS Type_Body SEMICOLON
                   { free($2); }
                 | TYPE CELL_INFO IS ARRAY LPAREN POSITIVE RANGE BOX RPAREN
                   OF IDENTIFIER SEMICOLON
                   { free($11); }
                 | SUBTYPE PIN_MAP_STRING IS STRING SEMICOLON
                 | SUBTYPE BSDL_EXTENSION IS STRING SEMICOLON
                 | error
                   {Print_Error(priv_data, _("Error in Standard Declarations"));
                    BUMP_ERROR; YYABORT; }
                 ;
Standard_Attributes : PIN_MAP | TAP_SCAN_IN | TAP_SCAN_OUT
                 | TAP_SCAN_CLOCK | TAP_SCAN_MODE | TAP_SCAN_RESET
                 | COMPONENT_CONFORMANCE | PORT_GROUPING | RUNBIST_EXECUTION
                 | INTEST_EXECUTION | COMPLIANCE_PATTERNS
                 | INSTRUCTION_LENGTH | INSTRUCTION_OPCODE
                 | INSTRUCTION_CAPTURE | INSTRUCTION_DISABLE
                 | INSTRUCTION_GUARD | INSTRUCTION_PRIVATE
                 | INSTRUCTION_USAGE | INSTRUCTION_SEQUENCE
                 | IDCODE_REGISTER | USERCODE_REGISTER
                 | REGISTER_ACCESS | BOUNDARY_CELLS
                 | BOUNDARY_LENGTH | BOUNDARY_REGISTER
                 | DESIGN_WARNING
                 | error
                   {Print_Error(priv_data, _("Error in Attribute identifier"));
                    BUMP_ERROR; YYABORT; }
                 ;
Attribute_Type   : IDENTIFIER
                   { free($1); }
                 | STRING
                 | DECIMAL_NUMBER
                 | error
                   {Print_Error(priv_data, _("Error in Attribute type identification"));
                    BUMP_ERROR; YYABORT; }
                 ;
Type_Body        : LPAREN ID_Bits RPAREN
                 | LPAREN ID_List RPAREN
                 | LPAREN LOW COMMA BOTH RPAREN
                 | ARRAY LPAREN DECIMAL_NUMBER TO DECIMAL_NUMBER RPAREN
                   OF IDENTIFIER
                   { free($8); }
                 | ARRAY LPAREN DECIMAL_NUMBER DOWNTO DECIMAL_NUMBER RPAREN
                   OF IDENTIFIER
                   { free($8); }
                 | RECORD Record_Body END RECORD
                 | error
                   {Print_Error(priv_data, _("Error in Type definition"));
                    BUMP_ERROR; YYABORT; }
                 ;
ID_Bits          : ID_Bit
                 | ID_Bits COMMA ID_Bit
                 ;
ID_List          : IDENTIFIER
                   { free($1); }
                 | ID_List COMMA IDENTIFIER
                   { free($3); }
                 ;
ID_Bit           : SINGLE_QUOTE BIN_X_PATTERN SINGLE_QUOTE
                   { free($2); }
                 | error
                   {Print_Error(priv_data, _("Error in Bit definition"));
                    BUMP_ERROR; YYABORT; }
                 ;
Record_Body      : Record_Element
                 | Record_Body Record_Element
                 ;
Record_Element   : IDENTIFIER COLON IDENTIFIER SEMICOLON
                   { free($1); free($3); }
                 | error
                   {Print_Error(priv_data, _("Error in Record Definition"));
                    BUMP_ERROR; YYABORT; }
                 ;
Defered_Constants: Defered_Constant
                 | Defered_Constants Defered_Constant
                 ;
Defered_Constant : CONSTANT Constant_Body
                 ;
Constant_Body    : IDENTIFIER COLON CELL_INFO SEMICOLON
                   { free($1); }
                 | error
                   {Print_Error(priv_data, _("Error in defered constant"));
                    BUMP_ERROR; YYABORT; }
                 ;
VHDL_Use_List    : VHDL_Use
                 | VHDL_Use_List VHDL_Use
                 ;
Package_Body     : PACKAGE BODY IDENTIFIER IS Constant_List END IDENTIFIER
                   { free($3); free($7); }
                   SEMICOLON
                 | error
                   {Print_Error(priv_data, _("Error in Package Body definition"));
                    BUMP_ERROR; YYABORT; }
                 ;
Constant_List    : Cell_Constant
                 | Constant_List Cell_Constant
                 ;
Cell_Constant    : CONSTANT IDENTIFIER COLON CELL_INFO COLON_EQUAL
                   LPAREN Triples_List RPAREN SEMICOLON
                   { free($2); }
                 | error
                   {Print_Error(priv_data, _("Error in Cell Constant definition"));
                    BUMP_ERROR; YYABORT; }
                 ;
Triples_List     : Triple
                 | Triples_List COMMA Triple
                 ;
Triple           : LPAREN Triple_Function COMMA Triple_Inst COMMA CAP_Data
                   RPAREN
                 | error
                   {Print_Error(priv_data, _("Error in Cell Data Record"));
                    BUMP_ERROR; YYABORT; }
                 ;
Triple_Function  : INPUT | OUTPUT2 | OUTPUT3 | INTERNAL | CONTROL
                 | CONTROLR | CLOCK | BIDIR_IN | BIDIR_OUT
                 | OBSERVE_ONLY
                 | error
                   {Print_Error(priv_data, _("Error in Cell_Type Function field"));
                    BUMP_ERROR; YYABORT; }
                 ;
Triple_Inst      : EXTEST | SAMPLE | INTEST | RUNBIST
                 | error
                   {Print_Error(priv_data, _("Error in BScan_Inst Instruction field"));
                    BUMP_ERROR; YYABORT; }
                 ;
CAP_Data         : PI | PO | UPD | CAP | X | ZERO | ONE
                 | error
                   {Print_Error(priv_data, _("Error in Constant CAP data source field"));
                    BUMP_ERROR; YYABORT; }
                 ;
VHDL_Use         : USE IDENTIFIER
                   {/* Parse Standard 1149.1 Package */
                    strcpy(priv_data->Package_File_Name, $2);
                    free($2);
                   }
                   PERIOD ALL SEMICOLON
                   {
                     priv_data->Reading_Package = 1;
                     bsdl_flex_switch_file(priv_data->scanner,
                                           priv_data->Package_File_Name);
                   }
                   User_Package
                   {
                     priv_data->Reading_Package = 0;
                   }
                 ;
User_Package     : PACKAGE IDENTIFIER
                   IS Defered_Constants END IDENTIFIER SEMICOLON Package_Body
                   { free($2); free($6); }
                 | error
                   {Print_Error(priv_data, _("Error in User-Defined Package declarations"));
                    BUMP_ERROR; YYABORT; }
                 ;
VHDL_Pin_Map     : ATTRIBUTE PIN_MAP OF IDENTIFIER
                   COLON ENTITY IS PHYSICAL_PIN_MAP SEMICOLON
                   { free($4); }
                 | error
                   {Print_Error(priv_data, _("Error in Pin_Map Attribute"));
                    BUMP_ERROR; YYABORT; }
                 ;
VHDL_Constant_List : VHDL_Constant
                 | VHDL_Constant_List VHDL_Constant
                 ;
VHDL_Constant    : CONSTANT VHDL_Constant_Part
                 ;
VHDL_Constant_Part : IDENTIFIER COLON PIN_MAP_STRING COLON_EQUAL
                   Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BSDL_Map_String
                   { free($1); }
                 | error
                   {Print_Error(priv_data, _("Error in Pin_Map_String constant declaration"));
                    BUMP_ERROR; YYABORT; }
                 ;
BSDL_Map_String  : Pin_Mapping
                 | BSDL_Map_String COMMA Pin_Mapping
                 ;
Pin_Mapping      : IDENTIFIER COLON Physical_Pin_Desc
                   { free($1); }
                 ;
Physical_Pin_Desc: Physical_Pin
                 | LPAREN Physical_Pin_List RPAREN
                 ;
Physical_Pin_List: Physical_Pin
                 | Physical_Pin_List COMMA Physical_Pin
                 ;
Physical_Pin     : IDENTIFIER
                   { free($1); }
                 | IDENTIFIER LPAREN DECIMAL_NUMBER RPAREN
                   { free($1); }
                 | DECIMAL_NUMBER
                 ;
VHDL_Tap_Signals : VHDL_Tap_Signal
                 | VHDL_Tap_Signals VHDL_Tap_Signal
                 ;
VHDL_Tap_Signal  : VHDL_Tap_Scan_In
                 | VHDL_Tap_Scan_Out
                 | VHDL_Tap_Scan_Clock
                 | VHDL_Tap_Scan_Mode
                 | VHDL_Tap_Scan_Reset
                 ;
VHDL_Tap_Scan_In : ATTRIBUTE TAP_SCAN_IN OF IDENTIFIER
                   COLON SIGNAL IS Boolean SEMICOLON
                   { free($4); }
                 ;
VHDL_Tap_Scan_Out : ATTRIBUTE TAP_SCAN_OUT OF IDENTIFIER
                    COLON SIGNAL IS Boolean SEMICOLON
                   { free($4); }
                 ;
VHDL_Tap_Scan_Mode : ATTRIBUTE TAP_SCAN_MODE OF IDENTIFIER
                   COLON SIGNAL IS Boolean SEMICOLON
                   { free($4); }
                 ;
VHDL_Tap_Scan_Reset : ATTRIBUTE TAP_SCAN_RESET OF IDENTIFIER
                   COLON SIGNAL IS Boolean SEMICOLON
                   { free($4); }
                 ;
VHDL_Tap_Scan_Clock : ATTRIBUTE TAP_SCAN_CLOCK OF IDENTIFIER COLON SIGNAL
                   IS LPAREN REAL_NUMBER COMMA Stop RPAREN SEMICOLON
                   { free($4); free($9); }
                 ;
Stop             : LOW | BOTH
                 ;
Boolean          : TRUE | FALSE
                 ;
VHDL_Inst_Length : ATTRIBUTE INSTRUCTION_LENGTH OF IDENTIFIER
                   COLON ENTITY IS DECIMAL_NUMBER SEMICOLON
                   {
                     bsdl_set_instruction_length(priv_data, $8);
                     free($4);
                   }
                 ;
VHDL_Inst_Opcode : ATTRIBUTE INSTRUCTION_OPCODE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BSDL_Opcode_Table
                   { free($4); }
                 ;
BSDL_Opcode_Table: Opcode_Desc
                 | BSDL_Opcode_Table COMMA Opcode_Desc
                 | error
                   {Print_Error(priv_data,
                      _("Error in Instruction_Opcode attribute statement"));
                    BUMP_ERROR;
                    YYABORT; }
                 ;
Opcode_Desc      : IDENTIFIER LPAREN Binary_Pattern_List RPAREN
                   { bsdl_add_instruction(priv_data, $1, $3); }
                 ;
Binary_Pattern_List : Binary_Pattern
                   { $$ = $1; }
                 | Binary_Pattern_List COMMA Binary_Pattern
                   {
                     Print_Warning(priv_data,
                       _("Multiple opcode patterns are not supported, first pattern will be used"));
                     $$ = $1;
                     free($3);
                   }
                 ;
Binary_Pattern   : BINARY_PATTERN
                   { $$ = $1; }
                 ;
VHDL_Inst_Details: VHDL_Inst_Detail
                 | VHDL_Inst_Details VHDL_Inst_Detail
                 ;
VHDL_Inst_Detail : VHDL_Inst_Capture
                 | VHDL_Inst_Disable
                 | VHDL_Inst_Guard
                 | VHDL_Inst_Private
                 | VHDL_Register_Access
                 | VHDL_Inst_Usage
                 | VHDL_Inst_Sequence
                 | VHDL_Idcode_Register
                 | VHDL_Usercode_Register
                 ;
VHDL_Inst_Capture: ATTRIBUTE INSTRUCTION_CAPTURE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BIN_X_PATTERN
                   { free($4); free($11); }
                 ;
VHDL_Inst_Disable: ATTRIBUTE INSTRUCTION_DISABLE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   IDENTIFIER
                   { free($4); free($11); }
                 ;
VHDL_Inst_Guard  : ATTRIBUTE INSTRUCTION_GUARD OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   IDENTIFIER
                   { free($4); free($11); }
                 ;
VHDL_Inst_Private: ATTRIBUTE INSTRUCTION_PRIVATE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   Private_Opcode_List
                   { free($4); }
                 ;
Private_Opcode_List : Private_Opcode
                 | Private_Opcode_List COMMA Private_Opcode
                 | error
                   {Print_Error(priv_data, _("Error in Opcode List"));
                    BUMP_ERROR;
                    YYABORT; }
                 ;
Private_Opcode   : IDENTIFIER
                   { free($1); }
                 ;
VHDL_Inst_Usage  : ATTRIBUTE INSTRUCTION_USAGE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {/* Syntax of string content to be changed in future */
                    free($4); }
                 ;
VHDL_Inst_Sequence : ATTRIBUTE INSTRUCTION_SEQUENCE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {/* Syntax of string content to be determined in future */
                    free($4); }
                 ;
VHDL_Idcode_Register: ATTRIBUTE IDCODE_REGISTER OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BIN_X_PATTERN
                   {
                     bsdl_set_idcode(priv_data, $11);
                     free($4);
                   }
                 ;
VHDL_Usercode_Register: ATTRIBUTE USERCODE_REGISTER OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BIN_X_PATTERN
                   {
		     bsdl_set_usercode(priv_data, $11);
		     free($4);
		   }
                 ;
VHDL_Register_Access: ATTRIBUTE REGISTER_ACCESS OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   Register_String
                   { free($4); }
                 ;
Register_String  : Register_Assoc
                 | Register_String COMMA Register_Assoc
                 ;
Register_Assoc   : Register_Decl LPAREN Reg_Opcode_List RPAREN
                   { bsdl_ac_apply_assoc(priv_data); }
                 ;
Register_Decl    : Standard_Reg
                   { bsdl_ac_set_register(priv_data, $1, 0); }
                 | IDENTIFIER LBRACKET DECIMAL_NUMBER RBRACKET
                   { bsdl_ac_set_register(priv_data, $1, $3); }
                 ;
Standard_Reg     : BOUNDARY
                   { $$ = strdup("BOUNDARY"); }
                 | BYPASS
                   { $$ = strdup("BYPASS"); }
                 | IDCODE
                   { $$ = strdup("IDCODE"); }
                 | USERCODE
                   { $$ = strdup("USERCODE"); }
                 | DEVICE_ID
                   { $$ = strdup("DEVICE_ID"); }
                 ;
Reg_Opcode_List  : Reg_Opcode
                 | Reg_Opcode_List COMMA Reg_Opcode
                 ;
Standard_Inst    : BOUNDARY
                   { $$ = strdup("BOUNDARY"); }
                 | BYPASS
                   { $$ = strdup("BYPASS"); }
                 | HIGHZ
                   { $$ = strdup("HIGHZ"); }
                 | IDCODE
                   { $$ = strdup("IDCODE"); }
                 | USERCODE
                   { $$ = strdup("USERCODE"); }
                 ;
Reg_Opcode       : IDENTIFIER
                   { bsdl_ac_add_instruction(priv_data, $1); }
                 | Standard_Inst
                   { bsdl_ac_add_instruction(priv_data, $1); }
                 ;
VHDL_Boundary_Details: VHDL_Boundary_Detail
                 | VHDL_Boundary_Details VHDL_Boundary_Detail
                 ;
VHDL_Boundary_Detail: VHDL_Boundary_Cells
                 | VHDL_Boundary_Length
                 ;
VHDL_Boundary_Cells: ATTRIBUTE BOUNDARY_CELLS OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BSDL_Cell_List
                   { free($4); }
                 ;
BSDL_Cell_List   : BCell_Identifier
                 | BSDL_Cell_List COMMA BCell_Identifier
                 ;
BCell_Identifier : IDENTIFIER
                   { free($1); }
                 ;
VHDL_Boundary_Length: ATTRIBUTE BOUNDARY_LENGTH OF IDENTIFIER
                   COLON ENTITY IS DECIMAL_NUMBER SEMICOLON
                   {
                     bsdl_set_bsr_length(priv_data, $8);
                     free($4);
                   }
                 ;
VHDL_Boundary_Register: ATTRIBUTE BOUNDARY_REGISTER OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BSDL_Cell_Table
                   { free($4); }
                 ;
BSDL_Cell_Table  : Cell_Entry
                 | BSDL_Cell_Table COMMA Cell_Entry
                 | error
                   {Print_Error(priv_data, _("Error in Boundary Cell description"));
                    BUMP_ERROR; YYABORT; }
                 ;
Cell_Entry       : DECIMAL_NUMBER LPAREN Cell_Info RPAREN
                   { bsdl_ci_apply_cell_info(priv_data, $1); }
                 ;
Cell_Info        : Cell_Spec
                   { bsdl_ci_no_disable(priv_data); }
                 | Cell_Spec COMMA Disable_Spec
                 ;
Cell_Spec        : IDENTIFIER COMMA Port_Name COMMA Cell_Function
                   COMMA Safe_Value
                   {
                     free($1);
                     bsdl_ci_set_cell_spec(priv_data, $5, $7);
                   }
                 ;
Port_Name        : IDENTIFIER
                   {
                     bsdl_prt_add_name(priv_data, $1);
                     bsdl_prt_add_bit(priv_data);
                   }
                 | IDENTIFIER LPAREN DECIMAL_NUMBER RPAREN
                   {
                     bsdl_prt_add_name(priv_data, $1);
                     bsdl_prt_add_range(priv_data, $3, $3);
                   }
                 | ASTERISK
                   {
                     bsdl_prt_add_name(priv_data, strdup("*"));
                     bsdl_prt_add_bit(priv_data);
                   }
                 ;
Cell_Function    : INPUT
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
Safe_Value       : IDENTIFIER
                   { $$ = $1; }
                 | DECIMAL_NUMBER
                   {
                     char *tmp;
                     tmp = (char *)malloc(2);
                     snprintf(tmp, 2, "%i", $1);
		     tmp[1] = '\0';
                     $$ = tmp;
                   }
                 ;
Disable_Spec     : DECIMAL_NUMBER COMMA DECIMAL_NUMBER COMMA Disable_Value
                   { bsdl_ci_set_cell_spec_disable(priv_data, $1, $3, $5); }
                 ;
Disable_Value    : Z
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
VHDL_Design_Warning: /* Null Statement */
                 | ATTRIBUTE DESIGN_WARNING OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   { free($4); }
                 ;
VHDL_Component_Conformance: ATTRIBUTE COMPONENT_CONFORMANCE OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   { free($4); }
                 ;
VHDL_Compliance_Patterns: ATTRIBUTE COMPLIANCE_PATTERNS OF IDENTIFIER
                   COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     bsdl_flex_switch_buffer(priv_data->scanner,
                                             priv_data->buffer_for_switch);
                   }
                   BSDL_Compliance_Pattern
                   { free($4); }
                 ;
BSDL_Compliance_Pattern : LPAREN Physical_Pin_List RPAREN
                   {bsdl_flex_set_bin_x(priv_data->scanner);}
                   LPAREN Bin_X_Pattern_List RPAREN
                 ;
Bin_X_Pattern_List : BIN_X_PATTERN
                     { free($1); }
                   | Bin_X_Pattern_List COMMA BIN_X_PATTERN
                     { free($3); }
                   ;
Quoted_String    : QUOTED_STRING
                   {Init_Text(priv_data);
                    Store_Text(priv_data, $1);
                    free($1); }
                 | Quoted_String CONCATENATE QUOTED_STRING
                   {Store_Text(priv_data, $3);
                    free($3); }
                 ;
%%  /* End rules, begin programs  */
/*----------------------------------------------------------------------*/
static void Init_Text(parser_priv_t *priv_data)
{
  if (priv_data->len_buffer_for_switch == 0) {
    priv_data->buffer_for_switch = (char *)malloc(160);
    priv_data->len_buffer_for_switch = 160;
  }
  priv_data->buffer_for_switch[0] = '\0';
}
/*----------------------------------------------------------------------*/
static void Store_Text(parser_priv_t *priv_data, char *Source)
{ /* Save characters from VHDL string in local string buffer.           */
  size_t req_len;
  char   *SourceEnd;

  SourceEnd = ++Source;   /* skip leading '"' */
  while (*SourceEnd && (*SourceEnd != '"') && (*SourceEnd != '\n'))
    SourceEnd++;
  /* terminate Source string with NUL character */
  *SourceEnd = '\0';

  req_len = strlen(priv_data->buffer_for_switch) + strlen(Source) + 1;
  if (req_len > priv_data->len_buffer_for_switch) {
    priv_data->buffer_for_switch = (char *)realloc(priv_data->buffer_for_switch,
                                                   req_len);
    priv_data->len_buffer_for_switch = req_len;
  }
  strcat(priv_data->buffer_for_switch, Source);
}
/*----------------------------------------------------------------------*/
static void Print_Error(parser_priv_t *priv_data, const char *Errmess)
{
  if (priv_data->Reading_Package)
    bsdl_msg(BSDL_MSG_ERR, _("In Package %s, Line %d, %s.\n"),
             priv_data->Package_File_Name,
             bsdl_flex_get_lineno(priv_data->scanner),
             Errmess);
  else
    if (priv_data->jtag_ctrl.debug || (priv_data->jtag_ctrl.mode >= 0))
      bsdl_msg(BSDL_MSG_ERR, _("Line %d, %s.\n"),
               bsdl_flex_get_lineno(priv_data->scanner),
               Errmess);
}
/*----------------------------------------------------------------------*/
static void Print_Warning(parser_priv_t *priv_data, const char *Warnmess)
{
  if (priv_data->Reading_Package)
    bsdl_msg(BSDL_MSG_WARN, _("In Package %s, Line %d, %s.\n"),
             priv_data->Package_File_Name,
             bsdl_flex_get_lineno(priv_data->scanner),
             Warnmess);
  else
    if (priv_data->jtag_ctrl.debug || (priv_data->jtag_ctrl.mode >= 0))
      bsdl_msg(BSDL_MSG_WARN, _("Line %d, %s.\n"),
               bsdl_flex_get_lineno(priv_data->scanner),
               Warnmess);
}
/*----------------------------------------------------------------------*/
static void Give_Up_And_Quit(parser_priv_t *priv_data)
{
  Print_Error(priv_data, "Too many errors");
}
/*----------------------------------------------------------------------*/
void yyerror(parser_priv_t *priv_data, const char *error_string)
{
}
/*----------------------------------------------------------------------*/
parser_priv_t *bsdl_parser_init(FILE *f, int mode, int debug)
{
  parser_priv_t *new_priv;

  if (!(new_priv = (parser_priv_t *)malloc(sizeof(parser_priv_t)))) {
    bsdl_msg(BSDL_MSG_ERR, _("Out of memory, %s line %i\n"), __FILE__, __LINE__);
    return NULL;
  }

  new_priv->jtag_ctrl.mode  = mode;
  new_priv->jtag_ctrl.debug = debug;

  new_priv->Reading_Package = 0;
  new_priv->buffer_for_switch = NULL;
  new_priv->len_buffer_for_switch = 0;

  if (!(new_priv->scanner = bsdl_flex_init(f, mode, debug))) {
    free(new_priv);
    new_priv = NULL;
  }

  bsdl_sem_init(new_priv);

  return new_priv;
}
/*----------------------------------------------------------------------*/
void bsdl_parser_deinit(parser_priv_t *priv_data)
{
  bsdl_sem_deinit(priv_data);
  bsdl_flex_deinit(priv_data->scanner);
  free(priv_data);
}
