/* */
'wmake.exe clean'
/*'set include=C:\dev\OS2TK45\som\include;C:\dev\OS2TK45\inc;%include%'
'set lib=C:\dev\OS2TK45\som\lib;%lib%'*/
'wmake.exe'
if rc=0 then
do
  '@copy emitlnk.dll c:\os2tk45\som\lib >nul'
  '@copy lnk.efw c:\os2tk45\som\include >nul'
exit
  'sc.exe -s"lnk" test.idl'
end
pause