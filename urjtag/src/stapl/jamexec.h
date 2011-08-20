/****************************************************************************/
/*                                                                          */
/*  Module:         jamexec.h                                               */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Prototypes for statement execution functions            */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMEXEC_H
#define INC_JAMEXEC_H

#include <stdint.h>

/****************************************************************************/
/*                                                                          */
/*  Global variables                                                        */
/*                                                                          */
/****************************************************************************/

extern int32_t urj_jam_current_file_position;

extern int32_t urj_jam_current_statement_position;

extern int32_t urj_jam_next_statement_position;

/* prototype for external function in jamarray.c */
extern int urj_jam_6bit_char (int ch);

/* prototype for external function in jamsym.c */
extern BOOL urj_jam_check_init_list (char *name, int32_t *value);

/****************************************************************************/
/*                                                                          */
/*  Function Prototypes                                                     */
/*                                                                          */
/****************************************************************************/

JAM_RETURN_TYPE urj_jam_get_statement
    (char *statement_buffer, char *label_buffer);

int32_t urj_jam_get_line_of_position (int32_t position);

JAME_INSTRUCTION urj_jam_get_instruction (char *statement_buffer);

int urj_jam_skip_instruction_name (const char *statement_buffer);

#endif /* INC_JAMEXEC_H */
