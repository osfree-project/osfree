Parse Arg innsi nsifiles outnsi
trace o
If Stream( innsi, 'C', 'QUERY EXISTS' ) = '' Then Call Abort innsi 'does not exist. Cannot continue!'
If Stream( nsifiles, 'C', 'QUERY EXISTS' ) = '' Then Call Abort nsifiles 'does not exist. Cannot continue!'

Call Stream outnsi, 'C', 'OPEN WRITE REPLACE'

Do While Lines( innsi ) > 0
   line = Linein( innsi )
   If Countstr( '[', line ) = 1 & Countstr( ']', line ) = 1 Then
      Do
         Parse Var line '[' section ']' .
         If Lower( section ) = 'uninstall' Then Call UninstallFiles nsifiles, outnsi
         Else Call InstallFiles nsifiles, outnsi, section
      End
   Else
      Call Lineout outnsi, line
End
Call Stream outnsi, 'C', 'CLOSE'
Return 0

/*
 * Read the nsifiles file and write all lines for the supplied section to the outnsi file
 */
InstallFiles: Procedure
Parse Arg nsifiles, outnsi, section
Call Stream nsifiles, 'C','OPEN READ'
state = 'start'
Do While Lines( nsifiles ) > 0
   line = Linein( nsifiles )
   If Strip( line ) = '' | Left( line, 1 ) = '#' Then Iterate
   Select
      When state = 'start' Then
         Do
            Parse Var line '[' sec ']' .
            If sec = section Then state = 'in'
         End
      When state = 'in' Then
         Do
            If Countstr( '[', line ) = 1 & Countstr( ']', line ) = 1 Then Leave
            Call Lineout outnsi, line
         End
      Otherwise Nop
   End
End
Call Stream nsifiles, 'C', 'CLOSE'
Return

UninstallFiles: Procedure
Parse Arg nsifiles, outnsi, section
curdir = ''
Call Stream nsifiles, 'C','OPEN READ'
Do While Lines( nsifiles ) > 0
   line = Linein( nsifiles )
   If Strip( line ) = '' | Left( line, 1 ) = '#' | (Countstr( '[', line ) = 1 & Countstr( ']', line ) = 1) Then Iterate
   Select
      When Word( line, 1 ) = 'File' Then
         Do
            If Countstr( '/oname=', line ) = 1 Then
               Do
                  Parse Var line . '/oname=' fn .
               End
            Else Parse Var line . fn
            pos = Lastpos( '\', fn )
            If pos \= 0 Then fn = Substr( fn, pos+1 )
            Call Lineout outnsi, 'Delete' curdir'\'fn
         End
      When Word( line, 1 ) = 'SetOutPath' Then
         Do
            -- remove previous directory before setting a new one
            If curdir \= '' Then Call Lineout outnsi, 'RMDir' curdir
            Parse Var line . curdir
         End
      Otherwise Nop
   End
End
Call Lineout outnsi, 'Delete $INSTDIR\uninstall.exe'
If curdir \= '' Then Call Lineout outnsi, 'RMDir' curdir
Call Stream nsifiles, 'C', 'CLOSE'
Return

Abort: Procedure
Parse Arg msg
Say msg
Exit 1
