/* $Id: jfs_pageio.c,v 1.1.1.1 2003/05/21 13:38:02 pasha Exp $ */

static char *SCCSID = "@(#)1.12  11/5/98 15:27:00 src/jfs/ifs/jfs_pageio.c, sysjfs, w45.fs32, 990417.1";
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

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <fsd.h>
#include "jfs_os2.h"
#include "jfs_util.h"
#include "jfs_types.h"
#include "jfs_lock.h"
#include "jfs_inode.h"
#include "jfs_filsys.h"
#include "jfs_proto.h"
#include "jfs_debug.h"


/* Request list to be passed to the driver's strategy2 routine.
 */
struct pagereq {
	Req_List_Header		pr_hdr;			/* 20: */
	struct _pr_list {
		PB_Read_Write	pr_rw;			/* 52: */
		SG_Descriptor	pr_sg;			/* 8: */
	}			pr_list[MAXPGREQ];	/* 480: (60*8) */
};							/* (500) */


/* Event semaphore for synchronization with iodone routine.
 */
event_t		pg_event;
XSPINLOCK_T	pg_spinlock;
#define PAGING_LOCK()	XSPINLOCK_LOCK(&pg_spinlock)
#define PAGING_UNLOCK()	XSPINLOCK_UNLOCK(0, &pg_spinlock)
#define PAGING_NOLOCK()	XSPINLOCK_NOLOCK(0, &pg_spinlock)
#define PAGING_SLEEP(Event, Flag)	\
	XEVENT_SLEEP(Event, &pg_spinlock, Flag|T_XSPINLOCK)
#define PAGING_WAKEUP(Event)	XEVENT_WAKEUP(Event)

/* Request list - defined in 16-bit segment in misc.asm
 */
extern struct pagereq	pgreq;
extern Req_List_Header * _Seg16	p16_pgreq;	/* Seg16 pointer to pgreq */
extern void * _Seg16	p16_pageIOdone16;	/* Seg16 pointer to iodone */

/* forward references
 */ 
void APIENTRY pageIOdone(struct pagereq *);
int32	getblkno(struct inode *, uint32, uint32 *);
void	Strategy2(void * _Seg16, Req_List_Header * _Seg16);

/*
 * NAME:        jfs_pageio
 *
 * FUNCTION:    support for i/o to the pager's swap file
 *
 * PARAMETERS:	vp - vnode for the swap file
 *		cmdlist - description of i/o operations needed
 *
 * RETURN VALUES:
 *		0 - success
 *		errors from subroutines. 
 */

int32
jfs_pageio(struct vnode		*vp,
	   struct PageCmdHeader *cmdlist)
{
	cmdev_t		*cdp;
	inode_t 	*ip = VP2IP(vp);
	int		i, rc;
	int32		ipri;
	uint32		fsbno;	/* starting block number to read/write */
	struct PageCmd  *pgcmd; /* pointer to current command in list */
        Req_List_Header *rlhp;  /* pointer to request list header */
        Req_Header      *rhp;   /* pointer to request header */
        PB_Read_Write   *rwp;   /* pointer to request */

	/* Serialize on the swap file inode.
	 */
	IWRITE_LOCK(ip);

        /* Initialize the request list and fill in the
	 * necessary fields in the list header.
         */
	bzero(&pgreq, sizeof(struct pagereq));
        rlhp = &pgreq.pr_hdr;
        rlhp->Count = cmdlist->OpCount;
	if (ip->i_ipmnt->i_Strategy3)
        	rlhp->Notify_Address = (void *)pageIOdone;
	else
		*(void * _Seg16 *)(&rlhp->Notify_Address) = p16_pageIOdone16;
        rlhp->Request_Control = (cmdlist->InFlags & PGIO_FI_ORDER)
				? RLH_Notify_Done | RLH_Exe_Req_Seq
				: RLH_Notify_Done;
        rlhp->Block_Dev_Unit = ip->i_dev;

	/* Fill in a request for each command in the input list.
	 */
	assert(cmdlist->OpCount > 0);

	for (i=0, pgcmd = cmdlist->PageCmdList;
	     i < cmdlist->OpCount;
	     i++, pgcmd++)
	{
		/* Fill in request header.
		 * These fields are set to zero by bzero, above:
		 *	rhp->Req_control
		 *	rhp->Status
		 *	rwp->RW_Flags
		 */
		rhp = &pgreq.pr_list[i].pr_rw.RqHdr;
		rhp->Length = sizeof(struct _pr_list);
		rhp->Old_Command = PB_REQ_LIST;
		rhp->Command_Code = pgcmd->Cmd;
		rhp->Head_Offset = (ULONG)rhp - (ULONG)rlhp;
		rhp->Priority = pgcmd->Priority;
		rhp->Hint_Pointer = -1;

		/* Fill in read/write request.
		 */
		rwp = &pgreq.pr_list[i].pr_rw;
		rc = getblkno(ip, pgcmd->FileOffset, &fsbno);
		if (rc)
		{
			/* request is not valid, return error */
			IWRITE_UNLOCK(ip);
			return rc;
		}
		rwp->Start_Block = fsbno;
		rwp->Block_Count = PSIZE >> (ip->i_ipmnt->i_l2pbsize);
		rwp->SG_Desc_Count = 1;

		/* Fill in the scatter/gather descriptor
		 */
		pgreq.pr_list[i].pr_sg.BufferPtr = (void *)pgcmd->Addr;
		pgreq.pr_list[i].pr_sg.BufferSize = PSIZE;
	}

	/* Length in last request must be set to terminal value.
	 */
	rhp->Length = RH_LAST_REQ;

	IS_QUIESCE(ip->i_ipmnt->i_cachedev);	/* Block if hard quiesce */

        /* Before calling the driver, set the i/o event semaphore to
         * indicate that we're waiting for results.  This avoids a race
         * with bmIOdone (running in interrupt context) for the case
         * where the i/o request goes really fast, or fails.
         */
	pg_event = EVENT_WAIT;

        /* Call the strategy2 interface.
         */
	if (ip->i_ipmnt->i_Strategy3)
        	CallStrat3(ip->i_ipmnt->i_Strategy3, (Req_List_Header *)&pgreq);
	else
		Strategy2(ip->i_ipmnt->i_Strategy2, p16_pgreq);

	/* Wait for the request to complete.
	 */
	PAGING_LOCK();
	if (pg_event != EVENT_NULL)
	{
		PAGING_SLEEP(&pg_event, T_NORELOCK);
		PAGING_NOLOCK();
	}
	else
		PAGING_UNLOCK();

	/* If hard quiesce is in progress, and this is the last pending I/O,
	 * wake up the quiescing thread
	 */
	ipri = IOCACHE_LOCK();
	cdp = ip->i_ipmnt->i_cachedev;
	if ((--cdp->cd_pending_requests == 0) && (cdp->cd_flag & CD_QUIESCE))
		IOCACHE_WAKEUP(&cdp->cd_iowait);
	IOCACHE_UNLOCK(ipri);

	/* Check for errors and update status info in the command list.
	 * Set return value to error code from first failing command.
	 */
	rc = 0;
	for (i=0; i < cmdlist->OpCount; i++)
	{
		pgcmd = &cmdlist->PageCmdList[i];
		rhp = &pgreq.pr_list[i].pr_rw.RqHdr;

		pgcmd->Status = rhp->Status;
		pgcmd->Error = rhp->Error_Code;
		if ((rc == 0) && (pgcmd->Error != 0))
			rc = pgcmd->Error;
	}

	IWRITE_UNLOCK(ip);
	return rc;
}


/*
 * NAME:        getblkno
 *
 * FUNCTION:    Determines the file system block number for the
 *		offset within the file specified by ip.
 *
 * PARAMETERS:	ip - inode for the swap file
 *		offset - page aligned offset within the file
 *		blkno - place to store the file system block number
 *
 * RETURN VALUES:
 *		0 - success
 *		errors from subroutines. 
 */
static int32
getblkno(struct inode	*ip,
	 uint32		offset, 
	 uint32		*blkno)
{
	int32		rc;
	lxdlist_t	lxdlist;
	lxd_t		lxd;
	xadlist_t	xadlist;
	xad_t		xad;
	int64		xaddr;
	int64		lblkno;

	/* NOTE: the input offset value and the output blkno
	 * value are both specified as 32 bits.  This is because
	 * the pager calls with a 32 bit offset -- and so cannot
	 * use a file larger than 4GB.
	 */

	/* Convert per file logical block number 
	 * to file system block number.
	 */
	lblkno = offset >> L2PSIZE;
	lxdlist.maxnlxd = 1;
	lxdlist.nlxd = 1;
	lxdlist.lxd = &lxd;
	LXDoffset(&lxd, lblkno << ip->i_ipmnt->i_l2nbperpage);
	LXDlength(&lxd, PSIZE >> ip->i_ipmnt->i_l2bsize);
	xadlist.maxnxad = 1;
	xadlist.nxad = 0;
	xadlist.xad = &xad;
	if (rc = xtLookupList(ip, &lxdlist, &xadlist, 0))
		return rc;
	if (xadlist.nxad != 1)
		return ENXIO;
		
	xaddr = addressXAD(&xad);
	*blkno = LBLK2PBLK(ip->i_ipmnt, xaddr);

	return 0;
}


/*
 * Routine called at interrupt to indicate completion of the i/o.
 */
void APIENTRY
pageIOdone(struct pagereq *prp)
{
	/* Wake up the pageio routine so it can complete.
	 */
	PAGING_LOCK();
	PAGING_WAKEUP(&pg_event);
	PAGING_UNLOCK();
}
/*
 * This is called from on 16-bit stack from strat2
 */
void
pageIOdone2(struct pagereq * _Seg16 prp)
{
	KernThunkStackTo32();
//	pageIOdone(prp);
	pageIOdone(0);
	KernThunkStackTo16();
}
