/*    
	mfs_regexp.c	1.13
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*	mfs file comparison function using regexp style compare

 */

#include "platform.h"

#include <stdio.h>
#include <string.h>

#include "windows.h"
#include "mfs_config.h"
#include "mfs_core.h"

static int charicmp(char, char);

/************************************************************************/
static int 
charicmp (
        char                    char1,
        char                    char2 )
 
{
 
        char                    Char1 = ( 'a' <= char1 ) && ( char1 <= 'z' ) ?
                                        char1 - 'a' + 'A' : char1;
        char                    Char2 = ( 'a' <= char2 ) && ( char2 <= 'z' ) ?
                                        char2 - 'a' + 'A' : char2;
 
        return ( Char2 - Char1 );
 
}

/************************************************************************/
DWORD
mfs_regexp(DWORD lpFileName, DWORD lpFilter, DWORD p3, DWORD p4)
{
	/*   The following code is provided by Tarang and I trust him...
	*/
        LPSTR                   lpTempFileName = (LPSTR)lpFileName;
        LPSTR                   lpTempFilter   = (LPSTR)lpFilter;
        char                    TempToken [ 2 ];
	char			TempFilter [ 2 ];
        BOOL                    Matched = FALSE;

        if ( ( ! (LPSTR)lpFileName ) || ( ! *(LPSTR)lpFileName ) ||
             ( ! (LPSTR)lpFilter ) || ( ! *(LPSTR)lpFilter ) )
                return 0L;

	if ( ! lstrcmp ( ( LPSTR )lpFilter, "*.*" ) )
	{
		wsprintf ( TempFilter, "*" );
		lpTempFilter = TempFilter;
		lpFilter     = ( DWORD )TempFilter;
	}

        while ( ( lpTempFilter ) && ( *lpTempFilter ) && ( ! Matched ) )
        {
                memset ( TempToken, 0, sizeof ( TempToken ) );
                switch ( *lpTempFilter )
                {
                        default:
                                if ( charicmp ( *lpTempFileName, *lpTempFilter ) )
                                {
                                        lpTempFileName = (LPSTR)lpFileName;
                                        lpTempFilter = strpbrk ( lpTempFilter, " ,;" );
                                        if ( lpTempFilter )
                                                lpTempFilter++;
                                }
                                else
                                {
                                        lpTempFilter++;
                                        lpTempFileName++;
                                        switch ( *lpTempFilter )
                                        {
                                                default:
                                                        break;

                                                case '\0':
                                                case ' ':
                                                case ',':
                                                case ';':
                                                        if ( ! *lpTempFileName )
                                                                Matched = TRUE;
                                                        break;
                                        }
                                }
                                break;

                        case '?':
                                lpTempFilter++;
                                lpTempFileName++;
                                break;

                        case '*':
                                if ( ! ( TempToken [ 0 ] = *( ++lpTempFilter ) ) )
                                        Matched = TRUE;
                                else
                                {
                                        lpTempFilter++;
                                        while ( ( lpTempFileName = strpbrk ( lpTempFileName, TempToken ) ) &&
                                                ( ! Matched ) )
                                                Matched = mfs_regexp ( (DWORD)++lpTempFileName, (DWORD)lpTempFilter, 0L, 0L );
                                        if ( ( ! lpTempFileName ) && ( ! Matched ) )
                                        {
                                                lpTempFileName = (LPSTR)lpFileName;
                                                lpTempFilter = strpbrk ( lpTempFilter, " ,;" );
                                                if ( lpTempFilter )
                                                        lpTempFilter++;
                                        }
                                }
                                break;

                        case '\0':
                        case ' ':
                        case ',':
                        case ';':
                                Matched = TRUE;
                                break;
                }
        }

        return (DWORD)Matched;

#ifdef OLDSTUFF
    char	*s;       /* p1 */
    char        *pattern; /* p2 */
    int 	 last;
    int 	 matched;
    int 	 reverse;
    char         pp[128], *p;

    s = (char *)p1;
    pattern = (char *)p2;

    strcpy(pp, pattern);
    p = (char *) strtok(pp, " ,;");
    while ( p ) {
    	for ( ; *p; s++, p++)
	    switch (*p) {
	    case '\\':
		/* Literal match with following character; fall through. */
		p++;
	    default:
		if ((toupper(*s) != *p) && (tolower(*s) != *p))
		    return(FALSE);
		continue;

	    case '?':
		/* Match anything. */
		if (*s == '\0')
		    return(FALSE);
		continue;

	    case '*':
		/* Trailing star matches everything. */
		return(*++p ? Star(s, p) : TRUE);

	    case '[':
		/* [^....] means inverse character class. */
		if ((reverse = p[1]) == '^')
		    p++;
		for (last = 0400, matched = FALSE; *++p && *p != ']'; last = *p)
		    /* This next line requires a good C compiler. */
		    if (*p == '-' ? *s <= *++p && *s >= last : *s == *p)
			matched = TRUE;
		if (matched == reverse)
		    return(FALSE);
		continue;
	    }
	if ( *s == '\0'  ||  *s == '/' )
		return TRUE;
	p = (char *) strtok(NULL, " ,;");
    }
    return FALSE;
#endif
}


