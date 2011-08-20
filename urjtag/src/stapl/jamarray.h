/****************************************************************************/
/*                                                                          */
/*  Module:         jamarray.h                                              */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Constants and function prototypes for array support     */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMARRAY_H
#define INC_JAMARRAY_H

#include <stdint.h>

JAM_RETURN_TYPE urj_jam_read_boolean_array_data
    (JAMS_HEAP_RECORD *heap_record, char *statement_buffer);

JAM_RETURN_TYPE urj_jam_read_integer_array_data
    (JAMS_HEAP_RECORD *heap_record, char *statement_buffer);

JAM_RETURN_TYPE urj_jam_get_array_value
    (JAMS_SYMBOL_RECORD *symbol_record, int32_t index, int32_t *value);

#endif /* INC_JAMARRAY_H */
