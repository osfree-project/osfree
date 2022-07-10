/* */
'wmake.exe clean'
/*'set include=C:\dev\OS2TK45\som\include;C:\dev\OS2TK45\inc;%include%'
'set lib=C:\OS2TK45\som\lib;%lib%'*/
'wmake.exe'
if rc=0 then
do
  '@copy emitpasi.dll c:\os2tk45\som\lib >nul'
  '@copy pasi.efw c:\os2tk45\som\include >nul'
exit
  'sc.exe -s"pas" test.idl'
end
pause