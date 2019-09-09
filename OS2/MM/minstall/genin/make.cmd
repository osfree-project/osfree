@echo off
set INCLUDE=%INCLUDE%;%INCLUDE_TOOLKIT%;%INCLUDE_WATCOM%
set LIB=%LIB%;%LIB_TOOLKIT%;%LIB_WATCOM%
\IbmC\bin\icc /Ge- /Gm+ /W2 /C /Ms genin.c
if errorlevel 1 goto End
ilink genin.def genin.obj
copy genin.dll c:\mmos2\dll
:End
