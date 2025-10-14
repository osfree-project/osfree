@echo off
setlocal
call setvars-w64

where /q wmake
if errorlevel 1 (
    start /b /wait cmd /c _setup
    call setvars-w64
    where /q wmake
    if errorlevel 1 (
      echo Error configuring osFree Build Environment
      exit /b
    )
)

wmake -h %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal
