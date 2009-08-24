/* $Id: jfs_extent.c,v 1.1 2000/04/21 10:58:08 ktk Exp $ */

static char *SCCSID = "@(#)1.5  7/30/98 14:14:21 src/jfs/ifs/jfs_extent.c, sysjfs, w45.fs32, 990417.1";
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
 *
 * Module: jfs_extent.c:
 */

#ifdef _JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <jfs_os2.h>
#endif /* _JFS_OS2 */
 
#include "jfs_types.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_dmap.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_xtree.h"
#include "jfs_txnmgr.h"
#include "jfs_debug.h"

/*
 * forward references
 */
static int32 extBalloc(inode_t *ip, int64 hint, int64 *nblocks, int64 *blkno);
static int32 extBrealloc(inode_t *ip, int64 blkno, int64 nblks,
			 int64 *newnblks, int64 *newblkno);
int32 extRecord(inode_t *ip, cbuf_t *cp);
int64 extRoundDown(int64 nb);

/*
 * external references
 */
int32 dbExtend(inode_t *ip, int64 blkno, int64 nblocks, int64 addnblocks);


#define DPD(a)          (printf("(a): %d\n",(a)))
#define DPC(a)          (printf("(a): %c\n",(a)))
#define DPL1(a)					\
{						\
	if ((a) >> 32)				\
		printf("(a): %x%08x  ",(a));	\
	else					\
		printf("(a): %x  ",(a) << 32);	\
}
#define DPL(a)					\
{						\
	if ((a) >> 32)				\
		printf("(a): %x%08x\n",(a));	\
	else					\
		printf("(a): %x\n",(a) << 32);	\
}
	
#define DPD1(a)         (printf("(a): %d  ",(a)))
#define DPX(a)          (printf("(a): %08x\n",(a)))
#define DPX1(a)         (printf("(a): %08x  ",(a)))
#define DPS(a)          (printf("%s\n",(a)))
#define DPE(a)          (printf("\nENTERING: %s\n",(a)))
#define DPE1(a)          (printf("\nENTERING: %s",(a)))
#define DPS1(a)         (printf("  %s  ",(a)))


/*
 * NAME:	extAlloc()
 *
 * FUNCTION:    allocate an extent for a specified page range within a
 *		file.
 *
 * PARAMETERS:
 *	ip	- the inode of the file.
 *	xlen	- requested extent length.
 *	pno	- the starting page number with the file.
 *	xp	- pointer to an xad.  on entry, xad describes an
 *		  extent that is used as an allocation hint if the
 *		  xaddr of the xad is non-zero.  on successful exit,
 *		  the xad describes the newly allocated extent.
 *	abnr	- boolean_t indicating whether the newly allocated extent
 *		  should be marked as allocated but not recorded.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 *      ENOSPC	- insufficient disk resources.
 */
int32
extAlloc(inode_t *ip,
	 int64 xlen,
	 int64 pno,
	 xad_t *xp,
	 boolean_t abnr)
{
	int64 nxlen, nxaddr, xoff, hint, xaddr = 0;
	int32 rc, nbperpage;
	uint8 xflag;

	/* validate extent length */
	if (xlen > MAXXLEN)
		xlen = MAXXLEN;

	/* get the number of blocks per page */
	nbperpage = ip->i_ipmnt->i_nbperpage;

	/* get the page's starting extent offset */
	xoff = pno << ip->i_ipmnt->i_l2nbperpage;

	/* check if an allocation hint was provided */
	if (hint = addressXAD(xp))
	{
		/* get the size of the extent described by the hint */
		nxlen = lengthXAD(xp);

		/* check if the hint is for the portion of the file
		 * immediately previous to the current allocation
		 * request and if hint extent has the same abnr
		 * value as the current request.  if so, we can
		 * extend the hint extent to include the current
		 * extent if we can allocate the blocks immediately
		 * following the hint extent.
		 */
		if (offsetXAD(xp) + nxlen == xoff &&
		    abnr == ((xp->flag & XAD_NOTRECORDED) ? TRUE : FALSE))
			xaddr = hint + nxlen;

		/* adjust the hint to the last block of the extent */
		hint += (nxlen-1);
	}

	/* allocate the disk blocks for the extent.  initially, extBalloc()
	 * will try to allocate disk blocks for the requested size (xlen). 
	 * if this fails (xlen contigious free blocks not avaliable), it'll
	 * try to allocate a smaller number of blocks (producing a smaller
	 * extent), with this smaller number of blocks consisting of the
	 * requested number of blocks rounded down to the next smaller
	 * power of 2 number (i.e. 16 -> 8).  it'll continue to round down
	 * and retry the allocation until the number of blocks to allocate
	 * is smaller than the number of blocks per page.
	 */
	nxlen = xlen;
	if (rc = extBalloc(ip, hint ? hint : INOHINT(ip), &nxlen, &nxaddr))
	{
		return(rc);
	}

	/* determine the value of the extent flag */
	xflag = (abnr == TRUE) ? XAD_NOTRECORDED : 0;

	/* if we can extend the hint extent to cover the current request, 
	 * extend it.  otherwise, insert a new extent to
	 * cover the current request.
	 */
	if (xaddr && xaddr == nxaddr)
		rc = xtExtend(0,ip,xoff,(int32)nxlen,0);
	else
  		rc = xtInsert(0,ip,xflag,xoff,(int32)nxlen,&nxaddr,0);

	/* if the extend or insert failed, 
	 * free the newly allocated blocks and return the error.
	 */
	if (rc)
	{
		dbFree(ip, nxaddr, nxlen);
		return(rc);
	}

	/* update the number of blocks allocated to the file */
	ip->i_nblocks += nxlen;

	/* set the results of the extent allocation */
	XADaddress(xp,nxaddr);
	XADlength(xp,nxlen);
	XADoffset(xp,xoff);
	xp->flag = xflag;

	return(0);
}


/*
 * NAME:        extRealloc()
 *
 * FUNCTION:    extend the allocation of a file extent containing a
 *		partial back last page.
 *
 * PARAMETERS:
 *	ip	- the inode of the file.
 *	cp	- cbuf for the partial backed last page.
 *	xlen	- request size of the resulting extent.
 *	xp	- pointer to an xad. on successful exit, the xad
 *		  describes the newly allocated extent.
 *	abnr	- boolean_t indicating whether the newly allocated extent
 *		  should be marked as allocated but not recorded.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 *      ENOSPC	- insufficient disk resources.
 */
int32
extRealloc(inode_t *ip,
	   cbuf_t  *cp,
	   int64 nxlen,
	   xad_t *xp,
	   boolean_t abnr)
{
	int64 xaddr, xlen, nxaddr, delta, xoff, nblks;
	int64 ntail, nextend, ninsert;
	int32 rc, nbperpage = ip->i_ipmnt->i_nbperpage;
        uint8 xflag;

	/* validate extent length */
	if (nxlen > MAXXLEN)
		nxlen = MAXXLEN;

	/* get the extend (partial) page's disk block address and
	 * number of blocks.
	 */
	xaddr = addressPXD(&cp->cm_pxd);
	xlen = lengthPXD(&cp->cm_pxd);

	/* get the extent offset of the cbuf */
	xoff = cp->cm_blkno << ip->i_ipmnt->i_l2nbperpage;

	/* if the extend page is abnr and if the request is for
	 * the extent to be allocated and recorded, 
	 * make the page allocated and recorded.
	 */
	if (cp->cm_abnr == TRUE && abnr == FALSE)
	{
		if (rc = extRecord(ip,cp))
			return(rc);
	}

	/* try to allocated the request number of blocks for the
	 * extent.  dbRealloc() first tries to satisfy the request
	 * by extending the allocation in place. otherwise, it will
 	 * try to allocate a new set of blocks large enough for the
	 * request.  in satisfying a request, dbReAlloc() may allocate
	 * less than what was request but will always allocate enough
	 * space as to satisfy the extend page.
	 */
	if (rc = extBrealloc(ip, xaddr, xlen, &nxlen, &nxaddr))
		return(rc);

	delta = nxlen - xlen;

	/* check if the extend page is not abnr but the request is abnr
	 * and the allocated disk space is for more than one page.  if this
	 * is the case, there is a miss match of abnr between the extend page
	 * and the one or more pages following the extend page.  as a result,
	 * two extents will have to be manipulated. the first will be that
	 * of the extent of the extend page and will be manipulated thru
	 * an xtExtend() or an xtTailgate(), depending upon whether the
	 * disk allocation occurred as an inplace extension.  the second
	 * extent will be manipulated (created) through an xtInsert() and
	 * will be for the pages following the extend page.
	 */
	if (abnr == TRUE && cp->cm_abnr == FALSE && nxlen > nbperpage)
	{
		ntail = nbperpage;
		nextend = ntail - xlen;
		ninsert = nxlen - nbperpage;

		xflag = XAD_NOTRECORDED;
	}
	else
	{
		ntail = nxlen;
		nextend = delta;
		ninsert = 0;

		xflag = (cp->cm_abnr == TRUE) ? XAD_NOTRECORDED : 0;
	}

	/* if we were able to extend the disk allocation in place,
	 * extend the extent.  otherwise, move the extent to a
	 * new disk location.
	 */
        if (xaddr == nxaddr)
	{
		/* extend the extent */
		if (rc = xtExtend(0,ip,xoff+xlen,(int32)nextend,0))
		{
			dbFree(ip, xaddr+xlen, delta);
			return(rc);
		}
	}
	else
	{
		/*
		 * move the extent to a new location:
		 *
		 * xtTailgate() accounts for relocated tail extent;
		 */
		if (rc = xtTailgate(0,ip,xoff,(int32)ntail,nxaddr,0))
		{
			dbFree(ip, nxaddr, nxlen);
			return(rc);
		}
	}


	/* check if we need to also insert a new extent */
	if (ninsert)
	{
		/* perform the insert.  if it fails, free the blocks
		 * to be inserted and make it appear that we only did
		 * the xtExtend() or xtTailgate() above.
		 */
		xaddr = nxaddr + ntail;
		if (xtInsert(0, ip, xflag, xoff+ntail, (int32)ninsert, &xaddr, 0))
		{
			dbFree(ip, xaddr, (int64)ninsert);
			delta = nextend;
			nxlen = ntail;
			xflag = 0;
		}
	}

	/* update the inode with the number of blocks allocated */
	ip->i_nblocks += delta;

	/* set the return results */
	XADaddress(xp,nxaddr);
	XADlength(xp,nxlen);
	XADoffset(xp,xoff);
	xp->flag = xflag;

	return(0);
}


/*
 * NAME:        extHint()
 *
 * FUNCTION:    produce an extent allocation hint for a file offset.
 *
 * PARAMETERS:
 *	ip	- the inode of the file.
 *	offset  - file offset for which the hint is needed.
 *	xp	- pointer to the xad that is to be filled in with
 *		  the hint.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 */
int32
extHint(inode_t *ip,
	int64 offset,
	xad_t *xp)
{
        xadlist_t xadl;
        lxdlist_t lxdl;
	lxd_t lxd;
        int64 prev;
        int32 rc, nbperpage = ip->i_ipmnt->i_nbperpage;

	/* init the hint as "no hint provided" */
	XADaddress(xp,0);

	/* determine the starting extent offset of the page previous
	 * to the page containing the offset.
	 */
	prev = ((offset & ~CM_OFFSET) >> ip->i_ipmnt->i_l2bsize) - nbperpage;

	/* if the offsets in the first page of the file,
	 * no hint provided.
	 */
	if (prev < 0)
		return(0);

	/* prepare to lookup the previous page's extent info */
	lxdl.maxnlxd = 1;
	lxdl.nlxd = 1;
	lxdl.lxd = &lxd;
	LXDoffset(&lxd,prev)
	LXDlength(&lxd,nbperpage);

	xadl.maxnxad = 1;
	xadl.nxad = 0;
	xadl.xad = xp;

	/* perform the lookup */
	if (rc = xtLookupList(ip, &lxdl, &xadl, 0))
		return(rc);

	/* check if not extent exists for the previous page.  
	 * this is possible for sparse files.
	 */
        if (xadl.nxad == 0)
        {
                assert(ISSPARSE(ip));
                return(0);
        }

	/* only preserve the abnr flag within the xad flags
	 * of the returned hint.
	 */
	xp->flag &= XAD_NOTRECORDED;

	assert(xadl.nxad == 1);
	assert(lengthXAD(xp) == nbperpage);

	return(0);
}


/*
 * NAME:        extRecord()
 *
 * FUNCTION:    change a page with a file from not recorded to recorded.
 *
 * PARAMETERS:
 *	ip	- inode of the file.
 *	cp	- cbuf of the file page.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 *      ENOSPC	- insufficient disk resources.
 */
int32
extRecord(inode_t *ip,
	  cbuf_t  *cp)
{
	int32 rc;
	xad_t xad;

	/* cbuf should be marked as abnr */
	assert(cp->cm_abnr);

	/* construct the xad in prep for updating the extent */
	XADaddress(&xad,addressPXD(&cp->cm_pxd));
	XADlength(&xad,lengthPXD(&cp->cm_pxd));
	XADoffset(&xad,cp->cm_blkno << ip->i_ipmnt->i_l2nbperpage);
	xad.flag = 0;

	/* update the extent */
	if (rc = xtUpdate(0,ip,&xad))
		return(rc);

	/* no longer abnr */
	cp->cm_abnr = FALSE;

	/* mark the cbuf as modified */
	cp->cm_modified = TRUE;

	return(0);
}


/*
 * NAME:        extFill()
 *
 * FUNCTION:    allocate disk space for a file page that represents
 *		a file hole.
 *
 * PARAMETERS:
 *	ip	- the inode of the file.
 *	cp	- cbuf of the file page represent the hole.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 *      ENOSPC	- insufficient disk resources.
 */
int32
extFill(inode_t *ip,
	cbuf_t  *cp)
{
	int32 rc, nbperpage = ip->i_ipmnt->i_nbperpage;
	xad_t xad;

	/* cbuf should indicate a hole and file must be sparse */
	assert(cp->cm_hole == TRUE);
	assert(ISSPARSE(ip));

	/* initialize the extent allocation hint */
	XADaddress(&xad,0);

	/* allocate an extent to fill the hole */
	if (rc = extAlloc(ip,nbperpage,cp->cm_blkno,&xad,FALSE))
		return(rc);

	assert(lengthPXD(&xad) == nbperpage);

	/* update the cbuf with the newly allocated extent info */
	PXDaddress(&cp->cm_pxd,addressXAD(&xad));
	PXDlength(&cp->cm_pxd,nbperpage);

	/* not longer a hole */
	cp->cm_hole = FALSE;

	/* mark cbuf as modified */
	cp->cm_modified = TRUE;

	return(0);
}


/*
 * NAME:	extBalloc()
 *
 * FUNCTION:    allocate disk blocks to form an extent.
 *
 *		initially, we will try to allocate disk blocks for the
 *		requested size (nblocks).  if this fails (nblocks 
 *		contigious free blocks not avaliable), we'll try to allocate
 *		a smaller number of blocks (producing a smaller extent), with
 *		this smaller number of blocks consisting of the requested
 *		number of blocks rounded down to the next smaller power of 2
 *		number (i.e. 16 -> 8).  we'll continue to round down and
 *		retry the allocation until the number of blocks to allocate
 *		is smaller than the number of blocks per page.
 *		
 * PARAMETERS:
 *	ip	 - the inode of the file.
 *	hint	 - disk block number to be used as an allocation hint.
 *	*nblocks - pointer to an int64 value.  on entry, this value specifies
 *		   the desired number of block to be allocated. on successful
 *		   exit, this value is set to the number of blocks actually
 *		   allocated.
 *	blkno	 - pointer to a block address that is filled in on successful
 *		   return with the starting block number of the newly 
 *		   allocated block range.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 *      ENOSPC	- insufficient disk resources.
 */
static int32
extBalloc(inode_t *ip,
       	  int64   hint,
          int64   *nblocks,
       	  int64   *blkno)
{
	int64 nb, nblks, daddr, max;
	int32 rc, nbperpage = ip->i_ipmnt->i_nbperpage;
        bmap_t *mp = ip->i_ipmnt->i_ipbmap->i_bmap;

	/* get the number of blocks to initially attempt to allocate.
	 * we'll first try the number of blocks requested unless this
	 * number is greater than the maximum number of contigious free
	 * blocks in the map. in that case, we'll start off with the 
	 * maximum free.
	 */
	max = (int64)1 << mp->db_maxfreebud;
	if (*nblocks >= max && *nblocks > nbperpage)
		nb = nblks = (max > nbperpage) ? max : nbperpage;
	else
		nb = nblks = *nblocks;

	/* try to allocate blocks */
	while (rc = dbAlloc(ip, hint, nb, &daddr))
	{
		/* if something other than an out of space error,
		 * stop and return this error.
		 */
		if (rc != ENOSPC)
			return(rc);

		/* decrease the allocation request size */
		nb = MIN(nblks,extRoundDown(nb));

		/* give up if we cannot cover a page */
		if (nb < nbperpage)
			return(rc);
	}

	*nblocks = nb;
	*blkno = daddr;

	return(0);
}


/*
 * NAME:	extBrealloc()
 *
 * FUNCTION:    attempt to extend an extent's allocation.
 *
 *		initially, we will try to extend the extent's allocation
 *		in place.  if this fails, we'll try to move the extent
 *		to a new set of blocks. if moving the extent, we initially
 *		will try to allocate disk blocks for the requested size
 *		(nnew).  if this fails 	(nnew contigious free blocks not
 *		avaliable), we'll try  to allocate a smaller number of
 *		blocks (producing a smaller extent), with this smaller
 *		number of blocks consisting of the requested number of
 *		blocks rounded down to the next smaller power of 2
 *		number (i.e. 16 -> 8).  we'll continue to round down and
 *		retry the allocation until the number of blocks to allocate
 *		is smaller than the number of blocks per page.
 *		
 * PARAMETERS:
 *	ip	 - the inode of the file.
 *	blkno    - starting block number of the extents current allocation.
 *	nblks    - number of blocks within the extents current allocation.
 *	newnblks - pointer to a int64 value.  on entry, this value is the
 *		   the new desired extent size (number of blocks).  on
 *		   successful exit, this value is set to the extent's actual
 *		   new size (new number of blocks).
 *	newblkno - the starting block number of the extents new allocation.
 *
 * RETURN VALUES:
 *      0       - success
 *      EIO	- i/o error.
 *      ENOSPC	- insufficient disk resources.
 */
static int32
extBrealloc(inode_t *ip,
       	  int64   blkno,
          int64   nblks,
       	  int64   *newnblks,
       	  int64   *newblkno)
{
	int32 rc;

	/* try to extend in place */
	if ((rc = dbExtend(ip,blkno,nblks,*newnblks-nblks)) == 0)
	{
		*newblkno = blkno;
		return(0);
	}
	else
	{
		if (rc != ENOSPC)
			return(rc);
	}

	/* in place extension not possible.  
	 * try to move the extent to a new set of blocks.
	 */
	return(extBalloc(ip, blkno, newnblks, newblkno));
}


/*
 * NAME:        extRoundDown()
 *
 * FUNCTION:    round down a specified number of blocks to the next
 *		smallest power of 2 number.
 *
 * PARAMETERS:
 *	nb	- the inode of the file.
 *
 * RETURN VALUES:
 *      next smallest power of 2 number.
 */
static int64
extRoundDown(int64 nb)
{
        int i;
        uint64 m, k;

        for (i = 0, m = (uint64) 1 << 63; i < 64; i++, m >>= 1)
        {
                if (m & nb)
                        break;
        }

        i = 63 - i;
        k = (uint64)1 << i;
	k = ((k-1) & nb) ? k : k >> 1;

	return(k);
}
