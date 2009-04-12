/* $Id: jfs_lock.h,v 1.1.1.1 2003/05/21 13:35:52 pasha Exp $ */

/* static char *SCCSID = "@(#)1.24  9/13/99 11:15:13 src/jfs/common/include/jfs_lock.h, sysjfs, w45.fs32, fixbld";
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
#ifndef _H_JFS_LOCK
#define _H_JFS_LOCK

/*
 *	jfs_lock.h
 *
 * JFS lock definition for globally referenced locks
 */

/*
 * Change History :
 *
 */

#include "mmph.h"						// D230860

/*
 *	event
 */
typedef	int32		event_t;

#define event_init(event)	*(event) = EVENT_NULL

#ifdef	_JFS_OS2
/*
 *	OS2
 *	---
 */
#ifdef	_JFS_SLEEPLOCK
extern int32 xIOLock(SpinLock_t *Lock);
extern void xIOUnlock(SpinLock_t *Lock);
extern int32 xIOSleep(event_t *Event, SpinLock_t *Lock);
extern void xIOWakeup(event_t *Event);
extern void xIOWakeup_rc(event_t *Event, int32 rc);

/* extern void jfsIODone(Req_List_Header *rlhp);	*/
#endif	/* _JFS_SLEEPLOCK */

extern void tLock(TSPINLOCK_T	*Lock);
extern void tXBlock(TSPINLOCK_T	*Lock);
extern void tXSpin(TSPINLOCK_T	*Lock);
extern void tSleep(event_t *Event, TSPINLOCK_T *Lock);
extern void tWakeup(event_t *Event);
extern void tUnlock(TSPINLOCK_T	*Lock);

#ifdef	_JFS_SLEEPLOCK
extern int32 xLock(SpinLock_t *SpinLock, int32 *SleepLock);
extern void xUnlock(SpinLock_t *SpinLock, int32 *SleepLock);
extern int32 xSleep(event_t *Event, SpinLock_t *SpinLock, int32 *SleepLock);
extern void xWakeup(event_t *Event);
extern void xWakeup_rc(event_t *Event, int32 rc);
#endif	/* _JFS_SLEEPLOCK */

/*
 *	lock
 */
/* spin lock: thread-interrupt/interrupt-interrupt */
#define	XSPINLOCK_T	SpinLock_t	/* uint32 */
#define XSPINLOCK_ALLOC(Lock,Flag,Class,Occurrence)\
	KernAllocSpinLock(Lock)
#define XSPINLOCK_FREE(Lock)		KernFreeSpinLock(Lock)
#define XSPINLOCK_INIT(Lock)
#define XSPINLOCK_LOCK(Lock)		(KernAcquireSpinLock(Lock),0)
#define XSPINLOCK_UNLOCK(xipl,Lock)	KernReleaseSpinLock(Lock)
#define XSPINLOCK_NOLOCK(xipl,Lock)

/* spin lock: thread-thread */
#define	SPINLOCK_T	SpinLock_t	/* uint32 */
#define SPINLOCK_ALLOC(Lock,Flag,Class,Occurrence)\
	KernAllocSpinLock(Lock)
#define SPINLOCK_FREE(Lock)		KernFreeSpinLock(Lock)
#define SPINLOCK_INIT(Lock)
#define SPINLOCK_LOCK(Lock)		KernAcquireSpinLock(Lock)
#define SPINLOCK_UNLOCK(Lock)		KernReleaseSpinLock(Lock)

/* transforming spin lock: thread-thread */
/*	Definition moved to jfs_types.h.
typedef struct {
	SpinLock_t	SpinLock;
	int32		SleepLock;
} TSPINLOCK_T;
*/

#define TSPINLOCK_ALLOC(Lock,Flag,Class,Occurrence)\
	KernAllocSpinLock(&(Lock)->SpinLock)
#define TSPINLOCK_FREE(Lock)		KernFreeSpinLock(&(Lock)->SpinLock)
#define TSPINLOCK_INIT(Lock)		(Lock)->SleepLock = 0
#define TSPINLOCK_LOCK(Lock)		tLock(Lock)
#define TSPINLOCK_UNLOCK(Lock)		tUnlock(Lock)
#define TSPINLOCK_XBLOCK(Lock)		tXBlock(Lock)
#define TSPINLOCK_XSPIN(Lock)		tXSpin(Lock)

/* mutex lock: thread-thread */
#define	MUTEXLOCK_T	MutexLock_t
#define MUTEXLOCK_ALLOC(Lock,Flag,Class,Occurrence)\
	KernAllocMutexLock(Lock)
#define MUTEXLOCK_FREE(Lock)		KernFreeMutexLock(Lock)
#define MUTEXLOCK_INIT(Lock)
#define MUTEXLOCK_LOCK(Lock)		KernRequestExclusiveMutex(Lock)
#define MUTEXLOCK_LOCK_TRY(Lock)	KernTryRequestExclusiveMutex(Lock)
#define	MUTEXLOCK_UNLOCK(Lock)		KernReleaseExclusiveMutex(Lock)
#define MUTEXLOCK_NOLOCK(Lock)

/* readers/writer lock: thread-thread */
#define	RDWRLOCK_T	MutexLock_t
#define RDWRLOCK_ALLOC(Lock,Flag,Class,Occurrence)\
	KernAllocMutexLock(Lock)
#define RDWRLOCK_FREE(Lock)		KernFreeMutexLock(Lock)
#define RDWRLOCK_INIT(Lock)
#define READ_LOCK(Lock)			KernRequestSharedMutex(Lock)
#define	READ_UNLOCK(Lock)		KernReleaseSharedMutex(Lock)
#define WRITE_LOCK(Lock)		KernRequestExclusiveMutex(Lock)
#define WRITE_LOCK_TRY(Lock)		KernTryRequestExclusiveMutex(Lock)
#define	WRITE_UNLOCK(Lock)		KernReleaseExclusiveMutex(Lock)

/*
 *	lock class
 */
#define	LOCK_ALLOC_PIN		0
#define	LOCK_ALLOC_PAGED	0

#define	JFS_LOCK_CLASS		0	/* jfs lock */
#define	JFS_SYNC_LOCK_CLASS	0	/* sync */
#define	JFS_RENAME_LOCK_CLASS	0	/* rename */
#define	JFS_EA_LOCK_CLASS	0	/* EA */
#define	JFS_CACHE_LOCK_CLASS	0	/* cachemgr lock */
#define	JFS_IRDWR_LOCK_CLASS	0	/* read/write */
#define	JFS_INODE_LOCK_CLASS	0	/* inode */
#define	JFS_FLOCK_LOCK_CLASS	0	/* file range */
#define	JFS_NCACHE_LOCK_CLASS	0	/* dnlc */
#define	JFS_ICACHE_LOCK_CLASS	0	/* inode cache */
#define	JFS_IOCACHE_LOCK_CLASS	0	/* i/o buffer cache */
#define	JFS_BMAP_LOCK_CLASS	0	/* bmap */
#define	JFS_IAGFREE_LOCK_CLASS	0	/* imap */
#define	JFS_AG_LOCK_CLASS	0	/* imap */
#define	JFS_TXN_LOCK_CLASS	0	/* transaction */
#define	JFS_LOGTBL_LOCK_CLASS	0	/* log table */
#define	JFS_LOG_LOCK_CLASS	0	/* log */
#define	JFS_LOGSYNC_LOCK_CLASS	0	/* synclist */
#define	JFS_GC_LOCK_CLASS	0	/* group commit */
#define	JFS_LCACHE_LOCK_CLASS	0	/* log buffer cache */
#define JFS_DASD_LOCK_CLASS	0	/* direct access (raw) I/0 */
#define JFS_POOL_LOCK_CLASS	0	/* allocation pools */
#define JFS_UNIDD_LOCK_CLASS	0	/* unicode dd */
#define	JFS_PAGING_LOCK_CLASS	0	/* paging spinlock */

/*
 *	Event
 *
 * APIRET APIENTRY KernBlock(uint32 EventID, uint32 Timeout, 
 *			     uint32 Flags,
 *			     void *pLock, 
 *			     uint32 *pData);
 * . thread is inserted on the event list hashed by EventID;
 * . if pLock is not NULL, its lock type is specified by Flags, and
 *   it is released before blocked, and reacquired when resumed
 *   (unless T_NORELOCK option is specified);
 *
 * APIRET APIENTRY KernWakeup(uint32 EventID, uint32 Flags, 
 *			      uint32 *pNumThreads, uint32 Data);
 * . there's no event anchor to test whether there are any waiting threads;
 * . Flags specifies the wakeup group and whether to return result;
 */
#define EVENT_NULL	0
#define	EVENT_WAIT	-1

/* timeout */
#define	TIMEOUT_FOREVER		-1

/* lock type flag for KernBlock() */
#define	T_XSPINLOCK		BLOCK_SPINLOCK
#define	T_SPINLOCK		BLOCK_SPINLOCK
#define	T_TSPINLOCK		BLOCK_SPINLOCK
#define	T_MUTEXLOCK		BLOCK_EXCLUSIVE_MUTEX
#define	T_READLOCK		BLOCK_SHARED_MUTEX
#define	T_WRITELOCK		BLOCK_EXCLUSIVE_MUTEX

#define	T_NORELOCK		BLOCK_NOACQUIRE

/* thread-interrupt */

/* It will be extremely rare that KernBlock will return with *(Event) not
 * set to EVENT_NULL.  Therefore, the case where we will block only once must
 * be optimal.
 */
#define	XEVENT_SLEEP(Event,Lock,Flag)\
{\
	ASSERT((Flag) & T_XSPINLOCK);\
	(*(Event))++;\
	while (1)\
	{\
		KernBlock((ULONG)(Event),TIMEOUT_FOREVER,\
			  (Flag)|BLOCK_UNINTERRUPTABLE,Lock,NULL);\
		if (*(Event) == EVENT_NULL)\
			break;\
		if ((Flag) & T_NORELOCK)\
		{\
			XSPINLOCK_LOCK(Lock);\
			if (*(Event) == EVENT_NULL)\
			{\
				XSPINLOCK_UNLOCK(0,Lock);\
				break;\
			}\
		}\
	}\
}
#define	XEVENT_SLEEP_RC(Event,Lock,Flag,rc)\
{\
	ASSERT((Flag) & T_XSPINLOCK);\
	(*(Event))++;\
	while (1)\
	{\
		KernBlock((ULONG)(Event),TIMEOUT_FOREVER,\
			  (Flag)|BLOCK_UNINTERRUPTABLE,Lock,(ULONG *)&(rc));\
		if (*(Event) == EVENT_NULL)\
			break;\
		if ((Flag) & T_NORELOCK)\
		{\
			XSPINLOCK_LOCK(Lock);\
			if (*(Event) == EVENT_NULL)\
			{\
				XSPINLOCK_UNLOCK(0,Lock);\
				break;\
			}\
		}\
	}\
}
#define	XEVENT_WAKEUP(Event)\
{\
	(*(Event)) = EVENT_NULL;\
	KernWakeup((ULONG)(Event),0,NULL,0);\
}
#define	XEVENT_WAKEUP_RC(Event,rc)\
{\
	(*(Event)) = EVENT_NULL;\
	KernWakeup((ULONG)(Event),WAKEUP_DATA,NULL,rc);\
}

/* thread-thread : TSPINLOCK_T */
#define TEVENT_SLEEP(Event,Lock,Flag)	tSleep(Event,Lock)
#define TEVENT_WAKEUP(Event)		tWakeup(Event)

/* thread-thread */
#define	EVENT_SLEEP(Event,Lock,Flag)\
{\
	ASSERT((Flag) & T_MUTEXLOCK);\
	(*(Event))++;\
	while (1)\
	{\
		KernBlock((ULONG)(Event),TIMEOUT_FOREVER,\
			  (Flag)|BLOCK_UNINTERRUPTABLE,Lock,NULL);\
		if (*(Event) == EVENT_NULL)\
			break;\
		if ((Flag) & T_NORELOCK)\
		{\
			MUTEXLOCK_LOCK(Lock);\
			if (*(Event) == EVENT_NULL)\
			{\
				MUTEXLOCK_UNLOCK(Lock);\
				break;\
			}\
		}\
	}\
}
/* Retry logic must exist outside the macro to protect from spurrious signals
 */
#define	EVENT_SLEEP_ONCE(Event,Lock,Flag)\
{\
	ASSERT((Flag) & T_MUTEXLOCK);\
	(*(Event))++;\
	KernBlock((ULONG)(Event),TIMEOUT_FOREVER,\
		  (Flag)|BLOCK_UNINTERRUPTABLE,Lock,NULL);\
}
#define	EVENT_SLEEP_RC(Event,Lock,Flag,rc)\
{\
	ASSERT((Flag) & T_MUTEXLOCK);\
	(*(Event))++;\
	while (1)\
	{\
		KernBlock((ULONG)(Event),TIMEOUT_FOREVER,\
			  (Flag)|BLOCK_UNINTERRUPTABLE,Lock,(ULONG *)&(rc));\
		if (*(Event) == EVENT_NULL)\
			break;\
		if ((Flag) & T_NORELOCK)\
		{\
			MUTEXLOCK_LOCK(Lock);\
			if (*(Event) == EVENT_NULL)\
			{\
				MUTEXLOCK_UNLOCK(Lock);\
				break;\
			}\
		}\
	}\
}
#define	EVENT_WAKEUP(Event)\
{\
	(*(Event)) = EVENT_NULL;\
	KernWakeup((ULONG)(Event),0,NULL,0);\
}
#define	EVENT_WAKEUP_RC(Event,rc)\
{\
	(*(Event)) = EVENT_NULL;\
	KernWakeup((ULONG)(Event),WAKEUP_DATA,NULL,rc);\
}

/*
 *	inode lock (per inode)
 */
#define INODE_LOCK(ip)		SPINLOCK_LOCK(&((ip)->i_nodelock))
#define INODE_UNLOCK(ip)	SPINLOCK_UNLOCK(&((ip)->i_nodelock))

/*
 *	filelock (per inode)
 */
#define	FILELOCK_LOCK(ip)	TSPINLOCK_LOCK(&((ip)->i_gnode.gn_byte_lock))
#define	FILELOCK_UNLOCK(ip)	TSPINLOCK_UNLOCK(&((ip)->i_gnode.gn_byte_lock))
#endif	/* _JFS_OS2 */


/*
 *	file system global lock
 *	-----------------------
 */
extern MUTEXLOCK_T	jfsLock;
#define JFS_LOCK()		MUTEXLOCK_LOCK(&jfsLock)
#define JFS_UNLOCK()		MUTEXLOCK_UNLOCK(&jfsLock)

/*
 *	inode cache lock (per system)
 */
extern TSPINLOCK_T	jfsICacheLock;
#define ICACHE_LOCK()		TSPINLOCK_LOCK(&jfsICacheLock)
#define ICACHE_UNLOCK()		TSPINLOCK_UNLOCK(&jfsICacheLock)

/*
 *	inode read/write lock (per inode)
 */
// BEGIN D230860
#define IREAD_LOCK(ip)	\
{			\
	MMPHPreIREAD_LOCK(ip)		\
	READ_LOCK(&((ip)->i_rdwrlock));	\
	MMPHPostIREAD_LOCK()		\
}
#define IREAD_UNLOCK(ip)	\
{				\
	MMPHPreIREAD_UNLOCK(ip)			\
	READ_UNLOCK(&((ip)->i_rdwrlock));	\
}
// END D230860

#define IWRITE_LOCK(ip)	\
{\
	MMPHPreIWRITE_LOCK(ip)		\
	WRITE_LOCK(&((ip)->i_rdwrlock));\
	if (ip->i_xlock & FSXLOCK)\
	{\
		do {\
			printf("fsxlock:%d\n", ip->i_number);\
			ip->i_xlock |= FSXWANT;\
			EVENT_SLEEP(&(ip)->i_fsevent, &(ip)->i_rdwrlock, T_WRITELOCK);\
		} while (ip->i_xlock & FSXLOCK);\
	}\
	MMPHPostIWRITE_LOCK()		\
}

#define IWRITE_LOCK_TRY(ip)	WRITE_LOCK_TRY(&((ip)->i_rdwrlock))
// BEGIN D230860
#define IWRITE_UNLOCK(ip)	\
{				\
	MMPHPreIWRITE_UNLOCK(ip)		\
	WRITE_UNLOCK(&((ip)->i_rdwrlock));	\
}
// END D230860
#define IWRITE_LOCK_LIST	iwritelocklist

/*
 *      aggregate-wide rename lock
 */
#define RENAME_LOCK(ipmnt)      MUTEXLOCK_LOCK(&((ipmnt)->i_renamelock))
#define RENAME_UNLOCK(ipmnt)    MUTEXLOCK_UNLOCK(&((ipmnt)->i_renamelock))

/*
 *	i/o cache lock
 */
extern XSPINLOCK_T	jfsIOCacheLock;

#endif /* _H_JFS_LOCK */
