@echo off
setlocal
rem Edit here to use your own *.conf
rem set comspec=d:\os2\cmd.exe
rem set os2_shell=d:\os2\cmd.exe
rem set conf=git.conf
rem --------------------------------
rem c:\rexx\regina setenv.cmd %conf% >nul 2>&1
call setvars-w32
wmake -h %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
