/* $Id: jfs_extendfs.h,v 1.1 2000/04/21 10:57:57 ktk Exp $ */

/* static char *SCCSID = "@(#)1.4  12/4/98 12:56:58 src/jfs/common/include/jfs_extendfs.h, sysjfs, w45.fs32, 990417.1";
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
 */
#ifndef	_H_JFS_EXTENDFS
#define _H_JFS_EXTENDFS	

/*
 *	jfs_extendfs.h
 */
/*
 *	extendfs parameter list
 */
typedef struct {
	uint32  flag;		/* 4: */
	uint8	dev;		/* 1: */
	uint8	pad[3];		/* 3: */
	int64   LVSize;		/* 8: LV size in LV block */
	int64   FSSize;		/* 8: FS size in LV block */
	int32   LogSize;	/* 4: inlinelog size in LV block */
} extendfs_t;			/* (28) */

/* plist flag */
#define EXTENDFS_QUERY		0x00000001

#ifndef	_JFS_UTILITY
#ifdef	_JFS_OS2
int32 jfs_extendfs(char *pData, uint32 lenData, uint16 *pbufsize);
#endif	/* _JFS_OS2 */
#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_EXTENDFS */
