/* $Id: jfs_readlink.c,v 1.1 2000/04/21 10:58:15 ktk Exp $ */

static char *SCCSID = "@(#)1.3  7/30/98 14:18:46 src/jfs/ifs/jfs_readlink.c, sysjfs, w45.fs32, 990417.1";
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
 * Module: jfs_readlink.c
 */

#include <sys/file.h>
#include <sys/errno.h>

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_debug.h"

/*
 * NAME:	jfs_readlink(vp, uiop, crp)
 *
 * FUNCTION:	read a symbolic link <vp> into <uiop>
 *
 * PARAMETER:	vp 	- pointer to the vnode that represents the 
 *			  symlink we want to read
 *		uiop	- How much to read and where it goes
 *		crp	- credential
 *
 * RETURN:	EINVAL	- if not a symbolic link
 *		errors from subroutines
 */
jfs_readlink(
	struct vnode	*vp,		/* symlink vnode */
	struct uio	*uiop,
	struct ucred	*crp)
{
	int32	rc = 0;
	inode_t *ip = VP2IP(vp);
	int32	cnt;
	jbuf_t	*bp;

NOISE(1,("jfs_readlink: ip:0x%08x\n", ip));

	if (vp->v_vntype != VLNK)
		return EINVAL;

	IREAD_LOCK(ip);

	/* validate the buffer size vs target path name size
	 * (AES requires ERANGE if the link name won't fit)
	 */
	if (ip->i_size > uiop->uio_resid)
	{
		rc = ERANGE;
		goto out; 
	}

	/*
	 * read the target path name
	 */
	if (ip->i_size <= IDATASIZE)
	{
		/*
		 * fast symbolic link
		 *
		 * read target path name inline from on-disk inode
		 */
		cnt = MIN(ip->i_size, uiop->uio_iov->iov_len);
		rc = uiomove(ip->i_fastsymlink, cnt, UIO_READ, uiop);
	}
	else
	{
		/* 
		 * read target path name from a single extent
		 *
		 * target path name <= PATH_MAX < buffer page size
		 *
		 * even though the data of symlink object (target 
		 * path name) is treated as non-journaled user data,
		 * it is read/written thru buffer cache for performance.
		 */
		if (rc = bmRead(ip, 0, ip->i_size, bmREAD_PAGE, &bp))
			goto out;

		cnt = MIN(ip->i_size, uiop->uio_iov->iov_len);
		rc = uiomove(bp->b_bdata, cnt, UIO_READ, uiop);

		bmRelease(bp);
	}

out:
	IREAD_UNLOCK(ip);

NOISE(1,("jfs_readlink: rc:%d\n", rc));
	return rc;
}
