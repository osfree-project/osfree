@echo off

set vpbase=f:\dev\vp21
set watcom=f:\dev\watcom
set PATH=%vpbase%\bin.os2;%watcom%\binp;%PATH%
rem set INCLUDE=f:\os2tk45\h;%INCLUDE%
set BEGINLIBPATH = %vpbase%\bin.os2;

set vpcopt=/$P+ /$S- /$Speed- /$I- /$AlignCode- /$AlignData- /$Delphi+ /ucommon;%vpbase%\units.os2;%vpbase%\source\rtl;%vpbase%\source\os2 /i%vpbase%\source\rtl /l%vpbase%\lib.os2 /oout /m
rem set rcopt=-r -n -i %vpbase%\source\rtl
set rcopt=-bt=os2 -r -zm -i=%vpbase%\source\rtl -i=%watcom%\h\os2

if .%_4ver%. == .. goto okay
echo This batch file is best viewed with CMD.EXE :-)
cmd /c %0
exit

:okay
mkdir out 1>nul 2>nul

rem ************* first, compile resources ***************

if exist out\sysIcons.res goto skip1
wrc %rcopt% sysIcons.rc -fo=out\sysIcons.res
if errorlevel == 1 goto Error
:skip1

if exist out\lxlite.res goto skip2
wrc %rcopt% lxLite.rc -fo=out\lxLite.res
if errorlevel == 1 goto Error
:skip2

if exist os2api.res goto skip3
wrc %rcopt% os2api.rc -fo=out\os2api.res
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

rem copy out\lxLite.exe     ..\lxLite\      1>nul
rem copy out\chCase.exe     ..\lxLite\      1>nul
rem copy out\noEA.exe       ..\lxLite\      1>nul
rem copy out\sysIcons.exe   ..\lxLite\      1>nul
rem copy out\unLock.exe     ..\lxLite\      1>nul

rem echo Well done. Now change to ..\ directory and run m_lxLite.cmd

exit

:Error
echo Project not completed
