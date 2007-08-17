/*
 * $Source: /netlabs.cvs/osfree/src/cmd/xcopy/verbose.c,v $
 * $Revision: 1.1 $
 * $Date: 2003/10/20 13:59:43 $
 * $Author: prokushev $
 *
 * Output error/warning/verbose strings to stream.
 *
 * $Log: verbose.c,v $
 * Revision 1.1  2003/10/20 13:59:43  prokushev
 * + XCOPY
 * * clean target
 *
 * Revision 1.8  1997/01/22 00:41:46  vitus
 * Loglevel now unsigned
 * References verbose.h, not overbose.h (?)
 *
 * Revision 1.7  1996/09/10 00:36:58  vitus
 * Changed to HFILE usage
 *
 * Revision 1.6  1996/05/12 01:21:55  vitus
 * Changed timer to 24hour display
 *
 * Revision 1.5  1996/05/11 00:56:35  vitus
 * Added support of GNU C compiler
 *
 * Revision 1.4  1996/03/23 23:42:29  vitus
 * Moved newline to end of format string
 *
 * Revision 1.3  1996/03/13 23:35:48  vitus
 * Changed to binkley-style output
 *
 * Revision 1.2  1996/03/13 21:09:11  vitus
 * Removed initialisation of fpLogfile if IBM CSet++
 *
 * Revision 1.1  1996/01/27 02:53:24  vitus
 * Initial revision
 *
 */
static char vcid[]="$Id: verbose.c,v 1.1 2003/10/20 13:59:43 prokushev Exp $";


#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCL_DOS
#include <os2.h>

#include "verbose.h"



static HFILE	hLogfile = 2;			/* stderr? */
static unsigned	iLoglevel = 1;			/* only fatal errors */

static char const achLevel[] = "!*+:# ";



void
Verbose(unsigned level,char *fmt,...)
{
    APIRET rc;

    if( level <= iLoglevel )
    {
	va_list	argp;
	char	*str = malloc(strlen(fmt)+200);
	char	*buf = malloc(LOGBUFSIZ);
	ULONG	written;
	time_t	now = 0;
	struct tm * timep;

	if( level >= strlen(achLevel) )
	    str[0] = ' ';
	else
	    str[0] = achLevel[level];

	time( &now );
	timep = localtime( &now );
	strftime( &str[1], sizeof(str)-30, " %d %b %H:%M:%S ", timep );

	strcat( str, fmt );			/* append format string */
	strcat( str, "\r\n" );			/* append newline */

	va_start(argp,fmt);
	vsprintf( buf, str, argp );
	rc = DosWrite( hLogfile, buf, strlen(buf), &written );
	va_end(argp);

	free( buf );
	free( str );
    }
}




HFILE
SetLogfile(HFILE new)
{
    HFILE old = hLogfile;
    hLogfile = new;
    return old;
}




unsigned
SetLoglevel(unsigned new)
{
    unsigned	old = iLoglevel;

    iLoglevel = new;
    return old;
}


