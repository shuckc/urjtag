;
; $Id: UrJTAG.nsi 1205 2008-05-06 15:35:49Z kawk $
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
; the CFLAGS=-mno-cygwin setting.
;
; TODO: Install DLLs for cables (InpOut32.DLL, FTD2XX.DLL etc.)


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
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
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


;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN

  !insertmacro MUI_DESCRIPTION_TEXT ${SecExe} \
    "UrJTAG executable"

  !insertmacro MUI_DESCRIPTION_TEXT ${SecDoc} \
    "Documentation for UrJTAG"

  !insertmacro MUI_DESCRIPTION_TEXT ${SecData} \
    "BSDL include files and part descriptions for autodetection"

!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  RMDir /r "$INSTDIR\doc"
  RMDir /r "$INSTDIR\data"
  Delete "$INSTDIR\jtag.exe"
  Delete "$INSTDIR\uninst.exe"
  RMDir /r "$INSTDIR"
  DeleteRegKey /ifempty HKCU "Software\UrJTAG"

SectionEnd
