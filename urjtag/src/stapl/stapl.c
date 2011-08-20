/*
 * $Id$
 *
 * Copyright (C) 2011, Michael Vacek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * Written by Michael Vacek <michael.vacek@gmail.com>, 2011.
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "jamexprt.h"
#include "jamutil.h"
#include <urjtag/chain.h>
#include <urjtag/cable.h>

/***********************************************************************
*   Global variables
***********************************************************************/
/* UrJTAG */
static urj_cable_t *current_cable;
static urj_chain_t *current_chain;

/* file buffer for JAM input file */
static char *file_buffer = NULL;
static int32_t file_pointer = 0L;
static int32_t file_length = 0L;

int urj_jam_getc (void);
int urj_jam_seek (int32_t offset);
int urj_jam_jtag_io (int tms, int tdi, int read_tdo);
int urj_jam_jtag_io_transfer (int count, char *tdi, char *tdo);
void urj_jam_message (const char *message_text);
void urj_jam_export_integer (const char *key, int32_t value);
void urj_jam_export_boolean_array (char *key, unsigned char *data, int32_t count);
void urj_jam_flush_and_delay (int32_t microseconds);
int urj_stapl_run (urj_chain_t *chain, char *STAPL_file_name,
                   char *STAPL_action);

int
urj_jam_getc (void)
{
    int ch = EOF;

    if (file_pointer < file_length)
    {
        ch = (int) file_buffer[file_pointer++];
    }

    return ch;
}

int
urj_jam_seek (int32_t offset)
{
    int return_code = EOF;

    if ((offset >= 0L) && (offset < file_length))
    {
        file_pointer = offset;
        return_code = 0;
    }

    return return_code;
}

// Bitwise JTAG communication via UrJTAG
int
urj_jam_jtag_io (int tms, int tdi, int read_tdo)
{
    int tdo = 0;

    if (read_tdo)
    {
        urj_tap_cable_defer_get_tdo (current_cable);
        tdo = urj_tap_cable_get_tdo_late (current_cable);
    }

    urj_tap_chain_defer_clock (current_chain, tms ? 0x01 : 0, tdi ? 0x01 : 0,
                               1);

    return tdo;
}

// Vector-based JTAG communication via UrJTAG
int
urj_jam_jtag_io_transfer (int count, char *tdi, char *tdo)
{
    int i = 0;
    int status = 1;
    char *temp_in;
    char *temp_out;

    // if no data are requested, only schedule tdo transmit
    if (tdo == NULL)
    {
        for (i = 0; i < count; i++)
            urj_jam_jtag_io ((i == count - 1),
                             tdi[i >> 3] & (1 << (i & 7)),
                             (tdo != NULL));
    }
    else
    {
        temp_in = malloc (count);
        temp_out = malloc (count);
        if ((temp_in == NULL) || (temp_out == NULL))
            status = 0;

        if (status != 0)
        {
            // decode bytes into bits to use them in UrJTAG interface
            for (i = 0; i < count; i++)
            {
                temp_in[i] = tdi[i >> 3] & (1 << (i & 7));
            }

            /* loop in the SHIFT-DR(IR) state, TMS set to 0 */
            urj_tap_cable_defer_transfer (current_cable, count - 1,
                                          temp_in, temp_out);


            // get the last bit in register and change TMS to 1
            urj_tap_cable_defer_get_tdo (current_cable);
            urj_tap_chain_defer_clock (current_chain, 1, temp_in[count - 1],
                                       1);

            urj_tap_cable_flush (current_cable, URJ_TAP_CABLE_COMPLETELY);

            urj_tap_cable_transfer_late (current_cable, temp_out);
            temp_out[count - 1] = urj_tap_cable_get_tdo_late (current_cable);


            // code bits back into bytes for Jam STAPL Player
            for (i = 0; i < count; i++)
            {
                if (temp_out[i])
                {
                    tdo[i >> 3] |= (1 << (i & 7));
                }
                else
                {
                    tdo[i >> 3] &= ~(unsigned int) (1 << (i & 7));
                }
            }
            free (temp_in);
            free (temp_out);
        }
        else
        {
            return status;
        }
    }

    return status;
}

void
urj_jam_message (const char *message_text)
{
    urj_log (URJ_LOG_LEVEL_NORMAL, "%s\n", message_text);
}

void
urj_jam_export_integer (const char *key, int32_t value)
{
    urj_log (URJ_LOG_LEVEL_DETAIL, "Export: key = \"%s\", value = %d\n", key,
             value);
}

#define HEX_LINE_CHARS 72
#define HEX_LINE_BITS (HEX_LINE_CHARS * 4)

void
urj_jam_export_boolean_array (char *key, unsigned char *data, int32_t count)
{
    uint32_t size, line, lines, linebits, value, j, k;
    char string[HEX_LINE_CHARS + 1];
    int32_t i, offset;

    if (count > HEX_LINE_BITS)
    {
        urj_log (URJ_LOG_LEVEL_DETAIL,
                 "Export: key = \"%s\", %d bits, value = HEX\n", key, count);
        lines = (count + (HEX_LINE_BITS - 1)) / HEX_LINE_BITS;

        for (line = 0; line < lines; ++line)
        {
            if (line < (lines - 1))
            {
                linebits = HEX_LINE_BITS;
                size = HEX_LINE_CHARS;
                offset = count - ((line + 1) * HEX_LINE_BITS);
            }
            else
            {
                linebits = count - ((lines - 1) * HEX_LINE_BITS);
                size = (linebits + 3) / 4;
                offset = 0L;
            }

            string[size] = '\0';
            j = size - 1;
            value = 0;

            for (k = 0; k < linebits; ++k)
            {
                i = k + offset;
                if (data[i >> 3] & (1 << (i & 7)))
                    value |= (1 << (i & 3));
                if ((i & 3) == 3)
                {
                    string[j] = (char) jam_todigit (value);
                    value = 0;
                    --j;
                }
            }
            if ((k & 3) > 0)
                string[j] = (char) jam_todigit (value);

            urj_log (URJ_LOG_LEVEL_DETAIL, "%s\n", string);
        }
    }
    else
    {
        size = (count + 3) / 4;
        string[size] = '\0';
        j = size - 1;
        value = 0;

        for (i = 0; i < count; ++i)
        {
            if (data[i >> 3] & (1 << (i & 7)))
                value |= (1 << (i & 3));
            if ((i & 3) == 3)
            {
                string[j] = (char) jam_todigit (value);
                value = 0;
                --j;
            }
        }
        if ((i & 3) > 0)
            string[j] = (char) jam_todigit (value);

        urj_log (URJ_LOG_LEVEL_DETAIL,
                 "Export: key = \"%s\", %d bits, value = HEX %s\n", key,
                 count, string);
    }
}

void
urj_jam_flush_and_delay (int32_t microseconds)
{
    urj_tap_cable_flush (current_cable, URJ_TAP_CABLE_COMPLETELY);
    usleep (microseconds);
}

static const char * const error_text[] = {
/* JAMC_SUCCESS            0 */ "success",
/* JAMC_OUT_OF_MEMORY      1 */ "out of memory",
/* JAMC_IO_ERROR           2 */ "file access error",
/* JAMC_SYNTAX_ERROR       3 */ "syntax error",
/* JAMC_UNEXPECTED_END     4 */ "unexpected end of file",
/* JAMC_UNDEFINED_SYMBOL   5 */ "undefined symbol",
/* JAMC_REDEFINED_SYMBOL   6 */ "redefined symbol",
/* JAMC_INTEGER_OVERFLOW   7 */ "integer overflow",
/* JAMC_DIVIDE_BY_ZERO     8 */ "divide by zero",
/* JAMC_CRC_ERROR          9 */ "CRC mismatch",
/* JAMC_INTERNAL_ERROR    10 */ "internal error",
/* JAMC_BOUNDS_ERROR      11 */ "bounds error",
/* JAMC_TYPE_MISMATCH     12 */ "type mismatch",
/* JAMC_ASSIGN_TO_CONST   13 */ "assignment to constant",
/* JAMC_NEXT_UNEXPECTED   14 */ "NEXT unexpected",
/* JAMC_POP_UNEXPECTED    15 */ "POP unexpected",
/* JAMC_RETURN_UNEXPECTED 16 */ "RETURN unexpected",
/* JAMC_ILLEGAL_SYMBOL    17 */ "illegal symbol name",
/* JAMC_VECTOR_MAP_FAILED 18 */ "vector signal name not found",
/* JAMC_USER_ABORT        19 */ "execution cancelled",
/* JAMC_STACK_OVERFLOW    20 */ "stack overflow",
/* JAMC_ILLEGAL_OPCODE    21 */ "illegal instruction code",
/* JAMC_PHASE_ERROR       22 */ "phase error",
/* JAMC_SCOPE_ERROR       23 */ "scope error",
/* JAMC_ACTION_NOT_FOUND  24 */ "action not found",
};

static const char * const exit_text_v2[] = {
    "Success",
    "Checking chain failure",
    "Reading IDCODE failure",
    "Reading USERCODE failure",
    "Reading UESCODE failure",
    "Entering ISP failure",
    "Unrecognized device",
    "Device revision is not supported",
    "Erase failure",
    "Device is not blank",
    "Device programming failure",
    "Device verify failure",
    "Read failure",
    "Calculating checksum failure",
    "Setting security bit failure",
    "Querying security bit failure",
    "Exiting ISP failure",
    "Performing system test failure",
    "Unknown exit code",
};

static const char * const exit_text_vd[] = {
    "Success",
    "Illegal initialization values",
    "Unrecognized device",
    "Device revision is not supported",
    "Device programming failure",
    "Device is not blank",
    "Device verify failure",
    "SRAM configuration failure",
    "Unknown exit code",
};

/***********************************************************************
 *
 * JAM PLAYER main function
 *
 **********************************************************************/
/* *********************************************************************
 * urj_stapl_run (chain, STAPL_file_name, STAPL_action);
 *
 * Main entry point for the 'stapl' command. Calls the stapl parser.
 *
 * Checks the jtag-environment (availability of SIR instruction and SDR
 * register). Initializes all svf-global variables and performs clean-up
 * afterwards.
 *
 * Parameter:
 *   chain            : pointer to global chain
 *   STAPL_file_name  : file name of STAPL file
 *   stop_on_mismatch : 1 = stop upon tdo mismatch
 *                      0 = continue upon mismatch
 *   ref_freq         : reference frequency for RUNTEST
 *
 * Return value:
 *   URJ_STATUS_OK, URJ_STATUS_FAIL
 * ********************************************************************/
int
urj_stapl_run (urj_chain_t *chain, char *STAPL_file_name, char *STAPL_action)
{

    bool help = false;
    char *filename = NULL;
    int32_t offset = 0L;
    int32_t error_line = 0L;
    JAM_RETURN_TYPE crc_result = JAMC_SUCCESS;
    JAM_RETURN_TYPE exec_result = JAMC_SUCCESS;
    unsigned short expected_crc = 0;
    unsigned short actual_crc = 0;
    char key[33] = { 0 };
    char value[257] = { 0 };
    int exit_status = 0;
    int exit_code = 0;
    int format_version = 0;
    time_t start_time = 0;
    time_t end_time = 0;
    int time_delta = 0;
    char *workspace = NULL;
    char *action = NULL;
    char *init_list[10];
    FILE *fp = NULL;
    struct stat sbuf;
    int32_t workspace_size = 0;
    const char *exit_string = NULL;
    int reset_jtag = 1;

    init_list[0] = NULL;

    /* print out the version string and copyright message */
    urj_log (URJ_LOG_LEVEL_NORMAL,
             "Jam STAPL Player Version 2.5 (20040526)\nCopyright\
 (C) 1997-2004 Altera Corporation\n");

    filename = STAPL_file_name;
    action = &STAPL_action[2];

    if (chain == NULL)
    {
        urj_error_set (URJ_ERROR_NO_CHAIN, "%s: no JTAG chain available",
                       (const char *restrict) "stapl");
        return URJ_STATUS_FAIL;
    }
    if (chain->parts == NULL)
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       "%s: chain without any parts", "stapl");
        return URJ_STATUS_FAIL;
    }
    if (chain->cable == NULL)
    {
        urj_error_set (URJ_ERROR_NOTFOUND,
                       "%s: chain without cable", "stapl");
        return URJ_STATUS_FAIL;
    }
    else
    {
        current_chain = chain;
        current_cable = chain->cable;
    }

    if (help || (filename == NULL))
    {
        exit_status = 1;
    }
    else if ((workspace_size > 0) &&
             ((workspace =
               (char *) malloc ((size_t) workspace_size)) == NULL))
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 "Error: can't allocate memory (%d Kbytes)\n",
                 (int) (workspace_size / 1024L));
        exit_status = 1;
    }
    else if (access (filename, 0) != 0)
    {
        urj_log (URJ_LOG_LEVEL_ERROR, "Error: can't access file \"%s\"\n",
                 filename);
        exit_status = 1;
    }
    else
    {
        /* get length of file */
        if (stat (filename, &sbuf) == 0)
            file_length = sbuf.st_size;

        if ((fp = fopen (filename, "rb")) == NULL)
        {
            urj_log (URJ_LOG_LEVEL_ERROR, "Error: can't open file \"%s\"\n",
                     filename);
            exit_status = 1;
        }
        else
        {
            /*
             *  Read entire file into a buffer
             */

            file_buffer = (char *) malloc ((size_t) file_length);

            if (file_buffer == NULL)
            {
                urj_log (URJ_LOG_LEVEL_ERROR,
                         "Error: can't allocate memory (%d Kbytes)\n",
                         (int) (file_length / 1024L));
                exit_status = 1;
            }
            else
            {
                if (fread (file_buffer, 1, (size_t) file_length, fp) !=
                    (size_t) file_length)
                {
                    urj_log (URJ_LOG_LEVEL_ERROR,
                             "Error reading file \"%s\"\n", filename);
                    exit_status = 1;
                }
            }

            fclose (fp);
        }

        if (exit_status == 0)
        {
            /*
             *  Check CRC
             */
            crc_result = urj_jam_check_crc (file_buffer, file_length,
                                        &expected_crc, &actual_crc);

            switch (crc_result)
            {
            case JAMC_SUCCESS:
                urj_log (URJ_LOG_LEVEL_DETAIL,
                         "CRC matched: CRC value = %04X\n", actual_crc);
                break;

            case JAMC_CRC_ERROR:
                urj_log (URJ_LOG_LEVEL_ERROR,
                         "CRC mismatch: expected %04X, actual %04X\n",
                         expected_crc, actual_crc);
                break;

            case JAMC_UNEXPECTED_END:
                urj_log
                    (URJ_LOG_LEVEL_DETAIL,
                     "Expected CRC not found, actual CRC value = %04X\n",
                     actual_crc);
                break;

            default:
                urj_log (URJ_LOG_LEVEL_ERROR,
                         "CRC function returned error code %d\n", crc_result);
                break;
            }

            /*
             *  Dump out NOTE fields
             */
            while (urj_jam_get_note (file_buffer, file_length,
                                 &offset, key, value, 256) == 0)
            {
                urj_log (URJ_LOG_LEVEL_DETAIL, "NOTE \"%s\" = \"%s\"\n", key,
                         value);
            }

            // Execute the JAM program
            time (&start_time);

            exec_result = urj_jam_execute (file_buffer, file_length,
                                       workspace, workspace_size, action,
                                       init_list, reset_jtag, &error_line,
                                       &exit_code, &format_version);

            time (&end_time);

            if (exec_result == JAMC_SUCCESS)
            {
                if (format_version == 2)
                {
                    exit_string = exit_text_v2[exit_code];
                }
                else
                {
                    exit_string = exit_text_vd[exit_code];
                }

                urj_log (URJ_LOG_LEVEL_NORMAL, "Exit code = %d... %s\n",
                         exit_code, exit_string);
            }
            else if ((format_version == 2) &&
                     (exec_result == JAMC_ACTION_NOT_FOUND))
            {
                if ((action == NULL) || (*action == '\0'))
                {
                    urj_log
                        (URJ_LOG_LEVEL_NORMAL,
                         "Error: no action specified for Jam file.\nProgram terminated.\n");
                }
                else
                {
                    urj_log
                        (URJ_LOG_LEVEL_NORMAL,
                         "Error: action \"%s\" is not supported for this Jam file.\nProgram terminated.\n",
                         action);
                }
            }
            else if (exec_result < ARRAY_SIZE(error_text))
            {
                urj_log (URJ_LOG_LEVEL_NORMAL,
                         "Error on line %d: %s.\nProgram terminated.\n",
                         error_line, error_text[exec_result]);
            }
            else
            {
                urj_log (URJ_LOG_LEVEL_NORMAL, "Unknown error code %d\n",
                         exec_result);
            }

            /*
             *      Print out elapsed time
             */
            time_delta = (int) (end_time - start_time);
            urj_log (URJ_LOG_LEVEL_DETAIL, "Elapsed time = %02u:%02u:%02u\n", time_delta / 3600,        /* hours */
                     (time_delta % 3600) / 60,  /* minutes */
                     time_delta % 60);  /* seconds */
        }
    }

    if (workspace != NULL)
        free (workspace);
    if (file_buffer != NULL)
        free (file_buffer);

    urj_log (URJ_LOG_LEVEL_NORMAL, "STAPL execution finished \n");

    return URJ_STATUS_OK;
}
