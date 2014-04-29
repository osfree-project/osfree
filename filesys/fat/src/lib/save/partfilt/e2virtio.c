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
#include "e2virtio.h"
#include "e2wrap.h"

USHORT DoVirtualIO (USHORT Modifier, NPVirtUnitRec pUnitRec, ULONG SectorRBA,
                    PSCATGATENTRY pSGList, USHORT cSGList,ULONG XferOffset)
{
 USHORT	Result;

 switch (Modifier)
 {
  case IOCM_READ:
	Result= ReadFakeSector(pUnitRec,SectorRBA,pSGList,cSGList,XferOffset);
	break;
  case IOCM_READ_VERIFY:
	Result= 0;		/* Nothing to do for verify. */
	break;	
  case IOCM_WRITE:
  case IOCM_WRITE_VERIFY:
	Result= WriteFakeSector(pUnitRec,SectorRBA,pSGList,cSGList,XferOffset);
	break;
 }
 return Result;
}

USHORT ReadFakeSector (NPVirtUnitRec pUnitRec, ULONG SectorRBA,
                       PSCATGATENTRY pSGList, USHORT cSGList,
                       ULONG XferOffset)
{
MBR		FakeMBR;
ULONG		DiskSize;

USHORT	GeoNumHeads =	pUnitRec->pSourceUnitRec->GeoNumHeads;
USHORT	GeoTrackSec =	pUnitRec->pSourceUnitRec->GeoTrackSec;
ULONG		SectorsPerCylinder= GeoNumHeads*GeoTrackSec;

   memset (&FakeMBR,0,sizeof(MBR));

   DiskSize = pUnitRec->NumSectors+pUnitRec->NumExtraSectors;

   if (!SectorRBA)
      {
      FakeMBR.Signature=0xAA55;
      FakeMBR.PartitionTable[0].SysIndicator= PARTITION_EXTENDED;
      FakeMBR.PartitionTable[0].RelativeSectors= SectorsPerCylinder;
      FakeMBR.PartitionTable[0].NumSectors= DiskSize-SectorsPerCylinder;
      ComputeCHS (SectorsPerCylinder,GeoNumHeads,GeoTrackSec,
                  &(FakeMBR.PartitionTable[0].BegSecCyl),
                  &(FakeMBR.PartitionTable[0].BegHead));
      ComputeCHS (DiskSize-1,GeoNumHeads,GeoTrackSec,
                  &(FakeMBR.PartitionTable[0].EndSecCyl),
                  &(FakeMBR.PartitionTable[0].EndHead));
      }
   else if (SectorRBA == SectorsPerCylinder)
      { 
      FakeMBR.Signature=0xAA55;
      switch (pUnitRec->PartSysIndicator & ~PARTITION_HIDDEN)
         {
         case PARTITION_BOOTMANAGER:
         case PARTITION_DOSSMALL:
         case PARTITION_DOSMED:
         case PARTITION_DOSLARGE:
         case PARTITION_IFS:
            FakeMBR.PartitionTable[0].SysIndicator = pUnitRec->PartSysIndicator;
            break;
         default :
            if (rgfFakePart[pUnitRec->PartSysIndicator])
               FakeMBR.PartitionTable[0].SysIndicator = PARTITION_IFS;
            else
               FakeMBR.PartitionTable[0].SysIndicator= pUnitRec->PartSysIndicator;
            break;
         }

      FakeMBR.PartitionTable[0].RelativeSectors= pUnitRec->NumExtraSectors-
                                                   SectorsPerCylinder;
      FakeMBR.PartitionTable[0].NumSectors= pUnitRec->NumSectors;
      ComputeCHS (pUnitRec->NumExtraSectors,GeoNumHeads,GeoTrackSec,
                  &(FakeMBR.PartitionTable[0].BegSecCyl),
                  &(FakeMBR.PartitionTable[0].BegHead));
      ComputeCHS (DiskSize-1,GeoNumHeads,GeoTrackSec,
                  &(FakeMBR.PartitionTable[0].EndSecCyl),
                  &(FakeMBR.PartitionTable[0].EndHead));
      }
 return CopyToSGList (&FakeMBR,sizeof(MBR),pSGList,cSGList,XferOffset);

#undef pMBR
}

/*
 * This pragma is not necessary for MS Visual C++ (M.Willm 1995-11-14)
 */
#ifndef __MSC__
#pragma argsused
#endif
USHORT WriteFakeSector (NPVirtUnitRec pUnitRec, ULONG SectorRBA,
                       PSCATGATENTRY pSGList, USHORT cSGList,
                       ULONG XferOffset)
{
 return IOERR_MEDIA_WRITE_PROTECT;	/* Tell 'em they can't write to disk */
}

/* Function to compute the Cylinder/Head/Sector value for a specified */
/* relative block address. Will set cylinder to 1023 if real cylinder>1023 */
VOID ComputeCHS (ULONG RBA, USHORT GeoNumHeads, USHORT GeoTrackSec,
                 USHORT FAR *CylSec, UCHAR FAR *Head)
{
 ULONG		SectorsPerCylinder= GeoNumHeads*GeoTrackSec;
 ULONG		CHSCylinder;
 USHORT		CHSHead;
 USHORT		CHSSector;

 /* Convert DiskSize into a CHS address... */
 CHSCylinder= RBA/SectorsPerCylinder;
 RBA= RBA%SectorsPerCylinder;
 CHSHead= RBA/GeoTrackSec;
 CHSSector= (RBA%GeoTrackSec)+1;
 if (CHSCylinder>1023) CHSCylinder= 1023;
 *Head= CHSHead;
 *CylSec= MAKE_SEC_CYL(CHSSector,CHSCylinder);
}

/* Copies Count bytes to the SG list pointed to by pSGList at the offset */
/* specified by XferOffset. */
USHORT CopyToSGList (void FAR *Buffer, ULONG Count, PSCATGATENTRY pSGList,
                     USHORT cSGList, ULONG XferOffset)
{
 ULONG	ppXferBuf;
 ULONG	XferBufLen;
 USHORT	XferLength;
 void	far* VirtAddr;

 while ((cSGList)&&(pSGList->XferBufLen<=XferOffset)) 
 {					/* Seek into the scatgat list */
  XferOffset-=(pSGList++)->XferBufLen;
  cSGList--;
 }
 XferBufLen= 0;
 while (Count)
 {
  if (!XferBufLen)			/* Need to use next SG entry */
  {
   if (!(cSGList--))			/* We run out of SG list elements */
    return IOERR_CMD_SGLIST_BAD;
   ppXferBuf= pSGList->ppXferBuf+ XferOffset;
   XferBufLen= pSGList->XferBufLen- XferOffset;
   pSGList++;
   XferOffset= 0;
  }

  XferLength= 32768;	/* To fit in USHORT */
  if (Count<XferLength) XferLength= Count;
  if (XferBufLen<XferLength) XferLength= XferBufLen;

  if (!(VirtAddr= PhysToVirt(ppXferBuf,XferLength)))
  {
   ENABLE
   return IOERR_CMD_SGLIST_BAD;
  }
  memcpy (VirtAddr,Buffer,XferLength);
  ENABLE
  ppXferBuf+= XferLength;
  ((char far *)Buffer)+= XferLength;
  Count-= XferLength;
  XferBufLen-= XferLength;
 }
 return 0;
}
