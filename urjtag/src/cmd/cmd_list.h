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

_URJ_CMD(quit)
_URJ_CMD(help)
_URJ_CMD(frequency)
_URJ_CMD(cable)
_URJ_CMD(reset)
_URJ_CMD(discovery)
_URJ_CMD(idcode)
_URJ_CMD(detect)
_URJ_CMD(signal)
_URJ_CMD(scan)
_URJ_CMD(salias)
_URJ_CMD(bit)
_URJ_CMD(register)
_URJ_CMD(initbus)
_URJ_CMD(print)
_URJ_CMD(part)
_URJ_CMD(bus)
_URJ_CMD(instruction)
_URJ_CMD(shift)
_URJ_CMD(dr)
_URJ_CMD(get)
_URJ_CMD(test)
_URJ_CMD(shell)
_URJ_CMD(set)
_URJ_CMD(endian)
_URJ_CMD(peek)
_URJ_CMD(poke)
_URJ_CMD(pod)
_URJ_CMD(readmem)
_URJ_CMD(writemem)
_URJ_CMD(detectflash)
_URJ_CMD(flashmem)
_URJ_CMD(eraseflash)
_URJ_CMD(script)
_URJ_CMD(include)
_URJ_CMD(addpart)
_URJ_CMD(usleep)
#ifdef ENABLE_SVF
_URJ_CMD(svf)
#endif
#ifdef ENABLE_BSDL
_URJ_CMD(bsdl)
#endif
_URJ_CMD(debug)
_URJ_CMD(bfin)

#undef _URJ_CMD
