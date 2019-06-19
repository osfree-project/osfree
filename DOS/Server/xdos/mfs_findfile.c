/*    
	mfs_findfile.c	1.22
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

*	MFS find file implementation.

 */

#include "platform.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#include "xdos.h"
#include "Log.h"
#include "BinTypes.h"
#include "xdosproc.h"
#include "dosdefn.h"
#include "Endian.h"

#include "mfs_config.h"
#include "mfs_core.h"
#include "mfs_dir.h"

#ifndef TM_TIME
#define TM_TIME(ft)	((((ft)->tm_hour<<11) & 0xF800) | \
			(((ft)->tm_min<<5) & 0x07E0) | \
			(((ft)->tm_sec>>1) & 0x001F))
#endif

#ifndef TM_DATE
#define TM_DATE(ft)	(((((ft)->tm_year - 80)<<9) & 0xFE00) | \
			((((ft)->tm_mon+1)<<5) & 0x01E0) | \
			(((ft)->tm_mday) & 0x001F))
#endif

extern CONFIG xdos;
void CopyDTAData(LPBYTE ,char *,WORD ,WORD ,DWORD ,BYTE );
unsigned long error_code(unsigned int);
static int dirp;

/*********************************************************************/
DWORD 
mfs_findfirst(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   LPSTR     filename; /* p1 */
   int       attrib;   /* p2 */
   LPFFINFO  lpff;     /* p3 */
   int    nDrive;
   char   szDir[_MAX_PATH];
   DWORD  status;

   filename = (LPSTR)p1;
   attrib = (int)p2;
   lpff = (LPFFINFO)p3;

   /*
   **    Special case for _A_VOLID...
   */
   if (attrib == _A_VOLID)
      {
      if (toupper(filename[0]) < 'A' || toupper(filename[0]) > 'Z')
         nDrive = MFS_GETDRIVE();
      else if (filename[1] != ':')
              nDrive = MFS_GETDRIVE();
           else nDrive = toupper(filename[0]) - 'A' + 1;

      MFS_GETCWD(nDrive, szDir, sizeof(szDir));
      if ((strcmp(szDir, "/") == 0 || strcmp(szDir, "\\") == 0) && xdos.drivemap[nDrive] != NULL)
         {
	 strcpy(lpff->FileName, xdos.drivemap[nDrive]->name);
         return MFS_SUCCESS;
         }
      else return MFS_FAILURE;
      }
   else {
        /*   Retrive directory spec, then chdir to that directory and
        **   call findnext to find first file...
	*/
	MFS_ALTNAME(szDir, filename);
        MFS_CHDIR(szDir);
	status = MFS_FINDNEXT(lpff);
	if (status == 0L && (lpff->FileAttr & attrib))
	   return MFS_SUCCESS;
	}

   errno = ENOENT;
   return MFS_FAILURE;
}

/*********************************************************************/
DWORD 
mfs_findnext(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
	return(mfs_findfile((DWORD)NULL, p2, p3, p4));
}

/*********************************************************************/
DWORD 
mfs_findfile(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR   dosname;   /* p1 */
	int	attr;      /* p2 */
	LPBYTE  dta;	   /* p3 */
	char    dirbuf[256];
	char	result[256];
	BYTE	bAttr = 0x0;
	LPSTR   filename;
	char   *fname;
	char   *p;
	struct stat sbuf; 	 
	struct  tm *filetime;

	dosname = (LPSTR)p1;
	attr = (int)p2;
	dta = (LPBYTE)p3;

	if (dosname) 
           {
	   if (dirp) 
	      mfs_closedir(dirp, 0L, 0L, 0L);
	   strcpy(result, dosname);
	   p = (char *)strrchr(result,'\\');
	   if (p) 
              {
	      *p++ = 0;
	      } 
           else {
		p = (char *)strrchr(result,'/');
		if (p) 
                   {
		   *p++ = 0;
		   } 
                else {
		     mfs_getcwd(0L, (DWORD)dirbuf, 256L, 0L);
		     mfs_config(XMFS_CFG_DOSNAME, 0L, (DWORD)result, (DWORD)dirbuf);
		     p = dosname;
		     }
		}
	   memcpy((char *)dta+8,p,12);
	   *(dta+20) = 0;
	   *(dta+7) = (BYTE)attr;
	   dirp = (int)mfs_opendir((DWORD)result, 0L, 0L, 0L);
	   }
	else {
	     attr = (int)*(dta+7);
	     }
	filename = (LPSTR)(dta+8);

	if (dirp == 0)
	   return MFS_FAILURE;

	while (TRUE) 
           {
	   fname = (LPSTR)mfs_readdir((DWORD)dirp, 0L, 0L, 0L);
	   if (fname == NULL) 
              {
	      mfs_closedir((DWORD)dirp, 0L, 0L, 0L);
	      dirp = 0;
	      error_code(NO_MORE_FILES);
	      return MFS_FAILURE;
	      }
	    
	   /* check for match with filespec */
	   if (mfs_regexp((DWORD)fname, (DWORD)filename, 0L, 0L)) 
              {
	      /* matched, now check file attributes */
	      if (mfs_stat((DWORD)fname, (DWORD)&sbuf, 0L, 0L) == 0) 
                 {	
		
		 if (sbuf.st_mode & S_IFDIR) 
                    {
		    if ((attr & 0x10) == 0)
		       continue;
		    bAttr = 0x10;
		    } 
                 else {
		      if ((attr & 0x8010) == 0x8010)
			 continue;
		      bAttr = 0;
		      }
		 }
	      else 
		 {
		continue;
	         }

	      filetime = localtime(&sbuf.st_mtime);
	      strupr(fname);
	      CopyDTAData(dta,fname, TM_TIME(filetime), TM_DATE(filetime),
			(DWORD)sbuf.st_size,bAttr);
	      break;
	      } 
	   } /* while */
	return MFS_SUCCESS;
}


