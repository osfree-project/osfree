/* $Id: jfs_dio.c,v 1.1.1.1 2003/05/21 13:36:36 pasha Exp $ */

static char *SCCSID = "@(#)1.22  9/13/99 14:58:53 src/jfs/ifs/jfs_dio.c, sysjfs, w45.fs32, fixbld";
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
*/
/*
 *	jfs_dio.c	- Manager for direct I/O
 */

/*
 * Change History :
 *
 */

#ifdef	_JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#endif	/* _JFS_OS2 */
#include "mmph.h"

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_xtree.h"
#include "jfs_dmap.h"		/* for function prototype */
#include "jfs_io.h"
#include "jfs_dio.h"
#include "jfs_debug.h"

/*
 *	dio manager
 */
struct diomgr	diomgr;

#ifdef	_JFS_OS2
/* use the jfs definition for page size */
#define PAGESIZE PSIZE
#endif	/* _JFS_OS2 */

iobuf_t *dioGetBuf(void);
static int32 dioStartIO(dio_t	*dp,
			int64	paddr,
			int64   plen,
			int64	offset,
			caddr_t	dest,
			int32	flag);
static void dioSetError(dio_t *dp,
			int64 offset,
			int32 error);
static void dioFreeBuf(iobuf_t *bp);

#ifdef _JFS_OS2
/*
 *	coalesce control
 *
 * max_dio: set to maximum number of pages to coalesce in one direct I/O request
 * max_plen: set to maximum number of bytes for one direct I/O request.
 */
#define MAX_DIO	16
int32	max_dio = MAX_DIO;
int32	max_plen = (MAX_DIO) << L2PSIZE;
int32	max_plen16 = (MAX_RAPAGE) << L2PSIZE; /* CallStrat2 limit */
#endif	/* _JFS_OS2 */


/*
 * NAME: dioInit
 *
 * FUNCTION: Initialize the direct I/O manager.
 *
 * PARAMETERS:
 *	nbufs	- number of pinned I/O buffers to allocate
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dioInit(int32 nbufs)
{
	iobuf_t *bp, *next;
	int32 n;
	int32	size;
#ifdef _JFS_OS2
	Req_List_Header	*rlhp;
	Req_Header	*rhp;
#endif	/* _JFS_OS2 */
	
#ifdef _JFS_OS2
	size = sizeof(iobuf_t) + max_dio * sizeof(SG_Descriptor);
	if ((diomgr.freebufs = (iobuf_t *)xmalloc(nbufs * size, 0,
			pinned_heap|XMALLOC_CONTIG|XMALLOC_HIGH)) == NULL)
			return(ENOMEM);
#endif	/* _JFS_OS2 */

	bzero(diomgr.freebufs, nbufs * size);

	/* put them on the free list. */
	for (n = nbufs, bp = diomgr.freebufs; n - 1; bp = next, n--)
	{
		next = (iobuf_t *)((char *)bp + size);
		bp->b_next =  next;
	}
	bp->b_next = NULL;

#ifdef _JFS_OS2
	/* Initialize device buffer fixed attributes */
	for( bp = diomgr.freebufs, n = 0; n < nbufs;
		n++, bp = (iobuf_t *)((char *)bp + size))
	{
		bp->b_iodone = dioIODone;

		/*
		 * fill in request list header
		 */
		rlhp = &bp->bio_rlh;
		rlhp->Count = 1;
		rlhp->Notify_Address = (void *)jfsIODone;
		rlhp->Request_Control = RLH_Single_Req |
					RLH_Notify_Err | RLH_Notify_Done;
		/* rlhp->Block_Dev_Unit; */
		/* rlhp->Lst_Status */

		/*
		 * fill in request header
		 */
		rhp = &bp->bio_rh.RqHdr;
		rhp->Length = RH_LAST_REQ;
		rhp->Old_Command = PB_REQ_LIST;
		/* rhp->Command_Code */
		rhp->Head_Offset = sizeof(Req_List_Header);
		rhp->Req_Control = 0;
		rhp->Priority = PRIO_FOREGROUND_USER;
		/* rhp->Status */
		/* rhp->Error_Code */
		rhp->Hint_Pointer = -1;	/* no hint */
	}
#endif	/* _JFS_OS2 */

	diomgr.diofree = NULL;
	diomgr.iovfree = NULL;
	event_init(&diomgr.bufwait);

	/* allocate/initialize the dio manager lock */
	DIO_LOCK_ALLOC();
	DIO_LOCK_INIT();

	/* allocate/initialize the dio i/o lock */
	DIO_IOLOCK_ALLOC();
	DIO_IOLOCK_INIT();

	return(0);
}


/*
 * NAME: dioRead
 *
 * FUNCTION: Performs direct I/O read of part of file specified
 *
 * PARAMETERS:
 *	dp	- directio structure
 *	paddr	- physical block address to read
 *	plen	- number of bytes to read
 *	offset	- byte offset into file
 *	dest	- starting address of user data
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dioRead(dio_t	*dp,
	int64	paddr,
	int64   plen,
	int64	offset,
	caddr_t	dest)
{
	int32 rc;

   MMPHPredioRead();       /* MMPH Performance Hook */

	rc = dioStartIO(dp,paddr,plen,offset,dest,DIO_READ);

   MMPHPostdioRead();      /* MMPH Performance Hook */
	return(rc);
}


/*
 * NAME: dioWrite
 *
 * FUNCTION: Performs direct I/O write of part of file specified
 *
 * PARAMETERS:
 *	dp	- directio structure
 *	paddr	- physical block address to write
 *	plen	- number of bytes to write
 *	offset	- byte offset into file
 *	src	- starting address of user data
 *	abnr	- Indicates if piece of file is allocated but not recorded
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dioWrite(dio_t	*dp,
	int64	paddr,
	int64   plen,
	int64	offset,
	caddr_t	src,
	boolean_t abnr)
{
	int32 rc, flag;

   MMPHPredioWrite();      /* MMPH Performance Hook */

	flag = (abnr == TRUE) ? (DIO_WRITE|DIO_ABNR) : DIO_WRITE;

	rc = dioStartIO(dp,paddr,plen,offset,src,flag);

   MMPHPostdioWrite();     /* MMPH Performance Hook */
	return(rc);
}


/*
 * NAME: dioStart
 *
 * FUNCTION: Initializes directio structures for specified inode
 *
 * PARAMETERS:
 *	ip	- inode
 *	dpp	- Pointer to directio structure, set on return
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dioStart(inode_t *ip,
	 dio_t **dpp)
{
	dio_t *dp;
	int32 n;

	DIO_LOCK();

	if (diomgr.diofree == NULL)
	{
		n = PAGESIZE / sizeof(dio_t);
		if ((diomgr.diofree = (dio_t *)xmalloc(n * sizeof(dio_t),
						0, pinned_heap)) == NULL)
		{
			DIO_UNLOCK();
			return(ENOMEM);
		}

		/* put them on the free list.
		 */
		for (dp = diomgr.diofree; n - 1; dp++, n--)
			dp->dio_diofree =  dp + 1;
		dp->dio_diofree = NULL;
	}

	dp = diomgr.diofree;
	diomgr.diofree = dp->dio_diofree;

	DIO_UNLOCK();

	bzero(dp,sizeof(dio_t));
        dp->dio_ip = ip;
        dp->dio_freeiov = &dp->dio_firstiov;
	event_init(&dp->dio_iowait);

	/* set the return value.
	 */
	*dpp = dp;

	return(0);
}


/*
 * NAME: dioEnd
 *
 * FUNCTION: Waits for queued direct I/Os to complete;
 *	cleans up directio structures
 *
 * PARAMETERS:
 *	dp	- directio structure
 *	erroff	- on error, filled in with byte offset of the first failing
 *		  block
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
dioEnd(dio_t *dp,
       int64 *erroff)
{
	int32 rc, error, ipri, l2bsize, l2bfactor;
	diovec_t *dvp, *next;
	dioelm_t *dep;
	inode_t *ip;
	xad_t xad;
	int32	index;

	ipri = DIO_IOLOCK();
	
	if (dp->dio_iocnt != 0)
	{
		DIO_SLEEP(&dp->dio_iowait, T_NORELOCK);
		DIO_NOLOCK(ipri);
	}
	else
		DIO_IOUNLOCK(ipri);


#ifdef _JFS_OS2
	/* Need to unpin the user pages for the direct I/O.  On AIX we are able
	 * to do this in the dioIODone() function.  This is not allowed during
	 * interrupt time on OS/2 so we have to wait to do this until here
	 */
	dvp = &(dp->dio_firstiov);
	for( index = 0; index < dvp->dv_nios; index++ ) {
	    if ((dvp->dv_ios[index].de_flag & DIO_NO_UNLOCK) == 0)
		if (rc = KernVMUnlock( &dvp->dv_ios[index].de_lockh ))
		{
			jFYI(1,("KernVMUnlock failed: rc = %d\n", rc));
			brkpoint();
		}
	}

	dvp = dp->dio_iovlist;
	while( dvp != NULL ) {
		for( index = 0; index < dvp->dv_nios; index++ ) {
		    if ((dvp->dv_ios[index].de_flag & DIO_NO_UNLOCK) == 0)
			if (rc = KernVMUnlock( &dvp->dv_ios[index].de_lockh ))
			{
				jFYI(1,("KernVMUnlock failed: rc = %d\n", rc));
				brkpoint();
			}
		}
		dvp = dvp->dv_next;
	}
#endif	/* _JFS_OS2 */

	if (dp->dio_abnrlist != NULL)
	{
		ip = dp->dio_ip;
		l2bfactor = ip->i_ipmnt->i_l2bfactor;
		l2bsize = ip->i_ipmnt->i_l2bsize;

		for (dep = dp->dio_abnrlist->de_abnrnext; ;
						dep = dep->de_abnrnext)
		{
			if (dp->dio_error && dp->dio_erroff <= dep->de_offset)
				break;

			/* Convert physical block address to aggregate block
			 * address;
			 * Convert byte counts to aggregate block counts
			 */
        		XADaddress(&xad,dep->de_paddr >> l2bfactor);
        		XADlength(&xad,dep->de_plen >> l2bsize);
        		XADoffset(&xad,dep->de_offset >> l2bsize);
        		xad.flag = 0;

			if (rc = xtUpdate(0,ip,&xad))
			{
				dp->dio_error = rc;
				dp->dio_erroff = dep->de_offset;
				break;
			}

			if (dep == dp->dio_abnrlist)
				break;
		}
	}

	if ((error = dp->dio_error) != 0)
		*erroff = dp->dio_erroff;

	DIO_LOCK();

	/* Put dynamically allocated diovecs for this dio onto the freelist */
	for (dvp = dp->dio_iovlist; dvp != NULL; dvp = next)
	{
		next = dvp->dv_next;
		dvp->dv_next = diomgr.iovfree;
		diomgr.iovfree = dvp;
	}

	dp->dio_diofree = diomgr.diofree;
	diomgr.diofree = dp;

	DIO_UNLOCK();

	return(error);
}


/*
 * NAME: dioStartIO
 *
 * FUNCTION: Do work for read/write direct
 *
 * PARAMETERS:
 *	dp	- directio structure
 *	paddr	- physical block address to read/write
 *	plen	- number of bytes to read/write
 *	offset	- byte offset into file
 *	dest	- starting address of user data
 *	flag	- DIO_READ: read; DIO_WRITE: write;
 *		  DIO_ABNR: allocated but not recorded section for write
 *
 * RETURNS: 0 for success; Other indicates failure
 */
static int32
dioStartIO(dio_t	*dp,
	   int64	paddr,
	   int64   	plen,
	   int64	offset,
	   caddr_t	dest,
	   int32	flag)
{
	int32 rc, n, ipri, l2bsize;
	inode_t *ip;
	diovec_t *dvp, *ndvp;
	dioelm_t *dep;
	iobuf_t *bp;
	inode_t	*ipmnt;
	int32	page_offset;
	Req_List_Header	*rlhp;	/* pointer to list header */
	Req_Header	*rhp;	/* pointer to header */
	PB_Read_Write	*rwp;	/* pointer to request */
	KernPageList_t	*sgp;	/* S/G descriptor */
	ULONG		pgcnt;
	int64		curlen;

	ipmnt = dp->dio_ip->i_ipmnt;

	assert(plen != 0 && (plen & (ipmnt->i_pbsize-1)) == 0);

	/* Since we have a fixed number of Scatter/Gather lists for each I/O
	 * buffer on OS/2, we must split the I/O request into at most that
	 * number of pages.  This is handled at this level instead of by the
	 * caller since it is only for OS/2, and it must happen for both read
	 * and write.  This is the common place for both of those.
	 */
	do {
		/* Determine curlen value.  Should be at most max_dio number of
		 * pages of data to be written.  If user buffer is not page
		 * aligned, allow for one extra scatter-gather list entry.
		 */
		if (ipmnt->i_Strategy3)
			curlen = MIN(plen,
			    ((int32)dest & CM_OFFSET) ? max_plen-PAGESIZE :
							max_plen);
		else
			curlen = MIN(plen,
			    ((int32)dest & CM_OFFSET) ? max_plen16-PAGESIZE :
							max_plen16);

		dvp = dp->dio_freeiov;
		if (dvp->dv_nios == DIO_MAXIOS)
		{
			DIO_LOCK();

			if (diomgr.iovfree == NULL)
			{
				n = PAGESIZE / sizeof(diovec_t);
				if ((diomgr.iovfree = (diovec_t *) xmalloc(
					n * sizeof(diovec_t),0,pinned_heap)) ==
						NULL)
				{
					ipri = DIO_IOLOCK();
					dioSetError(dp,offset,ENOMEM);
					DIO_IOUNLOCK(ipri);
					return(ENOMEM);
				}

				for (ndvp = diomgr.iovfree; n - 1; ndvp++, n--)
					ndvp->dv_next =  ndvp + 1;
				ndvp->dv_next = NULL;
			}

			ndvp = diomgr.iovfree;
			diomgr.iovfree = ndvp->dv_next;

			DIO_UNLOCK();

			/* Add ndvp to the list of dynamically allocated
			 * diovec's for the dio.  We started with one diovec
			 * that was statically allocated as part of the dio
			 * struct and is NOT placed on this list.  The list is
			 * simply used by dioEnd() to free the diovecs that were
			 * needed in addition to the diovec in the dio struct.
			 */
			ndvp->dv_next = dp->dio_iovlist;
			dp->dio_iovlist = ndvp;

			dp->dio_freeiov = ndvp;
			ndvp->dv_nios = 0;

			dvp = ndvp;
		}

		dep = &dvp->dv_ios[dvp->dv_nios];
		dvp->dv_nios += 1;

		dep->de_offset = offset;
		dep->de_plen = curlen;
		dep->de_paddr = paddr;
		dep->de_flag = 0;

		if (flag & DIO_ABNR)
		{
			assert(flag & DIO_WRITE);
			assert((offset & CM_OFFSET) == 0);

			if (curlen & (CM_BSIZE-1))
			{
				ip = dp->dio_ip;

				if (CM_OFFTOCBLK(offset+curlen-1) !=
						CM_OFFTOCBLK(ip->i_size-1))
					assert(0);

				l2bsize = ipmnt->i_l2bsize;

				if ((curlen & (CM_BSIZE-1)) !=
					(CM_BTOBLKS(ip->i_size,ipmnt->i_bsize,l2bsize)
						<< l2bsize))
					assert(0);
			}

			if (dp->dio_abnrlist == NULL)
			{
				dep->de_abnrnext = dep;
				dp->dio_abnrlist = dep;
			}
			else
			{
				dep->de_abnrnext =
					dp->dio_abnrlist->de_abnrnext;
				dp->dio_abnrlist->de_abnrnext = dep;
				dp->dio_abnrlist = dep;
			}
		}

		bp = dioGetBuf();

		bp->b_flags = (flag & DIO_READ) ? B_READ : 0;
		bp->b_next = NULL;
		bp->b_jfsbp = (iobuf_t *)dep;
		dep->de_dp = dp;
		bp->b_iodone = dioIODone;

		/* fill in request list header */
		rlhp = &bp->bio_rlh;
		/* rlhp->Count = 1; */
		/* rlhp->Notify_Address = (void *)jfsIODone; */
		/* rlhp->Request_Control =
			RLH_Single_Req | RLH_Notify_Err | RLH_Notify_Done; */
		rlhp->Block_Dev_Unit = dp->dio_ip->i_dev;
		rlhp->Lst_Status = 0;

		/* fill in request header */
		rhp = &bp->bio_rh.RqHdr;
		/* rhp->Length = RH_LAST_REQ; */
		/* rhp->Old_Command = PB_REQ_LIST; */
		rhp->Command_Code = (flag & DIO_READ) ? PB_READ_X : PB_WRITE_X;
		/* rhp->Head_Offset = sizeof(Req_List_Header); */
		/* rhp->Req_Control = 0; */
		/* rhp->Priority = PRIO_FOREGROUND_USER; */
		rhp->Status = 0;
		rhp->Error_Code = 0;
		/* rhp->Hint_Pointer = -1; */

		/* fill in r/w request */
		rwp = &bp->bio_rh;

		/* fill in extent and page frame of r/w request */
		rwp->Start_Block = paddr;
		rwp->Block_Count = curlen >> ipmnt->i_l2pbsize;

		/* update S/G descriptor */
		sgp = (KernPageList_t *)bp->bio_sg;
retry_lock:
		/* Lock user buffer area */
		if (rc = KernVMLock(VMDHL_WRITE|VMDHL_LONG, dest, curlen,
			&dep->de_lockh, sgp, &pgcnt))
		{
			/* If ERROR_UVIRTLOCK, we can't lock the page, but we
			 * can assume the page is fixed, so it is safe to send
			 * to the device driver.  We can only do this, though,
			 * if the request was for a single page.  If it was
			 * more, let's repeat for the first page of the
			 * request.
			 */
			if (rc == ERROR_UVIRTLOCK)
			{
				jFYI(1,("dioStartIO: ERROR_UVIRTLOCK detected.\n"));

				/* Offset within memory page */
				page_offset = (int32)dest & CM_OFFSET;

				/* Is request within single page?
				 */
				if ((page_offset + curlen)
				    <= PAGESIZE)
				{
					rc = KernLinToPageList(dest, curlen,
							       sgp, &pgcnt);
					assert (rc == 0);
					dep->de_flag |= DIO_NO_UNLOCK;
				}
				else if (curlen > PAGESIZE)
				{
					curlen = PAGESIZE;
					dep->de_plen = curlen;
					rwp->Block_Count =
						curlen >> ipmnt->i_l2pbsize;
					goto retry_lock;
				}
				else
				{
					/* We're down to one filesystem page,
					 * but the user buffer spans memory
					 * pages.  ERROR_UVIRTLOCK indicates
					 * that one of these pages is fixed.
					 * Try to both the first and second
					 * parts, if one succeeds the other
					 * must be UVIRT.
					 */
					ULONG	curlen2 = page_offset + curlen -
							  PAGESIZE;
					VOID	*dest2 = (VOID *)((int32)dest +
								  PAGESIZE -
								  page_offset);
					ULONG	pgcnt2;
					KernPageList_t *pl;

					rc = KernVMLock(VMDHL_WRITE|VMDHL_LONG,
							dest,
							PAGESIZE-page_offset,
							&dep->de_lockh, sgp,
							&pgcnt);
					if (rc == 0)
					{
						/* First part succeeded,
						 * second must be UVIRT
						 */
						pl = sgp+pgcnt;

						rc = KernLinToPageList(
							dest2, curlen2,
							pl, &pgcnt2);
						assert (rc == 0);
						pgcnt += pgcnt2;
					}
					else if (rc == ERROR_UVIRTLOCK)
					{
						/* First part UVIRT, try
						 * second.
						 */

						rc = KernLinToPageList(
							dest,
							PAGESIZE-page_offset,
							sgp, &pgcnt);
						assert(rc == 0);
						pl = sgp + pgcnt;
						rc = KernVMLock(VMDHL_WRITE|									VMDHL_LONG,
								dest2, curlen2,
								&dep->de_lockh,
								pl, &pgcnt2);
						if (rc == 0)
						{
							pgcnt += pgcnt2;
						}
						else if (rc == ERROR_UVIRTLOCK)
						{
							rc = KernLinToPageList(
							  dest2, curlen2,
							  pl, &pgcnt2);
							assert (rc == 0);
							pgcnt += pgcnt2;
							dep->de_flag |=
								DIO_NO_UNLOCK;
						}
						else
							goto lock_failed;
					}
					else
						goto lock_failed;
				}
			}
			else
			{
lock_failed:
				dvp->dv_nios -= 1;

				ipri = DIO_IOLOCK();

				dioFreeBuf(bp);
				dioSetError(dp,offset,rc);
				DIO_IOUNLOCK(ipri);
				jFYI(1,("KernVMLock failed: rc = %d\n", rc));
				return(rc);
			}
		}

		assert (pgcnt <= MAX_DIO);
		rwp->SG_Desc_Count = pgcnt;
		rwp->Blocks_Xferred = 0;
		rwp->RW_Flags = 0;

		ipri = DIO_IOLOCK();

		dp->dio_iocnt++;

		DIO_IOUNLOCK(ipri);

		/* block if hard quiesce */
		if (ipmnt->i_cachedev)
			IS_QUIESCE(ipmnt->i_cachedev);

      MMPHdioStartIO();       /* MMPH Performance Hook */

		if (ipmnt->i_Strategy3)
			CallStrat3(ipmnt->i_Strategy3,
				   &bp->bio_rlh);
		else
			CallStrat2(ipmnt->i_Strategy2,
				   &bp->bio_rlh);

		/* Update read/write information locally to determine if we had
		 * to split the request into multiple device driver calls.  If
		 * so, we will loop back up and do another one.
		 */
		plen -= curlen;
		paddr += (curlen >> ipmnt->i_l2pbsize);
		offset += curlen;
		dest += curlen;
	} while( plen > 0 );

	return 0;
}


/*
 * NAME: dioIODone
 *
 * FUNCTION: Perform I/O done processing for a direct I/O request.
 *
 * PARAMETERS:
 *	bp	- Pointer to buffer header for completed I/O
 *
 * RETURNS: 0 for success; Other indicates failure
 */
void
dioIODone(iobuf_t *bp)
{
	int32 rc, error, ipri;
	cmdev_t *cdp;
	dioelm_t *dep;
	dio_t *dp;

	dep = (dioelm_t *) bp->b_jfsbp;
	dp = dep->de_dp;

	/* If hard quiesce is in progress, and this is the last pending I/O,
	 * wake up the quiescing thread
	 */
	ipri = IOCACHE_LOCK();
	cdp = dp->dio_ip->i_ipmnt->i_cachedev;
	if (cdp && (--cdp->cd_pending_requests == 0) &&
	    (cdp->cd_flag & CD_QUIESCE))
		IOCACHE_WAKEUP(&cdp->cd_iowait);
	IOCACHE_UNLOCK(ipri);

        error = (bp->b_flags & B_ERROR) ? EIO : 0;

#ifdef _JFS_OS2
	/* On OS/2 the KernVMUnlock() call is not allowed during interrupt time.
	 * We will do the unlocks of all the I/O requests for this dio buffer
	 * in dioEnd()
	 */
#endif	/* _JFS_OS2 */

	ipri = DIO_IOLOCK();

	dioFreeBuf(bp);

	if (error)
		dioSetError(dp,dep->de_offset,error);

	if (--dp->dio_iocnt == 0)
		DIO_WAKEUP(&dp->dio_iowait);

	DIO_IOUNLOCK(ipri);
}


/*
 * NAME: dioGetBuf
 *
 * FUNCTION: Get an I/O buffer from the free list
 *
 * PARAMETERS: NONE
 *
 * RETURNS: Buffer from the free list
 */
static
iobuf_t *
dioGetBuf(void)
{
	int32 ipri;
	iobuf_t *bp;

	ipri = DIO_IOLOCK();

	while ((bp = diomgr.freebufs) == NULL)
		DIO_SLEEP(&diomgr.bufwait, 0);

	diomgr.freebufs = bp->b_next;
	bp->b_next = NULL;

	DIO_IOUNLOCK(ipri);

	return(bp);
}


/*
 * NAME: dioFreeBuf
 *
 * FUNCTION: Put an I/O buffer back on the free list
 *
 * PRE CONDITIONS: DIO_IOLOCK() held by caller.
 *
 * PARAMETERS:
 *	bp	- Buffer to place onto direct I/O free list
 *
 * RETURNS: NONE
 */
static void
dioFreeBuf(iobuf_t *bp)
{
	bp->b_next = diomgr.freebufs;
	diomgr.freebufs = bp;
	if (diomgr.bufwait != EVENT_NULL)
		DIO_WAKEUP(&diomgr.bufwait);
}


/*
 * NAME: dioSetError
 *
 * FUNCTION: Set error condition offset in directio structure
 *
 * PARAMETERS:
 *	dp	- directio structure
 *	offset	- byte offset of error
 *	error	- error value
 *
 * NOTES: Error offset and value will be set to the first byte offset having an
 *	error.  This function will determine if there has been an earlier error
 *	than this one and reset the values in the directio structure if
 *	necessary.
 *
 * RETURNS: NONE
 */
static void
dioSetError(dio_t *dp,
	    int64 offset,
	    int32 error)
{
	if (dp->dio_error == 0 || offset < dp->dio_erroff)
	{
		(dp)->dio_error = error;
		(dp)->dio_erroff = offset;
	}

}


#ifdef _JFS_DEBUG_DIO
void dumpiobuf(iobuf_t	*bp)
{
	ULONG	last, index;

	printf("(bp->b_flags): %08x  ",(bp->b_flags));
	printf("(bp->b_jfsbp): %08x  ",(bp->b_jfsbp));
	printf("(bp->b_next): %08x  ",(bp->b_next));
	printf("(bp->b_iodone): %08x  ",(bp->b_iodone));
	printf("Req_List_Header\n");
	printf("(bp->bio_rlh.Count): %d  ",(bp->bio_rlh.Count));
	printf("(bp->bio_rlh.Notify_Address): %08x  ",
		(bp->bio_rlh.Notify_Address));
	printf("(bp->bio_rlh.Request_Control): %08x  ",
		(bp->bio_rlh.Request_Control));
	printf("(bp->bio_rlh.Block_Dev_Unit): %08x  ",
		(bp->bio_rlh.Block_Dev_Unit));
	printf("(bp->bio_rlh.Lst_Status): %08x  ",(bp->bio_rlh.Lst_Status));
	printf("(bp->bio_rlh.y_Done_Count): %d\n",(bp->bio_rlh.y_Done_Count));
	printf("(bp->bio_rlh.y_PhysAddr): %d\n",(bp->bio_rlh.y_PhysAddr));
	printf("PB_Read_Write\n");
	printf("Req_Header\n");
	printf("(bp->bio_rh.RqHdr.Length): %d  ",(bp->bio_rh.RqHdr.Length));
	printf("(bp->bio_rh.RqHdr.Old_Command): %08x  ",
		(bp->bio_rh.RqHdr.Old_Command));
	printf("(bp->bio_rh.RqHdr.Command_Code): %08x  ",
		(bp->bio_rh.RqHdr.Command_Code));
	printf("(bp->bio_rh.RqHdr.Head_Offset): %d  ",
		(bp->bio_rh.RqHdr.Head_Offset));
	printf("(bp->bio_rh.RqHdr.Req_Control): %08x  ",
		(bp->bio_rh.RqHdr.Req_Control));
	printf("(bp->bio_rh.RqHdr.Priority): %d\n",(bp->bio_rh.RqHdr.Priority));
	printf("(bp->bio_rh.RqHdr.Status): %08x  ",(bp->bio_rh.RqHdr.Status));
	printf("(bp->bio_rh.RqHdr.Error_Code): %08x  ",
		(bp->bio_rh.RqHdr.Error_Code));
	printf("(bp->bio_rh.RqHdr.Notify_Address): %08x  ",
		(bp->bio_rh.RqHdr.Notify_Address));
	printf("(bp->bio_rh.RqHdr.Hint_Pointer): %d  ",
		(bp->bio_rh.RqHdr.Hint_Pointer));
	printf("(bp->bio_rh.RqHdr.Waiting): %d  ",(bp->bio_rh.RqHdr.Waiting));
	printf("(bp->bio_rh.RqHdr.FT_Orig_Pkt): %d  ",
		(bp->bio_rh.RqHdr.FT_Orig_Pkt));
	printf("(bp->bio_rh.RqHdr.Physical): %d\n",(bp->bio_rh.RqHdr.Physical));

	printf("(bp->bio_rh.Start_Block): %d  ",(bp->bio_rh.Start_Block));
	printf("(bp->bio_rh.Block_Count): %d  ",(bp->bio_rh.Block_Count));
	printf("(bp->bio_rh.Blocks_Xferred): %d\n",(bp->bio_rh.Blocks_Xferred));
	printf("(bp->bio_rh.RW_Flags): %08x  ",(bp->bio_rh.RW_Flags));
	printf("(bp->bio_rh.SG_Desc_Count): %d  ",(bp->bio_rh.SG_Desc_Count));
	printf("(bp->bio_rh.SG_Desc_Count2): %d  ",(bp->bio_rh.SG_Desc_Count2));
	printf("SG_Descriptors\n");
	last = bp->bio_rh.SG_Desc_Count;
	for(index = 0; index < last; index++) {
		printf("(bp->bio_sg[%d].BufferPtr): %d  ",index,
			(bp->bio_sg[index].BufferPtr));
		printf("(bp->bio_sg[%d].BufferSize): %d\n",index,
			(bp->bio_sg[index].BufferSize));
	}
}
#endif	/* _JFS_DEBUG_DIO */
