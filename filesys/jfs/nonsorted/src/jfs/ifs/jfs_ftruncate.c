/* $Id: jfs_ftruncate.c,v 1.1 2000/04/21 10:58:09 ktk Exp $ */

static char *SCCSID = "@(#)1.14  11/1/99 13:21:48 src/jfs/ifs/jfs_ftruncate.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_ftruncate.c
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
#include "jfs_txnmgr.h"
#include "jfs_cachemgr.h"
#include "jfs_dasdlim.h"					// F226941
#include "jfs_proto.h"
#include "jfs_debug.h"

/* external reference */
extern int32	TlocksLow;	/* TRUE if running out of transaction locks */

/* forward reference */
static int32 iExtend(int32 tid, inode_t *ip, int64 newsize);

/*
 * NAME:	jfs_ftruncate(vp, flags, length, vinfo, crp)
 *
 * FUNCTION:	truncate regular file <vp> to the specified size <length>.
 *
 * PARAMETER:	vp 	_ file to be truncated.
 *		flags	- open flags
 *		length	- new length
 *		vinfo	- unused
 *		crp	- credential
 *
 * RETURN:	
 *
 * note: EINVAL: JFS does NOT support ftruncate() on file types
 * other than regular filei: the effect of ftruncate()/truncate()
 * on file types other than regular file is unspecified. [XPG4.2].
 * 
 * LFS validation ?: the file must be open for writing.
 */
jfs_ftruncate(
	struct vnode	*vp,		/* file */
	int32		flags,		/* open flags */
	offset_t	length,		/* new length */
	caddr_t		vinfo,		/* Gfs specific inofo */
	struct ucred	*crp)		/* credential structure */
{
	int64	orig_nblocks;					// F226941
	int32	rc = 0;
	int32	tid;
	inode_t	*ip = VP2IP(vp);
	extern int iwritelockx();
	extern int iwriteunlockx();

	if ((ip->i_mode & IFMT) != IFREG)
		return EINVAL;

	IWRITE_LOCK(ip);

	/*
	 * verify mandatory/enforcement-mode file section locking
	 */
/*
	if (ENF_LOCK(ip->i_mode))
	{
		struct eflock bf;

		bf.l_type = F_WRLCK;
		bf.l_whence = 0;
		bf.l_start = length;
		bf.l_len = 0;      
		bf.l_pid = U.U_procp->p_pid;
		bf.l_sysid = 0;
		bf.l_vfs = MNT_XJFS;

		if ((rc = common_reclock(IP2GP(ip),
				ip->i_size,
				length,
				&bf,
		      		((flags & (FNDELAY|FNONBLOCK)) ?
					INOFLCK : SLPFLCK|INOFLCK),
				0,
				0,
				iwritelockx,
				iwriteunlockx)) ||
		    bf.l_type != F_UNLCK)
		{
			rc = rc ? rc : EAGAIN;
			goto out;
		}
	}
*/

	/*
	 *	truncate down
	 */
	if (length < ip->i_size)
	{
// BEGIN D230860
		/*
		 * All inodes must be locked before beginning transaction
		 */
		if (ip->i_dasdlim &&
		   (ip->i_dasdlim->flag & DLIM_DIRS_LOCKED) == 0)
			dasd_write_lock(0, ip->i_dasdlim);
// END D230860

		txBegin(ip->i_ipmnt, &tid, 0);

		orig_nblocks = ip->i_nblocks;			// F226941

		if (rc = iTruncate(tid, ip, (int64)length))
		{
			txEnd(tid);
			goto out;
		}
		ip->i_flag |= IFSYNC;

		DLIM_UPDATE(tid, ip , ip->i_nblocks - orig_nblocks);  // F226941

		rc = DLIM_TXCOMMIT(tid, ip, 0);			// F226941

		txEnd(tid);

		goto out;
	}
	/*
	 *	truncate up (extension)
	 */
	else if (length > ip->i_size)
	{
#ifndef _JFS_OS2
		/* check u_limit */
		if (length > U.U_limit)
		{
			rc = EFBIG;
			goto out;
		}
#endif /* _JFS_OS2 */

// BEGIN F226941
		orig_nblocks = ip->i_nblocks;			// D233382
#ifndef _JFS_FASTDASD						// D233382
		if (ip->i_dasdlim)
		{
			/*
			 * All inodes must be locked before begining
			 * transaction
			 */
			if ((ip->i_dasdlim->flag & DLIM_DIRS_LOCKED) == 0) // D230860
				dasd_write_lock(0, ip->i_dasdlim);  // D230860

			txBegin(ip->i_ipmnt, &tid, 1);
		}
		else
#endif /* _JFS_FASTDASD */					// D233382
		{
#ifdef _JFS_LAZYCOMMIT
			txBeginAnon(ip->i_ipmnt);		// D233382
#endif
			tid = 0;
		}
// END F226941

		if (rc = iExtend(tid, ip, (int64)length))
		{
#ifndef _JFS_FASTDASD						// D233382
			if (tid)				// F226941
				txEnd(tid);			// F226941
#endif								// D233382
			goto out;
		}

		ip->i_flag |= IFSYNC;

// BEGIN F226941
		DLIM_UPDATE(tid, ip, ip->i_nblocks - orig_nblocks);
#ifndef _JFS_FASTDASD						// D233382
		if (tid)
		{
			rc = DLIM_TXCOMMIT(tid, ip, 0);
			txEnd(tid);
		}
		else
#endif /* _JFS_FASTDASD */					// D233382
// END F226941
		/* if available tlocks are scarce and inode has anonymous
		 * tlocks, OR if we modified a metadata page that is on the
		 * synclist, commit inode.
	 	 */
        	if ((TlocksLow && ip->i_atlhead) ||
		    (ip->i_flag & ISYNCLIST))
		{
			txBegin(ip->i_ipmnt, &tid, COMMIT_FORCE);
			rc = txCommit(tid, 1, &ip, 0);		// F226941
			txEnd(tid);
		}
	}
	/*
	 *	no change in size
	 */
	else /* (length == ip->i_size) */
		/* mark the inode as changed and updated (ala BSD) */
		imark(ip, ICHG|IUPD);
out:
	DLIM_WRITE_UNLOCK(ip, ip->i_dasdlim);			// F226941

	return rc;
}


/*
 * NAME:	iTruncate(tid, ip, newsize)
 *
 * FUNCTION:    truncate up/down a regular file to specified size, or 
 *		truncate down directory or symbolic link to zero length 
 *		(length is ignored and assumed to be zero if the object
 * 		is a directory or symbolic link). 
 *		if length is > 0, the file must be open (i.e.bound to a 
 *		VM segment). 
 *		return 0 if type is not one of these.
 *
 * PARAMETER:	ip	- inode to truncate
 *		newsize	- new size of the file
 *
 * RETURN:
 *			
 * SERIALIZATION: the IWRITE_LOCK is held on entry/exit.
 */
int32 iTruncate(
	int32	tid, 
	inode_t	*ip, 
	int64	newsize)
{
	int32	rc;
	int64	nbytes;
	int64	pfirst, bfirst;
/*
printf("iTruncate: ip:0x%08x eof:0x%08x:0x%08x\n", ip, newsize); 
*/
	if ((ip->i_mode & IFMT) != IFREG)
		return EINVAL;

	/* if truncating to a non-zero newsize, make sure
	 * file is bound to its cache control object.
	 */
	if (newsize != 0 && ip->i_cacheid == NULL)
	{
		if (rc = iBindCache(ip))
	 		return rc;
	}

	/*
	 * if the newsize is not an integral number of pages,
	 * the file between newsize and next page boundary will 
	 * be cleared.
	 * if truncating into a file hole, it will cause
	 * a full block to be allocated for the logical block.
	 */

	/*
	 * if the file was commited with zero link count before
	 * (temporary file), its persistent resources were already
	 * freed at that time: just truncate/free working resources;
	 */
	if (ip->i_cflag & COMMIT_NOLINK)
	{
		rc = xtTruncate(0, ip, newsize, COMMIT_WMAP);
		return rc;
	}

	/*
	 * delete pages and set new size.
	 */
	rc = xtTruncate(tid, ip, newsize, COMMIT_TRUNCATE|COMMIT_PWMAP);

	imark(ip, ICHG|IUPD);

	return rc;
}


/*
 * NAME:	iExtend(tid, ip, newsize)
 *
 * FUNCTION:	extend inode size for ip to newsize.  extension is
 * 		performed by writing a null bytes to offset (newsize-1)
 *		within the file through a call to writei().
 *
 *		this routine is used by itrunc and iclear to 'trunc up'
 *		a regular file.
 *
 * PARAMETERS:	ip	- pointer to inode 
 *		newsize	- new file size.
 *
 * RETURN:	0	- file sucessfully extended.
 *		E2BIG	- new size is beyond maximum file size
 *		errnos returned by writei().
 */
static 
iExtend(
	int32	tid,
	inode_t	*ip,
	int64	newsize)
{
/*
printf("iExtend: ip:0x%08x eof:0x%08x:0x%08x\n", ip, newsize); 
*/
	return ExtendI(ip, newsize, TRUE);
}
