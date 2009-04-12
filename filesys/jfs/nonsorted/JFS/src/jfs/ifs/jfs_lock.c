/* $Id: jfs_lock.c,v 1.1.1.1 2003/05/21 13:37:48 pasha Exp $ */

static char *SCCSID = "@(#)1.15  9/13/99 15:04:52 src/jfs/ifs/jfs_lock.c, sysjfs, w45.fs32, fixbld";
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
 *	jfs_lock.c: OS/2 synchronization services
 *
 * OS/2 serialization: SMP non-preemptive kernel;
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <jfs_os2.h>
#include <jfs_types.h>
#include <jfs_lock.h>
#include <jfs_io.h>
#include <jfs_debug.h>

#define LOCK_LOCKED	0x00000001
#define LOCK_WANTED	0x00000002

#ifdef	_JFS_SLEEPLOCK
/* forward reference */
static void xIODone(XSPINLOCK_T *Lock);

/*
 *	thread-interrupt i/o synchronization and
 *	off-level interrupt handler support
 */
/* custom lock for serialization between thread and interrupt CS: */

/* jfs i/o SleepLock */
struct {
	unsigned	locked:  1;
	unsigned	wanted: 31;
} jfsIOLock = {0, 0};

/* tail pointer to jfs off-level iodone circular singly-linked list */
iobuf_t	*jfsIODoneList = NULL;


/*
 *	xIOLock()
 *
 * called by thread at entry of thread-interrupt CS;
 *	IOCACHE_LOCK() of data cache manager, 
 */
int32 xIOLock(XSPINLOCK_T	*SpinLock)
{
	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/* acquire SleepLock */
	while (jfsIOLock.locked) /* while (jfsIOLock & LOCK_LOCKED) */
	{
		jfsIOLock.wanted++; /* jfsIOLock |= LOCK_WANTED; */
		/* wait for SleepLock: release SpinLock */
		KernBlock((ULONG)&jfsIOLock,-1,
			  T_XSPINLOCK|BLOCK_UNINTERRUPTABLE,SpinLock,NULL);
		/* SpinLock reacquired */
	}

	jfsIOLock.locked = 1; /* jfsIOLock |= LOCK_LOCKED; */

	/* release SpinLock */
	KernReleaseSpinLock(SpinLock);	/* unlock_enable() */
}


/*
 *	xIOTryLock()
 *
 * called by thread at entry of thread-interrupt CS;
 *	IOCACHE_LOCK() of data cache manager, 
 */
int32 xIOTryLock(XSPINLOCK_T	*SpinLock)
{
	int32	rc = 0;		/* init NotLocked */

	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/* acquire SleepLock */
	if (jfsIOLock.locked == 0)
	{
		jfsIOLock.locked = 1; /* jfsIOLock |= LOCK_LOCKED; */
		rc = 1;			/* return Locked */
	}

	/* release SpinLock */
	KernReleaseSpinLock(SpinLock);	/* unlock_enable() */

	return rc;
}


/*
 *	xIOSleep()
 *
 * called by thread while in thread-interrupt CS:
 *	IOCACHE_SLEEP(&event); 
 */
int32 xIOSleep(event_t	*Event,
	XSPINLOCK_T	*SpinLock)
{
	int32	rc = 0;

	/* caller must beholding SleepLock */
	assert(jfsIOLock.locked); /* assert(jfsIOLock & LOCK_LOCKED); */

	*Event = EVENT_WAIT;

	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/*
	 * process buffers on iodonelist if any
	 */
	if (jfsIODoneList)
		xIODone(SpinLock);

	/* is specified event signalled by current thread itself 
	 * during processing iodonelist above ?
	 */
	while (*Event == EVENT_WAIT)
	{
		/* release SleepLock */
		jfsIOLock.locked = 0; /* jfsIOLock &= ~LOCK_LOCKED; */

		/* wake up waiting threads on SleepLock */
		if (jfsIOLock.wanted) /* if (jfsIOLock & LOCK_WANTED) */
		{
			jfsIOLock.wanted--; /* jfsIOLock &= ~LOCK_WANTED; */
			KernWakeup((ULONG)&jfsIOLock, WAKEUP_ONE, NULL, 0);
		}

		/* wait for event: release SpinLock */
		KernBlock((ULONG)Event, -1, T_XSPINLOCK|BLOCK_UNINTERRUPTABLE,
			  SpinLock, (ULONG *)&rc);
		/* SpinLock reacquired */

		/* acquire SleepLock */
		while (jfsIOLock.locked) /* while (jfsIOLock & LOCK_LOCKED) */
		{
			jfsIOLock.wanted++; /* jfsIOLock |= LOCK_WANTED; */
			/* wait for SleepLock: release SpinLock */
			KernBlock((ULONG)&jfsIOLock, -1,
				  T_XSPINLOCK|BLOCK_UNINTERRUPTABLE, SpinLock,
				  NULL);
			/* SpinLock reacquired */
		}

		jfsIOLock.locked = 1; /* jfsIOLock |= LOCK_LOCKED; */
	}

	/* release SpinLock */
	KernReleaseSpinLock(SpinLock);	/* unlock_enable() */

	return rc;
}


/*
 *	xIOWakeup()
 *
 * called by thread or interrupt handler while in thread-interrupt CS;
 *	IOCACHE_WAKEUP(&event);
 */
void xIOWakeup(event_t	*Event)
{
	assert(jfsIOLock.locked); /* assert(jfsIOLock & LOCK_LOCKED); */

	/* signal wakeup */
	*Event = EVENT_NULL;

	KernWakeup((ULONG)Event, 0, NULL, 0);
}

void xIOWakeup_rc(event_t	*Event,
	int32	rc)
{
	assert(jfsIOLock.locked); /* assert(jfsIOLock & LOCK_LOCKED); */

	/* signal wakeup */
	*Event = EVENT_NULL;

	KernWakeup((ULONG)Event, WAKEUP_DATA, NULL, rc);
}


/*
 *	xIOUnlock():
 *
 * called by thread at exit of thread-interrupt CS;
 *	IOCACHE_UNLOCK() of data cache manager, 
 */
void xIOUnlock(XSPINLOCK_T	*SpinLock)
{
	assert(jfsIOLock.locked); /* assert(jfsIOLock & LOCK_LOCKED); */

	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/*
	 * process buffers on off-level iodone list;
	 */
	if (jfsIODoneList)
		xIODone(SpinLock);

	/* release SleepLock */
	jfsIOLock.locked = 0; /* jfsIOLock &= ~LOCK_LOCKED; */

	/* wake up waiting threads on SleepLock */
	if (jfsIOLock.wanted) /* if (jfsIOLock & LOCK_WANTED) */
	{
		jfsIOLock.wanted--; /* jfsIOLock &= ~LOCK_WANTED; */

		/* release SpinLock */
		KernReleaseSpinLock(SpinLock);	/* unlock_enable() */

		KernWakeup((ULONG)&jfsIOLock, WAKEUP_ONE, NULL, 0);
	}
	else
		/* release SpinLock */
		KernReleaseSpinLock(SpinLock);	/* unlock_enable() */
}


/*
 *	xIODone(): iodone off-level service
 *
 * called by thread and interrupt handler callback; 
 *
 * on entry/exit, SpinLock(disabled + jfsIOPagerLock) and 
 * SleepLock(jfsIOLock) is being held; 
 *
 * while processing buffers, SpinLock is released (enabled + jfsIOPagerLock 
 * unlocked), and protected under SleepLock(jfsIOLock).
 */
static void xIODone(XSPINLOCK_T	*SpinLock)
{
	iobuf_t	*xIODoneList, *head, *tail;

	/*
	 * process buffers on iodone list
	 */
	while (jfsIODoneList)
	{
		/* transfer iodonelist to worklist */
		xIODoneList = jfsIODoneList;
		jfsIODoneList = NULL;

		/* release SpinLock */
		KernReleaseSpinLock(SpinLock);	/* unlock_enable() */

		/*
		 * process buffers on work list
		 *
		 * N.B. current state is euqivalent to XSPINLOCK_LOCK(),
		 * and b_iodone() should NOT acquire it again.
		 */
		while ((tail = xIODoneList) != NULL)
		{
			head = tail->b_next;	
			if (head == tail)
				xIODoneList = NULL;
			else
				tail->b_next = head->b_next;

			head->b_next = NULL;

			/*
			 * 	b_iodone:
			 *
			 * b_iodone may invoke soft_wakeup()
			 */
			(*head->b_iodone)(head);
		}

		/* acquire SpinLock */
		KernAcquireSpinLock(SpinLock);	/* disable_lock() */
	}
}


/*
 *	jfsIODone(): interrupt handler callback
 *
 * called by DD ISR as iodone callback;
 */
void APIENTRY jfsIODone(
	Req_List_Header *rlhp)	/* Request List Header */
{
        iobuf_t		*iobp;

        /* get pointer to the iobuf header struct */
        iobp = (iobuf_t *)((caddr_t)rlhp - RLH_OFFSET);

	if (rlhp->Lst_Status & RLH_Unrec_Error)
		iobp->b_flags |= B_ERROR;

	/* acquire SpinLock */
	KernAcquireSpinLock(&jfsIOPagerLock);	/* disable_lock() */

	/*
	 * insert the buffer at the tail of the off-level iodone list 
	 * so that the buffers are processed in the order of completion.
	 */
	if (jfsIODoneList == NULL)
		iobp->b_next = iobp;
	else
	{
		iobp->b_next = jfsIODoneList->b_next; 
		jfsIODoneList->b_next = iobp;
	}

	jfsIODoneList = iobp;

	/*
	 * schedule the buffer for off-level processing.
	 */
	/* try to acquire SleepLock */
	if (!(jfsIOLock.locked)) /* if (!(jfsIOLock & LOCK_LOCKED)) */
	{
		jfsIOLock.locked = 1; /* jfsIOLock |= LOCK_LOCKED; */

		/*
		 * process buffers on off-level iodone list;
		 */
		xIODone(&jfsIOPagerLock);

		/* release SleepLock */
		jfsIOLock.locked = 0; /* jfsIOLock &= ~LOCK_LOCKED; */

		/* wake up waiting threads on SleepLock */
		if (jfsIOLock.wanted) /* if (jfsIOLock & LOCK_WANTED) */
		{
			jfsIOLock.wanted--; /* jfsIOLock &= ~LOCK_WANTED; */

			/* release SpinLock */
			KernReleaseSpinLock(&jfsIOPagerLock);	/* unlock_enable() */

			KernWakeup((ULONG)&jfsIOLock, WAKEUP_ONE, NULL, 0);
		}
	}
	else
		/* release SpinLock */
		KernReleaseSpinLock(&jfsIOPagerLock);	/* unlock_enable() */
}
#endif	/* _JFS_SLEEPLOCK */


/*
 *	transforming lock: thread-thread spinlock
 *
 * function: custom lock structure;
 * normally operate under spinlock;
 * but if to be blocked (hopefully rarely), 
 * keep the lock and force other threads to block;
 * when resumed, wake up sleeping theads and operate under spinlock;
 */	

/*
 *	tLock()
 *
 */
void tLock(TSPINLOCK_T	*Lock)
{
	/* acquire SpinLock */
	KernAcquireSpinLock(&Lock->SpinLock);	/* disable_lock() */

	/* is SpinLock holder sleeping ? */
	while (Lock->SleepLock & LOCK_LOCKED)
	{
		Lock->SleepLock |= LOCK_WANTED;
		/* wait for SleepLock: release SpinLock */
		KernBlock((ULONG)(&Lock->SleepLock),-1,
			  T_SPINLOCK|BLOCK_UNINTERRUPTABLE,&Lock->SpinLock,
			  NULL);
		/* SpinLock reacquired */
	}
}


/*
 *	tXBlock()
 *
 * transform spinlock into blocking lock;
 */
void tXBlock(TSPINLOCK_T	*Lock)
{
	/* mark SpinLock holder may be blocked */
	Lock->SleepLock |= LOCK_LOCKED;

	/* release SpinLock */
	KernReleaseSpinLock(&Lock->SpinLock);	/* unlock_enable() */
}


/*
 *	tXSpin()
 *
 * transform blocking lock into spinlock;
 */
void tXSpin(TSPINLOCK_T	*Lock)
{
	/* acquire SpinLock */
	KernAcquireSpinLock(&Lock->SpinLock);	/* disable_lock() */

	/* mark SpinLock holder back in operation */
	Lock->SleepLock &= ~LOCK_LOCKED;

	/* wake up waiting threads on SleepLock */
	if (Lock->SleepLock & LOCK_WANTED)
	{
		Lock->SleepLock &= ~LOCK_WANTED;
		KernWakeup((ULONG)(&Lock->SleepLock), 0, NULL, 0);
	}
}


/*
 *	tSleep()
 *
 */
void tSleep(event_t	*Event,
	TSPINLOCK_T	*Lock)
{
	/* caller is holding Lock->SpinLock */

	*Event = EVENT_WAIT;

	/*
	 * wait for event wakeup
	 */
	while (*Event == EVENT_WAIT)
	{
		/* wait for event: release SpinLock */
		KernBlock((ULONG)Event, -1, T_SPINLOCK|BLOCK_UNINTERRUPTABLE,
			  &Lock->SpinLock, 0);
		/* SpinLock reacquired */

		/* is SpinLock holder sleeping  ? */
		while (Lock->SleepLock & LOCK_LOCKED)
		{
			Lock->SleepLock |= LOCK_WANTED;
			/* wait for SleepLock: release SpinLock */
			KernBlock((ULONG)(&Lock->SleepLock), -1,
				  T_SPINLOCK|BLOCK_UNINTERRUPTABLE,
				  &Lock->SpinLock, NULL);
			/* SpinLock reacquired */
		}
	}
}


/*
 *	tWakeup()
 *
 */
void tWakeup(event_t	*Event)
{
	/* assert(Lock->SleepLock & LOCK_LOCKED); */

	/* signal wakeup */
	*Event = EVENT_NULL;

	KernWakeup((ULONG)Event, 0, NULL, 0);
}


/*
 *	tUnlock():
 */
void tUnlock(TSPINLOCK_T	*Lock)
{
	/* release SpinLock */
	KernReleaseSpinLock(&Lock->SpinLock);	/* unlock_enable() */
}


#ifdef	_JFS_SLEEPLOCK
/*
 *	Roll Your Own synchronization services based on spinlock. 
 *
 * function: custom lock structure;
 */	

/*
 *	xLock()
 *
 */
int32 xLock(SPINLOCK_T	*SpinLock,
	int32	*SleepLock)
{
	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/* acquire SleepLock */
	while (*SleepLock & LOCK_LOCKED)
	{
		*SleepLock |= LOCK_WANTED;
		/* wait for SleepLock: release SpinLock */
		KernBlock((ULONG)SleepLock,-1,T_SPINLOCK|BLOCK_UNINTERRUPTABLE,
			  SpinLock,NULL);
		/* SpinLock reacquired */
	}

	*SleepLock |= LOCK_LOCKED;

	/* release SpinLock */
	KernReleaseSpinLock(SpinLock);	/* unlock_enable() */
}


/*
 *	xSleep()
 *
 */
int32 xSleep(event_t	*Event,
	SPINLOCK_T	*SpinLock,
	int32		*SleepLock)
{
	/* caller must beholding SleepLock */
	assert(*SleepLock & LOCK_LOCKED);

	*Event = EVENT_WAIT;

	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/*
	 * wait for event wakeup
	 */
	while (*Event == EVENT_WAIT)
	{
		/* release SleepLock */
		*SleepLock &= ~LOCK_LOCKED;

		/* wake up waiting threads on SleepLock */
		if (*SleepLock & LOCK_WANTED)
		{
			*SleepLock &= ~LOCK_WANTED;
			KernWakeup((ULONG)SleepLock, 0, NULL, 0);
		}

		/* wait for event */
		KernBlock((ULONG)Event, -1, T_SPINLOCK|BLOCK_UNINTERRUPTABLE,
			  SpinLock, 0);

		/* acquire SleepLock */
		while (*SleepLock & LOCK_LOCKED)
		{
			*SleepLock |= LOCK_WANTED;
			/* wait for SleepLock: release SpinLock */
			KernBlock((ULONG)SleepLock, -1,
				  T_SPINLOCK|BLOCK_UNINTERRUPTABLE, SpinLock,
				  NULL);
			/* SpinLock reacquired */
		}

		*SleepLock |= LOCK_LOCKED;
	}

	/* release SpinLock */
	KernReleaseSpinLock(SpinLock);	/* unlock_enable() */
}


/*
 *	xWakeup()
 *
 */
void xWakeup(event_t	*Event)
{
	/* assert(SleepLock & LOCK_LOCKED); */

	/* signal wakeup */
	*Event = EVENT_NULL;

	KernWakeup((ULONG)Event, 0, NULL, 0);
}

void xWakeup_rc(event_t	*Event,
	int32	rc)
{
	/* assert(SleepLock & LOCK_LOCKED); */

	/* signal wakeup */
	*Event = EVENT_NULL;

	KernWakeup((ULONG)Event, WAKEUP_DATA, NULL, rc);
}


/*
 *	xUnlock():
 *
 */
void xUnlock(SPINLOCK_T	*SpinLock,
	int32	*SleepLock)
{
	/* assert(SleepLock & LOCK_LOCKED); */

	/* acquire SpinLock */
	KernAcquireSpinLock(SpinLock);	/* disable_lock() */

	/* release SleepLock */
	*SleepLock &= ~LOCK_LOCKED;

	/* wake up waiting threads on SleepLock */
	if (*SleepLock & LOCK_WANTED)
	{
		*SleepLock &= ~LOCK_WANTED;
		KernWakeup((ULONG)SleepLock, 0, NULL, 0);
	}

	/* release SpinLock */
	KernReleaseSpinLock(SpinLock);	/* unlock_enable() */
}
#endif	/* _JFS_SLEEPLOCK */
