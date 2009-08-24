/* $Id: jfs_imap.c,v 1.1 2000/04/21 10:58:11 ktk Exp $ */

static char *SCCSID = "@(#)1.29  11/1/99 13:31:00 src/jfs/ifs/jfs_imap.c, sysjfs, w45.fs32, fixbld";
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
 * Change History :
 *
 */

/*
 *	jfs_imap.c: inode allocation map manager
 *
 * Serialization:
 *   Each AG has a simple lock which is used to control the serialization of
 *	the AG level lists.  This lock should be taken first whenever an AG
 *	level list will be modified or accessed.
 *
 *   Each IAG is locked by obtaining the buffer for the IAG page.
 *
 *   There is also a inode lock for the inode map inode.  A read lock needs to
 *	be taken whenever an IAG is read from the map or the global level
 *	information is read.  A write lock needs to be taken whenever the global
 *	level information is modified or an atomic operation needs to be used.
 *
 *	If more than one IAG is read at one time, the read lock may not
 *	be given up until all of the IAG's are read.  Otherwise, a deadlock
 *	may occur when trying to obtain the read lock while another thread
 *	holding the read lock is waiting on the IAG already being held.
 *
 *   The control page of the inode map is read into memory by diMount().
 *	Thereafter it should only be modified in memory and then it will be
 *	written out when the filesystem is unmounted by diUnmount().
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif	/* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_txnmgr.h"
#include "jfs_imap.h"
#include "jfs_dmap.h"
#include "jfs_logmgr.h"
#include "jfs_superblock.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

/*
 * imap locks
 */
/* iag free list lock */
#define	IAGFREE_LOCK_ALLOC(imap)\
	MUTEXLOCK_ALLOC(&imap->im_freelock,LOCK_ALLOC_PAGED,JFS_IAGFREE_LOCK_CLASS,0)
#define	IAGFREE_LOCK_FREE(imap)	MUTEXLOCK_FREE(&imap->im_freelock)
#define	IAGFREE_LOCK_INIT(imap)	MUTEXLOCK_INIT(&imap->im_freelock)
#define IAGFREE_LOCK(imap)	MUTEXLOCK_LOCK(&imap->im_freelock)
#define IAGFREE_UNLOCK(imap)	MUTEXLOCK_UNLOCK(&imap->im_freelock)

/* per ag iag list locks */
#define	AG_LOCK_ALLOC(imap, index)\
	MUTEXLOCK_ALLOC(&imap->im_aglock[index],LOCK_ALLOC_PAGED,JFS_AG_LOCK_CLASS,index)
#define	AG_LOCK_FREE(imap,index)	MUTEXLOCK_FREE(&imap->im_aglock[index])
#define	AG_LOCK_INIT(imap,index)	MUTEXLOCK_INIT(&(imap->im_aglock[index]))
#define AG_LOCK(imap,agno)	MUTEXLOCK_LOCK(&imap->im_aglock[agno])
#define AG_UNLOCK(imap,agno)	MUTEXLOCK_UNLOCK(&imap->im_aglock[agno])

#define ATOMIC_ADD(addr, incr)	(addr) += (incr)

/*
 * external references
 */
int32 readSuper(inode_t *ipmnt, cbuf_t **bpp);
int32 txFileSystemDirty(inode_t *);

/*
 * forward references
 */
static int32 diAllocAG(imap_t *imap, int32 agno, boolean_t dir, ialloc_t *iap);
static int32 diAllocAny(imap_t *imap, int32 agno, boolean_t dir, ialloc_t *iap);
static int32 diAllocBit(imap_t *imap, iag_t *iagp, int32 ino);
static int32 diAllocExt(imap_t *imap, int32 agno, ialloc_t *iap);
static int32 diAllocIno(imap_t *imap, int32 agno, ialloc_t *iap);
static int32 diFindFree(uint32 word, int32 start);
static int32 diIAGRead(imap_t *imap, int32 iagno, jbuf_t **bpp);
static int32 diNewExt(imap_t *imap, iag_t *iagp, int32 extno);
static int32 diNewIAG(imap_t	*imap, int32 *iagnop, int32 agno, jbuf_t **bpp);
static int32 diReadSpecial(inode_t *ip);
static void duplicateIXtree(inode_t *ipimap, int64 blkno, int32	xlen, int64 *);


/*
 *	debug code for double-checking inode map
 */
/* #define	_JFS_DEBUG_IMAP	1 */

#ifdef	_JFS_DEBUG_IMAP
#define DBG_DIINIT(imap)	DBGdiInit(imap)
#define DBG_DIALLOC(imap, ino)	DBGdiAlloc(imap, ino)
#define DBG_DIFREE(imap, ino)	DBGdiFree(imap, ino)

void *DBGdiInit(imap_t *imap);
void DBGdiAlloc(imap_t *imap, ino_t ino);
void DBGdiFree(imap_t *imap, ino_t ino);
#else
#define DBG_DIINIT(imap)
#define DBG_DIALLOC(imap, ino)
#define DBG_DIFREE(imap, ino)
#endif	/* _JFS_DEBUG_IMAP */


/*
 * NAME:        diMount()
 *
 * FUNCTION:    initialize the incore inode map control structures for
 *		a fileset or aggregate init time.
 *
 *              the inode map's control structure (dinomap_t) is 
 *              brought in from disk and placed in virtual memory.
 *
 * PARAMETERS:
 *      ipimap  - pointer to inode map inode for the aggregate or fileset.
 *
 * RETURN VALUES:
 *      0       - success
 *      ENOMEM  - insufficient free virtual memory.
 *      EIO  	- i/o error.
 */
int32
diMount(inode_t	*ipimap)
{
	int32 rc;
	imap_t *imap;
	jbuf_t *bp;
	int32	index;

	/*
	 * allocate/initialize the in-memory inode map control structure
	 */
	/* allocate the in-memory inode map control structure. */
        if ((imap = (imap_t *)xmalloc(sizeof(imap_t), L2PSIZE,
					kernel_heap)) == NULL)
                return(ENOMEM);

	/* read the on-disk inode map control structure. */
	if (rc = bmRead(ipimap, IMAPBLKNO, PSIZE, bmREAD_PAGE, &bp))
	{
		xmfree((void *)imap,kernel_heap);
		return(rc);
	}

	/* copy the on-disk version to the in-memory version. */
	bcopy(bp->b_bdata,&imap->im_imap,sizeof(dinomap_t));

	/* release the buffer. */
	bmRelease(bp);

	/*
	 * allocate/initialize inode allocation map locks
	 */
	/* allocate and init iag free list lock */
	IAGFREE_LOCK_ALLOC(imap);
	IAGFREE_LOCK_INIT(imap);

	/* allocate and init ag list locks */
	for (index = 0; index < MAXAG; index++)
	{
		AG_LOCK_ALLOC(imap, index);
		AG_LOCK_INIT(imap, index);
	}

	/* bind the inode map inode and inode map control structure
	 * to each other.
	 */
	imap->im_ipimap = ipimap;
	ipimap->i_imap = imap;

	DBG_DIINIT(imap);

	return(0);
}


/*
 * NAME:        diUnmount()
 *
 * FUNCTION:    write to disk the incore inode map control structures for
 *		a fileset or aggregate at unmount time.
 *
 * PARAMETERS:
 *      ipimap  - pointer to inode map inode for the aggregate or fileset.
 *
 * RETURN VALUES:
 *      0       - success
 *      ENOMEM  - insufficient free virtual memory.
 *      EIO  	- i/o error.
 */
int32
diUnmount(inode_t	*ipimap,
	uint32	mounterror)
{
	int32 rc;
	imap_t *imap;
	jbuf_t *bp;
	int32	index;

	if (mounterror || isReadOnly(ipimap))
		goto release;

	/*
	 * update the on-disk inode map control structure
	 */
	/* read the on-disk inode map control structure */
	if (rc = bmRead(ipimap, IMAPBLKNO, PSIZE, bmREAD_PAGE, &bp))
		return(rc);

	/* copy the in-memory version to the on-disk version */
	bcopy(&ipimap->i_imap->im_imap,bp->b_bdata,sizeof(dinomap_t));

	/* synchronously write out the control structure */
	bmWrite(bp);

release: /* release resources */

	/*
	 * free inode allocation map locks
	 */
	imap = ipimap->i_imap;

	IAGFREE_LOCK_FREE(imap);

	for (index = 0; index < MAXAG; index++)
		AG_LOCK_FREE(imap, index);

	/*
	 * free in-memory control structure
	 */
	xmfree(imap, kernel_heap);

	return(0);
}


/*
 *	diSync()
 */
int32 diSync(inode_t	*ipimap)
{
	imap_t	*mp = ipimap->i_imap;
	jbuf_t *bp;

	/*
	 * write imap global conrol page
	 */
	/* read the on-disk inode map control structure */
	if (bmRead(ipimap, IMAPBLKNO, PSIZE, bmREAD_PAGE, &bp))
		return(EIO);

	/* copy the in-memory version to the on-disk version */
	bcopy(&mp->im_imap,bp->b_bdata,sizeof(dinomap_t));

	/* write out the control structure */
	bmLazyWrite(bp, 0);

	/*
	 * write out dirty pages of imap
	 */
	bmInodeWrite(ipimap);

	return(0);
}


/*
 * NAME:        diRead()
 *
 * FUNCTION:    initialize an incore inode from disk.
 *
 *		on entry, the specifed incore inode should itself
 *		specify the disk inode number corresponding to the
 *		incore inode (i.e. i_number should be initialized).
 *		
 *		this routine handles incore inode initialization for
 *		both "special" and "regular" inodes.  special inodes
 *		are those required early in the mount process and
 *	        require special handling since much of the file system
 *		is not yet initialized.  these "special" inodes are
 *		identified by a NULL inode map inode pointer and are
 *		actually initialized by a call to diReadSpecial().
 *		
 *		for regular inodes, the iag describing the disk inode
 *		is read from disk to determine the inode extent address
 *		for the disk inode.  with the inode extent address in
 *		hand, the page of the extent that contains the disk
 *		inode is read and the disk inode is copied to the
 *		incore inode.
 *
 * PARAMETERS:
 *      ip  -  pointer to incore inode to be initialized from disk.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO  	- i/o error.
 */
int32
diRead(inode_t *ip)
{
	int32 iagno, ino, extno, rc;
	inode_t *ipimap;
	dinode_t *dp;
	iag_t *iagp;
	jbuf_t *bp;
	int64 blkno, agstart;
	imap_t *imap;

	/* for 'special'inode access (w/o ipimap mapping),
	 * delegate to diReadSpecial();
	 */
        if ((ipimap = ip->i_ipimap) == NULL)
                return(diReadSpecial(ip));

	/* determine the iag number for this inode (number) */
	iagno = INOTOIAG(ip->i_number);

	/* read the iag */
	imap = ipimap->i_imap;
	IREAD_LOCK(ipimap);
        rc = diIAGRead(imap, iagno, &bp);
	IREAD_UNLOCK(ipimap);
        if (rc)
	{
		return(rc);
	}

        iagp = (iag_t *)bp->b_bdata;

	/* determine inode extent that holds the disk inode */
	ino = ip->i_number & (INOSPERIAG-1);
	extno = ino >> L2INOSPEREXT;

//	assert(lengthPXD(&iagp->inoext[extno]) == ipimap->i_imap->im_nbperiext);
//	assert(addressPXD(&iagp->inoext[extno]));
	if ((lengthPXD(&iagp->inoext[extno]) != ipimap->i_imap->im_nbperiext) ||
	    (addressPXD(&iagp->inoext[extno]) == 0))
	{
		bmRelease(bp);
		return ESTALE;
	}

	/* get disk block number of the page within the inode extent
	 * that holds the disk inode.
	 */
	blkno = INOPBLK(&iagp->inoext[extno],
			ino,ipimap->i_ipmnt->i_l2nbperpage);

	/* get the ag for the iag */
	agstart = iagp->agstart;

	bmRelease(bp);

	/* read the page of disk inode */
        if (rc = bmRead(ipimap, blkno, PSIZE, bmREAD_BLOCK, &bp))
		return(rc);

	/* locate the the disk inode requested */
        dp = (dinode_t *) bp->b_bdata;
	dp += (ino & (INOSPERPAGE-1));

//assert(ip->i_number == dp->di_number);
//assert(dp->di_nlink);
	if (ip->i_number != dp->di_number)
	{
		txFileSystemDirty(ip->i_ipmnt);
		rc = EIO;
	}
	else if (dp->di_nlink == 0)
		rc = ESTALE;
	else
		/* copy the disk inode to the in-memory inode */
		bcopy(dp,&ip->i_dinode,DISIZE);

	bmRelease(bp);

	/* set the ag for the inode */
	ip->i_agstart = agstart;
	
	return(rc);
}


/*
 * NAME:        diReadSpecial()
 *
 * FUNCTION:    initialize a 'special' inode from disk.
 *
 *		on entry, the specifed incore inode should itself
 *		specify the disk inode number corresponding to the
 *		incore inode (i.e. i_number should be initialized).
 *		
 *		this routines handles aggregate level inodes that
 *		are required early in the mount process and cannot
 *		be handled by normal means (i.e. diRead()) because
 *		much of the file system is not yet intialized.  the
 *	        disk inodes for these "special" inodes are at fixed
 *		disk locations.  they are brought into memory and
 *		copied to the incore inode.
 *
 * PARAMETERS:
 *      ip  -  pointer to incore inode to be initialized from disk.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO  	- i/o error.
 */
static int32
diReadSpecial(inode_t *ip)
{
	int64	address;
        cbuf_t	*bp;
        dinode_t *dp;
	ino_t	inum;
	inode_t	*ipmnt;
        int32	rc;

	/* if mount inode or log inode, no disk inodes exists:
	 * zero on-disk inode portion of in-memory inode.
	 */
        if (ip->i_number == 0)
        {
                bzero(&ip->i_dinode, DISIZE);
                return(0);
        }

	ipmnt = ip->i_ipmnt;

	/*
	 * If ip->i_number >= 32 (INOSPEREXT), then read from secondary
	 * aggregate inode table.
	 */

	if (ip->i_number >= INOSPEREXT)
	{
		address = addressPXD(&ipmnt->i_ait2) << ipmnt->i_l2bsize;
		inum = ip->i_number - INOSPEREXT;
	}
	else
	{
		address = AITBL_OFF;
		inum = ip->i_number;
	}

	/* read the page of fixed disk inode (AIT) in raw mode */
        if (rc = rawRead(ip->i_ipmnt, address, &bp))
                return(rc);

	/* get the pointer to the disk inode of interest */
        dp = (dinode_t *)(bp->cm_cdata);
        dp += inum;

	/* copy on-disk inode to in-memory inode */
        bcopy(dp, &ip->i_dinode, DISIZE);

	/* release the page */
        rawRelease(bp);

        return(0);
}


/*
 * NAME:        diWrite()
 *
 * FUNCTION:    write the on-disk inode portion of the in-memory inode
 *		to its corresponding on-disk inode.
 *
 *		on entry, the specifed incore inode should itself
 *		specify the disk inode number corresponding to the
 *		incore inode (i.e. i_number should be initialized).
 *
 *		the inode contains the inode extent address for the disk
 *		inode.  with the inode extent address in hand, the
 *		page of the extent that contains the disk inode is
 *		read and the disk inode portion of the incore inode
 *		is copied to the disk inode.
 *		
 * PARAMETERS:
 *	tid -  transacation id
 *      ip  -  pointer to incore inode to be written to the inode extent.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO  	- i/o error.
 */
int32
diWrite(int32	tid,
	inode_t	*ip)
{
	int32		rc;
	int32		ino;
	inode_t		*ipimap;
	dinode_t	*dp;
	jbuf_t		*bp;
	int64		blkno;
	int32		dioffset;
	uint32		type;
	int32		lid;
	tlock_t		*ditlck, *tlck;
	linelock_t	*dilinelock, *ilinelock;
	lv_t		*lv;
	int32		n;

	ipimap = ip->i_ipimap;

	ino = ip->i_number & (INOSPERIAG-1);

	assert(lengthPXD(&(ip->i_ixpxd)) == ipimap->i_imap->im_nbperiext);
	assert(addressPXD(&(ip->i_ixpxd)));

	/*
	 * read the page of disk inode containing the specified inode:
	 */
	/* compute the block address of the page */
	blkno = INOPBLK(&(ip->i_ixpxd),ino,ipimap->i_ipmnt->i_l2nbperpage);

	/* read the page of disk inode */
retry:
        if (rc = bmRead(ipimap, blkno, PSIZE, bmREAD_BLOCK, &bp))
		return(rc);

	/* get the pointer to the disk inode */
        dp = (dinode_t *) bp->b_bdata;
	dp += (ino & (INOSPERPAGE-1));
	dioffset = (ino & (INOSPERPAGE-1)) << L2DISIZE;
	
	/*
	 * acquire transaction lock on the on-disk inode;
	 * N.B. tlock is acquired on ipimap not ip;
	 */
	if ((ditlck = txLock(tid, ipimap, bp, tlckINODE|tlckENTRY)) == NULL)
		goto retry;
	dilinelock = (linelock_t *)&ditlck->lock;

	/*
	 * copy btree root from in-memory inode to on-disk inode
	 *
	 * (tlock is taken from inline B+-tree root in in-memory
	 * inode when the B+-tree root is updated, which is pointed 
	 * by ip->i_blid as well as being on tx tlock list)
	 *
	 * further processing of btree root is based on the copy 
	 * in in-memory inode, where txLog() will log from, and, 
	 * for xtree root, txUpdateMap() will update map and reset
	 * XAD_NEW bit;
	 */ 
btRoot:
	if ((lid = ip->i_blid) == 0)
		goto inlineData;
	ip->i_blid = 0;

	tlck = &TxLock[lid];
	type = tlck->type;
	tlck->type |= tlckBTROOT;
	tlck->bp = bp;
	ilinelock = (linelock_t *)&tlck->lock;

	/*
	 *	regular file: 16 byte (XAD slot) granularity
	 */
	if (type & tlckXTREE)
	{	
		xtpage_t	*p, *xp;
		xad_t		*xad;

		/*
		 * copy xtree root from inode to dinode:
		 */
		p = (xtpage_t *)&ip->i_btroot;
		xp = (xtpage_t *)&dp->di_btroot;
		lv = (lv_t *)&ilinelock->lv;
		for (n = 0; n < ilinelock->index; n++, lv++)
		{
			bcopy(&p->xad[lv->offset], &xp->xad[lv->offset], lv->length << L2XTSLOTSIZE);
		}

		/* reset on-disk (metadata page) xtree XAD_NEW bit */
		xad = &xp->xad[XTENTRYSTART];
		for (n = XTENTRYSTART; n < xp->header.nextindex; n++, xad++)
			if (xad->flag & (XAD_NEW|XAD_EXTENDED))
				xad->flag &= ~(XAD_NEW|XAD_EXTENDED);
	}
	/*
	 *	directory: 32 byte (directory entry slot) granularity
	 */
	else if (type & tlckDTREE)
	{
		dtpage_t	*p, *xp;

		/*
		 * copy dtree root from inode to dinode:
		 */
		p = (dtpage_t *)&ip->i_btroot;
		xp = (dtpage_t *)&dp->di_btroot;
		lv = (lv_t *)&ilinelock->lv;
		for (n = 0; n < ilinelock->index; n++, lv++)
		{
			bcopy(&p->slot[lv->offset], &xp->slot[lv->offset], lv->length << L2DTSLOTSIZE);
		}
	}
	else
	{
		printf("diWrite: UFO tlock\n");
		brkpoint1(ip);
	}

	/*
	 * copy inline data from in-memory inode to on-disk inode:
	 * 128 byte slot granularity
	 */
inlineData:
	if (ip->i_cflag & COMMIT_INLINEEA)
	{
		lv = (lv_t *)&dilinelock->lv[dilinelock->index];
		lv->offset = (dioffset + 3 * 128) >> L2INODESLOTSIZE;
		lv->length = 1;
		bcopy(&ip->i_inlineea, &dp->di_inlineea, INODESLOTSIZE);
		dilinelock->index++;
	
		ip->i_cflag &= ~COMMIT_INLINEEA;
	}

	/*
	 *	lock/copy inode base: 128 byte slot granularity
	 */
baseDinode:
	lv = (lv_t *)&dilinelock->lv[dilinelock->index];
	lv->offset = dioffset >> L2INODESLOTSIZE;
	lv->length = 1;
	bcopy(&ip->i_dinode, dp, INODESLOTSIZE);
	dilinelock->index++;

// BEGIN D233382
#ifdef _JFS_FASTDASD
	/*
	 * We aren't logging changes to the DASD used in directory inodes,
	 * but we need to write them to disk.  If we don't unmount cleanly,
	 * mount will recalculate the DASD used.
	 */
	if (((ip->i_mode & IFMT) == IFDIR) &&
	    (ip->i_ipmnt->i_mntflag & JFS_DASD_ENABLED))
		bcopy(&ip->i_DASD, &dp->di_DASD, sizeof(dasd_t));
#endif /*  _JFS_FASTDASD */
// END D233382

	/* release the buffer holding the updated on-disk inode. 
	 * the buffer will be later written by commit processing.
	 */
	bmRelease(bp);

	return(rc);
}


/*
 * NAME:        diFree(ipimap,iap)
 *
 * FUNCTION:    free a specified inode from the inode working map
 *		for a fileset or aggregate.
 *
 *		if the inode to be freed represents the first (only)
 *		free inode within the iag, the iag will be placed on
 *		the ag free inode list.
 *	
 *		freeing the inode will cause the inode extent to be
 *		freed if the inode is the only allocated inode within
 *		the extent.  in this case all the disk resource backing
 *		up the inode extent will be freed. in addition, the iag
 *		will be placed on the ag extent free list if the extent
 *		is the first free extent in the iag.  if freeing the
 *		extent also means that no free inodes will exist for
 *		the iag, the iag will also be removed from the ag free
 *		inode list.
 *
 *		the iag describing the inode will be freed if the extent
 *		is to be freed and it is the only backed extent within
 *		the iag.  in this case, the iag will be removed from the
 *		ag free extent list and ag free inode list and placed on
 *		the inode map's free iag list.
 *
 *		a careful update approach is used to provide consistency
 *		in the face of updates to multiple buffers.  under this
 *		approach, all required buffers are obtained before making
 *		any updates and are held until all updates are complete.
 *
 * PARAMETERS:
 *      ipimap  - pointer to incore inode for the fileset or aggregate.
 *      iap  	- ialloc_t describing the inode (number) to be freed.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO  	- i/o error.
 */
int32
diFree( inode_t	*ipimap,
	ialloc_t *iap)
{
	int32	rc;
	ino_t	inum = iap->ino;
	iag_t	*iagp, *aiagp, *biagp, *ciagp, *diagp;
	jbuf_t	*bp, *abp, *bbp, *cbp, *dbp;
	int32	iagno, ino, extno, bitno, sword, agno;
	int32	back, fwd, index;
	uint32	bitmap, mask;
	imap_t	*imap = ipimap->i_imap;
	int64	xaddr;
	int32	xlen;
	pxd_t	freepxd;
	int32	tid;
	inode_t	*iplist[3];
	tlock_t	*tlck;
	pxdlock_t	*pxdlock;

	/* get the iag number containing the inode.
	 */
	iagno = INOTOIAG(inum);

	/* make sure that the iag is contained within 
	 * the map.
	 */
	assert(iagno < imap->im_nextiag);

	/* get the allocation group for this ino.
	 */
	agno = BLKTOAG(iap->agstart,ipimap);

	/* Lock the AG specific inode map information
	 */
	AG_LOCK(imap, agno);

	/* Obtain read lock in imap inode.  Don't release it until we have
	 * read all of the IAG's that we are going to.
	 */
	IREAD_LOCK(ipimap);

	/* read the iag.
	 */
        if (rc = diIAGRead(imap,iagno,&bp))
	{
		IREAD_UNLOCK(ipimap);
               	return(rc);
	}
        iagp = (iag_t *) bp->b_bdata;

	/* get the inode number and extent number of the inode within
	 * the iag and the inode number within the extent.
	 */
	ino = inum & (INOSPERIAG-1);
	extno = ino >> L2INOSPEREXT;
	bitno = ino & (INOSPEREXT-1);
	mask = HIGHORDER >> bitno;

	assert(iagp->wmap[extno] & mask);
	assert((iagp->pmap[extno] & mask) == 0);
	assert(addressPXD(&iagp->inoext[extno]));

	/* compute the bitmap for the extent reflecting the freed inode.
	 */
	bitmap = iagp->wmap[extno] & ~mask;

	/*
	 * 	inode extent still has some inodes or below low water mark:
	 *	keep the inode extent;
	 */
	if (bitmap || 
	    imap->im_agctl[agno].numfree < 96 ||
	    (imap->im_agctl[agno].numfree < 288 &&
		(( (imap->im_agctl[agno].numfree * 100) /
		imap->im_agctl[agno].numinos) <= 25)))
	{
		/* if the iag currently has no free inodes (i.e.,
		 * the inode being freed is the first free inode of iag),
		 * insert the iag at head of the inode free list for the ag.
		 */
		if (iagp->nfreeinos == 0)
		{
			/* check if there are any iags on the ag inode
			 * free list.  if so, read the first one so that
			 * we can link the current iag onto the list at
			 * the head.
			 */
			if ((fwd = imap->im_agctl[agno].inofree) >= 0)
			{
				/* read the iag that currently is the head
				 * of the list.
				 */
        			if (rc = diIAGRead(imap,fwd,&abp))
				{
					IREAD_UNLOCK(ipimap);
					AG_UNLOCK(imap, agno);
					bmRelease(bp);
               				return(rc);
				}
        			aiagp = (iag_t *) abp->b_bdata;

				/* make current head point back to the iag.
				 */
				aiagp->inofreeback = iagno;

				bmLazyWrite(abp,0);
			}

			/* iag points forward to current head and iag
			 * becomes the new head of the list.
			 */
			iagp->inofreefwd = imap->im_agctl[agno].inofree;
			iagp->inofreeback = -1;
			imap->im_agctl[agno].inofree = iagno;
		}
		IREAD_UNLOCK(ipimap);

		/* update the free inode summary map for the extent if
		 * freeing the inode means the extent will now have free
		 * inodes (i.e., the inode being freed is the first free 
		 * inode of extent),
		 */
		if (iagp->wmap[extno] == ONES)
		{
			sword = extno >> L2EXTSPERSUM;
			bitno = extno & (EXTSPERSUM-1);
			iagp->inosmap[sword] &= ~(HIGHORDER >> bitno);
		}

		/* update the bitmap.
		 */
		iagp->wmap[extno] = bitmap;
		DBG_DIFREE(imap, inum);

		/* update the free inode counts at the iag, ag and
		 * map level.
		 */
		iagp->nfreeinos += 1;
		imap->im_agctl[agno].numfree += 1;
		ATOMIC_ADD(imap->im_numfree, 1);

		/* release the AG inode map lock
		 */
		AG_UNLOCK(imap, agno);

		/* write the iag */
		bmLazyWrite(bp,0);

		return(0);
	}


	/*
	 *	inode extent has become free and above low water mark:
	 *	free the inode extent;
	 */

	/*
	 *	prepare to update iag list(s) (careful update step 1)
	 */
	abp = bbp = cbp = dbp = NULL;
	fwd = back = -1;

	/* check if the iag currently has no free extents.  if so,
	 * it will be placed on the head of the ag extent free list.
	 */
	if (iagp->nfreeexts == 0)
	{
		/* check if the ag extent free list has any iags.
		 * if so, read the iag at the head of the list now.
		 * this (head) iag will be updated later to reflect
		 * the addition of the current iag at the head of
		 * the list.
		 */
		if ((fwd = imap->im_agctl[agno].extfree) >= 0)
		{
        		if (rc = diIAGRead(imap,fwd,&abp))
				goto error_out;
        		aiagp = (iag_t *) abp->b_bdata;
		}
	}
	else
	{
		/* iag has free extents. check if the addition of a free
		 * extent will cause all extents to be free within this
		 * iag.  if so, the iag will be removed from the ag extent
		 * free list and placed on the inode map's free iag list.
		 */
		if (iagp->nfreeexts == (EXTSPERIAG-1))
		{
			/* in preparation for removing the iag from the
			 * ag extent free list, read the iags preceeding
			 * and following the iag on the ag extent free
			 * list.
			 */
			if ((fwd = iagp->extfreefwd) >= 0)
			{
        			if (rc = diIAGRead(imap,fwd,&abp))
					goto error_out;
        			aiagp = (iag_t *) abp->b_bdata;
			}

			if ((back = iagp->extfreeback) >= 0)
			{
        			if (rc = diIAGRead(imap,back,&bbp))
					goto error_out;
        			biagp = (iag_t *) bbp->b_bdata;
			}
		}
	}

	/* remove the iag from the ag inode free list if freeing
	 * this extent cause the iag to have no free inodes.
	 */
	if (iagp->nfreeinos == (INOSPEREXT-1))
	{
		/* in preparation for removing the iag from the
		 * ag inode free list, read the iags preceeding
		 * and following the iag on the ag inode free
		 * list.  before reading these iags, we must make
	 	 * sure that we already don't have them in hand
		 * from up above, since re-reading an iag (buffer)
		 * we are currently holding would cause a deadlock.
		 */
		if (iagp->inofreefwd >= 0)
		{
			if (iagp->inofreefwd == fwd)
        			ciagp = (iag_t *) abp->b_bdata;
			else if (iagp->inofreefwd == back)
        			ciagp = (iag_t *) bbp->b_bdata;
			else
			{
				if (rc = diIAGRead(imap,iagp->inofreefwd,&cbp))
					goto error_out;
				assert(cbp != NULL);
        			ciagp = (iag_t *) cbp->b_bdata;
			}
			assert(ciagp != NULL);
		}

		if (iagp->inofreeback >= 0)
		{
			if (iagp->inofreeback == fwd)
        			diagp = (iag_t *) abp->b_bdata;
			else if (iagp->inofreeback == back)
        			diagp = (iag_t *) bbp->b_bdata;
			else
			{
        			if (rc = diIAGRead(imap,iagp->inofreeback,&dbp))
					goto error_out;
				assert(dbp != NULL);
        			diagp = (iag_t *) dbp->b_bdata;
			}
			assert(diagp != NULL);
		}
	}

	IREAD_UNLOCK(ipimap);

	/*
	 * invalidate any page of the inode extent freed from buffer cache;
	 */
	freepxd = iagp->inoext[extno];
	xaddr = addressPXD(&iagp->inoext[extno]);
	xlen = lengthPXD(&iagp->inoext[extno]);
	bmExtentInvalidate(ipimap, xaddr, xlen);

	/*
	 *	update iag list(s) (careful update step 2)
	 */
	/* add the iag to the ag extent free list if this is the
	 * first free extent for the iag.
	 */
	if (iagp->nfreeexts == 0)
	{
		if (fwd >= 0)
			aiagp->extfreeback = iagno;

		iagp->extfreefwd = imap->im_agctl[agno].extfree;
		iagp->extfreeback = -1;
		imap->im_agctl[agno].extfree = iagno;
	}
	else
	{
		/* remove the iag from the ag extent list if all extents
		 * are now free and place it on the inode map iag free list.
		 */
		if (iagp->nfreeexts == (EXTSPERIAG-1))
		{
			if (fwd >= 0)
				aiagp->extfreeback = iagp->extfreeback;

			if (back >= 0)
				biagp->extfreefwd = iagp->extfreefwd;
			else
				imap->im_agctl[agno].extfree = iagp->extfreefwd;

			iagp->extfreefwd = iagp->extfreeback = -1;

			IAGFREE_LOCK(imap);
			iagp->iagfree = imap->im_freeiag;
			imap->im_freeiag = iagno;
			IAGFREE_UNLOCK(imap);
		}
	}

	/* remove the iag from the ag inode free list if freeing
	 * this extent causes the iag to have no free inodes.
	 */
	if (iagp->nfreeinos == (INOSPEREXT-1))
	{
		if (iagp->inofreefwd >= 0)
			ciagp->inofreeback = iagp->inofreeback;

		if (iagp->inofreeback >= 0)
			diagp->inofreefwd = iagp->inofreefwd;
		else
			imap->im_agctl[agno].inofree = iagp->inofreefwd;

		iagp->inofreefwd = iagp->inofreeback = -1;
	}

	/* update the inode extent address and working map 
	 * to reflect the free extent.
	 * the permanent map should have been updated already 
	 * for the inode being freed.
	 */
	assert(iagp->pmap[extno] == 0);
	iagp->wmap[extno] = 0;
	DBG_DIFREE(imap, inum);
	PXDlength(&iagp->inoext[extno],0);
	PXDaddress(&iagp->inoext[extno],0);

	/* update the free extent and free inode summary maps
	 * to reflect the freed extent.
	 * the inode summary map is marked to indicate no inodes 
	 * available for the freed extent.
	 */
	sword = extno >> L2EXTSPERSUM;
	bitno = extno & (EXTSPERSUM-1);
	mask = HIGHORDER >> bitno;
	iagp->inosmap[sword] |= mask;
	iagp->extsmap[sword] &= ~mask;

	/* update the number of free inodes and number of free extents
	 * for the iag.
	 */
	iagp->nfreeinos -= (INOSPEREXT-1);
	iagp->nfreeexts += 1;

	/* update the number of free inodes and backed inodes
	 * at the ag and inode map level.
	 */
	imap->im_agctl[agno].numfree -= (INOSPEREXT-1);
	imap->im_agctl[agno].numinos -= INOSPEREXT;
	ATOMIC_ADD(imap->im_numfree, -(INOSPEREXT-1));
	ATOMIC_ADD(imap->im_numinos, -INOSPEREXT);

	if (abp) bmLazyWrite(abp,0);
	if (bbp) bmLazyWrite(bbp,0);
	if (cbp) bmLazyWrite(cbp,0);
	if (dbp) bmLazyWrite(dbp,0);

	/*
	 * start transaction to update block allocation map
	 * for the inode extent freed;
	 *
	 * N.B. AG_LOCK is released and iag will be released below, and 
	 * other thread may allocate inode from/reusing the ixad freed
	 * BUT with new/different backing inode extent from the extent 
	 * to be freed by the transaction;  
	 */
	txBegin(ipimap->i_ipmnt, &tid, COMMIT_FORCE);

	/* acquire tlock of the iag page of the freed ixad 
	 * to force the page NOHOMEOK (even though no data is
	 * logged from the iag page) until NOREDOPAGE|FREEXTENT log 
	 * for the free of the extent is committed;
	 * write FREEXTENT|NOREDOPAGE log record
	 * N.B. linelock is overlaid as freed extent descriptor;
	 */ 
	tlck = txLock(tid, ipimap, bp, tlckINODE|tlckFREE);
	pxdlock = (pxdlock_t *)&tlck->lock;
	pxdlock->flag = mlckFREEPXD;
	pxdlock->pxd = freepxd;
	pxdlock->index = 1;

	bmRelease(bp);

	ipimap->i_flag |= IFSYNC;
	iplist[0] = ipimap;

	/*
	 * logredo needs the IAG number and IAG extent index in order
	 * to ensure that the IMap is consistent.  The least disruptive
	 * way to pass these values through  to the transaction manager
	 * is in the iplist array.  
	 * 
	 * It's not pretty, but it works.
	 */
	iplist[1] = (inode_t *) iagno;
	iplist[2] = (inode_t *) extno;

	rc = txCommit(tid, 1, &iplist[0], COMMIT_FORCE);	// D233382

	txEnd(tid);

	/* unlock the AG inode map information */
	AG_UNLOCK(imap, agno);

	return(0);

error_out:
	IREAD_UNLOCK(ipimap);

	if (abp) bmRelease(abp);
	if (bbp) bmRelease(bbp);
	if (cbp) bmRelease(cbp);
	if (dbp) bmRelease(dbp);

	AG_UNLOCK(imap, agno);

	bmRelease(bp);

	return(rc);
}


/*
 * NAME:        diAlloc(pip,dir,iap)
 *
 * FUNCTION:    allocate a disk inode from the inode working map 
 *		for a fileset or aggregate.
 *
 * PARAMETERS:
 *      pip  	- pointer to incore inode for the parent inode.
 *      dir  	- TRUE if the new disk inode is for a directory.
 *      iap  	- pointer to a ialloc_t to be filled in on successful return
 *		  with the disk inode number allocated, its extent address
 *		  and the start of the ag.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
int32
diAlloc(inode_t		*pip,
	boolean_t	dir,
	ialloc_t	*iap)
{
	int32 rc, ino, iagno, addext, extno, bitno, sword;
	int32 nwords, rem, i, agno;
	uint32 mask, inosmap, extsmap;
	inode_t *ipimap;
	jbuf_t *bp;
	ino_t inum;
	iag_t *iagp;
	imap_t *imap;

	/* get the pointers to the inode map inode and the
	 * corresponding imap control structure.
	 */
	ipimap = pip->i_ipimap;
	imap = ipimap->i_imap;

	/* for a directory, the allocation policy is to start 
	 * at the ag level using the preferred ag.
	 */
	if (dir == TRUE)
	{
		agno = dbNextAG(ipimap->i_ipmnt->i_ipbmap);
		AG_LOCK(imap, agno);
		goto tryag;
	}

	/* for files, the policy starts off by trying to allocate from
	 * the same iag containing the parent disk inode:
	 * try to allocate the new disk inode close to the parent disk
	 * inode, using parent disk inode number + 1 as the allocation
	 * hint.  (we use a left-to-right policy to attempt to avoid
	 * moving backward on the disk.)  compute the hint within the
	 * file system and the iag.
	 */
	inum = pip->i_number + 1;
	ino = inum & (INOSPERIAG-1);

	/* back off the the hint if it is outside of the iag */
	if (ino == 0)
		inum = pip->i_number;

	/* get the ag number of this iag */
	agno = BLKTOAG(pip->i_agstart,ipimap);

	/* lock the AG inode map information */
	AG_LOCK(imap, agno);

	/* Get read lock on imap inode */
	IREAD_LOCK(ipimap);

	/* get the iag number and read the iag */
	iagno = INOTOIAG(inum);
        if (rc = diIAGRead(imap,iagno,&bp))
	{
		IREAD_UNLOCK(ipimap);
		return(rc);
	}
        iagp = (iag_t *) bp->b_bdata;

	/* determine if new inode extent is allowed to be added to the iag.
	 * new inode extent can be added to the iag if the ag
	 * has less than 32 free disk inodes and the iag has free extents.
	 */
	addext = (imap->im_agctl[agno].numfree < 32 && iagp->nfreeexts);

	/*
	 *	try to allocate from the IAG
	 */
	/* check if the inode may be allocated from the iag 
	 * (i.e. the inode has free inodes or new extent can be added).
	 */
	if (iagp->nfreeinos || addext)
	{
		/* determine the extent number of the hint.
		 */
		extno = ino >> L2INOSPEREXT;

		/* check if the extent containing the hint has backed
		 * inodes.  if so, try to allocate within this extent.
		 */
		if (addressPXD(&iagp->inoext[extno]))
		{
			bitno = ino & (INOSPEREXT-1);
			if ((bitno = diFindFree(iagp->wmap[extno],bitno))
								< INOSPEREXT)
			{
				ino = (extno << L2INOSPEREXT) + bitno;

				/* a free inode (bit) was found within this
				 * extent, so allocate it.
				 */
				rc = diAllocBit(imap,iagp,ino);
				IREAD_UNLOCK(ipimap);
				if (rc)
				{
					assert(rc == EIO);
        				bmRelease(bp);
				}
				else
				{
					/* set the results of the allocation
					 * and write the iag.
					 */
					iap->ino = (iagno << L2INOSPERIAG)
								 + ino;
					DBG_DIALLOC(imap, iap->ino);
					iap->ixpxd = iagp->inoext[extno];
					iap->agstart = iagp->agstart;
 					bmLazyWrite(bp, 0);
				}

				/* free the AG lock and return.
				 */
				AG_UNLOCK(imap, agno);
				return(rc);
			}

			if (!addext)
				extno = (extno == EXTSPERIAG-1) ? 0 : extno+1;
		}

		/*
		 * no free inodes within the extent containing the hint.
		 *
		 * try to allocate from the backed extents following
		 * hint or, if appropriate (i.e. addext is true), allocate
		 * an extent of free inodes at or following the extent
		 * containing the hint.
		 * 
		 * the free inode and free extent summary maps are used
		 * here, so determine the starting summary map position
		 * and the number of words we'll have to examine.  again,
		 * the approach is to allocate following the hint, so we
		 * might have to initially ignore prior bits of the summary
		 * map that represent extents prior to the extent containing
		 * the hint and later revisit these bits.
		 */
		bitno = extno & (EXTSPERSUM-1);
		nwords = (bitno == 0) ? SMAPSZ : SMAPSZ+1;        
		sword = extno >> L2EXTSPERSUM;
	
		/* mask any prior bits for the starting words of the
		 * summary map.
		 */
		mask = ONES << (EXTSPERSUM - bitno);
		inosmap = iagp->inosmap[sword] | mask;
		extsmap = iagp->extsmap[sword] | mask;

		/* scan the free inode and free extent summary maps for
		 * free resources.
		 */
		for (i = 0; i < nwords; i++)
		{
			/* check if this word of the free inode summary
			 * map describes an extent with free inodes.
			 */
			if (~inosmap)
			{
				/* an extent with free inodes has been
				 * found. determine the extent number
				 * and the inode number within the extent.
				 */
				rem = diFindFree(inosmap,0);
				extno = (sword << L2EXTSPERSUM) + rem;
				rem = diFindFree(iagp->wmap[extno],0);
				assert(rem < INOSPEREXT);

				/* determine the inode number within the
				 * iag and allocate the inode from the
				 * map.
				 */
				ino = (extno << L2INOSPEREXT) + rem;
				rc = diAllocBit(imap,iagp,ino);
				IREAD_UNLOCK(ipimap);
				if (rc)
				{
					assert(rc == EIO);
        				bmRelease(bp);
				}
				else
				{
					/* set the results of the allocation
					 * and write the iag.
					 */
					iap->ino = (iagno << L2INOSPERIAG)
								 + ino;
					DBG_DIALLOC(imap, iap->ino);
					iap->ixpxd = iagp->inoext[extno];
					iap->agstart = iagp->agstart;
 					bmLazyWrite(bp, 0);
				}

				/* free the AG lock and return.
				 */
				AG_UNLOCK(imap, agno);
				return(rc);
		
			}

			/* check if we may allocate an extent of free
			 * inodes and whether this word of the free
			 * extents summary map describes a free extent.
			 */
			if (addext && ~extsmap)
			{
				/* a free extent has been found.  determine
				 * the extent number.
				 */
				rem = diFindFree(extsmap,0);
				extno = (sword << L2EXTSPERSUM) + rem;

				/* allocate an extent of free inodes.
				 */
				if (rc = diNewExt(imap,iagp,extno))
				{
					/* if there is no disk space for a
					 * new extent, try to allocate the
					 * disk inode from somewhere else.
					 */
					if (rc == ENOSPC)
						break;

					assert(rc == EIO);
        				bmRelease(bp);
				}
				else
				{
					/* set the results of the allocation
					 * and write the iag.
					 */
					iap->ino = (iagno << L2INOSPERIAG) + 
							(extno << L2INOSPEREXT);
					DBG_DIALLOC(imap, iap->ino);
					iap->ixpxd = iagp->inoext[extno];
					iap->agstart = iagp->agstart;
 					bmLazyWrite(bp, 0);
				}
				/* free the imap inode & the AG lock & return.
				 */
				IREAD_UNLOCK(ipimap);
				AG_UNLOCK(imap, agno);
				return(rc);
			}

			/* move on to the next set of summary map words.
			 */
			sword = (sword == SMAPSZ-1) ? 0 : sword+1;
			inosmap = iagp->inosmap[sword];
			extsmap = iagp->extsmap[sword];
		}
	}
	/* unlock imap inode */
	IREAD_UNLOCK(ipimap);

	/* nothing doing in this iag, so release it. */
        bmRelease(bp);

tryag:
	/*
	 * try to allocate anywhere within the same AG as the parent inode.
	 */
	rc = diAllocAG(imap,agno,dir,iap);

	AG_UNLOCK(imap, agno);

	if (rc != ENOSPC)
		return(rc);

	/*
	 * try to allocate in any AG.
	 */
	return(diAllocAny(imap,agno,dir,iap));
}


/*
 * NAME:        diAllocAG(imap,agno,dir,iap)
 *
 * FUNCTION:    allocate a disk inode from the allocation group.
 *
 *		this routine first determines if a new extent of free
 *		inodes should be added for the allocation group, with
 *		the current request satisfied from this extent. if this
 *		is the case, an attempt will be made to do just that.  if
 *		this attempt fails or it has been determined that a new 
 *		extent should not be added, an attempt is made to satisfy
 *		the request by allocating an existing (backed) free inode
 *		from the allocation group.
 *
 * PRE CONDITION: Already have the AG lock for this AG.
 *
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      agno  	- allocation group to allocate from.
 *      dir  	- TRUE if the new disk inode is for a directory.
 *      iap  	- pointer to a ialloc_t to be filled in on successful return
 *		  with the disk inode number allocated, its extent address
 *		  and the start of the ag.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
static int32
diAllocAG(imap_t	 *imap,
	  int32		agno,
	  boolean_t	dir,
	  ialloc_t	*iap)
{
	int32	rc, addext, numfree, numinos;

	/* get the number of free and the number of backed disk 
	 * inodes currently within the ag.
	 */
	numfree = imap->im_agctl[agno].numfree;
	numinos = imap->im_agctl[agno].numinos;

	/* determine if we should allocate a new extent of free inodes
	 * within the ag: for directory inodes, add a new extent
	 * if there are a small number of free inodes or number of free
	 * inodes is a small percentage of the number of backed inodes.
	 */
	if (dir == TRUE)
		addext = (numfree < 64 ||
		 (numfree < 256 && ((numfree * 100) / numinos) <= 20));
	else
		addext = (numfree == 0);

	/*
	 * try to allocate a new extent of free inodes.
	 */
	if (addext)
	{
		/* if free space is not avaliable for this new extent, try
		 * below to allocate a free and existing (already backed)
		 * inode from the ag.
		 */
		if ((rc = diAllocExt(imap,agno,iap)) != ENOSPC)
			return(rc);
	}

	/*
	 * try to allocate an existing free inode from the ag.
	 */
	return(diAllocIno(imap,agno,iap));
}


/*
 * NAME:        diAllocAny(imap,agno,dir,iap)
 *
 * FUNCTION:    allocate a disk inode from any other allocation group.
 *
 *		this routine is called when an allocation attempt within
 *		the primary allocation group has failed. if attempts to
 *		allocate an inode from any allocation group other than the
 *		specified primary group.
 *
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      agno  	- primary allocation group (to avoid).
 *      dir  	- TRUE if the new disk inode is for a directory.
 *      iap  	- pointer to a ialloc_t to be filled in on successful return
 *		  with the disk inode number allocated, its extent address
 *		  and the start of the ag.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
static  int32
diAllocAny(imap_t	*imap,
	   int32	agno,
	   boolean_t	dir,
	   ialloc_t	*iap)
{
	int32 ag, rc;
	int32 maxag = imap->im_ipimap->i_ipmnt->i_ipbmap->i_bmap->db_maxag;


	/* try to allocate from the ags following agno up to 
	 * the maximum ag number.
	 */
	for (ag = agno+1; ag <= maxag; ag++)
	{
		AG_LOCK(imap, ag);

		rc = diAllocAG(imap,ag,dir,iap);

		AG_UNLOCK(imap, ag);

		if (rc != ENOSPC)
			return(rc);
	}

	/* try to allocate from the ags in front of agno.
	 */
	for (ag = 0; ag < agno; ag++)
	{
		AG_LOCK(imap, ag);

		rc = diAllocAG(imap,ag,dir,iap);

		AG_UNLOCK(imap, ag);

		if (rc != ENOSPC)
			return(rc);
	}

	/* no free disk inodes.
	 */
	return(ENOSPC);
}


/*
 * NAME:        diAllocIno(imap,agno,iap)
 *
 * FUNCTION:    allocate a disk inode from the allocation group's free
 *		inode list, returning an error if this free list is
 *		empty (i.e. no iags on the list).
 *
 *		allocation occurs from the first iag on the list using
 *		the iag's free inode summary map to find the leftmost
 *		free inode in the iag. 
 *		
 * PRE CONDITION: Already have AG lock for this AG.
 *		
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      agno  	- allocation group.
 *      iap  	- pointer to a ialloc_t to be filled in on successful return
 *		  with the disk inode number allocated, its extent address
 *		  and the start of the ag.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
static int32
diAllocIno(imap_t	*imap,
	   int32	agno,
	   ialloc_t	*iap)
{
	int32 iagno, ino, rc, rem, extno, sword;
	jbuf_t *bp;
	iag_t *iagp;

	/* check if there are iags on the ag's free inode list.
	 */
	if ((iagno = imap->im_agctl[agno].inofree) < 0)
		return(ENOSPC);

	/* obtain read lock on imap inode */
	IREAD_LOCK(imap->im_ipimap);

	/* read the iag at the head of the list.
	 */
        if (rc = diIAGRead(imap,iagno,&bp))
	{
		IREAD_UNLOCK(imap->im_ipimap);
               	return(rc);
	}
        iagp = (iag_t *) bp->b_bdata;

	/* better be free inodes in this iag if it is on the
	 * list.
	 */
	assert(iagp->nfreeinos);

	/* scan the free inode summary map to find an extent
	 * with free inodes.
	 */
	for (sword = 0; ; sword++)
	{
		assert(sword < SMAPSZ);

		if (~iagp->inosmap[sword])
			break;
	}

	/* found a extent with free inodes. determine
	 * the extent number.
	 */
	rem = diFindFree(iagp->inosmap[sword],0);
	assert(rem < EXTSPERSUM);
	extno = (sword << L2EXTSPERSUM) + rem;

	/* find the first free inode in the extent.
	 */
	rem = diFindFree(iagp->wmap[extno],0);
	assert(rem < INOSPEREXT);

	/* compute the inode number within the iag. 
	 */
	ino = (extno << L2INOSPEREXT) + rem;

	/* allocate the inode.
	 */
	rc = diAllocBit(imap,iagp,ino);
	IREAD_UNLOCK(imap->im_ipimap);
	if (rc)
	{
        	bmRelease(bp);
		return(rc);
	}

	/* set the results of the allocation and write the iag.
	 */
	iap->ino = (ino_t) (iagno << L2INOSPERIAG) + ino;
	DBG_DIALLOC(imap, iap->ino);
	iap->ixpxd = iagp->inoext[extno];
	iap->agstart = iagp->agstart;
 	bmLazyWrite(bp, 0);

	return(0);
}


/*
 * NAME:        diAllocExt(imap,agno,iap)
 *
 * FUNCTION:   	add a new extent of free inodes to an iag, allocating
 *	       	an inode from this extent to satisfy the current allocation
 *	       	request.
 *		
 *		this routine first tries to find an existing iag with free
 *		extents through the ag free extent list.  if list is not
 *		empty, the head of the list will be selected as the home
 *		of the new extent of free inodes.  otherwise (the list is
 *		empty), a new iag will be allocated for the ag to contain
 *		the extent.
 *		
 *		once an iag has been selected, the free extent summary map
 *		is used to locate a free extent within the iag and diNewExt()
 *		is called to initialize the extent, with initialization
 *		including the allocation of the first inode of the extent
 *		for the purpose of satisfying this request.
 *
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      agno  	- allocation group number.
 *      iap  	- pointer to a ialloc_t to be filled in on successful return
 *		  with the disk inode number allocated, its extent address
 *		  and the start of the ag.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
static int32
diAllocExt(imap_t	*imap,
	   int32	agno,
	   ialloc_t	*iap)
{
	int32 rem, iagno, sword, extno, rc;
	jbuf_t *bp;
	iag_t *iagp;

	/* check if the ag has any iags with free extents.  if not,
	 * allocate a new iag for the ag.
	 */
	if ((iagno = imap->im_agctl[agno].extfree) < 0)
	{
		/* If successful, diNewIAG will obtain the read lock on the
		 * imap inode.
		 */
		if (rc = diNewIAG(imap,&iagno,agno,&bp))
		{
			return(rc);
		}
        	iagp = (iag_t *) bp->b_bdata;

		/* set the ag number if this a brand new iag
		 */
		iagp->agstart = AGTOBLK(agno,imap->im_ipimap);
	}	
	else 
	{
		/* read the iag.
		 */
		IREAD_LOCK(imap->im_ipimap);
	        if (rc = diIAGRead(imap,iagno,&bp))
		{
			assert(0);
		}
        	iagp = (iag_t *) bp->b_bdata;
	}

	/* using the free extent summary map, find a free extent.
	 */
	for (sword = 0; ; sword++)
	{
		assert(sword < SMAPSZ);
		if (~iagp->extsmap[sword])
			break;
	}

	/* determine the extent number of the free extent.
	 */
	rem = diFindFree(iagp->extsmap[sword],0);
	assert(rem < EXTSPERSUM);
	extno = (sword << L2EXTSPERSUM) + rem;

	/* initialize the new extent.
	 */
	rc = diNewExt(imap,iagp,extno);
	IREAD_UNLOCK(imap->im_ipimap);
	if (rc)
	{
		/* something bad happened.  if a new iag was allocated,
		 * place it back on the inode map's iag free list, and
		 * clear the ag number information.
		 */
		if (iagp->nfreeexts == EXTSPERIAG)
		{
			IAGFREE_LOCK(imap);
			iagp->iagfree = imap->im_freeiag;
			imap->im_freeiag = iagno;
			IAGFREE_UNLOCK(imap);
		}
        	bmLazyWrite(bp, 0);
		return(rc);
	}

	/* set the results of the allocation and write the iag.
	 */
	iap->ino = (ino_t) (iagno << L2INOSPERIAG) + (extno << L2INOSPEREXT);
	DBG_DIALLOC(imap, iap->ino);
	iap->ixpxd = iagp->inoext[extno];
	iap->agstart = iagp->agstart;

 	bmLazyWrite(bp, 0);

	return(0);
}


/*
 * NAME:        diAllocBit(imap,iagp,ino)
 *
 * FUNCTION:   	allocate a backed inode from an iag.
 *
 *		this routine performs the mechanics of allocating a
 *		specified inode from a backed extent.
 *
 *		if the inode to be allocated represents the last free
 *		inode within the iag, the iag will be removed from the
 *		ag free inode list.
 *
 *		a careful update approach is used to provide consistency
 *		in the face of updates to multiple buffers.  under this
 *		approach, all required buffers are obtained before making
 *		any updates and are held all are updates are complete.
 *		
 * PRE CONDITION: Already have buffer lock on iagp.  Already have AG lock on
 *	this AG.  Must have read lock on imap inode.
 *
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      iagp  	- pointer to iag. 
 *      ino   	- inode number to be allocated within the iag.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
static int32
diAllocBit(imap_t	*imap,
	   iag_t	*iagp,
	   int32	ino)
{
	int32 extno, bitno, agno, sword, rc;
	jbuf_t *abp, *bbp;
	iag_t *aiagp, *biagp;
	uint32 mask;

	/* check if this is the last free inode within the iag.
	 * if so, it will have to be removed from the ag free
	 * inode list, so get the iags preceeding and following
	 * it on the list.
	 */
	if (iagp->nfreeinos == 1)
	{
		abp = bbp = NULL;

		if (iagp->inofreefwd >= 0)
		{
        		if (rc = diIAGRead(imap,iagp->inofreefwd,&abp))
				return(rc);
        		aiagp = (iag_t *) abp->b_bdata;
		}

		if (iagp->inofreeback >= 0)
		{
        		if (rc = diIAGRead(imap,iagp->inofreeback,&bbp))
			{
				if (abp)
        				bmRelease(abp);
               			return(rc);
			}
        		biagp = (iag_t *) bbp->b_bdata;
		}
	}

	/* get the ag number, extent number, inode number within
	 * the extent.
	 */
	agno = BLKTOAG(iagp->agstart,imap->im_ipimap);
	extno = ino >> L2INOSPEREXT;
	bitno = ino & (INOSPEREXT-1);

	/* compute the mask for setting the map.
	 */
	mask = HIGHORDER >> bitno;

	/* the inode should be free and backed.
	 */
	assert((iagp->pmap[extno] & mask) == 0);
	assert((iagp->wmap[extno] & mask) == 0);
	assert(addressPXD(&iagp->inoext[extno]) != 0);

	/* mark the inode as allocated in the working map.
	 */
	iagp->wmap[extno] |= mask;

	/* check if all inodes within the extent are now
	 * allocated.  if so, update the free inode summary
	 * map to reflect this.
	 */
	if (iagp->wmap[extno] == ONES)
	{
		sword = extno >> L2EXTSPERSUM;
		bitno = extno & (EXTSPERSUM-1);
		iagp->inosmap[sword] |= (HIGHORDER >> bitno);
	}

	/* if this was the last free inode in the iag, remove the
	 * iag from the ag free inode list.
	 */
	if (iagp->nfreeinos == 1)
	{
		if (abp)
		{
        		aiagp->inofreeback = iagp->inofreeback;
 			bmLazyWrite(abp, 0);
		}

		if (bbp)
		{
        		biagp->inofreefwd = iagp->inofreefwd;
 			bmLazyWrite(bbp, 0);
		}
		else
		{
			imap->im_agctl[agno].inofree = iagp->inofreefwd;
		}
		iagp->inofreefwd = iagp->inofreeback = -1;
	}

	/* update the free inode count at the iag, ag, inode
	 * map levels.
	 */
	iagp->nfreeinos -= 1;
	imap->im_agctl[agno].numfree -= 1;
	ATOMIC_ADD(imap->im_numfree, -1);

	return(0);
}


/*
 * NAME:        diNewExt(imap,iagp,extno)
 *
 * FUNCTION:    initialize a new extent of inodes for an iag, allocating
 *	        the first inode of the extent for use for the current
 *	        allocation request.
 *
 *		disk resources are allocated for the new extent of inodes
 *		and the inodes themselves are initialized to reflect their
 *		existence within the extent (i.e. their inode numbers and
 *		inode extent addresses are set) and their initial state
 *		(mode and link count are set to zero).
 *
 *		if the iag is new, it is not yet on an ag extent free list
 *		but will now be placed on this list.
 *
 *		if the allocation of the new extent causes the iag to
 *		have no free extent, the iag will be removed from the
 *		ag extent free list.
 *
 *		if the iag has no free backed inodes, it will be placed
 *		on the ag free inode list, since the addition of the new
 *		extent will now cause it to have free inodes.
 *
 *		a careful update approach is used to provide consistency
 *		(i.e. list consistency) in the face of updates to multiple
 *		buffers.  under this approach, all required buffers are
 *		obtained before making any updates and are held until all
 *		updates are complete.
 *		
 * PRE CONDITION: Already have buffer lock on iagp.  Already have AG lock on
 *	this AG.  Must have read lock on imap inode.
 *
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      iagp  	- pointer to iag. 
 *      extno  	- extent number.
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 */
static int32
diNewExt(imap_t	*imap,
	 iag_t	*iagp,
	 int32	extno)
{
	int32 agno, iagno, fwd, back, freei, sword, rc;
	iag_t *aiagp, *biagp, *ciagp;
	jbuf_t *abp, *bbp, *cbp, *dbp;
	inode_t *ipimap, *ipbmap;
	int64 blkno, hint;
	int32 i, j;
	uint32 mask;
	ino_t ino;
	dinode_t *dp;

	/* better have free extents.
	 */
	assert(iagp->nfreeexts);

	/* get the inode map inode.
	 */
	ipimap = imap->im_ipimap;

	abp = bbp = cbp = NULL;

	/* get the ag and iag numbers for this iag.
	 */
	agno = BLKTOAG(iagp->agstart,ipimap);
	iagno = iagp->iagnum;

	/* check if this is the last free extent within the
	 * iag.  if so, the iag must be removed from the ag
	 * free extent list, so get the iags preceeding and
	 * following the iag on this list.
	 */
	if (iagp->nfreeexts == 1)
	{
		if ((fwd = iagp->extfreefwd) >= 0)
		{
        		if (rc = diIAGRead(imap,fwd,&abp))
				return(rc);
        		aiagp = (iag_t *) abp->b_bdata;
		}

		if ((back = iagp->extfreeback) >= 0)
		{
        		if (rc = diIAGRead(imap,back,&bbp))
				goto error_out;
        		biagp = (iag_t *) bbp->b_bdata;
		}
	}
	else
	{
		/* the iag has free extents.  if all extents are free
		 * (as is the case for a newly allocated iag), the iag
		 * must be added to the ag free extent list, so get
		 * the iag at the head of the list in preparation for
		 * adding this iag to this list.
		 */
		fwd = back = -1;
		if (iagp->nfreeexts == EXTSPERIAG)
		{
			if ((fwd = imap->im_agctl[agno].extfree) >= 0)
			{
        			if (rc = diIAGRead(imap,fwd,&abp))
					goto error_out;
        			aiagp = (iag_t *) abp->b_bdata;
			}
		}
	}

	/* check if the iag has no free inodes.  if so, the iag
	 * will have to be added to the ag free inode list, so get
	 * the iag at the head of the list in preparation for
	 * adding this iag to this list.  in doing this, we must
	 * check if we already have the iag at the head of
	 * the list in hand.
	 */
	if (iagp->nfreeinos == 0)
	{
		freei = imap->im_agctl[agno].inofree;

		if (freei >= 0)
		{
			if (freei == fwd)
			{
        			ciagp = aiagp;
			}
			else if (freei == back)
			{
        			ciagp = biagp;
			}
			else
			{
        			if (rc = diIAGRead(imap,freei,&cbp))
					goto error_out;
        			ciagp = (iag_t *) cbp->b_bdata;
			}
			assert(ciagp != NULL);
		}
	}

	/* allocate disk space for the inode extent.
	 */
	if ((extno == 0) || (addressPXD(&iagp->inoext[extno-1]) == 0))
	{
		ipbmap = ipimap->i_ipmnt->i_ipbmap;
		hint = (int64)agno << ipbmap->i_bmap->db_agl2size;
	}
	else
	{
		hint = addressPXD(&iagp->inoext[extno-1]) +
			lengthPXD(&iagp->inoext[extno-1]) - 1;
	}
	if (rc = dbAlloc(ipimap, hint, (int64)imap->im_nbperiext, &blkno))
		goto error_out;

	/* compute the inode number of the first inode within the
	 * extent.
	 */
	ino = (iagno << L2INOSPERIAG) + (extno << L2INOSPEREXT);

	/* initialize the inodes within the newly allocated extent a
	 * page at a time.
	 */
        for (i = 0; i < imap->im_nbperiext; i += ipimap->i_ipmnt->i_nbperpage)
        {
		/* get a buffer for this page of disk inodes.
	 	 */
		dbp = bmAssign(ipimap, blkno+i, blkno+i, PSIZE, bmREAD_BLOCK);
                dp = (dinode_t *)dbp->b_bdata;

		/* initialize the inode number, mode, link count and
		 * inode extent address.
		 */
                for (j = 0; j < INOSPERPAGE; j++, dp++, ino++)
                {
			dp->di_inostamp = ipimap->i_inostamp;
                        dp->di_number = ino;
                        dp->di_mode = 0;
                        dp->di_nlink = 0;
			PXDaddress(&(dp->di_ixpxd), blkno);
			PXDlength(&(dp->di_ixpxd), imap->im_nbperiext);
                }
                bmLazyWrite(dbp, 0);
        }

	/* if this is the last free extent within the iag, remove the
	 * iag from the ag free extent list.
	 */
	if (iagp->nfreeexts == 1)
	{
		if (fwd >= 0)
			aiagp->extfreeback = iagp->extfreeback;

		if (back >= 0)
			biagp->extfreefwd = iagp->extfreefwd;
		else
			imap->im_agctl[agno].extfree = iagp->extfreefwd;

		iagp->extfreefwd = iagp->extfreeback = -1;
	}
	else
	{
		/* if the iag has all free extents (newly allocated iag),
		 * add the iag to the ag free extent list.
		 */
		if (iagp->nfreeexts == EXTSPERIAG)
		{
			if (fwd >= 0)
				aiagp->extfreeback = iagno;

			iagp->extfreefwd = fwd;
			iagp->extfreeback = -1;
			imap->im_agctl[agno].extfree = iagno;
		}
	}

	/* if the iag has no free inodes, add the iag to the
	 * ag free inode list.
	 */
	if (iagp->nfreeinos == 0)
	{
		if (freei >= 0)
			ciagp->inofreeback = iagno;

		iagp->inofreefwd = imap->im_agctl[agno].inofree;
		iagp->inofreeback = -1;
		imap->im_agctl[agno].inofree = iagno;
	}

	/* initialize the extent descriptor of the extent. */
	PXDlength(&iagp->inoext[extno],imap->im_nbperiext);
	PXDaddress(&iagp->inoext[extno],blkno);

	/* initialize the working and persistent map of the extent.
	 * the working map will be initialized such that
	 * it indicates the first inode of the extent is allocated.
	 */
	iagp->wmap[extno] = HIGHORDER;
	iagp->pmap[extno] = 0;

	/* update the free inode and free extent summary maps
	 * for the extent to indicate the extent has free inodes
	 * and no longer represents a free extent.
	 */
	sword = extno >> L2EXTSPERSUM;
	mask = HIGHORDER >> (extno & (EXTSPERSUM-1));
	iagp->extsmap[sword] |= mask;
	iagp->inosmap[sword] &= ~mask;

	/* update the free inode and free extent counts for the
	 * iag.
	 */
	iagp->nfreeinos += (INOSPEREXT-1);
	iagp->nfreeexts -= 1;

	/* update the free and backed inode counts for the ag.
	 */
	imap->im_agctl[agno].numfree += (INOSPEREXT-1);
	imap->im_agctl[agno].numinos += INOSPEREXT;

	/* update the free and backed inode counts for the inode map.
	 */
	ATOMIC_ADD(imap->im_numfree, (INOSPEREXT-1));
	ATOMIC_ADD(imap->im_numinos, INOSPEREXT);

	/* write the iags.
	 */
	if (abp) bmLazyWrite(abp,0);
	if (bbp) bmLazyWrite(bbp,0);
	if (cbp) bmLazyWrite(cbp,0);

	return(0);

	error_out:

	/* release the iags.
	 */
	if (abp) bmRelease(abp);
	if (bbp) bmRelease(bbp);
	if (cbp) bmRelease(cbp);

	return(rc);
}


/*
 * NAME:        diNewIAG(imap,iagnop,agno)
 *
 * FUNCTION:   	allocate a new iag for an allocation group.
 *		
 *		first tries to allocate the iag from the inode map 
 *		iagfree list:  
 *		if the list has free iags, the head of the list is removed 
 *		and returned to satisfy the request.
 *		if the inode map's iag free list is empty, the inode map
 *		is extended to hold a new iag. this new iag is initialized
 *		and returned to satisfy the request.
 *
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      iagnop 	- pointer to an iag number set with the number of the
 *		  newly allocated iag upon successful return.
 *      agno  	- allocation group number.
 *	bpp	- Buffer pointer to be filled in with new IAG's buffer
 *
 * RETURN VALUES:
 *      0       - success.
 *      ENOSPC 	- insufficient disk resources.
 *      EIO  	- i/o error.
 *
 * serialization: 
 *	AG lock held on entry/exit;
 *	write lock on the map is held inside;
 *	read lock on the map is held on successful completion;
 *
 * note: new iag transaction: 
 * . synchronously write iag;
 * . write log of xtree and inode  of imap;
 * . commit;
 * . synchronous write of xtree (right to left, bottom to top);
 * . at start of logredo(): init in-memory imap with one additional iag page;
 * . at end of logredo(): re-read imap inode to determine
 *   new imap size;
 */
static int32
diNewIAG(imap_t	*imap,
	 int32	*iagnop,
	 int32	agno,
	 jbuf_t	**bpp)
{
	int32		rc;
	int32		iagno, i, xlen;
	inode_t 	*ipimap, *ipmnt;
	jbuf_t 		*bp;
	iag_t 		*iagp;
	int64		xaddr = 0;
	int64		blkno;
	xad_t		xad;
	int32		tid;
	inode_t		*iplist[1];

	/* pick up pointers to the inode map and mount inodes */
	ipimap = imap->im_ipimap;
	ipmnt = ipimap->i_ipmnt;

	/* acquire the free iag lock */
	IAGFREE_LOCK(imap);

	/* if there are any iags on the inode map free iag list, 
	 * allocate the iag from the head of the list.
	 */
	if (imap->im_freeiag >= 0)
	{
		/* pick up the iag number at the head of the list */
		iagno = imap->im_freeiag;

		/* determine the logical block number of the iag */
		blkno = IAGTOLBLK(iagno,ipmnt->i_l2nbperpage);
	}
	else
	{
		/* no free iags. the inode map will have to be extented
		 * to include a new iag.
		 */

		/* acquire inode map lock */
		IWRITE_LOCK(ipimap);

		assert(ipimap->i_size >> L2PSIZE == imap->im_nextiag+1);
	
		/* get the next avaliable iag number */
		iagno = imap->im_nextiag;
	
		/* make sure that we have not exceeded the maximum inode
		 * number limit.
		 */
		if (iagno > (MAXIAGS-1))
		{
			/* release the inode map lock */
			IWRITE_UNLOCK(ipimap);

			rc = ENOSPC;
			goto out;
		}

		/*
		 * synchronously append new iag page.
		 */
		/* determine the logical address of iag page to append */
		blkno = IAGTOLBLK(iagno, ipmnt->i_l2nbperpage);

		/* Allocate extent for new iag page */
		xlen = ipmnt->i_nbperpage;
		if (rc = dbAlloc(ipimap, 0, (int64)xlen, &xaddr))
		{
			/* release the inode map lock */
			IWRITE_UNLOCK(ipimap);

			goto out;
		}
	
		/* assign a buffer for the page */
	        bp = bmAssign(ipimap, blkno, xaddr, PSIZE, bmREAD_PAGE);
	        iagp = (iag_t *) bp->b_bdata;
	
		/* init the iag */
		bzero(iagp,sizeof(iag_t));
		iagp->iagnum = iagno;
		iagp->inofreefwd = iagp->inofreeback = -1;
	        iagp->extfreefwd = iagp->extfreeback = -1;
	        iagp->iagfree = -1;
	        iagp->nfreeinos = 0;
	        iagp->nfreeexts = EXTSPERIAG;
	
		/* initialize the free inode summary map (free extent
		 * summary map initialization handled by bzero).
		 */
		for (i = 0; i < SMAPSZ; i++)
	                iagp->inosmap[i] = ONES;
	
		/* synchronously write the iag page */
		if (bmWrite(bp))
		{
			/* Free the blocks allocated for the iag since it was
			 * not successfully added to the inode map
			 */
			dbFree(ipimap, xaddr, (int64)xlen);
	
			/* release the inode map lock */
			IWRITE_UNLOCK(ipimap);

			rc = EIO;
			goto out;
		}
	
		/* Now the iag is on disk */

		/*
		 * start tyransaction of update of the inode map
		 * addressing structure pointing to the new iag page;
		 */ 
		txBegin(ipmnt, &tid, COMMIT_FORCE);
	
		/* update the inode map addressing structure to point to it */
		if (rc = xtInsert(tid,ipimap,0,blkno,xlen,&xaddr,0))
		{
			/* Free the blocks allocated for the iag since it was
			 * not successfully added to the inode map
			 */
			dbFree(ipimap, xaddr, (int64)xlen);
	
			/* release the inode map lock */
			IWRITE_UNLOCK(ipimap);

			goto out;
		}
	
		/* update the inode map's inode to reflect the extension */
		ipimap->i_size += PSIZE;
		ipimap->i_nblocks += xlen;

		/*
		 * txCommit(COMMIT_FORCE) will synchronously write address 
		 * index pages and inode after commit in careful update order 
		 * of address index pages (right to left, bottom up);
		 */
		ipimap->i_flag |= IFSYNC;
		iplist[0] = ipimap;
		rc = txCommit(tid, 1, &iplist[0], COMMIT_FORCE);

		txEnd(tid);

		duplicateIXtree(ipmnt, blkno, xlen, &xaddr);

		/* update the next avaliable iag number */
		imap->im_nextiag += 1;
	
		/* Add the iag to the iag free list so we don't lose the iag
		 * if a failure happens now.
		 */
		imap->im_freeiag = iagno;

		/* release the inode map lock */
		IWRITE_UNLOCK(ipimap);
	}

	/* obtain read lock on map */
	IREAD_LOCK(ipimap);

	/* read the iag */
        if (rc = diIAGRead(imap, iagno, &bp))
	{
		IREAD_UNLOCK(ipimap);
		rc = EIO;
		goto out;
	}
        iagp = (iag_t *) bp->b_bdata;

	/* remove the iag from the iag free list */
	imap->im_freeiag = iagp->iagfree;
	iagp->iagfree = -1;

	/* set the return iag number and buffer pointer */
	*iagnop = iagno;
	*bpp = bp;

out:
	/* release the iag free lock */
	IAGFREE_UNLOCK(imap);

	return(rc);
}


/*
 * NAME:        diIAGRead()
 *
 * FUNCTION:    get the buffer for the specified iag within a fileset
 *		or aggregate inode map.
 *		
 * PARAMETERS:
 *      imap  	- pointer to inode map control structure.
 *      iagno  	- iag number.
 *      bpp  	- point to buffer pointer to be filled in on successful
 *		  exit.
 *
 * SERIALIZATION:
 *	must have read lock on imap inode
 *	(When called by diExtendFS, the filesystem is quiesced, therefore
 *	 the read lock is unnecessary.)
 *
 * RETURN VALUES:
 *      0       - success.
 *      EIO  	- i/o error.
 */
static int32
diIAGRead(imap_t	*imap,
	  int32		iagno,
	  jbuf_t	**bpp)
{
	int32 rc;
	int64 blkno;

	/* compute the logical block number of the iag. */
	blkno = IAGTOLBLK(iagno,imap->im_ipimap->i_ipmnt->i_l2nbperpage);

	/* read the iag. */
        if (rc = bmRead(imap->im_ipimap, blkno, PSIZE, bmREAD_PAGE, bpp))
	{
		*bpp = NULL;
		return(EIO);
	}

	return(0);
}

/*
 * NAME:        diFindFree()
 *
 * FUNCTION:    find the first free bit in a word starting at
 *		the specified bit position.
 *
 * PARAMETERS:
 *      word  	- word to be examined.
 *      start  	- starting bit position.
 *
 * RETURN VALUES:
 *      bit position of first free bit in the word or BITSPERINT32 if
 *	no free bits were found.
 */
static int32
diFindFree(uint32	word,
	   int32	start)
{
	int32 bitno;
	assert(start < BITSPERINT32);
	/* scan the word for the first free bit. */
	for (word <<= start, bitno = start; bitno < BITSPERINT32; 
						bitno++, word <<= 1)
	{
		if ((word & HIGHORDER) == 0)
			break;
	}
	return(bitno);
}

/*
 * NAME:	diUpdatePMap()
 *                                                                    
 * FUNCTION: Update the persistent map in an IAG for the allocation or 
 *	freeing of the specified inode.
 *                                                                    
 * PRE CONDITIONS: Working map has already been updated for allocate.
 *
 * PARAMETERS:
 *	ipimap	- Incore inode map inode
 *	inum	- Number of inode to mark in permanent map
 *	is_free	- If TRUE indicates inode should be marked freed, otherwise
 *		  indicates inode should be marked allocated.
 *
 * RETURNS: 0 for success
 */
int32
diUpdatePMap(
	inode_t		*ipimap,
	ino_t		inum,
	boolean_t	is_free,
	tblock_t	*tblk)
{
	int32 rc;
	iag_t *iagp;
	jbuf_t *bp;
	int32 iagno, ino, extno, bitno;
	imap_t *imap;
	uint32	mask;
	log_t	*log;
	logx_t	*logx;
	int32	ipri, lsn, difft, diffp;
	imap = ipimap->i_imap;
	/* get the iag number containing the inode */
	iagno = INOTOIAG(inum);
	/* make sure that the iag is contained within the map */
	assert(iagno < imap->im_nextiag);
	/* read the iag */
	IREAD_LOCK(ipimap);
        rc = diIAGRead(imap,iagno,&bp);
	IREAD_UNLOCK(ipimap);
        if (rc)
               	return(rc);
        iagp = (iag_t *) bp->b_bdata;
	/* get the inode number and extent number of the inode within
	 * the iag and the inode number within the extent.
	 */
	ino = inum & (INOSPERIAG-1);
	extno = ino >> L2INOSPEREXT;
	bitno = ino & (INOSPEREXT-1);
	mask = HIGHORDER >> bitno;
	/* 
	 * mark the inode free in persistent map:
	 */
	if (is_free == TRUE)
	{
		/* The inode should have been allocated both in working
		 * map and in persistent map;
		 * the inode will be freed from working map at the release
		 * of last reference release;
		 */
		assert(iagp->wmap[extno] & mask);
		assert(iagp->pmap[extno] & mask);
		/* update the bitmap for the extent of the freed inode */
		iagp->pmap[extno] = iagp->pmap[extno] & ~mask;
	}
	/*
	 * mark the inode allocated in persistent map:
	 */
	else
	{
		/* The inode should be already allocated in the working map
		 * and should be free in persistent map;
		 */
		assert(iagp->wmap[extno] & mask);
		assert((iagp->pmap[extno] & mask) == 0);
		/* update the bitmap for the extent of the allocated inode */
		iagp->pmap[extno] = iagp->pmap[extno] | mask;
	}
	/*
	 * update iag lsn
	 */
	lsn = tblk->lsn;
	log = (log_t *)tblk->ipmnt->i_iplog;
	logx = log->logx;
	if (bp->j_lsn != 0)
	{
		/* inherit older/smaller lsn */
		logdiff(difft, lsn, logx);
		logdiff(diffp, bp->j_lsn, logx);
		if (difft < diffp)
		{
			bp->j_lsn = lsn;
			/* move bp after tblock in logsync list */
			ipri = LOGSYNC_LOCK(logx);	
			CDLL_REMOVE(&logx->synclist, (logsyncblk_t *)bp, 
				    synclist);
			CDLL_INSERT_AFTER(&logx->synclist, (logsyncblk_t *)tblk, 
					  (logsyncblk_t *)bp, synclist);
			LOGSYNC_UNLOCK(ipri, logx);
		}
		/* inherit younger/larger clsn */
		assert(bp->j_clsn);
		logdiff(difft, tblk->clsn, logx);
		logdiff(diffp, bp->j_clsn, logx);
		if (difft > diffp)
			bp->j_clsn = tblk->clsn;
	}
	else
	{
		bp->j_logx = logx;
		bp->j_lsn = lsn;
		/* insert bp after tblock in logsync list */
		ipri = LOGSYNC_LOCK(logx);	
		logx->count++;
		CDLL_INSERT_AFTER(&logx->synclist, (logsyncblk_t *)tblk, 
				  (logsyncblk_t *)bp, synclist);
		LOGSYNC_UNLOCK(ipri, logx);
		bp->j_clsn = tblk->clsn;
	}
	bmLazyWrite(bp, logx->flag & JFS_COMMIT);
	return(0);
}


/*
 *	diExtendFS()
 *
 * function: update imap for extendfs();
 * 
 * note: AG size has been increased s.t. each k old contiguous AGs are 
 * coalesced into a new AG;
 */
int32 diExtendFS(
	inode_t	*ipimap,
	inode_t	*ipbmap)
{
	int32	rc, rcx = 0;
	imap_t	*imap = ipimap->i_imap;
	iag_t	*iagp, *hiagp;
	bmap_t	*mp = ipbmap->i_bmap;
	jbuf_t	*bp, *hbp;
	int32	i, n, head;
	int32	numinos, xnuminos = 0, xnumfree = 0;
	int64	agstart;

jEVENT(0,("diExtendFS: nextiag:%d numinos:%d numfree:%d\n", 
	imap->im_nextiag, imap->im_numinos, imap->im_numfree));
 
	/*
	 *	reconstruct imap 
	 *
	 * coalesce contiguous k (newAGSize/oldAGSize) AGs;
	 * i.e., (AGi, ..., AGj) where i = k*n and j = k*(n+1) - 1 to AGn;
	 * note: new AG size = old AG size * (2**x).
	 */
#ifdef	_JFS_FYI
	/*
	 *	inode map control page
	 */
	int32		in_freeiag;	 /* 4: free iag list anchor	*/
	int32		in_nextiag;	 /* 4: next free iag number	*/
	int32		in_numinos;	 /* 4: num of backed inodes */
	int32		in_numfree;	 /* 4: num of free backed inodes */
	/* in_nbperiext: num of blocks per inode extent */
	/* in_l2nbperiext: l2 of in_nbperiext */
	/* in_diskblock: for standalone test driver  */
	/* in_maxag: for standalone test driver  */
	/* pad[2016]: pad to 2048 */
	iagctl_t {	/* per AG control information */
		int32	inofree;	/* 4: free inode list anchor */
		int32	extfree;	/* 4: free extent list anchor */
		int32	numinos;	/* 4: number of backed inodes */
		int32	numfree;	/* 4: number of free backed inodes */
	} in_agctl[MAXAG]; /* 2048: AG control information */
#endif	/* _JFS_FYI */

	/* init per AG control information im_agctl[] */
	for (i = 0; i < MAXAG; i++)
	{
		imap->im_agctl[i].inofree = -1; /* free inode list */
		imap->im_agctl[i].extfree = -1; /* free extent list */
		imap->im_agctl[i].numinos = 0; /* number of backed inodes */
		imap->im_agctl[i].numfree = 0; /* number of free backed inodes */
	}

	/*
	 *	process each iag_t page of the map.
	 *
	 * rebuild AG Free Inode List, AG Free Inode Extent List;
	 */
	for (i = 0; i < imap->im_nextiag; i++)
	{
        	if (rc = diIAGRead(imap, i, &bp))
		{
			rcx = rc;
			continue;
		}
        	iagp = (iag_t *)bp->b_bdata;
		assert(iagp->iagnum == i);

		/* leave free iag in the free iag list */
		if (iagp->nfreeexts == EXTSPERIAG)
		{
			bmRelease(bp);
			continue;
		}

		/* agstart that computes to the same ag is treated as same; */
		agstart = iagp->agstart;
		/* iagp->agstart = agstart & ~(mp->db_agsize - 1); */
		n = iagp->agstart >> mp->db_agl2size;
/*
printf("diExtendFS: iag:%d agstart:%lld agno:%d\n", i, agstart, n);
*/

		/* compute backed inodes */
		numinos = (EXTSPERIAG - iagp->nfreeexts) << L2INOSPEREXT;
		if (numinos > 0)
		{
			/* merge AG backed inodes */
			imap->im_agctl[n].numinos += numinos;
			xnuminos += numinos;
		}

		/* if any backed free inodes, insert at AG free inode list */
		if (iagp->nfreeinos > 0)
		{	
			if ((head = imap->im_agctl[n].inofree) == -1)
				iagp->inofreefwd = iagp->inofreeback = -1;
			else
			{
        			if (rc = diIAGRead(imap, head, &hbp))
				{
					rcx = rc;
					goto nextiag;
				}
        			hiagp = (iag_t *)hbp->b_bdata;
				hiagp->inofreeback = iagp->iagnum;
				iagp->inofreefwd = head;
				iagp->inofreeback = -1;
				bmLazyWrite(hbp, 0);
			}

			imap->im_agctl[n].inofree = iagp->iagnum;

			/* merge AG backed free inodes */
			imap->im_agctl[n].numfree += iagp->nfreeinos;
			xnumfree += iagp->nfreeinos;
		}

		/* if any free extents, insert at AG free extent list */
		if (iagp->nfreeexts > 0)
		{
			if ((head = imap->im_agctl[n].extfree) == -1)
				iagp->extfreefwd = iagp->extfreeback = -1;
			else
			{
        			if (rc = diIAGRead(imap, head, &hbp))
				{
					rcx = rc;
					goto nextiag;
				}
        			hiagp = (iag_t *)hbp->b_bdata;
				hiagp->extfreeback = iagp->iagnum;
				iagp->extfreefwd = head;
				iagp->extfreeback = -1;
				bmLazyWrite(hbp, 0);
			}

			imap->im_agctl[n].extfree = iagp->iagnum;
		}

nextiag:
		bmLazyWrite(bp, 0);
	}

	ASSERT(xnuminos == imap->im_numinos && xnumfree == imap->im_numfree);

	return rcx;
}


/*
 *	duplicateIXtree()
 *
 * serialization: IWRITE_LOCK held on entry/exit
 *
 * note: shadow page with regular inode (rel.2);
 */
static void
duplicateIXtree(
	inode_t	*ipimap,
	int64	blkno,
	int32	xlen,
	int64	*xaddr)
{
	int32	rc;
	int32	tid;
	inode_t	*iplist[1];
	inode_t	*ip;
	cbuf_t	*bp;
	cbuf_t	*bpsuper;
	struct superblock *sb;
	inode_t	*ipmnt = ipimap->i_ipmnt;
	union mntvfs	dummyvfs;

	/* if AIT2 ipmap2 is bad, do not try to update it */
	if (ipmnt->i_mntflag & JFS_BAD_SAIT) /* s_flag */
		return;
	/* allocate/initialize a dummy vfs with ipimap2:
	 * fake inumber in ipimap2 to avoid collision with real ipimap inode
	 */ 
	memset(&dummyvfs, 0, sizeof(struct vfs));
	dummyvfs.filesetvfs.vfs_data = (caddr_t)ipmnt->i_ipaimap2;
	dummyvfs.dummyvfs.dev = ipimap->i_dev;
	ICACHE_LOCK();
	rc = iget((struct vfs *)&dummyvfs, FILESYSTEM_I, &ip, 0);
	ICACHE_UNLOCK();
	if (rc)
	{
		ipmnt->i_mntflag |= JFS_BAD_SAIT;
		if (rc = readSuper(ipmnt, &bpsuper))
			return;
		sb = (struct superblock *)(bpsuper->b_bdata);
		sb->s_flag |= JFS_BAD_SAIT;
		rawWrite(ipmnt, bpsuper, 1);
		return;
	}
	
	/* start transaction */
	txBegin(ip->i_ipmnt, &tid, COMMIT_FORCE);
	/* update the inode map addressing structure to point to it */
	iplist[0] = ip;
	if (rc = xtInsert(tid,ip,0,blkno,xlen,xaddr,0))
	{
		ipmnt->i_mntflag |= JFS_BAD_SAIT;
		txAbort(tid, 1);
		goto cleanup;

	}
	/* update the inode map's inode to reflect the extension */
	ip->i_size += PSIZE;
	ip->i_nblocks += xlen;
	ip->i_flag |= IFSYNC;
	rc = txCommit(tid, 1, &iplist[0], COMMIT_FORCE);	// D233382
cleanup:
	txEnd(tid);
	/* aysncronously write and invalidate pages */
	if (ip->i_cacheid)
	{
		bmInodeFlush(ip);
		iUnbindCache(ip);
	}
	/* free inode */
	ICACHE_LOCK();
	iunhash(ip);
	ICACHE_UNLOCK();
}


#ifdef	_JFS_DEBUG_IMAP
/*
 *	DBGdiInit()
 */
void *DBGdiInit(imap_t *imap)
{
	uint32	*dimap;
	int32	size;
	size = 64 * 1024;
        if ((dimap = (uint32 *)xmalloc(size, L2PSIZE, kernel_heap)) == NULL)
		assert(0);
	bzero((void *)dimap, size);
	imap->im_DBGdimap = dimap;
}

/*
 *	DBGdiAlloc()
 */
void DBGdiAlloc(imap_t *imap, ino_t ino)
{
	uint32	*dimap = imap->im_DBGdimap;
	int32	w, b;
	uint32	m;
	w = ino >> 5;
	b = ino & 31;
	m = 0x80000000 >> b;
	assert(w < 64*256);
	if (dimap[w] & m)
	{
printf("DEBUG diAlloc: duplicate alloc ino:0x%x\n", ino);
		brkpoint();
	}
	dimap[w] |= m;
}

/*
 *	DBGdiFree()
 */
void DBGdiFree(imap_t *imap, ino_t ino)
{
	uint32	*dimap = imap->im_DBGdimap;
	int32	w, b;
	uint32	m;
	w = ino >> 5;
	b = ino & 31;
	m = 0x80000000 >> b;
	assert(w < 64*256);
	if ((dimap[w] & m) == 0)
{
printf("DEBUG diFree: duplicate free ino:0x%x\n", ino);
		brkpoint();
}
	dimap[w] &= ~m;
}

void dump_cp(imap_t	*ipimap,
	     char	*function,
	     int	line)
{
	printf("\n* ********* *\nControl Page %s %d\n", function, line );
	printf("FreeIAG %d\tNextIAG %d\n", ipimap->im_freeiag,
		ipimap->im_nextiag);
	printf("NumInos %d\tNumFree %d\n", ipimap->im_numinos,
		ipimap->im_numfree );
	printf("AG InoFree %d\tAG ExtFree %d\n", ipimap->im_agctl[0].inofree,
		ipimap->im_agctl[0].extfree );
	printf("AG NumInos %d\tAG NumFree %d\n", ipimap->im_agctl[0].numinos,
		ipimap->im_agctl[0].numfree );
}

void dump_iag(iag_t	*iag,
	      char	*function,
	      int	line)
{
	printf("\n* ********* *\nIAG %s %d\n", function, line );
	printf("IagNum %d\tIAG Free %d\n", iag->iagnum, iag->iagfree );
	printf("InoFreeFwd %d\tInoFreeBack %d\n", iag->inofreefwd,
		iag->inofreeback );
	printf("ExtFreeFwd %d\tExtFreeBack %d\n", iag->extfreefwd,
		iag->extfreeback );
	printf("NFreeInos %d\tNFreeExts %d\n", iag->nfreeinos,
		iag->nfreeexts );
}
#endif	/* _JFS_DEBUG_IMAP */
