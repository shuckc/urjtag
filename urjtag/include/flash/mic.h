/*
 * $Id$
 *
 * Manufacturer's Identification Code declarations
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
 * [1] JEDEC Solid State Technology Association, "Standard Manufacturer's
 *     Identification Code", May 2003, Order Number: JEP106M
 *
 */

#ifndef STD_MIC_H
#define STD_MIC_H

/* Manufacturer's Identification Code - see Table 1 in [1] */

#define STD_MIC_AMD                             0x01
#define STD_MICN_AMD                            "AMD"
#define STD_MIC_AMI                             0x02
#define STD_MICN_AMI                            "AMI"
#define STD_MIC_FAIRCHILD                       0x83
#define STD_MICN_FAIRCHILD                      "Fairchild"
#define STD_MIC_FUJITSU                         0x04
#define STD_MICN_FUJITSU                        "Fujitsu"
#define STD_MIC_GTE                             0x85
#define STD_MICN_GTE                            "GTE"
#define STD_MIC_HARRIS                          0x86
#define STD_MICN_HARRIS                         "Harris"
#define STD_MIC_HITACHI                         0x07
#define STD_MICN_HITACHI                        "Hitachi"
#define STD_MIC_INMOS                           0x08
#define STD_MICN_INMOS                          "Inmos"
#define STD_MIC_INTEL                           0x89
#define STD_MICN_INTEL                          "Intel"
#define STD_MIC_ITT                             0x8A
#define STD_MICN_ITT                            "I.T.T."
#define STD_MIC_INTERSIL                        0x0B
#define STD_MICN_INTERSIL                       "Intersil"
#define STD_MIC_MONOLITHIC_MEMORIES             0x8C
#define STD_MICN_MONOLITHIC_MEMORIES            "Monolithic Memories"
#define STD_MIC_MOSTEK                          0x0D
#define STD_MICN_MOSTEK                         "Mostek"
#define STD_MIC_MOTOROLA                        0x0E
#define STD_MICN_MOTOROLA                       "Motorola"
#define STD_MIC_NATIONAL                        0x8F
#define STD_MICN_NATIONAL                       "National"
#define STD_MIC_NEC                             0x10
#define STD_MICN_NEC                            "NEC"
#define STD_MIC_RCA                             0x91
#define STC_MICN_RCA                            "RCA"
#define STD_MIC_RAYTHEON                        0x92
#define STD_MICN_RAYTHEON                       "Raytheon"
#define STD_MIC_CONEXANT                        0x13
#define STD_MICN_CONEXANT                       "Conexant (Rockwell)"
#define STD_MIC_SEEQ                            0x94
#define STD_MICN_SEEQ                           "Seeq"
#define STD_MIC_PHILIPS                         0x15
#define STD_MICN_PHILIPS                        "Philips Semi. (Signetics)"
#define STD_MIC_SYNERTEK                        0x16
#define STD_MICN_SYNERTEK                       "Synertek"
#define STD_MIC_TEXAS_INSTRUMENTS               0x97
#define STD_MICN_TEXAS_INSTRUMENTS              "Texas Instruments"
#define STD_MIC_TOSHIBA                         0x98
#define STD_MICN_TOSHIBA                        "Toshiba"
#define STD_MIC_XICOR                           0x19
#define STD_MICN_XICOR                          "Xicor"
#define STD_MIC_ZILOG                           0x1A
#define STD_MICN_ZILOG                          "Zilog"
#define STD_MIC_EUROTECHNIQUE                   0x9B
#define STD_MICN_EUROTECHNIQUE                  "Eurotechnique"
#define STD_MIC_MITSUBISHI                      0x1C
#define STD_MICN_MITSUBISHI                     "Mitsubishi"
#define STD_MIC_LUCENT                          0x9D
#define STD_MICN_LUCENT                         "Lucent (AT&T)"
#define STD_MIC_EXEL                            0x9E
#define STD_MICN_EXEL                           "Exel"
#define STD_MIC_ATMEL                           0x1F
#define STD_MICN_ATMEL                          "Atmel"
#define STD_MIC_SGS_THOMSON                     0x20
#define STD_MICN_SGS_THOMSON                    "SGS/Thomson"
#define STD_MIC_LATTICE                         0xA1
#define STD_MICN_LATTICE                        "Lattice Semi."
#define STD_MIC_NCR                             0xA2
#define STD_MICN_NCR                            "NCR"
#define STD_MIC_WAFER_SCALE_INTEGRATION         0x23
#define STD_MICN_WAFER_SCALE_INTEGRATION        "Wafer Scale Integration"
#define STD_MIC_IBM                             0xA4
#define STD_MICN_IBM                            "IBM"
#define STD_MIC_TRISTAR                         0x25
#define STD_MICN_TRISTAR                        "Tristar"
#define STD_MIC_VISIC                           0x26
#define STD_MICN_VISIC                          "Visic"
#define STD_MIC_INTL_CMOS_TECHNOLOGY            0xA7
#define STD_MICN_INTL_CMOS_TECHNOLOGY           "Intl. CMOS Technology"
#define STD_MIC_SSSI                            0xA8
#define STD_MICN_SSSI                           "SSSI"
#define STD_MIC_MICROCHIP_TECHNOLOGY            0x29
#define STD_MICN_MICROCHIP_TECHNOLOGY           "MicrochipTechnology"
#define STD_MIC_RICOH                           0x2A
#define STD_MICN_RICOH                          "Ricoh Ltd."
#define STD_MIC_VLSI                            0xAB
#define STD_MICN_VLSI                           "VLSI"
#define STD_MIC_MICRON_TECHNOLOGY               0x2C
#define STD_MICN_MICRON_TECHNOLOGY              "Micron Technology"
#define STD_MIC_HYUNDAI_ELECTRONICS             0xAD
#define STD_MICN_HYUNDAI_ELECTRONICS            "Hyundai Electronics"
#define STD_MIC_OKI_SEMICONDUCTOR               0xAE
#define STD_MICN_OKI_SEMICONDUCTOR              "OKI Semiconductor"
#define STD_MIC_ACTEL                           0x2F
#define STD_MICN_ACTEL                          "ACTEL"
#define STD_MIC_SHARP                           0xB0
#define STD_MICN_SHARP                          "Sharp"
#define STD_MIC_CATALYST                        0x31
#define STD_MICN_CATALYST                       "Catalyst"
#define STD_MIC_PANASONIC                       0x32
#define STD_MICN_PANASONIC                      "Panasonic"
#define STD_MIC_IDT                             0xB3
#define STD_MICN_IDT                            "IDT"
#define STD_MIC_CYPRESS                         0x34
#define STD_MICN_CYPRESS                        "Cypress"
#define STD_MIC_DEC                             0xB5
#define STD_MICN_DEC                            "DEC"
#define STD_MIC_LSI_LOGIC                       0xB6
#define STD_MICN_LSI_LOGIC                      "LSI Logic"
#define STD_MIC_ZARLINK                         0x37
#define STD_MICN_ZARLINK                        "Zarlink (formerly Plessey)"
#define STD_MIC_UTMC                            0x38
#define STD_MICN_UTMC                           "UTMC"
#define STD_MIC_THINKING_MACHINE                0xB9
#define STD_MICN_THINKING_MACHINE               "Thinking Machine"
#define STD_MIC_THOMSON_CSF                     0xBA
#define STD_MICN_THOMSON_CSF                    "Thomson CSF"
#define STD_MIC_INTEGRATED_CMOS                 0x3B
#define STD_MICN_INTEGRATED_CMOS                "Integrated CMOS(Vertex)"
#define STD_MIC_HONEYWELL                       0xBC
#define STD_MICN_HONEYWELL                      "Honeywell"
#define STD_MIC_TEKTRONIX                       0x3D
#define STD_MICN_TEKTRONIX                      "Tektronix"
#define STD_MIC_SUN_MICROSYSTEMS                0x3E
#define STD_MICN_SUN_MICROSYSTEMS               "Sun Microsystems"
#define STD_MIC_SST                             0xBF
#define STD_MICN_SST                            "SST"
#define STD_MIC_MOSEL                           0x40
#define STD_MICN_MOSEL                          "MOSEL"
#define STD_MIC_INFINEON                        0xC1
#define STD_MICN_INFINEON                       "Infineon (formerly Siemens)"
#define STD_MIC_MACRONIX                        0xC2
#define STD_MICN_MACRONIX                       "Macronix"
#define STD_MIC_XEROX                           0x43
#define STD_MICN_XEROX                          "Xerox"
#define STD_MIC_PLUS_LOGIC                      0xC4
#define STD_MICN_PLUS_LOGIC                     "Plus Logic"
#define STD_MIC_SUNDISK                         0x45
#define STD_MICN_SUNDISK                        "SunDisk"
#define STD_MIC_ELAN_CIRCUIT                    0x46
#define STD_MICN_ELAN_CIRCUIT                   "Elan Circuit Tech."
/* TODO */

#endif /* STD_MIC_H */
