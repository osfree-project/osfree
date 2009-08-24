/* $Id: jfs_bufmgr.h,v 1.1.1.1 2003/05/21 13:35:40 pasha Exp $ */

/* static char *SCCSID = "@(#)1.13  7/30/98 14:04:35 src/jfs/common/include/jfs_bufmgr.h, sysjfs, w45.fs32, 990417.1";
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
#ifndef _H_JFS_BUFMGR
#define _H_JFS_BUFMGR

/*
 *	jfs_bufmgr.h: buffer manager 
 *
 */

#include "jfs_io.h"
#include "jfs_cachemgr.h"

/*
 *		buffer cache buffer header
 *
 * N.B. cbuf_t, jbuf_t, and lbuf_t shares common fields, and
 *	cachelist/freelist;
 *
 * A buffer header consists of
 *  platform/device driver independent area and
 *  platform/device driver dependent area defined by host 
 *  device driver interface (platform dependent area 
 *  may contain platform independent information).
 *
 * N.B. buffer header size must be multiple of 8-byte, and
 * aligned on 8-byte boundary (any 8-byte field(s) must be aligned
 * on 8-byte boundary).
 *
 * Block driver bottom halves run without access to user process context, 
 * and are not allowed to page fault.
 */
#define	jbuf_t	cbuf_t
#define	b_bdata	cm_cdata

/*
 * j_xflag
 */
#define	B_BUFFER	0x8000	/* type = buffer */
#define	bmREAD_TYPE	0x7000	/* adress type flag */
#define	bmREAD_PAGE	0x4000	/* per file block */
#define	bmREAD_BLOCK	0x2000	/* per file system block */
#define	bmREAD_RAW	0x1000	/* per device physical block */
#define	B_NOHOMEOK	0x0800	/* do not pageout to home */
#define	B_SYNCPT	0x0400	/* pageout at next commit */
#define B_IO		0x0200	/* busy - I/O in progress */
#define B_INUSE		0x0100	/* busy - in use (ckecked out) */

/* bmAssign() by bmRead() flag */
#define bmREAD_ASSIGN	0x10000000	/* assign from read */

/* bmLazyWrite() force pageout flag */
#define bmWRITE_FORCE	0x00000001	/* COMMIT_FORCE */ 
#define bmWRITE_FLUSH	0x00000002	/* COMMIT_FLUSH */

/* i_biowait */
#define B_IOWAIT	0x01		/* fileset pageout control */

/*
 *	external declarations
 */
int32 bmInit(void);

int32 bmShutdown();

jbuf_t *bmAssign(
	register struct inode	*ip, 
	register int64		lblkno,
	register int64		pblkno,
	register int32		size,
	register uint32		flag);

int32 bmRead(
	register struct inode	*ip, 
	register int64		lblkno, 
	register int32		size,
	register uint32		flag,
	register jbuf_t		**bpp);

int32 bmWrite(
	register jbuf_t	*bp);

void bmLazyWrite(
	register jbuf_t	*bp,
	uint32		flag);

void bmAsyncWrite(
	register jbuf_t	*bp);

void bmRelease(
	register jbuf_t	*bp);

void bmSetXD(
	jbuf_t		*bp,
	int64		blkno,
	int32		size);

jbuf_t *bmAllocate(void);

void bmFree(
	jbuf_t	*bp);

int32 bmFlush(
	register jbuf_t	*bp);

void bmInvalidate(
	register jbuf_t	*bp);

int32 bmInodeFlush(
	struct inode	*ip);

int32 bmInodeWrite(
	struct inode	*ip);

void bmInodeInvalidate(
	register struct inode	*ip);

void bmExtentInvalidate(
	struct inode		*ip,
	int64		xaddr,
	int32		xlen);

void bmSync(
	struct inode	*ipmnt);

void bmUnmountSync(
	register struct inode	*ip);

void bmSanity(void);

#endif /* _H_JFS_BUFMGR */
