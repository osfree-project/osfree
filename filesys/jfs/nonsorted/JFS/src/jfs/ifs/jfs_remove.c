/* $Id: jfs_remove.c,v 1.1.1.1 2003/05/21 13:38:10 pasha Exp $ */

static char *SCCSID = "@(#)1.19  11/1/99 14:12:22 src/jfs/ifs/jfs_remove.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_remove.c
 */

/*
 * Change History :
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
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_txnmgr.h"
#include "jfs_dnlc.h"
#include "jfs_bufmgr.h"
#include "jfs_proto.h"
#include "jfs_debug.h"
#include <uni_inln.h>

/*
 * External declarations
 */
void	iwritelocklist(int, ... );

/*
 * forward references
 */
int	iStickyAccess(inode_t *, inode_t *, struct ucred *);
int	commitZeroLink(int32, inode_t *);

/*
 * NAME:	jfs_remove(vp, dvp, name, crp)
 *
 * FUNCTION:	remove a link to object <vp> named by <name> 
 *		from parent directory <dvp>
 *
 * PARAMETER:	vp 	- pointer to the vnode that represents the 
 *			  object we want to remove
 *		dvp	- parent directory of vp
 *		name 	- name for vp
 *		crp	- credential
 *
 * RETURN:	errors from subroutines
 *
 * note:
 * temporary file: if one or more processes have the file open
 * when the last link is removed, the link will be removed before
 * unlink() returns, but the removal of the file contents will be
 * postponed until all references to the files are closed.
 *
 * JFS does NOT support unlink() on directories.
 * EPERM: the path argument shall not name a directory unless
 * the thread has appropriate privileges and the implementation
 * supports using unlink() on directories.
 *
 * JFS supports sticky bit permission.
 * normally, to delete an existing file, user needs execute and write
 * permission of the directory containing the file, but he
 * does NOT need permissions for the file itself.
 * EACCESS: if S_ISVTX (aka sticky) bit is set for a directory, 
 * a file in the directory can be removed or renamed only if 
 * the user has write permission for the directory, and
 * either owns the file, owns the directory, or 
 * is the superuser (or have appropriate privileges).
 * (e.g., /tmp in which any user can create a file but 
 * should not be able to delete or rename files owned 
 * by others) [XPG4.2]
 */
jfs_remove(
	struct vnode	*vp,		/* object vnode */
	struct vnode	*dvp,		/* parent directory vnode */
	UniChar		*name,		/* object name */
	boolean_t	have_locks,	/* TRUE == have locks on inodes */
	struct ucred	*crp)
{
	int32	rc;
	int32	tid;		/* transaction id */
	inode_t *dip = VP2IP(dvp);	/* parent directory inode */
	inode_t *ip = VP2IP(vp);	/* object inode */
	ino_t	ino;
	component_t dname;		/* object name */
	inode_t	*iplist[2];
	int64	orig_nblocks;					// F226941
#ifdef _JFS_LAZYCOMMIT
	tblock_t *tblk;						// D230860
#endif

jEVENT(0,("jfs_remove: dip:0x%08x name:%s\n", dip, name));

 	/* JFS does NOT support unlink() for directory */
	if ((ip->i_mode & IFMT) == IFDIR)
		return EPERM;

	if (!have_locks)
		IWRITE_LOCK_LIST(2, dip, ip);

#ifdef _JFS_OS2
	/* validate write permission */
	if (rc = iAccess(ip, IWRITE, crp))
		goto out;
#endif /* _JFS_OS2 */

	/* Must be able to write to device */
	if (isReadOnly(ip))
	{
		rc = EROFS;
		goto out;
	}
	/*
	 * delete name entry from dnlc
	 */
	dname.name = name;
	dname.namlen = UniStrlen(name);
	ncDelete(dip->i_ipimap, dip->i_number, &dname);

	txBegin(dip->i_ipmnt, &tid, 0);

	orig_nblocks = ip->i_nblocks + dip->i_nblocks;		// F226941
	iplist[0] = dip;
	iplist[1] = ip;

#ifdef _JFS_LAZYCOMMIT
	tblk = &TxBlock[tid];					// D230860
	tblk->xflag |= COMMIT_DELETE;				// D230860
	tblk->ip = ip;						// D230860
#endif /* _JFS_LAZYCOMMIT */

	/*
	 * delete the entry of target file from parent directory
	 */
	ino = ip->i_number;
	if (rc = dtDelete(tid, dip, &dname, &ino, JFS_REMOVE))
	{
		if (rc == EIO)
			txAbort(tid, 1);	/* Marks FS Dirty */
		goto out1;
	}

	/* update target's inode */
	ip->i_nlink--;
	imark(ip, ICHG);

	/*
	 *	commit zero link count object
	 */
	if (ip->i_nlink == 0)
	{
assert(!(ip->i_cflag & COMMIT_NOLINK));
		/* free block resources */
		if (rc = commitZeroLink(tid, ip))
		{
			txAbort(tid, 1);	/* Marks FS Dirty */
			goto out1;
		}
	}

	/* updare parent directory inode */
	imark(dip, ICHG|IUPD|IFSYNC);

	DLIM_UPDATE(tid, ip, dip->i_nblocks - orig_nblocks);	// F226941

#ifndef _JFS_FASTDASD						// D233382
	if (ip->i_dasdlim && (ip->i_dasdlim->flag & DLIM_LOGGED))  // F226941
		rc = dasd_commit(tid, ip, 0);			// F226941
	else							// F226941
#endif /* _JFS_FASTDASD */					// D233382
		rc = txCommit(tid, 2, &iplist[0], 0);

out1:
	txEnd(tid);

out:
	if (!have_locks)
	{
		IWRITE_UNLOCK(ip);

		IWRITE_UNLOCK(dip);
	}

jEVENT(0,("jfs_remove: rc:%d\n", rc));
	return rc;
}

#ifndef _JFS_OS2
/*
 * NAME:	iStickyAccess(dp, ip, crp)
 *
 * FUNCTION:    validate that caller can delete/rename <ip> from <dp>
 *		for saved-text (aka sticky) directory (S_ISVTX bit set):
 *		if the bit is set for a directory a file in the 
 *		directory can be removed or renamed only if the 
 *		user has write permission for the directory, and
 *		either owns the file, owns the directory, or is
 *		the superuser (or have authority to bypass write checking).
 *		(e.g., /tmp in which any user can create a file but 
 *		should not be able to delete or rename files owned 
 *		by others)[defined by SPEC1170, but not defined by POSIX].
 *
 * PARAMETERS:	dp	- directory inode
 *		ip	- inode to unlink
 *		crp	- credential
 *
 * RETURNS:	0	if deleting is ok
 *              EPERM	otherwise
 */
iStickyAccess(
	inode_t *dip,		/* parent directory inode */
	inode_t *ip,		/* inode to unlink */
	struct ucred *crp)
{
	/* privcheck() returns a (non-zero) error code
	 * if one _lacks_ the privilege in question.
	 */
	if ((dip->i_mode & S_ISVTX) && 
	    (crp->cr_uid != ip->i_uid) && 
	    (crp->cr_uid != dip->i_uid) && 
	    privcheck_cr(BYPASS_DAC_WRITE, crp))
		return EPERM;

	return 0;
}
#endif

/*
 * NAME:	commitZeroLink()
 *
 * FUNCTION:    for non-directory, called by jfs_remove(),
 *		truncate a regular file, directory or symbolic
 *		link to zero length. return 0 if type is not 
 *		one of these.
 *
 *		if the file is currently associated with a VM segment
 *		only permanent disk and inode map resources are freed,
 *		and neither the inode nor indirect blocks are modified
 *		so that the resources can be later freed in the work
 *		map by ctrunc1.
 *		if there is no VM segment on entry, the resources are
 *		freed in both work and permanent map.
 *		(? for temporary file - memory object is cached even 
 *		after no reference:
 *		reference count > 0 -   )
 *
 * PARAMETERS:	cd	- pointer to commit data structure.
 *			  current inode is the one to truncate.
 *
 * RETURN :	Errors from subroutines
 */
commitZeroLink(
	int32	tid,
	inode_t	*ip)
{
	int32		rc = 0;
	int32		filetype, committype;
	tblock_t	*tblk;

	/* set the IFSYNC flag to force iwrite() to update inode */
	ip->i_flag |= IFSYNC;

	filetype = ip->i_mode & IFMT;
	switch(filetype) {
	case IFREG:
		break;
	case IFLNK:
		/* fast symbolic link */
		if (ip->i_size <= 256)
		{
			ip->i_size = 0;
			return 0;
		}
		break;
	default:
		assert(filetype != IFDIR);
		return 0;
	}

	/*
	 *	free from block allocation map:
	 *
	 * if there is no cache control element associated with 
	 * the file, free resources in both persistent and work map;
	 * otherwise just persistent map. 
	 */
	if (ip->i_cacheid)
	{
		committype = COMMIT_PMAP;

		/* mark for iClose() to free from working map */
		ip->i_cflag |= COMMIT_FREEWMAP;
	}
	else
		committype = COMMIT_PWMAP;

	/* mark transaction of block map update type */
	tblk = &TxBlock[tid];
	tblk->xflag |= committype;

	/*
	 * free EA
	 */
	if (ip->i_ea.flag & DXD_EXTENT)
	{
		/* free EA pages from cache */
		if (committype == COMMIT_PWMAP)
			bmExtentInvalidate(ip, addressDXD(&ip->i_ea), lengthDXD(&ip->i_ea));

		/* acquire maplock on EA to be freed from block map */
		txEA(tid, ip, &ip->i_ea, NULL);

		if (committype == COMMIT_PWMAP)
			ip->i_ea.flag = 0;
	}

	/*
	 * free ACL
	 */
	if (ip->i_acl.flag & DXD_EXTENT)
	{
		/* free ACL pages from cache */
		if (committype == COMMIT_PWMAP)
			bmExtentInvalidate(ip, addressDXD(&ip->i_acl),
					   lengthDXD(&ip->i_acl));

		/* acquire maplock on EA to be freed from block map */
		txEA(tid, ip, &ip->i_acl, NULL);

		if (committype == COMMIT_PWMAP)
			ip->i_acl.flag = 0;
	}

	/*
	 * free xtree/data (truncate to zero length):
	 * free xtree/data pages from cache if COMMIT_PWMAP, 
	 * free xtree/data blocks from persistent block map, and
	 * free xtree/data blocks from working block map if COMMIT_PWMAP;
	 */
	if (ip->i_size)
		rc = xtTruncate(tid, ip, 0, committype);

	return rc;
}


/*
 * NAME:	freeZeroLink()
 *
 * FUNCTION:    for non-directory, called by iClose(),
 *		free resources of a file from cache and WORKING map 
 *		for a file previously committed with zero link count
 *		while associated with a pager object,
 *
 * PARAMETER:	ip	- pointer to inode of file.
 *
 * RETURN:	0 -ok
 */
freeZeroLink(
	inode_t	*ip)
{
	int32	rc = 0;
	int32	type;

	/* return if not reg or symbolic link or if size is
	 * already ok.
	 */
	type = ip->i_mode & IFMT;

	switch(type) {
	case IFREG:
		break;
	case IFLNK:
		/* if its contained in inode nothing to do */
		if (ip->i_size <= 256)
			return 0;
		break;
	default:
		return 0;
	}

	/*
	 * free EA
	 */
	if (ip->i_ea.flag & DXD_EXTENT)
	{
		int64	xaddr;
		int32	xlen;
		maplock_t	maplock;	/* maplock for COMMIT_WMAP */
		pxdlock_t	*pxdlock;	/* maplock for COMMIT_WMAP */

		/* free EA pages from cache */
		xaddr = addressDXD(&ip->i_ea);
		xlen = lengthDXD(&ip->i_ea);
		bmExtentInvalidate(ip, xaddr, xlen);

		/* free EA extent from working block map */
		maplock.index = 1;
		pxdlock = (pxdlock_t *)&maplock;
		pxdlock->flag = mlckFREEPXD;
		PXDaddress(&pxdlock->pxd, xaddr);
		PXDlength(&pxdlock->pxd, xlen);
		txFreeMap(ip, pxdlock, 0, COMMIT_WMAP);
	}

	/*
	 * free ACL
	 */
	if (ip->i_acl.flag & DXD_EXTENT)
	{
		int64	xaddr;
		int32	xlen;
		maplock_t	maplock;	/* maplock for COMMIT_WMAP */
		pxdlock_t	*pxdlock;	/* maplock for COMMIT_WMAP */

		/* free ACL pages from cache */
		xaddr = addressDXD(&ip->i_acl);
		xlen = lengthDXD(&ip->i_acl);
		bmExtentInvalidate(ip, xaddr, xlen);

		/* free ACL extent from working block map */
		maplock.index = 1;
		pxdlock = (pxdlock_t *)&maplock;
		pxdlock->flag = mlckFREEPXD;
		PXDaddress(&pxdlock->pxd, xaddr);
		PXDlength(&pxdlock->pxd, xlen);
		txFreeMap(ip, pxdlock, 0, COMMIT_WMAP);
	}

	/*
	 * free xtree/data (truncate to zero length):
	 * free xtree/data pages from cache, and
	 * free xtree/data blocks from working block map;
	 */
	if (ip->i_size)
		rc = xtTruncate(0, ip, 0, COMMIT_WMAP);

	return rc;
}
