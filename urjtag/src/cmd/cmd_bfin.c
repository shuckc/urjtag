/* Copyright (C) 2008, 2009, 2010 Analog Devices, Inc.
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
 */


#include "sysdep.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <urjtag/part.h>
#include <urjtag/chain.h>
#include <urjtag/cmd.h>
#include <urjtag/bfin.h>

#include "cmd.h"

static int
cmd_bfin_run (urj_chain_t *chain, char *params[])
{
    int num_params;

    num_params = urj_cmd_params (params);

    if (num_params < 2)
    {
        urj_error_set (URJ_ERROR_SYNTAX,
                       "#parameters should be >= 2, not %d",
                       num_params);
        return URJ_STATUS_FAIL;
    }

    /* The remaining commands require cable or parts.  */

    if (urj_cmd_test_cable (chain) != URJ_STATUS_OK)
        return URJ_STATUS_FAIL;

    if (!chain->parts)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, "no parts, Run '%s' first",
                       "detect");
        return URJ_STATUS_FAIL;
    }

    if (chain->active_part >= chain->parts->len)
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, "no active part");
        return URJ_STATUS_FAIL;
    }

    if (!part_is_bfin (chain, chain->active_part))
    {
        urj_error_set (URJ_ERROR_ILLEGAL_STATE, "not a Blackfin part");
        return URJ_STATUS_FAIL;
    }

    assert (chain->active_part >= 0 && chain->active_part < chain->parts->len);

    if (strcmp (params[1], "emulation") == 0)
    {
        if (num_params != 3)
        {
            urj_error_set (URJ_ERROR_BFIN,
                           "'bfin emulation' requires 1 parameter, not %d",
                           num_params - 2);
            return URJ_STATUS_FAIL;
        }

        if (strcmp (params[2], "enable") == 0)
        {
            part_emulation_enable (chain, chain->active_part);
        }
        else if (strcmp (params[2], "trigger") == 0)
        {
            part_emulation_trigger (chain, chain->active_part);
        }
        else if (strcmp (params[2], "enter") == 0)
        {
            part_emulation_enable (chain, chain->active_part);
            part_emulation_trigger (chain, chain->active_part);
        }
        else if (strcmp (params[2], "return") == 0)
        {
            part_emulation_return (chain, chain->active_part);
        }
        else if (strcmp (params[2], "disable") == 0)
        {
            part_emulation_disable (chain, chain->active_part);
        }
        else if (strcmp (params[2], "exit") == 0)
        {
            part_emulation_return (chain, chain->active_part);
            part_emulation_disable (chain, chain->active_part);
        }
        else if (strcmp (params[2], "status") == 0)
        {
            uint16_t dbgstat, excause;
            const char *str_excause;
            urj_part_t *part;

            part_dbgstat_get (chain, chain->active_part);
            part = chain->parts->parts[chain->active_part];
            dbgstat = BFIN_PART_DBGSTAT (part);
            excause = part_dbgstat_emucause (chain, chain->active_part);

            switch (excause)
            {
                case 0x0: str_excause = "EMUEXCPT was executed"; break;
                case 0x1: str_excause = "EMUIN pin was asserted"; break;
                case 0x2: str_excause = "Watchpoint event occurred"; break;
                case 0x4: str_excause = "Performance Monitor 0 overflowed"; break;
                case 0x5: str_excause = "Performance Monitor 1 overflowed"; break;
                case 0x8: str_excause = "Emulation single step"; break;
                default:  str_excause = "Reserved??"; break;
            }

            urj_log (URJ_LOG_LEVEL_NORMAL, "Core [%d] DBGSTAT = 0x%"PRIx16"\n",
                     chain->active_part, dbgstat);

            urj_log (URJ_LOG_LEVEL_NORMAL,
                     "\tEMUDOF     = %u\n"
                     "\tEMUDIF     = %u\n"
                     "\tEMUDOOVF   = %u\n"
                     "\tEMUDIOVF   = %u\n"
                     "\tEMUREADY   = %u\n"
                     "\tEMUACK     = %u\n"
                     "\tEMUCAUSE   = 0x%x (%s)\n"
                     "\tBIST_DONE  = %u\n"
                     "\tLPDEC0     = %u\n"
                     "\tIN_RESET   = %u\n"
                     "\tIDLE       = %u\n"
                     "\tCORE_FAULT = %u\n"
                     "\tLPDEC1     = %u\n",
                     part_dbgstat_is_emudof (chain, chain->active_part),
                     part_dbgstat_is_emudif (chain, chain->active_part),
                     part_dbgstat_is_emudoovf (chain, chain->active_part),
                     part_dbgstat_is_emudiovf (chain, chain->active_part),
                     part_dbgstat_is_emuready (chain, chain->active_part),
                     part_dbgstat_is_emuack (chain, chain->active_part),
                     excause, str_excause,
                     part_dbgstat_is_bist_done (chain, chain->active_part),
                     part_dbgstat_is_lpdec0 (chain, chain->active_part),
                     part_dbgstat_is_in_reset (chain, chain->active_part),
                     part_dbgstat_is_idle (chain, chain->active_part),
                     part_dbgstat_is_core_fault (chain, chain->active_part),
                     part_dbgstat_is_lpdec1 (chain, chain->active_part));
        }
        else if (strcmp (params[2], "singlestep") == 0)
        {
            part_dbgstat_get (chain, chain->active_part);

            if (!part_dbgstat_is_emuready (chain, chain->active_part))
            {
                urj_error_set (URJ_ERROR_BFIN, "Run '%s' first",
                               "bfin emulation enter");
                return URJ_STATUS_FAIL;
            }

            /* TODO  Allow an argument to specify how many single steps.  */

            part_scan_select (chain, chain->active_part, DBGCTL_SCAN);
            part_dbgctl_bit_set_esstep (chain, chain->active_part);
            urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
            part_emuir_set (chain, chain->active_part, INSN_RTE, URJ_CHAIN_EXITMODE_IDLE);
            part_scan_select (chain, chain->active_part, DBGCTL_SCAN);
            part_dbgctl_bit_clear_esstep (chain, chain->active_part);
            urj_tap_chain_shift_data_registers_mode (chain, 0, 1, URJ_CHAIN_EXITMODE_UPDATE);
        }
        else
        {
            urj_error_set (URJ_ERROR_BFIN,
                           "unknown emulation subcommand '%s'", params[2]);
            return URJ_STATUS_FAIL;
        }

        return URJ_STATUS_OK;
    }
    else if (strcmp (params[1], "execute") == 0)
    {
        int execute_ret = URJ_STATUS_FAIL;

        part_dbgstat_get (chain, chain->active_part);

        if (!part_dbgstat_is_emuready (chain, chain->active_part))
        {
            urj_error_set (URJ_ERROR_BFIN, "Run '%s' first",
                           "bfin emulation enter");
            return URJ_STATUS_FAIL;
        }

        if (num_params > 2)
        {
            int i;
            struct bfin_insn *insns = NULL;
            struct bfin_insn **last = &insns;
            char tmp[8];
            char *tmpfile = NULL;

            for (i = 2; i < num_params; i++)
            {
                if (params[i][0] == '[' && params[i][1] == '0')
                {
                    uint64_t n;

                    if (sscanf (params[i], "[0%[xX]%"PRIx64"]", tmp, &n) != 2)
                        goto execute_cleanup;

                    *last = (struct bfin_insn *) malloc (sizeof (struct bfin_insn));
                    if (*last == NULL)
                        goto execute_cleanup;

                    (*last)->i = n;
                    (*last)->type = BFIN_INSN_SET_EMUDAT;
                    (*last)->next = NULL;
                    last = &((*last)->next);
                }
                else if (params[i][0] == '0')
                {
                    uint64_t n;

                    if (sscanf (params[i], "0%[xX]%"PRIx64, tmp, &n) != 2)
                        goto execute_cleanup;

                    *last = (struct bfin_insn *) malloc (sizeof (struct bfin_insn));
                    if (*last == NULL)
                        goto execute_cleanup;

                    (*last)->i = n;
                    (*last)->type = BFIN_INSN_NORMAL;
                    (*last)->next = NULL;
                    last = &((*last)->next);
                }
                else
                {
#ifndef HAVE_SYS_WAIT_H
                    urj_error_set (URJ_ERROR_BFIN,
                                   "Sorry, dynamic code not available for your platform");
                    goto execute_cleanup;
#else
                    unsigned char raw_insn[4];
                    char *tmp_buf;
                    char *tuples[] = {"uclinux", "linux-uclibc", "elf"};
                    size_t t;
                    FILE *fp;

                    /* 1024 should be plenty.  */
                    char insns_string[1024];
                    char *p = insns_string;

                    for (; i < num_params; i++)
                    {
                        p += snprintf (p, sizeof (insns_string) - (p - insns_string),
                                       "%s ", params[i]);
                        if (params[i][strlen (params[i]) - 1] == '"')
                            break;
                        if (params[i][strlen (params[i]) - 1] == ';')
                            break;
                    }
                    if (i == num_params)
                    {
                        urj_error_set (URJ_ERROR_BFIN,
                                       "unbalanced double quotes");
                        goto execute_cleanup;
                    }

                    /* p points past the '\0'. p - 1 points to the ending '\0'.
                       p - 2 points to the last double quote.  */
                    *(p - 2) = ';';
                    if (insns_string[0] == '"')
                        insns_string[0] = ' ';

                    /* HRM states that branches and hardware loop setup results
                       in undefined behavior.  Should check opcode instead?  */
                    if (strcasestr(insns_string, "jump") ||
                        strcasestr(insns_string, "call") ||
                        strcasestr(insns_string, "lsetup"))
                        urj_warning (_("jump/call/lsetup insns may not work in emulation\n"));

                    /* get a temporary file to work with -- a little racy */
                    if (!tmpfile)
                    {
                        tmpfile = tmpnam (NULL);
                        if (!tmpfile)
                            goto execute_cleanup;
                        tmpfile = strdup (tmpfile);
                        if (!tmpfile)
                            goto execute_cleanup;
                    }

                    /* try to find a toolchain in $PATH */
                    for (t = 0; t < ARRAY_SIZE(tuples); ++t)
                    {
                        int ret;

                        /* TODO  Pass -mcpu= to gas.  */
                        ret = asprintf (&tmp_buf,
                                        "bfin-%3$s-as --version >/dev/null 2>&1 || exit $?;"
                                        "echo '%1$s' | bfin-%3$s-as - -o \"%2$s\""
                                        " && bfin-%3$s-objcopy -O binary \"%2$s\"",
                                        insns_string, tmpfile, tuples[t]);
                        if (ret == -1)
                        {
                            urj_error_set (URJ_ERROR_OUT_OF_MEMORY, _("asprintf() failed"));
                            goto execute_cleanup;
                        }
                        ret = system (tmp_buf);
                        free (tmp_buf);
                        if (WIFEXITED(ret))
                        {
                            if (WEXITSTATUS(ret) == 0)
                                break;
                            /* 127 -> not found in $PATH */
                            else if (WEXITSTATUS(ret) == 127)
                                continue;
                            else
                            {
                                urj_error_set (URJ_ERROR_BFIN, "GAS failed parsing: %s",
                                               insns_string);
                                goto execute_cleanup;
                            }
                        }
                    }
                    if (t == ARRAY_SIZE(tuples))
                    {
                        urj_error_set (URJ_ERROR_BFIN,
                                       "unable to find Blackfin toolchain in $PATH\n");
                        goto execute_cleanup;
                    }

                    /* Read the binary blob from the toolchain */
                    fp = fopen (tmpfile, FOPEN_R);
                    if (fp == NULL)
                        goto execute_cleanup;

                    /* Figure out how many instructions there are */
                    t = 0;
                    p = insns_string;
                    while ((p = strchr(p, ';')) != NULL)
                    {
                        ++t;
                        ++p;
                    }

                    while (t-- && fread (raw_insn, 1, 2, fp) == 2)
                    {
                        uint16_t iw = raw_insn[0] | (raw_insn[1] << 8);
                        uint64_t n = iw;
                        int is_multiinsn = INSN_IS_MULTI (raw_insn[1]);

                        if ((iw & 0xf000) >= 0xc000)
                        {
                            if (fread (raw_insn, 1, 2, fp) != 2)
                                goto execute_cleanup;

                            iw = raw_insn[0] | (raw_insn[1] << 8);
                            n = (n << 16) | iw;
                        }

                        if (is_multiinsn)
                        {
                            if (fread (raw_insn, 1, 4, fp) != 4)
                                goto execute_cleanup;

                            n = (n << 32)
                                | ((uint64_t)raw_insn[0] << 16)
                                | ((uint64_t)raw_insn[1] << 24)
                                | raw_insn[2] | (raw_insn[3] << 8);
                        }

                        *last = (struct bfin_insn *) malloc (sizeof (struct bfin_insn));
                        if (*last == NULL)
                            goto execute_cleanup;

                        (*last)->i = n;
                        (*last)->type = BFIN_INSN_NORMAL;
                        (*last)->next = NULL;
                        last = &((*last)->next);
                    }

                    fclose (fp);
#endif
                }
            }

            part_execute_instructions (chain, chain->active_part, insns);
            execute_ret = URJ_STATUS_OK;

          execute_cleanup:
            if (tmpfile)
            {
                unlink (tmpfile);
                free (tmpfile);
            }
            while (insns)
            {
                struct bfin_insn *tmp = insns->next;
                free (insns);
                insns = tmp;
            }
        }

        if (execute_ret == URJ_STATUS_OK)
        {
            uint64_t emudat;

            emudat = part_emudat_get (chain, chain->active_part, URJ_CHAIN_EXITMODE_UPDATE);
            urj_log (URJ_LOG_LEVEL_NORMAL, "EMUDAT = 0x%"PRIx64"\n", emudat);

            part_dbgstat_get (chain, chain->active_part);
            if (part_dbgstat_is_core_fault (chain, chain->active_part))
                urj_warning (_("core fault detected\n"));
        }

        return execute_ret;
    }
    else if (strcmp (params[1], "reset") == 0)
    {
        int reset_what = 0;

        part_dbgstat_get (chain, chain->active_part);

        if (!part_dbgstat_is_emuready (chain, chain->active_part))
        {
            urj_error_set (URJ_ERROR_BFIN, "Run '%s' first",
                           "bfin emulation enter");
            return URJ_STATUS_FAIL;
        }

        if (num_params == 3)
        {
            if (!strcmp (params[2], "core"))
                reset_what |= 0x1;
            else if (!strcmp (params[2], "system"))
                reset_what |= 0x2;
            else
            {
                urj_error_set (URJ_ERROR_BFIN, "bad parameter '%s'", params[2]);
                return URJ_STATUS_FAIL;
            }
        }
        else if (num_params == 2)
            reset_what = 0x1 | 0x2;
        else
        {
            urj_error_set (URJ_ERROR_BFIN,
                           "'bfin emulation' requires 0 or 1 parameter, not %d",
                           num_params - 2);
            return URJ_STATUS_FAIL;
        }

        urj_log (URJ_LOG_LEVEL_NORMAL,
                 _("%s: reseting processor ... "), "bfin");
        fflush (stdout);
        if (reset_what == 0x3)
            software_reset (chain, chain->active_part);
        else if (reset_what & 0x1)
            bfin_core_reset (chain, chain->active_part);
        else if (reset_what & 0x2)
            chain_system_reset (chain);
        urj_log (URJ_LOG_LEVEL_NORMAL, _("OK\n"));

        return URJ_STATUS_OK;
    }
    else
    {
        urj_error_set (URJ_ERROR_BFIN,
                       "unknown command '%s'", params[1]);
        return URJ_STATUS_FAIL;
    }

    return URJ_STATUS_OK;
}


static void
cmd_bfin_help (void)
{
    urj_log (URJ_LOG_LEVEL_NORMAL,
             _("Usage: %s INSTRUCTIONs\n"
               "Usage: %s\n"
               "Usage: %s\n"
               "Blackfin specific commands\n"
               "\n"
               "INSTRUCTIONs are a sequence of Blackfin encoded instructions,\n"
               "double quoted assembly statements and [EMUDAT_IN]s\n"),
             "bfin execute",
             "bfin emulation enable|trigger|enter|return|disable|exit|singlestep|status",
             "bfin reset [core|system]");
}

static void
cmd_bfin_complete (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                   char * const *tokens, const char *text, size_t text_len,
                   size_t token_point)
{
    static const char * const main_cmds[] = {
        "execute",
        "emulation",
        "reset",
    };
    static const char * const emu_cmds[] = {
        "enable",
        "trigger",
        "enter",
        "return",
        "disable",
        "exit",
        "singlestep",
        "status",
    };
    static const char * const reset_cmds[] = {
        "core",
        "system",
    };

    switch (token_point)
    {
    case 1:
        urj_completion_mayben_add_matches (matches, match_cnt, text, text_len,
                                           main_cmds);
        break;

    case 2:
        if (!strcmp (tokens[1], "reset"))
            urj_completion_mayben_add_matches (matches, match_cnt, text,
                                               text_len, reset_cmds);
        else if (!strcmp (tokens[1], "emulation"))
            urj_completion_mayben_add_matches (matches, match_cnt, text,
                                               text_len, emu_cmds);
        break;
    }
}

const urj_cmd_t urj_cmd_bfin = {
    "bfin",
    N_("Blackfin specific commands"),
    cmd_bfin_help,
    cmd_bfin_run,
    cmd_bfin_complete,
};
