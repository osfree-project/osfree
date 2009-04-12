/* $Id: jfs_txnmgr.h,v 1.1 2000/04/21 10:57:59 ktk Exp $ */

/* static char *SCCSID = "@(#)1.12  11/1/99 12:25:03 src/jfs/common/include/jfs_txnmgr.h, sysjfs, w45.fs32, fixbld";
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
 *
 */

#ifndef _H_JFS_TXNMGR
#define _H_JFS_TXNMGR
/*
 *	jfs_txnmgr.h: transaction manager
 */

#include "jfs_xtree.h"
#include "jfs_logmgr.h"
#include "jfs_bufmgr.h"

/*
 *	transaction block
 */
typedef struct tblock {
	/*
	 * tblock_t and jbuf_t common area: struct logsyncblk
	 *
	 * the following 3 fields are the same as struct logsyncblk
	 * which is common to tblock and jbuf to form logsynclist
	 */
	uint16	xflag;		/* 2: tx commit type */
	uint16	flag;		/* 2: tx commit state */
	int32	lsn;		/* 4: recovery lsn */
	CDLL_ENTRY(logsyncblk)	logsynclist;	/* 8: logsynclist link */
				/* (16) */

	/* lock management */
	inode_t	*ipmnt;		/* 4: mount inode */
	uint16	next;		/* 2: index first tlock of tid or
				 *    of next tblock on freelist
				 */
	uint16	locker;		/* 2: tid of holder of tlock tid wants */
	event_t	waitor;		/* 4: eventlist of tids waiting on tid */

	/* log management */
	uint32	logtid;		/* 4: log transaction id */
				/* (32) */

	/* commit management */
	struct tblock	*cqnext;/* 4: commit queue link */
	int32	clsn;		/* 4: commit lsn */
	struct lbuf	*bp;	/* 4: */
	int32	pn;		/* 4: commit record log page number */
	int32	eor;		/* 4: commit record eor */
	event_t	gcwait;		/* 4: group commit event list:
			 	 *    ready transactions wait on this event 
			 	 *    for group commit completion.
			 	 */
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	struct inode	*ip;	/* 4: inode being created or deleted */
#else
	struct dasd_usage *du;	/* 4: DASD usage structure */
#endif
	int32	rsrvd;		/* 4: */
// END D230860
} tblock_t;			/* (64) */

extern struct tblock	*TxBlock;/* transaction block table */ 

/* commit flags: tblk->xflag */
#define	COMMIT_SYNC	0x0001	/* synchronous commit */
#define	COMMIT_FORCE	0x0002	/* force pageout at end of commit */
#define	COMMIT_FLUSH	0x0004	/* init flush at end of commit */
#define COMMIT_MAP	0x00f0
#define	COMMIT_PMAP	0x0010	/* update pmap */
#define	COMMIT_WMAP	0x0020	/* update wmap */
#define	COMMIT_PWMAP	0x0040	/* update pwmap */
#define	COMMIT_FREE	0x0f00
#define	COMMIT_DELETE	0x0100	/* inode delete			D230860 */	
#define	COMMIT_TRUNCATE	0x0200	/* file truncation */
#define	COMMIT_CREATE	0x0400	/* inode create 		D230860 */
#define	COMMIT_LAZY	0x0800	/* lazy commit			D230860 */

/* group commit flags tblk->flag: see jfs_logmgr.h */

/*
 *	transaction lock
 */
typedef struct tlock {
	uint16	next;		/* 2: index next lockword on tid locklist
				 *          next lockword on freelist
				 */
	uint16	tid;		/* 2: transaction id holding lock */

	uint16	flag;		/* 2: lock control */
	uint16	type;		/* 2: log type */

	jbuf_t	*bp;		/* 4: object page buffer locked */
	inode_t	*ip;		/* 4: object */
				/* (16) */

	int16	lock[24];	/* 48: overlay area */
} tlock_t;			/* (64) */

extern struct tlock	*TxLock;/* transaction lock table */

/*
 * tlock flag
 */
/* txLock state */
#define tlckPAGELOCK		0x8000
#define tlckINODELOCK		0x4000
#define tlckLINELOCK		0x2000
#define tlckINLINELOCK		0x1000
/* lmLog state */
#define tlckLOG			0x0800
/* updateMap state */
#define	tlckUPDATEMAP		0x0080
/* freeLock state */
#define tlckFREELOCK		0x0008
#define tlckWRITEPAGE		0x0004
#define tlckFREEPAGE		0x0002

/*
 * tlock type
 */
#define	tlckTYPE		0xf000
#define	tlckINODE		0x8000
#define	tlckXTREE		0x4000
#define	tlckDTREE		0x2000
#define	tlckMAP			0x1000
#define	tlckEA			0x0800
#define	tlckACL			0x0400
#define	tlckDATA		0x0200
#define	tlckBTROOT		0x0100

#define	tlckOPERATION		0x00ff
#define tlckGROW		0x0001	/* file grow */
#define tlckREMOVE		0x0002	/* file delete */
#define tlckTRUNCATE		0x0004	/* file truncate */
#define tlckRELOCATE		0x0008	/* file/directory relocate */
#define tlckENTRY		0x0001	/* directory insert/delete */
#define tlckEXTEND		0x0002	/* directory extend in-line */
#define tlckSPLIT		0x0010	/* splited page */
#define tlckNEW			0x0020	/* new page from split */
#define tlckFREE		0x0040	/* free page */
#define tlckRELINK		0x0080	/* update sibling pointer */


/*
 *	linelock for lmLog()
 *
 * note: linelock_t and its variations are overlaid
 * at tlock.lock: watch for alignment;
 */
typedef struct {
	uint8	offset;		/* 1: */
	uint8	length;		/* 1: */
} lv_t;				/* (2) */

#define	TLOCKSHORT	20
#define	TLOCKLONG	28

typedef struct {
	uint16	next;		/* 2: next linelock*/

	int8	maxcnt;		/* 1: */
	int8	index;		/* 1: */

	uint16	flag;		/* 2: */
	uint8	type;		/* 1: */
	uint8	l2linesize;	/* 1: log2 of linesize */
				/* (8) */

	lv_t	lv[20];		/* 40: */
} linelock_t;			/* (48) */

#define dtlock_t	linelock_t
#define itlock_t	linelock_t

typedef struct {
	uint16	next;		/* 2: */

	int8	maxcnt;		/* 1: */
	int8	index;		/* 1: */

	uint16	flag;		/* 2: */
	uint8	type;		/* 1: */
	uint8	l2linesize;	/* 1: log2 of linesize */
				/* (8) */

	lv_t	header;		/* 2: */
	lv_t	lwm;		/* 2: low water mark */
	lv_t	hwm;		/* 2: high water mark */
	lv_t	twm;		/* 2: */
				/* (16) */

	int32	pxdlock[8];	/* 32: */
} xtlock_t;			/* (48) */


/*
 *	maplock for txUpdateMap()
 *
 * note: maplock_t and its variations are overlaid
 * at tlock.lock/linelock: watch for alignment;
 * N.B. next field may be set by linelock, and should not
 * be modified by maplock;
 * N.B. index of the first pxdlock specifies index of next 
 * free maplock (i.e., number of maplock) in the tlock; 
 */
typedef struct {
	uint16	next;		/* 2: */

	uint8	maxcnt;		/* 2: */
	uint8	index;		/* 2: next free maplock index */

	uint16	flag;		/* 2: */
	uint8	type;		/* 1: */
	uint8	count;		/* 1: number of pxd/xad */
				/* (8) */

	pxd_t	pxd;		/* 8: */
} maplock_t;			/* (16): */

/* maplock flag */
#define	mlckALLOC		0x00f0	
#define	mlckALLOCXADLIST	0x0080	
#define	mlckALLOCPXDLIST	0x0040
#define	mlckALLOCXAD		0x0020
#define	mlckALLOCPXD		0x0010
#define	mlckFREE		0x000f
#define	mlckFREEXADLIST		0x0008
#define	mlckFREEPXDLIST		0x0004
#define	mlckFREEXAD		0x0002
#define	mlckFREEPXD		0x0001

#define	pxdlock_t	maplock_t

typedef struct {
	uint16	next;		/* 2: */

	uint8	maxcnt;		/* 2: */
	uint8	index;		/* 2: */

	uint16	flag;		/* 2: */
	uint8	type;		/* 1: */
	uint8	count;		/* 1: number of pxd/xad */
				/* (8) */

	void	*xdlist;	/* 4: pxd/xad list */
	int32	rsrvd;		/* 4: */
} xdlistlock_t;			/* (16): */


/*
 *	commit
 *
 * parameter to the commit manager routines
 */
typedef struct commit {
	int32	tid;      	/* 4: tid = index of tblock */
	uint32	flag;		/* 4: flags */
	log_t	*log;		/* 4: log inode */
	inode_t *ipmnt;		/* 4: mount inode */
	inode_t *ipimap;	/* 4: inode allocation map inode */
	inode_t *ipbmap;	/* 4: block allocation map inode */

	int32	nip;   		/* 4: number of entries in iplist */
	inode_t **iplist;	/* 4: list of pointers to inodes    F226941 */
				/* (32)				F226941 */

	/* log record descriptor on 64-bit boundary */
	lrd_t	lrd;    	/* : log record descriptor */
} commit_t;

/*
 * external declarations
 */
tlock_t	*txLock(
	int32		tid,
	inode_t		*ip,
	jbuf_t		*bp,
	uint32		flag);

tlock_t	*txMaplock(
	int32		tid,
	inode_t		*ip,
	uint32		flag);

int32 txCommit(
	int32		tid,
	int32		nip,
	inode_t		**iplist,
	uint32		flag);

void txBegin(
	inode_t		*ipmnt,
	int32		*tid,
	uint32		flag);

void txBeginAnon(						// D233382
	inode_t		*ipmnt);				// D233382

void txEnd(
	int32		tid);

void txAbort(
	int32		tid,
	int32		dirty);

linelock_t *txLinelock(
	linelock_t	*tlock);

void txFreeMap(
	inode_t		*ip,
	maplock_t	*maplock,
	tblock_t	*tblk,
	uint32		maptype);

void txEA(
	int32		tid,
	inode_t		*ip,
	dxd_t		*oldea,
	dxd_t		*newea);

void txFreelock(
	inode_t		*ip);

#endif /* _H_JFS_TXNMGR */
