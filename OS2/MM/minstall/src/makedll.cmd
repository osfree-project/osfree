@echo off
rem
rem MINSTALL.DLL (c) Copyright 2002-2005 Martin Kiewitz
rem
rem This file is part of MINSTALL.DLL for OS/2 / eComStation
rem
rem MINSTALL.DLL is free software: you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License as published by
rem  the Free Software Foundation, either version 3 of the License, or
rem  (at your option) any later version.
rem
rem MINSTALL.DLL is distributed in the hope that it will be useful,
rem  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem  GNU General Public License for more details.
rem
rem You should have received a copy of the GNU General Public License
rem  along with MINSTALL.DLL.  If not, see <http://www.gnu.org/licenses/>.
rem
set INCLUDE=%INCLUDE%;%INCLUDE_TOOLKIT%;%INCLUDE_WATCOM%
set LIB=%LIB%;%LIB_TOOLKIT%;%LIB_WATCOM%
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_basescr.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_ctrlscr.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_ctrlprc.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_customdll.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_imports.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_inistuff.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_cardinfo.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_helper.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_install.c
if errorlevel 1 goto End
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms mmi_main.c
if errorlevel 1 goto End
ilink mmi_main.def mmi_main.obj mmi_basescr.obj mmi_ctrlscr.obj mmi_ctrlprc.obj mmi_customdll.obj mmi_imports.obj mmi_inistuff.obj mmi_cardinfo.obj mmi_helper.obj mmi_install.obj ..\..\JimiHelp\stdcode\file.obj ..\..\JimiHelp\stdcode\globstr.obj ..\..\JimiHelp\stdcode\mciini.obj ..\..\JimiHelp\stdcode\msg.obj ..\..\JimiHelp\stdcode\dll.obj ..\..\JimiHelp\asm.32\crcs.obj
del minstall.dll
ren mmi_main.dll minstall.dll
copy minstall.dll c:\mmos2\dll
copy minstall.dll d:\testcase\mmecs
:End
