@echo off
set ROOT=%~dp0

set WATCOM=c:\watcom
set svn=\data\dev\svn-win32-1.6.6\bin
set tools=%root%\bin\tools
set FPPATH=c:\fpc\3.2.2\bin\i386-win32
set imgdir=\data\vm\img
set mkisofs=\data\dev\cdrtools\mkisofs.exe
set serverenv=win32
set env=WIN64
set os=WIN64
set REGINA=c:\rexx
set REXX=%REGINA%\regina
set SERVERENV=win32

set PATH=%WATCOM%\binnt;%WATCOM%\binw;%TOOLS%;%FPPATH%;%svn%;%regina%;\data\dev\qemu;\data\dev\cdrtools;\data\dev\Bochs-2.3.5;\data\dev\bin;\usr\local\wbin;%PATH%;
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os21x;%WATCOM%\h\dos;%WATCOM%\h\nt
set LIB=%WATCOM%\lib386\nt
set WD_PATH=%WATCOM%\binnt
set LANG=%WATCOM%
set WIPFC=%WATCOM%\wipfc
