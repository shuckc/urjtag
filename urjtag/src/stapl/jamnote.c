/****************************************************************************/
/*                                                                          */
/*  Module:         jamnote.c                                               */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Functions to extract NOTE fields from an JAM program    */
/*                                                                          */
/****************************************************************************/

#include "jamexprt.h"
#include "jamdefs.h"
#include "jamexec.h"
#include "jamutil.h"
#include <stdint.h>

BOOL urj_jam_get_note_key (char *statement_buffer, int32_t *key_begin,
                       int32_t *key_end);
BOOL urj_jam_get_note_value (char *statement_buffer, int32_t *value_begin,
                         int32_t *value_end);
int urj_jam_get_note (char *program, int32_t program_size, int32_t *offset,
                  char *key, char *value, int length);


/****************************************************************************/
/*                                                                          */

BOOL urj_jam_get_note_key
    (char *statement_buffer, int32_t *key_begin, int32_t *key_end)
/*                                                                          */
/*  Description:    This function finds the note key name in the statement  */
/*                  buffer and returns the start and end offsets            */
/*                                                                          */
/*  Returns:        true for success, false if key not found                */
/*                                                                          */
/****************************************************************************/
{
    int index = 0;
    BOOL quoted_string = false;

    index = urj_jam_skip_instruction_name (statement_buffer);

    /*
     *      Check if key string has quotes
     */
    if ((statement_buffer[index] == JAMC_QUOTE_CHAR) &&
        (index < JAMC_MAX_STATEMENT_LENGTH))
    {
        quoted_string = true;
        ++index;
    }

    /*
     *      Mark the beginning of the key string
     */
    *key_begin = index;

    /*
     *      Now find the end of the key string
     */
    if (quoted_string)
    {
        /* look for matching quote */
        while ((statement_buffer[index] != JAMC_NULL_CHAR) &&
               (statement_buffer[index] != JAMC_QUOTE_CHAR) &&
               (index < JAMC_MAX_STATEMENT_LENGTH))
        {
            ++index;
        }

        if (statement_buffer[index] == JAMC_QUOTE_CHAR)
        {
            *key_end = index;
        }
    }
    else
    {
        /* look for white space */
        while ((statement_buffer[index] != JAMC_NULL_CHAR) &&
               (!isspace (statement_buffer[index])) &&
               (index < JAMC_MAX_STATEMENT_LENGTH))
        {
            ++index;            /* skip over white space */
        }

        if (isspace (statement_buffer[index]))
        {
            *key_end = index;
        }
    }

    return (*key_end > *key_begin) ? true : false;
}

/****************************************************************************/
/*                                                                          */

BOOL urj_jam_get_note_value
    (char *statement_buffer, int32_t *value_begin, int32_t *value_end)
/*                                                                          */
/*  Description:    Finds the value field of a NOTE.  Could be enclosed in  */
/*                  quotation marks, or could not be.  Must be followed by  */
/*                  a semicolon.                                            */
/*                                                                          */
/*  Returns:        true for success, false for failure                     */
/*                                                                          */
/****************************************************************************/
{
    int index = 0;
    BOOL quoted_string = false;
    BOOL status = false;

    /* skip over white space */
    while ((statement_buffer[index] != JAMC_NULL_CHAR) &&
           (isspace (statement_buffer[index])) &&
           (index < JAMC_MAX_STATEMENT_LENGTH))
    {
        ++index;
    }

    /*
     *      Check if value string has quotes
     */
    if ((statement_buffer[index] == JAMC_QUOTE_CHAR) &&
        (index < JAMC_MAX_STATEMENT_LENGTH))
    {
        quoted_string = true;
        ++index;
    }

    /*
     *      Mark the beginning of the value string
     */
    *value_begin = index;

    /*
     *      Now find the end of the value string
     */
    if (quoted_string)
    {
        /* look for matching quote */
        while ((statement_buffer[index] != JAMC_NULL_CHAR) &&
               (statement_buffer[index] != JAMC_QUOTE_CHAR) &&
               (index < JAMC_MAX_STATEMENT_LENGTH))
        {
            ++index;
        }

        if (statement_buffer[index] == JAMC_QUOTE_CHAR)
        {
            *value_end = index;
            status = true;
            ++index;
        }
    }
    else
    {
        /* look for white space or semicolon */
        while ((statement_buffer[index] != JAMC_NULL_CHAR) &&
               (statement_buffer[index] != JAMC_SEMICOLON_CHAR) &&
               (!isspace (statement_buffer[index])) &&
               (index < JAMC_MAX_STATEMENT_LENGTH))
        {
            ++index;            /* skip over non-white space */
        }

        if ((statement_buffer[index] == JAMC_SEMICOLON_CHAR) ||
            (isspace (statement_buffer[index])))
        {
            *value_end = index;
            status = true;
        }
    }

    if (status)
    {
        while ((statement_buffer[index] != JAMC_NULL_CHAR) &&
               (isspace (statement_buffer[index])) &&
               (index < JAMC_MAX_STATEMENT_LENGTH))
        {
            ++index;            /* skip over white space */
        }

        /*
         *      Next character must be semicolon
         */
        if (statement_buffer[index] != JAMC_SEMICOLON_CHAR)
        {
            status = false;
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_get_note
    (char *program,
     int32_t program_size,
     int32_t *offset, char *key, char *value, int length)
/*                                                                          */
/*  Description:    Gets key and value of NOTE fields in the JAM file.      */
/*                  Can be called in two modes:  if offset pointer is NULL, */
/*                  then the function searches for note fields which match  */
/*                  the key string provided.  If offset is not NULL, then   */
/*                  the function finds the next note field of any key,      */
/*                  starting at the offset specified by the offset pointer. */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    char statement_buffer[JAMC_MAX_STATEMENT_LENGTH + 1];
    char label_buffer[JAMC_MAX_NAME_LENGTH + 1];
    JAME_INSTRUCTION instruction = JAM_ILLEGAL_INSTR;
    int32_t key_begin = 0L;
    int32_t key_end = 0L;
    int32_t value_begin = 0L;
    int32_t value_end = 0L;
    BOOL done = false;
    char *tmp_program = urj_jam_program;
    int32_t tmp_program_size = urj_jam_program_size;
    int32_t tmp_current_file_position = urj_jam_current_file_position;
    int32_t tmp_current_statement_position = urj_jam_current_statement_position;
    int32_t tmp_next_statement_position = urj_jam_next_statement_position;

    urj_jam_program = program;
    urj_jam_program_size = program_size;

    urj_jam_current_statement_position = 0L;
    urj_jam_next_statement_position = 0L;

    if (offset == NULL)
    {
        /*
         *      We will search for the first note with a specific key, and
         *      return only the value
         */
        status = urj_jam_seek (0L);
        urj_jam_current_file_position = 0L;
    }
    else
    {
        /*
         *      We will search for the next note, regardless of the key, and
         *      return both the value and the key
         */
        status = urj_jam_seek (*offset);
        urj_jam_current_file_position = *offset;
    }

    /*
     *      Get program statements and look for NOTE statements
     */
    while ((!done) && (status == JAMC_SUCCESS))
    {
        status = urj_jam_get_statement (statement_buffer, label_buffer);

        if (status == JAMC_SUCCESS)
        {
            instruction = urj_jam_get_instruction (statement_buffer);

            if (instruction == JAM_NOTE_INSTR)
            {
                if (urj_jam_get_note_key (statement_buffer, &key_begin, &key_end))
                {
                    statement_buffer[key_end] = JAMC_NULL_CHAR;

                    if ((offset != NULL)
                        || (strcasecmp (key, &statement_buffer[key_begin]) ==
                            0))
                    {
                        if (urj_jam_get_note_value
                            (&statement_buffer[key_end + 1], &value_begin,
                             &value_end))
                        {
                            done = true;
                            value_begin += (key_end + 1);
                            value_end += (key_end + 1);
                            statement_buffer[value_end] = JAMC_NULL_CHAR;

                            if (offset != NULL)
                            {
                                *offset = urj_jam_current_file_position;
                            }
                        }
                        else
                        {
                            status = JAMC_SYNTAX_ERROR;
                        }
                    }
                }
                else
                {
                    status = JAMC_SYNTAX_ERROR;
                }
            }
        }
    }

    /*
     *      Copy the key and value strings into buffers provided
     */
    if (done && (status == JAMC_SUCCESS))
    {
        if (offset != NULL)
        {
            /* only copy the key string if we were looking for all NOTEs */
            strncpy (key, &statement_buffer[key_begin],
                         JAMC_MAX_NAME_LENGTH);
        }
        strncpy (value, &statement_buffer[value_begin], length);
    }

    urj_jam_program = tmp_program;
    urj_jam_program_size = tmp_program_size;
    urj_jam_current_file_position = tmp_current_file_position;
    urj_jam_current_statement_position = tmp_current_statement_position;
    urj_jam_next_statement_position = tmp_next_statement_position;

    return status;
}
