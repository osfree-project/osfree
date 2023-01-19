/* MAKEWPI.CMD
   Expects to run from root of 4OS2 development workspace
   Expects files to found in distrib subdirectory because
   that is where mkzip puts them
 */

ARG param

pkgname  = '4os2'
verstr   = '3.11'

CALL RxFuncAdd 'SysIni', 'REXXUTIL', 'SysIni'

IF LEFT( param, 3 ) == 'MIN' THEN DO
    /* MIN builds at .wpi */
    makefull = 0
    fullpkg  = pkgname'_'verstr'.wpi'
    exeflag  = ''
END
ELSE DO
    /* Anything else builds at self extracting .exe */
    makefull = 1
    fullpkg  = pkgname'_'verstr'.exe'
    exeflag  = '-u'
END

IF STREAM( fullpkg, 'C', 'QUERY EXISTS') \= '' THEN
    '@del' fullpkg

PARSE VALUE SysIni('USER', 'WarpIN', 'Path') WITH warpin '00'x .
IF LEFT( warpin, 6 ) == 'ERROR:' THEN DO
    SAY 'Unable to locate WarpIN.'
    EXIT 1
END

distdir = 'distrib'
basedir = '.'

file = distdir || '\4os2.exe'
s = STREAM(file, 'C', 'QUERY EXISTS')
IF s == '' THEN DO
    SAY 'Cannot access' file '- check' distdir 'subdirectory content.'
    EXIT 1
END

CALL SETLOCAL
'@SET PATH='warpin';%PATH%'
'@SET BEGINLIBPATH='warpin

retcode = 0
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2.exe'          exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2.ico'          exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2.inf'          exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2.ini.tpl'      exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2.txt'          exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2alias.sample'  exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4os2h.msg'         exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4start.cmd.tpl'    exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir '4start.cmd.tpl2'   exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'changelog'         exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'examples.btm'      exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'jpos2dll.dll'      exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'keystack.exe'      exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'license.txt'       exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'option2.exe'       exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'readme.1st'        exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-a 1 -c' || distdir 'shralias.exe'      exeflag
IF RC > retcode THEN retcode = RC
'@wic.exe' fullpkg '-s' pkgname'.wis'
IF RC > retcode THEN retcode = RC

CALL ENDLOCAL

RETURN retcode

/* eof */
