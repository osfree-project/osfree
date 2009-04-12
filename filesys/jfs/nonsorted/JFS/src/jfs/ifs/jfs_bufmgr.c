/* $Id: jfs_bufmgr.c,v 1.2 2003/09/21 08:42:51 pasha Exp $ */

static char *SCCSID = "@(#)1.36.1.2  12/2/99 11:13:22 src/jfs/ifs/jfs_bufmgr.c, sysjfs, w45.fs32, fixbld";
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
 * PS 2001-2003 apply IBM fixes
 *
 */

/*
 *	jfs_bufmgr.c: buffer manager
 *
 * meta-data buffer cache (aka jcache) supporting no-steal, no-force
 * policy for transaction-oriented log-based recovery file system.
 *
 * for platform/device driver dependent issues, see jfs_bufmgr.h.
 *
 * note: for detail, RTFS.
 *
 *	buffer state:
 * busy (B_BUSY: checked out or in I/O) or not busy:
 *	a buffer is made B_BUSY when it is allocated (checked out) or
 *	when I/O is initiated for it.
 *	a busy buffer returns to non-nusy state at checkin or
 *	I/O completion.
 *
 * homeok or nohomeok (B_NOHOMEOK):
 *	a buffer is made B_NOHOMEOK when it is modified and
 *	not committed yet.
 *	a nohomeok buffer returns to homeok state when COMMIT
 *	record for the transaction which updated the page is
 *	written to log.
 *
 *
 *	buffer lists:
 * hashlist (per system): a doubly-linked list per hash class;
 *	each named buffer (a buffer that has a devices's block
 *	assigned to it) is placed on a hashlist	by hash class
 *	to be reclaimed.
 *	(valid vs invalid B_STALE/B_ERROR ?)
 *
 * freelist (per system): a circular doubly-linked list;
 *	a buffer that is not B_BUSY and not B_NOHOMEOK is on
 *	the freelist where it can be either reclaimed or recycled.
 *	
 * object pagelist (per inode): a doubly-linked list;
 *	all buffers containing metadata pagess of a file system object
 *	are on the object inode's pagelist.
 *
 * logsynclist (per log): a circular doubly-linked list;
 *	all buffers with lsn > 0 (i.e., its page logged/committed,
 *	but have not been propagated to home yet) are on the
 *	logsynclist of the log associated with the file system of
 *	the object of the page of the buffer in ascendinding order
 *	of lsn to compute syncpt of the log.
 *
 *	TBD serialization:
 * ? finer granularity locking:
 *	per hashlist/single freelist lock
 *	per hashlist/multiple freelist lock
 * ? lock ordering
 *
 *
 *	TBD resource deadlock prevention: resource ordering:
 * trouble = no steal policy + nohomeok buffers + no extension memory !
 * buffers are partitioned into N buffer pool ranked
 * from 0 to N-1 where N is the largest number of buffers a
 * transaction may hold simultaneously: each buffer has a rank
 * corresponding to the pool it belongs to;
 * each buffer request by a transaction is satisfied by
 * the buffers with ranks r, 0 <= r <= n, where n is the number
 * of buffers currently being held by the transaction;
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif	/* _JFS_OS2 */
#include "mmph.h"

#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_filsys.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_xtree.h"
#include "jfs_logmgr.h"
#include "jfs_proto.h"
#include "jfs_debug.h"


/*
 *	external references
 */
int32	txFileSystemDirty(inode_t *);

/*
 *	data buffer cache (cCache) control
 */
extern int32	nBuffer;
extern struct cCache cachemgr;

/*
 *	meta-data buffer cache (jCache) control
 */
struct jCache {
	int32		nBuffer;
	int32		nHashClass;
	struct chash	*HashTable;
} jCache;

/*
 *	buffer cache hashlist
 */
/* buffer cache hash function: jbuf_t *BHASH(inode_t *ip, int64 blkno) */
#define BHASH(ip,bn) \
    ( (struct chash *)(jCache.HashTable + (((int32)(ip)/sizeof(inode_t)+(int32)(bn)) & (jCache.nHashClass - 1))) )

#define HASHLIST_INIT(ANCHOR)	LIST_INIT(&(ANCHOR)->ch_hashlist)

#define HASHLIST_INSERT_HEAD(ANCHOR, ELM, FIELD)\
{\
	LIST_INSERT_HEAD(&(ANCHOR)->ch_hashlist, ELM, FIELD);\
	(ANCHOR)->ch_timestamp++;\
}

#define HASHLIST_REMOVE(ELM, FIELD)\
	LIST_REMOVE(ELM, FIELD)

#define	HASHLIST_SELF(ELM, FIELD)\
	LIST_SELF(ELM, FIELD)


/*
 *	buffer cache inode pagelist
 *
 * a doubly-linked list of pages of inode anchored in the inode
 */
#define PAGELIST_INSERT_HEAD(ANCHOR, ELM, FIELD)\
	CDLL_INSERT_HEAD(ANCHOR, ELM, FIELD)

#define PAGELIST_REMOVE(ANCHOR, ELM, FIELD)\
	CDLL_REMOVE(ANCHOR, ELM, FIELD)

#define	PAGELIST_SELF(ELM, FIELD)\
	CDLL_SELF(ELM, FIELD)


/*
 *	buffer cache cachelist
 *
 * a doubly-linked list of pages anchored in cachemgr.cachelist
 */
#define CACHELIST_INSERT_HEAD(ANCHOR, ELM, FIELD)\
{\
	CDLL_INSERT_HEAD(ANCHOR, ELM, FIELD);\
	(ELM)->cm_cached = TRUE;\
}

#define CACHELIST_INSERT_TAIL(ANCHOR, ELM, FIELD)\
{\
	CDLL_INSERT_TAIL(ANCHOR, ELM, FIELD);\
	(ELM)->cm_cached = TRUE;\
}

#define CACHELIST_INSERT_AFTER(ANCHOR, LISTELM, ELM, FIELD)\
{\
	CDLL_INSERT_AFTER(ANCHOR, LISTELM, ELM, FIELD);\
	(ELM)->cm_cached = TRUE;\
}

#define CACHELIST_REMOVE(ANCHOR, ELM, FIELD)\
{\
	CDLL_REMOVE(ANCHOR, ELM, FIELD);\
	(ELM)->cm_cached = FALSE;\
}

#define	CACHELIST_SELF(ELM, FIELD)\
	CDLL_SELF(ELM, FIELD)


/*
 *	buffer cache freelist
 *
 * a singly-linked list of pages anchored in cachemgr.freelist
 */
#define FREELIST_INSERT(ELM)\
{\
        (ELM)->cm_free = TRUE;\
        (ELM)->cm_freenxt = cachemgr.freelist;\
        cachemgr.freelist = ELM;\
        cachemgr.nfreecbufs++;\
}


/*
 *	buffer cache synchronization
 *
 * IOCACHE_LOCK: ref. jfs_cachemgr.h
 * LOGSYNC_LOCK: ref. jfs_logmgr.h
 */

/*
 * forward references
 */
static void release(register jbuf_t *bp);
static int32 bmIOwait(register jbuf_t *bp);


#ifdef	_JFS_STATISTICS
/*
 *	statistics
 */
struct statJCache {
	int32	reclaim;
	int32	read;
	int32	recycle;
	int32	lazywriteawrite;
	int32	recycleawrite;
	int32	logsyncawrite;
	int32	write;
} statJCache = {0};
#endif	/* _JFS_STATISTICS */


/*
 * NAME:	bmClear()
 *
 * FUNCTION:  zero-out the buffer's residual area.
 *
 * RETURN: none
 */
#define bmClear(bp, offset)\
{\
	bzero((bp)->cm_cdata + (offset), PSIZE - (offset));\
}


/*
 * NAME:	bmInit()
 *
 * FUNCTION: initialization at JFS bring up
 *
 * RETURN:
 *
 * note: system configuration information -> buffer cache configuration
 */
bmInit()
{
	int32		rc;
	jbuf_t	*bp;
	struct chash	*chp;
	int32	nJbuffer, nHashClass, nbyte, i;
	int32	size = 0;

	/*
	 * compute nominal meta-data buffer header/pool first
	 * (based on JFS buffer cache configuration),
	 * then compute hash table to bound hash list length (e.g. <= 2)
	 */
	/* meta-data cache size = 1/8 of cache size */
	nJbuffer = nBuffer >> 3;
	jCache.nBuffer = nJbuffer;

	/*
	 * allocate/initialize hashlist anchor table
	 */
	nHashClass = nJbuffer >> 1;
	/* round up to next smallest power of 2 */
	nHashClass = roundup2power(nHashClass);
	jCache.nHashClass = nHashClass;

	nbyte = sizeof(struct chash) * nHashClass;
	if ((chp = (struct chash *)xmalloc(nbyte, 0, pinned_heap)) == NULL)
		return ENOMEM;
	jCache.HashTable = chp;

	for (i = 0; i < nHashClass; chp++, i++)
	{
		chp->ch_flag = 0;
		HASHLIST_INIT(chp);
		chp->ch_timestamp = 0;
	}

jFYI(1,("jCache: nJbuffer:%d sizeof(jbuf_t):%d nHashClass:%d\n",
	nJbuffer, sizeof(jbuf_t), nHashClass));
	return 0;
}


/*
 * NAME:	bmShutdown()
 *
 * FUNCTION: cleanup at JFS shutdown
 *
 * RETURN:
 */
bmShutdown()
{
	return 0;
}


/*
 *	bmHold()
 *
 * FUNCTION: hold (set B_BUSY) buffer
 */
void bmHold(jbuf_t *bp)
{
	register int32	ipl;

	assert(bp->j_nohomeok);

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

#ifdef _JFS_LAZYCOMMIT
retry:								// D230860
#endif
	while (bp->cm_inuse)
	{
		IOCACHE_SLEEP(&bp->cm_event,0);
	}
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	if (bp->cm_read || bp->cm_write)
	{
		IOCACHE_SLEEP(&bp->cm_ioevent, 0);
		goto retry;
	}
#endif /* _JFS_LAZYCOMMIT */
// END D230860

	bp->cm_inuse = TRUE;

	IOCACHE_UNLOCK(ipl);	/* release jcache_lock */
}


/*
 * NAME:	bmAssign()
 *
 * FUNCTION:  assign a buffer to the specified block.
 *
 *	If the specified block is already in the buffer cache,
 *	reclaim it.
 *	Otherwise, recycle a buffer from freelist and
 *	assign it to the specified block.
 *
 * RETURN: address of the selected buffer's header marked B_BUSY.
 */
jbuf_t *bmAssign(
	register inode_t	*ip,
	register int64		lblkno,
	register int64		pblkno,
	register int32		size,
	register uint32		flag)
{
	register jbuf_t	*bp;
	int32	ipl;
	struct chash	*chp;
	uint32		type;
	int32		timestamp;
	cmctl_t		*ccp;

#ifdef MMIOPH
   int32 Hit = 1;                      /* (d201828) cache hit (1) or miss (0) */
#endif
   MMPHPrebmAssign();                  /* MMPH Performance Hook */

	type = flag & bmREAD_TYPE;

	chp = BHASH(ip, lblkno);

	ccp = (cmctl_t *)ip->i_cacheid;
	if (ccp == NULL)
	{
		iBindCache(ip);
		ccp = (cmctl_t *)ip->i_cacheid;
	}

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

	/* if new block assignment, skip hash table search */
	if (!(flag & bmREAD_ASSIGN))
		goto tryRecycle;

	/*
	 * reclaim the block's buffer if it is in the buffer cache.
	 *
	 * blocks are hashed by <ip, lblkno> where lblkno is
	 * . per file logical block number (data page access) or
	 * . per file system physical block number (e.g., index block
	 *   access)
	 * . per logical volume physical block number
	 * distinguished by bmREAD_TYPE of j_xflag
	 */
tryReclaim:
	for (bp = chp->ch_hashlist.head; bp; bp = bp->cm_hashlist.next)
		if ((bp->cm_blkno == lblkno) && (bp->j_ip == ip) &&
		    (bp->j_xflag & type))
		{
			/*
			 *	cache hit
			 *
			 * if the buffer is busy, wait for the buffer
			 * to be freed and then retry since the state
			 * of the buffer cache may have been changed
			 * while lock was released during sleep
			 * (e.g., buffer requested by caller may have
			 * been acquired by another thread)
			 */
			ASSERT(bp->cm_metadata);
			ASSERT(bp->j_xflag & B_BUFFER);
			/* if the cbuf is inuse (checked out),
			 * wait for it to be checked in and try again.
			 */
			if (bp->cm_inuse)
			{
				IOCACHE_SLEEP(&bp->cm_event,0);
				goto tryReclaim;
			}

			/* if the cbuf is in an i/o state,
			 * wait for the i/o to complete and try again.
			 */
			if (bp->cm_read || bp->cm_write)
			{
				IOCACHE_SLEEP(&bp->cm_ioevent,0);
				goto tryReclaim;
			}

			/*
			 * remove from the cachelist,
			 */
			if (bp->j_nohomeok == 0)
			{
#ifdef	_JFS_HEURISTICS_SLRU
				if (bp->cm_probationary)
				{
					if (bp == cachemgr.slruxp)
						cachemgr.slruxp = bp->cm_cachelist.prev;
				}
				else
					cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */

				CACHELIST_REMOVE(&cachemgr.cachelist, bp, cm_cachelist);
			}
#ifdef _JFS_HEURISTICS_SLRU
			/*
			 * Cache hit whether or not j_nohomeok
			 */
			bp->cm_hit = TRUE;			// D230860
#endif /*  _JFS_HEURISTICS_SLRU */

			goto out;
		}

	/*
	 *	cache miss
	 *
	 * recycle a buffer from freelist.
	 */
tryRecycle:
	timestamp = chp->ch_timestamp;

#ifdef MMIOPH
   Hit = 0;                            /* (d201828) */
#endif

	while ((bp = (jbuf_t *)cmRecycle(CM_WAIT, &ipl)) == NULL)
	{
		/* hashlist modified - retry hashlist reclaim */
		if (chp->ch_timestamp != timestamp)
			goto tryReclaim;
	}

	/*
	 * A buffer has been recycled from freelist;
	 */
	bp->cm_metadata = TRUE;

	/*
	 * initialize the block specific fields in the buffer header.
	 */
	bp->j_xflag = B_BUFFER | type;
	bp->j_lid = 0;
	bp->j_lsn = 0;
	/* insert on self logsynclist */
	CDLL_SELF((logsyncblk_t *)bp, synclist);
	CDLL_SELF(bp, cm_cachelist);  //PS01012003

	bp->j_clsn = 0;
	bp->j_nohomeok = 0;
	bp->j_logx = NULL;

	bp->j_ip = ip;
	bp->cm_ctl = ccp;
	bp->cm_blkno = lblkno;

	/* insert the buffer at head of inode pagelist */
	PAGELIST_INSERT_HEAD(&ccp->cc_jbuflist, bp, cm_pagelist);

	/* insert the buffer at the head of specified hashlist */
	HASHLIST_INSERT_HEAD(chp, bp, cm_hashlist);

	/* for non-resident page read, mark to read from disk */
	if (flag & bmREAD_ASSIGN)
		bp->cm_new = TRUE;
	/* for new block assignment, set its pxd */
	else
	{
		PXDaddress(&bp->cm_pxd, pblkno);
		PXDlength(&bp->cm_pxd, size >> ccp->cc_dev->cd_l2bsize);
	}

out:
	/* mark the buffer inuse (checked out) */
	bp->cm_inuse = TRUE;

	IOCACHE_UNLOCK(ipl);

jEVENT(0,("ba: bp:0x%x\n", bp));

bmAssign_Exit:

        MMPHPostbmAssign();       /* MMPH Performance Hook */

	return bp;
}


/*
 * NAME:	bmRead()
 *
 * FUNCTION:  assign a buffer to the specified block.
 *
 *	If the specified block is already in the buffer reclaim it.
 *	Otherwise, recycle a buffer from freelist, assign it to
 * 	the specified block, and then read the block's data into the buffer.
 *
 * RETURN:
 *	address of the selected buffer's header marked B_BUSY.
 *	I/O error return code.
 *
 * error recovery:
 *	If an I/O error occurs when reading the buffer, bmRead will	D228362
 *	release the buffer.						D228362
 */
bmRead(
	register inode_t	*ip,
	register int64		lblkno,
	register int32		size,
	register uint32		flag,
	register jbuf_t		**bpp)
{
	int32	rc = 0;
	int32	ipl;
	register jbuf_t *bp;
	int32	llen;
	uint8	xflag;
	int64	xaddr;
	int32	xlen;
	cmdev_t	*cdp;

jEVENT(0,("br: lblkno:%d%d\n", lblkno));

   MMPHPrebmRead();                    /* (d201828) MMPH Performance Hook */

	/*
	 * get the buffer assigned to the specified block.
	 */
	*bpp = bp = bmAssign(ip, lblkno, 0, size, flag | bmREAD_ASSIGN);

	/* assert(bp->cm_inuse = TRUE); */

	/*
	 * if buffer was reclaimed: done;
	 */
	if (!bp->cm_new) {
      rc = 0;
      goto TheEnd;
   }

	bp->cm_new = FALSE;

	/*
	 * buffer was recycled from the freelist:
	 * read the block's data into buffer from backing storage.
	 */
jEVENT(0,("br: bp:0x%x\n", bp));

#ifdef FreeJFS
	INCREMENT(statJCache.read);
#endif
	/*
	 * map the logical block address to physical block address
	 */
	llen = size >> ip->i_ipmnt->i_l2bsize;
	if (flag & bmREAD_PAGE)
	{
		/* convert per file logical block number
		 * to file system block number (imap_t ?)
		 */
		if (rc = xtLookup(ip, lblkno, llen, &xflag, &xaddr, &xlen, 0))
			goto TheEnd;
//		assert(llen == xlen);
		if (llen != xlen)
		{
			jALERT(1,("bmRead: bmREAD_PAGE: llen != xlen\n"));
			txFileSystemDirty(ip->i_ipmnt);
         rc = EIO;
			goto TheEnd;
		}
			
		PXDaddress(&bp->cm_pxd, xaddr);
		PXDlength(&bp->cm_pxd, xlen);
	}
	else if (flag & bmREAD_BLOCK)
	{
		PXDaddress(&bp->cm_pxd, lblkno);
		PXDlength(&bp->cm_pxd, llen);
	}

	/*
	 * read from disk
	 */
	/* queue the buffer to the cache device */
	IOQUEUE_READ(bp);
	bp->cm_ionxt = bp;

	/* start the i/o */
	cdp = bp->cm_ctl->cc_dev;
	cmDeviceStartIO(cdp, bp);
	
	ipl = IOCACHE_LOCK();

	/* wait for the read to complete */
	if (!bp->cm_iodone)
	{
		IOCACHE_SLEEP_RC(&bp->cm_ioevent,T_NORELOCK,rc);
		IOCACHE_NOLOCK(ipl);
	}
	else
	{
        rc = ((bp->cm_iobp->b_flags & B_ERROR) || (bp->cm_free)) ? EIO : 0; //PS01012003
		IOCACHE_UNLOCK(ipl);
	}

TheEnd:

	if (rc)
	{							// D228362
		jEVENT(1,("br: read error bp:0x%08x rc(%d)\n", bp, rc));
//		bmInvalidate(bp);				// D228362
	}							// D228362

   MMPHPostbmRead();                   /* (d201828) MMPH Performance Hook */

	return rc;
}


/*
 * NAME:	bmLazyWrite()
 *
 * FUNCTION: release the buffer, marking it so that the block will
 *	be written to the device if the buffer is to be recycled.
 *
 *	usage: write at commit time
 *
 * RETURN: none
 *
 * note: tx locked pages (NOHOMEOK) are NOT on freelist;
 */
void bmLazyWrite(
	register jbuf_t	*bp,
	uint32		force)
{
	register logx_t	*logx = bp->j_logx;
	register int32	diffl, diffp;
	register int32	ipl;
	cmdev_t	*cdp;

   MMPHPrebmLazyWrite();     /* MMPH Performance Hook */

jEVENT(0,("blw: bp:0x%x\n", bp));

	assert(bp->cm_inuse);

	ipl = IOCACHE_LOCK();

	bp->cm_inuse = FALSE;
	bp->cm_modified = TRUE;

	/*
	 * nohomeok page: do NOT insert back to cachelist;
	 */
	if (bp->j_nohomeok)
	{
		/*
		 * wake up anyone waiting on inuse for this buffer.
		 */
		if (bp->cm_event != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&bp->cm_event);
		}

		IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

      MMPHPostbmLazyWrite();           /* (d201828) MMPH Performance Hook */

		return;
	}

	/* assert(bp->j_nohomeok == 0); */

	/*
	 * log syncpt/inode recycle flush page: initiate asynchronous write;
	 */
	if (bp->j_xflag & B_SYNCPT ||
	    force & (COMMIT_FORCE | COMMIT_FLUSH))
	{
		bp->j_xflag &= ~B_SYNCPT;

		if (force & COMMIT_FLUSH)
			bp->cm_discard = TRUE;

		/*
		 * wake up anyone waiting on inuse for this buffer
		 * at completion of write;
		 */

		/* queue the buffer to the cache device */
		IOQUEUE_ASYNC_WRITE(bp);			// D230860
		bp->cm_ionxt = bp;

		bp->cm_ctl->cc_jiolevel++;

		cdp = bp->cm_ctl->cc_dev;
		cdp->cd_iolevel++;	

		IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

		/* start the i/o */
		cmDeviceStartIO(cdp, bp);

      MMPHPostbmLazyWrite(); /* MMPH Performance Hook */

		return;
	}

	/*
	 * insert back to cachelist for lazy write on recycle;
	 */
lazyWrite:
	release(bp);

	IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

   MMPHPostbmLazyWrite();              /* (d201828) MMPH Performance Hook */
}

#ifdef Drova
char FuckIbmComp1[100];
#endif

/*
 * NAME:	bmAsyncWrite()
 *
 * FUNCTION:  write asynchronously the specified buffer.
 *
 * RETURN: 0 if the I/O was successful; otherwise, errno.
 */
void bmAsyncWrite(
	register jbuf_t	*bp)
{
	cmdev_t		*cdp;
	int32		ipl;

jEVENT(0,("baw: bp:0x%x\n", bp));
#ifdef Drova
       FuckIbmComp1[0]++;
#endif
	assert(bp->cm_inuse);

	ipl = IOCACHE_LOCK();

	bp->cm_inuse = FALSE;
	bp->cm_modified = TRUE;

	/*
	 * wake up anyone waiting on inuse for this buffer
	 * at completion of write;
	 */

	/* queue the buffer to the cache device */
	IOQUEUE_ASYNC_WRITE(bp);				// D230860
	bp->cm_ionxt = bp;

	bp->cm_ctl->cc_jiolevel++;

	cdp = bp->cm_ctl->cc_dev;
	cdp->cd_iolevel++;	

	IOCACHE_UNLOCK(ipl);

	/* start the i/o */
	cmDeviceStartIO(cdp, bp);
}


/*
 * NAME:	bmWrite()
 *
 * FUNCTION:  write the specified buffer.
 *
 *	the buffer is marked as not B_BUSY via bmRelease()
 *	at pageout completion by bmWrite() (for synchronous
 *	write) or bmIODone() (for asynchronous write).
 *
 * RETURN: 0 if the I/O is launched successfully; otherwise, errno.
 */
bmWrite(
	register jbuf_t	*bp)
{
	int32	rc = 0;
	cmdev_t		*cdp;
	int32		ipl;

   MMPHPrebmWrite();                   /* (d201828) MMPH Performance Hook */

	ASSERT(!bp->cm_discard);				// D230860
jEVENT(0,("bw: bp:0x%x\n", bp));
	assert(bp->cm_inuse);
	/* assert(bp->j_nohomeok == 0); cf. commit write with force */

	ipl = IOCACHE_LOCK();

	/* bp->cm_inuse = FALSE; */ /* do NOT release at iodone */
	bp->cm_modified = TRUE;

	/*
	 * wake up anyone waiting on inuse for this buffer
	 * in subsequent release() after completion of write;
	 */

	/* queue the buffer to the cache device */
	IOQUEUE_WRITE(bp);
	bp->cm_ionxt = bp;

	bp->cm_ctl->cc_jiolevel++;

	cdp = bp->cm_ctl->cc_dev;
	cdp->cd_iolevel++;	

	IOCACHE_UNLOCK(ipl);

	/* start the i/o */
	cmDeviceStartIO(cdp, bp);

	/* wait for write completion */
	ipl = IOCACHE_LOCK();

	if (!bp->cm_iodone)
	{
		IOCACHE_SLEEP_RC(&bp->cm_ioevent,0,rc);
	}
	else
		rc = (bp->cm_iobp->b_flags & B_ERROR) ? EIO : 0;

	bp->cm_inuse = FALSE;

	/*
	 * nohomeok page: do NOT insert back to cachelist;
	 */
	if (bp->j_nohomeok)
	{
		/*
		 * wake up anyone waiting on inuse for this buffer.
		 */
		if (bp->cm_event != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&bp->cm_event);
		}

		IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

      MMPHPostbmWrite();               /* (d201828) MMPH Performance Hook */
#ifdef FreeJFS
		return rc;   //PS Везде RC проверяется!!!!
#else
		return;
#endif
	}

	release(bp);

	IOCACHE_UNLOCK(ipl);

   MMPHPostbmWrite();                  /* (d201828) MMPH Performance Hook */
	return rc;
}

/*
 * NAME:	bmIODone()
 *
 * FUNCTION:  buffer cache I/O completion processing.
 *
 * execution environment:
 *	This routine is called by iodone() under iodone_offl on
 *	an interrupt level or a process. In both cases it is
 *	called at interrupt priority INTIODONE.
 *
 *		This routine cannot page fault.
 *
 * RETURN: nothing
 *
 * NOTES:
 *	"bp->j_flag |= B_DONE" for MP safe/efficient callers was at
 *	a wrong place in iodone_offl(). It's been moved to here.
 *
 * N.B.: OS/2: jfsIOLock is held on entry/exit;
 */
void bmIODone(
	iobuf_t *iobp)
{
	cmdev_t *cdp;
	cmctl_t *ccp;
	jbuf_t	*bp, *nxp;
	struct logx	*logx;
	int32	ipl, lipl, error;

	/* get back the cbuf bound to the iobuf and
	 * the device that we did i/o for.
	 */
	bp = (cbuf_t *)iobp->b_jfsbp;
	cdp = bp->cm_ctl->cc_dev;

	/* check for an i/o error */
	error = (iobp->b_flags & B_ERROR) ? EIO : 0;

#ifdef _JFS_OS2
   if (error == EIO)
      SysLogError();
#endif

	/* get cmctl of the cbuf */
	ccp = bp->cm_ctl;

        ipl = IOCACHE_LOCK();

	if ((--cdp->cd_pending_requests == 0) && (cdp->cd_flag & CD_QUIESCE))
		IOCACHE_WAKEUP(&cdp->cd_iowait);

	bp->cm_iodone = TRUE;

	/*
	 *	read:
	 */
	if (bp->cm_read)
	{
		bp->cm_read = FALSE;

		/* if there are any threads waiting for this
		 * i/o to complete, wake them up.
		 */
		if (bp->cm_ioevent != EVENT_NULL)
		{
			IOCACHE_WAKEUP_RC(&bp->cm_ioevent,error);
		}

		/* if marked for discard on i/o complete or
		 * i/o error occured in reading the cbuf
		 * from disk, release the cbuf;
	 	 */
//PS21092003		if (bp->cm_discard || error)
		if (!bp->cm_inuse || error)
		{
			/* is the cbuf marked as inuse ? */
//PS21092003			if (bp->cm_inuse)
//PS21092003			{
				/* inuse cbufs cannot be released.
				 * it must be the error case.
				 */
//PS21092003				assert(!bp->cm_discard);

				/* wake up any threads waiting for the
				 * buffer to go out of use.
				 */
				if (bp->cm_event != EVENT_NULL)
				{
					IOCACHE_WAKEUP(&bp->cm_event);
				}

				bp->cm_inuse = FALSE;
//PS21092003			}

			/* release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(bp);

			/* is a deletion pending for the cmctl (i.e.
			 * deletion waiting for all i/o to complete).
			 */
			if (ccp->cc_delpend2)
			{
				/* a thread better be waiting for this */
				assert(ccp->cc_delwait != EVENT_NULL);

				/* is all i/o now complete for the cmctl ?
				 * if so, wake up the thread so that it can
				 * complete the cmctl deletion.
				 */
				if (ccp->cc_jbuflist.head == (jbuf_t *)&ccp->cc_jbuflist)
				{
					IOCACHE_WAKEUP(&ccp->cc_delwait);
				}
			}

        		IOCACHE_UNLOCK(ipl);
			return;
		}

		/*
		 * the jbuf must be inuse (no read-ahead), done.
		 */
//PS21092003		assert(bp->cm_inuse);
        	IOCACHE_UNLOCK(ipl);
		return;
	} /* end of read */
	/*
	 *	write:
	 */
	else
	{
		bp->cm_write = FALSE;

		/* cbuf remains modified if write error */
		if (!error)
			bp->cm_modified = FALSE;

		/*
		 * reset page lsn and remove from logsync list;
		 */
		if (bp->j_lsn)
		{
			logx = bp->j_logx;
			lipl = LOGSYNC_LOCK(logx);
	
			bp->j_logx = NULL;
			bp->j_lsn = 0;
			bp->j_clsn = 0;
			logx->count--;

			CDLL_REMOVE(&logx->synclist, (logsyncblk_t *)bp, synclist);

			/* remove stepping stone */
			if (logx->yah == bp)
				logx->yah = NULL;

			LOGSYNC_UNLOCK(lipl, logx);

			/* downcount file nohomeok page count */
			ccp->cc_njbufs--;
		}

		/* if there are any threads waiting for this
		 * i/o to complete, wake them up.
		 */
		if (bp->cm_ioevent != EVENT_NULL)
		{
			IOCACHE_WAKEUP_RC(&bp->cm_ioevent,error);
		}

		/* if the i/o level has been reached (synchronous
		 * write) or an i/o error occured, wake up the thread
		 * waiting on the object write completion.
		 */
		ccp->cc_jiolevel--;
        	if (ccp->cc_jiowait != EVENT_NULL)
		{
			if (ccp->cc_jiolevel == 0 || error)
			{
				IOCACHE_WAKEUP_RC(&ccp->cc_jiowait,error);
			}
		}

		cdp->cd_iolevel--;
		if (cdp->cd_iolevel == 0 && cdp->cd_iowait != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&cdp->cd_iowait);
		}

		/*
		 * the cbuf is to be discarded (released).
		 */
		if (bp->cm_discard)
		{
			/* if the write was the result of cbuf replacement,
			 * update the number of pending write due to cbuf
			 * replacement.
			 */
			if (bp->cm_lru)
			{
				bp->cm_lru = FALSE;
				cachemgr.numiolru -= 1;
			}

			if (bp->cm_event != EVENT_NULL)
			{
                            assert(bp->j_ip->i_number == 16);
				IOCACHE_WAKEUP(&bp->cm_event);
			}

			/* release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(bp);

			/* is a deletion pending for the cmctl ?
			 * (i.e., deletion waiting for all i/o to complete).
			 */
			if (ccp->cc_delpend2)
			{
				/* a thread better be waiting for this */
				assert(ccp->cc_delwait != EVENT_NULL);

				/* is all i/o now complete for the cmctl ?
				 * if so, wake up the thread so that it can
				 * complete the cmctl deletion.
				 */
				if (ccp->cc_jbuflist.head == (jbuf_t *)&ccp->cc_jbuflist)
				{
					IOCACHE_WAKEUP(&ccp->cc_delwait);
				}
			}

        		IOCACHE_UNLOCK(ipl);
			return;
		}

		/*
		 * the buffer is to be recycled/replaced:
		 */
		if (bp->cm_lru)
		{
			/* update the number of cbufs with i/o pending
			 * due to cbuf replacement.
			 */
			bp->cm_lru = FALSE;
			cachemgr.numiolru -= 1;

			/* if there was no i/o error, release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 * (if i/o error, do not insert back in cachelist)
			 */
#ifdef _JFS_LAZYCOMMIT
			if ((bp->j_nohomeok == 0) && !error)	// D230860
#else
			if (!error)
#endif
				cmRelCbuf(bp);

        		IOCACHE_UNLOCK(ipl);
			return;
		}

	} /* end of write */

	/*
	 * insert the cbuf at the tail of the cachelist
	 */
	if (bp->j_nohomeok == 0 && bp->cm_inuse == FALSE)
	{
#ifdef	_JFS_HEURISTICS_SLRU
		/* if page has been cache hit, insert at tail of protected segment */
		if (bp->cm_hit)
		{
			bp->cm_probationary = FALSE;

			/* is protected segment full ? */
			if (cachemgr.slrun >= cachemgr.slruN)
			{
				/* slide xp one over protected segment */
				if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
					nxp = cachemgr.slruxp->cm_cachelist.next;
				else
					nxp = cachemgr.cachelist.head;

				nxp->cm_probationary = TRUE;
				cachemgr.slruxp = nxp;
			}
			else
				cachemgr.slrun++;
	
			/* insert bp at tail of protected segment */
			CACHELIST_INSERT_TAIL(&cachemgr.cachelist, bp, cm_cachelist);
		}
		/* if page has been cache miss, insert at tail of probationary segment */
		else
		{
			bp->cm_probationary = TRUE;

			/* insert cp at tail of provationary segment as new xp */
			if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
			CACHELIST_INSERT_AFTER(&cachemgr.cachelist, cachemgr.slruxp, bp, cm_cachelist)	
			else
				CACHELIST_INSERT_HEAD(&cachemgr.cachelist, bp, cm_cachelist);
			cachemgr.slruxp = bp;
		}
#else
		CACHELIST_INSERT_TAIL(&cachemgr.cachelist, bp, cm_cachelist);
#endif	/* _JFS_HEURISTICS_SLRU */

		/*
		 * wakeup threads waiting on cm_inuse for this buffer
		 * for bmLazyWrite()/bmAsyncWrite();
		 */
		if (bp->cm_event != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&bp->cm_event);
		}
	
		/* wakeup any thread waiting for a free buf */
		if (cachemgr.freewait != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&cachemgr.freewait);
		}
//PS 28092001 Begin
#ifdef _JFS_LAZYCOMMIT
            else
            if ( (cachemgr.lazywait != EVENT_NULL) && (cachemgr.nfreecbufs >= cachemgr.minfree) )
            {
                   IOCACHE_WAKEUP(&cachemgr.lazywait);
            }
#endif
//PS 28092001 End
	}
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	else if (bp->cm_inuse == FALSE)
	{
		/*
		 * nohomeok is non-zero, so we can't put in cachelist, but
		 * we still want to wake up waitors
		 */
		if (bp->cm_event != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&bp->cm_event);
		}
	}
#endif /* _JFS_LAZYCOMMIT */
// END D230860

#ifdef Drova
                            FuckIbmComp1[0] += 5;
//                            FuckIbmComp1[1]++;
#endif

        IOCACHE_UNLOCK(ipl);
}


/*
 * NAME:	bmRelease(), release()
 *
 *	"bmRelease()" cares for the mutual exclusion,
 *	"release()" executes the actual buffer release operation.
 *
 * FUNCTION: Release the buffer, with no I/O implied.
 *
 *	for homeok page: mark the buffer not B_BUSY and
 *	insert into freelist so that it can be reclaimed
 *	or recycled;
 *	
 *	for nohomeok page: mark the buffer not B_BUSY
 *	(so that it can be reclaimed) but do not insert
 *	into freelist.
 *
 * RETURN: none
 *
 * execution environment: JCACHE_LOCK held on entry/exit.
 *	it is called by a process or an interrupt handler.
 *	They can page fault only if called under a process and
 *	the stack is not pinned.
 *	caller should be at interrupt priority INTIODONE or
 *	at a less favored priority.
 *
 * error recovery:
 *	? blocks with I/O errors are made non-reclaimable.
 */
static void release(
	register jbuf_t	*bp)
{
	cmdev_t	*cdp;						// D233382
	jbuf_t	*nxp;

	assert(bp->j_nohomeok == 0);

// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
	if (bp->cm_discard)
	{
		if (bp->cm_modified)
		{
			/* queue the buffer to the cache device */
			IOQUEUE_ASYNC_WRITE(bp);
			bp->cm_ionxt = bp;

			bp->cm_ctl->cc_jiolevel++;

			cdp = bp->cm_ctl->cc_dev;
			cdp->cd_iolevel++;	

			IOCACHE_UNLOCK(0);

			/* start the i/o */
			cmDeviceStartIO(cdp, bp);

			IOCACHE_LOCK();
		}
		else
			cmRelCbuf(bp);
		return;
	}
#endif /* _JFS_LAZYCOMMIT */
// END D233382

	/*
	 * insert buffer at cachelist:
	 */
#ifdef	_JFS_HEURISTICS_SLRU
	/* if page has been cache hit, insert at tail of protected segment */
	if (bp->cm_hit)
	{
		bp->cm_probationary = FALSE;

		/* is protected segment full ? */
		if (cachemgr.slrun >= cachemgr.slruN)
		{
			/* slide xp one over protected segment */
			if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
				nxp = cachemgr.slruxp->cm_cachelist.next;
			else
				nxp = cachemgr.cachelist.head;

			nxp->cm_probationary = TRUE;
			cachemgr.slruxp = nxp;
		}
		else
			cachemgr.slrun++;
	
		/* insert bp at tail of protected segment */
		CACHELIST_INSERT_TAIL(&cachemgr.cachelist, bp, cm_cachelist);
	}
	/* if page has been cache miss, insert at tail of probationary segment */
	else
	{
		bp->cm_probationary = TRUE;

		/* insert cp at tail of provationary segment as new xp */
		if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
			CACHELIST_INSERT_AFTER(&cachemgr.cachelist, cachemgr.slruxp, bp, cm_cachelist)
		else
			CACHELIST_INSERT_HEAD(&cachemgr.cachelist, bp, cm_cachelist);
		cachemgr.slruxp = bp;
	}
#else
	CACHELIST_INSERT_TAIL(&cachemgr.cachelist, bp, cm_cachelist);
#endif	/* _JFS_HEURISTICS_SLRU */

	/*
	 * wake up anyone waiting on inuse for this buffer.
	 */
	if (bp->cm_event != EVENT_NULL)
	{
		IOCACHE_WAKEUP(&bp->cm_event);
	}

	/*
	 * wake up anyone waiting on a free buffer.
	 */
	if (cachemgr.freewait != EVENT_NULL)
	{
                IOCACHE_WAKEUP(&cachemgr.freewait);
	}
//PS 28092001 Begin 
            else
            if ( (cachemgr.lazywait != EVENT_NULL) && (cachemgr.nfreecbufs >= cachemgr.minfree) )
            {
                   IOCACHE_WAKEUP(&cachemgr.lazywait);
            }
//PS 28092001 End
}

/*
 * NAME:	bmRelease()
 *
 * function: checkin the buffer;
 */
void bmRelease(
	register jbuf_t	*bp)
{
	register int32 ipl;	/* current interrupt priority */
	cmdev_t	*cdp;

jEVENT(0,("br: bp:0x%08x flag:0x%04x:%08x\n", bp, bp->j_xflag, bp->cm_flag));

	assert(bp->cm_inuse);

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

	bp->cm_inuse = FALSE;

	/*
	 * nohomeok page, mark the buffer reclaimable,
	 * but leave it non-recyclable (i.e., do not insert
	 * into cachelist);
	 */
	if (bp->j_nohomeok)
	{
		/*
		 * wake up anyone waiting for this buffer.
		 */
		if (bp->cm_event != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&bp->cm_event);
		}

		IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

		return;
	}

	/* assert(bp->j_nohomeok == 0) */

	/* if SYNCPT on, initiate async write */
	if (bp->j_xflag & B_SYNCPT)
	{
		bp->j_xflag &= ~ B_SYNCPT;

		ASSERT(bp->cm_modified);

		/*
		 * wake up anyone waiting for this buffer
		 * at completion of write;
		 */

		/* queue the buffer to the cache device */
		IOQUEUE_ASYNC_WRITE(bp);			// 230860
		bp->cm_ionxt = bp;

		bp->cm_ctl->cc_jiolevel++;

		cdp = bp->cm_ctl->cc_dev;
		cdp->cd_iolevel++;	

		IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

		cmDeviceStartIO(cdp, bp);

		return;
	}
		
	/* insert into cachelist */
	release(bp);

	IOCACHE_UNLOCK(ipl);	/* release jcache_lock */
}


/*
 * NAME:	bmSetXD()
 *
 * FUNCTION:  update buffer extent description
 *	when extent covered by the buffer is extended/relocated.
 *
 * RETURN: none
 *
 * note: for bmREAD_BLOCK pages only;
 */
void
bmSetXD(
	jbuf_t		*bp,
	int64		lblkno,	/* extent address (in fs block number) */
	int32		size)	/* extent size (in byte) */		
{
	struct chash	*chp;
	inode_t	*ip = bp->j_ip;
	int32	ipl;

	assert(bp->cm_inuse);

	if (bp->cm_blkno == lblkno)
		goto updateSize;

	/*
	 * update buffer extent address
	 */
	chp = BHASH(ip, lblkno);

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

	/* Remove the buffer from the previous hashlist */
	HASHLIST_REMOVE(bp, cm_hashlist);

	/* insert the buffer at the head of specified hashlist */
	HASHLIST_INSERT_HEAD(chp, bp, cm_hashlist);

	IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

	bp->cm_blkno = lblkno;
	PXDaddress(&bp->cm_pxd, lblkno);

	/*
	 * update buffer extent size
	 */
updateSize:
	PXDlength(&bp->cm_pxd, size >> ip->i_ipmnt->i_l2bsize);
}


/*
 * NAME:	bmFlush()
 *
 * FUNCTION: write synchronously the specified block and
 *	invalidate (make non-reclaimable) it.
 *
 *	usage: write EA's
 *
 * RETURN:
 *
 * note: caller should be holding the buffer.
 */
int32 bmFlush(
	register jbuf_t	*bp)
{
	int32	rc = 0;
	cmdev_t		*cdp;
	int32		ipl;

jEVENT(0,("bmFlush: bp:0x%x\n", bp));
	assert(bp->cm_inuse);

	ipl = IOCACHE_LOCK();

	/* bp->cm_inuse = FALSE; */ /* do NOT release at iodone */
	bp->cm_modified = TRUE;

	/* queue the buffer to the cache device */
	IOQUEUE_WRITE(bp);
	bp->cm_ionxt = bp;

	bp->cm_ctl->cc_jiolevel++;

	cdp = bp->cm_ctl->cc_dev;
	cdp->cd_iolevel++;	

	IOCACHE_UNLOCK(ipl);

	/* start the i/o */
	cmDeviceStartIO(cdp, bp);

	/* wait for write completion */
	ipl = IOCACHE_LOCK();

	if (!bp->cm_iodone)
	{
		IOCACHE_SLEEP_RC(&bp->cm_ioevent,0,rc);
	}
	else
		rc = (bp->cm_iobp->b_flags & B_ERROR) ? EIO : 0;

	bp->cm_inuse = FALSE;

	if (bp->cm_event != EVENT_NULL)
	{
		IOCACHE_WAKEUP(&bp->cm_event);
	}

	cmRelCbuf(bp);

	IOCACHE_UNLOCK(ipl);

	return rc;
}


/*
 * NAME:	bmInodeWrite()
 *
 * FUNCTION:  write meta-data inode synchronously if dirty;
 *
 * RETURNS: nothing
 */
bmInodeWrite(
	inode_t	*ip)
{
	int32	rc;
	register int32	ipl;
	register jbuf_t	*bp, *bpnext, *iotail;
	cmctl_t	*ccp;
	cmdev_t	*cdp;

	ccp = ip->i_cacheid;
	cdp = ccp->cc_dev;
	iotail = NULL;

	ipl = IOCACHE_LOCK(); /* acquire jcache_lock */

	/*
	 * unbind buffers from inode pagelist and initiate pageout
	 */	
        for (bp = ccp->cc_jbuflist.head; bp != (jbuf_t *)&ccp->cc_jbuflist; bp = bpnext)
        {
		/* get next buf pointer */
               	bpnext = bp->cm_pagelist.next;

		/*
		 * skip busy buffer
		 */
		if (bp->cm_write || bp->cm_read)
			continue;

		if (bp->j_nohomeok)
		{
			bp->j_xflag |= B_SYNCPT;
			continue;
		}

		if (bp->cm_inuse)
			continue;

		/*
		 * init pageout of non-busy homeok dirty buffer
		 */
		if (bp->cm_modified)
		{
			/*
			 * remove from cachelist
			 */
#ifdef	_JFS_HEURISTICS_SLRU
			if (bp->cm_probationary)
			{
				if (bp == cachemgr.slruxp)
					cachemgr.slruxp = bp->cm_cachelist.prev;
			}
			else
				cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
			CACHELIST_REMOVE(&cachemgr.cachelist, bp, cm_cachelist);

			/* queue the buffer to the cache device */
			IOQUEUE_WRITE(bp);

			/* append at tail of device iolist */
			if (iotail)
			{
				bp->cm_ionxt = iotail->cm_ionxt;
				iotail->cm_ionxt = bp;
			}
			else
				bp->cm_ionxt = bp;
			iotail = bp;

			ccp->cc_jiolevel++;

			cdp->cd_iolevel++;	
		}
        }

	IOCACHE_UNLOCK(ipl); /* release jcache_lock */

	/* start i/o on the cache device */
	if (iotail)
		cmDeviceStartIO(cdp, iotail);

        ipl = IOCACHE_LOCK();

	/* wait for the writes to complete */
	assert(ccp->cc_jiowait == EVENT_NULL);
	if (ccp->cc_jiolevel)
	{
		IOCACHE_SLEEP_RC(&ccp->cc_jiowait,T_NORELOCK,rc);
		IOCACHE_NOLOCK(ipl);
	}
	else
		IOCACHE_UNLOCK(ipl);
}


/*
 * NAME:	bmInodeFlush()
 *
 * FUNCTION:  write asynchronously if dirty and
 *	invalidate all buffers of the specified inode;
 *
 *	usage: inode recycle: the inode has been committed
 *	(buffers should be invalidated since, e.g.,
 *	the same name (ip, block address) for the buffer
 *	may be used for new file system after the file
 *	system containing the block has been deleted)
 *	unmount flush:
 *
 * RETURNS: nothing
 *
 * note:
 *	the inode being recycled is not held by any thread:
 *	buffers should be either on freelist or
 *	busy for pageout in progress
 *
 *	buffer should be unbound from inode at pageout
 *	since inode may have been recycled and no longer
 *	bound with the buffer at pageout completion.
 */
bmInodeFlush(
	inode_t	*ip)
{
	register int32	ipl;
	register jbuf_t	*bp, *bpnext, *iotail;
	cmctl_t	*ccp;
	cmdev_t	*cdp;

	ccp = ip->i_cacheid;
	cdp = ccp->cc_dev;
	iotail = NULL;

	ipl = IOCACHE_LOCK(); /* acquire jcache_lock */

	/*
	 * unbind buffers from inode pagelist and initiate pageout
	 */	
        for (bp = ccp->cc_jbuflist.head; bp != (jbuf_t *)&ccp->cc_jbuflist; bp = bpnext)
        {
#ifndef _JFS_LAZYCOMMIT
		assert(bp->cm_inuse == FALSE);
		assert(bp->j_nohomeok == 0);
#endif /* _JFS_LAZYCOMMIT */

		/* get next buf pointer now since be may be released below.
		 */
               	bpnext = bp->cm_pagelist.next;

		/*
		 * busy buffer: must be in pageout;
		 */
#ifdef _JFS_LAZYCOMMIT
		if (bp->cm_inuse || bp->j_nohomeok || bp->cm_write ||
		    bp->cm_read)				// D230860
#else /* ! _JFS_LAZYCOMMIT */
		if (bp->cm_write || bp->cm_read)
#endif /* _JFS_LAZYCOMMIT */
			/* buffer will be removed from logsync list and
			 * released at pageout completion;
			 */
			bp->cm_discard = TRUE;
		/*
		 * non-busy buffer;
		 */
		else
		{
			/*
			 * remove from cachelist
			 */
#ifdef	_JFS_HEURISTICS_SLRU
			if (bp->cm_probationary)
			{
				if (bp == cachemgr.slruxp)
					cachemgr.slruxp = bp->cm_cachelist.prev;
			}
			else
				cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
			CACHELIST_REMOVE(&cachemgr.cachelist, bp, cm_cachelist);

			/* dirty buffer: initiate async pageout */
			if (bp->cm_modified)
			{
				/* invalidate buffer at pageout completion */
				bp->cm_discard = TRUE;

				/* queue the buffer to the cache device */
				IOQUEUE_ASYNC_WRITE(bp);	// D230860

				/* append at tail of device iolist */
				if (iotail)
				{
					bp->cm_ionxt = iotail->cm_ionxt;
					iotail->cm_ionxt = bp;
				}
				else
					bp->cm_ionxt = bp;
				iotail = bp;

				ccp->cc_jiolevel++;

				cdp->cd_iolevel++;	
			}
			/* non-dirty buffer: release the buffer */
			else
				cmRelCbuf(bp);
		}
        }

	IOCACHE_UNLOCK(ipl); /* release jcache_lock */

	/* start i/o on the cache device */
	if (iotail)
		cmDeviceStartIO(cdp, iotail);
}


/*
 * NAME:	bmInvalidate()
 *
 * FUNCTION: invalidate (make non-reclaimable) the specified block
 *	     without writing it back.
 *
 *	usage: invalidate buffer with bmREAD_RAW,
 *		inode remove (per page at a time ?)
 *
 * RETURN: nothing
 *
 * note: caller should be holding the buffer.
 */
void bmInvalidate(
	register jbuf_t	*bp)
{
	register int32 ipl, lipl;
	register logx_t	*logx;

	assert(bp->cm_inuse);

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

	bp->cm_inuse = FALSE;

// BEGIN D231252
	if (bp->j_nohomeok)
	{
		/*
		 * We can't invalidate the page since a pending transaction
		 * is referencing it.  We will delete it after it is written.
		 */
		bp->cm_discard = TRUE;

		if (bp->cm_event != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&bp->cm_event); 
		}
		IOCACHE_UNLOCK(ipl);

		return;
	}
// END D231252

	/*
	 * remove from logsynclist since it will not be written out;
	 */
	if (bp->j_lsn)
	{
		logx = bp->j_logx;
		lipl = LOGSYNC_LOCK(logx);

		bp->j_lsn = 0;
		logx->count--;

		CDLL_REMOVE(&logx->synclist, (logsyncblk_t *)bp, synclist);

		LOGSYNC_UNLOCK(lipl, logx);
	}

	/* release the cbuf:
	 * remove from hashlist, remove from pagelist, and
	 * insert at head of freelist;
	 */
	cmRelCbuf(bp);

	IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

	return;
}


/*
 * NAME:	bmExtentInvalidate()
 *
 * FUNCTION:  invalidate (make non-reclaimable) all buffers
 *	of the specified extent without writing them back.
 *
 *	usage: inode extent free;
 *
 * RETURN: nothing
 */
void bmExtentInvalidate(
	register inode_t	*ip,
	register int64		xaddr,
	register int32		xlen)
{
	register jbuf_t	*bp;
	register int32	ipl, lipl;
	register int64	lblkno;
	struct chash	*chp;
	register logx_t	*logx;

	lblkno = xaddr;

	ipl = IOCACHE_LOCK(); /* acquire jcache_lock */

invalidatePage:
	chp = BHASH(ip, lblkno);

	/*
	 * search for the buffer of the extent page;
	 */
	for (bp = chp->ch_hashlist.head; bp; bp = bp->cm_hashlist.next)
		if ((bp->cm_blkno == lblkno) && (bp->j_ip == ip) &&
		    (bp->j_xflag & bmREAD_BLOCK))
		{
			/* is buffer in pageout ? */
			if (bp->cm_write || bp->cm_read)
			{
				bp->cm_discard = TRUE;
				break;
			}

			/*
			 * remove from logsynclist;
			 */
			if (bp->j_lsn)
			{
				logx = bp->j_logx;
				lipl = LOGSYNC_LOCK(logx);

				bp->j_lsn = 0;
				logx->count--;

				CDLL_REMOVE(&logx->synclist, (logsyncblk_t *)bp, synclist);

				LOGSYNC_UNLOCK(lipl, logx);
			}

			/* buffer must be on cachelist:
			 * move to head of freelist
			 */
			if (bp->j_nohomeok == 0)
			{
				/*
				 * remove from cachelist
				 */
#ifdef	_JFS_HEURISTICS_SLRU
				if (bp->cm_probationary)
				{
					if (bp == cachemgr.slruxp)
						cachemgr.slruxp = bp->cm_cachelist.prev;
				}
				else
					cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
				CACHELIST_REMOVE(&cachemgr.cachelist, bp, cm_cachelist);
			}
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
			else
			{
				bp->cm_discard = TRUE;
				break;
			}
#endif /* _JFS_LAZYCOMMIT */
// END D230860

			/* release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(bp);

			break;
		}

	lblkno += ip->i_ipmnt->i_nbperpage;
	if (lblkno < xaddr + xlen)
		goto invalidatePage;

	IOCACHE_UNLOCK(ipl); /* release jcache_lock */
}


/*
 * NAME:	bmInodeInvalidate()
 *
 * FUNCTION:  invalidate (make non-reclaimable) all buffers
 *	of the specified inode without writing them back.
 *
 *	usage: inode remove ?
 *	(per page at a time via bmInvalidate() or per inode via
 *	bmInodeInvalidate())
 *
 * RETURN: nothing
 *
 * note: cf. bmInodeFlush().
 */
void bmInodeInvalidate(
	register inode_t	*ip)
{
	register jbuf_t	*bp, *bpnext;
	register int32	ipl;
	cmctl_t	*ccp;

	ccp = ip->i_cacheid;

	ipl = IOCACHE_LOCK(); /* acquire jcache_lock */

	/*
	 * scan the inode bufferlist and invalidate
	 */
	for (bp = ccp->cc_jbuflist.head; bp != (jbuf_t *)&ccp->cc_jbuflist; bp = bpnext)
	{
		/* get next buf pointer now since be may be released below.
		 */
               	bpnext = bp->cm_pagelist.next;

		/* is buffer in pageout ? */
		if (bp->cm_write)
			bp->cm_discard = TRUE;
		else
		{
			if (bp->cm_inuse)
				bp->cm_inuse = FALSE;
			else
			{
				/*
				 * remove from cachelist
				 */
#ifdef	_JFS_HEURISTICS_SLRU
				if (bp->cm_probationary)
				{
					if (bp == cachemgr.slruxp)
						cachemgr.slruxp =
							bp->cm_cachelist.prev;
				}
				else
					cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
				CACHELIST_REMOVE(&cachemgr.cachelist, bp,
						 cm_cachelist);
			}

			/* release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(bp);
		}
        }

	IOCACHE_UNLOCK(ipl); /* release jcache_lock */
}


/*
 * NAME:	bmSync()
 *
 * FUNCTION:  wait for completion of all pageouts of buffers
 *	of the specified fileset
 *
 *	usage: wait for completion of all asynchronous
 *	pageout of buffers of all inodes of the file system
 *	initiated via iuncache()/bmInodeFlush() at unmount()
 *	before updating superblock.
 *	no separate global invalidation is required since
 *	flush of each inode invalidates all buffers of
 *	the inode, and therefore all buffers of the file
 *	system.
 *
 * RETURN: nothing
 *
 * note:
 *	all of the file system's blocks should have been flushed
 *	prior to calling this service via iuncache()/bmInodeFlush().
 *	for each inode basis.
 */
void bmSync(
	register inode_t	*ipmnt)		/* mount inode */
{
	register cmdev_t	*cdp;
	register int32 ipl;

	cdp = ipmnt->i_cachedev;

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

	if (cdp->cd_iolevel > 0)
	{
		IOCACHE_SLEEP(&cdp->cd_iowait,T_NORELOCK);
		IOCACHE_NOLOCK(ipl);	/* release jcache_lock */
	}
	else
		IOCACHE_UNLOCK(ipl);	/* release jcache_lock */
}


/*
 * NAME:	bmLogSync()
 *
 * FUNCTION:
 * initiate pageout for all modified journaled pages which
 * are in homeok state.
 * if the nohomeok bit is set the syncpt bit is set so that
 * the page will be forced at the next commit processing.
 *
 * initiate pageout of pages (HOMEOK && !PAGEOUT)
 * set SYNCPT of pages otherwise;
 * syncpt address = lsn of the entry at the head of list
 *
 * for each log, the variable logsync is maintained as
 * an lower bound of the oldest transaction or page.
 * its value is suitable for the log address in a log
 * syncpt record.
 *
 * when new log sync point addresses are "published"
 * for each log in the system, the current transaction ages
 * are included in the computation as lsn of tblk protects
 * log records associated with allocation/deallocation of
 * resources until lsn of the page who caused the log is
 * inherited by allocation map page in case, after commit,
 * the page is paged out and syncpt is made but system crashed
 * before allocation map page is updated and inherits the lsn
 * (then, tblk is freed).
 *
 * serialization:
 * LOG_LOCK held on entry/exit;
 * lock order: JCACHE_LOCK, LOGSYNC_LOCK
 *
 * note:
 *	log sync list (recovery_lsn list)
 * per log cdll of buffer page and tblks;
 * sorted on lsn in ascending order (smaller lsn = older logage):
 *
 *	journaled page;
 * page is on hashlist;
 * page is either NOTHOMEOK (not on freelist) or HOMEOK (on freelist);
 * page becomes NOTHOMEOKi|DIRTY when tlock is acquired on the page;
 * page is inserted in logsync list when lsn is assigned to it
 * lmLog() time;
 * relocate at logmove() and setbitmaps();
 * page is made HOMEOK when it is bmLazyWrite()d and inserted
 * in freelist at txUnlock() time;
 * page is removed from freelist and initiated for pageout by
 * either recycle() or bmLogSync();
 *  bmflush() at iuncache() to recycle inode;
 * at pageout completion, bmIODone() resets page lsn to 0 and
 * removes from logsynclist and inserts in freelist;
 */
int32 bmLogSync(
	register struct log	*log)		/* log inode */
{
	register int32 jipl, lipl;
	register struct logx *logx = log->logx;
	register struct logsyncblk	*lp;
	void 	*hlp;
	register jbuf_t	*bp, *devtail;
	register int32 diffp, diffl;
	register int32	n = 0, w = 0, q = 0;

jEVENT(0,("bmLogSync:\n"));

	hlp = (void *)&logx->synclist;

	jipl = IOCACHE_LOCK();	/* acquire jcache_lock */
	lipl = LOGSYNC_LOCK(logx);

	assert(logx->flag & JFS_GROUPCOMMIT);

	/*
	 *	flush pages in group commit log synclist
	 */
gcLogSync:
	for (lp = logx->synclist.head; lp != hlp; lp = lp->synclist.next)
	{
		n++;

		/* skip tblk on the list */
		if (!(lp->xflag & B_BUFFER))
			continue;
		bp = (struct cbuf *)lp;

                /* initiate pageout or set syncpt bit on page */
                if (!bp->cm_inuse &&	/* not checked out */
		    !bp->cm_write &&	/* not in pageout */
		    bp->j_nohomeok == 0)/* homeok */
		{
			/*
			 * remove from cachelist
			 */
#ifdef	_JFS_HEURISTICS_SLRU
			if (bp->cm_probationary)
			{
				if (bp == cachemgr.slruxp)
					cachemgr.slruxp = bp->cm_cachelist.prev;
			}
			else
				cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
			CACHELIST_REMOVE(&cachemgr.cachelist, bp, cm_cachelist);

			/* mark stepping stone "you are here" */
			logx->yah = bp;

			/* insert at tail of device iolist */
			w++;
                        cmDeviceQueueIO(bp, CM_WRITE);

			if (++q >= 8)
			{
				LOGSYNC_UNLOCK(lipl, logx);

				devtail = cmQueueIO();

				IOCACHE_UNLOCK(jipl);

				/* asynchronous write */
				cmStartIO(devtail);

				q = 0;

				jipl = IOCACHE_LOCK();
				lipl = LOGSYNC_LOCK(logx);
			}

			/* restart from beginning of the logsync list
			 * if stepping stone has been removed from race
			 * on logsync list while locks has been released;
			 */
			if (logx->yah != bp)
				goto gcLogSync;
		}
		else if (!bp->cm_write)
			bp->j_xflag |= B_SYNCPT;
        }

	/*
	 *	compute next logsync value
	 *
	 * lsn of the first logsyncblk (either page or tblk).
	 */
out:
	if (q)
	{
		LOGSYNC_UNLOCK(lipl, logx);

		devtail = cmQueueIO();

		IOCACHE_UNLOCK(jipl);	/* release jcache_lock */

		/* asynchronous write */
		cmStartIO(devtail);

		lipl = LOGSYNC_LOCK(logx);
	}
	else
		IOCACHE_UNLOCK(jipl);	/* release jcache_lock */

	if ((lp = logx->synclist.head) != (void *)hlp)
		logx->sync = lp->lsn;
	else
		logx->sync = logx->lsn;

	LOGSYNC_UNLOCK(lipl, logx);

jEVENT(0,("bmLogSync: lsn:0x%x syncpt:0x%x sync:0x%x (%d:%d)\n",
	logx->lsn, logx->syncpt, logx->sync, n, w));
	return 0;
}


/*
 *	bmLogSyncRelease()
 *
 * function:	release buffer from logsync;
 *		called by abort;
 *
 * N.B.: OS/2: jfsIOLock should not be nested;
 */
void bmLogSyncRelease(
	jbuf_t	*bp)
{
	register int32	ipl, lipl;
	register logx_t	*logx = bp->j_logx;

	ipl = IOCACHE_LOCK();

// BEGIN D230860
//PS21092003	assert(bp->j_nohomeok);
	if (bp->j_nohomeok)   //PS21092003
           bp->j_nohomeok--;

	if (bp->j_nohomeok)
	{
		IOCACHE_UNLOCK(ipl);
		return;
	}
// END D230860
       if(logx)  //PS21092003
         {
	  lipl = LOGSYNC_LOCK(logx);
	  bp->j_logx = NULL;
	  bp->j_lsn = 0;
	  bp->j_clsn = 0;
	  logx->count--;
	  CDLL_REMOVE(&logx->synclist, (logsyncblk_t *)bp, synclist);
	  CDLL_SELF((logsyncblk_t *)bp, synclist);
	  LOGSYNC_UNLOCK(lipl, logx);
//PS21092003 BEGIN
         }
       else
         {
	  bp->j_lsn = 0;
	  bp->j_clsn = 0;
         }
//PS21092003 End
	release(bp);
			
	IOCACHE_UNLOCK(ipl);
}

// BEGIN D230860
#ifdef _JFS_CACHESTATS
/*
 *	getCacheStats()
 */
void	getCacheStats(
	struct cachestats *cs)
{
	cbuf_t		*bp;
	struct chash	*chp;
	int32		i;
	int32		ipl;

	bzero(cs, sizeof(struct cachestats));
	ipl = IOCACHE_LOCK();

	cs->cachesize = cachemgr.cachesize;
	cs->hashsize = cachemgr.hashsize;
	cs->nfreecbufs = cachemgr.nfreecbufs;
	cs->minfree = cachemgr.minfree;
	cs->maxfree = cachemgr.maxfree;
	cs->numiolru = cachemgr.numiolru;
	cs->slrun = cachemgr.slrun;
	cs->slruN = cachemgr.slruN;

	/*
	 * Data
	 */
jEVENT(0,("calculating stats on data buffers\n"));
	for (i = 0; i < cachemgr.hashsize; i++)
	{
		chp = cachemgr.hashtbl + i;
jEVENT(0,("chp = 0x%x\n", chp));
		for (bp = chp->ch_hashlist.head; bp; bp = bp->cm_hashlist.next)
		{
			if (bp->cm_inuse)
				cs->cbufs_inuse++;
			else if (bp->cm_read || bp->cm_write)
				cs->cbufs_io++;
			else
			{
				if (!bp->cm_cached)
				{
					jEVENT(2,("bp 0x%x not inuse, io, nohomeok or cached\n", bp));
				}
				else if (bp->cm_probationary)
					cs->cbufs_probationary++;
				else
					cs->cbufs_protected++;
			}
		}
	}

	/*
	 * Meta-Data
	 */
jEVENT(0,("calculating stats on meta-data buffers\n"));
	for (i = 0; i < jCache.nHashClass; i++)
	{
		chp = jCache.HashTable + i;
jEVENT(0,("chp = 0x%x\n", chp));
		for (bp = chp->ch_hashlist.head; bp; bp = bp->cm_hashlist.next)
		{
			if (bp->cm_inuse)
				cs->jbufs_inuse++;
			else if (bp->cm_read || bp->cm_write)
				cs->jbufs_io++;
			else if (bp->j_nohomeok)
				cs->jbufs_nohomeok++;
			else
			{
				if (!bp->cm_cached)
				{
					jEVENT(2,("bp 0x%x not inuse, io, nohomeok or cached\n", bp));
				}
				else if (bp->cm_probationary)
					cs->jbufs_probationary++;
				else
					cs->jbufs_protected++;
			}
		}
	}
	
	IOCACHE_UNLOCK(ipl);
}
#endif /* _JFS_CACHESTATS */
// END D230860

#ifdef	FreeJFS // _JFS_STATISTICS
/*
 *	bmStatistics()
 */
bmStatistics(
	caddr_t	arg,
	int32	flag)
{
	int32	rc;

	/* copy out the argument */
	if (rc = copyout((caddr_t)&statJCache, (caddr_t)arg,
			 sizeof(struct statJCache)))
		return rc;

	/* reset the counters */
	if (flag)
	{
		statJCache.reclaim = 0;
		statJCache.read = 0;
		statJCache.recycle = 0;
		statJCache.lazywriteawrite = 0;
		statJCache.recycleawrite = 0;
		statJCache.logsyncawrite = 0;
		statJCache.write = 0;
	}

	return 0;
}
#endif	/* _JFS_STATISTICS */


#ifdef	_JFS_DEBUG
/*
 *	bmSanity()
 */
void bmSanity()
{
	jbuf_t		*bp;
	struct chash	*chp;
	inode_t		*ip;
	int32		i;
	int32		nhash = 0, nfree = 0;

	printf("\n - jcache sanity check -\n");

	/*
	 *	hashlist
	 */
	printf("\nhashlist display:\n");
	for (chp = jCache.HashTable, i = 0; i < jCache.nHashClass; chp++, i++)
	{
		for (bp = chp->ch_hashlist.head; bp; bp = bp->cm_hashlist.next)
		{
			nhash++;

			ip = bp->j_ip;
			printf("%3d: bp:0x%08x xflag:0x%08x flag:0x%08x ip:0x%08x\n",
				bp->whoami, bp, bp->j_xflag, bp->cm_flag, ip);

			ASSERT(bp->cm_hashlist.next != bp);
		}
	}
}
#endif	/* _JFS_DEBUG */
