/* $Id: jfs_cachemgr.c,v 1.1 2000/04/21 10:58:04 ktk Exp $ */

static char *SCCSID = "@(#)1.47  11/1/99 12:41:20 src/jfs/ifs/jfs_cachemgr.c, sysjfs, w45.fs32, fixbld";
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
 *
 * Module: jfs_cachemgr.c
 */

/*
 * Change History :
 *
 *
 */


#ifdef	_JFS_OS2
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#include "mmph.h"
#endif	/* _JFS_OS2 */

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_cachemgr.h"
#include "jfs_xtree.h"
#include "jfs_dmap.h"		/* for function prototype */
#include "jfs_proto.h"
#include "jfs_debug.h"

#define _JFS_SMARTIO						// D230860

#ifdef	_JFS_OS2
/* use the jfs definition for page size */
#define PAGESIZE PSIZE
#endif	/* _JFS_OS2 */

/*
 *	 cbuf buffer cache (cCache) manager
 */
extern uint32	CacheSize;
int32	nBuffer;
struct cCache cachemgr;

/*
 *	read-ahead control
 *
 * max_rapage is set to the maximum number of pages to read-ahead at one time.
 * This is an extern so that it can be modified at runtime to tune the
 * filesystem for specific situations.  The tune code should verify that it
 * never goes below MIN_RAPAGE, and that it is a power of 2 multiple.
 */
int32	max_rapage = MAX_RAPAGE;


/*
 *	coalesce control
 *
 * max_coalesce is set to maximum number of pages to coalece in one request.
 */
int32	max_coalesce = MAX_RAPAGE;

/*
 *	cache manager lock
 */
#define CACHEMGR_LOCK_ALLOC()\
	MUTEXLOCK_ALLOC(&cachemgr.cachelock,LOCK_ALLOC_PAGED,JFS_CACHE_LOCK_CLASS,0)
#define CACHEMGR_LOCK_INIT()    MUTEXLOCK_INIT(&cachemgr.cachelock)
#define CACHEMGR_LOCK()         MUTEXLOCK_LOCK(&cachemgr.cachelock)
#define CACHEMGR_UNLOCK()       MUTEXLOCK_UNLOCK(&cachemgr.cachelock)

/*
 *	i/o cache lock
 */
XSPINLOCK_T	jfsIOCacheLock;

#define IOCACHE_LOCK_ALLOC()\
	XSPINLOCK_ALLOC((void *)&jfsIOCacheLock,LOCK_ALLOC_PIN,JFS_IOCACHE_LOCK_CLASS,-1)
#define IOCACHE_LOCK_INIT()	XSPINLOCK_INIT(&jfsIOCacheLock)
/* IOCACHE_LOCK()/IOCACHE_UNLOCK(): refer to jfs_cachemgr.h */
/* IOCACHE_SLEEP()/IOCACHE_WAKEUP(): refer to jfs_cachemgr.h */


/*
 *	buffer cache cachelist
 *
 * a doubly-linked list of pages anchored in cachemgr.cachelist
 */
#define CACHELIST_INSERT_HEAD(ANCHOR, ELM, FIELD)\
{\
	CDLL_INSERT_HEAD(ANCHOR, ELM, FIELD);\
	(ELM)->cm_cached = TRUE;\
}

#define CACHELIST_INSERT_TAIL(ANCHOR, ELM, FIELD)\
{\
	CDLL_INSERT_TAIL(ANCHOR, ELM, FIELD);\
	(ELM)->cm_cached = TRUE;\
}

#define CACHELIST_INSERT_AFTER(ANCHOR, LISTELM, ELM, FIELD)\
{\
	CDLL_INSERT_AFTER(ANCHOR, LISTELM, ELM, FIELD);\
	(ELM)->cm_cached = TRUE;\
}

#define CACHELIST_REMOVE(ANCHOR, ELM, FIELD)\
{\
	CDLL_REMOVE(ANCHOR, ELM, FIELD);\
	(ELM)->cm_cached = FALSE;\
}

#define	CACHELIST_SELF(ELM, FIELD)\
	CDLL_SELF(ELM, FIELD)


/*
 *	buffer cache hashlist
 */
#define HASHLIST_INIT(ANCHOR)	LIST_INIT(&(ANCHOR)->ch_hashlist)

#define HASHLIST_INSERT_HEAD(ANCHOR, ELM, FIELD)\
{\
	LIST_INSERT_HEAD(&(ANCHOR)->ch_hashlist, ELM, FIELD);\
	(ANCHOR)->ch_timestamp++;\
}

#define HASHLIST_REMOVE(ELM, FIELD)\
	LIST_REMOVE(ELM, FIELD)

#define	HASHLIST_SELF(ELM, FIELD)\
	LIST_SELF(ELM, FIELD)

/*
 *	buffer cache inode pagelist
 *
 * a doubly-linked list of pages of inode anchored in the inode
 */
#define PAGELIST_INSERT_HEAD(ANCHOR, ELM, FIELD)\
{\
	CDLL_INSERT_HEAD(ANCHOR, ELM, FIELD);\
	(ELM)->cm_ctl->cc_ncbufs += 1;\
}

#define PAGELIST_INSERT_TAIL(ANCHOR, ELM, FIELD)\
{\
	CDLL_INSERT_TAIL(ANCHOR, ELM, FIELD);\
	(ELM)->cm_ctl->cc_ncbufs += 1;\
}

#define PAGELIST_REMOVE(ANCHOR, ELM, FIELD)\
{\
	CDLL_REMOVE(ANCHOR, ELM, FIELD);\
	(ELM)->cm_ctl->cc_ncbufs -= 1;\
}

#define	PAGELIST_SELF(ELM, FIELD)\
	CDLL_SELF(ELM, FIELD)

/*
 * external references
 */
int32 extFill(inode_t *ip, cbuf_t *cp);
extern void bmIODone(iobuf_t *bp);

/*
 * forward references
 */
void cmIODone(iobuf_t *bp);
static void cmReadAhead(inode_t *ip, int64 lblkno, cbuf_t *cp, int32 rdsize,
	cbuf_t **ratail);
static cbuf_t *cmWriteCtl(cmctl_t *ccp, int64 blkno, int64 nblks, int32); //D230860
static cbuf_t *cmLookup(cmctl_t *ccp, int64 blkno);
static cbuf_t *cmGet(cmctl_t *ccp, int64 blkno, uint32 flag);
static void cmRelCtl(cmctl_t *ccp, int64 blkno, int64 nblks);
#ifdef	_JFS_OS2
static cbuf_t *cmCoalesce(cmdev_t *cdp, cbuf_t **xiotail, cbuf_t *cp);
#endif	/* _JFS_OS2 */

static int32 rawInit(void);
static void rawStartIO(cbuf_t *bp);
void rawIODone(iobuf_t	*iobp);


#ifdef	_JFS_STATISTICS
/*
 *	statistics
 */
struct statBCache {
	int32	iobufwait;
} statBCache = {0};
#endif	/* _JFS_STATISTICS */

int32	init_failed_unrecoverable = 0;

/*
 * NAME:	cmInit()
 *
 * FUNCTION:	initialize the cache manager at JFS initialization.
 *
 *		pinned memory is allocated for the cbuf hash headers, the
 *		cbufs, and cache buffers.  the hash headers and cache list
 *		are initialized. all cbufs are placed on the free list.
 *
 * PARAMETERS: 	none.
 *
 * RETURN VALUES:
 *	0	- success
 *	ENOMEM	- insufficient memory
 *
 *						OS/2
 * JFS buffer cache buffer header:
 *  meta-data buffer prefix:	32 bytes	 32
 *  data buffer base:		64 bytes	 64
 *  OS/2 buffer extension:	 8 bytes	  8
 * I/O buffer header:
 *  OS/2: 16 + 72 + 8*8 =	152 bytes	152
 *						256
 */
cmInit()
{
	int32	rc;
	caddr_t bp, dp, iobpp;
	cbuf_t	*cp;
	iobuf_t	*iobp;
	chash_t *chp;
	int32	nHashClass, nbyte, bhsize, iobhsize, i;
#ifdef	_JFS_OS2
	ULONG	pgcnt;
	Req_List_Header	*rlhp;
	Req_Header	*rhp;
#endif	/* _JFS_OS2 */

	if (init_failed_unrecoverable)
		return ENOMEM;

	/* init the cache manager structure */
	cachemgr.devfree = NULL;
	cachemgr.devtail = NULL;
	cachemgr.ctlfree = NULL;
	cachemgr.minfree = MAX(8,nBuffer*.02);			// D233382
	cachemgr.maxfree = MAX(64,nBuffer*.04);			// D233382
	cachemgr.numiolru = 0;
	event_init(&(cachemgr.freewait));

	/*
	 * compute number of (buffer header + buffer page)
	 * (based on system configuration parameter CacheSize),
	 */
#ifdef	_JFS_OS2
	iobhsize = sizeof(iobuf_t) + (max_coalesce - 1) * sizeof(SG_Descriptor);
	bhsize = sizeof(cbuf_t) + iobhsize;
	i = PSIZE/bhsize;
	assert(i*bhsize == PSIZE);
#endif	/* _JFS_OS2 */
	nBuffer = CacheSize / PSIZE;
	cachemgr.cachesize = nBuffer;;

	/*
	 * allocate pinned memory for the buffer data pages
	 */
	nbyte = nBuffer * PSIZE;
	if ((dp = xmalloc(nbyte,0,pinned_heap|XMALLOC_HIGH)) == NULL)
		return(ENOMEM);

	/*
	 * allocate pinned memory for the buffer headers
	 */
	nbyte = nBuffer * bhsize;
	nbyte = (nbyte + (PSIZE - 1)) & ~(PSIZE - 1);
	if ((bp = xmalloc(nbyte, 0, pinned_heap|XMALLOC_HIGH)) == NULL)
	{
		/* We cannot free memory allocated above 16 Meg.
		 * Hopefully this will change soon.
		 */
		init_failed_unrecoverable = 1;
		return(ENOMEM);
	}
	memset(bp, 0, nbyte);

	/*
	 * allocate/initialize the hashlist anchor table
	 * (bound hash list length based on number of buffers);
	 */
	nHashClass = nBuffer >> 1;
	/* round up to next smallest power of 2 */
	nHashClass = roundup2power(nHashClass);
	cachemgr.hashsize = nHashClass;

	nbyte = nHashClass * sizeof(chash_t);
	if ((chp = (chash_t *)xmalloc(nbyte,0,pinned_heap|XMALLOC_HIGH)) == NULL)
	{
		/* We cannot free memory allocated above 16 Meg.
		 * Hopefully this will change soon.
		 */
		init_failed_unrecoverable = 1;
		return(ENOMEM);
	}
	cachemgr.hashtbl = chp;

        for (i = 0; i < nHashClass; i++, chp++)
        {
		chp->ch_hashlist.head = NULL;
		chp->ch_timestamp = 0;
        }

	/*
	 * initialize the cachelist
	 */
	cachemgr.cachelist.head = (cbuf_t *) &cachemgr.cachelist;
	cachemgr.cachelist.tail = (cbuf_t *) &cachemgr.cachelist;
	
	/*
	 * initialize/insert all cbufs on the freelist
	 */
	cachemgr.freelist = NULL;
	cachemgr.nfreecbufs = nBuffer;
	for (i = 0; i < nBuffer; i++, bp += bhsize, dp += PSIZE)
	{
		/*
		 * init jfs buffer header
		 */
		cp = (cbuf_t *)bp;
		cp->cm_flag = 0;
		cp->cm_cdata = dp;
		event_init(&cp->cm_event);
		event_init(&cp->cm_ioevent);

		cp->cm_free = TRUE;
		cp->cm_freenxt = cachemgr.freelist;
		cachemgr.freelist = cp;

#ifdef	_JFS_OS2
                /* convert the buffer address to a physical address */
                rc = KernLinToPageList(dp, PSIZE,
			(KernPageList_t *)&cp->cm_pgdesc, &pgcnt);
                assert(rc == 0 && pgcnt == 1);
#endif	/* _JFS_OS2 */

		/*
		 * init device i/o buffer header
		 */
		iobp = (iobuf_t *)(bp + sizeof(cbuf_t));

		/* bind jfs buffer with device i/o buffer */
		cp->cm_iobp = iobp;
		iobp->b_jfsbp = (void *)cp;

#ifdef	_JFS_OS2
		/*
		 * fill in request list header
		 */
		rlhp = &iobp->bio_rlh;
		rlhp->Count = 1;
		rlhp->Notify_Address = (void *)jfsIODone;
		rlhp->Request_Control = RLH_Single_Req |
					RLH_Notify_Err | RLH_Notify_Done;
		/* rlhp->Block_Dev_Unit */
		/* rlhp->Lst_Status */

		/*
		 * fill in request header
		 */
		rhp = &iobp->bio_rh.RqHdr;
		rhp->Length = RH_LAST_REQ;
		rhp->Old_Command = PB_REQ_LIST;
		/* rhp->Command_Code */
		rhp->Head_Offset = sizeof(Req_List_Header);
		rhp->Req_Control = 0;
		rhp->Priority = PRIO_FOREGROUND_USER;
		/* rhp->Status */
		/* rhp->Error_Code */
		/* rhp->Notify_Address */
		rhp->Hint_Pointer = -1;	/* no hint */
#endif	/* _JFS_OS2 */
	}

	/* allocate/initialize the cache manager lock */
	CACHEMGR_LOCK_ALLOC();
	CACHEMGR_LOCK_INIT();

	/* allocate/initialize the cache manager i/o lock */
	IOCACHE_LOCK_ALLOC();
	IOCACHE_LOCK_INIT();

#ifdef	_JFS_HEURISTICS_SLRU
	cachemgr.slrun = 0;
	cachemgr.slruN = (nBuffer/3)*2;
	cachemgr.slruxp = (void *)&cachemgr.cachelist;
#endif	/* _JFS_HEURISTICS_SLRU */

	/*
	 * initialize raw i/o cache
	 */
	if (rc = rawInit())
		return rc;

jFYI(1,("cCache: nBuffer:%d headerSize:%d:%d nHashClsss:%d\n",
	nBuffer, sizeof(cbuf_t), iobhsize, nHashClass));
	return(0);
}


/*
 * NAME:	cmMount()
 *
 * FUNCTION:	allocate and initialize a cache device for a file system.
 *
 *		a cmdev structure is allocated and initialized for the
 *		specified file system (device).  as part of the init of
 *		the cache device, pinned memory for the specified number
 *	        of device buffer headers is allocated and the buffer headers
 *		are placed on cmdev's device buffer header free list.
 *
 *		cmdevs are allocated from the cache manager free list.  if
 *	        the free list is empty, a page's worth of pinned memory will
 * 		be allocated and the free list will be replenished.
 *
 * PARAMETERS:
 *	ipmnt	- pointer to mount inode
 *	nbufs	- number of device buffer header to be allocated for this
 *		  cache device.
 *
 * RETURN VALUES:
 *	0	- success
 *	ENOMEM	- insufficient memory
 */
int32
cmMount(struct inode *ipmnt,
	int32 nbufs)
{
	cmdev_t	*cdp;
	int32	i;

	CACHEMGR_LOCK();

	/*
	 * allocate/initialize cache device structure
	 */
	if (cachemgr.devfree == NULL)
	{
		/* allocate pinned storage for more cache device structs */
		i = PSIZE / sizeof(cmdev_t);
		if ((cachemgr.devfree = (cmdev_t *)xmalloc(PSIZE,
						0, pinned_heap)) == NULL)
		{
			CACHEMGR_UNLOCK();
			return(ENOMEM);
		}

		/* insert the structs on the free list */
		for (cdp = cachemgr.devfree; i - 1; cdp++, i--)
			cdp->cd_freenxt = cdp + 1;
		cdp->cd_freenxt = NULL;
	}

	cdp = cachemgr.devfree;
	cachemgr.devfree = cdp->cd_freenxt;

	CACHEMGR_UNLOCK();

	/* initialize the cache device */
	cdp->cd_nextdev = NULL;
	cdp->cd_dev = ipmnt->i_dev;
	cdp->cd_flag = 0;
	cdp->cd_l2bsize = ipmnt->i_l2bsize;
	cdp->cd_l2pbsize = ipmnt->i_l2pbsize;
	cdp->cd_l2bfactor = ipmnt->i_l2bfactor;
	cdp->cd_nctls = 0;
	cdp->cd_iolevel = 0;
	cdp->cd_pending_requests = 0;
	event_init(&cdp->cd_iowait);
	event_init(&cdp->cd_quiesce_event);
	cdp->cd_iotail = NULL;
#ifdef	_JFS_OS2
	if (ipmnt->i_Strategy3)
	{
        	cdp->cd_strat3p = ipmnt->i_Strategy3;
		cdp->cd_flag |= CD_STRAT3;
	}
	else
        	cdp->cd_strat2p = ipmnt->i_Strategy2;
	assert(ipmnt->i_maxSGList);
#endif	/* _JFS_OS2 */

	/* bind cache device to mounted file system */
	ipmnt->i_cachedev = cdp;

jEVENT(0,("cmMount: cdp:0x%08x\n", cdp));
	return(0);
}


/*
 * NAME:	cmUnmount()
 *
 * FUNCTION:	free resources associated with a cache device.
 *
 *		the pinned memory for the cache device's device buffer
 *		headers are freed and the cache device is place on the
 *		free list.
 *
 *		a cache device should not be freed if it has any active
 *		cache control elements (cmctls).
 *
 * PARAMETERS:
 *	cdp	- pointer to the cache device (cmdev) to be deleted.
 *
 * RETURN VALUES: none.
 */
void
cmUnmount(cmdev_t *cdp)
{
	iobuf_t	*bp, *low;

	/* there should be no active cmctls for this cache device,
	 * unless swapper.dat was on our filesystem and
	 * the system is being shutdown.
	 * If we're shutting down, don't bother with freeing
	 * anything.
	 */
	if (cdp->cd_nctls != 0)
		return;

	/*
	 * free the cache device
	 */
	CACHEMGR_LOCK();

	cdp->cd_freenxt = cachemgr.devfree;
	cachemgr.devfree = cdp;

	CACHEMGR_UNLOCK();
}


/*
 * NAME:	cmCreate()
 *
 * FUNCTION:	allocate and initialize a cache control (cmctl) element
 *		under a specified cache device.
 *
 *		a cmctl is allocated from the cache manager's cmctl free
 *	        list and initialized.  if the free list is empty, a page's
 *		worth of pinned memory will be allocated and the free list
 *		will be replenished.
 *
 * PARAMETERS:
 *	cdp	- pointer to the cache device (cmdev) for the new cmctl.
 *	ccpp	- pointer to cmctl pointer to be filled in on successful
 *		  return with the pointer of the newly create cmctl.
 *
 * RETURN VALUES:
 *	0	- success
 *	ENOMEM	- insufficient memory
 */
int32
cmCreate(cmdev_t *cdp,
	 cmctl_t **ccpp)
{
	cmctl_t *ccp;
	int32 n;

	CACHEMGR_LOCK();

	/*
	 * allocate cmctl
	 */
	if (cachemgr.ctlfree == NULL)
	{
		/* allocate memory for the new cmctls */
		n = PAGESIZE / sizeof(cmctl_t);
		if ((cachemgr.ctlfree = (cmctl_t *)xmalloc(n * sizeof(cmctl_t),
						0, pinned_heap)) == NULL)
		{
			CACHEMGR_UNLOCK();
			return(ENOMEM);
		}

		/* insert them on the free list */
		for (ccp = cachemgr.ctlfree; n - 1; ccp++, n--)
		{
			ccp->cc_freenxt = ccp + 1;
			ccp->cc_flag = 0;
		}

		ccp->cc_freenxt = NULL;
		ccp->cc_flag = 0;
	}

	ccp = cachemgr.ctlfree;
	cachemgr.ctlfree = ccp->cc_freenxt;

	/* increment the cache dev cmctl count */
	cdp->cd_nctls += 1;

	CACHEMGR_UNLOCK();

	/*
	 * init the cmctl
	 */
        ccp->cc_valid = TRUE;

        ccp->cc_dev = cdp;
        ccp->cc_ncbufs = 0;
        ccp->cc_iolevel = 0;
	ccp->cc_iowait = EVENT_NULL;
	CDLL_INIT(&ccp->cc_cbuflist);
        ccp->cc_njbufs = 0;
	ccp->cc_jiolevel = 0;
	ccp->cc_jiowait = EVENT_NULL;
	CDLL_INIT(&ccp->cc_jbuflist);
	ccp->cc_delwait = EVENT_NULL;

	/* return cmctl */
	*ccpp = ccp;

	return(0);
}


/*
 * NAME:	cmDelete()
 *
 * FUNCTION:	free a cache control (cmctl) element.
 *
 *		all of the cmctl's cbufs are discarded and the cmctl
 *		cmctl is place on the cache manager's cmctl free list
 *		after waiting for any in progress cbuf i/o to complete.
 *
 * PARAMETERS:
 *	ccp	- pointer to the cmctl to be deleted.
 *
 * RETURN VALUES: none.
 */
int32
cmDelete(cmctl_t *ccp)
{
	int32	ipl;
	cmdev_t	*cdp;

        ipl = IOCACHE_LOCK();

	/* if the cmctl still has some cbufs that are in an i/o state,
	 * mark as the cmctl pending delete and
	 * wait for all the i/os to complete.
	 */
	if (ccp->cc_cbuflist.head != (cbuf_t *)&ccp->cc_cbuflist)
	{
		ccp->cc_delpend1 = TRUE;
		IOCACHE_SLEEP(&ccp->cc_delwait,0);
	}
	
	/* if the cmctl still has some jbufs that are in an i/o state,
	 * mark as the cmctl pending delete and
	 * wait for all the i/os to complete.
	 */
	if (ccp->cc_jbuflist.head != (cbuf_t *)&ccp->cc_jbuflist)
	{
		ccp->cc_delpend2 = TRUE;
		IOCACHE_SLEEP(&ccp->cc_delwait,T_NORELOCK);
        	IOCACHE_NOLOCK(ipl);
	}
	else
        	IOCACHE_UNLOCK(ipl);

	/*
	 * release the cmctl back to the cache dev
	 */
	CACHEMGR_LOCK();

	/* decrement the cache dev cmctl count */
	ccp->cc_dev->cd_nctls -= 1;

	/* insert the cmctl on the free list */
	ccp->cc_flag = 0;
	ccp->cc_freenxt = cachemgr.ctlfree;
	cachemgr.ctlfree = ccp;

	CACHEMGR_UNLOCK();

	return(0);
}


/*
 * NAME:	cmRead()
 *
 * FUNCTION:	assign a cbuf to a specified inode and offset.
 *
 *		cmGet() is called to get a cbuf for the inode's cache
 *		control element (cmctl) and the logical block with the
 *		inode that holds the specified offset.  if the returned
 *		cbuf has not been previously initialized, it will be
 *		initialized (typically from disk).  the returned cbuf
 *		will be in an inuse state.
 *
 * PARAMETERS:
 *	ip	- pointer to file's inode.
 *	offset	- starting offset with the file of the read.
 *	rdsize	- current read request size in page;
 *	cpp	- pointer to cbuf pointer to be filled in on successful
 *		  return with the pointer of the specified cbuf.
 *
 * RETURN VALUES:
 *	0	- success
 *	EIO	- i/o error
 */
int32
cmRead(inode_t	*ip,
	int64	offset,
	int32	rdsize,
	cbuf_t	**cpp)
{
	int32	rc = 0;
	int64	lblkno;
	int64	laddr;
	uint8	xflag;
	int64	xaddr;
	int32	xlen;
	cbuf_t	*cp, *rahead, *ratail, *iotail;
	cmdev_t	*cdp;
	int32	ipl, wait_read;

	/* get the logical block number associated with this offset */
	lblkno = offset >> CM_L2BSIZE;
jEVENT(0,("cmRead: offset:%d%d pno:%d%d rdsize:%d\n", offset, lblkno, rdsize));

   MMPHPrecmRead();        /* MMPH Performance Hook */

	/* get the cbuf for the page */
	cp = cmGet(ip->i_cacheid,lblkno,CM_WAIT|CM_READ);

	/*
	 *	page is NOT resident in cache:
	 */
	if (cp->cm_new)
	{
		/* look up backing extent for the page */
         	laddr = lblkno << ip->i_ipmnt->i_l2nbperpage;
         	if (rc = xtLookup(ip, laddr, ip->i_ipmnt->i_nbperpage,
				&xflag, &xaddr, &xlen, 0))
		{
			cmPut(cp,FALSE);
			goto cmRead_Exit;
		}

		cp->cm_new = FALSE;
		cp->j_ip = ip;

		/* page has NO backing extent - hole */
                if (xlen == 0)
                {
			/* mark the cbuf as hole and
			 * return zero-filled buffer.
			 */
			cp->cm_hole = TRUE;
			PXDaddress(&cp->cm_pxd,0);
			PXDlength(&cp->cm_pxd,0);
			bzero(cp->cm_cdata, CM_BSIZE);

			*cpp = cp;
			goto cmRead_Exit;
                }
		/* page has backing extent */
		else
		{
			/* bind cbuf with its backing extent */
			PXDaddress(&cp->cm_pxd, xaddr);
			PXDlength(&cp->cm_pxd, xlen);

			/* page is allocated but not recorded:
			 * mark the cbuf as abnr and return zero-filled buffer.
			 */
                        if (xflag & XAD_NOTRECORDED)
                        {
                                cp->cm_abnr = TRUE;
                                bzero(cp->cm_cdata, CM_BSIZE);

				*cpp = cp;
            			goto cmRead_Exit;
                        }
			/* page in data from disk */
                        else
                        {
                                /* init this buffer as head of pagein list */
				IOQUEUE_READ(cp);

                                ratail = cp;
                                wait_read = TRUE;

				/*
				 * construct read-ahead circular buffer list
				 */
				if (rdsize > 0 && ip->i_rapage != -1)
					cmReadAhead(ip, lblkno, cp, rdsize, &ratail);
				else
					ratail->cm_ionxt = ratail;
                        }
		}
	}
	/*
	 *	page is resident in cache and reclaimed:
	 */
	else
	{
		ratail = NULL;
		wait_read = FALSE;

		/*
		 * construct read-ahead circular buffer list
		 */
		if (cp->cm_rdahead && rdsize > 0 && ip->i_rapage != -1)
			cmReadAhead(ip, lblkno, cp, rdsize, &ratail);
	}

	/*
	 * read pages of pagein list from disk.
	 */
	if (ratail)
	{
		cdp = ratail->cm_ctl->cc_dev;
		rahead = ratail->cm_ionxt;

		/* start the i/o */
		cmDeviceStartIO(cdp, ratail);

		/* wait for the read to complete */
		if (wait_read)
		{
			ipl = IOCACHE_LOCK();

			if (!rahead->cm_iodone)
			{
				IOCACHE_SLEEP_RC(&rahead->cm_ioevent,T_NORELOCK,rc);
				IOCACHE_NOLOCK(ipl);
			}
			else
			{
				rc = (rahead->cm_iobp->b_flags & B_ERROR) ?
					EIO : 0;
				IOCACHE_UNLOCK(ipl);
			}
			if (rc)
			{					// D228362
				cp->cm_new = TRUE;		// D228362
				cmPut(cp,FALSE);		// D228362
				goto cmRead_Exit;
			}					// D228362
		}
	}

	*cpp = cp;

cmRead_Exit:

   MMPHPostcmRead();       /* MMPH Performance Hook */

	return(rc);
}


/*
 * NAME:	cmReadAhead()
 *
 * FUNCTION:	construct read-ahead buffer list;
 *
 * PARAMETERS:
 *	ip	- Inode to perform read-ahead against
 *	lblkno	- Logical block number of current page read
 *	cp	- current page buffer
 *	rdsize	- current read request size in page;
 *	ratail	- tail of buffer I/O list
 *
 * RETURNS: none.
 *
 * SERIALIZATION: sets ip->i_ra_lock to ensure serialization.  We cannot
 * hold the INODE_LOCK since we may have to do some I/O.
 */
static void
cmReadAhead(inode_t	*ip,
	int64	lblkno,
	cbuf_t	*cp,
	int32	rdsize,
	cbuf_t	**ratail)
{
	int32	rc;
	int32	ipl;						// D228112
	pxdlist_t	pxdlist;
	cbuf_t		*racp[MAX_RAPAGE], *xcp, *tail;
	int64		firstp, nextp, tripwire, lastp, xp, loff;
	int32		npages, llen, n, i, nbuffer = 0;

   MMPHPrecmReadAhead();       /* MMPH Performance Hook */

	/* set tail/head buffer in pagein list */
	tail = *ratail;

        INODE_LOCK(ip);
	if (ip->i_ra_lock)
	{
		/* Another thread is doing read-ahead */
		INODE_UNLOCK(ip);
		goto locked_out;
	}
	ip->i_ra_lock = 1;
	INODE_UNLOCK(ip);

	/*
	 * read-ahead tripwire triggered ?
	 */
	if (cp && cp->cm_rdahead)
	{
		cp->cm_rdahead = FALSE;

		/* is it a stray tripwire ? */
		if (lblkno != ip->i_rapage)
		{
			ip->i_rapage = lblkno;
			ip->i_stepsize = 1;
			ip->i_rasize = 0;
			goto out;
		}

		/* compute the first page to read-ahead */
		firstp = lblkno + ip->i_rasize;
		tripwire = firstp;

		/* compute number of pages to read-ahead */
		ip->i_stepsize = ip->i_stepsize == max_rapage ?
				max_rapage : ip->i_stepsize << 1;
		npages = CM_BTOCBLK(ip->i_size) - firstp + 1;
		npages = MIN(ip->i_stepsize, npages);
jEVENT(0,("cmReadAhead repeat start:%d%d n:%d tripwire:%d%d\n",
	firstp, npages, tripwire));
	}
	/*
	 * (re)start sequential read ?
	 */
	else
	{
		/* single page read ? */
		if (rdsize == 1)
		{
			/* is current page sequential to lat page read ? */
			if (lblkno == ip->i_rapage + 1)
			{
				/* compute the first page to read-ahead */
				firstp = lblkno + 1;
				tripwire = firstp;

				/* compute number of pages to read-ahead */
				ip->i_stepsize = MIN_RAPAGE;
				npages = ip->i_stepsize;
jEVENT(0,("cmReadAhead: restart1 start:%d%d n:%d tripwire:%d%d\n",
	firstp, npages, tripwire));
				goto raStart;
			}
			else
			{
				ip->i_rapage = lblkno;
				ip->i_stepsize = 1;
				ip->i_rasize = 0;
				goto out;
			}
		}

		/*
		 * multiple-page read: ramp up directly as if
		 * incremental steps have been performed;
		 */
		/* compute the first page to read-ahead */
		firstp = lblkno + 1;

		/* compute number of pages to read-ahead */
		npages = 1;
		i = 1;
		while (npages <= rdsize)
		{
			n = npages;
			npages += i;
			if (i < MAX_RAPAGE)
				i <<= 1;
		}

		tripwire = lblkno + n;

		if (lblkno != -1)
			npages -= 1;	/* subtract current page */
		n = CM_BTOCBLK(ip->i_size) - firstp + 1;
		npages = MIN(npages, n);

		ip->i_stepsize = i >> 1;
jEVENT(0,("cmReadAhead: restart2 start:%d%d n:%d tripwire:%d%d\n",
	firstp, npages, tripwire));
	}

raStart:
	n = 0;
	lastp = firstp + npages;

	/* build buffer list */
	for (nextp = firstp; nextp < lastp; )
	{
		/* max_rapage can be set higher than MAX_RAPAGE:
		 * split gathering of buffers into at most MAX_RAPAGE
		 * at a time.
		 */

		/* Loop through getting the buffers for read-ahead pages */
		xp = nextp;
                loff = xp << ip->i_ipmnt->i_l2nbperpage;
		llen = 0;
		for (i = 0, nbuffer = 0;
		     i < MAX_RAPAGE && xp < lastp;
		     i++, xp++)
		{
			/* get the cbuf for the page */
			racp[i] = xcp = cmGet(ip->i_cacheid, xp, CM_READ);

			/*
			 * page is NOT resident:
			 */
			if (xcp && xcp->cm_new)
			{
				xcp->j_ip = ip;
				/* increase pages to map */
                		llen += ip->i_ipmnt->i_nbperpage;
                		nbuffer++;
			}
			/* either free buffer is not available
			 * or the page is already resident:
			 */
			else
			{
				if (xcp)
					cmPut(xcp, FALSE);

				if (i == 0)
					goto raEnd;

				break;
			}
		}

		/* look up backing extents for the read-ahead pages */
                pxdlist.maxnpxd = nbuffer;
               	pxdlist.npxd = 0;
                if ((rc = xtLookupPageList(ip, loff, llen, &pxdlist, 0)) ||
		    pxdlist.npxd == 0)
		{
			/* free all buffers gathered */
			for (i= 0; i < nbuffer; i++)
				cmPut(racp[i],FALSE);

			goto raEnd;
		}

		/* free remaining buffers gathered */
		for (i= pxdlist.npxd; i < nbuffer; i++)
			cmPut(racp[i],FALSE);

		/* build read-ahead pagein list */

		ipl = IOCACHE_LOCK();				// D228112
		for (i = 0; i < pxdlist.npxd; i++)
		{
			xcp = racp[i];

			/* mark first page as tripwire for next read-ahead */
			if (nextp == tripwire)
			{
				xcp->cm_rdahead = TRUE;

				ip->i_rapage = tripwire;
			}

			xcp->cm_new = FALSE;

			/* mark the read-ahead buf not in use
			 * so it will be inserted in cachelist
			 * when the read finishes
			 */
			xcp->cm_inuse = FALSE;

			IOQUEUE_ASYNC_READ(xcp);		// D230860

			/* bind buffer with its extent */
			xcp->cm_pxd = pxdlist.pxd[i];

			/* append the buffer to pagein list */
			if (tail)
				tail->cm_ionxt = xcp;
			else
				*ratail = xcp;	/* save head */

			tail = xcp;

#ifdef	_JFS_WIP
			/*
			 * build list of coalesced list of cbufs now
			 * vs. cmCoalesce() at cmDeviceStartIO() time ?
			 */
#endif	/* _JFS_WIP */

			/* increment readahead page count */
			nextp++;
			n++;
		}
		IOCACHE_UNLOCK(ipl);				// D228112

		/* is page mapping terminated ? */
		if (pxdlist.npxd < MAX_RAPAGE)
			break;
	}

raEnd:
	/* any page ahead pages ready for pagein ? */
	if (n > 0)
	{
		if (nextp >= tripwire)
		{
			ip->i_rapage = tripwire;
			ip->i_rasize = nextp - tripwire;
		}
		else
		{
			ip->i_rapage = firstp;
			ip->i_rasize = n;
		}
	}
	else
	{
		ip->i_rapage = lblkno;
		ip->i_stepsize = 1;
		ip->i_rasize = 0;
	}

out:
        INODE_LOCK(ip);
	ip->i_ra_lock = 0;
        INODE_UNLOCK(ip);

locked_out:
	/* make singly-linked circular list */
	if (tail)
	{
		tail->cm_ionxt = *ratail;
		*ratail = tail;
	}

jEVENT(0,("cmReadAhead: rapage:%d%d stepsize:%d rasize:%d\n",
	ip->i_rapage, ip->i_stepsize, ip->i_rasize));

   MMPHPostcmReadAhead();              /* (d201828) MMPH Performance Hook */
	return;
}


/*
 * NAME:	cmReadWR()
 *
 * FUNCTION:	assign a cbuf for a specified logical block number
 *		within a file (cmctl) in preparation fron an update
 *		(write) to the logical block.
 *
 *		this routine is a special case of cmRead() and is used
 *		in implementing file writes. it is special in two ways:
 *		first, it does not initiate readahead.
 *		second, it does not initialize the cbuf from disk
 *		if the update (write) will completely overwrite
 *		the on disk portion of the logical block.
 *
 * PARAMETERS:
 *	ip	- pointer to inode for the file.
 *	offset	- offset within the file at which the update will
 *		  begin.  the logical block number is derived from
 *		  this value.
 *	nbytes	- size of the update.  this size should be fully
 *		  contained with a single logical block.
 *	cpp	- pointer to a cbuf pointer which is set with the
 *		  assign cbuf on success return.
 *
 * RETURN VALUES:
 *	0 	- success.
 *	EIO	- i/o error.
 */
int32
cmReadWR(inode_t *ip,
         int64 offset,
         int32 nbytes,
         cbuf_t **cpp)
{
	int32	rc;
	cbuf_t	*cp;
	cmdev_t	*cdp;
	int64	lblkno;
	int64	laddr;
	uint8	xflag;
	int64	xaddr;
	int32	xlen;
	int32	ipl;

	/* offset and nbytes must be for a single page */
	assert((offset & CM_OFFSET) + nbytes <= CM_BSIZE);

	/* get the page number associated with this offset */
	lblkno = offset >> CM_L2BSIZE;

   MMPHPrecmReadWR();                  /* (d201828) Performance trace hook */

	/* get the cbuf for the inode's cmctl and page number */
	cp = cmGet(ip->i_cacheid,lblkno,CM_WAIT|CM_WRITE);

	/* if the cbuf returned by cmGet() is initialized,
	 * just return it.
	 */
	if (!cp->cm_new)
	{
		*cpp = cp;
      rc = 0;
		goto TheEnd;
	}

	/* look up the backing extent for this logical block */
         laddr = lblkno << ip->i_ipmnt->i_l2nbperpage;
         if (rc = xtLookup(ip, laddr, ip->i_ipmnt->i_nbperpage,
			&xflag, &xaddr, &xlen, 0))
	 {
		cmPut(cp,FALSE);
		goto TheEnd;
	 }

	cp->j_ip = ip;

	/* is the page corresponds to a file hole ? */
	if (xlen == 0)
	{
		/* mark the cbuf as hole and
		 * return zero-filled buffer.
		 */
		assert(ip->i_mode & ISPARSE);

		cp->cm_new = FALSE;
		cp->cm_hole = TRUE;
		PXDaddress(&cp->cm_pxd,0);
		PXDlength(&cp->cm_pxd,0);

		bzero(cp->cm_cdata, CM_BSIZE);

		*cpp = cp;
		rc = 0;
		goto TheEnd;
	}

	/* logical block has a backing extent:
	 * update the cbuf with the extent;
	 */
	PXDaddress(&cp->cm_pxd, xaddr);
	PXDlength(&cp->cm_pxd, xlen);

	/* if the logical block is allocated but not recorded,
	 * mark the cbuf as abnr and return zero-filled buffer.
	 */
	if (xflag & XAD_NOTRECORDED)
	{
		cp->cm_new = FALSE;
		cp->cm_abnr = TRUE;
		bzero(cp->cm_cdata, CM_BSIZE);

		*cpp = cp;
		rc = 0;
		goto TheEnd;
	}

	/* if not all of the on-disk portion of the page is going
	 * to be overwritten, initialize the cbuf from disk.
	 */
	if ((offset & CM_OFFSET) ||
	    nbytes < (xlen << ip->i_ipmnt->i_l2bsize))
	{
		cp->cm_new = FALSE;

		/* queue the buffer to iolist */
		IOQUEUE_READ(cp);
		cp->cm_ionxt = cp;

		/* start the i/o */
		cdp = cp->cm_ctl->cc_dev;
		cmDeviceStartIO(cdp, cp);
	
		ipl = IOCACHE_LOCK();

		/* wait for the read to complete */
		if (!cp->cm_iodone)
		{
			IOCACHE_SLEEP_RC(&cp->cm_ioevent,T_NORELOCK,rc);
			IOCACHE_NOLOCK(ipl);
		}
		else
		{
			rc = (cp->cm_iobp->b_flags & B_ERROR) ? EIO : 0;
			IOCACHE_UNLOCK(ipl);
		}

		if (rc)
		{						// D228362
			cp->cm_new = TRUE;			// D228362
			cmPut(cp,FALSE);			// D228362
			goto TheEnd;
		}						// D228362
	}

	*cpp = cp;

TheEnd:

   MMPHPostcmReadWR();                 /* (d201828) Performance trace hook */

	return(rc);
}


/*
 * NAME:	cmKickStart()
 *
 * FUNCTION:	Performs readahead without waiting actually waiting for a
 *		buffer.  This is used to prefetch data when a file is open
 *		for read access.
 *
 * PARAMETERS:
 *	ip	- pointer to file's inode.
 *		  return with the pointer of the specified cbuf.
 */
void
cmKickStart(inode_t	*ip)
{
	int32	rc = 0;
	int64	lblkno;
	int64	laddr;
	uint8	xflag;
	int64	xaddr;
	int32	xlen;
	cbuf_t	*cp, *rahead, *ratail, *iotail;
	cmdev_t	*cdp;
	int32	ipl, wait_read;

jEVENT(0,("cmKickStart: ip:0x%x\n", ip));

   MMPHPrecmKickStart();

	ratail = 0;

	cmReadAhead(ip, -1, 0, 4, &ratail);

	/*
	 * read pages of pagein list from disk.
	 */
	if (ratail)
	{
		cdp = ratail->cm_ctl->cc_dev;

		/* start the i/o */
		cmDeviceStartIO(cdp, ratail);
	}
	else
		ip->i_rapage = 0;

   MMPHPostcmKickStart();
	return;
}


/*
 * NAME:	cmWrite()
 *
 * FUNCTION:	synchronously write modified cbufs to disk for a
 *		specified logical block range within a specified
 *		cmctl.
 *
 *		cmWriteCtl() is called to start i/o for the modified
 *		cbufs.
 *		
 *		cmWrite() should not be called to write inuse cbufs.
 *
 * PARAMETERS:
 *	ccp	- pointer to cmctl
 *	blkno   - starting logical block number within the cmctl of
 *		  range to be synchronously written to disk.
 *	nblks   - number of blocks in the range.
 *
 * RETURN VALUES:
 *	0	- success
 *	EIO	- i/o error
 */
int32
cmWrite(cmctl_t *ccp,
        int64 blkno,
        int64 nblks)
{
	int32	rc = 0;
	int32	ipl;
	cmdev_t	*cdp;
	cbuf_t	*iotail;

        MMPHPrecmWrite();       /* MMPH Performance Hook */

        ipl = IOCACHE_LOCK();

	/* write the modified cbufs within the specified range */
	iotail = cmWriteCtl(ccp, blkno, nblks, FALSE);		// D230860

	/* start i/o on the cache device */
	if (iotail)
	{
		IOCACHE_UNLOCK(ipl);

		cdp = iotail->cm_ctl->cc_dev;
		cmDeviceStartIO(cdp, iotail);

        	ipl = IOCACHE_LOCK();
	}

	/* wait for the writes to complete */
	assert(ccp->cc_iowait == EVENT_NULL);
	if (ccp->cc_iolevel)
	{
		IOCACHE_SLEEP_RC(&ccp->cc_iowait,T_NORELOCK,rc);
		IOCACHE_NOLOCK(ipl);
	}
	else
		IOCACHE_UNLOCK(ipl);

        MMPHPostcmWrite();      /* MMPH Performance Hook */

        return rc;
}


/*
 * NAME:	cmAsyncWrite()
 *
 * FUNCTION:	asynchronously write modified cbufs for a
 *		specified logical block range of the specified
 *		cmctl.
 *
 *		cmWriteCtl() is called to start i/o for the modified
 *		cbufs.
 *		
 *		cmAsyncWrite() should not be called to write inuse cbufs.
 *
 * PARAMETERS:
 *	ccp	- pointer to cmctl
 *	blkno   - starting logical block number within the cmctl of
 *		  range to be asynchronously written to disk.
 *
 *	nblks   - number of blocks in the range.
 *
 * RETURN VALUES: none.
 */
void
cmAsyncWrite(cmctl_t *ccp,
	     int64 blkno,
	     int64 nblks)
{
	int32 ipl;
	cmdev_t	*cdp;
	cbuf_t	*iotail;

   MMPHPrecmAsyncWrite();              /* (d201828) Performance trace hook */

        ipl = IOCACHE_LOCK();

	/* write the modified cbufs within the specified range */
	iotail = cmWriteCtl(ccp, blkno, nblks, TRUE);		// D230860

	IOCACHE_UNLOCK(ipl);

	/* start i/o on the cache device */
	if (iotail)
	{
		cdp = iotail->cm_ctl->cc_dev;
		cmDeviceStartIO(cdp, iotail);
	}

   MMPHPostcmAsyncWrite();             /* (d201828) Performance trace hook */
}


/*
 * NAME:	cmWriteCtl()
 *
 * FUNCTION:	write modified cbufs to disk for a specified logical block
 *	        range within a specified cmctl.
 *
 *		this routine searches for modified cbufs within the logical
 *		block range. for each cbuf found, the cbuf is removed from
 *		the cache and queued to the cache device if it is not
 *	   	currently in the process of i/o.  once all appropriate cbufs
 *		within the range are queued for i/o, i/o is started for
 *		the cache device.
 *
 *		no cbufs with the specified range should be in an inuse
 *		state.
 *
 * PARAMETERS:
 *	ccp	- pointer to cmctl
 *	blkno   - starting logical block number within the cmctl of
 *		  range to be asynchrounously written to disk.
 *	nblks   - number of blocks in the range.
 *	async	- TRUE if asynchronous write			D230860
 *
 * RETURN VALUES: none.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
static cbuf_t *cmWriteCtl(
	cmctl_t *ccp,
	int64 blkno,
	int64 nblks,
	int32 async)						// D230860
{
	int32	ipl;
	cbuf_t	*cp, *cpnext, *iotail;
        int64	last, b;

	/* must not be in the process of being deleted */
	assert(!ccp->cc_delpend1);

	/* any work to do ? */
        if (nblks <= 0)
                return;

	/* compute the last logical block of the range */
	last = blkno + nblks - 1;

	iotail = NULL;

	/* if the number of cbufs to be written is small
	 * in comparision with the total number of cbufs for this
	 * cmctl, locate the cbufs of the range using the cbuf hash.
	 * otherwise, find the cbufs by scanning the cmctl's cbuf list.
	 */
        if (nblks < ccp->cc_ncbufs >> 3)
        {
		/* number of cbufs is small: loop over the specified range.
		 */
                for (b = blkno; b <= last; b++)
                {
			/* check if the cbuf for this logical blocks
			 * is in the hash.
			 */
                        if ((cp = cmLookup(ccp,b)) != NULL)
			{
				/* must not be inuse to be written to disk */
				assert(!cp->cm_inuse);

				/* ignore if in i/o state */
				if (cp->cm_read || cp->cm_write)
					continue;

				/* ignore if NOT modified */
				if (!cp->cm_modified)
					continue;

#ifdef	_JFS_HEURISTICS_SLRU
				cp->cm_hit = FALSE;
				if (cp->cm_probationary)
				{
					if (cp == cachemgr.slruxp)
						cachemgr.slruxp = cp->cm_cachelist.prev;
				}
				else
					cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
				/* remove the cbuf from the cachelist */
				CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

				if (async)			// D230860
					IOQUEUE_ASYNC_WRITE(cp)	// D230860
				else				// D230860
					IOQUEUE_WRITE(cp)

				/* append at tail of device iolist */
				if (iotail)
				{
					cp->cm_ionxt = iotail->cm_ionxt;
					iotail->cm_ionxt = cp;
				}
				else
					cp->cm_ionxt = cp;
				iotail = cp;
#ifdef	_JFS_WIP
				/*
				 * build coalesced list of cbufs now
				 * vs. cmCoalesce() at cmDeviceStartIO() time ?
				 */
#endif	/* _JFS_WIP */

				ccp->cc_iolevel++;
			}
                }
        }
	else
	{
		/* number of cbufs is large: loop over the cmctl's
		 * cbuf list to find cbufs within the specified range.
		 */
        	for (cp = ccp->cc_cbuflist.head; cp != (cbuf_t *)&ccp->cc_cbuflist; cp = cpnext)
        	{
			/* get next pointer now since we may modify
			 * the cmctl's cbuf list below.
			 */
                	cpnext = cp->cm_pagelist.next;

			/* is this cbuf within the specified range ? */
                	if (cp->cm_blkno >= blkno && cp->cm_blkno <= last)
			{
				nblks--;

				/* must not be inuse to be written to disk */
				assert(!cp->cm_inuse);

				/* ignore if in i/o state */
				if (cp->cm_read || cp->cm_write)
					continue;

				/* ignore if NOT modified */
				if (!cp->cm_modified)
					continue;

#ifdef	_JFS_HEURISTICS_SLRU
				cp->cm_hit = FALSE;
				if (cp->cm_probationary)
				{
					if (cp == cachemgr.slruxp)
						cachemgr.slruxp = cp->cm_cachelist.prev;
				}
				else
					cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
				/* remove the cbuf from the cachelist */
				CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

				if (async)			// D230860
					IOQUEUE_ASYNC_WRITE(cp)	// D230860
				else				// D320860
					IOQUEUE_WRITE(cp)

				/* append at tail of device iolist */
				if (iotail)
				{
					cp->cm_ionxt = iotail->cm_ionxt;
					iotail->cm_ionxt = cp;
				}
				else
					cp->cm_ionxt = cp;
				iotail = cp;
#ifdef	_JFS_WIPOS2
				/*
				 * build coalesced list of cbufs now
				 * vs. cmCoalesce() at cmDeviceStartIO() time ?
				 */
#endif	/* _JFS_WIPOS2 */

				ccp->cc_iolevel++;

				if (nblks == 0)
					break;
			}
        	}
	}

	return iotail;
}


/*
 *	cmFlush()
 *
 * synchronously write all dirty pages
 */
int32
cmFlush(cmctl_t *ccp)
{
	int32	rc = 0;
	int32	ipl;
	cbuf_t	*cp, *cpnext, *iotail;
	cmdev_t	*cdp;
	int32	dirty = 1;	/* _JFS_DEBUG */

        MMPHPrecmFlush();       /* MMPH Performance Hook */

        ipl = IOCACHE_LOCK();

	iotail = NULL;

	/* loop over the cmctl's cbuf list to find dirty pages */
        for (cp = ccp->cc_cbuflist.head; cp != (cbuf_t *)&ccp->cc_cbuflist; cp = cpnext)
        {
		/* get next pointer now since we may modify
		 * the cmctl's cbuf list below.
		 */
               	cpnext = cp->cm_pagelist.next;

		/* must not be inuse to be written to disk */
		assert(!cp->cm_inuse);

		/* stop if NOT modified */
		if (!cp->cm_modified)
#ifdef	_JFS_DEBUG
		{
			dirty = 0;
			continue;
		}
		else if (dirty == 0)
		{
			printf("cmFlush: dirty page on clean segment\n");
			brkpoint1(cp);
		}
#else
			break;
#endif	/* _JFS_DEBUG */

		/* ignore if in i/o state */
		if (cp->cm_write)
			continue;

#ifdef	_JFS_HEURISTICS_SLRU
		cp->cm_hit = FALSE;
		if (cp->cm_probationary)
		{
			if (cp == cachemgr.slruxp)
				cachemgr.slruxp = cp->cm_cachelist.prev;
		}
		else
			cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
		/* remove the cbuf from the cachelist */
		CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

		IOQUEUE_WRITE(cp);

		/* append at tail of device iolist */
		if (iotail)
		{
			cp->cm_ionxt = iotail->cm_ionxt;
			iotail->cm_ionxt = cp;
		}
		else
			cp->cm_ionxt = cp;
		iotail = cp;
#ifdef	_JFS_WIP
		/*
		 * build coalesced list of cbufs now
		 * vs. cmCoalesce() at cmDeviceStartIO() time ?
		 */
#endif	/* _JFS_WIP */

		ccp->cc_iolevel++;
	}

	/* start i/o on the cache device */
	if (iotail)
	{
		IOCACHE_UNLOCK(ipl);

		cdp = ccp->cc_dev;
		cmDeviceStartIO(cdp, iotail);

        	ipl = IOCACHE_LOCK();
	}

	/* wait for the writes to complete */
	assert(ccp->cc_iowait == EVENT_NULL);
	if (ccp->cc_iolevel)
	{
		IOCACHE_SLEEP_RC(&ccp->cc_iowait,T_NORELOCK,rc);
		IOCACHE_NOLOCK(ipl);
	}
	else
		IOCACHE_UNLOCK(ipl);

   MMPHPostcmFlush();      /* MMPH Performance Hook */

	return rc;
}


/*
 * NAME:	cmRelease()
 *
 * FUNCTION:	flush cbufs for a specified cmctl;
 *
 * PARAMETERS:
 *	ccp	- pointer to cmctl
 */
void cmRelease(cmctl_t	*ccp)
{
	int32 ipl;
	cbuf_t	*cp, *cpnext;

        ipl = IOCACHE_LOCK();

        for (cp = ccp->cc_cbuflist.head; cp != (cbuf_t *)&ccp->cc_cbuflist; cp = cpnext)
        {
		/* get next pointer now since we may release
		 * the cbuf below.
		 */
               	cpnext = cp->cm_pagelist.next;

		/* must not be inuse if it is being released */
		assert(!cp->cm_inuse);

		/* if the cbuf is in an i/o state,
		 * mark it as discard so that it
		 * will be released when the i/o completes.
		 */
		if (cp->cm_read || cp->cm_write)
		{
			assert(cp->cm_ioevent == EVENT_NULL);
			cp->cm_discard = TRUE;
		}
		/* cbuf is in the cache: */
		else
		{
#ifdef	_JFS_HEURISTICS_SLRU
			if (cp->cm_probationary)
			{
				if (cp == cachemgr.slruxp)
					cachemgr.slruxp = cp->cm_cachelist.prev;
			}
			else
				cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
			/* remove from cachelist */
			CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

			/* release the cbuf:
			 * remove from hashlist,
			 * remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(cp);
		}
	}

        IOCACHE_UNLOCK(ipl);
}


/*
 * NAME:	cmTruncate()
 *
 * FUNCTION:	truncate (release) a specified byte range within an
 *		an inode (cmctl).
 *
 *		cmTruncate() is called to support file truncation
 *		and provides logical truncation of a cmctl, releasing
 *		cbufs with the specified byte range.  if the byte
 *		range starts in the middle of a cbuf, the cbuf will
 *		not be released, but will be truncated.  this means
 *		that the truncated portion of the cbuf will be zeroed
 *		and the cbuf's disk address will be adjusted to reflect
 *		the truncation (i.e. it will be adjusted such that it
 *		covers only the portion of the cbuf (file) that will be
 *		kept).
 *
 * PARAMETERS:
 *	ip	- pointer to inode
 *	offset  - starting byte offset of the range to be released.
 *	len     - number of bytes to release.
 *
 * RETURN VALUES:
 *	0	- success.
 *	EIO	- i/o error.
 *	ENOSPC	- insufficient disk resources.
 */
cmTruncate(inode_t *ip,
	   int64 offset,
	   int64 len)
{
	int32 ipl, nbzero, rc;
	int64 blkno, nblks;
	cmctl_t *ccp;
	uint32 coff;
	cbuf_t *cp;

	/* pick up cmctl of the file */
	ccp = (cmctl_t *)ip->i_cacheid;
	ASSERT(ccp != NULL);

	/* any work to do ? */
	ASSERT(len >= 0);
	if (len == 0)
		return(0);

	/* release all ? */
	if (offset == 0)
	{
		cmRelease(ccp);
		return 0;
	}

	/*
	 *	truncate partial page if necessary
	 */
	/* if the truncation range starts in the middle of a page,
	 * zero the portion of the page within the range and
	 * truncate page's disk address to reflect the truncation.
	 */
	if (coff = (offset & CM_OFFSET))
	{
		/* determine how many bytes are being released from
		 * the cbuf.
		 */
		nbzero = MIN(CM_BSIZE - coff, len);

		/* read up the cbuf */
		if (rc = cmRead(ip, offset, -1, &cp))
			return(rc);

		/* if the cbuf represents a file hole, fill it */
		if (cp->cm_hole == TRUE)
		{
			/* must be sparse file */
			assert(ISSPARSE(ip));
                        if (rc = extFill(ip,cp))
                        {
                        	cmPut(cp,FALSE);
                        	return(rc);
                        }
		}
		else
		{
			/* if not ABNR (or a file hole), zero fill the
		 	 * portion of the buffer that is being truncated.
		 	 */
			if (cp->cm_abnr == FALSE)
				bzero(cp->cm_cdata+coff,nbzero);
			
		}

		/* truncate the cbuf's disk address to reflect the
		 * portion being released.
		 */
		nblks = CM_BTOBLKS(coff,ip->i_ipmnt->i_bsize,ip->i_ipmnt->i_l2bsize);
		PXDlength(&cp->cm_pxd,nblks);

		/* put the cbuf, marking it as modified if not ABNR.
		 */
		cmPut(cp,(cp->cm_abnr == FALSE) ? TRUE : FALSE);

		/* account for the work we have done against the
		 * range.
		 */
		len -= nbzero;
		offset += nbzero;

		/* are we done ? */
		if (len == 0)
			return(0);
	}

	/*
	 *	release whole pages truncated
	 */
	/* get the starting logical block number and number of logical
	 * blocks to be released;
	 */
	blkno = offset >> CM_L2BSIZE;
	nblks = CM_BTOCBLKS(len);

        ipl = IOCACHE_LOCK();

	/* must not be in the process of being deleted */
	assert(!ccp->cc_delpend1);

	cmRelCtl(ccp,blkno,nblks);

        IOCACHE_UNLOCK(ipl);

	return(0);
}


/*
 * NAME:	cmRelCtl()
 *
 * FUNCTION:	release cbufs for a specified logical block range within
 *		a specified cmctl.
 *
 *		this routine searches for cbufs within the logical block
 *		range. for each cbuf found, the cbuf is release (removed
 *		from the cache and placed on the free list) if it is not
 *		in the process of i/o. if it is in the process of i/o, the
 *		cbuf is marked as discard and will be placed on the free
 *		list when the i/o completes. no cbufs with the specified
 *		range should be in an inuse state.
 *
 * PARAMETERS:
 *	ccp	- pointer to cmctl
 *	blkno   - starting logical block number within the cmctl of
 *		  range to be released.
 *	nblks   - number of blocks in the range.
 *
 * RETURN VALUES: none.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
static void
cmRelCtl(cmctl_t *ccp,
	  int64 blkno,
	  int64 nblks)
{
	cbuf_t	*cp, *cpnext;
        int64 last, b;

	/* any work to do ? */
	if (nblks <= 0)
		return;

	/* compute the last logical block of the range */
	last = blkno + nblks - 1;

	/* if the number of cbufs to be released is small
	 * in comparision with the total number of cbufs for this
	 * cmctl, locate the cbufs of the range using the cbuf hash.
	 * otherwise, find the cbufs by scanning the cmctl's cbuf list.
	 */
        if (nblks < ccp->cc_ncbufs >> 3)
        {
		/* number of cbufs is small: loop over the specified range.
		 */
                for (b = blkno; b <= last; b++)
                {
			/* check if the cbuf for this logical blocks
			 * is in the hash.
			 */
                        if ((cp = cmLookup(ccp,b)) != NULL)
			{
				/* must not be inuse if it is being released */
				assert(!cp->cm_inuse);

				/* if the cbuf is in an i/o state,
				 * mark it as discard so that it will
				 * be released when the i/o completes.
				 */
				if (cp->cm_read || cp->cm_write)
				{
					assert(cp->cm_ioevent == EVENT_NULL);
					cp->cm_discard = TRUE;
				}
				/* cbuf is in the cache: */
				else
				{
#ifdef	_JFS_HEURISTICS_SLRU
					if (cp->cm_probationary)
					{
						if (cp == cachemgr.slruxp)
							cachemgr.slruxp = cp->cm_cachelist.prev;
					}
					else
						cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
					/* remove from cachelist */
					CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

					/* release the cbuf:
					 * remove from hashlist,
					 * remove from pagelist, and
					 * insert at head of freelist;
					 */
                        		cmRelCbuf(cp);
				}
			}
                }
        }
	else
	{
		/* number of cbufs is large: loop over the cmctl's
		 * cbuf list to find cbufs within the specified range.
		 */
        	for (cp = ccp->cc_cbuflist.head; cp != (cbuf_t *)&ccp->cc_cbuflist; cp = cpnext)
        	{
			/* get next pointer now since we may release
			 * the cbuf below.
			 */
                	cpnext = cp->cm_pagelist.next;

			/* is this cbuf within the specified range ?
			 */
                	if (cp->cm_blkno >= blkno && cp->cm_blkno <= last)
			{
				/* must not be inuse if it is being released */
				assert(!cp->cm_inuse);

				/* if the cbuf is in an i/o state,
				 * mark it as discard so that it
				 * will be released when the i/o completes.
				 */
				if (cp->cm_read || cp->cm_write)
				{
					assert(cp->cm_ioevent == EVENT_NULL);
					cp->cm_discard = TRUE;
				}
				/* cbuf is in the cache: */
				else
				{
#ifdef	_JFS_HEURISTICS_SLRU
					if (cp->cm_probationary)
					{
						if (cp == cachemgr.slruxp)
							cachemgr.slruxp = cp->cm_cachelist.prev;
					}
					else
						cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
					/* remove from cachelist */
					CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

					/* release the cbuf:
					 * remove from hashlist,
					 * remove from pagelist, and
					 * insert at head of freelist;
					 */
                        		cmRelCbuf(cp);
				}
			}
        	}
	}	
}


/*
 * NAME:	cmLookup()
 *
 * FUNCTION:	Lookup a specified cbuf in the cbuf hash.
 *
 * PARAMETERS:
 *	ccp	- pointer to the cmctl
 *	blkno	- logical block number within the cmctl
 *
 * RETURN VALUES:
 *	pointer to the cbuf if found in the hash or NULL if not found.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
static cbuf_t *
cmLookup(cmctl_t *ccp,
         int64 blkno)
{
        chash_t *chp;
        cbuf_t *cp;

	/* get the hash chain header */
	chp = CM_HASH(ccp, blkno);

	/* scan the hash chain for the specified cbuf */
        for (cp = chp->ch_hashlist.head; cp; cp = cp->cm_hashlist.next)
        {
                if (cp->cm_blkno == blkno && cp->cm_ctl == ccp)
			break;
	}

	return(cp);
}


/*
 * NAME:	cmGet()
 *
 * FUNCTION:	get a cbuf for specified cmctl and logical block number.
 *
 *		the cbuf hash is searched for the specified cbuf. if found
 *		in the hash, the existing cbuf (pointer) is returned.
 *		otherwise, a new cbuf is allocated and place in the hash
 *		and on the cmctl's cbuf list, with the new cbuf (pointer)
 *		returned.
 *
 *		all returned cbufs are in an inuse state and those that have
 *		been newly allocated are also marked as	new.
 *
 * PARAMETERS:
 *	ccp	- pointer to the cmctl
 *	blkno	- logical block number within the cmctl
 *	wait	- True indicates should wait for buffer
 *
 * RETURN VALUES:
 *	pointer to the specified cbuf.
 */
static cbuf_t *
cmGet(cmctl_t *ccp,
      int64 blkno,
      uint32 flag)
{
	int32	ipl, timestamp;
	chash_t *chp;
	cbuf_t	*cp, *head_ra;

#ifdef MMIOPH
   int32 Hit = 1;                      /* (d201828) cache hit (1) or miss (0) */
#endif
   MMPHPrecmGet();                     /* (d201828) MMPH Performance Hook */

	/* get the hash chain header */
	chp = CM_HASH(ccp, blkno);

	ipl = IOCACHE_LOCK();

	/* must not be in the process of being deleted */
	assert(!ccp->cc_delpend1);

tryReclaim:
	/*
	 * search the hash chain for the cbuf
	 */
	for (cp = chp->ch_hashlist.head; cp; cp = cp->cm_hashlist.next)
	{
		/* check cmctl and page number for a match */
 		if (cp->cm_ctl != ccp ||
		    cp->cm_blkno != blkno)
			continue;

		/*
		 * 	cache hit:
		 */
		/* if the cbuf is inuse,
		 * wait for it to change and try again.
		 */
		if (cp->cm_inuse)
		{
			if (flag & CM_WAIT)
			{
				IOCACHE_SLEEP(&cp->cm_event,0);
				goto tryReclaim;
			}
			else
				goto dont_wait;
		}

		/* if the cbuf is in an i/o state,
		 * wait for the i/o to complete and try again.
		 */
		if (cp->cm_read || cp->cm_write)
		{
			if (flag & CM_WAIT)
			{
				IOCACHE_SLEEP(&cp->cm_ioevent,0);
				goto tryReclaim;
			}
			else
				goto dont_wait;
		}

		/* remove the cbuf from the cachelist */
#ifdef	_JFS_HEURISTICS_SLRU
		cp->cm_hit = TRUE;
		if (cp->cm_probationary)
		{
			if (cp == cachemgr.slruxp)
				cachemgr.slruxp = cp->cm_cachelist.prev;
		}
		else
			cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
		CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

		/*
		 * if buffer on clean segment (cp->cm_modified == FALSE)
		 * is checked out for write (flag & CM_WRITE),
		 * it will be moved from clean segment to dirty segment
		 * at cmPut() time;
		 */
		goto done;
	}

	/*
	 *	cache miss:
	 */
tryRecycle:
	timestamp = chp->ch_timestamp;

#ifdef MMIOPH
   Hit = 0;                            /* (d201828) */
#endif

	/* allocate a new cbuf.
	 * cmRecycle() return NULL if it had to wait to get a new cbuf.
	 * in this case, we'll call it again until it returns a cbuf
	 * (non-NULL) or unless the hash chain timestamp value has
	 * changed.  if the timestamp has change, it
	 * means that something has been inserted and we have to do a
	 * rescan of the hash to ensure that what was inserted is not
	 * what we are trying to create.
	 */
	while ((cp = cmRecycle(flag & CM_WAIT, &ipl)) == NULL)
	{
		if (flag & CM_WAIT)
		{
			if (timestamp != chp->ch_timestamp)
				goto tryReclaim;
		}
		else
			goto dont_wait;
	}

	/* bind cbuf to the page of cmctl */
	cp->cm_ctl = ccp;
	cp->cm_blkno = blkno;

	/* insert at head of hashlist */
	HASHLIST_INSERT_HEAD(chp, cp, cm_hashlist);

	/* insert into pagelist */
	if (flag & CM_READ)
	{
		PAGELIST_INSERT_TAIL(&ccp->cc_cbuflist, cp, cm_pagelist);
	}
	else
		PAGELIST_INSERT_HEAD(&ccp->cc_cbuflist, cp, cm_pagelist);

	/* mark the cbuf it was newly allocated */
	cp->cm_new = TRUE;
	
done:
	/* mark the cbuf as inuse */
	ASSERT(!cp->cm_inuse);
	ASSERT(!cp->cm_read);
	ASSERT(!cp->cm_write);
	ASSERT(!cp->cm_discard);
	ASSERT(!cp->cm_cached);

	cp->cm_inuse = TRUE;

	IOCACHE_UNLOCK(ipl);

   MMPHPostcmGet();                    /* (d201828) MMPH Performance Hook */
	return(cp);

dont_wait:
	IOCACHE_UNLOCK(ipl);

   MMPHPostcmGet();                    /* (d201828) MMPH Performance Hook */
	return(NULL);
}


/*
 * NAME:	cmGetE()
 *
 * FUNCTION:	allocate a cbuf for a new page for write.
 *
 * PARAMETERS:
 *	ccp	- pointer to the cmctl
 *	blkno	- logical block number within the cmctl
 *
 * RETURN VALUES:
 *	cp	- pointer to a newly inserted cbuf
 *
 * serialization: IWRITE_LOCK held on entry/exit;
 */
cbuf_t *
cmGetE(cmctl_t *ccp,
	int64 blkno,
	uint32	flag)
{
	int32	ipl;
	chash_t *chp;
	cbuf_t	*cp, *cp2;

   MMPHPrecmGetE();                    /* (d201828) Performance trace hook */

	ASSERT(flag & CM_WRITE);

	/* get the hash chain header */
	chp = CM_HASH(ccp, blkno);

	ipl = IOCACHE_LOCK();

	/* must not be in the process of being deleted */
	assert(!ccp->cc_delpend1);

	/* allocate a free cbuf */
	while ((cp = cmRecycle(CM_WAIT, &ipl)) == NULL)
		;

	/* init the cbuf with the cmctl and logical block number */
	cp->cm_ctl = ccp;
	cp->cm_blkno = blkno;

	/* insert at head of hashlist */
	HASHLIST_INSERT_HEAD(chp, cp, cm_hashlist);

	/* insert at head of pagelist */
	PAGELIST_INSERT_HEAD(&ccp->cc_cbuflist, cp, cm_pagelist);

	/* mark it as new and inuse */
	cp->cm_new = TRUE;
	cp->cm_inuse = TRUE;

	IOCACHE_UNLOCK(ipl);

   MMPHPostcmGetE();                   /* (d201828) Performance trace hook */

	return(cp);
}


/*
 * NAME:	cmRecycle()
 *
 * FUNCTION:	allocate a new cbuf from the free list.
 *
 *		if the size of the cache manager's cbuf free list is
 *		small, cbufs will be stolen from the cbuf cache list
 *		to replenish the free list.
 *
 * PARAMETERS:
 *	wait	- boolean indicating whether the caller wants to wait
 *		  for a free cbuf.
 *
 * RETURN:
 *	pointer to a newly allocated cbuf or NULL if the allocation
 *	required waiting.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
cbuf_t *
cmRecycle(int32 wait,
	int32	*ipl)
{
	cbuf_t	*cp, *devtail = 0;
	int32	nb, i;

	/*
	 * try to keep free buffers availble:
	 *
	 * check if the free list needs replenishing from the cache
	 * list. we'll replenish the free list if the number of cbufs
	 * on the free list plus the number of cbufs being written to
	 * disk for the purpose of replacement (i.e. heading for the
	 * free list) is less than or equal to minfree.
	 */
	if (cachemgr.nfreecbufs + cachemgr.numiolru <= cachemgr.minfree)
	{
		/* compute the number of cbufs to recycle from the cache
		 * list to keep the number of cbufs on the freelist
		 * or heading for the freelist to be maxfree.
		 */
		nb = cachemgr.maxfree - cachemgr.nfreecbufs -
						cachemgr.numiolru;

		/*
		 * recycle from the head of the cachelist.
		 */
		for (i = 0, cp = cachemgr.cachelist.head;
		     i < nb && cp != (cbuf_t *) &cachemgr.cachelist;
		     i++, cp = cachemgr.cachelist.head)
		{
			/* remove the cbuf from the cachelist */
#ifdef	_JFS_HEURISTICS_SLRU
			if (cp->cm_probationary)
			{
				if (cp == cachemgr.slruxp)
					cachemgr.slruxp = cp->cm_cachelist.prev;
			}
			else
				cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
			CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);

			/* if the cbuf is modified, queue it to
			 * the cache device iolist for lru-write so
			 * it will be inserted into freelist
			 * when write is completed.
			 */
			if (cp->cm_modified)
				cmDeviceQueueIO(cp, CM_WRITE|CM_LRU);
			/* otherwise, release the cbuf:
			 * remove from hashlist,
			 * remove from pagelist, and
			 * insert at head of freelist;
			 */
			else
				cmRelCbuf(cp);
		}

		/* if we queued i/o, get it started */
		if (cachemgr.devtail != NULL)
		{
			devtail = cmQueueIO();

			IOCACHE_UNLOCK(*ipl);

			cmStartIO(devtail);

			*ipl = IOCACHE_LOCK();
		}
	}

	/*
	 * if the freelist is empty, wait if required.
	 */
        if (cachemgr.nfreecbufs == 0)
	{
		if (wait)
		{
			IOCACHE_SLEEP(&cachemgr.freewait,0);
		}

		return(NULL);
	}
	/*
	 * If we initiated I/O, we released the IOCACHE_LOCK, therefore
	 * return zero.
	 */
	if (devtail)
		return(NULL);

	/*
	 * remove a cbuf from the freelist.
	 */
	cp = cachemgr.freelist;
	cachemgr.freelist = cp->cm_freenxt;
        cachemgr.nfreecbufs--;

	cp->cm_flag = 0;
#ifdef	_JFS_HEURISTICS_SLRU
	/* cp->cm_hit = FALSE; */
#endif	/* _JFS_HEURISTICS_SLRU */
	cp->cm_event = EVENT_NULL;

	return(cp);
}


/*
 * NAME:	cmPut()
 *
 * FUNCTION:	put an inuse cbuf, marking its modification state
 *		as specified.
 *
 *		if specified (dirty is TRUE), the cbuf is marked
 *		as modified.  the cbuf is marked as no longer inuse.
 *
 *		if the cbuf is currently marked as new, it will be
 *		released unless it is also marked as modified.
 *		if modified, the new state is cleared.
 *
 *		if the cbuf is not to be released, it will be placed
 *		in the cbuf cache list.
 *
 * PARAMETERS:
 *	cp	- pointer to the cbuf to be put.
 *	dirty	- indication of whether the cbuf should be marked as
 *		  modified.
 *
 * RETURN VALUES: none.
 */
void
cmPut(cbuf_t *cp,
      int32  dirty)
{
	int32 ipl;
	cbuf_t	*nxp;
	cmctl_t	*ccp;

   MMPHPrecmPut();                     /* (d201828) Performance trace hook */
	
	ipl = IOCACHE_LOCK();

	/* must not be in the process of being deleted */
	ccp = cp->cm_ctl;
	ASSERT(cp->cm_inuse);
	ASSERT(!ccp->cc_delpend1);

	cp->cm_inuse = FALSE;

	/* should the cbuf be marked as dirty ? */
	if (dirty)
	{
		/* if buffer is on clean segment of pagelist,
		 * move to head of dirty segment
		 */
		if (cp->cm_modified == FALSE)
		{
			CDLL_MOVE_TO_HEAD(&ccp->cc_cbuflist, cp, cm_pagelist);

			cp->cm_modified = TRUE;
		}
	}

	/* if the cbuf is marked as modified, it should not be marked as new.
	 * (a cbuf is marked as new if the cbuf was acquired due to
	 * cache miss (cmGet()) or empty buffer was requested (cmGetE()).
	 * the new bit may persist to cmPut() if something has gone wrong and
	 * the cbuf should be released if it is not modified.
	 */
	if (cp->cm_modified)
		cp->cm_new = FALSE;

	/* if the cbuf is (still) marked as new, release the cbuf:
	 * remove from hashlist, remove from pagelist, and
	 * insert at head of freelist;
	 */
	if (cp->cm_new)
	{
		assert(cp->cm_event == EVENT_NULL);
		cmRelCbuf(cp);
	}
	/* otherwise, insert at tail of the cachelist */
	else
	{
#ifdef	_JFS_HEURISTICS_SLRU
		/* if page has been cache hit, insert at tail of protected segment */
		if (cp->cm_hit)
		{
			cp->cm_probationary = FALSE;

			/* is protected segment full ? */
			if (cachemgr.slrun >= cachemgr.slruN)
			{
				/* slide xp one over protected segment */
				if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
					nxp = cachemgr.slruxp->cm_cachelist.next;
				else
					nxp = cachemgr.cachelist.head;

				nxp->cm_probationary = TRUE;
				cachemgr.slruxp = nxp;
			}
			else
				cachemgr.slrun++;
	
			/* insert bp at tail of protected segment */
			CACHELIST_INSERT_TAIL(&cachemgr.cachelist, cp, cm_cachelist);
		}
		/* if page has been cache miss, insert at tail of probationary segment */
		else
		{
			cp->cm_probationary = TRUE;

			/* insert cp at tail of provationary segment as new xp */
			if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
				CACHELIST_INSERT_AFTER(&cachemgr.cachelist, cachemgr.slruxp, cp, cm_cachelist)
			else
				CACHELIST_INSERT_HEAD(&cachemgr.cachelist, cp, cm_cachelist);
			cachemgr.slruxp = cp;
		}
#else
		CACHELIST_INSERT_TAIL(&cachemgr.cachelist, cp, cm_cachelist);
#endif	/* _JFS_HEURISTICS_SLRU */

		/* wakeup any thread waiting for a free buf */
		if (cachemgr.freewait != EVENT_NULL)
		{
			IOCACHE_WAKEUP(&cachemgr.freewait);
		}

		if (cp->cm_event != EVENT_NULL)
		{
                	IOCACHE_WAKEUP(&cp->cm_event);
		}
	}
	
	IOCACHE_UNLOCK(ipl);

   MMPHPostcmPut();                    /* (d201828) Performance trace hook */
}


/*
 * NAME:	cmRelCbuf()
 *
 * FUNCTION:	release a cbuf, insert it at head of the freelist.
 *
 *		the cbuf is removed from the cbuf hash and the owning
 *		cmctl's cbuf list. it is then placed on the cache
 *		manager's cbuf free list.
 *
 *		this routine should not be called for cached or inuse
 *		cbufs.
 *		
 * PARAMETERS:
 *	cp	- pointer to cbuf to be released.
 *
 * RETURN: none.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
void
cmRelCbuf(cbuf_t *cp)
{
        ASSERT(!cp->cm_inuse);
        ASSERT(!cp->cm_read);
        ASSERT(!cp->cm_write);
        ASSERT(!cp->cm_cached);
	
	/*
	 * remove the cbuf from the pagelist
	 */
	if (cp->cm_metadata)
	{
#ifdef _JFS_LAZYCOMMIT
		/*
		 * We can't add to free list until all tlocks are cleaned up
		 * and nohomeok goes to zero
		 */
		if (cp->j_nohomeok)				// D230860
			return;					// D230860
#endif /* _JFS_LAZYCOMMIT */

  		CDLL_REMOVE(&cp->cm_ctl->cc_jbuflist, cp, cm_pagelist); //D226403
	}
	else
	{
		PAGELIST_REMOVE(&cp->cm_ctl->cc_cbuflist, cp, cm_pagelist);
	}
        PAGELIST_SELF(cp, cm_pagelist);

	/*
	 * remove the cbuf from the hashlist
	 */
	HASHLIST_REMOVE(cp, cm_hashlist);
        HASHLIST_SELF(cp, cm_hashlist);

	/*
	 * insert the cbuf at the head of the freelist
	 */
        cp->cm_free = TRUE;
        cp->cm_freenxt = cachemgr.freelist;
        cachemgr.freelist = cp;
        cachemgr.nfreecbufs++;

	/* wake up any threads waiting for a free cbuf */
	if (cachemgr.freewait != EVENT_NULL)
	{
                IOCACHE_WAKEUP(&cachemgr.freewait);
	}
}


/*
 * NAME:	cmFreeCbuf()
 *
 * FUNCTION:	return cbuf to freelist
 *
 * PARAMETERS:
 *	cp	- pointer to cbuf to be returned to freelist;
 *
 * RETURN: none.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
void cmFreeCbuf(cbuf_t *cp)
{
	cp->cm_flag = 0;

	/*
	 * insert the cbuf at the head of the freelist
	 */
        cp->cm_free = TRUE;
        cp->cm_freenxt = cachemgr.freelist;
        cachemgr.freelist = cp;
        cachemgr.nfreecbufs++;

	/* wake up any threads waiting for a free cbuf */
	if (cachemgr.freewait != EVENT_NULL)
	{
                IOCACHE_WAKEUP(&cachemgr.freewait);
	}
}


/*
 * NAME:	cmDeviceQueueIO()
 *
 * FUNCTION:	queue a cbuf to a cache device for i/o.
 *
 *		based upon the specified iotype, the cbuf is marked for
 *		read or write and moved to the i/o portion of the cmctl's
 *		cbuf list.  if the cbuf's cache device (cmdev) is not on
 *		the cache manager's queued device list, it is placed on
 *		the tail of this list.  the cbuf is then placed on the
 *		tail of the cmdev's list of cbufs queued for i/o.
 *
 * PARAMETERS:
 *	cp	- pointer to cbuf to be queued for i/o
 *	iotype	- type of i/o operation
 *
 * RETURN VALUES: none.
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
void
cmDeviceQueueIO(cbuf_t *cp,
	  int32 iotype)
{
	cmdev_t	*cdp, *devtail;
	cbuf_t	*iotail;

	ASSERT(!cp->cm_new);

	/* get the cache device of the cbuf */
	cdp = cp->cm_ctl->cc_dev;

	/*
	 *	read:
	 */
	if (iotype & CM_READ)
	{
		cp->cm_read = TRUE;
	}
	/*
	 *	write:
	 */
	else
	{
		ASSERT(cp->cm_modified);
		ASSERT(!cp->cm_inuse);

		cp->cm_write = TRUE;

		/* increment cmctl write count */
		if (cp->cm_metadata)
		{
			cp->cm_ctl->cc_jiolevel++;
			cdp->cd_iolevel++;
		}
		else
			cp->cm_ctl->cc_iolevel++;

		/* if the cbuf is being written as part of cbuf replacement,
		 * mark the cbuf, and
		 * increment the count of lru'ed cbufs with i/o pending.
		 */
		if (iotype & CM_LRU)
		{
			cp->cm_lru = TRUE;
			cachemgr.numiolru += 1;
		}
	}

	/* init the ioevent wait anchor. thread will wait on this
	 * event if they attempt to access the cbuf while it is in
	 * i/o.
	 */
	cp->cm_ioevent = EVENT_NULL;
	cp->cm_iodone = FALSE;

	/*
	 * insert the cbuf at the tail of the cache device's iolist
	 */
	if ((iotail = cdp->cd_iotail) != NULL)
	{
		cp->cm_ionxt = iotail->cm_ionxt;
		iotail->cm_ionxt = cp;
	}
	else
	{
		cp->cm_ionxt = cp;
	}
	cdp->cd_iotail = cp;

	/* if the cache device does NOT have any queued i/o,
	 * insert the cache device at the tail of the iolist of cache
	 * devices with pending i/o.
	 */
	if (cdp->cd_nextdev == NULL)
	{
		/* place the cache device at the tail of the
		 * cache device list.
		 */
		if ((devtail = cachemgr.devtail) != NULL)
		{
			cdp->cd_nextdev = devtail->cd_nextdev;
			devtail->cd_nextdev = cdp;
		}
		else
		{
			cdp->cd_nextdev = cdp;
		}
		cachemgr.devtail = cdp;
	}
}


/*
 *	cmQueueIO()
 *
 * privatize list (via cm_devnxt) of list (via cm_ionxt) of buffers to i/o;
 * accumumated via cmDeviceQueueIO(), and
 * to be i/o by cmStartIO();
 *
 * serialization: IOCACHE_LOCK held on entry/exit;
 */
cbuf_t	*cmQueueIO()
{
	cmdev_t *cdp;
	cbuf_t	*devtail, *iotail;

	/* convert global device iolist into private iolist */
	devtail = NULL;
	while (cachemgr.devtail != NULL)
	{
		/* remove the cmdev at the head of the iolist */
		if ((cdp = cachemgr.devtail->cd_nextdev) == cachemgr.devtail)
			cachemgr.devtail = NULL;
		else
			cachemgr.devtail->cd_nextdev = cdp->cd_nextdev;
		cdp->cd_nextdev = NULL;

		/* insert device iolist at tail of private iolist */
		iotail = cdp->cd_iotail;
		cdp->cd_iotail = NULL;
		if (devtail != NULL)
		{
			iotail->cm_devnxt = devtail->cm_devnxt;
			devtail->cm_devnxt = iotail;
		}
		else
			iotail->cm_devnxt = iotail;
		devtail = iotail;
	}

	return devtail;
}


/*
 * NAME:	cmStartIO()
 *
 * FUNCTION:	start device i/o for all cache devices with queued
 *		cbufs.
 *
 *		beginning with the cache device (cmdev) at the head
 *		of the cache manager's queued device list, each cmdev
 *		is removed from the list and cmDeviceStartIO() is
 *		called for the cmdev to actually start the i/o.
 *
 * PARAMETERS:	none.
 *
 * RETURN VALUES: none.
 *
 * serialization: IOCACHE_LOCK is NOT held during device i/o;
 */
void
cmStartIO(cbuf_t	*devtail)
{
	cmdev_t *cdp;
	cbuf_t	*iotail;

	/* start i/o for each cmdev */
	while (devtail != NULL)
	{
		/* remove the cmdev at the head of the iolist */
		if ((iotail = devtail->cm_devnxt) == devtail)
			devtail = NULL;
		else
			devtail->cm_devnxt = iotail->cm_devnxt;
		iotail->cm_devnxt = NULL;

		/* start the i/o for the cmdev */
		cdp = iotail->cm_ctl->cc_dev;
		cmDeviceStartIO(cdp, iotail);
	}
}


/*
 * NAME:	cmDeviceStartIO()
 *
 * FUNCTION:	start device i/o for a specified cache device (cmdev).
 *
 *		for each cbuf on the cmdev's queued cbuf list, a device
 *		buffer header is allocated from the cmdev's device buffer
 *		free list and filled out with the information needed to
 *		perform the i/o operation.  as a cbuf is processed, it
 *		is removed from the cmdev's queued cbuf list and its device
 *		buffer header is chained together with the headers for
 *		already processed cbufs.  cbuf processing continues until
 *		all queued cbufs have been handled or we run out free
 *		buffer headers.  at this point the device driver strategy
 *		routine is called, passing the constructed list of device
 *		buffer headers.
 *		
 *		iodone processing will redrive the cache device (i.e. call
 *		cmDeviceStartIO()) at the time it frees device buffer
 *		headers to handle the situation in which we have run out
 *		of headers before all queued cbufs have been processed.
 *
 * PARAMETERS:
 *	cdp	- pointer to cmdev to start i/o on.
 *
 * RETURN VALUES: none.
 *
 * serialization: IOCACHE_LOCK is NOT held during device i/o;
 */
void cmDeviceStartIO(
	cmdev_t *cdp,
	cbuf_t	*iotail)
#ifdef _JFS_OS2
{
	int32	rc;
	cbuf_t	*cp;
	iobuf_t	*iobp;	/* current i/o buffer header */
	iobuf_t *next;
	ULONG	pagect;
// BEGIN D230860
#ifdef _JFS_SMARTIO
	cbuf_t	*cp2;
	cbuf_t	*prev;
	cbuf_t	*last;
	int64	xaddr;
	int64	xaddr2;
#endif
// END D230860
	Req_List_Header	*rlhp;  /* pointer to list header */
	Req_Header	*rhp;   /* pointer to header */
	PB_Read_Write	*rwp;   /* pointer to request */
	SG_Descriptor	*sgp;	/* S/G descriptor */

   MMPHPrecmDeviceStartIO();           /* (d201828) MMPH Performance Hook */

	/*
	 * build a list of iobufs, linked thru b_next,
	 * where each iobuf is bound with, thru b_jfsbp,
	 * (head of list of coalesced) cbuf (linked thru cm_ionxt);
	 */
#ifdef	_multirequest_io
	iobuf_t	*head;	/* first i/o buffer header */

	head = cdp->cd_bufs;
	iobp = NULL;
#endif	/* _multirequest_io */

	while (iotail != NULL)
	{
// BEGIN D230860
#ifdef _JFS_SMARTIO
		/* Find buffer with smallest physical address */
		last = cp = iotail->cm_ionxt;
		xaddr = addressPXD(&cp->cm_pxd);
		for (cp2 = cp->cm_ionxt; cp2 != iotail->cm_ionxt;
		     cp2 = cp2->cm_ionxt)
		{
			xaddr2 = addressPXD(&cp2->cm_pxd);
			if (xaddr2 < xaddr)
			{
				xaddr = xaddr2;
				cp = cp2;
				prev = last;
			}
			last = cp2;
		}
		if (cp != iotail->cm_ionxt)		/* Not first cbuf */
		{
			prev->cm_ionxt = cp->cm_ionxt;
			if (cp == iotail)
				iotail = prev;
		}
		else if (cp == iotail)			/* only cbuf */
			iotail = 0;
		else
			iotail->cm_ionxt = cp->cm_ionxt;
#else /* _JFS_SMARTIO */
// END D230860
		/* remove the cbuf from the cbuf iolist */
		if ((cp = iotail->cm_ionxt) == iotail)
			iotail = NULL;
		else
			iotail->cm_ionxt = cp->cm_ionxt;
#endif /* _JFS_SMARTIO 						D230860 */
		cp->cm_ionxt = NULL;

		/*
		 *	try to coalesce
		 */
		if (!cp->cm_metadata && iotail)
		{
			cp = cmCoalesce(cdp, &iotail, cp);
			iobp = cp->cm_iobp;
			rwp = &iobp->bio_rh;
		}
		else
		{
         MMPHphysIOReq();              /* (d201828) MMPH Performance Hook */

			iobp = cp->cm_iobp;

			/*
			 * fill in extent and page frame of r/w request
			 */
			rwp = &iobp->bio_rh;
			rwp->Start_Block = addressPXD(&cp->cm_pxd) << cdp->cd_l2bfactor;
			rwp->Block_Count = lengthPXD(&cp->cm_pxd) << cdp->cd_l2bfactor;
	
                	rwp->SG_Desc_Count = 1;

			/*
			 * update S/G descriptor
			 */
			sgp = iobp->bio_sg;
			sgp->BufferPtr = cp->cm_pgdesc.BufferPtr;
			sgp->BufferSize = lengthPXD(&cp->cm_pxd) << cdp->cd_l2bsize;
		}

		rwp->Blocks_Xferred = 0;
		rwp->RW_Flags = 0;

		/*
		 * fill in request list header
		 */
		rlhp = &iobp->bio_rlh;
		/* rlhp->Count = 1; */
		/* rlhp->Notify_Address = (void *)jfsIODone; */
		/* rlhp->Request_Control = RLH_Single_Req |
					RLH_Notify_Err | RLH_Notify_Done; */
		rlhp->Block_Dev_Unit = cdp->cd_dev;
		rlhp->Lst_Status = 0;

		/*
		 * fill in request header
		 */
		rhp = &iobp->bio_rh.RqHdr;
		/* rhp->Length = RH_LAST_REQ; */
		/* rhp->Old_Command = PB_REQ_LIST; */
		rhp->Command_Code = (cp->cm_read) ? PB_READ_X : PB_WRITE_X;
		/* rhp->Head_Offset = sizeof(Req_List_Header); */
		/* rhp->Req_Control = 0; */
// BEGIN 230860
		if (cp->cm_async)
			rhp->Priority = (cp->cm_read) ? PRIO_PAGER_READ_AHEAD :
							PRIO_LAZY_WRITE;
		else
			rhp->Priority = PRIO_FOREGROUND_USER;
// END 230860
		rhp->Status = 0;
		rhp->Error_Code = 0;
		/* rhp->Hint_Pointer = -1; */

		/* set the buf header flags to indicate the type of
		 * i/o operation.
		 */
                iobp->b_flags = (cp->cm_read) ? B_READ : 0;

		/* set iodone callback */
		if (cp->cm_metadata)
			iobp->b_iodone = bmIODone;
		else
			iobp->b_iodone = cmIODone;

		iobp->b_next = NULL; /* singlerequest_io */

		IS_QUIESCE(cdp);	/* block if hard quiesce */

		if (cdp->cd_flag & CD_STRAT3)
			CallStrat3(cdp->cd_strat3p, rlhp);
		else
			CallStrat2(cdp->cd_strat2p, rlhp);
	}	

#ifdef	_multirequest_io
        /* if a buffer header list has been built, terminate with NULL;
         * send the buffers to the device driver to do the i/o.
	 */
	if (head != NULL && head != (iobuf_t *)cdp->cd_bufs)
	{
		iobp->b_next = NULL;

                for (iobp = head; iobp; iobp = next)
		{
			/* acquire next iobp before CallStrat3() since
			 * b_next of current iobp may be rest by iodone;
			 */
			next = iobp->b_next;

			IS_QUIESCE(cdp);	/* block if hard quiesce */

			if (cdp->cd_flag & CD_STRAT3)
				CallStrat3(cdp->cd_strat3p, &iobp->bio_rlh);
			else
				CallStrat2(cdp->cd_strat2p, &iobp->bio_rlh);
		}
	}
#endif	/* _multirequest_io */

   MMPHPostcmDeviceStartIO();          /* (d201828) MMPH Performance Hook */
}
#endif	/* _JFS_OS2 */


#ifdef	_JFS_OS2
/*
 *	cmCoalesce()
 *
 * try to coalesce consecutive i/o requests in device i/o list which are
 * . in the same direction of i/o (read or write)
 * . contiguous on disk;
 * into a single i/o request;
 *
 * RETURN: the head of the list of coalesced cbufs in ascending order
 *	   linked thru cm_ionxt and NULL terminated;
 *
 * serialization: IOCACHE_LOCK is NOT held during device i/o;
 */
static cbuf_t *cmCoalesce(
	cmdev_t *cdp,
	cbuf_t	**xiotail,
	cbuf_t	*cp)
{
	cbuf_t	*iotail, *head, *tail;
#ifdef _JFS_SMARTIO
	cbuf_t	*last;						// D230860
#endif
	int64	xaddr, hxaddr, txaddr;
	int32	xlen;
	int32	ascend;
	iobuf_t	*iobp;
	PB_Read_Write	*rwp;
	uint16	SG_Desc_Count;
	uint32	Block_Count;
	SG_Descriptor	*pagelist;

	/* init head and tail of coalesce list with cp */
	iotail = *xiotail;
	head = tail = cp;
	hxaddr = txaddr = addressPXD(&cp->cm_pxd);

	/* initialize scatter/gather page frame list */
	SG_Desc_Count = 1;
	Block_Count = lengthPXD(&cp->cm_pxd);

   MMPHphysIOReq();                    /* (d201828) MMPH Performance Hook */

	/* assert(iotail); */

	/* scan device I/O list for contiguous I/O request pages
	 * with cp (cp had been previously removed by caller)
	 */
	while (iotail)
	{
// BEGIN D230860
#ifdef _JFS_SMARTIO
		/*
		 * Scan list for contiguous pages.  We know I/O list contains
		 * only later physical pages
		 */
		last = 0;
		ascend = 0;
		for (cp = iotail->cm_ionxt;
		     (last == 0) || (cp != iotail->cm_ionxt);
		     last = cp, cp = cp->cm_ionxt)
		{
			/* is next cp of the same object ?
			 * (avoid partial page in the middle)
			 * *** WHAT DOES THIS MEAN? ***
			 */
//			if (cp->cm_ctl != head->cm_ctl)
			if (cp->cm_metadata)	/* different iodone routine */
				continue;

			/* is next cp in the same I/O direction ? */
			if (cp->cm_read != head->cm_read)
				continue;

			/*
			 * is page physically contiguous on disk
			 * with head or tail of coalesce list ?
			 */
			xaddr = addressPXD(&cp->cm_pxd);
			xlen = lengthPXD(&cp->cm_pxd);

			if (xaddr == (txaddr + lengthPXD(&tail->cm_pxd)))
			{
				ascend = 1;
				break;
			}
		}
		if (ascend == 0)
			break;

		/* remove cp from device I/O list */
		if (cp != iotail->cm_ionxt)		/* not first */
		{
			last->cm_ionxt = cp->cm_ionxt;
			if (cp == iotail)
				iotail = last;
		}
		else if (cp == iotail)			/* only one */
			iotail = NULL;
		else				/* first but not last */
			iotail->cm_ionxt = cp->cm_ionxt;
#else /* _JFS_SMARTIO */
// END D230860
		/* get head cp of device I/O list */
		cp = iotail->cm_ionxt;

		/* is next cp of the same object ?
		 * (avoid partial page in the middle)
		 */
		if (cp->cm_ctl != head->cm_ctl)
			break;

		/* is next cp in the same I/O direction ? */
		if (cp->cm_read != head->cm_read)
			break;

		/*
		 * is page physically contiguous on disk
		 * with head or tail of coalesce list ?
		 */
		xaddr = addressPXD(&cp->cm_pxd);
		xlen = lengthPXD(&cp->cm_pxd);
		/* are pages in ascending order in i/o list ? */
		if (xaddr == (txaddr + lengthPXD(&tail->cm_pxd)))
			ascend = 1;
		/* are pages in descending order in i/o list ? */
		else if ((xaddr + xlen) == hxaddr)
			ascend = 0;
		else
			break;

		/* jfs_cpio++; number of coalesced pages */

		/* remove cp from device I/O list */
		if (cp == iotail)
			iotail = NULL;
		else
			iotail->cm_ionxt = cp->cm_ionxt;
#endif /* _JFS_SMARTIO 						D230860 */

		MMPHphysIOReq();

		/* add cp at the appropriate place (head or tail)
		 * on the gather list in ascending order.
		 */
#ifndef _JFS_SMARTIO						// D230860
		if (ascend)
		{
#endif								// D230860
			/* insert at tail */
			tail->cm_ionxt = cp;
			cp->cm_ionxt = NULL;
			tail = cp;
			txaddr = xaddr;
#ifndef _JFS_SMARTIO						// D230860
		}
		else
		{
			/* insert at head */
			cp->cm_ionxt = head;
			head = cp;
			hxaddr = xaddr;
		}
#endif /* _JFS_SMARTIO */					// D230860

		/* merge the page to coalesce */
		Block_Count += xlen;
		SG_Desc_Count++;

		if (SG_Desc_Count >= max_coalesce)
			break;
	}

finish:
	/*
	 * construct scatter/gather page frame list in iobp
	 */
	iobp = head->cm_iobp;
	pagelist = iobp->bio_sg;
	for (cp = head; cp; cp = cp->cm_ionxt)
	{
		pagelist->BufferPtr = cp->cm_pgdesc.BufferPtr;
		pagelist->BufferSize = lengthPXD(&cp->cm_pxd) << cdp->cd_l2bsize;
		*pagelist++;
	}

	/* set start address of coalesce with xaddr of head of
	 * the coalesce list
	 */
	rwp = &iobp->bio_rh;
	rwp->SG_Desc_Count = SG_Desc_Count;
	rwp->Start_Block = hxaddr << cdp->cd_l2bfactor;
	rwp->Block_Count = Block_Count <<= cdp->cd_l2bfactor;

	/* performance statistics: number of coalesced i/o */
/*
	if (head->cm_ionxt)
		jfs_cio++;
*/

	/* return the head of the coalesce list */
	*xiotail = iotail;
	return head;
}
#endif	/* _JFS_OS2 */


/*
 * NAME:        cmIODone()
 *
 * FUNCTION:	perform i/o done processing for an i/o request.
 *
 *		the device buffer header for the i/o request is freed
 *		(placed on the cmdev's free buffer header list) and
 *		cache device is redriven if need be.  the cbuf associated
 *		with the i/o request is move from the i/o portion of
 *		the cmctl's cbuf list and any threads waiting for i/o
 *		to complete on this cbuf will be readied.  if the cbuf
 *		is to be discarded, it is released.  for read requests,
 *		the cbuf will also be released if the i/o returned in
 *		error. for write requests, the cbuf will also be released
 *		if the cbuf is currently being replaced and the i/o
 *		completed successfully.
 *
 *		for delete pending cmctls, the thread waiting for all i/o
 *		to complete (and be discarded) will be awoken if the i/o
 *		completion (and discard) is for the final cbuf of the cmctl.
 *		
 *		if a thread is waiting on the cmctl to reach a particular
 *		i/o level, the thread will be awoken here if the level
 *		has been reached by the completion of this (write) request
 *		or this is a write request which returned in error.
 *		
 *		if the cbuf is not to be released, it will be placed
 *		in the cbuf cache list.
 *		
 * PARAMETERS:
 *	bp	- pointer to buf header for completed I/O.
 *
 * RETURN VALUES: none.
 *
 * serialization: in OS/2, IOCACHE_LOCK is held on entry/exit;
 */
void cmIODone(
	iobuf_t *iobp)
{
	cmdev_t *cdp;
	cmctl_t *ccp;
	cbuf_t	*cp;
	int32	ipl, error;
	cbuf_t	*nxp;
#ifdef	_JFS_OS2
	cbuf_t	*cpnext;
#endif	/* _JFS_OS2 */

	/* get back the cbuf bound to the iobuf and
	 * the device that we did i/o for.
	 */
	cp = (cbuf_t *)iobp->b_jfsbp;
	cdp = cp->cm_ctl->cc_dev;

	/* check for an i/o error */
        error = (iobp->b_flags & B_ERROR) ? EIO : 0;

#ifdef _JFS_OS2
   if (error == EIO)
      SysLogError();
#endif

        ipl = IOCACHE_LOCK();

	/* If hard quiesce is in progress, and this is the last pending I/O,
	 * wake up the quiescing thread
	 */
	if ((--cdp->cd_pending_requests == 0) && (cdp->cd_flag & CD_QUIESCE))
		IOCACHE_WAKEUP(&cdp->cd_iowait);

	/*
	 *	scan coalesce list and scatter pages
	 */
	for ( ; cp; cp = cpnext)
	{
		/* remember next page on the coalesce list
		 * (e.g., page may be released)
		 */
		cpnext = cp->cm_ionxt;

		/* remove cp from coalesce list */
		cp->cm_ionxt = NULL;

	/* get cmctl of the cbuf */
	ccp = cp->cm_ctl;				// D230860

	cp->cm_iodone = TRUE;

	/*
	 *	read:
	 */
	if (cp->cm_read)
	{
		cp->cm_read = FALSE;

		/* if there are any threads waiting for this
		 * i/o to complete, wake them up.
		 */
		if (cp->cm_ioevent != EVENT_NULL)
		{
			IOCACHE_WAKEUP_RC(&cp->cm_ioevent,error);
		}

		/* if marked for discard on i/o complete or
		 * i/o error occured in reading the cbuf
		 * from disk, release the cbuf;
	 	 */
		if (cp->cm_discard || error)
		{
			/* is the cbuf marked as inuse ? */
			if (cp->cm_inuse)
			{
				/* inuse cbufs cannot be released.
				 * it must be the error case.
				 */
				assert(!cp->cm_discard);

				cp->cm_inuse = FALSE;
			}

			/* wake up any threads waiting for the	// D228112
			 * buffer to go out of use.		// D228112
			 */					// D228112
			if (cp->cm_event != EVENT_NULL)		// D228112
			{					// D228112
				IOCACHE_WAKEUP(&cp->cm_event);	// D228112
			}					// D228112

			/* release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(cp);

			/* is a deletion pending for the cmctl (i.e.
			 * deletion waiting for all i/o to complete).
			 */
			if (ccp->cc_delpend1)
			{
				/* a thread better be waiting for this */
				assert(ccp->cc_delwait != EVENT_NULL);

				/* is all i/o now complete for the cmctl ?
				 * if so, wake up the thread so that it can
				 * complete the cmctl deletion.
				 */
				if (ccp->cc_cbuflist.head == (cbuf_t *)&ccp->cc_cbuflist)
				{
					IOCACHE_WAKEUP(&ccp->cc_delwait);
				}
			}

			continue;
		}

		/* if the cbuf is inuse, done.
		 * otherwise, cbuf was a readahead:
		 * insert at tail of cachelist below;
		 */
		if (cp->cm_inuse)
			continue;
	} /* end of read */
	/*
	 *	write:
	 */
	else
	{
		cp->cm_write = FALSE;

		/* check if a thread is waiting on an i/o level to
		 * be reached (i.e. synchronous write).
		 */
		/* if the i/o level has been reached (synchronous
		 * write) or an i/o error occured, wake up the thread
		 * waiting on the object write completion.
		 */
		ccp->cc_iolevel--;
        	if (ccp->cc_iowait != EVENT_NULL)
		{
			if (ccp->cc_iolevel == 0 || error)
			{
				IOCACHE_WAKEUP_RC(&ccp->cc_iowait,error);
			}
		}

		/* if there are any threads waiting for this
		 * i/o to complete, wake them up.
		 */
		if (cp->cm_ioevent != EVENT_NULL)
		{
			IOCACHE_WAKEUP_RC(&cp->cm_ioevent,error);
		}

		/* cbuf remains modified if write error */
		if (!error)
			cp->cm_modified = FALSE;

		/*
		 * the cbuf is to be discarded (released).
		 */
		if (cp->cm_discard)
		{
			/* if the write was the result of cbuf replacement,
			 * update the number of pending write due to cbuf
			 * replacement.
			 */
			if (cp->cm_lru)
			{
				cp->cm_lru = FALSE;
				cachemgr.numiolru -= 1;
			}

			/* release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 */
			cmRelCbuf(cp);

			/* is a deletion pending for the cmctl ?
			 * (i.e., deletion waiting for all i/o to complete).
			 */
			if (ccp->cc_delpend1)
			{
				/* a thread better be waiting for this */
				assert(ccp->cc_delwait != EVENT_NULL);

				/* is all i/o now complete for the cmctl ?
				 * if so, wake up the thread so that it can
				 * complete the cmctl deletion.
				 */
				if (ccp->cc_cbuflist.head == (cbuf_t *)&ccp->cc_cbuflist)
				{
					IOCACHE_WAKEUP(&ccp->cc_delwait);
				}
			}

        		IOCACHE_UNLOCK(ipl);
			return;
			continue;
		} /* cm_discard */

		/*
		 * the buffer is to be recycled/replaced:
		 */
		if (cp->cm_lru)
		{
			/* update the number of cbufs with i/o pending
			 * due to cbuf replacement.
			 */
			cp->cm_lru = FALSE;
			cachemgr.numiolru -= 1;

			/* if there was no i/o error, release the cbuf:
			 * remove from hashlist, remove from pagelist, and
			 * insert at head of freelist;
			 * (if i/o error, do not insert back in cachelist)
			 */
			if (!error)
				cmRelCbuf(cp);

			continue;
		} /* cm_lru */

		/*
		 * move from dirty segment to tail of clean segment in pagelist
		 */
		CDLL_MOVE_TO_TAIL(&ccp->cc_cbuflist, cp, cm_pagelist);
	} /* end of write */

#ifdef	_JFS_HEURISTICS_SLRU
	/* if page has been cache hit, insert at tail of protected segment */
	if (cp->cm_hit)
	{
		cp->cm_probationary = FALSE;

		/* is protected segment full ? */
		if (cachemgr.slrun >= cachemgr.slruN)
		{
			/* slide xp one over protected segment */
			if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
				nxp = cachemgr.slruxp->cm_cachelist.next;
			else
				nxp = cachemgr.cachelist.head;

			nxp->cm_probationary = TRUE;
			cachemgr.slruxp = nxp;
		}
		else
			cachemgr.slrun++;
	
		/* insert bp at tail of protected segment */
		CACHELIST_INSERT_TAIL(&cachemgr.cachelist, cp, cm_cachelist);
	}
	/* if page has been cache miss, insert at tail of probationary segment */
	else
	{
		cp->cm_probationary = TRUE;

		/* insert cp at tail of provationary segment as new xp */
		if (cachemgr.slruxp != (void *)&cachemgr.cachelist)
			CACHELIST_INSERT_AFTER(&cachemgr.cachelist, cachemgr.slruxp, cp, cm_cachelist)	
		else
			CACHELIST_INSERT_HEAD(&cachemgr.cachelist, cp, cm_cachelist);
		cachemgr.slruxp = cp;
	}
#else
	CACHELIST_INSERT_TAIL(&cachemgr.cachelist, cp, cm_cachelist);
#endif	/* _JFS_HEURISTICS_SLRU */

	/* wake up any threads waiting for the			// D228112
	 * buffer to go out of use.				// D228112
	 */							// D228112
	if (cp->cm_event != EVENT_NULL)				// D228112
	{							// D228112
		IOCACHE_WAKEUP(&cp->cm_event);			// D228112
	}							// D228112

	/* wakeup any thread waiting for a free buf */
	if (cachemgr.freewait != EVENT_NULL)
	{
		IOCACHE_WAKEUP(&cachemgr.freewait);
	}

	} /* end for scan coalesce list */

        IOCACHE_UNLOCK(ipl);
}


#ifdef	_JFS_OS2
/*
 *	jfsIODone(): interrupt handler callback
 *
 * called by DD ISR as iodone callback;
 */
void APIENTRY jfsIODone(
	Req_List_Header *rlhp)	/* Request List Header */
{
        iobuf_t		*iobp;

        /* get pointer to the iobuf header struct */
        iobp = (iobuf_t *)((caddr_t)rlhp - RLH_OFFSET);

	if (rlhp->Lst_Status & RLH_Unrec_Error)
		iobp->b_flags |= B_ERROR;

	/*
	 * 	b_iodone:
	 */
	(*iobp->b_iodone)(iobp);
}
#endif	/* _JFS_OS2 */


/*
 * NAME:	cmSetXD()
 *
 * FUNCTION:  update cache buffer extent description
 *	when extent covered by the buffer is extended/relocated.
 *
 * RETURN: none
 *
 * note: caller is holding the buffer;
 */
void cmSetXD(
	inode_t		*ip,
	cbuf_t		*cp,
	int64		blkno,  /* page number */
	int64		xaddr,	/* extent address (in fs block number) */
	int32		size)	/* extent size (in fs block number) */		
{
	chash_t 	*chp;
	cmctl_t		*ccp = ip->i_cacheid;
	int32		ipl;

	/*
	 * update buffer extent address
	 */
	chp = CM_HASH(ccp, blkno);

	ipl = IOCACHE_LOCK();	/* acquire jcache_lock */

	/* Remove the cbuf from the previous hashlist */
	HASHLIST_REMOVE(cp, cm_hashlist);

	/* insert the cbuf at the head of specified hashlist */
	HASHLIST_INSERT_HEAD(chp, cp, cm_hashlist);

	IOCACHE_UNLOCK(ipl);	/* release jcache_lock */

	cp->cm_blkno = blkno;
	PXDaddress(&cp->cm_pxd, xaddr);

	/*
	 * update buffer extent size
	 */
updateSize:
	PXDlength(&cp->cm_pxd, size);
	
	return;
}


/*
 *	cmQueryCache()
 *
 * Checks if specified piece of file is in cache.
 *
 * ip		- inode
 * offset	- byte offset into file
 * nbytes	- number of bytes to check
 *
 * Returns: number of bytes not cached starting at the specified offset up to
 *	the length specified.
 */
int64
cmQueryCache(inode_t *ip,
	     int64 offset,
	     int64 nbytes)
{
        int64 nblks, blkno, first, last, lowblk;
	cbuf_t *cp;
        cmctl_t *ccp;
	int32 ipl;

        if ((ccp = ip->i_cacheid) == NULL)
                return(nbytes);

        if (ccp->cc_ncbufs == 0)
                return(nbytes);

	first = CM_OFFTOCBLK(offset);
	last = CM_OFFTOCBLK(offset + nbytes - 1);
	nblks = last - first + 1;

	lowblk = CM_MAXLBLKNO+1;

	ipl = IOCACHE_LOCK();

        if (nblks < ccp->cc_ncbufs >> 3)
	{
		for (blkno = first; blkno <= last; blkno++)
		{
			if (cmLookup(ccp,blkno) != NULL)
			{
				lowblk = blkno;
				break;
			}
		}
	}
	else
	{
        	for (cp = ccp->cc_cbuflist.head;
		     cp != (cbuf_t *)&ccp->cc_cbuflist && nblks > 0;
		     cp = cp->cm_pagelist.next)
		{
			if (cp->cm_blkno >= first && cp->cm_blkno <= last)
			{
				lowblk = MIN(lowblk,cp->cm_blkno);
				nblks--;
			}
		}
	}

	IOCACHE_UNLOCK(ipl);

	if (lowblk != CM_MAXLBLKNO+1)
		nbytes = (lowblk == first) ? 0 :
				 (lowblk << CM_L2BSIZE) - offset;

	return(nbytes);
}


/*
 *	cmSendFile()
 *
 * Builds list of cache buffers to send to TCPIP.
 *
 * ip		- inode
 * offset	- byte offset into file
 * nbytes	- number of bytes to check
 * cacheList	- pointer to cachelist containing requested buffers
 *
 * Returns: zero if successful
 */
int32
cmSendFile(inode_t *ip,
	     int64 offset,
	     int64 *pLen,
	     cache_list_t *cacheList)
{
	cmdev_t	*cdp;
	uint32	coff;
	int32	count = 0;
	cbuf_t	*cp;
	int64	first;
	uint32	i;
	int32	ipl;
	uint32	j;
	int64	last;
	int64	lblkno;
	uint32	llen;
	uint32	loff;
	uint32	nb;
	uint32	nbuffer;
	int64	nbytes;
	int64	nBytesRead = 0;
	int64	next;
	int64	offs = offset;
	pxdlist_t pxdlist;
	cbuf_t	*racp[MAX_CACHE_LIST];
	cbuf_t	*ratail;
	int32	rc;
	cbuf_t	*tail;
	int64	xp;

	nbytes = *pLen;

	/* Get any buffers that are in cache and available */

cmSendFile_Top:

	ipl = IOCACHE_LOCK();

	while (count < MAX_CACHE_LIST && nBytesRead < nbytes)
	{
		lblkno = offs >> CM_L2BSIZE;
		cp = cmLookup(ip->i_cacheid, lblkno);
		if (cp == 0)
			break;	/* Not in cache */
		if (cp->cm_read || cp->cm_write)
			break;	/* We don't want to wait */

		if (cp->cm_inuse)
		{
			if ((cp->cm_sendfile == FALSE) ||
			    (cp->cm_event != EVENT_NULL))
				/* A non-sendfile thread is either using or
				 * waiting on this buffer.  We don't want to
				 * starve local I/O.
				 */
				break;
			else
				/* We are sharing with other sendfile threads */
				cp->cm_usecount++;
		}
		else
		{
			/* remove the cbuf from the cachelist */
#ifdef	_JFS_HEURISTICS_SLRU
			cp->cm_hit = TRUE;
			if (cp->cm_probationary)
			{
				if (cp == cachemgr.slruxp)
					cachemgr.slruxp = cp->cm_cachelist.prev;
			}
			else
				cachemgr.slrun--;
#endif	/* _JFS_HEURISTICS_SLRU */
			CACHELIST_REMOVE(&cachemgr.cachelist, cp, cm_cachelist);
			cp->cm_inuse = TRUE;
			cp->cm_sendfile = TRUE;
			cp->cm_usecount = 1;
		}

		/* Add cbuf to cacheList */
		coff = (uint)offs & CM_OFFSET;
		nb = MIN(nbytes - nBytesRead, CM_BSIZE - coff);
		cacheList->linearList[count].Addr = (ULONG)cp->cm_cdata+coff;
		cacheList->linearList[count].Size = nb;
		cacheList->physicalList[count].Addr =
			(uint32)cp->cm_iobp->bio_sg[0].BufferPtr + coff;
		cacheList->physicalList[count].Size = nb;
		cacheList->cbuf[count] = cp;

		offs += nb;
		nBytesRead += nb;
		count++;
	}
	IOCACHE_UNLOCK(ipl);

	if (count)
	{
		cacheList->kCacheList.LinListCount = count;
		cacheList->kCacheList.LinearList = cacheList->linearList;
		cacheList->kCacheList.PhysListCount = count;
		cacheList->kCacheList.PhysicalList = cacheList->physicalList;
		cacheList->nCbufs = count;

		*pLen = nBytesRead;

		if (nBytesRead == nbytes)
			return 0;		/* We're done */
	}

	/* Initiate read-ahead (if necessary) for following pages */

	ratail = tail = 0;
	first = offs >> CM_L2BSIZE;
	last = (offs + (nbytes - nBytesRead -1 )) >> CM_L2BSIZE;
	last = MIN(first + MAX_CACHE_LIST - 1,  last);

	lblkno = first;

	while (lblkno <= last)
	{
		loff = lblkno << ip->i_ipmnt->i_l2nbperpage;
		llen = nbuffer = next = 0;
		for (xp = lblkno; xp <= last; xp++)
		{
			cp = cmGet(ip->i_cacheid, xp, CM_READ);
			if (cp && cp->cm_new)
			{
				cp->j_ip = ip;
				racp[nbuffer] = cp;
				llen += ip->i_ipmnt->i_nbperpage;
				nbuffer++;
			}
			else
			{
				/* Either the page is already in the cache,
				 * or we can't get a new cbuf without blocking.
				 * Either way, we want to skip this one.
				 */
				if (cp)
					cmPut(cp, FALSE);
				next = xp+1;
				break;
			}
		}
		if (nbuffer)
		{
			i = 0;
			while (i < nbuffer)
			{
				pxdlist.maxnpxd = nbuffer - i;
				pxdlist.npxd = 0;
				rc = xtLookupPageList(ip, loff, llen, &pxdlist,
						      0);
				if (rc)
				{
					/* Something wrong */
					cmPut(racp[i], FALSE);
					loff += ip->i_ipmnt->i_nbperpage;
					llen -= ip->i_ipmnt->i_nbperpage;
					i++;
				}
				else if (pxdlist.npxd == 0)
				{
					/* Must be a hole */
					cp = racp[i];
					cp->cm_hole = TRUE;
					PXDaddress(&cp->cm_pxd, 0);
					PXDlength(&cp->cm_pxd, 0);
					bzero(cp->cm_cdata, CM_BSIZE);
					cp->cm_new = FALSE;
					cmPut(cp, FALSE);
					loff += ip->i_ipmnt->i_nbperpage;
					llen -= ip->i_ipmnt->i_nbperpage;
					i++;
				}
				else
				{
					ipl = IOCACHE_LOCK();	// D228112
					for (j = 0; j < pxdlist.npxd; j++)
					{
						cp = racp[i];
						cp->cm_new = FALSE;
						cp->cm_inuse = FALSE;
						IOQUEUE_ASYNC_READ(cp); // D230860
						cp->cm_pxd = pxdlist.pxd[j];
						if (tail)
							tail->cm_ionxt = cp;
						else
							ratail = cp;

						tail = cp;
						loff += ip->i_ipmnt->i_nbperpage;
						llen -= ip->i_ipmnt->i_nbperpage;
						i++;
					}
					IOCACHE_UNLOCK(ipl);	// D228112
				}
			}
		}
		if (next)
			lblkno = next;
		else
			break;
	}

	if (tail)
	{
		tail->cm_ionxt = ratail;
		ratail = tail;
	}

	if (ratail)
	{
		cdp = ratail->cm_ctl->cc_dev;
		cmDeviceStartIO(cdp, ratail);
	}

	if (count == 0)
	{
		/* We can't return until we have something */
		lblkno = offset >> CM_L2BSIZE;
		cp = cmGet(ip->i_cacheid, lblkno, CM_WAIT | CM_READ);
		cmPut(cp, FALSE);

		goto cmSendFile_Top;
	}

	return 0;
}


/*
 * NAME:	rawIO()
 *
 * FUNCTION:	raw address I/O;
 *
 * serialization: IOCACHE_LOCK;
 */
/*
 *	rawInit()
 */
int32 rawInit()
{
	return 0;
}


/*
 *	rawRead()
 *
 * function: synchronously read a page;
 */
int32 rawRead(
	inode_t	*ipmnt,
	int64	offset,
	cbuf_t	**bpp)
{
	int32		rc;	
	int32		ipl;
	cbuf_t		*bp;

	ipl = IOCACHE_LOCK();

	/* allocate a JFS buffer */
	while ((bp = (cbuf_t *)cmRecycle(CM_WAIT, &ipl)) == NULL);
		;
	
	/*
	 * initialize buffer for device driver
	 */
	IOQUEUE_READ(bp);

	bp->j_ip = ipmnt;
	
	/* map the logical block address to physical block address */
	bp->cm_blkno = offset >> ipmnt->i_l2pbsize;

	IOCACHE_UNLOCK(ipl);

	/*
	 *	initiate pagein of the page
	 */
	rawStartIO(bp);

	/*
	 * Wait for the read to complete.
	 */
	ipl = IOCACHE_LOCK();

	if (!bp->cm_iodone)
	{
		IOCACHE_SLEEP_RC(&bp->cm_ioevent,0,rc);
	}
	else
		rc = (bp->cm_iobp->b_flags & B_ERROR) ? EIO : 0;

	if (rc)
		cmFreeCbuf(bp);

	IOCACHE_UNLOCK(ipl);

	*bpp = bp;

	return rc;
}


/*
 *	rawWrite()
 *
 * function: synchronously write a page and free its buffer;
 *
 * note: caller is holding the buffer;
 */
int32 rawWrite(
	inode_t	*ipmnt,
	cbuf_t	*bp,
	uint32	flag)	/* release ? */
{
	int32		rc;	
	int32		ipl;
	
	/*
	 * initialize buffer for device driver
	 */
	IOQUEUE_WRITE(bp);

	bp->j_ip = ipmnt;

	/*
	 *	initiate pageout of the page
	 */
	rawStartIO(bp);

	/*
	 * Wait for the write to complete.
	 */
	ipl = IOCACHE_LOCK();

	if (!bp->cm_iodone)
	{
		IOCACHE_SLEEP_RC(&bp->cm_ioevent,0,rc);
	}
	else
		rc = (bp->cm_iobp->b_flags & B_ERROR) ? EIO : 0;

	if (flag)
		cmFreeCbuf(bp);

	IOCACHE_UNLOCK(ipl);

	return rc;
}


/*
 *	rawStartIO()
 */
void rawStartIO(cbuf_t *bp)
{
	iobuf_t		*iobp;
        Req_List_Header *rlhp;  /* pointer to list header */
        Req_Header      *rhp;   /* pointer to header */
        PB_Read_Write   *rwp;   /* pointer to request */
	SG_Descriptor	*sgp;	/* S/G descriptor */
	cmdev_t		*cdp;

	iobp = bp->cm_iobp;

	/*
	 * set the device buf header
	 */
	iobp->b_flags = (bp->cm_read) ? B_READ : B_WRITE;
	iobp->b_iodone = rawIODone;

	/*
	 * fill in request list header
	 */
	rlhp = &iobp->bio_rlh;
	/* rlhp->Count = 1; */
	/* rlhp->Notify_Address = (void *)jfsIODone; */
	/* rlhp->Request_Control = RLH_Single_Req |
				   RLH_Notify_Err | RLH_Notify_Done; */
        rlhp->Block_Dev_Unit = bp->j_ip->i_dev;
	rlhp->Lst_Status = 0;

	/*
	 * fill in request header
	 */
	rhp = &iobp->bio_rh.RqHdr;
	/* rhp->Length = RH_LAST_REQ; */
	/* rhp->Old_Command = PB_REQ_LIST; */
        rhp->Command_Code = (bp->cm_read) ? PB_READ_X : PB_WRITE_X;
	/* rhp->Head_Offset = sizeof(Req_List_Header); */
	/* rhp->Req_Control = 0; */
	/* rhp->Priority = PRIO_FOREGROUND_USER; */
	rhp->Status = 0;
	rhp->Error_Code = 0;
	/* rhp->Notify_Address */
	/* rhp->Hint_Pointer = -1; */

        /*
	 * fill in read/write request
	 */
        rwp = &iobp->bio_rh;
        rwp->Start_Block = bp->cm_blkno;
        rwp->Block_Count = PSIZE >> bp->j_ip->i_l2pbsize;
        rwp->Blocks_Xferred = 0;
        rwp->RW_Flags = 0;

	/* update S/G descriptor */
	rwp->SG_Desc_Count = 1;
	sgp = iobp->bio_sg;
	*sgp = bp->cm_pgdesc;

	cdp = bp->j_ip->i_cachedev;
	if (cdp)
		IS_QUIESCE(cdp);	/* block if hard quiesce */

        /* Call the strategy2 interface */
	if (bp->j_ip->i_Strategy3)
        	CallStrat3(bp->j_ip->i_Strategy3, rlhp);
	else
        	CallStrat2(bp->j_ip->i_Strategy2, rlhp);
}


/*
 *	rawIODone()
 *
 * N.B.: OS/2: jfsIOLock is held on entry/exit;
 */
void rawIODone(
	register iobuf_t	*iobp)
{
	register cbuf_t	*bp;
	cmdev_t		*cdp;
	register int32	ipl;
	int32	error;

	/*
	 * get back jfs buffer bound to the io buffer
	 */
	bp = (cbuf_t *)iobp->b_jfsbp;

	/* check for an error on the i/o */
        error = (iobp->b_flags & B_ERROR) ? EIO : 0;

	ipl = IOCACHE_LOCK();		/* disable+lock */

	bp->cm_iodone = TRUE;
	bp->cm_read = bp->cm_write = FALSE;

	/* If hard quiesce is in progress, and this is the last pending I/O,
	 * wake up the quiescing thread
	 */
	cdp = bp->j_ip->i_cachedev;
	if (cdp && (--cdp->cd_pending_requests == 0) &&
		    (cdp->cd_flag & CD_QUIESCE))
		IOCACHE_WAKEUP(&cdp->cd_iowait);

	IOCACHE_UNLOCK(ipl);	/* unlock+enable */

	/*
	 * wake up waitor for synchronous i/o
	 */
	if (bp->cm_ioevent != EVENT_NULL)
	{
		IOCACHE_WAKEUP_RC(&bp->cm_ioevent,error);
	}
}


/*
 *	rawAllocate()
 */
cbuf_t *rawAllocate()
{
	int32		ipl;
	cbuf_t		*bp;

	ipl = IOCACHE_LOCK();

	/* allocate a JFS buffer */
	while ((bp = (cbuf_t *)cmRecycle(CM_WAIT, &ipl)) == NULL);
		;
	
	IOCACHE_UNLOCK(ipl);

	return bp;
}


/*
 *	rawRelease()
 */
void rawRelease(cbuf_t *bp)
{
	int32	ipl;

	ipl = IOCACHE_LOCK();

	cmFreeCbuf((cbuf_t *)bp);

	IOCACHE_UNLOCK(ipl);
}
