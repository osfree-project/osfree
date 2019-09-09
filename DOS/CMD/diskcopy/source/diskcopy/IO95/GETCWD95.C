/* $RCSfile: GETCWD95.C $
   $Locker: ska $	$Name:  $	$State: Exp $

   	getdcwd/getcwd/getcurrdir

   	This code requires to define "maximum" values for the filesystem:
   		MAXPATH95		260
   		MAXNAME95		255
	Unfortunately, these values are assumptions for Win95 & DOS only!

   $Log: GETCWD95.C $
   Revision 1.2  2000/01/11 09:34:33  ska
   add: support Turbo C v2.01

   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dos.h>
#include <dir.h>

#ifdef USE_IO95
#undef USE_IO95
#endif

#include "dir95.h"
#include "io95.h"
#include "io95_loc.h"

#ifndef lint
static char const rcsid[] = 
	"$Id: GETCWD95.C 1.2 2000/01/11 09:34:33 ska Exp ska $";
#endif


/*
	Get current working directory without drive/colon/leading backslash

	Return:
		0: success
		-1: error
*/
int getcurdir95(int drive, char * const dir)
{	struct REGPACK rp;

	assert(dir);
	rp.r_ax = 0x7147;
	rp.r_si = FP_OFF(dir);
	rp.r_ds = FP_SEG(dir);
	rp.r_dx = drive;

	if(callWin95(0x47, &rp)) {
		errno = rp.r_ax;
		return -1;
	}

	return 0;
}

char * getdcwd95(int drive, char *buf, int buflen)
{	
	char hbuf[MAXPATH95 * 2];

	if(getcurdir95(drive, hbuf))
		return NULL;

	/* Construct the fully-qualified path */
	if(!buflen)
		buflen = strlen(hbuf) + 1;
	else if(buflen < strlen(hbuf) + 4) {
		errno = ERANGE;
		return NULL;
	}
	if(!buf && (buf = malloc(buflen)) == NULL)
		return NULL;
	if((*buf = 'A' - 1 + drive) == 'A' - 1)
		*buf = 'A' + getdisk();
	buf[1] = ':';

	if(*hbuf == '\\')
		strcpy(&buf[2], hbuf);
	else {
		buf[2] = '\\';
		strcpy(&buf[3], hbuf);
	}

	return buf;
}

char * getcwd95(char *buf, int buflen)
{	return getdcwd95(0, buf, buflen);	}
