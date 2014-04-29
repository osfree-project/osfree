#include "fat32def.h"

typedef struct _DriveInfo
{
BYTE   bPartType;
USHORT DiskNum;
USHORT nHeads;
USHORT nCylinders;
HFILE  hDisk;
USHORT nSectorsPerTrack;
USHORT StartHead;
USHORT StartCylinder;
USHORT StartSector;
USHORT EndHead;
USHORT EndCylinder;
USHORT EndSector;
ULONG  ulRelStartSector;
ULONG  ulRelEndSector;
BPB    bpb;
ULONG  cTrackSize;
PTRACKLAYOUT pTrack;
ULONG  ulStartOfFAT;
PBYTE  pbFATSector[512];
ULONG  ulCurFATSector;
PBYTE  pbCluster;
ULONG  ulCurCluster;
USHORT usClusterSize;
ULONG  ulStartOfData;
ULONG  ulTotalClusters;
PBYTE  pFatBits;
} DRIVEINFO, *PDRIVEINFO;


IMPORT DRIVEINFO rgDrives[];
IMPORT USHORT    usDriveCount;
IMPORT BOOL      fDetailed;
IMPORT BOOL      fShowBootSector;
IMPORT BYTE rgfFakePart[];
IMPORT BYTE rgfFakeComp[];


IMPORT APIRET InitProg(void);
IMPORT BOOL   DumpDirectory(PDRIVEINFO pDrive, ULONG ulCluster, PSZ pszDrive);
IMPORT APIRET OpenDisk(WORD wDiskNum, PHFILE phFile);
IMPORT APIRET CloseDisk(HFILE hFile);
IMPORT VOID   MarkCluster(PDRIVEINFO pDrive, ULONG ulCluster);
IMPORT ULONG GetNextCluster(PDRIVEINFO pDrive, ULONG ulCluster);
