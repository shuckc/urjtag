/****************************************************************************/
/*                                                                          */
/*  Module:         jamcrc.c                                                */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Functions to calculate Cyclic Redundancy Check codes    */
/*                                                                          */
/****************************************************************************/

#include <stdint.h>
#include "jamexprt.h"
#include "jamdefs.h"
#include "jamexec.h"
#include "jamutil.h"

#include <libiberty.h>

void urj_jam_crc_init (unsigned short *shift_register);
void urj_jam_crc_update (unsigned short *shift_register, int data);
unsigned short urj_jam_get_crc_value (unsigned short *shift_register);
int urj_jam_check_crc (char *program, int32_t program_size,
                   unsigned short *expected_crc, unsigned short *actual_crc);
/****************************************************************************/
/*                                                                          */

void
urj_jam_crc_init (unsigned short *shift_register)
/*                                                                          */
/*  Description:    This function initializes CRC shift register.  It must  */
/*                  be called before urj_jam_crc_update().                      */
/*                                                                          */
/*  Returns:        Nothing                                                 */
/*                                                                          */
/****************************************************************************/
{
    *shift_register = 0xffff;   /* start with all ones in shift reg */
}

/****************************************************************************/
/*                                                                          */

void
urj_jam_crc_update (unsigned short *shift_register, int data)
/*                                                                          */
/*  Description:    This function updates crc shift register by shifting    */
/*                  in the new data bits.  Must be called for each bytes in */
/*                  the order that they appear in the data stream.          */
/*                                                                          */
/*  Returns:        Nothing                                                 */
/*                                                                          */
/****************************************************************************/
{
    int bit, feedback;
    unsigned short shift_register_copy;

    shift_register_copy = *shift_register;      /* copy it to local variable */

    for (bit = 0; bit < 8; bit++)       /* compute for each bit */
    {
        feedback = (data ^ shift_register_copy) & 0x01;
        shift_register_copy >>= 1;      /* shift the shift register */
        if (feedback)
        {
            shift_register_copy ^= 0x8408;      /* invert selected bits */
        }
        data >>= 1;             /* get the next bit of input_byte */
    }

    *shift_register = shift_register_copy;
}

/****************************************************************************/
/*                                                                          */

unsigned short
urj_jam_get_crc_value (unsigned short *shift_register)
/*                                                                          */
/*  Description:    The content of the shift_register is the CRC of all     */
/*                  bytes passed to urj_jam_crc_update() since the last call    */
/*                  to urj_jam_crc_init().                                      */
/*                                                                          */
/*  Returns:        CRC value from shift register.                          */
/*                                                                          */
/***************************************************************************/
{
    /* CRC is complement of shift register */
    return (unsigned short) ~(*shift_register);
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_check_crc
    (char *program,
     int32_t program_size,
     unsigned short *expected_crc, unsigned short *actual_crc)
/*                                                                          */
/*  Description:    This function reads the entire input stream and         */
/*                  computes the CRC of everything up to the CRC statement  */
/*                  itself (and the preceding new-line, if applicable).     */
/*                  Carriage return characters (0x0d) which are followed    */
/*                  by new-line characters (0x0a) are ignored, so the CRC   */
/*                  will not change when the file is converted from MS-DOS  */
/*                  text format (with CR-LF) to UNIX text format (only LF)  */
/*                  and visa-versa.                                         */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    BOOL comment = false;
    BOOL quoted_string = false;
    BOOL in_statement = false;
    BOOL in_instruction = false;
    BOOL found_expected_crc = false;
    int ch = 0;
    int32_t position = 0L;
    int32_t left_quote_position = -1L;
    unsigned short crc_shift_register = 0;
    unsigned short crc_shift_register_backup[4] = { 0 };
    int ch_queue[4] = { 0 };
    unsigned short tmp_expected_crc = 0;
    unsigned short tmp_actual_crc = 0;
    JAM_RETURN_TYPE status = JAMC_SUCCESS;

    urj_jam_program = program;
    urj_jam_program_size = program_size;

    status = urj_jam_seek (0);

    urj_jam_crc_init (&crc_shift_register);

    while ((status == JAMC_SUCCESS) && (!found_expected_crc))
    {
        ch = urj_jam_getc ();

        if ((ch != EOF) && (ch != JAMC_RETURN_CHAR))
        {
            urj_jam_crc_update (&crc_shift_register, ch);

            if ((!comment) && (!quoted_string))
            {
                if (ch == JAMC_COMMENT_CHAR)
                {
                    /* beginning of comment */
                    comment = true;
                }
                else if (ch == JAMC_QUOTE_CHAR)
                {
                    /* beginning of quoted string */
                    quoted_string = true;
                    left_quote_position = position;
                }
            }

            /*
             *      Check if this is the CRC statement
             */
            if ((!comment) && (!quoted_string) &&
                in_statement && in_instruction &&
                (isspace ((char) ch_queue[3])) &&
                (ch_queue[2] == 'C') &&
                (ch_queue[1] == 'R') &&
                (ch_queue[0] == 'C') && (isspace ((char) ch)))
            {
                status = JAMC_SYNTAX_ERROR;
                crc_shift_register = crc_shift_register_backup[3];

                /* skip over any additional white space */
                do
                {
                    ch = urj_jam_getc ();
                }
                while ((ch != EOF) && (isspace ((char) ch)));

                if (isxdigit ((char) ch))
                {
                    /* get remaining three characters of CRC */
                    ch_queue[2] = urj_jam_getc ();
                    ch_queue[1] = urj_jam_getc ();
                    ch_queue[0] = urj_jam_getc ();

                    if ((isxdigit ((char) ch_queue[2])) &&
                        (isxdigit ((char) ch_queue[1])) &&
                        (isxdigit ((char) ch_queue[0])))
                    {
                        tmp_expected_crc = (unsigned short)
                            ((hex_value (ch) << 12) |
                             (hex_value (ch_queue[2]) << 8) |
                             (hex_value (ch_queue[1]) << 4) |
                             hex_value (ch_queue[0]));

                        /* skip over any additional white space */
                        do
                        {
                            ch = urj_jam_getc ();
                        }
                        while ((ch != EOF) && (isspace ((char) ch)));

                        if (ch == JAMC_SEMICOLON_CHAR)
                        {
                            status = JAMC_SUCCESS;
                            found_expected_crc = true;
                        }
                    }
                }
            }

            /* check if we are reading the instruction name */
            if ((!comment) && (!quoted_string) && (!in_statement) &&
                (jam_is_name_char ((char) ch)))
            {
                in_statement = true;
                in_instruction = true;
            }

            /* check if we are finished reading the instruction name */
            if ((!comment) && (!quoted_string) && in_statement &&
                in_instruction && (!jam_is_name_char ((char) ch)))
            {
                in_instruction = false;
            }

            if ((!comment) && (!quoted_string) && in_statement &&
                (ch == JAMC_SEMICOLON_CHAR))
            {
                /* end of statement */
                in_statement = false;
            }

            if (comment &&
                ((ch == JAMC_NEWLINE_CHAR) || (ch == JAMC_RETURN_CHAR)))
            {
                /* end of comment */
                comment = false;
            }
            else if (quoted_string && (ch == JAMC_QUOTE_CHAR) &&
                     (position > left_quote_position))
            {
                /* end of quoted string */
                quoted_string = false;
            }
        }

        if (ch == EOF)
        {
            /* end of file */
            status = JAMC_UNEXPECTED_END;
        }

        ++position;             /* position of next character to be read */

        if (ch != JAMC_RETURN_CHAR)
        {
            ch_queue[3] = ch_queue[2];
            ch_queue[2] = ch_queue[1];
            ch_queue[1] = ch_queue[0];
            ch_queue[0] = ch;

            crc_shift_register_backup[3] = crc_shift_register_backup[2];
            crc_shift_register_backup[2] = crc_shift_register_backup[1];
            crc_shift_register_backup[1] = crc_shift_register_backup[0];
            crc_shift_register_backup[0] = crc_shift_register;
        }
    }

    tmp_actual_crc = urj_jam_get_crc_value (&crc_shift_register);

    if (found_expected_crc && (expected_crc != NULL))
    {
        *expected_crc = tmp_expected_crc;
    }

    if (actual_crc != NULL)
    {
        *actual_crc = tmp_actual_crc;
    }

    if (found_expected_crc && (status == JAMC_SUCCESS) &&
        (tmp_expected_crc != tmp_actual_crc))
    {
        status = JAMC_CRC_ERROR;
    }

    return status;
}
