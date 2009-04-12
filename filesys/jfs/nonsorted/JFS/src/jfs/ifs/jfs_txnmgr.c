/* $Id: jfs_txnmgr.c,v 1.2 2003/09/21 08:43:01 pasha Exp $ */

static char *SCCSID = "@(#)1.48  11/1/99 15:07:06 src/jfs/ifs/jfs_txnmgr.c, sysjfs, w45.fs32, fixbld";
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
 */

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

/*
 *      jfs_txnmgr.c: transaction manager
 *
 * notes:
 * transaction starts with txBegin() and ends with txCommit()
 * or txAbort().
 *
 * tlock is acquired at the time of update;
 * (obviate scan at commit time for xtree and dtree)
 * tlock and bp points to each other;
 * (no hashlist for bp -> tlock).
 *
 * special cases:
 * tlock on in-memory inode:
 * in-place tlock in the in-memory inode itself;
 * converted to page lock by iWrite() at commit time.
 *
 * tlock during write()/mmap() under anonymous transaction (tid = 0):
 * transferred (?) to transaction at commit time.
 *
 * use the page itself to update allocation maps
 * (obviate intermediate replication of allocation/deallocation data)
 * hold on to bp+lock thru update of maps
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */
#include "mmph.h"

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_logmgr.h"
#include "jfs_txnmgr.h"
#include "jfs_superblock.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_proto.h"
#include "jfs_debug.h"

/*
 *      transaction management structures
 */
struct {
        /* tblock */
        int32   freetid;        /* 4: index of a free tid structure */
        event_t freewait;       /* 4: eventlist of free tblock */

        /* tlock */
        int32   freelock;       /* 4: index first free lock word */
#ifdef _JFS_OS2
	event_t freelockwait;	/* 4: eventlist of free tlock */
	event_t lowlockwait;	/* 4: eventlist of ample tlocks  D230860 */
#endif
        int32   tlocksInUse;    /* 4: Number of tlocks in use */
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	XSPINLOCK_T LazyLock;	/* 4: synchronize sync_queue & unlock_queue */
	event_t	Lazy_event;	/* 4: This is where lazycommit thread sleeps */
/*	tblock_t *sync_queue;	 * 4: Transactions waiting for data sync */
	tblock_t *unlock_queue; /* 4: Transactions waiting to be released */
	tblock_t *unlock_tail;	/* 4: Tail of unlock_queue */
	int32	lazyQsize;	/* 4: # of tblocks in unlock queue  D233382*/
	int32	lazyQwait;	/* 4: Make new transactions wait until unlock */
				/*    queue shrinks		D233382 */
#endif /* _JFS_LAZYCOMMIT */
// END D230860
} TxAnchor;

int32   nTxBlock = 512;         /* number of transaction blocks */
struct tblock   *TxBlock;       /* transaction block table */

extern int32    nBuffer;        /* number of buffers */
int32   nTxLock = 2048;         /* number of transaction locks */
int32   TxLockHWM = 2048 * .8;  /* High water mark for number of txLocks used */
struct tlock    *TxLock;        /* transaction lock table */
extern event_t  SyncEvent;      /* Event in which sync daemon sleeps */
int32	TlocksLow = 0;		/* Flag indicating low number of avail. tlocks*/
#ifdef _JFS_LAZYCOMMIT
int32	LazyCommitRunning = 0;	/* TRUE if lazycommit thread running  D230860 */
int32	LazyHWM = 64;		/* Slow down when we have this may outstanding
				 * transactions in unlock queue  D233382 */
int32	LazyLWM = 8;		/* Get going again when we're down to this many
				 * in unlock queue		D233382 */
#endif /* _JFS_LAZYCOMMIT */


/*
 *      transaction management lock
 */
MUTEXLOCK_T     jfsTxnLock;

#define TXN_LOCK_ALLOC()\
        MUTEXLOCK_ALLOC(&jfsTxnLock, LOCK_ALLOC_PIN, JFS_TXN_LOCK_CLASS, -1)
#define TXN_LOCK_INIT()         MUTEXLOCK_INIT(&jfsTxnLock)
#define TXN_LOCK()              MUTEXLOCK_LOCK(&jfsTxnLock)
#define TXN_UNLOCK()            MUTEXLOCK_UNLOCK(&jfsTxnLock)
#define TXN_NOLOCK()            MUTEXLOCK_NOLOCK(&jfsTxnLock)

// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
#define LAZY_LOCK_ALLOC()\
	XSPINLOCK_ALLOC(&TxAnchor.LazyLock, LOCK_ALLOC_PIN, \
			JFS_LAZY_LOCK_CLASS, -1)
#define LAZY_LOCK_INIT()	XSPINLOCK_INIT(&TxAnchor.LazyLock)
#define LAZY_LOCK()		XSPINLOCK_LOCK(&TxAnchor.LazyLock)
#define LAZY_UNLOCK()		XSPINLOCK_UNLOCK(0, &TxAnchor.LazyLock)
#define LAZY_SLEEP()\
	XEVENT_SLEEP(&TxAnchor.Lazy_event, &TxAnchor.LazyLock, T_XSPINLOCK)
#define LAZY_WAKEUP()\
	{if (TxAnchor.Lazy_event != EVENT_NULL)\
		XEVENT_WAKEUP(&TxAnchor.Lazy_event) }
#endif /* _JFS_LAZYCOMMIT */
// END D230860

/*
 * TXN_SLEEP uses EVENT_SLEEP_ONCE because we don't want the macro to
 * recheck the event before returning.  Retry logic exist outside the macro
 * to protect from spurrious wakeups.
 */
#define TXN_SLEEP(event, flag)\
        EVENT_SLEEP_ONCE(event, &jfsTxnLock, T_MUTEXLOCK|flag)
#define TXN_WAKEUP(event)\
        EVENT_WAKEUP(event)

/*
 * Get a transaction lock from the free list.  If the number in use is
 * greater than the high water mark, wake up the sync daemon.  This should
 * free some anonymous transaction locks.  (TXN_LOCK must be held.)
 */

#ifdef _JFS_OS2
#define TXLOCK_ALLOC(LID)\
{\
        while ((LID = TxAnchor.freelock) == 0)\
        {\
		TXN_SLEEP(&TxAnchor.freelockwait, 0);\
        }\
        TxAnchor.freelock = TxLock[LID].next;\
        if (( (++TxAnchor.tlocksInUse) > TxLockHWM) && (TlocksLow == 0))\
	{\
		TlocksLow = 1;\
                KernWakeup((ULONG)&SyncEvent, 0, 0, 0);\
	}\
}
/*PS 28092001
#define TXLOCK_ALLOC(LID)\
{\
        while ((LID = TxAnchor.freelock) == 0)\
        {\
		TXN_SLEEP(&TxAnchor.freelockwait, 0);\
        }\
        TxAnchor.freelock = TxLock[LID].next;\
        HIGHWATERMARK(stattx.maxlid,LID);\
        if ((++TxAnchor.tlocksInUse > TxLockHWM) && (TlocksLow == 0))\
	{\
		TlocksLow = 1;\
                KernWakeup((ULONG)&SyncEvent, 0, 0, 0);\
	}\
}
*/

#define TXLOCK_FREE(LID)\
{\
        TxLock[LID].next = TxAnchor.freelock;\
        TxAnchor.freelock = LID;\
        TxAnchor.tlocksInUse--;\
	if (TlocksLow && (TxAnchor.tlocksInUse < TxLockHWM))\
	{\
		TlocksLow = 0;\
		if (TxAnchor.lowlockwait != EVENT_NULL)\
			TXN_WAKEUP(&TxAnchor.lowlockwait);\
	}\
	if (TxAnchor.freelockwait != EVENT_NULL)\
		TXN_WAKEUP(&TxAnchor.freelockwait);\
}
#endif /* _JFS_OS2 */

/*
 *      statistics
 */
struct {
        int32   maxtid;         /* 4: biggest tid ever used */
        int32   maxlid;         /* 4: biggest lid ever used */
        int32   ntid;           /* 4: # of transactions performed */
        int32   nlid;           /* 4: # of tlocks acquired */
        int32   waitlock;       /* 4: # of tlock wait */
} stattx = {0};


/*
 * external references
 */
void    bmHold(jbuf_t *bp);
void    bmLogSyncRelease(jbuf_t *bp);
int32   iFlushCache(struct inode *ip);
int32   lmGroupCommit(log_t *log, tblock_t *tblk);
void    lmSync(log_t *);
int32 readSuper(inode_t *ipmnt, cbuf_t **bpp);

/*
 * forward references
 */
int32   diLog(log_t *log, tblock_t *tblk, lrd_t *lrd, tlock_t *tlck, commit_t *cd);
void    dtLog(log_t *log, tblock_t *tblk, lrd_t *lrd, tlock_t *tlck);
void    inlineLog(log_t *log, tblock_t *tblk, lrd_t *lrd, tlock_t *tlck);
void    mapLog(log_t *log, tblock_t *tblk, lrd_t *lrd, tlock_t *tlck);
int32   txAbortCommit(commit_t *cd, int32 exval);
void    txAllocPMap(inode_t *ipbmap, maplock_t *maplock, tblock_t *tblk);
int32   txFileSystemDirty(inode_t *ipmnt);
void    txForce(tblock_t *tblk);
int32   txLog(log_t *log, tblock_t *tblk, commit_t *cd);
int32   txMoreLock(void);
void    txUnlock(tblock_t *tblk, uint32 flag);
#ifdef _JFS_LAZYCOMMIT
void    txUpdateMap(tblock_t*tblk);				// D230860
void	txRelease(tblock_t *tblk);				// D230860
#else
void    txUpdateMap(tblock_t*tblk, commit_t *cd);
#endif
void    xtLog(log_t *log, tblock_t *tblk, lrd_t *lrd, tlock_t *tlck);

/*
 *              transaction block/lock management
 *              ---------------------------------
 */

/*
 * NAME:        txInit()
 *
 * FUNCTION:    initialize transaction management structures
 *
 * RETURN:
 *
 * serialization: single thread at jfs_init()
 */
txInit()
{
        int32   rc;
        int32   k, n, size;

        /*
         * allocate/initialize the transaction management lock
         */
        TXN_LOCK_ALLOC();
        TXN_LOCK_INIT();

        /*
         * initialize transaction block (tblock) table
         *
         * transaction id (tid) = tblock index
         * tid = 0 is reserved.
         */
        size = sizeof(tblock_t) * nTxBlock;
        if ((TxBlock = (tblock_t *)xmalloc(size, L2PSIZE, kernel_heap)) == NULL)
                return ENOMEM;

        for (k = 1; k < nTxBlock - 1; k++) {
                TxBlock[k].next = k + 1;
                event_init(&TxBlock[k].gcwait);
                event_init(&TxBlock[k].waitor);
        }
        TxBlock[k].next = 0;
        event_init(&TxBlock[k].gcwait);
        event_init(&TxBlock[k].waitor);

        TxAnchor.freetid = 1;
        event_init(&TxAnchor.freewait);

        stattx.maxtid = 1;      /* statistics */

        /*
         * initialize transaction lock (tlock) table
         *
         * transaction lock id = tlock index
         * tlock id = 0 is reserved.
         */
        size = sizeof(tlock_t) * nTxLock;
        if ((TxLock = (tlock_t *)xmalloc(size, L2PSIZE, kernel_heap)) == NULL)
                return ENOMEM;

#ifdef _JFS_OS2
        /* initialize tlock table */
        for (k = 1; k < nTxLock - 1; k ++)
                TxLock[k].next = k + 1;
        TxLock[k].next = 0;
        event_init(&TxAnchor.freelockwait);
        event_init(&TxAnchor.lowlockwait);			// D230860
#endif	/* _JFS_OS2 */

        TxAnchor.freelock = 1;
        TxAnchor.tlocksInUse = 0;

        stattx.maxlid = 1;      /* statistics */

        return 0;
}


/*
 * NAME:        txBegin()
 *
 * FUNCTION:    start a transaction.
 *
 * PARAMETER:   ipmnt   - mount inode
 *              tid     - transaction id (out)
 *              flag    - force for nested tx;
 *
 * RETURN:
 *
 * note: flag force allows to start tx for nested tx
 * to prevent deadlock on logsync barrier;
 */
void
txBegin(
        inode_t *ipmnt,
        int32   *tid,
        uint32  flag)
{
        int32           t;
        tblock_t        *tblk;
        log_t           *log;

        MMPHPretxBegin();               /* (d201828) Performance trace hook */

        log = (log_t *)ipmnt->i_iplog;

        TXN_LOCK();

retry:
	if (flag != COMMIT_FORCE)						//PS21092003
	{
		/*
		 * synchronize with logsync barrier
		 */
		if (log->syncbarrier)
		{
			TXN_SLEEP(&log->syncwait,0);
			goto retry;
		}

// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
		/*
		 * Don't begin transaction if we're getting starved for tlocks
		 */
		if (TlocksLow && !flag)
		{
			TXN_SLEEP(&TxAnchor.lowlockwait, 0);
			goto retry;
		}

		if (TxAnchor.lazyQwait)				// D233382
		{						// D233382
			TXN_SLEEP(&TxAnchor.freewait,0);	// D233382
			goto retry;				// D233382
		}						// D233382
#endif /* _JFS_LAZYCOMMIT */
// END D230860
	}

        /*
         * allocate transaction id/block
         */
        if ((t = TxAnchor.freetid) == 0)
        {
                TXN_SLEEP(&TxAnchor.freewait,0);
                goto retry;
        }

        tblk = &TxBlock[t];
        TxAnchor.freetid = tblk->next;

        /*
         * initialize transaction
         */
        bzero(tblk, sizeof(tblock_t));

        tblk->ipmnt = ipmnt;
        ++log->logtid;
        tblk->logtid = log->logtid;
        tblk->waitor = EVENT_NULL;
        tblk->gcwait = EVENT_NULL;

        ++log->active;

        *tid = t;

#ifdef FreeJFS
        HIGHWATERMARK(stattx.maxtid,t);         /* statistics */
        INCREMENT(stattx.ntid);                 /* statistics */
#endif

        TXN_UNLOCK();

        MMPHPosttxBegin();              /* (d201828) Performance trace hook */
}


// BEGIN D233382
#ifdef _JFS_LAZYCOMMIT
/*
 * NAME:        txBeginAnon()
 *
 * FUNCTION:    start an anonymous transaction.
 *		Blocks if logsync or available tlocks are low to prevent
 *		anonymous tlocks from depleting supply.
 *
 * PARAMETER:   ipmnt   - mount inode
 *
 * RETURN:	none
 */
void
txBeginAnon(
        inode_t *ipmnt)
{
        log_t           *log;
        int32           ipl;       //PS21092003

        log = (log_t *)ipmnt->i_iplog;

        TXN_LOCK();

retry:
        /*
         * synchronize with logsync barrier
         */
        if (log->syncbarrier)
        {
                TXN_SLEEP(&log->syncwait,0);
                goto retry;
        }
//PS 28092001 Begin
        if ( cachemgr.nfreecbufs < cachemgr.minfree )
        {
                TXN_UNLOCK();
                ipl=IOCACHE_LOCK();
                IOCACHE_SLEEP(&cachemgr.numiolru,3);
                IOCACHE_UNLOCK(ipl);
                TXN_LOCK();
                goto retry;
        }
//PS 28092001 End
	/*
	 * Don't begin transaction if we're getting starved for tlocks
	 */
	if (TlocksLow)
	{
		TXN_SLEEP(&TxAnchor.lowlockwait, 0);
		goto retry;
	}
//PS 28092001 Begin
       else
         if(TxAnchor.lazyQwait)
          {
		TXN_SLEEP(&TxAnchor.freewait, 0);
		goto retry;
          }
//PS 28092001 End
        TXN_UNLOCK();
}
#endif /* _JFS_LAZYCOMMIT */
// END D233382


/*
 *      txEnd()
 *
 * function: free specified transaction block.
 *
 *      logsync barrier processing:
 *
 * serialization:
 */
void
txEnd(
        int32   tid)
{
        tblock_t        *tblk = &TxBlock[tid];
        log_t           *log;

        MMPHPretxEnd();                 /* (d201828) Performance trace hook */

        TXN_LOCK();

        /*
         * wakeup transactions waiting on the page locked
         * by the current transaction
         */
        if (tblk->waitor != EVENT_NULL)
        {
                TXN_WAKEUP(&tblk->waitor);
        }

        log = (log_t *)tblk->ipmnt->i_iplog;			// D233382

// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	/*
	 * Lazy commit thread can't free this guy until we mark it UNLOCKED,
	 * otherwise, we would be left with a transaction that may have been
	 * reused.
	 *
	 * Lazy commit thread will turn off tblkGC_LAZY before calling this
	 * routine.
	 */
	if (tblk->flag & tblkGC_LAZY)
	{
jEVENT(0,("txEnd called w/lazy tid: 0x%x, tblk = 0x%x\n", tid, tblk));
		TXN_UNLOCK();

		XSPINLOCK_LOCK(&log->gclock);	    // LOGGC_LOCK	D233382
		tblk->flag |= tblkGC_UNLOCKED;
		XSPINLOCK_UNLOCK(0, &log->gclock);  // LOGGC_UNLOCK	D233382
              LAZY_LOCK();                        //PS21092003
              LAZY_WAKEUP();                      //PS21092003
              LAZY_UNLOCK();                      //PS21092003
		return;
	}
// END D230860

jEVENT(0,("txEnd: tid: 0x%x, tblk = 0x%x\n", tid, tblk));
#endif /* _JFS_LAZYCOMMIT */

        assert (tblk->next == 0);

        /*
         * insert tblock back on freelist
         */
        tblk->next = TxAnchor.freetid;
        TxAnchor.freetid = tid;

        /*
         * mark the tblock not active
         */
        --log->active;

        /*
         * synchronize with logsync barrier
         */
        if (log->syncbarrier && log->active == 0)
        {
                /* forward log syncpt */
                /* lmSync(log); */

jALERT(0,("     log barrier off: 0x%08x\n", log->logx->lsn));

#ifdef  _JFS_WIP_QUIESCE
                if (log->quiescewait != EVENT_NULL)
                {
                        e_wakeupx(&log->quiescewait, E_WKX_NO_PREEMPT);
                }
#endif  /* _JFS_WIP_QUIESCE */

                /* enable new transactions start */
                log->syncbarrier = 0;

                /* wakeup all waitors for logsync barrier */
                if (log->syncwait != EVENT_NULL)
                {
                        TXN_WAKEUP(&log->syncwait);
                }
        }

        /*
         * wakeup all waitors for a free tblock
         */
        if (TxAnchor.freewait != EVENT_NULL)
        {
                TXN_WAKEUP(&TxAnchor.freewait);
        }

        TXN_UNLOCK();

        MMPHPosttxEnd();                /* (d201828) Performance trace hook */
}


/*
 *      txLock()
 *
 * function: acquire a transaction lock on the specified <bp>
 *
 * parameter:
 *
 * return:      transaction lock id
 *
 * serialization:
 */
tlock_t *txLock(
        int32           tid,
        inode_t         *ip,
        jbuf_t          *bp,
        uint32          type)
{
        int32           lid, xtid;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        linelock_t      *linelock;
        int32           dev;
        int64           xaddr;
        int32           hash, h;
        xtpage_t        *p;
        int64           lblkno;
        int32           size;
        uint32          flag;
        cmctl_t         *ccp;

        TXN_LOCK();

retry:
        /* is page not locked by a transaction ? */
pageLock:
        if (bp == NULL || (lid = bp->j_lid) == 0)
                goto allocateLock;

jEVENT(0,("txLock: tid:%d ip:0x%08x bp:0x%08x grantLock:%d\n",
        tid, ip, bp, lid));

        /* is page locked by the requester transaction ? */
        tlck = &TxLock[lid];
        if ((xtid = tlck->tid) == tid)
                goto grantLock;

        /*
         * is page locked by anonymous transaction/lock ?
         *
         * (page update without transaction (i.e., file write) is
         * locked under anonymous transaction tid = 0:
         * anonymous tlocks maintained on anonymous tlock list of
         * the inode of the page and available to all anonymous
         * transactions until txCommit() time at which point
         * they are transferred to the transaction tlock list of
         * the commiting transaction of the inode)
         */
        if (xtid == 0)
        {
                tlck->tid = tid;
                goto grantLock;
        }

        goto waitLock;

        /*
         * allocate a tlock
         */
allocateLock:
        TXLOCK_ALLOC(lid);
        tlck = &TxLock[lid];

        /*
         * initialize tlock
         */
initLock:
        tlck->tid  = tid;

        /* mark tlock for meta-data page */
        if (bp->j_xflag & B_BUFFER)
        {
                assert(bp->cm_inuse);

                tlck->flag = tlckPAGELOCK;

                /* mark the page dirty and nohomeok */
                bp->cm_modified = TRUE;
                bp->j_nohomeok++;

jEVENT(0,("locking bp = 0x%x, j_nohomeok = %d tid=0x%x tlck=0x%x\n", bp,
	  bp->j_nohomeok, tid, tlck));

                /* upcount file nohomeok meta-data page count */
                ccp = bp->cm_ctl;
                ccp->cc_njbufs++;

		/* if anonymous transaction, and buffer is on the group
		 * commit synclist, mark inode to show this.  This will
		 * prevent the buffer from being marked nohomeok for too
		 * long a time.
		 */
		if ((tid == 0) && bp->j_lsn)
			ip->i_flag |= ISYNCLIST;
        }
        /* mark tlock for in-memory inode */
        else
                tlck->flag = tlckINODELOCK;

        tlck->type  = 0;

        /* bind the tlock and the page */
        tlck->ip  = ip;
        tlck->bp = bp;
        bp->j_lid = lid;

        /*
         * enqueue transaction lock to transaction/inode
         */
        /* insert the tlock at head of transaction tlock list */
        if (tid)
        {
                tlck->next = TxBlock[tid].next;
                TxBlock[tid].next = lid;
        }
        /* anonymous transaction:
         * insert the tlock at head of inode anonymous tlock list
         */
        else
        {
                tlck->next = ip->i_atlhead;
                if (ip->i_atlhead)
                        ip->i_atlhead = lid;
                else
                        ip->i_atlhead = ip->i_atltail = lid;
        }

        /* initialize type dependent area for linelock */
        linelock = (linelock_t *)&tlck->lock;
        linelock->next = 0;
        linelock->flag  = tlckLINELOCK;
        linelock->maxcnt = TLOCKSHORT;
        linelock->index = 0;

        switch (type & tlckTYPE)
        {
        case tlckDTREE:
                linelock->l2linesize = L2DTSLOTSIZE;
                break;

        case tlckXTREE:
                linelock->l2linesize = L2XTSLOTSIZE;

                xtlck = (xtlock_t *)linelock;
                xtlck->header.offset = 0;
                xtlck->header.length = 2;

                if (type & tlckNEW)
                {
                        xtlck->lwm.offset = XTENTRYSTART;
                }
                else
                {
                        if (bp->j_xflag & B_BUFFER)
                                p = (xtpage_t *)bp->b_bdata;
                        else
                                p = (xtpage_t *)&ip->i_btroot;
                        xtlck->lwm.offset = p->header.nextindex;
                }
                xtlck->lwm.length = 0;  /* ! */

                xtlck->index = 2;
                break;

        case tlckINODE:
                linelock->l2linesize = L2INODESLOTSIZE;
                break;

        default:
                jERROR(1,("UFO tlock:0x%08x\n", tlck));
                brkpoint1(tlck);
        }

        /*
         * update tlock vector
         */
grantLock:
        tlck->type |= type;

        TXN_UNLOCK();

        return tlck;

        /*
         * page is being locked by another transaction:
         */
waitLock:
#ifdef FreeJFS
        INCREMENT(stattx.waitlock);     /* statistics */
#endif
        bmRelease(bp);

        TxBlock[tid].locker = xtid;
        TXN_SLEEP(&TxBlock[xtid].waitor,T_NORELOCK);
        TXN_NOLOCK();

        return NULL;
}


// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
/*
 * NAME:        txRelease()
 *
 * FUNCTION:    Release buffers associated with transaction locks, but don't
 *		mark homeok yet.  The allows other transactions to modify
 *		buffers, but won't let them go to disk until commit record
 *		actually gets written.
 *
 * PARAMETER:
 *              tblk    -
 *
 * RETURN:      Errors from subroutines.
 */
static void
txRelease(
        tblock_t        *tblk)
{
        jbuf_t          *bp;
        int32           lid;
        tlock_t         *tlck;

	TXN_LOCK();						// D233382

        for (lid = tblk->next; lid > 0; lid = tlck->next)
        {
		tlck = &TxLock[lid];
		if ((bp = tlck->bp) != NULL &&
		    (tlck->type & tlckBTROOT) == 0)
		{
			assert(bp->j_xflag & B_BUFFER);
			bp->j_lid = 0;
		}
	}

// BEGIN D233382
        /*
         * wakeup transactions waiting on a page locked
         * by the current transaction
         */
        if (tblk->waitor != EVENT_NULL)
        {
                TXN_WAKEUP(&tblk->waitor);
        }

	TXN_UNLOCK();
// END D233382
}
#endif /* _JFS_LAZYCOMMIT */
// END D230860


/*
 * NAME:        txUnlock()
 *
 * FUNCTION:    Initiates pageout of pages modified by tid in journalled
 *              objects and frees their lockwords.
 *
 * PARAMETER:
 *              flag    -
 *
 * RETURN:      Errors from subroutines.
 */
static void
txUnlock(
        tblock_t        *tblk,
        uint32          flag)
{
        tlock_t         *tlck;
        linelock_t      *linelock;
        int32           lid, next, llid, k;
        jbuf_t          *bp;
        log_t           *log;
        logx_t          *logx;
        int32           lpri;
        uint32          force;
        int32           difft, diffp;

jEVENT(0,("txUnlock: tblk = 0x%x\n", tblk));
        log = (log_t *)tblk->ipmnt->i_iplog;
        logx = log->logx;
        force = flag & COMMIT_FLUSH;
        if (log->syncbarrier)
                force |= COMMIT_FORCE;

        /*
         * mark page under tlock homeok (its log has been written):
         * if caller has specified FORCE (e.g., iRecycle()), or
         * if syncwait for the log is set (i.e., the log sync point
         * has fallen behind), or
         * if syncpt is set for the page, or
         * if the page is new, initiate pageout;
         * otherwise, leave the page in memory.
         */
        for (lid = tblk->next; lid > 0; lid = next)
        {
                tlck = &TxLock[lid];
                next = tlck->next;

jEVENT(0,("unlocking lid = 0x%x, tlck = 0x%x\n", lid, tlck));

                /* unbind page from tlock */
                if ((bp = tlck->bp) != NULL &&
                    (tlck->type & tlckBTROOT) == 0)
                {
                        assert(bp->j_xflag & B_BUFFER);

                        /* hold buffer B_BUSY */
                        bmHold(bp);

                        assert(bp->j_nohomeok > 0);
                        bp->j_nohomeok--;

                        /* inherit younger/larger clsn */
                        if (bp->j_clsn)
                        {
                                logdiff(difft, tblk->clsn, logx);
                                logdiff(diffp, bp->j_clsn, logx);
                                if (difft > diffp)
                                        bp->j_clsn = tblk->clsn;
                        }
                        else
                                bp->j_clsn = tblk->clsn;

#ifndef _JFS_LAZYCOMMIT						// D230860
                        bp->j_lid = 0;
#endif								// D230860

                        if (tlck->flag & tlckWRITEPAGE)
                                bmLazyWrite(bp, force);
                        else if (tlck->flag & tlckFREEPAGE)
                        {
                                assert(bp->j_nohomeok == 0);
                                bmInvalidate(bp);
                        }
                        /* release page which has been forced */
                        else
                        {
                                bp->j_xflag &= ~B_SYNCPT;
                                bmRelease(bp);
                        }
                }

                /* insert tlock, and linelock(s) of the tlock if any,
                 * at head of freelist
                 */
                TXN_LOCK();

                llid = ((linelock_t *)&tlck->lock)->next;
                while (llid)
                {
                        linelock = (linelock_t *)&TxLock[llid];
                        k = linelock->next;
                        TXLOCK_FREE(llid);
                        llid = k;
                }
                TXLOCK_FREE(lid);

                TXN_UNLOCK();
        }
        tblk->next = 0;

        /*
         * remove tblock from logsynclist
         * (allocation map pages inherited lsn of tblk and
         * has been inserted in logsync list at txUpdateMap())
         */
        if (tblk->lsn)
        {
                lpri = LOGSYNC_LOCK(logx);

                logx->count--;

                CDLL_REMOVE(&logx->synclist, (logsyncblk_t *)tblk, synclist);

                LOGSYNC_UNLOCK(lpri, logx);
        }
}


/*
 *      txMaplock()
 *
 * function: allocate a transaction lock for freed page/entry;
 *      for freed page, maplock is used as xtlock/dtlock type;
 */
tlock_t *txMaplock(
        int32           tid,
        inode_t         *ip,
        uint32          type)
{
        int32           lid;
        tlock_t         *tlck;
        maplock_t       *maplock;

        TXN_LOCK();

        /*
         * allocate a tlock
         */
        TXLOCK_ALLOC(lid);
        tlck = &TxLock[lid];

        TXN_UNLOCK();

        /*
         * initialize tlock
         */
        tlck->tid  = tid;

        /* bind the tlock and the object */
        tlck->flag = tlckINODELOCK;
        tlck->ip  = ip;
        tlck->bp = NULL;

        tlck->type  = type;

        /*
         * enqueue transaction lock to transaction/inode
         */
        /* insert the tlock at head of transaction tlock list */
        if (tid)
        {
                tlck->next = TxBlock[tid].next;
                TxBlock[tid].next = lid;
        }
        /* anonymous transaction:
         * insert the tlock at head of inode anonymous tlock list
         */
        else
        {
                tlck->next = ip->i_atlhead;
                if (ip->i_atlhead)
                        ip->i_atlhead = lid;
                else
                        ip->i_atlhead = ip->i_atltail = lid;
        }

        /* initialize type dependent area for maplock */
        maplock = (maplock_t *)&tlck->lock;
        maplock->next = 0;
        maplock->maxcnt = 0;
        maplock->index = 0;

        return tlck;
}


/*
 *      txLinelock()
 *
 * function: allocate a transaction lock for log vector list
 */
linelock_t *txLinelock(
        linelock_t      *tlock)
{
        int32           lid;
        tlock_t         *tlck;
        linelock_t      *linelock;

        TXN_LOCK();

        /* allocate a TxLock structure */
        TXLOCK_ALLOC(lid);
        tlck = &TxLock[lid];

        TXN_UNLOCK();

        /* initialize linelock */
        linelock = (linelock_t *)tlck;
        linelock->next = 0;
        linelock->flag = tlckLINELOCK;
        linelock->maxcnt = TLOCKLONG;
        linelock->index = 0;

        /* append linelock after tlock */
        linelock->next = tlock->next;
        tlock->next = lid;

        return linelock;
}



/*
 *              transaction commit management
 *              -----------------------------
 */

/*
 * NAME:        txCommit()
 *
 * FUNCTION:    commit the changes to the objects specified in
 *              clist. Both the data and the inode on disk are changed
 *              if the IFSYNC flag is set; otherwise only the  disk
 *              inode is changed. For journalled segments only the
 *              changes of the caller are committed, ie by tid.
 *              for non-journalled segments the data are flushed to
 *              disk and then the change to the disk inode and indirect
 *              blocks committed (so blocks newly allocated to the
 *              segment will be made a part of the segment atomically).
 *
 *              The i_flags IACC and ICHG are cleared. if IFSYNC is
 *              set then IFSYNC and IUPD are also cleared.
 *
 *              all of the segments specified in clist must be in
 *              one file system. no more than 6 segments are needed
 *              to handle all unix svcs.
 *
 *              if the i_nlink field (i.e. disk inode link count)
 *              is zero, and the type of inode is a regular file or
 *              directory, or symbolic link , the inode is truncated
 *              to zero length. the truncation is committed but the
 *              VM resources are unaffected until it is closed (see
 *              iput and iclose).
 *
 * PARAMETER:
 *
 * RETURN:
 *
 * serialization:
 *              on entry the inode lock on each segment is assumed
 *              to be held.
 *
 * i/o error:
 */
txCommit(
        int32   tid,            /* transaction identifier */
        int32   nip,            /* number of inodes to commit */
        inode_t **iplist,       /* list of inode to commit */
        uint32  flag)
{
        int32           rc = 0, rc1 = 0;
        commit_t        cd;
        log_t           *log;
        tblock_t        *tblk;
        tlock_t         *tlck;
        lrd_t           *lrd;
        int32           lid;
        int32           lsn;
        inode_t         *ip, *ipmnt;
        int32           k, n;
        ino_t           top;
        int32           tid1;

        MMPHPretxCommit();              /* (d201828) Performance trace hook */

        /* is read-only file system ? */
        if (isReadOnly(iplist[0]))
        {
                rc = EROFS;
                goto TheEnd;
        }

        ipmnt = iplist[0]->i_ipmnt;
        cd.ipmnt = ipmnt;
        tid1=tid;          //PS21092003        

        if (tid == 0)
                txBegin(ipmnt, &tid, 0);
        tblk = &TxBlock[tid];
        cd.tid=tid;         //PS 28092001        

        /*
         * initialize commit structure
         */
        cd.ipimap  = iplist[0]->i_ipimap;
        cd.ipbmap  = ipmnt->i_ipbmap;
        log = (log_t *)ipmnt->i_iplog;
        cd.log = log;

        /* initialize log record descriptor in commit */
        lrd = &cd.lrd;
        lrd->logtid = tblk->logtid;
        lrd->backchain = 0;

#ifdef _JFS_LAZYCOMMIT
//PS 28092001 Begin
        for (k = 0; k < nip; k++)				
            {
            ip = iplist[k];
            if ( ip->i_mode & ISWAPFILE )
               flag |= COMMIT_FORCE;
            }
//PS 28092001 End
	tblk->xflag |= flag;					// D233382

        if (LazyCommitRunning && ((flag & COMMIT_FORCE) == 0) && // D230860
	    ((tblk->xflag & COMMIT_DELETE) == 0))		// D230860
		tblk->xflag |= COMMIT_LAZY;			// D230860
#endif /* _JFS_LAZYCOMMIT */
        /*
         *      prepare non-journaled objects for commit
         *
         * lock the memory object of the regular file object
         * if the IFSYNC flag is set to prevent mmappers
         * from adding new disk blocks.
         *
         * flush data pages of non-journaled file
         * to prevent the file getting non-initialized disk blocks
         * in case of crash.
         * (new blocks - )
         */
	cd.iplist = iplist;					// F226941
        cd.nip = nip;
#ifndef _JFS_LAZYCOMMIT						// D230860
        for (k = 0; k < nip; k++)				// F226941
        {
                ip = iplist[k];

                if (ip->i_mode & IFJOURNAL)
                        continue;

                if ((ip->i_mode & IFMT) == IFREG)
                {
                        /* pageout data pages synchronously */
                        rc = iFlushCache(ip);
                        if (rc1 == 0)
                                rc1 = rc;
                }
        }
#endif /* _JFS_LAZYCOMMIT */					// D230860

        /*
         *      acquire transaction lock on (on-disk) inodes
         *
         * update on-disk inode from in-memory inode
         * acquiring transaction locks for AFTER records
         * on the on-disk inode of file object
         *
         * sort the inodes array by i_number in descending order
         * to prevent deadlock when acquiring transaction lock
         * of on-disk inodes on multiple on-disk inode pages by
         * multiple concurrent transactions
         */
        for (k = 0; k < cd.nip; k++)
        {
                top = (cd.iplist[k])->i_number;
                for (n = k + 1; n < cd.nip; n++)
                {
                        ip = cd.iplist[n];
                        if (ip->i_number > top)
                        {
                                top = ip->i_number;
                                cd.iplist[n] = cd.iplist[k];
                                cd.iplist[k] = ip;
                        }
                }

                ip = cd.iplist[k];

//PS 28092001         if ((ip->i_mode & IFMT) & IFREG)
//PS 28092001         {
                        /* inherit anonymous tlock(s) of inode */
                        if (ip->i_atlhead)
                        {
                                TxLock[ip->i_atltail].next = tblk->next;
                                tblk->next = ip->i_atlhead;
                                ip->i_atlhead = ip->i_atltail = 0;
                        }
//PS 28092001          }

                /*
                 * acquire transaction lock on on-disk inode page
                 * (become first tlock of the tblk's tlock list)
                 */
                if (rc = diWrite(tid, ip))
                        goto out;
        }

        /*
         *      write log records from transaction locks
         *
         * txUpdateMap() resets XAD_NEW in XAD.
         */
        if (rc = txLog(log, tblk, &cd))
                goto TheEnd;

	/*
	 * force log only if the IFSYNC flag was set or more than one file
	 * was in commit list
	 */
        if ((cd.iplist[0])->i_flag & IFSYNC || cd.nip > 1 ||
            flag & COMMIT_FORCE)
                tblk->xflag |= COMMIT_SYNC;

#ifdef _JFS_LAZYCOMMIT
//PS21092003        for (k = 0; k < nip; k++,rc1=rc)
     for (k = 0; k < nip; k++)
        {
                ip = iplist[k];
                if (ip->i_mode & IFJOURNAL)
                        continue;
                if ((ip->i_mode & IFMT) == IFREG)
                {
			if (tblk->xflag & (COMMIT_CREATE | COMMIT_DELETE))
			{
				assert(tblk->ip == ip);
			}
			if (ip->i_flag & IUPDNEW)
			{
				/* pageout data pages synchronously */
				rc = iFlushCache(ip);
				if (rc1 == 0)
				rc1 = rc;
			}
			else if (ip->i_cacheid)
				/* pageout data pages asynchronously */
				cmAsyncWrite(ip->i_cacheid, 0,
//PS21092003					CM_BTOBLKS(ip->i_size, ipmnt->i_bsize,
					CM_BTOFBLKS(ip->i_size, ipmnt->i_l2bsize));
		}
        }
	/*
	 * Ensure that inode isn't reused before
	 * lazy commit thread finishes processing
	 */
	if (tblk->xflag & COMMIT_CREATE)
		jfs_hold(IP2VP(tblk->ip));

#endif /* _JFS_LAZYCOMMIT */
// END D230860

        /*
         *      write COMMIT log record
         */
        lrd->type = LOG_COMMIT;
        lrd->length = 0;
        lsn = lmLog(log, tblk, lrd, NULL);

#ifdef _JFS_LAZYCOMMIT  //PS21092003
        if (tblk->xflag & COMMIT_SYNC)
	{
// BEGIN D233382
//PS21092003 #ifdef _JFS_LAZYCOMMIT
		if (LazyCommitRunning && !(flag & COMMIT_FORCE))
		{
			LAZY_LOCK();
			TxAnchor.lazyQsize++;
			if ((TxAnchor.lazyQwait == 0) &&
			    (TxAnchor.lazyQsize > LazyHWM))
			{
jEVENT(0,("Lazy Commit Queue hit HWM\n"));
				TxAnchor.lazyQwait = 1;
			}
			LAZY_UNLOCK();
		}
//PS21092003 #endif /* _JFS_LAZYCOMMIT */
// END D233382
                lmGroupCommit(log, tblk);
//PS21092003 Begin
                if (flag & COMMIT_FORCE)
                   txForce(tblk);

                if (! LazyCommitRunning || (flag & COMMIT_FORCE))  
                    txUpdateMap(tblk);           
	}
    else                  
            /*
             * non-sync transactions SHOULDN'T be modifying the inode or
             * block maps, BUT if it does happen, the pmaps are never updated
             * unless we call txUpdateMap().
             */
            txUpdateMap(tblk);               // D242170

        txRelease(tblk);                         // D230860

        if ((tblk->flag & tblkGC_LAZY) == 0)               // D230860
            txUnlock(tblk, flag);
#else  /* ! _JFS_LAZYCOMMIT */
        if (tblk->xflag & COMMIT_SYNC)
                lmGroupCommit(log, tblk);
//PS End
//	}

        /*
         *      - transaction is now committed -
         */

        /*
         * force pages in careful update
         * (imap addressing structure update)
         */
        if (flag & COMMIT_FORCE)
                txForce(tblk);

        /*
         *      update allocation map.
         *
         * update inode allocation map and inode:
         * free pager lock on memory object of inode if any.
         * update  block allocation map.
         *
         * txUpdateMap() resets XAD_NEW in XAD.
         */
#ifdef Drova   //PS21092003
	if (! LazyCommitRunning || (flag & COMMIT_FORCE))	// D233382
		txUpdateMap(tblk);				// D230860
#else
//        txUpdateMap(tblk, &cd);
#endif

        /*
         *      free transaction locks and pageout/free pages
         */
//PS21092003	txRelease(tblk);					// D230860

//	if ((tblk->flag & tblkGC_LAZY) == 0)			// D230860
        	txUnlock(tblk, flag);
#endif /* _JFS_LAZYCOMMIT */


        /*
         *      reset in-memory object state
         */
        for (k = 0; k < cd.nip; k++)
        {
                ip = cd.iplist[k];

                /*
                 * reset in-memory inode state
                 */
                ip->i_flag &= ~(IFSYNC|IACC|ICHG|IUPD|IUPDNEW|ISYNCLIST);

                ip->i_synctime = 0;

                ip->i_bxflag = 0;
                ip->i_blid = 0;
        }

out:
        if (rc != 0)
                txAbortCommit(&cd, rc);
        else
            {                               //PS 28092001
                rc = rc1;
                if(tid1 == 0)             //PS 28092001
                   txEnd(tid);              //PS 28092001
            }                               //PS 28092001

TheEnd:
        MMPHPosttxCommit();             /* (d201828) Performance trace hook */
        return rc;
}


/*
 * NAME:        txLog()
 *
 * FUNCTION:    Writes AFTER log records for all lines modified
 *              by tid for segments specified by inodes in comdata.
 *              Code assumes only WRITELOCKS are recorded in lockwords.
 *
 * PARAMETERS:
 *
 * RETURN :
 */
static int32
txLog(
        log_t           *log,
        tblock_t        *tblk,
        commit_t        *cd)
{
        int32           rc = 0;
        inode_t         *ip;
        int32           lid;
        tlock_t         *tlck;
        lrd_t           *lrd = &cd->lrd;

        /*
         * write log record(s) for each tlock of transaction,
         */
        for (lid = tblk->next; lid > 0; lid = tlck->next)
        {
                tlck = &TxLock[lid];

                tlck->flag |= tlckLOG;

                /* initialize lrd common */
                ip = tlck->ip;
                lrd->aggregate = ip->i_dev;
                lrd->log.redopage.fileset = ip->i_fileset;
                lrd->log.redopage.inode = ip->i_number;

                /* write log record of page from the tlock */
                switch (tlck->type & tlckTYPE)
                {
                case tlckXTREE:
                        xtLog(log, tblk, lrd, tlck);
                        break;

                case tlckDTREE:
                        dtLog(log, tblk, lrd, tlck);
                        break;

                case tlckINODE:
                        diLog(log, tblk, lrd, tlck, cd);
                        break;

                case tlckMAP:
                        mapLog(log, tblk, lrd, tlck);
                        break;

                default:
                        jERROR(1,("UFO tlock:0x%08x\n", tlck));
                        brkpoint();
                }
        }

        return rc;
}


/*
 *      diLog()
 *
 * function:    log inode tlock and format maplock to update bmap;
 */
int32 diLog(
        log_t           *log,
        tblock_t        *tblk,
        lrd_t           *lrd,
        tlock_t         *tlck,
        commit_t        *cd)
{
        int32           rc = 0;
        jbuf_t          *bp;
        pxd_t           *pxd;
        pxdlock_t       *pxdlock;
        int32           i, nlock;

        bp = tlck->bp;

        /* initialize as REDOPAGE record format */
        lrd->log.redopage.type = LOG_INODE;
        lrd->log.redopage.l2linesize = L2INODESLOTSIZE;

        pxd = &lrd->log.redopage.pxd;

        /*
         *      inode after image
         */
        if (tlck->type & tlckENTRY)
        {
                /* log after-image for logredo(): */
                lrd->type = LOG_REDOPAGE;
                *pxd = bp->cm_pxd;
                lrd->backchain = lmLog(log, tblk, lrd, tlck);

                /* mark page as homeward bound */
                tlck->flag |= tlckWRITEPAGE;
        }
        /*
         *      free inode extent
         *
         * (pages of the freed inode extent have been invalidated and
         * a maplock for free of the extent has been formatted at txLock()
         * time);
         *
         * the tlock had been acquired on the inode allocation map page
         * (iag) that specifies the freed extent, even though the map
         * page is not itself logged, to prevent pageout of the map
         * page before the log;
         */
        else if (tlck->type & tlckFREE)
        {
                assert(tlck->type & tlckFREE);

                /* log LOG_NOREDOINOEXT of the freed inode extent for
                 * logredo() to start NoRedoPage filters, and to update
                 * imap and bmap for free of the extent;
                 */
                lrd->type = LOG_NOREDOINOEXT;
                        /*
                         * For the LOG_NOREDOINOEXT record, we need
                         * to pass the IAG number and inode extent
                         * index (within that IAG) from which the
                         * the extent being released.  These have been
                         * passed to us in the iplist[1] and iplist[2].
                         */
                lrd->log.noredoinoext.iagnum = (int32) (cd->iplist[1]);
                lrd->log.noredoinoext.inoext_idx = (int32) (cd->iplist[2]);

                pxdlock = (pxdlock_t *)&tlck->lock;
                *pxd = pxdlock->pxd;
                lrd->backchain = lmLog(log, tblk, lrd, NULL);

                /* update bmap */
                tlck->flag |= tlckUPDATEMAP;

                /* mark page as homeward bound */
                tlck->flag |= tlckWRITEPAGE;
        }
        else
        {
                jERROR(2,("diLog: UFO type tlck:0x%08x\n", tlck));
        }
#ifdef  _JFS_WIP
        /*
         *      alloc/free external EA extent
         *
         * a maplock for txUpdateMap() to update bPWMAP for alloc/free
         * of the extent has been formatted at txLock() time;
         */
        else
        {
                assert(tlck->type & tlckEA);

                /* log LOG_UPDATEMAP for logredo() to update bmap for
                 * alloc of new (and free of old) external EA extent;
                 */
                lrd->type = LOG_UPDATEMAP;
                pxdlock = (pxdlock_t *)&tlck->lock;
                nlock = pxdlock->index;
                for (i = 0; i < nlock; i++, pxdlock++)
                {
                        if (pxdlock->flag & mlckALLOCPXD)
                                lrd->log.updatemap.type = LOG_ALLOCPXD;
                        else
                                lrd->log.updatemap.type = LOG_FREEPXD;
                        lrd->log.updatemap.nxd = 1;
                        lrd->log.updatemap.pxd = pxdlock->pxd;
                        lrd->backchain = lmLog(log, tblk, lrd, NULL);
                }

                /* update bmap */
                tlck->flag |= tlckUPDATEMAP;
        }
#endif  /* _JFS_WIP */

        return rc;
}


/*
 *      dtLog()
 *
 * function:    log dtree tlock and format maplock to update bmap;
 */
void dtLog(
        log_t           *log,
        tblock_t        *tblk,
        lrd_t           *lrd,
        tlock_t         *tlck)
{
        inode_t         *ip;
        jbuf_t          *bp;
        maplock_t       *maplock;
        pxdlock_t       *pxdlock;
        pxd_t           *pxd;

        ip = tlck->ip;
        bp = tlck->bp;

        /* initialize as REDOPAGE/NOREDOPAGE record format */
        lrd->log.redopage.type = LOG_DTREE;
        lrd->log.redopage.l2linesize = L2DTSLOTSIZE;

        pxd = &lrd->log.redopage.pxd;

        if (tlck->type & tlckBTROOT)
                lrd->log.redopage.type |= LOG_BTROOT;

        /*
         *      page extension via relocation: entry insertion;
         *      page extension in-place: entry insertion;
         *      new right page from page split, reinitialized in-line
         *      root from root page split: entry insertion;
         */
        if (tlck->type & (tlckNEW|tlckEXTEND))
        {
                /* log after-image of the new page for logredo():
                 * mark log (LOG_NEW) for logredo() to initialize
                 * freelist and update bmap for alloc of the new page;
                 */
                lrd->type = LOG_REDOPAGE;
                if (tlck->type & tlckEXTEND)
                        lrd->log.redopage.type |= LOG_EXTEND;
                else
                        lrd->log.redopage.type |= LOG_NEW;
                *pxd = bp->cm_pxd;
                lrd->backchain = lmLog(log, tblk, lrd, tlck);

                /* format a maplock for txUpdateMap() to update bPMAP for
                 * alloc of the new page;
                 */
                if (tlck->type & tlckBTROOT)
                        return;
                tlck->flag |= tlckUPDATEMAP;
                pxdlock = (pxdlock_t *)&tlck->lock;
                pxdlock->flag = mlckALLOCPXD;
                pxdlock->pxd = *pxd;

                pxdlock->index = 1;

                /* mark page as homeward bound */
                tlck->flag |= tlckWRITEPAGE;
                return;
        }

        /*
         *      entry insertion/deletion,
         *      sibling page link update (old right page before split);
         */
        if (tlck->type & (tlckENTRY|tlckRELINK))
        {
                /* log after-image for logredo(): */
                lrd->type = LOG_REDOPAGE;
                *pxd = bp->cm_pxd;
                lrd->backchain = lmLog(log, tblk, lrd, tlck);

                /* mark page as homeward bound */
                tlck->flag |= tlckWRITEPAGE;
                return;
        }

        /*
         *      page deletion: page has been invalidated
         *      page relocation: source extent
         *
         *      a maplock for free of the page has been formatted
         *      at txLock() time);
         */
        if (tlck->type & (tlckFREE|tlckRELOCATE))
        {
                /* log LOG_NOREDOPAGE of the deleted page for logredo()
                 * to start NoRedoPage filter and to update bmap for free
                 * of the deletd page
                 */
                lrd->type = LOG_NOREDOPAGE;
                pxdlock = (pxdlock_t *)&tlck->lock;
                *pxd = pxdlock->pxd;
                lrd->backchain = lmLog(log, tblk, lrd, NULL);

                /* a maplock for txUpdateMap() for free of the page
                 * has been formatted at txLock() time;
                 */
                tlck->flag |= tlckUPDATEMAP;
        }
        return;
}


/*
 *      xtLog()
 *
 * function:    log xtree tlock and format maplock to update bmap;
 */
void xtLog(
        log_t           *log,
        tblock_t        *tblk,
        lrd_t           *lrd,
        tlock_t         *tlck)
{
        inode_t         *ip;
        jbuf_t          *bp;
        xtpage_t        *p;
        xtlock_t        *xtlck;
        maplock_t       *maplock;
        xdlistlock_t    *xadlock;
        pxdlock_t       *pxdlock;
        pxd_t           *pxd;
        int32           next, lwm, hwm;
	int32		i;					// D230860

        ip = tlck->ip;
        bp = tlck->bp;

        /* initialize as REDOPAGE/NOREDOPAGE record format */
        lrd->log.redopage.type = LOG_XTREE;
        lrd->log.redopage.l2linesize = L2XTSLOTSIZE;

        pxd = &lrd->log.redopage.pxd;

        if (tlck->type & tlckBTROOT)
        {
                lrd->log.redopage.type |= LOG_BTROOT;
                p = (xtpage_t *)&ip->i_btroot;
        }
        else
                p = (xtpage_t *)bp->b_bdata;
        next = p->header.nextindex;

        xtlck = (xtlock_t *)&tlck->lock;

        maplock = (maplock_t *)&tlck->lock;
        xadlock = (xdlistlock_t *)maplock;

        /*
         *      entry insertion/extension;
         *      sibling page link update (old right page before split);
         */
        if (tlck->type & (tlckNEW|tlckGROW|tlckRELINK))
        {
                /* log after-image for logredo():
                 * logredo() will update bmap for alloc of new/extended
                 * extents (XAD_NEW|XAD_EXTEND) of XAD[lwm:next) from
                 * after-image of XADlist;
                 * logredo() resets (XAD_NEW|XAD_EXTEND) flag when
                 * applying the after-image to the meta-data page.
                 */
                lrd->type = LOG_REDOPAGE;
                *pxd = bp->cm_pxd;
                lrd->backchain = lmLog(log, tblk, lrd, tlck);

                /* format a maplock for txUpdateMap() to update bPMAP
                 * for alloc of new/extended extents of XAD[lwm:next)
                 * from the page itself;
                 * txUpdateMap() resets (XAD_NEW|XAD_EXTEND) flag.
                 */
                lwm = xtlck->lwm.offset;
		if (lwm == 0)
			lwm = XTPAGEMAXSLOT;

                if (lwm == next)
                        goto out;
                tlck->flag |= tlckUPDATEMAP;
                xadlock->flag = mlckALLOCXADLIST;
                xadlock->count = next - lwm;
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
		if ((xadlock->count <= 2) && (tblk->xflag & COMMIT_LAZY))
		{
			/*
			 * Lazy commit may allow xtree to be modified before
			 * txUpdateMap runs.  Copy xad into linelock to
			 * preserve correct data.
			 */
			xadlock->xdlist = &xtlck->pxdlock;
			bcopy(&p->xad[lwm], xadlock->xdlist,
			      sizeof(xad_t) * xadlock->count);

			for (i = 0; i < xadlock->count; i++)
				p->xad[lwm+i].flag &= ~(XAD_NEW|XAD_EXTENDED);
		}
		else
		{
			/*
			 * xdlist will point to into inode's xtree, ensure
			 * that transaction is not committed lazily.
			 */
                	xadlock->xdlist = &p->xad[lwm];
			tblk->xflag &= ~COMMIT_LAZY;
		}
#else /* _JFS_LAZYCOMMIT */
// END D230860
                xadlock->xdlist = &p->xad[lwm];
#endif /* _JFS_LAZYCOMMIT */
jEVENT(0,("xtLog: alloc ip:0x%08x bp:0x%08x tlck:0x%x lwm:%d count:%d\n",
        tlck->ip, bp, tlck, lwm, xadlock->count));

                maplock->index = 1;

out:
                /* mark page as homeward bound */
                tlck->flag |= tlckWRITEPAGE;

                return;
        }

        /*
         *      page deletion: file deletion/truncation (ref. xtTruncate())
         *
         * (page will be invalidated after log is written and bmap
         * is updated from the page);
         */
        if (tlck->type & tlckFREE)
        {
                /* LOG_NOREDOPAGE log for NoRedoPage filter:
                 * if page free from file delete, NoRedoFile filter from
                 * inode image of zero link count will subsume NoRedoPage
                 * filters for each page;
                 * if page free from file truncattion, write NoRedoPage
                 * filter;
                 *
                 * upadte of block allocation map for the page itself:
                 * if page free from deletion and truncation, LOG_UPDATEMAP
                 * log for the page itself is generated from processing
                 * its parent page xad entries;
                 */
                /* if page free from file truncation, log LOG_NOREDOPAGE
                 * of the deleted page for logredo() to start NoRedoPage
                 * filter for the page;
                 */
                if (tblk->xflag & COMMIT_TRUNCATE)
                {
                        /* write NOREDOPAGE for the page */
                        lrd->type = LOG_NOREDOPAGE;
                        *pxd = bp->cm_pxd;
                        lrd->backchain = lmLog(log, tblk, lrd, NULL);
                }

                /* init LOG_UPDATEMAP of the freed extents
                 * XAD[XTENTRYSTART:hwm) from the deleted page itself
                 * for logredo() to update bmap;
                 */
                lrd->type = LOG_UPDATEMAP;
                lrd->log.updatemap.type = LOG_FREEXADLIST;
                xtlck = (xtlock_t *)&tlck->lock;
                hwm = xtlck->hwm.offset;
                lrd->log.updatemap.nxd = hwm - XTENTRYSTART + 1;
                /* reformat linelock for lmLog() */
                xtlck->header.offset = XTENTRYSTART;
                xtlck->header.length = hwm - XTENTRYSTART + 1;
                xtlck->index = 1;
                lrd->backchain = lmLog(log, tblk, lrd, tlck);

                /* format a maplock for txUpdateMap() to update bmap
                 * to free extents of XAD[XTENTRYSTART:hwm) from the
                 * deleted page itself;
                 */
                tlck->flag |= tlckUPDATEMAP;
                xadlock->flag = mlckFREEXADLIST;
                xadlock->count = hwm - XTENTRYSTART + 1;
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
		if ((xadlock->count <= 2) && (tblk->xflag & COMMIT_LAZY))
		{
			/*
			 * Lazy commit may allow xtree to be modified before
			 * txUpdateMap runs.  Copy xad into linelock to
			 * preserve correct data.
			 */
			xadlock->xdlist = &xtlck->pxdlock;
			bcopy(&p->xad[XTENTRYSTART], xadlock->xdlist,
			      sizeof(xad_t) * xadlock->count);
		}
		else
		{
			/*
			 * xdlist will point to into inode's xtree, ensure
			 * that transaction is not committed lazily.
			 */
                	xadlock->xdlist = &p->xad[XTENTRYSTART];
			tblk->xflag &= ~COMMIT_LAZY;
		}
#else /* ! _JFS_LAZYCOMMIT */
// END D230860
                xadlock->xdlist = &p->xad[XTENTRYSTART];
#endif /* _JFS_LAZYCOMMIT */
jEVENT(0,("xtLog: free ip:0x%08x bp:0x%08x count:%d lwm:2\n",
        tlck->ip, bp, xadlock->count));

                maplock->index = 1;

                /* mark page as invalid */
                if (tblk->xflag & COMMIT_PWMAP)
                        tlck->flag |= tlckFREEPAGE;
                /*
                else (tblk->xflag & COMMIT_PMAP)
                         ? release the page;
                 */
                return;
        }

        /*
         *      page/entry truncation: file truncation (ref. xtTruncate())
         *
         *     |----------+------+------+---------------|
         *                |      |      |
         *                |      |     hwm - hwm before truncation
         *                |     next - truncation point
         *               lwm - lwm before truncation
         * header ?
         */
        if (tlck->type & tlckTRUNCATE)
        {
                pxd_t   tpxd;   /* truncated extent of xad */

#ifdef _JFS_LAZYCOMMIT
		/*
		 * For truncation the entire linelock may be used, so it would
		 * be difficult to store xad list in linelock itself.
		 * Therefore, we'll just force transaction to be committed
		 * synchronously, so that xtree pages won't be changed before
		 * txUpdateMap runs.
		 */
		tblk->xflag &= ~COMMIT_LAZY;			// D230860
#endif /* _JFS_LAZYCOMMIT */
                lwm = xtlck->lwm.offset;
		if (lwm == 0)
			lwm = XTPAGEMAXSLOT;
                hwm = xtlck->hwm.offset;

                /*
                 *      write log records
                 */
                /*
                 * allocate entries XAD[lwm:next):
                 */
                if (lwm < next)
                {
                        /* log after-image for logredo():
                         * logredo() will update bmap for alloc of new/extended
                         * extents (XAD_NEW|XAD_EXTEND) of XAD[lwm:next) from
                         * after-image of XADlist;
                         * logredo() resets (XAD_NEW|XAD_EXTEND) flag when
                         * applying the after-image to the meta-data page.
                         */
                        lrd->type = LOG_REDOPAGE;
                        *pxd = bp->cm_pxd;
                        lrd->backchain = lmLog(log, tblk, lrd, tlck);
                }

                /*
                 * truncate entry XAD[hwm == next - 1]:
                 */
                if (hwm == next - 1)
                {
                        /* init LOG_UPDATEMAP for logredo() to update bmap for
                         * free of truncated delta extent of the truncated
                         * entry XAD[next - 1]:
                         * (xtlck->pxdlock = truncated delta extent);
                         */
                        pxdlock = (pxdlock_t *)&xtlck->pxdlock;
                        /* assert(pxdlock->type & tlckTRUNCATE); */
                        lrd->type = LOG_UPDATEMAP;
                        lrd->log.updatemap.type = LOG_FREEPXD;
                        lrd->log.updatemap.nxd = 1;
                        lrd->log.updatemap.pxd = pxdlock->pxd;
                        tpxd = pxdlock->pxd; /* save to format maplock */
                        lrd->backchain = lmLog(log, tblk, lrd, NULL);
                }

                /*
                 * free entries XAD[next:hwm]:
                 */
                if (hwm >= next)
                {
                        /* init LOG_UPDATEMAP of the freed extents
                         * XAD[next:hwm] from the deleted page itself
                         * for logredo() to update bmap;
                         */
                        lrd->type = LOG_UPDATEMAP;
                        lrd->log.updatemap.type = LOG_FREEXADLIST;
                        xtlck = (xtlock_t *)&tlck->lock;
                        hwm = xtlck->hwm.offset;
                        lrd->log.updatemap.nxd = hwm - next + 1;  // D233382
                        /* reformat linelock for lmLog() */
                        xtlck->header.offset = next;		// D233382
                        xtlck->header.length = hwm - next + 1;	// D233382
                        xtlck->index = 1;
                        lrd->backchain = lmLog(log, tblk, lrd, tlck);
                }

                /*
                 *      format maplock(s) for txUpdateMap() to update bmap
                 */
                maplock->index = 0;

                /*
                 * allocate entries XAD[lwm:next):
                 */
                if (lwm < next)
                {
                        /* format a maplock for txUpdateMap() to update bPMAP
                         * for alloc of new/extended extents of XAD[lwm:next)
                         * from the page itself;
                         * txUpdateMap() resets (XAD_NEW|XAD_EXTEND) flag.
                         */
                        tlck->flag |= tlckUPDATEMAP;
                        xadlock->flag = mlckALLOCXADLIST;
                        xadlock->count = next - lwm;
                        xadlock->xdlist = &p->xad[lwm];

jEVENT(0,("xtLog: alloc ip:0x%08x bp:0x%08x count:%d lwm:%d next:%d\n",
        tlck->ip, bp, xadlock->count, lwm, next));
                        maplock->index++;
                        xadlock++;
                }

                /*
                 * truncate entry XAD[hwm == next - 1]:
                 */
                if (hwm == next - 1)
                {
                        pxdlock_t       *pxdlock;

                        /* format a maplock for txUpdateMap() to update bmap
                         * to free truncated delta extent of the truncated
                         * entry XAD[next - 1];
                         * (xtlck->pxdlock = truncated delta extent);
                         */
                        tlck->flag |= tlckUPDATEMAP;
                        pxdlock = (pxdlock_t *)xadlock;
                        pxdlock->flag = mlckFREEPXD;
                        pxdlock->count = 1;
                        pxdlock->pxd = tpxd;

jEVENT(0,("xtLog: truncate ip:0x%08x bp:0x%08x count:%d hwm:%d\n",
        ip, bp, pxdlock->count, hwm));
                        maplock->index++;
                        xadlock++;
                }

                /*
                 * free entries XAD[next:hwm]:
                 */
                if (hwm >= next)
                {
                        /* format a maplock for txUpdateMap() to update bmap
                         * to free extents of XAD[next:hwm] from thedeleted
                         * page itself;
                         */
                        tlck->flag |= tlckUPDATEMAP;
                        xadlock->flag = mlckFREEXADLIST;
                        xadlock->count = hwm - next + 1;
                        xadlock->xdlist = &p->xad[next];

jEVENT(0,("xtLog: free ip:0x%08x bp:0x%08x count:%d next:%d hwm:%d\n",
        tlck->ip, bp, xadlock->count, next, hwm));
                        maplock->index++;
                }

                /* mark page as homeward bound */
                tlck->flag |= tlckWRITEPAGE;
        }
        return;
}


/*
 *      mapLog()
 *
 * function:    log from maplock of freed data extents;
 */
void
mapLog(
        log_t           *log,
        tblock_t        *tblk,
        lrd_t           *lrd,
        tlock_t         *tlck)
{
        int32           rc = 0;
        pxdlock_t       *pxdlock;
        int32           i, nlock;
        pxd_t           *pxd;

        /*
         *      page relocation: free the source page extent
         *
         * a maplock for txUpdateMap() for free of the page
         * has been formatted at txLock() time saving the src
         * relocated page address;
         */
        if (tlck->type & tlckRELOCATE)
        {
                /* log LOG_NOREDOPAGE of the old relocated page
                 * for logredo() to start NoRedoPage filter;
                 */
                lrd->type = LOG_NOREDOPAGE;
                pxdlock = (pxdlock_t *)&tlck->lock;
                pxd = &lrd->log.redopage.pxd;
                *pxd = pxdlock->pxd;
                lrd->backchain = lmLog(log, tblk, lrd, NULL);

                /* (N.B. currently, logredo() does NOT update bmap
                 * for free of the page itself for (LOG_XTREE|LOG_NOREDOPAGE);
                 * if page free from relocation, LOG_UPDATEMAP log is
                 * specifically generated now for logredo()
                 * to update bmap for free of src relocated page;
                 * (new flag LOG_RELOCATE may be introduced which will
                 * inform logredo() to start NORedoPage filter and also
                 * update block allocation map at the same time, thus
                 * avoiding an extra log write);
                 */
                lrd->type = LOG_UPDATEMAP;
                lrd->log.updatemap.type = LOG_FREEPXD;
                lrd->log.updatemap.nxd = 1;
                lrd->log.updatemap.pxd = pxdlock->pxd;
                lrd->backchain = lmLog(log, tblk, lrd, NULL);

                /* a maplock for txUpdateMap() for free of the page
                 * has been formatted at txLock() time;
                 */
                tlck->flag |= tlckUPDATEMAP;
                return;
        }
        /*
         *
         * Otherwise it's not a relocate request
         *
         */
        else
        {
                /* log LOG_UPDATEMAP for logredo() to update bmap for
                 * free of truncated/relocated delta extent of the data;
                 * e.g.: external EA extent, relocated/truncated extent
                 * from xtTailgate();
                 */
                lrd->type = LOG_UPDATEMAP;
                pxdlock = (pxdlock_t *)&tlck->lock;
                nlock = pxdlock->index;
                for (i = 0; i < nlock; i++, pxdlock++)
                {
                        if (pxdlock->flag & mlckALLOCPXD)
                                lrd->log.updatemap.type = LOG_ALLOCPXD;
                        else
                                lrd->log.updatemap.type = LOG_FREEPXD;
                        lrd->log.updatemap.nxd = 1;
                        lrd->log.updatemap.pxd = pxdlock->pxd;
                        lrd->backchain = lmLog(log, tblk, lrd, NULL);
jEVENT(0,("mapLog: xaddr:0x%x%08x xlen:0x%08x\n",
        addressPXD(&pxdlock->pxd), lengthPXD(&pxdlock->pxd)));
                }

                /* update bmap */
                tlck->flag |= tlckUPDATEMAP;
        }
}


/*
 *      txEA()
 *
 * function:    acquire maplock for EA/ACL extents or
 *              set COMMIT_INLINE flag;
 */
void
txEA(
        int32   tid,
        inode_t *ip,
        dxd_t   *oldea,
        dxd_t   *newea)
{
        tlock_t         *tlck = NULL;
        pxdlock_t       *maplock, *pxdlock;

        /*
         * format maplock for alloc of new EA extent
         */
        if (newea)
        {
                /* Since the newea could be a completely zeroed entry we need to
                 * check for the two flags which indicate we should actually
                 * commit new EA data
                 */
                if (newea->flag & DXD_EXTENT)
                {
                        tlck = txMaplock(tid, ip, tlckMAP);
                        maplock = (pxdlock_t *)&tlck->lock;
                        pxdlock = (pxdlock_t *)maplock;
                        pxdlock->flag = mlckALLOCPXD;
                        PXDaddress(&pxdlock->pxd, addressDXD(newea));
                        PXDlength(&pxdlock->pxd, lengthDXD(newea));
                        pxdlock++;
                        maplock->index = 1;
                        ip->i_flag |= IFSYNC; //PS 28092001
                }
                else if (newea->flag & DXD_INLINE)
                {
                        tlck = NULL;
                        ip->i_cflag |= COMMIT_INLINEEA;
                }
        }

        /*
         * format maplock for free of old EA extent
         */
        if (!(ip->i_cflag & COMMIT_NOLINK) &&
            oldea->flag & DXD_EXTENT)
        {
                if (tlck == NULL)
                {
                        tlck = txMaplock(tid, ip, tlckMAP);
                        maplock = (pxdlock_t *)&tlck->lock;
                        pxdlock = (pxdlock_t *)maplock;
                        maplock->index = 0;
                }
                pxdlock->flag = mlckFREEPXD;
                PXDaddress(&pxdlock->pxd, addressDXD(oldea));
                PXDlength(&pxdlock->pxd, lengthDXD(oldea));
                maplock->index++;
                ip->i_flag |= IFSYNC; //PS 28092001
        }
}


/*
 *      txForce()
 *
 * function: synchronously write pages locked by transaction
 *              after txLog() but before txUpdateMap();
 */
void txForce(
        tblock_t        *tblk)
{
        tlock_t         *tlck;
        int32           lid, next;
        jbuf_t          *bp;

        /*
         * reverse the order of transaction tlocks in
         * careful update order of address index pages
         * (right to left, bottom up)
         */
        tlck = &TxLock[tblk->next];
        lid = tlck->next;
        tlck->next = 0;
        while (lid)
        {
                tlck = &TxLock[lid];
                next = tlck->next;
                tlck->next = tblk->next;
                tblk->next = lid;
                lid = next;
        }

        /*
         * synchronously write the page, and
         * hold the page for txUpdateMap();
         */
        for (lid = tblk->next; lid > 0; lid = next)
        {
                tlck = &TxLock[lid];
                next = tlck->next;

                if ((bp = tlck->bp) != NULL &&
                    (tlck->type & tlckBTROOT) == 0)
                {
                        assert(bp->j_xflag & B_BUFFER);

                        if (tlck->flag & tlckWRITEPAGE)
                        {
                                tlck->flag &= ~tlckWRITEPAGE;

                                /* do not release page to freelist */
                                /* assert(bp->j_xflag & B_NOHOMEOK); */
                                assert(bp->j_nohomeok);
                                bmHold(bp);
                                bmWrite(bp);
                        }
                }
        }
}


/*
 *      txUpdateMap()
 *
 * function:    update persistent allocation map (and working map
 *              if appropriate);
 *
 * parameter:
 */
static void
txUpdateMap(
#ifdef _JFS_LAZYCOMMIT
        tblock_t        *tblk)					// D230860
#else
        tblock_t        *tblk,
        commit_t        *cd)
#endif
{
        inode_t         *ip;
        inode_t         *ipimap;
        inode_t         *ipbmap;
        int32           lid;
        tlock_t         *tlck;
        maplock_t       *maplock;
        pxdlock_t       pxdlock;
        uint32          maptype;
        int32           k, nlock;

#ifdef _JFS_LAZYCOMMIT
        ipimap = tblk->ipmnt->i_ipimap;				// D230860
#else
        ipimap = cd->ipimap;
#endif

        maptype = (tblk->xflag & COMMIT_PMAP) ? COMMIT_PMAP : COMMIT_PWMAP;

        /*
         *      update inode allocation map
         *
         * update allocation state in pmap and
         * update lsn of the pmap page;
         * update in-memory inode flag/state
         *
         * unlock mapper/write lock
         */
//BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	if (tblk->xflag & COMMIT_CREATE)
	{
		ip = tblk->ip;
		ASSERT(ip->i_cflag & COMMIT_NEW);
		ip->i_cflag &= ~COMMIT_NEW;
		diUpdatePMap(ipimap, ip->i_number, FALSE, tblk);
		ipimap->i_cflag |= DIRTY;
			/* update persistent block allocation map
			 * for the allocation of inode extent;
		 */
		pxdlock.flag = mlckALLOCPXD;
		pxdlock.pxd = ip->i_ixpxd;
		pxdlock.index = 1;
		txAllocPMap(ip, (maplock_t *)&pxdlock, tblk);
		jfs_rele(IP2VP(ip));
	}
	else if (tblk->xflag & COMMIT_DELETE)
	{
		ip = tblk->ip;
		assert((ip->i_nlink == 0) && !(ip->i_cflag & COMMIT_NOLINK));
		ip->i_cflag |= COMMIT_NOLINK;
		diUpdatePMap(ipimap, ip->i_number, TRUE, tblk);
		ipimap->i_cflag |= DIRTY;
	}
#else /* _JFS_LAZYCOMMIT */
//END D230860
        /*
         * scan inode list of transaction for object allocation/free:
         */
        for (k = 0; k < cd->nip; k++)
        {
                ip = cd->iplist[k];

                /*
                 * update persistent inode allocation map:
                 */
                /* allocate inode in pmap for object creation:
                 * inode has been allocated in wmap at allocation time;
                 */
                if (ip->i_cflag & COMMIT_NEW)
                {
                        ip->i_cflag &= ~COMMIT_NEW;
                        diUpdatePMap(ipimap, ip->i_number, FALSE, tblk);
                        cd->ipimap->i_cflag |= DIRTY;

                        /* update persistent block allocation map
                         * for the allocation of inode extent;
                         */
                        pxdlock.flag = mlckALLOCPXD;
                        pxdlock.pxd = ip->i_ixpxd;
                        pxdlock.index = 1;
                        txAllocPMap(ip, (maplock_t *)&pxdlock, tblk);
                }
                /* free inode in pmap for object deletion:
                 * inode will be freed in wmap at last reference release;
                 */
                else if (ip->i_nlink == 0 &&
                         !(ip->i_cflag & COMMIT_NOLINK)) /* ??? */
                {
                        ip->i_cflag |= COMMIT_NOLINK;
                        diUpdatePMap(ipimap, ip->i_number, TRUE, tblk);
                        cd->ipimap->i_cflag |= DIRTY;
                }
        }
#endif /* _JFS_LAZYCOMMIT */

        /*
         *      update block allocation map
         *
         * update allocation state in pmap (and wmap) and
         * update lsn of the pmap page;
         */
        /*
         * scan each tlock/page of transaction for block allocation/free:
         *
         * for each tlock/page of transaction, update map.
         *  ? are there tlock for pmap and pwmap at the same time ?
         */
        for (lid = tblk->next; lid > 0; lid = tlck->next)
        {
                tlck = &TxLock[lid];

                if ((tlck->flag & tlckUPDATEMAP) == 0)
                        continue;

                /*
                 * extent list:
                 * . in-line PXD list:
                 * . out-of-line XAD list:
                 */
                maplock = (maplock_t *)&tlck->lock;
                nlock = maplock->index;

                for (k = 0; k < nlock; k++, maplock++)
                {
                        /*
                         * allocate blocks in persistent map:
                         *
                         * blocks have been allocated from wmap at alloc time;
                         */
                        if (maplock->flag & mlckALLOC)
                        {
                                txAllocPMap(ipimap, maplock, tblk); // D230860
                        }
                        /*
                         * free blocks in persistent and working map:
                         * blocks will be freed in pmap and then in wmap;
                         *
                         * ? tblock specifies the PMAP/PWMAP based upon
                         * transaction
                         *
                         * free blocks in persistent map:
                         * blocks will be freed from wmap at last reference
                         * release of the object for regular files;
                         *
                         * Alway free blocks from both persistent & working
                         * maps for directories
                         */
                        else /* (maplock->flag & mlckFREE) */
                        {
                                if (tlck->type & tlckDTREE)
                                        txFreeMap(ipimap, maplock, // D230860
                                                  tblk, COMMIT_PWMAP);
                                else
                                        txFreeMap(ipimap, maplock, // D230860
						  tblk, maptype);
                        }
                }
        }
}


/*
 *      txAllocPMap()
 *
 * function: allocate from persistent map;
 *
 * parameter:
 *      ipbmap  -
 *      malock -
 *              xad list:
 *              pxd:
 *
 *      maptype -
 *              allocate from persistent map;
 *              free from persistent map;
 *              (e.g., tmp file - free from working map at releae
 *               of last reference);
 *              free from persistent and working map;
 *
 *      lsn     - log sequence number;
 */
static void txAllocPMap(
        inode_t         *ip,
        maplock_t       *maplock,
        tblock_t        *tblk)
{
        inode_t         *ipbmap = ip->i_ipmnt->i_ipbmap;
        xdlistlock_t    *xadlistlock;
        xad_t           *xad;
        int64           xaddr;
        int32           xlen;
        pxdlock_t       *pxdlock;
        xdlistlock_t    *pxdlistlock;
        pxd_t           *pxd;
        int32           n;

        /*
         * allocate from persistent map;
         */
        if (maplock->flag & mlckALLOCXADLIST)
        {
                xadlistlock = (xdlistlock_t *)maplock;
                xad = xadlistlock->xdlist;
                for (n = 0; n < xadlistlock->count; n++, xad++)
                {
                        if (xad->flag & (XAD_NEW|XAD_EXTENDED))
                        {
                                xaddr = addressXAD(xad);
                                xlen = lengthXAD(xad);
                                dbUpdatePMap(ipbmap, FALSE, xaddr, (int64)xlen, tblk);
                                xad->flag &= ~(XAD_NEW|XAD_EXTENDED);
jEVENT(0,("allocPMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                        }
                }
        }
        else if (maplock->flag & mlckALLOCPXD)
        {
                pxdlock = (pxdlock_t *)maplock;
                xaddr = addressPXD(&pxdlock->pxd);
                xlen = lengthPXD(&pxdlock->pxd);
                dbUpdatePMap(ipbmap, FALSE, xaddr, (int64)xlen, tblk);
jEVENT(0,("allocPMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
        }
        else /* (maplock->flag & mlckALLOCPXDLIST) */
        {
                pxdlistlock = (xdlistlock_t *)maplock;
                pxd = pxdlistlock->xdlist;
                for (n = 0; n < pxdlistlock->count; n++, pxd++)
                {
                        xaddr = addressPXD(pxd);
                        xlen = lengthPXD(pxd);
                        dbUpdatePMap(ipbmap, FALSE, xaddr, (int64)xlen, tblk);
jEVENT(0,("allocPMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                }
        }
}


/*
 *      txFreeMap()
 *
 * function:    free from persistent and/or working map;
 *
 * todo: optimization
 */
void txFreeMap(
        inode_t         *ip,
        maplock_t       *maplock,
        tblock_t        *tblk,
        uint32          maptype)
{
        inode_t         *ipbmap = ip->i_ipmnt->i_ipbmap;
        xdlistlock_t    *xadlistlock;
        xad_t           *xad;
        int64           xaddr;
        int32           xlen;
        pxdlock_t       *pxdlock;
        xdlistlock_t    *pxdlistlock;
        pxd_t           *pxd;
        int32           n;

jEVENT(0,("txFreeMap: tblk:0x%08x maplock:0x%08x maptype:0x%08x\n", tblk, maplock, maptype));

        /*
         * free from persistent map;
         */
        if (maptype == COMMIT_PMAP || maptype == COMMIT_PWMAP)
        {
                if (maplock->flag & mlckFREEXADLIST)
                {
                        xadlistlock = (xdlistlock_t *)maplock;
                        xad = xadlistlock->xdlist;
                        for (n = 0; n < xadlistlock->count; n++, xad++)
                        {
                                if (!(xad->flag & XAD_NEW))
                                {
                                        xaddr = addressXAD(xad);
                                        xlen = lengthXAD(xad);
                                        dbUpdatePMap(ipbmap, TRUE, xaddr, (int64)xlen, tblk);
jEVENT(0,("freePMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                                }
                        }
                }
                else if (maplock->flag & mlckFREEPXD)
                {
                        pxdlock = (pxdlock_t *)maplock;
                        xaddr = addressPXD(&pxdlock->pxd);
                        xlen = lengthPXD(&pxdlock->pxd);
                        dbUpdatePMap(ipbmap, TRUE, xaddr, (int64)xlen, tblk);
jEVENT(0,("freePMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                }
                else /* (maplock->flag & mlckALLOCPXDLIST) */
                {
                        pxdlistlock = (xdlistlock_t *)maplock;
                        pxd = pxdlistlock->xdlist;
                        for (n = 0; n < pxdlistlock->count; n++, pxd++)
                        {
                                xaddr = addressPXD(pxd);
                                xlen = lengthPXD(pxd);
                                dbUpdatePMap(ipbmap, TRUE, xaddr, (int64)xlen, tblk);
jEVENT(0,("freePMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                        }
                }
        }

        /*
         * free from working map;
         */
        if (maptype == COMMIT_PWMAP || maptype == COMMIT_WMAP)
        {
                if (maplock->flag & mlckFREEXADLIST)
                {
                        xadlistlock = (xdlistlock_t *)maplock;
                        xad = xadlistlock->xdlist;
                        for (n = 0; n < xadlistlock->count; n++, xad++)
                        {
                                xaddr = addressXAD(xad);
                                xlen = lengthXAD(xad);
                                dbFree(ip, xaddr, (int64)xlen);
                                xad->flag = 0;
jEVENT(0,("freeWMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                        }
                }
                else if (maplock->flag & mlckFREEPXD)
                {
                        pxdlock = (pxdlock_t *)maplock;
                        xaddr = addressPXD(&pxdlock->pxd);
                        xlen = lengthPXD(&pxdlock->pxd);
                        dbFree(ip, xaddr, (int64)xlen);
jEVENT(0,("freeWMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                }
                else /* (maplock->flag & mlckFREEPXDLIST) */
                {
                        pxdlistlock = (xdlistlock_t *)maplock;
                        pxd = pxdlistlock->xdlist;
                        for (n = 0; n < pxdlistlock->count; n++, pxd++)
                        {
                                xaddr = addressPXD(pxd);
                                xlen = lengthPXD(pxd);
                                dbFree(ip, xaddr, (int64)xlen);
jEVENT(0,("freeWMap: xaddr:0x%08x:0x%08x xlen:%d\n", xaddr, xlen));
                        }
                }
        }
}


/*
 *      txFreelock()
 *
 * function:    remove tlock from inode anonymous locklist
 */
void
txFreelock(inode_t      *ip)
{
        tlock_t *xtlck, *tlck;
        int32   xlid, lid;

        xtlck = (tlock_t *)&ip->i_atlhead;

        while (lid = xtlck->next)
        {
                tlck = &TxLock[lid];
                if (tlck->flag & tlckFREELOCK)
                {
                        xtlck->next = tlck->next;
                        TXLOCK_FREE(lid);
                }
                else
                {
                        xtlck = tlck;
                        xlid = lid;
                }
        }

        if (ip->i_atlhead)
                ip->i_atltail = xlid;
        else
                ip->i_atltail = 0;
}


#ifdef  _JFS_WIP
/*
 * NAME:        txFreeLock(lw)
 *
 * FUNCTION:    removes tlock from transaction locklist
 *              and insert on freelist.
 *
 * PARAMETERS:  lw - index of tlock.
 *
 * RETURN:     0
 *
 * SERIALIZATION:
 */
int32 txFreeLock(lid)
int32   lid;
{
        int32   tid, prev, next;

        /*
         * remove tlock from tid locklist
         */
        tid = TxLock[lid].tid;
        prev = -1;
        next = TxBlock[tid].next;
        while (next != lid)
        {
                prev = next;
                next = TxLock[next].tidnxt;
        }

        if (prev < 0)
                TxBlock[tid].next = TxLock[lid].next;
        else
                TxLock[prev].next = TxLock[lid].next;

        /*
         * insert tlock at head of freelist
         */
        TXLOCK_FREE(lid);

        return 0;
}
#endif  /* _JFS_WIP */


/*
 *      txAbort()
 *
 * function: abort tx before commit;
 *
 * frees line-locks and segment locks for all
 * segments in comdata structure.
 * Optionally sets state of file-system to FM_DIRTY in super-block.
 * log age of page-frames in memory for which caller has
 * are reset to 0 (to avoid logwarap).
 */
void txAbort(
        int32   tid,
        int32   dirty)
{
        int32   lid, next;
        jbuf_t  *bp;

jEVENT(1,("txAbort: tid:%d dirty:0x%08x\n", tid, dirty));

        /*
         * free tlocks of the transaction
         */
        for (lid = TxBlock[tid].next; lid > 0; lid = next)
        {
                next = TxLock[lid].next;

                bp = TxLock[lid].bp;

		if (bp)						// D233382
		{
			bp->j_lid = 0;

			/*
			 * reset lsn of page to avoid logwarap:
			 *
			 * (page may have been previously committed by another
			 * transaction(s) but has not been paged, i.e.,
			 * it may be on logsync list even though it has not
			 * been logged for the current tx.)
			 */
			if (bp->j_xflag & B_BUFFER && bp->j_lsn)
				bmLogSyncRelease(bp);
		}
                /* insert tlock at head of freelist */
                TXN_LOCK();
                TXLOCK_FREE(lid);
                TXN_UNLOCK();
        }

        /* caller will free the transaction block */

        TxBlock[tid].next = 0;

        /*
         * mark filesystem dirty
         */
        if (dirty)
                txFileSystemDirty(TxBlock[tid].ipmnt);

        return;
}


/*
 *      txAbortCommit()
 *
 * function: abort commit.
 *
 * frees tlocks of transaction; line-locks and segment locks for all
 * segments in comdata structure. frees malloc storage
 * sets state of file-system to FM_MDIRTY in super-block.
 * log age of page-frames in memory for which caller has
 * are reset to 0 (to avoid logwarap).
 */
int32 txAbortCommit(
        commit_t        *cd,
        int32           exval)
{
        int32   rc;
        int32   tid, lid, next;
        jbuf_t  *bp;

        assert(exval == EIO || exval == ENOMEM);
jEVENT(1,("txAbortCommit: cd:0x%08x\n", cd));

        /*
         * free tlocks of the transaction
         */
        tid = cd->tid;
        for (lid = TxBlock[tid].next; lid > 0; lid = next)
        {
                next = TxLock[lid].next;

                bp = TxLock[lid].bp;
                if (bp)                              /* 207090 */
                {                                    /* 207090 */
                   bp->j_lid = 0;

                   /*
                    * reset lsn of page to avoid logwarap;
                    */
                   if (bp->j_xflag & B_BUFFER)
                           bmLogSyncRelease(bp);
                }                                    /* 207090 */

                /* insert tlock at head of freelist */
                TXN_LOCK();
                TXLOCK_FREE(lid);
                TXN_UNLOCK();
        }

        TxBlock[tid].next = 0;

        /* free the transaction block */
//PS 28092001        txEnd(tid);

        /*
         * mark filesystem dirty
         */
        rc = txFileSystemDirty(cd->ipmnt);

        return rc;
}


//BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
/*
 *      jfs_lazycommit(void)
 *
 *	All transactions except those changing ipimap (COMMIT_FORCE) are
 *	processed by this routine.  This insures that the inode and block
 *	allocation maps are updated in order.  For synchronous transactions,
 *	let the user thread finish processing after txUpdateMap() is called.
 */
int32 jfs_lazycommit()
{
	int32		WorkDone;
	log_t		*log;					// D233382
	tblock_t	*tblk;

	if (LazyCommitRunning)
		return ERROR_INVALID_PARAMETER;

	LAZY_LOCK_ALLOC();
	LAZY_LOCK_INIT();
	event_init(&TxAnchor.Lazy_event);
	TxAnchor.unlock_queue = TxAnchor.unlock_tail = 0;
	TxAnchor.lazyQsize = TxAnchor.lazyQwait = 0;		// D233382
	LazyCommitRunning = 1;

	LAZY_LOCK();
	while (TRUE)
	{
		WorkDone = 0;

		while (tblk = TxAnchor.unlock_queue)
		{
			/*
			 * We can't get ahead of user thread.  Spinning is
			 * simpler than blocking/waking.  We shouldn't spin
			 * very long, since user thread shouldn't be blocking
			 * between lmGroupCommit & txEnd.
			 */
			WorkDone = 1;

			if (((tblk->flag & tblkGC_READY) == 0) &&  // D233382
			    ((tblk->flag & tblkGC_UNLOCKED) == 0))
			{
				/* Release and re-obtain LAZY_LOCK.  Thread
				 * processing interrupt may need it.
				 */
				LAZY_UNLOCK();			// D233382
jEVENT(0,("jfs_lazycommit: tblk 0x%x not unlocked\n", tblk));
				LAZY_LOCK();			// D233382
                            LAZY_SLEEP();               //PS21092003
				break;
			}

			/*
			 * Remove first transaction from queue
			 */
			TxAnchor.unlock_queue = tblk->cqnext;
			tblk->cqnext = 0;
			if (TxAnchor.unlock_tail == tblk)
				TxAnchor.unlock_tail = 0;

// BEGIN D233382
			TxAnchor.lazyQsize--;
			if (TxAnchor.lazyQwait &&
			    (TxAnchor.lazyQsize < LazyLWM))
			{
				TxAnchor.lazyQwait = 0;
				TXN_WAKEUP(&TxAnchor.freewait);
			}
// END D233382

			LAZY_UNLOCK();
jEVENT(0,("jfs_lazycommit: processing tblk 0x%x\n", tblk));

			txUpdateMap(tblk);

// BEGIN D233382
			log = (log_t *)tblk->ipmnt->i_iplog;

			XSPINLOCK_LOCK(&log->gclock);		// LOGGC_LOCK

			tblk->flag |= tblkGC_COMMITTED;

			if ((tblk->flag & tblkGC_READY) ||
			    (tblk->flag & tblkGC_LAZY))
				log->gcrtc--;

			if (tblk->flag & tblkGC_READY)
                        {
			   XEVENT_WAKEUP(&tblk->gcwait);	// LOGGC_WAKEUP
			   XSPINLOCK_UNLOCK(0, &log->gclock);	// LOGGC_UNLOCK
                        }
                     else
      			if (tblk->flag & tblkGC_LAZY)
			{
				tblk->flag &= ~tblkGC_LAZY;
			       XSPINLOCK_UNLOCK(0, &log->gclock);	//
				txUnlock(tblk, 0);
				txEnd(tblk-TxBlock); /* Convert back to tid */
			}
                     else
			       XSPINLOCK_UNLOCK(0, &log->gclock);	//
// END D233382

jEVENT(0,("jfs_lazycommit: done: tblk = 0x%x\n", tblk));
			LAZY_LOCK();
		}

/*		if (TxAnchor.sync_queue)
		{
			* XXXXXX *
		}
 */
		if (! WorkDone)
		{
jEVENT(0,("jfs_lazycommit: sleeping\n"));
			LAZY_SLEEP();
		}
	}
}

void	txLazyUnlock(
	tblock_t	*tblk)
{
	LAZY_LOCK();

	if (TxAnchor.unlock_tail)
		TxAnchor.unlock_tail->cqnext = tblk;
	else
		TxAnchor.unlock_queue = tblk;
	TxAnchor.unlock_tail = tblk;
	tblk->cqnext = 0;
	LAZY_WAKEUP();
	LAZY_UNLOCK();
}
#endif /* _JFS_LAZYCOMMIT */
//END D230860


/*
 *      txFileSystemDirty(ipmnt)
 *
 * Mark the file system dirty.
 */
int32 txFileSystemDirty(
        inode_t *ipmnt)
{
        int32   rc = 0;
        cbuf_t  *bp;
        struct superblock *sb;

        if (rc = readSuper(ipmnt, &bp))
                return rc;
        sb = (struct superblock *)(bp->b_bdata);
        sb->s_state = FM_DIRTY;

        /* write out superblock synchronously and invalidate page */
        rc = rawWrite(ipmnt, bp, 1);

        return rc;
}


#ifdef  _JFS_DEBUG
/*
 *      txSanity()
 */
void txSanity()
{
        tblock_t        *tblk;
        tlock_t         *tlck;

        int32   ntblk = 0, ntlck = 0, i;

        printf("\n - txnmgr sanity check -\n");

        /*
         *      tblock list
         */
        printf("\ntblock list: maxtid:%d\n", stattx.maxtid);
        for (i = 1; i <= stattx.maxtid; i++)
        {
                tblk = &TxBlock[i];
                ntblk++;
        }

        printf("ntblock:%d\n", ntblk);

        /*
         *      tlock list
         */
        printf("\ntlock list: maxlid:%d\n", stattx.maxlid);
        for (i = 1; i <= stattx.maxlid; i++)
        {
                tlck = &TxLock[i];
                if (tlck->bp)
                        ntblk++;
        }

        printf("ntlock:%d\n", ntlck);
}
#endif  /* _JFS_DEBUG */
