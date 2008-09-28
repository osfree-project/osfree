// PARSER.C - parsing routines for 4xxx / TCMD family
//   (c) 1988 - 1998  Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <share.h>
#include <string.h>

#include "4all.h"


extern PIPE_INHERIT *gpPipeParent;

static int ProcessCommands( char *, char * );
static int InternalCommands( char * );
static int ExternalCommands( char *, char * );
static int command_groups( char *, int );
int continued_line( char * );

static int fSetCMDLINE;
char *pszWindowTitle;

// used by %@EXEC[] in all platforms
int DoINT2E( char *cmd2e )
{
        int rval;
        CRITICAL_VARS save_cv;

        // save the critical variables
        memmove( &save_cv, &cv, sizeof(cv) );

        // ( re)initialize the critical vars
        rval = cv.bn;
        memset( &cv, '\0', sizeof(cv) );
        cv.bn = rval;

        // we have to force the CALL flag to 1 to keep from clobbering the
        //   current batch file if the program calls another batch file
        //   from within an INT 2Eh
        cv.call_flag |= 1;

        strcpy( gszCmdline, cmd2e );
        rval = command( gszCmdline, 0 );

        // restore the critical vars
        memmove( &cv, &save_cv, sizeof(cv) );

        return rval;
}


// find 4START & 4EXIT
void find_4files( char *szFileName )
{
        PIPE_INHERIT *pPipeSave;
        char *arg;
        int nSaveErrorLevel, nSaveCallFlag;
        PCH feptr;

        // first, check .INI data for FSPath (no fallback - the file
        //   HAS to be there or it will be ignored)
        if ( gpIniptr->FSPath != INI_EMPTYSTR ) {

                strcpy( gszCmdline, ( gpIniptr->StrData + gpIniptr->FSPath ));

                // force trailing backslash on path name
                mkdirname( gszCmdline, szFileName+3 );

        } else {

                // look in the COMSPEC directory
                if (( feptr = get_variable( COMSPEC )) == 0L )
                        return;

                // remove "4xxx.xxx" & replace w/ 4START or 4EXIT
                sprintf( gszCmdline, FMT_FAR_PREC_STR, MAXFILENAME-1, feptr );
                insert_path( gszCmdline, szFileName+3, gszCmdline );
        }

        // if not found in COMSPEC, look in the boot root directory
        if ((( arg = searchpaths( gszCmdline, NULL, TRUE )) == NULL ) && ( gpIniptr->FSPath == INI_EMPTYSTR ) && ( _stricmp( gszCmdline, szFileName ) != 0 ))
                arg = searchpaths( szFileName, NULL, TRUE );

        if ( arg != NULL ) {

                // check for LFN name
                AddQuotes( arg );

                if (( pszCmdLineOpts != NULL ) && ( *pszCmdLineOpts ))
                        sprintf( gszCmdline, "%s %s", arg, pszCmdLineOpts );
                else
                        strcpy( gszCmdline, arg );

                nSaveErrorLevel = gnErrorLevel;

                // treat it as a CALL to keep batch_cmd() from overwriting
                //   batch arguments inside a pipe
                nSaveCallFlag = cv.call_flag;
                cv.call_flag |= 1;

                // save pipe shared memory so it's not closed yet
                pPipeSave = gpPipeParent;
                gpPipeParent = 0L;
                ProcessCommands( gszCmdline, NULLSTR );
                gpPipeParent = pPipeSave;
                cv.call_flag = nSaveCallFlag;
                gnErrorLevel = nSaveErrorLevel;
        }
}


// batch file processor
int BatchCLI( void )
{
        int ch, rval = 0;

        // if offset == -1, we processed a CANCEL or QUIT
        for ( cv.exception_flag = 0; ( bframe[cv.bn].offset >= 0L ); ) {

                if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {

                    // check for stack overflow
                    if ( cv.exception_flag & BREAK_STACK_OVERFLOW )
                        ch = ALL_CHAR;

                    // check for "ON ERROR / ERRORMSG" request
                    else if ( cv.exception_flag & BREAK_ON_ERROR ) {

                        cv.exception_flag = 0;
                        strcpy( gszCmdline, (( bframe[cv.bn].OnError != NULL ) ? bframe[cv.bn].OnError : bframe[cv.bn].OnErrorMsg ));
                        goto BatchExecCmd;

                    } else {

                        // Reset ^C so we come back here if the user types
                        //   ^C in response to the (Y/N)? prompt
                        cv.exception_flag = 0;

                        // Got ^C internally or from an external program.

                        // check for "ON BREAK" request
                        if ( bframe[ cv.bn ].OnBreak != NULL ) {
                                strcpy( gszCmdline, bframe[cv.bn].OnBreak );
                                goto BatchExecCmd;
                        }

                        EnableSignals();

                            // console is redirected - write to display &
                            //   read from keyboard
                            sprintf( gszCmdline, CANCEL_BATCH_JOB, bframe[cv.bn].pszBatchName );
                            WriteTTY( gszCmdline );

                            for ( ; ; ) {

                                ch = GetKeystroke( EDIT_NO_ECHO | EDIT_BIOS_KEY | EDIT_UC_SHIFT);

                                if ( isprint( ch )) {
                                        WriteTTY( (char *)&ch );
                                        if (( ch == YES_CHAR ) || ( ch == NO_CHAR ) || ( ch == ALL_CHAR ))
                                                break;
                                        WriteTTY( "\b" );
                                }

                                honk();
                            }

                            WriteTTY( "\r\n" );
                    }

                    cv.exception_flag = 0;

                    if ( ch != NO_CHAR ) {

                        // CANCEL ALL nested batch files
                        //   by setting the file pointers to EOF
                        if ( ch == ALL_CHAR ) {

                                for ( ch = 0; ( ch < cv.bn ); ch++ )
                                        bframe[ch].offset = -1L;

                                // force abort in things like GLOBAL
                                cv.exception_flag = CTRLC;
                        }

                        // cancel current batch file
                        bframe[ cv.bn ].offset = -1L;
                        rval = CTRLC;

                        // clear DO / IFF parsing flags
                        cv.f.lFlags = 0L;
                        break;
                    }

                    continue;
                }

                // Reset ^C handling
                EnableSignals();

                if ( open_batch_file() == 0 )
                        break;

                // get file input
                if ( getline( bframe[cv.bn].bfd, gszCmdline, MAXLINESIZ-1, EDIT_COMMAND ) == 0 )
                        break;
                bframe[cv.bn].uBatchLine++;
BatchExecCmd:

                // parse & execute the command.  BATCH_RETURN is a special
                //   code sent by functions like GOSUB/RETURN to end recursion
                if ( command( gszCmdline, 2 ) == BATCH_RETURN )
                        break;
        }

        if ( setjmp( cv.env ) == -1 )
                rval = CTRLC;

        close_batch_file();

        // disable signal handling momentarily
        HoldSignals();

        return rval;
}


// open the batch file and seek to the saved offset
int open_batch_file( void )
{
        if (( bframe[ cv.bn ].bfd < 0 ) && (( bframe[ cv.bn ].flags & BATCH_REXX ) == 0 )) {

                // Batch file was closed (either by us or Netware)
                while (( bframe[ cv.bn ].bfd = _sopen( bframe[ cv.bn ].pszBatchName, (O_RDONLY | O_BINARY), SH_DENYWR) ) < 0 ) {

                        // if missing file is on fixed disk, assume worst & give up
                        if ( bframe[ cv.bn ].pszBatchName[0] > 'B' ) {
                                error( ERROR_4DOS_MISSING_BATCH, bframe[ cv.bn ].pszBatchName );
                                return 0;
                        }

                        qprintf( STDERR, INSERT_DISK, bframe[ cv.bn ].pszBatchName );
                        (void)GetKeystroke( EDIT_NO_ECHO | EDIT_ECHO_CRLF );
                }

                // seek to saved position
                _lseek( bframe[cv.bn].bfd, bframe[cv.bn].offset, SEEK_SET );
        }

        return 1;
}


// close the current batch file
void close_batch_file( void )
{
        // if it's not an in-memory batch file, & it's still open, close it
        if (( cv.bn >= 0 ) && ( bframe[cv.bn].bfd != IN_MEMORY_FILE ) && ( bframe[cv.bn].bfd >= 0 )) {
                (void)_close( bframe[cv.bn].bfd );
                bframe[cv.bn].bfd = -1;
        }
}


// get command line from console or file
int getline( int fd, char *line, int nMaxSize, int nEditFlag )
{
        int i;
        int fPipe = 0;

        // get STDIN input (if not redirected or CTTY'd) from egets()
        if (( fd == STDIN ) && ( QueryIsConsole( STDIN ))) {

                // KEYS OFF?
                if ( gpIniptr->LineIn ) {

                        STRINGINBUF Length;

                        Length.cb = (( nMaxSize > 255 ) ? 255 : nMaxSize );
                        Length.cchIn = 0;
                        *line = '\0';
                        (void)KbdStringIn( line, &Length, 0, 0 );
                        line[ Length.cchIn ] = '\0';
                        crlf();

                        return ( Length.cchIn );

                } else
                        return ( egets( line, nMaxSize, nEditFlag ));
        }

        // if it's an in-memory batch file, read the far buffer
        if ( fd == IN_MEMORY_FILE )
                (void)memmove( line, &(bframe[cv.bn].in_memory_buffer[bframe[cv.bn].offset]), nMaxSize );

        else {
                // check to see if it's a pipe
                if (( fPipe = QueryIsPipeHandle( fd )) != 0 ) {

                        unsigned int bytes_read;
                        int rval;

                        // we have to read anonymous pipes a byte at a
                        //   time, since we can't peek or rewind them
                        i = 0;
                        do {
                                rval = FileRead( fd, line+i, 1, &bytes_read );
                                if ( bytes_read == 0 )
                                        break;
                                if ( line[ i++ ] == '\n' )
                                        break;
                        } while (( rval == 0 ) && ( i < nMaxSize ));
                        nMaxSize = i;

                } else
                nMaxSize = _read( fd, line, nMaxSize );
        }

        // get a line and set the file pointer to the next line
        for ( i = 0; ; i++, line++ ) {

                if (( i >= nMaxSize ) || ( *line == EoF ))
                        break;

                if ( *line == '\r' ) {

                        // skip a LF following a CR or LF
                        if (( ++i < nMaxSize ) && ( line[1] == '\n' ))
                                i++;
                        break;

                } else if ( *line == '\n' ) {
                        i++;
                        break;
                }
        }

        // truncate the line
        *line = '\0';

        if ( i >= 0 ) {

                // save the next line's position
                if (( fd == IN_MEMORY_FILE ) || (( cv.bn >= 0 ) && ( fd == bframe[cv.bn].bfd )))
                        bframe[cv.bn].offset += i;
                if ( fPipe == 0 )       // only do a seek on disk files, not pipes
                if ( fd != IN_MEMORY_FILE )
                        (void)_lseek( fd, (long)( i - nMaxSize ), SEEK_CUR );
        }

        return i;
}


#define AND_CONDITION 1
#define OR_CONDITION 2

// Parse the command line, perform alias & variable expansion & redirection,
//   and execute it.
int command( char *line, int fOptions )
{
        extern char *pszBatchDebugLine;

        static int fStepOver = 0;
        char *pszStartLine;
        int i, rval = 0, fEcho = 0, fStep;
        char condition, cEoL;
        REDIR_IO redirect;

        if (( line == NULL ) || ( *line == '\0' ) || ( *line == ':' ))
                return 0;

        // clear stdin, stdout, stderr, & pipe flags
        memset( &redirect, '\0', sizeof(REDIR_IO) );

        // echo the line if at the beginning of the line, and either:
        //   1. In a batch file and ECHO is on
        //   2. VERBOSE is on
        if ( line == gszCmdline )
                fEcho = (( cv.bn >= 0 ) ? bframe[cv.bn].echo_state : cv.verbose );

        // history logging
        if (( gpIniptr->HistLogOn ) && ( fOptions & 1 ))
                (void)_log_entry( line, 1 );

        for ( ; ; ) {

                fStep = 0;

                // check for ^C or stack overflow
                if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {
                        rval = CTRLC;
                        break;
                }

                // ( re)enable ^C and ^BREAK
                EnableSignals();

                // reset the default switch character (things like IF kill it!)
                gpIniptr->SwChr = '/';

                if (( rval == ABORT_LINE) || ( rval == BATCH_RETURN ) || (( cv.bn >= 0 ) && ( bframe[cv.bn].offset < 0L )))
                        break;

                // kludge for WordPerfect Office bug ("COMMAND /C= ...")
                strip_leading( line, "= \t\r\n,;" );
                if ( *line == '\0' )
                        break;

                // get the international format chars
                QueryCountryInfo();

                // kludge to check for a leading `
                //   (for example, "if "%a" == "%b" `a1 ^ a2`)
                if ( *line == '`' ) {
                        if (( pszStartLine = scan( ++line, "`", QUOTES + 1 )) == BADQUOTES )
                                break;
                        // strip trailing `
                        if ( *pszStartLine != '\0' )
                                strcpy( pszStartLine, pszStartLine + 1 );
                }

                pszStartLine = strcpy( gszCmdline, line );

                // perform alias expansion
                if (( rval = alias_expand( pszStartLine )) != 0 )
                        break;

                // check for leading '@' (no echo)
                if (( *pszStartLine == '@' ) || ( strnicmp( pszStartLine, "*@", 2 ) == 0 )) {

                        if ( *pszStartLine == '*' )
                                strcpy( pszStartLine+1, skipspace( pszStartLine+2 ));
                        else {
                                pszStartLine++;
                                strcpy( pszStartLine, skipspace( pszStartLine ));
                        }

                        // check for "persistent echo" flag
                        fEcho &= 2;
                        fSetCMDLINE = 0;

                } else
                        fSetCMDLINE = 1;

                // skip blank lines or those beginning with REM or a label
                if (( *pszStartLine == ':' ) || (( line = first_arg( pszStartLine )) == NULL ))
                        break;

                // check for IFF / THEN / ELSEIFF / ENDIFF condition
                //   (if waiting for ENDIFF or ELSEIFF, ignore the command)
                if (( iff_parsing( line, pszStartLine ) != 0 ) || (( rval = do_parsing( line )) != 0 )) {

                        // check for DO / END condition
                        if ( rval == BATCH_RETURN )
                                break;

                        // skip this command - look for next compound command
                        //  (ignoring pipes & conditionals)
                        i = gpIniptr->CmdSep;

                        if (( line = scan( pszStartLine, (char *)&i, QUOTES_PARENS )) == BADQUOTES )
                                break;
                        if ( *line )
                                line++;
                        continue;
                }

                // check for ?"prompt" command single-stepping (Novell DOS 7)
                if (( *pszStartLine == '?' ) && ( QueryIsConsole( STDIN ))) {

                    line = skipspace( pszStartLine + 1 );

                    // "? > ..." a kludge for Mike B.!
                    if (( *line != '\0' ) && ( *line != '>' ) && ( *line != '|' ) && ( *line != gpIniptr->CmdSep)) {

                        char *pszPrompt = line;

                        if ( *line == '"' ) {
                                // get prompt text enclosed in quotes
                                pszPrompt++;
                                line++;
                                while (( *line != '\0' ) && ( *line != '"' ))
                                        line++;
                                if ( *line == '"' )
                                        *line++ = '\0';
                        }

                        if ( QueryInputChar( pszPrompt, YES_NO ) == NO_CHAR )
                                break;
                        strcpy( pszStartLine, skipspace( line ));
                    }
                }

                // IFF may have removed command
                if (( line = first_arg( pszStartLine )) == NULL )
                        break;

                // skip a REM
                // kludge for dumdums who do "rem > file"
                if (( _stricmp( "REM", line ) == 0 ) && (*(skipspace( pszStartLine+3 )) != '>' )) {

                        // echo the REM lines if ECHO is ON
                        if ( fEcho )
                                printf( FMT_STR_CRLF, pszStartLine );
                        break;
                }

                // check last argument for an escape char (line continuation)
                if (( rval = continued_line( pszStartLine )) != 0 )
                        break;

                // Do variable expansion (except for certain internal commands)
                //   var_expand() will ignore variables inside command groups
                if ((( i = findcmd( pszStartLine, 0 )) < 0 ) || ( commands[i].pflag & CMD_EXPAND_VARS )) {
                        if ((( rval = var_expand( pszStartLine, 0 )) != 0 ) || (*(pszStartLine = skipspace( pszStartLine )) == '\0' ))
                                break;
                }

                line = (( *(skipspace( pszStartLine )) == '(' ) ? QUOTES_PARENS : QUOTES );

                // Trailing command group in internal command?  ("IF a==b (")
                if (( i >= 0 ) && ( commands[i].pflag & CMD_GROUPS )) {

                        // check last argument for a " ("
                        line = strend( pszStartLine ) - 1;
                        if (( *line == '(' ) && ( isdelim( line[ -1 ] )) && (( rval = command_groups( line, 0 )) != 0 ))
                                break;

                        // for commands like IF, IFF, FOR, etc. watch for ( )'s
                        line = QUOTES_PARENS;
                }

                // check for batch single-stepping
                if (( fOptions & 2 ) && ( cv.bn >= 0 ) && ( gpIniptr->SingleStep ) && ( fStepOver == 0 ) && ( QueryIsConsole( STDIN ))) {

                        extern int BatchDebugger( void );

                        pszBatchDebugLine = pszStartLine;
                        rval = BatchDebugger( );

                        if (( rval == ERROR_EXIT ) || ( rval == 'Q' )) {
                                for ( rval = cv.bn; ; rval-- ) {
                                        bframe[ rval ].offset = -1L;
                                        if ( rval == 0 )
                                                break;
                                }
                                break;
                        } else if ( rval == 'J' ) {
                                // 'Jump'?
// Future enhancement - jump to any line in batch file
                                rval = 0;
                                break;
                        } else if ( rval == 'S' ) {
                                // 'S'(tep over)
                                fStep = fStepOver = 1;
                        } else if (( rval == 'O' ) || ( rval == ESCAPE ))
                                gpIniptr->SingleStep = 0;

                        // default is 'T'(race into)
                }

                // DETACH passes entire line to child
                if (( i >= 0 ) && ( commands[i].pflag & CMD_DETACH_LINE ))
                        line = strend( pszStartLine );
                else
                // get compound command char, pipe, conditional, or EOL
                if (( line = scan( pszStartLine, NULL, line )) == BADQUOTES )
                        break;

                rval = 0;
                cEoL = condition = 0;

                // terminate command & save special char (if any)
                if ( *line ) {
                        cEoL = *line;
                        *line++ = '\0';
                }

                // command ECHOing
                if (( fEcho ) && ( *pszStartLine ))
                        printf( FMT_STR_CRLF, pszStartLine );

                // command logging
                if ( gpIniptr->LogOn )
                        (void)_log_entry( pszStartLine, 0 );

                // process compound command char, pipe, or conditional
                if ( cEoL == '|' ) {

                        if ( *line == '|' ) {

                                // conditional OR (a || b)
                                condition |= OR_CONDITION;
                                line++;

                        } else {

                                char *arg;

                                // check last argument for a group (" (")
                                //   and pass the group to the child process
                                arg = strend( line ) - 1;
                                if (( *arg == '(' ) && ( isdelim( arg[ -1 ] )) && (( rval = command_groups( arg, 0 )) != 0 ))
                                        break;

                                if (( rval = open_pipe( &redirect, line )) != 0 )
                                        break;
                                line--;
                        }

                } else if (( cEoL == '&' ) && ( *line == '&' )) {

                        // conditional AND (a && b)
                        condition |= AND_CONDITION;
                        line++;
                }

                // do I/O redirection except for some internal commands
                //   ( redir() will check for command grouping)
                if ((( gpIniptr->Expansion & EXPAND_NO_REDIR ) == 0 ) && (( i < 0 ) || ( commands[i].pflag & CMD_EXPAND_REDIR ))) {

                        if ( redir( pszStartLine, &redirect )) {
                                rval = ERROR_EXIT;
                                break;
                        }
                }

                // strip leading delimiters & trailing whitespace
                strip_leading( pszStartLine, ", \t\n\f\r" );
                strip_trailing( pszStartLine, " \t\r\n" );

                rval = ProcessCommands( pszStartLine, line );

                // change icon label back to the default
                if ( i < 0 )
                        update_task_list(( cv.bn < 0 ) ? NULL : bframe[cv.bn].pszBatchName );

                // check for ^C or stack overflow
                if (( setjmp( cv.env ) == -1 ) || ( cv.exception_flag )) {
                        rval = CTRLC;
                        break;
                }

                if ( fStep ) {
                        // reenable batch debugger after stepping over
                        //   a CALL, GOSUB, etc.
                        fStep = fStepOver = 0;
                }

                // clean everything up
                line = gszCmdline;

                // check for funny return value from IFF  (this kludge allows
                //   you to pipe to a command following the IFF; otherwise
                //   IFF would eat the pipe itself)
                if ( rval == 0x666 ) {
                        rval = 0;
                        continue;
                }

                // if conditional command failed, skip next command(s)
                while ((( rval != 0 ) && ( condition & AND_CONDITION )) || (( rval == 0 ) && ( condition & OR_CONDITION ))) {

                        // check for command grouping
                        line = skipspace( line );
                        if (( line = scan( line, NULL, ((*line == '(' ) ? QUOTES_PARENS : QUOTES) )) == BADQUOTES)
                                break;

                        if (( *line == '&' ) && ( line[1] == '&' )) {
                                condition = AND_CONDITION;
                                line += 2;
                        } else if (( *line == '|' ) && ( line[1] == '|' )) {
                                condition = OR_CONDITION;
                                line += 2;
                        } else
                                break;
                }

                ClosePipe( line );
                // clean up redirection & check pipes from last command
                unredir( &redirect, &rval );
        }

        // clean up I/O redirection
        unredir( &redirect, &rval );

        if ( fOptions & 1 )
                crlf();

        // disable signal handling momentarily
        HoldSignals();

        if ( fStep )
                fStepOver = 0;

        return rval;
}


static int ProcessCommands( char *pszStartLine, char *line )
{
        char *pszCmdName;
        char *arg;
        int i, rval = 0;

        if ( *pszStartLine != '\0' ) {

            // save the line continuation (if any) onto the stack
            arg = _alloca( strlen( line ) + 1 );
            line = strcpy( arg, line );

            if ( *pszStartLine == '(' ) {

                pszStartLine = strcpy( gszCmdline, pszStartLine );

                // Process command groups
                if (( rval = command_groups( pszStartLine, 1 )) == 0 ) {
                    PIPE_INHERIT *pPipeSave;

                    // don't return to parent pipe process in the middle of a
                    //   command group!
                    pPipeSave = gpPipeParent;
                    gpPipeParent = 0L;
                    rval = command( pszStartLine, 0 );
                    gpPipeParent = pPipeSave;
                }

            } else if (( pszStartLine[1] == ':' ) && ( isdelim( pszStartLine[2] ))) {

                // a disk change request
                rval = __cd( pszStartLine, CD_CHANGEDRIVE | CD_SAVELASTDIR | CD_NOFUZZY );

            // try internal commands
            } else if (( rval = InternalCommands( pszStartLine )) == -1) {
                // if it's a directory name, do a fast CDD
                // Have to break on first '/' (annoying UNIX users) because
                //   too many people do things like "program/a"
                if (( gpIniptr->UnixPaths ) || ( strnicmp( pszStartLine, "../", 3 ) == 0 ))
                        pszCmdName = ntharg( pszStartLine, 0x800 );
                else
                        pszCmdName = first_arg( pszStartLine );

                i = ( strlen( pszCmdName ) - 1 );

                // check for implicit directory change (trailing '\')
                // call "parse_line" because we need to process any
                //   escaped characters
                if (( i >= 0 ) && (( pszCmdName[i] == '\\' ) || ( pszCmdName[i] == '/' )))
                    rval = parse_line( "cdd", pszStartLine, cdd_cmd, CMD_STRIP_QUOTES );
                else {
                    // search for external command
                    rval = ExternalCommands( pszStartLine, pszCmdName );
                }
            }
        }

        // restore remainder of line
        strcpy( gszCmdline, line );

        return rval;
}


// search for an internal command, & execute it if found
static int InternalCommands( char *pszStartLine )
{
        extern int nRexxError;
        int i;
        char *pszCmdName;
        int rval = 0;

        // search for an internal command
        if (( i = findcmd( pszStartLine, 0 )) < 0 )
                return i;

        // save the command name (for things like USAGE)
        gpInternalName = commands[i].cmdname;

        // check for "command /?" & display help if found
        //   else, call the internal function
        if ((( pszCmdName = first_arg( pszStartLine + strlen( gpInternalName ))) != NULL ) && (pszCmdName[0] == gpIniptr->SwChr ) && (pszCmdName[1] == '?' ) && (pszCmdName[2] == '\0' ))
                return ( _help( gpInternalName, pszCmdName ));

        // check if command only allowed in a batch file
        if (( commands[i].pflag & CMD_ONLY_BATCH ) && ( cv.bn == -1 )) {
                error( ERROR_4DOS_ONLY_BATCH, gpInternalName );
                return USAGE_ERR;
        }

        pszWindowTitle = pszStartLine;
        rval = parse_line( pszStartLine, (char *)(pszStartLine+strlen( gpInternalName)), commands[i].func, commands[i].pflag );

        // ON ERROR usually returns a CTRLC, which isn't really accurate
        if (( rval == CTRLC ) && ( cv.exception_flag & BREAK_ON_ERROR ))
                rval = gnInternalErrorLevel;
        else if ( rval == BATCH_RETURN_RETURN )
                rval = BATCH_RETURN;
        else if (( rval == BATCH_RETURN ) || (( gnInternalErrorLevel = rval ) == ABORT_LINE ) || ( gnInternalErrorLevel == 0x666 )) {
                // clear internal error level flag
                gnInternalErrorLevel = 0;
        }

        nRexxError = gnInternalErrorLevel;

        // for some reason CMD.EXE sets ERRORLEVEL on some
        //   (but not all!) internal commands
        if (( commands[i].pflag & CMD_SET_ERRORLEVEL ) && ( rval != ABORT_LINE ) && ( rval != BATCH_RETURN ))
                gnErrorLevel = rval;
        return rval;
}


// search for an external command or batch file, & execute it if found
static int ExternalCommands( char *pszStartLine, char *pszCmdName )
{
        char *ptr, *pszSave;
        PCH feptr;
        int i, rval = 0;

        pszSave = pszStartLine;

        // search for external command, & set batch name pointer
        *(scan( pszCmdName + 1, "`=", QUOTES + 1 )) = '\0';

        // nitwit JPI Modula-2 escapes first char of command!
        EscapeLine( pszCmdName );

        ptr = _alloca( strlen( pszCmdName ) + 1 );
        gpBatchName = pszCmdName = strcpy( ptr, pszCmdName );

        // Wildcards not allowed in program names; note that we have to kludge
        //   the check for '[' because of Netware stupidity with names
        //   like "[:\dos\program.exe"
        ptr = pszCmdName;
        if (( *ptr == '[' ) && ( ptr[1] == ':' ))
                ptr += 2;

        // if not a COM, EXE, BTM, or BAT (or CMD), or
        //   a user-defined "executable extension", return w/error
        if (( strpbrk( ptr, WILD_CHARS ) == NULL ) && (( ptr = searchpaths( pszCmdName, NULL, TRUE )) != NULL )) {

                // DOS only allows a 127 character command line, so save the
                //   full command line to var CMDLINE.  If command line preceded
                //   by a '@', remove CMDLINE from the environment
                if ( fSetCMDLINE ) {
                    strins( pszStartLine, CMDLINE_VAR );
                    add_variable( pszStartLine );
                    strcpy( pszStartLine, pszStartLine + strlen( CMDLINE_VAR ));
                } else
                    add_variable( CMDLINE_VAR );
                pszWindowTitle = pszStartLine;
                pszStartLine += strlen( pszCmdName );

                pszCmdName = mkfname( ptr, 0 );
                if (( ptr = ext_part( pszCmdName )) == NULL )
                        ptr = NULLSTR;

                // OS/2 2.0 runs a DOS box on a .BAT file
                if (( _stricmp( ptr, COM ) == 0 ) || ( _stricmp( ptr, EXE ) == 0 ) || ( _stricmp( ptr, BAT ) == 0 ))
                {
                    gnErrorLevel = rval = parse_line( pszCmdName, pszStartLine, external, CMD_STRIP_QUOTES | CMD_CLOSE_BATCH );
                }
                else if ( _stricmp( ptr, CMD ) == 0 ) {
                    // check .CMD files for REXX or EXTPROC
                    rval = process_rexx( pszCmdName, pszStartLine, FALSE );
                } else if ( _stricmp( ptr, BTM ) == 0 )
                    rval = parse_line( pszCmdName, pszStartLine, batch, CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_CLOSE_BATCH | CMD_UCASE_CMD );

                else {

                    // is it an executable extension?
                    if ( *(feptr = executable_ext( ptr )) != '\0' ) {

                        AddQuotes( pszCmdName );

                        // check for length & save remainder of command line
                        i = strlen( feptr ) + strlen( pszCmdName ) + 1;

                        if (( i + strlen( pszStartLine )) >= MAXLINESIZ )
                                return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));

                        memmove( ( gszCmdline + i ), pszStartLine, (MAXLINESIZ - i) );
                        sprintf( gszCmdline, "%Fs %s%s", feptr, pszCmdName, gszCmdline+i );

                        // go back so we can support aliases, internal and
                        //   external commands
                        rval = command( gszCmdline, 0 );

                    } else {
                        // OS/2 CMD.EXE allows you to execute program
                        //   names like "program.pgm"
                        // NT will execute ASSOC'd file extensions
                        gnErrorLevel = rval = parse_line( pszCmdName, pszStartLine, external, (CMD_STRIP_QUOTES | CMD_CLOSE_BATCH) );
                    }
                }

        } else {
                // check for UNKNOWN_CMD alias
                if ( get_alias( UNKNOWN_COMMAND ) != 0L ) {

                        static int nUCLoop = 0;

                        if ( ++nUCLoop > 10 )
                                rval = error( ERROR_4DOS_UNKNOWN_CMD_LOOP, pszSave );

                        else {

                                if (( strlen( pszSave ) + 14 ) > MAXLINESIZ )
                                        return ( error( ERROR_4DOS_COMMAND_TOO_LONG, pszSave ));
                                strcpy( gszCmdline, pszSave );
                                strins( gszCmdline, " " );
                                strins( gszCmdline, UNKNOWN_COMMAND );

                                // go back so we can support aliases, internal
                                //   and external commands
                                rval = command( gszCmdline, 0 );
                        }

                        nUCLoop = 0;

                } else {
                        // unknown command
                        gnErrorLevel = rval = error( ERROR_4DOS_UNKNOWN_COMMAND, pszCmdName );
                }
        }

        return rval;
}


// process command grouping & get additional lines if necessary
static int command_groups( char *pszStartLine, int remove_parens )
{
        int rval;
        char *ptr;

        // first get the command group; then remove leading & trailing parens
        //   (except when retrieving continuation lines)
        for ( ; ; ) {

                if (( ptr = scan( pszStartLine, ")", QUOTES_PARENS )) == BADQUOTES)
                        return ERROR_EXIT;

                if ( *ptr == '\0' ) {

                        // go back & get some more input
                        //   ("line" has to be empty to get here!)

                        // don't remove ( ) 's if a line continuation is found
                        remove_parens = 0;

                        strcat( pszStartLine, " " );
                        ptr = strend( pszStartLine );

                        if (( rval = ( CMDBUFSIZ - (UINT)(( ptr + 2) - gszCmdline))) <= 0 ) {
                                error( ERROR_4DOS_COMMAND_TOO_LONG, NULL );
                                return BATCH_RETURN;
                        }

                        if ( rval > ( MAXLINESIZ - 1 ))
                                rval = MAXLINESIZ - 1;

                        // if not a batch file, prompt for more input
                        if ( cv.bn < 0 ) {

                                printf( COMMAND_GROUP_MORE );
                                rval = getline( STDIN, ptr, rval, EDIT_COMMAND );

                                // add it to history list
                                addhist( ptr );

                                // history logging
                                if ( gpIniptr->HistLogOn )
                                        (void)_log_entry( ptr, 1 );

                        } else {
                                rval = getline( bframe[ cv.bn ].bfd, ptr, rval, EDIT_COMMAND );
                                bframe[cv.bn].uBatchLine++;
                        }

                        if ( rval == 0 ) {
                                error( ERROR_4DOS_UNBALANCED_PARENS, pszStartLine );
                                return BATCH_RETURN;
                        }

                        // ignore blank and comment lines
                        if (( first_arg( ptr ) == NULL ) || ( _stricmp( first_arg( ptr ), "REM" ) == 0 )) {
                                ptr[-1] = '\0';
                                continue;
                        }

                        // skip leading whitespace
                        strcpy( ptr, skipspace( ptr ) );
                        if (( ptr[-2] != '(' ) && ( *ptr != ')' )) {
                                strins( ptr, "  " );
                                *ptr = gpIniptr->CmdSep;
                        }

                } else {

                        // remove the first ( ) set
                        if ( remove_parens ) {
                                strcpy( ptr, ptr + 1 );
                                strcpy( pszStartLine, pszStartLine + 1 );
                        }

                        return 0;
                }
        }
}


// get additional line(s) when previous line ends in an escape
int continued_line( char *pszStartLine )
{
        int nLength;

        if (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 ) {

            for ( ; ; ) {

                if (( nLength = strlen( pszStartLine )) == 0 )
                        break;
                pszStartLine += ( nLength - 1 );

                // check for single trailing escape character
                //  (plus a kludge for trailing %=)
                if ((( gpIniptr->Expansion & EXPAND_NO_VARIABLES ) == 0 ) && ( *pszStartLine == '=' ) && ( nLength > 1 ) && ( pszStartLine[-1] == '%' ))
                        pszStartLine--;
                else if (( *pszStartLine != gpIniptr->EscChr ) || (( nLength > 1 ) && ( pszStartLine[1] == gpIniptr->EscChr )))
                        break;

                // get some more input
                *pszStartLine = ' ';

                if (( nLength = (CMDBUFSIZ - (UINT)((pszStartLine + 2) - gszCmdline))) <= 0 ) {
                        error( ERROR_4DOS_COMMAND_TOO_LONG, NULL );
                        return BATCH_RETURN;
                }

                if ( nLength > ( MAXLINESIZ - 1 ))
                        nLength = MAXLINESIZ - 1;

                // if not a batch file, prompt for more input
                if ( cv.bn < 0 ) {

                        printf( COMMAND_GROUP_MORE );
                        getline( STDIN, pszStartLine, nLength, EDIT_COMMAND );

                        // add it to history list
                        addhist( pszStartLine );

                } else {
                        getline( bframe[cv.bn].bfd, pszStartLine, nLength, EDIT_COMMAND );
                        bframe[cv.bn].uBatchLine++;
                }
            }
        }

        return 0;
}


// parse the command line into Argc, Argv format
int parse_line( char *pszCmdName, char *pszLine, int (*cmd)(int, char **), int pflag)
{
        static int Argc;                // argument count
        static char *Argv[ARGMAX];      // argument pointers
        static char *pszTemp;
        static char cQuote;

        // pflag bit arguments are:
        //   CMD_STRIP_QUOTES - strip quoting
        //   CMD_ADD_NULLS - add terminators to each argument
        //   CMD_CLOSE_BATCH - close open batch file before executing command

        // if it's an external or special case internal, close the batch file
        if ( pflag & CMD_CLOSE_BATCH )
                close_batch_file();

        // set command / program name
        if ( pflag & CMD_UCASE_CMD )
                strupr( pszCmdName );
        Argv[0] = pszCmdName;

        pszLine = skipspace( pszLine );

        // loop through arguments
        for ( Argc = 1; (( Argc < ARGMAX ) && ( *pszLine )); Argc++ )  {

                pszTemp = pszLine;              // save start of argument

                while ( isdelim( *pszLine ) == 0 ) {

                    if ((( gpIniptr->Expansion & EXPAND_NO_QUOTES) == 0 ) && ((*pszLine == SINGLE_QUOTE) || (*pszLine == DOUBLE_QUOTE))) {

                        cQuote = *pszLine;

                        // strip a single quote
                        if (( *pszLine == SINGLE_QUOTE ) && ( pflag & CMD_STRIP_QUOTES ))
                                strcpy( pszLine, pszLine + 1 );
                        else
                                pszLine++;

                        // arg ends after matching close quote
                        while (( *pszLine ) && ( *pszLine != cQuote )) {

                                // collapse any "escaped" characters
                                if ( pflag & CMD_STRIP_QUOTES ) {
                                        // CMD.EXE kludge to NOT collapse
                                        // escape chars inside double quotes
                                        if ( cQuote != DOUBLE_QUOTE )
                                                escape( pszLine );
                                }

                                if ( *pszLine )
                                        pszLine++;
                        }

                        // strip a single quote
                        if (( *pszLine == SINGLE_QUOTE ) && ( pflag & CMD_STRIP_QUOTES ))
                                strcpy( pszLine, pszLine + 1 );
                        else if ( *pszLine )
                                pszLine++;

                    } else {

                        // collapse any "escaped" characters
                        if ( pflag & CMD_STRIP_QUOTES )
                                escape( pszLine );
                        if ( *pszLine )
                                pszLine++;
                    }
                }

                // check flag for terminators
                if ((( pflag & CMD_ADD_NULLS ) != 0 ) && ( *pszLine != '\0' ))
                        *pszLine++ = '\0';

                // update the Argv pointer
                Argv[ Argc ] = pszTemp;

                // skip delimiters for Argv[2] onwards
                while (( *pszLine ) && ( isdelim( *pszLine )))
                        pszLine++;
        }

        Argv[Argc] = NULL;

        // test if only processing line, not calling func
        if ( cmd == NULL )
                return 0;

        // update title bar (unless inside a batch file)
        if ((( cv.bn < 0 ) && ( cmd == external )) || ( cmd == batch ))
                update_task_list( Argv[0] );

        // call the requested function (indirectly)
        return (*cmd)( Argc, Argv );
}


// search paths for file -- return pathname if found, NULL otherwise
char * searchpaths( char *filename, char *pszSearchPath, int fSearchCurrentDir )
{
        static char szFileName[MAXFILENAME + 32];       // expanded filename
        char *pOldExe, *pNewExe;
        int i, n, nPass = 0;
        PCH feptr, fenvptr = 0L, lpPathExt = 0L, lpExt;
        FILESEARCH dir;
        int fHPFS;

        // build a legal filename ( right filename & extension size)
        insert_path( szFileName, fname_part( filename ), filename );

        // check for existing extension
        pOldExe = ext_part( szFileName );

        fHPFS = ifs_type( szFileName );

        // check for relative path spec
        if ( strstr( szFileName, "..." ) != NULL ) {

            if ( mkfname( szFileName, 0 ) == NULL )
                return NULL;

        // no path searches if path already specified!
        } else if ( path_part( filename ) != NULL )
            nPass = -1;

        else {

            if ( pszSearchPath != NULL )
                fenvptr = pszSearchPath;

            else if (( fenvptr = get_variable( PATH_VAR )) != 0L ) {

                // if path has a ";.", DON'T search the current directory first!
                for ( feptr = fenvptr; (( feptr = strchr( feptr, ';' )) != 0L ) ; feptr++ ) {
                    if (( feptr[1] == '.' ) && (( feptr[2] == ';' ) || ( feptr[2] == '\0' )))
                        goto next_path;
                }
            }

            // don't look in the current directory?
            if ( fSearchCurrentDir == FALSE )
                goto next_path;
        }

        if (( pszSearchPath == NULL ) && ( gpIniptr->PathExt ))
            lpPathExt = get_variable( PATHEXT );

        // search the PATH for the file
        for ( ; ; ) {

            if ( pOldExe != NULL ) {
                if ( is_file( szFileName ))
                    return szFileName;
            }

            // if LFN/HPFS/NTFS, support odd names like "file1.comp.exe"
            if (( pOldExe == NULL ) || ( fHPFS )) {

                // if no extension, try an executable one
                pNewExe = strend( szFileName );

                // first check with "filename.*" for any possible match
                strcpy( pNewExe, WILD_EXT );
                i = 0;

                if ( is_file( szFileName )) {
                    lpExt = lpPathExt;
                    for ( i = 0; ( nPass <= 0 ); i++ ) {

                        if ( lpPathExt ) {

                                if ( *lpExt == '\0' )
                                        break;

                                // get next PATHEXT argument
                                sscanf( lpExt, "%*[;,=]%31[^;,=]%n", pNewExe, &n );
                                lpExt += n;

                        } else {
                                if ( executables[i] == NULL )
                                        break;
                                strcpy( pNewExe, executables[i] );
                        }

                        // look for the file
                        if ( is_file( szFileName ))
                                return szFileName;
                    }

                    // check for user defined executable extensions
                    feptr = glpEnvironment;

                    for ( n = 0; (( feptr != 0L ) && ( *feptr != '\0' )); feptr = next_env( feptr )) {

                        if ( *feptr == '.' ) {

                            // add extension & look for filename
                            sscanf( feptr, "%13[^=]", pNewExe );

                            // search directory for possible wildcard match
                            if ( find_file( FIND_FIRST, szFileName, 0x2107, &dir, szFileName ) != NULL )
                                return szFileName;
                        }
                    }
                }
            }

next_path:
            if ( fenvptr == 0L )
                return NULL;

            // get next PATH argument, skipping delimiters
            sscanf( fenvptr, "%*[;,=]%260[^;,=]%n", szFileName, &n );

            // check for end of path
            if ( szFileName[0] == '\0' ) {
              return NULL;
            }

            fenvptr += n;

            // make new directory search name
            mkdirname( szFileName, fname_part( filename ));

            // make sure specified drive is ready
            if (( is_net_drive( szFileName ) == 0 ) && ( QueryDriveReady( gcdisk( szFileName )) == 0 ))
                goto next_path;
        }
}


// display the DOS / OS2 command line prompt
void show_prompt( void )
{
        extern char gaPushdQueue[];

        int c;
        char *eptr;
        char *ptr;
        PCH feptr;

        UCHAR   pBuf[81] = "";
        ULONG   cbMsgL;
        APIRET  ulrc;

        // odd kludge for UNC names
        if (( gnCurrentDisk = _getdrive()) < 0 )
                gnCurrentDisk = 0;

        // get prompt from environment, or use default if none
        if (( feptr = get_variable( PROMPT_NAME )) != 0L ) {

                // expand any environment variables in the PROMPT string
                strcpy( gszCmdline, feptr );
                (void)var_expand( gszCmdline, 1 );
                eptr = gszCmdline;

        } else {
                eptr = ((( gnCurrentDisk == 0 ) || ( gnCurrentDisk > 2 )) ? "[$p]" : "[$n]");
        }

        for ( ; ( *eptr != '\0' ); eptr++ ) {

                if ((( c = *eptr ) == '$' ) && ( eptr[1] )) {

                        // special character follows a '$'
                        switch ( _ctoupper( *(++eptr ) )) {
                        case 'A':       // ampersand
                                c = '&';
                                break;

                        case 'B':       // vertical bar
                                c = '|';
                                break;

                        case 'C':       // open paren
                                c = '(';
                                break;

                        case 'D':       // current date
                                if ( *eptr == 'd' ) {
                                        // "Sat 01-01-92"
                                        printf( "%.4s%s", gdate(0 ), gdate(1) );
                                } else {
                                        // "Sat  Jan 1, 1992"
                                        qputs( gdate( 0 ) );
                                }

                                continue;

                        case 'E':       // ESCAPE (for ANSI sequences)
                                c = ESC;
                                break;

                        case 'F':       // close paren
                                c = ')';
                                break;

                        case 'G':
                                c = '>';
                                break;

                        case 'H':       // destructive backspace
                                qputs( "\010 \010" );
                                continue;

                        case 'I':       // display Program Selector help
                                {
                                int i, n;
                                VIOMODEINFO vioMode;

                                vioMode.cb = sizeof( vioMode );
                                (void)VioGetMode( &vioMode, 0 );

                                // set attribute (inverse white on mono screen,
                                //   white on blue for color screen)
                                i = ( vioMode.fbType == 0 ) ? 0x70 : 0x1F;

                                ulrc = DosGetMessage(NULL, 0, pBuf, sizeof(pBuf), 1492, "OSO001.MSG", &cbMsgL);
                                if (ulrc != NO_ERROR)
                                printf(all_GetSystemErrorMessage(ulrc));
                                (void)VioWrtCharStrAtt( pBuf, strlen( pBuf ), 0, 0, (PBYTE)&i, 0 );

                                // kludge if we're on line 0
                                GetCurPos( &i, &n );
                                if ( i == 0 )
                                        SetCurPos( 1, n );
                                }
                                // ignore $i in TCMD
                                continue;

                        case 'L':
                                c = '<';
                                break;

                        case 'M':       // current time w/o seconds
                                ptr = gtime( *eptr == 'm' );

                                // remove seconds (but preserve "a" or "p")
                                strcpy( ptr + 5, ptr + 8 );
                                qputs( ptr );
                                continue;

                        case 'N':       // default drive letter
                                if ( gnCurrentDisk == 0 )
                                        continue;
                                c = gnCurrentDisk + 64;
                                break;

                        case 'P':       // current directory
                                if (( ptr = gcdir( NULL, 1 )) != NULL ) {

                                    if ( *eptr == 'P' ) {
                                        // HPFS & NTFS preserve case in names,
                                        //   so just leave it alone
                                        if ( ifs_type( ptr ) != 0 ) {
                                                qputs( ptr );
                                                continue;
                                        }
                                    }

                                    qputs((( *eptr == 'p' ) ? strlwr( ptr ) : strupr( ptr )));
                                }
                                continue;

                        case 'Q':       // Equals sign
                                c = '=';
                                break;

                        case 'R':       // errorlevel
                                printf( FMT_INT, gnErrorLevel );
                                continue;

                        case 'S':       // print space
                                c = ' ';
                                break;

                        case 'T':       // current time
                                qputs( gtime( *eptr == 't' ) );
                                continue;

                        case 'V':       // OS version number
                                printf( gszOsVersion );
                                continue;

                        case 'X':       // default dir for specified drive
                                if (( ptr = gcdir( (( eptr[1] ) ? eptr + 1 : NULL ), 1 )) != NULL )
                                        qputs(( *eptr == 'x' ) ? ptr : strupr( ptr ));


                                if ( eptr[1] ) {
                                        eptr++;
                                        if ( eptr[1] == ':' )
                                                eptr++;
                                }
                                continue;

                        case 'Z':       // shell nesting level
                                printf( FMT_UINT, gpIniptr->ShellNum );
                                continue;

                        case '_':       // CR/LF
                                c = '\n';
                                break;

                        case '$':       // just print a $
                                c = *eptr;
                                break;

                        case '+':       // print a + for each PUSHD
                                for ( feptr = (char *)gaPushdQueue; ( *feptr != '\0' ); feptr = next_env( feptr ) )
                                        qputc( STDOUT, '+' );

                        default:        // unknown metachar - ignore it!
                                continue;
                        }
                }

                qputc( STDOUT, (char)c );
        }
}

