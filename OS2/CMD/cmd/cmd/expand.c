// EXPAND.C
//   Copyright (c) 1988 - 1998  Rex C. Conn  All rights reserved.
//
//   Expand command aliases
//   Expand environment variables, internal variables, and variable functions
//   Perform redirection
//   History processing

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <process.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <share.h>
#include <string.h>

#include "4all.h"

void SeekToEnd( int);
static int var_func( char *, char * );
static char * var_internal( char * );
static int AttributeString( char *, int * );
static void format_long( char *, char *, unsigned long );


// duplicate a file handle, and close the old handle
void dup_handle( unsigned int old_fd, unsigned int new_fd )
{
        if ( _dup2( old_fd, new_fd ) < 0 )
                _close( new_fd );
        _close( old_fd );
}


// kludge to back up over a ^Z at EoF
void SeekToEnd( int fd )
{
        int ch = 0;

        if ( _lseek( fd, -1L, SEEK_END ) >= 0L ) {

                (void)_read( fd, &ch, 1 );
                // bug in NT makes it read _past_ ^Z!!
                if ( ch == 0x1A )
                        (void)_lseek( fd, -1L, SEEK_CUR );
        }
}


// Redirecting to CLIP: - build filename in TMP directory
void RedirToClip( char *pszName, int fStd )
{
        char *arg;
        PCH feptr;

        // look for TEMP disk area defined in environment, or default to boot
        feptr = GetTempDirectory( pszName );

        // if no TMP directory, use TCMD directory
        if ( feptr == 0L )
                strcpy( pszName, path_part( _pgmptr ));

        if ( fStd == 0 )
                arg = "CLIP_IN.JPS";
        else if ( fStd == 1 )
                arg = "CLIP_OUT.JPS";
        else
                arg = "CLIP_TMP.JPS";
        mkdirname( pszName, arg );
}


// copy the file to the clipboard
int CopyToClipboard( int fh )
{
        PCH lpGlobalMemory;
        int nBytesRead;
        LONG lMemSize;

        if (( pfnWOC == NULL ) || ( (*pfnWOC)( ghHAB ) == 0 ))
                return ( error( ERROR_4DOS_CLIPBOARD_INUSE, NULL ));

        (*pfnWEC)( ghHAB );
        // set clipboard buffer size
        if (( lMemSize = QuerySeekSize( fh )) > 0L ) {

                DosAllocSharedMem( (PVOID)&lpGlobalMemory, NULL, lMemSize + 2, OBJ_GETTABLE | OBJ_GIVEABLE | OBJ_TILE | PAG_COMMIT | PAG_READ | PAG_WRITE );

                // save file
                RewindFile( fh );
                setmode( fh, O_BINARY );
                if (( nBytesRead = _read( fh, lpGlobalMemory, lMemSize )) >= 0 ) {
                        lpGlobalMemory[ nBytesRead ] = '\0';
                        (*pfnWSCD)( ghHAB, (LONG)lpGlobalMemory, CF_TEXT, CFI_POINTER );
                }
        }
        (*pfnWCC)( ghHAB );

        return 0;
}


// Redirecting from CLIP: - copy from the clipboard to the specified file
int CopyFromClipboard( char *pszFilename )
{
        PCH lpClipMemory;
        int fh, rval = 0;
        unsigned long ulFormat;

        // remove any leftover file
        remove( pszFilename );
        if (( pfnWQCFI == NULL ) || ( (*pfnWQCFI)( ghHAB, CF_TEXT, &ulFormat ) == 0 ))
                return ( error( ERROR_4DOS_CLIPBOARD_NOT_TEXT, NULL ));

        if ( (*pfnWOC)( ghHAB ) == 0 )
                return ( error( ERROR_4DOS_CLIPBOARD_INUSE, NULL ));

        lpClipMemory = (PCH)(*pfnWQCD)( ghHAB, CF_TEXT );
        // attempted kludge for OS/2 bug (it usually doesn't work) - OS/2
        //   doesn't make clipboard memory readable for VIO apps!
        if (( rval= DosGetSharedMem( lpClipMemory, PAG_READ )) != 0 )
                lpClipMemory = 0L;

        if ( lpClipMemory != 0L ) {

            if (( fh = _sopen( pszFilename, (O_WRONLY | O_BINARY | O_CREAT | O_TRUNC), SH_DENYWR, S_IWRITE | S_IREAD )) < 0 )
                rval = error( _doserrno, pszFilename );
            else {
                // save file
                _write( fh, lpClipMemory, strlen( lpClipMemory ));
                _close( fh );
            }
        }

        (*pfnWCC)( ghHAB );
        return rval;
}



// Perform I/O redirection (except pipes).  All > and <'s will be removed.
int redir( char *start_line, REDIR_IO *redirect )
{
        static char delims[] = "  \t;,<>|()`+=[]";
        static char os2_delims[] = "  \t,:=\"|()";

        char *line, *arg;
        int fd, i, fClip, nOpenMode = 0, nShareMode, nc, name_length;
        char os2_format;
        char fname[MAXFILENAME], redirections[MAXREDIR];
        int nAppType;

        if ( gpIniptr->Expansion & EXPAND_NO_REDIR )
                return 0;

        // check for an external DOS / Win16 command (can't redirect them here)
        arg = first_arg( start_line );
        if (( arg != NULL ) && ( findcmd( arg, 0 ) < 0 )) {

                if (( arg = searchpaths( arg, NULL, TRUE )) != NULL ) {
                        nAppType = app_type( arg );
                        if (( nAppType >= 0 ) && ( nAppType & 0x1620 ))
                                return 0;
                }
        }

        delims[1] = gpIniptr->CmdSep;
        os2_delims[1] = gpIniptr->CmdSep;

        for ( ; ; ) {

                // check for I/O redirection (< or >)
                if (( line = scan( start_line, "<>", (( *start_line == '(' ) ? QUOTES_PARENS : QUOTES ))) == BADQUOTES )
                        return ERROR_EXIT;
                if ( *line == '\0' )
                        return 0;

                // set the NOCLOBBER default
                nc = gpIniptr->NoClobber;

                // save pointer to redirection character
                arg = line++;

                nShareMode = SH_DENYWR;
                os2_format = 0;

                // clear I/O redirection flags
                for ( i = 0; ( i < MAXREDIR ); i++ )
                        redirections[ i ] = 0;
                fClip = 0;

                // force batch file close (for Netware bug)
                close_batch_file();

                if ( *arg == '>' ) {

                        // redirecting output (stdout and/or stderr)
                        redirections[STDOUT] = 1;

                        if ( *line == '>' ) {
                                // append to output file
                                nOpenMode = (O_RDWR | O_TEXT);
                                line++;
                        } else          // overwrite the output file
                                nOpenMode = (O_WRONLY | O_TEXT | O_TRUNC);

                        if ( *line == '&' ) {

                                // redirect STDERR too (>&) or STDERR only (>&>)
                                redirections[ STDERR ] = 1;

                                if ( *(++line ) == '>' ) {
                                        // redirect STDERR only
                                        redirections[ STDOUT ] = 0;
                                        line++;
                                // it _might_ be CMD.EXE "2>&1" format
                                } else if ( isdigit( *line ))
                                        os2_format = 1;
                        }

                        if ( *line == '!' ) {   // override NOCLOBBER
                                nc = 0;
                                line++;
                        }

                } else {

                        // redirecting input (stdin)
                        redirections[0] = 1;

                        // kludge for morons who do "file << file2"
                        while ( *line == '<' )
                                line++;

                        // CMD.EXE allows input redirection like "<&n"
                        if ( *line == '&' ) {
                                os2_format = 1;
                                line++;
                        }
                }

                // get the file name (can't use sscanf() with []'s)
                line = skipspace( line );
                if (( name_length = ( scan( line, delims, QUOTES ) - line )) > (MAXFILENAME- 1 ))
                        name_length = (MAXFILENAME - 1 );
                sprintf( fname, FMT_PREC_STR, name_length, line );
                EscapeLine( fname );

                if ( *arg == '>' ) {

                        // CMD.EXE allows output redirection like "n>&n"
                        fd = -1;

                        // check for internal file ID redirection (0 - 9)
                        if (( isdigit( fname[0] )) && ( fname[1] == '\0' ) && ( os2_format )) {
                                // >& maybe didn't mean redirect STDERR
                                redirections[STDERR] = 0;
                                fd = ( *fname - '0' );
                        } else
                                os2_format = 0;

                        // check for redirecting from other than STDOUT/STDERR
                        if (( isdigit( arg[-1] )) && ( strchr( os2_delims, arg[-2] ) != NULL )) {
                                redirections[ STDOUT ] = redirections[STDERR] = 0;
                                redirections[ ( *(--arg) - '0' )] = 1;
                        }

                        // open the file if not using a pre-defined handle
                        if ( fd == -1 ) {

                                // have to do QueryIsDevice() first for "NUL:"
                                if ( QueryIsDevice( fname )) {
                                        nOpenMode |= O_CREAT;
                                        nShareMode = SH_DENYNO;
                                } else {

                                        // expand filename ("dir > ...\file" )
                                        if ( mkfname( fname, 0 ) == NULL )
                                                return ERROR_EXIT;

                                        // if NOCLOBBER set, don't overwrite an
                                        //   existing file, and ensure the file
                                        //   exists on an append
                                        if ( nc ) {
                                                if ( nOpenMode & O_TRUNC )
                                                        nOpenMode |= ( O_CREAT | O_EXCL );
                                        } else
                                                nOpenMode |= O_CREAT;
                                }

                                if ( stricmp( fname, CLIP ) == 0 ) {

                                        // build temp file name
                                        RedirToClip( fname, 1 );
                                        fClip = 1;

                                        if ( nOpenMode & O_RDWR ) {
                                                CopyFromClipboard( fname );
                                                nOpenMode = O_RDWR | O_CREAT | O_TEXT;
                                        } else
                                                nOpenMode = O_RDWR | O_CREAT | O_TEXT | O_TRUNC;
                                        nShareMode = SH_DENYRW;
                                }

                                if (( fd = _sopen( fname, nOpenMode, nShareMode, ( S_IREAD | S_IWRITE ))) < 0 )
                                        return ( error(( errno == EEXIST ) ? ERROR_FILE_EXISTS : _doserrno, fname ));

                                // set "writing to CLIPBOARD" flag
                                if ( fClip ) {
                                    if ( redirections[ STDOUT ] )
                                        redirect->fClip[ STDOUT ] = 1;
                                    else if ( redirections[ STDERR ] )
                                        redirect->fClip[ STDERR ] = 1;
                                }
                        }

                        // if appending, go the end of the file
                        if (( nOpenMode & O_TRUNC ) == 0 )
                                SeekToEnd( fd );

                } else {

                        // check for redirecting to other than STDIN
                        if (( isdigit( arg[-1] )) && ( strchr( os2_delims, arg[-2] ) != NULL )) {
                                redirections[0] = 0;
                                redirections[ ( *(--arg ) - '0' )] = 1;
                        }

                        // already redirected this handle?
                        for ( i = 0; ( i < MAXREDIR ); i++ ) {
                                if (( redirect->std[ i ] ) && ( redirections[ i ] ))
                                        return ( error( ERROR_ALREADY_ASSIGNED, arg ));
                        }

                        // check for internal file ID redirection (0 - 9)
                        if ( os2_format ) {

                                if (( isdigit( fname[0] )) && ( fname[1] == '\0' ))
                                        fd = ( fname[0] - '0' );
                                else
                                        return ( error( ERROR_4DOS_BAD_SYNTAX, arg ));

                        } else {

                                if (( QueryIsDevice( fname ) == 0 ) && ( mkfname( fname, 0 ) == NULL ))
                                        return ERROR_EXIT;

                                // if CLIP:, copy clipboard to file & open it
                                if ( stricmp( fname, CLIP ) == 0 ) {
                                        // build temp file name
                                        RedirToClip( fname, 0 );
                                        CopyFromClipboard( fname );
                                }

                                // open the redirection source
                                if (( fd = _sopen( fname, O_RDONLY | O_BINARY, SH_DENYNO )) < 0 )
                                        return ( error( _doserrno, fname ));
                        }
                }

                // do redirection (backwards because of bug in NT)
                for ( i = MAXREDIR - 1; ( i >= 0 ); i-- ) {

                        if ( redirections[ i ] ) {
                                // kludge for IBM'ers who do "1>nul | rxqueue"
                                if ( redirect->std[ i ] == 0 )
                                        redirect->std[ i ] = _dup( i );
                                // don't inherit the saved handles!
                                NoInherit( redirect->std[ i ] );
                                _dup2( fd, i );
                        }
                }

                // don't close internal handles!
                if ( os2_format == 0 )
                        _close( fd );

                // now rub out the filename & redirection markers
                strcpy( arg, line + name_length );
        }
}


// Reset all redirection: stdin = stdout = stderr = console.
//   if DOS & a pipe is open, close it and reopen stdin to the pipe.
void unredir( REDIR_IO *redirect, int *pnError )
{
        int i;
        extern PIPE_INHERIT *gpPipeSource;
        RESULTCODES rescResults;
        unsigned int procID;

        gnChildPipeProcess = redirect->nChildPipe;

        // make sure we've removed the pipe name
        add_variable( SHAREMEM_PIPE_ENV );

        // clean up STDIN, STDOUT, and STDERR, and close open pipe
        for ( i = 0; ( i < MAXREDIR ); i++ ) {

                if ( redirect->std[ i ] ) {

                        // read CLIP: temp file & stuff it into the clipboard
                        if ( i <= STDERR ) {
                                if ( redirect->fClip[i] )
                                        CopyToClipboard( i );
                        }

                        dup_handle( redirect->std[ i ], i );
                        redirect->std[ i ] = 0;
                        redirect->fClip[ i ] = 0;

// FIXME - delete CLIP_IN.JPS & CLIP_OUT.JPS!
                }
        }

        if ( redirect->pipe_open ) {
                // wait for the child process (& its processes) to end
                DosWaitChild( DCWA_PROCESSTREE, DCWW_WAIT, &rescResults, (PPID)&procID, (PID)redirect->pipe_open );
                HoldSignals();

                // recover remainder of line ( if any) from child process
                if ( redirect->pPipeSource != 0L ) {
                        strcpy( gszCmdline, redirect->pPipeSource->CommandLine );
                        // close shared memory block
                        FreeMem( (PVOID)(redirect->pPipeSource) );
                        redirect->pPipeSource = 0L;
                }

                gnErrorLevel = gnInternalErrorLevel = rescResults.codeResult;
                if (( gnErrorLevel != 0 ) && ( pnError != NULL ))
                        *pnError = gnErrorLevel;

                redirect->nChildPipe = 0;
                gnChildPipeProcess = redirect->lPreviousPipe;
                EnableSignals();
                redirect->pipe_open = 0;
        }
}


// if in a pipe, send remainder of line back to parent process
void ClosePipe( char *line )
{
        extern PIPE_INHERIT *gpPipeParent;

        if ( gpPipeParent != 0L ) {
                strcpy( gpPipeParent->CommandLine, line );
                FreeMem( (PVOID)gpPipeParent );
                gpPipeParent = 0L;

                *line = '\0';
        }
}


// Expand aliases ( first argument on the command line)
int alias_expand( char *line )
{
        static char DELIMS[] = "%[^ <>=+|]%n";
        char *cptr;
        char aliasline[CMDBUFSIZ+1], scratch[5];
        char *arg, *eol, eolchar;
        int alen, argnum, loopctr, vars_exist;
        PCH feptr;

        // check for alias expansion disabled
        if ( gpIniptr->Expansion & EXPAND_NO_ALIASES )
                return 0;

        // beware of aliases with no whitespace before ^ or |
        DELIMS[3] = gpIniptr->CmdSep;

        for ( loopctr = 0; ; loopctr++ ) {

                vars_exist = 0;

                // skip past '?' (prompt before executing line) character
                if ( *line == '?' ) {

                        line = skipspace( ++line );
                        if ( *line == '\0' )
                                return 0;

                        // skip optional prompt string
                        if ( *line == '"' )
                                line = skipspace( scan( line, WHITESPACE, QUOTES+1 ));
                }

                // parse the first command in the line
                if (( arg = first_arg( line )) == NULL )
                        return ERROR_EXIT;

                // a '*' means don't do alias expansion on this command name
No_Alias:
                if ( *arg == '*' ) {
                        strcpy( gpNthptr, gpNthptr+1 );
                        return 0;
                }

                // skip past '@' (no history save) character
                if ( *arg == '@' ) {
                        if ( *(++arg) == '\0' )
                                return 0;
                        gpNthptr++;
                        goto No_Alias;
                }

                // strip things like <, >, |, etc.
                sscanf( arg, DELIMS, arg, &alen );

                // adjust for leading whitespace
                alen += ( gpNthptr - line );

                // search the environment for the alias
                if (( feptr = get_alias( arg )) == 0L )
                        return 0;

                // look for alias loops
                if ( loopctr > 16 )
                        return (error(ERROR_4DOS_ALIAS_LOOP, NULL ));

                // check if alias is too long
                if ( strlen( feptr ) >= (CMDBUFSIZ - 1 ))
                        return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));
                strcpy( aliasline, feptr );

                // get the end of the first command & its args
                if (( eol = scan( line, NULL, QUOTES )) == BADQUOTES )
                        return ERROR_EXIT;

                eolchar = *eol;
                *eol = '\0';

                // process alias arguments
                for ( cptr = aliasline; ( *cptr != '\0' ); ) {

                        if (( cptr = scan( cptr, "%", BACK_QUOTE )) == BADQUOTES )
                                return ERROR_EXIT;

                        if ( *cptr != '%' )
                                break;

                        // check for alias count (%#) or alias arg (%n&)
                        if ( cptr[1] == '#' ) {

                                strcpy(cptr,cptr+2 );

                                // %# evaluates to number of args in alias line
                                for ( argnum = 1; ( ntharg( line, argnum | 0x2000 ) != NULL ); argnum++ )
                                        ;

                                IntToAscii( argnum-1, scratch );
                                strins( cptr, scratch );
                                continue;

                        } else if (( isdigit(cptr[1]) == 0 ) && ( cptr[1] != gpIniptr->ParamChr )) {

                                // Not an alias arg; probably an environment
                                //   variable.  Ignore it.

                                cptr++;

                                for ( ; (( isalnum( *cptr )) || ( *cptr == '_' ) || ( *cptr == '$' )); cptr++ )
                                        ;

                                // ignore %% variables
                                if ( *cptr == '%' )
                                        cptr++;
                                continue;
                        }

                        // strip the '%'
                        strcpy( cptr, cptr+1 );

                        // %& defaults to %1&
                        argnum = (( *cptr == gpIniptr->ParamChr ) ? 1 : atoi( cptr ));
                        while ( isdigit( *cptr ))
                                strcpy( cptr, cptr+1 );

                        // flag highest arg processed
                        if ( argnum > vars_exist)
                                vars_exist = argnum;

                        // get matching argument from command line
                        arg = ntharg( line, argnum | 0x2000 );

                        if ( *cptr == gpIniptr->ParamChr ) {
                                // get command tail
                                strcpy( cptr, cptr+1 );
                                arg = gpNthptr;
                                vars_exist = 0xFF;
                        }

                        if ( arg == NULL )
                                continue;

                        // expand alias line
                        if (( strlen( aliasline ) + strlen( arg )) >= (CMDBUFSIZ - 1 ))
                                return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));
                        strins( cptr, arg );

                        // don't try to expand the argument we just added
                        cptr += strlen( arg );
                }

                // restore the end-of-command character (^, |, '\0', etc.)
                *eol = eolchar;

                // if alias variables exist, delete command line until
                //   highest referenced command, compound command char, pipe,
                //   conditional, or EOL ); else just collapse the alias name
                if ( vars_exist ) {

                        arg = scan( line + alen, NULL, QUOTES );

                        if (( ntharg( line + alen, vars_exist | 0x2000 ) != NULL ) && ( arg > gpNthptr )) {
                                arg = gpNthptr;
                                // preserve original whitespace (or lack)
                                if ( iswhite( arg[-1] ))
                                        arg--;
                        }

                } else
                        arg = line + alen;

                strcpy( line, arg );

                // check for overlength line
                if (( strlen( line ) + strlen( aliasline )) >= ( CMDBUFSIZ - 1 ))
                        return (error(ERROR_4DOS_COMMAND_TOO_LONG, NULL ));

                // insert the alias
                strins( line, aliasline );

                // check for nested variables disabled
                if ( gpIniptr->Expansion & EXPAND_NO_NESTED_ALIASES )
                        return 0;
        }
}


// expand shell variables
int var_expand( char *start_line, int fRecurse )
{
        static int recursive_loop_ctr;
        char *vline, *var;
        char szBuffer[MAXLINESIZ+8], func[12];
        char *line, *start_var, *old_var = NULL;
        int i, n, nLoopCount = 0, fAliasFunc;
        PCH feptr;

        // check for variable expansion disabled
        if ( gpIniptr->Expansion & EXPAND_NO_VARIABLES )
                return 0;

        // reset nLoopCount if initial entry into var_expand()
        if ( fRecurse == 0 )
                recursive_loop_ctr = 0;

        for ( line = start_line; ; ) {

                // Get the end of the first command & its args.
                //   We do this in two parts because scan() displays
                //   an error message which we don't want to see twice.

                if (( vline = scan( start_line, NULL, ((( *start_line == '(' ) && (fRecurse == 0 )) ? QUOTES_PARENS : QUOTES ))) == BADQUOTES )
                        return ERROR_EXIT;

                // don't do variable expansion inside command groups
                if (( line = scan( line, "%", ((( *line == '(' ) && ( fRecurse == 0 )) ? "`(" : BACK_QUOTE ))) == BADQUOTES )
                        return ERROR_EXIT;

                // make sure we're not getting infinite loopy here
                if ( line == old_var ) {
                        if ( ++nLoopCount > 16 )
                                return ( error( ERROR_4DOS_VARIABLE_LOOP, line ));
                } else {
                        nLoopCount = 0;
                        old_var = line;
                }

                fAliasFunc = 0;

                // only do variable substitution for first command on line

                // strip the %
                if (( *line == '\0' ) || ( line >= vline ) || ( *strcpy( line, line+1 ) == '\0' ) || ( line+1 >= vline ))
                        return 0;
                vline = line;

                if ( *vline == '%' ) {
                        // skip %% after stripping first one
                        line++;
                        continue;
                }

                // check for variable function or explicit variable
                // call var_expand() recursively if necessary to resolve
                //   nested variables

                func[0] = '\0';
                if (( *vline == '@' ) || ( *vline == '[' )) {

                        if ( *vline == '@' ) {
                                // get the variable function name
                                sscanf( ++vline, "%10[^[]%n", func, &n );
                                if ( func[0] == '\0' )
                                        return ( error( ERROR_4DOS_BAD_SYNTAX, line ));
                                vline += n;
                        }

                        // save args & call var_expand() recursively
                        // check for nested variable functions (delay
                        //   incrementing "vline" so we can get nested
                        //   functions)
                        if (( *vline != '[' ) || (*( var = scan( vline, "]", "\"`[" )) == '\0' ))
                                return ( error( ERROR_4DOS_BAD_SYNTAX, line ));

                        start_var = ++vline;

                        vline = var + 1;

                        // build new line composed of the function arguments
                        if (( n = (int)( var - start_var )) >= MAXLINESIZ-1 ) {
var_too_long:
                                return ( error( ERROR_4DOS_COMMAND_TOO_LONG, NULL ));
                        }

                        sprintf( szBuffer, FMT_PREC_STR, n, start_var );

                        // are there nested vars to resolve?
                        if ( strchr( szBuffer, '%' ) != NULL ) {

                                // make sure we don't go "infinite loopy" here
                                if ( ++recursive_loop_ctr > 10 ) {
                                        recursive_loop_ctr = 0;
                                        return ( error( ERROR_4DOS_VARIABLE_LOOP, NULL ));
                                }

                                // call ourselves to resolve nested vars
                                // set "fRecurse" so we don't do alias expansion
                                n = var_expand( szBuffer, 1 );

                                recursive_loop_ctr = 0;

                                if ( n )
                                        return n;
                        }

                        if ( func[0] ) {
                                // process the variable function
                                var = szBuffer;
                                if (( n = var_func( func, var )) == -6666 ) {
                                        // don't recurse %@ALIAS!
                                        n = 0;
                                        fAliasFunc = 1;
                                }

                                if ( n != 0 )
                                        return (( n > 0 ) ? ( error( n, line )) : -n);
                        } else
                                var = NULL;

                        // collapse the function name & args
                        strcpy( line, vline );

                } else {

                        // kludge for COMMAND.COM/CMD.EXE compatibility:
                        //   FOR variables may have a max length of 1!
                        //   Have to check FOR vars _before_ batch vars
                        sprintf( func, "\001%c", *vline );

                        if (( feptr = get_variable( func )) != 0L ) {

                                // collapse the variable spec
                                strcpy( line, vline+1 );
                                goto for_variable;
                        }

                        // get the environment (or internal) variable name
                        if (( cv.bn >= 0 ) && (( isdigit( *vline )) || ( *vline == gpIniptr->ParamChr ))) {

                                // parse normal batch file variables (%5)

                                // kludge for "%01" idiocy w/COMMAND.COM
                                if ( *vline == '0' )
                                        vline++;
                                else {
                                        while ( isdigit( *vline ))
                                                vline++;
                                }

                                // parse batch file %& variables
                                if ( *vline == gpIniptr->ParamChr )
                                        vline++;

                        } else if (( *vline == '#' ) || ( *vline == '?' )) {

                                // %# evaluates to number of args in batch file
                                // %? evaluates to exit code of last external program
                                // %?? evaluates to high byte of exit code (DOS)
                                vline++;

                        } else {

                                // parse environment variables (%var%)

                                // test for %_? ( internal errorlevel)
                                if (( vline[0] == '_' ) && ( vline[1] == '?' ))
                                    vline += 2;

                                else {

                                    for ( ; (( isalnum( *vline )) || ( *vline == '_' ) || ( *vline == '$' )); vline++ )
                                        ;

                                    // skip a trailing '%'
                                    if ( *vline == '%' )
                                        strcpy( vline, vline + 1 );
                                }
                        }

                        // variable names can't be > 80 characters
                        if (( n = ( int)( vline - line )) > 80 )
                                n = 80;
                        else if ( n == 0 )
                                continue;
                        sprintf( szBuffer, FMT_PREC_STR, n, line );
                        var = NULL;

                        // collapse the variable spec
                        strcpy( line, vline );
                }

                // if not a variable function, expand it
                if (( var == NULL ) && ( szBuffer[0] != '\0' )) {

                        // first, try expanding batch vars (%n, %&, %n&, and %#);
                        //   then check for internal variables
                        if ( cv.bn >= 0 ) {

                            // check for %n, %&, and %n&
                            for ( i = 0; ( szBuffer[i] != '\0' ); i++ ) {
                                if (( isdigit( szBuffer[i] ) == 0 ) && ( szBuffer[i] != gpIniptr->ParamChr ))
                                    break;
                            }

                            // expand %n, %&, and %n&
                            if ( szBuffer[i] == '\0' ) {

                                // %& defaults to %1&
                                vline = szBuffer;
                                i = (( *vline == gpIniptr->ParamChr ) ? 1 : atoi( vline )) + bframe[cv.bn].Argv_Offset;

                                for ( vline = szBuffer; isdigit( *vline ); vline++ )
                                        ;

                                for ( n = 0; ( n < i ) && ( bframe[cv.bn].Argv[n] != NULL ); n++ )
                                        ;

                                if ( *vline == gpIniptr->ParamChr ) {

                                        // get command tail
                                        var = szBuffer;
                                        for ( *var = '\0'; ( bframe[cv.bn].Argv[n] != NULL ); n++ ) {

                                                if (( strlen( var ) + strlen( bframe[cv.bn].Argv[n] )) >= MAXLINESIZ - 2 )
                                                        goto var_too_long;

                                                if ( *var )
                                                        strcat( var, " " );
                                                strcat( var, bframe[cv.bn].Argv[n] );
                                        }

                                } else
                                        var = bframe[cv.bn].Argv[n];

                            } else if ( szBuffer[0] == '#' ) {

                                // %# evaluates to number of args in batch file
                                //   (after adjusting for SHIFT)
                                for ( n = 0; ( bframe[cv.bn].Argv[ n + bframe[cv.bn].Argv_Offset ] != NULL ); n++ )
                                        ;

                                var = szBuffer;
                                IntToAscii((( n > 0 ) ? n - 1 : 0 ), var );
                            }
                        }

                        // check for an environment variable
                        if ( var == NULL ) {

                            if (( feptr = get_variable( szBuffer )) != 0L ) {

for_variable:
                                if ( strlen( feptr ) >= MAXLINESIZ-1 )
                                    goto var_too_long;

                                var = szBuffer;
                                strcpy( var, feptr );

                            } else {
                                // not a batch or env var; check for internal
                                var = var_internal( szBuffer );
                            }
                        }
                }

                // insert the variable
                if ( var != NULL ) {

                        i = strlen( var );
                        if (( strlen( start_line ) + i ) >= (unsigned int)(( start_line == gszCmdline ) ? CMDBUFSIZ - 1 : MAXLINESIZ - 1 ))
                                goto var_too_long;

                        strins( line, var );

                        // kludge to allow alias expansion when variable is
                        //   first arg on command line
                        var = first_arg( start_line );
                        if (( var != NULL ) && ( fRecurse == 0 ) && ((UINT)( line - start_line ) < strlen( var ))) {
                                if ( alias_expand( start_line ) != 0 )
                                        return ERROR_EXIT;
                        }

                        // check for nested variables disabled
                        if (( fAliasFunc ) || ( gpIniptr->Expansion & EXPAND_NO_NESTED_VARIABLES ))
                                line += i;
                }
        }
}


// process a variable function (%@...)
static int var_func(char *func, char *szBuffer )
{
        extern char *FUNC_ARRAY[];
        static FILESEARCH ffdir;
        static char szFindFilename[MAXFILENAME];
        char *var, *fname;
        int length, offset = 0, i = 0, n = 0;
        unsigned int day, month, year;
        unsigned long ram = 0L;
        long lOffset = 0L;
        char *fptr;
        char *arg, szDrive[512];
        QDISKINFO DiskInfo;
        FILESEARCH dir;

        // variable functions MUST have an argument!
        if (( fname = first_arg( szBuffer )) == NULL )
            fname = NULLSTR;

        // set drive arg (so both "a" and "a:" will work)
        if (( fname[0] == '\\' ) && ( fname[1] == '\\' ))
                strcpy( szDrive, fname );
        else
                sprintf( szDrive, FMT_DISK, ((( fname[0] == '\0' ) || (( fname[1] != '\0' ) && ( fname[1] != ':' ))) ? gcdisk( NULL ) + 64 : *fname ));

        for ( offset = 0, var = szBuffer; ( FUNC_ARRAY[offset] != NULL ); offset++ ) {

            // check FUNC_ARRAY for a function name match
            if ( _stricmp( func, FUNC_ARRAY[offset]) == 0 ) {

                switch ( offset ) {
                case FUNC_ALIAS:

                    // return the alias definition
                    if (( fptr = get_alias( var )) == 0L )
                        fptr = NULLSTR;
                    sprintf( szBuffer, FMT_FAR_PREC_STR, MAXLINESIZ-1, fptr );
                    return -6666;

                case FUNC_ASCII:

                    // return the ASCII value of the specified character
                    //   first, check for something like ASCII[^X`]
                    escape( var );
                    IntToAscii( *var, var );
                    break;

                case FUNC_ATTRIB:

                    // check file attributes
                    StripQuotes( fname );
                    if (( n = QueryFileMode( fname, (unsigned int *)&i)) == 0 ) {

                        // if no second arg, return current attributes as string
                        if (( fname = ntharg( var, 1 )) == NULL ) {
                                strcpy( var, show_atts( i ));
                                break;
                        }

                        // get the desired test (NRHSDA)
                        if ( AttributeString( fname, &n ) != 0 )
                            return ERROR_INVALID_PARAMETER;

                        // test file attributes against flags
                        if ((( fname = ntharg( var, 2 )) != NULL ) && (_ctoupper(*fname ) == 'P' ))
                                IntToAscii((( i & n) == n), var );
                        else
                                IntToAscii(( i == n ), var );

                    } else
                        *var = '\0';

                    break;

                case FUNC_CDROM:

                    // return 1 if the drive is a CD-ROM
                    IntToAscii( QueryIsCDROM( szDrive ), var );
                    break;

                case FUNC_CHAR:

                    // return the character for the specified ASCII value
                    sprintf( var, FMT_CHAR, atoi( var ));
                    break;

                case FUNC_CLIP:

                    // paste from the clipboard
                    if (( i = atoi( var )) < 0 )        // line to read
                        return ERROR_INVALID_PARAMETER;

                    n = -1;
                    *var = '\0';
                    // make sure clipboard has text data
                    // WinQueryClipbrdFmtInfo
                    if (( pfnWQCFI != NULL ) && ( (*pfnWQCFI)( ghHAB, CF_TEXT, &ram ) != 0 )) {

                        PCH lpClipMemory;

                        // WinOpenClipboard
                        if ( (*pfnWOC)( ghHAB ) != 0 ) {

                                // WinQueryClipbrdData
                                lpClipMemory = (PCH)(*pfnWQCD)( ghHAB, CF_TEXT );

                                // kludge for OS/2 bug - it doesn't make
                                //  clipboard memory readable for VIO apps!
                                if ( DosGetSharedMem( lpClipMemory, PAG_READ ) != 0 )
                                        lpClipMemory = NULLSTR;

                                // copy clipboard to the buffer
                                do {
                                        sscanf( lpClipMemory, "%[^\r\n]%*1[^\n]%*1[\n]%n", var, &offset );
                                        if ( i == 0 )
                                                break;
                                        lpClipMemory += offset;
                                } while (( *lpClipMemory != '\0' ) && ( i-- > 0 ));
                                n = i;

                                // WinCloseClipbrd
                                (*pfnWCC)( ghHAB );
                        }
                    }

                    if ( n != 0 )
                        var = "**EOC**";
                    break;

                case FUNC_COMMA:

                    // format a long integer by inserting commas (or other
                    // character specified by country_info.szThousandsSeparator)
                    AddCommas( var );
                    break;

                case FUNC_CONVERT:

                    // convert between two number bases
                    if (( sscanf( var, "%d ,%d , %n", &i, &n, &offset ) < 2 ) || ( i < 2 ) || ( n < 2 ) || ( n > 36 ))
                        return ERROR_INVALID_PARAMETER;

                    // "i" is input base; "n" is output base
                    var += offset;

                    for ( ; ( isalnum( *var )); var++ ) {
                        offset = (( isdigit( *var )) ? *var - '0' : ( _ctoupper( *var ) - 'A') + 10 );
                        ram = ( ram * i ) + offset;
                    }

                    var = szBuffer;
                    _ultoa( ram, var, n );
                    strupr( var );
                    break;

                case FUNC_DOW:          // day of week (Sun - Sat)
                case FUNC_DOWI:         // day of week (1-7)
                    if (( n = MakeDaysFromDate( (unsigned long *)&lOffset, var )) != 0 )
                        return n;
                    n = (int)(( lOffset + 2 ) % 7 );
                    if ( offset == FUNC_DOW )
                        strcpy( var, daytbl[n] );
                    else
                        IntToAscii( n+1, var );
                    break;

                case FUNC_DOY:          // day of year (1-366)
                case FUNC_DAY:
                case FUNC_MONTH:
                case FUNC_YEAR:
                    if ( GetStrDate( var, &month, &day, &year ) != 3 )
                        return ERROR_4DOS_INVALID_DATE;

                    if ( offset == FUNC_DOY ) {

                        // get days for previous months
                        for ( i = 1; ( i < ( int)month ); i++ ) {
                            if ( i == 2 )
                                day += ((( year % 4 ) == 0 ) ? 29 : 28 );
                            else
                                day += ((( i == 4 ) || ( i == 6 ) || ( i == 9 ) || ( i == 11 )) ? 30 : 31 );
                        }
                        IntToAscii( day, var );

                    } else if ( offset == FUNC_DAY )
                        IntToAscii( day, var );
                    else
                        IntToAscii( (( offset == FUNC_MONTH ) ? month : year ), var );
                    break;

                case FUNC_DATE:

                    // return number of days since 1/1/80 for a specified date
                    if (( n = MakeDaysFromDate(( unsigned long *)&ram, var )) != 0 )
                        return n;

                    sprintf( var, FMT_LONG, ram );
                    break;

                case FUNC_DESCRIPT:

                    // return description for specified file
                    var = szDrive;
                    *var = '\0';
                    process_descriptions( mkfname( fname, 0 ), var, DESCRIPTION_READ | DESCRIPTION_PROCESS );
                    break;

                case FUNC_DEVICE:

                    // return a 1 if the name is a character device
                    IntToAscii( QueryIsDevice( fname ), var );
                    break;

                case FUNC_DISKFREE:
                case FUNC_DISKTOTAL:
                case FUNC_DISKUSED:
                {
                    // TODO: LONGLONG llTemp;
                    double llTemp;

                    // return the disk stats
                    if ( QueryDiskInfo( szDrive, &DiskInfo, 0 ))
                        return -ERROR_EXIT;

                    if (( fname = ntharg( var, 1 )) != NULL ) {
                        i = *fname;
                        n = fname[1];
                    }
                    if ( offset == FUNC_DISKFREE )
                        llTemp = DiskInfo.BytesFree;
                    else if ( offset == FUNC_DISKTOTAL )
                        llTemp = DiskInfo.BytesTotal;
                    else
                        llTemp = ( DiskInfo.BytesTotal - DiskInfo.BytesFree );
                    // get the size value (B, K, or M)
                    // if user wants K or M, shift it
                    if ( i != 0 ) {

                        if ( i == 'k' )
                                llTemp /= 1000;
                        else if ( i == 'K' )
                                llTemp /= 1024;
                        else if ( i == 'm' )
                                llTemp /= 1000000;
                        else if ( i == 'M' )
                                llTemp /= 1048576;
                    }

                    sprintf( var, (( tolower( n ) == 'c' ) ? "%Lq" : "%q" ), llTemp );
                    break;
                }

                case FUNC_DOSMEM:

                    // get free RAM memory
                    // get free OS/2 memory
                    ram = QuerySysInfo( QSV_TOTAVAILMEM );
                    format_long( var, fname, ram );
                    break;

                case FUNC_INC:
                case FUNC_DEC:
                    // shorthand for %@EVAL[var+1] or %@EVAL[var-1]
                    sprintf( strend(var), (( offset == FUNC_INC ) ? "+1=0%c8" : "-1=0%c8" ), gaCountryInfo.szDecimal[0] );

                case FUNC_EVAL:

                    // calculate simple algebraic expressions
                    if ( evaluate( var ) != 0 )
                        return -ERROR_EXIT;
                    break;

                case FUNC_EXECUTE:
                case FUNC_EXECSTR:

                    // execute a command & return its result

                    // save gszCmdline onto stack
                    func = _alloca( strlen( gszCmdline ) + 1 );
                    strcpy( func, gszCmdline );
                    i = ( *var == '@' );

                    // return the (1st line) of the command output
                    if ( offset == FUNC_EXECSTR ) {
                        if ( GetTempDirectory( fname ) == 0L )
                            *fname = '\0';
                        UniqueFileName( fname );
                        arg = _alloca( strlen( fname ) + 1 );
                        fname = strcpy( arg, fname );
                        strins( var, "( " );
                        sprintf( strend( var ), " ) >!%s", fname );
                    }

                    if ( offset == FUNC_EXECSTR ) {

                        // disable echoing so we don't get the wrong result
                        if ( cv.bn >= 0 ) {
                            n = bframe[cv.bn].echo_state;
                            bframe[cv.bn].echo_state = 0;
                        }
                        i = cv.verbose;
                        cv.verbose = 0;

                        DoINT2E( var );
                        if ( cv.bn >= 0 )
                            bframe[cv.bn].echo_state = n;

                        cv.verbose = i;
                        lOffset = 0;

                        // restore gszCmdline
                        strcpy( gszCmdline, func );
                        goto GetFirstLine;
                    }

                    // don't return result if line began with '@'
                    if ( i == 1 )
                        var = NULLSTR;
                    else
                        IntToAscii( DoINT2E( var ), var );

                    // restore gszCmdline
                    strcpy( gszCmdline, func );
                    break;

                case FUNC_EXPAND:

                    // do a wildcard expansion of the filename argument
                    if (( *fname == '\0' ) || ( mkfname( fname, MKFNAME_NOERROR ) == NULL ))
                        return ERROR_INVALID_PARAMETER;
                    arg = _alloca( strlen( fname ) + 1 );
                    fname = strcpy( arg, fname );
                    GetSearchAttributes( ntharg( var, 1 ));

                    // return all matching files
                    *var = '\0';
                    for ( n = FIND_FIRST; ( find_file( n, fname, 0x9517, &dir, szDrive ) != NULL ); n = FIND_NEXT ) {
                        if ( strlen( var ) + strlen( szBuffer ) + 2 >= (MAXLINESIZ - 1)) {
                            DosFindClose( dir.hdir );
                            return ERROR_4DOS_COMMAND_TOO_LONG;
                        }
                        if ( n != FIND_FIRST )
                                strcat( var, " " );
                        AddQuotes( szDrive );
                        strcat( var, szDrive );
                    }
                    break;

                case FUNC_EXTENSION:

                    // extension
                    StripQuotes( fname );
                    if ((( var = ext_part( fname )) != NULL ) && ( *var == '.' ))
                        var++;
                    break;

                case FUNC_FILECLOSE:

                    // close the specified file handle
                    if (( i = atoi( var )) <= 2 )
                        return ERROR_INVALID_HANDLE;
                    if ( QueryIsPipeHandle( i ))
                        IntToAscii( DosClose( i ), var );
                    else
                            IntToAscii( _close( i ), var );
                    break;

                case FUNC_FILENAME:

                    // filename
                    StripQuotes( var );
                    var = fname_part( var );
                    break;

                case FUNC_FILEAGE:      // age of file (as a long)
                case FUNC_FILEDATE:     // file date
                case FUNC_FILETIME:     // file time

                    if ( find_file( FIND_FIRST, fname, 0x2017, &dir, NULL ) != NULL ) {

                        // check for LastWrite, LastAccess, or Creation
                        n = 0;
                        if (( fname = ntharg( var, 1 )) != NULL ) {
                                strupr( fname );
                                if ( *fname == 'A' )
                                        n = 1;
                                else if ( *fname == 'C' )
                                        n = 2;
                        }

                        if ( offset == FUNC_FILEAGE ) {
                            if ( n == 0 ) {
                                ram = dir.fdLW.fdLWrite;
                                ram = ( ram << 16 ) + dir.ftLW.ftLWrite;
                            } else if ( n == 1 ) {
                                ram = dir.fdLA.fdLAccess;
                                ram = ( ram << 16 ) + dir.ftLA.ftLAccess;
                            } else {
                                ram = dir.fdC.fdCreation;
                                ram = ( ram << 16 ) + dir.ftC.ftCreation;
                            }
                            sprintf( var, FMT_ULONG, ram );

                        } else if ( offset == FUNC_FILEDATE ) {

                            // replace leading space with a 0
                            if ( n == 0 ) {
                                if (*( var = FormatDate( dir.fdLW.fdateLastWrite.month, dir.fdLW.fdateLastWrite.day, dir.fdLW.fdateLastWrite.year+80 )) == ' ' )
                                    *var = '0';
                            } else if ( n == 1 ) {
                                if (*( var = FormatDate( dir.fdLA.fdateLastAccess.month, dir.fdLA.fdateLastAccess.day, dir.fdLA.fdateLastAccess.year+80 )) == ' ' )
                                    *var = '0';
                            } else {
                                if (*( var = FormatDate( dir.fdC.fdateCreation.month, dir.fdC.fdateCreation.day, dir.fdC.fdateCreation.year+80 )) == ' ' )
                                    *var = '0';
                            }

                        } else {
                            // file time
                            if ( n == 0 )
                                sprintf( var, "%02u%c%02u", dir.ftLW.ftimeLastWrite.hours, gaCountryInfo.szTimeSeparator[0], dir.ftLW.ftimeLastWrite.minutes );
                            else if ( n == 1 )
                                sprintf( var, "%02u%c%02u", dir.ftLA.ftimeLastAccess.hours, gaCountryInfo.szTimeSeparator[0], dir.ftLA.ftimeLastAccess.minutes );
                            else
                                sprintf( var, "%02u%c%02u", dir.ftC.ftimeCreation.hours, gaCountryInfo.szTimeSeparator[0], dir.ftC.ftimeCreation.minutes );
                        }

                    } else
                        *var = '\0';
                    break;

                case FUNC_FILEOPEN:

                    // open the specified file & return its handle
                    if (( fname = ntharg( var, 1 )) == NULL )
                        return ERROR_INVALID_PARAMETER;

                    if ( _stricmp( fname, OPEN_READ ) == 0 )
                        n = O_RDONLY | O_BINARY;
                    else if ( _stricmp( fname, OPEN_WRITE ) == 0 )
                        n = O_WRONLY | O_TEXT | O_CREAT | O_TRUNC;
                    else if ( _stricmp( fname, OPEN_APPEND ) == 0 )
                        n = O_RDWR | O_TEXT | O_CREAT;
                    else
                        return ERROR_INVALID_PARAMETER;

                    // check for "binary" mode
                    if ((( fname = ntharg( var, 2 )) != NULL ) && (( *fname == 'B' ) || ( *fname == 'b' ))) {
                        n &= ~O_TEXT;
                        n |= O_BINARY;
                        // binary mode assumes non-truncate!
                        if ( n & O_WRONLY )
                            n &= ~O_TRUNC;
                    }

                    fname = first_arg( var );
                    StripQuotes( fname );

                    // first try a normal open
                    if (( i = _sopen( fname, n, (( n & O_RDONLY ) ? SH_DENYWR : SH_COMPAT), ( S_IREAD | S_IWRITE) )) >= 0 ) {

                        // if appending, go to EoF
                        if ( n & O_RDWR ) {
                            if ( n & O_BINARY )
                                QuerySeekSize( i );
                            else
                                SeekToEnd( i );
                        }
                    }

                    // if open failed & it's a pipe, try to create it
                    if (( i == -1 ) && ( QueryIsPipeName( fname ))) {

                        if (( i = DosCreateNPipe( fname, (unsigned long *)&n,
                          ( NP_INHERIT | NP_ACCESS_DUPLEX ),
                          ( NP_NOWAIT | NP_TYPE_BYTE | NP_READMODE_BYTE | 0x7F ),
                          4096, 4096, 0 )) != 0 )
                                return i;

                        DosConnectNPipe( n );
                        DosSetNPHState( (HPIPE)n, ( NP_WAIT | NP_READMODE_BYTE ));
                        i = _hdopen( (LONG)n, O_RDWR | O_TEXT );
                    }

                    var = szBuffer;
                    IntToAscii( i, var );
                    break;

                case FUNC_FILEREAD:

                    // read a line from the specified file handle
                    // second argument is optional read length
                    if (( sscanf( var, "%d ,%d", &i, &n ) < 1 ) || ( i < 3 ) || ( n < 0 ))
                        return ERROR_INVALID_PARAMETER;

                    if ( n == 0 ) {
                        if ( getline( i, var, MAXARGSIZ, EDIT_DATA ) <= 0 )
                            var = END_OF_FILE_STR;
                    } else {
                        if ( n > MAXARGSIZ )
                                n = MAXARGSIZ;
                        if (( n = _read( i, var, n )) <= 0 )
                                var = END_OF_FILE_STR;
                        else
                                var[n] = '\0';
                    }

                    break;

                case FUNC_FILES:

                    // get the desired test (NRHSDA)
                    init_dir();

                    // kludge for %@files[xxx,n]
                    fname = ntharg( var, 1 );
                    offset = ((( fname != NULL ) && (_stricmp( fname, "n" ) == 0)) ? 0x1507 : 0x1517 );

                    // set inclusive/exclusive modes
                    GetSearchAttributes( fname );
                    if (( fname = first_arg( var )) == NULL )
                        return ERROR_INVALID_PARAMETER;

                    // return number of matching files
                    for ( i = 0, n = FIND_FIRST; ( find_file( n, fname, offset, &dir, NULL ) != NULL ); i++, n = FIND_NEXT)
                        ;
                    IntToAscii( i, var );
                    break;

                case FUNC_FILESEEK:

                    // seek to the specified offset
                    if (( sscanf( var, "%d ,%ld ,%d", &i, &lOffset, &n ) != 3 ) || ( i < 3 ) || ((unsigned int)n > 2 ))
                        return ERROR_INVALID_PARAMETER;

                    sprintf( var, FMT_LONG, _lseek( i, lOffset, n ));
                    break;

                case FUNC_FILESEEKL:

                    if (( sscanf( var, "%d ,%ld", &i, &lOffset ) != 2 ) || ( i < 3 ))
                        return ERROR_INVALID_PARAMETER;

                    // rewind & read a line from the file
                    RewindFile( i );
                    for ( ; ( lOffset > 0L ); lOffset-- ) {
                        if ( getline( i, var, MAXARGSIZ, EDIT_DATA ) <= 0 )
                            break;
                    }
                    sprintf( var, FMT_LONG, _lseek( i, 0L, SEEK_CUR ));
                    break;

                case FUNC_FILESIZE:

                    // display allocated size?
                    if ((( var = ntharg( var, 2 )) != NULL ) && ( _ctoupper( *var ) == 'A' ))
                        i = 1;

                    fname = first_arg( szBuffer );
                    if (( fname == NULL ) || ( mkfname( fname, MKFNAME_NOERROR ) == NULL ))
                        return ERROR_INVALID_PARAMETER;

                    // file size test
                    if (( ram = (ULONG)QueryFileSize( fname, i )) != (ULONG)-1L ) {

                        // get the size value (B, K, or M)
                        if (( fname = ntharg( szBuffer, 1 )) == NULL )
                            fname = NULLSTR;

                        // round file sizes upwards (to match DIR /4)
                        if ( *fname == 'k' )
                                ram += 999;
                        else if ( *fname == 'K' )
                                ram += 1023;
                        else if ( *fname == 'm' )
                                ram += 999999L;
                        else if ( *fname == 'M' )
                                ram += 1048575L;

                        var = szBuffer;
                        format_long( var, fname, ram );

                    } else
                        var = "-1";

                    break;

                case FUNC_FILEWRITE:

                    // write a line to the specified file
                    sscanf( var, "%d %*1[,]%n", &i, &n );

                    if ( i < 1 )
                        return ERROR_INVALID_PARAMETER;

                    if (( n = qprintf( i, FMT_STR_CRLF, var + n )) > 0 )
                        n++;
                    IntToAscii( n, var );
                    break;

                case FUNC_FILEWRITEB:

                    // write chars to the specified file
                    sscanf( var, "%d,%d %*1[,]%n", &i, &length, &n );

                    if ( i < 3 )
                        return ERROR_INVALID_PARAMETER;

                    n = _write( i, var+n, length );
                    IntToAscii( n, var );
                    break;

                case FUNC_FINDFIRST:
                    // initialize the search structure
                    memset( &ffdir, '\0', sizeof(FILESEARCH) );

                    // get filespec
                    if (( fname = first_arg( var )) == NULL )
                        return ERROR_INVALID_PARAMETER;
                    copy_filename( szFindFilename, fname );

                case FUNC_FINDNEXT:

                    // find first/next file
                    init_dir();

                    // set inclusive/exclusive modes
                    GetSearchAttributes( ntharg( var, 1 ));

                    if ( find_file((( offset == FUNC_FINDFIRST) ? FIND_FIRST : FIND_NEXT), szFindFilename, 0x9517, &ffdir, var ) == NULL ) {
                            *var = '\0';
                            break;
                    }

                    offset = FUNC_FINDNEXT;
                    break;

                case FUNC_FINDCLOSE:

                    // close the "find file" search handle
                    IntToAscii( DosFindClose( ffdir.hdir ), var );
                    break;

                case FUNC_FORMAT:

                    // format a string
                    fname = scan( var, ",", QUOTES );
                    if (( *fname != ',' ) || ( fname[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;
                    *fname++ = '\0';

                    // save format and source strings
                    arg = _alloca( strlen( var ) + 3 );
                    var = strcpy( arg, var );
                    arg = _alloca( strlen( fname ) + 1 );
                    fname = strcpy( arg, fname );
                    strins( var, "%" );
                    strcat( var, "s" );
                    sprintf( szBuffer, var, fname );
                    var = szBuffer;
                    break;

                case FUNC_FULLNAME:

                    // return fully qualified filename
                    var = mkfname( fname, 0 );
                    break;

                case FUNC_IF:
                    {
                    extern int TestCondition(char *, int);

                    // return a value based upon the result of the condition
                    var = scan( var, ",", QUOTES );
                    if (( *var != ',' ) || ( var[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;

                    *var++ = '\0';
                    arg = _alloca( strlen( var ) + 1 );
                    var = strcpy( arg, var );

                    n = TestCondition( szBuffer, 2 );

                    // reset the default switch character
                    gpIniptr->SwChr = '/';
                    if ( n == -USAGE_ERR)
                        return ERROR_4DOS_BAD_SYNTAX;

                    szBuffer[0] = '\0';
                    if ( n > 0 )
                        sscanf( var, "%[^,]", szBuffer );
                    else {
                        // n == 0
                        sscanf( var, "%*[^,],%[^\032]", szBuffer );
                    }
                    var = szBuffer;
                    break;
                    }

                case FUNC_INDEX:

                    // return the index of the first part of the source string
                    //   that includes the search substring
                    var = scan( var, ",", QUOTES );
                    if ((*var != ',' ) || ( var[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;

                    *var++ = '\0';

                    // Perform simple case-insensitive search
                    EscapeLine( szBuffer );
                    EscapeLine( var );
                    for ( n = strlen( var ); (( szBuffer[i] ) && ( _strnicmp( szBuffer+i, var, n ) != 0 )); i++ )
                        ;

                    if ( szBuffer[i] == '\0' )
                        i = -1;

                    var = szBuffer;
                    IntToAscii( i, var );
                    break;

                case FUNC_INTEGER:

                    // return the integer part
                    fname = "%[, 0123456789]";
                    fname[2] = gaCountryInfo.szThousandsSeparator[0];

                    // kludge for stupid people who do %@integer[.1234]
                    if ( *var == gaCountryInfo.szDecimal[0] ) {
                        var = "0";
                        break;
                    }

                    // kludge for stupid people who do "%@integer[10-10-94]"
                    i = ((( *var == '-' ) || ( *var == '+' )) && ( isdigit( var[1] )));
                    sscanf( var+i, fname, var+i );

                    // kludge for +/- 0
                    if (( i > 0 ) && ( stricmp( var+i, "0" ) == 0 ))
                        strcpy( var, var+i );
                    break;

                case FUNC_INSERT:

                    // insert a string into another
                    if (( sscanf( var, "%d ,%n", &i, &n ) < 1 ) || ( i < 0 ))
                        return ERROR_INVALID_PARAMETER;

                    var += n;
                    fname = var;
                    var = scan( var, ",", QUOTES );
                    if (*var != ',' )
                        return ERROR_INVALID_PARAMETER;
                    *var++ = '\0';

                    if ( i > (int)strlen( var ))
                        i = strlen( var );
                    strins( var + i, fname );
                    break;

                case FUNC_LABEL:

                    // get the volume label for the specified drive
                    if ( QueryVolumeInfo( szDrive, var, &ram ) == NULL )
                        *var = '\0';
                    break;

                case FUNC_LEFT:
                case FUNC_RIGHT:

                    // substring - check for valid syntax
                    //   LEFT[length,string]    RIGHT[length,string]

                    if ( sscanf( var, "%d ,%n", &i, &n ) < 1 )
                        return ERROR_INVALID_PARAMETER;

                    var += n;
                    if ( offset == FUNC_LEFT )
                        sprintf( szBuffer, FMT_PREC_STR, i, var );
                    else {
                        n = strlen( var );
                        if ( i > n )
                                i = n;
                        sprintf( szBuffer, FMT_STR, strend( var ) - i );
                    }
                    var = szBuffer;
                    break;

                case FUNC_LENGTH:

                    // length of string
                    IntToAscii( strlen( var ), var );
                    break;

                case FUNC_LINE:

                    // get line from file
                    if (( sscanf( var, "%*[^,],%ld", &lOffset ) != 1 ) || ( lOffset < 0L ))
                        return ERROR_INVALID_PARAMETER;

                case FUNC_LINES:        // get # of lines in file

                    if ( offset == FUNC_LINES )
                        lOffset = LONG_MAX;
                    if ( QueryIsCON( fname ))
                        gnGFH = STDIN;

                    else {

                        if (( *fname ) && ( mkfname( fname, 0 ) == NULL ))
                            return -ERROR_EXIT;
GetFirstLine:
                        if (( gnGFH = _sopen( fname, (O_RDONLY | O_BINARY), SH_DENYWR )) < 0 ) {
                                return ERROR_4DOS_CANT_OPEN;
                        }
                    }

                    // read a line from the file
                    for ( ; ( lOffset >= 0L ); lOffset-- ) {
                        if ( getline( gnGFH, var, MAXLINESIZ-1, EDIT_DATA ) <= 0 ) {
                            if ( offset == FUNC_LINE )
                                var = END_OF_FILE_STR;
                            break;
                        }
                    }

                    if ( gnGFH )
                        gnGFH = _close( gnGFH );

                    if ( offset == FUNC_LINES )
                        sprintf( var, FMT_LONG, ( LONG_MAX - ( lOffset + 1 )));
                    else if ( offset == FUNC_EXECSTR )
                        remove( fname );

                    break;

                case FUNC_LOWER:

                    // shift string to lower case
                    strlwr( var );
                    break;

                case FUNC_MAKEAGE:

                    // make an "age long" from the specified date & time
                    month = day = year = 0;
                    if ( GetStrDate( var, &month, &day, &year ) != 3 )
                        return ERROR_4DOS_INVALID_DATE;

                    if ( year > 1900 )
                        year -= 1900;
                    if ( year >= 80 )
                        year -= 80;
                    ram = ( day + ( month << 5 ) + ( year << 9 ));
                    ram <<= 16;

                    for ( fname = var; (( *fname != '\0' ) && ( *fname++ != ',' )); )
                        ;

                    day = month = year = 0;
                    sscanf( fname, "%u%*c%u%*c%u", &day, &month, &year );
                    if (( day >= 24 ) || ( month >= 60 ) || ( year >= 60 ))
                        return ERROR_4DOS_INVALID_TIME;

                    ram += (ULONG)(( year / 2 ) + ( month << 5 ) + ( day << 11 ));
                    sprintf( var, FMT_ULONG, ram );
                    break;

                case FUNC_MAKEDATE:

                    // make a formatted date from the number of days
                    sscanf( var, FMT_ULONG, &ram );

                    if (( n = MakeDateFromDays( ram, &year, &month, &day )) != 0 )
                        return n;

                    // replace leading space with a 0
                    if (*( var = FormatDate( month, day, year )) == ' ' )
                        *var = '0';
                    break;

                case FUNC_MAKETIME:

                    // make a formatted time from the number of seconds
                    sscanf( var, FMT_ULONG, &ram );

                    // don't allow anything past 23:59:59
                    if ( ram >= 86400L )
                        return ERROR_INVALID_PARAMETER;

                    sprintf( var, "%02lu%c%02lu%c%02lu", ( ram / 3600 ), gaCountryInfo.szTimeSeparator[0], ((ram % 3600 ) / 60 ), gaCountryInfo.szTimeSeparator[0], (ram % 60 ) );
                    break;

                case FUNC_NAME:

                    // filename
                    StripQuotes( fname );
                    var = fname_part( fname );
                    if (( fname = strrchr( var, '.' )) != NULL )
                        *fname = '\0';
                    break;

                case FUNC_NUMERIC:

                    // returns 1 if the argument is numeric
                    IntToAscii( QueryIsNumeric( var ), szBuffer );
                    var = szBuffer;
                    break;

                case FUNC_PATH:

                    // path part
                    StripQuotes( fname );
                    var = path_part( fname );
                    break;

                case FUNC_RANDOM:
                    {
                    // get min & max
                    long lStart, lEnd;
                    if (( sscanf( var, "%ld ,%ld", &lStart, &lEnd ) != 2 ) || ( lStart > lEnd))
                        return ERROR_INVALID_PARAMETER;

                    sprintf( var, FMT_LONG, GetRandom( lStart, lEnd ));
                    break;
                    }

                case FUNC_READSCR:

                    // read a string from the screen
                    if (( GetCursorRange( var, &i, &n ) != 0 ) || ( sscanf( var, "%*d ,%*d ,%d",&length) != 1 ))
                        return ERROR_INVALID_PARAMETER;

                    if ( length >= MAXARGSIZ - 1 )
                        length = MAXARGSIZ - 2;
                    for ( offset = 0; ( offset < length ); offset++, n++ ) {
                        day = 2;
                        (void)VioReadCellStr((char *)( var+offset), (PUSHORT)&day, i,n, 0 );
                    }

                    var[offset] = '\0';
                    break;

                case FUNC_READY:

                    // is specified drive ready?
                    IntToAscii( QueryDriveReady( gcdisk( szDrive )), var );
                    break;

                case FUNC_REMOTE:

                    // is specified drive remote?
                    if ( is_net_drive( szDrive ))
                        IntToAscii( 1, var );
                    else
                        IntToAscii( QueryDriveRemote( gcdisk( szDrive )), var );
                    break;

                case FUNC_REMOVABLE:

                    // is specified drive removable?
                    IntToAscii( QueryDriveRemovable( gcdisk( szDrive )), var );
                    break;

                case FUNC_REPEAT:

                    // repeat character "n" times
                    if (( sscanf( var, "%c%*[^,],%d", &i, &n ) < 1 ) || ( (unsigned int)n > 255 ))
                        return ERROR_INVALID_PARAMETER;
                    memset( var, (char)i, n );
                    var[n] = '\0';
                    break;

                case FUNC_REPLACE:
                {
                    // replaces occurrences of "str1" with "str2"
                    var = scan( szBuffer, ",", QUOTES );
                    if (( *var != ',' ) || ( var[1] == '\0' ) || ( var == szBuffer ))
                        return ERROR_INVALID_PARAMETER;

                    *var++ = '\0';
                    EscapeLine( szBuffer );
                    fname = strdup( szBuffer );

                    arg = scan( var, ",", QUOTES );
                    if (( *arg != ',' ) || ( arg[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;

                    *arg++ = '\0';
                    EscapeLine( var );
                    var = strdup( var );
                    strcpy( szBuffer, arg );

                    arg = szBuffer;
                    if ( *arg == '\0' )
                        return ERROR_INVALID_PARAMETER;

                    while ((( arg = strstr( arg, fname )) != NULL ) && (( strlen( szBuffer ) + 2 ) < MAXLINESIZ )) {
                        strcpy( arg, arg + strlen( fname ));
                        strins( arg, var );
                        arg += strlen( var );
                    }

                    free( fname );
                    free( var );
                    var = szBuffer;
                    break;
                }
                case FUNC_REXX:
                    {
                        // execute the REXX expression
                        typedef LONG APIENTRY RXSTART( LONG, PRXSTRING, PSZ, PRXSTRING, PSZ, LONG, PRXSYSEXIT, PSHORT, PRXSTRING );
                        extern RXSTART *pfnREXXSTART;
                        SHORT rexxrc = 0, fReturn = 0;
                        char szInstoreBuf[MAXLINESIZ+10];
                        RXSTRING rexx_cmd, rexx_result, Instore[2];
                        CRITICAL_VARS save_cv;

                        // initialize the REXX back door
                        if ( InitializeREXX( 1 ))
                                return ERROR_EXIT;

                        // call the REXX interpreter for a single command
                        var = skipspace( var );
                        if ( *var == '=' ) {
                                fReturn = 1;
                                sprintf( szInstoreBuf, "return %s\r\n\032",var+1 );
                        } else
                                sprintf( szInstoreBuf, "%s\r\n\032", var );

                        Instore[0].strptr = szInstoreBuf;
                        Instore[0].strlength = strlen( szInstoreBuf );
                        Instore[1].strptr = 0L;
                        Instore[1].strlength = 0;

                        // save the arguments in REXX string format
                        rexx_cmd.strlength = (ULONG)strlen( var );
                        rexx_cmd.strptr = (PCH)var;

                        rexx_result.strlength = 0;
                        rexx_result.strptr = 0L;

                        // save the critical variables
                        memmove( &save_cv, &cv, sizeof(cv) );
                        // OS2 2.x and NT
                        if ((*pfnREXXSTART)( 1, &rexx_cmd, first_arg( var ), Instore, "CMD", RXSUBROUTINE, 0L, &rexxrc, &rexx_result ) != 0 )
                                return ERROR_EXIT;

                        if ( rexx_result.strptr != 0L ) {
                                rexx_result.strptr[ rexx_result.strlength ] = '\0';
                                strcpy( var, rexx_result.strptr );
                        } else if ( fReturn )
                                IntToAscii( rexxrc, var );
                        else
                                *var = '\0';

                        // free any memory allocated w/REXX "EXIT" or "RETURN"
                        FreeMem( rexx_result.strptr );

                        // restore the critical vars
                        memmove( &cv, &save_cv, sizeof(cv) );
                        break;
                    }

                case FUNC_SEARCH:

                    // search for filename
                    // defaults to search in PATH
                    if (( var = strchr( var, ',' )) != NULL )
                        var = skipspace( ++var );
                    else
                        var = NULL;

                    if (( var = searchpaths( fname, var, TRUE )) != NULL )
                        mkfname( var, 0 );
                    break;

                case FUNC_SELECT:               // popup a selection window
                    {

                    int top, left, bottom, right, sort = 0;
                    // 4DOS gets 64K; everything else gets realloc'd
                    unsigned int uListSize = 0xFFF0;
                    unsigned long size = 0L;
                    char * *list = 0L;
                    char title[80];

                    // get line from file - check for valid syntax
                    if (( sscanf( var, "%*[^,],%d ,%d ,%d ,%d %*[,]%79[^,],%d", &top, &left, &bottom, &right, title, &sort ) < 4 ) || (( bottom - top ) < 2 ) || (( right - left ) < 2 ))
                        return ERROR_INVALID_PARAMETER;

                    // only set fd to STDIN if we're reading from a disk file
                    if ( QueryIsCON( fname ))
                        n = STDIN;

                    else {

                        if (( *fname ) && ( mkfname( fname, 0 ) == NULL ))
                            return -ERROR_EXIT;

                        if (( n = _sopen( fname, (O_RDONLY | O_BINARY), SH_DENYWR)) < 0 ) {
                                return ERROR_4DOS_CANT_OPEN;
                        }
                    }

                    HoldSignals();

                    fptr = AllocMem( &uListSize );
                    uListSize -= 0xFF;

                    for ( i = 0; ; i++ ) {

                        // allocate memory for 256 entries at a time
                        if (( i % 256 ) == 0 ) {
                            size += 1024;
                            list = (char * *)ReallocMem( (char *)list, size );
                            if (( i == 0 ) && ( list != 0L ))
                                list[0] = fptr;
                        }

                        if ( getline( n, szBuffer, MAXARGSIZ, EDIT_DATA ) <= 0 )
                            break;

                        length = strlen( szBuffer ) + 1;

                        if (( list == 0L ) || ( list[0] == 0L ) || (((unsigned int)( fptr - list[0]) + length ) >= uListSize )) {
                            uListSize += 0xFF;
                            uListSize += 0xFFF0L;
                            fptr = ReallocMem( fptr, uListSize );
                            uListSize -= 0xFF;
                            if ( fptr == 0L ) {
                                _close( n );
                                FreeMem( list[0] );
                                FreeMem( (char *)list );
                                return ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }

                        strcpy( fptr, szBuffer );
                        list[i] = fptr;
                        fptr += length;
                    }

                    // if reading from STDIN, reset it to the console
                    if ( n == 0 )
                        _dup2( STDOUT, STDIN );
                    else
                        _close( n );

                    EnableSignals();

                    if ( i == 0 ) {
                            FreeMem( list[0]);
                            FreeMem( (char *)list );
                            return ERROR_4DOS_FILE_EMPTY;
                    }

                    *(++fptr ) = '\0';

                    // call the popup window
                    if (( fptr = wPopSelect( top, left, ( bottom - top ) - 1, ( right - left ) - 1, list, i, 1, title, NULL, NULL, sort )) != 0L )
                        strcpy( var, fptr );
                    else
                        var = NULL;

                    // free the pointer array & list memory
                    FreeMem( list[0] );
                    FreeMem( (char *)list );

                    // reenable signal handling after cleanup
                    EnableSignals();

                    // if we aborted wPopSelect with ^C, bomb after cleanup
                    if ( cv.exception_flag )
                        longjmp( cv.env, -1 );

                    break;
                    }

                case FUNC_STRIP:

                    // remove specified characters from string
                    fname = scan( var, ",", QUOTES );
                    if (( *fname != ',' ) || ( fname[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;
                    *fname++ = '\0';
                    i = strlen( var ) - 1;
                    if (( *var == '"' ) && ( var[i] == '"' )) {
                        var[ i ] = '\0';
                        strcpy( var, var + 1 );
                    }
                    EscapeLine( var );

                    while (( var = strpbrk( fname, szBuffer )) != NULL )
                        strcpy( var, var+1 );
                    var = fname;
                    break;

                case FUNC_INSTR:

                    // substring - check for valid syntax
                    //   INSTR[start,length,string]

                    if ((( n = sscanf( var, "%d ,%d %*1[,]%n", &i, &length, &offset )) < 2 ) || ( i < 0 ))
                        return ERROR_INVALID_PARAMETER;
                    if ( n <= 2 )
                        n = INT_MAX;
                    else
                        n = length;
                    var += offset;
                    goto get_substr;

                case FUNC_SUBSTR:

                    // substring - check for valid syntax
                    //   SUBSTR[string,start,length]

                    var = scan( var, ",", QUOTES );
                    if ( *var != ',' )
                        return ERROR_INVALID_PARAMETER;

                    *var++ = '\0';

                    // default to remainder of string
                    n = INT_MAX;
                    if (( sscanf( var, "%d ,%d", &i, &n ) < 1 ) || ( i < 0 ))
                        return ERROR_INVALID_PARAMETER;
                    var = szBuffer;
get_substr:
                    length = strlen( var );

                    if ( n > 0 )
                        var += (( length > i) ? i : length);
                    else {
                        n = -n;
                        length--;
                        var += (( length > i) ? length - i : 0 );
                    }

                    sprintf( var, FMT_PREC_STR, n, var );
                    break;

                case FUNC_TIME:

                    // return number of seconds since midnight
                    sscanf( var, "%lu%*c%u%*c%u", &ram, &i, &n );
                    if ((ram > 24) || ( i >= 60 ) || ( n >= 60 ))
                        return ERROR_4DOS_INVALID_TIME;
                    sprintf( var, FMT_ULONG, ( ram * 3600L ) + (unsigned long)(( i * 60 ) + n ));
                    break;

                case FUNC_TIMER:

                    // return current elapsed time for specified timer
                    i = atoi( var );
                    if (( i > 3 ) || ( i < 1 ))
                        return ERROR_INVALID_PARAMETER;

                    // system date
                    _timer( i - 1, var );
                    break;

                case FUNC_TRIM:

                    // return string with leading & trailing whitespace stripped
                    strip_trailing( var, WHITESPACE );
                    strcpy( var, skipspace( var ));
                    break;

                case FUNC_UNIQUE:

                    // Creat a unique filename
                    // if no path specified, use the current directory
                    StripQuotes( fname );
                    strcpy( var, ((*fname == '\0' ) ? gcdir( NULL, 1 ) : fname ));

                    if ( mkfname( var, 0 ) == NULL )
                        return -ERROR_EXIT;

                    if (( n = UniqueFileName( var )) != 0 )
                        return n;

                    break;

                case FUNC_UPPER:

                    // shift string to upper case
                    strupr( var );
                    break;

                case FUNC_WILD:

                    // wildcard comparison
                    fname = var;
                    var = scan( var, ",", QUOTES );
                    if ( *var != ',' )
                        return ERROR_INVALID_PARAMETER;
                    *var++ = '\0';

                    var = (( wild_cmp( var, fname, 1, TRUE ) == 0 ) ? "1" : "0" );
                    break;

                case FUNC_WORD:
                case FUNC_WORDS:

                    // get the i'th word from the line (base 0 )
                    // or the total number of words on the line (@WORDS)

                    // were custom delimiters requested?
                    if ( *var == '"' ) {
                        i = strlen( fname );
                        if ( fname[i-1] == '"' )
                                fname[i-1] = '\0';
                        EscapeLine( ++fname );
                        var += i + 1;
                    } else
                        fname = " ,\t";

                    if ( offset == FUNC_WORD ) {
                        // check for negative offset (scan backwards from end)
                        if (( n = *var ) == '-' )
                            var++;
                        if ( sscanf( var, "%d ,%[^\n]", &i, var ) < 1 )
                            return ERROR_INVALID_PARAMETER;
                    } else
                      i = 0x4000;

                    var = strcpy( szBuffer, var );

                    if (( n == '-' ) && ( *var != '\0' ))
                        var = strend( var ) - 1;

                    for ( ; ; ) {

                        char cQuote, *ptr;

                        // find start of arg[i]
                        while (( *var != '\0' ) && ( var >= szBuffer ) && ( strchr( fname, *var ) != NULL ))
                            var += (( n == '-' ) ? -1 : 1 );

                        // search for next delimiter character
                        for ( ptr = var, cQuote = 0; (( *var != '\0' ) && ( var >= szBuffer )); ) {

                            // ignore whitespace inside quotes
                            if ((( *var == '`' ) || ( *var == '"' )) && ( strchr( fname, *var ) == NULL ))  {
                                if (cQuote == *var )
                                    cQuote = 0;
                                else if ( cQuote == 0 )
                                    cQuote = *var;
                            } else if (( cQuote == 0 ) && ( strchr( fname, *var ) != NULL ))
                                break;

                            var += (( n == '-' ) ? -1 : 1 );
                        }

                        if ( i == 0 ) {

                            // this is the argument I want - copy it & return
                            if (( n = (int)( var - ptr )) < 0 ) {
                                n = -n;
                                var++;
                            } else
                                var = ptr;

                            if ( n > MAXARGSIZ )
                                n = MAXARGSIZ;
                            var[n] = '\0';
                            break;
                        }

                        if (( *var == '\0' ) || ( var <= szBuffer ))
                                break;

                        i += (( i < 0 ) ? 1 : -1 );
                    }

                    if ( offset == FUNC_WORD ) {
                      if ( i != 0 )
                        var = NULL;
                    } else
                      IntToAscii(( 0x4001 - i ), var );
                    break;

                case FUNC_EAREAD:
                    // read an (ASCII) EA

                    // fname points to EA name (i.e., ".COMMENT")
                    fname = scan( var, ",", QUOTES );
                    if (( *fname != ',' ) || ( fname[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;
                    *fname++ = '\0';
                    var = first_arg( var );
                    StripQuotes( var );

                    n = 1023;
                    EAReadASCII( var, fname, szBuffer, &n );
                    var = szBuffer;

                    break;

                case FUNC_EAWRITE:
                    // write an (ASCII) EA

                    // fname points to EA name (i.e., ".COMMENT")
                    fname = scan( var, ",", QUOTES );
                    if (( *fname != ',' ) || ( fname[1] == '\0' ))
                        return ERROR_INVALID_PARAMETER;

                    *fname++ = '\0';

                    // fptr points to EA value
                    fptr = scan( fname, ",", QUOTES );
                    if ( *fptr != ',' )
                        return ERROR_INVALID_PARAMETER;

                    *fptr++ = '\0';
                    var = first_arg( var );
                    StripQuotes( var );

                    IntToAscii(( EAWriteASCII( var, fname, fptr ) == 0 ), var );
                    break;

                case FUNC_EXETYPE:

                    // return the program type
                    if ( mkfname( fname, 0 ) == NULL )
                        return -ERROR_EXIT;

                    if (( i = app_type( fname )) < 0 )
                        var = "UNKNOWN";
                    else if ( i & 0x1600 )
                        var = "WIN";
                    else if ( i & 0x20 )
                        var = "DOS";
                    else if ( i <= 1 )
                        var = "FS";
                    else if ( i == 2 )
                        var = "AVIO";
                    else
                        var = "PM";

                    break;

                case FUNC_FSTYPE:
                    {
                        char FSQBuffer[64];

                        // file system type ("FAT", "HPFS", etc.)
                        length = sizeof(FSQBuffer );
                        if (( i = DosQueryFSAttach( szDrive, 1, 1, (PVOID)FSQBuffer, (PULONG)&length )) == 0 )
                                strcpy( var, (char *)(( FSQBuffer + 9 ) + *((PUSHORT)( FSQBuffer + 2 ))));
                        else
                                return i;
                        break;
                    }

                default:

                    // Bogus match, dude!  Probably 4OS2 looking for a
                    //   DOS-specific function
                    continue;
                }

                // we got a match, so exit now
                if ( var != szBuffer )
                        sprintf( szBuffer, FMT_PREC_STR, MAXLINESIZ-1, (( var == NULL ) ? NULLSTR : var ));

                return 0;
            }
        }

        // unknown variable function?
        return ERROR_INVALID_FUNCTION;
}


// convert an attribute text string to its binary equivalent
static int AttributeString(char *ptr, int *attrib)
{
        if ( ptr == NULL )
                return 0;

        strupr( ptr );

        for ( ; ( *ptr != '\0' ); ptr++ ) {

            if ( *ptr == 'N' )
                *attrib = 0;
            else if ( *ptr == 'R' )
                *attrib |= _A_RDONLY;
            else if ( *ptr == 'H' )
                *attrib |= _A_HIDDEN;
            else if ( *ptr == 'S' )
                *attrib |= _A_SYSTEM;
            else if ( *ptr == 'D' )
                *attrib |= _A_SUBDIR;
            else if ( *ptr == 'A' )
                *attrib |= _A_ARCH;
            else if ( *ptr != '_' )
                return ERROR_INVALID_PARAMETER;
        }

        return 0;
}


// format a long value for FUNC_EMS, FUNC_XMS, etc.
static void format_long( char *var, char *format, unsigned long lVal )
{
        // lVal comes in as a value in B - if user wants K or M, shift it
        if ( format != NULL ) {
                if ( *format == 'k' )
                        lVal /= 1000L;
                else if ( *format == 'K' )
                        lVal >>= 10;
                else if ( *format == 'm' )
                        lVal /= 1000000L;
                else if ( *format == 'M' )
                        lVal >>= 20;
        }

        sprintf( var, ((( format != NULL ) && ( tolower( format[1] ) == 'c' )) ? "%Lu" : FMT_ULONG), lVal );
}


// process the internal variables (%_...)
static char * var_internal( char *var )
{
        extern char *VAR_ARRAY[];

        int i = 0, n, offset;
        DATETIME sysDateTime;
        BYTE DeviceInfo;
        VIOCONFIGINFO vioConfig;

        if (( _stricmp( var, "?" ) == 0 ) || ( _stricmp( var, "ERRORLEVEL" ) == 0 )) {
            // exit code of last external program?
            IntToAscii( gnErrorLevel, var );
            return var;
        }

        for ( offset = 0; ; offset++ ) {

            // if it doesn't begin with an _, it's not an internal var
            if (( *var != '_' ) || ( VAR_ARRAY[offset] == NULL ))
                return NULL;

            if ( _stricmp( var + 1, VAR_ARRAY[offset]) == 0 ) {

                switch ( offset ) {

                case VAR_4VER:

                    // Version of 4DOS / TCMD / 4OS2 / 4NT / TCMD32
                    var = gszMyVersion;
                    break;

                case VAR_CMDPROC:
                    var = SHORT_NAME;
                    break;

                case VAR_ANSI:

                    // ANSI loaded?
                    IntToAscii( QueryIsANSI(), var );
                    break;

                case VAR_APMAC:         // get AC status
                case VAR_APMBATT:       // get battery status
                case VAR_APMLIFE:       // get remaining battery life
                    {
                        HFILE    DevHandle;
                        ULONG    ulCategory;
                        ULONG    ulFunction;
                        ULONG    ulParmLen;
                        ULONG    ulDataLen;
                        ULONG    ulAction;
                        UCHAR    uchDataArea;
                        PARMPACK ParmPack;

                        if ( DosOpen( "APM$", &DevHandle, &ulAction, 0, FILE_NORMAL, FILE_OPEN, OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, 0) != 0 ) {
                                // APM not installed!
                                *var = '\0';
                                break;
                        }

                        ulCategory  = IOCTL_POWER;
                        ulFunction  = POWER_GETPOWERSTATUS;
                        ulParmLen   = sizeof(ParmPack);
                        ulDataLen   = sizeof(uchDataArea);
                        ParmPack.ParmLength = 7;

                        // get power status
                        DosDevIOCtl( DevHandle, ulCategory, ulFunction, &ParmPack, sizeof(ParmPack), &ulParmLen, &uchDataArea, sizeof(uchDataArea), &ulDataLen );

                        if ( offset == VAR_APMAC ) {
                                if ( ParmPack.ACStatus > 1 )
                                        ParmPack.ACStatus = 2;
                                var = ACList[ ParmPack.ACStatus ];
                        } else if ( offset == VAR_APMBATT ) {
                                if ( ParmPack.BatteryStatus > 4 )
                                        ParmPack.BatteryStatus = 4;
                                var = BatteryList[ ParmPack.BatteryStatus ];
                        } else {
                                // if chLife == 255, return "unknown"
                                if ( ParmPack.BatteryLife > 100 )
                                        strcpy( var, BatteryList[4] );
                                else
                                        IntToAscii( ParmPack.BatteryLife, var );
                        }

                        DosClose( DevHandle );
                        break;
                    }

                case VAR_BATCH:

                    // current batch nesting level
                    IntToAscii( cv.bn+1, var );
                    break;

                case VAR_BATCHLINE:

                    // current batch line
                    IntToAscii((( cv.bn >= 0 ) ? bframe[cv.bn].uBatchLine : -1 ), var );
                    break;

                case VAR_BATCHNAME:

                    // current batch name
                    strcpy( var, (( cv.bn >= 0 ) ? filecase( bframe[cv.bn].pszBatchName ) : NULLSTR ));
                    break;

                case VAR_BG_COLOR:
                case VAR_FG_COLOR:

                    // foreground or background color at current cursor position
                    GetAtt( (unsigned int *)&i, (unsigned int *)&n );

                    if (( gpIniptr->BrightBG ) || ( gpLIS->typeProcess == PT_WINDOWABLEVIO))
                        strcpy( var, (( offset == VAR_FG_COLOR) ? colors[( i&0xF)].shade : colors[( i>>4)].shade ));

                    else {

                        if ( offset == VAR_FG_COLOR)
                            sprintf( var, "%s%s%s", (( i & 0x08) ? BRIGHT : NULLSTR), (( i & 0x80 ) ? BLINK : NULLSTR),colors[i&0x7].shade );
                        else
                            strcpy( var, colors[( i >> 4) & 0x7 ].shade );
                    }
                    break;

                case VAR_BOOT:

                    // boot drive
                    sprintf( var, FMT_CHAR, gpIniptr->BootDrive );
                    break;

                case VAR_CI:
                    IntToAscii( gpIniptr->CursI, var );
                    break;

                case VAR_CO:
                    IntToAscii( gpIniptr->CursO, var );
                    break;

                case VAR_CODEPAGE:

                    // current code page
                    IntToAscii( QueryCodePage(), var );
                    break;

                case VAR_COLUMN:
                case VAR_ROW:

                    // current cursor row or column position
                    GetCurPos( &i, &n );
                    IntToAscii( (( offset == VAR_ROW) ? i : n), var );
                    break;

                case VAR_COLUMNS:

                    // number of screen columns
                    IntToAscii( GetScrCols(), var );
                    break;

                case VAR_COUNTRY:
                    // current country code
                    IntToAscii( gaCountryCode.country, var );
                    break;

                case VAR_CPU:

                    // get the CPU type
                    IntToAscii( get_cpu(), var );
                    break;

                case VAR_CWD:

                    // current working directory
                    var = gcdir( NULL, 0 );
                    break;

                case VAR_CWDS:

                    // cwd w/backslash guaranteed
                    if (( var = gcdir( NULL, 0 )) != NULL )
                        mkdirname( var, NULLSTR );
                    break;

                case VAR_CWP:

                    // current working directory with no drive
                    if (( var = gcdir( NULL, 0 )) != NULL )
                        var += 2;
                    break;

                case VAR_CWPS:

                    // cwd with no drive w/backslash guaranteed
                    if (( var = gcdir( NULL, 0 )) != NULL ) {
                        var += 2;
                        mkdirname( var, NULLSTR);
                    }

                    break;

                case VAR_DATE:

                    // system date
                    QueryDateTime( &sysDateTime );

                    // replace leading space with a 0
                    if (*( var = FormatDate(sysDateTime.month,sysDateTime.day,sysDateTime.year )) == ' ' )
                        *var = '0';
                    break;

                case VAR_DAY:

                    // system date
                    QueryDateTime( &sysDateTime );
                    IntToAscii( sysDateTime.day, var );
                    break;

                case VAR_DNAME:

                    // set DescriptionName
                    if ( gpIniptr->DescriptName == INI_EMPTYSTR)
                        var = DESCRIPTION_FILE;
                    else
                        var = (char *)( gpIniptr->StrData + gpIniptr->DescriptName );
                    break;

                case VAR_DISK:

                    // current disk
                    *var = (char)( gcdisk( NULL ) + 64);
                    var[1] = '\0';
                    break;

                case VAR_DOS:

                    // operating system flavor (DOS, WIN, OS2, or NT)
                    var = "OS2";
                    break;

                case VAR_DOSVER:

                    // get the DOS or OS/2 version
                    var = gszOsVersion;
                    break;

                case VAR_DOW:
                case VAR_DOWI:
                    // get the day of week (Mon, Tue, etc. or 1 - 7)
                    QueryDateTime( &sysDateTime );

                    if ( offset == VAR_DOW )
                        strcpy( var, daytbl[(int)sysDateTime.weekday] );
                    else
                        IntToAscii( sysDateTime.weekday+1, var );
                    break;

                case VAR_DOY:

                    // return the day of year (1-366)
                    QueryDateTime( &sysDateTime );
                    n = sysDateTime.day;

                    // get days for previous months
                    for ( i = 1; ( i < ( int)sysDateTime.month); i++) {
                        if ( i == 2 )
                            n += ((( sysDateTime.year % 4 ) == 0 ) ? 29 : 28 );
                        else
                            n += ((( i == 4) || ( i == 6 ) || ( i == 9 ) || ( i == 11 )) ? 30 : 31 );
                    }

                    IntToAscii( n, var );
                    break;

                case VAR_HLOGFILE:

                    // name of command log file
                    strcpy( var, (( gpIniptr->HistLogOn) ? GetLogName(1 ) : NULLSTR));
                    break;

                case VAR_HOUR:

                    // system date
                    QueryDateTime( &sysDateTime );
                    IntToAscii( sysDateTime.hours, var );
                    break;

                case VAR_IERRORLEVEL:

                    // exit code of last internal command
                    IntToAscii( gnInternalErrorLevel, var );
                    break;

                case VAR_KBHIT:
                    // is a key waiting?
                    IntToAscii(( _kbhit() != 0 ), var );
                    break;

                case VAR_LASTDISK:

                    // return last active disk
                    for ( i = 26; ( i > 2 ); i-- ) {
                        if ( QueryDriveExists( i ))
                                break;
                    }

                    sprintf( var, FMT_CHAR, i+64);
                    break;

                case VAR_LOGFILE:

                    // name of command log file
                    strcpy( var, (( gpIniptr->LogOn) ? GetLogName( 0 ) : NULLSTR));
                    break;

                case VAR_MINUTE:

                    // system date
                    QueryDateTime( &sysDateTime );
                    IntToAscii( sysDateTime.minutes, var );
                    break;
                case VAR_MONITOR:

                    // get monitor type (MONO or COLOR)
                    vioConfig.cb = sizeof( vioConfig );
                    ( void)VioGetConfig( 0, &vioConfig, 0 );

                    var = ((( vioConfig.display == 0 ) || ( vioConfig.display == 3 ) || ( vioConfig.display == 10 ) || ( vioConfig.display == 11 )) ? MONO_MONITOR : COLOR_MONITOR);
                    break;

                case VAR_MONTH:

                    // system date
                    QueryDateTime( &sysDateTime );
                    IntToAscii( sysDateTime.month, var );
                    break;

                case VAR_MOUSE:

                    // assume mouse present for everything else
                    var = "1";
                    break;

                case VAR_NDP:

                    // get type (0 (none), 8087, 80287 or 80387)
                    // check to see if coprocessor is installed, then get type
                    ( void)DosDevConfig( &DeviceInfo, DEVINFO_COPROCESSOR );
                    IntToAscii((( DeviceInfo ) ? get_ndp() : 0 ), var );
                    break;

                case VAR_PID:

                    // current process ID for 4OS2 or 4NT or Take Command
                    IntToAscii( gpLIS->pidCurrent, var );
                    break;

                case VAR_PIPE:

                    // current process inside a pipe?
                    IntToAscii( QueryIsPipeHandle( STDIN ), var );
                    break;

                case VAR_ROWS:

                    // number of screen rows
                    IntToAscii( GetScrRows() + 1, var );
                    break;

                case VAR_SECOND:

                    // system date
                    QueryDateTime( &sysDateTime );
                    IntToAscii( sysDateTime.seconds, var );
                    break;

                case VAR_SHELL:

                    // shell level
                    IntToAscii( gpIniptr->ShellNum, var );
                    break;

                case VAR_SYSERR:

                    // last DOS/OS2/NT error
                    IntToAscii( gnSysError, var );
                    break;

                case VAR_TIME:

                    // system time (24-hour); replace leading space with a 0
                    if ( *( var = gtime( 1 )) == ' ' )
                        *var = '0';
                    break;

                case VAR_TRANSIENT:

                    // if loaded with /C, gnTransient == 1
                    IntToAscii( gnTransient, var );
                    break;

                case VAR_VIDEO:

                    // get video adaptor type
                    vioConfig.cb = sizeof( vioConfig );
                    VioGetConfig( 0, &vioConfig, 0 );

                    // if undefined value, default to vga!
                    if ( vioConfig.adapter > 9)
                        vioConfig.adapter = 3;
                    var = video_type[ vioConfig.adapter ];
                    break;

                case VAR_WINTITLE:

                    // get the title of our window
                    *var = '\0';
                    {
                        SWCNTRL swctl;
                        HSWITCH hswitch;

                        if (( pfnWQSH != 0L ) && (( hswitch = (*pfnWQSH)( 0, gpLIS->pidCurrent )) != NULL )) {
                                swctl.szSwtitle[0] = '\0';
                                (void)(*pfnWQSE)( hswitch, &swctl );
                                var = swctl.szSwtitle;
                        }
                    }
                    break;

                case VAR_YEAR:

                    // system date
                    QueryDateTime( &sysDateTime );
                    IntToAscii( sysDateTime.year, var );
                    break;

                case VAR_XPIXELS:
                    if ( pfnWQSV != NULL )
                        IntToAscii( (*pfnWQSV)( HWND_DESKTOP, SV_CXSCREEN ), var );
                    else
                        return NULL;
                    break;

                case VAR_YPIXELS:
                    if ( pfnWQSV != NULL )
                        IntToAscii( (*pfnWQSV)( HWND_DESKTOP, SV_CYSCREEN ), var );
                    else
                        return NULL;
                    break;

                case VAR_OS2_PPID:

                    // parent process ID
                    IntToAscii( gpLIS->pidParent, var );
                    break;

                case VAR_OS2_SID:

                    // current session ID
                    IntToAscii( gpLIS->sgCurrent, var );
                    break;

                case VAR_OS2_PTYPE:

                    // current process type (0=FS, 2=Window, 3=PM, 4=detached)
                    var = gaPType[ gpLIS->typeProcess ];
                    break;

                default:
                    // Bogus match, dude!  Probably 4DOS looking for a
                    //   OS2-specific function
                    continue;
                }

                // found a match
                return var;
            }
        }
}


// collapse escape characters for the entire line
void EscapeLine(char *line)
{
        if (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 ) {

            for ( ; ( *line != '\0' ); line++ ) {

                if ( *line == gpIniptr->EscChr ) {
                    strcpy( line, line + 1 );
                    *line = (char)escape_char( *line );
                }
            }
        }
}


// collapse escape characters
void escape( char *line )
{
        if (( *line == gpIniptr->EscChr ) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 )) {
                strcpy( line, line + 1 );
                *line = (char)escape_char( *line );
        }
}


// convert the specified character to it's escaped equivalent
int escape_char( int c )
{
        if (( c = tolower( c )) == 'b' )
                c = '\b';               // backspace
        else if ( c == 'c' )
                c = ',';                // comma
        else if ( c == 'e' )
                c = '\033';             // ESC
        else if ( c == 'f' )
                c = '\f';               // form feed
        else if ( c == 'k' )
                c = '`';
        else if ( c == 'n' )
                c = '\n';               // line feed
        else if ( c == 'q' )
                c = '"';
        else if ( c == 'r' )
                c = '\r';               // CR
        else if ( c == 's' )
                c = ' ';                // space
        else if ( c == 't' )
                c = '\t';               // tab

        return c;
}


#define HISTORY_APPEND 1
#define HISTORY_FREE 2
#define HISTORY_PAUSE 4
#define HISTORY_READ 8


// print the command history, read it from a file, or clear it
int history_cmd( int argc, char **argv )
{
        char *arg;
        int rval = 0;
        long fHistory;
        PCH hptr, lpList;

        lpList = (( _ctoupper( gpInternalName[0] ) == 'D' ) ? glpDirHistory : glpHistoryList );

        // check for switch - if we have args, we have to have a switch!
        if (( GetSwitches( argv[1], "AFPR", &fHistory, 1 ) != 0 ) || (( fHistory == 0 ) && ( argc > 1 )))
                return ( usage( HISTORY_USAGE ));

        // clear the history list
        if ( fHistory & HISTORY_FREE ) {

                *lpList = '\0';
                lpList[1] = '\0';

        } else if ( fHistory & HISTORY_APPEND ) {

                // add string to history list
                glpHptr = NULL;
                if ( argv[1] != NULL ) {
                    if ( lpList == glpHistoryList )
                        addhist( argv[1] );
                    else
                        SaveDirectory( lpList, argv[1] );
                }

        } else  if ( fHistory & HISTORY_READ ) {

                // read a history file
                if (( arg = first_arg( argv[1] )) == NULL )
                        return ( usage( HISTORY_USAGE ));

                if (( mkfname( arg, 0 ) == NULL ) || (( gnGFH = _sopen( arg, (O_RDONLY | O_BINARY),SH_DENYWR)) < 0 ))
                        return ( error( _doserrno, arg ));

                // add the line to the history
                for ( arg = gszCmdline, glpHptr = 0L; ( getline( gnGFH, arg, MAXLINESIZ - ((int)( arg - gszCmdline ) + 1 ), EDIT_DATA ) > 0 ); ) {

                        // if last char is escape character, append
                        //   the next line
                        if ( *arg ) {
                            arg += ( strlen( arg ) - 1 );
                            if ( *arg == gpIniptr->EscChr )
                                continue;
                        }

                        // skip blank lines, leading whitespace, & comments
                        arg = skipspace( gszCmdline );
                        if (( *arg ) && ( *arg != ':' )) {
                            if ( lpList == glpHistoryList )
                                addhist( arg );
                            else
                                SaveDirectory( lpList, arg );
                        }

                        arg = gszCmdline;
                }

                gnGFH = _close( gnGFH );

        } else {

                // display the history, optionally pausing after each page
                init_page_size();

                if ( fHistory & HISTORY_PAUSE ) {
                        gnPageLength = GetScrRows();
                }

                for ( hptr = lpList; ( *hptr != '\0' ); hptr = next_env( hptr ))
                        more_page( hptr, 0 );
        }

        return rval;
}


// add a command to the history list
void addhist( char *histstr )
{
        unsigned int length;
        PCH history_end;
        // OS/2 needs a semaphore to keep processes from simultaneously writing
        //   the alias list
        HMTX SemHandle = 0;

        histstr = skipspace( histstr );

        length = strlen( histstr );

        // check for history not modified, or command too short or too long
        if (( glpHptr != 0L ) || ( *histstr == '@' ) || ( strnicmp( histstr, "*@", 2 ) == 0 ) || ( *histstr == '\0' ) || ( length < gpIniptr->HistMin ) || (( length + 2 ) > gpIniptr->HistorySize ))
                return;
        // disable signals temporarily
        HoldSignals();

        // block other processes & threads while updating history list
        RequestSemaphore( &SemHandle, SEMAPHORE_NAME );

        // check if history size has been changed in .INI dialog
        if ( gpIniptr->LocalHistory ) {
                if ( gpIniptr->HistoryNew > gpIniptr->HistorySize )
                        glpHistoryList = ReallocMem( glpHistoryList, gpIniptr->HistoryNew );
                gpIniptr->HistorySize = gpIniptr->HistoryNew;
        }

        // history entries are separated by a '\0'
        for ( ; ; ) {

                history_end = end_of_env( glpHistoryList );
                if (( length + (unsigned int)( history_end - glpHistoryList ) + 2 ) < gpIniptr->HistorySize )
                        break;

                // delete the oldest ( first) history entry
                memmove( glpHistoryList, next_env( glpHistoryList ), gpIniptr->HistorySize - (unsigned int)( next_env( glpHistoryList ) - glpHistoryList) );
        }

        strcpy( history_end, histstr );
        history_end[ length + 1 ] = '\0';

        // reset history pointer
        glpHptr = 0L;

        // free the semaphore & reenable signals
        FreeSemaphore( SemHandle );
        EnableSignals();
}


// return previous command in history list
char *prev_hist( PCH s )
{
        if ( s <= glpHistoryList ) {
                // don't wrap back to end if the user doesn't want to!
                if (( s != 0L ) && ( gpIniptr->HistoryWrap == 0 ))
                        return glpHistoryList;
                s = end_of_env( glpHistoryList );
        }

        if ( s > glpHistoryList )
                s--;

        for ( ; (( s > glpHistoryList ) && ( s[-1] != '\0' )); s-- )
                ;

        return s;
}


// return next command in history list
char *next_hist(PCH s)
{
        PCH pchEnv;

        if ( s == 0L ) {
                if ( gpIniptr->HistoryWrap )
                        s = glpHistoryList;     // wrapped around
        } else {
                pchEnv = next_env( s );
                if ( *pchEnv != '\0' )
                        s = pchEnv;
                else if ( gpIniptr->HistoryWrap )
                        s = glpHistoryList;
        }

        return s;
}

