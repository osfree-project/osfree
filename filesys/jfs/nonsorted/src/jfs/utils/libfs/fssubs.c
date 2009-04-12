/* $Id: fssubs.c,v 1.2 2004/03/21 02:43:16 pasha Exp $ */

static char *SCCSID = "@(#)1.6  12/4/98 12:51:02 src/jfs/utils/libfs/fssubs.c, jfslib, w45.fs32, 990417.1";
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
 *
 *	fssubs.c: fs utility subroutines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSMEMMGR

#include <os2.h>

#include "jfs_types.h"
#include "jfs_aixisms.h"
#include "jfs_superblock.h"
#include "jfs_filsys.h"
#include "jfs_inode.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_cntl.h"
#include "jfs_defragfs.h"
#include "jfs_debug.h"

#include "devices.h"
#include "debug.h"

#include "fssubs.h"
#include "utilsubs.h"
#include "message.h"

/* forward references */
static void bAllocate(void);

/* Global Data */

int32	Fail_Quietly = 0;	// If set, don't report errors

HFILE	LVHandle;
LV_t 	LVMount;	// logical volume descriptor
LV_t	*lvMount = &LVMount;

dinode_t	DIBMap;		// block allocation map inode
dinode_t	*diBMap = &DIBMap;

dinode_t	DIIMap;		// inode allocation map inode
dinode_t	*diIMap = &DIIMap;

int32	IAGNumber = 0;

/*
 *	file system under defragfs
 */
FS_t	FSMount;	// file system descriptor
FS_t	*fsMount = &FSMount;

/* imap xtree sequential read control */
int64	IMapXtreeLMLeaf;	/* imap xtree leftmost leaf */
xtpage_t	IMapXtreeLeaf;  /* imap xtree leaf buffer */
IMapXtree_t	IMapXtree = {&IMapXtreeLeaf, XTENTRYSTART, 1};

/*
 *	openLV()
 */
int32 openLV(char	*LVName)
{
	int32	rc = 0;
	ULONG ulO = 0;
	BYTE fsbuf[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
	PFSQBUFFER2 pfsbuf = (PFSQBUFFER2) fsbuf;
	ULONG  cbuf = sizeof(fsbuf);
	PBYTE fsdname = NULL;
	int32	pbsize;
	int64	LVSize, hiddenLVSize;

	/*
	 *	validate and retrieve device parameters:
	 */
	/* validate and convert LVName to vpfsi.vpi_drive */
	if (LVName[0] >= 'a' && LVName[0] <= 'z')
		LVMount.LVNumber = LVName[0] - 'a';
	else if (LVName[0] >= 'A' && LVName[0] <= 'Z')
		LVMount.LVNumber = LVName[0] - 'A';
	else
	{
		message_user(MSG_OSO_CANT_FIND_DRIVE, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		return -1;
	}

	/* verify that the device is a mounted JFS file system */
	if (rc = DosQueryFSAttach(LVName, ulO, FSAIL_QUERYNAME, pfsbuf, &cbuf))
	{
		if (!Fail_Quietly)
			message_user(MSG_OSO_CANT_FIND_DRIVE, NULL, 0,
				     STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		return rc;
	}

	fsdname = pfsbuf->szName + pfsbuf->cbName + 1;
	if (strcmp(fsdname, "JFS") != 0 && strcmp(fsdname, "jfs") != 0)
	{
		if (!Fail_Quietly)
			message_user(MSG_JFS_NOT_JFS, NULL, 0, STDOUT_CODE,
				     NO_RESPONSE, JFS_MSG);
		return -1;
	}

	/* open the device and get its physical block size */
	if (rc = ujfs_open_device(LVName, &LVHandle, &pbsize, READONLY))
	{
		if (!Fail_Quietly)
			message_user(MSG_OSO_CANT_OPEN, NULL, 0, STDOUT_CODE,
				     NO_RESPONSE, OSO_MSG);
		return rc;
	}

	LVMount.pbsize = pbsize;
	LVMount.l2pbsize = log2shift(pbsize);

	/* get logical volume size */
	if (rc = ujfs_get_dev_size(LVHandle, &LVSize, &hiddenLVSize))
	{
		message_user(MSG_OSO_ERR_ACCESSING_DISK, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		ujfs_close(LVHandle);
		return rc;
	}

	LVMount.LVSize = LVSize >> LVMount.l2pbsize;

	return rc;
}
	

/*
 *	openFS() 
 */ 
int32 openFS(void)
{
	int32	rc;
	struct superblock	*sb;
	buf_t	*bp;

	/*
	 *	validate and retrieve fs parameters from superblock
	 */
	/* try to read the primary superblock */
	rc = bRawRead(LVHandle, (int64)SUPER1_OFF, (int32)PAGESIZE, &bp);
	if (rc != 0)
	{
		/* try to read the secondary superblock */
		rc = bRawRead(LVHandle, (int64)SUPER2_OFF, (int32)PAGESIZE, &bp);
		if (rc != 0)
		{
			return rc;
		}
	}

	sb = (struct superblock *)bp->b_data;

	/* check magic/version number */
	if (strncmp(sb->s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC))
	 || (sb->s_version != JFS_VERSION))
	{
		message_user(MSG_JFS_BAD_SUPERBLOCK, NULL, 0, STDOUT_CODE, NO_RESPONSE, JFS_MSG);
		return -1;
	}

	if (sb->s_state & FM_DIRTY)
	{
		message_user(MSG_JFS_DIRTY, NULL, 0, STDOUT_CODE, NO_RESPONSE, JFS_MSG);
		return -1;
	}

	/* assert(lvMount->pbsize == sb->s_pbsize); */
	/* assert(lvMount->l2pbsize == sb->s_l2pbsize); */

	fsMount->bsize = sb->s_bsize;
	fsMount->l2bsize = sb->s_l2bsize;
	fsMount->l2bfactor = sb->s_l2bfactor;
	fsMount->nbperpage = PAGESIZE >> fsMount->l2bsize;
	fsMount->l2nbperpage = log2shift(fsMount->nbperpage);

	fsMount->FSSize = sb->s_size >> sb->s_l2bfactor;
	
	fsMount->AGSize = sb->s_agsize;

	bRelease(bp);

	return rc;
}


/*
 *	closeFS()
 */
void closeFS(void)
{
	ujfs_close(LVHandle);
}


/*
 * NAME:	readBMapGCP()
 *
 * FUNCTION:	read the bmap global control page and 
 *		the initial level L2, L1.0 and L0.0 control pages;
 *
 * note: bmap file structure is satic during defragfs()
 * (defragfs() and extendfs() are mutually exclusive operation);
 */
int32 readBMapGCP(
	BMap_t	*bMap)
{
	int32	rc;
	int64	xaddr;
	int32	xlen;
	dbmap_t	*bcp;

	/* first 4 pages are made contiguous by mkfs() */
	xlen = 4 << fsMount->l2nbperpage;
        rc = xtLookup(diBMap, (int64)0, &xaddr, &xlen, 0);
        if ( rc )
                return rc;

	rc = pRead(fsMount, xaddr, xlen, (void *)bMap);
	if ( rc )
	{
		return rc;
	}

	bcp = (dbmap_t *)bMap;
	fsMount->nAG = bcp->dn_numag;

	return 0;
}

		
/*
 * NAME:	readBMapLCP()
 *
 * FUNCTION:  	read level control page of given level for given block number;
 */
int32 readBMapLCP(
	int64	bn,
	int32	level,
	dmapctl_t *lcp)

{
	int32	rc = 0;
	int64	xoff, xaddr;
	int32	xlen;

	switch (level)
	{
	case 0:
		xoff = BLKTOL0(bn, fsMount->l2nbperpage);
		break;
	case 1:
		xoff = BLKTOL1(bn, fsMount->l2nbperpage);
		break;
	case 2:
		xoff = fsMount->nbperpage;
		break;
	}

	xlen = fsMount->nbperpage;
	rc = xtLookup(diBMap, xoff, &xaddr, &xlen, 0);		
	if ( rc != 0 )
		return rc;

	rc = pRead(fsMount, xaddr, xlen, (void *)lcp);
	if ( rc )
	{
		return rc;
	}
			
	return 0;
}


/*
 * NAME:        readBMap()
 *
 * FUNCTION:	Read ndmaps dmap pages starting from the start_dmap.    
 *
 * note: bmap file structure is satic during defragfs()
 * (defragfs() and extendfs() are mutually exclusive operation);
 */
int32 readBMap(
	int64	start_dmap,   /* the dmap number to start read */
	int32	ndmaps,     /* number of dmap pages read */
	dmap_t	*dmap)	    /* buffer pointer */
{
        int32	rc;
	int64	xaddr;	
	int64	xoff;
	int32	xlen0, xlen;

	/* convert dmap number to xoffset */
	xaddr = start_dmap << L2BPERDMAP;
        xoff = BLKTODMAP(xaddr, fsMount->l2nbperpage);

	xlen0 = ndmaps << fsMount->l2nbperpage;
	xlen = fsMount->nbperpage;
	for ( ; xlen0 > 0; xlen0 -= xlen)  
        {
        	rc = xtLookup(diBMap, xoff, &xaddr, &xlen, 0);
        	if ( rc )
                	return rc;

		rc = pRead(fsMount, xaddr, xlen, (void *)dmap);
		if ( rc )
		{
			return rc;
		}

		xoff += xlen;
		dmap = (dmap_t *)((char *)dmap + PAGESIZE);
	}

	return 0;
}


/*
 * NAME:	readIMapGCPSequential()
 *
 * FUNCTION:	read the imap global control page.
 */
int32 readIMapGCPSequential(
	IMap_t	*iMap,
	iag_t	*iag)
{
	int32	rc;
	xtpage_t	*p;
	int64	xaddr;

	/* read in leftmost leaft xtpage */
	rc = xtLMLeaf(diIMap, &IMapXtreeLeaf); 
	if ( rc != 0 )
		return rc;

	p = &IMapXtreeLeaf;
	IMapXtreeLMLeaf = addressPXD(&p->header.self);

	/* read IMap control page */
	xaddr = addressXAD(&p->xad[XTENTRYSTART]);
	rc = pRead(fsMount, xaddr, fsMount->nbperpage, (void *)iMap);

	/* init for start of bitmap page read */
	IAGNumber = 0;
	IMapXtree.index = XTENTRYSTART;
	IMapXtree.page = 1;	/* skip for global control page */
	iag->iagnum = -1;

	return rc;
}


/*
 * NAME:	readIMapSequential()
 *
 * FUNCTION:	read one iag page at a time. 
 *
 * state variable:
 *	IMapXtree.leaf - current imap leaf xtpage under scan;
 *	IMapXtree.index - current xad entry index in iMapXtreeLeaf; 
 *	IMapXtree.page - iag number to read within current imap extent; 
 *
 * note: IMap pages may grow (but NOT freed) dynamically; 
 */
int32 readIMapSequential(
	iag_t	*iag)
{
	int32	rc;
	xtpage_t	*p;
	int64	xaddr;
	int32	xlen, delta;

	p = &IMapXtreeLeaf;

	while(1) 
	{
		/* continue with current leaf xtpage ? */
		if (IMapXtree.index < p->header.nextindex)
		{
			/* determine iag page extent */
			xaddr = addressXAD(&p->xad[IMapXtree.index]);
			xlen = lengthXAD(&p->xad[IMapXtree.index]);

			if (IMapXtree.page)
			{
				/* compute offset within current extent */
                                delta = IMapXtree.page << fsMount->l2nbperpage;
                                xaddr += delta;
                                xlen -= delta;
			}

			/* read a iag page */
			rc = pRead(fsMount, xaddr, fsMount->nbperpage,
				   (void *)iag);
			if ( rc != 0 )
			{
				return rc;
			}

			/*current extent has more iag */
			if ( xlen > fsMount->nbperpage ) 	
			{
				/* continue to read from current extent */
				IMapXtree.page++;
			}
			/*current extent has NO more iag */
			else 
			{
				IMapXtree.index++;
				IMapXtree.page = 0;
			}

			return 0;
		}

		/* read next/right sibling leaf xtpage */
		if (p->header.next != 0)
		{
			xaddr = p->header.next;

			/* init for next leaf xtpage */
			IMapXtree.index = XTENTRYSTART;
			IMapXtree.page = 0;
		}
		/*
		 * end of IMap file: cycle back to leftmost leaf
		 */
		else
		{
			/* init for start of bitmap page read */
			IAGNumber = 0;
			IMapXtree.index = XTENTRYSTART;
			IMapXtree.page = 1; /* skip for global control page */

			if (p->header.prev != 0)
				xaddr = IMapXtreeLMLeaf;
 			/* a single leaf xtree */
			else
				continue;
		}

		/* read next xtree leaf page */
		rc = pRead(fsMount, xaddr, fsMount->nbperpage,
			   (void *)&IMapXtreeLeaf);
		if ( rc != 0 )
		{
			return rc;
		}
	}
}


/*
 * NAME:        xtLMLeaf()
 *
 * FUNCTION:    read in leftmost leaf page of the xtree
 *              by traversing down leftmost path of xtree;
 */
int32 xtLMLeaf(
        dinode_t	*dip,	/* disk inode */
        xtpage_t	*pp)	/* pointer to leftmost leaf xtpage */
{
        int             rc;
        xtpage_t        *p;
        int64		xaddr;

        /* start from root in dinode */
        p = (xtpage_t *)&dip->di_btroot;
        /* is this page leaf ? */
        if (p->header.flag & BT_LEAF)
	{
		p->header.next = p->header.prev = 0;
		memcpy(pp, p, DISIZE);
                return 0;
	}

        /*
         * traverse down leftmost child node to the leftmost leaf of xtree
         */
        while (TRUE)
        {
                /* read in the leftmost child page */
		xaddr = addressXAD(&p->xad[XTENTRYSTART]);
		rc = pRead(fsMount, xaddr, fsMount->nbperpage, (void *)pp);
                if (rc)
		{
                        return rc;
		}

                /* is this page leaf ? */
                if (pp->header.flag & BT_LEAF)
                        return 0;
		else
                	p = pp;
        }
}


/*
 *	 xtree key/entry comparison: extent offset
 *
 * return:
 *	-1: k < start of extent
 *	 0: start_of_extent <= k <= end_of_extent
 *	 1: k > end_of_extent
 */
#define XT_CMP(CMP, K, X, OFFSET64)\
{\
	OFFSET64 = offsetXAD(X);\
	(CMP) = ((K) >= OFFSET64 + lengthXAD(X)) ? 1 :\
	      ((K) < OFFSET64) ? -1 : 0;\
}


/*
 *	xtLookup()
 *
 * function:	search for the xad entry covering specified offset.
 *
 * parameters:
 *	ip	- file object;
 *	xoff	- extent offset;
 *	cmpp	- comparison result:
 *	btstack	- traverse stack;
 *	flag	- search process flag (XT_INSERT);
 *
 * returns:
 *	btstack contains (bn, index) of search path traversed to the entry.
 *	*cmpp is set to result of comparison with the entry returned.
 *	the page containing the entry is pinned at exit.
 */
int32 xtLookup(
	dinode_t	*dip,
	int64		xoff,	/* offset of extent */
	int64		*xaddr,
	int32		*xlen,
	uint32		flag)
{
	int32		rc = 0;
	buf_t		*bp = NULL;	/* page buffer */
	xtpage_t	*p;	/* page */
	xad_t		*xad;
	int64		t64;
	int32		cmp = 1, base, index, lim;
	int32		t32;

	/*
	 *	search down tree from root:
	 *
	 * between two consecutive entries of <Ki, Pi> and <Kj, Pj> of
	 * internal page, child page Pi contains entry with k, Ki <= K < Kj.  
	 *
 	 * if entry with search key K is not found
 	 * internal page search find the entry with largest key Ki 
	 * less than K which point to the child page to search;
 	 * leaf page search find the entry with smallest key Kj 
	 * greater than K so that the returned index is the position of
	 * the entry to be shifted right for insertion of new entry.
	 * for empty tree, search key is greater than any key of the tree.
	 */

	/* start with inline root */
	p = (xtpage_t *)&dip->di_btroot;

xtSearchPage:
	lim = p->header.nextindex - XTENTRYSTART; 

	/*
	 * binary search with search key K on the current page
	 */
	for (base = XTENTRYSTART; lim; lim >>= 1)
	{
		index = base + (lim >> 1);

		XT_CMP(cmp, xoff, &p->xad[index], t64);
		if (cmp == 0)
		{
			/*
			 *	search hit
			 */
			/* search hit - leaf page:
			 * return the entry found
			 */
			if (p->header.flag & BT_LEAF)
			{
				/* save search result */
				xad = &p->xad[index];
				t32 = xoff - offsetXAD(xad);
				*xaddr = addressXAD(xad) + t32; 
				*xlen = MIN(*xlen, lengthXAD(xad) - t32);

				rc = cmp;
				goto out;
			}

			/* search hit - internal page:
			 * descend/search its child page 
			 */
			goto descend;
		}

		if (cmp > 0) {
			base = index + 1;
			--lim;
		}
	}

	/*
	 *	search miss
	 *
	 * base is the smallest index with key (Kj) greater than 
	 * search key (K) and may be zero or maxentry index.
	 */
	/*
	 * search miss - leaf page: 
	 *
	 * return location of entry (base) where new entry with
	 * search key K is to be inserted.
	 */
	if (p->header.flag & BT_LEAF)
	{
		rc = cmp;
		goto out;
	}

	/*
	 * search miss - non-leaf page:
	 *
	 * if base is non-zero, decrement base by one to get the parent
	 * entry of the child page to search.
	 */
	index = base ? base - 1 : base;

	/* 
	 * go down to child page
	 */
descend:
	/* get the child page block number */
	t64 = addressXAD(&p->xad[index]);

	/* release parent page */
	if (bp)
		bRelease(bp);

	/* read child page */
	bRead(fsMount, t64, fsMount->nbperpage, &bp);

	p = (xtpage_t *)bp->b_data;
	goto xtSearchPage;

out:
	/* release current page */
	if (bp)
		bRelease(bp);

	return rc;
}


/*
 * NAME:	fscntl()
 *
 * FUNCTION:	call fs control service;
 */
int32 fscntl(
	uint32	cmd,
	void	*pList,		/* parameter list */
	ULONG	*pListLen,	/* length of pList */
	void	*pData,		/* data area */
	ULONG	*pDataLen)	/* length of pData */
{
	int32	rc;

	rc = DosFSCtl(
		pData,		/* data area */
		*pDataLen,	/* in: length of pData */
		pDataLen,	/* in/out: returned pData length */
		pList,		/* parameter list */
		*pListLen,	/* in: length of pList */
		pListLen,	/* in/out: return pList length */
		cmd,		/* function code */
		"JFS",		/* route FSD name */
		-1,		/* file handle */
		FSCTL_FSDNAME);	/* method of route */

	return rc;
}


/*
 *	pRead()
 *
 * read into specified buffer;
 */
int32 pRead(
	FS_t	*fsMount,
	int64	xaddr,	/* in bsize */
	int32	xlen,	/* in bsize */
	void	*p)	/* buffer */
{
	int32	rc;
	int64	off;	/* offset in byte */
	int32	len;	/* length in byte */

	/* read in from disk */
	off = xaddr << fsMount->l2bsize;
	len = xlen << fsMount->l2bsize;
	if (rc = ujfs_rw_diskblocks(LVHandle, off, len, p, GET))
	{
		message_user(MSG_OSO_READ_ERROR, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
	}

	return rc;
}


/*
 *	buffer pool
 *	===========
 */
int32	NBUFFER = 8;
int32	nBuffer = 0;
buf_t	*bCachelist = NULL;

static void bAllocate(void)
{
	buf_t	*bp;
	char	*buffer;
	int32	i;

	/* allocate buffer header */
	if ((bp = malloc(NBUFFER * sizeof(buf_t))) == NULL)
	{
		message_user(MSG_OSO_NOT_ENOUGH_MEMORY, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		exit (ENOSPC);
	}

	/* allocate buffer pages */
	if ((buffer = malloc(NBUFFER * PAGESIZE)) == NULL)
	{
		message_user(MSG_OSO_NOT_ENOUGH_MEMORY, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		exit (ENOSPC);
	}

	/* insert buffer headers in lru/freelist */
	for (i = 0; i < NBUFFER; i++, bp++, buffer += PAGESIZE)
	{
		bp->b_data = buffer;
		bp->b_next = bCachelist;
		bCachelist = bp;
	}

	nBuffer += NBUFFER;
}

/*
 *	bRead()
 *
 * aloocate, read and return a buffer;
 */
int32 bRead(
	FS_t	*fsMount,
	int64	xaddr,	/* in bsize */
	int32	xlen,	/* in bsize */
	buf_t	**bpp)
{
	int32	rc;
	int64	off;	/* offset in byte */
	int32	len;	/* length in byte */
	buf_t	*bp;

	/* allocate buffer */
	if (bCachelist == NULL)
		bAllocate();
	bp = bCachelist;
	bCachelist = bp->b_next;

	/* read in from disk */
	off = xaddr << fsMount->l2bsize;
	len = xlen << fsMount->l2bsize;
	rc = ujfs_rw_diskblocks(LVHandle, off, len, (void *)bp->b_data, GET);
	if (rc == 0)
		*bpp = bp;
	else
	{
		message_user(MSG_OSO_READ_ERROR, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		bRelease(bp);
	}

	return rc;
}

int32 bRawRead(
	uint32	LVHandle,
	int64	off,	/* in byte */
	int32	len,	/* in byte */
	buf_t	**bpp)
{
	int32	rc;
	buf_t	*bp;

	/* allocate buffer */
	if (bCachelist == NULL)
		bAllocate();
	bp = bCachelist;
	bCachelist = bp->b_next;

	/* read in from disk */
	rc = ujfs_rw_diskblocks(LVHandle, off, len, (void *)bp->b_data, GET);
	if (rc == 0)
		*bpp = bp;
	else
	{
		message_user(MSG_OSO_READ_ERROR, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		bRelease(bp);
	}

	return rc;
}

void bRelease(
	buf_t	*bp)
{
	/* release buffer */
	bp->b_next = bCachelist;
	bCachelist = bp;
}
