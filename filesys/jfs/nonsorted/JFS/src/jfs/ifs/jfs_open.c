/* $Id: jfs_open.c,v 1.1.1.1 2003/05/21 13:38:00 pasha Exp $ */

static char *SCCSID = "@(#)1.17  11/1/99 14:00:18 src/jfs/ifs/jfs_open.c, sysjfs, w45.fs32, fixbld";
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
 * Module: jfs_open.c
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
#include "jfs_dasdlim.h"					// F226941
#include "jfs_proto.h"
#include "jfs_debug.h"

#ifdef _JFS_OS2
#include "jfs_ea.h"
#define OPEN_FLAGS_LARGEFILE	0x08000000 /* Officially defined in bsedosp.h */
#endif	/* _JFS_OS2 */

/*
 * NAME:	jfs_open(vp, flag, ext, vinfop, crp)
 *
 * FUNCTION:	open a file <vp> with open flag <flag>.
 *
 * PARAMETER:	vp	- vnode to open
 *		flag	- open flags from the file pointer.
 *		ext	- external data for device driver.
 *		vinfop	- pointer to the vinfo field for the open file
 *		crp	- credential
 *
 * RETURN:	ENOENT	- non-positive link count
 *		errors from subroutines
 *
 * note: silly story of open/type/mode flag ...
 * at user level, open flag (O_xxxx) is defined by fcntl.h,
 * file type and permission flags (S_xxxx defined by 
 * sys/mode.h included in sys/stat.h);
 * in LFS, open flag is converted to file flag (Fxxxx defined 
 * by fcntl.h) and to vnode type (Vxxxx defined by sys/vnode.h);
 * in PFS, file type and permission is defined by Ixxxx flag
 * (also defined by sys/mode.h) in addition to file flag and
 * vnode type;
 */
jfs_open(
	struct vnode	*vp,		/* object vnode */
	int32		flag,		/* open(2) flag	*/
	uint32		oflag,		/* file existence options */
	EAOP		*pcEABuf,
	struct ucred	*crp)		/* credential */
{
	int32	rc, rc1 = 0;	
	inode_t *ip = VP2IP(vp);	/* object inode */
	int32 	type;
#ifdef _JFS_OS2
	FEALIST	*pfealist = NULL;
	inode_t	*iplist[1];
#endif	/* _JFS_OS2 */

jEVENT(0,("jfs_open(ip:0x%08x, flag:0x%08x)\n", ip, flag));

	/* validate file type */
	type = ip->i_mode & IFMT;
	if (! (type == IFREG || type == IFDIR))
	{
jEVENT(0,("jfs_open(ip:0x%08x, flag:0x%08x ERROR(EINVAL)\n", ip, flag));
		return EINVAL;
	}

#ifdef _JFS_OS2
	/* validate the EAOP buffer, FEALIST size storage location and the
	 * entire FEALIST storage area.  Once all the storage has been
	 * validated, the entire FEALIST is validated for format and the size is
	 * computed and compared against the limit.
	 */
	if (pcEABuf && (oflag & OPEN_ACTION_REPLACE_IF_EXISTS))
	{
		if (rc = jfs_ValidateUserFEAList(pcEABuf,
				&pfealist, &pcEABuf->oError))
		{
			/* something failed -- bail out */
			return rc;
		}
	}
#endif	/* _JFS_OS2 */

	/* named file does not exist anymore ? */
	if (ip->i_nlink == 0)
	{
		rc = ENOENT;
		goto out;
	}

	/* validate open access permission */
	if (rc = iOpenAccess(ip, flag, crp))
		goto out;

#ifdef _JFS_OS2
	rc = iOpen(ip, flag, oflag, pfealist, crp);
#endif

out:
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

	/* If opening a file readonly, and we aren't explicitly told that
	 * the access is random, perform readahead of first 32K
	 */
	if ((rc == 0) &&
	    ((flag & (OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY)) ==
	     OPEN_ACCESS_READONLY) &&
	    ((flag & OPEN_FLAGS_RANDOM) == 0))
	{
		if (ip->i_size > 0)
			cmKickStart(ip);
	}

jEVENT(0,("jfs_open(rc:%d)\n", rc));

	return rc;
}


/*
 * NAME:	iOpenAccess()
 *
 * FUNCTION:	check access permissions before an open
 *		called by jfs_open() and jfs_create().
 *
 * PARAMETER:	ip	- inode in question
 *		flag	- permissions to check
 *		crp	- credential
 *
 * RETURN:
 */
int32 iOpenAccess(
	inode_t		*ip,	/* inode to check access for */
	int32		flag,	/* open flags */
	struct ucred	*crp)
{
	int32 rc = 0;
	int32 mode;
	struct gnode *gp = IP2GP(ip);		

	if (((flag & OPEN_FLAGS_LARGEFILE) == 0) &&
	    (ip->i_size > 0x7fffffff))
		return ERROR_ACCESS_DENIED;

	/* set up mode for call to iAccess  */
	switch (flag & (OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY))
	{
	    case (OPEN_ACCESS_READWRITE):
		mode = (IREAD|IWRITE);
		break;

	    case (OPEN_ACCESS_WRITEONLY):
		mode = IWRITE;
		break;

	    default:
		mode = IREAD;
	}

	/* check for sharing violation */
	if ((mode & IREAD) && (gp->gn_drcnt))
		rc = ETXTBSY;
	else if ((mode & IWRITE) && (gp->gn_dwcnt))
		rc = ETXTBSY;
	else if (gp->gn_flags & GNF_NSHARE)
		rc = ETXTBSY;

	/* check permissions */
	else
		rc = iAccess(ip, mode, crp);

	return rc;
}


/*
 * NAME:	iOpen()
 *
 * FUNCTION:	common function that opens a file.
 *		called by jfs_open() and jfs_create().
 *
 * PARAMETER:	ip	- inode to open
 *		flag	- open flags from the file pointer.
 *		ext	- external data used by the device driver.
 *		crp	- credential
 *
 * RETURN:	EBUSY	- if truncating a mapped file
 *		errors from subroutines		
 *
 * note: at most a single commit may occur in jfs_open() and jfs_create()
 * as FEXEC and FTRUNC are mutually exclusive.
 */
int32 iOpen(
	inode_t *ip,		/* inode of file to open */
	int32	flag,		/* open(2) flags */
	int32	tflag,		/* trucation flag */
	FEALIST	*pfealist,	/* EAs if truncating */
	struct ucred *crp)	/* credential */
{
	int32	rc = 0;		
	struct gnode *gp;	/* gnode pointer from inode	*/
	int64	orig_nblocks;					// F226941
	int32	tid;

	/* bind the newly opened file to a cache control object */
	if ((ip->i_mode & IFMT) == IFREG && ip->i_cacheid == NULL)
	{
		if (rc = iBindCache(ip))
			return rc;
	}

	/* if open/create FTRUNC, truncate to zero length */
	if (tflag & OPEN_ACTION_REPLACE_IF_EXISTS)
	{
		if (isReadOnly(ip))
			return EROFS;

		orig_nblocks = ip->i_nblocks;			// F226941
// BEGIN D230860
		/*
		 * Cannot begin transaction until all inodes are locked
		 */
		if (ip->i_dasdlim &&
		    (ip->i_dasdlim->flag & DLIM_DIRS_LOCKED) == 0)
			dasd_write_lock(0, ip->i_dasdlim);
// END D230860
		txBegin(ip->i_ipmnt, &tid, 0);

		if (rc = iTruncate(tid, ip, 0))
		{
			txEnd(tid);
			return rc;
		}

		imark(ip, IACC|ICHG|IUPD|IFSYNC);		// D233382

#ifdef _JFS_OS2
		rc = jfs_InitializeFEAList(ip, tid, pfealist);

		/* If we successfully truncated the file, but failed adding the
		 * EA's, we are in a bad state.  Punt...
		 */
		if (rc)
		{
			txAbort(tid, 0);
			txEnd(tid);
			return rc;
		}
#endif	/* _JFS_OS2 */
		DLIM_UPDATE(tid, ip, ip->i_nblocks - orig_nblocks);  // F226941

		rc = DLIM_TXCOMMIT(tid, ip, 0);			// F226941
		txEnd(tid);
		if (rc)
			return rc;
	}

	/*
	 * update per open counters
	 */
	gp = IP2GP(ip);

#ifdef	_JFS_OS2

	/* keep count of opens by type
	 */
	switch (flag & (OPEN_ACCESS_READWRITE | OPEN_ACCESS_WRITEONLY))
	{
	    case (OPEN_ACCESS_READWRITE):
		gp->gn_wrcnt++;
		gp->gn_rdcnt++;
		break;

	    case (OPEN_ACCESS_WRITEONLY):
		gp->gn_wrcnt++;
		break;

	    default:
		gp->gn_rdcnt++;
	}

	/* set up sharing mode
	 */
	switch (flag & (OPEN_SHARE_DENYREAD))
	{
	    case(OPEN_SHARE_DENYWRITE):
		gp->gn_dwcnt++;
		break;

	    case(OPEN_SHARE_DENYREAD):
		gp->gn_drcnt++;
		break;

	    case(OPEN_SHARE_DENYREADWRITE):
		gp->gn_flags |= GNF_NSHARE;
		break;
	}
	if ((flag & OPEN_FLAGS_LARGEFILE) == 0)
		gp->gn_lgcnt++;

#endif	/* _JFS_OS2 */

	return rc;
}
