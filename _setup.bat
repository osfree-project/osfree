@echo off
rem Install wget
choco install wget --no-progress

rem Download latest Open Watcom
wget https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-win-x64.exe
setup.exe  -s p -ns
dir

rem Install OpenWatcom
open-watcom-2_0-c-win-x64.exe -s -p -ns

rem Download Regina REXX
wget https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.4/Regina394w64.exe

rem Install Regina REXX
Regina394w64.exe /S /D=C:\REXX

rem Download FPC
wget  https://downloads.sourceforge.net/project/freepascal/Win32/3.2.2/fpc-3.2.2.i386-win32.exe
dir

rem install fpc
fpc-3.2.2.i386-win32.exe /silent

rem build
rem set
rem dir
rem call build-git.cmd
