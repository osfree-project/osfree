@echo off

if !%root%!==!! (
  echo This script is not standalone
  goto exit
)

rem modify path to find our wget and unzip
set path=%root%\tools\conf;%path%


rem say "hello"
cls
echo Welcome to the osFree Build Environment installer!
echo.
echo Now osFree Build Environment will be downloaded and installed...
echo.


:echo Enter osFree Build Environment install path (e.g.: d:\):
:set /P installpath=

set installpath=c:\osFreeBE

rem Download osFreeBE
if not exist "%tmp%/osFreeBE.zip" (
  wget-win32.exe ftp://ftp.osfree.org/upload/osFreeBE/osFreeBE.zip -P "%tmp%" -c
  if errorlevel 1 (
    echo Error downloading osFree build environment
    rem cleanup
    if exist "%tmp%\osFreeBE.zip" del "%tmp%\osFreeBE.zip"
    goto exit
  )
)

rem Install osFreeBE
unzipwin32 -o "%tmp%\osFreeBE.zip" -d "%installpath%"
if errorlevel 1 (
    echo Error unpacking osFree build environment
    rem cleanup
)

:exit
