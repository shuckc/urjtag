;
; $Id$
;
; Script to create Installer for Windows platforms using 
;   "nullsoft scriptable install system" (NSIS)
;   (available from http://nsis.sourceforge.net)
;
; Copyright (C) 2009 K. Waschk
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
; 02111-1307, USA.
;
; Written by K. Waschk, 2009
; Based on "Modern UI Basic Example Script" by Joost Verburg
;
; Last tested with NSIS version 2.42
;
; Run makensis.exe in the root of an UrJTAG source directory
; extracted from a distributed archive of UrJTAG (make dist)
; after configuring and compiling. Usually you want to compile
; with --with-ftd2xx, --with-inpout32, --enable-relocatable and
; the CFLAGS=-mno-cygwin setting. To make UrJTAG search for
; its data files and BSDL declarations in the correct path,
; add JTAG_BIN_DIR and JTAG_DATA_DIR as follows to the CFLAGS
; on the same line together with ./configure and its options
; (this is used for building the UrJTAG.exe distributable):
;
;  CFLAGS="-mno-cygwin -O2 -DJTAG_BIN_DIR=\\\"/\\\" -DJTAG_DATA_DIR=\\\"/data\\\""
;  ./configure --enable-relocatable \
;       --with-ftd2xx=/tmp/FTDI_CDM_204 \
;       --with-libusb=/tmp/LibUSB-Win32_112 \
;       --with-inpout32
;
; This script now expects InpOut32.dll in the current directory
; as well. You can get an InpOut32.dll that works on 32 bit AND
; 64 bit Windows, including Vista, from 
;     http://www.highrez.co.uk/Downloads/InpOut32/
;
; No drivers for FTDI cables are installed. FTD2XX.DLL must be
; in your PATH somewhere. It usually is installed with the cable
; drivers.
;
;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "UrJTAG"
  OutFile "UrJTAG.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\UrJTAG"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\UrJTAG" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\UrJTAG" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "UrJTAG executable" SecExe

  SetOutPath "$INSTDIR"
  File src\jtag.exe
  File inpout32.dll
  WriteRegStr HKCU "Software\UrJTAG" "" $INSTDIR
  WriteUninstaller "$INSTDIR\uninst.exe"

SectionEnd

Section "Documentation" SecDoc

  SetOutPath "$INSTDIR\doc"
  File doc\UrJTAG.txt
  WriteRegStr HKCU "Software\UrJTAG" "" $INSTDIR
  WriteUninstaller "$INSTDIR\uninst.exe"

SectionEnd

Section "Data files" SecData

  SetOutPath "$INSTDIR\data"
  File /r /x Makefile /x Makefile.am /x Makefile.in data\*
  WriteRegStr HKCU "Software\UrJTAG" "" $INSTDIR
  WriteUninstaller "$INSTDIR\uninst.exe"

SectionEnd

Section "Start Menu Entries" SecStartMenu

 !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\JTAG Shell.lnk" "$INSTDIR\jtag.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Documentation.lnk" "$INSTDIR\doc\UrJTAG.txt"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd



;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN

  !insertmacro MUI_DESCRIPTION_TEXT ${SecExe} \
    "UrJTAG executable"

  !insertmacro MUI_DESCRIPTION_TEXT ${SecDoc} \
    "Documentation for UrJTAG"

  !insertmacro MUI_DESCRIPTION_TEXT ${SecData} \
    "BSDL include files and part descriptions for autodetection"

  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} \
    "Links to UrJTAG in Start Menu"

!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  RMDir /r "$INSTDIR\doc"
  RMDir /r "$INSTDIR\data"
  Delete "$INSTDIR\jtag.exe"
  Delete "$INSTDIR\uninst.exe"
  RMDir /r "$INSTDIR"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey /ifempty HKCU "Software\UrJTAG"

SectionEnd
