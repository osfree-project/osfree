@echo off
@set os2_shell >nul 2>&1
if errorlevel 2 goto _win

:_os2
call build-os2 %1 %2 %3 %4 %5 %6 %7 %8 %9
exit

:_win
call build-w64 %1 %2 %3 %4 %5 %6 %7 %8 %9
