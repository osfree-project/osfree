/* $Id: jfs_dio.h,v 1.1 2000/04/21 10:57:56 ktk Exp $ */

/* static char *SCCSID = "@(#)1.10  2/22/99 10:28:41 src/jfs/common/include/jfs_dio.h, sysjfs, w45.fs32, 990417.1";
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

#ifndef H_JFS_DIO
#define H_JFS_DIO

/*
 *	dioelm_t
 */
struct dioelement 
{
	struct dioelement *	de_abnrnext;	/* 4: ABNR list next pointer */
	int32			de_flag;	/* 4: flags */
	int64			de_plen;	/* 8: byte count of I/O */
	int64			de_paddr;	/* 8: physical block of I/O */
	int64			de_offset;	/* 8: byte offset of file */
#ifdef _JFS_OS2
	KernVMLock_t		de_lockh;	/* 12: Lock to pin user buf */
	struct directio		*de_dp;		/* 4: My dp for dioIODone() */
#endif	/* _JFS_OS2 */
};		/* 52 */

/*
 * de_flag values
 */
#define	DIO_NO_UNLOCK	0x0001	/* buffer should not be unlocked after I/O */

typedef	struct dioelement dioelm_t;

#define	DIO_READ	0x00000001
#define	DIO_WRITE	0x00000002
#define	DIO_ABNR	0x00000004

#define	DIO_MAXIOS	32

/*
 *	dio_t
 */
struct diovector
{
	struct diovector *	dv_next;	/* 4: more I/Os for this buf */
	int32  			dv_nios;	/* 4: # I/Os for this vector */
	struct dioelement	dv_ios[DIO_MAXIOS];	/* 52*32: */
};		/* 1672 */
typedef	struct diovector diovec_t;

struct directio
{
	struct inode *		dio_ip;		/* 4: */
	int32			dio_iocnt;	/* 4: total in process I/Os */
	struct directio *	dio_diofree;	/* 4: */
	struct diovector *	dio_iovlist;	/* 4: allocated vectors */
	struct diovector * 	dio_freeiov;	/* 4: free vectors */
	int32			dio_error;	/* 4: error rc */
	int64			dio_erroff;	/* 8: first error offset */
	struct dioelement *	dio_abnrlist;	/* 4: ABNR list header */
	event_t			dio_iowait;	/* 4: */
	struct diovector 	dio_firstiov;	/* 1672: */
};		/* 1712 */
typedef	struct directio dio_t;

/*
 *	diomgr
 */
struct diomgr
{
	MUTEXLOCK_T		cachelock;   /* cache manager lock 	    */
	XSPINLOCK_T		iocachelock; /* cache manager i/o lock      */
	iobuf_t * 		freebufs;
	event_t			bufwait;
	struct directio * 	diofree;
	struct diovector * 	iovfree;
};


/*
 *	dio synchronization
 */
/* dio manager lock: thread-thread */
#define DIO_LOCK_ALLOC()\
	MUTEXLOCK_ALLOC(&diomgr.cachelock,LOCK_ALLOC_PAGED,JFS_CACHE_LOCK_CLASS,0)
#define DIO_LOCK_INIT()    MUTEXLOCK_INIT(&diomgr.cachelock)
#define DIO_LOCK()         MUTEXLOCK_LOCK(&diomgr.cachelock)
#define DIO_UNLOCK()       MUTEXLOCK_UNLOCK(&diomgr.cachelock)

/* dio i/o lock: thread-interrupt */
#define DIO_IOLOCK_ALLOC() \
	XSPINLOCK_ALLOC((void *)&diomgr.iocachelock, LOCK_ALLOC_PIN, JFS_IOCACHE_LOCK_CLASS, -1)
#define DIO_IOLOCK_INIT()	XSPINLOCK_INIT(&diomgr.iocachelock)
#define DIO_IOLOCK()		XSPINLOCK_LOCK(&diomgr.iocachelock)
#define DIO_IOUNLOCK(xipl)	XSPINLOCK_UNLOCK((xipl), &diomgr.iocachelock)
#define DIO_NOLOCK(xipl)	XSPINLOCK_NOLOCK((xipl), &diomgr.iocachelock)
#define DIO_WAKEUP(event)	XEVENT_WAKEUP((event))

#ifdef	_JFS_OS2
#define	DIO_SLEEP(event,flag)\
	XEVENT_SLEEP((event), &diomgr.iocachelock, T_XSPINLOCK|flag)
#endif	/* _JFS_OS2 */


/*
 *	external declarations
 */
int32 dioInit(int32 nbufs);

int32 dioRead(dio_t	*dp,
	      int64	paddr,
	      int64	plen,
	      int64	offset,
	      caddr_t	dest);

int32 dioStart(inode_t	*ip,
	       dio_t	**dpp);

int32 dioEnd(dio_t	*dp,
	     int64	*erroff);

int32 dioWrite(dio_t	*dp,
	       int64	paddr,
	       int64	plen,
	       int64	offset,
	       caddr_t	src,
	       boolean_t	abnr);

void dioIODone(iobuf_t *bp);

#endif	/* H_JFS_DIO */
