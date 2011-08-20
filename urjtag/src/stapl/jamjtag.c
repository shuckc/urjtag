/****************************************************************************/
/*                                                                          */
/*  Module:         jamjtag.c                                               */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Contains array management functions, including          */
/*                  functions for reading array initialization data in      */
/*                  compressed formats.                                     */
/*                                                                          */
/*  Revisions:      1.1 allow WAIT USECS without using JTAG hardware if     */
/*                  JTAG hardware has not yet been initialized -- this is   */
/*                  needed to create delays in VECTOR (non-JTAG) programs   */
/*                                                                          */
/****************************************************************************/

#include <stdint.h>
#include "jamexprt.h"
#include "jamdefs.h"
#include "jamsym.h"
#include "jamstack.h"
#include "jamutil.h"
#include "jamjtag.h"

/*
*   Global variable to store the current JTAG state
*/
JAME_JTAG_STATE urj_jam_jtag_state = JAM_ILLEGAL_JTAG_STATE;

/*
*   Store current stop-state for DR and IR scan commands
*/
JAME_JTAG_STATE urj_jam_drstop_state = IDLE;
JAME_JTAG_STATE urj_jam_irstop_state = IDLE;

/*
*   Store current padding values
*/
int urj_jam_dr_preamble = 0;
int urj_jam_dr_postamble = 0;
int urj_jam_ir_preamble = 0;
int urj_jam_ir_postamble = 0;
int urj_jam_dr_length = 0;
int urj_jam_ir_length = 0;
int32_t *urj_jam_dr_preamble_data = NULL;
int32_t *urj_jam_dr_postamble_data = NULL;
int32_t *urj_jam_ir_preamble_data = NULL;
int32_t *urj_jam_ir_postamble_data = NULL;
char *urj_jam_dr_buffer = NULL;
char *urj_jam_ir_buffer = NULL;

/*
*   Table of JTAG state names
*/
struct JAMS_JTAG_MAP
{
    JAME_JTAG_STATE state;
    char string[JAMC_MAX_JTAG_STATE_LENGTH + 1];
} static const jam_jtag_state_table[] =
{
    {RESET, "RESET"},
    {IDLE, "IDLE"},
    {DRSELECT, "DRSELECT"},
    {DRCAPTURE, "DRCAPTURE"},
    {DRSHIFT, "DRSHIFT"},
    {DREXIT1, "DREXIT1"},
    {DRPAUSE, "DRPAUSE"},
    {DREXIT2, "DREXIT2"},
    {DRUPDATE, "DRUPDATE"},
    {IRSELECT, "IRSELECT"},
    {IRCAPTURE, "IRCAPTURE"},
    {IRSHIFT, "IRSHIFT"},
    {IREXIT1, "IREXIT1"},
    {IRPAUSE, "IRPAUSE"},
    {IREXIT2, "IREXIT2"},
    {IRUPDATE, "IRUPDATE"}
};

/*
*   This structure shows, for each JTAG state, which state is reached after
*   a single TCK clock cycle with TMS high or TMS low, respectively.  This
*   describes all possible state transitions in the JTAG state machine.
*/
struct JAMS_JTAG_MACHINE
{
    JAME_JTAG_STATE tms_high;
    JAME_JTAG_STATE tms_low;
} static const jam_jtag_state_transitions[] =
{
/* RESET     */
    {
    RESET, IDLE},
/* IDLE      */
    {
    DRSELECT, IDLE},
/* DRSELECT  */
    {
    IRSELECT, DRCAPTURE},
/* DRCAPTURE */
    {
    DREXIT1, DRSHIFT},
/* DRSHIFT   */
    {
    DREXIT1, DRSHIFT},
/* DREXIT1   */
    {
    DRUPDATE, DRPAUSE},
/* DRPAUSE   */
    {
    DREXIT2, DRPAUSE},
/* DREXIT2   */
    {
    DRUPDATE, DRSHIFT},
/* DRUPDATE  */
    {
    DRSELECT, IDLE},
/* IRSELECT  */
    {
    RESET, IRCAPTURE},
/* IRCAPTURE */
    {
    IREXIT1, IRSHIFT},
/* IRSHIFT   */
    {
    IREXIT1, IRSHIFT},
/* IREXIT1   */
    {
    IRUPDATE, IRPAUSE},
/* IRPAUSE   */
    {
    IREXIT2, IRPAUSE},
/* IREXIT2   */
    {
    IRUPDATE, IRSHIFT},
/* IRUPDATE  */
    {
    DRSELECT, IDLE}
};

/*
*   This table contains the TMS value to be used to take the NEXT STEP on
*   the path to the desired state.  The array index is the current state,
*   and the bit position is the desired endstate.  To find out which state
*   is used as the intermediate state, look up the TMS value in the
*   urj_jam_jtag_state_transitions[] table.
*/
static const unsigned short jam_jtag_path_map[16] = {
    0x0001, 0xFFFD, 0xFE01, 0xFFE7, 0xFFEF, 0xFF0F, 0xFFBF, 0xFF0F,
    0xFEFD, 0x0001, 0xF3FF, 0xF7FF, 0x87FF, 0xDFFF, 0x87FF, 0x7FFD
};

/*
*   Flag bits for urj_jam_jtag_io() function
*/
#define TMS_HIGH   1
#define TMS_LOW    0
#define TDI_HIGH   1
#define TDI_LOW    0
#define READ_TDO   1
#define IGNORE_TDO 0

int urj_jam_init_jtag (void);
int urj_jam_set_drstop_state (JAME_JTAG_STATE state);
int urj_jam_set_irstop_state (JAME_JTAG_STATE state);
int urj_jam_set_dr_preamble (int count, int start_index, int32_t *data);
int urj_jam_set_ir_preamble (int count, int start_index, int32_t *data);
int urj_jam_set_dr_postamble (int count, int start_index, int32_t *data);
int urj_jam_set_ir_postamble (int count, int start_index, int32_t *data);
void urj_jam_jtag_reset_idle (void);
int urj_jam_goto_jtag_state (JAME_JTAG_STATE state);
JAME_JTAG_STATE urj_jam_get_jtag_state_from_name (char *name);
int urj_jam_do_wait_cycles (int32_t cycles, JAME_JTAG_STATE wait_state);
int urj_jam_do_wait_microseconds (int32_t microseconds,
                              JAME_JTAG_STATE wait_state);
void urj_jam_jtag_concatenate_data (char *buffer, int32_t *preamble_data,
                                int32_t preamble_count, int32_t *target_data,
                                int32_t start_index, int32_t target_count,
                                int32_t *postamble_data,
                                int32_t postamble_count);
void urj_jam_jtag_extract_target_data (char *buffer, int32_t *target_data,
                                   int32_t start_index,
                                   int32_t preamble_count,
                                   int32_t target_count);
int urj_jam_jtag_drscan (int start_state, int count, char *tdi, char *tdo);
int urj_jam_jtag_irscan (int start_state, int count, char *tdi, char *tdo);
int urj_jam_do_irscan (int32_t count, int32_t *data, int32_t start_index);
int urj_jam_swap_ir (int32_t count, int32_t *in_data, int32_t in_index,
                 int32_t *out_data, int32_t out_index);
int urj_jam_do_drscan (int32_t count, int32_t *data, int32_t start_index);
int urj_jam_swap_dr (int32_t count, int32_t *in_data, int32_t in_index,
                 int32_t *out_data, int32_t out_index);
void urj_jam_free_jtag_padding_buffers (int reset_jtag);

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE
urj_jam_init_jtag (void)
/*                                                                          */
/****************************************************************************/
{
    void **symbol_table = NULL;
    JAMS_STACK_RECORD *stack = NULL;

    /* initial JTAG state is unknown */
    urj_jam_jtag_state = JAM_ILLEGAL_JTAG_STATE;

    /* initialize global variables to default state */
    urj_jam_drstop_state = IDLE;
    urj_jam_irstop_state = IDLE;
    urj_jam_dr_preamble = 0;
    urj_jam_dr_postamble = 0;
    urj_jam_ir_preamble = 0;
    urj_jam_ir_postamble = 0;
    urj_jam_dr_length = 0;
    urj_jam_ir_length = 0;

    if (urj_jam_workspace != NULL)
    {
        symbol_table = (void **) urj_jam_workspace;
        stack = (JAMS_STACK_RECORD *) & symbol_table[JAMC_MAX_SYMBOL_COUNT];
        urj_jam_dr_preamble_data = (int32_t *) & stack[JAMC_MAX_NESTING_DEPTH];
        urj_jam_dr_postamble_data =
            &urj_jam_dr_preamble_data[JAMC_MAX_JTAG_DR_PREAMBLE / 32];
        urj_jam_ir_preamble_data =
            &urj_jam_dr_postamble_data[JAMC_MAX_JTAG_DR_POSTAMBLE / 32];
        urj_jam_ir_postamble_data =
            &urj_jam_ir_preamble_data[JAMC_MAX_JTAG_IR_PREAMBLE / 32];
        urj_jam_dr_buffer =
            (char *) &urj_jam_ir_postamble_data[JAMC_MAX_JTAG_IR_POSTAMBLE / 32];
        urj_jam_ir_buffer = &urj_jam_dr_buffer[JAMC_MAX_JTAG_DR_LENGTH / 8];
    }
    else
    {
        urj_jam_dr_preamble_data = NULL;
        urj_jam_dr_postamble_data = NULL;
        urj_jam_ir_preamble_data = NULL;
        urj_jam_ir_postamble_data = NULL;
        urj_jam_dr_buffer = NULL;
        urj_jam_ir_buffer = NULL;
    }

    return JAMC_SUCCESS;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE
urj_jam_set_drstop_state (JAME_JTAG_STATE state)
/*                                                                          */
/****************************************************************************/
{
    urj_jam_drstop_state = state;

    return JAMC_SUCCESS;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE
urj_jam_set_irstop_state (JAME_JTAG_STATE state)
/*                                                                          */
/****************************************************************************/
{
    urj_jam_irstop_state = state;

    return JAMC_SUCCESS;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_set_dr_preamble
    (int count, int start_index, int32_t *data)
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    int alloc_longs = 0;
    int i = 0;
    int bit = 0;

    if (count >= 0)
    {
        if (urj_jam_workspace != NULL)
        {
            if (count > JAMC_MAX_JTAG_DR_PREAMBLE)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_dr_preamble = count;
            }
        }
        else
        {
            if (count > urj_jam_dr_preamble)
            {
                alloc_longs = (count + 31) >> 5;
                free (urj_jam_dr_preamble_data);
                urj_jam_dr_preamble_data =
                    (int32_t *) malloc (alloc_longs * sizeof (int32_t));

                if (urj_jam_dr_preamble_data == NULL)
                {
                    status = JAMC_OUT_OF_MEMORY;
                }
                else
                {
                    urj_jam_dr_preamble = count;
                }
            }
            else
            {
                urj_jam_dr_preamble = count;
            }
        }

        if (status == JAMC_SUCCESS)
        {
            for (i = 0; i < count; ++i)
            {
                bit = i + start_index;

                if (data == NULL)
                {
                    urj_jam_dr_preamble_data[i >> 5] |= (1L << (bit & 0x1f));
                }
                else
                {
                    if (data[bit >> 5] & (1L << (bit & 0x1f)))
                    {
                        urj_jam_dr_preamble_data[i >> 5] |= (1L << (bit & 0x1f));
                    }
                    else
                    {
                        urj_jam_dr_preamble_data[i >> 5] &=
                            ~(uint32_t) (1L << (bit & 0x1f));
                    }
                }
            }
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_set_ir_preamble
    (int count, int start_index, int32_t *data)
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    int alloc_longs = 0;
    int i = 0;
    int bit = 0;

    if (count >= 0)
    {
        if (urj_jam_workspace != NULL)
        {
            if (count > JAMC_MAX_JTAG_IR_PREAMBLE)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_ir_preamble = count;
            }
        }
        else
        {
            if (count > urj_jam_ir_preamble)
            {
                alloc_longs = (count + 31) >> 5;
                free (urj_jam_ir_preamble_data);
                urj_jam_ir_preamble_data =
                    (int32_t *) malloc (alloc_longs * sizeof (int32_t));

                if (urj_jam_ir_preamble_data == NULL)
                {
                    status = JAMC_OUT_OF_MEMORY;
                }
                else
                {
                    urj_jam_ir_preamble = count;
                }
            }
            else
            {
                urj_jam_ir_preamble = count;
            }
        }

        if (status == JAMC_SUCCESS)
        {
            for (i = 0; i < count; ++i)
            {
                bit = i + start_index;

                if (data == NULL)
                {
                    urj_jam_ir_preamble_data[i >> 5] |= (1L << (bit & 0x1f));
                }
                else
                {
                    if (data[bit >> 5] & (1L << (bit & 0x1f)))
                    {
                        urj_jam_ir_preamble_data[i >> 5] |= (1L << (bit & 0x1f));
                    }
                    else
                    {
                        urj_jam_ir_preamble_data[i >> 5] &=
                            ~(uint32_t) (1L << (bit & 0x1f));
                    }
                }
            }
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_set_dr_postamble
    (int count, int start_index, int32_t *data)
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    int alloc_longs = 0;
    int i = 0;
    int bit = 0;

    if (count >= 0)
    {
        if (urj_jam_workspace != NULL)
        {
            if (count > JAMC_MAX_JTAG_DR_POSTAMBLE)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_dr_postamble = count;
            }
        }
        else
        {
            if (count > urj_jam_dr_postamble)
            {
                alloc_longs = (count + 31) >> 5;
                free (urj_jam_dr_postamble_data);
                urj_jam_dr_postamble_data =
                    (int32_t *) malloc (alloc_longs * sizeof (int32_t));

                if (urj_jam_dr_postamble_data == NULL)
                {
                    status = JAMC_OUT_OF_MEMORY;
                }
                else
                {
                    urj_jam_dr_postamble = count;
                }
            }
            else
            {
                urj_jam_dr_postamble = count;
            }
        }

        if (status == JAMC_SUCCESS)
        {
            for (i = 0; i < count; ++i)
            {
                bit = i + start_index;

                if (data == NULL)
                {
                    urj_jam_dr_postamble_data[i >> 5] |= (1L << (bit & 0x1f));
                }
                else
                {
                    if (data[bit >> 5] & (1L << (bit & 0x1f)))
                    {
                        urj_jam_dr_postamble_data[i >> 5] |= (1L << (bit & 0x1f));
                    }
                    else
                    {
                        urj_jam_dr_postamble_data[i >> 5] &=
                            ~(uint32_t) (1L << (bit & 0x1f));
                    }
                }
            }
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_set_ir_postamble
    (int count, int start_index, int32_t *data)
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    int alloc_longs = 0;
    int i = 0;
    int bit = 0;

    if (count >= 0)
    {
        if (urj_jam_workspace != NULL)
        {
            if (count > JAMC_MAX_JTAG_IR_POSTAMBLE)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_ir_postamble = count;
            }
        }
        else
        {
            if (count > urj_jam_ir_postamble)
            {
                alloc_longs = (count + 31) >> 5;
                free (urj_jam_ir_postamble_data);
                urj_jam_ir_postamble_data =
                    (int32_t *) malloc (alloc_longs * sizeof (int32_t));

                if (urj_jam_ir_postamble_data == NULL)
                {
                    status = JAMC_OUT_OF_MEMORY;
                }
                else
                {
                    urj_jam_ir_postamble = count;
                }
            }
            else
            {
                urj_jam_ir_postamble = count;
            }
        }

        if (status == JAMC_SUCCESS)
        {
            for (i = 0; i < count; ++i)
            {
                bit = i + start_index;

                if (data == NULL)
                {
                    urj_jam_ir_postamble_data[i >> 5] |= (1L << (bit & 0x1f));
                }
                else
                {
                    if (data[bit >> 5] & (1L << (bit & 0x1f)))
                    {
                        urj_jam_ir_postamble_data[i >> 5] |= (1L << (bit & 0x1f));
                    }
                    else
                    {
                        urj_jam_ir_postamble_data[i >> 5] &=
                            ~(uint32_t) (1L << (bit & 0x1f));
                    }
                }
            }
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

void
urj_jam_jtag_reset_idle (void)
/*                                                                          */
/****************************************************************************/
{
    int i = 0;

    /*
     *      Go to Test Logic Reset (no matter what the starting state may be)
     */
    for (i = 0; i < 5; ++i)
    {
        urj_jam_jtag_io (TMS_HIGH, TDI_LOW, IGNORE_TDO);
    }

    /*
     *      Now step to Run Test / Idle
     */
    urj_jam_jtag_io (TMS_LOW, TDI_LOW, IGNORE_TDO);

    urj_jam_jtag_state = IDLE;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE
urj_jam_goto_jtag_state (JAME_JTAG_STATE state)
/*                                                                          */
/****************************************************************************/
{
    int tms = 0;
    int count = 0;
    JAM_RETURN_TYPE status = JAMC_SUCCESS;

    if (urj_jam_jtag_state == JAM_ILLEGAL_JTAG_STATE)
    {
        /* initialize JTAG chain to known state */
        urj_jam_jtag_reset_idle ();
    }

    if (urj_jam_jtag_state == state)
    {
        /*
         *      We are already in the desired state.  If it is a stable state,
         *      loop here.  Otherwise do nothing (no clock cycles).
         */
        if ((state == IDLE) ||
            (state == DRSHIFT) ||
            (state == DRPAUSE) || (state == IRSHIFT) || (state == IRPAUSE))
        {
            urj_jam_jtag_io (TMS_LOW, TDI_LOW, IGNORE_TDO);
        }
        else if (state == RESET)
        {
            urj_jam_jtag_io (TMS_HIGH, TDI_LOW, IGNORE_TDO);
        }
    }
    else
    {
        while ((urj_jam_jtag_state != state) && (count < 9))
        {
            /*
             *      Get TMS value to take a step toward desired state
             */
            tms = (jam_jtag_path_map[urj_jam_jtag_state] & (1 << state)) ?
                TMS_HIGH : TMS_LOW;

            /*
             *      Take a step
             */
            urj_jam_jtag_io (tms, TDI_LOW, IGNORE_TDO);

            if (tms)
            {
                urj_jam_jtag_state =
                    jam_jtag_state_transitions[urj_jam_jtag_state].tms_high;
            }
            else
            {
                urj_jam_jtag_state =
                    jam_jtag_state_transitions[urj_jam_jtag_state].tms_low;
            }

            ++count;
        }
    }

    if (urj_jam_jtag_state != state)
    {
        status = JAMC_INTERNAL_ERROR;
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAME_JTAG_STATE
urj_jam_get_jtag_state_from_name (char *name)
/*                                                                          */
/*  Description:    Finds JTAG state code corresponding to name of state    */
/*                  supplied as a string                                    */
/*                                                                          */
/*  Returns:        JTAG state code, or JAM_ILLEGAL_JTAG_STATE if string    */
/*                  does not match any valid state name                     */
/*                                                                          */
/****************************************************************************/
{
    int i = 0;
    JAME_JTAG_STATE jtag_state = JAM_ILLEGAL_JTAG_STATE;

    for (i = 0; (jtag_state == JAM_ILLEGAL_JTAG_STATE) &&
         (i < (int) ARRAY_SIZE(jam_jtag_state_table)); ++i)
    {
        if (strcmp (name, jam_jtag_state_table[i].string) == 0)
        {
            jtag_state = jam_jtag_state_table[i].state;
        }
    }

    return jtag_state;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_do_wait_cycles
    (int32_t cycles, JAME_JTAG_STATE wait_state)
/*                                                                          */
/*  Description:    Causes JTAG hardware to loop in the specified stable    */
/*                  state for the specified number of TCK clock cycles.     */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    int tms = 0;
    int32_t count = 0L;
    JAM_RETURN_TYPE status = JAMC_SUCCESS;

    if (urj_jam_jtag_state != wait_state)
    {
        status = urj_jam_goto_jtag_state (wait_state);
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Set TMS high to loop in RESET state
         *      Set TMS low to loop in any other stable state
         */
        tms = (wait_state == RESET) ? TMS_HIGH : TMS_LOW;

        for (count = 0L; count < cycles; count++)
        {
            urj_jam_jtag_io (tms, TDI_LOW, IGNORE_TDO);
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_do_wait_microseconds
    (int32_t microseconds, JAME_JTAG_STATE wait_state)
/*                                                                          */
/*  Description:    Causes JTAG hardware to sit in the specified stable     */
/*                  state for the specified duration of real time.  If      */
/*                  no JTAG operations have been performed yet, then only   */
/*                  a delay is performed.  This permits the WAIT USECS      */
/*                  statement to be used in VECTOR programs without causing */
/*                  any JTAG operations.                                    */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    JAM_RETURN_TYPE status = JAMC_SUCCESS;

    if ((urj_jam_jtag_state != JAM_ILLEGAL_JTAG_STATE) &&
        (urj_jam_jtag_state != wait_state))
    {
        status = urj_jam_goto_jtag_state (wait_state);
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *  Flush all data and wait for specified time interval
         */
        urj_jam_flush_and_delay (microseconds);
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

void urj_jam_jtag_concatenate_data
    (char *buffer,
     int32_t *preamble_data,
     int32_t preamble_count,
     int32_t *target_data,
     int32_t start_index,
     int32_t target_count, int32_t *postamble_data, int32_t postamble_count)
/*                                                                          */
/*  Description:    Copies preamble data, target data, and postamble data   */
/*                  into one buffer for IR or DR scans.  Note that buffer   */
/*                  is an array of char, while other arrays are of int32_t      */
/*                                                                          */
/*  Returns:        nothing                                                 */
/*                                                                          */
/****************************************************************************/
{
    int32_t i = 0L;
    int32_t j = 0L;
    int32_t k = 0L;

    for (i = 0L; i < preamble_count; ++i)
    {
        if (preamble_data[i >> 5] & (1L << (i & 0x1f)))
        {
            buffer[i >> 3] |= (1 << (i & 7));
        }
        else
        {
            buffer[i >> 3] &= ~(unsigned int) (1 << (i & 7));
        }
    }

    j = start_index;
    k = preamble_count + target_count;
    for (; i < k; ++i, ++j)
    {
        if (target_data[j >> 5] & (1L << (j & 0x1f)))
        {
            buffer[i >> 3] |= (1 << (i & 7));
        }
        else
        {
            buffer[i >> 3] &= ~(unsigned int) (1 << (i & 7));
        }
    }

    j = 0L;
    k = preamble_count + target_count + postamble_count;
    for (; i < k; ++i, ++j)
    {
        if (postamble_data[j >> 5] & (1L << (j & 0x1f)))
        {
            buffer[i >> 3] |= (1 << (i & 7));
        }
        else
        {
            buffer[i >> 3] &= ~(unsigned int) (1 << (i & 7));
        }
    }
}

/****************************************************************************/
/*                                                                          */

void urj_jam_jtag_extract_target_data
    (char *buffer,
     int32_t *target_data,
     int32_t start_index, int32_t preamble_count, int32_t target_count)
/*                                                                          */
/*  Description:    Copies target data from scan buffer, filtering out      */
/*                  preamble and postamble data.   Note that buffer is an   */
/*                  array of char, while target_data is an array of int32_t */
/*                                                                          */
/*  Returns:        nothing                                                 */
/*                                                                          */
/****************************************************************************/
{
    int32_t i = 0L;
    int32_t j = 0L;
    int32_t k = 0L;

    j = preamble_count;
    k = start_index + target_count;
    for (i = start_index; i < k; ++i, ++j)
    {
        if (buffer[j >> 3] & (1 << (j & 7)))
        {
            target_data[i >> 5] |= (1L << (i & 0x1f));
        }
        else
        {
            target_data[i >> 5] &= ~(uint32_t) (1L << (i & 0x1f));
        }
    }
}

int
urj_jam_jtag_drscan (int start_state, int count, char *tdi, char *tdo)
{
    int status = 1;

    /*
     *      First go to DRSHIFT state
     */
    switch (start_state)
    {
    case 0:                    /* IDLE */
        urj_jam_jtag_io (1, 0, 0);  /* DRSELECT */
        urj_jam_jtag_io (0, 0, 0);  /* DRCAPTURE */
        urj_jam_jtag_io (0, 0, 0);  /* DRSHIFT */
        break;

    case 1:                    /* DRPAUSE */
        urj_jam_jtag_io (1, 0, 0);  /* DREXIT2 */
        urj_jam_jtag_io (0, 0, 0);  /* DRSHIFT */
        break;

    case 2:                    /* IRPAUSE */
        urj_jam_jtag_io (1, 0, 0);  /* IREXIT2 */
        urj_jam_jtag_io (1, 0, 0);  /* IRUPDATE */
        urj_jam_jtag_io (1, 0, 0);  /* DRSELECT */
        urj_jam_jtag_io (0, 0, 0);  /* DRCAPTURE */
        urj_jam_jtag_io (0, 0, 0);  /* DRSHIFT */
        break;

    default:
        status = 0;
    }

    if (status)
    {
        status = urj_jam_jtag_io_transfer (count, tdi, tdo);

        urj_jam_jtag_io (0, 0, 0);  /* DRPAUSE */
    }

    return status;
}

int
urj_jam_jtag_irscan (int start_state, int count, char *tdi, char *tdo)
{
    int status = 1;

    /*
     *      First go to IRSHIFT state
     */
    switch (start_state)
    {
    case 0:                    /* IDLE */
        urj_jam_jtag_io (1, 0, 0);  /* DRSELECT */
        urj_jam_jtag_io (1, 0, 0);  /* IRSELECT */
        urj_jam_jtag_io (0, 0, 0);  /* IRCAPTURE */
        urj_jam_jtag_io (0, 0, 0);  /* IRSHIFT */
        break;

    case 1:                    /* DRPAUSE */
        urj_jam_jtag_io (1, 0, 0);  /* DREXIT2 */
        urj_jam_jtag_io (1, 0, 0);  /* DRUPDATE */
        urj_jam_jtag_io (1, 0, 0);  /* DRSELECT */
        urj_jam_jtag_io (1, 0, 0);  /* IRSELECT */
        urj_jam_jtag_io (0, 0, 0);  /* IRCAPTURE */
        urj_jam_jtag_io (0, 0, 0);  /* IRSHIFT */
        break;

    case 2:                    /* IRPAUSE */
        urj_jam_jtag_io (1, 0, 0);  /* IREXIT2 */
        urj_jam_jtag_io (0, 0, 0);  /* IRSHIFT */
        break;

    default:
        status = 0;
    }

    if (status)
    {
        status = urj_jam_jtag_io_transfer (count, tdi, tdo);

        urj_jam_jtag_io (0, 0, 0);  /* IRPAUSE */
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_do_irscan
    (int32_t count, int32_t *data, int32_t start_index)
/*                                                                          */
/*  Description:    Shifts data into instruction register                   */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    int start_code = 0;
    int alloc_chars = 0;
    int shift_count = (int) (urj_jam_ir_preamble + count + urj_jam_ir_postamble);
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    JAME_JTAG_STATE start_state = JAM_ILLEGAL_JTAG_STATE;

    switch (urj_jam_jtag_state)
    {
    case JAM_ILLEGAL_JTAG_STATE:
    case RESET:
    case IDLE:
        start_code = 0;
        start_state = IDLE;
        break;

    case DRSELECT:
    case DRCAPTURE:
    case DRSHIFT:
    case DREXIT1:
    case DRPAUSE:
    case DREXIT2:
    case DRUPDATE:
        start_code = 1;
        start_state = DRPAUSE;
        break;

    case IRSELECT:
    case IRCAPTURE:
    case IRSHIFT:
    case IREXIT1:
    case IRPAUSE:
    case IREXIT2:
    case IRUPDATE:
        start_code = 2;
        start_state = IRPAUSE;
        break;

    default:
        status = JAMC_INTERNAL_ERROR;
        break;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_jtag_state != start_state)
        {
            status = urj_jam_goto_jtag_state (start_state);
        }
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_workspace != NULL)
        {
            if (shift_count > JAMC_MAX_JTAG_IR_LENGTH)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
        }
        else if (shift_count > urj_jam_ir_length)
        {
            alloc_chars = (shift_count + 7) >> 3;
            free (urj_jam_ir_buffer);
            urj_jam_ir_buffer = (char *) malloc (alloc_chars);

            if (urj_jam_ir_buffer == NULL)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_ir_length = alloc_chars * 8;
            }
        }
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Copy preamble data, IR data, and postamble data into a buffer
         */
        urj_jam_jtag_concatenate_data
            (urj_jam_ir_buffer,
             urj_jam_ir_preamble_data,
             urj_jam_ir_preamble,
             data,
             start_index, count, urj_jam_ir_postamble_data, urj_jam_ir_postamble);

        /*
         *      Do the IRSCAN
         */
        urj_jam_jtag_irscan (start_code, shift_count, urj_jam_ir_buffer, NULL);

        /* urj_jam_jtag_irscan() always ends in IRPAUSE state */
        urj_jam_jtag_state = IRPAUSE;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_irstop_state != IRPAUSE)
        {
            status = urj_jam_goto_jtag_state (urj_jam_irstop_state);
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_swap_ir
    (int32_t count,
     int32_t *in_data,
     int32_t in_index, int32_t *out_data, int32_t out_index)
/*                                                                          */
/*  Description:    Shifts data into instruction register, capturing output */
/*                  data                                                    */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    int start_code = 0;
    int alloc_chars = 0;
    int shift_count = (int) (urj_jam_ir_preamble + count + urj_jam_ir_postamble);
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    JAME_JTAG_STATE start_state = JAM_ILLEGAL_JTAG_STATE;

    switch (urj_jam_jtag_state)
    {
    case JAM_ILLEGAL_JTAG_STATE:
    case RESET:
    case IDLE:
        start_code = 0;
        start_state = IDLE;
        break;

    case DRSELECT:
    case DRCAPTURE:
    case DRSHIFT:
    case DREXIT1:
    case DRPAUSE:
    case DREXIT2:
    case DRUPDATE:
        start_code = 1;
        start_state = DRPAUSE;
        break;

    case IRSELECT:
    case IRCAPTURE:
    case IRSHIFT:
    case IREXIT1:
    case IRPAUSE:
    case IREXIT2:
    case IRUPDATE:
        start_code = 2;
        start_state = IRPAUSE;
        break;

    default:
        status = JAMC_INTERNAL_ERROR;
        break;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_jtag_state != start_state)
        {
            status = urj_jam_goto_jtag_state (start_state);
        }
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_workspace != NULL)
        {
            if (shift_count > JAMC_MAX_JTAG_IR_LENGTH)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
        }
        else if (shift_count > urj_jam_ir_length)
        {
            alloc_chars = (shift_count + 7) >> 3;
            free (urj_jam_ir_buffer);
            urj_jam_ir_buffer = (char *) malloc (alloc_chars);

            if (urj_jam_ir_buffer == NULL)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_ir_length = alloc_chars * 8;
            }
        }
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Copy preamble data, IR data, and postamble data into a buffer
         */
        urj_jam_jtag_concatenate_data
            (urj_jam_ir_buffer,
             urj_jam_ir_preamble_data,
             urj_jam_ir_preamble,
             in_data,
             in_index, count, urj_jam_ir_postamble_data, urj_jam_ir_postamble);

        /*
         *      Do the IRSCAN
         */
        urj_jam_jtag_irscan
            (start_code, shift_count, urj_jam_ir_buffer, urj_jam_ir_buffer);

        /* urj_jam_jtag_irscan() always ends in IRPAUSE state */
        urj_jam_jtag_state = IRPAUSE;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_irstop_state != IRPAUSE)
        {
            status = urj_jam_goto_jtag_state (urj_jam_irstop_state);
        }
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Now extract the returned data from the buffer
         */
        urj_jam_jtag_extract_target_data
            (urj_jam_ir_buffer, out_data, out_index, urj_jam_ir_preamble, count);
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_do_drscan
    (int32_t count, int32_t *data, int32_t start_index)
/*                                                                          */
/*  Description:    Shifts data into data register (ignoring output data)   */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    int start_code = 0;
    int alloc_chars = 0;
    int shift_count = (int) (urj_jam_dr_preamble + count + urj_jam_dr_postamble);
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    JAME_JTAG_STATE start_state = JAM_ILLEGAL_JTAG_STATE;

    switch (urj_jam_jtag_state)
    {
    case JAM_ILLEGAL_JTAG_STATE:
    case RESET:
    case IDLE:
        start_code = 0;
        start_state = IDLE;
        break;

    case DRSELECT:
    case DRCAPTURE:
    case DRSHIFT:
    case DREXIT1:
    case DRPAUSE:
    case DREXIT2:
    case DRUPDATE:
        start_code = 1;
        start_state = DRPAUSE;
        break;

    case IRSELECT:
    case IRCAPTURE:
    case IRSHIFT:
    case IREXIT1:
    case IRPAUSE:
    case IREXIT2:
    case IRUPDATE:
        start_code = 2;
        start_state = IRPAUSE;
        break;

    default:
        status = JAMC_INTERNAL_ERROR;
        break;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_jtag_state != start_state)
        {
            status = urj_jam_goto_jtag_state (start_state);
        }
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_workspace != NULL)
        {
            if (shift_count > JAMC_MAX_JTAG_DR_LENGTH)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
        }
        else if (shift_count > urj_jam_dr_length)
        {
            alloc_chars = (shift_count + 7) >> 3;
            free (urj_jam_dr_buffer);
            urj_jam_dr_buffer = (char *) malloc (alloc_chars);

            if (urj_jam_dr_buffer == NULL)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_dr_length = alloc_chars * 8;
            }
        }
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Copy preamble data, DR data, and postamble data into a buffer
         */
        urj_jam_jtag_concatenate_data
            (urj_jam_dr_buffer,
             urj_jam_dr_preamble_data,
             urj_jam_dr_preamble,
             data,
             start_index, count, urj_jam_dr_postamble_data, urj_jam_dr_postamble);

        /*
         *      Do the DRSCAN
         */
        urj_jam_jtag_drscan (start_code, shift_count, urj_jam_dr_buffer, NULL);

        /* urj_jam_jtag_drscan() always ends in DRPAUSE state */
        urj_jam_jtag_state = DRPAUSE;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_drstop_state != DRPAUSE)
        {
            status = urj_jam_goto_jtag_state (urj_jam_drstop_state);
        }
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

JAM_RETURN_TYPE urj_jam_swap_dr
    (int32_t count,
     int32_t *in_data,
     int32_t in_index, int32_t *out_data, int32_t out_index)
/*                                                                          */
/*  Description:    Shifts data into data register, capturing output data   */
/*                                                                          */
/*  Returns:        JAMC_SUCCESS for success, else appropriate error code   */
/*                                                                          */
/****************************************************************************/
{
    int start_code = 0;
    int alloc_chars = 0;
    int shift_count = (int) (urj_jam_dr_preamble + count + urj_jam_dr_postamble);
    JAM_RETURN_TYPE status = JAMC_SUCCESS;
    JAME_JTAG_STATE start_state = JAM_ILLEGAL_JTAG_STATE;

    switch (urj_jam_jtag_state)
    {
    case JAM_ILLEGAL_JTAG_STATE:
    case RESET:
    case IDLE:
        start_code = 0;
        start_state = IDLE;
        break;

    case DRSELECT:
    case DRCAPTURE:
    case DRSHIFT:
    case DREXIT1:
    case DRPAUSE:
    case DREXIT2:
    case DRUPDATE:
        start_code = 1;
        start_state = DRPAUSE;
        break;

    case IRSELECT:
    case IRCAPTURE:
    case IRSHIFT:
    case IREXIT1:
    case IRPAUSE:
    case IREXIT2:
    case IRUPDATE:
        start_code = 2;
        start_state = IRPAUSE;
        break;

    default:
        status = JAMC_INTERNAL_ERROR;
        break;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_jtag_state != start_state)
        {
            status = urj_jam_goto_jtag_state (start_state);
        }
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_workspace != NULL)
        {
            if (shift_count > JAMC_MAX_JTAG_DR_LENGTH)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
        }
        else if (shift_count > urj_jam_dr_length)
        {
            alloc_chars = (shift_count + 7) >> 3;
            free (urj_jam_dr_buffer);
            urj_jam_dr_buffer = (char *) malloc (alloc_chars);

            if (urj_jam_dr_buffer == NULL)
            {
                status = JAMC_OUT_OF_MEMORY;
            }
            else
            {
                urj_jam_dr_length = alloc_chars * 8;
            }
        }
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Copy preamble data, DR data, and postamble data into a buffer
         */
        urj_jam_jtag_concatenate_data
            (urj_jam_dr_buffer,
             urj_jam_dr_preamble_data,
             urj_jam_dr_preamble,
             in_data,
             in_index, count, urj_jam_dr_postamble_data, urj_jam_dr_postamble);

        /*
         *      Do the DRSCAN
         */
        urj_jam_jtag_drscan
            (start_code, shift_count, urj_jam_dr_buffer, urj_jam_dr_buffer);

        /* urj_jam_jtag_drscan() always ends in DRPAUSE state */
        urj_jam_jtag_state = DRPAUSE;
    }

    if (status == JAMC_SUCCESS)
    {
        if (urj_jam_drstop_state != DRPAUSE)
        {
            status = urj_jam_goto_jtag_state (urj_jam_drstop_state);
        }
    }

    if (status == JAMC_SUCCESS)
    {
        /*
         *      Now extract the returned data from the buffer
         */
        urj_jam_jtag_extract_target_data
            (urj_jam_dr_buffer, out_data, out_index, urj_jam_dr_preamble, count);
    }

    return status;
}

/****************************************************************************/
/*                                                                          */

void
urj_jam_free_jtag_padding_buffers (int reset_jtag)
/*                                                                          */
/*  Description:    Frees memory allocated for JTAG IR and DR buffers       */
/*                                                                          */
/*  Returns:        nothing                                                 */
/*                                                                          */
/****************************************************************************/
{
    /*
     *      If the JTAG interface was used, reset it to TLR
     */
    if (reset_jtag && (urj_jam_jtag_state != JAM_ILLEGAL_JTAG_STATE))
    {
        urj_jam_jtag_reset_idle ();
    }

    if (urj_jam_workspace == NULL)
    {
        if (urj_jam_dr_preamble_data != NULL)
        {
            free (urj_jam_dr_preamble_data);
            urj_jam_dr_preamble_data = NULL;
        }

        if (urj_jam_dr_postamble_data != NULL)
        {
            free (urj_jam_dr_postamble_data);
            urj_jam_dr_postamble_data = NULL;
        }

        if (urj_jam_dr_buffer != NULL)
        {
            free (urj_jam_dr_buffer);
            urj_jam_dr_buffer = NULL;
        }

        if (urj_jam_ir_preamble_data != NULL)
        {
            free (urj_jam_ir_preamble_data);
            urj_jam_ir_preamble_data = NULL;
        }

        if (urj_jam_ir_postamble_data != NULL)
        {
            free (urj_jam_ir_postamble_data);
            urj_jam_ir_postamble_data = NULL;
        }

        if (urj_jam_ir_buffer != NULL)
        {
            free (urj_jam_ir_buffer);
            urj_jam_ir_buffer = NULL;
        }
    }
}
