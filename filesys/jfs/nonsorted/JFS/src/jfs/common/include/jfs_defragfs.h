/* $Id: jfs_defragfs.h,v 1.1.1.1 2003/05/21 13:35:46 pasha Exp $ */

/* static char *SCCSID = "@(#)1.5  12/4/98 12:56:25 src/jfs/common/include/jfs_defragfs.h, jfsdefrag, w45.fs32, 990417.1";
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
#ifndef	_H_JFS_DEFRAGFS
#define _H_JFS_DEFRAGFS	

/*
 *	jfs_defragfs.h
 */
/*
 *	defragfs parameter list
 */
typedef struct {
	uint32  flag;		/* 4: */
	uint8	dev;		/* 1: */
	uint8	pad[3];		/* 3: */
	int32	fileset;	/* 4: */
	uint32  inostamp;	/* 4: */
	ino_t   ino;		/* 4: */
	uint32	gen;		/* 4: */
	int64   xoff;		/* 8: */
	int64   old_xaddr;	/* 8: */
	int64   new_xaddr;	/* 8: */
	int32   xlen;		/* 4: */
} defragfs_t;			/* (52) */

/* plist flag */
#define DEFRAGFS_SYNC		0x80000000
#define DEFRAGFS_COMMIT		0x40000000
#define DEFRAGFS_RELOCATE	0x10000000

#define	INODE_TYPE		0x0000F000   /* IFREG or IFDIR */

#define EXTENT_TYPE		0x000000ff
#define DTPAGE			0x00000001
#define XTPAGE			0x00000002
#define DATAEXT			0x00000004
#define EAEXT			0x00000008

#ifndef	_JFS_UTILITY
#ifdef	_JFS_OS2
int32 jfs_defragfs(char *pData, uint32 lenData, uint16 *pbufsize);
#endif	/* _JFS_OS2 */
#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_DEFRAGFS */
