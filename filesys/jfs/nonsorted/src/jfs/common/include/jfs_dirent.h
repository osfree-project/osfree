/* $Id: jfs_dirent.h,v 1.1 2000/04/21 10:57:57 ktk Exp $ */

/* static char *SCCSID = "@(#)1.9  6/15/99 14:21:11 src/jfs/common/include/jfs_dirent.h, sysjfs, w45.fs32, fixbld";
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/*
 * Change History :
 *
 */

#ifndef _H_JFS_DIRENT
#define _H_JFS_DIRENT

#include "jfs_filsys.h"
#include "jfs_dasdlim.h"					// F226941

/*
 *	Pathname Lookup Structure
 */
struct	pathlook {
	UniChar		*path;		/* pointer to full path name */
	UniChar		*pathlast;	/* pointer to last component in path */
	uint32		statlast;	/* status of last component */
	struct vnode	*vp;		/* vnode for last component */
	struct vnode	*dvp;		/* vnode for terminal directory */
	struct dasd_usage dlim;		// DASD limit/usage structure   F226941
};

typedef struct pathlook pathlook_t;

/* Defines for pl_statlast
 */
#define	PL_NOENT	2		/* last component does not exist */
#define PL_EXIST	4		/* last component is a file */
#define PL_ISDIR	6		/* last component is a directory */
#define PL_WCARD	8		/* last component contains wildcard */


/*
 *	Directory Entry ala AIX
 */

struct	dirent {
	uint32	d_offset;	/* real offset after this entry */
	ino_t	d_ino;		/* inode number of entry */
	uint16	d_reclen;	/* length of this record */
	uint16	d_namlen;	/* length of string in d_name */
	UniChar	d_name[JFS_NAME_MAX+1];	/* name must be no longer than this */
};

#ifndef	_JFS_UTILITY

#pragma pack(1)

#define FFBUFHD		(sizeof(FILEFINDBUF) - CCHMAXPATHCOMP)
#define FFBUFHD2	(sizeof(FILEFINDBUF2) - CCHMAXPATHCOMP)
#define FFBUFHD3L	(sizeof(FILEFINDBUF3L) - CCHMAXPATHCOMP - sizeof(ULONG))
#define FFBUFHD4L	(sizeof(FILEFINDBUF4L) - CCHMAXPATHCOMP - sizeof(ULONG))


/***    FindBufFixed - structure of the portion of a FileFindBuf that is
 *      the same for all levels of output.
 */


struct FindBufFixed {
	FDATE		fdateCreation;
	FTIME		ftimeCreation;
	FDATE		fdateLastAccess;
	FTIME		ftimeLastAccess;
	FDATE		fdateLastWrite;
	FTIME		ftimeLastWrite;
	LONGLONG	cbFile;
	LONGLONG	cbFileAlloc;
	ULONG		attrFile;
};

#pragma pack()

#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_DIRENT */
