
// щ Д ДДДДНН = Д  щ  Д = ННДДДД Д щ
// і                               і
//    ЬЫЫЫЫЫЫЫЬ   ЬЫЬ  ЬЫЫЫЫЫЫЫЫЬ          ъ  ъДДДНДДНДННДДННННДНННННННННОД
// і ЫЫЫЫЯЯЯЫЫЫЫ ЫЫЫЫЫ ЫЫЫЯ   ЯЫЫЫ і           AiR-BOOT - Installer/2    є
// є ЫЫЫЫЬЬЬЫЫЫЫ ЫЫЫЫЫ ЫЫЫЬ   ЬЫЫЫ є      ъ ДДДДНДННДДННННДННННННННДНННННОД
// є ЫЫЫЫЫЫЫЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫЫЫЫЫЫЯ  є       Section: AiR-BOOTUP Package   є
// є ЫЫЫЫ   ЫЫЫЫ ЫЫЫЫЫ ЫЫЫЫ ЯЫЫЫЫЬ є     і Created: 24/10/02             є
// і ЯЫЫЯ   ЯЫЫЯ  ЯЫЯ  ЯЫЫЯ   ЯЫЫЯ і     і Last Modified:                і
//                  ЬЬЬ                  і Number Of Modifications: 000  і
// щ              ЬЫЫЯ             щ     і INCs required: *none*         і
//      ДДДДДДД ЬЫЫЯ                     є Written By: Martin Kiewitz    і
// і     ЪїЪїіЬЫЫЫЬЬЫЫЫЬ           і     є (c) Copyright by              і
// є     АЩіАЩЯЫЫЫЯЯЬЫЫЯ           є     є      AiR ON-Line Software '02 ъ
// є    ДДДДДДД    ЬЫЫЭ            є     є All rights reserved.
// є              ЬЫЫЫДДДДДДДДД    є    ДОНННДНННННДННННДННДДНДДНДДДъДД  ъ
// є             ЬЫЫЫЭі іЪїііД     є
// і            ЬЫЫЫЫ АДііАЩіД     і
//             ЯЫЫЫЫЭДДДДДДДДДД     
// і             ЯЯ                і
// щ Дґ-=’iз йп-Liпо SйџвW’зо=-ГДД щ

#define INCL_NOPMAPI
#define INCL_DOS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include <malloc.h>

#include <global.h>

USHORT PHYS_EnumeratePhysicalDisks (void) {
   USHORT NumDrives = 0;

   if (DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS, &NumDrives, sizeof(NumDrives),NULL, 0) != 0)
      return 0;
   return NumDrives;
 }

USHORT PHYS_GetIOCTLHandle (PSZ DriveID) {
   USHORT IOCTLHandle = 0;

   if (DosPhysicalDisk(INFO_GETIOCTLHANDLE, &IOCTLHandle, sizeof(IOCTLHandle),DriveID, 3) != 0)
      return 0;
   return IOCTLHandle;
 }

VOID PHYS_FreeIOCTLHandle (USHORT IOCTLHandle) {
   DosPhysicalDisk(INFO_FREEIOCTLHANDLE, NULL, 0, &IOCTLHandle, sizeof(IOCTLHandle));
   return;
 }

BOOL PHYS_ReadPhysicalSector (USHORT IOCTLHandle, UCHAR Cylinder, UCHAR Head, UCHAR SectorCount, PBYTE BufferPtr) {
   ULONG       TrackLayoutLen  = sizeof(TRACKLAYOUT)+sizeof(ULONG)*(SectorCount-1);
   TRACKLAYOUT *TrackLayoutPtr = malloc(TrackLayoutLen);
   ULONG       cbParms = sizeof(TrackLayoutPtr);
   ULONG       cbData  = 512;
   INT         i;

   TrackLayoutPtr->bCommand      = 0x01;
   TrackLayoutPtr->usHead        = Head;
   TrackLayoutPtr->usCylinder    = Cylinder;
   TrackLayoutPtr->usFirstSector = 0;
   TrackLayoutPtr->cSectors      = SectorCount;

   for (i=0; i<SectorCount; i++) {
      TrackLayoutPtr->TrackTable[i].usSectorNumber = i+1;
      TrackLayoutPtr->TrackTable[i].usSectorSize   = 512;
    }

   i = 1;
   if (DosDevIOCtl(IOCTLHandle, IOCTL_PHYSICALDISK, PDSK_READPHYSTRACK,
       TrackLayoutPtr, cbParms, &cbParms, BufferPtr, cbData, &cbData))
      i = 0;
   free (TrackLayoutPtr);
   return i;
 }

BOOL PHYS_WritePhysicalSector (USHORT IOCTLHandle, UCHAR Cylinder, UCHAR Head, UCHAR SectorCount, PBYTE BufferPtr) {
   ULONG       TrackLayoutLen  = sizeof(TRACKLAYOUT)+sizeof(ULONG)*(SectorCount-1);
   TRACKLAYOUT *TrackLayoutPtr = malloc(TrackLayoutLen);
   ULONG       cbParms = sizeof(TrackLayoutPtr);
   ULONG       cbData  = 512;
   INT         i;

   TrackLayoutPtr->bCommand      = 0x01;
   TrackLayoutPtr->usHead        = Head;
   TrackLayoutPtr->usCylinder    = Cylinder;
   TrackLayoutPtr->usFirstSector = 0;
   TrackLayoutPtr->cSectors      = SectorCount;

   for (i=0; i<SectorCount; i++) {
      TrackLayoutPtr->TrackTable[i].usSectorNumber = i+1;
      TrackLayoutPtr->TrackTable[i].usSectorSize   = 512;
    }

   i = 1;
   if (DosDevIOCtl(IOCTLHandle, IOCTL_PHYSICALDISK, PDSK_WRITEPHYSTRACK,
       TrackLayoutPtr, cbParms, &cbParms, BufferPtr, cbData, &cbData))
      i = 0;
   free (TrackLayoutPtr);
   return i;
 }

USHORT PHYS_GetGeometrySectorsPerTrack (USHORT IOCTLHandle) {
   DEVICEPARAMETERBLOCK DeviceParmBlock;
   ULONG                ulDataLength;
   
   if (!DosDevIOCtl(IOCTLHandle, IOCTL_PHYSICALDISK, PDSK_GETPHYSDEVICEPARAMS,
       NULL, 0, NULL, &DeviceParmBlock, sizeof(DeviceParmBlock), &ulDataLength))
      return DeviceParmBlock.cSectorsPerTrack;
   return 0;
 }

USHORT PHYS_GetGeometryHeadsPerCylinder (USHORT IOCTLHandle) {
   DEVICEPARAMETERBLOCK DeviceParmBlock;
   ULONG                ulDataLength;
   
   if (!DosDevIOCtl(IOCTLHandle, IOCTL_PHYSICALDISK, PDSK_GETPHYSDEVICEPARAMS,
       NULL, 0, NULL, &DeviceParmBlock, sizeof(DeviceParmBlock), &ulDataLength))
      return DeviceParmBlock.cHeads;
   return 0;
 }
