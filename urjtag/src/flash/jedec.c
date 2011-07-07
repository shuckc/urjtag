/*
 * $Id$
 *
 * Copyright (C) 2003 Matan Ziv-Av
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
 * Written by Matan Ziv-Av, 2003.
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <urjtag/log.h>
#include <urjtag/error.h>
#include <urjtag/flash.h>
#include <urjtag/bus.h>

#include "flash.h"

#include "cfi.h"
#include "intel.h"
#include "jedec.h"

/* Manufacturers */
#define MANUFACTURER_AMD        0x0001
#define MANUFACTURER_ATMEL      0x001F
#define MANUFACTURER_FUJITSU    0x0004
#define MANUFACTURER_ST         0x0020
#define MANUFACTURER_SST        0x00BF
#define MANUFACTURER_TOSHIBA    0x0098
#define MANUFACTURER_MX         0x00C2

/* AMD */
#define AM29F800BB      0x2258
#define AM29F800BT      0x22D6
#define AM29LV800BB     0x225B
#define AM29LV800BT     0x22DA
#define AM29LV400BT     0x22B9
#define AM29LV400BB     0x22BA
#define AM29LV160DT     0x22C4
#define AM29LV160DB     0x2249
#define AM29BDS323D     0x22D1
#define AM29BDS643D     0x227E
#define AM29LV081B      0x0038
#define AM29LV040B      0x004F

/* Atmel */
#define AT49xV16x       0x00C0
#define AT49xV16xT      0x00C2

/* Fujitsu */
#define MBM29LV160TE    0x22C4
#define MBM29LV160BE    0x2249
#define MBM29LV800BB    0x225B

/* ST - www.st.com */
#define M29W800T        0x00D7
#define M29W800B        0x005B
#define M29W160DT       0x22C4
#define M29W160DB       0x2249

/* SST */
#define SST39LF800      0x2781
#define SST39LF160      0x2782

/* Toshiba */
#define TC58FVT160      0x00C2
#define TC58FVB160      0x0043

/* MX */
#define MX29LV400T      0x22B9
#define MX29LV400B      0x22BA

struct mtd_erase_region_info
{
    uint32_t offset;            /* At which this region starts, from the beginning of the MTD */
    uint32_t erasesize;         /* For this region */
    uint32_t numblocks;         /* Number of blocks of erasesize in this region */
};

struct amd_flash_info
{
    const int mfr_id;
    const int dev_id;
    const char *name;
    const long size;
    const uint8_t interface_width;
    const int numeraseregions;
    const struct mtd_erase_region_info regions[4];
};

static const struct amd_flash_info table[] = {
    {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV160DT,
        .name = "AMD AM29LV160DT",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 31 },
            { .offset = 0x1F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x1F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x1FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV160DB,
        .name = "AMD AM29LV160DB",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 31 }
        }
    }, {
        .mfr_id = MANUFACTURER_TOSHIBA,
        .dev_id = TC58FVT160,
        .name = "Toshiba TC58FVT160",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 31 },
            { .offset = 0x1F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x1F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x1FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_FUJITSU,
        .dev_id = MBM29LV160TE,
        .name = "Fujitsu MBM29LV160TE",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 31 },
            { .offset = 0x1F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x1F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x1FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_TOSHIBA,
        .dev_id = TC58FVB160,
        .name = "Toshiba TC58FVB160",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 31 }
        }
    }, {
        .mfr_id = MANUFACTURER_FUJITSU,
        .dev_id = MBM29LV160BE,
        .name = "Fujitsu MBM29LV160BE",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 31 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV800BB,
        .name = "AMD AM29LV800BB",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 15 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29F800BB,
        .name = "AMD AM29F800BB",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 15 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV800BT,
        .name = "AMD AM29LV800BT",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 15 },
            { .offset = 0x0F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x0F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x0FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29F800BT,
        .name = "AMD AM29F800BT",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 15 },
            { .offset = 0x0F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x0F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x0FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_FUJITSU,
        .dev_id = MBM29LV800BB,
        .name = "Fujitsu MBM29LV800BB",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 15 }
        }
    }, {
        .mfr_id = MANUFACTURER_ST,
        .dev_id = M29W800T,
        .name = "ST M29W800T",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 15 },
            { .offset = 0x0F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x0F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x0FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_ST,
        .dev_id = M29W800B,
        .name = "ST M29W800B",
        .size = 0x00100000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 15 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV400BB,
        .name = "AMD AM29LV400BB",
        .size = 0x00080000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x02000, .numblocks = 1 },
            { .offset = 0x002000, .erasesize = 0x01000, .numblocks = 2 },
            { .offset = 0x003000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 7 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV400BT,
        .name = "AMD AM29LV400BT",
        .size = 0x00080000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x08000, .numblocks = 7 },
            { .offset = 0x070000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x078000, .erasesize = 0x01000, .numblocks = 2 },
            { .offset = 0x07C000, .erasesize = 0x02000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_ST,
        .dev_id = M29W160DT,
        .name = "ST M29W160DT",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 31 },
            { .offset = 0x1F0000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x1F8000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x1FC000, .erasesize = 0x04000, .numblocks = 1 }
        }
    }, {
        .mfr_id = MANUFACTURER_ST,
        .dev_id = M29W160DB,
        .name = "ST M29W160DB",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 31 }
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29BDS323D,
        .name = "AMD AM29BDS323D",
        .size = 0x00400000,
        .interface_width = CFI_INTERFACE_X16,
        .numeraseregions = 3,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 48 },
            { .offset = 0x300000, .erasesize = 0x10000, .numblocks = 15 },
            { .offset = 0x3f0000, .erasesize = 0x02000, .numblocks = 8 },
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29BDS643D,
        .name = "AMD AM29BDS643D",
        .size = 0x00800000,
        .interface_width = CFI_INTERFACE_X16,
        .numeraseregions = 3,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 96 },
            { .offset = 0x600000, .erasesize = 0x10000, .numblocks = 31 },
            { .offset = 0x7f0000, .erasesize = 0x02000, .numblocks = 8 },
        }
    }, {
        .mfr_id = MANUFACTURER_ATMEL,
        .dev_id = AT49xV16x,
        .name = "Atmel AT49xV16x",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 2,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x02000, .numblocks = 8 },
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 31 }
        }
    }, {
        .mfr_id = MANUFACTURER_ATMEL,
        .dev_id = AT49xV16xT,
        .name = "Atmel AT49xV16xT",
        .size = 0x00200000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 2,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 31 },
            { .offset = 0x1F0000, .erasesize = 0x02000, .numblocks = 8 }
        }
    }, {
        .mfr_id = MANUFACTURER_MX,
        .dev_id = MX29LV400T,
        .name = "MX 29LV400T",
        .size = 0x0080000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 7 },
            { .offset = 0x070000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x078000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x07c000, .erasesize = 0x04000, .numblocks = 1 },
        }
    }, {
        .mfr_id = MANUFACTURER_MX,
        .dev_id = MX29LV400B,
        .name = "MX 29LV400B",
        .size = 0x0080000,
        .interface_width = CFI_INTERFACE_X8_X16,
        .numeraseregions = 4,
        .regions = {
            { .offset = 0x010000, .erasesize = 0x10000, .numblocks = 7 },
            { .offset = 0x008000, .erasesize = 0x08000, .numblocks = 1 },
            { .offset = 0x004000, .erasesize = 0x02000, .numblocks = 2 },
            { .offset = 0x000000, .erasesize = 0x04000, .numblocks = 1 },
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV040B,
        .name = "AMD AM29LV040B",
        .size = 0x0080000,
        .interface_width = CFI_INTERFACE_X8,
        .numeraseregions = 1,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 8 },
        }
    }, {
        .mfr_id = MANUFACTURER_AMD,
        .dev_id = AM29LV081B,
        .name = "AMD AM29LV081B",
        .size = 0x0100000,
        .interface_width = CFI_INTERFACE_X8,
        .numeraseregions = 1,
        .regions = {
            { .offset = 0x000000, .erasesize = 0x10000, .numblocks = 16 },
        }
    }
};

int
urj_flash_jedec_detect (urj_bus_t *bus, uint32_t adr,
                        urj_flash_cfi_array_t **cfi_array)
{
    int manid = 0, devid = 0;
    int ba, bw;
    int dev_idx, j;
    urj_flash_cfi_query_structure_t *cfi;
    urj_bus_area_t area;

    *cfi_array = calloc (1, sizeof (urj_flash_cfi_array_t));
    if (!*cfi_array)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof (urj_flash_cfi_array_t));
        return URJ_STATUS_FAIL;
    }

    (*cfi_array)->bus = bus;
    (*cfi_array)->address = adr;
    if (URJ_BUS_AREA (bus, adr, &area) != URJ_STATUS_OK)
        // retain error state
        return URJ_STATUS_FAIL;
    if (URJ_BUS_TYPE (bus) != URJ_BUS_TYPE_PARALLEL)
        return URJ_STATUS_FAIL;
    bw = area.width;
    if (bw != 8 && bw != 16 && bw != 32)
    {
        urj_error_set (URJ_ERROR_INVALID, "bus width %d", bw);
        return URJ_STATUS_FAIL;
    }
    (*cfi_array)->bus_width = ba = bw / 8;

    (*cfi_array)->cfi_chips = calloc (1, sizeof (urj_flash_cfi_chip_t *) * ba);
    if (!(*cfi_array)->cfi_chips)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof (urj_flash_cfi_chip_t *) * ba);
        return URJ_STATUS_FAIL;
    }

    (*cfi_array)->cfi_chips[0] = calloc (1, sizeof (urj_flash_cfi_chip_t));
    if (!(*cfi_array)->cfi_chips[0])
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "calloc(%zd,%zd) fails",
                       (size_t) 1, sizeof (urj_flash_cfi_chip_t));
        return URJ_STATUS_FAIL;
    }


    /* device index to table, -1 means no match found */
    dev_idx = -1;


    /* probe device with Autoselect applicable for
       -    x16 devices             on 16 bit bus
       - x8_x16 devices in  x8 mode on  8 bit bus
       - x8_x16 devices in x16 mode on 16 bit bus
       also refer to the discussion of amd_flash_address_shift() in amd.c */
    if (dev_idx < 0 && (bw == 8 || bw == 16))
    {
        /* compare mask for manufacturer and device id */
        int id_mask = (1 << bw) - 1;

        URJ_BUS_WRITE (bus, adr, 0xf0);
        URJ_BUS_WRITE (bus, adr + 0xaaa, 0xaa);
        URJ_BUS_WRITE (bus, adr + 0x555, 0x55);
        URJ_BUS_WRITE (bus, adr + 0xaaa, 0x90);

        manid = URJ_BUS_READ (bus, adr + 0);
        devid = URJ_BUS_READ (bus, adr + (1 << 1));
        URJ_BUS_WRITE (bus, adr, 0xf0);

        for (dev_idx = ARRAY_SIZE (table) - 1; dev_idx >= 0; dev_idx--)
        {
            if (table[dev_idx].interface_width == CFI_INTERFACE_X16 ||
                table[dev_idx].interface_width == CFI_INTERFACE_X8_X16)
            {
                /* strip down ids to current bus width */
                manid &= id_mask;
                devid &= id_mask;
                if (manid == (table[dev_idx].mfr_id & id_mask) &&
                    devid == (table[dev_idx].dev_id & id_mask))
                    /* manufacturer and device id matched */
                    break;
            }
        }
    }


    /* probe device with Autoselect applicable for
       - x8 devices on 8 bit bus */
    if (dev_idx < 0 && bw == 8)
    {
        URJ_BUS_WRITE (bus, adr, 0xf0);
        URJ_BUS_WRITE (bus, adr + 0x555, 0xaa);
        URJ_BUS_WRITE (bus, adr + 0x2aa, 0x55);
        URJ_BUS_WRITE (bus, adr + 0x555, 0x90);

        manid = URJ_BUS_READ (bus, adr + 0);
        devid = URJ_BUS_READ (bus, adr + 1);
        URJ_BUS_WRITE (bus, adr, 0xf0);

        for (dev_idx = ARRAY_SIZE (table) - 1; dev_idx >= 0; dev_idx--)
        {
            if (table[dev_idx].interface_width == CFI_INTERFACE_X8)
            {
                if (manid == table[dev_idx].mfr_id && devid == table[dev_idx].dev_id)
                    /* manufacturer and device id matched */
                    break;
            }
        }
    }


    urj_log (URJ_LOG_LEVEL_NORMAL, "dev ID=%04x   man ID=%04x\n", devid, manid);

    if (dev_idx < 0)
    {
        urj_error_set (URJ_ERROR_NOTFOUND, "amd_flash_info table");
        return URJ_STATUS_FAIL;
    }

    cfi = &(*cfi_array)->cfi_chips[0]->cfi;

    cfi->identification_string.pri_id_code = CFI_VENDOR_AMD_SCS;
    cfi->identification_string.pri_vendor_tbl = NULL;
    cfi->identification_string.alt_id_code = 0;
    cfi->identification_string.alt_vendor_tbl = NULL;

    cfi->device_geometry.device_size = table[dev_idx].size;
    /* annotate chip width */
    cfi->device_geometry.device_interface = table[dev_idx].interface_width;
    switch (table[dev_idx].interface_width)
    {
    case CFI_INTERFACE_X8:
        (*cfi_array)->cfi_chips[0]->width = 1;
        break;
    case CFI_INTERFACE_X16:
        (*cfi_array)->cfi_chips[0]->width = 2;
        break;
    case CFI_INTERFACE_X8_X16:
        (*cfi_array)->cfi_chips[0]->width = ba;
        break;
    case CFI_INTERFACE_X32:
        (*cfi_array)->cfi_chips[0]->width = 4;
        break;
    case CFI_INTERFACE_X16_X32:
        (*cfi_array)->cfi_chips[0]->width = ba;
        break;
    default:
        /* unsupported interface geometry */
        (*cfi_array)->cfi_chips[0]->width = 1;
        cfi->device_geometry.device_interface = CFI_INTERFACE_X8;
        urj_error_set (URJ_ERROR_UNSUPPORTED,
                       "interface geometry %d", table[dev_idx].interface_width);
        return URJ_STATUS_FAIL;
    }

    cfi->device_geometry.number_of_erase_regions = table[dev_idx].numeraseregions;

    cfi->device_geometry.erase_block_regions =
        malloc (cfi->device_geometry.number_of_erase_regions *
                sizeof (urj_flash_cfi_erase_block_region_t));
    if (!cfi->device_geometry.erase_block_regions)
    {
        urj_error_set (URJ_ERROR_OUT_OF_MEMORY, "malloc(%zd) fails",
                       cfi->device_geometry.number_of_erase_regions
                       * sizeof (urj_flash_cfi_erase_block_region_t));
        return URJ_STATUS_FAIL;
    }

    for (j = 0; j < cfi->device_geometry.number_of_erase_regions; j++)
    {
        cfi->device_geometry.erase_block_regions[j].erase_block_size =
            table[dev_idx].regions[j].erasesize;
        cfi->device_geometry.erase_block_regions[j].number_of_erase_blocks =
            table[dev_idx].regions[j].numblocks;
    }

    urj_log (URJ_LOG_LEVEL_NORMAL,
             "Found %s flash,  size = %li bytes.\n", table[dev_idx].name,
             table[dev_idx].size);

    return URJ_STATUS_OK;
}
