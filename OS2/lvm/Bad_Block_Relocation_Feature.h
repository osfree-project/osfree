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

#ifndef  BBR_DATA_STRUCTURES

#define BBR_DATA_STRUCTURES

#include "lvm_gbls.h"

/* The following definition is the numeric ID for Bad Block Relocation.  */
#define BBR_FEATURE_ID  101

#define BBR_FEATURE_MAJOR_VERSION       0x0002
#define BBR_FEATURE_MINOR_VERSION       0x0000

/* The following definitions are used for the disk structures supporting bad block relocation. */

/* NOTE: BBR stands for Bad Block Relocation. */

#define BBR_TABLE_MASTER_SIGNATURE  0x00726D62
#define BBR_TABLE_SIGNATURE         0x01726276


typedef struct _BBR_Table_Entry {
                                   CARDINAL32           BadSector;
                                   CARDINAL32           ReplacementSector;
                                } BBR_Table_Entry;

typedef struct _LVM_BBR_Table_First_Sector {
                                              CARDINAL32          Signature;                   /* Signature for the first sector of the BBR Table. Use BBR_TABLE_MASTER_SIGNATURE here. */
                                              CARDINAL32          CRC;                         /* CRC for this sector.  */
                                              CARDINAL32          Sequence_Number;             /* Used to resolve conflicts when the primary and secondary tables do not match. */
                                              CARDINAL32          Table_Size;                  /* The number of BBR_Table_Entries in the BBR Table. */
                                              CARDINAL32          Table_Entries_In_Use;        /* The number of BBR Table entries which are in use. */
                                              CARDINAL32          Sectors_Per_Table;           /* The number of LVM_BBR_Table_Sectors used to hold the BBR Table. */
                                              CARDINAL32          First_Replacement_Sector;    /* The location of the first replacement sector. */
                                              CARDINAL32          Last_Replacement_Sector;     /* The location of the last replacement sector. */
                                              CARDINAL32          Replacement_Sector_Count;    /* The number of replacement sectors. */
                                              CARDINAL32          Flags;                       /* Flags global to the Bad Block Relocation Feature. */
                                              LVM_Classes         Actual_Class;                /* Assigned by LVM.  This should always be Partition_Class or something is wrong! */
                                              BOOLEAN             Top_Of_Class;                /* Assigned by LVM.  */
                                              CARDINAL32          Feature_Sequence_Number;     /* Assigned by LVM.  This should always be or something is wrong! */
                                           } LVM_BBR_Table_First_Sector;

/*  Flags for LVM_BBR_Table_First_Sector  */
#define BBR_Flag_Write_Verify    0x00000001   // Indicate convert Write I/O to Write/Verify

#define BBR_TABLE_ENTRIES_PER_SECTOR   62

typedef struct _LVM_BBR_Table_Sector {
                                        CARDINAL32          Signature;                /* Signature for a sector of the BBR_Table which is not the first sector of the BBR Table. Use BBR_TABLE_SIGNATURE here. */
                                        CARDINAL32          CRC;                      /* CRC for this sector of the BBR Table. */
                                        CARDINAL32          Sequence_Number;          /* Used to resolve conflicts when the primary and secondary tables do not match. */
                                        BBR_Table_Entry     BBR_Table[BBR_TABLE_ENTRIES_PER_SECTOR];
                                        CARDINAL32          reserved1;                /* for block alignment */
                                     } LVM_BBR_Table_Sector;

//
// Combined structure to hold entire BBR feature data as it exists on disk.
typedef struct _LVM_BBR_Feature
{
   LVM_BBR_Table_First_Sector  control;
   char                        reserved1[BYTES_PER_SECTOR - sizeof(LVM_BBR_Table_First_Sector)];
   LVM_BBR_Table_Sector        remap[1];
} LVM_BBR_Feature;

/* The following defines establish the minimum and maximum number of replacement sectors which can be allocated for
   Bad Block Relocation.  Otherwise, 1 replacement sector per MB of disk space is allocated.                          */
#define BBR_FLOOR    62
#define BBR_LIMIT  4096

#endif

