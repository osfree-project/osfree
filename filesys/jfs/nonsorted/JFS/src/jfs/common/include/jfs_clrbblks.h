/* $Id: jfs_clrbblks.h,v 1.1.1.1 2003/05/21 13:35:42 pasha Exp $ */

/* static char *SCCSID = "@(#)1.3  12/4/98 12:56:06 src/jfs/common/include/jfs_clrbblks.h, sysjfs, w45.fs32, 990417.1";*/
/*
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
 *   MODULE_NAME:		jfs_clrbblks.h
 *
 *   COMPONENT_NAME: 	sysjfs
 *
*/
#ifndef	_H_JFS_CLRBBLKS
#define _H_JFS_CLRBBLKS	

/*
 *	clrbblks parameter list
 */
typedef struct {
	uint32	flag;		/* 4: */
	uint8	dev;		/* 1: */
	uint8	pad[3];		/* 3: */
	int32	fileset;		/* 4: */
	uint32	inostamp;	/* 4: */
	ino_t	ino;		/* 4: */
	uint32	gen;		/* 4: */
	int64	xoff;		/* 8: */
	int64	old_xaddr;	/* 8: */
	int64	new_xaddr;	/* 8: */
	int32	xlen;		/* 4: */
	int32	agg_blksize;	/* 4: */
} clrbblks_t;			/* (56) */

/* plist flag */
#define CLRBBLKS_SYNC		0x80000000
#define CLRBBLKS_COMMIT		0x40000000
#define CLRBBLKS_BADBLKALLOC 	0x20000000
#define CLRBBLKS_RELOCATE	0x10000000
#define CLRBBLKS_EA		0x00800000

#define	INODE_TYPE		0x0000F000   /* IFREG or IFDIR */

#define EXTENT_TYPE		0x000000ff
#define DTPAGE			0x00000001
#define XTPAGE			0x00000002
#define DATAEXT			0x00000004
#define EAEXT			0x00000008

#ifndef	_JFS_UTILITY
int32 jfs_clrbblks(char *pData, uint32 lenData, uint16 *pbufsize);
#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_CLRBBLKS */
