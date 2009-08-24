/* $Id: jfs_umount.c,v 1.1 2000/04/21 10:58:17 ktk Exp $ */

static char *SCCSID = "@(#)1.15  11/1/99 15:10:15 src/jfs/ifs/jfs_umount.c, sysjfs, w45.fs32, fixbld";
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
 */

/*
 * Change History :
 *
 */

/*
 *	jfs_umount.c
 *
 * note: file system in transition to aggregate/fileset:
 * (ref. jfs_mount.c)
 *
 * file system unmount is interpreted as mount of the single/only 
 * fileset in the aggregate and, if unmount of the last fileset, 
 * as unmount of the aggerate;
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
#include "jfs_bufmgr.h"
#include "jfs_superblock.h"
#include "jfs_imap.h"
#include "jfs_dmap.h"
#include "jfs_dnlc.h"
#include "jfs_proto.h"
#include "jfs_dasdlim.h"					// D233382
#include "jfs_debug.h"

/* For OS/2 jfs_debug.h defines static to be null, but we need updateSuper()
 * to be static.
 */
#if defined(static)
#undef static
#endif

/* external references not covered in header files */
int32 readSuper(inode_t *ipmnt, cbuf_t **bpp);
void dasd_umount(struct vfs *);					// F226941

/* forward references */
static int32 updateSuper(inode_t *ipmnt);

/*
 * NAME:	jfs_umount(vfsp, flags, crp)
 *
 * FUNCTION:	vfs_umount()
 *
 * PARAMETERS:	vfsp	- virtual file system pointer
 *		flags	- unmount for shutdown
 *		crp	- credential
 *
 * RETURN :	EBUSY	- device has open files
 */
jfs_umount(
	register struct vfs	*vfsp,
	int32	 		flags,
	struct ucred		*crp)
{
	int32	rc = 0;
        dev_t	fsdev;
        ino_t	fileset;
	int32	forced;
        struct inode *rootip;
	struct vnode *rootvp;
	struct inode	*ipimap, *ipaimap, *ipaimap2, *ipbmap, *ipmnt, *iplog;

jFYI(1,("\n	UnMount JFS: vfs:0x%08x\n", vfsp));

	/*
	 * purge volume information from DASD limit structures
	 */
	dasd_umount(vfsp);					// F226941

	/*
	 * get the mount inode representing the mounted aggregate
	 */
	ipimap = (struct inode *)vfsp->vfs_data;
	ipmnt = ipimap->i_ipmnt;

	/*
	 * serialize mount/unmount
	 */
	JFS_LOCK();

	/*
	 *	unmount fileset:
	 */
umntFileset:
	/* retrieve the mounted fs device */
        fsdev = ipmnt->i_dev;
	fileset = ipimap->i_number;

	/*
	 * Purge directory name lookup cache for the fileset. 
	 */
	ncUnmount(ipimap);

	/*
	 * close inodes of the fileset in inode cache
	 *
	 * There should be no active files in the fileset
	 * being unmounted (unless this is a shutdown
	 * unmount, in which case we force the vfs to be
	 * unmounted anyway).
	 */
jEVENT(0,("jfs_umount: close fileset inodes\n"));
	forced = (vfsp->vfs_flag & VFS_SHUTDOWN);
	if ((rc = iUnmount(vfsp, forced)) && !forced)
		goto out;

	/*
	 *	point of no return for unmount: 
	 *
	 * ignore failure of further intermediate steps and
	 * return success to notify LFS to cleanup.
	 */
	rc = 0;

	/*
	 * close root vode/inode (skipped in iunmount() of the fileset)
	 */
	rootvp = vfsp->vfs_mntd;
	vfsp->vfs_mntd = NULL;

	rootip = VP2IP(rootvp);
//	assert(rootip->i_count == 1);
	ICACHE_LOCK();
	iuncache(rootip, 1);
	ICACHE_UNLOCK();

	/*
	 * close fileset inode allocation map (aka fileset inode)
	 */
jEVENT(0,("jfs_umount: close ipimap:0x%08x\n", ipimap));
	diUnmount(ipimap, 0);
	bmInodeFlush(ipimap);
	iUnbindCache(ipimap);

	ICACHE_LOCK();
	iunhash(ipimap);
	ICACHE_UNLOCK();

	/* more fileset in aggregate ? */
	if (--ipmnt->i_count)
		goto out;
	
	/*
	 *	unmount aggregate (file system)
	 */
umntAggregate:
	MUTEXLOCK_FREE(&ipmnt->i_renamelock);

	/*
	 * close secondary aggregate inode allocation map
	 */
	ipaimap2 = ipmnt->i_ipaimap2;
	if (ipaimap2)
	{
jEVENT(0,("jfs_umount: close ipaimap2:0x%08x\n", ipaimap2));
		diUnmount(ipmnt->i_ipaimap2, 0);
		bmInodeFlush(ipaimap2);
		iUnbindCache(ipaimap2);

		ICACHE_LOCK();
		iunhash(ipaimap2);
		ICACHE_UNLOCK();
	}

	/*
	 * close aggregate inode allocation map
	 */
	ipaimap= ipmnt->i_ipaimap;	
jEVENT(0,("jfs_umount: close ipaimap:0x%08x\n", ipaimap));
	diUnmount(ipmnt->i_ipaimap, 0);
	bmInodeFlush(ipaimap);
	iUnbindCache(ipaimap);

	ICACHE_LOCK();
	iunhash(ipaimap);
	ICACHE_UNLOCK();

	/*
	 * close aggregate block allocation map
	 */
	ipbmap = ipmnt->i_ipbmap;	
jEVENT(0,("jfs_umount: close ipbmap:0x%08x\n", ipbmap));
	dbUnmount(ipmnt->i_ipbmap, 0);
	bmInodeFlush(ipbmap);
	iUnbindCache(ipbmap);

	ICACHE_LOCK();
	iunhash(ipbmap);
	ICACHE_UNLOCK();

	/*
	 * free cache device for aggregate i/o
	 */
	cmUnmount(ipmnt->i_cachedev);

	/*
	 * ensure all file system file pages are propagated to their
	 * home blocks on disk (and their in-memory buffer pages are 
	 * invalidated) BEFORE updating file system superblock state
	 * (to signify file system is unmounted cleanly, and thus in 
	 * consistent state) and log superblock active file system 
	 * list (to signify skip logredo()).
	 */

	/*
	 *	update superblock and close log 
	 *
	 * if mounted read-write and log based recovery was enabled
	 */
	if (ipmnt->i_iplog)
	{
		/*
		 * update superblock: synchronously write and 
		 * invalidate its in-memory buffer page
		 */
		rc = updateSuper(ipmnt);

		/*
	 	 * close log: 
		 *
		 * remove file system from log active file system list.
	 	 */
		iplog = ipmnt->i_iplog;
		rc = lmLogClose(ipmnt, iplog);
	}

	/*
	 *	close file system device
	 */
	ipmnt->i_devfp = NULL;

	/*
	 * close "mount" inode.
	 */
	ICACHE_LOCK();
	iunhash(ipmnt);
	ICACHE_UNLOCK();

out:
	JFS_UNLOCK();

	/* debug sanity check */
	/* iSanity(); */
	/* bmSanity(); */

jFYI(0,("	UnMount JFS Complete: %d\n", rc));
	return rc;
}


/*
 *	updateSuper(ipmnt)
 *
 * update superblock if it is mounted read-write
 */
static int32
updateSuper(
	inode_t	*ipmnt)
{
	int32	rc = 0;
	cbuf_t	*bp;
	struct superblock *sb;

	if (rc = readSuper(ipmnt, &bp))
		return rc;
	sb = (struct superblock *)(bp->cm_cdata);

 	/*
	 * file system state transition: 
	 * if mounted-clean, mark it as unmounted-clean;
 	 * otherwise state remains as dirty.
	 */
	if (sb->s_state == FM_MOUNT)
		sb->s_state = FM_CLEAN;

#ifdef _JFS_FASTDASD
	/*
	 * Synchronize DASD_PRIME flag in superblock with the one stored in
	 * the mount inode
	 */
	sb->s_flag = (sb->s_flag & ~JFS_DASD_PRIME) |		// D233382
		     (ipmnt->i_mntflag & JFS_DASD_PRIME);	// D233382
#endif /* _JFS_FASTDASD */

	/* write out superblock synchronously and invalidate page */
	rc = rawWrite(ipmnt, bp, 1);

	return rc;
}
