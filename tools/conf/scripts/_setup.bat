@echo off
setlocal enabledelayedexpansion

rem ---------------------------------------------------------------
rem  _setup.cmd  -  загрузка и установка osFree Build Environment
rem ---------------------------------------------------------------

if !%root%!==!! (
  echo This script is not standalone
  goto exit
)

rem Пути к утилитам (лежат рядом со скриптом)
set "WGET=%root%tools\conf\wget-win32.exe"
set "UNZIPEXE=%root%tools\conf\unzipwin32.exe"

rem Проверка наличия необходимых утилит
if not exist "%WGET%" (
  echo ERROR: wget-win32.exe not found at %WGET%
  goto exit
)
if not exist "%UNZIPEXE%" (
  echo ERROR: unzipwin32.exe not found at %UNZIPEXE%
  goto exit
)

rem Каталог установки (фиксированный)
set "INSTALLPATH=%root%osFreeBE"

rem Список зеркал (основной + резервные)
set "MIRRORS[0]=https://mirror.osfree.org/upload/osFreeBE"
set "MIRRORS[1]=ftp://ftp.osfree.org/upload/osFreeBE"
set "MIRRORS[2]=https://ftp.osfree.org/upload/osFreeBE"
set "MIRRORS[3]=ftp://mirror.osfree.org/upload/osFreeBE"
set "MIRROR_COUNT=3"

set "TMPZIP=%tmp%\osFreeBE.zip"
set "TMPHASH=%tmp%\osFreeBE.zip.sha256"

echo ---------------------------------------------------------------
echo Welcome to the osFree Build Environment installer!
echo.
echo Target installation directory: %INSTALLPATH%
echo ---------------------------------------------------------------
set "SUCCESS=0"

for /l %%m in (0,1,%MIRROR_COUNT%) do (
  if !%SUCCESS%! == !0! (
    set "BASE_URL=!MIRRORS[%%m]!"
    if not "!BASE_URL!"=="" (
      echo.
      echo Trying mirror: !BASE_URL!
      echo [1/2] Downloading osFreeBE.zip...
      "%WGET%" --no-check-certificate "!BASE_URL!/osFreeBE.zip" -O "%TMPZIP%" -c
      if not errorlevel 1 (
        echo Download succeeded.
        goto install
      )
      echo FAILED: Could not download archive from this mirror.
      if exist "%TMPZIP%" del "%TMPZIP%" >nul 2>&1
    )
  )
)

:install
if !%SUCCESS%! neq !1! (
  echo.
  echo ERROR: Could not download a valid osFreeBE.zip from any mirror.
  goto exit
)

echo.
echo [2/2] Extracting osFreeBE %TMPZIP% to %INSTALLPATH%...
if not exist "%INSTALLPATH%" mkdir "%INSTALLPATH%"
set path=%root%\tools\conf;%path%
%UNZIPEXE% -o %TMPZIP% -d "%INSTALLPATH%"
if errorlevel 1 (
  echo ERROR: Failed to extract the archive.
  del "%TMPZIP%" "%TMPHASH%" >nul 2>&1
  goto exit
)

echo Cleaning up temporary files...
if exist "%TMPZIP%" del "%TMPZIP%" >nul 2>&1
if exist "%TMPHASH%" del "%TMPHASH%" >nul 2>&1

echo.
echo ---------------------------------------------------------------
echo osFree Build Environment has been successfully installed!
echo.
echo You can now run _wcc.cmd from any project directory.
echo ---------------------------------------------------------------

:exit
endlocal
