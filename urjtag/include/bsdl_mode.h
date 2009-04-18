/*
 * $Id$
 *
 * Copyright (C) 2008, Arnim Laeuger
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
 * Written by Arnim Laeuger <arniml@users.sourceforge.net>, 2008.
 *
 */

#ifndef BSDL_MODE_H
#define BSDL_MODE_H

#define BSDL_MODE_MSG_NOTE     (1 <<  0)
#define BSDL_MODE_MSG_WARN     (1 <<  1)
#define BSDL_MODE_MSG_ERR      (1 <<  2)
#define BSDL_MODE_MSG_FATAL    (1 <<  3)

#define BSDL_MODE_MSG_ALL      (BSDL_MODE_MSG_FATAL | \
                                BSDL_MODE_MSG_ERR   | \
                                BSDL_MODE_MSG_WARN  | \
                                BSDL_MODE_MSG_NOTE)
#define BSDL_MODE_MSG_ALWAYS   BSDL_MODE_MSG_FATAL

#define BSDL_MODE_SYN_CHECK    (1 <<  4)
#define BSDL_MODE_INSTR_PRINT  (1 <<  5)
#define BSDL_MODE_INSTR_EXEC   (1 <<  6)
#define BSDL_MODE_IDCODE_CHECK (1 <<  7)
#define BSDL_MODE_ACTION_ALL   (BSDL_MODE_SYN_CHECK   | \
                                BSDL_MODE_INSTR_PRINT | \
                                BSDL_MODE_INSTR_EXEC  | \
                                BSDL_MODE_IDCODE_CHECK)

#define BSDL_MODE_INCLUDE1     (BSDL_MODE_MSG_ALWAYS)
#define BSDL_MODE_INCLUDE2     (BSDL_MODE_SYN_CHECK  | \
                                BSDL_MODE_INSTR_EXEC | \
                                BSDL_MODE_MSG_WARN   | \
                                BSDL_MODE_MSG_ERR    | \
                                BSDL_MODE_MSG_FATAL)
#define BSDL_MODE_DETECT       (BSDL_MODE_SYN_CHECK    | \
                                BSDL_MODE_INSTR_EXEC   | \
                                BSDL_MODE_IDCODE_CHECK | \
                                BSDL_MODE_MSG_ALWAYS)
#define BSDL_MODE_TEST         (BSDL_MODE_SYN_CHECK  | \
                                BSDL_MODE_MSG_ALL)
#define BSDL_MODE_DUMP         (BSDL_MODE_SYN_CHECK   | \
                                BSDL_MODE_INSTR_PRINT | \
                                BSDL_MODE_MSG_WARN    | \
                                BSDL_MODE_MSG_ERR     | \
                                BSDL_MODE_MSG_FATAL)

#endif /* BSDL_MODE_H */
