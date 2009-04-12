/* $Id: jfs_link.c,v 1.1.1.1 2003/05/21 13:37:46 pasha Exp $ */

static char *SCCSID = "@(#)1.5  7/29/99 08:48:41 src/jfs/ifs/jfs_link.c, sysjfs, w45.fs32, fixbld";
/*
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
 *
 * Module: jfs_link.c
 */

/*
 * Change History :
 *
 */


#include <sys/vnode.h>
#include <sys/errno.h>

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_dnlc.h"
#include "jfs_debug.h"

/*
 * NAME:	jfs_link(vp, dvp, name, crp)
 *
 * FUNCTION:	create a link to <vp> by the name = <name>
 *		in the parent directory <dvp>
 *
 * PARAMETER:	vp 	- target object
 *		dvp	- parent directory of new link
 *		name	- name of new link to target object
 *		crp	- credential
 *
 * RETURN:	Errors from subroutines
 *
 * note:
 * JFS does NOT support link() on directories (to prevent circular
 * path in the directory hierarchy);
 * EPERM: the target object is a directory, and either the caller
 * does not have appropriate privileges or the implementation prohibits
 * using link() on directories [XPG4.2].
 *
 * JFS does NOT support links between file systems:
 * EXDEV: target object and new link are on different file systems and
 * implementation does not support links between file systems [XPG4.2].
 */
jfs_link(
	struct vnode	*vp,
	struct vnode	*dvp,
	char		*name,
	struct ucred	*crp)
{
	int32		rc;
	int32		tid;
	inode_t		*dip = VP2IP(dvp);
	inode_t 	*ip = VP2IP(vp);
	ino_t		ino;
	component_t	dname;
	btstack_t	btstack;
	inode_t		*iplist[2];

NOISE(1,("jfs_link: dip:0x%08x name:%s\n", dip, name));

	/* dip must be a directory */
	if ((dip->i_mode & IFMT) != IFDIR)
		return ENOTDIR;

	/* JFS does NOT support link() on directories */
	if ((ip->i_mode & IFMT) == IFDIR)
		return EPERM;

	/* JFS does NOT support links between file systems */
	if (ip->i_dev != dip->i_dev)
		return EXDEV;

	txBegin(dip->i_ipmnt, &tid, 0);

	/*
	 * Lock regular files, then directories
	 */
	IWRITE_LOCK(ip);					// F226941
	IWRITE_LOCK(dip);					// F226941

	/* link count overflow on target object ? */
	if (ip->i_nlink >= LINK_MAX)
	{
		rc = EMLINK;
		goto out;
	}

	/* validate directory search/write permission */
	if (rc = iAccess(dip, IEXEC|IWRITE, crp))
		goto out;

	/*
	 * scan parent directory for entry/freespace
	 */
	dname.name = name;
	dname.namlen = strlen(name);
	if (rc = dtSearch(dip, &dname, &ino, &btstack, JFS_CREATE))
		goto out;

	/*
	 * create entry for new link in parent directory
	 */
	ino = ip->i_number;
	if (rc = dtInsert(tid, dip, &dname, &ino, &btstack))
		goto out;

	/* update object inode */
	ip->i_nlink++;	/* for new link */
	imark(ip, ICHG);

	/* update parent directory inode */
	imark(dip, ICHG|IUPD|IFSYNC);

	iplist[0] = dip;
	iplist[1] = ip;
	rc = txCommit(tid, 2, &iplist[0], 0);

	/*
	 * insert the new link to dnlc
	 */
	ncEnter(dip->i_ipimap, dip->i_number, &dname, ino, NULL);

out:
	IWRITE_UNLOCK(ip);
	IWRITE_UNLOCK(dip);

	txEnd(tid);

NOISE(1,("jfs_link: rc:%d\n", rc));
	return rc;
}
