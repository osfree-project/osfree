@echo off

if NOT .%VPDIR%. == .. goto vpdirok
echo Please set the VPDIR environement to your VirtualPascal installation
goto End

:vpdirok
if NOT .%VPBASEDONE%. == .. goto vpbasedone
set vpbase=%VPDIR%
set PATH=%vpbase%\bin.os2;%PATH%
set BEGINLIBPATH=%vpbase%\bin.os2;%BEGINLIBPATH%;
set vpbasedone=1

:vpbasedone
set vpcopt=/$P+ /CO /$S- /$Speed- /$I- /$AlignCode- /$AlignData- /ucommon;%vpbase%\units.os2;%vpbase%\source\rtl;%vpbase%\source\os2 /i%vpbase%\source\rtl /l%vpbase%\lib.os2 /oout /m
set rcopt=-r -n -i %vpbase%\source\rtl

if .%DEBUG%. == .. goto nodebug
echo Building with debug
set vpcopt=/$D+ %vpcopt%
echo vpcopt=%VPCOPT%
:nodebug

if .%_4ver%. == .. goto okay
echo This batch file is designed for CMD.EXE
goto End

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

rem if exist out\lxlite.exe goto skip4
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

echo Well done.
echo Now test it all and create the rpm afterwards.
echo If you dislike rpm you could create a distribution via notUsed\m_lxLite.cmd
goto End

:Error
echo Project not completed

:End
