/* $RCSfile: FOPEN95.C $
   $Locker: ska $	$Name:  $	$State: Exp $

	FILE *fopen(const char fnam[], const char mode[])

	Open the file "fnam" in the mode "mode" supporting LFN of Win95.
	If Win95 is not running, DOS 4 functionality is used.

	Mode supports: r, w, a, +, t,& b

	Note: This function does NOT support DOS prior version 4!

   $Log: FOPEN95.C $
   Revision 1.2  2000/01/11 09:39:33  ska
   add: STDIO95.H

   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#ifndef HAVE_FDOPEN
#include <io.h>
#endif

#ifdef USE_IO95
#undef USE_IO95
#endif
#include "stdio95.h"

#ifndef HAVE_FDOPEN
FILE *fdopen(int fd, const char mode[])
{	FILE *f;

	if((f = fopen("NUL", mode)) == NULL)
		return NULL;

	if(dup2(fd, fileno(f)) == 0)
		return f;

	fclose(f);
	return NULL;
}
#undef HAVE_FDOPEN
#define HAVE_FDOPEN fdopen
#endif

FILE *fopen95(const char fnam[], const char mode[])
{	FILE *h;
	int fd;
	int omode, amode;
	const char *p;

	assert(fnam);
	assert(mode);

	if((p = strpbrk(mode, "rwa")) == NULL)
		return NULL;		/* no open mode */

	switch(*p) {
	case 'r': amode = O95_RDONLY; omode = 0; break;
	case 'w': amode = O95_WRONLY; omode = O95_CREAT | O95_TRUNC; break;
	default: amode = O95_WRONLY; omode = O95_CREAT | O95_APPEND; break;
	}

	omode |= strchr(mode, '+')? O95_RDWR: amode;

	if((p = strpbrk(mode, "tb")) != NULL)
		omode |= *p == 't'? O95_TEXT: O95_BINARY;

	if((fd = open95(fnam, omode, S95_IREAD | S95_IWRITE)) == -1)
		return NULL;

	h = HAVE_FDOPEN(fd, (char*)mode);
	if(!h)
		close(fd);

	return h;
}
