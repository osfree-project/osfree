/* $Id: jfs_xtree.c,v 1.1 2000/04/21 10:58:18 ktk Exp $ */

static char *SCCSID = "@(#)1.28  8/9/99 04:06:41 src/jfs/ifs/jfs_xtree.c, sysjfs, w45.fs32, currbld";
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
 */
/*
 *      jfs_xtree.c: extent allocation descriptor B+-tree manager
 *
 * TBD: compression;
 */
 /* Change History :
 *
 *
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif /* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_bufmgr.h"
#include "jfs_xtree.h"
#include "jfs_txnmgr.h"
#include "jfs_dmap.h"           /* for function prototype */
#include "jfs_defragfs.h"
#include "jfs_debug.h"

/*
 * xtree local flag
 */
#define XT_INSERT       0x00000001

/*
 *       xtree key/entry comparison: extent offset
 *
 * return:
 *      -1: k < start of extent
 *       0: start_of_extent <= k <= end_of_extent
 *       1: k > end_of_extent
 */
#define XT_CMP(CMP, K, X, OFFSET64)\
{\
        OFFSET64 = offsetXAD(X);\
        (CMP) = ((K) >= OFFSET64 + lengthXAD(X)) ? 1 :\
              ((K) < OFFSET64) ? -1 : 0;\
}

/* write a xad entry */
#define XT_PUTENTRY(XAD, FLAG, OFF, LEN, ADDR)\
{\
        (XAD)->flag = (FLAG);\
        XADoffset((XAD), (OFF));\
        XADlength((XAD), (LEN));\
        XADaddress((XAD), (ADDR));\
}

/* get page buffer for specified block address */
#define XT_GETPAGE(IP, BN, BP, TYPE, SIZE, P, RC)\
{\
        BT_GETPAGE(IP, BN, BP, TYPE, SIZE, P, RC)\
        if (!(RC))\
        {\
                if (((P)->header.nextindex < XTENTRYSTART) ||\
                    ((P)->header.nextindex > (P)->header.maxentry) ||\
                    ((P)->header.maxentry > (((BN)==0)?XTROOTMAXSLOT:PSIZE>>L2XTSLOTSIZE)))\
                {\
                        jEVENT(1,("XT_GETPAGE: xtree page corrupt\n"));\
                        txFileSystemDirty((IP)->i_ipmnt);\
                        RC = EIO;\
                }\
        }\
}

/* for consistency */
#define XT_PUTPAGE(BP) BT_PUTPAGE(BP)

/* xtree entry parameter descriptor */
typedef struct {
        jbuf_t          *bp;
        int16           index;
        uint8           flag;
        int64           off;
        int64           addr;
        int32           len;
        pxdlist_t       *pxdlist;
} xtsplit_t;


#ifdef _JFS_STATISTICS
/*
 *      statistics
 */
static struct {
        uint32  search;
        uint32  fastSearch;
        uint32  split;
} xtStat = {0};
#endif  /* _JFS_STATISTICS */


/*
 * forward references
 */
static int32 xtSearch(
        inode_t         *ip,
        int64           xoff,
        int32           *cmpp,
        btstack_t       *btstack,
        uint32          flag);

static int32 xtSplitUp(
        int32           tid,
        inode_t         *ip,
        xtsplit_t       *split,
        btstack_t       *btstack);

static int32 xtSplitPage(
        int32           tid,
        inode_t         *ip,
        xtsplit_t       *split,
        jbuf_t          **rbpp,
        int64           *rbnp);

static int32 xtSplitRoot(
        int32           tid,
        inode_t         *ip,
        xtsplit_t       *split,
        jbuf_t          **rbpp);

static int32 xtDeleteUp(
        int32           tid,
        inode_t         *ip,
        jbuf_t          *fbp,
        xtpage_t        *fp,
        btstack_t       *btstack);

static int32 xtSearchNode(
        inode_t         *ip,
        xad_t           *xad,
        int32           *cmpp,
        btstack_t       *btstack,
        uint32          flag);

static int32 xtRelink(
        int32           tid,
        inode_t         *ip,
        xtpage_t        *fp);

/* External references */

int32   txFileSystemDirty(inode_t *);

/*
 *      debug control
 */
/*      #define _JFS_DEBUG_XTREE        1 */


/*
 *      xtLookup()
 *
 * function: map a single page into a physical extent;
 */
xtLookup(
        inode_t         *ip,
        int64           lstart,
        int32           llen,
        uint8           *pflag,
        int64           *paddr,
        int32           *plen,
        uint32          flag)
{
        int32           rc = 0;
        btstack_t       btstack;
        int32           cmp;
        int64           bn;
        jbuf_t          *bp;
        xtpage_t        *p;
        int32           index;
        xad_t           *xad;
        int64           size, xoff, xend, lend;
        int32           xlen;
        int64           xaddr;

        *plen = 0;

        /* is lookup offset beyond eof ? */
        size = (ip->i_size + (ip->i_ipmnt->i_bsize - 1)) >> ip->i_ipmnt->i_l2bsize;
        if (lstart >= size)
                return 0;

        /*
         * search for the xad entry covering the logical extent
         */
search:
        if (rc = xtSearch(ip, lstart, &cmp, &btstack, 0))
                return rc;

        /*
         *      compute the physical extent covering logical extent
         *
         * N.B. search may have failed (e.g., hole in sparse file),
         * and returned the index of the next entry.
         */
        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /* is xad found covering start of logical extent ?
         * lstart is a page start address,
         * i.e., lstart cannot start in a hole;
         */
        if (cmp)
                goto out;

        /*
         * lxd covered by xad
         */
        xad = &p->xad[index];
        xoff = offsetXAD(xad);
        xlen = lengthXAD(xad);
        xend = xoff + xlen;
        xaddr = addressXAD(xad);

        /* initialize new pxd */
        *pflag = xad->flag;
        *paddr = xaddr + (lstart - xoff);
        /* a page must be fully covered by an xad */
        *plen = MIN(xend - lstart, llen);

out:
        XT_PUTPAGE(bp);

        return rc;
}


/*
 *      xtLookupPageList()
 *
 * function: map a single logical extent into a list of physical page extent;
 *
 * note: a page being written (even a single byte) is backed fully,
 *      except the last page which is only backed with blocks
 *      required to cover the last byte;
 *      the extent backing a page is fully contained within an xad;
 */
xtLookupPageList(
        inode_t         *ip,
        int64           lstart,
        int32           llen,
        pxdlist_t       *pxdlist,       /* pxd list (in/out) */
        uint32          flag)
{
        int32           rc = 0;
        btstack_t       btstack;
        int32           cmp;
        int64           bn;
        jbuf_t          *bp;
        xtpage_t        *p;
        int32           index;
        xad_t           *xad;
        pxd_t           *pxd;
        int64           size, xoff, xaddr;
        int32           xlen, plen;
        int32           npxd, maxnpxd;

        npxd = pxdlist->npxd = 0;
        maxnpxd = pxdlist->maxnpxd;
        pxd = pxdlist->pxd;

        /* is lookup offset beyond eof ? */
        size = (ip->i_size + (ip->i_ipmnt->i_bsize - 1)) >> ip->i_ipmnt->i_l2bsize;
        if (lstart >= size)
                return 0;

        /*
         * search for the xad entry covering the logical extent
         */
search:
        if (rc = xtSearch(ip, lstart, &cmp, &btstack, 0))
                return rc;

        /*
         *      compute the physical extent covering logical extent
         *
         * N.B. search may have failed (e.g., hole in sparse file),
         * and returned the index of the next entry.
         */
        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /* is xad found covering start of logical extent ?
         * lstart is a page start address,
         * i.e., lstart cannot start in a hole;
         */
        if (cmp)
                goto out;

        xad = &p->xad[index];
        if (xad->flag & XAD_NOTRECORDED)
                goto out;

        xoff = offsetXAD(xad);
        xlen = lengthXAD(xad);
        xaddr = addressXAD(xad);

        /* xoff <= lstart < xend */

        plen = lstart - xoff;
        xoff = lstart;
        xaddr += plen;
        xlen -= plen;

        /*
         * construct an xad for a page
         */
pagexad:
        plen = MIN(xlen, ip->i_ipmnt->i_nbperpage);
        PXDaddress(pxd, xaddr);
        PXDlength(pxd, plen);

        if (++npxd >= maxnpxd)
                goto out;
        pxd++;

        /* any more logical extent to map ? */
        llen -= plen;
        if (llen == 0)
                goto out;

        xoff += plen;
        xlen -= plen;

        /* any more physical extent backed by current xad ? */
        if (xlen)
        {
                xaddr += plen;
        }
        else
        {
                /* get next xad */
                if (index == p->header.nextindex - 1)
                {
                        if (p->header.flag & BT_ROOT)
                                goto out;

                        if ((bn = p->header.next) == 0)
                                goto out;

                        XT_PUTPAGE(bp);

                        /* get next sibling page */
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                        if (rc)
                                return rc;

                        index = XTENTRYSTART;
                        xad = &p->xad[index];
                }
                else
                {
                        index++;
                        xad++;
                }

                if (xad->flag & XAD_NOTRECORDED)
                        goto out;

                /* is next xad logically contiguous ? */
                if (xoff != offsetXAD(xad))
                        goto out;

                xaddr = addressXAD(xad);
                xlen = lengthXAD(xad);
        }

        goto pagexad;

out:
        XT_PUTPAGE(bp);

        pxdlist->npxd = npxd;

        return rc;
}


/*
 *      xtLookupList()
 *
 * function: map a single logical extent into a list of physical extent;
 *
 * parameter:
 *      inode_t         *ip,
 *      lxdlist_t       *lxdlist,       lxd list (in)
 *      xadlist_t       *xadlist,       xad list (in/out)
 *      uint32          flag)
 *
 * coverage of lxd by xad under assumption of
 * . lxd's are ordered and disjoint.
 * . xad's are ordered and disjoint.
 *
 * return:
 *      0:      success
 *
 * note: a page being written (even a single byte) is backed fully,
 *      except the last page which is only backed with blocks
 *      required to cover the last byte;
 *      the extent backing a page is fully contained within an xad;
 */
xtLookupList(
        inode_t         *ip,
        lxdlist_t       *lxdlist,       /* lxd list (in) */
        xadlist_t       *xadlist,       /* xad list (in/out) */
        uint32          flag)
{
        int32           rc = 0;
        btstack_t       btstack;
        int32           cmp;
        int64           bn;
        jbuf_t          *bp;
        xtpage_t        *p;
        int32           index;
        lxd_t           *lxd;
        xad_t           *xad, *pxd;
        int64           size, lstart, lend, xstart, xend, lastxend, pstart;
        int32           llen, xlen, lastxlen, plen;
        int64           xaddr, lastxaddr, paddr;
        int32           nlxd, npxd, maxnpxd;
        int64           lastoff;
        int32           lastlen;

        npxd = xadlist->nxad = 0;
        maxnpxd = xadlist->maxnxad;
        pxd = xadlist->xad;

        nlxd = lxdlist->nlxd;
        lxd = lxdlist->lxd;

        lstart = offsetLXD(lxd);
        llen = lengthLXD(lxd);
        lend = lstart + llen;

        size = (ip->i_size + (ip->i_ipmnt->i_bsize - 1)) >> ip->i_ipmnt->i_l2bsize;

        /*
         * search for the xad entry covering the logical extent
         */
search:
        if (lstart >= size)
                return 0;

        if (rc = xtSearch(ip, lstart, &cmp, &btstack, 0))
                return rc;

        /*
         *      compute the physical extent covering logical extent
         *
         * N.B. search may have failed (e.g., hole in sparse file),
         * and returned the index of the next entry.
         */
map:
        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /* is xad on the next sibling page ? */
        if (index == p->header.nextindex)
        {
                if (p->header.flag & BT_ROOT)
                        goto mapend;

                if ((bn = p->header.next) == 0)
                        goto mapend;

                XT_PUTPAGE(bp);

                /* get next sibling page */
                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;

                index = XTENTRYSTART;
        }

        xad = &p->xad[index];

        /*
         * is lxd covered by xad ?
         */
compare:
        xstart = offsetXAD(xad);
        xlen = lengthXAD(xad);
        xend = xstart + xlen;
        xaddr = addressXAD(xad);

compare1:
        if (xstart < lstart)
                goto compare2;

        /* (lstart <= xstart) */

        /* lxd is NOT covered by xad */
        if (lend <= xstart)
        {
                /*
                 * get next lxd
                 */
                if (--nlxd == 0)
                        goto mapend;
                lxd++;

                lstart = offsetLXD(lxd);
                llen = lengthLXD(lxd);
                lend = lstart + llen;
                if (lstart >= size)
                        goto mapend;

                /* compare with the current xad  */
                goto compare1;
        }
        /* lxd is covered by xad */
        else /* (xstart < lend) */
        {
                /* initialize new pxd */
                pstart = xstart;
                plen = MIN(lend - xstart, xlen);
                paddr = xaddr ;

                goto cover;
        }

        /* (xstart < lstart) */
compare2:
        /* lxd is covered by xad */
        if (lstart < xend)
        {
                /* initialize new pxd */
                pstart = lstart;
                plen = MIN(xend - lstart, llen);
                paddr = xaddr + (lstart - xstart);

                goto cover;
        }
        /* lxd is NOT covered by xad */
        else /* (xend <= lstart) */
        {
                /*
                 * get next xad
                 *
                 * linear search next xad covering lxd on
                 * the current xad page, and then tree search
                 */
                if (index == p->header.nextindex - 1)
                {
                        if (p->header.flag & BT_ROOT)
                                goto mapend;

                        XT_PUTPAGE(bp);
                        goto search;
                }
                else
                {
                        index++;
                        xad++;

                        /* compare with new xad */
                        goto compare;
                }
        }

        /*
         * lxd is covered by xad and a new pxd has been initialized
         * (lstart <= xstart < lend) or (xstart < lstart < xend)
         */
cover:
        /* finalize pxd corresponding to current xad */
        XT_PUTENTRY(pxd, xad->flag, pstart, plen, paddr);

        if (++npxd >= maxnpxd)
                goto mapend;
        pxd++;

        /*
         * lxd is fully covered by xad
         */
        if (lend <= xend)
        {
                /*
                 * get next lxd
                 */
                if (--nlxd == 0)
                        goto mapend;
                lxd++;

                lstart = offsetLXD(lxd);
                llen = lengthLXD(lxd);
                lend = lstart + llen;
                if (lstart >= size)
                        goto mapend;

                /*
                 * test for old xad covering new lxd
                 * (old xstart < new lstart)
                 */
                goto compare2;
        }
        /*
         * lxd is partially covered by xad
         */
        else /* (xend < lend)  */
        {
                /*
                 * get next xad
                 *
                 * linear search next xad covering lxd on
                 * the current xad page, and then next xad page search
                 */
                if (index == p->header.nextindex - 1)
                {
                        if (p->header.flag & BT_ROOT)
                                goto mapend;

                        if ((bn = p->header.next) == 0)
                                goto mapend;

                        XT_PUTPAGE(bp);

                        /* get next sibling page */
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                        if (rc)
                                return rc;

                        index = XTENTRYSTART;
                        xad = &p->xad[index];
                }
                else
                {
                        index++;
                        xad++;
                }

                /*
                 * test for new xad covering old lxd
                 * (old lstart < new xstart)
                 */
                goto compare;
        }

mapend:
        xadlist->nxad = npxd;

out:
        XT_PUTPAGE(bp);

        return rc;
}


/*
 *      xtSearch()
 *
 * function:    search for the xad entry covering specified offset.
 *
 * parameters:
 *      ip      - file object;
 *      xoff    - extent offset;
 *      cmpp    - comparison result:
 *      btstack - traverse stack;
 *      flag    - search process flag (XT_INSERT);
 *
 * returns:
 *      btstack contains (bn, index) of search path traversed to the entry.
 *      *cmpp is set to result of comparison with the entry returned.
 *      the page containing the entry is pinned at exit.
 */
static int32
xtSearch(
        inode_t         *ip,
        int64           xoff,   /* offset of extent */
        int32           *cmpp,
        btstack_t       *btstack,
        uint32          flag)
{
        int32           rc = 0;
        int32           cmp = 1;        /* init for empty page */
        int64           bn;     /* block number */
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* page */
        xad_t           *xad;
        int32           base, index, lim, btindex;
        btframe_t       *btsp;
        int32           nsplit = 0;     /* number of pages to split */
        int64           t64;

        INCREMENT(xtStat.search);

        BT_CLR(btstack);

        btstack->nsplit = 0;

        /*
         *      search down tree from root:
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
                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;

                /* try sequential access heuristics with the previous
                 * access entry in target leaf page:
                 * once search narrowed down into the target leaf,
                 * key must either match an entry in the leaf or
                 * key entry does not exist in the tree;
                 */
fastSearch:
                if ((ip->i_btorder & BT_SEQUENTIAL) &&
                    (p->header.flag & BT_LEAF) &&
                    (index = ip->i_btindex) < p->header.nextindex)
                {
                        xad = &p->xad[index];
                        t64 = offsetXAD(xad);
                        if (xoff < t64 + lengthXAD(xad))
                        {
                                if (xoff >= t64)
                                {
                                        *cmpp = 0;
                                        goto out;
                                }

                                /* stop sequential access heuristics */
                                goto binarySearch;
                        }
                        else /* (t64 + lengthXAD(xad)) <= xoff */
                        {
                                /* try next sequential entry */
                                index++;
                                if (index < p->header.nextindex)
                                {
                                        xad++;
                                        t64 = offsetXAD(xad);
                                        if (xoff < t64 + lengthXAD(xad))
                                        {
                                                if (xoff >= t64)
                                                {
                                                        *cmpp = 0;
                                                        goto out;
                                                }

                                                /* miss: key falls between
                                                 * previous and this entry
                                                 */
                                                *cmpp = 1;
                                                goto out;
                                        }

                                        /* (xoff >= t64 + lengthXAD(xad));
                                         * matching entry may be further out:
                                         * stop heuristic search
                                         */
                                        /* stop sequential access heuristics */
                                        goto binarySearch;
                                }

                                /* (index == p->header.nextindex);
                                 * miss: key entry does not exist in
                                 * the target leaf/tree
                                 */
                                *cmpp = 1;
                                goto out;
                        }

                        /*
                         * if hit, return index of the entry found, and
                         * if miss, where new entry with search key is
                         * to be inserted;
                         */
out:
                        /* compute number of pages to split */
                        if (flag & XT_INSERT)
                        {
                                if (p->header.nextindex == p->header.maxentry)
                                        nsplit++;
                                else
                                        nsplit = 0;
                                btstack->nsplit = nsplit;
                        }

                        /* save search result */
                        btsp = btstack->top;
                        btsp->bn = bn;
                        btsp->index = index;
                        btsp->bp = bp;

                        /* update sequential access heuristics */
                        ip->i_btindex = index;

                        INCREMENT(xtStat.fastSearch);
                        return 0;
                }

                /* well, ... full search now */
binarySearch:
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
                                 *      search hit
                                 */
                                /* search hit - leaf page:
                                 * return the entry found
                                 */
                                if (p->header.flag & BT_LEAF)
                                {
                                        *cmpp = cmp;

                                        /* compute number of pages to split */
                                        if (flag & XT_INSERT)
                                        {
                                                if (p->header.nextindex == p->header.maxentry)
                                                        nsplit++;
                                                else
                                                        nsplit = 0;
                                                btstack->nsplit = nsplit;
                                        }

                                        /* save search result */
                                        btsp = btstack->top;
                                        btsp->bn = bn;
                                        btsp->index = index;
                                        btsp->bp = bp;

                                        /* init sequential access heuristics */
                                        btindex = ip->i_btindex;
                                        if (index == btindex ||
                                            index == btindex + 1)
                                                ip->i_btorder = BT_SEQUENTIAL;
                                        else
                                                ip->i_btorder = BT_RANDOM;
                                        ip->i_btindex = index;

                                        return 0;
                                }

                                /* search hit - internal page:
                                 * descend/search its child page
                                 */
                                goto next;
                        }

                        if (cmp > 0) {
                                base = index + 1;
                                --lim;
                        }
                }

                /*
                 *      search miss
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
                        *cmpp = cmp;

                        /* compute number of pages to split */
                        if (flag & XT_INSERT)
                        {
                                if (p->header.nextindex == p->header.maxentry)
                                        nsplit++;
                                else
                                        nsplit = 0;
                                btstack->nsplit = nsplit;
                        }

                        /* save search result */
                        btsp = btstack->top;
                        btsp->bn = bn;
                        btsp->index = base;
                        btsp->bp = bp;

                        /* init sequential access heuristics */
                        btindex = ip->i_btindex;
                        if (base == btindex ||
                            base == btindex + 1)
                                ip->i_btorder = BT_SEQUENTIAL;
                        else
                                ip->i_btorder = BT_RANDOM;
                        ip->i_btindex = base;

                        return 0;
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
next:
                /* update number of pages to split */
                if (p->header.nextindex == p->header.maxentry)
                        nsplit++;
                else
                        nsplit = 0;

                /* push (bn, index) of the parent page/entry */
                BT_PUSH(btstack, bn, index);

                /* get the child page block number */
                bn = addressXAD(&p->xad[index]);

                /* unpin the parent page */
                XT_PUTPAGE(bp);
        }
}


/*
 *      xtInsert()
 *
 * function:
 *
 * parameter:
 *      tid     - transaction id;
 *      ip      - file object;
 *      xflag   - extent flag (XAD_NOTRECORDED):
 *      xoff    - extent offset;
 *      xlen    - extent length;
 *      xaddrp  - extent address pointer (in/out):
 *              if (*xaddrp)
 *                      caller allocated data extent at *xaddrp;
 *              else
 *                      allocate data extent and return its xaddr;
 *      flag    -
 *
 * return:
 */
int32
xtInsert(
        int32           tid,    /* transaction id */
        inode_t         *ip,
        uint8           xflag,
        int64           xoff,
        int32           xlen,
        int64           *xaddrp,/* extent address (in/out) */
        uint32          flag)
{
        int32           rc = 0;
        int64           xaddr, hint;
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* base B+-tree index page */
        int64           bn;
        int32           index, nextindex;
        btstack_t       btstack;/* traverse stack */
        xtsplit_t       split;  /* split information */
        xad_t           *xad;
        int32           cmp, hit;
        tlock_t         *tlck;
        xtlock_t        *xtlck;

jEVENT(0,("xtInsert: nxoff:0x%x%08x nxlen:0x%08x\n", xoff, xlen));

        /*
         *      search for the entry location at which to insert:
         *
         * xtFastSearch() and xtSearch() both returns (leaf page
         * pinned, index at which to insert).
         * n.b. xtSearch() may return index of maxentry of
         * the full page.
         */
        if (rc = xtSearch(ip, xoff, &cmp, &btstack, XT_INSERT))
                return rc;

        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /* This test must follow BT_GETSEARCH since bp must be valid if
         * be branch to out: */
        if (cmp == 0)
        {
                rc = EEXIST;
                goto out;
        }

        /*
         * allocate data extent requested
         *
         * allocation hint: last xad
         */
        if ((xaddr = *xaddrp) == 0)
        {
                if (index > XTENTRYSTART)
                {
                        xad = &p->xad[index - 1];
                        hint = addressXAD(xad) + lengthXAD(xad) - 1;
                }
                else
                        hint = 0;
                if (rc = dbAlloc(ip, hint, (int64)xlen, &xaddr))
                        goto out;
        }

        /*
         *      insert entry for new extent
         */
        xflag |= XAD_NEW;

        /*
         *      if the leaf page is full, split the page and
         *      propagate up the router entry for the new page from split
         *
         * The xtSplitUp() will insert the entry and unpin the leaf page.
         */
        nextindex = p->header.nextindex;
        if (nextindex == p->header.maxentry)
        {
                split.bp = bp;
                split.index = index;
                split.flag = xflag;
                split.off = xoff;
                split.len = xlen;
                split.addr = xaddr;
                split.pxdlist = NULL;
                if (rc = xtSplitUp(tid, ip, &split, &btstack))
                {
                        /* undo data extent allocation */
                        if (*xaddrp == 0)
                                dbFree(ip, xaddr, (int64)xlen);
                        return rc;
                }

                *xaddrp = xaddr;
                ip->i_flag |= IUPDNEW;
                return 0;
        }

        /*
         *      insert the new entry into the leaf page
         */
        /*
         * acquire a transaction lock on the leaf page;
         *
         * action: xad insertion/extension;
         */
        tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        /* if insert into middle, shift right remaining entries. */
        if (index < nextindex)
                memmove(&p->xad[index + 1], &p->xad[index],
                        (nextindex - index) * sizeof(xad_t));

        /* insert the new entry: mark the entry NEW */
        xad = &p->xad[index];
        XT_PUTENTRY(xad, xflag, xoff, xlen, xaddr);

        /* advance next available entry index */
        ++p->header.nextindex;

        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index, xtlck->lwm.offset) : index;
        xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

        *xaddrp = xaddr;
        ip->i_flag |= IUPDNEW;

out:
        /* unpin the leaf page */
        XT_PUTPAGE(bp);

        return rc;
}


/*
 *      xtSplitUp()
 *
 * function:
 *      split full pages as propagating insertion up the tree
 *
 * parameter:
 *      tid     - transaction id;
 *      ip      - file object;
 *      split   - entry parameter descriptor;
 *      btstack - traverse stack from xtSearch()
 *
 * return:
 */
static int32
xtSplitUp(
        int32           tid,
        inode_t         *ip,
        xtsplit_t       *split,
        btstack_t       *btstack)
{
        int32           rc = 0;
        jbuf_t          *sbp;
        xtpage_t        *sp;    /* split page */
        jbuf_t          *rbp;
        xtpage_t        *rp;    /* new right page split from sp */
        int64           rbn;    /* new right page block number */
        jbuf_t          *rcbp;
        xtpage_t        *rcp;   /* right child page */
        int64           rcbn;   /* right child page block number */
        int32           skip;   /* index of entry of insertion */
        int32           nextindex;/* next available entry index of p */
        btframe_t       *parent;/* parent page entry on traverse stack */
        xad_t           *xad;
        int64           xaddr;
        int32           xlen;
        int32           nsplit; /* number of pages split */
        pxdlist_t       pxdlist;
        pxd_t           *pxd;
        tlock_t         *tlck;
        xtlock_t        *xtlck;

        sbp = split->bp;
        sp = BT_PAGE(ip, sbp, xtpage_t);

        /* is inode xtree root extension/inline EA area free ? */
        if ((sp->header.flag & BT_ROOT) &&
            (sp->header.maxentry < XTROOTMAXSLOT) &&
            (ip->i_mode & INLINEEA))
        {
                sp->header.maxentry = XTROOTMAXSLOT;
                ip->i_mode &= ~INLINEEA;

                /*
                 * acquire a transaction lock on the leaf page;
                 *
                 * action: xad insertion/extension;
                 */
                tlck = txLock(tid, ip, sbp, tlckXTREE|tlckGROW);
                xtlck = (xtlock_t *)&tlck->lock;

                /* if insert into middle, shift right remaining entries. */
                skip = split->index;
                nextindex = sp->header.nextindex;
                if (skip < nextindex)
                        memmove(&sp->xad[skip + 1], &sp->xad[skip],
                                (nextindex - skip) * sizeof(xad_t));

                /* insert the new entry: mark the entry NEW */
                xad = &sp->xad[skip];
                XT_PUTENTRY(xad, split->flag, split->off, split->len, split->addr);

                /* advance next available entry index */
                ++sp->header.nextindex;

                xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(skip, xtlck->lwm.offset) : skip;
                xtlck->lwm.length = sp->header.nextindex - xtlck->lwm.offset;

                return 0;
        }

        /*
         * allocate new index blocks to cover index page split(s)
         *
         * allocation hint: ?
         */
        if (split->pxdlist == NULL)
        {
                nsplit = btstack->nsplit;
                split->pxdlist = &pxdlist;
                pxdlist.maxnpxd = pxdlist.npxd = 0;
                pxd = &pxdlist.pxd[0];
                xlen = ip->i_ipmnt->i_nbperpage;
                for (; nsplit > 0; nsplit--, pxd++)
                {
                        if ((rc = dbAlloc(ip, (int64)0, (int64)xlen, &xaddr)) == 0)
                        {
                                PXDaddress(pxd, xaddr);
                                PXDlength(pxd, xlen);

                                pxdlist.maxnpxd++;

                                continue;
                        }

                        /* undo allocation */

                        XT_PUTPAGE(sbp);
                        return rc;
                }
        }

        /*
         * Split leaf page <sp> into <sp> and a new right page <rp>.
         *
         * The split routines insert the new entry into the leaf page,
         * and acquire txLock as appropriate.
         * return <rp> pinned and its block number <rpbn>.
         */
        rc = (sp->header.flag & BT_ROOT) ?
            xtSplitRoot(tid, ip, split, &rbp) :
            xtSplitPage(tid, ip, split, &rbp, &rbn);
        if (rc)
                return EIO;

        XT_PUTPAGE(sbp);

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
         * There are a maximum of 3 pages pinned at any time:
         * right child, left parent and right parent (when the parent splits)
         * to keep the child page pinned while working on the parent.
         * make sure that all pins are released at exit.
         */
        while ((parent = BT_POP(btstack)) != NULL)
        {
                /* parent page specified by stack frame <parent> */

                /* keep current child pages <rcp> pinned */
                rcbp = rbp;
                rcbn = rbn;
                rcp = BT_PAGE(ip, rcbp, xtpage_t);

                /*
                 * insert router entry in parent for new right child page <rp>
                 */
                /* get/pin the parent page <sp> */
                XT_GETPAGE(ip, parent->bn, sbp, xtpage_t, PSIZE, sp, rc);
                if (rc)
                        goto errout2;

                /*
                 * The new key entry goes ONE AFTER the index of parent entry,
                 * because the split was to the right.
                 */
                skip = parent->index + 1;

                /*
                 * split or shift right remaining entries of the parent page
                 */
                nextindex = sp->header.nextindex;
                /*
                 * parent page is full - split the parent page
                 */
                if (nextindex == sp->header.maxentry)
                {
                        /* init for parent page split */
                        split->bp = sbp;
                        split->index = skip; /* index at insert */
                        split->flag = XAD_NEW;
                        split->off = offsetXAD(&rcp->xad[XTENTRYSTART]);
                        split->len = ip->i_ipmnt->i_nbperpage;
                        split->addr = rcbn;

                        /* unpin previous right child page */
                        XT_PUTPAGE(rcbp);

                        /* The split routines insert the new entry,
                         * and acquire txLock as appropriate.
                         * return <rp> pinned and its block number <rpbn>.
                         */
                        rc = (sp->header.flag & BT_ROOT) ?
                             xtSplitRoot(tid, ip, split, &rbp) :
                             xtSplitPage(tid, ip, split, &rbp, &rbn);
                        if (rc)
                                goto errout1;

                        XT_PUTPAGE(sbp);
                        /* keep new child page <rp> pinned */
                }
                /*
                 * parent page is not full - insert in parent page
                 */
                else
                {
                        /*
                         * insert router entry in parent for the right child page
                         * from the first entry of the right child page:
                         */
                        /*
                         * acquire a transaction lock on the parent page;
                         *
                         * action: router xad insertion;
                         */
                        tlck = txLock(tid, ip, sbp, tlckXTREE|tlckGROW);
                        xtlck = (xtlock_t *)&tlck->lock;

                        /* if insert into middle, shift right remaining entries */
                        if (skip < nextindex)
                                memmove(&sp->xad[skip + 1], &sp->xad[skip],
                                        (nextindex - skip) << L2XTSLOTSIZE);

                        /* insert the router entry */
                        xad = &sp->xad[skip];
                        XT_PUTENTRY(xad, XAD_NEW,
                            offsetXAD(&rcp->xad[XTENTRYSTART]), ip->i_ipmnt->i_nbperpage, rcbn);

                        /* advance next available entry index. */
                        ++sp->header.nextindex;

                        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(skip, xtlck->lwm.offset) : skip;
                        xtlck->lwm.length = sp->header.nextindex - xtlck->lwm.offset;

                        /* unpin parent page */
                        XT_PUTPAGE(sbp);

                        /* exit propagate up */
                        break;
                }
        }

        /* unpin current right page */
        XT_PUTPAGE(rbp);

        return 0;

        /*
         * If something fails in the above loop we were already walking back
         * up the tree and the tree is now inconsistent.
         * release all pages we're holding.
         */
errout1:
        XT_PUTPAGE(sbp);

errout2:
        XT_PUTPAGE(rcbp);

        return rc;
}


/*
 *      xtSplitPage()
 *
 * function:
 *      split a full non-root page into
 *      original/split/left page and new right page
 *      i.e., the original/split page remains as left page.
 *
 * parameter:
 *      int32           tid,
 *      inode_t         *ip,
 *      xtsplit_t       *split,
 *      jbuf_t          **rbpp,
 *      uint64          *rbnp,
 *
 * return:
 *      Pointer to page in which to insert or NULL on error.
 */
static int32
xtSplitPage(
        int32           tid,
        inode_t         *ip,
        xtsplit_t       *split,
        jbuf_t          **rbpp,
        int64           *rbnp)
{
        int32           rc = 0;
        jbuf_t          *sbp;
        xtpage_t        *sp;
        jbuf_t          *rbp;
        xtpage_t        *rp;    /* new right page allocated */
        int64           rbn;    /* new right page block number */
        jbuf_t          *bp;
        xtpage_t        *p;
        int64           bn, nextbn;
        int32           skip, maxentry, middle, righthalf, n;
        xad_t           *xad;
        pxdlist_t       *pxdlist;
        pxd_t           *pxd;
        tlock_t         *tlck;
        xtlock_t        *sxtlck, *rxtlck, *xtlck;

        sbp = split->bp;
        sp = BT_PAGE(ip, sbp, xtpage_t);

        INCREMENT(xtStat.split);

        /*
         * allocate the new right page for the split
         */
        pxdlist = split->pxdlist;
        pxd = &pxdlist->pxd[pxdlist->npxd];
        pxdlist->npxd++;
        rbn = addressPXD(pxd);
        rbp = bmAssign(ip, rbn, rbn, PSIZE, bmREAD_BLOCK);
        /* rbp->j_lblkno = rbn; */

jEVENT(0,("xtSplitPage: ip:0x%08x sbp:0x%08x rbp:0x%08x\n", ip, sbp, rbp));

        /*
         * acquire a transaction lock on the new right page;
         *
         * action: new page;
         */
        tlck = txLock(tid, ip, rbp, tlckXTREE|tlckNEW);
        rxtlck = (xtlock_t *)&tlck->lock;
        rxtlck->lwm.offset = XTENTRYSTART;

        rp = (xtpage_t *)rbp->b_bdata;
        rp->header.self = *pxd;
        rp->header.flag = sp->header.flag & BT_TYPE;
        rp->header.maxentry = sp->header.maxentry;
        rp->header.nextindex = XTENTRYSTART;

        /*
         * acquire a transaction lock on the split page
         */
        tlck = txLock(tid, ip, sbp, tlckXTREE|tlckGROW);
        sxtlck = (xtlock_t *)&tlck->lock;

        /*
         * initialize/update sibling pointers of <sp> and <rp>
         */
        nextbn = sp->header.next;
        rp->header.next = nextbn;
        rp->header.prev = addressPXD(&sp->header.self);
        sp->header.next = rbn;

        skip = split->index;

        /*
         *      sequential append at tail (after last entry of last page)
         *
         * if splitting the last page on a level because of appending
         * a entry to it (skip is maxentry), it's likely that the access is
         * sequential. adding an empty page on the side of the level is less
         * work and can push the fill factor much higher than normal.
         * if we're wrong it's no big deal -  we will do the split the right
         * way next time.
         * (it may look like it's equally easy to do a similar hack for
         * reverse sorted data, that is, split the tree left, but it's not.
         * Be my guest.)
         */
        if (nextbn == 0 && skip == sp->header.maxentry)
        {
                /*
                 * acquire a transaction lock on the new/right page;
                 *
                 * action: xad insertion;
                 */
                /* insert entry at the first entry of the new right page */
                xad = &rp->xad[XTENTRYSTART];
                XT_PUTENTRY(xad, split->flag, split->off, split->len, split->addr);

                rp->header.nextindex = XTENTRYSTART + 1;

                /* rxtlck->lwm.offset = XTENTRYSTART; */
                rxtlck->lwm.length = 1;

                *rbpp = rbp;
                *rbnp = rbn;

                ip->i_nblocks += lengthPXD(pxd);

jEVENT(0,("xtSplitPage: lp:0x%08x rp:0x%08x\n", sp, rp));
                return 0;
        }

        /*
         *      non-sequential insert (at possibly middle page)
         */

        /*
         * update previous pointer of old next/right page of <sp>
         */
        if (nextbn != 0)
        {
                XT_GETPAGE(ip, nextbn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                {
                        bmInvalidate(rbp);
                        return rc;
                }

                /*
                 * acquire a transaction lock on the next page;
                 *
                 * action:sibling pointer update;
                 */
                tlck = txLock(tid, ip, bp, tlckXTREE|tlckRELINK);

                p->header.prev = rbn;

                /* sibling page may have been updated previously, or
                 * it may be updated later;
                 */

                XT_PUTPAGE(bp);
        }

        /*
         * split the data between the split and new/right pages
         */
        maxentry = sp->header.maxentry;
        middle = maxentry >> 1;
        righthalf = maxentry - middle;

        /*
         * skip index in old split/left page - insert into left page:
         */
        if (skip <= middle)
        {
                /* move right half of split page to the new right page */
                memmove(&rp->xad[XTENTRYSTART], &sp->xad[middle],
                        righthalf << L2XTSLOTSIZE);

                /* shift right tail of left half to make room for new entry */
                if (skip < middle)
                        memmove(&sp->xad[skip + 1], &sp->xad[skip],
                                (middle - skip) << L2XTSLOTSIZE);

                /* insert new entry */
                xad = &sp->xad[skip];
                XT_PUTENTRY(xad, split->flag, split->off, split->len, split->addr);

                /* update page header */
                sp->header.nextindex = middle + 1;
                sxtlck->lwm.offset = (sxtlck->lwm.offset) ? MIN(skip, sxtlck->lwm.offset) : skip;

                rp->header.nextindex = XTENTRYSTART + righthalf;
        }
        /*
         * skip index in new right page - insert into right page:
         */
        else
        {
                /* move left head of right half to right page */
                n = skip - middle;
                memmove(&rp->xad[XTENTRYSTART], &sp->xad[middle], n << L2XTSLOTSIZE);

                /* insert new entry */
                n += XTENTRYSTART;
                xad = &rp->xad[n];
                XT_PUTENTRY(xad, split->flag, split->off, split->len, split->addr);

                /* move right tail of right half to right page */
                if (skip < maxentry)
                        memmove(&rp->xad[n + 1], &sp->xad[skip],
                                (maxentry - skip) << L2XTSLOTSIZE);

                /* update page header */
                sp->header.nextindex = middle;
                sxtlck->lwm.offset = (sxtlck->lwm.offset) ? MIN(middle, sxtlck->lwm.offset) : middle;

                rp->header.nextindex = XTENTRYSTART + righthalf + 1;
        }

        sxtlck->lwm.length = sp->header.nextindex - sxtlck->lwm.offset;

        /* rxtlck->lwm.offset = XTENTRYSTART; */
        rxtlck->lwm.length = rp->header.nextindex - XTENTRYSTART;

        *rbpp = rbp;
        *rbnp = rbn;

        ip->i_nblocks += lengthPXD(pxd);

jEVENT(0,("xtSplitPage: lp:0x%08x rp:0x%08x\n", sp, rp));
        return rc;
}


/*
 *      xtSplitRoot()
 *
 * function:
 *      split the full root page into
 *      original/root/split page and new right page
 *      i.e., root remains fixed in tree anchor (inode) and
 *      the root is copied to a single new right child page
 *      since root page << non-root page, and
 *      the split root page contains a single entry for the
 *      new right child page.
 *
 * parameter:
 *      int32           tid,
 *      inode_t         *ip,
 *      xtsplit_t       *split,
 *      jbuf_t          **rbpp)
 *
 * return:
 *      Pointer to page in which to insert or NULL on error.
 */
static int32
xtSplitRoot(
        int32           tid,
        inode_t         *ip,
        xtsplit_t       *split,
        jbuf_t          **rbpp)
{
        xtpage_t        *sp;
        jbuf_t          *rbp;
        xtpage_t        *rp;
        int64           rbn;
        int32           skip, nextindex;
        xad_t           *xad;
        pxd_t           *pxd;
        pxdlist_t       *pxdlist;
        tlock_t         *tlck;
        xtlock_t        *xtlck;

        sp = (xtpage_t *)&ip->i_btroot;

        INCREMENT(xtStat.split);

        /*
         *      allocate a single (right) child page
         */
        pxdlist = split->pxdlist;
        pxd = &pxdlist->pxd[pxdlist->npxd];
        pxdlist->npxd++;
        rbn = addressPXD(pxd);
        rbp = bmAssign(ip, rbn, rbn, PSIZE, bmREAD_BLOCK);
        /* rbp->j_lblkno = rbn; */

jEVENT(0,("xtSplitRoot: ip:0x%08x bp:0x%08x\n", ip, rbp));

        /*
         * acquire a transaction lock on the new right page;
         *
         * action: new page;
         */
        tlck = txLock(tid, ip, rbp, tlckXTREE|tlckNEW);
        xtlck = (xtlock_t *)&tlck->lock;

        rp = (xtpage_t *)rbp->b_bdata;
        rp->header.flag = (sp->header.flag & BT_LEAF) ? BT_LEAF : BT_INTERNAL;
        rp->header.self = *pxd;
        rp->header.nextindex = XTENTRYSTART;
        rp->header.maxentry = PSIZE >> L2XTSLOTSIZE;

        /* initialize sibling pointers */
        rp->header.next = 0;
        rp->header.prev = 0;

        /*
         * copy the in-line root page into new right page extent
         */
        nextindex = sp->header.maxentry;
        memmove(&rp->xad[XTENTRYSTART], &sp->xad[XTENTRYSTART],
                (nextindex - XTENTRYSTART) << L2XTSLOTSIZE);

        /*
         * insert the new entry into the new right/child page
         * (skip index in the new right page will not change)
         */
        skip = split->index;
        /* if insert into middle, shift right remaining entries */
        if (skip != nextindex)
                memmove(&rp->xad[skip + 1], &rp->xad[skip],
                        (nextindex - skip) * sizeof(xad_t));

        xad = &rp->xad[skip];
        XT_PUTENTRY(xad, split->flag, split->off, split->len, split->addr);

        /* update page header */
        rp->header.nextindex = nextindex + 1;

        xtlck->lwm.offset = XTENTRYSTART;
        xtlck->lwm.length = rp->header.nextindex - XTENTRYSTART;

        /*
         *      reset the root
         *
         * init root with the single entry for the new right page
         * set the 1st entry offset to 0, which force the left-most key
         * at any level of the tree to be less than any search key.
         */
        /*
         * acquire a transaction lock on the root page (in-memory inode);
         *
         * action: root split;
         */
        tlck = txLock(tid, ip, split->bp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        xad = &sp->xad[XTENTRYSTART];
        XT_PUTENTRY(xad, XAD_NEW, 0, ip->i_ipmnt->i_nbperpage, rbn);

        /* update page header of root */
        sp->header.flag &= ~BT_LEAF;
        sp->header.flag |= BT_INTERNAL;

        sp->header.nextindex = XTENTRYSTART + 1;

        xtlck->lwm.offset = XTENTRYSTART;
        xtlck->lwm.length = 1;

        *rbpp = rbp;

        ip->i_nblocks += lengthPXD(pxd);

jEVENT(0,("xtSplitRoot: lp:0x%08x rp:0x%08x\n", sp, rp));
        return 0;
}


/*
 *      xtExtend()
 *
 * function: extend in-place;
 *
 * note: existing extent may or may not have been committed.
 * caller is responsible for pager buffer cache update, and
 * working block allocation map update;
 * update pmap: alloc whole extended extent;
 */
int32
xtExtend(
        int32           tid,    /* transaction id */
        inode_t         *ip,
        int64           xoff,   /* delta extent offset */
        int32           xlen,   /* delta extent length */
        uint32          flag)
{
        int32           rc = 0;
        int32           cmp;
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* base B+-tree index page */
        int64           bn;
        int32           index, nextindex, len;
        btstack_t       btstack;/* traverse stack */
        xtsplit_t       split;  /* split information */
        xad_t           *xad;
        int64           xaddr;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        int32           rootsplit = 0;

/*
printf("xtExtend: nxoff:0x%x%08x nxlen:0x%08x\n", xoff, xlen);
*/

        /* there must exist extent to be extended */
        if (rc = xtSearch(ip, xoff - 1, &cmp, &btstack, 0))
                return rc;
        assert(cmp == 0);

        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /* extension must be contiguous */
        xad = &p->xad[index];
/*
printf("xtExtend: xoff:0x%x%08x xlen:0x%08x xaddr:0x%x%08x\n",
        offsetXAD(xad), lengthXAD(xad), addressXAD(xad));
*/
        assert((offsetXAD(xad) + lengthXAD(xad)) == xoff);

        /*
         * acquire a transaction lock on the leaf page;
         *
         * action: xad insertion/extension;
         */
        tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        /* extend will overflow extent ? */
        xlen = lengthXAD(xad) + xlen;
        if ((len = xlen - MAXXLEN) <= 0)
                goto extendOld;

        /*
         *      extent overflow: insert entry for new extent
         */
insertNew:
        xoff = offsetXAD(xad) + MAXXLEN;
        xaddr = addressXAD(xad) + MAXXLEN;
        nextindex = p->header.nextindex;

        /*
         *      if the leaf page is full, insert the new entry and
         *      propagate up the router entry for the new page from split
         *
         * The xtSplitUp() will insert the entry and unpin the leaf page.
         */
        if (nextindex == p->header.maxentry)
        {
                rootsplit = p->header.flag & BT_ROOT;

                /* xtSpliUp() unpins leaf pages */
                split.bp = bp;
                split.index = index + 1;
                split.flag = XAD_NEW;
                split.off = xoff;       /* split offset */
                split.len = len;
                split.addr = xaddr;
                split.pxdlist = NULL;
                if (rc = xtSplitUp(tid, ip, &split, &btstack))
                        return rc;

                /*
                 * if leaf root has been split, original root has been
                 * copied to new child page, i.e., original entry now
                 * resides on the new child page;
                 */
                if (rootsplit)
                {
                        if (p->header.nextindex == XTENTRYSTART + 1)
                        {
                                xad = &p->xad[XTENTRYSTART];
                                bn = addressXAD(xad);

                                /* get new child page */
                                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);

                                tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
                                xtlck = (xtlock_t *)&tlck->lock;
                        }
                }
                else
                        /* get back old page */
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        }
        /*
         *      insert the new entry into the leaf page
         */
        else
        {
                /* insert the new entry: mark the entry NEW */
                xad = &p->xad[index + 1];
                XT_PUTENTRY(xad, XAD_NEW, xoff, xlen, xaddr);

                /* advance next available entry index */
                ++p->header.nextindex;
        }

        /* get back old entry */
        xad = &p->xad[index];
        xlen = MAXXLEN;

        /*
         * extend old extent
         */
extendOld:
        if (!(xad->flag & XAD_NEW))
                xad->flag |= XAD_EXTENDED;
        XADlength(xad, xlen);

        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index, xtlck->lwm.offset) : index;
        xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

        /* unpin the leaf page */
        XT_PUTPAGE(bp);

        ip->i_flag |= IUPDNEW;
        return rc;
}


/*
 *      xtTailgate()
 *
 * function: split existing 'tail' extent
 *      (split offset >= start offset of tail extent), and
 *      relocate and extend the split tail half;
 *
 * note: existing extent may or may not have been committed.
 * caller is responsible for pager buffer cache update, and
 * working block allocation map update;
 * update pmap: free old split tail extent, alloc new extent;
 */
int32
xtTailgate(
        int32           tid,    /* transaction id */
        inode_t         *ip,
        int64           xoff,   /* split/new extent offset */
        int32           xlen,   /* new extent length */
        int64           xaddr,  /* new extent address */
        uint32          flag)
{
        int32           rc = 0;
        int32           cmp;
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* base B+-tree index page */
        int64           bn;
        int32           index, nextindex, llen, rlen;
        btstack_t       btstack;/* traverse stack */
        xtsplit_t       split;  /* split information */
        xad_t           *xad;
        pxd_t           *pxd;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        tlock_t         *mtlck;
        maplock_t       *pxdlock;
        int32           rootsplit = 0;

/*
printf("xtTailgate: nxoff:0x%x%08x nxlen:0x%08x nxaddr:0x%x%08x\n",
        xoff, xlen, xaddr);
*/

        /* there must exist extent to be tailgated */
        if (rc = xtSearch(ip, xoff, &cmp, &btstack, XT_INSERT))
                return rc;
        assert(cmp == 0);

        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /* entry found must be last entry */
        nextindex = p->header.nextindex;
        assert(index == nextindex - 1);

        /*
         * acquire tlock of the leaf page containing original entry
         */
        tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        /* completely replace extent ? */
        xad = &p->xad[index];
/*
printf("xtTailgate: xoff:0x%x%08x xlen:0x%08x xaddr:0x%x%08x\n",
        offsetXAD(xad), lengthXAD(xad), addressXAD(xad));
*/
        if ((llen = xoff - offsetXAD(xad)) == 0)
                goto updateOld;

        /*
         *      partially replace extent: insert entry for new extent
         */
insertNew:
        /*
         *      if the leaf page is full, insert the new entry and
         *      propagate up the router entry for the new page from split
         *
         * The xtSplitUp() will insert the entry and unpin the leaf page.
         */
        if (nextindex == p->header.maxentry)
        {
                rootsplit = p->header.flag & BT_ROOT;

                /* xtSpliUp() unpins leaf pages */
                split.bp = bp;
                split.index = index + 1;
                split.flag = XAD_NEW;
                split.off = xoff;       /* split offset */
                split.len = xlen;
                split.addr = xaddr;
                split.pxdlist = NULL;
                if (rc = xtSplitUp(tid, ip, &split, &btstack))
                        return rc;

                /*
                 * if leaf root has been split, original root has been
                 * copied to new child page, i.e., original entry now
                 * resides on the new child page;
                 */
                if (rootsplit)
                {
                        if (p->header.nextindex == XTENTRYSTART + 1)
                        {
                                xad = &p->xad[XTENTRYSTART];
                                bn = addressXAD(xad);

                                /* get new child page */
                                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);

                                tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
                                xtlck = (xtlock_t *)&tlck->lock;
                        }
                }
                else
                        /* get back old page */
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        }
        /*
         *      insert the new entry into the leaf page
         */
        else
        {
                /* insert the new entry: mark the entry NEW */
                xad = &p->xad[index + 1];
                XT_PUTENTRY(xad, XAD_NEW, xoff, xlen, xaddr);

                /* advance next available entry index */
                ++p->header.nextindex;
        }

        /* get back old XAD */
        xad = &p->xad[index];

        /*
         * truncate/relocate old extent at split offset
         */
updateOld:
        /* update dmap for old/committed/truncated extent */
        rlen = lengthXAD(xad) - llen;
        if (!(xad->flag & XAD_NEW))
        {
                /* free from PWMAP at commit */
                mtlck = txMaplock(tid, ip, tlckMAP);
                pxdlock = (maplock_t *)&mtlck->lock;
                pxdlock->flag = mlckFREEPXD;
                PXDaddress(&pxdlock->pxd, addressXAD(xad) + llen);
                PXDlength(&pxdlock->pxd, rlen);
                pxdlock->index = 1;
jEVENT(0,("xtTailgate: free extent xaddr:0x%x%08x xlen:0x%08x\n",
        addressPXD(&pxdlock->pxd), lengthPXD(&pxdlock->pxd)));
        }
        else
                /* free from WMAP */
                dbFree(ip, addressXAD(xad)+llen, (int64)rlen);

        if (llen)
                /* truncate */
                XADlength(xad, llen);
        else
                /* replace */
                XT_PUTENTRY(xad, XAD_NEW, xoff, xlen, xaddr);

        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index, xtlck->lwm.offset) : index;
        xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

        /* unpin the leaf page */
        XT_PUTPAGE(bp);

        ip->i_flag |= IUPDNEW;
        return rc;
}


/*
 *      xtUpdate()
 *
 * function: update XAD;
 *
 *      update extent for allocated_but_not_recorded or
 *      compressed extent;
 *
 * parameter:
 *      nxad    - new XAD;
 *                logical extent of the specified XAD must be completely
 *                contained by an existing XAD;
 */
xtUpdate(
        int32           tid,
        inode_t         *ip,
        xad_t           *nxad)  /* new XAD */
{
        int32           rc = 0;
        int32           cmp;
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* base B+-tree index page */
        int64           bn;
        int32           index0, index, newindex, nextindex;
        btstack_t       btstack;/* traverse stack */
        xtsplit_t       split;  /* split information */
        xad_t           *xad, *lxad, *rxad;
        uint8           xflag;
        int64           nxoff, xoff;
        int32           nxlen, xlen, lxlen, rxlen;
        int64           nxaddr, xaddr;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        int32           rootsplit = 0, newpage = 0;

        /* there must exist extent to be tailgated */
        nxoff = offsetXAD(nxad);
        nxlen = lengthXAD(nxad);
        nxaddr = addressXAD(nxad);
/*
printf("xtUpdate: nxflag:0x%x nxoff:0x%x%08x nxlen:0x%08x nxaddr:0x%x%08x\n",
        nxad->flag, nxoff, nxlen, nxaddr);
*/
        if (rc = xtSearch(ip, nxoff, &cmp, &btstack, XT_INSERT))
                return rc;
        assert(cmp == 0);

        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index0);

        /*
         * acquire tlock of the leaf page containing original entry
         */
        tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        xad = &p->xad[index0];
        xflag = xad->flag;
        xoff = offsetXAD(xad);
        xlen = lengthXAD(xad);
        xaddr = addressXAD(xad);
/*
printf("xtUpdate: xflag:0x%x xoff:0x%x%08x xlen:0x%08x xaddr:0x%x%08x\n",
        xflag, xoff, xlen, xaddr);
*/

        /* nXAD must be completely contained within XAD */
        assert(xoff <= nxoff);
        assert(nxoff + nxlen <= xoff + xlen);

        index = index0;
        newindex = index + 1;
        nextindex = p->header.nextindex;

#ifdef  _JFS_WIP_NOCOALESCE
        if (xoff < nxoff)
                goto updateRight;

        /*
         * replace XAD with nXAD
         */
replace: /* (nxoff == xoff) */
        if (nxlen == xlen)
        {
                /* replace XAD with nXAD:recorded */
                *xad = *nxad;
                xad->flag = xflag & ~XAD_NOTRECORDED;

                goto out;
        }
        else /* (nxlen < xlen) */
                goto updateLeft;
#endif  /* _JFS_WIP_NOCOALESCE */

/* #ifdef _JFS_WIP_COALESCE */
        if (xoff < nxoff)
                goto coalesceRight;

        /*
         * coalesce with left XAD
         */
coalesceLeft: /* (xoff == nxoff) */
        /* is XAD first entry of page ? */
        if (index == XTENTRYSTART)
                goto replace;

        /* is nXAD logically and physically contiguous with lXAD ? */
        lxad = &p->xad[index - 1];
        lxlen = lengthXAD(lxad);
        if (!(lxad->flag & XAD_NOTRECORDED) &&
            (nxoff == offsetXAD(lxad) + lxlen) &&
            (nxaddr == addressXAD(lxad) + lxlen) &&
            (lxlen + nxlen < MAXXLEN))
        {
                /* extend right lXAD */
                index0 = index - 1;
                XADlength(lxad, lxlen + nxlen);

                /* If we just merged two extents together, need to make sure the
                 * right extent gets logged.  If the left one is marked XAD_NEW,
                 * then we know it will be logged.  Otherwise, mark as
                 * XAD_EXTENDED
                 */
                if (!(lxad->flag & XAD_NEW))
                        lxad->flag |= XAD_EXTENDED;

                if (xlen > nxlen)
                {
                        /* truncate XAD */
                        XADoffset(xad, xoff + nxlen);
                        XADlength(xad, xlen - nxlen);
                        XADaddress(xad, xaddr + nxlen);
                        goto out;
                }
                else /* (xlen == nxlen) */
                {
                        /* remove XAD */
                        if (index < nextindex - 1)
                                memmove(&p->xad[index], &p->xad[index + 1],
                                        (nextindex - index - 1) << L2XTSLOTSIZE);

                        p->header.nextindex--;

                        index = index0;
                        newindex = index + 1;
                        nextindex = p->header.nextindex;
                        xoff = nxoff = offsetXAD(lxad);
                        xlen = nxlen = lxlen + nxlen;
                        xaddr = nxaddr = addressXAD(lxad);
                        goto coalesceRight;
                }
        }

        /*
         * replace XAD with nXAD
         */
replace: /* (nxoff == xoff) */
        if (nxlen == xlen)
        {
                /* replace XAD with nXAD:recorded */
                *xad = *nxad;
                xad->flag = xflag & ~XAD_NOTRECORDED;

                goto coalesceRight;
        }
        else /* (nxlen < xlen) */
                goto updateLeft;

        /*
         * coalesce with right XAD
         */
coalesceRight: /* (xoff <= nxoff) */
        /* is XAD last entry of page ? */
        if (newindex == nextindex)
        {
                if (xoff == nxoff)
                        goto out;
                goto updateRight;
        }

        /* is nXAD logically and physically contiguous with rXAD ? */
        rxad = &p->xad[index + 1];
        rxlen = lengthXAD(rxad);
        if (!(rxad->flag & XAD_NOTRECORDED) &&
            (nxoff + nxlen == offsetXAD(rxad)) &&
            (nxaddr + nxlen == addressXAD(rxad)) &&
            (rxlen + nxlen < MAXXLEN))
        {
                /* extend left rXAD */
                XADoffset(rxad, nxoff);
                XADlength(rxad, rxlen + nxlen);
                XADaddress(rxad, nxaddr);

                /* If we just merged two extents together, need to make sure
                 * the left extent gets logged.  If the right one is marked
                 * XAD_NEW, then we know it will be logged.  Otherwise, mark as
                 * XAD_EXTENDED
                 */
                if (!(rxad->flag & XAD_NEW))
                        rxad->flag |= XAD_EXTENDED;

                if (xlen > nxlen)
                        /* truncate XAD */
                        XADlength(xad, xlen - nxlen);
                else /* (xlen == nxlen) */
                {
                        /* remove XAD */
                        memmove(&p->xad[index], &p->xad[index + 1],
                                (nextindex - index - 1) << L2XTSLOTSIZE);

                        p->header.nextindex--;
                }

                goto out;
        }
        else if (xoff == nxoff)
                goto out;

        assert(xoff < nxoff);
/* #endif _JFS_WIP_COALESCE */

        /*
         * split XAD into (lXAD, nXAD):
         *
         *          |---nXAD--->
         * --|----------XAD----------|--
         *   |-lXAD-|
         */
updateRight: /* (xoff < nxoff) */
        /* truncate old XAD as lXAD:not_recorded */
        xad = &p->xad[index];
        XADlength(xad, nxoff - xoff);

        /* insert nXAD:recorded */
        if (nextindex == p->header.maxentry)
        {
/*
printf("xtUpdate.updateRight.split p:0x%08x\n", p);
*/
                rootsplit = p->header.flag & BT_ROOT;

                /* xtSpliUp() unpins leaf pages */
                split.bp = bp;
                split.index = newindex;
                split.flag = xflag & ~XAD_NOTRECORDED;
                split.off = nxoff;
                split.len = nxlen;
                split.addr = nxaddr;
                split.pxdlist = NULL;
                if (rc = xtSplitUp(tid, ip, &split, &btstack))
                        return rc;

                /*
                 * if leaf root has been split, original root has been
                 * copied to new child page, i.e., original entry now
                 * resides on the new child page;
                 */
                if (rootsplit)
                {
                        if (p->header.nextindex == XTENTRYSTART + 1)
                        {
                                xad = &p->xad[XTENTRYSTART];
                                bn = addressXAD(xad);

                                /* get new child page */
                                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);

                                tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
                                xtlck = (xtlock_t *)&tlck->lock;
                        }
                }
                else
                {
                        /* get back old page */
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);

                        /* is nXAD on new page ? */
                        if (newindex > (p->header.maxentry >> 1))
                        {
                                newindex = newindex - p->header.nextindex
                                           + XTENTRYSTART;
                                newpage = 1;
                        }
                }
        }
        else
        {
                /* if insert into middle, shift right remaining entries */
                if (newindex < nextindex)
                        memmove(&p->xad[newindex + 1], &p->xad[newindex],
                                (nextindex - newindex) << L2XTSLOTSIZE);

                /* insert the entry */
                xad = &p->xad[newindex];
                *xad = *nxad;
                xad->flag = xflag & ~XAD_NOTRECORDED;

                /* advance next available entry index. */
                ++p->header.nextindex;
        }

        /*
         * does nXAD force 3-way split ?
         *
         *          |---nXAD--->|
         * --|----------XAD-------------|--
         *   |-lXAD-|           |-rXAD -|
         */
        if (nxoff + nxlen == xoff + xlen)
                goto out;

        /* reorient nXAD as XAD for further split XAD into (nXAD, rXAD) */
        if (newpage)
        {
                /* close out old page */
                xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index0, xtlck->lwm.offset) : index0;
                xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

                bn = p->header.next;
                XT_PUTPAGE(bp);

                /* get new right page */
                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);

                tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
                xtlck = (xtlock_t *)&tlck->lock;

                index0 = index = newindex;
        }
        else
                index++;

        newindex = index + 1;
        nextindex = p->header.nextindex;
        xlen = xlen - (nxoff - xoff);
        xoff = nxoff;
        xaddr = nxaddr;

        /* recompute split pages */
        if (nextindex == p->header.maxentry)
        {
/*
printf("xtUpdate: updateRight+Left recompute split pages: p:0x%08x\n", p);
*/
                XT_PUTPAGE(bp);

                if (rc = xtSearch(ip, nxoff, &cmp, &btstack, XT_INSERT))
                        return rc;
                assert(cmp == 0);

                /* retrieve search result */
                BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index0);
                assert(index0 == index);
        }

        /*
         * split XAD into (nXAD, rXAD)
         *
         *          ---nXAD---|
         * --|----------XAD----------|--
         *                    |-rXAD-|
         */
updateLeft: /* (nxoff == xoff) && (nxlen < xlen) */
        /* update old XAD with nXAD:recorded */
        xad = &p->xad[index];
        *xad = *nxad;
        xad->flag = xflag & ~XAD_NOTRECORDED;

        /* insert rXAD:not_recorded */
        xoff = xoff + nxlen;
        xlen = xlen - nxlen;
        xaddr = xaddr + nxlen;
        if (nextindex == p->header.maxentry)
        {
                rootsplit = p->header.flag & BT_ROOT;

/*
printf("xtUpdate.updateLeft.split p:0x%08x\n", p);
*/
                /* xtSpliUp() unpins leaf pages */
                split.bp = bp;
                split.index = newindex;
                split.flag = xflag;
                split.off = xoff;
                split.len = xlen;
                split.addr = xaddr;
                split.pxdlist = NULL;
                if (rc = xtSplitUp(tid, ip, &split, &btstack))
                        return rc;

                /*
                 * if leaf root has been split, original root has been
                 * copied to new child page, i.e., original entry now
                 * resides on the new child page;
                 */
                if (rootsplit)
                {
                        if (p->header.nextindex == XTENTRYSTART + 1)
                        {
                                xad = &p->xad[XTENTRYSTART];
                                bn = addressXAD(xad);

                                /* get new child page */
                                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);

                                tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
                                xtlck = (xtlock_t *)&tlck->lock;
                        }
                }
                else
                        /* get back old page */
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        }
        else
        {
                /* if insert into middle, shift right remaining entries */
                if (newindex < nextindex)
                        memmove(&p->xad[newindex + 1], &p->xad[newindex],
                                (nextindex - newindex) << L2XTSLOTSIZE);

                /* insert the entry */
                xad = &p->xad[newindex];
                XT_PUTENTRY(xad, xflag, xoff, xlen, xaddr);

                /* advance next available entry index. */
                ++p->header.nextindex;
        }

out:
        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index0, xtlck->lwm.offset) : index0;
        xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

        /* unpin the leaf page */
        XT_PUTPAGE(bp);

        ip->i_flag |= IUPDNEW;
        return rc;
}


/*
 *      xtAppend()
 *
 * function: grow in append mode from contiguous region specified ;
 *
 * parameter:
 *      tid             - transaction id;
 *      ip              - file object;
 *      xflag           - extent flag:
 *      xoff            - extent offset;
 *      maxblocks       - max extent length;
 *      xlen            - extent length (in/out);
 *      xaddrp          - extent address pointer (in/out):
 *      flag            -
 *
 * return:
 */
int32 xtAppend(
        int32           tid,    /* transaction id */
        inode_t         *ip,
        uint8           xflag,
        int64           xoff,
        int32           maxblocks,  /* @GD1 */
        int32           *xlenp, /* (in/out) */
        int64           *xaddrp,/* (in/out) */
        uint32          flag)
{
        int32           rc = 0;
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* base B+-tree index page */
        int64           bn, xaddr;
        int32           index, nextindex;
        btstack_t       btstack;/* traverse stack */
        xtsplit_t       split;  /* split information */
        xad_t           *xad;
        int32           cmp;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        int32           nsplit, nblocks, xlen;
        pxdlist_t       pxdlist;
        pxd_t           *pxd;

        xaddr = *xaddrp;
        xlen = *xlenp;
jEVENT(0,("xtAppend: xoff:%lld maxblocks:%d xlen:%d xaddr:%lld\n", xoff, maxblocks, xlen, xaddr));

        /*
         *      search for the entry location at which to insert:
         *
         * xtFastSearch() and xtSearch() both returns (leaf page
         * pinned, index at which to insert).
         * n.b. xtSearch() may return index of maxentry of
         * the full page.
         */
        if (rc = xtSearch(ip, xoff, &cmp, &btstack, XT_INSERT))
                return rc;
        if (cmp == 0)
        {
                rc = EEXIST;
                goto out;
        }

insert:
        /* retrieve search result */
        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);

        /*
         *      insert entry for new extent
         */
        xflag |= XAD_NEW;

        /*
         *      if the leaf page is full, split the page and
         *      propagate up the router entry for the new page from split
         *
         * The xtSplitUp() will insert the entry and unpin the leaf page.
         */
        nextindex = p->header.nextindex;
        if (nextindex < p->header.maxentry)
                goto insertLeaf;

        /*
         * allocate new index blocks to cover index page split(s)
         */
        nsplit = btstack.nsplit;
        split.pxdlist = &pxdlist;
        pxdlist.maxnpxd = pxdlist.npxd = 0;
        pxd = &pxdlist.pxd[0];
        nblocks = ip->i_ipmnt->i_nbperpage;
        for (; nsplit > 0; nsplit--, pxd++, xaddr += nblocks, maxblocks -= nblocks)   /* @GD1 */
        {
                if ((rc = dbAllocBottomUp(ip, xaddr, (int64)nblocks)) == 0)
                {
                        PXDaddress(pxd, xaddr);
                        PXDlength(pxd, nblocks);

                        pxdlist.maxnpxd++;

                        continue;
                }

                /* undo allocation */

                goto out;
        }

        xlen = MIN(xlen, maxblocks);            /* @GD1 */

        /*
         * allocate data extent requested
         */
        if (rc = dbAllocBottomUp(ip, xaddr, (int64)xlen))
                goto out;

        split.bp = bp;
        split.index = index;
        split.flag = xflag;
        split.off = xoff;
        split.len = xlen;
        split.addr = xaddr;
        if (rc = xtSplitUp(tid, ip, &split, &btstack))
        {
                /* undo data extent allocation */
                dbFree(ip, *xaddrp, (int64)*xlenp);

                return rc;
        }

        *xaddrp = xaddr;
        *xlenp = xlen;
        ip->i_flag |= IUPD;
        return 0;

        /*
         *      insert the new entry into the leaf page
         */
insertLeaf:
        /*
         * allocate data extent requested
         */
        if (rc = dbAllocBottomUp(ip, xaddr, (int64)xlen))
                goto out;

        /*
         * acquire a transaction lock on the leaf page;
         *
         * action: xad insertion/extension;
         */
        tlck = txLock(tid, ip, bp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        /* insert the new entry: mark the entry NEW */
        xad = &p->xad[index];
        XT_PUTENTRY(xad, xflag, xoff, xlen, xaddr);

        /* advance next available entry index */
        ++p->header.nextindex;

        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index, xtlck->lwm.offset) : index;
        xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

        *xaddrp = xaddr;
        *xlenp = xlen;
        ip->i_flag |= IUPD;

out:
        /* unpin the leaf page */
        XT_PUTPAGE(bp);

        return rc;
}


/* - TBD for defragmentaion/reorganization -
 *
 *      xtDelete()
 *
 * function:
 *      delete the entry with the specified key.
 *
 *      N.B.: whole extent of the entry is assumed to be deleted.
 *
 * parameter:
 *
 * return:
 *       ENOENT: if the entry is not found.
 *
 * exception:
 */
int32
xtDelete(
        int32           tid,
        inode_t         *ip,
        int64           xoff,
        int32           xlen,
        uint32          flag)
{
        int32           rc = 0;
        btstack_t       btstack;
        int32           cmp;
        int64           bn;
        jbuf_t          *bp;
        xtpage_t        *p;
        int32           index, nextindex;
        tlock_t         *tlck;
        xtlock_t        *xtlck;

        /*
         * find the matching entry; xtSearch() pins the page
         */
        if (rc = xtSearch(ip, xoff, &cmp, &btstack, 0))
                return rc;

        BT_GETSEARCH(ip, btstack.top, bn, bp, xtpage_t, p, index);
        if (cmp) {
                /* unpin the leaf page */
                XT_PUTPAGE(bp);
                return ENOENT;
        }

        /*
         * delete the entry from the leaf page
         */
        nextindex = p->header.nextindex;
        --p->header.nextindex;

        /*
         * if the leaf page bocome empty, free the page
         */
        if (p->header.nextindex == XTENTRYSTART)
                return (xtDeleteUp(tid, ip, bp, p, &btstack));

        /*
         * acquire a transaction lock on the leaf page;
         *
         * action:xad deletion;
         */
        tlck = txLock(tid, ip, bp, tlckXTREE);
        xtlck = (xtlock_t *)&tlck->lock;
        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index, xtlck->lwm.offset) : index;

        /* if delete from middle, shift left/compact the remaining entries */
        if (index < nextindex - 1)
                memmove(&p->xad[index], &p->xad[index + 1],
                        (nextindex - index - 1) * sizeof(xad_t));

        XT_PUTPAGE(bp);

        return 0;
}


/* - TBD for defragmentaion/reorganization -
 *
 *      xtDeleteUp()
 *
 * function:
 *      free empty pages as propagating deletion up the tree
 *
 * parameter:
 *
 * return:
 */
static int32
xtDeleteUp(
        int32           tid,
        inode_t         *ip,
        jbuf_t          *fbp,
        xtpage_t        *fp,
        btstack_t       *btstack)
{
        int32           rc = 0;
        jbuf_t          *bp;
        xtpage_t        *p;
        int32           index, nextindex;
        int64           xaddr;
        int32           xlen;
        btframe_t       *parent;
        tlock_t         *tlck;
        xtlock_t        *xtlck;

        /*
         * keep root leaf page which has become empty
         */
        if (fp->header.flag & BT_ROOT)
        {
                /* keep the root page */
                fp->header.flag &= ~BT_INTERNAL;
                fp->header.flag |= BT_LEAF;
                fp->header.nextindex = XTENTRYSTART;

                /* XT_PUTPAGE(fbp); */

                return 0;
        }

        /*
         * free non-root leaf page
         */
        if (rc = xtRelink(tid, ip, fp))
                return rc;

        xaddr = addressPXD(&fp->header.self);
        xlen = lengthPXD(&fp->header.self);
        /* free the page extent */
        dbFree(ip, xaddr, (int64)xlen);

        /* free the buffer page */
        bmInvalidate(fbp);

        /*
         * propagate page deletion up the index tree
         *
         * If the delete from the parent page makes it empty,
         * continue all the way up the tree.
         * stop if the root page is reached (which is never deleted) or
         * if the entry deletion does not empty the page.
         */
        while ((parent = BT_POP(btstack)) != NULL)
        {
                /* get/pin the parent page <sp> */
                XT_GETPAGE(ip, parent->bn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;

                index = parent->index;

                /* delete the entry for the freed child page from parent.
                 */
                nextindex = p->header.nextindex;

                /*
                 * the parent has the single entry being deleted:
                 * free the parent page which has become empty.
                 */
                if (nextindex == 1)
                {
                        if (p->header.flag & BT_ROOT)
                        {
                                /* keep the root page */
                                p->header.flag &= ~BT_INTERNAL;
                                p->header.flag |= BT_LEAF;
                                p->header.nextindex = XTENTRYSTART;

                                /* XT_PUTPAGE(fbp); */

                                break;
                        }
                        else
                        {
                                /* free the parent page */
                                if (rc = xtRelink(tid, ip, p))
                                        return rc;

                                xaddr = addressPXD(&p->header.self);
                                /* free the page extent */
                                dbFree(ip, xaddr, (int64)ip->i_ipmnt->i_nbperpage);

                                /* unpin/free the buffer page */
                                bmInvalidate(bp);

                                /* propagate up */
                                continue;
                        }
                }
                /*
                 * the parent has other entries remaining:
                 * delete the router entry from the parent page.
                 */
                else
                {
                        /*
                         * acquire a transaction lock on the leaf page;
                         *
                         * action:xad deletion;
                         */
                        tlck = txLock(tid, ip, bp, tlckXTREE);
                        xtlck = (xtlock_t *)&tlck->lock;
                        xtlck->lwm.offset = (xtlck->lwm.offset) ? MIN(index, xtlck->lwm.offset) : index;

                        /* if delete from middle,
                         * shift left/compact the remaining entries in the page
                         */
                        if (index < nextindex - 1)
                                memmove(&p->xad[index], &p->xad[index + 1],
                                        (nextindex - index - 1) << L2XTSLOTSIZE);

                        --p->header.nextindex;
jEVENT(0,("xtDeleteUp(entry): 0x%llx[%d]\n", parent->bn, index));
                }

                /* unpin the parent page */
                XT_PUTPAGE(bp);

                /* exit propagation up */
                break;
        }

        return 0;
}


/*
 * NAME:        xtRelocate()
 *
 * FUNCTION:    relocate xtpage or data extent of regular file;
 *              This function is mainly used by defragfs utility.
 *
 * NOTE:        This routine does not have the logic to handle
 *              uncommitted allocated extent. The caller should call
 *              txCommit() to commit all the allocation before call
 *              this routine.
 */
xtRelocate(int32        tid,
        inode_t         *ip,
        xad_t           *oxad,  /* old XAD */
        int64           nxaddr, /* new xaddr */
        int32           xtype)  /* extent type: XTPAGE or DATAEXT */
{
        int32           rc = 0;
        tblock_t        *tblk;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        jbuf_t          *bp,*pbp, *lbp, *rbp;   /* page buffer */
        xtpage_t        *p, *pp, *rp, *lp;      /* base B+-tree index page */
        xad_t           *xad;
        pxd_t           *pxd;
        int64           xoff, xsize;
        int32           xlen;
        int64           oxaddr, sxaddr, dxaddr, nextbn, prevbn;
        cbuf_t          *cp;
        int64           offset, nbytes, nbrd, pno;
        int32           nb, npages, nblks;
        int64           bn;
        int32           cmp;
        int32           index;
        pxdlock_t       *pxdlock;
        btstack_t       btstack;        /* traverse stack */

        xtype = xtype & EXTENT_TYPE;

        xoff = offsetXAD(oxad);
        oxaddr = addressXAD(oxad);
        xlen = lengthXAD(oxad);

        /* validate extent offset */
        offset = xoff << ip->i_ipmnt->i_l2bsize;
        if (offset >= ip->i_size)
                return ESTALE;  /* stale extent */

jEVENT(0,("xtRelocate: xtype:%d xoff:%lld xlen:%d xaddr:%lld:%lld\n",
        xtype, xoff, xlen, oxaddr, nxaddr));

        /*
         *      1. get and validate the parent xtpage/xad entry
         *      covering the source extent to be relocated;
         */
        if (xtype == DATAEXT)
        {
                /* search in leaf entry */
                rc = xtSearch(ip, xoff, &cmp, &btstack, 0);
                if (rc)
                        return rc;
                if (cmp)
                {
                        XT_PUTPAGE(pbp);
                        return ESTALE;
                }

                /* retrieve search result */
                BT_GETSEARCH(ip, btstack.top, bn, pbp, xtpage_t, pp, index);

                /* validate for exact match with a single entry */
                xad = &pp->xad[index];
                if (addressXAD(xad) != oxaddr || lengthXAD(xad) != xlen)
                {
                        XT_PUTPAGE(pbp);
                        return ESTALE;
                }
        }
        else /* (xtype == XTPAGE) */
        {
                /* search in internal entry */
                rc = xtSearchNode(ip, oxad, &cmp, &btstack, 0);
                if (rc)
                        return rc;
                if (cmp)
                {
                        XT_PUTPAGE(pbp);
                        return ESTALE;
                }

                /* retrieve search result */
                BT_GETSEARCH(ip, btstack.top, bn, pbp, xtpage_t, pp, index);

                /* xtSearchNode() validated for exact match with a single entry
                 */
                xad = &pp->xad[index];
        }
jEVENT(0,("xtRelocate: parent xad entry validated.\n"));

        /*
         *      2. relocate the extent
         */
        if (xtype  == DATAEXT)
        {
                /* if the extent is allocated-but-not-recorded
                 * there is no real data to be moved in this extent,
                 */
                if (xad->flag & XAD_NOTRECORDED)
                        goto out;
                else
                        /* release xtpage for cmRead()/xtLookup() */
                        XT_PUTPAGE(pbp);

                /*
                 *      cmRelocate()
                 *
                 * copy target data pages to be relocated;
                 *
                 * data extent must start at page boundary and
                 * multiple of page size (except the last data extent);
                 * read in each page of the source data extent into cbuf,
                 * update the cbuf extent descriptor of the page to be
                 * homeward bound to new dst data extent
                 * copy the data from the old extent to new extent.
                 * copy is essential for compressed files to avoid problems
                 * that can arise if there was a change in compression
                 * algorithms.
                 * it is a good strategy because it may disrupt cache
                 * policy to keep the pages in memory afterwards.
                 */
                offset = xoff << ip->i_ipmnt->i_l2bsize;
                assert((offset & CM_OFFSET) == 0);
                nbytes = xlen << ip->i_ipmnt->i_l2bsize;
                pno = offset >> CM_L2BSIZE;
                npages = (nbytes + (CM_BSIZE - 1)) >> CM_L2BSIZE;
/*
                npages = ((offset + nbytes - 1) >> CM_L2BSIZE) -
                         (offset >> CM_L2BSIZE) + 1;
*/
                sxaddr = oxaddr;
                dxaddr = nxaddr;

                /* process the request one cache buffer at a time */
                for (nbrd = 0; nbrd < nbytes; nbrd += nb,
                     offset += nb, pno++, npages--)
                {
                        /* compute page size */
                        nb = MIN(nbytes - nbrd, CM_BSIZE);

                        /* get the cache buffer of the page */
                        if (rc = cmRead(ip, offset, npages, &cp))
                                break;

                        assert(addressPXD(&cp->cm_pxd) == sxaddr);
                        assert(!cp->cm_modified);

                        /* bind buffer with the new extent address */
                        nblks = nb >> ip->i_ipmnt->i_l2bsize;
                        cmSetXD(ip, cp, pno, dxaddr, nblks);

                        /* release the cbuf, mark it as modified */
                        cmPut(cp, TRUE);

                        dxaddr += nblks;
                        sxaddr += nblks;
                }

                /* get back parent page */
                rc = xtSearch(ip, xoff, &cmp, &btstack, 0);
                BT_GETSEARCH(ip, btstack.top, bn, pbp, xtpage_t, pp, index);
jEVENT(0,("xtRelocate: target data extent relocated.\n"));
        }
        else /* (xtype  == XTPAGE) */
        {
                /*
                 * read in the target xtpage from the source extent;
                 */
                XT_GETPAGE(ip, oxaddr, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                {
                        XT_PUTPAGE(pbp);
                        return rc;
                }

                /*
                 * read in sibling pages if any to update sibling pointers;
                 */
                rbp = NULL;
                if (p->header.next)
                {
                        nextbn = p->header.next;
                        XT_GETPAGE(ip, nextbn, rbp, xtpage_t, PSIZE, rp, rc);
                        if (rc)
                        {
                                XT_PUTPAGE(pbp);
                                XT_PUTPAGE(bp);
                                return (rc);
                        }
                }

                lbp = NULL;
                if (p->header.prev)
                {
                        prevbn = p->header.prev;
                        XT_GETPAGE(ip, prevbn, lbp, xtpage_t, PSIZE, lp, rc);
                        if (rc)
                        {
                                XT_PUTPAGE(pbp);
                                XT_PUTPAGE(bp);
                                if (rbp)
                                        XT_PUTPAGE(rbp);
                                return (rc);
                        }
                }

                /* at this point, all xtpages to be updated are in memory */

                /*
                 * update sibling pointers of sibling xtpages if any;
                 */
                if (lbp)
                {
                        tlck = txLock(tid, ip, lbp, tlckXTREE|tlckRELINK);
                        lp->header.next = nxaddr;
                        XT_PUTPAGE(lbp);
                }

                if (rbp)
                {
                        tlck = txLock(tid, ip, rbp, tlckXTREE|tlckRELINK);
                        rp->header.prev = nxaddr;
                        XT_PUTPAGE(rbp);
                }

                /*
                 * update the target xtpage to be relocated
                 *
                 * update the self address of the target page
                 * and write to destination extent;
                 * redo image covers the whole xtpage since it is new page
                 * to the destination extent;
                 * update of bmap for the free of source extent
                 * of the target xtpage itself:
                 * update of bmap for the allocation of destination extent
                 * of the target xtpage itself:
                 * update of bmap for the extents covered by xad entries in
                 * the target xtpage is not necessary since they are not
                 * updated;
                 * if not committed before this relocation,
                 * target page may contain XAD_NEW entries which must
                 * be scanned for bmap update (logredo() always
                 * scan xtpage REDOPAGE image for bmap update);
                 * if committed before this relocation (tlckRELOCATE),
                 * scan may be skipped by commit() and logredo();
                 */
                /* tlckNEW init  xtlck->lwm.offset = XTENTRYSTART; */
                tlck = txLock(tid, ip, bp, tlckXTREE|tlckNEW);
                xtlck = (xtlock_t *)&tlck->lock;

                /* update the self address in the xtpage header */
                pxd = &p->header.self;
                PXDaddress(pxd, nxaddr);

                /* linelock for the after image of the whole page */
                xtlck->lwm.length = p->header.nextindex - xtlck->lwm.offset;

                /* update the buffer extent descriptor of target xtpage */
                xsize = xlen << ip->i_ipmnt->i_l2bsize;
                bmSetXD(bp, nxaddr, xsize);

                /* unpin the target page to new homeward bound */
                XT_PUTPAGE(bp);
jEVENT(0,("xtRelocate: target xtpage relocated.\n"));
        }

        /*
         *      3. acquire maplock for the source extent to be freed;
         *
         * acquire a maplock saving the src relocated extent address;
         * to free of the extent at commit time;
         */
out:
        /* if DATAEXT relocation, write a LOG_UPDATEMAP record for
         * free PXD of the source data extent (logredo() will update
         * bmap for free of source data extent), and update bmap for
         * free of the source data extent;
         */
        if (xtype == DATAEXT)
                tlck = txMaplock(tid, ip, tlckMAP);
        /* if XTPAGE relocation, write a LOG_NOREDOPAGE record
         * for the source xtpage (logredo() will init NoRedoPage
         * filter and will also update bmap for free of the source
         * xtpage), and update bmap for free of the source xtpage;
         * N.B. We use tlckMAP instead of tlkcXTREE because there
         *      is no buffer associated with this lock since the buffer
         *      has been redirected to the target location.
         */
        else /* (xtype  == XTPAGE) */
                tlck = txMaplock(tid, ip, tlckMAP|tlckRELOCATE);

        pxdlock = (pxdlock_t *)&tlck->lock;
        pxdlock->flag = mlckFREEPXD;
        PXDaddress(&pxdlock->pxd, oxaddr);
        PXDlength(&pxdlock->pxd, xlen);
        pxdlock->index = 1;

        /*
         *      4. update the parent xad entry for relocation;
         *
         * acquire tlck for the parent entry with XAD_NEW as entry
         * update which will write LOG_REDOPAGE and update bmap for
         * allocation of XAD_NEW destination extent;
         */
jEVENT(0,("xtRelocate: update parent xad entry.\n"));
        tlck = txLock(tid, ip, pbp, tlckXTREE|tlckGROW);
        xtlck = (xtlock_t *)&tlck->lock;

        /* update the XAD with the new destination extent; */
        xad = &pp->xad[index];
        xad->flag |= XAD_NEW;
        XADaddress(xad, nxaddr);

        xtlck->lwm.offset = MIN(index, xtlck->lwm.offset);
        xtlck->lwm.length = pp->header.nextindex - xtlck->lwm.offset;

        /* unpin the parent xtpage */
        XT_PUTPAGE(pbp);

        /* remind that object has been updated */
        ip->i_flag |= IUPD;

        return rc;
}


/*
 *      xtSearchNode()
 *
 * function:    search for the internal xad entry covering specified extent.
 *              This function is mainly used by defragfs utility.
 *
 * parameters:
 *      ip      - file object;
 *      xad     - extent to find;
 *      cmpp    - comparison result:
 *      btstack - traverse stack;
 *      flag    - search process flag;
 *
 * returns:
 *      btstack contains (bn, index) of search path traversed to the entry.
 *      *cmpp is set to result of comparison with the entry returned.
 *      the page containing the entry is pinned at exit.
 */
static int32
xtSearchNode(inode_t    *ip,
        xad_t           *xad,   /* required XAD entry */
        int32           *cmpp,
        btstack_t       *btstack,
        uint32          flag)
{
        int32           rc = 0;
        int64           xoff, xaddr;
        int32           xlen;
        int32           cmp = 1;        /* init for empty page */
        int64           bn;     /* block number */
        jbuf_t          *bp;    /* page buffer */
        xtpage_t        *p;     /* page */
        int32           base, index, lim;
        btframe_t       *btsp;
        int64           t64;

        BT_CLR(btstack);

        xoff = offsetXAD(xad);
        xlen = lengthXAD(xad);
        xaddr = addressXAD(xad);

        /*
         *      search down tree from root:
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
                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;
                if (p->header.flag &  BT_LEAF)
                        return ESTALE;

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
                                 *      search hit
                                 *
                                 * verify for exact match;
                                 */
                                if (xaddr == addressXAD(&p->xad[index]) &&
                                    xoff == offsetXAD(&p->xad[index]))
                                {
                                        *cmpp = cmp;

                                        /* save search result */
                                        btsp = btstack->top;
                                        btsp->bn = bn;
                                        btsp->index = index;
                                        btsp->bp = bp;

                                        return 0;
                                }

                                /* descend/search its child page */
                                goto next;
                        }

                        if (cmp > 0) {
                                base = index + 1;
                                --lim;
                        }
                }

                /*
                 *      search miss - non-leaf page:
                 *
                 * base is the smallest index with key (Kj) greater than
                 * search key (K) and may be zero or maxentry index.
                 * if base is non-zero, decrement base by one to get the parent
                 * entry of the child page to search.
                 */
                index = base ? base - 1 : base;

                /*
                 * go down to child page
                 */
next:
                /* get the child page block number */
                bn = addressXAD(&p->xad[index]);

                /* unpin the parent page */
                XT_PUTPAGE(bp);
        }
}


/*
 *      xtRelink()
 *
 * function:
 *      link around a freed page.
 *
 * Parameter:
 *      int32           tid,
 *      inode_t         *ip,
 *      xtpage_t        *p)
 *
 * returns:
 */
static int32
xtRelink(
        int32           tid,
        inode_t         *ip,
        xtpage_t        *p)
{
        int32           rc = 0;
        jbuf_t          *bp;
        int64           nextbn, prevbn;
        tlock_t         *tlck;

        nextbn = p->header.next;
        prevbn = p->header.prev;

        /* update prev pointer of the next page */
        if (nextbn != 0)
        {
                XT_GETPAGE(ip, nextbn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;

                /*
                 * acquire a transaction lock on the page;
                 *
                 * action: update prev pointer;
                 */
                tlck = txLock(tid, ip, bp, tlckXTREE|tlckRELINK);

                /* the page may already have been tlock'd */

                p->header.prev = prevbn;

                XT_PUTPAGE(bp);
        }

        /* update next pointer of the previous page */
        if (prevbn != 0) {
                XT_GETPAGE(ip, prevbn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;

                /*
                 * acquire a transaction lock on the page;
                 *
                 * action: update next pointer;
                 */
                tlck = txLock(tid, ip, bp, tlckXTREE|tlckRELINK);

                /* the page may already have been tlock'd */

                p->header.next = nextbn;

                XT_PUTPAGE(bp);
        }

        return 0;
}


/*
 *      xtInitRoot()
 *
 * initialize file root (inline in inode)
 */
void
xtInitRoot(
        int32   tid,
        inode_t *ip)
{
        xtpage_t        *p;
        tlock_t         *tlck;

        /*
         * acquire a transaction lock on the root
         *
         * action:
         */
        tlck = txLock(tid, ip, (jbuf_t *)&ip->i_bxflag, tlckXTREE|tlckNEW);

        p = (xtpage_t *)&ip->i_btroot;

        p->header.flag = DXD_INDEX | BT_ROOT | BT_LEAF;

        p->header.nextindex = XTENTRYSTART;
        p->header.maxentry = XTROOTINITSLOT;

        ip->i_size = 0;

        return;
}


/*
 *      xtTruncate()
 *
 * function:
 *      traverse for truncation logging backward bottom up;
 *      terminate at the last extent entry at the current subtree
 *      root page covering new down size.
 *      truncation may occur within the last extent entry.
 *
 * parameter:
 *      int32           tid,
 *      inode_t         *ip,
 *      int64           newsize,
 *      int32           type)   {PWMAP, PMAP, WMAP; DELETE, TRUNCATE}
 *
 * return:
 *
 * note:
 *      PWMAP:
 *       1. remove (commit zero link count) with i_pager = 0
 *          by commitZeroLink() at commit time;
 *       2. truncate (non-COMMIT_NOLINK file)
 *          by jfs_truncate() or jfs_open(O_TRUNC):
 *          xtree is updated;
 *       map update via tlock at commit time;
 *      PMAP:
 *       1. remove (commit zero link count) with i_pager != 0
 *          by commitZeroLink() at commit time
 *          (wmap will be freed at last reference release):
 *          xtree is NOT updated (always truncates to 0, i.e.,
 *          all index pages will be freed and not updated);
 *       map update via tlock at commit time;
 *      WMAP:
 *       1. remove (free zero link count) on last reference release
 *          (pmap has been freed at commit zero link count);
 *       2. truncate (COMMIT_NOLINK file, i.e., tmp file):
 *          xtree is updated;
 *       map update directly at truncation time;
 *
 *      if (DELETE)
 *              no LOG_NOREDOPAGE is required (NOREDOFILE is sufficient);
 *      else if (TRUNCATE)
 *              must write LOG_NOREDOPAGE for deleted index page;
 *
 * pages may already have been tlocked by anonymous transactions
 * during file growth (i.e., write) before truncation;
 *
 * except last truncated entry, deleted entries remains as is
 * in the page (nextindex is updated) for other use
 * (e.g., log/update allocation map): this avoid copying the page
 * info but delay free of pages;
 *
 * TODO:
 * subtransaction of incremental truncation for large number of xad's
 * (complete by logredo()/fsck() in case crash);
 */
int32
xtTruncate(
        int32           tid,
        inode_t         *ip,
        int64           newsize,
        int32           flag)
{
        int32           rc = 0;
        int64           teof;
        jbuf_t          *bp;
        xtpage_t        *p;
        int64           bn;
        int32           index, nextindex;
        xad_t           *xad;
        int64           xoff, xaddr;
        int32           xlen, len, freexlen;
        btstack_t       btstack;
        btframe_t       *parent;
        tblock_t        *tblk;
        tlock_t         *tlck;
        xtlock_t        *xtlck;
        xdlistlock_t    xadlock;        /* maplock for COMMIT_WMAP */
        pxdlock_t       *pxdlock;       /* maplock for COMMIT_WMAP */
        int64           nfreed;
        int32           freed, log;
        int32           lid;

        /* save object truncation type */
        if (tid)
        {
                tblk = &TxBlock[tid];
                tblk->xflag |= flag;
        }

        nfreed = 0;

        flag &= COMMIT_MAP;
        if (flag != COMMIT_WMAP)
                log = 1;
        else
        {
                log = 0;
                xadlock.flag = mlckFREEXADLIST;
                xadlock.index = 1;
        }

        /*
         * invalidate data pages of truncated region
         * if it has a segment unless type is COMMIT_PMAP.
         *
         * if the newsize is not an integral number of pages,
         * the file between newsize and next page boundary will
         * be cleared.
         * if truncating into a file hole, it will cause
         * a full block to be allocated for the logical block.
         */
        if (ip->i_cacheid && flag != COMMIT_PMAP)
        {
                if (rc = cmTruncate(ip, newsize, ip->i_size - newsize))
                        return rc;
        }

#ifdef  _JFS_COMPRESSION
        /*
         * if compression, wait for any pending pageouts to complete.
         * since re-allocation of backing disk blocks may occur during
         * pageout.
         */
        if (ip->i_cacheid && ip->i_compress)
                vms_iowait(ip->i_cacheid);
#endif  /* _JFS_COMPRESSION */

        /*
         * release page blocks of truncated region <teof, eof>
         *
         * free the data blocks from the leaf index blocks.
         * delete the parent index entries corresponding to
         * the freed child data/index blocks.
         * free the index blocks themselves which aren't needed
         * in new sized file.
         *
         * index blocks are updated only if the blocks are to be
         * retained in the new sized file.
         * if type is PMAP, the data and index pages are NOT
         * freed, and the data and index blocks are NOT freed
         * from  working map.
         * (this will allow continued access of data/index of
         * temporary file (zerolink count file truncated to zero-length)).
         */
        teof = (newsize + ip->i_ipmnt->i_bsize - 1) >> ip->i_ipmnt->i_l2bsize;

        /* clear stack */
        BT_CLR(&btstack);

        /*
         * start with root
         *
         * root resides in the inode
         */
        bn = 0;

        /*
         * first access of each page:
         */
getPage:
        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        if (rc)
                return rc;

        /* process entries backward from last index */
        index = p->header.nextindex - 1;

        if (p->header.flag & BT_INTERNAL)
                goto getChild;

        /*
         *      leaf page
         */

        /* Since this is the rightmost leaf, and we may have already freed
         * a page that was formerly to the right, let's make sure that the
         * next pointer is zero.
         */
        p->header.next = 0;

        /* does region covered by leaf page precede teof ? */
        xad = &p->xad[index];
        xoff = offsetXAD(xad);
        xlen = lengthXAD(xad);
        if (teof >= xoff + xlen)
        {
                freed = 0;
                XT_PUTPAGE(bp);
                goto getParent;
        }

        /* (re)acquire tlock of the leaf page */
        if (log)
        {
                tlck = txLock(tid, ip, bp, tlckXTREE);
                tlck->type = tlckXTREE | tlckTRUNCATE;
                xtlck = (xtlock_t *)&tlck->lock;
                xtlck->hwm.offset = p->header.nextindex - 1;
        }

        /* is region covered by leaf beyond teof ? */
        if (teof == 0)  /* no need to count nfreed from leaf */
        {
                freed = 1;
                goto freeLeaf;
        }
        else
                freed = 0;

        /*
         * scan backward leaf page entries
         */
        for ( ; index >= XTENTRYSTART; index--)
        {
                xad = &p->xad[index];
                xoff = offsetXAD(xad);
                xlen = lengthXAD(xad);
                xaddr = addressXAD(xad);

                /*
                 * entry beyond eof: continue scan of current page
                 *          xad
                 * ---|---=======------->
                 *   eof
                 */
                if (teof < xoff)
                {
                        nfreed += xlen;
                        continue;
                }

                /*
                 * (xoff <= teof): last entry to be deleted from page;
                 * other entries remain in page: keep and update the page.
                 */

                /*
                 * eof == entry_start: delete the entry
                 *           xad
                 * -------|=======------->
                 *       eof
                 *
                 */
                if (teof == xoff)
                {
                        nextindex = index;

                        nfreed += xlen;
                }
                /*
                 * eof within the entry: truncate the entry.
                 *          xad
                 * -------===|===------->
                 *          eof
                 */
                else if (teof < xoff + xlen)
                {
                        /* update truncated entry */
                        len = teof - xoff;
                        freexlen = xlen - len;
                        XADlength(xad, len);

                        /* save pxd of truncated extent in tlck */
                        xaddr += len;
                        if (log) /* COMMIT_PWMAP|COMMIT_PMAP */
                        {
                                xtlck->lwm.offset = (xtlck->lwm.offset) ?
                                        MIN(index, xtlck->lwm.offset) : index;
                                xtlck->lwm.length = index + 1 -
                                                    xtlck->lwm.offset;
                                pxdlock = (pxdlock_t *)&xtlck->pxdlock;
                                pxdlock->flag = mlckFREEPXD;
                                PXDaddress(&pxdlock->pxd, xaddr);
                                PXDlength(&pxdlock->pxd, freexlen);
                        }
                        /* free truncated extent */
                        else /* COMMIT_WMAP */
                        {
                                pxdlock = (pxdlock_t *)&xadlock;
                                pxdlock->flag = mlckFREEPXD;
                                PXDaddress(&pxdlock->pxd, xaddr);
                                PXDlength(&pxdlock->pxd, freexlen);
                                txFreeMap(ip, pxdlock, 0, COMMIT_WMAP);

                                /* reset map lock */
                                xadlock.flag = mlckFREEXADLIST;
                        }

                        /* current entry is new last entry; */
                        nextindex = index + 1;

                        nfreed += freexlen;
                }
                /*
                 * eof beyond the entry:
                 *          xad
                 * -------=======---|--->
                 *                 eof
                 */
                else /* (xoff + xlen < teof) */
                {
                        nextindex = index + 1;
                }

                if (nextindex < p->header.nextindex)
                {
                        if (!log) /* COMMIT_WAMP */
                        {
                                xadlock.xdlist = &p->xad[nextindex];
                                xadlock.count = p->header.nextindex - nextindex;
                                txFreeMap(ip, (maplock_t *)&xadlock, 0, COMMIT_WMAP);
                        }

                        /* COMMIT_PMAP always truncates to 0 length; */
                        assert(flag != COMMIT_PMAP);
                        p->header.nextindex = nextindex;
                }

                XT_PUTPAGE(bp);

                /* assert(freed == 0); */
                goto getParent;
        } /* end scan of leaf page entries */

        freed = 1;

        /*
         * leaf page become empty: free the page if type != PMAP
         */
freeLeaf:       /* assert(freed == 1); */
        if (log) /* COMMIT_PWMAP|COMMIT_PMAP */
        {
                /* txCommit() with tlckFREE:
                 * free data extents covered by leaf [XTENTRYSTART:hwm);
                 * invalidate leaf if COMMIT_PWMAP;
                 * if (TRUNCATE), will write LOG_NOREDOPAGE;
                 */
                tlck->type = tlckXTREE | tlckFREE;
        }
        else /* COMMIT_WAMP */
        {
                /* free data extents covered by leaf */
                xadlock.xdlist = &p->xad[XTENTRYSTART];
                xadlock.count = p->header.nextindex - XTENTRYSTART;
                txFreeMap(ip, (maplock_t *)&xadlock, 0, COMMIT_WMAP);
        }

        if (p->header.flag & BT_ROOT)
        {
                if (flag != COMMIT_PMAP) /* COMMIT_PWMAP|COMMIT_WMAP */
                {
                        p->header.flag &= ~BT_INTERNAL;
                        p->header.flag |= BT_LEAF;
                        p->header.nextindex = XTENTRYSTART;
                }

                XT_PUTPAGE(bp); /* debug */
                goto out;
        }
        else
        {
                if (log) /* COMMIT_PWMAP|COMMIT_PMAP */
                {
                        /* page will be invalidated for
                         * COMMIT_PMAP at COMMIT_WMAP,
                         * COMMIT_PWMAP at tx completion
                         */
                        XT_PUTPAGE(bp);
                }
                else /* COMMIT_WMAP */
                {
                        if (lid = bp->j_lid)
                                TxLock[lid].flag |= tlckFREELOCK;

                        /* invalidate empty leaf page */
                        bmInvalidate(bp);
                }
        }

        /*
         * the leaf page become empty: delete the parent entry
         * for the leaf page if the parent page is to be kept
         * in the new sized file and if type != PMAP
         */

        /*
         * go back up to the parent page
         */
getParent:
        /* pop/restore parent entry for the current child page */
        if ((parent = BT_POP(&btstack)) == NULL)
                /* current page must have been root */
                goto out;

        /* get back the parent page */
        bn = parent->bn;
        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        if (rc)
                return rc;

        index = parent->index;

        /*
         * child page was not empty:
         */
        if (freed == 0)
        {
                /* has any entry deleted from parent ? */
                if (index < p->header.nextindex - 1)
                {
                        /* (re)acquire tlock on the parent page */
                        if (log) /* COMMIT_PWMAP|COMMIT_PMAP */
                        {
                                /* txCommit() with tlckTRUNCATE:
                                 * free child extents covered by parent [);
                                 */
                                tlck = txLock(tid, ip, bp, tlckXTREE);
                                xtlck = (xtlock_t *)&tlck->lock;
                                xtlck->twm.offset = index;
                                if (!(tlck->type & tlckTRUNCATE))
                                {
                                        xtlck->hwm.offset = p->header.nextindex - 1;
                                        tlck->type = tlckXTREE | tlckTRUNCATE;
                                }
                        }
                        else /* COMMIT_WMAP */
                        {
                                /* free child extents covered by parent */
                                xadlock.xdlist= &p->xad[index+1];
                                xadlock.count = p->header.nextindex - index - 1;
                                txFreeMap(ip, (maplock_t *)&xadlock, 0, COMMIT_WMAP);
                        }

                        p->header.nextindex = index + 1;

                        /* freed = 0; */
                        goto getParent;
                }
                else
                        goto out;
        }

        /*
         * child page was empty:
         */
        nfreed += lengthXAD(&p->xad[index]);

        /*
         * parent page become empty: free the page
         */
        if (index == XTENTRYSTART)
        {
                if (log) /* COMMIT_PWMAP|COMMIT_PMAP */
                {
                        /* txCommit() with tlckFREE:
                         * free child extents covered by parent;
                         * invalidate parent if COMMIT_PWMAP;
                         */
                        tlck = txLock(tid, ip, bp, tlckXTREE);
                        xtlck = (xtlock_t *)&tlck->lock;
                        xtlck->twm.offset = index;
                        xtlck->hwm.offset = p->header.nextindex - 1;
                        tlck->type = tlckXTREE | tlckFREE;
                }
                else /* COMMIT_WMAP */
                {
                        /* free child extents covered by parent */
                        xadlock.xdlist = &p->xad[XTENTRYSTART];
                        xadlock.count = p->header.nextindex - XTENTRYSTART;
                        txFreeMap(ip, (maplock_t *)&xadlock, 0, COMMIT_WMAP);
                }

                if (p->header.flag & BT_ROOT)
                {
                        if (flag != COMMIT_PMAP) /* COMMIT_PWMAP|COMMIT_WMAP */
                        {
                                p->header.flag &= ~BT_INTERNAL;
                                p->header.flag |= BT_LEAF;
                                p->header.nextindex = XTENTRYSTART;
                        }

                        XT_PUTPAGE(bp);         /* debug */
                        goto out;
                }
                else
                {
                        if (log) /* COMMIT_PWMAP|COMMIT_PMAP */
                        {
                                /* page will be invalidated for
                                 * COMMIT_PMAP at COMMIT_WMAP,
                                 * COMMIT_PWMAP at tx completion
                                 */
                                XT_PUTPAGE(bp);
                        }
                        else /* COMMIT_WMAP */
                        {
                                if (lid = bp->j_lid)
                                        TxLock[lid].flag |= tlckFREELOCK;

                                /* invalidate parent page */
                                bmInvalidate(bp);
                        }

                        /* parent has become empty and freed:
                         * go back up to its parent page
                         */
                        /* freed = 1; */
                        goto getParent;
                }
        }
        /*
         * parent page still has entries for front region;
         */
        else
        {
                /* try truncate region covered by preceding entry
                 * (process backward)
                 */
                index--;

                /* go back down to the child page corresponding
                 * to the entry
                 */
                goto getChild;
        }

        /*
         *      internal page: go down to child page of current entry
         */
getChild:
        /* save current parent entry for the child page */
        BT_PUSH(&btstack, bn, index);

        /* get child page */
        xad = &p->xad[index];
        bn = addressXAD(xad);

        /*
         * first access of each internal entry:
         */
        /* release parent page */
        XT_PUTPAGE(bp);

        /* process the child page */
        goto getPage;

out:
        /*
         * update file resource stat unless PMAP only.
         */
        if (flag != COMMIT_PMAP)
        {
                /* set size. ok to do this because scb_combit is set.
                 */
                ip->i_size = newsize;

#ifdef  _JFS_WIP
                /* truncate the new last page of the file's allocation
                 * if the page should be partially back and
                 * is previous to the old last page or
                 * is the old last page and should have fewer fragments.
                 * (not for compression).
                 */
                if (ip->i_compress == 0)
                {
                        nfrags = BTOFR(newsize, fperpage);
                        if (nfrags != fperpage &&
                            (newlastp < oldlastp ||
                             (newlastp == oldlastp && nfrags < oldnfrags)))
                        {
                                vcs_relfrag(ip->i_seg, newlastp, nfrags, &oldf);
                                dlistadd(anchor, oldf.word);
                                nfreed += (fperpage - oldf.fptr.nfrags);
                        }
                }
#endif  /* _JFS_WIP */

                /* update nblocks to reflect freed blocks; can't just set to 0
                 * if newsize == 0 since we may still have blocks for the EAs
                 * and/or ACLs which will be handled by jfs_IntializeFEAList()
                 */
                if (newsize == 0)
                        ip->i_nblocks = ((ip->i_ea.flag & DXD_EXTENT) ?
                                                lengthDXD(&ip->i_ea) : 0) +
                                        ((ip->i_acl.flag & DXD_EXTENT) ?
                                                lengthDXD(&ip->i_acl) : 0);
                else
                        ip->i_nblocks -= nfreed;
        }

        /*
         * free tlock of invalidated pages
         */
        if (flag == COMMIT_WMAP)
                txFreelock(ip);

        return 0;
}


#ifdef _JFS_STATISTICS
/*
 *      xtStatistics
 */
void
xtStatistics()
{
        /* report statistics */
}
#endif  /* _JFS_STATISTICS */


#ifdef _JFS_DEBUG_XTREE
/*
 *      xtDisplayTree()
 *
 * function: traverse forward
 */
int32
xtDisplayTree(
        inode_t         *ip)
{
        int32           rc = 0;
        jbuf_t          *bp;
        xtpage_t        *p;
        int64           bn, pbn;
        int32           index, lastindex, v, h;
        xad_t           *xad;
        btstack_t       btstack;
        btframe_t       *btsp;
        btframe_t       *parent;

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
getPage:
        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        if (rc)
                return rc;

        /* process entries forward from first index */
        index = XTENTRYSTART;
        lastindex = p->header.nextindex - 1;

        if (p->header.flag & BT_INTERNAL)
        {
                /*
                 * first access of each internal page
                 */
                goto getChild;
        }
        else /* (p->header.flag & BT_LEAF) */
        {
                /*
                 * first access of each leaf page
                 */
                printf("leaf page ");
                xtDisplayPage(ip, bn, p);

                /* unpin the leaf page */
                XT_PUTPAGE(bp);
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
        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
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

        /* get child page */
        xad = &p->xad[index];
        bn = addressXAD(xad);

        /*
         * first access of each internal entry:
         */
        /* release parent page */
        XT_PUTPAGE(bp);

        printf("traverse down 0x%x%08x[%d]->0x%x%08x\n", pbn, index, bn);
        v++;
        h = index;

        /* process the child page */
        goto getPage;
}


/*
 *      xtDisplayPage()
 *
 * function: display page
 */
int32
xtDisplayPage(
        inode_t         *ip,
        int64           bn,
        xtpage_t        *p)
{
        int32           rc = 0;
        jbuf_t          *bp;
        xad_t           *xad;
        int64           xaddr, xoff;
        int32           xlen, i, j;

        if (p == NULL)
        {
                XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                if (rc)
                        return rc;
        }

        /* display page control */
        printf("bn:0x%x%08x flag:0x%x nextindex:%d\n",
                bn, p->header.flag, p->header.nextindex);

        /* display entries */
        xad = &p->xad[XTENTRYSTART];
        for (i = XTENTRYSTART, j = 1; i < p->header.nextindex; i++, xad++, j++)
        {
                xoff = offsetXAD(xad);
                xaddr = addressXAD(xad);
                xlen = lengthXAD(xad);
                printf("\t[%d] 0x%x%08x:0x%x%08x(%d)", i, xoff, xaddr, xlen);

                if (j == 4)
                {
                        printf("\n");
                        j = 0;
                }
        }

        printf("\n");
}
#endif  /* _JFS_DEBUG_XTREE */


#ifdef _JFS_WIP
/*
 *      xtGather()
 *
 * function:
 *      traverse for allocation acquiring tlock at commit time
 *      (vs at the time of update) logging backward top down
 *
 * note:
 *      problem - establishing that all new allocation have been
 *      processed both for append and random write in sparse file
 *      at the current entry at the current subtree root page
 *
 */
int
xtGather(t)
btree_t *t;
{
        int32           rc = 0;
        xtpage_t        *p;
        uint64          bn;
        int16           index;
        btentry_t       *e;
        btstack_t       btstack;
        struct btsf     *parent;

        /* clear stack */
        BT_CLR(&btstack);

        /*
         * start with root
         *
         * root resides in the inode
         */
        bn = 0;
        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        if (rc)
                return rc;

        /* new root is NOT pointed by a new entry
        if (p->header.flag & NEW)
                allocate new page lock;
                write a NEWPAGE log;
         */

dopage:
        /*
         * first access of each page:
         */
        /* process entries backward from last index */
        index = p->header.nextindex - 1;

        if (p->header.flag & BT_LEAF) {
                /*
                 * first access of each leaf page
                 */
NOISE(0,("commit(leaf): bn:%d\n", bn));
                /* process leaf page entries backward */
                for ( ; index >= XTENTRYSTART; index--) {
                        e = &p->xad[index];
                        /*
                         * if newpage, log NEWPAGE.
                         *
                        if (e->flag & XAD_NEW) {
                                nfound =+ entry->length;
                                update current page lock for the entry;
                                newpage(entry);
                         *
                         * if moved, log move.
                         *
                        } else if (e->flag & XAD_MOVED) {
                                reset flag;
                                update current page lock for the entry;
                        }
                         */
                }

                /* unpin the leaf page */
                XT_PUTPAGE(bp);

                /*
                 * go back up to the parent page
                 */
getParent:
                /* restore parent entry for the current child page */
                if ((parent = BT_POP(&btstack)) == NULL)
                        /* current page must have been root */
                        return 0;

                if ((index = parent->index) == XTENTRYSTART) {
                        /*
                         * parent page scan completed
                         */
                        /* go back up to the parent page */
                        goto getParent;
                } else {
                        /*
                         * parent page has entries remaining
                         */
                        /* get back the parent page */
                        bn = parent->bn;
                        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
                        if (rc)
                                return EIO;

                        /* first subroot page which
                         * covers all new allocated blocks
                         * itself not new/modified.
                         * (if modified from split of descendent,
                         * go down path of split page)

                        if (nfound == nnew &&
                            !(p->header.flag & (NEW | MOD)))
                                exit scan;
                         */

                        /* process parent page entries backward */
                        index--;
                }
        } else {
                /*
                 * first access of each internal page
                 */
        }

        /*
         * internal page: go down to child page of current entry
         */
NOISE(0,("commit(internal): bn:%d(%d)\n", bn, index));

        /* save current parent entry for the child page */
        BT_PUSH(&btstack, bn, index);

        /* get current entry for the child page */
        e = &p->xad[index];

        /*
         * first access of each internal entry:
         */
        /*
         * if new entry, log btree_tnewentry.
         *
        if (e->flag & XAD_NEW)
                update parent page lock for the entry;
         */

        /* release parent page */
        XT_PUTPAGE(bp);

        /* get child page */
        bn = e->bn;
        XT_GETPAGE(ip, bn, bp, xtpage_t, PSIZE, p, rc);
        if (rc)
                return rc;

        /*
         * first access of each non-root page:
         */
        /*
         * if new, log btree_newpage.
         *
        if (p->header.flag & NEW)
                allocate new page lock;
                write a NEWPAGE log (next, prev);
         */

        /* process the child page */
        goto dopage;

out:
        return 0;
}
#endif  /* _JFS_WIP */
