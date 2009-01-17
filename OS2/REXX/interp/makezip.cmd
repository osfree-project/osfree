/**/
Parse Upper Arg target version .
If Wordpos(target,'DJG VCP EMX W32') = 0 Then Call Usage
If Stream('d:\djgpp\bin\go32.exe','C','QUERY EXISTS') = '' Then
  extra.djg = 'c:\djgpp\bin\go32.exe'
Else
  extra.djg = 'd:\djgpp\bin\go32.exe'
If Stream('f:\emx\bin\emx.exe','C','QUERY EXISTS') = '' Then
  extra.emx = 'c:\emx\bin\emx.exe'
Else
  extra.emx = 'f:\emx\bin\emx.exe'
Parse Value 'dos\go32 dos\emx os2\emx vc' With src.djg src.vcp src.emx src.w32
Parse Value 'libregna.a regina.a regina.a regina.lib' With lib.djg lib.vcp lib.emx lib.w32
Parse Value 'rexx.exe rexx.exe rexx.exe regina.exe' With exe.djg exe.vcp exe.emx exe.w32
Parse Value 'rex rex rexx rexx' With ext.djg ext.vcp ext.emx ext.w32
If Stream('tmp','C','QUERY EXISTS') = '' Then  'mkdir tmp'
'cd tmp'
If rc \= 0 Then Abort( 'Cannot change dir to "tmp"' )
'del *.*'
'copy ..\COPYING-LIB'
'copy ..\rexxsaa.h'
'copy ..\README.08?'
'copy ..\README.2*'
'copy ..\BUGS'
'copy ..\README.' || target 'README'
'copy ..\file_id.diz.' || target 'file_id.diz'
'copy ..\' || src.target || '\' || exe.target
'copy ..\' || src.target || '\' || lib.target
If extra.target \= '' Then 'copy' extra.target
If target = 'W32' Then
   Do
      'copy ..\' || src.target || 'regina.dll'
      'copy ..\' || src.target || 'test1.dll'
      'copy ..\' || src.target || 'test2.dll'
      'copy ..\' || src.target || 'rexx.lib'
   End
If Stream('demo','C','QUERY EXISTS') = '' Then  'mkdir demo'
'copy ..\demo\*.rexx demo\*.' || ext.target
'zip -r rx' || version || target '*'
'del ..\rx' || version || target ||'.zip'
'move *.zip ..'
'cd ..'
Return 0

usage: Procedure
Say 'Usage: makezip target version'
Say 'where: target is one of emx, djg, vcp, w32'
Say '       version is 20 etc'
Exit 0

abort: Procedure
Parse Arg msg
Say msg
Exit 1
