/* 
 * Make a distribution of THE
 * Required for those platforms that use GNU make:
 *   DOSDJP
 *   OS2EMX - Regina
 *   OS2OS2 - OS/2 Rexx
 *   OS2    - OS/2 Rexx/Trans
 *   DOSEMX
 */
Parse Arg ver verdot platform src reg
If ver = '' Then Call usage
'mkdir tmp'
'cd tmp'
'del /Y *.*'
'copy ..\the.exe .'
'copy' src || '\THE_Help.txt .'
'copy' src || '\demo.txt .'
'copy' src || '\*.the .'
'copy' src || '\*.tld .'
'copy' src || '\COPYING .'
'copy' src || '\HISTORY .'
'copy' src || '\TODO .'
'copy' src || '\README .'
select
  when platform = 'DOSDJG' then do
/*       if reg = '' then call usage  */
       'copy' src || '\dosgo32.diz file_id.diz'
/*       'copy' reg || '\go32.exe .' */
       int = ''
       suffix = 'djg'
       interpreter = 'Regina'
       end
  when platform = 'DOSEMX' then do
       if src = '' then call usage
       'copy' src || '\dosvcpi.diz file_id.diz'
       int = ''
       suffix = 'vcp'
       interpreter = 'Regina'
       end
  when platform = 'OS2EMX' then do
       if reg = '' then call usage
       'copy' src || '\os2.diz file_id.diz'
       'copy' reg || '\regina.dll .'
       int = 'R'
       suffix = '_os2'
       interpreter = 'Regina'
       end
  when platform = 'OS2OS2' then do
       'copy' src || '\os2.diz file_id.diz'
       'copy c:\dll\curses.dll .'
       int = ''
       suffix = 'os2'
       interpreter = 'OS/2 Rexx'
       end
  when platform = 'OS2' then do
       'copy' src || '\os2.diz file_id.diz'
       'copy c:\dll\curses.dll .'
       'copy c:\dll\rexxtran.dll .'
       int = ''
       suffix = 'os2'
       interpreter = 'any Rexx'
       end
   otherwise call usage
end
'c:\bin\the -b -p' src || '\fix.diz -a "' || ver verdot int interpreter '" file_id.diz'
'zip c:\dist\the' || ver || int || suffix '*'
/*
 * We are in tmp directory at the moment
 */
select
  when platform = 'OS2' then do
       'copy' src || '\os2.diz file_id.diz'
       'copy' src || '\theos2.ico the.ico'
       'copy' src || '\*.the .'
       'copy c:\dll\curses.dll .'
       'copy c:\dll\rexxtran.dll .'
       int = ''
       suffix = 'os2'
       interpreter = 'any Rexx'
       'mkdir doc'
       'cd doc'
       'mkdir images'
       'copy' src || '\images\*.png images'
       'make -f' src || '\emxos2.mak INT=REXXTRANS helpviewer'
       'cd ..'
       wpifile = 'c:\dist\the' || ver || int || suffix'.wpi'
       'del' wpifile
       '\warpin\wic' wpifile '-a 1 the.exe *.dll THE_Help.txt HISTORY TODO README *.the the.ico 2' 'doc\*.html doc\images\*.png doc\comm\*.html doc\commset\*.html doc\commsos\*.html doc\misc\*.html -s' src'\the.wis'
       end
   otherwise nop
end
'cd ..'
Return

usage:
Say 'makedist version versiondot platform src reg'
Say 'where:'
Say '      version    - 25'
Say '      versiondot - 2.5'
Say '      platform   - one of DOSDJG DOSEMX OS2 OS2EMX OS2OS2'
Say '      src        - source of THE'
Say '      reg        - directory where regina.dll or go32.exe is'
Exit 1
