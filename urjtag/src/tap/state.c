/*
 * $Id$
 *
 * TAP state handling
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 */

#include <urjtag/tap_state.h>
#include <urjtag/chain.h>

static const char *
urj_tap_state_name (int state)
{
    switch (state)
    {
    case URJ_TAP_STATE_UNKNOWN_STATE:		return "UNKNOWN_STATE";
    case URJ_TAP_STATE_TEST_LOGIC_RESET:	return "TEST_LOGIC_RESET";
    case URJ_TAP_STATE_RUN_TEST_IDLE:		return "RUN_TEST_IDLE";
    case URJ_TAP_STATE_SELECT_DR_SCAN:		return "SELECT_DR_SCAN";
    case URJ_TAP_STATE_CAPTURE_DR:		return "CAPTURE_DR";
    case URJ_TAP_STATE_SHIFT_DR:		return "SHIFT_DR";
    case URJ_TAP_STATE_EXIT1_DR:		return "EXIT1_DR";
    case URJ_TAP_STATE_PAUSE_DR:		return "PAUSE_DR";
    case URJ_TAP_STATE_EXIT2_DR:		return "EXIT2_DR";
    case URJ_TAP_STATE_UPDATE_DR:		return "UPDATE_DR";
    case URJ_TAP_STATE_SELECT_IR_SCAN:		return "SELECT_IR_SCAN";
    case URJ_TAP_STATE_CAPTURE_IR:		return "CAPTURE_IR";
    case URJ_TAP_STATE_SHIFT_IR:		return "SHIFT_IR";
    case URJ_TAP_STATE_EXIT1_IR:		return "EXIT1_IR";
    case URJ_TAP_STATE_PAUSE_IR:		return "PAUSE_IR";
    case URJ_TAP_STATE_EXIT2_IR:		return "EXIT2_IR";
    case URJ_TAP_STATE_UPDATE_IR:		return "UPDATE_IR";
    default:					return "??????";
    }
}

static void
urj_tap_state_dump (int state)
{
    urj_log (URJ_LOG_LEVEL_DEBUG, "tap_state: %s\n",
             urj_tap_state_name (state));
}

static void
urj_tap_state_dump_2 (int state0, int state1, int tms)
{
    urj_log (URJ_LOG_LEVEL_DEBUG, "tap_state: %16s =(tms:%d)=> %s\n",
             urj_tap_state_name (state0), tms, urj_tap_state_name (state1));
}

int
urj_tap_state (urj_chain_t *chain)
{
    return chain->state;
}

int
urj_tap_state_init (urj_chain_t *chain)
{
    urj_tap_state_dump (URJ_TAP_STATE_UNKNOWN_STATE);
    return chain->state = URJ_TAP_STATE_UNKNOWN_STATE;
}

int
urj_tap_state_done (urj_chain_t *chain)
{
    urj_tap_state_dump (URJ_TAP_STATE_UNKNOWN_STATE);
    return chain->state = URJ_TAP_STATE_UNKNOWN_STATE;
}

int
urj_tap_state_reset (urj_chain_t *chain)
{
    urj_tap_state_dump (URJ_TAP_STATE_TEST_LOGIC_RESET);
    return chain->state = URJ_TAP_STATE_TEST_LOGIC_RESET;
}

int
urj_tap_state_set_trst (urj_chain_t *chain, int old_trst, int new_trst)
{
    old_trst = old_trst ? 1 : 0;
    new_trst = new_trst ? 1 : 0;

    if (old_trst != new_trst)
    {
        if (new_trst)
            chain->state = URJ_TAP_STATE_TEST_LOGIC_RESET;
        else
            chain->state = URJ_TAP_STATE_UNKNOWN_STATE;
    }

    urj_tap_state_dump (chain->state);
    return chain->state;
}

int
urj_tap_state_clock (urj_chain_t *chain, int tms)
{
    int oldstate = chain->state;

    if (tms)
    {
        switch (chain->state)
        {
        case URJ_TAP_STATE_TEST_LOGIC_RESET:
            break;
        case URJ_TAP_STATE_RUN_TEST_IDLE:
        case URJ_TAP_STATE_UPDATE_DR:
        case URJ_TAP_STATE_UPDATE_IR:
            chain->state = URJ_TAP_STATE_SELECT_DR_SCAN;
            break;
        case URJ_TAP_STATE_SELECT_DR_SCAN:
            chain->state = URJ_TAP_STATE_SELECT_IR_SCAN;
            break;
        case URJ_TAP_STATE_CAPTURE_DR:
        case URJ_TAP_STATE_SHIFT_DR:
            chain->state = URJ_TAP_STATE_EXIT1_DR;
            break;
        case URJ_TAP_STATE_EXIT1_DR:
        case URJ_TAP_STATE_EXIT2_DR:
            chain->state = URJ_TAP_STATE_UPDATE_DR;
            break;
        case URJ_TAP_STATE_PAUSE_DR:
            chain->state = URJ_TAP_STATE_EXIT2_DR;
            break;
        case URJ_TAP_STATE_SELECT_IR_SCAN:
            chain->state = URJ_TAP_STATE_TEST_LOGIC_RESET;
            break;
        case URJ_TAP_STATE_CAPTURE_IR:
        case URJ_TAP_STATE_SHIFT_IR:
            chain->state = URJ_TAP_STATE_EXIT1_IR;
            break;
        case URJ_TAP_STATE_EXIT1_IR:
        case URJ_TAP_STATE_EXIT2_IR:
            chain->state = URJ_TAP_STATE_UPDATE_IR;
            break;
        case URJ_TAP_STATE_PAUSE_IR:
            chain->state = URJ_TAP_STATE_EXIT2_IR;
            break;
        default:
            chain->state = URJ_TAP_STATE_UNKNOWN_STATE;
            break;
        }
    }
    else
    {
        switch (chain->state)
        {
        case URJ_TAP_STATE_TEST_LOGIC_RESET:
        case URJ_TAP_STATE_RUN_TEST_IDLE:
        case URJ_TAP_STATE_UPDATE_DR:
        case URJ_TAP_STATE_UPDATE_IR:
            chain->state = URJ_TAP_STATE_RUN_TEST_IDLE;
            break;
        case URJ_TAP_STATE_SELECT_DR_SCAN:
            chain->state = URJ_TAP_STATE_CAPTURE_DR;
            break;
        case URJ_TAP_STATE_CAPTURE_DR:
        case URJ_TAP_STATE_SHIFT_DR:
        case URJ_TAP_STATE_EXIT2_DR:
            chain->state = URJ_TAP_STATE_SHIFT_DR;
            break;
        case URJ_TAP_STATE_EXIT1_DR:
        case URJ_TAP_STATE_PAUSE_DR:
            chain->state = URJ_TAP_STATE_PAUSE_DR;
            break;
        case URJ_TAP_STATE_SELECT_IR_SCAN:
            chain->state = URJ_TAP_STATE_CAPTURE_IR;
            break;
        case URJ_TAP_STATE_CAPTURE_IR:
        case URJ_TAP_STATE_SHIFT_IR:
        case URJ_TAP_STATE_EXIT2_IR:
            chain->state = URJ_TAP_STATE_SHIFT_IR;
            break;
        case URJ_TAP_STATE_EXIT1_IR:
        case URJ_TAP_STATE_PAUSE_IR:
            chain->state = URJ_TAP_STATE_PAUSE_IR;
            break;
        default:
            chain->state = URJ_TAP_STATE_UNKNOWN_STATE;
            break;
        }
    }

    urj_tap_state_dump_2 (oldstate, chain->state, tms);
    return chain->state;
}
