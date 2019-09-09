/* $Id: DIR95.H 1.1 2000/01/11 09:10:09 ska Exp ska $
   $Locker: ska $	$Name:  $	$State: Exp $

	Declaration for the POSIX.1 opendir() family supporting
	Win95 LFN API (in addition to the standard DOS API)

   $Log: DIR95.H $
   Revision 1.1  2000/01/11 09:10:09  ska
   Auto Check-in

*/

#ifndef _DIR95_h_
#define _DIR95_h_

#include <stdio.h>
#include "io95.h"
#include "find95.h"


struct dirent95 {

/*   long              d_ino;			*/
/*   unsigned short    d_reclen;		*/
/*   unsigned short    d_namlen;		*/

   char              d_name[MAXNAME95+1];
};

typedef struct {
	long	dd_cnt;		/* for seekdir()/telldir() */
	short  dd_stat;      /* status return from last lookup          */
	FF_Block95   dd_dta; 	/* disk transfer area for this dir.        */
	struct dirent95 dd_de;	/* will be returned by readdir() */
	char	dd_dir[1];		/* fully-qualifed name of directory */
   		/* this member is dynamically extended that the name fits in
   			there.
   			Note: The DTA, which is active during the findfirst/findnext
   			calls, is probably smaller than the "normal" size of 128 bytes.
   			This should not cause any problems, as I see more than one
   			implementations assuming this, that causes no crash.
   		*/
} DIR95;

extern DIR95 *opendir95(const char * const);
extern struct dirent95 *readdir95(DIR95 * const);
extern long telldir95(DIR95 * const);
extern void seekdir95(DIR95 * const, long);
extern void closedir95(DIR95 * const);

#define rewinddir95(dirp)   seekdir95(dirp,0L)

int getcurdir95(int drive, char * const dir);
/*	Retrieve current directory of drive.

	drive: 0 = current drive; 1 = A:; 2 = B:
	dir must point to a buffer large enough to recieve the path, one
	should allocate at least MAXPATH95 bytes.

	Return:
		-1: failure
		0: success; *dir filled without drive/colon and leading backslash
*/

char * getdcwd95(int drive, char *buf, int buflen);
/*	Get current working directory of drive.

	drive: 0 = current drive; 1 = A:; 2 = B:

	If buf == NULL, buflen bytes will be allocated.
	If buf == NULL && buflen == 0, a buffer will be allocated, that is
	as small as the directory string will fit in.
	If buf != NULL && buflen less than the length of the directory
	string, a failure occurs and 'errno' is set to ERANGE.

	The allocated buffer should be free()'ed by the programmer.

	Return:
		== NULL: failure
		!= NULL: buf or allocated buffer: success
*/

char * getcwd95(char *buf, int buflen);
/* Return the current working directory of the current drive.

	Equal to:	getdcwd95(0,buf,buflen)
*/

/* Always use IO95 functions */
#ifdef USE_IO95
#define MAXNAME MAXNAME95
#define MAXPATH MAXPATH95
#define dirent dirent95
#define DIR DIR95
#define opendir opendir95
#define readdir readdir95
#define telldir telldir95
#define seekdir seekdir95
#define closedir closedir95
#define rewinddir rewinddir95
#endif

#endif
