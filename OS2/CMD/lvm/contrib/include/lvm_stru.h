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
 * Module: Bad_Block_Relocation_Feature.h
 */ 

/*
 * Change History:
 * 
 */

/*
 * Description:  This header file defines structures used throughout the
 *               LVM Engine.
 *
 */

#ifndef LVM_ENGINE_STRUCTURES

#define LVM_ENGINE_STRUCTURES  1

#include "lvm_gbls.h"      /* CARDINAL32, CARDINAL16, ADDRESS, BOOLEAN */

#include "lvm_cons.h"

#include "lvm_type.h"

#include "lvm_data.h"

#include "LVM_LIST.H"


/*--------------------------------------------------
 * Type definitions
 --------------------------------------------------*/

/* The following structure is used to describe the geometry of a disk drive. */
typedef struct _Drive_Geometry_Record {
                                         CARDINAL16  Cylinders;
                                         CARDINAL16  Heads;
                                         CARDINAL16  Sectors;
                                      } Drive_Geometry_Record;

/* The following structure is used to track the status of the physical disk drives in the system. */
typedef struct _Disk_Drive_Data 
{
  CARDINAL32  DriveArrayIndex;  /* The Index used to access this Disk_Drive_Data */ 
                                /* record in the DriveArray.                     */
  ADDRESS     External_Handle;  /* The Handle used by those outside of the   */ 
                                /* LVM Engine to access this Drive_Data.     */
  CARDINAL32  Drive_Size;       /* The total number of sectors on the drive. */
  CARDINAL32  Sectors_Per_Cylinder; 
  /* Needed for converting CHS addresses into  LBA addresses. 
     It is used repeatedly,  so calculate it once and store it here. */ 
  CARDINAL32  Cylinder_Limit;   /* The LBA of the first sector above the 1024 cylinder limit. */
  DoubleWord  Drive_Serial_Number;  
     /* The serial number assigned to this drive.  For info. purposes only. */
  DoubleWord  Boot_Drive_Serial_Number;         
     /* The serial number of the boot drive when this disk was partitioned. */
  CARDINAL32  Install_Flags;    /* Reserved for use by the Install program. */
  DLIST       Partitions;       /* A list of the partitions on this drive.  */
  CARDINAL32  Primary_Partition_Count;   
         /* The number of primary partitions on this drive. */
  CARDINAL32  Logical_Partition_Count;          
         /* The number of logical drives on this drive. */
  Drive_Geometry_Record  Geometry;  /* The geometry for this disk drive. */
  BOOLEAN     Cylinder_Limit_Applies;   
         /* TRUE if the 1024 cylinder limit applies to this drive. */
  BOOLEAN     ChangesMade;      /* Set to TRUE if the drive data or the partitioning of this drive has been changed. */
  BOOLEAN     IO_Error;         /* Set to TRUE if an I/O error occurs while attempting to read or write to this drive. */
  BOOLEAN     Corrupt;          /* Set to TRUE if the partitioning information read from the drive is incorrect. */
  BOOLEAN     Record_Initialized;  /* Tracks whether or not this record has been fully initialized. */
  BOOLEAN     Is_PRM;           /* If TRUE, then this drive is a PRM
                                   (Partitionable Removable Media ) device. */
  BOOLEAN     Is_Big_Floppy;    /* Set to TRUE if the drive is formatted as 
                                   a big floppy. */
  BOOLEAN     Reboot_Flag;      /* Used by Install. */
  BOOLEAN     Unusable;         /* If TRUE, then the MBR is not accessible. */
  BOOLEAN     Fake_Volumes_In_Use;              
    /* If TRUE, then "fake" compatibility volumes are being used to represent partitions on this drive. */
  char        Drive_Name[DISK_NAME_SIZE];   /* User assigned name for this disk drive. */
  int LastErrorIOCTL;  //EK
  int LastError;  
  BOOLEAN   NonFatalCorrupt;  
  
} Disk_Drive_Data;

/* The following define is used for the TAG value whenever an item of type Disk_Drive_Data is put into or taken out of a DLIST.
 */
#define DISK_DRIVE_DATA_TAG   8917626

/* An LVM Volume may have several features active upon it.  These features will each have their own context for the volume.
   To keep track of the features and their contexts, a feature chain is built using the following structure:                   
 */
typedef struct _Feature_Context_Data {
                                       Feature_ID_Data  *              Feature_ID;
                                       ADDRESS                         Function_Table;           /* A pointer to the function t
able for this feature. */
                                       ADDRESS                         Data;
                                       DLIST                           Partitions;
                                       struct _Feature_Context_Data  * Old_Context;
                                     } Feature_Context_Data;


/* The following defines the Partition Table Index for new Partitions.  The Partition Table Index is the 0 based number of an e
ntry
   in the Partition Table.  Since partition tables have only 4 entries, then the possible Partition Table Index values are 0 - 
3.  If
   a partition is newly created, then it has no Partition Table Index because it has never had an entry in a Partition Table.  
It will
   be given an entry in a partition table when the LVM Engine does a commit operation.  However, until then, it has no Partitio
n Table
   Index.  To signify this, the value below is used for the Partition Table Index until a real Partition Table Index becomes av
ailable. */
#define NEW_PARTITION_TABLE_INDEX    65535

/* The following enum and structure is used to track the status of partitions which reside on the physical disk drives in the s
ystem. */
typedef enum _Partition_Types 
{
   FreeSpace, /* This partition record describes a block of free space. */
   Partition, /* This partition record describes an actual partition. */
   MBR_EBR,   /* This partition record describes disk space allocated for an MBR or EBR. */
} Partition_Types;

typedef struct _Partition_Data 
{
   ADDRESS                 External_Handle;                             
 /* The handle used by those outside of the LVM Engine to access this Partition_Data. */
   CARDINAL32              Drive_Index;                                 
 /* The index in the DriveArray for the Drive on which this partition resides. */
   ADDRESS                 Drive_Partition_Handle;                      
 /* The handle used to access this record in the Partitions list for the drive containing this partition. */
   ADDRESS                 External_Volume_Handle;                      
 /* The handle used by those outside of the LVM Engine to access the volume that this partition belongs to. */
   ADDRESS                 Volume_Handle;                               
 /* The handle of the volume which this partition is a part of.  NULL if not part of a volume. */
   ADDRESS                 Parent_Handle;                               
 /* The external handle of the volume or aggregate that this partition is a child of. */
   LVM_Signature_Sector *  Signature_Sector;                            
 /* If this is an LVM volume, then this will point to the LVM Signature Sector for the volume. */
   Feature_Context_Data *  Feature_Data;                                
 /* Used by the feature controlling this partition record. */
   CARDINAL32              Feature_Index;                               
 /* Used during the discovery process to track which entry in the LVM_Feature_Array (in the LVM Signature Sector) is being processed. */
   LBA                     Starting_Sector;  
 /* The LBA of the first sector in this partition. */
   CARDINAL32              Partition_Size;                              
 /* The number of sectors in this partition. */
   CARDINAL32              Usable_Size;                                 
 /* The number of sectors in this partition which are available to the filesystem and not used for overhead. */
   Partition_Record        Partition_Table_Entry; 
 /* The partition table entry for this partition. */
   DLA_Entry               DLA_Table_Entry;  
 /* The drive letter assignment table entry for this partition. */
   CARDINAL16              Partition_Table_Index;  
 /* The index in the Partition Table where the partition table entry for this 
    partition resides. If this Partition_Data structure is for a new partition, 
    then this field will be NEW_PARTITION_TABLE_INDEX. 
 */
    char                    Partition_Name[PARTITION_NAME_SIZE]; 
 /* The user assigned name for this partition. */
    char                    File_System_Name[FILESYSTEM_NAME_SIZE];      
 /* The name of the filesystem in use on this partition, if it is known. */
    Partition_Types         Partition_Type;                              
 /* What does this partition data record describe? */
    BOOLEAN                 New_Partition;                               
 /* TRUE if the partition was created during this LVM Engine session.  FALSE otherwise. */
    BOOLEAN                 Primary_Partition;                           
 /* Set to TRUE if this partition data record describes a primary partition. */
    BOOLEAN                 Spanned_Volume;
 /* Set to TRUE if this partition is part of a spanned volume. */
    BOOLEAN                 Migration_Needed;                            
 /* Used to track the migration of old style Boot Manager Menu entries to the new LVM style entries. */
 
 } Partition_Data;

/* The following define is used for the TAG value whenever an item of type Partition_Data is put into or taken out of a DLIST. 
*/
#define PARTITION_DATA_TAG   192837


/* The following structure is used to track filesystem expansion DLLs. */
typedef struct _LVM_Expansion_DLL_Record 
{
    HMODULE    Expansion_DLL_Handle;
    BOOLEAN    (* _System Quiesce) ( char Drive_Letter);
    BOOLEAN    (* _System Resume) ( char Drive_Letter );
    char       Expansion_DLL_Name[CCHMAXPATH];
} LVM_Expansion_DLL_Record;


#define LVM_EXPANSION_DLL_TAG  848302

/* The following structure is used to track the status of Volumes in the system. */
typedef struct _Volume_Data 
{
   DoubleWord     Volume_Serial_Number; /* The serial number assigned to this volume. */
   ADDRESS        External_Handle; /* The handle used by those outside of the LVM Engine to access this Volume_Data. */
   ADDRESS        Volume_Handle;   /* The handle of this volume in the Volumes list. */
   CARDINAL32     Volume_Size;     /* The number of sectors comprising the volume. */
   CARDINAL32     Partition_Count; /* The number of partitions which comprise this volume. */
   CARDINAL32     Drive_Letter_Conflict;                        
/* 0 indicates that the drive letter preference for this volume is unique.
   1 indicates that the drive letter preference for this volume
     is not unique, but this volume got its preferred drive letter anyway.
   2 indicates that the drive letter preference for this volume
     is not unique, and this volume did NOT get its preferred drive letter.
   4 indicates that this volume is currently "hidden" - i.e. it has
     no drive letter preference at the current time. */
   Partition_Data *   Partition;                                    
   /* The partition or aggregate that this volume is made of. */
   CARDINAL32         Next_Aggregate_Number;                        
   /* The number to use when forming a name for the next aggregate created.  
   Used during volume creation and expansion. */
   LVM_Expansion_DLL_Record *  Expansion_DLL_Data;                           
   /* This points to the expansion DLL of the filesystem for which this volume 
      has been formatted. */
   BOOLEAN      New_Volume;                                   
   /* If TRUE, then this volume did not exist prior to this run of the LVM Engine. */
   BOOLEAN      Compatibility_Volume;                         
   /* TRUE indicates that this volume is compatible with older versions of OS/2.
      FALSE indicates that this is an LVM specific volume and can not be used without OS2LVM.DMD. */
   BOOLEAN      ChangesMade; /* Set to TRUE if something about this volume was altered. */
   BOOLEAN      Foreign_Volume;   
   /* Set to TRUE if the Boot Drive S/N for this volume does not match that of the drive the system is currently booted from. */
   char          Volume_Name[VOLUME_NAME_SIZE];   
   /* The user assigned name for this volume. */
   char          File_System_Name[FILESYSTEM_NAME_SIZE];    
   /* The name of the file system in use on this volume, if it is known. */
   char                        Drive_Letter_Preference;                      
   /* The drive letter that this volume desires to be. */
   char          Current_Drive_Letter;                         
   /* The drive letter by which this volume can currently be accessed.  
      This may not be the same as its drive preference if there was some 
      kind of conflict, or if the drive preference was a *. */
   char          Initial_Drive_Letter;                         
   /* The drive letter assigned to this volume by the operating system 
      when LVM was started. This may be different from the Drive_Letter_Preference 
      if there were conflicts, and may be different from the Current_Drive_Letter.  
      This will be 0x0 if the Volume did not exist when the LVM Engine was opened 
      (i.e. it was created during this LVM session). */
   BYTE          UnitID;   /* The UnitID of the volume as reported by OS2LVM. */
   BOOLEAN       On_Boot_Manager_Menu;  /* Set to TRUE if this volume is on the Boot Manager Menu. */
   BYTE          Device_Type;  
   /* Indicates what type of device the Volume resides on:
     0 = Hard Drive under LVM Control
     1 = PRM under LVM Control
     2 = CD-ROM
     3 = Network drive
     4 = Unknown device NOT under LVM Control
     See lvm_intr.h for defines representing these values.
   */
    BOOLEAN     Can_Be_Altered;                               
    /* Set to TRUE if the Volume can be altered.  
       Set to FALSE if the volume can not be changed in any way.
       NOTE:  If Can_Be_Altered is FALSE, the volume can NOT be changed, 
       but it can be deleted.              */
    BOOLEAN     Expand_Volume;   
    /* Set to TRUE if Expand_Volume function was run against this volume. */
    BOOLEAN     Filesystem_Is_Expandable;    
    /* Set to TRUE if LVM finds the appropriate DLL to perform an expansion for the filesystem on this volume. */
    BOOLEAN     Quiesce_Resume_Supported;    
    /* Set to TRUE if the filesystem on this volume supports the Quiesce and Resume functions as found in JFS. */
    BOOLEAN     Volume_Is_Quiesced;                           
    /* Set to TRUE if a volume is in the quiesced state.  
       Used during the volume expansion process. */
    BOOLEAN    Convert_To_LVM_V1;                            
    /* Set to TRUE if the current volume needs to be converted to LVM Version 1 format.  
       This should only happen if the current version of LVM was installed as part of 
       a fixpak and the fixpak is being backed out for some reason. */
 } Volume_Data;

/* The following define is used for the TAG value whenever an item of type Volume_Data is put into or taken out of a DLIST. */

#define VOLUME_DATA_TAG   45987612

/* The following structure is used with the KillSector list.  It describes a sector on a drive that needs to be overwritten in

   order to ensure proper operation of the system.  Sectors are typically added to this list when volumes are deleted ( get
   rid of any LVM Signature sectors to avoid confusion and accidental reuse), or partitions created (overwrite the boot sector 
-
   this is how OS/2 determines that a partition is not formatted).                                                             
   */
typedef struct _Kill_Sector_Data {
                                   CARDINAL32   Drive_Index;
                                   LBA          Sector_ID;
                                 } Kill_Sector_Data;

/* The following define is used for the TAG value whenever an item of type Kill_Sector_Data is put into or taken out of a DLIST
. */
#define KILL_SECTOR_DATA_TAG  0x72766B62


/* The following structure is for use with the Duplicate_Check function.  It is used to tell the Duplicate_Check function what 
to
   check for, either duplicate names or duplicate serial numbers.  The Duplicate_Check function is designed to be passed into
   the ForEachItem function of the DLIST module so that, before adding a new Name or S/N, an entire list of items can be checke
d
   to see if it already contains that Name or S/N.                                                                             
     */
typedef struct _Duplicate_Check_Parameter_Record {
                                                   CARDINAL32  New_Name_Length;
                                                   CARDINAL32  Max_Name_Length;
                                                   ADDRESS     Handle;                         /* The external handle of the pa
rtition/volume whose name is being checked. */
                                                   DoubleWord  New_Serial_Number;
                                                   char *      New_Name;
                                                   BOOLEAN     Duplicate_Name_Found;
                                                   BOOLEAN     Duplicate_Serial_Number_Found;
                                                   BOOLEAN     Check_Name;                     /* If TRUE, then a check is perf
ormed for a duplicate name.  If FALSE, then a check is performed for a duplicate S/N. */
                                                   BYTE        Reserved;                       /* Alignment. */
                                                 } Duplicate_Check_Parameter_Record;


#endif
