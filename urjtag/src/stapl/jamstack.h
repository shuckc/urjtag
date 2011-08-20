/****************************************************************************/
/*                                                                          */
/*  Module:         jamstack.h                                              */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Prototypes for stack management functions               */
/*                                                                          */
/*  Revisions:      1.1 added urj_jam_free_stack()                              */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMSTACK_H
#define INC_JAMSTACK_H

#include <stdint.h>

/****************************************************************************/
/*                                                                          */
/*  Type definitions                                                        */
/*                                                                          */
/****************************************************************************/

/* types of stack records */
typedef enum
{
    JAM_ILLEGAL_STACK_TYPE = 0,
    JAM_STACK_FOR_NEXT,
    JAM_STACK_PUSH_POP,
    JAM_STACK_CALL_RETURN,
    JAM_STACK_MAX
} JAME_STACK_RECORD_TYPE;

/* stack record structure */
typedef struct
{
    JAME_STACK_RECORD_TYPE type;
    JAMS_SYMBOL_RECORD *iterator;       /* used only for FOR/NEXT */
    int32_t for_position;       /* used only for FOR/NEXT */
    int32_t stop_value;         /* used only for FOR/NEXT */
    int32_t step_value;         /* used only for FOR/NEXT */
    int32_t push_value;         /* used only for PUSH/POP */
    int32_t return_position;    /* used only for CALL/RETURN */
} JAMS_STACK_RECORD;

/****************************************************************************/
/*                                                                          */
/*  Global variables                                                        */
/*                                                                          */
/****************************************************************************/

extern JAMS_STACK_RECORD *urj_jam_stack;

/****************************************************************************/
/*                                                                          */
/*  Function prototypes                                                     */
/*                                                                          */
/****************************************************************************/

JAM_RETURN_TYPE urj_jam_init_stack (void);

void urj_jam_free_stack (void);

JAM_RETURN_TYPE urj_jam_push_stack_record (JAMS_STACK_RECORD *stack_record);

JAMS_STACK_RECORD *urj_jam_peek_stack_record (void);

JAM_RETURN_TYPE urj_jam_pop_stack_record (void);

JAM_RETURN_TYPE urj_jam_push_fornext_record
    (JAMS_SYMBOL_RECORD *iterator,
     int32_t for_position, int32_t stop_value, int32_t step_value);

JAM_RETURN_TYPE urj_jam_push_pushpop_record (int32_t value);

JAM_RETURN_TYPE urj_jam_push_callret_record (int32_t return_position);

#endif /* INC_JAMSTACK_H */
