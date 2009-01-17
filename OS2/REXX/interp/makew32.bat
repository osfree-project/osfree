@echo off
if "x%1" = "x" goto noparm
if "x%2" = "x" goto noparm
mkdir dist
cd dist
copy ..\%1\regina.exe
copy ..\%1\regina.dll
copy ..\%1\rexx.lib
copy ..\%1\rexx.exe
copy ..\%1\rxstack.exe
copy ..\%1\rxqueue.exe
copy ..\%1\regina.lib
copy ..\%1\test1.dll
copy ..\%1\test2.dll
copy ..\%1\libregina_cygwin.a
copy ..\%1\libregina_ming.a
copy ..\%1\regina_borland.lib
copy %REGINA_SRCDIR%\COPYING-LIB
copy %REGINA_SRCDIR%\rexxsaa.h
copy %REGINA_SRCDIR%\README.2?
copy %REGINA_SRCDIR%\BUGS
copy %REGINA_SRCDIR%\TODO
copy %REGINA_SRCDIR%\README.W32 README
copy %REGINA_SRCDIR%\file_id.diz.w32 file_id.diz
mkdir demo
copy %REGINA_SRCDIR%\demo\*.* demo
del rex??w32.zip
zip -r rex%2w32 *
cd ..
goto exit
:noparm
echo Must supply directory of built files and version number
echo eg. makew32 vc-st 20
:exit
