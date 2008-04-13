@echo off
set INCLUDE=%INCLUDE%;%INCLUDE_TOOLKIT%;%INCLUDE_WATCOM%
set LIB=%LIB%;%LIB_TOOLKIT%;%LIB_WATCOM%
\IbmC\bin\icc /Gm+ /W2 /C /Ms crc32.c
if errorlevel 1 goto End
ilink crc32.def crc32.obj ..\..\..\JimiHelp\stdcode\globstr.obj ..\..\..\JimiHelp\asm.32\crcs.obj
:End
