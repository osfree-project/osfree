/*
 * $Header$
 */

/************************************************************************/
/*                       Linux partition filter.                        */
/*          (C) Copyright Deon van der Westhuysen, July 1995.           */
/*                                                                      */
/*  Dedicated to Jesus Christ, my Lord and Saviour.                     */
/*                                                                      */
/* This program is free software; you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation; either version 2, or (at your option)  */
/* any later version.                                                   */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this program; if not, write to the Free Software          */
/* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            */
/*                                                                      */
/*  This code is still under development; expect some rough edges.      */
/*                                                                      */
/************************************************************************/

#include "debug.h"
#include "e2data.h"
#include "e2router.h"
#include "e2filt.h"
#include "e2wrap.h"

/* Any IORBs for the base units arrive here, including requests generated */
/* by accesses to the data in the virtual partitions. */

void FilterHandler (PIORB pIORB)
{
NPBaseUnitRec	pUnitRec;			/* Ptr to unit record */

   pUnitRec= (NPBaseUnitRec) pIORB->UnitHandle;	/* Get ptr to unit record */
						   /* from the unit handle */
   pUnitRec->SaveReqCtrl= pIORB->RequestControl;	/* Save request flags */
   pUnitRec->SaveNotify= pIORB->NotifyAddress;	/* Save notify address */
   pUnitRec->SaveReserved= pIORB->Reserved_1;	/* Save reserved field */
   pIORB->Reserved_1= pIORB->UnitHandle;		/* Save unit handle */

   pIORB->UnitHandle= pUnitRec->UnitHandle;	/* Restore old unit handle */
   pIORB->NotifyAddress= &FilterNotifyWrapper;	/* Install our routine */
   pIORB->RequestControl|= IORB_ASYNC_POST;	/* Request notification */
   pIORB->RequestControl&= ~IORB_CHAIN;		/* No IORBs to chain */

   /* Check for commands that we might wish to change. None yet. */
   /* except allocation- but that was allready done in the router */

   /* If we are still here then the command was not intercepted... */
   pUnitRec->pADDEntry(pIORB);	/* Ask the original driver to do the request */
}

/* This routine is called by the original device driver as soon as the */
/* filtered request is done. Here the fields we modified in the IORB are */
/* restored before we pass it back to the original caller. */
void FilterNotify (PIORB pIORB)
{
NPBaseUnitRec	pUnitRec;			/* Ptr to unit record */
NPIORBQueue	pQueue;				/* Queue request came from */

   pUnitRec= (NPBaseUnitRec) pIORB->Reserved_1;	/* Get ptr to unit record */

   pIORB->UnitHandle= (USHORT) pUnitRec;		/* Restore old unit handle */
   pIORB->RequestControl= pUnitRec->SaveReqCtrl;	/* Restore rest of IORB */
   pIORB->Reserved_1= pUnitRec->SaveReserved;
   pIORB->NotifyAddress= pUnitRec->SaveNotify;

   pQueue= &(pUnitRec->Hdr.IORBQueue);		/* Get ptr to IORB queue */

   pQueue->Flags&=~F_REQUEST_BUSY;		/* Indicate queue finished */
   NotifyDone (pIORB);				/* Notify caller */
   StartIORBQueue (pQueue);			/* Try to restart queue */
}
