@echo off
setlocal
rem Edit here to use your own *.conf
rem comspec=d:\os2\cmd.exe
rem set os2_shell=d:\os2\cmd.exe
set conf=%root%\conf\build.conf
rem --------------------------------
call setenv.cmd %conf% >nul 2>&1
wmake -h %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
