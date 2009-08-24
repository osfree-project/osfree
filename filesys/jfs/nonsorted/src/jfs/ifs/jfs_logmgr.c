/* $Id: jfs_logmgr.c,v 1.1 2000/04/21 10:58:12 ktk Exp $ */

static char *SCCSID = "@(#)1.32.1.2  12/2/99 11:13:31 src/jfs/ifs/jfs_logmgr.c, sysjfs, w45.fs32, fixbld";
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
 *	jfs_logmgr.c: log manager
 *
 * for related information, see transaction manager (jfs_txnmgr.c),
 * recovery manager (jfs_logredo.c), and buffer manager (jfs_bufmgr.c).
 *
 * note: for detail, RTFS.
 *
 *	log buffer manager:
 * special purpose buffer manager supporting log i/o requirements.
 * per log serial pageout of logpage
 * queuing i/o requests and redrive i/o at iodone
 * maintain current logpage buffer
 * no caching since append only
 * appropriate jfs buffer cache buffers as needed
 *
 *	group commit:
 * transactions which wrote COMMIT records in the same in-memory
 * log page during the pageout of previous/current log page(s) are
 * committed together by the pageout of the page.
 *
 *	TBD lazy commit:
 * transactions are committed asynchronously when the log page
 * containing it COMMIT is paged out when it becomes full;
 *
 *	serialization:
 * . a per log lock serialize log write.
 * . a per log lock serialize group commit.
 * . a per log lock serialize log open/close;
 *
 *	TBD log integrity:
 * careful-write (ping-pong) of last logpage to recover from crash
 * in overwrite.
 * detection of split (out-of-order) write of physical sectors
 * of last logpage via timestamp at end of each sector
 * with its mirror data array at trailer).
 *
 *	alternatives:
 * lsn - 64-bit monotonically increasing integer vs
 * 32-bit lspn and page eor.
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif	/* _JFS_OS2 */
#include "mmph.h"                       //d201828

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_bufmgr.h"
#include "jfs_logmgr.h"
#include "jfs_txnmgr.h"
#include "jfs_proto.h"
#include "jfs_debug.h"


#ifdef _JFS_OS2
/*
 *	single external log
 */
extern void * _Seg16 logStrat2;		/* single external log dd strategy */
extern void (* APIENTRY logStrat3)();	/* single external log dd strategy */
#endif


/*
 *	log manager
 */
struct lCache {
	int32	nLog;		/* number of active log */
	LIST_HEADER(log)	logList;
	MUTEXLOCK_T		logListLock;
	logx_t	*logx;		/* logx freelist header */
	int32	freecnt;	/* free lbuf count */
	lbuf_t	*freelist;	/* lbuf freelist header */
	iobuf_t	*iobp;		/* iobuf freelist header */
} lCache;


/*
 *	log mount/umount serialization
 */
static int32	lockid = 0;	/* lock occurence number */

#define	LOGTBL_LOCK_ALLOC()\
	MUTEXLOCK_ALLOC(&lCache.logListLock, LOCK_ALLOC_PAGED, JFS_LOGTBL_LOCK_CLASS, -1)
#define	LOGTBL_LOCK_INIT() 	MUTEXLOCK_INIT(&lCache.logListLock)

#define LOGTBL_LOCK()		MUTEXLOCK_LOCK(&lCache.logListLock)
#define LOGTBL_UNLOCK()		MUTEXLOCK_UNLOCK(&lCache.logListLock)


/*
 *	log read/write serialization (per log)
 */
#define LOG_LOCK_ALLOC(log, lockid)\
        MUTEXLOCK_ALLOC(&(log)->loglock, LOCK_ALLOC_PAGED, JFS_LOG_LOCK_CLASS, lockid)
#define	LOG_LOCK_FREE(log)	MUTEXLOCK_FREE(&log->loglock)
#define LOG_LOCK_INIT(log)	MUTEXLOCK_INIT(&(log)->loglock)

#define LOG_LOCK(log)		MUTEXLOCK_LOCK(&((log)->loglock))
#define LOG_UNLOCK(log)		MUTEXLOCK_UNLOCK(&((log)->loglock))


/*
 *	log group commit serialization (per log)
 */
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
/*
 *	We now take this lock at interrupt time.  It must be a spinlock.
 */

#define LOGGC_LOCK_ALLOC(log, lockid)\
        XSPINLOCK_ALLOC(&(log)->gclock, LOCK_ALLOC_PIN, JFS_GC_LOCK_CLASS, lockid)
#define LOGGC_LOCK_FREE(log)	XSPINLOCK_FREE(&log->gclock)
#define LOGGC_LOCK_INIT(log)	XSPINLOCK_INIT(&(log)->gclock)

#define LOGGC_LOCK(log)		XSPINLOCK_LOCK(&((log)->gclock))
#define LOGGC_UNLOCK(log)	XSPINLOCK_UNLOCK(0,&((log)->gclock))

#define LOGGC_SLEEP(log, tblk)\
	XEVENT_SLEEP(&(tblk)->gcwait, &(log)->gclock, T_XSPINLOCK)
#define LOGGC_WAKEUP(tblk)\
	XEVENT_WAKEUP(&(tblk)->gcwait)

#else /* ! _JFS_LAZYCOMMIT */
// END D230860

#define LOGGC_LOCK_ALLOC(log, lockid)\
        MUTEXLOCK_ALLOC(&(log)->gclock, LOCK_ALLOC_PAGED, JFS_GC_LOCK_CLASS, lockid)
#define LOGGC_LOCK_FREE(log)	MUTEXLOCK_FREE(&log->gclock)
#define LOGGC_LOCK_INIT(log)	MUTEXLOCK_INIT(&(log)->gclock)

#define LOGGC_LOCK(log)		MUTEXLOCK_LOCK(&((log)->gclock))
#define LOGGC_UNLOCK(log)	MUTEXLOCK_UNLOCK(&((log)->gclock))

#define LOGGC_SLEEP(log, tblk)\
	EVENT_SLEEP(&(tblk)->gcwait, &(log)->gclock, T_MUTEXLOCK)
#define LOGGC_WAKEUP(tblk)\
	EVENT_WAKEUP(&(tblk)->gcwait)

#endif /* _JFS_LAZYCOMMIT */

/*
 *	log sync serialization (per log)
 */
#define	LOGSYNC_DELTA(logsize)		MIN((logsize)/8, 128*LOGPSIZE)
#define	LOGSYNC_BARRIER(logsize)	((logsize)/4)
/*
#define	LOGSYNC_DELTA(logsize)		MIN((logsize)/4, 256*LOGPSIZE)
#define	LOGSYNC_BARRIER(logsize)	((logsize)/2)
*/


/*
 *	log buffer cache synchronization
 */
XSPINLOCK_T	jfsLCacheLock;

#define	LCACHE_LOCK_ALLOC()\
	XSPINLOCK_ALLOC((void *)&jfsLCacheLock, LOCK_ALLOC_PIN, JFS_LCACHE_LOCK_CLASS, -1)
#define	LCACHE_LOCK_INIT()	XSPINLOCK_INIT(&jfsLCacheLock)
#define	LCACHE_LOCK()		XSPINLOCK_LOCK(&jfsLCacheLock)
#define	LCACHE_UNLOCK(xipl)	XSPINLOCK_UNLOCK(xipl, &jfsLCacheLock)
#define	LCACHE_SLEEP(event)\
	XEVENT_SLEEP(event, &jfsLCacheLock, T_XSPINLOCK)
#define	LCACHE_WAKEUP(event)	XEVENT_WAKEUP(event)


/*
 *	lbuf buffer cache (lCache) control
 */
/* log buffer manager pageout control (cumulative, inclusive) */
#define	lbmREAD		0x0001
#define	lbmWRITE	0x0002	/* enqueue at tail of write queue;
				 * init pageout if at head of queue;
				 */
#define	lbmRELEASE	0x0004	/* remove from write queue
				 * at completion of pageout;
				 * do not free/recycle it yet:
				 * caller will free it;
				 */
#define	lbmSYNC		0x0008	/* do not return to freelist
				 * when removed from write queue;
				 */
#define lbmFREE		0x0010	/* return to freelist
				 * at completion of pageout;
				 * the buffer may be recycled;
				 */
#define	lbmDONE		0x0020
#define	lbmERROR	0x0040
#define lbmGC		0x0080	/* lbmIODone to perform post-GC processing
				 * of log page			D230860
				 */

/*
 * external references
 */
int32	bmLogSync(struct log *log);
void	vPut(struct inode *ip);
#ifdef _JFS_LAZYCOMMIT
void	txLazyUnlock(tblock_t *tblk);				// D230860
extern int32 LazyCommitRunning;					// D233382
#endif

/*
 * forward references
 */
static int32 lmWriteRecord(log_t *log, tblock_t	*tblk, lrd_t *lrd, tlock_t *tlck);

static int32 lmNextPage(log_t *log, uint32 flag);
static int32 lmLogInit(log_t *log);
static int32 lmLogShutdown(log_t *log);
static int32 lmLogFileSystem(log_t *log, dev_t fsdev, int32 activate);

static int32 lbmLogInit(log_t *log);
static void lbmLogShutdown(log_t *log);
static lbuf_t *lbmAllocate(log_t *log);
static void lbmFree(lbuf_t *bp);
static void lbmfree(lbuf_t *bp, int32 *ipl);
static void lbmRelease(log_t *log, uint32 flag);
static int32 lbmRead(log_t *log, int32 pn, lbuf_t **bpp);
static void lbmWrite(log_t *log, lbuf_t *bp, uint32 flag);
static void lbmDirectWrite(log_t *log, lbuf_t *bp, uint32 flag);
static int32 lbmIOWait(lbuf_t *bp, uint32 flag);
static void lbmIODone(iobuf_t *ddbp);
static void lbmDirectIODone(iobuf_t *ddbp);
void lbmStartIO(lbuf_t *bp, void (*iodone)());
#ifdef _JFS_LAZYCOMMIT
void	lmGCwrite(log_t *log);					// D230860
#endif


#ifdef	_JFS_STATISTICS
/*
 *	statistics
 */
struct statLCache {
	uint32	commit;		/* # of commit */
	uint32	pageinit;	/* # of pages written */
	uint32	pagedone;	/* # of page write */
	uint32	sync;		/* # of logsysnc() */
	uint32	maxbufcnt;	/* max # of buffers allocated */
} statLCache = {0};
#endif	/* _JFS_STATISTICS */


/*
 * NAME:	lmLog()
 *
 * FUNCTION:	write a log record;
 *
 * PARAMETER:
 *
 * RETURN:	lsn - offset to the next log record to write (end-of-log);
 *		-1  - error;
 *
 * note: todo: log error handler
 */
int32
lmLog(
	log_t		*log,
	tblock_t	*tblk,
	lrd_t		*lrd,		/* log record descriptor */
	tlock_t		*tlck)
{
	logx_t	*logx = log->logx;
	int32	lsn;	
	int32	diffp, difft;
	jbuf_t	*bp = NULL;
	int32	ipl;

jEVENT(0,("lmLog: log:0x%08x tblk:0x%08x, lrd:0x%08x tlck:0x%08x\n",
	log, tblk, lrd, tlck));

	LOG_LOCK(log);

	/* log by (out-of-transaction) JFS ? */
        if (tblk == NULL)
		goto writeRecord;

	/* log from page ? */
	if (tlck == NULL ||
	    tlck->type & tlckBTROOT ||
	    (bp = tlck->bp) == NULL)
		goto writeRecord;

	ASSERT(bp->j_ip == tlck->ip);				// D231252

	/*
	 *	initialize/update page/transaction recovery lsn
	 */
	lsn = logx->lsn;

	ipl = LOGSYNC_LOCK(logx);

	/*
	 * initialize page lsn if first log write of the page
	 */
        if (bp->j_lsn == 0)
	{
		bp->j_logx = logx;
		bp->j_lsn = lsn;
		logx->count++;

		/* insert page at tail of logsynclist */
		CDLL_INSERT_TAIL(&logx->synclist, (logsyncblk_t *)bp, synclist);
	}

	/*
	 *	initialize/update lsn of tblock of the page
	 *
	 * transaction inherits oldest lsn of pages associated
	 * with allocation/deallocation of resources (their
	 * log records are used to reconstruct allocation map
	 * at recovery time: inode for inode allocation map,
	 * B+-tree index of extent descriptors for block
	 * allocation map);
	 * allocation map pages inherit transaction lsn at
	 * commit time to allow forwarding log syncpt past log
	 * records associated with allocation/deallocation of
	 * resources only after persistent map of these map pages
	 * have been updated and propagated to home.
	 */
	/*
	 * initialize transaction lsn:
	 */
        if (tblk->lsn == 0)
	{
		/* inherit lsn of its first page logged */
		tblk->lsn = bp->j_lsn;
		logx->count++;

		/* insert tblock after the page on logsynclist */
		CDLL_INSERT_AFTER(&logx->synclist,
				  (logsyncblk_t *)bp,
				  (logsyncblk_t *)tblk,
				  synclist);
	}
	/*
	 * update transaction lsn:
	 */
        else
        {
		/* inherit oldest/smallest lsn of page */
                logdiff(diffp, bp->j_lsn, logx);
                logdiff(difft, tblk->lsn, logx);
                if (diffp < difft)
		{
			/* update tblock lsn with page lsn */
                        tblk->lsn = bp->j_lsn;

			/* move tblock after page on logsynclist */
			CDLL_REMOVE(&logx->synclist,
				    (logsyncblk_t *)tblk, synclist);
			CDLL_INSERT_AFTER(&logx->synclist,
					  (logsyncblk_t *)bp,
				    	  (logsyncblk_t *)tblk,
					  synclist);
		}
        }

	LOGSYNC_UNLOCK(ipl, logx);

	/*
	 *	write the log record
	 */
writeRecord:
        lsn = lmWriteRecord(log, tblk, lrd, tlck);

	/*
	 * forward log syncpt if log reached next syncpt trigger
	 */
	logdiff(diffp, lsn, logx);
	if (diffp >= log->nextsync)
		lsn = lmLogSync(log, 0);

	/* update end-of-log lsn */
	logx->lsn = lsn;

out:
	LOG_UNLOCK(log);

	/* return end-of-log address */
	return lsn;
}


/*
 * NAME:	lmWriteRecord()
 *
 * FUNCTION:	move the log record to current log page
 *
 * PARAMETER:	cd	- commit descriptor
 *
 * RETURN:	end-of-log address
 *			
 * serialization: LOG_LOCK() held on entry/exit
 */
static int32
lmWriteRecord(
	log_t		*log,
	tblock_t	*tblk,
	lrd_t		*lrd,
	tlock_t		*tlck)
{
	int32	lsn;		/* end-of-log address */
	lbuf_t	*bp;		/* dst log page buffer */
	logpage_t	*lp;	/* dst log page */
	caddr_t	dst;		/* destination address in log page */
	int32	dstoffset;	/* end-of-log offset in log page */
	int32	freespace;	/* free space in log page */
	caddr_t	p;		/* src meta-data page */
	caddr_t	src;
	int32	srclen;
	int32	nbytes;		/* number of bytes to move */
	int32	i;
	int32	len;
	linelock_t	*linelock;
	lv_t	*lv;
	lvd_t	*lvd;
	int32	l2linesize;

	len = 0;

	/* retrieve destination log page to write */
	bp = (lbuf_t *)log->bp;
	lp = (logpage_t *)bp->l_ldata;
	dstoffset = log->eor;

	/* any log data to write ? */
	if (tlck == NULL)
		goto moveLrd;

	/*
	 *	move log record data
	 */
	/* retrieve source meta-data page to log */
	if (tlck->flag & tlckPAGELOCK)
	{
		p = (caddr_t)(tlck->bp->b_bdata);
		linelock = (linelock_t *)&tlck->lock;
	}
	/* retrieve source in-memory inode to log */
	else if (tlck->flag & tlckINODELOCK)
	{
		p = (caddr_t)&tlck->ip->i_btroot;
		linelock = (linelock_t *)&tlck->lock;
	}
#ifdef	_JFS_WIP
	else /* (tlck->flag & tlckINLINELOCK) */
	{
		inlinelock = (inlinelock_t *)&tlck;
		p = (caddr_t)&inlinelock->pxd;
		linelock = (linelock_t *)&tlck;
	}
#endif	/* _JFS_WIP */
	else
	{
jERROR(2,("lmWriteRecord: UFO tlck:0x%08x\n", tlck));
	}
	l2linesize = linelock->l2linesize;

moveData:
	ASSERT(linelock->index <= linelock->maxcnt);		// D231252

	lv = (lv_t *)&linelock->lv;
	for (i = 0; i < linelock->index; i++, lv++)
	{
		if (lv->length == 0)
			continue;

		/* is page full ? */
		if (dstoffset >= LOGPSIZE - LOGPTLRSIZE)
		{
			/* page become full: move on to next page */
			lmNextPage(log, 0);

			bp = log->bp;
			lp = (logpage_t *)bp->l_ldata;
			dstoffset = LOGPHDRSIZE;			
		}
		
		/*
		 * move log vector data
		 */
		src = (uint8 *)p + (lv->offset << l2linesize);
		srclen = lv->length << l2linesize;
		len += srclen;
		while (srclen > 0)
		{
			freespace = (LOGPSIZE - LOGPTLRSIZE) - dstoffset;
			nbytes = MIN(freespace, srclen);
			dst = (caddr_t)lp + dstoffset;
			bcopy(src, dst, nbytes);
			dstoffset += nbytes;

			/* is page not full ? */
			if (dstoffset < LOGPSIZE - LOGPTLRSIZE)
				break;

			/* page become full: move on to next page */
			lmNextPage(log, 0);

			bp = (lbuf_t *)log->bp;
			lp = (logpage_t *)bp->l_ldata;
			dstoffset = LOGPHDRSIZE;			

			srclen -= nbytes;	
			src += nbytes;
		}

		/*
		 * move log vector descriptor
		 */
		len += 4;
		lvd = (lvd_t *)((caddr_t)lp + dstoffset);
		lvd->offset = lv->offset;
		lvd->length = lv->length;
		dstoffset += 4;
jEVENT(0,("lmWriteRecord: lv offset:%d length:%d\n", lv->offset, lv->length));
	}

	if (i = linelock->next)
	{
		linelock = (linelock_t *)&TxLock[i];
		goto moveData;
	}

	/*
	 *	move log record descriptor
	 */
moveLrd:
	lrd->length = len;

	src = (caddr_t)lrd;
	srclen = LOGRDSIZE;

	while (srclen > 0)
	{
		freespace = (LOGPSIZE - LOGPTLRSIZE) - dstoffset;
		nbytes = MIN(freespace, srclen);
		dst = (caddr_t)lp + dstoffset;
		bcopy(src, dst, nbytes);

		dstoffset += nbytes;
		srclen -= nbytes;	

		/* are there more to move than freespace of page ? */
		if (srclen)
			goto pageFull;

		/*
		 * end of log record descriptor
		 */

		/* update last log record eor */
		log->eor = dstoffset;
		bp->l_eor = dstoffset;
		lsn = (log->page << L2LOGPSIZE) + dstoffset;

		if (lrd->type & LOG_COMMIT)
		{
			tblk->clsn = lsn;
jEVENT(0,("wr: tclsn:0x%08x, beor:0x%08x\n", tblk->clsn, bp->l_eor));

			if (!(tblk->xflag & COMMIT_SYNC))
				goto testPageFull;

			INCREMENT(statLCache.commit); /* # of commit */

			/*
			 * enqueue tblock for group commit:
			 *
			 * enqueue tblock of non-trivial/synchronous COMMIT
			 * at tail of group commit queue
			 * (trivial/asynchronous COMMITs are ignored by
			 * group commit.)
			 */
			LOGGC_LOCK(log);

			/* init tblock gc state */
			tblk->flag = tblkGC_QUEUE;
			tblk->bp = log->bp;
                        tblk->pn = log->page;
                        tblk->eor = log->eor;
			tblk->gcwait = EVENT_NULL;

                        /* enqueue transaction to commit queue */
			tblk->cqnext = NULL;
			if (log->cqueue.head)
			{
				log->cqueue.tail->cqnext = tblk;
				log->cqueue.tail = tblk;
			}
			else
				log->cqueue.head = log->cqueue.tail = tblk;

			LOGGC_UNLOCK(log);
		}

jEVENT(0,("lmWriteRecord: lrd:0x%04x bp:0x%08x pn:%d eor:0x%08x\n",
	lrd->type, log->bp, log->page, dstoffset));

testPageFull:
		/* page not full ? */
		if (dstoffset < LOGPSIZE - LOGPTLRSIZE)
			return lsn;

pageFull:
		/* page become full: move on to next page */
		lmNextPage(log, 0);

		bp = (lbuf_t *)log->bp;
		lp = (logpage_t *)bp->l_ldata;
		dstoffset = LOGPHDRSIZE;			
		src += nbytes;
	}

	return lsn;
}


/*
 * NAME:	lmNextPage()
 *
 * FUNCTION:	write current page and allocate next page.
 *
 * PARAMETER:	log
 *
 * RETURN:	0
 *			
 * serialization: LOG_LOCK() held on entry/exit
 */
static int32
lmNextPage(
	log_t	*log,
	uint32	flag)
{
	logpage_t	*lp;
	int32	lspn;		/* log sequence page number */
	int32	pn;		/* current page number */
	lbuf_t	*bp;
	lbuf_t	*nextbp;
	tblock_t	*tblk;

	INCREMENT(statLCache.pageinit); /* # of pages written */

	/* get current log page number and log sequence page number */
	pn = log->page;
	bp = log->bp;
	lp = (logpage_t *)bp->l_ldata;
	lspn = lp->h.page;

	LOGGC_LOCK(log);

	/*
	 *	write or queue the full page at the tail of write queue
	 */
	/* get the tail tblk on commit queue */
	tblk = log->cqueue.tail;

	/* every tblk who has COMMIT record on the current page,
	 * and has not been committed, must be on commit queue
	 * since tblk is queued at commit queueu at the time
	 * of writing its COMMIT record on the page before
	 * page becomes full (even though the tblk thread
	 * who wrote COMMIT record may have been suspended
	 * currently);
	 */

	/* is page bound with outstanding tail tblk ? */
	if (tblk && tblk->pn == pn)
	{
		/* mark tblk for end-of-page */
		tblk->flag |= tblkGC_EOP;

		/* if page is not already on write queue,
		 * just enqueue (no lbmWRITE to prevent redrive)
		 * buffer to wqueue to ensure correct serial order
		 * of the pages since log pages will be added
		 * continuously (tblk bound with the page hasn't
		 * got around to init write of the page, either
		 * preempted or the page got filled by its COMMIT
		 * record);
		 * pages with COMMIT are paged out explicitly by
		 * tblk in lmGroupCommit();
		 */
		if (bp->l_wqnext == NULL)
		{
			/* bp->l_ceor = bp->l_eor; */
			/* lp->h.eor = lp->t.eor = bp->l_ceor; */
			lbmWrite(log, bp, 0);
		}
	}
	/* page is not bound with outstanding tblk:
	 * init write or mark it to be redriven (lbmWRITE)
	 */
	else
	{
		/* finalize the page */
		bp->l_ceor = bp->l_eor;
		lp->h.eor = lp->t.eor = bp->l_ceor;
		lbmWrite(log, bp, lbmWRITE|lbmRELEASE|lbmFREE);
	}
	LOGGC_UNLOCK(log);

	/*
	 *	allocate/initialize next page
	 */
	/* if log wraps, the first data page of log is 2
	 * (0 never used, 1 is superblock).
	 */
	log->page = (pn == log->size - 1) ? 2 : pn + 1;
	log->eor = LOGPHDRSIZE;	/* ? valid page empty/full at logRedo() */

	/* allocate/initialize next log page buffer */
       	nextbp = lbmAllocate(log);
	nextbp->l_pn = log->page;
	nextbp->l_eor = log->eor;
	log->bp = nextbp;

	/* initialize next log page */
	lp = (logpage_t *)nextbp->l_ldata;
	lp->h.page = lp->t.page = lspn + 1;
	lp->h.eor = lp->t.eor = LOGPHDRSIZE;

        return 0;
}
	

#ifdef _JFS_LAZYCOMMIT
/*
 * NAME:	lmGroupCommit()
 *
 * FUNCTION:	group commit
 *	initiate pageout of the pages with COMMIT in the order of
 *	page number - redrive pageout of the page at the head of
 *	pageout queue until full page has been written.
 *
 * RETURN:	
 *
 * NOTE:
 *	LOGGC_LOCK serializes log group commit queue, and
 *	transaction blocks on the commit queue.
 *	N.B. LOG_LOCK is NOT held during lmGroupCommit().
 */
int32
lmGroupCommit(
	log_t		*log,
	tblock_t	*tblk)
{
	int32	rc = 0;
	logx_t	*logx = log->logx;
	tblock_t *gltblk;	/* group leader transaction block */
	int32	tclsn, lclsn;

	LOGGC_LOCK(log);

	/* group committed already ? */
	if (tblk->flag & tblkGC_COMMITTED)
	{
		if (tblk->flag & tblkGC_ERROR)
			rc = EIO;

		LOGGC_UNLOCK(log);
		return rc;
	}
jEVENT(0,("lmGroup Commit: tblk = 0x%x, gcrtc = %d\n", tblk, log->gcrtc));

	/*
	 * group commit pageout in progress
	 */
	if ((!(log->cflag & logGC_PAGEOUT)) && log->cqueue.head)  // D233382
	{
		/*
		 * only transaction in the commit queue:
		 *
		 * start one-transaction group commit as
		 * its group leader.
		 */
		log->cflag |= logGC_PAGEOUT;

		lmGCwrite(log);
	}

// BEGIN D233382
	/* lmGCwrite gives up LOGGC_LOCK, check again */

	if (tblk->flag & tblkGC_COMMITTED)
	{
		if (tblk->flag & tblkGC_ERROR)
			rc = EIO;

		LOGGC_UNLOCK(log);
		return rc;
	}

	/* upcount transaction waiting for completion
	 */
	log->gcrtc++;
// END D233382

	if (tblk->xflag & COMMIT_LAZY)
	{
		tblk->flag |= tblkGC_LAZY;
		LOGGC_UNLOCK(log);
		return 0;
	}
	tblk->flag |= tblkGC_READY;

	LOGGC_SLEEP(log, tblk);

	assert (tblk->flag & tblkGC_COMMITTED)

	/* removed from commit queue */
	if (tblk->flag & tblkGC_ERROR)
		rc = EIO;

	LOGGC_UNLOCK(log);
	return rc;
}

/*
 * NAME:	lmGCwrite()
 *
 * FUNCTION:	group commit write
 *	initiate write of log page, building a group of all transactions
 *	with commit records on that page.
 *
 * RETURN:	None
 *
 * NOTE:
 *	LOGGC_LOCK must be held by caller.
 *	N.B. LOG_LOCK is NOT held during lmGroupCommit().
 */
void	lmGCwrite(
log_t	*log)
{
	lbuf_t		*bp;
	logpage_t	*lp;
	int32		gcpn;		/* group commit page number */
	tblock_t 	*tblk;
	tblock_t 	*xtblk;

	/*
	 * build the commit group of a log page
	 *
	 * scan commit queue and make a commit group of all
	 * transactions with COMMIT records on the same log page.
	 */
	/* get the head tblk on the commit queue */
	tblk = xtblk = log->cqueue.head;
	gcpn = tblk->pn;

	while (tblk && tblk->pn == gcpn)
	{
		xtblk = tblk;

		/* state transition: (QUEUE, READY) -> COMMIT */
		tblk->flag |= tblkGC_COMMIT;
		tblk = tblk->cqnext;
	}
	tblk = xtblk;	/* last tblk of the page */

	/*
	 * pageout to commit transactions on the log page.
	 */
	bp = (lbuf_t *)tblk->bp;
	lp = (logpage_t *)bp->l_ldata;
	/* is page already full ? */
	if (tblk->flag & tblkGC_EOP)
	{
		/* mark page to free at end of group commit of the page */
		tblk->flag &= ~tblkGC_EOP;
		tblk->flag |= tblkGC_FREE;
		bp->l_ceor = bp->l_eor;
		lp->h.eor = lp->t.eor = bp->l_ceor;
jEVENT(0,("gc: tclsn:0x%08x, bceor:0x%08x\n", tblk->clsn, bp->l_ceor));
		lbmWrite(log, bp, lbmWRITE|lbmRELEASE|lbmGC);
	}
	/* page is not yet full */
	else
	{
		bp->l_ceor = tblk->eor; /* ? bp->l_ceor = bp->l_eor; */
		lp->h.eor = lp->t.eor = bp->l_ceor;
jEVENT(0,("gc: tclsn:0x%08x, bceor:0x%08x\n", tblk->clsn, bp->l_ceor));
		lbmWrite(log, bp, lbmWRITE|lbmGC);
	}
}

/*
 * NAME:	lmPostGC()
 *
 * FUNCTION:	group commit post-processing
 *	Processes transactions after their commit records have been written
 *	to disk, redriving log I/O if necessary.
 *
 * RETURN:	None
 *
 * NOTE:
 *	This routine is called a interrupt time by lbmIODone
 */
void	lmPostGC(
lbuf_t	*bp)
{
	log_t		*log = bp->l_log;
	logpage_t	*lp;
	tblock_t	*tblk;

	LOGGC_LOCK(log);
	/*
	 * current pageout of group commit completed.
	 *
	 * remove/wakeup transactions from commit queue who were
	 * group committed with the current log page
	 */
	while ((tblk = log->cqueue.head) && (tblk->flag & tblkGC_COMMIT))
	{
		/* if transaction was marked GC_COMMIT then
		 * it has been shipped in the current pageout
		 * and made it to disk - it is committed.
		 */

		if (bp->l_flag & lbmERROR)
			tblk->flag |= tblkGC_ERROR;

		/* remove it from the commit queue */
		log->cqueue.head = tblk->cqnext;
		if (log->cqueue.head == NULL)
			log->cqueue.tail = NULL;
		tblk->flag &= ~tblkGC_QUEUE;
		tblk->cqnext = 0;

jEVENT(0,("lmPostGC: tblk = 0x%x, flag = 0x%x\n", tblk, tblk->flag));

// BEGIN D233382
		if (LazyCommitRunning && !(tblk->xflag & COMMIT_FORCE))
			/*
			 * Hand tblk over to lazy commit thread
			 */
			txLazyUnlock(tblk);
		else
		{
			/* state transition: COMMIT -> COMMITTED */
			tblk->flag |= tblkGC_COMMITTED;

			if (tblk->flag & tblkGC_READY)
			{
				log->gcrtc--;
				LOGGC_WAKEUP(tblk);
			}
		}
// END D233382

		/* was page full before pageout ?
		 * (and this is the last tblk bound with the page)
		 */
		if (tblk->flag & tblkGC_FREE)
			lbmFree(bp);
		/* did page become full after pageout ?
		 * (and this is the last tblk bound with the page)
		 */
		else if (tblk->flag & tblkGC_EOP)
		{
			/* finalize the page */
			lp = (logpage_t *)bp->l_ldata;
			bp->l_ceor = bp->l_eor;
			lp->h.eor = lp->t.eor = bp->l_eor;
jEVENT(0,("lmPostGC: calling lbmWrite\n"));
			lbmWrite(log, bp, lbmWRITE|lbmRELEASE|lbmFREE);
		}

	}

	/* are there any transactions who have entered lnGroupCommit()
	 * (whose COMMITs are after that of the last log page written.
	 * They are waiting for new group commit (above at (SLEEP 1)):
	 * select the latest ready transaction as new group leader and
	 * wake her up to lead her group.
	 */
	if ((log->gcrtc > 0) && log->cqueue.head)		// D233382
		/*
		 * Call lmGCwrite with new group leader
		 */
		lmGCwrite(log);

	/* no transaction are ready yet (transactions are only just
	 * queued (GC_QUEUE) and not entered for group commit yet).
	 * let the first transaction entering group commit
	 * will elect hetself as new group leader.
	 */
	else
		log->cflag &= ~logGC_PAGEOUT;

	LOGGC_UNLOCK(log);
	return;
}

#else /* ! _JFS_LAZYCOMMIT */

/*
 * NAME:	lmGroupCommit()
 *
 * FUNCTION:	group commit
 *	initiate pageout of the pages with COMMIT in the order of
 *	page number - redrive pageout of the page at the head of
 *	pageout queue until full page has been written.
 *
 * RETURN:	
 *
 * NOTE:
 *	LOGGC_LOCK serializes log group commit queue, and
 *	transaction blocks on the commit queue.
 *	N.B. LOG_LOCK is NOT held during lmGroupCommit().
 */
int32
lmGroupCommit(
	log_t		*log,
	tblock_t	*tblk)
{
	int32	rc = 0;
	logx_t	*logx = log->logx;
	tblock_t *gltblk;	/* group leader transaction block */
	tblock_t *xtblk;
	int32	glcpn;		/* group leader commit page number */
	int32	gcpn;		/* group commit page number */
	int32	gctc;		/* group commit transaction count */
	lbuf_t	*bp;
	logpage_t	*lp;
	int32	tclsn, lclsn;

	LOGGC_LOCK(log);

	/* group committed already ? */
	if (tblk->flag & tblkGC_COMMITTED)
	{
		if (tblk->flag & tblkGC_ERROR)
			rc = EIO;

		LOGGC_UNLOCK(log);
		return rc;
	}

	/*
	 * group commit pageout in progress
	 */
	if (log->cflag & logGC_PAGEOUT)
	{
		/*
		 * transactions which enter this point are in commit
		 * queue but didn't arrive before the pageout,
		 * so they do not have their GC_COMMIT flag on
		 * (when the current pageout completes the page will
		 * need to be rewritten to guarantee that the transaction
		 * is on disk).
		 */
		/* state transition: QUEUE -> READY
		 * upcount transaction who can be asked for work;
		 */
		tblk->flag |= tblkGC_READY;
		log->gcrtc++;

		/*
		 * wait for next group commit.
		 *
		 * tblks may enter lmGroupCommit() not necessarily
		 * in the order of COMMIT records;
		 * record the latest ready transaction
		 * to select the next group leader.
		 */
		if (log->gclrt->flag & tblkGC_COMMIT)
			log->gclrt = tblk;	/* We're the first READY txn */
		else
		{
			logdiff(tclsn, tblk->clsn, logx);
			logdiff(lclsn, log->gclrt->clsn, logx);
			if (tclsn > lclsn)
				log->gclrt = tblk;
		}

		LOGGC_SLEEP(log, tblk);

		/*
		 * waken up from sleep waiting for new group commit above.
		 *
		 * if it is alreday committed, another transaction had
		 * been selected as group leader and has completed a new
		 * group commit: got free ride - done.
		 */
		if (tblk->flag & tblkGC_COMMITTED)
		{
			/* removed from commit queue */
			if (tblk->flag & tblkGC_ERROR)
				rc = EIO;

			LOGGC_UNLOCK(log);
			return rc;
		}

		/*
		 * selected as a new group leader
		 *
		 * if it is waken up and its flag isn't GC_COMMITTED,
		 * then it has been selected as new/next group leader.
		 * clear its GC_READY flag since, upon completion of
		 * group commit, this flag is used to signal that a
		 * wakeup is needed for free riders of the group
		 * (it won't need to wake itself up).
		 */
		assert(log->cflag & logGC_PAGEOUT);
		tblk->flag &= ~tblkGC_READY;
		log->gcrtc--;
	} 	
	/*
	 * group commit pageout is NOT in progress
	 */
	else
	{
		/*
		 * only transaction in the commit queue:
		 *
		 * start one-transaction group commit as
		 * its group leader.
		 */
		assert(log->gcrtc == 0);
		log->cflag |= logGC_PAGEOUT;
		log->gclrt = tblk;
	}

	/*
	 * if a transaction has reached this point,
	 * then it is the group leader of current group commit.
	 */

	/*
	 *	group commit transactions per log page.
	 *
	 * (transactions in comit queue are in the order of
	 * COMMIT records on the log page(s).
	 * the group leader is usually the last in the commit
	 * queue and may not be on the current page to pageout).
	 *
	 * interleaved pages without any COMMIT are paged out
	 * by lbm as redriving pageout at pageout completion.
	 */
	tblk->flag |= tblkGC_LEADER;
	gltblk = tblk;
	glcpn  = gltblk->pn;

	/*
	 * build the commit group of a log page
	 *
	 * scan commit queue and make a commit group of all
	 * transactions with COMMIT records on the same log page.
	 */
next_page:
	/* get the head tblk on the commit queue */
	tblk = xtblk = log->cqueue.head;
	gcpn = tblk->pn;

	while (tblk && tblk->pn == gcpn)
	{
		xtblk = tblk;

		/* state transition: (QUEUE, READY) -> COMMIT */
		tblk->flag |= tblkGC_COMMIT;

		tblk = tblk->cqnext;
	}
	tblk = xtblk;	/* last tblk of the page */

	/*
	 * synchronous pageout to commit transactions on the log page.
	 */
	bp = (lbuf_t *)tblk->bp;
	lp = (logpage_t *)bp->l_ldata;
	/* is page already full ? */
	if (tblk->flag & tblkGC_EOP)
	{
		/* mark page to free at end of group commit of the page */
		tblk->flag &= ~tblkGC_EOP;
		tblk->flag |= tblkGC_FREE;
		bp->l_ceor = bp->l_eor;
		lp->h.eor = lp->t.eor = bp->l_ceor;
jEVENT(0,("gc: tclsn:0x%08x, bceor:0x%08x\n", tblk->clsn, bp->l_ceor));
		lbmWrite(log, bp, lbmWRITE|lbmRELEASE|lbmSYNC);
	}
	/* page is not yet full */
	else
	{
		bp->l_ceor = tblk->eor; /* ? bp->l_ceor = bp->l_eor; */
		lp->h.eor = lp->t.eor = bp->l_ceor;
jEVENT(0,("gc: tclsn:0x%08x, bceor:0x%08x\n", tblk->clsn, bp->l_ceor));
		lbmWrite(log, bp, lbmWRITE|lbmSYNC);
	}
	
	LOGGC_UNLOCK(log);

	/*
	 * wait for pageout completion
	 */
        rc = lbmIOWait(bp, 0);

jEVENT(0,("gc(%d):%d:%d JFS LOG on fire!\n", gltblk->logtid, gcpn, tblk->eor));

	LOGGC_LOCK(log);
	
	/*
	 * current pageout of group commit completed.
	 *
	 * remove/wakeup transactions from commit queue who were
	 * group committed with the current log page
	 */
	tblk = log->cqueue.head;
	while (tblk && tblk->flag & tblkGC_COMMIT)
	{
		/* if transaction was marked GC_COMMIT then
		 * it has been shipped in the current pageout
		 * and made it to disk - it is committed.
		 */
		/* state transitoon: COMMIT -> COMMITTED */
		tblk->flag |= tblkGC_COMMITTED;

		if (rc)
			tblk->flag |= tblkGC_ERROR;

		/* remove it from the commit queue */
		log->cqueue.head = tblk->cqnext;
		if (log->cqueue.head == NULL)
			log->cqueue.tail = NULL;
		tblk->flag &= ~tblkGC_QUEUE;

		/* was transaction sleeping ? */
		if (tblk->flag & tblkGC_READY)
		{
			log->gcrtc--;
			LOGGC_WAKEUP(tblk);
		}

		/* was page full before pageout ?
		 * (and this is the last tblk bound with the page)
		 */
		if (tblk->flag & tblkGC_FREE)
			lbmFree(bp);
		/* did page become full after pageout ?
		 * (and this is the last tblk bound with the page)
		 */
		else if (tblk->flag & tblkGC_EOP)
		{
			/* finalize the page */
			bp->l_ceor = bp->l_eor;
			lp->h.eor = lp->t.eor = bp->l_eor;
			lbmWrite(log, bp, lbmWRITE|lbmRELEASE|lbmFREE);
		}

		tblk = tblk->cqnext;
	}


	/* continue group commit up to and including the log page
	 * of group commit leader;
	 * if there are new tblk (not part of current group commit)
	 * on the current page who were involved in filling up the page,
	 * the page will be written again as new group commit;
	 * or new/next page will be written as new group commit;
	 */
	if (gcpn != glcpn)
		goto next_page;

	/*
	 *	group commit completed for the group leader
	 */
	/* are there any transactions who have entered lnGroupCommit()
	 * (whose COMMITs are after that of the last group commit reader
	 * since all tblks who precede gc leader has been removed)
	 * while current group commit ?
	 * they are waiting for new group commit (above at (SLEEP 1)):
	 * select the latest ready transaction as new group leader and
	 * wake her up to lead her group.
	 */
	if (log->gcrtc > 0)
	{
		/* leave in LOGC_GCPAGEOUT state guaranteeing
		 * the transaction awoken will become the new leader:
		 * this will avoid a wasted wakeup.
		 */
		tblk = log->gclrt;
		assert(tblk->flag & tblkGC_QUEUE);
		assert(tblk->flag & tblkGC_READY);

		LOGGC_WAKEUP(tblk);
	}
	/* no transaction are ready yet (transactions are only just
	 * queued (GC_QUEUE) and not entered for group commit yet).
	 * let the first transaction entering group commit
	 * will elect hetself as new group leader.
	 */
	else
		log->cflag &= ~logGC_PAGEOUT;

	LOGGC_UNLOCK(log);

	return rc;
}
#endif /* _JFS_LAZYCOMMIT */


/*
 * NAME:	lmLogSync()
 *
 * FUNCTION:	write log SYNCPT record for specified log
 *	if new sync address is available
 *	(normally the case if sync() is executed by back-ground
 *	process).
 *	if not, explicitly run jfs_blogsync() to initiate
 *	getting of new sync address.
 *	calculate new value of i_nextsync which determines when
 *	this code is called again.
 *
 *	this is called only from lmLog().
 *
 * PARAMETER:	ip	- pointer to logs inode.
 *
 * RETURN:	0
 *			
 * serialization: LOG_LOCK() held on entry/exit
 */
int32
lmLogSync(
	log_t	*log,
	int32	nosyncwait)
{
	logx_t	*logx = log->logx;
	int32	logsize;
	int32	written;	/* written since last syncpt */
	int32	free;		/* free space left available */
	int32	delta;		/* additional delta to write normally */
	int32	more;		/* additional write granted */
	lrd_t	lrd;
	int32	lsn;

	/*
	 *	forward syncpt
	 */
	/* if last sync is same as last syncpt,
	 * invoke sync point forward processing to update sync.
	 */
	if (logx->sync == logx->syncpt)
		bmLogSync(log);

	/* if sync is different from last syncpt,
	 * write a SYNCPT record with syncpt = sync.
	 * reset syncpt = sync
	 */
	if (logx->sync != logx->syncpt)
	{
		lrd.logtid = 0;
		lrd.backchain = 0;
		lrd.type = LOG_SYNCPT;
		lrd.length = 0;
		lrd.log.syncpt.sync = logx->sync;
		lsn = lmWriteRecord(log, NULL, &lrd, NULL);

		logx->syncpt = logx->sync;
	}
	else
		lsn = logx->lsn;

	/*
	 *	setup next syncpt trigger (SWAG)
	 */
	logsize = logx->size;

	logdiff(written, lsn, logx);
	free = logsize - written;
	delta = LOGSYNC_DELTA(logsize);
	more = MIN(free/2, delta);
	if (more < 2*LOGPSIZE)
	{
jALERT(1,("\n ... Log Wrap ... Log Wrap ... Log Wrap ...\n\n"));
		/*
		 *	log wrapping
		 *
		 * option 1 - panic ? No.!
		 * option 2 - shutdown file systems
		 *	      associated with log ?
		 * option 3 - extend log ?
		 */
		/*
		 * option 4 - second chance
		 *
		 * mark log wrapped, and continue.
		 * when all active transactions are completed,
		 * mark log vaild for recovery.
		 * if crashed during invalid state, log state
		 * implies invald log, forcing fsck().
		 */
		/* mark log state log wrap in log superblock */
		/* log->state = LOGWRAP; */

		/* reset sync point computation */
		logx->syncpt = logx->sync = lsn;
		log->nextsync = delta;
	}		
	else	
		/* next syncpt trigger = written + more */
		log->nextsync = written + more;

	/* return if lmLogSync() from outside of transaction, e.g., sync() */
	if (nosyncwait)
		return lsn;

	/* if number of bytes written from last sync point is more
	 * than 1/4 of the log size, stop new transactions from
	 * starting until all current transactions are completed
	 * by setting syncbarrier flag.
	 */
	if (written > LOGSYNC_BARRIER(logsize) &&
	    logsize > 32*LOGPSIZE)
	{
		log->syncbarrier = 1;
jALERT(0,("	log barrier on: 0x%08x(%d:%d)\n", lsn, written, free));
	}

	return lsn;
}


/*
 * NAME:	lmSync()
 *
 * FUNCTION: compute new syncpoint address for each active log in the JFS
 *
 * serialization: serialization wrt lmLogInit()/lmLogShutdown()
 *	on existence of log
 */
void
lmSync(log_t	*mylog)
{
        int32	k;
	log_t	*log;

	/*
	 * single specific log sync
	 */
	if (mylog)
	{
		LOG_LOCK(mylog);

               	lmLogSync(mylog, 1);

		LOG_UNLOCK(mylog);

		return;
	}

	/*
	 * sync each active log under JFS
	 */
	LOGTBL_LOCK();

	for (log = lCache.logList.head; log; log = log->loglist.next)
        {
               	/*
		 * sync the log:
		 */
		LOG_LOCK(log);

                lmLogSync(log, 1);

		LOG_UNLOCK(log);
        }

	LOGTBL_UNLOCK();
}


/*
 * NAME:	lmLogOpen()
 *
 * FUNCTION:    open the log on first open;
 *	insert filesystem in the active list of the log.
 *
 * PARAMETER:	ipmnt	- file system mount inode
 *		iplog 	- log inode (out)
 *
 * RETURN:
 *
 * serialization:
 */
int32
lmLogOpen(
	inode_t	*ipmnt,
	inode_t	**iplog)
{
	int32	rc;
	dev_t	fsdev;		/* dev_t of file system */
	dev_t	logdev;		/* dev_t of log device */
	struct file *logdevfp;	/* file ptr for log device */
	inode_t	*ip;
	log_t	*log;
	union mntvfs	dummyvfs, *mntvfsp;
	int32	i;

jEVENT(0,("lmLogOpen: ipmnt:0x%08x\n", ipmnt));

	logdev = ipmnt->i_logdev;
	fsdev = ipmnt->i_dev;

	/*
	 * open the inode representing the log device (aka log inode)
	 */
	if (logdev != fsdev)
		goto externalLog;

	/*
	 *	in-line log in host file system
	 *
	 * file system to log have 1-to-1 relationship;
	 */
inlineLog:
	/*
	 * open log inode
	 *
	 * log inode is reserved inode of (dev_t = log device,
	 * fileset number = 0, i_number = LOG_I)
	 */
	mntvfsp = (union mntvfs *)&ipmnt->i_mntvfs;
	ICACHE_LOCK();
	/* force new inode allocation/initialization (IGET_IALLOC) */
	rc = iget((struct vfs *)mntvfsp, LOG_I, (inode_t **)&log, IGET_IALLOC);
	ICACHE_UNLOCK();
	if (rc)
		return rc;

	log->flag = JFS_INLINELOG;
	log->dev = logdev;
 	log->base = LBLK2PBLK(ipmnt, addressPXD(&ipmnt->i_logpxd));
	log->size = (lengthPXD(&ipmnt->i_logpxd) << ipmnt->i_l2bsize) >> L2LOGPSIZE;
	log->l2pbsize = ipmnt->i_l2pbsize;
#ifdef _JFS_OS2
	log->strat2p = ipmnt->i_Strategy2;
	log->strat3p = ipmnt->i_Strategy3;
#endif /* _JFS_OS2 */

	ip = (inode_t *)log;
	IWRITE_LOCK(ip);

	/*
	 * initialize log.
	 */
	if (rc = lmLogInit(log))
		goto errout10;

	goto out;

	/*
	 *	external log as separate logical volume
	 *
	 * file systems to log may have n-to-1 relationship;
	 */
externalLog:
	/*
	 * open log inode
	 *
	 * log inode is reserved inode of (dev_t = log device,
	 * fileset number = 0, i_number = 0), which acquire
	 * one i_count for each open by file system.
	 *
	 * hand craft dummy vfs to force iget() the special case of
	 * an in-memory inode allocation without on-disk inode
	 */
	memset(&dummyvfs, 0, sizeof(struct vfs));
	dummyvfs.filesetvfs.vfs_data = NULL;
	dummyvfs.dummyvfs.dev = logdev;
	dummyvfs.dummyvfs.ipmnt = NULL;
	ICACHE_LOCK();
	rc = iget((struct vfs *)&dummyvfs, 0, (inode_t **)&log, 0);
	ICACHE_UNLOCK();
	if (rc)
		return rc;

	log->flag = 0;
	log->dev = logdev;
	log->base = 0;
	log->size = 0;

	/*
	 * serialize open/close between multiple file systems
	 * bound with the log;
	 */
	ip = (inode_t *)log;
	IWRITE_LOCK(ip);

	/*
	 * subsequent open: add file system to log active file system list
	 */
#ifdef	_JFS_OS2
	if (log->strat2p)
#endif	/* _JFS_OS2 */
	{	
		if (rc = lmLogFileSystem(log, fsdev, 1))
			goto errout10;

		IWRITE_UNLOCK(ip);

		*iplog = ip;
jEVENT(0,("lmLogOpen: exit(0)\n"));
		return 0;	
	}

	/* decouple log inode from dummy vfs */
	vPut(ip);

	/*
	 * first open:
	 */
#ifdef	_JFS_OS2
	/*
	 * establish access to the single/shared (already open) log device
	 */
	logdevfp = (void *)logStrat2;
	log->strat2p = logStrat2;
	log->strat3p = logStrat3;

	log->l2pbsize = 9; /* todo: when OS/2 have multiple external log */
#endif	/* _JFS_OS2 */

	/*
	 * initialize log:
	 */
	if (rc = lmLogInit(log))
		goto errout20;

	/*
	 * add file system to log active file system list
	 */
	if (rc = lmLogFileSystem(log, fsdev, 1))
		goto errout30;

	/*
	 *	insert log device into log device list
	 */
out:
	LOGTBL_LOCK();
	LIST_INSERT_HEAD(&lCache.logList, log, loglist);
	lCache.nLog++;
	LOGTBL_UNLOCK();

	IWRITE_UNLOCK(ip);

	*iplog = ip;

jEVENT(0,("lmLogOpen: exit(0)\n"));
	return 0;

	/*
	 *	unwind on error
	 */
errout30: /* unwind lbmLogInit() */
	lbmLogShutdown(log);

errout20: /* close external log device */

errout10: /* free log inode */
	IWRITE_UNLOCK(ip);

	ICACHE_LOCK();
	iput(ip, NULL);
	ICACHE_UNLOCK();

jEVENT(0,("lmLogOpen: exit(rc)\n", rc));
	return rc;
}


/*
 * NAME:	lmLogInit()
 *
 * FUNCTION:	log initialization at first log open.
 *
 *	logredo() (or logformat()) should have been run previously.
 *	initialize the log inode from log superblock.
 *	set the log state in the superblock to LOGMOUNT and
 *	write SYNCPT log record.
 *		
 * PARAMETER:	log	- log inode
 *
 * RETURN:	0	- if ok
 *		EINVAL	- bad log magic number
 *		EFORMAT	- log not processed by logredo
 *		error returned from logwait()
 *			
 * serialization: single first open thread
 */
static int32
lmLogInit(
	log_t *log)		/* log inode */
{
	int32	rc = 0;
	logx_t	*logx;
	lrd_t	lrd;
	logsuper_t	*logsuper;
	lbuf_t	*bpsuper;
	lbuf_t	*bp;
	logpage_t	*lp;
	int32	lsn;

jEVENT(0,("lmLogInit: log:0x%08x\n", log));

	/*
	 * log inode is overlaid on generic inode where
	 * dinode have been zeroed out by iRead();
	 */

	/*
	 * initialize log i/o
	 */
	if (rc = lbmLogInit(log))
		return rc;

	/*
	 * validate log superblock
	 */
	if (rc = lbmRead(log, 1, &bpsuper))
		goto errout10;

	logsuper = (logsuper_t *)bpsuper->l_ldata;

	if (logsuper->magic != LOGMAGIC)
	{
jERROR(1,("*** Log Format Error ! ***\n"));
		rc = EFORMAT;
		goto errout20;
	}

	/* logredo() should have been run successfully. */
	if (logsuper->state != LOGREDONE)
	{
jERROR(1,("*** Log Is Dirty ! ***\n"));
		rc = EFORMAT;
		goto errout20;
	}

	/* initialize log inode from log superblock */
	if (log->flag & JFS_INLINELOG)
	{
		if (log->size != logsuper->size)
		{
			rc = EFORMAT;
			goto errout20;
		}
jFYI(0,("lmLogInit: inline log:0x%08x base:0x%x%08x size:0x%08x\n",
	log, log->base, log->size));
	}
	else
	{
		log->size = logsuper->size;
jFYI(0,("lmLogInit: external log:0x%08x base:0x%x%08x size:0x%08x\n",
	log, log->base, log->size));
	}

	log->flag |= JFS_GROUPCOMMIT;
/*
	log->flag |= JFS_LAZYCOMMIT;
*/
	log->page = logsuper->end/LOGPSIZE;
	log->eor = logsuper->end - (LOGPSIZE * log->page);

	/*
	 * initialize for log append write mode
	 */
	/* establish current/end-of-log page/buffer */
	if (rc = lbmRead(log, log->page, &bp))
		goto errout20;

	lp = (logpage_t *)bp->l_ldata;

jEVENT(0,("lmLogInit: lsn:0x%08x page:%d eor:%d:%d\n",
	logsuper->end, log->page, log->eor, lp->h.eor));

	ASSERT(log->eor == lp->h.eor);				// D230860

	log->bp = bp;
	bp->l_pn = log->page;
	bp->l_eor = log->eor;

        /* allocate/initialize the group commit serialization lock */
	LOGGC_LOCK_ALLOC(log, lockid);
	LOGGC_LOCK_INIT(log);

	/* if current page is full, move on to next page */
	if (log->eor >= LOGPSIZE - LOGPTLRSIZE)
		lmNextPage(log, 0);

        /* allocate/initialize the log write serialization lock */
	lockid++;
	LOG_LOCK_ALLOC(log, lockid);
	LOG_LOCK_INIT(log);

	/*
	 * initialize log syncpoint
	 */
	/*
	 * write the first SYNCPT record with syncpoint = 0
	 * (i.e., log redo up to HERE !);
	 * remove current page from lbm write queue at end of pageout
	 * (to write log superblock update), but do not release to freelist;
	 */
	lrd.logtid = 0;
	lrd.backchain = 0;
	lrd.type = LOG_SYNCPT;
	lrd.length = 0;
	lrd.log.syncpt.sync = 0;
	lsn = lmWriteRecord(log, NULL, &lrd, NULL);
	bp = log->bp;
	bp->l_ceor = bp->l_eor;
	lp = (logpage_t *)bp->l_ldata;
	lp->h.eor = lp->t.eor = bp->l_eor;
	lbmWrite(log, bp, lbmWRITE|lbmSYNC);
	if (rc = lbmIOWait(bp, 0))
		goto errout30;

	/* initialize logsync parameters */
	logx = log->logx;
	logx->flag = log->flag;
	logx->size = (log->size - 2) << L2LOGPSIZE;
	logx->lsn = lsn;
	logx->syncpt = lsn;
	logx->sync = logx->syncpt;
	log->nextsync = LOGSYNC_DELTA(logx->size);
	event_init(&log->syncwait);

jEVENT(0,("lmLogInit: lsn:0x%08x syncpt:0x%08x sync:0x%08x\n",
	logx->lsn, logx->syncpt, logx->sync));

	LOGSYNC_LOCK_ALLOC(logx, lockid);
	LOGSYNC_LOCK_INIT(logx);

	CDLL_INIT(&logx->synclist);

	logx->count = 0;
	logx->yah = NULL;

	/*
	 * initialize for lazy/group commit
	 */
	logx->clsn = lsn;

	/*
	 * update/write superblock
	 */
	logsuper->state = LOGMOUNT;
	++logsuper->serial;
	log->serial = logsuper->serial;
	lbmDirectWrite(log, bpsuper, lbmWRITE|lbmRELEASE|lbmSYNC);
	if (rc = lbmIOWait(bpsuper, lbmFREE))
		goto errout30;

jEVENT(0,("lmLogInit: exit(%d)\n", rc));
	return 0;

	/*
	 *	unwind on error
	 */
errout30: /* release log page */
	lbmFree(bp);

errout20: /* release log superblock */
	lbmFree(bpsuper);

errout10: /* unwind lbmLogInit() */
	lbmLogShutdown(log);

jEVENT(0,("lmLogInit: exit(%d)\n", rc));
	return rc;
}


/*
 * NAME:	lmLogClose()
 *
 * FUNCTION:	remove file system <ipmnt> from active list of log <iplog>
 *		and close it on last close.
 *
 * PARAMETER:	iplog	- log inode
 *		ipmnt  - file system mount inode
 *
 * RETURN:	errors from subroutines
 *
 * serialization:
 */
lmLogClose(
	inode_t *ipmnt,
	inode_t *iplog)
{
	int32	rc, rc1;
	dev_t	fsdev = ipmnt->i_dev;
	log_t	*log = (log_t *)iplog;
	logx_t	*logx;

jEVENT(0,("lmLogClose: log:0x%08x\n", iplog));

	/*
	 *	in-line log in host file system
	 */
inlineLog:
	if (log->flag & JFS_INLINELOG)
	{
		rc = lmLogShutdown(log);

		IWRITE_LOCK(iplog);
		goto out1;
	}

	/*
	 *	external log as separate logical volume
	 */
externalLog:

	/* serialize open/close between multiple file systems
	 * associated with the log
	 */
	IWRITE_LOCK(iplog);

	/*
	 * remove file system from log active file system list
	 */
	rc = lmLogFileSystem(log, fsdev, 0);

	if (iplog->i_count > 1)
		goto out2;

	/*
	 *	last close: shut down log
	 */
	rc = ((rc1 = lmLogShutdown(log)) && rc == 0) ? rc1 : rc;

	/*
	 * remove log device from log device table
	 */
out1:
	LOGTBL_LOCK();
	LIST_REMOVE(log, loglist);
	lCache.nLog--;
	LOGTBL_UNLOCK();

	/* free log inode pinned extension */
	logx = log->logx;
	xmfree(logx, pinned_heap);
	log->logx = NULL;

out2:
	IWRITE_UNLOCK(iplog);

	/*
	 * release reference of the in-memory log inode.
	 * i_mode of the log inode is zero therefore iput() will
	 * just put ip back on free list on last reference release.
	 */
	ICACHE_LOCK();
	iput(iplog, NULL);
	ICACHE_UNLOCK();

jFYI(0,("lmLogClose: exit(%d)\n", rc));
	return rc;
}


/*
 * NAME:	lmLogShutdown()
 *
 * FUNCTION:	log shutdown at last LogClose().
 *
 *		write log syncpt record.
 *		update super block to set redone flag to 0.
 *
 * PARAMETER:	log	- log inode
 *
 * RETURN:	0	- success
 *			
 * serialization: single last close thread
 */
static int32
lmLogShutdown(
	log_t *log)
{
	int32	rc;
	lrd_t	lrd;
	int32	lsn;
	logsuper_t	*logsuper;
	lbuf_t	*bpsuper;
	logx_t	*logx = log->logx;
	lbuf_t	*bp;
	logpage_t	*lp;

jEVENT(0,("lmLogShutdown: log:0x%08x\n", log));

	assert(log->cqueue.head == NULL);
	assert(logx->synclist.head == (void *)&logx->synclist);

	/*
	 * write the last SYNCPT record with syncpoint = 0
	 * (i.e., log redo up to HERE !)
	 */
	lrd.logtid = 0;
	lrd.backchain = 0;
	lrd.type = LOG_SYNCPT;
	lrd.length = 0;
	lrd.log.syncpt.sync = 0;
	lsn = lmWriteRecord(log, NULL, &lrd, NULL);
	bp = log->bp;
	lp = (logpage_t *)bp->l_ldata;
	lp->h.eor = lp->t.eor = bp->l_eor;
	lbmWrite(log, log->bp, lbmWRITE|lbmRELEASE|lbmSYNC);
	lbmIOWait(log->bp, lbmFREE);

	/*
	 * synchronous update log superblock
         * mark log state as shutdown cleanly
	 * (i.e., Log does not need to be replayed).
	 */
	if (rc = lbmRead(log, 1, &bpsuper))
		goto out;

	logsuper = (logsuper_t *)bpsuper->l_ldata;
	logsuper->state = LOGREDONE;
	logsuper->end = lsn;
	lbmDirectWrite(log, bpsuper, lbmWRITE|lbmRELEASE|lbmSYNC);
	rc = lbmIOWait(bpsuper, lbmFREE);

jEVENT(0,("lmLogShutdown: lsn:0x%08x page:%d eor:%d\n",
	lsn, log->page, log->eor));

	/*
	 * free resources of log
	 */
out:
	/* free group commit resources */
	LOGGC_LOCK_FREE(log);

	/* free log sync resources */
	LOGSYNC_LOCK_FREE(logx);

	/* free log write resources */
	LOG_LOCK_FREE(log);

	/*
	 * shutdown per log i/o
	 */
	lbmLogShutdown(log);

	if (rc)
	{
jEVENT(0,("lmLogShutdown: exit(%d)\n", rc));
	}
	return rc;
}


/*
 * NAME:	lmLogFileSystem()
 *
 * FUNCTION:	insert (<activate> = true)/remove (<activate> = false)
 *	file system into/from log active file system list.
 *
 * PARAMETE:	log	- pointer to logs inode.
 *		fsdev	- dev_t of filesystem.
 *		serial  - pointer to returned log serial number
 *		activate - insert/remove device from active list.
 *
 * RETURN:	0	- success
 *		errors returned by vms_iowait().
 *			
 * serialization: IWRITE_LOCK(log inode) held on entry/exit
 */
static int
lmLogFileSystem(
	log_t 	*log,
	dev_t	fsdev,
	int32	activate)
{
	int32		rc = 0;
	int32		bit, word;
	logsuper_t 	*logsuper;
	lbuf_t		*bpsuper;

	/*
	 * insert/remove file system device to log active file system list.
	 */
	if (rc = lbmRead(log, 1, &bpsuper))
		return rc;

	logsuper = (logsuper_t *)bpsuper->l_ldata;
	bit = minor(fsdev);
	word = bit/32;
	bit -= 32*word;
	if (activate)
		logsuper->active[word] |= (LEFTMOSTONE >> bit);
	else
		logsuper->active[word] &= (~(LEFTMOSTONE >> bit));

	/*
	 * synchronous write log superblock:
	 *
	 * write sidestream bypassing write queue:
	 * at file system mount, log super block is updated for
	 * activation of the file system before any log record
	 * (MOUNT record) of the file system, and at file system
	 * unmount, all meta data for the file system has been
	 * flushed before log super block is updated for deactivation
	 * of the file system.
	 */
	lbmDirectWrite(log, bpsuper, lbmWRITE|lbmRELEASE|lbmSYNC);
	rc = lbmIOWait(bpsuper, lbmFREE);

	return rc;
}


/*
 *	lmLogQuiesce()
 */
int32 lmLogQuiesce(log_t	*log)
{
	int32	rc;

	rc = lmLogShutdown(log);

	return rc;
}


/*
 *	lmLogResume()
 */
int32 lmLogResume(log_t	*log,
	inode_t	*ipmnt)
{
	int32	rc;

 	log->base = LBLK2PBLK(ipmnt, addressPXD(&ipmnt->i_logpxd));
	log->size = (lengthPXD(&ipmnt->i_logpxd) << ipmnt->i_l2bsize) >> L2LOGPSIZE;
	rc = lmLogInit(log);

	return rc;
}


/*
 *	lmInit()
 *
 * initialization at JFS bringup
 */
lmInit()
{
	/*
	 * allocate/initialize per system log resources
	 */
	/* allocate/initialize active log device list */
	lCache.nLog = 0;
	LIST_INIT(&lCache.logList);

	LOGTBL_LOCK_ALLOC();
	LOGTBL_LOCK_INIT();

	/*
	 * initialize log buffer manager (lbm)
	 */
	/* allocate/initialize log buffer cache lock */
	LCACHE_LOCK_ALLOC();
	LCACHE_LOCK_INIT();

	/* initialize logx freelist */
	lCache.logx = NULL;

	/* initialize  lbuf freelist */
	lCache.freecnt = 0;
	lCache.freelist = NULL;
	/* event_init(&lCache.freewait); */

	/* initialize i/o buffer freelist */
	lCache.iobp = NULL;

	return 0;
}


/*
 *	lmShutdown()
 *
 * cleanup at JFS shutdown
 */
lmShutdown()
{
	return 0;
}


/*
 *		log buffer manager (lbm)
 *		------------------------
 *
 * special purpose buffer manager supporting log i/o requirements.
 *
 * per log write queue:
 * log pageout occurs in serial order by fifo write queue and
 * restricting to a single i/o in pregress at any one time.
 * a circular singly-linked list
 * (logx->wrqueue points to the tail, and buffers are linked via
 * bp->wrqueue field), and
 * maintains log page in pageout ot waiting for pageout in serial pageout.
 *
 * per system freelist:
 * buffers are appropriated from medata buffer cache
 * to maintain non-blocking write of log pages.
 * lCache (log page freelist) is a NULL-terminated singly-linked list
 * (freelist points to the head, and buffers are linked via buffer
 * header freelist field) and maintains free log page buffers
 */

/*
 *	lbmLogInit()
 *
 * initialize per log I/O setup at lmLogInit()
 */
static int32 lbmLogInit(
	log_t	*log)	/* log inode */
{
	logx_t	*logx;
	int32	i;

jEVENT(0,("lbmLogInit: log:0x%08x\n", log));

	/*
	 * allocate log device (logx) structure
	 */
	if (lCache.logx == NULL)
	{
		/* allocate pinned storage for more log device structs */
		i = PSIZE / sizeof(logx_t);
		if ((lCache.logx = (logx_t *)xmalloc(PSIZE, 0, pinned_heap)) == NULL)
			return(ENOMEM);

		memset(lCache.logx, 0, PSIZE);

		/* insert the structs on the free list */
		for (logx = lCache.logx; i - 1; logx++, i--)
			logx->freelist = logx + 1;
		logx->freelist = NULL;
	}

	logx = lCache.logx;
	lCache.logx = logx->freelist;

	/* bind log device to mounted log */
	log->logx = logx;

	/* initialize current buffer cursor */
	log->bp = NULL;

	/* initialize log device write queue */
	logx->wqueue = NULL;

	return(0);
}


/*
 *	lbmLogShutdown()
 *
 * finalize per log I/O setup at lmLogShutdown()
 */
static void lbmLogShutdown(
	log_t	*log)	/* log inode */
{
	logx_t	*logx = log->logx;

jEVENT(0,("lbmLogShutdown: log:0x%08x\n", log));

	/*
	 * free log device structure
	 */
	logx->freelist = lCache.logx;
	lCache.logx = logx;
	log->logx = NULL;

	log->bp = NULL;
}


/*
 *	lbmAllocate()
 *
 * allocate an empty log buffer
 */
static lbuf_t *lbmAllocate(
	log_t	*log)	/* log inode */
{
	int32	rc;
	int32	ipl;
	logx_t	*logx = log->logx;
	lbuf_t	*bp;
	iobuf_t	*iobp;
#ifdef	_JFS_OS2
	Req_List_Header	*rlhp;
	Req_Header	*rhp;
	PB_Read_Write	*rwp;
	SG_Descriptor	*sgp;	/* S/G descriptor */
#endif	/* _JFS_OS2 */

	ipl = LCACHE_LOCK();	/* disable+lock */

	/*
	 * recycle from log buffer freelist if any
	 */
retry:
	if (bp = lCache.freelist)
	{
		/*
		 * free buffer avaialble:
		 */
		lCache.freelist = bp->l_freelist;
		lCache.freecnt--;

		LCACHE_UNLOCK(ipl);	/* unlock+enable */
		goto out;
	}

	/*
	 * no free buffer available:
	 */
getNewBuffer:
	LCACHE_UNLOCK(ipl);	/* unlock+enable */

	/*
	 * allocate a buffer from JFS buffer cache
	 */
	ipl = IOCACHE_LOCK();	/* disable+lock */

	while ((bp = (lbuf_t *)cmRecycle(CM_WAIT, &ipl)) == NULL)
	{
		IOCACHE_UNLOCK(ipl);	/* unlock+enable */
		ipl = LCACHE_LOCK();	/* disable+lock */
		goto retry;
	}

	IOCACHE_UNLOCK(ipl);	/* unlock+enable */

	/*
	 * initialize log buffer
	 */
out:
	bp->l_log = log;

	bp->l_flag = 0;
	bp->l_logx = logx;

	bp->l_wqnext = NULL;
	bp->l_freelist = NULL;

	bp->l_pn = 0;
	bp->l_eor = 0;
	bp->l_ceor = 0;

	bp->l_blkno = 0;

	event_init(&bp->l_ioevent);

	/*
	 * initialize device i/o buffer fixed attributes
	 */
	iobp = bp->l_iobp;

#ifdef	_JFS_OS2
	/*
	 * fill in request list header
	 */
	rlhp = &iobp->bio_rlh;
	/* rlhp->Count = 1; */
	/* rlhp->Notify_Address = (void *)jfsIODone; */
	/* rlhp->Request_Control = RLH_Single_Req |
				RLH_Notify_Err | RLH_Notify_Done; */
	rlhp->Block_Dev_Unit = log->dev;
	/* rlhp->Lst_Status */

	/*
	 * fill in request header
	 */
	/* rhp = &iobp->bio_rh.RqHdr; */
	/* rhp->Length = RH_LAST_REQ; */
	/* rhp->Old_Command = PB_REQ_LIST; */
	/* rhp->Command_Code */
	/* rhp->Head_Offset = sizeof(Req_List_Header); */
	/* rhp->Req_Control = 0; */
	/* rhp->Priority = PRIO_FOREGROUND_USER; */
	/* rhp->Status */
	/* rhp->Error_Code */
	/* rhp->Notify_Address */
	/* rhp->Hint_Pointer = -1; */

	/*
	 * fill in  r/w request
	 */
	rwp = &iobp->bio_rh;
        /* rwp->Start_Block */
        rwp->Block_Count = LOGPSIZE >> log->l2pbsize;
        /* rwp->Blocks_Xferred */
        /* rwp->RW_Flags */

	/* update S/G descriptor */
	rwp->SG_Desc_Count = 1;
	sgp = iobp->bio_sg;
	*sgp = bp->l_pgdesc;
#endif	/* _JFS_OS2 */

	return bp;
}


/*
 *	lbmFree()
 *
 * release a log buffer to freelist
 */
static void lbmFree(
	lbuf_t	*bp)
{
	int32	ipl;

	ipl = LCACHE_LOCK();

	lbmfree(bp, &ipl);

	LCACHE_UNLOCK(ipl);
}

static void lbmfree(
	lbuf_t	*bp,
	int32	*xipl)
{
	int32	ipl;

	assert(bp->l_wqnext == NULL);

	/* keep free buffers ready */
	if (lCache.freecnt < 3)
	{
		/*
		 * return the buffer to head of freelist
		 */
		bp->l_freelist = lCache.freelist;
		lCache.freelist = bp;
		lCache.freecnt++;

		return;
	}

	LCACHE_UNLOCK(*xipl);	/* unlock+enable */

	/*
	 * return extra jfs buffer
	 */
	ipl = IOCACHE_LOCK();	/* disable+lock */

	cmFreeCbuf((cbuf_t *)bp);

	IOCACHE_UNLOCK(ipl);	/* unlock+enable */

	*xipl = LCACHE_LOCK();	/* disable+lock */
}


/*
 *	lbmRelease()
 *
 * remove the log buffer from log device write queue;
 */
static void lbmRelease(
	log_t	*log,
	uint32	flag)
{
	logx_t	*logx = log->logx;
	lbuf_t	*bp, *tail;
	int32	ipl;

	bp = log->bp;

	ipl = LCACHE_LOCK();

	tail = logx->wqueue;

	/* single element queue */
	if (bp == tail)
	{
		logx->wqueue = NULL;
		bp->l_wqnext = NULL;
	}
	/* multi element queue */
	else
	{
		tail->l_wqnext = bp->l_wqnext;
		bp->l_wqnext = NULL;
	}

	if (flag & lbmFREE)
		lbmfree(bp, &ipl);

	LCACHE_UNLOCK(ipl);
}


/*
 *	lbmRead()
 */
static int32 lbmRead(
	register log_t		*log,		/* log inode */
	register int32		pn,		/* log page number */
	register lbuf_t		**bpp)
{
	register int32	rc;	
	register lbuf_t *bp;

 	/*
	 * allocate a log buffer
	 */
	*bpp = bp = lbmAllocate(log);
jEVENT(0,("lbmRead: bp:0x%08x pn:0x%08x\n", bp, pn));

	/*
	 * initialize buffer for device driver
	 */
	bp->l_flag = lbmREAD;
	
	/* map the logical block address to physical block address */
	bp->l_pn = pn;
	bp->l_blkno = log->base + (pn << (L2LOGPSIZE - log->l2pbsize));

	/*
	 * call the device driver to initiate the read.
	 */
	lbmStartIO(bp, lbmIODone);

	/*
	 * Wait for the read to complete.
	 */
	if (rc = lbmIOWait(bp, 0))
		lbmFree(bp);

	return rc;
}


/*
 *	lbmWrite()
 *
 * buffer at head of pageout queue stays after completion of
 * partial-page pageout and redriven by explicit initiation of
 * pageout by caller until full-page pageout is completed and
 * released.
 *
 * device driver i/o done redrives pageout of new buffer at
 * head of pageout queue when current buffer at head of pageout
 * queue is released at the completion of its full-page pageout.
 *
 * LOGGC_LOCK() serializes lbmWrite() by lmNextPage() and lmGroupCommit().
 * LCACHE_LOCK() serializes xflag between lbmWrite() and lbmIODone()
 */
static void lbmWrite(
	log_t	*log,	/* log inode */
	lbuf_t	*bp,
	uint32	flag)
{
	int32	rc;
	logx_t	*logx = log->logx;
	lbuf_t	*tail;
	int32	ipl;

jEVENT(0,("lbmWrite: bp:0x%08x flag:0x%08x pn:0x%08x\n",
	bp, flag, bp->l_pn));
	
	/* map the logical block address to physical block address */
	bp->l_blkno = log->base + (bp->l_pn << (L2LOGPSIZE - log->l2pbsize));

	ipl = LCACHE_LOCK();		/* disable+lock */

	/*
	 * initialize buffer for device driver
	 */
	bp->l_flag = flag;

	/*
	 *	insert bp at tail of write queue associated with log
	 *
	 * (request is either for bp already/currently at head of queue
	 * or new bp to be inserted at tail)
	 */
	tail = logx->wqueue;

	/* is buffer not already on write queue ? */
	if (bp->l_wqnext == NULL)
	{
		/* insert at tail of wqueue */
		if (tail == NULL)
		{
			logx->wqueue = bp;
			bp->l_wqnext = bp;
		}
		else
		{
			logx->wqueue = bp;
			bp->l_wqnext = tail->l_wqnext;
			tail->l_wqnext = bp;
		}

		tail = bp;
	}

	/* is buffer at head of wqueue and for write ? */
	if ((bp != tail->l_wqnext) || !(flag & lbmWRITE))
	{
		LCACHE_UNLOCK(ipl);	/* unlock+enable */
		return;
	}
	
	LCACHE_UNLOCK(ipl);	/* unlock+enable */

// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
	if (!(flag & lbmSYNC))
	{
		/*
		 * Can't hold spinlock during I/O
		 */
		LOGGC_UNLOCK(log);
	}
#endif /* _JFS_LAZYCOMMIT */
// END D233382

	/*
	 *	initiate pageout of the page at head of write queue
	 */
	lbmStartIO(bp, lbmIODone);

// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
	if (!(flag & lbmSYNC))
	{
		LOGGC_LOCK(log);
	}
#endif /* _JFS_LAZYCOMMIT */
// END D233382
}


/*
 *	lbmDirectWrite()
 *
 * initiate pageout bypassing write queue for sidestream
 * (e.g., log superblock) write;
 */
static void lbmDirectWrite(
	log_t	*log,	/* log inode */
	lbuf_t	*bp,
	uint32	flag)
{
jEVENT(0,("lbmDirectWrite: bp:0x%08x flag:0x%08x pn:0x%08x\n",
	bp, flag, bp->l_pn));

	/*
	 * initialize buffer for device driver
	 */
	bp->l_flag = flag;

	/* map the logical block address to physical block address */
	bp->l_blkno = log->base + (bp->l_pn << (L2LOGPSIZE - log->l2pbsize));

	/*
	 *	initiate pageout of the page
	 */
	lbmStartIO(bp, lbmDirectIODone);
}


/*
 * NAME:	lbmStartIO()
 *
 * FUNCTION:	Interface to DD strategy routine
 *
 * RETURN:      none
 *
 * serialization: LCACHE_LOCK() is NOT held during log i/o;
 */
void lbmStartIO(lbuf_t *bp,
	void (*iodone)())
{
        int             rc;
        iobuf_t		*iobp;	/* jfs io request structure */
        Req_List_Header *rlhp;  /* pointer to list header */
        Req_Header      *rhp;   /* pointer to header */
        PB_Read_Write   *rwp;   /* pointer to request */
	SG_Descriptor	*sgp;	/* S/G descriptor */

   MMPHPrelbmStartIO();         /* (d201828) Performance trace hook */

	/*
	 * bind log buffer with device i/o buffer
	 */
	iobp = bp->l_iobp;

	iobp->b_flags = (bp->l_flag & lbmREAD) ? B_READ : B_WRITE;
	iobp->b_iodone = iodone;

	/*
	 * fill in request list header
	 */
	rlhp = &iobp->bio_rlh;
	/* rlhp->Count = 1; */
	/* rlhp->Notify_Address = (void *)jfsIODone; */
	/* rlhp->Request_Control = RLH_Single_Req |
				   RLH_Notify_Err | RLH_Notify_Done; */
        /* rlhp->Block_Dev_Unit */
	rlhp->Lst_Status = 0;

	/*
	 * fill in request header
	 */
	rhp = &iobp->bio_rh.RqHdr;
	/* rhp->Length = RH_LAST_REQ; */
	/* rhp->Old_Command = PB_REQ_LIST; */
        rhp->Command_Code = (bp->l_flag & lbmREAD) ? PB_READ_X : PB_WRITE_X;
	/* rhp->Head_Offset = sizeof(Req_List_Header); */
	/* rhp->Req_Control = 0; */
	/*
	 * Assign high priority, since multiple transactions may be blocked
	 * in Group Commit
	 */
	rhp->Priority = PRIO_PAGER_HIGH;			// D230860
	rhp->Status = 0;
	rhp->Error_Code = 0;
	/* rhp->Notify_Address */
	/* rhp->Hint_Pointer = -1; */

        /*
	 * fill in read/write request
	 */
        rwp = &iobp->bio_rh;
        rwp->Start_Block = bp->l_blkno;
        /* rwp->Block_Count */
        rwp->Blocks_Xferred = 0;
        rwp->RW_Flags = 0;

	/* update S/G descriptor */
	/* rwp->SG_Desc_Count = 1; */
	/* sgp = iobp->bio_sg; */
	/* *sgp = bp->l_pgdesc; */

        /* Call the strategy2 interface */
	if (bp->l_log->strat3p)
        	CallStrat3(bp->l_log->strat3p, rlhp);
	else
        	CallStrat2(bp->l_log->strat2p, rlhp);

   MMPHPostlbmStartIO();         /* (d201828) Performance trace hook */
}


/*
 *	lbmIOWait()
 */
static int32 lbmIOWait(
	register lbuf_t *bp,
	uint32	flag)
{
	int32	rc;
	int32	ipl;

jEVENT(0,("lbmIOWait1: bp:0x%08x flag:0x%08x:0x%08x\n", bp, bp->l_flag, flag));

	ipl = LCACHE_LOCK();		/* disable+lock */

	if (!(bp->l_flag & lbmDONE))
	{
		LCACHE_SLEEP(&bp->l_ioevent);
	}

	rc = (bp->l_flag & lbmERROR) ? EIO : 0;

	if (flag & lbmFREE)
		lbmfree(bp, &ipl);

	LCACHE_UNLOCK(ipl);		/* unlock+enable */

jEVENT(0,("lbmIOWait2: bp:0x%08x flag:0x%08x:0x%08x\n", bp, bp->l_flag, flag));
	return rc;
}


/*
 *	lbmIODone()
 *
 * executed at INTIODONE level
 */
static void lbmIODone(
	register iobuf_t	*iobp)
{
	register lbuf_t	*bp, *nextbp, *tail;
	register logx_t	*logx;
	int32	ipl;

	/*
	 * get back jfs buffer bound to the i/o buffer
	 */
	bp = (lbuf_t *)iobp->b_jfsbp;
jEVENT(0,("lbmIODone: bp:0x%08x flag:0x%08x\n", bp, bp->l_flag));

	ipl = LCACHE_LOCK();		/* disable+lock */

	bp->l_flag |= lbmDONE;

	if (iobp->b_flags & B_ERROR) {
		bp->l_flag |= lbmERROR;

#ifdef _JFS_OS2
      SysLogError();
#endif

   }

	/*
	 *	pagein completion
	 */
	if (bp->l_flag & lbmREAD)
	{
		bp->l_flag &= ~lbmREAD;

		LCACHE_UNLOCK(ipl);	/* unlock+enable */

		/* wakeup I/O initiator */
		if (bp->l_ioevent != EVENT_NULL)
		{
			LCACHE_WAKEUP(&bp->l_ioevent);
		}

		return;
	}

	/*
	 *	pageout completion
	 *
	 * the bp at the head of write queue has completed pageout.
	 *
	 * if single-commit/full-page pageout, remove the current buffer
	 * from head of pageout queue, and redrive pageout with
	 * the new buffer at head of pageout queue;
	 * otherwise, the partial-page pageout buffer stays at
	 * the head of pageout queue to be redriven for pageout
	 * by lmGroupCommit() until full-page pageout is completed.
	 */
	bp->l_flag &= ~lbmWRITE;
	INCREMENT(statLCache.pagedone);

	/* update committed lsn */
	logx = bp->l_logx;
	logx->clsn = (bp->l_pn << L2LOGPSIZE) + bp->l_ceor;

	tail = logx->wqueue;

	/* single element queue */
	if (bp == tail)
	{
		/* remove head buffer of full-page pageout
		 * from log device write queue
		 */
		if (bp->l_flag & lbmRELEASE)
		{
			logx->wqueue = NULL;
			bp->l_wqnext = NULL;
		}
	}
	/* multi element queue */
	else
	{
		/* remove head buffer of full-page pageout
		 * from log device write queue
		 */
		if (bp->l_flag & lbmRELEASE)
		{
			nextbp = tail->l_wqnext = bp->l_wqnext;
			bp->l_wqnext = NULL;

			/*
			 * redrive pageout of next page at head of write queue:
			 * redrive next page without any bound tblk
			 * (i.e., page w/o any COMMIT records), or
			 * first page of new group commit which has been
			 * queued after current page (subsequent pageout
			 * is performed synchronously, except page without
			 * any COMMITs) by lmGroupCommit()) as indicated
			 * by lbmWRITE flag;
			 */
			if (nextbp->l_flag & lbmWRITE)
			{
				LCACHE_UNLOCK(ipl);	/* unlock+enable */
				
jEVENT(0,("lbmRedrive: bp:0x%08x flag:0x%08x\n", bp, bp->l_flag));
        			lbmStartIO(nextbp, lbmIODone);

				ipl = LCACHE_LOCK();	/* disable+lock */
			}
		}
	}

	/*
	 *	synchronous pageout:
	 *
	 * buffer has not necessarily been removed from write queue
	 * (e.g., synchronous write of partial-page with COMMIT):
	 * leave buffer for i/o initiator to dispose
	 */
	if (bp->l_flag & lbmSYNC)
	{
		LCACHE_UNLOCK(ipl);	/* unlock+enable */

		/* wakeup I/O initiator */
		if (bp->l_ioevent != EVENT_NULL)
		{
			LCACHE_WAKEUP(&bp->l_ioevent);
		}
	}
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	/*
	 *	Group Commit pageout:
	 */
	else if (bp->l_flag & lbmGC)
	{
		LCACHE_UNLOCK(ipl);
		lmPostGC(bp);
	}
#endif /* _JFS_LAZYCOMMIT */
// END D230860
	/*
	 *	asynchronous pageout:
	 *
	 * buffer must have been removed from write queue:
	 * insert buffer at head of freelist where it can be recycled
	 */
	else
	{
		assert(bp->l_flag & lbmRELEASE);
		assert(bp->l_flag & lbmFREE);
		lbmfree(bp, &ipl);

		LCACHE_UNLOCK(ipl);	/* unlock+enable */
	}
}


/*
 *	lbmDirectIODone()
 *
 * iodone() for lbmDirectWrite() to bypass write queue;
 * executed at INTIODONE level;
 */
static void lbmDirectIODone(
	register iobuf_t	*iobp)
{
	register lbuf_t	*bp;
	int32	ipl;

	/*
	 * get back jfs buffer bound to the io buffer
	 */
	bp = (lbuf_t *)iobp->b_jfsbp;
jEVENT(0,("lbmDirectIODone: bp:0x%08x flag:0x%08x\n", bp, bp->l_flag));

	ipl = LCACHE_LOCK();		/* disable+lock */

	bp->l_flag |= lbmDONE;

	if (iobp->b_flags & B_ERROR) {
		bp->l_flag |= lbmERROR;
#ifdef _JFS_OS2
      SysLogError();
#endif
   }

	/*
	 *	pageout completion
	 */
	bp->l_flag &= ~lbmWRITE;

	/*
	 *	synchronous pageout:
	 */
	if (bp->l_flag & lbmSYNC)
	{
		LCACHE_UNLOCK(ipl);	/* unlock+enable */

		/* wakeup I/O initiator */
		if (bp->l_ioevent != EVENT_NULL)
		{
			LCACHE_WAKEUP(&bp->l_ioevent);
		}
	}
	/*
	 *	asynchronous pageout:
	 */
	else
	{
		assert(bp->l_flag & lbmRELEASE);
		assert(bp->l_flag & lbmFREE);
		lbmfree(bp, &ipl);

		LCACHE_UNLOCK(ipl);	/* unlock+enable */
	}
}


/*
 * NAME:	lmLogFormat()/jfs_logform()
 *
 * FUNCTION:	format file system log (ref. jfs_logform()).
 *
 * PARAMETERS:
 *	log	- log inode (with common mount inode base);
 *	logAddress - start address of log space in FS block;
 *	logSize	- length of log space in FS block;
 *
 * RETURN:	0 -	success
 *		-1 -	i/o error
 */
int32 lmLogFormat(
	inode_t	*ipmnt,
        int64   logAddress,	/* log address in FS block */
        int32   logSize)	/* log size in FS blocks */
{
	int32	rc = 0;
	cbuf_t	*bp;
	logsuper_t	*logsuper;
	logpage_t	*lp;
	int32	lspn;	/* log sequence page number */
	struct lrd	*lrd_ptr;
	int32	npbperpage, npages;

jFYI(0,("lmLogFormat: logAddress:%lld logSize:%d\n",
	logAddress, logSize));

	/* allocate a JFS buffer */
	bp = rawAllocate();

	/* map the logical block address to physical block address */
	bp->cm_blkno = logAddress << ipmnt->i_l2bfactor;

	npbperpage = LOGPSIZE >> ipmnt->i_l2pbsize;
	npages = logSize / (LOGPSIZE >> ipmnt->i_l2bsize);

	/*
	 *	log space:
	 *
	 * page 0 - reserved;
	 * page 1 - log superblock;
	 * page 2 - log data page: A SYNC log record is written
	 *	    into this page at logform time;
	 * pages 3-N - log data page: set to empty log data pages;
	 */
	/*
	 *	init log superblock: log page 1
	 */
	logsuper = (logsuper_t *)bp->cm_cdata;

	logsuper->magic = LOGMAGIC;
	logsuper->version = LOGVERSION;
	logsuper->state = LOGREDONE;
	logsuper->flag = ipmnt->i_mntflag; /* ? */
	logsuper->size = npages;
	logsuper->bsize = ipmnt->i_bsize;
	logsuper->l2bsize = ipmnt->i_l2bsize;
	logsuper->end = 2*LOGPSIZE + LOGPHDRSIZE + LOGRDSIZE;

	bp->cm_blkno += npbperpage;
	rawWrite(ipmnt, bp, 0);

	/*
	 *	init pages 2 to npages-1 as log data pages:
	 *
	 * log page sequence number (lpsn) initialization:
	 *
	 * pn:   0     1     2     3                 n-1
	 *       +-----+-----+=====+=====+===.....===+=====+
	 * lspn:             N-1   0     1           N-2
	 *                   <--- N page circular file ---->
	 *
	 * the N (= npages-2) data pages of the log is maintained as
	 * a circular file for the log records;
	 * lpsn grows by 1 monotonically as each log page is written
	 * to the circular file of the log;
	 * Since the AIX DUMMY log record is dropped for this XJFS,
	 * and setLogpage() will not reset the page number even if
	 * the eor is equal to LOGPHDRSIZE. In order for binary search
	 * still work in find log end process, we have to simulate the
	 * log wrap situation at the log format time.
	 * The 1st log page written will have the highest lpsn. Then
	 * the succeeding log pages will have ascending order of
	 * the lspn starting from 0, ... (N-2)
	 */
	lp = (logpage_t *)bp->cm_cdata;

	/*
	 * initialize 1st log page to be written: lpsn = N - 1,
	 * write a SYNCPT log record is written to this page
	 */
	lp->h.page = lp->t.page = npages - 3;
	lp->h.eor = lp->t.eor = LOGPHDRSIZE + LOGRDSIZE;

	lrd_ptr = (struct lrd *)&lp->data;
	lrd_ptr->logtid = 0;
	lrd_ptr->backchain = 0;
	lrd_ptr->type = LOG_SYNCPT;
	lrd_ptr->length = 0;
	lrd_ptr->log.syncpt.sync = 0;

	bp->cm_blkno += npbperpage;
	rawWrite(ipmnt, bp, 0);

	/*
	 *	initialize succeeding log pages: lpsn = 0, 1, ..., (N-2)
	 */
	for (lspn = 0; lspn < npages - 3; lspn++)
	{
		lp->h.page = lp->t.page = lspn;
		lp->h.eor = lp->t.eor = LOGPHDRSIZE;

		bp->cm_blkno += npbperpage;
		rawWrite(ipmnt, bp, 0);
	}

	/*
	 *	finalize log
	 */
	/* release the buffer */
	rawRelease(bp);

	return rc;
}


#ifdef	_JFS_STATISTICS
/*
 *	lmStatistics()
 */
lmStatistics(
	caddr_t	arg,
	int32	flag)
{
	int32	rc;

	/* copy out the argument */
	if (rc = copyout((caddr_t)&statLCache, (caddr_t)arg,
			 sizeof(struct statLCache)))
		return rc;

	/* reset the counters */
	if (flag)
	{
		statLCache.commit = 0;
		statLCache.pageinit = 0;
		statLCache.pagedone = 0;
		statLCache.sync = 0;
		statLCache.maxbufcnt = 0;
	}

	return 0;
}
#endif	/* _JFS_STATISTICS */
