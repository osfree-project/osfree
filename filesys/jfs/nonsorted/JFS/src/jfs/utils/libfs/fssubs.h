/* $Id: fssubs.h,v 1.1.1.1 2003/05/21 13:41:50 pasha Exp $ */

/*
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
 *	fssubs.h
 *
 */

#ifndef _H_UJFS_FSSUBS
#define _H_UJFS_FSSUBS

#define	PAGESIZE	4096

/*
 * logical volume parameter
 */
extern	HFILE	LVHandle;
typedef struct {
	int64	LVSize;		// LV size in pbsize
	int32	pbsize;		// device block/sector size
	int32	l2pbsize;	// log2 of pbsize
	uint8	LVNumber;	// vpfsi.vpi_drive
} LV_t;

extern LV_t 	LVMount;
extern LV_t	*lvMount;


/*
 * file system parameter
 */
typedef struct {
	int64	FSSize;		// FS size in bsize

	int32	bsize;		// FS logical block size in byte
	int32	l2bsize;

	int32	l2bfactor;	// log2 (bsize/pbsize)

	int32	nbperpage;
	int32	l2nbperpage;

	int32	AGSize;		// FS AG size in bsize
	int32	nAG;		// number of AGs

	int64	LOGSize;	// log size in bsize
} FS_t;

extern FS_t	FSMount;
extern FS_t	*fsMount;


/*
 * block allocation map
 */
extern dinode_t	DIBMap;
extern dinode_t	*diBMap;

typedef struct {
	dbmap_t		ctl;
	dmapctl_t	l2;
	dmapctl_t	l1;
	dmapctl_t	l0;
	/* dmap_t	bag; */
} BMap_t;
extern  BMap_t	BMap;
extern  BMap_t	*bMap;


/*
 * inode allocation map
 */
extern dinode_t	DIIMap;
extern dinode_t	*diIMap;

typedef struct {
	dinomap_t	ctl;
	/* iag_t	iag; */
} IMap_t;
extern IMap_t	IMap;
extern IMap_t	*iMap;

/* imap xtree sequential read control */
extern int32	IAGNumber;
extern int64	IMapXtreeLMLeaf;	/* imap xtree leftmost leaf xaddr */
extern xtpage_t	IMapXtreeLeaf;  /* imap xtree leaf buffer */
typedef struct {
	xtpage_t	*leaf;	/* current imap leaf xtpage under scan */
	int32	index;	/* current xad entry index in iMapXtreeLeaf */ 
	int32	page;	/* iag number to read within current imap extent*/ 
} IMapXtree_t;
extern IMapXtree_t	IMapXtree;


/*
 *	buffer pool
 */
typedef struct {
	char  page[PAGESIZE];
} page_t; 

typedef struct bufhdr {
	struct bufhdr	*b_next;	/* 4: */
	char	*b_data;		/* 4: */
} buf_t;				/* (8) */

	
/*
 * function prototypes
 */
int32 openLV(char *LVName);
int32 openFS(void);
void closeFS(void);
int32 readBMapGCP(BMap_t *bMap);
int32 readBMapLCP(int64 bn, int32 level, dmapctl_t *lcp);
int32 readBMap(int64 start_dmap, int32 ndmaps, dmap_t *dmap);
int32 readIMapGCPSequential(IMap_t *iMap, iag_t *iag);
int32 readIMapSequential(iag_t *iag);
int32 xtLookup(dinode_t *dip, int64 xoff, int64	 *xaddr, int32 *xlen, 
	uint32 flag);
int32 xtLMLeaf(dinode_t *dip, xtpage_t *xpp);
int32 fscntl(uint32 cmd, void *pList, ULONG *pListLen, 
	void *pData, ULONG *pDataLen);
int32 pRead(FS_t *fsMount, int64 xaddr, int32 xlen, void *p);
int32 bRead(FS_t *fsMount, int64 xaddr, int32 xlen, buf_t **bpp);
int32 bRawRead(uint32 LVHandle, int64 off, int32 len, buf_t **bpp);
void bRelease(buf_t *bp);

#endif	/* _H_UJFS_FSSUBS */
