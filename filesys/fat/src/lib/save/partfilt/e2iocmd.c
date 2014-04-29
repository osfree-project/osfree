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

/* Command thats called when we receive an unknown/unsupported IORB command */

void BadCommand (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
   IORBError(pIORB,IOERR_CMD_NOT_SUPPORTED);	/* Set error */
   PartCommandDone (pUnitRec,pIORB);		/* Send it away. */
}

/* Macro to compute the length of the device table that we will create */
#define DEVICE_TABLE_LEN	sizeof(DEVICETABLE)+\
				sizeof(ADAPTERINFO)*(MountCount)+\
				sizeof(NPADAPTERINFO)*(MountCount-1)

/* Command to create a device table. */

void PartGetDeviceTable (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
#define pIOCFG		((PIORB_CONFIGURATION) pIORB)

NPVirtUnitRec		pCurrentUnitRec;	/* Ptr to UnitRec current */
					/* entry in the device table */
PDEVICETABLE		pDevTable;		/* Where to build dev table */
PADAPTERINFO		pAdapter;		/* Ptr to adapter entry */
UNITINFO		UnitInfo;		/* Default UnitInfo returned */
int			Count;

   if (pIOCFG->DeviceTableLen < DEVICE_TABLE_LEN)	/* Check table not too big */
      IORBError(pIORB,IOERR_CMD_SYNTAX);
   else
      {
      /* Static unit information... */

      UnitInfo.UnitIndex= 0;
      UnitInfo.UnitFlags= UF_NOSCSI_SUPT;
      UnitInfo.FilterADDHandle= 0;
      UnitInfo.UnitType= UIB_TYPE_DISK;
      UnitInfo.QueuingCount= 1;
      UnitInfo.UnitSCSITargetID= 0;
      UnitInfo.UnitSCSILUN= 0;
 
      /* Fill in device section... */
      pDevTable= pIOCFG->pDeviceTable;
      pDevTable->ADDLevelMajor= ADD_LEVEL_MAJOR;
      pDevTable->ADDLevelMinor= ADD_LEVEL_MINOR;
      pDevTable->ADDHandle= ADDHandle;
      pDevTable->TotalAdapters= MountCount;	/* One adapter for each unit */

      for (Count= 0,pAdapter=(PADAPTERINFO) &(pDevTable->pAdapter[MountCount]);
            Count<MountCount;
            Count++,pAdapter++)
         {
         /* Get pointer to the next mounted unit. */
         pCurrentUnitRec= VirtUnits+MountTable[Count];
         /* Fill in pointer to the this adapter */
         pDevTable->pAdapter[Count]= (NPADAPTERINFO) pAdapter;
         /* Fill in adapter section for each linux partition... */
         memcpy (pAdapter,&(pCurrentUnitRec->pSourceUnitRec->AdapterInfo),
               sizeof(ADAPTERINFO)-sizeof(UNITINFO));
         /* Fill in unit information for this unit... */
         UnitInfo.AdapterIndex= Count;
         UnitInfo.UnitHandle= (USHORT) pCurrentUnitRec;
         if (pCurrentUnitRec->pUnitInfo)		/* If UnitInfo was changed */
         memcpy (pAdapter->UnitInfo,
                  pCurrentUnitRec->pUnitInfo,
                  pCurrentUnitRec->UnitInfoLen);
         else						/* Else use default */
         memcpy (pAdapter->UnitInfo,
               &UnitInfo,
               sizeof(UnitInfo));
         }
      }
   PartCommandDone (pUnitRec,pIORB);

#undef pIOCFG
}

/* Command to change default unit information */
void PartChangeUnitInfo (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
#define pIOUC		((PIORB_UNIT_CONTROL) pIORB)

   pUnitRec->pUnitInfo= pIOUC->pUnitInfo;		/* Store ptr to new UnitInfo */
   pUnitRec->UnitInfoLen= pIOUC->UnitInfoLen;
   PartCommandDone (pUnitRec,pIORB);

#undef pIOUC
}

/* Command to return the geometry of the virtual unit */
void PartGetGeometry (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
#define pIOG		((PIORB_GEOMETRY) pIORB)

USHORT	NumHeads=	pUnitRec->pSourceUnitRec->GeoNumHeads;
					/* Num heads of base unit */
USHORT	TrackSectors=	pUnitRec->pSourceUnitRec->GeoTrackSec;
					/* Num sectors/track of base */
ULONG	SectorsPerCylinder= NumHeads*TrackSectors;
ULONG	DiskSize;

   if (pIOG->GeometryLen!=sizeof(GEOMETRY))	/* Check storage size */
      IORBError(pIORB,IOERR_CMD_SYNTAX);
    else
      {
      DiskSize= pUnitRec->NumSectors+pUnitRec->NumExtraSectors;
						      /* Compute disk size needed */
      pIOG->pGeometry->BytesPerSector= SECTOR_SIZE;	/* Fill in sector size */
      pIOG->pGeometry->NumHeads= NumHeads;		/* Fill in geometry info... */
      pIOG->pGeometry->SectorsPerTrack= TrackSectors;
      pIOG->pGeometry->TotalCylinders= ((DiskSize-1)/SectorsPerCylinder)+1;
						      /* Compute num cylinders */
      pIOG->pGeometry->TotalSectors= pIOG->pGeometry->NumHeads*
                                       pIOG->pGeometry->TotalCylinders*
                                       pIOG->pGeometry->SectorsPerTrack;
      }

   PartCommandDone (pUnitRec,pIORB);

#undef pIOG
}

/* Command to execute IO related IORBs. */
void PartDoIO (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
#define pIOXIO		((PIORB_EXECUTEIO) pIORB)

ULONG	SectorCount;
USHORT	Result;

   /* Check write permission flag for write operations... */
   if ((!(pUnitRec->Hdr.Flags & F_ALLOW_WRITE))&&
     ((pIORB->CommandModifier==IOCM_WRITE)||
      (pIORB->CommandModifier==IOCM_WRITE_VERIFY)))
      {
      IORBError(pIORB,IOERR_MEDIA_WRITE_PROTECT);
      PartCommandDone (pUnitRec,pIORB);
      return;
      }
 
/*BREAK*/
 
 /* Do IO on virtual sectors... */
 for (SectorCount = pIOXIO->RBA;
      (SectorCount < pUnitRec->NumVirtualSectors)&&
      ((SectorCount-pIOXIO->RBA)<pIOXIO->BlockCount);
      SectorCount++)
 {
  Result= DoVirtualIO (pIORB->CommandModifier,pUnitRec,SectorCount,
                       pIOXIO->pSGList,pIOXIO->cSGList,
                       (SectorCount-pIOXIO->RBA)*SECTOR_SIZE);
  if (Result)
  {
   IORBError(pIORB,Result);
   PartCommandDone (pUnitRec,pIORB);
   return;  
  }
  pIOXIO->BlocksXferred++;			/* Increase num transerfered */
 }

 /* 'Transfer' left-over sectors between the end of the partition and unit */
 if ((pIOXIO->RBA+pIOXIO->BlockCount)>
     (pUnitRec->NumSectors+pUnitRec->NumExtraSectors))
 {						/* Just increase xfer count */
  /* BREAK */
  if (pIOXIO->RBA>(pUnitRec->NumSectors+pUnitRec->NumExtraSectors))
   pIOXIO->BlocksXferred= pIOXIO->BlockCount;	/* Doesn't for for partitions */
						/* with less than the number */
						/* of virtual sectors... ! */
  else
   pIOXIO->BlocksXferred+= (pIOXIO->RBA+pIOXIO->BlockCount)-
                          (pUnitRec->NumSectors+pUnitRec->NumExtraSectors);
 }

 /* If we need some real sectors, call original driver, else done... */
 if ((pIOXIO->RBA+pIOXIO->BlockCount)>pUnitRec->NumVirtualSectors)
  PartFilterIORB (pUnitRec,pIORB);		/* Call driver */
 else
  PartCommandDone (pUnitRec,pIORB);		/* Done! */

#undef pIOXIO
}

/* Command to get unit status: simply send request to original driver. */
void PartGetUnitStatus (NPVirtUnitRec pUnitRec, PIORB pIORB)
{
 PartFilterIORB (pUnitRec,pIORB);
}
