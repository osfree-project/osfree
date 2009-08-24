/* $Id: jfs_statfs.c,v 1.1.1.1 2003/05/21 13:38:14 pasha Exp $ */

static char *SCCSID = "@(#)1.4  7/30/98 14:20:08 src/jfs/ifs/jfs_statfs.c, sysjfs, w45.fs32, 990417.1";
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
 * Module: jfs_statfs.c
 */

#include <sys/vfs.h>
#include <sys/statfs.h>

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_superblock.h"
#include "jfs_debug.h"
#include "jfs_imap.h"
#include "jfs_dmap.h"

/*
 * external references
 */
int32 readSuper(inode_t *ipmnt, cbuf_t **bpp);

/*
 * NAME:	jfs_statfs(vfsp, sfsp, crp)
 *
 * FUNCTION:	get file system status <sfsp> from vfs <vfsp>
 *
 * PARAMETER:	vfsp	- virtual file system
 *		sfsp	- file status information structure
 *		crp	- credential
 *
 * RETURN:	zero on success, non-zero on failure
 *			
 * serialization: statfs() and extendfs() serializes by inode lock
 *	of the inode map for both inode map and block allocation map. 
 *	All other access to fragment allocation map is serialized 
 *	under VMM locking.
 *
 * note: percolation of file system information:
 * support struct statfs (sys/statfs.h) for get file system status
 * service call statfs().
 * (XPG4.2 defines struct statvfs in sys/statvfs.h for statvfs()
 * which requires statfs() and additional information)
 */
jfs_statfs(
	register struct vfs	*vfsp,
	register struct statfs	*statfsp,
	struct ucred		*crp)
{
	register int32	rc;
	register inode_t *ipmnt;	/* mount inode */
	inode_t	*ipimap, *ipbmap;
	cbuf_t	*bpsuper;
	struct superblock *sb = NULL;
	int32	fsck_length, log_length;

NOISE(1,("jfs_statfs: vfs:0x%08x\n", vfsp));

	/*
	 * get the file system stats from the superblock
	 */
	ipimap = (struct inode *)vfsp->vfs_data;
	ipmnt = ipimap->i_ipmnt;
	if (rc = readSuper(ipmnt, &bpsuper))
		goto out;
	sb = (struct superblock *)(bpsuper->cm_cdata);

/*
	bcopy(sb->s_fname, statfsp->f_fname, sizeof(sb->s_fname));
*/
	bcopy(sb->s_fpack, statfsp->f_fpack, sizeof(sb->s_fpack));
	statfsp->f_bsize = PSIZE; /* preferred i/o block size */
	statfsp->f_fsize = sb->s_bsize; /* fundamental block size */
	fsck_length = lengthPXD(&(sb->s_fsckpxd));
	log_length = lengthPXD(&(sb->s_logpxd));

	rawRelease(bpsuper);

 	/* statfs()/extendfs() serialized by inode lock of the inode map 
	 * for both inode map and block allocation map. 
	 */
	IREAD_LOCK(ipimap);

	/*
	 * get the block stats from the bmap
	 */
	ipbmap = ipmnt->i_ipbmap;
	statfsp->f_blocks = (ipbmap->i_bmap->db_mapsize + fsck_length +
				log_length) >> ipbmap->i_bmap->db_l2nbperpage;
	statfsp->f_bfree = statfsp->f_bavail =
		ipbmap->i_bmap->db_nfree >> ipbmap->i_bmap->db_l2nbperpage;

	/*
	 * get the file stats from the ipimap
	 */
	statfsp->f_files = ipimap->i_imap->im_numinos;
	statfsp->f_ffree = ipimap->i_imap->im_numfree;

	/*
	 * fill in from vfs
	 */
	statfsp->f_fsid = vfsp->vfs_fsid;
	statfsp->f_vfstype = MNT_XJFS;
	statfsp->f_vfsnumber = vfsp->vfs_number;
	statfsp->f_name_max = JFS_NAME_MAX;

	/*
	 * fields in the statfs structure that we don't fill in ...
	 *
	long f_version;		version/type of statfs, 0 for now
	long f_type;		type of info, 0 for now
	long f_vfsoff;		reserved, for vfs specific data offset
	long f_vfslen;		reserved, for len of vfs specific data
	long f_vfsvers;		reserved, for vers of vfs specific data
	 */

out:
	IREAD_UNLOCK(ipimap);

	return rc;
}
