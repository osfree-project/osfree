/* $Id: jfs_inode.h,v 1.1 2000/04/21 10:57:58 ktk Exp $ */

/* static char *SCCSID = "@(#)1.27  6/15/99 14:30:12 src/jfs/common/include/jfs_inode.h, sysjfs, w45.fs32, fixbld";
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

#ifndef	_H_JFS_INODE
#define _H_JFS_INODE	

/*
 *	jfs_inode.h: in-memory inode/vnode manager
 *
 */

#include "jfs_dinode.h"
#include "jfs_lock.h"
#include "jfs_io.h"

/*
 *	inode hashlist anchor table
 */
extern int32 nhinode;

struct hinode {
	struct	inode *i_hlnext;	/* 4: hashlist next */
	struct	inode *i_hlprev;	/* 4: hashlist prev */

	int32	i_timestamp;		/* 4: */
	int32	i_vget;			/* 4: */
};					/* (16) */

extern struct hinode *ihashtbl;

/* hash function */
#define IHASH(ino,dev,hip)\
{\
        uint32	hash; 				  \
        hash = (uint)(ino) ^ (uint)(dev); 	  \
        hash = (hash >> 8) + (hash >> 12) + hash; \
        (hip) = &ihashtbl[hash & (uint)(nhinode-1)];  \
}

/* mount vfs (embedded in mount inode) */
union mntvfs {
	struct vfs	filesetvfs;
	struct {
		dev_t	dev;
		struct inode	*ipmnt;
	} dummyvfs;
};


/*
 *	in-memory inode
 *
 * inode resides as part of xnode (vnode + inode) aligned on 8-byte boundary
 *
 * todo: AIX vnode and AIX gnode contains dead/wasted space for JFS
 *       which may be redefined for other purpose.
 */

/*
 *	(AIX vnode prefix: 48 bytes)
 */
						/* (48) */
struct	inode {
	/*
	 *	in-memory working information (256 - 48 = 208 bytes)
	 *	----------------------------------------------------
	 */
	struct inode	*i_hlnext;	/* 4: hashlist header: next */
	struct inode	*i_hlprev;	/* 4; hashlist header: previous */
	struct inode	*i_flnext;	/* 4: freelist header: next */
	struct inode	*i_flprev;	/* 4; freelist header: previous */
					/* (64) */

	struct gnode	i_gnode;	/* 4 * 17 = 68: AIX gnode */
	/*
	 *	AIX gnode 
	 *
	struct gnode {
		enum vtype gn_type;	 * 4: object type: v_type * 
		int16	gn_flags;	 * 2: object attribute * 
		int16	rsrvd;		 * 2:
		uint32	gn_seg;		 * 4: pager object * 

		int32	gn_mwrcnt;	 * 4: cnt of map for write (jfs) * 
		int32	gn_mrdcnt;	 * 4: cnt of map for read (jfs) * 
		int32	gn_rdcnt;	 * 4: cnt of open for read (jfs) *
		int32	gn_wrcnt;	 * 4: cnt of open for writ (jfs) *
		int32	gn_excnt;	 * 4: cnt of open for exe (jfs) *
		int32	gn_rshcnt;	 * 4: cnt of open for read shar (jfs) *

		struct vnodeops *gn_ops; * 4: vnode ops vector: v_ops * 
		struct vnode *gn_vnode;	 * 4: vnode bound w/ this inode * 
		dev_t	gn_rdev;	 * 4: device dev_t: v_rdev * 
		chan_t	gn_chan;	 * 4: device chan, minor's minor * 

		TSPINLOCK_T	gn_reclk_lock;   * 8: filelock list lock * 
		int32		gn_reclk_event;  * 4: filelock eventlist * 
		struct filock  *gn_filocks;      * 4: filelock locked region list * 

		caddr_t	gn_data;	 * 4: inode bound w/ this gnode * 
	};
	*/ 
					/* (132) */

	dev_t		i_dev;		/* 4: */
	struct inode	*i_ipimap;	/* 4; mounted fileset inode */
	struct inode	*i_ipmnt;	/* 4: mounted aggregate inode */
	int32		i_count;	/* 4: reference count */

	uint16		i_flag;		/* 2: */
	uint8		i_cflag;        /* 1: commit flags */
	uint8		i_xlock;	/* 1; inode event synchronization */
	event_t		i_event;	/* 4; eventlist for inode activity */
	event_t		i_openevent;	/* 4: eventlist for nshare open modes */
	uint32		i_synctime;	/* 4: first modified time */

	SPINLOCK_T	i_nodelock;	/* 4: inode lock */
	RDWRLOCK_T	i_rdwrlock;	/* 8: read/write lock */

	int64		i_agstart;	/* @8: start xaddr of ag */
					/* (184) */

	/*
	 *	file read/write control:
	 */
	struct cache_ctl	*i_cacheid;/* 4: cache mgmt extension */

	/* btree search heuristics */
	int16		i_btorder;	/* 2: access order */		
	int16		i_btindex;	/* 2: btpage entry index */

	/* read-ahead/write-behind control */
	struct {			/* read-ahead/write-behind control */
		union {
			int8	  _flag;
			struct
			{
				unsigned  _wrbehind :1;	/* acting sequentially*/
				unsigned  _ra_lock  :1;	/* protects readahead */
				unsigned  _reserved :6;
			} s1;
		} u1;
		int8		stepsize;/* 1: num pages to rdahead */
		int16		rasize;	/* 2: num pages have rdahead */
		int64		rapage;	/* @8: rdahead page  */
		int64		wbpage;	/* @8: wrbehind page */
	} i_reg;
					/* (212) */

	/* anonymous tlock list header */
	int16	i_atlhead;		/* 2: anonymous tlock list head */
	int16	i_atltail;		/* 2: anonymous tlock list tail */

	/* pseudo buffer header for btroot (following two fields must be contiguous) */
	uint16	i_bxflag;	/* 2: xflag of pseudo jbuf of btree root */
	int16	i_blid;		/* 2: lid of pseudo jbuf of btree root */
					/* (220) */

	event_t		i_fsevent;	/* 4: eventlist for fs activity */
	struct dasd_usage *i_dasdlim;	/* 4: Dasd usage structure  F226941 */
	int32		rsrvd2[7];	/* 28:			F226941 */
					/* (256) */

	/*
	 *	on-disk inode persistent information (512 bytes)
	 *	-------------------------------------
	 */
	union {
		struct dinode   _dinode;	/* 512: on-disk inode */

                /*
                 *	mounted filesystem inode.
		 *
		 * represented by convention in-memory inode 
		 * (dev_t of mounted file system device, i_number = 0)
                 */
                struct
                {
			uint8	generic[128];		/* 128: */

			uint32		flag;		/* 4: flags */
			struct file	*devfp;		/* 4: fs device file */
			struct inode	*ipbmap;	/* 4: block alloc map */
			struct inode	*ipaimap;	/* 4: aggregate imap */

			uint64		blocks;		/* 8: file system size
							 * in 512 byte blocks 
							 */
			int32		bsize;		/* 4: logical block size
							 * in byte
							 */
			int16		l2bsize;	/* 2: log2(bsize) */
			int16		pbsize;		/* 2: physical block
							 * size in bytes
							 */
			int16		l2pbsize;	/* 2: log2(pbsize) */
			int16		l2bfactor;	/* 2: log2(bsize/pbsize) */

			int16		nbperpage;	/* 2: PSIZE/s_bsize */
			int16		l2nbperpage;	/* 2: log2(pfactor) */

			int16		l2niperblk;	/* 2: log2(s_bsize/disize) */
			int16		rsrvd2;		/* 2: */

			struct cache_dev  *cachedev;	/* 4: cache device */

			struct inode	*iplog;		/* 4: log inode */
			dev_t		logdev;		/* 4: log device */
			pxd_t		logpxd;		/* 8: inline log pxd */
			pxd_t		ait2;		/* 8: ait copy */

			MUTEXLOCK_T	renamelock;	/* 8: rename lock */

			struct	vfs	mntvfs;
			struct inode	*ipaimap2;	/* 4: 2ndary agg imap */
#ifdef	_JFS_OS2
			void * _Seg16	Strategy2;	/* 4: dd strategy */
			void	(* APIENTRY Strategy3)(Req_List_Header *);
							/* 4: flat strategy */
			uint16		maxSGList;	/* 2: max S/G list entries */
#endif	/* _JFS_OS2 */
		} _mntinode;
	} i_dinode;
};

typedef struct inode	inode_t;

/* fileset mode */
#define	FILESET_RDONLY	0x00000001

/* extended mode bits (on-disk inode i_mode): ref. jfs_dinode.h */

#ifdef _JFS_OS2

/* i_mode (actually in dinode) */
#define IFMT	0xF000		/* mask of file type */
#define IFDIR	0x4000		/* directory */
#define IFREG	0x8000		/* regular file */
#define IFLNK	0xA000		/* symbolic link */

#define ISUID	0x0800		/* set user identifier when exec'ing */
#define ISGID	0x0400		/* set group identifier when exec'ing */

#define IREAD	0x0100		/* read permission */
#define IWRITE	0x0080		/* write permission */
#define IEXEC	0x0040		/* execute permission */

/* more extended mode bits: attributes for OS/2 */
#define IREADONLY	0x02000000      /* no write access to file */
#define IARCHIVE	0x40000000      /* file archive bit */
#define ISYSTEM		0x08000000      /* system file */
#define IHIDDEN		0x04000000      /* hidden file */
#define IRASH		0x4E000000	/* mask for changeable attributes */
#define INEWNAME	0x80000000	/* non-8.3 filename format */
#define IDIRECTORY	0x20000000	/* directory (shadow of real bit) */
#define ATTRSHIFT	25		/* bits to shift to move attribute
					   specification to mode position */
#define ISWAPFILE	0x00800000      /* file open for pager swap space */

/* This should be defined in bsedos.h, but isn't */
#define FILE_NEWNAME	0x0040		/* non-8.3 filename format */

#endif /* _JFS_OS2 */

/* i_flag */
#define	IACC	0x0001		/* inode access time to be updated */
#define	ICHG	0x0002		/* inode has been changed */
#define	IUPD	0x0004		/* file has been modified */
#define	IFSYNC	0x0008		/* commit changes to data as well as inode */
#define	IUPDNEW	0x0010		/* file metadata has been modified */
#define	ISTALE	0x0020		/* stale inode of unmounted file system */
#define	ISYNCLIST	0x0040	/* 1 or more jbufs on group commit synclist */

/* i_cflag */
#define COMMIT_NEW	0x01    /* never committed inode   */
#define COMMIT_NOLINK	0x02    /* inode committed with zero link count */
#define COMMIT_INLINEEA	0x04    /* commit inode inline EA */
#define COMMIT_FREEWMAP	0x08    /* free WMAP at iClose() */
#define ICLOSE		0x10    /* inode being recycled */
#define IUNMOUNT	0x20	/* inode in a file system being unmounted */
#define DIRTY		0x40    /* dirty journaled file */
#define INOLOG		0x80	/* temporary file system: no log */

/* i_xlock */
#define	IXLOCK	   0x01		/* inode is in transition */
#define	IXWANT	   0x02		/* some process waiting on lock */
#define	FSXLOCK	   0x04		/* fs is in transition */
#define	FSXWANT	   0x08		/* some process waiting on lock */

/* iget() flags */
#define	IGET_IALLOC	0x00000001

/* i_btorder */
#define	BT_RANDOM		0x0000
#define	BT_SEQUENTIAL		0x0001
#define	BT_LOOKUP		0x0010
#define	BT_INSERT		0x0020
#define	BT_DELETE		0x0040


/*
 *	in-memory inode
 */
/* gnode */
#define i_mwrcnt	i_gnode.gn_mwrcnt
#define	i_mrdcnt	i_gnode.gn_mrdcnt
#define	i_rdcnt		i_gnode.gn_rdcnt
#define	i_wrcnt		i_gnode.gn_wrcnt
#define	i_excnt		i_gnode.gn_excnt
#define	i_rshcnt	i_gnode.gn_rshcnt

#define i_wrbehind	i_reg.u1.s1._wrbehind
#define i_ra_lock	i_reg.u1.s1._ra_lock
#define	i_stepsize	i_reg.stepsize
#define	i_rasize	i_reg.rasize
#define	i_rapage	i_reg.rapage
#define	i_wbpage	i_reg.wbpage

/* type dependent area */


/*
 *	on-disk inode
 */
#define	i_inostamp	i_dinode._dinode.di_inostamp
#define	i_fileset	i_dinode._dinode.di_fileset
#define	i_number	i_dinode._dinode.di_number
#define	i_gen		i_dinode._dinode.di_gen
#define	i_ixpxd		i_dinode._dinode.di_ixpxd
#define i_mode		i_dinode._dinode.di_mode
#define i_size		i_dinode._dinode.di_size
#define i_nblocks	i_dinode._dinode.di_nblocks
#define i_nlink		i_dinode._dinode.di_nlink
#define i_uid		i_dinode._dinode.di_uid
#define i_gid		i_dinode._dinode.di_gid
#define i_atime		i_dinode._dinode.di_atime
#define i_ctime		i_dinode._dinode.di_ctime
#define i_mtime		i_dinode._dinode.di_mtime
#define i_otime		i_dinode._dinode.di_otime
#define i_acl		i_dinode._dinode.di_acl
#define i_ea		i_dinode._dinode.di_ea
#define	i_compress	i_dinode._dinode.di_compress
#define i_acltype	i_dinode._dinode.di_acltype

#define i_dxd		i_dinode._dinode.di_dxd
#define i_btroot	i_dinode._dinode.di_btroot
#define i_DASD		i_dinode._dinode.di_DASD		// F226941
#define i_parent	i_dinode._dinode.di_parent
#define i_inlinedata	i_dinode._dinode.di_inlinedata
#define i_inlineea	i_dinode._dinode.di_inlineea

/* block allocation map */
#define i_bmap		i_dinode._dinode.di_bmap

/* fileset inode 1st half (inode allocation map) */
#define i_imap		i_dinode._dinode.di_imap
#define i_gengen	i_dinode._dinode.di_gengen

/* device special file */
#define i_rdev		i_dinode._dinode.di_rdev

/* symbolic link */
#define i_fastsymlink	i_dinode._dinode.di_fastsymlink

/* mount inode */
#define	i_mntflag	i_dinode._mntinode.flag
#define	i_devfp		i_dinode._mntinode.devfp
#define	i_ipbmap	i_dinode._mntinode.ipbmap
#define	i_ipaimap	i_dinode._mntinode.ipaimap
#define	i_ipaimap2	i_dinode._mntinode.ipaimap2
#define	i_blocks	i_dinode._mntinode.blocks
#define	i_bsize		i_dinode._mntinode.bsize
#define	i_l2bsize	i_dinode._mntinode.l2bsize
#define	i_pbsize	i_dinode._mntinode.pbsize
#define	i_l2pbsize	i_dinode._mntinode.l2pbsize
#define	i_l2bfactor	i_dinode._mntinode.l2bfactor
#define	i_nbperpage	i_dinode._mntinode.nbperpage
#define	i_l2nbperpage	i_dinode._mntinode.l2nbperpage
#define	i_l2niperblk	i_dinode._mntinode.l2niperblk
#define	i_cachedev	i_dinode._mntinode.cachedev
#define	i_iplog		i_dinode._mntinode.iplog
#define	i_logdev	i_dinode._mntinode.logdev
#define	i_logpxd	i_dinode._mntinode.logpxd
#define	i_ait2		i_dinode._mntinode.ait2
#define	i_renamelock	i_dinode._mntinode.renamelock
#define	i_mntvfs	i_dinode._mntinode.mntvfs
#ifdef	_JFS_OS2
#define	i_Strategy2	i_dinode._mntinode.Strategy2
#define	i_Strategy3	i_dinode._mntinode.Strategy3
#define	i_maxSGList	i_dinode._mntinode.maxSGList
#endif	/* _JFS_OS2 */

/*
 * vp <-> ip
 */
#define VP2GP(x)	((struct gnode *)((x)->v_gnode))
#define GP2IP(x)	((struct inode *)(((struct gnode *)(x))->gn_data))
#define VP2IP(x)	(GP2IP(VP2GP(x)))
#define IP2GP(x)	((struct gnode *)(&(((struct inode *)(x))->i_gnode)))
#define IP2VP(x)	((struct vnode *)((x)->i_gnode.gn_vnode))

#define isSpecial(ip)	((ip)->i_number < 2)

#define isReadOnly(ip) (((ip)->i_ipmnt->i_iplog) ? 0 : 1)

#define ISSPARSE(ip)   (((ip)->i_mode & ISPARSE) ? TRUE : FALSE)

#endif /* _H_JFS_INODE */
