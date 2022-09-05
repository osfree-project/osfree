@echo off

IF "%SOMBASE%"=="" GOTO sombase
SET %SC%=%SOMBASE%\bin\sc.exe

:body

IF NOT EXIST %SOMBASE%\include\somxh.gen goto normal
echo Generating C++ bindings in progress...
goto end

:normal
echo Generating the SOM C++ Bindings:
CD %SOMBASE%\include
SET SMNOADDSTAR=
echo somxh.gen > somxh.gen
%SC% -%% -sxh -mwinmult -mnochk *.idl
IF NOT ERRORLEVEL 0 GOTO end
IF EXIST somdtype.idl %SC% -mwinmult -mnochk -sxh -DEMIT_SOMDTYPES somdtype.idl
IF EXIST somxh.gen DEL somxh.gen
GOTO end

:sombase
    ECHO SOMBASE should be set before running somxh.
    GOTO end

:end
