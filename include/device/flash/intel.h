/*
 * $Id$
 *
 * Copyright (C) 2002 ETC s.r.o.
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
 * Written by Marcel Telka <marcel@telka.sk>, 2002.
 *
 * Documentation:
 * [1] Intel Corporation, "3 Volt Intel Strata Flash Memory 28F128J3A, 28F640J3A,
 *     28F320J3A (x8/x16)", April 2002, Order Number: 290667-011
 * [2] Intel Corporation, "3 Volt Synchronous Intel Strata Flash Memory 28F640K3, 28F640K18,
 *     28F128K3, 28F128K18, 28F256K3, 28F256K18 (x16)", June 2002, Order Number: 290737-005
 *
 */

#ifndef	FLASH_INTEL_H
#define	FLASH_INTEL_H

/* Intel CFI commands - see Table 4. in [1] and Table 3. in [2] */

#define	CFI_CMD_INTEL_READ_ARRAY		0xFF	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_READ_IDENTIFIER		0x90	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_READ_QUERY		0x98	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_READ_STATUS_REGISTER	0x70	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_CLEAR_STATUS_REGISTER	0x50	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_CONFIRM			0xD0	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_PROGRAM1			0x40	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_PROGRAM2			0x10	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_WRITE_TO_BUFFER		0xE8	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_BLOCK_ERASE		0x20	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_SUSPEND			0xB0	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */
#define	CFI_CMD_INTEL_RESUME			0xD0	/* 28FxxxJ3A, 28FxxxK3, 28FxxxK18 */

#endif /* FLASH_INTEL_H */
