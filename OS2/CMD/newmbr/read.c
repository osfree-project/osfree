#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmd_shared.h>

#include "mbr.h"
#include "mbrdefs.h"

char sectorBuff[512];

void read_chs(char * buffer,USHORT hDevice, int cyl, int head, int sector);
void write_chs(char * buffer,USHORT hDevice, int cyl, int head, int sector);

int main (int argc, char* argv[], char* envp[])
{

  PVOID pBuf;      /*  The address of the buffer where the returned
                      information is placed. */
  ULONG cbBuf;     /*  The length, in bytes, of the data buffer. */
  PVOID pParams;   /*  The address of the buffer used for input parameters. */
  ULONG ulDataLen = sizeof(USHORT);     /* Data return buffer length */
  USHORT hDevice;
  unsigned short int bufPtr;
  int i;
  FILE *fp;

  APIRET rc = NO_ERROR;

  int rez=0;
  int index;

  while ( (rez = getopt(argc,argv,"i:I:p:P:d:D:hH")) != -1)
  {
    switch (rez)
    {
      case 'i':;
      case 'I': ulTargetDrive=optarg; break;
      case 'p':;
      case 'P': printf("found argument \"p = %s\".\n",optarg); break;
      case 'd':;
      case 'D': printf("found argument \"d = %s\".\n",optarg); break;
      case 'h':;
      case 'H':;
      case '?': printf("Error found !\n"); /* usage(); */ break;
    };
  };

  if (argc>optind) printf ("Non-option argument %s\n", argv[optind]);

  /* Now get a handle to the first physical disk. */
  rc = DosPhysicalDisk(
          INFO_GETIOCTLHANDLE,  /* function */
          &hDevice,
          sizeof(hDevice),
          "1:\0\0",
          4
          );

  if (rc != NO_ERROR)
  {
    printf("DosPhysicalDisk (INFO_GETIOCTLHANDLE) error: return code = %u\n", rc);
    exit(-1);
  }
 else {
//  printf("DosPhysicalDisk:  Disk 1 handle: %ld\n",hDevice);
  }

 /* Now read the Master Boot Record (MBR) from Disk 1 */
 read_chs(sectorBuff,hDevice,0,0,1);

 /* Copy partition table to the new mbr */
 memcpy((void *)&mbr[PartTable], (void *)&sectorBuff[PartTable], 16*4);

 /* Write new MBR */
 write_chs(mbr,hDevice,0,0,1);

 /* Now close the handle. */
 rc = DosPhysicalDisk(
          INFO_FREEIOCTLHANDLE,  /* function */
          NULL,
          0,
          &hDevice,
          sizeof(hDevice)
          );

 if (rc != NO_ERROR) {
  printf("DosPhysicalDisk (INFO_FREEIOCTLHANDLE) error: return code = %u\n", rc);
  exit(-1);
  }
 else {
//  printf("DosPhysicalDisk:  Closed Disk 1 handle.\n");
  }

}


void read_chs(char * buffer,USHORT hDevice, int cyl, int head, int sector)
{

 TRACKLAYOUT trackLayout;
 ULONG cbParams;   /*  The length, in bytes, of the parameter buffer. */
 ULONG cbData;
 APIRET rc;

 /* printf("size of track layout is %d\n",sizeof(TRACKLAYOUT)); */
 /* size = 13 decimal */

 if (1 == sector)
  trackLayout.bCommand = 0x01;   /* means track layout starts w/sector 1
                                    & has consec sectors */
 else
  trackLayout.bCommand = 0;
 trackLayout.usHead = head;
 trackLayout.usCylinder = cyl;
 trackLayout.usFirstSector = 0;
 trackLayout.cSectors = 1;

 trackLayout.TrackTable[0].usSectorNumber = sector;
 trackLayout.TrackTable[0].usSectorSize = 512;

 cbData = 512;

 cbParams = sizeof(trackLayout);   /* equals 13 decimal */

 rc = DosDevIOCtl(
          hDevice, /* auto - casted to 4 bytes ? */
          0x09, /* category -- Physical Disk IOCtl */
          0x64, /* function -- read track */
          &trackLayout,  /* pParams */
          cbParams,  /* cbParmLenMax */
          &cbParams, /* pcbParmLen */
          buffer,
          cbData,
          &cbData
          );

 if (rc != NO_ERROR) {
  printf("DosDevIOCtl error: return code = %u\n", rc);
  exit(-1);
  }
 else {
  /* printf("DosDevIOCtl: read 1 sector.\n"); */
  }

}

void write_chs(char * buffer,USHORT hDevice, int cyl, int head, int sector)
{

 TRACKLAYOUT trackLayout;
 ULONG cbParams;   /*  The length, in bytes, of the parameter buffer. */
 ULONG cbData;
 APIRET rc;

 /* printf("size of track layout is %d\n",sizeof(TRACKLAYOUT)); */
 /* size = 13 decimal */

 if (1 == sector)
  trackLayout.bCommand = 0x01;   /* means track layout starts w/sector 1
                                    & has consec sectors */
 else
  trackLayout.bCommand = 0;
 trackLayout.usHead = head;
 trackLayout.usCylinder = cyl;
 trackLayout.usFirstSector = 0;
 trackLayout.cSectors = 1;

 trackLayout.TrackTable[0].usSectorNumber = sector;
 trackLayout.TrackTable[0].usSectorSize = 512;

 cbData = 512;

 cbParams = sizeof(trackLayout);   /* equals 13 decimal */

 rc = DosDevIOCtl(
          hDevice, /* auto - casted to 4 bytes ? */
          0x09, /* category -- Physical Disk IOCtl */
          0x44, /* function -- write track */
          &trackLayout,  /* pParams */
          cbParams,  /* cbParmLenMax */
          &cbParams, /* pcbParmLen */
          buffer,
          cbData,
          &cbData
          );

 if (rc != NO_ERROR) {
  printf("DosDevIOCtl error: return code = %u\n", rc);
  exit(-1);
  }
 else {
  /* printf("DosDevIOCtl: write 1 sector.\n"); */
  }

}
