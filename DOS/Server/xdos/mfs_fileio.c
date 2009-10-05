/*    
	mfs_fileio.c	1.52
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

*	mfs file operations interface

 */

#include "platform.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>  /* unlink() */
#include <sys/types.h>

#include <fcntl.h>
#include <utime.h>

#include <time.h>

#include "windows.h"
#include "Log.h"
#include "mfs_fileio.h"
#include "mfs_config.h"
#include "mfs_core.h"
#include "dosdefn.h"
#include "string.h"
#include "compat.h"
#include "Driver.h"

extern  CONFIG xdos;

static long mfs_openfile(char *, int, char *);
unsigned long seconds(unsigned int, unsigned int);

#define TM_TIME(ft)	((((ft)->tm_hour<<11) & 0xF800) | \
			(((ft)->tm_min<<5) & 0x07E0) | \
			(((ft)->tm_sec>>1) & 0x001F))

#define TM_DATE(ft)	(((((ft)->tm_year - 80)<<9) & 0xFE00) | \
			((((ft)->tm_mon+1)<<5) & 0x01E0) | \
			(((ft)->tm_mday) & 0x001F))

extern int GetCompatibilityFlags(int);

/*********************************************************************/
LPMFSFILE
checkhandle(int handle, int usage)
{
    	LPMFSFILE   fp;

	if (handle > 0 && handle < xdos.nfiles) 
           {
	   fp = xdos.fileptr[handle];
	   if (fp)
              if (fp->usage == usage)
                  return fp;
           }

	return NULL;
}

/*********************************************************************
**
**   Utility to get a handle to point to an MFSFILE
**
*********************************************************************/
int
gethandle()
{
	int handle;

	for (handle = 5;   handle < xdos.nfiles;   handle++) 
            {
	    if (xdos.fileptr[handle] == 0) 
	       break;
	    }

	if (handle == xdos.nfiles)
	   return -1;

	return handle;
}
	
/*********************************************************************/
int
freeindex(int nclosed)
{
static  int   index;
        int   freehandle;
        int   nextfree;
        int   handle;
 
        if (nclosed == 0)
           return 0;
 
        freehandle = 0;
        nextfree   = 0;
        for (handle = 0;   handle < xdos.nfiles;   handle++)
            {
            if (xdos.fileio[handle].usage == CLOSED)
               {
               /*   Get at least the first one...
               */
               if (freehandle == 0)
                  {
                  freehandle = handle;
                  continue;
                  }
               /*
               **   Find first one past last one
               */
               if (nextfree == 0 && handle > index)
                  {
                  nextfree = handle;
                  break;
                  }
               }
            }
 
        if (nextfree)
           freehandle = nextfree;
        index = freehandle;
 
        if (freehandle)
           closefile(&xdos.fileio[freehandle]);
 
        return freehandle;
}
 
/*********************************************************************
**
**   create file, and open for read/write (mode & 1)
**	          and open for write only (mode & 1) == 0
**
**    r   open for reading
**    w   truncate or create for writing
**    a   append: open for writing at end of file or create for writing
**    r+  open for update (reading and writing)
**    w+  truncate or create for update
**    a+  append; open or create for update at EOF
**
*********************************************************************/
DWORD
mfs_create(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR   filename; /* p1 */
	int     mode;     /* p2 */
    	char   *openmask = "";

	filename = (LPSTR)p1;
	mode = (int)p2;
	openmask = "w+b";
#ifdef LATER
	change to if it succeeds, then set filemode...
#endif
	return mfs_openfile(filename, mode, openmask);
}

/*********************************************************************/
DWORD
mfs_delete(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR filename;  /* p1 */
	char  fname[256];

	filename = (LPSTR)p1;
	mfs_config(XMFS_CFG_ALTNAME, 0L, (DWORD)(LPSTR)fname, (DWORD)(LPSTR)filename);

	return unlink(fname);
}


/*********************************************************************
** 
**   open file, and open for read/write
**	        and open for write only
**		and open for read only
**
*********************************************************************/
DWORD 
mfs_open(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	LPSTR   filename;  /* p1 */
	int     mode;      /* p2 */
	char   *openmask;

	filename = (LPSTR)p1;
	mode = (int)p2;

    	switch(mode & 3) 
           {
	   case O_RDONLY:		
              openmask = "rb"; 
              break;

	   case O_RDWR:
	   case O_WRONLY:	
              openmask = "r+b";
              break;

	   default:
	      ERRSTR((LF_ERROR,"mfs_open: bad file open mode %x\n", mode));
	      return MAKELONG(INVALID_FUNCTION & 0xffff,0xffff);
    	   }

	return mfs_openfile(filename, mode, openmask);
}

/*********************************************************************/
long
mfs_opencreate(char *filename, int mode, int attr, int action)
{
	long result;
	int  create;
	char *openmask = "";

    	switch(mode & 3) 
           {
	   case O_RDONLY:		
              openmask = "rb"; 
              break;

	   case O_WRONLY:		
              openmask = "wb"; 
              break;

	   case O_RDWR:	
              openmask = "r+b";
              break;
           }
	result = mfs_openfile(filename, mode, openmask);
	create = 0;

	switch(action) 
           {
	   case 0x10: /* create new file        */
	      if (result < 0)
		 create++;
	      break;	

	   case 0x01: /* fail if does not exist */
	   case 0x02: /* open and truncate, fail otherwise */
	      break;
	}

	return result;
}

/*********************************************************************/
static int
swapfile(LPMFSFILE fp)
{
	return 0;
}

/*********************************************************************/
long
mfs_openfile(char *filename, int mode, char *openmask)
{
	char	altname[_MAX_PATH];
	int	slot,handle,freeslot;
	extern  int errno;
	int     drive;
	int	fd;
	int	nclosed;
	struct  stat filestat;
    	LPMFSFILE   fp;


	freeslot = 0;
	nclosed  = 0;

	for (slot = 5;   slot < xdos.nfiles;   slot++) 
            {
            /*   See if this file has been CLOSED 
            */
	    if (mode == 0 && xdos.fileio[slot].usage == CLOSED) 
               {
	       if (strcmp(filename, xdos.fileio[slot].filename) == 0)
                  {
		  if ((handle = gethandle()) < 0) 
                     {
		     ERRSTR((LF_ERROR,"mfs_openfile cannot get handle for file %s %d %d\n", filename, errno, slot));
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
		  mfs_seek(handle, 0L, 0L, 0L);
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
	   ERRSTR((LF_ERROR,"mfs_openfile: error no more handles filename %s err=%d\n", filename, errno));
	   return MAKELONG(NO_HANDLES & 0xffff,0xffff);
	   }
	/* 
 	**   Get the real unix name for now,
 	**   must add mfs_parser here...
	**   (drive is the currentdrive, or drive prefix) 
	*/
	drive = mfs_config(XMFS_CFG_ALTPATH, 0L, (DWORD)altname, (DWORD)filename);
	/* 
        **   This is the file slot to use for this new file entry 
        */
	slot = freeslot;
        fp = &xdos.fileio[slot];
    	fp->fs = fopen(altname,openmask);

	if (fp->fs == 0) 
           {
	   if (swapfile(fp) == 0) 
              {
	         if(GetCompatibilityFlags(0) & WD_FILENOTFOUND)
	         	ERRSTR((LF_ERROR,"Open failed on file: '%s'\n",altname));
	         return MAKELONG(convert_error(errno) & 0xffff,0xffff);		
	      }
	   }
	/* 
	**   Assign a filepointer to this file struct 
	*/
	xdos.fileptr[handle] = fp;
	fd = fileno(fp->fs);
	fstat(fd, &filestat);
	/* don't do a strdup() here, since strdup() will malloc the memory for the string */
	/* we need to use WinMalloc() here for portability */
	fp->filename = WinMalloc(strlen(filename) + 1);
	strcpy(fp->filename, filename);
	fp->usage    = OPENED;
	fp->mode     = mode;
	fp->type     = MSF_FILE;
	fp->drive    = drive;
	fp->fsize    = filestat.st_size;
	fp->refcnt   = 1;
	fp->atime    = 0;
	fp->adate    = 0;
	fp->handle   = slot;

	return handle;
}

/*********************************************************************/
DWORD 
mfs_flush(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;  /* p1 */

    	MFSFILE *fp;
    	int ret;

	handle = (int)p1;

    	fp = checkhandle(handle,OPENED);
    	if (fp) {
	   ret = fflush(fp->fs);
	if (ret == 0)
	    return MFS_SUCCESS;
    }

    return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
int
closefile(MFSFILE *fp)
{
	int rc = fclose(fp->fs);

	fp->fs     = 0;
	fp->handle = 0;    /* FAT specific */
	fp->dos    = -1;   /* FAT specific */
	fp->usage  = FREE;
	WinFree(fp->filename);

	return rc;
}

/*********************************************************************/
DWORD 
mfs_close(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;   /* p1 */
	MFSFILE *fp;

	handle = (int)p1;

	fp = checkhandle(handle, OPENED);
        if (fp == NULL) {
           fp = checkhandle(handle, CLOSED); 
	}

        if (fp != NULL)
           {
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
	else ERRSTR((LF_ERROR,"mfs_close: invalid handle %d\n",handle));

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
mfs_touch(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int	handle;        /* p1 */
	int	func;          /* p2 */
	unsigned int   *atime; /* p3 */
	unsigned int   *adate; /* p4 */
	MFSFILE        *fp;
        unsigned long   update;
#ifndef NETWARE
        struct utimbuf  modtimes;
#endif

	handle = (int)p1;
	func = (int)p2;
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
mfs_read(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;  /* p1 */
	LPSTR address; /* p2 */
	int   count;   /* p3 */
	MFSFILE *fp;
	long cnt;

	handle = (int)p1;
	address = (LPSTR)p2;
	count = (int)p3;

	if (address == NULL)
	   return MFS_SUCCESS;

	fp = checkhandle(handle, OPENED); 
	if (fp)
           {
	   cnt = fread(address,1,count,fp->fs);			
	   return (DWORD)cnt;
	   }
	ERRSTR((LF_ERROR,"mfs_read: bad file handle\n"));
	return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
DWORD 
mfs_write(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int   handle;   /* p1 */
	LPSTR address;  /* p2 */
	int   count;    /* p3 */
	MFSFILE *fp;
	long cnt;

	handle = (int)p1;
	address = (LPSTR)p2;
	count = (int)p3;

	fp = checkhandle(handle, OPENED); 
	if (fp)
           {
	   fp->dirty = 1;
	   if (count == 0) 
              {
		ftruncate(fileno(fp->fs), ftell(fp->fs));
	        cnt = 0;
	      }
	   else cnt = fwrite(address, 1, count, fp->fs);			
	   return (DWORD)cnt;
	   }
	ERRSTR((LF_ERROR,"mfs_write: bad file handle\n"));
	return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
DWORD 
mfs_seek(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int	handle;    /* p1 */
	int	distance;  /* p2 */
	int	mode;      /* p3 */
	long   *location;  /* p4 */
	long	offset;
	long	curloc;
	MFSFILE   *fp;

	handle = (int)p1;
	distance = (int)p2;
	mode = (int)p3;
	location = (long *)p4;

	fp = checkhandle(handle, OPENED); 
	if (fp)
           {
	   curloc = ftell(fp->fs);
	   fseek(fp->fs,distance,mode);
	   offset = ftell(fp->fs);

	   if (location) 
	      *location = offset;
	   return MFS_SUCCESS;
	   }
	ERRSTR((LF_ERROR,"mfs_seek: bad file handle\n"));
	return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*********************************************************************/
DWORD 
mfs_ioctl(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
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

        if (handle<=2) {
	  switch(cmd) {
	      case MFS_IOCTL_GETINFO:
		 if (handle==0)
		   info = 0x80 + 0x01;    /* ISDEV|ISCIN */
		 if (handle==1)
		   info = 0x80 + 0x02;    /* ISDEV|ISCOT */
		 return (DWORD)info;
	  }
	}

	/* the following are *drive* number dependant and not
	 * *file handle* dependant as they are in the current
	 * code base
	 */
	switch (cmd) {
	      /* function 8 */
	      case MFS_IOCTL_ISREMOVABLE:
		 return 1L;

	      /* function 9/10 */
	      case MFS_IOCTL_ISREMOTE:
		 if (attributte)
		    *attributte = 0;
		 return 0L;
	}

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

	      default:
		 break;
	      }
	   }
	ERRSTR((LF_ERROR,"mfs_ioctl: bad file handle %d\n",handle));
	return MAKELONG(INVALID_HANDLE & 0xffff,0xffff);
}

/*
 *	mfs_fnctl
 *		file system related routines
 *		handleinfo  get/set number of filehandles
 *		duphandle   map/dup a handle to another
 */
/*********************************************************************/
DWORD 
mfs_fcntl(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
        int     handle;    /* p1 */
        int     msg;       /* p2 */
        int     wparam;    /* p2 */
        int     lparam;    /* p3 */

	int	 rc = 0;
	int	 new;
    	MFSFILE *fp;
				
	handle = (int)p1;
	msg    = (int)p2;
	wparam = (int)p3;
	lparam = (int)p4;

	switch(msg) 
           {
	   case	MFS_FCNTL_HANDLEINFO:
		rc = mfs_config(XMFS_CFG_NFILES, 0L, 0L, 0L);
		if (wparam) 
		   {
		   mfs_config(XMFS_CFG_NFILES, 1L, 0L, (DWORD)(LPSTR)lparam);
		   xdos.fileio = (MFSFILE *) WinRealloc((char *)xdos.fileio,sizeof(MFSFILE) * lparam);
		   xdos.fileptr = (MFSFILE **) WinRealloc((char *)xdos.fileptr,sizeof(MFSFILE *) * lparam);
		   }
		break;						

	   case	MFS_FCNTL_DUPHANDLE:
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

DWORD 
mfs_finfo(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
	int     handle;    /* p1 */
	struct  stat filestat;
	struct  tm *filetime;
    	LPMFSFILE   fp = (LPMFSFILE)NULL;

        handle = (int)p1;

	/* 
	**   Get the filepointer to file struct 
	*/
	if (handle > 0 && handle < xdos.nfiles) {
	   fp = xdos.fileptr[handle];
	   if ( fp->fs) {
		fstat(fileno(fp->fs), &filestat);
		fp->fsize    = filestat.st_size;
		filetime     = localtime(&filestat.st_mtime);
		fp->atime    = TM_TIME(filetime);
		fp->adate    = TM_DATE(filetime);
	    }
	}

	return (DWORD)fp;
}


DWORD
mfs_fileinfo(DWORD p1, DWORD p2, DWORD p3, DWORD p4)
{
    DWORD file_handle;

    if (p1 == 0)
        file_handle = fileno((xdos.fileptr[p2])->fs);
    else
        file_handle = 0;
       
    /*
     *  Just pass this call in to the native driver code.
     *  It will fill in the structure appropriately.
     */
    return DRVCALL_FILES(PFH_GETFILEINFO, p1, file_handle, p3);
}


