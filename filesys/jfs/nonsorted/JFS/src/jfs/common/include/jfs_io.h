/* $Id: jfs_io.h,v 1.1.1.1 2003/05/21 13:35:52 pasha Exp $ */

/* static char *SCCSID = "@(#)1.6  7/30/98 14:06:58 src/jfs/common/include/jfs_io.h, sysjfs, w45.fs32, 990417.1";
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
 *
 *	jfs_io.h: platform dependent i/o
 */

#ifndef _H_JFS_IO
#define _H_JFS_IO

#ifdef	_JFS_OS2
#include <strat2.h>
#endif	/* _JFS_OS2 */

/*
 *	platform dependent i/o device buffer header (iobuf_t)
 */

#ifdef _JFS_OS2
/*
 *	extended device driver request list
 *
 * for header file, ref. os2/strat2.h;
 * for specification, ref. OS/2 Storage Device Driver Reference, 
 * Appendix C. Extended Device Driver Interface Specification.
 *
 * note: max m-request n-coalesce 16 + 20 + ((52 + 8 * n) * m)
 */ 
typedef struct iobuf {
	uint32		b_flags;	/* 4: flag word (see jfs_bufmgr.h) */
	void		*b_jfsbp;	/* 4: jfs buffer bound */
	struct iobuf	*b_next;	/* 4: iodonelist/freelist */
	void		(*b_iodone)(struct iobuf *);	/* 4: */
	Req_List_Header	bio_rlh;	/* 20 request list header */
	PB_Read_Write	bio_rh;		/* 52 request header */
	SG_Descriptor	bio_sg[1];	/* 8: scatter/gather list */
} iobuf_t;				/* (88+8*n) */

/* I/O buffer header <-> request list pointer */
#define	RLH_OFFSET	((int32)&((iobuf_t *)0)->bio_rlh)

/*
 * b_flags (ref. AIX sys/buf.h)
 */
#define B_WRITE 	0x0000	/* write pseudo-flag */
#define B_READ		0x0001	/* read when I/O occurs */
#define B_DONE		0x0002	/* I/O complete */
#define B_ERROR 	0x0004	/* error detected */
#define B_BUSY		0x0008	/* in use or I/O in progress */
#define B_INFLIGHT	0x0020	/* this request is in-flight */
#define B_AGE		0x0080	/* put at head of freelist when released */
#define B_ASYNC 	0x0100	/* don't wait for I/O completion */
#define B_DELWRI	0x0200	/* don't write till block is reassigned */
#define B_NOHIDE	0x0400	/* don't hide data pages during dma xfer */
#define B_STALE 	0x0800	/* data in buffer is no longer valid */
#define B_MORE_DONE	0x1000	/* more buffers to be processed */

#define B_PFSTORE 	0x2000	/* store operation */
#define B_PFPROT 	0x4000	/* protection violation */
#define B_SPLIT 	0x8000	/* ok to enable split read/write */
#define B_PFEOF 	0x10000	/* check for reference beyond end-of-file */
#define B_MPSAFE	0x40000 /* Invoker of strategy() is MP safe */
#define B_MPSAFE_INITIAL 0x80000 /* devstrat() converts B_MPSAFE into */
				/*	this flag */
#define B_COMPACTED	0x100000 /* comapcted coalesce list */
#define	B_DONTUNPIN	0x200000 /* pin() failed - don't unpin buf */

#define B_DIRTY		B_DELWRI	/* modified - write when recycled */
#define B_DISCARD	0x01000000	/* invalidate at i/o completion */

/*
 *	Macro to call Strategy 3 routine
 */
#define CallStrat3(StratRoutine, Packet) \
	(*(StratRoutine))(Packet)

void CallStrat2(void * _Seg16, Req_List_Header *);

extern void APIENTRY jfsIODone(Req_List_Header *rlhp);
#endif /* _JFS_OS2 */

#endif  /* _H_JFS_IO */
