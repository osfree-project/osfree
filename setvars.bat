@echo off
set WATCOM=c:\watcom
set src=c:\tmp\boot

set tools=%src%\tools\bin
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os21x;%WATCOM%\h\dos
set LIB=%WATCOM%\lib386\nt
set PATH=%WATCOM%\binnt;%PATH%
set WD_PATH=%WATCOM%\binnt
set LANG=%WATCOM%
