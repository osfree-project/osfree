// OS2CMDS.C - OS/2-specified commands for 4OS2 & TCMD/PM
//   Copyright (c) 1993 - 1997  Rex C. Conn   All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <share.h>
#include <string.h>

#include "4all.h"


// switch to specified window
int activate_cmd( int argc, char **argv )
{
        char szTitle[128];
        HWND hWnd;
        UINT i, uSize;
        ULONG ulList;
        PSWBLOCK pswblk;

        if ( argc < 2 ) {
activate_usage:
                return ( usage( ACTIVATE_USAGE ));
        }

        // check for existence of specified window (by title)
        sscanf( argv[1], "%*[\"]%127[^\"]", szTitle );

        if ( pfnWQSL == 0L )
                return ERROR_EXIT;

        ulList = (*pfnWQSL)( ghHAB, 0L, 0 );

        uSize = ( ulList * sizeof(SWENTRY)) + sizeof(HSWITCH);
        pswblk = (PSWBLOCK)AllocMem( &uSize );
        (*pfnWQSL)( ghHAB, pswblk, uSize );

        for ( i = 0; ( i < ulList ); i++ ) {
                if ( wild_cmp( szTitle, pswblk->aswentry[i].swctl.szSwtitle, 0, TRUE ) == 0 )
                        break;
        }

        if ( i == ulList )
                return ( error( ERROR_4DOS_INVALID_WINDOW_TITLE, argv[1] ));

        hWnd = pswblk->aswentry[i].swctl.hwnd;
        if ( argv[2] != NULL ) {

                if ( _strnicmp( argv[2], "MAX", 3 ) == 0 ) {
                        if ( pfnWPM != NULL )
                                (void)(*pfnWPM)( hWnd, WM_SYSCOMMAND, MPFROMSHORT(SC_MAXIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                } else if ( _strnicmp( argv[2], "MIN", 3 ) == 0 ) {
                        if ( pfnWPM != NULL )
                                (void)(*pfnWPM)( hWnd, WM_SYSCOMMAND, MPFROMSHORT(SC_MINIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                } else if ( _strnicmp( argv[2], "RES", 3 ) == 0 ) {
                        if ( pfnWPM != NULL )
                                (void)(*pfnWPM)( hWnd, WM_SYSCOMMAND, MPFROMSHORT(SC_RESTORE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                } else if ( _stricmp( argv[2], "CLOSE" ) == 0 ) {
                        if ( pfnWPM != NULL )
                                (void)(*pfnWPM)( hWnd, WM_SYSCOMMAND, MPFROMSHORT(SC_CLOSE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                        return 0;
                } else
                        goto activate_usage;
        }

        FreeMem( (char *)pswblk );

        // kludge - WinSetActiveWindow alone doesn't work if run from a VIO app

        // inform 4OS2 window it's losing the focus
        if ( pfnWPM != NULL )
                (void)(*pfnWPM)( ghwndWindowHandle, WM_ACTIVATE, MPFROMSHORT(FALSE), MPFROMHWND(ghwndWindowHandle) );

        // inform target window it's gaining the focus
        if ( pfnWPM != NULL )
                (void)(*pfnWPM)( hWnd, WM_ACTIVATE, MPFROMSHORT(TRUE), MPFROMHWND(hWnd) );

        if ( pfnWSAW != NULL )
                return (( (*pfnWSAW)( HWND_DESKTOP, hWnd ) == TRUE) ? 0 : error( (*pfnWGLE)( ghHAB ), szTitle ));
        return ERROR_EXIT;
}

// return or set the data path in the environment
int dpath_cmd( int argc, char **argv )
{
        PCH feptr;

        // if no args, display the current DPATH
        if ( argc == 1 ) {
                printf( FMT_FAR_STR_CRLF, (( feptr = get_variable( *argv )) == NULL ) ? NO_DPATH : feptr - 6);
                return 0;
        }

        // remove whitespace between args & null path spec (";" )
        sprintf( *argv+5, FMT_EQUAL_STR, (( argv[1] != NULL ) ? argv[1] + strspn( argv[1], " \t;=" ) : NULLSTR ));

        // add argument to environment
        return ( add_variable( *argv ));
}


// spawn an asynchronous detached process
int detach_cmd( int argc, char **argv )
{
        char *arg;
        char *pDetach, *pExt, achFailName[128], line[MAXLINESIZ+8];
        char szBuffer[MAXLINESIZ+8];
        char *lpCmd;
        RESULTCODES rescResults;

        if ( argc == 1 )
                return ( usage( DETACH_USAGE ));

        // copy the INI data for secondary copies of 4OS2
        SaveIniData();

        line[0] = '\0';

        arg = first_arg( argv[1] );
        pDetach = _alloca( strlen( arg ) + 1 );
        strcpy( pDetach, arg );

        // if not internal, alias, or redirection, try executing w/o 4OS2
        if (( findcmd( pDetach, 0 ) < 0 ) && ( get_alias( pDetach ) == 0L ) && ( strpbrk( argv[1], "<>|&" ) == NULL )) {

            // check for an external command
            if ((( arg = searchpaths( pDetach, NULL, TRUE )) != NULL ) && (( pExt = ext_part( arg )) != NULL )) {

                if (( stricmp( pExt, EXE ) == 0 ) || ( stricmp( pExt, COM ) == 0 )) {

                    lpCmd = (char *)arg;
                    if ( argv[2] != NULL ) {
                        // expand variables
                        if ( var_expand( strcpy( szBuffer, argv[2] ), 1 ) != 0 )
                            return ERROR_EXIT;
                    } else
                        szBuffer[0] = '\0';

                    sprintf( line, "%s%c%.*s%c", pDetach, 0, ( MAXLINESIZ - strlen( pDetach )), szBuffer, 0 );
                }
            }
        }

        // We may have to execute a copy of the command processor, in case
        //   the user is detaching an internal command (i.e., "DETACH dir" ).
        if ( line[0] == '\0' ) {
                sprintf(line, "%Fs%c/c %.*s%c", _pgmptr, 0, (MAXLINESIZ - strlen( _pgmptr )), argv[1], 0 );
                lpCmd = _pgmptr;
        }

        if (( argc = DosExecPgm( achFailName, sizeof( achFailName), EXEC_BACKGROUND, line, glpEnvironment, &rescResults, lpCmd )) != 0 )
                return (error( argc, argv[1] ));

        printf( PROCESS_ID_MSG, rescResults.codeTerminate );

        return 0;
}


// exit the current shell
int exit_cmd( int argc, char **argv )
{
        static int ExitFlag = 0;

        argc = (( argc == 1 ) ? gnErrorLevel : atoi( argv[1] ));

        // clean up any open batch files
        while ( exit_bat() == 0 )
                ;

        if ( ExitFlag++ == 0 )
                find_4files( AUTOEXIT );

        // kludge for OS/2 bug
        (void)DosCloseQueue( ghQueueHandle );

        _exit( argc );

        return ERROR_EXIT;
}


// toggle the keyboard (caps lock, numlock, scroll lock)
int keybd_cmd( int argc, char **argv )
{
        char *arg;
        BYTE bKeyStateTable[256];
        int fCapsLock, fNumLock, fScrollLock, fFlag;
        ULONG ulAction, ulLength;
        SHIFTSTATE ss;
        HFILE hf;

        if ( pfnWSKST == 0L )
                return ERROR_EXIT;

        DosOpen( "KBD$", &hf, &ulAction, 0L, 0, FILE_OPEN,
               OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE, 0 );

        ulAction = 0;
        ulLength = sizeof(ss);
        DosDevIOCtl(hf, IOCTL_KEYBOARD, KBD_GETSHIFTSTATE, 0, 0, &ulAction,
                &ss, sizeof(ss), &ulLength);

        // get the current key status
        (*pfnWSKST)( HWND_DESKTOP, bKeyStateTable, FALSE );

        fCapsLock = ( bKeyStateTable[ VK_CAPSLOCK ] & 1 );
        fNumLock = ( bKeyStateTable[ VK_NUMLOCK ] & 1 );
        fScrollLock = ( bKeyStateTable[ VK_SCRLLOCK ] & 1 );

        if ( argc == 1 ) {

                // display the current state
                printf( KBD_CAPS_LOCK, (( fCapsLock ) ? ON : OFF ));
                printf( KBD_NUM_LOCK, (( fNumLock ) ? ON : OFF ));
                printf( KBD_SCROLL_LOCK, (( fScrollLock ) ? ON : OFF ));

        } else {

                while ( *(++argv) != NULL ) {

                        arg = *argv + 2;
                        argc = (( *arg == '\0' ) ? 0 : switch_arg( *argv, "CNS" ));
                        if ( argc <= 0 )
                                return (usage( KEYBD_USAGE ));

                        fFlag = atoi( arg );
                        if ( argc & 1 ) {               // Caps Lock

                                fCapsLock = fFlag;
                                if ( fCapsLock != 0 )
                                        ss.fsState |= CAPSLOCK_ON;
                                else
                                        ss.fsState &= ~CAPSLOCK_ON;
                        } else if ( argc & 2 ) {        // Numlock

                                fNumLock = fFlag;
                                if ( fNumLock != 0 )
                                        ss.fsState |= NUMLOCK_ON;
                                else
                                        ss.fsState &= ~NUMLOCK_ON;
                        } else {                        // Scroll lock

                                fScrollLock = fFlag;
                                if ( fScrollLock != 0 )
                                        ss.fsState |= SCROLLLOCK_ON;
                                else
                                        ss.fsState &= ~SCROLLLOCK_ON;
                        }
                }

                // reset the keyboard
                bKeyStateTable[ VK_CAPSLOCK ] = (BYTE)fCapsLock;
                bKeyStateTable[ VK_NUMLOCK ] = (BYTE)fNumLock;
                bKeyStateTable[ VK_SCRLLOCK ] = (BYTE)fScrollLock;
                (*pfnWSKST)( HWND_DESKTOP, bKeyStateTable, TRUE );
                ulAction = sizeof(ss);
                ulLength = 0;
                DosDevIOCtl( hf, IOCTL_KEYBOARD, KBD_SETSHIFTSTATE, &ss,
                   sizeof(ss), &ulAction, 0, 0, &ulLength );

                DosClose( hf );
        }

        return 0;
}


// emulate the dimwit OS/2 KEYS command
int keys_cmd( int argc, char **argv )
{
        unsigned int row;
        PCH hptr;

        if ( argc == 1 )                // inquiring about KEYS status
                printf( KEYS_IS, gpInternalName, (( gpIniptr->HistMin < MAXLINESIZ) ? ON : OFF));

        else {                  // display history list or set new KEYS status

                if ( stricmp( argv[1], ON ) == 0 ) {

                        gpIniptr->HistMin = 0;
                        gpIniptr->LineIn = 0;
                        add_variable( "KEYS=ON" );

                } else if ( stricmp( argv[1], OFF ) == 0 ) {

                        gpIniptr->HistMin = MAXLINESIZ;
                        gpIniptr->LineIn = 1;
                        add_variable( "KEYS=OFF" );

                } else if ( stricmp( argv[1], KEYS_LIST ) == 0 ) {

                        // display the history list
                        for ( row = 1, hptr = glpHistoryList; ( *hptr != '\0' ); hptr = next_env( hptr), row++)
                                printf( "%5u: %Fs\n", row, hptr );

                } else
                        return ( usage( KEYS_USAGE ));
        }

        return 0;
}


// pump characters from a keyboard buffer into a program
int keystack_cmd( int argc, char **argv )
{
        extern SKEYS *pfnSendKeys;

        static char szKeystack[] = "keystack.exe";
        int rval = 0;
        STARTDATA StartData;
        PID sessionPid;         // Process ID (returned)
        ULONG SessID;           // Session ID (returned)
        char szBuf[256];

        if ( argc == 1 )
                return ( usage( KEYSTACK_USAGE ));

        // call .DLL
        if ( pfnSendKeys == 0L )
                return ( error( ERROR_4DOS_KEYSTACK_NOT_LOADED, NULL ));

        if (( argc = (*pfnSendKeys)( argv[1] )) != 0 )
                return ( error( argc + OFFSET_4DOS_MSG + OFFSET_SENDKEYS_MSG, argv[1] ));

        // make sure we eat the CR/LF keyup
        DosSleep( 75 );

        memset( &StartData, '\0', sizeof(STARTDATA) );
        StartData.Length = sizeof(STARTDATA);

        StartData.Related = SSF_RELATED_INDEPENDENT;
        StartData.PgmControl = SSF_CONTROL_INVISIBLE;

        StartData.FgBg = SSF_FGBG_BACK;
        StartData.ObjectBuffer = szBuf;
        StartData.ObjectBuffLen = 255;
        StartData.TraceOpt = SSF_TRACEOPT_NONE;

        // search for KEYSTACK.EXE
        if (( rval = FindInstalledFile( szBuf, szKeystack )) != 0)
                return ( error( rval, szKeystack ));
        StartData.PgmName = szBuf;

        StartData.SessionType = SSF_TYPE_PM;

        // detach the 32-bit KEYSTACK.EXE
        if (( rval = DosStartSession( &StartData, &SessID, &sessionPid )) != 0 )
                rval = error( rval, StartData.PgmName );

        return rval;
}


// display RAM statistics
int memory_cmd( int argc, char **argv )
{
        unsigned long ram;
        FILESEARCH dir;
        char *pszSwapFile, szBuffer[MAXFILENAME+1];

        printf( TOTAL_OS2_PHYSICAL_RAM, QuerySysInfo(QSV_TOTPHYSMEM) );
        printf( TOTAL_OS2_RESIDENT_RAM, QuerySysInfo(QSV_TOTAVAILMEM) );
        printf( OS2_BYTES_FREE, QuerySysInfo(QSV_TOTRESMEM));

        // display swap file size
        if ( gpIniptr->SwapPath == INI_EMPTYSTR ) {
                pszSwapFile = OS2_SWAPNAME;
                *pszSwapFile = gchSysBootDrive;
        } else {
                pszSwapFile = (char *)( gpIniptr->StrData + gpIniptr->SwapPath);
                if ( is_dir( pszSwapFile )) {
                        pszSwapFile = strcpy( szBuffer, pszSwapFile );
                        mkdirname( pszSwapFile, fname_part( OS2_SWAPNAME ));
                }
        }

        dir.size = 0L;
        if ( find_file( FIND_FIRST, pszSwapFile, 0x100, &dir, NULL ) != NULL )
                printf( OS2_SWAPFILE_SIZE, dir.size );

        // display environment stats
        ram = QueryMemSize( glpEnvironment );

        printf( TOTAL_ENVIRONMENT, ram );
        printf( LBYTES_FREE, (long)(( glpEnvironment + ram ) - ( end_of_env( glpEnvironment ) + 1 )));

        // display alias stats
        ram = QueryMemSize( glpAliasList );
        printf( TOTAL_ALIAS, ram );
        printf( LBYTES_FREE, (long)(( glpAliasList + ram ) - ( end_of_env( glpAliasList ) + 1 )));

        // display history stats
        printf( TOTAL_HISTORY, (long)gpIniptr->HistorySize );

        return 0;
}


// reboot the system
int reboot_cmd( int argc, char **argv )
{
        HFILE fd;
        unsigned int rval;
        long fReboot;
        unsigned long ulAction;

        // check for /S( hutdown) & /V(erify) switche
        if ( GetSwitches( argv[1], "SV", &fReboot, 0 ) != 0 )
                return (usage(REBOOT_USAGE));

        if (( fReboot & 2 ) && ( QueryInputChar( REBOOT_IT, YES_NO ) != YES_CHAR ))
                return 0;

        // run 4EXIT
        find_4files( AUTOEXIT );

        if ( fReboot & 1 ) {
                // shut down the file system
                (void)DosShutdown( 0L );
                (void)DosSleep( 2000L );
                qputs( SHUTDOWN_COMPLETE );
                return 0;
        }

        // Trick OS/2 into warm boot by going through the DOS box!
        if (( rval = DosOpen( "DOS$", &fd, &ulAction, 0L, FILE_NORMAL, FILE_OPEN, (OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_FAIL_ON_ERROR) ,0L )) == 0 ) {

                // be nice & shut down the file system
                HoldSignals();
                (void)DosShutdown( 0L );

                // reboot OS/2!
                (void)DosDevIOCtl( fd, 0xD5, 0xAB, (PVOID)NULL, 0L, (PULONG)NULL, (PVOID)NULL, 0L, (PULONG)NULL );
                (void)DosClose( fd );
        }

        return ( error( rval, DOS_SYS ));
}


#define START_NO_4OS2 1
#define START_TRANSIENT 2
#define START_LOCAL_LISTS 0x10
#define START_LOCAL_ALIAS 0x20
#define START_LOCAL_DIRECTORY 0x40
#define START_LOCAL_HISTORY 0x80
#define START_AND_WAIT 0x100
#define START_WIN_SEAMLESS 0x200
#define START_TCMD_TTY 0x400


// Start another job in a different session.  If no arguments, just start
//   another command processor
int start_cmd( int argc, char **argv )
{
        char *arg;
        char *pszCmd = NULLSTR, *line, szSessionName[62];
        unsigned int i, n, fStartOptions = 0, dos_vdm = 0;
        int rval = 0, fAppType = 0;
        STARTDATA stdata;
        PROGDETAILS Details;
        char achFailName[128];

        memset( (PVOID)&Details, '\0', sizeof(PROGDETAILS) );
        Details.progt.progc = PROG_31_ENHSEAMLESSVDM;

        // save beginning of command line
        line = gszCmdline;

        memset( &stdata, '\0', sizeof(STARTDATA) );

        stdata.Environment = glpEnvironment;    // pass the current environment
        stdata.InheritOpt = 1;          // inherit environment from 4OS2

        Details.swpInitial.fl = SWP_ACTIVATE | SWP_SHOW;

        stdata.ObjectBuffer = achFailName;
        stdata.ObjectBuffLen = sizeof( achFailName );

        if ( setjmp( cv.env ) == -1 )
            rval = CTRLC;

        else {

            // check for command line arguments
            for ( i = 0; (( arg = ntharg( argv[1], i )) != NULL ); i++ ) {

                // get the session title, if any (enclosed in double quotes)
                if ( *arg == DOUBLE_QUOTE ) {

                        // if we already have a title, it must be program name
                        if ( stdata.PgmTitle != 0L )
                                break;

                        if ( sscanf( arg+1, "%60[^\042]", szSessionName ) > 0 )
                                stdata.PgmTitle = szSessionName;
                        continue;

                } else if (*arg != gpIniptr->SwChr)
                        break;

                // must be a startup switch
                if (( stricmp( ++arg, START_BG_STR ) == 0 ) || ( strnicmp( arg, START_BG_STR, 1 ) == 0 )) {

                        // start as background session
                        stdata.FgBg = SSF_FGBG_BACK;

                } else if ( stricmp( arg, START_TRANSIENT_STR ) == 0 ) {

                        // make transient load
                        fStartOptions |= START_TRANSIENT;

                } else if ( strnicmp( arg, START_DOS_STR, 3 ) == 0 ) {

                        // start a foreground DOS VDM
                        dos_vdm = 1;
                        stdata.Environment = 0L;

                        // check if passing DOS strings
                        if ( arg[3] == '=' ) {

                                char szBuffer[512];
set_dos_environment:
                                // next arg is the filename
                                arg += 4;
                                mkfname( arg, 0 );

                                if (( gnGFH = _sopen( arg, (O_RDONLY | O_BINARY),SH_DENYWR)) < 0 )
                                        return ( error( _doserrno, arg ));

                                (void)DosAllocMem((PVOID)&( stdata.Environment),4096,(PAG_COMMIT | PAG_READ | PAG_WRITE));
                                memset( stdata.Environment, '\0', 4096 );

                                for ( n = 0; ( getline( gnGFH, szBuffer, 511, EDIT_DATA ) > 0 ); )
                                        n += sprintf( stdata.Environment+n, FMT_STR, szBuffer ) + 1;

                                gnGFH = _close( gnGFH );
                        }

                } else if ( stricmp( arg, START_FS_STR ) == 0 ) {

                        // start as full-screen foreground session
                        // (must be tested _before_ /FG & /F)
                        stdata.SessionType = NOTWINDOWCOMPAT;
                        Details.progt.progc = PROG_31_ENH;
                        // start full-screen
                        fStartOptions &= ~START_WIN_SEAMLESS;

                } else if (( stricmp( arg, START_FG_STR ) == 0 ) || ( strnicmp( arg, START_FG_STR,1 ) == 0 )) {

                        // make program foreground session
                        stdata.FgBg = 0;

                } else if ( stricmp( arg, START_NOINHERIT_STR ) == 0 ) {

                        // get environment from CONFIG.SYS rather than 4OS2
                        stdata.InheritOpt = 0;
                        stdata.Environment = 0L;

                } else if ( strnicmp( arg, START_ICON_STR, 5 ) == 0 ) {

                        // use specified icon file
                        stdata.IconFile = _alloca( strlen( arg + 4 ));
                        strcpy( stdata.IconFile, arg + 5 );

                } else if ( stricmp( arg, START_INV_STR ) == 0 ) {

                        // start up invisible
                        stdata.PgmControl |= SSF_CONTROL_INVISIBLE;
                        stdata.FgBg = SSF_FGBG_BACK;
                        Details.swpInitial.fl = SWP_HIDE;

                } else if ( stricmp( arg, START_KEEP_STR ) == 0 ) {

                        // start with 4OS2; keep session when program finishes
                        fStartOptions &= ~START_TRANSIENT;

                } else if ( stricmp( arg, START_LOCAL_STR ) == 0 ) {

                        // start session with local alias & history lists
                        fStartOptions |= START_LOCAL_LISTS;

                } else if ( stricmp( arg, START_LA_STR ) == 0 ) {

                        // start session with local alias list
                        fStartOptions |= START_LOCAL_ALIAS;

                } else if ( stricmp( arg, START_LD_STR ) == 0 ) {

                        // start session with local directory list
                        fStartOptions |= START_LOCAL_DIRECTORY;

                } else if ( stricmp( arg, START_LH_STR ) == 0 ) {

                        // start session with local history list
                        fStartOptions |= START_LOCAL_HISTORY;

                } else if ( stricmp( arg, START_MAX_STR ) == 0 ) {

                        // start maximized
                        stdata.PgmControl |= 2;
                        Details.swpInitial.fl = SWP_ACTIVATE | SWP_SHOW | SWP_MAXIMIZE;

                } else if ( stricmp( arg, START_MIN_STR ) == 0 ) {

                        // start minimized
                        stdata.PgmControl |= 4;
                        Details.swpInitial.fl = SWP_MINIMIZE;

                } else if ( stricmp( arg, START_NO_STR ) == 0 ) {

                        // start session without invoking 4OS2
                        fStartOptions |= START_NO_4OS2;

                } else if ( stricmp( arg, START_PGM_STR ) == 0 ) {

                        // next arg is program name, not session name
                        //   no more switches allowed after program name
                        arg = ntharg( argv[1], ++i );
                        break;

                } else if ( stricmp( arg, START_PM_STR ) == 0 ) {

                        // start as foreground PM app
                        stdata.SessionType = WINDOWAPI;

                } else if ( strnicmp( arg, START_POS_STR, 4 ) == 0 ) {

                        // start window at coordinates x,y with size x1, y1
                        if ( sscanf( gpNthptr+5, "%u,%u,%u,%u", &(stdata.InitXPos), &(stdata.InitYPos), &(stdata.InitXSize), &(stdata.InitYSize)) != 4 )
                                goto bad_parm;
                        i += 3;
                        stdata.PgmControl |= 0x8000;
                        Details.swpInitial.fl = SWP_ACTIVATE | SWP_SHOW | SWP_SIZE;

                } else if ( stricmp( arg, START_WAIT_STR ) == 0 ) {

                        // wait for child session to exit before continuing
                        stdata.Related = 1;

                } else if ( stricmp( arg, START_WIN_STR ) == 0 ) {

                        // start as foreground window app
                        stdata.SessionType = WINDOWCOMPAT;

                } else if ( strnicmp( arg, START_WIN3_STR, 4 ) == 0 ) {

                        stdata.Environment = 0L;

                        if ( stdata.SessionType == NOTWINDOWCOMPAT)
                                dos_vdm = 2;
                        else {
                                // start seamless
                                dos_vdm = 2;
                                fStartOptions |= START_WIN_SEAMLESS;
                                stdata.SessionType = WINDOWCOMPAT;
                        }

                        // standard mode seamless session requested?
                        if ( _ctoupper( arg[4] ) == 'S' ) {
                                if ( dos_vdm == 2 )
                                        Details.progt.progc = PROG_31_STDSEAMLESSVDM;
                                else
                                        Details.progt.progc = PROG_31_STD;
                                arg++;
                        }

                        // check for /WIN3( s)=filename
                        if ( arg[4] == '=' ) {
                                arg++;
                                stdata.Environment = 0L;
                                goto set_dos_environment;
                        }

                } else {
bad_parm:
                        return ( error( ERROR_INVALID_PARAMETER, arg ));
                }
            }

            // save the command line arguments
            strcpy( line, (( gpNthptr != NULL ) ? gpNthptr : NULLSTR ));

            // check STARTed command to see what the app type is
            if (( arg != NULL ) && ( *arg != '*' )) {

                // if it's an internal command or alias, use 4OS2/TCMD to run it
                if (( fStartOptions & START_TCMD_TTY) || (( findcmd( arg, 0 ) < 0 ) && ( get_alias( arg ) == NULL ))) {

                        // remove the external command argument
                        strcpy( line, line + strlen( arg ));

                        // if an external command, get the full pathname
                        if (( pszCmd = searchpaths( arg, NULL, TRUE )) == NULL ) {

                                // kludge for START "play.cmd & exit"
                                if (*arg != '=' )
                                        return ( error( ERROR_4DOS_UNKNOWN_COMMAND, arg ));
                                pszCmd = arg;

                        } else {

                                strupr( mkfname( pszCmd, 0 ));

                                // adjust for an HPFS name with embedded whitespace
                                if ( strpbrk( pszCmd, " \t, " ) != NULL ) {
                                        strins( pszCmd, "\"" );
                                        strcat( pszCmd, "\"" );
                                }
                        }

                        // CMD & BTM files default to windowed sessions
                        arg = ext_part( pszCmd );
                        if (( stricmp( arg, CMD ) == 0 ) || ( stricmp( arg, BTM ) == 0 )) {
                                fAppType = WINDOWCOMPAT;
                        // unknown types default to full screen
                        } else if (( fAppType = app_type( pszCmd )) <= 0 )
                                fAppType = NOTWINDOWCOMPAT;

                        else if ( fAppType & 0x1620 ) {

                                // it's a DOS or Windows app or .BAT file
                                dos_vdm = (( fAppType & 0x1600 ) ? 2 : 1 );
                                // start it the same as the current 4OS2
                                //  type (window or FS)
                                fAppType = (( fAppType & 3 ) | (( gpLIS->typeProcess == PT_WINDOWABLEVIO ) ? WINDOWCOMPAT : NOTWINDOWCOMPAT ));
                        }

                        // if session type not specified, or if it's set to /PM
                        //    set session type to program type
                        if (( stdata.SessionType == 0 ) || ((( stdata.SessionType == WINDOWAPI ) || ( fAppType == WINDOWAPI )) && ( dos_vdm == 0 )))
                                stdata.SessionType = fAppType;
                }
            }

            // DOS VDM start requested?
            if ( dos_vdm ) {

                // check for seamless request (Win3 app, /WIN and no /WAIT)
                if (( dos_vdm & 2 ) && ( stdata.Related == 0 ) && ( stdata.SessionType == WINDOWCOMPAT ))
                        fStartOptions |= START_WIN_SEAMLESS;
                else {
                        // VDM has command in PgmInputs, not PgmName
                        strins( line, pszCmd );
                        stdata.PgmName = NULL;
                        fStartOptions &= ~START_WIN_SEAMLESS;
                }

                // is it a Win 3 app?
                if (( dos_vdm & 2 ) && (( fStartOptions & START_WIN_SEAMLESS) == 0 )) {
                        strins( line, " " );
                        strins( line, (( arg = searchpaths( WIN_OS2, NULL, TRUE )) != NULL ) ? arg : WIN_OS2);
                        fStartOptions |= START_TRANSIENT;
                }

                if ( stdata.Environment == glpEnvironment )
                        stdata.Environment = 0L;

                // set 4DOS command line arguments (/C or /K)
                if (( fStartOptions & START_WIN_SEAMLESS ) == 0 )
                        strins( line, (( fStartOptions & START_TRANSIENT ) ? "/C " : "/K " ));

                if ( stdata.SessionType == NOTWINDOWCOMPAT )
                        stdata.SessionType = DOSVDM_FS;
                else if (( stdata.SessionType == WINDOWCOMPAT ) || ( stdata.SessionType == WINDOWAPI ))
                        stdata.SessionType = DOSVDM_WIN;
                else {
                        // start new process in same mode as current one
                        stdata.SessionType = (( gpLIS->typeProcess == PT_WINDOWABLEVIO ) ? DOSVDM_WIN : DOSVDM_FS );
                }
            } else {

                // if it's a PM app, start it without 4OS2
                // if not specified, default to a 4OS2 window
                if ( stdata.SessionType == WINDOWAPI ) {

                        if ( *pszCmd == '\0' ) {
                                // if no program name, it can't be a PM app!
                                stdata.SessionType = WINDOWCOMPAT;
                                fStartOptions &= ~START_NO_4OS2;
                        } else
                                fStartOptions |= START_NO_4OS2;

                } else if ( stdata.SessionType == 0 ) {
                        // start new 4OS2 in same mode as current one
                        stdata.SessionType = (( gpLIS->typeProcess == PT_WINDOWABLEVIO ) ? WINDOWCOMPAT : NOTWINDOWCOMPAT );
                }

                // if NO_OS2, don't invoke 4OS2/TCMD for new session
                //   change program name from 4OS2 to user-specified name
                if ( fStartOptions & START_NO_4OS2 )
                        stdata.PgmName = pszCmd;

                else {

                        // replace original command name w/fully-qualified name
                        if ( *pszCmd )
                                strins( line, pszCmd );

                        // check for local alias / history lists
                        if ( fStartOptions & START_LOCAL_LISTS )
                                strins( line, "/L " );
                        else {

                                if ( fStartOptions & START_LOCAL_ALIAS )
                                        strins( line, "/LA " );
                                if ( fStartOptions & START_LOCAL_DIRECTORY )
                                        strins( line, "/LD " );
                                if ( fStartOptions & START_LOCAL_HISTORY )
                                        strins( line, "/LH " );
                        }

                        // set command line arguments (/C or /K)
                        strins( line, (( fStartOptions & START_TRANSIENT ) ? "/C " : "/K " ));
                }
            }

            stdata.PgmInputs = (PBYTE)line;

            // if it's a seamless WIN app, start it with WinStartApp
            if (( fStartOptions & START_WIN_SEAMLESS ) && ( pfnWSA != 0L )) {
                Details.Length = sizeof( PROGDETAILS );
                Details.progt.fbVisible = SHE_VISIBLE;
                Details.pszExecutable = (( pszCmd[0] == '\0' ) ? "progman.exe" : pszCmd );
                StripQuotes( Details.pszExecutable );
                Details.pszParameters = line;
                Details.pszEnvironment = stdata.Environment;
                Details.pszStartupDir = NULLSTR;
                Details.swpInitial.hwndInsertBehind = HWND_TOP;
                Details.swpInitial.x = stdata.InitXPos;
                Details.swpInitial.y = stdata.InitYPos;
                Details.swpInitial.cx = stdata.InitXSize;
                Details.swpInitial.cy = stdata.InitYSize;
                (*pfnWSA)( NULLHANDLE, &Details, NULL, NULL, SAF_INSTALLEDCMDLINE );
            } else {
                if ( stdata.PgmName != NULL )
                        StripQuotes( stdata.PgmName );
                rval = start_session( &stdata, pszCmd );
            }
        }

        if (( dos_vdm ) && ( stdata.Environment != 0L ))
                (void)DosFreeMem( stdata.Environment );

        return rval;
}


// internal part of SHRALIAS command
int shralias_cmd( int argc, char **argv )
{
        static char SharedAliases[] = SHR_4OS2_ALIAS;
        static char SharedHistory[] = SHR_4OS2_HISTORY;
        static char SharedDirHist[] = SHR_4OS2_DIRHIST;
        static char SentinelSem[] = SHR_4OS2_SENTINEL;

        static char szShrAlias[] = "shralias.exe";
        char szBuf[MAXFILENAME];
        char *pszPgmName;

        int rval = 0;
        unsigned int fUnload = 0, fSentinel;
        PCH HistoryList, AliasList, DirectoryList;
        HEV hev = 0;
        RESULTCODES rescResults;
        static char szCmdLine[] = "shralias.exe\0/4os2\0\0";

        // check for and remove switches
        if ( argc > 1 ) {
                if ( _stricmp( argv[1], "/U" ) != 0 )
                        return ( usage( SHRALIAS_USAGE ));
                fUnload = 1;
        }

        // Check to see if the sentinel semaphore already exists
        fSentinel = DosOpenEventSem( SentinelSem, &hev );

        if ( fUnload ) {

                // unload request
                if ( fSentinel == 0 ) {
                        // post semaphore to signal detached SHRALIAS to die
                        DosPostEventSem( hev );
                        DosCloseEventSem( hev );
                        qputs( "SHRALIAS unloaded\n" );
                } else {
                        qprintf( STDERR, "SHRALIAS not loaded\n" );
                        rval = ERROR_EXIT;
                }

        } else if ( fSentinel == 0 ) {

                // Must be a copy already running; we don't need another
                qprintf( STDERR, "SHRALIAS already loaded\n" );
                DosCloseEventSem( hev );
                rval = ERROR_EXIT;

        } else {

                // detach SHRALIAS.EXE

                // check for existence of shared memory
                if (( DosGetNamedSharedMem( (PVOID)&HistoryList, SharedAliases, PAG_READ | PAG_WRITE ) != 0 ) && ( DosGetNamedSharedMem( (PVOID)&AliasList, SharedHistory, PAG_READ | PAG_WRITE ) != 0 )
                            && ( DosGetNamedSharedMem( (PVOID)&DirectoryList, SharedDirHist, PAG_READ | PAG_WRITE ) != 0 )) {
                        qprintf( STDERR, "No shared memory found\n" );
                        return ERROR_EXIT;
                }

                // search for SHRALIAS.EXE
                if (( rval = FindInstalledFile( szBuf, szShrAlias )) != 0)
                        return ( error( rval, szShrAlias ));
                pszPgmName = szBuf;

                // run the 32-bit version from 4OS2 as a detached session
                if (( rval = DosExecPgm( szBuf, sizeof( szBuf ), EXEC_BACKGROUND, szCmdLine, NULL, &rescResults, pszPgmName )) != 0 )
                        rval = error( rval, pszPgmName );
                else
                        qputs( "SHRALIAS loaded\n" );
        }

        return rval;
}


// change the window title
int title_cmd( int argc, char **argv )
{
        if ( argc == 1 )
                return ( usage( TITLE_USAGE ));

        strcpy( gszSessionTitle, argv[1] );
        update_task_list( NULL );

        return 0;
}

// change the window state (minimized, maximized, or normalized)
int window_cmd( int argc, char **argv )
{
        char *arg;
        int x, y, cx, cy;

        if ( ghwndWindowHandle == NULL )
                return ( error( ERROR_4DOS_NOT_WINDOWED, NULL ));

        if ( argc == 1 )
                return ( usage( WINDOW_USAGE ));

        // allow options with or without leading '/'
        arg = argv[1];
        if ( *arg == '/' )
                arg++;

        if ( strnicmp( arg, "MAX", 3 ) == 0 ) {
                if ( pfnWPM != NULL )
                        (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_MAXIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
        } else if ( strnicmp( arg, "MIN", 3 ) == 0 ) {
                if ( pfnWPM != NULL )
                        (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_MINIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
        } else if ( strnicmp( arg, "POS", 3 ) == 0 ) {

                // set position & size
                if ( sscanf( arg+4, "%d,%d,%d,%d", &x, &y, &cx, &cy ) != 4)
                        return ( usage( WINDOW_USAGE ));
                if ( pfnWSWP != NULL )
                        (*pfnWSWP)( ghwndWindowHandle, (HWND)0, x, y, cx, cy, SWP_SIZE | SWP_MOVE );
        } else if ( strnicmp( arg, "RES", 3 ) == 0 ) {
                if ( pfnWPM != NULL )
                        (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_RESTORE), MPFROM2SHORT(CMDSRC_MENU, TRUE) );
        } else if ( *arg == '"' ) {

                char szWindowTitle[64];

                if ( sscanf( arg+1, "%60[^\042]", szWindowTitle ) > 0 ) {

                        if ( cv.bn >= 0 ) {
                                if ( bframe[cv.bn].pszTitle != NULL )
                                        free( bframe[cv.bn].pszTitle );
                                bframe[cv.bn].pszTitle = _strdup( szWindowTitle );
                        } else
                                strcpy( gszSessionTitle, szWindowTitle );

                        update_task_list( NULL );
                }

        } else
                return ( usage( WINDOW_USAGE ));

        return 0;
}

