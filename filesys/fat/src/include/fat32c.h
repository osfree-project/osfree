/*   FAT32 defs
 *
 */

#ifdef __WATCOMC__
#include <ctype.h>
#endif

#if defined(__OS2__) && !defined(OS2_INCLUDED)
#define INCL_DOSPROCESS
#define INCL_DOSPROFILE
#define INCL_DOSSEMAPHORES
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_DOSDATETIME
#define INCL_LONGLONG
#define INCL_DOSMISC
#define INCL_VIO
#include <os2.h>

typedef HFILE HANDLE;
typedef unsigned char BYTE; 
typedef unsigned short WORD;
typedef unsigned int DWORD;

#include "fat32def.h"

#else

#include <windows.h>
#include <winioctl.h>  // From the Win32 SDK \Mstools\Include, or Visual Studio.Net

#endif

#pragma pack(1)
typedef struct tagFAT_BOOTSECTOR32
{
    // Common fields.
    BYTE sJmpBoot[3];
    BYTE sOEMName[8];
    WORD wBytsPerSec;
    BYTE bSecPerClus;
    WORD wRsvdSecCnt;
    BYTE bNumFATs;
    WORD wRootEntCnt;
    WORD wTotSec16; // if zero, use dTotSec32 instead
    BYTE bMedia;
    WORD wFATSz16;
    WORD wSecPerTrk;
    WORD wNumHeads;
    DWORD dHiddSec;
    DWORD dTotSec32;
    // Fat 32/16 only
    DWORD dFATSz32;
    WORD wExtFlags;
    WORD wFSVer;
    DWORD dRootClus;
    WORD wFSInfo;
    WORD wBkBootSec;
    BYTE Reserved[12];
    BYTE bDrvNum;
    BYTE Reserved1;
    BYTE bBootSig; // == 0x29 if next three fields are ok
    DWORD dBS_VolID;
    BYTE sVolLab[11];
    BYTE sBS_FilSysType[8];
} FAT_BOOTSECTOR32;

typedef struct {
    DWORD dLeadSig;         // 0x41615252
    BYTE sReserved1[480];   // zeros
    DWORD dStrucSig;        // 0x61417272
    DWORD dFree_Count;      // 0xFFFFFFFF
    DWORD dNxt_Free;        // 0xFFFFFFFF
    BYTE sReserved2[12];    // zeros
    DWORD dTrailSig;     // 0xAA550000
} FAT_FSINFO;


#pragma pack()

typedef struct 
    {
    int sectors_per_cluster;        // can be zero for default or 1,2,4,8,16,32 or 64
    char volume_label[12];
    }
format_params;

struct extbpb
{
  WORD BytesPerSect;
  BYTE SectorsPerCluster;
  WORD ReservedSectCount;
  BYTE NumFATs;
  WORD RootDirEnt;
  WORD TotalSectors16;
  BYTE MedisDesc;
  WORD FatSize;
  WORD SectorsPerTrack;
  WORD TracksPerCylinder;
  DWORD HiddenSectors;
  DWORD TotalSectors;
  BYTE Reserved[6];
  ULONGLONG PartitionLength;
};

//
// API
//

void die ( char * error, DWORD rc );
DWORD get_vol_id (void);
void seek_to_sect( HANDLE hDevice, DWORD Sector, DWORD BytesPerSect );
void write_sect ( HANDLE hDevice, DWORD Sector, DWORD BytesPerSector, void *Data, DWORD NumSects );
BOOL write_file ( HANDLE hDevice, BYTE *pData, DWORD ulNumBytes, DWORD *dwWritten );
void zero_sectors ( HANDLE hDevice, DWORD Sector, DWORD BytesPerSect, DWORD NumSects); //, DISK_GEOMETRY* pdgDrive  )
void open_drive (char *path, HANDLE *hDevice);
void lock_drive(HANDLE hDevice);
void unlock_drive(HANDLE hDevice);
void get_drive_params(HANDLE hDevice, struct extbpb *dp);
void set_part_type(UCHAR Dev, HANDLE hDevice, struct extbpb *dp);
void begin_format (HANDLE hDevice);
void remount_media (HANDLE hDevice);
void close_drive(HANDLE hDevice);
void mem_alloc(void **p, ULONG cb);
void mem_free(void *p, ULONG cb);
void query_freq(ULONGLONG *freq);
void query_time(ULONGLONG *time);
void check_vol_label(char *path, char **vol_label);
char *get_vol_label(char *path, char *vol);
void set_vol_label (char *path, char *vol);
void cleanup ( void );
void quit (int rc);
void show_progress (float fPercentWritten);
