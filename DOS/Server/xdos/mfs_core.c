/*    
	mfs_core.c	1.39
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

**   Core MFS logic (this code SHOULD be
**   system independent, i.e. ANSI C).
**
**   Multi File System:
**   - native file system (Un*x, Mac)
**   - FAT (A: drive, etc.)
**   - pseudo-disk (native file a-la DoubleSpace)
**
**   06-NOV-1995   Michael Dvorkin   
**   Initial release...
**
*/

#include "platform.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include <unistd.h>

#ifdef NETWARE
#define _SIZE_T_DEFINED_
#include <direct.h>
#include <nwdir.h>
#else
#include <dirent.h>
#endif

#include "windows.h"
#include "dos_error.h"
#include "Log.h"
#include "kerndef.h" /* WinMalloc and WinFree */

#include "mfs_config.h"
#include "mfs_core.h"
#include "fn_36.h"

/*** #define NETWARE 1 ***/
#define MFS_FILEBITS    (S_IFDIR|S_IFCHR|S_IFBLK)
unsigned long error_code(unsigned int);
void dos_setup(void);
int GetPathType(LPSTR);
int GetDriveFromPath(LPSTR);
int GetDriveFromHandle(DWORD);

MFSPROC MFSProc[MFSH_LASTHOOK+1] = /* native handlers */
{
mfs_init,
mfs_config,    /* mfs_config.c */
mfs_stat,
mfs_access,
mfs_open,      /* mfs_fileio.c */
mfs_close,     /* mfs_fileio.c */
mfs_read,      /* mfs_fileio.c */
mfs_write,
mfs_seek,      /* mfs_fileio.c */
mfs_flush,     /* mfs_fileio.c */
mfs_create,    /* mfs_fileio.c */
mfs_delete,    /* mfs_fileio.c */
mfs_rename,
mfs_mkdir,
mfs_chdir, 
mfs_getcwd,
mfs_regexp,    /* mfs_regexp.c */
mfs_opendir,
mfs_readdir,
mfs_closedir,
mfs_fcntl,     /* mfs_fileio.c */
mfs_ioctl,     /* mfs_fileio.c */
mfs_findfirst,
mfs_findnext,
mfs_findfile,  /* mfs_findfile.c */
mfs_rmdir,
mfs_touch,
mfs_splitpath,
mfs_makepath,
mfs_finfo,
mfs_copy,
mfs_filedrive,
mfs_diskfree,
mfs_fileinfo
};

MFSPROC FATProc[MFSH_LASTHOOK+1] = /* FAT handlers */
{
mfs_init,
mfs_config,
fat_stat,
fat_access,
fat_open,
fat_close,
fat_read,
fat_write,
fat_seek,
NULL,/*fat_flush,*/
fat_create,
fat_delete,
NULL,/*fat_rename,*/
fat_mkdir,
fat_chdir,
fat_getcwd,
  NULL,/*mfs_regexp,*/
fat_opendir,
fat_readdir,
fat_closedir,
NULL,/*fat_fcntl, - it's here but untested */
NULL,/*fat_ioctl, - it's here but untested */
fat_findfirst,
fat_findnext,
fat_findfile,
fat_rmdir,
NULL, /*fat_touch*/
NULL, /*mfs_splitpath,*/
NULL, /*mfs_makepath,*/
NULL, /*mfs_finfo,*/
NULL, /*mfs_copy*/
NULL, /*mfs_filedrive*/
NULL, /*mfs_diskfree*/
NULL  /*mfs_fileinfo*/
};

extern CONFIG xdos;
/*********************************************************************/
DWORD 
MFS_CALL(UINT uiHook, DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	DWORD   status = MFS_FAILURE;   /* callback completion code */
	MFSPROC *dispatch;
	int     nDrive;
static  int     nFindFileDrive;
static	BOOL    ynMFSInit = FALSE;

   if (!ynMFSInit || uiHook == MFSH_INIT)
      {
      mfs_init(0L,0L,0L,0L);
      ynMFSInit = TRUE;
      if (uiHook == MFSH_INIT)
         return 1L;
      }
   /*
   **   NOW WE MUST DETERMINE DRIVE TYPE TO CALL THE APPROPRIATE
   **   FUNCTION FROM THE DISPATCH TABLE. WE DO THAT BY PARSING
   **   FILE SPEC AND/OR FILE HANDLE.
   */
   if (uiHook >= MFSH_FIRSTHOOK && uiHook <= MFSH_LASTHOOK)
      {
      switch(uiHook)
         {
         case MFSH_STAT:          /* p1 = (LPSTR)pathname */
	 case MFSH_OPEN:
	 case MFSH_CREATE:
	 case MFSH_DELETE:
	 case MFSH_RENAME:
	 case MFSH_COPY:
	 case MFSH_MKDIR:
	 case MFSH_CHDIR:
	 case MFSH_REGEXP:
	 case MFSH_OPENDIR:
	 case MFSH_READDIR:
	 case MFSH_CLOSEDIR:
	 case MFSH_RMDIR:
	 case MFSH_FINDFIRST:
	 case MFSH_FILEDRIVE:
            nDrive = GetDriveFromPath((LPSTR)p1); 
	    break;

	 case MFSH_ACCESS:	/* p2 = (LPSTR)pathname */
            nDrive = GetDriveFromPath((LPSTR)p2); 
	    break;

	 case MFSH_FINDFILE:
            if (p1)
               nDrive = nFindFileDrive = GetDriveFromPath((LPSTR)p1); 
            else nDrive = nFindFileDrive;
	    break;

	 case MFSH_CLOSE:	/* p1 = (int)file descriptor */
	 case MFSH_READ:
	 case MFSH_WRITE:
	 case MFSH_SEEK:
	 case MFSH_FLUSH:
	 case MFSH_IOCTL:
	 case MFSH_FCNTL:
	 case MFSH_TOUCH:
            nDrive = GetDriveFromHandle(p1);
	    break;

	 case MFSH_DISKFREE:    /* p1 = (int)drive number */
	     nDrive = (int)p1;
	     break;

	 case MFSH_GETCWD:	/* use current drive */
	 case MFSH_CONFIG:
	 case MFSH_INIT:
	 case MFSH_FINDNEXT: 

         default:
            nDrive = xdos.currentdrive;
         }

      if (xdos.drivemap[nDrive] == NULL)
         {
         nDrive = xdos.currentdrive;
         }

      dispatch = xdos.drivemap[nDrive]->lpfnDispatch;
      if (dispatch != NULL && dispatch[uiHook] != NULL)
         status = (*dispatch[uiHook])(p1, p2, p3, p4);
      }

   return status;
}


/*********************************************************************/
DWORD 
mfs_init(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
   dos_setup();
   return 1;
}

/*********************************************************************/
DWORD 
mfs_stat(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR        filename;
	struct stat *sbuf;
        LPSTR LastChar;
        int nlength;


	filename = (LPSTR)p1;
	sbuf = (struct stat *)p2;

#ifdef mac
        /* Mac: need to normalize the filename, so use a wrapper */
        return UNIX_stat(filename, sbuf);
#else
        /* 
        ** In MS-DOS/MS-Windows, if a directory path (e.g. "C:\temp\")ends 
        ** with a '\' character then stat() gets confused and thinks its a
        ** bad path.  So we should trim it if its advisable. 
        */
        nlength = strlen( filename );

        /* Verify that the path is not simply "\" */
        if( nlength > 1 )
        {
          /* Verify that there is a separator at the end of the path. */
          LastChar = filename + nlength - 1;
          if( *LastChar == '\\' || *LastChar == '/' )
          {
            /* Verify we don't have the case "C:\" */
            if( *(LastChar - 1) != ':' )
            {
              *LastChar = '\0';
            }
          }
        }
        return stat(filename, sbuf);
#endif
}

/*********************************************************************/
DWORD 
mfs_access(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int 		mode;
	LPSTR		address;
	unsigned int   *attributte;

        int             i;
        int             rc;
        struct stat     fileinfo;
        char fname[256];
	char perm;

	mode = (int)p1;
	address = (LPSTR)p2;
	attributte = (unsigned int *)p3;

        MFS_ALTNAME((LPSTR)fname, (LPSTR)address);

        /* modes need to be interpreted according to who we are, vs. the
         * owner of the file.  if not same, then use other bits, otherwise
         * use owner bits. ie. we may have read only file...
         *
         * what about archive, system and hidden?
         * not applicable? for some devices, but do we save info?
         * we may use group/other bits?
         *
         */
        rc = mfs_stat((DWORD)fname, (DWORD)&fileinfo, 0L, 0L);
        if (mode == 0) 
           {
           if (rc == 0) 
              {
              i = fileinfo.st_mode;
              if (S_ISDIR(i))
                 rc = _A_SUBDIR;

              if (S_ISCHR(i))
                 return error_code(ACCESS_DENIED);

              if (S_ISBLK(i))
                 return error_code(ACCESS_DENIED);

#ifdef unix
	      /* Who are we? */
	      {
		uid_t uid = geteuid();
		gid_t gid = getegid();
		if (uid == fileinfo.st_uid)
		  perm = (fileinfo.st_mode & 0700) >> 6;
		else if (gid == fileinfo.st_gid)
		  perm = (fileinfo.st_mode & 0070) >> 3;
		else
		  perm = fileinfo.st_mode & 0007;
	      }
#else /* DOS, etc. use same bits in every field => always 0444, 0666, or 0777 */
	      perm = fileinfo.st_mode & 0007; /* Only use "other" field */
#endif
	      if (!(perm & 2))
		rc |= _A_RDONLY;

              /* Other bits not used currently */
              *attributte = rc;
              return MFS_SUCCESS;
              }

           *attributte = 0;
           return convert_error(errno);
           }
        else {
             /* set the file... */
             /* what bits can be set... */
             /* for now, we say it worked, if the file exists */
             /* use other bits, and force either the owner or */
             /* group bits to be correct? */
             /* n.b. this at least checks the file exists     */

             if (rc == 0)
                return MFS_SUCCESS;
             }

        return convert_error(errno);
}


/*********************************************************************/
DWORD 
mfs_rename(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	char		*from;	/* p1 */
	char		*to;	/* p2 */

        struct stat     fileinfo;
        int             rc;
        char fname[256];
        char tname[256];

	from = (LPSTR)p1;
	to   = (LPSTR)p2;

        MFS_ALTNAME((LPSTR)fname, (LPSTR)from);
        MFS_ALTNAME((LPSTR)tname, (LPSTR)to);

        /* check if the from file exists... */
        rc = mfs_stat((DWORD)fname, (DWORD)&fileinfo, 0L, 0L);

        /* if it does, link it to the new file... */
        if (rc == 0)
	   rc = rename(fname, tname);

	if (rc == 0)
	  return MFS_SUCCESS;

        return convert_error(errno);
}

/*********************************************************************/
DWORD 
mfs_copy(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   char *existing; /* p1 */
   char *new; /* p2 */
   BOOL existFail; /* p3 */

   HANDLE hFileExist, hFileNew;
   int count;
   long respond;
   char *buffer;
   unsigned int attrExist;

   existing = (char *)p1;
   new = (char *)p2;
   existFail = (BOOL)p3;

   if (MFS_ACCESS(0, existing, &attrExist) == MFS_FAILURE)
     return MFS_FAILURE; /* No such file -- can't copy */
   if (strcmp(existing, new) == 0)
     return MFS_FAILURE; /* Can't copy a file into itself -- I could probably do better */

   respond = MFS_OPEN(existing, O_RDONLY);
   if (respond < 0)
     return MFS_FAILURE;
   else
     hFileExist = (HANDLE)respond;

   respond = MFS_CREATE(new, O_WRONLY);
   if (respond < 0)
     {
       MFS_CLOSE(hFileExist);
       return MFS_FAILURE;
     }
   else
     hFileNew = (HANDLE)respond;

   buffer = WinMalloc(512);
   if (!buffer)
     {
       MFS_CLOSE(hFileNew);
       MFS_CLOSE(hFileExist);
       return MFS_FAILURE;
     }

   while ((count = MFS_READ(hFileExist, buffer, 512)))
     MFS_WRITE(hFileNew, buffer, 512);

   WinFree(buffer);
   MFS_CLOSE(hFileNew);
   MFS_CLOSE(hFileExist);

   return MFS_SUCCESS;
}

/*********************************************************************/
DWORD 
mfs_mkdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	char	*address;
	int	 mode;
        char 	 fname[256];

	address = (LPSTR)p1;
	mode = (int)p2;

        MFS_ALTNAME((LPSTR)fname, (LPSTR)address);

        if (mkdir(fname , mode) == -1) 
           return MAKELONG(ACCESS_DENIED & 0xffff,0xffff);

        return MFS_SUCCESS;
}

/*********************************************************************/
DWORD 
mfs_chdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	char *pathname;

        int  drive;
        char dir[256];
        DEVICE *dev;
   	DWORD status = MFS_FAILURE;

	pathname = (LPSTR)p1;
        /*
        **   Get full DOS path, including drive letter. Then scan
        **   throught the drives list to find cwd to replace.
        */
        MFS_ALTNAME(dir, pathname);
        drive = xdos.currentdrive;
        dev = xdos.drivemap[drive];
        if (dev)
           {
           status = chdir(dir);
           if (status == 0) /* successful chdir */
              {
              getcwd(dir, sizeof(dir));
              if (dev->cwd != NULL)
                 WinFree(dev->cwd);
              dev->cwd = (LPSTR)WinMalloc(strlen(dir)+1);
              strcpy(dev->cwd, dir);
              }
           }
        return status;
}

/*********************************************************************/
DWORD 
mfs_getcwd(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int	drive;         /* p1 */
	LPSTR	lpszPathName;  /* p2 */
	int	len;           /* p3 */

        char    dir[256];
        DEVICE *dev;

	drive = (int)p1;
	lpszPathName = (LPSTR)p2;
	len = (int)p3;

        if (drive == 0) 
           drive = xdos.currentdrive;
        dev = xdos.drivemap[drive];
        if (dev)
           {
           if (dev->cwd == NULL)
              {
              getcwd(dir, sizeof(dir));
              dev->cwd = (LPSTR)WinMalloc(strlen(dir)+1);
              strcpy(dev->cwd, dir);
              }
           strncpy(lpszPathName, dev->cwd, len);
           return MFS_SUCCESS;
           }

        *lpszPathName = '\0';
        return MFS_FAILURE;
}

/*********************************************************************/
DWORD 
mfs_opendir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR pathname;
        char  dir[256];

	pathname = (LPSTR)p1;

        MFS_ALTNAME(dir, pathname);

        return (DWORD)opendir(dir);
}

/*********************************************************************/
DWORD 
mfs_readdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int dirp; 
        struct dirent *dp;

	dirp = (int)p1;

        dp = readdir((DIR *)dirp);
        if (dp) 
           {
           return (DWORD)(LPSTR)dp->d_name;
           }
        return MFS_SUCCESS;
}

/*********************************************************************/
DWORD 
mfs_closedir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int  dirp;

	dirp = (int)p1;

        return (DWORD)closedir((DIR*) dirp);
}

/*********************************************************************/
DWORD 
mfs_splitpath(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   DWORD status = MFS_FAILURE;


   return status;
}

/*********************************************************************/
DWORD 
mfs_makepath(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
   DWORD status = MFS_FAILURE;


   return status;
}


/*********************************************************************/
DWORD 
mfs_rmdir(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR	address; /* p1 */
        char	fname[256];

	address = (LPSTR)p1;

        MFS_ALTNAME((LPSTR)fname, (LPSTR)address);

        if (rmdir(fname) == -1) 
           {
           return MAKELONG(ACCESS_DENIED & 0xffff,0xffff);
           }
        return MFS_SUCCESS;
}


/*********************************************************************/
int GetPathType(LPSTR path)
{
    if (strchr(path, ':') || strchr(path, '\\'))
       return MFS_PATH_DOS;
    else return MFS_PATH_UNIX;
}

/*********************************************************************/
DWORD mfs_filedrive(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
  LPSTR path; /* p1 */

  path = (LPSTR) p1;
  if (!path)
    {
      return xdos.currentdrive;
    }

  return GetDriveFromPath(path);
}

/*********************************************************************/
int GetDriveFromPath(LPSTR path)
{
	char	DosPath[_MAX_PATH];

	mfs_config(XMFS_CFG_DOSPATH, 0L, (DWORD)(LPSTR)DosPath, (DWORD)(LPSTR)path);

#ifndef NETWARE
	if (DosPath[1] != ':' || DosPath[0] < 'A' || DosPath[0] > 'Z')
	   return xdos.currentdrive;
	else return toupper(DosPath[0]) - 'A' + 1;
#else
	{
	register i;
	char     Drive[32];
	char    *lpsz;

	/*   Extract Drive portion of drive:/dir/file.ext
	*/
	if ((lpsz = strchr(DosPath, ':')) != NULL)
	   {
	   strncpy(Drive, DosPath, lpsz-&DosPath[0]);
	   Drive[lpsz-&DosPath[0]] = '\0';
	   /*
           **   Now look up xdos.drivemap for Drive match...
	   */
	   for (i = 0;  i < 31;  i++)
               {
               if (xdos.drivemap[i] != NULL)
                  if (strcmp(Drive, xdos.drivemap[i]->device) == NULL)
		     return i;
               }
	   }
	return xdos.currentdrive;
	}
#endif
}



/*********************************************************************/
int GetDriveFromHandle(DWORD Handle)
{
	int nDrive;

        if (xdos.fileptr[Handle])
           {
           nDrive = xdos.fileptr[Handle]->drive;
           if (!nDrive) /* ZERO means current drive ?! */
              nDrive = xdos.currentdrive;
           return nDrive;
           }
	else {
             return xdos.currentdrive;
             }
}

/*********************************************************************/
char * DriveMapping(int drive)
{
   if (drive < 0 || drive > 32 || xdos.drivemap[drive] == NULL)
      return NULL;

   if (drive)
      return xdos.drivemap[drive]->name;
   else return xdos.drivemap[xdos.currentdrive]->name;
}

/*********************************************************************/
DWORD mfs_diskfree(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
  int drive;       /* p1 */
  LPDISKFREE lpdf; /* p2 */

  drive = (int)p1;
  lpdf = (LPDISKFREE)p2;

  if (!disk_free(drive, &lpdf->free_clusters, &lpdf->total_clusters,
		 &lpdf->sector_size, &lpdf->sectors_per_cluster))
    return MFS_SUCCESS;
  return MFS_FAILURE;
}
