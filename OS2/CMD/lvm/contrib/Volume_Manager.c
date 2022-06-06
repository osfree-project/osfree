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
 * Module: Volume_Manager.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in Volume_Manager.C:
 *
 *
 * Description: Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *              Volume_Manager.H and Volume_Manager.C implement
 *              the volume management functions found in the
 *              LVM Engine Interface.
 *
 *
 *    Partition_Manager.C                                     Volume_Manager.C
 *            \                                               /
 *              \  |-------------------------------------|  /
 *                \|                                     |/
 *                 |        Engine.H and Engine.C        |
 *                /|                                     |\
 *              /  |-------------------------------------|  \
 *            /                    |                          \
 *    BootManager.C            Logging.C                     Handle_Manager.C
 *
 * Notes: LVM Drive Letter Assignment Tables (DLA_Tables) appear on the
 *        last sector of each track containing a valid MBR or EBR.  Since
 *        partitions must be track aligned, any track containing an MBR or
 *        EBR will be almost all empty sectors.  We will grab the last
 *        of these empty sectors for our DLT_Tables.
 *
 */

#include <stdlib.h>   /* malloc, free */
#include <stdio.h>    /* sprintf */
#include <string.h>   /* strlen */
#include <ctype.h>    /* toupper */

#define NEED_BYTE_DEFINED
#define INCL_DOS
#define INCL_ERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#include "engine.h"   /* Included for access to the global types and variables. */
#include "gbltypes.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */

#include "dlist.h"    /*  */
#include "diskio.h"   /*  */


#include "LVM_Constants.h"   /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */

#include "LVM_Interface.h"   /* */

#include "Handle_Manager.H"    /* Create_Handle, Destroy_Handle, Translate_Handle */

#include "partition_manager.h" /* */

#include "CRC.H"               /* INITIAL_CRC, CalculateCRC */

#include "Bad_Block_Relocation.h"

#include "Pass_Thru.h"

#include "drive_linking_feature.h"

#include "Logging.h"

#include "Volume_Manager.h"

#define _JFS_OS2
#define int32 INTEGER32
#define uint32 CARDINAL32
#include "jfs_cntl.h"   /* JFSCTL_QUIESCE, JFSCTL_RESUME */

#ifdef DEBUG

#ifdef PARANOID

#include <assert.h>   /* assert */

#endif

#endif


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define DELETE_DRIVE_IOCTL   0x6A
#define DP_DEVICEATTR_NON_REMOVABLE     0x0001       /* Set if media is non-removable */
#define DP_DEVICEATTR_PART_REMOVABLE    0x0008       /* Set if media is partitioned removable */
#define HIDDEN_VOLUME_FAILURE_FLAG      0x80000000

/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/
typedef struct _Dynamic_Drives_Data_packet {
                                             UCHAR cmd_info;
                                             UCHAR drive_unit;
                                           } Dynamic_Drive_Data;

typedef struct {
                  BIOSPARAMETERBLOCK  BPB;
                  CARDINAL16          CylinderCount;
                  BYTE                DeviceType;
                  CARDINAL16          DeviceAttributes;
               } DeviceParameterPacket;

typedef struct {
                 CARDINAL32  LVM_Drive_Number;
                 CARDINAL32  LVM_LBA;
                 char        LVM_Drive_Letter;
                 char        IFSM_Drive_Letter;
                 BYTE        UnitID;
                 BOOLEAN     Volume_Found;         /* If TRUE, then the volume was found and updated. */
               } Update_Current_Drive_Letter_Data;

typedef enum _ExtendFS_Commands {
                                  Count_Volumes,         /* Count the number of JFS volumes being expanded. */
                                  Fill,                  /* Fill in the DDI_ExtendRecord for each JFS volume being expanded. */
                                  Complete,              /* Complete the expansion of JFS volumes by running the JFS ExtendFS program. */
                                } ExtendFS_Commands;

typedef struct {

                 ExtendFS_Commands    Command;
                 CARDINAL32           Volume_Count;
                 DDI_ExtendRecord  *  Volume_Expansion_Array;
               } ExtendFS_Control_Data;

typedef struct {
                 CARDINAL32    Count;
                 char          Drive_Letter;
               } Drive_Letter_Count_Record;

typedef struct {
                 CARDINAL32    Volume_Serial_Number;
                 CARDINAL32    Boot_Drive_Serial_Number;
                 DLIST         Partition_List;
               } Potential_Volume_Data;

#define POTENTIAL_VOLUME_DATA_TAG  384335372

typedef struct {
                 Partition_Data *    PartitionRecord;
                 BOOLEAN             Found;
               } Potential_Volume_Search_Data;

typedef struct {
                 DLIST         Partition_List;
                 Volume_Data * VolumeRecord;
                 CARDINAL32    Feature_To_Stop_At;  /* The feature ID of the feature to stop at.  For use during volume expansions.  Set to 0 for Creations. */
                 CARDINAL32 *  Error_Code;
               } Apply_Features_Parameter_Record;


/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static CARDINAL32           Available_Drive_Letters = 0;          /* Used to track which drive letters are in use by volumes. */
static CARDINAL32           Deleted_Drive_Letters = 0;            /* Used to track which drive letters have had changes of significance ( deleted, hidden, expanded ). */
static BYTE                 Buffer[BYTES_PER_SECTOR];             /* Used to hold and validate LVM Signature Sectors. */
static RESULTCODES          Results;                              /* Used with DosExecPgm. */
static char                 ProgramName[] = "extendfs.exe";       /* Used with DosExecPgm to run the extendfs utility. */
static char                 FailureName[80];                      /* Used to hold a failure message from DosExecPgm if DosExecPgm fails. */
static char                 ProgramArgs[] = "extendfs.exe\0*:\0"; /* Used to tell extendfs what drive to operate on when it is invoked using DosExecPgm. */
#define DRIVE_LETTER_INDEX  13
static char *               Java_Call_Back_Class;                 /* Used during the creation of a volume if the user interface is written is Java. */


/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/

static void          _System Transfer_Partition_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Check_For_Corrupt_Drive(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Potential_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Destroy_Embedded_Lists(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static BOOLEAN       _System Kill_Non_LVM_Device_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error);
static void          _System Set_Initial_Drive_Letters(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Check_New_Drive_Letters(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Update_Current_Drive_Letter(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Check_For_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Create_Fake_Volumes_For_PRM(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Add_Fake_Volumes_To_Deleted_Volumes_List(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Current_Drive_Letter_Conflicts(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Drive_Letter_Conflicts(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static INTEGER32     _System Sort_By_Drive_Letter_Preference( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error);
static INTEGER32     _System Sort_By_Current_Drive_Letter( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error);
static Volume_Data * Create_Compatibility_Volume( Partition_Data * PartitionRecord, CARDINAL32 * Error );
static BOOLEAN       Is_Volume_Bootable(Volume_Data * VolumeRecord, BOOLEAN Check_Eligibility_Only, CARDINAL32 * Error_Code);
static Volume_Data * Create_Default_LVM_Volume( CARDINAL32 * Error_Code );
static BOOLEAN       Partition_List_Is_Valid( CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], BOOLEAN No_PRMs, CARDINAL32 * Error_Code );
static BOOLEAN       Allocate_Signature_Sectors( CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], BOOLEAN Make_Fake_EBR, CARDINAL32 * Error_Code );
static BOOLEAN       Free_Signature_Sectors( CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], CARDINAL32 * Error_Code );
static void          _System Complete_Partition_Initialization(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Remove_All_Partition_Features(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Commit_All_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Commit_Feature_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          Update_Partition_Data( Volume_Data * VolumeRecord, CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], CARDINAL32 * Error_Code );
static void          _System Count_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Clear_Boot_Sectors(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static BOOLEAN       Is_Volume_Startable(Volume_Data * VolumeRecord, BOOLEAN Check_Eligibility_Only, CARDINAL32 * Error_Code);
static void          _System Extend_FS(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Clear_Drive_Letter_Fields(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Count_Drive_Letter_Claims(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Reserved_Drive_Letters(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Existing_Potential_Volume(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Process_Potential_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Set_Feature_Index(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static INTEGER32     _System Sort_Partition_List_By_Feature_ID( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error);
static void          _System Discovery_Failed_Remove_Features_From_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static BOOLEAN       Feature_List_Is_Bogus( CARDINAL32 Feature_Count, LVM_Feature_Specification_Record FeaturesToUse[], DLIST Features_List, BOOLEAN * Aggregator_Found, CARDINAL32 * Error_Code);
static void          _System Apply_Features(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Java_Pass_Thru( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code);
static void          _System Build_Features_List(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Update_Parent_Child_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          Set_Volume_Parent_Child_Pointers( Volume_Data * VolumeRecord, CARDINAL32 * Error_Code);
static void          _System Find_Pending_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Set_Partition_Sequence_Numbers(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static INTEGER32     _System Filesystem_Sort( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error);
static void          _System Determine_Expansion_Potential(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Expandable_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_Expansion_DLL(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Find_And_Convert_LVM_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void          _System Check_Feature_List_For_Conversion(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN Initialize_Volume_Manager( CARDINAL32 * Error_Code )
{

  FUNCTION_ENTRY("Initialize_Volume_Manager")

  /* Has the Volumes list been initialized yet? */
  if ( Volumes != NULL )
  {

    /* The Volumes list has already been initialized!  We have an error! */

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#else

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Initialize_Volume_Manager")

    return FALSE;

#endif

#endif

  }

  /* Initialize the Volumes list. */
  Volumes = CreateList();

  if ( Volumes == NULL )
  {

    /* We did not have enough memory to create the Volumes list.  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Initialize_Volume_Manager")

    return FALSE;

  }

  /* Initialize the Aggregates list. */
  Aggregates = CreateList();
  if ( Aggregates == NULL )
  {

    /* Destroy the Volumes list as we don't need it anymore. */
    DestroyList(&Volumes, TRUE, Error_Code);

    /* We did not have enough memory to create the Aggregates list.  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Initialize_Volume_Manager")

    return FALSE;

  }

  /* Initialize Available_Drive_Letters.  Discover_Volumes will mark drive letters as being unavailable as it finds volumes. */
  Available_Drive_Letters = 0x3fffffc;

  /* All done! */

  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Initialize_Volume_Manager")

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Close_Volume_Manager( void )
{

  CARDINAL32  Error;   /* Used to hold the return code from the DestroyList function. */

  FUNCTION_ENTRY("Close_Volume_Manager")

  /* Has the Volumes list been created? */
  if ( Volumes != NULL )
  {

    /* Since a Volume_Data object has an embedded list in it, we must walk the list and destroy these embedded lists
       or else we will have a memory leak.                                                                            */
    ForEachItem( Volumes, &Destroy_Embedded_Lists, NULL, TRUE, &Error);

    /* Destroy the Volumes list as we don't need it anymore. */
    DestroyList(&Volumes, TRUE, &Error);

    /* Ensure that the Volumes list is NULL. */
    Volumes = (DLIST) NULL;

    /* Now eliminate the Aggregates list. */
    DestroyList(&Aggregates, TRUE, &Error);

    /* Ensure that the Volumes list is NULL. */
    Aggregates = (DLIST) NULL;

  }

  /* Clear our global variables. */
  Available_Drive_Letters = 0;
  Deleted_Drive_Letters = 0;


  FUNCTION_EXIT("Close_Volume_Manager")

  return;
}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Discover_Volumes( CARDINAL32 * Error_Code )
{

  DLIST                        Potential_Volumes;      /* Used to hold a list of Potential_Volume_Data structures.  Each of the structures in this list
                                                          represents a potential volume and contains a list of the partitions that claim to be a part of
                                                          that volume.                                                                                   */
  CARDINAL32                   Index;                  /* Used to walk the DriveArray. */
  CARDINAL32                   Error;                  /* Used with list functions. */
  Volume_Data *                Current_Volume = NULL;  /* Used when checking for duplicate drive letter preferences. */
  Partition_Data *             PartitionRecord = NULL; /* Used when checking whether or not the installable flag should remain set on a volume. */
  CARDINAL32                   Name_Count = 1;         /* Used to create unique names for any "fake" volumes being created. */
  BOOLEAN                      Volumes_Found;          /* Used when checking PRMs for Volumes. */

  FUNCTION_ENTRY("Discover_Volumes")

  /* Has the Volumes list been created? */
  if ( Volumes == NULL )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* We have an error!  This module has not been initialized yet! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  /* Create the Potential_Volumes list. */
  Potential_Volumes = CreateList();

  /* Did we succeed? */
  if ( Potential_Volumes == NULL )
  {

    LOG_ERROR("Unable to create the Potential_Volumes list!  Out of memory!")

    /* We must be out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  /* Scan all of the partitions on each drive.  For each partition claiming to be a part of a volume,
     see if a volume with a matching volume serial number and boot drive serial number exists in
     the Potential_Volumes list.  If it does, add the partition to the partition list for that volume,
     else create a new entry in the Potential_Volumes list and add the partition to the partitions
     list.  When we begin to examine features on a volume, we will examine the pass the list of partitions
     for each volume to each feature that the volume claims to have.  If any of the features don't like
     what they find, then volume is not created and is removed from the Potential_Volumes list.  When
     we are done, this list will contain all of the volumes that exist on the disk drives in the system.
     Note:  Features may alter the partitions list for a potential volume ( ex. an aggregator may remove
     several partitions from the list and replace them with an aggregate).                                     */


  LOG_EVENT("Scanning all drives for partitions that may be part of volumes.")

  for ( Index = 0; Index < DriveCount; Index++ )
  {

    LOG_EVENT1("Scanning the current drive.","Drive Number", Index + 1)

    /* Examine all of the partitions on this drive. */
    ForEachItem( DriveArray[Index].Partitions, &Find_Potential_Volumes, Potential_Volumes, TRUE, &Error);

    /* Was there a problem? */
    if ( Error != DLIST_SUCCESS )
    {

      /* Are we out of memory? */
      if ( Error == DLIST_OUT_OF_MEMORY )
      {

        LOG_ERROR("ForEachItem failed.  Return code indicates insufficient memory.")

        /* We are out of memory.  Cleanup and abort. */
        DestroyList(&Potential_Volumes, TRUE, &Error);

        if ( Error != DLIST_SUCCESS )
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
        else
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      }
      else
      {

        LOG_ERROR1("ForEachItem failed!","Error code", Error)

        /* Something went wrong and it is not anything that was expected.  Abort. */
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      }

      FUNCTION_EXIT("Discover_Volumes")

      return;

    }

  }

  LOG_EVENT("Drive scan complete.  Attempting to process the resulting list of partitions.")

  /* Now that we have the list of potential volumes, we need to process each potential volume and determine if they really are volumes. */
  ForEachItem(Potential_Volumes, &Process_Potential_Volumes, NULL, TRUE, &Error);

  if ( Error != DLIST_SUCCESS )
  {

    /* Are we out of memory? */
    if ( Error == DLIST_OUT_OF_MEMORY )
    {

      LOG_ERROR("ForEachItem failed.  Return code indicates insufficient memory.")

      /* We are out of memory.  Cleanup and abort. */
      DestroyList(&Potential_Volumes, FALSE, &Error);

      if ( Error != DLIST_SUCCESS )
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
      else
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    }
    else
    {

      LOG_ERROR1("ForEachItem failed!","Error code", Error)

      /* Something went wrong and it is not anything that was expected.  Abort. */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    }

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  LOG_EVENT("Processing of the Potential_Volumes list has been completed.")

  /* Now that all of the potential volumes have been processed, we can eliminate the Potential_Volumes list.  All of the
     real volumes have been moved to the Volumes list.  Anything remaining in this list is not a volume and can be deleted. */
  DestroyList(&Potential_Volumes, TRUE, &Error);

  /* Was there a problem? */
  if ( Error != DLIST_SUCCESS )
  {

    if ( Error == DLIST_OUT_OF_MEMORY )
    {

      LOG_ERROR("DestroyList failed.  Return code indicates insufficient memory.")

      /* We are out of memory.  */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    }
    else
    {

      LOG_ERROR1("DestroyList failed!","Error code", Error)

      /* Something went wrong and it is not anything that was expected.  Abort. */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    }

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  LOG_EVENT("Setting the Parent_Handle fields of the partitions/aggregates in each volume.")

  /* Now we must set the Parent_Handle fields in each of the partitions or aggregates contained in the Volumes
     listed in the Volumes list.                                                                                */
  ForEachItem(Volumes, &Update_Parent_Child_Data, NULL, TRUE, &Error);
  if ( Error != DLIST_SUCCESS )
  {

    LOG_ERROR1("ForEachItem failed.","Error code", Error)

    /* This should not have failed!  Abort! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  /* Now we must determine which of the volumes discovered so far can be expanded.  Only volumes that are unformatted
     or whose filesystem supports expansion can be expanded.                                                           */

  LOG_EVENT("Examining each volume to determine expandability.")

  /* Sort the list of volumes by filesystem.  This way we can easily do all of the volumes with a given filesystem at one time. */
  SortList(Volumes, &Filesystem_Sort, &Error);
  if ( Error != DLIST_SUCCESS )
  {

    LOG_ERROR1("SortList failed.","Error code", Error)

    /* This should not have failed!  Abort! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  /* Now that the volumes are grouped by filesystem, lets examine each filesystem and determine whether or not it supports expansion. */
  Current_Volume = NULL;
  ForEachItem(Volumes,&Determine_Expansion_Potential, &Current_Volume, TRUE, &Error);
  if ( Error != DLIST_SUCCESS )
  {

    LOG_ERROR1("ForEachItem failed.","Error code", Error)

    /* This should not have failed!  Abort! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

  /* Now we must examine any PRMs in the system.  PRMs are a special case.  PRMs are handled as follows:

        1.  If a PRM contains partitions, but none of those partitions belongs to a Volume, then we will
            create compatibility volumes for each partition on the PRM.

        2.  If a PRM contains partitions, and one or more of the partitions belongs to volumes, then
            we will NOT create compatibility volumes for the partitions which are not part of a volume.

        3.  If a PRM is formatted as a big floppy (the old style support for PRMs), then we will create a
            compatibility volume for the PRM.  However, this volume can never be altered or committed to disk
            as there is no place to put a DLA Table on a big floppy formatted PRM.

     In case #3, the Partition_Manager will flag big floppy formatted PRMs and create a "fake" MBR and partition
     for them.  This allows us to handle them here in the same fashion as cases #1 and #2.  We just need to keep
     track of whether or not a volume represents a big floppy ( or, more generally, whether or not a volume
     can be altered ).                                                                                             */

  LOG_EVENT("Examining any PRMs for volumes/partitions.")

  /* Lets find the PRMs. */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    /* Is the current drive a PRM?  Is the current drive corrupt? Is the current drive usable? */
    if ( DriveArray[Index].Is_PRM && ( ! DriveArray[Index].Corrupt || DriveArray[Index].NonFatalCorrupt ) && ( ! DriveArray[Index].Unusable ) )
    {

      LOG_EVENT1("PRM found.","Drive Number of PRM",Index + 1)

      LOG_EVENT("Checking the PRM for volumes.")

      /* We have a PRM.  Lets see if it has any Volumes. */
      Volumes_Found = FALSE;
      ForEachItem(DriveArray[Index].Partitions,&Check_For_Volumes, &Volumes_Found, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

      assert(Error == DLIST_SUCCESS);

#else

      if ( Error != DLIST_SUCCESS)
      {

        LOG_ERROR1("ForEachItem failed.", "Error code", Error)

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FUNCTION_EXIT("Discover_Volumes")

        return;

      }

#endif

#endif

      /* Were any volumes found? */
      if ( ! Volumes_Found )
      {

        LOG_EVENT("No volumes were found on the PRM.  Attempting to create fake volumes for any partitions on the PRM.")

        /* Since no Volumes were found on this PRM, we must create some "fake" volumes. */
        ForEachItem(DriveArray[Index].Partitions,&Create_Fake_Volumes_For_PRM, &Name_Count, TRUE, &Error);

        /* Did we succeed? */
        if ( Error != DLIST_SUCCESS )
        {

          if ( Error == DLIST_OUT_OF_MEMORY )
          {

            LOG_ERROR("ForEachItem failed.  Return code indicates insufficient memory.")

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          }
          else
          {

            LOG_ERROR1("ForEachItem failed.","Error code",Error)

            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          }

          FUNCTION_EXIT("Discover_Volumes")

          return;

        }

        /* We must indicate that "fake" volumes are in use on this PRM! */
        DriveArray[Index].Fake_Volumes_In_Use = TRUE;

      }
      else
      {

        LOG_EVENT("Volumes were found on the PRM!")

      }

    }

  }

  /* If we are in "Merlin" mode, then we can not reconcile drive letters with drive letter preferences as Merlin
     does not support the calls required.  We will skip this if we are in "Merlin" mode.                            */
  if ( ! Merlin_Mode )
  {

    LOG_EVENT("Attempting to reconcile drive letters with OS2LVM.")

    /* Now we must establish the current drive letter for each Volume.  Furthermore, we must create "fake" volumes
       to represent Non-LVM controlled resources (such as CD-ROMs and Network Drives) which consume a drive letter.
       The Reconcile_Drive_Letters function does all of that for us.                                                 */

    Reserved_Drive_Letters = Reconcile_Drive_Letters( FALSE, Error_Code );

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      LOG_ERROR1("Reconcile_Drive_Letters failed!","Error code",*Error_Code)

      FUNCTION_EXIT("Discover_Volumes")

      /* Something unexpected happened!  Abort. */
      return;

    }

  }

  /* Now we must save the current drive letter of each volume as the original drive letter for each volume.  If we
     are in "Merlin" mode, then this function will set the current drive letter and the initial drive letter
     equal to the drive letter preference.                                                                             */
  ForEachItem(Volumes,&Set_Initial_Drive_Letters, NULL, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

  assert(Error == DLIST_SUCCESS);

#else

  if ( Error != DLIST_SUCCESS)
  {

    LOG_ERROR1("ForEachItem failed.","Error code", Error)

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

#endif

#endif

  /* Once the Volumes list has been constructed, we must check for drive letter conflicts. */

  LOG_EVENT("Checking for drive letter conflicts.")

  /* We will sort the Volumes list based upon drive letter preference.  This way, all volumes with
     duplicate preferences will end up grouped together in the Volumes list.  We can then walk the
     volumes list and resolve who will actually get a drive letter for cases where multiple volumes
     want the same drive letter.                                                                     */
  SortList(Volumes, &Sort_By_Drive_Letter_Preference, &Error);

#ifdef DEBUG

#ifdef PARANOID

  assert(Error == DLIST_SUCCESS);

#else

  if ( Error != DLIST_SUCCESS)
  {

    LOG_ERROR1("SortList failed.","Error code", Error)

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

#endif

#endif

  /* Now we can walk the sorted list looking for duplicate entries. */
  Current_Volume = NULL;
  ForEachItem(Volumes, &Find_Drive_Letter_Conflicts, &Current_Volume, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

  assert(Error == DLIST_SUCCESS);

#else

  if ( Error != DLIST_SUCCESS)
  {

    LOG_ERROR1("ForEachItem failed.","Error code", Error)

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

#endif

#endif

  /* If there is a volume marked installable, we must make sure that it is not "Hidden", and it must have no drive letter conflicts. */
  if ( Install_Volume_Handle != NULL )
  {

    LOG_EVENT("Checking the volume marked installable.")

    /* Get the volume's data. */
    Current_Volume = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Install_Volume_Handle, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

    assert(Error == DLIST_SUCCESS);
    assert( Current_Volume->Compatibility_Volume );
    assert( Current_Volume->Partition_Count == 1 );

#else

    if ( Error != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Discover_Volumes")

      return;

    }

    if ( ( ! Current_Volume->Compatibility_Volume ) ||
         ( Current_Volume->Partition_Count != 1)
       )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Discover_Volumes")

      return;

    }

#endif

#endif

    /* Is the volume hidden?  Does the volume have a drive letter conflict? */
    if ( ( Current_Volume->Drive_Letter_Preference < 'C' ) ||
         ( Current_Volume->Drive_Letter_Preference > 'Z' ) ||
         ( Current_Volume->Drive_Letter_Conflict > 0 )
       )
    {

      LOG_ERROR("The volume marked installable has a drive letter conflict or is hidden!  Turning off the Install Flag!")

      /* This volume can not be marked installable!  Turn off the installable flag. */

      /* The volume must be a compatibility volume.  Lets get its partition data and examine it. */
      PartitionRecord = Current_Volume->Partition;

      /* Now turn off the install flag in the partition's DLA Table Entry. */
      PartitionRecord->DLA_Table_Entry.Installable = FALSE;

      /* Mark the drive that the partition is on as having been changed. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      /* Reset the Install_Volume_Handle. */
      Install_Volume_Handle = NULL;

    }

  }

  LOG_EVENT("Checking each volume to see if there are any changes pending.")

  /* Now we must check each volume to see if their features have any changes pending.  This can happen if one of the two
     copies of the feature data for a volume is corrupted.  This function will also check and update the sequence numbers
     of the partitions in the volumes.                                                                                     */
  ForEachItem(Volumes, &Find_Pending_Changes, NULL, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

  assert(Error == DLIST_SUCCESS);

#else

  if ( Error != DLIST_SUCCESS)
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

#endif

#endif

  LOG_EVENT("Attempting to load the expansion DLLs for the various filesystems.")

  /* Now we must find out which volumes contain filesystems that are expandable. */
  ForEachItem(Volumes, &Find_Expandable_Volumes, NULL, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

  assert(Error == DLIST_SUCCESS);

#else

  if ( Error != DLIST_SUCCESS)
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Volumes")

    return;

  }

#endif

#endif

  /* All done. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Discover_Volumes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Commit_Volume_Changes( CARDINAL32 * Error_Code )
{

  Dynamic_Drive_Data    Drive_Packet;               /* Used to dynamically delete drives from the IFSM. */
  CARDINAL32            Drive_Letter_Mask;          /* Used when looking for deleted drive letters. */
  APIRET                Return_Code;                /* Used for the DosDevIOCtl call. */
  CARDINAL32            Parameter_Size;             /* Used for the DosDevIOCtl call. */
  CARDINAL32            Index;                      /* Used to walk the drive array. */
  CARDINAL32            Count;                      /* Used to count the number of drives that had changes made to them. */
  char                  Drive_Letter;               /* Used during the dynamic drives phase of the commit operation. */
  ExtendFS_Control_Data Control_Data;               /* Used when determining the number of volumes being extended. */
  PDDI_Rediscover_param Rediscovery_Parameters;     /* Used with the Rediscovery IOCTL call. */
  PDDI_Rediscover_data  Rediscovery_Data;           /* Used with the Rediscovery IOCTL call. */


  FUNCTION_ENTRY("Commit_Volume_Changes")

  /* Has the Volumes list been created? */
  if ( Volumes == NULL )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* We have an error!  This module has not been initialized yet! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Commit_Volume_Changes")

    return;

  }

  LOG_EVENT("Processing the Volumes list.")

  /* We must walk the Volumes list and, for each LVM volume which has been created or modified, we must have
     each feature on that volume commit its changes.                                                          */
  ForEachItem( Volumes, &Commit_All_Changes, NULL, TRUE, Error_Code );


  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Commit_Volume_Changes")

    return;

  }

  /* If we are not running on Aurora, then we must force a reboot. */
  if ( Merlin_Mode )
  {

    RebootRequired = TRUE;

    FUNCTION_EXIT("Commit_Volume_Changes")

    return;

  }

  LOG_EVENT("Beginning the PRM check.")

  /* We must check for PRMs where Fake Volumes are in use and the PRM has its ChangesMade flag set.  This indicates that
     a change was made to the PRM which did not affect the fake volumes, such as assigning serial numbers to partitions
     and the disk itself.  In these cases, it is necessary to add the fake volumes to the deleted drive letters list.  This
     is necessary so that, when a rediscover is done on the PRM, OS2DASD and OS2LVM do not get confused by seeing partitions
     and volumes which match ones they have in their tables ( the fakes ) but have different serial numbers.  By adding the
     drive letters for the fake volumes to Deleted_Drive_Letters, these "fake" volumes will be erased from OS2LVM's and OS2DASD's
     tables so that, when the rediscover happens, they don't get into problems with partitions and volumes matching entries they
     already have, except for the serial number.                                                                                   */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    if ( DriveArray[Index].Is_PRM && DriveArray[Index].ChangesMade && ( ! DriveArray[Index].Unusable ) && DriveArray[Index].Fake_Volumes_In_Use )
    {

      LOG_EVENT1("Adding fake volumes to the list of drives to delete.","Drive Number", Index + 1)

      /* Add all of the fake volumes to Deleted_Drive_Letters. */
      ForEachItem(DriveArray[Index].Partitions, &Add_Fake_Volumes_To_Deleted_Volumes_List, NULL, TRUE, Error_Code);

      if ( *Error_Code != DLIST_SUCCESS )
      {

        RebootRequired = TRUE;
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FUNCTION_EXIT("Commit_Volume_Changes")

        return;

      }

    }

  }

  /* If any of the original volumes having drive letter assignments were deleted or modified,
     then a reboot may be required.  Lets see if any drive letters were deleted or modified.   */
  if ( Deleted_Drive_Letters != 0 )
  {

    LOG_EVENT("Drives have been deleted.  Determining if a reboot is needed.")

    /* If we are at install time, then any volume deletions could cause the CD-ROM to move.  This
       is BIG trouble for the install program.  While we could do a lot of work to determine if
       the CD-ROM will move, deletion of a volume at install time is a rare case ( mostly our own
       test labs ) so we will just force a reboot anytime a volume is deleted as part of the install process.         */

    /* Min_Install_Size is only > 0 during installation, so we will use that to see if we are at install time or not. */
    if ( Min_Install_Size > 0 )
    {

      LOG_EVENT("Install time detected.  A reboot is required.")

      /* Since we are at install time, force a reboot. */
      RebootRequired = TRUE;

    }
    else
    {

      LOG_EVENT("Attempting to delete the drives from OS2LVM's tables. ")

      /* We must try to delete the drives. */
      for ( Drive_Letter = 'C'; Drive_Letter <= 'Z'; Drive_Letter++ )
      {

        /* Calculate the bit corresponding to the current drive letter being tested. */
        Drive_Letter_Mask = 0x1 << ( Drive_Letter - 'A' );

        /* Is the drive letter to be deleted? */
        if ( ( Deleted_Drive_Letters & Drive_Letter_Mask ) != 0 )
        {

          /* This is a drive to delete! */

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"Attempting to delete drive letter %c!", Drive_Letter );
            Write_Log_Buffer();

          }

          /* Set up the Drive_Packet for the IOCTL call. */
          Drive_Packet.drive_unit = Drive_Letter - 'A';
          Drive_Packet.cmd_info = 0;

          /* Get the size of the parameter packet for the DosDevIOCtl call. */
          Parameter_Size = sizeof(Dynamic_Drive_Data);

          /* Call the IOCTL. */
          Return_Code = DosDevIOCtl(-1L, IOCTL_DISK, DELETE_DRIVE_IOCTL, &Drive_Packet, sizeof(Dynamic_Drive_Data), &Parameter_Size, NULL, 0, NULL );

          if ( Return_Code != NO_ERROR )
          {

            LOG_EVENT1("We could not delete the drive letter, so a reboot is required.","Error code", Return_Code)

            RebootRequired = TRUE;

            break;

          }

        }

      } /* End of FOR loop. */

    }

  }


  LOG_EVENT("Deletion of drives is complete.")

  /* Now that we have eliminated any drive letters that we needed to, we must check any
     new drive letters coming on line to ensure that there are no conflicts with CD-ROMS
     and PRMs as these can cause trouble for OS2LVM and OS2DASD.                          */
  if ( ! RebootRequired )
  {

    LOG_EVENT("Checking for conflicts between new volumes and CD-ROMs, PRMs, network drives, etc. ")

    /* We will walk the list of items and check the drive letter preference of any volume whose current drive letter is 0.
       A volume will only have a current drive letter of 0 if it is new, has had its drive letter assignment changed, or
       has just been hidden.                                                                                                  */
    ForEachItem(Volumes, &Check_New_Drive_Letters, NULL, TRUE, Error_Code);

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Commit_Volume_Changes")

      return;

    }

  }


  if ( !RebootRequired )
  {

    LOG_EVENT("Looking for drives that have been modified.")

    /* Lets examine the DriveArray to see which drives have been modified.  We must perform a rediscovery on them so that
       any new or modified volumes can be discovered and prepared for use.                                                 */

    /* Prepare to count the number of drives that have been modified. */
    Count = 0;

    /* Count the number of drives that have been modified. */
    for ( Index = 0; Index < DriveCount; Index++ )
    {

      if ( DriveArray[Index].ChangesMade && ( ! DriveArray[Index].Unusable ) )
        Count++;

    }

    /* Were any drives modified? */
    if ( Count > 0)
    {

      /* Now that we know how many drives have changed, we can create the parameter packet required for the call to Rediscover. */
      Rediscovery_Parameters = (PDDI_Rediscover_param) malloc( sizeof(DDI_Rediscover_param) + ( Count * sizeof(BYTE) )  - ( 1 * sizeof(BYTE) ) );

      /* Did we get the memory? */
      if ( Rediscovery_Parameters == NULL )
      {

        /* Since we could not get the memory we need for the Rediscovery IOCTL call, a reboot is required. */
        RebootRequired = TRUE;

      }
      else
      {

        /* Since we got the memory, lets initialize our parameter packet. */
        Rediscovery_Parameters->DDI_TotalDrives = Count;

        LOG_EVENT("Building the rediscovery control packet.")

        /* Walk the drive array and place the OS/2 Drive Number of each modified drive into the Rediscovery_Parameters. */
        for ( Index = 0, Count = 0; Index < DriveCount; Index++ )
        {

          if ( DriveArray[Index].ChangesMade && ( ! DriveArray[Index].Unusable ) )
          {

            /* Calculate and save the OS/2 Drive Number for this drive. */
            Rediscovery_Parameters->DDI_aDriveNums[Count] = Index + 1;

            /* Set up to save the next modified drive. */
            Count++;

          }

        }

        /* Now we must find out how many volumes are being extended. */

        LOG_EVENT("Checking for volumes which have been extended.")

        /* Set up the control data required by the Extend_FS function. */
        Control_Data.Command = Count_Volumes;
        Control_Data.Volume_Count = 0;
        Control_Data.Volume_Expansion_Array = NULL;

        /* Walk the list of volumes allowing the Extend_FS function to examine each volume. */
        ForEachItem(Volumes, &Extend_FS, &Control_Data, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

        assert(*Error_Code == DLIST_SUCCESS);

#else

        if ( *Error_Code != DLIST_SUCCESS)
        {

          free( Rediscovery_Parameters );

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FUNCTION_EXIT("Commit_Volume_Changes")

          return;

        }

#endif

#endif

        LOG_EVENT("Building the rediscovery data packet.")

        /* Allocate the memory for the IOCTL data packet. */
        Rediscovery_Data = (PDDI_Rediscover_data) malloc( ( Control_Data.Volume_Count * sizeof(DDI_ExtendRecord) ) + sizeof(DDI_Rediscover_data) - sizeof(DDI_ExtendRecord) );

        /* Did we get the memory? */
        if ( Rediscovery_Data != NULL )
        {

          /* Now we must fill in the data packet. */
          Rediscovery_Data->NewIFSMUnits = 0;
          Rediscovery_Data->DDI_TotalExtends = Control_Data.Volume_Count;

          /* Now fill in the array of DDI_ExtendRecord in the data packet. */
          Control_Data.Volume_Expansion_Array = (DDI_ExtendRecord *) &(Rediscovery_Data->DDI_aExtendRecords);
          Control_Data.Command = Fill;
          Control_Data.Volume_Count = 0;

          /* Walk the list of volumes allowing the Extend_FS function to examine each volume. */
          ForEachItem(Volumes, &Extend_FS, &Control_Data, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

          assert(*Error_Code == DLIST_SUCCESS);
          assert( Control_Data.Volume_Count == Rediscovery_Data->DDI_TotalExtends );

#else

          if ( ( *Error_Code != DLIST_SUCCESS ) || ( Control_Data.Volume_Count != Rediscovery_Data->DDI_TotalExtends ) )
          {

            free( Rediscovery_Parameters );
            free( Rediscovery_Data );

            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

            FUNCTION_EXIT("Commit_Volume_Changes")

            return;

          }

#endif

#endif

          /* If any of the volumes to expand use filesystems which support Quiesce and Resume, then the Extend_FS function would have tried to
             invoke the Quiesce function of these filesystems.  If the Quiesce fails (perhaps the swapper is on that volume - can't quiesce the
             swap file! ), then we must reboot.                                                                                                  */
          if ( ! RebootRequired )
          {

            LOG_EVENT("Beginning the Rediscovery Process.")

            /* We are ready to start the Rediscovery process.  */
            if ( Rediscover(Rediscovery_Parameters, Rediscovery_Data) != NO_ERROR )
              RebootRequired = TRUE;

          }


          /* We don't need the parameter or data packets anymore, so free them. */
          free( Rediscovery_Parameters );
          free( Rediscovery_Data );

        }
        else
        {

          RebootRequired = TRUE;

          /* We don't need the parameter packet anymore, so free it. */
          free( Rediscovery_Parameters );

        }

      }

    }

  }


  if ( ! RebootRequired )
  {

    LOG_EVENT("Rediscovery complete.  Preparing to reconcile drive letters.")

    /* Clear out the Current Drive Letter and Initial Drive Letter fields.  We must re-establish the values of these fields as they
       may have changed due to the Rediscover operation.                                                                             */
    ForEachItem(Volumes,&Clear_Drive_Letter_Fields,NULL,TRUE,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

          assert(*Error_Code == DLIST_SUCCESS);

#else

          if ( *Error_Code != DLIST_SUCCESS )
          {

            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

            FUNCTION_EXIT("Commit_Volume_Changes")

            return;

          }

#endif

#endif

    LOG_EVENT("Beginning to reconcile drive letters.")

    /* Now we must establish the current drive letter for each Volume.  Furthermore, we must create "fake" volumes
       to represent Non-LVM controlled resources (such as CD-ROMs and Network Drives) which consume a drive letter.
       The Reconcile_Drive_Letters function does all of that for us.                                                 */

    Reserved_Drive_Letters = Reconcile_Drive_Letters( FALSE, Error_Code );

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {


      FUNCTION_EXIT("Commit_Volume_Changes")

      /* Something unexpected happened!  Abort. */
      return;

    }

    /* Now we must save the current drive letter of each volume as the original drive letter for each volume.  If we
       are in "Merlin" mode, then this function will set the current drive letter and the initial drive letter
       equal to the drive letter preference.                                                                             */
    ForEachItem(Volumes,&Set_Initial_Drive_Letters, NULL, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS)
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Commit_Volume_Changes")

      return;

    }

#endif

#endif

  }


  /* Complete the expansion of any volumes we started to expand. */
  if ( ! RebootRequired )
  {

    LOG_EVENT("Beginning the final stage of the volume expansion process for those volumes which have been expanded.")

    /* Set up the control data required by the Extend_FS function. */
    Control_Data.Command = Complete;
    Control_Data.Volume_Count = 0;
    Control_Data.Volume_Expansion_Array = NULL;

    /* Walk the list of volumes allowing the Extend_FS function to examine each volume. */
    ForEachItem(Volumes, &Extend_FS, &Control_Data, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS)
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Commit_Volume_Changes")

      return;

    }

#endif

#endif

  }

  LOG_EVENT1("Volume changes have been committed.","Reboot Required", RebootRequired)

  FUNCTION_EXIT("Commit_Volume_Changes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partitions_On_Volume                         */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the Volume specified by Handle.          */
/*                                                                   */
/*   Input:Volume_Data * Volume_Information : The Volume_Data for the*/
/*                                            Volume of interest.    */
/*         CARDINAL32 * Error_Code : The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  The    */
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
/*                   If Handle is invalid, a trap is likely.         */
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
Partition_Information_Array Get_Partitions_On_Volume( Volume_Data * Volume_Information, CARDINAL32 * Error_Code )
{

  Partition_Information_Array   ReturnValue;              /* Used to hold the value being returned to the caller of this function. */


  FUNCTION_ENTRY("Get_Partitions_On_Volume")

  /* Initialize ReturnValue assuming failure. */
  ReturnValue.Partition_Array = NULL;
  ReturnValue.Count = 0;

  /* Has the Volumes list been created? */
  if ( Volumes == NULL )
  {
    /* We have an error!  This module has not been initialized yet! */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    FUNCTION_EXIT("Get_Partitions_On_Volume")

    return ReturnValue;

  }

  /* If the volume represents a non_LVM device, then there are no partitions associated with it. */
  if ( ( Volume_Information->Device_Type != LVM_HARD_DRIVE ) && ( Volume_Information->Device_Type != LVM_PRM ) )
  {


    FUNCTION_EXIT("Get_Partitions_On_Volume")

    /* Since there are no partitions, return an empty array and a 0 count. */
    return ReturnValue;

  }

  /* Allocate memory for the array being returned to the caller. */
//  printf("Allocate %i\n",Volume_Information->Partition_Count * sizeof(Partition_Information_Record));

  ReturnValue.Partition_Array = (Partition_Information_Record *) malloc( Volume_Information->Partition_Count * sizeof(Partition_Information_Record) );

  if ( ReturnValue.Partition_Array == NULL )
  {

    /* We are out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Get_Partitions_On_Volume")

    return ReturnValue;

  }

  /* ReturnValue.Count is currently 0.  We will use it to index the Partition_Array as we traverse the partition list again. */

  /* Now traverse the list and transfer the information from the list to the array. */
  Transfer_Partition_Data(Volume_Information->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &ReturnValue, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Was there an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* This should not be possible here.  We must have an internal error. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    /* Clean up ReturnValue. */
    ReturnValue.Count = 0;
    free(ReturnValue.Partition_Array);
    ReturnValue.Partition_Array = NULL;

    FUNCTION_EXIT("Get_Partitions_On_Volume")

    return ReturnValue;

  }

#endif

#endif

  /* Indicate success and return what we found. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Get_Partitions_On_Volume")

  return ReturnValue;

}


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
Volume_Control_Array Get_Volume_Control_Data( CARDINAL32 * Error_Code )
{

  Volume_Control_Array    ReturnValue;
  Volume_Data *           Current_Volume;
  CARDINAL32              Volume_Count;


  API_ENTRY("Get_Volume_Control_Data")

  /* Initialize ReturnValue assuming failure. */
  ReturnValue.Count = 0;
  ReturnValue.Volume_Control_Data = NULL;

  /* Has the Volumes list been created? */
  if ( Volumes == NULL )
  {
    /* We have an error!  This module has not been initialized yet! */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

  /* How many volumes are there? */
  ReturnValue.Count = GetListSize( Volumes, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    ReturnValue.Count = 0;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

#endif

#endif

  /* If there are no volumes, then we are done! */
  if ( ReturnValue.Count == 0 )
  {

    /* Indicate success. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

  /* Allocate memory for the array of Volume_Control_Record's being returned. */
  ReturnValue.Volume_Control_Data = (Volume_Control_Record *) malloc(ReturnValue.Count * sizeof(Volume_Control_Record) );

  /* Did we get the memory? */
  if ( ReturnValue.Volume_Control_Data == NULL )
  {

    /* We are out of memory!  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    /* Tell the user that the Volume_Control_Data array is empty. */
    ReturnValue.Count = 0;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

  /* Sort the list of Volumes based upon their current drive letters. */
  SortList(Volumes, &Sort_By_Current_Drive_Letter, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    ReturnValue.Count = 0;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

#endif

#endif

  /* Now walk the list of Volumes and copy the appropriate information to the ReturnValue.Volume_Control_Data array. */
  GoToStartOfList(Volumes, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    ReturnValue.Count = 0;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

#endif

#endif


  /* Get the first item in the Volumes list. */
  Current_Volume = GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, NULL, FALSE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    ReturnValue.Count = 0;

    API_EXIT("Get_Volume_Control_Data")

    return ReturnValue;

  }

#endif

#endif


  /* Volume_Count will be used to index the Volume_Control_Data array being returned.   Initialize it to 0. */
  Volume_Count = 0;

  /* Now copy the data for each volume in the Volumes list. */
  do
  {

    /* Copy the items we are interested in. */
    ReturnValue.Volume_Control_Data[Volume_Count].Volume_Handle = Current_Volume->External_Handle;
    ReturnValue.Volume_Control_Data[Volume_Count].Compatibility_Volume = Current_Volume->Compatibility_Volume;
    ReturnValue.Volume_Control_Data[Volume_Count].Volume_Serial_Number = Current_Volume->Volume_Serial_Number;
    ReturnValue.Volume_Control_Data[Volume_Count].Device_Type = Current_Volume->Device_Type;

    /* Now get the next volume in the list. */
    Current_Volume = GetNextObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Error_Code );

    /* Increment our index into the Volume_Control_Data array. */
    Volume_Count++;

  } while ( *Error_Code == DLIST_SUCCESS );


#ifdef DEBUG

#ifdef PARANOID

  assert( ( *Error_Code == DLIST_END_OF_LIST ) && ( Volume_Count == ReturnValue.Count ) );

#else

  /* Did we leave the do-while loop due to an error? */
  if ( (*Error_Code != DLIST_SUCCESS) && ( *Error_Code != DLIST_END_OF_LIST ) )
  {

     *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

     API_EXIT("Get_Volume_Control_Data")

     return ReturnValue;

  }

#endif

#endif

  /* All done!  Indicate success and return what we found. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Get_Volume_Control_Data")

  return ReturnValue;

}


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
Volume_Information_Record Get_Volume_Information( ADDRESS Volume_Handle, CARDINAL32 * Error_Code )
{

  ADDRESS                     Object;             /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                         ObjectTag;          /* Used when translating the Volume_Handle into a Volume_Data structure. */

  Volume_Information_Record   ReturnValue;        /* Used to hold the value we are going to return while we construct it. */
  Volume_Data  *              Volume_Information; /* Used to access Object as a Volume_Information_Record. */


  API_ENTRY("Get_Volume_Information")

  /* Initialize ReturnValue assuming a failure. */
  memset( &ReturnValue, 0, sizeof(Volume_Information_Record) );

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Get_Volume_Information")

    return ReturnValue;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Volume_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Get_Volume_Information")

    return ReturnValue;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != VOLUME_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Get_Volume_Information")

    return ReturnValue;

  }

  /* Establish access to Object as a Volume_Data structure. */
  Volume_Information = (Volume_Data *) Object;

  /* Now lets initialize ReturnValue using the contents of Object (which we know is a Volume_Data structure). */
  ReturnValue.Volume_Size = Volume_Information->Volume_Size;
  ReturnValue.Compatibility_Volume = Volume_Information->Compatibility_Volume;
  ReturnValue.Partition_Count = Volume_Information->Partition_Count;
  ReturnValue.Drive_Letter_Conflict = Volume_Information->Drive_Letter_Conflict;
  ReturnValue.Drive_Letter_Preference = Volume_Information->Drive_Letter_Preference;
  ReturnValue.Current_Drive_Letter = Volume_Information->Current_Drive_Letter;
  ReturnValue.Initial_Drive_Letter = Volume_Information->Initial_Drive_Letter;
  strncpy(ReturnValue.Volume_Name, Volume_Information->Volume_Name,VOLUME_NAME_SIZE);
  strncpy(ReturnValue.File_System_Name, Volume_Information->File_System_Name,FILESYSTEM_NAME_SIZE);
  ReturnValue.Status = 0;
  ReturnValue.Bootable = FALSE;
  ReturnValue.New_Volume = Volume_Information->New_Volume;

  /* If this volume represents a non-lvm device, then it is, by definition, not Bootable.  If it represents an LVM controlled
     device, then it may be bootable and we will have to check.                                                                */
  if ( ( Volume_Information->Device_Type == LVM_HARD_DRIVE ) || ( Volume_Information->Device_Type == LVM_PRM ) )
  {

    /* Is this volume marked installable and we are at install time? */
    if ( ( Install_Volume_Handle == Volume_Information->Volume_Handle ) && ( Min_Install_Size > 0 ) )
      ReturnValue.Status = 3;
    else
      if ( Is_Volume_Startable(Volume_Information,FALSE, Error_Code) )      /* Is this volume startable? */
          ReturnValue.Status = 2;
      else
        if ( Is_Volume_Bootable(Volume_Information, FALSE, Error_Code) )    /* Is this volume bootable? */
          ReturnValue.Status = 1;

    /* Is the volume bootable? */
    ReturnValue.Bootable = ( ReturnValue.Status != 0 );

  }

  /* All done. */

  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Get_Volume_Information")

  return ReturnValue;

}


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
/*   Notes:  This function provides limited compatibility for        */
/*           programs written to use the LVM Version 1 interface.    */
/*           Specifically, this function will only allow the         */
/*           creation of compatibility volumes.  Any attempt to      */
/*           create an LVM volume will result in an error code being */
/*           returned.                                               */
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
                          )
{

  API_ENTRY("Create_Volume")

  if ( Create_LVM_Volume )
  {

    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Create_Volume")

    return;

  }

  Create_Volume2(Name, FALSE, Bootable, Drive_Letter_Preference, 0, NULL, Partition_Count, Partition_Handles, Error_Code);

  API_EXIT("Create_Volume")

  return;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Create_Volume2( char                               Name[VOLUME_NAME_SIZE],
                             BOOLEAN                            Create_LVM_Volume,
                             BOOLEAN                            Bootable,
                             char                               Drive_Letter_Preference,
                             CARDINAL32                         Feature_Count,
                             LVM_Feature_Specification_Record   FeaturesToUse[],
                             CARDINAL32                         Partition_Count,
                             ADDRESS                            Partition_Handles[],
                             CARDINAL32 *                       Error_Code
                           )
{

  CARDINAL32                         Drive_Letter_Mask = 0x1;  /* Used to check the availability of Drive_Letter_Preference. */
  CARDINAL32                         Name_Size;                /* Used to hold the adjusted size of Name. */
  ADDRESS                            Object;                   /* Used when translating a Handle into a Partition_Data structure. */
  TAG                                ObjectTag;                /* Used when translating a Handle into a Partition_Data structure. */
  Partition_Data *                   PartitionRecord;          /* Used when accessing the data for a partition that the user has specified. */
  Duplicate_Check_Parameter_Record   Name_Check;               /* Used when checking for a duplicate volume name. */
  Volume_Data    *                   New_Volume = NULL;        /* Used to create the new volume. */
  CARDINAL32                         CurrentPartition;         /* Used to traverse the Partition_Handles array. */
  CARDINAL32                         Ignore_Error;             /* Used on function calls where we don't care about an error condition. */
  DLIST                              Volume_Partitions = NULL; /* Used when creating LVM volumes. */
  DLIST                              Features_List = NULL;     /* Used when creating an LVM volume. */
  Apply_Features_Parameter_Record    Apply_Features_Data;      /* Used when applying features during the creation of an LVM Volume. */
  Feature_Application_Data           Feature_Class_Data;       /* Used when applying features during the creation of an LVM Volume. */
  BOOLEAN                            Aggregator_Found;         /* Used to determine who should create the fake EBR for an LVM Volume. */
  CARDINAL32                         Count;                    /* Used when setting the sequence numbers of partitions in a volume. */


  API_ENTRY("Create_Volume2")

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Create_Volume2")

    return;

  }

  /* Check the parameters. */

  /* If Partition_Count is 0, or if Partition_Handles is NULL, then we have nothing to create a volume with! */
  if ( ( Partition_Count == 0 ) || ( Partition_Handles == NULL ) )
  {

    *Error_Code = LVM_ENGINE_INVALID_PARAMETER;

    API_EXIT("Create_Volume2")

    return;

  }

  /* If the requested drive letter is not available, we can not proceed.  */

  /* Have we been given a drive letter, a NULL, or a *.  A NULL is used to create a hidden volume. */
  if ( ( Drive_Letter_Preference != 0 ) && ( Drive_Letter_Preference != '*' ) )
  {

    /* Make sure that Drive_Letter_Preference has been capitalized. */
    Drive_Letter_Preference = (char ) toupper( Drive_Letter_Preference );

    /* Is the drive letter preference legal? */
    if ( ( Drive_Letter_Preference < 'C' ) || ( Drive_Letter_Preference > 'Z' ) )
    {

      /* The drive letter is out of range. */
      *Error_Code = LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Calculate the bit corresponding to this drive letter in the Available_Drive_Letter bitmap. */
    Drive_Letter_Mask = Drive_Letter_Mask << ( Drive_Letter_Preference - 'A' );

    /* Is the drive letter available? */
    if ( (Drive_Letter_Mask & Available_Drive_Letters ) == 0 )
    {

      /* The requested drive letter is not available! */
      *Error_Code = LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE;

      API_EXIT("Create_Volume2")

      return;

    }

  }

  /* If Bootable is TRUE, then Partition_Count must be 1.  This is a consistency check! */
  if ( Bootable && ( Partition_Count != 1 ) )
  {

    /* Indicate the error! */
    *Error_Code = LVM_ENGINE_TOO_MANY_PARTITIONS_SPECIFIED;

    API_EXIT("Create_Volume2")

    return;

  }

  /* If Bootable is TRUE, then Create_LVM_Volume must be FALSE as you can not boot off of an LVM volume! */
  if ( Bootable && Create_LVM_Volume )
  {

    /* Indicate the error! */
    *Error_Code = LVM_ENGINE_LVM_PARTITIONS_NOT_BOOTABLE;

    API_EXIT("Create_Volume2")

    return;

  }

  /* If Partition_Count > 1, then Create_LVM_Volume must be TRUE as a compatibility volume can only have 1 partition. */
  if ( ( ! Create_LVM_Volume) && ( Partition_Count > 1 ) )
  {

    /* Indicate the error. */
    *Error_Code = LVM_ENGINE_TOO_MANY_PARTITIONS_SPECIFIED;

    API_EXIT("Create_Volume2")

    return;

  }

  /* D201136 - The LVM VIO interface can't handle having the engine remove trailing spaces from the
               strings it passes in, so the engine was modified to copy the Name string passed in and
               operate on the copy.

     BEGIN                                                                                          BMR */

  /* Get the size of the Name for the Volume. */
  Name_Size = strlen(Name);

  /* Is the name too small? */
  if ( Name_Size == 0 )
  {

    /* Zero length names are not acceptable. */
    *Error_Code = LVM_ENGINE_BAD_NAME;

    API_EXIT("Create_Volume2")

    return;

  }

  /* We need a copy of Name so that we can adjust it as necessary. */
  Name_Check.New_Name = (char *) malloc( strlen(Name) + 1 );

  /* Did we get the memory? */
  if ( Name_Check.New_Name == NULL )
  {

    /* We must be out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    API_EXIT("Create_Volume2")

    return;

  }

  /* Copy Name to its new location. */
  strcpy(Name_Check.New_Name, Name);

  /* Now set Name to point to its copy. */
  Name = Name_Check.New_Name;

  /* D201136 - The LVM VIO interface can't handle having the engine remove trailing spaces from the
               strings it passes in, so the engine was modified to copy the Name string passed in and
               operate on the copy.

     END                                                                                            BMR */


  /* What is the size of the name?  We must eliminate leading and trailing spaces before we can determine the size of name. */
  Name = Adjust_Name( Name );

  /* Now get the size of Name. */
  Name_Size = strlen(Name);

  /* Is the name too big? */
  if ( Name_Size > VOLUME_NAME_SIZE )
  {

    /* The name is too big.  Indicate the error. */
    *Error_Code = LVM_ENGINE_NAME_TOO_BIG;

    API_EXIT("Create_Volume2")

    return;

  }

  /* Is the name too small? */
  if ( Name_Size == 0 )
  {

    /* Zero length names are not acceptable. */
    *Error_Code = LVM_ENGINE_BAD_NAME;

    API_EXIT("Create_Volume2")

    return;

  }

  /* Is the name already in use?  Lets find out. */

  /* Set up for the duplicate name check. */
  Name_Check.New_Name_Length = Name_Size;
  Name_Check.Handle = NULL;
  Name_Check.Duplicate_Name_Found = FALSE;
  Name_Check.Check_Name = TRUE;
  Name_Check.New_Name = Name;
  Name_Check.Max_Name_Length = VOLUME_NAME_SIZE;

  /* Perform the duplicate name check. */
  ForEachItem( Volumes, &Duplicate_Check, &Name_Check, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Create_Volume2")

    return;

  }

#endif

#endif

  /* Was a duplicate name found? */
  if ( Name_Check.Duplicate_Name_Found )
  {

    /* Indicate the error! */
    *Error_Code = LVM_ENGINE_DUPLICATE_NAME;

    API_EXIT("Create_Volume2")

    return;

  }

  /* We are ready to create a volume! */

  /* Are we creating a compatibility volume?  This is the easiest kind. */
  if ( ! Create_LVM_Volume )
  {

    /* We must translate the handle given to us to see if it really is a partition.  If so, we must check to see if this
       partition already belongs to a Volume, if the partition is bootable ( if the Bootable parameter is TRUE ), and if
       the partition is eligible to be part of the volume being created.                                                   */

    /* Translate the handle. */
    Translate_Handle( Partition_Handles[0], &Object, &ObjectTag, Error_Code );

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      API_EXIT("Create_Volume2")

      return;

    }

    /* From the ObjectTag we can tell what Object points to. */

    /* Is the object what we want? */
    if ( ObjectTag != PARTITION_DATA_TAG )
    {

      /* We have a bad handle.  */
      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Establish access to the Partition_Data structure we want to manipulate. */
    PartitionRecord = ( Partition_Data * ) Object;

    /* Is this partition already part of a volume? */
    if ( PartitionRecord->Volume_Handle != NULL )
    {

      /* This partition is already part of a volume. */
      *Error_Code = LVM_ENGINE_PARTITION_ALREADY_IN_USE;

      API_EXIT("Create_Volume2")

      return;

    }

    /* If the user wants a bootable volume, is the partition chosen bootable? */
    if ( Bootable )
    {

      /* To be bootable, the partition must be below the 1024 cylinder limit, if that applies.  Also, if Boot Manager is not
         installed, then the partition must be a primary partition on the first hard drive.                                    */

      /* Does the 1024 cylinder limit apply? */
      if ( DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies )
      {

        /* Is the entire partition below the Boot_Limit? */
        if ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) > DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
        {

          /* This partition is not bootable as it crosses the 1024 cylinder limit. */
          *Error_Code = LVM_ENGINE_1024_CYLINDER_LIMIT;

          API_EXIT("Create_Volume2")

          return;

        }

      }

      /* Is Boot Manager installed and active? */
      if ( ( Boot_Manager_Handle == NULL ) || ( ! Boot_Manager_Active ) )
      {

/* Boot Manager is not installed, or Boot Manager is not active.  This means that the partition must be on the first drive and must be a primary. */
//EK           
//todo
//debug
//      if ( ( PartitionRecord->Drive_Index != 0) || ( ! PartitionRecord->Primary_Partition ) )

        if (/* ( PartitionRecord->Drive_Index != 0) ||*/ ( ! PartitionRecord->Primary_Partition ) )
        {

          /* Indicate the error! */
          *Error_Code = LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE;

          API_EXIT("Create_Volume2")

          return;

        }

      }

    }

    /* If there were "fake" volumes on the drive, convert them to real volumes. */
    if ( DriveArray[PartitionRecord->Drive_Index].Fake_Volumes_In_Use )
    {
      Convert_Fake_Volumes_On_PRM_To_Real_Volumes( PartitionRecord->Drive_Index, Error_Code);

      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        API_EXIT("Create_Volume2")

        return;

      }

    }

    /* Lets fill in the DLA_Table_Entry for the partition.  The Create_Compatibility_Volume function will use the information
       in the DLA_Table_Entry of PartitionRecord to create the Volume_Data structure for the new volume.                       */
    PartitionRecord->DLA_Table_Entry.Volume_Serial_Number = Create_Serial_Number();

    /* Does the drive holding the partition have a S/N yet? */
    if ( DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number == 0 )
    {

      /* Give the drive a serial number. */
      DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number = Create_Serial_Number();

      /* Save the Boot Drive's S/N so that we know the partitions on this drive are associated with current Boot Drive. */
      DriveArray[PartitionRecord->Drive_Index].Boot_Drive_Serial_Number = Boot_Drive_Serial_Number;

    }

    /* Does the partition itself have a serial number yet? */
    if ( PartitionRecord->DLA_Table_Entry.Partition_Serial_Number == 0)
    {

      /* Give the partition a serial number. */
      PartitionRecord->DLA_Table_Entry.Partition_Serial_Number = Create_Serial_Number();

    }

    /* Finish filling in the DLA_Table_Entry for PartitionRecord. */
    PartitionRecord->DLA_Table_Entry.Drive_Letter = Drive_Letter_Preference;
    strncpy(PartitionRecord->DLA_Table_Entry.Volume_Name, Name, VOLUME_NAME_SIZE);

    /* Mark the drive containing the partition as having had changes made. */
    DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;


    /* Lets create a Volume_Data object for this new compatibility volume. */
    New_Volume = Create_Compatibility_Volume(PartitionRecord, Error_Code);

    /* Is this partition on a PRM? */
    if ( DriveArray[PartitionRecord->Drive_Index].Is_PRM )
      New_Volume->Device_Type = LVM_PRM;
    else
      New_Volume->Device_Type = LVM_HARD_DRIVE;

    /* Indicate that we have created a new volume. */
    New_Volume->New_Volume = TRUE;
    New_Volume->ChangesMade = TRUE;

    /* If the volume is supposed to be bootable, add it to the Boot Manager menu if Boot Manager is installed. */
    if ( Bootable && ( Boot_Manager_Handle != NULL ) )
    {

      /* Update the DLA Table entry for the partition belonging to this volume. */
      PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = TRUE;

      /* Update the volume's information so that it agrees with that for the partition. */
      New_Volume->On_Boot_Manager_Menu = TRUE;

    }
    else
    {

      /* Update the DLA Table entry for the partition belonging to this volume. */
      PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;

      /* Update the volume's information so that it agrees with that for the partition. */
      New_Volume->On_Boot_Manager_Menu = FALSE;

    }

  }
  else
  {

    /* We are to create an LVM Volume. */

    /* Create the FeaturesToUse list. */
    Features_List = CreateList();
    if ( Features_List == NULL )
    {

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      API_EXIT("Create_Volume2")

      return;

    }

    /* The first two features in the FeaturesToUse list are always PassThru followed by BBR.  */

    /* Add the PassThru feature as the first entry in the FeaturesToUse list. */
    Feature_Class_Data.Function_Table = PassThru_Function_Table;
    Feature_Class_Data.Actual_Class = Partition_Class;
    Feature_Class_Data.Feature_Sequence_Number = 0;
    Feature_Class_Data.Top_Of_Class = FALSE;
    Feature_Class_Data.Init_Data = NULL;
    InsertItem(Features_List, sizeof(Feature_Application_Data), &Feature_Class_Data, FEATURE_APPLICATION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      /* Set the return code. */
      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      else
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Add the BBR feature. */
    Feature_Class_Data.Function_Table = BBR_Function_Table;
    Feature_Class_Data.Actual_Class = Partition_Class;
    Feature_Class_Data.Feature_Sequence_Number = 1;
    Feature_Class_Data.Top_Of_Class = FALSE;
    Feature_Class_Data.Init_Data = FALSE;
    InsertItem(Features_List, sizeof(Feature_Application_Data), &Feature_Class_Data, FEATURE_APPLICATION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      /* Set the return code. */
      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      else
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Are the features specified by the user valid? */
    if ( Feature_Count > 0 )
    {

      /* Now we must check the features specified by the user.  They must be valid, in an acceptable order, and have no
         class conflicts.                                                                                                */
      if ( Feature_List_Is_Bogus(Feature_Count, FeaturesToUse, Features_List, &Aggregator_Found, Error_Code ) )
      {

        /* Free the FeaturesToUse list. */
        DestroyList(&Features_List,FALSE, &Ignore_Error);

        API_EXIT("Create_Volume2")

        return;

      }

    }

    /* If no aggregator was found, then only one partition may be specified! */
    if ( ( ! Aggregator_Found ) &&
         ( Partition_Count > 1 )
       )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      *Error_Code = LVM_ENGINE_TOO_MANY_PARTITIONS_SPECIFIED;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Now that the features have been checked, we must check the partitions specified. */

    /* Are the partitions specified valid? */
    if ( ! Partition_List_Is_Valid( Partition_Count, Partition_Handles, FALSE, Error_Code ) )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      API_EXIT("Create_Volume2")

      /* If there was an error, or if the Partition list was not valid, *Error_Code will already be set.  All we have to do is return. */
      return;

    }

    /* Allocate a list to hold the partitions that are to become part of the volume. */
    Volume_Partitions = CreateList();
    if ( Volume_Partitions == NULL )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      /* Set the return code. */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Lets allocate all of the LVM Signature Sectors.  There is one per partition. */
    if ( ! Allocate_Signature_Sectors( Partition_Count, Partition_Handles, Aggregator_Found, Error_Code ) )
    {

      /* We must free any memory we have already allocated! */
      Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
      DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      API_EXIT("Create_Volume2")

      return;

    }

    /* Since we have enough memory so far, and since the partition array is valid, lets create a new LVM Volume. */
    New_Volume = Create_Default_LVM_Volume( Error_Code );

    /* Was there a problem allocating a new LVM Volume? */
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      /* We must free any memory we have already allocated! */
      Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
      DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      API_EXIT("Create_Volume2")

      return;

    }

    /* We now have a "generic" LVM Volume.  Lets begin to customize it. */
    strncpy(New_Volume->Volume_Name,Name,VOLUME_NAME_SIZE);
    New_Volume->Partition_Count = Partition_Count;
    New_Volume->Drive_Letter_Preference = Drive_Letter_Preference;

    /* Now we must put the partitions into the Partitions List for the Volume. */
    for ( CurrentPartition = 0; CurrentPartition < Partition_Count; CurrentPartition++ )
    {

      /* We must translate the handle.  */

      /* Translate the handle. */
      Translate_Handle( Partition_Handles[CurrentPartition], &Object, &ObjectTag, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == HANDLE_MANAGER_NO_ERROR );

#else

      /* Was the handle valid? */
      if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
      {

        /* Since we have translated this handle once already, there must be an internal error! */
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        API_EXIT("Create_Volume2")

        return;

      }

#endif

#endif

      /* From the ObjectTag we can tell what Object points to. */

#ifdef DEBUG

#ifdef PARANOID

      assert( ObjectTag == PARTITION_DATA_TAG );

#else

      /* Was the handle valid? */
      if ( ObjectTag != PARTITION_DATA_TAG )
      {

        /* Since we have translated this handle once already, there must be an internal error! */
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        API_EXIT("Create_Volume2")

        return;

      }

#endif

#endif

      /* Establish access to the Partition_Data structure we want to manipulate. */
      PartitionRecord = ( Partition_Data * ) Object;

      /* Ensure that this partition is not on the Boot Manager Menu. */
      PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;

      /* Update a few fields in the LVM Signature Sector for this PartitionRecord. */
      PartitionRecord->Signature_Sector->Drive_Letter = New_Volume->Drive_Letter_Preference;
      PartitionRecord->Signature_Sector->Volume_Serial_Number = New_Volume->Volume_Serial_Number;
      strncpy(PartitionRecord->Signature_Sector->Volume_Name, New_Volume->Volume_Name, VOLUME_NAME_SIZE);

      /* Now add the partition to the Partitions List for this Volume. */
      InsertObject(Volume_Partitions, sizeof(Partition_Data), PartitionRecord, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

      /* Did we succeed? */
      if ( *Error_Code != DLIST_SUCCESS )
      {

        /* Are we out of memory? */
        if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        {

          /* Clean up what we can. */
          Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
          DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);
          DestroyList(&Features_List,FALSE, &Ignore_Error);
          DeleteItem(Volumes, FALSE, New_Volume->Volume_Handle, &Ignore_Error);
          Destroy_Handle(New_Volume->External_Handle, &Ignore_Error);
          free(New_Volume);

          /* Set the error code. */
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          API_EXIT("Create_Volume2")

          return;

        }
        else
        {

          /* This is some kind of an internal error! */

#ifdef DEBUG

#ifdef PARANOID

          assert(0);

#endif

#endif

          /* Set the error code. */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          API_EXIT("Create_Volume2")

          return;

        }

      }

    }

    /* Now that all of the partitions are successfully in the Volume_Partitions List, and each Partition has
       an LVM Signature Sector, lets initialize each of the features that will be active on these partitions.         */

    /* Set up to process each feature on all of the partitions in the volume. */
    Apply_Features_Data.Partition_List = Volume_Partitions;
    Apply_Features_Data.Error_Code = Error_Code;
    Apply_Features_Data.VolumeRecord = New_Volume;
    Apply_Features_Data.Feature_To_Stop_At = 0;
    ForEachItem(Features_List, &Apply_Features, &Apply_Features_Data, TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* Clean up what we can. */
      ForEachItem(Volume_Partitions, &Remove_All_Partition_Features , NULL, TRUE, &Ignore_Error);
      Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
      DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);
      DestroyList(&Features_List,FALSE, &Ignore_Error);
      DeleteItem(Volumes, FALSE, New_Volume->Volume_Handle, &Ignore_Error);
      Destroy_Handle(New_Volume->External_Handle, &Ignore_Error);
      free(New_Volume);

      /* Set the error code. */
      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      else
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return ;

    }

    /* There should only be a single item left in the list.  That item should be an aggregate. */
    if ( GetListSize(Volume_Partitions, Error_Code) != 1)
    {

      /* Something went wrong! */

      /* Clean up what we can. */
      ForEachItem(Volume_Partitions, &Remove_All_Partition_Features , NULL, TRUE, &Ignore_Error);
      Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
      DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);
      DestroyList(&Features_List,FALSE, &Ignore_Error);
      DeleteItem(Volumes, FALSE,  New_Volume->Volume_Handle, &Ignore_Error);
      Destroy_Handle(New_Volume->External_Handle, &Ignore_Error);
      free(New_Volume);

      /* Set the error return code. */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return ;

    }

    /* Extract the first partition in the volume. */
    PartitionRecord = GetObject(Volume_Partitions, sizeof(Partition_Data), PARTITION_DATA_TAG, NULL, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      {

        /* Clean up what we can. */
        ForEachItem(Volume_Partitions, &Remove_All_Partition_Features , NULL, TRUE, &Ignore_Error);
        Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
        DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);
        DestroyList(&Features_List,FALSE, &Ignore_Error);
        DeleteItem(Volumes, FALSE, New_Volume->Volume_Handle, &Ignore_Error);
        Destroy_Handle(New_Volume->External_Handle, &Ignore_Error);
        free(New_Volume);

      }
      else
      {

#ifdef DEBUG

#ifdef PARANOID

        assert(0);

#endif

#endif

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      }

      API_EXIT("Create_Volume2")

      return;

    }

    /* Save the PartitionRecord in the Volume's data. */
    New_Volume->Partition = PartitionRecord;

    /* We don't need the Volume_Partitions list anymore, so get rid of it. */
    DestroyList(&Volume_Partitions,FALSE, &Ignore_Error);

    /* Now that all memory has been successfully allocated, we can go ahead and modify the Partition_Data records for the
       partitions in the Partitions List of the new Volume.  We have avoided that so far because we did not want to do
       anything that could not be reversed if we ran out of memory and had to abort the operation.                           */
    Complete_Partition_Initialization(New_Volume->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, New_Volume, Error_Code);

    /* If we could not initialize one of the partitions in the Partitions List for this Volume, we must abort. */
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      {

        /* Clean up what we can. */
        Remove_All_Partition_Features(New_Volume->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, NULL, &Ignore_Error);
        DestroyList(&Features_List,FALSE, &Ignore_Error);
        Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
        DeleteItem(Volumes, FALSE, New_Volume->Volume_Handle, &Ignore_Error);
        Destroy_Handle(New_Volume->External_Handle, &Ignore_Error);
        free(New_Volume);

      }
      else
      {

#ifdef DEBUG

#ifdef PARANOID

        assert(0);

#endif

#endif

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      }

      API_EXIT("Create_Volume2")

      return;

    }

    /* Now that all of the partitions in the partition list have been initialized, along with their corresponding Signature Sectors
       and Feature data, we can save the size of the volume.                                                                         */
    New_Volume->Volume_Size = New_Volume->Partition->Usable_Size;

    /* We need to clear out the boot sector of each partition and set its Format Indicator. */
    Clear_Boot_Sectors(New_Volume->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, NULL, Error_Code);

    /* This should never fail!  If it does, then we have some internal error. */
    if ( *Error_Code != DLIST_SUCCESS)
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Set the filesystem name for the volume to "unformatted." */
    strncpy(New_Volume->File_System_Name, "unformatted", FILESYSTEM_NAME_SIZE);

    /* We don't need the FeaturesToUse list anymore, so free it. */
    DestroyList(&Features_List,FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* This should never happen!  Abort! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return;

    }

    /* Now we can set the sequence numbers of each partition in the LVM Volume. */
    Count = 1;
    Set_Partition_Sequence_Numbers(New_Volume->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &Count, Error_Code);

    /* Since this volume is not formatted yet, it can be expanded.  Set the appropriate flag in the volume record to indicate this. */
    New_Volume->Filesystem_Is_Expandable = TRUE;

    /* During the creation of this volume, aggregates may have been created.  Save the Next_Aggregate_Number in the LVM Signature Sector
       of the topmost partition/aggregate associated with this volume.                                                                    */
    PartitionRecord->Signature_Sector->Next_Aggregate_Number = New_Volume->Next_Aggregate_Number;

    /* We are all done!  The LVM volume has been created. */

  }

  /* We must update the parent pointers in the partitions belonging to the new volume.  This should never fail! */
  Set_Volume_Parent_Child_Pointers(New_Volume, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Create_Volume2")

    return;

  }

  /* Remove the drive letter assigned to this volume from the list of available drive letters, if the drive letter assigned is not NULL. */
  if ( ( Drive_Letter_Preference != 0 ) && ( Drive_Letter_Preference != '*' ) )
  {
    Available_Drive_Letters = (~Drive_Letter_Mask) & Available_Drive_Letters;

    /* It is possible that the drive letter assigned to this volume conflicts with the current drive letter assigned to a volume whose
       drive letter preference is a '*'.  Another possibility is that it conflicts with the current drive letter assigned to a volume
       whose drive letter preference conflicted with that of another volume.  In these cases, we must find the volume with the current
       drive letter which conflicts with the drive letter preference for this volume and zero out the current drive letter field.          */
    ForEachItem(Volumes, &Find_Current_Drive_Letter_Conflicts, &Drive_Letter_Preference, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Create_Volume2")

      return;

    }

#endif

#endif

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Create_Volume2")

  return;

}


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
void Delete_Volume( ADDRESS Volume_Handle, CARDINAL32 * Error_Code )
{

  Volume_Data *                VolumeRecord;                    /* Used to point to the Volume_Data structure we are manipulating. */
  Partition_Data *             PartitionRecord;                 /* Used to delete partitions associated with the volume being deleted. */
  ADDRESS                      Object;                          /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                          ObjectTag;                       /* Used when translating the Volume_Handle into a Volume_Data structure. */
  CARDINAL32                   Drive_Letter_Mask = 0x1;         /* Used when releasing the drive letter assigned to the volume being deleted. */
  Drive_Letter_Count_Record    Drive_Letter_Count;              /* Used to count the number of volumes with the same drive letter preference. */
  Plugin_Function_Table_V1 *   Old_Function_Table;              /* Used to access the function table associated with each partition/aggregate in the volume. */


  API_ENTRY("Delete_Volume")

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Delete_Volume")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Volume_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Delete_Volume")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != VOLUME_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Delete_Volume")

    return;

  }

  /* Establish access to the Volume_Data structure we want to manipulate. */
  VolumeRecord = ( Volume_Data * ) Object;

  /* If the volume can not be changed, abort unless it is a PRM. */
  if ( ( ! VolumeRecord->Can_Be_Altered ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Delete_Volume")

    return;

  }

  /* If the device is not a hard drive or PRM, abort! */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Delete_Volume")

    return;

  }

  /* Remove volume from volumes list. */
  DeleteItem(Volumes,FALSE, VolumeRecord->Volume_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Delete_Volume")

    return;

  }

#endif

#endif

  /* Is this volume the installable volume? */
  if ( Install_Volume_Handle == VolumeRecord->Volume_Handle )
    Install_Volume_Handle = NULL;

  /* Now dispose of the external handle used to reference this volume. */
  Destroy_Handle( VolumeRecord->External_Handle, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == HANDLE_MANAGER_NO_ERROR );

#else

  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Delete_Volume")

    return;

  }

#endif

#endif

  /* Get the partition data associated with the volume we are deleting. */
  PartitionRecord = VolumeRecord->Partition;

  /* Is this an LVM Volume or a Compatibility Volume? */
  if ( ! VolumeRecord->Compatibility_Volume )
  {

    /* We have an LVM volume.  This means that we will have an LVM Signature sector for each partition, and
       we must call each feature to allow the feature to remove this partition from its data structures.      */

    /* Call each feature and remove that feature from the partition. */
    Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;
    Old_Function_Table->Delete( PartitionRecord, TRUE, Error_Code);

    /* Did we succeed? */
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      API_EXIT("Delete_Volume")

      return;

    }

  }
  else
  {

    /* Now we can delete this partition. */

    /* Delete the partition's volume handle.  This will prevent the Delete_Partition function from thinking that this partition
       is part of an active volume, in which case it will not delete the partition.  Also, if this is a "fake" volume, it will
       prevent the Convert_Fake_Volumes_On_PRM_To_Real_Volumes function from trying to convert it.                                  */
    PartitionRecord->Volume_Handle = NULL;
    PartitionRecord->External_Volume_Handle = NULL;

    /* If this partition is part of a "fake" volume, then we must convert the volume to a real volume before deleting the partition.
       The only exception to this is if we are in Big Floppy Mode, in which case the partition itself is a fake!                       */
    if ( DriveArray[PartitionRecord->Drive_Index].Fake_Volumes_In_Use )
    {

      /* Is this disk in Big Floppy Mode? */
      if ( DriveArray[PartitionRecord->Drive_Index].Is_Big_Floppy )
      {

        /* Since we are in Big Floppy Mode, there can be only one volume for this disk.  Since we are deleting that volume,
           then there are no other volumes ( fake or real ) on the drive.  Therefore, we must Indicate that fake volumes
           are no longer in use on this disk.                                                                                  */
        DriveArray[PartitionRecord->Drive_Index].Fake_Volumes_In_Use = FALSE;

      }
      else
      {

        /* Since we are not in Big Floppy Mode, there may be other "fake" volumes on the disk.  Convert them into real volumes. */
        Convert_Fake_Volumes_On_PRM_To_Real_Volumes( PartitionRecord->Drive_Index, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          API_EXIT("Delete_Volume")

          return;

        }

      }

    }

    /* Now that all of the preliminary work is done, call the Delete_Partition function. */
    Delete_Partition( PartitionRecord->External_Handle, Error_Code);

    /* Did we succeed? */
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      API_EXIT("Delete_Volume")

      /* Something is very wrong here!  We must have an internal error. */
      return ;

    }

  }

  /* Now we must free the drive letter assigned to this volume, if there is one. */
  if ( ( VolumeRecord->Drive_Letter_Preference >= 'C' ) &&
       ( VolumeRecord->Drive_Letter_Preference <= 'Z' )
     )
  {

    /* Is there another volume that wanted this drive letter? */
    if ( ( VolumeRecord->Drive_Letter_Conflict == 0 ) && ( VolumeRecord->Current_Drive_Letter == VolumeRecord->Drive_Letter_Preference ) )
    {

      /* Since there were no drive letter conflicts, we can free the drive letter so that it may be reused. */

      Drive_Letter_Mask = 0x1 << ( VolumeRecord->Drive_Letter_Preference - 'A' );

      Available_Drive_Letters = Available_Drive_Letters | Drive_Letter_Mask;

    }
    else
    {

      /* We must see how many other volumes want this drive letter.  We will count the number of volumes that
         want this drive letter.  If the count > 0, then there is another volume that still wants this drive
         letter.  In this case, we will not free the drive letter.                                              */

      /* Set up to do the count. */
      Drive_Letter_Count.Count = 0;
      Drive_Letter_Count.Drive_Letter = VolumeRecord->Drive_Letter_Preference;

      /* Now find out how many volumes have this drive letter preference. */
      ForEachItem( Volumes, &Count_Drive_Letter_Claims, &Drive_Letter_Count, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        API_EXIT("Delete_Volume")

        return;

      }

#endif

#endif

      /* How many claims for this drive letter did we find? */
      if ( Drive_Letter_Count.Count == 0 )
      {

        /* Since no other volumes claimed this drive letter, free the drive letter. */
        Drive_Letter_Mask = 0x1 << ( Drive_Letter_Count.Drive_Letter - 'A' );

        Available_Drive_Letters = Available_Drive_Letters | Drive_Letter_Mask;

      }

    }

  }

  /* If the drive letter preference of this volume was '*', or if the drive letter preference was not the same
     as the current drive letter for the volume, then we must remove the current drive letter assigned to it
     from the list of reserved drive letters.  It was put in that list to indicate that, while selectable for
     a drive letter assignment, a reboot might be required.  Since the volume is being deleted, a reboot will no-longer
     be required if the current drive letter assigned to this volume is used as another volume's drive letter preference. */
  if ( ( VolumeRecord->Current_Drive_Letter != 0 ) &&
       ( ( VolumeRecord->Drive_Letter_Preference == '*' ) ||
         ( VolumeRecord->Drive_Letter_Preference != VolumeRecord->Current_Drive_Letter)
       )
     )
  {

    Drive_Letter_Mask = 0x1;

    Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

    Reserved_Drive_Letters = Reserved_Drive_Letters & ~Drive_Letter_Mask;

  }

  /* Now we must add this volume's current drive letter to the list of deleted drive letters. */
  if ( ( VolumeRecord->Current_Drive_Letter != 0 ) && ( !VolumeRecord->New_Volume ) )
  {

    Drive_Letter_Mask = 0x1;

    Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

    Deleted_Drive_Letters = Deleted_Drive_Letters | Drive_Letter_Mask;

  }

  /* Now free the memory that is being used for the Volume_Data structure. */
  free(VolumeRecord);

  /* All done.  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Delete_Volume")

  return;

}


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
void Hide_Volume( ADDRESS Volume_Handle, CARDINAL32 * Error_Code )
{

  Volume_Data *              VolumeRecord;                    /* Used to point to the Volume_Data structure we are manipulating. */
  Partition_Data *           PartitionRecord;                 /* Used to access the partition that belongs to this volume if this volume is marked installable. */
  ADDRESS                    Object;                          /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                        ObjectTag;                       /* Used when translating the Volume_Handle into a Volume_Data structure. */
  CARDINAL32                 Drive_Letter_Mask = 0x1;         /* Used when releasing the drive letter assigned to the volume being hidden. */
  Partition_Data_To_Update   New_Values;                      /* Used to update values in the Partition Records associated with the Volume. */
  Drive_Letter_Count_Record  Drive_Letter_Count;              /* Used to count the number of volumes with the same drive letter preference. */


  API_ENTRY("Hide_Volume")

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Hide_Volume")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Volume_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Hide_Volume")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != VOLUME_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Hide_Volume")

    return;

  }

  /* Establish access to the Volume_Data structure we want to manipulate. */
  VolumeRecord = ( Volume_Data * ) Object;

  /* If the volume can not be changed, abort. */
  if ( ! VolumeRecord->Can_Be_Altered )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Hide_Volume")

    return;

  }

  /* If the device is not a hard drive or PRM, abort! */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Hide_Volume")

    return;

  }

  /* Now we must free the drive letter assigned to this volume, if there is one. */
  if ( ( VolumeRecord->Drive_Letter_Preference >= 'C' ) &&
       ( VolumeRecord->Drive_Letter_Preference <= 'Z' )
     )
  {

    /* Is there another volume that wanted this drive letter? */
    if ( ( VolumeRecord->Drive_Letter_Conflict == 0 ) && ( VolumeRecord->Current_Drive_Letter == VolumeRecord->Drive_Letter_Preference ) )
    {

      /* Since there were no drive letter conflicts, we can free the drive letter so that it may be reused. */

      Drive_Letter_Mask = 0x1 << ( VolumeRecord->Drive_Letter_Preference - 'A' );

      Available_Drive_Letters = Available_Drive_Letters | Drive_Letter_Mask;

    }
    else
    {

      /* We must see how many other volumes want this drive letter.  We will count the number of volumes that
         want this drive letter.  If the count > 1, then there is another volume that still wants this drive
         letter.  In this case, we will not free the drive letter.                                              */

      /* Set up to do the count. */
      Drive_Letter_Count.Count = 0;
      Drive_Letter_Count.Drive_Letter = VolumeRecord->Drive_Letter_Preference;

      /* Now find out how many volumes have this drive letter preference. */
      ForEachItem( Volumes, &Count_Drive_Letter_Claims, &Drive_Letter_Count, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        API_EXIT("Hide_Volume")

        return;

      }

#endif

#endif

      /* How many claims for this drive letter did we find? */
      if ( Drive_Letter_Count.Count == 1 )
      {

        /* Since no other volumes claimed this drive letter, free the drive letter. */
        Drive_Letter_Mask = 0x1 << ( Drive_Letter_Count.Drive_Letter - 'A' );

        Available_Drive_Letters = Available_Drive_Letters | Drive_Letter_Mask;

      }

    }

  }

  /* If the drive letter preference of this volume was '*', or if the drive letter preference was not the same
     as the current drive letter for the volume, then we must remove the current drive letter assigned to it
     from the list of reserved drive letters.  It was put in that list to indicate that, while selectable for
     a drive letter assignment, a reboot might be required.  Since the volume is being deleted, a reboot will no-longer
     be required if the current drive letter assigned to this volume is used as another volume's drive letter preference. */
  if ( ( VolumeRecord->Current_Drive_Letter != 0 ) &&
       ( ( VolumeRecord->Drive_Letter_Preference == '*' ) ||
         ( VolumeRecord->Drive_Letter_Preference != VolumeRecord->Current_Drive_Letter)
       )
     )
  {

    Drive_Letter_Mask = 0x1;

    Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

    Reserved_Drive_Letters = Reserved_Drive_Letters & ~Drive_Letter_Mask;

  }

  /* Now we must add this volume's current drive letter to the list of deleted drive letters. */
  if ( ( VolumeRecord->Current_Drive_Letter != 0 ) && ( !VolumeRecord->New_Volume ) )
  {

    Drive_Letter_Mask = 0x1;

    Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

    Deleted_Drive_Letters = Deleted_Drive_Letters | Drive_Letter_Mask;

    /* Now we must remove the volume's current drive letter. */
    VolumeRecord->Current_Drive_Letter = 0;

  }

  /* Remove the Volume's drive letter assignment. */
  VolumeRecord->Drive_Letter_Preference = 0x0;

  /* Indicate that changes were made to the Volume's data. */
  VolumeRecord->ChangesMade = TRUE;

  /* If this is a "fake" volume, then turn it into a real volume.  Only Compatibility Volumes can be "fake" volumes. */
  if ( VolumeRecord->Compatibility_Volume && DriveArray[VolumeRecord->Partition->Drive_Index].Fake_Volumes_In_Use )
  {

    Convert_Fake_Volumes_On_PRM_To_Real_Volumes( VolumeRecord->Partition->Drive_Index, Error_Code);
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      API_EXIT("Hide_Volume")

      return;

    }

  }


  /* Now prepare to update the partition records associated with this volume. */
  New_Values.Update_Drive_Letter = TRUE;
  New_Values.New_Drive_Letter = 0x00;
  New_Values.Update_Volume_Name = FALSE;
  New_Values.New_Name = NULL;
  New_Values.Update_Boot_Manager_Status = FALSE;
  New_Values.On_Menu = FALSE;
  New_Values.Update_Volume_Spanning = FALSE;
  New_Values.Spanned_Volume = FALSE;

  /* Now, for every partition that is a part of the volume, remove the partition's drive letter assignement. */
  Update_Partitions_Volume_Data(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &New_Values, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Hide_Volume")

    return;

  }

#endif

#endif

  /* Is this volume marked installable? */
  if ( Install_Volume_Handle == VolumeRecord->Volume_Handle )
  {

    /* This volume is marked installable.  We must change this as you can not install to a hidden volume! */

    /* The volume must be a compatibility volume.  Lets get its partition data and examine it. */
    PartitionRecord = VolumeRecord->Partition;

    /* Now turn off the install flag in the partition's DLA Table Entry. */
    PartitionRecord->DLA_Table_Entry.Installable = FALSE;

    /* Mark the drive that the partition is on as having been changed. */
    DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    /* Reset Install_Volume_Handle as there is no-longer an installable volume. */
    Install_Volume_Handle = NULL;

  }


  /* All done.  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Hide_Volume")

  return;

}


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
void Expand_Volume ( ADDRESS         Volume_Handle,
                     CARDINAL32      Partition_Count,
                     ADDRESS         Partition_Handles[],
                     CARDINAL32 *    Error_Code
                   )
{

  Volume_Data *                      VolumeRecord;             /* Used to point to the Volume_Data structure we are manipulating. */
  Partition_Data *                   PartitionRecord;          /* Used when creating the New_Partitions list. */
  ADDRESS                            Object;                   /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                                ObjectTag;                /* Used when translating the Volume_Handle into a Volume_Data structure. */
  CARDINAL32                         Drive_Letter_Mask;        /* Used to test the drive letter bitmap to see if a drive letter is available or not. */
  CARDINAL32                         Ignore_Error;             /* Used on error paths. */
  Partition_Data_To_Update           New_Values;               /* Used to update partitions which are already a part of this volume. */
  DLIST                              New_Partitions;           /* Used to hold the partitions/aggregates which are being used to expand the volume. */
  DLIST                              Features_List;            /* Used to hold the features which exist on the volume being expanded. */
  CARDINAL32                         CurrentPartition;         /* Used to walk the Partition_Handles array. */
  CARDINAL32                         Target_Feature_ID;        /* Used to hold the numeric ID of the feature which will perform the expansion. */
  Plugin_Function_Table_V1 *         Function_Table;           /* Used to access the function table associated with the aggregate. */
  Apply_Features_Parameter_Record    Apply_Features_Data;      /* Used when applying features during the creation of an LVM Volume. */
  CARDINAL32                         Count;                    /* Used when setting the sequence numbers of partitions in a volume. */

  API_ENTRY("Expand_Volume")

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Expand_Volume")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Volume_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Expand_Volume")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != VOLUME_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Expand_Volume")

    return;

  }

  /* Establish access to the Volume_Data structure we want to manipulate. */
  VolumeRecord = ( Volume_Data * ) Object;

  /* Is this volume a compatibility volume? */
  if ( VolumeRecord->Compatibility_Volume )
  {

    /* You can not expand a compatibility volume! */
    *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

    API_EXIT("Expand_Volume")

    return;

  }

  /* If the volume can not be changed, abort. */
  if ( ! VolumeRecord->Can_Be_Altered )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Expand_Volume")

    return;

  }

  /* If the device is not a hard drive or PRM, abort! */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Expand_Volume")

    return;

  }

  /* If the volume is hidden, or if the volume does not have an active drive letter assignment, abort! */
  if ( ( VolumeRecord->Drive_Letter_Preference < 'C') || ( VolumeRecord->Drive_Letter_Preference > 'Z' ) ||
       ( ( ! VolumeRecord->New_Volume ) &&
         ( ( VolumeRecord->Initial_Drive_Letter < 'C') ||
           ( VolumeRecord->Initial_Drive_Letter > 'Z' )
         )
       )
     )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Expand_Volume")

    return;

  }

  if ( ! VolumeRecord->Filesystem_Is_Expandable )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Expand_Volume")

    return;

  }

  /* We need to find out which feature on the volume will perform the expansion. */

  /* Get the function table associated with the aggregate. */
  PartitionRecord = VolumeRecord->Partition;

  if ( PartitionRecord == NULL )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Expand_Volume")

    return;

  }

  Function_Table = (Plugin_Function_Table_V1 *) PartitionRecord->Feature_Data->Function_Table;

  /* If no features are willing to perform the expansion of the volume, then abort! */
  if ( ! Function_Table->Can_Expand( PartitionRecord, &Target_Feature_ID, Error_Code ) )
  {

    if ( *Error_Code == LVM_ENGINE_NO_ERROR )
      *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Expand_Volume")

    return;

  }

  /* We need to validate the Partition Handles passed to us. */
  if ( ! Partition_List_Is_Valid( Partition_Count, Partition_Handles, (VolumeRecord->Device_Type == LVM_PRM), Error_Code ) )
  {

    API_EXIT("Expand_Volume")

    /* If there was an error, or if the Partition list was not valid, *Error_Code will already be set.  All we have to do is return. */
    return;

  }

  /* Lets allocate all of the LVM Signature Sectors.  There is one per partition. */
  if ( ! Allocate_Signature_Sectors( Partition_Count, Partition_Handles, FALSE, Error_Code ) )
  {

    /* We must free any memory we have already allocated! */
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );

    API_EXIT("Expand_Volume")

    return;

  }

  /* We need to get a list of features for the volume we are expanding. */
  Features_List = CreateList();
  if ( Features_List == NULL )
  {

    /* Free the memory occupied by the LVM Signature Sectors we allocated for the new partitions. */
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );

    /* We are out of memory!  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    API_EXIT("Expand_Volume")

    return;

  }

  /* Get the features on the volume. */
  Build_Features_List( (ADDRESS) PartitionRecord, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, Features_List, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the memory occupied by the LVM Signature Sectors we allocated for the new partitions. */
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );

    /* Set the error code. */
    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Expand_Volume")

    return;

  }

  /* Now that we have the features on the volume, sort them by their Feature Sequence Number.  This will ensure that
     they will be applied to the new partitions in the correct order.                                                 */
  SortList(Features_List, &Sort_By_Feature_Sequence_Number, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the FeaturesToUse list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    /* Free the memory occupied by the LVM Signature Sectors we allocated for the new partitions. */
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );

    /* Set the error code. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Expand_Volume")

    return;

  }

  /* Allocate a list to hold the partitions that are to become part of the volume. */
  New_Partitions = CreateList();
  if ( New_Partitions == NULL )
  {

    /* Free the FeaturesToUse list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    /* Free the memory occupied by the LVM Signature Sectors we allocated for each of the new partitions. */
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );

    /* Set the return code. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    API_EXIT("Expand_Volume")

    return;

  }

  /* Now we must put the partitions into the New_Partitions List. */
  for ( CurrentPartition = 0; CurrentPartition < Partition_Count; CurrentPartition++ )
  {

    /* We must translate the handle.  */

    /* Translate the handle. */
    Translate_Handle( Partition_Handles[CurrentPartition], &Object, &ObjectTag, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == HANDLE_MANAGER_NO_ERROR );

#else

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      /* Since we have translated this handle once already, there must be an internal error! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Expand_Volume")

      return;

    }

#endif

#endif

    /* From the ObjectTag we can tell what Object points to. */

#ifdef DEBUG

#ifdef PARANOID

    assert( ObjectTag == PARTITION_DATA_TAG );

#else

    /* Was the handle valid? */
    if ( ObjectTag != PARTITION_DATA_TAG )
    {

      /* Since we have translated this handle once already, there must be an internal error! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Expand_Volume")

      return;

    }

#endif

#endif

    /* Establish access to the Partition_Data structure we want to manipulate. */
    PartitionRecord = ( Partition_Data * ) Object;

    /* Ensure that this partition is not on the Boot Manager Menu. */
    PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;

    /* Update a few fields in the LVM Signature Sector for this PartitionRecord. */
    PartitionRecord->Signature_Sector->Drive_Letter = VolumeRecord->Drive_Letter_Preference;
    PartitionRecord->Signature_Sector->Volume_Serial_Number = VolumeRecord->Volume_Serial_Number;
    strncpy(PartitionRecord->Signature_Sector->Volume_Name, VolumeRecord->Volume_Name, VOLUME_NAME_SIZE);

    /* Now add the partition to the New_Partitions List. */
    InsertObject(New_Partitions, sizeof(Partition_Data), PartitionRecord, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

    /* Did we succeed? */
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* Are we out of memory? */
      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      {

        /* Clean up what we can. */
        Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
        DestroyList(&New_Partitions,FALSE, &Ignore_Error);
        DestroyList(&Features_List,FALSE, &Ignore_Error);

        /* Set the error code. */
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        API_EXIT("Expand_Volume")

        return;

      }
      else
      {

        /* This is some kind of an internal error! */

#ifdef DEBUG

#ifdef PARANOID

        assert(0);

#endif

#endif

        /* Set the error code. */
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        API_EXIT("Expand_Volume")

        return;

      }

    }

  }

  /* Now that all of the partitions are successfully in the New_Partitions List, and each Partition has
     an LVM Signature Sector, lets initialize each of the features that will be active on these partitions.         */

  /* Set up to process each feature on all of the partitions in the New_Partitions list. */
  Apply_Features_Data.Partition_List = New_Partitions;
  Apply_Features_Data.Error_Code = Error_Code;
  Apply_Features_Data.VolumeRecord = VolumeRecord;
  Apply_Features_Data.Feature_To_Stop_At = Target_Feature_ID;
  ForEachItem(Features_List, &Apply_Features, &Apply_Features_Data, TRUE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Clean up what we can. */
    ForEachItem(New_Partitions, &Remove_All_Partition_Features , NULL, TRUE, &Ignore_Error);
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
    DestroyList(&New_Partitions,FALSE, &Ignore_Error);
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    /* Set the error code. */
    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Expand_Volume")

    return ;

  }

  /* Now that all of the features below the one performing the expansion have been added to the new partitions,
     it is time to perform the expansion itself.                                                                 */

  /* Call the Add_Partition entry point for the topmost feature on the volume.  This feature, if it will not be
     performing the expansion itself, will pass the request down to the next feature, which will do likewise, until
     the feature which can perform the expansion is reached.  After this feature performs the expansion, the
     call stack will unwind, giving each feature above the one performing the expansion a chance to update its
     internal data.  When the call returns here, all of the features above the one performing the expansion will
     have updated their internal data, and we can then update the Volume_Data for the volume itself.                 */
  Function_Table->Add_Partition ( VolumeRecord->Partition, New_Partitions, Error_Code );
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* Clean up what we can. */
    ForEachItem(New_Partitions, &Remove_All_Partition_Features , NULL, TRUE, &Ignore_Error);
    Free_Signature_Sectors( Partition_Count, Partition_Handles, &Ignore_Error );
    DestroyList(&New_Partitions,FALSE, &Ignore_Error);
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    API_EXIT("Expand_Volume")

    /* The error code has already been set by the call to Add_Partition. */
    return ;

  }

  /* Since all of the operations were successful, lets update the VolumeRecord and all of the PartitionRecords. */

  /* Update all of the existing partitions in the volume to ensure that their spanned volume flags are on. */
  New_Values.Update_Drive_Letter = FALSE;
  New_Values.New_Drive_Letter = 0;
  New_Values.Update_Volume_Name = FALSE;
  New_Values.New_Name = NULL;
  New_Values.Update_Boot_Manager_Status = FALSE;
  New_Values.On_Menu = FALSE;
  New_Values.Update_Volume_Spanning = TRUE;
  New_Values.Spanned_Volume = TRUE;

  /* Change every partition that is already a part of this volume.  */
  Update_Partitions_Volume_Data(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &New_Values, Error_Code);

  /* Update the count of partitions in the volume. */
  VolumeRecord->Partition_Count += Partition_Count;

  /* Update all of the new partitions in the volume. */
  Update_Partition_Data(VolumeRecord, Partition_Count, Partition_Handles, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == LVM_ENGINE_NO_ERROR);

#else

  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* We have been through these partitions several times already, so the only
       possible error here is an internal error of some sort!                    */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Expand_Volume")

    return;

  }

#endif

#endif

  /* Now that all of the partitions in the partition list have been initialized, along with their corresponding Signature Sectors
     and Feature data, we can finally save the new size of the volume.                                                            */
  VolumeRecord->Volume_Size = VolumeRecord->Partition->Usable_Size;

  /* Mark the VolumeRecord as having been expanded. */
  VolumeRecord->Expand_Volume = TRUE;

  /* Mark the VolumeRecord as having been changed. */
  VolumeRecord->ChangesMade = TRUE;


  /* We must update the parent pointers in each of the partitions in the volume.  This should NEVER fail! */
  Set_Volume_Parent_Child_Pointers(VolumeRecord, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    API_EXIT("Expand_Volume")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

  /* We must update the sequence numbers of the partitions in the volume.  This should NEVER fail! */
  Count = 1;
  Set_Partition_Sequence_Numbers(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &Count, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    API_EXIT("Expand_Volume")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

  /* We need to save the Next_Aggregate_Number in the LVM Signature Sector of the topmost aggregate.  Remember that
     Aggregates produced by the first version of Drive Linking do not have LVM Signature Sectors!                   */
  if ( VolumeRecord->Partition->Signature_Sector != NULL )
    VolumeRecord->Partition->Signature_Sector->Next_Aggregate_Number = VolumeRecord->Next_Aggregate_Number;


  /* Expand operations are modelled as a delete followed by an add.  When the volume is rediscovered, it will be rediscovered at the larger size.
     For volumes using filesystems which do not support Quiesce and Resume, we will add the drive letter for the volume  to the
     Deleted Drive Letters.  This will cause the volume to be deleted from the IFSM after the IFSM has unmounted the volume.  If the
     Volume can not be unmounted (open files will cause this), then a reboot will be forced.
     For volumes using filesystems which support Quiesce and Resume, then we don't need to force an IFSM delete of the volume, and we
     don't want to add it to the Deleted Drive Letters as this will cause problems.                                                                   */

  if ( ! VolumeRecord->Quiesce_Resume_Supported )
  {

    /* We must add the drive letter for this drive to the Deleted Drive Letters.  We must use
       the Current_Drive_Letter field so that the correct drive is deleted.                    */
    if ( VolumeRecord->Current_Drive_Letter != 0 )
    {

      /* Convert the Volume's drive letter to an ordinal. */
      Drive_Letter_Mask = (CARDINAL32) (VolumeRecord->Current_Drive_Letter - 'A');

      /* Calculate the bit in our Available_Drive_Letters bitmap which corresponds to the drive letter. */
      Drive_Letter_Mask = 0x1 << Drive_Letter_Mask;

      /* Mark the drive letter as being deleted.  Modify operations are modeled as deletes followed by adds. */
      Deleted_Drive_Letters = Deleted_Drive_Letters | Drive_Letter_Mask;

      /* Set the current drive letter to 0 as there is a possibility that the volume may be assigned a different drive letter after being rediscovered. */
      VolumeRecord->Current_Drive_Letter = 0;

    }

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Expand_Volume")

  return;

}


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
void Assign_Drive_Letter( ADDRESS      Volume_Handle,
                          char         New_Drive_Preference,
                          CARDINAL32 * Error_Code
                        )
{

  Volume_Data *             VolumeRecord;       /* Used to point to the Volume_Data structure we are manipulating. */
  ADDRESS                   Object;             /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                       ObjectTag;          /* Used when translating the Volume_Handle into a Volume_Data structure. */
  CARDINAL32                Drive_Letter_Mask;  /* Used to test the drive letter bitmap to see if a drive letter is available or not. */
  Partition_Data_To_Update  New_Values;         /* Used to actually set the new drive letters. */
  Drive_Letter_Count_Record Drive_Letter_Count; /* Used to count the number of volumes claiming  a drive letter. */

  API_ENTRY("Assign_Drive_Letter")

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Volume_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != VOLUME_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* Establish access to the Volume_Data structure we want to manipulate. */
  VolumeRecord = ( Volume_Data * ) Object;

  /* If the volume can not be changed, abort. */
  if ( ! VolumeRecord->Can_Be_Altered )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* If the device is not a hard drive or PRM, abort! */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* Uppercase the new drive letter. */
  New_Drive_Preference = (char) toupper(New_Drive_Preference);

  /* Is the drive letter within the range of A to Z, or *? */
  if ( ( New_Drive_Preference != '*' ) && ( ( New_Drive_Preference < 'C' ) || ( New_Drive_Preference > 'Z' ) ) )
  {

    /* The drive letter specified is out of range. */
    *Error_Code = LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* Is the new drive letter the same as the old one? */
  if ( VolumeRecord->Drive_Letter_Preference == New_Drive_Preference )
  {

    /* We are not changing the drive letter preference!  We are essentially done. */

    /* Is there an existing drive letter conflict?  If so, we will return an error to remind the user to fix this! */
    if ( VolumeRecord->Drive_Letter_Conflict != 0 )
    {

      *Error_Code = LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE;

    }
    else
    {

      /* If this is a "fake" volume, then turn it into a real volume.  Only Compatibility Volumes can be "fake" volumes. */
      if ( VolumeRecord->Compatibility_Volume && DriveArray[VolumeRecord->Partition->Drive_Index].Fake_Volumes_In_Use )
      {

        Convert_Fake_Volumes_On_PRM_To_Real_Volumes( VolumeRecord->Partition->Drive_Index, Error_Code);

      }
      else
        *Error_Code = LVM_ENGINE_NO_ERROR;

    }

    API_EXIT("Assign_Drive_Letter")

    return;

  }

  /* If the new drive letter is not '*' then we have some work to do. */
  if ( New_Drive_Preference != '*' )
  {

    /* Convert the new drive letter to an ordinal. */
    Drive_Letter_Mask = (CARDINAL32) (New_Drive_Preference - 'A');

    /* Calculate the bit in our Available_Drive_Letters bitmap which corresponds to the new drive letter. */
    Drive_Letter_Mask = 0x1 << Drive_Letter_Mask;

    /* Is that bit on in the Available_Drive_Letters variable? */
    if ( ( Available_Drive_Letters & Drive_Letter_Mask ) == 0 )
    {

      /* The drive letter is not available! */
      *Error_Code = LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE;

      API_EXIT("Assign_Drive_Letter")

      return;

    }

    /* Mark the drive letter as being in use. */
    Available_Drive_Letters = Available_Drive_Letters & (~Drive_Letter_Mask);

    /* It is possible that the drive letter assigned to this volume conflicts with the current drive letter assigned to a volume whose
       drive letter preference is a '*'.  Another possibility is that it conflicts with the current drive letter assigned to a volume
       whose drive letter preference conflicted with that of another volume.  In these cases, we must find the volume with the current
       drive letter which conflicts with the drive letter preference for this volume and zero out the current drive letter field.          */
    ForEachItem(Volumes, &Find_Current_Drive_Letter_Conflicts, &New_Drive_Preference, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Assign_Drive_Letter")

      return;

    }

#endif

#endif

  }

  /* Now we must free the drive letter assigned to this volume, if there is one. */
  if ( ( VolumeRecord->Drive_Letter_Preference >= 'C' ) &&
       ( VolumeRecord->Drive_Letter_Preference <= 'Z' )
     )
  {

    /* Is there another volume that wanted this drive letter? */
    if ( ( VolumeRecord->Drive_Letter_Conflict == 0 ) && ( VolumeRecord->Current_Drive_Letter == VolumeRecord->Drive_Letter_Preference ) )
    {

      /* Since there were no drive letter conflicts, we can free the drive letter so that it may be reused. */

      Drive_Letter_Mask = 0x1 << ( VolumeRecord->Drive_Letter_Preference - 'A' );

      Available_Drive_Letters = Available_Drive_Letters | Drive_Letter_Mask;

    }
    else
    {

      /* We must see how many other volumes want this drive letter.  We will count the number of volumes that
         want this drive letter.  If the count > 1, then there is another volume that still wants this drive
         letter.  In this case, we will not free the drive letter.                                              */

      /* Set up to do the count. */
      Drive_Letter_Count.Count = 0;
      Drive_Letter_Count.Drive_Letter = VolumeRecord->Drive_Letter_Preference;

      /* Now find out how many volumes have this drive letter preference. */
      ForEachItem( Volumes, &Count_Drive_Letter_Claims, &Drive_Letter_Count, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        API_EXIT("Assign_Drive_Letter")

        return;

      }

#endif

#endif

      /* How many claims for this drive letter did we find? */
      if ( Drive_Letter_Count.Count == 1 )
      {

        /* Since no other volumes claimed this drive letter, free the drive letter. */
        Drive_Letter_Mask = 0x1 << ( Drive_Letter_Count.Drive_Letter - 'A' );

        Available_Drive_Letters = Available_Drive_Letters | Drive_Letter_Mask;

      }

    }

  }

  /* If the drive letter preference of this volume was '*', or if the drive letter preference was not the same
     as the current drive letter for the volume, then we must remove the current drive letter assigned to it
     from the list of reserved drive letters.  It was put in that list to indicate that, while selectable for
     a drive letter assignment, a reboot might be required.  Since the drive letter of this volume is being changed,
     a reboot will no-longer be required if the current drive letter assigned to this volume is used as another volume's
     drive letter preference.                                                                                             */
  if ( ( VolumeRecord->Current_Drive_Letter != 0 ) &&
       ( ( VolumeRecord->Drive_Letter_Preference == '*' ) ||
         ( VolumeRecord->Drive_Letter_Preference != VolumeRecord->Current_Drive_Letter)
       )
     )
  {

    Drive_Letter_Mask = 0x1;

    Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

    Reserved_Drive_Letters = Reserved_Drive_Letters & ~Drive_Letter_Mask;

  }

  /* Add this to our list of Deleted_Drive_Letters as OS2LVM.DMD will model this change as a delete followed by an add.
     However, if all we are doing is assigning the current drive letter as the drive letter preference, then we don't
     need to do anything but write the new DLA Table entry and, if applicable, the new LVM Signature Sectors.  We don't
     need to do the full Delete/Add cycle.                                                                               */
  if ( ( VolumeRecord->Current_Drive_Letter != 0 ) &&
       ( !VolumeRecord->New_Volume ) &&
       ( VolumeRecord->Current_Drive_Letter != New_Drive_Preference )
     )
  {

    Drive_Letter_Mask = 0x1;

    Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

    Deleted_Drive_Letters = Deleted_Drive_Letters | Drive_Letter_Mask;

    /* Set the Current Drive Letter for this volume to NULL. */
    VolumeRecord->Current_Drive_Letter = 0;

  }

  /* Update VolumeData to reflect the new drive letter preference. */
  VolumeRecord->Drive_Letter_Preference = New_Drive_Preference;
  VolumeRecord->Drive_Letter_Conflict = 0;
  VolumeRecord->ChangesMade = TRUE;

  /* Is this the volume marked installable?  If it is, then we must check to see if Boot Manager is installed.  If Boot Manager
     is NOT installed, then, if the drive letter preference is not 'C', we must turn off the Installable flag as the volume
     can no longer be installed to!                                                                                              */
  if ( ( ( Boot_Manager_Handle == NULL ) ||
         ( Boot_Manager_Active == FALSE )
       ) &&
       ( VolumeRecord->Drive_Letter_Preference != 'C' ) &&
       ( VolumeRecord->Volume_Handle  == Install_Volume_Handle )
     )
  {

    /* We must turn off the Installable flag! */
    VolumeRecord->Partition->DLA_Table_Entry.Installable = FALSE;

    /* Since there is nolonger a volume marked installable, clear the Install_Volume_Handle. */
    Install_Volume_Handle = NULL;

  }

  /* Set up to change the drive letter preference listed in each of the partition records associated with this volume. */
  New_Values.Update_Drive_Letter = TRUE;
  New_Values.New_Drive_Letter = New_Drive_Preference;
  New_Values.Update_Volume_Name = FALSE;
  New_Values.New_Name = NULL;
  New_Values.Update_Boot_Manager_Status = FALSE;
  New_Values.On_Menu = FALSE;
  New_Values.Update_Volume_Spanning = FALSE;
  New_Values.Spanned_Volume = FALSE;

  /* Change every partition that is a part of this volume to have this drive letter assigned to them. */
  Update_Partitions_Volume_Data(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &New_Values, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == DLIST_SUCCESS);

#else

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* This should not have happened!  We must have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Assign_Drive_Letter")

    return;

  }

#endif

#endif

  /* If this is a "fake" volume, then turn it into a real volume.  Only Compatibility Volumes can be "fake" volumes. */
  if ( VolumeRecord->Compatibility_Volume && DriveArray[VolumeRecord->Partition->Drive_Index].Fake_Volumes_In_Use )
    Convert_Fake_Volumes_On_PRM_To_Real_Volumes( VolumeRecord->Partition->Drive_Index, Error_Code);
  else
  {

    /* Indicate success. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  API_EXIT("Assign_Drive_Letter")

  return;

}


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
void Set_Installable ( ADDRESS Volume_Handle, CARDINAL32 * Error_Code )
{

  Volume_Data *                VolumeRecord;                    /* Used to point to the Volume_Data structure we are manipulating. */
  Volume_Data *                Old_Volume;                      /* Used to point to a volume which is already marked installable so that we can turn off the installable flag. */
  Partition_Data *             PartitionRecord;                 /* Used to access the partition associated with the compatiblity volume to be marked installable. */
  ADDRESS                      Object;                          /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                          ObjectTag;                       /* Used when translating the Volume_Handle into a Volume_Data structure. */
  Drive_Letter_Count_Record    Drive_Letter_Count;              /* Used to count the number of volumes with the same drive letter preference. */


  API_ENTRY("Set_Installable")

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Set_Installable")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Volume_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Set_Installable")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != VOLUME_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Set_Installable")

    return;

  }

  /* Establish access to the Volume_Data structure we want to manipulate. */
  VolumeRecord = ( Volume_Data * ) Object;

  /* If the volume can not be changed, abort. */
  if ( ! VolumeRecord->Can_Be_Altered )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Set_Installable")

    return;

  }

  /* If the device is not a hard drive or PRM, abort! */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Set_Installable")

    return;

  }

  /* Is this volume large enough to be marked installable? */
  if ( VolumeRecord->Volume_Size < Min_Install_Size )
  {

    *Error_Code = LVM_ENGINE_VOLUME_TOO_SMALL;

    API_EXIT("Set_Installable")

    return;

  }

  /* Is this volume a compatibility volume? */
  if ( ! VolumeRecord->Compatibility_Volume )
  {

    /* You can only install to compatibility volumes! */
    *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

    API_EXIT("Set_Installable")

    return;

  }

#ifdef DEBUG

#ifdef PARANOID

  assert(VolumeRecord->Partition_Count == 1);

#else

  if ( VolumeRecord->Partition_Count != 1)
  {

    API_EXIT("Set_Installable")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

#endif

#endif

  /* Is this volume hidden?  You can't install to a hidden volume! */
  if ( ( VolumeRecord->Drive_Letter_Preference < 'C' ) || (VolumeRecord->Drive_Letter_Preference > 'Z' ) )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Set_Installable")

    return;

  }

  /* Does this volume have a drive letter conflict? */

  /* Set up to count how many volumes have the same drive letter preference.. */
  Drive_Letter_Count.Count = 0;
  Drive_Letter_Count.Drive_Letter = VolumeRecord->Drive_Letter_Preference;

  /* Now find out how many volumes have this drive letter preference. */
  ForEachItem( Volumes, &Count_Drive_Letter_Claims, &Drive_Letter_Count, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Set_Installable")

    return;

  }

#endif

#endif

  /* How many claims for this drive letter did we find? */
  if ( Drive_Letter_Count.Count > 1 )
  {

    /* The drive letter preference is not unique!  This could cause problems during installation, so we will reject this volume. */
    *Error_Code = LVM_ENGINE_BAD_DRIVE_LETTER_PREFERENCE;

    API_EXIT("Set_Installable")

    return;

  }


  /* Is this volume bootable? */
  if ( ! Is_Volume_Bootable(VolumeRecord, TRUE, Error_Code) || ( *Error_Code != LVM_ENGINE_NO_ERROR ) )
  {

    if ( *Error_Code == LVM_ENGINE_NO_ERROR )
      *Error_Code = LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE;

    API_EXIT("Set_Installable")

    return;

  }

  /* Is there another volume marked installable? */
  if ( Install_Volume_Handle != NULL )
  {

    /* Another volume is already marked installable.  Before we can mark the new volume installable, we must "unmark" the old one. */

    /* Get the old volume's data. */
    Old_Volume = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Install_Volume_Handle, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Set_Installable")

      return;

    }

#endif

#endif

#ifdef DEBUG

#ifdef PARANOID

    assert( Old_Volume->Compatibility_Volume );
    assert( Old_Volume->Partition_Count == 1 );

#else

    if ( ( ! Old_Volume->Compatibility_Volume ) ||
         ( Old_Volume->Partition_Count != 1)
       )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Set_Installable")

      return;

    }

#endif

#endif

    /* The old volume must be a compatibility volume.  Lets get its partition data and examine it. */
    PartitionRecord = Old_Volume->Partition;

    /* Now turn off the install flag in the partition's DLA Table Entry. */
    PartitionRecord->DLA_Table_Entry.Installable = FALSE;

    /* Mark the drive that the partition is on as having been changed. */
    DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  }

  /* Now we can get the partition associated with this compatibility volume and mark it as being installable. */
  PartitionRecord = VolumeRecord->Partition;

  /* If Boot Manager is not installed, this volume must be startable! */
  if ( ( Boot_Manager_Handle == NULL ) || (! Boot_Manager_Active ) )
  {

    /* Try to mark the volume startable. */
    Set_Startable ( PartitionRecord->External_Volume_Handle, Error_Code );
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      API_EXIT("Set_Installable")

      /* We can not set this partition startable, which means that we can not boot it!
         If we can't boot it, we can't install to it!  Abort.                              */
      return;

    }

  }

  /* Now turn on the install flag in the partition's DLA Table Entry. */
  PartitionRecord->DLA_Table_Entry.Installable = TRUE;

  /* Turn on the On_Boot_Manager flag in the partition's DLA Table Entry, whether or not Boot Manager is installed. */
  PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = TRUE;

  /* Update the VolumeRecord to show that it is on the Boot Manager Menu, whether or not Boot Manager is installed. */
  VolumeRecord->On_Boot_Manager_Menu = TRUE;

  /* Mark the drive that the partition is on as having been changed. */
  DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  /* Update the global variable holding the handle of the volume currently marked installable. */
  Install_Volume_Handle = VolumeRecord->Volume_Handle;

  /* If this is a "fake" volume, then turn it into a real volume.  Only Compatibility Volumes can be "fake" volumes. */
  if ( VolumeRecord->Compatibility_Volume && DriveArray[VolumeRecord->Partition->Drive_Index].Fake_Volumes_In_Use )
    Convert_Fake_Volumes_On_PRM_To_Real_Volumes( VolumeRecord->Partition->Drive_Index, Error_Code);
  else
  {

    /* Indicate success. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  API_EXIT("Set_Installable")

  return;

}


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
Volume_Information_Record Get_Installable_Volume ( CARDINAL32 * Error_Code )
{

  Volume_Information_Record  ReturnValue;
  Volume_Data *              VolumeRecord;

  API_ENTRY("Get_Installable_Volume")

  /* Assume failure. */
  memset(&ReturnValue,0,sizeof(Volume_Information_Record) );


  /* Is there a volume marked installable? */
  if ( Install_Volume_Handle != NULL)
  {

    /* Get the volume's data. */
    VolumeRecord = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Install_Volume_Handle, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Get_Installable_Volume")

      return ReturnValue;

    }

#endif

#endif

#ifdef DEBUG

#ifdef PARANOID

    assert( VolumeRecord->Compatibility_Volume );
    assert( VolumeRecord->Partition_Count == 1 );

#else

    if ( ( ! VolumeRecord->Compatibility_Volume ) ||
         ( VolumeRecord->Partition_Count != 1)
       )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      API_EXIT("Get_Installable_Volume")

      return ReturnValue;

    }

#endif

#endif

    /* Now transfer the data to ReturnValue. */
    ReturnValue.Volume_Size = VolumeRecord->Volume_Size;
    ReturnValue.Partition_Count = 1;
    ReturnValue.Drive_Letter_Conflict = VolumeRecord->Drive_Letter_Conflict;
    ReturnValue.Compatibility_Volume = VolumeRecord->Compatibility_Volume;
    ReturnValue.Bootable = TRUE;
    ReturnValue.Drive_Letter_Preference = VolumeRecord->Drive_Letter_Preference;
    strncpy(ReturnValue.Volume_Name, VolumeRecord->Volume_Name, VOLUME_NAME_SIZE);
    strncpy(ReturnValue.File_System_Name, VolumeRecord->File_System_Name, FILESYSTEM_NAME_SIZE);

    /* Indicate success. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }
  else
    *Error_Code = LVM_ENGINE_VOLUME_NOT_FOUND;

  API_EXIT("Get_Installable_Volume")

  return ReturnValue;

}


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
CARDINAL32 Get_Available_Drive_Letters ( CARDINAL32 * Error_Code )
{

  API_ENTRY("Get_Available_Drive_Letters")

  /* Has the Volume Manager been initialized? */
  if ( Volumes == NULL )
  {
    /* We have an error!  This module has not been initialized yet! */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Get_Available_Drive_Letters")

    return 0;

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Get_Available_Drive_Letters")

  /* Return Available_Drive_Letters. */
  return Available_Drive_Letters;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Get_Volume_Options(Volume_Data * VolumeRecord, CARDINAL32 * Error_Code)
{

  Partition_Data *             PartitionRecord;             /* Used to examine the first partition of a volume. */
  CARDINAL32                   ReturnValue = 0;             /* Used to hold the bitmap we will be returning to the caller. */
  BOOLEAN                      Corrupt_Drive_Found = FALSE; /* Used when checking the partitions in a volume to see if any of them lie on a corrupt drive. */
  Drive_Letter_Count_Record    Drive_Letter_Count;          /* Used to count the number of volumes with the same drive letter preference. */
  Plugin_Function_Table_V1 *   Aggregate_Function_Table;    /* Used to access the function table associated with the aggregate. */
  CARDINAL32                   Not_Needed;

  FUNCTION_ENTRY("Get_Volume_Options")

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* If this volume represents a non-LVM controlled device, then we can do nothing with it! */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    FUNCTION_EXIT("Get_Volume_Options")

    /* We can do nothing with this Volume. */
    return ReturnValue;

  }

  /* We can always delete a volume. */
  ReturnValue += DELETE_VOLUME;

  /* If the volume can not be changed, there is nothing else we can do with it. */
  if ( ! VolumeRecord->Can_Be_Altered )
  {

    FUNCTION_EXIT("Get_Volume_Options")

    return ReturnValue;

  }

  /* Since we have a volume, lets see what kind of partition is associated with it.  It will either be an aggregate or a partition. */
  PartitionRecord = VolumeRecord->Partition;

  /* If one or more of the partitions that comprise this volume lie on a corrupt drive, then there is nothing else we can do with this Volume. */

  /* Is this a Compatibility Volume? */
  if ( VolumeRecord->Compatibility_Volume )
  {

    /* Check the drive that the volume's partition resides on.   If it is corrupt, then return. */
    if ( DriveArray[PartitionRecord->Drive_Index].Corrupt )
    {

      FUNCTION_EXIT("Get_Volume_Options")

      return ReturnValue;

    }

  }
  else
  {

    /* We have an LVM Volume.  We must check all of the partitions that comprise it. */
    Check_For_Corrupt_Drive( (ADDRESS) PartitionRecord, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &Corrupt_Drive_Found, Error_Code );

    if ( ( *Error_Code != DLIST_SUCCESS ) && ( *Error_Code != DLIST_SEARCH_COMPLETE ) )
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    else
      *Error_Code = LVM_ENGINE_NO_ERROR;

    /* Was a corrupt drive found? */
    if ( Corrupt_Drive_Found || ( *Error_Code != LVM_ENGINE_NO_ERROR ) )
    {

      FUNCTION_EXIT("Get_Volume_Options")

      return ReturnValue;

    }

  }

  /* Since the Volume was not corrupt, we can at least set its name. */
  ReturnValue += CAN_SET_NAME;

  /* We can hide a volume if it is not already hidden. */
  if ( ( ( VolumeRecord->Drive_Letter_Preference >= 'C' ) &&
         ( VolumeRecord->Drive_Letter_Preference <= 'Z' )
       ) ||
       ( VolumeRecord->Drive_Letter_Preference == '*' )
     )
  {

    /* The volume is not currently hidden.  This means that we can hide it. */
    ReturnValue += HIDE_VOLUME;

  }

  /* What kind of a volume do we have here? */
  if ( ( ! VolumeRecord->Compatibility_Volume ) &&
       ( VolumeRecord->Filesystem_Is_Expandable ) &&
       ( VolumeRecord->Device_Type != LVM_PRM ) &&
       ( VolumeRecord->Drive_Letter_Preference >= 'C' ) &&                                       /* Test the drive letter preference to see if the volume is hidden or not. */
       ( VolumeRecord->Drive_Letter_Preference <= 'Z' ) &&                                       /* We do NOT want to allow the expansion of hidden volumes as the ExtendFS utility will not operate on them. */
       ( ( ( VolumeRecord->Initial_Drive_Letter >= 'C' ) &&                                          /* Test the initial drive letter to see if the volume has an active drive letter assignment. */
           ( VolumeRecord->Initial_Drive_Letter <= 'Z' ) ) ||
         VolumeRecord->New_Volume
       )
     )
  {

    /* We must find out if we can expand this volume.  Let's ask the volume's aggregate. */
    Aggregate_Function_Table = PartitionRecord->Feature_Data->Function_Table;
    if (Aggregate_Function_Table->Can_Expand(PartitionRecord, &Not_Needed, Error_Code) )
    {

      ReturnValue += EXPAND_VOLUME;

    }
#ifdef DEBUG
    else
    {

#ifdef PARANOID

      assert ( *Error_Code == LVM_ENGINE_NO_ERROR );

#else

      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        FUNCTION_EXIT("Get_Volume_Options")

        return 0;

      }

#endif

    }

#endif


  }
  else
  {

    /* If this is a compatibility volume, and if its partition is a primary partition on the first drive, and if the partition resides below the 1024
       cylinder limit (if it applies), then we can set this volume as being Startable. */

    /* Is this partition a primary partition on the first disk drive which is not marked Startable? */
    if ( VolumeRecord->Compatibility_Volume &&
         PartitionRecord->Primary_Partition &&
         ( PartitionRecord->Drive_Index == 0 ) &&
         ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) == 0 ) &&
         ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
           ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
       )
       )
    {

      /* This volume can be set startable. */
      ReturnValue += SET_STARTABLE;

    }

    /* In order to be installable, a volume must be a compatibility volume with its partition being
       below the 1024 cylinder limit (if it applies).  Furthermore, if Boot Manager is NOT installed,
       then the partition must be a primary partition on the first disk drive.                           */

    /* Is Boot Manager installed and active? */
    if ( ( Boot_Manager_Handle != NULL ) && Boot_Manager_Active && VolumeRecord->Compatibility_Volume )
    {

      /* Boot Manager is installed!  We don't need to worry about whether or not the partition
         is a primary partition, and we don't need to worry about what disk it resides on.  Is
         the partition below the 1024 cylinder limit?  Does the 1024 cylinder limit even apply? */
      if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
           ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
         )
      {

        /* We can set this volume installable if:
               it is install time
               it is large enough to install to
               it is not hidden
               it is not already marked installable. */
        if ( ( Install_Volume_Handle != VolumeRecord->Volume_Handle ) &&
             ( Min_Install_Size > 0 ) &&
             ( PartitionRecord->Partition_Size > Min_Install_Size ) &&
             ( VolumeRecord->Drive_Letter_Preference >= 'C' ) &&
             ( VolumeRecord->Drive_Letter_Preference <= 'Z' )
           )
        {

          /* Is there a drive letter conflict on this volume which could cause a problem? */
          if ( VolumeRecord->Drive_Letter_Conflict != 0 )
          {

            /* Does the conflict still exist?  Lets count how many volumes want this drive letter. */

            /* Set up to do the count. */
            Drive_Letter_Count.Count = 0;
            Drive_Letter_Count.Drive_Letter = VolumeRecord->Drive_Letter_Preference;

            /* Now find out how many volumes have this drive letter preference. */
            ForEachItem( Volumes, &Count_Drive_Letter_Claims, &Drive_Letter_Count, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

            assert( *Error_Code == DLIST_SUCCESS );

#else

            if ( *Error_Code != DLIST_SUCCESS )
            {

              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

              FUNCTION_EXIT("Get_Volume_Options")

              return 0;

            }

#endif

#endif

            /* If there was only 1 claim for this drive letter, then that claim must have been this volume.  We can
               therefore set this volume installable.                                                                */
            if ( Drive_Letter_Count.Count == 1 )
              ReturnValue += SET_VOLUME_INSTALLABLE;

          }
          else
          {

            /* Since there was no drive letter conflict to worry about, we can set this volume installable. */
            ReturnValue += SET_VOLUME_INSTALLABLE;

          }

        }

        /* While we are here, lets see if this volume is already on the Boot Manager Menu.  If it
           is, then we can set the REMOVE_FROM_BOOT_MANAGER_MENU bit.  If it is not on the
           Boot Manager Menu, then we can set the ADD_TO_BOOT_MANAGER_MENU flag.                   */
        if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
          ReturnValue += REMOVE_FROM_BOOT_MANAGER_MENU;
        else
          ReturnValue += ADD_TO_BOOT_MANAGER_MENU;

      }

    }
    else
    {

      /* Boot Manager is not installed.  This must be a primary partition on the first disk drive in order to be installable, and
         the drive letter preference must be 'C'.                                                                                  */
      if ( PartitionRecord->Primary_Partition && ( PartitionRecord->Drive_Index == 0 ) && ( VolumeRecord->Drive_Letter_Preference == 'C' ) )
      {

        /* Does the 1024 cylinder limit apply?  If so, is the partition below it?  Are we at install time? */
        if ( ( Min_Install_Size > 0 ) &&
             ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
               ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
             )
           )
        {

          /* Is there a drive letter conflict on this volume which could cause a problem? */
          if ( VolumeRecord->Drive_Letter_Conflict != 0 )
          {

            /* Does the conflict still exist?  Lets count how many volumes want this drive letter. */

            /* Set up to do the count. */
            Drive_Letter_Count.Count = 0;
            Drive_Letter_Count.Drive_Letter = VolumeRecord->Drive_Letter_Preference;

            /* Now find out how many volumes have this drive letter preference. */
            ForEachItem( Volumes, &Count_Drive_Letter_Claims, &Drive_Letter_Count, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

            assert( *Error_Code == DLIST_SUCCESS );

#else

            if ( *Error_Code != DLIST_SUCCESS )
            {

              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

              FUNCTION_EXIT("Get_Volume_Options")

              return 0;

            }

#endif

#endif

            /* If there was only 1 claim for this drive letter, then that claim must have been this volume.  We can
               therefore set this volume installable.                                                                */
            if ( Drive_Letter_Count.Count == 1 )
              ReturnValue += SET_VOLUME_INSTALLABLE;

          }
          else
          {

            /* Since there was no drive letter conflict to worry about, we can set this volume installable. */
            ReturnValue += SET_VOLUME_INSTALLABLE;

          }

        }

      }

    }

  }

  /* We can assign a drive letter to a volume regardless of what type of volume it is. */
  ReturnValue += ASSIGN_DRIVE_LETTER;

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Get_Volume_Options")

  return ReturnValue;

}


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
/*********************************************************************/
CARDINAL32 _System Convert_Volumes_To_V1 ( BOOLEAN *    Hidden_Volume_Conversion_Failure,
                                           CARDINAL32 * Error_Code )
{

  CARDINAL32      Unconvertable_Volumes = 0;

  FUNCTION_ENTRY("Convert_Volumes_To_V1")

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* We must now traverse the list of volumes and examine each LVM Volume.  In order for an LVM Volume to be converted
     to version 1 format, the volume must employ drive linking, BBR, and pass thru.  No other features are allowed, and
     all three of these must be present.  Any LVM volume not meeting these guidelines can not be converted!.             */
  ForEachItem(Volumes, &Find_And_Convert_LVM_Volumes, &Unconvertable_Volumes, TRUE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We had an error!  ForEachItem should never fail when walking the Volumes list! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }
  else
  {

    /* Were there any unconvertable volumes? */
    if ( Unconvertable_Volumes != 0 )
      *Error_Code = LVM_ENGINE_VOLUME_NOT_CONVERTED;
    else
      *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  /* Were there any hidden volumes that could not be converted? */
  if ( ( Unconvertable_Volumes & HIDDEN_VOLUME_FAILURE_FLAG ) != 0 )
  {

    *Hidden_Volume_Conversion_Failure = TRUE;
    Unconvertable_Volumes &=  ~HIDDEN_VOLUME_FAILURE_FLAG;

  }

  FUNCTION_EXIT("Convert_Volumes_To_V1")

  return Unconvertable_Volumes;

}


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
                                                       CARDINAL32 *                       _Seg16 Error_Code )
{

  Volume_Control_Array   Data;

  Data = Get_Volume_Control_Data(Error_Code);

  *Volume_Control_Data = Data.Volume_Control_Data;
  *Count = Data.Count;

  return;

}


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
                                                    )
{

  Volume_Information_Record   Data;

  Data = Get_Volume_Information( (ADDRESS) Volume_Handle, Error_Code );

  *Volume_Information = Data;

  return;

}


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
/*   Notes:  This function provides limited compatibility for        */
/*           programs written to use the LVM Version 1 interface.    */
/*           Specifically, this function will only allow the         */
/*           creation of compatibility volumes.  Any attempt to      */
/*           create an LVM volume will result in an error code being */
/*           returned.                                               */
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
                                           )
{

  if ( Create_LVM_Volume )
  {

    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;
    return;

  }

  Create_Volume2(Name, FALSE, Bootable, Drive_Letter_Preference, 0, NULL, Partition_Count, (ADDRESS *) Partition_Handles, Error_Code);

  return;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _Far16 _Pascal _loadds  CREATE_VOLUME216( char   * _Seg16                            Name,
                                               BOOLEAN                                    Create_LVM_Volume,
                                               BOOLEAN                                    Bootable,
                                               char                                       Drive_Letter_Preference,
                                               CARDINAL32                                 Feature_Count,
                                               LVM_Feature_Specification_Record * _Seg16  FeaturesToUse,
                                               CARDINAL32                                 Partition_Count,
                                               CARDINAL32 * _Seg16                        Partition_Handles,
                                               CARDINAL32 * _Seg16                        Error_Code
                                             )
{

  Create_Volume2( Name, Create_LVM_Volume, Bootable, Drive_Letter_Preference, Feature_Count, FeaturesToUse, Partition_Count, (ADDRESS *) Partition_Handles, Error_Code );

  return;

}


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
                                           )
{

  Delete_Volume( (ADDRESS) Volume_Handle, Error_Code );

  return;

}


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
                                         )
{

  Hide_Volume( (ADDRESS) Volume_Handle, Error_Code );

  return;

}


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
/*                                        or volume which is to be   */
/*                                        added to the volume being  */
/*                                        expanded.                  */
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
void _Far16 _Pascal _loadds EXPAND_VOLUME16 ( CARDINAL32            Volume_Handle,
                                              CARDINAL32            Partition_Count,
                                              CARDINAL32 * _Seg16   Partition_Handles,
                                              CARDINAL32 * _Seg16   Error_Code
                                            )
{

  Expand_Volume( (ADDRESS) Volume_Handle, Partition_Count, (ADDRESS *) Partition_Handles, Error_Code );

  return;

}


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
                                                 )
{

  Assign_Drive_Letter( (ADDRESS) Volume_Handle, New_Drive_Preference, Error_Code );

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Installable16                                */
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
void _Far16 _Pascal _loadds SET_INSTALLABLE16 ( CARDINAL32          Volume_Handle,
                                                CARDINAL32 * _Seg16 Error_Code
                                              )
{

  Set_Installable( (ADDRESS) Volume_Handle, Error_Code );

  return;

}


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
                                                     )
{
  Volume_Information_Record   Data;

  Data = Get_Installable_Volume( Error_Code );

  *Volume_Information = Data;

  return;

}


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
CARDINAL32 _Far16 _Pascal _loadds GET_AVAILABLE_DRIVE_LETTERS16 ( CARDINAL32 * _Seg16 Error_Code )
{

  return Get_Available_Drive_Letters( Error_Code );

}


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
CARDINAL32 _Far16 _Pascal _loadds GET_RESERVED_DRIVE_LETTERS16 ( CARDINAL32 *  _Seg16 Error_Code )
{

  return Get_Reserved_Drive_Letters( Error_Code );

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Convert_Fake_Volumes_On_PRM_To_Real_Volumes      */
/*                                                                   */
/*   Descriptive Name:  Converts any "fake" volumes created to       */
/*                      represent partitions on a PRM into real      */
/*                      compatibility volumes.  NOTE:  This will     */
/*                      not work for "fake" volumes created to       */
/*                      represent blank media or big floppy          */
/*                      formatted media!                             */
/*                                                                   */
/*   Input: CARDINAL32 DriveIndex - The index into the drive array   */
/*                                  for the entry representing the   */
/*                                  PRM whose "fake" volumes are to  */
/*                                  be converted to real volumes.    */
/*          CARDINAL32 * Error_Code - The address of a variable in   */
/*                                    which to place an error return */
/*                                    code.                          */
/*                                                                   */
/*   Output:  *Error_Code is set to LVM_ENGINE_NO_ERROR is this      */
/*            function completes successfully.  Any other value is an*/
/*            error code being returned due to an error being        */
/*            encountered.                                           */
/*                                                                   */
/*   Error Handling:  The only errors which can occur would be due   */
/*                    to corrupted data structures.  Thus, any error */
/*                    encountered by this function is an LVM Engine  */
/*                    internal error!                                */
/*                                                                   */
/*   Side Effects:  The status of the "fake" volumes on the          */
/*                  designated PRMs is altered.  If a commit         */
/*                  operation is done at some point after this       */
/*                  function completes, then the DLAT tables on the  */
/*                  PRM will be updated to reflect the new status.   */
/*                                                                   */
/*   Notes:  This function should not be used with "fake" volumes    */
/*           that were created to represent the entire PRM, such as  */
/*           those created to represent big floppy formatted media,  */
/*           or blank media (no partition table, and not big floppy  */
/*           formatted).                                             */
/*                                                                   */
/*********************************************************************/
void Convert_Fake_Volumes_On_PRM_To_Real_Volumes( CARDINAL32 DriveIndex, CARDINAL32 * Error_Code )
{

  FUNCTION_ENTRY("Convert_Fake_Volumes_On_PRM_To_Real_Volumes")

#ifdef DEBUG

  /* Is the DriveIndex in range? */
  if ( DriveIndex >= DriveCount )
  {

    FUNCTION_EXIT("Convert_Fake_Volumes_On_PRM_To_Real_Volumes")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

  /* Is the specified drive a PRM? */
  if ( ! DriveArray[DriveIndex].Is_PRM )
  {

    FUNCTION_EXIT("Convert_Fake_Volumes_On_PRM_To_Real_Volumes")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

  /* Is the PRM in Big Floppy Mode? */
  if ( DriveArray[DriveIndex].Is_Big_Floppy )
  {

    FUNCTION_EXIT("Convert_Fake_Volumes_On_PRM_To_Real_Volumes")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

#endif

  /* Lets scan the list of partitions and, for each partition with a "fake" volume, add that volume to the delete list.
     This is necessary to prevent problems with OS2LVM and OS2DASD.  When they originally saw the partitions on this
     PRM, they loaded the DLAT information for each partition.  This DLAT information did NOT include any volume information,
     so "fake" volume information was created.  Now that these "fake" volumes are real, we need to flush out the "fake"
     volume information in OS2LVM and OS2DASD so that they don't get confused when they see these same partitions with real
     volume information ( which won't match the "fake" volume information ).                                                      */
  ForEachItem(DriveArray[DriveIndex].Partitions, &Add_Fake_Volumes_To_Deleted_Volumes_List, NULL, TRUE, Error_Code);

  if ( *Error_Code != DLIST_SUCCESS )
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;    /* Convert to a DLIST Error to an LVM ENGINE error. */
  else
  {

    /* Indicate that there were no errors. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

    /* Indicate that there are no more fake volumes in use on this PRM. */
    DriveArray[DriveIndex].Fake_Volumes_In_Use = FALSE;

  }

  FUNCTION_EXIT("Convert_Fake_Volumes_On_PRM_To_Real_Volumes")

  return;

}




/*--------------------------------------------------
 * Private functions available.
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Transfer_Partition_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  Partition_Information_Array * ReturnValue = ( Partition_Information_Array * ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *              PartitionRecord = (Partition_Data *) Object;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;


  FUNCTION_ENTRY("Transfer_Partition_Data")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Transfer_Partition_Data")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Do we have an aggregate or a partition? */
    if ( PartitionRecord->Drive_Index != (CARDINAL32) -1L )
    {

      /* We have a partition.  Copy the necessary information to the Partition_Array. */
      Set_Partition_Information_Record( &( ReturnValue->Partition_Array[ReturnValue->Count] ), PartitionRecord);

      ReturnValue->Count++;

    }
    else
    {

      /* Now we must find the partitions list for the aggregate. */
      Current_Context = PartitionRecord->Feature_Data;

      /* The only feature's which will have a partitions list are Aggregators.  There can be only
         one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
      while ( Current_Context->Partitions == NULL )
      {

        /* Get the next context in the feature context chain for this aggregate. */
        Current_Context = Current_Context->Old_Context;

      }

      /* We have an aggregate.  We must process the partitions list of the aggregate. */
      ForEachItem( Current_Context->Partitions, &Transfer_Partition_Data, Parameters, TRUE, Error);

      if ( *Error != DLIST_SUCCESS )
      {

        FUNCTION_EXIT("Transfer_Partition_Data")

        return;

      }

    }


  }
#ifdef DEBUG
  else
  {

    /* We have a corrupt list!  This function should only be used with the Partitions list for a volume.  The
       partitions list for a volume only contains entries which are for partitions, so if we get anything else,
       then the list has been corrupted!                                                                        */


#ifdef PARANOID

    assert(0);

#endif


      *Error = DLIST_CORRUPTED;

      FUNCTION_EXIT("Transfer_Partition_Data")

      return;

  }
#endif

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Transfer_Partition_Data")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Potential_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  DLIST                           Potential_Volumes = ( DLIST ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *                PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable to point to the LVM Signature Sector.  We will need it to manipulate LVM Signature Sectors. */
  LVM_Signature_Sector *          Signature_Sector;

  /* Declare a local variable to use when searching the Potential_Volumes list for an existing entry. */
  Potential_Volume_Search_Data    Search_Data;

  /* Declare a local variable to use when creating an entry in the Potential_Volumes list. */
  Potential_Volume_Data           New_Potential_Volume;

  /* Declare a local variable to access the Pass Through layer's function table. */
  Plugin_Function_Table_V1 *      PT_Function_Table = (Plugin_Function_Table_V1 *) PassThru_Function_Table;

  FUNCTION_ENTRY("Find_Potential_Volumes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Find_Potential_Volumes")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    LOG_EVENT1("Examining the current partition.","Starting Sector of Partition", PartitionRecord->Starting_Sector )

    /* We have a partition.  Assume that this partition does not have an LVM Signature Sector. */
    PartitionRecord->Signature_Sector = NULL;

    /* Does this partition contain an LVM Signature Sector? */
    ReadSectors(PartitionRecord->Drive_Index + 1, PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size - 1, 1, &Buffer, Error);

    if ( ( *Error == DISKIO_NO_ERROR ) && Valid_Signature_Sector( PartitionRecord, (LVM_Signature_Sector *) &Buffer ) )
    {

      LOG_EVENT("A valid LVM Signature Sector was found for this partition.")

      /* Allocate memory to hold the LVM Signature Sector. */
      PartitionRecord->Signature_Sector = (LVM_Signature_Sector *) malloc( BYTES_PER_SECTOR );

      if ( PartitionRecord->Signature_Sector == NULL )
      {

        LOG_ERROR("Unable to allocate memory to hold the LVM Signature Sector of the current partition!")

        *Error = DLIST_OUT_OF_MEMORY;

        FUNCTION_EXIT("Find_Potential_Volumes")

        return;

      }

      /* Now save the LVM Signature Sector. */
      memcpy(PartitionRecord->Signature_Sector, &Buffer, BYTES_PER_SECTOR);

      /* Establish access to the LVM Signature Sector. */
      Signature_Sector = PartitionRecord->Signature_Sector;

      /* Now update the DLA_Table_Entry values of the partition. */
      if ( ( PartitionRecord->DLA_Table_Entry.Volume_Serial_Number != Signature_Sector->Volume_Serial_Number ) ||
           ( PartitionRecord->DLA_Table_Entry.Partition_Serial_Number != Signature_Sector->Partition_Serial_Number ) ||
           ( PartitionRecord->DLA_Table_Entry.Drive_Letter != Signature_Sector->Drive_Letter ) ||
           ( PartitionRecord->Partition_Table_Entry.Format_Indicator != LVM_PARTITION_INDICATOR ) ||
           ( strncmp(PartitionRecord->DLA_Table_Entry.Partition_Name, Signature_Sector->Partition_Name, PARTITION_NAME_SIZE) != 0 ) ||
           ( strncmp(PartitionRecord->DLA_Table_Entry.Volume_Name, Signature_Sector->Volume_Name, VOLUME_NAME_SIZE) != 0 )
         )
      {

        LOG_EVENT("The DLA Table Entries for the partition did not match the entries in the LVM Signature Sector.\n     Update the DLA Table Entries from the LVM Signature Sector.")

        /* Update the DLA Table Entry for this partition. */
        PartitionRecord->DLA_Table_Entry.Volume_Serial_Number = Signature_Sector->Volume_Serial_Number;
        PartitionRecord->DLA_Table_Entry.Partition_Serial_Number = Signature_Sector->Partition_Serial_Number;
        PartitionRecord->DLA_Table_Entry.Drive_Letter = Signature_Sector->Drive_Letter;
        strncpy(PartitionRecord->DLA_Table_Entry.Partition_Name, Signature_Sector->Partition_Name, PARTITION_NAME_SIZE);
        strncpy(PartitionRecord->DLA_Table_Entry.Volume_Name, Signature_Sector->Volume_Name, VOLUME_NAME_SIZE);

        /* Mark the drive so that the change will make it to disk. */
        DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      }

      /* Now update the Partition's usable size.  When the Partition Manager discovered this partition, it defaulted
         usable size to partition size.  This is fine for compatibility volumes, but not for LVM volumes because LVM
         reserves sectors at the end of each partition in an LVM volume.  To keep the filesystems from trying to place
         data over the LVM reserved area, we report the partition as being smaller than it actually is.  The value we
         report to the filesystems is based upon the partition's usable size, which is defined to be the size of the
         partition less the LVM reserved area.                                                                           */
      PartitionRecord->Usable_Size = Signature_Sector->Partition_Size_To_Report_To_User;

      /* Now update the Partition Table Entry to ensure that this partition is an LVM Partition. */
      PartitionRecord->Partition_Table_Entry.Format_Indicator = LVM_PARTITION_INDICATOR;

      /* Now allocate space for Feature Data. */
      PartitionRecord->Feature_Data = (Feature_Context_Data *) malloc ( sizeof( Feature_Context_Data ) );

      /* Did we get the memory? */
      if ( PartitionRecord->Feature_Data == NULL )
      {

        FUNCTION_EXIT("Find_Potential_Volumes")

        *Error = DLIST_OUT_OF_MEMORY;
        return;

      }

      /* Now initialize the feature data. */
      PartitionRecord->Feature_Data->Feature_ID = PT_Function_Table->Feature_ID;
      PartitionRecord->Feature_Data->Function_Table = PassThru_Function_Table;
      PartitionRecord->Feature_Data->Data = NULL;
      PartitionRecord->Feature_Data->Partitions = NULL;
      PartitionRecord->Feature_Data->Old_Context = NULL;

    }


    /* We have a partition.  Is it eligible to be a volume?  If it has the LVM format indicator and an
       LVM Signature Sector, or if it does not have the LVM Format Indicator but does have a volume
       serial number, then it is eligible to be a volume.                                               */
    if ( ( ( PartitionRecord->Partition_Table_Entry.Format_Indicator == LVM_PARTITION_INDICATOR ) &&
           ( PartitionRecord->Signature_Sector != NULL )
         ) ||
         ( ( PartitionRecord->DLA_Table_Entry.Volume_Serial_Number != 0 ) &&
           ( PartitionRecord->Partition_Table_Entry.Format_Indicator != LVM_PARTITION_INDICATOR )
         )
       )
    {

      LOG_EVENT("The current partition may be part of a volume!")

      /* This partition may be a volume or may be a part of a volume.  We must see if we have an entry in the Potential_Volumes
         list for the volume that this partition claims to be a part of.  If we do, then we will add this partition to the partitions
         list for the volume.  If we don't, then we must create an entry in the Potential_Volumes list for the volume this partition
         claims to be a part of, and then add this partition to the partitions list for the volume.                                     */
      Search_Data.PartitionRecord = PartitionRecord;
      Search_Data.Found = FALSE;
      ForEachItem(Potential_Volumes, Find_Existing_Potential_Volume, &Search_Data, TRUE, Error);

      if ( *Error != DLIST_SUCCESS )
      {

        FUNCTION_EXIT("Find_Potential_Volumes")

        /* We have a problem!  Abort. */
        return;

      }

      if ( !Search_Data.Found )
      {

        LOG_EVENT("The current partition belongs to a volume we have not seen yet.  Adding the new volume to our list of potential volumes.")

        /* We must create an entry in the Potential Volumes list for the volume claimed by the current partition.  */
        New_Potential_Volume.Partition_List = CreateList();
        if (New_Potential_Volume.Partition_List == NULL )
        {

          FUNCTION_EXIT("Find_Potential_Volumes")

          /* We are out of memory! */
          *Error = DLIST_OUT_OF_MEMORY;
          return;

        }

        /* Now finish initializing the new entry. */
        New_Potential_Volume.Volume_Serial_Number = PartitionRecord->DLA_Table_Entry.Volume_Serial_Number;
        New_Potential_Volume.Boot_Drive_Serial_Number = DriveArray[PartitionRecord->Drive_Index].Boot_Drive_Serial_Number;

        /* Now we must add this partition to the partitions list for the volume. */
        InsertObject( New_Potential_Volume.Partition_List,
                      sizeof(Partition_Data),
                      PartitionRecord,
                      PARTITION_DATA_TAG,
                      NULL,
                      AppendToList,
                      FALSE,
                      Error);

        if ( *Error != DLIST_SUCCESS )
        {

          LOG_ERROR1("InsertObject failed.","Error code", *Error)

          FUNCTION_EXIT("Find_Potential_Volumes")

          /* We have a problem!  Abort. */
          return;

        }

        /* Now we must add the entry to the Potential Volumes list. */
        InsertItem(Potential_Volumes,
                   sizeof(Potential_Volume_Data),
                   &New_Potential_Volume,
                   POTENTIAL_VOLUME_DATA_TAG,
                   NULL,
                   AppendToList,
                   FALSE,
                   Error);

        if ( *Error != DLIST_SUCCESS )
        {

          LOG_ERROR1("InsertItem failed!","Error code", *Error)

          FUNCTION_EXIT("Find_Potential_Volumes")

          /* We have a problem!  Abort. */
          return;

        }

      }

    }

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Find_Potential_Volumes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Destroy_Embedded_Lists(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *              Volume_Information = (Volume_Data *) Object;
  Partition_Data *           Aggregate;
  Plugin_Function_Table_V1 * Aggregate_Function_Table;

  FUNCTION_ENTRY("Destroy_Embedded_Lists")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Destroy_Embedded_Lists")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* If this is an LVM volume, we need to delete the aggregate associated with this volume. */
  if ( !Volume_Information->Compatibility_Volume )
  {

    Aggregate = Volume_Information->Partition;
    Aggregate_Function_Table = Aggregate->Feature_Data->Function_Table;
    Aggregate_Function_Table->Delete(Aggregate, FALSE, Error);

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Destroy_Embedded_Lists")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static INTEGER32 _System Sort_By_Drive_Letter_Preference( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error)
{

  /* Declare local variables so that we can access the Volume_Data objects without having to typecast each time. */
  Volume_Data *    Volume1 = (Volume_Data *) Object1;
  Volume_Data *    Volume2 = (Volume_Data *) Object2;

  FUNCTION_ENTRY("Sort_By_Drive_Letter_Preference")

#ifdef DEBUG

  /* Are Object1 and Object2 what we think it should be? */
  if ( ( Object1Tag != VOLUME_DATA_TAG ) || ( Object2Tag != VOLUME_DATA_TAG ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Sort_By_Drive_Letter_Preference")

    return 0;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Is Volume1 < Volume2? */
  if ( Volume1->Drive_Letter_Preference < Volume2->Drive_Letter_Preference )
  {

    FUNCTION_EXIT("Sort_By_Drive_Letter_Preference")

    return -1;

  }

  /* Is Volume1 > Volume2? */
  if ( Volume1->Drive_Letter_Preference > Volume2->Drive_Letter_Preference )
  {

    FUNCTION_EXIT("Sort_By_Drive_Letter_Preference")

    return 1;

  }

  /* They must be equal! */

  FUNCTION_EXIT("Sort_By_Drive_Letter_Preference")

  return 0;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Drive_Letter_Conflicts(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    Volume1 = (Volume_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecase each time. */
  Volume_Data **   Volume2 = (Volume_Data **) Parameters;

  /* Declare a variable to be used in setting the Available_Drive_Letters variable. */
  CARDINAL32       Drive_Letter_Mask = 0x1;

  /* Declare a variable to hold the drive letter we are dealing with. */
  char             Drive_Letter;


  FUNCTION_ENTRY("Find_Drive_Letter_Conflicts")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Find_Drive_Letter_Conflicts")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Initialize Volume1 to no drive letter conflict. */
  Volume1->Drive_Letter_Conflict = 0;

  /* Now lets begin our drive letter conflict checking. */

  /* Is Volume1 hidden or set to '*'? */
  if ( ( Volume1->Drive_Letter_Preference == '*' ) ||
       ( ( Volume1->Drive_Letter_Preference < 'C' ) || ( Volume1->Drive_Letter_Preference > 'Z' ) )
     )
  {

    if ( Volume1->Drive_Letter_Preference != '*' )
    {

      /* This volume is hidden. */
      Volume1->Drive_Letter_Conflict = 4;

    }

    FUNCTION_EXIT("Find_Drive_Letter_Conflicts")

    return;

  }

  /* Is Volume2 == NULL.  If so, then we have not seen a volume yet. */
  if ( *Volume2 == NULL )
  {

    /* This is the first volume we have seen.  Our Parameter is the address of a variable of type Volume_Data *.  We will
       save a pointer to this Volume_Data object in our Parameter so that, when we get invoked again, we will be able to view it. */
    *Volume2 = Volume1;

    /* Save the drive letter we are dealing with. */
    Drive_Letter = Volume1->Drive_Letter_Preference;

  }
  else
  {

    /* Do Volume1 and Volume2 claim the same drive letter? */
    if ( Volume1->Drive_Letter_Preference == (*Volume2)->Drive_Letter_Preference )
    {

      /* Volume1 and Volume2 claim the same drive letter. */

      /* Save the drive letter. */
      Drive_Letter = Volume1->Drive_Letter_Preference;

      /* We must decide who will get/has gotten the drive letter. */

      /* If we are in Merlin Mode, then Current_Drive_Letter can NOT be used to find out who got the drive letter! */
      if ( ! Merlin_Mode )
      {

        /* Since we are NOT in Merlin mode, we can tell who got the drive letter and who did not. */

        /* Did Volume1 get the drive letter? */
        if ( Volume1->Current_Drive_Letter == Volume1->Drive_Letter_Preference )
        {

          /* Volume1 got the drive letter.  Mark him as having the drive letter. */
          Volume1->Drive_Letter_Conflict = 1;

          /* Indicate that Volume two did NOT get the drive letter. */
          (*Volume2)->Drive_Letter_Conflict = 2;

        }
        else
        {

          /* Did Volume2 get the drive letter? */
          if ( (*Volume2)->Current_Drive_Letter == (*Volume2)->Drive_Letter_Preference )
          {

            /* Volume2 got the drive letter.  Mark him as having the drive letter. */
            (*Volume2)->Drive_Letter_Conflict = 1;

            /* Indicate that Volume1 did NOT get the drive letter. */
            Volume1->Drive_Letter_Conflict = 2;

          }
          else
          {

            /* Neither Volume1 nor Volume2 got the drive letter.  There must be another volume that has this drive letter! */

            /* Indicate that Volume1 did NOT get the drive letter. */
            Volume1->Drive_Letter_Conflict = 2;

            /* Indicate that Volume two did NOT get the drive letter. */
            (*Volume2)->Drive_Letter_Conflict = 2;

          }

        }

      }
      else
      {

        /* Since we are in Merlin Mode, we must make a guess as to who will get what drive letter. */

        /* Is either Volume1 or Volume2 a foreign volume? */
        if ( Volume1->Foreign_Volume )
        {

          /* Volume1 is a foreign volume.  It doesn't matter if Volume2 is a foreign volume or not, as Volume2 was encountered
             prior to Volume1 and thus gets the drive letter in either case.  Set Volume1's Drive_Letter_Conflict field.         */
          Volume1->Drive_Letter_Conflict = 2;

          /* Indicate that Volume2 had a conflict, but "won". */
          (*Volume2)->Drive_Letter_Conflict = 1;

        }
        else
        {

          /* Is Volume2 a foreign volume? */
          if ( (*Volume2)->Foreign_Volume )
          {

            /* Volume2 is a foreign volume.  It loses out to Volume1, and Volume1 gets the drive letter. */
            (*Volume2)->Drive_Letter_Conflict = 2;

            /* Indicate that Volume1 had a conflict, but "won" the right to use the drive letter. */
            Volume1->Drive_Letter_Conflict = 1;

          }
          else
          {

            /* Since neither Volume1 nor Volume2 is a foreign volume, then Volume2 wins as it was encountered first. */
            Volume1->Drive_Letter_Conflict = 2;

            /* Indicate that Volume2 had a conflict but got the drive letter anyway. */
            (*Volume2)->Drive_Letter_Conflict = 1;

          }

        }

      }

    }
    else
    {

      /* Since they don't claim the same drive letter, they don't conflict.  Save Volume1 so we can reference it later. */
      *Volume2 = Volume1;

      /* Save the drive letter claimed by Volume1. */
      Drive_Letter = Volume1->Drive_Letter_Preference;

    }

  }

  /* Now we must mark the drive letter as not being available. */
  Drive_Letter_Mask = Drive_Letter_Mask << ( Drive_Letter - 'A' );

  Available_Drive_Letters = Available_Drive_Letters & ( ~Drive_Letter_Mask );

  FUNCTION_EXIT("Find_Drive_Letter_Conflicts")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static Volume_Data * Create_Compatibility_Volume( Partition_Data * PartitionRecord, CARDINAL32 * Error )
{

  Volume_Data *  New_Volume = NULL;

  FUNCTION_ENTRY("Create_Compatibility_Volume")

  /* Get the memory. */
  New_Volume = (Volume_Data *) malloc( sizeof(Volume_Data) );

  if ( New_Volume == NULL )
  {

    /* No more memory!  Abort. */
    *Error = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Create_Compatibility_Volume")

    return New_Volume;

  }

  /* Clear the memory we received. */
  memset(New_Volume,0,sizeof(Volume_Data) );

  /* Initialize the new Volume_Data object. */
  New_Volume->Volume_Serial_Number = PartitionRecord->DLA_Table_Entry.Volume_Serial_Number;
  New_Volume->Volume_Size = PartitionRecord->Partition_Size;
  New_Volume->Partition_Count = 1;                                                             /* Compatibility volumes are always 1! */
  New_Volume->Compatibility_Volume = TRUE;
  New_Volume->Can_Be_Altered = TRUE;
  New_Volume->Drive_Letter_Conflict = 0;
  New_Volume->On_Boot_Manager_Menu = PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu;
  strncpy(New_Volume->Volume_Name, PartitionRecord->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);
  strncpy(New_Volume->File_System_Name, PartitionRecord->File_System_Name, FILESYSTEM_NAME_SIZE);
  New_Volume->Drive_Letter_Preference = PartitionRecord->DLA_Table_Entry.Drive_Letter;
  New_Volume->Partition = PartitionRecord;

  /* Now add this Volume_Data structure to the Volumes list. */
  New_Volume->Volume_Handle = InsertObject(Volumes, sizeof(Volume_Data), New_Volume, VOLUME_DATA_TAG, NULL, AppendToList, FALSE, Error);
  if ( *Error != DLIST_SUCCESS )
  {

    /* We have a problem!  Abort! */
    if ( *Error == DLIST_OUT_OF_MEMORY )
      *Error = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error = LVM_ENGINE_INTERNAL_ERROR;

    /* Cleanup what we can. */
    free(New_Volume);

    FUNCTION_EXIT("Create_Compatibility_Volume")

    return New_Volume;

  }

  /* Now we need an external handle for this volume. */
  New_Volume->External_Handle = Create_Handle( New_Volume, VOLUME_DATA_TAG, sizeof(Volume_Data), Error);

  if ( *Error != HANDLE_MANAGER_NO_ERROR )
  {

    /* We have a problem! Abort. */
    if ( *Error == HANDLE_MANAGER_OUT_OF_MEMORY)
      *Error = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Create_Compatibility_Volume")

    return New_Volume;

  }

  /* Now the Volume_Data object is complete.  Finish filling in some fields in the Partition_Data for the partition
     associated with this volume.                                                                                     */
  PartitionRecord->Volume_Handle = New_Volume->Volume_Handle;
  PartitionRecord->External_Volume_Handle = New_Volume->External_Handle;

  /* Is this volume a foreign volume?  If so, what this volume has for the serial number of the boot drive will not match
     that of the actual boot drive.  This is important for resolving drive letter conflicts.  If a foreign drive claims
     the same drive letter as a non-foreign drive, the foreign drive loses.                                                */
  if ( DriveArray[PartitionRecord->Drive_Index].Boot_Drive_Serial_Number != Boot_Drive_Serial_Number )
  {

    /* We have a foreign volume. */
    New_Volume->Foreign_Volume = TRUE;

  }
  else
    New_Volume->Foreign_Volume = FALSE;


  /* Are we at install time? */
  if ( Min_Install_Size > 0 )
  {

    /* Is this volume marked installable? */
    if ( PartitionRecord->DLA_Table_Entry.Installable )
    {

      /* Have we already seen a volume marked installable, or is this one too small to be marked installable? */
      if ( ( Install_Volume_Handle != NULL ) || ( PartitionRecord->Usable_Size < Min_Install_Size ) )
      {

        /* Since there already is a volume marked installable, unmark this one. */
        PartitionRecord->DLA_Table_Entry.Installable = FALSE;

        /* Indicate that changes were made so that our change will make it to disk if the user does a Commit. */
        DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      }
      else
        Install_Volume_Handle = New_Volume->Volume_Handle;

    }

  }


  FUNCTION_EXIT("Create_Compatibility_Volume")

  return New_Volume;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Is_Volume_Bootable(Volume_Data * VolumeRecord, BOOLEAN Check_Eligibility_Only, CARDINAL32 * Error_Code)
{

  Partition_Data *    PartitionRecord;    /* Used to examine the partition associated with a compatibility volume. */

  FUNCTION_ENTRY("Is_Volume_Bootable")

  /* If Boot Manager is installed and active and this volume is on the boot manager menu, then the volume is bootable. */
  if ( ( Boot_Manager_Handle != NULL ) && Boot_Manager_Active )
  {

    if ( VolumeRecord->On_Boot_Manager_Menu )
    {

      FUNCTION_EXIT("Is_Volume_Bootable")

      return TRUE;

    }
    else
      if ( ! Check_Eligibility_Only )
      {

        FUNCTION_EXIT("Is_Volume_Bootable")

        return FALSE;

      }

  }

  /* We must determine if this volume is a compatibility volume, if it is on the first drive, if its corresponding
     partition is a primary partition, and if its corresponding partition is the first active primary partition on the
     drive.                                                                                                             */

  /* Is this a compatibility volume? */
  if ( VolumeRecord->Compatibility_Volume )
  {

    /* It should have only one partition associated with it.  Lets get that partition and examine it. */
    PartitionRecord = VolumeRecord->Partition;

    if ( Check_Eligibility_Only )
    {

      /* Is this partition below the 1024 cylinder limit, if the limit applies? */
      if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
           ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
         )
      {

        /* Is Boot Manager Installed? */
        if ( ( Boot_Manager_Handle != NULL ) && Boot_Manager_Active )
        {

          FUNCTION_EXIT("Is_Volume_Bootable")

          /* This partition is eligible to be booted. */
          return TRUE;

        }
        else
        {

          /* This partition must be a primary partition and must be on the first disk. */
          if ( PartitionRecord->Primary_Partition &&
               ( PartitionRecord->Drive_Index == 0 )
             )
          {

            FUNCTION_EXIT("Is_Volume_Bootable")

            return TRUE;

          }
          else
          {

            FUNCTION_EXIT("Is_Volume_Bootable")

            return FALSE;

          }

        }

      }
      else
      {

        FUNCTION_EXIT("Is_Volume_Bootable")

        return FALSE;

      }

    }
    else
    {

      /* If we get here, then Boot Manager is not installed and we are NOT checking for Boot Eligibility. */

      /* In order for this partition to be bootable, it must be a primary partition, it must be active, and it must be on the first drive. */
      if ( PartitionRecord->Primary_Partition &&
           ( PartitionRecord->Drive_Index == 0 ) &&
           ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) != 0 )
         )
      {

        FUNCTION_EXIT("Is_Volume_Bootable")

        return TRUE;

      }
      else
      {

        FUNCTION_EXIT("Is_Volume_Bootable")

        return FALSE;

      }

    }

  }

  FUNCTION_EXIT("Is_Volume_Bootable")

  /* If we get here, then the volume is not bootable. */
  return FALSE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Is_Volume_Startable(Volume_Data * VolumeRecord, BOOLEAN Check_Eligibility_Only, CARDINAL32 * Error_Code)
{

  Partition_Data *    PartitionRecord;    /* Used to examine the partition associated with a compatibility volume. */

  FUNCTION_ENTRY("Is_Volume_Startable")

  /* We must determine if this volume is a compatibility volume, if it is on the first drive, if its corresponding
     partition is a primary partition, and if its corresponding partition is the first active primary partition on the
     drive.                                                                                                             */

  /* Is this a compatibility volume? */
  if ( VolumeRecord->Compatibility_Volume )
  {

    /* It should have only one partition associated with it.  Lets get that partition and examine it. */
    PartitionRecord = VolumeRecord->Partition;

    if ( Check_Eligibility_Only )
    {

      /* Is this partition below the 1024 cylinder limit, if the limit applies? */
      if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
           ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
         )
      {

        /* This partition must be a primary partition and must be on the first disk. */
        if ( PartitionRecord->Primary_Partition &&
             ( PartitionRecord->Drive_Index == 0 )
           )
        {

          FUNCTION_EXIT("Is_Volume_Startable")

          return TRUE;

        }
        else
        {

          FUNCTION_EXIT("Is_Volume_Startable")

          return FALSE;

        }

      }
      else
      {

        FUNCTION_EXIT("Is_Volume_Startable")

        return FALSE;

      }

    }
    else
    {

      /* In order for this partition to be startable, it must be a primary partition, it must be active, and it must be on the first drive. */
      if ( PartitionRecord->Primary_Partition &&
           ( PartitionRecord->Drive_Index == 0 ) &&
           ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) != 0 )
         )
      {

        FUNCTION_EXIT("Is_Volume_Startable")

        return TRUE;

      }
      else
      {

        FUNCTION_EXIT("Is_Volume_Startable")

        return FALSE;

      }

    }

  }

  FUNCTION_EXIT("Is_Volume_Startable")

  /* If we get here, then the volume is not startable. */
  return FALSE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Partition_List_Is_Valid( CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], BOOLEAN No_PRMs, CARDINAL32 * Error_Code )
{

  ADDRESS                            Object;                   /* Used when translating a Handle into a Partition_Data structure. */
  TAG                                ObjectTag;                /* Used when translating a Handle into a Partition_Data structure. */
  CARDINAL32                         CurrentPartition;         /* Used to walk the Partition_Handles array. */
  Partition_Data *                   PartitionRecord;          /* Used when accessing the data for a partition that the user has specified. */


  FUNCTION_ENTRY("Partition_List_Is_Valid")

  /* Validate the list of partition handles. */
  for ( CurrentPartition = 0; CurrentPartition < Partition_Count; CurrentPartition++ )
  {

    /* We must translate the handle given to use to see if it really is a partition.  */

    /* Translate the handle. */
    Translate_Handle( Partition_Handles[CurrentPartition], &Object, &ObjectTag, Error_Code );

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Partition_List_Is_Valid")

      return FALSE;

    }

    /* From the ObjectTag we can tell what Object points to. */

    /* Is the object what we want? */
    if ( ObjectTag != PARTITION_DATA_TAG )
    {

      /* We have a bad handle.  */
      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Partition_List_Is_Valid")

      return FALSE;

    }

    /* Establish access to the Partition_Data structure we want to manipulate. */
    PartitionRecord = ( Partition_Data * ) Object;

    /* Does this partition record represent something other than a partition? */
    if ( PartitionRecord->Partition_Type != Partition )
    {

      /* We have a bad handle.  */
      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Partition_List_Is_Valid")

      return FALSE;

    }

    /* Is this partition already part of a volume? */
    if ( PartitionRecord->Volume_Handle != NULL )
    {

      /* This partition is already part of a volume. */
      *Error_Code = LVM_ENGINE_PARTITION_ALREADY_IN_USE;

      FUNCTION_EXIT("Partition_List_Is_Valid")

      return FALSE;

    }

    /* PRMs can not be drive linked!  Check to see if any of the partitions are a PRM, and, if so, are we are trying to link partitions. */
    if ( ( DriveArray[PartitionRecord->Drive_Index].Is_PRM && (Partition_Count > 1) ) ||
         ( DriveArray[PartitionRecord->Drive_Index].Is_PRM && No_PRMs )
       )
    {

      /* We have a bad handle.  */
      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Partition_List_Is_Valid")

      return FALSE;

    }

  }

  /* All of the partitions are valid!  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Partition_List_Is_Valid")

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Allocate_Signature_Sectors( CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], BOOLEAN Make_Fake_EBR, CARDINAL32 * Error_Code )
{

  ADDRESS                            Object;                   /* Used when translating a Handle into a Partition_Data structure. */
  TAG                                ObjectTag;                /* Used when translating a Handle into a Partition_Data structure. */
  CARDINAL32                         CurrentPartition;         /* Used to walk the Partition_Handles array. */
  Partition_Data *                   PartitionRecord;          /* Used when accessing the data for a partition that the user has specified. */
  LVM_Signature_Sector *             Signature_Sector;         /* Used when initializing the LVM Signature Sectors.                         */

  FUNCTION_ENTRY("Allocate_Signature_Sectors")

  /* We must allocate an LVM Signature Sector for every partition in the array. */
  for ( CurrentPartition = 0; CurrentPartition < Partition_Count; CurrentPartition++ )
  {

    /* We must translate the handle given to use to see if it really is a partition.  */

    /* Translate the handle. */
    Translate_Handle( Partition_Handles[CurrentPartition], &Object, &ObjectTag, Error_Code );

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Allocate_Signature_Sectors")

      return FALSE;

    }

    /* From the ObjectTag we can tell what Object points to. */

    /* Is the object what we want? */
    if ( ObjectTag != PARTITION_DATA_TAG )
    {

      /* We have a bad handle.  */
      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Allocate_Signature_Sectors")

      return FALSE;

    }

    /* Establish access to the Partition_Data structure we want to manipulate. */
    PartitionRecord = ( Partition_Data * ) Object;

    /* Does this partition already have an LVM Signature sector? */
    if ( PartitionRecord->Signature_Sector == NULL )
    {

      /* Allocate a signature sector for this partition. */
      PartitionRecord->Signature_Sector = ( LVM_Signature_Sector *) malloc( BYTES_PER_SECTOR );

      if ( PartitionRecord->Signature_Sector == NULL )
      {

        /* We are out of memory! */
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        FUNCTION_EXIT("Allocate_Signature_Sectors")

        return FALSE;

      }

    }

    /* Initialize the LVM Signature Sector and the DLA Table Entry for this partition. */
    Signature_Sector = PartitionRecord->Signature_Sector;
    memset(Signature_Sector,0,BYTES_PER_SECTOR);
    Signature_Sector->LVM_Signature1 = LVM_PRIMARY_SIGNATURE;
    Signature_Sector->LVM_Signature2 = LVM_SECONDARY_SIGNATURE;
    Signature_Sector->Signature_Sector_CRC = 0;

    /* Does the drive containing this partition have a serial number? */
    if ( DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number == 0 )
    {

      /* Create a serial number for the drive. */
      DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number = Create_Serial_Number();

      /* Mark the drive as changed so that the new serial number will make it to disk. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

    /* Does this partition have a serial number? */
    if ( PartitionRecord->DLA_Table_Entry.Partition_Serial_Number == 0 )
    {

      /* Since it does not have a serial number, give it one. */
      PartitionRecord->DLA_Table_Entry.Partition_Serial_Number = Create_Serial_Number();

      /* Mark the drive holding this partition as changed so that the new serial number will make it to disk. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

    Signature_Sector->Partition_Serial_Number = PartitionRecord->DLA_Table_Entry.Partition_Serial_Number;
    Signature_Sector->Partition_Start = PartitionRecord->Starting_Sector;
    PartitionRecord->DLA_Table_Entry.Partition_Start = PartitionRecord->Starting_Sector;
    Signature_Sector->Partition_End = PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size - 1;
    Signature_Sector->Partition_Sector_Count = PartitionRecord->Partition_Size;
    PartitionRecord->DLA_Table_Entry.Partition_Size = PartitionRecord->Partition_Size;
    Signature_Sector->Partition_Size_To_Report_To_User = PartitionRecord->Partition_Size - 1;                 /* One sector reserved for LVM Signature Sector. */
    Signature_Sector->LVM_Reserved_Sector_Count = 1;
    Signature_Sector->Fake_EBR_Location = 0;
    Signature_Sector->Fake_EBR_Allocated = FALSE;

    if ( Make_Fake_EBR )
    {

      /* This is the first partition of an LVM Volume.  We must create a "fake" EBR. */
      Signature_Sector->LVM_Reserved_Sector_Count += SYNTHETIC_SECTORS_PER_TRACK;
      Signature_Sector->Partition_Size_To_Report_To_User = PartitionRecord->Partition_Size - Signature_Sector->LVM_Reserved_Sector_Count;
      Signature_Sector->Fake_EBR_Location = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;
      Signature_Sector->Fake_EBR_Allocated = TRUE;

    }

    Signature_Sector->Boot_Disk_Serial_Number = Boot_Drive_Serial_Number;
    Signature_Sector->Volume_Serial_Number = 0;
    Signature_Sector->LVM_Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
    Signature_Sector->LVM_Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;
    strncpy(Signature_Sector->Partition_Name, PartitionRecord->Partition_Name, PARTITION_NAME_SIZE);
    PartitionRecord->Usable_Size = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;

  }

  /* All of the partitions have signature sectors allocated!  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Allocate_Signature_Sectors")

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Free_Signature_Sectors( CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], CARDINAL32 * Error_Code )
{

  ADDRESS                            Object;                   /* Used when translating a Handle into a Partition_Data structure. */
  TAG                                ObjectTag;                /* Used when translating a Handle into a Partition_Data structure. */
  CARDINAL32                         CurrentPartition;         /* Used to walk the Partition_Handles array. */
  Partition_Data *                   PartitionRecord;          /* Used when accessing the data for a partition that the user has specified. */


  FUNCTION_ENTRY("Free_Signature_Sectors")

  /* We must free the LVM Signature Sector for every partition in the array which has one. */
  for ( CurrentPartition = 0; CurrentPartition < Partition_Count; CurrentPartition++ )
  {

    /* We must translate the handle given to use to see if it really is a partition.  */

    /* Translate the handle. */
    Translate_Handle( Partition_Handles[CurrentPartition], &Object, &ObjectTag, Error_Code );

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Free_Signature_Sectors")

      return FALSE;

    }

    /* From the ObjectTag we can tell what Object points to. */

    /* Is the object what we want? */
    if ( ObjectTag != PARTITION_DATA_TAG )
    {

      /* We have a bad handle.  */
      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      FUNCTION_EXIT("Free_Signature_Sectors")

      return FALSE;

    }

    /* Establish access to the Partition_Data structure we want to manipulate. */
    PartitionRecord = ( Partition_Data * ) Object;

    /* If this partition has a signature sector allocated, then free it. */
    if ( PartitionRecord->Signature_Sector != NULL )
    {

      memset(PartitionRecord->Signature_Sector,0,sizeof( LVM_Signature_Sector ) );
      free(PartitionRecord->Signature_Sector);

      PartitionRecord->Signature_Sector = NULL;
      PartitionRecord->Usable_Size = PartitionRecord->Partition_Size;

    }

    /* Does this partition record have feature data? */
    if ( PartitionRecord->Feature_Data != NULL )
    {

      memset(PartitionRecord->Feature_Data,0,sizeof(Feature_Context_Data) );
      free(PartitionRecord->Feature_Data);

      PartitionRecord->Feature_Data = NULL;

    }

    /* Clear out some of the entries in the DLA Table for the partition. */
    PartitionRecord->DLA_Table_Entry.Drive_Letter = 0x00;
    PartitionRecord->DLA_Table_Entry.Volume_Serial_Number = 0x00;
    PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;
    PartitionRecord->DLA_Table_Entry.Installable = FALSE;
    PartitionRecord->DLA_Table_Entry.Volume_Name[0] = 0x00;

  }

  /* All of the partitions have signature sectors allocated!  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Free_Signature_Sectors")

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static Volume_Data *  Create_Default_LVM_Volume( CARDINAL32 * Error_Code )
{

  Volume_Data *            New_Volume = NULL;
  CARDINAL32               Ignore_Error;

  FUNCTION_ENTRY("Create_Default_LVM_Volume")

  /* Get the memory. */
  New_Volume = (Volume_Data *) malloc( sizeof(Volume_Data) );

  if ( New_Volume == NULL )
  {

    /* No more memory!  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Create_Default_LVM_Volume")

    return NULL;

  }

  /* Clear the memory we received. */
  memset(New_Volume,0,sizeof(Volume_Data) );

  /* Initialize the new Volume_Data object. */
  New_Volume->Volume_Serial_Number = Create_Serial_Number();
  New_Volume->Compatibility_Volume = FALSE;
  New_Volume->Can_Be_Altered = TRUE;
  New_Volume->On_Boot_Manager_Menu = FALSE;
  New_Volume->New_Volume = TRUE;
  New_Volume->ChangesMade = TRUE;
  New_Volume->Foreign_Volume = FALSE;
  New_Volume->Partition = NULL;
  New_Volume->Next_Aggregate_Number = 1;

  /* Now add this Volume_Data structure to the Volumes list. */
  New_Volume->Volume_Handle = InsertObject(Volumes, sizeof(Volume_Data), New_Volume, VOLUME_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have a problem!  Abort! */
    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    /* Cleanup what we can. */
    free(New_Volume);

    FUNCTION_EXIT("Create_Default_LVM_Volume")

    return NULL;

  }

  /* Now we need an external handle for this volume. */
  New_Volume->External_Handle = Create_Handle( New_Volume, VOLUME_DATA_TAG, sizeof(Volume_Data), Error_Code);

  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    /* We have a problem! Abort. */
    if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY)
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    /* Cleanup what we can. */
    DeleteItem(Volumes, FALSE, New_Volume->Volume_Handle, &Ignore_Error);
    free(New_Volume);

    FUNCTION_EXIT("Create_Default_LVM_Volume")

    return NULL;

  }

  FUNCTION_EXIT("Create_Default_LVM_Volume")

  return New_Volume;

}



/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Complete_Partition_Initialization(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  Volume_Data * VolumeRecord = ( Volume_Data *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;


  FUNCTION_ENTRY("Complete_Partition_Initialization")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Complete_Partition_Initialization")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Fill in the fields that indicate that this partition is now part of a volume. */
    PartitionRecord->External_Volume_Handle = VolumeRecord->External_Handle;
    PartitionRecord->Volume_Handle = VolumeRecord->Volume_Handle;
    PartitionRecord->DLA_Table_Entry.Volume_Serial_Number = VolumeRecord->Volume_Serial_Number;
    PartitionRecord->DLA_Table_Entry.Drive_Letter = VolumeRecord->Drive_Letter_Preference;
    PartitionRecord->Partition_Table_Entry.Format_Indicator = LVM_PARTITION_INDICATOR;
    strncpy(PartitionRecord->DLA_Table_Entry.Volume_Name, VolumeRecord->Volume_Name, VOLUME_NAME_SIZE);

    if ( VolumeRecord->Partition_Count > 1)
      PartitionRecord->Spanned_Volume = TRUE;
    else
      PartitionRecord->Spanned_Volume = FALSE;

    /* Is this partition record an aggregate? */
    if ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
    {

      /* Now we must find the partitions list for the aggregate. */
      Current_Context = PartitionRecord->Feature_Data;

      /* The only feature's which will have a partitions list are Aggregators.  There can be only
         one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
      while ( Current_Context->Partitions == NULL )
      {

        /* Get the next context in the feature context chain for this aggregate. */
        Current_Context = Current_Context->Old_Context;

      }

      /* We must process the partitions which are part of this aggregate. */
      ForEachItem( Current_Context->Partitions, &Complete_Partition_Initialization, Parameters, TRUE, Error);
      if ( *Error != DLIST_SUCCESS)
      {

        FUNCTION_EXIT("Complete_Partition_Initialization")

        return;

      }

    }
    else
    {

      /* We must mark the drive containing this partition as being changed so that our changes to this partition will be written to disk. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Complete_Partition_Initialization")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Remove_All_Partition_Features(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a variable so that we can access the function table associated with the features on this partition/aggregate. */
  Plugin_Function_Table_V1 * Old_Function_Table;

  FUNCTION_ENTRY("Remove_All_Partition_Features")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Remove_All_Partition_Features")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {

    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Remove_All_Partition_Features")

    return;

  }

#endif

#endif

  /* We must call the delete function for this partition. */
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;
  Old_Function_Table->Remove_Features(PartitionRecord, Error);

  FUNCTION_EXIT("Remove_All_Partition_Features")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Commit_All_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data * VolumeRecord = ( Volume_Data *) Object;

  FUNCTION_ENTRY("Commit_All_Changes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Commit_All_Changes")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Is this an LVM volume? */
  if ( ! VolumeRecord->Compatibility_Volume )
  {

    /* For each volume, we must commit the feature changes on each of its partitions. */
    Commit_Feature_Changes(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, VolumeRecord, Error);
    if ( *Error == LVM_ENGINE_NO_ERROR )
      VolumeRecord->ChangesMade = FALSE;      /* All changes successfully written to disk. */

  }
  else
    VolumeRecord->ChangesMade = FALSE;

  /* Indicate success so that subsequent volumes will be processed.  The only errors encountered should be I/O errors, which
     will be marked in the DriveArray.  Thus, we don't want to stop processing volumes here.                                    */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Commit_All_Changes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Commit_Feature_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  Volume_Data * VolumeRecord = ( Volume_Data *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access the function table of the first feature associated with PartitionRecord. */
  Plugin_Function_Table_V1 * Partition_Function_Table;

  FUNCTION_ENTRY("Commit_Feature_Changes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Commit_Feature_Changes")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Commit_Feature_Changes")

    return;

  }

#endif

#endif

  /* Call the Commit function associated with this partition record. */
  Partition_Function_Table = PartitionRecord->Feature_Data->Function_Table;
  Partition_Function_Table->Commit( VolumeRecord, PartitionRecord, Error);

  /* The only errors we should encounter are I/O errors.  Since I/O errors are tracked in the DriveArray, we don't need to
     concern ourselves with any errors returned here.  Indicate success and leave.                                          */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Commit_Feature_Changes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void Update_Partition_Data( Volume_Data * VolumeRecord, CARDINAL32 Partition_Count, ADDRESS Partition_Handles[], CARDINAL32 * Error_Code )
{

  ADDRESS                            Object;                   /* Used when translating a Handle into a Partition_Data structure. */
  TAG                                ObjectTag;                /* Used when translating a Handle into a Partition_Data structure. */
  CARDINAL32                         CurrentPartition;         /* Used to walk the Partition_Handles array. */
  Partition_Data *                   PartitionRecord;          /* Used when accessing the data for a partition that the user has specified. */


  FUNCTION_ENTRY("Update_Partition_Data")

  /* We must update each partition in the array. */
  for ( CurrentPartition = 0; CurrentPartition < Partition_Count; CurrentPartition++ )
  {

    /* We must translate the handle given to us to see if it really is a partition.  */

    /* Translate the handle. */
    Translate_Handle( Partition_Handles[CurrentPartition], &Object, &ObjectTag, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == HANDLE_MANAGER_NO_ERROR );
    assert( ObjectTag == PARTITION_DATA_TAG );

#else

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      /* Something is wrong as all of these handles were supposed to have been checked already! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Update_Partition_Data")

      return ;

    }

    /* From the ObjectTag we can tell what Object points to. */

    /* Is the object what we want? */
    if ( ObjectTag != PARTITION_DATA_TAG )
    {

      /* Something is wrong as all of these handles were supposed to have been checked already! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Update_Partition_Data")

      return ;

    }

#endif

#endif

    /* Establish access to the Partition_Data structure we want to manipulate. */
    PartitionRecord = ( Partition_Data * ) Object;

#ifdef DEBUG

#ifdef PARANOID

    assert( PartitionRecord->Partition_Type == Partition );

#else

    /* Does this partition record represent a partition? */
    if ( PartitionRecord->Partition_Type != Partition )
    {

      /* Something is wrong as all of these handles were supposed to have been checked already! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Update_Partition_Data")

      return ;

    }

#endif

#endif

    /* Update the PartitionRecord. */
    PartitionRecord->External_Volume_Handle = VolumeRecord->External_Handle;
    PartitionRecord->Volume_Handle = VolumeRecord->Volume_Handle;
    PartitionRecord->DLA_Table_Entry.Volume_Serial_Number = VolumeRecord->Volume_Serial_Number;
    PartitionRecord->DLA_Table_Entry.Drive_Letter = VolumeRecord->Drive_Letter_Preference;
    strncpy( PartitionRecord->DLA_Table_Entry.Volume_Name, VolumeRecord->Volume_Name, VOLUME_NAME_SIZE);
    strncpy( PartitionRecord->File_System_Name, VolumeRecord->File_System_Name, FILESYSTEM_NAME_SIZE);
    PartitionRecord->Partition_Table_Entry.Format_Indicator = LVM_PARTITION_INDICATOR;
    PartitionRecord->Spanned_Volume = TRUE;

    /* Mark the drive containing the partition as being dirty. */
    DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  }

  /* Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Update_Partition_Data")

  return ;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Count_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  Volume_Data * New_Volume = ( Volume_Data *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;


  FUNCTION_ENTRY("Count_Partitions")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Count_Partitions")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );
  assert( PartitionRecord->Feature_Data != NULL );

#else

  /* Does this partition record represent a partition? */
  if ( ( PartitionRecord->Partition_Type != Partition ) ||
       ( PartitionRecord->Feature_Data == NULL )
     )
  {
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Count_Partitions")

    return;

  }

#endif

#endif

  /* Is this partition record an aggregate? */
  if ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
  {

    /* Now we must find the partitions list for the aggregate. */
    Current_Context = PartitionRecord->Feature_Data;

    /* The only feature's which will have a partitions list are Aggregators.  There can be only
       one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
    while ( Current_Context->Partitions == NULL )
    {

      /* Get the next context in the feature context chain for this aggregate. */
      Current_Context = Current_Context->Old_Context;

    }

    /* Since this partition record is an aggregate, it doesn't count.  Process its Partitions list. */
    ForEachItem(Current_Context->Partitions, &Count_Partitions, Parameters, TRUE, Error);

  }
  else
  {

    /* Since this is not an aggregate, count it. */
    New_Volume->Partition_Count += 1;

    /* Set its Volume related fields. */
    PartitionRecord->External_Volume_Handle = New_Volume->External_Handle;
    PartitionRecord->Volume_Handle = New_Volume->Volume_Handle;

    /* If this partition is a PRM, then mark the volume accordingly. */
    if ( ( New_Volume->Device_Type != LVM_PRM ) && DriveArray[PartitionRecord->Drive_Index].Is_PRM )
    {

      New_Volume->Device_Type = LVM_PRM;

    }

    /* Indicate success and leave. */
    *Error = DLIST_SUCCESS;

  }

  FUNCTION_EXIT("Count_Partitions")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Get_Volume_Size( ADDRESS Handle, CARDINAL32 * Error_Code)
{

  Volume_Data * VolumeRecord;

  FUNCTION_ENTRY("Get_Volume_Size")

  /* Find the volume specified by Handle in the Volumes list. */

  /* Get a pointer to the Volume Data. */
  VolumeRecord = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Handle, TRUE, Error_Code);

  /* Did we get the pointer? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Volume_Size")

    return 0;

  }

  FUNCTION_EXIT("Get_Volume_Size")

  /* Return the volume size. */
  return VolumeRecord->Volume_Size;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Clear_Boot_Sectors(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare the structure needed to add a Boot Sector to the Kill List. */
  Kill_Sector_Data  SectorData;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;


  FUNCTION_ENTRY("Clear_Boot_Sectors")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Clear_Boot_Sectors")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );
  assert( PartitionRecord->Feature_Data != NULL );

#else

  /* Does this partition record represent a partition? */
  if ( ( PartitionRecord->Partition_Type != Partition ) ||
       ( PartitionRecord->Feature_Data == NULL )
     )
  {
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Clear_Boot_Sectors")

    return;

  }

#endif

#endif

  /* Is this partition record an aggregate? */
  if ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
  {

    /* Now we must find the partitions list for the aggregate. */
    Current_Context = PartitionRecord->Feature_Data;

    /* The only feature's which will have a partitions list are Aggregators.  There can be only
       one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
    while ( Current_Context->Partitions == NULL )
    {

      /* Get the next context in the feature context chain for this aggregate. */
      Current_Context = Current_Context->Old_Context;

    }

    /* Since this partition record is an aggregate, it doesn't have a Boot Sector to kill. */
    ForEachItem(Current_Context->Partitions, &Clear_Boot_Sectors, Parameters, TRUE, Error);

  }
  else
  {

    /* Since this is not an aggregate, add its Boot Sector to the Kill_Sector list. */
    SectorData.Sector_ID = PartitionRecord->Starting_Sector;
    SectorData.Drive_Index = PartitionRecord->Drive_Index;

    /* Add the Boot Sector to the KillSector list. */
    InsertItem(KillSector, sizeof(Kill_Sector_Data), &SectorData, KILL_SECTOR_DATA_TAG, NULL, AppendToList, FALSE, Error);

    /* Did we succeed? */
    if ( *Error != DLIST_SUCCESS )
    {

      /* Are we out of memory? */
      if ( *Error == DLIST_OUT_OF_MEMORY )
      {

        FUNCTION_EXIT("Clear_Boot_Sectors")

        /* Return to caller. */
        return;

      }
      else
      {

        /* This must be some kind of internal error! */
        *Error = DLIST_CORRUPTED;

        FUNCTION_EXIT("Clear_Boot_Sectors")

        return;

      }

    }

    /* Indicate success and leave. */
    *Error = DLIST_SUCCESS;

  }

  FUNCTION_EXIT("Clear_Boot_Sectors")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Check_For_Corrupt_Drive(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  BOOLEAN * Corrupt_Drive_Found = ( BOOLEAN * ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *              PartitionRecord = (Partition_Data *) Object;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;


  FUNCTION_ENTRY("Check_For_Corrupt_Drive")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) || ( PartitionRecord->Partition_Type != Partition ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Check_For_Corrupt_Drive")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Do we have an aggregate or a partition? */
  if ( PartitionRecord->Drive_Index != (CARDINAL32) -1L )
  {

    /* We have a partition.  Check the drive it resides on for corruption. */
    if ( DriveArray[PartitionRecord->Drive_Index].Corrupt && !DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt)
    {

      *Corrupt_Drive_Found = TRUE;
      *Error = DLIST_SEARCH_COMPLETE;

      FUNCTION_EXIT("Check_For_Corrupt_Drive")

      return;

    }

  } else  {

    /* Now we must find the partitions list for the aggregate. */
    Current_Context = PartitionRecord->Feature_Data;

    /* The only feature's which will have a partitions list are Aggregators.  There can be only
       one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
    while ( Current_Context->Partitions == NULL )
    {

      /* Get the next context in the feature context chain for this aggregate. */
      Current_Context = Current_Context->Old_Context;

    }

    /* We have an aggregate.  We must process the partitions list of the aggregate. */
    ForEachItem( Current_Context->Partitions, &Check_For_Corrupt_Drive, Parameters, TRUE, Error);

    if ( ( *Error != DLIST_SUCCESS ) || *Corrupt_Drive_Found )
    {

      FUNCTION_EXIT("Check_For_Corrupt_Drive")

      return;

    }

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Check_For_Corrupt_Drive")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Extend_FS(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Volume_Data *            VolumeRecord = ( Volume_Data * ) Object;

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  ExtendFS_Control_Data *  Command_Data = (ExtendFS_Control_Data *) Parameters;

  /* Declare a variable to hold the return code from the DosExecPgm API. */
  APIRET        ReturnCode;

  FUNCTION_ENTRY("Extend_FS")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Extend_FS")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this volume have to be expanded? */
  if ( ( VolumeRecord->Expand_Volume ) &&  ( VolumeRecord->Filesystem_Is_Expandable ) && ( ! RebootRequired ) )
  {

    /* What are we to do with this volume? */
    switch ( Command_Data->Command )
    {

      case Count_Volumes : /* Only count volumes that can be expanded without unmounting the filesystem!  This means that the filesystem
                              used on the volume must support Quiesce and Resume.  The volumes must also have a valid drive letter preference
                              and have an active drive letter assignment.                                                                      */
                           if ( ( VolumeRecord->Quiesce_Resume_Supported ) &&
                                ( ( VolumeRecord->Current_Drive_Letter >= 'C' ) &&
                                  ( VolumeRecord->Current_Drive_Letter <= 'Z' ) &&
                                  ( VolumeRecord->Initial_Drive_Letter >= 'C' ) &&
                                  ( VolumeRecord->Initial_Drive_Letter <= 'Z' )
                                )
                              )
                           {

                             /* Add this volume to our count of volumes requiring expansion. */
                             Command_Data->Volume_Count = Command_Data->Volume_Count + 1;

                             if ( Logging_Enabled )
                             {

                               sprintf(Log_Buffer,"Extend_FS invoked in Count mode.\n     The current volume qualifies and has been counted.\n     The current volume has a Current Drive Letter of %c.\n     The current volume has an initial drive letter of %c.", VolumeRecord->Current_Drive_Letter, VolumeRecord->Initial_Drive_Letter );
                               Write_Log_Buffer();

                             }

                           }

                           break;

      case Fill  : /* We are only interested in volumes that can be expanded without unmounting the filesystem! */
                   if ( ( VolumeRecord->Quiesce_Resume_Supported ) &&
                        ( ( VolumeRecord->Current_Drive_Letter >= 'C' ) &&
                          ( VolumeRecord->Current_Drive_Letter <= 'Z' ) &&
                          ( VolumeRecord->Initial_Drive_Letter >= 'C' ) &&
                          ( VolumeRecord->Initial_Drive_Letter <= 'Z' )
                        )
                      )
                   {

                     if ( Logging_Enabled )
                     {

                       sprintf(Log_Buffer,"Extend_FS invoked in Fill mode.\n     The current volume qualifies and a Volume_Expansion_Array entry is being prepared for it.\n     The current volume has a Current Drive Letter of %c.\n     The current volume has an initial drive letter of %c.", VolumeRecord->Current_Drive_Letter, VolumeRecord->Initial_Drive_Letter );
                       Write_Log_Buffer();

                     }

                     /* Fill in the data required to get this volume expanded. */
                     Command_Data->Volume_Expansion_Array[Command_Data->Volume_Count].DDI_Volume_UnitID = VolumeRecord->UnitID;
                     Command_Data->Volume_Expansion_Array[Command_Data->Volume_Count].DDI_Volume_SerialNumber = VolumeRecord->Volume_Serial_Number;
                     Command_Data->Volume_Count = Command_Data->Volume_Count + 1;

                     LOG_EVENT("Attempting to quiesce the filesystem on the volume.")

                     /* Quiesce the filesystem. */
                     if ( ! VolumeRecord->Expansion_DLL_Data->Quiesce(VolumeRecord->Current_Drive_Letter) )
                     {

                       LOG_EVENT("The filesystem on the volume failed the Quiesce request!")

                       /* We could not quiesce the filesystem, therefore a reboot is required. */
                       RebootRequired = TRUE;

                       /* Abort the ForEachItem operation without generating an error. */
                       *Error = DLIST_SEARCH_COMPLETE;

                     }
                     else
                       VolumeRecord->Volume_Is_Quiesced = TRUE;

                   }

                   break;

      case Complete : /* We are interested in any volume that was expanded. */

                      /* Complete the volume expansion process. */

                      /* Make sure that we have a valid drive letter.  At this point, Initial_Drive_Letter and Current_Drive_Letter
                         are the same, so we only need to check the Current_Drive_Letter field.                                      */
                      if ( ( VolumeRecord->Current_Drive_Letter >= 'C' ) &&
                           ( VolumeRecord->Current_Drive_Letter <= 'Z' )
                         )
                      {


                        if ( Logging_Enabled )
                        {

                          sprintf(Log_Buffer,"Extend_FS invoked in Complete mode.\n     The current volume qualifies and the expansion process for it will be completed.\n     The current volume has a Current Drive Letter of %c.\n     The current volume has an initial drive letter of %c.", VolumeRecord->Current_Drive_Letter, VolumeRecord->Initial_Drive_Letter );
                          Write_Log_Buffer();

                        }

                        /* Tell the filesystem to resume operations. */

                        /* Do we need to restart the filesystem? */
                        if ( VolumeRecord->Quiesce_Resume_Supported && VolumeRecord->Volume_Is_Quiesced )
                        {

                          LOG_EVENT("Instructing the filesystem on the volume to resume operations.")

                          /* Restart the filesystem. */
                          if  ( VolumeRecord->Expansion_DLL_Data->Resume(VolumeRecord->Current_Drive_Letter) )
                            VolumeRecord->Volume_Is_Quiesced = FALSE;
                          else
                          {

                            LOG_EVENT("The filesystem could not be restarted!  A reboot is now required!")

                            RebootRequired = TRUE;       /* We could not restart the filesystem, therefore a reboot is required. */

                          }

                        }

                        /* If we are not in a reboot situation, and the volume is formatted, we can call the ExtendFS system utility. */
                        if ( ( ! RebootRequired ) && ( strncmp(VolumeRecord->File_System_Name, "unformatted", FILESYSTEM_NAME_SIZE ) != 0 ) )
                        {

                          LOG_EVENT("Calling the filesystem expansion utility for this volume.")

                          /* Since we have a valid drive letter, we can call the ExtendFS system utility. */

                          /* Use the DosExecPgm API to run the extendfs utility.  */
                          ProgramArgs[DRIVE_LETTER_INDEX] = VolumeRecord->Current_Drive_Letter;               /* Specify the volume to expand. */
                          ReturnCode = DosExecPgm(FailureName, sizeof(FailureName), EXEC_SYNC, ProgramArgs, NULL, &Results, ProgramName);

                          /* Did we succeed? */
                          if ( ( ReturnCode != NO_ERROR ) || ( Results.codeResult != NO_ERROR ) || ( Results.codeTerminate != NO_ERROR ) )
                          {

                            LOG_EVENT("The filesystem expansion utility failed!  A reboot is now required.")

                            /* We need a reboot! */
                            RebootRequired = TRUE;

                          }

                        }

                      }

                      /* As a last check, we should never encounter a quiesced volume at this point! */
                      if ( VolumeRecord->Volume_Is_Quiesced )
                      {

                        LOG_ERROR("The current volume is quiesced when it should not be!  A reboot is now required!")

                        /* We should not have a Quiesced volume here!  Reboot! */
                        RebootRequired = TRUE;

                      }

                      break;

      default :
                /* If we get here, then we have an unrecognized command! */
                *Error = DLIST_CORRUPTED;

                LOG_ERROR1("Unrecognized command given to Extend_FS!","Command", Command_Data->Command)

                FUNCTION_EXIT("Extend_FS")

                return;

                break;  /* Keep the compiler happy. */

    }


  }

  FUNCTION_EXIT("Extend_FS")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static INTEGER32 _System Sort_By_Current_Drive_Letter( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error)
{

  /* Declare local variables so that we can access the Volume_Data objects without having to typecast each time. */
  Volume_Data *    Volume1 = (Volume_Data *) Object1;
  Volume_Data *    Volume2 = (Volume_Data *) Object2;

  /* Some volumes may not have a current drive letter yet (i.e. they could be new volumes ), so we must use
     their drive letter preference here instead.  Declare local variables so that we can hold the drive
     letters we actually intend to use for the comparisons.                                                   */
  char             Drive_Letter1;
  char             Drive_Letter2;

  FUNCTION_ENTRY("Sort_By_Current_Drive_Letter")

#ifdef DEBUG

  /* Are Object1 and Object2 what we think it should be? */
  if ( ( Object1Tag != VOLUME_DATA_TAG ) || ( Object2Tag != VOLUME_DATA_TAG ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Sort_By_Current_Drive_Letter")

    return 0;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Extract the drive letter used to represent Volume1. */
  if ( Volume1->Current_Drive_Letter == 0 )
    Drive_Letter1 = Volume1->Drive_Letter_Preference;
  else
    Drive_Letter1 = Volume1->Current_Drive_Letter;

  /* Extract the drive letter used to represent Volume2. */
  if ( Volume2->Current_Drive_Letter == 0 )
    Drive_Letter2 = Volume2->Drive_Letter_Preference;
  else
    Drive_Letter2 = Volume2->Current_Drive_Letter;

  /* Is Volume1 < Volume2? */
  if ( Drive_Letter1 < Drive_Letter2 )
  {

    FUNCTION_EXIT("Sort_By_Current_Drive_Letter")

    return -1;

  }

  /* Is Volume1 > Volume2? */
  if ( Drive_Letter1 > Drive_Letter2 )
  {

    FUNCTION_EXIT("Sort_By_Current_Drive_Letter")

    return 1;

  }

  /* They must be equal! */

  FUNCTION_EXIT("Sort_By_Current_Drive_Letter")

  return 0;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Check_New_Drive_Letters(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  APIRET                Return_Code;                /* Used for the DosDevIOCtl call. */
  DeviceParameterPacket DevParms;                   /* Used with DosDevIOCtl to get the device parameters for the device associated with a drive letter. */
  Dynamic_Drive_Data    Drive_Packet;               /* Used to dynamically delete drives from the IFSM. */
  CARDINAL32            Parameter_Size;             /* Used for the DosDevIOCtl call. */
  char                  Drive_Letter;               /* Used during the dynamic drives phase of the commit operation. */
  CARDINAL32            Buffer_Size;                /* Used with DosQueryFSAttach. */

  FUNCTION_ENTRY("Check_New_Drive_Letters")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Check_New_Drive_Letters")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this volume already have a drive letter assigned to it? */
  if ( ( VolumeRecord->Drive_Letter_Preference == '*' ) || ( ( VolumeRecord->Current_Drive_Letter >= 'C' ) && ( VolumeRecord->Current_Drive_Letter <= 'Z' ) ) )
  {

    FUNCTION_EXIT("Check_New_Drive_Letters")

    /* Since this volume already has a drive letter assigned to it, we don't need to check it.  */
    return;

  }

  /* Well, the Object Tags look OK, so lets get to it. */

  /* We must test this drive letter to see if we can use it dynamically. */
  Drive_Letter = VolumeRecord->Drive_Letter_Preference;

  /* We must query the device parameters to see if we have a device here.  If there is already a
     device at this drive letter, then we must reboot the system to get the conflict resolved.    */

  /* Set up the Drive_Packet for the IOCTL call. */
  Drive_Packet.drive_unit = Drive_Letter - 'A';
  Drive_Packet.cmd_info = 0;

  /* Set up the size fields. */
  Parameter_Size = sizeof(Dynamic_Drive_Data);
  Buffer_Size = sizeof(DeviceParameterPacket);

  /* Call the IOCTL. */
  Return_Code = DosDevIOCtl(-1L,
                            IOCTL_DISK,
                            DSK_GETDEVICEPARAMS,
                            &Drive_Packet,
                            sizeof(Dynamic_Drive_Data),
                            &Parameter_Size,
                            &DevParms ,
                            sizeof(DeviceParameterPacket),
                            &Buffer_Size );

  /* If the drive letter is not in use, then the IOCTL should fail with return code ERROR_INVALID_DRIVE. */
  if ( Return_Code != ERROR_INVALID_DRIVE )
  {

      RebootRequired = TRUE;

      /* We don't need to check anymore drive letters since a reboot is already required. */
      *Error = DLIST_SEARCH_COMPLETE;

  }

  FUNCTION_EXIT("Check_New_Drive_Letters")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Check_For_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *  PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  BOOLEAN *         Volume_Found = ( BOOLEAN * ) Parameters;


  FUNCTION_ENTRY("Check_For_Volumes")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Check_For_Volumes")

    return;

}

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition part of a volume? */
    if ( PartitionRecord->Volume_Handle != NULL )
    {

      /* Indicate that we found a volume. */
      *Volume_Found = TRUE;

      /* We don't need to search any further.  Indicate that we found what we were looking for. */
      *Error = DLIST_SEARCH_COMPLETE;

    }

  }

  FUNCTION_EXIT("Check_For_Volumes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Create_Fake_Volumes_For_PRM(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *  PartitionRecord = (Partition_Data *) Object;

  /* Delcare a local variable to allow us to access the "fake" Volume created for a partition. */
  Volume_Data *     VolumeRecord;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  CARDINAL32 *      Name_Count = ( CARDINAL32 * ) Parameters;

  /* Declare a local variable to hold the name of our "fake" volume. */
  char              Name[VOLUME_NAME_SIZE] = "";

  FUNCTION_ENTRY("Create_Fake_Volumes_For_PRM")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Create_Fake_Volumes_For_PRM")

    return;

}

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition part of a volume? */
    if ( PartitionRecord->Volume_Handle == NULL )
    {

      /* Create a "fake" volume for this partition. */

      do
      {

        /* If the PRM is in Big Floppy Mode, then we will use "PRM xxx" as the name of the fake volume.  Otherwise, we will
           use "V xxx" as the name of the volume.                                                                              */
        if ( DriveArray[PartitionRecord->Drive_Index].Is_Big_Floppy )
          sprintf(Name, "[ BIGFLOPPY %lu ]", *Name_Count);
        else
          sprintf(Name, "[ REMOVABLE %lu ]", *Name_Count);

        /* Now create the volume. */
        Create_Volume2(Name, FALSE, FALSE, '*', 0, NULL,1, &(PartitionRecord->External_Handle), Error);

        /* Update our name counter.  If we succeeded, then we need to update it so that the next time we make a name we get
           a unique value.  If we did not succeed, then it must be because of a name conflict with something the user named a volume. */
        *Name_Count = *Name_Count + 1;

      } while ( *Error == LVM_ENGINE_DUPLICATE_NAME );

      /* Was there an error we care about? */
      switch ( *Error )
      {

        case LVM_ENGINE_OUT_OF_MEMORY : *Error = DLIST_OUT_OF_MEMORY;
                                        break;
        case LVM_ENGINE_INTERNAL_ERROR : *Error = DLIST_CORRUPTED;
                                         break;
        case LVM_ENGINE_NO_ERROR : *Error = DLIST_SUCCESS;

                                   /* Now we must get the Volume and set the New_Volume flag to FALSE. */

                                   /* Get a pointer to the Volume Data. */
                                   VolumeRecord = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, PartitionRecord->Volume_Handle, TRUE, Error);

#ifdef DEBUG

                                   /* Did we get the pointer? */
                                   if ( *Error != DLIST_SUCCESS )
                                   {

                                     *Error = LVM_ENGINE_INTERNAL_ERROR;

                                     FUNCTION_EXIT("Create_Fake_Volumes_For_PRM")

                                     return;

                                   }

#endif

                                   /* Now turn off the New_Volume flag. */
                                   VolumeRecord->New_Volume = FALSE;

                                   /* If this "fake" volume represents a PRM which is in Big Floppy Mode, then the volume
                                      can not be changed, only deleted.                                                    */
                                   if ( DriveArray[PartitionRecord->Drive_Index].Is_Big_Floppy )
                                     VolumeRecord->Can_Be_Altered = FALSE;

                                   break;
        default : *Error = DLIST_CORRUPTED;
                  break;                   /* Keep the compiler happy. */

      }

    }

  }

  FUNCTION_EXIT("Create_Fake_Volumes_For_PRM")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Add_Fake_Volumes_To_Deleted_Volumes_List(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *  PartitionRecord = (Partition_Data *) Object;

  /* Delcare a local variable to allow us to access the "fake" Volume associated with this partition. */
  Volume_Data *     VolumeRecord;

  /* Declare a variable to manipulate the deleted drive letter bitmap. */
  CARDINAL32        Drive_Letter_Mask;

  FUNCTION_ENTRY("Add_Fake_Volumes_To_Deleted_Volumes_List")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Add_Fake_Volumes_To_Deleted_Volumes_List")

    return;

}

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition part of a volume? */
    if ( PartitionRecord->Volume_Handle != NULL )
    {

      /* Get the Volume_Data for the "fake" Volume that this partition belongs to. */

      /* Extract the Volume_Data from the Volumes list. */
      VolumeRecord = GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, PartitionRecord->Volume_Handle, TRUE, Error);

#ifdef DEBUG

#ifdef PARANOID

      assert(*Error == DLIST_SUCCESS);

#else

      if ( *Error != DLIST_SUCCESS)
      {

        FUNCTION_EXIT("Add_Fake_Volumes_To_Deleted_Volumes_List")

        return;

      }

#endif

#endif

      /* Now that we have the volume, lets add its drive letter to the list of deleted drive letters. */
      if ( ( VolumeRecord->Current_Drive_Letter != 0 ) && ( !VolumeRecord->New_Volume ) )
      {

        Drive_Letter_Mask = 0x1;

        Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

        Deleted_Drive_Letters = Deleted_Drive_Letters | Drive_Letter_Mask;

      }

    }

  }

  FUNCTION_EXIT("Add_Fake_Volumes_To_Deleted_Volumes_List")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Current_Drive_Letter_Conflicts(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  char *           Drive_Letter = (char *) Parameters;

  FUNCTION_ENTRY("Find_Current_Drive_Letter_Conflicts")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Find_Current_Drive_Letter_Conflicts")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this volume have a current drive letter assigned to it? */
  if ( ( VolumeRecord->Current_Drive_Letter >= 'C' ) && ( VolumeRecord->Current_Drive_Letter <= 'Z' ) )
  {

    /* We must see if the current drive letter of this volume matches our parameter. */
    if ( VolumeRecord->Current_Drive_Letter == *Drive_Letter )
    {

#ifdef DEBUG

#ifdef PARANOID

      assert( VolumeRecord->Current_Drive_Letter != VolumeRecord->Drive_Letter_Preference );

      assert( VolumeRecord->Drive_Letter_Preference == '*' );

#else

      if ( ( VolumeRecord->Current_Drive_Letter == VolumeRecord->Drive_Letter_Preference ) ||
           ( VolumeRecord->Drive_Letter_Preference != '*' )
         )
      {

        /* This should never happen!  The code which prevents conflicts between drive preferences should have prevented this! */
        *Error = DLIST_CORRUPTED;

        FUNCTION_EXIT("Find_Current_Drive_Letter_Conflicts")

        return;

      }

#endif

#endif

      /* We must zero out the current drive letter field of this volume. */
      VolumeRecord->Current_Drive_Letter = 0;

      /* We are done.  There should be no other conflicts like this, so we can stop looking. */
      *Error = DLIST_SEARCH_COMPLETE;

    }

  }

  FUNCTION_EXIT("Find_Current_Drive_Letter_Conflicts")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN _System Kill_Non_LVM_Device_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  /* We can not delete a volume until all of its components have been dealt with.  So, in case we abort early, we will set
     FreeMemory to false.  Once we have taken care of all of the components of the volume which is being deleted, then we can
     set FreeMemory to TRUE.                                                                                                 */
  *FreeMemory = FALSE;

  FUNCTION_ENTRY("Kill_Non_LVM_Device_Volumes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Kill_Non_LVM_Device_Volumes")

    return FALSE;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this volume represent a non-LVM device? */
  if ( ( VolumeRecord->Device_Type != LVM_HARD_DRIVE ) && ( VolumeRecord->Device_Type != LVM_PRM ) )
  {

    /* We have a non-LVM device.  Lets begin deleting this volume. */

    /* Dispose of the external handle used to reference this volume. */
    Destroy_Handle( VolumeRecord->External_Handle, Error );

  #ifdef DEBUG

  #ifdef PARANOID

    assert( *Error == HANDLE_MANAGER_NO_ERROR );

  #else

    if ( *Error != HANDLE_MANAGER_NO_ERROR )
    {

      *Error = DLIST_CORRUPTED;

      FUNCTION_EXIT("Kill_Non_LVM_Device_Volumes")

      return FALSE;

    }

  #endif

  #endif

    /* Since this volume represents a non-LVM device, it has no partitions or other structures to delete!  We just need to set
       FreeMemory to TRUE and return a function return value of TRUE and this volume is deleted.                                 */
    *FreeMemory = TRUE;

    FUNCTION_EXIT("Kill_Non_LVM_Device_Volumes")

    return TRUE;

  }

  FUNCTION_EXIT("Kill_Non_LVM_Device_Volumes")

  /* This volume does not represent a non-LVM volume, so return FALSE so that it will not be deleted. */
  return FALSE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Update_Current_Drive_Letter(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *                    PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access the Volume_Data which corresponds to the partition we are processing. */
  Volume_Data *                       VolumeRecord;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  Update_Current_Drive_Letter_Data *  LVM_Data = ( Update_Current_Drive_Letter_Data * ) Parameters;

  FUNCTION_ENTRY("Update_Current_Drive_Letter")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Update_Current_Drive_Letter")

    return;

  }

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this partition record represent a partition?  Is this partition at the LBA specified by OS2LVM?  Does this partition have a volume associated with it? */
  if ( ( PartitionRecord->Partition_Type == Partition) &&
       ( PartitionRecord->Drive_Index == ( LVM_Data->LVM_Drive_Number - 1 ) ) &&
       ( ( PartitionRecord->Starting_Sector == LVM_Data->LVM_LBA ) || ( DriveArray[PartitionRecord->Drive_Index].Is_PRM && DriveArray[PartitionRecord->Drive_Index].Is_Big_Floppy ) ) &&
       ( PartitionRecord->Volume_Handle != NULL )
     )
  {

    /* Get the Volume_Data for the Volume that this partition belongs to. */

    /* Extract the Volume_Data from the Volumes list. */
    VolumeRecord = GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, PartitionRecord->Volume_Handle, TRUE, Error);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error == DLIST_SUCCESS);

#else

    if ( *Error != DLIST_SUCCESS)
    {

      FUNCTION_EXIT("Update_Current_Drive_Letter")

      return;

    }

#endif

#endif


    /* Update the Current_Drive_Letter field in the VolumeRecord. */
    VolumeRecord->Current_Drive_Letter = LVM_Data->IFSM_Drive_Letter;

    /* Update the UnitID field with the UnitID supplied by OS2LVM. */
    VolumeRecord->UnitID = LVM_Data->UnitID;

    /* Set the Volume_Found flag. */
    LVM_Data->Volume_Found = TRUE;

    /* We have found what we are looking for, so end the search. */
    *Error = DLIST_SEARCH_COMPLETE;

  }

  FUNCTION_EXIT("Update_Current_Drive_Letter")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Reserved_Drive_Letters(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecase each time. */
  CARDINAL32 *     Reserved_Drive_Letters = (CARDINAL32 *) Parameters;

  /* Declare a variable to be used in setting the *Reserved_Drive_Letters variable. */
  CARDINAL32       Drive_Letter_Mask = 0x1;


  FUNCTION_ENTRY("Find_Reserved_Drive_Letters")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Find_Reserved_Drive_Letters")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Does the volume have a drive letter preference of '*'? */
  if ( VolumeRecord->Drive_Letter_Preference == '*' )
  {

    /* Does the volume have a current drive letter? */
    if ( VolumeRecord->Current_Drive_Letter != 0 )
    {

      /* Now we must mark the current drive letter as being reserved. */
      Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

      *Reserved_Drive_Letters = ( *Reserved_Drive_Letters ) | Drive_Letter_Mask;

    }

  }


  FUNCTION_EXIT("Find_Reserved_Drive_Letters")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Reconcile_Drive_Letters( BOOLEAN Update_NON_LVM_Volumes_Only, CARDINAL32 * Error_Code )
{

  Volume_Data *                     New_Volume;
  CARDINAL32                        Drive_Letter_Mask;
  CARDINAL32                        Reserved_Drive_Letters = 0;
  APIRET                            Query_Parms_Result;
  APIRET                            Query_Filesystem_Result;
  Dynamic_Drive_Data                Drive_Packet;                                           /* Used for the DosDevIOCtl call. */
  CARDINAL32                        Parameter_Size;                                         /* Used for the DosDevIOCtl call. */
  DeviceParameterPacket             DevParms;                                               /* Used with DosDevIOCtl to get the device parameters for the device associated with a drive letter. */
  BYTE                              QueryBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH) + 5]; /* Used with DosQueryFSAttach. */
  FSQBUFFER2 *                      QueryResult = (FSQBUFFER2 *) &QueryBuffer;              /* Used with DosQueryFSAttach. */
  char                              Drive_Name[3];                                          /* Used with DosQueryFSAttach. */
  CARDINAL32                        Buffer_Size;                                            /* Used with DosQueryFSAttach. */
  char *                            Filesystem_Name;                                        /* Used with DosQueryFSAttach. */

  Update_Current_Drive_Letter_Data  LVM_Data;
  char                              Drive_Letter;

  CARDINAL32 *                      Name_Count = NULL;                                      /* Used when creating unique names for "fake" volumes. */
  CARDINAL32                        V_Count = 1;                                            /* Used when creating unique names for "fake" volumes. */
  CARDINAL32                        CDROM_Count = 1;                                        /* Used when creating unique names for "fake" volumes. */
  CARDINAL32                        LAN_Count = 1;                                          /* Used when creating unique names for "fake" volumes. */

  FUNCTION_ENTRY("Reconcile_Drive_Letters")

/* We must determine the current drive letters for each drive.  If Update_NON_LVM_Volumes_Only is TRUE, then we will
   only update those volumes that represent devices which are NOT under the control of LVM.  The way we do this is
   as follows:

   1.  Delete all Volumes representing non-LVM devices
   2.  Traverse all drive letters.  For each drive letter get the OS2LVM View of the drive letter.
   3.  Find the device which corresponds to the data returned by OS2LVM and update its
       Current_Drive_Letter field, unless Update_NON_LVM_Volumes_Only is TRUE, in which case
       just ignore it.
   4.  For all drive letters that OS2LVM returns no data for, use GET_DEV_PARMS to determine if
       there is a device at that drive letter or not.  If there is a device at that drive letter,
       then create a "fake" volume for it.

When we are done, all Volumes should have the correct Current_Drive_Letter value.

*/

  LOG_EVENT("Deleting all Volumes representing non-LVM devices (i.e. network, cdrom, etc.).")

  /* Delete all of the volumes representing non-LVM devices. */
  PruneList(Volumes, &Kill_Non_LVM_Device_Volumes,NULL, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

        assert(*Error_Code == DLIST_SUCCESS);

#else

        if ( *Error_Code != DLIST_SUCCESS)
        {

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FUNCTION_EXIT("Reconcile_Drive_Letters")

          return 0;

        }

#endif

#endif

  LOG_EVENT("Querying OS2LVM about each drive letter.")

  /* Now we must find out what OS2LVM knows about each drive letter. */
  for ( Drive_Letter = 'C'; Drive_Letter <= 'Z'; Drive_Letter++ )
  {

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"Processing drive letter %c.", Drive_Letter);
      Write_Log_Buffer();

    }

    /* Save the drive letter we are currently working with.  It may be needed later by the Update_Current_Drive_Letter function. */
    LVM_Data.IFSM_Drive_Letter = Drive_Letter;
    LVM_Data.LVM_Drive_Number = 0xFFFFFFFFL;

    if ( Get_LVM_View( Drive_Letter, &LVM_Data.LVM_Drive_Number, &LVM_Data.LVM_LBA, &LVM_Data.LVM_Drive_Letter, &LVM_Data.UnitID) &&
         ( LVM_Data.LVM_Drive_Number <= DriveCount )
       )
    {

      /* Are we updating volumes representing LVM controlled devices or not? */
      if ( ! Update_NON_LVM_Volumes_Only )
      {

        LOG_EVENT("OS2LVM recognized the current drive letter.")

        /* OS2LVM knows about a volume which is currently assigned to this drive letter.  Lets find that volume and
           update its Current_Drive_Letter field.                                                                     */

        /* We must walk the partitions list for the drive specified by OS2LVM and find the partition specified by LVM_LBA.
           Once we have found that partition, its partition data will give us the handle of the Volume it belongs to.  Once
           we have the handle, we can get the volume data and update its Current_Drive_Letter field.  All of this is done
           for us by the Update_Current_Drive_Letter function.                                                                */

        LVM_Data.Volume_Found = FALSE;

        ForEachItem(DriveArray[LVM_Data.LVM_Drive_Number - 1].Partitions, &Update_Current_Drive_Letter, &LVM_Data, TRUE, Error_Code);

        /* Did we succeed in finding the volume and updating its current drive letter? */
        if ( ( *Error_Code != DLIST_SUCCESS ) || ( ! LVM_Data.Volume_Found ) )
        {

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"OS2LVM knows about drive letter %c, which we don't!\n     Marking drive %d corrupt!", Drive_Letter, LVM_Data.LVM_Drive_Number );
            Write_Log_Buffer();

          }

          /* OS2LVM knows about a volume that we do not!  We must have rejected that volume when scanning the drives
             in the system for partitions and volumes.  Thus, that drive must be corrupt.  Make sure that the corrupt flag is set! */
          DriveArray[LVM_Data.LVM_Drive_Number - 1].Corrupt = TRUE;
          if(DriveArray[LVM_Data.LVM_Drive_Number - 1].LastErrorIOCTL == 0 &&
             DriveArray[LVM_Data.LVM_Drive_Number - 1].LastError == 0)
               DriveArray[LVM_Data.LVM_Drive_Number - 1].NonFatalCorrupt  = TRUE;
          else
             DriveArray[LVM_Data.LVM_Drive_Number - 1].NonFatalCorrupt  = FALSE;

          /* Add the drive letter of the volume to the deleted drive letters bitmap. */

          Drive_Letter_Mask = 0x1;

          Drive_Letter_Mask = Drive_Letter_Mask << ( Drive_Letter - 'A' );

          Deleted_Drive_Letters = Deleted_Drive_Letters | Drive_Letter_Mask;

        }

      }

    }
    else
    {

      LOG_EVENT("OS2LVM did not recognize the drive letter!  Checking to see what is there (i.e. network, cdrom, etc.).")

      /* OS2LVM did not recognize the drive letter!  Lets see if anything is really using this drive letter. */

      /* We will query the device parameters and the attached filesystem.  If no filesystem is attached and the
         query of the device parameters fails, then we will assume that the drive letter is not in use.  If
         either one succeeds, then we must assume that the drive letter is in use by a non-LVM device.  In this
         case we must create a "fake" volume to represent this non-LVM device.                                   */

      /* Query the device parameters. */

      /* Set up the Drive_Packet for the IOCTL call. */
      Drive_Packet.drive_unit = Drive_Letter - 'A';
      Drive_Packet.cmd_info = 0;

      /* Set up the size fields. */
      Parameter_Size = sizeof(Dynamic_Drive_Data);
      Buffer_Size = sizeof(DeviceParameterPacket);

      /* Call the IOCTL. */
      Query_Parms_Result = DosDevIOCtl(-1L, IOCTL_DISK, DSK_GETDEVICEPARAMS, &Drive_Packet, sizeof(Dynamic_Drive_Data), &Parameter_Size, &DevParms , sizeof(DeviceParameterPacket), &Buffer_Size );

      /* Lets see if we can find out if a filesystem is attached to the drive letter.  We need to do this whether or not the Query Device Parameters succeeded. */
      Drive_Name[0] = Drive_Letter;
      Drive_Name[1] = ':';
      Drive_Name[2] = 0x00;
      Buffer_Size = sizeof(FSQBUFFER2) + (3 * CCHMAXPATH) + 5;

      /* Find out what is associated with the drive letter. */
      Query_Filesystem_Result = DosQueryFSAttach(Drive_Name,0,FSAIL_QUERYNAME , QueryResult, &Buffer_Size);

      /* Did both the queries fail? */
      if ( ( Query_Parms_Result != NO_ERROR ) && ( Query_Filesystem_Result != NO_ERROR ) )
      {

        LOG_EVENT("No filesystem is attached to the current drive letter.  OS/2 has no drive parameters associated with the current drive letter.")

        LOG_EVENT("The current drive letter is NOT in use.")

        /* We will assume that this drive letter is not being used by anything in the system.  Continue on to the next drive letter to be tested. */
        continue;

      }

      /* Extract the filesystem name from the query result. */
      Filesystem_Name = (char *) ( (CARDINAL32) &(QueryResult->szName) + QueryResult->cbName + 1);

      /* We must create a "fake" volume for this drive letter.  The Volume name must be unique within the system! */

      LOG_EVENT("Creating a fake volume for the device attached to the current drive letter.")

      /* Allocate the memory for a new Volume_Data record. */
      New_Volume = (Volume_Data *) malloc(sizeof(Volume_Data) );

      if ( New_Volume == NULL )
      {

        /* We are out of memory!  Abort! */
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        FUNCTION_EXIT("Reconcile_Drive_Letters")

        return 0;

      }

      /* Begin initializing the fields of New_Volume. */
      New_Volume->Volume_Serial_Number = Create_Serial_Number();
      New_Volume->Partition_Count = 0;                            /* This is a "fake" volume representing a non-LVM device -- it has no partitions! */
      New_Volume->Drive_Letter_Conflict = 0;                      /* It has no drive letter conflicts. */
      New_Volume->Partition = NULL;
      New_Volume->New_Volume = FALSE;
      New_Volume->Compatibility_Volume = TRUE;
      New_Volume->ChangesMade = FALSE;
      New_Volume->Foreign_Volume = FALSE;
      New_Volume->Drive_Letter_Preference = '*';
      New_Volume->Current_Drive_Letter = Drive_Letter;
      New_Volume->Initial_Drive_Letter = Drive_Letter;
      New_Volume->On_Boot_Manager_Menu = FALSE;
      New_Volume->Can_Be_Altered = FALSE;
      New_Volume->Expand_Volume = FALSE;

      /* Set the volume size based upon the geometry returned as part of the Get Device Parameters call done earlier. */
      if ( Query_Parms_Result == NO_ERROR )
      {

        New_Volume->Volume_Size = DevParms.BPB.cLargeSectors;

      }
      else
        New_Volume->Volume_Size = 0;        /* Since we have no parameters, make it 0. */

      /* Use the results of the Query on the attached filesystem to set some fields. */
      if ( Query_Filesystem_Result == NO_ERROR)
      {

        /* Save the filesystem name. */
        strncpy(New_Volume->File_System_Name, Filesystem_Name, FILESYSTEM_NAME_SIZE );

        /* Set up Name_Count assuming that the current "fake" volume will get a generic "V" based name. */
        Name_Count = &V_Count;

        /* Set the device type based upon the filesystem that is attached. */
        if ( strcmp(Filesystem_Name, "LAN" ) == 0 )
        {

          LOG_EVENT("The fake volume being created is for a LAN (network) drive.")

          /* Set the device type. */
          New_Volume->Device_Type = NETWORK_DRIVE;

          /* LAN Connections do NOT have a drive letter preference of '*'.  By definition, their drive letter
             preference is the same as their actual drive letter.                                              */
          New_Volume->Drive_Letter_Preference = Drive_Letter;

          /* We must add this drive letter to the Reserved_Drive_Letters.  We do this here because LAN drives
             do not have a drive letter preference of '*', which is what the rest of the code looks for to
             determine whether or not the current drive letter of a volume needs to be added to the Reserved_Drive_Letters. */
          Drive_Letter_Mask = 0x1;
          Drive_Letter_Mask = Drive_Letter_Mask << ( Drive_Letter - 'A' );
          Reserved_Drive_Letters = Reserved_Drive_Letters | Drive_Letter_Mask;

          /* Set Name_Count to the count used for LAN drives. */
          Name_Count = &LAN_Count;

        }
        else
          if ( strcmp(Filesystem_Name, "CDFS") == 0 )
          {

            LOG_EVENT("The fake volume being created is for a CDROM.")

            /* Set the device type. */
            New_Volume->Device_Type = NON_LVM_CDROM;

            /* Set Filesystem_Name to CDROM so that the volume name will be of the form "CDROM X", where X is a number. */
            strcpy(Filesystem_Name, "CDROM");

            /* Set Name_Count to the count used for CDROMs. */
            Name_Count = &CDROM_Count;

          }
          else
          {

            LOG_EVENT("The fake volume being created is for a non-LVM device.")

            New_Volume->Device_Type = NON_LVM_DEVICE;

          }

        /* Add a space to Filesystem_Name so that, when Create_Unique_Name is run, the number it appends to Filesystem_Name will
           be separated from Filesystem_Name by a space.                                                                           */
        strcat(Filesystem_Name," ");

        if ( ! Create_Unique_Name( VOLUME_NAMES, TRUE, Filesystem_Name, Name_Count, New_Volume->Volume_Name, VOLUME_NAME_SIZE) )
        {

          /* There can't be enough partitions in the system for this to fail.  Therefore the failure is not name related.
             We must have some kind of internal error.                                                                     */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FUNCTION_EXIT("Reconcile_Drive_Letters")

          return 0;

        }

      }
      else
      {

        LOG_EVENT("Assuming that the fake volume being created is for a non-LVM device.")

        /* Assume a non-LVM device that is not a CD-ROM. */
        New_Volume->Device_Type = NON_LVM_DEVICE;
        New_Volume->File_System_Name[0] = 0;
        Filesystem_Name[0] = 'V';                           /* Default name for an unrecognized device. */
        Filesystem_Name[1] = 0;
        Name_Count = &V_Count;

        /* If we have device parameters available, we must check for a CD-ROM with no media. */
        if ( Query_Parms_Result == NO_ERROR )
        {

          LOG_EVENT("The fake volume is being created for a CD-ROM which has no media in it.")

          /* We can determine we have a CD-ROM by checking the bytes per sector, the sectors per track, and the
             device type.  Bytes per sector will be 2048 for a CD-ROM, and sectors per track will be the
             equivalent of -1, and the device type will be other.                                                    */
          if ( ( DevParms.BPB.usBytesPerSector == 2048 ) &&
               ( DevParms.BPB.usSectorsPerTrack == (unsigned short ) -1 ) &&
               ( DevParms.BPB.bDeviceType == 7)
             )
          {

            New_Volume->Device_Type = NON_LVM_CDROM;
            strncpy(New_Volume->File_System_Name, "CDFS", FILESYSTEM_NAME_SIZE );
            strcpy(Filesystem_Name,"CDROM ");
            Name_Count = &CDROM_Count;

          }

        }

        /* Now create a unique name. */
        if ( ! Create_Unique_Name( VOLUME_NAMES, TRUE, Filesystem_Name, Name_Count, New_Volume->Volume_Name, VOLUME_NAME_SIZE) )
        {

          /* There can't be enough partitions in the system for this to fail.  Therefore the failure is not name related.
             We must have some kind of internal error.                                                                     */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FUNCTION_EXIT("Reconcile_Drive_Letters")

          return 0;

        }

      }

      /* Now add this Volume_Data structure to the Volumes list. */
      New_Volume->Volume_Handle = InsertObject(Volumes, sizeof(Volume_Data), New_Volume, VOLUME_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
      if ( *Error_Code != DLIST_SUCCESS )
      {

        /* We have a problem!  Abort! */
        if ( *Error_Code == DLIST_OUT_OF_MEMORY )
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
        else
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        /* Cleanup what we can. */
        free(New_Volume);

        FUNCTION_EXIT("Reconcile_Drive_Letters")

        return 0;

      }

      /* Now we need an external handle for this volume. */
      New_Volume->External_Handle = Create_Handle( New_Volume, VOLUME_DATA_TAG, sizeof(Volume_Data), Error_Code);

      if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
      {

        /* We have a problem! Abort. */
        if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY)
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
        else
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FUNCTION_EXIT("Reconcile_Drive_Letters")

        return 0;

      }

    }

  }

  LOG_EVENT("Determining the list of reserved drive letters.")

  /* Now we must determine the list of reserved drive letters.  All of the "fake" volumes created to represent things
     which consume drive letters but are not LVM controlled entities have a drive letter preference of '*' ( except
     for LAN connections, which are handled separately above ).  Also, any real volumes with drive letter preferences
     of '*' fall into this category and will be treated the same.  Basically, for everything with a drive letter
     assignement of '*', their current drive letters will be treated as reserved drive letters.  This means that
     the drive letters can be selected for use as the drive letter preference of a volume, but doing so may result
     in a reboot.  We need to find all of the volumes whose drive letter preference is '*' and whose current drive
     letter is non-zero, and put their current drive letters into the list of reserved drive letters.                    */
  ForEachItem( Volumes, &Find_Reserved_Drive_Letters, &Reserved_Drive_Letters, TRUE, Error_Code);

  LOG_EVENT1("Have determined the list of reserved drive letters.","Reserved Drive Letters Bitmap",Reserved_Drive_Letters)

  FUNCTION_EXIT("Reconcile_Drive_Letters")

  return Reserved_Drive_Letters;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Set_Initial_Drive_Letters(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  FUNCTION_ENTRY("Set_Initial_Drive_Letters")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Set_Initial_Drive_Letters")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* If we are in "Merlin" mode, then the current drive letter has not been set.  We will set it to the drive letter preference. */
  if ( Merlin_Mode )
    VolumeRecord->Current_Drive_Letter = VolumeRecord->Drive_Letter_Preference;

  /* Copy the Current_Drive_Letter field into the Initial_Drive_Letter field. */
  VolumeRecord->Initial_Drive_Letter = VolumeRecord->Current_Drive_Letter;

  FUNCTION_EXIT("Set_Initial_Drive_Letters")

  /* All done! */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Clear_Drive_Letter_Fields(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  FUNCTION_ENTRY("Clear_Drive_Letter_Fields")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Clear_Drive_Letter_Fields")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  VolumeRecord->Current_Drive_Letter = 0;
  VolumeRecord->Initial_Drive_Letter = 0;

  FUNCTION_EXIT("Clear_Drive_Letter_Fields")

  /* All done! */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Count_Drive_Letter_Claims(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *                VolumeRecord = (Volume_Data *) Object;

  /* Delcare a local variable so that we can access our parameters without having to typecast each time. */
  Drive_Letter_Count_Record *  Drive_Letter_Count = (Drive_Letter_Count_Record *) Parameters;

  FUNCTION_ENTRY("Count_Drive_Letter_Claims")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Count_Drive_Letter_Claims")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does the current volume have the specified drive letter as its Drive_Letter_Preference? */
  if ( VolumeRecord->Drive_Letter_Preference == Drive_Letter_Count->Drive_Letter )
  {

    /* The volume claims the drive letter so count it. */
    Drive_Letter_Count->Count += 1;

  }

  FUNCTION_EXIT("Count_Drive_Letter_Claims")

  /* All done! */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Existing_Potential_Volume(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Potential_Volume_Data object without having to typecast each time. */
  Potential_Volume_Data *         Potential_Volume = (Potential_Volume_Data *) Object;

  /* Delcare a local variable so that we can access our parameters without having to typecast each time. */
  Potential_Volume_Search_Data *  Search_Data = (Potential_Volume_Search_Data *) Parameters;

  FUNCTION_ENTRY("Find_Existing_Potential_Volume")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != POTENTIAL_VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Potential_Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Find_Existing_Potential_Volume")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does the current volume match the one specified in our search parameters? */
  if ( ( Potential_Volume->Volume_Serial_Number == Search_Data->PartitionRecord->DLA_Table_Entry.Volume_Serial_Number ) &&
       ( Potential_Volume->Boot_Drive_Serial_Number == DriveArray[Search_Data->PartitionRecord->Drive_Index].Boot_Drive_Serial_Number )
     )
  {

    /* The partition belongs to this volume!  Lets add it to the Partitions List for this volume. */
    InsertObject( Potential_Volume->Partition_List,
                  sizeof(Partition_Data),
                  Search_Data->PartitionRecord,
                  PARTITION_DATA_TAG,
                  NULL,
                  AppendToList,
                  FALSE,
                  Error);

    if ( *Error == DLIST_SUCCESS )
    {

      /* Halt further searches as we have already found what we were looking for. */
      *Error = DLIST_SEARCH_COMPLETE;

      /* Tell the caller that we found an existing potential volume and added the partition to its partition list. */
      Search_Data->Found = TRUE;

    }

  }

  FUNCTION_EXIT("Find_Existing_Potential_Volume")

  /* *Error has already been set, so just return. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Set_Feature_Index(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  FUNCTION_ENTRY("Set_Feature_Index")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof( Partition_Data ) ) );

#else

  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Set_Feature_Index")

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Set_Feature_Index")

    return;

  }

#endif

#endif

  /* Set the Feature_Index field to 0. */
  PartitionRecord->Feature_Index = 0;

  /* Indicate success. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Set_Feature_Index")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static INTEGER32 _System Sort_Partition_List_By_Feature_ID( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error)
{

  /* Declare local variables so that we can access the Partition_Data objects without having to typecast each time. */
  Partition_Data *    Partition1 = (Partition_Data *) Object1;
  Partition_Data *    Partition2 = (Partition_Data *) Object2;

  FUNCTION_ENTRY("Sort_Partition_List_By_Feature_ID")

#ifdef DEBUG

  /* Are Object1 and Object2 what we think they should be? */
  if ( ( Object1Tag != PARTITION_DATA_TAG ) ||
       ( Object2Tag != PARTITION_DATA_TAG )
     )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Sort_Partition_List_By_Feature_ID")

    return 0;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Is Partition1 < Partition2? */
  if ( Partition1->Signature_Sector->LVM_Feature_Array[Partition1->Feature_Index].Feature_ID < Partition2->Signature_Sector->LVM_Feature_Array[Partition2->Feature_Index].Feature_ID )
  {

    FUNCTION_EXIT("Sort_Partition_List_By_Feature_ID")

    return -1;

  }

  /* Is Partition1 > Partition2? */
  if ( Partition1->Signature_Sector->LVM_Feature_Array[Partition1->Feature_Index].Feature_ID > Partition2->Signature_Sector->LVM_Feature_Array[Partition2->Feature_Index].Feature_ID )
  {

    FUNCTION_EXIT("Sort_Partition_List_By_Feature_ID")

    return 1;

  }

  /* They must be equal! */

  FUNCTION_EXIT("Sort_Partition_List_By_Feature_ID")

  return 0;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Process_Potential_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Potential_Volume_Data object without having to typecast each time. */
  Potential_Volume_Data *         Potential_Volume = (Potential_Volume_Data *) Object;

  Partition_Data *                PartitionRecord;   /* Used to process the Partition List for this volume. */
  Volume_Data *                   New_Volume;        /* Used to create an entry in the Volumes list if the current volume is acceptable. */
  CARDINAL32                      LocalError;        /* Used on error paths. */
  BOOLEAN                         Create_Volume;     /* Used when processing an LVM Volume. */
  BOOLEAN                         Feature_Complete;  /* Used when processing an LVM Volume to prevent deadlock situations. */
  CARDINAL32                      Current_Feature_ID;/* Used when processing an LVM Volume. */
  ADDRESS                         Current_Handle;    /* Used when processing an LVM Volume. */
  Find_Feature_Data_Record        Search_Data;       /* Used when processing an LVM Volume. */
  CARDINAL32                      Count;             /* Used to determine the number of aggregates/partitions in the partition list
                                                        for a potential volume.                                                      */
  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;

  /* Declare a local variable to make it easy to access the function table for the partition. */
  Plugin_Function_Table_V1 *      Function_Table;


  FUNCTION_ENTRY("Process_Potential_Volumes")


#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != POTENTIAL_VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Potential_Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Process_Potential_Volumes")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* In order to process a potential volume, we must check to see what kind of a volume we have.  If we have a compatibility
     volume, then we must check to ensure that there is only one partition claiming to be a part of that volume, and that all
     of the fields in the DLA Table Entry for the partition in the volume are correct.  For an LVM Volume, we must check that
     features on the volume as listed in the LVM Signature Sector are available, that BBR is the first one listed, and that
     all of the features listed in the LVM Signature Sector accept the partitions in the partition list, thereby allowing the
     volume to be formed.                                                                                                       */

  /* Lets get a partition associated with this volume and find out what kind of a volume we should have. */
  PartitionRecord = (Partition_Data *) GetObject(Potential_Volume->Partition_List,sizeof(Partition_Data), PARTITION_DATA_TAG, NULL, FALSE, Error);

  /* Did we succeed? */
  if ( *Error != DLIST_SUCCESS )
  {

    LOG_ERROR1("GetObject failed!","Error code", *Error)

    FUNCTION_EXIT("Process_Potential_Volumes")

    /* This is bad news as this call should not have failed.  Abort! */
    return;

  }

#ifdef DEBUG

  if ( PartitionRecord == NULL )
  {

    /* PartitionData should not be NULL here!  Abort. */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Process_Potential_Volumes")

    return;

  }

#endif

  /* Now that we have the partition, see what kind of a volume it claims to belong to.  If it has an LVM Signature Sector,
     it must belong to an LVM volume.  If it does not have an LVM Signature Sector, it must belong to a compatibility volume. */
  if ( PartitionRecord->Signature_Sector == NULL )
  {

    LOG_EVENT("Processing compatibility volume.")

    /* We have a compatibility volume.  Compatibility volumes have only one partition in them.  How many partitions
       claim to be a part of this one?                                                                                 */
    if ( ( GetListSize(Potential_Volume->Partition_List, Error) > 1 ) ||
         ( *Error != DLIST_SUCCESS )
       )
    {

      LOG_ERROR("The current volume is invalid as it has too many partitions for a compatibility volume!")

      /* This volume has too many partitions claiming to be a part of it!  Destroy the partitions list for the volume
         and move on to the next entry in the Potential Volumes list.                                                  */
      DestroyList( &(Potential_Volume->Partition_List), FALSE, &LocalError );

      /* The Potential Volumes list will be deleted by the Discover_Volumes function, so we don't need to deal with
         removing the current Potential_Volume_Data record from the Potential_Volumes list here.                       */

      FUNCTION_EXIT("Process_Potential_Volumes")

      return;

    }

    /* We have a compatibility volume.  Lets check its DLA Table Entry.                                                */

    /* Is the DLA Table Entry complete enough?  There must be at least a Volume Serial Number and a Partition Serial Number.
       Check for these.  If these are all Ok, then create an entry in the Volumes list for this partition.                    */
    if ( ( PartitionRecord->DLA_Table_Entry.Volume_Serial_Number != 0 ) &&
         ( PartitionRecord->DLA_Table_Entry.Partition_Serial_Number != 0 )
       )
    {

      LOG_EVENT("The compatibility volume has been accepted.")

      /* We have a compatibility volume!  Create a Volume_Data object for it. */
      New_Volume = Create_Compatibility_Volume(PartitionRecord, Error);

      /* Was there an error? */
      if ( *Error != LVM_ENGINE_NO_ERROR )
      {

        LOG_ERROR1("Create_Compatibilty_Volume failed!","Error code",*Error)

        /* Translate to the appropriate DLIST error. */
        if ( *Error == LVM_ENGINE_OUT_OF_MEMORY )
          *Error = DLIST_OUT_OF_MEMORY;
        else
          *Error = DLIST_CORRUPTED;

        FUNCTION_EXIT("Process_Potential_Volumes")

        return;

      }

      /* Set some fields in the PartitionRecord. */
      PartitionRecord->Spanned_Volume = FALSE;

      /* Does this partition lie on a PRM?   If so, mark the volume accordingly. */
      if ( DriveArray[PartitionRecord->Drive_Index].Is_PRM )
        New_Volume->Device_Type = LVM_PRM;

    }

  }
  else
  {

    LOG_EVENT("Processing LVM volume.")

    /* We have an LVM Volume.  */

    /* The feature data for all partitions should already have been initialized to point to the pass thru layer. */

    /* We must apply the features to the volume in the correct order.  This is done as follows:

          The Feature_Index field for all partitions in the partition list is set to 0.
          The Partition List for the volume is sorted by feature ID.
          Set Create_Volume to TRUE.

          LOOP 1

            Set Feature_Complete to FALSE
            Make the first partition in the partition list the current partition.

            LOOP 2

              Save the handle of the current item in the partition list as Current_Handle

              The partition list is passed to the current feature ( as indicated in the Feature_Index field ) of the
                feature array of the current partition in the partition list.  The Feature will increment the
                Feature_Index field of any partitions that it accepts and takes ownership of.  It will also update the
                Feature_Data on these partitions, thereby adding itself to the feature chain for these partitions.  If
                the Feature creates an aggregate, it will remove the partitions in the aggregate from the partition list
                and append the aggregate it creates at the end of the partitions list.  The Feature_Index field of any
                such aggregate will be set to 0 by the Feature.  If a Feature accepts one or more partitions/aggregates,
                it will issue a return code of COMPLETE.  If it accepts no partitions/aggregates, it will issue a return
                code of INCOMPLETE.

              IF the feature issues a return code of COMPLETE THEN

                set Feature_Complete to TRUE
                exit LOOP 2

              ELSE

                Set the current item in the list using Current_Handle.

                Save as Current_Feature_ID the feature ID of the current feature ( as indicated in the Feature_Index
                field ) of the feature array of the current partition in the partition list

                WHILE ( NOT end of list ) AND ( the feature ID of the current feature of the current partition in the
                        partition list is the same as Current_Feature_ID ) DO

                  Make the next item in the list the current item

                END WHILE

                IF ( the feature ID of the current feature of the current partition in the partition list is the same
                     as Current_Feature_ID )  THEN

                  The volume can not be constructed!

                  Set Create_Volume to FALSE.

                  exit LOOP 2

                END IF

              END IF

            END LOOP 2

            IF NOT Feature_Complete THEN

              We can not create the volume as something is missing!

              Set Create_Volume to FALSE.

              exit LOOP 1

            ELSE

              Sort the partition list by feature ID.

              IF there is only 1 item left in the list THEN

                IF ( the LVM_Feature_Array for the item in the list is empty ) OR
                   ( the Feature_Index for the item in the list points to an empty entry in the LVM_Feature_Array ) THEN

                  exit LOOP 1.

                END IF

              END IF

            END IF

          END LOOP 1

          IF Create_Volume THEN

            create a Volume_Data record for the volume.
            Add the volume to the master list of Volumes.

          ELSE

            Delete the partitions list for this potential volume.

          END IF

    */

    /* Set the Feature_Index field for all partitions in the partition list to 0. */
    ForEachItem(Potential_Volume->Partition_List,&Set_Feature_Index,NULL, TRUE, Error);
    if ( *Error != DLIST_SUCCESS )
    {

      FUNCTION_EXIT("Process_Potential_Volumes")

      /* This should not have failed!  Abort. */
      return;

    }

    /* Sort the Partition List using the feature ID indicated by Feature_Index. */
    SortList(Potential_Volume->Partition_List, &Sort_Partition_List_By_Feature_ID, Error);
    if ( *Error != DLIST_SUCCESS )
    {

      FUNCTION_EXIT("Process_Potential_Volumes")

      /* This should not have failed!  Abort. */
      return;

    }

    /* Set Create_Volume to TRUE. */
    Create_Volume = TRUE;

    /* LOOP 1 */
    for (;;)
    {

      /* Set Feature_Complete to FALSE */
      Feature_Complete = FALSE;

      /* Make the first partition in the partition list the current partition. */
      GoToStartOfList( Potential_Volume->Partition_List, Error);
      if ( *Error != DLIST_SUCCESS )
      {

        FUNCTION_EXIT("Process_Potential_Volumes")

        /* This should not have failed!  Abort. */
        return;

      }

      /* LOOP 2 */
      for (;;)
      {

        /* Save the handle of the current item in the partition list as Current_Handle */
        Current_Handle = GetHandle( Potential_Volume->Partition_List, Error);
        if ( *Error != DLIST_SUCCESS )
        {

          FUNCTION_EXIT("Process_Potential_Volumes")

          /* This should not have failed!  Abort. */
          return;

        }

        /* Get the current partition from the partition list. */
        PartitionRecord = (Partition_Data *) GetObject(Potential_Volume->Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, NULL, FALSE, Error);
        if ( *Error != DLIST_SUCCESS )
        {

          FUNCTION_EXIT("Process_Potential_Volumes")

          /* This is bad news as this call should not have failed.  Abort! */
          return;

        }

        /* The feature list is passed to the current feature ( as indicated in the Feature_Index field ) of the
           feature array of the current partition in the partition list.  The Feature will increment the
           Feature_Index field of any partitions that it accepts and takes ownership of.  It will also update the
           Feature_Data on these partitions, thereby adding itself to the feature chain for these partitions.  If
           the Feature creates an aggregate, it will remove the partitions in the aggregate from the partition list
           and append the aggregate it creates at the end of the partitions list.  The Feature_Index field of any
           such aggregate will be set to 0 by the Feature.  If a Feature accepts one or more partitions/aggregates,
           it will issue a return code of LVM_ENGINE_NO_ERROR.  If it accepts no partitions/aggregates, it will
           issue a return code of LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE.                                             */

        /* Get the current feature ID. */
        Current_Feature_ID = PartitionRecord->Signature_Sector->LVM_Feature_Array[PartitionRecord->Feature_Index].Feature_ID;

        /* If Current_Feature_ID is 0, then we have no more features.  We should only get here if this is the first feature
           we are trying to process!  We can abort the processing of this partition as it can not be part of a volume.       */
        if ( Current_Feature_ID == 0 )
        {

          Create_Volume = FALSE;
          break;

        }

        /* Get the function table associated with this feature. */
        Search_Data.Feature_ID = Current_Feature_ID;
        ForEachItem(Available_Features, &Find_Feature_Given_ID, &Search_Data, TRUE, Error);
        if ( *Error != DLIST_SUCCESS )
        {

          FUNCTION_EXIT("Process_Potential_Volumes")

          /* This is bad news as this call should not have failed.  Abort! */
          return;

        }

        /* Was the specified feature found? */
        if ( Search_Data.Function_Table == NULL )
        {

          LOG_ERROR("The current volume specified a feature which does not exist!")

          /* The specified feature does not exist on this machine.  Abort the creation of the volume. */
          Create_Volume = FALSE;
          break;

        }

        /* Call the Discover function for the feature. */
        Function_Table = (Plugin_Function_Table_V1 *) (Search_Data.Function_Table);
        Function_Table->Discover( Potential_Volume->Partition_List, Error );

        /* Did we succeed? */
        switch(*Error)
        {
          case LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE : /* The feature needs something more before it can complete.  Proceed to the first partition with a different feature. */

                                                        /* Make sure the list is pointing to the partition we started with. */
                                                        GoToSpecifiedItem(Potential_Volume->Partition_List, Current_Handle, Error);
                                                        if ( *Error != DLIST_SUCCESS )
                                                        {

                                                          /* This should not have failed!  Either there is an internal error, or the feature removed the current item
                                                             but did not signal success!  We can not continue!  Abort.                                                 */
                                                          *Error = DLIST_CORRUPTED;

                                                          FUNCTION_EXIT("Process_Potential_Volumes")

                                                          return;

                                                        }

                                                        PartitionRecord = (Partition_Data *) GetNextObject(Potential_Volume->Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, Error);
                                                        while ( ( *Error == DLIST_SUCCESS) && ( Current_Feature_ID == PartitionRecord->Signature_Sector->LVM_Feature_Array[PartitionRecord->Feature_Index].Feature_ID ) )
                                                          PartitionRecord = (Partition_Data *) GetNextObject(Potential_Volume->Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, Error);

                                                        switch (*Error)
                                                        {
                                                          case DLIST_SUCCESS : /* We found another partition with a different feature as its current feature.  */
                                                                               break;
                                                          case DLIST_END_OF_LIST : /* We did not find another partition with a different current feature!  We can not create this volume
                                                                                      as something is missing and we don't know what it is.  Abort the creation of this volume and move
                                                                                      on to the next volume.                                                                             */

                                                                                   LOG_ERROR("Unable to create the current volume as some feature data is missing.")

                                                                                   Create_Volume = FALSE;
                                                                                   break;
                                                          default : /* There was some kind of unexpected error!  Abort! */

                                                                    FUNCTION_EXIT("Process_Potential_Volumes")

                                                                    return;
                                                        }

                                                        break;


          case LVM_ENGINE_NO_ERROR : /* The feature successfully completed! */
                                     Feature_Complete = TRUE;
                                     break;  /* Keep the compiler happy. */

          default : /* Some kind of error occurred!  Abort the creation of this volume. */
                    *Error = DLIST_CORRUPTED;

                    FUNCTION_EXIT("Process_Potential_Volumes")

                    return;

                    break; /* Keep the compiler happy. */
        }

        if ( ( ! Create_Volume ) || Feature_Complete )
          break;                 /* Exit LOOP 2 */

      } /* End of LOOP 2 */

      /* Did the current feature complete its discovery processes successfully? */
      if ( ( ! Feature_Complete ) || ( ! Create_Volume ) )
      {

        /* Since we exited the inner loop without a single feature completing its discovery process successfully, we
           must be missing something (another partition perhaps), or the feature data for one (or more) of the features
           on the volume must have been corrupted.  Either way, we can not create the volume.  Abort and move on.         */

        Create_Volume = FALSE;
        break;                  /* Exit LOOP 1 */

      }

      Count = GetListSize(Potential_Volume->Partition_List, Error);
      if ( *Error != DLIST_SUCCESS )
      {

        FUNCTION_EXIT("Process_Potential_Volumes")

        /* This should not have failed!  Abort. */
        return;

      }

      /* Are there any partitions or aggregates left?  If the features rejected all of the partitions, then
         we can not make a volume!                                                                           */
      if ( Count == 0 )
      {

        LOG_ERROR("The current volume can not be reconstructed as the feature data is corrupt or all of the required features are not available.")

        /* We can not create a volume, so indicate that and exit the loop. */
        Create_Volume = FALSE;
        break;

      }

      /* If there is more than one partition/aggregate remaining, then we still have more work to do. */
      if ( Count > 1 )
      {

        /* Sort the partition list by feature ID. */
        SortList(Potential_Volume->Partition_List, &Sort_Partition_List_By_Feature_ID, Error);
        if ( *Error != DLIST_SUCCESS )
        {

          FUNCTION_EXIT("Process_Potential_Volumes")

          /* This should not have failed!  Abort. */
          return;

        }

      }
      else
      {

        /* Get the last item in the list. */
        PartitionRecord = (Partition_Data *) GetObject(Potential_Volume->Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, NULL, FALSE, Error);
        if ( *Error != DLIST_SUCCESS )
        {

          FUNCTION_EXIT("Process_Potential_Volumes")

          /* This is bad news as this call should not have failed.  Abort! */
          return;

        }

        /* Have we processed all of its features yet? */
        if ( ( PartitionRecord->Signature_Sector == NULL ) ||
             ( PartitionRecord->Signature_Sector->LVM_Feature_Array[PartitionRecord->Feature_Index].Feature_ID == 0 )
           )
        {

          /* We have processed all of the features on this partition/aggregate.  We are done. */
          break;  /* Exit LOOP 1 */

        }

      }

    } /* End of LOOP 1 */

    /* Should we create a volume? */
    if ( Create_Volume )
    {

      LOG_EVENT("Volume accepted.  Recreating the LVM Volume under way.")
      /* The potential volume we are processing is complete enough to become a real volume.  Create a volume data
         record for it and place it in the list of volumes.                                                        */

      /* We have an LVM Volume.  Create a compatibility volume which we will then modify. */
      New_Volume = Create_Compatibility_Volume(PartitionRecord, Error);

      /* Was there an error? */
      if ( *Error != LVM_ENGINE_NO_ERROR )
      {

        /* Translate to the appropriate DLIST error. */
        if ( *Error == LVM_ENGINE_OUT_OF_MEMORY )
          *Error = DLIST_OUT_OF_MEMORY;
        else
          *Error = DLIST_CORRUPTED;

        FUNCTION_EXIT("Process_Potential_Volumes")

        return;

      }

      /* Now modify New_Volume to make it an LVM volume. */
      New_Volume->Compatibility_Volume = FALSE;

      /* We may have an aggregate here.  If we have an aggregate, we must count the partitions in the aggregate here.
         Also, we must set some of the Partition Record fields using the Volume information which is now available. */
      if ( PartitionRecord->Drive_Index != (CARDINAL32) -1L )
      {

        /* We have only a single partition here - there are no aggregators on this volume! */
        New_Volume->Partition_Count = 1;

        /* Now set some of the fields in the PartitionRecord using the volume data we have. */
        PartitionRecord->External_Volume_Handle = New_Volume->External_Handle;
        PartitionRecord->Volume_Handle = New_Volume->Volume_Handle;

        /* If this partition is a PRM, then mark the volume accordingly. */
        if ( ( New_Volume->Device_Type != LVM_PRM ) && DriveArray[PartitionRecord->Drive_Index].Is_PRM )
        {

          New_Volume->Device_Type = LVM_PRM;

        }

      }
      else
      {

        /* Save the Next_Aggregate_Number in case this volume is expanded later.   Remember that Version 1
           aggregates do not have LVM Signature Sectors!                                                    */
        if ( PartitionRecord->Signature_Sector != NULL )
          New_Volume->Next_Aggregate_Number = PartitionRecord->Signature_Sector->Next_Aggregate_Number;

        /* Now we must find the partitions list for the aggregate. */
        Current_Context = PartitionRecord->Feature_Data;

        /* The only feature's which will have a partitions list are Aggregators.  There can be only
           one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
        while ( Current_Context->Partitions == NULL )
        {

          /* Get the next context in the feature context chain for this aggregate. */
          Current_Context = Current_Context->Old_Context;

        }

        /* Prepare to count the partitions in the aggregate. */
        New_Volume->Partition_Count = 0;

        /* Now find out how many partitions are actually in the volume.
           Also, set some of the Partition Record fields using the Volume information which is now available.            */
        ForEachItem(Current_Context->Partitions, &Count_Partitions, New_Volume, TRUE, Error);

        /* Did we succeed? */
        if ( *Error != DLIST_SUCCESS )
        {

          FUNCTION_EXIT("Process_Potential_Volumes")

          return;

        }

      }

      /* Set the New_Volume and ChangesMade fields. */
      New_Volume->New_Volume = FALSE;
      New_Volume->ChangesMade = FALSE;

    }
    else
    {

      /* We must remove any features that have attached themselves to any of the partitions in the partition list. */
      ForEachItem( Potential_Volume->Partition_List, &Discovery_Failed_Remove_Features_From_Partitions, NULL, TRUE, &LocalError);

      /* Since we will not be creating a volume, delete the partitions list as we don't need it anymore. */
      DestroyList( &(Potential_Volume->Partition_List), FALSE, &LocalError );

    }

  }

  /* Indicate success so that we may proceed to the next potential volume. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Process_Potential_Volumes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Discovery_Failed_Remove_Features_From_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *                PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable to make it easy to access the function table for the partition. */
  Plugin_Function_Table_V1 *      Function_Table;

  FUNCTION_ENTRY("Discovery_Failed_Remove_Features_From_Partitions")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof( Partition_Data ) ) );

#else

  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Discovery_Failed_Remove_Features_From_Partitions")

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Discovery_Failed_Remove_Features_From_Partitions")

    return;

  }

#endif

#endif

  /* Call the Remove_Partition function for the topmost feature on each partition.  */
  if ( PartitionRecord->Feature_Data != NULL )
  {

    Function_Table = (Plugin_Function_Table_V1 *) PartitionRecord->Feature_Data->Function_Table;
    Function_Table->Remove_Features(PartitionRecord, Error);
    if ( *Error != LVM_ENGINE_NO_ERROR )
    {

      /* This should never happen!  Abort! */
      *Error = DLIST_CORRUPTED;

      FUNCTION_EXIT("Discovery_Failed_Remove_Features_From_Partitions")

      return;

    }

  }

  /* Indicate success. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Discovery_Failed_Remove_Features_From_Partitions")

  return;



}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static BOOLEAN Feature_List_Is_Bogus( CARDINAL32 Feature_Count, LVM_Feature_Specification_Record FeaturesToUse[], DLIST Features_List, BOOLEAN * Aggregator_Found, CARDINAL32 * Error_Code)
{

  CARDINAL32                 Current_Feature;
  Plugin_Function_Table_V1 * Current_Function_Table = NULL;
  BOOLEAN                    ClassExclusive_Found = FALSE;
  BOOLEAN                    GlobalExclusive_Found = FALSE;
  BOOLEAN                    TopExclusive_Found = FALSE;
  BOOLEAN                    BottomExclusive_Found = FALSE;
  LVM_Classes                Current_Class = Partition_Class;
  Find_Feature_Data_Record   Find_Feature_Data;
  Feature_ID_Data *          Current_Feature_ID = NULL;
  Feature_Application_Data   Feature_Application_Record;

  FUNCTION_ENTRY("Feature_List_Is_Bogus")

  /* Initialize Aggregator_Found to false.  We will set it to TRUE if we find one.  */
  *Aggregator_Found = FALSE;

  /* Walk the features array. */
  for ( Current_Feature = 0; Current_Feature < Feature_Count; Current_Feature++)
  {

    /* Is the specified class a valid class? */
    if ( FeaturesToUse[Current_Feature].Actual_Class > Volume_Class )
    {

      /* We have an illegal value for the LVM Class to use for this feature! */
      *Error_Code = LVM_ENGINE_WRONG_CLASS_FOR_FEATURE;

      FUNCTION_EXIT("Feature_List_Is_Bogus")

      return TRUE;

    }

    /* Did the user specify BBR as a feature?   If so, generate an error. */
    if ( FeaturesToUse[Current_Feature].Feature_ID == BBR_FEATURE_ID )
    {

      /* We have a bad feature ID! */
      *Error_Code = LVM_ENGINE_BAD_FEATURE_ID;

      FUNCTION_EXIT("Feature_List_Is_Bogus")

      return TRUE;

    }

    /* We must find the Feature which has the specified Feature ID. */
    Find_Feature_Data.Feature_ID = FeaturesToUse[Current_Feature].Feature_ID;
    Find_Feature_Data.Function_Table = NULL;
    ForEachItem(Available_Features, &Find_Feature_Given_ID,&Find_Feature_Data,TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* This should not happen here! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Feature_List_Is_Bogus")

      return TRUE;

    }

    /* Was a feature with a matching feature ID found? */
    if ( Find_Feature_Data.Function_Table == NULL)
    {

      /* We have a bad feature ID! */
      *Error_Code = LVM_ENGINE_BAD_FEATURE_ID;

      FUNCTION_EXIT("Feature_List_Is_Bogus")

      return TRUE;

    }

    /* Now that we have the function table for the feature, we can perform some checks on it. */
    if ( Current_Function_Table == NULL )
    {

      /* This is the first feature we have examined. */
      Current_Function_Table = Find_Feature_Data.Function_Table;
      Current_Feature_ID = Current_Function_Table->Feature_ID;


      /* Set the current class. */
      Current_Class = FeaturesToUse[Current_Feature].Actual_Class;

      /* Does the feature support the specified class? */
      if ( ! Current_Feature_ID->ClassData[Current_Class].ClassMember )
      {

        /* The feature does not support the specified class! */
        *Error_Code = LVM_ENGINE_WRONG_CLASS_FOR_FEATURE;

        FUNCTION_EXIT("Feature_List_Is_Bogus")

        return TRUE;

      }

      /* Since the feature supports the specified class, set our flags based on the attributes of our feature. */
      ClassExclusive_Found = Current_Feature_ID->ClassData[Current_Class].ClassExclusive;
      GlobalExclusive_Found = Current_Feature_ID->ClassData[Current_Class].GlobalExclusive;
      TopExclusive_Found = Current_Feature_ID->ClassData[Current_Class].TopExclusive;
      BottomExclusive_Found = Current_Feature_ID->ClassData[Current_Class].BottomExclusive;


    }
    else
    {

      /* We have already seen other features.  How does the data for this one stack up against the others? */

      /* Get the new function table and feature ID.  These are now the current function table and feature ID. */
      Current_Function_Table = Find_Feature_Data.Function_Table;
      Current_Feature_ID = Current_Function_Table->Feature_ID;

      /* Check the class.  If the specified class for the current feature is not supported by the current feature, or
         if the class specified for the feature must come before the current class, then the specified class is in error. */
      if ( ( FeaturesToUse[Current_Feature].Actual_Class < Current_Class ) ||
           ( ! Current_Feature_ID->ClassData[FeaturesToUse[Current_Feature].Actual_Class].ClassMember )
         )
      {

        /* We have an illegal value for the LVM Class to use for this feature! */
        *Error_Code = LVM_ENGINE_WRONG_CLASS_FOR_FEATURE;

        FUNCTION_EXIT("Feature_List_Is_Bogus")

        return TRUE;

      }

      /* Since the class checks out, lets check the flags. */
      if ( GlobalExclusive_Found || Current_Feature_ID->ClassData[FeaturesToUse[Current_Feature].Actual_Class].GlobalExclusive )
      {

        /* The specified features are not compatible! */
        *Error_Code = LVM_ENGINE_INCOMPATIBLE_FEATURES_SELECTED;

        FUNCTION_EXIT("Feature_List_Is_Bogus")

        return TRUE;

      }

      if ( Current_Class == FeaturesToUse[Current_Feature].Actual_Class )
      {

        if ( ( ClassExclusive_Found || Current_Feature_ID->ClassData[FeaturesToUse[Current_Feature].Actual_Class].ClassExclusive ) ||
             ( BottomExclusive_Found && Current_Feature_ID->ClassData[FeaturesToUse[Current_Feature].Actual_Class].BottomExclusive ) ||
             ( TopExclusive_Found && Current_Feature_ID->ClassData[FeaturesToUse[Current_Feature].Actual_Class].TopExclusive )
           )
        {

          /* The specified features are not compatible! */
          *Error_Code = LVM_ENGINE_INCOMPATIBLE_FEATURES_SELECTED;

          FUNCTION_EXIT("Feature_List_Is_Bogus")

          return TRUE;

        }

      }
      else
      {

        /* Since we have a new class here, lets update our flags. */
        Current_Class = FeaturesToUse[Current_Feature].Actual_Class;
        BottomExclusive_Found = FALSE;
        TopExclusive_Found = FALSE;
        ClassExclusive_Found = FALSE;

      }

    }

    /* Does this feature have support for the current interface? */
    if ( ! Current_Feature_ID->Interface_Support[Interface_In_Use].Interface_Supported )
    {

      /* This feature can not be used by the current interface. */
      *Error_Code = LVM_ENGINE_FEATURE_NOT_SUPPORTED_BY_INTERFACE;

      FUNCTION_EXIT("Feature_List_Is_Bogus")

      return TRUE;

    }

    /* Now we can add this feature to the Features_List.   Setup the Feature_Application_Record so that we can add
       this feature to the Features_List.                                                                            */
    Feature_Application_Record.Function_Table = Current_Function_Table;
    Feature_Application_Record.Actual_Class = Current_Class;
    Feature_Application_Record.Feature_Sequence_Number = Current_Feature + 2;  /* 0 = PassThru, 1 = BBR, so 2 is the first feature after BBR. */
    Feature_Application_Record.Init_Data = FeaturesToUse[Current_Feature].Init_Data;

    /* Is this feature the top feature in its class? */
    if ( ( ( Current_Feature +1 ) >= Feature_Count ) ||
         ( FeaturesToUse[Current_Feature].Actual_Class != FeaturesToUse[Current_Feature + 1].Actual_Class )
       )
      Feature_Application_Record.Top_Of_Class = TRUE;  /* This is either the last feature, or the last feature before a class change. */
    else
      Feature_Application_Record.Top_Of_Class = FALSE;

    /* Is this feature an aggregator? */
    if ( Feature_Application_Record.Actual_Class == Aggregate_Class )
      *Aggregator_Found = TRUE;

    /* Add this feature to the Features_List. */
    InsertItem(Features_List, sizeof(Feature_Application_Data), &Feature_Application_Record, FEATURE_APPLICATION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* Set the error code. */
      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      else
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Feature_List_Is_Bogus")

      return TRUE;

    }

  }

  /* It looks like the feature's specified are all valid, and their ordering is valid.  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Feature_List_Is_Bogus")

  return FALSE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Apply_Features(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  Feature_Application_Data *        FeatureRecord = (Feature_Application_Data *) Object;            /* Used to access the information on the feature to apply. */
  Apply_Features_Parameter_Record * ParameterData = (Apply_Features_Parameter_Record *) Parameters;  /* Used to get the partition list and the address of the error return variable. */
  Feature_ID_Data *                 Feature_ID;

  FUNCTION_ENTRY("Apply_Features")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == FEATURE_APPLICATION_DATA_TAG ) && ( ObjectSize == sizeof( Feature_Application_Data ) ) );

#else

  if ( ( ObjectTag != FEATURE_APPLICATION_DATA_TAG ) || ( ObjectSize != sizeof( Feature_Application_Data ) ) )
  {

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Apply_Features")

    return;

  }

#endif

#endif

  /* Since we have a valid FeatureRecord, lets apply it to the Partition_List in the ParameterData. */

  /* Extract the Feature ID data from the function table in the FeatureRecord. */
  Feature_ID = FeatureRecord->Function_Table->Feature_ID;

  /* Does the numeric ID of this feature match the ID of the feature we are to stop at? */
  if ( ( ParameterData->Feature_To_Stop_At != 0 ) &&
       ( Feature_ID->ID == ParameterData->Feature_To_Stop_At)
     )
  {

    /* We have reached the feature we are to stop at!  Abort any further processing. */
    *Error = DLIST_SEARCH_COMPLETE;

    FUNCTION_EXIT("Apply_Features")

    return;

  }

  /* Now, based upon the current user interface type, call the Create entry point for the feature passing it the
     correct user interface routine.                                                                             */
  switch ( Interface_In_Use )
  {
    case PM_Interface : /* Use the PM Interface routine provided by the feature. */
                        FeatureRecord->Function_Table->Create( ParameterData->Partition_List,
                                                               ParameterData->VolumeRecord,
                                                               FeatureRecord->Init_Data,
                                                               Feature_ID->Interface_Support[PM_Interface].VIO_PM_Calls.Create_and_Configure,
                                                               FeatureRecord->Actual_Class,
                                                               FeatureRecord->Top_Of_Class,
                                                               FeatureRecord->Feature_Sequence_Number,
                                                               ParameterData->Error_Code);
                        break;
    case VIO_Interface : /* Use the VIO Interface routine provided by the feature. */
                         FeatureRecord->Function_Table->Create( ParameterData->Partition_List,
                                                                ParameterData->VolumeRecord,
                                                                FeatureRecord->Init_Data,
                                                                Feature_ID->Interface_Support[VIO_Interface].VIO_PM_Calls.Create_and_Configure,
                                                                FeatureRecord->Actual_Class,
                                                                FeatureRecord->Top_Of_Class,
                                                                FeatureRecord->Feature_Sequence_Number,
                                                                ParameterData->Error_Code);
                         break;

    case Java_Interface : /* We must do a call-back to Java. */

                          /* Set the class name to use for the callback. */
                          Java_Call_Back_Class = Feature_ID->Interface_Support[Java_Interface].Java_Interface_Class;

                          /* Call the feature's create function. */
                          FeatureRecord->Function_Table->Create( ParameterData->Partition_List,
                                                                 ParameterData->VolumeRecord,
                                                                 FeatureRecord->Init_Data,
                                                                 &Java_Pass_Thru,
                                                                 FeatureRecord->Actual_Class,
                                                                 FeatureRecord->Top_Of_Class,
                                                                 FeatureRecord->Feature_Sequence_Number,
                                                                 ParameterData->Error_Code);
                          break;

    default : /* Unknown user interface type!  Something is very wrong as this should not be possible! */
              *Error = DLIST_CORRUPTED;
              *(ParameterData->Error_Code) = LVM_ENGINE_INTERNAL_ERROR;

              FUNCTION_EXIT("Apply_Features")

              return;

              break; /* Keep the compiler happy. */
  }


  /* Did we succeed? */
  if ( *(ParameterData->Error_Code) != LVM_ENGINE_NO_ERROR )
  {

    /* We must abort before processing any other features. */
    *Error = DLIST_SEARCH_COMPLETE;

  }
  else
    *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Apply_Features")

  return;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Java_Pass_Thru( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code)
{

  Java_Call_Back(Java_Call_Back_Class, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, Error_Code);

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Build_Features_List(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  /* Delcare a local variable so that we can access our parameters without having to typecase each time. */
  DLIST                      Features_List = (DLIST) Parameters;

  /* Declare a local variable to ease manipulation of the Feature Data context chain. */
  Feature_Context_Data *     Feature_Data;
  Feature_Context_Data *     Original_Feature_Data;

  /* Declare a local variable for use in accessing the function table of each feature. */
  Plugin_Function_Table_V1 * Function_Table;

  /* Declare a local variable for use when adding a feature to the Features_List. */
  Feature_Application_Data   Feature_App_Data;

  /* Declare a variable for loop control while looping through the feature data context chain. */
  BOOLEAN                   Last_Feature_Processed = FALSE;

  /* Declare a variable for use when searching the Features_List. */
  Feature_Search_Data       Search_Data;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;

  FUNCTION_ENTRY("Build_Features_List")


#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof( Partition_Data ) ) );

#else

  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Build_Features_List")

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Build_Features_List")

    return;

  }

#endif

#endif

  /* We will follow the feature data context chain for this partition.  Since this function should only be called
     for LVM Volumes, every partition it examines should have feature context data.  If it does not, then we have
     a serious internal error of some sort.                                                                        */

  if ( PartitionRecord->Feature_Data == NULL )
  {

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Build_Features_List")

    return;

  }

  /* Get the feature data. */
  Feature_Data = PartitionRecord->Feature_Data;
  Original_Feature_Data = Feature_Data;

  /* Process the feature data chain for this partition/aggregate. */
  do
  {

    /* Does the current feature appear in our list of features? */
    Search_Data.Feature_ID = Feature_Data->Feature_ID->ID;
    Search_Data.Found = FALSE;
    ForEachItem(Features_List,&Find_Existing_Feature, &Search_Data, TRUE, Error);
    if ( *Error != DLIST_SUCCESS )
    {

      FUNCTION_EXIT("Build_Features_List")

      /* This should not happen!  Abort! */
      return;

    }

    /* If our current feature is not in the list of features, then add it. */
    if ( ! Search_Data.Found )
    {

      /* Get its current class data. */
      Function_Table = Feature_Data->Function_Table;
      Feature_App_Data.Function_Table = Feature_Data->Function_Table;
      PartitionRecord->Feature_Data = Feature_Data;
      Feature_App_Data.Init_Data = NULL;
      Function_Table->ReturnCurrentClass(PartitionRecord, &Feature_App_Data.Actual_Class, &Feature_App_Data.Top_Of_Class, &Feature_App_Data.Feature_Sequence_Number );

      /* Restore the feature data for the partition. */
      PartitionRecord->Feature_Data = Original_Feature_Data;

      /* Add the feature to the list of features. */
      InsertItem(Features_List, sizeof( Feature_Application_Data ), &Feature_App_Data, FEATURE_APPLICATION_DATA_TAG, NULL, AppendToList, FALSE, Error );
      if ( *Error != DLIST_SUCCESS )
      {

        FUNCTION_EXIT("Build_Features_List")

        return;

      }

    }

    /* Have we processed the last feature on this aggregate/partition? */
    if ( Feature_Data->Old_Context == NULL)
      Last_Feature_Processed = TRUE;
    else
      Feature_Data = Feature_Data->Old_Context;

  } while ( ! Last_Feature_Processed );


  /* Assume success at this point. */
  *Error = DLIST_SUCCESS;

  /* If this is an Aggregate, we must process all of its partitions also! */
  if ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
  {

    /* Now we must find the partitions list for the aggregate. */
    Current_Context = PartitionRecord->Feature_Data;

    /* The only feature's which will have a partitions list are Aggregators.  There can be only
       one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
    while ( Current_Context->Partitions == NULL )
    {

      /* Get the next context in the feature context chain for this aggregate. */
      Current_Context = Current_Context->Old_Context;

    }

    /* Process each partition in the list of partitions belonging to this aggregate. */
    ForEachItem( Current_Context->Partitions, &Build_Features_List, Parameters, TRUE, Error);

  }


  FUNCTION_EXIT("Build_Features_List")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Volume_Features                              */
/*                                                                   */
/*   Descriptive Name: Returns the feature ID information for each of*/
/*                     the features that are installed on the        */
/*                     volume.                                       */
/*                                                                   */
/*   Input: Volume_Data * VolumeRecord - The volume to use.          */
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
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  Memory is allocated using the LVM Engine's memory*/
/*                  manager for the array of Feature_ID_Data items   */
/*                  being returned.                                  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Feature_Information_Array _System Get_Volume_Features( Volume_Data * VolumeRecord, CARDINAL32 * Error_Code )
{

  Partition_Data *              PartitionRecord;
  DLIST                         Features_List;
  Feature_Information_Array     Feature_Information;
  CARDINAL32                    Ignore_Error;

  FUNCTION_ENTRY("Get_Volume_Features")

  /* Initialize Feature_Information assuming failure. */
  Feature_Information.Count = 0;
  Feature_Information.Feature_Data = NULL;

  /* Has the Volume Manager been opened yet? */
  if ( Volumes == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    FUNCTION_EXIT("Get_Volume_Features")

    return Feature_Information;

  }

  /* Is this an LVM Volume? */
  if ( VolumeRecord->Compatibility_Volume )
  {

    /* This is NOT an LVM Volume! */
    *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

    FUNCTION_EXIT("Get_Volume_Features")

    return Feature_Information;

  }

  /* Extract the partition/aggregate. */
  PartitionRecord = VolumeRecord->Partition;

  /* We need to get a list of features for the volume we are processing. */
  Features_List = CreateList();
  if ( Features_List == NULL )
  {

    /* We are out of memory!  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Get_Volume_Features")

    return Feature_Information;

  }

  /* Get the features on the volume. */
  Build_Features_List( (ADDRESS) PartitionRecord, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, Features_List, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    /* Set the error code. */
    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Volume_Features")

    return Feature_Information;

  }

  /* Now that we have the features on the volume, sort them by their Feature Sequence Number.  This will ensure that
     they will be applied to the new partitions in the correct order.                                                 */
  SortList(Features_List, &Sort_By_Feature_Sequence_Number, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    /* Set the error code. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Volume_Features")

    return Feature_Information;

  }

  /* Now we can allocate the array for returning all of the features we found. */
  Feature_Information.Count = GetListSize(Features_List, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Volume_Features")

    return Feature_Information;

  }

  /* PassThru and BBR are always in the Available Features array but should not be reported. */
  if ( Feature_Information.Count > 2 )
  {

    /* Adjust the count to remove PassThru and BBR. */
    Feature_Information.Count -= 2;

    /* Allocate memory. */
    Feature_Information.Feature_Data = (Feature_ID_Data *) malloc( Feature_Information.Count * sizeof(Feature_ID_Data) );

    if ( Feature_Information.Feature_Data == NULL )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      /* We are out of memory! */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FUNCTION_EXIT("Get_Volume_Features")

      return Feature_Information;

    }

    /* Now we must fill in the array!.  */
    Feature_Information.Count = 0;
    ForEachItem(Features_List, &Load_Feature_Information_Array2 ,&Feature_Information, TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      /* This should not happen! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      free(Feature_Information.Feature_Data);

      Feature_Information.Feature_Data = NULL;

      FUNCTION_EXIT("Get_Volume_Features")

      return Feature_Information;

    }

  }
  else
    Feature_Information.Count = 0;

  /* Free the Features_List as we don't need it anymore. */
  DestroyList(&Features_List,FALSE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }
  else
  {

    /* Indicate success. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  FUNCTION_EXIT("Get_Volume_Features")

  return Feature_Information;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Issue_Volume_Feature_Command                     */
/*                                                                   */
/*   Descriptive Name: Issues a feature specific command to all of   */
/*                     features on a volume.                         */
/*                                                                   */
/*   Input: Volume_Data * VolumeRecord - The volume to use.          */
/*          CARDINAL32 Feature_ID - The numeric ID assigned to the   */
/*                                  feature which is to receive the  */
/*                                  command being issued.            */
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
/*   Output: If successful, a Feature_Information_Array structure is */
/*           returned with a non-zero Count.  Also, *Error_Code will */
/*           be set to LVM_ENGINE_NO_ERROR.  If an error occurs,     */
/*           then the Count field in the structure will be 0 and     */
/*           (*Error_Code) will contain a non-zero error code.       */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  Memory is allocated using the LVM Engine's memory*/
/*                  manager for the array of Feature_ID_Data items   */
/*                  being returned.                                  */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
 void _System Issue_Volume_Feature_Command( Volume_Data * VolumeRecord,
                                            CARDINAL32 Feature_ID,
                                            ADDRESS InputBuffer,
                                            CARDINAL32 InputSize,
                                            ADDRESS * OutputBuffer,
                                            CARDINAL32 * OutputSize,
                                            CARDINAL32 * Error_Code )
 {

   Partition_Data *              PartitionRecord;
   Plugin_Function_Table_V1 *    Function_Table;

   FUNCTION_ENTRY("Issue_Volume_Feature_Command")

   /* Has the Volume Manager been opened yet? */
   if ( Volumes == NULL )
   {

     /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
     *Error_Code = LVM_ENGINE_NOT_OPEN;

     FUNCTION_EXIT("Issue_Volume_Feature_Command")

     return;

   }

   /* Is this an LVM Volume? */
   if ( VolumeRecord->Compatibility_Volume )
   {

     /* This is NOT an LVM Volume! */
     *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

     FUNCTION_EXIT("Issue_Volume_Feature_Command")

     return;

   }

   /* Extract the partition/aggregate. */
   PartitionRecord = VolumeRecord->Partition;

   /* Extract the function table from the feature data. */
   Function_Table = (Plugin_Function_Table_V1 *) PartitionRecord->Feature_Data->Function_Table;

   /* Now call the PassThru function.  This is the PassThru function of the topmost feature on the volume.
      If the command is not meant for this feature, it will pass the command to the next feature down.  Eventually
      it will reach every feature on the volume.  This ensures that every instance of a feature on a volume will
      receive the command.                                                                                               */
   Function_Table->PassThru(Feature_ID, PartitionRecord, InputBuffer, InputSize, OutputBuffer, OutputSize, Error_Code);

   FUNCTION_EXIT("Issue_Volume_Feature_Command")

   return;

 }


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void Set_Volume_Parent_Child_Pointers( Volume_Data * VolumeRecord, CARDINAL32 * Error_Code)
{

  FUNCTION_ENTRY("Set_Volume_Parent_Child_Pointers")

  if ( VolumeRecord->Compatibility_Volume )
  {

    /* We have a compatibility volume.  This means that the volume has a single child which is
       a partition.  We must set the child's Parent_Handle to the external handle for the volume
       as the volume is its only parent.                                                          */
    VolumeRecord->Partition->Parent_Handle = VolumeRecord->External_Handle;

    /* All done! */
    *Error_Code = LVM_ENGINE_NO_ERROR;

    FUNCTION_EXIT("Set_Volume_Parent_Child_Pointers")

    return;

  }

  /* We have an LVM Volume.  This means that the "partition" associated with the volume could be an
     aggregate.  We will use the Update_Partitions_Parent_Data function from the Partition Manager to
     handle this as it knows what to do if the child is an aggregate.                                 */
  Update_Partitions_Parent_Data(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, VolumeRecord->External_Handle, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  FUNCTION_EXIT("Set_Volume_Parent_Child_Pointers")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Update_Parent_Child_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data * VolumeRecord = ( Volume_Data *) Object;

  FUNCTION_ENTRY("Update_Parent_Child_Data")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Update_Parent_Child_Data")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */
  Set_Volume_Parent_Child_Pointers( VolumeRecord, Error);

  /* The Set_Volume_Parent_Child_Pointers function should never fail!  If it does, then one of our data
     structures must be corrupted!                                                                       */
  if ( *Error != LVM_ENGINE_NO_ERROR )
    *Error = DLIST_CORRUPTED;
  else
    *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Update_Parent_Child_Data")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Pending_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data *               VolumeRecord = ( Volume_Data *) Object;

  /* The following variables are used to manipulate the partition(s) and features associated with the volume. */
  Partition_Data *            PartitionRecord;
  Plugin_Function_Table_V1  * FunctionTable;

  /* The following variable is used when setting the sequence numbers of partitions in a volume. */
  CARDINAL32                  Count;

  FUNCTION_ENTRY("Find_Pending_Changes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Find_Pending_Changes")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Is this an LVM Volume? */
  if ( ! VolumeRecord->Compatibility_Volume )
  {

    /* We must update the sequence numbers of the partitions in the volume.  This should NEVER fail! */
    Count = 1;
    Set_Partition_Sequence_Numbers(VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &Count, Error);
    if ( *Error != DLIST_SUCCESS )
    {

      *Error = DLIST_CORRUPTED;

      FUNCTION_EXIT("Find_Pending_Changes")

      return;

    }

    /* Get the partition/aggregate associated with the volume. */
    PartitionRecord = VolumeRecord->Partition;

    /* Get the function table for the features installed on the partition/aggregate. */
    FunctionTable = ( Plugin_Function_Table_V1 * ) PartitionRecord->Feature_Data->Function_Table;

    /* Now find out if any of the features have pending changes. */
    VolumeRecord->ChangesMade = FunctionTable->ChangesPending( PartitionRecord, Error);

  }
  else
    VolumeRecord->ChangesMade = FALSE;


  FUNCTION_EXIT("Find_Pending_Changes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Set_Partition_Sequence_Numbers(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  /* Delcare a local variable so that we can access our parameters without having to typecase each time. */
  CARDINAL32 *               Count = (CARDINAL32 * ) Parameters;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;


  FUNCTION_ENTRY("Set_Partition_Sequence_Numbers")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof( Partition_Data ) ) );

#else

  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Set_Partition_Sequence_Numbers")

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {

    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Set_Partition_Sequence_Numbers")

    return;

  }

#endif

#endif

  /* Assume success at this point. */
  *Error = DLIST_SUCCESS;

  /* If this is an Aggregate, we must process all of its partitions! */
  if ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
  {

    /* Now we must find the partitions list for the aggregate. */
    Current_Context = PartitionRecord->Feature_Data;

    /* The only feature's which will have a partitions list are Aggregators.  There can be only
       one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
    while ( Current_Context->Partitions == NULL )
    {

      /* Get the next context in the feature context chain for this aggregate. */
      Current_Context = Current_Context->Old_Context;

    }

    /* Process each partition in the list of partitions belonging to this aggregate. */
    ForEachItem( Current_Context->Partitions, &Set_Partition_Sequence_Numbers, Parameters, TRUE, Error);

  }
  else
  {

    /* This is a partition.  Set the sequence number, if appropriate. */
    if ( PartitionRecord->Signature_Sector->Sequence_Number != *Count )
    {

      PartitionRecord->Signature_Sector->Sequence_Number = *Count;
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

    /* Update the counter. */
    *Count += 1;

  }


  FUNCTION_EXIT("Set_Partition_Sequence_Numbers")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static INTEGER32     _System Filesystem_Sort( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error)
{

  /* Declare local variables so that we can access the Volume_Data objects without having to typecast each time. */
  Volume_Data *    Volume1 = (Volume_Data *) Object1;
  Volume_Data *    Volume2 = (Volume_Data *) Object2;

  FUNCTION_ENTRY("Filesystem_Sort")

#ifdef DEBUG

  /* Are Object1 and Object2 what we think it should be? */
  if ( ( Object1Tag != VOLUME_DATA_TAG ) || ( Object2Tag != VOLUME_DATA_TAG ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Filesystem_Sort")

    return 0;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  FUNCTION_EXIT("Filesystem_Sort")

  /* Is Volume1 < Volume2? */
  return strncmp(Volume1->File_System_Name, Volume2->File_System_Name, FILESYSTEM_NAME_SIZE);

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Determine_Expansion_Potential(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    Volume1 = (Volume_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecase each time. */
  Volume_Data **   Volume2 = (Volume_Data **) Parameters;

  FUNCTION_ENTRY("Determine_Expansion_Potential")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Determine_Expansion_Potential")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Is this the first volume we have seen? */
  if ( *Volume2 != NULL )
  {

    /* Does the Volume1 have the same filesystem as Volume2? */
    if ( strncmp(Volume1->File_System_Name, (*Volume2)->File_System_Name, FILESYSTEM_NAME_SIZE) == 0 )
    {

      /* Since Volume1 has the same filesystem as Volume2, we can just copy the values from Volume2 to Volume1. */
      Volume1->Filesystem_Is_Expandable = (*Volume2)->Filesystem_Is_Expandable;
      Volume1->Quiesce_Resume_Supported = (*Volume2)->Quiesce_Resume_Supported;

      FUNCTION_EXIT("Determine_Expansion_Potential")

      /* All done. */
      return;

    }

  }

  /* We must see if the filesystem on Volume1 supports expansion, quiesce, and resume. */

  /* If the filesystem on Volume1 is "unformatted" then it supports expansion, by definition.  It does not support quiesce and resume, though. */
  if ( strncmp(Volume1->File_System_Name, "unformatted", FILESYSTEM_NAME_SIZE) == 0 )
  {

    Volume1->Filesystem_Is_Expandable = TRUE;
    Volume1->Quiesce_Resume_Supported = FALSE;

    (*Volume2) = Volume1;

    FUNCTION_EXIT("Determine_Expansion_Potential")

    /* All done. */
    return;

  }

  FUNCTION_EXIT("Determine_Expansion_Potential")

  /* All done. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Expandable_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data *                     VolumeRecord = ( Volume_Data *) Object;

  /* The following variables are used to get the name of the filesystem that has attached itself to a volume. */
  BYTE                              QueryBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH) + 5]; /* Used with DosQueryFSAttach. */
  FSQBUFFER2 *                      QueryResult = (FSQBUFFER2 *) &QueryBuffer;              /* Used with DosQueryFSAttach. */
  char                              Drive_Name[3];                                          /* Used with DosQueryFSAttach. */
  CARDINAL32                        Buffer_Size;                                            /* Used with DosQueryFSAttach. */
  char *                            Filesystem_Name;                                        /* Used with DosQueryFSAttach. */
  APIRET                            Query_Filesystem_Result;

  /* The following variables are used when loading a DLL. */
#define ERROR_MESSAGE_SIZE 128
  APIRET                            ReturnCode;                        /* Used with the OS/2 APIs to load and access a DLL. */
  char                              ErrorMessage[ERROR_MESSAGE_SIZE];  /* Used with DosLoadModule to hold an error message if DosLoadModule fails. */
  CARDINAL32                        ExtraFileHandlesNeeded;            /* Used if we run out of file handles while trying to load a DLL. */
  CARDINAL32                        CurrentMaxFileHandles;             /* Used if we run out of file handles while trying to load a DLL. */

  /* The following variable is used to hold the entry points of interest in a filesystem expansion DLL. */
  LVM_Expansion_DLL_Record          Expansion_DLL_Data;

  /* The following variable is used to hold the handle of an Expansion_DLL_Data record once it has been put into the Filesystem_Expansion_DLLs list. */
  ADDRESS                           Expansion_DLL_Handle;

  FUNCTION_ENTRY("Find_Expandable_Volumes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Find_Expandable_Volumes")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Is the volume unformatted?  If so, then it can be expanded. */
  if ( strncmp(VolumeRecord->File_System_Name, "unformatted", FILESYSTEM_NAME_SIZE) == 0 )
  {

    /* This volume is unformatted, so it can be expanded! */
    VolumeRecord->Filesystem_Is_Expandable = TRUE;
    VolumeRecord->Quiesce_Resume_Supported = FALSE;

    FUNCTION_EXIT("Find_Expandable_Volumes")

    return;

  }

  /* Is this an LVM Volume with an active drive letter assignment? */
  if ( ( ! VolumeRecord->Compatibility_Volume ) &&
       ( VolumeRecord->Initial_Drive_Letter >= 'C' ) &&
       ( VolumeRecord->Initial_Drive_Letter <= 'Z' )
     )
  {

    /* Lets see if a filesystem is attached to the initial drive letter for this volume.*/
    Drive_Name[0] = VolumeRecord->Initial_Drive_Letter;
    Drive_Name[1] = ':';
    Drive_Name[2] = 0x00;
    Buffer_Size = sizeof(FSQBUFFER2) + (3 * CCHMAXPATH) + 5;

    /* Find out what is associated with the drive letter. */
    Query_Filesystem_Result = DosQueryFSAttach(Drive_Name,0,FSAIL_QUERYNAME , QueryResult, &Buffer_Size);

    /* Did the query fail? */
    if ( Query_Filesystem_Result == NO_ERROR )
    {

      /* Extract the filesystem name from the query result. */
      Filesystem_Name = (char *) ( (CARDINAL32) &(QueryResult->szName) + QueryResult->cbName + 1);

      /* Now we must create the expansion utility dll name. */

      /* Prepare the Expansion_DLL_Data. */
      memset(&Expansion_DLL_Data,0,sizeof(LVM_Expansion_DLL_Record) );

      /* Make the name. */
      Expansion_DLL_Data.Expansion_DLL_Name[0] = 'e';
      strncat(Expansion_DLL_Data.Expansion_DLL_Name,Filesystem_Name, CCHMAXPATH);
      strncat(Expansion_DLL_Data.Expansion_DLL_Name,".dll", CCHMAXPATH);

      /* Do we already have a DLL loaded by that name? */
      VolumeRecord->Expansion_DLL_Data = &Expansion_DLL_Data;
      ForEachItem(Filesystem_Expansion_DLLs, &Find_Expansion_DLL, &(VolumeRecord->Expansion_DLL_Data), TRUE, Error);
      if ( *Error != DLIST_SUCCESS )
      {

        FUNCTION_EXIT("Find_Expandable_Volumes")

        return;

      }

      /* Did we find a matching DLL? */
      if ( VolumeRecord->Expansion_DLL_Data != &Expansion_DLL_Data )
      {

        /* We have already loaded the DLL for the filesystem on this volume.  Lets set the volume's expansion flags. */
        VolumeRecord->Filesystem_Is_Expandable = TRUE;

        if ( ( VolumeRecord->Expansion_DLL_Data->Quiesce != NULL ) && ( VolumeRecord->Expansion_DLL_Data->Resume != NULL ) )
          VolumeRecord->Quiesce_Resume_Supported = TRUE;
        else
          VolumeRecord->Quiesce_Resume_Supported = FALSE;

        FUNCTION_EXIT("Find_Expandable_Volumes")

        /* We are done here, so return. */
        return;

      }

      /* Since we do not have a matching DLL already loaded, we must now load this DLL. */
      do
      {

        /* Call DosLoadModule to load the DLL. */
        ReturnCode = DosLoadModule(&ErrorMessage[0], ERROR_MESSAGE_SIZE, Expansion_DLL_Data.Expansion_DLL_Name, &(Expansion_DLL_Data.Expansion_DLL_Handle) );

        /* Did we succeed? */
        switch ( ReturnCode )
        {

          case NO_ERROR : /* No error, so continue processing this DLL. */
                          break;
          case ERROR_TOO_MANY_OPEN_FILES : /* We need to increase the number of file handles available to this process. */
                                           ExtraFileHandlesNeeded = 10;
                                           ReturnCode = DosSetRelMaxFH( ( PLONG ) &ExtraFileHandlesNeeded,&CurrentMaxFileHandles);
                                           if (ReturnCode != NO_ERROR)
                                           {

                                             /* Something is wrong.  We must be out of memory or something like that. */
                                             *Error = DLIST_OUT_OF_MEMORY;

                                             FUNCTION_EXIT("Find_Expandable_Volumes")

                                             return;

                                           }
                                           else
                                             ReturnCode = ERROR_TOO_MANY_OPEN_FILES;  /* Restore the original value of ReturnCode. */

                                           break;
          case ERROR_NOT_ENOUGH_MEMORY : /* We are out of memory!  Abort. */
                                         *Error = DLIST_OUT_OF_MEMORY;

                                         FUNCTION_EXIT("Find_Expandable_Volumes")

                                         return;
                                         break;
          default: /* We can not use the current DLL.  Proceed to the next one. */
                   *Error = DLIST_SUCCESS;

                   FUNCTION_EXIT("Find_Expandable_Volumes")

                   return;

                   break;  /* Keep the compiler happy. */

        }

      } while ( ReturnCode != NO_ERROR );


      /* Get the entry point for extending the filesystem.  If this entry point does not exist, then this is not a filesystem expansion DLL! */
//EK
//debug
{ static char funcname[80]="extendfs32";
      ReturnCode = DosQueryProcAddr(Expansion_DLL_Data.Expansion_DLL_Handle, 0, funcname, ( PFN * ) &Expansion_DLL_Data.Quiesce );
//      ReturnCode = DosQueryProcAddr(Expansion_DLL_Data.Expansion_DLL_Handle, 0, "extendfs32", ( PFN * ) &Expansion_DLL_Data.Quiesce );
} 

      /* Did we succeed? */
      if ( ReturnCode != NO_ERROR )
      {

        /* The entry point was not found.  We will assume that this DLL is not a filesystem expansion module. */
        ReturnCode = DosFreeModule(Expansion_DLL_Data.Expansion_DLL_Handle);

        /* We will report success and move on to the next potential filesystem expansion module. */
        *Error = DLIST_SUCCESS;

        FUNCTION_EXIT("Find_Expandable_Volumes")

        return;

      }

      /* This filesystem supports expansion. */
      VolumeRecord->Filesystem_Is_Expandable = TRUE;

      /* Assume that Quiesce and Resume are supported until proven otherwise. */
      VolumeRecord->Quiesce_Resume_Supported = TRUE;

      /* Get the entry points for the two functions we need. */
      ReturnCode = DosQueryProcAddr(Expansion_DLL_Data.Expansion_DLL_Handle, 0, "Quiesce", ( PFN * ) &Expansion_DLL_Data.Quiesce );

      /* Did we succeed? */
      if ( ReturnCode != NO_ERROR )
      {

        /* The entry point was not found.  This filesystem must not support Quiesce and Resume, but it does support expandsion. */
        VolumeRecord->Quiesce_Resume_Supported = FALSE;

      }
      else
      {

        /* Get the next entry point we need. */
        ReturnCode = DosQueryProcAddr(Expansion_DLL_Data.Expansion_DLL_Handle, 0, "Resume", ( PFN * ) &Expansion_DLL_Data.Resume );

        /* Did we succeed? */
        if ( ReturnCode != NO_ERROR )
        {

          /* The entry point was not found.  We will assume that this DLL does not support Quiesce and Resume. */
          VolumeRecord->Quiesce_Resume_Supported = FALSE;

        }

      }

      /* Now add this DLL to our list of filesystem expansion DLLs. */
      Expansion_DLL_Handle = InsertItem(Filesystem_Expansion_DLLs, sizeof(LVM_Expansion_DLL_Record), &Expansion_DLL_Data, LVM_EXPANSION_DLL_TAG, NULL, AppendToList, FALSE, Error);
      if ( *Error != DLIST_SUCCESS)
      {

        /* Free the DLL. */
        ReturnCode = DosFreeModule(Expansion_DLL_Data.Expansion_DLL_Handle);

        FUNCTION_EXIT("Find_Expandable_Volumes")

        /* Return to caller.  *Error has already been set. */
        return;

      }

      /* Now we can fill in the necessary fields in the VolumeRecord. */
      VolumeRecord->Expansion_DLL_Data = GetObject(Filesystem_Expansion_DLLs, sizeof(LVM_Expansion_DLL_Record), LVM_EXPANSION_DLL_TAG, Expansion_DLL_Handle, FALSE, Error);
      if ( *Error != DLIST_SUCCESS)
      {

        /* Free the DLL. */
        ReturnCode = DosFreeModule(Expansion_DLL_Data.Expansion_DLL_Handle);

        FUNCTION_EXIT("Find_Expandable_Volumes")

        /* Return to caller.  *Error has already been set. */
        return;

      }

    }
    else
    {

      /* Since there is no filesystem attached, but the volume is formatted, we will not allow it to be expanded. */
      VolumeRecord->Filesystem_Is_Expandable = FALSE;
      VolumeRecord->Quiesce_Resume_Supported = FALSE;

    }

  }
  else
  {
    /* This is either a compatibility volume or a hidden volume. */
    VolumeRecord->Filesystem_Is_Expandable = FALSE;
    VolumeRecord->Quiesce_Resume_Supported = FALSE;
  }

  FUNCTION_EXIT("Find_Expandable_Volumes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_Expansion_DLL(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  LVM_Expansion_DLL_Record *   Current_DLL = (LVM_Expansion_DLL_Record *) Object;
  LVM_Expansion_DLL_Record **  DLL_To_Find = (LVM_Expansion_DLL_Record ** ) Parameters;

  FUNCTION_ENTRY("Find_Expansion_DLL")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != LVM_EXPANSION_DLL_TAG ) || ( ObjectSize != sizeof(LVM_Expansion_DLL_Record) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Find_Expansion_DLL")

    return;

  }

#endif

  /* Since the TAG and size are O.K., well will proceed. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does the DLL name of the current DLL Data match the name of the DLL we are looking for? */
  if ( strncmp(Current_DLL->Expansion_DLL_Name, (*DLL_To_Find)->Expansion_DLL_Name, CCHMAXPATH) == 0 )
  {

    /* We have found what we are looking for!  Save the address of this expansion dll record! */
    *(DLL_To_Find) = Current_DLL;

    /* Indicate that we found what we wanted and the search is over. */
    *Error = DLIST_SEARCH_COMPLETE;

  }

  FUNCTION_EXIT("Find_Expansion_DLL")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Find_And_Convert_LVM_Volumes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data *        VolumeRecord = ( Volume_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  CARDINAL32 *         Unconvertable_Volumes = ( CARDINAL32 *) Parameters;

  /* Declare a local variable to be used in manipulating the *Unconvertable_Volumes bitmap. */
  CARDINAL32           Drive_Letter_Mask;

  /* Declare a local variable to hold the list of features for the volume being processed. */
  DLIST                Features_List;

  /* Declare a local variable for use in processing the Features_List for the volume. */
  BOOLEAN              Unconvertable = FALSE;

  /* Declare a local variable for use on error paths. */
  CARDINAL32           Ignore_Error;

  FUNCTION_ENTRY("Find_And_Convert_LVM_Volumes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Find_And_Convert_LVM_Volumes")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Is this volume a compatibility volume?  If it is, then we don't need to convert it. */
  if ( VolumeRecord->Compatibility_Volume )
  {

    *Error = DLIST_SUCCESS;

    FUNCTION_EXIT("Find_And_Convert_LVM_Volumes")

    return;

  }

  /* We need to get a list of features for the volume we are processing so we can determine if the volume can be converted to LVM Version 1 format. */

  /* Create the Features_List. */
  Features_List = CreateList();
  if ( Features_List == NULL )
  {

    /* We are out of memory!  Abort. */
    *Error = DLIST_OUT_OF_MEMORY;

    FUNCTION_EXIT("Find_And_Convert_LVM_Volumes")

    return;

  }

  /* Get the features on the volume. */
  Build_Features_List( (ADDRESS) VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, Features_List, Error);
  if ( *Error != DLIST_SUCCESS )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    FUNCTION_EXIT("Find_And_Convert_LVM_Volumes")

    return;

  }

  /* Now we must determine if the volume can be converted.  To be converted, the volume must employ all three of the
     following features:  Drive Linking, BBR, and Pass Thru.                                                           */

  /* If Features_List has more than 3 items in it, then we know it can not be converted. */
  if ( ( GetListSize( Features_List, Error) == 3 ) && ( *Error == DLIST_SUCCESS ) )
  {

    /* Since the number of features is correct, we must scan the features to see if they are indeed drive linking, BBR, and Pass Thru. */
    ForEachItem(Features_List, &Check_Feature_List_For_Conversion, (ADDRESS) &Unconvertable, TRUE, Error);
    if ( *Error != DLIST_SUCCESS)
    {

      /* Free the Features_List using Ignore_Error so that the error code in *Error is preserved. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

    }
    else
    {

      /* Free the Features_List as we don't need it anymore. */
      DestroyList(&Features_List,FALSE, Error);

    }

  }
  else
    Unconvertable = TRUE;  /* The volume can not be converted - it has the wrong number of features or we hit an internal error! */


  if ( Unconvertable )
  {

    /* This volume can not be converted since it has the wrong number or type of features.  Add its drive letter to the bitmap
       representing volumes that could not be converted.                                                                       */
    if ( VolumeRecord->Current_Drive_Letter >= 'A' )
    {

      Drive_Letter_Mask = 0x1;

      Drive_Letter_Mask = Drive_Letter_Mask << ( VolumeRecord->Current_Drive_Letter - 'A' );

      *Unconvertable_Volumes = *Unconvertable_Volumes | Drive_Letter_Mask;

    }
    else
      *Unconvertable_Volumes = *Unconvertable_Volumes | HIDDEN_VOLUME_FAILURE_FLAG;

  }

  FUNCTION_EXIT("Find_And_Convert_LVM_Volumes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name:                                                  */
/*                                                                   */
/*   Descriptive Name:                                               */
/*                                                                   */
/*   Input:                                                          */
/*                                                                   */
/*   Output:                                                         */
/*                                                                   */
/*   Error Handling:                                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Check_Feature_List_For_Conversion(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{


  /* Declare a local variable so that we can access the Feature_Application_Data objects without having to typecast each time. */
  Feature_Application_Data *    Feature = (Feature_Application_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  BOOLEAN *                     Unconvertable = ( BOOLEAN *) Parameters;

  FUNCTION_ENTRY("Check_Feature_List_For_Conversion")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != FEATURE_APPLICATION_DATA_TAG ) || ( ObjectSize != sizeof( Feature_Application_Data ) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Unexpected object tag or object size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    /* We have a TAG or size which is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Check_Feature_List_For_Conversion")

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Is the current feature one of the ones that are allowed? */
  if ( ( Feature->Function_Table->Feature_ID->ID == PASS_THRU_FEATURE_ID ) ||
       ( Feature->Function_Table->Feature_ID->ID == BBR_FEATURE_ID ) ||
       ( Feature->Function_Table->Feature_ID->ID == DRIVE_LINKING_FEATURE_ID )
     )
  {

    FUNCTION_EXIT("Check_Feature_List_For_Conversion")

    /* Since the feature is allowed, we have nothing to do here. */
    return;

  }

  /* Since the feature was NOT allowed, we must end the list traversal and indicate that this volume could not be converted. */

  /* Indicate that the volume could not be converted. */
  *Unconvertable = TRUE;

  /* End the list traversal. */
  *Error = DLIST_SEARCH_COMPLETE;

  FUNCTION_EXIT("Check_Feature_List_For_Conversion")

  return;

}

