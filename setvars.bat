@echo off
set ROOT=\data\src\svn\osfree\trunk\

set WATCOM=\data\dev\watcom
set svn=\data\dev\svn-win32-1.6.6\bin
set tools=%root%\bin\tools
SET REGINA=\data\dev\regina
set FPPATH=\data\dev\fpc\2.0.4\bin\i386-win32
set imgdir=\data\vm\img
set shell=c:\windows\cmd.exe
set rexx=\data\dev\regina\regina.exe
set mkisofs=\data\dev\cdrtools\mkisofs.exe
set serverenv=win32
set os=WIN32

set PATH=%WATCOM%\binnt;%TOOLS%;%FPPATH%;%svn%;%regina%;\data\dev\qemu;\data\dev\cdrtools;\data\dev\Bochs-2.3.5;\data\dev\bin;\usr\local\wbin;%PATH%;
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os21x;%WATCOM%\h\dos
set LIB=%WATCOM%\lib386\nt
set PATH=%WATCOM%\binnt;%PATH%
set WD_PATH=%WATCOM%\binnt
set LANG=%WATCOM%
