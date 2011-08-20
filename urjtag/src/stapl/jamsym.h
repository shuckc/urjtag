/****************************************************************************/
/*                                                                          */
/*  Module:         jamsym.h                                                */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Prototypes for symbol-table management functions        */
/*                                                                          */
/*  Revisions:      1.1 added urj_jam_free_symbol_table()                       */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMSYM_H
#define INC_JAMSYM_H

#include <stdint.h>

/****************************************************************************/
/*                                                                          */
/*  Type definitions                                                        */
/*                                                                          */
/****************************************************************************/

/* types of symbolic names */
typedef enum
{
    JAM_ILLEGAL_SYMBOL_TYPE = 0,
    JAM_LABEL,
    JAM_INTEGER_SYMBOL,
    JAM_BOOLEAN_SYMBOL,
    JAM_INTEGER_ARRAY_WRITABLE,
    JAM_BOOLEAN_ARRAY_WRITABLE,
    JAM_INTEGER_ARRAY_INITIALIZED,
    JAM_BOOLEAN_ARRAY_INITIALIZED,
    JAM_DATA_BLOCK,
    JAM_PROCEDURE_BLOCK,
    JAM_SYMBOL_MAX
} JAME_SYMBOL_TYPE;

/* symbol record structure */
typedef struct JAMS_SYMBOL_STRUCT
{
    char name[JAMC_MAX_NAME_LENGTH + 1];
    JAME_SYMBOL_TYPE type;
    int32_t value;
    int32_t position;
    struct JAMS_SYMBOL_STRUCT *parent;
    struct JAMS_SYMBOL_STRUCT *next;
} JAMS_SYMBOL_RECORD;

/****************************************************************************/
/*                                                                          */
/*  Global variables                                                        */
/*                                                                          */
/****************************************************************************/

extern JAMS_SYMBOL_RECORD **urj_jam_symbol_table;

extern void *urj_jam_symbol_bottom;

extern JAMS_SYMBOL_RECORD *urj_jam_current_block;

extern int urj_jam_version;

extern BOOL urj_jam_checking_uses_list;

/****************************************************************************/
/*                                                                          */
/*  Function prototypes                                                     */
/*                                                                          */
/****************************************************************************/

JAM_RETURN_TYPE urj_jam_init_symbol_table (void);

void urj_jam_free_symbol_table (void);

JAM_RETURN_TYPE urj_jam_add_symbol
    (JAME_SYMBOL_TYPE type, char *name, int32_t value, int32_t position);

JAM_RETURN_TYPE urj_jam_get_symbol_value
    (JAME_SYMBOL_TYPE type, char *name, int32_t *value);

JAM_RETURN_TYPE urj_jam_set_symbol_value
    (JAME_SYMBOL_TYPE type, char *name, int32_t value);

JAM_RETURN_TYPE urj_jam_get_symbol_record
    (char *name, JAMS_SYMBOL_RECORD **symbol_record);

#endif /* INC_JAMSYM_H */
