@echo off
IF "%SOMBASE%"=="" GOTO sombase
set SC=%SOMBASE%\bin\sc.exe

IF EXIST somstars.gen GOTO end

:body
echo Generating the SOM C Bindings:
echo (Warning, these bindings are not CORBA-compliant)
CD %SOMBASE%\include
ATTRIB -R som.h
ATTRIB -R gen_c.efw
ATTRIB -R gen_emit.efw
ATTRIB -R gen_make.efw
COPY som.hs       som.h > NUL
COPY gen_c.efs    gen_c.efw  > NUL
COPY gen_emit_c.efs gen_emit_c.efw  > NUL
COPY gen_make_c.efs gen_make_c.efw > NUL
echo somstars.gen > somstars.gen
SET SMNOADDSTAR=
%SC% -%% -maddstar -mwinmult -mnochk -sh *.idl
IF NOT ERRORLEVEL 0 GOTO end
IF EXIST somdtype.idl %SC% -maddstar -mwinmult -mnochk -sh -DEMIT_SOMDTYPES somdtype.idl
GOTO end

:sombase
    ECHO SOMBASE should be set before running somstars.
    GOTO end

:end
    rem ENDLOCAL
