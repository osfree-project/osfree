/* $Id: jfs_mknod.c,v 1.1.1.1 2003/05/21 13:37:58 pasha Exp $ */

static char *SCCSID = "@(#)1.3  7/30/98 14:17:46 src/jfs/ifs/jfs_mknod.c, sysjfs, w45.fs32, 990417.1";
/*
 *
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
 * Module: jfs_mknod.c
 */

#include <sys/vfs.h>
#include <sys/cred.h>
#include <sys/errno.h>

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_dnlc.h"
#include "jfs_bufmgr.h"
#include "jfs_debug.h"


/*
 * NAME:	jfs_mknod(dvp, name, mode, dev, crp)
 *
 * FUNCTION:	make a new object in directory <dvp> with mode = <mode>,
 *		name = <pname>, and rdev = <dev>.
 *
 * PARAMETER:	dvp 	- directory vnode
 *		name	- name of new object
 *		mode	- create mode (rwxrwxrwx).
 *		dev	- new device number if block/character-special file
 *		crp	- credential
 *
 * RETURN:	Errors from subroutines
 *
 * note: JFS allows mknod() to create a special file.
 * XPG4.2: the only portable use of mknod() is to create a FIFO-special file
 * with mode = S_IFIFO and dev = 0.
 */
jfs_mknod(
	struct vnode	*dvp,
	caddr_t		name,
	mode_t		mode,
	dev_t		dev,
	struct ucred	*crp)
{
	int32	rc;
	int32	tid;
	inode_t	*dip = VP2IP(dvp);
	inode_t *ip;
	ino_t	ino;
	component_t	dname;
	struct vfs	*vfsp = dvp->v_vfsp;
	btstack_t	btstack;
	inode_t		*iplist[2];

	/* JFS does NOT support mknod() of directory */
	if ((mode & IFMT) == IFDIR)
		return EISDIR;
		
	if ((mode & IFMT) != IFIFO && (mode & IFMT) != IFSOCK)
		if (rc = privcheck_cr(DEV_CONFIG, crp))
			return rc;

	IWRITE_LOCK(dip);

	txBegin(dip->i_ipmnt, &tid, 0);

	/* validate search+write permission on parent directory */
	if (rc = iAccess(dip, IEXEC|IWRITE, crp))
		goto out1;

	/*
	 * scan parent directory for entry/freespace
	 * (dtSearch() returns parent directory page pinned)
	 */
	dname.name = name;
	dname.namlen = strlen(name);
	if (rc = dtSearch(dip, &dname, &ino, &btstack, JFS_CREATE))
		goto out1;

	/*
	 * allocate in-memory+on-disk inode:
	 * (iAlloc() returns new, locked inode)
	 */
	if (rc = iAlloc(vfsp, dip, mode, &ip, crp))
	{	
		/* release parent directory page */
		BT_PUTSEARCH(&btstack);

		goto out1;
	}

	/*
	 * create entry in parent directory
	 * (dtInsert() releases parent directory page)
	 */
	ino = ip->i_number;
	if (rc = dtInsert(tid, dip, &dname, &ino, &btstack))
	{
		/* discard the new inode */
		ip->i_nlink = 0;

		goto out2;
	}

	/*
	 * fo a block or character special file, the passed-in device number
	 * needs to be set into the inode's device field and the gnode's
	 * "real device" field.
	 */
	if ((ip->i_mode & IFMT) == IFBLK || (ip->i_mode & IFMT) == IFCHR)
	{
		ip->i_rdev = dev;
		IP2GP(ip)->gn_rdev = dev;
	}

	imark(ip, IACC|ICHG|IUPD|IFSYNC);	

	/* update parent directory inode */
	imark(dip, ICHG|IUPD|IFSYNC);

	/*
	 * insert entry for the new file to dnlc
	 */
	ncEnter(dip->i_ipimap, dip->i_number, &dname, ino, NULL);

	iplist[0] = dip;
	iplist[1] = ip;
	rc = txCommit(tid, 2, &iplist[0], 0);

out2:
	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, vfsp);
	ICACHE_UNLOCK();

out1:
	IWRITE_UNLOCK(dip);

	txEnd(tid);

	return rc;
}
