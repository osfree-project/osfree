#define INCL_DOSERRORS
#define INCL_BSEDEV
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#include <os2.h>
#include <udsk.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void read_chs(char * buffer, USHORT hDevice, int cyl, int head, int sector)
{

 TRACKLAYOUT trackLayout;
 ULONG cbParams;   /*  The length, in bytes, of the parameter buffer. */
 ULONG cbData;
 APIRET rc;

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

APIRET APIENTRY DskSecRead(const PSZ pszDisk, const LONGLONG ullLBA, const ULONG bufSize, VOID * buffer)
{
  DEVICEPARAMETERBLOCK devparamblock;
  ULONG   ulParmLen;
  ULONG   ulDataLen;
  UCHAR   bCommand;
  USHORT  hDevice;
  ULONG   rc;
  BYTE buf[10];

  int s;
  int h;
  int c;
  int S;
  int H;
  long long ll;

  /* Now get a handle to the first physical disk. */
  rc = DosPhysicalDisk(
          INFO_GETIOCTLHANDLE,  /* function */
          &hDevice,
          sizeof(hDevice),
          &buf,
          strlen(buf)+1
          );

  bCommand=0;
  ulParmLen=sizeof(bCommand);
  ulDataLen=sizeof(devparamblock);

  rc = DosDevIOCtl(
          hDevice,        /* auto - casted to 4 bytes ? */
          0x09,           /* category -- Physical Disk IOCtl */
          0x63,           /* function -- get geometry */
          &bCommand,      /* pParams */
          ulParmLen,       /* cbParmLenMax */
          &ulParmLen,      /* pcbParmLen */
          &devparamblock,
          ulDataLen,
          &ulDataLen
          );

  H=devparamblock.cHeads;
  S=devparamblock.cSectorsPerTrack;

  ll= ((long long)ullLBA.ulHi << 32) | (ullLBA.ulLo);

  s=(ll % S) + 1;
  h=((ll+1-s)%(H*S))/S;
  c=(ll+1-s)/(H*S);

  read_chs(buffer, hDevice, c, h, s);
}

APIRET APIENTRY DskSecWrite(const PSZ pszDisk, const LONGLONG ullLBA, const ULONG bufSize, const PVOID buffer)
{
}

APIRET APIENTRY DskBootSecRead(const PSZ pszDisk, VOID * buffer)
{
  LONGLONG ll;

  ll.ulLo=1;
  ll.ulHi=0;

  return DskSecRead(pszDisk, ll, 1, buffer);
}

APIRET APIENTRY DskBootSecWrite(const PSZ pszDisk, const PVOID buffer)
{
  LONGLONG ll;

  ll.ulLo=1;
  ll.ulHi=0;

  return DskSecWrite(pszDisk, ll, 1, buffer);
}

APIRET APIENTRY DskMbrRead(const PSZ pszDisk, VOID * buffer)
{
  LONGLONG ll;

  ll.ulLo=1;
  ll.ulHi=0;

  return DskSecRead(pszDisk, ll, 1, buffer);
}

APIRET APIENTRY DskMbrWrite(const PSZ pszDisk, const PVOID buffer)
{
  LONGLONG ll;

  ll.ulLo=1;
  ll.ulHi=0;

  return DskSecWrite(pszDisk, ll, 1, buffer);
}
