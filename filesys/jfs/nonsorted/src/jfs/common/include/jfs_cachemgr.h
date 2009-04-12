/* $Id: jfs_cachemgr.h,v 1.2 2000/09/27 18:38:13 mike Exp $ */

/* static char *SCCSID = "@(#)1.30  9/13/99 11:02:11 src/jfs/common/include/jfs_cachemgr.h, sysjfs, w45.fs32, fixbld";
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

#ifndef _H_JFS_CACHEMGR
#define _H_JFS_CACHEMGR
/*
 *	jfs_cachemgr.h
 */

/*
 * D230860: Report cache usage statistics
 */
#ifndef _JFS_CACHESTATS
#define _JFS_CACHESTATS
#endif

#include "jfs_io.h"
#include "jfs_lock.h"

#define	_JFS_HEURISTICS_SLRU	/* segmented LRU */

/*
 *	 cbuf buffer cache (cCache) manager
 */
struct cCache
{
	MUTEXLOCK_T		cachelock;	/* cache manager lock */
	struct cache_dev	*devfree;	/* cmdev freelist */
	struct cache_dev	*devtail;	/* cmdev iolist tail */
	struct cache_ctl	*ctlfree;	/* cmctl freelist */
	int32			cachesize;	/* total number of cbufs */
	int32			hashsize;	/* number of hashclass */
	struct chash		*hashtbl;	/* cbuf hashlist table */
	CDLL_HEADER(cbuf)	cachelist;	/* cbuf cachelist header */
	struct cbuf		*freelist;	/* cbuf freelist header */
	int32			nfreecbufs;	/* number of free cbufs */
	int32			minfree;	/* min num of free cbufs(lru) */
	int32			maxfree;	/* max num of free cbufs(lru) */
        event_t                 freewait;	/* waitlist for free cbuf */
	int32			numiolru;	/* num of cbuf in i/o for lru */
#ifdef	_JFS_HEURISTICS_SLRU
	int32			slrun;		/* slru prot seg count */
	int32			slruN;		/* max. slru prot seg count */
	struct cbuf		*slruxp;	/* slru seg boundary pointer */
#endif	/* _JFS_HEURISTICS_SLRU */
};

extern	struct cCache cachemgr;


/*
 *	 cache device per mounted file system.
 *
 * cache device list:
 * mutually exclusive list:
 *  iolist: singly-linked (cd_nextdev) circular list
 *		anchored by tail pointer cachemgr.devtail (empty = NULL);
 *  freelist: singly-linked (cd_freenxt) NULL-terminated list
 *		anchored by head pointer cachemgr.devfree:
 */
typedef struct cache_dev
{
	union {
		struct cache_dev * _freenxt;	/* 4: next free cmdev */
		struct cache_dev * _nextdev;	/* 4: next cmdev w/pending I/O */
	} u1;		
	dev_t			cd_dev;		/* 4: file system device id */
	uint8			cd_flag;	/* 1: */
	int8			cd_l2bsize;	/* 1: log2 fs block size */
	int8			cd_l2pbsize;	/* 1: log2 device block size */
	int8			cd_l2bfactor;	/* 1: log2(bsize/psize) */
	int16			cd_nctls;	/* 2: number of active cmctls */
	int16			cd_iolevel;	/* 2: current I/O level */
	event_t			cd_iowait;	/* 4: iolevel wait event */
	struct cbuf		*cd_iotail;	/* 4: cbuf iolist tail */
	iobuf_t			*cd_bufs;	/* 4: device buf freelist */
	int16			cd_pending_requests; /* 2: any I/O requests */
	int16			cd_reserved;	/* 2: */
	event_t			cd_quiesce_event; /* 4: quiesce event */

#ifdef _JFS_OS2
	union {
		void * _Seg16	_strat2p;	/* 4: pointer to DD strategy2 */
		void		(* APIENTRY _strat3p)(Req_List_Header *);
						/* 4: pointer to DD strategy3 */
	} u2;
#endif	/* _JFS_OS2 */
} cmdev_t;					/* (40) */

/* macros for accessing cmdev fields */
#define	cd_freenxt		u1._freenxt
#define	cd_nextdev		u1._nextdev
#define cd_strat2p		u2._strat2p
#define cd_strat3p		u2._strat3p


/* cd_flag */
#define	CD_STRAT3	0x01	/* Flat strategy3 routine is available */
#define CD_QUIESCE	0x02	/* LVM has quiesced filesystem */

/*
 *	 cache control per active file system object
 */
typedef struct cache_ctl
{
        union {
                uint32	_flag;			/* 4: cmctl info */
                struct
                {
                        unsigned   _valid    :1;/* cmctl is active */
                        unsigned   _delpend1 :1;/* delete on I/O complete */
                        unsigned   _delpend2 :1;/* delete on I/O complete */

                        unsigned   _rsrvd   :29;
                } s1;
        } u1;
	union {
        	struct cache_ctl * _freenxt;	/* 4: cmctl freelist link */
        	struct cache_dev * _dev;	/* 4: cmctl's cmdev */
	} u2;
        int32           	cc_ncbufs;	/* 4: number of cbufs */
        int32           	cc_iolevel;	/* 4: current I/O level */
	event_t			cc_iowait;	/* 4: iolevel wait event */
	CDLL_HEADER(cbuf) cc_cbuflist;		/* 8; cbuf pagelist header */
        int32           	cc_njbufs;	/* 4: number of jbufs */
        int32           	cc_jiolevel;	/* 4: current I/O level */
	event_t			cc_jiowait;	/* 4: iolevel wait event */
	CDLL_HEADER(cbuf) cc_jbuflist;		/* 8; jbuf pagelist header */
	event_t			cc_delwait;	/* 4: delpend wait event */
	int32			rsrvd[3];	/* 12: */
} cmctl_t;					/* (64) */

/* macros for accessing cmctl fields */
#define	cc_flag			u1._flag
#define	cc_valid		u1.s1._valid
#define	cc_delpend1		u1.s1._delpend1
#define	cc_delpend		cc_delpend1
#define	cc_delpend2		u1.s1._delpend2
#define	cc_freenxt		u2._freenxt
#define	cc_dev			u2._dev


/*
 *	cache element per active data page:
 *
 * N.B. cbuf_t, jbuf_t, and lbuf_t shares common fields, and
 *	cachelist/freelist;
 *
 *		cache element lists:
 *
 * object pagelist: doubly-linked (cm_ctlfwd/cm_ctlbwd) NULL-terminated
 * 	list anchored by cmctl head pointer cc_cbuflist;
 *		
 * hashlist: doubly-linked (cm_hashfwd/cm_hashbwd) NULL-terminated list
 *	anchored by head pointer ch_head of hashlist anchor
 *	in cachemgr hashlist anchor table;
 *
 * mutually exclusive list:
 *
 *  lru cachelist: doubly-linked (cm_cachefwd/cm_cachebwd);
 *	anchored by head/tail header cachemgr.cachelist;
 *
 *  freelist: singly-linked (cm_freenxt)
 *	anchored by head pointer cachemgr.freelist;
 *
 *  iolist: singly-linked (cm_ionxt) circular list
 *	anchored by cache device tail pointer cd_iotail (empty = NULL);
 */
typedef struct cbuf {
	/*
	 * meta-data buffer prefix area
	 *
	 * jbuf_t and tblock_t common area: logsyncblk prefix:
	 * the following 3 fields are the same as struct logsyncblk
	 * which is common to jbuf_t and tblock_t to form logsynclist.
	 */
					/* - logsyncblk_t - */
	uint16	j_xflag;		/* 2: pageout control flags */
	int16	j_lid;			/* 2: tlock id */
	int32	j_lsn;			/* 4: recovery lsn */
	CDLL_ENTRY(logsyncblk) j_logsynclist;/* 8: log logsynclist link */
					/* - logsyncblk_t(16) - */

	int32	j_clsn;			/* 4: commit lsn */
	int16	j_nohomeok;		/* 2: */
	int16	whoami;			/* 2: */
	struct logx	*j_logx;	/* 4: ?log inode pinned extension */
	struct inode	*j_ip;		/* 4: jbuf's inode */
					/* (32) */
	/*
	 * data buffer base area
	 */
	union {
                uint32   	   _flag;	/* 4: cbuf info */
		struct {
			unsigned  _metadata :1;  /* data type */
			unsigned  _free     :1;  /* on freelist */
			unsigned  _cached   :1;  /* on cachelist */
			unsigned  _inuse    :1;  /* checked out */

			unsigned  _read     :1;  /* read in progress */
			unsigned  _write    :1;  /* write in progress */
			unsigned  _iodone   :1;  /* i/o done */
			unsigned  _ioerror  :1;  /* i/o error */

			unsigned  _new      :1;  /* newly allocated */
			unsigned  _modified :1;  /* modified */
			unsigned  _lru      :1;  /* lru in progress */
			unsigned  _discard  :1;  /* discard on I/O end */

			unsigned  _hole     :1;  /* not backed on disk */
			unsigned  _abnr     :1;  /* alloced but !recorded */
			unsigned  _probationary :1; /* slru probationary */
			unsigned  _hit      :1;	 /* slru cache hit */

			unsigned  _rdahead  :1;  /* readahead trigger */
			unsigned  _sendfile :1;  /* in use by sendfile */
			unsigned  _async    :1;  /* Asynchronous I/O  D230860 */
			unsigned  _rsrvd    :13;
		} s1;
	} u1;					/* 4: */
	cmctl_t			*cm_ctl;	/* 4: cbuf's inodex */
	CDLL_ENTRY(cbuf)	cm_pagelist;	/* 8: pagelist link */
	LIST_ENTRY(cbuf)	cm_hashlist;	/* 8: hashlist link */
	union {
		CDLL_ENTRY(cbuf)	_cachelist; /* 8: cachelist link */
		struct {
			struct cbuf * _devnxt;	/* 4: iolist of iolist link */
			struct cbuf * _ionxt;   /* 4: iolist link */
		} _iolist;
		struct cbuf * _freenxt;	/* 4: freelist link */
		uint32		_usecount;	/* 4: use count for sendfile */
	} u2;					/* 8: */
	pxd_t		cm_pxd;			/* 8: disk address */
	int64		cm_blkno;		/* @8: logical block number */
	caddr_t		cm_cdata;		/* 4: cache buffer address */
	event_t		cm_event;		/* 4: wait list for holds */
	event_t		cm_ioevent;		/* 4: i/o wait list */
	iobuf_t		*cm_iobp;		/* 4: iobuf bound */
						/* (96) */
#ifdef	_JFS_OS2
	/*
	 * OS/2 extension area
	 */
	SG_Descriptor	cm_pgdesc;		/* 8: page frame */
#endif	/* _JFS_OS2 */
						/* (104) */
} cbuf_t;

/* macros for accessing cbuf fields */
#define	cm_flag		u1._flag
#define	cm_metadata	u1.s1._metadata
#define	cm_inuse	u1.s1._inuse
#define	cm_read		u1.s1._read
#define	cm_write	u1.s1._write
#define	cm_iodone	u1.s1._iodone
#define	cm_new		u1.s1._new
#define	cm_hole		u1.s1._hole
#define	cm_cached	u1.s1._cached
#define	cm_free		u1.s1._free
#define	cm_lru		u1.s1._lru
#define	cm_discard	u1.s1._discard
#define	cm_modified	u1.s1._modified
#define cm_rdahead	u1.s1._rdahead
#define cm_sendfile	u1.s1._sendfile
#define cm_async	u1.s1._async
#define cm_abnr		u1.s1._abnr
#define cm_probationary	u1.s1._probationary
#define cm_hit		u1.s1._hit

/* mutually exclusive state overlay */
#define	cm_cachelist	u2._cachelist	/* cached */
#define	cm_devnxt	u2._iolist._devnxt	/* i/o */
#define	cm_ionxt	u2._iolist._ionxt	/* i/o */
#define	cm_freenxt	u2._freenxt	/* free */
#define	cm_usecount	u2._usecount	/* sendfile */

#define	IOQUEUE_READ(bp) \
{\
	bp->cm_read = TRUE;\
	bp->cm_ioevent = EVENT_NULL;\
	bp->cm_iodone = FALSE;\
	bp->cm_async = FALSE;\
}

#define	IOQUEUE_WRITE(bp) \
{\
	bp->cm_write = TRUE;\
	bp->cm_ioevent = EVENT_NULL;\
	bp->cm_iodone = FALSE;\
	bp->cm_async = FALSE;\
}

// BEGIN D230860
#define	IOQUEUE_ASYNC_READ(bp) \
{\
	bp->cm_read = TRUE;\
	bp->cm_ioevent = EVENT_NULL;\
	bp->cm_iodone = FALSE;\
	bp->cm_async = TRUE;\
}

#define	IOQUEUE_ASYNC_WRITE(bp) \
{\
	bp->cm_write = TRUE;\
	bp->cm_ioevent = EVENT_NULL;\
	bp->cm_iodone = FALSE;\
	bp->cm_async = TRUE;\
}
// END D230860

/* flags for queuing I/O */
#define	CM_WRITE	0x00000001
#define	CM_READ		0x00000002
#define	CM_LRU		0x00000004
#define	CM_WAIT		0x00000008


/*
 *	buffer cache hashlist header
 *
 * table of buffer hash class anchors  where
 * each hash class anchor contains a header for
 * doubly-linked lists of buffer headers linked via j_hashlist
 */
typedef struct chash {
	uint32     	ch_flag;	/* 4: ? */
	LIST_HEADER(cbuf)	ch_hashlist; /* 4: hashlist header */
	uint32		ch_timestamp;	/* 4: */
	SPINLOCK_T	ch_lock;	/* 4: Not Used? */
} chash_t;				/* (16) */

/* cbuf hash function */
#define CM_HASH(c,b)	\
	((chash_t *)(cachemgr.hashtbl + (((uint)(c) ^ (uint)(b)) & (uint)(cachemgr.hashsize-1))))

#define	CM_BSIZE	4096		/* size of a cache buffer	 */
#define	CM_L2BSIZE	12		/* l2 size of a cache buffer	 */
#define	CM_OFFSET	0x00000fff	/* mask for offset within buffer */

#define MIN_RAPAGE	2		/* minmum number of read-ahead pages */
#define MAX_RAPAGE	8		/* maximum number of read-ahead pages */
#define	CM_WRCLNBLKS	8		/* write behind cluster size	 */

#define	CM_MAXLBLKNO	((MAXFILESIZE >> CM_L2BSIZE) - 1) /* max lblkno */

/*  get block allocation allocation hint as location of disk inode */
#define	INOHINT(ip)	\
	(addressPXD(&((ip)->i_ixpxd)) + lengthPXD(&((ip)->i_ixpxd)) - 1)

/* number of bytes to number of pages */
#define CM_BTOCBLKS(b)  (( (b) + CM_BSIZE - 1) >> CM_L2BSIZE)

/* number of bytes (size) to a number of file system blocks */
#define CM_BTOFBLKS(b,l2s)      (((b) + (1 << (l2s)) - 1) >> l2s)

/* number of bytes (size) to  page number */
#define CM_BTOCBLK(b)  (((int64)((b) - 1)) >> CM_L2BSIZE)

/* number of bytes (size) to number of file system blocks in last page */
#define CM_BTOBLKS(b,bsize,l2bsize)      \
        (((((b) - 1) & CM_OFFSET) + (bsize)) >> (l2bsize))

/* offset to page number */
#define CM_OFFTOCBLK(o)         ((o) >> CM_L2BSIZE)

/* page number to size in bytes to the end of the page */
#define CM_CBLKTOB(b)  ((int64)((b) + 1 ) << CM_L2BSIZE)


/*
 *	Cache List - used by FS32_READFILEATCACHE
 */
#define	MAX_CACHE_LIST	8

typedef struct cache_list {
	KernCacheList_t	kCacheList;			/* 16: in bsekee.h  */
	uint32		nCbufs;				/* 4: # of entries  */
	KernPageList_t	linearList[MAX_CACHE_LIST];	/* 64: linear addrs */
	KernPageList_t	physicalList[MAX_CACHE_LIST];	/* 64: phys. addrs  */
	cbuf_t		*cbuf[MAX_CACHE_LIST];		/* 32: cbuf list    */
} cache_list_t;						/* (180) */

/*
 *	synchronization
 */
#define IOCACHE_LOCK() 		XSPINLOCK_LOCK(&jfsIOCacheLock)
#define IOCACHE_UNLOCK(xipl) 	XSPINLOCK_UNLOCK(xipl,&jfsIOCacheLock)
#define IOCACHE_NOLOCK(xipl) 	XSPINLOCK_NOLOCK(xipl,&jfsIOCacheLock)

#define IOCACHE_SLEEP(event,flag)\
    	XEVENT_SLEEP((event),&jfsIOCacheLock,T_XSPINLOCK|flag)
#define IOCACHE_SLEEP_RC(event,flag,rc) \
    	XEVENT_SLEEP_RC((event),&jfsIOCacheLock,T_XSPINLOCK|flag,rc)

#define IOCACHE_WAKEUP(event)	XEVENT_WAKEUP((event))
#define IOCACHE_WAKEUP_RC(event,rc) XEVENT_WAKEUP_RC((event),(rc))

/*
 *	Quiesce Filesystem
 */
#define IS_QUIESCE(cdp)						\
{								\
	int32 xipl;						\
	xipl = IOCACHE_LOCK();					\
	while ((cdp)->cd_flag & CD_QUIESCE)			\
		IOCACHE_SLEEP(&(cdp)->cd_quiesce_event, 0);	\
	(cdp)->cd_pending_requests++;				\
	IOCACHE_UNLOCK(xipl);					\
}

// BEGIN 230860
#ifdef _JFS_CACHESTATS
struct cachestats
{
	/*
	 * Copied from cachemgr structure
	 */
	int32	cachesize;
	int32	hashsize;
	int32	nfreecbufs;
	int32	minfree;
	int32	maxfree;
	int32	numiolru;
	int32	slrun;
	int32	slruN;

	/*
	 * Calculated
	 */
	int32	cbufs_protected;
	int32	cbufs_probationary;
	int32	cbufs_inuse;
	int32	cbufs_io;
	int32	jbufs_protected;
	int32	jbufs_probationary;
	int32	jbufs_inuse;
	int32	jbufs_io;
	int32	jbufs_nohomeok;
};
#endif /* _JFS_CACHESTATS */
// END 230860


#ifndef	_JFS_UTILITY
/*
 *	prototypes for cache manager services.
 */
int32 cmInit(void);

int32 cmMount(inode_t	*ipmnt, int32 nbufs);

void cmUnmount(struct cache_dev *cdp);

int32 cmCreate(struct cache_dev *cdp, struct cache_ctl **ccpp);

int32 cmDelete(struct cache_ctl *ccp);

int32 cmRead(inode_t *ip,
	int64 offset,
	int32	rdsize,
	cbuf_t **cpp);

void cmKickStart(inode_t *ip);

int32 cmReadWR(inode_t *ip,
	int64 offset,
	int32 nbytes,
	struct cbuf **cpp);

struct cbuf *cmGetE(struct cache_ctl *ccp, int64 lblkno, uint32 flag);

struct cbuf *cmRecycle(int32 wait, int32 *ipl);

void cmPut(cbuf_t *cp, int32  dirty);

void cmRelCbuf(cbuf_t *cp);

void cmFreeCbuf(struct cbuf *);

int32 cmTruncate(inode_t *ip,
                 int64 offset,
 		 int64 len);

void cmRelease(struct cache_ctl *ccp);

int32 cmWrite(struct cache_ctl *ccp,
              int64 blkno,
              int64 nblks);

void cmAsyncWrite(struct cache_ctl *ccp,
	     	  int64 blkno,
	     	  int64 nblks);

void cmDeviceQueueIO(cbuf_t *cp, int32 iotype);

cbuf_t *cmQueueIO(void);

void cmStartIO(cbuf_t *devtail);

void cmDeviceStartIO(cmdev_t *cdp, cbuf_t *iotail);

int64 cmQueryCache(inode_t *ip, int64 offset, int64 nbytes);

int32 cmFlush(cmctl_t *cpp);

void cmSetXD(inode_t *ip, cbuf_t *cp, int64 blkno, int64 xaddr, int32 size);

int32 cmSendFile(inode_t *ip, int64 offset, int64 *pLen,
		 cache_list_t *cacheList);

cbuf_t *rawAllocate(void);
int32 rawRead(inode_t *ipmnt, int64 offset, struct cbuf **bpp);
int32 rawWrite(inode_t *ipmnt, struct cbuf *bp, uint32 flag);
void rawRelease(struct cbuf *bp);
#endif	/* _JFS_UTILITY */
#endif  /* _H_JFS_CACHEMGR */
