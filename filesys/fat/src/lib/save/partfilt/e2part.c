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
#include "e2part.h"
#include "e2iocmd.h"
#include "e2virtio.h"
#include "e2wrap.h"

/* All IORBs for the virtual units (containing the virtual partitions) */
/* arrive here. From here it is routed to the function to service the */
/* command. */
void PartHandler (PIORB pIORB)
{
 NPVirtUnitRec	pUnitRec;			/* Pointer to unit record */
 USHORT		Command;			/* Command to executed */
 USHORT		Modifier;			/* Modifier for the command */

 Command= pIORB->CommandCode;			/* Get command to execute */
 Modifier= pIORB->CommandModifier;		/* Get modifier for command */
 if (Command==IOCC_CONFIGURATION)
  pUnitRec= VirtUnits;				/* Default unit for the */
						/* configuration request */
 else
  pUnitRec= (NPVirtUnitRec) pIORB->UnitHandle;	/* Get unit record for IORB */

 /* Route request to appropiate service routine. */
 switch (Command)
 {
  case IOCC_CONFIGURATION:
		switch (Modifier)
		{
		 case IOCM_GET_DEVICE_TABLE:
				PartGetDeviceTable (pUnitRec,pIORB);
				break;
		 default:	BadCommand (pUnitRec,pIORB);
				break;
		}
		break;
  case IOCC_UNIT_CONTROL:
		switch (Modifier)
		{
		 case IOCM_CHANGE_UNITINFO:
				PartChangeUnitInfo (pUnitRec,pIORB);
				break;
		 default:	BadCommand (pUnitRec,pIORB);
				break;
		}
		break;
  case IOCC_GEOMETRY:
		switch (Modifier)
		{
		 case IOCM_GET_MEDIA_GEOMETRY:
		 case IOCM_GET_DEVICE_GEOMETRY:
				PartGetGeometry (pUnitRec,pIORB);
				break;
		 default:	BadCommand (pUnitRec,pIORB);
				break;
		}
		break;
  case IOCC_EXECUTE_IO:
		switch (Modifier)
		{
		 case IOCM_READ:
		 case IOCM_READ_VERIFY:
		 case IOCM_WRITE:
		 case IOCM_WRITE_VERIFY:
				PartDoIO (pUnitRec,pIORB);
				break;
		 default:	BadCommand (pUnitRec,pIORB);
				break;
		}
		break;
  case IOCC_UNIT_STATUS:
		switch (Modifier)
		{
		 case IOCM_GET_UNIT_STATUS:
				PartGetUnitStatus (pUnitRec,pIORB);
				break;
		 default:	BadCommand (pUnitRec,pIORB);
				break;
		}
		break;
  default:	BadCommand (pUnitRec,pIORB);
		break;
 }
}

/* This function is called by the original device driver after the IORB */
/* request is satisfied. We restore the IORB before we pass it back to the */
/* caller. */
void PartNotify (PIORB pIORB)
{
#define pIOXIO		((PIORB_EXECUTEIO) pIORB)

 NPVirtUnitRec	pUnitRec;			/* Ptr to unit record */
 ULONG		PatchOffset;			/* Used to fix up boot sec */
 ULONG		PatchValue;

 if (pIORB->CommandCode==IOCC_CONFIGURATION)	/* If configuration IORB */
  pUnitRec= VirtUnits;				/* Default unit for the */
						/* configuration request */
 else
  pUnitRec= (NPVirtUnitRec) pIORB->Reserved_1;	/* Get unit record from IORB */

 /* Reverse any mappings that may have been done... */
 pIORB->UnitHandle= pIORB->Reserved_1;		/* Restore IORB fields */
 pIORB->RequestControl= pUnitRec->SaveReqCtrl; 
 pIORB->Reserved_1= pUnitRec->SaveReserved;
 pIORB->NotifyAddress= pUnitRec->SaveNotify;
 if (pIORB->CommandCode==IOCC_EXECUTE_IO)	/* If request involded IO */
 {
  /* Restore RBA and BlockCount fields, modify BlocksXferred... */
  pIOXIO->RBA= pUnitRec->SaveRBA;		
  pIOXIO->BlockCount= pUnitRec->SaveBlockCount;
  pIOXIO->BlocksXferred+= pUnitRec->SectorsDone;

  /* If the scatter/gather list was modified then restore it as well. */
  if ((pUnitRec->SGOffset)&&(pIORB->CommandModifier!=IOCM_READ_VERIFY))
  {
   /* If we didn't run out of SG list elements... */
   if (pIOXIO->cSGList)
    memcpy (pIOXIO->pSGList,&(pUnitRec->SaveSGEntry),sizeof(SCATGATENTRY));
   pIOXIO->cSGList= pUnitRec->SavecSGList;
   pIOXIO->pSGList= pUnitRec->SavepSGList;
   pIOXIO->ppSGList= pUnitRec->SaveppSGList;
  }

  /* OK first 'patch' here: if the FS type is FAT and the bootsector was */
  /* read them we must change the 'Hidden Sectors' field. */
  if ((pIOXIO->RBA<=pUnitRec->NumExtraSectors)&&
      ((pIOXIO->RBA+pIOXIO->BlockCount)>pUnitRec->NumExtraSectors))
  {
   PatchOffset= 0x1C+(pUnitRec->NumExtraSectors-pIOXIO->RBA)*SECTOR_SIZE;
   if ((pIORB->CommandModifier==IOCM_READ)||
       (pIORB->CommandModifier==IOCM_WRITE)||
       (pIORB->CommandModifier==IOCM_WRITE_VERIFY))  
   {
    PatchValue= pUnitRec->NumExtraSectors-
                (pUnitRec->pSourceUnitRec->GeoNumHeads*
                 pUnitRec->pSourceUnitRec->GeoTrackSec); 
    CopyToSGList (&PatchValue,sizeof(PatchValue),
                  pIOXIO->pSGList,pIOXIO->cSGList,PatchOffset);
   }
  }
 }
 /* Notify the caller and restart the queue. */
 PartCommandDone (pUnitRec,pIORB);

#undef pIOXIO
}

/* Pass the IORB to the base unit containing the source data for the virtual */
/* unit. Remap IO addresses to map virtual partition unto base data. */
void PartFilterIORB (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
#define pIOXIO		((PIORB_EXECUTEIO) pIORB)

 USHORT		SectorsDone;			/* Num virtual sectors */
						/* allready serviced */
 ULONG		XferOffset;			/* Offset in SG to start IO */
 ULONG		PatchOffset;			/* Used to fix up boot sec */
 
 pUnitRec->SaveReqCtrl= pIORB->RequestControl;	/* Save request control */
 pUnitRec->SaveReserved= pIORB->Reserved_1;	/* Save reserved field */
 pUnitRec->SaveNotify= pIORB->NotifyAddress;	/* Save old notify address */
 pIORB->Reserved_1= pIORB->UnitHandle;		/* Save our old handle */

 pIORB->RequestControl|= IORB_ASYNC_POST;	/* Ask for notification */
 pIORB->RequestControl&= ~IORB_CHAIN;		/* Disable chained request */
 pIORB->NotifyAddress= &PartNotifyWrapper;	/* Set our notify address */
 pIORB->UnitHandle= (USHORT) pUnitRec->pSourceUnitRec;
						/* Handle for our filter */

 /* Patch up IORB further for IO requests. */
 if (pIORB->CommandCode==IOCC_EXECUTE_IO)	/* If IO command... */
 {
  /* Patch hidden sector count */
  if ((pIOXIO->RBA<=pUnitRec->NumExtraSectors)&&
      ((pIOXIO->RBA+pIOXIO->BlockCount)>pUnitRec->NumExtraSectors))
  {
   PatchOffset= 0x1C+(pUnitRec->NumExtraSectors-pIOXIO->RBA)*SECTOR_SIZE;
   if ((pIORB->CommandModifier==IOCM_WRITE)||
       (pIORB->CommandModifier==IOCM_WRITE_VERIFY))  
   {
    CopyToSGList (&pUnitRec->HiddenSectors,sizeof(ULONG),
                  pIOXIO->pSGList,pIOXIO->cSGList,PatchOffset);
   }
  }

  pUnitRec->SaveBlockCount= pIOXIO->BlockCount;	/* Save IORB IO fields */
  pUnitRec->SectorsDone= pIOXIO->BlocksXferred;
  pUnitRec->SaveRBA= pIOXIO->RBA;

  pIOXIO->BlockCount-= pIOXIO->BlocksXferred;	/* Subtract num blocks we */
						/* allready transfered
  pIOXIO->BlocksXferred= 0;			/* Reset transfer count */

  /* Calculate the start sector for the request... */
  if (pIOXIO->RBA<pUnitRec->NumVirtualSectors)		
  {						/* Includes virtual sectors */
   XferOffset= (pUnitRec->NumVirtualSectors-pIOXIO->RBA)*SECTOR_SIZE;
						/* Calculate number of bytes */
						/* to skip past virt sectors */ 
   pIOXIO->RBA= pUnitRec->NumVirtualSectors-pUnitRec->NumExtraSectors+
                pUnitRec->StartRBA;		/* Calc starting RBA address */
  }
  else
  {						/* Past virtual sectors */
   XferOffset= 0;				/* No need to skip bytes */
   pIOXIO->RBA= pIOXIO->RBA-pUnitRec->NumExtraSectors+pUnitRec->StartRBA;
						/* Calc starting RBA address */
  }
  pUnitRec->SGOffset= XferOffset;		/* Save transfer offset */

  /* CHECK WAYS OF CHOPPING BLOCK COUNT */
  /* Safety net: chop block count if neccessary: */
  if ((pIOXIO->RBA+pIOXIO->BlockCount)>		/* If past source data */
      (pUnitRec->StartRBA+pUnitRec->NumSectors))	
  {
   BREAK
   pIOXIO->BlockCount= 0;			/* Tough: kill request... */
  }

  /* Also modify scatgat list if neccessary... */
  if ((XferOffset)&&(pIORB->CommandModifier!=IOCM_READ_VERIFY))
  {
   /* First save original pointers... */
   pUnitRec->SavecSGList= pIOXIO->cSGList;
   pUnitRec->SavepSGList= pIOXIO->pSGList;
   pUnitRec->SaveppSGList= pIOXIO->ppSGList;

   /* Seek to the offset to start IO */
   while ((pIOXIO->cSGList)&&(pIOXIO->pSGList->XferBufLen<=XferOffset))
   {						/* Skip past SG entries */
    XferOffset-=(pIOXIO->pSGList++)->XferBufLen;
    pIOXIO->cSGList--;
    pIOXIO->ppSGList+= sizeof (SCATGATENTRY);
   }

   /* If we didn't run out of SG list elements... */
   if (pIOXIO->cSGList)
   {
    memcpy (&(pUnitRec->SaveSGEntry),pIOXIO->pSGList,sizeof(SCATGATENTRY));
    pIOXIO->pSGList->XferBufLen-= XferOffset;
    pIOXIO->pSGList->ppXferBuf+= XferOffset;
   }
  }
 }
 E2FilterIORB (pIORB);		/* Ask our handler to handle it further... */

#undef pIOXIO
}

/* Function that is called when the routine that services the IORB has */
/* finished processing that IORB. We notify the caller and try to restart */
/* the IORB queue. */
void PartCommandDone (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
 NPIORBQueue	pQueue;				/* Pointer to IORB queue */

 pQueue= &(pUnitRec->Hdr.IORBQueue);		/* Get pointer to queue */
 pQueue->Flags&=~F_REQUEST_BUSY;		/* Indicate queue finished */
 NotifyDone (pIORB);				/* Notify caller */
 StartIORBQueue (pQueue);			/* Try to restart queue */
}
