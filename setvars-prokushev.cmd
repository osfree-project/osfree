@echo off

set WATCOM=c:\watcom
set ROOT=c:\_work\svn\osfree

set FPPATH=c:\programs\pp\bin\os2

set TOOLS=%ROOT%\tools\bin
set PATH=%WATCOM%\binp;%TOOLS%;%FPPATH%;%PATH%
set INCLUDE=%WATCOM%\h;%WATCOM%\h\dos;%WATCOM%\h\os2;%WATCOM%\h\win
set FINCLUDE=%WATCOM%\src\fortran
set EDPATH=%WATCOM%\eddat
set HELP=%WATCOM%\binp\help;%HELP%
set BOOKSHELF=%WATCOM%\binp\help;%BOOKSHELF%
set BEGINLIBPATH=%WATCOM%\binp\dll
set LIBOS2=%WATCOM%\lib386\os2
set LIB=%WATCOM%\lib286;%WATCOM%\lib286\dos;%WATCOM%\lib286\win;%ROOT%\lib
