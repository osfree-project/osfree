@echo off
set version=139

rem ***************************************************************************
rem
rem          please run this file to make lxLite distribution archive
rem
rem ***************************************************************************

copy lxLite_src\lxLite.rc lxLite\API >nul
copy lxLite_src\os2API.rc lxLite\API >nul
cd lxLite
rem noEA /y lxLite.exe chCase.exe noEA.exe unLock.exe install.cmd
rem eaUtil lxLite.exe	..\lxLite.EA	/j /o /p
rem eaUtil chCase.exe	..\chCase.EA	/j /o /p
rem eaUtil noEA.exe		..\noEA.EA	/j /o /p
rem eaUtil unLock.exe	..\unLock.EA	/j /o /p
rem eautil install.cmd	..\install.EA	/j /o /p
chcase * /cfal /y
ren lxlite.* lxLite.* >nul
ren chcase.* chCase.* >nul
ren unlock.* unLock.* >nul
ren noea.* noEA.* >nul
ren sysicons.* sysIcons.* >nul
lxLite /f chCase.exe unLock.exe noEA.exe sysIcons.exe
lxLite /f /x /yur lxlite.exe
cd ..

rem *** Archiving ***
del lxlt%version%.zip
zip -9 -r lxlt%version% lxLite/*
del lxlt%version%s.zip
zip -9 -r lxlt%version%s * -x lxLite_src/*.res -x lxLite_src/out/ -x lxLite_src/out/* -x .gitignore -x *.exe -x *.zip
