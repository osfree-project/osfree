rem 
rem Build with MSC 6.0/VAC/MASM
rem
setlocal
SET DEBUG=0
SET COMSPEC=D:\OS2\CMD.EXE
SET OS2_SHELL=D:\OS2\CMD.EXE
SET LANG=en_US
SET CPP_DBG_LANG=CPP
rem SET DDK=f:\ddk
rem SET DDKTOOLS=f:\ddktools
SET TOOLKIT=f:\os2tk45
rem SET MSC=%ddktools%\toolkits\msc60
rem SET MASM=%ddktools%\toolkits\masm60
rem SET IBMC=%ddk%\base\tools\OS2.386\LX.386\BIN\vacpp
SET IBMC=f:\dev\vac365
SET INCLUDE=..\include;%ibmc%\include;%ibmc%\INCLUDE\OS2;%include%
SET LIB=..\lib;%ibmc%\lib;%LIB%
SET BEGINLIBPATH=%ibmc%\RUNTIME;%ibmc%\DLL
SET PATH=%ibmc%\BIN;%MASM%\BINP;%PATH%
SET DPATH=%ibmc%\msg;%ibmc%\HELP;%ibmc%\runtime;%ibmc%\LOCALE;%ibmc%;%DPATH%
SET NLSPATHTEMP=%ibmc%\locale\%%N
SET NLSPATH=%NLSPATHTEMP%;%NLSPATH%
SET NLSPATHTEMP=
rem call %IBMC%\bin\setenv.cmd
rem call %MSC%\binp\setenv.cmd
nmake -f makefile.icc
endlocal
