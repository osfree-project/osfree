@echo off
set ROOT=c:\a\osfree\osfree\

set WATCOM=c:\watcom
set svn=\data\dev\svn-win32-1.6.6\bin
set tools=%root%\bin\tools
set FPPATH=с:\fpc\3.2.2\bin\i386-win32
set imgdir=\data\vm\img
set mkisofs=\data\dev\cdrtools\mkisofs.exe
set serverenv=win32
set os=WIN32

set PATH=%WATCOM%\binnt64;%WATCOM%\binw;%TOOLS%;%FPPATH%;%svn%;%regina%;\data\dev\qemu;\data\dev\cdrtools;\data\dev\Bochs-2.3.5;\data\dev\bin;\usr\local\wbin;%PATH%;
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os21x;%WATCOM%\h\dos
set LIB=%WATCOM%\lib386\nt
set WD_PATH=%WATCOM%\binnt64
set LANG=%WATCOM%
