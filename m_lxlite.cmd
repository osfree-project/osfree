@echo off
set version=133

rem ***************************************************************************
rem
rem          please run this file to make lxLite distribution archive
rem
rem ***************************************************************************

copy lxLite_src\lxLite.rc lxLite\API >nul
copy lxLite_src\os2API.rc lxLite\API >nul
cd lxLite
noEA /y lxLite.exe chCase.exe noEA.exe unLock.exe install.cmd
eaUtil lxLite.exe	..\lxLite.EA	/j /o /p
eaUtil chCase.exe	..\chCase.EA	/j /o /p
eaUtil noEA.exe		..\noEA.EA	/j /o /p
eaUtil unLock.exe	..\unLock.EA	/j /o /p
eautil install.cmd	..\install.EA	/j /o /p
chcase * /cfal /y
ren lxlite.* lxLite.* >nul
ren chcase.* chCase.* >nul
ren unlock.* unLock.* >nul
ren noea.* noEA.* >nul
ren sysicons.* sysIcons.* >nul
lxLite /f chCase.exe unLock.exe noEA.exe sysIcons.exe
lxLite /f /x /yur lxlite.exe
cd ..

rem *** Prepare sources ***
del lxLite_src\*.res
del /n lxLite_src\out\*
rmdir lxLite_src\out

rem *** Archiving ***
zip -z -9 -r lxlt%version% lxLite/*
del lxLite\*.exe
zip -z -9 -r lxlt%version%s * -x *.zip
