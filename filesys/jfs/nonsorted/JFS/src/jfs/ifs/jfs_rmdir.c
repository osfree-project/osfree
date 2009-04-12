/* $Id: jfs_rmdir.c,v 1.2 2003/09/21 08:43:00 pasha Exp $ */

static char *SCCSID = "@(#)1.17  11/1/99 14:26:52 src/jfs/ifs/jfs_rmdir.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_rmdir.c
 */

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_dnlc.h"
#include "jfs_proto.h"
#include "jfs_bufmgr.h"
#include "jfs_debug.h"
#include <uni_inln.h>

/*
 * External declarations
 */
void	iwritelocklist(int, ... );

/*
 * NAME:	jfs_rmdir(vp, dvp, name, crp)
 *
 * FUNCTION:	remove a link to child directory <vp> with name = <name> 
 *		from parent directory <dvp>
 *
 * PARAMETER:	vp 	- object directory vnode 
 *		dvp	- parent directory vnode
 *		name	- object name
 *		crp	- credential
 *
 * RETURN:	EINVAL	- if name is . or ..
 *		EINVAL  - if . or .. exist but are invalid.
 *		errors from subroutines
 *
 * note:
 * if other threads have the directory open when the last link 
 * is removed, the "." and ".." entries, if present, are removed before 
 * rmdir() returns and no new entries may be created in the directory, 
 * but the directory is not removed until the last reference to 
 * the directory is released (cf.unlink() of regular file).
 *
 * refer to vn_remove() (jfs_remove.c) for access permission.
 */
jfs_rmdir(
	struct vnode	*vp,
	struct vnode	*dvp,
	UniChar		*name,
	struct ucred	*crp)
{
	int32 	rc;
	int32	tid;		/* transaction id */
	inode_t *dip = VP2IP(dvp);
	inode_t *ip = VP2IP(vp);
	ino_t	ino;
	component_t dname;	
	inode_t	*iplist[2];
	int64	orig_nblocks;					// F226941
#ifdef _JFS_LAZYCOMMIT
	tblock_t *tblk;						// D230860
#endif

jEVENT(0,("jfs_rmdir: dip:0x%08x name:%s\n", dip, name));

	if ((ip->i_mode & IFMT) != IFDIR)
		return ENOTDIR;

	/* prevent suicide and parenticide */
	if (name[0] == '.')
	{
		/* removing ".." */
		if (name[1] == '.' && name[2] == '\0')
			return EEXIST;
		/* removing "." */
		else if (name[1] == '\0')
			return EINVAL;
	}

	if (isReadOnly(ip))
		return EROFS;

	txBegin(dip->i_ipmnt, &tid, 0);

	orig_nblocks = dip->i_nblocks + ip->i_nblocks;		// F226941

	/* directory must be empty to be removed */
    	if (rc = dtEmpty(ip))
		goto out;
 
	/*
	 * delete name entry from dnlc (before directory update)
	 */
	dname.name = name;
	dname.namlen = UniStrlen(name);
	ncDelete(dip->i_ipimap, dip->i_number, &dname);

	iplist[0] = dip;
	iplist[1] = ip;

#ifdef _JFS_LAZYCOMMIT
	tblk = &TxBlock[tid];					// D230860
	tblk->xflag |= COMMIT_DELETE;				// D230860
	tblk->ip = ip;						// D230860
#endif /* _JFS_LAZYCOMMIT */

	/*
	 * delete the entry of target directory from parent directory
	 */
	ino = ip->i_number;
	if (rc = dtDelete(tid, dip, &dname, &ino, JFS_REMOVE))
	{
		if (rc == EIO)
			txAbort(tid, 1);
		goto out;
	}

	/* mark the target directory as deleted */
	ip->i_nlink = 0;
	imark(ip, ICHG|IFSYNC);		

	/* update parent directory's link count corresponding
	 * to ".." entry of the target directory deleted
	 */
	dip->i_nlink--;
	imark(dip, ICHG|IUPD|IFSYNC);

	/* free EA from both persistent and working map */
	if (ip->i_ea.flag & DXD_EXTENT)
	{
		/* free EA pages */
		bmExtentInvalidate(ip, addressDXD(&ip->i_ea), lengthDXD(&ip->i_ea));

		txEA(tid, ip, &ip->i_ea, NULL);
	}
	ip->i_ea.flag = 0;

	/* free ACL from both persistent and working map */
	if (ip->i_acl.flag & DXD_EXTENT)
	{
		/* free ACL pages */
		bmExtentInvalidate(ip, addressDXD(&ip->i_acl),
				   lengthDXD(&ip->i_acl));

		txEA(tid, ip, &ip->i_acl, NULL);
	}
	ip->i_acl.flag = 0;

	DLIM_UPDATE(tid, dip, dip->i_nblocks - orig_nblocks);	// F226941

#ifndef _JFS_FASTDASD						// D233382
	if (ip->i_dasdlim && (ip->i_dasdlim->flag & DLIM_LOGGED))  // F226941
		rc = dasd_commit(tid, ip, 0);			// F226941
	else							// F226941
#endif /* _JFS_FASTDASD */					// D233382
		rc = txCommit(tid, 2, &iplist[0], 0);
	
out:
	txEnd(tid);

jEVENT(0,("jfs_rmdir: rc:%d\n", rc));
	return rc;
}


#ifdef	_JFS_OS2
/*
 *	jfs_xrmdir()
 *
 * caller holds locks on dip and ip;
 */
jfs_xrmdir(
	struct inode	*dip,
	struct inode	*ip,
	UniChar		*name)
{
	int64	orig_nblocks;					// F226941
	int32 	rc;
	int32	tid;		/* transaction id */
	ino_t	ino;
	component_t dname;	
	inode_t	*iplist[2];
#ifdef _JFS_LAZYCOMMIT
       struct tblock *delTxBlk;   //PS 28092001
#endif

	orig_nblocks = ip->i_nblocks + dip->i_nblocks;		// F226941
	/*
	 * delete name entry from dnlc (before directory update)
	 */
	dname.name = name;
	dname.namlen = UniStrlen(name);
	ncDelete(dip->i_ipimap, dip->i_number, &dname);

	txBegin(dip->i_ipmnt, &tid, 0);

	iplist[0] = dip;
	iplist[1] = ip;
//PS 28092001 Begin
#ifdef _JFS_LAZYCOMMIT
       delTxBlk         =  &TxBlock[tid];
       delTxBlk->xflag |= COMMIT_DELETE;
       delTxBlk->ip     = ip;
#endif
//PS 28092001 end
	/*
	 * delete the entry of target directory from parent directory
	 */
	ino = ip->i_number;
	if (rc = dtDelete(tid, dip, &dname, &ino, JFS_REMOVE))
	{
		if (rc == EIO)
			txAbort(tid, 1);
		goto out;
	}

	/* mark the target directory as deleted */
	ip->i_nlink = 0;
	imark(ip, ICHG|IFSYNC);		

	/* update parent directory's link count corresponding
	 * to ".." entry of the target directory deleted
	 */
	dip->i_nlink--;
	imark(dip, ICHG|IUPD|IFSYNC);

// BEGIN F226941
// BEGIN D230860
	if (ip->i_ea.flag & DXD_EXTENT)
	{
		/* free EA pages */
		bmExtentInvalidate(ip, addressDXD(&ip->i_ea),
				   lengthDXD(&ip->i_ea));

		txEA(tid, ip, &ip->i_ea, NULL);
	}
	DLIM_UPDATE(tid, dip, dip->i_nblocks - orig_nblocks);
// END D230860

#ifndef _JFS_FASTDASD						// D233382
	if (ip->i_dasdlim && (ip->i_dasdlim->flag & DLIM_LOGGED))
		rc = dasd_commit(tid, ip, 0);
	else
#endif /* _JFS_FASTDASD */					// D233382
// END F226941
		rc = txCommit(tid, 2, &iplist[0], 0);
	
out:
	txEnd(tid);

	return rc;
}
#endif	/* _JFS_OS2 */
