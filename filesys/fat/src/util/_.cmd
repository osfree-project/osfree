setlocal
set watcom=f:\dev\watcom
call %WATCOM%\owsetenv.cmd
wmake -f makefile.wcc
endlocal
