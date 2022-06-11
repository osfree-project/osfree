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
 * Module: Partition_Manager.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in Partition_Manager.C:
 *
 *
 * Description: Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *              Partition_Manager.H and Partition_Manager.C implement
 *              the partition management functions found in the
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
 *            /                                               \
 *    BootManager.C                                          Handle_Manager.C
 *
 * Notes: LVM Drive Letter Assignment Tables (DLA_Tables) appear on the
 *        last sector of each track containing a valid MBR or EBR.  Since
 *        partitions must be track aligned, any track containing an MBR or
 *        EBR will be almost all empty sectors.  We will grab the last
 *        of these empty sectors for our DLT_Tables.
 *
 */


/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#include <stdlib.h>   /* malloc, free */
#include <stdio.h>    /* sprintf */
#include <string.h>   /* strlen */

#define NEED_BYTE_DEFINED
#include "engine.h"   /* Included for access to the global types and variables. */
#include "lvm_gbls.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */

#include "dlist.h"    /*  */
#include "diskio.h"   /*  */


#include "lvm_cons.h"   /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */

#define NEED_BYTE_DEFINED
#include "lvm_intr.h"   /* */

#include "lvm_hand.h"    /* Create_Handle, Destroy_Handle, Translate_Handle */
#include "Partition_Manager.h" /* Included to let the compiler check the consistency of declarations in Partition_Manager.H and Partition_Manager.C. */
#include "Volume_Manager.h"    /* Get_Volume_Size */

#include "extboot.h"           /* struct Extended_Boot */

#include "crc.h"               /* INITIAL_CRC, CalculateCRC */

#include "mbr.h"               /* mbr */

#include "logging.h"

#ifdef DEBUG

#include <assert.h>   /* assert */

#endif


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define OEM_ID_OFFSET         0x3
#define OEM_NAME_LENGTH       7
#define NEW_MBR_NAME          "I13X"
#define NEW_MBR_NAME_OFFSET   0xd5
#define PARTITION_ACTIVE_FLAG 0X80


/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/
typedef struct _MBR_EBR_Build_Data {
                                     LBA        Extended_Partition_Start;
                                     LBA        Extended_Partition_End;
                                     LBA        EBR_Start;
                                     LBA        EBR_End;
                                     CARDINAL32 EBR_Size;
                                     LBA        Logical_Drive_Start;
                                     CARDINAL32 Logical_Drive_Size;
                                     BOOLEAN    Find_Start;
                                     BOOLEAN    New_MBR_Needed;
                                   } MBR_EBR_Build_Data;

typedef struct _Partition_Search_Record {
                                          CARDINAL32   Serial_Number;   /* input - The serial number to search for. */
                                          ADDRESS      Handle;          /* output - The handle of the matching partition. */
                                        } Partition_Search_Record;

typedef struct _Primary_Partition_Status {
                                           BOOLEAN    Active_Boot_Manager_Found;
                                           BOOLEAN    Non_Hidden_Primary_Found;
                                           BOOLEAN    Active_Primary_Found;
                                         } Primary_Partition_Status;

/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static Master_Boot_Record     MBR;                                    /* Used to do I/O to the MBR of a drive.  Used to read EBRs on a drive. */
static Master_Boot_Record     EBR;                                    /* Used to write to EBRs on a drive. */
static BOOLEAN                Partition_Table_Entry_Claimed[4];       /* Used when ordering entries in the MBR. */
static BOOLEAN                Partition_Table_Index_In_Use[4];        /* Used when ordering entries in the MBR. */
static BYTE                   DLA_Sector[BYTES_PER_SECTOR];           /* Used when examining a sector that may contain a DLA Table. */
static BYTE                   MBR_DLA_Sector[BYTES_PER_SECTOR];       /* Used to create/hold the DLA Table that corresponds to the MBR. */
static BYTE                   Boot_Sector[BYTES_PER_SECTOR];          /* Used to hold boot sectors that we are examining. */
static BYTE                   Fake_EBR[BYTES_PER_SECTOR];             /* Used to create "fake" EBRs for LVM partitions. */
static char                   OEM_Name[] = "IBM 20";                  /* The OEM Name to search for when attempting to determine if an MBR is really a Boot Sector in disguise. */
static char                   OEM_Name2[] = "IBM 4.50";               /* The OEM Name used by Aurora for boot sectors. */
static BOOLEAN                Partition_Manager_Initialized = FALSE;  /* Used to track whether or not the Partition_Manager has been initialized. */
static BOOLEAN                Avoid_CHS = FALSE;                      /* If TRUE, then all CHS vs. (size,offset) checking will be bypassed. */

#ifdef WIPE_BOOT_SECTOR

static BYTE                   Kill_Boot_Sector[BYTES_PER_SECTOR];     /* Used to overwrite the boot sector on newly created partitions. */

#endif



/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
static ADDRESS Find_And_Allocate_Partition( CARDINAL32         DriveArrayIndex, /* The index in the Drive Array to use. */
                                            LBA                StartingSector,  /* The LBA at which the partition starts. */
                                            CARDINAL32         Size,            /* The size of the partition in sectors. */
                                            Partition_Types    Partition_Type,  /* The type of the partition - Partition, or MBR_EBR */
                                            BOOLEAN            Primary_Partition,
                                            Partition_Record * Partition_Table_Entry,
                                            CARDINAL16         Partition_Table_Index,
                                            DLA_Entry  *       DLA_Table_Entry,
                                            BOOLEAN            Prior_Existing_Partition,   /* TRUE if this is an existing partition. */
                                            BOOLEAN            Migration_Needed,           /* TRUE if there exists a Boot Manager Alias Table entry. */
                                            CARDINAL32 *       Error_Code);

static ADDRESS Allocate_Partition_From_Free_Space( CARDINAL32         DriveArrayIndex, /* The index in the Drive Array to use. */
                                                   Partition_Data *   Free_Space,      /* The block of free space to allocate the partition from. */
                                                   LBA                StartingSector,  /* The LBA at which the partition starts. */
                                                   CARDINAL32         Size,            /* The size of the partition in sectors. */
                                                   Partition_Types    Partition_Type,  /* The type of the partition - Partition, or MBR_EBR */
                                                   BOOLEAN            Primary_Partition,
                                                   Partition_Record * Partition_Table_Entry,
                                                   CARDINAL16         Partition_Table_Index,
                                                   DLA_Entry  *       DLA_Table_Entry,
                                                   BOOLEAN            Prior_Existing_Partition,
                                                   BOOLEAN            Migration_Needed,
                                                   CARDINAL32 *       Error_Code);

static void Translate_Format_Indicator( Partition_Data * PartitionRecord );

static BOOLEAN DLA_Table_Is_Valid( CARDINAL32 DriveArrayIndex, BOOLEAN Processing_EBR );

static BOOLEAN Partition_Table_Entry_In_Use ( Partition_Record * Partition_Table_Entry );

static void _System Find_Partition(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static void _System Count_Eligible_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static void _System Transfer_Partition_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static void Coalesce_Free_Space( Partition_Data * Free_Space, CARDINAL32 * Error_Code);

static void Delete_EBR( Partition_Data * PartitionRecord, CARDINAL32 * Error_Code);

static void Create_MBR( void );

static void _System Update_Table_Entries( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static void _System Write_Changes( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static void Add_Extended_Partition_To_MBR(LBA Extended_Partition_Start, LBA Extended_Partition_End, CARDINAL32 Drive_Array_Index, CARDINAL32 * Error_Code);

static BOOLEAN Can_Be_Primary( Partition_Data * Free_Space, CARDINAL32 * Error_Code );

static BOOLEAN Can_Be_Non_Primary( Partition_Data * Free_Space, CARDINAL32 * Error_Code );

static void _System Find_Active_Primary( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static void _System Build_Features_List(Partition_Data * PartitionRecord, DLIST Features_List, CARDINAL32 * Error);



/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: Initialize_Partition_Manager                     */
/*                                                                   */
/*   Descriptive Name: Initializes the Partition Manager for use.    */
/*                     This function must be called before any other */
/*                     functions in this module are used.  Also, this*/
/*                     function assumes that the DriveArray has      */
/*                     already been created and initialized!         */
/*                                                                   */
/*   Input: BOOLEAN Ignore_CHS - Tells the partition manager to skip */
/*                               all partition record CHS consistency*/
/*                               checks.  This will allow drives with*/
/*                               CHS vs. (size, offset) address      */
/*                               problems to be processed as long as */
/*                               there are no other errors in the    */
/*                               drive's partitioning.               */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: *Error_Code will be set to LVM_ENGINE_NO_ERROR if this  */
/*           function succeeds.  *Error_Code will contain an error   */
/*           code otherwise.                                         */
/*                                                                   */
/*   Error Handling: This function will return an error code if an   */
/*                   error is detected.  It will restore the state   */
/*                   of the DriveArray to what it was before this    */
/*                   function is called.                             */
/*                                                                   */
/*   Side Effects: Entries in the DriveArray will have their         */
/*                 Partitions lists modified.                        */
/*                                                                   */
/*   Notes: The DriveArray must be created and fully initialized     */
/*          before this function is called.  The Handle Manager must */
/*          also be Initialized prior to calling this function.      */
/*                                                                   */
/*********************************************************************/
void Initialize_Partition_Manager( BOOLEAN Ignore_CHS, CARDINAL32 * Error_Code )
{

  CARDINAL32         Index;  /* Used to walk the DriveArray. */
  Partition_Data   * Data;   /* Used to create the initial free space entry in each Partitions list in the DriveArray. */


  FUNCTION_ENTRY("Initialize_Partition_Manager")

  /* We assume that the drive array is set up for us.  What we will do here is, for each entry in the drive array,
     we will place a single Partition_Data entry in its Partitions list.  This entry will represent the entrie drive
     as being free space.  Then, as partitions are discovered, created, or deleted, we will operate on the Partitions
     list using standard memory management algorithms which have been modified for our use here.                       */


#ifdef DEBUG

#ifdef PARANOID

  assert( ! Partition_Manager_Initialized);

#else

  /* Has the Partition Manager been initialized already? */
  if ( Partition_Manager_Initialized )
  {

    FUNCTION_EXIT("Initialize_Partition_Manager")

    /* This should not have happened!  We have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

#endif

#endif

  /* Lets start the initialization process. */

#ifdef WIPE_BOOT_SECTOR

  /* Initialize the Kill_Boot_Sector variable.  This is used to kill the boot sector on any newly created partitions. */
  memset(&Kill_Boot_Sector,0xf6,BYTES_PER_SECTOR);

#endif

  /* Walk the DriveArray. */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert( ListEmpty(DriveArray[Index].Partitions, Error_Code) && ( *Error_Code == DLIST_SUCCESS ) );

#else

    /* Check that the Partitions list for this drive is empty.  If it is not, then we have an internal error! */
    if ( ! ListEmpty(DriveArray[Index].Partitions, Error_Code) )
    {

      /* The Partitions list should have been empty!  Since it is not, we have some kind of internal error! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Initialize_Partition_Manager")

      return;

    }

#endif

#endif

    /* Allocate memory for a Partition_Data structure. */
    Data = ( Partition_Data * ) malloc( sizeof( Partition_Data ) );

    /* Null out the Partition_Data structure. */
    memset( Data, 0, sizeof(Partition_Data) );

    /* Initialize the fields in Data. */
    Data->Drive_Index = Index;
    Data->Partition_Size = DriveArray[Index].Drive_Size;
    Data->Usable_Size = DriveArray[Index].Drive_Size;
    Data->New_Partition = FALSE;          /* This entry is not "new" in the sense that it was created by the user during this LVM Engine session.  The entry we are
                                             creating is designed to anchor the memory management algorithms being used.  As such, it doesn't count as "new".        */
    Data->Partition_Type = FreeSpace;     /* Tell it like it is!  This Partition_Data structure represents free space! */
    Data->Spanned_Volume = FALSE;         /* This is not a volume! */


    /* We don't need to worry about the other fields as they only pertain to partitions, not free space. */

    /* Now place this record in the Partitions list for this DriveArray entry. */
    Data->Drive_Partition_Handle = InsertObject( DriveArray[Index].Partitions, sizeof(Partition_Data), (ADDRESS) Data, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code );

    /* Did the item make it into the list? */
    if ( *Error_Code != DLIST_SUCCESS )
    {

#ifdef DEBUG

#ifdef PARANOID

      assert ( *Error_Code == DLIST_OUT_OF_MEMORY );

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

#else

      /* We had an error!  Are we out of memory? */
      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      {

        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      }
      else
      {

        /* Since memory was not a problem, we must have an internal error! */
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      }

#endif

#else

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

#endif

      /* We must undo what we have thus far done to the DriveArray.  Calling Close_Partition_Manager will accomplish this. */
      Close_Partition_Manager();

      FUNCTION_EXIT("Initialize_Partition_Manager")

      return;

    }

    /* Now that we have successfully placed Data in the Partitions list for the current entry in the DriveArray, lets get a handle for it that
       can be used by those outside of the LVM Engine.                                                                                         */
    Data->External_Handle = Create_Handle( Data, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);

    /* Did we succeed? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
      {

        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

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

      /* We must undo what we have thus far done to the DriveArray.  Calling Close_Partition_Manager will accomplish this. */
      Close_Partition_Manager();

      FUNCTION_EXIT("Initialize_Partition_Manager")

      return;
    }

  } /* End of for loop. */

  /* Indicate that the Partition Manager has been initialized. */
  Partition_Manager_Initialized = TRUE;

  /* Enable/Disable CHS vs. (size,offset) consistency checking. */
  Avoid_CHS = Ignore_CHS;

  /* All done.  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Initialize_Partition_Manager")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Close_Partition_Manager                          */
/*                                                                   */
/*   Descriptive Name: This function closes the Partition Manager    */
/*                     and frees all memory that it is responsible   */
/*                     for managing.                                 */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: All memory controlled by this module is freed.    */
/*                                                                   */
/*   Notes:  This function assumes that the DriveArray is still      */
/*           valid!  If the DriveArray is not valid then a trap      */
/*           or exception may occur.                                 */
/*                                                                   */
/*********************************************************************/
void Close_Partition_Manager( void )
{

  CARDINAL32    Index;   /* Used to walk the DriveArray. */
  CARDINAL32    Error;   /* Used to hold the error return code from the DeleteAllItems function. */


  FUNCTION_ENTRY("Close_Partition_Manager")

  /* What we need to do here is free all of the Partition_Data entries which reside in the Partitions list of each entry in the DriveArray. */


  /* Walk the DriveArray. */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    /* Empty the list of Partitions for this entry in the DriveArray. */
    DeleteAllItems(DriveArray[Index].Partitions, TRUE, &Error);

    /* The DeleteAllItems call should not fail.  If it does, we are not going to worry about it since we are closing the Partition Manager anyway. */

#ifdef DEBUG

    assert(Error == DLIST_SUCCESS);

#endif

  }

  /* Indicate that the Partition Manager is closed. */
  Partition_Manager_Initialized = FALSE;

  /* Reset the Avoid_CHS flag. */
  Avoid_CHS = FALSE;

  /* All done. */

  FUNCTION_EXIT("Close_Partition_Manager")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Discover_Partitions                              */
/*                                                                   */
/*   Descriptive Name: This function walks the DriveArray, and, for  */
/*                     drive with an entry in the DriveArray, it     */
/*                     reads the partitioning information for the    */
/*                     drive and converts it into a Partition List.  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If successful, the *ErrorCode will be                   */
/*              LVM_ENGINE_NO_ERROR and each drive in the DriveArray */
/*              will have its Partitions list filled in.             */
/*           If an unrecoverable error is encountered, then          */
/*              *Error_Code will contain an error code.              */
/*                                                                   */
/*   Error Handling: This function will not return an error unless   */
/*                   none of the drives in the DriveArray could be   */
/*                   accessed successfully.  In any case, the        */
/*                   IO_Error and Corrupt fields in each entry in    */
/*                   the drive array will be set based upon whether  */
/*                   the drive could be accessed and whether the     */
/*                   partitioning information found (if any) was     */
/*                   valid.                                          */
/*                                                                   */
/*   Side Effects: Each disk drive with an entry in the DiskArray    */
/*                 will have its partitioning information read.      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Discover_Partitions( CARDINAL32 * Error_Code )
{

  CARDINAL32             Index;                     /* Used to walk the DriveArray. */
  CARDINAL32             MBR_EBR_LBA;               /* The LBA address of the MBR/EBR we are currently processing. */
  CARDINAL32             Next_EBR_LBA;              /* The LBA address of the next EBR to process. */
  CARDINAL32             Extended_Partition_LBA;    /* The LBA of the starting sector of an extended partition.  */
  CARDINAL32             Partition_Index;           /* Used to walk partition tables. */
  CARDINAL32             IO_Error_Count = 0;        /* Used to track how many drives have I/O errors when we try to access their partition information. */
  struct Extended_Boot * BSector;                   /* Used to access the OEM ID during the test for a Boot Sector. */
  CARDINAL32             OEM_Name_Length;           /* Used when accessing the OEM ID field during the test for a Boot Sector. */
  char *                 New_MBR_ID;                /* Used when testing for the new style MBR, which supports booting over the 1024 cylinder limit. */

  /* The following variables are used to access the DLA Table, if it exists. */
  DLA_Table_Sector *   DLA_Table = (DLA_Table_Sector *) &DLA_Sector;
  DLA_Entry *          DLA_Table_Entry;                              /* Used to point at a specific entry in the DLA_Table. */
  CARDINAL32           DLA_Table_Index;                              /* Used to walk the DLA Table. */

  /* The values in the partition table need to be interpreted.  They can not be used directly.  The following local variables are used to transform
     what is in the partition table to usable values.                                                                                                */
  CARDINAL32             Starting_Cylinder;
  CARDINAL32             Starting_Head;
  CARDINAL32             Starting_Sector;
  CARDINAL32             Ending_Cylinder;
  CARDINAL32             Ending_Head;
  CARDINAL32             Ending_Sector;

  /* The following local variables are used when converting the starting and ending CHS addresses of a partition into LBA values. */
  LBA                    Starting_CHS_LBA;
  LBA                    Ending_CHS_LBA;

  /* The following local variables are used to calculate the starting and ending LBA values for the partition based upon the partition offset. */
  LBA                    Starting_LBA;
  LBA                    Ending_LBA;

  CARDINAL32             Calculated_Size;           /* Used to check fields in a partition record for validity. */

  BOOLEAN                EBRs_Remain;               /* Used to control the MBR/EBR processing loop.  Set to TRUE as long as there are unprocessed EBRs on the current disk. */
  BOOLEAN                EBR_Found;                 /* Used to determine if more than one EBR is defined in an MBR/EBR. */
  BOOLEAN                EBR_Allocated;             /* Used to control when an EBR is added to the list of allocations against a drive. */
  BOOLEAN                Keep_Partition;            /* Used to identify partitions which are valid enough to keep and those which must be thrown away. */
  BOOLEAN                Is_Not_Boot_Sector;        /* Used to flag MBRs which are actually Boot Sectors. */
  BOOLEAN                Migration_Needed;          /* Used to indicate partitions which FDISK placed on the Boot Manager Menu. */

  /* The following local variables are used to migrate Boot Manager Alias information which is stored in EBRs.
     The information to be migrated is the Alias Name and whether or not the item is on the Boot Manager Menu.   */
  AliasTableEntry *    BootManagerAlias = (AliasTableEntry *) ( (CARDINAL32) &MBR + ALIAS_TABLE_OFFSET);
  DLA_Entry            Fake_DLA_Entry;

  /* The following variable is used when creating "fake" partitions for large floppy formatted PRMs. */
  Partition_Record     Fake_Partition_Table_Entry;
extern int LastErrorIOCTL;
extern int LastError;
  int i64;
  FUNCTION_ENTRY("Discover_Partitions")

#ifdef DEBUG

#ifdef PARANOID

  assert( Partition_Manager_Initialized );

#else

  /* Has the Partition Manager been initialized already? */
  if ( ! Partition_Manager_Initialized )
  {

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"Error: The Partition Manager has not yet been initialize!\n");
      Write_Log_Buffer();

    }

    /* This should not have happened!  We have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Discover_Partitions")

    return;

  }

#endif

#endif

  /* This is where the fun begin.  We will walk the DriveArray.  For each entry in the DriveArray, we will read in the MBR.  We will
     add each partition in the MBR to the Partitions list for this entry in the DriveArray.  If there is an EBR, we will follow the
     EBR chain adding any partitions found there to the Partitions list for this entry in the DriveArray.                             */

  for ( Index = 0; Index < DriveCount; Index++ )
  {

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"Beginning to process drive %d.", Index + 1);
      Write_Log_Buffer();
      sprintf(Log_Buffer,"\tGeometry.Cylinders %6d (decimal)", DriveArray[Index].Geometry.Cylinders);
      Write_Log_Buffer();
      sprintf(Log_Buffer,"\tGeometry.Heads     %6d (decimal)", DriveArray[Index].Geometry.Heads);
      Write_Log_Buffer();
      sprintf(Log_Buffer,"\tGeometry.Sectors   %6d (decimal)", DriveArray[Index].Geometry.Sectors);
      Write_Log_Buffer();

    }

    /* Set up to read the MBR.  The MBR is always LBA 0. */
    MBR_EBR_LBA = 0;

    /* We have no extended partitions yet, so initialize Extended_Partition_LBA to 0. */
    Extended_Partition_LBA = 0;

    /* Assume that there are no EBRs. */
    Next_EBR_LBA = 0;

    /* We will repeat the MBR/EBR processing code while there are MBR/EBRs to process. */
    do
    {

      /* Initialize our loop control variable so that we will exit the loop unless another EBR is found. */
      EBRs_Remain = FALSE;

      /* Initialize EBR_Found to FALSE.  Only 1 EBR may be defined in an MBR/EBR.  We will assume that no EBRs are defined in the
         MBR/EBR that we are about to process.                                                                                     */
      EBR_Found = FALSE;

      if ( Logging_Enabled )
      {

        sprintf(Log_Buffer,"Attempting to read the MBR.");
        Write_Log_Buffer();

      }

      /* Read the MBR/EBR and process it. */
      ReadSectors(Index + 1,  /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                  MBR_EBR_LBA,
                  1,          /* We only want the MBR/EBR. */
                  &MBR,       /* The Buffer to use. */
                  Error_Code);
/* Correct MBR.PartitionValues to fit DriveArray[Index].Geometry.Sectors EK */
  { int i;           
  
    i64 = 64;
//    if(DriveArray[Index].Geometry.Sectors >= 64)
//      i64 = DriveArray[Index].Geometry.Sectors+1;
/*      
    for(i=0; i<4; i++)
    {  if(MBR.Partition_Table[i].Starting_Sector > DriveArray[Index].Geometry.Sectors)
              MBR.Partition_Table[i].Starting_Sector = DriveArray[Index].Geometry.Sectors;
       if(MBR.Partition_Table[i].Ending_Sector > DriveArray[Index].Geometry.Sectors)
              MBR.Partition_Table[i].Ending_Sector = DriveArray[Index].Geometry.Sectors;
    }
*/    
  }
       DriveArray[Index].LastErrorIOCTL  = LastErrorIOCTL;  //EK
       DriveArray[Index].LastError = LastError;

      /* Was the read successful? */
      if ( *Error_Code != DISKIO_NO_ERROR )
      {

#ifdef DEBUG

#ifdef PARANOID

        /* Assume we have an I/O error. */

        assert( *Error_Code == DISKIO_READ_FAILED );

        /* We will mark this entry in the drive array as having had an I/O error.  We will also mark it as being corrupt as this will
           keep the engine from doing anything with it.  Corrupt drives are only allowed to delete partitions and volumes, and since
           this drive will have no partitions, marking it corrupt will prevent the engine from allowing anything to be done to it.    */
        DriveArray[Index].IO_Error = TRUE;
        DriveArray[Index].Corrupt = TRUE;
        DriveArray[Index].NonFatalCorrupt  = FALSE;

        /* If we were trying to read the MBR when the read operation failed, then this drive is unusable. */
        if ( MBR_EBR_LBA == 0 )
          DriveArray[Index].Unusable = TRUE;

        /* Increment our count of drives with I/O errors. */
        IO_Error_Count++;

        /* We must move on to the next entry in the DriveArray.  Ensure that EBRs_Remain is FALSE so that we will escape the do-while loop. */
        EBRs_Remain = FALSE;

        continue;

#else

        /* Was this an I/O error or something worse? */
        if ( *Error_Code == DISKIO_READ_FAILED )
        {

          /* We will mark this entry in the drive array as having had an I/O error.  We will also mark it as being corrupt as this will
             keep the engine from doing anything with it.  Corrupt drives are only allowed to delete partitions and volumes, and since
             this drive will have no partitions, marking it corrupt will prevent the engine from allowing anything to be done to it.    */
          DriveArray[Index].IO_Error = TRUE;
          DriveArray[Index].Corrupt = TRUE;

          /* If we were trying to read the MBR when the read operation failed, then this drive is unusable. */
          if ( MBR_EBR_LBA == 0 )
            DriveArray[Index].Unusable = TRUE;

          /* Increment our count of drives with I/O errors. */
          IO_Error_Count++;

          /* We must move on to the next entry in the DriveArray.  Ensure that EBRs_Remain is FALSE so that we will escape the do-while loop. */
          EBRs_Remain = FALSE;

          continue;

        }
        else
        {

          /* Since the problem was not an I/O error, then we must have some kind of internal error!  Abort. */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          /* Attempt to clean-up anything we may have done. */
          Close_Partition_Manager();

          FUNCTION_EXIT("Discover_Partitions")

          return;

        }

#endif

#else

        /* We must have had an I/O error. */

        if ( Logging_Enabled )
        {

          sprintf(Log_Buffer,"Unable to read an MBR/EBR!  Marking the drive corrupt.");
          Write_Log_Buffer();

        }

        /* We will mark this entry in the drive array as having had an I/O error.  We will also mark it as being corrupt as this will
           keep the engine from doing anything with it.  Corrupt drives are only allowed to delete partitions and volumes, and since
           this drive will have no partitions, marking it corrupt will prevent the engine from allowing anything to be done to it.    */
        DriveArray[Index].IO_Error = TRUE;
        DriveArray[Index].Corrupt = TRUE;
        DriveArray[Index].NonFatalCorrupt  = FALSE;

        /* If we were trying to read the MBR when the read operation failed, then this drive is unusable. */
        if ( MBR_EBR_LBA == 0 )
        {

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"We were unable to read the MBR, so the drive is being marked unusable!");
            Write_Log_Buffer();

          }

          DriveArray[Index].Unusable = TRUE;

        }

        /* Increment our count of drives with I/O errors. */
        IO_Error_Count++;

        /* We must move on to the next entry in the DriveArray.  Ensure that EBRs_Remain is FALSE so that we will escape the do-while loop. */
        EBRs_Remain = FALSE;

        continue;

#endif

      }


      /* We now have what we think is an MBR ( or EBR). */

      if ( Logging_Enabled )
      {

        sprintf(Log_Buffer,"We have what may be an MBR/EBR.  Getting the corresponding DLAT.");
        Write_Log_Buffer();

      }


      /* Read in the corresponding DLA Table. */
      ReadSectors(Index + 1,    /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                  MBR_EBR_LBA + DriveArray[Index].Geometry.Sectors - 1,
                  1,            /* We only want the DLA Table. */
                  &DLA_Sector,         /* The Buffer to use. */
                  Error_Code);

      /* Was the read successful? */
      if ( *Error_Code != DISKIO_NO_ERROR )
      {

        if ( Logging_Enabled )
        {

          sprintf(Log_Buffer,"Unable to read the DLAT!");
          Write_Log_Buffer();

        }

        /* Zero out the DLA_Sector so that we don't get confused by any data left over from a previous DLA Table. */
        memset(&DLA_Sector,0,BYTES_PER_SECTOR);

        /* We will mark this entry in the drive array as having had an I/O error.  */
        DriveArray[Index].IO_Error = TRUE;

        /* We will not worry about this I/O failure as the only data lost is for legacy volumes.  We can always reconstruct this information
           for LVM volumes as this information is stored in an LVM volume as well as in the DLA_Table.                                         */

#ifdef DEBUG

#ifdef PARANOID

        assert( *Error_Code == DISKIO_READ_FAILED );

#else

        /* Was this an I/O error or something worse? */
        if ( *Error_Code != DISKIO_READ_FAILED )
        {

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"ReadSectors failed with DISKIO error %d (decimal)", *Error_Code );
            Write_Log_Buffer();

          }

          /* Since the problem was not an I/O error, then we must have some kind of internal error!  Abort. */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          /* Attempt to clean-up anything we may have done. */
          Close_Partition_Manager();

          FUNCTION_EXIT("Discover_Partitions")

          return;

        }

#endif

#endif

      }

      /* Now we have what we hope is an MBR, and it corresponding DLA Table. */

      /* We will attempt to validate this as an MBR.  If it is an EBR, it will pass all of the tests for an MBR, so we just need
         to worry about validating this as an MBR.

         To validate this as an MBR, we have three possible situations to consider:  we have an MBR, there is no MBR,
         or we have a Boot Sector.  The reason we can have a Boot Sector is that our handling of removable media changed in Warp
         Fixpak 35 and Merlin Fixpak 6.  In these fixpaks, we added support for PRMs (Partitioned Removable Media).  PRMs are things
         like Zip drives, Jaz drives, EZFlyer, SparQ, and Syjet drives, which offer high capacity and near fixed disk drive performance.
         Prior to these fixpaks, these devices were supported using Large Floppy mode, in which they were treated as large floppy disks.
         They were even formatted as large floppy disks.  Drives formatted in this fashion do not have an MBR.  In its place they have
         a Boot Sector.  Thus, when we try to read the MBR from a drive, we can end up with an MBR, a Boot Sector, or garbage (No MBR).
         The fun part is that the Boot Sector uses the same signature value as the MBR!  This means that we will have to resort to
         some tricks to tell the two apart.                                                                                              */

      /* How can we distinguish between a corrupt MBR and a Boot Sector? or no MBR?  */

      /* If there is no signature, then we will say there is no MBR.

         If the signature is O.K., we have to decide if we have a boot sector or an MBR.  IBM is the only (known) company to use Large
         Floppy formatted media.  The Boot Sector used for Large Floppy formatted media contains an OEMID field at offset 0x3.  This
         field will contain "IBM 20" or "IBM 4.50" as the first seven characters if it is an OS/2 Large Floppy Boot Sector.  We can use
         this to decide if what we have is a Boot Sector or an MBR.

         If we think we have an MBR, we must decide if it is corrupted or not.  In order for an MBR to be valid, the entries in its
         partition table must meet the following conditions:

              not overlap
              not place partitions or parts thereof beyond the end of the drive
              Only have one extended partition, if any
              If an extended partition is declared, it must contain an EBR with the correct signature at the start of the extended partition
              agree with the drive's geometry

         If any of these are violated, then we will declare the MBR to be corrupt.  If all conditions are met, then we will declare the
         MBR to be valid.                                                                                                                      */

      /* Lets start. */

      /* Is the Signature valid? */
      if ( MBR.Signature == MBR_EBR_SIGNATURE )
      {

        /* We have a possible MBR/EBR! */

        if ( Logging_Enabled )
        {

          sprintf(Log_Buffer,"Signature Test Passed.");
          Write_Log_Buffer();

        }


        /* If we are looking at what we think is an MBR, then we must check for the possibility that this is a Boot Sector. */
        if ( MBR_EBR_LBA == 0 )
        {

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"Performing the Boot Sector test.");
            Write_Log_Buffer();

          }

          /* Is this really a Boot Sector instead of an MBR?  To find out, lets check for the Boot Sector signature "IBM " at
             offset OEM_ID_OFFSET in the MBR.  Also, this field is NOT NULL terminated!                                         */

          /* We will assume that we have a Boot Sector until proven otherwise. */
          Is_Not_Boot_Sector = FALSE;

          /* Establish access to the MBR, only as a structure instead of an array of BYTEs. */
          BSector = ( struct Extended_Boot * ) &MBR;

          /* Determine the length of the OEM_Name.  This is the maximum number of characters that we must compare in order to determine
             if we have a Boot Sector or not.                                                                                                */
          OEM_Name_Length = strlen( OEM_Name );

          /* Compare the characters in the OEM_Name with those at the target location within the MBR. */
          Is_Not_Boot_Sector =  strncmp(BSector->Boot_OEM, OEM_Name, OEM_Name_Length);

          /* If we did not find "IBM 20" then check for "IBM 4.50". */
          if ( Is_Not_Boot_Sector )
          {

            /* Determine the length of the OEM_Name.  This is the maximum number of characters that we must compare in order to determine
               if we have a Boot Sector or not.                                                                                                */
            OEM_Name_Length = strlen( OEM_Name2 );

            /* Compare the characters in the OEM_Name with those at the target location within the MBR. */
            Is_Not_Boot_Sector =  strncmp(BSector->Boot_OEM, OEM_Name2, OEM_Name_Length);

          }

          /* If we still think we have an MBR, we must check to see if we are on a PRM or not.  If we are on a PRM,
             then we may have a special case.  On PRMs, if there is an MBR but NO partitions are defined, then we
             treat it as if it was in Big Floppy mode.  We must check for this special case here.                   */
          if ( Is_Not_Boot_Sector && DriveArray[Index].Is_PRM )
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Testing for PRM with no partitions.");
              Write_Log_Buffer();

            }

            /* Set Is_Not_Boot_Sector to FALSE.  We will set it to TRUE again if we find any partitions. */
            Is_Not_Boot_Sector = FALSE;

            /* Now we will walk the partition table and see if it defines any partitions. */
            for ( Partition_Index = 0; Partition_Index < 4; Partition_Index++ )
            {

              /* A partition table entry which is not in use will be all 0s.  Another indicator that a partition table entry is not
                 in use is for its format indicator to be 0.  Is this entry in use?                                                  */
              if ( Partition_Table_Entry_In_Use( & ( MBR.Partition_Table[Partition_Index] ) )  && ( MBR.Partition_Table[Partition_Index].Format_Indicator != 0 ) )
              {

                /* Since we found a possible partition, we don't need to treat this as a Big Floppy PRM.  Indicate that we have an MBR and exit the loop. */
                Is_Not_Boot_Sector = TRUE;

                break;

              }

            }

          }

        }
        else
          Is_Not_Boot_Sector = TRUE;  /* We have an EBR, so it can't be a boot sector. */


        /* Was it a Boot Sector?  If it looks like a boot sector and we are on a PRM, then we will treat it as
           a boot sector.  Otherwise, if it looks like a boot sector but is on a fixed disk, we will treat it as
           an MBR in need of being upgraded.  Otherwise, we will accept it as an MBR.                              */
        if ( Is_Not_Boot_Sector || ( ! DriveArray[Index].Is_PRM ) )
        {

          /* We may actually have an MBR (or EBR)! */

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"Boot Sector test complete.  We have an MBR or EBR!");
            Write_Log_Buffer();

          }


          /* If we have an MBR, we must check to see if we need to upgrade it. */
          if ( MBR_EBR_LBA == 0 )
          {

            /* Lets see if we need to upgrade the MBR.  The new MBR allows booting from partitions which are beyond the 1024 cylinder limit
               if the BIOS on the machine supports the new INT13 extensions for large drives.                                                */

            /* Does the current drive have more than 1024 cylinders? */
            if ( DriveArray[Index].Geometry.Cylinders >= 1024 )
            {

              /* Lets see what we have for an MBR. */
              New_MBR_ID = (char *) ( ( (CARDINAL32) &MBR ) + NEW_MBR_NAME_OFFSET);

              /* Do we have the new MBR already? */
              if ( strncmp(NEW_MBR_NAME, New_MBR_ID, strlen(NEW_MBR_NAME) ) != 0)
              {

                if ( Logging_Enabled )
                {

                  sprintf(Log_Buffer,"The MBR needs to be upgraded!");
                  Write_Log_Buffer();

                }

                /* We want a new MBR here.  By setting Is_Not_Boot_Sector to false, the MBR will be flagged as bad and a new one will be created. */
                Is_Not_Boot_Sector = FALSE;

              }

            }

            /* Since this is an MBR, we will make an entry in the partitions list for it whether or not the entries it contains are valid. */

            /* Make an entry in the Partitions list for the MBR/EBR. */
            Find_And_Allocate_Partition( Index,                               /* The index in the Drive Array to use. */
                                         MBR_EBR_LBA,                         /* The LBA at which the partition starts.  In this case, the LBA of the MBR/EBR. */
                                         DriveArray[Index].Geometry.Sectors,  /* We will allocate a full track.  Since partitions must be track aligned, the next partition can not start until the next track anyway. */
                                         MBR_EBR,                             /* This is an MBR */
                                         FALSE,                               /* This does not count as a primary partition. */
                                         NULL,                                /* There is no partition table entry for an MBR. */
                                         NEW_PARTITION_TABLE_INDEX,
                                         NULL,                                /* There is no DLA table entry for an MBR. */
                                         Is_Not_Boot_Sector,                  /* If we thought that this was a boot sector, and we are not on a PRM, we just found another FDISK bug which we must fix by laying down a new MBR. */
                                         FALSE,                               /* This is not a partition, so there can be no alias table entry for it. */
                                         Error_Code);

            if ( *Error_Code != LVM_ENGINE_NO_ERROR )
            {

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"Find_And_Allocate_Partition failed while trying to allocate an MBR.\n     LVM error code %d (decimal)", *Error_Code);
                Write_Log_Buffer();

              }

              /* We have a problem.  Since Allocate_Existing_Paritition has already set *Error_Code, all we need to do is clean-up and return. */
              Close_Partition_Manager();

              FUNCTION_EXIT("Discover_Partitions")

              return;

            }

          }
          else
          {

            /* We will make an entry in the Partitions list for an EBR only if the entries in the EBR are valid.  This avoids the
               problem of, on corrupted disks, getting an EBR in the Partitions list which does not have a corresponding partition.
               If this happens, when we write the partition information back to the disk, we will write an EBR with no corresponding
               partition, thereby leaving the drive in a corrupted state.  Thus, we could never put the drive into a non-corrupt state
               and recover it.                                                                                                          */
            EBR_Allocated = FALSE;

          }


          /* If we detected what we thought was a Boot Sector, but we were not looking at a PRM, then we found either an old
             PC-DOS MBR or one of the invalid MBRs laid down by FDISK.  We will fix this by laying down a new MBR.  When we
             called Find_And_Allocate_Partition, we set the new partition flag to TRUE if we had a bad MBR.  This will cause
             a new MBR to be written to the disk when Commit_Partition_Changes is called, but only if the drive needing the
             new MBR has its ChangesMade flag set.  Thus, we must ensure that the ChangesMade flag is set if we need a new MBR. */
          if ( ! Is_Not_Boot_Sector )
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Marking the drive dirty to get the new MBR written to disk.");
              Write_Log_Buffer();

            }

            /* Set the ChangesMade flag so that our new MBR will be written to disk. */
            DriveArray[Index].ChangesMade = TRUE;

          }

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"Testing the DLAT table.");
            Write_Log_Buffer();

          }

          /* Do we have a valid DLA_Table? */
          if ( ! DLA_Table_Is_Valid( Index, (MBR_EBR_LBA != 0) ) )
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"The DLAT table is NOT valid.");
              Write_Log_Buffer();

            }

            /* Set the DLA_Table pointer to NULL.  This will let everyone know that the current DLA Table is not valid. */
            DLA_Table = NULL;

          }
          else
          {

            /* The DLA Table is valid.  Make sure that DLA_Table points to our DLA Sector buffer. */
            DLA_Table = (DLA_Table_Sector *) &DLA_Sector;

          }

          /* Now we must make an entry in the Partitions list for each valid partition in the MBR/EBR. */

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"Beginning to scan the partition table in the MBR or EBR.");
            Write_Log_Buffer();

          }

          /* Now we will walk the partition table making entries for each partition listed. */
          for ( Partition_Index = 0; Partition_Index < 4; Partition_Index++ )
          {

            /* Clear the Migration_Needed flag.  This should only be set for partitions which have a Boot Manager Alias created by FDISK. */
            Migration_Needed = FALSE;

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Examining index %d.", Partition_Index );
              Write_Log_Buffer();

            }

            /* A partition table entry which is not in use will be all 0s.  Another indicator that a partition table entry is not
               in use is for its format indicator to be 0.  Is this entry in use?                                                  */
            if ( Partition_Table_Entry_In_Use( & ( MBR.Partition_Table[Partition_Index] ) )  && ( MBR.Partition_Table[Partition_Index].Format_Indicator != 0 ) )
            {

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"The partition table entry is in use.  Attempting to validate.");
                Write_Log_Buffer();

              }

              /* Assume we will keep this partition. */
              Keep_Partition = TRUE;

              /* Translate the fields in the partition table entry so that we can make use of them. */

              /* The fields in the partition table that represent the CHS address of the partition are all 8 bits.  However,
                 some of the values used for CHS are not 8 bits.  Cylinders, for instance, is 10 bits.  Sector is only 6 bits.
                 Thus, to squeeze everything in, the top two bits of the 10 bit Cylinder value are stored as the top two bits
                 of the 8 bit field in which the sector value is stored.  We must extract these two bits from the 8 bit field
                 in which the sector value is stored and correctly adjust our Cylinder value with them.  Similarly, we must
                 only use the low order 6 bits of the sector field for the actual sector value.                                 */

              /* Extract the starting CHS address of the partition from the partition record. */
              Starting_Head = MBR.Partition_Table[Partition_Index].Starting_Head;
              Starting_Sector = MBR.Partition_Table[Partition_Index].Starting_Sector%i64; // % 64;  /* Extract the low order 6 bits. */
//            Starting_Cylinder = MBR.Partition_Table[Partition_Index].Starting_Cylinder + ( ( MBR.Partition_Table[Partition_Index].Starting_Sector / 64 ) * 256);
              Starting_Cylinder = MBR.Partition_Table[Partition_Index].Starting_Cylinder + ( ( MBR.Partition_Table[Partition_Index].Starting_Sector / i64 ) * 256);

              /* Extract the ending CHS address of the partition from the partition record. */
              Ending_Head = MBR.Partition_Table[Partition_Index].Ending_Head;
              Ending_Sector = MBR.Partition_Table[Partition_Index].Ending_Sector%i64; // % 64;  /* Extract the low order 6 bits. */
//            Ending_Cylinder = MBR.Partition_Table[Partition_Index].Ending_Cylinder + ( ( MBR.Partition_Table[Partition_Index].Ending_Sector / 64 ) * 256);
              Ending_Cylinder = MBR.Partition_Table[Partition_Index].Ending_Cylinder + ( ( MBR.Partition_Table[Partition_Index].Ending_Sector / i64 ) * 256);

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"The CHS values for this partition are:\n     Starting Head %d (decimal)\n     Starting_Sector %d (decimal)\n     Starting_Cylinder %d (decimal)", Starting_Head, Starting_Sector, Starting_Cylinder);
                Write_Log_Buffer();

                sprintf(Log_Buffer,"     Ending Head %d (decimal)\n     Ending Sector %d (decimal)\n     Ending Cylinder %d (decimal)", Ending_Head, Ending_Sector, Ending_Cylinder);
                Write_Log_Buffer();

              }

              /* Now that we have the correct CHS addresses for the partition, lets calculate their equivalent LBA values.

                 To convert a CHS into an LBA, use the formula:

                     LBA = (Cylinder * Sectors Per Cylinder) + (Head * sectors per track) + (Sector - 1)

                 The (Sector - 1) comes from the fact that LBA is 0 based whereas CHS is not, at least for the Sector portion of the address. */
              Starting_CHS_LBA = (Starting_Cylinder * DriveArray[Index].Sectors_Per_Cylinder) + (Starting_Head * DriveArray[Index].Geometry.Sectors) + (Starting_Sector - 1);
              Ending_CHS_LBA = (Ending_Cylinder * DriveArray[Index].Sectors_Per_Cylinder) + (Ending_Head * DriveArray[Index].Geometry.Sectors) + (Ending_Sector - 1);

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"The starting and ending LBA values, based on the CHS values, are:\n     Starting_CHS_LBA is %d (decimal)\n     Ending_CHS_LBA is %d (decimal)", Starting_CHS_LBA, Ending_CHS_LBA);
                Write_Log_Buffer();

              }

              /* On large drives or drives which do not support geometry translation, partitions above the 1024 cylinder limit will
                 have CHS values that are the same as the geometry values for the drive.  These partitions are accessed through their
                 LBA addresses, which can be calculated from the partition size and partition offset.  For these partitions, the only
                 checking that can be done is to try to put the partition into the Partitions list and see if it overlaps another
                 partition.  For partitions below the 1024 cylinder limit, there should be valid CHS values for the start and end of the
                 partition, in which case we can perform several checks to see if partition record is corrupt.                            */

              /* Calculate the Starting_LBA and Ending_LBA using the partition offset and partition size fields of the partition table entry. */
              if ( MBR_EBR_LBA == 0 )
              {

                /* Since MBR_EBR_LBA is 0, we must be processing the MBR. */
                Starting_LBA = MBR.Partition_Table[Partition_Index].Sector_Offset;

              }
              else
              {

                /* Since MBR_EBR_LBA > 0, we must be processing an EBR. */

                /* Is the EBR entry for a partition or another EBR? */
                if ( ( MBR.Partition_Table[Partition_Index].Format_Indicator == EBR_INDICATOR ) ||
                     ( MBR.Partition_Table[Partition_Index].Format_Indicator == WINDOZE_EBR_INDICATOR )
                   )
                {

#ifdef DEBUG

#ifdef PARANOID

                assert( Extended_Partition_LBA > 0);

#endif

#endif
                  /* This EBR is for another EBR!  This means that the offset specified is relative to the extended partition
                     in which this EBR resides.  Thus, we must add LBA of the extended partition to the offset of this EBR to
                     get the correct Starting_LBA for partition record we are processing.                                     */
                  Starting_LBA = Extended_Partition_LBA + MBR.Partition_Table[Partition_Index].Sector_Offset ;

                }
                else
                {

                  /* This EBR entry is for a partition.  This means that the offset specified in the partition table entry is
                     relative to the EBR we are processing.  To calculate the LBA of the starting sector of this partition, it
                     will be the LBA of the EBR we are processing plus the offset from this EBR to the partition.               */
                  Starting_LBA = MBR_EBR_LBA + MBR.Partition_Table[Partition_Index].Sector_Offset;

                }

              }

              /* Now calculate the Ending_LBA. */
              Ending_LBA = Starting_LBA + MBR.Partition_Table[Partition_Index].Sector_Count - 1;

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"The starting and ending LBA values, based upon the size and offset, are:\n     Starting LBA is %d (decimal)\n     Ending LBA is %d (decimal)", Starting_LBA, Ending_LBA);
                Write_Log_Buffer();

              }

              /* If the drive is large, then we could have a partition which exceeds the capacity of the CHS fields in the
                 partition table to describe.  In this case, or if we are specifically instructed to skip these checks,
                 we can not perform consistency checking on this partition table entry.                                      */
              if ( ( ! Avoid_CHS ) &&
                   ( Starting_CHS_LBA < ( DriveArray[Index].Cylinder_Limit - 1 ) ) &&
                   ( Ending_CHS_LBA   < ( DriveArray[Index].Cylinder_Limit - 1 ) )
                 )
              {

                if ( Logging_Enabled )
                {

                  sprintf(Log_Buffer,"Since the drive %i is below the 1024 cylinder limit (based upon the CHS values),\nconsistency checks will be performed on the partition's CHS and size-offset fields.",Index);
                  Write_Log_Buffer();

                }

                /* Now we can do some checking on this partition record to see if it is corrupt.  We will check:

                       Is the ending address > starting address?
                       Is the ending address on the drive?
                       Is the starting address on the drive?
                       Is the starting address track aligned?
                       Does the size field in the partition record agree with the starting and ending addresses of the partition?
                       Does the Sector_Offset field in the partition record agree with the starting address of the partition?

                   If any of these are incorrect, we may have a problem!

                   If any of these are incorrect, we will flag the current entry in the DriveArray as being corrupt!  However, if the
                   starting address < ending address, and both the starting and ending addresses are on the drive, we will try to
                   make an entry in the Partitions list for this partition.  If we can not make such an entry, then this partition
                   must overlap with one already defined, in which we will make no entry in the Partitions list for this partition.     */

                /* Is the ending address >= starting address?  */
                if ( Ending_CHS_LBA < Starting_CHS_LBA )
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The partitions Ending_CHS_LBA is less than its Starting_CHS_LBA!");
                    Write_Log_Buffer();

                  }

                  /* This partition table entry is bogus.  Throw it away. */
                  DriveArray[Index].Corrupt = TRUE;
                  Keep_Partition = FALSE;

                }

                /* Are the starting and ending addresses on the drive?  The maximum LBA for the drive is the size of the drive (in sectors) - 1. */
                if ( ( Starting_CHS_LBA >= DriveArray[Index].Drive_Size ) && ( Ending_CHS_LBA >= DriveArray[Index].Drive_Size ) )
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The partition does not fit on the disk!");
                    Write_Log_Buffer();

                  }

                  /* This partition table entry describes a partition which does not fit on the disk!  Ignore it. */
                  DriveArray[Index].Corrupt = TRUE;
                  Keep_Partition = FALSE;

                }

#ifdef UNIX_PARTITIONS_BAD

                /* Is the starting address track aligned?  If it is, then starting address modulo sectors per track will be 0. */
                if ( ( Starting_CHS_LBA % DriveArray[Index].Geometry.Sectors ) != 0 )
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The partition does not start on a track boundary!");
                    Write_Log_Buffer();

                  }

                  /* We may have a drive geometry problem here ( where the drive was partitioned by some other OS which did not use the
                     same drive geometry as we are.  We will mark the current DriveArray entry as being corrupt, but continue on because
                     we may still be able to correctly list the partitions for the user.                                                   */
                  DriveArray[Index].Corrupt = TRUE;

                }

#endif

#ifndef IGNORE_FDISK_BUGS

                if ( Keep_Partition )
                {
                  /* Does the size field agree with the starting and ending addresses of the partitions? */
                  Calculated_Size = Ending_CHS_LBA - Starting_CHS_LBA + 1;

                  if ( Calculated_Size != MBR.Partition_Table[Partition_Index].Sector_Count )
                  {
    if(DriveArray[Index].Geometry.Sectors >= 64)
    { ; //??
    } else {
                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The size field in the partition table entry does not agree with\n     the CHS values in the partition table entry!");
                      Write_Log_Buffer();

                    }

                    /* The size does not agree.  The partition table entry is corrupt!  We will mark the current DriveArray entry as being
                       corrupt, but continue on because we may still be able to correctly list the partitions for the user.                  */
                    DriveArray[Index].Corrupt = TRUE;
    }                

                  }

                  /* Does the Sector_Offset field agree with the starting address?  */

                  /* Are we processing an MBR?  We can tell by looking at the LBA of the MBR/EBR we are processing.  The MBR is always at LBA 0! */
                  if ( MBR_EBR_LBA == 0 )
                  {

                    /* We are processing the MBR, so the Sector_Offset should be equal to the LBA of the first sector of the partition.  */
                    if ( MBR.Partition_Table[Partition_Index].Sector_Offset  != Starting_CHS_LBA )
                    {

                      if ( Logging_Enabled )
                      {

                        sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The offset field in the partition table entry does not agree with\n     the CHS values in the partition table entry!");
                        Write_Log_Buffer();

                      }

                      /* The Sector_Offset does not agree.  The partition table entry is corrupt!  We will mark the current DriveArray entry as being
                         corrupt, but continue on because we may still be able to correctly list the partitions for the user.                          */
                      DriveArray[Index].Corrupt = TRUE;

                    }

                  }
                  else
                  {

                    /* We are processing an EBR, so the sector offset for this ebr entry depends upon whether or not the entry is for
                       a partition or another EBR.  If it is for a partition, then the sector offset is relative to the EBR we are processing.
                       If the EBR entry is for another EBR, then sector offset is relative to the start of the extended partition in
                       which the EBR resides.                                                                                                   */

                    /* Is the EBR entry for a partition or another EBR? */
                    if ( ( MBR.Partition_Table[Partition_Index].Format_Indicator == EBR_INDICATOR ) ||
                         ( MBR.Partition_Table[Partition_Index].Format_Indicator == WINDOZE_EBR_INDICATOR )
                       )
                    {

                      /* This EBR entry is for another EBR!  */
                      if ( MBR.Partition_Table[Partition_Index].Sector_Offset != ( Starting_CHS_LBA - Extended_Partition_LBA ) )
                      {

                        if ( Logging_Enabled )
                        {

                          sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The offset field in the partition table entry does not agree with\n     the CHS values in the partition table entry!");
                          Write_Log_Buffer();

                        }

                        /* The Sector_Offset does not agree.  The partition table entry is corrupt!  We will mark the current DriveArray entry as being
                           corrupt, but continue on because we may still be able to correctly list the partitions for the user.                          */
                        DriveArray[Index].Corrupt = TRUE;

                      }

                    }
                    else
                    {

                      /* This EBR entry is for a partition. */
                      if ( MBR.Partition_Table[Partition_Index].Sector_Offset != ( Starting_CHS_LBA - MBR_EBR_LBA ) )
                      {

                        if ( Logging_Enabled )
                        {

                          sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The offset field in the partition table entry does not agree with\n     the CHS values in the partition table entry!");
                          Write_Log_Buffer();

                        }

                        /* The Sector_Offset does not agree.  The partition table entry is corrupt!  We will mark the current DriveArray entry as being
                           corrupt, but continue on because we may still be able to correctly list the partitions for the user.                          */
                        DriveArray[Index].Corrupt = TRUE;

                      }

                    }

                  }

#ifdef DEBUG

#ifdef PARANOID

                  if ( DriveArray[Index].Corrupt != TRUE )
                  {

                    assert( ( Starting_LBA == Starting_CHS_LBA ) && ( Ending_LBA == Ending_CHS_LBA) );

                  }

#else

                  /* As a final check, do the Starting_LBA and Ending_LBA agree with the Starting_CHS_LBA and Ending_CHS_LBA?  This
                     is really a check on ourselves since, if the size and offset checks passed, then this should pass also.          */
                  if ( ( DriveArray[Index].Corrupt != TRUE ) &&
                       ( ( Starting_LBA != Starting_CHS_LBA ) || ( Ending_LBA != Ending_CHS_LBA) ) )
                  {

                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,
                              "Internal consistency check has failed!\n     Starting_LBA is %d (decimal)\n     Starting_CHS_LBA is %d (decimal)\n     Ending_LBA is %d (decimal)\n     Ending_CHS_LBA is %d (decimal)\n     The size and offset checks passed, so these should all agree!",
                              Starting_LBA,
                              Starting_CHS_LBA,
                              Ending_LBA,
                              Ending_CHS_LBA);
                      Write_Log_Buffer();

                    }

                    /* Our calculation of the Starting_LBA or Ending_LBA must be wrong! */
                    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                    FUNCTION_EXIT("Discover_Partitions")

                    return;

                  }

#endif

#endif

                }

#endif

              }
              else
              {

                /* We may not be able to check the CHS addresses for consistency, but lets see what we can do with the LBA addresses
                   calculated from the Size and Sector_Offset fields of the partition table.                                          */

                /* Is the ending address >= starting address?  */
                if ( Ending_LBA < Starting_LBA )
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The Ending_LBA is less than the Starting_LBA!");
                    Write_Log_Buffer();

                  }

                  /* This partition table entry is bogus.  Throw it away. */
                  DriveArray[Index].Corrupt = TRUE;
                  Keep_Partition = FALSE;

                }

                /* Are the starting and ending addresses on the drive?  The maximum LBA for the drive is the size of the drive (in sectors) - 1. */
                if ( ( Starting_LBA >= DriveArray[Index].Drive_Size ) && ( Ending_LBA >= DriveArray[Index].Drive_Size ) )
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The partition does not fit on the drive!");
                    Write_Log_Buffer();

                  }

                  /* This partition table entry describes a partition which does not fit on the disk!  Ignore it. */
                  DriveArray[Index].Corrupt = TRUE;
                  Keep_Partition = FALSE;

                }

              }


              if ( Keep_Partition )
              {

                /* We will assume that there is no DLA Table Entry for the current partition. */
                DLA_Table_Entry = NULL;

                /* Set up the fake DLA Entry. */
                memset(&Fake_DLA_Entry, 0, sizeof(DLA_Entry) );

                /* Is there a Boot Manager Alias for this partition? */
                if ( MBR_EBR_LBA != 0)
                {

                  /* We are processing a partition table entry from an EBR. */

                  /* Does this partition table entry define a partition or another EBR? */
                  if ( ( MBR.Partition_Table[Partition_Index].Format_Indicator != EBR_INDICATOR ) &&
                       ( MBR.Partition_Table[Partition_Index].Format_Indicator != WINDOZE_EBR_INDICATOR )
                     )
                  {

                    /* This partition table entry defines a partition.  Lets see if there is an active AliasTableEntry for this partition. */

                    /* There is only one partition defined per EBR, so there will be, at most, one alias table entry in use in an EBR.  This
                       will always be the first one, which we are already set up to point to.  See if it is active.                             */
                    if ( BootManagerAlias->On_Boot_Manager_Menu &&                                                     /* Is it bootable? */
                         ( strncmp(BootManagerAlias->Name, ALIAS_TABLE_ENTRY_MIGRATION_TEXT, ALIAS_NAME_SIZE) != 0 )   /* Has it been migrated already? */
                       )
                    {

                      /* Set the appropriate fields in the Fake DLA Table Entry. */
                      memset(&Fake_DLA_Entry, 0, sizeof(DLA_Entry) );
                      strncpy(Fake_DLA_Entry.Partition_Name,BootManagerAlias->Name,ALIAS_NAME_SIZE);
                      strncpy(Fake_DLA_Entry.Volume_Name,BootManagerAlias->Name,ALIAS_NAME_SIZE);

                      /* We must set the Partition_Size and Partition_Start fields to make this an acceptable, minimum DLA Table Entry. */
                      Fake_DLA_Entry.Partition_Size = MBR.Partition_Table[Partition_Index].Sector_Count;
                      Fake_DLA_Entry.Partition_Start = Starting_LBA;

                      /* Since there was a Boot Manager Alias for this partition, this partition must be migrated. */
                      Migration_Needed = TRUE;

                    }

                  }

                }

                /* Set up access to the corresponding DLA Table entry, if there is one. */
                if ( DLA_Table != NULL )
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Searching for a DLAT entry for this partition.");
                    Write_Log_Buffer();

                  }

                  /* There is a DLA Table. */

                  /* Search the DLA Table for an entry that matches our current partition. */
                  for ( DLA_Table_Index = 0; DLA_Table_Index < 4; DLA_Table_Index++)
                  {

                    /* Does the Starting_LBA for our partition match the Partition_Start field of the current entry in the DLA Table?
                       Does the partition size match that specified in the DLA Table?                                                  */
                    if ( ( DLA_Table->DLA_Array[DLA_Table_Index].Partition_Start == Starting_LBA ) &&
                         ( DLA_Table->DLA_Array[DLA_Table_Index].Partition_Size == MBR.Partition_Table[Partition_Index].Sector_Count )
                       )
                    {

                      /* We have a match.  We have found the DLA Table Entry that goes with this partition table entry. */

                      if ( Logging_Enabled )
                      {

                        sprintf(Log_Buffer,"DLAT entry found.");
                        Write_Log_Buffer();

                      }

                      /* Save the DLA Table Entry for this partition. */
                      DLA_Table_Entry = &(DLA_Table->DLA_Array[DLA_Table_Index]);

                      /* If we need to migrate this logical drive because it had a Boot Manager Alias, then we need to save some data. */
                      if ( Migration_Needed )
                      {

                        /* Does this partition have a name?  If not, use the one from the Fake DLA Entry. */
                        if ( DLA_Table_Entry->Partition_Name[0] == 0x0 )
                          strncpy(DLA_Table_Entry->Partition_Name, Fake_DLA_Entry.Partition_Name, PARTITION_NAME_SIZE);

                        /* Does this partition have a volume name?  If not, use the one from the Fake DLA Entry. */
                        if ( DLA_Table_Entry->Volume_Name[0] == 0x0 )
                          strncpy(DLA_Table_Entry->Volume_Name, Fake_DLA_Entry.Volume_Name, VOLUME_NAME_SIZE);

                      }

                      /* Now zero out the Partition_Start field so that this entry can not be selected by another partition.
                         This is being a little paranoid, but what the heck!                                                  */
                      DLA_Table->DLA_Array[DLA_Table_Index].Partition_Start = 0;

                      /* Exit the for loop.  We don't need to search anymore. */
                      break;

                    }

                  }

                }
                else
                  DLA_Table_Entry = &Fake_DLA_Entry;

                /* Make an entry in the Partitions list for this partition. */

                /* If this partition entry describes an EBR, we need to do a little more checking as the rules for partitioning state that
                   only a single EBR may be defined in any MBR/EBR.                                                                         */
                if ( ( MBR.Partition_Table[Partition_Index].Format_Indicator == EBR_INDICATOR ) ||
                     ( MBR.Partition_Table[Partition_Index].Format_Indicator == WINDOZE_EBR_INDICATOR )
                   )
                {

                  /* Is this the first EBR defined in the current MBR/EBR? */
                  if ( EBR_Found )
                  {

                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,"Error:  Too many extended partitions defined in the current MBR-EBR!");
                      Write_Log_Buffer();

                    }

                      /* We have already seen an extended partition!  Only 1 is allowed per MBR/EBR!  Mark this
                         DriveArray entry as being corrupt!                                                      */
                      DriveArray[Index].Corrupt = TRUE;

                      /* Discard this partition table entry and proceed to the next one. */
                      continue;

                  }
                  else
                  {

                    /* Indicate that we have found an EBR. */
                    EBR_Found = TRUE;

                    /* Is this the start of the extended partition, or is this EBR entry for something already in the extended
                       partition?  We can tell by looking at Extended_Partition_LBA.  If it is 0, then we have not yet found
                       the start of the extended partition and this EBR must be the start of the extended partition.  If
                       Extended_Partition_LBA is not 0, then we are already in the extended partition.                          */
                    if ( Extended_Partition_LBA == 0 )
                    {

                      /* We are at the start of the extended partition.  Save its LBA. */
                      Extended_Partition_LBA = Starting_LBA;

                    }

                  }

                  /* We will NOT make an entry in the Partitions list for this EBR.  The entry for this EBR will be made when
                     we process this EBR.                                                                                      */

                }
                else
                {

                  if ( Logging_Enabled )
                  {

                    sprintf(Log_Buffer,"Attempting to read the Boot Sector of the partition.");
                    Write_Log_Buffer();

                  }

                  /* Since we have a partition, we must read its boot sector into the Boot_Sector variable so that it can be
                     examined later.  This is used to determine whether or not a partition is formatted, or exactly what kind
                     of IFS is being used on a partition whose Format_Indicator is 7.                                           */
                  ReadSectors(Index + 1,    /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                              Starting_LBA,
                              1,            /* We only want the Boot Sector. */
                              &Boot_Sector,         /* The Buffer to use. */
                              Error_Code);

                  /* Was the read successful? */
                  if ( *Error_Code != DISKIO_NO_ERROR )
                  {

                    /* Zero out the Boot_Sector variable so that we don't get confused by any data left over from a previous DLA Table. */
                    memset(&Boot_Sector,0,BYTES_PER_SECTOR);

                    /* We will mark this entry in the drive array as having had an I/O error.  */
                    DriveArray[Index].IO_Error = TRUE;

                    /* We will not worry about this I/O failure as the data read would only be used to determine if the partition is formatted or not. */

#ifdef DEBUG

#ifdef PARANOID

                    assert( *Error_Code == DISKIO_READ_FAILED );

#else

                    /* Was this an I/O error or something worse? */
                    if ( *Error_Code != DISKIO_READ_FAILED )
                    {

                      if ( Logging_Enabled )
                      {

                        sprintf(Log_Buffer,"ReadSectors failed with DISKIO error code %d (decimal)", *Error_Code);
                        Write_Log_Buffer();

                      }

                      /* Since the problem was not an I/O error, then we must have some kind of internal error!  Abort. */
                      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                      /* Attempt to clean-up anything we may have done. */
                      Close_Partition_Manager();

                      FUNCTION_EXIT("Discover_Partitions")

                      return;

                    }

#endif

#endif

                  }

                  /* Is this partition a primary partition?  If we are processing the MBR, then it is.  If we are processing an EBR, then
                     it is not.  We can use the MBR_EBR_LBA variable to determine whether or not we are processing an MBR.  MRB_EBR_LBA
                     will be 0 if we are processing an MBR since LBA 0 is always the address of the MBR.                                      */

                  if ( MBR_EBR_LBA == 0)
                  {

                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,"Adding a primary partition to the partitions list for drive %d.", Index + 1);
                      Write_Log_Buffer();

                    }

                    /* We are processing the MBR, so this partition is a primary partition. */

                    /* Make an entry in the Partitions list for this partition. */
                    Find_And_Allocate_Partition( Index,
                                                 Starting_LBA,
                                                 MBR.Partition_Table[Partition_Index].Sector_Count,
                                                 Partition,
                                                 TRUE,
                                                 &(MBR.Partition_Table[Partition_Index]),
                                                 Partition_Index,
                                                 DLA_Table_Entry,
                                                 TRUE,
                                                 FALSE,
                                                 Error_Code);

                    /* If there was no error, update our primary partition counter. */
                    if ( *Error_Code == LVM_ENGINE_NO_ERROR )
                    {

                      /* Increment the count of primary partitions for this drive. */
                      DriveArray[Index].Primary_Partition_Count++;

                      if ( Logging_Enabled )
                      {

                        sprintf(Log_Buffer,"There are now %d primary partitions on this drive.", DriveArray[Index].Primary_Partition_Count);
                        Write_Log_Buffer();

                      }


                    }

                  }
                  else
                  {

                    /* We are processing an EBR, so this is NOT a primary partition. */

                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,"Adding an EBR and a logical drive to the partitions list for drive %d.", Index + 1);
                      Write_Log_Buffer();

                    }

                    /* Have we put the EBR in the Partitions list yet? */
                    if ( ! EBR_Allocated )
                    {

                      /* Make an entry in the Partitions list for the MBR/EBR. */
                      Find_And_Allocate_Partition( Index,                               /* The index in the Drive Array to use. */
                                                   MBR_EBR_LBA,                         /* The LBA at which the partition starts.  In this case, the LBA of the MBR/EBR. */
                                                   DriveArray[Index].Geometry.Sectors,  /* We will allocate a full track.  Since partitions must be track aligned, the next partition can not start until the next track anyway. */
                                                   MBR_EBR,                             /* This is an MBR */
                                                   FALSE,                               /* This does not count as a primary partition. */
                                                   NULL,                                /* There is no partition table entry for an MBR. */
                                                   NEW_PARTITION_TABLE_INDEX,
                                                   NULL,                                /* There is no DLA table entry for an MBR. */
                                                   Is_Not_Boot_Sector,                  /* If we thought that this was a boot sector, and we are not on a PRM, we just found another FDISK bug which we must fix by laying down a new MBR. */
                                                   FALSE,
                                                   Error_Code);

                      if ( *Error_Code == LVM_ENGINE_NO_ERROR )
                      {

                        /* We need to ensure that a duplicate entry for the EBR is not added to the Partitions list.  Setting EBR_Allocated will prevent this. */
                        EBR_Allocated = TRUE;

                        /* Make an entry in the Partitions list for this partition. */
                        Find_And_Allocate_Partition( Index,
                                                     Starting_LBA,
                                                     MBR.Partition_Table[Partition_Index].Sector_Count,
                                                     Partition,
                                                     FALSE,
                                                     &(MBR.Partition_Table[Partition_Index]),
                                                     Partition_Index,
                                                     DLA_Table_Entry,
                                                     TRUE,
                                                     Migration_Needed,
                                                     Error_Code);

                      }

                    }
                    else
                    {

                      if ( Logging_Enabled )
                      {

                        sprintf(Log_Buffer,"Adding a logical drive to the partitions list for drive %d.", Index + 1);
                        Write_Log_Buffer();

                      }

                      /* Make an entry in the Partitions list for this partition. */
                      Find_And_Allocate_Partition( Index,
                                                   Starting_LBA,
                                                   MBR.Partition_Table[Partition_Index].Sector_Count,
                                                   Partition,
                                                   FALSE,
                                                   &(MBR.Partition_Table[Partition_Index]),
                                                   Partition_Index,
                                                   DLA_Table_Entry,
                                                   TRUE,
                                                   Migration_Needed,
                                                   Error_Code);

                    }

                    /* If there was no error, update our logical partition counter. */
                    if ( *Error_Code == LVM_ENGINE_NO_ERROR )
                    {

                      /* Increment the count of non-primary partitions for this drive. */
                      DriveArray[Index].Logical_Partition_Count++;

                      if ( Logging_Enabled )
                      {

                        sprintf(Log_Buffer,"There are now %d logical drives on the current drive.", DriveArray[Index].Logical_Partition_Count);
                        Write_Log_Buffer();

                      }

                    }

                  }

                }

                /* Was there an error? */
                if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                {

                  /* Was the error due to a shortage of memory, an internal error, or becuase this partition table entry is corrupt? */
                  if ( *Error_Code != LVM_ENGINE_BAD_PARTITION )
                  {

                    /* Something unexpected happened! */

#ifdef DEBUG

#ifdef PARANOID

                    assert(0);

#endif

#endif

                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,"Find_And_Allocate_Partition failed with LVM error code %d (decimal)", *Error_Code);
                      Write_Log_Buffer();

                    }

                    /* We need to clean up before we abort. */
                    Close_Partition_Manager();

                    FUNCTION_EXIT("Discover_Partitions")

                    /* Abort. */
                    return;

                  }
                  else
                  {

                    if ( Logging_Enabled )
                    {

                      sprintf(Log_Buffer,"Error:  Invalid Partition Table Entry!\n     The partition overlaps another partition!");
                      Write_Log_Buffer();

                    }

                    /* This partition table entry is bad.  Set the corrupt flag and discard this partition table entry. */
                    DriveArray[Index].Corrupt = TRUE;

                    /* If this partition table entry was an EBR, we must clear the EBRs_Remain flag as this EBR is not valid! */
                    if ( ( MBR.Partition_Table[Partition_Index].Format_Indicator == EBR_INDICATOR ) ||
                         ( MBR.Partition_Table[Partition_Index].Format_Indicator == WINDOZE_EBR_INDICATOR )
                       )
                    {

                      /* Clear the EBRs_Remain flag so that if no other valid EBRs are found, we will properly exit the do-while loop. */
                      EBRs_Remain = FALSE;

                      /* Clear the EBR_Found flag as this EBR was not valid. */
                      EBR_Found = FALSE;

                    }

                  }

                }
                else
                {

                  /* If this partition table describes an EBR, then we must do a little extra work here to ensure that this EBR will be processed. */
                  if ( ( MBR.Partition_Table[Partition_Index].Format_Indicator == EBR_INDICATOR ) ||
                       ( MBR.Partition_Table[Partition_Index].Format_Indicator == WINDOZE_EBR_INDICATOR )
                     )
                  {

                    /* Set the loop control variable so that we will return to process this EBR. */
                    EBRs_Remain = TRUE;

                    /* Save the starting LBA of the EBR so that we don't have to recalculate it later. */
                    Next_EBR_LBA = Starting_LBA;

                  }

                }

              } /* End of code for a partition table entry which is good enough to keep. */

            } /* End of code for a Partition Table entry which is in use. */

          } /* End of for loop to place valid partitions into the Partitions list. */

        } /* End of the processing code for MBRs which passed the Boot Sector test. */
        else
        {

          /* We have a Boot Sector! */

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"Boot Sector test complete.  We have a Boot Sector!");
            Write_Log_Buffer();

          }

          /* If we have a PRM here, then the PRM is in the "Big Floppy" format.  In this case, we will
             create a "fake" MBR for it, and we will create a "fake" partition for it.  Otherwise,
             the partitioning of the drive is corrupt!                                                  */
          if ( DriveArray[Index].Is_PRM )
          {

            /* We must create the "fake" MBR and partition for this PRM drive. */

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"We have a PRM in Big Floppy Format.  Creating a fake MBR and partition for it.");
              Write_Log_Buffer();

            }


            /* Make an entry in the Partitions list for the MBR/EBR. */
            Find_And_Allocate_Partition( Index,                               /* The index in the Drive Array to use. */
                                         0,                                   /* The LBA at which the partition starts.  In this case, the LBA of the MBR/EBR. */
                                         DriveArray[Index].Geometry.Sectors,  /* We will allocate a full track.  Since partitions must be track aligned, the next partition can not start until the next track anyway. */
                                         MBR_EBR,                             /* This is an MBR */
                                         FALSE,                               /* This does not count as a primary partition. */
                                         NULL,                                /* There is no partition table entry for an MBR. */
                                         NEW_PARTITION_TABLE_INDEX,
                                         NULL,                                /* There is no DLA table entry for an MBR. */
                                         FALSE,                               /* Set to FALSE so that, if the user changes from Big Floppy to PRM, a new MBR will be written. */
                                         FALSE,                               /* This is not a partition, so there can be no alias table entry for it. */
                                         Error_Code);

            if ( *Error_Code != LVM_ENGINE_NO_ERROR )
            {

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"Error:  Find_And_Allocate_Partition failed with LVM error %d (decimal)", *Error_Code);
                Write_Log_Buffer();

              }

              /* We have a problem.  Since Allocate_Existing_Paritition has already set *Error_Code, all we need to do is clean-up and return. */
              Close_Partition_Manager();

              FUNCTION_EXIT("Discover_Partitions")

              return;

            }

            /* In order to make a "fake" partition, we need a "fake" partition table entry for it. */
            memset( &Fake_Partition_Table_Entry, 0, sizeof( Partition_Record ) );
            Fake_Partition_Table_Entry.Boot_Indicator = 0x80;
            Fake_Partition_Table_Entry.Format_Indicator = FAT16_LARGE_PARTITION_INDICATOR;

            /* Now we must create a "fake" partition which occupies the rest of the disk. */
            Find_And_Allocate_Partition( Index,                               /* The index in the Drive Array to use. */
                                         DriveArray[Index].Geometry.Sectors,  /* The LBA of the first sector of the second track on the disk. */
                                         (DriveArray[Index].Geometry.Cylinders * DriveArray[Index].Geometry.Heads * DriveArray[Index].Geometry.Sectors) - DriveArray[Index].Geometry.Sectors,
                                         Partition,                           /* This is a partition */
                                         TRUE,                                /* This does counts as a primary partition. */
                                         &Fake_Partition_Table_Entry,         /* Use the "fake" partition table entry. */
                                         NEW_PARTITION_TABLE_INDEX,
                                         NULL,                                /* There is no DLA table entry for a "fake" partition like this one. */
                                         TRUE,
                                         FALSE,                               /* There is no alias table entry for this partition. */
                                         Error_Code);

            if ( *Error_Code != LVM_ENGINE_NO_ERROR )
            {

              if ( Logging_Enabled )
              {

                sprintf(Log_Buffer,"Find_And_Allocate_Partition failed with LVM error code %d (decimal)", *Error_Code);
                Write_Log_Buffer();

              }

              /* We have a problem.  Since Allocate_Existing_Paritition has already set *Error_Code, all we need to do is clean-up and return. */
              Close_Partition_Manager();

              FUNCTION_EXIT("Discover_Partitions")

              return;

            }

            /* Now we must mark the drive as being in big floppy mode, and we must clear the ChangesMade flag as we don't
               want these "fake" changes being written to the disk!                                                        */
            DriveArray[Index].Is_Big_Floppy = TRUE;
            DriveArray[Index].ChangesMade = FALSE;

            /* Now, since we have a "fake" primary partition, we must update the primary partition counter for the drive. */
            DriveArray[Index].Primary_Partition_Count = 1;

          }
          else
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Error:  A Boot Sector was found instead of an MBR on a fixed disk!\n     Marking the drive as being corrupt!");
              Write_Log_Buffer();

            }

            /* Mark the drive as having corrupt partition information. */
            DriveArray[Index].Corrupt = TRUE;

          }

        }

      } /* End of code for processing MBRs with correct signature. */
      else
      {

        /* If we are processing an EBR, then we have a corrupt EBR and we are done.  If we are processing an MBR, then we must check to see if we have a PRM. */

        /* If we have a PRM here, then the PRM is in the "Big Floppy" format.  In this case, we will
           create a "fake" MBR for it, and we will create a "fake" partition for it.  Otherwise,
           the partitioning of the drive is corrupt!                                                  */
        if ( DriveArray[Index].Is_PRM && ( MBR_EBR_LBA == 0 ) )
        {

          if ( Logging_Enabled )
          {

            sprintf(Log_Buffer,"The drive is a PRM whose media is totally blank (no MBR).\n     Creating a fake MBR and partition for this drive.");
            Write_Log_Buffer();

          }

          /* We must create the "fake" MBR and partition for this PRM drive. */

          /* Make an entry in the Partitions list for the MBR/EBR. */
          Find_And_Allocate_Partition( Index,                               /* The index in the Drive Array to use. */
                                       0,                                   /* The LBA at which the partition starts.  In this case, the LBA of the MBR/EBR. */
                                       DriveArray[Index].Geometry.Sectors,  /* We will allocate a full track.  Since partitions must be track aligned, the next partition can not start until the next track anyway. */
                                       MBR_EBR,                             /* This is an MBR */
                                       FALSE,                               /* This does not count as a primary partition. */
                                       NULL,                                /* There is no partition table entry for an MBR. */
                                       NEW_PARTITION_TABLE_INDEX,
                                       NULL,                                /* There is no DLA table entry for an MBR. */
                                       FALSE,                               /* Set to FALSE so that, if the user changes from Big Floppy to PRM, a new MBR will be written. */
                                       FALSE,                               /* This is not a partition, so there can be no alias table entry for it. */
                                       Error_Code);

          if ( *Error_Code != LVM_ENGINE_NO_ERROR )
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Find_And_Allocate_Partition failed with LVM error code %d (decimal)", *Error_Code);
              Write_Log_Buffer();

            }

            /* We have a problem.  Since Allocate_Existing_Paritition has already set *Error_Code, all we need to do is clean-up and return. */
            Close_Partition_Manager();

            FUNCTION_EXIT("Discover_Partitions")

            return;

          }

          /* In order to make a "fake" partition, we need a "fake" partition table entry for it. */
          memset( &Fake_Partition_Table_Entry, 0, sizeof( Partition_Record ) );
          Fake_Partition_Table_Entry.Boot_Indicator = 0x80;
          Fake_Partition_Table_Entry.Format_Indicator = FAT16_LARGE_PARTITION_INDICATOR;

          /* Now we must create a "fake" partition which occupies the rest of the disk. */
          Find_And_Allocate_Partition( Index,                               /* The index in the Drive Array to use. */
                                       DriveArray[Index].Geometry.Sectors,  /* The LBA of the first sector of the second track on the disk. */
                                       (DriveArray[Index].Geometry.Cylinders * DriveArray[Index].Geometry.Heads * DriveArray[Index].Geometry.Sectors) - DriveArray[Index].Geometry.Sectors,
                                       Partition,                           /* This is a partition */
                                       TRUE,                                /* This does counts as a primary partition. */
                                       &Fake_Partition_Table_Entry,         /* Use the "fake" partition table entry. */
                                       NEW_PARTITION_TABLE_INDEX,
                                       NULL,                                /* There is no DLA table entry for a "fake" partition like this one. */
                                       TRUE,
                                       FALSE,                               /* There is no alias table entry for this partition. */
                                       Error_Code);

          if ( *Error_Code != LVM_ENGINE_NO_ERROR )
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Find_And_Allocate_Partition failed with LVM error code %d (decimal)", *Error_Code);
              Write_Log_Buffer();

            }

            /* We have a problem.  Since Allocate_Existing_Paritition has already set *Error_Code, all we need to do is clean-up and return. */
            Close_Partition_Manager();

            FUNCTION_EXIT("Discover_Partitions")

            return;

          }

          /* Now we must mark the drive as being in big floppy mode, and we must clear the ChangesMade flag as we don't
             want these "fake" changes being written to the disk!                                                        */
          DriveArray[Index].Is_Big_Floppy = TRUE;
          DriveArray[Index].ChangesMade = FALSE;

          /* Now, since we have a "fake" primary partition, we must update the primary partition counter for the drive. */
          DriveArray[Index].Primary_Partition_Count = 1;

        }
        else
        {

          /* Were we processing an MBR or an EBR.  If an MBR, then the fact that the signature was bad probably means that the drive has no MBR
             and is empty.  If this was an EBR, then the drive is corrupt.                                                                       */
          if ( MBR_EBR_LBA > 0 )
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"Error:  Did not find an EBR where one was expected!");
              Write_Log_Buffer();

            }

            /* Mark the drive as having corrupt partition information. */
            DriveArray[Index].Corrupt = TRUE;

          }
          else
          {

            if ( Logging_Enabled )
            {

              sprintf(Log_Buffer,"The fixed disk is empty (no MBR).");
              Write_Log_Buffer();

            }

          }

        }

      }

      /* Now, if there is an EBR, set up to load the EBR and process it as if it was an MBR. */
      if ( EBR_Found )
      {

        /* Set the address to load the new EBR from. */
        MBR_EBR_LBA = Next_EBR_LBA;

      }

    } while ( EBRs_Remain  );

    /* Out of MBRs and EBRs.  We are now done with this drive.  Lets move on to the next entry in the DriveArray. */

  } /* End of for loop. */

  /* Were any of the drives usable? */
  if ( IO_Error_Count >= DriveCount )
  {

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"Error:  IO Success check failed!\n     There were IO errors in critical areas on every drive!");
      Write_Log_Buffer();

    }

    /* Since there were errors on all of the drives, there is nothing we can do.  Close the Partition Manager and abort. */
    *Error_Code = LVM_ENGINE_IO_ERROR;

    Close_Partition_Manager();


  }
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Discover_Partitions")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Commit_Partition_Changes                         */
/*                                                                   */
/*   Descriptive Name: This function writes out the partitioning     */
/*                     information for any drive which has had its   */
/*                     partitioning information altered in any way.  */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be                 */
/*              LVM_ENGINE_NO_ERROR and the affected disk drives will*/
/*              have had their partitioning information updated.     */
/*                                                                   */
/*   Error Handling: If there is an I/O error, the DriveArray entry  */
/*                   for the partition experiencing the error will be*/
/*                   set to TRUE.                                    */
/*                                                                   */
/*   Side Effects: If there is an I/O error, the DriveArray entry    */
/*                 for the partition experiencing the error will be  */
/*                 set to TRUE.                                      */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Commit_Partition_Changes( CARDINAL32 * Error_Code )
{

  CARDINAL32           Index;                     /* Used to access the drive array. */
  CARDINAL32           Partition_Table_Index;     /* Used to initialize the structures which keep track of which entries in the MBR are in use. */
  MBR_EBR_Build_Data   Build_Data;                /* Used to collect information needed to calculate the start, end, and size of any extended partition which may exist. */
  BOOLEAN              IO_Error_Detected = FALSE; /* Used to track whether or not I/O errors were encountered. */
  DLA_Table_Sector *   EBR_DLA;                   /* Used to access the DLA table corresponding to an EBR. */
  DLA_Table_Sector *   MBR_DLA;                   /* Used to access the DLA tabel associated with the MBR. */
  Master_Boot_Record * Clean_MBR;                 /* Used to access the MBR and clear its partition table. */


  FUNCTION_ENTRY("Commit_Partition_Changes")

  /* Has the Partition Manager been initialized yet? */

#ifdef DEBUG

#ifdef PARANOID

  assert( Partition_Manager_Initialized );

#else

  /* Has the Partition Manager been initialized already? */
  if ( ! Partition_Manager_Initialized )
  {

    /* This should not have happened!  We have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    LOG_ERROR("Partition Manager has not been initialized!")

    FUNCTION_EXIT("Commit_Partition_Changes")

    return;

  }

#endif

#endif

  /* Begin the Commit process. */

  /* For each Drive Array entry which has its ChangesMade flag on, we must build and write to disk all of the MBR/EBRs. */
  for ( Index = 0; Index < DriveCount; Index++)
  {

    /* Does the current drive array entry have its ChangesMade flag set? */
    if ( ( DriveArray[Index].ChangesMade == TRUE ) && ( ! DriveArray[Index].Unusable ) )
    {

      /* We must rebuild all of the MBR/EBRs for this drive, as well as their corresponding DLA tables. */

      /* We will first go through the Partitions list updating the Partition_Table_Entry and DLA_Table_Entry fields for each
         Partition_Data record which represents a partition.  We will also keep the starting address of the first EBR we
         come across, as that will be the start of an extended partition.                                                      */

      /* Initialize Build_Data. */
      Build_Data.Extended_Partition_Start = 0;
      Build_Data.Extended_Partition_End = 0;
      Build_Data.EBR_Start = 0;
      Build_Data.EBR_End = 0;
      Build_Data.EBR_Size = 0;
      Build_Data.Logical_Drive_Start = 0;
      Build_Data.Logical_Drive_Size = 0;
      Build_Data.New_MBR_Needed = FALSE;

      LOG_EVENT("Attempting to determine if an extended partition is necessary.")

      /* Indicate that we are looking for the starting address of the extended partition. */
      Build_Data.Find_Start = TRUE;

      /* Process the Partitions list. */
      ForEachItem( DriveArray[Index].Partitions, &Update_Table_Entries, &Build_Data, TRUE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

      assert(*Error_Code == DLIST_SUCCESS);

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FUNCTION_EXIT("Commit_Partition_Changes")

        return;

      }

#endif

#endif

      /* Now, lets see if we have an extended partition.  If there is no extended partition, then we don't need to do another pass on
         the Partitions list to determine the end of the extended partition.                                                           */
      if ( Build_Data.Extended_Partition_Start != 0 )
      {

        LOG_EVENT("An extended partition is necessary.")

        /* Since an extended partition exists, we must determine where it ends.  We also need to determine the link entries for the EBRs.
           A "link entry", as I refer to it here, is the entry in one EBR which points to the location of the next EBR in the chain.  This
           link entry contains the offset from the beginning of the extended partition of the next EBR, and the size of the next EBR and its
           associated logical drive.  To establish the values for the link entries, we must walk the Partitions list in reverse order.        */

        LOG_EVENT1("The start of the extended partition has been determined.","Extended Partition Starting Sector", Build_Data.Extended_Partition_Start )

        /* Indicate that we are looking for the end of the extended partition.  This will also cause the Update_Table_Entries routine to
           build the link entries for the EBRs.                                                                                           */
        Build_Data.Find_Start = FALSE;

        /* Set the EBR_Start field to 0.  It was used during the last pass for calculating offsets for partition table entries.  We will
           use it on this pass to calculate EBR Link Entries.                                                                             */
        Build_Data.EBR_Start = 0;

        /* Process the partitions list in reverse. */
        ForEachItem( DriveArray[Index].Partitions, &Update_Table_Entries, &Build_Data, FALSE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

        assert(*Error_Code == DLIST_SUCCESS);

#else

        if ( *Error_Code != DLIST_SUCCESS )
        {

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FUNCTION_EXIT("Commit_Partition_Changes")

          return;

        }

#endif

#endif

        LOG_EVENT1("The end of the extended partition has been determined.","Extended Partition Ending Sector", Build_Data.Extended_Partition_End)

      }

      /* Now we can construct the MBRs and EBRs and write them to disk. */

      /* Do we need a new MBR?  If not, then read the old MBR into the MBR buffer. */
      if ( Build_Data.New_MBR_Needed )
      {

        LOG_EVENT("Creating a new MBR for the drive.")

        /* Create the new MBR in the MBR buffer. */
        Create_MBR();

      }
      else
      {

        LOG_EVENT("Reading in the existing MBR for the drive.")

        /* Read in the old MBR. */
        ReadSectors(Index + 1,    /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                    0,            /* MBR is always at LBA 0. */
                    1,            /* We only want the MBR/EBR. */
                    &MBR,         /* The Buffer to use. */
                    Error_Code);

        /* Was the read successful? */
        if ( *Error_Code != DISKIO_NO_ERROR )
        {

          LOG_ERROR2("ReadSectors failed while attempting to read the MBR for the current drive.","Drive Number",Index + 1,"Error code", *Error_Code)

          /* If this read fails, we can not process this drive! */
          DriveArray[Index].IO_Error = TRUE;

          /* Remember that an I/O error occured. */
          IO_Error_Detected = TRUE;

          /* Move on to the next drive. */
          continue;

        }

      }

      /* Clear out the old partition table in the MBR. */
      Clean_MBR = (Master_Boot_Record *) &MBR;                            /* Establish access to the MBR. */
      memset(Clean_MBR->Partition_Table,0,4 * sizeof(Partition_Record) ); /* Clear out the Partition table in the MBR. */

      /* Since the MBR is now empty, set our in use and claimed indicators accordingly. */
      for ( Partition_Table_Index = 0; Partition_Table_Index < 4; Partition_Table_Index++ )
      {

        Partition_Table_Entry_Claimed[Partition_Table_Index] = FALSE;
        Partition_Table_Index_In_Use[Partition_Table_Index] = FALSE;

      }

      /* Prepare the EBR buffer. */
      memset(&EBR,0,sizeof(Master_Boot_Record));

      /* Set the MBR/EBR signature in the EBR. */
      EBR.Signature = MBR_EBR_SIGNATURE;

      /* Set up the MBR DLA Table and the EBR DLA Table. */

      /* Establish access to the buffers used to hold the DLA tables. */
      MBR_DLA = (DLA_Table_Sector *) &MBR_DLA_Sector;
      EBR_DLA = (DLA_Table_Sector *) &DLA_Sector;

      /* Zero out the MBR_DLA_Sector and the DLA_Sector. */
      memset(&MBR_DLA_Sector, 0, BYTES_PER_SECTOR);
      memset(&DLA_Sector, 0, BYTES_PER_SECTOR);

      /* Now Initialize the common fields. */
      MBR_DLA->DLA_Signature1 = DLA_TABLE_SIGNATURE1;
      MBR_DLA->DLA_Signature2 = DLA_TABLE_SIGNATURE2;
      EBR_DLA->DLA_Signature1 = DLA_TABLE_SIGNATURE1;
      EBR_DLA->DLA_Signature2 = DLA_TABLE_SIGNATURE2;

      MBR_DLA->DLA_CRC = 0;
      EBR_DLA->DLA_CRC = 0;

      strncpy(MBR_DLA->Disk_Name, DriveArray[Index].Drive_Name, DISK_NAME_SIZE);
      strncpy(EBR_DLA->Disk_Name, DriveArray[Index].Drive_Name, DISK_NAME_SIZE);

      MBR_DLA->Disk_Serial_Number = DriveArray[Index].Drive_Serial_Number;
      EBR_DLA->Disk_Serial_Number = DriveArray[Index].Drive_Serial_Number;

      /* If the Boot Drive Serial Number stored with the drive is 0, it has not yet been initialized.  So
         set it to the current value.                                                                       */
      if ( DriveArray[Index].Boot_Drive_Serial_Number == 0 )
        DriveArray[Index].Boot_Drive_Serial_Number = Boot_Drive_Serial_Number;

      /* Set the Boot Drive Serial Number in the DLA Tables. */
      MBR_DLA->Boot_Disk_Serial_Number = DriveArray[Index].Boot_Drive_Serial_Number;
      EBR_DLA->Boot_Disk_Serial_Number = DriveArray[Index].Boot_Drive_Serial_Number;

      /* Save the disk geometry for use by OS2DASD. */
      MBR_DLA->Cylinders = DriveArray[Index].Geometry.Cylinders;
      MBR_DLA->Heads_Per_Cylinder = DriveArray[Index].Geometry.Heads;
      MBR_DLA->Sectors_Per_Track = DriveArray[Index].Geometry.Sectors;

      EBR_DLA->Cylinders = DriveArray[Index].Geometry.Cylinders;
      EBR_DLA->Heads_Per_Cylinder = DriveArray[Index].Geometry.Heads;
      EBR_DLA->Sectors_Per_Track = DriveArray[Index].Geometry.Sectors;

      /* Only the MBR DLA Table gets its Reboot field set.  This is only used during Install. */
      MBR_DLA->Reboot = DriveArray[Index].Reboot_Flag;
      EBR_DLA->Reboot = FALSE;

      /* Only the MBR DLA Table gets its Install_FLags field set.  This is only used during Install. */
      MBR_DLA->Install_Flags = DriveArray[Index].Install_Flags;
      EBR_DLA->Install_Flags = 0;

      memset(MBR_DLA->DLA_Array, 0, 4 * sizeof(DLA_Entry) );
      memset(EBR_DLA->DLA_Array, 0, 4 * sizeof(DLA_Entry) );

      /* Now traverse the Partitions list and write all of the EBRs to disk. */

      /* Process the partitions list in reverse. */
      ForEachItem( DriveArray[Index].Partitions, &Write_Changes, NULL, FALSE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

      assert(*Error_Code == DLIST_SUCCESS);

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FUNCTION_EXIT("Commit_Partition_Changes")

        return;

      }

#endif

#endif

      /* Now that all of the EBRs have been written to disk, we can finish the MBR and write it to disk. */

      if ( Build_Data.Extended_Partition_Start != 0 )
      {

        LOG_EVENT("Adding an entry for the extended partition to the partition table in the MBR.")

        /* We must add an entry to the MBR for our extended partition. */
        Add_Extended_Partition_To_MBR(Build_Data.Extended_Partition_Start, Build_Data.Extended_Partition_End, Index, Error_Code);

        /* Was there an error? */
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          LOG_ERROR1("Unable to add an entry for the extended partition to the partition table in the MBR!","Error code",*Error_Code)

          FUNCTION_EXIT("Commit_Partition_Changes")

          return;

        }

      }

      /* Now that the MBR is complete, write it to disk. */

      LOG_EVENT("Writing the MBR to disk.")

      /* Write the MBR. */
      WriteSectors(Index + 1,   /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                  0,            /* MBR is always at LBA 0. */
                  1,            /* We only want the MBR/EBR. */
                  &MBR,         /* The Buffer to use. */
                  Error_Code);

      /* Was the write successful? */
      if ( *Error_Code != DISKIO_NO_ERROR )
      {

        LOG_ERROR2("WriteSectors failed while writing the MBR to disk!","Drive Number",Index + 1,"Error code", *Error_Code)

        /* If this write fails, the drive may be unusable! Log the error against the drive. */
        DriveArray[Index].IO_Error = TRUE;

        /* Remember that an I/O error occured. */
        IO_Error_Detected = TRUE;

      }

      /* Now we must calculate the CRC for the MBR's corresponding DLA Table and then write that DLA Table to disk. */

      MBR_DLA->DLA_CRC = 0;

      MBR_DLA->DLA_CRC = CalculateCRC( INITIAL_CRC, &MBR_DLA_Sector, BYTES_PER_SECTOR);

      LOG_EVENT("Writing the DLA table for the MBR to disk.")

      /* Now write the DLA table to disk. */
      WriteSectors(Index + 1,                                    /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                  DriveArray[Index].Geometry.Sectors - 1,        /* The MBR DLA is always at LBA 0 + Sectors_Per_Track - 1. */
                  1,                                             /* We only want the DLA Table. */
                  &MBR_DLA_Sector,                               /* The Buffer to use. */
                  Error_Code);

      /* Was the write successful? */
      if ( *Error_Code != DISKIO_NO_ERROR )
      {

        LOG_ERROR2("WriteSectors failed while writing the DLA to disk!", "Disk Number", Index + 1, "Error code", *Error_Code)

        /* Log the error against the drive. */
        DriveArray[Index].IO_Error = TRUE;

        /* Remember that an I/O error occured. */
        IO_Error_Detected = TRUE;

      }

    }

  }

  if ( IO_Error_Detected )
  {

    LOG_ERROR("I/O errors were encountered during the Partition Manager commit process.")

    /* Indicate that I/O errors were encountered. */
    *Error_Code = LVM_ENGINE_IO_ERROR;

  }
  else
  {

    /* Indicate success. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  FUNCTION_EXIT("Commit_Partition_Changes")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_PartitionsOn_Drive                           */
/*                                                                   */
/*   Descriptive Name: Returns an array of partitions associated     */
/*                     with the drive specified by DriveArrayIndex.  */
/*                                                                   */
/*   Input:CARDINAL32 DriveArrayIndex : The index into the drive     */
/*                                      array for the drive to use.  */
/*         CARDINAL32 * Error_Code - The address of a CARDINAL32 in  */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: This function returns a structure.  The structure has   */
/*           two components: an array of partition information       */
/*           records and the number of entries in the array.  The    */
/*           array will contain a partition information record for   */
/*           each partition and block of free space on the drive.    */
/*           If no errors occur, then *Error_Code will be 0.  If an  */
/*           error does occur, then *Error_Code will be non-zero.    */
/*                                                                   */
/*   Error Handling: Any memory allocated for the return value will  */
/*                   be freed.  The Partition_Information_Array      */
/*                   returned by this function will contain a NULL   */
/*                   pointer for Partition_Array, and have a Count of*/
/*                   0.  *Error_Code will be non-zero.               */
/*                                                                   */
/*                   If DriveArrayIndex is invalid, a trap is likely.*/
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
Partition_Information_Array Get_Partitions_On_Drive( CARDINAL32 DriveArrayIndex, CARDINAL32 * Error_Code )
{
  Partition_Information_Array      ReturnValue;                   /* The structure to be returned to the caller. */
  CARDINAL32                       Eligible_Partition_Count = 0;  /* Used to count how many partitions in the partitions list are to be returned to the caller. */

  FUNCTION_ENTRY("Get_Partitions_On_Drive")

  /* Initialize ReturnValue assuming failure. */
  ReturnValue.Count = 0;
  ReturnValue.Partition_Array = NULL;

  /* Is the DriveArrayIndex valid? */
  if ( DriveArrayIndex >= DriveCount )
  {

    /* Bad drive array index!  Since this is a function which is internal to the engine, this should never happen! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Partitions_On_Drive")

    return ReturnValue;

  }

  /* How many partitions are on the drive which are eligible to be returned to the caller?  We only count free space and partitions, not entries for MBRs or EBRs. */
  ForEachItem(DriveArray[DriveArrayIndex].Partitions,&Count_Eligible_Partitions,&Eligible_Partition_Count,TRUE,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Was there an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* This should never happen here.  We have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Partitions_On_Drive")

    return ReturnValue;

  }

#endif

#endif

  /* Allocate memory for the array being returned to the caller. */
  ReturnValue.Partition_Array = (Partition_Information_Record *) malloc(Eligible_Partition_Count * sizeof(Partition_Information_Record) );

  if ( ReturnValue.Partition_Array == NULL )
  {

    /* We are out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Get_Partitions_On_Drive")

    return ReturnValue;

  }

  /* ReturnValue.Count is currently 0.  We will use it to index the Partition_Array as we traverse the partition list again. */

  /* Now traverse the list and transfer the information from the list to the array. */
  ForEachItem(DriveArray[DriveArrayIndex].Partitions,&Transfer_Partition_Data,&ReturnValue,TRUE,Error_Code);

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

    FUNCTION_EXIT("Get_Partitions_On_Drive")

    return ReturnValue;

  }

#endif

#endif

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Return what we have found to the caller. */

  FUNCTION_EXIT("Get_Partitions_On_Drive")

  return ReturnValue;

}


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
Partition_Information_Record  Get_Partition_Information( ADDRESS Partition_Handle, CARDINAL32 * Error_Code )
{

  Partition_Information_Record     ReturnValue; /* Used to hold the value we are going to return while we construct it. */

  ADDRESS                          Object;      /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                              ObjectTag;   /* Used when translating the Partition_Handle into a Partition_Data structure. */


  API_ENTRY("Get_Partition_Information")

  /* Initialize ReturnValue assuming a failure. */
  memset( &ReturnValue, 0, sizeof(Partition_Information_Record) );

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Get_Partition_Information")

    return ReturnValue;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Partition_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Get_Partition_Information")

    return ReturnValue;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != PARTITION_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Get_Partition_Information")

    return ReturnValue;

  }

  /* Now lets initialize ReturnValue using the contents of Object (which we know is a Partition_Data structure). */
  Set_Partition_Information_Record( &ReturnValue, (Partition_Data *) Object);

  /* All done. */

  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Get_Partition_Information")

  return ReturnValue;

}


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
ADDRESS Create_Partition( ADDRESS               Handle,
                          CARDINAL32            Size,
                          char                  Name[ PARTITION_NAME_SIZE ],
                          Allocation_Algorithm  algorithm,
                          BOOLEAN               Bootable,
                          BOOLEAN               Primary_Partition,
                          BOOLEAN               Allocate_From_Start,
                          CARDINAL32 *          Error_Code
                        )
{

  Partition_Data   *                 PartitionRecord;               /* Used to point to the Partition_Data structure we are manipulating. */
  Disk_Drive_Data  *                 Drive_Data;                    /* Used when Handle is the handle of a disk drive, not a block of free space. */
  ADDRESS                            Object;                        /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                                ObjectTag;                     /* Used when translating the Partition_Handle into a Partition_Data structure. */
  CARDINAL32                         Index;                         /* Used to access the drive array. */
  LBA                                Starting_Sector;               /* Used to calculate the LBA of the starting sector for the partition. */
  CARDINAL32                         Adjustment;                    /* Used when calculating adjustments to the size and/or starting sector of a partition. */
  Partition_Data   *                 Selected_Free_Space = NULL;    /* Used to point to the block of free space selected for creating the partition in. */
  ADDRESS                            New_Partition_Handle = NULL;   /* Used to hold the external handle for the new partition being created. */
  Duplicate_Check_Parameter_Record   Name_Check_Parameters;         /* Used to check for duplicate partition names. */
  Kill_Sector_Data                   Sector_To_Overwrite;           /* Used to place sectors in the KillSector list. */
  DLA_Entry                          DLA_Table_Entry;               /* The DLA table entry to use for the partition being created. */
  Partition_Record                   Partition_Table_Entry;         /* The partition table entry to use for the partition being created. */
  Primary_Partition_Status           Primary_Status_Record;         /* Used when determining the types of primary partitions which exist on a drive. */
  BOOLEAN                            Need_MBR = FALSE;              /* Set to TRUE for drives which have no MBR.  Reminds us to allocate an MBR for these drives. */
  BOOLEAN                            Free_Space_Selected = FALSE;   /* Used to control the do-while loop.  */
  BOOLEAN                            Still_Looking;                 /* Used to control the search for a suitable block of free space when Handle refers to a drive instead of a block of free space. */
  BOOLEAN                            FreeSpaceAvailable = FALSE;    /* Used to decide which error message to return when no acceptable block of free space is found. */
  BOOLEAN                            Wrong_Partition_Type = FALSE;  /* Used to decide which error message to return when no acceptable block of free space is found. */


  API_ENTRY("Create_Partition")

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Create_Partition")

    return NULL;

  }

    if ( Logging_Enabled )
    { sprintf(Log_Buffer,"Create_Partition: %s\n",Name);
      Write_Log_Buffer();
    }
  
  /* Zero out the Partition_Table_Entry and the DLA_Table_Entry. */
  memset(&Partition_Table_Entry,0,sizeof(Partition_Record) );
  memset(&DLA_Table_Entry,0,sizeof(DLA_Entry) );

  /* Set Boot_Sector to 0xf6.  When a partition is created, some of the code is shared with Discover_Partitions.  This code will
     look in the Boot_Sector variable to determine how to translate the Format_Indicator assigned to the new partition.  To
     ensure correct translation, set the Boot_Sector variable to all 0xf6.                                                          */
  memset(Boot_Sector, 0xf6, BYTES_PER_SECTOR);

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Create_Partition")

    return NULL;

  }

  /* D201136 - The LVM VIO interface can't handle having the engine remove trailing spaces from the
               strings it passes in, so the engine was modified to copy the Name string passed in and
               operate on the copy.

     BEGIN                                                                                          BMR */

  /* Get the current length of Name. */
  Name_Check_Parameters.New_Name_Length = strlen(Name);

  /* Is the length of the name acceptable? */
  if ( Name_Check_Parameters.New_Name_Length == 0 )
  {

    /* Indicate that the name is not acceptable. */
    *Error_Code = LVM_ENGINE_BAD_NAME;

    API_EXIT("Create_Partition")

    return NULL;

  }

  /* Copy Name so that we have something we can manipulate without causing grief to whoever called us. */
  Name_Check_Parameters.New_Name = (char *) malloc( strlen(Name) + 1);

  /* Did we get the memory? */
  if ( Name_Check_Parameters.New_Name == NULL )
  {

    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    API_EXIT("Create_Partition")

    return NULL;

  }

  /* Make the copy. */
  strcpy(Name_Check_Parameters.New_Name, Name);

  /* Set Name to point to the copy also. */
  Name = Name_Check_Parameters.New_Name;
    if ( Logging_Enabled )
    { sprintf(Log_Buffer,"Create_Partition2: %s\n",Name);
      Write_Log_Buffer();
    }

  /* D201136 - The LVM VIO interface can't handle having the engine remove trailing spaces from the
               strings it passes in, so the engine was modified to copy the Name string passed in and
               operate on the copy.

     END                                                                                            BMR */

  /* If we are given the handle of a drive instead of the handle of a block of free space, we will go through the switch statement below twice.
     The first time will be to choose a block of free space.  The second time will be to create a partition out of the block of free space.

     If we are given the handle of a block of free space, then we will go though the switch statement below only once.                               */
  do
  {

    /* From the ObjectTag we can tell what Object points to. */
    switch ( ObjectTag )
    {

      case PARTITION_DATA_TAG : /* We have been given the handle of a partition or block of free space. */

                                /* Establish access to the Partition_Data record we have been given. */
                                PartitionRecord = ( Partition_Data * ) Object;

                                /* Check the type of Partition_Data we have been given. */
                                if ( PartitionRecord->Partition_Type != FreeSpace )
                                {

                                  /* We have been given a bad handle.  A Create_Partition operation is not allowed on an existing partition. */
                                  *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }

                                /* Establish easy access to the DriveArray entry that corresponds to this Partition_Data. */
                                Index = PartitionRecord->Drive_Index;

 /* Is the Partitions list for the drive corrupt?  If so, the only legal operation is a delete! */
//                              if ( DriveArray[Index].Corrupt )
                                if ( DriveArray[Index].Corrupt &&  !DriveArray[Index].NonFatalCorrupt)
                                {

                                  /* Can't create anything on this drive! */
                                  *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }

                                /* A Size of 0 may only be specified if the allocation algorithm is all. */
                                if ( Size == 0 )
                                {

                                  /* Is the allocation algorithm All? */
                                  if ( algorithm == All )
                                  {

                                    /* We are to turn this block of free space into a single partition.
                                       Set size equal to the size of the block of free space.            */
                                    Size = PartitionRecord->Partition_Size;

                                  }
                                  else
                                  {

                                    /* The algorithm was not All.  This is not a valid partition creation request. */
                                    *Error_Code = LVM_ENGINE_REQUESTED_SIZE_TOO_SMALL;

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

                                }

                                /* Is the name specified for the partition acceptable? */

                                /* Adjust the name to eliminate any leading or trailing spaces. */
                                Name_Check_Parameters.New_Name = Adjust_Name(Name);
                                Name_Check_Parameters.New_Name_Length = strlen(Name);

                                /* Is the length of the name acceptable? */
                                if ( Name_Check_Parameters.New_Name_Length == 0 )
                                {

                                  /* Indicate that the name is not acceptable. */
                                  *Error_Code = LVM_ENGINE_BAD_NAME;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }
    if ( Logging_Enabled )
    { sprintf(Log_Buffer,"Create_Partition3:Name_Check_Parameters.New_Name= %s\n",Name_Check_Parameters.New_Name);
      Write_Log_Buffer();
    }

                                /* Is the name specified for the partition unique among the partitions on the drive? */

                                /* Set up for the name check. */
                                Name_Check_Parameters.Handle = NULL;
                                Name_Check_Parameters.Duplicate_Name_Found = FALSE;
                                Name_Check_Parameters.Check_Name = TRUE;
                                Name_Check_Parameters.Max_Name_Length = PARTITION_NAME_SIZE;

                                /* Now do the check. */
                                ForEachItem(DriveArray[Index].Partitions,&Duplicate_Check, &Name_Check_Parameters, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                                assert(*Error_Code == DLIST_SUCCESS);

#else

                                /* Was there an error? */
                                if ( *Error_Code != DLIST_SUCCESS )
                                {

                                  /* This should never happen!  Our internal data structures must be corrupted! */
                                  *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }

#endif

#endif

                                /* Was a duplicate name found? */
                                if ( Name_Check_Parameters.Duplicate_Name_Found )
                                {

                                  /* There was a duplicate name.  Signal the error and return. */
                                  *Error_Code = LVM_ENGINE_DUPLICATE_NAME;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }

                                /* If a drive does not have an MBR, it can not have any partitions.  This means that the only item in the
                                   Partitions list for the drive will be an entry representing all of the disk as free space.  Thus,
                                   we can test to see if the drive has an MBR by looking at how many items are in its Partitions list.       */

                                if ( GetListSize(DriveArray[Index].Partitions,Error_Code) == 1 )
                                {

                                  /* This drive has no MBR.  Indicate that we need one. */
                                  Need_MBR = TRUE;

                                }

#ifdef DEBUG

#ifdef PARANOID

                                assert ( *Error_Code == DLIST_SUCCESS );

#else

                                /* Was there an error? */
                                if ( *Error_Code != DLIST_SUCCESS )
                                {

                                  *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }

#endif

#endif

                                /* Adjust the requested size of the partition so that it is a multiple of Sectors_Per_Cylinder.
                                   Always round up!                                                                               */
                                if ( Size % DriveArray[Index].Sectors_Per_Cylinder )
                                {

                                  /* Convert size from sectors to cylinders. */
                                  Size = Size / DriveArray[Index].Sectors_Per_Cylinder;

                                  /* Round up. */
                                  Size++;

                                  /* Now turn size back into sectors. */
                                  Size = Size * DriveArray[Index].Sectors_Per_Cylinder;

                                }

                                /* Calculate the starting LBA for the partition. */
                                if ( Allocate_From_Start )
                                {

                                  /* The starting LBA of the partition is the same as the starting LBA of the block of free space. */
                                  Starting_Sector = PartitionRecord->Starting_Sector;

                                  if ( Need_MBR )
                                  {

#ifdef DEBUG

                                    assert(Starting_Sector == 0);

#endif

                                    /* Primary partitions must be track aligned.  Non-primary partitions must be cylinder aligned. */
                                    if ( Primary_Partition )
                                    {

                                      /* Since we have a primary partition, and since we need an MBR, the partition can start no sooner than
                                         the first sector of the track following the track containing the MBR.  Adjust Starting_Sector accordingly. */
                                      Starting_Sector = DriveArray[Index].Geometry.Sectors;

                                    }
                                    else
                                    {

                                      /* Non-primary partitions must be cylinder aligned.  The first sector of the cylinder they start on
                                         will hold their EBR.  Since the MBR takes up a sector on the first track of the first cylinder,
                                         a non-primary partition can not start until the second cylinder.  Adjust Starting_Sector accordingly. */
                                      Starting_Sector = DriveArray[Index].Sectors_Per_Cylinder;

                                    }

                                  }

                                }
                                else
                                {

                                  /* The partition will start somewhere within the block of free space.  Calculate the starting point. */

                                  /* Is the block of free space large enough to hold the partition? */
                                  if ( Size > PartitionRecord->Partition_Size )
                                  {

                                    /* Is the size of the block of free space within 5% of the size of the requested partition? */
                                    if ( ( Size - (Size / 20) ) <= PartitionRecord->Partition_Size )
                                    {

                                      /* The size of the requested partition is close enough.  We will "round" the requested size of the partition
                                         to match the size of the block of free space.                                                              */
                                      Size = PartitionRecord->Partition_Size;

                                    }
                                    else
                                    {

                                      /* The block of free space is too small to hold the requested partition.  Indicate an error and abort. */
                                      *Error_Code = LVM_ENGINE_REQUESTED_SIZE_TOO_BIG;

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                      return NULL;

                                    }

                                  }

                                  /* The block of free space is large enough to hold the partition, so calculate the starting point for the partition. */
                                  Starting_Sector = PartitionRecord->Starting_Sector + ( PartitionRecord->Partition_Size - Size );

                                  if ( Need_MBR )
                                  {

                                    /* Primary partitions must be track aligned.  Non-primary partitions must be cylinder aligned.
                                       Since the first sector of the first track is in use for the MBR, a partition can not start
                                       until the second track.                                                                       */
                                    if ( Starting_Sector < DriveArray[Index].Geometry.Sectors )
                                    {

                                      /* Primary partitions must be track aligned.  Non-primary partitions must be cylinder aligned. */
                                      if ( Primary_Partition )
                                      {

                                        /* Since we have a primary partition, and since we need an MBR, the partition can start no sooner than
                                           the first sector of the track following the track containing the MBR.  Adjust Starting_Sector accordingly. */
                                        Starting_Sector = DriveArray[Index].Geometry.Sectors;

                                      }
                                      else
                                      {

                                        /* Non-primary partitions must be cylinder aligned.  The first sector of the cylinder they start on
                                           will hold their EBR.  Since the MBR takes up a sector on the first track of the first cylinder,
                                           a non-primary partition can not start until the second cylinder.  Adjust Starting_Sector accordingly. */
                                        Starting_Sector = DriveArray[Index].Sectors_Per_Cylinder;

                                      }

                                    }

                                  }

                                }

                                /* Is the starting location for the partition correctly aligned? */
                                if ( Primary_Partition && ( Starting_Sector % DriveArray[Index].Geometry.Sectors ) )
                                {

                                  /* Primary partitions must be track aligned.  Make the starting point for the partition track aligned. */
                                  Starting_Sector += DriveArray[Index].Geometry.Sectors - ( Starting_Sector % DriveArray[Index].Geometry.Sectors ) ;

                                }
                                else
                                {

                                  /* Non-primary partitions must be cylinder aligned. */
                                  if ( (! Primary_Partition ) && ( Starting_Sector % DriveArray[Index].Sectors_Per_Cylinder ) )
                                  {

                                    /* Non-primary partitions must be cylinder aligned. */
                                    Starting_Sector += DriveArray[Index].Sectors_Per_Cylinder - ( Starting_Sector % DriveArray[Index].Sectors_Per_Cylinder );

                                  }

                                }

                                /* Is the calculated starting sector of the partition still within the block of free space? */
                                if ( Starting_Sector >= ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) )
                                {

                                  /* We can not create the partition in this block of free space.  This block of free space is too small
                                     and its odd alignement prevent us from creating an OS/2 acceptable partition.  Disks partitioned
                                     using Unix don't have the same rules for partitions as OS/2, Windows, Windows9x, and Windows NT do,
                                     and they may actually be able to use this small block of free space.  However, we can't.  Abort.     */
                                  *Error_Code = LVM_ENGINE_PARTITION_ALIGNMENT_ERROR;

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  return NULL;

                                }

                                /* Does the size of the partition have to shrink due to alignment? */
                                if ( ( Starting_Sector + Size ) > ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) )
                                {

                                  /* Fix the size of the partition. */
                                  Adjustment = ( Starting_Sector + Size ) - ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size );

                                  /* If the adjustment causes the partition's size to drop below ( one cylinder - 1 track), then we have a problem.  */
                                  if ( ( Adjustment > Size ) || ( ( Size - Adjustment ) < ( DriveArray[Index].Sectors_Per_Cylinder - DriveArray[Index].Geometry.Sectors ) ) )
                                  {

                                    /* The size of the resulting partition is too small to be used. */
                                    *Error_Code = LVM_ENGINE_PARTITION_ALIGNMENT_ERROR;

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

                                  /* Since the size of the resulting partition is still usable, make the change. */
                                  Size -= Adjustment;

                                }

                                /* Does the partition end on a cylinder boundary?  If not, make it so! */
                                Adjustment = ( Starting_Sector + Size ) % DriveArray[Index].Sectors_Per_Cylinder;
                                if ( Adjustment )
                                {

                                  /* Does the adjustment make the partition too small to create? */
                                  if ( ( Adjustment < Size ) && ( ( Size - Adjustment ) >= ( DriveArray[Index].Sectors_Per_Cylinder - DriveArray[Index].Geometry.Sectors ) ) )
                                  {

                                    /* Since the resulting size of the partition is acceptable, do it. */
                                    Size -= Adjustment;

                                  }
                                  else
                                  {

                                    /* The resulting size of the partition is too small.  Abort. */
                                    *Error_Code = LVM_ENGINE_PARTITION_ALIGNMENT_ERROR;

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

                                }

 /* Since the block of free space is big enough, can we create the type of partition requested?  There are
    limits on the number of Primary partitions that may exist, and there are limits on whether logical drives
    can exist, and, if they do, where they can be placed.  Also, if the partition is to be bootable, we must
    check to see if the 1024 cylinder limit is in effect, and, if so, can we create the partition below the
    1024 cylinder limit using the specified block of free space.                                               
    */

 /* Since the 1024 Cylinder limit check is easy, we will do that first. */

 /* Is the partition we are creating to be bootable? */
                                if ( Bootable )
                                {

 /* Is the type of partition being created appropriate?  If Boot Manager is NOT installed, then non-primary partitions
    can not be booted.  Similarly, without Boot Manager, primary partitions on disks other than the first one can not be booted. */

                                  if ( ( Boot_Manager_Handle == NULL ) || ( ! Boot_Manager_Active ) )
                                  {

                                    /* Boot Manager is NOT installed!  We are restricted to primary partitions on the first drive. */
//EK           
//todo
//                                  if ( ( ! Primary_Partition ) || ( Index != 0 ) )
                                    if ( ( ! Primary_Partition ) /* || ( Index != 0 ) */ )
                                    {

                                      /* We can not make the requested partition bootable! */
                                      *Error_Code = LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE;

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                      return NULL;

                                    }

                                  }

                                  /* Is the 1024 cylinder limit in effect? */
                                  if ( DriveArray[Index].Cylinder_Limit_Applies )
                                  {

                                    /* Does the partition end below the 1024 cylinder limit? */
                                    if ( ( Starting_Sector + Size ) > DriveArray[Index].Cylinder_Limit )
                                    {

                                        /* We can not create a bootable partition of the specified size from this block of free space. */
                                        *Error_Code = LVM_ENGINE_1024_CYLINDER_LIMIT;

                                        /* Free the copy of Name. */
                                        free(Name);

                                        API_EXIT("Create_Partition")

                                        return NULL;

                                    }

                                    /* We can create a bootable partition! */

                                  }

                                }

                                /* Once we get here, we know that:

                                        We have a block of free space to use
                                        The drive containing the block of free space is NOT corrupt
                                        The block of free space is large enough to create the partition with
                                        If the partition is to be bootable, then we can create it in the block of free space that we have
                                        We have the LBA of the starting sector of the partition, and the adjusted size of the partition.
                                                                                                                                             */

                                /* If we need an MBR, then there are no partitions and we are free to create any type of partition we want!  If
                                   already is an MBR, we must check to see if we can create the type of partition we need.                         */
                                if ( ! Need_MBR )
                                {

                                  /* Now we must check to see if we can create the proper type of partition - i.e. a primary partition or not. */

                                  /* Can we make the type of partition requested? */
                                  if ( Primary_Partition )
                                  {

                                    /* Find out if we can make a primary partition from the block of free space in PartitionRecord. */
                                    if ( ! Can_Be_Primary(PartitionRecord, Error_Code ) )
                                    {

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                       /* We can not make a primary partition!  *Error_Code has already been set by Can_Be_Primary, so just return. */
                                      return NULL;

                                    }

                                  }
                                  else
                                  {

                                    /* Find out if we can make a non-primary partition from the block of free space in PartitionRecord. */
                                    if ( ! Can_Be_Non_Primary(PartitionRecord, Error_Code ) )
                                    {

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                       /* We can not make a non-primary partition!  *Error_Code has already been set by Can_Be_Non_Primary, so just return. */
                                      return NULL;

                                    }

                                  }

                                }

                                /* At this point, there appears to be no reason why we can't create the requested partition. */

                                /* Lets now take care of any overhead items which must be created, like MBRs and EBRs. */

                                /* Do we need to create an MBR? */
                                if ( Need_MBR)
                                {

                                  /* MBRs always reside at LBA 0.  We will allocate a full track for the MBR.  The actual MBR will be created, on disk,
                                     by the Commit_Partition_Changes function.                                                                            */

                                  /* Allocate the MBR. */
                                  New_Partition_Handle = Allocate_Partition_From_Free_Space( Index, PartitionRecord, 0, DriveArray[Index].Geometry.Sectors, MBR_EBR, FALSE, NULL, NEW_PARTITION_TABLE_INDEX, NULL, FALSE, FALSE, Error_Code);

                                  /* Did we succeed? */
                                  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                  {

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    /* We have an error.  Abort the operation. */
                                    return NULL;

                                  }

                                }

                                /* Set up the DLA Table entry for the partition.  */
                                strncpy(DLA_Table_Entry.Partition_Name,Name,PARTITION_NAME_SIZE);
                                DLA_Table_Entry.Partition_Serial_Number = Create_Serial_Number();

    if ( Logging_Enabled )
    { sprintf(Log_Buffer,"Create_Partition5:DLA_Table_Entry.Partition_Name= %s\n",DLA_Table_Entry.Partition_Name);
      Write_Log_Buffer();
    }
                                /* Lets create the partition now. */
                                if ( Primary_Partition )
                                {

                                  /* Initialize the Primary_Status_Record. */
                                  memset(&Primary_Status_Record,0,sizeof(Primary_Status_Record) );

                                  /* Set the default Format_Indicator for a new partition. */
                                  ForEachItem( DriveArray[Index].Partitions, &Find_Active_Primary, &Primary_Status_Record, TRUE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

                                  assert( *Error_Code == DLIST_SUCCESS );

#else

                                  /* Was there an error? */
                                  if ( *Error_Code != DLIST_SUCCESS )
                                  {

                                    /* This should not happen!  Something must be corrupted! */
                                    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

#endif

#endif

                                  /* Is the active copy of Boot Manager on this partition? */
                                  if ( Primary_Status_Record.Active_Boot_Manager_Found )
                                  {

                                    /* Is there already a non-hidden primary partition?  Only one of these is allowed. */
                                    if ( Primary_Status_Record.Non_Hidden_Primary_Found )
                                    {

                                      /* Our new partition must be a hidden primary. */
                                      Partition_Table_Entry.Format_Indicator = NEW_PRIMARY_PARTITION_FORMAT_INDICATOR;

                                    }
                                    else
                                    {

                                      /* Our new partition must not be an active primary, but it must not be hidden, either. */
                                      Partition_Table_Entry.Format_Indicator = NEW_ACTIVE_PRIMARY_PARTITION_FORMAT_INDICATOR;

                                    }

                                  }
                                  else
                                  {

                                    /* Boot Manager does not exist on this drive. */

                                    /* Is there another active primary partition already? */
                                    if ( Primary_Status_Record.Active_Primary_Found )
                                    {

                                      /* We need to create a non-hidden, non-active primary partition. */
                                      Partition_Table_Entry.Format_Indicator = NEW_ACTIVE_PRIMARY_PARTITION_FORMAT_INDICATOR;

                                    }
                                    else
                                    {

                                      /* There are no active primary partitions.  We will make the new partiton active. */
                                      Partition_Table_Entry.Format_Indicator = NEW_ACTIVE_PRIMARY_PARTITION_FORMAT_INDICATOR;
                                      Partition_Table_Entry.Boot_Indicator = ACTIVE_PARTITION;

                                    }

                                  }

                                  /* Finish setting up the DLA_Table_Entry. */
                                  DLA_Table_Entry.Partition_Size = Size;
                                  DLA_Table_Entry.Partition_Start = Starting_Sector;

                                  /* If we created an MBR, then we must use the Find_And_Allocate_Partition function to create the partition.
                                     The handle we have for the block of free space may no-longer be valid as a result of our using it to create
                                     the MBR.  But, since we have the starting sector for the partition, and since the calculation of that starting
                                     sector took into account the creation of the MBR, we can use the Find_And_Allocate_Partition function.  This
                                     function will find the block of free space that the starting sector of our partition lies in and will allocate
                                     the partition from that.                                                                                             */
                                  if ( Need_MBR )
                                  {

                                    New_Partition_Handle = Find_And_Allocate_Partition( Index, Starting_Sector, Size, Partition, Primary_Partition, &Partition_Table_Entry, NEW_PARTITION_TABLE_INDEX, &DLA_Table_Entry, FALSE, FALSE, Error_Code);

                                    /* Did we succeed? */
                                    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                    {

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                      return NULL;

                                    }

                                  }
                                  else
                                  {

                                    New_Partition_Handle = Allocate_Partition_From_Free_Space( Index, PartitionRecord, Starting_Sector, Size, Partition, Primary_Partition, &Partition_Table_Entry, NEW_PARTITION_TABLE_INDEX, &DLA_Table_Entry, FALSE, FALSE, Error_Code);

                                    /* Did we succeed? */
                                    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                    {

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                      return NULL;

                                    }

                                  }

                                  /* Since we have successfully created the partition, increment the count of primary partitions on the drive. */
                                  DriveArray[Index].Primary_Partition_Count++;

                                }
                                else
                                {

                                  /* We must create an EBR.  The EBR must be on a cylinder boundary with the partition starting on the very next track.
                                     Our calculation of the starting sector for the partition has already given us a cylinder aligned starting sector.
                                     We can use that for the EBR, and then adjust the starting sector and size of the partition accordingly.            */

                                  /* Make the EBR. */
                                  New_Partition_Handle = Allocate_Partition_From_Free_Space( Index, PartitionRecord, Starting_Sector, DriveArray[Index].Geometry.Sectors, MBR_EBR, FALSE, NULL, NEW_PARTITION_TABLE_INDEX, NULL, FALSE, FALSE, Error_Code);

                                  /* Did we succeed? */
                                  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                  {

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

                                  /* Adjust Starting_Sector and Size. */
                                  Starting_Sector += DriveArray[Index].Geometry.Sectors;
                                  Size -= DriveArray[Index].Geometry.Sectors;

                                  /* Adjust the DLA_Table_Entry. */
                                  DLA_Table_Entry.Partition_Size = Size;
                                  DLA_Table_Entry.Partition_Start = Starting_Sector;

                                  /* Set the Boot_Indicator to 80h to indicate that this partition is active. */
                                  Partition_Table_Entry.Boot_Indicator = ACTIVE_PARTITION;

                                  /* Set the default Format_Indicator for a new partition. */
                                  Partition_Table_Entry.Format_Indicator = NEW_LOGICAL_DRIVE_FORMAT_INDICATOR;

                                  /* Now make the partition. */
                                  New_Partition_Handle = Find_And_Allocate_Partition( Index, Starting_Sector, Size, Partition, Primary_Partition, &Partition_Table_Entry, NEW_PARTITION_TABLE_INDEX, &DLA_Table_Entry, FALSE, FALSE, Error_Code);

                                  /* Did we succeed? */
                                  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                  {

                                    /* Mark the drive as corrupt since we have an EBR in there with no corresponding partition. */
                                    DriveArray[Index].Corrupt = TRUE;

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

                                  /* Adjust the count of non-primary partitions on the drive. */
                                  DriveArray[Index].Logical_Partition_Count++;

                                }

                                /* For OS/2 to recognize the partition as being unformatted, the first sector of the partition (the boot sector)
                                   must contain all 0xf6.  We will place the LBA of the boot sector for this new partition into the KillSector list
                                   so that, when changes are committed to the disk, this sector will be overwritten.                                  */
                                Sector_To_Overwrite.Drive_Index = Index;
                                Sector_To_Overwrite.Sector_ID = Starting_Sector;

                                InsertItem(KillSector, sizeof(Kill_Sector_Data), &Sector_To_Overwrite, KILL_SECTOR_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

                                /* Did the data make it to the KillSector list? */
                                if ( *Error_Code != DLIST_SUCCESS )
                                {

                                  /* Are we out of memory, or something worse? */
                                  if ( *Error_Code == DLIST_OUT_OF_MEMORY )
                                  {

                                    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

                                  }
                                  else
                                  {

#ifdef DEBUG

#ifdef PARANOID

                                    assert(0);

#else

                                    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

#endif

#endif

                                  }

                                  /* Free the copy of Name. */
                                  free(Name);

                                  API_EXIT("Create_Partition")

                                  /* Report the error! */
                                  return NULL;

                                }

                                /* Mark the DriveArray so that the Commit_Partition_Changes function can write our new partition to disk. */
                                DriveArray[Index].ChangesMade = TRUE;

                                /* If the drive affected had fake volumes on it, convert the fake volumes to real volumes. */
                                if ( DriveArray[Index].Is_PRM && DriveArray[Index].Fake_Volumes_In_Use )
                                {

                                  Convert_Fake_Volumes_On_PRM_To_Real_Volumes( Index, Error_Code);

                                  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                  {

                                    /* Free the copy of Name. */
                                    free(Name);

                                    API_EXIT("Create_Partition")

                                    return NULL;

                                  }

                                }

                                /* Break out of the do-while loop by setting Free_Space_Selected to FALSE. */
                                Free_Space_Selected = FALSE;

                                break;

      case DISK_DRIVE_DATA_TAG : /* We must search the free space on the disk for a block of free space large enough to create the
                                    specified partition, and which meets the requirements for the specified partition.                */

                                 /* Is the size of the new partition greater than 0?  It must be > 0 unless the allocation algorithm is All. */
                                 if ( ( Size == 0 ) && ( algorithm != All ) )
                                 {

                                   *Error_Code = LVM_ENGINE_REQUESTED_SIZE_TOO_SMALL;

                                   /* Free the copy of Name. */
                                   free(Name);

                                   API_EXIT("Create_Partition")

                                   return NULL;

                                 }

                                 /* If the allocation algorithm is Automatic, then convert it to one of the others. */
                                 if ( algorithm == Automatic )
                                 {

                                   /* If we are making a primary partition, try to put it near the beginning of the disk.
                                      If we are making a non-primary partition, try to place it at the end of the disk.    */
                                   if ( Primary_Partition )
                                     algorithm = First_Fit;
                                   else
                                     algorithm = Last_Fit;

                                 }

                                 /* Establish access to the Disk_Drive_Data associated with Handle. */
                                 Drive_Data = (Disk_Drive_Data *) Object;

                                 /* Get the DriveArray index for the drive associated with Handle. */
                                 Index = Drive_Data->DriveArrayIndex;

/* Is the Partitions list for the drive corrupt?  If so, the only legal operation is a delete! */
//EK                             if ( DriveArray[Index].Corrupt )
                                 if ( DriveArray[Index].Corrupt &&  !DriveArray[Index].NonFatalCorrupt)
                                 {

                                   /* Can't create anything on this drive! */
                                   *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                                   /* Free the copy of Name. */
                                   free(Name);

                                   API_EXIT("Create_Partition")

                                   return NULL;

                                 }

                                 /* If the request is for us to make a primary partition, are there any available slots in the MBR? */
                                 if ( Primary_Partition &&
                                      ( ( DriveArray[Index].Primary_Partition_Count == 4 ) ||
                                        ( ( DriveArray[Index].Primary_Partition_Count == 3 ) &&
                                          ( DriveArray[Index].Logical_Partition_Count > 0 )
                                        )
                                      )
                                    )
                                 {

                                   /* The MBR is full!  We can not create a primary partition! */
                                   *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION;

                                   /* Free the copy of Name. */
                                   free(Name);

                                   API_EXIT("Create_Partition")

                                   return NULL;


                                 }

                                 /* If the request is for us to make a non-primary partition, is there already an extended partition or a slot in the MBR for one? */
                                 if ( ( ! Primary_Partition ) && ( DriveArray[Index].Primary_Partition_Count == 4 ) )
                                 {

                                   /* The MBR is filled with Primary Partitions.  This means that there is no extended partition, and we can't create one! */
                                   *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE;

                                   /* Free the copy of Name. */
                                   free(Name);

                                   API_EXIT("Create_Partition")

                                   return NULL;

                                 }

                                 /* Adjust the requested size of the partition so that it is a multiple of Sectors_Per_Cylinder.
                                    Always round up!                                                                               */
                                 if ( Size % DriveArray[Index].Sectors_Per_Cylinder )
                                 {

                                   /* Convert size from sectors to cylinders. */
                                   Size = Size / DriveArray[Index].Sectors_Per_Cylinder;

                                   /* Round up. */
                                   Size++;

                                   /* Now turn size back into sectors. */
                                   Size = Size * DriveArray[Index].Sectors_Per_Cylinder;

                                   /* If we are making a primary partition, we can be 1 track less than a multiple of Sectors_Per_Cylinder.
                                      This will allow us to start a primary partition on the first track after the track containing the MBR.
                                      Without this adjustment, we can get into situations where we have almost an entire cylinder that we
                                      will never allow a partition to be created in.  As an example, if we have a drive where the drive contains
                                      a single extended partition of maximum size, there will be almost a cylinder of free space at the beginning
                                      of the drive.  This is because an extended partition must start on a cylinder boundary and, because of the
                                      MBR, the extended partition can't start until cylinder 1 on the disk.  This leave almost all of cylinder 0
                                      free for use.  A primary partition can start on a track boundary, so a primary partition can be placed
                                      in cylinder 0.  Such a primary partition could be used for Boot Manager, for instance.  Either way, a
                                      primary partition can be either a multiple of the cylinder size, or 1 track less than a multiple of the
                                      cylinder size.                                                                                                   */
                                   if ( Primary_Partition )
                                     Size -= DriveArray[Index].Geometry.Sectors;

                                 }


                                 /* We must select a block of free space on the drive for use in creating the partition specified.  */

                                 Still_Looking = TRUE;

                                 GoToStartOfList(DriveArray[Index].Partitions, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                                    assert( *Error_Code == DLIST_SUCCESS );

#else

                                    /* Were there any errors? */
                                    if ( *Error_Code != DLIST_SUCCESS )
                                    {

                                      /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
                                      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                      /* Free the copy of Name. */
                                      free(Name);

                                      API_EXIT("Create_Partition")

                                      return NULL;

                                    }

#endif

#endif

                                 PartitionRecord = (Partition_Data *) GetObject(DriveArray[Index].Partitions,sizeof(Partition_Data),PARTITION_DATA_TAG, NULL, FALSE, Error_Code);

                                 while ( ( *Error_Code == DLIST_SUCCESS ) && Still_Looking )
                                 {

                                   /* Do we have a block of free space? */
                                   if ( PartitionRecord->Partition_Type == FreeSpace )
                                   {

                                     /* Is this block of free space large enough to hold the partition, or is the allocation algorithm "All". */
                                     if ( ( PartitionRecord->Partition_Size < Size ) && ( algorithm != All ) )
                                     {

                                       /* This block of free space is not acceptable. */
                                       PartitionRecord = NULL;

                                     }
                                     else
                                     {

                                       /* Since we have a block of free space large enough to satisfy the request, set the FreeSpaceAvailable flag. */
                                       FreeSpaceAvailable = TRUE;

                                       /* If we are trying to make a bootable partition, then the block of free space must be able to hold the partition
                                          below the 1024 cylinder limit, if the 1024 cylinder limit applies.                                              */
                                       if ( Bootable  && DriveArray[Index].Cylinder_Limit_Applies )
                                       {

                                         /* See if the partition will end below the 1024 cylinder limit. */
                                         if ( Allocate_From_Start )
                                         {

                                           /* Calculate the end point of the partition if the partition starts at the beginning of the block of free space. */
                                           if ( ( PartitionRecord->Starting_Sector + Size ) > DriveArray[Index].Cylinder_Limit )
                                           {

                                             /* It did not end below the 1024 cylinder limit.  This block of free space is not acceptable. */
                                             PartitionRecord = NULL;

                                             /* Since the Partitions list is an ordered list, the next block of free space we find will place our partition
                                                even further over the 1024 cylinder limit, so there is no need to search the partitions list further.        */
                                             Still_Looking = FALSE;

                                           }

                                         }
                                         else
                                         {

                                           /* Since the partition is allocated from the end of the block of free space, the block of free space must
                                              end below the 1024 cylinder limit in order for the block of free space to be acceptable.                  */
                                           if ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) > DriveArray[Index].Cylinder_Limit )
                                           {

                                             /* The block of free space ended above the 1024 cylinder limit. */
                                             PartitionRecord = NULL;

                                             /* There is no point in looking further as all of the remaining blocks of free space in the partitions
                                                list (if there are any) will end further and further above the 1024 cylinder limit since the
                                                Partitions list is an ordered list based upon the starting sector of each item in the list.         */
                                             Still_Looking = FALSE;

                                           }

                                         }

                                       }

                                     }

                                     /* Do we have an acceptable block of free space to work with? */
                                     if ( PartitionRecord != NULL )
                                     {
                                       /* Can we create the desired type of partition from the block of free space? */
                                       if ( Primary_Partition )
                                       {

                                         if ( ! Can_Be_Primary( PartitionRecord, Error_Code) )
                                         {

                                           /* This block of free space can not be made into a primary partition! */
                                           PartitionRecord = NULL;
                                           Wrong_Partition_Type = TRUE;

                                           /* Clear the error code and continue looking for an appropriate block of free space. */
                                           *Error_Code = LVM_ENGINE_NO_ERROR;

                                         }

                                       }
                                       else
                                       {

                                         if ( ! Can_Be_Non_Primary( PartitionRecord, Error_Code ) )
                                         {

                                           /* This block of free space can not be made into a non-primary partition! */
                                           PartitionRecord = NULL;
                                           Wrong_Partition_Type = TRUE;

                                           /* Clear the error code and continue looking for an appropriate block of free space. */
                                           *Error_Code = LVM_ENGINE_NO_ERROR;

                                         }

                                       }

                                     }

                                     /* Do we have an acceptable block of free space to work with? */
                                     if ( PartitionRecord != NULL )
                                     {

                                       /* Which memory management algorithm are we using? */
                                       switch ( algorithm )
                                       {
                                         case Automatic :
                                         case First_Fit : /* Use the first block of free space to meet the requirements. */
                                                          Selected_Free_Space = PartitionRecord;

                                                          /* Stop looking.  We have found the first block of free space which meets our requirements. */
                                                          Still_Looking = FALSE;

                                                          break;

                                         case Best_Fit : /* Use the block of free space which is closest in size to the partition we are creating. */
                                                         if ( ( Selected_Free_Space == NULL) ||
                                                              ( ( PartitionRecord->Partition_Size - Size ) < (Selected_Free_Space->Partition_Size - Size) )
                                                            )
                                                         {

                                                           Selected_Free_Space = PartitionRecord;

                                                         }

                                                         break;

                                         case Last_Fit : /* Use the last block of free space to meet the requirements. */
                                                         Selected_Free_Space = PartitionRecord;

                                                         break;

                                         case From_Largest : /* Allocate this partition from the largest block of free space on the disk. */
                                                             if ( (Selected_Free_Space == NULL) ||
                                                                  ( PartitionRecord->Partition_Size > Selected_Free_Space->Partition_Size )
                                                                )
                                                             {

                                                               /* PartitionRecord pointed to a larger block of free space, so it becomes our current selection. */
                                                               Selected_Free_Space = PartitionRecord;

                                                             }

                                                             break;

                                         case From_Smallest : /* Allocate this partition from the smallest block of free space on the disk. */
                                                              if ( (Selected_Free_Space == NULL) ||
                                                                   ( PartitionRecord->Partition_Size < Selected_Free_Space->Partition_Size )
                                                                 )
                                                              {

                                                                /* PartitionRecord pointed to a larger block of free space, so it becomes our current selection. */
                                                                Selected_Free_Space = PartitionRecord;

                                                              }

                                                              break;

                                         case All : /* Allocate the entire disk as a single partition. */

                                                    /* How many items are in the Partitions list?  If only 1, then the drive has never been partitioned and
                                                       we can make the entire drive into a single partition.  If only 2, then the first entry must be the MBR
                                                       and the second entry must be free space - the very entry we are processing now.  So here too, we can
                                                       make the entire drive into a single partition.  If there are more than 2, then we can not make the drive
                                                       into a single partition and we must abort with an error.                                                  */

                                                    if ( ( GetListSize( DriveArray[Index].Partitions, Error_Code ) < 3 ) && ( *Error_Code == DLIST_SUCCESS ) )
                                                    {

                                                      /* We can turn the drive into a single partition!  Lets do it. */
                                                      Selected_Free_Space = PartitionRecord;
                                                      Size = PartitionRecord->Partition_Size;

                                                      /* No need to look further. */
                                                      Still_Looking = FALSE;

                                                    }
                                                    else
                                                    {

                                                      /* We can not turn the drive into a single partition, as requested.  Abort. */
                                                      *Error_Code = LVM_ENGINE_BAD_ALLOCATION_ALGORITHM;

                                                      /* Free the copy of Name. */
                                                      free(Name);

                                                      API_EXIT("Create_Partition")

                                                      return NULL;

                                                    }

                                                    break;
                                         default: /* Bad algorith! */
                                                  *Error_Code = LVM_ENGINE_BAD_ALLOCATION_ALGORITHM;

                                                  /* Free the copy of Name. */
                                                  free(Name);

                                                  API_EXIT("Create_Partition")

                                                  return NULL;

                                       }

                                     }

                                   }

                                   if ( Still_Looking )
                                   {

                                     /* Move on to the next item in the list. */
                                     PartitionRecord = (Partition_Data *) GetNextObject(DriveArray[Index].Partitions, sizeof(Partition_Data), PARTITION_DATA_TAG, Error_Code );

                                   }

                                 }

                                 if ( *Error_Code != DLIST_SUCCESS )
                                 {

                                   if ( *Error_Code == DLIST_END_OF_LIST )
                                   {

                                     /* Has a block of free space been chosen yet?  If not, then there are no acceptable blocks of free space. */
                                     if ( Selected_Free_Space == NULL )
                                     {

                                       /* No block of free space was chosen!  Abort. */

                                       /* Were there blocks of free space large enough to satisfy the request?  If so, then they must have been rejected
                                          due to the 1024 cylinder limit.                                                                                 */
                                       if ( FreeSpaceAvailable )
                                       {

                                         /* Was there a block of free space available below the 1024 cylinder limit but which could not be turned into the proper partition type? */
                                         if ( Wrong_Partition_Type )
                                         {

                                           /* What type of partition were we trying to create? */
                                           if ( Primary_Partition )
                                             *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION;
                                           else
                                             *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE;

                                         }
                                         else
                                           *Error_Code = LVM_ENGINE_1024_CYLINDER_LIMIT;

                                       }
                                       else
                                         *Error_Code = LVM_ENGINE_NOT_ENOUGH_FREE_SPACE;

                                       /* Free the copy of Name. */
                                       free(Name);

                                       API_EXIT("Create_Partition")

                                       return NULL;

                                     }
                                     else
                                     {

                                       /* We found an acceptable block of free space.  Lets prepare to use it. */
                                       *Error_Code = LVM_ENGINE_NO_ERROR;

                                     }

                                   }
                                   else
                                   {

#ifdef DEBUG

#ifdef PARANOID

                                     assert( 0 );

#endif

#endif

                                     /* We have an unexpected error!  Abort. */
                                     *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                     /* Free the copy of Name. */
                                     free(Name);

                                     API_EXIT("Create_Partition")

                                     return NULL;

                                   }

                                 }

                                 /* Do we have an acceptable block of free space? */
                                 if ( Selected_Free_Space == NULL )
                                 {

                                   /* Were there blocks of free space large enough to satisfy the request?  If so, then they must have been rejected
                                      due to the 1024 cylinder limit.                                                                                 */
                                   if ( FreeSpaceAvailable )
                                   {

                                     /* Was there a block of free space available below the 1024 cylinder limit but which could not be turned into the proper partition type? */
                                     if ( Wrong_Partition_Type )
                                     {

                                       /* What type of partition were we trying to create? */
                                       if ( Primary_Partition )
                                         *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION;
                                       else
                                         *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE;

                                     }
                                     else
                                       *Error_Code = LVM_ENGINE_1024_CYLINDER_LIMIT;

                                   }
                                   else
                                     *Error_Code = LVM_ENGINE_NOT_ENOUGH_FREE_SPACE;

                                   /* Free the copy of Name. */
                                   free(Name);

                                   API_EXIT("Create_Partition")

                                   return NULL;

                                 }

                                 /* Set Object to point to the block of free space we have chosen. */
                                 Object = (ADDRESS) Selected_Free_Space;

                                 /* Set ObjectTag to indicate that Object now points to a Partition_Data structure. */
                                 ObjectTag = PARTITION_DATA_TAG;

                                 /* Now set Free_Space_Selected so that we will return to the top of the do-while loop and create the
                                    partition from the block of free space that we have selected.                                      */
                                 Free_Space_Selected = TRUE;

                                 break;

      case VOLUME_DATA_TAG :
                             *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                             /* Free the copy of Name. */
                             free(Name);

                             API_EXIT("Create_Partition")

                             return NULL;

      default: /* Internal Error! */

#ifdef DEBUG

#ifdef PARANOID

               assert(0);

#endif

#endif

               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

               /* Free the copy of Name. */
               free(Name);

               API_EXIT("Create_Partition")

               return NULL;

    }

  } while ( Free_Space_Selected );


  /* Free the copy of Name. */
  free(Name);

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Create_Partition")

  return New_Partition_Handle;

}


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
void Delete_Partition( ADDRESS Partition_Handle, CARDINAL32 * Error_Code )
{

  Partition_Data   * PartitionRecord; /* Used to point to the Partition_Data structure we are manipulating. */
  ADDRESS            Object;          /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                ObjectTag;       /* Used when translating the Partition_Handle into a Partition_Data structure. */


  API_ENTRY("Delete_Partition")

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Delete_Partition")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Partition_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Delete_Partition")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != PARTITION_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Delete_Partition")

    return;

  }

  /* Establish access to the Partition_Data structure we want to manipulate. */
  PartitionRecord = ( Partition_Data * ) Object;

  /* Check to see if we can delete this partition!  */

  /* Does this partition record represent free space? */
  if ( PartitionRecord->Partition_Type == FreeSpace )
  {

    /* Free Space can't be deleted! */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Delete_Partition")

    return;

  }

  /* If the partition is part of a volume, we can not delete the partition until the after the
     volume has been deleted.                                                                                                                  */
  if ( PartitionRecord->Volume_Handle != NULL )
  {

    /* This partition is still part of a volume.  If the volume had been deleted, then Volume_Handle, External_Volume_Handle, and
       Volume_Partition_Handle would all be NULL.                                                                                  */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Delete_Partition")

    return;

  }

  /* Is this the Boot Manager partition? */
  if ( Partition_Handle == Boot_Manager_Handle )
  {

    /* Since this is the Boot Manager Partition that is being deleted, lets call the Remove_Boot_Manager function. */
    Remove_Boot_Manager(Error_Code);

    API_EXIT("Delete_Partition")

    return;

  }

  /* If this partition is an FT Mirror partition, then we can do nothing with it.  FTADMIN must be used to delete/modify/use
     this partition.  We must leave it alone!                                                                                 */
  if ( strncmp( PartitionRecord->File_System_Name, "FT HPFS", FILESYSTEM_NAME_SIZE ) == 0 )
  {

    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Delete_Partition")

    return;

  }

  /* Since this partition can be deleted, lets do it. */

  /* To delete the partition, we must first know if it is a primary or not.  If it is a primary partition, after we delete it we must update
     the Primary_Partition_Count for the drive.  If this is not a primary partition, then we must also delete the corresponding EBR which
     points to this partition.                                                                                                                */

  /* Is this partition a primary? */
  if ( ! PartitionRecord->Primary_Partition )
  {

    /* This is NOT a primary partition.  We must find the EBR which corresponds to this partition and delete it also. */

    /* Delete the EBR */
    Delete_EBR(PartitionRecord, Error_Code);

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      /* Our partitions list for the drive has been corrupted! */
      API_EXIT("Delete_Partition")

      return;

    }

  }

  /* Now we can delete the partition by changing its Partition_Data record into one for free space. */
  PartitionRecord->Partition_Type = FreeSpace;

  memset(&(PartitionRecord->Partition_Table_Entry),0,sizeof(Partition_Record) );
  memset(&(PartitionRecord->DLA_Table_Entry),0,sizeof(DLA_Entry) );
  PartitionRecord->File_System_Name[0] = 0;
  PartitionRecord->Usable_Size = PartitionRecord->Partition_Size;
  strcpy(PartitionRecord->Partition_Name,"");

  /* Now we must coalesce this block of free space with any adjacent blocks of free space. */
  Coalesce_Free_Space( PartitionRecord, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == LVM_ENGINE_NO_ERROR );

#else

  /* Was there an error? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* This should not happen, but if it does, we are in trouble! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    API_EXIT("Delete_Partition")

    return;

  }

#endif

#endif


  /* Was this partition a primary partition? */
  if ( PartitionRecord->Primary_Partition )
  {

    /* It was.  We must adjust the Primary_Partition_Count for the drive. */
    DriveArray[PartitionRecord->Drive_Index].Primary_Partition_Count--;

  }
  else
  {

    /* We had a non-primary partition.  We must adjust the Logical_Partition_Count for the drive. */
    DriveArray[PartitionRecord->Drive_Index].Logical_Partition_Count--;

  }

  /* Was the partition on a PRM?  If so, then we must see if the PRM was big floppy formatted.
     If it was, it isn't any longer, so we can turn off the Is_Big_Floppy flag.                  */
  if ( DriveArray[PartitionRecord->Drive_Index].Is_PRM && DriveArray[PartitionRecord->Drive_Index].Is_Big_Floppy )
    DriveArray[PartitionRecord->Drive_Index].Is_Big_Floppy = FALSE;

  /* Set the change flag for the drive to indicate that we have altered the contents of that drives partition information. */
  DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  /* All done. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Delete_Partition")

  return;

}


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
ADDRESS Get_Partition_Handle( CARDINAL32 Serial_Number, CARDINAL32 * Error_Code )
{

  CARDINAL32                Index;         /* Used to walk the DriveArray. */
  Partition_Search_Record   Search_Record; /* Used when searching the partitions list of a drive. */

  FUNCTION_ENTRY("Get_Partition_Handle")

  /* Set up the Search_Record. */
  Search_Record.Serial_Number = Serial_Number;
  Search_Record.Handle = NULL;

  /* We must examine each partition record on each drive until we find one with
     a serial number that matches Serial_Number.                                 */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    ForEachItem(DriveArray[Index].Partitions,&Find_Partition, &Search_Record, TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Get_Partition_Handle")

      return NULL;

    }

    /* Did we find a match? */
    if ( Search_Record.Handle != NULL )
    {

      /* We have a match! */

      /* Indicate success. */
      *Error_Code = LVM_ENGINE_NO_ERROR;

      FUNCTION_EXIT("Get_Partition_Handle")

      /* Return the handle. */
      return Search_Record.Handle;

    }

  }

  /* If we get here, then there were no errors, but a matching partition was not found. */
  *Error_Code = LVM_ENGINE_PARTITION_NOT_FOUND;

  FUNCTION_EXIT("Get_Partition_Handle")

  /* If we get here, then we did not find a partition with a matching serial number. */
  return NULL;

}


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
void Set_Active_Flag ( ADDRESS      Partition_Handle,
                       BYTE         Active_Flag,
                       CARDINAL32 * Error_Code
                     )
{

  Partition_Data   * PartitionRecord; /* Used to point to the Partition_Data structure we are manipulating. */
  ADDRESS            Object;          /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                ObjectTag;       /* Used when translating the Partition_Handle into a Partition_Data structure. */


  API_ENTRY("Set_Active_Flag")

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Set_Active_Flag")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Partition_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Set_Active_Flag")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != PARTITION_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Set_Active_Flag")

    return;

  }

  /* Establish access to the Partition_Data structure we want to manipulate. */
  PartitionRecord = ( Partition_Data * ) Object;

  /* Is the Partitions list for the drive corrupt?  If so, the only legal operation is a delete! */
  if ( DriveArray[PartitionRecord->Drive_Index].Corrupt && !DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt)
  {

    /* Can't create anything on this drive! */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Set_Active_Flag")

    return;

  }

  /* Now set the Active Flag field. */
  PartitionRecord->Partition_Table_Entry.Boot_Indicator = Active_Flag;

  /* Set the change flag for the drive to indicate that we have altered the contents of that drives partition information. */
  DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  /* All done. */

  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Set_Active_Flag")

  return;

}


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
void Set_OS_Flag ( ADDRESS      Partition_Handle,
                   BYTE         OS_Flag,
                   CARDINAL32 * Error_Code
                 )
{

  Partition_Data   * PartitionRecord; /* Used to point to the Partition_Data structure we are manipulating. */
  ADDRESS            Object;          /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                ObjectTag;       /* Used when translating the Partition_Handle into a Partition_Data structure. */


  API_ENTRY("Set_OS_Flag")

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("Set_OS_Flag")

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Partition_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Set_OS_Flag")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != PARTITION_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    API_EXIT("Set_OS_Flag")

    return;

  }

  /* Establish access to the Partition_Data structure we want to manipulate. */
  PartitionRecord = ( Partition_Data * ) Object;

  /* Is the Partitions list for the drive corrupt?  If so, the only legal operation is a delete! */
          
  if ( DriveArray[PartitionRecord->Drive_Index].Corrupt && !DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt)
  {

    /* Can't create anything on this drive! */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    API_EXIT("Set_OS_Flag")

    return;

  }

  /* Now set the OS Flag field. */
  PartitionRecord->Partition_Table_Entry.Format_Indicator = OS_Flag;

  /* Now set the filesystem name field to correspond to the OS Flag we just set. */
  Translate_Format_Indicator(PartitionRecord);

  /* Set the change flag for the drive to indicate that we have altered the contents of that drives partition information. */
  DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  /* All done. */

  *Error_Code = LVM_ENGINE_NO_ERROR;

  API_EXIT("Set_OS_Flag")

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Partition_Information_Record                 */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Set_Partition_Information_Record( Partition_Information_Record * Partition_Information, Partition_Data * PartitionRecord)
{

  CARDINAL32                    Index;            /* Used to access the DriveArray. */

  FUNCTION_ENTRY("Set_Partition_Information_Record")

  /* Zero out the Partition Information Record. */
  memset(Partition_Information,0, sizeof(Partition_Information_Record) );

  /* Setup to copy the relevant fields into the appropriate location in ReturnValue. */
  Index = PartitionRecord->Drive_Index;

  /* Copy the names (partition, filesystem, disk, and volume). */
  strncpy(Partition_Information->Partition_Name,PartitionRecord->Partition_Name, PARTITION_NAME_SIZE);
  strncpy(Partition_Information->File_System_Name,PartitionRecord->File_System_Name, FILESYSTEM_NAME_SIZE);
  strncpy(Partition_Information->Volume_Name,PartitionRecord->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);

  /* Do we have an aggregate?  If so, then there is no disk name! */
  if ( Index < DriveCount )
    strncpy(Partition_Information->Drive_Name,DriveArray[Index].Drive_Name,DISK_NAME_SIZE);

  /* Copy other values that don't need checking or calculation. */
  Partition_Information->Partition_Serial_Number = PartitionRecord->DLA_Table_Entry.Partition_Serial_Number;
  Partition_Information->Partition_Start = PartitionRecord->Starting_Sector;
  Partition_Information->True_Partition_Size = PartitionRecord->Partition_Size;
  Partition_Information->Usable_Partition_Size = PartitionRecord->Usable_Size;
  Partition_Information->Volume_Handle = PartitionRecord->External_Volume_Handle;
  Partition_Information->Primary_Partition = PartitionRecord->Primary_Partition;
  Partition_Information->Active_Flag = PartitionRecord->Partition_Table_Entry.Boot_Indicator;
  Partition_Information->OS_Flag = PartitionRecord->Partition_Table_Entry.Format_Indicator;
  Partition_Information->Partition_Handle = PartitionRecord->External_Handle;

  /* Do we have an aggregate?  If so, then there is no drive handle! */
  if ( Index < DriveCount )
    Partition_Information->Drive_Handle = DriveArray[Index].External_Handle;

  /* Is this Partition on the Boot Manager Menu, but is not part of a Volume? */
  if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu && ( PartitionRecord->Volume_Handle == NULL) )
    Partition_Information->On_Boot_Manager_Menu = TRUE;
  else
    Partition_Information->On_Boot_Manager_Menu = FALSE;

  /* Set values that depend upon whether or not this Partition_Data represents a block of free space or a real partition. */
  if ( PartitionRecord->Partition_Type == FreeSpace )
  {

    /* Since this Partition_Data represents free space, LVM_Interface.H specifies that these values be 0. */
    Partition_Information->Partition_Type = 0;
    Partition_Information->Partition_Status = 0;

    /* Now calculate the Boot_Limit for this block of free space. */

    /* Does the 1024 Cylinder limit apply?  If this is an aggregate, then, by definition, it always applies since you can't
       boot from an LVM volume.                                                                                               */
    if ( ( Index >= DriveCount ) || DriveArray[Index].Cylinder_Limit_Applies )
    {

      /* The Boot Limit is that portion of the block of free space which lies below the 1024 cylinder limit. */

      /* Calculate how much of this block of free space lies above and below the 1024 cylinder limit. */
      if ( ( Index >= DriveCount) || ( PartitionRecord->Starting_Sector >= DriveArray[Index].Cylinder_Limit ) )
      {

        /* The entire block of free space is above the 1024 cylinder limit.  The Boot Limit is therefore 0 since nothing bootable can be
           allocated from this block of free space.                                                                                        */
        Partition_Information->Boot_Limit = 0;

      } else  {

        /* Some (or all) of the block of free space lies below the 1024 cylinder limit. */
        if ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[Index].Cylinder_Limit )
        {

          /* The entire block of free space is under the 1024 cylinder limit. */
          Partition_Information->Boot_Limit = PartitionRecord->Partition_Size;

        }
        else
        {

          /* Only part of the block of free space lies below the 1024 cylinder limit.  */

          /* Calculate the number of sectors which lie below the 1024 cylinder limit. */
          Partition_Information->Boot_Limit = DriveArray[Index].Cylinder_Limit - PartitionRecord->Starting_Sector;

        }
//EK
//todo
//DEBUG
  Partition_Information->Boot_Limit = 0;

      }

    }
    else
    {

      /* There is no 1024 cylinder limit for this drive!  The entire block of free space can be used for a bootable partition. */
      Partition_Information->Boot_Limit = PartitionRecord->Partition_Size;

    }

  }
  else
  {

    /* Since this Partition_Data represents a real partition, we need to set these values accordingly. */

    /* If this is an LVM partition, its Format_Indicator will be LVM_PARTITION_INDICATOR. */
    if ( PartitionRecord->Partition_Table_Entry.Format_Indicator == LVM_PARTITION_INDICATOR )
    {

      Partition_Information->Partition_Type = 1;

    }
    else
    {

      Partition_Information->Partition_Type = 2;

    }

    /* To determine the Partition_Status, we need to see if the Volume_Handle is NULL.  If it is NULL, then this partition is not part of
       a volume.  If it is NOT NULL, then this partition is part of a volume and is therefore in use.  The only exceptions to this are the
       Boot Manager partition and any FT Mirror partitions, which are always considered to be "in use".                                                                   */
    if ( PartitionRecord->Volume_Handle == NULL )
    {

      /* Is this the Boot Manager partition, or is this an FT Mirror partition? */
      if ( ( PartitionRecord->External_Handle == Boot_Manager_Handle ) ||
           (  strncmp( PartitionRecord->File_System_Name, "FT HPFS", FILESYSTEM_NAME_SIZE ) == 0 )
         )
      {

        /* Mark this partition as being "In Use" so that it can not be selected to be part of a volume. */
        Partition_Information->Partition_Status = 1;

      }
      else
      {

        /* The partition represented by this Partition_Data is not assigned to a volume and is therefore "available". */
        Partition_Information->Partition_Status = 2;

      }

      /* Since this partition is not part of a volume, we can set Spanned_Volume to FALSE. */
      Partition_Information->Spanned_Volume = FALSE;

    }
    else
    {

      /* The partition represented by this Partition_Data is part of a volume and is therefore "in use". */
      Partition_Information->Partition_Status = 1;

      /* Since this partition is part of a volume, we need to set the Spanned_Volume and Volume_Drive_Letter fields. */
      Partition_Information->Spanned_Volume = PartitionRecord->Spanned_Volume;
      Partition_Information->Volume_Drive_Letter = PartitionRecord->DLA_Table_Entry.Drive_Letter;

    }

    /* Since this is a real partition, you can't allocate anything from it, therefore its Boot_Limit is 0. */
    Partition_Information->Boot_Limit = 0;

  }

  FUNCTION_EXIT("Set_Partition_Information_Record")

  return;

}


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
void New_MBR( ADDRESS Drive_Handle, CARDINAL32 * Error_Code )
{

  ADDRESS                       Object;
  TAG                           ObjectTag;
  Disk_Drive_Data *             Drive_Data;
  Partition_Data  *             MBR_Data;


  API_ENTRY("New_MBR")

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    API_EXIT("New_MBR")

    return;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Drive_Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    API_EXIT("New_MBR")

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case DISK_DRIVE_DATA_TAG : /* We have Disk_Drive_Data here. */
                               Drive_Data = ( Disk_Drive_Data * ) Object;

                               /* If the drive is corrupt, or if the drive is a PRM in big floppy mode, then we will not allow this option to be performed. */
                               if ( Drive_Data->Corrupt || Drive_Data->Is_Big_Floppy )
                               {

                                 /* Indicate that this operation is not available. */
                                 *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                                 API_EXIT("New_MBR")

                                 return;

                               }

                               /* We need to change the New_Partition flag on the MBR entry in the Partitions list.  If there
                                  is no entry for an MBR in the Partitions list, then we must make one.                        */

                               /* If there is more than one entry in the Partitions list, then one of those entries must be the MBR. */
                               if ( GetListSize(Drive_Data->Partitions, Error_Code) > 1 )
                               {

                                 /* There must be an entry for an MBR in the Partitions list.  All Partitions lists start with
                                    a single entry, which represents the entire drive as free space.  As partitions are created,
                                    they are "carved" out of that block of free space, and the number of items in the list grows.
                                    Since you must have an MBR before you can have partitions, if you have more than 1 item in the
                                    Partitions list, then one of those items must be an MBR.  Furthermore, since the MBR is always
                                    at LBA 0, it will always be the first item in the list.                                         */
                                 GoToStartOfList ( Drive_Data->Partitions, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

                                 assert(*Error_Code == DLIST_SUCCESS);

#else

                                 if ( *Error_Code != DLIST_SUCCESS )
                                 {

                                   /* This should never happen!  Our internal data structures must be corrupt! */
                                   *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                   API_EXIT("New_MBR")

                                   return;

                                 }
#endif

#endif

                                 MBR_Data = GetObject( Drive_Data->Partitions, sizeof(Partition_Data), PARTITION_DATA_TAG, NULL, FALSE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                                 assert(*Error_Code == DLIST_SUCCESS);

#else

                                 if ( *Error_Code != DLIST_SUCCESS )
                                 {

                                   /* This should never happen!  Our internal data structures must be corrupt! */
                                   *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                   API_EXIT("New_MBR")

                                   return;

                                 }
#endif

#endif

                                 /* Set the New_Partition flag.  This will cause Commit_Partition_Changes to write out a new MBR
                                    when it sees this entry in the Partitions list.                                               */
                                 MBR_Data->New_Partition = TRUE;

                                 /* Indicate that we have made changes to the drive associated with this DriveArray entry. */
                                 Drive_Data->ChangesMade = TRUE;

                               }
                               else
                               {

#ifdef DEBUG
                                 /* Are we here because GetListSize failed? */

#ifdef PARANOID

                                 assert(*Error_Code == DLIST_SUCCESS);

#else

                                 if ( *Error_Code != DLIST_SUCCESS )
                                 {

                                   /* This should never happen!  Our internal data structures must be corrupt! */
                                   *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                   API_EXIT("New_MBR")

                                   return;

                                 }
#endif

#endif

                                 /* There is no MBR on the drive!  We must create one! */

                                 /* MBRs always reside at LBA 0.  We will allocate a full track for the MBR.  The actual MBR will be created, on disk,
                                    by the Commit_Partition_Changes function.                                                                            */

                                 /* Allocate the MBR. */
                                 Find_And_Allocate_Partition( Drive_Data->DriveArrayIndex, 0, Drive_Data->Geometry.Sectors, MBR_EBR, FALSE, NULL, NEW_PARTITION_TABLE_INDEX, NULL, FALSE, FALSE, Error_Code);

                                 /* If there were no errors, then set the ChangesMade flag so that our new MBR will get written to disk. */
                                 if ( *Error_Code == LVM_ENGINE_NO_ERROR )
                                 {

                                   /* Indicate that we have made changes to the drive associated with this DriveArray entry. */
                                   Drive_Data->ChangesMade = TRUE;

                                 }

                               }

                               break;

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */

                              /* You can only place a new MBR on a drive, not on a partition!  Return an error. */
                              *Error_Code = LVM_ENGINE_BAD_HANDLE;
                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */

                           /* You can only place a new MBR on a drive, not on a volume!  Return an error. */
                           *Error_Code = LVM_ENGINE_BAD_HANDLE;
                           break;

    default : /* If it is not one of the above, then it is an internal error! */

  #ifdef DEBUG

  #ifdef PARANOID

  assert(0);

  #else
              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  #endif

  #endif
              API_EXIT("New_MBR")

              return;

              /* Keep the compiler happy. */
              break;
  }

  /* All done. */

  API_EXIT("New_MBR")

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
CARDINAL32 Get_Partition_Options(Partition_Data * PartitionRecord, CARDINAL32 * Error)
{
  CARDINAL32   ReturnValue = 0;            /* Used to hold the bitmap that we will be returning to the caller. */
  BOOLEAN      Unusable_DiskSpace = TRUE;  /* Used to keep track of whether or not we can create a partition from a particular block of free space. */


  FUNCTION_ENTRY("Get_Partition_Options")

   /* Does the partition record represent free space? */
   if ( PartitionRecord->Partition_Type == FreeSpace )
   {

     /* If the drive that this free space is on is corrupted, then we can do nothing with this block of free space. */
     if ( DriveArray[PartitionRecord->Drive_Index].Corrupt &&  !DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt)
     {

       FUNCTION_EXIT("Get_Partition_Options")

       /* There is nothing we can do with this free space, so return. */
       return ReturnValue;

     }

     /* Since this is free space, can we make it into a primary partition? */
     if ( Can_Be_Primary( PartitionRecord, Error ) && ( *Error == LVM_ENGINE_NO_ERROR ) )
     {

       /* Turn on the bit which indicates that we can make a primary partition out of this block of free space. */
       ReturnValue += CREATE_PRIMARY_PARTITION;

       /* Since we can create a partition, the disk space is not unusable. */
       Unusable_DiskSpace = FALSE;

     }

#ifdef DEBUG

#ifdef PARANOID

     assert( *Error != LVM_ENGINE_INTERNAL_ERROR );

#else

     /* Was there an error? */
     if ( *Error == LVM_ENGINE_INTERNAL_ERROR )
     {

       FUNCTION_EXIT("Get_Partition_Options")

       /* This should not have happened!  We must have an internal error. */
       return ReturnValue;

     }

#endif

#endif

     /* Can we turn this free space into a non-primary partition? */
     if ( Can_Be_Non_Primary( PartitionRecord, Error ) && ( *Error == LVM_ENGINE_NO_ERROR ) )
     {

       /* Turn on the bit which indicates that we can make a non-primary partition out of this block of free space. */
       ReturnValue += CREATE_LOGICAL_DRIVE;

       /* Since we can create a partition, the disk space is not unusable. */
       Unusable_DiskSpace = FALSE;

     }

#ifdef DEBUG

#ifdef PARANOID

     assert( *Error != LVM_ENGINE_INTERNAL_ERROR );

#else

     /* Was there an error? */
     if ( *Error == LVM_ENGINE_INTERNAL_ERROR )
     {

       FUNCTION_EXIT("Get_Partition_Options")

       /* This should not have happened!  We must have an internal error. */
       return ReturnValue;

     }

#endif

#endif

     /* We need to determine if a partition created from this block of free space can be made bootable. */
     if ( ! Unusable_DiskSpace )
     {

       /* Does this partition have a problem with the 1024 cylinder limit? */
       if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
            ( ( PartitionRecord->Starting_Sector + Min_Install_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
          )
       {

         /* Is Boot Manager installed and active? */
         if ( ( Boot_Manager_Handle != NULL ) && Boot_Manager_Active )
         {

           /* The partition should be bootable from Boot Manager.  Set the appropriate flags. */

           if ( ReturnValue && CREATE_PRIMARY_PARTITION )
           {

             ReturnValue += CAN_BOOT_PRIMARY;

           }

           if ( ReturnValue && CREATE_LOGICAL_DRIVE )
           {

             ReturnValue += CAN_BOOT_LOGICAL;

           }

         }
         else
         {

           /* To be bootable without Boot Manager, the partition must be a primary on the first drive in the system. */
//EK           
//todo
//         if ( ( PartitionRecord->Drive_Index == 0 ) && ( ReturnValue && CREATE_PRIMARY_PARTITION ) )
//EK debug
           if ( /*( PartitionRecord->Drive_Index == 0 ) && */ ( ReturnValue && CREATE_PRIMARY_PARTITION ) )
           {

             /* This block of free space is on the first drive, and enough of it is below the 1024 cylinder to limit to install and boot OS/2. */
             ReturnValue += CAN_BOOT_PRIMARY;

           }

         }

       }

     }

   } else {

     /* Since this is not free space, what else can we do with it? */

     /* If this partition is an FT Mirror partition, then we can do nothing with it.  FTADMIN must be used to delete/modify/use
        this partition.  We must leave it alone!                                                                                 */
     if (  strncmp( PartitionRecord->File_System_Name, "FT HPFS", FILESYSTEM_NAME_SIZE ) == 0 )
     {

       /* Indicate success. */
       *Error = LVM_ENGINE_NO_ERROR;

       FUNCTION_EXIT("Get_Partition_Options: FT Mirror partition")

       return ReturnValue;

     }

     /* If this partition is NOT part of a volume, then we can delete it, and maybe even add it
        to the Boot Manager menu, if Boot Manager is installed and active.                       */
     if ( PartitionRecord->Volume_Handle == NULL )
     {

       /* We can delete this partition. */
       ReturnValue += DELETE_PARTITION;

  if ( Logging_Enabled )
  {
    sprintf(Log_Buffer,"Get_Partition_Options: PartitionRecord->Drive_Index %x, Corrupt= %x nonFatal=%x\n", 
        PartitionRecord->Drive_Index, DriveArray[PartitionRecord->Drive_Index].Corrupt, DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt);
    Write_Log_Buffer();
  }     
       /* If the drive that this partition resides on is corrupt, then we can do nothing more with this partition. */
       if ( ! DriveArray[PartitionRecord->Drive_Index].Corrupt || DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt)
       {

         /* Since the drive was not corrupt, lets see if Boot Manager is installed? */
         if ( ( Boot_Manager_Handle != NULL ) && ( Boot_Manager_Handle != PartitionRecord->External_Handle ) && Boot_Manager_Active )
         {

           /* Boot Manager is installed!  We don't need to worry about whether or not the partition
              is a primary partition, and we don't need to worry about what disk it resides on.  Is
              the partition below the 1024 cylinder limit?  Does the 1024 cylinder limit even apply? */
           if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
                ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
              )
           {

             /* We can add this partition to the Boot Manager menu.  Is it already on the Boot Manager menu? */
             if ( ! PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
             {
               ReturnValue += ADD_TO_BOOT_MANAGER_MENU;

             }

             /* Since we can add this partition to the boot manager menu, it can be booted from.  Set the appropriate bits. */
             if ( PartitionRecord->Primary_Partition )
               ReturnValue += CAN_BOOT_PRIMARY;
             else
               ReturnValue += CAN_BOOT_LOGICAL;

           }

           /* If this partition is already on the Boot Manager menu, then we can remove it. */
           if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
             ReturnValue += REMOVE_FROM_BOOT_MANAGER_MENU;

         }

         /* If this partition is a primary partition, then we can make it the active primary partition, if it is not already. */
         if ( PartitionRecord->Primary_Partition )
         {

           /* Is it already marked active? */
           if ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) == 0 )
           {

             /* Since this partition is not marked active, we can mark it active.  Also, since it is
                a primary partition, it can be the active primary for the drive if it is marked active. */
             ReturnValue += SET_ACTIVE_PRIMARY;
             ReturnValue += SET_PARTITION_ACTIVE;

           }

         }
         else
         {

           /* Is it already marked active? */
           if ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) == 0 )
             ReturnValue += SET_PARTITION_ACTIVE;

         }

         /* If the partition is currently marked active, then we can mark it inactive. */
         if ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) != 0 )
           ReturnValue += SET_PARTITION_INACTIVE;

         /* To be set startable, a partition must be a primary partition on the first drive in the system. */
         if ( ( PartitionRecord->Primary_Partition ) && ( PartitionRecord->Drive_Index == 0 ) )
         {

           /* Is the partition already marked startable? */
           if ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator & 0x80 ) == 0 )
           {
             ReturnValue += SET_STARTABLE;

           }

           /* If the partition lies below the 1024 cylinder limit, then it can be bootable! */
           if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
                ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
              )
           {

             /* The CAN_BOOT_PRIMARY flag may already be set if Boot Manager is installed.  Use an 'OR' instead of addition to turn on the bit. */
             ReturnValue = ReturnValue | CAN_BOOT_PRIMARY;

           }

         }

       }

     }

     /* If this is not the Boot Manager partition, then we can change the name of the partition. */
     if ( PartitionRecord->Partition_Table_Entry.Format_Indicator != BOOT_MANAGER_INDICATOR )
     {

       ReturnValue += CAN_SET_NAME;

     }

   }

   /* Indicate success. */
   *Error = LVM_ENGINE_NO_ERROR;

   FUNCTION_EXIT("Get_Partition_Options")

   return ReturnValue;

}


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
                                                        )
{

  Partition_Information_Record   Data;

  Data = Get_Partition_Information( (ADDRESS) Partition_Handle, Error_Code);

  *Partition_Information = Data;

  return;

}


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
                                                      char      *  _Seg16   Name,
                                                      Allocation_Algorithm  algorithm,
                                                      BOOLEAN               Bootable,
                                                      BOOLEAN               Primary_Partition,
                                                      BOOLEAN               Allocate_From_Start,
                                                      CARDINAL32 * _Seg16   Error_Code
                                                    )
{

  return (CARDINAL32) Create_Partition( (ADDRESS) Handle, Size, Name, algorithm, Bootable, Primary_Partition, Allocate_From_Start, Error_Code);

}


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
                                              )
{

  Delete_Partition( (ADDRESS) Partition_Handle, Error_Code );

  return;

}


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
                                              )
{

  Set_Active_Flag( (ADDRESS) Partition_Handle, Active_Flag, Error_Code);

  return;

}


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
                                          )
{

  Set_OS_Flag( (ADDRESS) Partition_Handle, OS_Flag, Error_Code );

  return;

}


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
void _Far16 _Pascal _loadds NEW_MBR16( CARDINAL32 Drive_Handle, CARDINAL32 * _Seg16 Error_Code )
{

  New_MBR( (ADDRESS) Drive_Handle, Error_Code );

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partition_Features                           */
/*                                                                   */
/*   Descriptive Name: Returns the feature ID information for each of*/
/*                     the features that are installed on the        */
/*                     partition.                                    */
/*                                                                   */
/*   Input: Partition_Data * PartitionRecord - The partition to use. */
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
Feature_Information_Array _System Get_Partition_Features( Partition_Data * PartitionRecord, CARDINAL32 * Error_Code )
{

  DLIST                         Features_List;
  Feature_Information_Array     Feature_Information;
  CARDINAL32                    Ignore_Error;

  FUNCTION_ENTRY("Get_Partition_Features")

  /* Initialize Feature_Information assuming failure. */
  Feature_Information.Count = 0;
  Feature_Information.Feature_Data = NULL;

  /* We need to get a list of features on the partition. */
  Features_List = CreateList();
  if ( Features_List == NULL )
  {

    /* We are out of memory!  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FUNCTION_EXIT("Get_Partition_Features")

    return Feature_Information;

  }

  /* Get the features on the partition. */
  Build_Features_List( PartitionRecord, Features_List, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    FUNCTION_EXIT("Get_Partition_Features")

    return Feature_Information;

  }

  /* Now that we have the features on the partition, sort them by their Feature Sequence Number.  This will ensure that
     they will be applied to any new partitions in the correct order.                                                   */
  SortList(Features_List, &Sort_By_Feature_Sequence_Number, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    /* Set the error code. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Partition_Features")

    return Feature_Information;

  }

  /* Now we can allocate the array for returning all of the features we found. */
  Feature_Information.Count = GetListSize(Features_List, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Free the Features_List list. */
    DestroyList(&Features_List,FALSE, &Ignore_Error);

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Get_Partition_Features")

    return Feature_Information;

  }

  /* PassThru and BBR are always in the Available Features array but should not be reported. */
  if ( Feature_Information.Count > 0 )
  {

    /* Allocate memory. */
    Feature_Information.Feature_Data = (Feature_ID_Data *) malloc( Feature_Information.Count * sizeof(Feature_ID_Data) );

    if ( Feature_Information.Feature_Data == NULL )
    {

      /* Free the FeaturesToUse list. */
      DestroyList(&Features_List,FALSE, &Ignore_Error);

      /* We are out of memory! */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FUNCTION_EXIT("Get_Partition_Features")

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

      FUNCTION_EXIT("Get_Partition_Features")

      return Feature_Information;

    }

    /* Were any features placed into the Feature_Information?  If not, then free the memory! */
    if ( Feature_Information.Count == 0 )
    {

      free(Feature_Information.Feature_Data);
      Feature_Information.Feature_Data = NULL;

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

  FUNCTION_EXIT("Get_Partition_Features")

  return Feature_Information;

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
static ADDRESS Find_And_Allocate_Partition( CARDINAL32         DriveArrayIndex, /* The index in the Drive Array to use. */
                                            LBA                StartingSector,  /* The LBA at which the partition starts. */
                                            CARDINAL32         Size,            /* The size of the partition in sectors. */
                                            Partition_Types    Partition_Type,  /* The type of the partition - Partition, or MBR_EBR */
                                            BOOLEAN            Primary_Partition,
                                            Partition_Record * Partition_Table_Entry,
                                            CARDINAL16         Partition_Table_Index,
                                            DLA_Entry  *       DLA_Table_Entry,
                                            BOOLEAN            Prior_Existing_Partition,
                                            BOOLEAN            Migration_Needed,
                                            CARDINAL32 *       Error_Code)
{

  Partition_Data *  Free_Space;

  FUNCTION_ENTRY("Find_And_Allocate_Partition")

  /* We must find the block of free space which contains the specified LBA. */

  GoToStartOfList(DriveArray[DriveArrayIndex].Partitions,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* This should never happen!  We have an internal error. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Find_And_Allocate_Partition")

    return NULL;

  }

#endif

#endif

  /* Get the first item in the list. */
  Free_Space = (Partition_Data *) GetObject(DriveArray[DriveArrayIndex].Partitions,sizeof(Partition_Data), PARTITION_DATA_TAG, NULL, FALSE, Error_Code);

  /* Process the list looking for a Partition_Data record which holds the starting address requested. */
  while ( ( ( Free_Space->Starting_Sector + Free_Space->Partition_Size - 1) < StartingSector ) && ( *Error_Code == DLIST_SUCCESS ) )
  {

    Free_Space = (Partition_Data *) GetNextObject(DriveArray[DriveArrayIndex].Partitions,sizeof(Partition_Data), PARTITION_DATA_TAG, Error_Code);

  }

  /* Were there any errors returned from the DLIST manager? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* If we have an end of list error, then we searched the entire list but could not find any blocks of free space meeting our specifications! */
    if ( *Error_Code == DLIST_END_OF_LIST )
    {

      /* No block of free space was found which could contain the partition we have been asked to create.  The partition request must be bad. */
      *Error_Code = LVM_ENGINE_BAD_PARTITION;

    }
    else
    {

#ifdef DEBUG

#ifdef PARANOID

      assert(0);

#endif

#endif

      /* This case should never happen!  We must have an internal error! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    }

    FUNCTION_EXIT("Find_And_Allocate_Partition")

    return NULL;

  }

  /* Examine the Partition_Data record we have found.  Is it freespace? */
  if ( Free_Space->Partition_Type != FreeSpace)
  {

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FUNCTION_EXIT("Find_And_Allocate_Partition")

    return NULL;

  }

  /* Since we have a block of free space, is it large enough to hold the partition? */
  if ( ( Free_Space->Starting_Sector + Free_Space->Partition_Size ) < ( StartingSector + Size ) )
  {

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FUNCTION_EXIT("Find_And_Allocate_Partition")

    return NULL;

  }

  FUNCTION_EXIT("Find_And_Allocate_Partition")

  /* Now allocate the partition. */
  return ( Allocate_Partition_From_Free_Space( DriveArrayIndex, Free_Space, StartingSector, Size, Partition_Type, Primary_Partition, Partition_Table_Entry, Partition_Table_Index, DLA_Table_Entry, Prior_Existing_Partition, Migration_Needed, Error_Code ) );

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
static void Translate_Format_Indicator( Partition_Data * PartitionRecord )
{

  /* Establish access to the buffer holding the boot sector read in from disk. */
  struct Extended_Boot * Boot_Check = (struct Extended_Boot *) &Boot_Sector;

  /* Declare a variable to hold the contents of an unformatted Boot Sector's Boot_System_ID field. */
  char                   Unformatted_ID[] = {0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 0xf6, 0x0};

  FUNCTION_ENTRY("Translate_Format_Indicator")

  switch ( PartitionRecord->Partition_Table_Entry.Format_Indicator )
  {

    case 0 : /* Unused/unformatted partition. */
             strcpy(PartitionRecord->File_System_Name,"unformatted");
             break;
    case 1 : /* FAT Filesystem with 12 bit entries. */

             /* Is it unformatted? */
             if ( ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 ) ||
                  ( ( strncmp(Boot_Check->Boot_System_ID,"FAT     ",8) != 0 ) &&
                    ( strncmp(Boot_Check->Boot_System_ID,"FAT12   ",8) != 0 )
                  )
                )
             {

               /* Check for the FORMAT bug.  This bug in format causes a partition to be formatted as
                  HPFS but the Format Indicator in the Partition Table is never updated.                 */

               /* Is it HPFS? */
               if ( strncmp(Boot_Check->Boot_System_ID, "HPFS    ", 8) == 0 )
               {

                 strcpy(PartitionRecord->File_System_Name,"HPFS");

                 /* Fix the FORMAT bug by updating the Format Indicator and marking the drive dirty. */
                 PartitionRecord->Partition_Table_Entry.Format_Indicator = 7;
                 DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

               }
               else
                 strcpy(PartitionRecord->File_System_Name,"unformatted");

             }
             else
               strcpy(PartitionRecord->File_System_Name,"FAT12");

             break;
    case 2 : /* XENIX */
    case 3 : /* XENIX */
    case 0xFF : /* XENIX */
                strcpy(PartitionRecord->File_System_Name,"XENIX");
                break;
    case 4 : /* FAT Filesystem, 16 bit entries, partition size less than 32 MB. */
    case 6 : /* FAT Filesystem, 16 bit entries, partition size is 32MB or more. */

             /* Is it unformatted? */
             if ( ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 ) ||
                  ( ( strncmp(Boot_Check->Boot_System_ID,"FAT     ",8) != 0 ) &&
                    ( strncmp(Boot_Check->Boot_System_ID,"FAT16   ",8) != 0 )
                  )
                )
             {

               /* Check for the FORMAT bug.  This bug in format causes a partition to be formatted as
                  HPFS but the Format Indicator in the Partition Table is never updated.                 */

               /* Is it HPFS? */
               if ( strncmp(Boot_Check->Boot_System_ID, "HPFS    ", 8) == 0 )
               {

                 strcpy(PartitionRecord->File_System_Name,"HPFS");

                 /* Fix the FORMAT bug by updating the Format Indicator and marking the drive dirty. */
                 PartitionRecord->Partition_Table_Entry.Format_Indicator = 7;
                 DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

             }
             else
                 strcpy(PartitionRecord->File_System_Name,"unformatted");

             }
             else
               strcpy(PartitionRecord->File_System_Name,"FAT16");

             break;
    case 7 : /* IFS - could be HPFS, JFS, NTFS, or some other installable filesystem. */

             /* Is it HPFS? */
             if ( strncmp(Boot_Check->Boot_System_ID, "HPFS    ", 8) == 0 )
             {

               strcpy(PartitionRecord->File_System_Name,"HPFS");

             }
             else
             {

               /* Is it JFS? */
               if ( strncmp(Boot_Check->Boot_System_ID, "JFS     ", 8) == 0 )
               {

                 strcpy(PartitionRecord->File_System_Name,"JFS");

               }
               else
               {

                 /* Is it unformatted? */
                 if ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 )
                 {

                   strcpy(PartitionRecord->File_System_Name,"unformatted");

                 }
                 else
                 {

                   /* Is it NTFS? */
                   if ( strncmp( Boot_Check->Boot_OEM, "NTFS    ",8) == 0 )
                   {

                     strcpy(PartitionRecord->File_System_Name,"NTFS");

                   }
                   else
                   {

                     /* We don't know what it is! */
                     strcpy(PartitionRecord->File_System_Name,"IFS");

                   }

                 }

               }

             }

             break;
    case 8 : /* AIX */
    case 9 : /* AIX */
             strcpy(PartitionRecord->File_System_Name,"AIX");
             break;
    case 0xA : /* Boot Manager. */
               strcpy(PartitionRecord->File_System_Name,"Boot Manager");
               break;
    case 0x0B : /* FAT32 */
    case 0x0C : /* FAT32 */
    case 0x1B : /* FAT32 */
    case 0x1C : /* FAT32 */
               strcpy(PartitionRecord->File_System_Name,"FAT32");
               break;
    case 0x10 : /* OPUS */
                strcpy(PartitionRecord->File_System_Name,"OPUS");
                break;
    case 0x11 : /* Boot Manager hidden primary partition of type 1 (FAT 12). */
                /* Is it unformatted? */
                if ( ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 ) ||
                     ( ( strncmp(Boot_Check->Boot_System_ID,"FAT     ",8) != 0 ) &&
                       ( strncmp(Boot_Check->Boot_System_ID,"FAT12   ",8) != 0 )
                     )
                   )
                {

                  strcpy(PartitionRecord->File_System_Name,"unformatted");

                }
                else
                  strcpy(PartitionRecord->File_System_Name,"FAT12-H");
                break;
    case 0x14 : /* Boot Manager hidden primary partition of type 4 (FAT 16, partition size <= 32 MB). */
    case 0x16 : /* Boot Manager hidden primary partition of type 6 (FAT 16, partition size > 32 MB). */

                /* Is it unformatted? */
                if ( ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 ) ||
                     ( ( strncmp(Boot_Check->Boot_System_ID,"FAT     ",8) != 0 ) &&
                       ( strncmp(Boot_Check->Boot_System_ID,"FAT16   ",8) != 0 )
                     )
                   )
                {

                  strcpy(PartitionRecord->File_System_Name,"unformatted");

                }
                else
                  strcpy(PartitionRecord->File_System_Name,"FAT16-H");

                break;
    case 0x17 : /* Boot Manager hidden primary partition of type 7 ( IFS ). */

                /* Is it HPFS? */
                if ( strncmp(Boot_Check->Boot_System_ID, "HPFS    ", 8) == 0 )
                {

                  strcpy(PartitionRecord->File_System_Name,"HPFS-H");

                }
                else
                {

                  /* Is it JFS? */
                  if ( strncmp(Boot_Check->Boot_System_ID, "JFS     ", 8) == 0 )
                  {

                    strcpy(PartitionRecord->File_System_Name,"JFS-H");

                  }
                  else
                  {

                    /* Is it unformatted? */
                    if ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 )
                    {

                      strcpy(PartitionRecord->File_System_Name,"unformatted");

                    }
                    else
                    {

                      /* Is it NTFS? */
                      if ( strncmp( Boot_Check->Boot_OEM, "NTFS    ",8) == 0 )
                      {

                        strcpy(PartitionRecord->File_System_Name,"NTFS-H");

                      }
                      else
                      {

                        /* We don't know what it is! */
                        strcpy(PartitionRecord->File_System_Name,"IFS-H");

                      }

                    }

                  }

                }

                break;
    case 0x24 : /* NEC DOS 3.3 */
                strcpy(PartitionRecord->File_System_Name,"NEC DOS 3.3");
                break;
    case 0x35 : /* LVM Partition - Must check the boot sector to determine the format type. */
                if ( strncmp(Boot_Check->Boot_System_ID,Unformatted_ID,8) == 0 )
                 {

                   strcpy(PartitionRecord->File_System_Name,"unformatted");

                 }
                 else
                 {

                   if ( strncmp(Boot_Check->Boot_System_ID, "HPFS    ", 8) == 0 )
                   {

                     strcpy(PartitionRecord->File_System_Name,"HPFS");

                   }
                   else
                   {

                     if ( strncmp(Boot_Check->Boot_System_ID,"FAT     ",8) == 0 )
                     {

                       strcpy(PartitionRecord->File_System_Name,"FAT");

                     }
                     else
                     {

                       if ( strncmp(Boot_Check->Boot_System_ID,"JFS     ",8) == 0 )
                       {

                         strcpy(PartitionRecord->File_System_Name,"JFS");

                       }
                       else
                         strncpy(PartitionRecord->File_System_Name, Boot_Check->Boot_System_ID, 8);

                     }

                   }

                 }

                 break;
    case 0x40 : /* VENIX */
                strcpy(PartitionRecord->File_System_Name,"VENIX");
                break;
    case 0x50 : /* Disk Manager */
    case 0x51 : /* Disk Manager */
                strcpy(PartitionRecord->File_System_Name,"Disk Manager");
                break;
    case 0x52 : /* CP/M */
    case 0xDB : /* CP/M */
                strcpy(PartitionRecord->File_System_Name,"CP/M");
                break;
    case 0x56 : /* GoldenBow VFeature */
                strcpy(PartitionRecord->File_System_Name,"GoldenBow");
                break;
    case 0xE1 : /* SpeedStor */
    case 0xE4 : /* SpeedStor */
    case 0x61 : /* SpeedStor */
                strcpy(PartitionRecord->File_System_Name,"SpeedStor");
                break;
    case 0x63 : /* Unix System V */
                strcpy(PartitionRecord->File_System_Name,"UNIX");
                break;
    case 0x64 :
    case 0x65 : /* Novell NetWare */
                strcpy(PartitionRecord->File_System_Name,"NOVELL");
                break;
    case 0x75 : /* PC/IX */
                strcpy(PartitionRecord->File_System_Name,"PC/IX");
                break;
    case 0x80 : /* Minix */
                strcpy(PartitionRecord->File_System_Name,"MINIX");
                break;
    case 0x81 : /* LINUX */
    case 0x82 : /* LINUX */
                strcpy(PartitionRecord->File_System_Name,"LINUX");
                break;
    case 0x83 : /* LINUX EXT2 */
                strcpy(PartitionRecord->File_System_Name,"LINUX EXT2");
                break;
    case 0x85 : /* LINUX EXTENDED */
                strcpy(PartitionRecord->File_System_Name,"LINUX EXTENDED");
                break;

    case 0x86 : /* NT Stripe/Volume Set. */
    case 0x87 : /* HPFS Fault Tolerance Mirror Partition or NT Stripe/Volume Set. */
                /* Is it HPFS? */
                if ( strncmp(Boot_Check->Boot_System_ID, "HPFS    ", 8) == 0 )
                {

                  strcpy(PartitionRecord->File_System_Name,"FT HPFS");

                }
                else
                {

                  strcpy(PartitionRecord->File_System_Name,"NT");

                }
                break;
    case 0x8e :   /* LINUX LVM_PARTITION */
                strcpy(PartitionRecord->File_System_Name,"LINUX LVM");
                break;
    case 0x93 : /* Amoeba file system */
    case 0x94 : /* Amoeba bad block table. */
                strcpy(PartitionRecord->File_System_Name,"AMOEBA");
                break;
    case 0xA5 : /* FREEBSD */
                strcpy(PartitionRecord->File_System_Name,"FREEBSD");
                break;
    case 0xA6 : /* OPENBSD */
                strcpy(PartitionRecord->File_System_Name,"OPENBSD");
                break;
    case 0xA7 : /* NETBSD */
                strcpy(PartitionRecord->File_System_Name,"NETBSD");
                break;
    case 0xB7 : /* BSDI */
    case 0xB8 : /* BSDI */
                strcpy(PartitionRecord->File_System_Name,"BSDI");
                break;
    case 0xC1 : /* DR-DOS */
    case 0xC4 : /* DR-DOS */
    case 0xC6 : /* DR-DOS */
                strcpy(PartitionRecord->File_System_Name,"DR-DOS");
                break;
    case 0xC7 : /* Disabled HPFS Fault Tolerance Mirror Partition. */
                strcpy(PartitionRecord->File_System_Name,"FT HPFS");
                break;
    default: /* Unknown filesystem. */
             strcpy(PartitionRecord->File_System_Name,"????");
             break;
  }

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
static BOOLEAN DLA_Table_Is_Valid( CARDINAL32 DriveArrayIndex, BOOLEAN Processing_EBR )
{

  DLA_Table_Sector * DLA_Table = (DLA_Table_Sector *) DLA_Sector;
  BOOLEAN            Valid_DLA_Table = FALSE;
  CARDINAL32         Actual_CRC;
  CARDINAL32         Calculated_CRC;
  BOOLEAN            ChangesMade = FALSE;

  FUNCTION_ENTRY("DLA_Table_Is_Valid")

  /* We must check the signature and the CRC.  If both are good, then we will assume that the DLA Table is valid. */
  if ( ( DLA_Table->DLA_Signature1 == DLA_TABLE_SIGNATURE1 ) && ( DLA_Table->DLA_Signature2 == DLA_TABLE_SIGNATURE2 ) )
  {

    /* The signature is valid.  Is the CRC valid?  To find out, we need to calculate the CRC. */

    /* Before we can accurately calculate the CRC, we must save the CRC in the DLA_Table and set it to 0.  This is
       how the CRC was supposed to have been calculated.                                                            */
    Actual_CRC = DLA_Table->DLA_CRC;
    DLA_Table->DLA_CRC = 0;

    /* Now calculate the CRC. */
    Calculated_CRC = CalculateCRC( INITIAL_CRC, &DLA_Sector, BYTES_PER_SECTOR);

    /* Do the CRCs match? */
    if ( Calculated_CRC == Actual_CRC )
    {

      /* Since the CRCs match, we will assume that the DLA Table is valid. */
      Valid_DLA_Table = TRUE;

    }


  }

  if ( Valid_DLA_Table )
  {

    /* If the DLA Table is valid, then we must fill in some fields in the Disk_Drive_Data structure for the current drive. */

    /* Do we already have a serial number for the drive? */
    if ( DriveArray[DriveArrayIndex].Drive_Serial_Number != 0 )
    {

      /* Does this serial number match the existing one? */
      if ( DriveArray[DriveArrayIndex].Drive_Serial_Number != DLA_Table->Disk_Serial_Number )
      {

        /* We have a disagreement over the serial number of the drive.  We will trust the existing serial number for the drive. */

        /* Mark the drive as having changes made so that the Commit_Changes function will update this DLA table with the correct drive Serial Number. */
        DriveArray[DriveArrayIndex].ChangesMade = TRUE;

      }

    }
    else
    {

      /* Do we have a serial number to save? */
      if ( DLA_Table->Disk_Serial_Number != 0 )
      {

        /* Save the disk serial number. */
        DriveArray[DriveArrayIndex].Drive_Serial_Number = DLA_Table->Disk_Serial_Number;

        /* Indicate that we changed the global data for the drive. */
        ChangesMade = TRUE;

      }

    }

    /* Is there already a drive name? */
    if ( ( DriveArray[DriveArrayIndex].Drive_Name[0] == 0) && ( DLA_Table->Disk_Name[0] != 0 ) )
    {

      /* Save the drive name. */
      strncpy(DriveArray[DriveArrayIndex].Drive_Name, DLA_Table->Disk_Name, DISK_NAME_SIZE);

      /* Indicate that we changed the global data for the drive. */
      ChangesMade = TRUE;

    }

    /* Is there already a Boot Disk Serial Number? */
    if ( DriveArray[DriveArrayIndex].Boot_Drive_Serial_Number == 0 )
    {

      /* Save the Boot Disk Serial Number. */
      DriveArray[DriveArrayIndex].Boot_Drive_Serial_Number = DLA_Table->Boot_Disk_Serial_Number;

      /* Indicate that we changed the global data for the drive. */
      ChangesMade = TRUE;

    }

  }

  /* If we are processing the DLA Table for the MBR, then we need to save the Reboot_Flag and the Install_Flags.  These are only used by the Install program. */
  if ( ( ! Processing_EBR ) && Valid_DLA_Table )
  {

    DriveArray[DriveArrayIndex].Reboot_Flag = DLA_Table->Reboot;
    DriveArray[DriveArrayIndex].Install_Flags = DLA_Table->Install_Flags;

  }

  /* If we are processing an EBR and ChangesMade is TRUE, then the DLA Table for the MBR is non-existent or corrupted!  In this case,
     we can repair it by marking the drive as having changes made.  This will cause all of the DLA Tables to be re-written if Commit_Changes is called. */
  if ( ChangesMade && Processing_EBR )
  {

    /* Mark the drive as having changes made so that the Commit_Changes function will update the DLA tables. */
    DriveArray[DriveArrayIndex].ChangesMade = TRUE;

  }

  FUNCTION_EXIT("DLA_Table_Is_Valid")

  return Valid_DLA_Table;

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
static BOOLEAN Partition_Table_Entry_In_Use ( Partition_Record * Partition_Table_Entry )
{

  BYTE *      ByteArray;    /* We wish to check each byte in the Partition Table Entry.  This will allow us to do that easily. */
  CARDINAL32  I;            /* Used to index into the ByteArray. */

  FUNCTION_ENTRY("Partition_Table_Entry_In_Use")

  /* Set ByteArray to point to the beginning of the Partition Table Entry. */
  ByteArray = (BYTE *) Partition_Table_Entry;

  /* Examine each byte in the Partition Table Entry. */
  for ( I = 0; I < sizeof(Partition_Record); I++)
  {

    /* If any byte in the Partition Table Entry is not 0, then we must assume that the partition table entry is in use. */
    if ( ByteArray[I] != 0)
    {

      /* We found a non-zero byte! */
      FUNCTION_EXIT("Partition_Table_Entry_In_Use")

      return TRUE;

    }

  }


  FUNCTION_EXIT("Partition_Table_Entry_In_Use")

  /* Every byte in the partition table entry was 0.  The entry must not be in use. */
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
void _System Count_Eligible_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  CARDINAL32 *      ItemCount = ( CARDINAL32 * ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *  PartitionRecord = (Partition_Data *) Object;


  FUNCTION_ENTRY("Count_Eligible_Partitions")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Count_Eligible_Partitions")

    return;

  }

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition or an eligible block of free space? */
  if ( ( PartitionRecord->Partition_Type == Partition ) ||
       ( ( PartitionRecord->Partition_Type == FreeSpace ) &&
         ( PartitionRecord->Partition_Size >= Min_Free_Space_Size )
       )
     )
  {

    /* We have an eligible entry!  Count it. */
    *ItemCount += 1;

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Count_Eligible_Partitions")

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
static void _System Transfer_Partition_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  Partition_Information_Array * ReturnValue = ( Partition_Information_Array * ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *              PartitionRecord = (Partition_Data *) Object;



  FUNCTION_ENTRY("Transfer_Partition_Data")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Transfer_Partition_Data")

    return;

  }

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition or an eligible block of free space? */
  if ( ( PartitionRecord->Partition_Type == Partition ) ||
       ( ( PartitionRecord->Partition_Type == FreeSpace ) &&
         ( PartitionRecord->Partition_Size >= Min_Free_Space_Size )
       )
     )
  {

    Set_Partition_Information_Record( &( ReturnValue->Partition_Array[ReturnValue->Count] ), PartitionRecord);

    ReturnValue->Count++;

  }

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
static void Coalesce_Free_Space( Partition_Data * Free_Space, CARDINAL32 * Error_Code)
{
  Partition_Data *  PreviousRecord;        /* Used to access entries in the Partitions list which are before Free_Space. */
  Partition_Data *  NextRecord;            /* Used to access entries in the Partitions list which are after Free_Space. */
  DLIST             Partition_List;        /* Used to avoid excess pointer dereferences when manipulating the Partitions list for the drive. */

  FUNCTION_ENTRY("Coalesce_Free_Space")

  /* Initialize Partition_List using the Partitions list for the drive containing Free_Space. */
  Partition_List = DriveArray[Free_Space->Drive_Index].Partitions;

  /* We have a block of free space here.  If there is an adjacent block of free space prior to this block of free space, then
     we need to merge to two in order for our disk space allocation scheme to work correctly.                                 */

  /* Make the Partition_Data for Free_Space the current item in the Partition_List. */
  GoToSpecifiedItem(Partition_List,Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Coalesce_Free_Space")

    return;

  }

#endif

#endif

  /* Now that Free_Space is the current item in the list, lets find out who preceeds it. */

  /* Get the Partition_Data structure which immediately preceeds Free_Space from the Partition_List. */
  PreviousRecord = ( Partition_Data * ) GetPreviousObject(Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There should always be a prior item in the list since there should always be an entry for the MBR!  We should only be called
       when a partition has been deleted, which means there must have been, as a minimum, an MBR.                                    */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Coalesce_Free_Space")

    return;

  }

#endif

#endif

  /* Does the previous item in the list describe free space also? */
  if ( PreviousRecord->Partition_Type == FreeSpace )
  {

    /* Is the free space described by PreviousRecord contiguous with the free space described by Free_Space? */
    if ( ( PreviousRecord->Starting_Sector + PreviousRecord->Partition_Size ) == Free_Space->Starting_Sector )
    {

      /* They are contiguous!  Combine the two! */
      Free_Space->Partition_Size += PreviousRecord->Partition_Size;
      Free_Space->Usable_Size = Free_Space->Partition_Size;
      Free_Space->Starting_Sector = PreviousRecord->Starting_Sector;

    }

    /* Remove PreviousRecord from the list but don't delete him quite yet. */
    DeleteItem(Partition_List,FALSE,NULL,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS );

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    }

#endif

#endif

    /* Now lets get rid of PreviousRecord's handles. */
    Destroy_Handle(PreviousRecord->External_Handle,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == HANDLE_MANAGER_NO_ERROR );

#else

    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    }

#endif

#endif


    /* Zero out PreviousRecord. */
    memset( (ADDRESS) PreviousRecord,0,sizeof(Partition_Data) );

    /* Free the memory associated with PreviousRecord. */
    free(PreviousRecord);

  }

  /* We now need to examine the Partition_Data record which follows Free_Space to see if it can be merged with Free_Space. */

  /* Make the Partition_Data for Free_Space the current item in the Partition_List. */
  GoToSpecifiedItem(Partition_List,Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Were there any errors? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Coalesce_Free_Space")

    return;

  }

#endif

#endif

  /* Now that Free_Space is the current item in the list, lets find out who follows it. */

  /* Get the Partition_Data structure which immediately follows Free_Space from the Partition_List. */
  NextRecord = ( Partition_Data * ) GetNextObject(Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( ( *Error_Code == DLIST_SUCCESS ) || ( *Error_Code == DLIST_END_OF_LIST ) );

#else

  /* Were there any errors? */
  if ( ( *Error_Code != DLIST_SUCCESS ) && ( *Error_Code != DLIST_END_OF_LIST ) )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Coalesce_Free_Space")

    return;

  }

#endif

#endif

  /* Does the next item in the list (if there is one) describe free space also? */
  if ( ( *Error_Code == DLIST_SUCCESS ) && ( NextRecord->Partition_Type == FreeSpace ) )
  {

    /* Is the free space described by NextRecord contiguous with the free space described by Free_Space? */
    if ( ( Free_Space->Starting_Sector + Free_Space->Partition_Size ) == NextRecord->Starting_Sector )
    {

      /* They are contiguous!  Combine the two! */
      Free_Space->Partition_Size += NextRecord->Partition_Size;
      Free_Space->Usable_Size = Free_Space->Partition_Size;

    }

    /* Remove NextRecord from the list but don't delete him quite yet. */
    DeleteItem(Partition_List,FALSE,NULL,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

#else

    /* Were there any errors? */
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Coalesce_Free_Space")

      return;

    }

#endif

#endif

    /* Now lets get rid of NextRecord's handles. */
    Destroy_Handle(NextRecord->External_Handle,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == HANDLE_MANAGER_NO_ERROR );

#else

  /* Were there any errors? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Coalesce_Free_Space")

    return;

  }

#endif

#endif

    /* Zero out NextRecord. */
    memset( (ADDRESS) NextRecord,0,sizeof(Partition_Data) );

    /* Free the memory associated with NextRecord. */
    free(NextRecord);

  }

  /* All done.  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Coalesce_Free_Space")

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
static void Delete_EBR( Partition_Data * PartitionRecord, CARDINAL32 * Error_Code)
{

  Partition_Data *  PreviousRecord;        /* Used to access entries in the Partitions list which are before PartitionRecord. */
  DLIST             Partition_List;        /* Used to avoid excess pointer dereferences when manipulating the Partitions list for the drive. */
  Kill_Sector_Data  Sector_To_Kill;        /* Used to add the DLA table to the KillSector list. */


  FUNCTION_ENTRY("Delete_EBR")

  /* Initialize Partition_List using the Partitions list for the drive containing PartitionRecord. */
  Partition_List = DriveArray[PartitionRecord->Drive_Index].Partitions;


  /* We must find the EBR that corresponds to the partition described by PartitionRecord.  The EBR should immediately preceed
     PartitionRecord in the Partitions list.                                                                                   */


  /* Make PartitionRecord the current item in the Partition_List. */
  GoToSpecifiedItem(Partition_List,PartitionRecord->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Were there any errors? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Delete_EBR")

    return;

  }

#endif

#endif

  /* We must find the EBR.  The EBR will appear in the Partitions list prior to the non-primary partition it defines.  The
     only thing that may come between the EBR and its partition is free space, and this should only occur if the drive
     was partitioned on a machine which used a different geometry than the machine being used to delete the partition.
     Either way, we will work backwards until we find an EBR.                                                                  */

  /* Prepare for the search. */
  *Error_Code = DLIST_SUCCESS;

  /* Do the search. */
  do
  {

    /* Get the previous Partition_Data structure in the list. */
    PreviousRecord = ( Partition_Data * ) GetPreviousObject(Partition_List, sizeof(Partition_Data), PARTITION_DATA_TAG, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );
    assert( PreviousRecord->Partition_Type != Partition );

#else

    if ( ( *Error_Code != DLIST_SUCCESS ) || ( PreviousRecord->Partition_Type == Partition ) )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Delete_EBR")

      return;

    }

#endif

#endif

  } while ( (*Error_Code == DLIST_SUCCESS ) && ( PreviousRecord->Partition_Type != MBR_EBR )  );

  /* Before we can delete the EBR, we must add the sector for its DLA table to the KillSector list. */
  Sector_To_Kill.Drive_Index = PreviousRecord->Drive_Index;
  Sector_To_Kill.Sector_ID = PreviousRecord->Starting_Sector + PreviousRecord->Partition_Size - 1;

  /* Add the sector to the KillSector list. */
  InsertItem(KillSector, sizeof(Kill_Sector_Data), &Sector_To_Kill, KILL_SECTOR_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have a problem! */

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
    {

      /* Translate this to its LVM Engine equivalent. */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    }
    else
    {

#ifdef DEBUG

#ifdef PARANOID

      assert(0);

#else

      /* The only expected possible error is out of memory.  Since we did not get that, then we have
         and internal error of some sort.                                                             */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

#endif

#endif

    }

    FUNCTION_EXIT("Delete_EBR")

    return;

  }

  /* Now we can delete the EBR by changing its Partition_Data record into one for free space. */

  /* Since a Partition_Data record for free space has an external handle, we must create an external handle for our "new" block of free space. */
  PreviousRecord->External_Handle = Create_Handle((ADDRESS) PreviousRecord, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
    {

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
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

    FUNCTION_EXIT("Delete_EBR")

    return;

  }

  /* Begin changing the Partition_Data record from that of an MBR_EBR to that for Free Space. */
  PreviousRecord->Partition_Type = FreeSpace;

  memset(&(PreviousRecord->Partition_Table_Entry),0,sizeof(Partition_Record) );
  memset(&(PreviousRecord->DLA_Table_Entry),0,sizeof(DLA_Entry) );
  PreviousRecord->File_System_Name[0] = 0;
  PreviousRecord->Usable_Size = PreviousRecord->Partition_Size;
  strcpy(PreviousRecord->Partition_Name,"");

  /* Now we must coalesce this block of free space with any adjacent blocks of free space. */
  Coalesce_Free_Space( PreviousRecord, Error_Code);

  /* Return whatever error code we got from Coalesce_Free_Space. */

  FUNCTION_EXIT("Delete_EBR")

  return;

}


/* This is a worker!  It expects all necessary checking to have been done already.  It just takes the specified block of free space
   and carves a partition out of it.                                                                                                */
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
static ADDRESS Allocate_Partition_From_Free_Space( CARDINAL32         DriveArrayIndex, /* The index in the Drive Array to use. */
                                                   Partition_Data *   Free_Space,      /* The block of free space to allocate the partition from. */
                                                   LBA                StartingSector,  /* The LBA at which the partition starts. */
                                                   CARDINAL32         Size,            /* The size of the partition in sectors. */
                                                   Partition_Types    Partition_Type,  /* The type of the partition - Partition, or MBR_EBR */
                                                   BOOLEAN            Primary_Partition,
                                                   Partition_Record * Partition_Table_Entry,
                                                   CARDINAL16         Partition_Table_Index,
                                                   DLA_Entry  *       DLA_Table_Entry,
                                                   BOOLEAN            Prior_Existing_Partition,
                                                   BOOLEAN            Migration_Needed,
                                                   CARDINAL32 *       Error_Code)
{

  Partition_Data *  New_Partition;
  ADDRESS           New_Partition_Handle = NULL;  /* Used to hold the external handle of the new partition. */


  FUNCTION_ENTRY("Allocate_Partition_From_Free_Space")

  /* Make Free_Space the current item in the Partitions list. */
  GoToSpecifiedItem(DriveArray[DriveArrayIndex].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == DLIST_SUCCESS);

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

    return NULL;

  }

#endif

#endif

  /* Is the new partition smaller than the block of free space? */
  if ( Size < Free_Space->Partition_Size )
  {

    /* Does the Partition start at the beginning of the block of free space? */
    if ( Free_Space->Starting_Sector == StartingSector )
    {

      /* The new partition starts at the beginning of our block of free space. */

      /* Allocate memory for the new Partition_Data record. */
      New_Partition = (Partition_Data *) malloc( sizeof (Partition_Data) );
      if (New_Partition == NULL)
      {

        /* We have a problem here.  We are out of memory. */
        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

        return NULL;

      }

      /* Set the Partition_Data for the new partition equal to that of our Free_Space. */
      *New_Partition = *Free_Space;

      /* Now lets turn *New_Partition into a partition. */
      New_Partition->Partition_Size = Size;
      New_Partition->Usable_Size = Size;
      New_Partition->New_Partition = Prior_Existing_Partition ? FALSE : TRUE;
      New_Partition->Primary_Partition = Primary_Partition;
      New_Partition->Partition_Type = Partition_Type;
      New_Partition->Migration_Needed = Migration_Needed;

      /* Is there a partition table entry to accompany this partition? */
      if ( Partition_Table_Entry != NULL)
      {

        /* Save the Partition Table entry for this partition. */
        New_Partition->Partition_Table_Entry = *Partition_Table_Entry;

        /* Set the File_System_Name. */
        Translate_Format_Indicator( New_Partition );

      }
      else
      {

        /* Since there was no partition table entry for this partition, zero out the Partition_Table_Entry field. */
        memset(&(New_Partition->Partition_Table_Entry),0,sizeof(Partition_Record) );

      }

      /* Save the Partition Table Index. */
      New_Partition->Partition_Table_Index = Partition_Table_Index;

      /* Is there a DLA Table entry for this partition? */
      if ( DLA_Table_Entry != NULL )
      {

        /* Use the provided DLA Table entry. */
        New_Partition->DLA_Table_Entry = *DLA_Table_Entry;

        /* Use the DLA Table entry to fill in the Partition_Name. */
        strncpy(New_Partition->Partition_Name,DLA_Table_Entry->Partition_Name, PARTITION_NAME_SIZE);

      }
      else
      {

        /* Since there was no DLA Table entry for this partition, 0 out the DLA_Table_Entry field. */
        memset( &(New_Partition->DLA_Table_Entry), 0, sizeof(DLA_Entry) );

      }

      /* Now lets correct Free_Space's starting point and size. */
      Free_Space->Partition_Size -= Size;
      Free_Space->Usable_Size = Free_Space->Partition_Size;
      Free_Space->Starting_Sector = New_Partition->Starting_Sector + Size;

      /* Now lets insert New_Partition into the Partitions List before Free_Space as this will maintain the ordering of the Partitions list.
         Free_Space is the current item in the list. */
      New_Partition->Drive_Partition_Handle = InsertObject(DriveArray[DriveArrayIndex].Partitions,sizeof(Partition_Data),(ADDRESS)New_Partition,PARTITION_DATA_TAG, NULL, InsertBefore, TRUE, Error_Code);
      if ( *Error_Code != DLIST_SUCCESS )
      {

        /* Was the error due to a lack of memory? */
        if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        {

          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        }
        else
        {

#ifdef DEBUG

#ifdef PARANOID

          assert(0);

#endif

#endif

          /* This is unexpected.  We must have an internal error! */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        }

        FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

        return NULL;

      }

      /* If this partition is not an MBR/EBR, then we need to get an external handle for it. */
      if ( Partition_Type != MBR_EBR )
      {

        /* Now get an external handle for New_Partition. */
        New_Partition->External_Handle = Create_Handle((ADDRESS) New_Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);
        if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
        {

          if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
          {

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
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

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Save the external handle of the new partition. */
        New_Partition_Handle = New_Partition->External_Handle;

      }

    }
    else
    {

      /* The partition does not start at the begininng of the block of free space.  Does the Partition end at the end of the block of free space? */
      if ( ( Free_Space->Starting_Sector + Free_Space->Partition_Size ) == ( StartingSector + Size ) )
      {

        /* In this case, we will adjust the size of Free_Space and use New_Partition to represent our new partition. */

        /* Allocate memory for the new Partition_Data record. */
        New_Partition = (Partition_Data *) malloc( sizeof (Partition_Data) );
        if (New_Partition == NULL)
        {

          /* We have a problem here.  We are out of memory. */
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Initialize the new Partition_Data record. */
        *New_Partition = *Free_Space;

        /* Adjust the size of Free_Space. */
        Free_Space->Partition_Size -= Size;
        Free_Space->Usable_Size = Free_Space->Partition_Size;

        /* Now alter New_Partition so that it describes our partition instead of a block of free space. */
        New_Partition->Starting_Sector = Free_Space->Starting_Sector + Free_Space->Partition_Size;
        New_Partition->Partition_Size = Size;
        New_Partition->Usable_Size = Size;

        New_Partition->New_Partition = Prior_Existing_Partition ? FALSE : TRUE;
        New_Partition->Primary_Partition = Primary_Partition;
        New_Partition->Partition_Type = Partition_Type;
        New_Partition->Migration_Needed = Migration_Needed;

        /* Is there a partition table entry to accompany this partition? */
        if ( Partition_Table_Entry != NULL )
        {

          /* Save the partition table entry. */
          New_Partition->Partition_Table_Entry = *Partition_Table_Entry;

          /* Set the File_System_Name. */
          Translate_Format_Indicator( New_Partition );

        }
        else
        {

          /* Since there was no partition table entry to accompany this partition, zero out the Partition_Table_Entry field. */
          memset(&(New_Partition->Partition_Table_Entry),0,sizeof(Partition_Record) );

        }

        /* Save the Partition Table Index. */
        New_Partition->Partition_Table_Index = Partition_Table_Index;

        /* Is there a DLA Table entry for this partition? */
        if ( DLA_Table_Entry != NULL )
        {

          /* Use the provided DLA Table entry. */
          New_Partition->DLA_Table_Entry = *DLA_Table_Entry;

          /* Use the DLA Table entry to fill in the Partition_Name. */
          strncpy(New_Partition->Partition_Name,New_Partition->DLA_Table_Entry.Partition_Name, PARTITION_NAME_SIZE);

        }
        else
        {

          /* Since there was no DLA Table entry for this partition, 0 out the DLA_Table_Entry field. */
          memset( &(New_Partition->DLA_Table_Entry), 0, sizeof(DLA_Entry) );

        }

        /* Now lets insert New_Partition into the Partitions List after Free_Space as this will maintain the ordering of the Partitions list.
           Free_Space is the current item in the list. */
        New_Partition->Drive_Partition_Handle = InsertObject(DriveArray[DriveArrayIndex].Partitions,sizeof(Partition_Data),(ADDRESS) New_Partition,PARTITION_DATA_TAG, NULL, InsertAfter, TRUE, Error_Code);
        if ( *Error_Code != DLIST_SUCCESS )
        {

          /* Was the error due to a lack of memory? */
          if ( *Error_Code == DLIST_OUT_OF_MEMORY )
          {

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          }
          else
          {

#ifdef DEBUG

#ifdef PARANOID

            assert(0);

#endif

#endif
            /* This is unexpected.  We must have an internal error! */
            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          }

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* If this partition is not an MBR/EBR, then we need to get an external handle for it. */
        if ( Partition_Type != MBR_EBR )
        {

          /* Now get an external handle for New_Partition. */
          New_Partition->External_Handle = Create_Handle((ADDRESS) New_Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);
          if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
          {

            if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
            {

              *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

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

            FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

            return NULL;

          }

          /* Save the external handle of the new partition. */
          New_Partition_Handle = New_Partition->External_Handle;

        }

      }
      else
      {

        /* Worst case.  We must break the block of free space into three parts - Free space, the partition, and free space. */

        /* Allocate memory for the new Partition_Data record. */
        New_Partition = (Partition_Data *) malloc( sizeof (Partition_Data) );
        if (New_Partition == NULL)
        {

          /* We have a problem here.  We are out of memory. */
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Initialize New_Partition to match Free_Space. */
        *New_Partition = *Free_Space;

        /* New_Partition is currently equal to Free_Space.  We will turn New_Partition into the block of free space which preceeds the partition. */
        New_Partition->Partition_Size = StartingSector - New_Partition->Starting_Sector;
        New_Partition->Usable_Size = New_Partition->Partition_Size;

        /* Adjust the size and starting position of Free_Space. */
        Free_Space->Partition_Size -= New_Partition->Partition_Size;
        Free_Space->Usable_Size = Free_Space->Partition_Size;
        Free_Space->Starting_Sector = StartingSector;

        /* Now lets insert New_Partition into the Partitions List before Free_Space as this will maintain the ordering of the Partitions list.
           Free_Space is the current item in the list. */
        New_Partition->Drive_Partition_Handle = InsertObject(DriveArray[DriveArrayIndex].Partitions,sizeof(Partition_Data),(ADDRESS) New_Partition,PARTITION_DATA_TAG, NULL, InsertBefore, TRUE, Error_Code);
        if ( *Error_Code != DLIST_SUCCESS )
        {

          /* Was the error due to a lack of memory? */
          if ( *Error_Code == DLIST_OUT_OF_MEMORY )
          {

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          }
          else
          {

#ifdef DEBUG

#ifdef PARANOID

            assert(0);

#endif

#endif

            /* This is unexpected.  We must have an internal error! */
            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          }

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Now get an external handle for New_Partition. */
        New_Partition->External_Handle = Create_Handle((ADDRESS) New_Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);
        if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
        {

          if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
          {

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

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

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Make Free_Space the current item in the list again. */
        NextItem(DriveArray[DriveArrayIndex].Partitions,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

        assert( *Error_Code == DLIST_SUCCESS );

#else

        /* Were there any errors? */
        if ( *Error_Code != DLIST_SUCCESS )
        {

          /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

#endif

#endif

        /* Allocate memory for another Partition_Data record. */
        New_Partition = (Partition_Data *) malloc( sizeof (Partition_Data) );
        if (New_Partition == NULL)
        {

          /* We have a problem here.  We are out of memory. */
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Initialize New_Partition so that it matches Free_Space. */
        *New_Partition = *Free_Space;

        /* Now turn New_Partition into the block of free space which follows the partition being allocated. */
        New_Partition->Partition_Size = Free_Space->Partition_Size - Size;
        New_Partition->Usable_Size = New_Partition->Partition_Size;
        New_Partition->Starting_Sector = Free_Space->Starting_Sector + Size;

        /* Update the size of Free_Space. */
        Free_Space->Partition_Size = Size;
        Free_Space->Usable_Size = Size;

        /* Now lets insert New_Partition into the Partitions List after Free_Space as this will maintain the ordering of the Partitions list.
           Free_Space is the current item in the list. */
        New_Partition->Drive_Partition_Handle = InsertObject(DriveArray[DriveArrayIndex].Partitions,sizeof(Partition_Data),(ADDRESS) New_Partition,PARTITION_DATA_TAG, NULL, InsertAfter, TRUE, Error_Code);
        if ( *Error_Code != DLIST_SUCCESS )
        {

          /* Was the error due to a lack of memory? */
          if ( *Error_Code == DLIST_OUT_OF_MEMORY )
          {

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          }
          else
          {

#ifdef DEBUG

#ifdef PARANOID

            assert(0);

#endif

#endif

            /* This is unexpected.  We must have an internal error! */
            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          }

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }

        /* Now get an external handle for New_Partition. */
        New_Partition->External_Handle = Create_Handle((ADDRESS) New_Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);
        if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
        {

          if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
          {

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

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

          FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

          return NULL;

        }


        /* Now lets finish turning Free_Space into a partition. */
        Free_Space->Partition_Type = Partition_Type;
        Free_Space->New_Partition = Prior_Existing_Partition ? FALSE : TRUE;
        Free_Space->Primary_Partition = Primary_Partition;
        Free_Space->Migration_Needed = Migration_Needed;

        /* Is there a partition table entry to go with this partition? */
        if ( Partition_Table_Entry != NULL )
        {

          /* Save the partition table entry. */
          Free_Space->Partition_Table_Entry = *Partition_Table_Entry;

          /* Set the File_System_Name. */
          Translate_Format_Indicator( Free_Space );

        }
        else
        {

          /* Since there is no partition table entry for this partition, zero out the Partition_Table_Entry field. */
          memset( &(Free_Space->Partition_Table_Entry),0,sizeof(Partition_Record) );

        }

        /* Save the Partition Table Index. */
        Free_Space->Partition_Table_Index = Partition_Table_Index;

        /* Is there a DLA Table entry for this partition? */
        if ( DLA_Table_Entry != NULL )
        {

          /* Use the provided DLA Table entry. */
          Free_Space->DLA_Table_Entry = *DLA_Table_Entry;

          /* Use the DLA Table entry to fill in the Partition_Name. */
          strncpy(Free_Space->Partition_Name,Free_Space->DLA_Table_Entry.Partition_Name, PARTITION_NAME_SIZE);

        }
        else
        {

          /* Since there was no DLA Table entry for this partition, 0 out the DLA_Table_Entry field. */
          memset( &(Free_Space->DLA_Table_Entry), 0, sizeof(DLA_Entry) );

        }

        /* If this partition is an EBR, then it should not have an external handle.  */
        if ( Partition_Type == MBR_EBR )
        {

          /* We must dispose of the external handle for this partition record. */
          Destroy_Handle(Free_Space->External_Handle,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

          assert (*Error_Code == HANDLE_MANAGER_NO_ERROR);

#else

          if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
          {

            /* Abort.  We have some kind of internal error here! */
            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

            FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

            return NULL;

          }

#endif

#endif

          Free_Space->External_Handle = NULL;

        }
        else
        {

          /* Save the external handle of the new partition. */
          New_Partition_Handle = Free_Space->External_Handle;

        }

      } /* End of "worst" case - breaking 1 block of free space into two blocks of free space and a partition. */

    } /* End of the case where the partition and free space end at the same address. */

  } /* End of the case where the partition is not the same size as the block of free space. */
  else
  {

    /* This is the easy case!  The block of free space is the same size as the new partition.  We just need to convert the Partition_Data for
       the block of free space into that required by the new partition.                                                                       */
    Free_Space->Partition_Type = Partition_Type;
    Free_Space->New_Partition = Prior_Existing_Partition ? FALSE : TRUE;
    Free_Space->Primary_Partition = Primary_Partition;
    Free_Space->Migration_Needed = Migration_Needed;

    /* Is there a partition table entry for this partition? */
    if ( Partition_Table_Entry != NULL )
    {

      /* Save the partition table entry. */
      Free_Space->Partition_Table_Entry = *Partition_Table_Entry;

      /* Set the File_System_Name. */
      Translate_Format_Indicator( Free_Space );

    }
    else
    {

      /* Since there was no partition table entry for this partition, zero out the Partition_Table_Entry field. */
      memset(&(Free_Space->Partition_Table_Entry),0,sizeof(Partition_Record) );

    }

    /* Save the Partition Table Index. */
    Free_Space->Partition_Table_Index = Partition_Table_Index;

    /* Is there a DLA Table entry for this partition? */
    if ( DLA_Table_Entry != NULL )
    {

      /* Use the provided DLA Table entry. */
      Free_Space->DLA_Table_Entry = *DLA_Table_Entry;

      /* Use the DLA Talbe entry to fill in the Partition_Name. */
      strncpy(Free_Space->Partition_Name,Free_Space->DLA_Table_Entry.Partition_Name, PARTITION_NAME_SIZE);

    }
    else
    {

      /* Since there was no DLA Table entry for this partition, 0 out the DLA_Table_Entry field. */
      memset( &(Free_Space->DLA_Table_Entry), 0, sizeof(DLA_Entry) );

    }

    /* If this partition is an EBR, then it should not have an external handle.  */
    if ( Partition_Type == MBR_EBR )
    {

      /* We must dispose of the external handle for this partition record. */
      Destroy_Handle(Free_Space->External_Handle,Error_Code);


#ifdef DEBUG

#ifdef PARANOID

      assert(*Error_Code == HANDLE_MANAGER_NO_ERROR);

#else

      if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

        return NULL;

      }

#endif

#endif

      Free_Space->External_Handle = NULL;

    }
    else
    {

      /* Save the external handle of the new partition. */
      New_Partition_Handle = Free_Space->External_Handle;

    }

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Allocate_Partition_From_Free_Space")

  return New_Partition_Handle;

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
#include "mbr_os2.h"
//static struct _MBR mbr;

static void Create_MBR( void )
{

  FUNCTION_ENTRY("Create_MBR")

  /* Copy the contents of UOBootSeg to MBR.  UOBootSeg contains the default MBR, which is built separately from the LVM Engine. */
//????? EK
  memcpy(&MBR, (void *)&mbr, BYTES_PER_SECTOR);

  FUNCTION_EXIT("Create_MBR")

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
static void _System Update_Table_Entries( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast all the time. */
  MBR_EBR_Build_Data  * Build_Data;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *      PartitionRecord = (Partition_Data *) Object;

  /* We need some local variables for translating LBA addresses into CHS addresses. */
  CARDINAL32            Sector;
  CARDINAL32            Head;
  CARDINAL32            Cylinder;
  CARDINAL32            Ending_LBA;

  /* Declare a local variable to allow us to access the DriveArray easily. */
  CARDINAL32            Index;

  FUNCTION_ENTRY("Update_Table_Entries")

  /* Assume that we will succeed. */
  *Error = DLIST_SUCCESS;

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Update_Table_Entries")

    return;

  }

  /* Establish access to our parameters. */
  Build_Data = (MBR_EBR_Build_Data *) Parameters;

  /* Get the DriveArray index to use. */
  Index = PartitionRecord->Drive_Index;

  /* What kind of Partition_Data do we have here? */
  switch ( PartitionRecord->Partition_Type )
  {

    case MBR_EBR : /* We have an MBR or EBR. */
                   /* Are we looking for the start of an extended partition? */
                   if ( Build_Data->Find_Start )
                   {

                     /* Is this the MBR? */
                     if ( PartitionRecord->Starting_Sector == 0 )
                     {

                       /* Do we need to create a new MBR?  If the New_Partition flag is on, then we do. */
                       if ( PartitionRecord->New_Partition )
                       {

                         Build_Data->New_MBR_Needed = TRUE;

                       }

                     }
                     else
                     {

                       /* Save the start of this EBR.  It will be used for calculating the sector offset field in the
                          Partition Table Entry of the partition associated with this EBR.                               */
                       Build_Data->EBR_Start = PartitionRecord->Starting_Sector;

                       /* Have we found the start of the extended partition yet?  If we have not, then this EBR is it! */
                       if ( Build_Data->Extended_Partition_Start == 0 )
                       {

                         /* We have the first EBR on the disk.  This will be the start of the extended partition.  Save its starting location. */
                         Build_Data->Extended_Partition_Start = PartitionRecord->Starting_Sector;

                       }

                     }

                   }
                   else
                   {

                     /* When Find_Start is FALSE, we are traversing the Partitions list backwards.  This means that the first
                        non-primary partition we find is the end of the extended partition, and the first EBR we find is the
                        last EBR in the EBR chain for the extended partition.                                                   */

                     /* Do we have an EBR?  If its location on the disk is not LBA 0, then it is an EBR! */
                     if ( PartitionRecord->Starting_Sector != 0 )
                     {

                       /* We have an EBR.  Since the Partition_Table_Entry field is unused for an EBR, we will use it to hold the
                          link entry for the following EBR ( if the current EBR is not the last one in the EBR chain).             */

                       /* Is there an EBR following this one? */
                       if ( Build_Data->EBR_Start != 0 )
                       {

                         /* Calculate the starting CHS values for the EBR we are linking to. */
                         Convert_To_CHS( Build_Data->EBR_Start, Index, &Cylinder, &Head, &Sector );

                         /* Convert it to the format used for a partition table entry. */
                         Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head,  &Sector );

                         /* Save the calculated address into the Partition_Table_Entry for this Partition_Data record. */
                         PartitionRecord->Partition_Table_Entry.Starting_Cylinder = Cylinder;
                         PartitionRecord->Partition_Table_Entry.Starting_Head = Head;
                         PartitionRecord->Partition_Table_Entry.Starting_Sector = Sector;

                         /* Convert Ending_LBA to CHS form. */
                         Convert_To_CHS( Build_Data->EBR_End, Index, &Cylinder, &Head, &Sector );

                         /* Convert it to the format used for a partition table entry. */
                         Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head,  &Sector );

                         /* Save the calculated address into the Partition_Table_Entry for this Partition_Data record. */
                         PartitionRecord->Partition_Table_Entry.Ending_Cylinder = Cylinder;
                         PartitionRecord->Partition_Table_Entry.Ending_Head = Head;
                         PartitionRecord->Partition_Table_Entry.Ending_Sector = Sector;

                         /* Now we can update the size and offset fields. */
                         PartitionRecord->Partition_Table_Entry.Sector_Count = Build_Data->EBR_Size;
                         PartitionRecord->Partition_Table_Entry.Sector_Offset = Build_Data->EBR_Start - Build_Data->Extended_Partition_Start;

                         /* Now set the Boot Indicator to 0 and the Format Indicator to 5 to indicate that this entry is a link to another EBR. */
                         PartitionRecord->Partition_Table_Entry.Boot_Indicator = EBR_BOOT_INDICATOR;
                         PartitionRecord->Partition_Table_Entry.Format_Indicator = EBR_FORMAT_INDICATOR;

                       }
                       else
                       {

                         /* Make sure that the Partition_Table_Entry fields are zeroed out so that we don't mistake it for a link entry. */
                         memset( &(PartitionRecord->Partition_Table_Entry),0,sizeof(Partition_Record));

                       }

                       /* Now we must update Build_Data with the values for this EBR. */
                       Build_Data->EBR_Start = PartitionRecord->Starting_Sector;

                       /* For purposes of calculating an EBR Link Entry, this EBR ends at the point where its associated non-primary (logical
                          drive in the old terminology) ends.  The associated non-primary partition always follows the EBR with no other
                          partitions allowed in between.                                                                                       */
                       Build_Data->EBR_End = Build_Data->Logical_Drive_Start + Build_Data->Logical_Drive_Size - 1;

                       /* The size of the EBR is the difference between its starting and ending values, + 1 */
                       Build_Data->EBR_Size = (Build_Data->EBR_End - Build_Data->EBR_Start) + 1;

                     }

                   }

                   break;

    case Partition : /* We have a partition. */

                     /* If Build_Data indicates we are searching for the start of an extended partition, then we will also
                        update the Partition_Table_Entry and DLA_Table_Entry for this Partition_Data record.                */

                     if ( Build_Data->Find_Start )
                     {

                       /* Calculate the Starting CHS address for this partition. */
                       Convert_To_CHS( PartitionRecord->Starting_Sector, Index, &Cylinder, &Head, &Sector );

                       /* Convert it to the format used for a partition table entry. */
                       Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head,  &Sector );

                       /* Save the calculated address into the Partition_Table_Entry for this Partition_Data. */
                       PartitionRecord->Partition_Table_Entry.Starting_Cylinder = Cylinder;
                       PartitionRecord->Partition_Table_Entry.Starting_Head = Head;
                       PartitionRecord->Partition_Table_Entry.Starting_Sector = Sector;


                       /* Now lets repeat this process for the ending CHS address for the partition. */

                       /* Find the Ending CHS address for this partition. */
                       Ending_LBA =  PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size - 1;

                       /* Calculate the Ending CHS address for this partition. */
                       Convert_To_CHS( Ending_LBA, Index, &Cylinder, &Head, &Sector );

                       /* Convert it to the format used for a partition table entry. */
                       Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head,  &Sector );

                       /* Save the calculated address into the Partition_Table_Entry for this Partition_Data. */
                       PartitionRecord->Partition_Table_Entry.Ending_Cylinder = Cylinder;
                       PartitionRecord->Partition_Table_Entry.Ending_Head = Head;
                       PartitionRecord->Partition_Table_Entry.Ending_Sector = Sector;

                       /* Now we can update the offset and size fields. */
                       PartitionRecord->Partition_Table_Entry.Sector_Count = PartitionRecord->Partition_Size;

                       /* The offset depends upon whether or not this is a primary partition. */
                       if ( PartitionRecord->Primary_Partition )
                       {

                         /* The offset for a primary partition is equal to the LBA of the starting location of the partition. */
                         PartitionRecord->Partition_Table_Entry.Sector_Offset = PartitionRecord->Starting_Sector;

                       }
                       else
                       {

                         /* The offset for a non-primary partition is calculated as the offset from the start of the EBR which defines it. */
                         PartitionRecord->Partition_Table_Entry.Sector_Offset = PartitionRecord->Starting_Sector - Build_Data->EBR_Start;

                       }

                       /* Now we need to update the DLA Table Entry for this partition. */
                       PartitionRecord->DLA_Table_Entry.Partition_Size = PartitionRecord->Partition_Size;
                       PartitionRecord->DLA_Table_Entry.Partition_Start = PartitionRecord->Starting_Sector;

                     }
                     else
                     {

                       /* If this partition is a non-primary partition, we must place its size and starting location in the Build_Data
                          structure.  We will need it when building the EBR link entries.                                               */
                       if ( PartitionRecord->Primary_Partition != TRUE )
                       {

                         /* Save the starting location and size of the current partition. */
                         Build_Data->Logical_Drive_Start = PartitionRecord->Starting_Sector;
                         Build_Data->Logical_Drive_Size = PartitionRecord->Partition_Size;

                         /* If this is the first non-primary partition we have encountered, then it must be the last one inside of
                            the extended partition on this disk.  That means that the ending address of this partition is the ending
                            address of the extended partition.                                                                        */

                         /* Have we already found the last non-primary partition on the disk? */
                         if ( Build_Data->Extended_Partition_End == 0 )
                         {

                           /* This is the first non-primary partition that we have encountered on this pass.  Calculate its ending LBA and
                              place that in the Build_Data as the value for Extended_Partition_End.                                         */
                           Build_Data->Extended_Partition_End = PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size - 1;

                         }

                       }

                     }
                     break;

    case FreeSpace : /* We don't care about free space here! */
                     break;
    default: /* Error!  All of the valid partition types have been enumerated above.  If we get here, something has been corrupted! */

#ifdef DEBUG

#ifdef PARANOID

             assert(0);

#else

             *Error = DLIST_CORRUPTED;

#endif

#endif

             break;   /* Keep the compiler happy. */
  }


  FUNCTION_EXIT("Update_Table_Entries")

  return;

}


/*********************************************************************/
/*   Function Name:                                                  */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*********************************************************************/
void Convert_To_CHS(LBA Starting_Sector, CARDINAL32 Index, CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector )
{

  FUNCTION_ENTRY("Convert_To_CHS")

  /* Calculate the Starting CHS address for this partition. */
  *Sector = ( Starting_Sector % (CARDINAL32) DriveArray[Index].Geometry.Sectors ) + 1;
  *Head = ( Starting_Sector / (CARDINAL32) DriveArray[Index].Geometry.Sectors ) % DriveArray[Index].Geometry.Heads;
  *Cylinder = Starting_Sector / (CARDINAL32) DriveArray[Index].Sectors_Per_Cylinder;

#ifdef DEBUG

#ifdef PARANOID

  assert( *Sector <= DriveArray[Index].Geometry.Sectors );
  assert( *Sector > 0 );
  assert( *Head < DriveArray[Index].Geometry.Heads );

#endif

#endif

  /* Make sure that Cylinder is within the allowable limits for a partition table entry. */
  if ( *Cylinder >= MAX_CYLINDERS )
  {

    *Cylinder = MAX_CYLINDERS - 1;

    /* Since we have more than 1024 cylinders here, we are in a special situation.  In this case,
       to indicate that we have gone beyond the 1024 cylinder limit, we must max out both Heads and
       Sectors also.                                                                                   */
    *Head = DriveArray[Index].Geometry.Heads - 1;
    *Sector = DriveArray[Index].Geometry.Sectors;

  }

  /* Make sure that Head is within the allowable limits for a partition table entry. */
  if ( *Head >= MAX_HEADS )
  {

    *Head = MAX_HEADS - 1;

  }

  /* Make sure that Sector is within the allowable limits for a partition table entry. */
//  if ( *Sector > DriveArray[Index].Geometry.Sectors)
//  {
//    *Sector = DriveArray[Index].Geometry.Sectors;
//  }
    
  if ( *Sector > MAX_SECTORS )
  {
    *Sector = MAX_SECTORS;

  }

#ifdef DEBUG

#ifdef PARANOID

  assert( *Sector <= MAX_SECTORS );
  assert( *Sector > 0 );
  assert( *Head < MAX_HEADS );
  assert( *Cylinder < MAX_CYLINDERS );

#endif

#endif

  FUNCTION_EXIT("Convert_To_CHS")

  return;
}


/*********************************************************************/
/*   Function Name:                                                  */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*********************************************************************/
void _System Convert_To_CHS_With_No_Checking(LBA Starting_Sector, CARDINAL32 Index, CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code )
{

  FUNCTION_ENTRY("Convert_To_CHS_With_No_Checking")

  /* Is Index legal? */
  if ( Index >= DriveCount )
  {

    *Error_Code = LVM_ENGINE_DRIVE_NOT_FOUND;
    *Sector = 0;
    *Head = 0;
    *Cylinder = 0;

    FUNCTION_EXIT("Convert_To_CHS_With_No_Checking")

    return;

  }

  /* Calculate the Starting CHS address for this partition. */
  *Sector = ( Starting_Sector % (CARDINAL32) DriveArray[Index].Geometry.Sectors ) + 1;
  *Head = ( Starting_Sector / (CARDINAL32) DriveArray[Index].Geometry.Sectors ) % DriveArray[Index].Geometry.Heads;
  *Cylinder = Starting_Sector / (CARDINAL32) DriveArray[Index].Sectors_Per_Cylinder;

#ifdef DEBUG

#ifdef PARANOID

  assert( *Sector <= DriveArray[Index].Geometry.Sectors );
  assert( *Sector > 0 );
  assert( *Head < DriveArray[Index].Geometry.Heads );

#endif

#endif

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Convert_To_CHS_With_No_Checking")

  return;

}


/*********************************************************************/
/*   Function Name:                                                  */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*********************************************************************/
void Convert_CHS_To_Partition_Table_Format( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector )
{

  /* In the partition table, each of the entries for cylinders, heads, and sectors is 1 byte.  Also, cylinders
     was expanded to 10 bits while sectors was limited to 6 bits (so that the top two bits of its entry could
     be used to hold the top two bits of cylinders).  We need to put our Cylinder and Sector values into this
     format.                                                                                                    */

  FUNCTION_ENTRY("Convert_CHS_To_Partition_Table_Format")

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"After Head %i  Cylinder %i Sector %i\n",*Head, *Cylinder,  *Sector);
      Write_Log_Buffer();

    }
  
  *Sector += ( *Cylinder / 0x100) << 6;
  *Cylinder = *Cylinder % 0x100;

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"After Head %i  Cylinder %i Sector %i\n",*Head, *Cylinder,  *Sector);
      Write_Log_Buffer();

    }
#ifdef DEBUG

#ifdef PARANOID

  assert( *Sector < 256 );
  assert( *Head < 256 );
  assert( *Cylinder < 256 );

#endif

#endif

  FUNCTION_EXIT("Convert_CHS_To_Partition_Table_Format")

  return;

}


/*********************************************************************/
/*   Function Name:                                                  */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*********************************************************************/
void _System Convert_CHS_To_Partition_Table_Format_With_Checking( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code )
{

  FUNCTION_ENTRY("Convert_CHS_To_Partition_Table_Format_With_Checking")

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"Cylinder %i Head %i Cylinder %\n",*Cylinder, *Head, *Cylinder);
      Write_Log_Buffer();

    }
  
  /* Are the initial values in range? */
  if ( ( *Cylinder > 1023 ) ||
       ( *Head > 254 ) ||
       ( *Sector > 63 )
     )
  {

    *Error_Code = LVM_ENGINE_REQUESTED_SIZE_TOO_BIG;

    FUNCTION_EXIT("Convert_CHS_To_Partition_Table_Format_With_Checking")

    return;

  }

  /* In the partition table, each of the entries for cylinders, heads, and sectors is 1 byte.  Also, cylinders
     was expanded to 10 bits while sectors was limited to 6 bits (so that the top two bits of its entry could
     be used to hold the top two bits of cylinders).  We need to put our Cylinder and Sector values into this
     format.                                                                                                    */

  *Sector += ( *Cylinder / 0x100) << 6;
  *Cylinder = *Cylinder % 0x100;

#ifdef DEBUG

#ifdef PARANOID

  assert( *Sector < 256 );
  assert( *Head < 255 );
  assert( *Cylinder < 256 );

#endif

#endif

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Convert_CHS_To_Partition_Table_Format_With_Checking")

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
static void _System Write_Changes( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a variable so that we can walk the partition table to check for certain errors. */
  CARDINAL32            Partition_Index;

  /* Declare a local variable to keep track of where we put things in the MBR. */
  CARDINAL32            MBR_Index;

  /* Declare a variable to keep track of whether or not an active primary partition has been found. */
  BOOLEAN               Active_Primary_Found = FALSE;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *      PartitionRecord = (Partition_Data *) Object;

  /* Establish access to the DLA_Table for EBRs. */
  DLA_Table_Sector *    DLA_Table = (DLA_Table_Sector *) &DLA_Sector;

  /* Establish access to the DLA Table for MBRs. */
  DLA_Table_Sector *    MBR_DLA_Table = (DLA_Table_Sector *) & MBR_DLA_Sector;

  /* Declare a local variable to allow us to access the DriveArray easily. */
  CARDINAL32            Index;

  /* Establish access to the Boot Manager Alias in the EBR. */
  AliasTableEntry *      BootManagerAlias = (AliasTableEntry *) ( (CARDINAL32) &EBR + ALIAS_TABLE_OFFSET);

  FUNCTION_ENTRY("Write_Changes")

  /* Assume that we will succeed. */
  *Error = DLIST_SUCCESS;

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Write_Changes")

    return;

  }

  /* Get the DriveArray index to use. */
  Index = PartitionRecord->Drive_Index;

  /* What kind of Partition_Data do we have here? */
  switch ( PartitionRecord->Partition_Type )
  {

    case MBR_EBR : /* We have an MBR or EBR. */

                   /* Is this the MBR or an EBR?  We don't write out the MBR here because it may not be complete yet. */
                   if ( PartitionRecord->Starting_Sector != 0 )
                   {

                     /* Since this MBR/EBR did not occupy LBA 0, it can not be an MBR. */

                     /* The EBR Partition Table should already have the non-primary partition (logical drive in the old terminology)
                        associated with this EBR declared in entry 0.  If this EBR has a link to another EBR, that link will already have
                        been calculated and stored in the Partition_Table_Entry field of this Partition_Data record.  We just need to
                        copy it to the EBR Partition Table entry 1 and then write the EBR to disk.                                         */
                     EBR.Partition_Table[1] = PartitionRecord->Partition_Table_Entry;

                     LOG_EVENT2("Writing EBR to disk.","Drive Number", Index + 1,"Starting Sector",PartitionRecord->Starting_Sector)

                     /* Now write the EBR to disk. */
                     WriteSectors(Index + 1, PartitionRecord->Starting_Sector, 1, &EBR, Error );

                     /* Did the write succeed? */
                     if ( *Error != DISKIO_NO_ERROR )
                     {

                       /* Mark the I/O error in the drive array. */
                       DriveArray[Index].IO_Error = TRUE;

                       LOG_EVENT1("WriteSectors failed!","Error code", *Error)

                     }

                     /* Now zero out the partition table of the EBR. */
                     memset( EBR.Partition_Table, 0, 4 * sizeof(Partition_Record) );

                     /* The DLA Table that corresponds to this EBR is in DLA_Sector.  Since an EBR Link Entry has no DLA Table entry,
                        we just need to calculate the CRC for this table and write this table to disk.  It is written to the last
                        sector of the track containing the EBR.                                                                         */

                     /* Calculate the CRC. */
                     DLA_Table->DLA_CRC = 0;

                     DLA_Table->DLA_CRC = CalculateCRC(INITIAL_CRC, &DLA_Sector, BYTES_PER_SECTOR);

                     LOG_EVENT2("Writing the DLA Table for the EBR to disk.","Drive Number", Index + 1,"Starting Sector",PartitionRecord->Starting_Sector + DriveArray[Index].Geometry.Sectors - 1 )

                     /* Write the DLA Table to disk. */
                     WriteSectors(Index + 1, PartitionRecord->Starting_Sector + DriveArray[Index].Geometry.Sectors - 1,1,&DLA_Sector, Error);

                     /* Did the write succeed? */
                     if ( *Error != DISKIO_NO_ERROR )
                     {

                       /* Mark the I/O error in the drive array. */
                       DriveArray[Index].IO_Error = TRUE;

                       LOG_EVENT1("WriteSectors failed!", "Error code", *Error)

                     }

                     /* Now zero out the table entries in the DLA table. */
                     memset( &(DLA_Table->DLA_Array), 0, 4 * sizeof( DLA_Entry ) );

                   }

                   break;

    case Partition : /* We have a partition. */

                     /* Is this a primary partition? */
                     if ( PartitionRecord->Primary_Partition )
                     {

                       /* Find out where to place the partition table information in the MBR. */

                       /* Does this partition have a valid Partition Table Index? */
                       if ( PartitionRecord->Partition_Table_Index < 4 )
                       {

                         /* It does!  Lets see if that index is available.  If not, then lets make it available. */
                         if ( !Partition_Table_Index_In_Use[PartitionRecord->Partition_Table_Index] )
                         {

                           /* This index is available!  Lets use it. */
                           MBR_Index = PartitionRecord->Partition_Table_Index;

                         }
                         else
                         {

                           /* There is something already occupying this position.  If this position is claimed, then we have
                              a problem.  Claimed means that another partition on this drive has a valid Partition Table Index
                              for this position.  This should never happen because Partition Table Indexes are assigned as the
                              partition table entries are read from the partition table, so there can never be duplicates.
                              New Partitions are assigned Partition Table Indexes which are outside of the valid range.  Thus,
                              if this position in the partition table is already claimed, we must have an internal error of
                              some sort.                                                                                         */
                           if ( Partition_Table_Entry_Claimed[PartitionRecord->Partition_Table_Index] )
                           {

                             LOG_ERROR("Failure creating a partition table!  Multiple items claim the same position within the table!")

                             *Error = DLIST_CORRUPTED;

                             FUNCTION_EXIT("Write_Changes")

                             return;

                           }

                           /* Since this position in the partition table is not claimed, we can move the data in it to an
                              unused position within the partition table.                                                   */
                           for ( MBR_Index = 0; MBR_Index < 4; MBR_Index++ )
                           {

                             /* Is the current position unused? */
                             if ( !Partition_Table_Index_In_Use[MBR_Index] )
                             {

                               /* We can move the data to this position within the partition table. */
                               MBR.Partition_Table[MBR_Index] = MBR.Partition_Table[PartitionRecord->Partition_Table_Index];

                               /* We must move the DLA Table Entry that corresponds to this partition table entry. */
                               MBR_DLA_Table->DLA_Array[MBR_Index] = MBR_DLA_Table->DLA_Array[PartitionRecord->Partition_Table_Index];

                               /* Mark the current position as being in use. */
                               Partition_Table_Index_In_Use[MBR_Index] = TRUE;

                               /* Now free the old position so that we can use it. */
                               Partition_Table_Index_In_Use[PartitionRecord->Partition_Table_Index] = FALSE;

                               /* Break out of the for loop as we have found the entry we want. */
                               break;

                             }

                           }

                           /* If we get here and MBR_Index is too big or the desired entry in the partition table is still in use,
                              then we have an internal error because we are trying to put too many entries into the partition table.  */
                           if ( ( MBR_Index >= 4 ) || Partition_Table_Index_In_Use[PartitionRecord->Partition_Table_Index] )
                           {

                             LOG_ERROR("Can not create partition table!  There are too many items claiming to be in this partition table!")

                             *Error = DLIST_CORRUPTED;

                             FUNCTION_EXIT("Write_Changes")

                             return;

                           }

                         }

                         /* Set MBR_Index to location at which we are going to place the current partition table entry. */
                         MBR_Index = PartitionRecord->Partition_Table_Index;

                         /* Mark this position in the partition table as being in use. */
                         Partition_Table_Index_In_Use[MBR_Index] = TRUE;

                         /* Since this partition has a partition table index, mark this position in the partition table as being claimed. */
                         Partition_Table_Entry_Claimed[MBR_Index] = TRUE;

                       }
                       else
                       {

                         /* Since this partition did not have a valid partition table index, we will put it in the first available position. */

                         /* Find the first available entry in the partition table. */
                         for ( MBR_Index = 0; MBR_Index < 4; MBR_Index++ )
                         {

                           /* Is the current position unused? */
                           if ( !Partition_Table_Index_In_Use[MBR_Index] )
                           {

                             /* Mark the current position as being in use. */
                             Partition_Table_Index_In_Use[MBR_Index] = TRUE;

                             /* Exit the for loop as we now have the position to place our partition table entry in. */
                             break;

                           }

                         }

                         /* If MBR_Index is out of range, then we have an internal error as we are trying to place too many items
                            into the partition table in the MBR!                                                                   */
                         if ( MBR_Index >= 4 )
                         {

                           LOG_ERROR("Can not create partition table!  There are too many items claiming to be in this partition table!")

                           *Error = DLIST_CORRUPTED;

                           FUNCTION_EXIT("Write_Changes")

                           return;
                         }

                       }

                       /* We must place the partition table information for this partition into the MBR. */
                       MBR.Partition_Table[MBR_Index] = PartitionRecord->Partition_Table_Entry;

                       /* Place the DLA Table information for this partition into the MBR_DLA_Table. */
                       MBR_DLA_Table->DLA_Array[MBR_Index] = PartitionRecord->DLA_Table_Entry;

                       /* If the drive is marked as having "fake" volumes in use, then remove any volume information from the DLA Table in the MBR_DLA_Table. */
                       if ( DriveArray[PartitionRecord->Drive_Index].Fake_Volumes_In_Use )
                       {

                         LOG_EVENT1("Removing fake volume from drive.","Drive Number",PartitionRecord->Drive_Index)

                         /* We don't want to write out the "fake" volume information.  Remove the volume information from the DLA Table entry being
                            created for this partition in the MBR_DLA_Table.                                                                          */
                         MBR_DLA_Table->DLA_Array[MBR_Index].Volume_Serial_Number = 0;
                         MBR_DLA_Table->DLA_Array[MBR_Index].On_Boot_Manager_Menu = FALSE;
                         MBR_DLA_Table->DLA_Array[MBR_Index].Installable = FALSE;
                         MBR_DLA_Table->DLA_Array[MBR_Index].Drive_Letter = '*';
                         memset(MBR_DLA_Table->DLA_Array[MBR_Index].Volume_Name,0,VOLUME_NAME_SIZE);

                       }

                       /* Check to make sure that only one entry in the partition table is marked active!  If more than one are
                          marked active, then only the first one will get to keep its active flag.                               */
                       for ( Partition_Index = 0; Partition_Index < 4; Partition_Index++ )
                       {

                         /* Is the current entry marked active? */
                         if ( (MBR.Partition_Table[Partition_Index].Boot_Indicator & ACTIVE_PARTITION) != 0 )
                         {

                           /* Have we already seen an entry marked active? */
                           if ( Active_Primary_Found )
                           {

                             /* We already have an active primary partition!  Turn off the active flag for this one. */
                             MBR.Partition_Table[Partition_Index].Boot_Indicator &= (~ACTIVE_PARTITION);

                           }
                           else
                           {
                             /* Remember that we have found an active primary partition. */
                             Active_Primary_Found = TRUE;

                             /* Now, lets check the format indicator.  The format indicator must agree with the active flag in
                                that the format indicator can not be one of Boot Manager's "hidden" partition codes.             */
                             switch ( MBR.Partition_Table[Partition_Index].Format_Indicator )
                             {
                               case 0x11 : /* This is a Boot Manager hidden FAT12 partition.  Unhide it. */
                                           MBR.Partition_Table[Partition_Index].Format_Indicator = 0x01;
                                           break;

                               case 0x14 : /* This is a Boot Manager hidden FAT16 (< 32MB) partition.  Unhide it. */
                                           MBR.Partition_Table[Partition_Index].Format_Indicator = 0x04;
                                           break;

                               case 0x16 : /* This is a Boot Manager hidden FAT16 (> 32MB) partition.  Unhide it. */
                                           MBR.Partition_Table[Partition_Index].Format_Indicator = 0x06;
                                           break;

                               case 0x17 : /* This is a boot manager hidden IFS partition.  Unhide it. */
                                           MBR.Partition_Table[Partition_Index].Format_Indicator = 0x07;
                                           break;

                               default: /* It is not a Boot Manager hidden partition.  Ignore it. */
                                        break;

                             }

                           }

                         }

                       }

                     }
                     else
                     {

                       /* We must place the partition table information for this partition into the EBR.  It is always placed in entry 0 of the EBR Partition Table. */
                       EBR.Partition_Table[0] = PartitionRecord->Partition_Table_Entry;

                       /* If this partition is on the Boot Manager Menu, we will put "--> LVM" as the Boot Manager Alias so that FDISK and
                          other programs which look for this data in the old format will find something to display.                          */
                       if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
                       {

                         BootManagerAlias->On_Boot_Manager_Menu = TRUE;

                         /* If this partition is part of the installable volume, then we will use a slightly different name for it in the Boot Manager Alias Table. */
                         if ( PartitionRecord->DLA_Table_Entry.Installable )
                           strncpy(BootManagerAlias->Name, ALIAS_TABLE_ENTRY_MIGRATION_TEXT2, ALIAS_NAME_SIZE);
                         else
                           strncpy(BootManagerAlias->Name, ALIAS_TABLE_ENTRY_MIGRATION_TEXT, ALIAS_NAME_SIZE);

                       }
                       else
                       {

                         BootManagerAlias->On_Boot_Manager_Menu = FALSE;
                         memset(BootManagerAlias->Name,0,ALIAS_NAME_SIZE);

                       }

                       /* Place the DLA table information into the corresponding location in the DLA table. */
                       DLA_Table->DLA_Array[0] = PartitionRecord->DLA_Table_Entry;

                       /* If the drive is marked as having "fake" volumes in use, then remove any volume information from the DLA Table entry in the DLA_Table. */
                       if ( DriveArray[PartitionRecord->Drive_Index].Fake_Volumes_In_Use )
                       {

                         LOG_EVENT1("Removing fake volume from drive.","Drive Number",PartitionRecord->Drive_Index)

                         /* We don't want to write out the "fake" volume information.  Remove the volume information from the DLA Table entry being
                            created for this partition in the DLA_Table.                                                                          */
                         DLA_Table->DLA_Array[0].Volume_Serial_Number = 0;
                         DLA_Table->DLA_Array[0].On_Boot_Manager_Menu = FALSE;
                         DLA_Table->DLA_Array[0].Installable = FALSE;
                         DLA_Table->DLA_Array[0].Drive_Letter = '*';
                         memset(DLA_Table->DLA_Array[0].Volume_Name,0,VOLUME_NAME_SIZE);

                       }

                     }

                     break;

    case FreeSpace : /* We don't care about free space here! */
                     break;
    default: /* Error!  All of the valid partition types have been enumerated above.  If we get here, something has been corrupted! */

#ifdef DEBUG

#ifdef PARANOID

             assert(0);

#else

             *Error = DLIST_CORRUPTED;

#endif

#endif

             LOG_ERROR("Partition with invalid partition type encountered!")

             break;  /* Keep the compiler happy. */
  }

  /* Indicate success. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Write_Changes")

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
static void Add_Extended_Partition_To_MBR(LBA Extended_Partition_Start, LBA Extended_Partition_End, CARDINAL32 Drive_Array_Index, CARDINAL32 * Error_Code)
{

  /* We need a local variable to determine which partition table entry to use for the extended partition. */
  CARDINAL32  Partition_Table_Index;

  /* We need some local variables for translating LBA addresses into CHS addresses.  */
  CARDINAL32  Sector;
  CARDINAL32  Head;
  CARDINAL32  Cylinder;

  FUNCTION_ENTRY("Add_Extended_Partition_To_MBR")

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Find an unused entry in the partition table to use for the extended partition. */
  for ( Partition_Table_Index = 0; Partition_Table_Index < 4; Partition_Table_Index++ )
  {

    if ( !Partition_Table_Index_In_Use[Partition_Table_Index] )
      break;

  }

#ifdef DEBUG

#ifdef PARANOID

  assert( Partition_Table_Index < 4 );

#else

  if ( Partition_Table_Index >= 4 )
  {

    LOG_ERROR("There are no available entries in the MBR partition table to use for the extended partition!")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Add_Extended_Partition_To_MBR")

    return;

  }

#endif

#endif

  /* Calculate the Starting CHS address for this partition. */
  Convert_To_CHS( Extended_Partition_Start, Drive_Array_Index, &Cylinder, &Head, &Sector );

  /* Convert it to the format used for a partition table entry. */
  Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head,  &Sector );

  /* Save the calculated address into the Partition_Table_Entry for this Partition_Data. */
  MBR.Partition_Table[Partition_Table_Index].Starting_Cylinder = Cylinder;
  MBR.Partition_Table[Partition_Table_Index].Starting_Head = Head;
  MBR.Partition_Table[Partition_Table_Index].Starting_Sector = Sector;

  /* Now lets repeat this process for the ending CHS address for the partition. */

  /* Calculate the Ending CHS address for this partition. */
  Convert_To_CHS( Extended_Partition_End, Drive_Array_Index, &Cylinder, &Head, &Sector );

  /* Convert it to the format used for a partition table entry. */
  Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head,  &Sector );

  /* Save the calculated address into the Partition_Table_Entry for this Partition_Data. */
  MBR.Partition_Table[Partition_Table_Index].Ending_Cylinder = Cylinder;
  MBR.Partition_Table[Partition_Table_Index].Ending_Head = Head;
  MBR.Partition_Table[Partition_Table_Index].Ending_Sector = Sector;

  /* Now we can update the offset and size fields. */
  MBR.Partition_Table[Partition_Table_Index].Sector_Count = (Extended_Partition_End - Extended_Partition_Start) + 1;

  MBR.Partition_Table[Partition_Table_Index].Sector_Offset = Extended_Partition_Start;

  /* Now set the Boot Indicator to 0 and the Format Indicator to 5 to indicate that this entry is for an extended partition. */
  MBR.Partition_Table[Partition_Table_Index].Boot_Indicator = EBR_BOOT_INDICATOR;
  MBR.Partition_Table[Partition_Table_Index].Format_Indicator = EBR_FORMAT_INDICATOR;


  FUNCTION_EXIT("Add_Extended_Partition_To_MBR")

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
BOOLEAN Can_Be_Primary( Partition_Data * Free_Space, CARDINAL32 * Error_Code )
{

  Partition_Data *  PreviousRecord;
  Partition_Data *  NextRecord;
  CARDINAL32        Index = Free_Space->Drive_Index;
  CARDINAL32        Current_Test;

  FUNCTION_ENTRY("Can_Be_Primary")

  /* Make Free_Space the current item in the Partitions list. */
  GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

  /* If there was an error returned by GoToSpecifiedItem, it will be caught below. */

  /* We can only create a primary partition if there is an open partition table entry in the MBR AND the
     block of free space chosen does not lie in an extended partition.

     At this point, a block of free space does NOT lie in the middle of an extended partition if:

        It is the last item in the Partitions list
        If the partition record immediately before it in the Partitions list describes a primary partition or the MBR
        If the partition record immediately after it in the Partitions list describes a primary partition

  */

  /* Set up for the tests required. */
  Current_Test = 1;   /* The first test. */

  /* Start testing. */
  while ( ( Current_Test != 0 ) && ( *Error_Code == DLIST_SUCCESS ) )
  {

    switch ( Current_Test )
    {
      case 1 : /* Are there enough open entries in the MBR to create a primary partition? */
               if ( (DriveArray[Index].Primary_Partition_Count >= 4 ) ||
                    ( ( DriveArray[Index].Primary_Partition_Count == 3 ) &&
                      ( DriveArray[Index].Logical_Partition_Count > 0  )
                    )
                  )
               {

                 /* We can not make a primary partition as there are no entries in the MBR available. */
                 *Error_Code = LVM_ENGINE_TOO_MANY_PRIMARY_PARTITIONS;

                 FUNCTION_EXIT("Can_Be_Primary")

                 return FALSE;

               }

               /* This test was passed.  Move on to the next one. */
               Current_Test++;

               break;
      case 2 : /* Is Free_Space the last item in the Partitions list? */
               if ( AtEndOfList( DriveArray[Index].Partitions, Error_Code) )
               {

                 /* Since this is the last entry in the Partitions list, creating a primary partition is possible! */

                 /* Set Current_Test to 0 so that we will exit the do-while loop. */
                 Current_Test = 0;

               }
               else
               {

                 /* Either Free_Space is not the last item in the Partitions list, or there was an error. */
                 Current_Test++;

               }

               break;
      case 3 : /* Is the item immediately before Free_Space in the Partitions list describing a primary partition or MBR? */

               /* Free_Space is the current item in the Partitions list.  Lets get the item immediately before it. */
               PreviousRecord = ( Partition_Data * ) GetPreviousObject( DriveArray[Index].Partitions, sizeof( Partition_Data ), PARTITION_DATA_TAG, Error_Code );
               if ( ( *Error_Code == DLIST_SUCCESS ) &&
                    ( ( ( PreviousRecord->Partition_Type == Partition) &&
                        PreviousRecord->Primary_Partition
                      ) ||
                      ( ( PreviousRecord->Partition_Type == MBR_EBR ) &&
                        ( PreviousRecord->Starting_Sector == 0 )
                      )
                    )
                  )
               {

                 /* Since the entry before Free_Space is either an MBR or a primary partition, we can
                    create a primary partition.                                                             */

                 /* Set Current_Test to 0 so that we will exit the do-while loop. */
                 Current_Test = 0;

               }
               else
               {

                 /* Either there was an error, or the previous item in the Partitions list is not a partition or MBR. */
                 Current_Test++;

               }

               break;
      case 4 : /* Is the item immediately after Free_Space in the Partitions list describing a primary partition? */

               /* Test 3 changed which item was the current item in the list.  Lets set it back to Free_Space. */
               GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

               /* If no errors, continue the test. */
               if ( *Error_Code == DLIST_SUCCESS )
               {

                 /* Free_Space is the current item in the Partitions list.  Lets get the item immediately after it. */
                 NextRecord = ( Partition_Data * ) GetNextObject( DriveArray[Index].Partitions, sizeof( Partition_Data ), PARTITION_DATA_TAG, Error_Code );
                 if ( ( *Error_Code == DLIST_SUCCESS ) &&
                      ( ( NextRecord->Partition_Type == Partition) &&
                        NextRecord->Primary_Partition
                      )
                    )
                 {

                   /* Since the entry after Free_Space is primary partition, we can
                      create a primary partition.                                                             */

                   /* Set Current_Test to 0 so that we will exit the do-while loop. */
                   Current_Test = 0;

                   break;

                 }

               }

               /* Either there was an error, or the next item in the Partitions list is not a primary partition. */
               Current_Test++;

               break;
      case 5 : /* If you get here, then you can not create a primary partition! */

               /* Make Free_Space the current item in the Partitions list. */
               GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

               assert( *Error_Code == DLIST_SUCCESS );

#else

               /* Were there any errors? */
               if ( *Error_Code != DLIST_SUCCESS )
               {

                 /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
                 *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                 FUNCTION_EXIT("Can_Be_Primary")

                 return FALSE;

               }

#endif

#endif

               /* Now indicate that we can not make a primary partition here. */
               *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_PRIMARY_PARTITION;

               FUNCTION_EXIT("Can_Be_Primary")

               return FALSE;

      default: /* Internal error!  Should never get here! */
               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

               FUNCTION_EXIT("Can_Be_Primary")

               return FALSE;
    }


  }

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Were there any errors? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Can_Be_Primary")

    return FALSE;

  }

#endif

#endif


  /* Make Free_Space the current item in the Partitions list. */
  GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Were there any errors? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Can_Be_Primary")

    return FALSE;

  }

#endif

#endif


  /* If we get here, then we can make a primary partition. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Can_Be_Primary")

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
BOOLEAN Can_Be_Non_Primary( Partition_Data * Free_Space, CARDINAL32 * Error_Code )
{

  Partition_Data *  PreviousRecord;
  Partition_Data *  NextRecord;
  CARDINAL32        Index = Free_Space->Drive_Index;
  CARDINAL32        Current_Test;

  FUNCTION_ENTRY("Can_Be_Non_Primary")

  /* Make Free_Space the current item in the Partitions list. */
  GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

  /* In order to make a partition which is not a primary partition, there either must be an extended
     partition or the ability to create one, and the block of free space chosen must lie within the
     extended partition.

     To tell if a block of free space lies within an extended partition, we must check the items in
     the Partitions list which lie immediately before and after it.  If one of them is a
     non-primary partition, then we can create a non-primary partition.  If they are both primary
     partitions, then we must see if we can create an extended partition.  If not, then we can not
     create the non-primary partition.                                                               */

  /* Set up for the tests required. */
  Current_Test = 1;   /* The first test. */

  /* Start testing. */
  while ( ( Current_Test != 0 ) && ( *Error_Code == DLIST_SUCCESS ) )
  {

    switch ( Current_Test )
    {
      case 1 : /* Can we create an extended partition? */
               if ( DriveArray[Index].Logical_Partition_Count == 0 )  /* Does an extended partition already exist? */
               {

                 /* There is no existing extended partition.  Can we create one? */
                 if ( DriveArray[Index].Primary_Partition_Count < 4 )
                 {
                   /* We can make an extended partition!  It does not matter where on the disk the block of free space lies. */
                   Current_Test = 0;

                 }
                 else
                 {

                   /* There is no existing extended partition, and we can not create one!  Abort. */
                   *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE;

                   FUNCTION_EXIT("Can_Be_Non_Primary")

                   return FALSE;

                 }

               }
               else
               {

                 /* There is an existing extended partition.  We must continue testing to see if the block of free space
                    lies within the extended partition.                                                                   */
                 Current_Test++;

               }

               break;
      case 2 : /* Does the item in the Partitions list immediately before Free_Space represent an MBR or primary partition? */

               /* Free_Space is the current item in the Partitions list.  Lets get the item immediately before it. */
               PreviousRecord = ( Partition_Data * ) GetPreviousObject( DriveArray[Index].Partitions, sizeof( Partition_Data ), PARTITION_DATA_TAG, Error_Code );
               if ( ( *Error_Code == DLIST_SUCCESS ) &&
                    ( ( ( PreviousRecord->Partition_Type == Partition) &&
                        !PreviousRecord->Primary_Partition
                      ) ||
                      ( ( PreviousRecord->Partition_Type == MBR_EBR ) &&
                        ( PreviousRecord->Starting_Sector != 0 )
                      )
                    )
                  )
               {

                 /* Since the entry before Free_Space is neither an MBR nor a primary partition, we can
                    create a non-primary partition.                                                             */

                 /* Set Current_Test to 0 so that we will exit the do-while loop. */
                 Current_Test = 0;

               }
               else
               {

                 /* Either there was an error, or the previous item in the Partitions list is either a primary partition or an MBR. */
                 Current_Test++;

               }

               break;
      case 3 : /* Is Free_Space the last item in the Partitions list?  If it is, since the item before it was a primary or MBR,
                  and since we can not create an extended partition, we can't use this free space to create a non-primary partition. */

               /* Test 2 changed which item was the current item in the list.  Lets set it back to Free_Space. */
               GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

               /* If no errors, continue the test. */
               if ( *Error_Code == DLIST_SUCCESS )
               {

                 /* Test to see if the current partition record is the last one in the Partitions list for the drive. */
                 if ( AtEndOfList( DriveArray[Index].Partitions, Error_Code) )
                 {

                   /* There is an existing extended partition, there is a primary partition immediately before this
                      block of free space, and this block of free space is at the end of the disk.  This means that we
                      can NOT create a logical drive from this block of free space as this block of free space does not
                      lie within the existing extended partition and is not adjacent to it.  Abort.                      */
                   *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE;

                   FUNCTION_EXIT("Can_Be_Non_Primary")

                   return FALSE;

                 }
                 else
                 {

                   /* Either Free_Space is not the last item in the Partitions list, or there was an error. */
                   Current_Test++;

                 }

               }

               break;

      case 4 : /* Does the item following Free_Space describe a primary partition? */

               /* Free_Space is the current item in the Partitions list.  Lets get the item immediately after it. */
               NextRecord = ( Partition_Data * ) GetNextObject( DriveArray[Index].Partitions, sizeof( Partition_Data ), PARTITION_DATA_TAG, Error_Code );
               if ( ( *Error_Code == DLIST_SUCCESS ) &&
                    ( ( NextRecord->Partition_Type == Partition) &&
                      !NextRecord->Primary_Partition
                    ) ||
                    ( NextRecord->Partition_Type == MBR_EBR )
                  )
               {

                 /* Since the entry after Free_Space is NOT a primary partition, we can
                    create a non-primary partition.                                                             */

                 /* Set Current_Test to 0 so that we will exit the do-while loop. */
                 Current_Test = 0;

                 break;

               }

               /* Either there was an error, or the next item in the Partitions list is a primary partition. */
               Current_Test++;

               break;
      case 5 : /* If you get here, then you can not create the partition! */

               /* Make Free_Space the current item in the Partitions list. */
               GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

               assert( *Error_Code == DLIST_SUCCESS );

#else

               /* Were there any errors? */
               if ( *Error_Code != DLIST_SUCCESS )
               {

                 /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
                 *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                 FUNCTION_EXIT("Can_Be_Non_Primary")

                 return FALSE;

               }

#endif

#endif

               /* Now indicate that we can not make a non-primary partition from this block of free space. */
               *Error_Code = LVM_ENGINE_CAN_NOT_MAKE_LOGICAL_DRIVE;

               FUNCTION_EXIT("Can_Be_Non_Primary")

               return FALSE;

      default: /* Internal error!  Should never get here! */
               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

               FUNCTION_EXIT("Can_Be_Non_Primary")

               return FALSE;
    }

  }

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Were there any errors? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Can_Be_Non_Primary")

    return FALSE;

  }

#endif

#endif

  /* Make Free_Space the current item in the Partitions list. */
  GoToSpecifiedItem(DriveArray[Index].Partitions, Free_Space->Drive_Partition_Handle, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Were there any errors? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* There was an error during a DLIST manipulation function.  This should not happen!  Abort. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Can_Be_Non_Primary")

    return FALSE;

  }

#endif

#endif


  /* If we get here, then we can create the partition. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Can_Be_Non_Primary")

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
static void _System Find_Active_Primary( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast all the time. */
  Primary_Partition_Status *  Status_Record = (Primary_Partition_Status *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *      PartitionRecord = (Partition_Data *) Object;

  FUNCTION_ENTRY("Find_Active_Primary")

  /* Assume that we will succeed. */
  *Error = DLIST_SUCCESS;

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Find_Active_Primary")

    return;

  }

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Do we have a partition record for a primary partition? */
  if ( ( PartitionRecord->Partition_Type == Partition ) &&
       ( PartitionRecord->Primary_Partition )
     )
  {

    /* Is the partition active? */
    if ( (PartitionRecord->Partition_Table_Entry.Boot_Indicator & PARTITION_ACTIVE_FLAG) != 0 )
    {

      /* We found an active partition. */

      /* Is this the Boot Manager Partition? */
      if ( Boot_Manager_Active && (PartitionRecord->External_Handle == Boot_Manager_Handle) )
      {

        /* We found Boot Manager. */
        Status_Record->Active_Boot_Manager_Found = TRUE;

      }
      else
        Status_Record->Active_Primary_Found = TRUE;

    }
    else
    {

      /* Is this a hidden primary? */
      if ( ( PartitionRecord->Partition_Table_Entry.Format_Indicator != ( BOOT_MANAGER_HIDDEN_PARTITION_FLAG & FAT12_INDICATOR ) ) &&
           ( PartitionRecord->Partition_Table_Entry.Format_Indicator != ( BOOT_MANAGER_HIDDEN_PARTITION_FLAG & FAT16_SMALL_PARTITION_INDICATOR ) ) &&
           ( PartitionRecord->Partition_Table_Entry.Format_Indicator != ( BOOT_MANAGER_HIDDEN_PARTITION_FLAG & FAT16_LARGE_PARTITION_INDICATOR ) ) &&
           ( PartitionRecord->Partition_Table_Entry.Format_Indicator != ( BOOT_MANAGER_HIDDEN_PARTITION_FLAG & IFS_INDICATOR ) )
         )
      {

        /* We have found a non-hidden, non-active primary. */
        Status_Record->Non_Hidden_Primary_Found = TRUE;

      }

    }

    /* Indicate that we found what we were looking for and return. */
    *Error = DLIST_SEARCH_COMPLETE;

  }

  FUNCTION_EXIT("Find_Active_Primary")

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
void _System Update_Partitions_Volume_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access our Parameters without having to typecase each time. */
  Partition_Data_To_Update  *  New_Values = ( Partition_Data_To_Update * ) Parameters;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;

  FUNCTION_ENTRY("Update_Partitions_Volume_Data")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Update_Partitions_Volume_Data")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* We have a partition.  Lets update it. */

    /* Do we need to update the drive letter? */
    if ( New_Values->Update_Drive_Letter )
    {

      /* Update the drive letter in the DLA Table. */
      PartitionRecord->DLA_Table_Entry.Drive_Letter = New_Values->New_Drive_Letter;

      /* Is this partition part of an LVM Volume? */
      if ( PartitionRecord->Signature_Sector != NULL )
      {

        /* Since this partition is part of an LVM Volume, we must update the LVM Signature Sector as well. */
        PartitionRecord->Signature_Sector->Drive_Letter = New_Values->New_Drive_Letter;

      }

    }

    /* Do we need to update the Volume Name? */
    if ( New_Values->Update_Volume_Name )
    {

      /* Update the Volume Name in the DLA Table. */
      strncpy(PartitionRecord->DLA_Table_Entry.Volume_Name, New_Values->New_Name, VOLUME_NAME_SIZE);

      /* Is this partition part of an LVM Volume? */
      if ( PartitionRecord->Signature_Sector != NULL )
      {

        /* Since this partition is part of an LVM Volume, we must update the LVM Signature Sector as well. */
        strncpy(PartitionRecord->Signature_Sector->Volume_Name, New_Values->New_Name, VOLUME_NAME_SIZE);

      }

    }

    /* Do we need to update the Boot Manager Status? */
    if ( New_Values->Update_Boot_Manager_Status )
    {

      /* Update the On_Boot_Manager_Menu field in the DLA Table. */
      PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = New_Values->On_Menu;

      /* If we are removing this partition/volume from the Boot Manager Menu and the partition's Installable flag is on,
         if Boot Manager is installed and Active, we must turn off the Installable flag.                                  */
      if ( ( ! New_Values->On_Menu ) &&
           PartitionRecord->DLA_Table_Entry.Installable &&
           ( Boot_Manager_Handle != NULL ) &&
           Boot_Manager_Active )
      {

        /* Turn off the Installable flag. */
        PartitionRecord->DLA_Table_Entry.Installable = FALSE;

      }

    }

    /* Do we need to update the spanned volume field? */
    if ( New_Values->Update_Volume_Spanning )
    {

      PartitionRecord->Spanned_Volume = New_Values->Spanned_Volume;

    }

    /* Does this Partition Record represent an aggregate? */
    if ( PartitionRecord->Drive_Index >= DriveCount )
    {

      /* If this is an aggregate, then the Drive_Index will be 0xFFFFFFFF. */
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

        /* We will call ForEachItem to process the PartitionRecord in this Partitions List also. */
        ForEachItem( Current_Context->Partitions, &Update_Partitions_Volume_Data, New_Values, TRUE, Error);

      }

    }
    else
    {

      /* We must set the ChangesMade flag on the drive that corresponds to this partition record.   */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

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

      FUNCTION_EXIT("Update_Partitions_Volume_Data")

      return;

  }

#endif

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Update_Partitions_Volume_Data")

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
void _System Find_Partition(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  Partition_Search_Record * Search_Record = ( Partition_Search_Record * ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *  PartitionRecord = (Partition_Data *) Object;


  FUNCTION_ENTRY("Find_Partition")

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(0);

#endif

#endif

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FUNCTION_EXIT("Find_Partition")

    return;

}

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Does the serial number of this partition match the one we are looking for? */
    if ( PartitionRecord->DLA_Table_Entry.Partition_Serial_Number == Search_Record->Serial_Number )
    {

      Search_Record->Handle = PartitionRecord->External_Handle;

      /* Indicate that we found what we were looking for. */
      *Error = DLIST_SEARCH_COMPLETE;

    }

  }

  FUNCTION_EXIT("Find_Partition")

  return;

}


/*********************************************************************/
/*   Function Name:                                                  */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  None.                                                   */
/*********************************************************************/
#if 0
void _System Convert_Partition_Table_Format_To_CHS_With_Checking( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code )
{

  FUNCTION_ENTRY("Convert_Partition_Table_Format_To_CHS_With_Checking")

  /* Are the initial values in range? */
  if ( ( *Cylinder > 255 ) ||
       ( *Head > 254 ) ||
       ( *Sector > 255 )
     )
  {

    *Error_Code = LVM_ENGINE_INVALID_PARAMETER;

    FUNCTION_EXIT("Convert_Partition_Table_Format_To_CHS_With_Checking")

    return;

  }

  /* In the partition table, each of the entries for cylinders, heads, and sectors is 1 byte.  Also, cylinders
     was expanded to 10 bits while sectors was limited to 6 bits (so that the top two bits of its entry could
     be used to hold the top two bits of cylinders).  We need to convert our Cylinder and Sector values from
     this format.                                                                                               */

//  *Sector = *Sector % 64;  /* Extract the low order 6 bits. */
//  *Cylinder = *Cylinder + ( ( *Sector / 64 ) * 256); /* Add the top two bits from *Sector as the top two bits of *Cylinder. */
  *Sector = *Sector;  /* Extract the low order 6 bits. */
  *Cylinder = *Cylinder + ( ( *Sector / 256 ) * 256); /* Add the top two bits from *Sector as the top two bits of *Cylinder. */

#ifdef DEBUG

#ifdef PARANOID

//assert( *Sector < 64 );
  assert( *Sector < 256 );
  assert( *Head < 255 );
  assert( *Cylinder < 1024 );

#endif

#endif

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Convert_Partition_Table_Format_To_CHS_With_Checking")

  return;

}
#endif //0

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
void _System Convert_CHS_To_LBA(CARDINAL32 Index, CARDINAL32 Cylinder, CARDINAL32 Head, CARDINAL32 Sector, LBA * Starting_Sector, CARDINAL32 * Error_Code)
{

  FUNCTION_ENTRY("Convert_CHS_To_LBA")

  /* Assume failure. */
  *Starting_Sector = 0;

  /* Is Index legal? */
  if ( Index >= DriveCount )
  {

    *Error_Code = LVM_ENGINE_DRIVE_NOT_FOUND;

    FUNCTION_EXIT("Convert_CHS_To_LBA")

    return;

  }

  /* Are Cylinder, Head, and Sector valid? */
  if ( ( Cylinder > DriveArray[Index].Geometry.Cylinders ) ||
       ( Head > DriveArray[Index].Geometry.Heads ) ||
       ( Sector > DriveArray[Index].Geometry.Sectors )
     )
  {

    *Error_Code = LVM_ENGINE_INVALID_PARAMETER;

    FUNCTION_EXIT("Convert_CHS_To_LBA")

    return;

  }

  /* To convert a CHS into an LBA, use the formula:

          LBA = (Cylinder * Sectors Per Cylinder) + (Head * sectors per track) + (Sector - 1)

     The (Sector - 1) comes from the fact that LBA is 0 based whereas CHS is not, at least for the Sector portion of the address. */
  *Starting_Sector = (Cylinder * DriveArray[Index].Sectors_Per_Cylinder) + (Head * DriveArray[Index].Geometry.Sectors) + (Sector - 1);

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Convert_CHS_To_LBA")

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
static void _System Build_Features_List(Partition_Data * PartitionRecord, DLIST Features_List, CARDINAL32 * Error)
{

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


  FUNCTION_ENTRY("Build_Features_List")

  /* We will follow the feature data context chain for this partition, if it has one.  */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    /* There are no features on this partition. */
    *Error = LVM_ENGINE_NO_ERROR;

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

      /* This should not happen!  Abort! */
      *Error = LVM_ENGINE_INTERNAL_ERROR;

      FUNCTION_EXIT("Build_Features_List")

      return;

    }

    /* If our current feature is not in the list of features, then add it. */
    if ( ! Search_Data.Found )
    {

      /* Get its current class data. */
      Function_Table = Feature_Data->Function_Table;
      Feature_App_Data.Function_Table = Feature_Data->Function_Table;
      PartitionRecord->Feature_Data = Feature_Data;
      Function_Table->ReturnCurrentClass(PartitionRecord, &Feature_App_Data.Actual_Class, &Feature_App_Data.Top_Of_Class, &Feature_App_Data.Feature_Sequence_Number );
      Feature_App_Data.Init_Data = NULL;

      /* Restore the feature data for the partition. */
      PartitionRecord->Feature_Data = Original_Feature_Data;

      /* Add the feature to the list of features. */
      InsertItem(Features_List, sizeof( Feature_Application_Data ), &Feature_App_Data, FEATURE_APPLICATION_DATA_TAG, NULL, AppendToList, FALSE, Error );
      if ( *Error != DLIST_SUCCESS )
      {

        if ( *Error == DLIST_OUT_OF_MEMORY )
          *Error = LVM_ENGINE_OUT_OF_MEMORY;
        else
          *Error = LVM_ENGINE_INTERNAL_ERROR;

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


  /* Indicate success. */
  *Error = LVM_ENGINE_NO_ERROR;

  FUNCTION_EXIT("Build_Features_List")

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
void _System Update_Partitions_Parent_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access our Parameters without having to typecase each time. */
  ADDRESS          CurrentParent = Parameters;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *       Current_Context;

  FUNCTION_ENTRY("Update_Partitions_Parent_Data")


#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FUNCTION_EXIT("Update_Partitions_Parent_Data")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* We have a partition.  Lets set its parent. */
    PartitionRecord->Parent_Handle = CurrentParent;

    /* Does this Partition Record represent an aggregate? */
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

      /* We will call ForEachItem to process the PartitionRecord in this Partitions List also. */
      ForEachItem( Current_Context->Partitions, &Update_Partitions_Parent_Data, PartitionRecord->External_Handle, TRUE, Error);

    }
    else
    {

      /* We must set the ChangesMade flag on the drive that corresponds to this partition record.   */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

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

      FUNCTION_EXIT("Update_Partitions_Parent_Data")

      return;

  }

#endif

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Update_Partitions_Parent_Data")

  return;

}

