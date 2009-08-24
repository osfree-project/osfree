/**/
Parse arg ver .
If ver = '' Then
 Do
    Say 'Must supply version: eg 20'
    Exit
 End
SRC = Translate(Value('REGINA_SRCDIR',,'OS2ENVIRONMENT'),'\','/')
'cd vcp'
'mkdir dist'
'cd dist'
'copy ..\rexx.exe'
'copy ..\regina*.a'
'copy' SRC || '\COPYING-LIB'
'copy' SRC || '\rexxsaa.h'
'copy' SRC || '\README.3?'
'copy' SRC || '\BUGS'
'copy' SRC || '\README.VCP README'
'copy' SRC || '\README_SAFE'
'copy' SRC || '\file_id.diz.vcp file_id.diz'
'mkdir demo'
'copy' SRC || '\demo\*.rexx demo\*.rex'
'mkdir nls'
'copy ..\*.mtb nls'
'del rex??vcp.zip'
'zip -r rex'ver'vcp *'
'cd ..\..'
