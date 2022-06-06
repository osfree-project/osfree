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
 * Module: Drive_Linking_Feature.h
 */ 

/*
 * Change History:
 * 
 */

/*
 * Description: Describes the on-disk data structures used for drive linking.
 *
 */


#ifndef DRIVE_LINKING_STRUCTURES

#define DRIVE_LINKING_STRUCTURES 1

#include "gbltypes.h"
#include "lvm_types.h"

/* The following defines uniquely identify Drive Linking. */
#define DRIVE_LINKING_FEATURE_ID  100
#define DRIVE_LINKING_MAJOR_VERSION  2
#define DRIVE_LINKING_MINOR_VERSION  1


/* The following definitions are used for the disk structures supporting drive linking. */

#define LINK_TABLE_MASTER_SIGNATURE  0x434E4157L
#define LINK_TABLE_SIGNATURE         0X4D4D5652L

#define MAXIMUM_LINKS   246

#define DRIVE_LINKING_RESERVED_SECTOR_COUNT 4

#define LINKS_IN_FIRST_SECTOR 60

#define LINKS_IN_NEXT_SECTOR  62

typedef struct _Drive_Link {
                              DoubleWord       Drive_Serial_Number;
                              DoubleWord       Partition_Serial_Number;
                           } Drive_Link;

typedef struct _LVM_Link_Table_First_Sector {
                                               DoubleWord         Link_Table_Signature;              /* Use the LINK_TABLE_MASTER_SIGNATURE here. */
                                               DoubleWord         Link_Table_CRC;
                                               DoubleWord         Sequence_Number;                   /* Used to resolve conflicts when the primary and secondary tables do not match. */
                                               DoubleWord         Links_In_Use;
                                               Drive_Link         Link_Table[LINKS_IN_FIRST_SECTOR];
                                               DoubleWord         Aggregate_Serial_Number;           /* Used in version 2 and later.  */
                                               LVM_Classes        Actual_Class;                      /* Assigned by LVM.  This should always be Aggregate_Class or something wrong! */
                                               BOOLEAN            Top_Of_Class;                      /* Assigned by LVM.  */
                                               CARDINAL32         Feature_Sequence_Number;           /* Assigned by LVM.  */
                                            } LVM_Link_Table_First_Sector;

typedef struct _LVM_Link_Table_Sector {
                                         DoubleWord         Link_Table_Signature;   /* Use LINK_TABLE_SIGNATURE here. */
                                         DoubleWord         Link_Table_CRC;
                                         DoubleWord         Sequence_Number;        /* Used to resolve conflicts when the primary and secondary tables do not match. */
                                         Drive_Link         Link_Table[LINKS_IN_NEXT_SECTOR];
                                      } LVM_Link_Table_Sector;

typedef struct _DLink_Partition_Array{
                                       ADDRESS    Partition_Handle;   /* The external handle for the partition. This is the handle to use with LVM Engine API's.  */
                                       ADDRESS    Reserved;
                                       INTEGER32  Aggregate_Number;   /* Initialized to -1.  GUI will set this field to a non-negative value. */
                                       BOOLEAN    Use_Partition_Name;
                                       char       Partition_Name[PARTITION_NAME_SIZE];
                                       char       reserved;
                                       BOOLEAN    Use_Drive_Number;
                                       char       Drive_Name[DISK_NAME_SIZE];
                                       char       reserved1;
                                       CARDINAL32 Drive_Number;
                                       char       Aggregate_Name[PARTITION_NAME_SIZE];
                                       char       reserved2;
                                     } DLink_Partition_Array_Entry;

#define DLINK_PARTITION_ARRAY_ENTRY_TAG  9865468

typedef struct _DLink_Comm_Struct {
                                    CARDINAL32                  Count;            /* # of entries in the Partition Array. */
                                    CARDINAL32                  Sequence_Number;
                                    DLink_Partition_Array_Entry Partition_Array[1];
                                  } DLink_Comm_Struct;

#endif

