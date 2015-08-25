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
%parse-param {urj_vhdl_parser_priv_t *priv_data}
/* See https://lists.gnu.org/archive/html/bug-bison/2014-02/msg00002.html */
%lex-param {void *HACK}
%defines
%name-prefix "urj_vhdl_"

%{
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <urjtag/part.h>

#include "bsdl_sysdep.h"

#include "bsdl_types.h"
#include "bsdl_msg.h"

/* interface to flex */
#include "vhdl_bison.h"
#include "vhdl_parser.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define HACK priv_data->scanner
int yylex (YYSTYPE *, void *);

#if 1
#define ERROR_LIMIT 15
#define BUMP_ERROR \
    do { \
        if (urj_vhdl_flex_postinc_compile_errors (priv_data->scanner) > ERROR_LIMIT) \
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

static void Init_Text (urj_vhdl_parser_priv_t *);
static void Store_Text (urj_vhdl_parser_priv_t *, char *);
static void Print_Error (urj_vhdl_parser_priv_t *, const char *);
static void Give_Up_And_Quit (urj_vhdl_parser_priv_t *);

/* VHDL semantic action interface */
static void urj_vhdl_set_entity (urj_vhdl_parser_priv_t *, char *);
static void urj_vhdl_port_add_name (urj_vhdl_parser_priv_t *, char *);
static void urj_vhdl_port_add_bit (urj_vhdl_parser_priv_t *);
static void urj_vhdl_port_add_range (urj_vhdl_parser_priv_t *, int, int);
static void urj_vhdl_port_apply_port (urj_vhdl_parser_priv_t *);

//static void set_attr_bool (urj_vhdl_parser_priv_t *, char *, int);
static void set_attr_decimal (urj_vhdl_parser_priv_t *, char *, int);
static void set_attr_string (urj_vhdl_parser_priv_t *, char *, char *);
//static void set_attr_real (urj_vhdl_parser_priv_t *, char *, char *);
//static void set_attr_const (urj_vhdl_parser_priv_t *, char *, char *);

void yyerror (urj_vhdl_parser_priv_t *, const char *);
%}

%union
{
  int   integer;
  char *str;
}


%token ENTITY  PORT  GENERIC  USE  ATTRIBUTE  IS
%token OF  CONSTANT  STRING  END  ALL
%token PHYSICAL_PIN_MAP  PIN_MAP_STRING  TRUE  FALSE  SIGNAL
%token LOW  BOTH  IN  OUT  INOUT
%token BUFFER  LINKAGE  BIT  BIT_VECTOR  TO  DOWNTO
%token PACKAGE  BODY  TYPE  SUBTYPE  RECORD  ARRAY
%token POSITIVE  RANGE  CELL_INFO
%token INPUT  OUTPUT2  OUTPUT3  CONTROL  CONTROLR  INTERNAL
%token CLOCK  BIDIR  BIDIR_IN  BIDIR_OUT  EXTEST  SAMPLE
%token INTEST  RUNBIST  PI  PO  UPD  CAP  X BIN_X_PATTERN
%token ZERO  ONE  Z  IDENTIFIER
%token SINGLE_QUOTE  QUOTED_STRING  DECIMAL_NUMBER
%token REAL_NUMBER  CONCATENATE  SEMICOLON  COMMA
%token LPAREN  RPAREN  COLON
%token BOX  COLON_EQUAL  PERIOD ILLEGAL
%token BSDL_EXTENSION
%token OBSERVE_ONLY
%token STD_1532_2001 STD_1532_2002

%type <str> BIN_X_PATTERN
%type <str> IDENTIFIER
%type <str> QUOTED_STRING
%type <integer> DECIMAL_NUMBER
%type <integer> Boolean
%type <str> REAL_NUMBER

%start BSDL_Program

%%  /* End declarations, begin rules */

BSDL_Program : Begin_BSDL BSDL_Body End_BSDL
;
Begin_BSDL : ENTITY IDENTIFIER IS
             { urj_vhdl_set_entity (priv_data, $2); }
           | error
             {
               Print_Error (priv_data, _("Improper Entity declaration"));
               Print_Error (priv_data, _("Check if source file is BSDL"));
               BUMP_ERROR; YYABORT;     /* Probably not a BSDL source file */
             }
;
BSDL_Body : VHDL_Generic
            VHDL_Port
            VHDL_Use_Part
            VHDL_Elements
          | error
            {
              Print_Error (priv_data, _("Syntax Error"));
              BUMP_ERROR; YYABORT;
            }
;
End_BSDL : END IDENTIFIER SEMICOLON
           { free ($2); }
         | error
           {
             Print_Error (priv_data, _("Syntax Error"));
             BUMP_ERROR; YYABORT;
           }
;
VHDL_Generic        : GENERIC LPAREN PHYSICAL_PIN_MAP COLON STRING COLON_EQUAL
                      Quoted_String  RPAREN SEMICOLON
;
VHDL_Port           : PORT LPAREN Port_Specifier_List RPAREN SEMICOLON
                    | error
                      {
                        Print_Error (priv_data, _("Improper Port declaration"));
                        BUMP_ERROR; YYABORT;
                      }
;
Port_Specifier_List : Port_Specifier
                    | Port_Specifier_List SEMICOLON Port_Specifier
;
Port_Specifier      : Port_List COLON Function Scaler_Or_Vector
                      { urj_vhdl_port_apply_port (priv_data); }
;
Port_List           : IDENTIFIER
                      { urj_vhdl_port_add_name (priv_data, $1); }
                    | Port_List COMMA IDENTIFIER
                      { urj_vhdl_port_add_name (priv_data, $3); }
;
Function            : IN | OUT | INOUT | BUFFER | LINKAGE
;
Scaler_Or_Vector    : BIT
                      { urj_vhdl_port_add_bit (priv_data); }
                    | BIT_VECTOR LPAREN Vector_Range RPAREN
;
Vector_Range        : DECIMAL_NUMBER TO DECIMAL_NUMBER
                      { urj_vhdl_port_add_range (priv_data, $1, $3); }
                    | DECIMAL_NUMBER DOWNTO DECIMAL_NUMBER
                      { urj_vhdl_port_add_range (priv_data, $3, $1); }
;
VHDL_Use_Part : ISC_Use
              | Standard_Use
              | Standard_Use ISC_Use
              | Standard_Use VHDL_Use_List
              | error
                {
                  Print_Error (priv_data, _("Error in Package declaration(s)"));
                  BUMP_ERROR; YYABORT;
                }
;
Standard_Use  : USE IDENTIFIER
                {/* Parse Standard 1149.1 Package */
                  strcpy (priv_data->Package_File_Name, $2);
                  free ($2);
                }
                PERIOD ALL SEMICOLON
                {
                  priv_data->Reading_Package = 1;
                  urj_vhdl_flex_switch_file (priv_data->scanner,
                                             priv_data->Package_File_Name);
                }
                Standard_Package
                {
                  priv_data->Reading_Package = 0;
                }
;
Standard_Package : PACKAGE IDENTIFIER IS Standard_Decls Defered_Constants
                   Standard_Decls END IDENTIFIER SEMICOLON Package_Body
                   { free ($2); free ($8); }
                 | error
                   {
                     Print_Error (priv_data, _("Error in Standard Package"));
                     BUMP_ERROR; YYABORT;
                   }
;
Standard_Decls : Standard_Decl
               | Standard_Decls Standard_Decl
;
Standard_Decl  : ATTRIBUTE IDENTIFIER COLON Attribute_Type SEMICOLON
                 { free ($2); }
               | TYPE IDENTIFIER IS Type_Body SEMICOLON
                 { free ($2); }
               | TYPE CELL_INFO IS ARRAY LPAREN POSITIVE RANGE BOX RPAREN
                 OF IDENTIFIER SEMICOLON
                 { free ($11); }
               | SUBTYPE PIN_MAP_STRING IS STRING SEMICOLON
               | SUBTYPE BSDL_EXTENSION IS STRING SEMICOLON
               | error
                 {
                   Print_Error (priv_data, _("Error in Standard Declarations"));
                   BUMP_ERROR; YYABORT;
                 }
;
Attribute_Type : IDENTIFIER
                 { free ($1); }
               | STRING
               | DECIMAL_NUMBER
               | BSDL_EXTENSION
               | error
                 {
                   Print_Error (priv_data, _("Error in Attribute type identification"));
                   BUMP_ERROR; YYABORT;
                 }
;
Type_Body      : LPAREN ID_Bits RPAREN
               | LPAREN ID_List RPAREN
               | LPAREN LOW COMMA BOTH RPAREN
               | ARRAY LPAREN DECIMAL_NUMBER TO DECIMAL_NUMBER RPAREN
                 OF IDENTIFIER
                 { free ($8); }
               | ARRAY LPAREN DECIMAL_NUMBER DOWNTO DECIMAL_NUMBER RPAREN
                 OF IDENTIFIER
                 { free ($8); }
               | RECORD Record_Body END RECORD
               | error
                 {
                   Print_Error (priv_data, _("Error in Type definition"));
                   BUMP_ERROR; YYABORT;
                 }
;
ID_Bits        : ID_Bit
               | ID_Bits COMMA ID_Bit
;
ID_List        : IDENTIFIER
                 { free ($1); }
               | ID_List COMMA IDENTIFIER
                 { free ($3); }
;
ID_Bit         : SINGLE_QUOTE BIN_X_PATTERN SINGLE_QUOTE
                 { free ($2); }
               | error
                 {
                   Print_Error (priv_data, _("Error in Bit definition"));
                   BUMP_ERROR; YYABORT;
                 }
;
Record_Body    : Record_Element
               | Record_Body Record_Element
;
Record_Element : IDENTIFIER COLON IDENTIFIER SEMICOLON
                 { free ($1); free ($3); }
               | error
                 {
                   Print_Error (priv_data, _("Error in Record Definition"));
                   BUMP_ERROR; YYABORT;
                 }
;
Defered_Constants : Defered_Constant
                  | Defered_Constants Defered_Constant
;
Defered_Constant  : CONSTANT Constant_Body
;
Constant_Body     : IDENTIFIER COLON CELL_INFO SEMICOLON
                    { free ($1); }
                  | error
                    {
                      Print_Error (priv_data, _("Error in defered constant"));
                      BUMP_ERROR; YYABORT;
                    }
;
VHDL_Use_List   : VHDL_Use
                | VHDL_Use_List VHDL_Use
;
Package_Body    : PACKAGE BODY IDENTIFIER IS Constant_List END IDENTIFIER
                  { free ($3); free ($7); }
                  SEMICOLON
                | error
                  {
                    Print_Error (priv_data, _("Error in Package Body definition"));
                    BUMP_ERROR; YYABORT;
                  }
;
Constant_List   : Cell_Constant
                | Constant_List Cell_Constant
;
Cell_Constant   : CONSTANT IDENTIFIER COLON CELL_INFO COLON_EQUAL
                  LPAREN Triples_List RPAREN SEMICOLON
                  { free ($2); }
                | error
                  {
                    Print_Error (priv_data, _("Error in Cell Constant definition"));
                    BUMP_ERROR; YYABORT;
                  }
;
Triples_List    : Triple
                | Triples_List COMMA Triple
;
Triple          : LPAREN Triple_Function COMMA Triple_Inst COMMA CAP_Data
                  RPAREN
                | error
                  {
                    Print_Error (priv_data, _("Error in Cell Data Record"));
                    BUMP_ERROR; YYABORT;
                  }
;
Triple_Function : INPUT | OUTPUT2 | OUTPUT3 | INTERNAL | CONTROL
                | CONTROLR | CLOCK | BIDIR_IN | BIDIR_OUT
                | OBSERVE_ONLY
                | error
                  {
                    Print_Error (priv_data, _("Error in Cell_Type Function field"));
                    BUMP_ERROR; YYABORT;
                  }
;
Triple_Inst     : EXTEST | SAMPLE | INTEST | RUNBIST
                | error
                  {
                    Print_Error (priv_data, _("Error in BScan_Inst Instruction field"));
                    BUMP_ERROR; YYABORT;
                  }
;
CAP_Data        : PI | PO | UPD | CAP | X | ZERO | ONE
                | error
                  {
                    Print_Error (priv_data, _("Error in Constant CAP data source field"));
                    BUMP_ERROR; YYABORT;
                  }
;
VHDL_Use         : USE IDENTIFIER
                   {/* Parse Standard 1149.1 Package */
                    strcpy(priv_data->Package_File_Name, $2);
                    free($2);
                   }
                   PERIOD ALL SEMICOLON
                   {
                     priv_data->Reading_Package = 1;
                     urj_vhdl_flex_switch_file (priv_data->scanner,
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
VHDL_Elements : VHDL_Element
              | VHDL_Elements VHDL_Element
              | error
                {
                  Print_Error (priv_data, _("Unknown VHDL statement"));
                  BUMP_ERROR; YYABORT;
                }
;
VHDL_Element  : VHDL_Constant
              | VHDL_Attribute
;
VHDL_Constant      : CONSTANT VHDL_Constant_Part
;
VHDL_Constant_Part : IDENTIFIER COLON PIN_MAP_STRING COLON_EQUAL
                     Quoted_String SEMICOLON
                     // { set_attr_const (priv_data, $1, strdup ("PIN_MAP_STRING")); }
                     { free ($1); }
;
VHDL_Attribute     : ATTRIBUTE VHDL_Attribute_Types
;
VHDL_Attribute_Types : VHDL_Attr_Boolean
                     | VHDL_Attr_Decimal
                     | VHDL_Attr_Real
                     | VHDL_Attr_String
                     | VHDL_Attr_PhysicalPinMap
                     | error
                       {
                         Print_Error (priv_data, _("Error in Attribute specification"));
                         BUMP_ERROR; YYABORT;
                       }
;
VHDL_Attr_Boolean  : IDENTIFIER OF IDENTIFIER COLON SIGNAL IS Boolean SEMICOLON
                     {
                       //set_attr_bool (priv_data, $1, $7);
                       //free ($3);
                       /* skip boolean attributes for the time being */
                       free ($1); free ($3);
                     }
;
Boolean            : TRUE
                     { $$ = 1; }
                   | FALSE
                     { $$ = 0; }
;
VHDL_Attr_Decimal : IDENTIFIER OF IDENTIFIER COLON ENTITY IS DECIMAL_NUMBER SEMICOLON
                    {
                      set_attr_decimal (priv_data, $1, $7);
                      free ($3);
                    }
;
VHDL_Attr_Real   : IDENTIFIER OF IDENTIFIER COLON SIGNAL IS LPAREN REAL_NUMBER COMMA Stop RPAREN SEMICOLON
                   {
                     //set_attr_real (priv_data, $1, $8);
                     //free ($3);
                     /* skip real attributes for the time being */
                     free ($1); free ($3); free ($8);
                   }
;
Stop             : LOW | BOTH
;
VHDL_Attr_String : IDENTIFIER OF IDENTIFIER COLON ENTITY IS Quoted_String SEMICOLON
                   {
                     set_attr_string (priv_data, $1, strdup (priv_data->buffer));
                     free ($3);
                   }
;
VHDL_Attr_PhysicalPinMap : IDENTIFIER OF IDENTIFIER COLON ENTITY IS PHYSICAL_PIN_MAP SEMICOLON
                           { free ($1); free ($3); }
;
Quoted_String    : QUOTED_STRING
                   {
                     Init_Text (priv_data);
                     Store_Text (priv_data, $1);
                     free ($1);
                   }
                 | Quoted_String CONCATENATE QUOTED_STRING
                   {
                     Store_Text (priv_data, $3);
                     free ($3);
                   }
;
ISC_Use      : USE ISC_Packages PERIOD ALL SEMICOLON
             {
               priv_data->Reading_Package = 1;
               urj_vhdl_flex_switch_file (priv_data->scanner,
                                          priv_data->Package_File_Name);
             }
             ISC_Package
             {
               priv_data->Reading_Package = 0;
             }
;
ISC_Packages : STD_1532_2001
               {
                 strcpy (priv_data->Package_File_Name, "STD_1532_2001");
               }
             | STD_1532_2002
               {
                 strcpy (priv_data->Package_File_Name, "STD_1532_2002");
               }
;
ISC_Package : ISC_Package_Header ISC_Package_Body
;
ISC_Package_Header : PACKAGE ISC_Packages IS
                     Standard_Use
                     {
                       priv_data->Reading_Package = 1;
                     }
                     Standard_Decls
                     END ISC_Packages SEMICOLON
;
ISC_Package_Body   : PACKAGE BODY ISC_Packages IS
                     END ISC_Packages SEMICOLON
;
%%  /* End rules, begin programs  */
/*****************************************************************************
 * void Init_Text( urj_vhdl_parser_priv_t *priv )
 *
 * Allocates the internal test buffer if not already existing.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
Init_Text (urj_vhdl_parser_priv_t *priv)
{
    if (priv->len_buffer == 0)
    {
        /* @@@@ RFHH check malloc result */
        priv->buffer = malloc (160);
        priv->len_buffer = 160;
    }
    priv->buffer[0] = '\0';
}


/*****************************************************************************
 * void Store_Text( urj_vhdl_parser_priv_t *priv, char *Source )
 *
 * Appends the given String to the internal text buffer. The buffer
 * is extended if the string does not fit into the current size.
 *
 * Parameters
 *   priv   : private data container for parser related tasks
 *   String : pointer to string that is to be added to buffer
 *
 * Returns
 *   void
 ****************************************************************************/
static void
Store_Text (urj_vhdl_parser_priv_t *priv, char *Source)
{                               /* Save characters from VHDL string in local string buffer.           */
    size_t req_len;
    char *SourceEnd;

    SourceEnd = ++Source;       /* skip leading '"' */
    while (*SourceEnd && (*SourceEnd != '"') && (*SourceEnd != '\n'))
        SourceEnd++;
    /* terminate Source string with NUL character */
    *SourceEnd = '\0';

    req_len = strlen (priv->buffer) + strlen (Source) + 1;
    if (req_len > priv->len_buffer)
    {
        /* @@@@ RFHH check realloc result */
        priv->buffer = realloc (priv->buffer, req_len);
        priv->len_buffer = req_len;
    }
    strcat (priv->buffer, Source);
}

/*----------------------------------------------------------------------*/
static void
Print_Error (urj_vhdl_parser_priv_t *priv_data, const char *Errmess)
{
    urj_bsdl_jtag_ctrl_t *jc = priv_data->jtag_ctrl;

    if (priv_data->Reading_Package)
        urj_bsdl_err (jc->proc_mode,
                      _("In Package %s, Line %d, %s.\n"),
                      priv_data->Package_File_Name,
                      urj_vhdl_flex_get_lineno (priv_data->scanner), Errmess);
    else
        urj_bsdl_err (jc->proc_mode,
                      _("Line %d, %s.\n"),
                      urj_vhdl_flex_get_lineno (priv_data->scanner), Errmess);

    /* set an error if nothing else is pending */
    if (urj_error_get () == URJ_ERROR_OK)
        urj_bsdl_err_set (jc->proc_mode, URJ_ERROR_BSDL_VHDL,
                          "Parser error, see log for details");
}

/*----------------------------------------------------------------------*/
static void
Give_Up_And_Quit (urj_vhdl_parser_priv_t *priv_data)
{
    Print_Error (priv_data, _("Too many errors"));
}

/*----------------------------------------------------------------------*/
void
yyerror (urj_vhdl_parser_priv_t *priv_data, const char *error_string)
{
}


/*****************************************************************************
 * void urj_vhdl_sem_init( urj_vhdl_parser_priv_t *priv )
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
urj_vhdl_sem_init (urj_vhdl_parser_priv_t *priv)
{
    priv->tmp_port_desc.names_list = NULL;
    priv->tmp_port_desc.next = NULL;

    priv->jtag_ctrl->port_desc = NULL;

    priv->jtag_ctrl->vhdl_elem_first = NULL;
    priv->jtag_ctrl->vhdl_elem_last = NULL;
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
 * void free_port_list( urj_bsdl_port_desc_t *pl, int free_me )
 *
 * Deallocates the given list of port_desc.
 *
 * Parameters
 *  pl      : first port_desc to deallocate
 *  free_me : set to 1 to free memory for ai as well
 *
 * Returns
 *  void
 ****************************************************************************/
static void
free_port_list (urj_bsdl_port_desc_t *pl, int free_me)
{
    if (pl)
    {
        free_string_list (pl->names_list);
        free_port_list (pl->next, 1);

        if (free_me)
            free (pl);
    }
}


/*****************************************************************************
 * void free_elem_list( urj_vhdl_elem_t *el )
 *
 * Deallocates the given list of vhdl_elem items.
 *
 * Parameters
 *  el : first vhdl_elem to deallocate
 *
 * Returns
 *  void
 ****************************************************************************/
static void
free_elem_list (urj_vhdl_elem_t *el)
{
    if (el)
    {
        free_elem_list (el->next);

        if (el->name)
            free (el->name);

        if (el->payload)
            free (el->payload);
        free (el);
    }
}


/*****************************************************************************
 * void urj_vhdl_sem_deinit( urj_vhdl_parser_priv_t *priv )
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
urj_vhdl_sem_deinit (urj_vhdl_parser_priv_t *priv_data)
{
    urj_bsdl_port_desc_t *pd = priv_data->jtag_ctrl->port_desc;
    urj_vhdl_elem_t *el = priv_data->jtag_ctrl->vhdl_elem_first;

    /* free port_desc list */
    free_port_list (pd, 1);
    free_port_list (&(priv_data->tmp_port_desc), 0);

    /* free VHDL element list */
    free_elem_list (el);

    priv_data->jtag_ctrl = NULL;
}


/*****************************************************************************
 * urj_vhdl_parser_priv_t *urj_vhdl_parser_init( FILE *f, urj_bsdl_jtag_ctrl_t *jtag_ctrl )
 *
 * Initializes storage elements in the private parser structure that are
 * used for parser maintenance purposes.
 * Subsequently calls initializer functions for the scanner and the semantic
 * parts.
 *
 * Parameters
 *   f         : descriptor of file for scanning
 *   jtag_ctrl : pointer to jtag control structure
 *
 * Returns
 *   pointer to private parser structure
 ****************************************************************************/
urj_vhdl_parser_priv_t *
urj_vhdl_parser_init (FILE *f, urj_bsdl_jtag_ctrl_t *jtag_ctrl)
{
    urj_vhdl_parser_priv_t *new_priv;

    if (!(new_priv = malloc (sizeof (urj_vhdl_parser_priv_t))))
    {
        urj_bsdl_ftl_set (jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
        return NULL;
    }

    new_priv->jtag_ctrl = jtag_ctrl;

    new_priv->Reading_Package = 0;
    new_priv->buffer = NULL;
    new_priv->len_buffer = 0;

    if (!(new_priv->scanner = urj_vhdl_flex_init (f, jtag_ctrl->proc_mode)))
    {
        free (new_priv);
        new_priv = NULL;
    }

    urj_vhdl_sem_init (new_priv);

    return new_priv;
}


/*****************************************************************************
 * void urj_vhdl_parser_deinit( urj_vhdl_parser_priv_t *priv )
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
urj_vhdl_parser_deinit (urj_vhdl_parser_priv_t *priv_data)
{
    if (priv_data->buffer)
    {
        free (priv_data->buffer);
        priv_data->buffer = NULL;
    }

    urj_vhdl_sem_deinit (priv_data);
    urj_vhdl_flex_deinit (priv_data->scanner);
    free (priv_data);
}

/*****************************************************************************
 * void urj_vhdl_set_entity( urj_vhdl_parser_priv_t *priv, char *entityname )
 *
 * Applies the entity name from BSDL as the part name.
 *
 * Parameters
 *   priv       : private data container for parser related tasks
 *   entityname : entity name string, memory gets free'd
 *
 * Returns
 *   void
 ****************************************************************************/
static void
urj_vhdl_set_entity (urj_vhdl_parser_priv_t *priv, char *entityname)
{
    if (priv->jtag_ctrl->proc_mode & URJ_BSDL_MODE_INSTR_EXEC)
    {
        strncpy (priv->jtag_ctrl->part->part, entityname,
                 URJ_PART_PART_MAXLEN);
        priv->jtag_ctrl->part->part[URJ_PART_PART_MAXLEN] = '\0';
    }

    free (entityname);
}

/*****************************************************************************
 * void urj_vhdl_port_add_name( urj_vhdl_parser_priv_t *priv, char *name )
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
urj_vhdl_port_add_name (urj_vhdl_parser_priv_t *priv, char *name)
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
 * void urj_vhdl_port_add_bit( urj_vhdl_parser_priv_t *priv )
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
urj_vhdl_port_add_bit (urj_vhdl_parser_priv_t *priv)
{
    urj_bsdl_port_desc_t *pd = &(priv->tmp_port_desc);

    pd->is_vector = 0;
    pd->low_idx = 0;
    pd->high_idx = 0;
}


/*****************************************************************************
 * void urj_vhdl_port_add_range( urj_vhdl_parser_priv_t *priv, int low, int high )
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
urj_vhdl_port_add_range (urj_vhdl_parser_priv_t *priv, int low,
                         int high)
{
    urj_bsdl_port_desc_t *pd = &(priv->tmp_port_desc);

    pd->is_vector = 1;
    pd->low_idx = low;
    pd->high_idx = high;
}

/*****************************************************************************
 * void urj_vhdl_port_apply_port( urj_vhdl_parser_priv_t *priv )
 * Port name management function
 *
 * Applies the current temporary port description to the final list
 * of port descriptions.
 *
 * Parameters
 *   priv : private data container for parser related tasks
 *
 * Returns
 *   void
 ****************************************************************************/
static void
urj_vhdl_port_apply_port (urj_vhdl_parser_priv_t *priv)
{
    urj_bsdl_port_desc_t *tmp_pd = &(priv->tmp_port_desc);
    urj_bsdl_port_desc_t *pd = malloc (sizeof (urj_bsdl_port_desc_t));

    if (pd)
    {
        /* insert at top of list */
        pd->next = priv->jtag_ctrl->port_desc;
        priv->jtag_ctrl->port_desc = pd;

        /* copy information from temporary port descriptor */
        pd->names_list = tmp_pd->names_list;
        pd->is_vector = tmp_pd->is_vector;
        pd->low_idx = tmp_pd->low_idx;
        pd->high_idx = tmp_pd->high_idx;

        /* and reset temporary port descriptor */
        tmp_pd->names_list = NULL;
        tmp_pd->next = NULL;
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}

static void
add_elem (urj_vhdl_parser_priv_t *priv, urj_vhdl_elem_t *el)
{
    urj_bsdl_jtag_ctrl_t *jc = priv->jtag_ctrl;

    el->next = NULL;
    if (jc->vhdl_elem_last)
        jc->vhdl_elem_last->next = el;
    jc->vhdl_elem_last = el;

    if (!jc->vhdl_elem_first)
        jc->vhdl_elem_first = el;

    el->line = urj_vhdl_flex_get_lineno (priv->scanner);
}

#if 0
static void
set_attr_bool (urj_vhdl_parser_priv_t *priv, char *name, int value)
{
    urj_vhdl_elem_t *el = malloc (sizeof (urj_vhdl_elem_t));

    if (el)
    {
        el->type = VET_ATTRIBUTE_BOOL;
        el->name = name;
        el->payload.bool = value;

        add_elem (priv, el);
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}
#endif

static void
set_attr_decimal (urj_vhdl_parser_priv_t *priv, char *name, int value)
{
    urj_vhdl_elem_t *el = malloc (sizeof (urj_vhdl_elem_t));
    char *string = malloc (10);

    if (el && string)
    {
        el->type = URJ_BSDL_VET_ATTRIBUTE_DECIMAL;
        el->name = name;
        snprintf (string, 10, "%d", value);
        el->payload = string;

        add_elem (priv, el);
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}

static void
set_attr_string (urj_vhdl_parser_priv_t *priv, char *name, char *string)
{
    urj_vhdl_elem_t *el = malloc (sizeof (urj_vhdl_elem_t));

    /* skip certain attributes */
    if ((strcasecmp (name, "DESIGN_WARNING") == 0)
        || (strcasecmp (name, "BOUNDARY_CELLS") == 0)
        || (strcasecmp (name, "INSTRUCTION_SEQUENCE") == 0)
        || (strcasecmp (name, "INSTRUCTION_USAGE") == 0)
        || (strcasecmp (name, "ISC_DESIGN_WARNING") == 0))
    {
        free (name);
        free (string);
        free (el);
        return;
    }

    if (el)
    {
        el->type = URJ_BSDL_VET_ATTRIBUTE_STRING;
        el->name = name;
        el->payload = string;

        add_elem (priv, el);
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}

#if 0
static void
set_attr_real (urj_vhdl_parser_priv_t *priv, char *name, char *string)
{
    urj_vhdl_elem_t *el = malloc (sizeof (urj_vhdl_elem_t));

    if (el)
    {
        el->type = VET_ATTRIBUTE_REAL;
        el->name = name;
        el->payload.real = string;

        add_elem (priv, el);
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}
#endif

#if 0
static void
set_attr_const (urj_vhdl_parser_priv_t *priv, char *name, char *string)
{
    urj_vhdl_elem_t *el = malloc (sizeof (urj_vhdl_elem_t));

    if (el)
    {
        el->type = URJ_BSDL_VET_CONSTANT;
        el->name = name;
        el->payload = string;

        add_elem (priv, el);
    }
    else
        urj_bsdl_ftl_set (priv->jtag_ctrl->proc_mode, URJ_ERROR_OUT_OF_MEMORY,
                          "No memory");
}
#endif


/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
