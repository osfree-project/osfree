/* $Id: jfs_xtree.h,v 1.1 2000/04/21 10:57:59 ktk Exp $ */

/* static char *SCCSID = "@(#)1.11  8/9/99 04:21:59 src/jfs/common/include/jfs_xtree.h, sysjfs, w45.fs32, currbld";
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

#ifndef _H_JFS_XTREE
#define _H_JFS_XTREE

/*
 *      jfs_xtree.h: extent allocation descriptor B+-tree manager
 */

#include "jfs_btree.h"


/*
 *      extent allocation descriptor (xad)
 */
typedef struct xad {
        unsigned        flag:8;         /* 1: flag */
        unsigned        rsvrd:16;       /* 2: reserved */
        unsigned        off1:8;         /* 1: offset in unit of fsblksize */
        uint32          off2;           /* 4: offset in unit of fsblksize */
        unsigned        len:24;         /* 3: length in unit of fsblksize */
        unsigned        addr1:8;        /* 1: address in unit of fsblksize */
        uint32          addr2;          /* 4: address in unit of fsblksize */
} xad_t;                                /* (16) */

#define MAXXLEN         ((1 << 24) - 1)

#define XTSLOTSIZE      16
#define L2XTSLOTSIZE    4

/* xad_t field construction */
#define XADoffset(xad, offset64)\
{\
        (xad)->off1 = ((uint64)offset64) >> 32;\
        (xad)->off2 = (offset64) & 0xffffffff;\
}
#define XADaddress(xad, address64)\
{\
        (xad)->addr1 = ((uint64)address64) >> 32;\
        (xad)->addr2 = (address64) & 0xffffffff;\
}
#define XADlength(xad, length32)        (xad)->len = length32

/* xad_t field extraction */
#define offsetXAD(xad)\
        ( ((int64)((xad)->off1)) << 32 | (xad)->off2 )
#define addressXAD(xad)\
        ( ((int64)((xad)->addr1)) << 32 | (xad)->addr2 )
#define lengthXAD(xad)  ( (xad)->len )

/* xad list */
typedef struct {
        int16   maxnxad;
        int16   nxad;
        xad_t   *xad;
} xadlist_t;

/* xad_t flags */
#define XAD_NEW         0x01    /* new */
#define XAD_EXTENDED    0x02    /* extended */
#define XAD_COMPRESSED  0x04    /* compressed with recorded length */
#define XAD_NOTRECORDED 0x08    /* allocated but not recorded */
#define XAD_COW         0x10    /* copy-on-write */


/* possible values for maxentry */
#define XTROOTINITSLOT  10
#define XTROOTMAXSLOT   18
#define XTPAGEMAXSLOT   256
#define XTENTRYSTART    2

/*
 *      xtree page:
 */
typedef union {
        struct xtheader {
                int64           next;           /* 8: */
                int64           prev;           /* 8: */

                uint8           flag;           /* 1: */
                uint8           rsrvd1;         /* 1: */
                int16           nextindex;      /* 2: next index = number of entries */
                int16           maxentry;       /* 2: max number of entries */
                int16           rsrvd2;         /* 2: */

                pxd_t           self;           /* 8: self */
        } header;                               /* (32) */

        xad_t           xad[XTPAGEMAXSLOT];             /* 16 * maxentry: xad array */
} xtpage_t;

#ifndef _JFS_UTILITY
/*
 *      external declaration
 */
extern int32 xtLookup(
        inode_t         *ip,
        int64           lstart,
        int32           llen,
        uint8           *pflag,
        int64           *paddr,
        int32           *plen,
        uint32          flag);

extern xtLookupPageList(
        inode_t         *ip,
        int64           lstart,
        int32           llen,
        pxdlist_t       *pxdlist,
        uint32          flag);

extern int32 xtLookupList(
        inode_t         *ip,
        lxdlist_t       *lxdlist,
        xadlist_t       *xadlist,
        uint32          flag);

void xtInitRoot(
        int32           tid,
        inode_t         *ip);

extern int32 xtInsert(
        int32           tid,
        inode_t         *ip,
        uint8           xflag,
        int64           xoff,
        int32           xlen,
        int64           *xaddrp,
        uint32          flag);

extern int32 xtExtend(
        int32           tid,
        inode_t         *ip,
        int64           xoff,
        int32           xlen,
        uint32          flag);

extern int32 xtTailgate(
        int32           tid,
        inode_t         *ip,
        int64           xoff,
        int32           xlen,
        int64           xaddr,
        uint32          flag);

extern xtUpdate(
        int32           tid,
        inode_t         *ip,
        struct xad      *nxad);

extern int32 xtDelete(
        int32           tid,
        inode_t         *ip,
        int64           xoff,
        int32           xlen,
        uint32          flag);

extern int32 xtTruncate(
        int32           tid,
        inode_t         *ip,
        int64           newsize,
        int32           type);

extern int32 xtRelocate(int32   tid,
        inode_t         *ip,
        xad_t           *oxad,
        int64           nxaddr,
        int32           xtype);

extern int32 xtAppend(
        int32           tid,    /* transaction id */
        inode_t         *ip,
        uint8           xflag,
        int64           xoff,
        int32           maxblocks,              /* @GD1 */
        int32           *xlenp, /* (in/out) */
        int64           *xaddrp,/* (in/out) */
        uint32          flag);

#ifdef  _JFS_DEBUG
int32 xtDisplayTree(
        inode_t         *ip);

int32 xtDisplayPage(
        inode_t         *ip,
        int64           bn,
        xtpage_t        *p);
#endif  /* _JFS_DEBUG */
#endif  /* _JFS_UTILITY */

#endif /* !_H_JFS_XTREE */
