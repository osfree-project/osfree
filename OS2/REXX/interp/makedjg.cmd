/**/
Parse arg ver .
If ver = '' Then
 Do
    Say 'Must supply version: eg 20'
    Exit
 End
SRC = Translate(Value('REGINA_SRCDIR',,'OS2ENVIRONMENT'),'\','/')
'cd djg'
'mkdir dist'
'cd dist'
'copy ..\rexx.exe'
'copy ..\rxstack.exe'
'copy ..\rxqueue.exe'
'copy ..\libregin.a'
'copy' SRC || '\COPYING-LIB'
'copy' SRC || '\rexxsaa.h'
'copy' SRC || '\README.3?'
'copy' SRC || '\BUGS'
'copy' SRC || '\README_SAFE'
'copy' SRC || '\README.DJG README'
'copy' SRC || '\file_id.diz.djg file_id.diz'
'mkdir demo'
'copy' SRC || '\demo\*.rexx demo\*.rex'
'mkdir nls'
'copy ..\*.mtb nls'
'del rex??djg.zip'
'zip -r rex' || ver || 'djg *'
'cd ..\..'
