@echo off
setlocal
rem Edit here to use your own *.conf
set conf=valerius.conf
rem --------------------------------
call setenv.cmd %conf% >nul 2>&1
wmake -h %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
