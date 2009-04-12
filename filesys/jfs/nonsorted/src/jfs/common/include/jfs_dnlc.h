/* $Id: jfs_dnlc.h,v 1.1 2000/04/21 10:57:57 ktk Exp $ */

/* static char *SCCSID = "@(#)1.4  7/30/98 14:05:59 src/jfs/common/include/jfs_dnlc.h, sysjfs, w45.fs32, 990417.1";
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
#ifndef	_H_JFS_DNLC
#define _H_JFS_DNLC	

/*
 *	jfs_dnlc.h: dnlc
 */

/* dnlc client/server cookie */
typedef struct ncookie {
	struct nhash	*nhp;
	uint32	timestamp;
} ncookie_t;

/* external references */
int32 ncInit(void);

ino_t ncLookup(
	inode_t		*fileset,	
	ino_t		dino,	
	component_t	*nmp,
	ncookie_t	*cookie);

void ncEnter(
	inode_t		*fileset,
	ino_t		dino,	
	component_t	*nmp,
	ino_t		ino,
	ncookie_t	*cookie);

void ncDelete(
	inode_t		*fileset,
	ino_t		dino,	
	component_t	*nmp);

void ncUnmount(
	inode_t		*fileset);

#endif /* _H_JFS_DNLC */
