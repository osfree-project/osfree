@echo off
set root=.
:loop
if exist "%root%\tools\mk\all.mk" goto found
set root=%root%\..
goto loop
:found
set path=%root%\tools\conf\scripts;%path%
call build %1 %2 %3 %4 %5 %6 %7 %8 %9
