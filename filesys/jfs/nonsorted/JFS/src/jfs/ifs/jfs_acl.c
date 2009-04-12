/* $Id: jfs_acl.c,v 1.1.1.1 2003/05/21 13:36:12 pasha Exp $ */

static char *SCCSID = "@(#)1.14  7/6/99 09:43:56 src/jfs/ifs/jfs_acl.c, sysjfs, w45.fs32, fixbld";
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
 *
 *
*/

/*
 * Change History :
 *
 */

/*
 * Module: jfs_acl.c
 *
 * Overall design --
 *
 * Format:
 *
 *   JFS treats the ACL as string of bytes.  The format is known only to the
 *   caller.
 *
 * Operations:
 *
 *   The ACL may be set or retrieved.  Deleting is accomplished by setting a
 *   zero-length ACL.  An enumeration function returns files containing ACL's
 *   and all subdirectories within a directory along with their ACL's, one at
 *   a time in a findfirst/findnext manner.
 *
 * Storage:
 *
 *   On-disk:
 *
 *     ACL's are stored on disk using blocks allocated by dbAlloc() and
 *     written directly thru the buffer pool without going thru the cache
 *     manager.  An ACL may be in-lined in the inode if there is
 *     sufficient room available.
 *
 *     The functions jfs_ReadACL() and jfs_WriteACL() are responsible for
 *     moving ACL's between disk and memory.  jfs_WriteACL() will perform
 *     the required disk space allocation.  Freeing allocated on-disk
 *     storage is the responsibility of the functions which invoke
 *     jfs_WriteACL().
 *
 * Validation: none
 *
 *   Format:
 *
 *     JFS is not concerned with the format of the ACL.  No validation is done
 *     on the format.
 *
 *   User Buffer
 *
 *     Data is transferred to and from user buffers by KernCopyOut and
 *     KernCopyIn.  These routines verify the users buffers.
 *
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#define INCL_DOSPROCESS

#include <os2.h>

#include "jfs_os2.h"
#include "jfs_inode.h"
#include "jfs_dmap.h"
#include "jfs_debug.h"
#include "jfs_bufmgr.h"
#include "jfs_cachemgr.h"
#include "jfs_dtree.h"
#include "jfs_lock.h"
#include "jfs_dasdlim.h"					// F226941
#include "jfs_proto.h"
#include <priv/jfs_acl.h>

#include <uni_inln.h>

int32	txFileSystemDirty(inode_t *);

/*
 * Global data
 */

struct enum_instance *enum_anchor = 0;

SPINLOCK_T	enum_lock;
#define ENUM_LOCK()		SPINLOCK_LOCK(&enum_lock)
#define ENUM_UNLOCK()		SPINLOCK_UNLOCK(&enum_lock)

UniChar	*lonestar = L"*";					// F226941

/*
 * external declarations
 */
extern pool_t	*dirent_pool;
extern pool_t	*unipool;

int32 jfs_strfromUCS(char *, UniChar *, int32, int32);

/*
 * NAME: jfs_WriteACLInLine
 *
 * FUNCTION: Attempt to write an ACL inline if area is available
 *
 * PRE CONDITIONS:
 *	Already verified that the specified ACL is small enough to fit inline
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	acl	- ACL pointer
 *	length  - length in bytes of acl
 *	acl_dxd	- dxd_t structure to be filled in with necessary ACL information
 *		  if we successfully copy the ACL inline
 *
 * NOTES:
 *	Checks if the inode's inline area is available.  If so, copies ACL
 *      inline and sets i_acl fields appropriately.  Otherwise, returns
 *      failure, ACL will have to be put into an extent.
 *
 * RETURNS: 0 for successful copy to inline area; -1 if area not available
 */
int
jfs_WriteACLInLine (inode_t * ip, void *acl, int32 length, dxd_t *acl_dxd)
{
	int32	rc;

	/* make sure we have an ACL -- the NULL ACL is valid, but you
	 * can't copy it!
	 */
	if (length)
	{
		assert (length <= sizeof ip->i_inlineea);

		/* see if the space is available or if it is already being
		 * used for an inline ACL.
		 */
		if (((ip->i_mode & INLINEEA) == 0) && /* inline area is used */
		    ((ip->i_acl.flag & DXD_INLINE) == 0)) /* not by acl */
			return -1;

		acl_dxd->size = length;
		DXDlength(acl_dxd, 0);
		DXDaddress (acl_dxd, 0);
		rc = KernCopyIn(ip->i_inlineea, acl, length);
		if (rc)
		{
			acl_dxd->flag = 0;
			acl_dxd->size = 0;
			if (ip->i_acl.flag & DXD_INLINE)
			{
				/* Free up INLINE area */
				ip->i_mode |= INLINEEA;
			}
			return rc;
		}
		acl_dxd->flag = DXD_INLINE;
		ip->i_mode &= ~INLINEEA;
	}
	else
	{
		acl_dxd->flag = 0;
		acl_dxd->size = 0;
		DXDlength(acl_dxd, 0);
		DXDaddress(acl_dxd, 0);

		if (ip->i_acl.flag & DXD_INLINE)
		{
			/* Free up INLINE area */
			ip->i_mode |= INLINEEA;
		}
	}
	imark (ip, ICHG);

	return 0;
}


/*
 * NAME: jfs_WriteACL
 *
 * FUNCTION: Write an ACL for an inode
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	acl	- ACL pointer
 *	length	- ACL length in bytes
 *	acl_dxd	- dxd_t structure to be filled in appropriately with where the
 *		  ACL was copied
 *
 * NOTES: Will write ACL inline if able to, otherwise allocates blocks for an
 *	extent and synchronously writes it to those blocks.
 *
 * RETURNS: 0 for success; Anything else indicates failure
 */
int
jfs_WriteACL (inode_t * ip, void *acl, int32 length, dxd_t *acl_dxd)
{
	int	nblocks;
	int64	blkno;
	int64	hint;
	int	rc = 0;
	char	*cp;
	int	i;
	jbuf_t	*bp;
	int32	nbytes, nb;
	int32	bytes_to_write;

	/* quick check to see if this is an in-linable ACL.  Short ACL's
	 * and empty ACL's are all in-linable, provided the space exists.
	 */
	if (length <= sizeof ip->i_inlineea)
	{
		rc = jfs_WriteACLInLine (ip, acl, length, acl_dxd);
		if (rc != -1)
			return rc;
	}

	/* figure out how many blocks we need */
	nblocks = (length + (ip->i_ipmnt->i_bsize - 1))
			>> ip->i_ipmnt->i_l2bsize;

	hint = INOHINT (ip);

	rc = dbAlloc (ip, hint, nblocks, &blkno);
	if (rc != 0)
		return rc;

	/* Now have nblocks worth of storage to stuff into the ACL.
	 * loop over the ACL copying data into the buffer one page at
	 * a time.
	 */
	for (i = 0, cp = acl, nbytes = length; i < nblocks;
		i += ip->i_ipmnt->i_nbperpage, cp += PSIZE, nbytes -= nb)
	{
		/* Determine how many bytes for this request, and round up to
		 * the nearest aggregate block size for bmAssign request
		 */
		nb = MIN(PSIZE, nbytes);
		bytes_to_write = (((nb + (1 << ip->i_ipmnt->i_l2bsize) - 1)
			>> ip->i_ipmnt->i_l2bsize)) << ip->i_ipmnt->i_l2bsize;
		bp = bmAssign (ip, blkno + i, blkno + i, bytes_to_write,
			bmREAD_BLOCK);

		rc = KernCopyIn(bp->b_bdata, cp, nb);
		if (rc)
		{
			(void) dbFree (ip, blkno, nblocks);
			break;
		}

		if (rc = bmWrite (bp))
		{
			/* the write failed -- this means that the buffer
			 * is still assigned and the blocks are not being
			 * used.  this seems like the best error recovery
			 * we can get ...
			 */
			bmRelease(bp);
			(void) dbFree (ip, blkno, nblocks);
			break;
		}
	}
	if (! rc) {
		acl_dxd->flag = DXD_EXTENT;
		acl_dxd->size = length;
		DXDlength(acl_dxd, nblocks);
		DXDaddress(acl_dxd, blkno);
		if (ip->i_acl.flag & DXD_INLINE)
		{
			/* Free up INLINE area */
			ip->i_mode |= INLINEEA;
		}

		return 0;
	}
	return rc;
}


/*
 * NAME: jfs_ReadACLInLine
 *
 * FUNCTION: Read an inlined ACL into user's buffer
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	acl	- Pointer to buffer to fill in with ACL
 *	length	- On entry, length of user's buffer.
 *		  On exit, length of ACL.
 *
 * RETURNS: 0
 */
int32
jfs_ReadACLInLine (inode_t * ip, void *acl, int32 *length)
{
	int32	rc;

	if (ip->i_acl.size == 0)
	{
		*length = 0;
		return 0;
	}
	if (ip->i_acl.size > sizeof ip->i_inlineea)
	{
		txFileSystemDirty(ip->i_ipmnt);
		return EIO;
	}
	*length = ip->i_acl.size;
	rc = KernCopyOut(acl, (char *) ip->i_inlineea, ip->i_acl.size);

	return rc;
}


/*
 * NAME: jfs_ReadACL
 *
 * FUNCTION: copy ACL data into user's buffer
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	acl	- Pointer to buffer to fill in with ACL
 *	length	- On entry, length of user's buffer.
 *		  On exit, length of ACL.
 *
 * NOTES:  If ACL is inline calls jfs_ReadACLInLine() to copy ACL.
 *
 * SERIALIZATION: IREAD_LOCK held on entry/exit
 *
 * RETURNS: 0 for success; other indicates failure
 */
int
jfs_ReadACL (inode_t * ip, void *acl, int32 *length)
{
	int	nblocks;
	int64	blkno;
	int	rc;
	int	cbList;
	char	*cp;
	int	i;
	jbuf_t	*bp;
	int32	nbytes, nb;

	/* quick check for in-line ACL
	 */
	if (ip->i_acl.flag & DXD_INLINE)
		return jfs_ReadACLInLine (ip, acl, length);

	cbList = ip->i_acl.size;

	assert (cbList);

	if (cbList > *length)
		return ERROR_INSUFFICIENT_BUFFER;

	*length = cbList;

	/* figure out how many blocks were allocated when this ACL was
	 * originally written to disk.
	 */
	nblocks = lengthDXD(&ip->i_acl);
	blkno = addressDXD(&ip->i_acl);

	/* i have found the disk blocks which were originally used to store
	 * the ACL.  now i loop over each contiguous block copying the
	 * data into the buffer.
	 */
	for (i = 0, cp = acl, nbytes = cbList; i < nblocks;
		i += ip->i_ipmnt->i_nbperpage, cp += PSIZE, nbytes -= nb) {
		if (rc = bmRead (ip, blkno + i, PSIZE, bmREAD_BLOCK, &bp))
			return rc;

		nb = MIN(PSIZE, nbytes);
		rc = KernCopyIn(cp, bp->b_bdata, nb);
		bmRelease (bp);
		if (rc)
			return rc;
	}
	return 0;
}

/*
 * NAME: jfs_SetACL
 *
 * FUNCTION: Set's an inode's ACL, removing old one if necessary
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	acl	- ACL pointer
 *	length	- ACL length in bytes
 *
 * SERIALIZATION: IWRITE_LOCK held on entry/exit
 *
 * RETURNS: 0 for success; Anything else indicates failure
 */
int32
jfs_SetACL(inode_t * ip, void *acl, int32 length)
{
	dxd_t	dxd;
	int64	orig_nblocks;					// F226941
	int32	rc, rc1;
	int32	tid;

	if (isReadOnly(ip))
		return ERROR_WRITE_PROTECT;

	txBegin(ip->i_ipmnt, &tid, 0);

	orig_nblocks = ip->i_nblocks;				// F226941

	if (length == 0)
	{
		dxd.size = 0;
		DXDlength(&dxd, 0);
		DXDaddress(&dxd, 0);
		dxd.flag = 0;
		rc = 0;
	}
	else
		rc = jfs_WriteACL(ip, acl, length, &dxd);

	if (!rc)
	{
		if (ip->i_acl.flag & DXD_EXTENT)
			/* free old ACL pages from cache */
			bmExtentInvalidate(ip, addressDXD(&ip->i_acl),
					   lengthDXD(&ip->i_acl));


		txEA(tid, ip, &ip->i_acl, &dxd);

		ip->i_nblocks = ip->i_nblocks + lengthDXD(&dxd) -
				lengthDXD(&ip->i_acl);
		ip->i_acl = dxd;

		imark(ip, ICHG);
	}

	DLIM_UPDATE(tid, ip, ip->i_nblocks - orig_nblocks);	// F226941

	rc1 = DLIM_TXCOMMIT(tid, ip, 0);			// F226941
	rc = rc ? rc : rc1;
	txEnd(tid);

	return rc;
}

/*
 * NAME: jfs_CopyACL
 *
 * FUNCTION: Copy ACL from parent to new directory
 *
 * PRE CONDITIONS:
 *	Holding write lock on both inode and parent.
 *
 * PARAMETERS:
 *	ip	- Inode pointer
 *	dip	- Inode pointer of parent
 *	tid	- Transaction ID
 *
 * NOTES:
 *	Only called for directories, so inline area not checked.
 *
 * RETURNS: 0 for successful copy
 */
int
jfs_CopyACL (inode_t * ip, inode_t *dip, int32 tid)
{
	int32	bytes_left;
	int32	bytes_to_write;
	int64	blkno;
	dxd_t	dxd;
	int64	hint;
	int32	i;
	jbuf_t	*ibp;
	int32	length;
	int32	nblocks;
	int64	new_blkno;
	int32	nb;
	jbuf_t	*obp;
	int32	rc;

	/* If ACL is corrupt, don't inherit anything, but don't fail either */

	if (dip->i_acl.flag & DXD_CORRUPT)
		return NO_ERROR;

	assert(dip->i_acl.flag & DXD_EXTENT);

	length = dip->i_acl.size;
	nblocks = lengthDXD(&dip->i_acl);
	blkno = addressDXD(&dip->i_acl);

	assert(nblocks == ((length + ip->i_ipmnt->i_bsize - 1) >>
			   ip->i_ipmnt->i_l2bsize));

	hint = INOHINT(ip);

	rc = dbAlloc(ip, hint, nblocks, &new_blkno);
	if (rc != 0)
		return rc;

	bytes_left = length;

	for (i = 0; i < nblocks; i++)
	{
		if (rc = bmRead(dip, blkno + i, PSIZE, bmREAD_BLOCK, &ibp))
		{
			(void) dbFree(ip, new_blkno, nblocks);
			break;
		}
		nb = MIN(PSIZE, bytes_left);
		bytes_to_write = ((nb + ip->i_ipmnt->i_bsize - 1) >>
				  ip->i_ipmnt->i_l2bsize) <<
				 ip->i_ipmnt->i_l2bsize;
		obp = bmAssign (ip, new_blkno+i, new_blkno+i, bytes_to_write,
			       bmREAD_BLOCK);

		memcpy(obp->b_bdata, ibp->b_bdata, nb);
		bmRelease(ibp);
		rc = bmWrite(obp);
		if (rc)
		{
			bmRelease(obp);
			(void) dbFree(ip, new_blkno, nblocks);
			break;
		}
		bytes_left -= nb;
	}
	if (!rc)
	{
		dxd.flag = DXD_EXTENT;
		dxd.size = length;
		DXDlength(&dxd, nblocks);
		DXDaddress(&dxd, new_blkno);
		/* This routine is only called with a brand new directory
		 * inode.  No need to free previous ACL pages
		 */
		txEA(tid, ip, &ip->i_acl, &dxd);
		ip->i_nblocks += lengthDXD(&dxd);
		ip->i_acl = dxd;
		ip->i_acltype = dip->i_acltype;
		imark(ip, ICHG);
	}
	return rc;
}

/*
 * NAME: alloc_enum
 *
 * FUNCTION: Allocates ACL enumeration structure
 *
 * PARAMETERS: none
 *
 * SERIALIZATION: Takes and releases enum_lock
 *
 * RETURNS: address of new enum instance; zero on error
 */
enum_instance_t	*alloc_enum()
{
	enum_instance_t	*new_enum;

	new_enum = xmalloc(sizeof(enum_instance_t), 0, kernel_heap);

	if (new_enum == 0)
	{
		/* We wouldn't expect such a small allocation to fail */
		jEVENT(1,("alloc_enum: xmalloc failed!\n"));
		return 0;
	}
	new_enum->enum_lastmatch = (UniChar *)allocpool(unipool, 0);
	if (new_enum->enum_lastmatch == 0)
	{
		xmfree(new_enum, kernel_heap);
		return 0;
	}
	new_enum->enum_lastmatch[0] = 0;
	new_enum->enum_pattern.namlen = 1;			// F226941
	new_enum->enum_pattern.name = lonestar;			// F226941

	ENUM_LOCK();
	new_enum->next = enum_anchor;
	enum_anchor = new_enum;
	ENUM_UNLOCK();

	return new_enum;
}

/*
 * NAME: verify_enum()
 *
 * FUNCTION: Verifies that enum handle passed from the user is valid
 *
 * PARAMETERS:
 *	hEnum - enum handle to be verified
 *
 * SERIALIZATION: Takes and releases enum_lock
 *
 * RETURNS: zero if handle valid, ERROR_INVALID_HANDLE otherwise
 */
int32	verify_enum(
enum_instance_t *hEnum)
{
	enum_instance_t	*e;

	ENUM_LOCK();
	for (e = enum_anchor; e; e = e->next)
	{
		if (hEnum == e)
		{
			ENUM_UNLOCK();
			/* It is valid.  Now let's make sure that the drive
			 * hasn't been unmounted.
			 */
			if (hEnum->enum_vfsp->vfs_flag & VFS_SHUTDOWN)
				return ERROR_INVALID_HANDLE;
			else
				return 0;
		}
	}
	ENUM_UNLOCK();
	return ERROR_INVALID_HANDLE;
}

/*
 * NAME: free_enum()
 *
 * FUNCTION: Frees enum instance and resources
 *
 * PARAMETERS:
 *	hEnum - enum handle to be freed
 *
 * SERIALIZATION: Takes and releases enum_lock
 *
 * RETURNS: zero if handle valid, ERROR_INVALID_HANDLE otherwise
 */
int32	free_enum(
enum_instance_t *hEnum)
{
	enum_instance_t	*e;
	enum_instance_t	*last = 0;

	ENUM_LOCK();
	for (e = enum_anchor; e; e = e->next)
	{
		if (hEnum == e)
		{
			if (last)
				last->next = hEnum->next;
			else
				enum_anchor = hEnum->next;
			ENUM_UNLOCK();
// Begin F226941
			if (hEnum->enum_pattern.name != lonestar)	
				freepool(unipool,
					 (caddr_t *)hEnum->enum_pattern.name);
// End F226941
			freepool(unipool, (caddr_t *)hEnum->enum_lastmatch);
			xmfree(hEnum, kernel_heap);
			return 0;
		}
		last = e;
	}
	ENUM_UNLOCK();
	return ERROR_INVALID_HANDLE;
}

/*
 * NAME: enum_exit()
 *
 * FUNCTION: Frees enum instances associated with PID
 *
 * PARAMETERS:
 *	pid - process ID of exiting process
 *
 * SERIALIZATION: Takes and releases enum_lock
 *
 * RETURNS: none
 */
void	enum_exit(
int32 pid)
{
	enum_instance_t	*e;
	enum_instance_t	*last = 0;
	enum_instance_t	*next;

	ENUM_LOCK();
	for (e = enum_anchor; e; e = next)
	{
		next = e->next;
		if (e->enum_pid == pid)
		{
			if (last)
				last->next = next;
			else
				enum_anchor = next;
			freepool(unipool, (caddr_t *)e->enum_lastmatch);
			xmfree(e, kernel_heap);
		}
		else
			last = e;
	}
	ENUM_UNLOCK();
	return;
}

/*
 * NAME: jfs_enumACL()
 *
 * FUNCTION: Returns first/next file having ACL or subdirectory
 *
 * PARAMETERS:
 *	aclparms - jfs_access_parms structure
 *	pData    - pointer to users data buffer
 *	length   - input/output - length of data buffer
 *
 * SERIALIZATION:
 *
 * RETURNS: 0 for success
 *	    ERROR_NO_MORE_FILES - no more files w/ACLs or subdirectories
 *	    ERROR_INSUFFICIENT_BUFFER - buffer too small
 */
int32	jfs_enumACL(
struct vnode		*dvp,
struct jfs_access_parms	*aclparms,
char			*pData,
int32			*length)
{
	int32		acl_length;
	struct dirent	*dbufp;
	inode_t		*dip;
	struct jfs_access_enum_header *enum_header;
	enum_instance_t	*enump;
	inode_t		*ip;
	uint32		matches;
	int32		offset;
	int32		rc;
	int32		tbytes;
	int32		total_length;

	enump = (enum_instance_t *)aclparms->hEnum;
	dip = VP2IP(dvp);

	if (enump->enum_lastmatch[0] == 0)
		offset = 0;	/* first search */
	else
		offset = 1;	/* subsequent search */

	dbufp = (struct dirent *)allocpool(dirent_pool, 0);
	if (dbufp == 0)
		return ENOMEM;

	IREAD_LOCK(dip);

	if ((dip->i_nlink == 0) || ((dip->i_mode & IFMT) != IFDIR))
	{
		IREAD_UNLOCK(dip);
		freepool(dirent_pool, (caddr_t *)dbufp);
		return ENOTDIR;
	}

	matches = 1;

	while (TRUE)
	{
		tbytes = 0;
		rc = dtFind(dip, &enump->enum_pattern, enump->enum_lastmatch, // F226941
			    &offset, &matches, PSIZE, &tbytes, dbufp);
		IREAD_UNLOCK(dip);
		if (rc || (matches == 0))
		{
			aclparms->ulReturnCode = ERROR_NO_MORE_FILES;
			break;
		}

		UniStrncpy(enump->enum_lastmatch, dbufp->d_name,					   dbufp->d_namlen);
		enump->enum_lastmatch[dbufp->d_namlen] = 0;

		ICACHE_LOCK();
		rc = iget(dvp->v_vfsp, dbufp->d_ino, &ip, 0);
		ICACHE_UNLOCK();

		if (rc)
			break;

		IREAD_LOCK(ip);
		if (((ip->i_mode & IFMT) == IFDIR) ||
		    (ip->i_acl.flag &&
		     (ip->i_acltype & JFS_ACCESS_FLAGS_ACLTYPE_MASK) ==
		     (aclparms->ulFlags & JFS_ACCESS_FLAGS_ACLTYPE_MASK)))
		{
			total_length = sizeof(struct jfs_access_enum_header);
			if (ip->i_acl.flag)
				total_length += ip->i_acl.size;
			aclparms->cchData = total_length;
			if (total_length > *length)
			{
				IREAD_UNLOCK(ip);
				jfs_rele(IP2VP(ip));
				*length = 0;
				aclparms->ulReturnCode =
						ERROR_INSUFFICIENT_BUFFER;
				break;
			}
			enum_header = (struct jfs_access_enum_header *)pData;
			enum_header->cchName = jfs_strfromUCS(
					enum_header->achName, dbufp->d_name,
					CCHMAXPATHCOMP, dbufp->d_namlen);
			if (enum_header->cchName == -1)
			{
				IREAD_UNLOCK(ip);
				jfs_rele(IP2VP(ip));
				aclparms->ulReturnCode = ERROR_BUFFER_OVERFLOW;
				break;
			}
			if (ip->i_acl.flag &&
			    (ip->i_acltype & JFS_ACCESS_FLAGS_ACLTYPE_MASK) ==
			    (aclparms->ulFlags & JFS_ACCESS_FLAGS_ACLTYPE_MASK))
			{
				acl_length = *length - sizeof(struct jfs_access_enum_header);
				aclparms->ulReturnCode = jfs_ReadACL(ip,
					pData+sizeof(struct jfs_access_enum_header),
					&acl_length);
				aclparms->ulFlags = ip->i_acltype;
			}
			else
			{
				aclparms->ulFlags = 0;
				aclparms->ulReturnCode = NO_ERROR;
			}
			if ((ip->i_mode & IFMT) == IFDIR)
				aclparms->ulFlags |= JFS_ACCESS_FLAGS_DIRECTORY;

			IREAD_UNLOCK(ip);
			jfs_rele(IP2VP(ip));
			*length = total_length;
			break;
		}
		IREAD_UNLOCK(ip);
		jfs_rele(IP2VP(ip));

		IREAD_LOCK(dip);
	}
	freepool(dirent_pool, (caddr_t *)dbufp);
	return aclparms->ulReturnCode;
}

/*
 * NAME: jfs_getacc0()
 *
 * FUNCTION: Retrieves info about existence & size of ACL's
 *
 * PARAMETERS:
 *	ip	 - inode pointer
 *	aclparms - jfs_access_parms structure
 *
 * SERIALIZATION:
 *	Caller holds read lock on ip
 *
 * RETURNS: NO_ERROR for success
 *	    ERROR_INVALID_PARAMETER - Caller not requesting entire ACL
 *	    ERROR_FILE_EXISTS - inode has no ACL
 *	    ERROR_INVALID_DATA - corrupt acl or acltype mismatch
 *	Return code is also put into aclparms structure
 */
int32	jfs_getacc0(
inode_t			*ip,
struct jfs_access_parms	*ACLparms)
{
	if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK) !=
	    JFS_ACCESS_FLAGS_TARGETACL_ALL)
		ACLparms->ulReturnCode = ERROR_INVALID_PARAMETER;
	else if (ip->i_acl.flag & DXD_CORRUPT)
	{
		ACLparms->ulFlags = 0;
		ACLparms->cchData = 0;
		ACLparms->ulReturnCode = ERROR_INVALID_DATA;
	}
	else if ((ip->i_acl.flag == 0) || (ip->i_acl.size == 0))
	{
		ACLparms->ulFlags = 0;
		ACLparms->cchData = 0;
		ACLparms->ulReturnCode = ERROR_FILE_EXISTS;
	}
	else
	{
		ACLparms->cchData = ip->i_acl.size;
		if ((ip->i_acltype & JFS_ACCESS_FLAGS_ACLTYPE_MASK) !=
		    (ACLparms->ulFlags & JFS_ACCESS_FLAGS_ACLTYPE_MASK))
		{
			jEVENT(1,("jfs_getacc0: acltype mismatch!\n"));
			ACLparms->ulReturnCode = ERROR_INVALID_DATA;
		}
		else
			ACLparms->ulReturnCode = NO_ERROR;
		ACLparms->ulFlags = ip->i_acltype;
	}
	return ACLparms->ulReturnCode;
}

/*
 * NAME: jfs_getacc1()
 *
 * FUNCTION: Performs checks & retrieves file's (or directory's) acl
 *
 * PARAMETERS:
 *	ip	 - inode pointer
 *	aclparms - jfs_access_parms structure
 *	pData	 - pointer to data buffer
 *	length   - pointer to length of data buffer (input/output)
 *
 * SERIALIZATION:
 *	Caller holds read lock on ip
 *
 * RETURNS: NO_ERROR for success
 *	    ERROR_INVALID_PARAMETER - Caller not requesting entire ACL
 *	    ERROR_FILE_EXISTS - inode has no ACL
 *	    ERROR_INVALID_DATA - corrupt acl or acltype mismatch
 *	    ERROR_MORE_DATA - data buffer too small to hold ACL
 *	Return code is also put into aclparms structure
 */
int32	jfs_getacc1(
inode_t			*ip,
struct jfs_access_parms	*ACLparms,
char			*pData,
int32			*length)
{
	if ((ACLparms->ulFlags & JFS_ACCESS_FLAGS_TARGETACL_MASK) !=
	    JFS_ACCESS_FLAGS_TARGETACL_ALL)
		ACLparms->ulReturnCode = ERROR_INVALID_PARAMETER;
	else if (ip->i_acl.flag & DXD_CORRUPT)
	{
		*length = 0;
		ACLparms->ulFlags = 0;
		ACLparms->cchData = 0;
		ACLparms->ulReturnCode = ERROR_INVALID_DATA;
	}
	else if ((ip->i_acl.flag == 0) || (ip->i_acl.size == 0))
	{
		*length = 0;
		ACLparms->ulFlags = 0;
		ACLparms->cchData = 0;
		ACLparms->ulReturnCode = ERROR_FILE_EXISTS;
	}
	else
	{
		ACLparms->cchData = ip->i_acl.size;
		if ((ip->i_acltype & JFS_ACCESS_FLAGS_ACLTYPE_MASK) !=
		    (ACLparms->ulFlags & JFS_ACCESS_FLAGS_ACLTYPE_MASK))
		{
			*length = 0;
			jEVENT(1,("jfs_getacc1: acltype mismatch!\n"));
			ACLparms->ulReturnCode = ERROR_INVALID_DATA;
		}
		else if (ip->i_acl.size > *length)
		{
			*length = 0;
			ACLparms->ulReturnCode = ERROR_MORE_DATA;
		}
		else
			ACLparms->ulReturnCode = jfs_ReadACL(ip, pData, length);
		ACLparms->ulFlags = ip->i_acltype;
	}
	return ACLparms->ulReturnCode;
}
