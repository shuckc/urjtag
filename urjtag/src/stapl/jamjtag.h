/****************************************************************************/
/*                                                                          */
/*  Module:         jamjtag.h                                               */
/*                                                                          */
/*                  Copyright (C) Altera Corporation 1997                   */
/*                                                                          */
/*  Description:    Definitions of JTAG constants, types, and functions     */
/*                                                                          */
/****************************************************************************/

#ifndef INC_JAMJTAG_H
#define INC_JAMJTAG_H

#include <stdint.h>

/****************************************************************************/
/*                                                                          */
/*  Constant definitions                                                    */
/*                                                                          */
/****************************************************************************/

#define JAMC_MAX_JTAG_STATE_LENGTH 9

/****************************************************************************/
/*                                                                          */
/*  Enumerated Types                                                        */
/*                                                                          */
/****************************************************************************/

typedef enum
{
    JAM_ILLEGAL_JTAG_STATE = -1,
    RESET = 0,
    IDLE = 1,
    DRSELECT = 2,
    DRCAPTURE = 3,
    DRSHIFT = 4,
    DREXIT1 = 5,
    DRPAUSE = 6,
    DREXIT2 = 7,
    DRUPDATE = 8,
    IRSELECT = 9,
    IRCAPTURE = 10,
    IRSHIFT = 11,
    IREXIT1 = 12,
    IRPAUSE = 13,
    IREXIT2 = 14,
    IRUPDATE = 15
} JAME_JTAG_STATE;

extern int urj_jam_jtag_io_transfer (int count, char *tdi, char *tdo);
extern void urj_jam_flush_and_delay (int32_t microseconds);

/****************************************************************************/
/*                                                                          */
/*  Function Prototypes                                                     */
/*                                                                          */
/****************************************************************************/

JAM_RETURN_TYPE urj_jam_init_jtag (void);

JAME_JTAG_STATE urj_jam_get_jtag_state_from_name (char *name);

JAM_RETURN_TYPE urj_jam_set_drstop_state (JAME_JTAG_STATE state);

JAM_RETURN_TYPE urj_jam_set_irstop_state (JAME_JTAG_STATE state);

JAM_RETURN_TYPE urj_jam_set_dr_preamble
    (int count, int start_index, int32_t *data);

JAM_RETURN_TYPE urj_jam_set_ir_preamble
    (int count, int start_index, int32_t *data);

JAM_RETURN_TYPE urj_jam_set_dr_postamble
    (int count, int start_index, int32_t *data);

JAM_RETURN_TYPE urj_jam_set_ir_postamble
    (int count, int start_index, int32_t *data);

JAM_RETURN_TYPE urj_jam_goto_jtag_state (JAME_JTAG_STATE state);

JAM_RETURN_TYPE urj_jam_do_wait_cycles
    (int32_t cycles, JAME_JTAG_STATE wait_state);

JAM_RETURN_TYPE urj_jam_do_wait_microseconds
    (int32_t microseconds, JAME_JTAG_STATE wait_state);

JAM_RETURN_TYPE urj_jam_do_irscan
    (int32_t count, int32_t *data, int32_t start_index);

JAM_RETURN_TYPE urj_jam_swap_ir
    (int32_t count,
     int32_t *in_data,
     int32_t in_index, int32_t *out_data, int32_t out_index);

JAM_RETURN_TYPE urj_jam_do_drscan
    (int32_t count, int32_t *data, int32_t start_index);

JAM_RETURN_TYPE urj_jam_swap_dr
    (int32_t count,
     int32_t *in_data,
     int32_t in_index, int32_t *out_data, int32_t out_index);

void urj_jam_free_jtag_padding_buffers (int reset_jtag);

#endif /* INC_JAMJTAG_H */
