@echo off
rem canonicalize root
for %%i in (%root%) do set root=%%~fi\

set osfreebe=C:\osFreeBE
rem %ProgramFiles(x86)%
set WATCOM=%osfreebe%\watcom
set svn=\data\dev\svn-win32-1.6.6\bin
set FPPATH=%osfreebe%\fpc\bin\i386-win32
set imgdir=\data\vm\img
set mkisofs=\data\dev\cdrtools\mkisofs.exe
set serverenv=win32
set env=WIN64
set os=WIN64
set host=win32
set patch=%root%\tools\conf\patch-win32.exe
set wget=%root%\tools\conf\wget-win32.exe
set os2tk=%root%build\bin\host\%host%\os2tk45
set tools=%root%build\bin\host\%host%\bin
set REGINA=%osfreebe%\REXX\binnt
set REXX="%REGINA%\regina.exe"
set SERVERENV=win32
set TMP=%TEMP%
set ARCH=x86_64

rem set PATH=e:\win32\bin;%WATCOM%\binnt;%WATCOM%\binw;%TOOLS%;%OS2TK%\bin;%OS2TK%\som\bin;%OS2TK%\som\common\dll;%FPPATH%;%svn%;%regina%;\data\dev\qemu;\data\dev\cdrtools;\data\dev\Bochs-2.3.5;\data\dev\bin;\usr\local\wbin;%PATH%;
set PATH=%OS2TK%\bin;%OS2TK%\som\bin;%OS2TK%\som\common\dll;%WATCOM%\binnt;%WATCOM%\binw;%TOOLS%;%FPPATH%;%svn%;%regina%;\data\dev\qemu;\data\dev\cdrtools;\data\dev\Bochs-2.3.5;\data\dev\bin;\usr\local\wbin;%PATH%;
set INCLUDE=%WATCOM%\h;%WATCOM%\h\dos;%WATCOM%\h\nt;
rem %WATCOM%\h\os21x;
set LIB=%osfreebe%\watcom\lib386\nt
set WD_PATH=%WATCOM%\binnt
rem set LANG=%WATCOM%
set WIPFC=%WATCOM%\wipfc

