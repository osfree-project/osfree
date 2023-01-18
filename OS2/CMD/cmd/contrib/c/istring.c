// ISTRING.C - INI file string insertion routine for 4xxx / TCMD
//   Copyright 1992 - 1997, JP Software Inc., All Rights Reserved

#include <string.h>
#include <stdio.h>
#include <io.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <share.h> 

#include "product.h"

#include "4all.h"

#include "inifile.h"

// store a string; remove any previous string for the same item
int ini_string(INIFILE *InitData, int *dataptr, char *string, int slen)
{
    unsigned int i;
    int old_len, ptype, move_cnt;
    unsigned int offset;
    unsigned int *fixptr;
    char *old_string;

    // calculate length of previous string, change in string space
    old_len = ((offset = (unsigned int)*dataptr) == INI_EMPTYSTR) ? 0 : (strlen((old_string = InitData->StrData + offset)) + 1);

    // holler if no room
    if ((InitData->StrUsed + slen + 1 - old_len) > InitData->StrMax)
        return 1;

    // if there is an old string in the middle of the string space, collapse
    // it out of the way and adjust all pointers
    if (offset != INI_EMPTYSTR) {

        if ((move_cnt = InitData->StrUsed - (offset + old_len)) > 0) {

            memmove(old_string, old_string + old_len, move_cnt);
            for (i = 0; (i < guINIItemCount); i++) {

                fixptr = (unsigned int *)((char *)InitData + ((char *)gaINIItemList[i].pItemData - (char *)&gaInifile));
                ptype = (int)(INI_PTMASK & gaINIItemList[i].cParseType);

                if (((ptype == INI_STR) || (ptype == INI_PATH)) && (*fixptr != INI_EMPTYSTR) && (*fixptr > offset))
                    *fixptr -= old_len;
            }
        }

        InitData->StrUsed -= old_len;
    }

    // put new string in place and adjust space in use
    if (slen > 0) {
        memmove(InitData->StrData + InitData->StrUsed, string, slen + 1);
        *dataptr = InitData->StrUsed;
        InitData->StrUsed += (slen + 1);
    } else
        *dataptr = INI_EMPTYSTR;

    return 0;
}
#undef _lseek
// get command line from console or file
int getline2( int fd, char *line, int nMaxSize )
{
    int i;

    // get a line and set the file pointer to the next line
    nMaxSize = read( fd, line, nMaxSize );

    for ( i = 0; ; i++, line++ ) {

        if ( ( i >= nMaxSize ) || ( *line == 26 ) )
            break;

        // skip a LF following a CR or LF
        if ( ( *line == '\r' ) || ( *line == '\n' ) ) {
            if ( ( ++i < nMaxSize ) && ( line[1] == '\n' ) )
                i++;
            break;
        }
    }

    // truncate the line
    *line = '\0';

    if ( i >= 0 )
        (void)_lseek( fd, (long)( i - nMaxSize ), SEEK_CUR );

    return i;
}

// strip the specified leading characters
void StripLeading( char *arg, char *delims )
{
    while ( ( *arg != '\0' ) && ( strchr( delims, *arg ) != NULL ) )
        strcpy( arg, arg+1 );
}

// strip the specified trailing characters
void StripTrailing(char *arg, char *delims)
{
    int i;

    for ( i = strlen( arg ); ((--i >= 0 ) && ( strchr( delims, arg[i] ) != NULL )); )
        arg[i] = '\0';
}

// make up for the lack of OS/2 APIs!
// write a string to the .INI file
void TCWritePrivateProfileStr( char *pszSection, char *pszItem, char *pszBuffer )
{
    int i;
    int fd, nLength;
    unsigned int uSize;
    long lOffset = 0L;
    char szBuffer[1024], szOutput[1024], *pchTail;
    static char szIniName[260];
    extern char *_LpPgmName;

    sprintf( szIniName, "%s4OS2.INI", path_part( _LpPgmName ));

    if ( pszBuffer == NULL )
        pszBuffer = "";
    
    // open the INI file
    if ( ( fd = sopen( szIniName, (O_RDWR | O_BINARY | O_CREAT), SH_DENYWR, (S_IREAD | S_IWRITE) )) <= 0 )
        return;

    if ( pszItem != NULL )
        nLength = strlen( pszItem );

    while ( getline2( fd, szBuffer, 1023 ) > 0 ) {

        StripLeading( szBuffer, " \t[" );
        StripTrailing( szBuffer, " \t]" );
        if ( stricmp( szBuffer, pszSection ) == 0 ) {

            for ( ; ; ) {

                if ( ( pszItem != NULL ) && ( szBuffer[0] ) )
                    lOffset = _lseek( fd, 0L, SEEK_CUR );
                szBuffer[0] = '\0';
                i = getline2( fd, szBuffer, 1023 );

                StripLeading( szBuffer, " \t" );
                if ( ( i <= 0 ) || ( szBuffer[0] == '[' ) || (( pszItem != NULL ) && ( strnicmp( szBuffer, pszItem, nLength ) == 0 ) && ( szBuffer[ nLength ] == '=' )) ) {

                    // save remainder of file
                    uSize = 32768;
                    pchTail = AllocMem( &uSize );
                    nLength = read( fd, pchTail, uSize );
                    _lseek( fd, lOffset, SEEK_SET );
                    
                    if ( pszItem != NULL ) {
                        // write new line
                        if (!strpbrk(szBuffer, "\r\n"))
                            sprintf( szOutput, "\r\n%s=%s\r\n", pszItem, pszBuffer );
                        else
                            sprintf( szOutput, "%s=%s\r\n", pszItem, pszBuffer );
                        write( fd, szOutput, strlen(szOutput));
                    }

                    // if entry didn't exist, it was just
                    //   inserted & we need to move the
                    //   next section down
                    if ( szBuffer[0] == '[' ) {
                        sprintf( szOutput, "\r\n%s\r\n", szBuffer );
                        write( fd, szOutput, strlen(szOutput));
                    }

                    // append tail
                    if ( nLength > 0 )
                        write( fd, pchTail, nLength );

                    // truncate file
                    lOffset = _lseek( fd, 0L, SEEK_CUR );
                    (void)chsize( fd, lOffset );
                    FreeMem( pchTail );
                    goto TCWBye;
                }
            }
        }

        if ( pszItem == NULL )
            lOffset = _lseek( fd, 0L, SEEK_CUR );
    }

    // write new section
    if ( pszItem != NULL ) {
        sprintf( szOutput, "[%s]\r\n%s=%s\r\n", pszSection, pszItem, pszBuffer );
        write( fd, szOutput, strlen(szOutput));
    }
    TCWBye:
    close( fd );
}

