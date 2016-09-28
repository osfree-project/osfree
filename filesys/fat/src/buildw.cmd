rem
rem Build with OpenWatcom tools
rem
setlocal
set d=p
SET DEBUG=0
set WATCOM=f:\dev\watcom
set TOOLKIT=f:\os2tk45
set PATH=%toolkit%\bin;%watcom%\binp;\tools\bin;%path%
set BEGINLIBPATH=%toolkit%\dll;%watcom%\binp\dll;\tools\dll
call %WATCOM%\owsetenv.cmd
wmake -f makefile.wcc %1
endlocal
