/* createdoc.cmd:
    this calls xdoc\xdoc.exe on all the *.c files in \main and \helpers
    to have HTML reference documentation created.
    After this has been executed, open HTML\index.html to view the
    documentation. */

call RxFuncAdd 'SysLoadFuncs','RexxUtil','SysLoadFuncs'
call SysLoadFuncs

'@echo off'
'call xdoc "-iinclude" src\helpers\*.c'

rc = SysCreateObject("WPShadow", "index.html", directory(), "SHADOWID="directory()"\HTML\index.html", "R");

