/* $Id: jfs_symlink.c,v 1.1.1.1 2003/05/21 13:38:16 pasha Exp $ */

static char *SCCSID = "@(#)1.3  7/30/98 14:20:25 src/jfs/ifs/jfs_symlink.c, sysjfs, w45.fs32, 990417.1";
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
 * Module: jfs_symlnk.c
 */

#include <sys/vfs.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/cred.h>
#include <sys/errno.h>

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_debug.h"

/*
 * NAME:	jfs_symlink(dvp, name, src, crp)
 *
 * FUNCTION:	creates a symbolic link to <symlink> by name <name>
 *		in directory <dvp>
 *
 * PARAMETER:	dvp	- parent directory
 *		name 	- the component name of the new link
 *		src	- the path name of the existing object 
 *			  that will be the source of the link
 *		crp	- credential
 *
 * RETURN:	errors from subroutines
 *
 * note:
 * ENAMETOOLONG: pathname resolution of a symbolic link produced
 * an intermediate result whose length exceeds PATH_MAX [XPG4.2]
 */
jfs_symlink(
	struct vnode	*dvp,
	char		*name,
	char		*src,
	struct ucred	*crp)
{
	int32		rc = 0;
	int32		tid;
	struct vfs	*vfsp = dvp->v_vfsp;
	inode_t 	*dip = VP2IP(dvp);
	inode_t 	*ip;
	ino_t		ino = 0;
	component_t	dname;		
	int32		ssize;	/* source pathname size */
	jbuf_t		*bp;
	int32		bmask, xsize;
	int64		xlen;
	int64		xaddr;
	btstack_t	btstack;
	inode_t		*iplist[2];

NOISE(1,("jfs_symlink: dip:0x%08x name:%s\n", dip, name));

	/* symlink path name <= PATH_MAX <= buffer page size */
	ssize = strlen(src);
	if (ssize > JFS_PATH_MAX)
		return ENAMETOOLONG;

	txBegin(dip->i_ipmnt, &tid, 0);

	IWRITE_LOCK(dip);

	/* validate search+write permission on parent directory */
	if (rc = iAccess(dip, IEXEC|IWRITE, crp))
		goto out2;

	/*
	 * search parent directory for entry/freespace
	 * (dtSearch() returns parent directory page pinned)
	 */
	dname.name = name;
	dname.namlen = strlen(name);
	if (rc = dtSearch(dip, &dname, &ino, &btstack, JFS_CREATE))
		goto out2;

	/*
	 * allocate on-disk/in-memory inode for symbolic link:
	 * (iAlloc() returns new, locked inode)
	 */
	if (rc = iAlloc(vfsp, dip, IFLNK|0777, &ip, crp))
	{
		/* release parent directory page */
		BT_PUTSEARCH(&btstack);

		goto out2;
	}

	/*
	 * create entry for symbolic link in parent directory
	 */
	ino = ip->i_number;
	if (rc = dtInsert(tid, dip, &dname, &ino, &btstack))
		goto out1;

	/* fix symlink access permission
	 * (dir_create() ANDs in the u.u_cmask, 
	 * but symlinks really need to be 777 access)
	 */
	ip->i_mode |= 0777;
	imark(ip, ICHG|IUPD|IFSYNC);

	/* update parent directory inode */
	imark(dip, ICHG|IUPD|IFSYNC);

	/*
	 *	write symbolic link target path name
	 */
	ip->i_size = ssize;

	/*
	 * write source path name inline in on-disk inode (fast symbolic link)
	 */
	if (ssize <= IDATASIZE)
	{	
		ip->i_dxd.flag = DXD_INLINE;
		bcopy(src, ip->i_fastsymlink, ssize);
	}
	/*
	 * write source path name in a single extent
	 */
	else
	{
NOISE(1,("jfs_symlink: allocate extent ip:0x%08x\n", ip));
		/*
		 * even though the data of symlink object (source 
		 * path name) is treated as non-journaled user data,
		 * it is read/written thru buffer cache for performance.
		 */
		bmask = ip->i_ipmnt->i_bsize - 1;
		xsize = (ssize + bmask) & ~bmask;
		xlen = xsize >> ip->i_ipmnt->i_l2bsize;
		if ((rc = xtInsert(tid, ip, 0, 0, xlen, &xaddr, 0)) == 0)
		{
			bp = bmAssign(ip, 0, xaddr, ip->i_size, bmREAD_PAGE);
			bcopy(src, bp->b_bdata, ssize);
			bmAsyncWrite(bp);
			
			ip->i_nblocks = xlen;
		}
		else
			dtDelete(tid, dip, &dname, &ino, JFS_REMOVE);
	}

	/*
	 * commit update of parent directory and link object
	 *
	 * if extent allocation failed (ENOSPC),
	 * the parent inode is committed regardless to avoid
	 * backing out parent directory update (by dtInsert())
	 * and subsequent dtDelete() which is harmless wrt 
	 * integrity concern.  
	 * the symlink inode will be freed by iput() at exit
	 * as it has a zero link count (by dtDelete()) and 
	 * no permanant resources. 
	 */
	iplist[0] = dip;
	if (rc == 0)
	{
		iplist[1] = ip;
		rc = txCommit(tid, 2, &iplist[0], 0);
	}
	else
		rc = txCommit(tid, 1, &iplist[0], 0);

out1:
	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, vfsp);
	ICACHE_UNLOCK();

out2:
	IWRITE_UNLOCK(dip);

	txEnd(tid);

NOISE(1,("jfs_symlink: rc:%s\n", rc));
	return rc;
}
