// ENV.C - Environment routines (including aliases) for 4os2
//   (c) 1988 - 1998  Rex C. Conn   All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <string.h>

#include "4all.h"


#define SET_EXPRESSION 1
#define SET_MASTER 2
#define SET_PAUSE 4
#define SET_READ 8


static int SetFromFile( char *szLine, char *pchList, int fRemove )
{
    char *arg;
    char szBuffer[CMDBUFSIZ];
    int i, n, rval = 0;

    for ( i = 0; (( rval == 0 ) && (( arg = ntharg( szLine, i )) != NULL )); i++ ) {

        mkfname( arg, 0 );

        if (( gnGFH = _sopen( arg, (O_RDONLY | O_BINARY), SH_DENYWR )) < 0 )
            return ( error( _doserrno, arg ));

        for ( arg = szBuffer; (( rval == 0 ) && ( getline( gnGFH, arg, CMDBUFSIZ-((arg-szBuffer)+1), EDIT_DATA ) > 0 )); ) {

            // remove leading white space
            strip_leading( arg, WHITESPACE );

            // if last char is escape character, append
            //   the next line
            n = strlen( arg ) - 1;
            if (( arg[0] ) && ( arg[n] == gpIniptr->EscChr )) {
                arg += n;
                continue;
            }

            // skip blank lines & comments
            arg = szBuffer;
            if (( *arg ) && ( *arg != ':' )) {

                if ( fRemove ) {
                    // delete variable
                    for ( n = 0; (( szBuffer[n] ) && ( szBuffer[n++] != '=' )); )
                        ;
                    szBuffer[n] = '\0';
                }

                // create/modify a variable or alias
                rval = add_list( arg, pchList );
            }
        }

        gnGFH = _close( gnGFH );
    }

    return rval;
}


// create or display environment variables or aliases
int set_cmd( int argc, char **argv )
{
    char *arg;
    long fSet;
    PCH feptr, pchList;
    char szBuffer[CMDBUFSIZ];

    init_page_size();

    // set the pointer to either the environment or the alias list
    if ( _stricmp( gpInternalName, SET_COMMAND ) == 0 )  {

        pchList = 0L;

    } else
        pchList = glpAliasList;

    // strip leading switches
    if (( GetSwitches( argv[1], "AMPR", &fSet, 1 ) != 0) || (( fSet & SET_READ ) && ( first_arg( argv[1] ) == NULL )))
        return ( usage(( pchList == glpAliasList ) ? ALIAS_USAGE : SET_USAGE ));

    // check for master environment set
    if (( pchList == 0L ) && ( fSet & SET_MASTER ))
        pchList = glpMasterEnvironment;

    // read environment or alias file(s)
    if ( fSet & SET_READ )
        return ( SetFromFile( argv[1], pchList, 0 ));

    // pause after each page
    if ( fSet & SET_PAUSE ) {
        gnPageLength = GetScrRows();
    }

    if ( first_arg( argv[1] ) == NULL ) {

        if ( pchList == 0L )
            pchList = glpEnvironment;

        // print all the variables or aliases
        for ( feptr = pchList; ( *feptr != '\0' ); feptr = next_env( feptr ) ) {
            more_page( feptr, 0 );
        }

        if ( pchList == glpEnvironment ) {

            if (( arg = get_list( BEGINLIBPATH, pchList )) != 0L ) {
                sprintf( szBuffer, FMT_TWO_EQUAL_STR, BEGINLIBPATH, arg );
                more_page( szBuffer, 0 );
            }

            if (( arg = get_list( ENDLIBPATH, pchList )) != 0L ) {
                sprintf( szBuffer, FMT_TWO_EQUAL_STR, ENDLIBPATH, arg );
                more_page( szBuffer, 0 );
            }
        }

        // return an error message if no aliases exist
        // just return if no environment variables exist
        return (( feptr == glpAliasList ) ? error( ERROR_4DOS_NO_ALIASES, NULL ) : 0 );
    }

    if ( fSet & SET_EXPRESSION ) {

        if (( arg = strchr( argv[1], '=' )) != NULL )
            arg = skipspace( ++arg );
        else
            arg = argv[1];

        evaluate( arg );
        if ( cv.bn < 0 )
            qputs( arg );

        // create/modify/delete a variable
        return (( arg == argv[1] ) ? 0 : add_list( argv[1], pchList ));
    }

    // display the current variable or alias argument?
    // (setting environment vars requires a '='; it's optional with aliases)
    if ((( arg = strchr( argv[1], '=' )) == NULL ) && (( pchList == 0L ) || ( ntharg( argv[1], 1 ) == NULL ))) {

        if (( feptr = get_list( argv[1], pchList )) == 0L ) {
            return ( error((( pchList == glpAliasList ) ? ERROR_4DOS_NOT_ALIAS : ERROR_4DOS_NOT_IN_ENVIRONMENT),argv[1]));
        }

        printf( FMT_FAR_STR_CRLF, feptr );
        return 0;
    }

    // create/modify/delete a variable or alias
    return ( add_list( argv[1], pchList ));
}


#define UNSET_MASTER 1
#define UNSET_QUIET  2
#define UNSET_READ   4

// remove environment variable(s) or aliases
int unset_cmd( int argc, char **argv )
{
    char *arg;
    int i, rval = 0;
    long fUnset;
    PCH pchList;

    // set the pointer to either the environment or the alias list
    if ( _stricmp( gpInternalName, UNSET_COMMAND ) == 0 )  {

        pchList = 0L;

    } else
        pchList = glpAliasList;

    // strip leading switches
    if (( GetSwitches( argv[1], "MQR", &fUnset, 1 ) != 0 ) || ( first_arg( argv[1] ) == NULL ))
        return ( usage(( pchList == glpAliasList ) ? UNALIAS_USAGE : UNSET_USAGE ));

    // check for master environment set
    if (( pchList == 0L ) && ( fUnset & UNSET_MASTER ))
        pchList = glpMasterEnvironment;

    // read environment or alias file(s)
    if ( fUnset & UNSET_READ )
        return ( SetFromFile( argv[1], pchList, 1 ));

    for ( i = 0; (( arg = ntharg( argv[1], i )) != NULL ); i++ ) {

        if ( _stricmp( arg, "*" ) == 0 ) {

            // wildcard kill - null the environment or alias list
            if ( pchList == 0L )
                pchList = glpEnvironment;
            pchList[0] = '\0';
            pchList[1] = '\0';
            break;
        }

        // kill the variable or alias
        if ( get_list( arg, pchList ) == 0L ) {

            // check for "quiet" switch
            if ( fUnset & UNSET_QUIET )
                rval = ERROR_EXIT;
            else
                rval = error((( pchList == glpAliasList ) ? ERROR_4DOS_NOT_ALIAS : ERROR_4DOS_NOT_IN_ENVIRONMENT ), arg );

        } else if ( add_list( arg, pchList ) != 0)
            rval = ERROR_EXIT;
    }

    return rval;
}


#define ESET_ALIAS 1
#define ESET_MASTER 2

// edit an existing environment variable or alias
int eset_cmd( int argc, char **argv )
{
    char *arg;
    int nLength;
    int i, rval = 0;
    long fEset;
    PCH vname, feptr, pchList;
    unsigned char buffer[CMDBUFSIZ];

    // check for alias or master environment switches
    if (( GetSwitches( argv[1], "AM", &fEset, 1 ) != 0 ) || ( first_arg( argv[1] ) == NULL ))
        return ( usage( ESET_USAGE ));

    for ( i = 0; (( arg = ntharg( argv[1], i )) != NULL ); i++ ) {

        pchList = (( fEset & ESET_MASTER ) ? glpMasterEnvironment : glpEnvironment );

        // try environment variables first, then aliases
        if (( fEset & ESET_ALIAS ) || (( feptr = get_list( arg, pchList )) == 0L)) {

            // check for alias editing
            if (( feptr = get_list( arg, glpAliasList )) == 0L ) {
                rval = error((( fEset & ESET_ALIAS ) ? ERROR_4DOS_NOT_ALIAS : ERROR_4DOS_NOT_IN_ENVIRONMENT ), arg );
                continue;
            }

            pchList = glpAliasList;
        }

        // get the start of the alias or variable name
        for ( vname = feptr; (( vname > pchList ) && ( vname[-1] != '\0' )); vname-- )
            ;

        // length of alias/variable name
        nLength = (int)( feptr - vname );

        sprintf( buffer, "%.*Fs%.*Fs", nLength, vname, (( CMDBUFSIZ - 1 ) - nLength), feptr );

        // echo & edit the argument
        printf( FMT_FAR_PREC_STR, nLength, vname );
        (void)egets( buffer + nLength, (( CMDBUFSIZ - 1 ) - nLength ), EDIT_ECHO );

        if ( add_list( buffer, pchList ) != 0 )
            rval = ERROR_EXIT;
    }

    return rval;
}


// get environment variable
char * get_variable( char *envstr )
{
    return ( get_list( envstr, glpEnvironment ));
}


// get alias (a near pointer in the current data segment)
char * get_alias( char *alias )
{
    return ( get_list( alias, glpAliasList ));
}


// retrieve an environment or alias list entry
char * get_list( char *varname, PCH pchList )
{
    char *arg;
    int wildflag;
    PCH pchEnv;
    PCH pchStart;
    static char szLibPath[512];

    if ( pchList == 0L )
        pchList = glpEnvironment;
    // special case for BeginLIBPATH and EndLIBPATH
    if ( pchList == glpEnvironment ) {

        szLibPath[0] = '\0';
        if ( stricmp( varname, BEGINLIBPATH ) == 0 ) {
            if ((DosQueryExtLIBPATH( szLibPath, BEGIN_LIBPATH ) == NO_ERROR ) && ( szLibPath[0] != '\0' ))
                return szLibPath;
            return 0L;
        }

        if ( stricmp( varname, ENDLIBPATH ) == 0 ) {
            if ((DosQueryExtLIBPATH( szLibPath, END_LIBPATH ) == NO_ERROR ) && ( szLibPath[0] != '\0' ))
            return szLibPath;
            return 0L;
        }
    }

    for ( pchEnv = pchList; *pchEnv; ) {

        // aliases allow "wher*eis" ; so collapse the '*', and
        //   only match to the length of the varname
        arg = varname;
        wildflag = 0;
        pchStart = pchEnv;

        do {

            if (( pchList == glpAliasList ) && ( *pchEnv == '*' )) {
                pchEnv++;
                wildflag++;

                // allow entry of "ab*cd=def"
                if ( *arg == '*' )
                    arg++;
            }

            if ((( *arg == '\0' ) || ( *arg == '=' )) && ((( *pchEnv == '=' ) && ( pchEnv != pchStart )) || ( wildflag ))) {

                for ( ; ( *pchEnv ); pchEnv++ ) {
                    if ( *pchEnv == '=' )
                        return ++pchEnv;
                }
                return NULL;
            }

        } while ( _ctoupper( *pchEnv++ ) == _ctoupper( *arg++ ));

        while ( *pchEnv++ != '\0' )
            ;
    }

    return 0L;
}


// add or delete environment var
int add_variable( char *envstr )
{
    return ( add_list( envstr, glpEnvironment ));
}


// add a variable to the environment or alias to the alias list
int add_list( char *envstr, PCH pchList )
{
    char *line;
    PCH feptr, env_arg, env_end, last_var;
    unsigned int length;
    int rval = 0;
    ULONG size;     // size of environment or alias blocks

    // OS/2 & NT need semaphores to keep processes from simultaneously
    //  writing the alias list
    HMTX SemHandle = 0;
    char szVarName[32];

    if ( pchList == 0L )
        pchList = glpEnvironment;

    line = envstr;
    if ( *line == '=' ) {
        return ( error( ERROR_4DOS_BAD_SYNTAX, envstr ));
    }

    for ( ; (( *line ) && ( *line != '=' )); line++ ) {

        if ( pchList == glpAliasList ) {

            if ( iswhite( *line )) {
                strcpy( line, skipspace( line ) );
                break;
            }
        }
        else        // ensure environment entry is in upper case
            *line = (unsigned char)_ctoupper( *line );
    }

    if ( *line == '=' ) {

        // point to the first char of the argument
        line++;

        // collapse whitespace around '=' in aliases, but not in env
        //   variables, for COMMAND.COM compatibility (set abc def= ghi)
        if ( pchList == glpAliasList )
            strcpy( line, skipspace( line ));

    } else if ( *line ) {
        // add the missing '='
        strins( line, "=" );
        line++;
    }

    // removing single back quotes at the beginning and end of an alias
    //   argument (they're illegal there; the user is probably making a
    //   mistake with ALIAS /R)
    if (( *line == SINGLE_QUOTE ) && ( pchList == glpAliasList )) {

        // remove leading single quote
        strcpy( line, line + 1 );

        // remove trailing single quote
        if ((( length = strlen( line )) != 0 ) && ( line[--length] == SINGLE_QUOTE ))
            line[length] = '\0';
    }

    // block other processes & threads while updating alias list
    if ( pchList == glpAliasList ) {

        // disable signals temporarily
        HoldSignals();

        // get & lock a semaphore
        RequestSemaphore( &SemHandle, SEMAPHORE_NAME );
    }

    // get pointers to beginning & end of alias/environment space
    size = QueryMemSize( pchList );
        env_end = pchList + ( size - 4 );

    // get pointer to end of environment or alias variables
    last_var = end_of_env( pchList );

    length = strlen( envstr ) + 1;

    // special case for BeginLIBPATH and EndLIBPATH
    sscanf( envstr, "%31[^=]", szVarName );
    if (stricmp( szVarName, BEGINLIBPATH ) == 0) {
        if ((DosSetExtLIBPATH( line, BEGIN_LIBPATH ) == NO_ERROR))
            return 0;
        return ERROR_EXIT;
    }

    if (stricmp( szVarName, ENDLIBPATH ) == 0) {
        if ((DosSetExtLIBPATH( line, END_LIBPATH ) == NO_ERROR))
            return 0;
        return ERROR_EXIT;
    }

    // check for modification or deletion of existing entry
    if (( env_arg = get_list( envstr, pchList )) != 0L ) {

        // get the start of the alias or variable name
        for ( feptr = env_arg; (( feptr > pchList ) && ( feptr[-1] != '\0' )); feptr-- )
            ;

        if ( *line == '\0' ) {
            // delete an alias or environment variable
            memmove( feptr, next_env( feptr ), (unsigned int)( last_var - next_env(feptr)) + 1);
        } else {
            // get the relative length (vs. the old variable)
            length = strlen( line ) - strlen( env_arg );
        }
    }

    if ( *line != '\0' ) {

        // check for out of environment space
        if (( last_var + length ) >= env_end ) {

            // boost environment or alias list size
            if ( ReallocMem( pchList, size + ENVIRONMENT_SIZE ) == NULL) {
                rval = error((( pchList == glpAliasList ) ? ERROR_4DOS_OUT_OF_ALIAS : ERROR_4DOS_OUT_OF_ENVIRONMENT), NULL);
                goto add_bye;
            }

            // adjust the environment / alias list size
            size = QueryMemSize( pchList );
            if ( pchList == glpEnvironment )
                gpIniptr->EnvSize = (unsigned int)size;
            else if ( pchList == glpAliasList )
                gpIniptr->AliasSize = (unsigned int)size;
        }

        if ( env_arg != 0L ) {

            // modify an existing alias or environment variable
            //   adjust the space & insert new value
            feptr = next_env( feptr );
            memmove(( feptr + length ), feptr, (unsigned int)( last_var - feptr) + 1 );
            strcpy( env_arg, line );

        } else {
            // put it at the end & add an extra null
            strcpy( last_var, envstr );
            last_var[length] = '\0';
        }
    }

add_bye:
    if ( pchList == glpAliasList ) {
        // clear the semaphore
        FreeSemaphore( SemHandle );
        EnableSignals();
    }

    return rval;
}


// return a pointer to the next entry in environment or alias list
char * next_env( PCH pchEnv )
{
    int i;

    if (( i = strlen( pchEnv )) > 0 )
        i++;

    return ( pchEnv + i );
}


// get pointer to end of environment or alias list
char * end_of_env( PCH pchList )
{
    for ( ; ( *pchList != '\0' ); pchList = next_env( pchList ))
        ;

    return pchList;
}

