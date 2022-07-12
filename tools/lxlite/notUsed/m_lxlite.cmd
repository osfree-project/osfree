@echo off
set version=139

rem ***************************************************************************
rem
rem          please run this file to make lxLite distribution archive
rem
rem ***************************************************************************

mkdir ..\lxLite >nul
mkdir ..\lxLite\API >nul
copy ..\src\lxLite.rc ..\lxLite\API >nul
copy ..\src\os2API.rc ..\lxLite\API >nul
copy ..\contrib\lxrc.cmd ..\lxLite\API >nul
copy ..\contrib\lxrc.readme ..\lxLite\API >nul
copy ..\src\out\lxLite.exe ..\lxLite >nul
copy ..\src\out\chCase.exe ..\lxLite >nul
copy ..\src\out\noEA.exe ..\lxLite >nul
copy ..\src\out\sysIcons.exe ..\lxLite >nul
copy ..\src\out\unLock.exe ..\lxLite >nul
copy ..\contrib\lxLite.cfg ..\lxLite >nul
copy ..\contrib\install.cmd ..\lxLite >nul
copy ..\contrib\lxLite_f.ico ..\lxLite >nul
mkdir ..\lxLite\doc >nul
xcopy ..\doc ..\lxLite\doc /s
cd ..\lxLite
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
rm -rf lxLite
del lxlt%version%s.zip
zip -9 -r lxlt%version%s * -x src/*.res -x src/out/ -x src/out/* -x .gitignore -x *.exe -x *.zip
