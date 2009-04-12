/* $Id: devices.c,v 1.7 2004/07/24 00:49:06 pasha Exp $ */

static char *SCCSID = "@(#)1.22  3/12/99 10:31:14 src/jfs/utils/libfs/devices.c, jfslib, w45.fs32, 990417.1";
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *   MODULE_NAME:		devices.c
 *
 *   COMPONENT_NAME: 	jfslib
 *
 *   FUNCTIONS:
 *              ujfs_beginformat
 *              ujfs_check_adapter
 *              ujfs_close
 *              ujfs_get_dev_size
 *              ujfs_open_device
 *              ujfs_redeterminemedia
 *              ujfs_rw_diskblocks
 *              ujfs_rw_mbr
 *              ujfs_stdout_redirected
 *              ujfs_update_mbr
 *              ujfs_verify_device_type
 *
*/
/*  History
 *   PS15032004 - fix for work on flash drive
 *   PS16032004 - insert boot code
 */
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_DOSDEVIOCTL
  #define INCL_DOSDEVICES
  #include <os2.h>
  #include "jfs_types.h"
  #include <jfs_aixisms.h>
  #include <sysbloks.h>
  #include <fsd.h>
  #include <io.h>
  #include "mbr.h"
  #include <sesrqpkt.h>
  #include <ioctl.h>
#include <specdefs.h>
#include <dskioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "devices.h"
#include "debug.h"

/* The following variables are used for direct access */
/* I copy the definition of struct DPB and TRACKLAYOUT here
 * for easier to read code.
 *
** bios parameter block **

struct BPB {                         ** bios parameter block             **
  unsigned bytes_per_sector;         ** sector size                   2  **
  unsigned char sectors_per_cluster; ** sectors per allocation unit   1  **
  unsigned reserved_sectors;         ** number of reserved sectors    2  **
  unsigned char nbr_fats;            ** number of fats                1  **
  unsigned root_entries;             ** number of directory entries   2  **
  unsigned total_sectors;            ** number of sectors             2  **
  char media_type;                   ** fatid byte                    1  **
  unsigned sectors_per_fat;          ** sectors in a copy of the FAT  2  **
  unsigned sectors_per_track;        ** number of sectors per track   2  **
  unsigned number_of_heads;          ** number of heads               2  **
  unsigned hidden_sectors;           ** number of hidden sectors      2  **
  unsigned reserved_1;               **                               2  **
  unsigned large_total_sectors;      ** large total sectors           2  **
  unsigned reserved_2;               **                               2  **
  char reserved_3[6];                ** 6 reserved bytes              6  **
};                                   **     total byte size = 31         **

** device parameter block **

struct DPB {                         ** device parameter block           **
  struct BPB dev_bpb;                ** 31 byte extended bpb             **
  unsigned number_of_tracks;         ** number of tracks                 **
  char device_type;                  ** device type        see DT_       **
  unsigned device_attributes;        ** device attributes  see DA_       **
};

typedef struct _TRACKLAYOUT {
        BYTE   bCommand;
        USHORT usHead;
        USHORT usCylinder;
        USHORT usFirstSector;
        USHORT cSectors;
        struct {
                USHORT usSectorNumber;
                USHORT usSectorSize;
        } TrackTable[1];
} TRACKLAYOUT;
typedef TRACKLAYOUT FAR *PTRACKLAYOUT;
*********/
extern char BootCode[];     //PS16032004 - Boot code for bootblock
struct DPB  Datap;
/* note: In the data packet format for DSK_GETDEVICEPARAMS, the
 *       second field is "number of cylinders". It is equivalent
 *       to "number_of_tracks". It indicates the maximum number of
 *       cylinders for this partition.
 *       Assuming that a partition should always align to the cylinder
 *       boundary.
 */
TRACKLAYOUT *ptrklay;   /* pointer to TRACKLAYOUT. Note that the
                                * tracktable should have been inited
                                * in open_device routine, other fields
                                * will be set up in rw_diskblocks routine.
                                */

/*
 * NAME: ujfs_get_dev_size
 *
 * FUNCTION: Uses the device driver interface to determine the raw capacity of
 *      the specified device.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      device  - device
 *      size    - filled in with size of device; not modified if failure occurs
 *      hidden  - filled in with size of hidden part of device; not modified if
 *                failure occurs
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 if successful; anything else indicates failures
 */
int32 ujfs_get_dev_size( HFILE  device,
                         int64  *size,
                         int64  *hidden)
{
  int32               rc;
  unsigned char       CommandInfo = 0;
  unsigned long       ParmLen = sizeof(CommandInfo);
  struct DPB          DataPacket;
  unsigned long       DataLen = sizeof(DataPacket);
  register struct BPB *bpb;

  /*
   * Issue the call to get the raw device capacity
   */

  /*
   * As of right now, I don't understand the meaning of total_sectors and
   * large_total_sectors.  I think the sum gives me what I want
   */

  rc = DosDevIOCtl(device, IOCTL_DISK, DSK_GETDEVICEPARAMS, &CommandInfo,
                   sizeof(CommandInfo), &ParmLen, &DataPacket,
                   sizeof(DataPacket), &DataLen);
#if 0
  rc = DosDevIOCtl(&DataPacket, &CommandInfo, DSK_GETDEVICEPARAMS,
                   IOCTL_DISK, device);
#endif /* 0 */

  if ( rc == 0 )
  {
    bpb = &DataPacket.dev_bpb;
    *size = (int64)bpb->bytes_per_sector *
            (int64)(bpb->total_sectors + bpb->large_total_sectors);
    *hidden = (int64)bpb->bytes_per_sector * (int64)bpb->hidden_sectors;
  }
  return(rc);
}

/*
 * NAME: ujfs_open_device
 *
 * FUNCTION: Open the specified device and return the handle and  the
 *           struct DPB information.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      Device          - name of device to open
 *      FileHandle      - Filled in with handle specified device
 *      SectorSize      - Filled in with sector size of specified device; not
 *                        modified if failure occurs
 *      mode            - Indicates to open read-only, or read-write exclusive
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 for success; anything else indicates a failure
 */
int32 ujfs_open_device( char    *Device,
                        PHFILE  FileHandle,
                        int32  *SectorSize,
                        int32   mode )
{
  int32               rc = 0;
  ULONG               Action;
  int32               i;
  unsigned char       CommandInfo = 0;
  unsigned long       ParmLen = sizeof(CommandInfo);
  unsigned long       DataLen = sizeof(struct DPB);
  USHORT ulSecsPerTrk;
  struct DPB *tmp_dpb = &Datap;
  TRACKLAYOUT *tmp_layout = ptrklay;

  DBG_ERROR(("open_device: %s mode %s\n",Device,mode == RDWR_EXCL?"RDWR_EXCL":"READ"));

  if ( mode == RDWR_EXCL )
  {
    rc = DosOpen(Device, FileHandle, &Action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS, OPEN_FLAGS_DASD |
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYREADWRITE |
                 OPEN_ACCESS_READWRITE, 0);
  }
  else
  {
    rc = DosOpen(Device, FileHandle, &Action, 0, 0,
                 OPEN_ACTION_OPEN_IF_EXISTS, OPEN_FLAGS_DASD |
                 OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                 OPEN_ACCESS_READONLY, 0);
  }

  if ( rc != NO_ERROR )
  {
    return rc;
  }

  /*
   * Lock Device for exclusive mode only
   */
  if ( mode & RDWR_EXCL )
  {
    rc = DosDevIOCtl(*FileHandle, IOCTL_DISK, DSK_LOCKDRIVE, &CommandInfo,
                     sizeof(CommandInfo), &ParmLen, &CommandInfo,
                     sizeof(CommandInfo), &DataLen);
    if ( rc != NO_ERROR )
    {
      return rc;
    }
  }

  /*
   * Get the device information
   */
  rc = DosDevIOCtl(*FileHandle, IOCTL_DISK, DSK_GETDEVICEPARAMS,
                   &CommandInfo, sizeof(CommandInfo), &ParmLen, &Datap,
                   sizeof(struct DPB), &DataLen);

  if ( rc == NO_ERROR)
  {
    *SectorSize = Datap.dev_bpb.bytes_per_sector;
    ulSecsPerTrk =  Datap.dev_bpb.sectors_per_track;
    ptrklay =(TRACKLAYOUT *)malloc(sizeof(TRACKLAYOUT) + 4 * ulSecsPerTrk );
    if ( ptrklay == NULL )
    {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      return rc;
    }
    for (i = 0; i < ulSecsPerTrk; i++)
    {
      ptrklay->TrackTable[i].usSectorSize = *SectorSize;;
      ptrklay->TrackTable[i].usSectorNumber = i + 1;
    }
  }

  return rc;
}

/*
 * NAME: ujfs_beginformat
 *
 * FUNCTION: unmounts current FSD, and forces JFS to mount the drive
 *
 * PRE CONDITIONS: Drive has been opened and locked
 *
 * POST CONDITIONS: OS/2 and IFS are aware that a format is occurring
 *
 * PARAMETERS:
 *      device  - file handle of an opened device
 *
 * NOTES: This function must be followed by a call to ujfs_redeterminemedia
 *        before the drive is closed.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: return code from DosDevIOCtl
 */
int32 ujfs_beginformat(HFILE device)
{
  unsigned char   CommandInfo = 0;
  unsigned long   ParmLen = sizeof(CommandInfo);
  char            FSDname[] = "JFS";
  unsigned long   DataLen = sizeof(FSDname);

  return DosDevIOCtl(device, IOCTL_DISK, DSK_BEGINFORMAT, &CommandInfo,
                     sizeof(CommandInfo), &ParmLen, FSDname,
                     sizeof(FSDname), &DataLen);
}

/*
 * NAME: ujfs_redeterminemedia
 *
 * FUNCTION: unmounts current FSD, and causes OS/2 to rebuild VPB and mount
 *           the filesystem
 *
 * PRE CONDITIONS: Drive has been opened and locked
 *
 * POST CONDITIONS: File system is mounted - format condition cleared if present
 *
 * PARAMETERS:
 *      device  - file handle of an opened device
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: return code from DosDevIOCtl
 */
int32 ujfs_redeterminemedia(HFILE device)
{
  unsigned char   CommandInfo = 0;
  unsigned long   ParmLen = sizeof(CommandInfo);
  unsigned char   Reserved = 0;
  unsigned long   DataLen = sizeof(Reserved);

  return DosDevIOCtl(device, IOCTL_DISK, DSK_REDETERMINEMEDIA,
                     &CommandInfo, sizeof(CommandInfo), &ParmLen,
                     &Reserved, sizeof(Reserved), &DataLen);
}

/*
 * NAME: ujfs_rw_diskblocks
 *
 * FUNCTION: Read/Write specific number of bytes for an opened device.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      dev_ptr         - file handle of an opened device to read/write
 *      disk_offset     - byte offset from beginning of device for start of disk
 *                        block read/write
 *      disk_count      - number of bytes to read/write
 *      data_buffer     - On read this will be filled in with data read from
 *                        disk; on write this contains data to be written
 *      mode            - GET: read; PUT: write; VRFY: verify
 *
 * NOTES: A disk address is formed by {#cylinder, #head, #sector}
 *      In order to call this routine, the device must be opened by
 *      calling ujfs_open_device() so that the static global Datap
 *      and ptrklay are properly inited.
 *
 *      Also the DSK_READTRACK and DSK_WRITETRACK is a track based
 *      function. If it needs to read/write crossing track boundary,
 *      additional calls are used.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 */
int32 ujfs_rw_diskblocks( HFILE dev_ptr,
                          int64 disk_offset,
                          int32 disk_count,
                          void  *data_buffer,
                          int32 mode )
{

  uint32              actual;
  int32               rc;
  ULONG ulPhys_sector;
  ULONG ulNumSectors;
  ULONG ulSectorsPerCylinder;
  USHORT ulSecsPerTrk;
  USHORT numSecs, sCylinder, sHead, sFirstSec;
  uint32 ulbytesPsec;
  unsigned long       ParmLen ;
  struct DPB *Pdpb;
  TRACKLAYOUT *tmp_layout;
  char *ptr;

  Pdpb = &Datap;
  ulbytesPsec = Pdpb->dev_bpb.bytes_per_sector;
  DBG_ERROR(("rw_diskblocks: %s disk_offset %lld disk_count %d \n",mode == GET?"GET":"PUT",disk_offset,disk_count));
  if ( (disk_offset % ulbytesPsec) || (disk_count % ulbytesPsec ) )
  {
    DBG_ERROR(("Internal Error: %s(%d): disk_offset or disk_count is wrong \n",
               __FILE__, __LINE__ ))
    return ERROR_INVALID_PARAMETER;
  }
  tmp_layout = ptrklay;
  ulSecsPerTrk =  Pdpb->dev_bpb.sectors_per_track;
  ParmLen = sizeof(TRACKLAYOUT);
  ulPhys_sector = disk_offset / Pdpb->dev_bpb.bytes_per_sector;
  ulPhys_sector += Pdpb->dev_bpb.hidden_sectors;
  ulSectorsPerCylinder = ulSecsPerTrk * Pdpb->dev_bpb.number_of_heads;
  sCylinder = (USHORT)((ulPhys_sector) / ulSectorsPerCylinder);
  if ( sCylinder > Pdpb->number_of_tracks )
  {
    DBG_ERROR(("Internal error: %s(%d): Cylinder number %d beyond the maximum\n",
               __FILE__, __LINE__, sCylinder ))
    return ERROR_INVALID_PARAMETER;
  }
  ptr = (char *)data_buffer;
  ulNumSectors = disk_count / Pdpb->dev_bpb.bytes_per_sector;
  sHead = (USHORT)((ulPhys_sector % ulSectorsPerCylinder) / ulSecsPerTrk);
  sFirstSec = (USHORT)((ulPhys_sector % ulSectorsPerCylinder) % ulSecsPerTrk);
  ptrklay->bCommand = 0;
  ptrklay->usCylinder = sCylinder;
  DBG_ERROR(("rw_diskblocks: usCylinder %d MaxCylinder %d \n",ptrklay->usCylinder,Pdpb->number_of_tracks));

  while ( ptrklay->usCylinder <= Pdpb->number_of_tracks )    //PS15032004
  {
    ptrklay->usHead = sHead;
    while ( ptrklay->usHead  < Pdpb->dev_bpb.number_of_heads )
    {
      ptrklay->usFirstSector =  sFirstSec;
      numSecs = ulSecsPerTrk - ptrklay->usFirstSector;
      ptrklay->cSectors =
      (numSecs > ulNumSectors) ? ulNumSectors : numSecs;
      actual = ptrklay->cSectors * ulbytesPsec ;
  DBG_ERROR(("rw_diskblocks: Head %d Sector %d Cylinder %d \n",ptrklay->usHead,ptrklay->cSectors,ptrklay->usCylinder));
      switch ( mode )
      {
      case GET:
        rc = DosDevIOCtl(dev_ptr,
                         IOCTL_DISK,
                         DSK_READTRACK,
                         (PVOID)ptrklay,
                         ParmLen,
                         &ParmLen,
                         (void *)ptr,
                         (ULONG)actual,
                         (PULONG)&actual);

  #if 0
        rc = DosRead(dev_ptr, data_buffer, (int32)disk_count,
                     (PULONG)&actual);
        if ((rc == NO_ERROR) && (actual < disk_count))
          rc = ERROR_READ_FAULT;
  #endif
        break;
      case PUT:
        rc = DosDevIOCtl(dev_ptr,
                         IOCTL_DISK,
                         DSK_WRITETRACK,
                         (PVOID)ptrklay,
                         ParmLen,
                         &ParmLen,
                         (void *)ptr,
                         (ULONG)actual,
                         (PULONG)&actual);
  #if 0
        rc = DosWrite(dev_ptr, data_buffer, (int32)disk_count,
                      (PULONG)&actual);
        if ((rc == NO_ERROR) && (actual < disk_count))
          rc = ERROR_WRITE_FAULT;
  #endif
        break;
      case VRFY:
        rc = DosDevIOCtl(dev_ptr,
                         IOCTL_DISK,
                         DSK_VERIFYTRACK,
                         (PVOID)ptrklay,
                         ParmLen,
                         &ParmLen,
                         (void *)ptr,
                         (ULONG)actual,
                         (PULONG)&actual);
        break;
      default:
        DBG_ERROR(("Internal error: %s(%d): bad mode: %d\n",
                   __FILE__, __LINE__, mode))
        rc = ERROR_INVALID_HANDLE;
        break;
      }

      if ( rc != 0 )
      {
        DBG_ERROR(("Internal error: %s(%d): error %d\n",
                __FILE__, __LINE__, rc))
        return rc;
      }
      ulNumSectors -= ptrklay->cSectors;
      if ( ulNumSectors == 0 )
        goto outloop;
      else
      {
        ptr += actual;
        ptrklay->usHead++;
        sFirstSec = 0; /* for the next track, starting from
                        * sector 0
                        */
      }
    }
    ptrklay->usCylinder++;
    sHead = 0;  /* for the next cylinder, starting from head 0 */
  }
  if ( ulNumSectors > 0 )
  {
    DBG_ERROR(("Internal error: %s(%d): too many sectors to r/w\n",  __FILE__, __LINE__ ))
    return ERROR_INVALID_PARAMETER;
  }
  outloop:
  return rc;
}

/*
 * NAME: ujfs_close
 *
 * FUNCTION: Close the specified device and free the space for
 *           track layout table.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      Device          - File handle of the opened device
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 for success; anything else indicates a failure
 */
int32 ujfs_close( HFILE device)
{
  char            CommandInfo = '\0';
  unsigned long   ParmLen = sizeof(CommandInfo);
  unsigned long   DataLen = sizeof(CommandInfo);

  DosDevIOCtl(device, IOCTL_DISK, DSK_UNLOCKDRIVE, &CommandInfo,
              sizeof(CommandInfo), &ParmLen, &CommandInfo,
              sizeof(CommandInfo), &DataLen);
  free(ptrklay);
  return DosClose(device);
}


/*
 * NAME: ujfs_rw_mbr
 *
 * FUNCTION: Reads/writes the master boot record for a partition.
 *
 * PARAMETERS:
 *      dev_ptr - Device handle
 *      ourmbr  - Master boot record to write, or read into
 *      mode    - GET: read; PUT: write
 *
 * NOTES: Reads or writes to cylinder 0, head 0, sector 0 of the partition.
 *
 * RETURNS: 0 for success; Other indicates failure
 */
static int32
ujfs_rw_mbr(HFILE       dev_ptr,
            struct mbr  *ourmbr,
            int32       mode)
{
  int32       rc;
  ULONG       actual = sizeof(struct mbr);
  ULONG       ParmLen = sizeof(TRACKLAYOUT);

  ptrklay->usFirstSector = 0;
  ptrklay->usHead = 0;
  ptrklay->usCylinder = 0;
  ptrklay->cSectors = 1;

  switch (mode)
  {
  case GET:
    rc = DosDevIOCtl(dev_ptr, IOCTL_DISK, DSK_READTRACK, (PVOID)ptrklay,
                     ParmLen, &ParmLen, (void *)ourmbr, actual, &actual);
    break;
  case PUT:
    rc = DosDevIOCtl(dev_ptr, IOCTL_DISK, DSK_WRITETRACK,
                     (PVOID)ptrklay, ParmLen, &ParmLen, (void *)ourmbr,
                     actual, &actual);
    break;
  default:
    DBG_ERROR(("Internal error: %s(%d): bad mode: %d\n", __FILE__,
            __LINE__, mode))
    rc = ERROR_INVALID_HANDLE;
    break;
  }
  return rc;
}


/*
 * NAME: ujfs_update_mbr
 *
 * FUNCTION: Update master boot record's partition type to IFS_PART,
 *
 * PARAMETERS:
 *      dev_ptr - Handle for device
 *
 * NOTES: Read master boot record for the partition and set its partition type
 *      to indicate an IFS partition.
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
ujfs_update_mbr(HFILE dev_ptr)
{
  int32       rc;
  struct mbr  ourmbr;
  int16       index;
  UCHAR       cur_systind;

  /* Get the master boot record */
  if (rc = ujfs_rw_mbr(dev_ptr, &ourmbr, GET))
    return rc;

  /*
   * Look for a DOS partition: I copied this straight out of the HPFS code.
   * It is looking for the different possible DOS partition types, or an IFS
   * partition type.  I don't understand why it uses the first one it sees.
   */
  /* ADDITIONAL NOTE: It doesn't use the first one it sees, it tries to match
   * the location (lsn/bpb_hidden_sectors).  If not equal, it moves on.
   */
  /* 201823 Begin */
  for ( index=0; index < 4; index++ )
  {
// printf("Find ID %d lsn %d total %d\n",ourmbr.ptbl[index].systind,ourmbr.ptbl[index].lsn,ourmbr.ptbl[index].nsects);
     if (ourmbr.ptbl[index].lsn == Datap.dev_bpb.hidden_sectors)
     {
        cur_systind = ourmbr.ptbl[index].systind;
        /* Found partition, need to change system indicator? */
        if ((cur_systind & ~HIDDEN_PART_MASK) != IFS_PART)
        {
           /* If system indicator is within known range, get hidden att. */
           if (cur_systind <= 0x1f)
              cur_systind = cur_systind & HIDDEN_PART_MASK;
           else
              cur_systind = 0;

           /* Set partition type to IFS, preserving hidden attribute */
           ourmbr.ptbl[index].systind = IFS_PART | cur_systind;

           rc = ujfs_rw_mbr(dev_ptr, &ourmbr, PUT);
        }

        break;
     }
  }
  /* 201823 End */


  return rc;
}


/*
 * NAME: ujfs_check_adapter
 *
 * FUNCTION: Checks to see if adapter supports memory above 16 Megabytes
 *
 * PARAMETERS: none
 *
 * NOTES: Device Parmeter Block has been read in by ujfs_open_device
 *
 * RETURNS: 0 for success; Other indicates failure
 */
int32
ujfs_check_adapter()
{
	if (Datap.device_attributes & DA_ABOVE16MB)
		return 0;
	else
		return ERROR_NOT_SUPPORTED;
}
	
	
/*
 * NAME: 	ujfs_verify_device_type
 *
 * FUNCTION: 	Query the LVM for the partition type.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 ujfs_verify_device_type( HFILE Dev_IOPort )
{
    int32 vdt_rc = 0;
	
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
	
	/*
	 * initialize the LVM DosDevIOCtl parm and data packets
	 */
    pLVMpp->Command = 0;			/* Identify Volume Type */
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = 0;
    pLVMpp->LSN = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL;

    ppLen = sizeof(DDI_OS2LVM_param);
    dpLen = sizeof(DDI_OS2LVM_data);
	
	/*
	 * ask the LVM how many bad block lists it has for the filesystem
	 */
    vdt_rc = DosDevIOCtl( Dev_IOPort,			IOC_DC,	
			    IODC_LV,			(void *) pLVMpp,	
			    sizeof(DDI_OS2LVM_param),
			    &ppLen,			(void *) pLVMdp,	
			    sizeof(DDI_OS2LVM_data),	&dpLen
			    );	
	
    if( vdt_rc == 0 ) {   /* DosDevIOCtl successful */
      if( pLVMdp->ReturnData != 2 ) {	/* It's not an LVM volume */
        vdt_rc = ERROR_NOT_SUPPORTED;
        }
      }  /* end else DosDevIOCtl successful */

    return( vdt_rc );
}				/* end ujfs_verify_device_type() */


/*
 * NAME: ujfs_stdout_redirected
 *
 * FUNCTION: Calls the DosQueryHType API on the standard output
 *                handle to determine whether standard output is redirected.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:   0  if standard out IS NOT redirected
 *               -1  if standard out IS redirected
 */

int32 ujfs_stdout_redirected( )
{
  ULONG handtype, devattr;

  DosQueryHType (1, &handtype, &devattr);
  if( handtype != 1 )
    return( -1 );
  else
    return( 0 );
}

