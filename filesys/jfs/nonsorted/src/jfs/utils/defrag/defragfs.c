/* $Id: defragfs.c,v 1.2 2004/03/21 02:43:35 pasha Exp $ */

static char *SCCSID = "@(#)1.8  7/31/98 08:23:42 src/jfs/utils/defrag/defragfs.c, jfsdefrag, w45.fs32, 990417.1";
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
 *
 *	defragfs.c: defragment file system
 *
 * limitation: 
 * this version implements heuristics of simple compaction 
 * per allocation group or subset of it when it is large.
 * little of reports of pre- and post-defragmentation
 * are available.
 * (refer to design notes in defragfs.note for comments, etc.)
 *
 * further experiments for heuristics/policy of defragmentation
 * may be necessary, especially if file system/allocation group
 * size is large: i think most of building block services 
 * required/reusable to implement variations of defragmentation 
 * heuristics/policy in application level and ifs level are made
 * available now;
 *
 * restriction: defragmentation of file system is mutually
 * exclusive of extension of file system concurrently.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

/*
 *	debug control (ref. jfs_debug.h)
 */
#ifdef	_JFS_DEBUG
int32	jfsFYI = 1;	/* global enable FYI message */
int32	jfsALERT = 1;	/* global enable ALERT message */
int32	jfsERROR = 1;	/* global enable ERROR message */
#else	/* ! _JFS_DEBUG */
int32	jfsFYI = 0;	/* global disable FYI message */
int32	jfsALERT = 0;	/* global disable ALERT message */
int32	jfsERROR = 0;	/* global disable ERROR message */
#endif	/* _JFS_DEBUG */

/* command control */
#define DEFRAG           1
#define NODEFRAG_FREE    2
#define NODEFRAG_FULL    3
#define NODEFRAG_LONGRUN 4

int32	Query = FALSE;	/* just curious. */
int32	DoIt = TRUE;	/* go ahead. */

/* policy control */
#define XMOVESIZE	64	/* max size of extent to move + 1 */
#define BMAPSET		16	/* max # of bitmaps to be processed at a time */
#define	MIN_FREERATE	3	/* min free block % of AG to defragment */

/*
 *	device under defragfs
 */
char	LVName[4];	// device name

/*
 *	block allocation map
 */
BMap_t		BMap;		// block allocation map control page	
BMap_t		*bMap = &BMap;

/* bitmap page control */
int32	BMapPageNumber;
dmap_t	BMapSet[BMAPSET]; 

/*
 *	inode allocation map
 */
IMap_t		IMap;		// inode allocation map control page
IMap_t		*iMap = &IMap;

/* bitmap page control */
iag_t	iagbuf;  

/* inode extent buffer */
int32	IXNumber = 0;
int32	IXINumber = 0;	/* next inode index of ixbuf */
dinode_t ixbuf[INOSPEREXT];

/*
 *	relocation control
 */
int32	AGNumber;
int32	nBMapPerAG;	/* number of dmaps per AG  */
int32	nAGPerLCP;	/* number of ags covered by aglevel lcp */

int64	AGStart;	/* start aggr. block number of current AG  */
int64	AGEnd;		/* end aggr. blk number of current AG */
int32	AGBMapStart;	/* start bMap page number of current AG */
int32	AGBMapEnd;	/* end bMap page number of current AG */

int32	nBMapSet;	/* number of dmaps in a processing dmap set */
int64	BMapSetStart;	/* start block number of current BMapSet of AG */
int64	BMapSetEnd;	/* end block number of current BMapSet of AG */

int64	barrierStart;	/* start blk number of region in BMapSet to compact */
int64	barrierEnd;	/* end blk number of region in BMapSet to compact */
int64	barrier;	/* current barrier blk number in region in BMapSet */

/*
 *	inode-xad table
 */
/*
 *	ixad_t: movable extent allocation descriptor
 *
 *  movable extent descriptor specifying the object and xad.
 *  the length of the extent is specified by the ixadTbl list it
 *  is enqueued which is indexed by the extent length.
 *  for a regular file, the offset is used as search key.
 *  for a directory, the addr of left most dtpage of the level 
 *  is used to specify search domain.
 *
 *  When moving an extent, we must consider not to move
 *  extents that belong to one inode far apart.
 */
#define  INODE_TYPE	0x0000F000   /* IFREG or IFDIR */

typedef struct ixad {
	struct ixad	*next;  /* 4: next pointer */
	uint32	flag;		/* 4: INDISEAG, OUTSIDEAG, IFREG, IFDIR */
	int32	fileset;	/* 4: fileset number */
	uint32	inostamp;	/* 4: fileset timestamp */
	ino_t	inumber;	/* 4: inode number */
	uint32	gen;		/* 4: inode generation number */
	union {
		int64	xoff;	   /* @8: extent offset */
		int64	lmxaddr;   /* @8: extent address of leftmost dtpage 
				    * of the level whose idtentry covers
				    * the dtpage to be moved. 
				    * zero if dtpage is dtroot.
				    */
	} xkey;
	int64	xaddr;		/* @8: extent address */
} ixad_t;			/* (40) */

#define	f_xoff		xkey.xoff
#define	d_lmxaddr	xkey.lmxaddr

typedef struct ixadlist {
	ixad_t	*head;		/* 4: anchor of the list */
	int32	count;		/* 4: number of entry in the list */
} ixadlist_t;

ixadlist_t	ixadTbl[XMOVESIZE];
int32	nIxad = 0;
int32	nIxadPool = 0;
ixad_t	*ixadPool;      /* pointer to array (table) of ixad */	
ixad_t	*ixadHWM;
ixad_t	*ixadFreelist;  /* head of free list */

uint32	inoext_vector[2];

/*
 *	 inode traversal
 */
/* regular file */
int32	xtype;
int64	lmxaddr;
xtpage_t next_xtpg;  /* next xtpage to work on for the current inode */
xtpage_t *pnext_xtpg = NULL;
int32 nxtpg_inx = 0; /* the next index in next_xtpg to work on */

/* directory */
int64	lmpg_addr;
int32	lmxlen;
dtpage_t next_lmdtpg; /*next left-most dtpage to work on for the current inode*/
xtpage_t *pnext_lmdtpg = NULL;
dtpage_t next_dtpg;  /* next dtpage to work on for the current inode */
dtpage_t *pnext_dtpg = NULL;
int32 ndtpg_inx = 0; /* the next index in next_dtpg to work on */

/*
 *	statistics
 */
/* current AG working statistics */
int32	AGFreeRun[XMOVESIZE] = {0};
int32	AGAllocRun[XMOVESIZE] = {0};

/* FS cumulative summary */
int32	FSFreeRun[XMOVESIZE] = {0};
int32	FSAllocRun[XMOVESIZE] = {0};

/* Actions taken on AG's */
int32	AG_DEFRAG;
int32	AG_SKIP_FREE;
int32	AG_SKIP_FULL;
int32	AG_SKIP_LONG_RUN;

/* Count of free runs */
uint32	total_free_runs;

/*
 *	Parameter array for messages
 */
#define MAXPARMS	2
#define MAXSTR		80
char	*msg_parms[MAXPARMS];
char	msgstr[MAXSTR];

/*
 *	handy macro
 */
#define UZBIT	(0x80000000)
#define BUDSIZE(s,m)  (1 << ((s) -(m)))

/*
 * forward references
 */
static int32 defragfs(void);
static int32 examineAG(int32 agno, dmapctl_t *lcp, int32 *AGBMapStart, int32 *AGBMapEnd);
static int32 compactAG(int32 agno, int32 AGBMapStart, int32 AGBMapEnd, int32 nBMapSet);
static int32 buildIxadTbl(int32 agno, int64 barrierStart, int64 barrierEnd);
static int32 addXtree(dinode_t *dip, int64 barrierStart, int64 barrierEnd);
static int32 addDtree(dinode_t *dip, int64 barrierStart, int64 barrierEnd);
static ixad_t *allocIxad(int32 *rc);
static void insertIxad(int32 nblks, ixad_t *ixad);
static void washIxadTbl(int64 barrier);
static void purgeIxadTbl(void);
static int32 compactBMap(int32 BmapSetBMapStart, int32 bmx);
static int32 moveExtent(int32 BMapSetBMapStart, int64 fxaddr, int32 fxlen);
static int32 fscntlMove(int64 nxaddr, int32 xlen, ixad_t *ixad);
static void  dbFreeBMap(int32 BmapSetBMapStart, int64 blkno, int32 nblocks);
static int32 preamble(void);
static int32 getProvisions(void);
static int32 whatDoYouWant(int32 argc, char **argv);


/*
 * NAME:	main()
 *
 * FUNCTION:	Main routine of defragfs utility.
 */
main(
	int32	argc,
	char	**argv)
{
	int32	rc;

	/* parse command line arguments */
	if (rc = whatDoYouWant(argc, argv))
	{
		message_user(MSG_JFS_DEFRAGFS_USAGE, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
		goto out;
	}

	/* validate and open LV */
	if (rc = openLV(LVName))
		goto out;	

	/* open FS */
	if (rc = openFS())
		goto out;

	/* alloc/init resources */
	if (rc = getProvisions())
		goto out;

	/* init defragfs */
mainidea:
	if (rc = preamble())
		goto out;

	if (DoIt)
	{
		msg_parms[0] = LVName;
		message_user(MSG_JFS_DEFRAGMENTING, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
	}
	/*
	 * report pre-defragmentation statistics and 
	 * defragment the file system
	 */
	if (rc = defragfs())
		goto out;

	/* report post-defragmentation statistics */
	if (DoIt)
	{
		message_user(MSG_JFS_DEFRAGFS_COMPLETE, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
		DoIt = FALSE;
		goto mainidea;
	}

	/* close FS */
	closeFS();

out:
	return rc;
}


/*
 *	defragfs()
 */
static int32 defragfs(void)
{
	int32	rc = 0;
        dbmap_t	*gcp;	/* bmap global control page */
	dmapctl_t	*lcp; /* bmap level control page */
	int32	nBMap;
	int32	nAG;
	int32	nFullAG;
	int64	partialAG;
	int32	nodeperlcp;
	int32	lcpn;
	int32	i;
	
	gcp = (dbmap_t *)&BMap.ctl;

	/* number of full AGs */
	nFullAG = gcp->dn_mapsize >> gcp->dn_agl2size; 

	/* number of blocks in partial AG */
	partialAG = fsMount->FSSize & (gcp->dn_agsize - 1);

	/* Total AG's */
	nAG = nFullAG+(partialAG?1:0);

	/* number of nodes at agheigth level of summary tree */
	nodeperlcp = 1 << (L2LPERCTL - (gcp->dn_agheigth << 1)) ;

	/* number of AGs covered at agheight level of summary tree */
	nAGPerLCP = nodeperlcp / gcp->dn_agwidth ;

	/*
	 * init for lcp scan at aglevel
	 */
	/* lcp at ag level */
	switch (gcp->dn_aglevel)
	{
	case 0:
		lcp = (dmapctl_t *)&BMap.l0;
		break;
	case 1:
		lcp = (dmapctl_t *)&BMap.l1;
		break;
	case 2:
		lcp = (dmapctl_t *)&BMap.l2;
		break;
	}

	/* current level control page number at aglevel */
	lcpn = 0;

jEVENT(0,("defragfs: nFullAG:%d partialAG:%lld nAGPerLCP:%d\n", 
	nFullAG, partialAG, nAGPerLCP));

	/*
	 *	process each full AG
	 */
	for (AGNumber = 0; AGNumber < nFullAG; AGNumber++)
	{
		/* start and end block address of AG */
		AGStart = AGNumber << gcp->dn_agl2size;
		AGEnd = AGStart + gcp->dn_agsize - 1;

		/* start and end bitmap page number of AG */
		AGBMapStart = AGNumber * nBMapPerAG;	
		AGBMapEnd = AGBMapStart + nBMapPerAG - 1 ;

		/* does current lcp cover current AG ? */
		if (AGNumber >= ((lcpn + 1) * nAGPerLCP))
		{
			/* readin lcp covering i-th agwidth */
			rc = readBMapLCP(AGStart, gcp->dn_aglevel, lcp);
			if ( rc ) 
				return rc; /* i/o error */

			lcpn++;
		}

jFYI(1,("\t--- AG:%d AGStart:%lld AGEnd:%lld ---\n", 
	AGNumber, AGStart, AGEnd));

		/*
		 *	determine if this AG needs defragmentation
		 */
		rc = examineAG(AGNumber, lcp, &AGBMapStart, &AGBMapEnd);
		if (rc == NODEFRAG_FREE)
		{
			AG_SKIP_FREE++;
			continue;
		}
		else if (rc == NODEFRAG_FULL)
		{
			AG_SKIP_FULL++;
			continue;
		}
		else if (rc == NODEFRAG_LONGRUN)
		{
			AG_SKIP_LONG_RUN++;
			continue;
		}
		else if (rc != DEFRAG) /* i/o error */
			return rc;
		AG_DEFRAG++;

		/*
		 *	compact the AG
		 */
		nBMapSet = (nBMapPerAG <= BMAPSET) ? nBMapPerAG : BMAPSET;
		rc = compactAG(AGNumber, AGBMapStart, AGBMapEnd, nBMapSet);
		if ( rc )
		{
			return rc; /* i/o error */
		}

		/* display AG statistics */
jFYI(1,("\tRunLength   AllocatedRun   FreeRun\n", AGNumber));
		for (i = 1; i < XMOVESIZE; i++)
		{
			if ((AGAllocRun[i] + AGFreeRun[i]) != 0)
			{
jFYI(1,("\t%9d   %12d   %7d\n", i, AGAllocRun[i], AGFreeRun[i]));
				FSAllocRun[i] += AGAllocRun[i];
				AGAllocRun[i] = 0;
				total_free_runs += AGFreeRun[i];
				FSFreeRun[i] += AGFreeRun[i];
				AGFreeRun[i] = 0;
			}
        	} 
jFYI(1,("\t%8d+   %12d   %7d\n", i, AGAllocRun[0], AGFreeRun[0]));
		FSAllocRun[0] += AGAllocRun[0];
		AGAllocRun[0] = 0;
		total_free_runs += AGFreeRun[0];
		FSFreeRun[0] += AGFreeRun[0];
		AGFreeRun[0] = 0;
	} /* end of for-loop numags  */
	
	/*
	 *	partial last AG
	 *
	 * process the last AG, which may include virtual dmaps 
	 * (all marked as allocated) padding to make it full size 
	 * of AG, s.t. it is not very useful to examine level control
	 * pages to determine whether to defragment it or not:
	 * just go ahead and defragment it.
	 */
	if (partialAG)
	{
		AG_DEFRAG++;

		/* start and end block address of AG */
		AGStart = AGNumber << gcp->dn_agl2size;
		AGEnd = AGStart + partialAG - 1;

		/* note: partialAG may not be a multiple of BPERDMAP,
		 * the last dmap may be partially belonged to fssize,
		 * in which case, the part of the last dmap that
		 * is beyond fssize is marked as allocated in mkfs time.
		 */
		nBMap = (partialAG + BPERDMAP - 1) >>  L2BPERDMAP;
		AGBMapStart = nFullAG * nBMapPerAG;
		AGBMapEnd = AGBMapStart + nBMap -1;

jFYI(1,("\t--- partial AG:%d AGStart:%lld AGEnd:%lld ---\n", 
	AGNumber, AGStart, AGEnd));

		/*
		 *	compact the AG
		 */
		nBMapSet = (nBMap <= BMAPSET) ? nBMap : BMAPSET;
		rc = compactAG(AGNumber, AGBMapStart, AGBMapEnd, nBMapSet );
		if ( rc )
		{
			return rc; /* i/o error */
		}

		/* display AG statistics */
jFYI(1,("\tRunLength   AllocatedRun   FreeRun\n", AGNumber));
		for (i = 1; i < XMOVESIZE; i++)
		{
			if ((AGAllocRun[i] + AGFreeRun[i]) != 0)
			{
jFYI(1,("\t%9d   %12d   %7d\n", i, AGAllocRun[i], AGFreeRun[i]));
				FSAllocRun[i] += AGAllocRun[i];
				total_free_runs += AGFreeRun[i];
				FSFreeRun[i] += AGFreeRun[i];
			}
        	} 

jFYI(1,("\t%8d+   %12d   %7d\n", i, AGAllocRun[0], AGFreeRun[0]));
		FSAllocRun[0] += AGAllocRun[0];
		total_free_runs += AGFreeRun[0];
		FSFreeRun[0] += AGFreeRun[0];
	}

	/* display FS statistics */
jFYI(1,("\t=== FS summary ===\n"));
jFYI(1,("\tRunLength   AllocatedRun   FreeRun\n"));
	for (i = 1; i < XMOVESIZE; i++)
	{
			if ((FSAllocRun[i] + FSFreeRun[i]) != 0)
jFYI(1,("\t%9d   %12d   %7d\n", i, FSAllocRun[i], FSFreeRun[i]));
        } 

jFYI(1,("\t%8d+   %12d   %7d\n", i, FSAllocRun[0], FSFreeRun[0]));

	_itoa(nAG, msgstr,10);
	msg_parms[0] = msgstr;
	message_user(MSG_JFS_TOTAL_AGS, msg_parms, 1, STDOUT_CODE,
		     NO_RESPONSE, JFS_MSG);

	if (AG_SKIP_FREE)
	{
		_itoa(AG_SKIP_FREE, msgstr,10);
		msg_parms[0] = msgstr;
		message_user(MSG_JFS_SKIPPED_FREE, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
	}

	if (AG_SKIP_FULL)
	{
		_itoa(AG_SKIP_FULL, msgstr,10);
		msg_parms[0] = msgstr;
		message_user(MSG_JFS_SKIPPED_FULL, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
	}

	if (AG_SKIP_LONG_RUN)
	{
		_itoa(AG_SKIP_LONG_RUN, msgstr,10);
		msg_parms[0] = msgstr;
		message_user(MSG_JFS_SKIPPED_CONTIG, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
	}

	_itoa(AG_DEFRAG, msgstr,10);
	msg_parms[0] = msgstr;
	if (DoIt)
		message_user(MSG_JFS_NUM_DEFRAGED, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
	else
	{
		message_user(MSG_JFS_NUM_CANDIDATE, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);

		if (AG_DEFRAG)
		{
			_itoa(total_free_runs/AG_DEFRAG, msgstr, 10);
			msg_parms[0] = msgstr;
			message_user(MSG_JFS_AVG_FREE_RUNS, msg_parms, 1,
			     STDOUT_CODE, NO_RESPONSE, JFS_MSG);
		}
	}

	return rc;
}


/*
 * NAME:        examineAG()
 *
 * FUNCTION:    heuristics to determine whether to defragment
 *		the specified allocation group;
 */
static int32 examineAG(
	int32	agno, 	/* AG number */
	dmapctl_t	*lcp,	/* level control page covering AG */
	int32	*AGBMapStart, /* start bitmap apge number of AG */
	int32	*AGBMapEnd)   /* end bitmap page number of AG */
{
	dbmap_t	*gcp;
	int32	agstart;
	int32	freerate, f, r;
	int32	f0, f1, f2;

jEVENT(0,("examineAG: agno:%d blockRange:%lld:%lld bmapRange:%d:%d\n", 
	agno, AGStart, AGEnd, *AGBMapStart, *AGBMapEnd));

	gcp = (dbmap_t *)&BMap.ctl;

	/* start node index of agwidth in dcp */
	agstart = gcp->dn_agstart + 
		  gcp->dn_agwidth * (agno & (nAGPerLCP - 1));

	/* whole AG is free! */
	if (lcp->stree[agstart] >= gcp->dn_agl2size)
	{
		return NODEFRAG_FREE;
	}

	if (lcp->stree[agstart] == -1)
	{
		/* This is either entirely free or entirely allocated */
		if (gcp->dn_agfree[agno])
			return NODEFRAG_FREE;
		else
			return NODEFRAG_FULL;
	}

	/* compute % of free blocks */
	freerate =  (gcp->dn_agfree[agno] * 100) / gcp->dn_agsize;

jEVENT(0,("examineAG: nfree:%lld freerate:%d\n", 
	gcp->dn_agfree[agno], freerate));
jEVENT(0,("examineAG: lcp node:%d maxfreebuddy:%d\n", 
	agstart, lcp->stree[agstart]));

	/* too few free blocks */
	if (freerate < MIN_FREERATE) 
	{
		return NODEFRAG_FULL;
	}

	/*
	 * determine max (budmax) and its next two successive split 
	 * buddy size (budmin1 and budmin2) for the given free rate;
	 * skip defragment operation if the free blocks of AG are
	 * covered by sufficiently large contiguous extents;
	 *  free rate(%)	f0(%)	f1(%)	f2(%)
	 *  50 - 99 		50	25	12
	 *  25 - 49		25	12	6
	 *  12 - 24		12	6	3
	 *  6  - 12		6	3	1
	 *  3  - 6		3	1	0
	 */
	f0 = gcp->dn_agl2size;
	r = 100;
	f = freerate;
	while (f < r)
	{
		r /= 2;
		f0--;
	}
	f -= r;

	r /= 2;
	if (f >= r)  
	{
		f -= r;
		f1 = f0 - 1;
	}
	else
		f1 = -1;

	r /= 2;
	if (f >= r)
	{
		f -= r;
		f2 = f0 - 2;
	}
	else
		f2 = -1;

	if (f1 == -1 && f2 != -1)
	{
		f1 = f2;
		f2 = -1;
	}

	if (gcp->dn_agwidth == 2)
		goto agw2;

	/*
	 *	gcp->dn_agwidth == 1
	 */
agw1:
	if (lcp->stree[agstart] < f0)
		return DEFRAG;
		
	/* lcp->stree[agstart] == f0 */

	/* are majority of free blocks contiguous ? */
	if (f1 == -1 && f2 == -1)
	{
		return NODEFRAG_LONGRUN;
	}

	/* f1 != -1 && (f2 != -1 || f2 == -1) */

	/* check stree whether f2, f1 and f0 are contiguous */
	return DEFRAG;

	/*
	 *	gcp->dn_agwidth == 2
	 */
agw2:
	if ((lcp->stree[agstart] < f0 - 1) && 
	    (lcp->stree[agstart+1] < f0 - 1))
		return DEFRAG;
		
	/* lcp->stree[agstart] >= f0-1 || lcp->stree[agstart+1] >= f0-1 */

	/* are majority of free blocks contiguous ? */
	if (f1 == -1 && f2 == -1)
	{
		return NODEFRAG_LONGRUN;
	}

	return DEFRAG;
}


/*
 * NAME:	compactAG()
 *
 * FUNCTION:	compact allocated extents per BMapSet/AG;
 *		for each BMapSet, scan IMAP for IAGs associated
 *		with the current AG and glean allocated extents 
 *		residing in the BMapSet to build ixadTbl set, and
 *		for each ixadTbl set, scan BMapSet for free extents 
 *		to compact from the allocated extents in ixadTbl set;
 */
static int32 compactAG(
	int32	agno,
	int32	AGBMapStart,
	int32	AGBMapEnd,
	int32	nBMapSet)
{
	int32	rc, rcx = 0;
	int32	i, j, k, n;
	dmap_t	*bmp;

	BMapPageNumber = AGBMapStart;
jEVENT(0,("compactAG: agno:%d bmapRange:%d:%d\n", 
	agno, AGBMapStart, AGBMapEnd));

	/*
	 * process bitmaps of AG in set of nBMapSet at a time
	 */
	for (i = AGBMapStart; i <= AGBMapEnd; i += n)
	{
		/* read nBMapSet number of dmaps into buffer */
		n = MIN(AGBMapEnd - i + 1, nBMapSet);
		if (rc = readBMap(i, n, BMapSet))
		{
			return rc; /* i/o error */
		}

		/* at the beginning of this BMapSet,
		 * skip bitmaps if it is all allocated or all free
		 */
		for (j = 0; j < n; j++, BMapPageNumber++)
		{
			bmp = (dmap_t *)&BMapSet[j];
jEVENT(0,("compactAG: bmap:%d nfree:%d maxfreebuddy:%d\n", 
	BMapPageNumber, bmp->nfree, bmp->tree.stree[0]));

			/* if the dmap is all allocated, skip it */
			if (bmp->nfree == 0)
				continue;
			/* if the dmap is all free, skip it */
			else if (bmp->nfree == BPERDMAP)
				continue;
			else
				break;
		}
		/* any bitmap left to compact ? */
		if (j == n)
			continue;

		/*
		 * build movable extent table (ixadTbl)
		 *
		 * select movable extents of objects (inodes)
		 * from iag's associated with current AG, 
		 * which resides in the region covered by 
		 * the bitmaps in BMapSet;
	 	 */
		barrierStart = (i + j) << L2BPERDMAP;
		barrierEnd = ((i + n) << L2BPERDMAP) - 1;
build_ixadtbl:
		if (!DoIt)
			goto scan_bmap;

		rc = buildIxadTbl(agno, barrierStart, barrierEnd);
		if ( rc != 0 )
		{
			/* suspended from ixadTbl full ? */
			if (rc == ENOMEM)
				/* remember to resume */
				rcx = rc;
			else
			{
				return rc; /* i/o error */
			}
		}

		/*
		 * reorganize each dmap in current dmap set 
		 */
scan_bmap:
		for (k = j; k < n; k++)
		{
			bmp = (dmap_t *)&BMapSet[k];
			BMapPageNumber = i + k;
jEVENT(0,("compactAG: bmap:%d nfree:%d maxfreebuddy:%d\n", 
	BMapPageNumber, bmp->nfree, bmp->tree.stree[0]));

			/* if the dmap is all allocated, skip it */
			if (bmp->nfree == 0)
				continue;
			/* if the dmap is all free, skip it */
			else if (bmp->nfree == BPERDMAP)
				continue;

			/*
			 * compact region covered by dmap
			 */
			if (rc = compactBMap(i, k))
			{
				if (rc == ENOENT)
					break;	/* ixadTbl empty */
				else
					return rc; /* i/o error */
			}
		}

		/* more iags/objects to scan in AG ? */
		if (rcx == ENOMEM)
		{
			rcx = 0;
			goto build_ixadtbl;
		}

		/* purge ixadTbl */
		purgeIxadTbl();
	} /* end of for loop of BMapSet of AG */
	
	return 0;
}


/*
 * NAME:        buildIxadTbl()
 *
 * FUNCTION:    start/resume to build ixad table with size of XMOVESIZE.
 *		The index of the table is the size of extent starting
 *		from one aggr. blk up to XMOVESIZE aggr. blks.
 *		extents have size bigger than XMOVESIZE aggr. blks
 *		will not be moved.
 *		The table is built up by all the data extents address
 *		whose inode is belonged to the current dmap process group
 *		which starts from start_dmap ends with end_dmap.
 *
 * note: buildIxadTbl() and its subroutines acts like co-routine 
 * with compactAG(), i.e., they may suspend and resume with persistent 
 * state variables cross function calls;
 */
static int32 buildIxadTbl(
	int32	agno,
	int64	barrierStart,	/* start blk number of region */
	int64	barrierEnd)	/* end blk number of region */
{
	int32	rc;
	dbmap_t	*gcp;
	dinomap_t	*icp;
	dinode_t	*dip;
	xtpage_t	*p;
	int64	xaddr;
	int32	xlen;
	int32	n, i, j;

jEVENT(0,("buildIxadTbl: agno:%d region:%lld:%lld\n", 
	agno, barrierStart, barrierEnd));

	gcp = (dbmap_t *)&BMap.ctl;

	/*
	 * scan each IAG in the file set for IAG that belongs to AG
	 *
	 * for each call, resume from previous suspension point controlled by: 
	 * IAGNumber, IXNumber, IXINumber (at the start of AG, all set at 0);
	 */
	icp = (dinomap_t *)&IMap.ctl; 
	for ( ; IAGNumber < icp->in_nextiag; IAGNumber++)
	{
	 	/* If in the iagbuf still has some iag's that left over, 
		 * then iagbuf.iagnum should be the same as IAGNumber
		 */
		if (iagbuf.iagnum != IAGNumber)
		{
			rc = readIMapSequential(&iagbuf);
			if (rc)
				return rc; /* i/o error */

			/* for new IAG, reset IXNumber and IXINumber */
			IXNumber = IXINumber = 0;
		}

jEVENT(0,("buildIxadTbl: iagno:%d agstart:%lld nfree:%d\n", 
	iagbuf.iagnum, iagbuf.agstart, iagbuf.nfreeinos));

		/* is this IAG associated the current AG ? */
		if ((iagbuf.agstart >> gcp->dn_agl2size) != agno)
			continue;

		/* 
		 * scan each inode extent covered by the IAG
		 */
		for (i = IXNumber; i < EXTSPERIAG; i++, IXNumber++)
		{
			/* if there is leftover inodes, read them first */
			if (IXINumber > 0)
				goto read_inoext;

			if (iagbuf.wmap[i] == 0)
				continue;

			/* read in inode extent (4 pages) */
			xaddr = addressPXD(&iagbuf.inoext[i]);
			xlen = lengthPXD(&iagbuf.inoext[i]);
jEVENT(0,("buildIxadTbl: ixn:%d xaddr:%lld xlen:%d\n", IXNumber, xaddr, xlen));
			rc = pRead(fsMount, xaddr, xlen, (void *)ixbuf);
                        if ( rc != 0 )
			{
jERROR(1,("buildIXadTbl: inode extent i/o error rc=%d\n", rc));
				return rc; /* i/o error */
			}

			/*
			 * scan each inode in inode extent
			 */
read_inoext:
			for (j = IXINumber; j < INOSPEREXT; j++, IXINumber++)
			{
				if (ixbuf[j].di_nlink == 0 ||
				    ixbuf[j].di_inostamp != DIIMap.di_inostamp)
					continue; 

				dip = &ixbuf[j];
				switch(dip->di_mode & IFMT)
				{
					case IFREG:
						rc = addXtree(dip, barrierStart, barrierEnd);
						break;
					case IFDIR:
						rc = addDtree(dip, barrierStart, barrierEnd);
						break;
					default:
						continue;
				}
				if (rc != 0)
				{
					/* either ENOMEM or EIO */
					return rc;
				}
			} /* end-for inode extent scan */

			/* all inodes in this inoext have been processed */
			IXINumber = 0;
		} /* end-for an IAG scan */

		/* all inode extents in this IAG have been processed */
		IXNumber = 0;
	} /* end-for IMap scan */

	IAGNumber = 0;

	return 0;
}


/*
 * NAME:	addXtree()
 *
 * FUNCTION:	For a given inode of regular file,  
 *		scan its xtree and enter its allocated extents 
 *		into inode xad table;
 *		(xtree is scanned via its next sibling pointer);
 *
 * NOTE:	partial xtree may have been read previously:
 *		start from the previous stop point indicated by 
 *		xtype, lmxaddr, pnext_xtpg and nxtpg_inx.
 * 
 * RETURN:	0 -- ok
 *		ENOMEM -- No pre-allocated mem left.
 */
static int32 addXtree(
	dinode_t	*dip,
	int64	barrierStart,
	int64	barrierEnd)
{
	int32	rc;
	xtpage_t	*p; 
	int32	xlen, i;
	int64	xaddr, xoff;
	ixad_t	*ixad;

	/* start with root ? */
	if (pnext_xtpg == NULL)
	{
		pnext_xtpg = p = (xtpage_t *)&dip->di_btroot;
		if (p->header.flag & BT_LEAF)
		{
			xtype = DATAEXT;
		}
		else
		{
			xtype = XTPAGE;
			p->header.next = NULL;
			/* save leftmost child xtpage xaddr */
			lmxaddr = addressXAD(&p->xad[XTENTRYSTART]);
		}

		nxtpg_inx = XTENTRYSTART;
jEVENT(0,("addXtree: inumber:%d root xtype:%d n:%d\n", 
	dip->di_number, xtype, p->header.nextindex));
	}
	else
	{
		p = pnext_xtpg;
		/* xtype, lmxaddr, pnext_xtpg, nxtpg_inx */
jEVENT(0,("addXtree: inumber:%d xtype:%d\n", dip->di_number));
	}

	/*
	 * scan each level of xtree
	 */
	while(1)
	{
		/*
		 * scan each xtpage of current level of xtree
		 */
		while(1) 
		{
			/*
			 * scan each xad in current xtpage
			 */
			for (i = nxtpg_inx; i < p->header.nextindex; i++) 
			{
				/* test if extent is of interest */
				xoff = offsetXAD(&p->xad[i]);
				xaddr = addressXAD(&p->xad[i]) ;
				xlen = lengthXAD(&p->xad[i]) ;
jEVENT(0,("addXtree: inumber:%d xoff:%lld xaddr:%lld xlen:%d\n",
	dip->di_number, xoff, xaddr, xlen));
				if (xlen >=  XMOVESIZE)
					continue;

				if (xaddr < barrierStart)
					continue;
				if (xaddr + xlen > barrierEnd)
					continue;

				if (ixad = allocIxad(&rc))
				{
					ixad->flag = IFREG | xtype;
					ixad->fileset = dip->di_fileset;
					ixad->inostamp = dip->di_inostamp;
					ixad->inumber = dip->di_number;
					ixad->gen = dip->di_gen;
					ixad->f_xoff = xoff;
					ixad->xaddr = xaddr;
					insertIxad(xlen, ixad);
				}
				else
				{
					/* ixadTbl full */
					nxtpg_inx = i;
					return rc; /* ENOMEM */
				}
			}  /* end for current xtpage scan */

			/* read in next/right sibling xtpage */
			if (p->header.next != NULL)
			{
				xaddr = p->header.next;
				rc = pRead(fsMount, xaddr, fsMount->nbperpage, &next_xtpg);
				if ( rc != 0 )
				{
jERROR(1,("addXtree: i/o error\n"));
					return rc; /* i/o error */
				}

				pnext_xtpg = p = &next_xtpg;
				nxtpg_inx = XTENTRYSTART;
			}
			else
				break;
		} /* end while current level scan */

		/*
		 * descend: read leftmost xtpage of next lower level of xtree
		 */
		if (xtype == XTPAGE)
		{
			/* get the leftmost child page  */
			rc = pRead(fsMount, lmxaddr, fsMount->nbperpage, &next_xtpg);
			if ( rc != 0 )
			{
jERROR(1,("addXtree: i/o error\n"));
				return rc; /* i/o error */
			}

			pnext_xtpg = p = &next_xtpg;
			nxtpg_inx = XTENTRYSTART;
			if (p->header.flag & BT_LEAF)
				xtype = DATAEXT;
			else
			{
				xtype = XTPAGE;
				/* save leftmost child xtpage xaddr */
				lmxaddr = addressXAD(&p->xad[XTENTRYSTART]);
			}
		}
		else
			break;
	} /* end while level scan */

	/* this inode is done: reset variables */
	pnext_xtpg = NULL;

	return 0;
}


/*
 * NAME:	addDtree()
 *
 * FUNCTION:	For a given inode of directory, 
 *		go down to the dtree. insert the internal 
 *		and leaf nodes into the ixadTbl.
 *
 * NOTE:	partial dtree may have been read previously. We have
 *		to start from the previous stop point indicated by 
 *		next_dtpg and ndtpg_inx.
 */
static int32 addDtree(
	dinode_t	*dip,
	int64	barrierStart,
	int64	barrierEnd)
{
	int32	rc = 0;
	dtpage_t	*p; 
	int8	*stbl;
	int32	i, nbytes;
	pxd_t	*pxd;
	int64	xaddr, lmxaddr;
	int32	xlen;
	ixad_t	*ixad;

	/* start with root ? */
	if (pnext_dtpg == NULL)
	{
		pnext_dtpg = p = (dtpage_t *)&dip->di_btroot;

		/* is it leaf, i.e., inode inline data ? */
		if (p->header.flag & BT_LEAF)
		{
jEVENT(0,("addDtree: inumber:%d root leaf\n", dip->di_number));
			goto out;
		}
	
		p->header.next = NULL;
		/* save leftmost dtpage xaddr */
		lmpg_addr = 0;

		stbl = DT_GETSTBL(p);
		pxd = (pxd_t *)&p->slot[stbl[0]];
		/* save leftmost child dtpage extent */
		lmxaddr = addressPXD(pxd); /* leftmost child xaddr */
		lmxlen = lengthPXD(pxd);
		ndtpg_inx = 0;
jEVENT(0,("addDtree: inumber:%d root lmxaddr:%lld lmxlen:%d\n", 
	dip->di_number, lmxaddr, lmxlen));
	}
	else
	{
		p = pnext_dtpg;
		/* lmpg_addr, lmxaddr, pnext_dtpg, ndtpg_inx */
jEVENT(0,("addDtree: inumber:%d\n", dip->di_number));
	}

	/*
	 * scan each level of dtree
	 */
	while(1)
	{
		/*
		 * scan each dtpage of current level of dtree
		 */
		while(1) 
		{
			stbl = DT_GETSTBL(p);

			/*
			 * scan each idtentry in current dtpage
			 */
			for (i = ndtpg_inx; i < p->header.nextindex; i++) 
			{
				pxd = (pxd_t *)&p->slot[stbl[i]];

				/* test if extent is of interest */
				xaddr = addressPXD(pxd);
				xlen = lengthPXD(pxd) ;
jEVENT(0,("addDtree: inumber:%d xaddr:%lld xlen:%d\n",
	dip->di_number, xaddr, xlen));
				if (xaddr < barrierStart)
					continue;
				if (xaddr + xlen > barrierEnd)
					continue;

				if (ixad = allocIxad(&rc))
				{
					ixad->flag = IFDIR | DTPAGE;
					ixad->fileset = dip->di_fileset;
					ixad->inostamp = dip->di_inostamp;
					ixad->inumber = dip->di_number;
					ixad->gen = dip->di_gen;
					ixad->d_lmxaddr = lmpg_addr;
					ixad->xaddr = xaddr;
					insertIxad(xlen, ixad);
				}
				else
				{
					/* ixadTbl full */
					ndtpg_inx = i;
					return rc; /* ENOMEM */
				}
			}  /* end for loop */

			/* read in next/right sibling dtpage */
			if (p->header.next != NULL)
			{
				xaddr = p->header.next;
				rc = pRead(fsMount, xaddr, fsMount->nbperpage, &next_dtpg);
				if (rc != 0)
				{
jERROR(1,("addDtree: i/o error.\n"));
					return rc; /* i/o error */
				}

				pnext_dtpg = p = &next_dtpg;
				ndtpg_inx = 0;
			}
			else
				break;
		} /* end while current level scan */

		/*
		 * descend: read leftmost dtpage of next lower level of dtree
		 */
		/* the first child of the dtroot split may not have PSIZE */ 
		rc = pRead(fsMount, lmxaddr, lmxlen, &next_dtpg);
		if ( rc != 0 )
		{
jERROR(1,("addDtree: i/o error.\n"));
			return rc; /* i/o error */
		}

		pnext_dtpg = p = &next_dtpg;

		/* for dir, the leaf contains data, its pxd info 
		 * has been reported by the parent page. so we stop here 
		 */
		if (p->header.flag & BT_LEAF)
			break;

		/* save leftmost dtpage xaddr */
		lmpg_addr = lmxaddr;

		stbl = DT_GETSTBL(p);
		pxd = (pxd_t *)&p->slot[stbl[0]];
		/* save leftmost child dtpage extent */
		lmxaddr = addressPXD(pxd); /* leftmost child xaddr */
		lmxlen = lengthPXD(pxd);
		ndtpg_inx = 0;
	}

	/* reset global state variable for the inode */
out:
	pnext_dtpg = NULL;

	return rc;
}


/*
 * NAME:	allocIxad()
 *
 * FUNCTION:	allocate a free inodexaddr structure from the pre-allocated
 *		pool.
 *
 * RETURN:	rc = 0 -- ok
 *               ENOMEM --  the pool is empty.
 *               other error -- system error
 */
static ixad_t	*allocIxad(int32 *rc)
{
	register ixad_t	*ixad;
	int32	n;

	if (ixadFreelist == NULL)
	{
		n = MIN(nIxadPool, 1024);
		if (n == 0)
		{
			*rc = ENOMEM;
			return NULL;
		}
		else
			nIxadPool -= n;

		ixadFreelist = ixadHWM;

		/* init next 1024 entry */
		for (ixad = ixadFreelist; ixad < ixadFreelist + n - 1; ixad++)
			ixad->next = ixad + 1;
		ixad->next = NULL;

		ixadHWM = ++ixad;
	}

	ixad = ixadFreelist;
	ixadFreelist = ixadFreelist->next;
	nIxad++;

	*rc = 0;
	return ixad;
}


#define freeIxad(PTR)\
{\
	(PTR)->next = ixadFreelist;\
	ixadFreelist = (PTR);\
	nIxad--;\
}


/*
 * NAME:	insertIxad()
 *
 * FUNCTION:	insert ixad entry into the ixad table
 *		in ascending order of ixad xaddr.
 *
 * todo: may need do better than simple insertion sort;
 */
static void insertIxad(
	int32	nblks,	/* number of aggr. blks for the target extent */
	ixad_t	*ixad)	/* addr of pointer to inodexaddr */
{
	ixad_t	*tmp, *prev;

	/* empty list ? */
	if (ixadTbl[nblks].head == NULL)
	{
		ixadTbl[nblks].head = ixad;
		ixad->next = NULL;
		if (nblks <= 32)
			inoext_vector[0] |= UZBIT >> (nblks - 1);
		else
			inoext_vector[1] |= UZBIT >> ((nblks & (0x1F)) - 1);
	}
	else
	{
		prev = (ixad_t *)&ixadTbl[nblks].head;
		tmp = ixadTbl[nblks].head;
		while (tmp)
		{
			if (tmp->xaddr > ixad->xaddr)
			{
				ixad->next = tmp;
				prev->next = ixad;
				break;
			}
			else
			{
				prev = tmp;
				tmp = tmp->next;
			}
		}

		/* end of list ? */
		if (!tmp)
		{
			prev->next = ixad;
			ixad->next = NULL;
		}
	}

	ixadTbl[nblks].count++;
}


/*
 *	washIxadTbl()
 */
static void washIxadTbl(
	int64	barrier)
{
	ixad_t	*ixad, *head;
	int32	i;

	for (i = 1; i < XMOVESIZE; i++)
	{
		if (ixadTbl[i].count == 0)
			continue;

		ixad = ixadTbl[i].head;

		/* free ixads below watermark */
		while (ixad != NULL && ixad->xaddr < barrier)
		{
			head = ixad;
			ixad = ixad->next;
			ixadTbl[i].count--;
			freeIxad(head);
		}

		ixadTbl[i].head = ixad;
        } 
}


/*
 *	purgeIxadTbl()
 */
static void purgeIxadTbl(void)
{
	ixad_t	*ixad, *head;
	int32	i;

	for (i = 1; i < XMOVESIZE; i++)
	{
		if (ixadTbl[i].count == 0)
			continue;

		ixad = ixadTbl[i].head;

		/* free ixads below watermark */
		while (ixad != NULL)
		{
			head = ixad;
			ixad = ixad->next;
			freeIxad(head);
		}

		ixadTbl[i].count = 0;
		ixadTbl[i].head = NULL;
        } 

	nIxad = 0;
}


/*
 *	compactBMap()
 */
static int32 compactBMap(
	int32	BMapSetBMapStart,	/* current bitmap # in BMapSet[0] */
	int32	bmx)		/* current bitmap index in BMapSet */
{
	int32	rc = 0;
	uint32	x32, *wmap;
	int32	run, w, b0, b, n, i;
	uint32	mask;
	int64	xaddr0, xaddr;
	dmap_t	*bmp;
	int32	bmn, agn;

	agn = BMapSetBMapStart >> nBMapPerAG;

	bmn = BMapSetBMapStart + bmx;
	bmp = &BMapSet[bmx];
jEVENT(0,("compactBMap: bmap:%d\n", bmn)); 

	xaddr0 = bmn << L2BPERDMAP;
	wmap = bmp->wmap;

	b0 = 0;	/* start address of run in the dmap */
	b = b0;	/* address cursor */
	n = 0;	/* length of current run */

	/* determinr start alloc/free run */
	if (*wmap & 0x80000000)
		run = 1; /* alloc */
	else
		run = 0; /* free */

	/* scan each bit map word left to right */
	for (w= 0; w < LPERDMAP ; w++, wmap++)
	{
		x32 = *wmap;
		i = 32; /* bit in word counter */
		mask = 0x80000000; /* bit mask cursor */
		if (run == 0)
			goto free;

		/*
		 * scan alloc run
		 */
alloc:
		if (x32 == 0xffffffff)
		{
			b += 32;
			n += 32;
			continue;
		}

		while (i > 0 && (x32 & mask))
		{
			mask >>= 1;
			b++;
			n++;
			i--;
		}

		if (i)
		{
			if (n < XMOVESIZE)
				AGAllocRun[n]++;
			else
				AGAllocRun[0]++;

			/* alloc run stopped - switch to free run */
			run = 0; /* free */
			b0 = b;
			n = 0;
			goto free;
		}
		else
			continue;

		/*
		 * scan free run
		 */
free:
		if (x32 == 0)
		{
			b += 32;
			n += 32;
			continue;
		}

		while (i > 0 && !(x32 & mask))
		{
			mask >>= 1;
			b++;
			n++;
			i--;
		}

		if (i)
		{
			if (n < XMOVESIZE)
				AGFreeRun[n]++;
			else
				AGFreeRun[0]++;

			if (DoIt)
			{
				/* free run stopped - try to move extent into it */
				xaddr = xaddr0 + b0;
				rc = moveExtent(BMapSetBMapStart, xaddr, n);
				if (rc)
				{
					return rc; /* ixadTbl empty or i/o error */
				}
#ifdef	_JFS_DEBUG
#ifdef	_DEFRAGFS_DEBUG
if (!more())
	return rc;
#endif	/* _DEFRAGFS_DEBUG */
#endif	/* _JFS_DEBUG */
			}

			/* switch to alloc run */
			run = 1; /* alloc */
			n = 0;
			goto alloc;
		}
		else
			continue;
	}

	/* process last run */
	if (run == 1) 
	{
		if (n < XMOVESIZE)
			AGAllocRun[n]++;
		else
			AGAllocRun[0]++;
	}
	else
	{
		if (n < XMOVESIZE)
			AGFreeRun[n]++;
		else
			AGFreeRun[0]++;

		/* try to move extent into it */
		if (DoIt)
		{
			xaddr = xaddr0 + b0;
			rc = moveExtent(BMapSetBMapStart, xaddr, n);
		}
	}

	return rc;
}

	
/*
 * NAME:	moveExtent()
 *
 * FUNCTION:	Move extent(s) into the specified free extent
 *		(blkno, len), and 
 *		update the wmap in the dmap set for the moved extent 
 *		if this extent is within the current dmap set
 *
 * TODO: error handling
 */
static int32 moveExtent(
	int32	BMapSetBMapStart,  /* the dmap number in the BMapSet[0]  */
	int64	fxaddr,	/* the beginning blkno of the free space */
	int32	fxlen)	/* the number of blks of the free space */
{
	int32	rc = 0;
	ixad_t	*ixad = NULL, *head;
	int64	xaddr;
	int32	i;

jEVENT(0,("moveExtent: xaddr:%lld xlen:%d\n", fxaddr, fxlen));

	/* update the global blk address Barrier:
	 * extents before it should not be moved.
	 */
	barrier = fxaddr;

	/*
	 * move allocated extent(s) to the free extent;
	 */
	while (fxlen)
	{
		/* select best-fit ixad to move */
		i = MIN(fxlen, XMOVESIZE - 1);
		for ( ; i > 0; i--)
			if (ixadTbl[i].head != NULL)
				break;
		if (i > 0)
			ixad = ixadTbl[i].head;
		else if (nIxad == 0)
			return ENOENT;	/* ixadTbl empty */
		else
			return 0; /* no movalble extent */

		/* free ixads below watermark */
		while (ixad != NULL && ixad->xaddr < barrier)
		{
			/* free ixad */
			head = ixad;
			ixad = ixad->next;
			ixadTbl[i].head = ixad;
			ixadTbl[i].count--;
			freeIxad(head);
		}
		if ( ixad == NULL )
			continue;
		else
		{
			/* request fs to relocate the extent */
			rc = fscntlMove(fxaddr, i, ixad);
			if (rc == 0)
			{
				/* update the wmap in the dmap set for
				 * the moved extent IF this extent is
				 * within the current dmap set;
				 */
				xaddr = ixad->xaddr;
/*
				if ((xaddr + i -1 ) <= BMapSetEnd)
					dbFreeBMap(BMapSetBMapStart, xaddr, i);
				else if (xaddr <= BMapSetEnd)
					 dbFreeBMap(BMapSetBMapStart, xaddr, BMapSetEnd - xaddr + 1);
*/
				dbFreeBMap(BMapSetBMapStart, xaddr, i);

				/* remove ixad */
				head = ixad;
				ixad = ixad->next;
				ixadTbl[i].head = ixad;
				ixadTbl[i].count--;
				freeIxad(head);

				/* advance Barrier */
				barrier += i ;
				fxaddr = barrier;
				fxlen -= i;
			}
			else
			{
				/* stale source extent ? */
				if (rc = ESTALE)
				{
					/* remove ixad */
					head = ixad;
					ixad = ixad->next;
					ixadTbl[i].head = ixad;
					ixadTbl[i].count--;
					freeIxad(head);
				}		
				/* stale destination extent ? */
				else if (rc = ENOSPC)
				{
					/* advance Barrier */
					barrier += i ;
					fxaddr = barrier;
					fxlen -= i;
				}
				else /* i/o error */
				{
					return rc; /* i/o error */
				}
			}
		}
	} /* end while */

	return 0;
}


/*
 * NAME:	fscntlMove()
 *
 * FUNCTION:  attempts to move the number of len logical blks
 *    specified by ixad to the extent starting at blkno.
 */
static int32 fscntlMove(
	int64	nxaddr,
	int32	xlen, 
	ixad_t	*ixad)
{
	int32	rc;
	int32	pList;
	ULONG	pListLen = 0;
	defragfs_t	pData;
	ULONG	pDataLen = 0;

	pDataLen = sizeof(defragfs_t);

	/*
	 * move extent
	 */
	pData.flag = ixad->flag | DEFRAGFS_RELOCATE;
	pData.dev = lvMount->LVNumber;
	pData.fileset = ixad->fileset;
	pData.inostamp = ixad->inostamp;
	pData.ino = ixad->inumber;
	pData.gen = ixad->gen;
	pData.xoff = ixad->f_xoff;
	pData.old_xaddr = ixad->xaddr;
	pData.new_xaddr = nxaddr;
	pData.xlen = xlen;
jEVENT(0,("defragfs i:%d xoff:%lld xlen:%d xaddr:%lld:%lld\n", 
	pData.ino, pData.xoff, pData.xlen, pData.old_xaddr, pData.new_xaddr));

	rc = fscntl(JFSCTL_DEFRAGFS,
	       (void *)&pList, &pListLen,
	       (void *)&pData, &pDataLen);

	if (rc != 0 &&
	    (rc != ESTALE ||	/* source extent stale */ 
	     rc != ENOSPC))	/* destination extent stale */
		rc = EIO;	/* i/o error */

	return rc;
}


/*
 * NAME:	dbFreeBMap()
 *
 * FUNCTION:	update the wmap in the current dmap set.
 *		starting from blkno in the wmap, mark nblocks as free .
 */
static void dbFreeBMap(
	int32	BMapSetBMapStart,	/* the dmap number in the BMapSet[0] */
	int64	blkno,		/* starting block number to mark as free */
	int32	nblocks)	/* number of bits marked as free */
{
	dbmap_t	*gcp;		/* global control page  */
	dmap_t	*bmp;		/* bitmap page */
	dmtree_t	*tp;
	int32	bmn, i;
	int32	lzero, tzero;
	int32	bit, word, rembits, nb, nwords, wbitno, nw, agno;
	int8	size, maxsize, budsize;
	
	bmn = blkno >> L2BPERDMAP;
	i = bmn - BMapSetBMapStart;
	bmp = (dmap_t *)&BMapSet[i];
	tp = (dmtree_t *)&bmp->tree;

        /* determine the bit number and word within the dmap of the
         * starting block.
         */
	bit = blkno & (BPERDMAP - 1);
	word = bit >> L2DBWORD;

	/* block range better be within the dmap */ 
	assert(bit + nblocks <= BPERDMAP);

        /* free the bits of the dmaps words corresponding to the block range.
	 * if not all bits of the first and last words may be contained within
	 * the block range, work those words (i.e. partial first and/or last) 
	 * on an individual basis (a single pass), freeing the bits of interest
	 * by hand and updating the leaf corresponding to the dmap word. 
	 * a single pass will be used for all dmap words fully contained within
	 * the specified range. within this pass, the bits of all fully 
	 * contained dmap words will be marked as free in a single shot and 
	 * the leaves will be updated. 
	 * a single leaf may describe the free space of multiple dmap words,
         * so we may update only a subset of the actual leaves corresponding
	 * to the dmap words of the block range.
	 */
	for (rembits = nblocks; rembits > 0; rembits -= nb, bit += nb)
	{
                /* determine the bit number within the word and
                 * the number of bits within the word.
                 */
		wbitno = bit & (DBWORD-1);
		nb = MIN(rembits, DBWORD - wbitno);

		/* is only part of a word to be freed ? */
		if (nb < DBWORD)
		{
			/* free (zero) the appropriate bits within this
			 * dmap word. 
			 */
			bmp->wmap[word] &= ~(ONES << (DBWORD - nb) >> wbitno);
			word += 1;
		}
		else
		{
                        /* one or more dmap words are fully contained
                         * within the block range.  determine number of
                         * words to be freed and free (zero) those words.
                         */
			nwords = rembits >> L2DBWORD;
			memset(&bmp->wmap[word], 0, nwords*4);

			/* determine number of bits freed */
			nb = nwords << L2DBWORD;

			/* update the leaves to reflect the freed words */
			for (; nwords > 0; nwords -= nw)
			{
				/* determine the new value of leaf covering
				 * words freed from word as minimum of
				 * the l2 number of buddy size of bits being 
				 * freed and 
				 * the l2 (max) number of bits that can be 
				 * described by this leaf.
				 */
				countTZ32(word, &tzero);
				maxsize = ((word == 0) ? L2LPERDMAP : tzero) + BUDMIN;
				countLZ32(nwords, &lzero);
				budsize = 31 - lzero + BUDMIN;
				size = MIN(maxsize, budsize);

				/* get the number of dmap words handled */
				nw = BUDSIZE(size,BUDMIN);
				word += nw;
			}
		}
	}

	/* update the free count for this dmap */
	bmp->nfree += nblocks;
}


/*
 *	preamble()
 */
static int32 preamble(void)
{
	int32	rc = 0;
	int32	pList;
	ULONG	pListLen = 0;
	defragfs_t	pData;
	ULONG	pDataLen = 0;
	buf_t	*bp;
	dinode_t	*dip;
	int32	i;

	/*
	 * sync fs meta-data 
	 */
jEVENT(0,("preamble: sync.\n"));
	pDataLen = sizeof(defragfs_t);
	pData.flag = DEFRAGFS_SYNC;
	pData.dev = lvMount->LVNumber;

	rc = fscntl(JFSCTL_DEFRAGFS,
	       (void *)&pList, &pListLen,
	       (void *)&pData, &pDataLen);
	if (rc)
	{
		msg_parms[0] = "Sync Failure";
		message_user(MSG_JFS_UNEXPECTED_ERROR, msg_parms, 1,
			     STDOUT_CODE, NO_RESPONSE, JFS_MSG);
		return rc;
	}

        /*
         * read in the block allocation map inode (i_number = 2)
         */
        if (rc = bRawRead(LVHandle, (int64)AITBL_OFF, PAGESIZE, &bp))
	{
jERROR(1,("preamble: i/o error: rc=%d\n", rc));
                return rc;
	}

        /* locate the bmap inode in the buffer page */
	dip = (dinode_t *)bp->b_data;
        dip += BMAP_I;
	memcpy(diBMap, dip, DISIZE);

	bRelease(bp);

	/*
	 * read bmap control page(s)
	 */
	rc = readBMapGCP(bMap);
	if ( rc != 0 )
		return rc;

        /*
         * read in the fileset inode allocation map inode (i_number = 16)
         */
	i = FILESYSTEM_I / INOSPERPAGE;
        if (rc = bRawRead(LVHandle, (int64)(AITBL_OFF + PAGESIZE * i), PAGESIZE, &bp))
	{
jERROR(1,("preamble: i/o error: rc=%d\n", rc));
                return rc;
	}

        /* locate the inode in the buffer page */
	dip = (dinode_t *)bp->b_data;
        dip += FILESYSTEM_I & (INOSPERPAGE - 1);
	memcpy(diIMap, dip, DISIZE);
	
	bRelease(bp);

	/*
	 * read imap global control page
	 */
	rc = readIMapGCPSequential(iMap, &iagbuf);
	if ( rc != 0 )
		return rc;

	/*
	 * init statistics
	 */
	for (i = 0; i < XMOVESIZE; i++)
	{
		AGAllocRun[i] = 0;
		AGFreeRun[i] = 0;

		FSAllocRun[i] = 0;
		FSFreeRun[i] = 0;
        } 

	AG_DEFRAG = 0;
	AG_SKIP_FREE = 0;
	AG_SKIP_FULL = 0;
	AG_SKIP_LONG_RUN = 0;
	total_free_runs = 0;

	return rc;
}


/*
 *	getProvisions()
 *
 * allocate resources
 */
static int32 getProvisions(void)
{
	int32	nbytes, i;

	/*
	 * alloc/init the movable/relocatable extent table
	 */
	/* init list anchor table */
	for (i = 0; i < XMOVESIZE; i++)
	{
		ixadTbl[i].count = 0;
		ixadTbl[i].head = NULL;
        } 

	/* allocate movable extent entry pool:
	 *
	 * each dmap monitors BPERDMAP blks. 
	 * Assume one ixad_t entry per two blks.
 	 */ 
	nBMapPerAG = fsMount->AGSize >> L2BPERDMAP;
	nBMapSet = (nBMapPerAG <= BMAPSET) ? nBMapPerAG : BMAPSET;

	nbytes = (nBMapSet << (L2BPERDMAP - 1)) * sizeof(ixad_t);  
	nbytes = (nbytes + PAGESIZE - 1) & ~(PAGESIZE - 1);

	if ((ixadPool = (ixad_t *)malloc(nbytes)) == NULL)
	{
	message_user(MSG_OSO_NOT_ENOUGH_MEMORY, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
		return ENOMEM;
	}

	nIxadPool = nbytes / sizeof(ixad_t);
	ixadHWM = ixadPool;
	ixadFreelist = NULL;

	return 0;
}


/*
 *	whatDoYouWant()
 *
 * defrag  [-q] {device_name}
 *
 *  option:
 *	-q -	query. report current file system status. 
 *		only the status is reported, nothing is done.
 *
 *	no option - do the defragmentation and generate the result report.
 *
 *  device_name - device under defragmentation.
 */
static int32 whatDoYouWant(
	int32	argc,
	char	**argv)
{
	int32	rc = 0;
	int32	i;
	char	*argp;
	int	devFound = FALSE;

#ifdef NO_DRIVE_NEEDED
	UCHAR	cwd[80];

	/* initialize the disk name to be the current drive */
	getcwd(cwd, 80);
	LVName[0] = cwd[0];
	LVName[1] = cwd[1];
	LVName[2] = '\0';
#endif

	if (argc < 2)
		return -1;

	/* parse each of the command line parameters */
	for (i = 1; i < argc && rc == 0; i++)
	{
		argp = argv[i];

jEVENT(0,("whatDoYouWant: arg:%s\n", argp));

		/* a switch ? */
		if (*argp == '/' || *argp == '-')  
		{
			argp++;  /* increment past / or - */

			while (rc == 0 && *argp)
			{
				switch (toupper(*argp))
				{
				case 'Q':  /* Query fs status only */
					Query = TRUE;
					DoIt = FALSE;
					argp++;
					break;
				default:    /* unknown switch */
					rc = -1;
					goto out;
				}
			}

			if (*argp)
			{
				rc = -1;
				goto out;
			}

			continue;
		}

		/* a drive letter */
		if (isalpha(*argp) && argp[1] == ':' && !argp[2])
		{  
			/* only allow one drive letter to be specified */
			if (devFound)  
			{
				rc = -1;
				break;
			}
			else
			{
				strcpy(LVName, argp);
				devFound = TRUE;
			}

			continue;
		}

		rc = -1;
		break;
	} /* end for */

	if (!devFound)
		rc = -1;

out:
jEVENT(0,("whatDoYouWant: rc=%d\n", rc));
	return (rc);
}
