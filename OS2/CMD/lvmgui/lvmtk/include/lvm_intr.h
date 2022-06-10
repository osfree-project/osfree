/*
*
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
* Module: lvm_intr.h
*/

/*
* Change History:
*
*/

/*
* Functions: void                         Open_LVM_Engine
*            void                         Close_LVM_Engine
*            Drive_Control_Array          Get_Drive_Control_Data
*            Drive_Information_Record     Get_Drive_Status
*            Partition_Information_Array  Get_Partitions
*            Partition_Information_Record Get_Partition_Information
*            void                         Create_Partition
*            void                         Delete_Partition
*            void                         Set_Active_Flag
*            void                         Set_OS_Flag
*            Volume_Control_Array         Get_Volume_Control_Data
*            Volume_Information_Record    Get_Volume_Information
*            void                         Create_Volume
*            void                         Hide_Volume
*            void                         Expand_Volume
*            void                         Assign_Drive_Letter
*            void                         Set_Installable
*            void                         Set_Name
*            CARDINAL32                   Get_Valid_Options
*            BOOLEAN                      Boot_Manager_Is_Installed
*            void                         Add_To_Boot_Manager
*            void                         Remove_From_Boot_Manager
*            Boot_Manager_Menu            Get_Boot_Manager_Menu
*            void                         Install_Boot_Manager
*            void                         Remove_Boot_Manager
*            void                         Set_Boot_Manager_Options
*            void                         Get_Boot_Manager_Options
*            void                         New_MBR
*            CARDINAL32                   Get_Available_Drive_Letters
*            BOOLEAN                      Reboot_Required
*            void                         Set_Reboot_Flag
*            BOOLEAN                      Get_Reboot_Flag
*            void                         Set_Min_Install_Size
*            void                         Start_Logging
*            void                         Stop_Logging
*            void                         Export_Configuration
*
* Description:  This module defines the interface to LVM.DLL, which is the
*               engine that performs all of the disk partitioning/volume
*               creation work.
*
*/

#ifndef LVM_INTERFACE_H_INCLUDED

#define LVM_INTERFACE_H_INCLUDED 1

#include "lvm_gbls.h"         /* BYTE, CARDINAL32, ADDRESS, BOOLEAN, BYTE */

#include "lvm_cons.h"         /* DISK_NAME_SIZE, PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, FILESYSTEM_NAME_SIZE */
#include "lvm_type.h"         /* DoubleWord */

/* The following are invariant for a disk drive. */
typedef struct _Drive_Control_Record {
CARDINAL32   Drive_Number;                   /* OS/2 Drive Number for this drive. */
CARDINAL32   Drive_Size;                     /* The total number of sectors on the drive. */
DoubleWord   Drive_Serial_Number;            /* The serial number assigned to this drive.  For info. purposes only. */
ADDRESS      Drive_Handle;                   /* Handle used for operations on the disk that this record corresponds to. */
CARDINAL32   Cylinder_Count;                 /* The number of cylinders on the drive. */
CARDINAL32   Heads_Per_Cylinder;             /* The number of heads per cylinder for this drive. */
CARDINAL32   Sectors_Per_Track;              /* The number of sectors per track for this drive. */
BOOLEAN      Drive_Is_PRM;                   /* Set to TRUE if this drive is a PRM. */
BYTE         Reserved[3];                    /* Alignment. */
} Drive_Control_Record;

/* The following structure is returned by the Get_Drive_Control_Data function. */
typedef struct _Drive_Control_Array{
Drive_Control_Record *   Drive_Control_Data;       /* An array of drive control records. */
CARDINAL32               Count;                    /* The number of entries in the array of drive control records. */
} Drive_Control_Array;

/* The following structure defines the information that can be changed for a specific disk drive. */
typedef struct _Drive_Information_Record {
CARDINAL32   Total_Available_Sectors;        /* The number of sectors on the disk which are not currently assigned to a partition. */
CARDINAL32   Largest_Free_Block_Of_Sectors;  /* The number of sectors in the largest contiguous block of available sectors.  */
BOOLEAN      Corrupt_Partition_Table;        /* If TRUE, then the partitioning information found on the drive is incorrect! */
BOOLEAN      Unusable;                       /* If TRUE, the drive's MBR is not accessible and the drive can not be partitioned. */
BOOLEAN      IO_Error;                       /* If TRUE, then the last I/O operation on this drive failed! */
BOOLEAN      Is_Big_Floppy;                  /* If TRUE, then the drive is a PRM formatted as a big floppy (i.e. the old style removable media support). */
char         Drive_Name[DISK_NAME_SIZE];     /* User assigned name for this disk drive. */
} Drive_Information_Record;

typedef struct _Partition_Information_Record {
ADDRESS      Partition_Handle;                      /* The handle used to perform operations on this partition. */
ADDRESS      Volume_Handle;                         /* If this partition is part of a volume, this will be the handle of
the volume.  If this partition is NOT part of a volume, then this
handle will be 0.                                                  */
ADDRESS      Drive_Handle;                          /* The handle for the drive this partition resides on. */
DoubleWord   Partition_Serial_Number;               /* The serial number assigned to this partition. */
CARDINAL32   Partition_Start;                       /* The LBA of the first sector of the partition. */
CARDINAL32   True_Partition_Size;                   /* The total number of sectors comprising the partition. */
CARDINAL32   Usable_Partition_Size;                 /* The size of the partition as reported to the IFSM.  This is the
size of the partition less any LVM overhead.                     */
CARDINAL32   Boot_Limit;                            /* The maximum number of sectors from this block of free space that can be used to
create a bootable partition if you allocate from the beginning of the block of
free space.                                                       */
BOOLEAN      Spanned_Volume;                        /* TRUE if this partition is part of a multi-partition volume. */
BOOLEAN      Primary_Partition;                     /* True or False.  Any non-zero value here indicates that
this partition is a primary partition.  Zero here indicates
that this partition is a "logical drive" - i.e. it resides
inside of an extended partition. */
BYTE         Active_Flag;                           /* 80 = Partition is marked as being active.
0 = Partition is not active.                   */
BYTE         OS_Flag;                               /* This field is from the partition table.  It is known as the
OS flag, the Partition Type Field, Filesystem Type, and
various other names.

Values of interest

If this field is: (values are in hex)

07 = The partition is a compatibility partition formatted for use
with an installable filesystem, such as HPFS or JFS.
00 = Unformatted partition
01 = FAT12 filesystem is in use on this partition.
04 = FAT16 filesystem is in use on this partition.
0A = OS/2 Boot Manager Partition
35 = LVM partition
84 = OS/2 FAT16 partition which has been relabeled by Boot Manager to "Hide" it. */
BYTE         Partition_Type;                        /* 0 = Free Space
1 = LVM Partition (Part of an LVM Volume.)
2 = Compatibility Partition
All other values are reserved for future use. */
BYTE         Partition_Status;                      /* 0 = Free Space
1 = In Use - i.e. already assigned to a volume.
2 = Available - i.e. not currently assigned to a volume. */
BOOLEAN      On_Boot_Manager_Menu;                  /* Set to TRUE if this partition is not part of a Volume yet is on the Boot Manager Menu. */
BYTE         Reserved;                              /* Alignment. */
char         Volume_Drive_Letter;                   /* The drive letter assigned to the volume that this partition is a part of. */
char         Drive_Name[DISK_NAME_SIZE];            /* User assigned name for this disk drive. */
char         File_System_Name[FILESYSTEM_NAME_SIZE];/* The name of the filesystem in use on this partition, if it is known. */
char         Partition_Name[PARTITION_NAME_SIZE];   /* The user assigned name for this partition. */
char         Volume_Name[VOLUME_NAME_SIZE];         /* If this partition is part of a volume, then this will be the
name of the volume that this partition is a part of.  If this
record represents free space, then the Volume_Name will be
"FREE SPACE xx", where xx is a unique numeric ID generated by
LVM.DLL.  Otherwise it will be an empty string.                */
} Partition_Information_Record;

/* The following defines are for use with the Partition_Type field in the Partition_Information_Record. */
#define FREE_SPACE_PARTITION     0
#define LVM_PARTITION            1
#define COMPATIBILITY_PARTITION  2

/* The following defines are for use with the Partition_Status field in the Partition_Information_Record. */
#define PARTITION_IS_IN_USE      1
#define PARTITION_IS_AVAILABLE   2
#define PARTITION_IS_FREE_SPACE  0

/* The following structure is returned by various functions in the LVM Engine. */
typedef struct _Partition_Information_Array {
Partition_Information_Record * Partition_Array; /* An array of Partition_Information_Records. */
CARDINAL32                     Count;           /* The number of entries in the Partition_Array. */
} Partition_Information_Array;

/* The following items are invariant for a volume. */
typedef struct _Volume_Control_Record {
DoubleWord Volume_Serial_Number;            /* The serial number assigned to this volume. */
ADDRESS    Volume_Handle;                   /* The handle used to perform operations on this volume. */
BOOLEAN    Compatibility_Volume;            /* TRUE indicates that this volume is compatible with older versions of OS/2.
FALSE indicates that this is an LVM specific volume and can not be used without OS2LVM.DMD. */
BYTE       Device_Type;                     /* Indicates what type of device the Volume resides on:
0 = Hard Drive under LVM Control
1 = PRM under LVM Control
2 = CD-ROM
3 = Network drive
4 = Unknown device NOT under LVM Control
*/
BYTE       Reserved[2];                     /* Alignment. */
} Volume_Control_Record;

/* The following define the device types used in the Device_Type field of the Volume_Control_Record. */
#define LVM_HARD_DRIVE  0
#define LVM_PRM         1
#define NON_LVM_CDROM   2
#define NETWORK_DRIVE   3
#define NON_LVM_DEVICE  4

/* The following structure is returned by the Get_Volume_Control_Data function. */
typedef struct _Volume_Control_Array{
Volume_Control_Record *  Volume_Control_Data;      /* An array of volume control records. */
CARDINAL32               Count;                    /* The number of entries in the array of volume control records. */
} Volume_Control_Array;

/* The following information about a volume can (and often does) vary. */
typedef struct _Volume_Information_Record {
CARDINAL32 Volume_Size;                           /* The number of sectors comprising the volume. */
CARDINAL32 Partition_Count;                       /* The number of partitions which comprise this volume. */
CARDINAL32 Drive_Letter_Conflict;                 /* 0 indicates that the drive letter preference for this volume is unique.
1 indicates that the drive letter preference for this volume
is not unique, but this volume got its preferred drive letter anyway.
2 indicates that the drive letter preference for this volume
is not unique, and this volume did NOT get its preferred drive letter.
4 indicates that this volume is currently "hidden" - i.e. it has
no drive letter preference at the current time.                        */
BOOLEAN    Compatibility_Volume;                  /* TRUE if this is for a compatibility volume, FALSE otherwise. */
BOOLEAN    Bootable;                              /* Set to TRUE if this volume appears on the Boot Manager menu, or if it is
a compatibility volume and its corresponding partition is the first active
primary partition on the first drive.                                         */
char       Drive_Letter_Preference;               /* The drive letter that this volume desires to be. */
char       Current_Drive_Letter;                  /* The drive letter currently used to access this volume.  May be different than
Drive_Letter_Preference if there was a conflict ( i.e. Drive_Letter_Preference
is already in use by another volume ).                                          */
char       Initial_Drive_Letter;                  /* The drive letter assigned to this volume by the operating system when LVM was started.
This may be different from the Drive_Letter_Preference if there were conflicts, and
may be different from the Current_Drive_Letter.  This will be 0x0 if the Volume did
not exist when the LVM Engine was opened (i.e. it was created during this LVM session). */
BOOLEAN    New_Volume;                            /* Set to FALSE if this volume existed before the LVM Engine was opened.  Set to
TRUE if this volume was created after the LVM Engine was opened.                */
BYTE       Status;                                /* 0 = None.
1 = Bootable
2 = Startable
3 = Installable.           */
BYTE       Reserved_1;
char       Volume_Name[VOLUME_NAME_SIZE];         /* The user assigned name for this volume. */
char       File_System_Name[FILESYSTEM_NAME_SIZE];/* The name of the filesystem in use on this partition, if it is known. */
} Volume_Information_Record;

/* The following structure is used to return the feature information for the installed features, or the features on a volume. */
typedef struct _Feature_Information_Array {
CARDINAL32        Count;
Feature_ID_Data * Feature_Data;
} Feature_Information_Array;

/* The following structure defines an item on the Boot Manager Menu. */
typedef struct _Boot_Manager_Menu_Item {
ADDRESS     Handle;            /* A Volume or Partition handle. */
BOOLEAN     Volume;            /* If TRUE, then Handle is the handle of a Volume.  Otherwise, Handle is the handle of a partition. */
} Boot_Manager_Menu_Item;

/* The following structure is used to get a list of the items on the partition manager menu. */
typedef struct _Boot_Manager_Menu {
Boot_Manager_Menu_Item *  Menu_Items;
CARDINAL32                Count;
} Boot_Manager_Menu;

/* The following structure is used to specify an LVM Feature when creating a volume.  Since LVM Features may be part of
more than one LVM Class, the specific class to be used with the feature must also be specified.                        */
typedef struct _LVM_Feature_Specification_Record {
CARDINAL32       Feature_ID;     /* The feature ID of the feature to use. */
LVM_Classes      Actual_Class;   /* The LVM Class (supported by the specified feature) to use. */
ADDRESS          Init_Data;      /* The address of a buffer containing initialization data for this feature.
NULL if there is no initialization data being provided for this feature. */
} LVM_Feature_Specification_Record;

/* The following structure is used with the Get_Child_Handles function. */
/*
typedef struct {
CARDINAL32   Count;
ADDRESS *    Handles;
} LVM_Handle_Array_Record;
*/

/* The following preprocessor directives define the operations that can be performed on a partition, volume, or a block of free space.
These definitions represent bits in a 32 bit value returned by the Get_Valid_Options function.                                         */

#define CREATE_PRIMARY_PARTITION           1

#define CREATE_LOGICAL_DRIVE               2

#define DELETE_PARTITION                   4

#define SET_ACTIVE_PRIMARY                 8

#define SET_PARTITION_ACTIVE              0x10

#define SET_PARTITION_INACTIVE            0x20

#define SET_STARTABLE                     0x40

#define INSTALL_BOOT_MANAGER              0x80

#define REMOVE_BOOT_MANAGER               0x100

#define SET_BOOT_MANAGER_DEFAULTS         0x200

#define ADD_TO_BOOT_MANAGER_MENU          0x400

#define REMOVE_FROM_BOOT_MANAGER_MENU     0x800

#define DELETE_VOLUME                     0x1000

#define HIDE_VOLUME                       0x2000

#define EXPAND_VOLUME                     0x4000

#define SET_VOLUME_INSTALLABLE            0x8000

#define ASSIGN_DRIVE_LETTER               0x10000

#define CAN_BOOT_PRIMARY                  0x20000      /* If a primary is created from this block of free space, then it can be made bootable. */

#define CAN_BOOT_LOGICAL                  0x40000      /* If a logical drive is created from this block of free space, then OS/2 can boot from it by adding it to the boot manager menu. */

#define CAN_SET_NAME                      0x80000

#define SET_BOOT_MANAGER_STARTABLE        0x100000

/* The following enumeration defines the allocation strategies used by the Create_Partition function. */
typedef enum _Allocation_Algorithm {
Automatic,               /* Let LVM decide which block of free space to use to create the partition. */
Best_Fit,                /* Use the block of free space which is closest in size to the partition being created. */
First_Fit,               /* Use the first block of free space on the disk which is large enough to hold a partition of the specified size. */
Last_Fit,                /* Use the last block of free space on the disk which is large enough to hold a partition of the specified size. */
From_Largest,            /* Find the largest block of free space and allocate the partition from that block of free space. */
From_Smallest,           /* Find the smallest block of free space that can accommodate a partition of the size specified. */
All                      /* Turn the specified drive or block of free space into a single partition. */
} Allocation_Algorithm;

/* Error codes returned by the LVM Engine. */
#define LVM_ENGINE_NO_ERROR                            0
#define LVM_ENGINE_OUT_OF_MEMORY                       1
#define LVM_ENGINE_IO_ERROR                            2
#define LVM_ENGINE_BAD_HANDLE                          3
#define LVM_ENGINE_INTERNAL_ERROR                      4
#define LVM_ENGINE_ALREADY_OPEN                        5
#define LVM_ENGINE_NOT_OPEN                            6
#define LVM_ENGINE_NAME_TOO_BIG                        7
#define LVM_ENGINE_OPERATION_NOT_ALLOWED               8
#define LVM_ENGINE_DRIVE_OPEN_FAILURE                  9
#define LVM_ENGINE_BAD_PARTITION                      10
#define LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION     11
#define LVM_ENGINE_TOO_MANY_PRIMARY_PARTITIONS        12
#define LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE         13
#define LVM_ENGINE_REQUESTED_SIZE_TOO_BIG             14
#define LVM_ENGINE_1024_CYLINDER_LIMIT                15
#define LVM_ENGINE_PARTITION_ALIGNMENT_ERROR          16
#define LVM_ENGINE_REQUESTED_SIZE_TOO_SMALL           17
#define LVM_ENGINE_NOT_ENOUGH_FREE_SPACE              18
#define LVM_ENGINE_BAD_ALLOCATION_ALGORITHM           19
#define LVM_ENGINE_DUPLICATE_NAME                     20
#define LVM_ENGINE_BAD_NAME                           21
#define LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE        22
#define LVM_ENGINE_NO_DRIVES_FOUND                    23
#define LVM_ENGINE_WRONG_VOLUME_TYPE                  24
#define LVM_ENGINE_VOLUME_TOO_SMALL                   25
#define LVM_ENGINE_BOOT_MANAGER_ALREADY_INSTALLED     26
#define LVM_ENGINE_BOOT_MANAGER_NOT_FOUND             27
#define LVM_ENGINE_INVALID_PARAMETER                  28
#define LVM_ENGINE_BAD_FEATURE_SET                    29
#define LVM_ENGINE_TOO_MANY_PARTITIONS_SPECIFIED      30
#define LVM_ENGINE_LVM_PARTITIONS_NOT_BOOTABLE        31
#define LVM_ENGINE_PARTITION_ALREADY_IN_USE           32
#define LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE    33
#define LVM_ENGINE_VOLUME_NOT_FOUND                   34
#define LVM_ENGINE_DRIVE_NOT_FOUND                    35
#define LVM_ENGINE_PARTITION_NOT_FOUND                36
#define LVM_ENGINE_TOO_MANY_FEATURES_ACTIVE           37
#define LVM_ENGINE_PARTITION_TOO_SMALL                38
#define LVM_ENGINE_MAX_PARTITIONS_ALREADY_IN_USE      39
#define LVM_ENGINE_IO_REQUEST_OUT_OF_RANGE            40
#define LVM_ENGINE_SPECIFIED_PARTITION_NOT_STARTABLE  41
#define LVM_ENGINE_SELECTED_VOLUME_NOT_STARTABLE      42
#define LVM_ENGINE_EXTENDFS_FAILED                    43
#define LVM_ENGINE_REBOOT_REQUIRED                    44
#define LVM_ENGINE_CAN_NOT_OPEN_LOG_FILE              45
#define LVM_ENGINE_CAN_NOT_WRITE_TO_LOG_FILE          46
#define LVM_ENGINE_REDISCOVER_FAILED                  47

/* The following errors are specific to LVM Version 2 and are not used under OS/2 */
/*
#define LVM_ENGINE_INTERNAL_VERSION_FAILURE           48
#define LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE        49
#define LVM_ENGINE_BAD_FEATURE_ID                     50
#define LVM_ENGINE_NO_INIT_DATA                       51
#define LVM_ENGINE_NO_CONTEXT_DATA                    52
#define LVM_ENGINE_WRONG_CLASS_FOR_FEATURE            53
#define LVM_ENGINE_INCOMPATIBLE_FEATURES_SELECTED     54
#define LVM_ENGINE_NO_CHILDREN                        55
#define LVM_ENGINE_FEATURE_NOT_SUPPORTED_BY_INTERFACE 56
#define LVM_ENGINE_NO_PARENT                          57
#define LVM_ENGINE_VOLUME_HAS_NOT_BEEN_COMMITTED_YET  58
#define LVM_ENGINE_UNABLE_TO_REFERENCE_VOLUME         59
#define LVM_ENGINE_PARSING_ERROR                      60
#define LVM_ENGINE_INTERNAL_FEATURE_ERROR             61
#define LVM_ENGINE_VOLUME_NOT_CONVERTED               62
*/

/* Function Prototypes */

#ifndef USE_16_BIT_FUNCTIONS

/* ************************************************************************** *

Functions relating to the LVM Engine itself

************************************************************************** */

/****************************************************************************************************/
/*                                                                                                  */
/*   Function Name: Open_LVM_Engine                                                                 */
/*                                                                                                  */
/*   Descriptive Name: Opens the LVM Engine and readies it for use.                                 */
/*                                                                                                  */
/*   Input: BOOLEAN Ignore_CHS : If TRUE, then the LVM engine will not check the CHS values in the  */
/*                               MBR/EBR partition tables for validity.  This is useful if there    */
/*                               are drive geometry problems, such as the drive was partitioned and */
/*                               formatted with one geometry and then moved to a different machine  */
/*                               which uses a different geometry for the drive.  This would cause   */
/*                               the starting and ending CHS values in the partition tables to      */
/*                               be inconsistent with the size and partition offset entries in the  */
/*                               partition tables.  Setting Ignore_CHS to TRUE will disable the     */
/*                               LVM Engine's CHS consistency checks, thereby allowing the drive    */
/*                               to be partitioned.                                                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in which to store an error code   */
/*                                    should an error occur.                                        */
/*                                                                                                  */
/*   Output:  *Error_Code will be 0 if this function completes successfully.  If an error occurs,   */
/*            *Error_Code will contain a non-zero error code.                                       */
/*                                                                                                  */
/*   Error Handling: If this function aborts with an error, all memory allocated during the course  */
/*                   of this function will be released.  Disk read errors will be reported to the   */
/*                   user via pop-up error messages.  Disk read errors will only cause this         */
/*                   function to abort if none of the disk drives in the system could be            */
/*                   successfully read.                                                             */
/*                                                                                                  */
/*   Side Effects:  The LVM Engine will be initialized.  The partition tables for all OS2DASD       */
/*                  controlled disk drives will be read into memory.  Memory will be allocated for  */
/*                  the data structures used by the LVM Engine.                                     */
/*                                                                                                  */
/*   Notes:  This is provided for programs that used LVM Version 1.  This function assumes an       */
/*           LVM_Interface_Type of VIO_Interface.					                                */
/*                                                                                                  */
/****************************************************************************************************/
void _System Open_LVM_Engine( BOOLEAN Ignore_CHS, CARDINAL32 * Error_Code );

/****************************************************************************************************/
/*                                                                                                  */
/*   Function Name: Open_LVM_Engine2                                                                */
/*                                                                                                  */
/*   Descriptive Name: Opens the LVM Engine and readies it for use.                                 */
/*                                                                                                  */
/*   Input: BOOLEAN Ignore_CHS : If TRUE, then the LVM engine will not check the CHS values in the  */
/*                               MBR/EBR partition tables for validity.  This is useful if there    */
/*                               are drive geometry problems, such as the drive was partitioned and */
/*                               formatted with one geometry and then moved to a different machine  */
/*                               which uses a different geometry for the drive.  This would cause   */
/*                               the starting and ending CHS values in the partition tables to      */
/*                               be inconsistent with the size and partition offset entries in the  */
/*                               partition tables.  Setting Ignore_CHS to TRUE will disable the     */
/*                               LVM Engine's CHS consistency checks, thereby allowing the drive    */
/*                               to be partitioned.                                                 */
/*          LVM_Interface_Types Interface_Type - Indicate the type of user interface being used:    */
/*                               PM_Interface, VIO_Interface, or Java_Interface.  This lets the     */
/*                               LVM Engine know which interface support routines to call in any    */
/*                               plugin modules which may be loaded.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in which to store an error code   */
/*                                    should an error occur.                                        */
/*                                                                                                  */
/*   Output:  *Error_Code will be 0 if this function completes successfully.  If an error occurs,   */
/*            *Error_Code will contain a non-zero error code.                                       */
/*                                                                                                  */
/*   Error Handling: If this function aborts with an error, all memory allocated during the course  */
/*                   of this function will be released.  Disk read errors will be reported to the   */
/*                   user via pop-up error messages.  Disk read errors will only cause this         */
/*                   function to abort if none of the disk drives in the system could be            */
/*                   successfully read.                                                             */
/*                                                                                                  */
/*   Side Effects:  The LVM Engine will be initialized.  The partition tables for all OS2DASD       */
/*                  controlled disk drives will be read into memory.  Memory will be allocated for  */
/*                  the data structures used by the LVM Engine.                                     */
/*                                                                                                  */
/*   Notes:  This function is only available in LVM Version 2, and is not available in the current  */
/*           OS/2 implementation.                                                                   */
/*                                                                                                  */
/****************************************************************************************************/
/*
 * void _System Open_LVM_Engine2( BOOLEAN Ignore_CHS, LVM_Interface_Types Interface_Type, CARDINAL32 * Error_Code );
 */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Commit_Changes                                   */
/*                                                                   */
/*   Descriptive Name: Saves any changes made to the partitioning    */
/*                     information of the OS2DASD controlled disk    */
/*                     drives in the system.                         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  The function return value will be TRUE if all of the   */
/*            partitioning/volume changes made were successfully     */
/*            written to disk.  Also, *Error_Code will be 0 if no    */
/*            errors occur.                                          */
/*                                                                   */
/*            If an error occurs, then the furnction return value    */
/*            will be FALSE and *Error_Code will contain a non-zero  */
/*            error code.                                            */
/*                                                                   */
/*   Error Handling:  If an error occurs, the function return value  */
/*                    will be false and *Error_Code will be > 0.     */
/*                                                                   */
/*                    Disk read and write errors will be indicated by*/
/*                    setting the IO_Error field of the              */
/*                    Drive_Information_Record to TRUE.  Thus, if    */
/*                    the function return value is FALSE, and        */
/*                    *Error_Code indicates an I/O error, the caller */
/*                    of this function should call the               */
/*                    Get_Drive_Status function on each drive to     */
/*                    determine which drives had I/O errors.         */
/*                                                                   */
/*                    If a read or write error occurs, then the      */
/*                    engine may not have been able to create a      */
/*                    partition or volume.  Thus, the caller         */
/*                    may want to refresh all partition and volume   */
/*                    data to see what the engine was and was not    */
/*                    able to create.                                */
/*                                                                   */
/*   Side Effects:  The partitioning information of the disk drives  */
/*                  in the system may be altered.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Commit_Changes( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Java_Call_Back                               */
/*                                                                   */
/*   Descriptive Name: This function allows the calling Java program */
/*                     to set the call back address.  The call back  */
/*                     address is used when the LVM Engine or one of */
/*                     its plug-ins, needs to run a Java class to    */
/*                     gather information from the user.             */
/*                                                                   */
/*   Input: void (* Execute_Java_Class) ... - The address of a       */
/*                                            function that the LVM  */
/*                                            engine may call when   */
/*                                            it needs a Java class  */
/*                                            to be executed.  This  */
/*                                            is only required if the*/
/*                                            user interface being   */
/*                                            used is written in Java*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If the function completes successfully, then *Error_Code*/
/*           will be set to LVM_ENGINE_NO_ERROR.  Otherwise,         */
/*           *Error_Code will be set to a non-zero error code.       */
/*                                                                   */
/*   Error Handling:  If an error occurs, the function will abort and*/
/*                    *Error_Code will be set to a non-zero error    */
/*                    code.                                          */
/*                                                                   */
/*   Side Effects:  The Java call back address is set to point to the*/
/*                  specified function.  Once the Java call back     */
/*                  address is set, LVM plug-ins which require the   */
/*                  Java call back will be enabled and can be used   */
/*                  during the creation of LVM Volumes.              */
/*                                                                   */
/*   Notes:  If a Java interface is in use (as specified on the      */
/*           Open_LVM_Engine call), then this function must be called*/
/*           in order to enable those LVM plug-ins which require     */
/*           initialization information during the creation of an    */
/*           LVM Volume.  If these plug-ins are not enabled, then    */
/*           they will not be reported by the Get_Available_Features */
/*           API, nor can they be used or accessed by any other LVM  */
/*           Engine APIs.  Thus, this function should be called      */
/*           immediately after the Open_LVM_Engine API is called.    */
/*                                                                   */
/*           This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* void _System Set_Java_Call_Back( void (* _System Execute_Java_Class) ( char *       Class_Name,
* ADDRESS      InputBuffer,
* CARDINAL32   InputBufferSize,
* ADDRESS    * OutputBuffer,
* CARDINAL32 * OutputBufferSize,
* CARDINAL32 * Error_Code),
* CARDINAL32 * Error_Code );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Close_LVM_Engine                                 */
/*                                                                   */
/*   Descriptive Name: Closes the LVM Engine and frees any memory    */
/*                     held by the LVM Engine.                       */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects:  Any memory held by the LVM Engine is released.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Close_LVM_Engine ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Available_Features                           */
/*                                                                   */
/*   Descriptive Name: Returns the feature ID information for each of*/
/*                     the features that the LVM Engine knows about. */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If successful, a Feature_Information_Array structure is */
/*           returned with a non-zero Count.  Also, *Error_Code will */
/*           be set to LVM_ENGINE_NO_ERROR.  If an error occurs,     */
/*           then the Count field in the structure will be 0 and     */
/*           (*Error_Code) will contain a non-zero error code.       */
/*                                                                   */
/*   Error Handling: The only expected error is if this function is  */
/*                   called while the LVM Engine is not open.  This  */
/*                   should be the only error condition.             */
/*                                                                   */
/*   Side Effects:  Memory is allocated using the LVM Engine's memory*/
/*                  manager for the array of Feature_ID_Data items   */
/*                  being returned.                                  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Feature_Information_Array _System Get_Available_Features( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Issue_Feature_Command                            */
/*                                                                   */
/*   Descriptive Name: Issues a feature specific command to either   */
/*                     the Ring 0 or Ring 3 portion of the feature.  */
/*                                                                   */
/*   Input: CARDINAL32 Feature_ID - The numeric ID assigned to the   */
/*                                  feature which is to receive the  */
/*                                  command being issued.            */
/*          ADDRESS Handle - The handle of the volume, partition, or */
/*                           aggregate to which the feature command  */
/*                           is to be directed.                      */
/*          BOOLEAN Ring0 - If TRUE, then the command will be sent   */
/*                          to the Ring 0 portion of the feature.    */
/*                          If FALSE, then the command will be sent  */
/*                          to the Ring 3 portion of the feature.    */
/*          ADDRESS InputBuffer - A buffer containing the command and*/
/*                                any necessary information for the  */
/*                                feature to process the command.    */
/*          CARDINAL32 InputSize - The number of bytes in the        */
/*                                 InputBuffer.                      */
/*          ADDRESS * OutputBuffer - The address of a variable used  */
/*                                   to hold the location of the     */
/*                                   output buffer created by the    */
/*                                   feature in response to the      */
/*                                   command in InputBuffer.         */
/*          CARDINAL32 * OutputSize - The address of a variable used */
/*                                    to hold the number of bytes in */
/*                                    *OutputBuffer.                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be set to          */
/*           LVM_ENGINE_NO_ERROR.  If unsuccessful, then *Error_Code */
/*           will be set to a non-zero error code.  *OutputBuffer and*/
/*           *OutputSize are set by the feature.  If the feature     */
/*           specified does not exist, then *OutputBuffer will be    */
/*           NULL and *Outputsize will be 0.  If the feature does    */
/*           exist, then the value of *OutputBuffer and *OutputSize  */
/*           depend upon the feature.                                */
/*                                                                   */
/*   Error Handling: If the specified feature does not exist, then   */
/*                   *Error_Code will contain a non-zero error code, */
/*                   *OutputBuffer will be NULL, and *OutputSize will*/
/*                   be set to 0.  All other error conditions are    */
/*                   feature dependent.                              */
/*                                                                   */
/*   Side Effects:  Side effects are feature dependent.              */
/*                                                                   */
/*   Notes:  This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/* void _System Issue_Feature_Command( CARDINAL32 Feature_ID,
* ADDRESS Handle,
* BOOLEAN Ring0,
* ADDRESS InputBuffer,
* CARDINAL32 InputSize,
* ADDRESS * OutputBuffer,
* CARDINAL32 * OutputSize,
* CARDINAL32 * Error_Code );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Parse_Feature_Parameters                         */
/*                                                                   */
/*   Descriptive Name: This function allows access to the parsing    */
/*                     function of an LVM Plug-in Feature.  The      */
/*                     specified feature will be passed a list of    */
/*                     tokens to parse, and, if it parses the tokens */
/*                     successfully, will produce a buffer containing*/
/*                     initialization data that can be used with the */
/*                     Create_Volume API.  If it encounters an error,*/
/*                     the current item in the Tokens list will be   */
/*                     the offending item, and a pointer to an error */
/*                     message will be returned.                     */
/*                                                                   */
/*   Input: DLIST  Tokens - A DLIST of tokens to parse.  Parsing will*/
/*                          begin with the current entry in the list */
/*                          and proceed until there is an error or   */
/*                          until the specified feature has found a  */
/*                          complete command.  Each feature defines  */
/*                          what commands it will accept.            */
/*          LVM_Feature_Specification_Record * Feature_Data - A      */
/*                          pointer to a record which contains the   */
/*                          ID of the feature which is to parse the  */
/*                          DLIST of Tokens.  The other fields in    */
/*                          this record will be filled in by the     */
/*                          feature if it successfully parses the    */
/*                          tokens.                                  */
/*          char ** Error_Message - The address of a pointer to char.*/
/*                          This will be set to NULL if the feature  */
/*                          successfully parses the list of tokens,  */
/*                          or it will be set to point to an error   */
/*                          message if an error occurs.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32    */
/*                          variable into which an error code may    */
/*                          be placed.  The error code will be       */
/*                          LVM_ENGINE_NO_ERROR if this function     */
/*                          completes successfully, or a non-zero    */
/*                          error code if an error occurs.           */
/*                                                                   */
/*   Output: If there are no errors, the Actual_Class and Init_Data  */
/*           fields of *Feature_Data will be set, *Error_Message will*/
/*           be set to NULL, and Error_Code will be set to           */
/*           LVM_ENGINE_NO_ERROR.  The current item in the Tokens    */
/*           list will be the first token that was not parsed by the */
/*           feature (i.e. the first token after the command accepted*/
/*           by the plug-in).                                        */
/*                                                                   */
/*           If an error occurs, the values of Actual_Class and      */
/*           Init_Data in *Feature_Data are undefined.  *Error_Code  */
/*           will be set to LVM_ENGINE_PARSING_ERROR if the error is */
/*           parsing related, or some other non-zero value if the    */
/*           error is not parsing related.  If the error is parsing  */
/*           related, then *Error_Message will point to an error     */
/*           message which can be displayed for the user.  The       */
/*           current item in the Tokens list will be the token which */
/*           failed.                                                 */
/*                                                                   */
/*   Error Handling: If an parsing related error occurs, i.e. the    */
/*                   tokens in the Tokens list do not form a valid   */
/*                   command accepted by the specified feature, then */
/*                   the current item in the Tokens list will be the */
/*                   offending token, *Error_Message will be set to  */
/*                   point to an error message, and *Error_Code will */
/*                   be set to LVM_ENGINE_PARSING_ERROR.             */
/*                   If any other error occurs, the current item in  */
/*                   the Tokens list will be the token that was being*/
/*                   processed when the error occurs, and *Error_Code*/
/*                   will be set to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  The current item in the Tokens list may change.  */
/*                                                                   */
/*   Notes:  Each feature defines which commands it will accept, and */
/*           therefore which commands it will successfully parse.    */
/*                                                                   */
/*           This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* void _System Parse_Feature_Parameters( DLIST                               Tokens,
* LVM_Feature_Specification_Record  * Feature_Data,
* char **                             Error_Message,
* CARDINAL32 *                        Error_Code);
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Refresh_LVM_Engine                               */
/*                                                                   */
/*   Descriptive Name: This function causes the LVM Engine to look   */
/*                     for changes in the current system             */
/*                     configuration and update its internal tables  */
/*                     accordingly.                                  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be set to  */
/*                   a non-zero value.                               */
/*                                                                   */
/*   Side Effects: Volumes which represent non-LVM devices may have  */
/*                 their handles changed!                            */
/*                                                                   */
/*   Notes:  After calling this function, Get_Volume_Control_Data    */
/*           should be called to get the updated list of volumes.    */
/*           This is necessary as the handles of some volumes may    */
/*           have changed.                                           */
/*                                                                   */
/*********************************************************************/
void _System  Refresh_LVM_Engine( CARDINAL32 * Error_Code );

/* ************************************************************************** *

Functions relating to Drives

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Get_Drive_Control_Data                          */
/*                                                                   */
/*   Descriptive Name:  This function returns an array of            */
/*                      Drive_Control_Records.  These records provide*/
/*                      important information about the drives in the*/
/*                      system and provide the handles required to   */
/*                      access them.                                 */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  A Drive_Control_Array structure is returned.  If no    */
/*            errors occur, Drive_Control_Data will be non-NULL,     */
/*            Count will be greater than zero, and *Error_Code will  */
/*            be 0.                                                  */
/*                                                                   */
/*   Error Handling:  If an error occurs, the Drive_Control_Array    */
/*                    returned by this function will have NULL for   */
/*                    Drive_Control_Data, and 0 for Count.           */
/*                    *Error_Code will be greater than 0.            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Drive_Control_Records pointed to by    */
/*           Drive_Control_Data pointer in the Drive_Control_Array   */
/*           structure returned by this function.  The caller should */
/*           free this memory when they are done using it.           */
/*                                                                   */
/*********************************************************************/
Drive_Control_Array _System Get_Drive_Control_Data( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Get_Drive_Status                                */
/*                                                                   */
/*   Descriptive Name:  Returns the Drive_Information_Record for the */
/*                      drive specified by Drive_Handle.             */
/*                                                                   */
/*   Input: ADDRESS Drive_Handle - The handle of the drive to use.   */
/*                             Drive handles are obtained through the*/
/*                             Get_Drive_Control_Data function.      */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns the Drive_Information_Record for  */
/*           the drive associated with the specified Drive_Handle.   */
/*           If no errors occur, *Error_Code will be set to 0.  If an*/
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling:  If an error occurs, then *Error_Code will be   */
/*                    non-zero.                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Drive_Information_Record _System Get_Drive_Status( ADDRESS Drive_Handle, CARDINAL32 * Error_Code );

/* ************************************************************************** *

Functions relating to Partitions

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partitions                                   */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the object specified by Handle.          */
/*                                                                   */
/*   Input:ADDRESS Handle - This is the handle of a drive or volume. */
/*                      Drive handles are obtained through the       */
/*                      Get_Drive_Control_Data function.  Volume     */
/*                      handles are obtained through the             */
/*                      Get_Volume_Control_Data function.            */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  If     */
/*           Handle is the handle of a disk drive, then the returned */
/*           array will contain a partition information record for   */
/*           each partition and block of free space on that drive.   */
/*           If Handle is the handle of a volume, then the returned  */
/*           array will contain a partition information record for   */
/*           each partition which is part of the specified volume.   */
/*           If no errors occur, then *Error_Code will be 0.  If an  */
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling: Any memory allocated for the return value will  */
/*                   be freed.  The Partition_Information_Array      */
/*                   returned by this function will contain a NULL   */
/*                   pointer for Partition_Array, and have a Count of*/
/*                   0.  *Error_Code will be non-zero.               */
/*                                                                   */
/*                   If Handle is non-NULL and is invalid, a trap    */
/*                   is likely.                                      */
/*                                                                   */
/*   Side Effects:  Memory will be allocated to hold the array       */
/*                  returned by this function.                       */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Partition_Information_Records pointed  */
/*           to by Partition_Array pointer in the                    */
/*           Partition_Information_Array structure returned by this  */
/*           function.  The caller should free this memory when they */
/*           are done using it.                                      */
/*                                                                   */
/*********************************************************************/
Partition_Information_Array _System Get_Partitions( ADDRESS Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partition_Handle                             */
/*                                                                   */
/*   Descriptive Name: Returns the handle of the partition whose     */
/*                     serial number matches the one provided.       */
/*                                                                   */
/*   Input: CARDINAL32 Serial_Number - This is the serial number to  */
/*                                     look for.  If a partition with*/
/*                                     a matching serial number is   */
/*                                     found, its handle will be     */
/*                                     returned.                     */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If a partition with a matching serial number is found,  */
/*              then the function return value will be the handle    */
/*              of the partition found.  If no matching partition is */
/*              found, then the function return value will be NULL.  */
/*                                                                   */
/*   Error Handling:  If no errors occur, *Error_Code will be        */
/*                    LVM_ENGINE_NO_ERROR.  If an error occurs, then */
/*                    *Error_Code will be a non-zero error code.     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Get_Partition_Handle( CARDINAL32 Serial_Number, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partition_Information                        */
/*                                                                   */
/*   Descriptive Name: Returns the Partition_Information_Record for  */
/*                     the partition specified by Partition_Handle.  */
/*                                                                   */
/*   Input: ADDRESS Partition_Handle - The handle associated with the*/
/*                                     partition for which the       */
/*                                     Partition_Information_Record  */
/*                                     is desired.                   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A Partition_Information_Record is returned.  If there   */
/*           is no error, then *Error_Code will be 0.  If an error   */
/*           occurs, *Error_Code will be non-zero.                   */
/*                                                                   */
/*   Error Handling:  If the Partition_Handle is not a valid handle, */
/*                    a trap could result.  If it is a handle for    */
/*                    something other than a partition, an error code*/
/*                    will be returned in *Error_Code.               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Partition_Information_Record  _System Get_Partition_Information( ADDRESS Partition_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Partition                                 */
/*                                                                   */
/*   Descriptive Name: Creates a partition on a disk drive.          */
/*                                                                   */
/*   Input: ADDRESS         Handle - The handle of a disk drive or   */
/*                                   a block of free space.          */
/*          CARDINAL32      Size - The size, in sectors, of the      */
/*                                 partition to create.              */
/*          char            Name[] - The name to give to the newly   */
/*                                   created partition.              */
/*          Allocation_Algorithm algorithm - If Handle is a drive,   */
/*                                           then the engine will    */
/*                                           find a block of free    */
/*                                           space to use to create  */
/*                                           the partition.  This    */
/*                                           tells the engine which  */
/*                                           memory management       */
/*                                           algorithm to use.       */
/*          BOOLEAN         Bootable - If TRUE, then the engine will */
/*                                     only create the partition if  */
/*                                     it can be booted from.  If    */
/*                                     Primary_Partition is FALSE,   */
/*                                     then it is assumed that OS/2  */
/*                                     is the operating system that  */
/*                                     will be booted.               */
/*          BOOLEAN         Primary_Partition - If TRUE, then the    */
/*                                              engine will create   */
/*                                              a primary partition. */
/*                                              If FALSE, then the   */
/*                                              engine will create a */
/*                                              logical drive.       */
/*          BOOLEAN         Allocate_From_Start - If TRUE, then the  */
/*                                                engine will        */
/*                                                allocate the new   */
/*                                                partition from the */
/*                                                beginning of the   */
/*                                                selected block of  */
/*                                                free space.  If    */
/*                                                FALSE, then the    */
/*                                                partition will be  */
/*                                                allocated from the */
/*                                                end of the selected*/
/*                                                block of free      */
/*                                                space.             */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function return value will be the handle of the     */
/*           partition created.  If the partition could not be       */
/*           created, then NULL will be returned.                    */
/*           *Error_Code will be 0 if the partition was created.     */
/*           *Error_Code will be > 0 if the partition could not be   */
/*           created.                                                */
/*                                                                   */
/*   Error Handling:  If the partition can not be created, then any  */
/*                    memory allocated by this function will be      */
/*                    freed and the partitioning of the disk in      */
/*                    question will be unchanged.                    */
/*                                                                   */
/*                    If Handle is not a valid handle, then a trap   */
/*                    may result.                                    */
/*                                                                   */
/*                    If Handle represents a partition or volume,    */
/*                    then the function will abort and set           */
/*                    *Error_Code to a non-zero value.               */
/*                                                                   */
/*   Side Effects:  A partition may be created on a disk drive.      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Create_Partition( ADDRESS               Handle,
CARDINAL32            Size,
char                  Name[ PARTITION_NAME_SIZE ],
Allocation_Algorithm  algorithm,
BOOLEAN               Bootable,
BOOLEAN               Primary_Partition,
BOOLEAN               Allocate_From_Start,
CARDINAL32 *          Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Delete_Partition                                 */
/*                                                                   */
/*   Descriptive Name: Deletes the partition specified by            */
/*                     Partition_Handle.                             */
/*                                                                   */
/*   Input: ADDRESS Partition_Handle - The handle associated with the*/
/*                                     partition to be deleted.      */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition was deleted      */
/*           successfully.  *Error_Code will be > 0 if the partition */
/*           could not be deleted.                                   */
/*                                                                   */
/*   Error Handling: If the partition can not be deleted, then       */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects: A partition on a disk drive may be deleted.       */
/*                                                                   */
/*   Notes:  A partition can not be deleted if it is part of a       */
/*           volume!                                                 */
/*                                                                   */
/*********************************************************************/
void _System Delete_Partition( ADDRESS Partition_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Active_Flag                                  */
/*                                                                   */
/*   Descriptive Name: Sets the Active Flag field for a partition.   */
/*                                                                   */
/*   Input: ADDRESS Partition_Handle - The handle of the partition   */
/*                                     whose Active Flag is to be set*/
/*          BYTE Active_Flag - The new value for the Active Flag.    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the Active Flag was            */
/*           successfully set, otherwise *Error_Code will contain a  */
/*           non-zero error code indicating what went wrong.         */
/*                                                                   */
/*   Error Handling: If the Active Flag can not be set, this function*/
/*                   will abort without changing any disk structures.*/
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                                                                   */
/*   Side Effects:  The Active Flag for a partition may be modified. */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Active_Flag ( ADDRESS      Partition_Handle,
BYTE         Active_Flag,
CARDINAL32 * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_OS_Flag                                      */
/*                                                                   */
/*   Descriptive Name: Sets the OS Flag field for a partition.  This */
/*                     field is typically used to indicate the       */
/*                     filesystem used on the partition, which       */
/*                     generally gives an indication of which OS is  */
/*                     using that partition.                         */
/*                                                                   */
/*   Input: ADDRESS Partition_Handle - The handle of the partition   */
/*                                     whose Active Flag is to be set*/
/*          BYTE OS_Flag - The new value for the OS Flag.            */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the OS Flag was successfully   */
/*           set, otherwise *Error_Code will contain a non-zero error*/
/*           code indicating what went wrong.                        */
/*                                                                   */
/*   Error Handling: If the OS Flag can not be set, this function    */
/*                   will abort without changing any disk structures.*/
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                                                                   */
/*   Side Effects:  The OS Flag for a partition may be modified.     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_OS_Flag ( ADDRESS      Partition_Handle,
BYTE         OS_Flag,
CARDINAL32 * Error_Code
);

/* ************************************************************************** *

Functions relating to Volumes

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Volume_Control_Data                          */
/*                                                                   */
/*   Descriptive Name: This function returns a structure containing  */
/*                     an array of Volume_Control_Records.  These    */
/*                     records contain information about volumes     */
/*                     which is invariant - i.e. will not change for */
/*                     as long as the volume exists.  One of the     */
/*                     items in the Volume_Control_Record is the     */
/*                     handle for the volume.  This handle must be   */
/*                     used on all accesses to the volume.           */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A Volume_Control_Array structure is returned.           */
/*                                                                   */
/*           If there are no errors, then the Volume_Control_Data    */
/*           pointer in the Volume_Control_Array will be non-NULL,   */
/*           the Count field of the Volume_Control_Array will be     */
/*           >= 0, and *Error_Code will be 0.                        */
/*                                                                   */
/*           If an error does occur, then the Volume_Control_Data    */
/*           pointer in the the Volume_Control_Array will be NULL,   */
/*           the Count field of the Volume_Control_Array will be 0,  */
/*           and *Error_Code will be > 0.                            */
/*                                                                   */
/*   Error Handling: If an error occurs, then any memory allocated by*/
/*                   this function will be freed.                    */
/*                                                                   */
/*   Side Effects:  Memory for the returned array is allocated.      */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Volume_Control_Records pointed to by   */
/*           Volume_Control_Data pointer in the Volume_Control_Array */
/*           structure returned by this function.  The caller should */
/*           free this memory when they are done using it.           */
/*                                                                   */
/*********************************************************************/
Volume_Control_Array _System Get_Volume_Control_Data( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Volume_Information                           */
/*                                                                   */
/*   Descriptive Name:  This function returns the                    */
/*                      Volume_Information_Record for the volume     */
/*                      associated with Volume_Handle.               */
/*                                                                   */
/*   Input: ADDRESS Volume_Handle - The handle of the volume about   */
/*                                  which information is desired.    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a Volume_Information_Record.      */
/*                                                                   */
/*           If this function is successful, then *Error_Code will be*/
/*              0.                                                   */
/*                                                                   */
/*           If this function fails, then *Error_Code will be > 0.   */
/*                                                                   */
/*   Error Handling:  If Volume_Handle is not a valid handle, a trap */
/*                    will be likely.  If Volume_Handle is a drive or*/
/*                    partition handle, *Error_Code will be > 0.     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Volume_Information_Record _System Get_Volume_Information( ADDRESS Volume_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Volume                                    */
/*                                                                   */
/*   Descriptive Name:  This function creates a volume from a list of*/
/*                      partitions.  The partitions are specified by */
/*                      their corresponding handles.                 */
/*                                                                   */
/*   Input: char         Name[] - The name to assign to the newly    */
/*                                created volume.                    */
/*          BOOLEAN      Create_LVM_Volume - If TRUE, then an LVM    */
/*                                           volume is created,      */
/*                                           otherwise a             */
/*                                           compatibility volume is */
/*                                           created.                */
/*          BOOLEAN      Bootable - If TRUE, the volume will not be  */
/*                                  created unless OS/2 can be booted*/
/*                                  from it.                         */
/*          char         Drive_Letter_Preference - This is the drive */
/*                                                 letter to use for */
/*                                                 accessing the     */
/*                                                 newly created     */
/*                                                 volume.           */
/*          CARDINAL32   FeaturesToUse - This is currently reserved  */
/*                                       for future use and should   */
/*                                       always be set to 0.         */
/*          CARDINAL32   Partition_Count - The number of partitions  */
/*                                         to link together to form  */
/*                                         the volume being created. */
/*          ADDRESS      Partition_Handles[] - An array of partition */
/*                                             handles with one entry*/
/*                                             for each partition    */
/*                                             that is to become part*/
/*                                             of the volume being   */
/*                                             created.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was created.        */
/*           *Error_Code will be > 0 if the volume could not be      */
/*              created.                                             */
/*                                                                   */
/*   Error Handling: If any of the handles in the partition handles  */
/*                   array is not valid, then a trap is likely.  If  */
/*                   Partition_Count is greater than the number of   */
/*                   entries in the partition handles array, then a  */
/*                   trap is likely.  If any of the handles in the   */
/*                   partition array are not partition handles, then */
/*                   *Error_Code will be > 0.  If the volume can NOT */
/*                   be created, then *Error_Code will be > 0 and any*/
/*                   memory allocated by this function will be freed.*/
/*                   If the volume can NOT be created, then the      */
/*                   existing partition/volume structure of the disk */
/*                   will be unchanged.                              */
/*                                                                   */
/*   Side Effects:  A volume may be created.                         */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Create_Volume( char         Name[VOLUME_NAME_SIZE],
BOOLEAN      Create_LVM_Volume,
BOOLEAN      Bootable,
char         Drive_Letter_Preference,
CARDINAL32   FeaturesToUse,
CARDINAL32   Partition_Count,
ADDRESS      Partition_Handles[],
CARDINAL32 * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Volume2                                   */
/*                                                                   */
/*   Descriptive Name:  This function creates a volume from a list of*/
/*                      partitions.  The partitions are specified by */
/*                      their corresponding handles.                 */
/*                                                                   */
/*   Input: char         Name[] - The name to assign to the newly    */
/*                                created volume.                    */
/*          BOOLEAN      Create_LVM_Volume - If TRUE, then an LVM    */
/*                                           volume is created,      */
/*                                           otherwise a             */
/*                                           compatibility volume is */
/*                                           created.                */
/*          BOOLEAN      Bootable - If TRUE, the volume will not be  */
/*                                  created unless OS/2 can be booted*/
/*                                  from it.                         */
/*          char         Drive_Letter_Preference - This is the drive */
/*                                                 letter to use for */
/*                                                 accessing the     */
/*                                                 newly created     */
/*                                                 volume.           */
/*          CARDINAL32   Feature_Count - The number of features to   */
/*                                       install on the volume being */
/*                                       created.  This field is     */
/*                                       ignored if Create_LVM_Volume*/
/*                                       is FALSE.                   */
/*          LVM_Feature_Specification_Record FeaturesToUse[] - An    */
/*                                         array of feature IDs and  */
/*                                         their associated LVM      */
/*                                         classes used to designate */
/*                                         which features to install */
/*                                         on the volume being       */
/*                                         created and the order in  */
/*                                         which to install them.    */
/*                                         This field is ignored if  */
/*                                         Create_LVM_Volume is      */
/*                                         FALSE.                    */
/*          CARDINAL32   Partition_Count - The number of partitions  */
/*                                         to link together to form  */
/*                                         the volume being created. */
/*          ADDRESS      Partition_Handles[] - An array of partition */
/*                                             handles with one entry*/
/*                                             for each partition    */
/*                                             that is to become part*/
/*                                             of the volume being   */
/*                                             created.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was created.        */
/*           *Error_Code will be > 0 if the volume could not be      */
/*              created.                                             */
/*                                                                   */
/*   Error Handling: If any of the handles in the partition handles  */
/*                   array is not valid, then a trap is likely.  If  */
/*                   Partition_Count is greater than the number of   */
/*                   entries in the partition handles array, then a  */
/*                   trap is likely.  If any of the handles in the   */
/*                   partition array are not partition handles, then */
/*                   *Error_Code will be > 0.  If the volume can NOT */
/*                   be created, then *Error_Code will be > 0 and any*/
/*                   memory allocated by this function will be freed.*/
/*                   If the volume can NOT be created, then the      */
/*                   existing partition/volume structure of the disk */
/*                   will be unchanged.                              */
/*                                                                   */
/*   Side Effects:  A volume may be created.                         */
/*                                                                   */
/*   Notes:  This function is only available in LVM Version 2, and   */
/*           is not available in the current OS/2 implementation.    */
/*                                                                   */
/*********************************************************************/
/*
* void _System Create_Volume2( char                               Name[VOLUME_NAME_SIZE],
* BOOLEAN                            Create_LVM_Volume,
* BOOLEAN                            Bootable,
* char                               Drive_Letter_Preference,
* CARDINAL32                         Feature_Count,
* LVM_Feature_Specification_Record   FeaturesToUse[],
* CARDINAL32                         Partition_Count,
* ADDRESS                            Partition_Handles[],
* CARDINAL32 *                       Error_Code
* );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Delete_Volume                                    */
/*                                                                   */
/*   Descriptive Name: Deletes the volume specified by Volume_Handle.*/
/*                                                                   */
/*   Input: ADDRESS Volume_Handle - The handle of the volume to      */
/*                                  delete.  All partitions which are*/
/*                                  part of the specified volume will*/
/*                                  be deleted also.                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume and its partitions  */
/*           are successfully deleted.  Otherwise, *Error_Code will  */
/*           be > 0.                                                 */
/*                                                                   */
/*   Error Handling: *Error_Code will be > 0 if an error occurs.  If */
/*                   the volume or any of its partitions can not be  */
/*                   deleted, then any changes made by this function */
/*                   will be undone.                                 */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  A volume and its partitions may be deleted.      */
/*                  System memory may be freed as the internal       */
/*                  structures used to track the deleted volume      */
/*                  are no longer required.                          */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Delete_Volume( ADDRESS Volume_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Hide_Volume                                      */
/*                                                                   */
/*   Descriptive Name: Hide volume "hides" a volume from OS/2 by     */
/*                     removing its drive letter assignment.  Without*/
/*                     a drive letter assignment, OS/2 can not access*/
/*                     (or "see") the volume.                        */
/*                                                                   */
/*   Input: ADDRESS Volume_Handle - The handle of the volume to hide.*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was successfully    */
/*           hidden.  If the volume could not be hidden, then        */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: *Error_Code will be > 0 if the volume can not be*/
/*                   hidden.  If the volume can not be hidden, then  */
/*                   nothing will be altered.                        */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Hide_Volume( ADDRESS Volume_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Expand_Volume                                    */
/*                                                                   */
/*   Descriptive Name: This function expands an existing volume by   */
/*                     linking additional partitions to it.          */
/*                                                                   */
/*   Input: ADDRESS Volume_Handle - The handle of the volume to be   */
/*                                  expanded.                        */
/*          CARDINAL32 Partition_Count - The number of partitions or */
/*                                       volumes to be added to the  */
/*                                       volume being expanded.      */
/*          ADDRESS Partition_Handles[] - An array of handles.  Each */
/*                                        handle in the array is the */
/*                                        handle of a partition      */
/*                                        which is to be added to    */
/*                                        the volume being expanded. */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume is successfully     */
/*           expanded.  If the volume can not be expanded,           */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If the volume can not be expanded, the state of */
/*                   the volume is unchanged and any memory allocated*/
/*                   by this function is freed.                      */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                   If any of the partition handles in the          */
/*                   Partition_handles array are not valid handles,  */
/*                   then a trap may result.                         */
/*                                                                   */
/*                   If any of the partition handles in the          */
/*                   Partition_Handles array are actually drive      */
/*                   handles, then this function will abort and      */
/*                   set *Error_Code to a non-zero value.            */
/*                                                                   */
/*                   If Partition_Count is greater than the number of*/
/*                   entries in the Partition_Handles array, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*   Side Effects:  A volume may be expanded.  If the volume is      */
/*                  expanded using another volume, the partitions    */
/*                  on the second volume will be linked to those of  */
/*                  the first volume and all data on the second      */
/*                  volume will be lost.                             */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Expand_Volume ( ADDRESS         Volume_Handle,
CARDINAL32      Partition_Count,
ADDRESS         Partition_Handles[],
CARDINAL32 *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Assign_Drive_Letter                              */
/*                                                                   */
/*   Descriptive Name: Assigns a drive letter to a volume.           */
/*                                                                   */
/*   Input: ADDRESS Volume_Handle - The handle of the volume which   */
/*                                  is to have its assigned drive    */
/*                                  letter changed.                  */
/*          char  New_Drive_Preference - The new drive letter to     */
/*                                       assign to the volume.       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the drive letter was assigned  */
/*           successfully; otherwise *Error_Code will be > 0.        */
/*                                                                   */
/*   Error Handling: If the drive letter assignment can not be made, */
/*                   the volume will not be altered.                 */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  A volume may have its drive letter assignment    */
/*                  changed.                                         */
/*                                                                   */
/*   Notes:  If the drive letter being assigned is already in use by */
/*           volume which does not lie on removable media, then the  */
/*           drive assignment will NOT be made.                      */
/*                                                                   */
/*********************************************************************/
void _System Assign_Drive_Letter( ADDRESS      Volume_Handle,
char         New_Drive_Preference,
CARDINAL32 * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Installable                                  */
/*                                                                   */
/*   Descriptive Name: Marks a volume as being the volume to install */
/*                     OS/2 on.                                      */
/*                                                                   */
/*   Input: ADDRESS Volume_Handle - The handle of the volume to which*/
/*                                  OS/2 should be installed.        */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the volume is successfully marked as installable,    */
/*           *Error_Code will be 0; otherwise *Error_Code will       */
/*           be > 0.                                                 */
/*                                                                   */
/*   Error Handling: If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  The specified volume may be marked as            */
/*                  installable.                                     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Installable ( ADDRESS Volume_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Installable_Volume                           */
/*                                                                   */
/*   Descriptive Name: Marks a volume as being the volume to install */
/*                     OS/2 on.                                      */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If a volume is mared installable, its information will  */
/*           be returned and *Error_Code will be LVM_ENGINE_NO_ERROR.*/
/*           If there is no volume marked installable, then          */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: An error code is returned if there is an error. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Volume_Information_Record _System Get_Installable_Volume ( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Convert_Volumes_To_V1                            */
/*                                                                   */
/*   Descriptive Name: This function attempts to convert all LVM     */
/*                     volumes in the system into a format that can  */
/*                     be used by LVM Version 1, which was shipped   */
/*                     with Warp Server for e-business.  This        */
/*                     function returns a bitmap of the drive letters*/
/*                     corresponding to Volumes that can not be      */
/*                     converted.                                    */
/*                                                                   */
/*   Input: BOOLEAN * Hidden_Volume_Conversion_Failure - The address */
/*                                 of a BOOLEAN variable in which    */
/*                                 to store a flag indicating if     */
/*                                 there were hidden volumes that    */
/*                                 could not be converted.  If       */
/*                                 *Hidden_Volume_Conversion_Failure */
/*                                 is TRUE, then there were hidden   */
/*                                 volumes that could not be         */
/*                                 converted.  If FALSE, then there  */
/*                                 were no hidden volumes, or the    */
/*                                 hidden volumes that existed were  */
/*                                 converted successfully.           */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the drive letters     */
/*           corresponding to volumes that could not be converted to */
/*           LVM Version 1 format.  If this function is successful   */
/*           and all volumes were converted, then *Error_Code will be*/
/*           set to LVM_ENGINE_NO_ERROR and the bitmap returned will */
/*           have no bits set.  If this function failes, *Error_Code */
/*           will contain a non-zero error code and the bitmap       */
/*           returned by this function may be non-zero.              */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  All LVM volumes that can be converted to LVM     */
/*                  Version 1 format will be.                        */
/*                                                                   */
/*   Notes:  Bit 0 in the bitmap returned by this function represents*/
/*           drive letter 'A'.                                       */
/*                                                                   */
/*           This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* CARDINAL32 _System Convert_Volumes_To_V1 ( BOOLEAN *    Hidden_Volume_Conversion_Failure,
* CARDINAL32 * Error_Code ) ;
*/

/* ************************************************************************** *

Functions relating to Partitions, Drives, and Volumes.

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Name                                         */
/*                                                                   */
/*   Descriptive Name: Sets the name of a volume, drive, or partition*/
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of the drive, partition, or  */
/*                           volume which is to have its name set.   */
/*          char New_Name[] - The new name for the drive/partition/  */
/*                            volume.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the name is set as specified.  */
/*           If the name can not be set, *Error_Code will be > 0.    */
/*                                                                   */
/*   Error Handling: If the name can not be set, then drive/volume/  */
/*                   partition is not modified.                      */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*   Side Effects:  A drive/volume/partition may have its name set.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Name ( ADDRESS      Handle,
char         New_Name[],
CARDINAL32 * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Startable                                    */
/*                                                                   */
/*   Descriptive Name: Sets the specified volume or partition        */
/*                     startable.  If a volume is specified, it must */
/*                     be a compatibility volume whose partition is  */
/*                     a primary partition on the first drive.  If a */
/*                     partition is specified, it must be a primary  */
/*                     partition on the first drive in the system.   */
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of the partition or volume   */
/*                           which is to be set startable.           */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the specified volume or        */
/*           partition was set startable.                            */
/*           If the name can not be set, *Error_Code will be > 0.    */
/*                                                                   */
/*   Error Handling: If the volume or partition could not be set     */
/*                   startable, then nothing in the system is        */
/*                   changed.                                        */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*   Side Effects:  Any other partition or volume which is marked    */
/*                  startable will have its startable flag cleared.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Startable ( ADDRESS      Handle,
CARDINAL32 * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Valid_Options                                */
/*                                                                   */
/*   Descriptive Name: Returns a bitmap where each bit in the bitmap */
/*                     corresponds to a possible operation that the  */
/*                     LVM Engine can perform.  Those bits which are */
/*                     1 represent operations which can be performed */
/*                     on the item specified by Handle.  Those bits  */
/*                     which are 0 are not allowed on the item       */
/*                     specified by Handle.                          */
/*                                                                   */
/*   Input: ADDRESS Handle - This is any valid drive, volume, or     */
/*                           partition handle.                       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output:  A bitmap indicating which operations are valid on the  */
/*            item specified by Handle.                              */
/*                                                                   */
/*            If no errors occur, *Error_Code will be 0, otherwise   */
/*            *Error_Code will be > 0.                               */
/*                                                                   */
/*   Error Handling:  If Handle is not valid, a trap will be likely. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The values of the various bits in the bitmap returned   */
/*           by this function are defined near the beginning of this */
/*           file, immediately after all of the structure            */
/*           definitions.                                            */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _System Get_Valid_Options( ADDRESS Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Child_Handles                                */
/*                                                                   */
/*   Descriptive Name: Given the handle of a volume or aggregate,    */
/*                     this function will return the handles of the  */
/*                     children of the volume or aggregate.  This    */
/*                     allows the entire tree representation of a    */
/*                     volume to be traversed, a level at a time.    */
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of the volume or aggregate   */
/*                           whose children are required.            */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  If successful, an LVM_Handle_Array_Record is returned  */
/*            with a non-zero Count.  Also, *Error_Code will be set  */
/*            to LVM_ENGINE_NO_ERROR.  If an error occurs, then      */
/*            the Count field will be 0 and *Error_Code will contain */
/*            a non-zero error code.                                 */
/*                                                                   */
/*   Error Handling: If Handle is not a valid handle, then a trap is */
/*                   likely.  If Handle is the handle of partition,  */
/*                   then *Error_Code will be set to                 */
/*                   LVM_ENGINE_NO_CHILDREN.  If Handle is not a     */
/*                   volume or aggregate handle, then *Error_Code    */
/*                   will be set to LVM_ENGINE_BAD_HANDLE.           */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* LVM_Handle_Array_Record  _System Get_Child_Handles( ADDRESS Handle, CARDINAL32 * Error_Code);
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Parent_Handle                                */
/*                                                                   */
/*   Descriptive Name: Given the handle of a partition or aggregate, */
/*                     this function will return the handle of the   */
/*                     parent of the partition or aggregate.         */
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of the partition or aggregate*/
/*                           whose parent is required.               */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  If successful, the handle of the parent is returned    */
/*            as the function result and *Error_Code will be set to  */
/*            LVM_ENGINE_NO_ERROR.                                   */
/*            If an error occurs, then NULL will be the function     */
/*            result and *Error_Code will contain a non-zero error   */
/*            code.                                                  */
/*                                                                   */
/*   Error Handling: If Handle is not a valid handle, then a trap is */
/*                   likely.  If Handle is the handle of volume,     */
/*                   then *Error_Code will be set to                 */
/*                   LVM_ENGINE_NO_PARENT.  If Handle is not the     */
/*                   handle of a volume, partition, or aggregate     */
/*                   then *Error_Code will be set to                 */
/*                   LVM_ENGINE_BAD_HANDLE.                          */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* ADDRESS _System Get_Parent_Handle( ADDRESS Handle, CARDINAL32 * Error_Code);
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Features                                     */
/*                                                                   */
/*   Descriptive Name: Returns the feature ID information for each of*/
/*                     the features that are installed on the        */
/*                     item specified by Handle.                     */
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of the object to use.        */
/*                                                                   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If successful, a Feature_Information_Array structure is */
/*           returned with a non-zero Count.  Also, *Error_Code will */
/*           be set to LVM_ENGINE_NO_ERROR.  If an error occurs,     */
/*           then the Count field in the structure will be 0 and     */
/*           (*Error_Code) will contain a non-zero error code.       */
/*                                                                   */
/*   Error Handling:  If Handle is not a valid handle, a trap        */
/*                    will be likely.                                */
/*                                                                   */
/*   Side Effects:  Memory is allocated using the LVM Engine's memory*/
/*                  manager for the array of Feature_ID_Data items   */
/*                  being returned.                                  */
/*                                                                   */
/*   Notes:  This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* Feature_Information_Array _System Get_Features( ADDRESS Handle, CARDINAL32 * Error_Code );
*/

/* ************************************************************************** *

Functions relating to Boot Manager

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Boot_Manager_Is_Installed                        */
/*                                                                   */
/*   Descriptive Name: Indicates whether or not Boot Manager is      */
/*                     installed on the first or second hard drives  */
/*                     in the system.                                */
/*                                                                   */
/*   Input: BOOLEAN * Active - *Active is set to TRUE if LVM found an*/
/*                             active copy of Boot Manager on the    */
/*                             system.  If LVM could not find an     */
/*                             active copy of Boot Manager on the    */
/*                             system, but did find an inactive copy */
/*                             of Boot Manager, then *Active will be */
/*                             set to FALSE.                         */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: TRUE is returned if Boot Manager is found.  If this     */
/*           copy of Boot Manager is Active, then *Active will be set*/
/*           to TRUE.  If the copy of Boot Manager is not currently  */
/*           active, then *Active will be set to FALSE.              */
/*                                                                   */
/*           FALSE is returned if Boot Manager is not found or if an */
/*           error occurs.  In this case, *Active is undefined.      */
/*                                                                   */
/*           *Error_Code will be 0 if no errors occur; otherwise it  */
/*           will be > 0.                                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Boot_Manager_Is_Installed( BOOLEAN * Active, CARDINAL32 * Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Boot_Manager_Handle                          */
/*                                                                   */
/*   Descriptive Name: Returns the handle of the partition containing*/
/*                     Boot Manager.                                 */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If Boot Manager is NOT installed, NULL is returned.     */
/*           If Boot Manager is installed, whether it is active or   */
/*           not, the handle of the partition it resides in is       */
/*           returned.                                               */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Get_Boot_Manager_Handle( CARDINAL32 * Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Add_To_Boot_Manager                              */
/*                                                                   */
/*   Descriptive Name: Adds the volume/partition to the Boot Manager */
/*                     menu.                                         */
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of a partition or volume that*/
/*                           is to be added to the Boot Manager menu.*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition or volume was    */
/*           successfully added to the Boot Manager menu; otherwise  */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If the partition/volume can not be added to the */
/*                   Boot Manager menu, no action is taken and       */
/*                   *Error_Code will contain a non-zero error code. */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*                   If Handle represents a drive, then this function*/
/*                   will abort and set *Error_Code to a non-zero    */
/*                   value.                                          */
/*                                                                   */
/*   Side Effects:  The Boot Manager menu may be altered.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Add_To_Boot_Manager ( ADDRESS Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Remove_From_Boot_Manager                         */
/*                                                                   */
/*   Descriptive Name: Removes the specified partition or volume     */
/*                     from the Boot Manager menu.                   */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition or volume was    */
/*           successfully removed to the Boot Manager menu;          */
/*           otherwise *Error_Code will be > 0.                      */
/*                                                                   */
/*   Error Handling: If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*                   If Handle represents a drive, or if Handle      */
/*                   represents a volume or partition which is NOT on*/
/*                   the boot manager menu, then this function       */
/*                   will abort and set *Error_Code to a non-zero    */
/*                   value.                                          */
/*                                                                   */
/*   Side Effects:  The Boot Manager menu may be altered.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Remove_From_Boot_Manager ( ADDRESS Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Boot_Manager_Menu                            */
/*                                                                   */
/*   Descriptive Name: Returns an array containing the handles of the*/
/*                     partitions and volumes appearing on the       */
/*                     Boot Manager menu.                            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function returns a Boot_Manager_Menu structure.     */
/*           This structure contains two items: a pointer to an array*/
/*           of Boot_Manager_Menu_Items and a count of how many items*/
/*           are in the array.  Each Boot_Manager_Menu_Item contains */
/*           a handle and a BOOLEAN variable to indicate whether the */
/*           handle is for a partition or a volume.                  */
/*                                                                   */
/*           If this function is successful, then *Error_Code will   */
/*           be 0.                                                   */
/*                                                                   */
/*           If an error occurs, the Count field in the              */
/*           Boot_Manager_Menu will be 0 and the corresponding       */
/*           pointer will be NULL.  *Error_Code will be > 0.         */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                   any memory allocated by this function will be   */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Boot_Manager_Menu  _System Get_Boot_Manager_Menu ( CARDINAL32 * Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Install_Boot_Manager                             */
/*                                                                   */
/*   Descriptive Name: This function installs Boot Manager.  It can  */
/*                     be used to replace an existing Boot Manager   */
/*                     as well.                                      */
/*                                                                   */
/*   Input: CARDINAL32  Drive_Number - The number of the drive to    */
/*                                     install Boot Manager on.  Must*/
/*                                     be 1 or 2.                    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If this function is successful, then *Error_Code will be*/
/*           0; otherwise it will be > 0.                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be set to a*/
/*                   non-zero value.  Depending upon the error, it   */
/*                   is possible that the Boot Manager partition can */
/*                   be left in an unusuable state (such as for a    */
/*                   write error).                                   */
/*                                                                   */
/*   Side Effects: Boot Manager may be installed on drive 1 or 2.    */
/*                 The MBR for drive 1 may be altered.               */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Install_Boot_Manager ( CARDINAL32   Drive_Number, CARDINAL32 * Error_Code );           /* Only drives 0 and 1 are acceptable. */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Remove_Boot_Manager                              */
/*                                                                   */
/*   Descriptive Name: Removes Boot Manager from the system.         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if Boot Manager was successfully  */
/*           removed from the system; otherwise *Error_Code will     */
/*           be 0.                                                   */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  Boot Manager will be removed from the system.    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Remove_Boot_Manager( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Boot_Manager_Options                         */
/*                                                                   */
/*   Descriptive Name: Sets the Boot Managers Options.  The options  */
/*                     that can be set are: whether or not the time- */
/*                     out timer is active, how long the timer-out   */
/*                     is, the partition to boot by default, and     */
/*                     whether or not Boot Manager should display its*/
/*                     menu using default mode or advanced mode.     */
/*                                                                   */
/*   Input: ADDRESS Handle - The handle of the partition or volume   */
/*                           to boot if the time-out timer is active */
/*                           and the time-out value is reached.      */
/*          BOOLEAN Timer_Active - If TRUE, then the time-out timer  */
/*                                 is active.                        */
/*          CARDINAL32 Time_Out_Value - If the time-out timer is     */
/*                                      active, this is the time-out */
/*                                      value, in seconds.           */
/*          BOOLEAN Advanced_Mode - If TRUE, then Boot Manager will  */
/*                                  operate in advanced mode.  If    */
/*                                  FALSE, then normal mode will be  */
/*                                  in effect.                       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if no errors occur.  If an error  */
/*           does occur, then *Error_Code will be > 0.               */
/*                                                                   */
/*   Error Handling: If an error occurs, no changes will be made to  */
/*                   Boot Manager and *Error_Code will be set a      */
/*                   non-zero error code.                            */
/*                                                                   */
/*   Side Effects:  Boot Manager may be modified.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Boot_Manager_Options( ADDRESS      Handle,
BOOLEAN      Timer_Active,
CARDINAL32   Time_Out_Value,
BOOLEAN      Advanced_Mode,
CARDINAL32 * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Boot_Manager_Options                         */
/*                                                                   */
/*   Descriptive Name: This function returns the current Boot Manager*/
/*                     settings for the various Boot Manager options.*/
/*                                                                   */
/*   Input: ADDRESS * Handle - The handle for the default boot volume*/
/*                             or partition.                         */
/*          BOOLEAN * Handle_Is_Volume - If TRUE, then Handle        */
/*                                       represents a volume.  If    */
/*                                       FALSE, then Handle          */
/*                                       represents a partition.     */
/*          BOOLEAN * Timer_Active - If TRUE, then the time-out timer*/
/*                                   is active.  If FALSE, then the  */
/*                                   time-out timer is not active.   */
/*          CARDINAL32 * Time_Out_Value - If the time-out timer is   */
/*                                        active, then this is the   */
/*                                        number of seconds that Boot*/
/*                                        Manager will wait for user */
/*                                        input before booting the   */
/*                                        default volume/partition.  */
/*          BOOLEAN * Advanced_Mode - If TRUE, the Boot Manager is   */
/*                                    operating in advanced mode.  If*/
/*                                    FALSE, then Boot Manager is    */
/*                                    operating in normal mode.      */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Handle, *Handle_Is_Volume, *Timer_Active,              */
/*           *Time_out_value, *Advanced_Mode, and *Error_Code are all*/
/*           set by this function.  If there are no errors, then     */
/*           *Error_Code will be set to 0.  If any errors occur, then*/
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If any of the parameters are invalid, then a    */
/*                   trap is likely.  If Boot Manager is not         */
/*                   installed, then *Error_Code will be > 0.        */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Get_Boot_Manager_Options( ADDRESS    *  Handle,
BOOLEAN    *  Handle_Is_Volume,
BOOLEAN    *  Timer_Active,
CARDINAL32 *  Time_Out_Value,
BOOLEAN    *  Advanced_Mode,
CARDINAL32 * Error_Code
);

/* ************************************************************************** *

Other Functions

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Allocate_Engine_Memory                          */
/*                                                                   */
/*   Descriptive Name:  Allocates a block of memory using LVM.DLL's  */
/*                      memory management functions.                 */
/*                                                                   */
/*   Input: CARDINAL32 Size - The number of bytes of memory to       */
/*                            allocate.                              */
/*                                                                   */
/*   Output: The address of the block of memory which was allocated, */
/*           or NULL if the requested amount of memory could not be  */
/*           allocated.                                              */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  The specified number of bytes is allocated from  */
/*                  the memory manager imbedded in LVM.DLL.  Memory  */
/*                  allocated by this function must be freed using   */
/*                  Free_Engine_Memory function.  The use of any     */
/*                  memory manager to free the memory could result in*/
/*                  Bad Things Happening!                            */
/*                                                                   */
/*   Notes:  This function is specific to LVM version 2, and is not  */
/*           available in the current OS/2 implementation            */
/*                                                                   */
/*********************************************************************/
/*
* ADDRESS _System Allocate_Engine_Memory( CARDINAL32 Size );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Free_Engine_Memory                              */
/*                                                                   */
/*   Descriptive Name: Frees a memory object created by LVM.DLL and  */
/*                     returned to a user of LVM.DLL.                */
/*                                                                   */
/*   Input: ADDRESS Object : The address of the memory object to     */
/*                           free.  This could be the                */
/*                           Drive_Control_Data field of a           */
/*                           Drive_Control_Record, the               */
/*                           Partition_Array field of a              */
/*                           Partition_Information_Array structure,  */
/*                           or any other dynamically allocated      */
/*                           memory object created by LVM.DLL and    */
/*                           returned by a function in LVM.DLL.      */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  A trap or exception could occur if a bad address is     */
/*           passed into this function.                              */
/*                                                                   */
/*********************************************************************/
void _System Free_Engine_Memory( ADDRESS Object );

/*********************************************************************/
/*                                                                   */
/*   Function Name: New_MBR                                          */
/*                                                                   */
/*   Descriptive Name: This function lays down a new MBR on the      */
/*                     specified drive.                              */
/*                                                                   */
/*   Input: ADDRESS Drive_Handle - The handle of the drive on which  */
/*                                 the new MBR is to be placed.      */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the new MBR was successfully   */
/*           placed on the specified drive.  If the operation failed */
/*           for any reason, then *Error_Code will contain a non-zero*/
/*           error code.                                             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the existing MBR is not*/
/*                   altered and *Error_Code will be > 0.            */
/*                                                                   */
/*   Side Effects:  A new MBR may be placed on the specified drive.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System New_MBR( ADDRESS Drive_Handle, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Available_Drive_Letters                      */
/*                                                                   */
/*   Descriptive Name: This function returns a bitmap indicating     */
/*                     which drive letters are available for use.    */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the available drive   */
/*           letters.  If this function is successful, then          */
/*           *Error_Code will be set to 0.  Otherwise, *Error_Code   */
/*           will be > 0 and the bitmap returned will have all bits  */
/*           set to 0.                                               */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  A drive letter is available if it is not associated     */
/*           with a volume located on a disk drive controlled        */
/*           by OS2DASD.                                             */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _System Get_Available_Drive_Letters ( CARDINAL32 * Error_Code ) ;

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Reserved_Drive_Letters                       */
/*                                                                   */
/*   Descriptive Name: This function returns a bitmap indicating     */
/*                     which drive letters are reserved for use by   */
/*                     devices NOT under the control of LVM.         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the drive letters     */
/*           which are being used by devices which are NOT controlled*/
/*           by LVM.  While a Volume CAN be assigned a drive letter  */
/*           from this list, a reboot will almost always be required */
/*           in order for the assignment to take place.              */
/*           If this function is successful, then *Error_Code will be*/
/*           set to 0.  Otherwise, *Error_Code will be > 0 and the   */
/*           bitmap returned will have all bits set to 0.            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  Devices which are assigned drive letters but which are  */
/*           NOT under LVM control include:  CD-ROM, Network drives, */
/*           parallel port attached devices, and any DASD devices    */
/*           not controlled by OS2DASD.                              */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _System Get_Reserved_Drive_Letters ( CARDINAL32 * Error_Code ) ;

/*********************************************************************/
/*                                                                   */
/*   Function Name: Reboot_Required                                  */
/*                                                                   */
/*   Descriptive Name: This function indicates whether or not any    */
/*                     changes were made to the partitioning of the  */
/*                     disks in the system which would require a     */
/*                     reboot to make functional.                    */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value will be TRUE if the system    */
/*           must be rebooted as a result of disk partitioning       */
/*           changes.                                                */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Reboot_Required ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Changes_Pending                                  */
/*                                                                   */
/*   Descriptive Name: This function indicates whether or not any    */
/*                     changes were made to the partitioning of the  */
/*                     disks in the system which have not yet been   */
/*                     comitted to disk.                             */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value will be TRUE if there are     */
/*           uncomitted changes to the partitioning of one or more of*/
/*           the drives in the system.                               */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Changes_Pending ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Reboot_Flag                                  */
/*                                                                   */
/*   Descriptive Name: This function sets the Reboot Flag.  The      */
/*                     Reboot Flag is a special flag on the boot     */
/*                     disk used by the install program to keep      */
/*                     track of whether or not the system was just   */
/*                     rebooted.  It is used by the various phases   */
/*                     of install.                                   */
/*                                                                   */
/*   Input: BOOLEAN Reboot - The new value for the Reboot Flag.  If  */
/*                           TRUE, then the reboot flag will be set. */
/*                           If FALSE, then the reboot flag will be  */
/*                           cleared.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to 0 if there are no errors.    */
/*           *Error_Code will be > 0 if an error occurs.             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the value of the Reboot*/
/*                   Flag will be unchanged.                         */
/*                                                                   */
/*   Side Effects:  The value of the Reboot Flag may be changed.     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Reboot_Flag( BOOLEAN Reboot, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Reboot_Flag                                  */
/*                                                                   */
/*   Descriptive Name: This function returns the value of the Reboot */
/*                     Flag.  The Reboot Flag is a special flag on   */
/*                     the boot disk used by the install program to  */
/*                     keep track of whether or not the system was   */
/*                     just rebooted.  It is used by the various     */
/*                     phases of install.                            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function return value will be TRUE if no errors     */
/*           occur and the Reboot Flag is set.  *Error_Code will be  */
/*           0 under these conditions.  If an error occurs, the      */
/*           function return value will be FALSE and *Error_Code     */
/*           will be > 0.                                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                   The value of the reboot flag will be unchanged. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Get_Reboot_Flag( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*   Function Name: Set_Install_Flags                                */
/*                                                                   */
/*   Descriptive Name: This function sets the value of the Install   */
/*                     Flags.  The Install Flags reside in a 32 bit  */
/*                     field in the LVM dataspace.  These flags are  */
/*                     not used by LVM, thereby leaving Install free */
/*                     to use them for whatever it wants.            */
/*                                                                   */
/*   Input: CARDINAL32 Install_Flags - The new value for the Install */
/*                                     Flags.                        */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32    */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to 0 if there are no errors.    */
/*           *Error_Code will be > 0 if an error occurs.             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the value of the       */
/*                   Install Flags will be unchanged.                */
/*                                                                   */
/*   Side Effects:  The value of the Install Flags may be changed.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Install_Flags( CARDINAL32 Install_Flags, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Install_Flags                                */
/*                                                                   */
/*   Descriptive Name: This function returns the value of the Install*/
/*                     Flags.  The Install Flags reside in a 32 bit  */
/*                     field in the LVM dataspace.  These flags are  */
/*                     not used by LVM, thereby leaving Install free */
/*                     to use them for whatever it wants.            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function returns the current value of the Install   */
/*           Flags stored in the LVM Dataspace.                      */
/*           *Error_Code will be LVM_ENGINE_NO_ERROR if the function */
/*           is successful.  If an error occurs, the function will   */
/*           return 0 and *Error_Code will be > 0.                   */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _System Get_Install_Flags( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Min_Install_Size                             */
/*                                                                   */
/*   Descriptive Name: This function tells the LVM Engine how big a  */
/*                     partition/volume must be in order for it to   */
/*                     marked installable.  If this function is not  */
/*                     used to set the minimum size for an           */
/*                     installable partition/volume, the LVM Engine  */
/*                     will use a default value of 300 MB.           */
/*                                                                   */
/*   Input: CARDINAL32 Min_Sectors - The minimum size, in sectors,   */
/*                                   that a partition must be in     */
/*                                   order for it to be marked as    */
/*                                   installable.                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Min_Install_Size ( CARDINAL32  Min_Sectors );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Free_Space_Threshold                         */
/*                                                                   */
/*   Descriptive Name: This function tells the LVM Engine not to     */
/*                     report blocks of free space which are less    */
/*                     than the size specified.  The engine defaults */
/*                     to not reporting blocks of free space which   */
/*                     are smaller than 2048 sectors (1 MB).         */
/*                                                                   */
/*   Input: CARDINAL32 Min_Sectors - The minimum size, in sectors,   */
/*                                   that a block of free space must */
/*                                   be in order for the LVM engine  */
/*                                   to report it.                   */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Set_Free_Space_Threshold( CARDINAL32  Min_Sectors );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Read_Sectors                                     */
/*                                                                   */
/*   Descriptive Name: This function reads one or more sectors from  */
/*                     the specified drive and places the data read  */
/*                     in Buffer.                                    */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    read from.  The drives in the  */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to read from.     */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       read into memory.           */
/*          ADDRESS Buffer : The location to put the data read into. */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, then the data read will be placed in     */
/*              memory starting at Buffer, and *Error will be        */
/*              LVM_ENGINE_NO_ERROR.                                 */
/*           If Unsuccessful, then *Error will be > 0 and the        */
/*              contents of memory starting at Buffer is undefined.  */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be read into memory starting at Buffer.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Read_Sectors ( CARDINAL32          Drive_Number,
LBA                 Starting_Sector,
CARDINAL32          Sectors_To_Read,
ADDRESS             Buffer,
CARDINAL32 *        Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Write_Sectors                                    */
/*                                                                   */
/*   Descriptive Name: This function writes data from memory to one  */
/*                     or more sectors on the specified drive.       */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    write to.  The drives in the   */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to write to.      */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       be written.                 */
/*          ADDRESS Buffer : The location of the data to be written  */
/*                           to disk.                                */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, then the data at Buffer will be placed   */
/*              on the disk starting at the sector specified, and    */
/*              *Error will be LVM_ENGINE_NO_ERROR.                  */
/*           If Unsuccessful, then *Error will be > 0 and the        */
/*              contents of the disk starting at sector              */
/*              Starting_Sector is undefined.                        */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be written to disk.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Write_Sectors ( CARDINAL32          Drive_Number,
LBA                 Starting_Sector,
CARDINAL32          Sectors_To_Write,
ADDRESS             Buffer,
CARDINAL32 *        Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Rediscover_PRMs                                  */
/*                                                                   */
/*   Descriptive Name: Causes OS2LVM and OS2DASD to check PRMs for   */
/*                     new or changed media.                         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the Rediscover PRM operation was successful, then    */
/*           *Error_Code will be LVM_ENGINE_NO_ERROR.  If there      */
/*           was an error, then *Error_Code will be > 0.             */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  New volumes may be discovered and assigned drive */
/*                  letters by OS2LVM and OS2DASD.                   */
/*                                                                   */
/*   Notes: The LVM Engine must be CLOSED when this function is      */
/*          called as this function is disabled while it is open!    */
/*                                                                   */
/*********************************************************************/
void _System Rediscover_PRMs( CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_LVM_View                                     */
/*                                                                   */
/*   Descriptive Name:  This function gets the OS2LVM data for the   */
/*                      specified drive letter.  The intent is to    */
/*                      allow the determination of what drive letter */
/*                      a volume really has given the possibilities  */
/*                      of conflict or a drive preference of '*'.    */
/*                                                                   */
/*   Input:  char  IFSM_Drive_Letter : The drive letter for which the*/
/*                                     OS2LVM data is requested.     */
/*           CARDINAL32 * Drive_Number : The address of a variable   */
/*                                       to hold the OS/2 drive      */
/*                                       number of the drive         */
/*                                       containing the first        */
/*                                       partition of the volume     */
/*                                       currently assigned to the   */
/*                                       requested drive letter.     */
/*           CARDINAL32 * Partition_LBA : The address of a variable  */
/*                                        to hold the LBA of the     */
/*                                        first partition of the     */
/*                                        volume currently assigned  */
/*                                        to the requested drive     */
/*                                        letter.                    */
/*           char * LVM_Drive_Letter : The address of a variable to  */
/*                                     hold the drive letter that    */
/*                                     OS2LVM thinks the volume      */
/*                                     assigned to the requested     */
/*                                     drive letter should have.     */
/*           BYTE * UnitID : The address of a variable to hold the   */
/*                           OS2LVM unit ID for the volume associated*/
/*                           with the requested drive letter.        */
/*                                                                   */
/*   Output:  The function return value will be TRUE if the function */
/*            completed successfully.                                */
/*                                                                   */
/*   Error Handling: If this function fails, the specified drive     */
/*                   letter is either not in use, or is in use by a  */
/*                   device not controlled by OS2LVM.                */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function can be used with the LVM Engine open or    */
/*          closed.                                                  */
/*                                                                   */
/*********************************************************************/
BOOLEAN _System Get_LVM_View( char         IFSM_Drive_Letter,
CARDINAL32 * Drive_Number,
CARDINAL32 * Partition_LBA,
char *       LVM_Drive_Letter,
BYTE *       UnitID);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Start_Logging                                    */
/*                                                                   */
/*   Descriptive Name: Enables the LVM Engine logging.  Once enabled,*/
/*                     the LVM Engine logging function will log all  */
/*                     LVM Engine activity to the specified log file.*/
/*                     The data is logged in a binary format for     */
/*                     compactness and speed.                        */
/*                                                                   */
/*   Input: char * Filename - The filename of the file to use as the */
/*                            log file.                              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the logging file was successfully created, then      */
/*           *Error_Code will be 0.  If the log file could not be    */
/*           created, then *Error_Code will be > 0.                  */
/*                                                                   */
/*   Error Handling: If the log file can not be created, then        */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  A file may be created/opened for logging of      */
/*                  LVM Engine actions.                              */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Start_Logging( char * Filename, CARDINAL32 * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Stop_Logging                                     */
/*                                                                   */
/*   Descriptive Name: This function ends LVM Engine logging and     */
/*                     closes the log file.                          */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if this function completes        */
/*           successfully; otherwise it will be > 0.                 */
/*                                                                   */
/*   Error Handling: If the log file is not currently opened, or if  */
/*                   the close operation fails on the log file, then */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  The log file may be closed.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Stop_Logging ( CARDINAL32 * Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Export_Configuration                             */
/*                                                                   */
/*   Descriptive Name: This function creates a file containing all of*/
/*                     the LVM.EXE commands necessary to recreate the*/
/*                     current partition/volume layout.              */
/*                                                                   */
/*   Input: char * Filename - FQN of the output file to be created   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A file creating LVM commands will be created.           */
/*           *Error_Code will be 0 if this function completes        */
/*           successfully; otherwise it will be > 0.                 */
/*                                                                   */
/*   Error Handling: If the output file cannot be created, or if     */
/*                   the close operation fails on the log file, then */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  A file may be created.                           */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Export_Configuration( char * Filename, CARDINAL32 * Error_Code );

#ifdef BUILD_LVM_ENGINE

/* ************************************************************************** *

16 Bit Functions relating to the LVM Engine itself

************************************************************************** */

/****************************************************************************************************/
/*                                                                                                  */
/*   Function Name: OPEN_LVM_ENGINE16                                                               */
/*                                                                                                  */
/*   Descriptive Name: Opens the LVM Engine and readies it for use.                                 */
/*                                                                                                  */
/*   Input: BOOLEAN Ignore_CHS : If TRUE, then the LVM engine will not check the CHS values in the  */
/*                               MBR/EBR partition tables for validity.  This is useful if there    */
/*                               are drive geometry problems, such as the drive was partitioned and */
/*                               formatted with one geometry and then moved to a different machine  */
/*                               which uses a different geometry for the drive.  This would cause   */
/*                               the starting and ending CHS values in the partition tables to      */
/*                               be inconsistent with the size and partition offset entries in the  */
/*                               partition tables.  Setting Ignore_CHS to TRUE will disable the     */
/*                               LVM Engine's CHS consistency checks, thereby allowing the drive    */
/*                               to be partitioned.                                                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in which to store an error code   */
/*                                    should an error occur.                                        */
/*                                                                                                  */
/*   Output:  *Error_Code will be 0 if this function completes successfully.  If an error occurs,   */
/*            *Error_Code will contain a non-zero error code.                                       */
/*                                                                                                  */
/*   Error Handling: If this function aborts with an error, all memory allocated during the course  */
/*                   of this function will be released.  Disk read errors will be reported to the   */
/*                   user via pop-up error messages.  Disk read errors will only cause this         */
/*                   function to abort if none of the disk drives in the system could be            */
/*                   successfully read.                                                             */
/*                                                                                                  */
/*   Side Effects:  The LVM Engine will be initialized.  The partition tables for all OS2DASD       */
/*                  controlled disk drives will be read into memory.  Memory will be allocated for  */
/*                  the data structures used by the LVM Engine.                                     */
/*                                                                                                  */
/*   Notes:  This is provided for programs that used LVM Version 1.  This function assumes an       */
/*           LVM_Interface_Type of VIO_Interface.                                                   */
/*                                                                                                  */
/****************************************************************************************************/
void _Far16 _Pascal _loadds OPEN_LVM_ENGINE16( BOOLEAN Ignore_CHS, CARDINAL32 * _Seg16 Error_Code );

/****************************************************************************************************/
/*                                                                                                  */
/*   Function Name: OPEN_LVM_ENGINE216                                                              */
/*                                                                                                  */
/*   Descriptive Name: Opens the LVM Engine and readies it for use.                                 */
/*                                                                                                  */
/*   Input: BOOLEAN Ignore_CHS : If TRUE, then the LVM engine will not check the CHS values in the  */
/*                               MBR/EBR partition tables for validity.  This is useful if there    */
/*                               are drive geometry problems, such as the drive was partitioned and */
/*                               formatted with one geometry and then moved to a different machine  */
/*                               which uses a different geometry for the drive.  This would cause   */
/*                               the starting and ending CHS values in the partition tables to      */
/*                               be inconsistent with the size and partition offset entries in the  */
/*                               partition tables.  Setting Ignore_CHS to TRUE will disable the     */
/*                               LVM Engine's CHS consistency checks, thereby allowing the drive    */
/*                               to be partitioned.                                                 */
/*          LVM_Interface_Types Interface_Type - Indicate the type of user interface being used:    */
/*                               PM_Interface, VIO_Interface, or Java_Interface.  This lets the     */
/*                               LVM Engine know which interface support routines to call in any    */
/*                               plugin modules which may be loaded.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in which to store an error code   */
/*                                    should an error occur.                                        */
/*                                                                                                  */
/*   Output:  *Error_Code will be 0 if this function completes successfully.  If an error occurs,   */
/*            *Error_Code will contain a non-zero error code.                                       */
/*                                                                                                  */
/*   Error Handling: If this function aborts with an error, all memory allocated during the course  */
/*                   of this function will be released.  Disk read errors will be reported to the   */
/*                   user via pop-up error messages.  Disk read errors will only cause this         */
/*                   function to abort if none of the disk drives in the system could be            */
/*                   successfully read.                                                             */
/*                                                                                                  */
/*   Side Effects:  The LVM Engine will be initialized.  The partition tables for all OS2DASD       */
/*                  controlled disk drives will be read into memory.  Memory will be allocated for  */
/*                  the data structures used by the LVM Engine.                                     */
/*                                                                                                  */
/*   Notes:  This function is only available in LVM Version 2, and is not available in the current  */
/*           OS/2 implementation.                                                                   */
/*                                                                                                  */
/****************************************************************************************************/
/*
* void _Far16 _Pascal _loadds OPEN_LVM_ENGINE216( BOOLEAN Ignore_CHS, LVM_Interface_Types Interface_Type, CARDINAL32 * _Seg16 Error_Code );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: COMMIT_CHANGES16                                 */
/*                                                                   */
/*   Descriptive Name: Saves any changes made to the partitioning    */
/*                     information of the OS2DASD controlled disk    */
/*                     drives in the system.                         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  The function return value will be TRUE if all of the   */
/*            partitioning/volume changes made were successfully     */
/*            written to disk.  Also, *Error_Code will be 0 if no    */
/*            errors occur.                                          */
/*                                                                   */
/*            If an error occurs, then the furnction return value    */
/*            will be FALSE and *Error_Code will contain a non-zero  */
/*            error code.                                            */
/*                                                                   */
/*   Error Handling:  If an error occurs, the function return value  */
/*                    will be false and *Error_Code will be > 0.     */
/*                                                                   */
/*                    Disk read and write errors will be indicated by*/
/*                    setting the IO_Error field of the              */
/*                    Drive_Information_Record to TRUE.  Thus, if    */
/*                    the function return value is FALSE, and        */
/*                    *Error_Code indicates an I/O error, the caller */
/*                    of this function should call the               */
/*                    Get_Drive_Status function on each drive to     */
/*                    determine which drives had I/O errors.         */
/*                                                                   */
/*                    If a read or write error occurs, then the      */
/*                    engine may not have been able to create a      */
/*                    partition or volume.  Thus, the caller         */
/*                    may want to refresh all partition and volume   */
/*                    data to see what the engine was and was not    */
/*                    able to create.                                */
/*                                                                   */
/*   Side Effects:  The partitioning information of the disk drives  */
/*                  in the system may be altered.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _Far16 _Pascal _loadds COMMIT_CHANGES16( CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: CLOSE_LVM_ENGINE16                               */
/*                                                                   */
/*   Descriptive Name: Closes the LVM Engine and frees any memory    */
/*                     held by the LVM Engine.                       */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects:  Any memory held by the LVM Engine is released.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds CLOSE_LVM_ENGINE16 ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: REFRESH_LVM_ENGINE 16                            */
/*                                                                   */
/*   Descriptive Name: This function causes the LVM Engine to look   */
/*                     for changes in the current system             */
/*                     configuration and update its internal tables  */
/*                     accordingly.                                  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be set to  */
/*                   a non-zero value.                               */
/*                                                                   */
/*   Side Effects: Volumes which represent non-LVM devices may have  */
/*                 their handles changed!                            */
/*                                                                   */
/*   Notes:  After calling this function, Get_Volume_Control_Data    */
/*           should be called to get the updated list of volumes.    */
/*           This is necessary as the handles of some volumes may    */
/*           have changed.                                           */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds  REFRESH_LVM_ENGINE16( CARDINAL32 * _Seg16 Error_Code );

/* ************************************************************************** *

16 Bit Functions relating to Drives

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Get_Drive_Control_Data16                        */
/*                                                                   */
/*   Descriptive Name:  This function returns an array of            */
/*                      Drive_Control_Records.  These records provide*/
/*                      important information about the drives in the*/
/*                      system and provide the handles required to   */
/*                      access them.                                 */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  A Drive_Control_Array structure is returned.  If no    */
/*            errors occur, Drive_Control_Data will be non-NULL,     */
/*            Count will be greater than zero, and *Error_Code will  */
/*            be 0.                                                  */
/*                                                                   */
/*   Error Handling:  If an error occurs, the Drive_Control_Array    */
/*                    returned by this function will have NULL for   */
/*                    Drive_Control_Data, and 0 for Count.           */
/*                    *Error_Code will be greater than 0.            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Drive_Control_Records pointed to by    */
/*           Drive_Control_Data pointer in the Drive_Control_Array   */
/*           structure returned by this function.  The caller should */
/*           free this memory when they are done using it.           */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_DRIVE_CONTROL_DATA16( Drive_Control_Record * _Seg16 * _Seg16 Drive_Control_Data,
CARDINAL32 *                    _Seg16 Count,
CARDINAL32 *                    _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Get_Drive_Status16                              */
/*                                                                   */
/*   Descriptive Name:  Returns the Drive_Information_Record for the */
/*                      drive specified by Drive_Handle.             */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Handle - The handle of the drive to use.*/
/*                             Drive handles are obtained through the*/
/*                             Get_Drive_Control_Data function.      */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns the Drive_Information_Record for  */
/*           the drive associated with the specified Drive_Handle.   */
/*           If no errors occur, *Error_Code will be set to 0.  If an*/
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling:  If an error occurs, then *Error_Code will be   */
/*                    non-zero.                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_DRIVE_STATUS16( CARDINAL32                          Drive_Handle,
Drive_Information_Record *   _Seg16 Drive_Status,
CARDINAL32 *                 _Seg16 Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Partitions

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partitions16                                 */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the object specified by Handle.          */
/*                                                                   */
/*   Input:CARDINAL32 Handle - This is the handle of a drive or      */
/*                      volume.  Drive handles are obtained through  */
/*                      the Get_Drive_Control_Data function.  Volume */
/*                      handles are obtained through the             */
/*                      Get_Volume_Control_Data function.            */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  If     */
/*           Handle is the handle of a disk drive, then the returned */
/*           array will contain a partition information record for   */
/*           each partition and block of free space on that drive.   */
/*           If Handle is the handle of a volume, then the returned  */
/*           array will contain a partition information record for   */
/*           each partition which is part of the specified volume.   */
/*           If no errors occur, then *Error_Code will be 0.  If an  */
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling: Any memory allocated for the return value will  */
/*                   be freed.  The Partition_Information_Array      */
/*                   returned by this function will contain a NULL   */
/*                   pointer for Partition_Array, and have a Count of*/
/*                   0.  *Error_Code will be non-zero.               */
/*                                                                   */
/*                   If Handle is non-NULL and is invalid, a trap    */
/*                   is likely.                                      */
/*                                                                   */
/*   Side Effects:  Memory will be allocated to hold the array       */
/*                  returned by this function.                       */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Partition_Information_Records pointed  */
/*           to by Partition_Array pointer in the                    */
/*           Partition_Information_Array structure returned by this  */
/*           function.  The caller should free this memory when they */
/*           are done using it.                                      */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_PARTITIONS16( CARDINAL32                                      Handle,
Partition_Information_Record * _Seg16 * _Seg16  Partition_Array,
CARDINAL32 *                            _Seg16  Count,
CARDINAL32 *                            _Seg16  Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_PARTITION_HANDLE16                           */
/*                                                                   */
/*   Descriptive Name: Returns the handle of the partition whose     */
/*                     serial number matches the one provided.       */
/*                                                                   */
/*   Input: CARDINAL32 Serial_Number - This is the serial number to  */
/*                                     look for.  If a partition with*/
/*                                     a matching serial number is   */
/*                                     found, its handle will be     */
/*                                     returned.                     */
/*          CARDINAL32 * Handle - The address of a variable (32 bits)*/
/*                                to hold the handle being returned. */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If a partition with a matching serial number is found,  */
/*              then the function return value will be the handle    */
/*              of the partition found.  If no matching partition is */
/*              found, then the function return value will be NULL.  */
/*                                                                   */
/*   Error Handling:  If no errors occur, *Error_Code will be        */
/*                    LVM_ENGINE_NO_ERROR.  If an error occurs, then */
/*                    *Error_Code will be a non-zero error code.     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_PARTITION_HANDLE16( CARDINAL32          Serial_Number,
CARDINAL32 * _Seg16 Handle,
CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partition_Information16                      */
/*                                                                   */
/*   Descriptive Name: Returns the Partition_Information_Record for  */
/*                     the partition specified by Partition_Handle.  */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle associated with */
/*                                     the partition for which the   */
/*                                     Partition_Information_Record  */
/*                                     is desired.                   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A Partition_Information_Record is returned.  If there   */
/*           is no error, then *Error_Code will be 0.  If an error   */
/*           occurs, *Error_Code will be non-zero.                   */
/*                                                                   */
/*   Error Handling:  If the Partition_Handle is not a valid handle, */
/*                    a trap could result.  If it is a handle for    */
/*                    something other than a partition, an error code*/
/*                    will be returned in *Error_Code.               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds  GET_PARTITION_INFORMATION16( CARDINAL32                             Partition_Handle,
Partition_Information_Record * _Seg16  Partition_Information,
CARDINAL32 *                   _Seg16  Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Partition16                               */
/*                                                                   */
/*   Descriptive Name: Creates a partition on a disk drive.          */
/*                                                                   */
/*   Input: CARDINAL32      Handle - The handle of a disk drive or   */
/*                                   a block of free space.          */
/*          CARDINAL32      Size - The size, in sectors, of the      */
/*                                 partition to create.              */
/*          char            Name[] - The name to give to the newly   */
/*                                   created partition.              */
/*          Allocation_Algorithm algorithm - If Handle is a drive,   */
/*                                           then the engine will    */
/*                                           find a block of free    */
/*                                           space to use to create  */
/*                                           the partition.  This    */
/*                                           tells the engine which  */
/*                                           memory management       */
/*                                           algorithm to use.       */
/*          BOOLEAN         Bootable - If TRUE, then the engine will */
/*                                     only create the partition if  */
/*                                     it can be booted from.  If    */
/*                                     Primary_Partition is FALSE,   */
/*                                     then it is assumed that OS/2  */
/*                                     is the operating system that  */
/*                                     will be booted.               */
/*          BOOLEAN         Primary_Partition - If TRUE, then the    */
/*                                              engine will create   */
/*                                              a primary partition. */
/*                                              If FALSE, then the   */
/*                                              engine will create a */
/*                                              logical drive.       */
/*          BOOLEAN         Allocate_From_Start - If TRUE, then the  */
/*                                                engine will        */
/*                                                allocate the new   */
/*                                                partition from the */
/*                                                beginning of the   */
/*                                                selected block of  */
/*                                                free space.  If    */
/*                                                FALSE, then the    */
/*                                                partition will be  */
/*                                                allocated from the */
/*                                                end of the selected*/
/*                                                block of free      */
/*                                                space.             */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function return value will be the handle of the     */
/*           partition created.  If the partition could not be       */
/*           created, then NULL will be returned.                    */
/*           *Error_Code will be 0 if the partition was created.     */
/*           *Error_Code will be > 0 if the partition could not be   */
/*           created.                                                */
/*                                                                   */
/*   Error Handling:  If the partition can not be created, then any  */
/*                    memory allocated by this function will be      */
/*                    freed and the partitioning of the disk in      */
/*                    question will be unchanged.                    */
/*                                                                   */
/*                    If Handle is not a valid handle, then a trap   */
/*                    may result.                                    */
/*                                                                   */
/*                    If Handle represents a partition or volume,    */
/*                    then the function will abort and set           */
/*                    *Error_Code to a non-zero value.               */
/*                                                                   */
/*   Side Effects:  A partition may be created on a disk drive.      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _Far16 _Pascal _loadds CREATE_PARTITION16( CARDINAL32            Handle,
CARDINAL32            Size,
char  *     _Seg16    Name,
Allocation_Algorithm  algorithm,
BOOLEAN               Bootable,
BOOLEAN               Primary_Partition,
BOOLEAN               Allocate_From_Start,
CARDINAL32 * _Seg16   Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Delete_Partition16                               */
/*                                                                   */
/*   Descriptive Name: Deletes the partition specified by            */
/*                     Partition_Handle.                             */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle associated with */
/*                                     the partition to be deleted.  */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition was deleted      */
/*           successfully.  *Error_Code will be > 0 if the partition */
/*           could not be deleted.                                   */
/*                                                                   */
/*   Error Handling: If the partition can not be deleted, then       */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects: A partition on a disk drive may be deleted.       */
/*                                                                   */
/*   Notes:  A partition can not be deleted if it is part of a       */
/*           volume!                                                 */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds DELETE_PARTITION16( CARDINAL32          Partition_Handle,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Active_Flag16                                */
/*                                                                   */
/*   Descriptive Name: Sets the Active Flag field for a partition.   */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle of the partition*/
/*                                     whose Active Flag is to be set*/
/*          BYTE Active_Flag - The new value for the Active Flag.    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the Active Flag was            */
/*           successfully set, otherwise *Error_Code will contain a  */
/*           non-zero error code indicating what went wrong.         */
/*                                                                   */
/*   Error Handling: If the Active Flag can not be set, this function*/
/*                   will abort without changing any disk structures.*/
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                                                                   */
/*   Side Effects:  The Active Flag for a partition may be modified. */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_ACTIVE_FLAG16 ( CARDINAL32          Partition_Handle,
BYTE                Active_Flag,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_OS_Flag16                                    */
/*                                                                   */
/*   Descriptive Name: Sets the OS Flag field for a partition.  This */
/*                     field is typically used to indicate the       */
/*                     filesystem used on the partition, which       */
/*                     generally gives an indication of which OS is  */
/*                     using that partition.                         */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle of the partition*/
/*                                     whose Active Flag is to be set*/
/*          BYTE OS_Flag - The new value for the OS Flag.            */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the OS Flag was successfully   */
/*           set, otherwise *Error_Code will contain a non-zero error*/
/*           code indicating what went wrong.                        */
/*                                                                   */
/*   Error Handling: If the OS Flag can not be set, this function    */
/*                   will abort without changing any disk structures.*/
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                                                                   */
/*   Side Effects:  The OS Flag for a partition may be modified.     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_OS_FLAG16 ( CARDINAL32          Partition_Handle,
BYTE                OS_Flag,
CARDINAL32 * _Seg16 Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Volumes

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Volume_Control_Data16                        */
/*                                                                   */
/*   Descriptive Name: This function returns a structure containing  */
/*                     an array of Volume_Control_Records.  These    */
/*                     records contain information about volumes     */
/*                     which is invariant - i.e. will not change for */
/*                     as long as the volume exists.  One of the     */
/*                     items in the Volume_Control_Record is the     */
/*                     handle for the volume.  This handle must be   */
/*                     used on all accesses to the volume.           */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A Volume_Control_Array structure is returned.           */
/*                                                                   */
/*           If there are no errors, then the Volume_Control_Data    */
/*           pointer in the Volume_Control_Array will be non-NULL,   */
/*           the Count field of the Volume_Control_Array will be     */
/*           >= 0, and *Error_Code will be 0.                        */
/*                                                                   */
/*           If an error does occur, then the Volume_Control_Data    */
/*           pointer in the the Volume_Control_Array will be NULL,   */
/*           the Count field of the Volume_Control_Array will be 0,  */
/*           and *Error_Code will be > 0.                            */
/*                                                                   */
/*   Error Handling: If an error occurs, then any memory allocated by*/
/*                   this function will be freed.                    */
/*                                                                   */
/*   Side Effects:  Memory for the returned array is allocated.      */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Volume_Control_Records pointed to by   */
/*           Volume_Control_Data pointer in the Volume_Control_Array */
/*           structure returned by this function.  The caller should */
/*           free this memory when they are done using it.           */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_VOLUME_CONTROL_DATA16( Volume_Control_Record  *  _Seg16 * _Seg16 Volume_Control_Data,
CARDINAL32 *                       _Seg16 Count,
CARDINAL32 *                       _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Volume_Information16                         */
/*                                                                   */
/*   Descriptive Name:  This function returns the                    */
/*                      Volume_Information_Record for the volume     */
/*                      associated with Volume_Handle.               */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume about*/
/*                                  which information is desired.    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a Volume_Information_Record.      */
/*                                                                   */
/*           If this function is successful, then *Error_Code will be*/
/*              0.                                                   */
/*                                                                   */
/*           If this function fails, then *Error_Code will be > 0.   */
/*                                                                   */
/*   Error Handling:  If Volume_Handle is not a valid handle, a trap */
/*                    will be likely.  If Volume_Handle is a drive or*/
/*                    partition handle, *Error_Code will be > 0.     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_VOLUME_INFORMATION16( CARDINAL32                           Volume_Handle,
Volume_Information_Record  * _Seg16  Volume_Information,
CARDINAL32 *                 _Seg16  Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: CREATE_VOLUME16                                  */
/*                                                                   */
/*   Descriptive Name:  This function creates a volume from a list of*/
/*                      partitions.  The partitions are specified by */
/*                      their corresponding handles.                 */
/*                                                                   */
/*   Input: char         Name[] - The name to assign to the newly    */
/*                                created volume.                    */
/*          BOOLEAN      Create_LVM_Volume - If TRUE, then an LVM    */
/*                                           volume is created,      */
/*                                           otherwise a             */
/*                                           compatibility volume is */
/*                                           created.                */
/*          BOOLEAN      Bootable - If TRUE, the volume will not be  */
/*                                  created unless OS/2 can be booted*/
/*                                  from it.                         */
/*          char         Drive_Letter_Preference - This is the drive */
/*                                                 letter to use for */
/*                                                 accessing the     */
/*                                                 newly created     */
/*                                                 volume.           */
/*          CARDINAL32   FeaturesToUse - This is currently reserved  */
/*                                       for future use and should   */
/*                                       always be set to 0.         */
/*          CARDINAL32   Partition_Count - The number of partitions  */
/*                                         to link together to form  */
/*                                         the volume being created. */
/*          ADDRESS      Partition_Handles[] - An array of partition */
/*                                             handles with one entry*/
/*                                             for each partition    */
/*                                             that is to become part*/
/*                                             of the volume being   */
/*                                             created.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was created.        */
/*           *Error_Code will be > 0 if the volume could not be      */
/*              created.                                             */
/*                                                                   */
/*   Error Handling: If any of the handles in the partition handles  */
/*                   array is not valid, then a trap is likely.  If  */
/*                   Partition_Count is greater than the number of   */
/*                   entries in the partition handles array, then a  */
/*                   trap is likely.  If any of the handles in the   */
/*                   partition array are not partition handles, then */
/*                   *Error_Code will be > 0.  If the volume can NOT */
/*                   be created, then *Error_Code will be > 0 and any*/
/*                   memory allocated by this function will be freed.*/
/*                   If the volume can NOT be created, then the      */
/*                   existing partition/volume structure of the disk */
/*                   will be unchanged.                              */
/*                                                                   */
/*   Side Effects:  A volume may be created.                         */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds CREATE_VOLUME16( char   *     _Seg16 Name,
BOOLEAN             Create_LVM_Volume,
BOOLEAN             Bootable,
char                Drive_Letter_Preference,
CARDINAL32          FeaturesToUse,
CARDINAL32          Partition_Count,
CARDINAL32 * _Seg16 Partition_Handles,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: CREATE_VOLUME216                                 */
/*                                                                   */
/*   Descriptive Name:  This function creates a volume from a list of*/
/*                      partitions.  The partitions are specified by */
/*                      their corresponding handles.                 */
/*                                                                   */
/*   Input: char         Name[] - The name to assign to the newly    */
/*                                created volume.                    */
/*          BOOLEAN      Create_LVM_Volume - If TRUE, then an LVM    */
/*                                           volume is created,      */
/*                                           otherwise a             */
/*                                           compatibility volume is */
/*                                           created.                */
/*          BOOLEAN      Bootable - If TRUE, the volume will not be  */
/*                                  created unless OS/2 can be booted*/
/*                                  from it.                         */
/*          char         Drive_Letter_Preference - This is the drive */
/*                                                 letter to use for */
/*                                                 accessing the     */
/*                                                 newly created     */
/*                                                 volume.           */
/*          CARDINAL32   Feature_Count - The number of features to   */
/*                                       install on the volume being */
/*                                       created.  This field is     */
/*                                       ignored if Create_LVM_Volume*/
/*                                       is FALSE.                   */
/*          LVM_Feature_Specification_Record FeaturesToUse[] - An    */
/*                                         array of feature IDs and  */
/*                                         their associated LVM      */
/*                                         classes used to designate */
/*                                         which features to install */
/*                                         on the volume being       */
/*                                         created and the order in  */
/*                                         which to install them.    */
/*                                         This field is ignored if  */
/*                                         Create_LVM_Volume is      */
/*                                         FALSE.                    */
/*          CARDINAL32   Partition_Count - The number of partitions  */
/*                                         to link together to form  */
/*                                         the volume being created. */
/*          ADDRESS      Partition_Handles[] - An array of partition */
/*                                             handles with one entry*/
/*                                             for each partition    */
/*                                             that is to become part*/
/*                                             of the volume being   */
/*                                             created.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was created.        */
/*           *Error_Code will be > 0 if the volume could not be      */
/*              created.                                             */
/*                                                                   */
/*   Error Handling: If any of the handles in the partition handles  */
/*                   array is not valid, then a trap is likely.  If  */
/*                   Partition_Count is greater than the number of   */
/*                   entries in the partition handles array, then a  */
/*                   trap is likely.  If any of the handles in the   */
/*                   partition array are not partition handles, then */
/*                   *Error_Code will be > 0.  If the volume can NOT */
/*                   be created, then *Error_Code will be > 0 and any*/
/*                   memory allocated by this function will be freed.*/
/*                   If the volume can NOT be created, then the      */
/*                   existing partition/volume structure of the disk */
/*                   will be unchanged.                              */
/*                                                                   */
/*   Side Effects:  A volume may be created.                         */
/*                                                                   */
/*   Notes:  This function is only available in LVM Version 2, and   */
/*           is not available in the current OS/2 implementation.    */
/*                                                                   */
/*********************************************************************/
/*
* void _Far16 _Pascal _loadds  CREATE_VOLUME216( char   * _Seg16                            Name,
* BOOLEAN                                    Create_LVM_Volume,
* BOOLEAN                                    Bootable,
* char                                       Drive_Letter_Preference,
* CARDINAL32                                 Feature_Count,
* LVM_Feature_Specification_Record * _Seg16  FeaturesToUse,
* CARDINAL32                                 Partition_Count,
* CARDINAL32 * _Seg16                        Partition_Handles,
* CARDINAL32 * _Seg16                        Error_Code
* );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: Delete_Volume16                                  */
/*                                                                   */
/*   Descriptive Name: Deletes the volume specified by Volume_Handle.*/
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to   */
/*                                  delete.  All partitions which are*/
/*                                  part of the specified volume will*/
/*                                  be deleted also.                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume and its partitions  */
/*           are successfully deleted.  Otherwise, *Error_Code will  */
/*           be > 0.                                                 */
/*                                                                   */
/*   Error Handling: *Error_Code will be > 0 if an error occurs.  If */
/*                   the volume or any of its partitions can not be  */
/*                   deleted, then any changes made by this function */
/*                   will be undone.                                 */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  A volume and its partitions may be deleted.      */
/*                  System memory may be freed as the internal       */
/*                  structures used to track the deleted volume      */
/*                  are no longer required.                          */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds DELETE_VOLUME16( CARDINAL32          Volume_Handle,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Hide_Volume16                                    */
/*                                                                   */
/*   Descriptive Name: Hide volume "hides" a volume from OS/2 by     */
/*                     removing its drive letter assignment.  Without*/
/*                     a drive letter assignment, OS/2 can not access*/
/*                     (or "see") the volume.                        */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to   */
/*                                     hide.                         */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was successfully    */
/*           hidden.  If the volume could not be hidden, then        */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: *Error_Code will be > 0 if the volume can not be*/
/*                   hidden.  If the volume can not be hidden, then  */
/*                   nothing will be altered.                        */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds HIDE_VOLUME16( CARDINAL32          Volume_Handle,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Expand_Volume16                                  */
/*                                                                   */
/*   Descriptive Name: This function expands an existing volume by   */
/*                     linking additional partitions to it.          */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to be*/
/*                                  expanded.                        */
/*          CARDINAL32 Partition_Count - The number of partitions or */
/*                                       volumes to be added to the  */
/*                                       volume being expanded.      */
/*          CARDINAL32 Partition_Handles[] - An array of handles.    */
/*                                        Each handle in the array is*/
/*                                        the handle of a partition  */
/*                                        which is to be added to    */
/*                                        the volume being expanded. */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume is successfully     */
/*           expanded.  If the volume can not be expanded,           */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If the volume can not be expanded, the state of */
/*                   the volume is unchanged and any memory allocated*/
/*                   by this function is freed.                      */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                   If any of the partition handles in the          */
/*                   Partition_handles array are not valid handles,  */
/*                   then a trap may result.                         */
/*                                                                   */
/*                   If any of the partition handles in the          */
/*                   Partition_Handles array are actually drive      */
/*                   handles, then this function will abort and      */
/*                   set *Error_Code to a non-zero value.            */
/*                                                                   */
/*                   If Partition_Count is greater than the number of*/
/*                   entries in the Partition_Handles array, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*   Side Effects:  A volume may be expanded.  If the volume is      */
/*                  expanded using another volume, the partitions    */
/*                  on the second volume will be linked to those of  */
/*                  the first volume and all data on the second      */
/*                  volume will be lost.                             */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds EXPAND_VOLUME16 ( CARDINAL32               Volume_Handle,
CARDINAL32               Partition_Count,
CARDINAL32    * _Seg16   Partition_Handles,
CARDINAL32 * _Seg16      Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Assign_Drive_Letter16                            */
/*                                                                   */
/*   Descriptive Name: Assigns a drive letter to a volume.           */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume which*/
/*                                  is to have its assigned drive    */
/*                                  letter changed.                  */
/*          char  New_Drive_Preference - The new drive letter to     */
/*                                       assign to the volume.       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the drive letter was assigned  */
/*           successfully; otherwise *Error_Code will be > 0.        */
/*                                                                   */
/*   Error Handling: If the drive letter assignment can not be made, */
/*                   the volume will not be altered.                 */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  A volume may have its drive letter assignment    */
/*                  changed.                                         */
/*                                                                   */
/*   Notes:  If the drive letter being assigned is already in use by */
/*           volume which does not lie on removable media, then the  */
/*           drive assignment will NOT be made.                      */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds ASSIGN_DRIVE_LETTER16( CARDINAL32          Volume_Handle,
char                New_Drive_Preference,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Installable16                                */
/*                                                                   */
/*   Descriptive Name: Marks a volume as being the volume to install */
/*                     OS/2 on.                                      */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to   */
/*                                  which OS/2 should be installed.  */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the volume is successfully marked as installable,    */
/*           *Error_Code will be 0; otherwise *Error_Code will       */
/*           be > 0.                                                 */
/*                                                                   */
/*   Error Handling: If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  The specified volume may be marked as            */
/*                  installable.                                     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_INSTALLABLE16 ( CARDINAL32          Volume_Handle,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Installable_Volume16                         */
/*                                                                   */
/*   Descriptive Name: Marks a volume as being the volume to install */
/*                     OS/2 on.                                      */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If a volume is mared installable, its information will  */
/*           be returned and *Error_Code will be LVM_ENGINE_NO_ERROR.*/
/*           If there is no volume marked installable, then          */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: An error code is returned if there is an error. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_INSTALLABLE_VOLUME16 ( Volume_Information_Record * _Seg16  Volume_Information,
CARDINAL32 *                _Seg16  Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Partitions, Drives, and Volumes.

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Name16                                       */
/*                                                                   */
/*   Descriptive Name: Sets the name of a volume, drive, or partition*/
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the drive, partition,  */
/*                           or volume which is to have its name set.*/
/*          char New_Name[] - The new name for the drive/partition/  */
/*                            volume.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the name is set as specified.  */
/*           If the name can not be set, *Error_Code will be > 0.    */
/*                                                                   */
/*   Error Handling: If the name can not be set, then drive/volume/  */
/*                   partition is not modified.                      */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*   Side Effects:  A drive/volume/partition may have its name set.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_NAME16 ( CARDINAL32          Handle,
char       * _Seg16 New_Name,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_STARTABLE16                                  */
/*                                                                   */
/*   Descriptive Name: Sets the specified volume or partition        */
/*                     startable.  If a volume is specified, it must */
/*                     be a compatibility volume whose partition is  */
/*                     a primary partition on the first drive.  If a */
/*                     partition is specified, it must be a primary  */
/*                     partition on the first drive in the system.   */
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the partition or volume*/
/*                           which is to be set startable.           */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the specified volume or        */
/*           partition was set startable.                            */
/*           If the name can not be set, *Error_Code will be > 0.    */
/*                                                                   */
/*   Error Handling: If the volume or partition could not be set     */
/*                   startable, then nothing in the system is        */
/*                   changed.                                        */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*   Side Effects:  Any other partition or volume which is marked    */
/*                  startable will have its startable flag cleared.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void  _Far16 _Pascal _loadds SET_STARTABLE16 ( CARDINAL32           Handle,
CARDINAL32 *  _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Valid_Options16                              */
/*                                                                   */
/*   Descriptive Name: Returns a bitmap where each bit in the bitmap */
/*                     corresponds to a possible operation that the  */
/*                     LVM Engine can perform.  Those bits which are */
/*                     1 represent operations which can be performed */
/*                     on the item specified by Handle.  Those bits  */
/*                     which are 0 are not allowed on the item       */
/*                     specified by Handle.                          */
/*                                                                   */
/*   Input: CARDINAL32 Handle - This is any valid drive, volume, or  */
/*                           partition handle.                       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output:  A bitmap indicating which operations are valid on the  */
/*            item specified by Handle.                              */
/*                                                                   */
/*            If no errors occur, *Error_Code will be 0, otherwise   */
/*            *Error_Code will be > 0.                               */
/*                                                                   */
/*   Error Handling:  If Handle is not valid, a trap will be likely. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The values of the various bits in the bitmap returned   */
/*           by this function are defined near the beginning of this */
/*           file, immediately after all of the structure            */
/*           definitions.                                            */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _Far16 _Pascal _loadds GET_VALID_OPTIONS16( CARDINAL32          Handle,
CARDINAL32 * _Seg16 Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Boot Manager

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: BOOT_MANAGER_IS_INSTALLED16                      */
/*                                                                   */
/*   Descriptive Name: Indicates whether or not Boot Manager is      */
/*                     installed on the first or second hard drives  */
/*                     in the system.                                */
/*                                                                   */
/*   Input: BOOLEAN * Active - *Active is set to TRUE if LVM found an*/
/*                             active copy of Boot Manager on the    */
/*                             system.  If LVM could not find an     */
/*                             active copy of Boot Manager on the    */
/*                             system, but did find an inactive copy */
/*                             of Boot Manager, then *Active will be */
/*                             set to FALSE.                         */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: TRUE is returned if Boot Manager is found.  If this     */
/*           copy of Boot Manager is Active, then *Active will be set*/
/*           to TRUE.  If the copy of Boot Manager is not currently  */
/*           active, then *Active will be set to FALSE.              */
/*                                                                   */
/*           FALSE is returned if Boot Manager is not found or if an */
/*           error occurs.  In this case, *Active is undefined.      */
/*                                                                   */
/*           *Error_Code will be 0 if no errors occur; otherwise it  */
/*           will be > 0.                                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _Far16 _Pascal _loadds BOOT_MANAGER_IS_INSTALLED16( BOOLEAN * _Seg16 Active, CARDINAL32 * _Seg16 Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_BOOT_MANAGER_HANDLE16                        */
/*                                                                   */
/*   Descriptive Name: Returns the handle of the partition containing*/
/*                     Boot Manager.                                 */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If Boot Manager is NOT installed, NULL is returned.     */
/*           If Boot Manager is installed, whether it is active or   */
/*           not, the handle of the partition it resides in is       */
/*           returned.                                               */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:                                                          */
/*                                                                   */
/*********************************************************************/
ADDRESS _Far16 _Pascal _loadds GET_BOOT_MANAGER_HANDLE16( CARDINAL32 * _Seg16 Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Add_To_Boot_Manager16                            */
/*                                                                   */
/*   Descriptive Name: Adds the volume/partition to the Boot Manager */
/*                     menu.                                         */
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of a partition or volume  */
/*                      that is to be added to the Boot Manager menu.*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition or volume was    */
/*           successfully added to the Boot Manager menu; otherwise  */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If the partition/volume can not be added to the */
/*                   Boot Manager menu, no action is taken and       */
/*                   *Error_Code will contain a non-zero error code. */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*                   If Handle represents a drive, then this function*/
/*                   will abort and set *Error_Code to a non-zero    */
/*                   value.                                          */
/*                                                                   */
/*   Side Effects:  The Boot Manager menu may be altered.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds ADD_TO_BOOT_MANAGER16 ( CARDINAL32 Handle, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Remove_From_Boot_Manager16                       */
/*                                                                   */
/*   Descriptive Name: Removes the specified partition or volume     */
/*                     from the Boot Manager menu.                   */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition or volume was    */
/*           successfully removed to the Boot Manager menu;          */
/*           otherwise *Error_Code will be > 0.                      */
/*                                                                   */
/*   Error Handling: If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*                   If Handle represents a drive, or if Handle      */
/*                   represents a volume or partition which is NOT on*/
/*                   the boot manager menu, then this function       */
/*                   will abort and set *Error_Code to a non-zero    */
/*                   value.                                          */
/*                                                                   */
/*   Side Effects:  The Boot Manager menu may be altered.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds REMOVE_FROM_BOOT_MANAGER16 ( CARDINAL32 Handle, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Boot_Manager_Menu16                          */
/*                                                                   */
/*   Descriptive Name: Returns an array containing the handles of the*/
/*                     partitions and volumes appearing on the       */
/*                     Boot Manager menu.                            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function returns a Boot_Manager_Menu structure.     */
/*           This structure contains two items: a pointer to an array*/
/*           of Boot_Manager_Menu_Items and a count of how many items*/
/*           are in the array.  Each Boot_Manager_Menu_Item contains */
/*           a handle and a BOOLEAN variable to indicate whether the */
/*           handle is for a partition or a volume.                  */
/*                                                                   */
/*           If this function is successful, then *Error_Code will   */
/*           be 0.                                                   */
/*                                                                   */
/*           If an error occurs, the Count field in the              */
/*           Boot_Manager_Menu will be 0 and the corresponding       */
/*           pointer will be NULL.  *Error_Code will be > 0.         */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                   any memory allocated by this function will be   */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_BOOT_MANAGER_MENU16 ( Boot_Manager_Menu_Item * _Seg16 * _Seg16 Menu_Items,
CARDINAL32 *                      _Seg16 Count,
CARDINAL32 *                      _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Install_Boot_Manager16                           */
/*                                                                   */
/*   Descriptive Name: This function installs Boot Manager.  It can  */
/*                     be used to replace an existing Boot Manager   */
/*                     as well.                                      */
/*                                                                   */
/*   Input: CARDINAL32  Drive_Number - The number of the drive to    */
/*                                     install Boot Manager on.  Must*/
/*                                     be 1 or 2.                    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If this function is successful, then *Error_Code will be*/
/*           0; otherwise it will be > 0.                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be set to a*/
/*                   non-zero value.  Depending upon the error, it   */
/*                   is possible that the Boot Manager partition can */
/*                   be left in an unusuable state (such as for a    */
/*                   write error).                                   */
/*                                                                   */
/*   Side Effects: Boot Manager may be installed on drive 1 or 2.    */
/*                 The MBR for drive 1 may be altered.               */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds INSTALL_BOOT_MANAGER16 ( CARDINAL32   Drive_Number, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Remove_Boot_Manager16                            */
/*                                                                   */
/*   Descriptive Name: Removes Boot Manager from the system.         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if Boot Manager was successfully  */
/*           removed from the system; otherwise *Error_Code will     */
/*           be 0.                                                   */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  Boot Manager will be removed from the system.    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds REMOVE_BOOT_MANAGER16( CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Boot_Manager_Options16                       */
/*                                                                   */
/*   Descriptive Name: Sets the Boot Managers Options.  The options  */
/*                     that can be set are: whether or not the time- */
/*                     out timer is active, how long the timer-out   */
/*                     is, the partition to boot by default, and     */
/*                     whether or not Boot Manager should display its*/
/*                     menu using default mode or advanced mode.     */
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the partition or volume*/
/*                           to boot if the time-out timer is active */
/*                           and the time-out value is reached.      */
/*          BOOLEAN Timer_Active - If TRUE, then the time-out timer  */
/*                                 is active.                        */
/*          CARDINAL32 Time_Out_Value - If the time-out timer is     */
/*                                      active, this is the time-out */
/*                                      value, in seconds.           */
/*          BOOLEAN Advanced_Mode - If TRUE, then Boot Manager will  */
/*                                  operate in advanced mode.  If    */
/*                                  FALSE, then normal mode will be  */
/*                                  in effect.                       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if no errors occur.  If an error  */
/*           does occur, then *Error_Code will be > 0.               */
/*                                                                   */
/*   Error Handling: If an error occurs, no changes will be made to  */
/*                   Boot Manager and *Error_Code will be set a      */
/*                   non-zero error code.                            */
/*                                                                   */
/*   Side Effects:  Boot Manager may be modified.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_BOOT_MANAGER_OPTIONS16( CARDINAL32             Handle,
BOOLEAN             Timer_Active,
CARDINAL32          Time_Out_Value,
BOOLEAN             Advanced_Mode,
CARDINAL32 * _Seg16 Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Boot_Manager_Options16                       */
/*                                                                   */
/*   Descriptive Name: This function returns the current Boot Manager*/
/*                     settings for the various Boot Manager options.*/
/*                                                                   */
/*   Input: CARDINAL32 * Handle - The handle for the default boot    */
/*                             volume or partition.                  */
/*          BOOLEAN * Handle_Is_Volume - If TRUE, then Handle        */
/*                                       represents a volume.  If    */
/*                                       FALSE, then Handle          */
/*                                       represents a partition.     */
/*          BOOLEAN * Timer_Active - If TRUE, then the time-out timer*/
/*                                   is active.  If FALSE, then the  */
/*                                   time-out timer is not active.   */
/*          CARDINAL32 * Time_Out_Value - If the time-out timer is   */
/*                                        active, then this is the   */
/*                                        number of seconds that Boot*/
/*                                        Manager will wait for user */
/*                                        input before booting the   */
/*                                        default volume/partition.  */
/*          BOOLEAN * Advanced_Mode - If TRUE, the Boot Manager is   */
/*                                    operating in advanced mode.  If*/
/*                                    FALSE, then Boot Manager is    */
/*                                    operating in normal mode.      */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Handle, *Handle_Is_Volume, *Timer_Active,              */
/*           *Time_out_value, *Advanced_Mode, and *Error_Code are all*/
/*           set by this function.  If there are no errors, then     */
/*           *Error_Code will be set to 0.  If any errors occur, then*/
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If any of the parameters are invalid, then a    */
/*                   trap is likely.  If Boot Manager is not         */
/*                   installed, then *Error_Code will be > 0.        */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds GET_BOOT_MANAGER_OPTIONS16( CARDINAL32 * _Seg16 Handle,
BOOLEAN    * _Seg16 Handle_Is_Volume,
BOOLEAN    * _Seg16 Timer_Active,
CARDINAL32 * _Seg16 Time_Out_Value,
BOOLEAN    * _Seg16 Advanced_Mode,
CARDINAL32 * _Seg16 Error_Code
);

/* ************************************************************************** *

Other 16 Bit Functions

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  Free_Engine_Memory16                            */
/*                                                                   */
/*   Descriptive Name: Frees a memory object created by LVM.DLL and  */
/*                     returned to a user of LVM.DLL.                */
/*                                                                   */
/*   Input: ADDRESS Object : The address of the memory object to     */
/*                           free.  This could be the                */
/*                           Drive_Control_Data field of a           */
/*                           Drive_Control_Record, the               */
/*                           Partition_Array field of a              */
/*                           Partition_Information_Array structure,  */
/*                           or any other dynamically allocated      */
/*                           memory object created by LVM.DLL and    */
/*                           returned by a function in LVM.DLL.      */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  A trap or exception could occur if a bad address is     */
/*           passed into this function.                              */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds FREE_ENGINE_MEMORY16( ADDRESS _Seg16 Object );

/*********************************************************************/
/*                                                                   */
/*   Function Name: New_MBR16                                        */
/*                                                                   */
/*   Descriptive Name: This function lays down a new MBR on the      */
/*                     specified drive.                              */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Handle - The handle of the drive on     */
/*                                 which the new MBR is to be placed.*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the new MBR was successfully   */
/*           placed on the specified drive.  If the operation failed */
/*           for any reason, then *Error_Code will contain a non-zero*/
/*           error code.                                             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the existing MBR is not*/
/*                   altered and *Error_Code will be > 0.            */
/*                                                                   */
/*   Side Effects:  A new MBR may be placed on the specified drive.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds NEW_MBR16( CARDINAL32 Drive_Handle, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Available_Drive_Letters16                    */
/*                                                                   */
/*   Descriptive Name: This function returns a bitmap indicating     */
/*                     which drive letters are available for use.    */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the available drive   */
/*           letters.  If this function is successful, then          */
/*           *Error_Code will be set to 0.  Otherwise, *Error_Code   */
/*           will be > 0 and the bitmap returned will have all bits  */
/*           set to 0.                                               */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  A drive letter is available if it is not associated     */
/*           with a volume located on a disk drive controlled        */
/*           by OS2DASD.                                             */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _Far16 _Pascal _loadds GET_AVAILABLE_DRIVE_LETTERS16 ( CARDINAL32 * _Seg16 Error_Code ) ;

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_RESERVED_DRIVE_LETTERS16                     */
/*                                                                   */
/*   Descriptive Name: This function returns a bitmap indicating     */
/*                     which drive letters are reserved for use by   */
/*                     devices NOT under the control of LVM.         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the drive letters     */
/*           which are being used by devices which are NOT controlled*/
/*           by LVM.  While a Volume CAN be assigned a drive letter  */
/*           from this list, a reboot will almost always be required */
/*           in order for the assignment to take place.              */
/*           If this function is successful, then *Error_Code will be*/
/*           set to 0.  Otherwise, *Error_Code will be > 0 and the   */
/*           bitmap returned will have all bits set to 0.            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  Devices which are assigned drive letters but which are  */
/*           NOT under LVM control include:  CD-ROM, Network drives, */
/*           parallel port attached devices, and any DASD devices    */
/*           not controlled by OS2DASD.                              */
/*                                                                   */
/*********************************************************************/
CARDINAL32 _Far16 _Pascal _loadds GET_RESERVED_DRIVE_LETTERS16 ( CARDINAL32 *  _Seg16 Error_Code ) ;

/*********************************************************************/
/*                                                                   */
/*   Function Name: Reboot_Required16                                */
/*                                                                   */
/*   Descriptive Name: This function indicates whether or not any    */
/*                     changes were made to the partitioning of the  */
/*                     disks in the system which would require a     */
/*                     reboot to make functional.                    */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value will be TRUE if the system    */
/*           must be rebooted as a result of disk partitioning       */
/*           changes.                                                */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _Far16 _Pascal _loadds REBOOT_REQUIRED16 ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: CHANGES_PENDING16                                */
/*                                                                   */
/*   Descriptive Name: This function indicates whether or not any    */
/*                     changes were made to the partitioning of the  */
/*                     disks in the system which have not yet been   */
/*                     comitted to disk.                             */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value will be TRUE if there are     */
/*           uncomitted changes to the partitioning of one or more of*/
/*           the drives in the system.                               */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _Far16 _Pascal _loadds CHANGES_PENDING16 ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Reboot_Flag16                                */
/*                                                                   */
/*   Descriptive Name: This function sets the Reboot Flag.  The      */
/*                     Reboot Flag is a special flag on the boot     */
/*                     disk used by the install program to keep      */
/*                     track of whether or not the system was just   */
/*                     rebooted.  It is used by the various phases   */
/*                     of install.                                   */
/*                                                                   */
/*   Input: BOOLEAN Reboot - The new value for the Reboot Flag.  If  */
/*                           TRUE, then the reboot flag will be set. */
/*                           If FALSE, then the reboot flag will be  */
/*                           cleared.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to 0 if there are no errors.    */
/*           *Error_Code will be > 0 if an error occurs.             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the value of the Reboot*/
/*                   Flag will be unchanged.                         */
/*                                                                   */
/*   Side Effects:  The value of the Reboot Flag may be changed.     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_REBOOT_FLAG16( BOOLEAN Reboot, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Reboot_Flag16                                */
/*                                                                   */
/*   Descriptive Name: This function returns the value of the Reboot */
/*                     Flag.  The Reboot Flag is a special flag on   */
/*                     the boot disk used by the install program to  */
/*                     keep track of whether or not the system was   */
/*                     just rebooted.  It is used by the various     */
/*                     phases of install.                            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function return value will be TRUE if no errors     */
/*           occur and the Reboot Flag is set.  *Error_Code will be  */
/*           0 under these conditions.  If an error occurs, the      */
/*           function return value will be FALSE and *Error_Code     */
/*           will be > 0.                                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                   The value of the reboot flag will be unchanged. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN _Far16 _Pascal _loadds GET_REBOOT_FLAG16( CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*   Function Name: Set_Install_Flags16                              */
/*                                                                   */
/*   Descriptive Name: This function sets the value of the Install   */
/*                     Flags.  The Install Flags reside in a 32 bit  */
/*                     field in the LVM dataspace.  These flags are  */
/*                     not used by LVM, thereby leaving Install free */
/*                     to use them for whatever it wants.            */
/*                                                                   */
/*   Input: CARDINAL32 Install_Flags - The new value for the Install */
/*                                     Flags.                        */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to 0 if there are no errors.    */
/*           *Error_Code will be > 0 if an error occurs.             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the value of the       */
/*                   Install Flags will be unchanged.                */
/*                                                                   */
/*   Side Effects:  The value of the Install Flags may be changed.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds Set_Install_Flags16( CARDINAL32 Install_Flags, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Install_Flags16                              */
/*                                                                   */
/*   Descriptive Name: This function returns the value of the Install*/
/*                     Flags.  The Install Flags reside in a 32 bit  */
/*                     field in the LVM dataspace.  These flags are  */
/*                     not used by LVM, thereby leaving Install free */
/*                     to use them for whatever it wants.            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function returns the current value of the Install   */
/*           Flags stored in the LVM Dataspace.                      */
/*           *Error_Code will be LVM_ENGINE_NO_ERROR if the function */
/*           is successful.  If an error occurs, the function will   */
/*           return 0 and *Error_Code will be > 0.                   */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds Get_Install_Flags16( CARDINAL32 * _Seg16 Install_Flags, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Min_Install_Size16                           */
/*                                                                   */
/*   Descriptive Name: This function tells the LVM Engine how big a  */
/*                     partition/volume must be in order for it to   */
/*                     marked installable.  If this function is not  */
/*                     used to set the minimum size for an           */
/*                     installable partition/volume, the LVM Engine  */
/*                     will use a default value of 300 MB.           */
/*                                                                   */
/*   Input: CARDINAL32 Min_Sectors - The minimum size, in sectors,   */
/*                                   that a partition must be in     */
/*                                   order for it to be marked as    */
/*                                   installable.                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_MIN_INSTALL_SIZE16 ( CARDINAL32  Min_Sectors );

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_FREE_SPACE_THRESHOLD16                       */
/*                                                                   */
/*   Descriptive Name: This function tells the LVM Engine not to     */
/*                     report blocks of free space which are less    */
/*                     than the size specified.  The engine defaults */
/*                     to not reporting blocks of free space which   */
/*                     are smaller than 2048 sectors (1 MB).         */
/*                                                                   */
/*   Input: CARDINAL32 Min_Sectors - The minimum size, in sectors,   */
/*                                   that a block of free space must */
/*                                   be in order for the LVM engine  */
/*                                   to report it.                   */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds SET_FREE_SPACE_THRESHOLD16( CARDINAL32  Min_Sectors );

/*********************************************************************/
/*                                                                   */
/*   Function Name: READ_SECTORS16                                   */
/*                                                                   */
/*   Descriptive Name: This function reads one or more sectors from  */
/*                     the specified drive and places the data read  */
/*                     in Buffer.                                    */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    read from.  The drives in the  */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to read from.     */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       read into memory.           */
/*          ADDRESS Buffer : The location to put the data read into. */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, then the data read will be placed in     */
/*              memory starting at Buffer, and *Error will be        */
/*              LVM_ENGINE_NO_ERROR.                                 */
/*           If Unsuccessful, then *Error will be > 0 and the        */
/*              contents of memory starting at Buffer is undefined.  */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be read into memory starting at Buffer.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds READ_SECTORS16 ( CARDINAL32          Drive_Number,
LBA                 Starting_Sector,
CARDINAL32          Sectors_To_Read,
ADDRESS      _Seg16 Buffer,
CARDINAL32 * _Seg16 Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name: WRITE_SECTORS16                                  */
/*                                                                   */
/*   Descriptive Name: This function writes data from memory to one  */
/*                     or more sectors on the specified drive.       */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    write to.  The drives in the   */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to write to.      */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       be written.                 */
/*          ADDRESS Buffer : The location of the data to be written  */
/*                           to disk.                                */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, then the data at Buffer will be placed   */
/*              on the disk starting at the sector specified, and    */
/*              *Error will be LVM_ENGINE_NO_ERROR.                  */
/*           If Unsuccessful, then *Error will be > 0 and the        */
/*              contents of the disk starting at sector              */
/*              Starting_Sector is undefined.                        */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be written to disk.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds WRITE_SECTORS16 ( CARDINAL32          Drive_Number,
LBA                 Starting_Sector,
CARDINAL32          Sectors_To_Write,
ADDRESS      _Seg16 Buffer,
CARDINAL32 * _Seg16 Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name: REDISCOVER_PRMS16                                */
/*                                                                   */
/*   Descriptive Name: Causes OS2LVM and OS2DASD to check PRMs for   */
/*                     new or changed media.                         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the Rediscover PRM operation was successful, then    */
/*           *Error_Code will be LVM_ENGINE_NO_ERROR.  If there      */
/*           was an error, then *Error_Code will be > 0.             */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  New volumes may be discovered and assigned drive */
/*                  letters by OS2LVM and OS2DASD.                   */
/*                                                                   */
/*   Notes: The LVM Engine must be CLOSED when this function is      */
/*          called as this function is disabled while it is open!    */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds REDISCOVER_PRMS16( CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_LVM_VIEW16                                   */
/*                                                                   */
/*   Descriptive Name:  This function gets the OS2LVM data for the   */
/*                      specified drive letter.  The intent is to    */
/*                      allow the determination of what drive letter */
/*                      a volume really has given the possibilities  */
/*                      of conflict or a drive preference of '*'.    */
/*                                                                   */
/*   Input:  char  IFSM_Drive_Letter : The drive letter for which the*/
/*                                     OS2LVM data is requested.     */
/*           CARDINAL32 * Drive_Number : The address of a variable   */
/*                                       to hold the OS/2 drive      */
/*                                       number of the drive         */
/*                                       containing the first        */
/*                                       partition of the volume     */
/*                                       currently assigned to the   */
/*                                       requested drive letter.     */
/*           CARDINAL32 * Partition_LBA : The address of a variable  */
/*                                        to hold the LBA of the     */
/*                                        first partition of the     */
/*                                        volume currently assigned  */
/*                                        to the requested drive     */
/*                                        letter.                    */
/*           char * LVM_Drive_Letter : The address of a variable to  */
/*                                     hold the drive letter that    */
/*                                     OS2LVM thinks the volume      */
/*                                     assigned to the requested     */
/*                                     drive letter should have.     */
/*           BYTE * UnitID : The address of a variable to hold the   */
/*                           OS2LVM unit ID for the volume associated*/
/*                           with the requested drive letter.        */
/*                                                                   */
/*   Output:  The function return value will be TRUE if the function */
/*            completed successfully.                                */
/*                                                                   */
/*   Error Handling: If this function fails, the specified drive     */
/*                   letter is either not in use, or is in use by a  */
/*                   device not controlled by OS2LVM.                */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function can be used with the LVM Engine open or    */
/*          closed.                                                  */
/*                                                                   */
/*********************************************************************/
BOOLEAN _Far16 _Pascal _loadds GET_LVM_VIEW16( char                IFSM_Drive_Letter,
CARDINAL32 * _Seg16 Drive_Number,
CARDINAL32 * _Seg16 Partition_LBA,
char *       _Seg16 LVM_Drive_Letter,
BYTE *       _Seg16 UnitID);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Start_Logging16                                  */
/*                                                                   */
/*   Descriptive Name: Enables the LVM Engine logging.  Once enabled,*/
/*                     the LVM Engine logging function will log all  */
/*                     LVM Engine activity to the specified log file.*/
/*                     The data is logged in a binary format for     */
/*                     compactness and speed.                        */
/*                                                                   */
/*   Input: char * Filename - The filename of the file to use as the */
/*                            log file.                              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the logging file was successfully created, then      */
/*           *Error_Code will be 0.  If the log file could not be    */
/*           created, then *Error_Code will be > 0.                  */
/*                                                                   */
/*   Error Handling: If the log file can not be created, then        */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  A file may be created/opened for logging of      */
/*                  LVM Engine actions.                              */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds START_LOGGING16( char * _Seg16 Filename, CARDINAL32 * _Seg16 Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Stop_Logging16                                   */
/*                                                                   */
/*   Descriptive Name: This function ends LVM Engine logging and     */
/*                     closes the log file.                          */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if this function completes        */
/*           successfully; otherwise it will be > 0.                 */
/*                                                                   */
/*   Error Handling: If the log file is not currently opened, or if  */
/*                   the close operation fails on the log file, then */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  The log file may be closed.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds STOP_LOGGING16 ( CARDINAL32 * _Seg16 Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Export_Configuration16                           */
/*                                                                   */
/*   Descriptive Name: This function creates a file containing all of*/
/*                     the LVM.EXE commands necessary to recreate the*/
/*                     current partition/volume layout.              */
/*                                                                   */
/*   Input: char * Filename - FQN of the output file to be created   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A file creating LVM commands will be created.           */
/*           *Error_Code will be 0 if this function completes        */
/*           successfully; otherwise it will be > 0.                 */
/*                                                                   */
/*   Error Handling: If the output file cannot be created, or if     */
/*                   the close operation fails on the log file, then */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  A file may be created.                           */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds EXPORT_CONFIGURATION16( char *       _Seg16 Filename,
                                                    CARDINAL32 * _Seg16 Error_Code );

#endif

/* ************************************************************************** *

End of Functions Prototypes for use with Visual Age C/C++

************************************************************************** */

#else

/* ************************************************************************** *

Functions Prototypes for use with 16 Bit Compilers

************************************************************************** */

/* ************************************************************************** *

16 Bit Functions relating to the LVM Engine itself

************************************************************************** */

/****************************************************************************************************/
/*                                                                                                  */
/*   Function Name: OPEN_LVM_ENGINE16                                                                 */
/*                                                                                                  */
/*   Descriptive Name: Opens the LVM Engine and readies it for use.                                 */
/*                                                                                                  */
/*   Input: BOOLEAN Ignore_CHS : If TRUE, then the LVM engine will not check the CHS values in the  */
/*                               MBR/EBR partition tables for validity.  This is useful if there    */
/*                               are drive geometry problems, such as the drive was partitioned and */
/*                               formatted with one geometry and then moved to a different machine  */
/*                               which uses a different geometry for the drive.  This would cause   */
/*                               the starting and ending CHS values in the partition tables to      */
/*                               be inconsistent with the size and partition offset entries in the  */
/*                               partition tables.  Setting Ignore_CHS to TRUE will disable the     */
/*                               LVM Engine's CHS consistency checks, thereby allowing the drive    */
/*                               to be partitioned.                                                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in which to store an error code   */
/*                                    should an error occur.                                        */
/*                                                                                                  */
/*   Output:  *Error_Code will be 0 if this function completes successfully.  If an error occurs,   */
/*            *Error_Code will contain a non-zero error code.                                       */
/*                                                                                                  */
/*   Error Handling: If this function aborts with an error, all memory allocated during the course  */
/*                   of this function will be released.  Disk read errors will be reported to the   */
/*                   user via pop-up error messages.  Disk read errors will only cause this         */
/*                   function to abort if none of the disk drives in the system could be            */
/*                   successfully read.                                                             */
/*                                                                                                  */
/*   Side Effects:  The LVM Engine will be initialized.  The partition tables for all OS2DASD       */
/*                  controlled disk drives will be read into memory.  Memory will be allocated for  */
/*                  the data structures used by the LVM Engine.                                     */
/*                                                                                                  */
/*   Notes:  This is provided for programs that used LVM Version 1.  This function assumes an       */
/*           LVM_Interface_Type of VIO_Interface.						    */
/*                                                                                                  */
/****************************************************************************************************/
void far pascal _loadds OPEN_LVM_ENGINE16( BOOLEAN Ignore_CHS, CARDINAL32 far * Error_Code );

/****************************************************************************************************/
/*                                                                                                  */
/*   Function Name: OPEN_LVM_ENGINE216                                                              */
/*                                                                                                  */
/*   Descriptive Name: Opens the LVM Engine and readies it for use.                                 */
/*                                                                                                  */
/*   Input: BOOLEAN Ignore_CHS : If TRUE, then the LVM engine will not check the CHS values in the  */
/*                               MBR/EBR partition tables for validity.  This is useful if there    */
/*                               are drive geometry problems, such as the drive was partitioned and */
/*                               formatted with one geometry and then moved to a different machine  */
/*                               which uses a different geometry for the drive.  This would cause   */
/*                               the starting and ending CHS values in the partition tables to      */
/*                               be inconsistent with the size and partition offset entries in the  */
/*                               partition tables.  Setting Ignore_CHS to TRUE will disable the     */
/*                               LVM Engine's CHS consistency checks, thereby allowing the drive    */
/*                               to be partitioned.                                                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in which to store an error code   */
/*                                    should an error occur.                                        */
/*                                                                                                  */
/*   Output:  *Error_Code will be 0 if this function completes successfully.  If an error occurs,   */
/*            *Error_Code will contain a non-zero error code.                                       */
/*                                                                                                  */
/*   Error Handling: If this function aborts with an error, all memory allocated during the course  */
/*                   of this function will be released.  Disk read errors will be reported to the   */
/*                   user via pop-up error messages.  Disk read errors will only cause this         */
/*                   function to abort if none of the disk drives in the system could be            */
/*                   successfully read.                                                             */
/*                                                                                                  */
/*   Side Effects:  The LVM Engine will be initialized.  The partition tables for all OS2DASD       */
/*                  controlled disk drives will be read into memory.  Memory will be allocated for  */
/*                  the data structures used by the LVM Engine.                                     */
/*                                                                                                  */
/*   Notes:  This function is only available in LVM Version 2, and is not available in the current  */
/*           OS/2 implementation.                                                                   */
/*                                                                                                  */
/****************************************************************************************************/
/*
* void far pascal _loadds OPEN_LVM_ENGINE216( BOOLEAN Ignore_CHS, CARDINAL32 far * Error_Code );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: COMMIT_CHANGES16                                 */
/*                                                                   */
/*   Descriptive Name: Saves any changes made to the partitioning    */
/*                     information of the OS2DASD controlled disk    */
/*                     drives in the system.                         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  The function return value will be TRUE if all of the   */
/*            partitioning/volume changes made were successfully     */
/*            written to disk.  Also, *Error_Code will be 0 if no    */
/*            errors occur.                                          */
/*                                                                   */
/*            If an error occurs, then the furnction return value    */
/*            will be FALSE and *Error_Code will contain a non-zero  */
/*            error code.                                            */
/*                                                                   */
/*   Error Handling:  If an error occurs, the function return value  */
/*                    will be false and *Error_Code will be > 0.     */
/*                                                                   */
/*                    Disk read and write errors will be indicated by*/
/*                    setting the IO_Error field of the              */
/*                    Drive_Information_Record to TRUE.  Thus, if    */
/*                    the function return value is FALSE, and        */
/*                    *Error_Code indicates an I/O error, the caller */
/*                    of this function should call the               */
/*                    Get_Drive_Status function on each drive to     */
/*                    determine which drives had I/O errors.         */
/*                                                                   */
/*                    If a read or write error occurs, then the      */
/*                    engine may not have been able to create a      */
/*                    partition or volume.  Thus, the caller         */
/*                    may want to refresh all partition and volume   */
/*                    data to see what the engine was and was not    */
/*                    able to create.                                */
/*                                                                   */
/*   Side Effects:  The partitioning information of the disk drives  */
/*                  in the system may be altered.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN far pascal _loadds COMMIT_CHANGES16( CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: CLOSE_LVM_ENGINE16                               */
/*                                                                   */
/*   Descriptive Name: Closes the LVM Engine and frees any memory    */
/*                     held by the LVM Engine.                       */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects:  Any memory held by the LVM Engine is released.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds CLOSE_LVM_ENGINE16 ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: REFRESH_LVM_ENGINE16                             */
/*                                                                   */
/*   Descriptive Name: This function causes the LVM Engine to look   */
/*                     for changes in the current system             */
/*                     configuration and update its internal tables  */
/*                     accordingly.                                  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be set to  */
/*                   a non-zero value.                               */
/*                                                                   */
/*   Side Effects: Volumes which represent non-LVM devices may have  */
/*                 their handles changed!                            */
/*                                                                   */
/*   Notes:  After calling this function, Get_Volume_Control_Data    */
/*           should be called to get the updated list of volumes.    */
/*           This is necessary as the handles of some volumes may    */
/*           have changed.                                           */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds  REFRESH_LVM_ENGINE16( CARDINAL32 far * Error_Code );

/* ************************************************************************** *

16 Bit Functions relating to Drives

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  GET_DRIVE_CONTROL_DATA16                        */
/*                                                                   */
/*   Descriptive Name:  This function returns an array of            */
/*                      Drive_Control_Records.  These records provide*/
/*                      important information about the drives in the*/
/*                      system and provide the handles required to   */
/*                      access them.                                 */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output:  A Drive_Control_Array structure is returned.  If no    */
/*            errors occur, Drive_Control_Data will be non-NULL,     */
/*            Count will be greater than zero, and *Error_Code will  */
/*            be 0.                                                  */
/*                                                                   */
/*   Error Handling:  If an error occurs, the Drive_Control_Array    */
/*                    returned by this function will have NULL for   */
/*                    Drive_Control_Data, and 0 for Count.           */
/*                    *Error_Code will be greater than 0.            */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Drive_Control_Records pointed to by    */
/*           Drive_Control_Data pointer in the Drive_Control_Array   */
/*           structure returned by this function.  The caller should */
/*           free this memory when they are done using it.           */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_DRIVE_CONTROL_DATA16( Drive_Control_Record far ** Drive_Control_Data,
CARDINAL32 far *            Count,
CARDINAL32 far *            Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name:  GET_DRIVE_STATUS16                              */
/*                                                                   */
/*   Descriptive Name:  Returns the Drive_Information_Record for the */
/*                      drive specified by Drive_Handle.             */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Handle - The handle of the drive to use.*/
/*                             Drive handles are obtained through the*/
/*                             Get_Drive_Control_Data function.      */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns the Drive_Information_Record for  */
/*           the drive associated with the specified Drive_Handle.   */
/*           If no errors occur, *Error_Code will be set to 0.  If an*/
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling:  If an error occurs, then *Error_Code will be   */
/*                    non-zero.                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_DRIVE_STATUS16( CARDINAL32                      Drive_Handle,
Drive_Information_Record far *  Drive_Status,
CARDINAL32 *                    Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Partitions

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_PARTITIONS16                                 */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the object specified by Handle.          */
/*                                                                   */
/*   Input:CARDINAL32 Handle - This is the handle of a drive or      */
/*                      volume. Drive handles are obtained through   */
/*                      the Get_Drive_Control_Data function.  Volume */
/*                      handles are obtained through the             */
/*                      Get_Volume_Control_Data function.            */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  If     */
/*           Handle is the handle of a disk drive, then the returned */
/*           array will contain a partition information record for   */
/*           each partition and block of free space on that drive.   */
/*           If Handle is the handle of a volume, then the returned  */
/*           array will contain a partition information record for   */
/*           each partition which is part of the specified volume.   */
/*           If no errors occur, then *Error_Code will be 0.  If an  */
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling: Any memory allocated for the return value will  */
/*                   be freed.  The Partition_Information_Array      */
/*                   returned by this function will contain a NULL   */
/*                   pointer for Partition_Array, and have a Count of*/
/*                   0.  *Error_Code will be non-zero.               */
/*                                                                   */
/*                   If Handle is non-NULL and is invalid, a trap    */
/*                   is likely.                                      */
/*                                                                   */
/*   Side Effects:  Memory will be allocated to hold the array       */
/*                  returned by this function.                       */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Partition_Information_Records pointed  */
/*           to by Partition_Array pointer in the                    */
/*           Partition_Information_Array structure returned by this  */
/*           function.  The caller should free this memory when they */
/*           are done using it.                                      */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_PARTITIONS16( CARDINAL32                             Handle,
Partition_Information_Record far * *   Partition_Array,
CARDINAL32 far *                       Count,
CARDINAL32 far *                       Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_PARTITION_HANDLE16                           */
/*                                                                   */
/*   Descriptive Name: Returns the handle of the partition whose     */
/*                     serial number matches the one provided.       */
/*                                                                   */
/*   Input: CARDINAL32 Serial_Number - This is the serial number to  */
/*                                     look for.  If a partition with*/
/*                                     a matching serial number is   */
/*                                     found, its handle will be     */
/*                                     returned.                     */
/*          CARDINAL32 * Handle - The address of a variable (32 bits)*/
/*                                to hold the handle being returned. */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If a partition with a matching serial number is found,  */
/*              then the function return value will be the handle    */
/*              of the partition found.  If no matching partition is */
/*              found, then the function return value will be NULL.  */
/*                                                                   */
/*   Error Handling:  If no errors occur, *Error_Code will be        */
/*                    LVM_ENGINE_NO_ERROR.  If an error occurs, then */
/*                    *Error_Code will be a non-zero error code.     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_PARTITION_HANDLE16( CARDINAL32       Serial_Number,
CARDINAL32 far * Handle,
CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_PARTITION_INFORMATION16                      */
/*                                                                   */
/*   Descriptive Name: Returns the Partition_Information_Record for  */
/*                     the partition specified by Partition_Handle.  */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle associated with */
/*                                     the partition for which the   */
/*                                     Partition_Information_Record  */
/*                                     is desired.                   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A Partition_Information_Record is returned.  If there   */
/*           is no error, then *Error_Code will be 0.  If an error   */
/*           occurs, *Error_Code will be non-zero.                   */
/*                                                                   */
/*   Error Handling:  If the Partition_Handle is not a valid handle, */
/*                    a trap could result.  If it is a handle for    */
/*                    something other than a partition, an error code*/
/*                    will be returned in *Error_Code.               */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds  GET_PARTITION_INFORMATION16( CARDINAL32                                Partition_Handle,
Partition_Information_Record far *     Partition_Information,
CARDINAL32 far *                       Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: CREATE_PARTITION16                               */
/*                                                                   */
/*   Descriptive Name: Creates a partition on a disk drive.          */
/*                                                                   */
/*   Input: CARDINAL32      Handle - The handle of a disk drive or   */
/*                                   a block of free space.          */
/*          CARDINAL32      Size - The size, in sectors, of the      */
/*                                 partition to create.              */
/*          char            Name[] - The name to give to the newly   */
/*                                   created partition.              */
/*          Allocation_Algorithm algorithm - If Handle is a drive,   */
/*                                           then the engine will    */
/*                                           find a block of free    */
/*                                           space to use to create  */
/*                                           the partition.  This    */
/*                                           tells the engine which  */
/*                                           memory management       */
/*                                           algorithm to use.       */
/*          BOOLEAN         Bootable - If TRUE, then the engine will */
/*                                     only create the partition if  */
/*                                     it can be booted from.  If    */
/*                                     Primary_Partition is FALSE,   */
/*                                     then it is assumed that OS/2  */
/*                                     is the operating system that  */
/*                                     will be booted.               */
/*          BOOLEAN         Primary_Partition - If TRUE, then the    */
/*                                              engine will create   */
/*                                              a primary partition. */
/*                                              If FALSE, then the   */
/*                                              engine will create a */
/*                                              logical drive.       */
/*          BOOLEAN         Allocate_From_Start - If TRUE, then the  */
/*                                                engine will        */
/*                                                allocate the new   */
/*                                                partition from the */
/*                                                beginning of the   */
/*                                                selected block of  */
/*                                                free space.  If    */
/*                                                FALSE, then the    */
/*                                                partition will be  */
/*                                                allocated from the */
/*                                                end of the selected*/
/*                                                block of free      */
/*                                                space.             */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function return value will be the handle of the     */
/*           partition created.  If the partition could not be       */
/*           created, then NULL will be returned.                    */
/*           *Error_Code will be 0 if the partition was created.     */
/*           *Error_Code will be > 0 if the partition could not be   */
/*           created.                                                */
/*                                                                   */
/*   Error Handling:  If the partition can not be created, then any  */
/*                    memory allocated by this function will be      */
/*                    freed and the partitioning of the disk in      */
/*                    question will be unchanged.                    */
/*                                                                   */
/*                    If Handle is not a valid handle, then a trap   */
/*                    may result.                                    */
/*                                                                   */
/*                    If Handle represents a partition or volume,    */
/*                    then the function will abort and set           */
/*                    *Error_Code to a non-zero value.               */
/*                                                                   */
/*   Side Effects:  A partition may be created on a disk drive.      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 far pascal _loadds CREATE_PARTITION16( CARDINAL32            Handle,
CARDINAL32            Size,
char far              Name[ PARTITION_NAME_SIZE ],
Allocation_Algorithm  algorithm,
BOOLEAN               Bootable,
BOOLEAN               Primary_Partition,
BOOLEAN               Allocate_From_Start,
CARDINAL32 far *      Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: DELETE_PARTITION16                               */
/*                                                                   */
/*   Descriptive Name: Deletes the partition specified by            */
/*                     Partition_Handle.                             */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle associated with */
/*                                     the partition to be deleted.  */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition was deleted      */
/*           successfully.  *Error_Code will be > 0 if the partition */
/*           could not be deleted.                                   */
/*                                                                   */
/*   Error Handling: If the partition can not be deleted, then       */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects: A partition on a disk drive may be deleted.       */
/*                                                                   */
/*   Notes:  A partition can not be deleted if it is part of a       */
/*           volume!                                                 */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds DELETE_PARTITION16( CARDINAL32        Partition_Handle,
CARDINAL32 far *  Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_ACTIVE_FLAG16                                */
/*                                                                   */
/*   Descriptive Name: Sets the Active Flag field for a partition.   */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle of the partition*/
/*                                     whose Active Flag is to be set*/
/*          BYTE Active_Flag - The new value for the Active Flag.    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the Active Flag was            */
/*           successfully set, otherwise *Error_Code will contain a  */
/*           non-zero error code indicating what went wrong.         */
/*                                                                   */
/*   Error Handling: If the Active Flag can not be set, this function*/
/*                   will abort without changing any disk structures.*/
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                                                                   */
/*   Side Effects:  The Active Flag for a partition may be modified. */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_ACTIVE_FLAG16 ( CARDINAL32          Partition_Handle,
BYTE                Active_Flag,
CARDINAL32 far *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_OS_FLAG16                                    */
/*                                                                   */
/*   Descriptive Name: Sets the OS Flag field for a partition.  This */
/*                     field is typically used to indicate the       */
/*                     filesystem used on the partition, which       */
/*                     generally gives an indication of which OS is  */
/*                     using that partition.                         */
/*                                                                   */
/*   Input: CARDINAL32 Partition_Handle - The handle of the partition*/
/*                                     whose Active Flag is to be set*/
/*          BYTE OS_Flag - The new value for the OS Flag.            */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the OS Flag was successfully   */
/*           set, otherwise *Error_Code will contain a non-zero error*/
/*           code indicating what went wrong.                        */
/*                                                                   */
/*   Error Handling: If the OS Flag can not be set, this function    */
/*                   will abort without changing any disk structures.*/
/*                                                                   */
/*                   If Partition_Handle is not a valid handle, a    */
/*                   trap may result.                                */
/*                                                                   */
/*                   If Partition_Handle is a volume or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                                                                   */
/*   Side Effects:  The OS Flag for a partition may be modified.     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_OS_FLAG16 ( CARDINAL32          Partition_Handle,
BYTE                OS_Flag,
CARDINAL32 far *    Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Volumes

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_VOLUME_CONTROL_DATA16                        */
/*                                                                   */
/*   Descriptive Name: This function returns a structure containing  */
/*                     an array of Volume_Control_Records.  These    */
/*                     records contain information about volumes     */
/*                     which is invariant - i.e. will not change for */
/*                     as long as the volume exists.  One of the     */
/*                     items in the Volume_Control_Record is the     */
/*                     handle for the volume.  This handle must be   */
/*                     used on all accesses to the volume.           */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A Volume_Control_Array structure is returned.           */
/*                                                                   */
/*           If there are no errors, then the Volume_Control_Data    */
/*           pointer in the Volume_Control_Array will be non-NULL,   */
/*           the Count field of the Volume_Control_Array will be     */
/*           >= 0, and *Error_Code will be 0.                        */
/*                                                                   */
/*           If an error does occur, then the Volume_Control_Data    */
/*           pointer in the the Volume_Control_Array will be NULL,   */
/*           the Count field of the Volume_Control_Array will be 0,  */
/*           and *Error_Code will be > 0.                            */
/*                                                                   */
/*   Error Handling: If an error occurs, then any memory allocated by*/
/*                   this function will be freed.                    */
/*                                                                   */
/*   Side Effects:  Memory for the returned array is allocated.      */
/*                                                                   */
/*   Notes:  The caller becomes responsible for the memory allocated */
/*           for the array of Volume_Control_Records pointed to by   */
/*           Volume_Control_Data pointer in the Volume_Control_Array */
/*           structure returned by this function.  The caller should */
/*           free this memory when they are done using it.           */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_VOLUME_CONTROL_DATA16( Volume_Control_Record  far **  Volume_Control_Data,
CARDINAL32 far *              Count,
CARDINAL32 far *              Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_VOLUME_INFORMATION16                         */
/*                                                                   */
/*   Descriptive Name:  This function returns the                    */
/*                      Volume_Information_Record for the volume     */
/*                      associated with Volume_Handle.               */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume about*/
/*                                  which information is desired.    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a Volume_Information_Record.      */
/*                                                                   */
/*           If this function is successful, then *Error_Code will be*/
/*              0.                                                   */
/*                                                                   */
/*           If this function fails, then *Error_Code will be > 0.   */
/*                                                                   */
/*   Error Handling:  If Volume_Handle is not a valid handle, a trap */
/*                    will be likely.  If Volume_Handle is a drive or*/
/*                    partition handle, *Error_Code will be > 0.     */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_VOLUME_INFORMATION16( CARDINAL32                           Volume_Handle,
Volume_Information_Record  far *     Volume_Information,
CARDINAL32 far *                     Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: CREATE_VOLUME16                                  */
/*                                                                   */
/*   Descriptive Name:  This function creates a volume from a list of*/
/*                      partitions.  The partitions are specified by */
/*                      their corresponding handles.                 */
/*                                                                   */
/*   Input: char         Name[] - The name to assign to the newly    */
/*                                created volume.                    */
/*          BOOLEAN      Create_LVM_Volume - If TRUE, then an LVM    */
/*                                           volume is created,      */
/*                                           otherwise a             */
/*                                           compatibility volume is */
/*                                           created.                */
/*          BOOLEAN      Bootable - If TRUE, the volume will not be  */
/*                                  created unless OS/2 can be booted*/
/*                                  from it.                         */
/*          char         Drive_Letter_Preference - This is the drive */
/*                                                 letter to use for */
/*                                                 accessing the     */
/*                                                 newly created     */
/*                                                 volume.           */
/*          CARDINAL32   FeaturesToUse - This is currently reserved  */
/*                                       for future use and should   */
/*                                       always be set to 0.         */
/*          CARDINAL32   Partition_Count - The number of partitions  */
/*                                         to link together to form  */
/*                                         the volume being created. */
/*          ADDRESS      Partition_Handles[] - An array of partition */
/*                                             handles with one entry*/
/*                                             for each partition    */
/*                                             that is to become part*/
/*                                             of the volume being   */
/*                                             created.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was created.        */
/*           *Error_Code will be > 0 if the volume could not be      */
/*              created.                                             */
/*                                                                   */
/*   Error Handling: If any of the handles in the partition handles  */
/*                   array is not valid, then a trap is likely.  If  */
/*                   Partition_Count is greater than the number of   */
/*                   entries in the partition handles array, then a  */
/*                   trap is likely.  If any of the handles in the   */
/*                   partition array are not partition handles, then */
/*                   *Error_Code will be > 0.  If the volume can NOT */
/*                   be created, then *Error_Code will be > 0 and any*/
/*                   memory allocated by this function will be freed.*/
/*                   If the volume can NOT be created, then the      */
/*                   existing partition/volume structure of the disk */
/*                   will be unchanged.                              */
/*                                                                   */
/*   Side Effects:  A volume may be created.                         */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds CREATE_VOLUME16( char far         Name[VOLUME_NAME_SIZE],
BOOLEAN          Create_LVM_Volume,
BOOLEAN          Bootable,
char             Drive_Letter_Preference,
CARDINAL32       FeaturesToUse,
CARDINAL32       Partition_Count,
CARDINAL32       Partition_Handles[],
CARDINAL32 far * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: CREATE_VOLUME216                                 */
/*                                                                   */
/*   Descriptive Name:  This function creates a volume from a list of*/
/*                      partitions.  The partitions are specified by */
/*                      their corresponding handles.                 */
/*                                                                   */
/*   Input: char         Name[] - The name to assign to the newly    */
/*                                created volume.                    */
/*          BOOLEAN      Create_LVM_Volume - If TRUE, then an LVM    */
/*                                           volume is created,      */
/*                                           otherwise a             */
/*                                           compatibility volume is */
/*                                           created.                */
/*          BOOLEAN      Bootable - If TRUE, the volume will not be  */
/*                                  created unless OS/2 can be booted*/
/*                                  from it.                         */
/*          char         Drive_Letter_Preference - This is the drive */
/*                                                 letter to use for */
/*                                                 accessing the     */
/*                                                 newly created     */
/*                                                 volume.           */
/*          CARDINAL32   Feature_Count - The number of features to   */
/*                                       install on the volume being */
/*                                       created.  This field is     */
/*                                       ignored if Create_LVM_Volume*/
/*                                       is FALSE.                   */
/*          LVM_Feature_Specification_Record FeaturesToUse[] - An    */
/*                                         array of feature IDs and  */
/*                                         their associated LVM      */
/*                                         classes used to designate */
/*                                         which features to install */
/*                                         on the volume being       */
/*                                         created and the order in  */
/*                                         which to install them.    */
/*                                         This field is ignored if  */
/*                                         Create_LVM_Volume is      */
/*                                         FALSE.                    */
/*          CARDINAL32   Partition_Count - The number of partitions  */
/*                                         to link together to form  */
/*                                         the volume being created. */
/*          ADDRESS      Partition_Handles[] - An array of partition */
/*                                             handles with one entry*/
/*                                             for each partition    */
/*                                             that is to become part*/
/*                                             of the volume being   */
/*                                             created.              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was created.        */
/*           *Error_Code will be > 0 if the volume could not be      */
/*              created.                                             */
/*                                                                   */
/*   Error Handling: If any of the handles in the partition handles  */
/*                   array is not valid, then a trap is likely.  If  */
/*                   Partition_Count is greater than the number of   */
/*                   entries in the partition handles array, then a  */
/*                   trap is likely.  If any of the handles in the   */
/*                   partition array are not partition handles, then */
/*                   *Error_Code will be > 0.  If the volume can NOT */
/*                   be created, then *Error_Code will be > 0 and any*/
/*                   memory allocated by this function will be freed.*/
/*                   If the volume can NOT be created, then the      */
/*                   existing partition/volume structure of the disk */
/*                   will be unchanged.                              */
/*                                                                   */
/*   Side Effects:  A volume may be created.                         */
/*                                                                   */
/*   Notes:  This function is only available in LVM Version 2, and   */
/*           is not available in the current OS/2 implementation.    */
/*                                                                   */
/*********************************************************************/
/*
* void far pascal _loadds CREATE_VOLUME216( char far  *                             Name,
* BOOLEAN                                 Create_LVM_Volume,
* BOOLEAN                                 Bootable,
* char                                    Drive_Letter_Preference,
* CARDINAL32                              Feature_Count,
* LVM_Feature_Specification_Record far *  FeaturesToUse,
* CARDINAL32                              Partition_Count,
* CARDINAL32 far *                        Partition_Handles,
* CARDINAL32 far *                        Error_Code
* );
*/

/*********************************************************************/
/*                                                                   */
/*   Function Name: DELETE_VOLUME16                                  */
/*                                                                   */
/*   Descriptive Name: Deletes the volume specified by Volume_Handle.*/
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to   */
/*                                  delete.  All partitions which are*/
/*                                  part of the specified volume will*/
/*                                  be deleted also.                 */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume and its partitions  */
/*           are successfully deleted.  Otherwise, *Error_Code will  */
/*           be > 0.                                                 */
/*                                                                   */
/*   Error Handling: *Error_Code will be > 0 if an error occurs.  If */
/*                   the volume or any of its partitions can not be  */
/*                   deleted, then any changes made by this function */
/*                   will be undone.                                 */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  A volume and its partitions may be deleted.      */
/*                  System memory may be freed as the internal       */
/*                  structures used to track the deleted volume      */
/*                  are no longer required.                          */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds DELETE_VOLUME16( CARDINAL32          Volume_Handle,
CARDINAL32 far *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: HIDE_VOLUME16                                    */
/*                                                                   */
/*   Descriptive Name: Hide volume "hides" a volume from OS/2 by     */
/*                     removing its drive letter assignment.  Without*/
/*                     a drive letter assignment, OS/2 can not access*/
/*                     (or "see") the volume.                        */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to   */
/*                                     hide.                         */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume was successfully    */
/*           hidden.  If the volume could not be hidden, then        */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: *Error_Code will be > 0 if the volume can not be*/
/*                   hidden.  If the volume can not be hidden, then  */
/*                   nothing will be altered.                        */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds HIDE_VOLUME16( CARDINAL32          Volume_Handle,
CARDINAL32 far *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: EXPAND_VOLUME16                                  */
/*                                                                   */
/*   Descriptive Name: This function expands an existing volume by   */
/*                     linking additional partitions to it.          */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to be*/
/*                                  expanded.                        */
/*          CARDINAL32 Partition_Count - The number of partitions or */
/*                                       volumes to be added to the  */
/*                                       volume being expanded.      */
/*          CARDINAL32 Partition_Handles[] - An array of handles.    */
/*                                        Each handle in the array is*/
/*                                        the handle of a partition  */
/*                                        which is to be added to the*/
/*                                        volume being expanded.     */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the volume is successfully     */
/*           expanded.  If the volume can not be expanded,           */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If the volume can not be expanded, the state of */
/*                   the volume is unchanged and any memory allocated*/
/*                   by this function is freed.                      */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*                   If any of the partition handles in the          */
/*                   Partition_handles array are not valid handles,  */
/*                   then a trap may result.                         */
/*                                                                   */
/*                   If any of the partition handles in the          */
/*                   Partition_Handles array are actually drive      */
/*                   handles, then this function will abort and      */
/*                   set *Error_Code to a non-zero value.            */
/*                                                                   */
/*                   If Partition_Count is greater than the number of*/
/*                   entries in the Partition_Handles array, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*   Side Effects:  A volume may be expanded.  If the volume is      */
/*                  expanded using another volume, the partitions    */
/*                  on the second volume will be linked to those of  */
/*                  the first volume and all data on the second      */
/*                  volume will be lost.                             */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds EXPAND_VOLUME16 ( CARDINAL32        Volume_Handle,
CARDINAL32        Partition_Count,
CARDINAL32 far    Partition_Handles[],
CARDINAL32 far *  Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: ASSIGN_DRIVE_LETTER16                            */
/*                                                                   */
/*   Descriptive Name: Assigns a drive letter to a volume.           */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume which*/
/*                                  is to have its assigned drive    */
/*                                  letter changed.                  */
/*          char  New_Drive_Preference - The new drive letter to     */
/*                                       assign to the volume.       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the drive letter was assigned  */
/*           successfully; otherwise *Error_Code will be > 0.        */
/*                                                                   */
/*   Error Handling: If the drive letter assignment can not be made, */
/*                   the volume will not be altered.                 */
/*                                                                   */
/*                   If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  A volume may have its drive letter assignment    */
/*                  changed.                                         */
/*                                                                   */
/*   Notes:  If the drive letter being assigned is already in use by */
/*           volume which does not lie on removable media, then the  */
/*           drive assignment will NOT be made.                      */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds ASSIGN_DRIVE_LETTER16( CARDINAL32          Volume_Handle,
char                New_Drive_Preference,
CARDINAL32 far *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_INSTALLABLE16                                */
/*                                                                   */
/*   Descriptive Name: Marks a volume as being the volume to install */
/*                     OS/2 on.                                      */
/*                                                                   */
/*   Input: CARDINAL32 Volume_Handle - The handle of the volume to   */
/*                                  which OS/2 should be installed.  */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the volume is successfully marked as installable,    */
/*           *Error_Code will be 0; otherwise *Error_Code will       */
/*           be > 0.                                                 */
/*                                                                   */
/*   Error Handling: If Volume_Handle is not a valid handle, a trap  */
/*                   may result.                                     */
/*                                                                   */
/*                   If Volume_Handle is a partition or drive handle,*/
/*                   then this function will abort and set           */
/*                   *Error_Code to a non-zero value.                */
/*                                                                   */
/*   Side Effects:  The specified volume may be marked as            */
/*                  installable.                                     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_INSTALLABLE16 ( CARDINAL32          Volume_Handle,
CARDINAL32 far *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_INSTALLABLE_VOLUME16                         */
/*                                                                   */
/*   Descriptive Name: Marks a volume as being the volume to install */
/*                     OS/2 on.                                      */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If a volume is mared installable, its information will  */
/*           be returned and *Error_Code will be LVM_ENGINE_NO_ERROR.*/
/*           If there is no volume marked installable, then          */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: An error code is returned if there is an error. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_INSTALLABLE_VOLUME16 ( Volume_Information_Record * Volume_Information,
CARDINAL32 far *            Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Partitions, Drives, and Volumes.

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_NAME16                                       */
/*                                                                   */
/*   Descriptive Name: Sets the name of a volume, drive, or partition*/
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the drive, partition,  */
/*                           or volume which is to have its name set.*/
/*          char New_Name[] - The new name for the drive/partition/  */
/*                            volume.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the name is set as specified.  */
/*           If the name can not be set, *Error_Code will be > 0.    */
/*                                                                   */
/*   Error Handling: If the name can not be set, then drive/volume/  */
/*                   partition is not modified.                      */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*   Side Effects:  A drive/volume/partition may have its name set.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_NAME16 ( CARDINAL32       Handle,
char  far        New_Name[],
CARDINAL32 far * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_STARTABLE16                                  */
/*                                                                   */
/*   Descriptive Name: Sets the specified volume or partition        */
/*                     startable.  If a volume is specified, it must */
/*                     be a compatibility volume whose partition is  */
/*                     a primary partition on the first drive.  If a */
/*                     partition is specified, it must be a primary  */
/*                     partition on the first drive in the system.   */
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the partition or volume*/
/*                           which is to be set startable.           */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the specified volume or        */
/*           partition was set startable.                            */
/*           If the name can not be set, *Error_Code will be > 0.    */
/*                                                                   */
/*   Error Handling: If the volume or partition could not be set     */
/*                   startable, then nothing in the system is        */
/*                   changed.                                        */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*   Side Effects:  Any other partition or volume which is marked    */
/*                  startable will have its startable flag cleared.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void  far pascal _loadds SET_STARTABLE16 ( CARDINAL32       Handle,
CARDINAL32 far * Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_VALID_OPTIONS16                              */
/*                                                                   */
/*   Descriptive Name: Returns a bitmap where each bit in the bitmap */
/*                     corresponds to a possible operation that the  */
/*                     LVM Engine can perform.  Those bits which are */
/*                     1 represent operations which can be performed */
/*                     on the item specified by Handle.  Those bits  */
/*                     which are 0 are not allowed on the item       */
/*                     specified by Handle.                          */
/*                                                                   */
/*   Input: CARDINAL32 Handle - This is any valid drive, volume, or  */
/*                           partition handle.                       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output:  A bitmap indicating which operations are valid on the  */
/*            item specified by Handle.                              */
/*                                                                   */
/*            If no errors occur, *Error_Code will be 0, otherwise   */
/*            *Error_Code will be > 0.                               */
/*                                                                   */
/*   Error Handling:  If Handle is not valid, a trap will be likely. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  The values of the various bits in the bitmap returned   */
/*           by this function are defined near the beginning of this */
/*           file, immediately after all of the structure            */
/*           definitions.                                            */
/*                                                                   */
/*********************************************************************/
CARDINAL32 far pascal _loadds GET_VALID_OPTIONS16( CARDINAL32        Handle,
CARDINAL32 far *    Error_Code
);

/* ************************************************************************** *

16 Bit Functions relating to Boot Manager

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name: BOOT_MANAGER_IS_INSTALLED16                      */
/*                                                                   */
/*   Descriptive Name: Indicates whether or not Boot Manager is      */
/*                     installed on the first or second hard drives  */
/*                     in the system.                                */
/*                                                                   */
/*   Input: BOOLEAN * Active - *Active is set to TRUE if LVM found an*/
/*                             active copy of Boot Manager on the    */
/*                             system.  If LVM could not find an     */
/*                             active copy of Boot Manager on the    */
/*                             system, but did find an inactive copy */
/*                             of Boot Manager, then *Active will be */
/*                             set to FALSE.                         */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: TRUE is returned if Boot Manager is found.  If this     */
/*           copy of Boot Manager is Active, then *Active will be set*/
/*           to TRUE.  If the copy of Boot Manager is not currently  */
/*           active, then *Active will be set to FALSE.              */
/*                                                                   */
/*           FALSE is returned if Boot Manager is not found or if an */
/*           error occurs.  In this case, *Active is undefined.      */
/*                                                                   */
/*           *Error_Code will be 0 if no errors occur; otherwise it  */
/*           will be > 0.                                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN far pascal _loadds BOOT_MANAGER_IS_INSTALLED16( BOOLEAN far * Active, CARDINAL32 far * Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_BOOT_MANAGER_HANDLE16                        */
/*                                                                   */
/*   Descriptive Name: Returns the handle of the partition containing*/
/*                     Boot Manager.                                 */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If Boot Manager is NOT installed, NULL is returned.     */
/*           If Boot Manager is installed, whether it is active or   */
/*           not, the handle of the partition it resides in is       */
/*           returned.                                               */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  If Boot_Manager_Is_Installed returns FALSE, but this    */
/*           function does not return NULL, then the handle returned */
/*           represents an inactive copy of Boot Manager.  To        */
/*           activate an inactive Boot Manager, pass its handle to   */
/*           the Set_Startable function.                             */
/*                                                                   */
/*           See the Notes for the Boot_Manager_Is_Installed function*/
/*           for additional information.                             */
/*                                                                   */
/*********************************************************************/
ADDRESS far _pascal _loadds GET_BOOT_MANAGER_HANDLE16( CARDINAL32 far * Error_Code);

/*********************************************************************/
/*                                                                   */
/*   Function Name: ADD_TO_BOOT_MANAGER16                            */
/*                                                                   */
/*   Descriptive Name: Adds the volume/partition to the Boot Manager */
/*                     menu.                                         */
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of a partition or volume  */
/*                      that is to be added to the Boot Manager menu.*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition or volume was    */
/*           successfully added to the Boot Manager menu; otherwise  */
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If the partition/volume can not be added to the */
/*                   Boot Manager menu, no action is taken and       */
/*                   *Error_Code will contain a non-zero error code. */
/*                                                                   */
/*                   If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*                   If Handle represents a drive, then this function*/
/*                   will abort and set *Error_Code to a non-zero    */
/*                   value.                                          */
/*                                                                   */
/*   Side Effects:  The Boot Manager menu may be altered.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds ADD_TO_BOOT_MANAGER16 ( CARDINAL32 Handle, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: REMOVE_FROM_BOOT_MANAGER16                       */
/*                                                                   */
/*   Descriptive Name: Removes the specified partition or volume     */
/*                     from the Boot Manager menu.                   */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the partition or volume was    */
/*           successfully removed to the Boot Manager menu;          */
/*           otherwise *Error_Code will be > 0.                      */
/*                                                                   */
/*   Error Handling: If Handle is not a valid handle, a trap may     */
/*                   result.                                         */
/*                                                                   */
/*                   If Handle represents a drive, or if Handle      */
/*                   represents a volume or partition which is NOT on*/
/*                   the boot manager menu, then this function       */
/*                   will abort and set *Error_Code to a non-zero    */
/*                   value.                                          */
/*                                                                   */
/*   Side Effects:  The Boot Manager menu may be altered.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds REMOVE_FROM_BOOT_MANAGER16 ( CARDINAL32 Handle, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_BOOT_MANAGER_MENU16                          */
/*                                                                   */
/*   Descriptive Name: Returns an array containing the handles of the*/
/*                     partitions and volumes appearing on the       */
/*                     Boot Manager menu.                            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function returns a Boot_Manager_Menu structure.     */
/*           This structure contains two items: a pointer to an array*/
/*           of Boot_Manager_Menu_Items and a count of how many items*/
/*           are in the array.  Each Boot_Manager_Menu_Item contains */
/*           a handle and a BOOLEAN variable to indicate whether the */
/*           handle is for a partition or a volume.                  */
/*                                                                   */
/*           If this function is successful, then *Error_Code will   */
/*           be 0.                                                   */
/*                                                                   */
/*           If an error occurs, the Count field in the              */
/*           Boot_Manager_Menu will be 0 and the corresponding       */
/*           pointer will be NULL.  *Error_Code will be > 0.         */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                   any memory allocated by this function will be   */
/*                   freed.                                          */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_BOOT_MANAGER_MENU16 ( Boot_Manager_Menu_Item **  Menu_Items,
CARDINAL32 *               Count,
CARDINAL32 *               Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: INSTALL_BOOT_MANAGER16                           */
/*                                                                   */
/*   Descriptive Name: This function installs Boot Manager.  It can  */
/*                     be used to replace an existing Boot Manager   */
/*                     as well.                                      */
/*                                                                   */
/*   Input: CARDINAL32  Drive_Number - The number of the drive to    */
/*                                     install Boot Manager on.  Must*/
/*                                     be 1 or 2.                    */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If this function is successful, then *Error_Code will be*/
/*           0; otherwise it will be > 0.                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be set to a*/
/*                   non-zero value.  Depending upon the error, it   */
/*                   is possible that the Boot Manager partition can */
/*                   be left in an unusuable state (such as for a    */
/*                   write error).                                   */
/*                                                                   */
/*   Side Effects: Boot Manager may be installed on drive 1 or 2.    */
/*                 The MBR for drive 1 may be altered.               */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds INSTALL_BOOT_MANAGER16 ( CARDINAL32   Drive_Number, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: REMOVE_BOOT_MANAGER16                            */
/*                                                                   */
/*   Descriptive Name: Removes Boot Manager from the system.         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if Boot Manager was successfully  */
/*           removed from the system; otherwise *Error_Code will     */
/*           be 0.                                                   */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  Boot Manager will be removed from the system.    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds REMOVE_BOOT_MANAGER16( CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_BOOT_MANAGER_OPTIONS16                       */
/*                                                                   */
/*   Descriptive Name: Sets the Boot Managers Options.  The options  */
/*                     that can be set are: whether or not the time- */
/*                     out timer is active, how long the timer-out   */
/*                     is, the partition to boot by default, and     */
/*                     whether or not Boot Manager should display its*/
/*                     menu using default mode or advanced mode.     */
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the partition or volume*/
/*                           to boot if the time-out timer is active */
/*                           and the time-out value is reached.      */
/*          BOOLEAN Timer_Active - If TRUE, then the time-out timer  */
/*                                 is active.                        */
/*          CARDINAL32 Time_Out_Value - If the time-out timer is     */
/*                                      active, this is the time-out */
/*                                      value, in seconds.           */
/*          BOOLEAN Advanced_Mode - If TRUE, then Boot Manager will  */
/*                                  operate in advanced mode.  If    */
/*                                  FALSE, then normal mode will be  */
/*                                  in effect.                       */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if no errors occur.  If an error  */
/*           does occur, then *Error_Code will be > 0.               */
/*                                                                   */
/*   Error Handling: If an error occurs, no changes will be made to  */
/*                   Boot Manager and *Error_Code will be set a      */
/*                   non-zero error code.                            */
/*                                                                   */
/*   Side Effects:  Boot Manager may be modified.                    */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_BOOT_MANAGER_OPTIONS16( CARDINAL32          Handle,
BOOLEAN             Timer_Active,
CARDINAL32          Time_Out_Value,
BOOLEAN             Advanced_Mode,
CARDINAL32 far *    Error_Code
);

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_BOOT_MANAGER_OPTIONS16                       */
/*                                                                   */
/*   Descriptive Name: This function returns the current Boot Manager*/
/*                     settings for the various Boot Manager options.*/
/*                                                                   */
/*   Input: CARDINAL32 * Handle - The handle for the default boot    */
/*                             volume or partition.                  */
/*          BOOLEAN * Handle_Is_Volume - If TRUE, then Handle        */
/*                                       represents a volume.  If    */
/*                                       FALSE, then Handle          */
/*                                       represents a partition.     */
/*          BOOLEAN * Timer_Active - If TRUE, then the time-out timer*/
/*                                   is active.  If FALSE, then the  */
/*                                   time-out timer is not active.   */
/*          CARDINAL32 * Time_Out_Value - If the time-out timer is   */
/*                                        active, then this is the   */
/*                                        number of seconds that Boot*/
/*                                        Manager will wait for user */
/*                                        input before booting the   */
/*                                        default volume/partition.  */
/*          BOOLEAN * Advanced_Mode - If TRUE, the Boot Manager is   */
/*                                    operating in advanced mode.  If*/
/*                                    FALSE, then Boot Manager is    */
/*                                    operating in normal mode.      */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Handle, *Handle_Is_Volume, *Timer_Active,              */
/*           *Time_out_value, *Advanced_Mode, and *Error_Code are all*/
/*           set by this function.  If there are no errors, then     */
/*           *Error_Code will be set to 0.  If any errors occur, then*/
/*           *Error_Code will be > 0.                                */
/*                                                                   */
/*   Error Handling: If any of the parameters are invalid, then a    */
/*                   trap is likely.  If Boot Manager is not         */
/*                   installed, then *Error_Code will be > 0.        */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_BOOT_MANAGER_OPTIONS16( CARDINAL32 far * Handle,
BOOLEAN    far * Handle_Is_Volume,
BOOLEAN    far * Timer_Active,
CARDINAL32 far * Time_Out_Value,
BOOLEAN    far * Advanced_Mode,
CARDINAL32 far * Error_Code
);

/* ************************************************************************** *

Other 16 Bit Functions

************************************************************************** */

/*********************************************************************/
/*                                                                   */
/*   Function Name:  FREE_ENGINE_MEMORY16                            */
/*                                                                   */
/*   Descriptive Name: Frees a memory object created by LVM.DLL and  */
/*                     returned to a user of LVM.DLL.                */
/*                                                                   */
/*   Input: CARDINAL32 Object : The address of the memory object to  */
/*                           free.  This could be the                */
/*                           Drive_Control_Data field of a           */
/*                           Drive_Control_Record, the               */
/*                           Partition_Array field of a              */
/*                           Partition_Information_Array structure,  */
/*                           or any other dynamically allocated      */
/*                           memory object created by LVM.DLL and    */
/*                           returned by a function in LVM.DLL.      */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  A trap or exception could occur if a bad address is     */
/*           passed into this function.                              */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds FREE_ENGINE_MEMORY16( CARDINAL32 Object );

/*********************************************************************/
/*                                                                   */
/*   Function Name: New_MBR16                                        */
/*                                                                   */
/*   Descriptive Name: This function lays down a new MBR on the      */
/*                     specified drive.                              */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Handle - The handle of the drive on     */
/*                                 which the new MBR is to be placed.*/
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if the new MBR was successfully   */
/*           placed on the specified drive.  If the operation failed */
/*           for any reason, then *Error_Code will contain a non-zero*/
/*           error code.                                             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the existing MBR is not*/
/*                   altered and *Error_Code will be > 0.            */
/*                                                                   */
/*   Side Effects:  A new MBR may be placed on the specified drive.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds NEW_MBR16( CARDINAL32 Drive_Handle, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_AVAILABLE_DRIVE_LETTERS16                    */
/*                                                                   */
/*   Descriptive Name: This function returns a bitmap indicating     */
/*                     which drive letters are available for use.    */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the available drive   */
/*           letters.  If this function is successful, then          */
/*           *Error_Code will be set to 0.  Otherwise, *Error_Code   */
/*           will be > 0 and the bitmap returned will have all bits  */
/*           set to 0.                                               */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  A drive letter is available if it is not associated     */
/*           with a volume located on a disk drive controlled        */
/*           by OS2DASD.                                             */
/*                                                                   */
/*********************************************************************/
CARDINAL32 far pascal _loadds GET_AVAILABLE_DRIVE_LETTERS16 ( CARDINAL32 far * Error_Code ) ;

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_RESERVED_DRIVE_LETTERS16                     */
/*                                                                   */
/*   Descriptive Name: This function returns a bitmap indicating     */
/*                     which drive letters are reserved for use by   */
/*                     devices NOT under the control of LVM.         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: This function returns a bitmap of the drive letters     */
/*           which are being used by devices which are NOT controlled*/
/*           by LVM.  While a Volume CAN be assigned a drive letter  */
/*           from this list, a reboot will almost always be required */
/*           in order for the assignment to take place.              */
/*           If this function is successful, then *Error_Code will be*/
/*           set to 0.  Otherwise, *Error_Code will be > 0 and the   */
/*           bitmap returned will have all bits set to 0.            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  Devices which are assigned drive letters but which are  */
/*           NOT under LVM control include:  CD-ROM, Network drives, */
/*           parallel port attached devices, and any DASD devices    */
/*           not controlled by OS2DASD.                              */
/*                                                                   */
/*********************************************************************/
CARDINAL32 far pascal _loadds GET_RESERVED_DRIVE_LETTERS16 ( CARDINAL32 far * Error_Code ) ;

/*********************************************************************/
/*                                                                   */
/*   Function Name: REBOOT_REQUIRED16                                */
/*                                                                   */
/*   Descriptive Name: This function indicates whether or not any    */
/*                     changes were made to the partitioning of the  */
/*                     disks in the system which would require a     */
/*                     reboot to make functional.                    */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value will be TRUE if the system    */
/*           must be rebooted as a result of disk partitioning       */
/*           changes.                                                */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN far pascal _loadds REBOOT_REQUIRED16 ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: CHANGES_PENDING16                                */
/*                                                                   */
/*   Descriptive Name: This function indicates whether or not any    */
/*                     changes were made to the partitioning of the  */
/*                     disks in the system which have not yet been   */
/*                     comitted to disk.                             */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value will be TRUE if there are     */
/*           uncomitted changes to the partitioning of one or more of*/
/*           the drives in the system.                               */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN far pascal _loadds CHANGES_PENDING16 ( void );

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_REBOOT_FLAG16                                */
/*                                                                   */
/*   Descriptive Name: This function sets the Reboot Flag.  The      */
/*                     Reboot Flag is a special flag on the boot     */
/*                     disk used by the install program to keep      */
/*                     track of whether or not the system was just   */
/*                     rebooted.  It is used by the various phases   */
/*                     of install.                                   */
/*                                                                   */
/*   Input: BOOLEAN Reboot - The new value for the Reboot Flag.  If  */
/*                           TRUE, then the reboot flag will be set. */
/*                           If FALSE, then the reboot flag will be  */
/*                           cleared.                                */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to 0 if there are no errors.    */
/*           *Error_Code will be > 0 if an error occurs.             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the value of the Reboot*/
/*                   Flag will be unchanged.                         */
/*                                                                   */
/*   Side Effects:  The value of the Reboot Flag may be changed.     */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_REBOOT_FLAG16( BOOLEAN Reboot, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_REBOOT_FLAG16                                */
/*                                                                   */
/*   Descriptive Name: This function returns the value of the Reboot */
/*                     Flag.  The Reboot Flag is a special flag on   */
/*                     the boot disk used by the install program to  */
/*                     keep track of whether or not the system was   */
/*                     just rebooted.  It is used by the various     */
/*                     phases of install.                            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function return value will be TRUE if no errors     */
/*           occur and the Reboot Flag is set.  *Error_Code will be  */
/*           0 under these conditions.  If an error occurs, the      */
/*           function return value will be FALSE and *Error_Code     */
/*           will be > 0.                                            */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                   The value of the reboot flag will be unchanged. */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN far pascal _loadds GET_REBOOT_FLAG16( CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*                                                                   */
/*   Function Name: SET_INSTALL_FLAGS16                              */
/*                                                                   */
/*   Descriptive Name: This function sets the value of the Install   */
/*                     Flags.  The Install Flags reside in a 32 bit  */
/*                     field in the LVM dataspace.  These flags are  */
/*                     not used by LVM, thereby leaving Install free */
/*                     to use them for whatever it wants.            */
/*                                                                   */
/*   Input: CARDINAL32 Install_Flags - The new value for the Install */
/*                                     Flags.                        */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to 0 if there are no errors.    */
/*           *Error_Code will be > 0 if an error occurs.             */
/*                                                                   */
/*   Error Handling: If an error occurs, then the value of the       */
/*                   Install Flags will be unchanged.                */
/*                                                                   */
/*   Side Effects:  The value of the Install Flags may be changed.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_INSTALL_FLAGS16( CARDINAL32 Install_Flags, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_INSTALL_FLAGS16                              */
/*                                                                   */
/*   Descriptive Name: This function returns the value of the Install*/
/*                     Flags.  The Install Flags reside in a 32 bit  */
/*                     field in the LVM dataspace.  These flags are  */
/*                     not used by LVM, thereby leaving Install free */
/*                     to use them for whatever it wants.            */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: The function returns the current value of the Install   */
/*           Flags stored in the LVM Dataspace.                      */
/*           *Error_Code will be LVM_ENGINE_NO_ERROR if the function */
/*           is successful.  If an error occurs, the function will   */
/*           return 0 and *Error_Code will be > 0.                   */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error_Code will be > 0.    */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds GET_INSTALL_FLAGS16( CARDINAL32 far * Install_Flags, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Min_Install_Size16                           */
/*                                                                   */
/*   Descriptive Name: This function tells the LVM Engine how big a  */
/*                     partition/volume must be in order for it to   */
/*                     marked installable.  If this function is not  */
/*                     used to set the minimum size for an           */
/*                     installable partition/volume, the LVM Engine  */
/*                     will use a default value of 300 MB.           */
/*                                                                   */
/*   Input: CARDINAL32 Min_Sectors - The minimum size, in sectors,   */
/*                                   that a partition must be in     */
/*                                   order for it to be marked as    */
/*                                   installable.                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_MIN_INSTALL_SIZE16 ( CARDINAL32  Min_Sectors );

/*********************************************************************/
/*                                                                   */
/*   Function Name: SET_FREE_SPACE_THRESHOLD16                       */
/*                                                                   */
/*   Descriptive Name: This function tells the LVM Engine not to     */
/*                     report blocks of free space which are less    */
/*                     than the size specified.  The engine defaults */
/*                     to not reporting blocks of free space which   */
/*                     are smaller than 2048 sectors (1 MB).         */
/*                                                                   */
/*   Input: CARDINAL32 Min_Sectors - The minimum size, in sectors,   */
/*                                   that a block of free space must */
/*                                   be in order for the LVM engine  */
/*                                   to report it.                   */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None required.                                  */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds SET_FREE_SPACE_THRESHOLD16( CARDINAL32  Min_Sectors );

/*********************************************************************/
/*                                                                   */
/*   Function Name: READ_SECTORS16                                   */
/*                                                                   */
/*   Descriptive Name: This function reads one or more sectors from  */
/*                     the specified drive and places the data read  */
/*                     in Buffer.                                    */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    read from.  The drives in the  */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to read from.     */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       read into memory.           */
/*          ADDRESS Buffer : The location to put the data read into. */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, then the data read will be placed in     */
/*              memory starting at Buffer, and *Error will be        */
/*              LVM_ENGINE_NO_ERROR.                                 */
/*           If Unsuccessful, then *Error will be > 0 and the        */
/*              contents of memory starting at Buffer is undefined.  */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be read into memory starting at Buffer.  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds READ_SECTORS16 ( CARDINAL32          Drive_Number,
LBA                 Starting_Sector,
CARDINAL32          Sectors_To_Read,
void far *          Buffer,
CARDINAL32 far *    Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name: WRITE_SECTORS16                                  */
/*                                                                   */
/*   Descriptive Name: This function writes data from memory to one  */
/*                     or more sectors on the specified drive.       */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    write to.  The drives in the   */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to write to.      */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       be written.                 */
/*          ADDRESS Buffer : The location of the data to be written  */
/*                           to disk.                                */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, then the data at Buffer will be placed   */
/*              on the disk starting at the sector specified, and    */
/*              *Error will be LVM_ENGINE_NO_ERROR.                  */
/*           If Unsuccessful, then *Error will be > 0 and the        */
/*              contents of the disk starting at sector              */
/*              Starting_Sector is undefined.                        */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be written to disk.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds WRITE_SECTORS16 ( CARDINAL32          Drive_Number,
LBA                 Starting_Sector,
CARDINAL32          Sectors_To_Write,
void far *          Buffer,
CARDINAL32 far *    Error);

/*********************************************************************/
/*                                                                   */
/*   Function Name: REDISCOVER_PRMS16                                */
/*                                                                   */
/*   Descriptive Name: Causes OS2LVM and OS2DASD to check PRMs for   */
/*                     new or changed media.                         */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the Rediscover PRM operation was successful, then    */
/*           *Error_Code will be LVM_ENGINE_NO_ERROR.  If there      */
/*           was an error, then *Error_Code will be > 0.             */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  New volumes may be discovered and assigned drive */
/*                  letters by OS2LVM and OS2DASD.                   */
/*                                                                   */
/*   Notes: The LVM Engine must be CLOSED when this function is      */
/*          called as this function is disabled while it is open!    */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds REDISCOVER_PRMS16( CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: GET_LVM_VIEW16                                   */
/*                                                                   */
/*   Descriptive Name:  This function gets the OS2LVM data for the   */
/*                      specified drive letter.  The intent is to    */
/*                      allow the determination of what drive letter */
/*                      a volume really has given the possibilities  */
/*                      of conflict or a drive preference of '*'.    */
/*                                                                   */
/*   Input:  char  IFSM_Drive_Letter : The drive letter for which the*/
/*                                     OS2LVM data is requested.     */
/*           CARDINAL32 * Drive_Number : The address of a variable   */
/*                                       to hold the OS/2 drive      */
/*                                       number of the drive         */
/*                                       containing the first        */
/*                                       partition of the volume     */
/*                                       currently assigned to the   */
/*                                       requested drive letter.     */
/*           CARDINAL32 * Partition_LBA : The address of a variable  */
/*                                        to hold the LBA of the     */
/*                                        first partition of the     */
/*                                        volume currently assigned  */
/*                                        to the requested drive     */
/*                                        letter.                    */
/*           char * LVM_Drive_Letter : The address of a variable to  */
/*                                     hold the drive letter that    */
/*                                     OS2LVM thinks the volume      */
/*                                     assigned to the requested     */
/*                                     drive letter should have.     */
/*           BYTE * UnitID : The address of a variable to hold the   */
/*                           OS2LVM unit ID for the volume associated*/
/*                           with the requested drive letter.        */
/*                                                                   */
/*   Output:  The function return value will be TRUE if the function */
/*            completed successfully.                                */
/*                                                                   */
/*   Error Handling: If this function fails, the specified drive     */
/*                   letter is either not in use, or is in use by a  */
/*                   device not controlled by OS2LVM.                */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes: This function can be used with the LVM Engine open or    */
/*          closed.                                                  */
/*                                                                   */
/*********************************************************************/
BOOLEAN far   pascal _loadds GET_LVM_VIEW16( char IFSM_Drive_Letter,
CARDINAL32 far * Drive_Number,
CARDINAL32 far * Partition_LBA,
char far *       LVM_Drive_Letter,
BYTE far *       UnitID);

/*********************************************************************/
/*                                                                   */
/*   Function Name: Start_Logging16                                  */
/*                                                                   */
/*   Descriptive Name: Enables the LVM Engine logging.  Once enabled,*/
/*                     the LVM Engine logging function will log all  */
/*                     LVM Engine activity to the specified log file.*/
/*                     The data is logged in a binary format for     */
/*                     compactness and speed.                        */
/*                                                                   */
/*   Input: char * Filename - The filename of the file to use as the */
/*                            log file.                              */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If the logging file was successfully created, then      */
/*           *Error_Code will be 0.  If the log file could not be    */
/*           created, then *Error_Code will be > 0.                  */
/*                                                                   */
/*   Error Handling: If the log file can not be created, then        */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  A file may be created/opened for logging of      */
/*                  LVM Engine actions.                              */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds START_LOGGING16( char far * Filename, CARDINAL32 far * Error_Code );

/*********************************************************************/
/*                                                                   */
/*   Function Name: Stop_Logging16                                   */
/*                                                                   */
/*   Descriptive Name: This function ends LVM Engine logging and     */
/*                     closes the log file.                          */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be 0 if this function completes        */
/*           successfully; otherwise it will be > 0.                 */
/*                                                                   */
/*   Error Handling: If the log file is not currently opened, or if  */
/*                   the close operation fails on the log file, then */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  The log file may be closed.                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds STOP_LOGGING16 ( CARDINAL32 far *  Error_Code );


/*********************************************************************/
/*                                                                   */
/*   Function Name: Export_Configuration16                           */
/*                                                                   */
/*   Descriptive Name: This function creates a file containing all of*/
/*                     the LVM.EXE commands necessary to recreate the*/
/*                     current partition/volume layout.              */
/*                                                                   */
/*   Input: char * Filename - FQN of the output file to be created   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: A file creating LVM commands will be created.           */
/*           *Error_Code will be 0 if this function completes        */
/*           successfully; otherwise it will be > 0.                 */
/*                                                                   */
/*   Error Handling: If the output file cannot be created, or if     */
/*                   the close operation fails on the log file, then */
/*                   *Error_Code will be > 0.                        */
/*                                                                   */
/*   Side Effects:  A file may be created.                           */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void far pascal _loadds EXPORT_CONFIGURATION16( char far * Filename,
                                                CARDINAL32 far * Error_Code );

#endif

#endif
