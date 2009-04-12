/* $Id: jfs_mkdir.c,v 1.1 2000/04/21 10:58:13 ktk Exp $ */

static char *SCCSID = "@(#)1.19  11/1/99 13:54:41 src/jfs/ifs/jfs_mkdir.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_mkdir.c
 *
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
#include "jfs_dnlc.h"
#include "jfs_txnmgr.h"
#include "jfs_dasdlim.h"					// F226941
#include "jfs_debug.h"
#include "jfs_proto.h"

	#include <uni_inln.h>

#ifdef _JFS_OS2
#include "jfs_ea.h"

int     jfs_xrmdir(struct inode *, struct inode *, UniChar *);
int	jfs_CopyACL(inode_t *, inode_t *, int32);

#endif	/* _JFS_OS2 */

/*
 * NAME:	jfs_mkdir(dvp, name, mode, crp)
 *
 * FUNCTION:	create a child directory in the parent directory <dvp>
 *		with name = <name> and mode = <mode>
 *
 * PARAMETER:	dvp 	- parent directory vnode
 *		name	- name of child directory
 *		mode	- create mode (rwxrwxrwx).
 *		crp	- credential
 *
 * RETURN:	Errors from subroutines
 *
 * note:
 * EACCESS: user needs search+write permission on the parent directory
 */
jfs_mkdir(
	struct vnode	*dvp,
	UniChar		*name,
	mode_t		mode,
#ifdef _JFS_OS2
        EAOP		*pEABuf,
#endif	/* _JFS_OS2 */
	struct ucred	*crp)
{
	int32 	rc = 0, rc1 = 0;
	int32	tid;		/* transaction id */
	struct vfs	*vfsp = dvp->v_vfsp;
	inode_t	*dip = VP2IP(dvp); /* parent directory inode */
	struct dasd_usage	*du;				// F226941
	inode_t *ip = NULL;	/* child directory inode */
	ino_t	ino;
	component_t dname;	/* child directory name */
	btstack_t	btstack;
	inode_t	*iplist[2];
	int64	orig_nblocks;					// F226941
#ifdef _JFS_LAZYCOMMIT
	tblock_t *tblk;						// D230860
#endif
#ifdef _JFS_OS2
	FEALIST		*pfealist = NULL;
#endif	/* _JFS_OS2 */

NOISE(1,("jfs_mkdir: dip:0x%08x name:%s\n", dip, name));

	/*
	 * the named file exists for "." or ".."
	 */
	if (name[0] == '.')
	{
		if ((name[1] == '.' && name[2] == '\0') ||
		    name[1] == '\0')
			return EEXIST;
	}

#ifdef _JFS_OS2
	/* validate the EAOP buffer, FEALIST size storage location and the
	 * entire FEALIST storage area.  Once all the storage has been
	 * validated, the entire FEALIST is validated for format and the size is
	 * computed and compared against the limit.
	 */
	if (pEABuf)
	{
		if (rc = jfs_ValidateUserFEAList(pEABuf, &pfealist,
				&pEABuf->oError))
		{
			/* something failed -- bail out */
			return rc;
		}
	}
#endif	/* _JFS_OS2 */

	if (dip->i_nlink == 0)
	{
		rc = ENOENT;
		goto out1;
	}

	/* link count overflow on parent directory ? */
	if (dip->i_nlink >= LINK_MAX)
	{
		rc = EMLINK;
		goto out1;
	}

	/*
	 * search parent directory for entry/freespace
	 * (dtSearch() returns parent directory page pinned)
	 */
	dname.name = name;
	dname.namlen = UniStrlen(name);
	if (dname.namlen > JFS_NAME_MAX-1)
	{
		rc = ERROR_FILENAME_EXCED_RANGE;
		goto out1;
	}

// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
	if (isReadOnly(dip))
	{
		rc = EROFS;
		goto out1;
	}

	/*
	 * Either iAlloc() or txBegin() may block.  Deadlock can occur if we
	 * block there while holding dtree page, so we allocate the inode &
	 * begin the transaction before we search the directory.
	 */
	if (rc = iAlloc(vfsp, dip, IFDIR|mode|IDIRECTORY, &ip, crp))
		goto out1;

	txBegin(dip->i_ipmnt, &tid, 0);

	if (rc = dtSearch(dip, &dname, &ino, &btstack, JFS_CREATE))
	{
		ip->i_nlink = 0;
		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
		txEnd(tid);
		goto out1;
	}

	tblk = &TxBlock[tid];					// D230860
	tblk->xflag |= COMMIT_CREATE;				// D230860
	tblk->ip = ip;						// D230860

#else /* ! _JFS_LAZYCOMMIT */
// END D233382

	if (rc = dtSearch(dip, &dname, &ino, &btstack, JFS_CREATE))
		goto out1;

	if (isReadOnly(dip))
	{
		rc = EROFS;

		/* release parent directory page */
		BT_PUTSEARCH(&btstack);
		goto out1;
	}

	/*
	 * allocate on-disk/in-memory inode for child directory:
	 * (iAlloc() returns new, locked inode)
	 */
	if (rc = iAlloc(vfsp, dip, IFDIR|mode|IDIRECTORY, &ip, crp))
	{
		/* release parent directory page */
		BT_PUTSEARCH(&btstack);

jEVENT(0,("jfs_mkdir: iAlloc error(%d)\n", rc));
		goto out1;
	}

	txBegin(dip->i_ipmnt, &tid, 0);
#endif /* _JFS_LAZYCOMMIT */

	du = ip->i_dasdlim = dip->i_dasdlim;			// F226941

	orig_nblocks = dip->i_nblocks;				// F226941

	iplist[0] = dip;
	iplist[1] = ip;
	
	/*
	 * initialize the child directory in-line in inode
	 */
	dtInitRoot(tid, ip, dip->i_number);

	/*
	 * create entry in parent directory for child directory
	 * (dtInsert() releases parent directory page)
	 */
	ino = ip->i_number;
	if (rc = dtInsert(tid, dip, &dname, &ino, &btstack))
	{	
		/* discard new directory inode */
		ip->i_nlink = 0;

		if (rc == EIO)
			txAbort(tid, 1);	/* Marks Filesystem dirty */
		else
			txAbort(tid, 0);	/* Filesystem full */
		txEnd(tid);
		goto out3;
	}

	/* update child directory inode */
// BEGIN F226941
	setDASDLIMIT(&ip->i_DASD, 0);
	setDASDUSED(&ip->i_DASD, 0);
// END F226941

	ip->i_nlink++;	/* for '.' */ 
	imark(ip, IACC|ICHG|IUPD|IFSYNC);	

	/* update parent directory inode */
	dip->i_nlink++; /* for '..' from child directory */
	imark(dip, ICHG|IUPD|IFSYNC);	

	/*
	 * insert entry for the new file to dnlc
	 */
	ncEnter(dip->i_ipimap, dip->i_number, &dname, ino, NULL);

#ifdef _JFS_OS2

	if ((rc1 == 0) && pfealist)				// F226941
		rc1 = jfs_InitializeFEAList(ip, tid, pfealist);

	if ((rc1 == 0) && (dip->i_acl.flag) && (dip->i_acl.size))
		rc1 = jfs_CopyACL(ip, dip, tid);

#endif	/* _JFS_OS2 */

// BEGIN FF226941
	DLIM_UPDATE(tid, dip, dip->i_nblocks + ip->i_nblocks - orig_nblocks);
	setDASDUSED(&ip->i_DASD, ip->i_nblocks);		// D233382
#ifndef _JFS_FASTDASD						// D233382
	/*
	 * If the transaction modified the ancestors of the inode, the
	 * parent will be in the dasd usage list.  Otherwise, transaction
	 * will only change new directory and parent.
	 */
	if (du && (du->flag & DLIM_LOGGED))
		rc = dasd_commit(tid, ip, 0);
	else
#endif /* _JFS_FASTDASD */					// D233382
// END FF226941

		rc = txCommit(tid, 2, &iplist[0], 0);
	txEnd(tid);

#ifdef _JFS_OS2

	/* If we successfully added the directory, but failed adding the EA or
	 * ACL, we must cleanup the created directory entry and return the
	 * error.
	 */
	if (rc1)
	{
// BEGIN D230860
		if ((du) && (du->first_locked))
		{
			/*
			 * txCommit unlocked one or more inodes.
			 * We need to unlock all the directories &
			 * relock them.
			 */
			dip->i_dasdlim = 0;
			DLIM_WRITE_UNLOCK_DETACH(ip, du);
			DLIM_WRITE_LOCK_ATTACH(ip, du);
			dip->i_dasdlim = du;
		}
// END D230860
		jfs_xrmdir(dip, ip, name);
		rc = rc1;
	}

#endif	/* _JFS_OS2 */

out3:
	ip->i_dasdlim = 0;					// F226941
	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, vfsp);
	ICACHE_UNLOCK();

out1:

#ifdef _JFS_OS2
		/*
		 * this buffer was allocated by
		 * jfs_ValidateUserFEAList() at twice the
		 * size of the given list to provide buffer
		 * space for eliminating duplicate names
		 */
	if (pfealist)
		jfs_EABufFree((char *)pfealist, (pfealist->cbList << 1));
#endif	/* _JFS_OS2 */

NOISE(1,("jfs_mkdir: rc:%d\n", rc));
	return rc;
}
