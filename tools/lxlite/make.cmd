@echo off

set vpbase=O:\vp21
set PATH=%vpbase%\bin.os2;%PATH%
set BEGINLIBPATH = %vpbase%\bin.os2;

set vpcopt=/$P+ /$S- /$Speed- /$I- /$AlignCode- /$AlignData- /ucommon;%vpbase%\units.os2;%vpbase%\source\rtl;%vpbase%\source\os2 /i%vpbase%\source\rtl /l%vpbase%\lib.os2 /oout /m
set rcopt=-r -n -i %vpbase%\source\rtl

if .%_4ver%. == .. goto okay
echo This batch file is best viewed with CMD.EXE :-)
cmd /c %0
exit

:okay
mkdir out 1>nul 2>nul

rem ************* first, compile resources ***************

if exist sysIcons.res goto skip1
rc %rcopt% sysIcons.rc
if errorlevel == 1 goto Error
:skip1

if exist lxlite.res goto skip2
rc %rcopt% lxLite.rc
if errorlevel == 1 goto Error
:skip2

if exist os2api.res goto skip3
rc %rcopt% os2api.rc
if errorlevel == 1 goto Error
:skip3

if exist out\lxlite.exe goto skip4
vpc %vpcopt% lxLite.pas
if errorlevel == 1 goto Error
:skip4

if exist out\chCase.exe goto skip5
vpc %vpcopt% chCase.pas
if errorlevel == 1 goto Error
:skip5

if exist out\noEA.exe goto skip6
vpc %vpcopt% noEA.pas
if errorlevel == 1 goto Error
:skip6

if exist out\sysIcons.exe goto skip7
vpc %vpcopt% sysIcons.pas
if errorlevel == 1 goto Error
:skip7

if exist out\unLock.exe goto skip8
vpc %vpcopt% unLock.pas
if errorlevel == 1 goto Error
:skip8

copy out\lxLite.exe	..\lxLite\	1>nul
copy out\chCase.exe	..\lxLite\	1>nul
copy out\noEA.exe	..\lxLite\	1>nul
copy out\sysIcons.exe	..\lxLite\	1>nul
copy out\unLock.exe	..\lxLite\	1>nul

echo Well done. Now change to ..\ directory and run m_lxLite.cmd

exit

:Error
echo Project not completed
