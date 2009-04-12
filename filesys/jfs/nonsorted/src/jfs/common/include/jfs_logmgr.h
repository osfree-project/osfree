/* $Id: jfs_logmgr.h,v 1.2 2000/09/27 18:38:13 mike Exp $ */

/* static char *SCCSID = "@(#)1.22  9/13/99 11:24:09 src/jfs/common/include/jfs_logmgr.h, sysjfs, w45.fs32, fixbld";
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
#ifndef	_H_JFS_LOGMGR
#define _H_JFS_LOGMGR	

/*
 *	jfs_logmgr.h: log manager
 *
 */

/*
 * Change History :
 *
 */

#ifndef _JFS_LAZYCOMMIT	// get rid of warnings
#define _JFS_LAZYCOMMIT						// D230860
#endif

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_io.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"


/*
 *	log manager configuration parameters
 */
/*
 * log logical volumes start with this standard prefix.
 * this will be determined from an ODM query as will the log type
 */
#define	LOGNAME_PREFIX	"log"
#define	LOGTYPE		"jfs"

#define	MAXNLOG		256	/* max number of logs */
#define	LOGMINOR	8	/* reserved minor dev_t for log */

/* log page size */
#define	LOGPSIZE	4096	
#define	L2LOGPSIZE	12

/*
 *	log logical volume
 *
 * a log is used to make the commit operation on journalled
 * files within the same logical volume group atomic.
 * a log is implemented with a logical volume.
 * there is one log per logical volume group.
 *
 * block 0 of the log logical volume is not used (ipl etc).
 * block 1 contains a log "superblock" and is used by logFormat(),
 * lmLogInit(), lmLogShutdown(), and logRedo() to record status
 * of the log but is not otherwise used during normal processing.
 * blocks 2 - (N-1) are used to contain log records.
 *
 * when a volume group is varied-on-line, logRedo() must have
 * been executed before the file systems (logical volumes) in
 * the volume group can be mounted.
 */
/*
 *	log superblock (block 1 of logical volume)
 */
#define	LOGSUPER_B	1
#define	LOGSTART_B	2

#define	LOGMAGIC	0x87654321
#define	LOGVERSION	1

typedef struct {
	uint32	magic;		/* 4: log lv identifier */
	int32	version;	/* 4: version number */
	int32	serial;		/* 4: log open/mount counter */
	int32	size;		/* 4: size in number of LOGPSIZE blocks */
	int32	bsize;		/* 4: logical block size in byte */
	int32	l2bsize;	/* 4: log2 of bsize */

	uint32	flag;		/* 4: option */
	uint32	state;		/* 4: state - see below */

	int32	end;		/* 4: address of last log record set by logredo() */
	uint32	active[8];	/* 32: active file systems bit vector */
	int32	rsrvd[LOGPSIZE/4 - 17];
} logsuper_t;				

/* log flag: commit option (see jfs_filsys.h) */

/* log state */
#define	LOGMOUNT	0	/* log mounted by lmLogInit() */
#define LOGREDONE	1	/* log shutdown by lmLogShutdown().
				 * log redo completed by logredo().
				 */
#define LOGWRAP		2	/* log wrapped */
#define LOGREADERR	3	/* log read error detected in logredo() */


/*
 *	log logical page
 *
 * (this comment should be rewritten !)
 * the header and trailer structures (h,t) will normally have
 * the same page and eor value.
 * An exception to this occurs when a complete page write is not
 * accomplished on a power failure. Since the hardware may "split write"
 * sectors in the page, any out of order sequence may occur during powerfail
 * and needs to be recognized during log replay.  The xor value is
 * an "exclusive or" of all log words in the page up to eor.  This
 * 32 bit eor is stored with the top 16 bits in the header and the
 * bottom 16 bits in the trailer.  logredo can easily recognize pages
 * that were not completed by reconstructing this eor and checking
 * the log page.
 *
 * Previous versions of the operating system did not allow split
 * writes and detected partially written records in logredo by
 * ordering the updates to the header, trailer, and the move of data
 * into the logdata area.  The order: (1) data is moved (2) header
 * is updated (3) trailer is updated.  In logredo, when the header
 * differed from the trailer, the header and trailer were reconciled
 * as follows: if h.page != t.page they were set to the smaller of
 * the two and h.eor and t.eor set to 8 (i.e. empty page). if (only)
 * h.eor != t.eor they were set to the smaller of their two values.
 */
typedef struct {
	struct {		/* header */
		int32	page;	/* 4: log sequence page number */
		int16	rsrvd;	/* 2: */
		int16	eor;	/* 2: end-of-log offset of lasrt record write */
	} h;

	int32	data[LOGPSIZE/4 - 4];	/* log record area */

	struct {		/* trailer */
		int32	page;	/* 4: normally the same as h.page */
		int16	rsrvd;	/* 2: */
		int16	eor;	/* 2: normally the same as h.eor */
	} t;
} logpage_t;

#define LOGPHDRSIZE	8	/* log page header size */
#define LOGPTLRSIZE	8	/* log page trailer size */

/*
 *	log physical page (tbd: split/out-of-order write detection)
 */
typedef struct {
	struct {
		int16	data[255];
		int16	timestamp;
	} sector[8];
} ppage_t;


/*
 *	log record
 *
 * (this comment should be rewritten !)
 * jfs uses only "after" log records (only a single writer is allowed
 * in a  page, pages are written to temporary paging space if
 * if they must be written to disk before commit, and i/o is
 * scheduled for modified pages to their home location after
 * the log records containing the after values and the commit
 * record is written to the log on disk, undo discards the copy
 * in main-memory.)
 *
 * a log record consists of a data area of variable length followed by
 * a descriptor of fixed size LOGRDSIZE bytes.
 * the  data area is rounded up to an integral number of 4-bytes and
 * must be no longer than LOGPSIZE.
 * the descriptor is of size of multiple of 4-bytes and aligned on a
 * 4-byte boundary.
 * records are packed one after the other in the data area of log pages.
 * (sometimes a DUMMY record is inserted so that at least one record ends
 * on every page or the longest record is placed on at most two pages).
 * the field eor in page header/trailer points to the byte following
 * the last record on a page.
 */

/* log record types */
#define LOG_COMMIT		0x8000
#define LOG_SYNCPT		0x4000
#define LOG_MOUNT		0x2000
#define LOG_REDOPAGE		0x0800
#define LOG_NOREDOPAGE		0x0080
#define LOG_NOREDOINOEXT	0x0040
#define LOG_UPDATEMAP		0x0008
#define LOG_NOREDOFILE		0x0001

/* REDOPAGE/NOREDOPAGE log record data type */
#define	LOG_INODE		0x0001
#define	LOG_XTREE		0x0002
#define	LOG_DTREE		0x0004
#define	LOG_BTROOT		0x0010
#define	LOG_EA			0x0020
#define	LOG_ACL			0x0040
#define	LOG_DATA		0x0080
#define	LOG_NEW			0x0100
#define	LOG_EXTEND		0x0200
#define LOG_RELOCATE		0x0400

/* UPDATEMAP log record descriptor type */
#define	LOG_ALLOCXADLIST	0x0080
#define	LOG_ALLOCPXDLIST	0x0040
#define	LOG_ALLOCXAD		0x0020
#define	LOG_ALLOCPXD		0x0010
#define	LOG_FREEXADLIST		0x0008
#define	LOG_FREEPXDLIST		0x0004
#define	LOG_FREEXAD		0x0002
#define	LOG_FREEPXD		0x0001


typedef struct lrd {
	/*
	 * type independent area
	 */
	int32	logtid;		/* 4: log transaction identifier */
	int32	backchain;	/* 4: ptr to prev record of same transaction */
	uint16	type;		/* 2: record type */
	int16	length;		/* 2: length of data in record (in byte) */
	int32	aggregate;	/* 4: file system lv/aggregate */
				/* (16) */

	/*
	 * type dependent area (20)
	 */
	union {

		/*
		 *	COMMIT: commit
		 *
		 * transaction commit: no type-dependent information;
		 */

		/*
		 *	REDOPAGE: after-image
		 *
		 * apply after-image;
		 *
		 * N.B. REDOPAGE, NOREDOPAGE, and UPDATEMAP must be same format;
		 */
		struct {
			ino_t	fileset;	/* 4: fileset number */
			ino_t	inode;		/* 4: inode number */
			uint16	type;		/* 2: REDOPAGE record type */
			int16	l2linesize;	/* 2: log2 of line size */
			pxd_t	pxd;		/* 8: on-disk page pxd */
		} redopage;			/* (20) */

		/*
		 *	NOREDOPAGE: the page is freed
		 *
		 * do not apply after-image records which precede this record
		 * in the log with the same page block number to this page.
		 *
		 * N.B. REDOPAGE, NOREDOPAGE, and UPDATEMAP must be same format;
		 */
		struct {
			int32	fileset;	/* 4: fileset number */
			ino_t	inode;		/* 4: inode number */
			uint16	type;		/* 2: NOREDOPAGE record type */
			int16	rsrvd;		/* 2: reserved */
			pxd_t	pxd;		/* 8: on-disk page pxd */
		} noredopage;			/* (20) */

		/*
		 *	UPDATEMAP: update block allocation map
		 *
		 * either in-line PXD,
		 * or	  out-of-line  XADLIST;
		 *
		 * N.B. REDOPAGE, NOREDOPAGE, and UPDATEMAP must be same format;
		 */
		struct {
			ino_t	fileset;	/* 4: fileset number */
			ino_t	inode;		/* 4: inode number */
			uint16	type;		/* 2: UPDATEMAP record type */
			int16	nxd;		/* 2: number of extents */
			pxd_t	pxd;		/* 8: pxd */
		} updatemap;			/* (20) */

		/*
		 *	NOREDOINOEXT: the inode extent is freed
		 *
		 * do not apply after-image records which precede this
		 * record in the log with the any of the 4 page block
		 * numbers in this inode extent.
		 *
		 * NOTE: The fileset and pxd fields MUST remain in
		 * 	 the same fields in the REDOPAGE record format.
		 *
		 */
		struct {
			int32	fileset;	/* 4: fileset number */
			int32	iagnum;		/* 4: IAG number     */
			int32   inoext_idx;     /* 4: inode extent index */
			pxd_t	pxd;		/* 8: on-disk page pxd */
		} noredoinoext;			/* (20) */

		/*
		 *	SYNCPT: log sync point
		 *
		 * replay log upto syncpt address specified;
		 */
		struct {
			int32	sync;		/* 4: syncpt address (0 = here) */
		} syncpt;

		/*
		 *	MOUNT: file system mount
		 *
		 * file system mount: no type-dependent information;
		 */

		/*
		 *	? FREEXTENT: free specified extent(s)
		 *
		 * free specified extent(s) from block allocation map
		 * N.B.: nextents should be length of data/sizeof(xad_t)
		 */
		struct {
			int32	type;		/* 4: FREEXTENT record type */
			int32	nextent;	/* 4: number of extents */

			/* data: PXD or XAD list */
		} freextent;

		/*
		 *	? NOREDOFILE: this file is freed
		 *
		 * do not apply records which precede this record in the log
		 * with the same inode number.
		 *
		 * NOREDILE must be the first to be written at commit
		 * (last to be read in logredo()) - it prevents
		 * replay of preceding updates of all preceding generations
		 * of the inumber esp. the on-disk inode itself,
		 * but does NOT prevent
		 * replay of the
		 */
		struct {
			int32	fileset;	/* 4: fileset number */
			ino_t	inode;		/* 4: inode number */
		} noredofile;

		/*
		 *	? NEWPAGE:
		 *
		 * metadata type dependent
		 */
		struct {
			int32	fileset;	/* 4: fileset number */
			ino_t	inode;		/* 4: inode number */
			int32	type;		/* 4: NEWPAGE record type */
			pxd_t	pxd;		/* 8: on-disk page pxd */
		} newpage;

		/*
		 *	? DUMMY: filler
		 *
		 * no type-dependent information
		 */
	      } log;
} lrd_t;				/* (36) */

#define	LOGRDSIZE	(sizeof(struct lrd))

/*
 *	line vector descriptor
 */
typedef struct {
	int16	offset;
	int16	length;
} lvd_t;


/*
 *	log logical volume inode.
 *
 * represented by convention in-memory inode
 * (dev_t of log device, i_number = 0)
 *
 * N.B. overlaid on inode;
 */
typedef struct log {
	uint8	inode[208];	/* 208: in-memory area */
				/* - on-disk inode area - */
	uint8	dinode[128];	/* 128: dinode base area */

	LIST_ENTRY(log)	loglist;/* 8: active log list link */
	dev_t	dev;		/* 4: log lv number */
#ifdef _JFS_OS2
	void * _Seg16	strat2p;	/* 4: log device strategy routine */
	void	(* APIENTRY strat3p)(Req_List_Header *);
				/* 4: log device flat strategy routine */
#else
	struct file	*devfp;	/* 4: log device file */
#endif
	int32	serial;		/* 4: log mount serial number */
	struct logx	*logx;	/* 4: extension for pinned data */

	int64	base;		/* @8: log extent address (inline log ) */
	int32	size;		/* 4: log size in log page (in page) */
	int32	bsize;		/* 4: log page/block size in byte */
	int32	pbsize;		/* 4: hardware/LVM block size in bytes */
	int16	l2bsize;	/* 2: log2 of log2(bsize) */
	int16	l2pbsize;	/* 2: log2 of log2(pbsize) */

	uint32	flag;		/* 4: flag (dup) */
	uint32	state;		/* 4: state */

	/* log write */
	int32	logtid;		/* 4: log tid */
	int32	page;		/* 4: page number of eol page */
	int32	eor;		/* 4: eor of last record in eol page */
	struct lbuf	*bp;	/* 4: current log page buffer */

	MUTEXLOCK_T   loglock;	/* 4: log write serialization lock */

	/* syncpt */
	int32	nextsync;	/* 4: bytes to write before next syncpt */
	int32	active;		/* 4: */
	int32	syncbarrier;	/* 4: */
	event_t	syncwait;	/* 4: */

	/* commit */
	uint32	cflag;		/* 4: */
	struct {		/* 8: FIFO commit queue header */
		struct tblock	*head;
		struct tblock	*tail;
	} cqueue;
	int32	gcrtc;		/* 4: GC_READY transaction count */
	struct tblock	*gclrt;	/* 4: latest GC_READY transaction */
// BEGIN D230860
#ifdef _JFS_LAZYCOMMIT
	XSPINLOCK_T	gclock;	/* 4: group commit lock */
#else
	MUTEXLOCK_T	gclock;	/* 8: group commit lock */
#endif
// END D230860
} log_t;

/*
 * group commit flag
 */
/* log_t */
#define logGC_PAGEOUT	0x00000001

/* tblock_t/lbuf_t */
#define tblkGC_QUEUE		0x0001
#define tblkGC_READY		0x0002
#define tblkGC_COMMIT		0x0004
#define tblkGC_COMMITTED	0x0008
#define tblkGC_EOP		0x0010
#define tblkGC_FREE		0x0020
#define tblkGC_LEADER		0x0040
#define tblkGC_ERROR		0x0080
#define tblkGC_LAZY		0x0100				// D230860
#define tblkGC_UNLOCKED		0x0200				// D230860

/*
 *	logx - log inode pinned extension
 */
typedef struct logx {
	struct logx	*freelist;	/* 4: logx freelist link */
	uint32		flag;		/* 4: flag (dup) */
	int32		size;		/* 4: log data area size in byte */
	int32		lsn;		/* 4: end-of-log */
	int32		clsn;		/* 4: clsn */
	int32		syncpt;		/* 4: addr of last syncpt record */
	int32		sync;		/* 4: addr from last logsync() */
	CDLL_HEADER(logsyncblk) synclist;/* 8: logsynclist anchor */
#ifdef	_JFS_OS2
	SPINLOCK_T	synclock;	/* 4: synclist lock */
#endif
	struct lbuf	*wqueue;	/* 4: log pageout queue */
	jbuf_t		*yah;		/* 4: marker */
	int16		count;		/* 2: count */
	int16		nBuffer;	/* 2: number of iobuf */
	iobuf_t		*iobp;		/* 4: single io buffer */
} logx_t;				/* (64) */


/*
 *		log cache buffer header
 *
 * N.B. cbuf_t, jbuf_t, and lbuf_t shares common fields, and
 *	cachelist/freelist;
 */
typedef struct lbuf {
	/*
	 * meta-data buffer prefix area
	 */
	int32	l_prefix[7];		/* 28: */
	log_t	*l_log;			/* 4: inode associated with buffer */
					/* (32) */
	/*
	 * data buffer base area
	 */
	uint32	l_flag;			/* 4: pageout control flags */
	logx_t	*l_logx;		/* 4: log inode extension */

	struct lbuf	*l_wqnext;	/* 4: write queue link */
	struct lbuf	*l_freelist;	/* 4: freelistlink */

	int32	rsrvd1[3];		/* 12: */

	int32	l_pn;			/* 4: log page number */
	int32	l_eor;			/* 4: log record eor */
	int32	l_ceor;			/* 4: committed log record eor */

	int64	l_blkno;		/* @8: log page block number */
	caddr_t	l_ldata;		/* 4: data page */

	int32	rsrvd2;			/* 4: */

	event_t	l_ioevent;		/* 4: i/o done event */
	iobuf_t	*l_iobp;		/* 4: i/o buffer */
					/* (96) */
#ifdef	_JFS_OS2
	/*
	 * OS/2 extension area
	 */
	SG_Descriptor	l_pgdesc;	/* 8: page frame */
#endif	/* _JFS_OS2 */
					/* (104) */
} lbuf_t;


/*
 *	logsynclist block
 *
 * common logsyncblk prefix for jbuf_t and tblock_t
 */
typedef struct logsyncblk {
	uint16	xflag;		/* 2: flags */
	int16	lid;		/* 2: tlock id */
	int32	lsn;		/* 4: log sequence number */
	CDLL_ENTRY(logsyncblk)	synclist;/* 8: log sync list link */
} logsyncblk_t;			/* (16) */


/*
 *	logsynclist serialization (per log)
 */
#ifdef	_JFS_OS2
#define LOGSYNC_LOCK_ALLOC(logx, lockid)	\
	SPINLOCK_ALLOC((void *)&(logx)->synclock, LOCK_ALLOC_PIN,	\
	JFS_LOGSYNC_LOCK_CLASS, lockid)
#define LOGSYNC_LOCK_FREE(logx)	SPINLOCK_FREE(&(logx)->synclock)
#define LOGSYNC_LOCK_INIT(logx)	SPINLOCK_INIT(&(logx)->synclock)
#define	LOGSYNC_LOCK(logx)	(SPINLOCK_LOCK(&(logx)->synclock), 0)
#define	LOGSYNC_UNLOCK(xipl, logx)	SPINLOCK_UNLOCK(&(logx)->synclock)
#endif	/* _JFS_OS2 */

/* compute the difference in bytes of lsn from sync point */
#define logdiff(diff, lsn, logx)\
{\
	diff = (lsn) - (logx)->syncpt;\
	if (diff < 0)\
		diff += (logx)->size;\
}


#ifndef _JFS_UTILITY
int32 lmLogQuiesce(log_t *log);
int32 lmLogResume(log_t	*log, inode_t *ipmnt);
int32 lmLogFormat(inode_t *ipmnt, int64 logAddress, int32 logSize);
#endif	/* _JFS_UTILITY */

#endif /* _H_JFS_LOGMGR */	
