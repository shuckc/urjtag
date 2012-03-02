/*
 * $Id$
 *
 * Analog Devices unified Blackfin bus functions
 *
 * Copyright (C) 2008-2011 Analog Devices, Inc.
 * Licensed under the GPL-2 or later.
 *
 * Written by Mike Frysinger <vapier@gentoo.org> heavily leveraging
 * the work of Jie Zhang <jie.zhang@analog.com>.
 */

#include "blackfin.h"

#define IS_ASYNC_ADDR(params, adr) \
({ \
    unsigned long __addr = (unsigned long) (adr); \
    bfin_bus_params_t *__params = (void *) (params); \
    __addr >= __params->async_base && \
    __addr < __params->async_base + __params->async_size; \
})
#define ASYNC_BANK(params, adr) (((adr) & (((bfin_bus_params_t *)(params))->async_size - 1)) >> 20)

static int
bfin_bus_attach_sigs (urj_part_t *part, urj_part_signal_t **pins, int pin_cnt,
                      const char *spin, int off)
{
    int i;
    char buf[16];
    int ret = 0;

    for (i = 0; i < pin_cnt; ++i)
    {
        sprintf (buf, "%s%i", spin, i + off);
        ret |= urj_bus_generic_attach_sig (part, &pins[i], buf);
    }

    return ret;
}

int
bfin_bus_new (urj_bus_t *bus, const urj_param_t *cmd_params[],
              const bfin_bus_default_t *defaults)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    const urj_param_t **p[2] = {NULL, cmd_params};
    int ret = 0;
    size_t i, j;

    if (defaults != NULL)
    {
        ret = urj_param_init (p);
        if (ret != URJ_STATUS_OK)
            return ret;

        for (i = 0; defaults[i].bus_name != NULL; ++i)
        {
            if (strcmp (defaults[i].bus_name, bus->driver->name))
                continue;

            ret = urj_param_push (&urj_bus_param_list, p,
                                  defaults[i].param);
            if (ret != URJ_STATUS_OK)
            {
                urj_param_clear (p);
                return ret;
            }
        }
    }

    for (j = 0; j < 2; j++)
    {
        if (!p[j])
            continue;

        for (i = 0; p[j][i]; ++i)
            switch (p[j][i]->key)
            {
            case URJ_BUS_PARAM_KEY_HWAIT:
                {
                const char *hwait = p[j][i]->value.string;

                params->hwait_level = (hwait[0] == '/');
                if (params->hwait_level)
                    ++hwait;

                ret |= urj_bus_generic_attach_sig (part, &params->hwait, hwait);
                break;
                }
            default:
                urj_error_set (URJ_ERROR_SYNTAX, _("unknown bus parameter"));
                return 1;
            }
    }

    if (!params->async_base)
        params->async_base = 0x20000000;

    /* Most signals start at 0, but ADDR starts at 1 (because it's 16bit) */
    ret |= bfin_bus_attach_sigs (part, params->ams, params->ams_cnt, "AMS_B", 0);
    ret |= bfin_bus_attach_sigs (part, params->abe, params->abe_cnt, "ABE_B", 0);
    ret |= bfin_bus_attach_sigs (part, params->data, params->data_cnt, "DATA", 0);
    ret |= bfin_bus_attach_sigs (part, params->addr, params->addr_cnt, "ADDR", 1);

    ret |= urj_bus_generic_attach_sig (part, &params->aoe, "AOE_B");
    ret |= urj_bus_generic_attach_sig (part, &params->are, "ARE_B");
    ret |= urj_bus_generic_attach_sig (part, &params->awe, "AWE_B");

    if (params->sdram)
    {
        ret |= urj_bus_generic_attach_sig (part, &params->scas, "SCAS_B");
        ret |= urj_bus_generic_attach_sig (part, &params->sras, "SRAS_B");
        ret |= urj_bus_generic_attach_sig (part, &params->swe, "SWE_B");
        if (!params->sms_cnt)
        {
            ret |= urj_bus_generic_attach_sig (part, &params->sms[0], "SMS_B");
            params->sms_cnt = 1;
        }
        else
            ret |= bfin_bus_attach_sigs (part, params->sms, params->sms_cnt, "SMS_B", 0);
    }

    return ret;
}

int
bfin_bus_area (urj_bus_t *bus, uint32_t adr, urj_bus_area_t *area)
{
    bfin_bus_params_t *params = bus->params;

    if (adr < params->async_base)
    {
        /* we can only wiggle SDRAM pins directly, so cannot drive it */
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS,
                       _("reading external memory not supported"));
        return URJ_STATUS_FAIL;
    }
    else if (IS_ASYNC_ADDR(params, adr))
    {
        area->description = "asynchronous memory";
        area->start = params->async_base;
        area->length = params->async_size;
        area->width = 16;
    }
    else
    {
        /* L1 needs core to access it */
        urj_error_set (URJ_ERROR_OUT_OF_BOUNDS,
                       _("reading on-chip memory not supported"));
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}

static void
bfin_select_flash_sdram (urj_bus_t *bus)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    int i;

    if (params->sdram)
    {
        urj_part_set_signal_high (part, params->sras);
        urj_part_set_signal_high (part, params->scas);
        urj_part_set_signal_high (part, params->swe);
        for (i = 0; i < params->sms_cnt; ++i)
            urj_part_set_signal_high (part, params->sms[0]);
    }
}

void
bfin_select_flash (urj_bus_t *bus, uint32_t adr)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    int i;

    for (i = 0; i < params->ams_cnt; ++i)
        urj_part_set_signal (part, params->ams[i], 1,
                             !(ASYNC_BANK(params, adr) == i));

    for (i = 0; i < params->abe_cnt; ++i)
        urj_part_set_signal_low (part, params->abe[i]);

    if (params->hwait)
        urj_part_set_signal (part, params->hwait, 1, params->hwait_level);

    bfin_select_flash_sdram (bus);

    if (params->select_flash)
        params->select_flash (bus, adr);
}

void
bfin_unselect_flash (urj_bus_t *bus)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    int i;

    for (i = 0; i < params->ams_cnt; ++i)
        urj_part_set_signal_high (part, params->ams[i]);

    for (i = 0; i < params->abe_cnt; ++i)
        urj_part_set_signal_high (part, params->abe[i]);

    if (params->hwait)
        urj_part_set_signal (part, params->hwait, 1, params->hwait_level);

    bfin_select_flash_sdram (bus);

    if (params->unselect_flash)
        params->unselect_flash (bus);
}

void
bfin_setup_address (urj_bus_t *bus, uint32_t adr)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    int i;

    for (i = 0; i < params->addr_cnt; ++i)
        urj_part_set_signal (part, params->addr[i], 1, (adr >> (i + 1)) & 1);
}

void
bfin_set_data_in (urj_bus_t *bus)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    int i;

    for (i = 0; i < params->data_cnt; ++i)
        urj_part_set_signal_input (part, params->data[i]);
}

void
bfin_setup_data (urj_bus_t *bus, uint32_t data)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    int i;

    for (i = 0; i < params->data_cnt; ++i)
        urj_part_set_signal (part, params->data[i], 1, (data >> i) & 1);

}

static int
bfin_part_maybe_set_signal (urj_part_t *part, urj_part_signal_t *signal,
                            int out, int val)
{
    if (signal)
        return urj_part_set_signal (part, signal, out, val);
    return URJ_STATUS_OK;
}

int
bfin_bus_read_start (urj_bus_t *bus, uint32_t adr)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    urj_chain_t *chain = bus->chain;

    bfin_select_flash (bus, adr);

    bfin_part_maybe_set_signal (part, params->are, 1, 0);
    bfin_part_maybe_set_signal (part, params->awe, 1, 1);
    bfin_part_maybe_set_signal (part, params->aoe, 1, 0);

    bfin_setup_address (bus, adr);
    bfin_set_data_in (bus);

    urj_tap_chain_shift_data_registers (chain, 0);

    return URJ_STATUS_OK;
}

uint32_t
bfin_bus_read_end (urj_bus_t *bus)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    bfin_unselect_flash (bus);

    bfin_part_maybe_set_signal (part, params->are, 1, 1);
    bfin_part_maybe_set_signal (part, params->awe, 1, 1);
    bfin_part_maybe_set_signal (part, params->aoe, 1, 1);

    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < params->data_cnt; ++i)
        d |= (uint32_t) (urj_part_get_signal (part, params->data[i]) << i);

    return d;
}

uint32_t
bfin_bus_read_next (urj_bus_t *bus, uint32_t adr)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    urj_chain_t *chain = bus->chain;
    int i;
    uint32_t d = 0;

    bfin_setup_address (bus, adr);
    urj_tap_chain_shift_data_registers (chain, 1);

    for (i = 0; i < params->data_cnt; ++i)
        d |= (uint32_t) (urj_part_get_signal (part, params->data[i]) << i);

    return d;
}

void
bfin_bus_write (urj_bus_t *bus, uint32_t adr, uint32_t data)
{
    bfin_bus_params_t *params = bus->params;
    urj_part_t *part = bus->part;
    urj_chain_t *chain = bus->chain;

    bfin_select_flash (bus, adr);
    urj_part_set_signal_high (part, params->aoe);
    urj_part_set_signal_high (part, params->are);
    urj_part_set_signal_high (part, params->awe);

    bfin_setup_address (bus, adr);
    bfin_setup_data (bus, data);

    urj_tap_chain_shift_data_registers (chain, 0);

    urj_part_set_signal_low (part, params->awe);
    urj_part_set_signal_low (part, params->aoe);
    urj_tap_chain_shift_data_registers (chain, 0);
    urj_part_set_signal_high (part, params->awe);
    urj_part_set_signal_high (part, params->aoe);
    bfin_unselect_flash (bus);
    urj_tap_chain_shift_data_registers (chain, 0);
}

void
bfin_bus_printinfo (urj_log_level_t ll, urj_bus_t *bus)
{
    int i;

    for (i = 0; i < bus->chain->parts->len; i++)
        if (bus->part == bus->chain->parts->parts[i])
            break;

    urj_log (ll, _("%s (JTAG part No. %d)\n"), bus->driver->description, i);
}
