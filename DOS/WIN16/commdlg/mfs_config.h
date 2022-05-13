/*************************************************************************

	@(#)mfs_config.h	1.25 MFS configuration management.
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

**************************************************************************/
 
#ifndef mfs_config__h
#define	mfs_config__h

#include <stdio.h>

#ifdef NETWARE
#define KERNEL /* to get rid of mkdir, opendir, etc. in sys/stat.h - 
                  they are defined in NW's direct.h */
#endif

#include <sys/stat.h>

#ifdef NETWARE
#undef KERNEL
#endif

typedef DWORD (*MFSPROC) (DWORD, DWORD, DWORD, DWORD);
/*
**   MFS completion codes
*/
#define MFS_FAILURE   0xFFFFFFFF
#define MFS_SUCCESS   0L
/* 
**   MFS hooks 
*/
#define MFSH_INIT        0x0000
#define MFSH_FIRSTHOOK   0x0001
#define MFSH_CONFIG      0x0001
#define MFSH_STAT        0x0002
#define MFSH_ACCESS      0x0003
#define MFSH_OPEN        0x0004
#define MFSH_CLOSE       0x0005
#define MFSH_READ        0x0006
#define MFSH_WRITE       0x0007
#define MFSH_SEEK        0x0008
#define MFSH_FLUSH       0x0009
#define MFSH_CREATE      0x000A
#define MFSH_DELETE      0x000B
#define MFSH_RENAME      0x000C
#define MFSH_MKDIR       0x000D
#define MFSH_CHDIR       0x000E
#define MFSH_GETCWD      0x000F
#define MFSH_REGEXP      0x0010
#define MFSH_OPENDIR     0x0011
#define MFSH_READDIR     0x0012
#define MFSH_CLOSEDIR    0x0013
#define MFSH_FCNTL       0x0014
#define MFSH_IOCTL       0x0015
#define MFSH_FINDFIRST   0x0016
#define MFSH_FINDNEXT    0x0017
#define MFSH_FINDFILE    0x0018
#define MFSH_RMDIR       0x0019
#define MFSH_TOUCH       0x001A
#define MFSH_SPLITPATH   0x001B
#define MFSH_MAKEPATH    0x001C
#define MFSH_FINFO       0x001D
#define MFSH_COPY        0x001E
#define MFSH_FILEDRIVE   0x001F
#define MFSH_DISKFREE    0x0020
#define MFSH_FILEINFO    0x0021
#define MFSH_LASTHOOK    0x0021

/*
**   MFS Entry Point
*/
extern MFSPROC MFSProc[MFSH_LASTHOOK+1]; /* native handlers */
extern MFSPROC FATProc[MFSH_LASTHOOK+1]; /* FAT handlers */
DWORD MFS_CALL(UINT, DWORD, DWORD, DWORD, DWORD);
/*
**   MFS API Wrapper Macros
*/
#define MFS_INIT()              MFS_CALL(MFSH_INIT,     0L, 0L, 0L, 0L)
#define MFS_CONFIG()            MFS_CALL(MFSH_CONFIG,   0L, 0L, 0L, 0L)
#define MFS_STAT(p1,p2)         MFS_CALL(MFSH_STAT,     (DWORD)(p1), (DWORD)(p2), 0L, 0L)
#define MFS_ACCESS(p1,p2,p3)    MFS_CALL(MFSH_ACCESS,   (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_OPEN(p1, p2)        MFS_CALL(MFSH_OPEN,     (DWORD)(p1), (DWORD)(p2), 0L, 0L)
#define MFS_CLOSE(p1)           MFS_CALL(MFSH_CLOSE,    (DWORD)(p1), 0L, 0L, 0L)
#define MFS_READ(p1,p2,p3)      MFS_CALL(MFSH_READ,     (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_WRITE(p1,p2,p3)     MFS_CALL(MFSH_WRITE,    (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_SEEK(p1,p2,p3,p4)   MFS_CALL(MFSH_SEEK,     (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), (DWORD)(p4))
#define MFS_FLUSH(p1)           MFS_CALL(MFSH_FLUSH,    (DWORD)(p1), 0L, 0L, 0L)
#define MFS_CREATE(p1, p2)      MFS_CALL(MFSH_CREATE,   (DWORD)(p1), (DWORD)(p2), 0L, 0L)
#define MFS_DELETE(p1)          MFS_CALL(MFSH_DELETE,   (DWORD)(p1), 0L, 0L, 0L)
#define MFS_RENAME(p1,p2,p3)    MFS_CALL(MFSH_RENAME,   (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_COPYFILE(p1,p2,p3)  MFS_CALL(MFSH_COPY,     (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_MKDIR(p1,p2)        MFS_CALL(MFSH_MKDIR,    (DWORD)(p1), (DWORD)(p2), 0L, 0L)
#define MFS_CHDIR(p1)           MFS_CALL(MFSH_CHDIR,    (DWORD)(p1), 0L, 0L, 0L)
#define MFS_GETCWD(p1,p2,p3)    MFS_CALL(MFSH_GETCWD,   (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_REGEXP(p1, p2)      MFS_CALL(MFSH_REGEXP,   (DWORD)(p1), (DWORD)(p2), 0L, 0L)
#define MFS_OPENDIR(p1)         MFS_CALL(MFSH_OPENDIR,  (DWORD)(p1), 0L, 0L, 0L)
#define MFS_READDIR(p1)         MFS_CALL(MFSH_READDIR,  (DWORD)(p1), 0L, 0L, 0L)
#define MFS_CLOSEDIR(p1)        MFS_CALL(MFSH_CLOSEDIR, (DWORD)(p1), 0L, 0L, 0L)
#define MFS_FCNTL(p1,p2,p3,p4)  MFS_CALL(MFSH_FCNTL,    (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), (DWORD)(p4))
#define MFS_IOCTL(p1,p2,p3,p4)  MFS_CALL(MFSH_IOCTL,    (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), (DWORD)(p4))
//#define MFS_FINDFIRST(p1,p2,p3) MFS_CALL(MFSH_FINDFIRST,(DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_FINDNEXT(p1)        MFS_CALL(MFSH_FINDNEXT, (DWORD)(p1), 0L, 0L, 0L)
#define MFS_FINDFILE(p1,p2,p3)  MFS_CALL(MFSH_FINDFILE, (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)
#define MFS_RMDIR(p1)           MFS_CALL(MFSH_RMDIR,    (DWORD)(p1), 0L, 0L, 0L)
#define MFS_TOUCH(p1,p2,p3,p4)  MFS_CALL(MFSH_TOUCH,    (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), (DWORD)(p4))
#define MFS_FINFO(p1)           MFS_CALL(MFSH_FINFO,    (DWORD)(p1), 0L, 0L, 0L)
#define MFS_FILEDRIVE(p1)       MFS_CALL(MFSH_FILEDRIVE,(DWORD)(p1), 0L, 0L, 0L)
#define MFS_DISKFREE(p1,p2)     MFS_CALL(MFSH_DISKFREE, (DWORD)(p1), (DWORD)(p2), 0L, 0L)
#define MFS_FILEINFO(p1,p2,p3)	MFS_CALL(MFSH_FILEINFO, (DWORD)(p1), (DWORD)(p2), (DWORD)(p3), 0L)

/***#define MFS_SPLITPATH******/
/***#define MFS_MAKEPATH*******/

#define MFS_DOSNAME(p1,p2)      MFS_CALL(MFSH_CONFIG, XMFS_CFG_DOSNAME,  0L, (DWORD)(p1), (DWORD)(p2))
#define MFS_DOSPATH(p1,p2)      MFS_CALL(MFSH_CONFIG, XMFS_CFG_DOSPATH,  0L, (DWORD)(p1), (DWORD)(p2))
#define MFS_ALTNAME(p1,p2)      MFS_CALL(MFSH_CONFIG, XMFS_CFG_ALTNAME,  0L, (DWORD)(p1), (DWORD)(p2))
#define MFS_ALTPATH(p1,p2)      MFS_CALL(MFSH_CONFIG, XMFS_CFG_ALTPATH,  0L, (DWORD)(p1), (DWORD)(p2))
#define MFS_SETDRIVE(p1)        MFS_CALL(MFSH_CONFIG, XMFS_CFG_SETDRIVE, (DWORD)(p1), 0L, 0L)
#define MFS_GETDRIVE()          MFS_CALL(MFSH_CONFIG, XMFS_CFG_GETDRIVE, 0L, 0L, 0L)
#define MFS_GETBOOT()           MFS_CALL(MFSH_CONFIG, XMFS_CFG_GETBOOT,  0L, 0L, 0L)
#define MFS_NFILES(p1,p2)       MFS_CALL(MFSH_CONFIG, XMFS_CFG_NFILES,   (DWORD)(p1), (DWORD)(p2), 0L)
#define MFS_GETCONFIG()         MFS_CALL(MFSH_CONFIG, XMFS_CFG_GETCONFIG,0L, 0L, 0L)
#define MFS_DRIVETYPE(p1)       MFS_CALL(MFSH_CONFIG, XMFS_CFG_DRIVETYPE,(DWORD)p1, 0L, 0L)
#define MFS_SETMAXFH(p1)        MFS_CALL(MFSH_CONFIG, XMFS_CFG_SETMAXFH,(DWORD)p1, 0L, 0L)
#define MFS_GETDRIVEMAP(p1,p2,p3) MFS_CALL(MFSH_CONFIG, XMFS_CFG_GETDRIVEMAP, (DWORD)p1, (DWORD)p2, (DWORD)p3)
#define MFS_GETROOTDRIVE()      MFS_CALL(MFSH_CONFIG, XMFS_CFG_GETROOTDRIVE, 0L, 0L, 0L)

/* mfs config opcodes */
#define XMFS_CFG_INIT		0 
#define XMFS_CFG_DOSNAME	1
#define XMFS_CFG_DOSPATH	2
#define XMFS_CFG_ALTNAME	3
#define XMFS_CFG_ALTPATH	4
#define XMFS_CFG_SETDRIVE	5
#define XMFS_CFG_GETDRIVE	6
#define XMFS_CFG_GETBOOT	7
#define XMFS_CFG_NFILES		8
#define XMFS_CFG_GETCONFIG	9
#define XMFS_CFG_DRIVETYPE     10
#define XMFS_CFG_SETMAXFH      11
#define XMFS_CFG_GETDRIVEMAP   12
#define XMFS_CFG_GETROOTDRIVE  13

/* mfs_ioctl */
#define	MFS_IOCTL_GETINFO	0
#define	MFS_IOCTL_SETINFO	1
#define MFS_IOCTL_STATUS	2
#define MFS_IOCTL_READY		3
#define MFS_IOCTL_ISREMOVABLE	4
#define MFS_IOCTL_ISREMOTE	5

/* mfs_fcntl */
#define	MFS_FCNTL_HANDLEINFO	0
#define	MFS_FCNTL_DUPHANDLE	1

/* mfs path types */
#define MFS_PATH_UNIX           0
#define MFS_PATH_DOS            1
#define MFS_PATH_MAC            2
#define MFS_PATH_NOVELL         3

/* mfs drive types */
#define MFS_DRIVE_NATIVE        0
#define MFS_DRIVE_FAT           1
#define MFS_DRIVE_PSEUDO        2

/* mfs file attribs */
#define _A_NORMAL		0x00
#define _A_RDONLY		0x01
#define _A_HIDDEN		0x02
#define _A_SYSTEM		0x04
#define _A_VOLID		0x08
#define _A_SUBDIR		0x10
#define _A_ARCH			0x20

#define FREE            0
#define OPENED          1
#define CLOSED          2
#define SWAPPED         3
#define RESERVED       -1
#define MSF_FILE        0
#define MSF_DEVICE      1

/*
**   MFS/DOS support data structures...
*/
/*
**      mfs_xxxx correspond to native versions of:
**              open, close, read, write, ioctl...
**
**      on open, if we cannot open the file, ie. if we run out of
**      file descriptors, first, try to close a CLOSED file.  Then
**      SWAP a file, ie. it is closed, but not CLOSED, it will/can
**      be reopened when we need it.
**
**      on close, we CLOSE a file.  This means that we do not
**      close it immediately, but only when we need to.
**      On an open, check to see if it is CLOSED, if so reopen it,
**
**      all i/o will get the MFSFILE *, and use the device switch
**      table to handle the actual i/o routines.
**
*/
typedef struct _MFSFILE {
        int     handle;  /* slot                 */
        int     drive;   /* drive the file is on */
        int     type;    /* file/device/...      */
        int     usage;   /* open/locked          */
        int     mode;    /* mode file opened     */
        int     dirty;   /* is file dirty?       */
        int     refcnt;  /* how many have it open*/
        int     fsize;   /* file size at open    */
        LONG    atime;   /* access time          */
        LONG    adate;   /* access date          */
	int     dos;     /* DOS file handle     */
        void   *private; /* type specific        */
        char   *filename;/* file spec            */
        FILE   *fs;      /* fp from open         */
} MFSFILE, *LPMFSFILE;

typedef struct {
	int       drive;	/* the drive number 		      */
	int       type;		/* type of device, logical/physical   */
	char     *device;	/* the drive letter		      */
	char     *name;		/* pathname of root of drive (logical)*/
	char     *cwd;		/* normalized directory path on drive */
        MFSPROC  *lpfnDispatch; /* pointer to table with "native" handlers for that drive type */
} DEVICE, *LPDEVICE;

typedef struct {
	int	   dosmode;
	int	   memory;
	int	   extended;
	int	   nfiles;
	int	   equipment;
	int 	   currentdrive;
	int 	   lastdrive;
	int 	   logicaldrives;
	int    	   bootdrive;
	LPDEVICE   drivemap[32];
        LPMFSFILE  fileio;
	LPMFSFILE *fileptr;
} CONFIG, *LPCONFIG;


typedef struct {
	unsigned short  FileFlag;
	char 		FileName[256];
	unsigned long	FileTime;
	unsigned long	FileDate;
	unsigned long	FileSize;
	unsigned short	FileAttr;
	unsigned short	FileDrive;
	unsigned short  FileInfo;
} FFINFO, *LPFFINFO;

typedef struct {
        unsigned int    free_clusters;
        unsigned int    total_clusters;
        unsigned int    sector_size;
        unsigned int    sectors_per_cluster;
} DISKFREE, *LPDISKFREE;

#ifdef	LATER
typedef struct _FILEINFO
   {
   unsigned int   wFlags;      /* RESERVED                                             */
   unsigned int   wType;       /* Un*x, DOS, Mac, etc.                                 */
   int            nDrive;      /* 0- 'A', 1- 'B', etc.; -1 for Un*x                    */
   char          *lpSpec[4];   /* [0]- drive, [1]- dir, [2]- filename, [3]- extension  */
   }  FILEINFO, *LPFILEINFO;
/*
**    New mfs APIs to achieve some kind of system independent file spec...
**    (see mfs_path.c)
*/
LPFILEINFO mfs_getfinfo(char *);
void       mfs_freefinfo(LPFILEINFO);
char      *mfs_filespec(char *, LPFILEINFO, int);
int        mfs_pathtype(char *);
#ifdef DEBUG
void       mfs_printfinfo(LPFILEINFO);
#endif
#endif /* LATER */

int convert_error(int); /* Strange?! */

#endif	/* mfs_config__h */
