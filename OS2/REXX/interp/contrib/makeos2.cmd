/**/
Parse arg ver .
If ver = '' Then
 Do
    Say 'Must supply version: eg 20'
    Exit
 End
SRC = Translate(Value('REGINA_SRCDIR',,'OS2ENVIRONMENT'),'\','/')
here = Directory()
If Translate( here ) \= Translate( src ) Then
   Do
      Say 'Must be run from' src 'directory!'
      Exit 1
   End
'cd gcc'
'mkdir dist'
'cd dist'
'del /Y *.*'
'copy ..\regina.exe'
'copy ..\rexx.exe'
'copy ..\rxstack.exe'
'copy ..\rxqueue.exe'
'copy ..\execiser.exe'
'copy ..\threader.exe'
'copy ..\regina.dll'
'copy ..\rexx.a'
'copy ..\rexx.lib'
'copy ..\regina.a'
'copy ..\regina.lib'
'copy ..\test1.dll'
'copy ..\test2.dll'
'copy ..\regutil.dll'
'copy' SRC || '\COPYING-LIB'
'copy' SRC || '\rexxsaa.h'
'copy' SRC || '\README.3?'
'copy' SRC || '\BUGS'
'copy' SRC || '\README.OS2 README'
'copy' SRC || '\README_SAFE'
'copy' SRC || '\file_id.diz.os2 file_id.diz'
'mkdir demo'
'copy' SRC || '\demo\*.rexx demo'
'copy' SRC || '\regutil\*.rexx demo'
'mkdir nls'
'copy ..\*.mtb nls'
'zip -r rex' || ver || 'os2 *'
'cd ..\..'
