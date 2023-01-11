// BATCH.C - Batch file routines for 4xxx / TCMD family
//   Copyright ( c) 1988 - 1998  Rex C. Conn   All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <share.h>
#include <string.h>

#include "4all.h"


static int load_btm( void );
static int WhileTest( char * );
static int goto_label( char *, int );
static char * ExpandArgument( char *, int );
int TestCondition( char *, int );
static int _setlocal( void );
static int _endlocal( void );

static char DELIMS[] = "%9[^  .\"`\\+=:<>|%]";

extern int fNoComma;


// Execute a batch file (.BAT or .BTM or .CMD)
int batch( int argc, char **argv )
{
        extern char szWindowTitle[];
        int i;
        unsigned int save_echo;
        long lSaveFlags;

        // we can only nest batch files up to MAXBATCH levels deep
        if ( cv.bn >= MAXBATCH - 1 )
                return ( error( ERROR_4DOS_EXCEEDED_NEST, NULL ) );

        // set ECHO state flag (from parent file or CFGDOS)
        save_echo = (( cv.bn < 0 ) ? (unsigned int)gpIniptr->BatEcho : bframe[cv.bn].echo_state );

        // disable batch file nesting unless CALL ( call_flag == 1 )
        if ( cv.call_flag == 0 ) {

                // reset old batch file to the new batch file!
                if ( cv.bn >= 0 )
                        exit_bat();
                else            // first batch file counts as nested
                        cv.call_flag |= 1;
        }

        // disable ^C / ^BREAK during setup
        HoldSignals();

        // to allow nested batch files, create a batch frame containing
        //   all the info necessary for the batch process
        cv.bn++;

        // clear the current frame
        memset( &bframe[cv.bn], '\0', sizeof( bframe[cv.bn]) );
        bframe[cv.bn].bfd = -1;

        // save the fully expanded name & restore the original name
        //   (for COMMAND.COM compatibility)
        if ( cv.call_flag & 0x200 )
                bframe[cv.bn].pszBatchName = _strdup( argv[0] );
        else
                bframe[cv.bn].pszBatchName = filecase( _strdup( argv[0] ));
        argv[0] = gpBatchName;

        // save batch arguments (%0 - %argc) on the heap
        bframe[cv.bn].Argv = ( char **)malloc(( argc + 1 ) * sizeof( char *));
        bframe[cv.bn].Argv[argc] = NULL;

        for ( i = 0; ( i < argc ); i++ )
                bframe[cv.bn].Argv[i] = _strdup( argv[i] );

        // set ECHO state (previously saved from parent file or .INI file )
        bframe[cv.bn].echo_state = ( char)save_echo;

        EnableSignals();

        // load an "in-memory" batch file?
        if ( _stricmp( BTM, ext_part( bframe[cv.bn].pszBatchName )) == 0 ) {

                if (( setjmp( cv.env) == -1 ) || ( load_btm() != 0 )) {
                        exit_bat();
                        return (( cv.exception_flag ) ? CTRLC : ERROR_EXIT );
                }
        }

        // Save current window title into .pszTitle
        bframe[cv.bn].pszTitle = _strdup( szWindowTitle );

        // save the DO & IFF flags when CALLing another batch file
        if ( cv.call_flag )
                lSaveFlags = cv.f.lFlags;

        // clear the DO & IFF flags before running another batch file
        cv.f.lFlags = 0L;

        // if nesting, call BatchCLI() recursively...
        if ( cv.call_flag ) {

            // check for REXX file
            if ( cv.call_flag & 0x200 ) {

                cv.call_flag = 0;

                // we need to set ERRORLEVEL for REXX files
                argc = gnErrorLevel = nCallRexx();

            } else {

                // if CALL /Q, set echo off unless "persistent echo" set
                if (( cv.call_flag & 0x100 ) && (( bframe[ cv.bn ].echo_state & 2 ) == 0 ))
                        bframe[ cv.bn ].echo_state = 0;

                cv.call_flag = 0;
                if (( argc = BatchCLI()) == 0 )
                        argc = bframe[ cv.bn ].nReturn;
            }
            exit_bat();

            // restore the IFF flags
            cv.f.lFlags = lSaveFlags;

        } else {
            HoldSignals();
            argc = ABORT_LINE;          // end any FOR processing
        }

        return argc;
}


// load a batch file into memory (as a .BTM)
static int load_btm( void )
{
        extern void _System BatDComp(PCH, PCH);
        UINT uSize;
        LONG lOSize;
        unsigned long ulCSize = 0L;
        PCH fptr = 0L;

        if ( open_batch_file() == 0 )
                return ERROR_EXIT;

        // rewind & read the file header
        RewindFile( bframe[cv.bn].bfd );
        (void)_read( bframe[cv.bn].bfd, (void *)&ulCSize, 4 );

        // get size of original file
        lOSize = (LONG)QueryFileSize( bframe[cv.bn].pszBatchName, 0 );

        // add the line size to avoid protection violation
        uSize = (UINT)(lOSize + CMDBUFSIZ);

        if (( bframe[cv.bn].in_memory_buffer = AllocMem( (UINT *)&uSize )) == 0L ) {
                FreeMem(fptr);
                close_batch_file();
                return (error( ERROR_NOT_ENOUGH_MEMORY, bframe[cv.bn].pszBatchName ));
        }

        // rewind & read the file (max of 64K)
        RewindFile( bframe[cv.bn].bfd );
        (void)FileRead( bframe[cv.bn].bfd, bframe[cv.bn].in_memory_buffer, (UINT)lOSize, (UINT *)&uSize );

        close_batch_file();

        bframe[cv.bn].in_memory_buffer[uSize] = EoF;

        // change the file type to .BTM
        bframe[cv.bn].bfd = IN_MEMORY_FILE;

        return 0;
}


// clean up & exit a batch file
int exit_bat( void )
{
        extern int gnPipeBN;
        int n;

        HoldSignals();

        // if in a pipe, don't close files for the parent process!
        if ( cv.bn <= gnPipeBN )
                return ERROR_EXIT;

        // free the batch argument list
        free( bframe[cv.bn].pszBatchName );
        for ( n = 0; ( bframe[cv.bn].Argv[n] != NULL ); n++)
                free( bframe[cv.bn].Argv[n] );
        free( ( char *)bframe[cv.bn].Argv );

        // restore saved environment
        if ( bframe[cv.bn].local_env != 0L )
                (void)_endlocal();

        // free the internal buffer for .BTM files
        FreeMem( bframe[cv.bn].in_memory_buffer );

        // free the window title (if any)
        if ( bframe[cv.bn].pszTitle != NULL )
                free( bframe[cv.bn].pszTitle );

        // free ON xxx arguments
        if ( bframe[cv.bn].OnBreak != NULL ) {
                free( bframe[cv.bn].OnBreak );
                bframe[cv.bn].OnBreak = NULL;
        }

        if ( bframe[cv.bn].OnErrorMsg != NULL ) {
                free( bframe[cv.bn].OnErrorMsg );
                bframe[cv.bn].OnErrorMsg = NULL;
        }

        if ( bframe[cv.bn].OnError != NULL ) {

                free( bframe[cv.bn].OnError );
                bframe[cv.bn].OnError = NULL;

                // reset error popups to previous value
                DosError( (ULONG)bframe[cv.bn].OnErrorState );
        }

        // point back to previous batch file (if any)
        cv.bn--;

        // reset the window title
        update_task_list( NULL );

        return 0;
}


int BatchDebugger( void )
{
        extern int gnPopExitKey;
        int i, n;
        unsigned int nLength, uSize;
        unsigned long lListSize = 0L;
        char * fptr;
        char *lpBuf;
        char szBuffer[MAXFILENAME+12];
        char * *list = 0L;

        if (( n = _sopen( bframe[ cv.bn ].pszBatchName, (O_RDONLY | O_BINARY), SH_DENYWR) ) < 0 )
                return ( error( _doserrno, bframe[cv.bn].pszBatchName ));

        uSize = (UINT)( QueryFileSize( bframe[ cv.bn ].pszBatchName, 0 ) + 0x10 );
// FIXME - files > 64K??
        if (( lpBuf = AllocMem( &uSize )) != 0L ) {
                FileRead( n, lpBuf, uSize, (UINT *)&uSize );
                lpBuf[uSize] = '\0';
        }
        _close( n );

        if ( lpBuf == 0L )
                return ( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));

        HoldSignals();

        for ( i = 0, fptr = lpBuf; (( *fptr != '\0' ) && ( *fptr != 0x1A )); ) {

                // allocate memory for 256 entries at a time
                if (( i % 256 ) == 0 ) {
                    lListSize += ( 256 * sizeof(char *) );
                    if (( list = (char * *)ReallocMem( (char *)list, lListSize )) == 0L )
                        return ( error( ERROR_NOT_ENOUGH_MEMORY, NULL ));
                }

                list[i++] = fptr;

                sscanf( fptr, "%*[^\r\n]%n", &nLength );
                fptr += nLength;
                if ( *fptr == '\r' ) {
                        *fptr++ = '\0';
                        if ( *fptr == '\n' )
                                fptr++;
                } else if ( *fptr == '\n' )
                        *fptr++ = '\0';
        }

        // call the popup window
        sprintf( szBuffer, "%s [%u]", bframe[cv.bn].pszBatchName, bframe[cv.bn].uBatchLine );

        if ( wPopSelect( 2, 4, 8, 70, list, i, bframe[cv.bn].uBatchLine, szBuffer, DEBUGGER_PROMPT, "TSJXVAOQL", 0x12 ) == 0L )
                gnPopExitKey = ESC;

        FreeMem( lpBuf );
        FreeMem( (char *)list );

        // reenable signal handling after cleanup
        EnableSignals();

        // if we aborted wPopSelect with ^C, bomb after cleanup
        if ( cv.exception_flag )
                longjmp( cv.env, -1 );

        return gnPopExitKey;
}



// send a block of text to the screen
int battext_cmd( int argc, char **argv )
{
        char *arg;

        // make sure the batch file is still open
        if ( open_batch_file() == 0 )
                return BATCH_RETURN;

        // scan the batch file until we find an ENDTEXT
        while ( getline( bframe[cv.bn].bfd, gszCmdline, CMDBUFSIZ-1, EDIT_DATA ) > 0 ) {

                bframe[cv.bn].uBatchLine++;

                arg = first_arg( gszCmdline );
                if (( arg != NULL ) && ( _stricmp( arg, ENDTEXT ) == 0 ))
                        return 0;

                printf( FMT_STR_CRLF, gszCmdline );
        }

        return ( error( ERROR_4DOS_MISSING_ENDTEXT, NULL ));
}


// make a beeping noise (optionally specifying tone & length)
int beep_cmd( int argc, char **argv)
{
        unsigned int frequency, duration;

        frequency = gpIniptr->BeepFreq;
        duration = gpIniptr->BeepDur;

        do {
                if ( argv[1] != NULL ) {
                        frequency = atoi(*(++argv));
                        if ( argv[1] != NULL )
                                duration = atoi(*(++argv));
                }
                SysBeep( frequency, duration );
        } while ( argv[1] != NULL );

        return 0;
}


// CALL a nested batch file (or any executable file )
// (because the CALL argument may be a .COM or .EXE, we can't just call BATCH)
int call_cmd(int argc, char **argv)
{
        char *arg;

        if ( argc == 1 )
                return ( usage( CALL_USAGE ));

        cv.call_flag = 1;

        // support the dimwit CMD.EXE "CALL FILE /Q" option
        for ( argc = 0; (( arg = ntharg( argv[1], argc )) != NULL ); argc++) {

                // remove the /Q from the command line & set call_flag
                if ( _stricmp( arg, "/Q" ) == 0 ) {

                        strcpy( gpNthptr, skipspace( gpNthptr+2 ));

                        // /Q switch is the same as @ECHO OFF
                        cv.call_flag = 0x101;
                        break;
                }
        }

        argc = command( argv[1], 0 );
        cv.call_flag = 0;

        return argc;
}


// wait for the specified number of seconds
int delay_cmd( int argc, char **argv )
{
        ULONG wait_until = 1L;

        if ( argc > 1 )
                sscanf( argv[1], FMT_ULONG, &wait_until );

        SysWait( wait_until );

        return 0;
}


// DO loop
int do_cmd( int argc, char **argv)
{
        char *arg;
        LONG lSaveOffset, lRepetitor = -1L;
        LONG lStart = 0L, lEnd = 0L, lBy = 1L, lOldDoState, lCurrentDoState;
        char fIn = 0, fRepeat = 0, fTo = 0, fWhile = 0, fUntil = 0;
        char cInclusive = 0, cExclusive = 0;
        char szVar[80], *pExpression = NULL, *ptr;
        int rval = 0, n, fval = FIND_FIRST, hFH = -1;
        unsigned int uMode = 0x110, uSavedLine;
        long fDoFlags;
        FILESEARCH dir;

        dir.hdir = INVALID_HANDLE_VALUE;

        if (( arg = first_arg( argv[1] )) != NULL ) {

            if (( _stricmp( arg, DO_WHILE ) == 0 ) || ( _stricmp( arg, DO_UNTIL ) == 0 )) {

                if ( argv[2] == NULL ) {
do_usage:
                        return usage( DO_USAGE );
                }

                if ( _ctoupper( *arg ) == 'W' )
                        fWhile = 1;
                else
                        fUntil = 1;

                pExpression = argv[2];

            } else {

                if ( var_expand( argv[1], 1 ) != 0 )
                        return ERROR_EXIT;

                arg = skipspace( argv[1] );
                strip_trailing( arg, WHITESPACE );

                if ( _stricmp( arg, DO_FOREVER ) == 0 ) {

                        // repeat forever
                        fRepeat = 1;

                } else if ( is_signed_digit( *arg )) {

                        // repeat specified # of times
                        sscanf( arg, FMT_LONG, &lRepetitor );
                        fRepeat = 1;

                        if ( lRepetitor < 0 )
                                goto do_usage;

                } else if ((( ptr = ntharg( arg, 1 )) != NULL ) && ( stricmp( ptr, "in" ) == 0 )) {

                        // DO x in *.* ...   or   DO x in @filename
                        fIn = 1;
                        uMode = 0x10 | FIND_DATERANGE | FIND_CREATE | FIND_NO_DOTNAMES;

                        // save the inclusive/exclusive modes, in case we're
                        //   doing a DIR or SELECT
                        cInclusive = gchInclusiveMode;
                        cExclusive = gchExclusiveMode;

                        // initialize date/time/size ranges start & end values
                        ntharg( arg, 2 );
                        if (( pExpression = gpNthptr ) == NULL )
                                goto do_usage;

                        if ( GetRange( pExpression, &(dir.aRanges), 1 ) != 0 )
                                goto do_usage;

                        if ( dir.aRanges.pszExclude != NULL ) {
                                ptr = _alloca( strlen( dir.aRanges.pszExclude ) + 1 );
                                dir.aRanges.pszExclude = strcpy( ptr, dir.aRanges.pszExclude );
                        }

                        // check for and remove switches
                        if ( GetSwitches( pExpression, "*", &fDoFlags, 1 ) != 0 )
                                goto do_usage;

                        if ( fDoFlags )
                                uMode |= FIND_BYATTS | 0x07;

                        if ( *pExpression == '\0' )
                                goto do_usage;
                        sscanf( arg, "%79[^ \t=,]", szVar );

                } else {

                        // must be "DO x=1 to n [by z]" syntax
                        if ( sscanf( arg, "%79[^ \t=,] = %ld %*[TtOo] %ld %n", szVar, &lStart, &lEnd, &n ) < 4)
                                goto do_usage;

                        // check for optional "BY n" syntax
                        arg += n;
                        if ( *arg ) {
                                if ( _strnicmp( arg, DO_BY, 2 ) == 0 )
                                        sscanf( arg+2, FMT_LONG, &lBy );
                                else
                                        goto do_usage;
                        }

                        fTo = 1;
                }
            }
        }

        if ( pExpression != NULL ) {
                arg = _alloca( strlen( pExpression ) + 1 );
                pExpression = strcpy( arg, pExpression );
        }

        // save current position to RETURN to
        lSaveOffset = bframe[cv.bn].offset;

        // save previous DO flags
        lOldDoState = cv.f.lFlags;
        (cv.f.flag._do)++;

        // save the current line number
        uSavedLine = bframe[ cv.bn ].uBatchLine;

        lCurrentDoState = cv.f.lFlags;

        for ( ; ; ) {

                // we need to do a ^C check here in case somebody puts something
                //   like %_kbhit in the DO statement!
                if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {
                        rval = CTRLC;
                        DosFindClose( dir.hdir );
                        break;
                }

                // check if we've aborted the DO loop (i.e., a batch file
                //   chained to another batch file )
                if ( cv.f.flag._do == 0 )
                        break;

                if ( cv.f.flag._do_leave ) {
                        cv.f.flag._do_leave--;
                        break;
                }

                // if new offset < original offset, we must have GOTO'd
                //   to previous point, so abort this DO loop
                if ( bframe[ cv.bn ].offset < lSaveOffset )
                        break;

                // restore the saved file read offset
                bframe[ cv.bn ].offset = lSaveOffset;

                // restore saved line number
                bframe[ cv.bn ].uBatchLine = uSavedLine;

                // restore DO / IFF flags
                cv.f.lFlags = lCurrentDoState;

                // *.* or @filename loop?
                if ( fIn ) {

                        gszCmdline[0] = '\0';
                        if ( *pExpression == '@' ) {

                                // get argument from next line in file
                                strcpy( gszCmdline, pExpression + 1 );

                                if ( QueryIsCON( gszCmdline ))
                                        hFH = STDIN;

                                else if ( hFH < 0 ) {

                                        if ( _stricmp( gszCmdline, CLIP ) == 0 ) {
                                                RedirToClip( gszCmdline, 0 );
                                                CopyFromClipboard( gszCmdline );
                                        } else {
                                                mkfname( gszCmdline, 0 );
                                        }

                                        if (( hFH = _sopen( gszCmdline, (O_RDONLY | O_BINARY), SH_DENYWR )) < 0 ) {
                                                rval = error( _doserrno, gszCmdline );
                                                cv.f.flag._do_leave++;
                                        }
                                }

                                if (( hFH >= 0 ) && ( getline( hFH, gszCmdline, CMDBUFSIZ-1, EDIT_DATA ) <= 0 )) {
                                        // end of file - close & get next arg
                                        if ( hFH != STDIN )
                                                _close( hFH );
                                        hFH = -1;
                                        cv.f.flag._do_leave++;
                                }

                        } else {

                                // set VAR to each matching filename
                                gchInclusiveMode = cInclusive;
                                gchExclusiveMode = cExclusive;

                                // kludge for "...\*.*" syntax
                                if ( strstr( pExpression, "...\\" ) != NULL )
                                        mkfname( pExpression, MKFNAME_NOERROR );

                                // if no more matches, get next argument
                                if ( find_file( fval, pExpression, ( uMode | FIND_CREATE), &dir, gszCmdline ) == NULL )
                                        cv.f.flag._do_leave++;

                                fval = FIND_NEXT;
                        }

                        strins( gszCmdline, "=" );
                        strins( gszCmdline, szVar );
                        add_variable( gszCmdline );

                // Repetitor loop?
                } else if ( fRepeat ) {

                    if ( lRepetitor == 0L )
                        cv.f.flag._do_leave++;
                    else if ( lRepetitor > 0L )
                        lRepetitor--;

                // TO loop?
                } else if ( fTo ) {

                    sprintf( gszCmdline, "%s=%ld", szVar, lStart );
                    add_variable( gszCmdline );

                    // if condition fails, wait for ENDDO & then exit
                    if ( lBy < 0L ) {
                        if ( lStart < lEnd )
                            cv.f.flag._do_leave++;
                    } else if ( lStart > lEnd )
                        cv.f.flag._do_leave++;

                    lStart += lBy;

                // WHILE loop?
                } else if ( fWhile ) {
                        if (( n = WhileTest( pExpression )) < 0 ) {
                                rval = -n;
                                break;
                        } else if ( n == 0 ) {
                                // if condition fails, wait for ENDDO & exit
                                cv.f.flag._do_leave++;
                        }
                }

                if ((( rval = BatchCLI()) == CTRLC ) || ( cv.exception_flag ) || ( bframe[cv.bn].offset == -1L )) {
                        bframe[cv.bn].offset = -1L;
                        break;
                }

                // we need to do a ^C check here in case somebody puts
                //   something like %_kbhit in the UNTIL statement!
                if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {
                        rval = CTRLC;
                        DosFindClose( dir.hdir );
                        break;
                }
                EnableSignals();

                // check if we've aborted the DO loop
                if ( cv.f.flag._do == 0 )
                        break;

                // UNTIL loop?
                if ( fUntil ) {

                    if (( n = WhileTest( pExpression )) < 0 ) {
                        rval = -n;
                        break;

                    } else if ( n == 1 ) {

                        cv.f.flag._do_leave++;
                        bframe[cv.bn].offset = lSaveOffset;
                        bframe[cv.bn].uBatchLine = uSavedLine;

                        // dummy call just to skip to ENDDO
                        BatchCLI();
                    }
                }
        }

        if ( hFH > 0 ) {
                _close( hFH );
                hFH = -1;
        }

        // restore previous DO state (if _do == 0, we've had a GOTO)
        if ( cv.f.flag._do )
                cv.f.lFlags = lOldDoState;

        return rval;
}


// evaluate WHILE or UNTIL expression
static int WhileTest( char *pExpression )
{
        int condition;

        if ( cv.f.flag._do_end != 0 )
                return 0;

        if (( condition = TestCondition( strcpy( gszCmdline, pExpression ), 0 )) == -USAGE_ERR) {
                gpInternalName = "DO";
                return -( usage( DO_USAGE ));
        }

        return condition;
}


// check for DO processing
int do_parsing( char *cmd_name )
{
        // if not processing a DO just return
        if ( cv.f.flag._do == 0 )
            return 0;

        if ( _stricmp( cmd_name, DO_END ) == 0 ) {

            // if not ignoring nested DO's, return to do_cmd()
            if ( cv.f.flag._do_end == 0 )
                return BATCH_RETURN;

            // drop one "ignored DO loop" level
            (cv.f.flag._do_end)--;

            return 1;
        }

        if ( _stricmp( cmd_name, DO_DO ) == 0 ) {

            // if waiting for "ENDDO", ignore nested DO
            if ( cv.f.flag._do_end | cv.f.flag._do_leave )
                ( cv.f.flag._do_end)++;

        } else if (( cv.f.flag._do_end | cv.f.flag._do_leave ) == 0 ) {

            if ( _stricmp( cmd_name, DO_LEAVE ) == 0 ) {

                // ignore everything until END, then return to "do_cmd()" & exit
                cv.f.flag._do_leave++;

            } else if ( _stricmp( cmd_name, DO_ITERATE ) == 0 ) {

                // return to top of loop
                return BATCH_RETURN;
            }
        }

        return ( cv.f.flag._do_end | cv.f.flag._do_leave );
}


// echo arguments to the screen (stdout)
int echo_cmd( int argc, char **argv)
{
        if ( stricmp( gpInternalName, "echoerr" ) == 0 ) {
                argc = 0;
                goto EchoErr;
        }

        if ( argc == 1 )                        // just inquiring about ECHO state
                printf( ECHO_IS, (( cv.bn >= 0 ) ? bframe[cv.bn].echo_state : cv.verbose ) ? ON : OFF );

        else if (( argc = OffOn( argv[1] )) == 0 ) {

                // disable batch file or command line echoing
                if ( cv.bn >= 0 ) {
                        // unless "persistent echo" is enabled
                        bframe[cv.bn].echo_state &= 2;
                } else
                        cv.verbose = 0;

        } else if ( argc == 1 ) {

                // enable batch file or command line echoing
                if ( cv.bn >= 0 ) {
                        // don't turn off "persistent echo" (echo_state==2)
                        bframe[cv.bn].echo_state |= 1;
                } else
                        cv.verbose = 1;

        } else {        // print the line verbatim
EchoErr:
                qprintf( (( argc == 0 ) ? STDERR : STDOUT ), FMT_STR_CRLF, *argv + strlen( gpInternalName ) + 1 );
        }

        return 0;
}


// echo arguments to the screen (stdout) without a trailing CR/LF
int echos_cmd( int argc, char **argv)
{
        int fStd;

        // switch STDOUT to binary mode
        if ( argc > 1 ) {

                fStd = (( stricmp( gpInternalName, "echoserr" ) == 0 ) ? 2 : 1 );

                (void)_setmode( fStd, O_BINARY );
                if ( setjmp( cv.env ) != -1 )
                        qprintf( fStd, FMT_STR, *argv + strlen( gpInternalName ) + 1 );
                (void)_setmode( fStd, O_TEXT );
        }

        // disable signal handling momentarily
        HoldSignals();
        return 0;
}


#define FOR_ATTRIBUTES 1
#define FOR_NO_ESCAPE 2
#define FOR_FILESET 4
#define FOR_NO_DOTS 8
#define FOR_STEP 0x10
#define FOR_GLOBAL 0x20
#define FOR_GLOBAL_PASS2 0x40

// FOR - allows iterative execution of DOS commands
int for_cmd( int argc, char **argv)
{
        char *arg, *line;
        char *var, *argset, *pszCurrentDirectory = NULL, *pszRanges = NULLSTR;
        char source[MAXFILENAME], save_inclusive, save_exclusive;
        char *pszTokenArg, szTokens[64], szDelims[31];
        int i, n, rval = 0, hFH = -1, fval, mode = 0x900;
        int nStart = 0, nStep = 0, nEnd = 0, nLastVariable = 0;
        int nEOL = 0, nSkip = 0;
        long fForFlags;
        FILESEARCH dir;

        // initialize date/time/size ranges start & end values
        argv++;

        // save /A:x and range arguments for FOR /R
        source[0] = '\0';

        for ( i = 0; (( arg = ntharg( *argv, i )) != NULL ); i++ ) {
                if (( *arg == '/' ) && (( arg[1] == '[' ) || ( _ctoupper( arg[1] ) == 'A' ) || ( _ctoupper( arg[1] ) == 'H' ))) {
                        if ( i > 0 )
                                strcat( source, " " );
                        strcat( source, arg );
                } else
                        break;
        }

        if ( source[0] ) {
                pszRanges = _alloca( strlen( source ) + 1 );
                strcpy( pszRanges, source );
        }

        // get date/time/size/exception ranges
        GetRange( *argv, &(dir.aRanges), 1 );

        // preserve file exception list
        if ( dir.aRanges.pszExclude != NULL ) {
                arg = _alloca( strlen( dir.aRanges.pszExclude ) + 1 );
                dir.aRanges.pszExclude = strcpy( arg, dir.aRanges.pszExclude );
        }

        // check for and remove switches
        if ( GetSwitches( *argv, "*DFHLRZ", &fForFlags, 1 ) != 0 ) {
for_usage_error:
                return ( usage( FOR_USAGE ));
        }

        if ( fForFlags & FOR_FILESET ) {

                // get the /F token arguments
                strcpy( szTokens, "1" );
                strcpy( szDelims, " \t," );
                while ( **argv == '"' ) {

                        line = first_arg( *argv );
                        if (( arg = stristr( line, "eol=" )) != NULL )
                                nEOL = arg[4];
                        if (( arg = stristr( line, "skip=" )) != NULL )
                                nSkip = atoi(arg+5);
                        if (( arg = stristr( line, "delims=" )) != NULL )
                                sscanf( arg+7, "%31[^\"]", szDelims );
                        if (( arg = stristr( line, "tokens=" )) != NULL )
                                sscanf( arg+7, "%63[^ \t\"]", szTokens );
                        strcpy( *argv, skipspace( *argv + strlen( line )));
                }
        }

        if ( fForFlags & FOR_GLOBAL ) {

                // check for path argument
                if ( **argv != '%' ) {

                        // save the original directory
                        if (( pszCurrentDirectory = gcdir( NULL, 0 )) == NULL )
                                return ERROR_EXIT;
                        pszCurrentDirectory = strdup( pszCurrentDirectory );

                        arg = first_arg( *argv );
                        i = strlen( arg );

                        // check for variable in directory name
                        if ( var_expand( arg, 1 ) != 0 )
                                return ERROR_EXIT;

                        mkfname( arg, 0 );
                        __cd( arg, CD_CHANGEDRIVE );
                        strcpy( *argv, *argv + i );
                }

                strins( *argv, " /z " );
                if ( *pszRanges )
                        strins( *argv, pszRanges );
                strins( *argv, "/iq for " );
                rval = global_cmd( 1, --argv );

                // restore original directory
                if ( pszCurrentDirectory != NULL )
                        __cd( pszCurrentDirectory, CD_CHANGEDRIVE );
                return rval;
        }

        // save the inclusive/exclusive modes, in case we're
        //   doing a DIR or SELECT
        save_inclusive = gchInclusiveMode;
        save_exclusive = gchExclusiveMode;

        if ( fForFlags & 1 )
                mode |= 0x417;

        // check for proper syntax
        arg = ntharg( *argv, 1 );
        if (( arg == NULL ) || ( _stricmp( arg, FOR_IN ) != 0 ) || (( arg = first_arg(*argv)) == NULL ))
                goto for_usage_error;

        // get variable name & save it on the stack
        for ( ; ( *arg == '%' ); arg++ )
                ;
        if ( *arg == '\0' )
                goto for_usage_error;

        var = _alloca( strlen( arg ) + 4 );
        sprintf( var, "%s=", arg );

        // set flag if FOR variable is single char (kludge for COMMAND.COM
        //    & CMD.EXE limitation)
        if ( arg[1] == '\0' )
                strins( var, "\001" );

        ntharg( *argv, 2 );
        argset = gpNthptr;
        if (( argset == NULL ) || ( *argset != '(' ) || (( arg = strchr( argset, ')' )) == NULL ))
                goto for_usage_error;
        argset++;

        // rub out the ')', & point to the command
        *arg++ = '\0';
        arg = skipspace( arg );

        // ignore the DO keyword
        if (( line = first_arg( arg )) == NULL )
                goto for_usage_error;

        if ( _stricmp( line, FOR_DO ) == 0 ) {
                (void)next_arg( arg, 1 );
                if ( *arg == '\0' )
                        goto for_usage_error;
        }

        // compatibility kludge to change "%%v" to "%v" on command line
        for ( line = arg; ; ) {

                if (( arg = scan( arg, "%", BACK_QUOTE )) == BADQUOTES )
                        return ERROR_EXIT;
                if ( *arg == '\0' )
                        break;
                if (( *(++arg) == '%' ) && ( var[0] == 1 ) && ( _ctoupper( arg[1] ) == _ctoupper( var[1] )))
                        strcpy( arg, arg + 1 );
        }

        // save the command line onto the stack
        arg = _alloca( strlen( line ) + 2 );
        line = strcpy( arg, line );

        // do variable expansion on the file spec(s)
        strcpy( gszCmdline, argset );
        if ( var_expand( gszCmdline, 1 ) != 0 )
                return ERROR_EXIT;

        if ( fForFlags & FOR_STEP ) {
                // save the /L arguments
                sscanf( gszCmdline, "%d,%d,%d", &nStart, &nStep, &nEnd );
        } else {
                // save the variable set onto the stack
                arg = _alloca( strlen( gszCmdline ) + 1 );
                argset = strcpy( arg, gszCmdline );
        }

        for ( argc = 0; ; argc++ ) {

                if ( fForFlags & FOR_STEP ) {
                        sprintf( source, FMT_INT, nStart );
                        arg = source;
                } else {

                        source[0] = '\0';

                        // if FOR /R, insert current directory
                        if ( fForFlags & FOR_GLOBAL_PASS2 ) {
                                if (( arg = gcdir( NULL, 0 )) != NULL )
                                        copy_filename( source, arg );
                        }

                        if (( arg = ntharg( argset, argc )) == NULL )
                                break;
                        mkdirname( source, arg );
                }

                for ( fval = FIND_FIRST; ; ) {

                        // check for ^C abort
                        if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {

                                if ( fval == FIND_NEXT )
                                        (void)DosFindClose( dir.hdir );
                                rval = CTRLC;
                                goto for_bye;
                        }

                        // (re )enable ^C and ^BREAK
                        EnableSignals();

                        if (( rval == ABORT_LINE ) || ( bframe[cv.bn].offset == -1L ))
                                goto for_bye;

                        if ( source[0] == '\0' )
                                break;

                        // close file if not still getting input
                        if ( hFH > 0 ) {
                                if ((( fForFlags & FOR_FILESET ) == 0 ) && ( source[0] != '@' )) {
                                        _close( hFH );
                                        hFH = -1;
                                }
                        }

                        gszCmdline[0] = '\0';

                        if (( strpbrk( source, WILD_CHARS ) != NULL ) || ( fval == FIND_NEXT )) {

                                // if ARG includes wildcards, set VAR to each
                                //   matching filename from the disk
                                gchInclusiveMode = save_inclusive;
                                gchExclusiveMode = save_exclusive;

                                // kludge for "...\*.*" syntax
                                if ( strstr( source, "...\\" ) != NULL )
                                        mkfname( source, MKFNAME_NOERROR );

                                // if no more matches, get next argument
                                if ( find_file( fval, source, (mode | FIND_CREATE), &dir, gszCmdline ) == NULL )
                                        break;

                                fval = FIND_NEXT;
                                if (( fForFlags & FOR_NO_DOTS ) && ( QueryIsDotName( dir.name )))
                                        continue;

                        } else if ( fForFlags & FOR_FILESET ) {

                                if (( hFH >= 0 ) || ( is_file( source ))) {
                                        strcpy( gszCmdline, source );
                                        goto GetFileTokens;
                                } else if ( source[0] == '"' ) {
                                        // parse a string, not a file
                                        // strip leading/trailing "
                                        strcpy( gszCmdline, source );
                                        StripQuotes( gszCmdline );
                                        source[0] = '\0';
                                        goto GetStringTokens;
                                } else
                                        break;

                        } else if (( source[0] == '@' ) && (( hFH >= 0 ) || ( QueryIsDevice( source+1 )) || (( is_file( source+1 )) && ( is_file( source ) == 0 )))) {

GetFileTokens:
                                if ( hFH < 0 ) {

                                    // get FOR argument from next line in file
                                    if ( gszCmdline[0] == '\0' )
                                        strcpy( gszCmdline, source+1 );

                                    if ( QueryIsCON( gszCmdline ))
                                        hFH = STDIN;
                                    else {

                                        if ( _stricmp( gszCmdline, CLIP ) == 0 ) {
                                                RedirToClip( gszCmdline, 0 );
                                                CopyFromClipboard( gszCmdline );
                                        } else {
                                                mkfname( gszCmdline, 0 );
                                        }

                                        if (( hFH = _sopen( gszCmdline,(O_RDONLY | O_BINARY), SH_DENYWR)) < 0 ) {
                                                rval = error( _doserrno, gszCmdline );
                                                goto for_bye;
                                        }
                                    }
                                }

                                if ( getline( hFH, gszCmdline, CMDBUFSIZ-1, EDIT_DATA ) <= 0 ) {
                                        // end of file - close & get next arg
                                        if ( hFH != STDIN )
                                                _close( hFH );
                                        hFH = -1;
                                        break;
                                }

                                // skipping first few lines?
                                if ( nSkip > 0 ) {
                                        nSkip--;
                                        continue;
                                }

                                // parse the selected tokens
                                if ( fForFlags & FOR_FILESET ) {
GetStringTokens:
                                    // check for defined EOL character
                                    if (( nEOL != 0 ) && (( arg = strchr( gszCmdline, nEOL )) != NULL ))
                                        *arg = '\0';

                                    for ( i = 0; (( pszTokenArg = ntharg( szTokens, i )) != NULL ); i++ ) {

                                        if (( n = sscanf( pszTokenArg, "%d-%d", &nStart, &nEnd )) == 1 )
                                                nEnd = nStart;
                                        else if (( n == 0 ) || ( strlen( var ) > 3 ) || ( nEnd < nStart ))
                                                goto for_usage_error;

                                        arg = malloc( strlen( gszCmdline ) + strlen(var) + 3 );
                                        strcpy( arg, gszCmdline );
                                        GetToken( arg, szDelims, nStart, nEnd );

                                        // create new environment vars to expand
                                        strins( arg, var );
                                        arg[1] += i;
                                        add_variable( arg );
                                        free( arg );
                                        nLastVariable = i;
                                    }
                                }

                        } else {
                                // copy from "arg" so we can get the entire
                                //   argument if > MAXFILENAME
                                strcpy( gszCmdline, arg );
                                source[0] = '\0';
                        }

                        if (( fForFlags & FOR_FILESET ) == 0 ) {
                                // create new environment variables to expand
                                strins( gszCmdline, var );
                                add_variable( gszCmdline );
                        }

                        // execute the command
                        rval = command( line, 2 );
                }

                if ( fForFlags & FOR_STEP ) {
                        if (( nStep > 0 ) ? ( nStart >= nEnd ) : ( nStart <= nEnd ))
                                break;
                        nStart += nStep;
                }
        }

for_bye:
        if ( hFH > 0 ) {
                _close( hFH );
                hFH = -1;
        }

        // delete the FOR variable(s)
        for ( i = 0, n = var[1]; ( i <= nLastVariable ); i++ ) {
                var[1] = (char)( n + i );
                add_variable( var );
        }
        // disable signal handling momentarily
        HoldSignals();

        return rval;
}


// Call subroutine (GOSUB labelname )
int gosub_cmd( int argc, char **argv )
{
        LONG offset, lSaveFlags;
        unsigned int uSavedLine;

        // save current position to RETURN to
        offset = bframe[ cv.bn ].offset;
        uSavedLine = bframe[ cv.bn ].uBatchLine;

        // call the subroutine
        if (( argc = goto_label( argv[1], 1 )) == 0 ) {

                // save the DO / IFF flags state
                lSaveFlags = cv.f.lFlags;

                // clear the DO / IFF flags
                cv.f.lFlags = 0L;

                bframe[cv.bn].gsoffset++;
                if (( BatchCLI() == CTRLC ) || ( cv.exception_flag ))
                        bframe[ cv.bn ].offset = -1L;

                else if ( bframe[ cv.bn ].offset >= 0L ) {

                        // restore the saved file read offset
                        bframe[cv.bn].offset = offset;
                        bframe[cv.bn].uBatchLine = uSavedLine;

                        // restore the DO / IFF flags
                        cv.f.lFlags = lSaveFlags;
                }

                // RETURN may have set an errorlevel
                argc = gnInternalErrorLevel;
        }

        return argc;
}


// GOTO a label in the batch file
int goto_cmd( int argc, char **argv )
{
        int rval = ABORT_LINE;
        long fGoto;

        // check if they want to stay within an IFF or DO
        GetSwitches( argv[1], "I", &fGoto, 1 );

        // position file pointer & force bad return to abort multiple cmds
        if (( goto_label( argv[1], 1 ) == 0 ) && ( fGoto != 1 )) {
                // turn off DO & IFF flag when you do a GOTO unless a /I
                //   was specified
                if ( cv.f.flag._do )
                        rval = BATCH_RETURN;
                cv.f.lFlags = 0L;
        }

        return rval;
}


// position the file pointer following the label line
static int goto_label(char *label, int error_flag)
{
        static char GOTO_DELIMS[] = "+=:[/";
        char *ptr;
        char line[CMDBUFSIZ];
        int nLength;
        int nPass;

        // GOSUB / GOTO must have an argument!
        if ( label == NULL )
                return USAGE_ERR;

        // make sure the file is still open
        if ( open_batch_file() == 0 )
                return BATCH_RETURN;

        // CMD.EXE in OS/2 and NT search to end, then start at beginning!!

        // skip a leading @
        if ( *label == '@' )
                label++;

        // skip a leading ':' in label name
        if ( *label == ':' )
                label++;

        // remove a trailing :, =, and/or +
        strip_trailing( label, GOTO_DELIMS );
        nLength = strlen( label );

        nPass = 1;
SearchFromTop:

        // scan for the label
        while ( getline( bframe[cv.bn].bfd, line, CMDBUFSIZ-1, EDIT_DATA ) > 0 ) {

                bframe[cv.bn].uBatchLine++;

                // strip leading white space
                ptr = skipspace( line );
                if (( *ptr++ == ':' ) && ( *ptr != ':' )) {

                        for ( ; (( *ptr == ' ' ) || ( *ptr == '\t' ) || ( *ptr == '=' )); ptr++ )
                                ;

                        strip_trailing( ptr, " \t," );
                        if (( _strnicmp( label, ptr, nLength ) == 0 ) && (( ptr[nLength] == '\0' ) || ( iswhite(ptr[nLength]))))
                                return 0;
                }
        }

        nPass++;
        if ( nPass == 2 ) {
                // rewind file for search from top
                if ( bframe[cv.bn].bfd != IN_MEMORY_FILE )
                        RewindFile( bframe[cv.bn].bfd );
                bframe[ cv.bn ].offset = 0L;
                bframe[ cv.bn ].uBatchLine = 0;
                goto SearchFromTop;
        }

        // (for NT 4.0) - if label is :EOF, and no matching label is found,
        //   goto end & exit
        if ( stricmp( label, "EOF" ) == 0 )
                return 0;

        return ((error_flag) ? error( ERROR_4DOS_LABEL_NOT_FOUND, label ) : ERROR_EXIT );
}


// conditional IF command (including ELSE/ELSEIFF)
int if_cmd( int argc, char **argv )
{
        char *line;
        int fIff = 0, condition;

        // test the command name (IF or IFF)
        if ( gpInternalName[2] == 'F' )
                fIff = 1;

        if ( argc < 3 ) {
if_usage:
                return ( usage( fIff ? IFF_USAGE : IF_USAGE ));
        }

        line = _alloca( strlen( argv[1] ) + 1 );
        strcpy( line, argv[1] );

        // test condition & strip condition text from line
        if ((( condition = TestCondition( line, 1 )) == -USAGE_ERR ) || ( *line == '\0' ))
                goto if_usage;

        // IFF / THEN / ELSE support
        if (( fIff ) && ( _stricmp( THEN, first_arg( line )) == 0 )) {

                // increment IFF nesting flag
                cv.f.flag._iff++;

                // if condition is false, flag as "waiting for ELSE"
                if ( condition == 0 )
                        cv.f.flag._else++;

                // skip THEN & see if there's another argument
                next_arg( line, 1 );
                if ( *line == '\0' )
                        return 0x666;
        }

        // support (undocumented) CMD.EXE "IF condition (...) ELSE (...)" syntax
        if ( *line == '(' ) {

                char *argptr;

                argptr = scan( line, ")", QUOTES_PARENS );

                if ((( argptr = first_arg( argptr + 1 )) != NULL ) && ( _stricmp( argptr, "ELSE" ) == 0 )) {

                        // if first case is true, squash the ELSE
                        //   otherwise, set condition & remove the first case
                        if ( condition )
                                *gpNthptr = '\0';
                        else
                                strcpy( line, gpNthptr + 5 );

                        // force execution of the command
                        condition = 1;
                }
        }

        // if condition is true, execute the specified command
        if ( condition )
                return ( command( line, 2 ));

        // return non-zero for conditional tests (&& / ||)
        return (( fIff ) ? 0x666 : ERROR_EXIT );
}


// expand the argument
static char * ExpandArgument( char *line, int argcount )
{
        char buffer[CMDBUFSIZ];

        fNoComma = 1;
        line = ntharg( line, argcount | 0x6000 );
        fNoComma = 0;

        if ( line == NULL )
                return NULL;

        // do variable expansion
        strcpy( buffer, line );
        if ( var_expand( buffer, 1 ) != 0 )
                return NULL;

        // strip escape characters
        EscapeLine( buffer );

        // put argument into "line" (static buffer in ntharg())
        strcpy( line, skipspace( buffer ) );
        strip_trailing( line, WHITESPACE );

        return line;
}


// DO / IF / IFF testing
int TestCondition( char *line, int fIf )
{
        int condition;
        char *argptr;
        char *arg2ptr, *tptr;
        int argcount = 0, or_flag = 0, and_flag = 0, xor_flag = 0xFF;
        int notflag;
        char szBuf[128];

        // KLUDGE for compatibility with things like "if %1/==/ ..."
        //   (DO & IF[F] don't have any switches anyway)
        gpIniptr->SwChr = 0x7F;

        for ( ; ; ) {

            // (re )set result condition
            condition = notflag = 0;

            if (( argptr = ExpandArgument( line, argcount )) != NULL ) {

                // check for NOT condition
                if ( _stricmp( argptr, IF_NOT ) == 0 ) {
                        notflag = 1;
                        argcount++;
                        argptr = ExpandArgument( line, argcount );
                }
            }

            argcount++;
            if ( argptr == NULL )
                return -USAGE_ERR;

            // if ( "a"=="%a" .and. "%b"=="b" ) .or. .....
            if ( *argptr == '(' ) {
                if (( stristr( argptr, " .or. " ) != NULL ) || ( stristr( argptr, " .and. " ) != NULL ) || (stristr( argptr, " .xor. " ) != NULL )) {
                        // strip leading & trailing ( )
                        arg2ptr = strend( argptr ) - 1;
                        if ( *arg2ptr == ')' )
                                *arg2ptr = '\0';
                        strcpy( argptr, argptr + 1 );
                        tptr = _alloca( strlen( argptr ) + 1 );
                        argptr = strcpy( tptr, argptr );
                        condition = TestCondition( argptr, 0 );
                        goto Conditionals;
                }
            }

            tptr = _alloca( strlen( argptr ) + 1 );
            argptr = strcpy( tptr, argptr );
            if ((( arg2ptr = ExpandArgument( line, argcount )) == NULL ) && ( fIf & 1 ))
                return -USAGE_ERR;

            if ( arg2ptr == NULL )
                arg2ptr = NULLSTR;

            argcount++;

            if ( _stricmp( argptr, IF_EXIST ) == 0 ) {

                // check for existence of specified file
                // OS/2 & NT also look for directories
                condition = ( is_file_or_dir( arg2ptr ) != 0 );

            } else if ( _stricmp( argptr, IF_ISINTERNAL ) == 0 ) {

                // check for existence of specified internal command
                condition = ( findcmd( arg2ptr, 0 ) >= 0 );

            } else if ( _stricmp( argptr, IF_ISALIAS ) == 0 ) {

                // check for existence of specified alias
                condition = ( get_alias( arg2ptr ) != 0L );

            } else if (( _stricmp( argptr, IF_ISDIR ) == 0 ) || ( _stricmp( argptr, IF_DIREXIST ) == 0 )) {

                // check ready state & existence of specified directory
                //   ("if direxist" is for DR-DOS compatibility)
                condition = is_dir( arg2ptr );

            } else if ( _stricmp( argptr, IF_ISLABEL ) == 0 ) {

                // check existence of specified batch label
                if ( cv.bn >= 0 ) {

                    long save_offset;
                    int uSavedLine;

                    save_offset = bframe[cv.bn].offset;
                    uSavedLine = bframe[cv.bn].uBatchLine;
                    tptr = _alloca( strlen( arg2ptr ) + 1 );
                    arg2ptr = strcpy( tptr, arg2ptr );
                    condition = ( goto_label( arg2ptr, 0 ) == 0 );
                    bframe[cv.bn].uBatchLine = uSavedLine;
                    bframe[cv.bn].offset = save_offset;

                    // reset file pointer
                    if ( bframe[cv.bn].bfd != IN_MEMORY_FILE )
                        _lseek( bframe[cv.bn].bfd, save_offset, SEEK_SET );
                }

            } else if ( _stricmp( argptr, IF_DEFINED ) == 0 ) {
                // check for existence of specified variable
                condition = ( get_variable( arg2ptr ) != 0L );

            } else {

                // check for a "=" or "==" in the middle of the first argument
                if (( tptr = scan( argptr, "=", QUOTES )) == BADQUOTES )
                        return -USAGE_ERR;

                if (( *arg2ptr == '=' ) || ( *tptr == '\0' )) {

                        // DO test with no argument ("DO WHILE a")
                        if ( *arg2ptr == '\0' )
                                return -USAGE_ERR;

                        // kludge for STUPID people who do "if "%1==a" ..."
                        if (( fIf & 1 ) && ( *argptr == '"' ) && ( *arg2ptr != '=' ) && ( tptr[-1] == '"' ) && (( tptr = strstr( argptr, "==" )) != NULL )) {

                                argptr++;
                                strip_trailing( argptr, "\"" );
                                arg2ptr = NULLSTR;

                        } else {

                                // arg2ptr must be test type; save it to tptr
                                tptr = _alloca( strlen( arg2ptr ) + 1 );
                                strcpy( tptr, arg2ptr );
                                arg2ptr = NULL;
                        }
                }

                if ( *tptr == '=' ) {

                        // change a "==" to a "EQ"
                        while ( *tptr == '=' )
                                *tptr++ = '\0';

                        if (( *tptr ) || ( arg2ptr == NULLSTR )) {
                                if ( arg2ptr != NULL )
                                        argcount--;
                                arg2ptr = tptr;
                        }
                        tptr = EQ;
                }

                // we have to support various ERRORLEVEL constructs:
                //   if ERRORLEVEL 5 ; if ERRORLEVEL == 5 ; if ERRORLEVEL LT 5
                if (( _stricmp( argptr, IF_ERRORLEVEL ) == 0 ) && ( is_signed_digit(*tptr))) {

                        arg2ptr = tptr;
                        tptr = GE;

                } else if ( arg2ptr == NULL ) {

                        // have we got the second argument yet?
                        if (( arg2ptr = ExpandArgument( line, argcount )) == NULL )
                                return -USAGE_ERR;
                        argcount++;
                }

                // check error return of previous command or do string match
                if ( _stricmp( argptr, IF_ERRORLEVEL ) == 0 ) {
                        condition = ( gnErrorLevel - atoi( arg2ptr ));

                } else {

                        // do ASCII/numeric comparison
                        if (( QueryIsNumeric( argptr ) != 0 ) && ( QueryIsNumeric( arg2ptr ) != 0 )) {

                                // allow decimal comparisons
                                sprintf( szBuf, "%s-%s=0.8", argptr, arg2ptr );
                                evaluate( szBuf );

                                if ( szBuf[0] == '-' )
                                        condition = -1;
                                else if ( stricmp( szBuf, "0" ) != 0 )
                                        condition = 1;

                        } else
                                condition = fstrcmp( argptr, arg2ptr, 1 );
                }

                argptr = tptr;

                // modify result based on equality test
                if ( _stricmp( argptr, EQ ) == 0 )
                        condition = ( condition == 0 );
                else if ( _stricmp( argptr, GT ) == 0 )
                        condition = ( condition > 0 );
                else if ( _stricmp( argptr, GE ) == 0 )
                        condition = ( condition >= 0 );
                else if ( _stricmp( argptr, LT ) == 0 )
                        condition = ( condition < 0 );
                else if ( _stricmp( argptr, LE ) == 0 )
                        condition = ( condition <= 0 );

                else if (( _stricmp( argptr, NE ) == 0 ) || ( _stricmp( argptr, "!=" ) == 0 ))
                        condition = ( condition != 0 );
                else
                        return -USAGE_ERR;
            }

Conditionals:
            condition ^= notflag;

            // if OR_FLAG != 0, a successful OR test is in progress
            //   force condition to TRUE
            if ( or_flag ) {
                condition = 1;
                or_flag = 0;
            }

            // if XOR_FLAG != 0xFF, then an XOR test is in progress
            if ( xor_flag != 0xFF ) {
                condition = ( condition ^ xor_flag );
                xor_flag = 0xFF;
            }

            // if AND_FLAG != 0, an unsuccessful AND test is in progress
            //   force condition to FALSE
            if ( and_flag ) {
                condition = 0;
                and_flag = 1;
            }

            // check for conditionals
            fNoComma = 1;
            argptr = ntharg( line, argcount | 0x6000 );
            fNoComma = 0;

            if ( argptr == NULL )
                return (( fIf & 1 ) ? -USAGE_ERR : condition );

            // conditional OR
            if ( _stricmp( IF_OR, argptr ) == 0 ) {

                // We need to keep parsing the line, even if the last test was
                //   true (for IFF).  Save the condition in OR_FLAG.
                or_flag = condition;
                and_flag = xor_flag = 0;
                argcount++;

            // exclusive OR
            } else if ( _stricmp( IF_XOR, argptr ) == 0 ) {

                // Continue parsing the line to get the second test value
                xor_flag = condition;
                and_flag = or_flag = 0;
                argcount++;

            // conditional AND
            } else if ( _stricmp( IF_AND, argptr ) == 0 ) {

                // We need to keep parsing the line, even if the last test was
                //   false (for IFF).  Save the inverse condition in AND_FLAG.
                and_flag = ( condition == 0 );
                or_flag = xor_flag = 0;
                argcount++;

            } else {

                // remove the conditional test(s) & return the result
                if ( fIf & 1 ) {
                        fNoComma = 1;
                        next_arg( line, argcount | 0x6000 );
                        fNoComma = 0;
                }

                return condition;
            }
        }
}


// check for IFF THEN / ELSE / ENDIFF condition
//   (if waiting for ENDIFF or ELSE, ignore the command)
int iff_parsing( char *cmd_name, char *line )
{
        // extract the command name (first argument)
        // do it first so we don't have to duplicate it in do_parsing()
        DELIMS[4] = gpIniptr->CmdSep;
        DELIMS[5] = gpIniptr->SwChr;
        sscanf( cmd_name, DELIMS, cmd_name );

        // if not parsing an IFF just return
        // if waiting on a DO/END then don't test for IFF
        if (( cv.f.flag._iff == 0 ) || ( cv.f.flag._do_end | cv.f.flag._do_leave ))
                return 0;

        // terminated IFF tests?
        if ( _stricmp( cmd_name, ENDIFF ) == 0 ) {

                // drop one ENDIFF wait
                if ( cv.f.flag._endiff == 0 ) {
                        cv.f.flag._iff--;
                        cv.f.flag._else = 0;
                } else
                        cv.f.flag._endiff--;

                return 1;

        } else if ( _stricmp( cmd_name, IFF ) == 0 ) {

                // if "waiting for ELSE" or "waiting for ENDIFF" flags set,
                //   ignore nested IFFs
                if ( cv.f.flag._else )
                        cv.f.flag._endiff++;

        } else if (( cv.f.flag._endiff == 0 ) && (( _stricmp( cmd_name,ELSE) == 0 ) || ( _stricmp( cmd_name,ELSEIFF ) == 0 ))) {

                // are we waiting for an ELSE[IFF]?
                if ( cv.f.flag._else & 0x7F ) {

                        // remove an ELSE or make an ELSEIFF look like an IFF
                        cv.f.flag._else = 0;
                        strcpy( line, line+4 );

                        // an ELSEIFF is an implicit ENDIFF
                        if ( _stricmp( cmd_name, ELSEIFF ) == 0 )
                                cv.f.flag._iff--;

                        // if an ELSE doesn't have a ^, add one
                        else if (( *(line = skipspace( line )) != gpIniptr->CmdSep) && (*line != '\0' )) {
                                DELIMS[5] = '\0';
                                strins( line, DELIMS+4 );
                        }

                } else
                        cv.f.flag._else = 0x80; // wait for an ENDIFF
        }

        // waiting for ELSE[IFF] or ENDIFF?
        return ( cv.f.flag._else | cv.f.flag._endiff);
}


#define INKEY_CLEAR 1
#define INKEY_KEYMASK 2
#define INKEY_PASSWORD 4
#define INKEY_WAIT 8
#define INPUT_EDIT 0x10
#define INPUT_NO_CRLF 0x20
#define INPUT_NO_COLOR 0x40
#define INPUT_LENGTH 0x80
#define INPUT_DIGITS 0x100

// assign a single-key value to a shell variable (INKEY)
// assign a string to a shell variable (INPUT)
int inkey_input_cmd( int argc, char **argv )
{
        char *argptr;
        char *arg, *arg1, keylist[MAXARGSIZ+1];
        char *fptr;
        LONG time_delay = -1L;
        int i, len, nRow, nColumn, nInputFlag, nEditFlag = EDIT_DATA;
        int nMaxLength = MAXARGSIZ;
        long lFlags;

        // is it INKEY or INPUT?
        nInputFlag = _stricmp( gpInternalName, INKEY_COMMAND );

        // get the last argument & remove it from the command line
        argptr = argv[argc-1];
        if ( *argptr != '%' ) {
input_bad:
                return ( usage((( nInputFlag) ? INPUT_USAGE : INKEY_USAGE )));
        }

        *argptr++ = '\0';
        arg = _alloca( strlen( argptr ) +1 );
        argptr = strcpy( arg, argptr );

        keylist[0] = '\0';

        // check for optional input mask & timeout (& max length for INPUT)
        for ( argc = 0, argv++; ; argv++ ) {

            if ((( arg = *argv ) == NULL ) || ( *arg != gpIniptr->SwChr ))
                break;

            while ( *arg == gpIniptr->SwChr ) {

                arg1 = first_arg( arg );
                if (( lFlags = switch_arg( arg1, (( nInputFlag ) ? "C PWEXNLD" : "CKPW X  D"))) == 0 )
                        break;

                if ( lFlags == -1L )
                        goto input_bad;

                // flush the typeahead buffer?
                if ( lFlags & INKEY_CLEAR ) {
                        while ( _kbhit() )
                                (void)GetKeystroke( EDIT_NO_ECHO );
                }

                if ( lFlags & INKEY_PASSWORD )
                        nEditFlag |= EDIT_PASSWORD;

                if ( lFlags & INPUT_EDIT )
                        nEditFlag |= EDIT_ECHO;

                // save the key mask set onto the stack
                if ( lFlags & INKEY_KEYMASK )
                        sprintf( keylist, FMT_PREC_STR, MAXARGSIZ, arg+2 );
                else if ( lFlags & INPUT_LENGTH ) {     // INPUT length
                        sscanf( arg+2, FMT_UINT, &nMaxLength );
                        if ( gpIniptr->LineIn )
                                nMaxLength++;
                } else if ( lFlags & INKEY_WAIT )       // timeout delay
                        sscanf( arg+2, FMT_ULONG, &time_delay );
                else {
                        if ( lFlags & INPUT_NO_CRLF )
                                nEditFlag |= EDIT_NO_CRLF;
                        if ( lFlags & INPUT_NO_COLOR )
                                nEditFlag |= EDIT_NO_INPUTCOLOR;
                        if ( lFlags & INPUT_DIGITS )
                                nEditFlag |= EDIT_DIGITS;
                }

                // skip switch
                arg += strlen( arg1 );
            }
        }

        // print the (optional) prompt string
        if (( *argv != NULL ) && ( **argv ))
                qputs( *argv );

        len = sprintf( gszCmdline, "%.80s=", argptr );

        // allow editing an existing value?
        if (( nInputFlag ) && ( nEditFlag & EDIT_ECHO )) {
                if (( fptr = get_variable( argptr )) != 0L )
                        sprintf( gszCmdline + len, FMT_FAR_PREC_STR, MAXARGSIZ, fptr );
                // save cursor position, display value, and restore cursor
                GetCurPos( &nRow, &nColumn );
                if (( i = gpIniptr->InputColor ) == 0 )
                        i = -1;
                color_printf( i, FMT_STR, gszCmdline + len );
                SetCurPos( nRow, nColumn );
        }

        // wait the (optionally) specified length of time
        if ( time_delay >= 0L ) {

                for ( time_delay *= 18; ; time_delay-- ) {
continue_wait:
                        // is a key waiting?
                        if ( _kbhit() )
                                break;

                        if ( time_delay <= 0L ) {
                                if (( *argv != NULL ) && ( **argv ))
                                        crlf();
                                return 0;
                        }
                        SysBeep( 0, 1 );        // wait 1/18th second
                }
        }

        // get the variable
        if ( nInputFlag )
                getline( STDIN, gszCmdline+len, nMaxLength, nEditFlag );
        else {

                for ( ; ; ) {

                        // if EOF, we're redirecting & ran out of input
                        if (( argc = GetKeystroke( EDIT_NO_ECHO )) == EOF )
                                return ERROR_EXIT;

                        // if printable character, display it
                        if ((( nEditFlag & EDIT_PASSWORD) == 0 ) && ( argc >= 32 ) && ( argc < 0xFF ))
                                qputc( STDOUT, ( char)argc );

                        if ( nEditFlag & EDIT_DIGITS ) {
                                if ( isdigit( argc ))
                                        break;
                        } else if ( keylist[0] == '\0' )
                                break;

                        // check the key against the optional mask
                        for ( arg = keylist+1; (( *arg != '"' ) && ( *arg != '\0' )); arg++ ) {

                                // check for extended keys
                                if ( *arg == '[' ) {

                                        // can't use sscanf() because of []'s
                                        arg++;
                                        i = (int)( scan( arg, "]\"", QUOTES ) - arg );

                                        if ( argc == keyparse( arg, i ))
                                                goto good_key;

                                        arg += i;

                                } else if ( _ctoupper( argc ) == _ctoupper( *arg ))
                                        goto good_key;
                        }

                        // if printable character, back up over it
                        if ((( nEditFlag & EDIT_PASSWORD ) == 0 ) && ( argc >= 32 ) && ( argc < 0xFF ))
                                qputc( STDOUT, BS );

                        honk();

                        if ( time_delay > 0L )
                                goto continue_wait;
                }
good_key:
                // kludge for ENTER - set to @28
                if ( argc == 13 )
                        argc = 0x100 + 28;

                sprintf( gszCmdline + len,(( argc <= 0xFF ) ? FMT_CHAR : "@%u" ), ( argc & 0xFF ));
                if (( nEditFlag & EDIT_NO_CRLF ) == 0 )
                        crlf();
        }

        return ( add_variable( gszCmdline ));
}


// switch a file between .BAT and .BTM types
int loadbtm_cmd( int argc, char **argv)
{
        if ( argc == 1 ) {

                // just inquiring about LOADBTM state
                printf( LOADBTM_IS, ( bframe[cv.bn].bfd == IN_MEMORY_FILE ) ? ON : OFF );

        } else if (( argc = OffOn( argv[1] )) == 1 ) {

                // switch batch file to .BTM mode
                if ( bframe[cv.bn].bfd != IN_MEMORY_FILE )
                        return (load_btm());

        } else if ( argc == 0 ) {

                // switch batch file to .BAT mode (if not compressed)
                if (( bframe[cv.bn].bfd == IN_MEMORY_FILE ) && (( bframe[cv.bn].flags & BATCH_COMPRESSED) == 0 )) {

                        // turn off the IN_MEMORY_FILE flag
                        bframe[cv.bn].bfd = -1;

                        // free the internal .BTM buffer
                        FreeMem( bframe[cv.bn].in_memory_buffer );
                        bframe[cv.bn].in_memory_buffer = NULL;
                }

        } else
                return ( usage( LOADBTM_USAGE ));

        return 0;
}


// ON [BREAK | ERROR | ERRORMSG | ...] condition
int on_cmd( int argc, char **argv )
{
        char *arg;

        if (( arg = first_arg( argv[1] )) != NULL ) {

            if ( _stricmp( arg, ON_BREAK ) == 0 ) {

                if ( bframe[cv.bn].OnBreak != NULL )
                        free( bframe[cv.bn].OnBreak );
                bframe[cv.bn].OnBreak = (( argv[2] == NULL ) ? argv[2] : _strdup( argv[2] ));
                return 0;

            } else if ( _stricmp( arg, ON_ERROR ) == 0 ) {

                // clear previous condition
                if ( bframe[cv.bn].OnError != NULL ) {

                        free( bframe[cv.bn].OnError );

                        // reset previous error popup state
                        DosError( bframe[ cv.bn ].OnErrorState );
                }

                // set OnError condition & disable error popups
                if (( bframe[ cv.bn ].OnError = (( argv[2] == NULL ) ? argv[2] : _strdup( argv[2]))) != NULL ) {
                        bframe[ cv.bn ].OnErrorState = (( cv.bn > 0 ) ? bframe[cv.bn-1].OnErrorState : FERR_ENABLEHARDERR);
                        DosError( FERR_DISABLEHARDERR );
                }

                return 0;

            } else if ( _stricmp( arg, ON_ERRORMSG ) == 0 ) {

                // clear previous condition
                if ( bframe[ cv.bn ].OnErrorMsg != NULL )
                        free( bframe[ cv.bn ].OnErrorMsg );

                // set OnErrorMsg condition
                bframe[ cv.bn ].OnErrorMsg = (( argv[2] == NULL ) ? argv[2] : _strdup( argv[2] ));
                return 0;
            }
        }

        return ( usage( ON_USAGE ) );
}


// wait for a keystroke; display optional message
int pause_cmd( int argc, char **argv )
{
        // make sure STDOUT hasn't been redirected
        qprintf( ( _isatty( STDOUT ) ? STDOUT : STDERR ), FMT_STR, (( argc == 1 ) ? PAUSE_PROMPT : argv[1] ));

        (void)GetKeystroke( EDIT_NO_ECHO | EDIT_ECHO_CRLF );
        return 0;
}


// exit batch file or cancel nested batch files
//   NOTE: you can also use this command to set the ERRORLEVEL (even from
//   the command line!)
int quit_cmd( int argc, char **argv )
{
        if ( cv.bn >= 0 ) {

                // QUIT a batch file or CANCEL nested batch file sequence
                //   by positioning the file pointer in each file to EOF
                for ( argc = (( _ctoupper(**argv) == 'C' ) ? 0 : cv.bn); ( argc <= cv.bn); argc++)
                        bframe[argc].offset = -1L;
        }

        // set ERRORLEVEL
        if ( argv[1] != NULL ) {
                gnErrorLevel = atoi( argv[1] );
                if ( cv.bn >= 0 )
                        bframe[cv.bn].nReturn = gnErrorLevel;
        }

        // Abort the remainder of the batch file (or alias)
        return ABORT_LINE;
}


// REM - do nothing
int remark_cmd( int argc, char **argv )
{
        return 0;
}


// RETURN from a GOSUB
int ret_cmd( int argc, char **argv )
{
        // check for a previous GOSUB
        if ( bframe[cv.bn].gsoffset <= 0 )
                return ( error( ERROR_4DOS_BAD_RETURN, NULL ));

        bframe[cv.bn].gsoffset--;

        // set (optional) return value
        gnInternalErrorLevel = (( argv[1] != NULL ) ? atoi( argv[1] ) : 0 );

        // return a "return from nested BatchCLI()" code
        return BATCH_RETURN_RETURN;
}


// set cursor position
int scr_cmd( int argc, char **argv )
{
        int row, column;

        // make sure row & column are displayable on the current screen
        if (( argc < 3 ) || ( GetCursorRange( argv[1], &row, &column ) != 0 ))
                return ( usage( SCREEN_USAGE ));

        if ( argv[3] != NULL ) {

            if ( column == 999 ) {
                if (( column = (( GetScrCols() - strlen( argv[3] )) / 2 )) < 0 )
                        column = 0;
            }

            if ( row == 999 ) {
                if (( row = ( GetScrRows() / 2 )) < 0 )
                        row = 0;
            }
        }

        // move the cursor
        SetCurPos( row, column );

        // if the user specified a string, display it
        if ( argv[3] != NULL )
                qputs( argv[3] );

        return 0;
}


// set screen position and print a string with the specified attribute
//   directly to the screen (either horizontally or vertically)
int scrput_cmd( int argc, char **argv )
{
        char *pszText;
        int attribute = -1;
        int row, column, fVscrput = 0;

        // make sure row & column are displayable on the current screen
        if (( argc > 5 ) && ( GetCursorRange( argv[1], &row, &column ) == 0 )) {
                pszText = argv[3];
                attribute = GetColors( pszText, 0 );
        }

        if ( attribute == -1 )
                return ( usage( SCRPUT_USAGE ));

        // strip any quotes & process escape characters
        // We do it here to keep GetColors from removing whitespace around
        //   the text argument!
        parse_line( NULL, pszText, NULL, CMD_STRIP_QUOTES );

        fVscrput = ( *gpInternalName == 'V' );

        if ( column == 999 ) {
                column = ( GetScrCols() - (( fVscrput ) ? 0 : strlen( pszText ))) / 2;
                if ( column < 0 )
                        column = 0;
        }

        if ( row == 999 ) {
                row = ( GetScrRows() - (( fVscrput ) ? strlen( pszText ) : 0 )) / 2;
                if ( row < 0 )
                        row = 0;
        }

        // write the string with the specified attribute
        // first, check for VSCRPUT (vertical write )
        if ( fVscrput )
                WriteVStrAtt( row, column, attribute, pszText );
        else
                WriteStrAtt( row, column, attribute, pszText );

        return 0;
}


// save the current environment, alias list, and current drive/directory
//   (restored by ENDLOCAL or end of batch file )
int setlocal_cmd( int argc, char **argv )
{
        return ( _setlocal() );
}


// restore the disk, directory, and environment (saved by SETLOCAL)
int endlocal_cmd( int argc, char **argv )
{
        return ( _endlocal() );
}


static int _setlocal( void )
{
        // check for environment already saved
        if ( bframe[ cv.bn ].local_env != 0L )
                return ( error( ERROR_4DOS_ENV_SAVED, NULL ));

        // save parameter & separator & escape characters
        bframe[ cv.bn ].cLocalParameter = gpIniptr->ParamChr;
        bframe[ cv.bn ].cLocalEscape = gpIniptr->EscChr;
        bframe[ cv.bn ].cLocalSeparator = gpIniptr->CmdSep;
        bframe[ cv.bn ].cLocalDecimal = gpIniptr->DecimalChar;
        bframe[ cv.bn ].cLocalThousands = gpIniptr->ThousandsChar;

        // get environment and alias list sizes
        // disable signal handling momentarily
        HoldSignals();

        bframe[ cv.bn ].local_env_size = (UINT)(end_of_env( glpEnvironment ) - glpEnvironment ) + 1;
        bframe[ cv.bn ].local_alias_size = (UINT)(end_of_env( glpAliasList ) - glpAliasList ) + 1;

        // save the environment & alias list
        if ((( bframe[ cv.bn ].local_env = AllocMem((UINT *)(&( bframe[cv.bn].local_env_size )))) == 0L ) || (( bframe[cv.bn].local_alias = AllocMem((UINT *)(&( bframe[cv.bn].local_alias_size )))) == 0L ))
                return (error( ERROR_NOT_ENOUGH_MEMORY, NULL ));

        memmove( bframe[ cv.bn ].local_env, glpEnvironment, bframe[cv.bn].local_env_size );
        memmove( bframe[ cv.bn ].local_alias, glpAliasList, bframe[cv.bn].local_alias_size );

        EnableSignals();

        bframe[ cv.bn ].local_dir = _strdup( gcdir( NULL, 1 ) );
        return 0;
}


static int _endlocal( void )
{
        int rval = 0;

        // check for missing SETLOCAL (environment not saved)
        if (( bframe[ cv.bn ].local_env == 0L ) || ( bframe[ cv.bn ].local_alias == 0L ))
                return ( error( ERROR_4DOS_ENV_NOT_SAVED, NULL ));

        // restore the environment & alias list
        memmove( glpEnvironment, bframe[ cv.bn ].local_env, bframe[ cv.bn ].local_env_size );
        memmove( glpAliasList, bframe[ cv.bn ].local_alias, bframe[ cv.bn ].local_alias_size );

        FreeMem( bframe[ cv.bn ].local_env );
        FreeMem( bframe[ cv.bn ].local_alias );
        bframe[ cv.bn ].local_env = bframe[ cv.bn ].local_alias = NULL;
        bframe[ cv.bn ].local_env_size = bframe[ cv.bn ].local_alias_size = 0;

        // restore parameter & separator & escape characters
        gpIniptr->ParamChr = bframe[ cv.bn ].cLocalParameter;
        gpIniptr->EscChr = bframe[ cv.bn ].cLocalEscape;
        gpIniptr->CmdSep = bframe[ cv.bn ].cLocalSeparator;
        gpIniptr->DecimalChar = bframe[ cv.bn ].cLocalDecimal;
        gpIniptr->ThousandsChar = bframe[ cv.bn ].cLocalThousands;

        QueryCountryInfo();

        EnableSignals();

        // restore the original drive and directory
        if ( bframe[ cv.bn ].local_dir != 0 ) {
                rval = __cd( bframe[ cv.bn ].local_dir, CD_CHANGEDRIVE | CD_NOFUZZY | CD_NOERROR );
                free( bframe[ cv.bn ].local_dir );
                bframe[ cv.bn ].local_dir = 0;
        }

        return rval;
}


// shift the batch arguments upwards (-n) or downwards (+n)
int shift_cmd( int argc, char **argv )
{
        // get shift count
        if ( argc > 1 ) {

                // collapse starting at a particular argument
                if ( argv[1][0] == '/' ) {
                        argc = atoi( argv[1] + 1 );
                        for ( ; ( bframe[ cv.bn ].Argv[argc+1] != NULL ); argc++ )
                                bframe[ cv.bn ].Argv[argc] = bframe[ cv.bn].Argv[argc+1];
                        bframe[ cv.bn ].Argv[argc] = NULL;
                        return 0;
                }

                argc = atoi( argv[1] );
        }

        // make sure we don't shift past the Argv[] boundaries
        for ( ; (( argc < 0 ) && ( bframe[ cv.bn ].Argv_Offset > 0 )); bframe[ cv.bn ].Argv_Offset--, argc++ )
                ;

        for ( ; (( argc > 0 ) && ( bframe[ cv.bn ].Argv[bframe[ cv.bn ].Argv_Offset] != NULL )); bframe[ cv.bn ].Argv_Offset++, argc-- )
                ;

        return 0;
}


// Switch Case Statement
int switch_cmd( int argc, char **argv )
{
        extern int continued_line( char * );

        char *arg, *pszTest, *pszLine;
        int fNestingLevel = 0;

        if ( argc < 2 )
                argv[1] = NULLSTR;

        // make sure the batch file is still open
        if ( open_batch_file() == 0 )
                return BATCH_RETURN;

        // save SWITCH arguments
        pszTest = _alloca( strlen( argv[1] ) + 1 );
        strcpy( pszTest, argv[1] );

        // scan for a matching Case, Default, or the EndSwitch line
        while ( getline( bframe[cv.bn].bfd, gszCmdline, CMDBUFSIZ-1, EDIT_DATA ) > 0 ) {

                bframe[cv.bn].uBatchLine++;

                if ( continued_line( gszCmdline ) != 0 )
                        break;

                strip_leading( gszCmdline, WHITESPACE );
                strip_trailing( gszCmdline, WHITESPACE );

                // ignore nested SWITCH calls
                if ( strnicmp( gszCmdline, "switch ", 7 ) == 0 )
                        fNestingLevel++;

                else if ( stricmp( gszCmdline, "Default" ) == 0 ) {
                        if ( fNestingLevel == 0 )
                                return 0;

                } else if (( fNestingLevel == 0 ) && ( strnicmp( gszCmdline, "Case ", 5 ) == 0 )) {

                        // test condition
                        strcpy( gszCmdline, gszCmdline + 5 );
                        var_expand( gszCmdline, 1 );

                        // reformat line for TestCondition
                        strins( gszCmdline, " EQ " );
                        strins( gszCmdline, pszTest );
                        pszLine = gszCmdline;
                        while (( arg = stristr( pszLine, " .or. " )) != NULL ) {
                                pszLine = skipspace( arg + 6 );
                                strins( pszLine, " EQ " );
                                strins( pszLine, pszTest );
                        }

                        if ( TestCondition( gszCmdline, 0 ) == 1 )
                                return 0;

                } else if ( stricmp( gszCmdline, "EndSwitch" ) == 0 ) {
                        if ( fNestingLevel <= 0 )
                                return 0;
                        fNestingLevel--;
                }
        }

        return USAGE_ERR;
}


// found a trailing CASE or DEFAULT - skip everything until EndSwitch
int case_cmd( int argc, char **argv )
{
        extern int continued_line( char * );

        int fNestingLevel = 0;

        // make sure the batch file is still open
        if ( open_batch_file() == 0 )
                return BATCH_RETURN;

        // scan for the EndSwitch line
        while ( getline( bframe[cv.bn].bfd, gszCmdline, CMDBUFSIZ-1, EDIT_DATA ) > 0 ) {

                bframe[cv.bn].uBatchLine++;

                if ( continued_line( gszCmdline ) != 0 )
                        break;

                strip_leading( gszCmdline, WHITESPACE );
                strip_trailing( gszCmdline, WHITESPACE );

                // ignore nested SWITCH calls
                if ( strnicmp( gszCmdline, "switch ", 7 ) == 0 )
                        fNestingLevel++;

                else if ( stricmp( gszCmdline, "EndSwitch" ) == 0 ) {
                        if ( fNestingLevel <= 0 )
                                return 0;
                        fNestingLevel--;
                }
        }

        return USAGE_ERR;
}

