/*
 * $Id$
 *
 * Generic cable driver for FTDI's FT2232C chip in MPSSE mode.
 * Copyright (C) 2007 A. Laeuger
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
 * Written by Arnim Laeuger, 2007-2008.
 * Support for JTAGkey submitted by Laurent Gauch, 2008.
 * Support for FT2232H written by Michael Hennerich, 2009; adapted
 *      for urjtag codebase and submitted by Adam Megacz, 2010.
 *
 */

#include <sysdep.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <urjtag/cable.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>

#include "generic.h"
#include "generic_usbconn.h"

#include <urjtag/usbconn.h>
#include "usbconn/libftdx.h"

#include "cmd_xfer.h"

/* Maximum TCK frequency of FT2232 */
#define FT2232_MAX_TCK_FREQ 6000000

/* Maximum TCK frequency of FT2232H / FT4232H  */
#define FT2232H_MAX_TCK_FREQ 30000000

/* The default driver if not specified otherwise during connect */
#ifdef ENABLE_LOWLEVEL_FTD2XX
#define DEFAULT_DRIVER "ftd2xx-mpsse"
#else
#define DEFAULT_DRIVER "ftdi-mpsse"
#endif

/* repeat the definitions for MPSSE command processor here
   since we cannot rely on the existence of ftdi.h even though
   they're defined there */

/* Shifting commands IN MPSSE Mode*/
#define MPSSE_WRITE_NEG 0x01    /* Write TDI/DO on negative TCK/SK edge */
#define MPSSE_BITMODE   0x02    /* Write bits, not bytes */
#define MPSSE_READ_NEG  0x04    /* Sample TDO/DI on negative TCK/SK edge */
#define MPSSE_LSB       0x08    /* LSB first */
#define MPSSE_DO_WRITE  0x10    /* Write TDI/DO */
#define MPSSE_DO_READ   0x20    /* Read TDO/DI */
#define MPSSE_WRITE_TMS 0x40    /* Write TMS/CS */

/* FTDI MPSSE commands */
#define SET_BITS_LOW    0x80
/*BYTE DATA*/
/*BYTE Direction*/
#define SET_BITS_HIGH   0x82
/*BYTE DATA*/
/*BYTE Direction*/
#define GET_BITS_LOW    0x81
#define GET_BITS_HIGH   0x83
#define LOOPBACK_START  0x84
#define LOOPBACK_END    0x85
#define TCK_DIVISOR     0x86
#define SEND_IMMEDIATE  0x87

/* FT2232H / FT4232H only commands */
#define DISABLE_CLOCKDIV  0x8A /* Disables the clk divide by 5 to allow for a 60MHz master clock */
#define ENABLE_CLOCKDIV   0x8B /* Enables the clk divide by 5 to allow for backward compatibility with FT2232D */

/* bit and bitmask definitions for GPIO commands */
#define BIT_TCK         0
#define BIT_TDI         1
#define BIT_TDO         2
#define BIT_TMS         3
#define BITMASK_TDO     (1 << BIT_TDO)
#define BITMASK_TDI     (1 << BIT_TDI)
#define BITMASK_TCK     (1 << BIT_TCK)
#define BITMASK_TMS     (1 << BIT_TMS)

/* bit and bitmask definitions for Amontec JTAGkey */
#define BIT_JTAGKEY_nOE         4
#define BIT_JTAGKEY_TRST_N_OUT  0
#define BIT_JTAGKEY_SRST_N_OUT  1
#define BIT_JTAGKEY_TRST_N_OE_N 2
#define BIT_JTAGKEY_SRST_N_OE_N 3
#define BITMASK_JTAGKEY_nOE     (1 << BIT_JTAGKEY_nOE)
#define BITMASK_JTAGKEY_TRST_N_OUT (1 << BIT_JTAGKEY_TRST_N_OUT)
#define BITMASK_JTAGKEY_SRST_N_OUT (1 << BIT_JTAGKEY_SRST_N_OUT)
#define BITMASK_JTAGKEY_TRST_N_OE_N (1 << BIT_JTAGKEY_TRST_N_OE_N)
#define BITMASK_JTAGKEY_SRST_N_OE_N (1 << BIT_JTAGKEY_SRST_N_OE_N)

/* bit and bitmask definitions for Olimex ARM-USB-OCD */
#define BIT_ARMUSBOCD_nOE       4
#define BIT_ARMUSBOCD_nTRST     0
#define BIT_ARMUSBOCD_nTSRST    1
#define BIT_ARMUSBOCD_nTRST_nOE 2
#define BIT_ARMUSBOCD_RED_LED   3
#define BITMASK_ARMUSBOCD_nOE   (1 << BIT_ARMUSBOCD_nOE)
#define BITMASK_ARMUSBOCD_nTRST (1 << BIT_ARMUSBOCD_nTRST)
#define BITMASK_ARMUSBOCD_nTSRST (1 << BIT_ARMUSBOCD_nTSRST)
#define BITMASK_ARMUSBOCD_nTRST_nOE (1 << BIT_ARMUSBOCD_nTRST_nOE)
#define BITMASK_ARMUSBOCD_RED_LED (1 << BIT_ARMUSBOCD_RED_LED)

/* bit and bitmask definitions for Blackfin gnICE */
#define BIT_GNICE_nTRST         1
#define BIT_GNICE_nLED          3
#define BITMASK_GNICE_nTRST     (1 << BIT_GNICE_nTRST)
#define BITMASK_GNICE_nLED      (1 << BIT_GNICE_nLED)

/* bit and bitmask definitions for OOCDLink-s */
#define BIT_OOCDLINKS_nTRST_nOE 0
#define BIT_OOCDLINKS_nTRST     1
#define BIT_OOCDLINKS_nSRST_nOE 2
#define BIT_OOCDLINKS_nSRST     3
#define BITMASK_OOCDLINKS_nTRST_nOE (1 << BIT_OOCDLINKS_nTRST_nOE)
#define BITMASK_OOCDLINKS_nTRST (1 << BIT_OOCDLINKS_nTRST)
#define BITMASK_OOCDLINKS_nSRST_nOE (1 << BIT_OOCDLINKS_nSRST_nOE)
#define BITMASK_OOCDLINKS_nSRST (1 << BIT_OOCDLINKS_nSRST)

/* bit and bitmask definitions for Turtelizer 2 */
#define BIT_TURTELIZER2_nJTAGOE 4
#define BIT_TURTELIZER2_RST     6
#define BIT_TURTELIZER2_nTX1LED 2
#define BIT_TURTELIZER2_nRX1LED 3
#define BITMASK_TURTELIZER2_nJTAGOE (1 << BIT_TURTELIZER2_nJTAGOE)
#define BITMASK_TURTELIZER2_RST (1 << BIT_TURTELIZER2_RST)
#define BITMASK_TURTELIZER2_nTX1LED (1 << BIT_TURTELIZER2_nTX1LED)
#define BITMASK_TURTELIZER2_nRX1LED (1 << BIT_TURTELIZER2_nRX1LED)

/* bit and bitmask definitions for USB<=>JTAG&RS232 */
#define BIT_USBJTAGRS232_nTRST_nOE 2
#define BIT_USBJTAGRS232_nTRST     0
#define BIT_USBJTAGRS232_nSRST_nOE 3
#define BIT_USBJTAGRS232_nSRST     1
#define BITMASK_USBJTAGRS232_nTRST_nOE (1 << BIT_USBJTAGRS232_nTRST_nOE)
#define BITMASK_USBJTAGRS232_nTRST (1 << BIT_USBJTAGRS232_nTRST)
#define BITMASK_USBJTAGRS232_nSRST_nOE (1 << BIT_USBJTAGRS232_nSRST_nOE)
#define BITMASK_USBJTAGRS232_nSRST (1 << BIT_USBJTAGRS232_nSRST)

/* bit and bitmask definitions for USB to JTAG Interface */
#define BIT_USBTOJTAGIF_nTRST   4
#define BIT_USBTOJTAGIF_RST     6
#define BIT_USBTOJTAGIF_DBGRQ   7
#define BIT_USBTOJTAGIF_nRxLED  2
#define BIT_USBTOJTAGIF_nTxLED  3
#define BITMASK_USBTOJTAGIF_nTRST (1 << BIT_USBTOJTAGIF_nTRST)
#define BITMASK_USBTOJTAGIF_RST (1 << BIT_USBTOJTAGIF_RST)
#define BITMASK_USBTOJTAGIF_DBGRQ (1 << BIT_USBTOJTAGIF_DBGRQ)
#define BITMASK_USBTOJTAGIF_nRxLED (1 << BIT_USBTOJTAGIF_nRxLED)
#define BITMASK_USBTOJTAGIF_nTxLED (1 << BIT_USBTOJTAGIF_nTxLED)

/* bit and bitmask definitions for Xverve DT-USB-ST Signalyzer Tool */
#define BIT_SIGNALYZER_nTRST    4
#define BIT_SIGNALYZER_nSRST    5
#define BITMASK_SIGNALYZER_nTRST (1 << BIT_SIGNALYZER_nTRST)
#define BITMASK_SIGNALYZER_nSRST (1 << BIT_SIGNALYZER_nSRST)

/* bit and bitmask definitions for TinCanTools Flyswatter board*/
#define BIT_FLYSWATTER_nLED2    3
#define BIT_FLYSWATTER_nTRST    4
#define BIT_FLYSWATTER_nSRST    5
#define BIT_FLYSWATTER_nOE1     6
#define BIT_FLYSWATTER_nOE2     7
#define BITMASK_FLYSWATTER_nLED2 (1 << BIT_FLYSWATTER_nLED2)
#define BITMASK_FLYSWATTER_nTRST (1 << BIT_FLYSWATTER_nTRST)
#define BITMASK_FLYSWATTER_nSRST (1 << BIT_FLYSWATTER_nSRST)
#define BITMASK_FLYSWATTER_nOE1  (1 << BIT_FLYSWATTER_nOE1)
#define BITMASK_FLYSWATTER_nOE2  (1 << BIT_FLYSWATTER_nOE2)

/* --- Bit and bitmask definitions for usbScarab2 --- */
/* usbScarabeus2 is a design of Krzysztof Kajstura ( http://www.kristech.eu ). */
/* UrJTAG support added by Tomek Cedro ( http://www.tomek.cedro.info ) */
/* as a part of work for TP R&D (Polish Telecom, FT/Orange Group) http://www.tp.pl */
#define BIT_USBSCARAB2_nCONNECTED 5     // ADBUS
#define BIT_USBSCARAB2_TRST     0       // ACBUS
#define BIT_USBSCARAB2_nSRST    1       // ACBUS
#define BIT_USBSCARAB2_LED      3       // ACBUS
#define BITMASK_USBSCARAB2_LED  (1 << BIT_USBSCARAB2_LED)
#define BITMASK_USBSCARAB2_TRST (1 << BIT_USBSCARAB2_TRST)
#define BITMASK_USBSCARAB2_nSRST (1 << BIT_USBSCARAB2_nSRST)
#define BITMASK_USBSCARAB2_nCONNECTED (1 << BIT_USBSCARAB2_nCONNECTED)

/* bit and bitmask definitions for Milkymist JTAG/serial daughterboard */
#define BIT_MILKYMIST_VREF 4
#define BITMASK_MILKYMIST_VREF (1 << BIT_MILKYMIST_VREF)


typedef struct
{
    uint32_t mpsse_frequency;

    /* this driver issues several "Set Data Bits Low Byte" commands
       here is the place where cable specific values can be stored
       that are used each time this command is issued */
    uint8_t low_byte_value;
    uint8_t low_byte_dir;

    /* this driver issues several "Set Data Bits High Byte" commands
       here is the place where cable specific values can be stored
       that are used each time this command is issued */
    uint8_t high_byte_value;
    uint8_t high_byte_dir;

    /* the following variables store the bit position of TRST and RESET (SRST)
       for XOR'ing with the default values of low_byte_value and high_byte_value
       allowed values:
       <  0 : feature not used
       <  8 : applies to low byte
       < 12 : applies to high byte */
    int bit_trst;
    int bit_reset;

    /* variables to save last TDO value
       this acts as a cache to prevent multiple "Read Data Bits Low" transfer
       over USB for ft2232_get_tdo */
    unsigned int last_tdo_valid;
    unsigned int last_tdo;
    int signals;

    urj_tap_cable_cx_cmd_root_t cmd_root;
} params_t;


static const uint8_t imm_buf[1] = { SEND_IMMEDIATE };
static const urj_tap_cable_cx_cmd_t imm_cmd =
    { NULL, 1, 1, (uint8_t *) imm_buf, 0 };


static void
ft2232h_disable_clockdiv_by5 (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    urj_tap_cable_cx_cmd_queue( cmd_root, 0 );
    urj_tap_cable_cx_cmd_push( cmd_root, DISABLE_CLOCKDIV );
}

static void
ft2232_set_frequency_common (urj_cable_t *cable, uint32_t new_frequency, uint32_t max_frequency)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (!new_frequency || new_frequency > max_frequency)
        new_frequency = max_frequency;

    /* update ft2232 frequency if cable setting changed */
    if (new_frequency != params->mpsse_frequency)
    {
        uint32_t div;

        div = max_frequency / new_frequency;
        if (max_frequency % new_frequency)
            div++;

        if (div >= (1 << 16))
        {
            div = (1 << 16) - 1;
            urj_warning (_("Warning: Setting lowest supported frequency for FT2232%s: %d\n"),
                         max_frequency == FT2232H_MAX_TCK_FREQ ? "H" : "", max_frequency/div);
        }

        if (max_frequency == FT2232H_MAX_TCK_FREQ)
            ft2232h_disable_clockdiv_by5 (cable);

        /* send new divisor to device */
        div -= 1;
        urj_tap_cable_cx_cmd_queue (cmd_root, 0);
        urj_tap_cable_cx_cmd_push (cmd_root, TCK_DIVISOR);
        urj_tap_cable_cx_cmd_push (cmd_root, div & 0xff);
        urj_tap_cable_cx_cmd_push (cmd_root, (div >> 8) & 0xff);

        urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                               URJ_TAP_CABLE_COMPLETELY);

        params->mpsse_frequency = max_frequency / (div + 1);
        cable->frequency = params->mpsse_frequency;
    }
}


static void
ft2232_set_frequency (urj_cable_t *cable, uint32_t new_frequency)
{
    ft2232_set_frequency_common (cable, new_frequency, FT2232_MAX_TCK_FREQ);
}

static void
ft2232h_set_frequency (urj_cable_t *cable, uint32_t new_frequency)
{
    ft2232_set_frequency_common (cable, new_frequency, FT2232H_MAX_TCK_FREQ);
}

static int
ft2232_generic_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* safe default values */
    params->low_byte_value = 0;
    params->low_byte_dir = 0;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte */
    params->high_byte_value = 0;
    params->high_byte_value = 0;
    params->high_byte_dir = 0;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = -1;      /* not used */
    params->bit_reset = -1;     /* not used */

    params->last_tdo_valid = 0;
    params->signals = 0;

    return URJ_STATUS_OK;
}

static int
ft2232_jtagkey_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction:
       set nOE to '0' -> activate output enables */
    params->low_byte_value = 0;
    params->low_byte_dir = BITMASK_JTAGKEY_nOE;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0, nOE = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte
       default:
       TRST_N_OUT = 1
       TRST_N_OE_N = 0
       SRST_N_OUT = 1
       SRST_N_OE_N = 0 */
    params->high_byte_value =
        BITMASK_JTAGKEY_TRST_N_OUT | BITMASK_JTAGKEY_SRST_N_OUT;
    params->high_byte_dir =
        BITMASK_JTAGKEY_TRST_N_OUT | BITMASK_JTAGKEY_TRST_N_OE_N |
        BITMASK_JTAGKEY_SRST_N_OUT | BITMASK_JTAGKEY_SRST_N_OE_N;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_JTAGKEY_TRST_N_OUT + 8;      /* member of HIGH byte */
    params->bit_reset = BIT_JTAGKEY_SRST_N_OUT + 8;     /* member of HIGH byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}


static int
ft2232_armusbocd_init_common (urj_cable_t *cable, int is_ft2232h)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction:
       set nOE to '0' -> activate output enables */
    params->low_byte_value = 0;
    params->low_byte_dir = BITMASK_ARMUSBOCD_nOE;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0, nOE = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte
       default:
       TRST = 1
       TRST buffer enable = 0
       TSRST = 1
       RED LED on */
    params->high_byte_value = BITMASK_ARMUSBOCD_nTRST
        | BITMASK_ARMUSBOCD_nTSRST | BITMASK_ARMUSBOCD_RED_LED;
    params->high_byte_dir = BITMASK_ARMUSBOCD_nTRST
        | BITMASK_ARMUSBOCD_nTRST_nOE
        | BITMASK_ARMUSBOCD_nTSRST | BITMASK_ARMUSBOCD_RED_LED;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    if (is_ft2232h)
        ft2232h_set_frequency (cable, FT2232H_MAX_TCK_FREQ);
    else
        ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_ARMUSBOCD_nTRST + 8;         /* member of HIGH byte */
    params->bit_reset = BIT_ARMUSBOCD_nTSRST + 8;       /* member of HIGH byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}

static int
ft2232_armusbocd_init (urj_cable_t *cable)
{
    return ft2232_armusbocd_init_common (cable, 0);
}

static int
ft2232_armusbtiny_h_init (urj_cable_t *cable)
{
    return ft2232_armusbocd_init_common (cable, 1);
}


static int
ft2232_gnice_init_common (urj_cable_t *cable, int is_ft2232h)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* safe default values */
    params->low_byte_value = 0;
    params->low_byte_dir = 0;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte */
    params->high_byte_value = BITMASK_GNICE_nTRST;
    params->high_byte_dir = BITMASK_GNICE_nTRST | BITMASK_GNICE_nLED;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    if (is_ft2232h)
        /* On ADI boards with the onboard EZKIT Debug Agent, max TCK where things
           work is 15MHz. */
        ft2232h_set_frequency (cable, FT2232H_MAX_TCK_FREQ / 2);
    else
        ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_GNICE_nTRST + 8;     /* member of HIGH byte */
    params->bit_reset = -1;     /* not used */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST;

    return URJ_STATUS_OK;
}

static int
ft2232_gnice_init (urj_cable_t *cable)
{
    return ft2232_gnice_init_common (cable, 0);
}

static int
ft2232_gniceplus_init (urj_cable_t *cable)
{
    return ft2232_gnice_init_common (cable, 1);
}

static int
ft2232_oocdlinks_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction */
    params->low_byte_value = 0;
    params->low_byte_dir = 0;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte
       default:
       TRST = 1
       TRST buffer enable = 0
       SRST = 1
       SRST buffer enable = 0 */
    params->high_byte_value = BITMASK_OOCDLINKS_nTRST
        | BITMASK_OOCDLINKS_nSRST;
    params->high_byte_dir = BITMASK_OOCDLINKS_nTRST
        | BITMASK_OOCDLINKS_nTRST_nOE
        | BITMASK_OOCDLINKS_nSRST | BITMASK_OOCDLINKS_nSRST_nOE;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_OOCDLINKS_nTRST + 8; /* member of HIGH byte */
    params->bit_reset = BIT_OOCDLINKS_nSRST + 8;        /* member of HIGH byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}


static int
ft2232_turtelizer2_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction:
       set nJTAGOE to '0' -> activate output enables
       set RST to 0 -> inactive nSRST */
    params->low_byte_value = 0;
    params->low_byte_dir =
        BITMASK_TURTELIZER2_nJTAGOE | BITMASK_TURTELIZER2_RST;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte
       default:
       TX1LED on
       RX1LED on */
    params->high_byte_value = 0;
    params->high_byte_dir =
        BITMASK_TURTELIZER2_nTX1LED | BITMASK_TURTELIZER2_nRX1LED;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = -1;      /* not used */
    params->bit_reset = BIT_TURTELIZER2_RST;    /* member of LOW byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}


static int
ft2232_usbjtagrs232_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction */
    params->low_byte_value = 0;
    params->low_byte_dir = 0;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte
       default:
       TRST = 1
       TRST buffer enable = 0
       SRST = 1
       SRST buffer enable = 0 */
    params->high_byte_value = BITMASK_USBJTAGRS232_nTRST
        | BITMASK_USBJTAGRS232_nSRST;
    params->high_byte_dir = BITMASK_USBJTAGRS232_nTRST
        | BITMASK_USBJTAGRS232_nTRST_nOE
        | BITMASK_USBJTAGRS232_nSRST | BITMASK_USBJTAGRS232_nSRST_nOE;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_USBJTAGRS232_nTRST + 8; /* member of HIGH byte */
    params->bit_reset = BIT_USBJTAGRS232_nSRST + 8;        /* member of HIGH byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}


static int
ft2232_usbtojtagif_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction:
       nTRST = 1, RST = 1, DBGRQ = 0 */
    params->low_byte_value =
        BITMASK_USBTOJTAGIF_nTRST | BITMASK_USBTOJTAGIF_RST;
    params->low_byte_dir =
        BITMASK_USBTOJTAGIF_nTRST | BITMASK_USBTOJTAGIF_RST |
        BITMASK_USBTOJTAGIF_DBGRQ;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte
       default:
       RxLED on
       TxLED on */
    params->high_byte_value = 0;
    params->high_byte_value = 0;
    params->high_byte_dir =
        BITMASK_USBTOJTAGIF_nRxLED | BITMASK_USBTOJTAGIF_nTxLED;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    /* I-couplers can only work up to 3 MHz
       ref. http://www.hs-augsburg.de/~hhoegl/proj/usbjtag/usbjtag.html */
    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ / 2);

    params->bit_trst = BIT_USBTOJTAGIF_nTRST;   /* member of LOW byte */
    params->bit_reset = BIT_USBTOJTAGIF_RST;    /* member of LOW byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}


static int
ft2232_signalyzer_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction:
       nTRST = 1, nSRST = 1 */
    params->low_byte_value =
        BITMASK_SIGNALYZER_nTRST | BITMASK_SIGNALYZER_nSRST;
    params->low_byte_dir =
        BITMASK_SIGNALYZER_nTRST | BITMASK_SIGNALYZER_nSRST;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte */
    params->high_byte_value = 0;
    params->high_byte_dir = 0;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_SIGNALYZER_nTRST;    /* member of LOW byte */
    params->bit_reset = BIT_SIGNALYZER_nSRST;   /* member of LOW byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    return URJ_STATUS_OK;
}


static int
ft2232_flyswatter_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* static low byte value and direction:
       nTRST = 1, nSRST = 1 (ADBUS5 inverted),
       set nOE1 and nOE2 to '0' -> activate output enables */
    params->low_byte_value = BITMASK_FLYSWATTER_nTRST;
    params->low_byte_dir = BITMASK_FLYSWATTER_nOE1 | BITMASK_FLYSWATTER_nOE2 |
        BITMASK_FLYSWATTER_nTRST | BITMASK_FLYSWATTER_nSRST;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte */
    /* Turn LED2 on */
    params->high_byte_value = 0;
    params->high_byte_dir = BITMASK_FLYSWATTER_nLED2;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_FLYSWATTER_nTRST;    /* member of LOW byte */
    params->bit_reset = BIT_FLYSWATTER_nSRST;   /* member of LOW byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;


    return URJ_STATUS_OK;
}

static int
ft2232_usbscarab2_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* Check if cable is connected to the target and the target is powered on */
    urj_tap_cable_cx_cmd_queue (cmd_root, 1);
    urj_tap_cable_cx_cmd_push (cmd_root, GET_BITS_LOW);
    urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    if ((urj_tap_cable_cx_xfer_recv (cable) & BITMASK_USBSCARAB2_nCONNECTED)
        != 0)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("Please power on the TARGET board and connect VCC signal"));
        return URJ_STATUS_FAIL;
    }

    /* These bits will be set by default to: */
    params->low_byte_value = 0;
    params->low_byte_dir = 0;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               params->low_byte_dir | BITMASK_TCK
                               | BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte */
    /* nLED=0 */
    params->high_byte_value = 0 | BITMASK_USBSCARAB2_TRST;
    params->high_byte_dir =
        0 | BITMASK_USBSCARAB2_LED | BITMASK_USBSCARAB2_TRST |
        BITMASK_USBSCARAB2_nSRST;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232_set_frequency (cable, FT2232_MAX_TCK_FREQ);

    params->bit_trst = BIT_USBSCARAB2_TRST + 8; /* member of HIGH byte */
    params->bit_reset = BIT_USBSCARAB2_nSRST + 8;       /* member of HIGH byte */

    params->last_tdo_valid = 0;
    params->signals = URJ_POD_CS_TRST | URJ_POD_CS_RESET;

    urj_log (URJ_LOG_LEVEL_NORMAL, "Cable initialization OK!\n");
    return URJ_STATUS_OK;
}

static int
ft2232_milkymist_init (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    if (urj_tap_usbconn_open (cable->link.usb) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    /* Check if cable is connected to the target and the target is powered on */
    urj_tap_cable_cx_cmd_queue (cmd_root, 1);
    urj_tap_cable_cx_cmd_push (cmd_root, GET_BITS_LOW);
    urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    if ((urj_tap_cable_cx_xfer_recv (cable) & BITMASK_MILKYMIST_VREF) == 0)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE,
                       _("Vref not detected. Please power on Milkymist One"));
        return URJ_STATUS_FAIL;
    }

    params->low_byte_value = 0;
    params->low_byte_dir = 0;

    /* Set Data Bits Low Byte
       TCK = 0, TMS = 1, TDI = 0 */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, params->low_byte_value | BITMASK_TMS);
    urj_tap_cable_cx_cmd_push (cmd_root, params->low_byte_dir | BITMASK_TCK |
                               BITMASK_TDI | BITMASK_TMS);

    /* Set Data Bits High Byte */
    params->high_byte_value = 0;
    params->high_byte_value = 0;
    params->high_byte_dir = 0;
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_value);
    urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);

    ft2232h_set_frequency (cable, FT2232H_MAX_TCK_FREQ);

    params->bit_trst = -1;      /* not used */
    params->bit_reset = -1;     /* not used */

    params->last_tdo_valid = 0;
    params->signals = 0;

    return URJ_STATUS_OK;
}

static void
ft2232_generic_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_jtagkey_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_JTAGKEY_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_JTAGKEY_nOE);

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_JTAGKEY_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_JTAGKEY_TRST_N_OUT
                               | BITMASK_JTAGKEY_TRST_N_OE_N
                               | BITMASK_JTAGKEY_SRST_N_OUT |
                               BITMASK_JTAGKEY_SRST_N_OE_N);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_JTAGKEY_TRST_N_OUT |
                               BITMASK_JTAGKEY_TRST_N_OE_N |
                               BITMASK_JTAGKEY_SRST_N_OUT |
                               BITMASK_JTAGKEY_SRST_N_OE_N);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_JTAGKEY_TRST_N_OUT
                               | BITMASK_JTAGKEY_TRST_N_OE_N
                               | BITMASK_JTAGKEY_SRST_N_OUT |
                               BITMASK_JTAGKEY_SRST_N_OE_N);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_armusbocd_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_ARMUSBOCD_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_ARMUSBOCD_nOE);

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_ARMUSBOCD_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_ARMUSBOCD_nTRST
                               | BITMASK_ARMUSBOCD_nTRST_nOE |
                               BITMASK_ARMUSBOCD_nTSRST);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_ARMUSBOCD_nTRST |
                               BITMASK_ARMUSBOCD_nTRST_nOE |
                               BITMASK_ARMUSBOCD_nTSRST |
                               BITMASK_ARMUSBOCD_RED_LED);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_ARMUSBOCD_nTRST
                               | BITMASK_ARMUSBOCD_nTRST_nOE |
                               BITMASK_ARMUSBOCD_nTSRST);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}

static void
ft2232_gnice_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_GNICE_nTRST | BITMASK_GNICE_nLED);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_GNICE_nTRST | BITMASK_GNICE_nLED);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_GNICE_nTRST);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}

static void
ft2232_oocdlinks_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_OOCDLINKS_nTRST
                               | BITMASK_OOCDLINKS_nTRST_nOE
                               | BITMASK_OOCDLINKS_nSRST |
                               BITMASK_OOCDLINKS_nSRST_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_OOCDLINKS_nTRST |
                               BITMASK_OOCDLINKS_nTRST_nOE |
                               BITMASK_OOCDLINKS_nSRST |
                               BITMASK_OOCDLINKS_nSRST_nOE);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_OOCDLINKS_nTRST
                               | BITMASK_OOCDLINKS_nTRST_nOE
                               | BITMASK_OOCDLINKS_nSRST |
                               BITMASK_OOCDLINKS_nSRST_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_turtelizer2_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_TURTELIZER2_nJTAGOE);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_TURTELIZER2_nJTAGOE);

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_TURTELIZER2_nJTAGOE);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       switch off LEDs */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_TURTELIZER2_nTX1LED |
                               BITMASK_TURTELIZER2_nRX1LED);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_TURTELIZER2_nTX1LED |
                               BITMASK_TURTELIZER2_nRX1LED);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_usbjtagrs232_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBJTAGRS232_nTRST
                               | BITMASK_USBJTAGRS232_nTRST_nOE
                               | BITMASK_USBJTAGRS232_nSRST |
                               BITMASK_USBJTAGRS232_nSRST_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBJTAGRS232_nTRST |
                               BITMASK_USBJTAGRS232_nTRST_nOE |
                               BITMASK_USBJTAGRS232_nSRST |
                               BITMASK_USBJTAGRS232_nSRST_nOE);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBJTAGRS232_nTRST
                               | BITMASK_USBJTAGRS232_nTRST_nOE
                               | BITMASK_USBJTAGRS232_nSRST |
                               BITMASK_USBJTAGRS232_nSRST_nOE);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_usbtojtagif_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBTOJTAGIF_nTRST |
                               BITMASK_USBTOJTAGIF_RST);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBTOJTAGIF_nRxLED |
                               BITMASK_USBTOJTAGIF_nTxLED);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBTOJTAGIF_nRxLED |
                               BITMASK_USBTOJTAGIF_nTxLED);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_USBTOJTAGIF_nRxLED |
                               BITMASK_USBTOJTAGIF_nTxLED);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_signalyzer_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_SIGNALYZER_nTRST |
                               BITMASK_SIGNALYZER_nSRST);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_SIGNALYZER_nTRST |
                               BITMASK_SIGNALYZER_nSRST);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_SIGNALYZER_nTRST |
                               BITMASK_SIGNALYZER_nSRST);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}


static void
ft2232_flyswatter_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       disable output drivers */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_FLYSWATTER_nOE1 |
                               BITMASK_FLYSWATTER_nOE2);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_FLYSWATTER_nOE1 |
                               BITMASK_FLYSWATTER_nOE2);

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root,
                               BITMASK_FLYSWATTER_nOE1 |
                               BITMASK_FLYSWATTER_nOE2);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, BITMASK_FLYSWATTER_nLED2);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}

static void
ft2232_usbscarab2_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       deassert RST signals and blank LED */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}

static void
ft2232_milkymist_done (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Set Data Bits Low Byte
       set all to input */
    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);

    /* Set Data Bits High Byte
       set all to input */
    urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_cmd_push (cmd_root, 0);
    urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);

    urj_tap_cable_generic_usbconn_done (cable);
}

static void
ft2232_clock_schedule (urj_cable_t *cable, int tms, int tdi, int n)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    tms = tms ? 0x7f : 0;
    tdi = tdi ? 1 << 7 : 0;

    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    while (n > 0)
    {
        if (urj_tap_cable_cx_cmd_space
            (cmd_root, URJ_USBCONN_FTDX_MAXSEND_MPSSE) < 4)
        {
            /* no space left for Clock Data plus Send Immediate
               transfer queued commands to device and read receive data
               to internal buffer */
            urj_tap_cable_cx_xfer (cmd_root, &imm_cmd, cable,
                                   URJ_TAP_CABLE_COMPLETELY);
            urj_tap_cable_cx_cmd_queue (cmd_root, 0);
        }

        /* Clock Data to TMS/CS Pin (no Read) */
        urj_tap_cable_cx_cmd_push (cmd_root, MPSSE_WRITE_TMS |
                                   MPSSE_LSB | MPSSE_BITMODE |
                                   MPSSE_WRITE_NEG);
        if (n <= 7)
        {
            urj_tap_cable_cx_cmd_push (cmd_root, n - 1);
            n = 0;
        }
        else
        {
            urj_tap_cable_cx_cmd_push (cmd_root, 7 - 1);
            n -= 7;
        }
        urj_tap_cable_cx_cmd_push (cmd_root, tdi | tms);
    }

    params->signals &= ~(URJ_POD_CS_TMS | URJ_POD_CS_TDI | URJ_POD_CS_TCK);
    if (tms)
        params->signals |= URJ_POD_CS_TMS;
    if (tdi)
        params->signals |= URJ_POD_CS_TDI;
    // if (tck) params->signals |= URJ_POD_CS_TCK;
}


static void
ft2232_clock_compact_schedule (urj_cable_t *cable, int length, uint8_t byte)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    urj_tap_cable_cx_cmd_queue (cmd_root, 0);
    /* Clock Data to TMS/CS Pin (no Read) */
    urj_tap_cable_cx_cmd_push (cmd_root, MPSSE_WRITE_TMS |
                               MPSSE_LSB | MPSSE_BITMODE |
                               MPSSE_WRITE_NEG );
    urj_tap_cable_cx_cmd_push (cmd_root, length);
    urj_tap_cable_cx_cmd_push (cmd_root, byte);

    params->signals &= ~(URJ_POD_CS_TMS | URJ_POD_CS_TDI | URJ_POD_CS_TCK);
    if (byte >> length)
        params->signals |= URJ_POD_CS_TMS;
    if (byte >> 7)
        params->signals |= URJ_POD_CS_TDI;
    // if (tck) params->signals |= URJ_POD_CS_TCK;
}


static void
ft2232_clock (urj_cable_t *cable, int tms, int tdi, int n)
{
    params_t *params = cable->params;

    ft2232_clock_schedule (cable, tms, tdi, n);
    urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    params->last_tdo_valid = 0;
}


static void
ft2232_get_tdo_schedule (urj_cable_t *cable)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* Read Data Bits Low Byte */
    urj_tap_cable_cx_cmd_queue (cmd_root, 1);
    urj_tap_cable_cx_cmd_push (cmd_root, GET_BITS_LOW);
}


static int
ft2232_get_tdo_finish (urj_cable_t *cable)
{
    params_t *params = cable->params;
    int value;

    value = (urj_tap_cable_cx_xfer_recv (cable) & BITMASK_TDO) ? 1 : 0;

    params->last_tdo = value;
    params->last_tdo_valid = 1;

    return value;
}


static int
ft2232_get_tdo (urj_cable_t *cable)
{
    params_t *params = cable->params;

    ft2232_get_tdo_schedule (cable);
    urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    return ft2232_get_tdo_finish (cable);
}


static void
ft2232_set_signal_schedule (params_t *params, int mask, int val,
                            int set_low, int set_high)
{
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;

    /* filter for supported signals */
    mask &=
        URJ_POD_CS_TCK | URJ_POD_CS_TDI | URJ_POD_CS_TMS | URJ_POD_CS_TRST |
        URJ_POD_CS_RESET;
    if (mask != 0)
    {
        int sigs = (params->signals & ~mask) | (val & mask);
        uint8_t low_or = 0;
        uint8_t low_xor = 0;
        uint8_t high_xor = 0;

        /* prepare low and high byte */
        if (sigs & URJ_POD_CS_TCK)
            low_or |= BITMASK_TCK;
        if (sigs & URJ_POD_CS_TDI)
            low_or |= BITMASK_TDI;
        if (sigs & URJ_POD_CS_TMS)
            low_or |= BITMASK_TMS;
        /* TRST and RESET (SRST) are XOR'ed to the default value since
           the right value depends on the external circuitry (inverter or not) */
        if ((sigs & URJ_POD_CS_TRST) == 0)
            if (params->bit_trst >= 0)
            {
                if (params->bit_trst < 8)
                {
                    low_xor |= 1 << params->bit_trst;
                }
                else
                {
                    high_xor |= 1 << (params->bit_trst - 8);
                }
            }
        if ((sigs & URJ_POD_CS_RESET) == 0)
            if (params->bit_reset >= 0)
            {
                if (params->bit_reset < 8)
                {
                    low_xor |= 1 << params->bit_reset;
                }
                else
                {
                    high_xor |= 1 << (params->bit_reset - 8);
                }
            }

        if (set_low)
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, 0);
            urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_LOW);
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       (params->
                                        low_byte_value | low_or) ^ low_xor);
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       params->low_byte_dir | BITMASK_TCK
                                       | BITMASK_TDI | BITMASK_TMS);
        }

        if (set_high)
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, 0);
            urj_tap_cable_cx_cmd_push (cmd_root, SET_BITS_HIGH);
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       params->high_byte_value ^ high_xor);
            urj_tap_cable_cx_cmd_push (cmd_root, params->high_byte_dir);
        }

        params->signals = sigs;
    }
}


static int
ft2232_set_signal (urj_cable_t *cable, int mask, int val)
{
    params_t *params = cable->params;

    int prev_sigs = params->signals;

    ft2232_set_signal_schedule (params, mask, val, 1, 1);
    urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    params->last_tdo_valid = 0;

    return prev_sigs;
}


static void
ft2232_transfer_schedule (urj_cable_t *cable, int len, const char *in,
                          char *out)
{
    params_t *params = cable->params;
    urj_tap_cable_cx_cmd_root_t *cmd_root = &params->cmd_root;
    int in_offset = 0;
    int bitwise_len;
    int chunkbytes;

#if 0
    /* lower TMS for transfer
       also lower TCK to ensure correct clocking */
    ft2232_set_signal_schedule (params, URJ_POD_CS_TCK | URJ_POD_CS_TMS, 0, 1,
                                0);
#endif

    chunkbytes = len >> 3;
    while (chunkbytes > 0)
    {
        int byte_idx;

        /* reduce chunkbytes to the maximum amount we can receive in one step */
        if (out && chunkbytes > URJ_USBCONN_FTDX_MAXRECV)
            chunkbytes = URJ_USBCONN_FTDX_MAXRECV;
        /* reduce chunkbytes to the maximum amount that fits into one buffer
           for performance reasons */
        if (chunkbytes > URJ_USBCONN_FTDX_MAXSEND_MPSSE - 4)
            chunkbytes = URJ_USBCONN_FTDX_MAXSEND_MPSSE - 4;
        /* restrict chunkbytes to the maximum amount that can be transferred
           for one single operation */
        if (chunkbytes > (1 << 16))
            chunkbytes = 1 << 16;

    /***********************************************************************
     * Step 1:
     * Determine data shifting command (bytewise).
     * Either with or without read
     ***********************************************************************/
        if (out)
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, chunkbytes);
            /* Clock Data Bytes In and Out LSB First
               out on negative edge, in on positive edge */
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       MPSSE_DO_READ | MPSSE_DO_WRITE |
                                       MPSSE_LSB | MPSSE_WRITE_NEG);
        }
        else
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, 0);
            /* Clock Data Bytes Out on -ve Clock Edge LSB First (no Read) */
            urj_tap_cable_cx_cmd_push (cmd_root, MPSSE_DO_WRITE |
                                       MPSSE_LSB | MPSSE_WRITE_NEG);
        }
        /* set byte count */
        urj_tap_cable_cx_cmd_push (cmd_root, (chunkbytes - 1) & 0xff);
        urj_tap_cable_cx_cmd_push (cmd_root, ((chunkbytes - 1) >> 8) & 0xff);

    /*********************************************************************
     * Step 2:
     * Write TDI data in bundles of 8 bits.
     *********************************************************************/
        for (byte_idx = 0; byte_idx < chunkbytes; byte_idx++)
        {
            int bit_idx;
            unsigned char b = 0;

            for (bit_idx = 1; bit_idx < 256; bit_idx <<= 1)
                if (in[in_offset++])
                    b |= bit_idx;
            urj_tap_cable_cx_cmd_push (cmd_root, b);
        }

        /* recalc chunkbytes for next round */
        chunkbytes = (len - in_offset) >> 3;
    }

    /* determine bitwise shift amount */
    bitwise_len = (len - in_offset) % 8;
    if (bitwise_len > 0)
    {
    /***********************************************************************
     * Step 3:
     * Determine data shifting command (bitwise).
     * Either with or without read
     ***********************************************************************/
        if (out)
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, 1);
            /* Clock Data Bytes In and Out LSB First
               out on negative edge, in on positive edge */
            urj_tap_cable_cx_cmd_push (cmd_root,
                                       MPSSE_DO_READ | MPSSE_DO_WRITE |
                                       MPSSE_LSB | MPSSE_BITMODE |
                                       MPSSE_WRITE_NEG);
        }
        else
        {
            urj_tap_cable_cx_cmd_queue (cmd_root, 0);
            /* Clock Data Bytes Out on -ve Clock Edge LSB First (no Read) */
            urj_tap_cable_cx_cmd_push (cmd_root, MPSSE_DO_WRITE |
                                       MPSSE_LSB | MPSSE_BITMODE |
                                       MPSSE_WRITE_NEG);
        }
        /* determine bit count */
        urj_tap_cable_cx_cmd_push (cmd_root, bitwise_len - 1);

    /***********************************************************************
     * Step 4:
     * Write TDI data bitwise
     ***********************************************************************/
        {
            int bit_idx;
            unsigned char b = 0;
            for (bit_idx = 1; bit_idx < 1 << bitwise_len; bit_idx <<= 1)
            {
                if (in[in_offset++])
                    b |= bit_idx;
            }
            urj_tap_cable_cx_cmd_push (cmd_root, b);
        }
    }

    if (out)
    {
        /* Read Data Bits Low Byte to get current TDO,
           Do this only if we'll read out data nonetheless */
        urj_tap_cable_cx_cmd_queue (cmd_root, 1);
        urj_tap_cable_cx_cmd_push (cmd_root, GET_BITS_LOW);
        params->last_tdo_valid = 1;
    }
    else
        params->last_tdo_valid = 0;
}


static int
ft2232_transfer_finish (urj_cable_t *cable, int len, char *out)
{
    params_t *params = cable->params;
    int bitwise_len;
    int chunkbytes;
    int out_offset = 0;

    chunkbytes = len >> 3;
    bitwise_len = len % 8;

    if (out)
    {
        if (chunkbytes > 0)
        {
            uint32_t xferred;

      /*********************************************************************
       * Step 5:
       * Read TDO data in bundles of 8 bits if read is requested.
       *********************************************************************/
            xferred = chunkbytes;
            for (; xferred > 0; xferred--)
            {
                int bit_idx;
                unsigned char b;

                b = urj_tap_cable_cx_xfer_recv (cable);
                for (bit_idx = 1; bit_idx < 256; bit_idx <<= 1)
                    out[out_offset++] = (b & bit_idx) ? 1 : 0;
            }
        }

        if (bitwise_len > 0)
        {
      /***********************************************************************
       * Step 6:
       * Read TDO data bitwise if read is requested.
       ***********************************************************************/
            int bit_idx;
            unsigned char b;

            b = urj_tap_cable_cx_xfer_recv (cable);

            for (bit_idx = (1 << (8 - bitwise_len)); bit_idx < 256;
                 bit_idx <<= 1)
                out[out_offset++] = (b & bit_idx) ? 1 : 0;
        }

        /* gather current TDO */
        params->last_tdo =
            (urj_tap_cable_cx_xfer_recv (cable) & BITMASK_TDO) ? 1 : 0;
        params->last_tdo_valid = 1;
    }
    else
        params->last_tdo_valid = 0;

    return 0;
}


static int
ft2232_transfer (urj_cable_t *cable, int len, const char *in, char *out)
{
    params_t *params = cable->params;

    ft2232_transfer_schedule (cable, len, in, out);
    urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                           URJ_TAP_CABLE_COMPLETELY);
    return ft2232_transfer_finish (cable, len, out);
}


static void
ft2232_flush (urj_cable_t *cable, urj_cable_flush_amount_t how_much)
{
    params_t *params = cable->params;

    if (how_much == URJ_TAP_CABLE_OPTIONALLY)
        return;

    if (cable->todo.num_items == 0)
        urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                               how_much);

    while (cable->todo.num_items > 0)
    {
        int i, j, n;
        int post_signals = params->signals;
        int last_tdo_valid_schedule = params->last_tdo_valid;
        int last_tdo_valid_finish = params->last_tdo_valid;

        if (cable->todo.num_items == 1
            && cable->todo.data[cable->todo.next_item].action
               == URJ_TAP_CABLE_CLOCK_COMPACT
            && how_much != URJ_TAP_CABLE_COMPLETELY)
            break;

        for (j = i = cable->todo.next_item, n = 0; n < cable->todo.num_items;
             n++)
        {

            switch (cable->todo.data[i].action)
            {
            case URJ_TAP_CABLE_CLOCK:
            case URJ_TAP_CABLE_CLOCK_COMPACT:
                {
                    int tdi = cable->todo.data[i].arg.clock.tdi ? 1 << 7 : 0;
                    int length = 0;
                    uint8_t byte = 0;
                    int tms = 0;
                    int cn = 0;

                    if (cable->todo.data[i].action == URJ_TAP_CABLE_CLOCK_COMPACT)
                    {
                        length = cable->todo.data[i].arg.clock.n;
                        byte = cable->todo.data[i].arg.clock.tms;
                    }

                  more_cable_clock:

                    if (cable->todo.data[i].action == URJ_TAP_CABLE_CLOCK)
                    {
                        tms = cable->todo.data[i].arg.clock.tms ? 1 : 0;
                        cn = cable->todo.data[i].arg.clock.n;
                    }
                    while (cn > 0)
                    {
                        byte |= tms << length;
                        cn--;
                        length++;
                        if (length == 7)
                        {
                            ft2232_clock_compact_schedule (cable, 6, byte | tdi);
                            length = 0;
                            byte = 0;
                        }
                    }
                    if (n + 1 < cable->todo.num_items
                        && cable->todo.data[(i + 1) % cable->todo.max_items].action == URJ_TAP_CABLE_CLOCK
                        && (cable->todo.data[(i + 1) % cable->todo.max_items].arg.clock.tdi ? 1 << 7 : 0) == tdi)
                    {
                        i++;
                        if (i >= cable->todo.max_items)
                            i = 0;
                        n++;
                        goto more_cable_clock;
                    }
                    if (length)
                    {
                        if (n + 1 < cable->todo.num_items
                            || how_much == URJ_TAP_CABLE_COMPLETELY)
                            ft2232_clock_compact_schedule (cable, length - 1, byte | tdi);
                        else
                        {
                            cable->todo.data[i].action = URJ_TAP_CABLE_CLOCK_COMPACT;
                            cable->todo.data[i].arg.clock.tms = byte;
                            cable->todo.data[i].arg.clock.n = length;
                            i--;
                            if (i == -1)
                                i = cable->todo.max_items;
                        }
                    }

                    last_tdo_valid_schedule = 0;
                    break;
                }

            case URJ_TAP_CABLE_GET_TDO:
                if (!last_tdo_valid_schedule)
                {
                    ft2232_get_tdo_schedule (cable);
                    last_tdo_valid_schedule = 1;
                }
                break;

            case URJ_TAP_CABLE_SET_SIGNAL:
                ft2232_set_signal_schedule (params,
                                            cable->todo.data[i].arg.value.
                                            mask,
                                            cable->todo.data[i].arg.value.val,
                                            1, 1);
                last_tdo_valid_schedule = 0;
                break;

            case URJ_TAP_CABLE_TRANSFER:
                ft2232_transfer_schedule (cable,
                                          cable->todo.data[i].arg.transfer.
                                          len,
                                          cable->todo.data[i].arg.transfer.in,
                                          cable->todo.data[i].arg.transfer.
                                          out);
                last_tdo_valid_schedule = params->last_tdo_valid;
                break;

            default:
                break;
            }

            i++;
            if (i >= cable->todo.max_items)
                i = 0;
        }

        urj_tap_cable_cx_xfer (&params->cmd_root, &imm_cmd, cable,
                               how_much);

        while (j != i)
        {
            switch (cable->todo.data[j].action)
            {
            case URJ_TAP_CABLE_CLOCK:
                {
                    post_signals &=
                        ~(URJ_POD_CS_TCK | URJ_POD_CS_TDI | URJ_POD_CS_TMS);
                    post_signals |=
                        (cable->todo.data[j].arg.clock.
                         tms ? URJ_POD_CS_TMS : 0);
                    post_signals |=
                        (cable->todo.data[j].arg.clock.
                         tdi ? URJ_POD_CS_TDI : 0);
                    params->last_tdo_valid = last_tdo_valid_finish = 0;
                    break;
                }
            case URJ_TAP_CABLE_CLOCK_COMPACT:
                {
                    post_signals &=
                        ~(URJ_POD_CS_TCK | URJ_POD_CS_TDI | URJ_POD_CS_TMS);
                    post_signals |=
                        ((cable->todo.data[j].arg.clock.
                          tms >> cable->todo.data[j].arg.clock.
                          n) ? URJ_POD_CS_TMS : 0);
                    post_signals |=
                        (cable->todo.data[j].arg.clock.
                         tdi ? URJ_POD_CS_TDI : 0);
                    params->last_tdo_valid = last_tdo_valid_finish = 0;
                    break;
                }
            case URJ_TAP_CABLE_GET_TDO:
                {
                    int tdo;
                    int m;
                    if (last_tdo_valid_finish)
                        tdo = params->last_tdo;
                    else
                        tdo = ft2232_get_tdo_finish (cable);
                    last_tdo_valid_finish = params->last_tdo_valid;
                    m = urj_tap_cable_add_queue_item (cable, &cable->done);
                    cable->done.data[m].action = URJ_TAP_CABLE_GET_TDO;
                    cable->done.data[m].arg.value.val = tdo;
                    break;
                }
            case URJ_TAP_CABLE_SET_SIGNAL:
                {
                    int m =
                        urj_tap_cable_add_queue_item (cable, &cable->done);
                    cable->done.data[m].action = URJ_TAP_CABLE_SET_SIGNAL;
                    cable->done.data[m].arg.value.mask =
                        cable->todo.data[j].arg.value.mask;
                    cable->done.data[m].arg.value.val = post_signals;
                    int mask =
                        cable->todo.data[j].arg.value.
                        mask & ~(URJ_POD_CS_TCK | URJ_POD_CS_TDI |
                                 URJ_POD_CS_TMS | URJ_POD_CS_TRST |
                                 URJ_POD_CS_RESET);
                    post_signals =
                        (post_signals & ~mask) | (cable->todo.data[j].arg.
                                                  value.val & mask);
                }
            case URJ_TAP_CABLE_GET_SIGNAL:
                {
                    int m =
                        urj_tap_cable_add_queue_item (cable, &cable->done);
                    cable->done.data[m].action = URJ_TAP_CABLE_GET_SIGNAL;
                    cable->done.data[m].arg.value.sig =
                        cable->todo.data[j].arg.value.sig;
                    cable->done.data[m].arg.value.val =
                        (post_signals & cable->todo.data[j].arg.value.
                         sig) ? 1 : 0;
                    break;
                }
            case URJ_TAP_CABLE_TRANSFER:
                {
                    int r = ft2232_transfer_finish (cable,
                                                    cable->todo.data[j].arg.
                                                    transfer.len,
                                                    cable->todo.data[j].arg.
                                                    transfer.out);
                    last_tdo_valid_finish = params->last_tdo_valid;
                    free (cable->todo.data[j].arg.transfer.in);
                    if (cable->todo.data[j].arg.transfer.out)
                    {
                        int m = urj_tap_cable_add_queue_item (cable,
                                                              &cable->done);
                        if (m < 0)
                        {
                            // retain error state
                            // urj_log (URJ_LOG_LEVEL_NORMAL, "out of memory!\n");
                        }
                        cable->done.data[m].action = URJ_TAP_CABLE_TRANSFER;
                        cable->done.data[m].arg.xferred.len =
                            cable->todo.data[j].arg.transfer.len;
                        cable->done.data[m].arg.xferred.res = r;
                        cable->done.data[m].arg.xferred.out =
                            cable->todo.data[j].arg.transfer.out;
                    }
                }
            default:
                break;
            }

            j++;
            if (j >= cable->todo.max_items)
                j = 0;
            cable->todo.num_items--;
        }

        cable->todo.next_item = i;
    }
}


static int
ft2232_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    params_t *cable_params;

    /* perform urj_tap_cable_generic_usbconn_connect */
    if (urj_tap_cable_generic_usbconn_connect (cable, params) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    cable_params = malloc (sizeof (*cable_params));
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (*cable_params));
        /* NOTE:
         * Call the underlying usbport driver (*free) routine directly
         * not urj_tap_cable_generic_usbconn_free() since it also free's cable->params
         * (which is not established) and cable (which the caller will do)
         */
        cable->link.usb->driver->free (cable->link.usb);
        return URJ_STATUS_FAIL;
    }

    cable_params->mpsse_frequency = 0;
    cable_params->last_tdo_valid = 0;

    urj_tap_cable_cx_cmd_init (&cable_params->cmd_root);

    /* exchange generic cable parameters with our private parameter set */
    free (cable->params);
    cable->params = cable_params;

    return URJ_STATUS_OK;
}


static void
ft2232_cable_free (urj_cable_t *cable)
{
    params_t *params = cable->params;

    urj_tap_cable_cx_cmd_deinit (&params->cmd_root);

    urj_tap_cable_generic_usbconn_free (cable);
}


void
ftdx_usbcable_help (urj_log_level_t ll, const char *cablename)
{
    const char *ex_short = "[driver=DRIVER]";
    const char *ex_desc = "DRIVER     usbconn driver, either ftdi-mpsse or ftd2xx-mpsse\n";
    urj_tap_cable_generic_usbconn_help_ex (ll, cablename, ex_short, ex_desc);
}


const urj_cable_driver_t urj_tap_cable_ft2232_driver = {
    "FT2232",
    N_("Generic FTDI FT2232 Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_generic_init,
    ft2232_generic_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0000, 0x0000, "-mpsse", "FT2232", ft2232)

const urj_cable_driver_t urj_tap_cable_ft2232_armusbocd_driver = {
    "ARM-USB-OCD",
    N_("Olimex ARM-USB-OCD[-TINY] (FT2232) Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_armusbocd_init,
    ft2232_armusbocd_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x15BA, 0x0003, "-mpsse", "ARM-USB-OCD", armusbocd)
URJ_DECLARE_FTDX_CABLE(0x15BA, 0x0004, "-mpsse", "ARM-USB-OCD", armusbocdtiny)

const urj_cable_driver_t urj_tap_cable_ft2232_armusbtiny_h_driver = {
    "ARM-USB-OCD-H",
    N_("Olimex ARM-USB-TINY-H (FT2232H) Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_armusbtiny_h_init,
    ft2232_armusbocd_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x15BA, 0x002A, "-mpsse", "ARM-USB-TINY-H", armusbtiny_h)

const urj_cable_driver_t urj_tap_cable_ft2232_gnice_driver = {
    "gnICE",
    N_("Analog Devices Blackfin gnICE (FT2232) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_gnice_init,
    ft2232_gnice_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0456, 0xF000, "-mpsse", "gnICE", gnice)

const urj_cable_driver_t urj_tap_cable_ft2232_gniceplus_driver = {
    "gnICE+",
    N_("Analog Devices Blackfin gnICE+ (FT2232H) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_gniceplus_init,
    ft2232_gnice_done,
    ft2232h_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0456, 0xF001, "-mpsse", "gnICE+", gniceplus)

const urj_cable_driver_t urj_tap_cable_ft2232_jtagkey_driver = {
    "JTAGkey",
    N_("Amontec JTAGkey (FT2232) Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_jtagkey_init,
    ft2232_jtagkey_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0403, 0xCFF8, "-mpsse", "JTAGkey", jtagkey)

const urj_cable_driver_t urj_tap_cable_ft2232_oocdlinks_driver = {
    "OOCDLink-s",
    N_("OOCDLink-s (FT2232) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_oocdlinks_init,
    ft2232_oocdlinks_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0403, 0xbaf8, "-mpsse", "OOCDLink-s", oocdlinks)

const urj_cable_driver_t urj_tap_cable_ft2232_turtelizer2_driver = {
    "Turtelizer2",
    N_("Turtelizer 2 Rev. B (FT2232) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_turtelizer2_init,
    ft2232_turtelizer2_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0403, 0xBDC8, "-mpsse", "Turtelizer2", turtelizer2)

const urj_cable_driver_t urj_tap_cable_ft2232_usbjtagrs232_driver = {
    "USB-JTAG-RS232",
    N_("USB<=>JTAG&RS232 (FT2232) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_usbjtagrs232_init,
    ft2232_usbjtagrs232_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x1457, 0x5118, "-mpsse", "USB-JTAG-RS232", usbjtagrs232)

const urj_cable_driver_t urj_tap_cable_ft2232_usbtojtagif_driver = {
    "USB-to-JTAG-IF",
    N_("USB to JTAG Interface (FT2232) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_usbtojtagif_init,
    ft2232_usbtojtagif_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0000, 0x0000, "-mpsse", "USB-to-JTAG-IF", usbtojtagif)

const urj_cable_driver_t urj_tap_cable_ft2232_signalyzer_driver = {
    "Signalyzer",
    N_("Xverve DT-USB-ST Signalyzer Tool (FT2232) Cable (EXPERIMENTAL)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_signalyzer_init,
    ft2232_signalyzer_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0403, 0xbca1, "-mpsse", "Signalyzer", signalyzer)

const urj_cable_driver_t urj_tap_cable_ft2232_flyswatter_driver = {
    "Flyswatter",
    N_("TinCanTools Flyswatter (FT2232) Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_flyswatter_init,
    ft2232_flyswatter_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0403, 0x6010, "-mpsse", "Flyswatter", flyswatter)

const urj_cable_driver_t urj_tap_cable_ft2232_usbscarab2_driver = {
    "usbScarab2",
    N_("KrisTech usbScarabeus2 (FT2232) Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_usbscarab2_init,
    ft2232_usbscarab2_done,
    ft2232_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x0403, 0xbbe0, "-mpsse", "usbScarab2", usbscarab2)

const urj_cable_driver_t urj_tap_cable_ft2232_milkymist_driver = {
    "milkymist",
    N_("Milkymist JTAG/serial (FT2232) Cable"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ft2232_connect, },
    urj_tap_cable_generic_disconnect,
    ft2232_cable_free,
    ft2232_milkymist_init,
    ft2232_milkymist_done,
    ft2232h_set_frequency,
    ft2232_clock,
    ft2232_get_tdo,
    ft2232_transfer,
    ft2232_set_signal,
    urj_tap_cable_generic_get_signal,
    ft2232_flush,
    ftdx_usbcable_help
};
URJ_DECLARE_FTDX_CABLE(0x20b7, 0x0713, "-mpsse", "milkymist", milkymist)

/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
