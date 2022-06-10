/*****************************************************************************
 * lvmcalls.h                                                                *
 *                                                                           *
 * Copyright (C) 2011-2019 Alexander Taylor.                                 *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify it *
 *   under the terms of the GNU General Public License as published by the   *
 *   Free Software Foundation; either version 2 of the License, or (at your  *
 *   option) any later version.                                              *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful, but     *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of              *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 *   General Public License for more details.                                *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License along *
 *   with this program; if not, write to the Free Software Foundation, Inc., *
 *   59 Temple Place, Suite 330, Boston, MA  02111-1307  USA                 *
 *****************************************************************************/

#ifndef OS2_INCLUDED
    #include <os2.h>
#endif
#include <lvm_intr.h>


/*****************************************************************************
 ** CONSTANTS                                                               **
 *****************************************************************************/

// Additional error codes
#define LVM_ERROR_INCOMPATIBLE_PARTITIONING     0x1000

// Sector size
#define LVM_BYTES_PER_SECTOR            BYTES_PER_SECTOR
#define LVM_SECTORS_PER_MiB             ( 1048576 / BYTES_PER_SECTOR )
#define LVM_SECTOR_ROUNDFIX             (( BYTES_PER_SECTOR / 2 ) - 1 )

// Volume boot status
#define LVM_VOLUME_STATUS_NONE          0   // None
#define LVM_VOLUME_STATUS_BOOTABLE      1   // Bootable
#define LVM_VOLUME_STATUS_STARTABLE     2   // Startable
#define LVM_VOLUME_STATUS_INSTALLABLE   3   // Installable

// Volume device type
#define LVM_DEVICE_HDD                  LVM_HARD_DRIVE  // 0 - hard disk drive
#define LVM_DEVICE_PRM                  LVM_PRM         // 1 - partitioned removable media
#define LVM_DEVICE_CDROM                NON_LVM_CDROM   // 2 - CD/DVD (not controlled by LVM)
#define LVM_DEVICE_NETWORK              NETWORK_DRIVE   // 3 - network (not controlled by LVM)
#define LVM_DEVICE_UNKNOWN              NON_LVM_DEVICE  // 4 - unknown (not controlled by LVM)

// Possible constraints on a newly-created partition
#define LVM_CONSTRAINED_NONE            0
#define LVM_CONSTRAINED_PRIMARY         1
#define LVM_CONSTRAINED_LOGICAL         2
#define LVM_CONSTRAINED_UNUSABLE        9

#define LVM_BOOT_TIMEOUT_LIMIT          999   // Maximum Boot Manager timeout value

// File-system identification strings
#define LVM_FILESYSTEM_BOOTMGR          "Boot Manager"
#define LVM_FILESYSTEM_UNKNOWN          "????"


/*****************************************************************************
 ** FUNCTION ALIASES                                                        **
 *****************************************************************************
 * These aliases are intended to give the LVM API calls a more consistent    *
 * and intelligible naming convention.                                       *
 *****************************************************************************/

// LVM meta-functions
#define LvmClose()                          Close_LVM_Engine()
#define LvmCommit(pE)                       Commit_Changes(pE)
#define LvmOpen(bI, pE)                     Open_LVM_Engine(bI, pE)
#define LvmRefresh(pE)                      Refresh_LVM_Engine(pE)

// Disk-specific functions
#define LvmGetDisks(pE)                     Get_Drive_Control_Data(pE)
#define LvmGetDiskInfo(hD, pE)              Get_Drive_Status(hD, pE)

// Partition-specific functions
#define LvmCreatePartition(hA, iS, aN, AA, bB, bP, bA, pE) \
                                            Create_Partition(hA, iS, aN, AA, bB, bP, bA, pE)
#define LvmDeletePartition(hP, pE)          Delete_Partition(hP, pE)
#define LvmGetPartitionHandle(iS, pE)       Get_Partition_Handle(iS, pE)
#define LvmGetPartitionInfo(hP, pE)         Get_Partition_Information(hP, pE)
#define LvmGetPartitions(hA, pE)            Get_Partitions(hA, pE)
#define LvmSetActiveFlag(hA, bF, pE)        Set_Active_Flag(hA, bF, pE)
#define LvmSetOSFlag(hA, bF, pE)            Set_OS_Flag(hA, bF, pE)

// Volume-specific functions
#define LvmCreateVolume(cN, bL, bB, cL, iF, iC, aH, pE) \
                                            Create_Volume(cN, bL, bB, cL, iF, iC, aH, pE)
#define LvmDeleteVolume(hV, pE)             Delete_Volume(hV, pE)
#define LvmExpandVolume(hV, iC, aH, pE)     Expand_Volume(hV, iC, aH, pE)
#define LvmGetVolumes(pE)                   Get_Volume_Control_Data(pE)
#define LvmGetVolumeInfo(hV, pE)            Get_Volume_Information(hV, pE)
#define LvmHideVolume(hV, pE)               Hide_Volume(hV, pE)
#define LvmSetDriveLetter(hV, cL, pE)       Assign_Drive_Letter(hV, cL, pE)

// Functions applicable to partitions, drives, and volumes:
#define LvmGetValidOptions(hA, pE)          Get_Valid_Options(hA, pE)
#define LvmSetName(hA, aN, pE)              Set_Name(hA, aN, pE)
#define LvmSetStartable(hA, pE)             Set_Startable(hA, pE)

// Boot Manager functions
#define LvmAddToBootMgr(hA, pE)             Add_To_Boot_Manager(hA, pE)
#define LvmGetBootMgr(pE)                   Get_Boot_Manager_Handle(pE)
#define LvmGetBootMgrMenu(pE)               Get_Boot_Manager_Menu(pE)
#define LvmGetBootMgrOptions(hA, bV, bT, iT, bA, pE) \
                                            Get_Boot_Manager_Options(hA, bV, bT, iT, bA, pE)
#define LvmInstallBootMgr(iD, pE)           Install_Boot_Manager(iD, pE)
#define LvmDeleteBootMgr(pE)                Remove_Boot_Manager(pE)
#define LvmRemoveFromBootMgr(hA, pE)        Remove_From_Boot_Manager(hA, pE)
#define LvmSetBootMgrOptions(hA, bT, iT, bA, pE) \
                                            Set_Boot_Manager_Options(hA, bT, iT, bA, pE)

// Functions specific to the OS/2 install program (not used here)
#define LvmGetInstallableVolume(pE)         Get_Installable_Volume(pE)
#define LvmGetInstallFlags(pE)              Get_Install_Flags(pE)
#define LvmGetRebootFlag(pE)                Get_Reboot_Flag(pE)
#define LvmSetInstallable(hV, pE)           Set_Installable(hV, pE)
#define LvmSetInstallFlags(iF, pE)          Set_Install_Flags(iF, pE)
#define LvmSetMinInstallSize(iS)            Set_Min_Install_Size(iS)
#define LvmSetRebootFlag(bR, pE)            Set_Reboot_Flag(bR, pE)

// Other functions
#define LvmAvailableDriveLetters(pE)        Get_Available_Drive_Letters(pE)
#define LvmChangesPending()                 Changes_Pending()
#define LvmExportCfg(pF, pE)                Export_Configuration(pF, pE)
#define LvmFreeMem(p)                       Free_Engine_Memory(p)
#define LvmGetDriveLetterInfo(cL, pD, pP, pL, pU) \
                                            Get_LVM_View(cL, pD, pP, pL, pU)
#define LvmReservedDriveLetters(pE)         Get_Reserved_Drive_Letters(pE)
#define LvmNewMBR(hD, pE)                   New_MBR(hD, pE)
#define LvmReadSectors(iD, lS, iS, hB, pE)  Read_Sectors(iD, lS, iS, hB, pE)
#define LvmRebootRequired()                 Reboot_Required()
#define LvmRediscoverPRMs(pE)               Rediscover_PRMs(pE)
#define LvmSetFreeSpaceThreshold(iS)        Set_Free_Space_Threshold(iS)
#define LvmStartLogging(pF, pE)             Start_Logging(pF, pE)
#define LvmStopLogging(pE)                  Stop_Logging(pE)
#define LvmWriteSectors(iD, lS, iS, hB, pE) Write_Sectors(iD, lS, iS, hB, pE)


/*****************************************************************************
 ** OTHER MACROS                                                            **
 *****************************************************************************/

// Convert a number of sectors into megabytes
#define SECS_TO_MiB( iSecs )    (( iSecs + LVM_SECTOR_ROUNDFIX ) >> 11 )

// Convert a number of megabytes into sectors
#define MiB_TO_SECS( iMB )      (( iMB << 11 ) - LVM_SECTOR_ROUNDFIX )


/*****************************************************************************
 ** DATA TYPES                                                              **
 *****************************************************************************/

// Convenient pointer definitions for basic LVM types
typedef ADDRESS     *PADDRESS;
typedef BOOLEAN     *PBOOLEAN;
typedef CARDINAL    *PCARDINAL;
typedef CARDINAL16  *PCARDINAL16;
typedef CARDINAL32  *PCARDINAL32;
typedef INTEGER     *PINTEGER;
typedef INTEGER16   *PINTEGER16;
typedef INTEGER32   *PINTEGER32;
typedef REAL32      *PREAL32;
typedef REAL64      *PREAL64;

/* This structure is basically a concatenation of the LVM Drive_Control_Data and
 * Drive_Information_Record structures, which places all the information
 * together in one place for convenience.  We make some modifications to the
 * actual values, however; for instance, converting the size value from sectors
 * to megabytes.
 */
typedef struct _LVM_Disk_Info {
    CARDINAL32   iNumber;                   // OS/2 drive number of the disk
    CARDINAL32   iSize;                     // total size of the disk
    DoubleWord   iSerial;                   // disk serial number
    ADDRESS      handle;                    // LVM handle identifying this disk
    CARDINAL32   iCylinders;                // number of cylinders
    CARDINAL32   iHeads;                    // number of heads per cylinder
    CARDINAL32   iSectors;                  // number of sectors per track
    BOOLEAN      fPRM;                      // disk is partitioned removable media
    BYTE         reserved1[ 3 ];            // (reserved field for alignment)
    CARDINAL32   iTotalFree;                // total number of free sectors on the disk
    CARDINAL32   iLargestFree;              // size of the largest block of free space, in sectors
    BOOLEAN      fCorrupt;                  // disk partitioning information is invalid
    BOOLEAN      fUnusable;                 // disk MBR cannot be accessed
    BOOLEAN      fIOErr;                    // last I/O operation on the disk failed
    BOOLEAN      fBigFloppy;                // disk is a "large floppy" device
    char         szName[ DISK_NAME_SIZE ];  // user-assigned name of the disk
} LVMDISKINFO, *PLVMDISKINFO;


/* Similarly, this structure concatenates the fields from Volume_Control_Data
 * and Volume_Information_Record structures.  Again, we convert the size from
 * sectors to MiB when populating the structure.
 */
typedef struct _LVM_Volume_Info {
    DoubleWord iSerial;                      // serial number of this volume
    ADDRESS    handle;                       // handle of this volume
    BOOLEAN    fCompatibility;               // volume is a compatibility volume
    BYTE       bDevice;                      // type of device on which volume resides
    BYTE       reserved1[ 2 ];               //  (reserved field for alignment)
    CARDINAL32 iSize;                        // size of this volume
    CARDINAL32 iPartitions;                  // number of partitions in this volume
    CARDINAL32 iConflict;                    // a drive letter conflict occurred
    BYTE       reserved2;                    //  (actually a duplicate of fCompatibility)
    BOOLEAN    fBootable;                    // volume is bootable
    char       cPreference;                  // preferred (user-selected) drive letter
    char       cLetter;                      // actual current drive letter
    char       cInitial;                     // drive letter when LVM was opened
    BOOLEAN    fNew;                         // volume was created during this session
    BYTE       iStatus;                      // bootability status
    BYTE       reserved3;                    //  (reserved field for alignment)
    char       szName[ VOLUME_NAME_SIZE ];   // user-assigned volume name
    char       szFS[ FILESYSTEM_NAME_SIZE ]; // name of filesystem on this volume
} LVMVOLUMEINFO, *PLVMVOLUMEINFO;


