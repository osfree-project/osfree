@echo off

set WATCOM=c:\watcom
set ROOT=%GITHUB_WORKSPACE%\

set TKPATH=f:\os2tk45
set FPPATH=f:\dev\pp

set TOOLS=%ROOT%\bin\tools
set PATH=%WATCOM%\binnt64;%TOOLS%;%TKPATH%\bin;%FPPATH%\bin\os2;%PATH%
set INCLUDE=%WATCOM%\h;%WATCOM%\h\dos;%WATCOM%\h\os2
set FINCLUDE=%WATCOM%\src\fortran
set EDPATH=%WATCOM%\eddat
set HELP=%WATCOM%\binp\help;%HELP%
set BOOKSHELF=%WATCOM%\binp\help;%BOOKSHELF%
set BEGINLIBPATH=%WATCOM%\binp\dll
set LIBOS2=%WATCOM%\lib386\os2
set LIB=%WATCOM%\lib286;%WATCOM%\lib286\dos
rem IBM Classic REXX
rem set OS_REXX=Classic
rem IBM OREXX
rem set OS_REXX=Object
rem Regina REXX
rem set OS_REXX   = Regina
rem Special case
set OS_REXX=c:\rexx\regina.exe
set REXX=c:\rexx\regina.exe
SET OS=Windows


