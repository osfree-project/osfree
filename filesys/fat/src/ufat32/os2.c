/*   OS/2 specific functions
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <string.h>

#ifdef __WATCOMC__
#include <ctype.h>
#endif

#include "fat32c.h"
#include "fat32def.h"
#include "portable.h"

#define MAX_MESSAGE 2048

#define BLOCK_SIZE  0x4000
#define MAX_MESSAGE 2048
#define TYPE_LONG    0
#define TYPE_STRING  1
#define TYPE_PERC    2
#define TYPE_LONG2   3
#define TYPE_DOUBLE  4
#define TYPE_DOUBLE2 5

typedef HFILE HANDLE;

extern HANDLE hDev;

INT cdecl iShowMessage(PCDINFO pCD, USHORT usNr, USHORT usNumFields, ...);
PSZ       GetOS2Error(USHORT rc);

DWORD get_vol_id (void)
{
    DATETIME s;
    DWORD d;
    WORD lo,hi,tmp;

    DosGetDateTime( &s );

    lo = s.day + ( s.month << 8 );
    tmp = s.hundredths + (s.seconds << 8 );
    lo += tmp;

    hi = s.minutes + ( s.hours << 8 );
    hi += s.year;
   
    d = lo + (hi << 16);
    return(d);
}

void die ( char * error, DWORD rc )
{
    char pszMsg[MAX_MESSAGE];
    APIRET ret;
   
    // Format failed
    printf("%s\n", error);
    iShowMessage(NULL, 528, 0);
    printf("%s\n", GetOS2Error(rc));

    if ( rc )
        printf("Error code: %lu\n", rc);

    quit ( rc );
}

void seek_to_sect( HANDLE hDevice, DWORD Sector, DWORD BytesPerSect )
{
    LONGLONG llOffset, llActual;
    APIRET rc;
    
    llOffset = Sector * BytesPerSect;
    rc = DosSetFilePtrL( hDevice, (LONGLONG)llOffset, FILE_BEGIN, &llActual );
}

void write_sect ( HANDLE hDevice, DWORD Sector, DWORD BytesPerSector, void *Data, DWORD NumSects )
{
    DWORD dwWritten;
    BOOL ret;

    seek_to_sect ( hDevice, Sector, BytesPerSector );
    ret = DosWrite ( hDevice, Data, NumSects * BytesPerSector, (PULONG)&dwWritten );

    if ( ret )
        die ( "Failed to write", ret );
}

BOOL write_file ( HANDLE hDevice, BYTE *pData, DWORD ulNumBytes, DWORD *dwWritten )
{
    BOOL ret = TRUE;

    if (DosWrite ( hDevice, pData, (ULONG)ulNumBytes, (PULONG)dwWritten ))
        ret = FALSE;

    return ret;
}

void open_drive (char *path, HANDLE *hDevice)
{
  APIRET rc;
  ULONG  ulAction;
  char DriveDevicePath[]="\\\\.\\Z:"; // for DosOpenL
  char *p = path;

  if (path[1] == ':' && path[2] == '\0')
  {
    // drive letter (UNC)
    DriveDevicePath[4] = *path;
    p = DriveDevicePath;
  }

  rc = DosOpenL( p,              // filename
	      hDevice,           // handle returned
              &ulAction,         // action taken by DosOpenL
              0,                 // cbFile
              0,                 // ulAttribute
              OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_WRITE_THROUGH | 
              OPEN_FLAGS_NO_CACHE | OPEN_SHARE_DENYREADWRITE |
              OPEN_ACCESS_READWRITE, // | OPEN_FLAGS_DASD,
              NULL);             // peaop2

     //      OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE |
     //      OPEN_FLAGS_NO_CACHE  | OPEN_FLAGS_WRITE_THROUGH, // | OPEN_FLAGS_DASD,

     // OPEN_ACTION_OPEN_IF_EXISTS,         /* open flags   */
     // OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_DASD |
     // OPEN_FLAGS_WRITE_THROUGH, // | OPEN_FLAGS_NO_CACHE,

  if ( rc != 0 || hDevice == 0 )
      die( "Failed to open device - close any files before formatting,"
           "and make sure you have Admin rights when using fat32format"
            "Are you SURE you're formatting the RIGHT DRIVE!!!", rc );

  hDev = *hDevice;
}

void lock_drive(HANDLE hDevice)
{
  unsigned char  cmdinfo = '\0';
  unsigned long  parmlen = sizeof(cmdinfo);
  unsigned long  datalen = sizeof(cmdinfo);
  APIRET rc;

  rc = DosDevIOCtl( hDevice, IOCTL_DISK, DSK_LOCKDRIVE, &cmdinfo,
                    sizeof(cmdinfo), &parmlen, &cmdinfo, // Param packet
                    sizeof(cmdinfo), &datalen);

  if ( rc )
      die( "Failed to lock device" , rc);

}

void unlock_drive(HANDLE hDevice)
{
  unsigned char  cmdinfo = '\0';
  unsigned long  parmlen = sizeof(cmdinfo);
  unsigned long  datalen = sizeof(cmdinfo);
  APIRET rc;

  rc = DosDevIOCtl( hDevice, IOCTL_DISK, DSK_UNLOCKDRIVE, &cmdinfo,
                    sizeof(cmdinfo), &parmlen, &cmdinfo, // Param packet
                    sizeof(cmdinfo), &datalen);

  if ( rc )
      die( "Failed to unlock device" , rc );

}

#pragma pack(1)
struct parm {
    unsigned char command;
    unsigned char drive;
};

struct data {
    BIOSPARAMETERBLOCK bpb;
};
#pragma pack()

void get_drive_params(HANDLE hDevice, struct extbpb *dp)
{
  APIRET rc;
  struct parm p;
  struct data d;
  ULONG  parmio;
  ULONG  dataio;

  memset(&d, 0, sizeof(d));
  parmio = sizeof(p);
  dataio = sizeof(d);
  p.command = 1;
  p.drive   = 0;

  rc = DosDevIOCtl( hDevice, IOCTL_DISK, DSK_GETDEVICEPARAMS,
                      &p, parmio, &parmio,
                      &d, dataio, &dataio);

  if ( rc )
      die( "Failed to get device geometry", rc );

  dp->BytesPerSect = d.bpb.usBytesPerSector;
  dp->SectorsPerTrack = d.bpb.usSectorsPerTrack;
  dp->HiddenSectors = d.bpb.cHiddenSectors;
  dp->TracksPerCylinder = d.bpb.cHeads;

  //printf("cylinders=%u\n",   d.bpb.cCylinders);
  //printf("device type=%u\n", d.bpb.bDeviceType);
  //printf("device attr=%u\n", d.bpb.fsDeviceAttr);

  //if ((d.bpb.cSectors == 0) && (d.bpb.cLargeSectors))
      dp->TotalSectors = d.bpb.cLargeSectors;
  //else if ((d.bpb.cSectors) && (d.bpb.cLargeSectors == 0))
  //    dp->TotalSectors = d.bpb.cSectors;

  dp->PartitionLength = (ULONGLONG)dp->TotalSectors;
  dp->PartitionLength *= dp->BytesPerSect;

  //printf("TotalSectors=%lu, BytesPerSect=%u\n", 
  //       dp->TotalSectors, dp->BytesPerSect);

  //printf("BytesPerSect=%u\n", d.bpb.usBytesPerSector);
  //printf("SectorsPerTrack=%u\n", d.bpb.usSectorsPerTrack);
  //printf("HiddenSectors=%lu\n", d.bpb.cHiddenSectors);
  //printf("TracksPerCylinder=%u\n", d.bpb.cHeads);
  //printf("TotalSectors=%lu\n", dp->TotalSectors);
  //printf("PartitionLength=%llu\n", dp->PartitionLength);
}

void set_part_type(UCHAR Dev, HANDLE hDevice, struct extbpb *dp)
{
  //if ( rc && dp->HiddenSectors )
  //    die( "Failed to set parition info", rc );

  //rc = DosDevIOCtl( hDevice, IOCTL_DISK, DSK_SETDEVICEPARAMS, 
  //                  &p, parmio, &parmio,
  //                 &d, dataio, &dataio);
  //if ( rc )
  //      {
        // This happens because the drive is a Super Floppy
        // i.e. with no partition table. Disk.sys creates a PARTITION_INFORMATION
        // record spanning the whole disk and then fails requests to set the 
        // partition info since it's not actually stored on disk. 
	// So only complain if there really is a partition table to set      


        //if ( d.bpb.cHiddenSectors  )
	//		die( "Failed to set parition info", -6 );
        //}    
}


void begin_format (HANDLE hDevice)
{
  // Detach the volume from the old FSD 
  // and attach to the new one
  unsigned char  cmdinfo   = 0;
  unsigned long  parmlen   = sizeof(cmdinfo);
  unsigned char  fsdname[] = "FAT32"; 
  unsigned long  datalen   = sizeof(fsdname);
  APIRET rc;

  rc = DosDevIOCtl( hDevice, IOCTL_DISK, DSK_BEGINFORMAT, &cmdinfo, 
                    sizeof(cmdinfo), &parmlen, fsdname,
                    sizeof(fsdname), &datalen);

  if ( rc )
      die( "Failed to begin format device", rc );
}

void remount_media (HANDLE hDevice)
{
  // Redetermine media
  unsigned char cmdinfo  = 0;
  unsigned long parmlen  = sizeof(cmdinfo);
  unsigned char reserved = 0;
  unsigned long datalen  = sizeof(reserved);
  APIRET rc;

  rc = DosDevIOCtl( hDevice, IOCTL_DISK, DSK_REDETERMINEMEDIA,
                    &cmdinfo, sizeof(cmdinfo), &parmlen, // Param packet
                    &reserved, sizeof(reserved), &datalen);

  if ( rc )
      die( "Failed to do final remount!", rc );
}

void close_drive(HANDLE hDevice)
{
    // Close Device
    DosClose ( hDevice );
}

void mem_alloc(void **p, ULONG cb)
{
    if (!(DosAllocMem ( (void **)p, cb, PAG_COMMIT | PAG_READ | PAG_WRITE )))
            memset(*p, 0, cb);
}

void mem_free(void *p, ULONG cb)
{
    if (p)
       DosFreeMem(p);
}

void query_freq(ULONGLONG *freq)
{
    DosTmrQueryFreq( (ULONG *)freq );
}

void query_time(ULONGLONG *time)
{
    DosTmrQueryTime( (QWORD *)time );
}

void check_vol_label(char *path, char **vol_label)
{
    /* Current volume label  */
    char cur_vol[12];
    char testvol[12];
    /* file system information buffer */
    FSINFO fsiBuffer;
    char   c;
    ULONG  rc;

    memset(cur_vol, 0, sizeof(cur_vol));

    // Query the filesystem info, 
    // including the current volume label
    rc = DosQueryFSInfo((toupper(path[0]) - 'A' + 1), FSIL_VOLSER,
                       (PVOID)&fsiBuffer, sizeof(fsiBuffer));
        
    if (rc == NO_ERROR)
        // Current disk volume label
        strcpy(cur_vol, fsiBuffer.vol.szVolLabel);

    // The current file system type is FAT32
    iShowMessage(NULL, 1293, 1, TYPE_STRING, "FAT32");

    if (!cur_vol || !*cur_vol)
        // The disk has no a volume label
        iShowMessage(NULL, 125, 0);
    else
    {
        if (!vol_label || !*vol_label || !**vol_label)
        {
            // Enter the current volume label
            iShowMessage(NULL, 1318, 1, TYPE_STRING, path);

            // Read the volume label
            gets(testvol);

            if (stricmp(testvol, cur_vol))
            {
                // Incorrect volume  label for
                // disk %c is entered!
                iShowMessage(NULL, 636, 0);
                quit (1);
            }
        }
    }

    // Warning! All data on the specified hard disk
    // will be destroyed! Proceed with format (Yes(1)/No(0))?
    iShowMessage(NULL, 1271, 1, TYPE_STRING, path);

    c = getchar();

    if ( c != '1' && toupper(c) != 'Y' )
        quit (1);
 
    fflush(stdout);
}

char *get_vol_label(char *path, char *vol)
{
    static char default_vol[12] = "NO NAME    ";
    static char v[12];
    char *label = vol;

    if (!vol || !*vol)
    {
        fflush(stdin);
        // Enter up to 11 characters for the volume label
        iShowMessage(NULL, 1288, 0);

        label = &v;
    }

    memset(label, 0, 12);
    gets(label);

    if (!*label)
        label = &default_vol;

    if (strlen(label) > 11)
    {
       // volume label entered exceeds 11 chars
       iShowMessage(NULL, 154, 0);
       // truncate it
       label[11] = '\0';
    }

    return label;
}

void set_vol_label (char *path, char *vol)
{
  VOLUMELABEL vl = {0};
  APIRET rc;
  int diskno;
  int l;

  //printf("0000\n");

  if (!vol || !*vol)
    return;

  //printf("0001\n");

  l = strlen(vol);

  //printf("0002\n");

  if (!path || !*path)
    return;

  //printf("0003\n");

  //printf("path=%s, vol=%s\n", path, vol);

  diskno = toupper(*path) - 'A' + 1;

  memset(&vl, 0, sizeof(VOLUMELABEL));
  //printf("0004\n");
  strcpy(vl.szVolLabel, vol);
  //strcpy(fsi.vol.szVolLabel, vol);
  //printf("0005\n");
  //vl.szVolLabel[l] = '\0';
  //printf("0006\n");
  vl.cch = strlen(vl.szVolLabel);

  //printf("0007\n");
  //DosSleep(4);
  
  rc = DosSetFSInfo(diskno, FSIL_VOLSER,
                    (PVOID)&vl, sizeof(VOLUMELABEL));

  //printf("0008\n");
  if ( rc )
    printf ("Warning: failed to set the volume label, rc=%lu\n", rc);
}

void show_progress (float fPercentWritten)
{
  char str[128];
  static int pos = 0;
  //USHORT row, col;
  //char   chr = ' ';

  if (! pos)
  {
    pos = 1;
    // save cursor position
    printf("[s");
  }

  //VioGetCurPos(&row, &col, 0);
  //VioWrtNChar(&chr, 8, row, col, 0);
  //VioWrtCharStr(str, strlen(str), row, col, 0);

  // construct message
  sprintf(str, "%.2f%%", fPercentWritten);
  iShowMessage(NULL, 1312, 2, TYPE_STRING, str, 
                              TYPE_STRING, "...");
  DosSleep(100);
  // restore cursor position
  printf("[u");
  fflush(stdout); 
}
