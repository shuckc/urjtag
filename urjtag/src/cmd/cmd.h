/*
 * $Id$
 *
 * Copyright (C) 2003 ETC s.r.o.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the ETC s.r.o. nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Written by Marcel Telka <marcel@telka.sk>, 2003.
 *
 */

#ifndef URJ_SRC_CMD_H
#define URJ_SRC_CMD_H

#include <stdlib.h> /* qsort */
#include <stdbool.h>
#include <sysdep.h>

#include <urjtag/params.h>
#include <urjtag/types.h>

typedef struct
{
    char *name;
    char *description;
    void (*help) (void);
    /** @return URJ_STATUS_OK on success; URJ_STATUS_FAIL on error, both
     * syntax and library errors */
    int (*run) (urj_chain_t *chain, char *params[]);
    void (*complete) (urj_chain_t *chain, char ***matches, size_t *match_cnt,
                      char * const *tokens, const char *text, size_t text_len,
                      size_t token_point);
} urj_cmd_t;

#define _URJ_CMD(cmd) extern const urj_cmd_t urj_cmd_##cmd;
#include "cmd_list.h"

extern const urj_cmd_t * const urj_cmds[];

/**
 * Show a list of structures with name/desc
 */
#define urj_cmd_show_list(arr) \
do { \
    int i, max_len = 0; \
    for (i = 0; arr[i]; ++i) { \
        int this_len = strlen(arr[i]->name); \
        if (max_len < this_len) \
            max_len = this_len; \
    } \
    for (i = 0; arr[i]; ++i) \
        urj_log (URJ_LOG_LEVEL_NORMAL, "%-*s %s\n", max_len + 1, \
                 arr[i]->name, _(arr[i]->description)); \
} while (0)

/**
 * Internal completion helper for adding to the set of matches.
 *
 * @param match this must be malloced memory that may be freed in the
 *              future by common code -- do not free it yourself
 */
void urj_completion_add_match (char ***matches, size_t *cnt, char *match);

/**
 * Internal completion helper for adding to the set of matches.
 *
 * @param match this string will be strduped before being passed down
 *              to the urj_completion_add_match helper.
 */
void urj_completion_add_match_dupe (char ***matches, size_t *cnt,
                                    const char *match);

/**
 * Internal completion helper for possibly adding to the set of matches.
 * If text matches the leading portion of match, then it will be added.
 *
 * @param text  the string to compare to match (e.g. user input)
 * @param match the string to possibly add to the set of matches
 */
void urj_completion_maybe_add_match (char ***matches, size_t *cnt,
                                     const char *text, const char *match);

/**
 * This is just like urj_completion_maybe_add_match, except the length of
 * text is precomputed.  This is so common, we get a dedicated function.
 *
 * @param text     the string to compare to match (e.g. user input)
 * @param text_len the length of text
 * @param match    the string to possibly add to the set of matches
 */
void urj_completion_mayben_add_match (char ***matches, size_t *cnt,
                                      const char *text, size_t text_len,
                                      const char *match);

/**
 * This is just like urj_completion_mayben_add_match, except you pass in
 * an array of strings rather than just one single one.
 *
 * @param text     the string to compare to match (e.g. user input)
 * @param text_len the length of text
 * @param matchs   the array of strings to possibly add to the set of matches
 * @param num      the number of elements in the @matchs array
 */
void urj_completion_mayben_add_matches_num (char ***matches, size_t *cnt,
                                            const char *text, size_t text_len,
                                            const char * const *matchs,
                                            size_t num);

/**
 * This is just like urj_completion_mayben_add_matches_num, except the @num
 * arg is automatically computed.
 */
#define urj_completion_mayben_add_matches(matches, cnt, text, text_len, matchs) \
    urj_completion_mayben_add_matches_num (matches, cnt, text, text_len, \
                                           matchs, ARRAY_SIZE (matchs))

/**
 * This is just like urj_completion_mayben_add_matches_num, but the array
 * info comes straight from the urj_param_list_t structure.
 *
 * @param text       the string to compare to match (e.g. user input)
 * @param text_len   the length of text
 * @param param_list the list of parameter strings to possibly add to the set of matches
 */
void urj_completion_mayben_add_param_list (char ***matches, size_t *cnt,
                                           const char *text, size_t text_len,
                                           urj_param_list_t param_list);

/**
 * This is just like urj_completion_mayben_add_match, but the matching is
 * done against file names (and optionally searches the UrJTAG data dir).
 *
 * @param text     the string to compare to match (e.g. user input)
 * @param text_len the length of text
 * @param search   should relative paths search the UrJTAG data dir
 */
void urj_completion_mayben_add_file (char ***matches, size_t *cnt,
                                     const char *text, size_t text_len,
                                     bool search);

/**
 * Internal completion helper for matching against the signal list.
 * Since many functions involve signals as an option, unify the code
 * in one place.
 */
void cmd_signal_complete (urj_chain_t *chain, char ***matches,
                          size_t *match_cnt, const char *text, size_t text_len);

#endif /* URJ_CMD_H */
