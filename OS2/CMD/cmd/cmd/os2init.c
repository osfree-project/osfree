// OS2INIT - initialize 4OS2
//   (c) 1993 - 1998  Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <process.h>

#define INCL_ORDINALS
#include "4all.h"

static void RemoveQuotes( char * );
static void init_env( void );
static void InitSystemVariables( void );
static void init_ini( void );
static int  process_ini(char *);
static void default_ini( void );
static void set_window( void );


HAB ghHAB;

static int ini_done = 0;                // finished .INI file flag
int nSignalHandler = 2;                 // 0=default, 1=ignore, 2=install handler
int gnPipeBN = -1;

static PCH shell_sharedmem = 0L;        // shared memory area for this shell

PIPE_INHERIT *gpPipeParent;


void InitOS( int argc, char **argv )
{
    char *arg;
    ULONG ulTimes = 0L;
    char *errmsg, *pszOpts;

    (void)_setmode( STDIN, O_TEXT );
    (void)_setmode( STDOUT, O_TEXT );
    (void)_setmode( STDERR, O_TEXT );

    // initialize the critical variables
    memset( &cv, '\0', sizeof(cv) );
    cv.bn = -1;

    QueryCountryInfo();
    InitSystemVariables();          // initialize 4OS2 global variables

    (void)VioSetAnsi( ANSI_ON, 0 ); // enable ANSI escape sequences

    SetCurSize( 0 );                // set the default cursor shape

    // get shell #, initialize INI data, process any inherited INI data
    init_ini();

    // support the CMD.EXE SHELL syntax for specifying a COMSPEC
    //   directory - if first arg on line is a directory, it must
    //   be a COMSPEC request
    if (( argv[1] != NULL ) && ( argv[1][0] != gpIniptr->SwChr ) && ( is_dir( argv[1] ))) {
        strcpy( gszCmdline, strupr(*(++argv) ));
        mkdirname( gszCmdline, OS2_NAME );
    } else
        strcpy( (char *)gszCmdline, _pgmptr );

    // save _pgmptr (its location in the environment will be overwritten)
    _pgmptr = (char *)strdup( gszCmdline );

    // get program name (for session title)
    OS2_NAME = strdup( fname_part( gszCmdline ));

    // get title; if none, change to "4OS2 Full Screen" or "4OS2 Window"
    gszSessionTitle[0] = '\0';
    update_task_list( NULL );

    if ( setjmp( cv.env ) == -1 )
        _exit( ERROR_EXIT );

    RemoveQuotes( pszCmdLineOpts );
    for ( argc = 0; (( arg = ntharg( pszCmdLineOpts, argc )) != NULL ); argc++) {
next_start_arg:
        // check for .INI filename
        if ( *arg == '@' ) {
            // parse the user's file - if not an .INI file, must
            //  be something like "@COPY ..."
            if ( process_ini( arg ))
                break;
        } else if (( *arg == gpIniptr->SwChr ) || ( *arg == '-' )) {
            // kludge to detect "//inistuff" because ntharg()
            //  only returns the first "/"
            switch ( _ctoupper( gpNthptr[1] )) {
                case '/':       // INI file line
                    // skip leading '/'
                    argc++;
                    arg = gpNthptr;

                    // be sure default INI file has been processed
                    //   so // overrides
                    default_ini();

                    // copy so IniLine can add an extra null
                    strcpy( gszCmdline, first_arg( arg+2 ));
                    if ( IniLine( gszCmdline, &gaInifile, 0, 0, 0, &errmsg ))
                            error( ERROR_4DOS_BAD_DIRECTIVE, arg );
                    break;

                case 'C':       // transient load
                    gnTransient = 1;

                    // kludge for people who do "/Ccommand"
                    if ( arg[2] ) {
                        arg += 2;
                        gpNthptr += 2;
                        goto next_start_arg;
                    }

                    // check for a "4OS2 /C 4OS2" & turn it into
                    //   a "4OS2"
                    if (( errmsg = ntharg( pszCmdLineOpts, argc+1 )) != NULL )
                        errmsg = fname_part( errmsg);

                    if (( errmsg != NULL ) && ( cv.bn < 0 ) && ( stricmp( errmsg,OS2_NAME) == 0 )) {
                        gnTransient = 0;
                        argc++;
                    }

                    break;

            case 'K':       // permanent load in OS/2
                    gpIniptr->ShellLevel = 0;

                    // OS/2 passes STARTUP.CMD as "/kSTARTUP.CMD"
                    if ( isalnum( arg[2] )) {
                        arg += 2;
                        gpNthptr += 2;
                        goto next_start_arg;
                    }

                    break;

            case 'L':       // set local alias & history lists
                // be sure default INI file has been processed
                //   so /L overrides
                arg = gpNthptr;
                default_ini();

                switch ( _ctoupper( arg[2] )) {
                    case 'A':
                        gpIniptr->LocalAliases = 1;
                        break;
                    case 'D':
                        gpIniptr->LocalDirHistory = 1;
                        break;
                    case 'H':
                        gpIniptr->LocalHistory = 1;
                        break;
                    default:
                        gpIniptr->LocalAliases = 1;
                        gpIniptr->LocalHistory = 1;
                        gpIniptr->LocalDirHistory = 1;
                }

                break;

            case 'Q':       // for compatibility w/CMD.EXE
                break;

            case 'S':
                // don't set up a ^C / ^BREAK handler; they
                //   will be ignored
                nSignalHandler = 1;
                break;

            default:
                // kludge for "4OS2 -c ..."
                if ( *arg == '-' )
                    goto args_done;
                error( ERROR_INVALID_PARAMETER, arg );
            }
        } else
                break;
    }

args_done:
    pszOpts = gpNthptr;

    // process the default INI file if necessary
    default_ini();

    // set the default cursor shape (must be after INI file is processed)
    SetCurSize(0 );

    // display the passed command line if requested
    if (gpIniptr->INIDebug & INI_DB_SHOWTAIL) {
        qprintf( STDERR, TAILIS, NAME_HEADER, getcmd( gszCmdline ));
        (void)GetKeystroke( EDIT_NO_ECHO | EDIT_ECHO_CRLF );
    }

    // duplicate CMD.EXE IF bug
    if ( gpIniptr->DupBugs ) {
        // set IF processing
        commands[ findcmd( "IF", 1 )].pflag |= CMD_DETACH_LINE;
    }

    // get the international format chars
    QueryCountryInfo();

    // init environment, alias & history lists
    init_env();

    // set up ^C and ^BREAK handling
    HoldSignals();

    if (nSignalHandler == 2) {
        // set signal handler (structure must be on main() stack)
        pExceptionStruct->prev_structure = 0;
        pExceptionStruct->ExceptionHandler = (_ERR *)&BreakHandler;
        (void)DosSetExceptionHandler( pExceptionStruct );
        (void)DosSetSignalExceptionFocus( SIG_SETFOCUS, &ulTimes );
    }

    // if non-transient, display signon message
    if ( gnTransient == 0 ) {
        // Set the window position if requested
        set_window();

        // display copyright
        DisplayCopyright();
    }

    // get startup disk for 4START and 4EXIT
    AUTOSTART[0] = AUTOEXIT[0] = gpIniptr->BootDrive;

    // execute 4START.BTM / 4START.CMD
    find_4files( AUTOSTART );

    // execute remainder of command line
    if ( pszOpts != NULL ) {
        pszOpts = skipspace( pszOpts );
        if ( *pszOpts ) {
            RemoveQuotes( pszOpts );
            command( pszOpts, ( gnTransient == 0 ));
        }
    }
}


// support CMD.EXE syntax:  CMD /C ""a" b c" (with exception
//   for embedded whitespace: START /pgm "t start.cmd")
static void RemoveQuotes( char *line )
{
    int nLength;

    nLength = ( strlen( line ) - 1 );
    if (( line[0] == '"' ) && ( line[ nLength ] == '"' ) && ( is_file( first_arg( line )) == 0 )) {
        // remove the trailing double quote
        line[ nLength ] = '\0';

        // remove the leading double quote
        strcpy( line, line + 1 );
    }
}


// initialize the OS/2 environment, alias, and history space
static void init_env( void )
{
    static char szAliasSharename[32] = SHR_4OS2_ALIAS;
    static char szHistorySharename[32] = SHR_4OS2_HISTORY;
    static char szDirHistSharename[32] = SHR_4OS2_DIRHIST;
    PUINT       puSize;

    // make sure default environment size is big enough (minimum 4K)
    if ( QueryMemSize( glpEnvironment) < ENVIRONMENT_SIZE )
        (void)ReallocMem( glpEnvironment, ENVIRONMENT_SIZE );

    // try to open shared segments for the alias & history lists.  If
    //   they don't exist, create them.  (We create shared segs for the
    //   alias & history lists, so they can be shared in all sessions.)
    //   If we started with "4OS2 /L[a|d|h]", then use local (shared
    //   only for that session) lists for aliases and/or history.

    // create an extension consisting of the session number
    if ( gpIniptr->LocalAliases != 0 )
        sprintf( strend( szAliasSharename ), ".%03d", gpLIS->sgCurrent );

    if ( gpIniptr->LocalHistory != 0 )
        sprintf( strend( szHistorySharename ), ".%03d", gpLIS->sgCurrent );

    if ( gpIniptr->LocalDirHistory != 0 )
        sprintf( strend( szDirHistSharename ), ".%03d", gpLIS->sgCurrent );

    if ( DosGetNamedSharedMem( (PVOID)&glpAliasList, szAliasSharename, (PAG_READ | PAG_WRITE) )) {
        // allocate 4K out of a 64K sparse memory block
        if (( DosAllocSharedMem( (PVOID)&glpAliasList, szAliasSharename, 65535L, (PAG_READ | PAG_WRITE))) || ( DosSetMem( glpAliasList, ALIAS_SIZE, (PAG_COMMIT | PAG_READ | PAG_WRITE))))
            _exit( error(ERROR_NOT_ENOUGH_MEMORY, NULL ));

        // initialize the alias list to 0's
        memset( glpAliasList, '\0', ALIAS_SIZE );
    }

    // kludge for different entries used in OPTION
    gpIniptr->DirHistoryNew = gpIniptr->DirHistorySize;
    gpIniptr->HistoryNew = gpIniptr->HistorySize;

    if ( DosGetNamedSharedMem( (PVOID)&glpHistoryList, szHistorySharename, (PAG_READ | PAG_WRITE) )) {
        // default history size is 1K
        if ( DosAllocSharedMem( (PVOID)&glpHistoryList, szHistorySharename, gpIniptr->HistorySize + sizeof(UINT), (PAG_READ | PAG_WRITE | PAG_COMMIT) ))
            _exit( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));

        // initialize the history list to 0's
        memset( glpHistoryList, '\0', gpIniptr->HistorySize + sizeof(UINT) );

        // set the history size in the shared area
        puSize = (PUINT)glpHistoryList;
        *puSize = (UINT)gpIniptr->HistorySize;
    } else {
        // set size of existing shared memory
        puSize = (PUINT)glpHistoryList;
        gpIniptr->HistorySize = *puSize;
    }

    glpHistoryList += sizeof(UINT);

    if ( DosGetNamedSharedMem( (PVOID)&glpDirHistory, szDirHistSharename, (PAG_READ | PAG_WRITE) )) {
        // default directory history size is 256 bytes
        if ( DosAllocSharedMem( (PVOID)&glpDirHistory, szDirHistSharename, gpIniptr->DirHistorySize + sizeof(UINT), (PAG_READ | PAG_WRITE | PAG_COMMIT) ))
            _exit( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));

        // initialize the directory history list to 0's
        memset( glpDirHistory, '\0', gpIniptr->DirHistorySize + sizeof(UINT) );
        puSize = (PUINT)glpDirHistory;
        *puSize = (UINT)gpIniptr->DirHistorySize;
    } else {
        // set size of existing shared memory
        puSize = (PUINT)glpDirHistory;
        gpIniptr->DirHistorySize = *puSize;
    }

    glpDirHistory += sizeof(UINT);

    // create queue for child sessions to notify us when they exit
    sprintf( gszQueueName, "\\QUEUES\\4OS32\\TQ%03d.%03d", gpLIS->sgCurrent, gpLIS->pidCurrent);

    (void)DosCreateQueue( &ghQueueHandle, (QUE_FIFO | QUE_CONVERT_ADDRESS), gszQueueName );

    // set DescriptionName
    if ( gpIniptr->DescriptName != INI_EMPTYSTR )
        sprintf( DESCRIPTION_FILE, "%.63s", gpIniptr->StrData + gpIniptr->DescriptName );

    // set COMSPEC
    sprintf( gszCmdline, COMSPEC_OS2, COMSPEC, _pgmptr );
    add_variable( gszCmdline );
}


// initialize the various system variables (session type, process ID, etc.)
static void InitSystemVariables( void )
{
    int     i;
    char    *pchar, szOs2CmdLine[MAXLINESIZ + MAXFILENAME + 2];
    char    szHelperDllName[MAXFILENAME];
    HSWITCH hSwitch;
    SWCNTRL swctl;
    SEL     selGlobalSeg, selLocalSeg;
    HMODULE hmod, hmod2;
    PTIB    ptib;
    PIB     *ppib;
    char    *fptr;
    void    * _Seg16 pWork16;
    ULONG   *pulW16 = (PULONG)&pWork16, ulTemp;
    PFN     pfnWSTAI32;
    APIRET  rc;

    // allow 40 file handles
    (void)DosSetMaxFH( 40 );

    // read the info seg for session number & parent PID
    (void)DosGetInfoSeg( &selGlobalSeg, &selLocalSeg );

    // KLUDGE for WATCOM C problems in converting a pointer to a 16-bit segment.
    // We have to convert to a long first, then store it, otherwise Watcom C
    // converts it to 16-bit and immediately back to 32-bit

    *pulW16 = ( selLocalSeg << 16 );
    gpLIS = (PLINFOSEG)(void * _Seg16)pWork16;

    (void)DosGetInfoBlocks( &ptib, &ppib );

    // get address of the command line; program name is just before
    // major kludge for major bug? stupidity? oddity? in INSTAID
    pszCmdLineOpts = strdup( ppib->pib_pchcmd + strlen( ppib->pib_pchcmd ) + 1);

    pchar = (char *)ppib->pib_pchcmd - 1;

    // search for null char before name string
    while ( *--pchar )
        ;

    // set name
    _pgmptr = pchar + 1;

    // save the command line pointer before it gets overwritten
    ppib->pib_pchcmd = strdup( ppib->pib_pchcmd );

    glpMasterEnvironment = glpEnvironment = ppib->pib_pchenv;

    // save the boot drive
    DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &ulTemp, sizeof(ulTemp));
    gchSysBootDrive = (char)( ulTemp + 'A' - 1);

    DosQuerySysInfo(QSV_VERSION_REVISION, QSV_VERSION_REVISION, &ulTemp, sizeof(ulTemp));
    gnOS2_Revision = (unsigned int)( ulTemp + 'A' );

    // check to see if it's a child pipe process
    if (( fptr = get_variable( SHAREMEM_PIPE_ENV )) != NULL ) {

        if ( DosGetNamedSharedMem( (PPVOID)&gpPipeParent, fptr, PAG_READ | PAG_WRITE) == 0 ) {

            // read the critical variables & batch frames from shared mem
            memmove( &cv, gpPipeParent->CriticalVars, sizeof(CRITICAL_VARS) );
            memmove( bframe, gpPipeParent->BatchFrame, ( sizeof(BATCHFRAME) * MAXBATCH) );

            if ( cv.bn >= 0 ) {
                gnPipeBN = cv.bn;
                cv.call_flag = 1;

                // we don't need these arguments
                bframe[ cv.bn ].local_env = NULL;
                bframe[ cv.bn ].local_alias = NULL;
                bframe[ cv.bn ].in_memory_buffer = NULL;
                bframe[ cv.bn ].bfd = -1;
                bframe[ cv.bn ].OnBreak = NULL;
                bframe[ cv.bn ].OnError = NULL;
                bframe[ cv.bn ].OnErrorMsg = NULL;
                bframe[ cv.bn ].OnErrorState = 0;

                // read & save batch arguments from shared memory
                memmove( szOs2CmdLine, gpPipeParent->CommandLine, MAXLINESIZ + MAXFILENAME );
                memset( gpPipeParent->CommandLine, '\0', MAXLINESIZ );

                bframe[ cv.bn ].Argv = (char **)malloc( ARGMAX * sizeof(char *) );

                pchar = szOs2CmdLine;
                gpBatchName = bframe[cv.bn].pszBatchName = _strdup( pchar );
                pchar += strlen( pchar ) + 1;

                for ( i = 0; ( *pchar != '\0' ); i++ ) {
                    bframe[cv.bn].Argv[i] = _strdup( pchar );
                    pchar += strlen( pchar ) + 1;
                }

                bframe[cv.bn].Argv[i] = NULL;
            }
        }

        // remove the variable before a child process sees it!
        gpPipeParent->fInitialized = 1;
        add_variable( SHAREMEM_PIPE_ENV );
    }

    // get WinQueryWindowPos, WinSetWindowPos, WinQuerySysValue,
    //   WinPostMessage, and WinSetKeyboardStateTable
    if ( DosLoadModule( szOs2CmdLine, sizeof( szOs2CmdLine ), "PMWIN", &hmod ) == 0 ) {
        DosQueryProcAddr( hmod, ORD_WIN32INITIALIZE, NULL, (PFN *)&pfnWINIT );
        DosQueryProcAddr( hmod, ORD_WIN32TERMINATE, NULL, (PFN *)&pfnWTERM );
        DosQueryProcAddr( hmod, ORD_WIN32SETACTIVEWINDOW, NULL, (PFN *)&pfnWSAW );
        DosQueryProcAddr( hmod, ORD_WIN32GETLASTERROR, NULL, (PFN *)&pfnWGLE );
        DosQueryProcAddr( hmod, ORD_WIN32SETWINDOWPOS, NULL, (PFN *)&pfnWSWP );
        DosQueryProcAddr( hmod, ORD_WIN32QUERYWINDOWPOS, NULL, (PFN *)&pfnWQWP );
        DosQueryProcAddr( hmod, ORD_WIN32POSTMSG, NULL, (PFN *)&pfnWPM );
        DosQueryProcAddr( hmod, ORD_WIN32QUERYSYSVALUE, NULL, (PFN *)&pfnWQSV );
        DosQueryProcAddr( hmod, ORD_WIN32SETKEYBOARDSTATETABLE, NULL, (PFN *)&pfnWSKST );
        DosQueryProcAddr( hmod, ORD_WIN32SETWINDOWTEXT, NULL, (PFN *)&pfnWSWT );

        DosQueryProcAddr( hmod, ORD_WIN32OPENCLIPBRD, NULL, (PFN *)&pfnWOC );
        DosQueryProcAddr( hmod, ORD_WIN32EMPTYCLIPBRD, NULL, (PFN *)&pfnWEC );
        DosQueryProcAddr( hmod, ORD_WIN32SETCLIPBRDDATA, NULL, (PFN *)&pfnWSCD );
        DosQueryProcAddr( hmod, ORD_WIN32CLOSECLIPBRD, NULL, (PFN *)&pfnWCC );
        DosQueryProcAddr( hmod, ORD_WIN32QUERYCLIPBRDFMTINFO, NULL, (PFN *)&pfnWQCFI );
        DosQueryProcAddr( hmod, ORD_WIN32QUERYCLIPBRDDATA, NULL, (PFN *)&pfnWQCD );
    }

    if ( pfnWINIT != NULL )
        ghHAB = (*pfnWINIT)( 0 );

    // Get DosQueryExtLIBPATH and DosSetExtLIBPATH in Warp
    if ( gnOsVersion >= 230 ) {
        // Look for loaded JPOS2DLL.DLL first
        rc = DosLoadModule( szOs2CmdLine, sizeof( szOs2CmdLine ), "JPOS2DLL", &hmod );
        if (rc) {
            // If that failed, try the 4OS2.EXE directory
            FindInstalledFile ( szHelperDllName, "JPOS2DLL.DLL" );
            rc = DosLoadModule( szOs2CmdLine, sizeof( szOs2CmdLine ), szHelperDllName, &hmod );
        }
        if ( rc == 0 ) {
            DosQueryProcAddr( hmod, 3, NULL, (PFN *)&pfnSendKeys );
            DosQueryProcAddr( hmod, 4, NULL, (PFN *)&pfnQuitSendKeys );
        };// else We don't want to always see this stupid message
//            qprintf( STDERR, NO_DLL );
    }



    // get WinSetTitleAndIcon, WinQuerySwitchEntry, WinQuerySwitchHandle,
    //   WinStartApp, and WinChangeSwitchEntry
    if ( DosLoadModule( szOs2CmdLine, sizeof( szOs2CmdLine ), "PMSHAPI", &hmod2 ) == 0 ) {
        // KLUDGE for WATCOM C problems in converting a pointer to a 16-bit function.
        // We have to convert to a long first, then store it, otherwise Watcom C
        // converts it to 16-bit and immediately back to 32-bit
        DosQueryProcAddr( hmod2, ORD_WINSETTITLEANDICON, NULL, (PFN *)&pfnWSTAI32 );
        pWork16  = (SHORT (_Far16 *)())pfnWSTAI32;
        pfnWSTAI = (WSTAI)*pulW16;

        DosQueryProcAddr( hmod2, ORD_WIN32QUERYSWITCHLIST, NULL, (PFN *)&pfnWQSL );
        DosQueryProcAddr( hmod2, ORD_WIN32QUERYSWITCHENTRY, NULL, (PFN *)&pfnWQSE );
        DosQueryProcAddr( hmod2, ORD_WIN32QUERYSWITCHHANDLE, NULL, (PFN *)&pfnWQSH );
        DosQueryProcAddr( hmod2, ORD_WIN32CHANGESWITCHENTRY, NULL, (PFN *)&pfnWCSE );
        DosQueryProcAddr( hmod2, ORD_WIN32STARTAPP, NULL, (PFN *)&pfnWSA );
        DosQueryProcAddr( hmod2, ORD_PRF32OPENPROFILE, NULL, (PFN *)&pfnPOP );
        DosQueryProcAddr( hmod2, ORD_PRF32QUERYPROFILEDATA, NULL, (PFN *)&pfnPQPD );
        DosQueryProcAddr( hmod2, ORD_PRF32WRITEPROFILEDATA, NULL, (PFN *)&pfnPWPD );
        DosQueryProcAddr( hmod2, ORD_PRF32CLOSEPROFILE, NULL, (PFN *)&pfnPCP );

        if (( hSwitch = (HSWITCH)(*pfnWQSH)( 0, gpLIS->pidCurrent )) != NULL ) {
            (*pfnWQSE)( hSwitch, &swctl );
            ghwndWindowHandle = swctl.hwnd;
        }
    }
}


// initialize the INI file data
static void init_ini( void )
{
    unsigned int    i;
    PCH             prev_shell_sharedmem = 0L;
    char            shell_sharename[32];
    char            szFname[MAXFILENAME + 1];
    int             save_shell;
    PCH             prev_ini_data;
    INIFILE         *prev_ini;
    ULONG           prev_size;
    int             valid = 0;

    // get shell number
    for ( i = 0; ; i++ ) {
        // if shared memory for shell i exists, save its pointer so we
        //   can use it to inherit the INI file data
        // if not, i is our shell number -- reserve a shared memory segment
        //    for it
        sprintf( shell_sharename, "\\SHAREMEM\\4OS2.%03d\\SHELL%03d.%03d", VER_BUILD, gpLIS->sgCurrent, i );

        if ( DosGetNamedSharedMem( (PVOID)&prev_shell_sharedmem, shell_sharename, (PAG_READ | PAG_WRITE)) != 0 ) {
            if ( DosAllocSharedMem( (PVOID)&shell_sharedmem, shell_sharename, INI_TOTAL_BYTES, (PAG_READ | PAG_WRITE | PAG_COMMIT)) == 0 )
                memset( shell_sharedmem, '\0', INI_TOTAL_BYTES );
            break;
        }
    }

    // set shell number, clear out dummy log file name
    gaInifile.ShellNum = (char)i;

    if (( gaInifile.ShellNum != 0 ) && ( prev_shell_sharedmem != 0L )) {
        // we have an old INI file, see if we can inherit it (signature must
        // match and string / key data must fit in our data areas)
        prev_ini = (INIFILE *)(prev_ini_data = prev_shell_sharedmem);
        prev_size = QueryMemSize( prev_shell_sharedmem );
        valid = ( prev_size > (ULONG)sizeof(INIFILE) ) && ( prev_ini->INISig == INI_SIG ) && ( prev_ini->INIBuild == VER_BUILD ) && ( prev_ini->StrUsed <= INI_STRMAX ) && ( prev_ini->KeyUsed <= INI_KEYMAX );
    }

    if ( valid ) {
        // old INI file data seems valid so inherit it
        save_shell = gaInifile.ShellNum;
        memmove( &gaInifile, prev_ini_data, sizeof(INIFILE) );
        prev_ini_data += sizeof(INIFILE);
        memmove( gaIniStrings, prev_ini_data, prev_ini->StrUsed );
        prev_ini_data += prev_ini->StrUsed;
        memmove( gaIniKeys, prev_ini_data, ( 2 * sizeof(unsigned int) * prev_ini->KeyUsed ));
        gaInifile.ShellNum = save_shell;
    } else {
        // no inheritance, clear the INI file data
        gaInifile.StrUsed = 0;
        gaInifile.KeyUsed = INI_DEFKEYS;
        IniClear(&gaInifile );
        gaInifile.INIBuild = VER_BUILD;
        gaInifile.INISig = INI_SIG;
        gaInifile.BootDrive = gchSysBootDrive;
    }

    // set up pointer to strings area
    gaInifile.StrData = gaIniStrings;
    gaInifile.StrMax = INI_STRMAX;

    // set up pointer to keys area
    gaInifile.Keys = gaIniKeys;
    gaInifile.KeyMax = INI_KEYMAX;

    // Set up and clear option bit flags
    gaInifile.OBCount = OBSIZE;
    memset( &(gaInifile.OptBits), '\0', sizeof(gaInifile.OptBits) );

    // handle reprocessing original file in a secondary shell
    if ( gaInifile.ShellNum != 0 ) {
        // process user's "next" INI file if any specified
        if ( gaInifile.NextININame != INI_EMPTYSTR )
            (void)process_ini( strcpy( szFname, gaInifile.StrData + gaInifile.NextININame ));

        // otherwise reprocess primary file if it had a [Secondary] section
        else if (( gaInifile.PrimaryININame != INI_EMPTYSTR ) && (( gaInifile.SecFlag & INI_SECONDARY ) != 0 ))
            (void)process_ini( strcpy( szFname, gaInifile.StrData + gaInifile.PrimaryININame ));
    }
}


// process default INI file if no INI file has been processed yet
static void default_ini( void )
{
    if ( ini_done == 0 ) {
        // check in 4OS2.EXE directory & in root
        strcpy( gszCmdline, _pgmptr );
        insert_path( gszCmdline, OS2_INI, gszCmdline );

        if ( process_ini( gszCmdline ) && (( stricmp( gszCmdline+3, OS2_INI ) != 0 ) || ( gszCmdline[0] != gaInifile.BootDrive ))) {
            strcpy( gszCmdline+3, OS2_INI );
            gszCmdline[0] = gaInifile.BootDrive;
            (void)process_ini( gszCmdline );
        }
    }

    ini_done = 1;           // we need no more than one try at default
}


static int process_ini( char *fname )
{
    unsigned int    ecode = 0;
    char            fullname[MAXFILENAME+1];

    if ( *fname == '@' )
        fname++;

    strcpy( fullname, fname );
    mkfname( fullname, 0 );

    // if it processes OK, save the name for next shell
    if (( ecode = IniParse( fullname, (INIFILE *)&gaInifile, (( gaInifile.ShellNum != 0 ) ? INI_PRIMARY : INI_SECONDARY))) == 0 ) {
        (void)ini_string( gpIniptr, (int *)(&( gpIniptr->PrimaryININame )), fullname, strlen( fullname ));

        // show we processed a file ==> skip any default
        ini_done = 1;
    }

    return ecode;
}


// copy INI file data to the shared memory area for secondary copies
void SaveIniData( void )
{
    PCH pass_ini;

    if (( pass_ini = shell_sharedmem ) != NULL ) {
        memmove( pass_ini, (char *)&gaInifile, sizeof(INIFILE) );
        pass_ini += sizeof(INIFILE);
        memmove( pass_ini, (char *)gaIniStrings, gaInifile.StrUsed );
        pass_ini += gaInifile.StrUsed;
        memmove( pass_ini, (char *)gaIniKeys, (2 * sizeof(unsigned int) * gaInifile.KeyUsed) );
    }
}


// Set our window position if requested
static void set_window( void )
{
    if ( ghwndWindowHandle != NULL ) {
        // Pick window position. Default does nothing ==> OS/2 standard window
        // is used
        switch ( gpIniptr->WindowState ) {
            case 1:     // maximize the window
                (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_MAXIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                break;

            case 2:     // minimize the window
                (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_MINIMIZE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                break;

            case 3:     // Resize / move the window
                (void)(*pfnWSWP)( ghwndWindowHandle, (HWND)0,
                    gpIniptr->WindowX, gpIniptr->WindowY,
                    gpIniptr->WindowWidth, gpIniptr->WindowHeight,
                    SWP_SIZE | SWP_MOVE);

                // kludge for OS/2 bug
                (void)(*pfnWPM)( ghwndWindowHandle, WM_SYSCOMMAND, MPFROMSHORT(SC_RESTORE), MPFROM2SHORT(CMDSRC_MENU, TRUE));
                break;
        }
    }
}


// display copyright message
void DisplayCopyright( void )
{
    printf( OS2_VERSION, PROGRAM, gszOsVersion );
    printf( COPYRIGHT );
    crlf();
}

