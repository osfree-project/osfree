/* $Id: jfs_dmap.c,v 1.1 2000/04/21 10:58:06 ktk Exp $ */

static char *SCCSID = "@(#)1.25  6/15/99 17:33:20 src/jfs/ifs/jfs_dmap.c, sysjfs, w45.fs32, fixbld";
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
 *   MODULE_NAME:		jfs_dmap.c
 *
 *   COMPONENT_NAME: 	sysjfs
 *
 *   FUNCTION:			block allocation map manager
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
#endif	/* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_xtree.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_bufmgr.h"
#include "jfs_txnmgr.h"
#include "jfs_logmgr.h"
#include "jfs_dasdlim.h"					// F226941
#include "jfs_debug.h"

/*
 *	Debug code for double-checking block map
 */
/* #define	_JFS_DEBUG_DMAP	1 */

#ifdef	_JFS_DEBUG_DMAP
#define DBINITMAP(size,ipbmap,results) \
	DBinitmap(size,ipbmap,results)
#define DBALLOC(dbmap,mapsize,blkno,nblocks) \
	DBAlloc(dbmap,mapsize,blkno,nblocks)
#define DBFREE(dbmap,mapsize,blkno,nblocks) \
	DBFree(dbmap,mapsize,blkno,nblocks)
#define DBALLOCCK(dbmap,mapsize,blkno,nblocks) \
	DBAllocCK(dbmap,mapsize,blkno,nblocks)
#define DBFREECK(dbmap,mapsize,blkno,nblocks) \
	DBFreeCK(dbmap,mapsize,blkno,nblocks)

void DBinitmap(int64,inode_t *,uint32 **);
void DBAlloc(uint *,int64,int64,int64);
void DBFree(uint *,int64,int64,int64);
void DBAllocCK(uint *,int64,int64,int64);
void DBFreeCK(uint *,int64,int64,int64);
#else
#define DBINITMAP(size,ipbmap,results)
#define DBALLOC(dbmap, mapsize, blkno, nblocks)
#define DBFREE(dbmap, mapsize, blkno, nblocks)
#define DBALLOCCK(dbmap, mapsize, blkno, nblocks)
#define DBFREECK(dbmap, mapsize, blkno, nblocks)
#endif	/* _JFS_DEBUG_DMAP */

/*
 *	SERIALIZATION of the Block Allocation Map.
 *
 *	the working state of the block allocation map is accessed in
 *	two directions:
 *	
 *	1) allocation and free requests that start at the dmap
 *	   level and move up through the dmap control pages (i.e.
 *	   the vast majority of requests).
 * 
 * 	2) allocation requests that start at dmap control page
 *	   level and work down towards the dmaps.
 *	
 *	the serialization scheme used here is as follows. 
 *
 *	requests which start at the bottom are serialized against each 
 *	other through buffers and each requests holds onto its buffers 
 *	as it works it way up from a single dmap to the required level 
 *	of dmap control page.
 *	requests that start at the top are serialized against each other
 *	and request that start from the bottom by the multiple read/single
 *	write inode lock of the bmap inode. requests starting at the top
 *	take this lock in write mode while request starting at the bottom
 *	take the lock in read mode.  a single top-down request may proceed
 *	exclusively while multiple bottoms-up requests may proceed 
 * 	simultaneously (under the protection of busy buffers).
 *	
 *	in addition to information found in dmaps and dmap control pages,
 *	the working state of the block allocation map also includes read/
 *	write information maintained in the bmap descriptor (i.e. total
 *	free block count, allocation group level free block counts).
 *	a single exclusive lock (BMAP_LOCK) is used to guard this information
 *	in the face of multiple-bottoms up requests.
 *	(lock ordering: IREAD_LOCK, BMAP_LOCK);
 *	
 *	accesses to the persistent state of the block allocation map (limited
 *	to the persistent bitmaps in dmaps) is guarded by (busy) buffers.
 */
#define	BMAP_LOCK_ALLOC(mp)\
	MUTEXLOCK_ALLOC(&mp->db_bmaplock,LOCK_ALLOC_PAGED,JFS_BMAP_LOCK_CLASS,0)
#define	BMAP_LOCK_FREE(mp)	MUTEXLOCK_FREE(&mp->db_bmaplock)
#define	BMAP_LOCK_INIT(mp)	MUTEXLOCK_INIT(&mp->db_bmaplock)
#define BMAP_LOCK(mp)		MUTEXLOCK_LOCK(&mp->db_bmaplock)
#define BMAP_UNLOCK(mp)		MUTEXLOCK_UNLOCK(&mp->db_bmaplock)

/*
 * forward references
 */
static void dbAllocBits(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks);
static void dbSplit(dmtree_t *tp, int32 leafno, int8 splitsz, int8 newval);
static void dbBackSplit(dmtree_t *tp, int32 leafno);
static void dbJoin(dmtree_t *tp, int32 leafno, int8 newval);
static void dbAdjTree(dmtree_t *tp, int32 leafno, int8 newval);
static int32 dbAdjCtl(bmap_t *mp, int64 blkno, int8 newval, int32 alloc,
		      int32 level);
static int32 dbAllocAny(bmap_t *mp, int64 nblocks, int8 l2nb, int64 *results);
static int32 dbAllocNext(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks);
static int32 dbAllocNear(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks,
			 int8 l2nb, int64 *results);
static int32 dbAllocDmap(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks);
static int32 dbAllocDmapLev(bmap_t *mp, dmap_t *dp, int32 nblocks, int8 l2nb,
			    int64 *results);
static int32 dbAllocAG(bmap_t *mp, int32 agno, int64 nblocks, int8 l2nb,
		       int64 *results);
static int32 dbAllocCtl(bmap_t *mp, int64 nblocks, int8 l2nb, int64 blkno,
			int64 *results);
int32 dbExtend(inode_t *ip, int64 blkno, int64 nblocks, int64 addnblocks);
static int32 dbFindBits(uint32 word, int32 l2nb);
static int32 dbFindCtl(bmap_t *mp, int8 l2nb, int32 level, int64 *blkno);
static int32 dbFindLeaf(dmtree_t *tp, int8 l2nb, int32 *leafidx);
static void dbFreeBits(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks);
static int32 dbFreeDmap(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks);
static int32 dbMaxBud(char *cp);
int32 blkstol2(int64 nb);
void fsDirty(void);

int32 cntlz(uint32 value);
int32 cnttz(uint32 word);

static int32 dbAllocDmapBU(bmap_t *mp, dmap_t *dp, int64 blkno, int32 nblocks);
static int8 dbInitDmap(dmap_t *dp, int64 blkno, int32 nblocks);
static int8 dbInitDmapTree(dmap_t *dp);
static int8 dbInitTree(dmaptree_t *dtp);
static int32 dbInitDmapCtl(dmapctl_t *dcp, int32 level, int32 i);
static int32 dbGetL2AGSize(int64 nblocks);
static int64 dbMapFileSizeToMapSize(inode_t *ipbmap);

/*
 *	buddy table
 *
 * table used for determining buddy sizes within characters of 
 * dmap bitmap words.  the characters themselves serve as indexes
 * into the table, with the table elements yielding the maximum
 * binary buddy of free bits within the character.
 */
signed char budtab[256] = {
	3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1};


/*
 * NAME:    	dbMount()
 *
 * FUNCTION:	initializate the block allocation map.
 *
 *		memory is allocated for the in-core bmap descriptor and
 *		the in-core descriptor is initialized from disk.
 *
 * PARAMETERS:
 *      ipbmap	-  pointer to in-core inode for the block map.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOMEM	- insufficient memory
 *      EIO	- i/o error
 */
int32
dbMount(inode_t	*ipbmap)
{
        bmap_t *mp;
        jbuf_t *bp;

	/*
	 * allocate/initialize the in-memory bmap descriptor
	 */
	/* allocate memory for the in-memory bmap descriptor */
        if ((mp = (bmap_t *)xmalloc(sizeof(bmap_t), L2PSIZE,
                                        kernel_heap)) == NULL)
                return(ENOMEM);

	/* read the on-disk bmap descriptor. */
        if (bmRead(ipbmap, BMAPBLKNO, PSIZE, bmREAD_PAGE, &bp))
        {
                xmfree((void *)mp,kernel_heap);
                return(EIO);
        }

	/* copy the on-disk bmap descriptor to its in-memory version. */
        bcopy(bp->b_bdata,&mp->db_bmap,sizeof(dbmap_t));

        /* release the buffer. */
        bmRelease(bp);

	/* bind the bmap inode and the bmap descriptor to each other. */
        mp->db_ipbmap = ipbmap;
        ipbmap->i_bmap = mp;

	DBINITMAP(mp->db_mapsize,ipbmap,&mp->db_DBmap);

	/*
	 * allocate/initialize the bmap lock
	 */
	BMAP_LOCK_ALLOC(mp);
	BMAP_LOCK_INIT(mp);

        return(0);
}


/*
 * NAME:    	dbUnmount()
 *
 * FUNCTION:	terminate the block allocation map in preparation for
 *		file system unmount.
 *
 * 		the in-core bmap descriptor is written to disk and
 *		the memory for this descriptor is freed.
 *
 * PARAMETERS:
 *      ipbmap	-  pointer to in-core inode for the block map.
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 */
int32
dbUnmount(inode_t	*ipbmap,
	uint32	mounterror)
{
        jbuf_t *bp;
        bmap_t *mp = ipbmap->i_bmap;

	if (mounterror || isReadOnly(ipbmap))
		goto release;

	/* get the buffer for the on-disk bmap descriptor. */
        if (bmRead(ipbmap, BMAPBLKNO, PSIZE, bmREAD_PAGE, &bp))
                return(EIO);

        /* copy the in-memory version of the bmap to the on-disk version. */
        bcopy(&mp->db_bmap,bp->b_bdata,sizeof(dbmap_t));

        /* synchronous write the buffer. */
        if (bmWrite(bp))
		return(EIO);

release: /* release resources */

	BMAP_LOCK_FREE(mp);

	/* free the memory for the in-memory bmap. */
        xmfree(mp, kernel_heap);

        return(0);
}


/*
 *	dbSync()
 */
int32 dbSync(inode_t	*ipbmap)
{
        bmap_t *mp = ipbmap->i_bmap;
        jbuf_t *bp;

	/*
	 * write bmap global control page
	 */
	/* get the buffer for the on-disk bmap descriptor. */
        if (bmRead(ipbmap, BMAPBLKNO, PSIZE, bmREAD_PAGE, &bp))
                return(EIO);

        /* copy the in-memory version of the bmap to the on-disk version */
        bcopy(&mp->db_bmap,bp->b_bdata,sizeof(dbmap_t));

        /* write the buffer */
        bmLazyWrite(bp, 0);

	/*
	 * write out dirty pages of bmap
	 */
	bmInodeWrite(ipbmap);

        return(0);
}


/*
 * NAME:    	dbFree()
 *
 * FUNCTION:	free the specified block range from the working block
 *		allocation map.
 *
 *		the blocks will be free from the working map one dmap
 *		at a time.
 *
 * PARAMETERS:
 *      ip	-  pointer to in-core inode;
 *      blkno	-  starting block number to be freed.
 *      nblocks	-  number of blocks to be freed.
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 */
int32
dbFree(inode_t	*ip,
	int64	blkno,
	int64	nblocks)
{
	jbuf_t *bp;
	dmap_t *dp;
	int32 nb, rc;
	int64	lblkno, rem;
	inode_t	*ipbmap = ip->i_ipmnt->i_ipbmap;
	bmap_t *mp = ipbmap->i_bmap;

	IREAD_LOCK(ipbmap);

	/* block to be freed better be within the mapsize. */
	assert(blkno + nblocks <= mp->db_mapsize);

	/*
	 * free the blocks a dmap at a time.
	 */
	bp = NULL;
	for (rem = nblocks; rem > 0; rem -= nb, blkno += nb)
	{
		/* release previous dmap if any */
		if (bp)
			bmLazyWrite(bp, 0);

		/* get the buffer for the current dmap. */
        	lblkno = BLKTODMAP(blkno,mp->db_l2nbperpage);
        	if (bmRead(ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		{
			IREAD_UNLOCK(ipbmap);
                	return(EIO);
		}
		dp = (dmap_t *) bp->b_bdata;

		/* determine the number of blocks to be freed from
		 * this dmap.
		 */
		nb = MIN(rem, BPERDMAP - (blkno & (BPERDMAP-1)));

		DBALLOCCK(mp->db_DBmap, mp->db_mapsize, blkno, nb);

		/* free the blocks. */
        	if (rc = dbFreeDmap(mp,dp,blkno,nb))
		{
			bmRelease(bp);
			IREAD_UNLOCK(ipbmap);
			return(rc);
		}

		DBFREE(mp->db_DBmap, mp->db_mapsize, blkno, nb);
	}

	/* write the last buffer. */
	bmLazyWrite(bp, 0);

	IREAD_UNLOCK(ipbmap);

	return(0);
}


/*
 * NAME:	dbUpdatePMap()
 *
 * FUNCTION:    update the allocation state (free or allocate) of the
 *		specified block range in the persistent block allocation map.
 *		
 *		the blocks will be updated in the persistent map one
 *		dmap at a time.
 *
 * PARAMETERS:
 *      ipbmap	-  pointer to in-core inode for the block map.
 *      free	- TRUE if block range is to be freed from the persistent
 *		  map; FALSE if it is to   be allocated.
 *      blkno	-  starting block number of the range.
 *      nblocks	-  number of contiguous blocks in the range.
 *      tblk	-  transaction block;
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 */
int32
dbUpdatePMap(inode_t	*ipbmap,
	int32		free,
	int64		blkno,
	int64		nblocks,
	tblock_t	*tblk)
{
	int32 nblks, dbitno, wbitno, rbits, word, nbits, nwords;
	bmap_t *mp = ipbmap->i_bmap;
	int64	lblkno, rem, lastlblkno;
	uint32 mask;
	dmap_t *dp;
	jbuf_t *bp;
	log_t	*log;
	logx_t	*logx;
	int32	ipri, lsn, difft, diffp;

	/* the blocks better be within the mapsize. */
	assert(blkno + nblocks <= mp->db_mapsize);

	/* compute delta of transaction lsn from log syncpt */
	lsn = tblk->lsn;
	log = (log_t *)tblk->ipmnt->i_iplog;
	logx = log->logx;
	logdiff(difft, lsn, logx);

	/*
	 * update the block state a dmap at a time.
	 */
	bp = NULL;
	lastlblkno = 0;
	for (rem = nblocks; rem > 0; rem -= nblks, blkno += nblks)
	{
		/* get the buffer for the current dmap. */
        	lblkno = BLKTODMAP(blkno,mp->db_l2nbperpage);
		if (lblkno != lastlblkno)
		{
			if (bp)
				bmLazyWrite(bp, 0);

        		if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
                		return(EIO);
		}
		dp = (dmap_t *) bp->b_bdata;

		/* determine the bit number and word within the dmap of
		 * the starting block.  also determine how many blocks
		 * are to be updated within this dmap.
		 */
		dbitno = blkno & (BPERDMAP - 1);
		word = dbitno >> L2DBWORD;
		nblks = MIN(rem, BPERDMAP - dbitno);

		/* update the bits of the dmap words. the first and last
		 * words may only have a subset of their bits updated. if
		 * this is the case, we'll work against that word (i.e.
		 * partial first and/or last) only in a single pass.  a 
		 * single pass will also be used to update all words that
		 * are to have all their bits updated.
		 */
		for (rbits = nblks; rbits > 0; rbits -= nbits, dbitno += nbits)
		{
			/* determine the bit number within the word and
			 * the number of bits within the word.
			 */
			wbitno = dbitno & (DBWORD-1);
			nbits = MIN(rbits, DBWORD - wbitno);

			/* check if only part of the word is to be updated. */
			if (nbits < DBWORD)
			{
				/* update (free or allocate) the bits
				 * in this word.
				 */
				mask = (ONES << (DBWORD - nbits) >> wbitno);
				if (free)
					dp->pmap[word] &= ~mask;
				else
					dp->pmap[word] |= mask;
				
				word += 1;
			}
			else
			{
				/* one or more words are to have all
				 * their bits updated.  determine how
				 * many words and how many bits.
				 */
				nwords = rbits >> L2DBWORD;
				nbits = nwords << L2DBWORD;

				/* update (free or allocate) the bits
				 * in these words.
				 */
				if (free)
					memset(&dp->pmap[word], 0, nwords*4);
				else
					memset(&dp->pmap[word], ONES, nwords*4);

				word += nwords;
			}
		}

		/*
		 * update dmap lsn
		 */
		if (lblkno == lastlblkno)
			continue;

		lastlblkno = lblkno;
	
		if (bp->j_lsn != 0)
		{
			/* inherit older/smaller lsn */
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
	}

	/* write the last buffer. */
	if (bp)
		bmLazyWrite(bp, logx->flag & JFS_COMMIT);

	return(0);
}


/*
 * NAME:	dbNextAG()
 *
 * FUNCTION:    find the preferred allocation group for new allocations.
 *
 *		we try to keep the trailing (rightmost) allocation groups
 *		free for large allocations.  we try to do this by targeting
 *		new inode allocations towards the leftmost or 'active'
 *		allocation groups while keeping the rightmost or 'inactive'
 *		allocation groups free. once the active allocation groups
 *		have dropped to a certain percentage of free space, we add
 *		the leftmost inactive allocation group to the active set.
 *
 *		within the active allocation groups, we maintain a preferred
 *		allocation group which consists of a group with at least
 *		average free space over the active set. it is the preferred
 *		group that we target new inode allocation towards.  the 
 *		tie-in between inode allocation and block allocation occurs
 *		as we allocate the first (data) block of an inode and specify
 *		the inode (block) as the allocation hint for this block.
 *
 * PARAMETERS:
 *      ipbmap	-  pointer to in-core inode for the block map.
 *
 * RETURN VALUES:
 *      the preferred allocation group number.
 *
 * note: only called by dbAlloc();
 */
int32
dbNextAG(inode_t *ipbmap)
{
	int64 avgfree, inactfree, actfree, rem;
	int32 actags, inactags, l2agsize;
	bmap_t *mp = ipbmap->i_bmap;

	BMAP_LOCK(mp);

	/* determine the number of active allocation groups (i.e.
	 * the number of allocation groups up to and including
	 * the rightmost allocation group with blocks allocated
	 * in it.
	 */
	actags = mp->db_maxag+1;
	assert(actags <= mp->db_numag);

	/* get the number of inactive allocation groups (i.e. the
	 * number of allocation group following the rightmost group
	 * with allocation in it.
	 */
	inactags = mp->db_numag - actags;

	/* determine how many blocks are in the inactive allocation
	 * groups. in doing this, we must account for the fact that
	 * the rightmost group might be a partial group (i.e. file
	 * system size is not a multiple of the group size).
	 */
	l2agsize = mp->db_agl2size;
        rem = mp->db_mapsize & (mp->db_agsize-1);
	inactfree = (inactags && rem) ? ((inactags-1) << l2agsize) + rem :
						 inactags << l2agsize;

	/* now determine how many free blocks are in the active
	 * allocation groups plus the average number of free blocks
	 * within the active ags.
	 */
	actfree = mp->db_nfree - inactfree;
	avgfree = actfree / actags;

	/* check if not all of the allocation groups are active.
	 */
	if (actags < mp->db_numag)
	{
		/* not all of the allocation groups are active.  determine
		 * if we should extend the active set by 1 (i.e. add the
		 * group following the current active set).  we do so if
		 * the number of free blocks within the active set is less
		 * than the allocation group set and average free within
		 * the active set is less than 60%.  we activate a new group
		 * by setting the allocation group preference to the new
		 * group.
		 */
 		if (actfree < mp->db_agsize &&
		    ((avgfree * 100) >> l2agsize) < 60)
			mp->db_agpref = actags;
	}
	else
	{
		/* all allocation groups are in the active set.  check if
		 * the preferred allocation group has average free space.
		 * if not, re-establish the preferred group as the leftmost
		 * group with average free space.
		 */
		if (mp->db_agfree[mp->db_agpref] < avgfree)
		{
			for (mp->db_agpref = 0; mp->db_agpref < actags;
							 mp->db_agpref++)
			{
				if (mp->db_agfree[mp->db_agpref] <= avgfree)
					break;
			}
			assert( mp->db_agpref < mp->db_numag );
		}
	}

	BMAP_UNLOCK(mp);

	/* return the preferred group.
	 */
	return(mp->db_agpref);
}


/*
 * NAME:	dbAlloc()
 *
 * FUNCTION:    attempt to allocate a specified number of contiguous free
 *		blocks from the working allocation block map.
 *
 *		the block allocation policy uses hints and a multi-step
 *		approach.
 *
 *	  	for allocation requests smaller than the number of blocks
 *		per dmap, we first try to allocate the new blocks
 *		immediately following the hint.  if these blocks are not
 *		available, we try to allocate blocks near the hint.  if
 *		no blocks near the hint are available, we next try to 
 *		allocate within the same dmap as contains the hint.
 *
 *		if no blocks are available in the dmap or the allocation
 *		request is larger than the dmap size, we try to allocate
 *		within the same allocation group as contains the hint. if
 *		this does not succeed, we finally try to allocate anywhere
 *		within the aggregate.
 *
 *		we also try to allocate anywhere within the aggregate for
 *		for allocation requests larger than the allocation group
 *		size or requests that specify no hint value.
 *
 * PARAMETERS:
 *      ip	-  pointer to in-core inode;
 *      hint	- allocation hint.
 *      nblocks	- number of contiguous blocks in the range.
 *      results	- on successful return, set to the starting block number
 *		  of the newly allocated contiguous range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 */
int32
dbAlloc(inode_t *ip,
        int64 hint,
        int64 nblocks,
        int64 *results)
{
	int32 rc, agno;
	inode_t *ipbmap = ip->i_ipmnt->i_ipbmap;
	bmap_t *mp;
	jbuf_t *bp;
	int64 lblkno, blkno;
	dmap_t *dp;
	int8 l2nb;
	int64	mapSize;

	/* assert that nblocks is valid */
	assert(nblocks > 0);

	/* DASD limit check 					F226941 */
	if (OVER_LIMIT(ip, nblocks))
		return ENOSPC;

	/* get the log2 number of blocks to be allocated.
	 * if the number of blocks is not a log2 multiple, 
	 * it will be rounded up to the next log2 multiple.
	 */
	l2nb = BLKSTOL2(nblocks);

	mp = ipbmap->i_bmap;

retry:	/* serialize w.r.t.extendfs() */
	mapSize = mp->db_mapsize;

	/* the hint should be within the map */
	assert(hint < mapSize);

	/* if no hint was specified or the number of blocks to be
	 * allocated is greater than the allocation group size, try
	 * to allocate anywhere.
	 */
	if (hint == 0 || l2nb > mp->db_agl2size)
	{
		IWRITE_LOCK(ipbmap);

		rc = dbAllocAny(mp,nblocks,l2nb,results);
		if (rc == 0)
		{
			DBALLOC(mp->db_DBmap, mp->db_mapsize, *results,
				nblocks);
		}

		IWRITE_UNLOCK(ipbmap);
		return(rc);
	}

	/* we would like to allocate close to the hint.  adjust the
	 * hint to the block following the hint since the allocators
	 * will start looking for free space starting at this point.
	 * if the hint was the last block of the file system, try to
	 * allocate in the same allocation group as the hint.
	 */
	blkno = hint + 1;
	if (blkno >= mp->db_mapsize)
	{
		blkno--;
		goto tryag;
	}

	/* check if blkno crosses over into a new allocation group.
	 * if so, check if we should allow allocations within this
	 * allocation group.  we try to keep the trailing (rightmost)
	 * allocation groups of the file system free for large
	 * allocations and may want to prevent this allocation from
	 * spilling over into this space.
	 */
	if ((blkno & (mp->db_agsize-1)) == 0)
	{
		/* check if the AG is beyond the rightmost AG with
		 * allocations in it.  if so, call dbNextAG() to
		 * determine if the allocation should be allowed
		 * to proceed within this AG or should be targeted
		 * to another AG.
		 */
		agno = blkno >> mp->db_agl2size;
		if (agno > mp->db_maxag)
		{
			agno = dbNextAG(ipbmap);
			blkno = (int64)agno << mp->db_agl2size;
			goto tryag;
		}
	}

	/* check if the allocation request size can be satisfied from a
	 * single dmap.  if so, try to allocate from the dmap containing
	 * the hint using a tiered strategy.
	 */
	if (nblocks <= BPERDMAP)
	{
		IREAD_LOCK(ipbmap);

		/* get the buffer for the dmap containing the hint.
		 */
        	lblkno = BLKTODMAP(blkno,mp->db_l2nbperpage);
        	if (bmRead(ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		{
			IREAD_UNLOCK(ipbmap);
                	return(EIO);
		}
		dp = (dmap_t *) bp->b_bdata;
	
		/* first, try to satisfy the allocation request with the
		 * blocks beginning at the hint.
		 */
		if ((rc = dbAllocNext(mp,dp,blkno,(int32)nblocks)) != ENOSPC)
		{
			if (rc == 0)
			{
				*results = blkno;
				DBALLOC(mp->db_DBmap, mp->db_mapsize, *results,
					nblocks);
				bmLazyWrite(bp,0);
			}
			else
			{
				assert(rc == EIO);
				bmRelease(bp);
			}

			IREAD_UNLOCK(ipbmap);
			return(rc);
		}

		/* next, try to satisfy the allocation request with blocks
		 * near the hint.
		 */
		if ((rc = dbAllocNear(mp,dp,blkno,(int32)nblocks,l2nb,results))
								 != ENOSPC)
		{
			if (rc == 0)
			{
				DBALLOC(mp->db_DBmap, mp->db_mapsize, *results,
					nblocks);
				bmLazyWrite(bp,0);
			}
			else
				bmRelease(bp);

			IREAD_UNLOCK(ipbmap);
			return(rc);
		}

		/* try to satisfy the allocation request with blocks within
		 * the same allocation group as the hint.
		 */
		if ((rc = dbAllocDmapLev(mp,dp,(int32)nblocks,l2nb,results))
								 != ENOSPC)
		{
			if (rc == 0)
			{
				DBALLOC(mp->db_DBmap, mp->db_mapsize, *results,
					nblocks);
				bmLazyWrite(bp,0);
			}
			else
				bmRelease(bp);

			IREAD_UNLOCK(ipbmap);
			return(rc);
		}

		bmRelease(bp);
		IREAD_UNLOCK(ipbmap);
	}

tryag:
	IWRITE_LOCK(ipbmap);

	/* determine the allocation group number of the hint and try to
	 * allocate within this allocation group.  if that fails, try to
	 * allocate anywhere in the map.
	 */
	agno = blkno >> mp->db_agl2size;
	if ((rc = dbAllocAG(mp,agno,nblocks,l2nb,results)) == ENOSPC)
		rc = dbAllocAny(mp,nblocks,l2nb,results);
	if (rc == 0)
	{
		DBALLOC(mp->db_DBmap, mp->db_mapsize, *results, nblocks);
	}

	IWRITE_UNLOCK(ipbmap);

	return(rc);
}


/*
 * NAME:	dbAllocExact()
 *
 * FUNCTION:    try to allocate the requested extent;
 *
 * PARAMETERS:
 *      ip	- pointer to in-core inode;
 *      blkno	- extent address;
 *      nblocks	- extent length;
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 */
int32 dbAllocExact(inode_t *ip,
        int64	blkno,
        int32	nblocks)
{
	int32	rc;
	inode_t	*ipbmap = ip->i_ipmnt->i_ipbmap;
	bmap_t	*mp = ipbmap->i_bmap;
	dmap_t	*dp;
	int64	lblkno;
	jbuf_t	*bp;

	IREAD_LOCK(ipbmap);

	/*
	 * validate extent request:
	 *
	 * note: defragfs policy:
	 *  max 64 blocks will be moved.  
	 *  allocation request size must be satisfied from a single dmap.
	 */
	if (nblocks <= 0
	    || nblocks > BPERDMAP
	    || blkno >= mp->db_mapsize)
	{
		IREAD_UNLOCK(ipbmap);
		return EINVAL;
	}

	if (nblocks > ((int64)1 << mp->db_maxfreebud))
	{
		/* the free space is no longer available */
		IREAD_UNLOCK(ipbmap);
		return ENOSPC;
	}

	/* read in the dmap covering the extent */
       	lblkno = BLKTODMAP(blkno, mp->db_l2nbperpage);
       	if (bmRead(ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
	{
		IREAD_UNLOCK(ipbmap);
               	return(EIO);
	}
	dp = (dmap_t *)bp->b_bdata;

	/* try to allocate the requested extent */
	rc = dbAllocNext(mp, dp, blkno, nblocks);

	IREAD_UNLOCK(ipbmap);

	if (rc == 0)
	{
		DBALLOC(mp->db_DBmap, mp->db_mapsize, blkno, nblocks);
		bmLazyWrite(bp,0);
	}
	else /* rc == EIO or ENOSPC */
	{
		bmRelease(bp);
	}

	return(rc);
}


/*
 * NAME:	dbReAlloc()
 *
 * FUNCTION:    attempt to extend a current allocation by a specified
 *		number of blocks.
 *
 *		this routine attempts to satisfy the allocation request
 *		by first trying to extend the existing allocation in
 *		place by allocating the additional blocks as the blocks
 *		immediately following the current allocation.  if these
 *		blocks are not available, this routine will attempt to
 *		allocate a new set of contiguous blocks large enough
 *		to cover the existing allocation plus the additional
 *		number of blocks required.
 *
 * PARAMETERS:
 *      ip	    -  pointer to in-core inode requiring allocation.
 *      blkno	    -  starting block of the current allocation.
 *      nblocks	    -  number of contiguous blocks within the current
 *		       allocation.
 *      addnblocks  -  number of blocks to add to the allocation.
 *      results	-      on successful return, set to the starting block number
 *		       of the existing allocation if the existing allocation
 *		       was extended in place or to a newly allocated contiguous
 *		       range if the existing allocation could not be extended
 *		       in place.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 */
int32
dbReAlloc(inode_t *ip,
          int64 blkno,
          int64 nblocks,
          int64 addnblocks,
          int64 *results)
{
	int32 rc;

	/* try to extend the allocation in place.
	 */
	if ((rc = dbExtend(ip,blkno,nblocks,addnblocks)) == 0)
	{
		*results = blkno;
		return(0);
	}
	else
	{	
		if (rc != ENOSPC)
			return(rc);
	}

	/* could not extend the allocation in place, so allocate a
	 * new set of blocks for the entire request (i.e. try to get
	 * a range of contiguous blocks large enough to cover the
	 * existing allocation plus the additional blocks.)
	 */
	return(dbAlloc(ip, blkno+nblocks-1, addnblocks+nblocks, results));
}


/*
 * NAME:	dbExtend()
 *
 * FUNCTION:    attempt to extend a current allocation by a specified
 *		number of blocks.
 *
 *		this routine attempts to satisfy the allocation request
 *		by first trying to extend the existing allocation in
 *		place by allocating the additional blocks as the blocks
 *		immediately following the current allocation.
 *
 * PARAMETERS:
 *      ip	    -  pointer to in-core inode requiring allocation.
 *      blkno	    -  starting block of the current allocation.
 *      nblocks	    -  number of contiguous blocks within the current
 *		       allocation.
 *      addnblocks  -  number of blocks to add to the allocation.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 */
int32
dbExtend(inode_t *ip,
          int64 blkno,
          int64 nblocks,
          int64 addnblocks)
{
	int64 lblkno, lastblkno, extblkno;
	jbuf_t *bp;
	dmap_t *dp;
	int32 rc;
	inode_t *ipbmap = ip->i_ipmnt->i_ipbmap;
	bmap_t *mp;

	/* get the last block of the current allocation */
	lastblkno = blkno + nblocks - 1;

	/* determine the block number of the block following
	 * the existing allocation.
	 */
	extblkno = lastblkno + 1;

	IREAD_LOCK(ipbmap);

	/* better be within the file system */
	mp = ipbmap->i_bmap;
	assert(lastblkno >= 0 && lastblkno < mp->db_mapsize);

	/* we'll attempt to extend the current allocation in place by
	 * allocating the additional blocks as the blocks immediately
 	 * following the current allocation.  we only try to extend the
	 * current allocation in place if the number of additional blocks
	 * can fit into a dmap, the last block of the current allocation
	 * is not the last block of the file system, and the start of the
	 * inplace extension is not on an allocation group boundry.
	 */
	if (addnblocks > BPERDMAP || extblkno >= mp->db_mapsize ||
	    (extblkno & (mp->db_agsize-1)) == 0)
	{
		IREAD_UNLOCK(ipbmap);
		return(ENOSPC);
	}

	/* get the buffer for the dmap containing the first block
	 * of the extension.
	 */
        lblkno = BLKTODMAP(extblkno,mp->db_l2nbperpage);
        if (bmRead(ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
	{
		IREAD_UNLOCK(ipbmap);
               	return(EIO);
	}

	DBALLOCCK(mp->db_DBmap, mp->db_mapsize, blkno, nblocks);
	dp = (dmap_t *) bp->b_bdata;
	
	/* try to allocate the blocks immediately following the
	 * current allocation.
	 */
	rc = dbAllocNext(mp, dp, extblkno, (int32)addnblocks);

	IREAD_UNLOCK(ipbmap);

	/* were we successful ? */
	if (rc == 0)
	{
		DBALLOC(mp->db_DBmap, mp->db_mapsize, extblkno, addnblocks);
		bmLazyWrite(bp,0);
	}
	else
	{
		/* we were not successful */
		bmRelease(bp);
		assert(rc == ENOSPC || rc == EIO);
	}

	return(rc);
}


/*
 * NAME:	dbAllocNext()
 *
 * FUNCTION:    attempt to allocate the blocks of the specified block
 *		range within a dmap.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap.
 *      blkno	-  starting block number of the range.
 *      nblocks	-  number of contiguous free blocks of the range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * serialization: IREAD_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbAllocNext(bmap_t *mp,
            dmap_t *dp,
            int64 blkno,
            int32 nblocks)
{
	int32 dbitno, word, rembits, nb, nwords, wbitno, nw;
	int8 l2size, *leaf;
	uint32 mask;

	/* pick up a pointer to the leaves of the dmap tree.
	 */
	leaf = dp->tree.stree + dp->tree.leafidx;

	/* determine the bit number and word within the dmap of the
	 * starting block.
	 */
	dbitno = blkno & (BPERDMAP - 1);
	word = dbitno >> L2DBWORD;

	/* check if the specified block range is contained within
	 * this dmap.
	 */
	if (dbitno + nblocks > BPERDMAP)
		return(ENOSPC);

	/* check if the starting leaf indicates that anything
	 * is free.
	 */
	if (leaf[word] == NOFREE)
		return(ENOSPC);

	/* check the dmaps words corresponding to block range to see
	 * if the block range is free.  not all bits of the first and
	 * last words may be contained within the block range.  if this
	 * is the case, we'll work against those words (i.e. partial first
	 * and/or last) on an individual basis (a single pass) and examine
	 * the actual bits to determine if they are free.  a single pass
	 * will be used for all dmap words fully contained within the
	 * specified range.  within this pass, the leaves of the dmap
	 * tree will be examined to determine if the blocks are free. a
	 * single leaf may describe the free space of multiple dmap
	 * words, so we may visit only a subset of the actual leaves
	 * corresponding to the dmap words of the block range.
	 */
	for (rembits = nblocks; rembits > 0; rembits -= nb, dbitno += nb)
	{
		/* determine the bit number within the word and
		 * the number of bits within the word.
		 */
		wbitno = dbitno & (DBWORD-1);
		nb = MIN(rembits, DBWORD - wbitno);

		/* check if only part of the word is to be examined.
		 */
		if (nb < DBWORD)
		{
			/* check if the bits are free.
			 */
			mask = (ONES << (DBWORD - nb) >> wbitno);
			if ((mask & ~dp->wmap[word]) != mask)
				return(ENOSPC);

			word += 1;
		}
		else
		{
			/* one or more dmap words are fully contained
			 * within the block range.  determine how many
			 * words and how many bits.
			 */
			nwords = rembits >> L2DBWORD;
			nb = nwords << L2DBWORD;

			/* now examine the appropriate leaves to determine
			 * if the blocks are free.
			 */
			while (nwords > 0)
			{
				/* does the leaf describe any free space ?
				 */
				if (leaf[word] < BUDMIN)
					return(ENOSPC);

				/* determine the l2 number of bits provided
				 * by this leaf.
				 */
				l2size = MIN(leaf[word],NLSTOL2BSZ(nwords));

				/* determine how many words were handled.
				 */
				nw = BUDSIZE(l2size,BUDMIN);

				nwords -= nw;
				word += nw;
			}
		}
	}

	/* allocate the blocks.
	 */
	return(dbAllocDmap(mp,dp,blkno,nblocks));
}


/*
 * NAME:	dbAllocNear()
 *
 * FUNCTION:    attempt to allocate a number of contiguous free blocks near
 *		a specified block (hint) within a dmap.
 *
 *		starting with the dmap leaf that covers the hint, we'll
 *		check the next four contiguous leaves for sufficient free
 *		space.  if sufficient free space is found, we'll allocate
 *		the desired free space.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap.
 *      blkno	-  block number to allocate near.
 *      nblocks	-  actual number of contiguous free blocks desired.
 *      l2nb	-  log2 number of contiguous free blocks desired.
 *      results	-  on successful return, set to the starting block number
 *		   of the newly allocated range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * serialization: IREAD_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbAllocNear(bmap_t *mp,
            dmap_t *dp,
            int64 blkno,
            int32 nblocks,
            int8 l2nb,
            int64 *results)
{
	int32 word, lword, rc;
	int8 *leaf = dp->tree.stree + dp->tree.leafidx;
	
	/* determine the word within the dmap that holds the hint
	 * (i.e. blkno).  also, determine the last word in the dmap
	 * that we'll include in our examination.
	 */
        word = (blkno & (BPERDMAP - 1)) >> L2DBWORD;
	lword = MIN(word+4,LPERDMAP);

	/* examine the leaves for sufficient free space.
	 */
	for (; word < lword; word++)
	{
		/* does the leaf describe sufficient free space ?
		 */
		if (leaf[word] < l2nb)
			continue;

		/* determine the block number within the file system
		 * of the first block described by this dmap word.
		 */
        	blkno = dp->start + (word << L2DBWORD);

		/* if not all bits of the dmap word are free, get the
		 * starting bit number within the dmap word of the required
		 * string of free bits and adjust the block number with the
		 * value.
		 */
        	if (leaf[word] < BUDMIN)
                	blkno += dbFindBits(dp->wmap[word], l2nb);

		/* allocate the blocks.
		 */
        	if ((rc = dbAllocDmap(mp,dp,blkno,nblocks)) == 0)
                	*results = blkno;

        	return(rc);
	}

	return(ENOSPC);
}


/*
 * NAME:	dbAllocAG()
 *
 * FUNCTION:    attempt to allocate the specified number of contiguous
 *		free blocks within the specified allocation group.
 *
 *		unless the allocation group size is equal to the number
 *		of blocks per dmap, the dmap control pages will be used to
 *		find the required free space, if available.  we start the
 *		search at the highest dmap control page level which
 *		distinctly describes the allocation group's free space
 *		(i.e. the highest level at which the allocation group's
 *		free space is not mixed in with that of any other group).
 *		in addition, we start the search within this level at a
 *		height of the dmapctl dmtree at which the nodes distinctly
 *		describe the allocation group's free space.  at this height,
 *		the allocation group's free space may be represented by 1
 *		or two sub-trees, depending on the allocation group size.
 *		we search the top nodes of these subtrees left to right for
 *		sufficient free space.  if sufficient free space is found,
 *		the subtree is searched to find the leftmost leaf that 
 *		has free space.  once we have made it to the leaf, we
 *		move the search to the next lower level dmap control page
 *		corresponding to this leaf.  we continue down the dmap control
 *		pages until we find the dmap that contains or starts the
 *		sufficient free space and we allocate at this dmap.
 *
 *		if the allocation group size is equal to the dmap size,
 *		we'll start at the dmap corresponding to the allocation
 *		group and attempt the allocation at this level.
 *
 *		the dmap control page search is also not performed if the
 *		allocation group is completely free and we go to the first
 *		dmap of the allocation group to do the allocation.  this is
 *		done because the allocation group may be part (not the first
 *		part) of a larger binary buddy system, causing the dmap
 *		control pages to indicate no free space (NOFREE) within
 *		the allocation group.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *	agno	- allocation group number.
 *      nblocks	-  actual number of contiguous free blocks desired.
 *      l2nb	-  log2 number of contiguous free blocks desired.
 *      results	-  on successful return, set to the starting block number
 *		   of the newly allocated range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * note: IWRITE_LOCK(ipmap) held on entry/exit;
 */
static int32
dbAllocAG(bmap_t *mp,
          int32 agno,
          int64 nblocks,
          int8 l2nb,
          int64 *results)
{
	jbuf_t *bp;
	dmapctl_t *dcp;
	int32 rc, ti, i, k, m, n, agperlev;
	int64 blkno, lblkno;
	int8 budmin;

	/* allocation request should not be for more than the
	 * allocation group size.
	 */
	assert(l2nb <= mp->db_agl2size);

	/* determine the starting block number of the allocation
	 * group.
	 */
	blkno = (int64)agno << mp->db_agl2size;

	/* check if the allocation group size is the minimum allocation
	 * group size or if the allocation group is completely free. if
	 * the allocation group size is the minimum size of BPERDMAP (i.e.
	 * 1 dmap), there is no need to search the dmap control page (below)
	 * that fully describes the allocation group since the allocation
	 * group is already fully described by a dmap.  in this case, we
	 * just call dbAllocCtl() to search the dmap tree and allocate the
	 * required space if available.  
	 *
	 * if the allocation group is completely free, dbAllocCtl() is
	 * also called to allocate the required space.  this is done for
	 * two reasons.  first, it makes no sense searching the dmap control
	 * pages for free space when we know that free space exists.  second,
	 * the dmap control pages may indicate that the allocation group
	 * has no free space if the allocation group is part (not the first
	 * part) of a larger binary buddy system.
	 */
	if (mp->db_agsize == BPERDMAP || mp->db_agfree[agno] == mp->db_agsize)
	{
		rc = dbAllocCtl(mp,nblocks,l2nb,blkno,results);
		assert(!(rc == ENOSPC && mp->db_agfree[agno] == mp->db_agsize));
		return(rc);
	}

	/* the buffer for the dmap control page that fully describes the
	 * allocation group.
	 */
        lblkno = BLKTOCTL(blkno, mp->db_l2nbperpage, mp->db_aglevel);
        if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
               	return(EIO);
	dcp = (dmapctl_t *) bp->b_bdata;
	budmin = dcp->budmin;

	/* search the subtree(s) of the dmap control page that describes
	 * the allocation group, looking for sufficient free space.  to begin,
	 * determine how many allocation groups are represented in a dmap
	 * control page at the control page level (i.e. L0, L1, L2) that
	 * fully describes an allocation group. next, determine the starting
	 * tree index of this allocation group within the control page.
	 */
	agperlev = (1 << (L2LPERCTL - (mp->db_agheigth << 1))) / mp->db_agwidth;
	ti = mp->db_agstart + mp->db_agwidth * (agno & (agperlev-1));

	/* dmap control page trees fan-out by 4 and a single allocation 
	 * group may be described by 1 or 2 subtrees within the ag level
	 * dmap control page, depending upon the ag size. examine the ag's
	 * subtrees for sufficient free space, starting with the leftmost
	 * subtree.
	 */
	for (i = 0; i < mp->db_agwidth; i++, ti++)
	{
		/* is there sufficient free space ?
		 */
		if (l2nb > dcp->stree[ti])
			continue;
			
		/* sufficient free space found in a subtree. now search down
		 * the subtree to find the leftmost leaf that describes this
		 * free space.
		 */
        	for (k = mp->db_agheigth; k > 0; k--)
        	{
               		for (n = 0, m = (ti << 2) + 1; n < 4; n++)
			{
                       		if (l2nb <= dcp->stree[m+n])
				{
					ti = m + n;
                       			break;
				}
			}
               		assert(n < 4);
        	}

		/* determine the block number within the file system
		 * that corresponds to this leaf.
		 */
		if (mp->db_aglevel == 2)
			blkno = 0;
		else if (mp->db_aglevel == 1)
			blkno &= ~(MAXL1SIZE-1);
		else 	/* mp->db_aglevel == 0 */
			blkno &= ~(MAXL0SIZE-1);

		blkno += ((int64)(ti - dcp->leafidx)) << budmin;

		/* release the buffer in preparation for going down
		 * the next level of dmap control pages.
		 */
		bmRelease(bp);

		/* check if we need to continue to search down the lower
		 * level dmap control pages.  we need to if the number of
		 * blocks required is less than maximum number of blocks
		 * described at the next lower level.
		 */
		if (l2nb < budmin)
		{

			/* search the lower level dmap control pages to get
		 	 * the starting block number of the the dmap that
			 * contains or starts off the free space.
		 	 */
			if (rc = dbFindCtl(mp,l2nb,mp->db_aglevel-1,&blkno))
			{
				assert(rc != ENOSPC);
				return(rc);
			}
		}

		/* allocate the blocks.
		 */
		rc = dbAllocCtl(mp,nblocks,l2nb,blkno,results);
		assert(rc != ENOSPC);
		return(rc);
	}

	/* no space in the allocation group.  release the buffer and
	 * return ENOSPC.
	 */
	bmRelease(bp);

	return(ENOSPC);
}


/*
 * NAME:	dbAllocAny()
 *
 * FUNCTION:    attempt to allocate the specified number of contiguous
 *		free blocks anywhere in the file system.
 *
 *		dbAllocAny() attempts to find the sufficient free space by
 *		searching down the dmap control pages, starting with the
 *		highest level (i.e. L0, L1, L2) control page.  if free space
 *		large enough to satisfy the desired free space is found, the
 *		desired free space is allocated.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      nblocks	 -  actual number of contiguous free blocks desired.
 *      l2nb	 -  log2 number of contiguous free blocks desired.
 *      results	-  on successful return, set to the starting block number
 *		   of the newly allocated range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * serialization: IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbAllocAny(bmap_t *mp,
           int64 nblocks,
           int8 l2nb,
           int64 *results)
{
	int32 rc;
	int64 blkno = 0;

	/* starting with the top level dmap control page, search
	 * down the dmap control levels for sufficient free space.
	 * if free space is found, dbFindCtl() returns the starting
	 * block number of the dmap that contains or starts off the
	 * range of free space.
	 */
	if (rc = dbFindCtl(mp,l2nb,mp->db_maxlevel,&blkno))
		return(rc);

	/* allocate the blocks.
	 */
	rc = dbAllocCtl(mp,nblocks,l2nb,blkno,results);
	assert(rc != ENOSPC)
	return(rc);
}


/*
 * NAME:	dbFindCtl()
 *
 * FUNCTION:    starting at a specified dmap control page level and block
 *		number, search down the dmap control levels for a range of
 *	        contiguous free blocks large enough to satisfy an allocation
 *		request for the specified number of free blocks.
 *
 *		if sufficient contiguous free blocks are found, this routine
 *		returns the starting block number within a dmap page that
 *		contains or starts a range of contiqious free blocks that
 *		is sufficient in size.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      level	-  starting dmap control page level.
 *      l2nb	-  log2 number of contiguous free blocks desired.
 *      *blkno	-  on entry, starting block number for conducting the search.
 *		   on successful return, the first block within a dmap page
 *		   that contains or starts a range of contiguous free blocks.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * serialization: IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbFindCtl(bmap_t *mp,
	  int8 l2nb,
	  int32 level,
	  int64 *blkno)
{
	int32 rc, leafidx, lev;
	int64 b, lblkno;
	dmapctl_t *dcp;
	int8 budmin;
	jbuf_t *bp;

	/* starting at the specified dmap control page level and block
	 * number, search down the dmap control levels for the starting
	 * block number of a dmap page that contains or starts off 
	 * sufficient free blocks.
	 */
	for (lev = level, b = *blkno; lev >= 0; lev--)
	{
		/* get the buffer of the dmap control page for the block
		 * number and level (i.e. L0, L1, L2).
		 */
        	lblkno = BLKTOCTL(b, mp->db_l2nbperpage, lev);
        	if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
               		return(EIO);
		dcp = (dmapctl_t *) bp->b_bdata;
		budmin = dcp->budmin;

		/* search the tree within the dmap control page for
		 * sufficent free space.  if sufficient free space is found,
		 * dbFindLeaf() returns the index of the leaf at which
		 * free space was found.
		 */
		rc = dbFindLeaf((dmtree_t *)dcp,l2nb,&leafidx);

		/* release the buffer.
		 */
		bmRelease(bp);
		
		/* space found ?
		 */
		if (rc)
		{
			assert(lev == level);
			return(ENOSPC);
		}

		/* adjust the block number to reflect the location within
		 * the dmap control page (i.e. the leaf) at which free 
		 * space was found.
		 */
		b += (((int64)leafidx) << budmin);

		/* we stop the search at this dmap control page level if
		 * the number of blocks required is greater than or equal
		 * to the maximum number of blocks described at the next
		 * (lower) level.
		 */
		if (l2nb >= budmin)
			break;
	}

	*blkno = b;
	return(0);
}


/*
 * NAME:	dbAllocCtl()
 *
 * FUNCTION:    attempt to allocate a specified number of contiguous
 *		blocks starting within a specific dmap.  
 *		
 *		this routine is called by higher level routines that search
 *		the dmap control pages above the actual dmaps for contiguous
 *		free space.  the result of successful searches by these
 * 		routines are the starting block numbers within dmaps, with
 *		the dmaps themselves containing the desired contiguous free
 *		space or starting a contiguous free space of desired size
 *		that is made up of the blocks of one or more dmaps. these
 *		calls should not fail due to insufficent resources.
 *
 *		this routine is called in some cases where it is not known
 *		whether it will fail due to insufficient resources.  more
 *		specifically, this occurs when allocating from an allocation
 *		group whose size is equal to the number of blocks per dmap.
 *		in this case, the dmap control pages are not examined prior
 *		to calling this routine (to save pathlength) and the call
 *		might fail.
 *
 *		for a request size that fits within a dmap, this routine relies
 *		upon the dmap's dmtree to find the requested contiguous free
 *		space.  for request sizes that are larger than a dmap, the
 *		requested free space will start at the first block of the
 *		first dmap (i.e. blkno).
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      nblocks	 -  actual number of contiguous free blocks to allocate.
 *      l2nb	 -  log2 number of contiguous free blocks to allocate.
 *      blkno	 -  starting block number of the dmap to start the allocation
 *		    from.
 *      results	-  on successful return, set to the starting block number
 *		   of the newly allocated range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * serialization: IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbAllocCtl(bmap_t *mp,
	   int64 nblocks,
	   int8 l2nb,
	   int64 blkno,
	   int64 *results)
{
	int32 rc, nb;
	int64 b, lblkno, n;
	jbuf_t *bp;
	dmap_t *dp;

	/* check if the allocation request is confined to a single dmap.
	 */
	if (l2nb <= L2BPERDMAP)
	{
		/* get the buffer for the dmap.
		 */
        	lblkno = BLKTODMAP(blkno, mp->db_l2nbperpage);
        	if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
               		return(EIO);
		dp = (dmap_t *) bp->b_bdata;

		/* try to allocate the blocks.
		 */
		if (rc = dbAllocDmapLev(mp,dp,(int32)nblocks,l2nb,results))
			bmRelease(bp);
		else
			bmLazyWrite(bp,0);


		return(rc);
	}

	/* allocation request involving multiple dmaps. it must start on
	 * a dmap boundary.
	 */
	assert((blkno & (BPERDMAP-1)) == 0);

	/* allocate the blocks dmap by dmap.
	 */
	for (n = nblocks, b = blkno; n > 0; n -= nb, b += nb)
	{
		/* get the buffer for the dmap.
		 */
        	lblkno = BLKTODMAP(b, mp->db_l2nbperpage);
        	if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		{
			rc = EIO;
        		goto backout;
		}
		dp = (dmap_t *) bp->b_bdata;

		/* the dmap better be all free.
		 */
		assert(dp->tree.stree[ROOT] == L2BPERDMAP);

		/* determine how many blocks to allocate from this dmap.
		 */
		nb = MIN(n,BPERDMAP);

		/* allocate the blocks from the dmap.
		 */
		if (rc = dbAllocDmap(mp,dp,b,nb))
		{
			bmRelease(bp);
			goto backout;
		}

		/* write the buffer.
		 */
		bmLazyWrite(bp,0);
	}

	/* set the results (starting block number) and return.
	 */ 
	*results = blkno;
	return(0);

	/* something failed in handling an allocation request involving
	 * multiple dmaps.  we'll try to clean up by backing out any
	 * allocation that has already happened for this request.  if
	 * we fail in backing out the allocation, we'll mark the file
	 * system to indicate that blocks have been leaked.
	 */
backout:

	/* try to backout the allocations dmap by dmap.
	 */
	for (n = nblocks - n, b = blkno; n > 0; n -= BPERDMAP, b += BPERDMAP)
	{
		/* get the buffer for this dmap.
		 */
        	lblkno = BLKTODMAP(b, mp->db_l2nbperpage);
        	if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		{
			/* could not back out.  mark the file system
			 * to indicate that we have leaked blocks.
			 */
			fsDirty(); /* !!! */
jERROR(1,("dbAllocCtl: I/O Error: Block Leakage.\n"));
			continue;
		}
		dp = (dmap_t *) bp->b_bdata;

		/* free the blocks is this dmap.
		 */
		if (dbFreeDmap(mp,dp,b,BPERDMAP))
		{
			/* could not back out.  mark the file system
			 * to indicate that we have leaked blocks.
			 */
			bmRelease(bp);
			fsDirty(); /* !!! */
jERROR(1,("dbAllocCtl: Block Leakage.\n"));
			continue;
		}

		/* write the buffer.
		 */
		bmLazyWrite(bp,0);
	}

	return(rc);
}


/*
 * NAME:	dbAllocDmapLev()
 *
 * FUNCTION:    attempt to allocate a specified number of contiguous blocks
 *		from a specified dmap.
 *		
 *		this routine checks if the contiguous blocks are available.
 *		if so, nblocks of blocks are allocated; otherwise, ENOSPC is
 *		returned.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap to attempt to allocate blocks from. 
 *      l2nb	-  log2 number of contiguous block desired.
 *      nblocks	-  actual number of contiguous block desired.
 *      results	-  on successful return, set to the starting block number
 *		   of the newly allocated range.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient disk resources
 *      EIO	- i/o error
 *
 * serialization: IREAD_LOCK(ipbmap), e.g., from dbAlloc(), or 
 *	IWRITE_LOCK(ipbmap), e.g., dbAllocCtl(), held on entry/exit;
 */
static int32
dbAllocDmapLev(bmap_t *mp,
	       dmap_t *dp,
	       int32 nblocks,
	       int8 l2nb,
	       int64 *results)
{
	int64 blkno;
	int32 leafidx, rc;

	/* can't be more than a dmaps worth of blocks */
	assert(l2nb <= L2BPERDMAP);

	/* search the tree within the dmap page for sufficient
	 * free space.  if sufficient free space is found, dbFindLeaf()
	 * returns the index of the leaf at which free space was found.
	 */
	if (dbFindLeaf((dmtree_t *)&dp->tree,l2nb,&leafidx))
		return(ENOSPC);

	/* determine the block number within the file system corresponding
	 * to the leaf at which free space was found.
	 */
	blkno = dp->start + (leafidx << L2DBWORD);

	/* if not all bits of the dmap word are free, get the starting
	 * bit number within the dmap word of the required string of free
	 * bits and adjust the block number with this value.
	 */
	if (dp->tree.stree[leafidx + LEAFIND] < BUDMIN)
		blkno += dbFindBits(dp->wmap[leafidx], l2nb);

	/* allocate the blocks */
	if ((rc = dbAllocDmap(mp,dp,blkno,nblocks)) == 0)
		*results = blkno;

	return(rc);
}


/*
 * NAME:	dbAllocDmap()
 *
 * FUNCTION:    adjust the disk allocation map to reflect the allocation
 *		of a specified block range within a dmap.
 *
 *		this routine allocates the specified blocks from the dmap
 *		through a call to dbAllocBits(). if the allocation of the
 *		block range causes the maximum string of free blocks within
 *		the dmap to change (i.e. the value of the root of the dmap's
 *		dmtree), this routine will cause this change to be reflected
 *		up through the appropriate levels of the dmap control pages
 *		by a call to dbAdjCtl() for the L0 dmap control page that
 *		covers this dmap.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap to allocate the block range from.
 *      blkno	-  starting block number of the block to be allocated.
 *      nblocks	-  number of blocks to be allocated.
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static int32 dbAllocDmap(
	bmap_t	*mp,
	dmap_t	*dp,
	int64	blkno,
	int32	nblocks)
{
	int8 oldroot;
	int32 rc;

	/* save the current value of the root (i.e. maximum free string)
	 * of the dmap tree.
	 */
	oldroot = dp->tree.stree[ROOT];

	/* allocate the specified (blocks) bits */
	dbAllocBits(mp,dp,blkno,nblocks);

	/* if the root has not changed, done. */
	if (dp->tree.stree[ROOT] == oldroot)
		return(0);

	/* root changed. bubble the change up to the dmap control pages.
	 * if the adjustment of the upper level control pages fails,
	 * backout the bit allocation (thus making everything consistent).
	 */
	if (rc = dbAdjCtl(mp,blkno,dp->tree.stree[ROOT],1,0))
		dbFreeBits(mp,dp,blkno,nblocks);

	return(rc);
}


/*
 * NAME:	dbFreeDmap()
 *
 * FUNCTION:    adjust the disk allocation map to reflect the allocation
 *		of a specified block range within a dmap.
 *
 *		this routine frees the specified blocks from the dmap through
 *		a call to dbFreeBits(). if the deallocation of the block range
 *		causes the maximum string of free blocks within the dmap to
 *		change (i.e. the value of the root of the dmap's dmtree), this
 *		routine will cause this change to be reflected up through the
 *	        appropriate levels of the dmap control pages by a call to
 *		dbAdjCtl() for the L0 dmap control page that covers this dmap.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap to free the block range from.
 *      blkno	-  starting block number of the block to be freed.
 *      nblocks	-  number of blocks to be freed.
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbFreeDmap(bmap_t *mp,
	   dmap_t *dp,
	   int64 blkno,
	   int32 nblocks)
{
	int8 oldroot;
	int32 rc, word;

	/* save the current value of the root (i.e. maximum free string)
	 * of the dmap tree.
	 */
	oldroot = dp->tree.stree[ROOT];

	/* free the specified (blocks) bits */
	dbFreeBits(mp,dp,blkno,nblocks);

	/* if the root has not changed, done. */
	if (dp->tree.stree[ROOT] == oldroot)
		return(0);

	/* root changed. bubble the change up to the dmap control pages.
	 * if the adjustment of the upper level control pages fails,
	 * backout the deallocation. 
	 */
	if (rc = dbAdjCtl(mp,blkno,dp->tree.stree[ROOT],0,0))
	{
		word = (blkno & (BPERDMAP - 1)) >> L2DBWORD;

		/* as part of backing out the deallocation, we will have
		 * to back split the dmap tree if the deallocation caused
		 * the freed blocks to become part of a larger binary buddy
		 * system.
		 */
		if (dp->tree.stree[word] == NOFREE)
			dbBackSplit((dmtree_t *)&dp->tree,word);

		dbAllocBits(mp,dp,blkno,nblocks);
	}

	return(rc);
}


/*
 * NAME:	dbAllocBits()
 *
 * FUNCTION:    allocate a specified block range from a dmap.
 *
 *		this routine updates the dmap to reflect the working
 *		state allocation of the specified block range. it directly
 *		updates the bits of the working map and causes the adjustment
 *		of the binary buddy system described by the dmap's dmtree
 *		leaves to reflect the bits allocated.  it also causes the
 *		dmap's dmtree, as a whole, to reflect the allocated range.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap to allocate bits from.
 *      blkno	-  starting block number of the bits to be allocated.
 *      nblocks	-  number of bits to be allocated.
 *
 * RETURN VALUES: none
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static void
dbAllocBits(bmap_t *mp,
	    dmap_t *dp,
	    int64 blkno,
	    int32 nblocks)
{
	int32 dbitno, word, rembits, nb, nwords, wbitno, nw, agno;
	dmtree_t *tp = (dmtree_t *) &dp->tree;
	int8 size, *leaf;

        /* pick up a pointer to the leaves of the dmap tree */
	leaf = dp->tree.stree + LEAFIND;

        /* determine the bit number and word within the dmap of the
         * starting block.
         */
	dbitno = blkno & (BPERDMAP - 1);
	word = dbitno >> L2DBWORD;

	/* block range better be within the dmap */ 
	assert(dbitno + nblocks <= BPERDMAP);

        /* allocate the bits of the dmap's words corresponding to the block
	 * range. not all bits of the first and last words may be contained
	 * within the block range.  if this is the case, we'll work against
	 * those words (i.e. partial first and/or last) on an individual basis
	 * (a single pass), allocating the bits of interest by hand and
	 * updating the leaf corresponding to the dmap word. a single pass
	 * will be used for all dmap words fully contained within the
	 * specified range.  within this pass, the bits of all fully contained
	 * dmap words will be marked as free in a single shot and the leaves
	 * will be updated. a single leaf may describe the free space of
	 * multiple dmap words, so we may update only a subset of the actual
	 * leaves corresponding to the dmap words of the block range.
	 */
	for (rembits = nblocks; rembits > 0; rembits -= nb, dbitno += nb)
	{
                /* determine the bit number within the word and
                 * the number of bits within the word.
                 */
		wbitno = dbitno & (DBWORD-1);
		nb = MIN(rembits, DBWORD - wbitno);

		/* check if only part of a word is to be allocated.
                 */
		if (nb < DBWORD)
		{
                        /* allocate (set to 1) the appropriate bits within
                         * this dmap word.
                         */
			dp->wmap[word] |= (ONES << (DBWORD - nb) >> wbitno);

                        /* update the leaf for this dmap word. in addition
			 * to setting the leaf value to the binary buddy max
			 * of the updated dmap word, dbSplit() will split
			 * the binary system of the leaves if need be.
                         */
			dbSplit(tp, word, BUDMIN,
				dbMaxBud((char *)&dp->wmap[word]));

			word += 1;
		}
		else
		{
                        /* one or more dmap words are fully contained
                         * within the block range.  determine how many
                         * words and allocate (set to 1) the bits of these
			 * words.
                         */
			nwords = rembits >> L2DBWORD;
			memset(&dp->wmap[word], ONES, nwords*4);

                        /* determine how many bits.
                         */
			nb = nwords << L2DBWORD;

                        /* now update the appropriate leaves to reflect
                         * the allocated words.
                         */
			for (; nwords > 0; nwords -= nw)
			{
				assert(leaf[word] >= BUDMIN);

                                /* determine what the leaf value should be
                                 * updated to as the minimum of the l2 number
                                 * of bits being allocated and the l2 number
                                 * of bits currently described by this leaf.
                                 */
				size = MIN(leaf[word],NLSTOL2BSZ(nwords));

                        	/* update the leaf to reflect the allocation.
				 * in addition to setting the leaf value to
				 * NOFREE, dbSplit() will split the binary
				 * system of the leaves to reflect the current
				 * allocation (size).
                         	 */
				dbSplit(tp, word, size, NOFREE);

                                /* get the number of dmap words handled */
				nw = BUDSIZE(size,BUDMIN);
				word += nw;
			}
		}
	}

	/* update the free count for this dmap */
	dp->nfree -= nblocks;

	BMAP_LOCK(mp);

	/* if this allocation group is completely free,
	 * update the maximum allocation group number if this allocation
	 * group is the new max.
	 */
	agno = blkno >> mp->db_agl2size;
	if (agno > mp->db_maxag)
		mp->db_maxag = agno;

	/* update the free count for the allocation group and map */
	mp->db_agfree[agno] -= nblocks;
	mp->db_nfree -= nblocks;

	BMAP_UNLOCK(mp);
}


/*
 * NAME:	dbFreeBits()
 *
 * FUNCTION:    free a specified block range from a dmap.
 *
 *		this routine updates the dmap to reflect the working
 *		state allocation of the specified block range. it directly
 *		updates the bits of the working map and causes the adjustment
 *		of the binary buddy system described by the dmap's dmtree
 *		leaves to reflect the bits freed.  it also causes the dmap's
 *		dmtree, as a whole, to reflect the deallocated range.
 *
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      dp	-  pointer to dmap to free bits from.
 *      blkno	-  starting block number of the bits to be freed.
 *      nblocks	-  number of bits to be freed.
 *
 * RETURN VALUES: none
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static void
dbFreeBits(bmap_t *mp,
	   dmap_t *dp,
	   int64 blkno,
	   int32 nblocks)
{
	int32 dbitno, word, rembits, nb, nwords, wbitno, nw, agno;
	dmtree_t *tp = (dmtree_t *) &dp->tree;
	int8 size;

        /* determine the bit number and word within the dmap of the
         * starting block.
         */
	dbitno = blkno & (BPERDMAP - 1);
	word = dbitno >> L2DBWORD;

	/* block range better be within the dmap.
	 */ 
	assert(dbitno + nblocks <= BPERDMAP);

        /* free the bits of the dmaps words corresponding to the block range.
	 * not all bits of the first and last words may be contained within
	 * the block range.  if this is the case, we'll work against those
	 * words (i.e. partial first and/or last) on an individual basis
	 * (a single pass), freeing the bits of interest by hand and updating
	 * the leaf corresponding to the dmap word. a single pass will be used
	 * for all dmap words fully contained within the specified range.  
	 * within this pass, the bits of all fully contained dmap words will
	 * be marked as free in a single shot and the leaves will be updated. a
         * single leaf may describe the free space of multiple dmap words,
         * so we may update only a subset of the actual leaves corresponding
	 * to the dmap words of the block range.
	 *
	 * dbJoin() is used to update leaf values and will join the binary
	 * buddy system of the leaves if the new leaf values indicate this
	 * should be done.
	 */
	for (rembits = nblocks; rembits > 0; rembits -= nb, dbitno += nb)
	{
                /* determine the bit number within the word and
                 * the number of bits within the word.
                 */
		wbitno = dbitno & (DBWORD-1);
		nb = MIN(rembits, DBWORD - wbitno);

		/* check if only part of a word is to be freed.
		 */
		if (nb < DBWORD)
		{
			/* free (zero) the appropriate bits within this
			 * dmap word. 
			 */
			dp->wmap[word] &= ~(ONES << (DBWORD - nb) >> wbitno);

			/* update the leaf for this dmap word.
			 */
			dbJoin(tp, word, dbMaxBud((char *)&dp->wmap[word]));

			word += 1;
		}
		else
		{
                        /* one or more dmap words are fully contained
                         * within the block range.  determine how many
                         * words and free (zero) the bits of these words.
                         */
			nwords = rembits >> L2DBWORD;
			memset(&dp->wmap[word], 0, nwords*4);

			/* determine how many bits.
			 */
			nb = nwords << L2DBWORD;

			/* now update the appropriate leaves to reflect
			 * the freed words.
			 */
			for (; nwords > 0; nwords -= nw)
			{
				/* determine what the leaf value should be
				 * updated to as the minimum of the l2 number
				 * of bits being freed and the l2 (max) number
				 * of bits that can be described by this leaf.
				 */
				size = MIN(LITOL2BSZ(word,L2LPERDMAP,BUDMIN),
							NLSTOL2BSZ(nwords));

				/* update the leaf.
				 */
				dbJoin(tp, word, size);

				/* get the number of dmap words handled.
				 */
				nw = BUDSIZE(size,BUDMIN);
				word += nw;
			}
		}
	}

	/* update the free count for this dmap.
	 */
	dp->nfree += nblocks;

	BMAP_LOCK(mp);

	/* update the free count for the allocation group and 
	 * map.
	 */
	agno = blkno >> mp->db_agl2size;
	mp->db_nfree += nblocks;
	mp->db_agfree[agno] += nblocks;

	/* check if this allocation group is not completely free and
	 * if it is currently the maximum (rightmost) allocation group.
	 * if so, establish the new maximum allocation group number by
	 * searching left for the first allocation group with allocation.
	 */
	if ((mp->db_agfree[agno] == mp->db_agsize && agno == mp->db_maxag) ||
	    (agno == mp->db_numag - 1 &&
	     mp->db_agfree[agno] == (mp->db_mapsize & (BPERDMAP - 1))))
	{
		while (mp->db_maxag > 0)
		{
			mp->db_maxag -= 1;
			if (mp->db_agfree[mp->db_maxag] != mp->db_agsize)
				break;
		}

		/* re-establish the allocation group preference if the
		 * current preference is right of the maximum allocation
	 	 * group.
		 */
		if (mp->db_agpref > mp->db_maxag)
			mp->db_agpref = mp->db_maxag;
	}

	BMAP_UNLOCK(mp);
}


/*
 * NAME:	dbAdjCtl()
 *
 * FUNCTION:	adjust a dmap control page at a specified level to reflect
 *		the change in a lower level dmap or dmap control page's
 *		maximum string of free blocks (i.e. a change in the root
 *		of the lower level object's dmtree) due to the allocation
 *		or deallocation of a range of blocks with a single dmap.
 *
 *		on entry, this routine is provided with the new value of
 *		the lower level dmap or dmap control page root and the
 *		starting block number of the block range whose allocation
 *		or deallocation resulted in the root change.  this range
 *		is respresented by a single leaf of the current dmapctl
 *		and the leaf will be updated with this value, possibly
 *		causing a binary buddy system within the leaves to be 
 *		split or joined.  the update may also cause the dmapctl's
 *		dmtree to be updated.
 *
 *		if the adjustment of the dmap control page, itself, causes its
 *		root to change, this change will be bubbled up to the next dmap
 *		control level by a recursive call to this routine, specifying
 *		the new root value and the next dmap control page level to
 *		be adjusted.
 * PARAMETERS:
 *      mp	-  pointer to bmap descriptor
 *      blkno	-  the first block of a block range within a dmap.  it is
 *		   the allocation or deallocation of this block range that
 *		   requires the dmap control page to be adjusted.
 *      newval	-  the new value of the lower level dmap or dmap control
 *		   page root.
 *      alloc	-  TRUE if adjustment is due to an allocation.
 *      level	-  current level of dmap control page (i.e. L0, L1, L2) to
 *		   be adjusted.
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static int32
dbAdjCtl(bmap_t *mp,
	 int64 blkno,
	 int8 newval,
	 int32 alloc,
	 int32 level)
{
	jbuf_t *bp;
	int8 oldroot, oldval;
	int64 lblkno;
	dmapctl_t *dcp;
	int32 rc, leafno, ti;

	/* get the buffer for the dmap control page for the specified
	 * block number and control page level.
	 */
        lblkno = BLKTOCTL(blkno, mp->db_l2nbperpage, level);
        if (bmRead(mp->db_ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		return(EIO);
	dcp = (dmapctl_t *) bp->b_bdata;

	/* determine the leaf number corresponding to the block and
	 * the index within the dmap control tree.
	 */
	leafno = BLKTOCTLLEAF(blkno,dcp->budmin);
	ti = leafno + dcp->leafidx;

	/* save the current leaf value and the current root level (i.e.
	 * maximum l2 free string described by this dmapctl).
	 */
	oldval = dcp->stree[ti];
	oldroot = dcp->stree[ROOT];

	/* check if this is a control page update for an allocation.
	 * if so, update the leaf to reflect the new leaf value using
	 * dbSplit(); otherwise (deallocation), use dbJoin() to udpate
	 * the leaf with the new value.  in addition to updating the
	 * leaf, dbSplit() will also split the binary buddy system of
	 * the leaves, if required, and bubble new values within the
	 * dmapctl tree, if required.  similarly, dbJoin() will join
	 * the binary buddy system of leaves and bubble new values up
	 * the dmapctl tree as required by the new leaf value.
	 */
	if (alloc)
	{
		/* check if we are in the middle of a binary buddy
		 * system.  this happens when we are performing the
		 * first allocation out of an allocation group that
		 * is part (not the first part) of a larger binary
		 * buddy system.  if we are in the middle, back split
		 * the system prior to calling dbSplit() which assumes
		 * that it is at the front of a binary buddy system.
		 */
		if (oldval == NOFREE)
		{
			dbBackSplit((dmtree_t *)dcp,leafno);
			oldval = dcp->stree[ti];
		}
		dbSplit((dmtree_t *)dcp,leafno,dcp->budmin,newval);
	}
	else
	{
		dbJoin((dmtree_t *)dcp,leafno,newval);
	}

	/* check if the root of the current dmap control page changed due
	 * to the update and if the current dmap control page is not at
	 * the current top level (i.e. L0, L1, L2) of the map.  if so (i.e.
	 * root changed and this is not the top level), call this routine
	 * again (recursion) for the next higher level of the mapping to
	 * reflect the change in root for the current dmap control page.
	 */
	if (dcp->stree[ROOT] != oldroot)
	{
		/* are we below the top level of the map.  if so,
		 * bubble the root up to the next higher level.
		 */
 		if (level < mp->db_maxlevel)
		{
			/* bubble up the new root of this dmap control page to
	 	 	 * the next level.
			 */
			if (rc = dbAdjCtl(mp,blkno,dcp->stree[ROOT],alloc,
								level+1))
			{
				/* something went wrong in bubbling up the new
				 * root value, so backout the changes to the
				 * current dmap control page.
				 */
				if (alloc)
				{
					dbJoin((dmtree_t *)dcp,leafno,oldval);
				}
				else
				{
					/* the dbJoin() above might have
					 * caused a larger binary buddy system
					 * to form and we may now be in the
					 * middle of it.  if this is the case,
					 * back split the buddies.
					 */
					if (dcp->stree[ti] == NOFREE)
						dbBackSplit((dmtree_t *)dcp,
								leafno);
					dbSplit((dmtree_t *)dcp,leafno,	
							dcp->budmin, oldval);
				}

				/* release the buffer and return the error.
				 */
				bmRelease(bp);
				return(rc);
			}
		}
		else
		{
			/* we're at the top level of the map. update
			 * the bmap control page to reflect the size
			 * of the maximum free buddy system.
			 */
 			assert(level == mp->db_maxlevel);
 			assert(mp->db_maxfreebud == oldroot);
			mp->db_maxfreebud = dcp->stree[ROOT];
		}
	}

	/* write the buffer.
	 */
	bmLazyWrite(bp,0);
	return(0);
}


/*
 * NAME:	dbSplit()
 *
 * FUNCTION:    update the leaf of a dmtree with a new value, splitting
 *		the leaf from the binary buddy system of the dmtree's
 *		leaves, as required.
 *
 * PARAMETERS:
 *      tp	- pointer to the tree containing the leaf.
 *      leafno	- the number of the leaf to be updated.
 *      splitsz	- the size the binary buddy system starting at the leaf
 *		  must be split to, specified as the log2 number of blocks.
 *      newval	- the new value for the leaf.
 *
 * RETURN VALUES: none
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static void
dbSplit(dmtree_t *tp,
	int32 leafno,
	int8 splitsz,
	int8 newval)
{
	int32 budsz;
	int8 cursz, *leaf = tp->dmt_stree + tp->dmt_leafidx;

	/* check if the leaf needs to be split.
	 */
	if (leaf[leafno] > tp->dmt_budmin)
	{
		/* the split occurs by cutting the buddy system in half
		 * at the specified leaf until we reach the specified
		 * size.  pick up the starting split size (current size
		 * - 1 in l2) and the corresponding buddy size.
		 */
		cursz = leaf[leafno] - 1;
		budsz = BUDSIZE(cursz,tp->dmt_budmin);

		/* split until we reach the specified size.
		 */
		while (cursz >= splitsz)
		{
			/* update the buddy's leaf with its new value.
			 */
			dbAdjTree(tp,leafno ^ budsz,cursz);

			/* on to the next size and buddy.
			 */
			cursz -= 1;
			budsz >>= 1;
		}
	}

	/* adjust the dmap tree to reflect the specified leaf's new 
	 * value.
	 */
	dbAdjTree(tp,leafno,newval);
}


/*
 * NAME:	dbBackSplit()
 *
 * FUNCTION:    back split the binary buddy system of dmtree leaves
 *		that hold a specified leaf until the specified leaf
 *		starts its own binary buddy system.
 *
 *		the allocators typically perform allocations at the start
 *		of binary buddy systems and dbSplit() is used to accomplish
 *		any required splits.  in some cases, however, allocation
 *		may occur in the middle of a binary system and requires a
 *		back split, with the split proceeding out from the middle of
 *		the system (less efficient) rather than the start of the
 *		system (more efficient).  the cases in which a back split
 *		is required are rare and are limited to the first allocation
 *		within an allocation group which is a part (not first part)
 *		of a larger binary buddy system and a few exception cases
 *		in which a previous join operation must be backed out.
 *
 * PARAMETERS:
 *      tp	- pointer to the tree containing the leaf.
 *      leafno	- the number of the leaf to be updated.
 *
 * RETURN VALUES: none
 *
 * serialization: IREAD_LOCK(ipbmap) or IWRITE_LOCK(ipbmap) held on entry/exit;
 */
static void
dbBackSplit(dmtree_t *tp,
	    int32 leafno)
{
	int32 budsz, bud, w, bsz, size;
	int8 cursz, *leaf = tp->dmt_stree + tp->dmt_leafidx;

	/* leaf should be part (not first part) of a binary
	 * buddy system.
	 */
	assert(leaf[leafno] == NOFREE);

	/* the back split is accomplished by iteratively finding the leaf
	 * that starts the buddy system that contains the specified leaf and
	 * splitting that system in two.  this iteration continues until
	 * the specified leaf becomes the start of a buddy system. 
	 *
	 * determine maximum possible l2 size for the specified leaf.
	 */
	size = LITOL2BSZ(leafno,tp->dmt_l2nleafs,tp->dmt_budmin);

	/* determine the number of leaves covered by this size.  this
	 * is the buddy size that we will start with as we search for
	 * the buddy system that contains the specified leaf.
	 */
	budsz = BUDSIZE(size,tp->dmt_budmin);

	/* back split.
	 */
	while (leaf[leafno] == NOFREE)
	{
		/* find the leftmost buddy leaf.
		 */
		for (w = leafno, bsz = budsz; ;bsz <<= 1,
						 w = (w < bud) ? w : bud)
		{
			assert(bsz < tp->dmt_nleafs);

			/* determine the buddy.
			 */
			bud = w ^ bsz;

			/* check if this buddy is the start of the system.
			 */
			if (leaf[bud] != NOFREE)
			{
				/* split the leaf at the start of the
				 * system in two.
				 */ 
				cursz = leaf[bud] - 1;
				dbSplit(tp,bud,cursz,cursz);
				break;
			}
		}
	}

	assert(leaf[leafno] == size);
}


/*
 * NAME:	dbJoin()
 *
 * FUNCTION:    update the leaf of a dmtree with a new value, joining
 *		the leaf with other leaves of the dmtree into a multi-leaf
 *		binary buddy system, as required.
 *
 * PARAMETERS:
 *      tp	- pointer to the tree containing the leaf.
 *      leafno	- the number of the leaf to be updated.
 *      newval	- the new value for the leaf.
 *
 * RETURN VALUES: none
 */
static void
dbJoin(dmtree_t *tp,
       int32 leafno,
       int8 newval)
{
        int32 budsz, buddy;
	int8 *leaf;

	/* can the new leaf value require a join with other leaves ?
	 */
        if (newval >= tp->dmt_budmin)
        {
		/* pickup a pointer to the leaves of the tree.
		 */
		leaf = tp->dmt_stree + tp->dmt_leafidx;

		/* try to join the specified leaf into a large binary
		 * buddy system.  the join proceeds by attempting to join
		 * the specified leafno with its buddy (leaf) at new value.
		 * if the join occurs, we attempt to join the left leaf
		 * of the joined buddies with its buddy at new value + 1.
		 * we continue to join until we find a buddy that cannot be
		 * joined (does not have a value equal to the size of the
		 * last join) or until all leaves have been joined into a
		 * single system.
		 *
		 * get the buddy size (number of words covered) of
		 * the new value.
		 */
                budsz = BUDSIZE(newval,tp->dmt_budmin);

		/* try to join.
		 */
		while (budsz < tp->dmt_nleafs)
                {
			/* get the buddy leaf.
			 */
                        buddy = leafno ^ budsz;

			/* if the leaf's new value is greater than its
			 * buddy's value, we join no more.
			 */
                        if (newval > leaf[buddy])
                                break;

                        assert(newval == leaf[buddy]);

			/* check which (leafno or buddy) is the left buddy.
			 * the left buddy gets to claim the blocks resulting
			 * from the join while the right gets to claim none.
			 * the left buddy is also eligable to participate in
			 * a join at the next higher level while the right
			 * is not.
			 *
			 */
                        if (leafno < buddy)
                        {
				/* leafno is the left buddy.
				 */
                                dbAdjTree(tp,buddy,NOFREE);
                        }
                        else
                        {
				/* buddy is the left buddy and becomes
				 * leafno.
				 */
                                dbAdjTree(tp,leafno,NOFREE);
                                leafno = buddy;
                        }

			/* on to try the next join.
			 */
                        newval += 1;
		        budsz <<= 1;
                }
        }

	/* update the leaf value.
	 */
        dbAdjTree(tp,leafno,newval);
}


/*
 * NAME:	dbAdjTree()
 *
 * FUNCTION:    update a leaf of a dmtree with a new value, adjusting
 *		the dmtree, as required, to reflect the new leaf value.
 *		the combination of any buddies must already be done before
 *		this is called.
 *
 * PARAMETERS:
 *      tp	- pointer to the tree to be adjusted.
 *      leafno	- the number of the leaf to be updated.
 *      newval	- the new value for the leaf.
 *
 * RETURN VALUES: none
 */
static void
dbAdjTree(dmtree_t *tp,
	  int32 leafno,
	  int8 newval)
{
	int32 lp, pp, k;
	int8 max;

	/* pick up the index of the leaf for this leafno.
	 */
	lp = leafno + tp->dmt_leafidx;

	/* is the current value the same as the old value ?  if so,
	 * there is nothing to do.
	 */
	if (tp->dmt_stree[lp] == newval)
		return;

	/* set the new value.
	 */
	tp->dmt_stree[lp] = newval;

	/* bubble the new value up the tree as required.
	 */
	for (k = 0; k < tp->dmt_height; k++)
	{
		/* get the index of the first leaf of the 4 leaf
		 * group containing the specified leaf (leafno).
		 */
		lp = ((lp - 1) & ~0x03) + 1;

		/* get the index of the parent of this 4 leaf group.
		 */
		pp = (lp - 1) >> 2;

		/* determine the maximum of the 4 leaves.
		 */
		max = TREEMAX(&tp->dmt_stree[lp]);

		/* if the maximum of the 4 is the same as the
		 * parent's value, we're done.
		 */
		if (tp->dmt_stree[pp] == max)
			break;

		/* parent gets new value.
		 */
		tp->dmt_stree[pp] = max;

		/* parent becomes leaf for next go-round.
		 */
		lp = pp;
	}
}


/*
 * NAME:	dbFindLeaf()
 *
 * FUNCTION:    search a dmtree_t for sufficient free blocks, returning
 *		the index of a leaf describing the free blocks if 
 *		sufficient free blocks are found.
 *
 *		the search starts at the top of the dmtree_t tree and
 *		proceeds down the tree to the leftmost leaf with sufficient
 *		free space.
 *
 * PARAMETERS:
 *      tp	- pointer to the tree to be searched.
 *      l2nb	- log2 number of free blocks to search for.
 *	leafidx	- return pointer to be set to the index of the leaf
 *		  describing at least l2nb free blocks if sufficient
 *		  free blocks are found.
 *
 * RETURN VALUES:
 *      0	- success
 *      ENOSPC	- insufficient free blocks. 
 */
static int32
dbFindLeaf(dmtree_t *tp,
	   int8 l2nb,
	   int32 *leafidx)
{
	int32 ti, n, k, x;

	/* first check the root of the tree to see if there is
	 * sufficient free space.
	 */
        if (l2nb > tp->dmt_stree[ROOT])
		return(ENOSPC);

	/* sufficient free space available. now search down the tree
	 * starting at the next level for the leftmost leaf that
	 * describes sufficient free space.
	 */
        for (k = tp->dmt_height, ti = 1; k > 0; k--, ti = ((ti + n) << 2) + 1)
        {
		/* search the four nodes at this level, starting from
		 * the left.
		 */
                for (x = ti, n = 0; n < 4; n++)
		{
			/* sufficient free space found.  move to the next
			 * level (or quit if this is the last level).
			 */
                        if (l2nb <= tp->dmt_stree[x+n])
                                break;
		}

		/* better have found something since the higher
		 * levels of the tree said it was here.
		 */
                assert(n < 4);
        }

	/* set the return to the leftmost leaf describing sufficient
	 * free space.
	 */
	*leafidx = x + n - tp->dmt_leafidx;

	return(0);
}


/*
 * NAME:	dbFindBits()
 *
 * FUNCTION:    find a specified number of binary buddy free bits within a
 *		dmap bitmap word value.
 *
 *		this routine searches the bitmap value for (1 << l2nb) free
 *		bits at (1 << l2nb) alignments within the value.
 *
 * PARAMETERS:
 *      word	-  dmap bitmap word value.
 *      l2nb	-  number of free bits specified as a log2 number.
 *
 * RETURN VALUES:
 *      starting bit number of free bits.
 */
static int32
dbFindBits(uint32 word,
	   int32 l2nb)
{
	int32 bitno, nb;
	uint32 mask;

	/* get the number of bits.
	 */
	nb = 1 << l2nb;
	assert(nb <= DBWORD);

	/* complement the word so we can use a mask (i.e. 0s represent
	 * free bits) and compute the mask.
	 */
	word = ~word;
	mask = ONES << (DBWORD-nb);

	/* scan the word for nb free bits at nb alignments.
	 */
	for (bitno = 0; mask != 0; bitno += nb, mask >>= nb)
	{
		if ((mask & word) == mask)
			break;
	}

	/* return the bit number.
	 */
	return(bitno);
}


/*
 * NAME:	dbMaxBud(char *cp)
 *
 * FUNCTION:    determine the largest binary buddy string of free
 *		bits within 32-bits of the map.
 *
 * PARAMETERS:
 *      cp	-  pointer to the 32-bit value.
 *
 * RETURN VALUES:
 *      largest binary buddy of free bits within a dmap word.
 */
static int32
dbMaxBud(char *cp)
{
	/* check if the wmap word is all free. if so, the
	 * free buddy size is BUDMIN.
	 */
	if (*((uint *)cp) == 0)
		return(BUDMIN);

	/* check if the wmap word is half free. if so, the
	 * free buddy size is BUDMIN-1.
	 */
	if (*((uint16 *)cp) == 0 || *((uint16 *)cp+1) == 0)
		return(BUDMIN-1);

	/* not all free or half free. determine the free buddy
	 * size thru table lookup using quarters of the wmap word.
	 */
	return(MAX(MAX(budtab[*cp],budtab[*(cp+1)]),
			MAX(budtab[*(cp+2)],budtab[*(cp+3)])));
}


/*
 * NAME:	cnttz(uint word)
 *
 * FUNCTION:    determine the number of trailing zeros within a 32-bit
 *		value.
 *
 * PARAMETERS:
 *      value	-  32-bit value to be examined.
 *
 * RETURN VALUES:
 *      count of trailing zeros
 */
int32
cnttz(uint32 word)
{
	int32 n;

	for (n = 0; n < BITSPERINT32; n++, word >>= 1)
	{
		if (word & 0x01)
			break;
	}

	return(n);
}


/*
 * NAME:	cntlz(uint32 value)
 *
 * FUNCTION:    determine the number of leading zeros within a 32-bit
 *		value.
 *
 * PARAMETERS:
 *      value	-  32-bit value to be examined.
 *
 * RETURN VALUES:
 *      count of leading zeros
 */
int32
cntlz(uint32 value)
{
	int32 n;

	for (n = 0; n < BITSPERINT32; n++, value <<= 1)
	{
		if (value & HIGHORDER)
			break;
	}
	return(n);
}


/*
 * NAME:	blkstol2(int64 nb)
 *
 * FUNCTION:	convert a block count to its log2 value. if the block
 *	        count is not a l2 multiple, it is rounded up to the next
 *		larger l2 multiple.
 *
 * PARAMETERS:
 *      nb	-  number of blocks
 *
 * RETURN VALUES:
 *      log2 number of blocks
 */
int32
blkstol2(int64 nb)
{
	int32 l2nb;
	int64 mask;	/* meant to be signed */

	mask = (int64)1 << (BITSPERINT64-1);

	/* count the leading bits.
	 */
	for (l2nb = 0; l2nb < BITSPERINT64; l2nb++, mask >>= 1)
	{
		/* leading bit found.
		 */
		if (nb & mask)
		{
			/* determine the l2 value.
			 */
			l2nb = (BITSPERINT64-1) - l2nb;

			/* check if we need to round up.
			 */
			if (~mask & nb)
				l2nb++;

			return(l2nb);
		}
	}
	assert(0);
}


/*
 * NAME:	fsDirty()
 *
 * FUNCTION:    xxx
 *
 * PARAMETERS:
 *      ipmnt	- mount inode
 *
 * RETURN VALUES:
 *      none
 */
void fsDirty()
{
	printf("fsDirty(): bye-bye\n");
	assert(0);
}


/*
 * NAME:    	dbAllocBottomUp()
 *
 * FUNCTION:	alloc the specified block range from the working block
 *		allocation map.
 *
 *		the blocks will be alloc from the working map one dmap
 *		at a time.
 *
 * PARAMETERS:
 *      ip	-  pointer to in-core inode;
 *      blkno	-  starting block number to be freed.
 *      nblocks	-  number of blocks to be freed.
 *
 * RETURN VALUES:
 *      0	- success
 *      EIO	- i/o error
 */
int32 dbAllocBottomUp(
	inode_t	*ip,
	int64	blkno,
	int64	nblocks)
{
	jbuf_t *bp;
	dmap_t *dp;
	int32 nb, rc;
	int64	lblkno, rem;
	inode_t	*ipbmap = ip->i_ipmnt->i_ipbmap;
	bmap_t *mp = ipbmap->i_bmap;

	IREAD_LOCK(ipbmap);

	/* block to be allocated better be within the mapsize. */
	ASSERT(nblocks <= mp->db_mapsize - blkno);

	/*
	 * allocate the blocks a dmap at a time.
	 */
	bp = NULL;
	for (rem = nblocks; rem > 0; rem -= nb, blkno += nb)
	{
		/* release previous dmap if any */
		if (bp)
			bmLazyWrite(bp, 0);

		/* get the buffer for the current dmap. */
        	lblkno = BLKTODMAP(blkno,mp->db_l2nbperpage);
        	if (bmRead(ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		{
			IREAD_UNLOCK(ipbmap);
                	return(EIO);
		}
		dp = (dmap_t *)bp->b_bdata;

		/* determine the number of blocks to be allocated from
		 * this dmap.
		 */
		nb = MIN(rem, BPERDMAP - (blkno & (BPERDMAP-1)));

		DBFREECK(mp->db_DBmap, mp->db_mapsize, blkno, nb);

		/* allocate the blocks. */
        	if (rc = dbAllocDmapBU(mp,dp,blkno,nb))
		{
			bmRelease(bp);
			IREAD_UNLOCK(ipbmap);
			return(rc);
		}

		DBALLOC(mp->db_DBmap, mp->db_mapsize, blkno, nb);
	}

	/* write the last buffer. */
	bmLazyWrite(bp, 0);

	IREAD_UNLOCK(ipbmap);

	return(0);
}


static int32 dbAllocDmapBU(
	bmap_t	*mp,
	dmap_t	*dp,
	int64	blkno,
	int32	nblocks)
{
	int32	rc;
	int32	dbitno, word, rembits, nb, nwords, wbitno, agno;
	int8	oldroot, *leaf;
	dmaptree_t *tp = (dmaptree_t *)&dp->tree;

	/* save the current value of the root (i.e. maximum free string)
	 * of the dmap tree.
	 */
	oldroot = tp->stree[ROOT];

        /* pick up a pointer to the leaves of the dmap tree */
	leaf = tp->stree + LEAFIND;

        /* determine the bit number and word within the dmap of the
         * starting block.
         */
	dbitno = blkno & (BPERDMAP - 1);
	word = dbitno >> L2DBWORD;

	/* block range better be within the dmap */ 
	assert(dbitno + nblocks <= BPERDMAP);

        /* allocate the bits of the dmap's words corresponding to the block
	 * range. not all bits of the first and last words may be contained
	 * within the block range.  if this is the case, we'll work against
	 * those words (i.e. partial first and/or last) on an individual basis
	 * (a single pass), allocating the bits of interest by hand and
	 * updating the leaf corresponding to the dmap word. a single pass
	 * will be used for all dmap words fully contained within the
	 * specified range.  within this pass, the bits of all fully contained
	 * dmap words will be marked as free in a single shot and the leaves
	 * will be updated. a single leaf may describe the free space of
	 * multiple dmap words, so we may update only a subset of the actual
	 * leaves corresponding to the dmap words of the block range.
	 */
	for (rembits = nblocks; rembits > 0; rembits -= nb, dbitno += nb)
	{
                /* determine the bit number within the word and
                 * the number of bits within the word.
                 */
		wbitno = dbitno & (DBWORD-1);
		nb = MIN(rembits, DBWORD - wbitno);

		/* check if only part of a word is to be allocated.
                 */
		if (nb < DBWORD)
		{
                        /* allocate (set to 1) the appropriate bits within
                         * this dmap word.
                         */
			dp->wmap[word] |= (ONES << (DBWORD - nb) >> wbitno);

			word += 1;
		}
		else
		{
                        /* one or more dmap words are fully contained
                         * within the block range.  determine how many
                         * words and allocate (set to 1) the bits of these
			 * words.
                         */
			nwords = rembits >> L2DBWORD;
			memset(&dp->wmap[word], ONES, nwords*4);

                        /* determine how many bits */
			nb = nwords << L2DBWORD;
		}
	}

	/* update the free count for this dmap */
	dp->nfree -= nblocks;

	/* reconstruct summary tree */
	dbInitDmapTree(dp);

	BMAP_LOCK(mp);

	/* if this allocation group is completely free,
	 * update the highest active allocation group number 
	 * if this allocation group is the new max.
	 */
	agno = blkno >> mp->db_agl2size;
	if (agno > mp->db_maxag)
		mp->db_maxag = agno;

	/* update the free count for the allocation group and map */
	mp->db_agfree[agno] -= nblocks;
	mp->db_nfree -= nblocks;

	BMAP_UNLOCK(mp);

	/* if the root has not changed, done. */
	if (tp->stree[ROOT] == oldroot)
		return(0);

	/* root changed. bubble the change up to the dmap control pages.
	 * if the adjustment of the upper level control pages fails,
	 * backout the bit allocation (thus making everything consistent).
	 */
	if (rc = dbAdjCtl(mp,blkno,tp->stree[ROOT],1,0))
		dbFreeBits(mp,dp,blkno,nblocks);

	return(rc);
}


/*
 * NAME:	dbExtendFS()
 *
 * FUNCTION:	extend bmap from blkno for nblocks;
 * 		dbExtendFS() updates bmap ready for dbAllocBottomUp();
 *
 * L2
 *  |
 *   L1---------------------------------L1
 *    |                                  |
 *     L0---------L0---------L0           L0---------L0---------L0
 *      |          |          |            |          |          |
 *       d0,...,dn  d0,...,dn  d0,...,dn    d0,...,dn  d0,...,dn  d0,.,dm;
 * L2L1L0d0,...,dnL0d0,...,dnL0d0,...,dnL1L0d0,...,dnL0d0,...,dnL0d0,..dm
 *
 * <---old---><----------------------------extend----------------------->   
 */
int32 dbExtendFS(
	inode_t	*ipbmap, 
	int64	blkno,		/* start block number */ 
	int64	nblocks)	/* number of blocks */
{
	int32	rc;
	inode_t	*ipmnt = ipbmap->i_ipmnt;
	int32	nbperpage = ipmnt->i_nbperpage;
	int32	i, i0 = TRUE, j, j0 = TRUE, k, n;
	int64	newsize;
	int64	p;
	jbuf_t	*bp, *l2bp, *l1bp, *l0bp;
	dmapctl_t	*l2dcp, *l1dcp, *l0dcp;
	dmap_t	*dp;
	int8	*l0leaf, *l1leaf, *l2leaf;
	uint8	xflag;
	int32	xlen;
	int64	xaddr;
	bmap_t	*mp = ipbmap->i_bmap;
	int32	agno, l2agsize, oldl2agsize;
	int32	actags, inactags, l2nl;
	int64	ag_rem, actfree, inactfree, avgfree;

	newsize = blkno + nblocks;

jEVENT(0,("dbExtendFS: blkno:%lld nblocks:%lld newsize:%lld\n", 
	blkno, nblocks, newsize));

	/*
	 *	initialize bmap control page.
	 *
	 * all the data in bmap control page should exclude
	 * the mkfs hidden dmap page.
	 */
#ifdef	_JFS_FYI
	int64		dn_mapsize;	/* 8: number of blocks in aggregate  */
	int64		dn_nfree;	/* 8: num free blks in aggregate map */
	int32		dn_l2nbperpage;	/* 4: number of blks per page */
	int32		dn_numag;	/* 4: total number of ags            */
	int32		dn_maxlevel;	/* 4: max level number */
	int32		dn_maxag;	/* 4: max active alloc group number  */
	int32		dn_agpref;	/* 4: preferred alloc group (hint)   */
	int32		dn_aglevel;	/* 4: dmapctl level holding the AG   */
	int32		dn_agheigth;	/* 4: height in dmapctl of the AG    */
	int32		dn_agwidth;	/* 4: width in dmapctl of the AG     */
	int32		dn_agstart;	/* 4: start tree index at AG height  */
	int32		dn_agl2size;	/* 4: l2 num of blks per alloc group */
	int64		dn_agfree[MAXAG];  /* 8*MAXAG: per AG free count     */
	int64		dn_agsize;	/* 8: num of blks per alloc group    */
	int8		dn_maxfreebud;	/* 1: max free buddy system	     */
	uint8		pad[3007];	/* 3007: pad to 4096                 */
#endif	/* _JFS_FYI */
/*
printf("bmap: mapsize:%lld nfree:%lld maxlevel:%d maxfreebud:%d\n",
	mp->db_mapsize, mp->db_nfree, mp->db_maxlevel, mp->db_maxfreebud);
printf("bmap: agsize:%lld nag:%d maxag:%d\n",
	mp->db_agsize, mp->db_numag, mp->db_maxag);
printf("bmap: agpref:%d aglevel:%d agheigth:%d agwidth:%d\n",
	mp->db_agpref, mp->db_aglevel, mp->db_agheigth, mp->db_agwidth);

for (i = 0; i < mp->db_numag; i++)
	printf("agfree[%d]:%lld\n", i, mp->db_agfree[i]);

printf("sbh_dmap: db_mapsize: %lld  newsize: %lld\n", mp->db_mapsize, newsize );
*/

	/* update mapsize */
	mp->db_mapsize = newsize;
	mp->db_maxlevel = BMAPSZTOLEV(mp->db_mapsize);

	/* compute new AG size */
	l2agsize = dbGetL2AGSize(newsize);
	oldl2agsize = mp->db_agl2size;

	mp->db_agl2size = l2agsize;
	mp->db_agsize = 1 << l2agsize;

	/* compute new number of AG */
	agno = mp->db_numag;
	mp->db_numag = newsize >> l2agsize;
	mp->db_numag += (newsize % mp->db_agsize) ? 1 : 0;

	/*
	 *	reconfigure db_agfree[] 
	 * from old AG configuration to new AG configuration;
	 *
	 * coalesce contiguous k (newAGSize/oldAGSize) AGs;
	 * i.e., (AGi, ..., AGj) where i = k*n and j = k*(n+1) - 1 to AGn;
	 * note: new AG size = old AG size * (2**x).
	 */
	if (l2agsize == oldl2agsize)
		goto extend;
	k = 1 << (l2agsize - oldl2agsize);
	ag_rem = mp->db_agfree[0]; /* save agfree[0] */
	for (i = 0, n = 0; i < agno; n++)
	{
		mp->db_agfree[n] = 0; /* init collection point */

		/* coalesce cotiguous k AGs; */
		for (j = 0; j < k && i < agno; j++, i++)
		{
			/* merge AGi to AGn */
			mp->db_agfree[n] += mp->db_agfree[i];
		}	
	}
	mp->db_agfree[0] += ag_rem; /* restore agfree[0] */

	for ( ; n < MAXAG; n++)
		mp->db_agfree[n] = 0;

	/*
	 * update highest active ag number
	 */
#ifdef	_JFS_FYI
	/*
	 * compute db_maxag: highest active ag number 
	 * (the rightmost allocation group with blocks allocated in it);
	 */
	/* get last ag number: assert(mp->db_numag >= 1); */
	i = mp->db_numag - 1;

	/* is last ag partial ag ? */
	ag_rem = mp->db_mapsize & (mp->db_agsize - 1);
	if (ag_rem)
	{
		/* is last ag active ? */
		if (mp->db_agfree[i] < ag_rem)
		{
			mp->db_maxag = i;
			goto agpref;
		}	
		else
			i--;
	}

	/* scan backward for first ag with blocks allocated:
	 * (ag0 must be active from allocation of map itself)
	 */
	for ( ; i >= 0; i--)
	{
		if (mp->db_agfree[i] < mp->db_agsize)
			break;
	}
	mp->db_maxag = i;
#endif	/* _JFS_FYI */

	mp->db_maxag = mp->db_maxag / k;

/*
printf("bmapx: agsize:%lld nag:%d maxag:%d\n",
	mp->db_agsize, mp->db_numag, mp->db_maxag);
for (i = 0; i < mp->db_numag; i++)
	printf("agfree[%d]:%lld\n", i, mp->db_agfree[i]);
*/

	/*
	 *	extend bmap
	 *
	 * update bit maps and corresponding level control pages;
	 * global control page db_nfree, db_agfree[agno], db_maxfreebud;
	 */
extend:
	/* get L2 page */
	p = BMAPBLKNO + nbperpage; /* L2 page */
	rc = bmRead(ipbmap, p, PSIZE, bmREAD_PAGE, &l2bp);
	l2dcp = (dmapctl_t *)l2bp->b_bdata;
/*
printf("L2: p:%lld\n", p); 
*/

	/* compute start L1 */
	k = blkno >> L2MAXL1SIZE;
	l2leaf = l2dcp->stree + CTLLEAFIND + k;
	p = BLKTOL1(blkno, ipmnt->i_l2nbperpage); /* L1 page */

	/*
	 * extend each L1 in L2
	 */
	for ( ; k < LPERCTL; k++, p += nbperpage)
	{	
/*
printf("<L1: k:%d p:%lld blkno:%lld\n", k, p, blkno); 
*/
		/* get L1 page */
		if (j0)
		{
			/* read in L1 page: (blkno & (MAXL1SIZE - 1)) */
			if (bmRead(ipbmap, p, PSIZE, bmREAD_PAGE, &l1bp))
				goto errout;
			l1dcp = (dmapctl_t *)l1bp->b_bdata;

			/* compute start L0 */
			j = (blkno & (MAXL1SIZE - 1)) >> L2MAXL0SIZE;
			l1leaf = l1dcp->stree + CTLLEAFIND + j;
			p = BLKTOL0(blkno, ipmnt->i_l2nbperpage);
			j0 = FALSE;
		}
		else
		{
			/* assign/init L1 page */
			if (xtLookup(ipbmap, p, nbperpage, &xflag, &xaddr, &xlen, 0))
				goto errout;
			l1bp = bmAssign(ipbmap, p, xaddr, PSIZE, bmREAD_PAGE);
			l1dcp = (dmapctl_t *)l1bp->b_bdata;

			/* compute start L0 */
			j = 0;
			l1leaf = l1dcp->stree + CTLLEAFIND;
			p += nbperpage; /* 1st L0 of L1.k  */
		}

		/*
		 * extend each L0 in L1
		 */
		for ( ; j < LPERCTL; j++)
		{
/*
printf("<L0: j:%d p:%lld blkno:%lld\n", j, p, blkno); 
*/
			/* get L0 page */
			if (i0)
			{
				/* read in L0 page: (blkno & (MAXL0SIZE - 1)) */

				if (bmRead(ipbmap, p, PSIZE, bmREAD_PAGE, &l0bp))
					goto errout;
				l0dcp = (dmapctl_t *)l0bp->b_bdata;

				/* compute start dmap */
				i = (blkno & (MAXL0SIZE - 1)) >> L2BPERDMAP;
				l0leaf = l0dcp->stree + CTLLEAFIND + i;
				p = BLKTODMAP(blkno, ipmnt->i_l2nbperpage);
				i0 = FALSE;
			}
			else
			{
				/* assign/init L0 page */
				if (xtLookup(ipbmap, p, nbperpage, &xflag, &xaddr, &xlen, 0))
					goto errout;
				l0bp = bmAssign(ipbmap, p, xaddr, PSIZE, bmREAD_PAGE);
				l0dcp = (dmapctl_t *)l0bp->b_bdata;

				/* compute start dmap */
				i = 0;
				l0leaf = l0dcp->stree + CTLLEAFIND;
				p += nbperpage; /* 1st dmap of L0.j */ 
			}

			/*
			 * extend each dmap in L0
			 */
                       	for ( ; i < LPERCTL; i++)
                       	{
				/*
				 * reconstruct the dmap page, and
				 * initialize corresponding parent L0 leaf
				 */
				if (n = blkno & (BPERDMAP - 1))
				{
/*
printf("sbh_dmap:  i:%d p:%lld blkno:%lld read dmap page\n", i, p, blkno); 
*/
					/* read in dmap page: */
					if (bmRead(ipbmap, p, PSIZE, bmREAD_PAGE, &bp))
						goto errout;
					n = MIN(nblocks, BPERDMAP - n);
				}
				else
				{
/*
printf("sbh_dmap:  i:%d p:%lld blkno:%lld create new dmap page\n", i, p, blkno); 
*/
					/* assign/init dmap page */
					if (xtLookup(ipbmap, p, nbperpage, &xflag, &xaddr, &xlen, 0))
						goto errout;
					bp = bmAssign(ipbmap, p, xaddr, PSIZE, bmREAD_PAGE);
					n = MIN(nblocks, BPERDMAP);
				}

				dp = (dmap_t *)bp->b_bdata;
				*l0leaf = dbInitDmap(dp, blkno, n);
/*
printf("maxfreebud l0leaf:%d\n", *l0leaf);
*/

				mp->db_nfree += n;
				agno = dp->start >> l2agsize;
				mp->db_agfree[agno] += n;

				bmLazyWrite(bp, 0);

               			l0leaf++;
				p += nbperpage;

				blkno += n;
				nblocks -= n;
				if (nblocks == 0)
					break;
                       	} /* for each dmap in a L0 */
/*
printf(">L0: j:%d i:%d p:%lld blkno:%lld\n", j, i, p, blkno); 
*/

			/*
			 * build current L0 page from its leaves, and 
			 * initialize corresponding parent L1 leaf
			 */
			*l1leaf = dbInitDmapCtl(l0dcp, 0, ++i);
			bmLazyWrite(l0bp, 0);
/*
printf("maxfreebud l1leaf %d:%d\n", j, *l1leaf);
*/

			if (nblocks)
				l1leaf++; /* continue for next L0 */
			else
			{
				/* more than 1 L0 ? */
				if (j > 0)
					break; /* build L1 page */
				else
				{
					/* summarize in global bmap page */
					mp->db_maxfreebud = *l1leaf;
					bmRelease(l1bp);
					bmRelease(l2bp);
					goto finalize;
				}
			}
		} /* for each L0 in a L1 */
/*
printf(">L1: k:%d j:%d p:%lld blkno:%lld\n", k, j, p, blkno); 
*/

		/*
		 * build current L1 page from its leaves, and 
		 * initialize corresponding parent L2 leaf
		 */
		*l2leaf = dbInitDmapCtl(l1dcp, 1, ++j);
		bmLazyWrite(l1bp, 0);
/*
printf("maxfreebud l2leaf %d:%d\n", k, *l2leaf);
*/

		if (nblocks)
			l2leaf++; /* continue for next L1 */
		else
		{
			/* more than 1 L1 ? */
			if (k > 0)
				break; /* build L2 page */
			else
			{
				/* summarize in global bmap page */
				mp->db_maxfreebud = *l2leaf; 
				bmRelease(l2bp);
				goto finalize;
			}
		}
	} /* for each L1 in a L2 */

	assert(0);

	/*
	 *	finalize bmap control page
	 */
finalize:
/*
printf("bmap: mapsize:%lld nfree:%lld maxlevel:%d maxfreebud:%d\n",
	mp->db_mapsize, mp->db_nfree, mp->db_maxlevel, mp->db_maxfreebud);
printf("bmap: agsize:%lld nag:%d maxag:%d\n",
	mp->db_agsize, mp->db_numag, mp->db_maxag);
for (i = 0; i < mp->db_numag; i++)
	printf("agfree[%d]:%lld\n", i, mp->db_agfree[i]);
*/

	return 0;

errout:
	return EIO;
}


/*
 *	dbFinalizeBmap()
 */
int32 dbFinalizeBmap(
	inode_t	*ipbmap)
{
	int32	rc;
	bmap_t	*mp = ipbmap->i_bmap;
	int32	actags, inactags, l2nl;
	int64	ag_rem, actfree, inactfree, avgfree;
	int32	i, n;

	/*
	 *	finalize bmap control page
	 */
finalize:
	/* 
	 * compute db_agpref: preferred ag to allocate from
	 * (the leftmost ag with average free space in it);
	 */
agpref:
	/* get the number of active ags and inacitve ags */
	actags = mp->db_maxag+1;
	inactags = mp->db_numag - actags;

	/* determine how many blocks are in the inactive allocation
	 * groups. in doing this, we must account for the fact that
	 * the rightmost group might be a partial group (i.e. file
	 * system size is not a multiple of the group size).
	 */
	inactfree = (inactags && ag_rem) ? 
			((inactags-1) << mp->db_agl2size) + ag_rem 
			: inactags << mp->db_agl2size;

	/* determine how many free blocks are in the active
	 * allocation groups plus the average number of free blocks
	 * within the active ags.
	 */
	actfree = mp->db_nfree - inactfree;
	avgfree = actfree / actags;

	/* if the preferred allocation group has not average free space.
	 * re-establish the preferred group as the leftmost
	 * group with average free space.
	 */
	if (mp->db_agfree[mp->db_agpref] < avgfree)
	{
		for (mp->db_agpref = 0; mp->db_agpref < actags;
		     mp->db_agpref++)
		{
			if (mp->db_agfree[mp->db_agpref] >= avgfree)
				break;
		}
		assert( mp->db_agpref < mp->db_numag );
	}

	/*
	 * compute db_aglevel, db_agheigth, db_width, db_agstart:
	 * an ag is covered in aglevel dmapctl summary tree, 
	 * at agheight level height (from leaf) with agwidth number of nodes 
	 * each, which starts at agstart index node of the smmary tree node 
	 * array;
	 */
	mp->db_aglevel = BMAPSZTOLEV(mp->db_agsize);
	l2nl = mp->db_agl2size - (L2BPERDMAP + mp->db_aglevel * L2LPERCTL);
	mp->db_agheigth = l2nl >> 1;
	mp->db_agwidth = 1 << (l2nl - (mp->db_agheigth << 1));
	for (i = 5 - mp->db_agheigth, mp->db_agstart=0, n = 1; i > 0; i--) 
	{
		mp->db_agstart += n;
		n <<= 2;
	}

/*
printf("bmap: agpref:%d aglevel:%d agheigth:%d agwidth:%d\n",
	mp->db_agpref, mp->db_aglevel, mp->db_agheigth, mp->db_agwidth);
*/
}


/*
 * NAME:	dbInitDmap()/ujfs_idmap_page()
 *                                                                    
 * FUNCTION:	initialize working/persistent bitmap of the dmap page
 *		for the specified number of blocks:
 *                                                                    
 *		at entry, the bitmaps had been initialized as free (ZEROS);
 *		The number of blocks will only account for the actually 
 *		existing blocks. Blocks which don't actually exist in 
 *		the aggregate will be marked as allocated (ONES);
 *
 * PARAMETERS:
 *	dp	- pointer to page of map
 *	nblocks	- number of blocks this page
 *
 * RETURNS: NONE
 */
static int8 dbInitDmap(dmap_t	*dp,
	int64	Blkno,
	int32	nblocks)
{
	int32	blkno, w, b, r, nw, nb, i;
/*
printf("sbh_dmap:  in dbInitDmap blkno:%lld nblocks:%ld\n", Blkno, nblocks); 
*/

        /* starting block number within the dmap */
	blkno = Blkno & (BPERDMAP - 1);

	if (blkno == 0)
	{
		dp->nblocks = dp->nfree = nblocks;
		dp->start = Blkno;

		if (nblocks == BPERDMAP)
		{
			memset(&dp->wmap[0], 0, LPERDMAP*4);
			memset(&dp->pmap[0], 0, LPERDMAP*4);
			goto initTree;
		}
	}
	else
	{
		dp->nblocks += nblocks;
		dp->nfree += nblocks;
	}

	/* word number containing start block number */
	w = blkno >> L2DBWORD;

        /*
	 * free the bits corresponding to the block range (ZEROS):
	 * note: not all bits of the first and last words may be contained 
	 * within the block range.
	 */
	for (r = nblocks; r > 0; r -= nb, blkno += nb)
	{
		/* number of bits preceding range to be freed in the word */
		b = blkno & (DBWORD-1);
		/* number of bits to free in the word */
		nb = MIN(r, DBWORD - b);

		/* is partial word to be freed ? */
		if (nb < DBWORD)
		{
			/* free (set to 0) from the bitmap word */
			dp->wmap[w] &= ~(ONES << (DBWORD - nb) >> b);
			dp->pmap[w] &= ~(ONES << (DBWORD - nb) >> b);

			/* skip the word freed */
			w++;
		}
		else
		{
			/* free (set to 0) contiguous bitmap words */
			nw = r >> L2DBWORD;
			memset(&dp->wmap[w], 0, nw*4);
			memset(&dp->pmap[w], 0, nw*4);

			/* skip the words freed */
			nb = nw << L2DBWORD;
			w += nw;
		}
	}

	/*
	 * mark bits following the range to be freed (non-existing 
	 * blocks) as allocated (ONES)
	 */
/*
printf("sbh_dmap:  in dbInitDmap, preparing to mark unbacked, blkno:%ld nblocks:%ld\n",
		blkno, nblocks); 
*/

	if (blkno == BPERDMAP)
		goto initTree;

	/* the first word beyond the end of existing blocks */
	w = blkno >> L2DBWORD;

	/* does nblocks fall on a 32-bit boundary ? */
	b = blkno & (DBWORD-1);
/*
printf("sbh_dmap:  in dbInitDmap, b:%ld w:%ld mask: %lx\n", b, w, (ONES>>b)); 
*/
	if (b)
	{
		/* mark a partial word allocated */
		dp->wmap[w] = dp->pmap[w] = ONES >> b;
		w++;
	}

	/* set the rest of the words in the page to allocated (ONES) */
	for (i = w; i < LPERDMAP; i++)
		dp->pmap[i] = dp->wmap[i] = ONES;

	/*
	 * init tree
	 */
initTree:
	return(dbInitDmapTree(dp));
}


/*
 * NAME:	dbInitDmapTree()/ujfs_complete_dmap()
 *                                                                    
 * FUNCTION:	initialize summary tree of the specified dmap:
 *
 *		at entry, bitmap of the dmap has been initialized;
 *                                                                    
 * PARAMETERS:
 *	dp	- dmap to complete
 *	blkno	- starting block number for this dmap
 *	treemax	- will be filled in with max free for this dmap
 *
 * RETURNS:	max free string at the root of the tree
 */
static int8 dbInitDmapTree(dmap_t	*dp)
{
	dmaptree_t	*tp;
	int8	*cp;
	int32	i;

	/* init fixed info of tree */
	tp = &dp->tree;
	tp->nleafs = LPERDMAP;
	tp->l2nleafs = L2LPERDMAP;
	tp->leafidx = LEAFIND;
	tp->height = 4;
	tp->budmin = BUDMIN;

	/* init each leaf from corresponding wmap word:
	 * note: leaf is set to NOFREE(-1) if all blocks of corresponding
	 * bitmap word are allocated. 
	 */
	cp = tp->stree + tp->leafidx;
	for (i = 0; i < LPERDMAP; i++)
		*cp++ = dbMaxBud((char *)&dp->wmap[i]);

	/* build the dmap's binary buddy summary tree */
	return(dbInitTree(tp));
}


/*
 * NAME:	dbInitTree()/ujfs_adjtree()
 *                                                                    
 * FUNCTION:	initialize binary buddy summary tree of a dmap or dmapctl.
 *
 *		at entry, the leaves of the tree has been initialized 
 *		from corresponding bitmap word or root of summary tree
 *		of the child control page;
 *		configure binary buddy system at the leaf level, then
 *		bubble up the values of the leaf nodes up the tree.
 *
 * PARAMETERS:
 *	cp	- Pointer to the root of the tree
 *	l2leaves- Number of leaf nodes as a power of 2
 *	l2min	- Number of blocks that can be covered by a leaf
 *		  as a power of 2
 *
 * RETURNS: max free string at the root of the tree
 */
static int8 dbInitTree(dmaptree_t	*dtp)
{
	int32	l2max, l2free, bsize, nextb, i;
	int32	child, parent, nparent;
	int8	*tp, *cp, *cp1;

	tp = dtp->stree;

	/* Determine the maximum free string possible for the leaves */
	l2max = dtp->l2nleafs + dtp->budmin;

	/*
	 * configure the leaf levevl into binary buddy system
	 *
	 * Try to combine buddies starting with a buddy size of 1 
	 * (i.e. two leaves). At a buddy size of 1 two buddy leaves 
	 * can be combined if both buddies have a maximum free of l2min; 
	 * the combination will result in the left-most buddy leaf having 
	 * a maximum free of l2min+1.  
	 * After processing all buddies for a given size, process buddies 
	 * at the next higher buddy size (i.e. current size * 2) and 
	 * the next maximum free (current free + 1).  
	 * This continues until the maximum possible buddy combination 
	 * yields maximum free.
	 */
	for (l2free = dtp->budmin, bsize = 1; l2free < l2max; l2free++, bsize = nextb)
	{
		/* get next buddy size == current buddy pair size */
		nextb = bsize << 1;

		/* scan each adjacent buddy pair at current buddy size */
		for (i = 0, cp = tp + dtp->leafidx; 
		     i < dtp->nleafs;
		     i += nextb, cp += nextb )
		{
			/* coalesce if both adjacent buddies are max free */ 
			if (*cp == l2free && *(cp + bsize) == l2free)
			{
				*cp = l2free + 1; /* left take right */
				*(cp + bsize) = -1; /* right give left */ 
			}
		}
	}

	/*
	 * bubble summary information of leaves up the tree.
	 *
	 * Starting at the leaf node level, the four nodes described by
	 * the higher level parent node are compared for a maximum free and 
	 * this maximum becomes the value of the parent node.  
	 * when all lower level nodes are processed in this fashion then 
	 * move up to the next level (parent becomes a lower level node) and 
	 * continue the process for that level.
	 */
	for (child = dtp->leafidx, nparent = dtp->nleafs >> 2; 
	     nparent > 0;
	     nparent >>= 2, child = parent)
	{
		/* get index of 1st node of parent level */
		parent = (child - 1) >> 2;

		/* set the value of the parent node as the maximum 
		 * of the four nodes of the current level.
		 */
		for (i = 0, cp = tp + child, cp1 = tp + parent;
		     i < nparent; 
		     i++, cp += 4, cp1++)
			*cp1 = TREEMAX(cp);
	}

	return(*tp);
}


/*
 *	dbInitDmapCtl()
 *
 * function: initialize dmapctl page
 */
static dbInitDmapCtl(
	dmapctl_t	*dcp,
	int32		level,
	int32		i)	/* start leaf index not covered by range */
{
	int8	*cp;

	dcp->nleafs = LPERCTL;
	dcp->l2nleafs = L2LPERCTL;
	dcp->leafidx = CTLLEAFIND;
	dcp->height = 5;
	dcp->budmin = L2BPERDMAP + L2LPERCTL * level;

	/*
	 * initialize the leaves of current level that were not covered 
	 * by the specified input block range (i.e. the leaves have no 
	 * low level dmapctl or dmap).
	 */
	cp = &dcp->stree[CTLLEAFIND + i];
	for ( ; i < LPERCTL; i++)
		*cp++ = NOFREE;

	/* build the dmap's binary buddy summary tree */
	return(dbInitTree((dmaptree_t *)dcp));
}


/*
 * NAME:	dbGetL2AGSize()/ujfs_getagl2size()
 *                                                                    
 * FUNCTION:	Determine log2(allocation group size) from aggregate size
 *                                                                    
 * PARAMETERS:
 *	nblocks	- Number of blocks in aggregate
 *
 * RETURNS: log2(allocation group size) in aggregate blocks
 */
static int32 dbGetL2AGSize(
	int64	nblocks)
{
	int64	sz;
	int64	m;
	int32	l2sz;

	if (nblocks < BPERDMAP * MAXAG)
		return(L2BPERDMAP);

	/* round up aggregate size to power of 2 */
	m = ((uint64)1 << (BITSPERINT64-1));
	for (l2sz = BITSPERINT64; l2sz >= 0; l2sz--, m >>= 1)
	{
		if (m & nblocks)
			break;
	}

	sz = (int64)1 << l2sz;
	if (sz < nblocks)
		l2sz += 1;

	/* agsize = roundupSize/max_number_of_ag */
	return(l2sz - L2MAXAG);
}


/*
 * NAME:	dbMapFileSizeToMapSize()
 *                                                                    
 * FUNCTION:	compute number of blocks the block allocation map file 
 *		can cover from the map file size;
 *
 * RETURNS:	Number of blocks which can be covered by this block map file;
 */

/*
 * maximum number of map pages at each level including control pages
 */
#define MAXL0PAGES	(1 + LPERCTL)
#define MAXL1PAGES	(1 + LPERCTL * MAXL0PAGES)
#define MAXL2PAGES	(1 + LPERCTL * MAXL1PAGES)

/*
 * convert number of map pages to the zero origin top dmapctl level
 */
#define BMAPPGTOLEV(npages)	\
	(((npages) <= 3 + MAXL0PAGES) ? 0 \
       : ((npages) <= 2 + MAXL1PAGES) ? 1 : 2)

int64 dbMapFileSizeToMapSize(
	inode_t	*ipbmap)
{
	inode_t	*ipmnt = ipbmap->i_ipmnt;
	int64	nblocks;
	int64	npages, ndmaps;
	int32	level, i;
	int32	complete, factor;

	nblocks = ipbmap->i_size >> ipmnt->i_l2bsize;
	npages = nblocks >> ipmnt->i_l2nbperpage;
	level = BMAPPGTOLEV(npages);

	/* At each level, accumulate the number of dmap pages covered by 
	 * the number of full child levels below it;
	 * repeat for the last incomplete child level.
	 */
	ndmaps = 0;
	npages--; /* skip the first global control page */
	/* skip higher level control pages above top level covered by map */
	npages -= (2 - level);
	npages--; /* skip top level's control page */
	for (i = level; i >= 0; i--)
	{
		factor = (i == 2) ? MAXL1PAGES : ((i == 1) ? MAXL0PAGES : 1);
		complete = npages / factor;
		ndmaps += complete * ((i == 2) ? LPERCTL*LPERCTL
				    : ((i == 1) ? LPERCTL : 1));

		npages = npages % factor; /* pages in last/incomplete child */
		npages--; /* skip incomplete child's level control page */
	}

	/* convert the number of dmaps into the number of blocks 
	 * which can be covered by the dmaps;
	 */
	nblocks = ndmaps << L2BPERDMAP;

	return(nblocks);
}


#ifdef	_JFS_DEBUG_DMAP
/*
 *	DBinitmap()
 */
void DBinitmap( int64	size,
		inode_t	*ipbmap,
		uint32	**results )
{
	int64 npages;
	uint32 *dbmap, *d;
	int32 n;
	int64	lblkno, cur_block;
	dmap_t	*dp;
	jbuf_t	*bp;

	npages = size / 32768;
	npages += (size % 32768) ? 1 : 0;

        if ((dbmap = (uint32 *)xmalloc((int32)(npages * 4096), L2PSIZE,
                                        kernel_heap)) == NULL)
		assert(0);

	for (n = 0, d = dbmap; n < npages; n++, d += 1024)
		bzero(d,4096);

	/* Need to initialize from disk map pages
	 */
	for (d = dbmap, cur_block = 0; cur_block < size;
		cur_block += BPERDMAP, d+= LPERDMAP)
	{
		lblkno = BLKTODMAP(cur_block, ipbmap->i_bmap->db_l2nbperpage);
		if (bmRead(ipbmap, lblkno, PSIZE, bmREAD_PAGE, &bp))
		{
			assert(0);
		}
		dp = (dmap_t *) bp->b_bdata;

		bcopy(dp->wmap, d, LPERDMAP * sizeof(dp->wmap[0]));

		bmRelease(bp);
	}

	*results = dbmap;
}


/*
 *	DBAlloc()
 */
void DBAlloc( uint *dbmap,
	      int64 mapsize,
	      int64 blkno,
	      int64 nblocks )
{
	int32 word, nb, bitno;
	uint32 mask;

	assert(blkno > 0 && blkno < mapsize);
	assert(nblocks > 0 && nblocks <= mapsize);

	assert(blkno + nblocks <= mapsize);

	dbmap += (blkno / 32);
	while (nblocks > 0)
	{
		bitno =  blkno & (32-1);
		nb = MIN(nblocks, 32 - bitno);

		mask = (0xffffffff << (32 - nb) >> bitno);
                if ((mask & *dbmap) != 0)
			brkpoint2(blkno, *dbmap);
                assert ((mask & *dbmap) == 0);
		*dbmap |= mask;

		dbmap++;
		blkno += nb;
		nblocks -= nb;
	}
}


/*
 *	DBFree()
 */
void DBFree( uint *dbmap,
	     int64 mapsize,
	     int64 blkno,
	     int64 nblocks )
{
	int32 word, nb, bitno;
	uint32 mask;

	assert(blkno > 0 && blkno < mapsize);
	assert(nblocks > 0 && nblocks <= mapsize);

	assert(blkno + nblocks <= mapsize);

	dbmap += (blkno / 32);
	while (nblocks > 0)
	{
		bitno =  blkno & (32-1);
		nb = MIN(nblocks, 32 - bitno);

		mask = (0xffffffff << (32 - nb) >> bitno);
                if ((mask & *dbmap) != mask)
			brkpoint2(blkno,*dbmap);
                assert ((mask & *dbmap) == mask);
		*dbmap &= ~mask;

		dbmap++;
		blkno += nb;
		nblocks -= nb;
	}
}


/*
 *	DBAllocCK()
 */
void DBAllocCK( uint *dbmap,
		int64 mapsize,
		int64 blkno,
		int64 nblocks )
{
	int32 word, nb, bitno;
	uint32 mask;

	assert(blkno > 0 && blkno < mapsize);
	assert(nblocks > 0 && nblocks <= mapsize);

	assert(blkno + nblocks <= mapsize);

	dbmap += (blkno / 32);
	while (nblocks > 0)
	{
		bitno =  blkno & (32-1);
		nb = MIN(nblocks, 32 - bitno);

		mask = (0xffffffff << (32 - nb) >> bitno);
                assert ((mask & *dbmap) == mask);

		dbmap++;
		blkno += nb;
		nblocks -= nb;
	}
}


/*
 *	DBFreeCK()
 */
void DBFreeCK( uint *dbmap,
	       int64 mapsize,
	       int64 blkno,
	       int64 nblocks )
{
	int32 word, nb, bitno;
	uint32 mask;

	assert(blkno > 0 && blkno < mapsize);
	assert(nblocks > 0 && nblocks <= mapsize);

	assert(blkno + nblocks <= mapsize);

	dbmap += (blkno / 32);
	while (nblocks > 0)
	{
		bitno =  blkno & (32-1);
		nb = MIN(nblocks, 32 - bitno);

		mask = (0xffffffff << (32 - nb) >> bitno);
                assert ((mask & *dbmap) == 0);

		dbmap++;
		blkno += nb;
		nblocks -= nb;
	}
}


/*
 *	dbPrtMap()
 */
void dbPrtMap(bmap_t	*mp)
{
        printf("   mapsize:   %d%d\n", mp->db_mapsize);
        printf("   nfree:     %d%d\n", mp->db_nfree);
        printf("   numag:     %d\n",   mp->db_numag);
        printf("   agsize:    %d%d\n", mp->db_agsize);
        printf("   agl2size:  %d\n", mp->db_agl2size);
        printf("   agwidth:   %d\n", mp->db_agwidth);
        printf("   agstart:   %d\n", mp->db_agstart);
        printf("   agheigth:  %d\n", mp->db_agheigth);
        printf("   aglevel:   %d\n", mp->db_aglevel);
        printf("   maxlevel:  %d\n", mp->db_maxlevel);
        printf("   maxag:     %d\n", mp->db_maxag);
        printf("   agpref:    %d\n", mp->db_agpref);
        printf("   l2nbppg:   %d\n", mp->db_l2nbperpage);
}


/*
 *	dbPrtCtl()
 */
void dbPrtCtl(dmapctl_t *dcp)
{
        int i, j, n;

        printf("   height:    %08x\n", dcp->height);
        printf("   leafidx:   %08x\n", dcp->leafidx);
        printf("   budmin:    %08x\n", dcp->budmin);
        printf("   nleafs:    %08x\n", dcp->nleafs);
        printf("   l2nleafs:  %08x\n", dcp->l2nleafs);

        printf("\n Tree:\n");
        for (i = 0; i < CTLLEAFIND; i += 8)
        {
                n = MIN(8,CTLLEAFIND-i);

                for (j = 0; j < n; j++)
                        printf("  [%03x]: %02x",i+j,(char)dcp->stree[i+j]);
                printf("\n");
        }

        printf("\n Tree Leaves:\n");
        for (i = 0; i < LPERCTL; i += 8)
        {
                n = MIN(8,LPERCTL-i);

                for (j = 0; j < n; j++)
                        printf("  [%03x]: %02x",
                                i+j,(char)dcp->stree[i+j+CTLLEAFIND]);
                printf("\n");
        }
}
#endif	/* _JFS_DEBUG_DMAP */
