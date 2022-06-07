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
 * Module: lvm_data.h
 */ 

/*
 * Change History:
 * 
 */

/*
 * Description:  This module defines the disk structures used by LVM,
 *               including that of the Master Boot Record (MBR) and
 *               Extended Boot Records (EBR).
 *
 * Notes: LVM Drive Letter Assignment Tables (DLA_Tables) appear on the
 *        last sector of each track containing a valid MBR or EBR.  Since
 *        partitions must be track aligned, any track containing an MBR or
 *        EBR will be almost all empty sectors.  We will grab the last
 *        of these empty sectors for our DLT_Tables.
 *
 */

/* XLATOFF */
#ifndef LVM_DATA_H_INCLUDED

#define LVM_DATA_H_INCLUDED  1
/* XLATON */

/* XLATOFF */
#include "LVM_Gbls.h"
/* XLATON */

#include "LVM_Cons.h"

/* XLATOFF */
#include "LVM_Type.h"
/* XLATON */

/*--------------------------------------------------
 * Type definitions
 --------------------------------------------------*/

/* The following definitions define the drive letter assignment table used by LVM.
   For each partition table on the disk, there will be a drive letter assignment table in the last sector
   of the track containing the partition table. */

/* NOTE: DLA stands for Drive Letter Assignment. */

#define DLA_TABLE_SIGNATURE1  0x424D5202L
#define DLA_TABLE_SIGNATURE2  0x44464D50L


typedef struct _DLA_Entry { /* DE */
                             DoubleWord  Volume_Serial_Number;                        /* The serial number of the volume that this partition belongs to. */
                             DoubleWord  Partition_Serial_Number;                     /* The serial number of this partition. */
                             DoubleWord  Partition_Size;                              /* The size of the partition, in sectors. */
                             LBA         Partition_Start;                             /* The starting sector of the partition. */
                             BOOLEAN     On_Boot_Manager_Menu;                        /* Set to TRUE if this volume/partition is on the Boot Manager Menu. */
                             BOOLEAN     Installable;                                 /* Set to TRUE if this volume is the one to install the operating system on. */
                             char        Drive_Letter;                                /* The drive letter assigned to the partition. */
                             BYTE        Reserved;
                             char        Volume_Name[VOLUME_NAME_SIZE];               /* The name assigned to the volume by the user. */
                             char        Partition_Name[PARTITION_NAME_SIZE];         /* The name assigned to the partition. */
                          } DLA_Entry;

typedef struct _DLA_Table_Sector { /* DTS */
                                    DoubleWord     DLA_Signature1;             /* The magic signature (part 1) of a Drive Letter Assignment Table. */
                                    DoubleWord     DLA_Signature2;             /* The magic signature (part 2) of a Drive Letter Assignment Table. */
                                    DoubleWord     DLA_CRC;                    /* The 32 bit CRC for this sector.  Calculated assuming that this field and all unused space in the sector is 0. */
                                    DoubleWord     Disk_Serial_Number;         /* The serial number assigned to this disk. */
                                    DoubleWord     Boot_Disk_Serial_Number;    /* The serial number of the disk used to boot the system.  This is for conflict resolution when multiple volumes
                                                                                  want the same drive letter.  Since LVM.EXE will not let this situation happen, the only way to get this situation
                                                                                  is for the disk to have been altered by something other than LVM.EXE, or if a disk drive has been moved from one
                                                                                  machine to another.  If the drive has been moved, then it should have a different Boot_Disk_Serial_Number.  Thus,
                                                                                  we can tell which disk drive is the "foreign" drive and therefore reject its claim for the drive letter in question.
                                                                                  If we find that all of the claimaints have the same Boot_Disk_Serial_Number, then we must assign drive letters on
                                                                                  a first come, first serve basis.                                                                                      */
                                    CARDINAL32     Install_Flags;              /* Used by the Install program. */
                                    CARDINAL32     Cylinders;
                                    CARDINAL32     Heads_Per_Cylinder;
                                    CARDINAL32     Sectors_Per_Track;
                                    char           Disk_Name[DISK_NAME_SIZE];  /* The name assigned to the disk containing this sector. */
                                    BOOLEAN        Reboot;                     /* For use by Install.  Used to keep track of reboots initiated by install. */
                                    BYTE           Reserved[3];                /* Alignment. */
                                    DLA_Entry      DLA_Array[4];               /* These are the four entries which correspond to the entries in the partition table. */
                                 } DLA_Table_Sector;


/* The following definitions define the LVM signature sector which will appear as the last sector in an LVM partition. */


#define  LVM_PRIMARY_SIGNATURE   0x4A435332L
#define  LVM_SECONDARY_SIGNATURE 0x4252444BL

/* Put major version back to 1 to match current OS/2 implementation */
#define  CURRENT_LVM_MAJOR_VERSION_NUMBER   1        /* Define as appropriate. */
#define  CURRENT_LVM_MINOR_VERSION_NUMBER   0        /* Define as appropriate. */


/* The following definitions limit the number of LVM features that can be applied to a volume, as well as defining a "NULL" feature for use in feature table entries that are not being used. */
#define  MAX_FEATURES_PER_VOLUME  10     /* The maximum number of LVM features that can be applied to a volume. */
#define  NULL_FEATURE              0     /* No feature.  Used in all unused entries of the feature array in the LVM Signature sector. */


/* The following structure is used to hold the location of the feature specific data for LVM features. */
typedef struct _LVM_Feature_Data { /* LFD */
                                   DoubleWord     Feature_ID;                            /* The ID of the feature. */
                                   PSN            Location_Of_Primary_Feature_Data;      /* The PSN of the starting sector of the private data for this feature. */
                                   PSN            Location_Of_Secondary_Feature_Data;    /* The PSN of the starting sector of the backup copy of the private data for this feature. */
                                   DoubleWord     Feature_Data_Size;                     /* The number of sectors used by this feature for its private data. */
                                   Word           Feature_Major_Version_Number;          /* The integer portion of the version number of this feature. */
                                   Word           Feature_Minor_Version_Number;          /* The decimal portion of the version number of this feature. */
                                   BOOLEAN        Feature_Active;                        /* TRUE if this feature is active on this partition/volume, FALSE otherwise. */
                                   BYTE           Reserved[3];                           /* Alignment. */
                                 } LVM_Feature_Data;


/* The following structure defines the LVM Signature Sector.  This is the last sector of every partition which is part of an LVM volume.  It gives vital
   information about the version of LVM used to create the LVM volume that it is a part of, as well as which LVM features (BBR, drive linking, etc.) are
   active on the volume that this partition is a part of.                                                                                                   */
typedef struct _LVM_Signature_Sector { /* LSS */
                                        DoubleWord        LVM_Signature1;                             /* The first part of the magic LVM signature. */
                                        DoubleWord        LVM_Signature2;                             /* The second part of the magic LVM signature. */
                                        DoubleWord        Signature_Sector_CRC;                       /* 32 bit CRC for this sector.  Calculated using 0 for this field. */
                                        DoubleWord        Partition_Serial_Number;                    /* The LVM assigned serial number for this partition.  */
                                        LBA               Partition_Start;                            /* LBA of the first sector of this partition. */
                                        LBA               Partition_End;                              /* LBA of the last sector of this partition. */
                                        DoubleWord        Partition_Sector_Count;                     /* The number of sectors in this partition. */
                                        DoubleWord        LVM_Reserved_Sector_Count;                  /* The number of sectors reserved for use by LVM. */
                                        DoubleWord        Partition_Size_To_Report_To_User;           /* The size of the partition as the user sees it - i.e. (the actual size of the partition - LVM reserved sectors) rounded to a track boundary. */
                                        DoubleWord        Boot_Disk_Serial_Number;                    /* The serial number of the boot disk for the system.  If the system contains Boot Manager, then this is the serial number of the disk containing the active copy of Boot Manager. */
                                        DoubleWord        Volume_Serial_Number;                       /* The serial number of the volume that this partition belongs to. */
                                        CARDINAL32        Fake_EBR_Location;                          /* The location, on disk, of a Fake EBR, if one has been allocated. */
                                        Word              LVM_Major_Version_Number;                   /* Major version number of the LVM that created this partition. */
                                        Word              LVM_Minor_Version_Number;                   /* Minor version number of the LVM that created this partition. */
                                        char              Partition_Name[PARTITION_NAME_SIZE];        /* User defined partition name. */
                                        char              Volume_Name[VOLUME_NAME_SIZE];              /* The name of the volume that this partition belongs to. */
                                        LVM_Feature_Data  LVM_Feature_Array[MAX_FEATURES_PER_VOLUME]; /* The feature array.  This indicates which LVM features, if any, are active on this volume
                                                                                                         and what order they should be applied in.                                                  */
                                        char              Drive_Letter;                               /* The drive letter assigned to the volume that this partition is part of. */
                                        BOOLEAN           Fake_EBR_Allocated;                         /* If TRUE, then a fake EBR has been allocated. */
                                        char              Comment[COMMENT_SIZE];                      /* User comment. */
                                        char              Disk_Name[DISK_NAME_SIZE];                  /* Added to allow BBR to report the name of a disk when bad sectors are encountered on that disk. */
/* These next two fields are specific to LVM Version 2 and are not used in the OS/2 implementation */
/*                                        
 *                                      CARDINAL32        Sequence_Number;                            /* This indicates the order that partitions within a volume are used.  This number is 1 based.  A 0 here indicates that the volume was made by LVM Ver. 1. */
 *                                      CARDINAL32        Next_Aggregate_Number;                      /* Used during volume creation and expansion when creating unique names for aggregates. */
 */
                                        /* The remainder of the sector is reserved for future use and should be all zero or else the CRC will not come out correctly. */
                                     } LVM_Signature_Sector;


/* The following definitions define the format of a partition table and the Master Boot Record (MBR). */
typedef struct _Partition_Record { /* PR */
                                    Byte       Boot_Indicator;    /* 80h = active partition. */
                                    Byte       Starting_Head;
                                    Byte       Starting_Sector;   /* Bits 0-5 are the sector.  Bits 6 and 7 are the high order bits of the starting cylinder. */
                                    Byte       Starting_Cylinder; /* The cylinder number is a 10 bit value.  The high order bits of the 10 bit value come from bits 6 & 7 of the Starting_Sector field. */
                                    Byte       Format_Indicator;  /* An indicator of the format/operation system on this partition. */
                                    Byte       Ending_Head;
                                    Byte       Ending_Sector;
                                    Byte       Ending_Cylinder;
                                    DoubleWord Sector_Offset;     /* The number of sectors on the disk which are prior to the start of this partition. */
                                    DoubleWord Sector_Count;      /* The number of sectors in this partition. */
                                 } Partition_Record;

typedef struct _Master_Boot_Record { /* MBR */
                                      Byte                Reserved[446];
                                      Partition_Record    Partition_Table[4];
                                      Word                Signature;            /* AA55h in this field indicates that this is a valid partition table/MBR. */
                                   } Master_Boot_Record;

typedef Master_Boot_Record  Extended_Boot_Record;

/* The following definition covers the Boot Manager Alias Table in the EBR.

   The Alias Table in the EBR has 2 entries in it, although only the first one is actually used.  */
#define ALIAS_NAME_SIZE  8
typedef struct _AliasTableEntry { /* ATE */
                                  BOOLEAN     On_Boot_Manager_Menu;
                                  char        Name[ALIAS_NAME_SIZE];
                                } AliasTableEntry;

#define ALIAS_TABLE_OFFSET  0x18A

/* XLATOFF */
/* The following text is used for the Boot Manager Alias for items that were placed on the Boot Manager Menu by FDISK and
   which have since been migrated to the new LVM format.  This text is put into the Name field of an AliasTableEntry so
   that, if FDISK ( or another program which understands the old Boot Manager Menu format) is run, it will display
   something for those partitions/volumes which are on the Boot Manager Menu.

   NOTE: This text must be exactly ALIAS_NAME_SIZE characters in length!                                                     */
#define ALIAS_TABLE_ENTRY_MIGRATION_TEXT       "--> LVM "
#define ALIAS_TABLE_ENTRY_MIGRATION_TEXT2      "--> LVM*"

/* XLATON */

/* The following is the signature used for an Master Boot Record, an Extended Boot Record, and a Boot Sector. */
#define MBR_EBR_SIGNATURE  0xAA55


/* The following list of definitions defines the values of interest for the Format_Indicator in a Partition_Record. */
#define EBR_INDICATOR                          0x5
#define WINDOZE_EBR_INDICATOR                  0xF
#define UNUSED_INDICATOR                       0x0
#define IFS_INDICATOR                          0x7
#define FAT12_INDICATOR                        0x1
#define FAT16_SMALL_PARTITION_INDICATOR        0x4
#define FAT16_LARGE_PARTITION_INDICATOR        0x6
#define FAT16X_LARGE_PARTITION_INDICATOR       0xE
#define BOOT_MANAGER_HIDDEN_PARTITION_FLAG     0x10
#define LVM_PARTITION_INDICATOR                0x35
#define BOOT_MANAGER_INDICATOR                 0x0A


/* The following is the signature used in the Boot Sector for Boot Manager. */
#define BOOT_MANAGER_SIGNATURE       "APJ&WN"


/* The following is used for determining the synthetic geometry reported for Volumes employing drive linking. */
#define SYNTHETIC_SECTORS_PER_TRACK  63


/* XLATOFF */
#endif
/* XLATON */
