@echo off

setlocal

if !%SOMBASE%!==!! goto error
if "%2"=="" goto usage

set targetlang=c
if  "%1"=="-C"  goto setc
if  "%1"=="-c"  goto setc
if  "%1"=="/C"  goto setc
if  "%1"=="/c"  goto setc
goto skip

:setc
set targetlang=c
shift

:skip

if "%1"=="-C++" goto setCC
if "%1"=="-c++" goto setCC
if "%1"=="/C++" goto setCC
if "%1"=="/c++" goto setCC
if "%1"=="-CPP" goto setCC
if "%1"=="-CPp" goto setCC
if "%1"=="-Cpp" goto setCC
if "%1"=="-cpp" goto setCC
if "%1"=="-cpP" goto setCC
if "%1"=="-cPP" goto setCC
if "%1"=="-cPp" goto setCC
if "%1"=="/CPP" goto setCC
if "%1"=="/CPp" goto setCC
if "%1"=="/Cpp" goto setCC
if "%1"=="/cpp" goto setCC
if "%1"=="/cpP" goto setCC
if "%1"=="/cPP" goto setCC
if "%1"=="/cPp" goto setCC

goto skip3

:setCC
set targetlang=cpp
shift

:skip3

if "%2"=="" goto usage
if "%3"=="" goto skip4

:usage
echo "usage: newemit [-C | -C++] <className> <stem>"
goto exit

:error
echo SOMBASE environment variable not set
goto exit

:skip4

set class=%1
set stem=%2

echo interface %class% {}; > %stem%.idl

echo %stem%.idl:
%SOMBASE%\bin\sc.exe -aemitfile=_%stem%.idl -adeffile=gen_idl.efw -sgen -mnochk %stem%.idl
del %stem%.idl
rename _%stem%.idl %stem%.idl

echo %stem%.%targetlang%:
%SOMBASE%\bin\sc.exe -aemitfile=%stem%.%targetlang% -adeffile=gen_%targetlang%.efw -sgen -mnochk %stem%.idl

echo emit%stem%.%targetlang%:
%SOMBASE%\bin\sc.exe -aemitfile=emit%stem%.%targetlang% -adeffile=gen_emit_%targetlang%.efw -sgen -mnochk %stem%.idl

echo Makefile:
%SOMBASE%\bin\sc.exe -aemitfile=Makefile -adeffile=gen_make_%targetlang%.efw -sgen -mnochk %stem%.idl

echo %stem%.def
%SOMBASE%\bin\sc.exe -aemitfile=emit%stem%.def -adeffile=gen_def.efw -sgen -mnochk %stem%.idl

if exist makefile.nt rename makefile.nt makefile.bak

echo makefile.nt:
%SOMBASE%\bin\sc.exe -aemitfile=makefile.nt -adeffile=gen_mknt_%targetlang%.efw -sgen -mnochk %stem%.idl

echo %stem%.nid:
%SOMBASE%\bin\sc.exe -aemitfile=emit%stem%.nid -adeffile=gen_nid.efw -sgen -mnochk %stem%.idl

echo Makefile.32:
%SOMBASE%\bin\sc.exe -aemitfile=Makefile.32 -adeffile=gen_mk32_%targetlang%.efw -sgen -mnochk %stem%.idl

echo %stem%.efw:
if not exist %SOMBASE%\include\gen_temp.efw copy %SOMBASE%\include\gen_temp.efw %stem%.efw

:exit
endlocal
