/* $Id: jfs_dtree.h,v 1.1.1.1 2003/05/21 13:35:48 pasha Exp $ */

/* static char *SCCSID = "@(#)1.18  7/6/99 09:19:20 src/jfs/common/include/jfs_dtree.h, sysjfs, w45.fs32, fixbld";
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

#ifndef _H_JFS_DTREE
#define	_H_JFS_DTREE

/*
 *	jfs_dtree.h: directory B+-tree manager
 */

#ifdef _JFS_OS2
#include "jfs_dirent.h"
#endif /* _JFS_OS2 */

#include "jfs_btree.h"

typedef union {
        ino_t   ino;
        pxd_t   xd;
} ddata_t;


/*
 *      entry segment/slot
 *
 * an entry consists of type dependent head/only segment/slot and
 * additional segments/slots linked vi next field;
 * N.B. last/only segment of entry is terminated by next = -1;
 */
/*
 *	directory page slot
 */
typedef struct {
	int8	next;		/* 1: */
	int8	cnt;		/* 1: */
	UniChar	name[15];	/* 30: */
} dtslot_t;			/* (32) */

#define	DTSLOTSIZE	32
#define	L2DTSLOTSIZE	5
#define DTSLOTHDRSIZE	2
#define DTSLOTDATASIZE	30
#define DTSLOTDATALEN	15


/*
 *	 internal node entry head/only segment
 */
typedef struct {
	pxd_t	xd;		/* 8: child extent descriptor */

	int8	next;		/* 1: */
	uint8	namlen;		/* 1: */
	UniChar	name[11];	/* 22: 2-byte aligned */
} idtentry_t;			/* (32) */

#define DTIHDRSIZE	10
#define DTIHDRDATALEN	11

/* compute number of slots for entry */
#define	NDTINTERNAL(klen) ( ((4 + (klen)) + (15 - 1)) / 15 )


/*
 *	leaf node entry head/only segment
 */
typedef struct {
	ino_t	inumber;	/* 4: 4-byte aligned */	

	int8	next;		/* 1: */
	uint8	namlen;		/* 1: */
	UniChar	name[13];	/* 26: 2-byte aligned */
} ldtentry_t;			/* (32) */

#define DTLHDRSIZE	6
#define DTLHDRDATALEN	13

/* compute number of slots for entry */
#define	NDTLEAF(klen)	( ((2 + (klen)) + (15 - 1)) / 15 )


/*
 *	directory root page (in-line in on-disk inode):
 *
 * cf. dtpage_t below.
 */
typedef union {
	struct {
		dasd_t	DASD;		/* 16: DASD limit/usage info  F226941 */

		uint8	flag;		/* 1: */
		int8	nextindex;	/* 1: next free entry in stbl */
		int8	freecnt;	/* 1: free count */
		int8	freelist;	/* 1: freelist header */
	
		ino_t	idotdot;	/* 4: parent inode number */

		int8	stbl[8];	/* 8: sorted entry index table */
	} header;			/* (32) */

	dtslot_t	slot[9];
} dtroot_t;

#define DTROOTMAXSLOT	9

#define	dtEmpty(IP)	( ((dtroot_t *)&(IP)->i_btroot)->header.nextindex == 0 ? 0 : EACCES )


/*
 *	directory regular page:
 *
 *	entry slot array of 32 byte slot
 *
 * sorted entry slot index table (stbl):
 * contiguous slots at slot specified by stblindex,
 * 1-byte per entry
 *   512 byte block:  16 entry tbl (1 slot)
 *  1024 byte block:  32 entry tbl (1 slot)
 *  2048 byte block:  64 entry tbl (2 slot)
 *  4096 byte block: 128 entry tbl (4 slot)
 *
 * data area:
 *   512 byte block:  16 - 2 =  14 slot
 *  1024 byte block:  32 - 2 =  30 slot
 *  2048 byte block:  64 - 3 =  61 slot
 *  4096 byte block: 128 - 5 = 123 slot
 *
 * N.B. index is 0-based; index fields refer to slot index
 * except nextindex which refers to entry index in stbl;
 * end of entry stot list or freelist is marked with -1.
 */
typedef union {
	struct {
		int64	next;		/* 8: next sibling */
		int64	prev;		/* 8: previous sibling */

		uint8	flag;		/* 1: */
		int8	nextindex;	/* 1: next entry index in stbl */
		int8	freecnt;	/* 1: */
		int8	freelist;	/* 1: slot index of head of freelist */

		uint8	maxslot;	/* 1: number of slots in page slot[] */
		int8	stblindex;	/* 1: slot index of start of stbl */
		uint8	rsrvd[2];	/* 2: */

		pxd_t	self;		/* 8: self pxd */
	} header;			/* (32) */

	dtslot_t	slot[128];
} dtpage_t;

#define DTPAGEMAXSLOT        128

#define DT8THPGNODEBYTES     512
#define DT8THPGNODETSLOTS      1
#define DT8THPGNODESLOTS      16

#define DTQTRPGNODEBYTES    1024
#define DTQTRPGNODETSLOTS      1
#define DTQTRPGNODESLOTS      32

#define DTHALFPGNODEBYTES   2048
#define DTHALFPGNODETSLOTS     2
#define DTHALFPGNODESLOTS     64

#define DTFULLPGNODEBYTES   4096
#define DTFULLPGNODETSLOTS     4
#define DTFULLPGNODESLOTS    128

#define DTENTRYSTART	1

/* get sorted entry table of the page */
#define DT_GETSTBL(p) ( ((p)->header.flag & BT_ROOT) ?\
	((dtroot_t *)(p))->header.stbl : \
	(int8 *)&(p)->slot[(p)->header.stblindex] )


#ifndef	_JFS_UTILITY
#ifdef	_JFS_FYI
/*
 *	struct dirent (external directory entry format)
 *
 * external format of directory entry used by readdir() is defined by
 * struct dirent in <dirent.h>.
 * struct dirent requires name (d_name) terminated with NULL.
 *
 * XPV4.2 include file serial number (d_ino, i.e., inode number) field.
 * also, it restricts that the number of 'bytes' (not characters)
 * preceding the terminating null byte will not exceed NAME_MAX.
 *
 * readdir() rounds up the struct dirent to 4-byte boundary.
 * compute struct dirent size to cover namlen with terminating NULL
 * rounded up to 4-byte boundary:
 */

/*
 *	directory entry [POSIX]
 */
typedef struct {
	uint32	d_offset;	/* 4: offset of end of this entry
				 *    in the directory stream
				 */
	ino_t	d_ino;		/* 4: inode number */
	uint16	d_reclen;	/* 2: length of entry */
	uint16	d_namlen;	/* 2: length of name (w/o NULL) */
	uint8	d_name[JFS_NAME_MAX+1];	/* NULL-terminated filename
					 * rounded up to 4-byte boundary
					 */
} dirent_t;
#endif	/* _JFS_FYI */

#define DIRENTSIZ(namlen) \
    ( (sizeof(struct dirent) - 2*(JFS_NAME_MAX+1) + 2*((namlen)+1) + 3) &~ 3 )


/*
 *	external declarations
 */
void dtInitRoot(
	int32	tid,
	inode_t	*ip,
	ino_t	idotdot);

int32 dtSearch(
	inode_t		*ip,
	component_t	*key,
	ino_t		*data,
	btstack_t	*btstack,
	uint32		flag);

int32 dtInsert(
	int32		tid,
	inode_t		*ip,
	component_t 	*key,
	ino_t		*ino,
	btstack_t	*btstack);

int32 dtDelete(
	int32		tid,
	inode_t		*ip,
	component_t	*key,
	ino_t		*data,
	uint32 		flag);

int32 dtRelocate(int32	tid,
        inode_t		*ip,
	int64		lmxaddr,
        pxd_t           *opxd,
        int64           nxaddr);


int32 dtModify(
	int32		tid,
	inode_t		*ip,
	component_t	*key,
	ino_t		*orig_ino,
	ino_t		new_ino,
	uint32 		flag);

int32 dtRead(
	inode_t		*ip,
	int32		*offset,
	int32		ubytes,
	int32		*tbytes,
	struct dirent	*tdp);

#ifdef	_JFS_OS2
int32 dtFind(
        inode_t		*ip,
        component_t	*pattern,
	UniChar		*lastmatch,
        int32		*offset,
        uint32		*count,
        int32		ubytes,
        int32		*tbytes,
	struct dirent	*tdp);

int32 dtChangeCase(
	int32		tid,
	inode_t		*ip,
	component_t	*key,
	ino_t		*inum,
	uint32		flag);

int32 dtFindInum(						// F226941
        inode_t		*ip,					// F226941
        ino_t		inum,					// F226941
	component_t	*name);					// F226941
#endif	/* _JFS_OS2 */

#ifdef  _JFS_DEBUG
int32 dtDisplayTree(
        inode_t         *ip);

int32 dtDisplayPage(
        inode_t         *ip,
        int64           bn,
        dtpage_t        *p);
#endif  /* _JFS_DEBUG */
#endif  /* _JFS_UTILITY */

#endif /* !_H_JFS_DTREE */
