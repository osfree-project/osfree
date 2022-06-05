/*
 * Example of how to use sysgetline() and sysgetlinehistory()
 */
call RxFuncAdd 'sysloadfuncs', 'regutil', 'sysloadfuncs'
call sysloadfuncs
/* specify our history file */
histfile = syshomedirectory()||.dirsep'.history-regutil'
/* initialise and read the history file so we have access to previous lines */
rc = sysgetlinehistory( histfile, 'init')
rc = sysgetlinehistory( histfile, 'read')
Say 'This is an example of using the sysgetline() and sysgetlinehistory() functions.'
Say 'Type in an operating system command, and when you press ENTER it will be executed.'
Say '        *** BE CAREFUL ***'
Say 'You can scroll through the history by pressing Up Arrow and Down Arrow.'
Say 'You can use Left Arrow and Right Arrow to move the cursor within the line displayed.'
Say 'To exit the program; enter "quit"'
Say
Say 'History will be saved in:' histfile
Say
Do Forever
   cmd = sysgetline( '> ' )
   If cmd = 'not implemented' Then
      Do
         Say 'This program relies on a functioning Rexx function; sysgetline(). This function is not available. Aborting.'
         Exit 1
      End
   cmd = Strip( cmd )
   If Upper( cmd ) = 'QUIT' Then Leave
   /* execute cmd as shell command */
   Address System cmd
End
/* save our history for next time */
rc = sysgetlinehistory( histfile, 'write' )
