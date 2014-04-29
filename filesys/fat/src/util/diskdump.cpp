/* Test 64Kb I/O to disk */
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#define INCL_NOPM
#define INCL_VIO
#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

//#define BUFFERSIZE (65535*1)  /* works */
#define BUFFERSIZE (65536*2)  /* fails but is correct! */
//#define BUFFERSIZE (65537*1)  /* works */
//#define BUFFERSIZE (65024*1)  /* works */

#pragma pack(1)

typedef struct
   {
   BYTE   bCommand;
   USHORT usHead;
   USHORT usCylinder;
   USHORT usFirstSector;
   USHORT cSectors;
   struct
      {
      USHORT usSectorNumber;
      USHORT usSectorSize;
      }
   TrackTable[BUFFERSIZE/512];
} TRACK;

typedef struct
   {
   UCHAR BootInd;
   UCHAR startHead;
   UCHAR startSector;
   UCHAR startCyl;
   UCHAR SystemInd;
   UCHAR endHead;
   UCHAR endSector;
   UCHAR endCyl;
   ULONG Offset;
   ULONG Length;
   } PTENTRY;

typedef struct
   {
   PTENTRY ptEnt[4];
   } PTABLE;

typedef struct
   {
   BYTE bMBR[446];
   PTABLE pPTbl;
   USHORT uSig;
   } MBR;

int stopit = 0;
time_t t1, t2;

int main(int argcount, char *argval[])
   {
   APIRET rc, rrc;
   TRACK  trk;
   PCHAR  pBuffer;
   MBR    *mbr;
   USHORT DevHandle;
   char   drv[3] = "1:";
   ULONG  i, j;
   ULONG  ulCategory = IOCTL_PHYSICALDISK;
   ULONG  ulFunction = PDSK_READPHYSTRACK;
   ULONG  ulParmLen  = sizeof(TRACK);
   ULONG  ulDataLen  = BUFFERSIZE;
   ULONG  ulDataLength = BUFFERSIZE;
   ULONG  amount = BUFFERSIZE;
   char   msg[80];
   USHORT row, column;
   DEVICEPARAMETERBLOCK dpb;
   ULONG  cyls, heads, sectors, cursector, embroff, x;
   double dataread, diskcap, curpos;

   if (argcount > 1)
      {
      strcpy(drv, argval[1]);
      drv[1] = ':';
      drv[2] = 0;
      }

   if (!(amount%65536))
      {
      amount++;
      }

   ulDataLen = amount;
   ulDataLength = amount;

   pBuffer = (char*)malloc(amount);
   memset((void*)&trk, 0, sizeof(TRACK));

   trk.bCommand      = 0;
   trk.usHead        = 0;
   trk.usCylinder    = 0;
   trk.usFirstSector = 0;
   trk.cSectors      = 1;
   cursector         = 0;

   for (i = 0; i < 1; i++)
      {
      trk.TrackTable[i].usSectorNumber = i+1;
      trk.TrackTable[i].usSectorSize = 512;
      }

   rc = DosPhysicalDisk(INFO_GETIOCTLHANDLE,
               &DevHandle,
               sizeof(DevHandle),
               &drv,
               sizeof(drv));
   if (rc)
      {
      printf("DosPhysicalDisk rc = %d\n", rc);
      return -1;
      }
   rc = DosDevIOCtl(DevHandle,
                   ulCategory,
                   PDSK_GETPHYSDEVICEPARAMS,
                   &dpb,
                   sizeof(dpb),
                   (PULONG)&dpb,
                   &dpb,
                   ulDataLen,
                   &ulDataLength);
   if (rc)
      {
      printf("DosDevIOCtl rc = %d\n", rc);
      return -1;
      }
   else
      {
      cyls    = dpb.cCylinders;
      heads   = dpb.cHeads;
      sectors = dpb.cSectorsPerTrack;
      diskcap = (double)cyls * (double)heads * (double)sectors * 512.0;
      printf("Disk geometry: Cyls = %d, trks/cyl = %d, sec/trk = %d\n"
             "# BI  sCyl  sHd  sS id  eCyl  eHd  eS (adj. s c:h:s) (adj. end c:h:s)\n", cyls, heads, sectors);
      }

   rrc = VioGetCurPos(&row, &column, 0);
   if (rrc)
      printf("VioGetCurPos rc = %d\n", rrc);

   sprintf(msg, "% 8d % 5d % 6d", trk.usCylinder, trk.usHead, cursector);
   rrc = VioWrtCharStr( msg, strlen(msg), row, column, 0);
   if (rrc)
      printf("VioWrtCharStr rc = %d\n", rrc);

   rc = DosDevIOCtl(DevHandle,
                   ulCategory,
                   ulFunction,
                   &trk,
                   sizeof(TRACK),
                   &ulParmLen,
                   pBuffer,
                   ulDataLen,
                   &ulDataLength);
   if (rc == 0)
      {
      mbr = (MBR*)pBuffer;
      for (i = 0; i < 4; i++)
         {
         USHORT sCyl, eCyl;

         sCyl = ((mbr->pPTbl.ptEnt[i].startSector & 0xC0) << 2) + mbr->pPTbl.ptEnt[i].startCyl;
         eCyl = ((mbr->pPTbl.ptEnt[i].endSector & 0xC0) << 2) + mbr->pPTbl.ptEnt[i].endCyl;
         printf("%d %02X % 5d % 4d % 3d %02X % 5d % 4d % 3d (% 5d:% 3d:% 2d) (% 5d:% 3d:% 2d)\n",
               i,
               mbr->pPTbl.ptEnt[i].BootInd,
               sCyl,
               mbr->pPTbl.ptEnt[i].startHead,
               (mbr->pPTbl.ptEnt[i].startSector & 0x3f),
               mbr->pPTbl.ptEnt[i].SystemInd,
               eCyl,
               mbr->pPTbl.ptEnt[i].endHead,
               (mbr->pPTbl.ptEnt[i].endSector & 0x3f),
               (mbr->pPTbl.ptEnt[i].Offset/sectors)/heads,
               (mbr->pPTbl.ptEnt[i].Offset/sectors)%heads,
               (mbr->pPTbl.ptEnt[i].Offset%sectors)+1,
               ((mbr->pPTbl.ptEnt[i].Offset+mbr->pPTbl.ptEnt[i].Length-1)/sectors)/heads,
               ((mbr->pPTbl.ptEnt[i].Offset+mbr->pPTbl.ptEnt[i].Length-1)/sectors)%heads,
               ((mbr->pPTbl.ptEnt[i].Offset+mbr->pPTbl.ptEnt[i].Length-1)%sectors)+1);
         }
      printf("\n");
      for (i = 0; i < 4; i++)
         {
         if (mbr->pPTbl.ptEnt[i].SystemInd == 05)
            {
            trk.usHead        = (mbr->pPTbl.ptEnt[i].Offset/sectors)%heads;
            trk.usCylinder    = (mbr->pPTbl.ptEnt[i].Offset/sectors)/heads;
            trk.usFirstSector = 0;
            trk.cSectors      = 1;
            cursector         = mbr->pPTbl.ptEnt[i].Offset;
            embroff           = cursector;

            for (i = 0; i < 1; i++)
               {
               trk.TrackTable[i].usSectorNumber = (mbr->pPTbl.ptEnt[i].Offset%sectors)+1;
               trk.TrackTable[i].usSectorSize = 512;
               }
            rc = DosDevIOCtl(DevHandle,
                            ulCategory,
                            ulFunction,
                            &trk,
                            sizeof(TRACK),
                            &ulParmLen,
                            pBuffer,
                            ulDataLen,
                            &ulDataLength);
            if (rc == 0)
               {
               mbr = (MBR*)pBuffer;
               for (i = 0; i < 2; i++)
                  {
                  USHORT sCyl, eCyl;

                  sCyl = ((mbr->pPTbl.ptEnt[i].startSector & 0xC0) << 2) + mbr->pPTbl.ptEnt[i].startCyl;
                  eCyl = ((mbr->pPTbl.ptEnt[i].endSector & 0xC0) << 2) + mbr->pPTbl.ptEnt[i].endCyl;
                  x = (mbr->pPTbl.ptEnt[i].SystemInd == 05) ? embroff : cursector;
                  printf("%d %02X % 5d % 4d % 3d %02X % 5d % 4d % 3d (% 5d:% 3d:% 2d) (% 5d:% 3d:% 2d)\n",
                        i,
                        mbr->pPTbl.ptEnt[i].BootInd,
                        sCyl,
                        mbr->pPTbl.ptEnt[i].startHead,
                        (mbr->pPTbl.ptEnt[i].startSector & 0x3f),
                        mbr->pPTbl.ptEnt[i].SystemInd,
                        eCyl,
                        mbr->pPTbl.ptEnt[i].endHead,
                        (mbr->pPTbl.ptEnt[i].endSector & 0x3f),
                        ((x+mbr->pPTbl.ptEnt[i].Offset)/sectors)/heads,
                        ((x+mbr->pPTbl.ptEnt[i].Offset)/sectors)%heads,
                        ((x+mbr->pPTbl.ptEnt[i].Offset)%sectors)+1,
                        ((x+mbr->pPTbl.ptEnt[i].Offset+mbr->pPTbl.ptEnt[i].Length-1)/sectors)/heads,
                        ((x+mbr->pPTbl.ptEnt[i].Offset+mbr->pPTbl.ptEnt[i].Length-1)/sectors)%heads,
                        ((x+mbr->pPTbl.ptEnt[i].Offset+mbr->pPTbl.ptEnt[i].Length-1)%sectors)+1);
                  if (mbr->pPTbl.ptEnt[i].SystemInd == 05)
                     {
                     trk.usHead        = ((embroff+mbr->pPTbl.ptEnt[i].Offset)/sectors)%heads;
                     trk.usCylinder    = ((embroff+mbr->pPTbl.ptEnt[i].Offset)/sectors)/heads;
                     trk.usFirstSector = 0;
                     trk.cSectors      = 1;
                     cursector         = embroff + mbr->pPTbl.ptEnt[i].Offset;

                     for (i = 0; i < 1; i++)
                        {
                        trk.TrackTable[i].usSectorNumber = ((embroff+mbr->pPTbl.ptEnt[i].Offset)%sectors)+1;
                        trk.TrackTable[i].usSectorSize = 512;
                        }
                     rc = DosDevIOCtl(DevHandle,
                                     ulCategory,
                                     ulFunction,
                                     &trk,
                                     sizeof(TRACK),
                                     &ulParmLen,
                                     pBuffer,
                                     ulDataLen,
                                     &ulDataLength);
                     if (rc == 0)
                        {
                        mbr = (MBR*)pBuffer;
                        i = -1;
                        }
                     else
                        break;
                     }
                  }
               }
            break;
            }
         }
      }
   else
      {
      printf("\n%dKB I/O failed rc 0x%02x (cyl %d head %d sec %d)\n", ((BUFFERSIZE/512)/2), rc,
            trk.usCylinder, trk.usHead, cursector);
      return -1;
      }

   time(&t2);

   free (pBuffer);
   }

