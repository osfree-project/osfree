/* $Id: jfs_strat2.c,v 1.1 2000/04/21 10:58:16 ktk Exp $ */

static char *SCCSID = "@(#)1.5  7/30/98 14:20:17 src/jfs/ifs/jfs_strat2.c, sysjfs, w45.fs32, 990417.1";
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
 *	jfs_strat2.c
 *
 *	Provides wrapper around 16-bit strategy2 routine when strategy3
 *	routine is not available.
 *
 *	The strategy2 routine requires the request list to be in contiguous
 *	16-bit addressable memory.  We maintain a pool of these lists and copy
 *	data from the iobuf's which are passed in, putting the request lists
 *	back into the free pool when the callback (iodone) routine is called.
 */

#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <jfs_os2.h>

#include <jfs_types.h>
#include <jfs_inode.h>
#include <jfs_cachemgr.h>
#include <jfs_debug.h>

void Strategy2(void * _Seg16, Req_List_Header * _Seg16);
extern void * _Seg16	p16_IODone16;	/* Seg16 pointer to IODone16 routine*/

typedef struct	_ReqList
{
	union
	{
		Req_List_Header	*_origPacket;	/* 4: flat req list */
		struct _ReqList	*_nextFree;	/* 4: free list */
	} u1;
	Req_List_Header * _Seg16	rlh16;	/* 4: Seg16 address of rlh */
	Req_List_Header			rlh;	/* 20: request list header */
	PB_Read_Write			rh;	/* 52: request header */
	SG_Descriptor			sg[MAX_RAPAGE];	/* 64: scatter/gather */
} ReqList;					/* (144) */
#define	origPacket	u1._origPacket
#define	nextFree	u1._nextFree

/*
 * Number of items in request list pool.  This is the number of pending I/O
 * requests.  This number cannot exceed 64K/(sizeof ReqList) = 455.
 */
#define	REQ_LIST_POOL_SIZE	128

ReqList	*req_list_free;		/* Pointer to ReqList free list */
XSPINLOCK_T	req_list_spinlock;	/* Lock taken to protect req_list_free*/
event_t		req_list_event = EVENT_NULL;

/*	strat2Init	*/

int32	strat2Init()
{
	int32	rc;
	ReqList	*rlp, *last = 0;
	int32	i;

	XSPINLOCK_ALLOC(&req_list_spinlock, LOCK_ALLOC_PIN, 0, 0);
	XSPINLOCK_INIT(&req_list_spinlock);

	rlp = (ReqList *)xmalloc(REQ_LIST_POOL_SIZE*sizeof(ReqList), 0,
				 pinned_heap|XMALLOC_CONTIG|XMALLOC16);
	if (rlp == 0)
		return ENOMEM;

	for (i = 0; i < REQ_LIST_POOL_SIZE; i++)
	{
		rlp->nextFree = last;
		rlp->rlh16 = &(rlp->rlh);	/* Thunk to Seg16 */
		last = rlp;
		rlp++;
	}
	req_list_free = last;

	return 0;
}

void CallStrat2(
void * _Seg16	strat_routine,
Req_List_Header *packet)
{
	ReqList *rl;

	ASSERT (((PB_Read_Write *)((char *)packet+20))->SG_Desc_Count <= MAX_RAPAGE);

	XSPINLOCK_LOCK(&req_list_spinlock);
	while (req_list_free == 0)
	{
		XEVENT_SLEEP(&req_list_event, &req_list_spinlock, T_XSPINLOCK);
	}

	rl = req_list_free;
	req_list_free = rl->nextFree;

	XSPINLOCK_UNLOCK(0, &req_list_spinlock);

	rl->origPacket = packet;

	memcpy((void *)(&rl->rlh), packet, 72+(8*MAX_RAPAGE));

	*((void * _Seg16 *)&rl->rlh.Notify_Address) = p16_IODone16;

	KernSerialize16BitDD();

	Strategy2(strat_routine, rl->rlh16);

	KernUnserialize16BitDD();
}

void jfsIODone16(
Req_List_Header * _Seg16	rlh)
{
	ReqList *rl;
	Req_List_Header *rlh32;
	PB_Read_Write *rh32;
	void (* APIENTRY iodone32)(Req_List_Header *);

	KernThunkStackTo32();

	rl = (ReqList *)((char *)rlh - 8);
	rlh32 = rl->origPacket;
	rh32 = (PB_Read_Write *)((uint32)rlh32 + 20);
	rlh32->Lst_Status = rl->rlh.Lst_Status;
	rh32->RqHdr.Status = rl->rh.RqHdr.Status;

	XSPINLOCK_LOCK(&req_list_spinlock);

	rl->nextFree = req_list_free;
	req_list_free = rl;

	if (req_list_event != EVENT_NULL)
	{
		XEVENT_WAKEUP(&req_list_event)
	}

	XSPINLOCK_UNLOCK(0,&req_list_spinlock);

	*(void **)&iodone32 = rlh32->Notify_Address;
	(*iodone32)(rlh32);

	KernThunkStackTo16();
}
