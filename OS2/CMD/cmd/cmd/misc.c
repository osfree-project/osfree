// MISC.C - Miscellaneous support routines for 4xxx / TCMD family
//   (c) 1988 - 1998 Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <direct.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <share.h>
#include <string.h>

#include "4all.h"

static char * NextRange(char *);
static int GetRangeArgs(char *, RANGES *);
static int GetStrTime(char *, int *, int *);
static int GetStrSize(char *, unsigned long *);

int wild_brackets(char *, int, int );


int fNoComma = 0;


int QueryIsLFN( char *pszFilename )
{
        int n, fExt;

        // check for LFN/HPFS/NTFS name w/spaces or other invalid chars
        if ( strpbrk( pszFilename, " ,\t|=<>" ) != NULL )
                return 1;

        // check name for multiple extensions or
        //   > 8 char filename & 3 char extension
        for ( n = 0, fExt = 0; ( pszFilename[n] != '\0' ); n++ ) {
                if ( pszFilename[n] == '.' ) {
                    if (( fExt ) || ( n > 8 ))
                        return 1;
                    fExt = strlen( pszFilename + n );
                }
        }

        return (( n > 12 ) || (( fExt == 0 ) && ( n > 8 )) || ( fExt > 4 ));
}


// Get a file's size via the handle
long QuerySeekSize( int fh )
{
        return ( _lseek( fh, 0L, SEEK_END ));
}


// Rewind a file
long RewindFile( int fh )
{
        return ( _lseek( fh, 0L, SEEK_SET ));
}


// convert keystrokes
int cvtkey(unsigned int uKeyCode, unsigned int uContextBits)
{
        unsigned int i, uSearchKey;
        unsigned int uKeyCount = gpIniptr->KeyUsed;
        unsigned int *puScan = gpIniptr->Keys;
        unsigned int *puSubstitute = gpIniptr->Keys + uKeyCount;

        uSearchKey = _ctoupper( uKeyCode );
        uContextBits <<= 8;

        // scan the key mapping array
        for ( i = 0; ( i < uKeyCount ); i++ ) {

                // if key and context bits match, return the mapped key
                if ((uSearchKey == puScan[i]) && (( puSubstitute[i] & uContextBits) != 0 )) {

                        // if forced normal key return original code with force normal bit set
                        if (( puSubstitute[i] & (MAP_NORM_KEY << 8)) != 0 )
                                return (uKeyCode | (MAP_NORM_KEY << 8));

                        // otherwise return new key
                        return ( puSubstitute[i] & ((EXTKEY << 8) | 0xFF));
                }
        }

        // no match, return original code
        return uKeyCode;
}


// replace RTL isspace() - we only want to check for spaces & tabs
int iswhite( char c )
{
        return (( c == ' ' ) || ( c == '\t' ));
}


// test for delimiter (" \t,")
int isdelim( char c )
{
        return (( c == '\0' ) || ( iswhite( c )) || ( c == ',' ));
}


// skip past leading white space and return pointer to first non-space char
char * skipspace( char *line )
{
        while (( *line == ' ' ) || ( *line == '\t' ))
                line++;

        return line;
}


// return token x to token y without quote processing (for compatibility
//   with nitwit CMD.EXE syntax)
char * GetToken( char *pszLine, char *pszDelims, int nStart, int nEnd )
{
        int i, n;
        BOOL fReverse;
        char *ptr, *pszStartLine;
        char *pszStart = NULL, *pszEnd = NULL;

        // change to 0 offset!
        fReverse = ( nStart < 0 );
        nStart += (( fReverse ) ? 1 : -1 );
        nEnd += (( fReverse ) ? 1 : -1 );

        pszStartLine = pszLine;
        if (( fReverse ) && ( *pszLine != '\0' ))
                pszLine = strend( pszLine ) - 1;

        for ( i = nStart, n = 0; ; ) {

                // find start of arg[i]
                while (( *pszLine != '\0' ) && ( pszLine >= pszStartLine ) && ( strchr( pszDelims, *pszLine ) != NULL ))
                    pszLine += (( fReverse ) ? -1 : 1 );

                // search for next delimiter character
                for ( ptr = pszLine; (( *pszLine != '\0' ) && ( pszLine >= pszStartLine )); ) {

                        if ( strchr( pszDelims, *pszLine ) != NULL )
                                break;

                        pszLine += (( fReverse ) ? -1 : 1 );
                }

                if ( i == 0 ) {

                        // this is the argument I want - copy it & return
                        if (( n = (int)( pszLine - ptr )) < 0 ) {
                                n = -n;
                                pszLine++;
                        } else
                                pszLine = ptr;

                        if ( pszStart == NULL ) {

                                // located first argument - save pointer
                                pszStart = pszLine;
                                if ( nEnd == nStart )
                                        goto GotArg;

                                // reset & get last argument
                                i = nEnd;
                                pszLine = pszStartLine;
                                continue;
                        } else {
GotArg:
                                // located last argument - save pointer
                                pszEnd = pszLine + n;
                                break;
                        }
                }

                if (( *pszLine == '\0' ) || ( pszLine <= pszStartLine ))
                        break;

                i += (( fReverse ) ? 1 : -1 );
        }

        if ( i != 0 ) {
                // failed to find at start and/or end
                if ( pszStart == NULL ) {
                        pszEnd = pszStart = pszStartLine;
                } else
                        pszEnd = strend( pszStart );
        }

        sprintf( pszStartLine, FMT_PREC_STR, ( pszEnd - pszStart ), pszStart );

        return pszStartLine;
}


// return the first argument in the line
char * first_arg( char *line )
{
        return ( ntharg( line, 0 ));
}


// remove everthing up to the "argcount" argument(s)
char * next_arg( char *line, int argcount )
{
        (void)ntharg( line, argcount );
        return ( strcpy( line, (( gpNthptr != NULL ) ? gpNthptr : NULLSTR )));
}


// return the last argument in the line (skipping the first arg)
char * last_arg( char *line, int *i )
{
        for ( *i = 1; ( ntharg( line, *i ) != NULL ); (*i)++ )
                ;

        return (( *i == 1 ) ? NULL : ntharg( line, --(*i) ));
}


// NTHARG returns the nth argument in the command line (parsing by whitespace
//   & switches) or NULL if no nth argument exists
char * ntharg( char *line, int index )
{
        static char buf[ MAXARGSIZ+1 ];
        static char delims[] = "  \t,", *qptr;
        char *ptr;

        gpNthptr = NULL;
        if ( line == NULL )
                return NULL;

        qptr = QUOTES;
        delims[3] = (char)(( fNoComma == 0 ) ? ',' : '\0' );

        if ( index & 0x800 ) {
                // trick to disable switch character as delimiter
                delims[0] = ' ';
        } else
                delims[0] = gpIniptr->SwChr;

        if ( index & 0x2000 ) {
                // trick to allow []'s as quote chars
                qptr = "`\"[";
        }

        if ( index & 0x4000 ) {
                // trick to allow ( )'s as quote characters
                qptr = "`\"([";
        }

        if ( index & 0x1000 ) {
                // trick to disable ` as quote character
                qptr++;
        }

        index &= 0x7FF;

        for ( ; ; index-- ) {

                // find start of arg[i]
                line += strspn( line, delims+1 );
                if (( *line == '\0' ) || ( index < 0 ))
                        break;

                // search for next delimiter or switch character
                if (( ptr = scan(( *line == gpIniptr->SwChr ) ? line + 1 : line, delims, qptr )) == BADQUOTES)
                        break;

                if ( index == 0 ) {

                        // this is the argument I want - copy it & return
                        gpNthptr = line;
                        if (( index = (int)( ptr - line )) > MAXARGSIZ )
                                index = MAXARGSIZ;
                        sprintf( buf, FMT_PREC_STR, index, line );
                        return buf;
                }

                line = ptr;
        }

        return NULL;
}


// Find the first character in LINE that is also in SEARCH.
//   Return a pointer to the matched character or EOS if no match
//   is found.  If SEARCH == NULL, just look for end of line ((which
//   could be a pipe, compound command character, or conditional)
char * scan( char *line, char *pszSearch, char *quotestr )
{
        unsigned char cQuote;
        char *pSaveLine, cOpenParen, cCloseParen, fTerm;

        // kludge to allow detection of "%+" when looking for command separator
        fTerm = (char)(( pszSearch == NULL ) || ( strchr( pszSearch, gpIniptr->CmdSep ) != NULL ));

        if ( line != NULL ) {

            while ( *line != '\0' ) {

TestForQuotes:
                // test for a quote character or a () or [] expression
                if ((( *line == '(' ) || ( *line == '[' ) || (( gpIniptr->Expansion & EXPAND_NO_QUOTES ) == 0 )) && ( strchr( quotestr, *line ) != NULL ) && (( pszSearch == NULL ) || ( strchr( pszSearch, *line ) == NULL ))) {

                    if (( *line == '(' ) || ( *line == '[' )) {

                        cOpenParen = *line;
                        cCloseParen = (char)(( cOpenParen == '(' ) ? ')' : ']' );

                        pSaveLine = line;

                        // ignore characters within parentheses / brackets
                        for ( cQuote = 1; (( cQuote > 0 ) && ( *line != '\0' )); ) {

                            // nested parentheses?
                            if ( *(++line) == cOpenParen )
                                cQuote++;
                            else if ( *line == cCloseParen )
                                cQuote--;
                            // skip escape characters
                            else if (( *line == gpIniptr->EscChr ) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 ))
                                line++;
                            else if (( *line == '%' ) && ( line[1] == '=' ) && (( gpIniptr->Expansion & EXPAND_NO_VARIABLES ) == 0 )) {
                                // convert %= to default escape char
                                strcpy( line, line + 1 );
                                *line++ = gpIniptr->EscChr;
                            }
                        }

                        // if no trailing ], assume [ isn't a quote char!
                        if ( *line == '\0' ) {

                            if ( cOpenParen != '[' )
                                break;
                            line = pSaveLine;

                        // if only looking for the trailing ) or ], return
                        } else if (( pszSearch != NULL ) && ( *pszSearch == cCloseParen ))
                            break;

                    } else {

                        // ignore characters within quotes
                        for ( cQuote = *line; ( *(++line) != cQuote ); ) {

                            // skip escape characters
                            if (( *line == gpIniptr->EscChr ) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 ))
                                line++;
                            else if (( *line == '%' ) && ( line[1] == '=' )) {
                                if (( cQuote == '"' ) && (( gpIniptr->Expansion & EXPAND_NO_VARIABLES ) == 0 )) {
                                    // convert %= to default escape char
                                    strcpy( line, line + 1 );
                                    *line++ = gpIniptr->EscChr;
                                } else
                                    line += 2;
                            }

                            if ( *line == '\0' ) {

                                // unclosed double quotes OK?
                                if ( cQuote == '"' )
                                    return line;

                                error( ERROR_4DOS_NO_CLOSE_QUOTE, NULL );
                                return BADQUOTES;
                            }
                        }
                    }

                } else {

                    // skip escaped characters
                    for ( ; ; ) {

                        if (( *line == gpIniptr->EscChr ) && (( gpIniptr->Expansion & EXPAND_NO_ESCAPES ) == 0 )) {
                            if ( *(++line) != '\0' ) {
                                line++;
                                goto TestForQuotes;
                            } else
                                break;
                        } else if (( *line == '%' ) && ( line[1] == '=' )) {
                            if (( gpIniptr->Expansion & EXPAND_NO_VARIABLES ) == 0 ) {
                                // convert %= to default escape char
                                strcpy( line, line + 1 );
                                *line = gpIniptr->EscChr;
                            } else {
                                line += 2;
                                break;
                            }
                        } else
                            break;
                    }

                    if ( *line == '\0' )
                        break;

                    if ( pszSearch == NULL ) {

                        if (( gpIniptr->Expansion & EXPAND_NO_COMPOUNDS ) == 0 ) {

                            // check for pipes, compounds, or conditionals
                            if (( *line == '|' ) || (( *line == '&' ) && ( _strnicmp( line-1," && ", 4 ) == 0 )))
                                break;

                            // strange kludge when using & as a command
                            //   separator - this fouls up things like
                            //   |& and >&
                            if (( *line == gpIniptr->CmdSep ) && ( line[-1] != '|' ) && ( line[-1] != '>' ))
                                break;
                        }

                    } else if ( strchr( pszSearch, *line ) != NULL ) {

                        // make sure switch character has something
                        //  following it (kludge for "copy *.* a:/")
                        if (( *line != gpIniptr->SwChr ) || ( isdelim( line[1] ) == 0 ))
                            break;
                    }

                    // check for %+ when looking for terminator
                    if (( fTerm ) && ( *line == '%' ) && ( line[1] == '+' )) {

                        if (( gpIniptr->Expansion & EXPAND_NO_VARIABLES ) == 0 ) {
// FIXME - Add kludge for %var%+%var%!
                            strcpy( line, line + 1 );
                            *line = gpIniptr->CmdSep;
                            break;
                        }

                        line++;
                    }
                }

                if ( *line )
                    line++;
            }
        }

        return line;
}


int GetMultiCharSwitch( char *pszLine, char *pszSwitch, char *pszOutput, int nMaxLength )
{
        char *arg;
        int i;

        // check for and remove switches anywhere in the line
        *pszOutput = '\0';
        for ( i = 0; (( arg = ntharg( pszLine, i )) != NULL ); i++ ) {

                if (( *arg == gpIniptr->SwChr ) && ( _ctoupper( arg[1] ) == (int)*pszSwitch )) {

                        // save the switch argument
                        sprintf( pszOutput, FMT_PREC_STR, nMaxLength, arg + 2 );

                        // remove the switch(es)
                        strcpy( gpNthptr, gpNthptr + strlen( arg ) );
                        return 1;
                }
        }

        return 0;
}


// Scan the line for matching arguments, set the flags, and remove the switches
int GetSwitches( char *line, char *switch_list, long *fFlags, int fOnlyFirst )
{
        char *arg;
        int i;
        long lSwitch;

        if ( *switch_list == '*' )
                gchExclusiveMode = gchInclusiveMode = 0;

        // check for and remove switches anywhere in the line
        for ( i = 0, *fFlags = 0L; (( arg = ntharg( line, i )) != NULL ); ) {

                if ( *arg == gpIniptr->SwChr ) {

                        if (( lSwitch = switch_arg( arg, switch_list )) == -1L )
                                return ERROR_EXIT;

                        if ( lSwitch == 0 ) {
                                i++;
                                continue;
                        }

                        *fFlags |= lSwitch;

                        // remove the switch(es)
                        if ( fOnlyFirst )
                                strcpy( gpNthptr, skipspace( gpNthptr + strlen( arg )));
                        else
                                strcpy( gpNthptr, gpNthptr + strlen( arg ) );

                } else if ( fOnlyFirst ) {
                        // only want opts at beginning!
                        break;
                } else
                        i++;
        }

        return 0;
}


// check arg for the switch arguments - return 0 if not a switch, -1 if
//   a bad switch argument, else ORed the matching arguments (1 2 4 8 etc.)
long switch_arg( char *arg, char *match )
{
        int i;
        long lVal = 0L, lTemp;
        char *ptr;

        if (( arg != NULL ) && ( *arg++ == gpIniptr->SwChr )) {

                // check for /A:rhsda
                if (( _ctoupper( *arg ) == 'A' ) && ( arg[1] == ':' ) && ( *match == '*' )) {
                        // set gchInclusiveMode and gchExclusiveMode
                        GetSearchAttributes( arg + 1 );
                        return 1;
                }

                for ( i = 0; ( arg[i] != '\0' ); i++ ) {

                        // skip '*' denoting /A:xxx field
                        if ((( ptr = strchr( match, _ctoupper( arg[i] ))) == NULL ) || ( *ptr == '*' )) {

                                if ( isalpha( arg[i] )) {
                                        error( ERROR_INVALID_PARAMETER, arg );
                                        return -1L;
                                }

                                break;

                        } else {
                                lTemp = (long)( ptr - match );
                                lVal |= ( 1L << lTemp );
                        }
                }
        }

        return lVal;
}


// returns 1 if the argument is numeric
int QueryIsNumeric( char *pszNum )
{
        if (( pszNum == NULL ) || ( *pszNum == '\0' ))
                return 0;

        if (( is_signed_digit( *pszNum ) != 0 ) || (( *pszNum == gaCountryInfo.szDecimal[0] ) && ( isdigit ( pszNum[1] )) && ( strchr( pszNum+1, gaCountryInfo.szDecimal[0] ) == NULL ))) {
                for ( ++pszNum; (( *pszNum != '\0' ) && ( isdigit( *pszNum ) || ( *pszNum == gaCountryInfo.szThousandsSeparator[0] ) || ( *pszNum == gaCountryInfo.szDecimal[0] ))); pszNum++ )
                    ;
        }

        return ( *pszNum == '\0' );
}


// is the filename "CON"
int QueryIsCON( char *pszFileName )
{
        return (( _stricmp( pszFileName, CONSOLE ) == 0 ) || ( _stricmp( pszFileName, DEV_CONSOLE ) == 0 ));
}


// test if relative range spec is composed entirely of digits
int QueryIsRelative( char *str )
{
        for ( ; (*str != '\0' ); str++) {
                if ((*str < '0' ) || (*str > '9' )) {
                        if ((*str == ',' ) || (*str == '@' ) || (*str == ']' ))
                                break;
                        return 0;
                }
        }

        return 1;
}


// set the date / time / size range info for file searches
int GetRange( char *line, RANGES *aRanges, int fOnlyFirst )
{
        static char szExclude[MAXLINESIZ];

        char *arg;
        char *pSaveNthptr;
        char szBuffer[MAXLINESIZ];
        int i, fExclude, rval = 0;

        // initialize start & end values

        szExclude[0] = '\0';
        aRanges->pszExclude = NULL;

        // date & inclusive time
        aRanges->DTRS.DTStart = 0L;
        aRanges->DTRE.DTEnd = (ULONG)-1L;

        // exclusive time
        aRanges->TimeStart = 0;
        aRanges->TimeEnd = 0xFFFF;

        // size
        aRanges->SizeMin = 0L;
        aRanges->SizeMax = (ULONG)-1L;

        // default to "last write" on LFN / HPFS / NTFS
        aRanges->DateType = aRanges->TimeType = 0;
        if ( line == NULL )
                return 0;

        // check for range switches in the line
        for ( i = 0; (( arg = ntharg( line, i )) != NULL ); ) {

                // if not a range argument, get next switch
                if (( *arg != gpIniptr->SwChr ) || ( arg[1] != '[' )) {
                        if ( fOnlyFirst )
                                return 0;
                        i++;
                        continue;
                }

                // get start & end of switch
                pSaveNthptr = arg = gpNthptr + 1;

                // get start & end of switch
                if (( arg = scan( arg, "]", "`\"[" )) == BADQUOTES )
                        return ERROR_EXIT;
                if ( *arg == ']' )
                        *arg++ = '\0';

                // check for filename exclusions
                pSaveNthptr++;
                if ( *pSaveNthptr == '!' ) {
                        fExclude = 1;
                        pSaveNthptr++;
                } else
                        fExclude = 0;

                pSaveNthptr = strcpy( gpNthptr, pSaveNthptr );

                // kludge to allow variable expansion on range arguments
                //   for commands like EXCEPT, FOR, and SELECT
                strcpy( szBuffer, pSaveNthptr );

                if ( var_expand( szBuffer, 1 ) != 0 )
                        return ERROR_EXIT;

                if ( fExclude ) {

                        if ( szExclude[0] != '\0' )
                                return ( error( ERROR_4DOS_ALREADYEXCLUDED, szBuffer ));
                        sprintf( szExclude, FMT_PREC_STR, MAXLINESIZ - 1, szBuffer );
                        aRanges->pszExclude = szExclude;

                } else if (( rval = GetRangeArgs( szBuffer, aRanges )) != 0 )
                        return ( error( rval, line ));

                // remove the switch
                if ( fOnlyFirst )
                        arg = skipspace( arg );
                strcpy( pSaveNthptr, arg );
        }

        return 0;
}


// skip to the next range argument
static char * NextRange(char *arg)
{
        while (( *arg ) && ( *arg != ']' ) && ( *arg++ != ',' ))
                ;

        return ( skipspace( arg ));
}

// TODO: revise signed/unsigned issues
static int GetRangeArgs( char *arg, RANGES *aRanges )
{
        int c;
        int hours, minutes;
        unsigned int year, month, day;
        long lStart, lEnd;
        unsigned long lOffset;
        DATETIME sysDateTime;

        switch ( _ctoupper( *arg++ )) {
        case 'D':               // date range
                // check for Last Access or Created request
                if (( c = _ctoupper( *arg )) == 'A' ) {
                        aRanges->DateType = 1;
                        arg++;
                } else if ( c == 'C' ) {
                        aRanges->DateType = 2;
                        arg++;
                } else if ( c == 'W' )
                        arg++;

                // get current date & time
                MakeDaysFromDate( (unsigned long *)&lStart, NULLSTR );
                lEnd = lStart;

                // get first arg
                if ( is_signed_digit( *arg )) {

                    if ( MakeDaysFromDate( &lOffset, arg ) != 0 )
                        return ERROR_4DOS_INVALID_DATE;

                    if ( *arg == '-' )
                        lStart += lOffset;
                    else if ( *arg == '+' )
                        lEnd += lOffset;
                    else
                        lStart = lOffset;
                }

                // skip past separator
                while (( *arg ) && ( *arg != ']' )) {

                    // get time spec
                    if ( *arg == '@' ) {
                        if ( GetStrTime( ++arg, &hours, &minutes ) != 0 )
                            return ERROR_4DOS_INVALID_TIME;
                        aRanges->DTRS.DTS.TStart = (unsigned int)(( hours << 11 ) + (minutes << 5));
                    }

                    if ( *arg++ == ',' )
                        break;
                }

                arg = skipspace( arg );

                // get second arg
                if ( is_signed_digit( *arg )) {

                    if (MakeDaysFromDate( &lOffset, arg ) != 0 )
                        return ERROR_4DOS_INVALID_DATE;

                    if (*arg == '-' ) {
                        lEnd = lStart;
                        lStart += lOffset;
                    } else if (*arg == '+' )
                        lEnd = lStart + lOffset;
                    else if (( lEnd = lOffset) < lStart) {
                        lEnd = lStart;
                        lStart = lOffset;
                    }
                }

                // skip past date
                while ((*arg ) && (*arg != ']' )) {

                    // get time spec
                    if (*arg++ == '@' ) {
                        if (GetStrTime( arg, &hours, &minutes ) != 0 )
                            return ERROR_4DOS_INVALID_TIME;
                        aRanges->DTRE.DTE.TEnd = (unsigned int)((hours << 11) + (minutes << 5));
                    }
                }

                MakeDateFromDays( lStart, &year, &month,&day );
                aRanges->DTRS.DTS.DStart = ((year - 80 ) << 9) + (month << 5) + day;

                MakeDateFromDays( lEnd, &year, &month, &day );
                aRanges->DTRE.DTE.DEnd = ((year - 80 ) << 9) + (month << 5) + day;

                break;

        case 'S':               // size range
                // get first arg
// FIXME - add support for LONGLONG's!
                GetStrSize( arg, &lOffset );
                aRanges->SizeMin = lOffset;

                // skip past separator
                arg = NextRange( arg );

                // get second arg
                if ( is_signed_digit( *arg )) {

                        GetStrSize( arg, &lOffset );

                        if ( *arg == '-' ) {
                                aRanges->SizeMax = aRanges->SizeMin;
                                aRanges->SizeMin += lOffset;
                        } else if ( *arg == '+' )
                                aRanges->SizeMax = aRanges->SizeMin + lOffset;
                        else
                                aRanges->SizeMax = lOffset;
                }

                break;

        case 'T':               // time range
                // check for Last Access or Created request
                if ((c = _ctoupper(*arg )) == 'A' ) {
                        aRanges->TimeType = 1;
                        arg++;
                } else if (c == 'C' ) {
                        aRanges->TimeType = 2;
                        arg++;
                } else if (c == 'W' )
                        arg++;

                // get current date & time
                QueryDateTime( &sysDateTime );
                lStart = (sysDateTime.hours * 60 ) + sysDateTime.minutes;
                lEnd = lStart;

                // get first arg
                if ( GetStrTime( arg, &hours, &minutes ) != 0 )
                        return ERROR_4DOS_INVALID_TIME;

                if ( *arg == '-' )
                        lStart += (long)hours;
                else if ( *arg == '+' )
                        lEnd += (long)hours;
                else
                        lStart = (hours * 60 ) + minutes;

                // skip past separator
                arg = NextRange( arg );

                // get second arg
                if ( is_signed_digit( *arg )) {

                    if ( GetStrTime( arg, &hours, &minutes ) != 0 )
                        return ERROR_4DOS_INVALID_TIME;

                    if ( *arg == '-' ) {
                        lEnd = lStart;
                        lStart += (long)hours;
                    } else if (*arg == '+' )
                        lEnd = lStart + (long)hours;
                    else if (( lEnd = (hours * 60 ) + minutes) < lStart) {
                        lOffset = lEnd;
                        lEnd = lStart;
                        lStart = lOffset;
                    }
                }

                // check for lStart < 0 or lEnd > 23:59, & adjust time
                if ( lStart < 0L )
                        lStart += 1440L;
                if ( lEnd >= 1440L )
                        lEnd -= 1440L;

                aRanges->TimeStart = (unsigned int)((( lStart / 60 ) << 11) + (( lStart % 60 ) << 5));
                aRanges->TimeEnd = (unsigned int)((( lEnd / 60 ) << 11) + (( lEnd % 60 ) << 5));
        }

        return 0;
}


// get time
static int GetStrTime(char *arg, int *hours, int *minutes)
{
        char szAmPm[2];

        *hours = *minutes = 0;
        szAmPm[0] = '\0';

        if ( is_signed_digit( *arg ) == 0 )
                return ERROR_EXIT;

        if (( *arg == '+' ) || ( *arg == '-' ) || ( QueryIsRelative( arg ))) {

                *hours = atoi( arg );
                if (is_unsigned_digit(*arg ))
                        *arg = '+';

        } else if ( sscanf( arg,"%u%*1s%u%*u %1[APap]",hours,minutes,szAmPm) >= 1) {

                // check for AM/PM syntax
                if (*szAmPm != '\0' ) {
                        if ((*hours == 12) && (_ctoupper(*szAmPm) == 'A' ))
                                *hours -= 12;
                        else if ((_ctoupper(*szAmPm) == 'P' ) && (*hours < 12))
                                *hours += 12;
                }
        }

        return 0;
}


// get size
static int GetStrSize(char *arg, unsigned long *lSize )
{
        *lSize = 0L;
        sscanf( arg, FMT_LONG, lSize );
        while ( is_signed_digit( *arg ))
                arg++;

        // check for Kb or Mb in size range
        if ( *arg == 'k' )
                *lSize *= 1000L;
        else if ( *arg == 'K' )
                *lSize <<= 10;
        else if ( *arg == 'm' )
                *lSize *= 1000000L;
        else if ( *arg == 'M' )
                *lSize <<= 20;

        return 0;
}


// return the date from a string
int GetStrDate(char *arg, unsigned int *month, unsigned int *day, unsigned int *year)
{
        int rval;

        if ( gaCountryInfo.fsDateFmt == 0 ) {
                // USA
                rval = sscanf( arg, DATE_FMT, month, day, year );
        } else if ( gaCountryInfo.fsDateFmt == 1 ) {
                // Europe
                rval = sscanf( arg, DATE_FMT, day, month, year );
        } else {
                // Japan
                rval = sscanf( arg, DATE_FMT, year, month, day );
        }

        return rval;
}


// get the number of days since 1/1/80 for the specified date
int MakeDaysFromDate( unsigned long *lDays, char *szDate )
{
        unsigned int i;
        unsigned int year = 80, month = 1, day = 1;
        int rval = 0;
        DATETIME sysDateTime;

        *lDays = 0L;

        // if arg == NULLSTR, use current date
        if ( *szDate == '\0' ) {

                QueryDateTime(&sysDateTime );
                year = sysDateTime.year;
                month = sysDateTime.month;
                day = sysDateTime.day;
                rval = 3;

        } else if (( *szDate == '+' ) || ( *szDate == '-' ) || ( QueryIsRelative( szDate ))) {

                sscanf( szDate, FMT_LONG, lDays );

                // if it's not a date spec, force it to be a "+ relative" spec
                if ( is_unsigned_digit( *szDate ))
                        *szDate = '+';
                return 0;

        } else
                rval = GetStrDate( szDate, &month, &day, &year );

        if ( year < 80 )
                year += 2000;
        else if ( year < 100 )
                year += 1900;

        // don't allow anything before 1/1/1980
        if (( rval < 3 ) || ( month > 12 ) || ( day > 31 ) || ( year > 2099 ) || ( year < 1980 ))
                return ERROR_4DOS_INVALID_DATE;

        // get days for previous years
        for ( i = 1980; ( i < year ); i++ )
                *lDays += ((( i % 4 ) == 0 ) ? 366 : 365 );

        // get days for previous months
        for ( i = 1; ( i < month ); i++ ) {
            if ( i == 2 )
                *lDays += ((( year % 4 ) == 0 ) ? 29 : 28 );
            else
                *lDays += ((( i == 4 ) || ( i == 6 ) || ( i == 9 ) || (i == 11)) ? 30 : 31);
        }

        *lDays += ( day - 1 );

        return 0;
}


// get a date from the number of days since 1/1/80
int MakeDateFromDays( long lDays, unsigned int *year, unsigned int *month, unsigned int *day)
{
        // don't allow anything past 12/31/2099
        if (( lDays > 43829L ) || ( lDays < 0L ))
                return ERROR_INVALID_PARAMETER;

        for (*year = 80; ; (*year)++) {

            for (*month = 1; (*month <= 12); (*month)++) {

                if (*month == 2)
                        *day = (((*year % 4) == 0 ) ? 29 : 28);
                else
                        *day = (((*month == 4) || (*month == 6) || (*month == 9) || (*month == 11)) ? 30 : 31);

                if ((LONG)*day <= lDays)
                        lDays -= *day;
                else {
                        *day = (unsigned int)( lDays + 1);
                        return 0;
                }
            }
        }
}


// remove whitespace on both sides of the token
void collapse_whitespace(char *arg, char *token)
{
        // collapse any whitespace around the token
        for ( ; ; ) {

                if (( arg == NULL ) || (( arg = scan( arg, token, QUOTES )) == BADQUOTES ) || ( *arg == '\0' ))
                        return;

                // collapse leading whitespace
                while ( iswhite( arg[-1] )) {
                        arg--;
                        strcpy( arg, arg+1 );
                }

                // collapse following whitespace
                arg++;
                while ( iswhite( *arg ))
                        strcpy( arg, arg+1 );
        }
}


// strip the specified leading characters
void strip_leading( char *arg, char *delims )
{
        while (( *arg != '\0' ) && ( strchr( delims, *arg ) != NULL ))
                strcpy( arg, arg+1 );
}


// strip the specified trailing characters
void strip_trailing(char *arg, char *delims)
{
        int i;

        for ( i = strlen( arg ); ((--i >= 0 ) && ( strchr( delims, arg[i] ) != NULL )); )
                arg[i] = '\0';
}


// look for TMP / TEMP / TEMP4DOS disk area defined in environment
char *GetTempDirectory( char *pszName )
{
        char *feptr;

        if (( feptr = get_variable( "TMP" )) != 0L ) {
                strcpy( pszName, feptr );
                if ( is_dir( pszName ) == 0 )
                        feptr = 0L;
        }

        return feptr;
}

// check to see if user wants filenames in upper case
char * filecase( char *filename )
{
#if _NT
        // in NT 3.5, don't do _any_ case shifts
        if (( gnOsVersion >= 350 ) && ( gpIniptr->Upper == 0 ))
                return filename;
#endif
        // HPFS & NTFS preserve case in filenames, so just leave it alone
        if ( ifs_type( filename ) != 0 )
                return filename;

        return (( gpIniptr->Upper == 0 ) ? strlwr( filename ) : strupr( filename ));
}


void SetDriveString( char *pszDrives )
{
        int i, n;

        // get all of the valid hard disks from C -> Z
        memset( pszDrives, '\0', 30 );
        for ( i = 0, n = 3; ( n <= 26 ); n++ ) {
            pszDrives[i] = n + 64;
            if (( QueryDriveExists( n )) && ( QueryDriveReady( n )) && ( QueryIsCDROM( pszDrives+i ) == 0 ) && ( QueryDriveRemote( n ) == 0 ) && ( QueryDriveRemovable( n ) == 0 ))
                i++;
        }

        pszDrives[i] = '\0';
}


// get current directory, with leading drive specifier (or NULL if error)
char * gcdir( char *pszDrive, int fNoError )
{
        static char szCurrent[MAXFILENAME];
        int disk, rval;
        unsigned long DirPathLen = MAXPATH;

        disk = gcdisk( pszDrive );
        sprintf( szCurrent, FMT_ROOT, disk+64 );
        rval = DosQueryCurrentDir( disk, szCurrent+3, &DirPathLen );

        if (rval) {

                if ( fNoError == 0 ) {
                        // bad drive - format error message & return NULL
                        sprintf( szCurrent, FMT_DISK, disk+64 );
                        error( rval, szCurrent );
                }

                return NULL;
        }

        return ( filecase( szCurrent ));
}


// return the specified (or default) drive as 1=A, 2=B, etc.
int gcdisk( char *drive_spec )
{
        int nDrive;
        if (( drive_spec != NULL ) && ( drive_spec[1] == ':' ))
                return ( _ctoupper( *drive_spec ) - 64);
        if (( nDrive = _getdrive()) < 0 )
                nDrive = 0;
        return nDrive;
}


// return 1 if the specified name is "." or ".."
int QueryIsDotName( char *pszFileName )
{
        return (( *pszFileName == '.' ) && (( pszFileName[1] == '\0' ) || (( pszFileName[1] == '.' ) && ( pszFileName[2] == '\0' ))));
}


// return the path stripped of the filename (or NULL if no path)
char * path_part( char *s )
{
        static char buffer[MAXFILENAME];

        copy_filename( buffer, s );
        StripQuotes( buffer );

        // search path backwards for beginning of filename
        for ( s = strend( buffer ); ( --s >= buffer ); ) {

                // accept either forward or backslashes as path delimiters
                if (( *s == '\\' ) || ( *s == '/' ) || ( *s == ':' )) {
                        // take care of arguments like "d:.." & "..\.."
                        if ( _stricmp( s+1, ".." ) != 0 )
                                s[1] = '\0';
                        return buffer;
                }
        }

        return NULL;
}


// return the filename stripped of path & disk spec
char * fname_part( char *pszFileName )
{
        static char buf[MAXFILENAME];
        char *s;

        // search path backwards for beginning of filename
        for ( s = strend( pszFileName ); ( --s >= pszFileName ); ) {

                // accept either forward or backslashes as path delimiters
                if (( *s == '\\' ) || ( *s == '/' ) || ( *s == ':' )) {

                        // take care of arguments like "d:.." & "..\.."
                        if ( _stricmp( s+1, ".." ) == 0 )
                                s += 2;
                        break;
                }
        }

        // step past the delimiter char
        s++;

        // allow LFN / HPFS / NTFS long filenames
        copy_filename( buf, s );

        StripQuotes( buf );
        return buf;
}


// return the file extension only
char * ext_part( char *s )
{
        static char szBuf[65];
        char *ptr;

        // make sure extension is for filename, not a directory name
        if (( s == NULL ) || (( ptr = strrchr( s, '.' )) == NULL ) || ( strpbrk( ptr, "\\/:" ) != NULL ))
                return NULL;

        // don't read the next element in an include list
        sscanf( ptr, "%64[^;\"]", szBuf );

        return szBuf;
}


// copy a filename, max of 260 characters
void copy_filename(char *target, char *source )
{
        sprintf( target, FMT_PREC_STR, MAXFILENAME-1, source );
}


// check for LFN/HPFS/NTFS long filenames delineated by double quotes
//   and strip leading / trailing "
void StripQuotes( char *pszFileName )
{
        char *ptr;

        while (( ptr = strchr( pszFileName, '"' )) != NULL )
                strcpy( ptr, ptr+1 );

        // remove trailing whitespace
        strip_trailing( pszFileName, WHITESPACE );
}


void AddCommas( char *var )
{
        char *arg;

        // format a long integer by inserting commas (or other
        // character specified by country_info.szThousandsSeparator)
        if (( *var == '-' ) || ( *var == '+' ))
                var++;
        for ( arg = var; (( isdigit( *arg )) && ( *arg != '\0' ) && ( *arg != gaCountryInfo.szDecimal[0] )); arg++ )
                ;
        while (( arg -= 3 ) > var )
                strins( arg, gaCountryInfo.szThousandsSeparator );
}


int AddQuotes( char *pszFileName )
{
        // adjust for an LFN name with embedded whitespace
        if (( *pszFileName != '"' ) && ( strpbrk( pszFileName, " \t,=+&<>|" ) != NULL )) {
                strins( pszFileName, "\"" );
                strcat( pszFileName, "\"" );
                return 1;
        }

        return 0;
}


// make a file name from a directory name by appending '\' (if necessary)
//   and then appending the filename
int mkdirname( char *dname, char *pszFileName )
{
        int length;

        length = strlen( dname );

        if ( length >= ( MAXFILENAME - 2 ))
                return ERROR_EXIT;

        if (( *dname ) && ( strchr( "/\\:", dname[length-1] ) == NULL ) && ( *pszFileName != '\\' )) {
                strcat( dname, "\\" );
                length++;
        }

        sprintf( strend( dname ), FMT_PREC_STR, (( MAXFILENAME - 1 ) - length ), pszFileName );

        return 0;
}


// manufacture a short 8.3 name from an LFN / HPFS / NTFS name
int MakeShortFromLong( char *pszLong )
{
        char *arg;
        char szShort[MAXFILENAME+1];
        int i;

        if (( arg = strrchr( pszLong, '.' )) != NULL ) {
                *arg++ = '\0';
                sprintf( szShort, "%.8s.%.3s", pszLong, arg );
        } else
                sprintf( szShort, "%.8s", pszLong );

        for ( arg = szShort ; ( *arg != '\0' ); arg++ ) {
                if (iswhite( *arg ))
                        *arg = '_';
        }

        arg = strend( szShort ) - 1;
        for ( i = '0'; ( is_file( szShort )); i++ ) {

                if ( ext_part( szShort ) == NULL ) {
                        strcat( szShort, ".000" );
                        arg = strend( szShort ) - 1;
                }

                if ( *arg == '.' )
                        return ERROR_EXIT;

                if ( i == '9' ) {
                        arg--;
                        i = '0';
                }

                *arg = (char)i;
        }

        return 0;
}


// make a full file name including disk drive & path
char * mkfname( char *pszFileName, int fFlags )
{
        char *ptr;
        char temp[MAXFILENAME], *source, *curdir = NULLSTR;

        if ( pszFileName != NULL )
                StripQuotes( pszFileName );

        if (( pszFileName == NULL ) || ( *pszFileName == '\0' )) {
                if (( fFlags & MKFNAME_NOERROR ) == 0 )
                        error( ERROR_FILE_NOT_FOUND, NULLSTR );
                return NULL;
        }

        source = pszFileName;

        // check for network server names (Novell Netware or Lan Manager)
        //   or a named pipe ("\pipe\name...")
        if (( is_net_drive( pszFileName )) || ( QueryIsPipeName( pszFileName ))) {
                // don't do anything if it's an oddball network name - just
                //   return the filename, shifted to upper or lower case
                return ( filecase( pszFileName ));
        }

        // skip the disk specification
        if (( *pszFileName ) && ( pszFileName[1] == ':' ))
                pszFileName += 2;

        // get the current directory, & save to the temp buffer
        if (( curdir = gcdir( source, ( fFlags & MKFNAME_NOERROR ))) == NULL )
                return NULL;

        // skip the disk spec for the default directory
        copy_filename( temp, curdir );
        if (( temp[0] == '\\' ) && ( temp[1] == '\\' )) {

                // if it's a UNC name, skip the machine & sharename
                curdir = temp + strlen( temp );
                if (( ptr = strchr( temp+2, '\\' )) != NULL ) {
                        if (( ptr = strchr( ptr + 1, '\\' )) != NULL )
                                curdir = ptr + 1;
                }
        } else
                curdir = temp + 3;

        // if filespec defined from root, there's no default pathname
        if (( *pszFileName == '\\' ) || ( *pszFileName == '/' )) {
                pszFileName++;
                *curdir = '\0';
        }

        // handle Netware-like CD changes (..., ...., etc.)
        while (( ptr = strstr( pszFileName, "..." )) != NULL ) {

                char *ptr2;

                // HPFS and NTFS allow names like "abc...def"
                if ( ifs_type( temp ) != 0 ) {

                        // check start
                        if (( ptr > pszFileName ) && ( ptr[-1] != '/' ) && ( ptr[-1] != '\\' ))
                                break;

                        // check end
                        for ( ptr2 = ptr; ( *ptr2 == '.' ); ptr2++ )
                                ;
                        if (( *ptr2 != '\0' ) && ( *ptr2 != '\\' ) && ( *ptr2 != '/' ))
                                break;
                }

                // make sure we're not over the max length
                if (( strlen( pszFileName ) + 4 ) >= MAXFILENAME ) {
Mkfname_Error:
                        if (( fFlags & MKFNAME_NOERROR ) == 0 )
                                error( ERROR_FILE_NOT_FOUND, pszFileName );
                        return NULL;
                }

                strins( ptr+2, "\\." );
        }

        while (( pszFileName != NULL ) && ( *pszFileName )) {

                // don't strip trailing \ in directory name
                if (( ptr = strpbrk( pszFileName, "/\\" )) != NULL ) {
                        if (( ptr[1] != '\0' ) || ( ptr[-1] == '.' ))
                                *ptr = '\0';
                        ptr++;
                }

                // handle references to parent directory ("..")
                if ( _stricmp( pszFileName, ".." ) == 0 ) {

                        // don't delete root directory!
                        if (( pszFileName = strrchr( curdir, '\\' )) == NULL )
                                pszFileName = curdir;
                        *pszFileName = '\0';

                } else if ( _stricmp( pszFileName, "." ) != 0 ) {
                        // append the directory or filename to the temp buffer
                        if ( mkdirname( temp, pszFileName ))
                                goto Mkfname_Error;
                }

                pszFileName = ptr;
        }

        copy_filename( source, temp );

        // return expanded filename, shifted to upper or lower case
        return ( filecase( source ));
}


// make a filename with the supplied path & filename
void insert_path( char *target, char *source, char *path )
{
        // get path part first in case "path" and / or "source" are the same
        //   arg as "target"
        path = path_part( path );
        copy_filename( target, source );

        if ( path != NULL )
                strins( target, path );
}


// return non-zero if the specified file exists
int is_file( char *pszFileName )
{
        char wname[MAXFILENAME];
        FILESEARCH dir;

        copy_filename( wname, pszFileName );

        // check for valid drive, then expand filename to catch things
        //   like "\4dos\." and "....\wonky\*.*"
        if ( mkfname( wname, MKFNAME_NOERROR ) != NULL ) {
                if ( find_file( FIND_FIRST, wname, 0x2107, &dir, NULL ) != NULL )
                        return 1;
        }

        return 0;
}


// return non-zero if the specified file or directory exists
int is_file_or_dir( char *pszFileName )
{
        int fNoDots;
        char wname[MAXFILENAME];
        FILESEARCH dir;

        // check for valid drive, then expand filename to catch things
        //   like "\4dos\." and "....\wonky\*.*"
        copy_filename( wname, pszFileName );

        if ( mkfname( wname, MKFNAME_NOERROR ) != NULL ) {

                strip_trailing( (( wname[1] == ':' ) ? wname + 3 : wname + 1 ), "\\/" );

                // if filename has wildcards, don't return "." or ".."
                fNoDots = ( strpbrk( wname, WILD_CHARS ) != NULL );
                if ( find_file( FIND_FIRST, wname, (( fNoDots ) ? 0xA117 : 0x2117), &dir, NULL ) != NULL )
                        return (( dir.attrib & 0x10 ) ? 0x10 : 1 );
        }

        return 0;
}


// returns 1 if it's a directory, 0 otherwise
int is_dir( char *pszFileName )
{
        char *ptr;
        char szDirName[MAXFILENAME];
        FILESEARCH dir;

        ptr = pszFileName;

        // check to see if the drive exists
        if ( pszFileName[1] == ':' ) {

                // Another Netware kludge:  skip past "d:" when scanning for
                //   wildcards to allow for wacko drive names like ]:
                ptr += 2;

                if ( QueryDriveExists( gcdisk( pszFileName )) == 0 )
                        return 0;
        }

        // it can't be a directory if it has wildcards or include list
        if ( strpbrk( ptr, "*?" ) != NULL )
                return 0;

        if ((( ptr = scan( ptr, "[;", "\"" )) != BADQUOTES ) && ( *ptr == '\0' )) {

                // build a fully-qualified path name (& fixup Netware stuff)
                copy_filename( szDirName, pszFileName );
                if (( mkfname( szDirName, MKFNAME_NOERROR ) == NULL ) || ( szDirName[0] == '\0' ))
                        return 0;

                // "d:", "d:\" and "d:/" are assumed valid directories
                // Novell names like "SYS:" are also assumed valid directories
                if ((( szDirName[2] == '\0' ) || (( szDirName[2] == '\\' ) ||
                  ( szDirName[2] == '/' )) && (szDirName[3] == '\0' )) ||
                  (((( ptr = strchr( szDirName+2, ':' )) != NULL ) &&
                  ( ptr[1] == '\0' ) && ((unsigned int)( ptr - szDirName ) < 6 ) &&
                  ( strpbrk( szDirName, " ,\t=" ) == NULL )) &&
                  ( QueryIsDevice( szDirName ) == 0 )))
                        return 1;

                // remove a trailing "\" or "/"
                strip_trailing( szDirName, SLASHES );

                // try searching for it & see if it exists
                if ( find_file( FIND_FIRST, szDirName, 0x2317, &dir, NULL ) != NULL )
                        return (( dir.attrib & 0x10 ) ? 1 : 0 );

                // kludge for "\\server\dir" not working in NT & OS/2 & Netware
                if (( szDirName[0] == '\\' ) && ( szDirName[1] == '\\' )) {

                        // see if there are any subdirectories or files
                        mkdirname( szDirName, WILD_FILE );
                        if ( find_file( FIND_FIRST, szDirName, 0x2117, &dir, NULL ) != NULL )
                                return 1;
                }
        }

        return 0;
}


// check for network server names (Novell Netware or Lan Manager)
//   (like "\\server\blahblah" or "sys:blahblah")
int is_net_drive( char *pszFileName )
{
        return ((( pszFileName[0] == '\\' ) && ( pszFileName[1] == '\\' )) || (( pszFileName[1] != '\0' ) && ( strchr( pszFileName+2, ':' ) != NULL )));
}


// check for user defined executable extension
//   (kinda kludgy in order to support wildcards)
char *executable_ext( char *ptr )
{
        PCH feptr;

        for ( ptr++, feptr = glpEnvironment; ; feptr = next_env( feptr )) {

                if ( *feptr == '\0' ) {
                        break;
                }

                if (( *feptr++ == '.' ) && ( wild_cmp( feptr, (char *)ptr, TRUE, TRUE ) == 0 )) {

                        // get the argument
                        while (( *feptr ) && ( *feptr++ != '=' ))
                                ;
                        break;
                }

// FIXME!
//              while (( *feptr ) && ( *feptr++ != ';' ))
//                      ;
        }

        return feptr;
}


// Compare filenames for wildcard matches
//   Returns 0 for match; <> 0 for no match
//   *s  matches any collection of characters in the string
//         up to (but not including) the s.
//    ?  matches any single character in the other string.
//   [!abc-m] match a character to the set in the brackets; ! means reverse
//         the test; - means match if included in the range.
int wild_cmp( char *fpWildName, char *fpFileName, int fExtension, int fWildBrackets )
{
        int fWildStar = 0;
        char *w_start, *f_start;

        // skip ".." and "." (will only match on *.*)
        //   but add a kludge for HPFS names like ".toto"
        //   and another kludge for "[!.]*"
        if (( fpFileName[0] == '.' ) && ( *fpWildName != '[' ) && ( fExtension )) {
                if (( fpFileName[1] == '.' ) && ( fpFileName[2] == '\0' ))
                        fpFileName += 2;
                else if ( fpFileName[1] == '\0' )
                        fpFileName++;
        }

        for ( ; ; ) {

                // skip quotes in LFN or HPFS-style names
                while ( *fpWildName == '"' )
                        fpWildName++;

                if ( *fpWildName == '*' ) {

                        // skip past * and advance fpFileName until a match
                        //   of the characters following the * is found.
                        for ( fWildStar = 1; (( *(++fpWildName) == '*' ) || ( *fpWildName == '?' )); )
                                ;

                } else if ( *fpWildName == '?' ) {

                        // ? matches any single character (or possibly
                        //   no character, if at start of extension or EOS)
                        if (( *fpFileName == '.' ) && ( fExtension )) {

                                // beginning of extension - throw away any
                                //   more wildcards
                                while (( *(++fpWildName ) == '?' ) || ( *fpWildName == '*' ))
                                        ;
                                if ( *fpWildName == '.' )
                                        fpWildName++;
                                fpFileName++;

                        } else {
                                if ( *fpFileName )
                                        fpFileName++;
                                fpWildName++;
                        }

                } else if (( fWildBrackets ) && ( *fpWildName == '[' ) && ( fWildStar == 0 )) {

                        // [ ] checks for a single character (including ranges)
                        if ( wild_brackets( fpWildName++, *fpFileName, TRUE ) != 0 )
                                break;

                        if ( *fpFileName )
                                fpFileName++;

                        while (( *fpWildName ) && ( *fpWildName++ != ']' ))
                                ;

                } else {

                        if ( fWildStar ) {

                                // following a '*'; so we need to do a complex
                                //   match since there could be any number of
                                //   preceding characters
                                for ( w_start = fpWildName, f_start = fpFileName; (( *fpFileName ) && ( *fpWildName != '*' )); ) {

                                        if (( *fpFileName == '.' ) && ( fExtension )) {
                                                // kludge for "*.[!x]*" in LFNs
                                                if ( *fpWildName == '.' ) {

                                                    if (( fpWildName[1] == '[' ) || ( fpWildName[1] == '*' ) || ( fpWildName[1] == '?' ))
                                                        break;
                                                }
                                        }

                                        if ( *fpWildName == '[' ) {

                                                // get the first matching char
                                                if ( wild_brackets( fpWildName, *fpFileName, TRUE ) == 0 ) {

                                                        while (( *fpWildName ) && ( *fpWildName++ != ']' ))
                                                                ;
                                                        fpFileName++;
                                                        continue;
                                                }
                                        }

                                        if (( *fpWildName != '?' ) && ( _ctoupper( *fpWildName ) != _ctoupper( *fpFileName ))) {
                                                fpWildName = w_start;
                                                fpFileName = ++f_start;
                                        } else {
                                                fpWildName++;
                                                fpFileName++;
                                        }
                                }

                                // if "fpWildName" is still an expression, we failed
                                //   to find a match
                                if ( *fpWildName == '[' )
                                        break;

                                fWildStar = 0;

                        } else if (( _ctoupper( *fpWildName ) == _ctoupper( *fpFileName )) && ( *fpWildName != '\0' )) {

                                fpWildName++;
                                fpFileName++;

                        } else if (( *fpWildName == '.' ) && ( *fpFileName == '\0' ) && ( fExtension ))
                                fpWildName++;   // no extension

                        else
                                break;
                }
        }

        // a ';' means we're at the end of a filename in a group list
        // a '=' means we're at the end of an executable extension definition
        // a '"' means we're at the end of a quoted filename
        return ((( *fpWildName == ';' ) || ( *fpWildName == '=' ) || ( *fpWildName == '"' )) ? *fpFileName : *fpWildName - *fpFileName );
}


// Evaluate contents of brackets versus a specified character
// Returns 0 for match, 1 for failure
int wild_brackets( char *str, int c, int fIgnoreCase )
{
        int inverse = 0;

        if ( fIgnoreCase )
                c = _ctoupper( c );

        // check for inverse bracket "[!a-c]"
        if ( *(++str) == '!' ) {
                str++;
                inverse++;
        }

        // check for [] or [!] match
        if (( *str == ']' ) && ( c == 0 ))
                return (( inverse ) ? 1 : 0 );

        // loop til ending bracket or until compare fails
        for ( ; ; str++ ) {

                if (( *str == ']' ) || ( *str == '\0' )) {
                        inverse--;
                        break;
                }

                if ( str[1] == '-' ) {          // range test

                        if (( _ctoupper( *str ) <= _ctoupper( c )) && ( _ctoupper( c ) <= _ctoupper( str[2] )))
                                break;
                        str += 2;

                // single character
                } else if ( *str == '?' ) {

                        // kludge for [!?]
                        if ( c == 0 )
                                inverse--;
                        break;

                } else if ( c == (( fIgnoreCase ) ? _ctoupper( *str ) : *str ))
                        break;
        }

        return (( inverse ) ? 1 : 0 );
}


// exclude the specified file(s) from a directory search
int ExcludeFiles( char *szFiles, char *szFilename )
{
        char *arg;
        int i;

        for ( i = 0; (( arg = ntharg( szFiles, i )) != NULL ); i++ ) {
                StripQuotes( arg );
                if ( wild_cmp( (char *)arg, (char *)szFilename, TRUE, TRUE ) == 0 )
                        return 0;
        }

        return 1;
}


// return the date, formatted appropriately by country type
char * FormatDate( int month, int day, int year )
{
        static char date[10];
        int i;

        // make sure year is only 2 digits
        year %= 100;

        if ( gaCountryInfo.fsDateFmt == 1 ) {

                // Europe = dd-mm-yy
                i = day;                        // swap day and month
                day = month;
                month = i;

        } else if ( gaCountryInfo.fsDateFmt == 2 ) {

                // Japan = yy-mm-dd
                i = month;                      // swap everything!
                month = year;
                year = day;
                day = i;
        }

        sprintf( date, TIME_FMT, month, gaCountryInfo.szDateSeparator[0], day, gaCountryInfo.szDateSeparator[0], year, 0 );

        return date;
}


// honk the speaker (but shorter & more pleasantly than COMMAND.COM)
void honk( void )
{
        // flush the typeahead buffer before honking
        if ( QueryIsConsole( STDIN )) {
                while ( _kbhit() )
                        (void)GetKeystroke( EDIT_NO_ECHO );
        }

        SysBeep( gpIniptr->BeepFreq, gpIniptr->BeepDur );
}


void PopupEnvironment( int fAlias )
{
        unsigned int i, n, uSize = 0;
        char *fptr;
        char * *list = 0L;

        // get the environment or alias list into an array
        fptr = (( fAlias ) ? glpAliasList : glpEnvironment );
        for ( i = 0; ( *fptr ); fptr = next_env( fptr )) {

            // allocate memory for 32 entries at a time
            if (( i % 32 ) == 0 ) {
                uSize += 128;
                list = (char * *)ReallocMem( (char *)list, uSize );
            }

            list[ i++ ] = fptr;
        }

        // get batch variables into the list
        if (( fAlias == 0 ) && ( cv.bn >= 0 )) {

            for ( n = bframe[cv.bn].Argv_Offset; ( bframe[ cv.bn ].Argv[ n ] != NULL ); n++ ) {

                if (( i % 32 ) == 0 ) {
                    uSize += 128;
                    list = (char * *)ReallocMem( (char *)list, uSize );
                }

                fptr = (char *)_alloca( strlen( bframe[ cv.bn ].Argv[ n ] ) + 6 );
                sprintf( fptr, "%%%d=%s", n - bframe[cv.bn].Argv_Offset, bframe[ cv.bn ].Argv[ n ] );
                list[ i++ ] = fptr;
            }
        }

        // no entries or no matches?
        if ( i > 0 ) {
            // display the popup selection list
            wPopSelect( gpIniptr->PWTop, gpIniptr->PWLeft, gpIniptr->PWHeight, gpIniptr->PWWidth, list, i, 1, (( fAlias ) ? "Alias list" : "Environment" ), NULL, NULL, 1 );
            FreeMem( (char *)list );
        }
}


// return a single character answer matching the input mask
int QueryInputChar( char *pszPrompt, char *pszMask )
{
        int c, handle;

        handle = STDOUT;
        // check for output redirected, but input NOT redirected
        //   (this is for things like "echo y | del /q"
        if (( QueryIsConsole( STDOUT ) == 0 ) && ( QueryIsConsole( STDIN )))
                handle = STDERR;

        qprintf( handle, "%s (%s)? ", pszPrompt, pszMask );

        for ( ; ; ) {

            // get the character - if printable, display it
            // if it's not a Y or N, backspace, beep & ask again
            c = GetKeystroke( EDIT_NO_ECHO | EDIT_UC_SHIFT );

            if ( c == EOF )
                break;

            if (( c >= 27 ) && ( c < 0xFF ) && ( c != '/' )) {

                qputc( handle, (char)c );
                if (( c == ESCAPE ) || ( strchr( pszMask, c ) != NULL )) {

                    if (( c == 'A' ) || ( c == 'V' )) {

                        unsigned int i, uSize = 0;
                        char *fptr;
                        char * *list = 0L;

                        // get the environment or alias list into an array
                        fptr = (( c == 'A' ) ? glpAliasList : glpEnvironment );
                        for ( i = 0; ( *fptr ); fptr = next_env( fptr )) {

                            // allocate memory for 32 entries at a time
                            if (( i % 32 ) == 0 ) {
                                uSize += 128;
                                list = (char * *)ReallocMem( (char *)list, uSize );
                            }

                            list[ i++ ] = fptr;
                        }

                        // no entries or no matches?
                        if ( i > 0 ) {

                            // display the popup selection list
                            wPopSelect( gpIniptr->PWTop, gpIniptr->PWLeft, gpIniptr->PWHeight, gpIniptr->PWWidth, list, i, 1, (( c == 'A' ) ? "Alias list" : "Environment" ), NULL, NULL, 1 );
                            FreeMem( (char *)list );

                            // reenable signal handling after cleanup
                            EnableSignals();
                            qputc( handle, BS );
                            continue;
                        }

                    } else
                        break;
                }

                qputc( handle, BS );
            }

            honk();
        }

        qputc( handle, '\n' );

        return c;
}


// do a case-insensitive strstr()
char * stristr( char *str1, char *str2 )
{
        int i, nLength;

        nLength = strlen( str2 );
        for ( i = 0; ( i <= ( (int)strlen( str1 ) - nLength )); i++ ) {
                if ( _strnicmp( str1 + i, str2, nLength ) == 0 )
                        return ( str1 + i );
        }

        return NULL;
}


// insert a string inside another one
char * strins( char *str, char *insert_str )
{
        unsigned int inslen;

        // length of insert string
        if (( inslen = strlen( insert_str )) > 0 ) {

                // move original
                memmove( str+inslen, str, ( strlen( str ) + 1 ));

                // insert the new string into the hole
                memmove( str, insert_str, inslen );
        }

        return ( str);
}


// return a pointer to the end of the string
char * strend( char *s )
{
        return ( s + strlen( s ));
}


// return a pointer to the end of the string
char * strlast( char *s )
{
        return (( *s != '\0' ) ? ( s + strlen( s )) - 1 : s );
}


// write a long line to STDOUT & check for screen paging
void more_page( char *start, int col )
{
        int columns, i, fConsole;

        columns = GetScrCols();

        if (( fConsole = QueryIsConsole( STDOUT )) != 0 ) {

                for ( i = 0; ( start[i] != '\0' ); ) {

                        // count up column position
                        incr_column( start[i], &col );

                        if (( col > columns ) || ( start[i++] == '\n' )) {
                                printf( FMT_FAR_PREC_STR, i, start );
                                _page_break();
                                start += i;
                                i = col = 0;
                        }
                }
        }

        printf( FMT_FAR_STR, (char *)start );
        if ( col != columns )
                crlf();

        if ( fConsole )
                _page_break();
}


// increment the column counter
void incr_column( char c, int *column )
{
        if ( c != TAB )
                (*column)++;
        else
                *column += ( 8 - ( *column & 0x07 ));
}


long GetRandom( long lStart, long lEnd )
{
        // return random value
        static unsigned long lRandom = 1L;
        DATETIME sysDateTime;

        // set seed to random value based on initial time
        if ( lRandom == 1L ) {
                QueryDateTime( &sysDateTime );
                lRandom *= (long)( sysDateTime.seconds * sysDateTime.hundredths );
        }

        lEnd++;
        lEnd -= lStart;

        lRandom = (( lRandom * 214013L ) + 2531011L );
        lRandom = ( lRandom << 16 ) | ( lRandom >> 16 );

        return (( lRandom % lEnd ) + lStart );
}


// Return a 0 if the arg == "OFF", 1 if == "ON", -1 otherwise
int OffOn( char *arg )
{
        arg = skipspace( arg );

        if ( _stricmp( arg, OFF ) == 0 )
                return 0;

        return (( _stricmp( arg, ON ) == 0 ) ? 1 : -1 );
}


// get cursor position request, adjust if relative, & check for valid range
int GetCursorRange( char *arg, int *row, int *column )
{
        int nRow, nCol, nLen;

        GetCurPos( &nRow, &nCol );

        if ( sscanf( arg, "%d %*[,] %n%d", row, &nLen, column ) == 3) {

                // if relative range get current position & adjust
                if (( *arg == '+' ) || ( *arg == '-' ))
                        *row += nRow;

                arg += nLen;
                if (( *arg == '+' ) || ( *arg == '-' ))
                        *column += nCol;

                return ( verify_row_col( *row, *column ));
        }

        return ERROR_EXIT;
}


// scan for screen colors
// returns the attribute, and removes the colors from "pszStart"
int GetColors( char *pszStart, int nBorderFlag )
{
        char *pszCurrent;
        int nFG = -1, nBG = -1, nAttribute = -1;

        pszCurrent = pszStart;
        pszCurrent = ParseColors( pszStart, &nFG, &nBG );

        // if foreground & background colors are valid, set attribute
        if (( nFG >= 0 ) && ( nBG >= 0 )) {

                nAttribute = nFG + ( nBG << 4 );

                // check for border color set
                if (( nBorderFlag ) && ( pszCurrent != NULL ) && ( _strnicmp( first_arg( pszCurrent),BORDER,3) == 0 )) {

                        char *arg;

                        // skip "BORDER"
                        arg = ntharg( pszCurrent, 1 );
                        if (( arg != NULL ) && (( nFG = color_shade( arg )) <= 7 )) {

                                // Set the border color
                                VIOOVERSCAN overscan;

                                overscan.cb = sizeof(overscan);
                                overscan.type = 1;
                                overscan.color = nFG;
                                (void)VioSetState( &overscan, 0 );
                                // skip the border color name
                                ntharg( pszCurrent, 2 );
                        }
                }

                // remove the color specs from the line
                pszCurrent = (( gpNthptr != NULL ) ? gpNthptr : NULLSTR );
                strcpy( pszStart, pszCurrent );
        }

        return nAttribute;
}


// if ANSI, send an ANSI color set sequence to the display; else, twiddle the
//   screen attributes directly
void set_colors( int attrib )
{
        // 4OS2 always forces ANSI on
        printf( "\033[0;%s%s%u;%um", (( attrib & 0x08 ) ? "1;" : NULLSTR), ((attrib & 0x80 ) ? "5;" : NULLSTR ), colors[attrib & 0x07].ansi,(colors[(attrib & 0x70 ) >> 4].ansi) + 10 );
}


// get foreground & background attributes from an ASCII string
char * ParseColors( char *line, int *nFG, int *nBG )
{
        char *arg;
        int i, nIntensity = 0, nAttrib;

        for ( ; ; ) {

                if (( arg = first_arg( line )) == NULL )
                        return NULL;

                if ( _strnicmp( arg, BRIGHT, 3 ) == 0 ) {
                        // set intensity bit
                        nIntensity |= 0x08;
                } else if ( _strnicmp( arg, BLINK, 3 ) == 0 ) {
                        // set blinking bit
                        nIntensity |= 0x80;
                } else
                        break;

                // skip BRIGHT or BLINK
                line = (( ntharg( line, 1 ) != NULL ) ? gpNthptr : NULLSTR);
        }

        // check for foreground color match
        if (( nAttrib = color_shade( arg )) <= 15)
                *nFG = nIntensity + nAttrib;

        // "ON" is optional
        i = 1;
        if ((( arg = ntharg( line, 1 )) != NULL ) && ( stricmp( arg, ON ) == 0 ))
                i++;

        // check for BRIGHT background
        if ((( arg = ntharg( line, i )) != NULL ) && ( _strnicmp( arg, BRIGHT, 3 ) == 0 )) {
                nIntensity = 0x08;
                i++;
        } else
                nIntensity = 0;

        // check for background color match
        if (( nAttrib = color_shade( ntharg( line, i ))) <= 15 ) {
                *nBG = nAttrib + nIntensity;
                ntharg( line, ++i );
        }

        return gpNthptr;
}


// match color against list
int color_shade( char *arg )
{
        int i;

        if ( arg != NULL ) {

                // allow 0-15 as well as Blue, Green, etc.
                if ( is_signed_digit( *arg ))
                        return ( atoi( arg ));

                for ( i = 0; ( i <= 7 ); i++ ) {
                        // check for color match
                        if ( _strnicmp( arg, colors[i].shade, 3 ) == 0 )
                                return i;
                }
        }

        return 0xFF;
}


// read / write the description file(s)
int process_descriptions( char *inname, char *outname, int flags )
{
        int i;
        char *dname_part;
        int hFH, rval = 0;
        unsigned int bytes_read, bytes_written, fLFN;
        unsigned int nMode = (O_RDWR | O_BINARY), fTruncate = 0;
        char *arg, szDName[MAXFILENAME], *new_description = NULLSTR;
        char *dptr = 0L, *pchRead = 0L, *pchWrite = 0L;
        char *fptr, *fdesc;
        long lReadOffset, lWriteOffset;
        unsigned int attrib = 0;

        // check if no description processing requested
        if (( gpIniptr->Descriptions == 0 ) && (( flags & DESCRIPTION_PROCESS ) == 0 ))
                return 0;
        flags &= ~DESCRIPTION_PROCESS;

        // check DescriptionName for EA storage
        if ( stricmp( "ea", DESCRIPTION_FILE ) == 0 ) {

                char szDescription[512];
                int n;

                szDescription[0] = '\0';
                if ( flags & DESCRIPTION_READ ) {
                        n = 511;
                        EAReadASCII( inname, SUBJECT_EA, szDescription, &n );
                        if (( flags & DESCRIPTION_WRITE ) == 0 )
                                strcpy( outname, szDescription );
                }

                // if we're writing a new description, it's in the
                //   format "process_description(outname,description,flags)"
                if ( flags & DESCRIPTION_CREATE ) {
                        strcpy( szDescription, outname );
                        outname = inname;
                }

                if ( flags & DESCRIPTION_WRITE ) {
                        if ( EAWriteASCII( outname, SUBJECT_EA, szDescription ) == 0 )
                                return ERROR_EXIT;
                }

                return 0;
        }

        // disable ^C / ^BREAK handling
        HoldSignals();

        // read the descriptions ( from path if necessary)
        if (( inname != NULL ) && ( flags & DESCRIPTION_READ )) {

            // read 4K blocks
            bytes_read = 4098;
            if (( pchRead = AllocMem( &bytes_read )) == 0L )
                return ERROR_EXIT;

            insert_path( szDName, DESCRIPTION_FILE, inname );

            // it's not an error to not have a DESCRIPT.ION file!
            if ((hFH = _sopen( szDName, (O_RDONLY | O_BINARY), SH_DENYWR )) >= 0 ) {

                dname_part = fname_part( inname );

                // read 4K blocks of the DESCRIPT.ION file
                for ( lReadOffset = 0L; (( FileRead( hFH, pchRead, 4096, &bytes_read ) == 0 ) && (bytes_read != 0 )); ) {

                        // back up to the end of the last line & terminate there
                        if (( i = bytes_read ) == 4096 ) {
                                for ( ; ((--i > 0 ) && ( pchRead[i] != '\n' ) && ( pchRead[i] != '\r' )); )
                                        ;
                                i++;
                        }
                        pchRead[i] = '\0';
                        lReadOffset += i;

                        // read a line & try for a match
                        for ( fptr = pchRead; (( *fptr != '\0' ) && ( *fptr != EoF )); ) {

                                // check for LFNs & strip quotes
                                if ( *fptr == '"' ) {

                                    if (( fdesc = strchr( ++fptr, '"' )) != 0L )
                                        *fdesc++ = '\0';

                                } else {

                                    // skip to the description part (kinda kludgy
                                    // to avoid problems if no description
                                    for ( fdesc = fptr; ; fdesc++ ) {

                                        if (( *fdesc == ' ' ) || ( *fdesc == ',' ) || ( *fdesc == '\t' ))
                                                break;
                                        if (( *fdesc == '\r' ) || ( *fdesc == '\n' ) || ( *fdesc == '\0' )) {
                                                fdesc = 0L;
                                                break;
                                        }
                                    }
                                }

                                if ( fdesc != 0L ) {

                                    // wipe out space between name & description
                                    *fdesc++ = '\0';

                                    if ( stricmp( dname_part, fptr ) == 0 ) {

                                        // just return the matching description?
                                        if (( flags & DESCRIPTION_WRITE ) == 0 )
                                                sscanf( fdesc, DESCRIPTION_SCAN, outname );

                                        dptr = fdesc;
                                        break;
                                    }

                                    fptr = fdesc;
                                }

                                // skip the description & get next filename
                                for ( ; (( *fptr != '\0' ) && ( *fptr++ != '\n' )); )
                                        ;
                        }

                        if (( dptr != 0L ) || ( bytes_read < 4096 ))
                                break;

                        // seek to the end of the last line of the current block
                        _lseek( hFH, lReadOffset, SEEK_SET );
                }

                _close( hFH );
            }
        }

        if (( flags & DESCRIPTION_WRITE ) || ( flags & DESCRIPTION_REMOVE )) {

            // if we're writing a new description, it's in the
            //   format "process_description(outname,description,flags)"
            if ( flags & DESCRIPTION_CREATE ) {

                if ( dptr != 0L ) {
                        // save extended part of old description
                        sscanf( dptr, "%*[^\004\032\r\n]%n", &bytes_read );
                        dptr += bytes_read;
                        new_description = outname;
                } else
                        dptr = (char *)outname;

                outname = inname;
            }

            // if no description, and we're not removing descriptions, exit
            if ( dptr == 0L ) {
                if (( flags & DESCRIPTION_REMOVE ) == 0 )
                    goto descript_bye;
            } else {
                // if we're adding a description, we may need to create a file
                nMode |= O_CREAT;
            }

            bytes_read = 4098;
            if (( pchWrite = AllocMem( &bytes_read )) == 0L ) {
                rval = ERROR_EXIT;
                goto descript_bye;
            }

            // open the target DESCRIPT.ION file
            insert_path( szDName, DESCRIPTION_FILE, outname );

            if (( hFH = _sopen( szDName, nMode, SH_DENYRW, (S_IREAD | S_IWRITE))) < 0 )
                rval = _doserrno;

            else {

                fLFN = ifs_type( szDName );
                lReadOffset = lWriteOffset = 0L;

                // point to the name part
                dname_part = szDName + ((( dname_part = path_part( szDName )) != NULL ) ? strlen( dname_part ) : 0 );

                // read 4k blocks of the DESCRIPT.ION file
                while (( FileRead( hFH, pchWrite, 4096, &bytes_read ) == 0 ) && (bytes_read != 0 )) {

                        // back up to the end of the last line, and seek
                        //   to the end of the last line of the current block
                        for ( i = bytes_read; (( --i > 0 ) && ( pchWrite[i] != '\n' )); )
                                ;

                        pchWrite[++i] = '\0';
                        lReadOffset += i;

                        // read a line & try for a match
                        for ( fptr = pchWrite; (( *fptr != '\0' ) && ( *fptr != EoF )); ) {

                                // look for argument match or file not found
                                // if not found, delete this description
                                // check for LFNs
                                if ( *fptr == '"' )
                                        sscanf( fptr, "\"%[^\"\n]%*[^\n]\n%n", dname_part, &nMode );
                                else
                                        sscanf( fptr, "%[^ ,\t\n]%*c%*[^\n]\n%n", dname_part, &nMode );

                                // don't let non-LFN OS kill LFN descriptions
                                if (( fLFN == 0 ) && (( *fptr == '"' ) || ( strlen( dname_part ) > 12 ))) {

                                        // point to beginning of next line
                                        fptr += nMode;

                                } else if (( _stricmp( outname, szDName ) == 0 ) || (( flags & DESCRIPTION_REMOVE ) && ( QueryFileMode( szDName, &attrib ) == ERROR_FILE_NOT_FOUND ))) {
                                        fTruncate = 1;

                                        // collapse matching or missing filename
                                        strcpy( fptr, fptr + nMode );

                                } else
                                        // point to beginning of next line
                                        fptr += nMode;
                        }

                        if ( fTruncate ) {

                                if (( i = strlen( pchWrite )) > 0 ) {

                                        _lseek( hFH, lWriteOffset, SEEK_SET );
                                        (void)FileWrite( hFH, pchWrite, i, (unsigned int *)&bytes_written );

                                        // save current write position &
                                        //   restore read position
                                        lWriteOffset += bytes_written;
                                }

                        } else
                                lWriteOffset = lReadOffset;

                        if ( bytes_read < 4096 )
                                break;

                        _lseek( hFH, lReadOffset, SEEK_SET );
                }

                // if truncating, or we have a description, write it out
                if (( fTruncate ) || ( dptr != 0L )) {

                        _lseek( hFH, lWriteOffset, SEEK_SET );

                        // truncate the file
                        if ( fTruncate )
                                (void)_chsize( hFH, lWriteOffset );

                        // add the new description (if any) to the list
                        if ( dptr != 0L ) {

                                // get description length
                                for ( i = 0; (( dptr[i] ) && ( dptr[i] != '\r' ) && ( dptr[i] != '\n' )); i++ )
                                        ;

                                // format is: filename description[cr][lf]
                                if (( i ) || ( *new_description )) {

                                    // check for LFN/HPFS/NTFS name
                                    if (( arg = fname_part( outname )) != NULL ) {

                                        if ( QueryIsLFN( arg ))
                                            qprintf( hFH, "\"%s\" %s%.*Fs\r\n", arg, new_description, i, dptr );
                                        else
                                            qprintf( hFH, "%s %s%.*Fs\r\n", arg, new_description, i, dptr );
                                    }

                                    // make sure we don't delete the list
                                    lWriteOffset = 1L;
                                }
                        }

                        _close( hFH );

                        // restore description file name
                        strcpy( dname_part, DESCRIPTION_FILE );

                        // if filesize == 0, delete the DESCRIPT.ION file
                        if ( lWriteOffset == 0L )
                                remove( szDName );

                        else if ( lReadOffset == 0 ) {

                                // make file hidden & set archive (for BACKUP)
                                // but only if we just created it!
                                (void)SetFileMode( szDName, (_A_HIDDEN | _A_ARCH) );
                        }

                } else
                        _close( hFH );
            }
        }

descript_bye:

        FreeMem( pchRead );
        FreeMem( pchWrite );

        // enable ^C / ^BREAK handling
        EnableSignals();

        return rval;
}


// clear the screen (using ANSI)
void clear_screen(void)
{
        VIOOVERSCAN overscan;

        if ( gpIniptr->StdColor != 0 ) {

                set_colors( gpIniptr->StdColor & 0xFF );

                // check for default border (high byte)
                if (( overscan.color = ( gpIniptr->StdColor >> 8 )) != 0xFF ) {
                        overscan.cb = sizeof( overscan );
                        overscan.type = 1;
                        (void)VioSetState( &overscan, 0 );
                }
        }

        // 4OS2 forces ANSI compatibility, so we _know_ this will work!
        printf( "\033[2J" );

}

char *show_atts( int attribute )
{
        static char atts[6];

        strcpy( atts, "_____" );
        if ( attribute & _A_RDONLY )
                atts[0] = 'R';
        if ( attribute & _A_HIDDEN )
                atts[1] = 'H';
        if ( attribute & _A_SYSTEM )
                atts[2] = 'S';
        if ( attribute & _A_ARCH )
                atts[3] = 'A';
        if ( attribute & _A_SUBDIR )
                atts[4] = 'D';

        return atts;
}

// read a disk label
int getlabel( char *arg )
{
    char volume_name[MAXFILENAME];
    unsigned long serial_number = 0L;

    // first check for a valid drive name
    if ( arg[1] != ':' )
        return ( error( ERROR_INVALID_DRIVE, arg ));

    // read the label
    if ( QueryVolumeInfo( arg, volume_name, &serial_number ) == NULL )
        return ERROR_EXIT;

    printf( VOLUME_LABEL, _ctoupper( *arg ), volume_name );

    // display the disk serial number
    if ( serial_number )
        printf( VOLUME_SERIAL, ( serial_number >> 16 ), ( serial_number & 0xFFFF ));

    return 0;
}
