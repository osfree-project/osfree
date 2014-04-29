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
#include "e2wrap.h"
#include "e2filt.h"
#include "e2part.h"

/* Macro to test whether an pointer points to an element in the array */
#define IS_ELEMENT(PTR,ARRAY,SIZE)	((((VOID*)PTR)>=((VOID*)ARRAY))&&\
					 (((VOID*)PTR)<((VOID*)(ARRAY+SIZE))))

/* All IORBs arrive here. We queue the IORB and then call the queue server */
/* to take over. */
void E2FilterIORB (PIORB pIORB)
{
 NPRecHeader	pUnitRec;			/* Ptr to unit rec for IORB */
 NPIORBQueue	pQueue;				/* Ptr to queue for IORB */

 if (pIORB->CommandCode==IOCC_CONFIGURATION)	/* For configuration IORBs: */
  pUnitRec= (NPRecHeader) VirtUnits;		/* Assign IORB to the first */
						/* virtual unit. (For queue) */
 else
  pUnitRec= (NPRecHeader) pIORB->UnitHandle;	/* Get pointer to unit */
						/* from the unit handle */
 /* Check to see if pUnitRec is a pointer to a valid unit record. */
 if (!(IS_ELEMENT(pUnitRec,BaseUnits,NumBaseUnits)||
       IS_ELEMENT(pUnitRec,VirtUnits,NumVirtUnits)))
 {
  IORBError (pIORB,IOERR_NO_SUCH_UNIT);		/* Tell them they are crazy! */
  NotifyDone (pIORB);				/* Notify that we are done */
 }
 else
 {
  pQueue= &(pUnitRec->IORBQueue);		/* Get pointer to IORB queue */
  DISABLE					/* Same safety... */
  AddIORBToQueue (pQueue,pIORB);		/* Add IORB to queue */
  ENABLE					/* Re-enable interrupts */
  StartIORBQueue (pQueue);			/* Try to restart the queue */
 }
}

/* Function to add an IORB to a queue of IORBs maintainded for the unit. */
/* MUST be called with interrupts disabled. */
void AddIORBToQueue (NPIORBQueue pQueue, PIORB pIORB)
{
 if (pQueue->pTail)				/* Queue had IORBs waiting: */
  pQueue->pTail->pNxtIORB= pIORB;		/* Add IORB into queue */
 else						/* Queue was empty: */
  pQueue->pHead= pIORB;				/* Add IORB into queue */
 while (pIORB->RequestControl&IORB_CHAIN)	/* If not last IORB in chain */
  pIORB= pIORB->pNxtIORB;			/* Seek next IORB in request */
 (pQueue->pTail= pIORB)->pNxtIORB= 0;		/* Set new tail pointer and */
						/* clear ptr to chained IORB */
}

/* Function retrieve an IORB from the queue. Returns NULL if the queue was */
/* empty. MUST be called with interrupts disabled. */
PIORB GetIORBFromQueue (NPIORBQueue pQueue)
{
 PIORB	pReturnIORB;				/* Pointer to return */

 if (pQueue->pHead)				/* If queue is not empty: */
 {
  pReturnIORB= pQueue->pHead;			/* Get pointer to head */
  pQueue->pHead= pReturnIORB->pNxtIORB;		/* Update the head pointer */
  if (!pQueue->pHead)				/* If queue now is empty: */
   pQueue->pTail= NULL;				/* Clear the tail pointer */
 }
 else
  pReturnIORB= NULL;				/* Queue empty: return NULL */
 return pReturnIORB;
}

/* Function to service an IORB queue. It first checks that the queue is not */
/* being serviced before entering the service loop. */
void StartIORBQueue (NPIORBQueue pQueue)
{
 PIORB			pIORB;			/* Ptr to IORB to execute */
 NPRecHeader		pUnitRec;		/* Pointer to unit record */

 DISABLE					
 if (!(pQueue->Flags&F_SERVER_ACTIVE))		/* Check to no one is busy */
						/* servicing this queue */
 {
  pQueue->Flags|=F_SERVER_ACTIVE;		/* Grap control of queue */
  /* If we get where we have exclusize access to the queue. */
  while ((!(pQueue->Flags&F_REQUEST_BUSY))&&
         ((pIORB= GetIORBFromQueue (pQueue))!=NULL))
						/* Loop while there is an */
						/* IORB ready to be serviced */
						/* and the previous IORB is */
						/* finished. */
  {
   pQueue->Flags|=F_REQUEST_BUSY;		/* Show request in progress */
   ENABLE

   if (pIORB->CommandCode==IOCC_CONFIGURATION)	/* For configuration IORBs: */
    pUnitRec= (NPRecHeader) VirtUnits;		/* Assign IORB to the first */
						/* virtual unit. (For queue) */
   else
    pUnitRec= (NPRecHeader) pIORB->UnitHandle;	/* Get pointer to unit */
						/* from the unit handle */

   /* We will handle (de)allocation and allocation checks here since it is */
   /* common between the two parts- filter and virtual units. */
   if (pIORB->CommandCode==IOCC_UNIT_CONTROL)	/* Unit control command? */
   {
    if (pIORB->CommandModifier==IOCM_ALLOCATE_UNIT)
    {						/* Allocate unit??? */
     if (pUnitRec->Flags&F_ALLOCATED)		
      IORBError (pIORB,IOERR_UNIT_ALLOCATED);	/* Error if allocated */
     else
      pUnitRec->Flags|=F_ALLOCATED;		/* Else allocate the unit */
     pQueue->Flags&=~F_REQUEST_BUSY;		/* Indicate queue finished */
     NotifyDone (pIORB);
     continue;					/* Service next request */
    }
    if (pIORB->CommandModifier==IOCM_DEALLOCATE_UNIT)
    {						/* Deallocate unit??? */
     if (!(pUnitRec->Flags&F_ALLOCATED))	
      IORBError (pIORB,IOERR_UNIT_NOT_ALLOCATED);
						/* Error if not allocated */
     else
      pUnitRec->Flags&=~F_ALLOCATED;		/* Else deallocate unit */
     pQueue->Flags&=~F_REQUEST_BUSY;		/* Indicate queue finished */
     NotifyDone (pIORB);
     continue;					/* Service next request */
    }
   }

   /* Do allocations checks... if notify points to us, skip check. */
   if (!(pUnitRec->Flags&F_ALLOCATED))		/* If unit is not allocated: */
    if (pIORB->NotifyAddress!=&PartNotifyWrapper)
						/* and we didn't make request */
     if (pIORB->CommandCode!=IOCC_CONFIGURATION)
						/*  and not configuration */
     {
      IORBError (pIORB,IOERR_UNIT_NOT_ALLOCATED);
						/* Then it is an error */
      pQueue->Flags&=~F_REQUEST_BUSY;		/* Indicate queue finished */
      NotifyDone (pIORB);
      continue;					/* Service next request */
     }


   if (IS_ELEMENT(pUnitRec,BaseUnits,NumBaseUnits)) 
    FilterHandler (pIORB);			/* Handler for base units */
   else if (IS_ELEMENT(pUnitRec,VirtUnits,NumVirtUnits))
    PartHandler (pIORB);			/* Handler for virtual units */
   DISABLE
  }
  /* Tell others that this server is not active any more... */
  pQueue->Flags&=~F_SERVER_ACTIVE;
 }
 ENABLE
}

/* Notify client that the IORB processing is completed. */
void NotifyDone (PIORB pIORB)
{
 pIORB->Status |= IORB_DONE;			/* Set 'DONE' flag */
 if (pIORB->RequestControl&IORB_ASYNC_POST)	/* If notify flag set... */
  pIORB->NotifyAddress(pIORB);			/* Notify the caller */
}

/* Function to return an error code via IORB */
void IORBError (PIORB pIORB, USHORT ErrorCode)
{
 if (ErrorCode)					/* If really an error: */
 {
  pIORB->Status |= IORB_ERROR;			/* Set error flag */
  pIORB->ErrorCode= ErrorCode;			/* Fill in error code field */
 }
}
