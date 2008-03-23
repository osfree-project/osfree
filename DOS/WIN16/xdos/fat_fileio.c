/*    
	fat_fileio.c	1.18
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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <utime.h>

#include "windows.h"
#include "kerndef.h"
#include "Log.h"

#include "fat_tools.h"
#include "mfs_config.h"
#include "mfs_core.h"
#include "dosdefn.h"

extern CONFIG xdos;
extern int DiskHandle;
extern int DosFlag;

static int doshandle(int, int);

unsigned long seconds(unsigned int, unsigned int);
int     dos_open(char *, WORD);
int     dos_close(int);
int     dos_read(int, char *, int);
int     dos_write(int, char *, int);
int     dos_lseek(int, long, int);
int     WriteDirEntry(struct DirEntry *, long);
int     DosExist(char *, struct DirEntry *, long *);
WORD    GetWord(BYTE *);
int     GetNextCluster(WORD);
int     SetNextCluster(WORD, WORD);

/*********************************************************************/
DWORD
fat_create(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR   filename; /* p1 */
	int     mode;     /* p2 */

	char    Path[_MAX_PATH];

	filename = (LPSTR)p1;
	mode = (int)p2;

	MFS_DOSPATH(Path, filename);

	mode |= 0x0100; /* _O_CREAT */
	return fat_open((DWORD)Path, (DWORD)mode, 0L, 0L);
}

/*********************************************************************/
DWORD
fat_delete(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR  filename;  /* p1 */
	char   Path[_MAX_PATH];
        int    temp;
        long   EntryAddress;
        struct DirEntry Dir;

	filename = (LPSTR)p1;
	MFS_DOSPATH(Path, filename);

        DosFlag = 0;
        if (DosExist(Path, &Dir, &EntryAddress) < 0) 
	   {
           DosFlag = (PATH_NOT_FOUND | FAT_CARRY_FLAG);
           return MFS_FAILURE;
           }

        if (Dir.Attrib & DIRECTORY) 
	   {
           DosFlag = (FILE_NOT_FOUND | FAT_CARRY_FLAG);
           return MFS_FAILURE;
           }

        if (Dir.Attrib & READ_ONLY) 
	   {
           DosFlag = (ACCESS_DENIED | FAT_CARRY_FLAG);
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
fat_open(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR   filename;  /* p1 */
	int     mode;      /* p2 */
	char    Path[_MAX_PATH];
        int     DosHandle;

        int     slot,handle,freeslot;
        extern  int errno;
        int     nclosed;
        LPMFSFILE   fp;
	DWORD   DosDrive;
 

	filename = (LPSTR)p1;
	mode = (int)p2;

	DosDrive = MFS_DOSPATH(Path, filename);

if (!TheSameDisk(DiskHandle))
   DiskInit(DriveMapping(DosDrive));

        DosHandle = dos_open(Path, mode);
	if (DosHandle < 0)
           {
           return DosHandle;
           }

	freeslot = 0;
        nclosed = 0;
 
        for (slot = 5;   slot < xdos.nfiles;   slot++)
            {
            /*   See if this file has been CLOSED
            */
            if (mode == 0 && xdos.fileio[slot].usage == CLOSED)
               {
               if (strcmp(Path, xdos.fileio[slot].filename) == 0)
                  {
                  if ((handle = gethandle()) < 0)
                     {
                     return MAKELONG(NO_HANDLES & 0xffff,0xffff);
                     }
                  /*
                  **   Handle is new handle for slot
                  */
                  xdos.fileptr[handle] = &xdos.fileio[slot];
                  /*
                  **   It is now OPEN, so seek to 0
                  **   ie. we opened the file, ie. rewind
                  */
                  xdos.fileio[slot].usage = OPENED;
		  xdos.fileio[slot].dos = DosHandle; /* That's what I forgot to do!!! */
                  fat_seek(handle, 0L, 0L, 0L);
                  xdos.fileio[slot].refcnt = 1; /* it has a user... */
                  return handle;
                  }
               }
            if (xdos.fileio[slot].usage == CLOSED)
               nclosed++;
 
            if (freeslot == 0 && xdos.fileio[slot].usage == FREE)
               freeslot = slot;
            }
        /*
        **   Do we have a freeslot?
	*/
        if (freeslot == 0)
           freeslot = freeindex(nclosed);
 
        handle = gethandle();
 
        if (handle < 0 || freeslot == 0)
           {
           return MAKELONG(NO_HANDLES & 0xffff,0xffff);
           }
        /*
        **   This is the file slot to use for this new file entry
        */
        slot = freeslot;
        fp = &xdos.fileio[slot];
        /*
        **   Assign a filepointer to this file struct
        */
        xdos.fileptr[handle] = fp;
        fp->fs       = NULL;
        fp->filename = (char *)WinStrdup(Path);
        fp->usage    = OPENED;
        fp->mode     = mode;
        fp->type     = MSF_FILE;
        fp->drive    = DosDrive;
        fp->fsize    = 100; /* STUB -- filestat.st_size;*/
        fp->refcnt   = 1;
        fp->atime    = 0;
        fp->adate    = 0;
        fp->handle   = slot;
	fp->dos      = DosHandle;
 
        return handle;
}

/*********************************************************************/
DWORD 
fat_flush(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;  /* p1 */
    	int   ret = 0;

	handle = (int)p1;
/*******
	dos = doshandle(handle, OPENED);
	if (dos != -1)
	   ret = fflush(dos);   fflush accepts STREAM, not HANDLE *******/
	if (ret == 0) 
	    return MFS_SUCCESS;

    return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
DWORD 
fat_close(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int       handle;   /* p1 */
        LPMFSFILE fp;

	handle = (int)p1;

	fp = checkhandle(handle, OPENED);
	if (fp)
	   {
	   dos_close(fp->dos);
	   if (fp->refcnt > 0)
	      fp->refcnt--;
	   if (fp->refcnt == 0)
	      {
	      if (fp->mode == 0)
		 {
		 fp->usage = CLOSED;
		 xdos.fileptr[handle] = 0;
		 return MFS_SUCCESS;
		 }
	      xdos.fileptr[handle] = 0;
	      return (DWORD)closefile(fp);
              }
           return MFS_SUCCESS;
           }
        /*
        **    Check if the handle is reserved, and return ok
        **    otherwise return error?
        */
        if (handle < 5)
           return MFS_SUCCESS;

        return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
DWORD 
fat_touch(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
	int	func;          /* p1 */
	int	handle;        /* p2 */
	unsigned int   *atime; /* p3 */
	unsigned int   *adate; /* p4 */
	MFSFILE        *fp;
        unsigned long   update;
#ifndef NETWARE
        struct utimbuf  modtimes;
#endif

	func = (int)p1;
	handle = (int)p2;
	atime = (unsigned int *)p3;
	adate = (unsigned int *)p4;
 
	fp = checkhandle(handle,OPENED);
	if(fp) {
		if(func) {
			fp->atime = *atime;
			fp->adate = *adate;
                        update = seconds (*adate, *atime);
#ifndef NETWARE
                        modtimes.actime  = update;
                        modtimes.modtime = update;
			utime(fp->filename,&modtimes);
#endif

		} else {
			/* if atime == 0 && adate == 0 */
			/* then get atime/adate...     */
			*atime = fp->atime;
			*adate = fp->adate;
		}

		return MFS_SUCCESS;
	}
	return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
DWORD 
fat_read(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;  /* p1 */
	LPSTR address; /* p2 */
	int   count;   /* p3 */
	int   cnt, dos;

	handle = (int)p1;
	address = (LPSTR)p2;
	count = (int)p3;
	dos = doshandle(handle, OPENED);
	cnt = dos_read(dos, address, count);
	return (DWORD)cnt;
}

/*********************************************************************/
DWORD 
fat_write(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;   /* p1 */
	LPSTR address;  /* p2 */
	int   count;    /* p3 */
	int   cnt, dos;

	handle = (int)p1;
	address = (LPSTR)p2;
	count = (int)p3;
	dos = doshandle(handle, OPENED);
	cnt = dos_write(dos, address, count);
	return (DWORD)cnt;
}

/*********************************************************************/
DWORD 
fat_seek(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int	handle;    /* p1 */
	int	distance;  /* p2 */
	int	mode;      /* p3 */
	long   *location;  /* p4 */
	long	offset;
	int	dos;

	handle = (int)p1;
	distance = (int)p2;
	mode = (int)p3;
	location = (long *)p4;
	dos = doshandle(handle, OPENED);

	offset = dos_lseek(dos, distance, mode);

	if (location) 
	   *location = offset;

	return MFS_SUCCESS;
}

/*********************************************************************/
DWORD 
fat_ioctl(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
	int     handle;    /* p1 */
        int     cmd;       /* p2 */
        int     mode;      /* p3 */
        long   *attributte;/* p4 */
	int	info = 0;
	MFSFILE *fp;

        handle = (int)p1;
        cmd = (int)p2;
        mode = (int)p3;
        attributte = (long *)p4;

	fp = checkhandle(handle, OPENED);
	if (fp)
           {
	   switch(cmd) 
              {
	      /* function 0 */
	      case MFS_IOCTL_GETINFO:
		 if (fp->type == MSF_FILE) 
                    {
		    info = fp->drive;
		    info |= (fp->dirty) ? 0x40 : 0;
		    }
		 if (fp->type == MSF_DEVICE) 
                    {
		    info = 0x80 + 0x20;
		    }
		 return (DWORD)info;

	      /* function 1 */
	      case MFS_IOCTL_SETINFO:
		 return 0L;

	      /* function 6 */
	      case MFS_IOCTL_STATUS:
		 return 0xFFFFFFFF;

	      /* function 7 */
	      case MFS_IOCTL_READY:
		 return 0xFFFFFFFF;

	      /* function 8 */
	      case MFS_IOCTL_ISREMOVABLE:
		 return 1L;

	      /* function 9/10 */
	      case MFS_IOCTL_ISREMOTE:
		 if (attributte)
		    *attributte = 0;
		 return 0;

	      default:
		 break;
	      }
	   }
	return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*
 *	fat_fnctl
 *		file system related routines
 *		handleinfo  get/set number of filehandles
 *		duphandle   map/dup a handle to another
 */
/*********************************************************************/
DWORD 
fat_fcntl(DWORD p1, DWORD p2, DWORD p3, DWORD p4) /* %%% */
{
        int     handle;    /* p1 */
        int     msg;       /* p2 */
        int     wparam;    /* p2 */
        int     lparam;    /* p3 */

        int      rc = 0;
        int      new;
        MFSFILE *fp;

        handle = (int)p1;
        msg    = (int)p2;
        wparam = (int)p3;
        lparam = (int)p4;

        switch(msg)
           {
           case MFS_FCNTL_HANDLEINFO:
                rc = mfs_config(XMFS_CFG_NFILES, 0L, 0L, 0L);
                if (wparam)
                   {
                   mfs_config(XMFS_CFG_NFILES, 1L, 0L, (DWORD)(LPSTR)lparam);
                   xdos.fileio = (MFSFILE *) WinRealloc((char *)xdos.fileio,sizeof(MFSFILE) * lparam);
                   xdos.fileptr = (MFSFILE **) WinRealloc((char *)xdos.fileptr,sizeof(MFSFILE *) * lparam);
                   }
                break;

           case MFS_FCNTL_DUPHANDLE:
                /*
                **   Duplicate the handle...
                **   Do we have one to use? If so free it...
                */
                if (lparam == -1)
                   {
                   /*   Find a alternate handle to use...
                   **   'new' has the new handle
                   */
                   for (new = 5;   new<xdos.nfiles;   new++)
                        {
                        if (xdos.fileptr[new] == 0)
                           break;
                        }
                   if (new == xdos.nfiles)
                      return MAKELONG(errno & 0xffff,0xffff);
                   /*
                   **   'new' is a free slot
                   */
                   }
                else {
                     /*   Close the handle, if opened
                     */
                     new = lparam;
                     fp = checkhandle(new,OPENED);
                     if (fp)
                        mfs_close((DWORD)new, 0L, 0L, 0L);
                     /*
                     **   'new' is now available to us;
                     **   'new' MAY be one of our reserved entries!
                     */
                     }
                /*
                **   Set newhandle to handle...
                */
                xdos.fileptr[new] = &xdos.fileio[handle];
                /*
                **   Increment the reference count of the file
                */
                xdos.fileio[handle].refcnt++;
                return (DWORD)new;
           }

	return (DWORD)rc;
}


/*********************************************************************/
static int doshandle(int handle, int usage)
{
        LPMFSFILE   fp;
 
	if (handle > 0 && handle < xdos.nfiles)
           {
           fp = xdos.fileptr[handle];
	   if (fp)
	      if (fp->usage == usage)
                 {
	         return fp->dos;
                 }
	   }
  	return -1;
}



