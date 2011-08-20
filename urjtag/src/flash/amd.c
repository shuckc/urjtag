/*
 * $Id$
 *
 * Flash driver for AMD Am29LV640D, Am29LV641D, Am29LV642D
 * Copyright (C) 2003 AH
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
 * Written by August Hörandl <august.hoerandl@gmx.at>
 * Modified by Marcel Telka <marcel@telka.sk>, 2003.
 *
 * Documentation:
 * [1] Advanced Micro Devices, "Am29LV640D/Am29LV641D",
 *     September 20, 2002     Rev B, 22366b8.pdf
 * [2] Advanced Micro Devices, "Am29LV642D",
 *     August 14, 2001    Rev A, 25022.pdf
 * [3] Spansion, "S29GL-N MirrorBit Flash Family"
 *     October 13, 2006    Rev B, Amendment 3
 *
 */

#include <sysdep.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>     /* usleep */

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>

#include "flash.h"
#include "cfi.h"
#include "amd.h"


/* The code below assumes a connection of the flash chip address LSB (A0)
 * to A0, A1 or A2 of the byte-addressed CPU bus dependent on the bus width.
 *
 *     8 Bit devices: A0..Ax connected to A0..Ax of CPU bus
 *  8/16 Bit devices: A0..Ax connected to A1..Ax+1 of CPU bus
 *    16 Bit devices: A0..Ax connected to A1..Ax+1 of CPU bus
 * 16/32 Bit devices: A0..Ax connected to A2..Ax+2 of CPU bus
 *    32 Bit devices: A0..Ax connected to A2..Ax+2 of CPU bus
 *
 * The offset computed by amd_flash_address_shift()  is used here dependent on
 * the bus width (8, 16 or 32 bit) to align the patterns emitted on the
 * address lines at either A0, A1 or A2. */

/* NOTE: It does not work for SoC chips or boards with extra address decoders
 * that do address alignment themselves, such as the Samsung S3C4510B. The bus
 * driver has to deal with this. - kawk 2008-01 */

static int
amd_flash_address_shift (urj_flash_cfi_array_t *cfi_array)
{
    if (cfi_array->bus_width == 4)
        return 2;

    /* else: cfi_array->bus_width is 2 (16 bit) or 1 (8 bit): */

    switch (cfi_array->cfi_chips[0]->cfi.device_geometry.device_interface)
    {
    case CFI_INTERFACE_X8_X16: /* regardless whether 8 or 16 bit mode */
    case CFI_INTERFACE_X16:    /* native */
        return 1;

    case CFI_INTERFACE_X16_X32:        /* e.g. 32 bit flash in 16 bit mode */
    case CFI_INTERFACE_X32:    /* unlikely */
        return 2;

    default:
        break;
    }

    if (cfi_array->bus_width == 2)
        return 1;

    return 0;
}

/* autodetect, we can handle this chip */
static int
amd_flash_autodetect32 (urj_flash_cfi_array_t *cfi_array)
{
    if (cfi_array->bus_width != 4)
        return 0;
    return (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
            == CFI_VENDOR_AMD_SCS);
}

static int
amd_flash_autodetect16 (urj_flash_cfi_array_t *cfi_array)
{
    if (cfi_array->bus_width != 2)
        return 0;
    return (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
            == CFI_VENDOR_AMD_SCS);
}

static int
amd_flash_autodetect8 (urj_flash_cfi_array_t *cfi_array)
{
    if (cfi_array->bus_width != 1)
        return 0;
    return (cfi_array->cfi_chips[0]->cfi.identification_string.pri_id_code
            == CFI_VENDOR_AMD_SCS);
}

/*
 * check device status
 *   URJ_STATUS_OK   PASS
 *   URJ_STATUS_FAIL FAIL
 */
/*
 * first implementation: see [1], page 29
 */
#if 0
static int
amdstatus29 (urj_flash_cfi_array_t *cfi_array, uint32_t adr,
             int data)
{
    urj_bus_t *bus = cfi_array->bus;
    int o = amd_flash_address_shift (cfi_array);
    int timeout;
    uint32_t dq7mask = ((1 << 7) << 16) + (1 << 7);
    uint32_t dq5mask = ((1 << 5) << 16) + (1 << 5);
    uint32_t bit7 = (data & (1 << 7)) != 0;
    uint32_t data1;

    for (timeout = 0; timeout < 100; timeout++)
    {
        data1 = URJ_BUS_READ (bus, adr << o);
        data1 = URJ_BUS_READ (bus, adr << o);
        urj_log (URJ_LOG_LEVEL_DEBUG, "amdstatus %d: %04lX (%04lX) = %04lX\n",
                 timeout, (long unsigned) data1,
                 (long unsigned) (data1 & dq7mask), (long unsigned) bit7);
        if (((data1 & dq7mask) == dq7mask) == bit7)     /* FIXME: This looks non-portable */
            return URJ_STATUS_OK;

        if ((data1 & dq5mask) == dq5mask)
            break;
        usleep (100);
    }

    data1 = URJ_BUS_READ (bus, adr << o);
    if (((data1 & dq7mask) == dq7mask) == bit7) /* FIXME: This looks non-portable */
        return URJ_STATUS_OK;

    urj_error_set (URJ_ERROR_FLASH, "hardware failure");
    return URJ_STATUS_FAIL;
}
#endif /* 0 */


#if 1
/*
 * second implementation: see [1], page 30
 */
static int
amdstatus (urj_flash_cfi_array_t *cfi_array, uint32_t adr, int data)
{
    urj_bus_t *bus = cfi_array->bus;

    int timeout;
    uint32_t togglemask = ((1 << 6) << 16) + (1 << 6);  /* DQ 6 */
    /*  int dq5mask = ((1 << 5) << 16) + (1 << 5); DQ5 */

    for (timeout = 0; timeout < 7000; timeout++)
    {
        uint32_t data1 = URJ_BUS_READ (bus, adr);
        uint32_t data2 = URJ_BUS_READ (bus, adr);

        urj_log (URJ_LOG_LEVEL_DEBUG,
                 "amdstatus %d: %04lX/%04lX   %04lX/%04lX \n",
                 timeout, (long unsigned) data1, (long unsigned) data2,
                 (long unsigned) (data1 & togglemask),
                 (long unsigned) (data2 & togglemask));
        if ((data1 & togglemask) == (data2 & togglemask))
            return URJ_STATUS_OK;

        /*    if ( (data1 & dq5mask) != 0 )   TODO */
        /*      return URJ_STATUS_OK; */
        urj_log (URJ_LOG_LEVEL_DEBUG, "amdstatus %d: %04lX/%04lX\n",
                 timeout, (long unsigned)data1, (long unsigned)data2);
        usleep (100);
    }

    urj_error_set (URJ_ERROR_FLASH, "hardware failure");
    return URJ_STATUS_FAIL;
}

#else /* 1 */

/* Note: This implementation of amdstatus() has been added by patch
         [ 1429825 ] EJTAG driver (some remaining patch lines for flash/amd.c)
         It's a quirk workaround and seems to break status polling for other chips.
         Therefore it's deactivated at the moment but kept for reference. */
/*
 * second implementation: see [1], page 30
 */
static int
amdstatus (urj_flash_cfi_array_t *cfi_array, uint32_t adr, int data)
{
    urj_bus_t *bus = cfi_array->bus;
    int o = amd_flash_address_shift (cfi_array);
    int timeout;
    uint32_t togglemask = ((1 << 6) << 16) + (1 << 6);  /* DQ 6 */
    /*  int dq5mask = ((1 << 5) << 16) + (1 << 5); DQ5 */
    uint32_t data1, data2;

    data1 = URJ_BUS_READ (bus, adr);
    for (timeout = 0; timeout < 100; timeout++)
    {
        data2 = URJ_BUS_READ (bus, adr);


        urj_log (URJ_LOG_LEVEL_DEBUG,
                 "amdstatus %d: %04lX/%04lX   %04lX/%04lX \n",
                 timeout, (long unsigned) data1, (long unsigned) data2,
                 (long unsigned) (data1 & togglemask),
                 (long unsigned) (data2 & togglemask));
        /* Work around an issue with RTL8181: toggle bits don't
           toggle when reading the same flash address repeatedly
           without any other memory access in between.  Other
           bits reflect the current status, and data after the
           operation is complete - only Q6/Q2 bits don't toggle
           when they should.  Looks like the CPU not deasserting
           CE or OE, so data is output to the bus continuously.
           So, check for the correct data read twice instead.  */
        /*if ( (data1 & togglemask) == (data2 & togglemask)) */
        if ((data1 == data) && (data2 == data))
            return URJ_STATUS_OK;

        /*    if ( (data1 & dq5mask) != 0 )   TODO */
        /*      return URJ_STATUS_OK; */
        if (urj_log_status.level <= URJ_LOG_LEVEL_DEBUG)
            urj_log (URJ_LOG_LEVEL_DEBUG, "amdstatus %d: %04lX/%04lX\n",
                     timeout, (long unsigned) data1, (long unsigned) data2);
        else
            urj_log (URJ_LOG_LEVEL_NORMAL, ".");
        usleep (100);
        data1 = data2;
    }

    urj_error_set (URJ_ERROR_FLASH, "hardware failure");
    return URJ_STATUS_FAIL;
}

#endif /* 0 */

static void
amd_flash_read_array (urj_flash_cfi_array_t *cfi_array)
{
    /* Read Array */
    URJ_BUS_WRITE (cfi_array->bus, cfi_array->address, 0x00F000F0); /* AMD reset */
}

#if 0
static int
amdisprotected (parts * ps, urj_flash_cfi_array_t *cfi_array,
                uint32_t adr)
{
    uint32_t data;
    int o = amd_flash_address_shift (cfi_array);

    URJ_BUS_WRITE (ps, cfi_array->address + (0x0555 << o), 0x00aa00aa);       /* autoselect p29, sector erase */
    URJ_BUS_WRITE (ps, cfi_array->address + (0x02aa << o),
                   0x00550055);
    URJ_BUS_WRITE (ps, cfi_array->address + (0x0555 << o),
                   0x00900090);

    data = URJ_BUS_READ (ps, adr + (0x0002 << 2));
    /* Read Array */
    amd_flash_read_array (ps);  /* AMD reset */

    return ((data & 0x00ff00ff) != 0);
}
#endif /* 0 */

static void
amd_flash_print_info (urj_log_level_t ll, urj_flash_cfi_array_t *cfi_array)
{
    int mid, cid, prot;
    urj_bus_t *bus = cfi_array->bus;
    int o = amd_flash_address_shift (cfi_array);

    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00aa00aa);      /* autoselect p29 */
    URJ_BUS_WRITE (bus, cfi_array->address + (0x02aa << o), 0x00550055);
    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00900090);
    mid = URJ_BUS_READ (bus, cfi_array->address + (0x00 << o)) & 0xFFFF;
    cid = URJ_BUS_READ (bus, cfi_array->address + (0x01 << o)) & 0xFFFF;
    prot = URJ_BUS_READ (bus, cfi_array->address + (0x02 << o)) & 0xFF;
    amd_flash_read_array (cfi_array); /* AMD reset */
    urj_log (ll, _("Chip: AMD Flash\n\tManufacturer: "));
    switch (mid & 0xff)
    {
    case 0x0001:
        urj_log (ll, "AMD");
        urj_log (ll, _("\n\tChip: "));
        switch (cid)
        {
        case 0x0049:
            urj_log (ll, "AM29LV160DB");
            break;
        case 0x0093:
            urj_log (ll, "Am29LV065D");
            break;
        case 0x004F:
            urj_log (ll, "Am29LV040B");
            break;
        case 0x22D7:
            urj_log (ll, "Am29LV640D/Am29LV641D/Am29LV642D");
            break;
        case 0x225B:
            urj_log (ll, "Am29LV800B");
            break;
        case 0x227E:           /* 16-bit mode */
        case 0x007E:           /* 8-bit mode */
            urj_log (ll, "S92GLxxxN");
            break;
        default:
            urj_log (ll, _("Unknown (ID 0x%04x)"), cid);
            break;
        }
        break;
    case 0x001f:
        urj_log (ll, "Atmel");
        urj_log (ll, _("\n\tChip: "));
        switch (cid)
        {
        case 0x01c8:
            urj_log (ll, "AT49BV322D");
            break;
        case 0x01c9:
            urj_log (ll, "AT49BV322DT");
            break;
        case 0x01d2:
            urj_log (ll, "AT49BW642DT");
            break;
        case 0x01d6:
            urj_log (ll, "AT49BW642D");
            break;
        default:
            urj_log (ll, _("Unknown (ID 0x%04x)"), cid);
            break;
        }
        break;
    case 0x0020:
        urj_log (ll, "ST/Samsung");
        urj_log (ll, _("\n\tChip: "));
        switch (cid)
        {
        case 0x00ca:
            urj_log (ll, "M29W320DT");
            break;
        case 0x00cb:
            urj_log (ll, "M29W320DB");
            break;
        case 0x22ed:
            urj_log (ll, "M29W640DT");
            break;
        default:
            urj_log (ll, _("Unknown (ID 0x%04x)"), cid);
            break;
        }
        break;
    case 0x00C2:
        urj_log (ll, "Macronix");
        urj_log (ll, _("\n\tChip: "));
        switch (cid)
        {
        case 0x2249:
            urj_log (ll, "MX29LV160B");
            break;
        case 0x22a7:
            urj_log (ll, "MX29LV320CT");
            break;
        case 0x22a8:
            urj_log (ll, "MX29LV320CB");
            break;
        case 0x22CB:
            urj_log (ll, "MX29LV640B");
            break;
        default:
            urj_log (ll, _("Unknown (ID 0x%04x)"), cid);
            break;
        }
        break;
    case 0x00DA:
        urj_log (ll, "Winbond");
        urj_log (ll, _("\n\tChip: "));
        switch (cid & 0xff)
        {
        case 0x007E:
            urj_log (ll, "W19B320AT/B");
            break;
        default:
            urj_log (ll, _("Unknown (ID 0x%04x)"), cid);
            break;
        }
        break;
    default:
        urj_log (ll, _("Unknown manufacturer (ID 0x%04x) Chip (ID 0x%04x)"),
                 mid, cid);
        break;
    }
    urj_log (ll, _("\n\tProtected: %04x\n"), prot);

    /* Read Array */
    URJ_BUS_WRITE (bus, cfi_array->address + (0x0000 << o), 0x00ff00ff);
}

static int
amd_flash_erase_block (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    urj_bus_t *bus = cfi_array->bus;
    int o = amd_flash_address_shift (cfi_array);

    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_erase_block 0x%08lX\n",
             (long unsigned) adr);

    /*      urj_log (URJ_LOG_LEVEL_NORMAL, "protected: %d\n", amdisprotected(ps, cfi_array, adr)); */

    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00aa00aa);      /* autoselect p29, sector erase */
    URJ_BUS_WRITE (bus, cfi_array->address + (0x02aa << o), 0x00550055);
    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00800080);
    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00aa00aa);
    URJ_BUS_WRITE (bus, cfi_array->address + (0x02aa << o), 0x00550055);
    URJ_BUS_WRITE (bus, adr, 0x00300030);

    if (amdstatus (cfi_array, adr, 0xffff) == URJ_STATUS_OK)
    {
        urj_log (URJ_LOG_LEVEL_NORMAL, "flash_erase_block 0x%08lX DONE\n",
                 (long unsigned) adr);
        amd_flash_read_array (cfi_array);     /* AMD reset */
        return URJ_STATUS_OK;
    }
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_erase_block 0x%08lX FAILED\n",
             (long unsigned) adr);
    /* Read Array */
    amd_flash_read_array (cfi_array); /* AMD reset */

    urj_error_set (URJ_ERROR_FLASH_ERASE, "unknown erase error");
    return URJ_STATUS_FAIL;
}

static int
amd_flash_unlock_block (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_unlock_block 0x%08lX IGNORE\n",
             (long unsigned) adr);
    return URJ_STATUS_OK;
}

static int
amd_flash_lock_block (urj_flash_cfi_array_t *cfi_array, uint32_t adr)
{
    urj_log (URJ_LOG_LEVEL_NORMAL, "flash_lock_block 0x%08lX IGNORE\n",
             (long unsigned) adr);
    return URJ_STATUS_OK;
}

static int
amd_flash_program_single (urj_flash_cfi_array_t *cfi_array, uint32_t adr,
                          uint32_t data)
{
    int status;
    urj_bus_t *bus = cfi_array->bus;
    int o = amd_flash_address_shift (cfi_array);

    urj_log (URJ_LOG_LEVEL_DEBUG, "\nflash_program 0x%08lX = 0x%08lX\n",
             (long unsigned) adr, (long unsigned) data);

    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00aa00aa);      /* autoselect p29, program */
    URJ_BUS_WRITE (bus, cfi_array->address + (0x02aa << o), 0x00550055);
    URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00A000A0);

    URJ_BUS_WRITE (bus, adr, data);
    status = amdstatus (cfi_array, adr, data);
    /*      amd_flash_read_array(ps); */

    return status;
}

static int
amd_program_buffer_status (urj_flash_cfi_array_t *cfi_array, uint32_t adr,
                           uint32_t data)
{
    /* NOTE: Status polling according to [3], Figure 1.
       The current method for status polling is not compatible with 32 bit (2x16) configurations
       since it only checks the DQ7 bit of the lower chip. */
    urj_bus_t *bus = cfi_array->bus;
    int timeout;
    const uint32_t dq7mask = (1 << 7);
    const uint32_t dq5mask = (1 << 5);
    uint32_t bit7 = data & dq7mask;
    uint32_t data1;

    for (timeout = 0; timeout < 7000; timeout++)
    {
        data1 = URJ_BUS_READ (bus, adr);
        urj_log (URJ_LOG_LEVEL_DEBUG,
                 "amd_program_buffer_status %d: %04lX (%04lX) = %04lX\n",
                 timeout, (long unsigned) data1,
                 (long unsigned) (data1 & dq7mask), (long unsigned) bit7);
        if ((data1 & dq7mask) == bit7)
            return URJ_STATUS_OK;

        if ((data1 & dq5mask) == dq5mask)
            break;
        usleep (100);
    }

    data1 = URJ_BUS_READ (bus, adr);
    if ((data1 & dq7mask) == bit7)
        return URJ_STATUS_OK;

    return URJ_STATUS_FAIL;
}

static int
amd_flash_program_buffer (urj_flash_cfi_array_t *cfi_array, uint32_t adr,
                          uint32_t *buffer, int count)
{
    /* NOTE: Write buffer programming operation according to [3], Figure 1. */
    int status;
    urj_bus_t *bus = cfi_array->bus;
    urj_flash_cfi_chip_t *cfi_chip = cfi_array->cfi_chips[0];
    int o = amd_flash_address_shift (cfi_array);
    int wb_bytes = cfi_chip->cfi.device_geometry.max_bytes_write;
    int chip_width = cfi_chip->width;
    int offset = 0;

    urj_log (URJ_LOG_LEVEL_DEBUG,
             "\nflash_program_buffer 0x%08lX, count 0x%08X\n",
             (long unsigned) adr, count);

    while (count > 0)
    {
        int wcount, idx;
        uint32_t sa = adr;

        /* determine length of next multi-byte write */
        wcount = wb_bytes - (adr % wb_bytes);
        wcount /= chip_width;
        if (wcount > count)
            wcount = count;

        URJ_BUS_WRITE (bus, cfi_array->address + (0x0555 << o), 0x00aa00aa);
        URJ_BUS_WRITE (bus, cfi_array->address + (0x02aa << o), 0x00550055);
        URJ_BUS_WRITE (bus, adr, 0x00250025);
        URJ_BUS_WRITE (bus, sa, wcount - 1);

        /* write payload to write buffer */
        for (idx = 0; idx < wcount; idx++)
        {
            URJ_BUS_WRITE (bus, adr, buffer[offset + idx]);
            adr += cfi_array->bus_width;
        }
        offset += wcount;

        /* program buffer to flash */
        URJ_BUS_WRITE (bus, sa, 0x00290029);

        status = amd_program_buffer_status (cfi_array,
                                            adr - cfi_array->bus_width,
                                            buffer[offset - 1]);
        /*      amd_flash_read_array(ps); */
        if (status != URJ_STATUS_OK)
        {
            urj_error_set (URJ_ERROR_FLASH_PROGRAM, "status fails after write");
            return URJ_STATUS_FAIL;
        }

        count -= wcount;
    }

    return URJ_STATUS_OK;
}

static int
amd_flash_program (urj_flash_cfi_array_t *cfi_array, uint32_t adr,
                   uint32_t *buffer, int count)
{
    urj_flash_cfi_query_structure_t *cfi = &(cfi_array->cfi_chips[0]->cfi);
    int max_bytes_write = cfi->device_geometry.max_bytes_write;

#ifndef FLASH_MULTI_BYTE
    max_bytes_write = 1;
#endif

    /* multi-byte writes supported? */
    if (max_bytes_write > 1) {
        int result;
        result = amd_flash_program_buffer (cfi_array, adr, buffer, count);
        if (result == 0)
            return 0;

        /* Some flashes support max_bytes_write.
         * But some of them don't support S29 style write buffer.
         * See also the datasheet about AT49BV322D.
         */
        cfi->device_geometry.max_bytes_write = 1;
    }

    /* unroll buffer to single writes */
    int idx;
    for (idx = 0; idx < count; idx++)
    {
        int status = amd_flash_program_single (cfi_array, adr, buffer[idx]);
        if (status != URJ_STATUS_OK)
            return status;
        adr += cfi_array->bus_width;
    }

    return URJ_STATUS_OK;
}

static int
amd_flash_program32 (urj_flash_cfi_array_t *cfi_array, uint32_t adr,
                     uint32_t *buffer, int count)
{
    /* Single byte programming is forced for 32 bit (2x16) flash configuration.
       a) lack of testing capbilities for 2x16 multi-byte write operation
       b) amd_flash_program_buffer() is not 2x16 compatible at the moment
       due to insufficiency of amd_program_buffer_status()
       Closing these issues will obsolete amd_flash_program32(). */
    int idx;

    /* unroll buffer to single writes */
    for (idx = 0; idx < count; idx++)
    {
        int status = amd_flash_program_single (cfi_array, adr, buffer[idx]);
        if (status != URJ_STATUS_OK)
            return status;
        adr += cfi_array->bus_width;
    }

    return URJ_STATUS_OK;
}

const urj_flash_driver_t urj_flash_amd_32_flash_driver = {
    N_("AMD/Fujitsu Standard Command Set"),
    N_("supported: AMD 29LV640D, 29LV641D, 29LV642D; 2x16 Bit"),
    4,                          /* buswidth */
    amd_flash_autodetect32,
    amd_flash_print_info,
    amd_flash_erase_block,
    amd_flash_lock_block,
    amd_flash_unlock_block,
    amd_flash_program32,
    amd_flash_read_array,
};

const urj_flash_driver_t urj_flash_amd_16_flash_driver = {
    N_("AMD/Fujitsu Standard Command Set"),
    N_("supported: AMD 29LV800B, S29GLxxxN; MX29LV640B, W19B320AT/B; 1x16 Bit"),
    2,                          /* buswidth */
    amd_flash_autodetect16,
    amd_flash_print_info,
    amd_flash_erase_block,
    amd_flash_lock_block,
    amd_flash_unlock_block,
    amd_flash_program,
    amd_flash_read_array,
};

const urj_flash_driver_t urj_flash_amd_8_flash_driver = {
    N_("AMD/Fujitsu Standard Command Set"),
    N_("supported: AMD 29LV160, AMD 29LV065D, AMD 29LV040B, S29GLxxxN, W19B320AT/B; 1x8 Bit"),
    1,                          /* buswidth */
    amd_flash_autodetect8,
    amd_flash_print_info,
    amd_flash_erase_block,
    amd_flash_lock_block,
    amd_flash_unlock_block,
    amd_flash_program,
    amd_flash_read_array,
};
