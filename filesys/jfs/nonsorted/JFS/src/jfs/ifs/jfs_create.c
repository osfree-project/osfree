/* $Id: jfs_create.c,v 1.2 2003/09/21 08:42:53 pasha Exp $ */

static char *SCCSID = "@(#)1.20  11/1/99 12:47:55 src/jfs/ifs/jfs_create.c, sysjfs, w45.fs32, fixbld";
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
 *
 *
 * Module: jfs_create.c
 *
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
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_dtree.h"
#include "jfs_xtree.h"
#include "jfs_dnlc.h"
#include "jfs_txnmgr.h"
#include "jfs_proto.h"
#ifdef _JFS_OS2
#include "jfs_ea.h"
#endif	/* _JFS_OS2 */
#include "jfs_debug.h"
#include <uni_inln.h>

#ifdef _JFS_OS2
int     jfs_remove(struct vnode *, struct vnode *, UniChar *, boolean_t,
			struct ucred *);
#endif	/* _JFS_OS2 */

/*
 * NAME:	jfs_create(dvp, vpp, flag, name, mode, vinfop, crp)
 *
 * FUNCTION:	create a file in the parent directory <dvp>
 *		with name = <name> and mode = <mode> and
 *		open it with oflag = <flag>.
 *
 * PARAMETER:	dvp	- parent directory vnode
 *		vpp	- vnode of new file (out)
 *		flag	- open flags from the file pointer.
 *		name	- name of new file
 *		mode	- rwx permission for new file
 *		vinfo	- network information for this file 
 *			  (from the file pointer).
 *		crp	- credential
 *
 * RETURN:	object vnode with a reference
 *
 * note:
 * creat(path, mode) is equivalent to 
 * open(pathe, O_WRONLY|O_CREAT|O_TRUNC, mode);
 * EACCESS: user needs search permission on the parent directory, and,
 * if the file does not exist (i.e., create a new file), 
 * write permission on the parent directory, or
 * if the file exist, permission sepcified by oflag on the file;
 */
jfs_create(
	struct vnode	*dvp,		/* parent directory vnode */
	struct vnode	**vpp,		/* vnode of new file */
	int		flag,		/* flag from open file pointer */
#ifdef _JFS_OS2
	int		oflag,		/* file existence options */
#endif
	UniChar *	name,		/* name of new file */
#ifdef _JFS_OS2
	EAOP		*pcEABuf,
#endif
	struct ucred	*crp)		/* credential */
{
	int32	rc, rc1 = 0;
	int32	tid;		/* transaction id */
	struct vfs	*vfsp = dvp->v_vfsp;
	inode_t	*dip = VP2IP(dvp);	/* parent directory inode */
	inode_t	*ip;		/* child file inode */
	ino_t	ino;
	component_t dname;	/* child file name */
	btstack_t	btstack;
	inode_t	*iplist[2];
	int64	orig_nblocks;					// 226941
#ifdef _JFS_LAZYCOMMIT
	tblock_t	*tblk;					// D230860
#endif
#ifdef _JFS_OS2
	int	mode = 0;
	FEALIST	*pfealist = NULL;
#endif	/* _JFS_OS2 */
	struct dasd_usage	*orig_du;			// D230860

jEVENT(0,("jfs_create: dip:0x%08x name:%s oflag:0x%08x\n", dip, name, oflag));

	*vpp = NULL;

	/* <dvp> must be a directory */
	if ((dip->i_mode & IFMT) != IFDIR)
		return ENOTDIR;

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
	 * validated, the entire FEALIST is validated for format.
	 */
	if (pcEABuf)
	{
		if (rc = jfs_ValidateUserFEAList(pcEABuf, &pfealist,
				&pcEABuf->oError))
		{
			/* something failed -- bail out */
			return rc;
		}
	}
#endif	/* _JFS_OS2 */

	if (dip->i_nlink == 0)
	{
		rc = ENOENT;
		goto errout1;
	}

	/*
	 * scan parent directory for entry/freespace
	 * (dtSearch() returns parent directory page pinned)
	 */
	dname.name = name;
	dname.namlen = UniStrlen(name);
	if (dname.namlen > JFS_NAME_MAX-1)
	{
		rc = ERROR_FILENAME_EXCED_RANGE;
		goto errout1;
	}
// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
	if (isReadOnly(dip))
	{
		rc = EROFS;
		goto errout1;
	}
	/*
	 * Either iAlloc() or txBegin() may block.  Deadlock may occur if we
	 * block while holding dtree page, so we will call these before doing
	 * the directory search.
	 */
	txBegin(dip->i_ipmnt, &tid, 0);   //PS 28092001
	if (rc = iAlloc(vfsp, dip, IFREG|mode, &ip, crp))
              {
              txAbort(tid,0);         //PS21092003
       	txEnd(tid);
		goto errout1;
              }

//PS 28092001	txBegin(dip->i_ipmnt, &tid, 0);

	tblk = &TxBlock[tid];
	tblk->xflag |= COMMIT_CREATE;
	tblk->ip = ip;

#endif /* _JFS_LAZYCOMMIT */
// END D233382
	rc = dtSearch(dip, &dname, &ino, &btstack, JFS_CREATE);
	switch (rc)
	{
	case 0:
		/*
		 *	entry does not exist in directory
		 */

#ifndef _JFS_LAZYCOMMIT						// D233382
		if (isReadOnly(dip))
		{
			rc = EROFS;

			/* release parent directory page */
			BT_PUTSEARCH(&btstack);
			
			goto errout1;
		}
		/*
		 * allocate in-memory+on-disk inode:
		 * (iAlloc() returns new, locked inode)
		 */
		if (rc = iAlloc(vfsp, dip, IFREG|mode, &ip, crp))
		{	
			/* release parent directory page */
			BT_PUTSEARCH(&btstack);

			goto errout1;
		}

		txBegin(dip->i_ipmnt, &tid, 0);
#endif /*  _JFS_LAZYCOMMIT */					// D233382

// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
#endif /* _JFS_LAZYCOMMIT */
// END D230860

		orig_nblocks = dip->i_nblocks;			// F226941

		iplist[0] = dip;
		iplist[1] = ip;
	
		/*
		 * initialize the child XAD tree root in-line in inode
		 */
		xtInitRoot(tid, ip);

		/*
		 * create entry in parent directory
		 * (dtInsert() releases parent directory page)
		 */
		ino = ip->i_number;
		if (rc = dtInsert(tid, dip, &dname, &ino, &btstack))
		{
			/* discard the new inode */
			ip->i_nlink = 0;

			if (rc == EIO)
				txAbort(tid, 1);	/* Marks FS dirty */
			else
				txAbort(tid, 0);	/* FS full */
			txEnd(tid);
			goto errout2;
		}

		/* update child file inode */
		/* ip->i_size = IDATASIZE; */
		imark(ip, IACC|ICHG|IUPD|IFSYNC);	

		/* update parent directory inode */
		imark(dip, ICHG|IUPD|IFSYNC);

#ifdef _JFS_OS2
// BEGIN F226941
		if (orig_du = dip->i_dasdlim)			// D230860
		{
			/*
			 * New inode will inherit dasd usage from parent.
			 * Since the file will stay open, the structure must
			 * be copied from the callers stack to heap storage.
			 */
			ip->i_dasdlim = dlim_copy(dip->i_dasdlim);
			if (ip->i_dasdlim == 0)
			{
				/* discard the new inode */
				ip->i_nlink = 0;
				txAbort(tid, 0);		// D230860
				txEnd(tid);			// D230860
				rc = ENOMEM;
				goto errout2;
			}
			/*
			 * For simplicity, dip & ip should point to the same
			 * structure.
			 */
			dip->i_dasdlim = ip->i_dasdlim;
		}
// END F226941

		if (pfealist)
		{
			rc1 = jfs_InitializeFEAList(ip, tid, pfealist);
				/*
				 * this buffer was allocated by
				 * jfs_ValidateUserFEAList() at twice the
				 * size of the given list to provide buffer
				 * space for eliminating duplicate names
				 */
			jfs_EABufFree((char *)pfealist, (pfealist->cbList << 1));
		}
#endif	/* _JFS_OS2 */

		/*
		 * insert entry for the new file to dnlc
		 */
		ncEnter(dip->i_ipimap, dip->i_number, &dname, ino, NULL);

// BEGIN F226941

		DLIM_UPDATE(tid, dip, dip->i_nblocks + ip->i_nblocks
						- orig_nblocks);
#ifndef _JFS_FASTDASD						// D233382
		/*
		 * If the transaction modified the ancestors of the inode,
		 * parent will be in i_dasdlim array.  Otherwise transaction
		 * only affects new file and parent.
		 */
		if (dip->i_dasdlim && (dip->i_dasdlim->flag & DLIM_LOGGED))
			rc = dasd_commit(tid, ip, 0);
		else
#endif /* _JFS_FASTDASD */					// D233382
			rc = txCommit(tid, 2, &iplist[0], 0);

// END F226941

		txEnd(tid);

#ifdef _JFS_OS2
		/* If we successfully created the file, but did not add the EAs
		 * successfully, we need to cleanup the file creation and return
		 * the error.
		 */
		if (rc1)
		{
// BEGIN D230860
			if ((ip->i_dasdlim) && (ip->i_dasdlim->first_locked))
			{
				/*
				 * txCommit unlocked one or more inodes.
				 * We need to unlock all the directories &
				 * relock them.
				 */
				DLIM_UNLOCK_DIRS(dip, ip->i_dasdlim);
				DLIM_LOCK_DIRS(dip, ip->i_dasdlim);
			}
// END D230860
			jfs_remove(IP2VP(ip), dvp, name, TRUE, crp);
			rc = rc1;
		}

// BEGIN F226941
		/*
		 * ip->i_dasdlim & dip->i_dasdlim are now copies of
		 * the original dasd_usage struct.  The caller will
		 * release the inodes locks using the original struct.
		 * Reset DLIM_DIRS_LOCKED flag in copy so inodes won't
		 * be unlocked twice.
		 */
		if (ip->i_dasdlim)
		{
			/*
			 * In addition, make sure caller does not unlock
			 * inodes already unlocked by txCommit
			 */
			orig_du->first_locked = ip->i_dasdlim->first_locked; // D230860
			ip->i_dasdlim->flag &= ~DLIM_DIRS_LOCKED;
		}
// END F226941
#endif	/* _JFS_OS2 */

		if (rc)
			goto errout2;

		/* is FS in transition ? */
		if (ip->i_xlock & FSXLOCK)
		{
			do {
				ip->i_xlock |= FSXWANT;
				/* release lock */
				EVENT_SLEEP(&ip->i_fsevent, &ip->i_rdwrlock, T_WRITELOCK);
				/* lock reacquired at wakeup */
			} while (ip->i_xlock & FSXLOCK);
		}

		break;

	case EEXIST:
		/*
		 *	entry exist in directory
		 */
#ifdef	_JFS_OS2
// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
		/* We should never get here */
              txAbort(tid,0);         //PS21092003
		txEnd(tid);
		ip->i_nlink = 0;
		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
#endif /* _JFS_LAZYCOMMIT */
// END D233382
		rc = ERROR_FILE_EXISTS;
              break;                  //PS21092003
#endif	/* _JFS_OS2 */

	default:
// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
              txAbort(tid,0);         //PS21092003
		txEnd(tid);
		ip->i_nlink = 0;
		ICACHE_LOCK();
		iput(ip, vfsp);
		ICACHE_UNLOCK();
#endif /* _JFS_LAZYCOMMIT */
// END D233382
		goto errout1;
	}

	/*
	 * open the file object 
	 */
	rc = iOpen(ip, flag, 0, NULL, crp);

	if (rc)
	{
		goto errout2;
	}

	*vpp = IP2VP(ip);

jEVENT(0,("jfs_create: rc:%d\n", rc));
	return rc;

	/*
	 *	unwind on error
	 */
errout2:
	dlim_release(ip);					// F226941

	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, vfsp);
	ICACHE_UNLOCK();

errout1:
#ifdef _JFS_OS2
	if (pfealist)
			/*
			 * this buffer was allocated by
			 * jfs_ValidateUserFEAList() at twice the
			 * size of the given list to provide buffer
			 * space for eliminating duplicate names
			 */
		jfs_EABufFree((char *)pfealist, (pfealist->cbList << 1));
#endif	/* _JFS_OS2 */

jEVENT(0,("jfs_create: rc:%d\n", rc));
	return rc;
}
