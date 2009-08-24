/* $Id: jfs_dtree.c,v 1.2 2003/09/21 08:42:54 pasha Exp $ */

static char *SCCSID = "@(#)1.36.1.2  12/2/99 11:13:25 src/jfs/ifs/jfs_dtree.c, sysjfs, w45.fs32, fixbld";
/*
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
*/

/*
 * Change History :
 * PS 2001-2003 apply IBM fixes
 *
 */

/*
 *	jfs_dtree.c: directory B+-tree manager
 *
 * B+-tree with variable length key directory:
 *
 * each directory page is structured as an array of 32-byte
 * directory entry slots initialized as a freelist
 * to avoid search/compaction of free space at insertion.
 * when an entry is inserted, a number of slots are allocated
 * from the freelist as required to store variable length data
 * of the entry; when the entry is deleted, slots of the entry
 * are returned to freelist.
 *
 * leaf entry stores full name as key and file serial number
 * (aka inode number) as data.
 * internal/router entry stores sufffix compressed name
 * as key and simple extent descriptor as data.
 *
 * each directory page maintains a sorted entry index table
 * which stores the start slot index of sorted entries
 * to allow binary search on the table.
 *
 * directory starts as a root/leaf page in on-disk inode
 * inline data area.
 * when it becomes full, it starts a leaf of a external extent
 * of length of 1 block. each time the first leaf becomes full,
 * it is extended rather than split (its size is doubled),
 * until its length becoms 4 KBytes, from then the extent is split
 * with new 4 Kbyte extent when it becomes full
 * to reduce external fragmentation of small directories.
 *
 * blah, blah, blah, for linear scan of directory in pieces by
 * readdir().
 *
 *
 *	case-insensitive directory file system
 *
 * names are stored in case-sensitive way in leaf entry.
 * but stored, searched and compared in case-insensitive (uppercase) order
 * (i.e., both search key and entry key are folded for search/compare):
 * (note that case-sensitive order is BROKEN in storage, e.g.,
 *  sensitive: Ad, aB, aC, aD -> insensitive: aB, aC, aD, Ad
 *
 *  entries which folds to the same key makes up a equivalent class
 *  whose members are stored as contiguous cluster (may cross page boundary)
 *  but whose order is arbitrary and acts as duplicate, e.g.,
 *  abc, Abc, aBc, abC)
 *
 * once match is found at leaf, requires scan forward/backward
 * either for, in case-insensitive search, duplicate
 * or for, in case-sensitive search, for exact match
 *
 * router entry must be created/stored in case-insensitive way
 * in internal entry:
 * (right most key of left page and left most key of right page
 * are folded, and its suffix compression is propagated as router
 * key in parent)
 * (e.g., if split occurs <abc> and <aBd>, <ABD> trather than <aB>
 * should be made the router key for the split)
 *
 * case-insensitive search:
 *
 * 	fold search key;
 *
 *	case-insensitive search of B-tree:
 *	for internal entry, router key is already folded;
 *	for leaf entry, fold the entry key before comparison.
 *
 *	if (leaf entry case-insensitive match found)
 *		if (next entry satisfies case-insensitive match)
 *			return EDUPLICATE;
 *		if (prev entry satisfies case-insensitive match)
 *			return EDUPLICATE;
 *		return match;
 *	else
 *		return no match;
 *
 * 	serialization:
 * target directory inode lock is being held on entry/exit
 * of all main directory service routines.
 *
 *	log based recovery:
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */
#include "mmph.h"

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_inode.h"
#include "jfs_bufmgr.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_txnmgr.h"
#include "jfs_dmap.h"		/* for function prototype */
#include "jfs_debug.h"

	#include "uni_inln.h"

/* dtree split parameter */
typedef struct {
	jbuf_t		*bp;
	int16		index;
	int16		nslot;
	component_t	*key;
	ddata_t		*data;
	pxdlist_t	*pxdlist;
} dtsplit_t;

/*
 * special entry prototype
 */
static UniChar nameDot[2] = L".";
static UniChar nameDotDot[3] = L"..";
static component_t dnameDot = {1, nameDot};
static component_t dnameDotDot = {2, nameDotDot};

static struct edir {
	struct direntDot {
		uint32	d_offset;
		ino_t	d_ino;
		uint16	d_reclen;
		uint16	d_namlen;
		UniChar	d_name[2]; /* dot (.) */
	} dot;
	struct direntDotDot{
		uint32	dd_offset;
		ino_t	dd_ino;
		uint16	dd_reclen;
		uint16	dd_namlen;
		UniChar	dd_name[4]; /* dot dot (..) */
	} dotdot;
} edir = {{0x0001ffff, 0, 16, 1, L"."}, {0x0002ffff, 0, 20, 2, L".."}};

/* get page buffer for specified block address */
#define DT_GETPAGE(IP, BN, BP, TYPE, SIZE, P, RC)\
{\
	BT_GETPAGE(IP, BN, BP, TYPE, SIZE, P, RC)\
	if (!(RC))\
	{\
		if (((P)->header.nextindex > (((BN)==0)?DTROOTMAXSLOT:(P)->header.maxslot)) ||\
		    ((BN) && ((P)->header.maxslot > DTPAGEMAXSLOT)))\
		{\
			jEVENT(1,("DT_GETPAGE: dtree page corrupt\n"));\
			txFileSystemDirty((IP)->i_ipmnt);\
			RC = EIO;\
		}\
	}\
}

/* for consistency */
#define DT_PUTPAGE(BP) BT_PUTPAGE(BP)

#ifdef _JFS_STATISTICS
/*
 *	statistics
 */
static uint32	dtstat_Search = 0;
static uint32	dtstat_Insert = 0;
static uint32	dtstat_split = 0;
static uint32	dtstat_rootsplit = 0;
static uint32	dtstat_Delete = 0;
static uint32	dtstat_free = 0;
#endif	/* _JFS_STATISTICS */

/*
 * forward references
 */
static int32 dtSplitUp(
	int32		tid,
	inode_t		*ip,
	dtsplit_t	*split,
	btstack_t	*btstack);

static int32 dtSplitPage(
	int32		tid,
	struct inode	*ip,
	dtsplit_t	*split,
	jbuf_t		**rbpp,
	pxd_t		*rxdp);

static int32 dtExtendPage(
	int32		tid,
	inode_t		*ip,
	dtsplit_t	*split,
	btstack_t	*btstack);

static int32 dtSplitRoot(
	int32		tid,
	inode_t		*ip,
	dtsplit_t	*split,
	jbuf_t		**rbpp);

static int32 dtDeleteUp(
	int32		tid,
	inode_t		*ip,
	jbuf_t		*fbp,
	btstack_t	*btstack);

static int32 dtSearchNode(
	inode_t		*ip,
	int64		lmxaddr,
	pxd_t		*kpxd,
	btstack_t	*btstack);

static int32 dtRelink(
	int32		tid,
	inode_t		*ip,
	dtpage_t	*p);

static int32 dtReadFirst(
	inode_t		*ip,
	btstack_t	*btstack);

static int32 dtReadNext(
	inode_t		*ip,
	int32		*offset,
	btstack_t	*btstack);

static int32 dtCompare(
	component_t	*key,
	dtpage_t	*p,
	int32		si);

static void dtGetLeafPrefixKey(
	dtpage_t	*lp,
	int32		li,
	dtpage_t	*rp,	
	int32		ri,
	component_t	*key);

static void dtGetKey(
	dtpage_t	*p,
	int32		i,
	component_t	*key);

#ifdef	_JFS_OS2
static int32 dtPrefixWild(
	UniChar		*pattern,
	component_t	*prefix);

static int32 dtCompareWild(
	UniChar	*w,
	UniChar	*s,
	int32	*m);

static int32 ciCompare(
	component_t	*key,
	dtpage_t	*p,
	int32		si);

static void ciGetLeafPrefixKey(
	dtpage_t	*lp,
	int32		li,
	dtpage_t	*rp,	
	int32		ri,
	component_t	*key);

#define ciToUpper(c)	UniStrupr((c)->name)
#endif	/* _JFS_OS2 */

static void dtInsertEntry(
	dtpage_t	*p,	
	int32		index,
	component_t	*key,
	ddata_t		*data,
	dtlock_t	**dtlock);

static void dtMoveEntry(
	dtpage_t	*sp,
	int32		si,
	dtpage_t	*dp,
	dtlock_t	**sdtlock,
	dtlock_t	**ddtlock);

static void dtDeleteEntry(
	dtpage_t	*p,
	int32		fi,
	dtlock_t	**dtlock);

static void dtTruncateEntry(
	dtpage_t	*p,
	int32		ti,
	dtlock_t	**dtlock);

static void dtLinelockFreelist(
	dtpage_t	*p,
	int32		m,
	dtlock_t	**dtlock);

/* external references */
int32	txFileSystemDirty(inode_t *);
extern pool_t	*unipool;

/*
 *	debug control
 */
/*	#define	_JFS_DEBUG_DTREE	1 */


/*
 *	dtSearch()
 *
 * function:
 *	Search for the entry with specified key
 *
 * parameter:
 *
 * return: 0 - search result on stack, leaf page pinned;
 *	   errno - I/O error
 */
int32
dtSearch(
	inode_t		*ip,
	component_t	*key,
	ino_t		*data,
	btstack_t	*btstack,
	uint32		flag)
{
	int32		rc = 0;
	int32		cmp = 1;/* init for empty page */
	int64		bn;	
	jbuf_t		*bp;
	dtpage_t	*p;
	int8		*stbl;
	int32		base, index, lim;
	btframe_t	*btsp;
	pxd_t		*pxd;
	int32		psize = 288;	/* initial in-line directory */
	ino_t		inumber;
#ifdef	_JFS_OS2
	component_t	ciKey;

	ciKey.name = (UniChar *)allocpool(unipool, 0);

	/* uppercase search key for c-i directory */
	UniStrcpy(ciKey.name, key->name);
	ciKey.namlen = key->namlen;
	ciToUpper(&ciKey);
#endif	/* _JFS_OS2 */

        MMPHPredtSearch();      /* MMPH Performance Hook */

	BT_CLR(btstack);	/* reset stack */

	/* init level count for max pages to split */
	btstack->nsplit = 1;	

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
	 *
	 * by convention, root bn = 0.
	 */
	for (bn = 0; ;)
	{
		/* get/pin the page to search */
		DT_GETPAGE(ip, bn, bp, dtpage_t, psize, p, rc);
		if (rc)
			goto dtSearch_Exit;
		
		/* get sorted entry table of the page */
		stbl = DT_GETSTBL(p);

		/*
		 * binary search with search key K on the current page.
		 */
		for (base = 0, lim = p->header.nextindex; lim; lim >>= 1)
		{
			index = base + (lim >> 1);

#ifdef	_JFS_CASE_SENSITIVE
			if ((cmp = dtCompare(key, p, stbl[index])) == 0)
#endif	/* _JFS_CASE_SENSITIVE */
#ifdef	_JFS_OS2
			if (p->header.flag & BT_LEAF)
			{
				/* uppercase leaf name to compare */
				cmp = ciCompare(&ciKey, p, stbl[index]);
			}
			else
			{
				/* router key is in uppercase */
				cmp = dtCompare(&ciKey, p, stbl[index]);
			}
			if (cmp == 0)
#endif	/* _JFS_OS2 */
			{
				/*
				 *	search hit
				 */
				/* search hit - leaf page:
				 * return the entry found
				 */
				if (p->header.flag & BT_LEAF)
				{
					inumber = ((ldtentry_t *)&p->slot[stbl[index]])->inumber;

					/*
					 * search for JFS_LOOKUP
					 */
					if (flag == JFS_LOOKUP)
					{
						*data = inumber;
						rc = 0;
						goto out;
					}

					/*
					 * search for JFS_CREATE
					 */
					if (flag == JFS_CREATE)
					{
						*data = inumber;
						rc = EEXIST;
						goto out;
					}

					/*
					 * search for JFS_REMOVE or JFS_RENAME
					 */
					if ((flag == JFS_REMOVE ||
					     flag == JFS_RENAME) &&
					    *data != inumber)
					{
						rc = ESTALE;
						goto out;
					}

					/*
					 * JFS_REMOVE|JFS_FINDDIR|JFS_RENAME
					 */
					/* save search result */
					*data = inumber;
					btsp = btstack->top;
					btsp->bn = bn;
					btsp->index = index;
					btsp->bp = bp;

					rc = 0;
					goto dtSearch_Exit;
				}

				/* search hit - internal page:
				 * descend/search its child page
				 */
				goto getChild;
			}

			if (cmp > 0)
			{
				base = index + 1;
				--lim;
			}
		}

		/*
		 *	search miss
		 *
		 * base is the smallest index with key (Kj) greater than
		 * search key (K) and may be zero or (maxindex + 1) index.
		 */
		/*
		 * search miss - leaf page
		 *
		 * return location of entry (base) where new entry with
		 * search key K is to be inserted.
		 */
		if (p->header.flag & BT_LEAF)
		{
			/*
			 * search for JFS_LOOKUP, JFS_REMOVE, or JFS_RENAME
			 */
			if (flag == JFS_LOOKUP || flag == JFS_REMOVE ||
			    flag == JFS_RENAME)
			{
				rc = ENOENT;
				goto out;
			}

			/*
			 * search for JFS_CREATE|JFS_FINDDIR:
			 *
			 * save search result
			 */
			*data = 0;
			btsp = btstack->top;
			btsp->bn = bn;
			btsp->index = base;
			btsp->bp = bp;

			rc = 0;
			goto dtSearch_Exit;
		}

		/*
		 * search miss - internal page
		 *
		 * if base is non-zero, decrement base by one to get the parent
		 * entry of the child page to search.
		 */
		index = base ? base - 1 : base;

		/*
		 * go down to child page
		 */
getChild:
		/* update max. number of pages to split */
		if (btstack->nsplit >= 8)
		{
			/* Something's corrupted, mark filesytem dirty so
			 * chkdsk will fix it.
			 */
			jERROR(1,("stack overrun in dtSearch!\n"));
			txFileSystemDirty(ip->i_ipmnt);
			rc = EIO;
			goto out;
		}
		btstack->nsplit++;

		/* push (bn, index) of the parent page/entry */
		BT_PUSH(btstack, bn, index);

		/* get the child page block number */
		pxd = (pxd_t *)&p->slot[stbl[index]];
		bn = addressPXD(pxd);
		psize = lengthPXD(pxd) << ip->i_ipmnt->i_l2bsize;

		/* unpin the parent page */
		DT_PUTPAGE(bp);
	}

out:
	DT_PUTPAGE(bp);

dtSearch_Exit:

#ifdef	_JFS_OS2
	freepool(unipool, (caddr_t *)ciKey.name);
#endif

	MMPHPostdtSearch();     /* MMPH Performance Hook */

	return rc;
}


/*
 *	dtInsert()
 *
 * function: insert an entry to directory tree
 *
 * parameter:
 *
 * return: 0 - success;
 *	   errno - failure;
 */
int32
dtInsert(
	int32		tid,
	inode_t		*ip,
	component_t 	*name,
	ino_t		*fsn,
	btstack_t	*btstack)
{
	int32		rc = 0;
	jbuf_t		*bp;	/* page buffer */
	dtpage_t	*p;	/* base B+-tree index page */
	int64		bn;
	int32		index;
	uint8		*stbl;
	dtsplit_t	split;	/* split information */
	ddata_t		data;
	int32		lid;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;
	int32		n;

	/*
	 *	retrieve search result
	 *
	 * dtSearch() returns (leaf page pinned, index at which to insert).
	 * n.b. dtSearch() may return index of (maxindex + 1) of
	 * the full page.
	 */
	BT_GETSEARCH(ip, btstack->top, bn, bp, dtpage_t, p, index);

	/*
	 *	insert entry for new key
	 */
	n = NDTLEAF(name->namlen);
	data.ino = *fsn;

	/*
	 *	leaf page does not have enough room for new entry:
	 *
	 *	extend/split the leaf page;
	 *
	 * dtSplitUp() will insert the entry and unpin the leaf page.
	 */
	if (n > p->header.freecnt)
	{
		split.bp = bp;
		split.index = index;
		split.nslot = n;
		split.key = name;
		split.data = &data;
		rc = dtSplitUp(tid, ip, &split, btstack);
		return rc;
	}

	/*
	 *	leaf page does have enough room for new entry:
	 *
	 *	insert the new data entry into the leaf page;
	 */
	/*
	 * acquire a transaction lock on the leaf page
	 */
	tlck = txLock(tid, ip, bp, tlckDTREE|tlckENTRY);
	dtlck = (dtlock_t *)&tlck->lock;
	ASSERT(dtlck->index == 0);
	lv = (lv_t *)&dtlck->lv[0];

	/* linelock header */
	lv->offset = 0;
	lv->length = 1;
	dtlck->index++;

	dtInsertEntry(p, index, name, &data, &dtlck);

	/* linelock stbl of non-root leaf page */
	if (!(p->header.flag & BT_ROOT))
	{
		if (dtlck->index >= dtlck->maxcnt)
			dtlck = (dtlock_t *)txLinelock(dtlck);
		lv = (lv_t *)&dtlck->lv[dtlck->index];
		n = index >> L2DTSLOTSIZE;
		lv->offset = p->header.stblindex + n;
		lv->length = ((p->header.nextindex - 1) >> L2DTSLOTSIZE) - n + 1;
		dtlck->index++;
	}

	/* unpin the leaf page */
	DT_PUTPAGE(bp);

	return 0;
}


/*
 *	dtSplitUp()
 *
 * function: propagate insertion bottom up;
 *
 * parameter:
 *
 * return: 0 - success;
 *	   errno - failure;
 * 	leaf page unpinned;
 */
static int32
dtSplitUp(
	int32		tid,
	inode_t		*ip,
	dtsplit_t	*split,
	btstack_t	*btstack)
{
	int32		rc = 0;
	jbuf_t		*sbp;
	dtpage_t	*sp;	/* split page */
	jbuf_t		*rbp;
	dtpage_t	*rp;	/* new right page split from sp */
	pxd_t		rpxd;	/* new right page extent descriptor */
	jbuf_t		*lbp;
	dtpage_t	*lp;	/* left child page */
	int32 		skip;	/* index of entry of insertion */
	uint8		*stbl;
	btframe_t	*parent;/* parent page entry on traverse stack */
	int64		xaddr, nxaddr;
	int32		xlen, xsize;
	pxdlist_t	pxdlist;
	pxd_t		*pxd;
	component_t	key = {0, 0};
	ddata_t		*data = split->data;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;
	int32		n;

        MMPHPredtSplitUp();     /* MMPH Performance Hook */

#ifdef	_JFS_OS2
	/* We're kind of tight on stack space, so allocate this from pool
	 */
	key.name = (UniChar *)allocpool(unipool, 0);
	if (key.name == 0) {
		//return ENOMEM;
           rc = ENOMEM;
           goto dtSplitUp_Exit;
   }
#endif	/* _JFS_OS2 */

	/* get split page */
	sbp = split->bp;
	sp = BT_PAGE(ip, sbp, dtpage_t);

	/*
	 *	split leaf page
	 *
	 * The split routines insert the new entry, and
	 * acquire txLock as appropriate.
	 */
	/*
	 *	split root leaf page:
	 */
	if (sp->header.flag & BT_ROOT)
	{
		/*
		 * allocate a single extent child page
		 */
		xlen = 1;
		n = ip->i_ipmnt->i_bsize >> L2DTSLOTSIZE;
		n -= (n + 31) >> L2DTSLOTSIZE; /* stbl size */
		n -= DTROOTMAXSLOT - sp->header.freecnt; /* header + entries */
		if (n <= split->nslot)
			xlen++;
		if (rc = dbAlloc(ip, 0, (int64)xlen, &xaddr))
			goto freeKeyName;

		pxdlist.maxnpxd = 1;
		pxdlist.npxd = 0;
		pxd = &pxdlist.pxd[0];
		PXDaddress(pxd, xaddr);
		PXDlength(pxd, xlen);
		split->pxdlist = &pxdlist;
	    	rc = dtSplitRoot(tid, ip, split, &rbp);

		DT_PUTPAGE(rbp);
		DT_PUTPAGE(sbp);

		ip->i_size = xlen << ip->i_ipmnt->i_l2bsize;

		goto freeKeyName;
	}

	/*
	 *	extend first leaf page
	 *
	 * extend the 1st extent if less than buffer page size
	 * (dtExtendPage() reurns leaf page unpinned)
	 */
	if (ip->i_size < PSIZE)
	{
		pxd = &sp->header.self;
		xaddr = addressPXD(pxd);
		xlen = lengthPXD(pxd);
		xsize = xlen << ip->i_ipmnt->i_l2bsize;
		n = xsize >> L2DTSLOTSIZE;
		n -= (n + 31) >> L2DTSLOTSIZE; /* stbl size */
		if ((n + sp->header.freecnt) <= split->nslot)
			n = xlen + (xlen << 1);
		else
			n = xlen;
		if (rc = dbReAlloc(ip->i_ipmnt->i_ipbmap, xaddr, (int64)xlen, (int64)n, &nxaddr))
			goto extendOut;

		pxdlist.maxnpxd = 1;
		pxdlist.npxd = 0;
		pxd = &pxdlist.pxd[0];
		PXDaddress(pxd, nxaddr)
		PXDlength(pxd, xlen + n);
		split->pxdlist = &pxdlist;
		if (rc = dtExtendPage(tid, ip, split, btstack))
		{
			nxaddr = addressPXD(pxd);
			if (xaddr != nxaddr)
			{
				/* free relocated extent */
				xlen = lengthPXD(pxd);
				dbFree(ip, nxaddr, (int64)xlen);
			}
			else
			{
				/* free extended delta */
				xlen = lengthPXD(pxd) - n;
				xaddr = addressPXD(pxd) + xlen;
				dbFree(ip, xaddr, (int64)n);
			}
		}
		else
		{
			/* Since i_nblocks could also contain an EA, need to use
			 * the length of the extent to determine the size of the
			 * directory.
			 */
			ip->i_size = lengthPXD(pxd) << ip->i_ipmnt->i_l2bsize;
		}

extendOut:
		DT_PUTPAGE(sbp);
		goto freeKeyName;
	}

	/*
	 *	split leaf page <sp> into <sp> and a new right page <rp>.
	 *
	 * return <rp> pinned and its extent descriptor <rpxd>
	 */
	/*
	 * allocate new directory page extent and
	 * new index page(s) to cover page split(s)
	 *
	 * allocation hint: ?
	 */
	n = btstack->nsplit;
	pxdlist.maxnpxd = pxdlist.npxd = 0;
	xlen = ip->i_ipmnt->i_nbperpage;
	for (pxd = pxdlist.pxd; n > 0; n--, pxd++)
	{
		if ((rc = dbAlloc(ip, 0, (int64)xlen, &xaddr)) == 0)
		{
			PXDaddress(pxd, xaddr);
			PXDlength(pxd, xlen);
			pxdlist.maxnpxd++;
			continue;
		}

		DT_PUTPAGE(sbp);

		/* undo allocation */
		goto splitOut;
	}

	split->pxdlist = &pxdlist;
	if (rc = dtSplitPage(tid, ip, split, &rbp, &rpxd))
	{
		DT_PUTPAGE(sbp);

		/* undo allocation */
		goto splitOut;
	}

	ip->i_size += PSIZE;

	/*
	 * propagate up the router entry for the leaf page just split
	 *
	 * insert a router entry for the new page into the parent page,
	 * propagate the insert/split up the tree by walking back the stack
	 * of (bn of parent page, index of child page entry in parent page)
 	 * that were traversed during the search for the page that split.
	 *
	 * the propagation of insert/split up the tree stops if the root
	 * splits or the page inserted into doesn't have to split to hold
	 * the new entry.
	 *
	 * the parent entry for the split page remains the same, and
	 * a new entry is inserted at its right with the first key and
	 * block number of the new right page.
	 *
	 * There are a maximum of 4 pages pinned at any time:
	 * two children, left parent and right parent (when the parent splits).
	 * keep the child pages pinned while working on the parent.
	 * make sure that all pins are released at exit.
	 */
	while ((parent = BT_POP(btstack)) != NULL)
	{
		/* parent page specified by stack frame <parent> */

		/* keep current child pages (<lp>, <rp>) pinned */
		lbp = sbp;
		lp = sp;
		rp = BT_PAGE(ip, rbp, dtpage_t);

		/*
		 * insert router entry in parent for new right child page <rp>
		 */
		/* get the parent page <sp> */
		DT_GETPAGE(ip, parent->bn, sbp, dtpage_t, PSIZE, sp, rc);
		if (rc)
		{
			DT_PUTPAGE(lbp);
			DT_PUTPAGE(rbp);
			goto splitOut;
		}

	 	/*
		 * The new key entry goes ONE AFTER the index of parent entry,
		 * because the split was to the right.
		 */
		skip = parent->index + 1;

		/*
		 * compute the key for the router entry
		 *
		 * key suffix compression:
		 * for internal pages that have leaf pages as children,
		 * retain only what's needed to distinguish between
		 * the new entry and the entry on the page to its left.
		 * If the keys compare equal, retain the entire key.
		 *
		 * note that compression is performed only at computing
		 * router key at the lowest internal level.
		 * further compression of the key between pairs of higher
		 * level internal pages loses too much information and
		 * the search may fail.
		 * (e.g., two adjacent leaf pages of {a, ..., x} {xx, ...,}
		 * results in two adjacent parent entries (a)(xx).
		 * if split occurs between these two entries, and
		 * if compression is applied, the router key of parent entry
		 * of right page (x) will divert search for x into right
		 * subtree and miss x in the left subtree.)
		 *
		 * the entire key must be retained for the next-to-leftmost
		 * internal key at any level of the tree, or search may fail
		 * (e.g., ?)
		 */
		switch (rp->header.flag & BT_TYPE) {
		case BT_LEAF:
			/*
			 * compute the length of prefix for suffix compression
			 * between last entry of left page and first entry
			 * of right page
			 */
#ifdef	_JFS_OS2
			if ((sp->header.flag & BT_ROOT && skip > 1) ||
			    sp->header.prev != 0 || skip > 1)
			{
				/* compute uppercase router prefix key */
				ciGetLeafPrefixKey(lp, lp->header.nextindex - 1,
						   rp, 0, &key);
			}
			else /* next to leftmost entry of lowest internal level */
			{
				/* compute uppercase router key */
				dtGetKey(rp, 0, &key);
				key.name[key.namlen] = 0;
				ciToUpper(&key);
			}
#endif	/* _JFS_OS2 */

			n = NDTINTERNAL(key.namlen);
			break;

		case BT_INTERNAL:
			dtGetKey(rp, 0, &key);
			n = NDTINTERNAL(key.namlen);
			break;

		default:
jERROR(2,("dtSplitUp(): UFO!\n"));
			break;
		}

		/* unpin left child page */
		DT_PUTPAGE(lbp);

		/*
		 * compute the data for the router entry
		 */
		data->xd = rpxd;	/* child page xd */

		/*
		 * parent page is full - split the parent page
		 */
		if (n > sp->header.freecnt)
		{
			/* init for parent page split */
			split->bp = sbp;
			split->index = skip; /* index at insert */
			split->nslot = n;
			split->key = &key;
			/* split->data = data; */

			/* unpin right child page */
			DT_PUTPAGE(rbp);

	 		/* The split routines insert the new entry,
			 * acquire txLock as appropriate.
			 * return <rp> pinned and its block number <rbn>.
			 */
			rc = (sp->header.flag & BT_ROOT) ?
			    dtSplitRoot(tid, ip, split, &rbp) :
			    dtSplitPage(tid, ip, split, &rbp, &rpxd);
			if (rc)
			{
				DT_PUTPAGE(sbp);
				goto splitOut;
			}

			/* sbp and rbp are pinned */
		}
		/*
		 * parent page is not full - insert router entry in parent page
		 */
		else
		{	
			/*
			 * acquire a transaction lock on the parent page
			 */
			tlck = txLock(tid, ip, sbp, tlckDTREE|tlckENTRY);
			dtlck = (dtlock_t *)&tlck->lock;
			ASSERT(dtlck->index == 0);
			lv = (lv_t *)&dtlck->lv[0];

			/* linelock header */
			lv->offset = 0;
			lv->length = 1;
			dtlck->index++;

			/* linelock stbl of non-root parent page */
			if (!(sp->header.flag & BT_ROOT))
			{
				lv++;
				n = skip >> L2DTSLOTSIZE;
				lv->offset = sp->header.stblindex + n;
				lv->length = ((sp->header.nextindex - 1) >> L2DTSLOTSIZE) - n + 1;
				dtlck->index++;
			}

			dtInsertEntry(sp, skip, &key, data, &dtlck);

			/* exit propagate up */
			break;
		}
	}

	/* unpin current split and its right page */
	DT_PUTPAGE(sbp);
	DT_PUTPAGE(rbp);

	/*
	 * free remaining extents allocated for split
	 */
splitOut:
	n = pxdlist.npxd;
	pxd = &pxdlist.pxd[n];
	for ( ; n < pxdlist.maxnpxd; n++, pxd++)
		dbFree(ip, addressPXD(pxd), (int64)lengthPXD(pxd));

freeKeyName:
#ifdef _JFS_OS2
	freepool(unipool, (caddr_t *)key.name);
#endif

dtSplitUp_Exit:

        MMPHPostdtSplitUp();    /* MMPH Performance Hook */

	return rc;
}


/*
 *	dtSplitPage()
 *
 * function: Split a non-root page of a btree.
 *
 * parameter:
 *
 * return: 0 - success;
 *	   errno - failure;
 *	return split and new page pinned;
 */
static int32
dtSplitPage(
	int32		tid,
	struct inode	*ip,
	dtsplit_t	*split,
	jbuf_t		**rbpp,
	pxd_t		*rpxdp)
{
	int32		rc = 0;
	jbuf_t		*sbp;
	dtpage_t	*sp;
	int64		sbn;
	jbuf_t		*rbp;
	dtpage_t	*rp;	/* new right page allocated */
	int64		rbn;	/* new right page block number */
	jbuf_t		*bp;
	dtpage_t	*p;
	int64		nextbn;
	pxdlist_t	*pxdlist;
	pxd_t		*pxd;
	int32		skip, nextindex, half, left, nxt, off, si;
	ldtentry_t	*ldtentry;
	idtentry_t	*idtentry;
	uint8		*stbl;
	dtslot_t	*f;
	int32		fsi, stblsize;
	tlock_t		*tlck;
	dtlock_t	*sdtlck, *rdtlck, *dtlck;
	lv_t		*slv, *rlv, *lv;
	int32		n;

	/* get split page */
	sbp = split->bp;
	sp = BT_PAGE(ip, sbp, dtpage_t);

#ifdef _JFS_STATISTICS
	++dtstat_split;
#endif	/* _JFS_STATISTICS */

	/*
	 * allocate the new right page for the split
	 */
	pxdlist = split->pxdlist;
	pxd = &pxdlist->pxd[pxdlist->npxd];
	pxdlist->npxd++;
	rbn = addressPXD(pxd);
	rbp = bmAssign(ip, rbn, rbn, PSIZE, bmREAD_BLOCK);
	/* rbp->b_lblkno = rbn; */

jEVENT(0,("dtSplitPage: ip:0x%08x sbp:0x%08x rbp:0x%08x\n", ip, sbp, rbp));

	/*
	 * acquire a transaction lock on the new right page
	 */
	tlck = txLock(tid, ip, rbp, tlckDTREE|tlckNEW);
	rdtlck = (dtlock_t *)&tlck->lock;

	rp = (dtpage_t *)rbp->b_bdata;
	rp->header.self = *pxd;

	/*
	 * acquire a transaction lock on the split page
	 *
	 * action:
	 */
	tlck = txLock(tid, ip, sbp, tlckDTREE|tlckENTRY);
	sdtlck = (dtlock_t *)&tlck->lock;

	/* linelock header of split page */
	ASSERT(sdtlck->index == 0);
	slv = (lv_t *)&sdtlck->lv[0];
	slv->offset = 0;
	slv->length = 1;
	sdtlck->index++;

	/*
	 * initialize/update sibling pointers between sp and rp
	 */
	nextbn = sp->header.next;
	rp->header.next = nextbn;
	rp->header.prev = addressPXD(&sp->header.self);
	sp->header.next = rbn;

	/*
	 * initialize new right page
	 */
	rp->header.flag = sp->header.flag;

	/* compute sorted entry table at start of extent data area */
	rp->header.nextindex = 0;
	rp->header.stblindex = 1;

	n = PSIZE >> L2DTSLOTSIZE;
	rp->header.maxslot = n;
	stblsize = (n + 31) >> L2DTSLOTSIZE;	/* in unit of slot */

	/* init freelist */
	fsi = rp->header.stblindex + stblsize;
	rp->header.freelist = fsi;
	rp->header.freecnt = rp->header.maxslot - fsi;

	/*
	 *	sequential append at tail: append without split
	 *
	 * If splitting the last page on a level because of appending
	 * a entry to it (skip is maxentry), it's likely that the access is
	 * sequential. Adding an empty page on the side of the level is less
	 * work and can push the fill factor much higher than normal.
	 * If we're wrong it's no big deal, we'll just do the split the right
	 * way next time.
	 * (It may look like it's equally easy to do a similar hack for
	 * reverse sorted data, that is, split the tree left,
	 * but it's not. Be my guest.)
	 */
	if (nextbn == 0 && split->index == sp->header.nextindex)
	{
		/* linelock header + stbl (first slot) of new page */
		rlv = (lv_t *)&rdtlck->lv[rdtlck->index];
		rlv->offset = 0;
		rlv->length = 2;
		rdtlck->index++;

		/*
		 * initialize freelist of new right page
		 */
		f = &rp->slot[fsi];
		for (fsi++; fsi < rp->header.maxslot; f++, fsi++)
			f->next = fsi;
		f->next = -1;

		/* insert entry at the first entry of the new right page */
		dtInsertEntry(rp, 0, split->key, split->data, &rdtlck);

		goto out;
	}

	/*
	 *	non-sequential insert (at possibly middle page)
	 */

	/*
	 * update prev pointer of previous right sibling page;
	 */
	if (nextbn != 0)
	{
		DT_GETPAGE(ip, nextbn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
		{
			bmInvalidate(rbp);
			return rc;
		}

		/*
		 * acquire a transaction lock on the next page
		 */
		tlck = txLock(tid, ip, bp, tlckDTREE|tlckRELINK);
jEVENT(0,("dtSplitPage: tlck = 0x%x, ip = 0x%x, bp=0x%x\n", tlck, ip, bp));
		dtlck = (dtlock_t *)&tlck->lock;

		/* linelock header of previous right sibling page */
		lv = (lv_t *)&dtlck->lv[dtlck->index];
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		p->header.prev = rbn;

		DT_PUTPAGE(bp);
	}

	/*
	 * split the data between the split and right pages.
	 */
	skip = split->index;
	half = (PSIZE >> L2DTSLOTSIZE) >> 1; /* swag */
	left = 0;

	/*
	 *	compute fill factor for split pages
	 *
	 * <nxt> traces the next entry to move to rp
	 * <off> traces the next entry to stay in sp
	 */
	stbl = (uint8 *)&sp->slot[sp->header.stblindex];
	nextindex = sp->header.nextindex;
	for (nxt = off = 0; nxt < nextindex; ++off)
	{
		if (off == skip)
			/* check for fill factor with new entry size */
			n = split->nslot;
		else
		{
			si = stbl[nxt];
			switch (sp->header.flag & BT_TYPE) {
			case BT_LEAF:
				ldtentry = (ldtentry_t *)&sp->slot[si];
				n = NDTLEAF(ldtentry->namlen);
				break;

			case BT_INTERNAL:
				idtentry = (idtentry_t *)&sp->slot[si];
				n = NDTINTERNAL(idtentry->namlen);
				break;

			default:
				break;
			}

			++nxt; /* advance to next entry to move in sp */
		}

		left += n;
		if (left >= half)
			break;
	}

	/* <nxt> poins to the 1st entry to move */

	/*
	 *	move entries to right page
	 *
	 * dtMoveEntry() initializes rp and reserves entry for insertion
	 *
	 * split page moved out entries are linelocked;
	 * new/right page moved in entries are linelocked;
	 */
	/* linelock header + stbl of new right page */
	rlv = (lv_t *)&rdtlck->lv[rdtlck->index];
	rlv->offset = 0;
	rlv->length = 5;	
	rdtlck->index++;

	dtMoveEntry(sp, nxt, rp, &sdtlck, &rdtlck);

	sp->header.nextindex = nxt;

	/*
	 * finalize freelist of new right page
	 */
	fsi = rp->header.freelist;
	f = &rp->slot[fsi];
	for (fsi++; fsi < rp->header.maxslot; f++, fsi++)
		f->next = fsi;
	f->next = -1;

	/*
	 * the skipped index was on the left page,
	 */
	if (skip <= off)
	{
		/* insert the new entry in the split page */
		dtInsertEntry(sp, skip, split->key, split->data, &sdtlck);

		/* linelock stbl of split page */
		if (sdtlck->index >= sdtlck->maxcnt)
			sdtlck = (dtlock_t *)txLinelock(sdtlck);
		slv = (lv_t *)&sdtlck->lv[sdtlck->index];
		n = skip >> L2DTSLOTSIZE;
		slv->offset = sp->header.stblindex + n;
		slv->length = ((sp->header.nextindex - 1) >> L2DTSLOTSIZE) - n + 1;
		sdtlck->index++;
	}
	/*
	 * the skipped index was on the right page,
	 */
	else
	{
		/* adjust the skip index to reflect the new position */
		skip -= nxt;

		/* insert the new entry in the right page */
		dtInsertEntry(rp, skip, split->key, split->data, &rdtlck);
	}

out:
	*rbpp = rbp;
	*rpxdp = *pxd;

	ip->i_nblocks += lengthPXD(pxd);

jEVENT(0,("dtSplitPage: ip:0x%08x sp:0x%08x rp:0x%08x\n", ip, sp, rp));
	return 0;
}


/*
 *	dtExtendPage()
 *
 * function: extend 1st/only directory leaf page
 *
 * parameter:
 *
 * return: 0 - success;
 *	   errno - failure;
 *	return extended page pinned;
 */
static int32
dtExtendPage(
	int32		tid,
	inode_t		*ip,
	dtsplit_t	*split,
	btstack_t	*btstack)
{
	int32		rc;
	jbuf_t		*sbp, *pbp;
	dtpage_t	*sp, *pp;
	pxdlist_t	*pxdlist;
	pxd_t		*pxd, *tpxd;
	int64		xaddr, txaddr;
	int32		xlen, xsize;
	int32		newstblindex, newstblsize;
	int32		oldstblindex, oldstblsize;
	int32		fsi, last;
	dtslot_t	*f;
	btframe_t	*parent;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	pxdlock_t	*pxdlock;
	lv_t		*lv;
	uint32		type;
	int32		n;

	/* get page to extend */
	sbp = split->bp;
	sp = BT_PAGE(ip, sbp, dtpage_t);
	
	/* get parent/root page */
	parent = BT_POP(btstack);
	DT_GETPAGE(ip, parent->bn, pbp, dtpage_t, PSIZE, pp, rc);
	if (rc)
		goto out;

	/*
	 *	extend the extent
	 */
	pxdlist = split->pxdlist;
	pxd = &pxdlist->pxd[pxdlist->npxd];
	pxdlist->npxd++;

	xaddr = addressPXD(pxd);
	tpxd = &sp->header.self;
	txaddr = addressPXD(tpxd);
	/* in-place extension */
	if (xaddr == txaddr)
	{
		type = tlckEXTEND;
	}
	/* relocation */
	else
	{
		type = tlckNEW;

		/* save moved extent descriptor for later free */
		tlck = txMaplock(tid, ip, tlckDTREE|tlckRELOCATE);
		pxdlock = (pxdlock_t *)&tlck->lock;
		pxdlock->flag = mlckFREEPXD;
		pxdlock->pxd = sp->header.self;
		pxdlock->index = 1;
	}

	/*
	 *	extend the page
	 */
	sp->header.self = *pxd;

jEVENT(0,("dtExtendPage: ip:0x%08x sbp:0x%08x sp:0x%08x\n", ip, sbp, sp));

	/*
	 * acquire a transaction lock on the extended/leaf page
	 */
	tlck = txLock(tid, ip, sbp, tlckDTREE|type);
	dtlck = (dtlock_t *)&tlck->lock;
	lv = (lv_t *)&dtlck->lv[0];

	/* update buffer extent descriptor of extended page */
	xlen = lengthPXD(pxd);
	xsize = xlen << ip->i_ipmnt->i_l2bsize;
	bmSetXD(sbp, xaddr, xsize);

	/*
	 * copy old stbl to new stbl at start of extended area
	 */
	oldstblindex = sp->header.stblindex;
	oldstblsize = (sp->header.maxslot + 31) >> L2DTSLOTSIZE;
	newstblindex = sp->header.maxslot;
	n = xsize >> L2DTSLOTSIZE;
	newstblsize = (n + 31) >> L2DTSLOTSIZE;
	bcopy(&sp->slot[oldstblindex], &sp->slot[newstblindex], sp->header.nextindex);

	/*
	 * in-line extension: linelock old area of extended page
	 */
	if (type == tlckEXTEND)
	{
		/* linelock header */
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;
		lv++;

		/* linelock new stbl of extended page */
		lv->offset = newstblindex;
		lv->length = newstblsize;
	}
	/*
	 * relocation: linelock whole relocated area
	 */
	else
	{
		lv->offset = 0;
		lv->length = sp->header.maxslot + newstblsize;
	}

	dtlck->index++;

	sp->header.maxslot = n;
	sp->header.stblindex = newstblindex;
	/* sp->header.nextindex remains the same */

	/*
	 * add old stbl region at head of freelist
	 */
	fsi = oldstblindex;
	f = &sp->slot[fsi];
	last = sp->header.freelist;
	for (n = 0; n < oldstblsize; n++, fsi++, f++)
	{
		f->next = last;
		last = fsi;
	}
	sp->header.freelist = last;
	sp->header.freecnt += oldstblsize;

	/*
	 * append free region of newly extended area at tail of freelist
	 */
	/* init free region of newly extended area */
	fsi = n = newstblindex + newstblsize;
	f = &sp->slot[fsi];
	for (fsi++; fsi < sp->header.maxslot; f++, fsi++)
		f->next = fsi;
	f->next = -1;

	/* append new free region at tail of old freelist */
	fsi = sp->header.freelist;
	if (fsi == -1)
		sp->header.freelist = n;
	else
	{
		do {
			f = &sp->slot[fsi];
			fsi = f->next;
		} while (fsi != -1);	

		f->next = n;
	}

	sp->header.freecnt += sp->header.maxslot - n;

	/*
	 * insert the new entry
	 */
	dtInsertEntry(sp, split->index, split->key, split->data, &dtlck);

	/*
	 * linelock any freeslots residing in old extent
	 */
	if (type == tlckEXTEND)
	{
		n = sp->header.maxslot >> 2;
		if (sp->header.freelist < n)
			dtLinelockFreelist(sp, n, &dtlck);
	}
	
	/*
	 *	update parent entry on the parent/root page
	 */
	/*
	 * acquire a transaction lock on the parent/root page
	 */
	tlck = txLock(tid, ip, pbp, tlckDTREE|tlckENTRY);
	dtlck = (dtlock_t *)&tlck->lock;
	lv = (lv_t *)&dtlck->lv[dtlck->index];

	/* linelock parent entry - 1st slot */
	lv->offset = 1;
	lv->length = 1;
	dtlck->index++;

	/* update the parent pxd for page extension */
	tpxd = (pxd_t *)&pp->slot[1];
	*tpxd = *pxd;

	/* Since the directory might have an EA and/or ACL associated with it
	 * we need to make sure we take that into account when setting the
	 * i_nblocks
	 */
	ip->i_nblocks = xlen +
			((ip->i_ea.flag & DXD_EXTENT)?lengthDXD(&ip->i_ea):0) +
			((ip->i_acl.flag & DXD_EXTENT)?lengthDXD(&ip->i_acl):0);

jEVENT(0,("dtExtendPage: ip:0x%08x sbp:0x%08x sp:0x%08x\n", ip, sbp, sp));

out:
	DT_PUTPAGE(pbp);
	return 0;
}


/*
 *	dtSplitRoot()
 *
 * function:
 *	split the full root page into
 *	original/root/split page and new right page
 *	i.e., root remains fixed in tree anchor (inode) and
 *	the root is copied to a single new right child page
 *	since root page << non-root page, and
 *	the split root page contains a single entry for the
 *	new right child page.
 *
 * parameter:
 *
 * return: 0 - success;
 *	   errno - failure;
 *	return new page pinned;
 */
static int32
dtSplitRoot(
	int32		tid,
	inode_t		*ip,
	dtsplit_t	*split,
	jbuf_t		**rbpp)
{
	jbuf_t		*sbp;
	dtroot_t	*sp;
	jbuf_t		*rbp;
	dtpage_t	*rp;
	int64		rbn;
	int32		xlen;
	int32		xsize;
	dtslot_t	*f;
	int8		*stbl;
	int32		fsi, stblsize, n;
	idtentry_t	*s;
	pxd_t		*ppxd;
	pxdlist_t	*pxdlist;
	pxd_t		*pxd;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;

	/* get split root page */
	sbp = split->bp;
	sp = (dtroot_t *)&ip->i_btroot;

	/*
	 *	allocate/initialize a single (right) child page
	 *
	 * N.B. at first split, a one (or two) block to fit new entry
	 * is allocated; at subsequent split, a full page is allocated;
	 */
	pxdlist = split->pxdlist;
	pxd = &pxdlist->pxd[pxdlist->npxd];
	pxdlist->npxd++;
	rbn = addressPXD(pxd);
	xlen = lengthPXD(pxd);
	xsize = xlen << ip->i_ipmnt->i_l2bsize;
	rbp = bmAssign(ip, rbn, rbn, xsize, bmREAD_BLOCK);
	/* rbp->b_lblkno = rbn; */

	/*
	 * acquire a transaction lock on the new right page
	 */
	tlck = txLock(tid, ip, rbp, tlckDTREE|tlckNEW);
	dtlck = (dtlock_t *)&tlck->lock;

	rp = (dtpage_t *)rbp->b_bdata;
	rp->header.flag = (sp->header.flag & BT_LEAF) ? BT_LEAF : BT_INTERNAL;
	rp->header.self = *pxd;

	/* initialize sibling pointers */
	rp->header.next = 0;
	rp->header.prev = 0;

	/*
	 *	move in-line root page into new right page extent
	 */
	/* linelock header + copied entries + new stbl (1st slot) in new page */
	ASSERT(dtlck->index == 0);
	lv = (lv_t *)&dtlck->lv[0];
	lv->offset = 0;
	lv->length = 10; /* 1 + 8 + 1 */
	dtlck->index++;

	n = xsize >> L2DTSLOTSIZE;
	rp->header.maxslot = n;
	stblsize = (n + 31) >> L2DTSLOTSIZE;

	/* copy old stbl to new stbl at start of extended area */
	rp->header.stblindex = DTROOTMAXSLOT;
	stbl = (int8 *)&rp->slot[DTROOTMAXSLOT];
	bcopy(sp->header.stbl, stbl, sp->header.nextindex);
	rp->header.nextindex = sp->header.nextindex;

	/* copy old data area to start of new data area */
	bcopy(&sp->slot[1], &rp->slot[1], IDATASIZE);

	/*
	 * append free region of newly extended area at tail of freelist
	 */
	/* init free region of newly extended area */
	fsi = n = DTROOTMAXSLOT + stblsize;
	f = &rp->slot[fsi];
	for (fsi++; fsi < rp->header.maxslot; f++, fsi++)
		f->next = fsi;
	f->next = -1;

	/* append new free region at tail of old freelist */
	fsi = sp->header.freelist;
	if (fsi == -1)
		rp->header.freelist = n;
	else
	{
		rp->header.freelist = fsi;

		do {
			f = &rp->slot[fsi];
			fsi = f->next;
		} while (fsi != -1);	

		f->next = n;
	}

	rp->header.freecnt = sp->header.freecnt + rp->header.maxslot - n;

	/*
	 * insert the new entry into the new right/child page
	 * (skip index in the new right page will not change)
	 */
	dtInsertEntry(rp, split->index, split->key, split->data, &dtlck);

	/*
	 *	reset parent/root page
	 *
	 * set the 1st entry offset to 0, which force the left-most key
	 * at any level of the tree to be less than any search key.
	 *
	 * The btree comparison code guarantees that the left-most key on any
	 * level of the tree is never used, so it doesn't need to be filled in.
	 */
	/*
	 * acquire a transaction lock on the root page (in-memory inode)
	 */
	tlck = txLock(tid, ip, sbp, tlckDTREE|tlckNEW|tlckBTROOT); /* tlckENTRY ? */
	dtlck = (dtlock_t *)&tlck->lock;

	/* linelock root */
	ASSERT(dtlck->index == 0);
	lv = (lv_t *)&dtlck->lv[0];
	lv->offset = 0;
	lv->length = DTROOTMAXSLOT;
	dtlck->index++;

	/* update page header of root */
	if (sp->header.flag & BT_LEAF)
	{
		sp->header.flag &= ~BT_LEAF;
		sp->header.flag |= BT_INTERNAL;
	}

	/* init the first entry */
	s = (idtentry_t *)&sp->slot[DTENTRYSTART];
	ppxd = (pxd_t *)s;
	*ppxd = *pxd;
	s->next = -1;
	s->namlen = 0;

	stbl = sp->header.stbl;
	stbl[0] = DTENTRYSTART;
	sp->header.nextindex = 1;

	/* init freelist */
	fsi = DTENTRYSTART + 1;
	f = &sp->slot[fsi];

	/* init free region of remaining area */
	for (fsi++; fsi < DTROOTMAXSLOT; f++, fsi++)
		f->next = fsi;
	f->next = -1;

	sp->header.freelist = DTENTRYSTART + 1;
	sp->header.freecnt = DTROOTMAXSLOT - (DTENTRYSTART + 1);

	*rbpp = rbp;

	ip->i_nblocks += lengthPXD(pxd);
	return 0;
}


/*
 *	dtDelete()
 *
 * function: delete the entry(s) referenced by a key.
 *
 * parameter:
 *
 * return:
 */
int32
dtDelete(
	int32		tid,
	inode_t		*ip,
	component_t	*key,
	ino_t		*ino,
	uint32 		flag)
{
	int32		rc = 0;
	int64		bn;
	jbuf_t		*bp;
	dtpage_t	*p;
	int32		index;
	btstack_t	btstack;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;
	int32		i;

	/*
	 *	search for the entry to delete:
	 *
	 * dtSearch() returns (leaf page pinned, index at which to delete).
	 */
	if (rc = dtSearch(ip, key, ino, &btstack, flag))
		return rc;

	/* retrieve search result */
	BT_GETSEARCH(ip, btstack.top, bn, bp, dtpage_t, p, index);

	/*
	 * the leaf page becomes empty, delete the page
	 */
	if (p->header.nextindex == 1)
	{
		/* delete empty page */
		rc = dtDeleteUp(tid, ip, bp, &btstack);
	}
	/*
	 * the leaf page has other entries remaining:
	 *
	 * delete the entry from the leaf page.
	 */
	else
	{
		/*
		 * acquire a transaction lock on the leaf page
		 */
		tlck = txLock(tid, ip, bp, tlckDTREE|tlckENTRY);
		dtlck = (dtlock_t *)&tlck->lock;

		/*
		 * Do not assume that dtlck->index will be zero.  During a
		 * rename within a directory, this transaction may have
		 * modified this page already when adding the new entry.
		 */

		/* linelock header */
		if (dtlck->index >= dtlck->maxcnt)
			dtlck = (dtlock_t *)txLinelock(dtlck);
		lv = (lv_t *)&dtlck->lv[dtlck->index];
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		/* linelock stbl of non-root leaf page */
		if (!(p->header.flag & BT_ROOT))
		{
			if (dtlck->index >= dtlck->maxcnt)
				dtlck = (dtlock_t *)txLinelock(dtlck);
			lv = (lv_t *)&dtlck->lv[dtlck->index];
			i = index >> L2DTSLOTSIZE;
			lv->offset = p->header.stblindex + i;
			lv->length = ((p->header.nextindex - 1) >> L2DTSLOTSIZE) - i + 1;
			dtlck->index++;
		}

		/* free the leaf entry */
		dtDeleteEntry(p, index, &dtlck);

		DT_PUTPAGE(bp);
	}

	return rc;
}


/*
 *	dtDeleteUp()
 *
 * function:
 *	free empty pages as propagating deletion up the tree
 *
 * parameter:
 *
 * return:
 */
static int32
dtDeleteUp(
	int32		tid,
	inode_t		*ip,
	jbuf_t		*fbp,
	btstack_t	*btstack)
{
	int32		rc = 0;
	jbuf_t		*bp;
	dtpage_t	*fp, *p;
	int32		index, nextindex;
	uint8		*stbl;
	int64		xaddr;
	int32		xlen;
	btframe_t	*parent;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;
	pxdlock_t	*pxdlock;
	int32		i;

	/* get page to delete */
	fp = BT_PAGE(ip, fbp, dtpage_t);

	/*
	 *	keep the root leaf page which has become empty
	 */
	if (fp->header.flag & BT_ROOT)
	{
		/*
		 * reset the root
		 *
		 * dtInitRoot() acquires txlock on the root
		 */
		dtInitRoot(tid, ip, ip->i_parent);

		DT_PUTPAGE(fbp);

		return 0;
	}

	/*
	 *	free the non-root leaf page
	 */
	/*
	 * acquire a transaction lock on the page
	 *
	 * write FREEXTENT|NOREDOPAGE log record
	 * N.B. linelock is overlaid as freed extent descriptor, and
	 * the buffer page is freed;
	 */
	tlck = txMaplock(tid, ip, tlckDTREE|tlckFREE);
	pxdlock = (pxdlock_t *)&tlck->lock;
	pxdlock->flag = mlckFREEPXD;
	pxdlock->pxd = fp->header.self;
	pxdlock->index = 1;

	/* update sibling pointers */
	if (rc = dtRelink(tid, ip, fp))
		return rc;

	xlen = lengthPXD(&fp->header.self);
	ip->i_nblocks -= xlen;

	/* free/invalidate its buffer page */
	bmInvalidate(fbp);	

	/*
	 *	propagate page deletion up the directory tree
	 *
	 * If the delete from the parent page makes it empty,
	 * continue all the way up the tree.
	 * stop if the root page is reached (which is never deleted) or
	 * if the entry deletion does not empty the page.
	 */
	while ((parent = BT_POP(btstack)) != NULL)
	{
		/* pin the parent page <sp> */
		DT_GETPAGE(ip, parent->bn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;
		
		/*
		 * free the extent of the child page deleted
		 */
		index = parent->index;

		/*
		 * delete the entry for the child page from parent
		 */
		nextindex = p->header.nextindex;

		/*
		 * the parent has the single entry being deleted:
		 *
		 * free the parent page which has become empty.
		 */
		if (nextindex == 1)
		{
			/*
			 * keep the root internal page which has become empty
			 */
			if (p->header.flag & BT_ROOT)
			{
				/*
				 * reset the root
				 *
				 * dtInitRoot() acquires txlock on the root
				 */
				dtInitRoot(tid, ip, ip->i_parent);

				DT_PUTPAGE(bp);

				return 0;
			}
			/*
			 * free the parent page
			 */
			else
			{
				/*
				 * acquire a transaction lock on the page
				 *
				 * write FREEXTENT|NOREDOPAGE log record
				 */
				tlck = txMaplock(tid, ip, tlckDTREE|tlckFREE);
				pxdlock = (pxdlock_t *)&tlck->lock;
				pxdlock->flag = mlckFREEPXD;
				pxdlock->pxd = p->header.self;
				pxdlock->index = 1;

				/* update sibling pointers */
				if (rc = dtRelink(tid, ip, p))
					return rc;

				xlen = lengthPXD(&p->header.self);
				ip->i_nblocks -= xlen;
		
				/* free/invalidate its buffer page */
				bmInvalidate(bp);	

				/* propagate up */
				continue;
			}
		}

		/*
		 * the parent has other entries remaining:
		 *
		 * delete the router entry from the parent page.
		 */
		/*
		 * acquire a transaction lock on the page
		 *
		 * action: router entry deletion
		 */
		tlck = txLock(tid, ip, bp, tlckDTREE|tlckENTRY);
		dtlck = (dtlock_t *)&tlck->lock;

		/* linelock header */
		if (dtlck->index >= dtlck->maxcnt)		// D231252
			dtlck = (dtlock_t *)txLinelock(dtlck);	// D231252
		lv = (lv_t *)&dtlck->lv[dtlck->index];		// D231252
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		/* linelock stbl of non-root leaf page */
		if (!(p->header.flag & BT_ROOT))
		{
			if (dtlck->index < dtlck->maxcnt)	// D231252
				lv++;
// BEGIN D231252
			else
			{
				dtlck = (dtlock_t *)txLinelock(dtlck);
				lv = (lv_t *)&dtlck->lv[0];
			}
// END D231252
			i = index >> L2DTSLOTSIZE;
			lv->offset = p->header.stblindex + i;
			lv->length = ((p->header.nextindex - 1) >> L2DTSLOTSIZE) - i + 1;
			dtlck->index++;
		}

		/* free the router entry */
		dtDeleteEntry(p, index, &dtlck);

		/* reset key of new leftmost entry of level (for consistency) */
		if (index == 0 &&
		    ((p->header.flag & BT_ROOT) || p->header.prev == 0))
			dtTruncateEntry(p, 0, &dtlck);

		/* unpin the parent page */
		DT_PUTPAGE(bp);

		/* exit propagation up */
		break;
	}

	ip->i_size -= PSIZE;
	return 0;
}


/*
 * NAME:        dtRelocate()
 *
 * FUNCTION:    relocate dtpage (internal or leaf) of directory;
 *              This function is mainly used by defragfs utility.
 */
int32 dtRelocate(int32	tid,
        inode_t		*ip,
	int64		lmxaddr,
        pxd_t           *opxd,	/* old PXD */
        int64           nxaddr)	/* new PXD */

{
	int32		rc = 0;
	int32		cmp;
	jbuf_t		*bp, *pbp, *lbp, *rbp;
	dtpage_t	*p, *pp, *rp, *lp;
	int64		bn;
	int32		index;
	btstack_t	btstack;
	pxd_t		*pxd;
	int64		oxaddr, nextbn, prevbn;
	int32		xlen, xsize;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	pxdlock_t	*pxdlock;
	int8		*stbl;
	lv_t		*lv;

	oxaddr = addressPXD(opxd);
	xlen = lengthPXD(opxd);

jEVENT(0,("dtRelocate: lmxaddr:%lld xaddr:%lld:%lld xlen:%d\n",
	lmxaddr, oxaddr, nxaddr, xlen));

	/*
	 *	1. get the internal parent dtpage covering
	 *	router entry for the tartget page to be relocated;
	 */
	rc = dtSearchNode(ip, lmxaddr, opxd, &btstack);
	if (rc)
		return rc;

	/* retrieve search result */
	BT_GETSEARCH(ip, btstack.top, bn, pbp, dtpage_t, pp, index);
jEVENT(0,("dtRelocate: parent router entry validated.\n"));
		
	/*
	 *	2. relocate the target dtpage
	 */
	/* read in the target page from src extent */
	DT_GETPAGE(ip, oxaddr, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
	{
		/* release the pinned parent page */
		DT_PUTPAGE(pbp);
                return rc;
	}

	/*
	 * read in sibling pages if any to update sibling pointers;
	 */
	rbp = NULL;
	if (p->header.next)
	{
		nextbn = p->header.next;
		DT_GETPAGE(ip, nextbn, rbp, dtpage_t, PSIZE, rp, rc);
		if (rc)
		{
			DT_PUTPAGE(bp);
			DT_PUTPAGE(pbp);
			return (rc);
		}
	}

	lbp = NULL;
	if (p->header.prev)
	{
		prevbn = p->header.prev;
		DT_GETPAGE(ip, prevbn, lbp, dtpage_t, PSIZE, lp, rc);
		if (rc)
		{
			DT_PUTPAGE(bp);
			DT_PUTPAGE(pbp);
			if (rbp)
				DT_PUTPAGE(rbp);
			return (rc);
		}
	}

	/* at this point, all xtpages to be updated are in memory */

	/*
	 * update sibling pointers of sibling dtpages if any;
	 */
	if (lbp)
	{
		tlck = txLock(tid, ip, lbp, tlckDTREE|tlckRELINK);
		dtlck = (dtlock_t *)&tlck->lock;
		/* linelock header */
		ASSERT(dtlck->index == 0);
		lv = (lv_t *)&dtlck->lv[0];
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		lp->header.next = nxaddr;
		DT_PUTPAGE(lbp);
	}

	if (rbp)
	{
		tlck = txLock(tid, ip, rbp, tlckDTREE|tlckRELINK);
		dtlck = (dtlock_t *)&tlck->lock;
		/* linelock header */
		ASSERT(dtlck->index == 0);
		lv = (lv_t *)&dtlck->lv[0];
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		rp->header.prev = nxaddr;
		DT_PUTPAGE(rbp);
	}

	/*
	 * update the target dtpage to be relocated
	 *
	 * write LOG_REDOPAGE of LOG_NEW type for dst page
	 * for the whole target page (logredo() will apply
	 * after image and update bmap for allocation of the
	 * dst extent), and update bmap for allocation of
	 * the dst extent;
	 */
	tlck = txLock(tid, ip, bp, tlckDTREE|tlckNEW);
	dtlck = (dtlock_t *)&tlck->lock;
	/* linelock header */
	ASSERT(dtlck->index == 0);
	lv = (lv_t *)&dtlck->lv[0];

	/* update the self address in the dtpage header */
	pxd = &p->header.self;
	PXDaddress(pxd, nxaddr);

	/* the dst page is the same as the src page, i.e.,
	 * linelock for afterimage of the whole page;
	 */
	lv->offset = 0;
	lv->length = p->header.maxslot;
	dtlck->index++;

	/* update the buffer extent descriptor of the dtpage */
        xsize = xlen << ip->i_ipmnt->i_l2bsize;
        bmSetXD(bp, nxaddr, xsize);

	/* unpin the relocated page */
	DT_PUTPAGE(bp);
jEVENT(0,("dtRelocate: target dtpage relocated.\n"));

	/* the moved extent is dtpage, then a LOG_NOREDOPAGE log rec
	 * needs to be written (in logredo(), the LOG_NOREDOPAGE log rec
	 * will also force a bmap update ).
	 */

	/*
	 *	3. acquire maplock for the source extent to be freed;
	 */
out:
	/* for dtpage relocation, write a LOG_NOREDOPAGE record
	 * for the source dtpage (logredo() will init NoRedoPage
	 * filter and will also update bmap for free of the source
	 * dtpage), and upadte bmap for free of the source dtpage;
	 */
	tlck = txMaplock(tid, ip, tlckDTREE|tlckFREE);
	pxdlock = (pxdlock_t *)&tlck->lock;
	pxdlock->flag = mlckFREEPXD;
	PXDaddress(&pxdlock->pxd, oxaddr);
	PXDlength(&pxdlock->pxd, xlen);
	pxdlock->index = 1;

	/*
	 *	4. update the parent router entry for relocation;
	 *
	 * acquire tlck for the parent entry covering the target dtpage;
	 * write LOG_REDOPAGE to apply after image only;
	 */
jEVENT(0,("dtRelocate: update parent router entry.\n"));
	tlck = txLock(tid, ip, pbp, tlckDTREE|tlckENTRY);
	dtlck = (dtlock_t *)&tlck->lock;
	lv = (lv_t *)&dtlck->lv[dtlck->index];
	
	/* update the PXD with the new address */
	stbl = DT_GETSTBL(pp);
	pxd = (pxd_t *)&pp->slot[stbl[index]];
	PXDaddress(pxd, nxaddr);
	lv->offset = stbl[index];
	lv->length = 1;
	dtlck->index++;

	/* unpin the parent dtpage */
	DT_PUTPAGE(pbp);

	return rc;
}


/*
 * NAME:	dtSearchNode()
 *
 * FUNCTION:	Search for an dtpage containing a specified address
 *              This function is mainly used by defragfs utility.
 *
 * NOTE:	Search result on stack, the found page is pinned at exit.
 *		The result page must be an internal dtpage.
 *		lmxaddr give the address of the left most page of the
 *		dtree level, in which the required dtpage resides.
 */
static int32 dtSearchNode(
	inode_t		*ip,
	int64		lmxaddr,/* xaddr of left most page of the level */
	pxd_t		*kpxd,	/* search key pxd */
	btstack_t	*btstack)
{
	int32           rc = 0;
        int64           bn;
        jbuf_t          *bp;
        dtpage_t        *p;
	int32		psize = 288;	/* initial in-line directory */
        int8            *stbl;
        int32           i;
        pxd_t           *pxd;
        btframe_t       *btsp;

	BT_CLR(btstack);        /* reset stack */
	
	/*
         *      descend tree to the level with specified leftmost page
         *
	 *  by convention, root bn = 0.
         */
	for (bn = 0;  ; )
	{
		/* get/pin the page to search */
		DT_GETPAGE(ip, bn, bp, dtpage_t, psize, p, rc);
		if (rc)
                        return rc;

		/* does the xaddr of leftmost page of the levevl
		 * matches levevl search key ?
		 */
		if (p->header.flag & BT_ROOT)
		{
			if (lmxaddr == 0)
				break;
		}
		else if (addressPXD(&p->header.self) == lmxaddr)
			break;

		/*
		 * descend down to leftmost child page
		 */
		if (p->header.flag & BT_LEAF)
			return ESTALE;

		/* get the leftmost entry */
		stbl = DT_GETSTBL(p);
                pxd = (pxd_t *)&p->slot[stbl[0]];

                /* get the child page block address */
                bn = addressPXD(pxd);
		psize = lengthPXD(pxd) << ip->i_ipmnt->i_l2bsize;

                /* unpin the parent page */
                DT_PUTPAGE(bp);
	}

	/*
	 *	search each page at the current levevl
	 */
loop:
	stbl = DT_GETSTBL(p);
	for (i = 0; i < p->header.nextindex; i++)
        {
		pxd = (pxd_t *)&p->slot[stbl[i]];

		/* found the specified router entry */
		if (addressPXD(pxd) == addressPXD(kpxd) &&
		    lengthPXD(pxd) == lengthPXD(kpxd))	
		{
			btsp = btstack->top;
			btsp->bn = bn;
			btsp->index = i;
			btsp->bp = bp;

			return 0;
		}
	}

	/* get the right sibling page if any */
	if (p->header.next != 0L)
		bn = p->header.next;
	else
	{	
		DT_PUTPAGE(bp);
		return ESTALE;
	}

	/* unpin current page */
	DT_PUTPAGE(bp);

	/* get the right sibling page */
	DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
		return rc;

	goto loop;
}


/*
 *	dtRelink()
 *
 * function:
 *	link around a freed page.
 *
 * parameter:
 *	fp:	page to be freed
 *
 * return:
 */
static int32
dtRelink(
	int32		tid,
	inode_t		*ip,
	dtpage_t	*p)
{
	int32		rc;
	jbuf_t		*bp;
	int64		nextbn, prevbn;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;

	nextbn = p->header.next;
	prevbn = p->header.prev;

	/* update prev pointer of the next page */
	if (nextbn != 0)
	{
		DT_GETPAGE(ip, nextbn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;

		/*
		 * acquire a transaction lock on the next page
		 *
		 * action: update prev pointer;
		 */
		tlck = txLock(tid, ip, bp, tlckDTREE|tlckRELINK);
jEVENT(0,("dtRelink nextbn: tlck = 0x%x, ip = 0x%x, bp=0x%x\n", tlck, ip, bp));
		dtlck = (dtlock_t *)&tlck->lock;

		/* linelock header */
		if (dtlck->index >= dtlck->maxcnt)		// D231252
			dtlck = (dtlock_t *)txLinelock(dtlck);	// D231252
		lv = (lv_t *)&dtlck->lv[dtlck->index];		// D231252
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		p->header.prev = prevbn;
		DT_PUTPAGE(bp);
	}

	/* update next pointer of the previous page */
	if (prevbn != 0)
	{
		DT_GETPAGE(ip, prevbn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;

		/*
		 * acquire a transaction lock on the prev page
		 *
		 * action: update next pointer;
		 */
		tlck = txLock(tid, ip, bp, tlckDTREE|tlckRELINK);
jEVENT(0,("dtRelink prevbn: tlck = 0x%x, ip = 0x%x, bp=0x%x\n", tlck, ip, bp));
		dtlck = (dtlock_t *)&tlck->lock;

		/* linelock header */
		if (dtlck->index >= dtlck->maxcnt)		// D231252
			dtlck = (dtlock_t *)txLinelock(dtlck);	// D231252
		lv = (lv_t *)&dtlck->lv[dtlck->index];		// D231252
		lv->offset = 0;
		lv->length = 1;
		dtlck->index++;

		p->header.next = nextbn;
		DT_PUTPAGE(bp);
	}

	return 0;
}


/*
 *	dtInitRoot()
 *
 * initialize directory root (inline in inode)
 */
void
dtInitRoot(
	int32	tid,
	inode_t	*ip,
	ino_t	idotdot)
{
	dtroot_t	*p;
	int32		fsi;
	dtslot_t	*f;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;

	/*
	 * acquire a transaction lock on the root
	 *
	 * action: directory initialization;
	 */
	tlck = txLock(tid, ip, (jbuf_t *)&ip->i_bxflag, tlckDTREE|tlckENTRY|tlckBTROOT);
	dtlck = (dtlock_t *)&tlck->lock;

	/* linelock root */
	ASSERT(dtlck->index == 0);
	lv = (lv_t *)&dtlck->lv[0];
	lv->offset = 0;
	lv->length = DTROOTMAXSLOT;
	dtlck->index++;

	p = (dtroot_t *)&ip->i_btroot;

	p->header.flag = DXD_INDEX | BT_ROOT | BT_LEAF;

	p->header.nextindex = 0;

	/* init freelist */
	fsi = 1;
	f = &p->slot[fsi];

	/* init data area of root */
	for (fsi++; fsi < DTROOTMAXSLOT; f++, fsi++)
		f->next = fsi;
	f->next = -1;

	p->header.freelist = 1;
	p->header.freecnt = 8;

	/* init '..' entry in btroot */
	p->header.idotdot = idotdot;

	ip->i_size = IDATASIZE;

	return;
}
#ifdef Drova
/*
 *	dtRead()
 *
 * function: read directory entries sequentially
 *	from the specified entry offset
 *
 * parameter:
 *
 * return: offset = (pn, index) of start entry
 *	of next jfs_readdir()/dtRead()
 *
 * ref. jfs_readdir().
 */
int32
dtRead(
	inode_t		*ip,
	int32		*offset,
	int32		ubytes,		/* user buffer size */
	int32		*tbytes,	/* number of bytes written to <tdp> */
	struct dirent	*tdp)		/* translation buffer */
{
	int32		rc = 0;
	struct dtoffset{
		int16	pn;
		int16	index;
	} *dtoffset = (struct dtoffset *)offset;
	int64		bn;
	jbuf_t		*bp;
	dtpage_t	*p;
	int32		index;
	int8		*stbl;
	btstack_t	btstack;
	btframe_t	*parent;
	int32		i, next;
	ldtentry_t	*d;
	dtslot_t	*t;
	int32		d_reclen, d_namlen, len;
	UniChar		*d_name;
	struct direntDot *tdpDot;
	struct direntDotDot *tdpDotDot;

	/*
	 * get starting entry to translate:
	 *
	 * dtReadFirst()/dtReadNext() returns bp pinned
	 */
	if (*offset == 0)
	{
		/* build "." entry */
		if ((*tbytes + sizeof(struct direntDot)) > ubytes)
			goto out;

		tdpDot = (struct direntDot *)tdp;
		*tdpDot = edir.dot;
		tdpDot->d_ino = ip->i_number;

		*tbytes = sizeof(struct direntDot);
		tdp = (struct dirent *)((caddr_t)tdp + sizeof(struct direntDot));

		/* build ".." entry */
		if ((*tbytes + sizeof(struct direntDotDot)) > ubytes)
			goto out;

		tdpDotDot = (struct direntDotDot *)tdp;
		*tdpDotDot = edir.dotdot;
		tdpDotDot->dd_ino = ip->i_parent;

		*tbytes += sizeof(struct direntDotDot);
		tdp = (struct dirent *)((caddr_t)tdp + sizeof(struct direntDotDot));

		if ((rc = dtEmpty(ip)) == 0)
		{
			*offset = -1;
			return rc;
		}

		if (rc = dtReadFirst(ip, &btstack))
			return rc;
	}
	else
		if (rc = dtReadNext(ip, offset, &btstack))
			return rc;

	/* get start leaf page and index */
	BT_GETSEARCH(ip, btstack.top, bn, bp, dtpage_t, p, index);

	/* offset beyond directory eof ? */
	if (bn < 0)
		return 0;

loop:
	/*
	 * read/translate on-disk directory entry (struct ldtentry_t)
	 * into readdir() directory entry (struct dirent).
	 *
	 * unused space in the directory are not returned
	 * to the user, i.e., more than requested size may have to
	 * be read from directory to fill the user's buffer.
	 */
	stbl = DT_GETSTBL(p);

	/* translate each directory entry in directory page buffer */
	for (i = index; i < p->header.nextindex; i++)
	{
		d = (ldtentry_t *)&p->slot[stbl[i]];

		/* if user buffer does not have space
		 * for the current entry, stop.
		 */
		d_namlen = d->namlen;
		d_reclen = DIRENTSIZ(d_namlen);
		if ((*tbytes + d_reclen) > ubytes)
			goto out;

		/* construct struct dirent from struct ldtentry */
		tdp->d_offset = *offset;	/* !!! */
		tdp->d_ino = d->inumber;
		tdp->d_reclen = d_reclen;
		tdp->d_namlen = d_namlen;

		d_name = tdp->d_name;

		/* copy the name of head/only segment */
		len = MIN(d_namlen, DTLHDRDATALEN);
		UniStrncpy(d_name, d->name, len);

		/* copy name in the additional segment(s) */
		next = d->next;
		while (next >= 0)
		{
			t = (dtslot_t *)&p->slot[next];
			d_name += len;
			d_namlen -= len;
			len = MIN(d_namlen, DTSLOTDATALEN);
			UniStrncpy(d_name, t->name, len);

			next = t->next;
		}

		/* terminate name with NULL */
		tdp->d_name[tdp->d_namlen] = '\0';

		/* advance to next dirent */
		*tbytes += d_reclen;
		tdp = (struct dirent *)((caddr_t)tdp + d_reclen);

		dtoffset->index++;
	}

	/*
	 * get next leaf page
	 */
	if (p->header.flag & BT_ROOT)
		goto out;
	else
	{
		bn = p->header.next;
		if (bn == 0)
			goto out;
	}

	/* unpin previous leaf page */
	DT_PUTPAGE(bp);

	/* get next leaf page */
	DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
		return rc;

	/* update offset (pn:index) for new page */
	dtoffset->pn++;
	dtoffset->index = index = 0;

	goto loop;

out:
	DT_PUTPAGE(bp);

	return rc;
}
#endif

/*
 *	dtReadFirst()
 *
 * function: get the leftmost page of the directory
 */
static int32
dtReadFirst(
	inode_t		*ip,
	btstack_t	*btstack)
{
	int32		rc = 0;
	int64		bn;
	int32		psize = 288;	/* initial in-line directory */
	jbuf_t		*bp;
	dtpage_t	*p;
	int8		*stbl;
	btframe_t	*btsp;
	pxd_t		*xd;

	BT_CLR(btstack);	/* reset stack */

	/*
	 *	descend leftmost path of the tree
	 *
	 * by convention, root bn = 0.
	 */
	for (bn = 0; ;)
	{
		DT_GETPAGE(ip, bn, bp, dtpage_t, psize, p, rc);
		if (rc)
			return rc;
		
		/*
		 * leftmost leaf page
		 */
		if (p->header.flag & BT_LEAF)
		{
			/* return leftmost entry */
			btsp = btstack->top;
			btsp->bn = bn;
			btsp->index = 0;
			btsp->bp = bp;

			return 0;
		}

		/*
		 * descend down to leftmost child page
		 */
		/* push (bn, index) of the parent page/entry */
		BT_PUSH(btstack, bn, 0);

		/* get the leftmost entry */
		stbl = DT_GETSTBL(p);
		xd = (pxd_t *)&p->slot[stbl[0]];

		/* get the child page block address */
		bn = addressPXD(xd);
		psize = lengthPXD(xd) << ip->i_ipmnt->i_l2bsize;

		/* unpin the parent page */
		DT_PUTPAGE(bp);
	}
}
#ifdef Drova
/*
 *	dtReadNext()
 *
 * function: get the page of the specified offset (pn:index)
 *
 * return: if (offset > eof), bn = -1;
 *
 * note: if index > nextindex of the target leaf page,
 * start with 1st entry of next leaf page;
 */
static int32
dtReadNext(
	inode_t		*ip,
	int32		*offset,
	btstack_t	*btstack)
{
	int32		rc = 0;
	struct dtoffset{
		int16	pn;
		int16	index;
	} *dtoffset = (struct dtoffset *)offset;
	int64		bn;
	jbuf_t		*bp;
	dtpage_t	*p;
	int32		index;
	int32		pn;
	int8		*stbl;
	btframe_t	*btsp, *parent;
	pxd_t		*xd;

	/*
	 * get leftmost leaf page pinned
	 */
	if (rc = dtReadFirst(ip, btstack))
		return rc;

	/* get leaf page */
	BT_GETSEARCH(ip, btstack->top, bn, bp, dtpage_t, p, index);

	/* get the start offset (pn:index) */
	pn = dtoffset->pn;
	index = dtoffset->index;

	/* start at leftmost page ? */
	if (pn == 0)
	{
		/* offset beyond eof ? */
		if (index < p->header.nextindex)
			goto out;

		if (p->header.flag & BT_ROOT)
		{
			bn = -1;
			goto out;
		}

		/* start with 1st entry of next leaf page */
		dtoffset->pn++;
		dtoffset->index = index = 0;
		goto a;
	}

	/* start at non-leftmost page: scan parent pages for large pn */
	if (p->header.flag & BT_ROOT)
	{
		bn = -1;
		goto out;
	}

	/* start after next leaf page ? */
	if (pn > 1)
		goto b;

	/* get leaf page pn = 1 */
a:
	bn = p->header.next;

	/* unpin leaf page */
	DT_PUTPAGE(bp);

	/* offset beyond eof ? */
	if (bn == 0)
	{
		bn = -1;
		goto out;
	}

	goto c;

	/*
	 * scan last internal page level to get target leaf page
	 */
b:
	/* unpin leftmost leaf page */
	DT_PUTPAGE(bp);

	/* get left most parent page */
	btsp = btstack->top;
	parent = btsp - 1;
	bn = parent->bn;
	DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
		return rc;
	
	/* scan parent pages at last internal page level */
	while (pn >= p->header.nextindex)
	{
		pn -= p->header.nextindex;

		/* get next parent page address */
		bn = p->header.next;

		/* unpin current parent page */
		DT_PUTPAGE(bp);

		/* offset beyond eof ? */
		if (bn == 0)
		{
			bn = -1;
			goto out;
		}
		
		/* get next parent page */
		DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;

		/* update parent page stack frame */
		parent->bn = bn;
	}

	/* get leaf page address */
	stbl = DT_GETSTBL(p);
	xd = (pxd_t *)&p->slot[stbl[pn]];
	bn = addressPXD(xd);

	/* unpin parent page */
	DT_PUTPAGE(bp);
		
	/*
	 * get target leaf page
	 */
c:
	DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
		return rc;

	/*
	 * leaf page has been completed:
	 * start with 1st entry of next leaf page
	 */
	if (index >= p->header.nextindex)
	{
		bn = p->header.next;

		/* unpin leaf page */
		DT_PUTPAGE(bp);

		/* offset beyond eof ? */
		if (bn == 0)
		{
			bn = -1;
			goto out;
		}

		/* get next leaf page */
		DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;
		
		/* start with 1st entry of next leaf page */
		dtoffset->pn++;
		dtoffset->index = 0;
	}

out:
	/* return target leaf page pinned */
	btsp = btstack->top;
	btsp->bn = bn;
	btsp->index = dtoffset->index;
	btsp->bp = bp;

	return 0;
}
#endif

#ifdef	_JFS_OS2
#define	FINDDIR_READ	1	/* read directory entries */
#define	FINDDIR_FIND	2	/* find directory entries */

/*
 *	dtFind()
 *
 * function: find directory entries of <ip> according to <pattern>
 *	in implementation independent format.  Return entries
 *      beginning with the entry following <lastmatch>.  If
 *	<lastmatch> has been deleted from the directory, the first
 *	entry which sorts after <lastmatch> is the first entry returned.
 *	<offset> is composed of <page_number, index> and has the
 * 	following values:
 *		<-1,-1>	end of directory reached
 *		<-1,0>	looking for "."
 *		<-1,1>	looking for ".."
 *		<0,0>	search from start of directory
 *		anything else is within the directory.
 *
 * ref. jfs_finddir().
 */
dtFind(
	inode_t		*ip,
	component_t	*pattern,
	UniChar		*lastmatch,
	int32		*offset,
	uint32		*count,
	int32		ubytes,		/* user buffer size */
	int32		*tbytes,	/* number of bytes written to <tdp> */
	struct dirent	*tdp)		/* translation buffer */
{
	int32		rc = 0;
	int64		bn;
	jbuf_t		*bp;
	dtpage_t	*p;
	int32		index;
	int8		*stbl;
	btstack_t	btstack;
	int32		i, next;
	ldtentry_t	*d;
	dtslot_t	*t;
	int32		d_reclen, d_namlen, len;
	UniChar		*d_name;
	struct direntDot *tdpDot;
	struct direntDotDot *tdpDotDot;
	int32		find;	/* read or match ? */
	component_t	dname;
	component_t	sname;
	int32		prefix, nmatch, n = 0;
	ino_t		ino = 0;
	struct dtoffset {
		int16	pn;
		int16	index;
	} *dtoffset = (struct dtoffset *)offset;
	inode_t		*tip;

        MMPHPredtFind();        /* MMPH Performance Hook */

	/*
	 * compute the first entry to start matching
	 */
	/* find non-wildcard prefix of pattern if any */
	dname.name = (UniChar *)allocpool(unipool, 0);
	if (dname.name == 0)
	{
		rc = ENOMEM;
		goto out2;
	}
	find = dtPrefixWild(pattern->name, &dname);
	prefix = dname.namlen;	

	/*
	 * find if "." and/or ".." are to be returned
	 */
	if ((dtoffset->pn == -1) && (dtoffset->index == 0))
	{
		dtoffset->index = 1;
		if ((find == FINDDIR_READ) ||
		   (dtCompareWild(pattern->name, nameDot, &nmatch) == 0))
		{
			if ((*tbytes + sizeof(struct direntDot)) > ubytes)
				goto out1;

			/* build "." entry */
			tdpDot = (struct direntDot *)tdp;
			*tdpDot = edir.dot;
			tdpDot->d_ino = ip->i_number;

			*tbytes = sizeof(struct direntDot);

			if (++n >= *count)
				goto out1;

			tdp = (struct dirent *)((caddr_t)tdp
					+ sizeof(struct direntDot));
		}
	}

	if ((dtoffset->pn == -1) && (dtoffset->index == 1))
	{
		dtoffset->index = 2;
		if ((find == FINDDIR_READ) ||
		   (dtCompareWild(pattern->name, nameDotDot, &nmatch) == 0))
		{
			if ((*tbytes + sizeof(struct direntDotDot)) > ubytes)
				goto out1;

			/* build ".." entry */
			tdpDotDot = (struct direntDotDot *)tdp;
			*tdpDotDot = edir.dotdot;
			tdpDotDot->dd_ino = ip->i_parent;

			*tbytes += sizeof(struct direntDotDot);

			if (++n >= *count)
				goto out1;

			tdp = (struct dirent *)((caddr_t)tdp
					+ sizeof(struct direntDotDot));
		}
	}

	/* we're past the pseudo directory entries.  set a real offset.
	 */
	if (dtoffset->pn == -1)
		*offset = 0;

	/* check for empty directory
	 */
	if ((rc = dtEmpty(ip)) == 0)
	{
		*offset = -1;
		goto out1;
	}

	/* If resuming a search (e.g., fs_findnext) then use the
	 * lastmatch name to find the current location in the
	 * directory.  If there is a prefix before any wildcard,
	 * use that to find the starting location; otherwise
	 * search from the beginning of the directory.
	 */
	if ((*offset > 0) || (prefix > 0))
	{
		if (*offset)
		{
			sname.name = lastmatch;
			sname.namlen = UniStrlen(lastmatch);
		}
		else
			sname = dname;

                rc = dtSearch(ip, &sname, &ino, &btstack, JFS_FINDDIR);
        }
        else
        {
                /* locate the start of the directory */
                if (rc = dtReadFirst(ip, &btstack))
		{
			*offset = -1;
			goto out1;
		}
        }

	/* get start leaf page and index */
	BT_GETSEARCH(ip, btstack.top, bn, bp, dtpage_t, p, index);

	/* offset beyond directory eof ? */
	if (bn < 0)
	{
		*offset = -1;
		goto out1;
	}

	/* if resuming a search and the last matching name is still
	 * found (as indicated by the return of an inumber), bump the
	 * index past the last match
	 */
	if (*offset > 0 && ino != 0)
		index++;

	/*
	 *	linear scan for wildcard match
	 */
loop:
	/*
	 * read/translate on-disk directory entry (struct ldtentry_t)
	 * into readdir() directory entry (struct dirent).
	 *
	 * unused space in the directory are not returned
	 * to the user, i.e., more than requested size may have to
	 * be read from directory to fill the user's buffer.
	 */
	stbl = DT_GETSTBL(p);

	/* translate each directory entry in directory page buffer */
	for (i = index; i < p->header.nextindex; i++)
	{
		d = (ldtentry_t *)&p->slot[stbl[i]];

		/* if user buffer does not have space
		 * for the current entry, stop.
		 */
		d_namlen = d->namlen;
		d_reclen = DIRENTSIZ(d_namlen);
		if ((*tbytes + d_reclen) > ubytes)
			goto out;

		if (find == FINDDIR_FIND)
		{
			/* pattern matching */
			dtGetKey(p, i, &dname);
			dname.name[dname.namlen] = '\0';

			/* uppercase entry name to compare */
			ciToUpper(&dname);
			rc = dtCompareWild(pattern->name, dname.name, &nmatch);
			if (rc > 0)
			{
				/* advance the offset value and keep looking */
				dtoffset->index++;
				continue;
			}
			else if (rc < 0)
			{
				/* no more matching entries */
				*offset = -1;
				goto out;
			}
		}

		/* advance the offset value */
		dtoffset->index++;

		/* construct struct dirent from struct ldtentry */
		tdp->d_offset = *offset;
		tdp->d_ino = d->inumber;
		tdp->d_reclen = d_reclen;
		tdp->d_namlen = d_namlen;

		d_name = tdp->d_name;

		/* copy the name of head/only segment */
		len = MIN(d_namlen, DTLHDRDATALEN);
		UniStrncpy(d_name, d->name, len);

		/* copy name in the additional segment(s) */
		next = d->next;
		while (next >= 0)
		{
			t = (dtslot_t *)&p->slot[next];
			d_name += len;
			d_namlen -= len;
			len = MIN(d_namlen, DTSLOTDATALEN);
			UniStrncpy(d_name, t->name, len);

			next = t->next;
		}

		/* terminate name with NULL */
		tdp->d_name[tdp->d_namlen] = '\0';

		/* advance to next dirent */
		*tbytes += d_reclen;
		tdp = (struct dirent *)((caddr_t)tdp + d_reclen);

		/* requested number of entries copied? */
		if (++n >= *count)
			goto out;
	}

	/*
	 * get next leaf page
	 */
	if (p->header.flag & BT_ROOT)
	{
		*offset = -1;
		goto out;
	}
	else
	{
		bn = p->header.next;
		if (bn == 0)
		{
			*offset = -1;
			goto out;
		}
	}

	/* unpin previous leaf page */
	DT_PUTPAGE(bp);

	/* get next leaf page */
	DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
		//return rc;
           goto dtFind_Exit;

	dtoffset->pn++;
	dtoffset->index = index = 0;

	goto loop;

out:
	rc = 0;
	DT_PUTPAGE(bp);
out1:
	freepool(unipool, (caddr_t *)dname.name);
out2:
	*count = n;

dtFind_Exit:

        MMPHPostdtFind();       /* MMPH Performance Hook */

	return rc;
}


/*
 *	dtPrefixWild()
 *
 * return: prefix string up to but not including the 1st wildcard
 *	character and the position of the 1st wildcard character of
 *	the specified pattern if any.
 *	e.g., given abc*xyz, it would return 3 as firstw, and
 *	abc as prefix.
 */
static int32
dtPrefixWild(
	UniChar		*pattern,	/* pattern with wildcard */
	component_t	*prefix)
{
	UniChar		*p;
	int32		i;
	UniChar		*s = prefix->name;	/* non-wildcard prefix */

	/* find the first wildcard character in the pattern */
	for (p = pattern, i = 0; *p != '\0'; p++, i++)
		if ((*p == '*' || *p == '?') ||
		  (*p == '.' && (*(p+1) == 0 || *(p+1) == '*' || *(p+1) == '?')))
			break;

	prefix->namlen = i;	/* position of 1st wildcard char */

	/* pattern has non-wildcard prefix */
	if (i > 0)
	{
		/* copy non-wildcard prefix of pattern to prefix */
		UniStrncpy(s, pattern, i);

		/* make prefix NULL terminated */
		s[i] = '\0';
	}
	/* pattern has wildcard prefix */
	else
	{
		while (*p == '*')
			p++;

		if (*p == '\0')
			return FINDDIR_READ;
	}

	return FINDDIR_FIND;
}
	

/*
 *	dtCompareWild()
 *
 * function: wildcard match
 *  * - sequence of zero or more characters
 *  ? - any one single character except period OR no characters at period
 *	or end of name
 *
 * return: 0 - match
 *	  >0 - w sorts before s
 *	  <0 - w sorts after s
 */
static int32
dtCompareWild(
	register UniChar	*w,	/* search pattern with wildcard */
	register UniChar	*s,	/* target string */
	register int32		*m)	/* number of matched char */
{
	UniChar	*justpaststar = 0;	/* Position just past last '*'	*/
	UniChar	*string_ptr;		/* Position of string when matching * */
	int32	string_count;		/* saves *m when matching '*'	*/
	UniChar	*ptr;			/* general purpose pointer */
	UniChar *string_dot = 0;	/* Set if period added to string */
	UniChar *pattern_dot = 0;	/* Set if period added to pattern */
	int32	qseen = 0;		/* Set if we've seen a '?' */
	int32	rc;

/* If no dot in s, add one */

	for (ptr = s; *ptr; ptr++)
		if (*ptr == '.')
			goto dotfound1;

/* No dot found in s */

	string_dot = ptr;
	*ptr = '.';
	*(++ptr) = 0;

dotfound1:

/* If no dot in w, then
 *	if no '*' in w, add a dot at the end,
 *	else add a dot at the end only if we added one to s
 */
	for (ptr = w; *ptr; ptr++)
	{
		if (*ptr == '.')
			goto dotfound2;
		if (*ptr == '*' && string_dot == 0)
			goto dotfound2;
	}

/* No dot found in w */
	pattern_dot = ptr;
	*ptr = '.';
	*(++ptr) = 0;

dotfound2:
	*m = 0;
	while (1)
	{
		if (*w == '*')
		{
			w++;
			justpaststar = w;
			string_count = *m;
			string_ptr = s;
		}
		else if (*s == 0)		/* End of string */
			if (*w == '?')
				w++;		/* Skip past ? in pattern */
			else if (*w)
				goto sorts_before;
			else
				goto match_found;
		else if (*w == '?')
		{
			qseen = 1;
			w++;
			if (*s != '.')		/* If dot, '?' matches nothing */
				s++;		/* '?' matches whatever */
		}
		else if (*w == *s)
		{
			w++;
                     if( (s == nameDotDot)&&(*w != '.')) s++; //PS21092003
			s++;
			(*m)++;
		}
		else if (justpaststar)		/* We've seen a '*' */
		{
			w = justpaststar;
			*m = ++string_count;	/* Match one character */
			s = ++string_ptr;
		}
		else if (*s < *w)
			goto sorts_before;
		else if (qseen)			/* If we've already seen a ?
						 * we don't stop reading dir
						 */
			goto sorts_before;
		else
			goto sorts_after;
	}
sorts_before:
	rc = 1;
	goto remove_dots;
sorts_after:
	rc = -1;
	goto remove_dots;
match_found:
	rc = 0;

remove_dots:
	if (string_dot)
		*string_dot = 0;
	if (pattern_dot)
		*pattern_dot = 0;
	return	rc;
}
#endif	/* _JFS_OS2 */


/*
 *	dtCompare()
 *
 * function: compare search key with an (leaf/internal) entry
 *
 * return:
 *	< 0 if k is < record
 *	= 0 if k is = record
 *	> 0 if k is > record
 */
static int32
dtCompare(
	component_t	*key,	/* search key */
	dtpage_t	*p,	/* directory page */
	int32		si)	/* entry slot index */
{
	register int32	rc;
	register UniChar *kname, *name;
	register int32	klen, namlen, len;
	ldtentry_t	*lh;
	idtentry_t	*ih;
	dtslot_t	*t;

	/*
	 * force the left-most key on internal pages, at any level of
	 * the tree, to be less than any search key.
	 * this obviates having to update the leftmost key on an internal
	 * page when the user inserts a new key in the tree smaller than
	 * anything that has been stored.
	 *
	 * (? if/when dtSearch() narrows down to 1st entry (index = 0),
	 * at any internal page at any level of the tree,
	 * it descends to child of the entry anyway -
	 * ? make the entry as min size dummy entry)
	 *
	 * if (e->index == 0 && h->prevpg == P_INVALID && !(h->flags & BT_LEAF))
	 * return (1);
	 */

	kname = key->name;
	klen = key->namlen;
	
	/*
	 * leaf page entry
	 */
	if (p->header.flag & BT_LEAF)
	{
		lh = (ldtentry_t *)&p->slot[si];
		si = lh->next;
		name = lh->name;
		namlen = lh->namlen;
		len = MIN(namlen, DTLHDRDATALEN);
	}
	/*
	 * internal page entry
	 */
	else
	{
		ih = (idtentry_t *)&p->slot[si];
		si = ih->next;
		name = ih->name;
		namlen = ih->namlen;
		len = MIN(namlen, DTIHDRDATALEN);
	}

	/* compare with head/only segment */
	len = MIN(klen, len);
	if ((rc = (*kname - *name)) ||
	    (rc = UniStrncmp(kname, name, len)))
		return rc;

	klen -= len;
	namlen -= len;

	/* compare with additional segment(s) */
	kname +=len;
	while (klen > 0 && namlen > 0)
	{
		/* compare with next name segment */
		t = (dtslot_t *)&p->slot[si];
		len = MIN(namlen, DTSLOTDATALEN);
		len = MIN(klen, len);
		name = t->name;
		if ((rc = (*kname - *name)) ||
		    (rc = UniStrncmp(kname, name, len)))
			return rc;

		klen -= len;
		namlen -= len;
		kname +=len;
		si = t->next;
	}

	return (klen - namlen);
}


#ifdef Drova
/*
 *	dtGetLeafPrefixKey()
 *
 * function: compute prefix of suffix compression
 *	     from two adjacent leaf entries
 *	     across page boundary
 *
 * return:
 *	Number of prefix bytes needed to distinguish b from a.
 *
 * note: key compression
 *
 * suffix compression:
 *  each key stores prefix bytes sufficient to distinguish between
 *  two adjacent successor entries.
 *
 * prefix compression:
 *  each key is stored as a tuple of the form:
 *	<length_of_prefix_key, key_suffix>
 *  where length_of_prefix_key = number of prefix bytes of
 *  the previous entry identical to the current entry, and
 *  key_suffix = the rest of the key value of the current entry.
 */
static void
dtGetLeafPrefixKey(
	dtpage_t	*lp,	/* left page */
	int32		li,	/* left entry index */
	dtpage_t	*rp,	/* right page */
	int32		ri,	/* right entry index */
	component_t	*key)
{
	register int32	klen, namlen;
	register UniChar *pl, *pr, *kname;
	UniChar		lname[JFS_NAME_MAX+1];
	component_t	lkey = {0, lname};
	UniChar		rname[JFS_NAME_MAX+1];
	component_t	rkey = {0, rname};

	/* get left and right key */
	dtGetKey(lp, li, &lkey);
	dtGetKey(rp, ri, &rkey);

	/* compute prefix */
	klen = 0;
	kname = key->name;
	namlen = MIN(lkey.namlen, rkey.namlen);
	for (pl = lkey.name, pr = rkey.name;
	     namlen;
	     pl++, pr++, namlen--, klen++, kname++)
	{
		*kname = *pr;
		if (*pl != *pr)
		{
			key->namlen = klen + 1;
			return;
		}
	}

	/* l->namlen <= r->namlen since l <= r */
	if (lkey.namlen < rkey.namlen)
	{
		*kname = *pr;
		key->namlen = klen + 1;
	}
	else /* l->namelen == r->namelen */
		key->namlen = klen;

	return;
}
#endif

#ifdef	_JFS_OS2
/*
 *	ciCompare()
 *
 * function: compare search key with an (leaf/internal) entry
 *
 * return:
 *	< 0 if k is < record
 *	= 0 if k is = record
 *	> 0 if k is > record
 */
static int32
ciCompare(
	component_t	*key,	/* search key */
	dtpage_t	*p,	/* directory page */
	int32		si)	/* entry slot index */
{
	register int32		rc;
	register UniChar	*kname, *name, x;
	register int32		klen, namlen, len;
	ldtentry_t	*lh;
	idtentry_t	*ih;
	dtslot_t	*t;
	int32		i;

	/*
	 * force the left-most key on internal pages, at any level of
	 * the tree, to be less than any search key.
	 * this obviates having to update the leftmost key on an internal
	 * page when the user inserts a new key in the tree smaller than
	 * anything that has been stored.
	 *
	 * (? if/when dtSearch() narrows down to 1st entry (index = 0),
	 * at any internal page at any level of the tree,
	 * it descends to child of the entry anyway -
	 * ? make the entry as min size dummy entry)
	 *
	 * if (e->index == 0 && h->prevpg == P_INVALID && !(h->flags & BT_LEAF))
	 * return (1);
	 */

	kname = key->name;
	klen = key->namlen;
	
	/*
	 * leaf page entry
	 */
	if (p->header.flag & BT_LEAF)
	{
		lh = (ldtentry_t *)&p->slot[si];
		si = lh->next;
		name = lh->name;
		namlen = lh->namlen;
		len = MIN(namlen, DTLHDRDATALEN);
	}
	/*
	 * internal page entry
	 */
	else
	{
		ih = (idtentry_t *)&p->slot[si];
		si = ih->next;
		name = ih->name;
		namlen = ih->namlen;
		len = MIN(namlen, DTIHDRDATALEN);
	}

	/* compare with head/only segment */
	len = MIN(klen, len);
	for (i = 0; i < len; i++, kname++, name++)
	{
		/* uppercase the characer to match */
		x = UniToupper(*name);
		if (rc = *kname - x)
			return rc;
	}

	klen -= len;
	namlen -= len;

	/* compare with additional segment(s) */
	while (klen > 0 && namlen > 0)
	{
		/* compare with next name segment */
		t = (dtslot_t *)&p->slot[si];
		len = MIN(namlen, DTSLOTDATALEN);
		len = MIN(klen, len);
		name = t->name;
		for (i = 0; i < len; i++, kname++, name++)
		{
			/* uppercase the characer to match */
			x = UniToupper(*name);
			if (rc = *kname - x)
				return rc;
		}

		klen -= len;
		namlen -= len;
		si = t->next;
	}

	return (klen - namlen);
}


/*
 *	ciGetLeafPrefixKey()
 *
 * function: compute prefix of suffix compression
 *	     from two adjacent leaf entries
 *	     across page boundary
 *
 * return:
 *	Number of prefix bytes needed to distinguish b from a.
 */
static void
ciGetLeafPrefixKey(
	dtpage_t	*lp,	/* left page */
	int32		li,	/* left entry index */
	dtpage_t	*rp,	/* right page */
	int32		ri,	/* right entry index */
	component_t	*key)
{
	register int32	klen, namlen;
	register UniChar *pl, *pr, *kname;
	UniChar		lname[JFS_NAME_MAX+1];
	component_t	lkey = {0, lname};
	UniChar		rname[JFS_NAME_MAX+1];
	component_t	rkey = {0, rname};

	/* get left and right key */
	dtGetKey(lp, li, &lkey);
	lkey.name[lkey.namlen] = 0;
	ciToUpper(&lkey);

	dtGetKey(rp, ri, &rkey);
	rkey.name[rkey.namlen] = 0;
	ciToUpper(&rkey);

	/* compute prefix */
	klen = 0;
	kname = key->name;
	namlen = MIN(lkey.namlen, rkey.namlen);
	for (pl = lkey.name, pr = rkey.name;
	     namlen;
	     pl++, pr++, namlen--, klen++, kname++)
	{
		*kname = *pr;
		if (*pl != *pr)
		{
			key->namlen = klen + 1;
			return;
		}
	}

	/* l->namlen <= r->namlen since l <= r */
	if (lkey.namlen < rkey.namlen)
	{
		*kname = *pr;
		key->namlen = klen + 1;
	}
	else /* l->namelen == r->namelen */
		key->namlen = klen;

	return;
}

#endif	/* _JFS_OS2 */


/*
 *	dtGetKey()
 *
 * function: get key of the entry
 */
static void
dtGetKey(
	dtpage_t	*p,
	int32		i,	/* entry index */
	component_t	*key)
{
	int32		si;
	int8		*stbl;
	ldtentry_t	*lh;
	idtentry_t	*ih;
	dtslot_t	*t;
	int32		namlen, len;
	UniChar		*name, *kname;

	/* get entry */
	stbl = DT_GETSTBL(p);
	si = stbl[i];
	if (p->header.flag & BT_LEAF)
	{
		lh = (ldtentry_t *)&p->slot[si];
		si = lh->next;
		namlen = lh->namlen;
		name = lh->name;
		len = MIN(namlen, DTLHDRDATALEN);
	}
	else
	{
		ih = (idtentry_t *)&p->slot[si];
		si = ih->next;
		namlen = ih->namlen;
		name = ih->name;
		len = MIN(namlen, DTIHDRDATALEN);
	}

	key->namlen = namlen;
	kname = key->name;

	/*
	 * move head/only segment
	 */
	UniStrncpy(kname, name, len);

	/*
	 * move additional segment(s)
	 */
	while (si >= 0)
	{
		/* get next segment */
		t = &p->slot[si];
		kname += len;
		namlen -= len;
		len = MIN(namlen, DTSLOTDATALEN);
		UniStrncpy(kname, t->name, len);

		si = t->next;
	}
}


/*
 *	dtInsertEntry()
 *
 * function: allocate free slot(s) and
 *	     write a leaf/internal entry
 *
 * return: entry slot index
 */
static void
dtInsertEntry(
	dtpage_t	*p,	/* directory page */
	int32		index,	/* new entry index */
	component_t	*key,	/* key */
	ddata_t		*data,	/* data */
	dtlock_t	**dtlock)	/* dt linelock */
{
	dtslot_t	*h, *t, *f;
	ldtentry_t	*lh;
	idtentry_t	*ih;
	int32		hsi, fsi, klen, len, nextindex;
	UniChar		*kname, *name;
	int8		*stbl;
	pxd_t		*xd;
	dtlock_t	*dtlck = *dtlock;
	lv_t		*lv;
	int32		xsi, n;

	klen = key->namlen;
	kname = key->name;

	/* allocate a free slot */
	hsi = fsi = p->header.freelist;
	h = &p->slot[fsi];
	p->header.freelist = h->next;
	--p->header.freecnt;

	/* open new linelock */
	if (dtlck->index >= dtlck->maxcnt)
		dtlck = (dtlock_t *)txLinelock(dtlck);
			
	lv = (lv_t *)&dtlck->lv[dtlck->index];
	lv->offset = hsi;

	/* write head/only segment */
	if (p->header.flag & BT_LEAF)
	{
		lh = (ldtentry_t *)h;
		lh->next = h->next;
		lh->inumber = data->ino;
		lh->namlen = klen;
		name = lh->name;
		len = MIN(klen, DTLHDRDATALEN);
	}
	else
	{
		ih = (idtentry_t *)h;
		ih->next = h->next;
		xd = (pxd_t *)ih;
		*xd = data->xd;
		ih->namlen = klen;
		name = ih->name;
		len = MIN(klen, DTIHDRDATALEN);
	}

	UniStrncpy(name, kname, len);	

	n = 1;
	xsi = hsi;

	/* write additional segment(s) */
	t = h;
	klen -= len;
	while (klen)
	{
		/* get free slot */
		fsi = p->header.freelist;
		t = &p->slot[fsi];
		p->header.freelist = t->next;
		--p->header.freecnt;

		/* is next slot contiguous ? */
		if (fsi != xsi + 1)
		{
			/* close current linelock */
			lv->length = n;
			dtlck->index++;

			/* open new linelock */
			if (dtlck->index < dtlck->maxcnt)
				lv++;
			else
			{
				dtlck = (dtlock_t *)txLinelock(dtlck);
				lv = (lv_t *)&dtlck->lv[0];
			}

			lv->offset = fsi;
			n = 0;
		}

		kname += len;
		len = MIN(klen, DTSLOTDATALEN);
		UniStrncpy(t->name, kname, len);	

		n++;
		xsi = fsi;
		klen -= len;
	}

	/* close current linelock */
	lv->length = n;
	dtlck->index++;

	*dtlock = dtlck;

	/* terminate last/only segment */
	if (h == t)
	{
		/* single segment entry */
		if (p->header.flag & BT_LEAF)
			lh->next = -1;
		else
			ih->next = -1;
	}
	else
		/* multi-segment entry */
		t->next = -1;

	/* if insert into middle, shift right succeeding entries in stbl */
	stbl = DT_GETSTBL(p);
	nextindex = p->header.nextindex;
	if (index < nextindex)
		memmove(stbl + index + 1, stbl + index, nextindex - index);

	stbl[index] = hsi;

	/* advance next available entry index of stbl */
	++p->header.nextindex;
}


/*
 *	dtMoveEntry()
 *
 * function: move entries from split/left page to new/right page
 *
 *	nextindex of dst page and freelist/freecnt of both pages
 *	are updated.
 */
static void
dtMoveEntry(
	dtpage_t	*sp,	/* src page */
	int32		si,	/* src start entry index to move */
	dtpage_t	*dp,	/* dst page */
	dtlock_t	**sdtlock,
	dtlock_t	**ddtlock)
{
	int32		ssi, next;	/* src slot index */
	int32		di;		/* dst entry index */
	int32		dsi;		/* dst slot index */
	int8		*sstbl, *dstbl;	/* sorted entry table */	
	int32		snamlen, len;
	ldtentry_t	*slh, *dlh;
	idtentry_t	*sih, *dih;
	dtslot_t	*h, *s, *d, *f;
	dtlock_t	*sdtlck = *sdtlock, *ddtlck = *ddtlock;
	lv_t		*slv, *dlv;
	int32		xssi, ns, nd;
	int32		sfsi;

	sstbl = (int8 *)&sp->slot[sp->header.stblindex];
	dstbl = (int8 *)&dp->slot[dp->header.stblindex];

	dsi = dp->header.freelist;	/* first (whole page) free slot */
	sfsi = sp->header.freelist;

	/* linelock destination entry slot */
	dlv = (lv_t *)&ddtlck->lv[ddtlck->index];
	dlv->offset = dsi;

	/* linelock source entry slot */
	slv = (lv_t *)&sdtlck->lv[sdtlck->index];
	slv->offset = sstbl[si];
	xssi = slv->offset - 1;

	/*
	 * move entries
	 */
	ns = nd = 0;
	for (di = 0; si < sp->header.nextindex; si++, di++)
	{
		ssi = sstbl[si];
		dstbl[di] = dsi;

		/* is next slot contiguous ? */
		if (ssi != xssi + 1)
		{
			/* close current linelock */
			slv->length = ns;
			sdtlck->index++;

			/* open new linelock */
			if (sdtlck->index < sdtlck->maxcnt)
				slv++;
			else
			{
				sdtlck = (dtlock_t *)txLinelock(sdtlck);
				slv = (lv_t *)&sdtlck->lv[0];
			}

			slv->offset = ssi;
			ns = 0;
		}

		/*
		 * move head/only segment of an entry
		 */
		/* get dst slot */
		h = d = &dp->slot[dsi];

		/* get src slot and move */
		s = &sp->slot[ssi];
		if (sp->header.flag & BT_LEAF)
		{
			/* get source entry */
			slh = (ldtentry_t *)s;
			snamlen = slh->namlen;

			len = MIN(snamlen, DTLHDRDATALEN);
			dlh = (ldtentry_t *)h;
			bcopy(slh, dlh, 6 + len*2);

			next = slh->next;

			/* update dst head/only segment next field */
			dsi++;
			dlh->next = dsi;
		}
		else
		{
			sih = (idtentry_t *)s;
			snamlen = sih->namlen;

			len = MIN(snamlen, DTIHDRDATALEN);
			dih = (idtentry_t *)h;
			bcopy(sih, dih, 10 + len*2);
			next = sih->next;

			dsi++;
			dih->next = dsi;
		}

		/* free src head/only segment */
		s->next = sfsi;
		s->cnt = 1;
		sfsi = ssi;

		ns++;
		nd++;
		xssi = ssi;

		/*
		 * move additional segment(s) of the entry
		 */
		snamlen -= len;
		while ((ssi = next) >= 0)
		{
			/* is next slot contiguous ? */
			if (ssi != xssi + 1)
			{
				/* close current linelock */
				slv->length = ns;
				sdtlck->index++;

				/* open new linelock */
				if (sdtlck->index < sdtlck->maxcnt)
					slv++;
				else
				{
					sdtlck = (dtlock_t *)txLinelock(sdtlck);
					slv = (lv_t *)&sdtlck->lv[0];
				}

				slv->offset = ssi;
				ns = 0;
			}	

			/* get next source segment */
			s = &sp->slot[ssi];

			/* get next destination free slot */
			d++;

			len = MIN(snamlen, DTSLOTDATALEN);
			UniStrncpy(d->name, s->name, len);

			ns++;
			nd++;
			xssi = ssi;

			dsi++;
			d->next = dsi;

			/* free source segment */
			next = s->next;
			s->next = sfsi;
			s->cnt = 1;
			sfsi = ssi;

			snamlen -= len;
		} /* end while */

		/* terminate dst last/only segment */
		if (h == d)
		{
			/* single segment entry */
			if (dp->header.flag & BT_LEAF)
				dlh->next = -1;
			else
				dih->next = -1;
		}
		else
			/* multi-segment entry */
			d->next = -1;
	} /* end for */

	/* close current linelock */
	slv->length = ns;
	sdtlck->index++;
	*sdtlock = sdtlck;

	dlv->length = nd;
	ddtlck->index++;
	*ddtlock = ddtlck;

	/* update source header */
	sp->header.freelist = sfsi;
	sp->header.freecnt += nd;
	
	/* update destination header */
	dp->header.nextindex = di;

	dp->header.freelist = dsi;
	dp->header.freecnt -= nd;
}


/*
 *	dtDeleteEntry()
 *
 * function: free a (leaf/internal) entry
 *
 * log freelist header, stbl, and each segment slot of entry
 * (even though last/only segment next field is modified,
 * physical image logging requires all segment slots of
 * the entry logged to avoid applying previous updates
 * to the same slots)
 */
static void
dtDeleteEntry(
	dtpage_t	*p,	/* directory page */
	int32		fi,	/* free entry index */
	dtlock_t	**dtlock)
{
	int32		fsi;	/* free entry slot index */
	int8		*stbl;
	dtslot_t	*t;
	int32		si, freecnt;
	dtlock_t	*dtlck = *dtlock;
	lv_t		*lv;
	int32		xsi, n;

	/* get free entry slot index */
	stbl = DT_GETSTBL(p);
	fsi = stbl[fi];

	/* open new linelock */
	if (dtlck->index >= dtlck->maxcnt)
		dtlck = (dtlock_t *)txLinelock(dtlck);
	lv = (lv_t *)&dtlck->lv[dtlck->index];

	lv->offset = fsi;

	/* get the head/only segment */
	t = &p->slot[fsi];
	if (p->header.flag & BT_LEAF)
		si = ((ldtentry_t *)t)->next;
	else
		si = ((idtentry_t *)t)->next;
	t->next = si;
	t->cnt = 1;

	n = freecnt = 1;
	xsi = fsi;

	/* find the last/only segment */
	while (si >= 0)
	{
		/* is next slot contiguous ? */
		if (si != xsi + 1)
		{
			/* close current linelock */
			lv->length = n;
			dtlck->index++;

			/* open new linelock */
			if (dtlck->index < dtlck->maxcnt)
				lv++;
			else
			{
				dtlck = (dtlock_t *)txLinelock(dtlck);
				lv = (lv_t *)&dtlck->lv[0];
			}

			lv->offset = si;
			n = 0;
		}

		n++;
		xsi = si;
		freecnt++;

		t = &p->slot[si];
		t->cnt = 1;
		si = t->next;
	}

	/* close current linelock */
	lv->length = n;
	dtlck->index++;

	*dtlock = dtlck;

	/* update freelist */
	t->next = p->header.freelist;
	p->header.freelist = fsi;
	p->header.freecnt += freecnt;

	/* if delete from middle,
	 * shift left the succedding entries in the stbl
	 */
	si = p->header.nextindex;
	if (fi < si - 1)
		memmove(&stbl[fi], &stbl[fi + 1], si - fi - 1);

	p->header.nextindex--;
}


/*
 *	dtTruncateEntry()
 *
 * function: truncate a (leaf/internal) entry
 *
 * log freelist header, stbl, and each segment slot of entry
 * (even though last/only segment next field is modified,
 * physical image logging requires all segment slots of
 * the entry logged to avoid applying previous updates
 * to the same slots)
 */
static void
dtTruncateEntry(
	dtpage_t	*p,	/* directory page */
	int32		ti,	/* truncate entry index */
	dtlock_t	**dtlock)
{
	int32		tsi;	/* truncate entry slot index */
	int8		*stbl;
	dtslot_t	*t;
	int32		si, freecnt;
	dtlock_t	*dtlck = *dtlock;
	lv_t		*lv;
	int32		fsi, xsi, n;

	/* get free entry slot index */
	stbl = DT_GETSTBL(p);
	tsi = stbl[ti];

	/* open new linelock */
	if (dtlck->index >= dtlck->maxcnt)
		dtlck = (dtlock_t *)txLinelock(dtlck);
	lv = (lv_t *)&dtlck->lv[dtlck->index];

	lv->offset = tsi;

	/* get the head/only segment */
	t = &p->slot[tsi];
	ASSERT(p->header.flag & BT_INTERNAL);
	((idtentry_t *)t)->namlen = 0;
	si = ((idtentry_t *)t)->next;
	((idtentry_t *)t)->next = -1;

	n = 1;
	freecnt = 0;
	fsi = si;
	xsi = tsi;

	/* find the last/only segment */
	while (si >= 0)
	{
		/* is next slot contiguous ? */
		if (si != xsi + 1)
		{
			/* close current linelock */
			lv->length = n;
			dtlck->index++;

			/* open new linelock */
			if (dtlck->index < dtlck->maxcnt)
				lv++;
			else
			{
				dtlck = (dtlock_t *)txLinelock(dtlck);
				lv = (lv_t *)&dtlck->lv[0];
			}

			lv->offset = si;
			n = 0;
		}

		n++;
		xsi = si;
		freecnt++;

		t = &p->slot[si];
		t->cnt = 1;
		si = t->next;
	}

	/* close current linelock */
	lv->length = n;
	dtlck->index++;

	*dtlock = dtlck;

	/* update freelist */
	if (freecnt == 0)
		return;
	t->next = p->header.freelist;
	p->header.freelist = fsi;
	p->header.freecnt += freecnt;
}


/*
 *	dtLinelockFreelist()
 */
static void
dtLinelockFreelist(
	dtpage_t	*p,	/* directory page */
	int32		m,	/* max slot index */
	dtlock_t	**dtlock)
{
	int32		fsi;	/* free entry slot index */
	dtslot_t	*t;
	int32		si;
	dtlock_t	*dtlck = *dtlock;
	lv_t		*lv;
	int32		xsi, n;

	/* get free entry slot index */
	fsi = p->header.freelist;

	/* open new linelock */
	if (dtlck->index >= dtlck->maxcnt)
		dtlck = (dtlock_t *)txLinelock(dtlck);
	lv = (lv_t *)&dtlck->lv[dtlck->index];

	lv->offset = fsi;

	n = 1;
	xsi = fsi;

	t = &p->slot[fsi];
	si = t->next;

	/* find the last/only segment */
	while (si < m && si >= 0)
	{
		/* is next slot contiguous ? */
		if (si != xsi + 1)
		{
			/* close current linelock */
			lv->length = n;
			dtlck->index++;

			/* open new linelock */
			if (dtlck->index < dtlck->maxcnt)
				lv++;
			else
			{
				dtlck = (dtlock_t *)txLinelock(dtlck);
				lv = (lv_t *)&dtlck->lv[0];
			}

			lv->offset = si;
			n = 0;
		}

		n++;
		xsi = si;

		t = &p->slot[si];
		si = t->next;
	}

	/* close current linelock */
	lv->length = n;
	dtlck->index++;

	*dtlock = dtlck;
}


#ifdef Drova
/*
 * NAME: dtModify
 *
 * FUNCTION: Modify the inode number part of a directory entry
 *
 * PARAMETERS:
 *	tid	- Transaction id
 *	ip	- Inode of parent directory
 *	key	- Name of entry to be modified
 *	orig_ino	- Original inode number expected in entry
 *	new_ino	- New inode number to put into entry
 *	flag	- JFS_RENAME
 *
 * RETURNS:
 *	ESTALE	- If entry found does not match orig_ino passed in
 *	ENOENT	- If no entry can be found to match key
 *	0	- If successfully modified entry
 */
int32
dtModify(
	int32		tid,
	inode_t		*ip,
	component_t	*key,
	ino_t		*orig_ino,
	ino_t		new_ino,
	uint32 		flag)
{
	int32		rc;
	int64		bn;
	jbuf_t		*bp;
	dtpage_t	*p;
	int32		index;
	btstack_t	btstack;
	tlock_t		*tlck;
	dtlock_t	*dtlck;
	lv_t		*lv;
	int8		*stbl;
	int32		entry_si;	/* entry slot index */
	ldtentry_t	*entry;

	/*
	 *	search for the entry to modify:
	 *
	 * dtSearch() returns (leaf page pinned, index at which to modify).
	 */
	if (rc = dtSearch(ip, key, orig_ino, &btstack, flag))
		return rc;

	/* retrieve search result */
	BT_GETSEARCH(ip, btstack.top, bn, bp, dtpage_t, p, index);

	/*
	 * acquire a transaction lock on the leaf page of named entry
	 */
	tlck = txLock(tid, ip, bp, tlckDTREE|tlckENTRY);
	dtlck = (dtlock_t *)&tlck->lock;

	/* get slot index of the entry */
	stbl = DT_GETSTBL(p);
	entry_si = stbl[index];

	/* linelock entry */
	ASSERT(dtlck->index == 0);
	lv = (lv_t *)&dtlck->lv[0];
	lv->offset = entry_si;
	lv->length = 1;
	dtlck->index++;

	/* get the head/only segment */
	entry = (ldtentry_t *) &p->slot[entry_si];

	/* substitute the inode number of the entry */
	entry->inumber = new_ino;

	/* unpin the leaf page */
	DT_PUTPAGE(bp);

	return 0;
}
#endif

#ifdef	_JFS_OS2
/*
 * NAME: dtChangeCase
 *
 * FUNCTION: Changes the case of a directory entry in place
 *
 * PARAMETERS:
 *	tid	- Transaction id
 *	ip	- Inode of parent directory
 *	key	- Name of entry to be modified
 *	inum	- Inode number that we expect to find
 *	flag 	- JFS_RENAME
 *
 * RETURNS:
 *	ENOENT	- If no entry can be found to match key
 *	0	- If successfully modified entry
 */
int32
dtChangeCase(
	int32		tid,
	inode_t		*ip,
	component_t	*key,
	ino_t		*inum,
	uint32		flag)
{
	int64		bn;
	jbuf_t		*bp;
	btstack_t	btstack;
	dtlock_t	*dtlck;
	int32		index;
	int32		klen;
	UniChar		*kname;
	ldtentry_t	*leaf_entry;
	int32		len;
	lv_t		*lv;
	int32		n;
	dtpage_t	*p;
	int32		psi;
	int32		rc;
	int32		si;
	dtslot_t	*slot;
	int8		*stbl;
	tlock_t		*tlck;

	/*
	 * Search for entry to modify
	 */
	if (rc = dtSearch(ip, key, inum, &btstack, flag))
		return rc;

	/*
	 * retrieve search results
	 */
	BT_GETSEARCH(ip, btstack.top, bn, bp, dtpage_t, p, index);

	/*
	 * acquire transaction lock on leaf page
	 */
	tlck = txLock(tid, ip, bp, tlckDTREE|tlckENTRY);
	dtlck = (dtlock_t *)&tlck->lock;
	lv = (lv_t *)&dtlck->lv[0];

	/*
	 * Get head/only slot
	 */
	stbl = DT_GETSTBL(p);
	si = stbl[index];
	leaf_entry = (ldtentry_t *) &p->slot[si];

	/*
	 * open first line lock
	 */
	lv->offset = si;
	n = 1;

	klen = key->namlen;
	kname = key->name;

	assert (klen == leaf_entry->namlen);

	/*
	 * write first/only segment
	 */
	len = MIN(klen, DTLHDRDATALEN);
	UniStrncpy(leaf_entry->name, kname, len);

	klen -= len;

	/* Remember last slot index and get next one
	 */
	psi = si;
	si = leaf_entry->next;

	while (klen)
	{
		/*
		 * Get next slot
		 */
		slot = &p->slot[si];

		if (si != psi+1)
		{
			/* close current linelock
			 */
			lv->length = n;
			dtlck->index++;

			/* open new linelock
			 */
			if (dtlck->index < dtlck->maxcnt)
				lv++;
			else
			{
				dtlck = (dtlock_t *)txLinelock(dtlck);
				lv = (lv_t *)&dtlck->lv[0];
			}
			lv->offset = si;
			n = 0;
		}
		n++;
		kname += len;
		len = MIN(klen, DTSLOTDATALEN);
		UniStrncpy(slot->name, kname, len);

		psi = si;
		si = slot->next;
		klen -= len;
	}

	/*
	 * Close current linelock
	 */
	lv->length = n;
	dtlck->index++;

	/*
	 * unpin the leaf page
	 */
	DT_PUTPAGE(bp);

	return 0;
}

// Begin F226941
/*
 * NAME: dtFindInum
 *
 * FUNCTION: Finds specific inode number within directory
 *
 * PARAMETERS:
 *	ip	- Inode of directory
 *	inum	- Number of inode to find
 *	name	- Name of entry found
 *
 * SERIALIZATION:
 *	IREAD_LOCK must be held on ip
 *
 * RETURNS:
 *	ENOENT	- If no entry can be found to match inum
 *	0	- If successful
 */
int32
dtFindInum(
inode_t		*ip,
ino_t		inum,
component_t	*name)
{
	int32		rc = 0;
	int64		bn;
	jbuf_t		*bp;
	dtpage_t	*p;
	int32		index;
	int8		*stbl;
	btstack_t	btstack;
	int32		i, next;
	ldtentry_t	*d;
	dtslot_t	*t;
	int32		d_namlen, len;
	UniChar		*d_name;

	if (dtEmpty(ip) == 0)
		return ENOENT;

	rc = dtReadFirst(ip, &btstack);
	if (rc)
		return ENOENT;

	BT_GETSEARCH(ip, btstack.top, bn, bp, dtpage_t, p, index);

	while (1)
	{
		stbl = DT_GETSTBL(p);

		for (i = index; i < p->header.nextindex; i++)
		{
			d = (ldtentry_t *)&p->slot[stbl[i]];
			if (d->inumber == inum)
			{
				/*
				 * inode found!
				 */
				name->namlen = d_namlen = d->namlen;
				d_name = name->name;

				/*
				 * Copy name of head/only segment
				 */
				len = MIN(d_namlen, DTLHDRDATALEN);
				UniStrncpy(d_name, d->name, len);

				/*
				 * Copy name in additional segments
				 */
				next = d->next;
				while (next >= 0)
				{
					t = (dtslot_t *)&p->slot[next];
					d_name += len;
					d_namlen -= len;
					ASSERT(d_namlen > 0);
					len = MIN(d_namlen, DTSLOTDATALEN);
					UniStrncpy(d_name, t->name, len);
					next = t->next;
				}

				DT_PUTPAGE(bp);

				/*
				 * Terminate name
				 */
				name->name[name->namlen] = 0;

				return NO_ERROR;
			}
		}
		/*
		 * get next leaf page
		 */
		if (p->header.flag & BT_ROOT)
			break;

		bn = p->header.next;
		if (bn == 0)
			/* Last leaf, no more entries */
			break;

		DT_PUTPAGE(bp);

		DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;
	}

	/*
	 * If we get here, we didn't find the inode
	 */
	DT_PUTPAGE(bp);
	return ENOENT;
}
// End F226941

#endif	/* _JFS_OS2 */

#ifdef drova  // _JFS_STATISTICS
/*
 *	dtStatistics
 */
void
dtStatistics()
{
	/* report statistics */
}
#endif	/* _JFS_STATISTICS */


#ifdef _JFS_DEBUG_DTREE
/*
 *	dtDisplayTree()
 *
 * function: traverse forward
 */
int32
dtDisplayTree(
	inode_t		*ip)
{
	int32		rc;
	jbuf_t		*bp;
	dtpage_t	*p;
	int64		bn, pbn;
	int32		index, lastindex, v, h;
	pxd_t		*xd;
	btstack_t	btstack;
	btframe_t	*btsp;
	btframe_t	*parent;
	uint8		*stbl;
	int32		psize = 256;

	printf("display B+-tree.\n");

	/* clear stack */
	btsp = btstack.stack;

	/*
	 * start with root
	 *
 	 * root resides in the inode
	 */
	bn = 0;
	v = h = 0;

	/*
	 * first access of each page:
	 */
newPage:
	DT_GETPAGE(ip, bn, bp, dtpage_t, psize, p, rc);
	if (rc)
		return rc;

	/* process entries forward from first index */
	index = 0;
	lastindex = p->header.nextindex - 1;

	if (p->header.flag & BT_INTERNAL)
	{
		/*
		 * first access of each internal page
		 */
		printf("internal page ");
		dtDisplayPage(ip, bn, p);

		goto getChild;
	}
	else /* (p->header.flag & BT_LEAF) */
	{
		/*
	 	 * first access of each leaf page
	 	 */
		printf("leaf page ");
		dtDisplayPage(ip, bn, p);

		/*
		 * process leaf page entries
		 *
		for ( ; index <= lastindex; index++)
		{
		}
		 */
			
		/* unpin the leaf page */
		DT_PUTPAGE(bp);
	}

	/*
	 * go back up to the parent page
	 */
getParent:
	/* pop/restore parent entry for the current child page */
	if ((parent = (btsp == btstack.stack ? NULL : --btsp)) == NULL)
		/* current page must have been root */
		return;

	/*
	 * parent page scan completed
	 */
	if ((index = parent->index) == (lastindex = parent->lastindex))
	{
	 	/* go back up to the parent page */
		goto getParent;
	}

	/*
	 * parent page has entries remaining
	 */
	/* get back the parent page */
	bn = parent->bn;
	/* v = parent->level; */
	DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
	if (rc)
		return rc;

	/* get next parent entry */
	index++;
	
	/*
	 * internal page: go down to child page of current entry
	 */
getChild:
	/* push/save current parent entry for the child page */
	btsp->bn = pbn = bn;
	btsp->index = index;
	btsp->lastindex = lastindex;
	/* btsp->level = v; */
	/* btsp->node = h; */
	++btsp;

	/* get current entry for the child page */
	stbl = DT_GETSTBL(p);
	xd = (pxd_t *)&p->slot[stbl[index]];
	
	/*
	 * first access of each internal entry:
	 */

	/* get child page */
	bn = addressPXD(xd);
	psize = lengthPXD(xd) << ip->i_ipmnt->i_l2bsize;

	printf("traverse down 0x%llx[%d]->0x%llx\n", pbn, index, bn);
	v++;
	h = index;

	/* release parent page */
	DT_PUTPAGE(bp);

	/* process the child page */
	goto newPage;
}


/*
 *	dtDisplayPage()
 *
 * function: display page
 */
int32
dtDisplayPage(
	inode_t		*ip,
	int64		bn,
	dtpage_t	*p)
{
	int32		rc;
	jbuf_t		*bp;
	ldtentry_t	*lh;
	idtentry_t	*ih;
	pxd_t		*xd;
	int32		i, j;
	uint8		*stbl;
	UniChar		name[JFS_NAME_MAX+1];
	component_t	key = {0, name};
	int32		freepage = 0;

	if (p == NULL)
	{
		freepage = 1;
		DT_GETPAGE(ip, bn, bp, dtpage_t, PSIZE, p, rc);
		if (rc)
			return rc;
	}

	/* display page control */
	printf("bn:0x%llx flag:0x%08x nextindex:%d\n",
		bn, p->header.flag, p->header.nextindex);

	/* display entries */
	stbl = DT_GETSTBL(p);
	for (i = 0, j = 1; i < p->header.nextindex; i++, j++)
	{
		dtGetKey(p, i, &key);
		key.name[key.namlen] ='\0';
		if (p->header.flag & BT_LEAF)
		{
			lh = (ldtentry_t *)&p->slot[stbl[i]];
			printf("\t[%d] %s:%d", i, key.name, lh->inumber);
		}
		else
		{
			ih = (idtentry_t *)&p->slot[stbl[i]];
			xd = (pxd_t *)ih;
			bn = addressPXD(xd);
			printf("\t[%d] %s:0x%llx", i, key.name, bn);
		}
		
		if (j == 4)
		{
			printf("\n");
			j = 0;
		}
	}

	printf("\n");

	if (freepage)
		DT_PUTPAGE(bp);

	return 0;
}
#endif	/* _JFS_DEBUG_DTREE */
