@echo off
rem download wget and unzip for win32 manually
rem https://kumisystems.dl.sourceforge.net/project/mingw/Installer/mingw-get-setup.exe
rem https://eternallybored.org/misc/wget/1.19.4/32/wget.exe
rem set path=e:\win32\bin;%path%
rem or...
rem Install wget
choco install wget --no-progress

wget http://www2.cs.uidaho.edu/~jeffery/win32/unzip.exe -P "%tmp%" -c

rem Download latest Open Watcom
rem wget https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/open-watcom-2_0-c-win-x64.exe -P "c:/Documents and settings/Downloads"
rem wget ftp://ftp.osfree.org/upload/ow/ow-1.9.zip -P "c:/Documents and Settings/Downloads"
wget ftp://ftp.osfree.org/upload/ow/ow-1.9.zip -P "%tmp%" -c

rem Install OpenWatcom
rem "c:/Documents and settings/Downloads/open-watcom-2_0-c-win-x64.exe" -s -p -ns
"%tmp%\unzip" -o "%tmp%/ow-1.9.zip" -d c:\

rem Download JWASM
rem wget https://github.com/Baron-von-Riedesel/JWasm/releases/download/v2.15/JWasm_v215_win32.zip -P "c:/Documents and Settings/Downloads"
rem wget https://github.com/Baron-von-Riedesel/JWasm/releases/download/v2.15/JWasm_v215_win32.zip -P "%tmp%" -c

rem Install JWASM
rem "%tmp%\unzip" -o "%tmp%/JWasm_v215_win32.zip" JWasm.exe -d c:\watcom\binnt

rem Download Regina REXX
rem wget --no-check-certificate https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.4/Regina394w32.exe -P "c:/Documents and settings/Downloads"
wget --no-check-certificate https://sourceforge.net/projects/regina-rexx/files/regina-rexx/3.9.4/Regina394w32.exe -P "%tmp%" -c

rem Install Regina REXX
"%tmp%\Regina394w32.exe" /S /D=C:\REXX

rem Download FPC
rem wget --no-check-certificate https://downloads.sourceforge.net/project/freepascal/Win32/3.2.2/fpc-3.2.2.i386-win32.exe -P "c:/Documents and settings/Downloads"
rem wget --no-check-certificate https://downloads.sourceforge.net/project/freepascal/OS_2/3.2.2/os2322.zip -P "c:/Documents and settings/Downloads"
wget --no-check-certificate https://downloads.sourceforge.net/project/freepascal/Win32/3.2.2/fpc-3.2.2.i386-win32.exe -P "%tmp%" -c
wget --no-check-certificate https://downloads.sourceforge.net/project/freepascal/OS_2/3.2.2/os2322.zip -P "%tmp%" -c

rem install fpc
"%tmp%\fpc-3.2.2.i386-win32.exe" /silent
"%tmp%\unzip" -o "%tmp%\os2322.zip" -d "%tmp%"
"%tmp%\unzip" -o "%tmp%\baseos2.zip" units\* -dc:\fpc\3.2.2

rem dir "c:/Documents and settings/Downloads"
rem dir c:\fpc /s
rem build
rem call build-git.cmd
