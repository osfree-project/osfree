#ifndef FAT32DEF_H
#define FAT32DEF_H

typedef struct _BPB
{
USHORT BytesPerSector;
BYTE   SectorsPerCluster;
USHORT ReservedSectors;
BYTE   NumberOfFATs;
USHORT RootDirEntries;
USHORT TotalSectors;
BYTE   MediaDescriptor;
USHORT SectorsPerFat;
USHORT SectorsPerTrack;
USHORT Heads;
ULONG  HiddenSectors;
ULONG  BigTotalSectors;
ULONG  BigSectorsPerFat;
USHORT ExtFlags;
USHORT FS_Version;
ULONG  RootDirStrtClus;
USHORT FSinfoSec;
USHORT BkUpBootSec;
BYTE   bReserved[12];
} BPB, *PBPB;

#pragma pack(1)

typedef struct _Fat32Parms
{
ULONG  ulDiskIdle;
ULONG  ulBufferIdle;
ULONG  ulMaxAge;
USHORT fMessageActive;
USHORT fUseShortNames;
USHORT fEAS;
BYTE   szVersion[10];

USHORT fLW;
USHORT fInShutDown;
ULONG  ulTotalReads;
ULONG  ulTotalHits;
ULONG  ulTotalRA;
USHORT usCacheUsed;
USHORT usDirtySectors;
USHORT usPendingFlush;
USHORT usDirtyTreshold;
USHORT volatile usCacheSize;
USHORT usSegmentsAllocated;
USHORT fTranslateNames;
ULONG  ulCurCP;
} F32PARMS, *PF32PARMS;

typedef struct _Options
{
volatile USHORT fTerminate;
ULONG  ulLWTID;
ULONG  ulEMTID;
BYTE   bLWPrio;
} LWOPTS, * PLWOPTS;

typedef struct _BootSector
{
BYTE bJmp[3];
BYTE oemID[8];
BPB  bpb;
BYTE bUnknown[3];
ULONG ulVolSerial;
BYTE VolumeLabel[11];
BYTE FileSystem[8];
} BOOTSECT, *PBOOTSECT;

typedef struct _BOOTFSINFO
{
ULONG ulInfSig;
ULONG ulFreeClusters;
ULONG ulNextFreeCluster;
ULONG ulReserved[3];
} BOOTFSINFO, *PBOOTFSINFO;

typedef struct _DiskOffset
{
BYTE   Head ;
int    Sectors:6;
int    Cylinders256:2;
BYTE   Cylinders;
} DISKOFFSET;

typedef struct _Partition
{
BYTE        iBoot;
DISKOFFSET  Start;
BYTE        cSystem;
DISKOFFSET  End;
ULONG       ulRelSectorNr;
ULONG       ulTotalSectors;
} PARTITIONENTRY, *PPARTITIONENTRY;

typedef struct _MasterBootrecord
{
PARTITIONENTRY partition[4];
} MBR, *PMBR;

typedef struct _DirEntry
{
BYTE   bFileName[8];
BYTE   bExtention[3];
BYTE   bAttr;
BYTE   fEAS;
BYTE   bReserved;
FTIME  wCreateTime;
FDATE  wCreateDate;
FDATE  wAccessDate;
USHORT wClusterHigh;

FTIME wLastWriteTime;
FDATE wLastWriteDate;
USHORT wCluster;
ULONG  ulFileSize;
} DIRENTRY, * PDIRENTRY;

typedef struct _LongNameEntry
{
BYTE   bNumber;
USHORT usChar1[5];
BYTE   bAttr;
BYTE   bReserved;
BYTE   bVFATCheckSum;
USHORT usChar2[6];
USHORT wCluster;
USHORT usChar3[2];
} LNENTRY, * PLNENTRY;

#pragma pack()


#define PARTITION_ENTRY_UNUSED          0x00      
#define PARTITION_FAT_12                0x01      
#define PARTITION_XENIX_1               0x02      
#define PARTITION_XENIX_2               0x03      
#define PARTITION_FAT_16                0x04      
#define PARTITION_EXTENDED              0x05      
#define PARTITION_HUGE                  0x06      
#define PARTITION_IFS                   0x07      
#define PARTITION_BOOTMANAGER           0x0A
#define PARTITION_FAT32                 0x0B      
#define PARTITION_FAT32_XINT13          0x0C      
#define PARTITION_XINT13                0x0E      
#define PARTITION_XINT13_EXTENDED       0x0F      
#define PARTITION_PREP                  0x41      
#define PARTITION_UNIX                  0x63      
#define VALID_NTFT                      0xC0      
#define PARTITION_LINUX		             0x83
#define PARTITION_HIDDEN                0x10

#ifndef FP_SEG
#define FP_SEG(fp) (*((unsigned _far *)&(fp)+1))
#define FP_OFF(fp) (*((unsigned _far *)&(fp)))
#endif

#define FAT32_VERSION "0.91á"

#define FSINFO_OFFSET  484
#define MBRTABLEOFFSET 446
#define FILE_VOLID     0x0008
#define FILE_LONGNAME  0x000F
#define FILE_HIDE      0x8000
#define FAT_EOF        0x0FFFFFFF
#define FAT_EOF2       0x0FFFFFF8
#define FAT_BAD_CLUSTER 0x0FFFFFF7
#define FAT_ASSIGN_NEW 0xFFFFFFFF
#define FAT_NOTUSED    0x00000000
#define SECTOR_SIZE    512
#define MAX_DRIVES     10
#define DELETED_ENTRY  0xE5
#define ERROR_VOLUME_NOT_MOUNTED 0xEE00
#define ERROR_VOLUME_DIRTY 627
#define FAT32MAXPATHCOMP 250
#define FAT32MAXPATH 260
#define OPEN_ACCESS_EXECUTE   0x0003
#define MAX_EA_SIZE ((ULONG)65536)
#define EA_EXTENTION " EA. SF"
#define FILE_HAS_NO_EAS          0x00
#define FILE_HAS_EAS             0xEA
#define FILE_HAS_CRITICAL_EAS    0xEC



#define FAT32_CLEAN_SHUTDOWN  0x08000000
#define FAT32_NO_DISK_ERROR   0x04000000

/* FSCTL function numbers */
#define FAT32_GETLOGDATA      0x8000
#define FAT32_SETMESSAGE      0x8001
#define FAT32_STARTLW         0x8002
#define FAT32_STOPLW          0x8003
#define FAT32_DOLW            0x8004
#define FAT32_SETPARMS        0x8005
#define FAT32_GETPARMS        0x8006
#define FAT32_SETTRANSTABLE   0x8007
#define FAT32_WIN2OS          0x8008
#define FAT32_EMERGTHREAD     0x8009

#define FAT32_SECTORIO        0x9014

/* IOCTL function numbers */
#define IOCTL_FAT32        IOCTL_GENERAL
#define FAT32_SETRASECTORS    0xF0
#define FAT32_GETVOLCLEAN     0xF1
#define FAT32_MARKVOLCLEAN    0xF2
#define FAT32_FORCEVOLCLEAN   0xF3
#define FAT32_RECOVERCHAIN    0xF4
#define FAT32_DELETECHAIN     0xF5
#define FAT32_QUERYRASECTORS  0xF6
#define FAT32_GETFREESPACE    0xF7
#define FAT32_SETFILESIZE     0xF8
#define FAT32_QUERYEAS        0xF9
#define FAT32_SETEAS          0xFA
#define FAT32_SETCLUSTER      0xFB

typedef struct _FileSizeStruct
{
BYTE   szFileName[FAT32MAXPATH];
ULONG  ulFileSize;
} FILESIZEDATA, *PFILESIZEDATA;

typedef struct _MarkFileEASBuf
{
BYTE   szFileName[FAT32MAXPATH];
BYTE   fEAS;
} MARKFILEEASBUF, *PMARKFILEEASBUF;

typedef struct _SetClusterData
{
ULONG ulCluster;
ULONG ulNextCluster;
} SETCLUSTERDATA, *PSETCLUSTERDATA;

#endif
