/****************************************************************************/
/*                                                                          */
/*  Module:         jamexprt.h                                              */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    JAM Interpreter Export Header File                      */
/*                                                                          */
/*  Revisions:      1.1 removed error code JAMC_UNSUPPORTED FEATURE, added  */
/*                  JAMC_VECTOR_MAP_FAILED                                  */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMEXPRT_H
#define INC_JAMEXPRT_H

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/****************************************************************************/
/*                                                                          */
/*  Return codes from most JAM functions                                    */
/*                                                                          */
/****************************************************************************/

#define JAM_RETURN_TYPE int

#define JAMC_SUCCESS           0
#define JAMC_OUT_OF_MEMORY     1
#define JAMC_IO_ERROR          2
#define JAMC_SYNTAX_ERROR      3
#define JAMC_UNEXPECTED_END    4
#define JAMC_UNDEFINED_SYMBOL  5
#define JAMC_REDEFINED_SYMBOL  6
#define JAMC_INTEGER_OVERFLOW  7
#define JAMC_DIVIDE_BY_ZERO    8
#define JAMC_CRC_ERROR         9
#define JAMC_INTERNAL_ERROR   10
#define JAMC_BOUNDS_ERROR     11
#define JAMC_TYPE_MISMATCH    12
#define JAMC_ASSIGN_TO_CONST  13
#define JAMC_NEXT_UNEXPECTED  14
#define JAMC_POP_UNEXPECTED   15
#define JAMC_RETURN_UNEXPECTED 16
#define JAMC_ILLEGAL_SYMBOL   17
#define JAMC_VECTOR_MAP_FAILED 18
#define JAMC_USER_ABORT        19
#define JAMC_STACK_OVERFLOW    20
#define JAMC_ILLEGAL_OPCODE    21
#define JAMC_PHASE_ERROR       22
#define JAMC_SCOPE_ERROR       23
#define JAMC_ACTION_NOT_FOUND  24

/****************************************************************************/
/*                                                                          */
/*  Function Prototypes                                                     */
/*                                                                          */
/****************************************************************************/

JAM_RETURN_TYPE urj_jam_execute
    (char *program,
     int32_t program_size,
     char *workspace,
     int32_t workspace_size,
     char *action,
     char **init_list,
     int reset_jtag,
     int32_t *error_line, int *exit_code, int *format_version);

JAM_RETURN_TYPE urj_jam_get_note
    (char *program,
     int32_t program_size,
     int32_t *offset, char *key, char *value, int length);

JAM_RETURN_TYPE urj_jam_check_crc
    (char *program,
     int32_t program_size,
     unsigned short *expected_crc, unsigned short *actual_crc);

int urj_jam_getc (void);

int urj_jam_seek (int32_t offset);

int urj_jam_jtag_io (int tms, int tdi, int read_tdo);

void urj_jam_message (const char *message_text);

void urj_jam_export_integer (const char *key, int32_t value);

void urj_jam_export_boolean_array (char *key, unsigned char *data, int32_t count);

int jam_vector_map (int signal_count, char **signals);

int jam_vector_io
    (int signal_count,
     int32_t *dir_vect, int32_t *data_vect, int32_t *capture_vect);

#endif /* INC_JAMEXPRT_H */
