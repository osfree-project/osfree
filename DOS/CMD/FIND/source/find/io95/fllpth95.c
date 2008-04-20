/* $RCSfile: FLLPTH95.C $
   $Locker: ska $	$Name:  $	$State: Exp $

   	char *fullpath(char *buffer, char *path, int buflen)

   	Fully-qualify path.
   	If buffer != NULL, buflen tells its size.
   	If buffer == NULL, a buffer of buflen bytes is malloc'ed.

   	This code requires to define "maximum" values for the filesystem:
   		MAXPATH95		260
   		MAXNAME95		255
	Unfortunately, these values are assumptions for Win95 & DOS only!

	If the path starts with "\\\\" both backslashes remain; that
	gives some support for UNC path spec.

   $Log: FLLPTH95.C $
   Revision 1.2  2000/01/11 09:34:30  ska
   add: support Turbo C v2.01

   Revision 1.1  2000/01/11 09:10:08  ska
   Auto Check-in

*/

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <dir.h>

#include "io95.h"

#define MAXDRVLEN 2

#ifndef lint
static char const rcsid[] = 
	"$Id: FLLPTH95.C 1.2 2000/01/11 09:34:30 ska Exp ska $";
#endif


/*
 * fullpath
 */

char *fullpath95(char * const Xbuffer, const char *path, int Xbuflen)
{
	int drive, buflen;
	char *p;	/* points to place, where to start a new path component */
	char *buffer;	/* buffer to be work over */
	char *lowbound;	/* boundary never to undergo */
	const char *pp;

	assert(path);

	if((buflen = Xbuflen) <= 0)
		buflen = MAXPATH95;
	if((buffer = Xbuffer) == NULL && (buffer = malloc(buflen)) == NULL)
		return NULL;

	if(*path == '\\' && path[1] == '\\') {	/* assume an UNC path */
		/* two shares must be present! */
		if(*(pp = &path[2]) == '\\' || *pp == '/')
			goto errRet;
		while(*pp && *pp != '\\' && *pp != '/') ++pp;
		if(!*pp) goto errRet;			/* at least two shares */
		while(*pp && *pp != '\\' && *pp != '/') ++pp;
		if(buflen <= (drive = pp - path))
			goto errRet;
		memcpy(buffer, path, drive);
		path = pp;
		lowbound = p = buffer + drive;
	}
	else {
		lowbound = buffer + 2;
		if(path[1] == ':') {
			if(!isalpha(*path))
				goto errRet;		/* invalid file spec */
			drive = toupper(path[0]) - 'A';
			path += 2;
		}
		else
			drive = getdisk();		/* current drive */

		if(*path == '\\' || *path == '/') {
			/* fully-qualified path needn't getcwd */
			*buffer = drive + 'A';
			buffer[1] = ':';
			*(p = &buffer[2]) = '\0';
		}
		else if(!getdcwd95(drive + 1, buffer, buflen))	/* get working directory failed */
			goto errRet;
		else if((p = strchr(buffer, '\0'))[-1] == '\\')
			--p;
	}

   /*
	Now copy all remaining components from path to buffer
   */

	do {
		/* first chop path delimiter characters */
		while(*path == '/' || *path == '\\') ++path;
		/* second break, if path has been used up */
		if(!*path) break;
		/* third identify '.' and '..' */
		if(*path == '.') switch(path[1]) {
			case '/': case '\\': case '\0':
				++path;		/* skip '.' */
				continue;
			case '.':		/* probably: '..' */
				switch(path[2]) {
				case '/': case '\\': case '\0':
					path += 2;		/* skip '..' */
					/* chop last path component */
					while(--p > lowbound && *p != '/' && *p != '\\');
					continue;
				}
				break;
		}
		/* forth copy this path component */
		*p++ = '\\';
		while(*path && *path != '/' && *path != '\\')
			if(p >= buffer + buflen)
				goto errRet;		/* buffer overflow */
			else *p++ = *path++;
	} while(1);

	if(p == lowbound)		/* root drive */
		*p++ = '\\';
	*p = '\0';

	if(!Xbuffer && Xbuflen <= 0
	 && (p = realloc(buffer, strlen(buffer) + 1)) != NULL)
	 	return p;

	return buffer;

errRet:
	if(!Xbuffer)
		free(buffer);

	return NULL;
}
