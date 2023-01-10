@echo off
rem canonicalize root
for %%i in (%root%) do set root=%%~fi\

set WATCOM=c:\watcom
set svn=\data\dev\svn-win32-1.6.6\bin
set FPPATH=c:\fpc\3.2.2\bin\i386-win32
set imgdir=\data\vm\img
set mkisofs=\data\dev\cdrtools\mkisofs.exe
set serverenv=win32
set env=WIN32
set os=WIN32
set host=win32
set os2tk=%root%build\bin\host\%host%\os2tk45
set tools=%root%build\bin\host\%host%\bin
set REGINA=c:\rexx
set REXX=%REGINA%\regina
set SERVERENV=win32

set PATH=e:\win32\bin;%WATCOM%\binnt;%WATCOM%\binw;%TOOLS%;%OS2TK%\bin;%OS2TK%\som\bin;%FPPATH%;%svn%;%regina%;\data\dev\qemu;\data\dev\cdrtools;\data\dev\Bochs-2.3.5;\data\dev\bin;\usr\local\wbin;%PATH%;
set INCLUDE=%WATCOM%\h;%WATCOM%\h\os21x;%WATCOM%\h\dos;%WATCOM%\h\nt
set LIB=%WATCOM%\lib386\nt
set WD_PATH=%WATCOM%\binnt
rem set LANG=%WATCOM%
set WIPFC=%WATCOM%\wipfc
