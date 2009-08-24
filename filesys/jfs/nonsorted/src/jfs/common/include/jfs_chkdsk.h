/* $Id: jfs_chkdsk.h,v 1.1 2000/04/21 10:57:55 ktk Exp $ */

/* static char *SCCSID = "@(#)1.3  12/4/98 12:55:44 src/jfs/common/include/jfs_chkdsk.h, sysjfs, w45.fs32, 990417.1";
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
 * MODULE_NAME:		jfs_chkdsk.h
 *
 * COMPONENT_NAME: 	sysjfs
 *
 *	common data & function prototypes
 *
 */
#ifndef	_H_JFS_CHKDSK
#define _H_JFS_CHKDSK

/*
 *  parameter list
 */
typedef struct {
	uint32	flag;		/* 4: */
	uint8	dev;		/* 1: */
	uint8	pad[3];		/* 3: */
	int32	fileset;		/* 4: */
	uint32	inostamp;	/* 4: */
} chkdsk_t;			/* (16) */

/* 
 * plist flag values
 */
#define CHKDSK_SYNC		0x80000000


#ifndef	_JFS_UTILITY
int32 jfs_chkdsk(char *pData, uint32 lenData, uint16 *pbufsize);
#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_CHKDSK */
