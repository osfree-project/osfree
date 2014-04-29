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
#include "e2wrap.h"
#include "e2inutil.h"

UCHAR		InitIORB[MAX_IORB_SIZE]= {0};	/* IORB to use */

/* Add a unit that we might want to use to the base unit list after */
/* allocating the unit. */

USHORT InitAddBaseUnit (PADDEntryPoint pADDEntry, PADAPTERINFO pAdapterInfo)
{
NPBaseUnitRec	pBaseUnit;				
USHORT		   UnitHandle;				
USHORT		   Result;

   if (NumBaseUnits == MAX_LINUX_PARTITIONS)	/* Check table full */
      return 0xFFFF;
   pBaseUnit = BaseUnits+NumBaseUnits;		/* Pointer to new entry */
   UnitHandle= pAdapterInfo->UnitInfo->UnitHandle; 
   if ((Result=InitAllocateUnit (pADDEntry,UnitHandle))!=0)
		            				/* Try to allocate unit */
      return Result;				/* If failed, return error */

   /* Fill first free base unit record with info for current unit */
   pBaseUnit->Hdr.Flags= 0;
   pBaseUnit->UnitHandle= pAdapterInfo->UnitInfo->UnitHandle;
   pBaseUnit->pADDEntry= pADDEntry;

   /* If virtualize all parts: */
	/* Disable DASD support for */
	/* the original base unit. */

   if (InstallFlags & FI_ALLPART)
      pAdapterInfo->UnitInfo->UnitFlags |= UF_NODASD_SUPT;

   memcpy (&pBaseUnit->AdapterInfo, pAdapterInfo, sizeof(ADAPTERINFO));

   /* Get geometry for this unit; ignore any errors getting it. */
   InitGetGeometry (pBaseUnit->pADDEntry,pBaseUnit->UnitHandle,
                  &(pBaseUnit->GeoNumHeads),&(pBaseUnit->GeoTrackSec));
   NumBaseUnits++;					/* Keep this entry */
   return 0;
}

/* Function to remove the last unit in the BaseUnits array. Deallocates the */
/* unit. Should not be called for 'commited' units. */

USHORT InitRemoveBaseUnit (void)
{
USHORT		UnitHandle;
PADDEntryPoint	pADDEntry;

   NumBaseUnits--;					/* Remove unit */
   UnitHandle= BaseUnits[NumBaseUnits].UnitHandle;
   pADDEntry= BaseUnits[NumBaseUnits].pADDEntry;
   return InitFreeUnit (pADDEntry,UnitHandle); 		/* Deallocate unit */
}

/* Function to install our filter on a base unit. Called after we found a */
/* partition that we want to keep on this unit. */
USHORT InitFilterBaseUnit (int Index)
{
NPBaseUnitRec	pBaseUnit;				
USHORT		Result;

   pBaseUnit= BaseUnits+Index;			/* Pointer to last base unit */
   pBaseUnit->AdapterInfo.UnitInfo->UnitHandle= (USHORT) pBaseUnit;
   pBaseUnit->AdapterInfo.UnitInfo->FilterADDHandle= ADDHandle;
   Result= InitChangeUnitInfo (pBaseUnit->pADDEntry,pBaseUnit->UnitHandle,
                             pBaseUnit->AdapterInfo.UnitInfo);
   if (Result)					/* Error changing info... */
      pBaseUnit->AdapterInfo.AdapterUnits= 0;	/* No units on adapter */
   return Result;
}

int InitAddVirtualUnit (ULONG StartRBA, ULONG NumSectors, UCHAR SysIndicator)
{
NPVirtUnitRec	pVirtUnit;
ULONG		NumShort;
ULONG		SectorsPerCylinder;
USHORT		GeoNumHeads;
USHORT		GeoTrackSec;
 
 
   if (NumVirtUnits == MAX_LINUX_PARTITIONS)		/* Check table full */
      return 0;

   pVirtUnit= VirtUnits+(NumVirtUnits++);			/* Grab next entry */
   pVirtUnit->Hdr.Flags= 0;

   if (InstallFlags & FI_ALLOWWRITE)			   /* If writes allowed: */
      pVirtUnit->Hdr.Flags|= F_ALLOW_WRITE;		/* Make unit writable */

   pVirtUnit->PartSysIndicator = SysIndicator;		/* Save sys indicator */
   pVirtUnit->pSourceUnitRec = BaseUnits+NumBaseUnits-1;
   pVirtUnit->StartRBA = StartRBA;
   pVirtUnit->NumSectors = NumSectors;

   GeoNumHeads = pVirtUnit->pSourceUnitRec->GeoNumHeads;
   GeoTrackSec = pVirtUnit->pSourceUnitRec->GeoTrackSec;
   SectorsPerCylinder = GeoNumHeads*GeoTrackSec;

   NumShort = pVirtUnit->NumSectors % SectorsPerCylinder;
   if (NumShort)
      NumShort = SectorsPerCylinder - NumShort;

   pVirtUnit->NumExtraSectors = NumShort +
	   SectorsPerCylinder * +((NumShort < GeoTrackSec) ? 2 : 1);

   pVirtUnit->NumVirtualSectors= pVirtUnit->NumExtraSectors;
   return 1;
}

/* Function to read the device table from a adapter device driver */
USHORT InitReadDevTable (PADDEntryPoint pAddEP, PDEVICETABLE pDevTable,
                         USHORT DevTableLen)
{
#define pIOCF	((PIORB_CONFIGURATION) InitIORB)

 pIOCF->iorbh.Length= sizeof(IORB_CONFIGURATION);
 pIOCF->iorbh.UnitHandle= 0;
 pIOCF->iorbh.CommandCode= IOCC_CONFIGURATION;
 pIOCF->iorbh.CommandModifier= IOCM_GET_DEVICE_TABLE;

 pIOCF->pDeviceTable= pDevTable;
 pIOCF->DeviceTableLen= DevTableLen;

 return InitSendIORB((PIORB) pIOCF,pAddEP);

#undef pIOCF
}

/* Function to allocate a unit. Needed before any function except */
/* InitReadDevTable */
USHORT InitAllocateUnit (PADDEntryPoint pAddEP, USHORT UnitHandle)
{
#define pIOUC	((PIORB_UNIT_CONTROL) InitIORB)

 pIOUC->iorbh.Length= sizeof(IORB_UNIT_CONTROL);
 pIOUC->iorbh.UnitHandle= UnitHandle;
 pIOUC->iorbh.CommandCode= IOCC_UNIT_CONTROL;
 pIOUC->iorbh.CommandModifier= IOCM_ALLOCATE_UNIT;
 pIOUC->Flags= 0;
 return InitSendIORB((PIORB) pIOUC,pAddEP);

#undef pIOUC
}

/* Function to release an allocated unit. */
USHORT InitFreeUnit (PADDEntryPoint pAddEP, USHORT UnitHandle)
{
#define pIOUC	((PIORB_UNIT_CONTROL) InitIORB)

 pIOUC->iorbh.Length= sizeof(IORB_UNIT_CONTROL);
 pIOUC->iorbh.UnitHandle= UnitHandle;
 pIOUC->iorbh.CommandCode= IOCC_UNIT_CONTROL;
 pIOUC->iorbh.CommandModifier= IOCM_DEALLOCATE_UNIT;
 pIOUC->Flags= 0;
 return InitSendIORB((PIORB) pIOUC,pAddEP);

#undef pIOUC
}

/* Function to change the unit information of a unit. */
USHORT InitChangeUnitInfo (PADDEntryPoint pAddEP, USHORT UnitHandle,
                           PUNITINFO pUnitInfo)
{
#define pIOUC	((PIORB_UNIT_CONTROL) InitIORB)

 pIOUC->iorbh.Length= sizeof(IORB_UNIT_CONTROL);
 pIOUC->iorbh.UnitHandle= UnitHandle;
 pIOUC->iorbh.CommandCode= IOCC_UNIT_CONTROL;
 pIOUC->iorbh.CommandModifier= IOCM_CHANGE_UNITINFO;
 pIOUC->Flags= 0;
 pIOUC->pUnitInfo= pUnitInfo;
 pIOUC->UnitInfoLen= sizeof(UNITINFO);
 return InitSendIORB((PIORB) pIOUC,pAddEP);

#undef pIOUC
}

/* Function to get disk geometry of an unit. */
USHORT InitGetGeometry (PADDEntryPoint pAddEP, USHORT UnitHandle,
                        USHORT FAR *NumHeads, USHORT FAR *TrackSectors)
{
#define pIOG	((PIORB_GEOMETRY) InitIORB)

 GEOMETRY	Geometry;
 USHORT		Result;

 pIOG->iorbh.Length= sizeof(IORB_GEOMETRY);
 pIOG->iorbh.UnitHandle= UnitHandle;
 pIOG->iorbh.CommandCode= IOCC_GEOMETRY;
 pIOG->iorbh.CommandModifier= IOCM_GET_DEVICE_GEOMETRY;
 pIOG->pGeometry= &Geometry;
 pIOG->GeometryLen= sizeof (Geometry);
 memset (&Geometry, 0, sizeof(Geometry));

 Result= InitSendIORB((PIORB) pIOG,pAddEP);
 *NumHeads= Geometry.NumHeads;
 *TrackSectors= Geometry.SectorsPerTrack;
 if ((Result)||(*(NumHeads)==0)||(*(TrackSectors)==0))
 {
  *NumHeads= 32;
  *TrackSectors= 32;
 }
 return Result;

#undef pIOG
}

/* Reads a sector into the buffer at physical address ppSectorBuf. */
USHORT InitReadSector (PADDEntryPoint pAddEP, USHORT UnitHandle,
                       ULONG SectorRBA, ULONG ppSectorBuf)
{
#define pIOXIO	((PIORB_EXECUTEIO) InitIORB)

 PSCATGATENTRY		pScatGat;
 
/* Fill in the scatter/gather list...*/
 pScatGat= (PSCATGATENTRY) pIOXIO->iorbh.DMWorkSpace;
 pScatGat->XferBufLen= SECTOR_SIZE;
 pScatGat->ppXferBuf= ppSectorBuf;

 pIOXIO->iorbh.Length= sizeof (IORB_EXECUTEIO);
 pIOXIO->iorbh.UnitHandle= UnitHandle;
 pIOXIO->iorbh.CommandCode= IOCC_EXECUTE_IO;
 pIOXIO->iorbh.CommandModifier= IOCM_READ;
 pIOXIO->cSGList= 1;
 pIOXIO->pSGList= pScatGat;
 pIOXIO->ppSGList= ppDataSeg+OFFSETOF((void FAR*)pScatGat);
 pIOXIO->RBA= SectorRBA;
 pIOXIO->BlockCount= 1;
 pIOXIO->BlocksXferred= 0;
 pIOXIO->BlockSize= SECTOR_SIZE;
 pIOXIO->Flags= 0;
 return InitSendIORB ((PIORB) pIOXIO,pAddEP);

#undef PIOXIO
}

/* Function to submit an IORB. Waits for request to finish before returning */
USHORT InitSendIORB(PIORB pIORB, PADDEntryPoint pADDEntry)
{
 pIORB->NotifyAddress  = (void far *) ProcRun;
 pIORB->RequestControl = IORB_ASYNC_POST;
 pIORB->ErrorCode      = 0;
 pIORB->Status         = 0;

 (*pADDEntry)(pIORB);

 DISABLE
 while (!(pIORB->Status & IORB_DONE))
 {
  ProcBlock(pIORB);
  DISABLE
 }
 ENABLE
 return pIORB->ErrorCode;
}
