/* $Id: jfs_rawio.c,v 1.1.1.1 2003/05/21 13:38:02 pasha Exp $ */

static char *SCCSID = "@(#)1.7  7/30/98 14:18:17 src/jfs/ifs/jfs_rawio.c, sysjfs, w45.fs32, 990417.1";
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
 */
/*
 *	jfs_rawio.c	- Raw I/O to disk partition (direct access)
 *
 *	Note:  It is possible to get DASD read & write requests when the
 *	filesystem is not fully mounted.  That is when FS_MOUNT was passed
 *	the MOUNT_ACCEPT flag.  We cannot assume that any inodes exist.  We
 *	create a dummy inode with enough information for called routines to
 *	behave as expected.
 *
 *	These routines were not written to be efficient.  If there is a
 *	requirement that this I/O perform well, this will need to be
 *	redesigned.
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"

#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_io.h"
#include "jfs_dio.h"
#include "jfs_cachemgr.h"
#include "jfs_debug.h"

/* Global Data
 */
inode_t		*FreeDummyInodes = 0;	/* Free list of dummy inodes	   */
MUTEXLOCK_T	dasd_lock;		/* lock protecting FreeDummyInodes */

/* Macros
 */
#define		DASD_LOCK()	MUTEXLOCK_LOCK(&dasd_lock)
#define		DASD_UNLOCK()	MUTEXLOCK_UNLOCK(&dasd_lock)

/* Forward references
 */
inode_t	*getDummyInode(struct vfs *vfsp);
void	releDummyInode(inode_t *dummy_inode);

/*
 * NAME: dasd_write
 *
 * FUNCTION: Write to direct access file descriptor
 *
 * PARAMETERS:
 *	offset	- Starting byte offset for write request
 *	pdata	- Starting address of user data buffer
 *	plen	- On entry, number of bytes to be write
 *		  On exit, number of bytes actually write
 *	vfsp	- pointer to vfs structure
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dasd_write(
int64		offset,
caddr_t		pdata,
int64		*plen,
struct vfs	*vfsp)
{
	int64	blocknum;
	cbuf_t	*bp;
	dio_t	*dp;
	inode_t	*dummy_inode;
	int64	erroff;
	int64	fbytes;
	int64	lbytes;
	int64	mbytes;
	int64	nbytes = *plen;
	int32	rc;

	dummy_inode = getDummyInode(vfsp);

	/* If offset does not start on sector boundary, calculate number of
	 * bytes in first partial sector.
	 */
	if (offset & (vfsp->vfs_bsize - 1))
	{
		fbytes = MIN(nbytes, CM_BSIZE - (offset & CM_OFFSET));
		nbytes -= fbytes;
	}
	else
		fbytes = 0;

	/* If write does not end on sector boundary, calculate number of
	 * bytes in last sector
	 */
	if (nbytes & vfsp->vfs_bsize - 1)
		lbytes = nbytes & CM_OFFSET;
	else
		lbytes = 0;

	mbytes = nbytes - lbytes;

	nbytes = 0;	/* Now counting bytes actually written */

	/* First (partial) block */
	if (fbytes)
	{
		if (rc = rawRead(dummy_inode, offset, &bp))
			goto out1;
		if (rc = copyin(pdata, bp->cm_cdata+(offset & CM_OFFSET),
				fbytes))
		{
			rawRelease(bp);
			goto out1;
		}
		if (rc = rawWrite(dummy_inode, bp, 1))
			goto out1;
		nbytes = fbytes;
		offset += fbytes;
		pdata += fbytes;
	}
	/* Write full sectors
	 */
	if (mbytes)
	{
		blocknum = offset >> dummy_inode->i_l2pbsize;
		if (rc = dioStart(dummy_inode, &dp))
			goto out1;
		(void) dioWrite(dp, blocknum, mbytes, offset, pdata, 0);
		if (rc = dioEnd(dp, &erroff))
		{
			nbytes += (erroff - offset);
			goto out1;
		}
		nbytes += mbytes;
		offset += mbytes;
		pdata += mbytes;
	}
	/* Write last (partial) block
	 */
	if (lbytes)
	{
		if (rc = rawRead(dummy_inode, offset, &bp))
			goto out1;
		if (rc = copyin(pdata, bp->cm_cdata, lbytes))
		{
			rawRelease(bp);
			goto out1;
		}
		if (rc = rawWrite(dummy_inode, bp, 1))
			goto out1;
		nbytes = +lbytes;
	}
out1:
	releDummyInode(dummy_inode);
	*plen = nbytes;
	return (rc);
}

/*
 * NAME: dasd_read
 *
 * FUNCTION: Read from direct access file descriptor
 *
 * PARAMETERS:
 *	offset	- Starting byte offset for read request
 *	pdata	- Starting address of user data buffer
 *	plen	- On entry, number of bytes to be read
 *		  On exit, number of bytes actually read
 *	vfsp	- pointer to vfs structure
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dasd_read(
int64		offset,
caddr_t		pdata,
int64		*plen,
struct vfs	*vfsp)
{
	int64	blocknum;
	cbuf_t	*bp;
	dio_t	*dp;
	inode_t	*dummy_inode;
	int64	erroff;
	int64	fbytes;
	int64	lbytes;
	int64	mbytes;
	int64	nbytes = *plen;
	int32	rc;

	dummy_inode = getDummyInode(vfsp);

	/* If offset does not start on sector boundary, calculate number of
	 * bytes in first partial sector.
	 */
	if (offset & (vfsp->vfs_bsize - 1))
	{
		fbytes = MIN(nbytes, CM_BSIZE - (offset & CM_OFFSET));
		nbytes -= fbytes;
	}
	else
		fbytes = 0;

	/* If read does not end on sector boundary, calculate number of
	 * bytes in last sector
	 */
	if ((nbytes & vfsp->vfs_bsize - 1))
		lbytes = nbytes & CM_OFFSET;
	else
		lbytes = 0;

	mbytes = nbytes - lbytes;

	nbytes = 0;	/* Now counting bytes actually read */

	/* First (partial) block */
	if (fbytes)
	{
		if (rc = rawRead(dummy_inode, offset, &bp))
			goto out2;
		rc = copyout(bp->cm_cdata+(offset & CM_OFFSET), pdata, fbytes);
		rawRelease(bp);
		if (rc)
			goto out2;
		nbytes = fbytes;
		offset += fbytes;
		pdata += fbytes;
	}
	/* Read full sectors
	 */
	if (mbytes)
	{
		blocknum = offset >> dummy_inode->i_l2pbsize;
		if (rc = dioStart(dummy_inode, &dp))
			goto out2;
		(void) dioRead(dp, blocknum, mbytes, offset, pdata);
		if (rc = dioEnd(dp, &erroff))
		{
			nbytes += (erroff - offset);
			goto out2;
		}
		nbytes += mbytes;
		offset += mbytes;
		pdata += mbytes;
	}
	/* Read last (partial) block
	 */
	if (lbytes)
	{
		if (rc = rawRead(dummy_inode, offset, &bp))
		{
			rawRelease(bp);
			goto out2;
		}
		rc = copyout(bp->cm_cdata, pdata, lbytes);
		rawRelease(bp);
		if (rc)
			goto out2;
		nbytes += lbytes;
	}
out2:
	releDummyInode(dummy_inode);
	*plen = nbytes;
	return (rc);
}

/*
 * NAME: getDummyInode
 *
 * FUNCTION: Allocates & initializes dummy inode which is used by called I/O
 *	     routines
 *
 * PARAMETERS:
 *	vfsp	- vfsp for filesystem - may not be fully mounted
 *
 * RETURNS: pointer to properly-initialized dummy inode
 */
inode_t	*getDummyInode(
struct vfs	*vfsp)
{
	inode_t *dummy_inode;

	DASD_LOCK();
	if (FreeDummyInodes == 0) {
		assert (FreeDummyInodes = (inode_t *)xmalloc(sizeof(inode_t),
							     0, pinned_heap));
		bzero(FreeDummyInodes, sizeof(inode_t));
	}
	dummy_inode = FreeDummyInodes;
	FreeDummyInodes = dummy_inode->i_flnext;
	DASD_UNLOCK();

	/* Initialize only the fields used by rawRead/Write & dioRead/Write
	 */
	dummy_inode->i_dev = vfsp->vfs_vpfsi->vpi_unit;
	dummy_inode->i_ipmnt = dummy_inode;
	dummy_inode->i_Strategy2 = vfsp->vfs_strat2p;
	dummy_inode->i_Strategy3 = vfsp->vfs_strat3p;
	dummy_inode->i_pbsize = vfsp->vfs_bsize;
	dummy_inode->i_l2pbsize = log2shift(vfsp->vfs_bsize);

	return dummy_inode;
}

/*
 * NAME: releDummyInode
 *
 * FUNCTION: Puts dummy inode back on freelist
 *
 * PARAMETERS:
 *	dummy_inode	- pointer to dummy inode
 *
 * RETURNS: Nothing
 */
void	releDummyInode(
inode_t		*dummy_inode)
{
	DASD_LOCK();
	dummy_inode->i_flnext = FreeDummyInodes;
	FreeDummyInodes = dummy_inode;
	DASD_UNLOCK();
}
