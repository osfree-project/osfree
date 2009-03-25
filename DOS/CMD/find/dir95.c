/* $RCSfile: DIR95.C $
   $Locker: ska $	$Name:  $	$State: Exp $

   	dir - open/close/read/seek/tell

	POSIX.1 variant

	== For instance:
	DIR95 *dir;
	struct dirent95 *de;

	if((dir = opendir95(pathname)) != NULL) {
		while((de = readdir95(dir)) != NULL)
			printf("%ld. entry = \"%s\"\n", telldir95(dir), de->d_name);
		closedir95(dir);
	}
	==

	If the path starts with "\\\\" both backslashes remain; that
	gives some support for UNC path spec.

   $Log: DIR95.C $
   Revision 1.2  2000/01/11 09:34:23  ska
   add: support Turbo C v2.01

   Revision 1.1  2000/01/11 09:10:08  ska
   Auto Check-in

*/

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <dos.h>
#include <dir.h>

#include "dir95.h"
#include "io95.h"

#define PATTERN "\\*.*"		/* search pattern */
#define MAXDRVLEN 2

#ifndef offsetof
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#endif

#ifndef lint
static char const rcsid[] = 
	"$Id: DIR95.C 1.2 2000/01/11 09:34:23 ska Exp ska $";
#endif


/*
 * opendir
 */

DIR95 *opendir95(const char * const name)
{
   register DIR95 *nd;
   char cwd[MAXPATH95 + 2048];
   	/* make sure that cwd can hold:
   		1) a path component
   		2) the string terminator
   		3) the drive spec
   		4) the "root" backslash
   		5) some room while constructing the fully-qualified path
   	*/

   	assert(name);

	if(!fullpath95(cwd, name, sizeof(cwd) - sizeof(PATTERN)))
		return NULL;

	assert(strlen(cwd) > 2);

	if(!cwd[3])		/* root */
		strcpy(&cwd[2], PATTERN);
	else strcat(cwd, PATTERN);		/* append the search pattern */

   /* construct DIR95 */
   if ((nd = (DIR95 *)malloc(sizeof(DIR95) + max(strlen(cwd), DTALEN -
      sizeof(DIR95) - offsetof(DIR95, dd_dta)))) == NULL)
	  return NULL;

   strcpy(nd->dd_dir, cwd);

   /* perform the first search */
   nd->dd_cnt = 1;			/* force seekdir() to re-read directory */
   seekdir95(nd, 0l);

   if (nd->dd_stat) {		/* failed */
      free((char *)nd);
      return NULL;
   }

   return nd;
}


struct dirent95 *readdir95(DIR95 * const dirp)
{	
	assert(dirp);
	if (dirp->dd_stat)			/* already at end of directory */
		return NULL;

	++dirp->dd_cnt;				/* keep track of read entries */
	assert(dirp->dd_dta.ff_95.ff_longname[0]);
	strcpy(dirp->dd_de.d_name, dirp->dd_dta.ff_95.ff_longname);

   /* read ahead */
   dirp->dd_stat = findnext95(&dirp->dd_dta);

   return &dirp->dd_de;
}


void closedir95(DIR95 * const dirp)
{
	assert(dirp);
	findstop95(&dirp->dd_dta);
	free((char *)dirp);
}


void seekdir95(DIR95 * const dirp, long pos)
{	
	assert(dirp);
	if(dirp->dd_cnt > pos) {		/* we seek backwards -> restart */
		findstop95(&dirp->dd_dta);
		dirp->dd_stat = findfirst95(dirp->dd_dir, &dirp->dd_dta, 0x37);
		dirp->dd_cnt = 0;
	}

	while(dirp->dd_cnt < pos && !dirp->dd_stat) {
		dirp->dd_stat = findnext95(&dirp->dd_dta);
		++dirp->dd_cnt;
	}
}


long telldir95(DIR95 * const dirp)
{
	assert(dirp);
	return dirp->dd_cnt;
}
