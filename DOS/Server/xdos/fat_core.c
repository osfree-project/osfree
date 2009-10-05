/*    
	fat_core.c	1.21
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

/*   08-NOV-1995   Michael Dvorkin   Initial release */

#include "platform.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#ifndef NETWARE
#include <dirent.h>
#else   
#define _SIZE_T_DEFINED_
#include <direct.h>
#include <nwdir.h>
#endif  

#include "windows.h"
#include "dos_error.h"
#include "dos.h"
#include "Log.h"
#include "kerndef.h" /* WinMalloc() and WinFree() */

#include "fat_tools.h"
#include "mfs_config.h"
#include "mfs_core.h"

#define MFS_FILEBITS    (S_IFDIR|S_IFCHR|S_IFBLK)

extern CONFIG xdos;
extern int    DiskHandle;
extern int    DosFlag;
/*
*/
static FFINFO  DirStream;
static LPFFINFO lpDirStream;

BpbInfo *BootBlock;
FtableEntry Ftable[FTABLE_SIZE];
DWORD GetDword(BYTE *);
WORD  GetWord(BYTE *);
void  PutWord(BYTE *, WORD);
void  PutDword(BYTE *, DWORD);
int   DosExist(char *, struct DirEntry *, long *);
int   GetFreeDirEnt(struct DirEntry *, long *);
int   NameSplit(char *, char *, char *);
int   GetNextFreeCluster(WORD);
int   WriteDirEntry(struct DirEntry *, long);
int   MakeNewCluster(struct DirEntry *);
int   InitDirEnt(void);
int   GetNextDirEnt(struct DirEntry *, long *);
int   dos_chdir(char *, struct DirEntry *, long *);
int   GetNextCluster(WORD);
int   SetNextCluster(WORD, WORD);
void  SaveInfo(void);
void  RestoreInfo(void);
void  SetTimeDate(struct DirEntry *);
/*********************************************************************/
DWORD 
fat_stat(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR        filename; /* p1 */
	struct stat *sbuf;     /* p2 */

	char   Path[_MAX_PATH];
        long   EntryAddress;
        struct DirEntry Dir;
	int    Mode = 0;
	DWORD  DosDrive;

	filename = (LPSTR)p1;
	sbuf = (struct stat *)p2;

	DosDrive = MFS_DOSNAME(Path, filename);

if (!TheSameDisk(DiskHandle))
   DiskInit(DriveMapping(DosDrive));

        if (DosExist(Path, &Dir, &EntryAddress) < 0)
           {
	   errno = ENOENT;
           return MFS_FAILURE;
           }

	sbuf->st_dev   = 0;
	sbuf->st_rdev  = sbuf->st_dev;
	sbuf->st_nlink = 1;

	Mode |= S_IFREG;
	if (Dir.Attrib & READ_ONLY)
	   Mode |= S_IREAD;
        else Mode |= S_IWRITE;

	if (Dir.Attrib  & DIRECTORY)
	   Mode |= S_IFDIR;
	sbuf->st_mode  = Mode;

	sbuf->st_size  = GetDword((BYTE *)&(Dir.Size[0]));
	sbuf->st_atime = 0;               /* STUB */
	sbuf->st_ctime = sbuf->st_atime;  /* STUB */
	sbuf->st_mtime = sbuf->st_atime;  /* STUB */

	return MFS_SUCCESS;
}

/*********************************************************************/
DWORD 
fat_access(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int 		mode;
	LPSTR		address;
	unsigned int   *attribute;

        int    rc;
        char   Path[_MAX_PATH];
        long   EntryAddress;
        struct DirEntry Dir;

	mode = (int)p1;
	address = (LPSTR)p2;
	attribute = (unsigned int *)p3;

        MFS_DOSNAME((LPSTR)Path, (LPSTR)address);

        if (DosExist(Path, &Dir, &EntryAddress) < 0)
           {
	   errno = ENOENT;
           return MFS_FAILURE;
           }
        if (mode == GET_ATTRIBUTE) 
           {
           *attribute = Dir.Attrib;
           return MFS_SUCCESS;
           } 
        else if (mode == SET_ATTRIBUTE) 
                {
                Dir.Attrib = *attribute;
                lseek(DiskHandle, EntryAddress, SEEK_SET);
                rc = write(DiskHandle, (LPCSTR)&Dir, 32);
                if (rc < 0)
                   logstr(LF_ERROR,"write error #12\n");
                return MFS_SUCCESS;
                }
	return MFS_SUCCESS;	
}


/*********************************************************************/
DWORD fat_rename(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
	char		*from;
	char		*to;

        struct stat     fileinfo;
        int             rc;
        char fname[256];
        char tname[256];

	from = (LPSTR)p1;
	to   = (LPSTR)p2;

        MFS_DOSNAME((LPSTR)fname, (LPSTR)from);
        MFS_DOSNAME((LPSTR)tname, (LPSTR)to);

        /* check if the from file exists... */
        rc = fat_stat((DWORD)fname, (DWORD)&fileinfo, 0L, 0L);

        /* if it does, link it to the new file... */
        if (rc == 0) 
	   rc = rename(fname, tname);

        return convert_error(errno);
}

/*********************************************************************/
DWORD 
fat_copy(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   DWORD status = MFS_FAILURE;


   return status;
}

/*********************************************************************/
DWORD 
fat_mkdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	char	*address;
	int	 mode;

        char 	 Path[256];
        char    *Tail;
        char    Head[255];
        long    EntryAddress;
        struct  DirEntry Dir;
        char    FullName[12];
        char    Base[9];
        char    Ext[4];

	address = (LPSTR)p1;
	mode = (int)p2;

        MFS_DOSNAME(Path, address);

        if (DosExist(Path, &Dir, &EntryAddress) == 0) 
           {
           DosFlag = (FILE_EXISTS | FAT_CARRY_FLAG);
           return MFS_FAILURE;
           }

        Tail = GetHead(&Head[0], Path);
        if (Head[0] != 0)
           dos_chdir((char *)&Head[0], &Dir, &EntryAddress);

        if (GetFreeDirEnt(&Dir, &EntryAddress) == -1) 
           {
           errno = ENOENT;
           DosFlag = CANNOT_MAKE_DIRECTORY_ENTRY | FAT_CARRY_FLAG;
           return MFS_FAILURE;
           }

        /* Split the name */
        strcpy(FullName, Tail);
        NameSplit(FullName,Base,Ext);
        strncpy((char *)&Dir.Name[0], Base,8);
        strncpy((char *)&Dir.Name[8], Ext,3);

        /* Initialise the fields */
        Dir.Attrib = DIRECTORY;
        PutWord((BYTE *)&Dir.Start[0], GetNextFreeCluster(0));
        PutDword((BYTE *)&Dir.Size[0], 0);
        SetTimeDate(&Dir);

        /* Make a directory entry */
        WriteDirEntry(&Dir,EntryAddress);

        /* Write out a new cluster for this directory entry */
        MakeNewCluster(&Dir);

        return MFS_SUCCESS;
}

/*********************************************************************/
DWORD 
fat_chdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	char *pathname;

        char     Path[_MAX_PATH];
        char    *Tail;
        char    Head[255];
        long    EntryAddress;
        struct  DirEntry Dir;
   	DWORD   status = MFS_FAILURE;
	DWORD   DosDrive;


	pathname = (LPSTR)p1;
        /*
        **   Get full DOS path, including drive letter. Then scan
        **   throught the drives list to find cwd to replace.
        */
#ifdef SEVERE
        printf("CHDIR: pathname '%s' ", pathname);
#endif
        DosDrive = MFS_DOSNAME(Path, pathname);
if (!TheSameDisk(DiskHandle))
   DiskInit(DriveMapping(DosDrive));

        if (Path[0] >= 'A' && Path[0] <= 'Z' && Path[1] == ':' && Path[2] == '\\')
           strcpy(&Path[0], &Path[2]);
        if (DosExist(Path, &Dir, &EntryAddress) == 0)
           {
           DosFlag = (FILE_EXISTS | FAT_CARRY_FLAG);
           return MFS_FAILURE;
           }

        Tail = GetHead(&Head[0], Path);
        if (Head[0] != 0)
           {
           status = dos_chdir((char *)&Head[0], &Dir, &EntryAddress);
	   if (status == 0L)
              {
              if (xdos.drivemap[0]->cwd != NULL)
                 WinFree(xdos.drivemap[0]->cwd);
              xdos.drivemap[0]->cwd = (LPSTR)WinMalloc(strlen(Path)+1);
              strcpy(xdos.drivemap[0]->cwd, Path);
#ifdef SEVERE
              printf("CHDIR: cwd '%s'\n", xdos.drivemap[0]->cwd);
#endif
              }
           }
        return status;
}

/*********************************************************************/
DWORD 
fat_getcwd(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int	drive;
	LPSTR	lpszPathName;
	int	len;

	drive = (int)p1;
	lpszPathName = (LPSTR)p2;
	len = (int)p3;

	if (drive == 0)
	   drive = xdos.currentdrive;
	if (xdos.drivemap[drive] != NULL)
	   if (xdos.drivemap[drive]->cwd != NULL)
	      {
	      strcpy(lpszPathName, xdos.drivemap[drive]->cwd);
#ifdef LATER
              /*
              **   DOS getcwd should strip leading backslash...
              */
              if (lpszPathName[0] == '/' || lpszPathName[0] == '\\')
                 strcpy((char *)&lpszPathName[0], (char *)&lpszPathName[1]);
#endif
	      return MFS_SUCCESS;
	      }

        *lpszPathName = '\0';
        return MFS_FAILURE;
}

/*********************************************************************/
DWORD 
fat_opendir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR       pathname; /* p1 */
        char        dir[256];
	DWORD       status;

	pathname = (LPSTR)p1;
        MFS_DOSNAME(dir, pathname);

        if (strcmp(dir, "\\") == 0)
           {
           strcpy(DirStream.FileName, "\\");
           lpDirStream = &DirStream;
           InitDirEnt();
           SaveInfo();
           return (DWORD)lpDirStream; 
           }
        RestoreInfo();
	status = fat_findfirst((DWORD)dir, (DWORD)DIRECTORY, (DWORD)&DirStream, 0L);
        if (status != MFS_SUCCESS)
	   return (DWORD)NULL;
	else do {
		if (DirStream.FileAttr & _A_SUBDIR)
                   {
		   lpDirStream = &DirStream;
		   return (DWORD)lpDirStream;
		   }
		}
	     while (!(status = fat_findnext((DWORD)&DirStream, 0L, 0L, 0L)));
	     
        return (DWORD)NULL;
}

/*********************************************************************/
DWORD 
fat_readdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int 	    dirp;    /* p1 */
	DWORD       status;

	dirp = (int)p1;
        
        if (lpDirStream == NULL)
	   return (DWORD)NULL;

        RestoreInfo();
	status = fat_findnext((DWORD)&DirStream, 0L, 0L, 0L);
	if (status == 0L)
           {
           
           SaveInfo();
	   return (DWORD)(LPSTR)&DirStream.FileName[0];
           }
        
	return (DWORD)NULL;
}


/*********************************************************************/
DWORD 
fat_closedir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int  dirp;
	dirp = (int)p1;
        
	lpDirStream = NULL;
        RestoreInfo();
        return (DWORD)NULL;
}

/*********************************************************************/
DWORD 
fat_splitpath(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   DWORD status = MFS_FAILURE;


   return status;
}

/*********************************************************************/
DWORD 
fat_makepath(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   DWORD status = MFS_FAILURE;


   return status;
}


/*********************************************************************/
DWORD 
fat_rmdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR	address; /* p1 */
        char	Path[_MAX_PATH];
        int     temp;
        long    EntryAddress, EntryOffset;
        struct  DirEntry Dir, TempDir;

	address = (LPSTR)p1;

        MFS_DOSNAME(Path, address);

        DosFlag = 0;
        if (DosExist(Path, &Dir, &EntryAddress) < 0) 
	   {
           DosFlag = (PATH_NOT_FOUND | FAT_CARRY_FLAG);
           return MFS_FAILURE;
           }

        if (Dir.Attrib & READ_ONLY) 
	   {
           DosFlag = (ACCESS_DENIED | FAT_CARRY_FLAG);
           return MFS_FAILURE;
           } 
        dos_chdir(Path, &Dir, &EntryAddress);
        while ( GetNextDirEnt(&TempDir, &EntryOffset) != -1) 
              {
              if ((TempDir.Name[0] == '.') || (TempDir.Name[0] == 0xE5))
                 continue;
              if (TempDir.Name[0] == 0)
                 break;
              DosFlag = (FILE_EXISTS | FAT_CARRY_FLAG);
              return MFS_FAILURE;
              }

        temp = GetWord((BYTE *)&Dir.Start[0]);
        while ((temp = GetNextCluster(temp)) != -1)
              SetNextCluster(temp, 0);
        temp = GetWord((BYTE *)&Dir.Start[0]);
        SetNextCluster(temp, 0);
        Dir.Name[0] = 0xE5;
        return (DWORD)WriteDirEntry(&Dir,EntryAddress);
}


/*********************************************************************/
DWORD 
fat_config(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	return MFS_SUCCESS;
}



/*********************************************************************/
DWORD 
fat_regexp(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	return MFS_SUCCESS;
}



