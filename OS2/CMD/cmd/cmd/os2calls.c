// OS2CALLS.C - OS/2 routines for 4OS2 & TCMD/PM
//   Copyright ( c) 1992 - 1997  Rex C. Conn   All rights reserved

#include "product.h"

#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "4all.h"


char szWindowTitle[128];

int EXPENTRY rexx_subcom( PRXSTRING, PUSHORT, PRXSTRING );

int nRexxError;         // if != 0 on return, signal REXX than an error occurred

static unsigned long SignalNestingLevel = 0L;
typedef LONG APIENTRY RXSTART( LONG, PRXSTRING, PSZ, PRXSTRING, PSZ, LONG, PRXSYSEXIT, PSHORT, PRXSTRING );
RXSTART *pfnREXXSTART = 0L;
typedef APIRET APIENTRY RXHALT( PID, TID );
static RXHALT *pfnREXXSETHALT = 0L;
static HMODULE hModREXX;
static HMODULE hModREXXAPI;


/*     PEXCEPTIONREPORTRECORD       = Report Record             */
/*     PEXCEPTIONREGISTRATIONRECORD = Registration Record       */
/*     PCONTEXTRECORD               = Context Record            */
/*     PVOID                        = System Reserved           */
ULONG BreakHandler( PEXCEPTIONREPORTRECORD pxcptrepr, PEXCEPTIONREGISTRATIONRECORD pxcptregr, PCONTEXTRECORD pcr, PVOID sysinfo )
{
    // OS/2 doesn't print ^C
    if ( EH_EXIT_UNWIND & pxcptrepr->fHandlerFlags )
        return (XCPT_CONTINUE_SEARCH);

    if ( EH_UNWINDING & pxcptrepr->fHandlerFlags )
        return XCPT_CONTINUE_SEARCH;

    if ( EH_NESTED_CALL & pxcptrepr->fHandlerFlags )
        return XCPT_CONTINUE_SEARCH;

    // first, make sure it's a signal & not an exception
    if ( pxcptrepr->ExceptionNum != XCPT_SIGNAL )
        return XCPT_CONTINUE_SEARCH;

    // disable signal handling momentarily
    HoldSignals();
    if (( pxcptrepr->ExceptionInfo[0] == XCPT_SIGNAL_INTR ) || ( pxcptrepr->ExceptionInfo[0] == XCPT_SIGNAL_BREAK )) {
        (void)VioWrtTTY( "^C\r\n", 4, 0 );
    }

    // acknowledge the signal
    (void)DosAcknowledgeSignalException( pxcptrepr->ExceptionInfo[0] );

    if ( pxcptrepr->ExceptionInfo[0] == XCPT_SIGNAL_KILLPROC )
        _exit( CTRLC );

    cv.exception_flag |= CTRLC;     // flag batch files to abort

    // reset DO / IFF parsing flags if not in a batch file
    if ( cv.bn < 0 )
        cv.f.lFlags = 0;

    BreakOut();

    return 1L;
}


void BreakOut( void )
{
    extern int fNoComma;

    fNoComma = 0;

    // if we're running a child process or pipe, stomp it
    if ( gnChildPipeProcess != 0 ) {
        (void)DosKillProcess( 0, gnChildPipeProcess );
        gnChildPipeProcess = 0;
    }

    if ( gnChildProcess != 0 ) {
        (void)DosKillProcess( 0, gnChildProcess );
        (void)VioWrtTTY( EXTERN_BREAK, strlen( EXTERN_BREAK ), 0 );
        gnChildProcess = 0;
    }

    SetCurSize( 0 );                // reset normal cursor shape

    if ( gnGFH > 0 )                // close global file descriptor?
        gnGFH = _close( gnGFH );

    longjmp( cv.env, -1 );          // jump to last setjmp()
}


// Execute an external program (.COM or .EXE)
int external( int argc, char **argv )
{
    int         appType;
    char        *pName, szOs2CmdLine[MAXLINESIZ+4];
    STARTDATA   stdata;
    ULONG       pidProcess;
    char        *arg, achFailName[128];
    char        szComspec[MAXFILENAME];
    PCH         feptr;
    RESULTCODES rescResults;

    // initialize start structure
    memset( &stdata, '\0', sizeof(STARTDATA) );

    if ( argv[1] == NULL )
        argv[1] = NULLSTR;

    pName = fname_part( argv[0] );

    // check for a "4OS2 /C 4OS2" & turn it into a "4OS2"
    if (( cv.bn < 0 ) && ( gnTransient ) && ( stricmp( pName, OS2_NAME ) == 0 ) && ( strnicmp( argv[1], "/C", 2 ) != 0 )) {
        // turn off /C switch
        gnTransient = 0;

        // print the copyright that wasn't done the first time
        DisplayCopyright();

        return ( command( argv[1], 0 ));
    }

    // check the app type - if it's a PM app, use DosStartSession
    //   instead of DosExecPgm
    if (( appType = app_type( argv[0] )) < 0 )
        return ( error( -appType, argv[0] ));

    // make sure command args have leading whitespace!
    if (( argv[1] != NULLSTR ) && ( isspace( argv[1][-1] )))
        argv[1]--;

    // if the app is a PM app, or if it's a full-screen app & we're in
    //   an AVIO window, or if it's a DOS or Windows app in OS/2 2.0,
    //   use DosStartSession
    if (( appType & 0x1620 ) || ( appType == WINDOWAPI ) || (( appType < 2 ) && ( gpLIS->typeProcess == PT_WINDOWABLEVIO ))) {
        stdata.InheritOpt = 1;
        stdata.Related = 1;
        stdata.SessionType = appType & 0x3;

        // don't wait on PM/DOS/WIN apps started from command line
        if (( appType >= WINDOWAPI ) && ( cv.bn < 0 ) && ( gnTransient == 0 ) && ( gpIniptr->ExecWait == 0 ))
            stdata.Related = 0;

        // is it a DOS or Windows app?
        if ( appType & 0x1620 ) {
            // start in same type as current session
            stdata.SessionType = (( gpLIS->typeProcess == PT_WINDOWABLEVIO ) ? DOSVDM_WIN : DOSVDM_FS );
            sprintf( szOs2CmdLine, "/C %s%s", argv[0], argv[1] );

            // if it's a Windows app, insert the Windows program
            //   name (WINOS2.COM) into the command line arguments
            if ( appType & 0x1600 ) {
                // save argv[0] before another searchpaths()
                arg = _alloca( strlen( argv[0] ) + 1 );
                argv[0] = strcpy( arg, argv[0] );
                strins( szOs2CmdLine + 3, " " );
                strins( szOs2CmdLine + 3, (( argv[1] = searchpaths( WIN_OS2, NULL, TRUE )) != NULL ) ? argv[1] : WIN_OS2);
                stdata.SessionType = DOSVDM_FS;
            }

            stdata.PgmTitle = pName;
            stdata.PgmInputs = szOs2CmdLine;
        } else {
            stdata.PgmName = argv[0];
            stdata.PgmInputs = (PBYTE)argv[1];
            stdata.Environment = glpEnvironment;
        }

        // call DosStartSession
        gnErrorLevel = start_session( &stdata, argv[0] );
    } else {
        // copy the INI data for secondary copies of 4OS2
        SaveIniData();

        // format the argument string
        sprintf( szOs2CmdLine, "%s%c%.*s%c", argv[0], 0, (MAXLINESIZ-strlen( argv[0])), argv[1], 0 );

        // if we're executing CMD.EXE, change COMSPEC
        szComspec[0] = '\0';
        if ( stricmp( CMD_EXE, fname_part( argv[0] )) == 0 ) {
            if (( feptr = get_variable( COMSPEC )) != NULL )
                strcpy( szComspec, feptr );

            sprintf( gszCmdline, FMT_TWO_EQUAL_STR, COMSPEC, argv[0] );
            add_variable( gszCmdline );
        }

        gnErrorLevel = DosExecPgm( achFailName, sizeof( achFailName ), EXEC_ASYNCRESULT, szOs2CmdLine, glpEnvironment, &rescResults, argv[0] );
        (void)VioSetAnsi( ANSI_ON, 0 ); // enable ANSI escape sequences

        // reset COMSPEC if necessary
        if ( szComspec[0] ) {
            sprintf( gszCmdline, FMT_TWO_EQUAL_STR, COMSPEC, szComspec );
            add_variable( gszCmdline );
        }

        if ( gnErrorLevel != 0 )
            gnErrorLevel = error( gnErrorLevel, achFailName );

        else {
            // trap ^C's here so we can clean up child process
            if ( setjmp( cv.env ) != -1 )
                gnChildProcess = rescResults.codeTerminate;

            // wait for child to end
            (void)DosWaitChild( DCWA_PROCESSTREE, DCWW_WAIT, &rescResults, &pidProcess, gnChildProcess );
            gnChildProcess = 0;
            gnErrorLevel = rescResults.codeResult;

            // disable signal handling momentarily
            HoldSignals();
        }
    }

    return gnErrorLevel;
}


// initialize the REXX "back door"
int InitializeREXX( int fError )
{
    static int initialized = 0;
    static char szSubcomExe[] = "RexxRegisterSubcomExe";
    // pointer to RxSubcomRegister
    typedef APIRET APIENTRY RXREGISTER( PSZ, PFN, PUCHAR );
    RXREGISTER *pfnREXXREGISTER = 0L;
    int rval;
    char buf[128];

    if ( initialized != 0 )
        return 0;

    // initialize REXXSAA pointer (& leave the module active?)
    if ((( rval = DosLoadModule( buf, sizeof(buf), "REXX", &hModREXX )) != 0 ) || (( rval = DosQueryProcAddr( hModREXX, 0, "RexxStart", (PFN *)&pfnREXXSTART)) != 0 ) || (( rval = DosQueryProcAddr( hModREXX,0,"RexxSetHalt",(PFN *)&pfnREXXSETHALT)) != 0 ))
        return (( fError ) ? error( rval, "REXX" ) : ERROR_EXIT);

    // initialize RexxRegisterSubcomExe pointer & define subcommand handler
    if ((( rval = DosLoadModule( buf, sizeof(buf), "REXXAPI", &hModREXXAPI)) != 0 ) || (( rval = DosQueryProcAddr( hModREXXAPI, 0, (PSZ)szSubcomExe, (PFN *)&pfnREXXREGISTER )) != 0 ))
        return (( fError ) ? error( rval, szSubcomExe ) : ERROR_EXIT);

    // register the subcommand handler
    (*pfnREXXREGISTER)( "CMD", (PFN)&rexx_subcom, 0L );
    (*pfnREXXREGISTER)( "4OS2", (PFN)&rexx_subcom, 0L );

    initialized++;

    return 0;
}


// deinstall the REXX "back door"
int UninstallREXX( void )
{
    static char szSubcom[] = "RexxDeregisterSubcom";
    // pointer to RexxDeregisterSubcom & RexxDeregisterExit
    typedef APIRET APIENTRY RXDEREGISTER( PSZ, PSZ );
    RXDEREGISTER *pfnREXXDEREGISTER = 0L;

    // Raise a HALT condition in any running REXX program
    if ( pfnREXXSETHALT != 0L )
        (*pfnREXXSETHALT)( gpLIS->pidCurrent, gpLIS->tidCurrent );

    if ( DosQueryProcAddr( hModREXXAPI, 0, (PSZ)szSubcom, (PFN *)&pfnREXXDEREGISTER ) != 0 )
        return ERROR_EXIT;

    // deregister the subcommand handler
    (*pfnREXXDEREGISTER)( "CMD", 0L );
    (*pfnREXXDEREGISTER)( "4OS2", 0L );

    return 0;
}


// check if a .CMD file is a REXX file; if yes, call the REXX interpreter
// if no, check if .CMD file requests an external processor
int process_rexx( char *pszCmdName, char *line, int fRexx )
{
    char *arg;

    // open the .CMD file
    if (( gnGFH = _sopen( pszCmdName, (O_RDONLY | O_BINARY), SH_DENYWR )) > 0 ) {
        // save the passed line so we can use "gszCmdline" for scratch
        arg = _alloca( strlen( line ) + 1 );
        line = strcpy( arg, line );

        // read the first line of the file
        getline( gnGFH, gszCmdline, MAXLINESIZ - 1, EDIT_DATA );
        gnGFH = _close( gnGFH );

        // if it's a REXX comment (begins with "/*"), call the
        //   REXX interpreter
        if (( gszCmdline[0] == '/' ) && ( gszCmdline[1] == '*' )) {
            // set flag for batch()
            cv.call_flag = 0x200;

            // point gpRexxCmdline to processed but non-terminated
            //   command line arguments
            (void)parse_line( pszCmdName, line, NULL, CMD_STRIP_QUOTES );
            gpRexxCmdline = _alloca( strlen( line ) + 1 );
            strcpy( gpRexxCmdline, line );
        } else if ((( arg = first_arg( gszCmdline )) != NULL ) && ( stricmp( arg,"EXTPROC") == 0 )) {
            // EXTPROC request - call an external processor
            //   next arg is processor name
            (void)next_arg( gszCmdline, 1 );

            // add the .CMD name & options
            if (( arg = path_part( gpBatchName )) == NULL )
                    arg = NULLSTR;
            sprintf( strend( gszCmdline )," %s%s%s", arg, fname_part( pszCmdName ), line );
            return ( command( gszCmdline, 0 ));
        }
    }

    // call the batch file processor
    return ( parse_line( pszCmdName, line, batch, CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_CLOSE_BATCH | CMD_UCASE_CMD ));
}


// 4OS2 REXX subcommand handler
int EXPENTRY rexx_subcom( PRXSTRING cmd, PUSHORT flags, PRXSTRING result )
{
    // OS/2 2.x exception handling has to go on the stack!
    EXCEPTIONREGISTRATIONRECORD RexxExceptionStruct;
    ULONG ulTimes = 0L;
    int rc;

    HoldSignals();
    RexxExceptionStruct.ExceptionHandler = (_ERR *)&BreakHandler;
    (void)DosSetExceptionHandler( &RexxExceptionStruct );
    (void)DosSetSignalExceptionFocus( SIG_SETFOCUS, &ulTimes );
    EnableSignals();

    // for some reason, returning NULL & 0L doesn't work, so we'll return
    //   a string value of "0" (overridden by the return value of command())
    strcpy( result->strptr, "0" );
    result->strlength = 1L;

    *flags = RXSUBCOM_OK;
    nRexxError = 0;

    // OS/2 1.3 doesn't put a '\0' on the end of the command string
    memmove( gszCmdline, cmd->strptr, (UINT)(cmd->strlength) );
    gszCmdline[ (UINT)(cmd->strlength) ] = '\0';

    rc = command( gszCmdline, 0 );

    if ( nRexxError )
        *flags = RXSUBCOM_ERROR;

    (void)DosUnsetExceptionHandler( &RexxExceptionStruct );
    sprintf( result->strptr, FMT_UINT, rc );

    EnableSignals();

    // if 4OS2 trapped a ^C, signal the REXX process
    if ( cv.exception_flag ) {
        cv.exception_flag = 0;
        if ( pfnREXXSETHALT != 0L )
            (*pfnREXXSETHALT)( gpLIS->pidCurrent, gpLIS->tidCurrent );
    }

    return rc;
}


// call the REXX interpreter
int nCallRexx( void )
{
    int             rc;
    SHORT           rexxrc = 0;
    RXSTRING        rexx_cmd, rexx_result;
    CRITICAL_VARS   save_cv;

    // initialize the REXX back door
    if ( InitializeREXX( 1 ))
        return ERROR_EXIT;

    // call REXX to execute a batch file
    bframe[ cv.bn ].flags |= BATCH_REXX;

    // save the arguments in REXX string format
    rexx_cmd.strlength = (ULONG)strlen( gpRexxCmdline );
    rexx_cmd.strptr = (PCH)gpRexxCmdline;

    rexx_result.strlength = 0;
    rexx_result.strptr = 0L;

    // save the critical variables
    memmove( &save_cv, &cv, sizeof(cv) );

    rc = (*pfnREXXSTART)(( *gpRexxCmdline != '\0' ), &rexx_cmd, bframe[cv.bn].pszBatchName, 0, "CMD", RXCOMMAND, NULL, &rexxrc, &rexx_result );

    // free any memory allocated w/REXX "EXIT" or "RETURN"
    FreeMem( rexx_result.strptr );

    bframe[cv.bn].flags &= ~BATCH_REXX;

    // restore the critical vars
    memmove( &cv, &save_cv, sizeof(cv) );

    return (( rc == 0 ) ? rexxrc : rc );
}


// call VIEW.EXE
int _help(char *arg, char *opts)
{
    char argline[128];
    PROGDETAILS Details;

    if ( arg != NULL ) {
        // skip leading *
        while (*arg == '*' )
            arg++;
    }

    // check for TTY-style help requested
    if ( opts != NULL ) {
        UINT size = 4096;
        ULONG nMsgLen;
        PCHAR lpMsg;

        lpMsg = AllocMem( &size );
        nMsgLen = size;
        if (findcmdhelp( arg, 1 )==0) {
            if ( DosGetMessage( NULL, 0, lpMsg, size, findcmd( arg, 1 ), "OSO001H.MSG", &nMsgLen ) == 0 ) {
                lpMsg[nMsgLen] = '\0';
                printf( FMT_FAR_STR, lpMsg);

                FreeMem( lpMsg );
            }
        } else {
            if ( DosGetMessage( NULL, 0, lpMsg, size, findcmdhelp( arg, 1 ), "OSO001.MSG", &nMsgLen ) == 0 ) {
                lpMsg[nMsgLen] = '\0';
                printf( FMT_FAR_STR, lpMsg);

                FreeMem( lpMsg );
            }
        }
        return 0;
    }

    // initialize start structure
    memset( &Details, '\0', sizeof(PROGDETAILS) );

    Details.Length = sizeof(PROGDETAILS);
    Details.progt.progc = PROG_PM;
    Details.progt.fbVisible = SHE_VISIBLE;
    Details.swpInitial.fl = SWP_ACTIVATE | SWP_SHOW | SWP_SIZE | SWP_MOVE;
    Details.pszEnvironment = glpEnvironment;
    Details.pszStartupDir = NULLSTR;
    Details.swpInitial.hwndInsertBehind = HWND_TOP;

    // get VIEW.EXE location
    if (( Details.pszExecutable = searchpaths( HELP_EXE, NULL, TRUE )) == 0L ) {
        // can't find VIEW.EXE
        honk();
        return ERROR_EXIT;
    }

    // check for other .INF files requested
    //   (if > 1 arg, first arg is .INF name )
    if ( ntharg( arg, 1 ) != NULL )
        argline[0] = '\0';
    else if ( gpIniptr->HelpBook != INI_EMPTYSTR)
        strcpy( argline, ( char *)( gpIniptr->StrData + gpIniptr->HelpBook) );
    else {
        strcpy( argline, "CMDREF" );
    }

    // check for command line arguments
    if ( arg != NULL ) {
        strip_trailing( arg, SLASHES );
        sprintf( strend( argline ), " %.*s", ( 125 - strlen( argline )),arg);
    }

    arg = skipspace( argline );

    // start "VIEW.EXE *.INF ..." in the desktop window
    if ( pfnWSA != NULL )
        (*pfnWSA)( NULLHANDLE, &Details, arg, NULL, 0 );

    return 0;
}


int FindInstalledFile( char *path, char *filename )
{
    strcpy( path, _pgmptr );
    insert_path( path, filename, path );
    return !is_file_or_dir( path );
}

// call OPTION.EXE
int _option( void )
{
    int             rval = 0;
    char            szOptionName[MAXFILENAME], szArgs[64], *pszHelp;
    char            *pIniData = 0L, *pIniStrings, *pIniKeys, *pSaveStrData;
    unsigned int    *pSaveKeys;
    STARTDATA       stdata;

    // Search for OPTION.EXE, complain and return if not found
    if (( rval = FindInstalledFile( szOptionName, OPTION_EXE )) != 0)
        return ( error( rval, OPTION_EXE ));

    // get shared memory and set up pointers
    sprintf( szArgs, "\\SHAREMEM\\4OS2%x.OPT", gpLIS->pidCurrent );
    if (( rval = DosAllocSharedMem( (PPVOID)&pIniData, szArgs, INI_TOTAL_BYTES, PAG_COMMIT | PAG_READ | PAG_WRITE )) != 0 )
        return ( error( rval, szArgs ));

    pIniStrings = pIniData + sizeof(INIFILE);
    pIniKeys = pIniStrings + INI_STRMAX;

    // Save string and keys pointers
    pSaveStrData = gpIniptr->StrData;
    pSaveKeys = gpIniptr->Keys;

    // copy INIFILE data to shared memory
    memmove( pIniData, (char *)gpIniptr, sizeof(INIFILE) );
    memmove( pIniStrings, gpIniptr->StrData, INI_STRMAX);
    memmove( pIniKeys, (char *)(gpIniptr->Keys), INI_KEYS_SIZE);

    // Find VIEW.EXE and make its name the second argument
    if (( pszHelp = searchpaths( HELP_EXE, NULL, TRUE )) != 0L )
        sprintf(strend(szArgs), " %s", pszHelp);

    // initialize start structure
    memset( &stdata, '\0', sizeof(STARTDATA) );
    stdata.InheritOpt = 1;
    stdata.SessionType = WINDOWAPI;
    stdata.PgmName = szOptionName;
    stdata.PgmInputs = (PBYTE)szArgs;
    stdata.Environment = glpEnvironment;
    stdata.Related = 1;

    // Run OPTION.EXE
    if ((rval = start_session( &stdata, szOptionName )) == 0) {
        // Successful -- retrieve modified data, restore pointers
        memmove( (char *)gpIniptr, pIniData, sizeof(INIFILE));
        gpIniptr->StrData = pSaveStrData;
        gpIniptr->Keys = pSaveKeys;
        memmove( gpIniptr->StrData, pIniStrings, INI_STRMAX );
        memmove( (char *)(gpIniptr->Keys), pIniKeys, INI_KEYS_SIZE );
    }

    DosFreeMem( pIniData );
    update_task_list( NULL );
    return 0;
}


// Set up output side of a pipe.
int open_pipe( REDIR_IO *redirect, char *line )
{
    int             i, rval;
    unsigned int    uSize;
    RESULTCODES     rescResults;
    char            *arg, *pszCmd, *pszCmd2, *pszExt, c;
    char            achFailName[128], szOs2CmdLine[MAXLINESIZ+MAXFILENAME+2];
    char            szExpandedLine[MAXLINESIZ];
    HFILE           hPipeRead, hPipeWrite;

    // create the pipe
    if (( rval = DosCreatePipe( &hPipeRead, &hPipeWrite, 8192 )) != 0 )
        return ( error( rval, NULL ));

    // save STDIN & change it to point to the read handle
    redirect->std[STDIN] = _dup( STDIN );
    dup_handle( _hdopen( hPipeRead, O_TEXT | O_RDONLY ), STDIN );

    // make sure we're in text mode
    (void)_setmode( STDIN, O_TEXT );

    // make the write handle non-inheritable ( so it will send EOF)
    NoInherit( hPipeWrite );

    // |& means pipe STDERR too
    if (( c = *line ) == '&' )
        line++;

    szOs2CmdLine[0] = '\0';
    line = skipspace( line );
    pszCmd = pszCmd2 = first_arg( line );

    // kludge for "prog | *prog2"
    if (( pszCmd2 != NULL ) && ( *pszCmd2 == '*' ))
        pszCmd2++;

    // if not internal, alias, or redirection, try executing w/o 4OS2 / TCMD/2
    if (( pszCmd != NULL ) && ( findcmd( pszCmd2, 0 ) < 0 ) && ( get_alias( pszCmd ) == 0L ) && ( strpbrk( line, "<>|&" ) == NULL )) {
        // check for an external command
        if ((( arg = searchpaths( pszCmd2, NULL, TRUE )) != NULL ) && (( pszExt = ext_part( arg)) != NULL ) && ( app_type( arg ) != WINDOWAPI)) {
            if (( _stricmp( pszExt, EXE ) == 0 ) || ( _stricmp( pszExt, COM ) == 0 )) {
                strcpy( szExpandedLine, line + strlen( pszCmd ));
                var_expand( szExpandedLine, 0 );
                sprintf( szOs2CmdLine, "%s%c%.*s%c", arg, 0, (MAXLINESIZ - strlen( arg )), szExpandedLine, 0 );
            }
        }
    }

    // We may have to execute a copy of the command processor, in case
    //   the user is piping to an internal command (i.e., "dir | list /s")
    if ( szOs2CmdLine[0] == '\0' ) {
        // copy the INI data for secondary copies of 4OS2
        SaveIniData();

        // make sure nobody else is using this name!
        for ( i = 'A'; ( redirect->pPipeSource == 0 ); i++ ) {
            sprintf( szOs2CmdLine, SHAREMEM_NAME, gpLIS->pidCurrent, i );

            uSize = sizeof(CRITICAL_VARS) + ( sizeof(BATCHFRAME) * MAXBATCH) + MAXLINESIZ + MAXFILENAME + 0x10;
            rval = DosAllocSharedMem( (PPVOID)&(redirect->pPipeSource), szOs2CmdLine, uSize, PAG_COMMIT | PAG_READ | PAG_WRITE );
            if (( rval != 0 ) && ( rval != ERROR_ALREADY_EXISTS ))
                return rval;
        }

        // save critical vars & batch file arguments to shared memory
        redirect->pPipeSource->fInitialized = 0;
        memmove( redirect->pPipeSource->CriticalVars, &cv, sizeof(CRITICAL_VARS) );
        memmove( redirect->pPipeSource->BatchFrame, bframe, ( sizeof(BATCHFRAME) * MAXBATCH) );

        // put the shared memory name in the environment so the child process
        //   can find it
        strins( szOs2CmdLine, SHAREMEM_PIPE_ENV );

        add_variable( szOs2CmdLine );

        // clear the batch file args (%0 - %n) area
        memset( szOs2CmdLine, '\0', MAXLINESIZ + MAXFILENAME + 1 );

        if ( cv.bn >= 0 ) {
            // save current batch filename & arg list
            arg = szOs2CmdLine;
            arg += sprintf( arg, FMT_STR, bframe[cv.bn].pszBatchName ) + 1;

            for ( i = 0; ( bframe[cv.bn].Argv[i] != NULL ); i++ )
                arg += sprintf( arg, FMT_STR, bframe[cv.bn].Argv[i] ) + 1;
        }

        memmove( redirect->pPipeSource->CommandLine, szOs2CmdLine, MAXLINESIZ + MAXFILENAME );

        sprintf( szOs2CmdLine, "%s%c/c %.*s%c", _pgmptr, 0, ( MAXLINESIZ - strlen( _pgmptr )), line, 0 );
        arg = NULL;
    } else
        redirect->pPipeSource = 0L;

    if (( rval = DosExecPgm( achFailName, sizeof( achFailName ), EXEC_ASYNCRESULT, szOs2CmdLine, glpEnvironment, &rescResults, (( arg != NULL ) ? arg : _pgmptr ))) != 0 ) {
        rval = gnErrorLevel = error( rval, achFailName );

    } else {
        // set the "pipe active" flag to the ID of the child process
        redirect->nChildPipe = redirect->pipe_open = rescResults.codeTerminate;
        gnChildPipeProcess = rescResults.codeTerminate;

        // wait for 4OS2 child to acknowledge receipt
        if ( redirect->pPipeSource ) {
            for ( i = 0; (( redirect->pPipeSource->fInitialized == 0 ) && ( i < 400 )); i++ )
                DosSleep( 25 );
        }
    }

    // restore STDIN
    dup_handle( redirect->std[STDIN], STDIN );
    redirect->std[STDIN] = 0;

    if ( rval )
        return rval;

    // save STDOUT & point it to the pipe
    redirect->std[STDOUT] = _dup( STDOUT );
    // TODO: check - was O_RDWR
    dup_handle( _hdopen( hPipeWrite, O_TEXT | O_WRONLY ), STDOUT );

    // make sure we're in text mode
    (void)_setmode( STDOUT, O_TEXT );

    if ( c == '&' ) {
        // pipe STDERR too
        redirect->std[STDERR] = _dup( STDERR );
        _dup2( STDOUT, STDERR );
    }

    return 0;
}


// disable inheritance for the specified handle
void NoInherit( HFILE hFH )
{
    unsigned int uHandleState = 0;

    (void)DosQueryFHState( hFH, (PULONG)&uHandleState );
    uHandleState &= 0x7F88;         // remove non-settable bits
    uHandleState |= 0x0080;         // set non-inheritance bit

    (void)DosSetFHState( hFH, uHandleState );
}


// read a block from the specified file
int FileRead(int fd, char *fptr, unsigned int length, unsigned int *bytes_read)
{
    return ( DosRead( fd, fptr, (ULONG)length, (PULONG)bytes_read ));
}


// write a block to the specified file
int FileWrite( int fd, char *fptr, unsigned int length, unsigned int *bytes_written )
{
    return ( DosWrite( fd, fptr, (ULONG)length, (PULONG)bytes_written ));
}


// free OS2 memory block
void FreeMem( char *fptr )
{
    if ( fptr != 0L )
        (void)DosFreeMem( fptr );
}


// allocate OS2 memory
char *AllocMem(unsigned int *size )
{
    PVOID BaseAddress = 0L;
    ULONG ulAlloc = (( (ULONG)*size + 0x17FFFFL ) & 0xFFF00000L );

    // allocate (but don't commit!) amount rounded to nearest MB, so we'll have lots of room
    if ( DosAllocMem( &BaseAddress, ulAlloc, (PAG_WRITE | PAG_READ)) == 0 )
        (void)DosSetMem( BaseAddress, (ULONG)*size, (PAG_READ | PAG_WRITE | PAG_COMMIT ));

    return (char *)BaseAddress;
}


// grow or shrink OS2 memory block
char *ReallocMem( char *fptr, ULONG size )
{
    ULONG old_size;

    if ( fptr == 0L )
        return ( AllocMem(( unsigned int *)&size ));

    if (( old_size = QueryMemSize( fptr )) < size ) {
        if ( DosSetMem( fptr+old_size, ( size-old_size ), PAG_READ | PAG_WRITE | PAG_COMMIT) != 0 ) {
            FreeMem( fptr );
            return 0L;
        }
    }

    return fptr;
}


// temporarily disable the ^C / ^BREAK / KILLPROCESS signals
void HoldSignals( void )
{
    (void)DosEnterMustComplete( &SignalNestingLevel );
}


// enable the ^C / ^BREAK / KILLPROCESS signals
void EnableSignals( void )
{
    while ( SignalNestingLevel != 0 )
        (void)DosExitMustComplete( &SignalNestingLevel );
}


// request and lock a semaphore
void RequestSemaphore( HMTX *SemHandle, char *pszSemaphoreName )
{
    if ( DosCreateMutexSem( pszSemaphoreName, SemHandle, 0L, TRUE ) == ERROR_DUPLICATE_NAME ) {
        *SemHandle = 0;
        (void)DosOpenMutexSem( pszSemaphoreName, SemHandle );

        // wait (max 10 seconds) for clear semaphore, then lock it
        (void)DosRequestMutexSem( *SemHandle, 10000 );
    }
}


// release & free the semaphore
void FreeSemaphore( HMTX SemHandle )
{
    (void)DosReleaseMutexSem( SemHandle );
    (void)DosCloseMutexSem( SemHandle );
}


// wait the specified number of seconds
void SysWait( unsigned long ulSeconds )
{
    // OS/2 counts in milliseconds
    ulSeconds *= 1000;
    (void)DosSleep( ulSeconds );
}


// beep the system speaker
int SysBeep( unsigned int frequency, unsigned int duration )
{
    // OS2 does everything in milliseconds
    duration *= 54;

    // anything less than 37Hz just sleeps
    //   instead of beeps
    if ( frequency < 37 )
        return ( DosSleep( duration ));

    return ( DosBeep( frequency, duration ));
}


// Get the current date & time
void QueryDateTime( DATETIME *sysDateTime )
{
    (void)DosGetDateTime( sysDateTime );
}


// Set the current date & time
int SetDateTime( DATETIME *sysDateTime )
{
    return ( DosSetDateTime( sysDateTime ));
}


// set the file date & time
int SetFileDateTime( char *pszFilename, int hFile, DATETIME *sysDateTime, int fField )
{
    int             rval = 0;
    FILESTATUS3     FileInfoBuf;
    unsigned long   ulAction;

    if (( hFile != 0 ) || (( rval = DosOpen( pszFilename, (PULONG)&hFile, &ulAction, 0L, 0L, OPEN_ACTION_OPEN_IF_EXISTS, OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READWRITE, 0L )) == 0 )) {
        DosQueryFileInfo( (HFILE)hFile, 1, &FileInfoBuf, sizeof(FILESTATUS3) );

        if ( fField == 0 ) {
            FileInfoBuf.fdateLastWrite.year = sysDateTime->year - 1980;
            FileInfoBuf.fdateLastWrite.month = sysDateTime->month;
            FileInfoBuf.fdateLastWrite.day = sysDateTime->day;
            FileInfoBuf.ftimeLastWrite.hours = sysDateTime->hours;
            FileInfoBuf.ftimeLastWrite.minutes = sysDateTime->minutes;
            FileInfoBuf.ftimeLastWrite.twosecs = sysDateTime->seconds;
        } else if ( fField == 1 ) {
            FileInfoBuf.fdateLastAccess.year = sysDateTime->year - 1980;
            FileInfoBuf.fdateLastAccess.month = sysDateTime->month;
            FileInfoBuf.fdateLastAccess.day = sysDateTime->day;
            FileInfoBuf.ftimeLastAccess.hours = sysDateTime->hours;
            FileInfoBuf.ftimeLastAccess.minutes = sysDateTime->minutes;
            FileInfoBuf.ftimeLastAccess.twosecs = sysDateTime->seconds;
        } else {
            FileInfoBuf.fdateCreation.year = sysDateTime->year - 1980;
            FileInfoBuf.fdateCreation.month = sysDateTime->month;
            FileInfoBuf.fdateCreation.day = sysDateTime->day;
            FileInfoBuf.ftimeCreation.hours = sysDateTime->hours;
            FileInfoBuf.ftimeCreation.minutes = sysDateTime->minutes;
            FileInfoBuf.ftimeCreation.twosecs = sysDateTime->seconds;
        }

        DosSetFileInfo( (HFILE)hFile, 1, &FileInfoBuf, sizeof(FILESTATUS3) );

        if ( pszFilename != NULL )
            DosClose( hFile );
    }

    return rval;
}


// Get the file size (-1 if file doesn't exist)
long QueryFileSize( char *fname, int fAllocated )
{
    int         fval = FIND_FIRST;
    long        lSize = 0L;
    FILESEARCH  dir;
    QDISKINFO   DiskInfo;
    char        szFileName[MAXFILENAME];

    strcpy( szFileName, fname );
    if ( fAllocated )
        QueryDiskInfo( szFileName, &DiskInfo, 0 );

    for ( ; ( find_file( fval, szFileName, 0x107, &dir, NULL ) != NULL ); fval = FIND_NEXT ) {
        if ( fAllocated ) {
            if ( dir.size > 0L )
                lSize += (unsigned long)(( dir.size + ( DiskInfo.ClusterSize - 1 )) / DiskInfo.ClusterSize ) * DiskInfo.ClusterSize;
        } else
            lSize += dir.size;
    }

    return (( fval == FIND_FIRST ) ? -1L : lSize );
}


// Get the file attributes
int QueryFileMode( char *fname, unsigned int *attrib)
{
    APIRET16 APIENTRY16 DosQFileMode( PSZ, PUSHORT, ULONG );

    // clear out the upper 16 bits when INT = 32bit
    *attrib = 0;
    return ( DosQFileMode( fname, (PUSHORT)attrib, 0L ));
}


// Get the file attributes
int SetFileMode( char *fname, unsigned int attrib)
{
    APIRET16 APIENTRY16 DosSetFileMode(PSZ, USHORT, ULONG);

    return ( DosSetFileMode( fname,attrib,0L ));
}


// check for disk write verify flag
int QueryVerifyWrite( void )
{
    ULONG VerifyFlag;

    (void)DosQueryVerify( &VerifyFlag );
    return VerifyFlag;
}


// set disk write verify flag
void SetVerifyWrite( int fVerify )
{
    (void)DosSetVerify( fVerify );
}


// get the current code page
int QueryCodePage( void )
{
    ULONG CodePageList, DataLength;
    (void)DosQueryCp( sizeof(CodePageList), &CodePageList, &DataLength );

    return (int)CodePageList;
}


#include <bseord.h>

// set the code page
int SetCodePage( int cp )
{
    typedef APIRET (APIENTRY * DSCP)(ULONG,ULONG);
    int         rc = 0;
    char        buf[128];
    HMODULE     hmod;
    DSCP        pfnDSCP = 0L;

    if (( rc = DosSetProcessCp( cp )) != 0 )
        return ( error( rc, NULL ));

    // This code won't work in Warp 4, but we keep it for compatibility
    //   w/2.x & 3.x
    if ( DosLoadModule( buf, sizeof(buf), "DOSCALLS", &hmod ) == 0 ) {
        DosQueryProcAddr( hmod, ORD_DOS32SETCP, NULL, (PFN *)&pfnDSCP );
        if ( pfnDSCP != 0L )
            pfnDSCP( cp, 0 );
        DosFreeModule( hmod );
    }

    VioSetCp( 0, cp, 0 );

    return 0;
}


// return the size of the specified memory block
unsigned long QueryMemSize( char *fptr )
{
    unsigned long size = 0x100000L;
    unsigned long flags;

    (void)DosQueryMem( fptr, &size, &flags );
    return size;
}


// get various disk info ( free space, total space, cluster size)
int QueryDiskInfo( char *drive, QDISKINFO * DiskInfo, int fNoErrors )
{
    int rval;
    FSALLOCATE FSInfoBuf;

    if (( rval = DosQueryFSInfo( gcdisk( drive ), 1, (PVOID)&FSInfoBuf, sizeof(FSInfoBuf))) != 0 )
        return (( fNoErrors ) ? rval : error( rval, drive ));

    DiskInfo->BytesTotal = (FSInfoBuf.cbSector * FSInfoBuf.cSectorUnit);
    DiskInfo->ClusterSize = DiskInfo->BytesTotal;
    DiskInfo->BytesTotal *= FSInfoBuf.cUnit;
    DiskInfo->BytesFree = (FSInfoBuf.cbSector * FSInfoBuf.cSectorUnit);
    DiskInfo->BytesFree *= FSInfoBuf.cUnitAvail;

    // kludge for TVFS bug (it returns 0 for everything!)
    if ( DiskInfo->ClusterSize == 0 )
        DiskInfo->ClusterSize = -1;

    return 0;
}


// call OS to get the volume label
char *QueryVolumeInfo( char *arg, char *volume_name, unsigned long *serial_number )
{
    int rval;
    struct {
        ULONG ulVSN;
        VOLUMELABEL vol;
    } FSInfoBuf;

    if (( arg = gcdir( arg, 0 )) == NULL )
        return NULL;

    // OS/2 may return an error if the disk isn't labeled
    if ((( rval = DosQueryFSInfo( gcdisk( arg ), 2, (PVOID)&FSInfoBuf, sizeof(FSInfoBuf))) != 0 ) && ( rval != ERROR_NO_VOLUME_LABEL )) {
        error( rval, arg );
        return NULL;
    }

    sprintf( volume_name, FMT_STR, ((( rval != 0 ) || (FSInfoBuf.vol.cch == 0 )) ? UNLABELED : FSInfoBuf.vol.szVolLabel));

    // get the volume serial number
    *serial_number = FSInfoBuf.ulVSN;

    return volume_name;
}


// Check for ANSI - return 1 if loaded, 0 if absent
int QueryIsANSI( void )
{
    USHORT usANSI = 0;

    VioGetAnsi( &usANSI, 0 );
    return usANSI;
}


// return the file system type for the specified (or default) drive
//      0 - FAT
//      1 - HPFS or other ( usually LAN)
int ifs_type( char *drive )
{
    char FSQBuffer[64], DeviceName[3];
    PSZ psz;
    unsigned int length;

    if (( drive != NULL ) && ( *drive == '"' ))
        drive++;

    // if it's a net name (\\server\sharename ) it's most likely an HPFS vol
    if (( drive != NULL ) && ( is_net_drive( drive )))
        return 1;

    // get the drive name (if no drive specified, get the default)
    sprintf( DeviceName, FMT_DISK, ( gcdisk( drive ) + 64 ));

    length = sizeof(FSQBuffer);
    if ( DosQueryFSAttach( DeviceName, 1, 1, (PVOID)FSQBuffer, (PULONG)&length ) != 0 )
        return 0;

    psz = (FSQBuffer + 9) + *((USHORT *)(FSQBuffer + 2));
    if ( psz == 0L )
        return 0;

    // LAN & unknown types drives default to HPFS type display (return 1 )
    return ( _stricmp( psz, "FAT" ) != 0 );
}


// return 1 if the specified drive is a CD-ROM
int QueryIsCDROM( char *drive )
{
    char FSQBuffer[64];
    unsigned int length;

    // file system type ("FAT", "HPFS", etc.)
    length = sizeof( FSQBuffer );

    if ( DosQueryFSAttach( drive, 1, 1, (PVOID)FSQBuffer, (PULONG)&length ) == 0 )
        return ( stricmp( "CDFS", (char *)((FSQBuffer + 9) + *((PUSHORT)(FSQBuffer + 2)))) == 0 );

    return 0;
}


// return 1 if the handle is the console, 0 otherwise
int QueryIsConsole( int handle )
{
    ULONG HandType, FlagWord;

    (void)DosQueryHType( handle, &HandType, &FlagWord );

    // is it a character device?
    return (( HandType & 0x01 ) && ( FlagWord & 0x02 ));
}


// read the .EXE header to see what type the application is (0 - 3)
int app_type( char *fname )
{
    USHORT *iptr, i;
    int rval;
    long new_exe_offset;
    char buffer[64], *ptr;
#pragma pack(1)
    NEW_EXE ne;
#pragma pack()
    unsigned long appType;

    // if it's a .BAT file, return it as a DOS program
    if ((( ptr = ext_part( fname )) != NULL ) && ( stricmp( ptr, BAT ) == 0 ))
        return 0x20;

    if (( rval = DosQueryAppType( fname, &appType )) != 0 )
        return -rval;

    if ( appType == 0 ) {
        // if type is unknown, read the .EXE header to see if it's a PM app
        if (( gnGFH = _sopen( fname, (O_RDONLY | O_BINARY), SH_DENYNO )) >= 0 ) {
            // read old .EXE header
            (void)_read( gnGFH, buffer, 64 );

            // check signature to make sure it's really an .EXE file
            if (( buffer[0] == 'M' ) && ( buffer[1] == 'Z' )) {
                // seek to new .EXE header
                new_exe_offset = _lseek( gnGFH, *((long *)(buffer + 0x3C)), SEEK_SET );
                (void)_read( gnGFH, &ne, sizeof(ne) );

                // seek to module reference table
                _lseek( gnGFH, (new_exe_offset + ne.modtab), SEEK_SET );
                iptr = (USHORT *)_alloca(ne.cmod * 2);
                (void)_read( gnGFH, ( char *)iptr, (ne.cmod * 2) );

                // scan the imported names table looking for PMWIN
                for ( i = 0; ( i < ne.cmod ); i++, iptr++ ) {
                    // read another imported name
                    _lseek( gnGFH, (new_exe_offset + ne.imptab + *iptr ), SEEK_SET );
                    (void)_read( gnGFH, buffer, 64 );

                    // if app calls PMWIN.DLL, it's a PM app
                    if ( strnicmp( buffer, "\005PMWIN", 6 ) == 0 ) {
                        appType = WINDOWAPI;
                        break;
                    }
                }
            }
            gnGFH = _close( gnGFH );
        }
    }

    // remove all but NOTSPECIFIED, NOTWINDOWCOMPAT, WINDOWCOMPAT,
    //   & WINDOWAPI (& DOS/WINDOWS flags if OS/2 2.0+)
    return ( appType & 0x1623 );
}


// return various global OS/2 2.0 info
unsigned int QuerySysInfo( int subject )
{
    unsigned int info = 0;

    (void)DosQuerySysInfo( subject, subject, (PVOID)&info, sizeof(info) );

    return info;
}


// check to see if the specified name is a named pipe
int QueryIsPipeName( char *pipename )
{
    // skip server name ("\\server\pipe\...")
    if (( pipename[0] == '\\' ) && ( pipename[1] == '\\' )) {
        if (( pipename = strchr( pipename+2, '\\' )) == NULL )
                pipename = NULLSTR;
    }

    // if "\pipe\..." & no wildcards, assume named pipe exists
    return (( strnicmp( pipename, "\\PIPE\\", 6 ) == 0 ) && ( strpbrk( pipename, WILD_CHARS ) == NULL ));
}


// check to see if the specified handle is connected to a pipe
int QueryIsPipeHandle(int handle )
{
    ULONG HandType = 0, FlagWord;

    (void)DosQueryHType( handle, &HandType, &FlagWord );
    return (HandType == 2);
}


// OS/2 check if "fname" is a character device
int QueryIsDevice( char *fname )
{
    int     rval;
    char    szFullName[MAXFILENAME], szDeviceName[MAXFILENAME];

    // check for CLIP: pseudo-device
    if ( stricmp( fname, CLIP ) == 0 )
        return 1;

    // ignore drive names & strip path & trailing colon from device name
    copy_filename( szDeviceName, fname );
    if ( strlen( szDeviceName ) > 2)
        strip_trailing( szDeviceName, ":" );

    rval = DosQueryPathInfo( szDeviceName, FIL_QUERYFULLNAME, szFullName, MAXFILENAME-1 );
    if (( rval = (( rval == 0 ) && ( strnicmp( szFullName, "\\dev\\", 5 ) == 0 ))) == 1 )
        strcpy( fname, szFullName );

    return rval;
}


// returns 1 if the specified drive exists, 0 otherwise
int QueryDriveExists( int drive )
{
    ULONG   DriveMap;
    ULONG   DriveNumber = 0;

    (void)DosQueryCurrentDisk( &DriveNumber, &DriveMap );

    // change drive number to 0-based
    drive--;

    // OS/2 returns drive mapping in 0 to 25 bits in DriveMap
    return ((( DriveMap >>= drive ) & 1 ) == 1 );
}


// returns 1 if the specified drive is remote, 0 if local or it doesn't exist
int QueryDriveRemote( int drive )
{
    int     rval;
    char    FSQBuffer[64], DeviceName[3];
    int     length;

    // get the drive name
    sprintf( DeviceName, FMT_DISK, drive+64 );

    // turn off VIOPOPUP error window
    (void)DosError( 2 );

    length = sizeof( FSQBuffer );
    rval = DosQueryFSAttach( DeviceName, 1, 1, (PVOID)FSQBuffer, (PULONG)&length );

    // turn VIOPOPUP error window back on
    (void)DosError( 1 );

    // if drive is remote, iType == 4
    return (( rval == 0 ) && ( FSQBuffer[0] == 4 ));
}


// returns 1 if the specified drive is ready, 0 if it isn't
int QueryDriveReady( int drive )
{
    char curdir[MAXPATH+1];
    unsigned int maxpath = MAXPATH;
    int rval = MAXPATH;

    // turn off VIOPOPUP error window
    (void)DosError( 2 );

    rval = ( DosQueryCurrentDir( (ULONG)drive, curdir, (PULONG)&maxpath ) == 0 );

    // turn VIOPOPUP error window back on
    (void)DosError( 1 );

    return rval;
}


// returns 1 if the specified drive is ready, 0 if it isn't
int QueryDriveRemovable( int drive )
{
    int rc;
    char Disk[4];
    ULONG Action, DataLength;
    HFILE Drive_Handle;
    BYTE Drive_Data;

    sprintf( Disk, FMT_DISK, drive+64 );

    // turn off VIOPOPUP error window
    (void)DosError( 2 );

    rc = DosOpen( Disk, &Drive_Handle, &Action, 0L, FILE_NORMAL, FILE_OPEN,
                  OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_DASD, 0L );

    // turn VIOPOPUP error window back on
    (void)DosError( 1 );

    // ERROR_NOT_READY indicates that the drive is removable, but there is   :
    //   currently no disk inserted.
    if ( rc == ERROR_NOT_READY )
        return 1;

    // ERROR_DISK_CHANGE means you switched to B: in a one-drive system
    if (( rc != NO_ERROR ) && ( rc != ERROR_DISK_CHANGE ))
        return 0;

    DataLength = sizeof( Drive_Data);
    (void)DosDevIOCtl( Drive_Handle, 8, 0x0020, NULL, 0L, 0L, &Drive_Data, DataLength, &DataLength );
    (void)DosClose( Drive_Handle );

    // if Drive_Data = 0, it's removable; if 1, it's fixed
    return ( Drive_Data == 0 );
}


// check if it's a +, -, or 0-9
int is_signed_digit( int c )
{
    return ( isdigit( c ) || ( c == '+' ) || ( c == '-' ));
}


// check if it's a 0-9
int is_unsigned_digit( int c )
{
    return ( isdigit( c ));
}


// get the country code ( for date formatting)
void QueryCountryInfo( void )
{
    unsigned long info_len, numCP, CodePages[4];

    (void)DosQueryCp( sizeof(CodePages), CodePages, &numCP );

    gaCountryCode.country = 0;
    gaCountryCode.codepage = CodePages[0];

    (void)DosQueryCtryInfo( sizeof(gaCountryInfo), &gaCountryCode, &gaCountryInfo, &info_len );

    gaCountryCode.country = gaCountryInfo.country;
    gaCountryCode.codepage = gaCountryInfo.codepage;

    // set am/pm info (if TimeFmt==0, use default country info)
    if ( gpIniptr->TimeFmt == 1 )   // 12-hour format
        gaCountryInfo.fsTimeFmt = 0;
    else if ( gpIniptr->TimeFmt == 2)       // 24-hour format
        gaCountryInfo.fsTimeFmt = 1;
    else
        gaCountryInfo.fsTimeFmt &= 1;

    if ( gpIniptr->DecimalChar )
        gaCountryInfo.szDecimal[0] = ((gpIniptr->DecimalChar == 1) ? '.' : ',');
    if ( gpIniptr->ThousandsChar )
        gaCountryInfo.szThousandsSeparator[0] = ((gpIniptr->ThousandsChar == 1) ? '.' : ',');

    // make sure the OS version string has the right characters
    SetOSVersion();
}


void SetOSVersion( void )
{
    // set gszOsVersion, with kludge for Warp returning "20.30"
    if (( _osmajor == 20 ) && ( _osminor >= 30 )) {
        if ( _osminor >= 40 )
            sprintf( gszOsVersion, "4%c%02u", gaCountryInfo.szDecimal[0], ( _osminor - 40 ) * 10 );
        else
            sprintf( gszOsVersion, "3%c%02u", gaCountryInfo.szDecimal[0], ( _osminor - 30 ));
    } else
        sprintf( gszOsVersion, "%u%c%02u", _osmajor/10, gaCountryInfo.szDecimal[0], _osminor );

    // set PROGRAM and gszMyVersion
    sprintf( PROGRAM, SET_PROGRAM, VER_MAJOR, gaCountryInfo.szDecimal[0], VER_MINOR, VER_REVISION );
    sprintf( gszMyVersion, "%u%c%02u", VER_MAJOR, gaCountryInfo.szDecimal[0], VER_MINOR );

    // Set numeric version
    gnOsVersion = ( _osmajor * 10 ) + _osminor;
}


// map character to upper case (including foreign language chars)
int _ctoupper( int c )
{
    if ( c < 'a' )
        return c;

    if (( c >= 'a' ) && ( c <= 'z' ))
        return ( c - 32);

    if (( c >= 0x80 ) && ( c <= 0xFF )) {
        (void)DosMapCase( 1, &gaCountryCode, (PCHAR)&c );
    }

    return c;
}


// map string to upper case (including foreign language chars)
char * strupr( char *source )
{
    (void)DosMapCase( strlen( source ), &gaCountryCode, source );

    return source;
}


// replace RTL chdir (which doesn't set _doserrno properly)
int chdir( char *pathname )
{
    return ((( _doserrno = DosSetCurrentDir( pathname )) != 0 ) ? -1 : 0 );
}


// replace RTL mkdir (which doesn't set _doserrno properly)
int mkdir( char *pathname )
{
    return ((( _doserrno = DosCreateDir( pathname, 0 )) != 0 ) ? -1 : 0 );
}


// replace RTL remove (which doesn't set _doserrno properly)
int remove( const char *filename )
{
    return ((( _doserrno = DosDelete( ( char *)filename )) != 0 ) ? -1 : 0 );
}


// replace RTL rmdir (which doesn't set _doserrno properly)
int rmdir( char *pathname )
{
    return ((( _doserrno = DosDeleteDir( pathname )) != 0 ) ? -1 : 0 );
}


// rename a file (the C Set/2 RTL rename() has a bug when renaming to
//   different case on an HPFS volume)
int rename( const char *source, const char *target )
{
    return ((( _doserrno = DosMove(( char *)source,( char *)target)) != 0 ) ? -1 : 0 );
}


// create a unique filename in the specified drive & path
int UniqueFileName( char *szFname )
{
    int fd, i, nPathLen;
    int rval = 0;
    char buffer[MAXFILENAME+1];
    DATETIME sysDateTime;

    // kludge for OS/2 bug (file gets created from previous
    //   invocation, but OS/2 doesn't know it's there yet!)
    DosSleep( 25 );

    mkdirname( strcpy( buffer, szFname ), NULLSTR );
    nPathLen = strlen( buffer );

    QueryDateTime( &sysDateTime );

    for ( i = sysDateTime.hundredths; ( i <= 0xFFF ); i++ ) {
        // create a temporary filename using the date & time
        sprintf( buffer+nPathLen, "%02x%02x%02x%02x.%03x", sysDateTime.day, sysDateTime.hours, sysDateTime.minutes, sysDateTime.seconds, i );

        // try to create the temporary file
        errno = 0;
        if ((( fd = _sopen( buffer, (O_BINARY | O_CREAT | O_EXCL | O_WRONLY),SH_DENYRW, (S_IREAD | S_IWRITE))) >= 0 ) || ((errno != EEXIST) && (errno != EACCES))) {
            rval = ERROR_4DOS_CANT_OPEN;
            break;
        }
    }

    if ( fd >= 0 ) {
        _close( fd );
        if (errno == 0 ) {
            strcpy( szFname, buffer );
            return 0;
        }
    }
    return rval;
}


// make a short filename from a long filename
char *MakeSFN( char *pszInputName, char *pszOutputName )
{
    static char     szLongName[MAXFILENAME];
    int             i;
    char            *arg;
    char            szName[20], szExt[5];
    unsigned int    n, uBufferSize;

    szLongName[0] = '\0';

    if ( ifs_type( pszInputName ) == FAT ) {

        // if copying from FAT -> HPFS, check EA for .LONGNAME
        if ( ifs_type( pszOutputName ) != FAT ) {
            // if copying from FAT to HPFS, check for longname
            uBufferSize = MAXFILENAME;
            if (( EAReadASCII( pszInputName, LONGNAME_EA, szLongName, (PINT)&uBufferSize ) != FALSE ) && ( szLongName[0] ))
                insert_path( pszOutputName, szLongName, pszOutputName );
        }

    } else if ( ifs_type( pszOutputName ) == FAT ) {
        // if copying from HPFS -> FAT, save name in .LONGNAME EA
        strcpy( szLongName, fname_part( pszOutputName ) );

        // truncate / rename existing name (if necessary)
        sscanf( szLongName, "%8[^.]", szName );
        szExt[0] = '\0';
        if (( arg = ext_part( szLongName )) != NULL )
            sscanf( arg, "%4[^\n]", szExt );
        strcat( szName, szExt );

        // replace invalid filename chars
        while (( arg = strpbrk( szName, " \t,=<>|;+" )) != NULL )
            *arg = '_';

        if ( stricmp( szLongName, szName ) != 0 ) {
            // new name is different, so save it!

            // check for new name conflict
            for ( i = 0; ( i <= 9999 ); i++ ) {

                insert_path( pszOutputName, szName, pszOutputName );
                if ( is_file( pszOutputName ) == 0 )
                    break;
                if (( arg = strchr( szName, '.' )) == NULL )
                    arg = strend( szName );

                // if name + suffix > max name size, shorten the name
                n = sprintf( szExt, FMT_INT, i );
                if (( n + (int)( arg - szName )) > 8 ) {
                    strcpy( arg - n, arg );
                    arg -= n;
                }

                // insert new suffix
                strins( arg, szExt );
            }

        } else
            szLongName[0] = '\0';
    }
    return szLongName;
}


// Find the first or next matching file
//      fflag   Find First or Find Next flag
//      arg     filename to search for
//      attrib  search attribute to use
//                 0x100 - don't display any error messages
//                 0x200 - only return directories
//                 0x400 - check inclusive/exclusive match
//                 0x800 - check date / time range
//              type of search handle to use
//                 0x1000 - (FIND_CREATE) - HDIR_CREATE
//                 anything else - HDIR_SYSTEM
//                 0x4000 - don't do case conversion
//                 0x8000 - don't return "." or ".."
//      dir     pointer to directory structure
//      filename  pointer to place to return filename
char * find_file( int fflag, char *arg, unsigned int attrib, FILESEARCH *dir, char *filename )
{
    int i;
    char fname[MAXFILENAME], *name_part, *next_name, *ptr;
    int fval = fflag, rval, mode, fWildBrackets = 0;
    unsigned int uTime;
    unsigned long srch_cnt = 1;
    unsigned long ulDTRange;

next_list:
    // Check for include list by looking for ; and extracting first name
    i = ((( ptr = path_part( arg )) != NULL ) ? strlen( ptr ) + (int)( *arg == '"' ) : 0 );
    name_part = arg + i;

    if (( ptr = scan( arg+i, ";", "\"[" )) == BADQUOTES )
        return NULL;
    if (( i = (int)(ptr - arg)) > MAXFILENAME - 1 )
        i = MAXFILENAME - 1;
    sprintf( fname, FMT_PREC_STR, i, arg );

    if ( arg[i] )
        i++;

    // point "next_name" to next name in include list (or EOS)
    next_name = arg + i;

    // if not a FIND_FIRST, set the flag to treat brackets as wildcards
    if (( strchr( fname, '[' ) != NULL ) && ( fval == FIND_NEXT ))
        fWildBrackets = 1;

    // check for HPFS long filenames delineated by double quotes
    //   and strip leading / trailing " (need to do it here as
    //   well as in mkfname() for things like IF EXIST "file 1")
    StripQuotes( fname );

    for ( mode = ( attrib & 0xFF ); ( srch_cnt != 0 ); ) {
        // search for the next matching file
        if ( fval == FIND_FIRST ) {
RetryFindFirst:
            // create directory search handle
            dir->hdir = (( attrib & FIND_CREATE ) ? HDIR_CREATE : HDIR_SYSTEM );

            // set IFS type flag for upper/lower case setting
            dir->fHPFS = ifs_type( arg );

            rval = DosFindFirst( fname, &(dir->hdir), mode, dir, sizeof(FILEFINDBUF4), &srch_cnt, FIL_QUERYEASIZE );
        } else if ( strpbrk( fname, WILD_CHARS ) == NULL )
            rval = ERROR_FILE_NOT_FOUND;
        else
            rval = DosFindNext( dir->hdir, (PVOID)dir, sizeof(FILEFINDBUF4), &srch_cnt );

        if ( rval ) {
            // couldn't find file; if FIND_FIRST, display error
            if ( fval == FIND_FIRST ) {
                // 4OS2 handles UNIX-style "[a-c]" syntax which
                //   neither CMD.EXE nor OS/2 recognize
                for ( i = 0, ptr = fname; (( ptr = strchr( ptr, '[' )) != NULL ); ) {
                    // replace [] with a single '?'
                    *ptr++ = '?';
                    while ( *ptr != '\0' ) {
                        i = *ptr;
                        strcpy( ptr, ptr+1 );
                        if ( i == ']' )
                            break;
                    }
                }

                if ( i == ']' ) {
                    if (( dir->hdir != HDIR_CREATE ) && ( dir->hdir != HDIR_SYSTEM ))
                        DosFindClose( dir->hdir );
                    fWildBrackets = 1;
                    goto RetryFindFirst;
                }
            }

            if (( fflag == FIND_FIRST ) || (( rval != ERROR_FILE_NOT_FOUND ) && ( rval != ERROR_NO_MORE_FILES ))) {
                if (( attrib & 0x100 ) == 0 ) {
                    // kludge to rebuild original name;
                    //   otherwise *WOK.WOK would display
                    //   as *.WOK
                    if (( i = (int)( scan( arg, ";","\"[" ) - arg )) > MAXFILENAME - 1 )
                        i = MAXFILENAME - 1;
                    sprintf( fname, FMT_PREC_STR, i, arg );
                    error( rval, fname );
                }
            }

            // close search handle
            if ( attrib & FIND_CREATE )
                DosFindClose( dir->hdir );

            // check include list for another argument
            if ( *next_name ) {
                // remove the previous name & get next
                //   one from include list
                strcpy( name_part, next_name );

                fval = FIND_FIRST;
                srch_cnt = 1;
                goto next_list;
            }
            return NULL;
        }

        fval = FIND_NEXT;

        // skip "." and ".."?
        if (( attrib & FIND_NO_DOTNAMES ) && ( QueryIsDotName( dir->name )))
            continue;

        // only retrieving directories? (ignore "." and "..")
        if (( attrib & 0x200 ) && ((( dir->attrib & _A_SUBDIR ) == 0 ) || ( QueryIsDotName( dir->name ) != 0 )))
            continue;

        // kludge for OS/2 bug returning directories instead of files
        if ((( attrib & _A_SUBDIR) == 0 ) && ( dir->attrib & _A_SUBDIR ))
            continue;

        // check for inclusive / exclusive matches
        if ( attrib & 0x400 ) {
            // retrieve files with specified attributes?
            if (( gchInclusiveMode & dir->attrib ) != gchInclusiveMode )
                continue;

            // don't retrieve files with specified attributes?
            if (( gchExclusiveMode & dir->attrib ) != 0 )
                continue;
        }

        // check for date / time ranges
        if ( attrib & 0x800 ) {
            if ( dir->aRanges.DateType == 0 )
                ulDTRange = ((long)( dir->fdLW.fdLWrite ) << 16) + dir->ftLW.ftLWrite;
            else if ( dir->aRanges.DateType == 1 )
                ulDTRange = ((long)( dir->fdLA.fdLAccess) << 16) + dir->ftLA.ftLAccess;
            else
                ulDTRange = ((long)( dir->fdC.fdCreation) << 16) + dir->ftC.ftCreation;

            if (( ulDTRange < dir->aRanges.DTRS.DTStart) || ( ulDTRange > dir->aRanges.DTRE.DTEnd))
                continue;

            if ( dir->aRanges.TimeType == 0 )
                uTime = dir->ftLW.ftLWrite;
            else if ( dir->aRanges.TimeType == 1 )
                uTime = dir->ftLA.ftLAccess;
            else
                uTime = dir->ftC.ftCreation;

            uTime &= 0xFFE0;

            // if start > end, we wrapped around midnight
            if ( dir->aRanges.TimeEnd < dir->aRanges.TimeStart ) {
                if (( uTime < dir->aRanges.TimeStart ) && ( uTime > dir->aRanges.TimeEnd ))
                    continue;

            } else if (( uTime < dir->aRanges.TimeStart ) || ( uTime > dir->aRanges.TimeEnd))
                continue;

            if (( dir->size < dir->aRanges.SizeMin ) || ( dir->size > dir->aRanges.SizeMax))
                continue;

            // check for filename exclusions
            if (( dir->aRanges.pszExclude != NULL ) && ( ExcludeFiles( dir->aRanges.pszExclude, dir->name ) == 0 ))
                continue;
        }

        // check unusual wildcard matches
        if (( stricmp( name_part, dir->name ) == 0 ) || ( wild_cmp( name_part, dir->name, TRUE, fWildBrackets ) == 0 ))
            break;
    }

    // if no target filename requested, just return a non-NULL response
    if ( filename == NULL ) {
        // if no case conversion wanted, just return
        if (( attrib & 0x4000 ) == 0 ) {
            if (( gpIniptr->Upper == 0 ) && ( dir->fHPFS == 0 ) && (( attrib & _A_VOLID) == 0 ))
                strlwr( dir->name );
        }
        return ( char *)-1;
    }

    // copy the saved source path
    insert_path( filename, dir->name, fname );

    // return matching filename shifted to upper or lower case
    if ( dir->fHPFS )
        return filename;

    return (( gpIniptr->Upper == 0 ) ? strlwr( filename ) : strupr( filename ));
}


// change the name in the task list
void update_task_list( char *pszWindowTitle )
{
    SWCNTRL swctl;
    HSWITCH hswitch;

    // if started with a /C, don't bother changing the title/icon
    if (( gnTransient ) || ( pfnWSTAI == 0L ))
        return;

    szWindowTitle[0] = '\0';

    // WinQuerySwitchHandle
    if (( hswitch = (*pfnWQSH)( 0, gpLIS->pidCurrent )) != NULLHANDLE ) {
        swctl.szSwtitle[0] = '\0';

        // WinQuerySwitchEntry
        (void)(*pfnWQSE)( hswitch, &swctl );

        // if name is NULL, reset to the original title
        if ( pszWindowTitle == NULL ) {
            if ( gszSessionTitle[0] == '\0' ) {
                if (( swctl.szSwtitle[0] == '\0' ) || ( stricmp( fname_part( swctl.szSwtitle ), OS2_NAME ) == 0 )) {
                    // if title is 4OS2.EXE, change to session type
                    //   ("4OS2 Full Screen" or "4OS2 Window")
                    strcpy( swctl.szSwtitle, (( gpLIS->typeProcess == PT_FULLSCREEN ) ? OS2_FS : OS2_WIN ));
                }
                sprintf( gszSessionTitle, FMT_PREC_STR, 60, swctl.szSwtitle );
            } else {
                // check for a title change by WINDOW in batch file
                if (( cv.bn >= 0 ) && ( bframe[cv.bn].pszTitle != NULL ))
                    sprintf( swctl.szSwtitle, FMT_PREC_STR, 60, bframe[cv.bn].pszTitle );
                else
                    strcpy( swctl.szSwtitle, gszSessionTitle );
            }
        } else {
            // add title, less path
            sprintf( swctl.szSwtitle, FMT_PREC_STR, 60, fname_part( pszWindowTitle ));
            // kludge to ensure batch title is in upper case
            if (( cv.bn >= 0 ) && ( stricmp( pszWindowTitle, bframe[cv.bn].pszBatchName ) == 0 ))
                strupr( swctl.szSwtitle );
        }

        // WinChangeSwitchEntry
        (void)(*pfnWCSE)( hswitch, &swctl );

        // WinSetTitleAndIcon

        // Warp is different!  (Buggy?)  It requires the icon file
        //   name; else it clears the icon altogether!
        //   Since we don't have an icon filename for externals, for
        //   now we'll just set the icon to the 4os2 icon
        if ( gnOsVersion >= 230 && gnOsVersion < 245 ) {
            pfnWSTAI((PSZ)( swctl.szSwtitle ), (( gpLIS->typeProcess == PT_WINDOWABLEVIO ) ? "4os2w.ico" : "4os2f.ico" ));
        } else {
            pfnWSTAI((PSZ)( swctl.szSwtitle ), (PSZ)(( pszWindowTitle == NULL ) ? _pgmptr : pszWindowTitle ));
        }

        // WinSetWindowText (WinSetTitleAndIcon doesn't always work!)
        strcpy( szWindowTitle, swctl.szSwtitle );
        pfnWSWT( ghwndWindowHandle, swctl.szSwtitle );
    }
}


// configure a VDM with the "DosSetting" environment variables
char * GetDOSSettings( void )
{
    char *feptr, *lpEnvironment = 0L, *lpEnvPtr;

    for ( feptr = glpEnvironment; ( *feptr != '\0' ); feptr = next_env( feptr ) ) {
        if ( strnicmp( feptr, "DosSetting.", 11 ) == 0 ) {
            if ( lpEnvironment == 0L ) {
                (void)DosAllocMem( (PVOID)&lpEnvironment, 4096, (PAG_COMMIT | PAG_READ | PAG_WRITE));
                memset( lpEnvironment, '\0', 4096 );
                lpEnvPtr = lpEnvironment;
            }

            strcpy( lpEnvPtr, feptr + 11 );
            lpEnvPtr += strlen( lpEnvPtr ) + 1;
        }
    }

    return lpEnvironment;
}


// call DosStartSession; optionally waiting for child session to end
int start_session( STARTDATA *stdata, char *program_name )
{
    int rval = 0, fRestoreWindow = 0;
    unsigned long idsession, DataLength = 0;
    char achFailName[MAXFILENAME];
    REQUESTDATA Request;
    PVOID DataAddress = 0L;
    STATUSDATA StatusData;
    SWP swp;
    PID pid;

    stdata->Length = sizeof(STARTDATA);
    stdata->PgmHandle = 0L;
    stdata->TraceOpt = 0;
    stdata->Reserved = 0;
    stdata->ObjectBuffer = achFailName;
    stdata->ObjectBuffLen = sizeof( achFailName ) - 1;
    achFailName[0] = '\0';

    // get DOS settings from the environment (DosSetting.xxx=yyy)
    if (( stdata->SessionType & 4 ) && ( stdata->Environment == 0L ))
        stdata->Environment = GetDOSSettings();

    // if related, point to termination queue ( created in init_env())
    if ( stdata->Related ) {
        stdata->TermQ = gszQueueName;
        (void)DosPurgeQueue( ghQueueHandle );
    } else
        stdata->TermQ = (PBYTE)0L;

    rval = DosStartSession( stdata, &idsession, &pid );

    if (( rval != 0 ) && ( rval != ERROR_SMG_START_IN_BACKGROUND ))
        return ( error( rval, (( achFailName[0]) ? achFailName : program_name )));

    if ( stdata->Related ) {
        // bond 4OS2 / TCMD & the child session
        StatusData.Length = sizeof( StatusData );
        StatusData.SelectInd = 0;
        StatusData.BondInd = 1;
        (void)DosSetSession( idsession, &StatusData );

        // trap ^C's here so we can clean up child session
        if ( setjmp( cv.env ) == -1 ) {
            (void)DosStopSession( 1, idsession );
            rval = CTRLC;

        } else {
            // kludge for odd OS/2 32-bit bug with termQ wait
            (void)DosSleep( 200 );

            // change icon label to program name
            if ( program_name[0] )
                update_task_list( program_name );

            // Minimize our own window while the child is running
            if (( ghwndWindowHandle != 0 ) && ( pfnWQWP != NULL ) && ((*pfnWQWP)( ghwndWindowHandle, &swp ))) {
                if ( pfnWPM != NULL )
                    (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_MINIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE) );
                fRestoreWindow = 1;
            }

            // wait for program to end
            (void)DosReadQueue( ghQueueHandle, &Request, &DataLength, &DataAddress, 0, 0, (PBYTE)&pid, 0 );

            // free the results pointer
            if ( DataAddress != 0L ) {
                rval = ((SHORT *)DataAddress)[1];
                (void)DosFreeMem( DataAddress );
            }
        }

        // free memory allocated for DOS session settings
        if (( stdata->SessionType & 4 ) && ( stdata->Environment != 0L ))
            (void)DosFreeMem( stdata->Environment );

        // disable ^C / ^BREAK handling momentarily
        HoldSignals();

        // Put our window back in its original state (maximized
        //   or restored)
        if (( fRestoreWindow != 0 ) && (( swp.fl & SWP_MINIMIZE ) == 0 )) {
            if ( pfnWPM != NULL )
                (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT((( swp.fl & SWP_MAXIMIZE) == 0 ) ? SC_RESTORE : SC_MAXIMIZE ), MPFROM2SHORT(CMDSRC_MENU, TRUE) );
        }
    }

    return rval;
}

