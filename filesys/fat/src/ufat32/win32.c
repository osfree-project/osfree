/*  Win32-specific functions
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fat32c.h"

extern HANDLE hDev;

DWORD get_vol_id (void)
{
    SYSTEMTIME s;
    DWORD d;
    WORD lo,hi,tmp;

    GetLocalTime( &s );

    lo = s.wDay + ( s.wMonth << 8 );
    tmp = (s.wMilliseconds/10) + (s.wSecond << 8 );
    lo += tmp;

    hi = s.wMinute + ( s.wHour << 8 );
    hi += s.wYear;
   
    d = lo + (hi << 16);
    return(d);
}

void die ( char * error, DWORD rc )
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

	if ( dw )
		{
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );

		// Display the error message and exit the process

		fprintf ( stderr, "%s\nGetLastError()=%d: %s\n", error, dw, lpMsgBuf );	
		}
	else
		{
		fprintf ( stderr, "%s\n", error );	
		}

    LocalFree(lpMsgBuf);
 
    quit (dw);
}

void seek_to_sect( HANDLE hDevice, DWORD Sector, DWORD BytesPerSect )
{
    LONGLONG Offset;
    LONG HiOffset;
    
    Offset = Sector * BytesPerSect ;
    HiOffset = (LONG) (Offset>>32);
    SetFilePointer ( hDevice, (LONG) Offset , &HiOffset , FILE_BEGIN );
}

void write_sect ( HANDLE hDevice, DWORD Sector, DWORD BytesPerSector, void *Data, DWORD NumSects )
{
    DWORD dwWritten;
    BOOL ret;

    seek_to_sect ( hDevice, Sector, BytesPerSector );
    ret=WriteFile ( hDevice, Data, NumSects*BytesPerSector, &dwWritten, NULL );

    if ( !ret )
        die ( "Failed to write", ret );
}

BOOL write_file ( HANDLE hDevice, BYTE *pData, DWORD ulNumBytes, DWORD *dwWritten )
{
    return WriteFile ( hDevice, pData, ulNumBytes, dwWritten, NULL );
}

void open_drive (char *path , HANDLE *hDevice)
{
  char DriveDevicePath[]="\\\\.\\Z:"; // for CreateFile
  char *p = path;

  if (strlen(path) == 2 && path[1] == ':')
  {
    // drive letter (UNC)
    DriveDevicePath[4] = *path;
    p = DriveDevicePath;
  }

  // open the drive
  *hDevice = CreateFile (
      p,  
      GENERIC_READ | GENERIC_WRITE,
      0,
      NULL, 
      OPEN_EXISTING, 
      FILE_FLAG_NO_BUFFERING,
      NULL);
  if ( *hDevice ==  INVALID_HANDLE_VALUE )
      die( "Failed to open device - close any files before formatting,"
           "and make sure you have Admin rights when using fat32format"
            "Are you SURE you're formatting the RIGHT DRIVE!!!", -1 );

  hDev = *hDevice;
}

void lock_drive(HANDLE hDevice)
{
  BOOL bRet;
  DWORD cbRet;

  // lock it
  bRet = DeviceIoControl( hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &cbRet, NULL );

  if ( !bRet )
      die( "Failed to lock device", bRet );
}

void unlock_drive(HANDLE hDevice)
{
  BOOL  bRet;
  DWORD cbRet;

  bRet = DeviceIoControl( hDevice, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &cbRet, NULL );

  if ( !bRet )
      die( "Failed to unlock device", -8 );
}

void get_drive_params(HANDLE hDevice, struct extbpb *dp)
{
  BOOL  bRet;
  DWORD cbRet;
  DISK_GEOMETRY          dgDrive;
  PARTITION_INFORMATION  piDrive;

  // work out drive params
  bRet = DeviceIoControl ( hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY,
                           NULL, 0, &dgDrive, sizeof(dgDrive),
                           &cbRet, NULL);

  if ( !bRet )
      die( "Failed to get device geometry", -10 );

  bRet = DeviceIoControl ( hDevice, 
        IOCTL_DISK_GET_PARTITION_INFO,
        NULL, 0, &piDrive, sizeof(piDrive),
        &cbRet, NULL);

  if ( !bRet )
      die( "Failed to get parition info", -10 );

  // Only support hard disks at the moment 
  //if ( dgDrive.BytesPerSector != 512 )
  //{
  //    die ( "This version of fat32format only supports hard disks with 512 bytes per sector.\n" );
  //}
  dp->BytesPerSect = dgDrive.BytesPerSector;
  dp->PartitionLength = piDrive.PartitionLength.QuadPart;
  dp->SectorsPerTrack = dgDrive.SectorsPerTrack;
  dp->HiddenSectors =  piDrive.HiddenSectors;
  dp->TracksPerCylinder = dgDrive.TracksPerCylinder;
}

void set_part_type(UCHAR Drv, HANDLE hDevice, struct extbpb *dp)
{
  BOOL  bRet;
  DWORD cbRet;
  SET_PARTITION_INFORMATION spiDrive;

  spiDrive.PartitionType = 0x0c; // FAT32 LBA. 
  bRet = DeviceIoControl ( hDevice, 
      IOCTL_DISK_SET_PARTITION_INFO,
      &spiDrive, sizeof(spiDrive),
      NULL, 0, 
      &cbRet, NULL);

  if ( !bRet )
      {
      // This happens because the drive is a Super Floppy
      // i.e. with no partition table. Disk.sys creates a PARTITION_INFORMATION
      // record spanning the whole disk and then fails requests to set the 
      // partition info since it's not actually stored on disk. 
      // So only complain if there really is a partition table to set      
      if ( dp->HiddenSectors  )
	  die( "Failed to set parition info", -6 );
      }    
}

void begin_format (HANDLE hDevice)
{
    // none
}

void remount_media (HANDLE hDevice)
{
  BOOL bRet;
  DWORD cbRet;

  bRet = DeviceIoControl( hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &cbRet, NULL );

  if ( !bRet )
      die( "Failed to dismount device", -7 );

  unlock_drive( hDevice );
}

void close_drive(HANDLE hDevice)
{
    // Close Device
    CloseHandle( hDevice );
}


void mem_alloc(void **p, ULONG cb)
{
    *p = (void *) VirtualAlloc ( NULL, cb, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
}

void mem_free(void *p, ULONG cb)
{
    if (p)
        VirtualFree(p, cb, 0);
}

void query_freq(ULONGLONG *freq)
{
    LARGE_INTEGER *frequency = (LARGE_INTEGER *)freq;
    QueryPerformanceFrequency( frequency );
}

void query_time(ULONGLONG *time)
{
    LARGE_INTEGER *t = (LARGE_INTEGER *)time;
    QueryPerformanceCounter( t );
}

void check_vol_label(char *path, char **vol_label)
{
    char c;

    printf ( "Warning ALL data on drive '%s' will be lost irretrievably, are you sure\n(y/n) ", path );
    
    c = getchar();

    if ( toupper(c) != 'Y' )
        quit (1);
}

char *get_vol_label(char *path, char *vol)
{
    return vol;
}

void set_vol_label (char *path, char *vol)
{

}

void show_progress (char *str)
{

}
