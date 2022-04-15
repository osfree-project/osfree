/*******************************************************************/
/*                                                                 */
/* REXXUDEM.CMD                                                    */
/*                                                                 */
/* This program calls the various REXX external functions          */
/* provided in the REXXUTIL.C sample function package.             */
/*                                                                 */
/* Each function is called once to illustrate how it is used.      */
/* Some of the functions have multiple options and multiple        */
/* types of output, but not all options are used here.             */
/*                                                                 */
/* Some of the functions in REXXUTIL only work in windowed         */
/* or fullscreen command prompt sessions, not in PMREXX.           */
/* This is because they are doing vio-type operations such as      */
/* placing the cursor at a particular place on the screen.         */
/* So if this program is run in PMREXX, it will start a new        */
/* windowed session and run itself there.                          */
/*                                                                 */
/*******************************************************************/
'@echo off'

/* Detect if we are running under PMREXX.                          */
/* Some of the REXXUTIL functions (such as SysCurPos) only work    */
/* in OS/2 command prompt sessions, not in PMREXX.                 */
If Address() = 'PMREXX' Then  Do
  Say 'This program will now run itself in a windowed '
  Say '  command prompt session'
  'START REXXUDEM.CMD /C /WIN'
  Exit
  End
  /* We now know we are running in a line-mode session.            */

/*
 * Now load all functions
 */
say "Load all functons..."
rc = rxfuncadd('SysLoadFuncs','REXXUTIL', 'SysLoadFuncs')
say rc
call SysLoadFuncs
say "Done."


/*** Demonstrate SysOS2Ver ***/
call SysCls
say; say 'Demonstrating SysOS2Ver/SysVersion'
Say 'You are running OS/2 version 'SysOS2Ver()
say 'This is library version 'SysUtilVersion( )

/*** Demonstrate SysBootDrive ***/
/* save result for other tests */
say; say 'Demonstrating SysBootDrive'
bootdrive =SysBootDrive( )
say 'Boot drive is:      'bootdrive
say 'Boot drive type is: 'SysFileSystemType(bootdrive)
say 'SysProcessType      'SysProcessType( )
say 'Process codepage:   'SysQueryProcessCodePage( )
call SysPause 'Press a key'

/*** Demonstrate SysCls ***/
say;
call SysPause 'Demonstrating SysCls, press Enter key...'
call SysCls
say; say 'Did you see that quick screen-clear via SysCls?'
call SysPause  'Press a key'

/*** Demonstrate SysGetKey ***/
say;
say 'Demonstrating SysGetKey...'
say 'SysGetKey with NOECHO option - Press any letter key...'
keypressed = SysGetKey(NOECHO)
say 'You pressed 'keypressed
say;
say 'SysGetKey with ECHO option - Press any letter key...'
keypressed = SysGetKey(ECHO)
say;
say 'You pressed 'keypressed
call SysPause 'Press a key'

/*** Demonstrate SysQueryExtLIBPATH & SysSetExtLIBPATH ***/
call SysCls
say 'Demonstrating SysQueryExtLIBPATH & SysSetExtLIBPATH:'
savedBeginPath =SysQueryExtLIBPATH("B")
savedEndPath =SysQueryExtLIBPATH("E")
say '  Begin:  'savedBeginPath
say '  End:    'savedEndPath
say 'Setting both to c:\os2'
call SysSetExtLIBPATH "c:\os2",'b'
call SysSetExtLIBPATH "c:\os2",'e'
say '  Begin:  'SysQueryExtLIBPATH("B")
say '  End:    'SysQueryExtLIBPATH("E")
say 'Appending using %BEGINLIBPATH%;c:\os2\dll'
say 'and %ENDLIBPATH%;c:\os2\dll'
call SysSetExtLIBPATH "%BEGINLIBPATH%;c:\os2\dll",'b'
call SysSetExtLIBPATH "%ENDLIBPATH%;c:\os2\dll",'e'
say '  Begin:  'SysQueryExtLIBPATH("B")
say '  End:    'SysQueryExtLIBPATH("E")
say 'Clear both:'
call SysSetExtLIBPATH ,'b'
call SysSetExtLIBPATH ,'e'
say '  Begin:  'SysQueryExtLIBPATH("B")
say '  End:    'SysQueryExtLIBPATH("E")
say 'Restore previous:'
call SysSetExtLIBPATH savedBeginPath,'b'
call SysSetExtLIBPATH savedEndPath,'e'
say '  Begin:  'SysQueryExtLIBPATH("B")
say '  End:    'SysQueryExtLIBPATH("E")
call SysPause 'Press a key'
call SysCls

/*** Demonstrate SysQueryClassList ***/

say;
call SysPause 'Demonstrating SysQueryClassList, press Enter key...'
call SysQueryClassList names
DO i = 1 TO names.0
   SAY i names.i
END
say 'Found 'names.0' classes'
call SysPause 'Press a key'
call SysCls


/*** Demonstrate SysAddFileHandle ***/
say;
call SysPause 'Demonstrating SysAddFileHandle, press Enter key...'
say 'Current number of file handles 'SysAddFileHandle('0')
call SysAddFileHandle(10)
say 'Add 10 handles, now number of file handles 'SysAddFileHandle(0)
call SysPause 'Press a key'

/*** Demonstrate SysCurPos ***/
call SysCls
say; say 'Demonstrating SysCurPos...'
call SysCurPos '3', '0'
call SysPause 'Press a key'
call SysCurPos '10', '50'
call SysPause 'Press a key'
call SysCurPos '20', '20'
call SysPause 'Press a key'
call SysCurPos '5', '65'
call SysPause 'Press a key'
call SysCurPos '23', '0'
call SysPause


/*** Demonstrate SysCurState ***/
call SysCls
call SysCurState 'OFF'
say; say 'Demonstrating SysCurState..'; say
call SysPause 'Note that the cursor is hidden.  Press Enter key...'
call SysCurState 'ON'


/*** Demonstrate SysDriveInfo ***/
call SysCls
say; say 'Demonstrating SysDriveInfo by displaying',
          bootdrive' drive info:'
say SysDriveInfo(bootdrive)
call SysPause


/*** Demonstrate SysDriveMap ***/
call SysCls
say; say 'Demonstrating SysDriveMap by displaying all available drives:'
say SysDriveMap( )
call SysPause


/*** Demonstrate SysGetMessage ***/
call SysCls
say; say 'Demonstrating SysGetMessage by reading message #34 from OSO001.MSG'; say
say SysGetMessage(34, , 'A:', 'the diskette labeled "Disk 2"', 'SER0002')
Say '  (that was just a demonstration.  You do not have to insert a diskette)'
call SysPause 'Press a key'

/*** Demonstrate SysDumpVariables ***/
call SysCls
say 'Demonstrating SysDumpVariables...'
say 'This is a dump of current variables:'
call SysDumpVariables
say
call SysPause 'Press a key'
say 'Performing SysDumpVariables rxtemp.fil for next test'
call SysDumpVariables 'rxtemp.fil'
call SysPause 'Press a key'

/*** Demonstrate SysSetIcon ***/
say
say 'Demonstrating SysSetIcon...'
say 'add rexxudem.ico to rxtemp.fil'
Ret = SysSetIcon('rxtemp.fil', 'rexxudem.ico')
if Ret then say "Returned OK"
else say "Returned BAD"
call SysPause 'You can check the file and then Press a key'

/*** Demonstrate SysPutEA ***/
say; say 'Demonstrating SysPutEA...'
/*
call SysSay 'Creating rxtemp.fil...'
call lineout 'rxtemp.fil', 'xyzpdq'
call stream 'rxtemp.fil', 'C', 'CLOSE'
say 'done.'*/
OS2_rc = SysPutEA('rxtemp.fil', 'REXXUDEM', 'Demonstration of SysPutEA')
If OS2_rc = 0 Then Say 'rxtemp.fil now has an Extended Attribute',
                       'named REXXUDEM.'
Else Say 'We have a problem, the OS/2 file system returned error' OS2_rc
call SysPause 'Press a key'


/*** Demonstrate SysGetEA ***/
call SysCls
say; say 'Demonstrating SysGetEA...'
OS2_rc = SysGetEA('rxtemp.fil', 'REXXUDEM', 'my_output_variable')
If OS2_rc = 0 Then Do
  Say 'the value of the REXXUDEM Exnded Attribute',
      'is "'my_output_variable'"'
  If my_output_variable == 'Demonstration of SysPutEA' then
    Say '  ...and that is the right value.'
  else Say '  ...but that is the wrong value!'
  End
Else Say 'We have a problem, the OS/2 file system returned error' OS2_rc
call SysPause 'Press a key'


/*** Demonstrate SysGetFileDateTime and SysSetFileDateTime ***/
call SysCls
say; say 'Demonstrating SysGetFileDateTime and SysSetFileDateTime:'
say; say "Current rxtemp.fil times and dates:"
say 'Create:  'SysGetFileDateTime("rxtemp.fil", C)
say 'Access:  'SysGetFileDateTime("rxtemp.fil", A)
say 'Write:   'SysGetFileDateTime("rxtemp.fil", W)
say; say "Changing file modified time and date to:"
say "2000-03-20 13:25:02"
call SysSetFileDateTime "rxtemp.fil","2000-03-20","13:25:02"
say 'Write:   'SysGetFileDateTime("rxtemp.fil", W)
say; say "restoring to current:"
call SysSetFileDateTime "rxtemp.fil"
say 'Write:   'SysGetFileDateTime("rxtemp.fil", W)
call SysPause 'Press a key'

/*** Demonstrate SysFileDelete ***/
call SysCls
say; say 'Demonstrating SysFileDelete by deleting the file rxtemp.fil:'
call SysPause
if SysFileDelete('rxtemp.fil')=0 then
  say 'The file rxtemp.fil has been deleted.'
else
  say 'Could not delete rxtemp.fil!'
call SysPause


/*** Demonstrate SysFileSearch ***/
call SysCls
say; say 'Demonstrating SysFileSearch:'
say 'About to search the 'bootdrive'\CONFIG.SYS files for'
say 'the word "device":'; say
call SysPause; say
call SysFileSearch 'device', bootdrive'\config.sys', 'file'
say 'Lines found:'; say
do num=1 to file.0
  say file.num
end
say
call SysPause


/*** Demonstrate SysFileTree ***/
call SysCls
say; say 'Demonstrating SysFileTree by displaying all entries in',
          bootdrive' root subdir:'
call SysPause
call SysFileTree bootdrive'\*', 'files', 'B'
say;
do num=1 to files.0
  say files.num
end
call SysPause


/*** Demonstrate SysIni ***/
call SysCls
say; say 'Demonstrating SysIni by placing time information in OS2.INI file...'
time=Time()
call SysIni , 'REXXUDEM', 'TIME', time
call SysPause
say; say 'Demonstrating SysIni by retrieving time information from OS2.INI file...'
val = SysIni( , 'REXXUDEM', 'TIME')
say 'The retrieved information = 'val; say
call SysPause
say; say 'Deleting the time information from the OS2.INI file...'
call SysINI , 'REXXUDEM', 'TIME', '$RXDEL'
call SysPause

/*** Demonstrate SysMkDir ***/
call SysCls
say; say 'Demonstrating SysMkDir:'
say 'RC when trying to create directory',
    bootdrive'\RXTMPDIR = 'SysMkDir(bootdrive'\RXTMPDIR')
call SysPause


/*** Demonstrate SysRmDir ***/
call SysCls
say; say 'Demonstrating SysRmDir:'
say 'RC when trying to remove directory',
    bootdrive'\RXTMPDIR = 'SysRmDir(bootdrive'\RXTMPDIR')
call SysPause


/*** Demonstrate SysSearchPath ***/
call SysCls
say; say 'Demonstrating SysSearchPath by searching for CMD.EXE in PATH:'
call SysPause
say 'Directory containing CMD.EXE is:  'SysSearchPath('PATH', 'cmd.exe')
call SysPause


/*** Demonstrate SysSleep ***/
call SysCls
say; say 'Demonstrating SysSleep:  Sleeping for 2 seconds...'
call SysSleep '2'
call SysPause


/*** Demonstrate SysTempFileName ***/
call SysCls
say; say 'Demonstrating SysTempFileName:'; say
say 'Unique file in' bootdrive'\OS2 =',
    SysTempFileName(bootdrive'\OS2\OS2.???'); say
call SysPause


/*** Demonstrate SysTextScreenRead ***/
call SysCls
say; say 'Demonstrating SysTextScreenRead by reading the entire screen.'; say
do i=0 to 10
 say copies(' ', i*5)'Hello'
end
say
call SysPause 'Press Enter key to read the screen...'
screen = SysTextScreenRead(0, 0)
say 'Screen read.'
call SysPause 'Press Enter key to clear the screen then restore it...'
call SysCls
call SysSay screen
call SysPause


/*** Demonstrate SysTextScreenSize ***/
call SysCls
say; say 'Demonstrating SysTextScreenSize...'
parse value SysTextScreenSize() with row col
say 'Rows='row', Columns='col
call SysPause

/*** Demonstrate SysDropFuncs ***/
call SysCls
say; say 'Demonstrating SysDropFuncs...'
Call SysDropFuncs
Say 'The functions have now been dropped.'

say; say 'REXXUDEM demonstration program is complete.'
exit

SysPause:
parse arg prompt
  if prompt='' then
    prompt='Press Enter key when ready . . .'
  call SysSay prompt
  Pull .
  say
return

SysSay:
parse arg string
  call charout 'STDOUT', string
return

