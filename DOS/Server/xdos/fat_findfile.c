/*    
	fat_findfile.c	1.15
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

 */

#include "platform.h"

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "xdos.h"
#include "Log.h"
#include "BinTypes.h"
#include "xdosproc.h"
#include "dosdefn.h"
#include "Endian.h"

#include "fat_tools.h"
#include "mfs_config.h"
#include "mfs_core.h"
#include "mfs_dir.h"

extern CONFIG xdos;
extern int    DiskHandle;

void CopyDTAData(LPBYTE ,char *,WORD ,WORD ,DWORD ,BYTE );
unsigned int findfile( char *,int  , LPBYTE );
static DWORD dwDir;

int     dos_chdir(char *, struct DirEntry *, long *);
WORD    GetWord(BYTE *);
DWORD   GetDword(BYTE *);
int     GetNextDirEnt(struct DirEntry *, long *);
int     NameCombine(char *, char *);
unsigned long error_code(unsigned int);
/*********************************************************************/
DWORD 
fat_findfirst(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	char    *Directory;   /* p1 */
        unsigned attribute;   /* p2 */
        LPFFINFO lpff;        /* p3 */

	char    Path[_MAX_PATH];
        long    lDummy;
        struct  DirEntry Entry;
	DWORD   status;	

	Directory = (char *)p1;
        attribute = (unsigned)p2;
        lpff = (LPFFINFO)p3;

	if (lpff == NULL)
	   {
	   errno = ENOENT;
	   return MFS_FAILURE;
           }
        /*
        **    Special case for _A_VOLID...
        */
	if (attribute == _A_VOLID) 
           if (xdos.drivemap[1] != NULL) /* hardcoded A: drive */
              {
              strcpy(lpff->FileName, xdos.drivemap[1]->name); /* return A: drive mapping */
              return MFS_SUCCESS;
              }
           else return MFS_FAILURE;
           
	MFS_DOSNAME(Path, Directory);
        dos_chdir(Path, &Entry, &lDummy);

        status = fat_findnext((DWORD)lpff, 0L, 0L, 0L);
	if (status == MFS_SUCCESS /*&& (lpff->FileAttr & attribute)*/)
	   return MFS_SUCCESS;
        else return MFS_FAILURE;
}


/*********************************************************************/
DWORD 
fat_findnext(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPFFINFO lpff; /* p1 */
        long     lDummy;
        struct   DirEntry Entry;

	lpff = (LPFFINFO)p1;

        while (1) 
	   {
           if (GetNextDirEnt(&Entry, &lDummy) < 0) 
              {
              errno = ENOENT;
              return MFS_FAILURE;
              }
           if ((Entry.Attrib != lpff->FileAttr) && (lpff->FileAttr == _A_SUBDIR)) 
              {
              errno = ENOENT;
              return MFS_FAILURE;
              }
           if (Entry.Name[0] == 0) 
              {
              errno = EMFILE;
              return MFS_FAILURE;
              }
           if (Entry.Name[0] != 0xE5)
              break;
           }

        NameCombine((char *)&Entry.Name[0], (char *)&lpff->FileName[0]);
        if (Entry.Attrib == _A_SUBDIR)
           lpff->FileAttr = Entry.Attrib;
        else lpff->FileAttr = 0; /* we don't care about anything but dirs */
        lpff->FileTime = GetWord((BYTE *)&Entry.Time[0]);
        lpff->FileDate = GetWord((BYTE *)&Entry.Date[0]);
        lpff->FileSize = GetDword((BYTE *)&Entry.Size[0]);
        return MFS_SUCCESS;
}


/*********************************************************************/
DWORD 
fat_findfile(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
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
	DWORD  DosDrive;

	dosname = (LPSTR)p1;
	attr = (int)p2;
	dta = (LPBYTE)p3;

	if (dosname) 
           {
           DosDrive = MFS_DOSNAME(result, dosname);
 
if (!TheSameDisk(DiskHandle))
   DiskInit(DriveMapping(DosDrive));

	   if (dwDir) 
	      fat_closedir(dwDir, 0L, 0L, 0L);
	   strcpy(result, dosname);
	   p = (char *)strrchr(result,'\\');
	   if (p) 
	      p++;  /* in 'D:\*.*' string 'p' points to 1st star */
           else {
		fat_getcwd(0L, (DWORD)dirbuf, 256L, 0L);
		mfs_config(XMFS_CFG_DOSNAME, 0L, (DWORD)result, (DWORD)dirbuf);
		p = dosname;
		}
	   memcpy((void *)(dta+8),p,12);
	   *(dta+20) = 0;
	   *(dta+7) = (BYTE)attr;

           if (*(p-1) == '\\')
              *p = '\0'; /* this makes 'D:\' from 'D:\*.*' */

           if (result[1] == ':' && result[2] == '\\' && result[3] == '\0')
              dwDir = fat_opendir((DWORD)&result[2], 0L, 0L, 0L); /* pass single backslash, i.e. root directory */
	   else dwDir = fat_opendir((DWORD)result, 0L, 0L, 0L);
	   }
	else {
	     attr = (int)*(dta+7);
	     }
	filename = (LPSTR)(dta+8);
	if (dwDir == 0L)
	   return MFS_FAILURE;

	while (TRUE) 
           {
	   fname = (LPSTR)fat_readdir(dwDir, 0L, 0L, 0L);
	   if (fname == NULL) 
              {
	      fat_closedir(dwDir, 0L, 0L, 0L);
	      dwDir = 0L;
	      error_code(NO_MORE_FILES);
	      return MFS_FAILURE;
	      }
	    
	   /* check for match with filespec */
	   if (mfs_regexp((DWORD)fname, (DWORD)filename, 0L, 0L))  /* this should eventually be fat_regexp */
              {
	      /* matched, now check file attributes */
	      if (fat_stat((DWORD)fname, (DWORD)&sbuf, 0L, 0L) == 0) 
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
	      CopyDTAData(dta,fname,0,0,0,bAttr);
	      break;
	      } 
	   } /* while */
	return MFS_SUCCESS;
}

