@echo off
set ROOT=k:\var\src\os2\svn\osfree

set WATCOM=k:\watcom
set svn=%root%\..\svnw32\bin
SET REGINA=k:\regina
set tools=%src%\tools\bin
set TKPATH=k:\os2tk45
set FPPATH=k:\fpwin

set PATH=%WATCOM%\binp;%TOOLS%;%TKPATH%\bin;%FPPATH%\bin\win32;%svn%;%regina%;%PATH%
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os21x;%WATCOM%\h\dos
set LIB=%WATCOM%\lib386\nt
set PATH=%WATCOM%\binnt;%PATH%
set WD_PATH=%WATCOM%\binnt
set LANG=%WATCOM%
