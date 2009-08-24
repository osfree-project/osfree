/* $Id: jfs_getattr.c,v 1.1.1.1 2003/05/21 13:37:08 pasha Exp $ */

static char *SCCSID = "@(#)1.3  7/30/98 14:15:39 src/jfs/ifs/jfs_getattr.c, sysjfs, w45.fs32, 990417.1";
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
 * Module: jfs_getattr.c
 *
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
#include "jfs_debug.h"

/* forward references */
void	get_vattr(inode_t *, struct vattr *);

/*
 * NAME:	jfs_getattr(vp, vattrp, crp)
 *
 * FUNCTION:	get attributes of <vp> in fs independent format <vattrp>
 *
 * PARAMETER:	vp 	_ object vnode
 *		vattrp	- fs independent attribute format (out)
 *		crp	- credential
 *
 * RETURN:	0
 *
 * note:
 * percolation of file status:
 * the field name and file mode bits (file type/permission) are from
 * struct stat (in sys/stat.h and sys/mode.h included in sys/stat.h) 
 * which specifies minimum implementation independent information 
 * for get file status service (stat()).
 * (struct vattr is a superset of struct stat, and acts as implementation 
 * dependent intermediate form between LFS and underlying PFS types). 
 *
 * read/write/execute permission of the named file is NOT required.
 */
jfs_getattr(
	struct vnode	*vp,
	struct vattr	*vattrp,
	struct ucred	*crp)
{
	inode_t *ip = VP2IP(vp);

NOISE(1,("jfs_getattr: ip:0x%08x\n", ip));

	IREAD_LOCK(ip);

	get_vattr(ip, vattrp);

	IREAD_UNLOCK(ip);

	return 0;
}


/*
 * NAME:	get_vattr(ip, vattrp)
 *
 * FUNCTION:	return attribute of <ip> in <vattrp>
 *
 * PARAMETER:	ip	- object inode
 *		vattrp	- fs independent attribute format (out)
 *
 * RETURN:	void
 */
void
get_vattr(inode_t	*ip,
	 struct vattr	*vattrp)
{
	/*
	 * POSIX 1003.1b-1993/SPEC 1170 attributes
	 */
	vattrp->va_mode = ip->i_mode;		/* [POSIX] st_mode */
	vattrp->va_serialno = ip->i_number;	/* [POSIX] st_ino */
	vattrp->va_dev = brdev(ip->i_dev);	/* [POSIX] st_dev */
	vattrp->va_nlink = ip->i_nlink;		/* [POSIX] st_nlink */
	vattrp->va_uid = ip->i_uid;		/* [POSIX] st_uid */
	vattrp->va_gid = ip->i_gid;		/* [POSIX] st_gid */
	vattrp->va_size = ip->i_size;		/* [POSIX] st_size */

	INODE_LOCK(ip);
	vattrp->va_atime = ip->i_atime;		/* [POSIX] st_atime */
	vattrp->va_mtime = ip->i_mtime;		/* [POSIX] st_mtime*/
	vattrp->va_ctime = ip->i_ctime;		/* [POSIX] st_ctime */
#ifdef _JFS_OS2
	vattrp->va_otime = ip->i_otime;		/* [OS/2] create time */
#endif
	INODE_UNLOCK(ip);

	/* [SPEC1170] st_rdev: for character/block special file */
	vattrp->va_rdev = (dev_t)ip->i_rdev;
	/* [SPEC1170] st_blocks: number of blocks of filesystem-specific size
	 */
	vattrp->va_blocks = ip->i_nblocks; 
	/* [SPEC1170] st_blksize: filesystem-specific preferred I/O block size */
	vattrp->va_blocksize = PSIZE;
}
