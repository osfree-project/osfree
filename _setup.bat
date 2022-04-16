@echo off
rem Install wget
choco install wget --no-progress

rem Download latest Open Watcom
wget https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-win-x64.exe -P "c:/Documents and settings/Downloads"
dir

rem Install OpenWatcom
"c:/Documents and settings/Downloads/open-watcom-2_0-c-win-x64.exe" -s -p -ns

rem Download Regina REXX
wget https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.4/Regina394w64.exe -P "c:/Documents and settings/Downloads"

rem Install Regina REXX
"c:/Documents and settings/Downloads/Regina394w64.exe" /S /D=C:\REXX

rem Download FPC
wget  https://downloads.sourceforge.net/project/freepascal/Win32/3.2.2/fpc-3.2.2.i386-win32.exe -P "c:/Documents and settings/Downloads"
dir

rem install fpc
"c:/Documents and settings/Downloads/fpc-3.2.2.i386-win32.exe" /silent

rem build
set
dir
rem call build-git.cmd
