/* $Id: jfs_fsync.c,v 1.1.1.1 2003/05/21 13:37:08 pasha Exp $ */

static char *SCCSID = "@(#)1.5  3/12/99 10:48:21 src/jfs/ifs/jfs_fsync.c, sysjfs, w45.fs32, 990417.1";
/*
 *
 8
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
 * Module: jfs_fsync.c
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_txnmgr.h"
#include "jfs_debug.h"

/*
 * NAME:	jfs_fsync(vp, flags, vinfo, crp)
 *
 * FUNCTION:	sync regular file.
 *
 * PARAMETERS:	vp	- is the pointer to the vnode that represents the file
 *			  to be written to disk.
 *		flags	- open flags 
 *		vinfo	- file descriptor for mapped file
 *		crp	- credential
 *
 *
 * RETURN:	
 */
jfs_fsync(
	struct vnode	*vp,      	/* vnode */
	int32		flags,		/* open flags */
	int32		vinfo,		/* fd used for mapped files */
	struct ucred	*crp)		/* credential structure */
{
	int32	rc;
	int32	tid;
	inode_t	*ip = VP2IP(vp);                 
	inode_t	*iplist[1];

jEVENT(0,("jfs_fsync: ip:0x%08x\n", ip));

	if (isReadOnly(ip))
		return 0;

	IWRITE_LOCK(ip);

	txBegin(ip->i_ipmnt, &tid, 0);

	ip->i_flag |= IFSYNC;
	iplist[0] = ip;
	rc = txCommit(tid, 1, &iplist[0], 0);

	txEnd(tid);

	IWRITE_UNLOCK(ip);

jEVENT(0,("jfs_fsync: rc:%d\n", rc));

	return rc;
}
