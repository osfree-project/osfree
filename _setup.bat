@echo off
rem Install wget
choco install wget unzip --no-progress

rem Download latest Open Watcom
rem wget https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-win-x64.exe -P "c:/Documents and settings/Downloads"
wget ftp://ftp.osfree.org/upload/ow/ow-1.9.zip -P "c:/Documents and Settings/Downloads"

rem Install OpenWatcom
rem "c:/Documents and settings/Downloads/open-watcom-2_0-c-win-x64.exe" -s -p -ns
unzip "c:/Documents and Settings/Downloads/ow-1.9.zip" -d c:\

rem Download Regina REXX
wget https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.4/Regina394w32.exe -P "c:/Documents and settings/Downloads"

rem Install Regina REXX
"c:\Documents and Settings\Downloads\Regina394w32.exe" /S /D=C:\REXX

rem Download FPC
wget  https://downloads.sourceforge.net/project/freepascal/Win32/3.2.2/fpc-3.2.2.i386-win32.exe -P "c:/Documents and settings/Downloads"

rem install fpc
"c:\Documents and Settings\Downloads\fpc-3.2.2.i386-win32.exe" /silent

rem build
rem call build-git.cmd
