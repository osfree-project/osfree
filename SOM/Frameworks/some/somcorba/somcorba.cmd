@echo off

IF "%SOMBASE%"=="" GOTO sombase
SET SC=%SOMBASE%\bin\sc.exe

:body
echo Generating the CORBA C Bindings:
CD %SOMBASE%\include
ATTRIB -R som.h
ATTRIB -R gen_c.efw
ATTRIB -R gen_emit.efw
ATTRIB -R gen_make.efw
ATTRIB -R gen_mk32.efw
COPY som.hc       som.h > NUL
COPY gen_c.efc    gen_c.efw  > NUL
COPY gen_emit.efc gen_emit.efw  > NUL
COPY gen_make.efc gen_make.efw  > NUL
COPY gen_mk32.efc gen_mk32.efw > NUL
SET SMADDSTAR=
echo somcorba.gen  > somcorba.gen
%SC% -%% -S150000 -mnoaddstar -mnochk -sh *.idl
IF EXIST somdtype.idl %SC% -mnoaddstar -mnochk -sh -DEMIT_SOMDTYPES somdtype.idl
IF NOT ERRORLEVEL 0 GOTO end
GOTO end

:sombase
    ECHO SOMBASE should be set before running somcorba.
    GOTO end

:end
