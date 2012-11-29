/*
 * Cable driver fo Analog Devices, Inc. USB ICEs
 *
 * Copyright (C) 2010, Analog Devices, Inc.
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
 * Written by Paula R. Bertrand, 2010.
 */

#include <sysdep.h>

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <urjtag/tap_state.h>
#include <urjtag/cable.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>

#include "libiberty.h"

#include "generic.h"
#include "generic_usbconn.h"

#include <urjtag/usbconn.h>
#include "usbconn/libusb.h"

/*
 * Internal Structures
 */

/* JTAG TMS/TDI Data */
typedef struct
{
    uint8_t tms;            /* TMS data */
    uint8_t tdi;            /* TDI data */
} tap_pairs;

/* For collecting data */
typedef struct
{
    int32_t idx;            /* Index where data is to be collected */
    int32_t pos;            /* Bit position where data is to be collected */
} dat_dat;

/* Master scan control structure */
typedef struct
{
    int32_t total;             /* Max number of tap pointers */
    int32_t cur_idx;           /* Where to add next, or total */
    int32_t bit_pos;           /* Position to place next bit */
    int32_t num_dat;           /* Total posible data collection points */
    int32_t cur_dat;           /* Index to dat array for data to be collected */
    int32_t rcv_dat;           /* Index to retreive collected data */
    dat_dat *dat;              /* Pointer to data collection points */
    unsigned char *cmd;        /* Pointer to command, which encompasses pairs */
    tap_pairs *pairs;          /* Pointer to tap pairs array */
} num_tap_pairs;

/* Cable params_t structure with our data */
typedef struct
{
    uint32_t cur_freq;               /* JTAG Frequency */
    uint16_t version;                /* ICE-100B Firmware Version */
    uint32_t default_scanlen;        /* #Scan pairs in scan */
    uint32_t trigger_scanlen;        /* High water mark */
    int32_t tap_pair_start_idx;      /* depends on firmware version */
    int32_t num_rcv_hdr_bytes;       /* Number of data bytes in received raw scan data header */
    int32_t max_raw_data_tx_items;   /* depends on firmware version */
    int32_t wr_ep;                   /* USB End Write Point */
    int32_t wr_timeout;              /* USB Write Timeout */
    int32_t wr_buf_sz;               /* USB Write Buffer Size */
    int32_t r_ep;                    /* USB End Read Point */
    int32_t r_timeout;               /* USB Read Timeout */
    int32_t r_buf_sz;                /* USB Read Buffer Size */
    num_tap_pairs tap_info;          /* For collecting and sending tap scans */
    char *firmware_filename;         /* The update firmware file name */
} params_t;

/* Emulators's USB Data structure */
typedef struct
{
    uint32_t command;       /* What to do */
    uint32_t buffer;        /* used for Kit only, always initialized to 0 */
    uint32_t count;         /* Amount of data in bytes to send */
} usb_command_block;


/*
 * Internal Prototypes
 */

static int perform_scan (urj_cable_t *cable, uint8_t **rdata);
static int do_rawscan (urj_cable_t *cable, uint8_t firstpkt, uint8_t lastpkt,
                       int32_t collect_dof, int32_t dif_cnt, uint8_t *raw_buf,
                       uint8_t *out);
static int build_clock_scan (urj_cable_t *cable, int32_t *start_idx, int32_t *num_todo_items);
static int add_scan_data (urj_cable_t *cable, int32_t num_bits, char *in, char *out);
static void get_recv_data (urj_cable_t *cable, int32_t idx, int32_t dat_idx, uint8_t **rcv_dataptr);
static uint16_t do_host_cmd (urj_cable_t *cable, uint8_t cmd, uint8_t param, int32_t r_data);
static uint32_t do_single_reg_value (urj_cable_t *cable, uint8_t reg, int32_t r_data,
                                     int32_t wr_data, uint32_t data);


/*
 * Debug Macros
 */

#if 0    /* set to 1 to output debug info about scans */

//#define DSP_SCAN_DATA
#define DUMP_EACH_RCV_DATA
//#define DSP_SCAN_CAUSE
#define DEBUG(...)    printf(__VA_ARGS__)

#else

#define DEBUG(...)

#endif


/*
 * Internal Data, defines and Macros
 */
#define ICE_DEFAULT_SCAN_LEN            0x7FF0    /* Max DIF is 0x2AAA8, but DMA is only 16 bits. */
#define ICE_TRIGGER_SCAN_LEN            0x7FD8    /* Start checking for RTI/TLR for xmit */

#define SELECTIVE_RAW_SCAN_HDR_SZ       12

#define DAT_SZ                          0x8000    /* size allocated for reading data */
#define DAT_SZ_INC                      0x40      /* size to increase if data full */

/* USB Emulator Commands */
#define HOST_GET_FW_VERSION             0x01    /* get the firmware version */
#define HOST_REQUEST_RX_DATA            0x02    /* host request to transmit data */
#define HOST_REQUEST_TX_DATA            0x04    /* host request to transmit data */
#define HOST_GET_SINGLE_REG             0x08    /* set a JTAG register */
#define HOST_SET_SINGLE_REG             0x09    /* set a JTAG register */
#define HOST_PROGRAM_FLASH              0x0C    /* program flash */
#define HOST_HARD_RESET_JTAG_CTRLR      0x0E    /* do a hard reset on JTAG controller */
#define HOST_SET_TRST                   0x1F    /* changes TRST Line state */
#define HOST_GET_TRST                   0x20    /* gets TRST Line state */
#define HOST_DO_SELECTIVE_RAW_SCAN      0x21    /* Return only data needed */

/* Registers */
#define REG_AUX                         0x00
#define REG_SCR                         0x04
#define REG_FREQ                        0x40

#define SCR_DEFAULT                     0x30A0461
#define SCR_TRST_BIT                    0x0000040

/* Ice USB controls */
#define ICE_100B_WRITE_ENDPOINT         0x06
#define ICE_100B_READ_ENDPOINT          0x05
#define ICE_100B_USB_WRITE_TIMEOUT      10000
#define ICE_100B_USB_READ_TIMEOUT       30000
#define ICE_100B_WRITE_BUFFER_SIZE      0x9800
#define ICE_100B_READ_BUFFER_SIZE       0x8000

#define ICE100B_DOC_URL \
    "http://docs.blackfin.uclinux.org/doku.php?id=hw:jtag:ice100b"

/* frequency settings for ice-100b */
#define MAX_FREQ    4        /* size of freq_set */
static const uint8_t freq_set[MAX_FREQ]     = { 9, 4, 2, 1 };
static const uint32_t avail_freqs[MAX_FREQ] = { 5000000, 10000000, 17000000, 25000000 };


/*
 * Internal Macros
 */

#define adi_usb_read_or_ret(p, buf, len) \
do { \
    int __ret, __actual, __size = (len); \
    __ret = libusb_bulk_transfer (((urj_usbconn_libusb_param_t *)p)->handle, \
                                  cable_params->r_ep | LIBUSB_ENDPOINT_IN, \
                                  (unsigned char *)(buf), __size, \
                                  &__actual, cable_params->r_timeout); \
    if (__ret || __actual != __size) \
    { \
        urj_error_IO_set (_("%s: unable to read from usb to " #buf ": %i;" \
                            "wanted %i bytes but only received %i bytes"), \
                          __func__, __ret, __size, __actual); \
        return URJ_STATUS_FAIL; \
    } \
} while (0)

#define adi_usb_write_or_ret(p, buf, len) \
do { \
    int __ret, __actual, __size = (len); \
    __ret = libusb_bulk_transfer (((urj_usbconn_libusb_param_t *)p)->handle, \
                                  cable_params->wr_ep, \
                                  (unsigned char *)(buf), __size, \
                                  &__actual, cable_params->wr_timeout); \
    if (__ret || __actual != __size) \
    { \
        urj_error_IO_set (_("%s: unable to write from " #buf " to usb: %i;" \
                            "wanted %i bytes but only wrote %i bytes"), \
                          __func__, __ret, __size, __actual); \
        return URJ_STATUS_FAIL; \
    } \
} while (0)

/*
 * System Interface Functions
 */

/*
 * Gets available Frequency index.  Currently only used by ICE-100B
 */
static int adi_get_freq (uint32_t freq, int arr_sz, const uint32_t *freq_arr)
{
    int i;

    /* Verify Frequency is valid */
    for (i = 0; i < arr_sz; i++)
    {
        if (freq == freq_arr[i])
        {   /* spot on */
            break;
        }
        else if (freq < freq_arr[i])
        {   /* an in between frequency */
            if (i > 0)
                i--;
            break;
        }
    }

    if (i == arr_sz)
    {   /* must of entered something above the max! */
        i--;
    }

    return i;
}

/*
 * Sets ICE-X Frequency
 */
static void ice100b_set_freq (urj_cable_t *cable, uint32_t freq)
{
    params_t *params = cable->params;

    /* Verify Frequency is valid */
    if (freq != params->cur_freq)
    {   /* only change if different from current settings */
        int idx = adi_get_freq (freq, MAX_FREQ, &avail_freqs[0]);

        if (avail_freqs[idx] != params->cur_freq)
        {   /* only change if different from current settings
             * this call's frequency may have been not one of
             * the defined settings, but ends up there */
            params->cur_freq = freq;
            do_single_reg_value (cable, REG_FREQ, 1, 1, freq_set[idx]);
            cable->frequency = params->cur_freq;
        }
    }
}

/*
 * This function sets us up the cable and data
 */
static int adi_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    int i, ret;
    params_t *cable_params;

    /* Handle Parameters (if need be)
     * This will connect us to our Driver, parameters
     * include VID & PID, */
    ret = urj_tap_cable_generic_usbconn_connect (cable, params);
    if (ret != URJ_STATUS_OK)
        return ret;
    /* Do our set up with our parameters */
    cable_params = malloc (sizeof (*cable_params));
    if (!cable_params)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (*cable_params));
        cable->link.usb->driver->free (cable->link.usb);
        return URJ_STATUS_FAIL;
    }

    cable_params->tap_info.dat = malloc (sizeof (dat_dat) * DAT_SZ);
    if (!cable_params->tap_info.dat)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                       sizeof (dat_dat) * DAT_SZ);
        cable->link.usb->driver->free (cable->link.usb);
        free (cable_params);
        return URJ_STATUS_FAIL;
    }

    /* Initialize receive data array to unused */
    for (i = 0; i < DAT_SZ; ++i)
    {
        cable_params->tap_info.dat[i].idx = -1;
        cable_params->tap_info.dat[i].pos = -1;
    }

    cable_params->cur_freq         = 0;
    cable_params->version          = 0;
    cable_params->tap_info.total   = 0;
    cable_params->tap_info.cur_idx = 0;
    cable_params->tap_info.bit_pos = 0;
    cable_params->tap_info.num_dat = DAT_SZ;
    cable_params->tap_info.rcv_dat = -1;
    cable_params->tap_info.cur_dat = -1;
    cable_params->tap_info.pairs   = NULL;
    cable_params->tap_info.cmd     = NULL;

    /* exchange generic cable parameters with our private parameter set */
    free (cable->params);
    cable->params = cable_params;

    return URJ_STATUS_OK;
}

struct flash_block
{
    uint32_t addr;
    int bytes;
    uint8_t *data;
    struct flash_block *next;
};

/* Macros for converting between hex and binary.  */

#define NIBBLE(x)      (hex_value (x))
#define HEX2(buffer)   ((NIBBLE ((buffer)[0]) << 4) + NIBBLE ((buffer)[1]))
#define HEX4(buffer)   ((HEX2 (buffer) << 8) + HEX2 ((buffer) + 2))
#define ISHEX(x)       (hex_p (x))
#define ISHEX2(buffer) (ISHEX ((buffer)[0]) && ISHEX ((buffer)[1]))
#define ISHEX4(buffer) (ISHEX2 (buffer) && ISHEX2 ((buffer + 2)))


static int
ice_read_hex_file (const char *filename, struct flash_block **flash_list)
{
    FILE *hex_file;
    char *input_line, *p;
    size_t len;
    int lineno, i;
    bool done = false;
    struct flash_block *last_flash_block = NULL, *q;
    int base_address = 0;

    hex_file = fopen (filename, FOPEN_R);
    if (!hex_file)
    {
        urj_error_IO_set (_("Unable to open file `%s'"), filename);
        return URJ_STATUS_FAIL;
    }

    hex_init ();

    input_line = NULL;
    len = 0;
    lineno = 0;
    while (getline (&input_line, &len, hex_file) != -1 && !done)
    {
        int byte_count, address, record_type, checksum;
        int sum;

        lineno++;
        p = input_line;
        /* A line should contain
           1. start code : 1 character ':'
           2. byte count : 2 hex digits
           3. address    : 4 hex digits
           4. record type: 2 hex digits
           5. data       : 2n hex digits
           6. checksum   : 2 hex digits */
        if (len < 11)
        {
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("Line %d is too short in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }

        if (*p++ != ':')
        {
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("Invalid start code on line %d in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }

        if (!ISHEX2 (p))
        {
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("Bad byte count on line %d in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }
        byte_count = HEX2 (p);
        p += 2;
        sum = byte_count;

        if (!ISHEX4 (p))
        {
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("Bad address on line %d in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }
        address = HEX4 (p);
        p += 4;
        sum += (address >> 8) + (address & 0xff);
        sum &= 0xff;

        /* record type */
        if (!ISHEX2 (p))
            goto bad_record_type;
        record_type = HEX2 (p);
        p += 2;
        sum += record_type;
        sum &= 0xff;

        switch (record_type)
        {
        case 0:
            q = last_flash_block;

            if (!q
                || q->addr + q->bytes != base_address + address)
            {
                q = malloc (sizeof (*q));
                if (!q)
                {
                    urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                                   _("malloc (%zd) failed"), sizeof (*q));
                    free (input_line);
                    return URJ_STATUS_FAIL;
                }
                q->addr = base_address + address;
                q->data = NULL;
                q->bytes = 0;
                q->next = NULL;

                if (last_flash_block)
                    last_flash_block->next = q;
                else
                    *flash_list = q;
                last_flash_block = q;
            }

            q->data = realloc (q->data, q->bytes + byte_count);
            if (!q->data)
            {
                urj_error_set (URJ_ERROR_OUT_OF_MEMORY,
                               _("realloc (%d) failed"),
                               q->bytes + byte_count);
                free (input_line);
                return URJ_STATUS_FAIL;
            }

            for (i = 0; i < byte_count; i++)
            {
                int data;

                if (!ISHEX2 (p))
                {
                    urj_error_set (URJ_ERROR_FIRMWARE,
                                   _("Bad HEX data %c%c on line %d in file %s"),
                                   p[0], p[1], lineno, filename);
                    free (input_line);
                    return URJ_STATUS_FAIL;
                }
                data = HEX2 (p);
                q->data[q->bytes] = data;
                q->bytes++;
                p += 2;
                sum += data;
            }
            break;

        case 1:
            done = true;
            break;

        case 2:
            /* fall through */
        case 4:
            if (!ISHEX4 (p))
            {
                urj_error_set (URJ_ERROR_FIRMWARE,
                               _("Bad extended segment address on line %d in file %s"),
                               lineno, filename);
                free (input_line);
                return URJ_STATUS_FAIL;
            }
            base_address = HEX4 (p);
            sum += (base_address >> 8) + (base_address & 0xff);
            sum &= 0xff;
            base_address <<= (record_type == 2 ? 4 : 16);
            p += 4;
            break;

        default:
        bad_record_type:
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("Bad HEX record type on line %d in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }

        if (!ISHEX2 (p))
        {
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("Bad HEX checksum on line %d in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }
        checksum = HEX2 (p);
        if (((sum + checksum) & 0xff) != 0)
        {
            urj_error_set (URJ_ERROR_FIRMWARE,
                           _("The checksum is not correct on line %d in file %s"),
                           lineno, filename);
            free (input_line);
            return URJ_STATUS_FAIL;
        }
    }

    free (input_line);
    return URJ_STATUS_OK;
}

/* Calculate the CRC using forward CRC-16-CCITT algorithm.  */

static uint16_t
ice_calculate_crc (struct flash_block *p)
{
    uint16_t crc = 0xffff;

    while (p)
    {
        int i;

        for (i = 0; i < p->bytes; i++)
        {
            uint8_t byte = p->data[i];
            int j;

            for (j = 0; j < 8; j++)
            {
                bool add = ((crc >> 15) != (byte >> 7));
                crc <<= 1;
                byte <<= 1;
                if (add)
                    crc ^= 0x1021;
            }
        }
        p = p->next;
    }

    return crc;
}

static int
ice_send_flash_data (urj_cable_t *cable, struct flash_block *p, uint16_t crc)
{
    params_t *cable_params = cable->params;
    /* ICE_100B_READ_BUFFER_SIZE / 4 (i.e. 0x2000) was chosen by experiments.
       It might not be related to ICE_100B_READ_BUFFER_SIZE at all.  */
    uint8_t buffer[ICE_100B_READ_BUFFER_SIZE / 4];
    uint8_t first = 1, last = 0;

    while (p)
    {
        int remaining = p->bytes;
        uint32_t address = p->addr;

        while (remaining)
        {
            usb_command_block usb_cmd_blk;
            uint32_t count;

            urj_log (URJ_LOG_LEVEL_NORMAL, "updating ...\n");

            if (remaining < ICE_100B_READ_BUFFER_SIZE / 4 - 16)
                count = remaining;
            else
                count = ICE_100B_READ_BUFFER_SIZE / 4 - 16;
            remaining -= count;
            if (remaining == 0)
                last = 1;

            buffer[0] = first;
            buffer[1] = last;
            buffer[2] = HOST_PROGRAM_FLASH;
            buffer[3] = 0;
            memcpy (buffer + 4, &address, 4);
            memcpy (buffer + 8, &count, 4);
            memcpy (buffer + 12, &crc, 2);
            memcpy (buffer + 16, p->data + p->bytes - remaining - count, count);

            usb_cmd_blk.command = HOST_REQUEST_TX_DATA;
            usb_cmd_blk.count = count + 16;
            usb_cmd_blk.buffer = 0;
            adi_usb_write_or_ret (cable->link.usb->params, &usb_cmd_blk, sizeof (usb_cmd_blk));

            adi_usb_write_or_ret (cable->link.usb->params, buffer, usb_cmd_blk.count);

            first = 0;

            address += count;
        }

        p = p->next;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL, "done\n");

    return URJ_STATUS_OK;
}

/* The CRC is stored in the output buffer after programming the firmware
   into the flash.  Read the buffer after programming get the CRC.  */

static int
ice_firmware_crc (urj_cable_t *cable, uint16_t *p)
{
    params_t *cable_params = cable->params;
    usb_command_block usb_cmd_blk;

    usb_cmd_blk.command = HOST_REQUEST_RX_DATA;
    usb_cmd_blk.count = 2;
    usb_cmd_blk.buffer = 0;

    adi_usb_write_or_ret (cable->link.usb->params, &usb_cmd_blk, sizeof (usb_cmd_blk));

    adi_usb_read_or_ret (cable->link.usb->params, p, sizeof (*p));

    return URJ_STATUS_OK;
}

static int
ice_update_firmware (urj_cable_t *cable, const char *filename)
{
    struct flash_block *flash_list = NULL, *p;
    unsigned short crc1, crc2;
    int ret;

    urj_log (URJ_LOG_LEVEL_NORMAL, _("Updating to firmware %s\n"), filename);

    if ((ret = ice_read_hex_file (filename, &flash_list)) != URJ_STATUS_OK)
        return ret;

    crc1 = ice_calculate_crc (flash_list);

    ret = ice_send_flash_data (cable, flash_list, crc1);
    if (ret != URJ_STATUS_OK)
        return ret;

    if ((ret = ice_firmware_crc (cable, &crc2)) != URJ_STATUS_OK)
        return ret;

    while (flash_list)
    {
        p = flash_list->next;
        free (flash_list->data);
        free (flash_list);
        flash_list = p;
    }

    if (crc1 == crc2)
    {
        return URJ_STATUS_OK;
    }
    else
    {
        urj_error_set (URJ_ERROR_FIRMWARE, _("CRCs do NOT match"));
        return URJ_STATUS_FAIL;
    }
}

/*
 * This function sets us up the cable and data
 */
static int ice_connect (urj_cable_t *cable, const urj_param_t *params[])
{
    params_t *cable_params;
    int ret, i;

    if ((ret = adi_connect (cable, params)) != URJ_STATUS_OK)
        return ret;

    cable_params = cable->params;
    cable_params->default_scanlen = ICE_DEFAULT_SCAN_LEN;
    cable_params->trigger_scanlen = ICE_TRIGGER_SCAN_LEN;
    cable_params->wr_ep           = ICE_100B_WRITE_ENDPOINT;
    cable_params->r_ep            = ICE_100B_READ_ENDPOINT;
    cable_params->wr_timeout      = ICE_100B_USB_WRITE_TIMEOUT;
    cable_params->r_timeout       = ICE_100B_USB_READ_TIMEOUT;
    cable_params->wr_buf_sz       = ICE_100B_WRITE_BUFFER_SIZE;
    cable_params->r_buf_sz        = ICE_100B_READ_BUFFER_SIZE;
    cable_params->firmware_filename = NULL;

    if (params != NULL)
        for (i = 0; params[i] != NULL; i++)
        {
            switch (params[i]->key)
            {
            case URJ_CABLE_PARAM_KEY_FIRMWARE:
                cable_params->firmware_filename = strdup (params[i]->value.string);
                if (!cable_params->firmware_filename)
                {
                    urj_log (URJ_LOG_LEVEL_ERROR,
                             _("strdup (%s) fails\n"), params[i]->value.string);
                    return URJ_STATUS_FAIL;
                }
                break;
            default:
                break;
            }
        }

    return URJ_STATUS_OK;
}

/*
 * This function actually connects us to our ICE
 */
static int ice_init (urj_cable_t *cable)
{
    params_t *cable_params = cable->params;
    int ret;

    /* Open usb conn port */
    if (urj_tap_usbconn_open (cable->link.usb))
        return URJ_STATUS_FAIL;

    cable_params->version = do_host_cmd (cable, HOST_GET_FW_VERSION, 0, 1);
    do_host_cmd (cable, HOST_HARD_RESET_JTAG_CTRLR, 0, 0);
    urj_log (URJ_LOG_LEVEL_NORMAL, _("%s firmware version is %d.%d.%d\n"),
             cable->driver->name,
             ((cable_params->version >> 8) & 0xFF),
             ((cable_params->version >> 4) & 0x0F),
             ((cable_params->version)      & 0x0F));

    if (cable_params->version < 0x0107)
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 _("The firmware on the ICE-100B needs to be updated. "
                   "Please go to <%s> to learn how to update the firmware.\n"), ICE100B_DOC_URL);
    }

    if (cable_params->firmware_filename)
    {
        ret = ice_update_firmware (cable, cable_params->firmware_filename);

        if (ret == URJ_STATUS_OK)
        {
            urj_log (URJ_LOG_LEVEL_NORMAL,
                     _("The firmware has been updated successfully. "
                       "Please unplug the ICE-100B cable and reconnect it to finish the update process.\n"));
        }
        else
        {
            urj_log_error_describe (URJ_LOG_LEVEL_ERROR);
            urj_log (URJ_LOG_LEVEL_ERROR,
                     _("The firmware failed to update.\n"));
        }
    }

    if (cable_params->version < 0x0107 || cable_params->firmware_filename)
    {
        return URJ_STATUS_FAIL;
    }

    /* Set frequency to lowest value */
    ice100b_set_freq (cable, avail_freqs[0]);
    cable_params->tap_pair_start_idx = SELECTIVE_RAW_SCAN_HDR_SZ;
    cable_params->max_raw_data_tx_items = cable_params->wr_buf_sz - cable_params->tap_pair_start_idx;
    cable_params->num_rcv_hdr_bytes = cable_params->tap_pair_start_idx;

    return URJ_STATUS_OK;
}

/*
 * Set our data free!
 */
static void ice_cable_free (urj_cable_t *cable)
{
    params_t *cable_params = cable->params;
    num_tap_pairs *tap_info = &cable_params->tap_info;

    if (tap_info->pairs != NULL)
    {
        free (tap_info->cmd);
        free (tap_info->dat);
        tap_info->dat = NULL;
        tap_info->pairs = NULL;
        tap_info->cmd = NULL;
        tap_info->total = 0;
        tap_info->cur_idx = 0;
        tap_info->bit_pos = 0;
        tap_info->num_dat = 0;
        tap_info->cur_dat = -1;
        tap_info->rcv_dat = -1;
    }
    free (cable_params->firmware_filename);
    urj_tap_cable_generic_usbconn_free (cable);
}

/*
 * This function adds padding of 0 to tdi and tms
 */
static void add_zero_padding (num_tap_pairs *tap_ptr, int32_t start_index, uint8_t bit_pos, int32_t pad_amount)
{
    int32_t n = start_index;
    int32_t count = pad_amount;
    uint8_t pos = bit_pos;

    /* this clears any additional bits to a byte boundary */
    while (count--)
    {
        tap_ptr->pairs[n].tms &= ~(pos);
        tap_ptr->pairs[n].tdi &= ~(pos);
        pos >>= 1;
        if (!pos)
        {
            n++;
            break;
        }
    }

    /* now we just need to set any additional bytes to 0
     * this should be quicker than continuing with each
     * individual bit */
    count /= 8;

    while (count--)
    {
        tap_ptr->pairs[n].tms = 0;
        tap_ptr->pairs[n].tdi = 0;
        n++;
    }
}

/*
 * takes tdi and tms and sends it out right away
 */
static void adi_clock (urj_cable_t *cable, int32_t tms, int32_t tdi, int32_t cnt)
{
    int32_t n = cnt;
    int32_t i = 0;
    int32_t padding = 32 - (cnt % 32);
    params_t *cable_params = cable->params;
    num_tap_pairs *tap_info = &cable_params->tap_info;
    uint8_t firstpkt = 1;
    uint8_t lastpkt = 1;
    int32_t collect_data = 0;
    uint8_t *in = NULL;
    uint8_t *out = NULL;
    int32_t dif_cnt = 0;
    uint8_t bit_set = 0x80;

    if (tap_info->pairs == NULL)
    {
        unsigned char *cmd;
        int32_t new_sz = cnt + padding;

        cmd = malloc ((sizeof (tap_pairs) * new_sz) + 1 + cable_params->tap_pair_start_idx);
        if (cmd == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                           (sizeof (tap_pairs) * new_sz) + 1 + cable_params->tap_pair_start_idx);
            return;
        }

        /* point our pairs to the space that was allocated */
        tap_info->pairs = (tap_pairs *)(cmd + cable_params->tap_pair_start_idx);    /* new pointer */

        /* initialize some of our structure */
        tap_info->cur_idx = 1;
        tap_info->total = cnt + padding;
        tap_info->num_dat = 0;
        tap_info->pairs[i].tms = 0;
        tap_info->pairs[i].tdi = 0;

        /* go through and set tms and tdi to the appropriate values */
        while (n-- > 0)
        {
            tap_info->pairs[i].tms |= tms ? bit_set : 0;
            tap_info->pairs[i].tdi |= tdi ? bit_set : 0;
            bit_set >>= 1;
            if (!bit_set)
            {
                /* start over again */
                bit_set = 0x80;
                i++;
                tap_info->pairs[i].tms = 0;
                tap_info->pairs[i].tdi = 0;
            }
        }

        add_zero_padding (tap_info, i, bit_set, padding);

        tap_info->cur_idx = new_sz / 8;     /* we scan in multiples of 32 */
        dif_cnt = (new_sz * 2) / 8;        /* dif_cnt is both TMS and TDI so *2 and then figure out how many BYTES */
        in = (uint8_t *)tap_info->pairs;
        out = malloc ( 16 );             /* allocate just for header */

        do_rawscan (cable, firstpkt, lastpkt, collect_data, dif_cnt, &in[0] - cable_params->tap_pair_start_idx, out);

        /* free memory */
        if (tap_info->pairs)
        {
            free (cmd);
            tap_info->pairs = NULL;
            tap_info->cmd = NULL;
        }

        free (out);

        /* reset some of our structure variables */
        tap_info->total = 0;
        tap_info->cur_idx = 0;
        tap_info->bit_pos = 0;
        tap_info->cur_dat = -1;
        tap_info->rcv_dat = -1;
    }
    else
    {
        /* should never get here unless someone forgot to free our pairs */
        urj_warning (_("tap_info->pairs should be NULL but it is not."));
        return;
    }
}

/*
 * Function is unused!!!
 */
static int adi_get_tdo (urj_cable_t *cable)
{
    urj_log (URJ_LOG_LEVEL_ERROR, _("%s function is not supported"), __func__);

    return URJ_STATUS_FAIL;
}

/*
 * Function is unused!!!
 */
static int adi_transfer (urj_cable_t *cable, int len, const char *in, char *out)
{
    urj_log (URJ_LOG_LEVEL_ERROR, _("%s function is not supported"), __func__);

    return URJ_STATUS_FAIL;
}

/*
 * Creates the scan, sends the scan and collects data
 */
static void adi_flush (urj_cable_t *cable, urj_cable_flush_amount_t how_much)
{
    params_t *cable_params = cable->params;
    num_tap_pairs *tap_info = &cable_params->tap_info;
    int32_t scan_out = (how_much == URJ_TAP_CABLE_COMPLETELY) ? 3 : 0;    /* Assigned a number for debug, !0 will do scan */
    int32_t tdo_idx = -1;
    int32_t i, j, k, n;
    uint8_t *tdo_ptr = NULL;

    if (how_much == URJ_TAP_CABLE_OPTIONALLY)
    {
        return;
    }
    else if (cable->todo.num_items == 0)
    {   /* If we get here, there are no data to collect (since we forced a scan previously) */
        if (how_much == URJ_TAP_CABLE_COMPLETELY && tap_info->pairs)
        {
#ifdef DSP_SCAN_DATA
            int32_t x = 0, z;
            char tms[64], tdi[64];

            DEBUG ("::::Flushing Scan Output\n");
            z = 0;
            while (z <= tap_info->cur_idx)
            {
                sprintf (&tms[x], " %2.2X", tap_info->pairs[z].tms);
                sprintf (&tdi[x], " %2.2X", tap_info->pairs[z].tdi);
                x += 3;
                z++;
                if ((z & 0x0F) == 0)
                {
                    DEBUG ("\t%s\n\t%s\n\n", tms, tdi);
                    x = 0;
                }
            }
            if (x > 0)
                DEBUG ("\t%s\n\t%s\n\n", tms, tdi);
#endif
            tdo_ptr = NULL;
            perform_scan (cable, &tdo_ptr);
            if (tdo_ptr)
            {
                free (tdo_ptr);
                tdo_ptr = NULL;
            }
            if (tap_info->pairs)
            {
                free (tap_info->cmd);
                tap_info->pairs = NULL;
                tap_info->cmd = NULL;
            }
            tap_info->total = 0;
            tap_info->cur_idx = 0;
            tap_info->bit_pos = 0;
            tap_info->cur_dat = -1;
            tap_info->rcv_dat = -1;
        }
        return;
    }

    while (cable->todo.num_items > 0)
    {
        urj_cable_queue_info_t *ptr_todo = &cable->todo;
        urj_cable_queue_info_t *ptr_done = &cable->done;
        urj_cable_queue_t *todo_data;
        urj_cable_queue_t *done_data;

        for (j = i = cable->todo.next_item, n = 0; n < cable->todo.num_items; n++)
        {
            todo_data = &ptr_todo->data[i];

            switch (todo_data->action)
            {   /* build the scan */
            case URJ_TAP_CABLE_CLOCK:
                build_clock_scan (cable, &i, &n);
                break;
            case URJ_TAP_CABLE_GET_TDO:
                if (!scan_out)
                    scan_out = 1;    /* Assigned a number for debug, !0 will do scan */
                break;
            case URJ_TAP_CABLE_SET_SIGNAL:
                /* process it after */
                break;
            case URJ_TAP_CABLE_GET_SIGNAL:
                /* process it after */
                break;
            case URJ_TAP_CABLE_TRANSFER:
                add_scan_data (cable, todo_data->arg.transfer.len,
                               todo_data->arg.transfer.in,
                               todo_data->arg.transfer.out);
                if (!scan_out && todo_data->arg.transfer.out)
                    scan_out = 2;    /* Assigned a number for debug, !0 will do scan */
                break;
            default:
                DEBUG("default action == %d; i = %d, n = %d\n", todo_data->action, i, n);
                break;
            }

            i++;
            if (i >= ptr_todo->max_items)
            {   /* wrap around buffer! */
                i = 0;
            }
        }

        if (!scan_out && cable->chain)
        {   /* Scan out if we reached our trigger point and we have
             * access to tap state and it is RTI or TLR */
            if (tap_info->cur_idx >= cable_params->trigger_scanlen &&
                (cable->chain->state == URJ_TAP_STATE_RUN_TEST_IDLE ||
                 cable->chain->state == URJ_TAP_STATE_RESET))
            {
                scan_out = 7;    /* Assigned a number for debug, !0 will do scan */
            }
            else if (tap_info->cur_idx >= cable_params->trigger_scanlen)
            {   /* Else scan out if we reached our trigger point */
                scan_out = 6;    /* Assigned a number for debug, !0 will do scan */
            }
            else if (tap_info->cur_idx >= cable_params->default_scanlen)
            {
                urj_log (URJ_LOG_LEVEL_ERROR,
                         _("FAULT! idx overflow!! idx = %d and max should be %#X\n"),
                         tap_info->cur_idx, cable_params->default_scanlen);
            }
        }

        if (tap_info->pairs && scan_out)
        {
#ifdef DSP_SCAN_DATA
            int32_t x = 0, z;
            char tms[128], tdi[128];

            z = 0;
            while (z <= tap_info->cur_idx)
            {
                sprintf (&tms[x], " %2.2X", tap_info->pairs[z].tms);
                sprintf (&tdi[x], " %2.2X", tap_info->pairs[z].tdi);
                x += 3;
                z++;
                if ((z & 0x0F) == 0)
                {
                    DEBUG ("\t%s\n\t%s\n\n", tms, tdi);
                    x = 0;
                }
            }
            if (x > 0)
                DEBUG ("\t%s\n\t%s\n\n", tms, tdi);
#endif
#ifdef DSP_SCAN_CAUSE
            DEBUG ("Scan Output because ");
            switch (scan_out)
            {
            case 1: DEBUG ("flush to get TDO set"); break;
            case 2: DEBUG ("need to read data back"); break;
            case 3: DEBUG ("flush is to be complete"); break;
            case 4: DEBUG ("state is Run-Test-Idle"); break;
            case 5: DEBUG ("state is Test-Logic-Reset"); break;
            case 6: DEBUG ("there's a lot of data"); break;
            case 7: DEBUG ("there's a lot of data & in a reset or idle state"); break
            default: DEBUG ("Duh?"); break;
            }
#endif
            tdo_ptr = NULL;
            perform_scan (cable, &tdo_ptr);
        }
#ifdef DSP_SCAN_DATA
        else
            DEBUG ("Skipping sending out scan, Idx = %d\n", tap_info->cur_idx);
#endif
        /* Here we send out the scan/cmd
         * need to look at how_much */
        while (j != i)
        {
            todo_data = &ptr_todo->data[j];

            switch (todo_data->action)
            {   /* Pick up data if need be */
            case URJ_TAP_CABLE_CLOCK:
                /* Nothing needs to be done */
                break;
            case URJ_TAP_CABLE_GET_TDO:
                {   /* get last bit */
                    k = urj_tap_cable_add_queue_item (cable, &cable->done);
                    done_data = &ptr_done->data[k];

                    done_data->action = URJ_TAP_CABLE_GET_TDO;
                    if (tdo_ptr && (tdo_idx != -1) && (tdo_idx <= tap_info->cur_dat))
                    {
                        int32_t dat_idx = tap_info->dat[tdo_idx].idx;
                        int32_t bit_set = tap_info->dat[tdo_idx].pos;

                        done_data->arg.value.val = (tdo_ptr[dat_idx + cable_params->tap_pair_start_idx] & bit_set) ? 1 : 0;
                        bit_set >>= 1;

                        if (!bit_set)
                        {
                            bit_set = 0x80;
                            dat_idx++;
                        }
                        tap_info->dat[tdo_idx].idx = dat_idx;
                        tap_info->dat[tdo_idx].pos = bit_set;
                    }
                    else
                    {
                        done_data->arg.value.val = (tdo_ptr) ? 1 : 0;
                    }
                }
                break;
            case URJ_TAP_CABLE_SET_SIGNAL:
                /* not currently used, if it will be, revisit */
                cable->driver->set_signal (cable, 0xff, todo_data->arg.value.sig);
                break;
            case URJ_TAP_CABLE_GET_SIGNAL:
                /* not currently used, if it will be, revisit */
                {
                    int32_t k = urj_tap_cable_add_queue_item (cable, &cable->done);
                    done_data = &ptr_done->data[k];

                    done_data->action = URJ_TAP_CABLE_GET_SIGNAL;
                    done_data->arg.value.sig = cable->driver->get_signal (cable, done_data->arg.value.sig);
                }
                break;
            case URJ_TAP_CABLE_TRANSFER:
                /* set up the get data */
                free (todo_data->arg.transfer.in);
                todo_data->arg.transfer.in = NULL;
                if ((todo_data->arg.transfer.out != NULL) && (tdo_ptr != NULL))
                {
                    int32_t k = urj_tap_cable_add_queue_item (cable, &cable->done);
                    done_data = &ptr_done->data[k];

                    get_recv_data (cable, j, tap_info->rcv_dat, &tdo_ptr);
                    tap_info->rcv_dat++;
                    done_data->action = URJ_TAP_CABLE_TRANSFER;
                    done_data->arg.xferred.len = todo_data->arg.transfer.len;
                    done_data->arg.xferred.res = 0;
                    done_data->arg.xferred.out = todo_data->arg.transfer.out;
                    tdo_idx++;
                    scan_out++;
                }
                break;
            default:
                DEBUG("default; j = %d, n = %d - other end\n", j, n);
                break;
            }

            j++;
            if (j >= ptr_todo->max_items)
                j = 0;

            ptr_todo->num_items--;
        }

        ptr_todo->next_item = i;
    }

    /* need to free memory */
    if (tdo_ptr)
    {
        free (tdo_ptr);
        tdo_ptr = NULL;
    }

    if (scan_out > 0)
    {
        if (tap_info->pairs)
        {
            free (tap_info->cmd);
            tap_info->pairs = NULL;
            tap_info->cmd = NULL;
        }
        tap_info->total = 0;
        tap_info->cur_idx = 0;
        tap_info->bit_pos = 0;
        tap_info->cur_dat = -1;
        tap_info->rcv_dat = -1;
    }
}

/*
 * Get TRST state if Firmware version supports it
 * TODO: Since when developing this product,
 *       Functionality has changed.  But at this point,
 *       we only get TRST
 */
static int ice_get_sig (urj_cable_t *cable, urj_pod_sigsel_t sig)
{
    params_t *cable_params = cable->params;
    if (cable_params->version < 0x0106)
        return URJ_STATUS_FAIL;
    else
        return do_host_cmd (cable, HOST_GET_TRST, 0, 1);
}

/*
 * Set TRST state if Firmware version supports it
 * TODO: Since when developing this product,
 *       Functionality has changed.  But at this point,
 *       we only set TRST
 */
static int ice_set_sig (urj_cable_t *cable, int mask, int val)
{
    params_t *cable_params = cable->params;
    if (cable_params->version < 0x0106)
    {
        urj_warning (_("Setting TRST is unavailable for Firmware Versions less than 1.0.6"));
        return URJ_STATUS_FAIL;
    }
    else
        return do_host_cmd (cable, HOST_SET_TRST, (val ? 1 : 0), 1);
}

/*
 * Takes Data received (rcv_dataptr) and puts it in
 * todo date out transfer
 */
static void get_recv_data (urj_cable_t *cable, int32_t idx, int32_t idx_dat, uint8_t **rcv_dataptr)
{
    params_t *cable_params = cable->params;
    int32_t len = cable->todo.data[idx].arg.transfer.len;
    char *buf = cable->todo.data[idx].arg.transfer.out;
    num_tap_pairs *tap_info = &cable_params->tap_info;
    int32_t dat_idx = tap_info->dat[idx_dat].idx;
    uint8_t *rcvBuf = (*rcv_dataptr) + cable_params->num_rcv_hdr_bytes+ dat_idx;
    int32_t bit_set = tap_info->dat[idx_dat].pos;
    int32_t i;

#ifdef DUMP_EACH_RCV_DATA
    DEBUG ("Idx = %d; Read len = %d\n", dat_idx, len);
#endif

    if ((buf == NULL) || (idx_dat < 0))
    {
        DEBUG("get_recv_data(): %s\n", (buf == NULL) ? "No output buffer" : "No Received Data");
        return;
    }

    for (i = 0; i < len; i++)
    {
        *buf++ = (*rcvBuf & bit_set) ? 1 : 0;

#ifdef DUMP_EACH_RCV_DATA
        DEBUG ("%d", (int32_t)*(buf - 1));
        if (((i + 1) % 64) == 0)
            putchar ('\n');
        else if (((i + 1) % 8) == 0)
            putchar (' ');
#endif
        bit_set >>= 1;

        if (!bit_set)
        {
            bit_set = 0x80;
            rcvBuf++;
            dat_idx++;
        }
    }

#ifdef DUMP_EACH_RCV_DATA
    if ((len & 1) == 1)
        DEBUG (" next bit is %d\n", (rcvBuf[dat_idx] & bit_set) ? 1 : 0);
    else
        putchar ('\n');
#endif

    /* this is set for getting the extra TDO bits */
    tap_info->dat[idx_dat].idx = dat_idx;
    tap_info->dat[idx_dat].pos = bit_set;
}

/*
 * This function takes CABLE_TRANSFER todo data,
 * and adds it to the tms/tdi scan structure
 * If reading data, sets that up too
 */
static int add_scan_data (urj_cable_t *cable, int32_t num_bits, char *in, char *out)
{
    params_t *cable_params = cable->params;
    int32_t bit_cnt  = num_bits % 8;
    int32_t byte_cnt = (num_bits >> 3) + (bit_cnt ? 1 : 0);
    int32_t i, bit_set;
    tap_pairs *tap_scan = NULL;
    int32_t idx;
    num_tap_pairs *tap_info = &cable_params->tap_info;

    if (in == NULL)
        urj_warning (_("NO IN DATA!!!%s"), out ? _(" BUT there is out data!") : "");

    if (tap_info->pairs == NULL)
    {   /* really should never get here, but must not crash system. Would be rude */
        int32_t new_sz = cable_params->default_scanlen + 4;
        unsigned char *cmd;

        cmd = malloc ((sizeof (tap_pairs) * new_sz) + 1 + cable_params->tap_pair_start_idx);
        if (cmd == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                           (sizeof (tap_pairs) * new_sz) + 1 + cable_params->tap_pair_start_idx);
            return URJ_STATUS_FAIL;
        }

        tap_info->cur_dat = -1;
        tap_info->rcv_dat = -1;
        tap_info->bit_pos = 0x80;
        tap_info->total = new_sz;
        tap_scan = tap_info->pairs = (tap_pairs *)(cmd + cable_params->tap_pair_start_idx);    /* new pointer */
        tap_info->cmd = cmd;    /* new pointer */
        tap_scan->tms = 0;
        tap_scan->tdi = 0;
        idx = tap_info->cur_idx = 1;    /* first pair is 0 */
        tap_scan++;
        tap_scan->tdi = 0;
        tap_scan->tms = 0;
    }
    else if ((tap_info->total - tap_info->cur_idx) < byte_cnt)
    {   /* to small, increase size! */
        unsigned char *cmd;
        int32_t new_sz;

        DEBUG("Reallocating scan_data\n");

        new_sz = tap_info->total + byte_cnt + 8;
        cmd = realloc (tap_info->cmd, (sizeof (tap_pairs) * new_sz) + 4 + cable_params->tap_pair_start_idx);
        if (cmd == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("realloc(%zd) fails"),
                           (sizeof (tap_pairs) * new_sz) + 4 + cable_params->tap_pair_start_idx);
            return URJ_STATUS_FAIL;
        }

        tap_info->total = new_sz;        /* resize size */
        tap_scan = tap_info->pairs = (tap_pairs *)(cmd + cable_params->tap_pair_start_idx);    /* new pointer */
        tap_info->cmd = cmd;            /* new pointer */
        idx = tap_info->cur_idx;         /* to add on */
        tap_scan = &tap_info->pairs[idx];
    }
    else
    {
        idx = tap_info->cur_idx;            /* to add on */
        tap_scan = &tap_info->pairs[idx];
    }

    bit_set = tap_info->bit_pos;

    if (out)
    {   /* Setup where we start to read, can be more than 1 */
        if (tap_info->rcv_dat == -1)
        {
            tap_info->rcv_dat = 0;
        }
        tap_info->cur_dat++;
        if (tap_info->cur_dat >= tap_info->num_dat)
        {
            int32_t new_sz;
            dat_dat *datPtr;

            new_sz = tap_info->num_dat + DAT_SZ_INC;
            datPtr = realloc (tap_info->dat, sizeof (dat_dat) * new_sz);
            if (datPtr == NULL)
            {
                urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("realloc(%zd) fails"),
                               sizeof (dat_dat) * new_sz);
                return URJ_STATUS_FAIL;
            }
            tap_info->dat = datPtr;
            tap_info->num_dat = new_sz;

        }
        tap_info->dat[tap_info->cur_dat].idx = idx;
        tap_info->dat[tap_info->cur_dat].pos = bit_set;
    }

    /* Build Scan.  TMS will always be zero! */
    for (i = 0; i < num_bits; i++, in++)
    {
        tap_scan->tdi |= *in ? bit_set : 0;
        bit_set >>= 1;
        if (!bit_set)
        {
            bit_set = 0x80;
            idx++;
            tap_scan++;
            tap_scan->tdi = 0;
            tap_scan->tms = 0;
        }
    }

    tap_info->cur_idx = idx;
    tap_info->bit_pos = bit_set;

    return URJ_STATUS_OK;
}

/*
 * This function takes CABLE_CLOCK todo data,
 * and builds the tms/tdi scan structure
 */
static int build_clock_scan (urj_cable_t *cable, int32_t *start_idx, int32_t *num_todo_items)
{
    params_t *cable_params = cable->params;
    num_tap_pairs *tap_info = &cable_params->tap_info;
    urj_cable_queue_info_t *ptr_todo = &cable->todo;
    tap_pairs *tap_scan = NULL;
    urj_cable_queue_t *scan_data = NULL;
    int32_t i, n, idx = tap_info->cur_idx, cur_idx = *start_idx, bit_set;

    if (tap_info->pairs == NULL)
    {   /* Allocate tap_scan */
        int32_t new_sz = cable_params->default_scanlen + 4;
        unsigned char *cmd;

        cmd = malloc ((sizeof (tap_pairs) * new_sz) + 1 + cable_params->tap_pair_start_idx);
        if (cmd == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                           (sizeof (tap_pairs) * new_sz) + 1 + cable_params->tap_pair_start_idx);
            return URJ_STATUS_FAIL;
        }

        tap_info->total = new_sz;
        tap_info->bit_pos = 0x80;
        tap_info->cur_dat = -1;
        tap_info->rcv_dat = -1;
        tap_info->cmd = cmd;
        tap_info->pairs = (tap_pairs *)(cmd + cable_params->tap_pair_start_idx);    /* new pointer */

        tap_scan = tap_info->pairs;
        tap_scan->tms = 0;
        tap_scan->tdi = 0;
        tap_scan++;
        tap_scan->tms = 0;
        tap_scan->tdi = 0;
        idx = tap_info->cur_idx = 1;            /* put NULL for first one */
    }
    else if ((tap_info->total - tap_info->cur_idx) < 5)
    {   /* need more memory, really should never get here */
        int32_t new_sz = tap_info->total + 20;
        unsigned char *cmd;

        DEBUG("Reallocating ClockScan\n");

        cmd = realloc (tap_info->cmd, (sizeof (tap_pairs) * new_sz) + 4);
        if (cmd == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("realloc(%zd) fails"),
                           (sizeof (tap_pairs) * new_sz) + 4);
            return URJ_STATUS_FAIL;
        }
        tap_info->cmd = cmd;
        tap_info->pairs = (tap_pairs *)(cmd + cable_params->tap_pair_start_idx);    /* new pointer */

        tap_scan = tap_info->pairs;
        idx = tap_info->cur_idx;
        tap_scan = &tap_scan[idx];
        tap_info->total = new_sz;
    }
    else
    {
        idx = tap_info->cur_idx;
        tap_scan = &tap_info->pairs[idx];
    }

    bit_set = tap_info->bit_pos;
    scan_data = &ptr_todo->data[cur_idx];

    for (n = *num_todo_items; (n < ptr_todo->num_items) && (scan_data->action == URJ_TAP_CABLE_CLOCK); n++)
    {   /* for each CABLE_CLOCK todo entry, create scan */
        for (i = 0; i < scan_data->arg.clock.n; i++)
        {
            tap_scan->tms |= scan_data->arg.clock.tms ? bit_set : 0;
            tap_scan->tdi |= scan_data->arg.clock.tdi ? bit_set : 0;
            bit_set >>= 1;
            if (!bit_set)
            {
                bit_set = 0x80;
                idx++;            /* filled up, go to the next one */
                tap_scan++;
                tap_scan->tms = 0;
                tap_scan->tdi = 0;
            }
        }
        cur_idx++;
        if (cur_idx >= ptr_todo->max_items)
        {   /* wrap around buffer! */
            cur_idx = 0;
        }
        scan_data = &ptr_todo->data[cur_idx];
    }

    tap_info->cur_idx = idx;
    tap_info->bit_pos = bit_set;
    *start_idx = ((cur_idx == 0) ? ptr_todo->max_items : cur_idx) - 1;
    *num_todo_items = n - 1;

    return URJ_STATUS_OK;
}

/*
 * Read & Write Registers
 *
 * XXX: error handling doesn't quite work with this return
 * XXX: probably needs converting from memory arrays to byte shifts
 *      so we work regardless of host endian
 */
static uint32_t do_single_reg_value (urj_cable_t *cable, uint8_t reg, int32_t r_data, int32_t wr_data, uint32_t data)
{
    params_t *cable_params = cable->params;
    usb_command_block usb_cmd_blk;
    union {
        uint8_t b[24];
        uint32_t l[6];
    } cmd_buffer;
    uint32_t count = 0;
    int32_t i, size = wr_data ? 8 : 4;

    usb_cmd_blk.command = HOST_REQUEST_TX_DATA;
    usb_cmd_blk.count = size;
    usb_cmd_blk.buffer = 0;

    adi_usb_write_or_ret (cable->link.usb->params, &usb_cmd_blk, sizeof (usb_cmd_blk));
    i = 0;

    /* send HOST_SET_SINGLE_REG command */
    cmd_buffer.b[i++] = 1;
    cmd_buffer.b[i++] = 0;
    cmd_buffer.b[i++] = wr_data ? HOST_SET_SINGLE_REG : HOST_GET_SINGLE_REG;
    cmd_buffer.b[i++] = reg;
    if (wr_data)
    {
        cmd_buffer.l[i / 4] = data;
    }

    adi_usb_write_or_ret (cable->link.usb->params, cmd_buffer.b, size);

    if (r_data)
        adi_usb_read_or_ret (cable->link.usb->params, &count, sizeof (count));

    return count;
}

/*
 * Send Host Command.
 *
 * XXX: error handling doesn't quite work with this return
 * XXX: probably needs converting from memory arrays to byte shifts
 *      so we work regardless of host endian
 */
static uint16_t do_host_cmd (urj_cable_t *cable, uint8_t cmd, uint8_t param, int32_t r_data)
{
    params_t *cable_params = cable->params;
    usb_command_block usb_cmd_blk;
    uint16_t results = 0;
    union {
        uint8_t b[20];
        uint32_t l[20/4];
    } cmd_buffer;
    int32_t i, size = 4;

    usb_cmd_blk.command = HOST_REQUEST_TX_DATA;
    usb_cmd_blk.count = 4;
    usb_cmd_blk.buffer = 0;

    adi_usb_write_or_ret (cable->link.usb->params, &usb_cmd_blk, sizeof (usb_cmd_blk));
    i = 0;

    /* send command */
    cmd_buffer.b[i++] = param;
    cmd_buffer.b[i++] = 0;
    cmd_buffer.b[i++] = cmd;
    cmd_buffer.b[i] = 0;

    adi_usb_write_or_ret (cable->link.usb->params, cmd_buffer.b, size);

    if (r_data)
    {
        usb_cmd_blk.command = HOST_REQUEST_RX_DATA;
        usb_cmd_blk.count = 2;
        usb_cmd_blk.buffer = 0;

        adi_usb_write_or_ret (cable->link.usb->params, &usb_cmd_blk, sizeof (usb_cmd_blk));

        adi_usb_read_or_ret (cable->link.usb->params, &results, sizeof (results));
    }

    return results;
}

/*
 *    Controlling function to do a scan.
 *    rdata is a pointer to storage for the pointer
 *    allocated here to return data if needed
 */
static int perform_scan (urj_cable_t *cable, uint8_t **rdata)
{
    params_t *cable_params = cable->params;
    num_tap_pairs *tap_info = &cable_params->tap_info;
    uint8_t firstpkt = 1, lastpkt = 0, *in = NULL, *out = NULL;
    int32_t idx, collect_data = 0;
    int32_t cur_len = cable_params->tap_info.cur_idx;
    int32_t rem_len;

    /* Data is scan as 32 bit words, so boundaries are adjusted here */
    if (tap_info->bit_pos != 0x80) /* meaning no dangling bits? */
    {   /* yes, so straighten out! */
        cur_len++;
        tap_info->pairs[cur_len].tms = 0;
        tap_info->pairs[cur_len].tdi = 0;
    }

    /* Pad with zeros */
    cur_len++;
    tap_info->pairs[cur_len].tms = 0;
    tap_info->pairs[cur_len].tdi = 0;

    while (cur_len & 0x03)
    {   /* expect to be in 32 bit words */
        cur_len++;
        tap_info->pairs[cur_len].tms = 0;
        tap_info->pairs[cur_len].tdi = 0;
    }

    tap_info->cur_idx = cur_len;
    rem_len = cur_len * sizeof (tap_pairs);

    if (cur_len > cable_params->default_scanlen)
    {
        urj_log (URJ_LOG_LEVEL_ERROR,
                 _("%s: TAP Scan length %d is greater than DIF Memory"),
                 __func__, tap_info->cur_idx);
        return URJ_STATUS_FAIL;
    }

    if (tap_info->cur_dat != -1)
    {   /* yes we have data, so allocate for data plus header */
        size_t len;

        len = cur_len + cable_params->tap_pair_start_idx + 16;
        if (tap_info->dat[0].idx > 12)
            len -= tap_info->dat[0].idx;

        out = malloc (len);
        if (out == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%zd) fails"),
                           len);
            return URJ_STATUS_FAIL;
        }
        *rdata = out;
        collect_data = 1;
    }
    else
    {   /* no data, so allocate for just header */
        out = malloc (cable_params->tap_pair_start_idx + 16);
        if (out == NULL)
        {
            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("malloc(%d) fails"),
                           cable_params->tap_pair_start_idx + 16);
            return URJ_STATUS_FAIL;
        }
        collect_data = 0;
    }

    in = (uint8_t *)tap_info->pairs;
    idx = 0;

    /* Here if data is too large, we break it up into manageable chunks */
    do
    {
        cur_len = (rem_len >= cable_params->max_raw_data_tx_items) ? cable_params->max_raw_data_tx_items : rem_len;

        if (cur_len == rem_len)
            lastpkt = 1;

        do_rawscan (cable, firstpkt, lastpkt, collect_data, cur_len, &in[idx] - cable_params->tap_pair_start_idx, out);

        rem_len -= cur_len;
        idx += cur_len;
        firstpkt = 0;

    } while (rem_len);

    if (tap_info->dat[0].idx == -1)
    {   /* no data to return, so free it */
        free (out);
    }

    return URJ_STATUS_OK;
}

/*
 *    description of raw scan packet structure:
 *
 *        [0]        : first packet flag (do setup work if needed)
 *        [1]        : last packet flag (start the scan and cleanup if needed)
 *        [2]        : command ID
 *        [3]        : collect DOF flag (need to read DOF)
 *        [4-5]      : DIF count
 *        [6-7]      : scan length count
 *        [8-9]      : first scan pair
 *        [10...]    : more scan pairs
 *
 *    Data input:
 *
 *        firstpkt   : Is this the first packet of the scan? 0 = NO
 *        lastpkt    : Is this the last packet of the scan? 0 = NO
 *        collect_dof : Are we collecting data?  0 = NO
 *        dif_cnt   : Number of bytes to send
 *        *raw_buf    : Pointer to Scan Data buffer * cmd to send
 *        *out       : Pointer to Scan Data buffer to receive
 *
 * XXX: probably needs converting from memory arrays to byte shifts
 *      so we work regardless of host endian
 */
static int do_rawscan (urj_cable_t *cable, uint8_t firstpkt, uint8_t lastpkt,
                       int32_t collect_dof, int32_t dif_cnt, uint8_t *raw_buf,
                       uint8_t *out)
{
    params_t *cable_params = cable->params;
    usb_command_block usb_cmd_blk;
    num_tap_pairs *tap_info = &cable_params->tap_info;
    int32_t i, dof_start = 0;
    uint32_t data;
    uint32_t size = cable_params->tap_pair_start_idx + dif_cnt;

    usb_cmd_blk.command = HOST_REQUEST_TX_DATA;
    usb_cmd_blk.count = size;
    usb_cmd_blk.buffer = 0;

    /* first send Xmit request with the count of what will be sent */
    adi_usb_write_or_ret (cable->link.usb->params, &usb_cmd_blk, sizeof (usb_cmd_blk));
    i = 0;

    /* send HOST_DO_SELECTIVE_RAW_SCAN command */
    raw_buf[i++] = firstpkt;
    raw_buf[i++] = lastpkt;
    raw_buf[i++] = HOST_DO_SELECTIVE_RAW_SCAN;
    if ((collect_dof && lastpkt) && (tap_info->dat[0].idx > 12))
    {
        int32_t j, offset;

        dof_start = tap_info->dat[0].idx;
        offset = dof_start & 7;
        dof_start -= offset & 7;
        tap_info->dat[0].idx = offset;

        for (j = 1; j <= tap_info->cur_dat; j++)
        {
            tap_info->dat[j].idx -= dof_start;
        }
    }

    raw_buf[i++] = collect_dof ? 1 : 0;
    data = dif_cnt / 4;         /* dif count in longs */
    memcpy (raw_buf + i, &data, 4);
    data = tap_info->cur_idx / 4;  /* count in longs */
    memcpy (raw_buf + i + 2, &data, 4);

    /* only Ice emulators use this */
    memcpy (raw_buf + i + 4, &dof_start, 4);

    adi_usb_write_or_ret (cable->link.usb->params, raw_buf, size);

    if (lastpkt)
    {
        int32_t cur_rd_bytes = 0, tot_bytes_rd = 0, rd_bytes_left;

        rd_bytes_left = cable_params->num_rcv_hdr_bytes + ((collect_dof) ? (tap_info->cur_idx - dof_start) : 0);

        while (tot_bytes_rd < rd_bytes_left)
        {
            cur_rd_bytes = ((rd_bytes_left - tot_bytes_rd) > cable_params->r_buf_sz) ?
                cable_params->r_buf_sz : (rd_bytes_left - tot_bytes_rd);

            adi_usb_read_or_ret (cable->link.usb->params, out + tot_bytes_rd, cur_rd_bytes);
            tot_bytes_rd += cur_rd_bytes;
        }

        if (out[0] != 2)
        {
            urj_log (URJ_LOG_LEVEL_ERROR, _("%s: Scan Error!"), __func__);
            return URJ_STATUS_FAIL;
        }
    }

    return URJ_STATUS_OK;
}

static void
ice_cable_help (urj_log_level_t ll, const char *cablename)
{
    const char *ex_short = "[firmware=FILE]";
    const char *ex_desc = "FILE       Upgrade the ICE firmware.  See this page:\n"
        "           " ICE100B_DOC_URL "\n";
    urj_tap_cable_generic_usbconn_help_ex (ll, cablename, ex_short, ex_desc);
}

/*
 * Cable Intefaces
 */

/* This is for the ICE-100B emulator */
const urj_cable_driver_t urj_tap_cable_ice100B_driver = {
    "ICE-100B",
    N_("Analog Devices ICE-X Cable (0x064B)"),
    URJ_CABLE_DEVICE_USB,
    { .usb = ice_connect, },
    urj_tap_cable_generic_disconnect,
    ice_cable_free,
    ice_init,
    urj_tap_cable_generic_usbconn_done,
    ice100b_set_freq,
    adi_clock,
    adi_get_tdo,
    adi_transfer,
    ice_set_sig,
    ice_get_sig,
    adi_flush,
    ice_cable_help,
    URJ_CABLE_QUIRK_ONESHOT
};
URJ_DECLARE_USBCONN_CABLE(0x064B, 0x1225, "libusb", "ICE-100B", ice100B)
URJ_DECLARE_USBCONN_CABLE(0x064B, 0x0225, "libusb", "ICE-100B", ice100Bw)

/*
 Local Variables:
 mode:C
 c-default-style:gnu
 indent-tabs-mode:nil
 End:
*/
