/* $Id: jfs_proto.h,v 1.1 2000/04/21 10:57:59 ktk Exp $ */

/* static char *SCCSID = "@(#)1.13  7/23/99 08:07:55 src/jfs/common/include/jfs_proto.h, sysjfs, w45.fs32, fixbld";
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

/*
 * Change History :
 *
 */

#ifndef _H_JFS_PROTO
#define _H_JFS_PROTO

#include "jfs_types.h"
#include "jfs_txnmgr.h"

/*
 * external declarations for log manager
 */

int32 lmInit(void);

int32 lmLog(
	log_t		*log,
	tblock_t	*tblk,
	lrd_t		*lrd,
	tlock_t		*tlck);

int32 lmLogOpen(
	inode_t		*ipmnt,
	inode_t		**iplog);

int32 lmLogClose(
	inode_t		*ipmnt,
	inode_t		*iplog);

int32 lmLogSync(
	log_t		*log,
	int32		nosyncwait);

/*
 * external declarations for inode manager
 */
int32 iAccess(
	inode_t		*ip,
	int32		m,
	struct ucred	*crp);

int32 iget(
	struct vfs	*vfsp,
	ino_t		ino,
	struct inode	**ipp,
	uint32		flag);

int32 iAlloc(
	struct vfs      *vfsp,
	inode_t         *pip,
	mode_t          mode,
	inode_t         **ipp,
	struct ucred    *crp);

int32 iBindCache(
	inode_t		*ip);

int32 iUnbindCache(
	struct inode 	*ip);

void imark(
	struct inode	*ip,
	int		flag);

int32 iuncache(
	struct inode	*ip,
	uint32		flag);

void ixfree(
	int32		x);

int32 iUnmount(
	struct vfs	*vfsp,
	int32		forced);

int32 iunhash(
	struct inode	*ip);

void iSanity(void);

int32 iput(
	struct inode	*ip,
	struct vfs	 *vfsp);
/*
 * Others
 */

#ifdef _JFS_OS2

void SysLogError(void);

void *	ProcessToGlobal(void *,					// F226941
			uint32);				// F226941
#endif

int32 iOpenAccess(
	inode_t		*ip,
	int32		flag,
	struct ucred	*crp);

int32 iOpen(
	inode_t		*ip,
	int32		flag,
	int32		tflag,
#ifdef _JFS_OS2
	FEALIST		*pfealist,
#endif	/* _JFS_OS2 */
	struct ucred	*crp);

int32 iTruncate(
	int32		tid,
	inode_t		*ip,
	int64		newsize);

int32 ExtendI(
	inode_t		*ip,
	int64		newsize,
	boolean_t	abnr);

void iSyncFS(
	struct vfs *vfsp);
#endif /* _H_JFS_PROTO */
