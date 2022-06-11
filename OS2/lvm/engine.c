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
 * Module: engine.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following two functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in ENGINE.C:
 *
 *            void                     Open_LVM_Engine
 *            BOOLEAN                  Commit_Changes
 *            void                     Close_LVM_Engine
 *            Drive_Control_Array      Get_Drive_Control_Data
 *            Drive_Information_Record Get_Drive_Status
 *            void                     Set_Name
 *            CARDINAL32               Get_Valid_Options
 *            BOOLEAN                  Reboot_Required
 *            void                     Set_Min_Install_Size
 *            void                     Start_Logging
 *            void                     Stop_Logging
 *            void                     Export_Configuration
 *
 * Description: Engine.H and Engine.C define (and hold) the core data of
 *              the LVM engine.  Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *
 *
 *    Partition_Manager.C                                     Volume_Manager.C
 *            \                                               /
 *              \  |-------------------------------------|  /
 *                \|                                     |/
 *                 |        Engine.H and Engine.C        |
 *                /|                                     |\
 *              /  |-------------------------------------|  \
 *            /                  |                            \
 *    BootManager.C          Logging.h                       Handle_Manager.C
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

#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR
#include <os2.h>      /* DosError, FERR_DISABLEHARDERR */
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* strncpy, strlen */
#include <ctype.h>    /* toupper */
#include <stdio.h>    /* sprintf */
#include <time.h>     /* struct tm, localtime */
#define NEED_BYTE_DEFINED
#define  DECLARE_GLOBALS 1
#include "engine.h"   /* Include engine.h to declare the global types and variables. */
#include "lvm_gbls.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */
#include "dlist.h"    /* CreateList, DestroyList, ForEachItem */
#include "diskio.h"   /* OpenDrives, CloseDrives, GetDriveCount, GetDriveGeometry */


#include "lvm_cons.h" /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */

#define NEED_BYTE_DEFINED
#include "lvm_intr.h" /* Open_LVM_Engine, Close_LVM_Engine, Commit_Changes, Get_Drive_Control_Data,
                                Export_Configuration, Get_Drive_Status, Get_Reboot_Flag, Get_Valid_Options,
                                Reboot_Required, Set_Min_Install_Size, Set_Name, Set_Reboot_Flag, Start_Logging,
                                Stop_Logging */

#include "lvm_hand.h" /* Initialize_Handle_Manager, Create_Handle, Destroy_Handle, Translate_Handle */
#include "Partition_Manager.h" /* Initialize_Partition_Manager, Close_Partition_Manager, Discover_Partitions, Commit_Partition_Changes */
#include "Volume_Manager.h"    /* Initialize_Volume_Manger, Close_Volume_Manager, Discover_Volumes, Commit_Volume_Changes */
#include "BootManager.h"       /* Discover_Boot_Manager */
#include "crc.h"               /* Build_CRC_Table, CalculateCRC, INITIAL_CRC */
#include "logging.h"           /* Log_Current_Configuration, Write_Log_Buffer, Logging_Enabled */
#include "lvm_plug.h"          /* LVM_Plugin_DLL_Interface, LVM_Common_Services_V1, Plugin_Function_Table_V1, PLUGIN_FUNCTION_TABLE_V1_TAG */
#include "Bad_Block_Relocation.h"
#include "Pass_Thru.h"

#ifdef DEBUG

#include <assert.h>

#endif


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define CRC_POLYNOMIAL     0xEDB88320L



/*--------------------------------------------------
 * Private Type Definitions
 --------------------------------------------------*/
typedef struct _Name_Counter {
                               CARDINAL32     Free_Space_Counter;
                               CARDINAL32     Available_Counter;
                               CARDINAL32     Corrupt_Counter;
                               CARDINAL32     Drive_Count;
                             } Name_Counter;

typedef struct _Find_And_Parse_Record {
                                        DLIST                               Tokens;
                                        LVM_Feature_Specification_Record  * Feature_Data;
                                        char **                             Error_Message;
                                        BOOLEAN                             Feature_Found;
                                        CARDINAL32                        * Error_Code;
                                      } Find_And_Parse_Record;

/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static CARDINAL32 Serial_Numbers_Issued = 0;            /* Used by the Create_Serial_Number function. */
static BYTE       Kill_Sector[BYTES_PER_SECTOR];        /* Used to overwrite the sectors in the KillSector list. */


/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
static void      _System DetermineFreeSpace(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Overwrite_Sectors(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Close_All_Features(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Assign_Serial_Numbers_And_Names(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Update_Disk_Names_In_Signature_Sectors(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Clear_Startable_Flags(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Check_For_Volume_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      APIENTRY Exit_Procedure( CARDINAL32 Error_Code );
static BOOLEAN   Load_Plugins(CARDINAL32 * Error_Code);
static void      _System Process_Plugins(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

static BOOLEAN AddDriveLinking_Plugins( CARDINAL32 * Error);

static void      _System Fill_Handle_Array(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static INTEGER32 _System Sort_Plugins(ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag,CARDINAL32 * Error_Code);
static void      _System Find_Feature_And_Parse(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void      _System Free_Expansion_DLLs(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/
/*--------------------------------------------------
 * External Functions
 --------------------------------------------------*/
void Discover_Boot_Manager( CARDINAL32 * Error_Code );
void Migrate_Old_Boot_Manager_Menu_Items( CARDINAL32 * Error_Code );
void Commit_Boot_Manager_Changes( CARDINAL32 * Error_Code );
void Close_Boot_Manager( void );
BOOLEAN Can_Boot_Manager_Be_Installed( CARDINAL32 * Error_Code);


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
/*           LVM_Interface_Type of VIO_Interface.                                                   */
/*                                                                                                  */
/****************************************************************************************************/
void _System Open_LVM_Engine( BOOLEAN Ignore_CHS, CARDINAL32 * Error_Code )
{

  API_ENTRY( "Open_LVM_Engine - compatibility API" )

  Open_LVM_Engine2(Ignore_CHS, VIO_Interface, Error_Code);

  API_EXIT( "Open_LVM_Engine - compatibility API" )

  return;

}


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
/*   Notes:  None.                                                                                  */
/*                                                                                                  */
/****************************************************************************************************/
void _System Open_LVM_Engine2( BOOLEAN Ignore_CHS, LVM_Interface_Types Interface_Type, CARDINAL32 * Error_Code )
{

  CARDINAL32            Index;                   /* Used to step through the DriveArray. */
  Name_Counter          Name_Count;              /* Used when creating unique names for Volumes, Partitions, and Drives. */
  APIRET                ReturnCode;              /* Used with the DosError API. */
  DDI_Rediscover_param  Rediscovery_Parameters;  /* Used to do a PRM Rediscover so that PRMs with new media will be recognized. */
  DDI_Rediscover_data   Rediscovery_Data;        /* Used to do a PRM Rediscover so that PRMs with new media will be recognized. */
  CARDINAL32            Ignore_Error;            /* Used on error paths. */

  /* Opening the LVM Engine consists of:

        Allocating the global data structures
        Finding out the number of drives we have to deal with
        Finding out the geometry of the drives
        Obtaining handles to access the drives
        Initializing the global data structures
        Initializing the Partition Manager
        Initializing the Volume Manager
        Discovering what partitions may exist on the drives
        Discovering what volumes amy exist

  */


  API_ENTRY( "Open_LVM_Engine" )

  /* Has the engine been opened already? */
  if ( DriveArray != NULL )
  {

    LOG_ERROR("The LVM Engine is ALREADY open!")

    API_EXIT( "Open_LVM_Engine" )

    /* The LVM Engine has already been opened! */
    *Error_Code = LVM_ENGINE_ALREADY_OPEN;

    return;

  }

  /* Save the interface being used. */
  Interface_In_Use = Interface_Type;

  /* We need to disable Hard Error popups. */
  ReturnCode = DosError(FERR_DISABLEHARDERR);

#ifdef DEBUG

#ifdef PARANOID

  assert( ReturnCode == NO_ERROR );

#else

  if ( ReturnCode != NO_ERROR )
  {

    LOG_ERROR1("Attempt to disable hard error popups failed.","Return Code", ReturnCode)

    API_EXIT( "Open_LVM_Engine" )

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* We need to initialize the global variables for the LVM Engine. */

  /* Initialize the Available_Features list. */
  Available_Features = NULL;
  Available_Features = CreateList();

  /* Did we succeed? */
  if ( Available_Features == NULL )
  {

    LOG_ERROR("LVM_ENGINE_OUT_OF_MEMORY.  Can't create the Available_Features list.")

    API_EXIT( "Open_LVM_Engine" )

    /* Since we could not create the Available_Features list, we must be out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    return;

  }

  /* Initialize the Filesystem_Expansion_DLLs list. */
  Filesystem_Expansion_DLLs = NULL;
  Filesystem_Expansion_DLLs = CreateList();


  /* Did we succeed? */
  if ( Filesystem_Expansion_DLLs == NULL )
  {

    LOG_ERROR("LVM_ENGINE_OUT_OF_MEMORY.  Can't create the Filesystem_Expansion_DLLs list.")

    API_EXIT( "Open_LVM_Engine" )

    /* Eliminate the Available_Features list. */
    DestroyList(&Available_Features, TRUE, &Ignore_Error);

    /* Since we could not create the KillSector list, we must be out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    return;

  }

  /* Initialize the KillSector list. */
  KillSector = NULL;
  KillSector = CreateList();

  /* Did we succeed? */
  if ( KillSector == NULL )
  {

    LOG_ERROR("LVM_ENGINE_OUT_OF_MEMORY.  Can't create the KillSector list!")

    API_EXIT( "Open_LVM_Engine" )

    /* Eliminate the Available_Features list. */
    DestroyList(&Available_Features, TRUE, &Ignore_Error);

    /* Eliminate the Filesystem_Expansion_DLLs list. */
    DestroyList(&Filesystem_Expansion_DLLs, TRUE, &Ignore_Error);

    /* Since we could not create the KillSector list, we must be out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    return;

  }

  /* Initialize the Kill_Sector to all 0xf6. */
  memset(&Kill_Sector,0xf6,BYTES_PER_SECTOR);

  /* Now let's build the CRC table so that the CalculateCRC function will work. */
  Build_CRC_Table();

  /* Now lets  initialize the Handle Manager. */
  if ( ! Initialize_Handle_Manager() )
  {

    /* We could not initialize the Handle Manager.  We are probably out of memory! */

    /* Eliminate the Available_Features list. */
    DestroyList(&Available_Features, TRUE, &Ignore_Error);

    /* Eliminate the Filesystem_Expansion_DLLs list. */
    DestroyList(&Filesystem_Expansion_DLLs, TRUE, &Ignore_Error);

    /* Eliminate the KillSector list. */
    DestroyList(&KillSector, TRUE, &Ignore_Error);

    /* Ensure that the KillSector list is NULL so that Close_LVM_Engine doesn't try to do something with it. */
    KillSector = NULL;

    LOG_ERROR("LVM_ENGINE_OUT_OF_MEMORY.  Unable to initialize the Handle Manager!")

    API_EXIT( "Open_LVM_Engine" )

    /* Indicate the error we are reporting. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    return;

  }

  API_EXIT( "Open_LVM_Engine" )

  /* Now we need to gather information about the drives in the system.  To do this, we must initialize the DiskIO module. */
  if ( ! OpenDrives( Error_Code ) )
  {

    /* We could not open the DiskIO module!  Abort. */
    Close_LVM_Engine();

    LOG_ERROR("LVM_ENGINE_DRIVE_OPEN_FAILURE.  Unable to open the DiskIO module!")

    API_EXIT( "Open_LVM_Engine" )

    /* Set *Error_Code to indicate the problem. */
    *Error_Code = LVM_ENGINE_DRIVE_OPEN_FAILURE;

    return;

  }

  /* How many drives are there in the system? */
  DriveCount = GetDriveCount( Error_Code );

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DISKIO_NO_ERROR );

#else

  if ( *Error_Code != DISKIO_NO_ERROR )
  {

    /* This should not be possible here, so something is really wrong!  Abort! */
    Close_LVM_Engine();

    LOG_ERROR("LVM_ENGINE_INTERNAL_ERROR.  Error getting the drive count!")

    API_EXIT( "Open_LVM_Engine" )

    /* Set the error return code. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  if ( DriveCount == 0 )
  {

    /* There are no drives to partition.  Abort. */
    Close_LVM_Engine();

    LOG_ERROR("LVM_ENGINE_NO_DRIVES_FOUND")

    API_EXIT( "Open_LVM_Engine" )

    /* Set the error return code. */
    *Error_Code = LVM_ENGINE_NO_DRIVES_FOUND;

    return;

  }

  /* Perform a PRM Rediscover operation so that any PRMs with new media will be recognized. */
  Rediscovery_Parameters.DDI_TotalDrives = 0;
 /* 0 here indicates that a PRM only rediscover is to be performed instead of a full rediscover. */
  Rediscovery_Parameters.DDI_aDriveNums[0] = 1;
  Rediscovery_Data.DDI_TotalExtends = 0;
  Rediscovery_Data.NewIFSMUnits = 0;

  /* Do the PRM Rediscovery.  */
  ReturnCode = Rediscover( &Rediscovery_Parameters, &Rediscovery_Data );

  /* Did we succeed? */
  if ( ReturnCode != NO_ERROR )
  {

    /* If we are running on Aurora, this IOCTL can fail if LVM's Dynamic Drive Discovery has been disabled.  Currently, Fault Tolerance (FT)
       is unable to handle the dynamic discovery of drives.  When FT is active, LVM's Dynamic Drive Discovery is disabled.  When LVM's
       Dynamic Drive Discovery is disabled, this IOCTL will return ERROR_GEN_FAILURE.  If we are on running on system where LVM is not installed,
       then this IOCTL will return a different error code.  Thus, we can use this error to determine if we are running on Aurora with FT active
       or if we are running on some older release of OS/2.                                                                                         */
    if ( ReturnCode != ERROR_GEN_FAILURE )
    {

      Merlin_Mode = TRUE;

      LOG_EVENT1("PRM Rediscovery failed.","Return Code", ReturnCode)

      LOG_EVENT("Enterning Merlin Mode.")

    }
    else
    {

      LOG_EVENT1("PRM Rediscovery failed.","Return Code",ReturnCode)

      LOG_EVENT("Staying in Aurora Mode.")

    }

  }

  /* We must install an exit procedure so that, if our process crashes after we disable PRM Rediscovery, we can enable PRM Rediscovery.  If we don't
     do this, then the user could lose access to their PRMs until the system is rebooted.                                                            */
  if ( ! Merlin_Mode )
  {

    /* Install the exit procedure which enables PRM Rediscovery. */
    DosExitList(EXLST_ADD, &Exit_Procedure);

    /* Now disable PRM Rediscovery while the engine is running. */
    PRM_Rediscovery_Control(FALSE);

  }

  /* Allocate memory for the drive array. */
  DriveArray = (Disk_Drive_Data *) malloc( DriveCount * sizeof(Disk_Drive_Data) );

  /* Did we get the memory? */
  if ( DriveArray == NULL )
  {

    /* We did not get the memory!  Abort! */
    Close_LVM_Engine();

    LOG_ERROR("LVM_ENGINE_OUT_OF_MEMORY.  Could not allocate memory for the DriveArray!")

    API_EXIT( "Open_LVM_Engine" )

    /* Set the error return code. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    return;

  }

  /* Clear out the DriveArray. */
  memset(DriveArray,0,DriveCount * sizeof(Disk_Drive_Data) );

  /* Initialize each record in the drive array. */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    /* Set the Record_Initialized flag to FALSE just in case we don't finish initializing this record. */
    DriveArray[Index].Record_Initialized = FALSE;

    /* Get the drive geometry. */
    GetDriveGeometry( Index + 1, &(DriveArray[Index].Geometry),&(DriveArray[Index].Is_PRM), &(DriveArray[Index].Cylinder_Limit_Applies), Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DISKIO_NO_ERROR );

#else

    /* Did we get the geometry? */
    if ( *Error_Code != DISKIO_NO_ERROR )
    {

      /* This call should not have failed!  Something is very wrong - abort! */
      Close_LVM_Engine();

      LOG_ERROR("LVM_ENGINE_INTERNAL_ERROR.  Unable to get the drive geometry!")

      API_EXIT( "Open_LVM_Engine" )

      /* Set the error return code. */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

#endif

#endif

    /* If we are NOT running on Aurora, then the PRM information we got from the GetDriveGeometry function will be invalid as
       only Aurora and later support the method used to obtain this information.  As a result, we will turn off the Is_PRM flag
       as we can not be sure that it should be turned on.                                                                        */
    if ( Merlin_Mode )
      DriveArray[Index].Is_PRM = FALSE;

    /* Now that we have the geometry, we can calculate some values that will be needed later. */
    DriveArray[Index].Sectors_Per_Cylinder = DriveArray[Index].Geometry.Heads * DriveArray[Index].Geometry.Sectors;
    DriveArray[Index].Cylinder_Limit = ( 1024 * DriveArray[Index].Sectors_Per_Cylinder );

    /* Now that we have the geometry, we can calculate the size of the drive. */
    DriveArray[Index].Drive_Size = DriveArray[Index].Geometry.Cylinders * DriveArray[Index].Sectors_Per_Cylinder;

    /* Initialize the flags. */
    DriveArray[Index].ChangesMade = FALSE;
    DriveArray[Index].IO_Error = FALSE;
    DriveArray[Index].Corrupt = FALSE;
    DriveArray[Index].NonFatalCorrupt  = FALSE;

    DriveArray[Index].Is_Big_Floppy = FALSE;
    DriveArray[Index].Unusable = FALSE;
    DriveArray[Index].Fake_Volumes_In_Use = FALSE;

    /* Save the Index used to access this entry in the DriveArray.  It will be needed later by those who reach this entry through a
       handle instead of through the DriveArray.                                                                                      */
    DriveArray[Index].DriveArrayIndex = Index;

    /* Initialize counters. */
    DriveArray[Index].Primary_Partition_Count = 0;
    DriveArray[Index].Logical_Partition_Count = 0;

    /* Initialize the Drive_Serial_Number to 0 as that is currently not known.  That will be discovered when we start
       looking for partitions.  The same goes for the Drive_Name.                                                       */
    DriveArray[Index].Drive_Serial_Number = 0;
    DriveArray[Index].Drive_Name[0] = 0;
    DriveArray[Index].Boot_Drive_Serial_Number = 0;

    /* Now lets get a handle for this DriveArray entry. */
    DriveArray[Index].External_Handle = Create_Handle(&(DriveArray[Index]),DISK_DRIVE_DATA_TAG,sizeof(Disk_Drive_Data),Error_Code);
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      /* We could not create the handle.  We must abort. */
      Close_LVM_Engine();

      LOG_ERROR("Unable to create a handle for an entry in the DriveArray!")

      API_EXIT( "Open_LVM_Engine" )

      /* The error code was already set by Create_Handle, so we need to just return. */
      return;

    }

    /* Create the Partitions list.  This list will be used to track the partitions that exist on the disk drive which
       corresponds to this entry in the DriveArray.                                                                     */
    DriveArray[Index].Partitions = CreateList();
    if ( DriveArray[Index].Partitions == NULL )
    {

      /* We could not create the Partitions list, so we are most likely out of memory! Abort. */
      Close_LVM_Engine();

      LOG_ERROR1("LVM_ENGINE_OUT_OF_MEMORY","Drive Index", Index)

      API_EXIT( "Open_LVM_Engine" )

      /* Indicate the error. */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      return;

    }

    /* Now this record is sufficiently initialized that we can set Record_Initialized flag.  The Record_Initialized flag is used by Close_LVM_Engine
       to determine which records in the DriveArray have fields that require deallocation, such as an initializes Partitions field.                       */
    DriveArray[Index].Record_Initialized = TRUE;

  } /* End of for loop. */

  /* Now that the global data has been allocated and initialized, it is time to initialize the Partition Manager. */
  Initialize_Partition_Manager( Ignore_CHS, Error_Code );
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* We could not initialize the Partition Manager.  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR("Unable to initialize the Partition Manager!")

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Initialize_Partition_Manager, so all we have to do now is return. */
    return;

  }


  /* Now that the Partition Manager has been initialize, it is time to initialize the Volume Manager. */
  if ( ! Initialize_Volume_Manager( Error_Code ) )
  {

    /* We could not initialize the Volume Manager.  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR1("Initialize_Volume_Manager failed.","Error code", *Error_Code)

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Initialize_Volume_Manager, so all we have to do now is return. */
    return;

  }


  /* Now we must look for any plugin features. */
  if ( ! Load_Plugins(Error_Code) )
  {

    /* We had a major error while attempting to load LVM Plug-ins.  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR("Unable to load plug-in features!")

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Load_Plugins, so all we have to do now is return. */
    return;

  }


  /* Now that all of the setup work has been done, lets see what partitions are out there! */
  Discover_Partitions( Error_Code );

  /* Was there an error? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* There was an error during partition discovery!  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR1("Discover Partitions failed.","Error code", *Error_Code)

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Initialize_Partition_Manager, so all we have to do now is return. */
    return;

  }

  /* We will assume that we booted off of the first drive.  If we did not, then Boot Manager must be installed,
     and Discover_Boot_Manager will correct our assumption.                                                      */
  Boot_Drive_Serial_Number = DriveArray[0].Drive_Serial_Number;

  /* Now we need to discover if Boot Manager is out there. */
  Discover_Boot_Manager( Error_Code );

  /* Was there an error? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* There was an error during Boot Manager discovery!  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR1("Discover_Boot_Manager failed.","Error code", *Error_Code)

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Discover_Boot_Manager, so all we have to do now is return. */
    return;

  }


  /* Now that all of the partitions have been discovered, lets see what volumes are out there! */
  Discover_Volumes( Error_Code );

  /* Was there an error? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* There was an error during volume discovery!  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR1("Discover_Volumes failed.","Error code", *Error_Code)

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Discover_Volumes, so all we have to do now is return. */
    return;

  }

  /* Now we can migrate any items on the Boot Manager Menu from the old format to the new format. */
  Migrate_Old_Boot_Manager_Menu_Items( Error_Code );

  /* Was there an error? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    /* There was an error during Boot Manager Migration!  Close the LVM Engine! */
    Close_LVM_Engine();

    LOG_ERROR1("Migrate_Old_Boot_Manager_Menu_Items failed.","Error code", *Error_Code)

    API_EXIT( "Open_LVM_Engine" )

    /* The Error_Code was set by the call to Migrate_Old_Boot_Manager_Menu_Items, so all we have to do now is return. */
    return;

  }

  /* Lets make sure that all drives and partitions have serial numbers and names. */

  /* Set up the counters. */
  Name_Count.Drive_Count = 1;
  Name_Count.Free_Space_Counter = 1;
  Name_Count.Available_Counter = 1;
  Name_Count.Corrupt_Counter = 1;

  /* Examine all of the drives and partitions.  Assign names and serials numbers as needed. */
  for (Index = 0; Index < DriveCount; Index++)
  {

    /* Does the current drive have a serial number? */
    if ( DriveArray[Index].Drive_Serial_Number == 0 )
    {

      /* Assign this drive a serial number. */
      DriveArray[Index].Drive_Serial_Number = Create_Serial_Number();

      /* Mark the drive as having been changed. */
      DriveArray[Index].ChangesMade = TRUE;

    }

    /* Does the current drive already have a name? */
    if ( DriveArray[Index].Drive_Name[0] == 0 )
    {

      /* Since this drive does not have a name, lets give it one. */
      Name_Count.Drive_Count = Index + 1;
      if ( !Create_Unique_Name(DISK_NAMES, TRUE, "D", &(Name_Count.Drive_Count), DriveArray[Index].Drive_Name, DISK_NAME_SIZE) )
      {

        /* This should never fail as the number of drives that can be put into a system is far less than the number of names
           that are available for it.  Therefore, we must have had an internal error of some kind.                            */
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        Close_LVM_Engine();

        LOG_ERROR("LVM_ENGINE_INTERNAL_ERROR.  Could not create a unique disk name!")

        API_EXIT( "Open_LVM_Engine" )

        return;

      }

      /* Mark the drive as having been changed. */
      DriveArray[Index].ChangesMade = TRUE;

    }

    /* Now check the partitions on this drive. */
    ForEachItem(DriveArray[Index].Partitions,&Assign_Serial_Numbers_And_Names, &Name_Count, TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS)
    {

      /* There should never be an error here!  Something has corrupted our data structures! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      Close_LVM_Engine();

      LOG_ERROR("LVM_ENGINE_INTERNAL_ERROR.  Failure while trying to assign serial numbers and names to partitions which don't have them!")

      API_EXIT( "Open_LVM_Engine" )

      return;

    }

  }

  /* Log our current state. */
  Log_Current_Configuration();

  API_EXIT( "Open_LVM_Engine" )

  /* All done!  Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
BOOLEAN Commit_Changes( CARDINAL32 * Error_Code )
{

  CARDINAL32   Partition_Error = LVM_ENGINE_NO_ERROR;   /* Used to hold the error code from the Commit_Partition_Changes function. */
  CARDINAL32   Volume_Error = LVM_ENGINE_NO_ERROR;      /* Used to hold the error code from the Commit_Volume_Changes function. */
  CARDINAL32   Boot_Manager_Error = LVM_ENGINE_NO_ERROR;/* Used to hold the error code from the Commit_Boot_Manager_Changes function. */

  CARDINAL32   Index;                                   /* Used to walk the drive array. */

  API_ENTRY( "Commit_Changes" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Commit_Changes" )

    /* The Engine has not been opened yet, so there are no changes to commit! */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return FALSE;

  }

  /* Log our current state. */
  LOG_EVENT("The following configuration report is the LVM configuration prior to attempting to commit any changes.")
  Log_Current_Configuration();

  /* Before we can commit any changes to disk, we must clear the I/O error flag for each drive.  This flag is used by
     Commit_Partition_Changes and Commit_Volume_Changes to indicate I/O errors which occurred during their attempts to access the disk. */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    /* Is the Corrupt flag set?  If it is, we don't want to change anything! */
//EK    if ( ! DriveArray[Index].Corrupt )
    if ( ! DriveArray[Index].Corrupt || DriveArray[Index].NonFatalCorrupt)
    {

      /* The Corrupt flag was not set.  Set the IO_Error flag. */
      DriveArray[Index].IO_Error = FALSE;

    }

  }

  /* Before we can commit any partition or volume changes, we must process the KillSector list. */

  /* Are there any sectors in the list to kill? */
  if ( GetListSize(KillSector, Error_Code) > 0 )
  {

    /* We must kill all of the sectors in this list by overwriting them. */
    ForEachItem(KillSector, &Overwrite_Sectors, NULL, TRUE, Error_Code);

  }

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Was there an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR("Failure while processing the KillSector list!")

    API_EXIT( "Commit_Changes" )

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return FALSE;

  }

#endif

#endif

  /* Since the KillSector list was processed successfully, we can delete any entries in it. */
  DeleteAllItems (KillSector, TRUE, Error_Code);
#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  /* Was there an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR("Failure while processing the KillSector list!")

    API_EXIT( "Commit_Changes" )

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return FALSE;

  }

#endif

#endif

  LOG_EVENT("Committing partition changes")

  /* Commit all of the changes to disk now. */
  Commit_Partition_Changes( &Partition_Error );

  if ( Partition_Error != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Commit_Partition_Changes failed!","Error Code",Partition_Error)

  }

  /* If there were no errors, or there was no internal error, then we can commit the volume changes.  If there was
     an I/O error during the committing of the partition changes, then the entry in the DiskArray for the drive
     having the error will be marked.  The Volume Manager will check the DriveArray for I/O errors and if it
     finds any, check to see if any of its Volumes are affected by the I/O error.  Volumes affected by the I/O
     error will not be created.                                                                                      */
  if ( Partition_Error != LVM_ENGINE_INTERNAL_ERROR )
  {

    LOG_EVENT("Committing volume changes")

    /* Commit the volume changes. */
    Commit_Volume_Changes( &Volume_Error );

    if ( Volume_Error != LVM_ENGINE_NO_ERROR )
    {

      LOG_ERROR1("Commit_Volume_Changes","Error code", Volume_Error)

    }

    /* Is it safe to try to commit the Boot Manager changes? */
    if ( Volume_Error != LVM_ENGINE_INTERNAL_ERROR )
    {

      LOG_EVENT("Committing Boot Manager changes")

      /* Commit the Boot Manager changes. */
      Commit_Boot_Manager_Changes( &Boot_Manager_Error );

      if ( Boot_Manager_Error != LVM_ENGINE_NO_ERROR )
      {

        LOG_ERROR1("Commit_Boot_Manager_Changes failed.", "Error code", Boot_Manager_Error)

      }

    }

  }

  /* Were all of the changes committed successfully? */
  if ( ( Partition_Error != LVM_ENGINE_NO_ERROR ) || ( Volume_Error != LVM_ENGINE_NO_ERROR ) || ( Boot_Manager_Error != LVM_ENGINE_NO_ERROR ) )
  {

    /* Was the error reported by the partition manager? */
    if ( Partition_Error != LVM_ENGINE_NO_ERROR )
    {

      /* Pass the error back to the caller. */
      *Error_Code = Partition_Error;

    }
    else
    {

      /* Was the error reported by the Volume Manager? */
      if ( Volume_Error != LVM_ENGINE_NO_ERROR )
      {

        /* Pass the error back to our caller. */
        *Error_Code = Volume_Error;

      }
      else
      {

        /* The error must have come from the Boot Manager.  Pass the error back to our caller. */
        *Error_Code = Boot_Manager_Error;

      }

    }

    LOG_EVENT1("Returning FALSE with an error code to caller.", "Error code", *Error_Code)

    API_EXIT( "Commit_Changes" )

    return FALSE;

  }
  else
  {

    /* Since all of the changes were successfully committed, we must clear all of the ChangesMade flags in the system
       as there are no changes pending.                                                                                */
    for ( Index = 0; Index < DriveCount; Index++ )
    {

      /* Is the IO_Error flag set?  If it is, we don't want to change anything! */
      if ( ! DriveArray[Index].IO_Error )
      {

        /* The IO_Error flag was not set, so turn off the ChangesMade flag. */
        DriveArray[Index].ChangesMade = FALSE;

      }

    }

    /* Indicate no error and return. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  /* Log our current state. */
  LOG_EVENT("The following configuration report is the LVM configuration AFTER  changes.")
  Log_Current_Configuration();
  
  API_EXIT( "Commit_Changes" )

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Close_LVM_Engine                                 */
/*                                                                   */
/*   Descriptive Name: Closes the LVM Engine and frees any memory    */
/*                     held by the LVM Engine.                       */
/*                                                                   */
/*   Input: None.                                                    */
/*   Output:  None.                                                  */
/*   Error Handling: N/A                                             */
/*   Side Effects:  Any memory held by the LVM Engine is released.   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void Close_LVM_Engine ( void )
{

  CARDINAL32     Index;    /* Used to access the drive array. */
  CARDINAL32     Error;    /* Used to hold the error return code from Destroy_All_Handles. */

  API_ENTRY( "Close_LVM_Engine" )

  /* Has the DriveArray been initialized? */
  if ( DriveArray != NULL)
  {

    /* If the DriveArray has been initialized, then the Partition_Manager and the Volume_Manager may have been initialized also.
       Since we are closing the engine, close them as well.                                                                         */
    Close_Volume_Manager();
    Close_Partition_Manager();
    Close_Boot_Manager();

    /* The DriveArray has been initialized to some degree.  We must step through the initialized entries and free their Partition lists. */
    for ( Index = 0; Index < DriveCount; Index++ )
    {

      if ( DriveArray[Index].Record_Initialized )
      {

        /* We need to dispose of the Partitions list for this entry in the DriveArray. */
        DestroyList( &(DriveArray[Index].Partitions), TRUE, &Error );

      }
      else
      {

        /* Since we have found a record which was not initialized, then this is as far as the
           Open_LVM_Engine function got before it aborted.  We don't need to examine anymore
           records in the DriveArray.  We will set Index equal to DriveCount so that we will
           abort the for loop.                                                                 */
        Index = DriveCount;

      }

    } /* end of for loop. */

    /* Now we can deallocate the DriveArray. */
    free(DriveArray);

    /* Set DriveArray to NULL so that it is clear that the LVM Engine has not been initialized. */
    DriveArray = NULL;

  }

  /* Has the Available_Features list been created yet? */
  if ( Available_Features != NULL )
  {

    /* We must cycle through the entries in the Available_Features list and call the
       Close_Feature entry point for each Feature.                                           */
    ForEachItem(Available_Features, &Close_All_Features, NULL, TRUE, &Error);

    /* Eliminate the Available_Features list. */
    DestroyList(&Available_Features, TRUE, &Error);

    /* Ensure that Available_Features is NULL. */
    Available_Features = NULL;

  }

  /* Has the Filesystem_Expansion_DLLs list been created yet? */
  if ( Filesystem_Expansion_DLLs != NULL )
  {

    /* We must cycle through the entries in the Filesystem_Expansion_DLLs list and release
       each of the DLLs in the list.                                                          */
    ForEachItem(Filesystem_Expansion_DLLs, &Free_Expansion_DLLs, NULL, TRUE, &Error);

    /* Eliminate the Filesystem_Expansion_DLLs list. */
    DestroyList(&Filesystem_Expansion_DLLs, TRUE, &Error);

    /* Ensure that Filesystem_Expansion_DLLs is NULL. */
    Filesystem_Expansion_DLLs = NULL;

  }

  /* Has the KillSector list been allocated yet? */
  if ( KillSector != NULL )
  {

    /* Eliminate the KillSector list. */
    DestroyList(&KillSector, TRUE, &Error);

    /* Ensure that the KillSector list is NULL so that Close_LVM_Engine doesn't try to do something with it. */
    KillSector = NULL;

  }

  /* Set the DriveCount to 0 so that it agrees with the DriveArray being NULL (i.e. no drive array - no drives). */
  DriveCount = 0;

  /* Reset the various flags and counters. */
  Min_Install_Size = 0;
  RebootRequired = FALSE;
  Install_Volume_Handle = NULL;
  Boot_Drive_Serial_Number = 0;
  Boot_Manager_Active = FALSE;
  Boot_Manager_Handle = NULL;
  Min_Free_Space_Size = 2048;
  Reserved_Drive_Letters = 0;
  Merlin_Mode = FALSE;

  /* Delete any handles which may be in use. */
  Destroy_All_Handles( &Error );

  /* Now enable PRM Rediscovery.  This may have been turned off when the engine was opened. */
  if ( ! Merlin_Mode)
    PRM_Rediscovery_Control(TRUE);
  else
    Merlin_Mode = FALSE;              /* Reset Merlin_Mode to its default value. */

  /* Close the DiskIO module as we don't need it anymore. */
  CloseDrives();

  /* Close the log file. */
  Stop_Logging( &Error );

  API_EXIT( "Close_LVM_Engine" )

  return;

}


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
void _System  Refresh_LVM_Engine( CARDINAL32 * Error_Code )
{

  API_ENTRY( "Refresh_LVM_Engine" )

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Refresh_LVM_Engine" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* If we are NOT running on Aurora, then we must skip the call to Reconcile_Drive_Letters
     as the operating system does not support the features required for it to work correctly. */
  if ( Merlin_Mode )
  {

    API_EXIT( "Refresh_LVM_Engine" )

    return;

  }

  /* Save the reserved drive letters as determined by Reconcile_Drive_Letters. */
  Reserved_Drive_Letters = Reconcile_Drive_Letters(TRUE, Error_Code);

  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_EVENT1("Reconcile_Drive_Letters failed.","Error code", *Error_Code)

  }

  API_EXIT( "Refresh_LVM_Engine" )

  /* All done. */
  return;

}


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
CARDINAL32 _System Get_Reserved_Drive_Letters ( CARDINAL32 * Error_Code )
{

  API_ENTRY( "Get_Reserved_Drive_Letters" )

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Reserved_Drive_Letters" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return 0;

  }

  LOG_EVENT1("Returning the reserved drive letters.","The reserved drive letter bitmap", Reserved_Drive_Letters)

  API_EXIT( "Get_Reserved_Drive_Letters" )

  return Reserved_Drive_Letters;

}


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
Drive_Control_Array Get_Drive_Control_Data( CARDINAL32 * Error_Code )
{

  Drive_Control_Array    ReturnValue;   /* The Drive_Control_Array returned to the caller. */
  CARDINAL32             Index;         /* Used for stepping through the DriveArray. */

  API_ENTRY( "Get_Drive_Control_Data" )

  /* Initialize the ReturnValue.  Use 0 and NULL so that if we abort we won't have to set these values. */
  ReturnValue.Count = 0;
  ReturnValue.Drive_Control_Data = NULL;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Drive_Control_Data" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return ReturnValue;

  }

  /* Now lets work on getting the real values to return to our caller. */

  /* Allocate memory for the Drive_Control_Data array in the ReturnValue. */
  ReturnValue.Drive_Control_Data = ( Drive_Control_Record * ) malloc( DriveCount * sizeof( Drive_Control_Record ) );

  /* Did we get the memory? */
  if ( ReturnValue.Drive_Control_Data == NULL )
  {

    LOG_ERROR("LVM_ENGINE_OUT_OF_MEMORY")

    API_EXIT( "Get_Drive_Control_Data" )

    /* Since we could not get the memory, we can not complete the operation.  Abort. */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    return ReturnValue;

  }

  /* Since we got the memory, lets copy the required data to it. */


  /* Step through both the DriveArray and our ReturnValue.Drive_Control_Data. */
  for ( Index = 0; Index < DriveCount; Index++ )
  {

    /* The DriveArray is 0 based, but OS/2's drive numbers are 1 based.  Thus, by adding 1 to Index, we get the OS/2 drive number for the current drive. */
    ReturnValue.Drive_Control_Data[Index].Drive_Number = Index + 1;

    /* Take the Drive_Size, Drive_Handle, and Drive_Serial_Number from the DriveArray. */
    ReturnValue.Drive_Control_Data[Index].Drive_Size = DriveArray[Index].Drive_Size;
    ReturnValue.Drive_Control_Data[Index].Drive_Serial_Number = DriveArray[Index].Drive_Serial_Number;
    ReturnValue.Drive_Control_Data[Index].Drive_Handle = DriveArray[Index].External_Handle;
    ReturnValue.Drive_Control_Data[Index].Drive_Is_PRM = DriveArray[Index].Is_PRM;

    /* Return the geometry for the drive. */
    ReturnValue.Drive_Control_Data[Index].Cylinder_Count = DriveArray[Index].Geometry.Cylinders;
    ReturnValue.Drive_Control_Data[Index].Heads_Per_Cylinder = DriveArray[Index].Geometry.Heads;
    ReturnValue.Drive_Control_Data[Index].Sectors_Per_Track = DriveArray[Index].Geometry.Sectors;

  }

  /* Finish off the ReturnValue by placing the number of drives in Count. */
  ReturnValue.Count = DriveCount;

  LOG_EVENT1("Returning data on X drives.", "X", ReturnValue.Count)

  API_EXIT( "Get_Drive_Control_Data" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return ReturnValue;

}


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
Drive_Information_Record Get_Drive_Status( ADDRESS Drive_Handle, CARDINAL32 * Error_Code )
{

  Disk_Drive_Data *          Drive_Data;   /* Used to access the data for the drive specified by Drive_Handle. */
  Drive_Information_Record   ReturnValue;  /* The value returned by this function. */
  ADDRESS                    Object;       /* Used when converting Drive_Handle into Drive_Data. */
  TAG                        ObjectTag;    /* Used when converting Drive_Handle into Drive_Data. */

  API_ENTRY( "Get_Drive_Status" )

  /* Initialize ReturnValue so that if we abort with an error, ReturnValue will be appropriate. */
  ReturnValue.Drive_Name[0] = 0;
  ReturnValue.Total_Available_Sectors = 0;
  ReturnValue.Largest_Free_Block_Of_Sectors = 0;
  ReturnValue.Corrupt_Partition_Table = FALSE;
  ReturnValue.Unusable = FALSE;
  ReturnValue.IO_Error = FALSE;
  ReturnValue.Is_Big_Floppy = FALSE;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Drive_Status" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return ReturnValue;

  }

  /* Is the Drive_Handle valid? */
  Translate_Handle(Drive_Handle,&Object, &ObjectTag, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_EVENT1("Translate_Handle failed.", "Error code", *Error_Code)

    API_EXIT( "Get_Drive_Status" )

    /* We have a bad handle.  Abort. */

    return ReturnValue;

  }

  /* The Drive_Handle may have been a valid handle, but a handle for what?  Lets find out. */
  if ( ObjectTag != DISK_DRIVE_DATA_TAG )
  {

    LOG_ERROR1("The specified handle is NOT the handle of a drive!", "The ObjectTag for this handle", ObjectTag)

    API_EXIT( "Get_Drive_Status" )

    /* This is a handle, just not a drive handle!  Abort. */
    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    return ReturnValue;

  }

  /* Since the handle was good, object must point to Disk_Drive_Data.  Initialize Drive_Data with Object. */
  Drive_Data = (Disk_Drive_Data *) Object;

  /* Since the handle was good, lets get the data requested. */

  /* To get the Largest_Free_Block_Of_Sectors and the Total_Available_Sectors, we must run the list of partitions for the drive and determine these values. */
  ForEachItem(Drive_Data->Partitions,&DetermineFreeSpace,&ReturnValue,TRUE,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert(*Error_Code == DLIST_SUCCESS);

#else

  /* Was the result successful? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("ForEachItem failed while attempting to determine the largest free block of sectors and the total available sectors!","Error code", *Error_Code)

    API_EXIT( "Get_Drive_Status" )

    /* Indicate an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    /* Set the fields modified to 0 so that we return all 0's in ReturnValue. */
    ReturnValue.Largest_Free_Block_Of_Sectors = 0;
    ReturnValue.Total_Available_Sectors = 0;

    /* Return to caller. */
    return ReturnValue;

  }

#endif

#endif

  /* Copy the remaining data from the DriveArray to ReturnValue. */
  strncpy( ReturnValue.Drive_Name , Drive_Data->Drive_Name, DISK_NAME_SIZE );
  ReturnValue.Corrupt_Partition_Table = Drive_Data->Corrupt;
  ReturnValue.IO_Error = Drive_Data->IO_Error;
  ReturnValue.Unusable = Drive_Data->Unusable;
  ReturnValue.Is_Big_Floppy = Drive_Data->Is_Big_Floppy;
  ReturnValue.LastErrorIOCTL = Drive_Data->LastErrorIOCTL;
  ReturnValue.LastError = Drive_Data->LastError;

  LOG_EVENT3("Returning the following values:","Drive Index", Drive_Data->DriveArrayIndex, "Drive Name", ReturnValue.Drive_Name, "Largest block of freespace", ReturnValue.Largest_Free_Block_Of_Sectors)

  LOG_EVENT3("Returning the following values:", "Total available sectors", ReturnValue.Total_Available_Sectors, "Corrupt Partition Table Flag", ReturnValue.Corrupt_Partition_Table, "I/O Error Flag", ReturnValue.IO_Error)

  LOG_EVENT2("Returning the following values:", "Unusable Flag", ReturnValue.Unusable, "Is Big Floppy Flag", ReturnValue.Is_Big_Floppy )

  API_EXIT( "Get_Drive_Status" )

  /* Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return ReturnValue;

}



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
void Set_Name ( ADDRESS      Handle,
                char         New_Name[],
                CARDINAL32 * Error_Code
              )
{

  ADDRESS                             Object;                  /* Used when translating Handle into something usable. */
  TAG                                 ObjectTag;               /* Used when translating Handle into something usabel. */
  Disk_Drive_Data *                   Drive_Data;
  Partition_Data *                    PartitionData;
  Volume_Data *                       Volume_Record;
  CARDINAL32                          Index;                   /* Used to traverse the DriveArray. */
  CARDINAL32                          New_Name_Size;           /* Used to hold the size of New_Name so that it doesn't have to be recalculated. */
  Duplicate_Check_Parameter_Record    Name_Check_Parameters;
  BOOLEAN                             Kill_Existing_Name;
  char *                              Temp = NULL;             /* Used to make a copy of New_Name. */
  Partition_Data_To_Update            New_Values;

  API_ENTRY( "Set_Name" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Set_Name" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed!", "Error code", *Error_Code)

    API_EXIT( "Set_Name" )

    return;

  }

  /* Get the size of New_Name.  We will need it later. */
  New_Name_Size = strlen(New_Name);

  /* If the new name is a null string, then we will kill the existing name.  We need to check this here so that we can differentiate
     between New_Name being a null string and New_Name being all spaces.                                                              */
  if ( New_Name_Size == 0 )
  {

    Kill_Existing_Name = TRUE;

    LOG_EVENT("The new name size is 0!  As a result, we will kill the existing name!")

  }
  else
  {

    Kill_Existing_Name = FALSE;

  }

  /* If the user did not specify a null string for New_Name, we must strip away any leading or trailing spaces in New_Name. */
  if ( ! Kill_Existing_Name )
  {

    /* D201136 - The LVM VIO interface can't handle having the engine remove trailing spaces from the
                 strings it passes in, so the engine was modified to copy the Name string passed in and
                 operate on the copy.

       BEGIN                                                                                          BMR */

    /* Make a copy of New_Name. */
    Temp = (char *) malloc( New_Name_Size + 1 );

    /* Did we get the memory? */
    if ( Temp == NULL )
    {

      LOG_ERROR("Could not allocate a temporary buffer for the name!  Out of Memory!")

      API_EXIT( "Set_Name" )

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      return;

    }

    /* Now copy New_Name into the memory allocated for the copy of New_Name. */
    strcpy(Temp, New_Name);

    /* We only want to work with the copy, so set New_Name to point to the copy of New_Name. */
    New_Name = Temp;

    /* D201136 - The LVM VIO interface can't handle having the engine remove trailing spaces from the
                 strings it passes in, so the engine was modified to copy the Name string passed in and
                 operate on the copy.

       END                                                                                            BMR */

    /* Eliminate leading and trailing spaces from New_Name. */
    New_Name = Adjust_Name(New_Name);

    /* What is the size of New_Name now? */
    New_Name_Size = strlen(New_Name);

    /* A zero length name is not acceptable. */
    if ( New_Name_Size == 0 )
    {

      LOG_ERROR("The length of the user specified name is 0 after removing leading and trailing spaces!")

      API_EXIT( "Set_Name" )

      *Error_Code = LVM_ENGINE_BAD_NAME;
      free(Temp);

      return;

    }

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case DISK_DRIVE_DATA_TAG : /* We have Disk_Drive_Data here. */
                               Drive_Data = ( Disk_Drive_Data * ) Object;

                               /* If the drive is a PRM in big floppy mode, then we can not change its name. */
                               if ( Drive_Data->Is_Big_Floppy )
                               {

                                 LOG_ERROR("This operation can not be performed on Big Floppy formatted media!")

                                 API_EXIT( "Set_Name" )

                                 /* We can not perform this operation on this drive. */
                                 *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                                 return;

                               }

                               /* Does this drive have a serial number? */
                               if ( Drive_Data->Drive_Serial_Number == 0 )
                               {

                                 /* This drive did not have a serial number.  Give it one. */
                                 Drive_Data->Drive_Serial_Number = Create_Serial_Number();

                               }

                               /* If the New_Name is the null string, kill the existing name and return. */
                               if ( Kill_Existing_Name )
                               {

                                 /* Design Change!  Everything must have a unique name, so killing an existing name is no longer allowed! */
                                 LOG_ERROR("Error: 0 length names are not allowed.  Everything must have a unique name!")

                                 API_EXIT( "Set_Name" )

                                 /* The new name is too small!  Indicate the error. */
                                 *Error_Code = LVM_ENGINE_BAD_NAME;

                                 /* If we allocated memory for a copy of New_Name, we must free it. */
                                 if ( Temp != NULL )
                                   free(Temp);

                                 return;

                               }

                               /* Is the new name too long?  */
                               if ( New_Name_Size > DISK_NAME_SIZE )
                               {

                                 LOG_ERROR("Error: The new name is too big!")

                                 API_EXIT( "Set_Name" )

                                 /* The new name is too big!  Indicate the error. */
                                 *Error_Code = LVM_ENGINE_NAME_TOO_BIG;

                                 /* If we allocated memory for a copy of New_Name, we must free it. */
                                 if ( Temp != NULL )
                                   free(Temp);

                                 return;

                               }

                               /* Is the drive name unique? */

                               /* Traverse the drive array comparing names.  If a duplicate is found, abort. */
                               for ( Index = 0; Index < DriveCount; Index++ )
                               {

                                 /* We don't want to look at the drive whose name we are changing.  */
                                 if ( Index != Drive_Data->DriveArrayIndex )
                                 {

                                   /* Compare the names.  If a match is found, return an error to the caller. */
                                   if ( strncmp(DriveArray[Index].Drive_Name, New_Name, DISK_NAME_SIZE) == 0 )
                                   {

                                     LOG_ERROR("Error: The new name is not unique!")

                                     API_EXIT( "Set_Name" )

                                     /* We have a duplicate name.  Indicate the error and return. */
                                     *Error_Code = LVM_ENGINE_DUPLICATE_NAME;

                                     /* If we allocated memory for a copy of New_Name, we must free it. */
                                     if ( Temp != NULL )
                                       free(Temp);

                                     return;

                                   }

                                 }

                               }

                               /* Set the name. */
                               strncpy(Drive_Data->Drive_Name, New_Name, DISK_NAME_SIZE);

                               /* Since the disk drive name is stored in the LVM Signature Sector, we must update any partitions
                                  on this disk drive which have LVM Signature Sectors.                                            */
                               ForEachItem(DriveArray[Drive_Data->DriveArrayIndex].Partitions,&Update_Disk_Names_In_Signature_Sectors,NULL,TRUE,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                               assert(*Error_Code == DLIST_SUCCESS);

#else

                               if ( *Error_Code != DLIST_SUCCESS )
                               {

                                 LOG_ERROR("Error: ForEachItem failed to update the disk name in all of the LVM Signature Sectors!")

                                 API_EXIT( "Set_Name" )

                                 *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                 /* If we allocated memory for a copy of New_Name, we must free it. */
                                 if ( Temp != NULL )
                                   free(Temp);

                                 return;

                               }

#endif

#endif

                               /* Mark the drive as having been changed. */
                               Drive_Data->ChangesMade = TRUE;

                               /*  D201295 If Set_Name is used to give a drive a name and the drive does not have an MBR,
                                   the name was not being saved.  Set_Name will now create an MBR if necessary.

                                   BEGIN                                                                                      BMR */

                               /* If this drive has never been partitioned, then we must create an MBR.  If we do not, then there
                                  will be no DLA Table to store the Drive Name in.                                                 */

                               /* If there is only 1 item in the Partitions list for this drive, then this drive has never been partitioned. */
                               if ( GetListSize(Drive_Data->Partitions, Error_Code ) <= 1 )
                               {

                                 /* Did we get here because GetListSize failed? */
#ifdef DEBUG

#ifdef PARANOID

                                 assert(*Error_Code == DLIST_SUCCESS);

#else

                                 if ( *Error_Code != DLIST_SUCCESS )
                                 {

                                   LOG_ERROR1("GetListSize failed!", "Error code", *Error_Code)

                                   API_EXIT( "Set_Name" )

                                   *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                   /* If we allocated memory for a copy of New_Name, we must free it. */
                                   if ( Temp != NULL )
                                     free(Temp);

                                   return;

                                 }

#endif

#endif

                                 /* We need to create a new MBR for this drive.  Call the New_MBR function! */
                                 New_MBR( Drive_Data->External_Handle, Error_Code);

                                 if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                                 {

                                   LOG_ERROR1("New_MBR failed.", "Error code", *Error_Code)

                                 }


                               }

                               /*  D201295 If Set_Name is used to give a drive a name and the drive does not have an MBR,
                                   the name was not being saved.  Set_Name will now create an MBR if necessary.

                                   END                                                                                        BMR */


                               break;

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */
                              PartitionData = ( Partition_Data * ) Object;

                              /* Is this Partition_Data record for the Boot Manager partition or for Free Space?  */
                              if ( ( PartitionData->Partition_Table_Entry.Format_Indicator == BOOT_MANAGER_INDICATOR ) ||
                                   ( PartitionData->Partition_Type == FreeSpace )
                                 )
                              {

                                LOG_ERROR("Can't change the name of Boot Manager or a block of free space!")

                                API_EXIT( "Set_Name" )

                                /* Changing the name of Boot Manager or of a block of Free Space is not allowed. */
                                *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                                /* If we allocated memory for a copy of New_Name, we must free it. */
                                if ( Temp != NULL )
                                  free(Temp);

                                return;

                              }

                              /* Does this drive have a serial number? */
                              if ( DriveArray[PartitionData->Drive_Index].Drive_Serial_Number == 0 )
                              {

                                /* This drive did not have a serial number.  Give it one. */
                                DriveArray[PartitionData->Drive_Index].Drive_Serial_Number = Create_Serial_Number();

                              }

                              /* Does this partition have a serial number? */
                              if ( PartitionData->DLA_Table_Entry.Partition_Serial_Number == 0 )
                              {

                                /* This partition has no serial number!  Give it one. */
                                PartitionData->DLA_Table_Entry.Partition_Serial_Number = Create_Serial_Number();

                              }

                              /* If the size of New_Name is zero, kill the exiting name for the partition and return. */
                              if ( Kill_Existing_Name )
                              {

                                /* Design change:  Everything must now have a unique name.  Therefore, names can no longer be killed! */
                                LOG_ERROR("0 length names are not allowed!")

                                API_EXIT( "Set_Name" )

                                /* The new name is too small!  Indicate the error. */
                                *Error_Code = LVM_ENGINE_BAD_NAME;

                                /* If we allocated memory for a copy of New_Name, we must free it. */
                                if ( Temp != NULL )
                                  free(Temp);

                                return;

                              }

                              /* Is the new name too long? */
                              if ( New_Name_Size > PARTITION_NAME_SIZE )
                              {

                                LOG_ERROR("Error: The new name is too long!")

                                API_EXIT( "Set_Name" )

                                /* The new name is too big!  Indicate the error. */
                                *Error_Code = LVM_ENGINE_NAME_TOO_BIG;

                                /* If we allocated memory for a copy of New_Name, we must free it. */
                                if ( Temp != NULL )
                                  free(Temp);

                                return;

                              }

                              /* Prepare to check for duplicate names on partitions located on the same drive. */
                              Name_Check_Parameters.Duplicate_Name_Found = FALSE;
                              Name_Check_Parameters.New_Name = New_Name;
                              Name_Check_Parameters.New_Name_Length = New_Name_Size;
                              Name_Check_Parameters.Handle = PartitionData->External_Handle;
                              Name_Check_Parameters.Check_Name = TRUE;                       /* Check the name, not the serial number. */
                              Name_Check_Parameters.Max_Name_Length = PARTITION_NAME_SIZE;

                              /* Check for duplicate names. */
                              ForEachItem(DriveArray[PartitionData->Drive_Index].Partitions,&Duplicate_Check,&Name_Check_Parameters,TRUE,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                              assert( *Error_Code == DLIST_SUCCESS );

#else

                              if ( *Error_Code != DLIST_SUCCESS )
                              {

                                LOG_ERROR1("ForEachItem failed!", "Error code", *Error_Code)

                                API_EXIT( "Set_Name" )

                                *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                /* If we allocated memory for a copy of New_Name, we must free it. */
                                if ( Temp != NULL )
                                  free(Temp);

                                return;

                              }

#endif

#endif

                              /* Were there any duplicate names? */
                              if ( Name_Check_Parameters.Duplicate_Name_Found )
                              {

                                LOG_ERROR("The new name is not unique!")

                                API_EXIT( "Set_Name" )

                                /* We have a duplicate name.  Indicate the error and return. */
                                *Error_Code = LVM_ENGINE_DUPLICATE_NAME;

                                /* If we allocated memory for a copy of New_Name, we must free it. */
                                if ( Temp != NULL )
                                  free(Temp);

                                return;

                              }

                              /* Set the name. */
                              strncpy(PartitionData->Partition_Name, New_Name, PARTITION_NAME_SIZE);
                              strncpy(PartitionData->DLA_Table_Entry.Partition_Name,PartitionData->Partition_Name, PARTITION_NAME_SIZE);

                              /* Does this partition have an LVM_Signature_Sector associated with it? */
                              if ( PartitionData->Signature_Sector != NULL )
                              {

                                /* Update the partition name in the LVM Signature Sector. */
                                strncpy(PartitionData->Signature_Sector->Partition_Name,PartitionData->Partition_Name, PARTITION_NAME_SIZE);

                              }

                              /* Indicate that changes were made to a drive. */
                              DriveArray[PartitionData->Drive_Index].ChangesMade = TRUE;

                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */
                           Volume_Record = ( Volume_Data * ) Object;

                           /* If the volume can not be changed, abort. */
                           if ( ! Volume_Record->Can_Be_Altered )
                           {

                             LOG_ERROR("The specified volume can not be altered!")

                             API_EXIT( "Set_Name" )

                             /* We can not perform this operation on this volume. */
                             *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                             return;

                           }

                           /* If the size of the new name is 0, kill the existing name and return. */
                           if ( Kill_Existing_Name )
                           {

                             /* Design Change:  Everything must have a name, and the name must be unique!  Killing of names is no longer allowed. */

/*                             memset(Volume_Record->Volume_Name,0, VOLUME_NAME_SIZE);                */

                             LOG_ERROR("0 length names are not allowed!")

                             API_EXIT( "Set_Name" )

                             /* The new name is too small!  Indicate the error. */
                             *Error_Code = LVM_ENGINE_BAD_NAME;

                             /* If we allocated memory for a copy of New_Name, we must free it. */
                             if ( Temp != NULL )
                               free(Temp);

                             return;

                           }
                           else
                           {

                             /* Is the new name too long?  Remember that the NULL terminator must fit within the allotted space also. */
                             if ( strlen(New_Name) > VOLUME_NAME_SIZE )
                             {

                               LOG_ERROR("The new name is too long!")

                               API_EXIT( "Set_Name" )

                               /* The new name is too big!  Indicate the error. */
                               *Error_Code = LVM_ENGINE_NAME_TOO_BIG;

                               /* If we allocated memory for a copy of New_Name, we must free it. */
                               if ( Temp != NULL )
                                 free(Temp);

                               return;

                             }

                             /* Prepare to check for duplicate volume names. */
                             Name_Check_Parameters.Duplicate_Name_Found = FALSE;
                             Name_Check_Parameters.New_Name = New_Name;
                             Name_Check_Parameters.New_Name_Length = New_Name_Size;
                             Name_Check_Parameters.Handle = Volume_Record->External_Handle;
                             Name_Check_Parameters.Check_Name = TRUE;
                             Name_Check_Parameters.Max_Name_Length = VOLUME_NAME_SIZE;

                             /* Check for duplicate names. */
                             ForEachItem(Volumes,&Duplicate_Check,&Name_Check_Parameters,TRUE,Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                             assert( *Error_Code == DLIST_SUCCESS );

#else

                             if ( *Error_Code != DLIST_SUCCESS )
                             {

                               LOG_ERROR1("ForEachItem failed.", "Error code", *Error_Code)

                               API_EXIT( "Set_Name" )

                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                               /* If we allocated memory for a copy of New_Name, we must free it. */
                               if ( Temp != NULL )
                                 free(Temp);

                               return;

                             }

#endif

#endif

                             /* Were there any duplicate names? */
                             if ( Name_Check_Parameters.Duplicate_Name_Found )
                             {

                               LOG_ERROR("The new name is not unique!")

                               API_EXIT( "Set_Name" )

                               /* We have a duplicate name.  Indicate the error and return. */
                               *Error_Code = LVM_ENGINE_DUPLICATE_NAME;

                               /* If we allocated memory for a copy of New_Name, we must free it. */
                               if ( Temp != NULL )
                                 free(Temp);

                               return;

                             }

                             /* Set the name. */
                             strncpy(Volume_Record->Volume_Name, New_Name, VOLUME_NAME_SIZE);

                             /* Set up to update the Volume Name for all of the partitions associated with this volume. */
                             New_Values.Update_Drive_Letter = FALSE;
                             New_Values.New_Drive_Letter = 'B';
                             New_Values.Update_Volume_Name = TRUE;
                             New_Values.New_Name = New_Name;
                             New_Values.Update_Boot_Manager_Status = FALSE;
                             New_Values.On_Menu = FALSE;
                             New_Values.Update_Volume_Spanning = FALSE;
                             New_Values.Spanned_Volume = FALSE;

                             /* Set the name in the DLA Tables of the partitions that comprise this volume. */
                             Update_Partitions_Volume_Data( Volume_Record->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &New_Values, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                             assert( *Error_Code == LVM_ENGINE_NO_ERROR );

#else

                             if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                             {

                               LOG_ERROR1("Update_Partitions_Volume_Data failed.", "Error code", *Error_Code)

                               API_EXIT( "Set_Name" )

                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                               /* If we allocated memory for a copy of New_Name, we must free it. */
                               if ( Temp != NULL )
                                 free(Temp);

                               return;

                             }

#endif

#endif

                           }

                           /* If this is a "fake" volume, then turn it into a real volume.  Only Compatibility Volumes can be "fake" volumes. */
                           if ( Volume_Record->Compatibility_Volume && DriveArray[Volume_Record->Partition->Drive_Index].Fake_Volumes_In_Use )
                           {

                             Convert_Fake_Volumes_On_PRM_To_Real_Volumes( Volume_Record->Partition->Drive_Index, Error_Code);

                             if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                             {

                               LOG_ERROR1("Convert_Fake_Volumes_On_PRM_To_Real_Volumes failed.","Error code", *Error_Code)

                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                               /* If we allocated memory for a copy of New_Name, we must free it. */
                               if ( Temp != NULL )
                                 free(Temp);

                               API_EXIT( "Set_Name" )

                               return;

                             }

                           }

                           /* Indicate that changes were made. */
                           Volume_Record->ChangesMade = TRUE;

                           break;

    default : /* If it is not one of the above, then it is an internal error! */

#ifdef DEBUG

#ifdef PARANOID

              assert(0);

#else

              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

#endif

#endif

              LOG_ERROR1("Unrecognized ObjectTag returned by Translate_Handle!","Object Tag", ObjectTag)

              API_EXIT( "Set_Name" )

              /* If we allocated memory for a copy of New_Name, we must free it. */
              if ( Temp != NULL )
                free(Temp);

              return;

              /* Keep the compiler happy. */
              break;
  }

  /* If we allocated memory for a copy of New_Name, we must free it. */
  if ( Temp != NULL )
    free(Temp);

  API_EXIT( "Set_Name" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
void Set_Startable ( ADDRESS      Handle,
                     CARDINAL32 * Error_Code
                   )
{

  ADDRESS                             Object;                  /* Used when translating Handle into something usable. */
  TAG                                 ObjectTag;               /* Used when translating Handle into something usabel. */
  Partition_Data *                    PartitionData;
  Volume_Data *                       Volume_Record;
  CARDINAL32                          Index;                   /* Used to traverse the DriveArray. */

  API_ENTRY( "Set_Startable" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Set_Startable" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed.", "Error code", *Error_Code)

    API_EXIT( "Set_Startable" )

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */
                              PartitionData = ( Partition_Data * ) Object;

                              /* Is this partition part of a volume? */
                              if ( PartitionData->Volume_Handle != NULL )
                              {

                                LOG_ERROR("The specified partition is part of a volume!")

                                API_EXIT( "Set_Startable" )

                                *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;
                                return;

                              }

                              /* Is this partition eligible to be set startable?  In order to be set startable,
                                 it must be a primary partition on the first drive.                               */
                              if ( ( ! PartitionData->Primary_Partition ) || ( PartitionData->Drive_Index != 0 ) )
                              {

                                LOG_ERROR("The specified partition is not a primary partition on the first drive.")

                                API_EXIT( "Set_Startable" )

                                *Error_Code = LVM_ENGINE_SPECIFIED_PARTITION_NOT_STARTABLE;
                                return;

                              }

                              /* We must clear the Active Flag for all primary partitions on the first drive. */
                              ForEachItem(DriveArray[0].Partitions,&Clear_Startable_Flags, NULL, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                             assert(*Error_Code == DLIST_SUCCESS);

#else

                             if ( *Error_Code != DLIST_SUCCESS )
                             {

                               LOG_ERROR1("ForEachItem failed!","Error code", *Error_Code)

                               API_EXIT( "Set_Startable" )

                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                               return;

                             }

#endif

#endif

                              /* Now set the Active Flag for this partition. */
                              PartitionData->Partition_Table_Entry.Boot_Indicator |= 0x80;

                              /* Make sure that the Boot Manager "Hidden" bit is off. */
                              if ( PartitionData->Partition_Table_Entry.Format_Indicator < 0x18 )
                                PartitionData->Partition_Table_Entry.Format_Indicator = PartitionData->Partition_Table_Entry.Format_Indicator & (~BOOT_MANAGER_HIDDEN_PARTITION_FLAG);

                              /* Is this the Boot Manager Partition? */
                              if ( Boot_Manager_Handle == Handle)
                                Boot_Manager_Active = TRUE;
                              else
                                Boot_Manager_Active = FALSE;

                              /* Did marking this partition startable change the Boot Drive Serial Number? */
                              if ( Boot_Drive_Serial_Number != DriveArray[0].Boot_Drive_Serial_Number )
                              {

                                /* Does the drive have a serial number? */
                                if ( DriveArray[0].Drive_Serial_Number == 0 )
                                  DriveArray[0].Drive_Serial_Number = Create_Serial_Number();

                                /* Update the Boot_Drive_Serial_Numbers for drives using the previous one. */
                                for ( Index = 0; Index < DriveCount; Index++ )
                                {

                                  if ( DriveArray[Index].Boot_Drive_Serial_Number == Boot_Drive_Serial_Number )
                                  {
                                     DriveArray[Index].Boot_Drive_Serial_Number = DriveArray[0].Drive_Serial_Number;
                                     DriveArray[Index].ChangesMade = TRUE;
                                  }

                                }

                                /* Save the new Boot Drive Serial Number. */
                                Boot_Drive_Serial_Number = DriveArray[0].Drive_Serial_Number;

                              }

                              /* Indicate that changes were made to a drive. */
                              DriveArray[0].ChangesMade = TRUE;

                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */
                           Volume_Record = ( Volume_Data * ) Object;

                           /* If the volume can not be changed, abort. */
                           if ( ! Volume_Record->Can_Be_Altered )
                           {

                             LOG_ERROR("The specified volume can not be altered!")

                             API_EXIT( "Set_Startable" )

                             /* We can not perform this operation on this volume. */
                             *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                             return;

                           }

                           /* Is this an LVM volume? */
                           if ( ! Volume_Record->Compatibility_Volume)
                           {

                             LOG_ERROR("The specified volume is NOT a compatibility volume!")

                             API_EXIT( "Set_Startable" )

                             /* LVM volumes are not bootable or startable. */
                             *Error_Code = LVM_ENGINE_SELECTED_VOLUME_NOT_STARTABLE;
                             return;

                           }

                           /* Since this is a compatibility volume, we must examine its corresponding partition
                              to see if this volume can be set startable.                                        */

                           /* Get the partition that corresponds to this volume. */
                           PartitionData = Volume_Record->Partition;

#ifdef DEBUG

#ifdef PARANOID

                           assert( PartitionData != NULL );

#else

                           if ( PartitionData == NULL )
                           {

                             LOG_ERROR("The partition pointer for the specified volume is NULL!")

                             API_EXIT( "Set_Startable" )

                             *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
                             return;

                           }
#endif

#endif

                           /* Now lets see if this partition can be set startable. */

                           /* In order to be set startable, it must be a primary partition on the first drive.   */
                           if ( ( ! PartitionData->Primary_Partition ) || ( PartitionData->Drive_Index != 0 ) )
                           {

                             LOG_ERROR("The partition corresponding to the volume is not a primary partition on the first drive.")

                             API_EXIT( "Set_Startable" )

                             *Error_Code = LVM_ENGINE_SELECTED_VOLUME_NOT_STARTABLE;
                             return;

                           }

                           /* We must clear the Active Flag for all primary partitions on the first drive. */
                           ForEachItem(DriveArray[0].Partitions,&Clear_Startable_Flags, NULL, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                           assert(*Error_Code == DLIST_SUCCESS);

#else

                           if ( *Error_Code != DLIST_SUCCESS )
                           {

                             LOG_ERROR1("ForEachItem failed!","Error code", *Error_Code)

                             API_EXIT( "Set_Startable" )

                             *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                             return;

                           }

#endif

#endif

                             /* Now set the Active Flag for this partition. */
                           PartitionData->Partition_Table_Entry.Boot_Indicator |= 0x80;

                           /* Is this the Boot Manager Partition? */
                           if ( Boot_Manager_Handle == Handle)
                             Boot_Manager_Active = TRUE;
                           else
                             Boot_Manager_Active = FALSE;

                           /* Indicate that changes were made to a drive. */
                           DriveArray[0].ChangesMade = TRUE;

                           /* If this was a "fake" volume, then make it a real volume. */
                           if ( DriveArray[0].Is_PRM && DriveArray[0].Fake_Volumes_In_Use )
                           {

                             Convert_Fake_Volumes_On_PRM_To_Real_Volumes( 0, Error_Code);
                             if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                             {

                               LOG_ERROR1("Convert_Fake_Volumes_On_PRM_To_Real_Volumes failed!", "Error code", *Error_Code)

                               API_EXIT( "Set_Startable" )

                               return;

                             }

                           }

                           /* Did marking this partition startable change the Boot Drive Serial Number? */
                           if ( Boot_Drive_Serial_Number != DriveArray[0].Boot_Drive_Serial_Number )
                           {

                             /* Does the drive have a serial number? */
                             if ( DriveArray[0].Drive_Serial_Number == 0 )
                               DriveArray[0].Drive_Serial_Number = Create_Serial_Number();

                             /* Update the Boot_Drive_Serial_Numbers for drives using the previous one. */
                             for ( Index = 0; Index < DriveCount; Index++ )
                             {

                               if ( DriveArray[Index].Boot_Drive_Serial_Number == Boot_Drive_Serial_Number )
                               {
                                  DriveArray[Index].Boot_Drive_Serial_Number = DriveArray[0].Drive_Serial_Number;
                                  DriveArray[Index].ChangesMade = TRUE;
                               }

                             }

                             /* Save the new Boot Drive Serial Number. */
                             Boot_Drive_Serial_Number = DriveArray[0].Drive_Serial_Number;

                           }

                           /* Is there another volume marked installable?  Is the volume marked installable the same as the volume
                              being marked startable?  If not, then we have a volume marked installable which will not be startable.
                              Furthermore, since Boot Manager is not a volume, the volume being marked startable can not be Boot Manager,
                              which means that the volume marked installable will not even be bootable.  Thus, we
                              must remove the installable flag from the volume marked installable as it can nolonger be
                              installed to successfully.                                                                      */
                           if ( ( Install_Volume_Handle != NULL ) && ( Install_Volume_Handle != Volume_Record->Volume_Handle ) )
                           {

                             /* Get the old volume's data. */
                             Volume_Record = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Install_Volume_Handle, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                             assert(*Error_Code == DLIST_SUCCESS);

#else

                             if ( *Error_Code != DLIST_SUCCESS )
                             {

                               LOG_ERROR1("GetObject failed!", "Error code", *Error_Code)

                               API_EXIT( "Set_Startable" )

                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                               return;

                             }

#endif

#endif

#ifdef DEBUG

#ifdef PARANOID

                             assert( Volume_Record->Compatibility_Volume );
                             assert( Volume_Record->Partition_Count == 1 );

#else

                             if ( ( ! Volume_Record->Compatibility_Volume ) ||
                                  ( Volume_Record->Partition_Count != 1)
                                )
                             {

                               LOG_ERROR("The volume already marked installable is not a compatibility volume!")

                               API_EXIT( "Set_Startable" )

                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                               return;

                             }

#endif

#endif

                             /* The old volume must be a compatibility volume.  Lets get its partition data and examine it. */
                             PartitionData = Volume_Record->Partition;

                             /* Now turn off the install flag in the partition's DLA Table Entry. */
                             PartitionData->DLA_Table_Entry.Installable = FALSE;

                             /* Mark the drive that the partition is on as having been changed. */
                             DriveArray[PartitionData->Drive_Index].ChangesMade = TRUE;

                           }

                           break;

    default :
              /* We have a bad handle! */
              *Error_Code = LVM_ENGINE_BAD_HANDLE;

              LOG_ERROR1("Unexpected ObjectTag returned from Translate_Handle!", "Object Tag", ObjectTag)

              API_EXIT( "Set_Startable" )

              return;

              /* Keep the compiler happy. */
              break;

  }

  API_EXIT( "Set_Startable" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;


}


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
CARDINAL32 Get_Valid_Options( ADDRESS Handle, CARDINAL32 * Error_Code )
{

  ADDRESS                             Object;          /* Used when translating Handle into something usable. */
  TAG                                 ObjectTag;       /* Used when translating Handle into something usabel. */
  Disk_Drive_Data *                   Drive_Data;
  Partition_Data *                    PartitionData;
  Volume_Data *                       Volume_Record;
  CARDINAL32                          ReturnValue = 0; /* Used to hold the bitmap which will be returned to the caller. */

  API_ENTRY( "Get_Valid_Options" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Valid_Options" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return ReturnValue;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed!", "Error code", *Error_Code)

    API_EXIT( "Get_Valid_Options" )

    return ReturnValue;

  }

  /* Assume that we will succeed. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case DISK_DRIVE_DATA_TAG : /* We have Disk_Drive_Data here. */
                               Drive_Data = ( Disk_Drive_Data * ) Object;

                               /* We can always set the name of a drive. */
                               ReturnValue += CAN_SET_NAME;

                               break;

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */
                              PartitionData = ( Partition_Data * ) Object;

                              /* Find out what we can do with this partition. */
                              ReturnValue = Get_Partition_Options( PartitionData, Error_Code );

                              if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                              {

                                LOG_ERROR1("Get_Partition_Options failed!", "Error code", *Error_Code)

                              }
  LOG_EVENT1("Get_Valid_Options:","Get_Partition_Options", ReturnValue);

                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */
                           Volume_Record = ( Volume_Data * ) Object;

                           /* Find out what we can do with this volume. */
                           ReturnValue = Get_Volume_Options( Volume_Record, Error_Code );

                           if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                           {

                             LOG_ERROR1("Get_Volume_Options failed!","Error code", *Error_Code)

                           }

                           break;

    default : /* If it is not one of the above, then it is an internal error! */

#ifdef DEBUG

#ifdef PARANOID

              assert(0);

#else

              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

#endif

#endif

              LOG_ERROR1("Unexpected Object Tag was returned by Translate_Handle!", "Object tag", ObjectTag)

              API_EXIT( "Get_Valid_Options" )

              return ReturnValue;

              /* Keep the compiler happy. */
              break;

  }

  /* Now lets add the main Boot Manager options.  */

  /* If Boot Manager is already installed, then we can remove Boot Manager from the system. */
  if ( Boot_Manager_Handle != NULL )
  {

    ReturnValue += REMOVE_BOOT_MANAGER;
    ReturnValue += SET_BOOT_MANAGER_DEFAULTS;

    /* If Boot Manager is NOT active, then we can make Boot Manager Active. */
    if ( ! Boot_Manager_Active )
      ReturnValue += SET_BOOT_MANAGER_STARTABLE;

  }
  else
  {

    /* Since Boot Manager is not installed, see if it can be installed. */
    if ( Can_Boot_Manager_Be_Installed( Error_Code) )
    {

      /* Boot Manager can be installed! */
      ReturnValue += INSTALL_BOOT_MANAGER;

    }

    /* If there was an error returned by the Can_Boot_Manager_Be_Installed function, it will be propagated back to the caller below. */

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      LOG_ERROR1("Can_Boot_Manager_Be_Installed failed.","Error code", *Error_Code)

    }

  }

  LOG_EVENT1("Returning the option bitmap to caller.","Option bitmap", ReturnValue)

  API_EXIT( "Get_Valid_Options" )

  /* Return to caller.  *Error_Code has already been set. */
  return ReturnValue;

}


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
BOOLEAN Reboot_Required ( void )
{

  API_ENTRY( "Reboot_Required" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Reboot_Required" )

    /* The Engine has not been opened yet!  Return FALSE as we have not yet been able to do anything which would require a reboot. */

    return FALSE;

  }

  if ( RebootRequired )
  {

    LOG_EVENT("A reboot is required.")

  }
  else
  {

    LOG_EVENT("A reboot is NOT required.")

  }

  API_EXIT( "Reboot_Required" )

  return RebootRequired;

}


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
BOOLEAN _System Changes_Pending ( void )
{

  CARDINAL32    Index;                 /* Used to walk the drive array. */
  BOOLEAN       Changes_Found = FALSE; /* Used when searching the Volumes list. */
  CARDINAL32    Error_Code;

  API_ENTRY( "Changes_Pending" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Changes_Pending" )

    /* The Engine has not been opened yet!  Return FALSE as we have not yet been able to change anything!  */

    return FALSE;

  }

  /* Examine the drive array to see if any changes are pending there. */
  for ( Index = 0; Index < DriveCount; Index++)
  {

    /* Does the current drive have any changes logged against it? */
    if ( DriveArray[Index].ChangesMade )
    {

      LOG_EVENT("Changes are pending!")

      API_EXIT( "Changes_Pending" )

      return TRUE;

    }

  }

  /* Do any volumes have changes pending? */
  ForEachItem(Volumes,&Check_For_Volume_Changes, &Changes_Found, TRUE, &Error_Code);

  /* There should never be an error here!  If there is, then we don't want the caller to
     perform a Commit operation, so tell the caller that there are no pending changes.    */
  if ( Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("When checking for Volume Changes, ForEachItem failed!", "Error code", Error_Code)

    API_EXIT( "Changes_Pending" )

    return FALSE;

  }

  if ( Changes_Found )
  {

    LOG_EVENT("There are changes pending!")

  }
  else
  {

    LOG_EVENT("There are NO changes pending.")

  }

  API_EXIT( "Changes_Pending" )

  /* Return the result of our search for pending changes. */
  return Changes_Found;

}


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
void Set_Min_Install_Size ( CARDINAL32  Min_Sectors )
{

  API_ENTRY( "Set_Min_Install_Size" )

  /* Save Min_Sectors in the global variable Min_Install_Size. */
  Min_Install_Size = Min_Sectors;

  LOG_EVENT1("The Minimum Install Size has been changed.","The new Minimum Install Size",Min_Sectors)

  API_EXIT( "Set_Min_Install_Size" )

  return;

}


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
void Set_Free_Space_Threshold ( CARDINAL32  Min_Sectors )
{

  API_ENTRY( "Set_Free_Space_Threshold" )

  Min_Free_Space_Size = Min_Sectors;

  LOG_EVENT1("The Free Space Threshold has been changed.","The new Free Space Threshold", Min_Free_Space_Size)

  API_EXIT( "Set_Free_Space_Threshold" )

  return;

}


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
Partition_Information_Array Get_Partitions( ADDRESS Handle, CARDINAL32 * Error_Code )
{

  ADDRESS                       Object;
  TAG                           ObjectTag;
  Disk_Drive_Data *             Drive_Data;
  Volume_Data *                 Volume_Record;
  Partition_Information_Array   ReturnValue;


  API_ENTRY( "Get_Partitions" )

  /* Initialize ReturnValue assuming a failure. */
  ReturnValue.Count = 0;
  ReturnValue.Partition_Array = NULL;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Partitions" )

    return ReturnValue;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed!","Error code", *Error_Code)

    API_EXIT( "Get_Partitions" )

    return ReturnValue;

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case DISK_DRIVE_DATA_TAG : /* We have Disk_Drive_Data here. */
                               Drive_Data = ( Disk_Drive_Data * ) Object;

                               /* Lets get the list of partitions for this drive. */
                               ReturnValue = Get_Partitions_On_Drive(Drive_Data->DriveArrayIndex, Error_Code);

                               if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                               {

                                 LOG_ERROR1("Get_Partitions_On_Drive failed!","Error code", *Error_Code)

                               }

                               break;

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */

                              /* There is no list of partitions associated with a partition!  Return an error. */
                              *Error_Code = LVM_ENGINE_BAD_HANDLE;

                              LOG_ERROR("The handle passed in was for a partition instead of a volume or disk drive!")

                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */
                           Volume_Record = ( Volume_Data * ) Object;

                           ReturnValue = Get_Partitions_On_Volume(Volume_Record, Error_Code);

                           if ( *Error_Code != LVM_ENGINE_NO_ERROR )
                           {

                             LOG_ERROR1("Get_Partitions_On_Volume failed!","Error code", *Error_Code)

                           }

                           break;

    default : /* If it is not one of the above, then it is an internal error! */

#ifdef DEBUG

#ifdef PARANOID

assert(0);

#else
              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

#endif

#endif

              LOG_ERROR1("LVM_ENGINE_INTERNAL_ERROR - Unknown ObjectTag!", "Object Tag", ObjectTag)

              /* Keep the compiler happy. */
              break;
  }

  /* All done. */

  LOG_EVENT1("Returning data for X partitions.","X",ReturnValue.Count)

  API_EXIT( "Get_Partitions" )

  return ReturnValue;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Create_Serial_Number                             */
/*                                                                   */
/*   Descriptive Name: Creates a 32 bit serial number.               */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The function return value is a 32 bit serial number.    */
/*                                                                   */
/*   Error Handling: None.  If an error occurs, that means that the  */
/*                   internal structures of the LVM Engine have been */
/*                   corrupted!                                      */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/

CARDINAL32 _System Create_Serial_Number(void)
{

  time_t                             Current_Time;
  CARDINAL32                         Index;           /* Used to access the drive array. */
  CARDINAL32                         Error;           /* Used to hold the error return code from DLIST functions. */
  Duplicate_Check_Parameter_Record   Dup_Check_Parms;

  FUNCTION_ENTRY( "Create_Serial_Number" )

  do
  {

    /* Assume that a duplicate serial number will not be produced. */
    Dup_Check_Parms.Duplicate_Serial_Number_Found = FALSE;

    /* Set the Check_Name flag so that the Duplicate_Check function will check for duplicate S/N instead of duplicate names. */
    Dup_Check_Parms.Check_Name = FALSE;

    /* Get the current time.  We will use it as the basis for our serial numbers. */
    Current_Time = time( NULL );

    /* Use the CRC function to create a 32 bit value. */
    Dup_Check_Parms.New_Serial_Number = CalculateCRC(INITIAL_CRC, &Current_Time, sizeof( time_t ) );

    /* Factor in the number of serial numbers already issued. */
    Dup_Check_Parms.New_Serial_Number = CalculateCRC( Dup_Check_Parms.New_Serial_Number, &Serial_Numbers_Issued, sizeof (Serial_Numbers_Issued) );

    /* Increment the count of serial numbers issued.  This is used to minimize the probability of producing duplicate serial numbers. */
    Serial_Numbers_Issued++;

    /* check for duplicate serial number in the system. */
    for ( Index = 0; Index < DriveCount; Index++ )
    {

      /* Does the current drive already have this serial number? */
      if ( DriveArray[Index].Drive_Serial_Number != Dup_Check_Parms.New_Serial_Number )
      {

        /* Do any of the partitions on this drive have this serial number? */
        ForEachItem(DriveArray[Index].Partitions, &Duplicate_Check, &Dup_Check_Parms, TRUE, &Error );

#ifdef DEBUG

        assert(Error == DLIST_SUCCESS);

#endif

        /* Are there any volumes? */
        if ( ( Volumes != NULL ) &&
             ( ! ListEmpty(Volumes,&Error) ) &&
             ( ! Dup_Check_Parms.Duplicate_Serial_Number_Found )
           )
        {

          /* Do any volumes in the system already have this serial number? */
          ForEachItem(Volumes,&Duplicate_Check, &Dup_Check_Parms, TRUE, &Error);

#ifdef DEBUG

          assert(Error == DLIST_SUCCESS);

#endif

        }

        /* Are there any Aggregates? */
        if ( ( Aggregates != NULL ) &&
             ( ! ListEmpty(Aggregates,&Error) ) &&
             ( ! Dup_Check_Parms.Duplicate_Serial_Number_Found )
           )
        {

          /* Do any aggregates in the system already have this serial number? */
          ForEachItem(Aggregates,&Duplicate_Check, &Dup_Check_Parms, TRUE, &Error);

#ifdef DEBUG

          assert(Error == DLIST_SUCCESS);

#endif

        }

      }
      else
      {

        /* We have a duplicate serial number. */
        Dup_Check_Parms.Duplicate_Serial_Number_Found = TRUE;

      }

    }

  } while ( Dup_Check_Parms.Duplicate_Serial_Number_Found || ( Dup_Check_Parms.New_Serial_Number == 0 ) );


  FUNCTION_EXIT( "Create_Serial_Number" )

  /* No duplicates!  Return Temp as the Serial Number. */
  return Dup_Check_Parms.New_Serial_Number;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Duplicate_Check                                  */
/*                                                                   */
/*   Descriptive Name: This function is intended for use with the    */
/*                     ForEachItem function in the DLIST module.  It */
/*                     checks for either a duplicate name in a DLIST */
/*                     of Volume_Data or Partition_Data, or for a    */
/*                     duplicate serial number in a DLIST of         */
/*                     Volume_Data or Partition_Data.                */
/*                                                                   */
/*   Input:  ADDRESS Object : The object to be examined.  This is    */
/*                            provided by the ForEachItem function.  */
/*           TAG ObjectTag : The TAG value associated with the item  */
/*                           whose address is in Object.  This is    */
/*                           provided by the ForEachItem function.   */
/*           CARDINAL32 ObjectSize : The size, in bytes, of the item */
/*                                   whose address is in Object.     */
/*                                   This is provided by the         */
/*                                   ForEachItem function.           */
/*           ADDRESS Parameters : This value is passed into          */
/*                                ForEachItem, which then passes it  */
/*                                to this function.  This should be  */
/*                                the address of a                   */
/*                                Duplicate_Check_Parameter_Record,  */
/*                                which tells this function what to  */
/*                                check for.                         */
/*           CARDINAL32 * Error : The address of a variable which    */
/*                                be used to hold the error return   */
/*                                code generated by this function.   */
/*                                                                   */
/*   Output:  The fields in the Duplicate_Check_Parameter_Record are */
/*            updated as appropriate.  If no errors occur, *Error    */
/*            will be set to DLIST_SUCCESS.  If an error occurs, then*/
/*            *Error will be set to a non-zero error code.           */
/*                                                                   */
/*   Error Handling: If an error is detected, an error code will be  */
/*                   returned.                                       */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Duplicate_Check( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast all the time. */
  Duplicate_Check_Parameter_Record  *  Dup_Check_Data;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *      PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access the Volume_Data without having to typecast it each time. */
  Volume_Data *         VolumeRecord = (Volume_Data *) Object;

//  FUNCTION_ENTRY("Duplicate_Check")

  /* Assume that we will succeed. */
  *Error = DLIST_SUCCESS;

  /* Establish access to our parameters. */
  Dup_Check_Data  = ( Duplicate_Check_Parameter_Record *) Parameters;


#ifdef DEBUG

#ifdef PARANOID

  /* Are we doing a Name check or a S/N check? */
  if ( Dup_Check_Data->Check_Name )
    assert( Dup_Check_Data->Max_Name_Length > 0 );

#else

  if ( Dup_Check_Data->Check_Name && ( Dup_Check_Data->Max_Name_Length == 0 ) )
  {

    LOG_EVENT("Duplicate_Check aborting.  Name check requested with a max. name length of 0!")

    FUNCTION_EXIT("Duplicate_Check")

    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* What kind of Object do we have here? */
  switch ( ObjectTag )
  {

    case PARTITION_DATA_TAG : /* We have a partition. */

                              /* Are we doing a Name check or a S/N check? */
                              if ( Dup_Check_Data->Check_Name )
                              {

                                /* Is this the partition whose name is being changed?  If so, skip it! */
                                if ( ( PartitionRecord->External_Handle != Dup_Check_Data->Handle ) &&
                                     ( &( PartitionRecord->Partition_Name[0] ) != &( Dup_Check_Data->New_Name[0] ) )
                                   )
                                {

                                  /* Lets see if the names match. */
                                  if ( strncmp(PartitionRecord->Partition_Name, Dup_Check_Data->New_Name, Dup_Check_Data->Max_Name_Length) == 0 )
                                  {

                                    /* The names match!  Set the Duplicate_Name_Found flag. */
                                    Dup_Check_Data->Duplicate_Name_Found = TRUE;

                                    /* Indicate that we found what we were looking for. */
                                    *Error = DLIST_SEARCH_COMPLETE;

                                  }

                                }

                              }
                              else
                              {

                                /* We are doing a S/N check. */
                                if ( Dup_Check_Data->New_Serial_Number == PartitionRecord->DLA_Table_Entry.Partition_Serial_Number )
                                {

                                  /* We have a duplicate Serial Number. */
                                  Dup_Check_Data->Duplicate_Serial_Number_Found = TRUE;

                                  /* Indicate that we found what we were looking for. */
                                  *Error = DLIST_SEARCH_COMPLETE;

                                }

                              }

                              break;

    case VOLUME_DATA_TAG : /* We have a volume. */

                           /* Are we doing a Name check or a S/N check? */
                           if ( Dup_Check_Data->Check_Name )
                           {

                             /* Is this the volume whose name is being changed?  If so, skip it! */
                             if ( ( &( VolumeRecord->Volume_Name[0] ) != &( Dup_Check_Data->New_Name[0] ) ) &&
                                  ( VolumeRecord->External_Handle != Dup_Check_Data->Handle )
                                )
                             {

                               /* Lets see if the names match. */
                               if ( strncmp(VolumeRecord->Volume_Name, Dup_Check_Data->New_Name, Dup_Check_Data->Max_Name_Length) == 0 )
                               {

                                 /* The names match!  Set the Duplicate_Name_Found flag. */
                                 Dup_Check_Data->Duplicate_Name_Found = TRUE;

                                 /* Indicate that we found what we were looking for and return. */
                                 *Error = DLIST_SEARCH_COMPLETE;

                               }

                             }

                           }
                           else
                           {

                             /* We are doing a S/N check. */
                             if ( Dup_Check_Data->New_Serial_Number == VolumeRecord->Volume_Serial_Number )
                             {

                               /* We have a duplicate Serial Number. */
                               Dup_Check_Data->Duplicate_Serial_Number_Found = TRUE;

                               /* Indicate that we found what we were looking for and return. */
                               *Error = DLIST_SEARCH_COMPLETE;

                             }

                           }

                           break;

    default: /* We have an error! */
             *Error = DLIST_CORRUPTED;

#ifdef DEBUG

#ifdef PARANOID

             assert(0);

#endif

#endif

             break;

  }

//  FUNCTION_EXIT("Duplicate_Check")

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
BOOLEAN _System Create_Unique_Name(  CARDINAL32  Name_Lists_To_Use, BOOLEAN Add_Brackets, char * BaseName, CARDINAL32 * Initial_Count, char * Buffer, CARDINAL32 BufferSize)
{

  Duplicate_Check_Parameter_Record   Dup_Check_Data;         /* Used with the Duplicate_Check function. */
  CARDINAL32                         BaseNameSize;           /* Used when determining if a unique name can fit in Buffer. */
  CARDINAL32                         Index;                  /* Used when walking the DriveArray. */
  CARDINAL32                         CountSize;              /* Used to track the number of digits in the number being added to BaseName. */
  CARDINAL32                         Bracket_Adjustment = 0; /* Used to adjust our name size calculation for the inclusion of brackets. */
  CARDINAL32                         Error;                  /* Used with the ForEachItem function. */
  char                               CountBuffer[20];        /* The buffer used to convert *Initial_Count to a string. */

  FUNCTION_ENTRY("Create_Unique_Name")

  /* Get the size of BaseName.  We will need it when we computer whether or not our unique name will fit in Buffer. */
  BaseNameSize = strlen(BaseName);

  /* Set up Bracket_Adjustment.  If we are using brackets, then the adjustment is 4 as we must take into account the brackets
     themselves as well as the spaces which immediately preceed or follow them.  If we are NOT using brackets, then the
     adjustment is 0.                                                                                                           */
  if ( Add_Brackets )
    Bracket_Adjustment = 4;
  else
    Bracket_Adjustment = 0;

  /* We will loop until we get a unique name or until the names we are generating are too big to fit in Buffer. */
  do
  {

    /* Convert *Initial_Count into a string. */
    CountBuffer[0] = 0;                          /* Empty CountBuffer by making it a NULL string. */
    sprintf(CountBuffer,"%lu",*Initial_Count);

    /* Save the size of the string in CountBuffer. */
    CountSize = strlen(CountBuffer);

    /* Will our unique name fit in Buffer?  We will take into account the NULL at the end of the name.    */
    if ( ( BaseNameSize + CountSize + Bracket_Adjustment + 1 ) > BufferSize )
    {

      LOG_ERROR("Can not create a unique name as the base name size is larger than our name creation buffer!")

      FUNCTION_EXIT("Create_Unique_Name")

      /* We can not make a unique name in Buffer, so abort. */
      return FALSE;

    }

    /* Increment *Initial_Count so it will be ready for use on the next pass through the loop, or by the next call to Create_Unique_Name. */
    *Initial_Count = *Initial_Count + 1;

    /* Create what we hope is a unique name. */
    Buffer[0] = 0;                /* Eliminate any string that may be in Buffer. */

    if ( Add_Brackets )
    {

      strcpy(Buffer,"[ ");
      strcat(Buffer, BaseName);
      strcat(Buffer,CountBuffer);
      strcat(Buffer," ]");

    }
    else
    {

      strcpy(Buffer, BaseName);
      strcat(Buffer,CountBuffer);

    }

    /* Now we must see if what we have is unique. */

    /* Prepare Dup_Check_Data for use. */
    Dup_Check_Data.New_Name_Length = strlen(Buffer);
    Dup_Check_Data.Max_Name_Length = VOLUME_NAME_SIZE;
    Dup_Check_Data.Handle = 0;
    Dup_Check_Data.New_Serial_Number = 0;
    Dup_Check_Data.New_Name = Buffer;
    Dup_Check_Data.Duplicate_Name_Found = FALSE;
    Dup_Check_Data.Duplicate_Serial_Number_Found = 0;
    Dup_Check_Data.Check_Name = TRUE;

    /* Are we to check Disk Names for duplicates? */
    if ( ( Name_Lists_To_Use & DISK_NAMES ) != 0 )
    {

      /* We must check Disk Names for duplicates. */
      for ( Index = 0; ( Index < DriveCount) && ( ! Dup_Check_Data.Duplicate_Name_Found ); Index++ )
      {

        /* We will check our unique name candidate against the disk name. */
        if ( ( &( Buffer[0] ) != &( DriveArray[Index].Drive_Name[0] ) ) &&
             ( strncmp(Buffer,DriveArray[Index].Drive_Name, DISK_NAME_SIZE) == 0 )
           )
        {

          /* We have a duplicate name!  Set the Duplicate_Name_Found field in the Dup_Check_Data and break out of the for loop
             so that we may cycle around inside of the do-while loop and try another unique name.                               */
          Dup_Check_Data.Duplicate_Name_Found = TRUE;

          break;

        }

      }

    }

    /* Are we to check Volume Names? */
    if ( ( !Dup_Check_Data.Duplicate_Name_Found) && ( ( Name_Lists_To_Use & VOLUME_NAMES ) != 0 ) )
    {

      /* We will check for a duplicate name in the Volumes list. */
      ForEachItem(Volumes,&Duplicate_Check,&Dup_Check_Data,TRUE,&Error);

#ifdef DEBUG

#ifdef PARANOID

      assert( Error == DLIST_SUCCESS );

#else

      if ( Error != DLIST_SUCCESS )
      {

        LOG_ERROR1("ForEachItem failed!", "Error code", Error)

        FUNCTION_EXIT("Create_Unique_Name")

        return FALSE;

      }

#endif

#endif

    }

    /* Are we to check Partition Names? */
    if ( ( !Dup_Check_Data.Duplicate_Name_Found) && ( ( Name_Lists_To_Use & PARTITION_NAMES ) != 0 ) )
    {

      /* Our name is unique so far.  Lets check it against partition names. */
      for ( Index = 0; ( Index < DriveCount) && ( ! Dup_Check_Data.Duplicate_Name_Found ); Index++ )
      {

        /* Now we will compare our unique name candidate against all of the names of the partitions on the current drive. */
        Dup_Check_Data.Max_Name_Length = PARTITION_NAME_SIZE;
        ForEachItem(DriveArray[Index].Partitions,&Duplicate_Check,&Dup_Check_Data, TRUE, &Error);

#ifdef DEBUG

#ifdef PARANOID

        assert( Error == DLIST_SUCCESS );

#else

        if ( Error != DLIST_SUCCESS )
        {

          LOG_ERROR1("ForEachItem failed!", "Error code", Error)

          FUNCTION_EXIT("Create_Unique_Name")

          return FALSE;

        }

#endif

#endif


      }

    }

  } while ( Dup_Check_Data.Duplicate_Name_Found );

  /* We finally have a unique name!  Return success! */

  FUNCTION_EXIT("Create_Unique_Name")

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Adjust_Name                                      */
/*                                                                   */
/*   Descriptive Name: Removes leading and trailing spaces from Name.*/
/*                                                                   */
/*   Input: char * Name : The name from which leading and trailing   */
/*                        spaces are to be removed.                  */
/*                                                                   */
/*   Output: The function return value is a pointer to the first     */
/*           character in Name which is not a space.                 */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects:  If Name has trailing spaces, the first trailing  */
/*                  space will be overwritten with a 0x0.            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
char * _System Adjust_Name( char * Name )
{

  char *  New_Name = Name;  /* Used to scan Name for spaces and the terminating null character. */
  char *  End_Of_Name;      /* Used to hold a pointer to the terminating null character in Name. */

  FUNCTION_ENTRY("Adjust_Name")

  /* Find the end of New_Name. */
  while ( *New_Name != 0x0 )
  {

    New_Name++;

  }

  /* Save the position of the terminating NULL in Name. */
  End_Of_Name = New_Name;

  /* If Name was originally NULL, then New_Name will equal Name at this point.  Check for this. */
  if ( New_Name == Name )
  {

    FUNCTION_EXIT("Adjust_Name")

    return New_Name;

  }

  /* Now move backward while there are spaces. */
  do
  {

    New_Name--;

  } while ( (*New_Name == ' ' ) && ( New_Name != Name ) );

  /* Is New_Name pointing to a space? */
  if ( *New_Name == ' ' )
  {

    LOG_EVENT("The name to adjust is all spaces!")

    FUNCTION_EXIT("Adjust_Name")

    /* The string must be all spaces!  Return a pointer to the terminating NULL character in Name. */
    return End_Of_Name;

  }

  /* Since *New_Name is not a space, the character following it must be a space or the terminating null.  Either way, make it the
     terminating null character.                                                                                                  */
  New_Name++;
  *New_Name = 0;

  /* Now lets find the first character which is not a space. */
  New_Name = Name;

  while ( *New_Name == ' ' )
  {

    New_Name++;

  }

  FUNCTION_EXIT("Adjust_Name")

  /* Now we have what we want: trailing spaces have been eliminated by overwriting the first trailing space with a null character,
     and leading spaces have been eliminated by getting a pointer to the first non-space character.  Return New_Name!               */
  return New_Name;

}


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
void Set_Reboot_Flag( BOOLEAN Reboot, CARDINAL32 * Error_Code )
{

  API_ENTRY( "Set_Reboot_Flag" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Set_Reboot_Flag" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* The Reboot Flag is kept in the DLA Table of the first drive in the system.  The Engine can not be opened unless there is
     at least one drive in the system.                                                                                          */
  DriveArray[0].Reboot_Flag = Reboot;

  /* Indicate that we made a change to something on the drive. */
  DriveArray[0].ChangesMade = TRUE;

  API_EXIT( "Set_Reboot_Flag" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
BOOLEAN Get_Reboot_Flag( CARDINAL32 * Error_Code )
{

  BOOLEAN  Original_Reboot_Flag;  /* Used to retain the original value of the reboot flag. */

  API_ENTRY( "Get_Reboot_Flag" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Set_Reboot_Flag" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return FALSE;

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* The Reboot Flag is kept in the DLA Table of the first drive in the system.  The Engine can not be opened unless there is
     at least one drive in the system.                                                                                          */
  Original_Reboot_Flag = DriveArray[0].Reboot_Flag;

  /* Now set the Reboot_Flag to FALSE. */
  DriveArray[0].Reboot_Flag = FALSE;

  /* Indicate that we made a change to something on the drive. */
  DriveArray[0].ChangesMade = TRUE;

  if ( Original_Reboot_Flag )
  {

    LOG_EVENT("The Reboot Flag is TRUE")

  }
  else
  {

    LOG_EVENT("The Reboot Flag is FALSE")

  }

  API_EXIT( "Get_Reboot_Flag" )

  /* Return the original value of the Reboot Flag. */
  return Original_Reboot_Flag;

}

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
void Set_Install_Flags( CARDINAL32 Install_Flags, CARDINAL32 * Error_Code )
{

  API_ENTRY( "Set_Install_Flags" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Set_Install_Flags" )
    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* The Install Flags are kept in the DLA Table of the first drive in the system.  The Engine can not be opened unless there is
     at least one drive in the system.                                                                                          */
  DriveArray[0].Install_Flags = Install_Flags;

  /* Indicate that we made a change to something on the drive. */
  DriveArray[0].ChangesMade = TRUE;

  API_EXIT( "Set_Install_Flags" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
CARDINAL32 Get_Install_Flags( CARDINAL32 * Error_Code )
{

  API_ENTRY( "Get_Install_Flags" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Install_Flags" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return FALSE;

  }

  LOG_EVENT1("Returning the Install Flags.", "Install Flags", DriveArray[0].Install_Flags )

  API_EXIT( "Get_Install_Flags" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* The Install Flags are kept in the DLA Table of the first drive in the system.  The Engine can not be opened unless there is
     at least one drive in the system.                                                                                          */
  return DriveArray[0].Install_Flags;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Allocate_Engine_Memory( CARDINAL32 Size )
{

  ADDRESS   ReturnValue = NULL;  /* Used to hold the return value prior to function completion. */

  API_ENTRY( "Allocate_Engine_Memory" )

  if ( Size != 0 )
  {

    ReturnValue = malloc(Size);

  }

  API_EXIT( "Allocate_Engine_Memory" )

  return ReturnValue;

}


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
void Free_Engine_Memory( ADDRESS Object )
{

  API_ENTRY( "Free_Engine_Memory" )

  if ( Object != NULL )
  {
    free(Object);
  }

  API_EXIT( "Free_Engine_Memory" )

  return;

}


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
/*              memory starting at Buffer, and *Error will be 0.     */
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
void Read_Sectors ( CARDINAL32          Drive_Number,
                    LBA                 Starting_Sector,
                    CARDINAL32          Sectors_To_Read,
                    ADDRESS             Buffer,
                    CARDINAL32 *        Error)
{

  API_ENTRY( "Read_Sectors" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Read_Sectors" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Indicate success. */
  *Error = LVM_ENGINE_NO_ERROR;

  ReadSectors(Drive_Number, Starting_Sector, Sectors_To_Read, Buffer, Error);

  if ( *Error != DISKIO_NO_ERROR )
  {

    LOG_ERROR1("ReadSectors failed!", "Error code", *Error)

  }


  switch ( *Error )
  {

    case DISKIO_NO_ERROR :
                            *Error = LVM_ENGINE_NO_ERROR;
                            break;
    case DISKIO_READ_FAILED  :
    case DISKIO_WRITE_FAILED :
                               *Error = LVM_ENGINE_IO_ERROR;
                               break;
    case DISKIO_OUT_OF_MEMORY :
                                *Error = LVM_ENGINE_OUT_OF_MEMORY;
                                break;
    case DISKIO_REQUEST_OUT_OF_RANGE :
                                       *Error = LVM_ENGINE_IO_REQUEST_OUT_OF_RANGE;
                                       break;
    default :
              *Error = LVM_ENGINE_INTERNAL_ERROR;
              break;

  }

  if ( *Error != LVM_ENGINE_NO_ERROR )
  {

    LOG_EVENT1("Error code has been translated.","The LVM Engine Error Code",*Error)

  }

  API_EXIT( "Read_Sectors" )

  return;

}


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
void Write_Sectors ( CARDINAL32          Drive_Number,
                     LBA                 Starting_Sector,
                     CARDINAL32          Sectors_To_Write,
                     ADDRESS             Buffer,
                     CARDINAL32 *        Error)
{

  API_ENTRY( "Write_Sectors" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Write_Sectors" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Indicate success. */
  *Error = LVM_ENGINE_NO_ERROR;

  WriteSectors(Drive_Number, Starting_Sector, Sectors_To_Write, Buffer, Error);

  if ( *Error != DISKIO_NO_ERROR )
  {

    LOG_ERROR1("WriteSectors failed!","Error code", *Error)

  }

  /* Translate the error code into an LVM Engine error code. */
  switch ( *Error )
  {

    case DISKIO_NO_ERROR :
                            *Error = LVM_ENGINE_NO_ERROR;
                            break;
    case DISKIO_READ_FAILED  :
    case DISKIO_WRITE_FAILED :
                               *Error = LVM_ENGINE_IO_ERROR;
                               break;
    case DISKIO_OUT_OF_MEMORY :
                                *Error = LVM_ENGINE_OUT_OF_MEMORY;
                                break;
    case DISKIO_REQUEST_OUT_OF_RANGE :
                                       *Error = LVM_ENGINE_IO_REQUEST_OUT_OF_RANGE;
                                       break;
    default :
              *Error = LVM_ENGINE_INTERNAL_ERROR;
              break;

  }

  if ( *Error != LVM_ENGINE_NO_ERROR )
  {

    LOG_EVENT1("Error code has been translated.","The LVM Engine Error Code",*Error)

  }

  API_EXIT( "Write_Sectors" )

  return;

}


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
void _System Rediscover_PRMs( CARDINAL32 * Error_Code )
{

  DDI_Rediscover_param  Rediscovery_Parameters;  /* Used to do a PRM Rediscover so that PRMs with new media will be recognized. */
  DDI_Rediscover_data   Rediscovery_Data;        /* Used to do a PRM Rediscover so that PRMs with new media will be recognized. */
  CARDINAL32            ReturnCode;              /* Used to hold the return value from the Rediscover function call. */

  API_ENTRY( "Rediscover_PRMs" )

  /* Is the LVM Engine already open?  If so, then we can not proceed as the LVM Engine will have disabled PRM Rediscovery in OS2LVM. */
  if ( DriveArray != NULL )
  {

    LOG_ERROR("The LVM Engine is open!  This function is not available while the LVM Engine is open!")

    API_EXIT( "Rediscover_PRMs" )

    *Error_Code = LVM_ENGINE_ALREADY_OPEN;

    return;

  }

  /* We must open the DISKIO module. */
  if ( OpenDrives( &ReturnCode ) )
  {

    /* Perform a PRM Rediscover operation so that any PRMs with new media will be recognized. */
    Rediscovery_Parameters.DDI_TotalDrives = 0;   /* 0 here indicates that a PRM only rediscover is to be performed instead of a full rediscover. */
    Rediscovery_Parameters.DDI_aDriveNums[0] = 1;
    Rediscovery_Data.DDI_TotalExtends = 0;
    Rediscovery_Data.NewIFSMUnits = 0;

    /* Do the PRM Rediscovery.  */
    ReturnCode = Rediscover( &Rediscovery_Parameters, &Rediscovery_Data );

    if ( ReturnCode != NO_ERROR )
    {

      LOG_ERROR1("Rediscover failed!"," Error Code", ReturnCode)

    }

    /* We must close the DISKIO module. */
    CloseDrives();

    /* Did we succeed? */
    if ( ReturnCode != NO_ERROR )
      *Error_Code = LVM_ENGINE_REDISCOVER_FAILED;
    else
      *Error_Code = LVM_ENGINE_NO_ERROR;

  }
  else
  {

    /* We could not open the DISKIO module! */
    *Error_Code = LVM_ENGINE_DRIVE_OPEN_FAILURE;

  }

  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_EVENT1("Error code has been translated.","The new LVM Engine error code", *Error_Code)

  }

  API_EXIT( "Rediscover_PRMs" )

  return;

}





/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: DetermineFreeSpace                               */
/*                                                                   */
/*   Descriptive Name: This function is designed to be used with the */
/*                     ForEachItem function in DLIST.  It should     */
/*                     only be used on lists containing items of     */
/*                     type Partition_Data.  Its purpose is to scan  */
/*                     a list of Partition_Data items and add up the */
/*                     total number of sectors residing in           */
/*                     Partition_Data entries marked as FreeSpace.   */
/*                     It also keeps the size of the largest         */
/*                     Partition_Data entry marked as FreeSpace.     */
/*                                                                   */
/*   Input: ADDRESS Object : This is the address of an entry in the  */
/*                           DLIST being scanned.  It must point to  */
/*                           item of type Partition_Data for this    */
/*                           function to work correctly.             */
/*          TAG ObjectTag : This is the TAG value assigned to Object.*/
/*                          For this function to work, ObjectTag     */
/*                          should be equal to PARTITION_DATA_TAG.   */
/*          CARDINAL32 ObjectSize : This is the size, in bytes, of   */
/*                                  the item pointed to by Object.   */
/*                                  This value is not currently used */
/*                                  by this function.                */
/*          ADDRESS Parameters : This is the address of an item of   */
/*                               type Drive_Information_Record.  This*/
/*                               function will manipulate the        */
/*                               Largest_Free_Block_Of_Sectors and   */
/*                               Total_Available_Sectors fields of   */
/*                               the Drive_Information_Record at the */
/*                               address specified by Parameters.    */
/*                               These fields should be set to 0     */
/*                               before the first call to this       */
/*                               function.                           */
/*                                                                   */
/*   Output: The Largest_Free_Block_Of_Sectors and                   */
/*           Total_Available_Sectors fields of the                   */
/*           Drive_Information_Record pointed to by Parameters may   */
/*           be updated, if appropriate.                             */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error will be set to a     */
/*                   non-zero value.                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System DetermineFreeSpace(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  Drive_Information_Record * ReturnValue = (Drive_Information_Record * ) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  FUNCTION_ENTRY("DetermineFreeSpace")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof(Partition_Data) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected Object Tag or Object Size!", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("DetermineFreeSpace")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent free space? */
  if ( PartitionRecord->Partition_Type == FreeSpace )
  {

    /* We are in business!  Is the size of this partition greater than any we have seen before? */
    if ( ReturnValue->Largest_Free_Block_Of_Sectors < PartitionRecord->Partition_Size )
    {

      /* This is the biggest we have seen so far!  Save it. */
      ReturnValue->Largest_Free_Block_Of_Sectors = PartitionRecord->Partition_Size;

    }

    /* Add this partition to the count of total free space on the drive. */
    ReturnValue->Total_Available_Sectors += PartitionRecord->Partition_Size;

  }

  FUNCTION_EXIT("DetermineFreeSpace")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Assign_Serial_Numbers                            */
/*                                                                   */
/*   Descriptive Name: This function is designed to be used with the */
/*                     ForEachItem function in DLIST.  It should     */
/*                     only be used on lists containing items of     */
/*                     type Partition_Data.  Its purpose is to scan  */
/*                     a list of Partition_Data items for partitions */
/*                     and ensure that any partitions found have     */
/*                     serial numbers.                               */
/*                                                                   */
/*   Input: ADDRESS Object : This is the address of an entry in the  */
/*                           DLIST being scanned.  It must point to  */
/*                           item of type Partition_Data for this    */
/*                           function to work correctly.             */
/*          TAG ObjectTag : This is the TAG value assigned to Object.*/
/*                          For this function to work, ObjectTag     */
/*                          should be equal to PARTITION_DATA_TAG.   */
/*          CARDINAL32 ObjectSize : This is the size, in bytes, of   */
/*                                  the item pointed to by Object.   */
/*                                  This value is not currently used */
/*                                  by this function.                */
/*          ADDRESS Parameters : This parameter is not used by this  */
/*                               function.                           */
/*                                                                   */
/*   Output: Any partitions in the partition list which did not have */
/*           serial numbers will now have serial numbers.  The drive */
/*           corresponding to these partitions will have its         */
/*           ChangesMade flag set to TRUE.                           */
/*                                                                   */
/*   Error Handling: If an error occurs, *Error will be set to a     */
/*                   non-zero value.                                 */
/*                                                                   */
/*   Side Effects:  None.                                            */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void _System Assign_Serial_Numbers_And_Names(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast all the time. */
  Name_Counter *    Count = (Name_Counter *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  FUNCTION_ENTRY("Assign_Serial_Numbers_And_Names")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof(Partition_Data) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected Object Tag or Object Size!", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Assign_Serial_Numbers_And_Names")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* We are in business!  Does this partition have a serial number? */
    if ( PartitionRecord->DLA_Table_Entry.Partition_Serial_Number == 0 )
    {

      /* Give it a serial number. */
      PartitionRecord->DLA_Table_Entry.Partition_Serial_Number = Create_Serial_Number();

      /* Does the DLA_Table_Entry for this partition have all the required information? */
      PartitionRecord->DLA_Table_Entry.Partition_Start = PartitionRecord->Starting_Sector;
      PartitionRecord->DLA_Table_Entry.Partition_Size = PartitionRecord->Partition_Size;

      /* Mark the drive so that this change can get written to disk. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

    /* Is this partition an LVM partition?  If so, then we must make sure that the disk name in the LVM Signature Sector
       agrees with the current disk name.  The disk name in the LVM Signature Sector is used by the OS2LVM BBR code to
       identify drives with problems.                                                                                    */
    if ( PartitionRecord->Signature_Sector != NULL )
    {

      /* We have an LVM Signature Sector!  Do the disk names agree? */
      if ( strncmp(PartitionRecord->Signature_Sector->Disk_Name, DriveArray[PartitionRecord->Drive_Index].Drive_Name, DISK_NAME_SIZE) != 0 )
      {

        /* We must update the LVM Signature Sector. */
        strncpy(PartitionRecord->Signature_Sector->Disk_Name, DriveArray[PartitionRecord->Drive_Index].Drive_Name, DISK_NAME_SIZE);

        /* Mark the drive as having had changes made so that the updated LVM Signature Sector will be written to disk. */
        DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      }

    }

    /* Does the DriveArray entry corresponding to this partition record have it Corrupt flag on? */
    if ( ( DriveArray[PartitionRecord->Drive_Index].Corrupt && !DriveArray[PartitionRecord->Drive_Index].NonFatalCorrupt) &&
         ( PartitionRecord->Partition_Type != MBR_EBR ) )
    {

      /* Set the partition name to "[ C# ]" */
      if ( ! Create_Unique_Name(PARTITION_NAMES, TRUE, "C", &(Count->Corrupt_Counter),PartitionRecord->Partition_Name, PARTITION_NAME_SIZE) )
      {

        LOG_ERROR("Create_Unique_Name failed!")

        FUNCTION_EXIT("Assign_Serial_Numbers_And_Names")

        /* This should never fail as the number of partitions in the system can never exceed the number of unique names available! */
        *Error = DLIST_CORRUPTED;

        return;

      }

    }
    else
    {
      /* Now lets start assigning default names. */
      switch ( PartitionRecord->Partition_Type )
      {

        case FreeSpace : /* This partition gets a name of the form "[ FSxx ]" where xx is a cardinal value. */

                         /* Set the partition name to "[ FSxx ]" */
                         if ( ! Create_Unique_Name(PARTITION_NAMES, TRUE, "FS", &(Count->Free_Space_Counter),PartitionRecord->Partition_Name, PARTITION_NAME_SIZE) )
                         {

                           LOG_ERROR("Create_Unique_Name failed!")

                           FUNCTION_EXIT("Assign_Serial_Numbers_And_Names")

                           /* This should never fail as the number of partitions in the system can never exceed the number of unique names available! */
                           *Error = DLIST_CORRUPTED;

                           return;

                         }

                         break;

        case Partition : /* If this does not already have a name, give it one of the form "[ Axx ]". */

                         /* Is this the Boot Manager partition? */
                         if ( PartitionRecord->External_Handle == Boot_Manager_Handle )
                         {

                           /* This is the Boot Manager partition.  Give it the name reserved for the Boot Manager partition. */
                           sprintf(PartitionRecord->Partition_Name,"[ BOOT MANAGER ]");

                         }
                         else
                         {

                           /* Is Partition_Name NULL? */
                           if ( PartitionRecord->Partition_Name[0] == 0 )
                           {

                             /* Set the partition name to "[ Ax ]" */
                             if ( ! Create_Unique_Name(PARTITION_NAMES, TRUE, "A", &(Count->Available_Counter),PartitionRecord->Partition_Name, PARTITION_NAME_SIZE) )
                             {

                               LOG_ERROR("Create_Unique_Name failed!")

                               FUNCTION_EXIT("Assign_Serial_Numbers_And_Names")

                               /* This should never fail as the number of partitions in the system can never exceed the number of unique names available! */
                               *Error = DLIST_CORRUPTED;

                               return;

                             }

                           }

                         }

                         break;
        case MBR_EBR : /* Do nothing.  We don't need to give names to MBR/EBRs. */
                       break;
        default: /* If we get here, then something has been corrupted! */

#ifdef DEBUG

#ifdef PARANOID

          assert(0);

#endif

#endif

          *Error = DLIST_CORRUPTED;

          break;

      }

    }

  }

  FUNCTION_EXIT("Assign_Serial_Numbers_And_Names")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

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
static void _System Clear_Startable_Flags(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  FUNCTION_ENTRY("Clear_Startable_Flags")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof(Partition_Data) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Unexpected Object Tag or Object Size!", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Clear_Startable_Flags")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* We are in business!  Is this a primary partition? */
    if ( PartitionRecord->Primary_Partition )
    {

      /* Clear the Active Flag. */
      PartitionRecord->Partition_Table_Entry.Boot_Indicator &= ~(0x80);

    }

  }

  FUNCTION_EXIT("Clear_Startable_Flags")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

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
static void _System Overwrite_Sectors(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Kill_Sector_Data without having to typecast each time. */
  Kill_Sector_Data *    Sector_Data = (Kill_Sector_Data *) Object;

  FUNCTION_ENTRY("Overwrite_Sectors")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == KILL_SECTOR_DATA_TAG ) && ( ObjectSize == sizeof(Kill_Sector_Data) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != KILL_SECTOR_DATA_TAG ) || ( ObjectSize != sizeof( Kill_Sector_Data ) ) )
  {

    LOG_ERROR2("Unexpected Object Tag or Object Size!", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Overwrite_Sectors")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Kill_Sector_Data. */

  /* Overwrite the specified sector. */
  WriteSectors(Sector_Data->Drive_Index + 1, Sector_Data->Sector_ID, 1, &Kill_Sector, Error);

  /* If there was an I/O error, flag it in the DriveArray. */
  if ( *Error != DISKIO_NO_ERROR )
  {

    LOG_EVENT3("WriteSectors failed!","Drive Number",Sector_Data->Drive_Index + 1,"Sector ID", Sector_Data->Sector_ID, "Count", 1)

    DriveArray[Sector_Data->Drive_Index].IO_Error = TRUE;

  }

  /* We don't care if the write succeeds or not - this is just an extra safety precaution against some extremely unlikely occurrences. */

  FUNCTION_EXIT("Overwrite_Sectors")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

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
BOOLEAN _System Valid_Signature_Sector( Partition_Data * PartitionRecord, LVM_Signature_Sector * Signature_Sector)
{

  CARDINAL32    Old_CRC;
  CARDINAL32    Calculated_CRC;

  FUNCTION_ENTRY("Valid_Signature_Sector")

  /* Are the signatures correct? */
  if ( ( Signature_Sector->LVM_Signature1 != LVM_PRIMARY_SIGNATURE ) ||
       ( Signature_Sector->LVM_Signature2 != LVM_SECONDARY_SIGNATURE )
     )
  {

    LOG_EVENT("Invalid LVM Signature Sector")

    FUNCTION_EXIT("Valid_Signature_Sector")

    /* This is not a valid LVM Signature Sector. */
    return FALSE;

  }

  /* Is the CRC correct? */
  Old_CRC = Signature_Sector->Signature_Sector_CRC;
  Signature_Sector->Signature_Sector_CRC = 0;

  Calculated_CRC = CalculateCRC( INITIAL_CRC, Signature_Sector, BYTES_PER_SECTOR);

  if ( Calculated_CRC != Old_CRC )
  {

    LOG_EVENT("Invalid CRC")

    FUNCTION_EXIT("Valid_Signature_Sector")

    /* This is not a valid LVM Signature Sector. */
    return FALSE;

  }

  /* Is the version correct? */
  if ( ( Signature_Sector->LVM_Major_Version_Number > CURRENT_LVM_MAJOR_VERSION_NUMBER ) ||
       ( ( Signature_Sector->LVM_Major_Version_Number == CURRENT_LVM_MAJOR_VERSION_NUMBER ) &&
         ( Signature_Sector->LVM_Minor_Version_Number > CURRENT_LVM_MINOR_VERSION_NUMBER )
       )
     )
  {

    LOG_EVENT("Version Mismatch!")

    FUNCTION_EXIT("Valid_Signature_Sector")

    /* We have a version of the Signature Sector that we don't understand! */
    return FALSE;

  }

  /* Does the partition information in the LVM Signature Sector agree with what is in the Partition Record? */
  if ( ( Signature_Sector->Partition_Start != PartitionRecord->Starting_Sector ) ||
       ( Signature_Sector->Partition_Sector_Count != PartitionRecord->Partition_Size )
     )
  {

    LOG_EVENT("The partition information in the Signature Sector does not agree with what is in the Partition Record!")

    FUNCTION_EXIT("Valid_Signature_Sector")

    /* The partition has been moved or the LVM Signature Sector has been corrupted because they don't agree on
       the location and size of the partition!  We must assume that the LVM Signature Sector is invalid.        */

    return FALSE;

  }

  FUNCTION_EXIT("Valid_Signature_Sector")

  return TRUE;

}


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
/*********************************************************************/
void _System Set_Java_Call_Back( void (* _System Execute_Java_Class) ( char *       Class_Name,
                                                                       ADDRESS      InputBuffer,
                                                                       CARDINAL32   InputBufferSize,
                                                                       ADDRESS    * OutputBuffer,
                                                                       CARDINAL32 * OutputBufferSize,
                                                                       CARDINAL32 * Error_Code),
                                 CARDINAL32 * Error_Code )
{
  API_ENTRY( "Set_Java_Call_Back" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Set_Java_Call_Back" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Save the Java Call Back address. */
  Java_Call_Back = Execute_Java_Class;

  API_EXIT( "Set_Java_Call_Back" )

  return;

}


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
Feature_Information_Array _System Get_Available_Features( CARDINAL32 * Error_Code )
{

  Feature_Information_Array   Available_Features_Array;

  Available_Features_Array.Count = 0;
  Available_Features_Array.Feature_Data = NULL;

  API_ENTRY("Get_Available_Features")

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Available_Features" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return Available_Features_Array;

  }


  Available_Features_Array.Feature_Data = NULL;

  Available_Features_Array.Count = GetListSize(Available_Features, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("GetListSize failed!","Error code", *Error_Code)

    API_EXIT( "Get_Available_Features" )

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return Available_Features_Array;

  }

  /* PassThru and BBR are always in the Available Features array but should not be reported. */
  if ( Available_Features_Array.Count > 2 )
  {

    /* Adjust the count to remove PassThru and BBR. */
    Available_Features_Array.Count -= 2;

    /* Allocate memory. */
    Available_Features_Array.Feature_Data = (Feature_ID_Data *) malloc( Available_Features_Array.Count * sizeof(Feature_ID_Data) );

    if ( Available_Features_Array.Feature_Data == NULL )
    {

      LOG_ERROR("Unable to allocate the Available Features Array!  Out of memory!")

      API_EXIT( "Get_Available_Features" )

      /* We are out of memory! */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      return Available_Features_Array;

    }

    /* Now we must fill in the array!.  */
    Available_Features_Array.Count = 0;
    ForEachItem(Available_Features,&Load_Feature_Information_Array ,&Available_Features_Array, TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      LOG_ERROR1("ForEachItem failed!", "Error code", *Error_Code)

      API_EXIT( "Get_Available_Features" )

      /* This should not happen! */
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      free(Available_Features_Array.Feature_Data);

      Available_Features_Array.Feature_Data = NULL;

      return Available_Features_Array;

    }

  }
  else
    Available_Features_Array.Count = 0;

  API_EXIT( "Get_Available_Features" )

  return Available_Features_Array;

}


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
/*********************************************************************/
void _System Parse_Feature_Parameters( DLIST                               Tokens,
                                       LVM_Feature_Specification_Record  * Feature_Data,
                                       char **                             Error_Message,
                                       CARDINAL32 *                        Error_Code)
{

  Find_And_Parse_Record   Parsing_Data;
  CARDINAL32              Local_Error;

  API_ENTRY( "Parse_Feature_Parameters" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Parse_Feature_Parameters" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Now we must find the feature that the user specified. */

  /* Prepare the search data. */
  Parsing_Data.Tokens = Tokens;
  Parsing_Data.Feature_Data = Feature_Data;
  Parsing_Data.Error_Message = Error_Message;
  Parsing_Data.Feature_Found = FALSE;
  Parsing_Data.Error_Code = Error_Code;
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Find the feature and execute its parsing function. */
  ForEachItem(Available_Features, &Find_Feature_And_Parse, &Parsing_Data, TRUE, &Local_Error);
  if ( Local_Error != DLIST_SUCCESS )
  {

    LOG_ERROR1("ForEachItem failed!", "Error code", Local_Error )

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }

  if ( ! Parsing_Data.Feature_Found )
  {

    LOG_ERROR("The specified feature was not found!")

    *Error_Code = LVM_ENGINE_BAD_FEATURE_ID;

  }

  API_EXIT( "Parse_Feature_Parameters" )

  return;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Issue_Feature_Command( CARDINAL32 Feature_ID,
                                    ADDRESS Handle,
                                    BOOLEAN Ring0,
                                    ADDRESS InputBuffer,
                                    CARDINAL32 InputSize,
                                    ADDRESS * OutputBuffer,
                                    CARDINAL32 * OutputSize,
                                    CARDINAL32 * Error_Code )
{

  ADDRESS                             Object;                  /* Used when translating Handle into something usable. */
  TAG                                 ObjectTag;               /* Used when translating Handle into something usabel. */
  Volume_Data *                       Volume_Record;
  Partition_Data *                    Partition_Record;
  Plugin_Function_Table_V1 *          Function_Table;

  API_ENTRY( "Issue_Feature_Command" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Issue_Feature_Command" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed!", "Error code", *Error_Code)

    API_EXIT( "Issue_Feature_Command" )

    return;

  }

  /* Choose the appropriate operation based upon the ObjectTag. */
  switch ( ObjectTag )
  {

    case VOLUME_DATA_TAG : /* We are to issue the feature command against a volume. */
                           /* Establish access to the volume data. */
                           Volume_Record = (Volume_Data *) Object;

                           /* Is this a Ring 0 or Ring 3 command? */
                           if ( Ring0 )
                           {

                             /* Is this a new volume?  If so, then there are no Ring 0 features yet! */
                             if ( Volume_Record->New_Volume )
                             {

                               LOG_ERROR("Issue_Feature_Command can not be used with a new volume!")

                               *Error_Code = LVM_ENGINE_VOLUME_HAS_NOT_BEEN_COMMITTED_YET;

                             }
                             else
                             {

                               /* Does the volume have a drive letter? */
                               if ( ( toupper(Volume_Record->Initial_Drive_Letter) < 'A' ) ||
                                    ( toupper(Volume_Record->Initial_Drive_Letter) > 'Z' )
                                  )
                               {

                                 LOG_ERROR("The volume specified does not have a drive letter and can not be referenced!")

                                 *Error_Code = LVM_ENGINE_UNABLE_TO_REFERENCE_VOLUME;

                               }
                               else
                               {

                                 /* Is this volume a compatibility volume? */
                                 if ( Volume_Record->Compatibility_Volume )
                                 {

                                   LOG_ERROR("Compatibility Volumes do not have features!")

                                   *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

                                 }
                                 else
                                 {
                                   /* We must call the IOCTL interface. */
                                   Issue_Ring0_Feature_Command(Feature_ID,
                                                               toupper(Volume_Record->Initial_Drive_Letter),
                                                               InputBuffer,
                                                               Error_Code);

                                   switch ( *Error_Code )
                                   {

                                     case DISKIO_NO_ERROR : *Error_Code = LVM_ENGINE_NO_ERROR;
                                                            break;
                                     case DISKIO_OUT_OF_MEMORY :
                                     case DISKIO_NOT_ENOUGH_HANDLES :
                                                                      LOG_ERROR1("Issue_Ring0_Feature_Command failed!","Error code", *Error_Code)

                                                                      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
                                                                      break;
                                     case DISKIO_UNEXPECTED_OS_ERROR :
                                                                       LOG_ERROR1("Issue_Ring0_Feature_Command failed!","Error code", *Error_Code)

                                                                       *Error_Code = LVM_ENGINE_IO_ERROR;
                                                                       break;
                                     default :
                                               LOG_ERROR1("Issue_Ring0_Feature_Command failed!","Error code", *Error_Code)

                                               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
                                               break;
                                   }

                                 }

                               }

                             }

                           }
                           else
                           {

                             /* Issue the command to the Ring 3 portion of every feature on the specified volume. */
                             Issue_Volume_Feature_Command( Volume_Record,
                                                           Feature_ID,
                                                           InputBuffer,
                                                           InputSize,
                                                           OutputBuffer,
                                                           OutputSize,
                                                           Error_Code);

                           }

                           break;

    case PARTITION_DATA_TAG : /* We are to issue the feature command against a partition or aggregate. */

                              /* Establish access to the partition data. */
                              Partition_Record = (Partition_Data *) Object;

                              /* Does the partition have any features associated with it? */
                              if ( Partition_Record->Feature_Data == NULL )
                              {

                                LOG_ERROR("The specified partition has no features associated with it!")

                                *Error_Code = LVM_ENGINE_BAD_PARTITION;
                                break;

                              }

                              /* Is the command destined for Ring0?  If so, then we can not allow it as we only support
                                 feature commands going to volumes at Ring 0.                                            */
                              if ( Ring0 )
                              {

                                LOG_ERROR("Only Volumes can have a feature command sent to Ring 0.")

                                *Error_Code =  LVM_ENGINE_OPERATION_NOT_ALLOWED;
                                break;

                              }

                              /* Extract the function table from the feature data. */
                              Function_Table = (Plugin_Function_Table_V1 *) Partition_Record->Feature_Data->Function_Table;

                              /* Now call the PassThru function.  This is the PassThru function of the topmost feature on the partition/aggregate.
                                 If the command is not meant for this feature, it will pass the command to the next feature down.                     */
                              Function_Table->PassThru(Feature_ID, Partition_Record, InputBuffer, InputSize, OutputBuffer, OutputSize, Error_Code);

                              break;

    default : /* We can not issue a feature command against this object! */
              LOG_ERROR1("We can not issue a feature command against the object associated with the specified handle.", "Object Tag", ObjectTag)

              *Error_Code = LVM_ENGINE_BAD_HANDLE;
              break;

  }

  API_EXIT( "Issue_Feature_Command" )

  return;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
LVM_Handle_Array_Record  _System Get_Child_Handles( ADDRESS Handle, CARDINAL32 * Error_Code)
{

  LVM_Handle_Array_Record             LVM_Handle_Array;
  ADDRESS                             Object;                  /* Used when translating Handle into something usable. */
  TAG                                 ObjectTag;               /* Used when translating Handle into something usabel. */
  Partition_Data *                    PartitionRecord;
  Volume_Data *                       VolumeRecord;

  /* Declare a variable to walk the context chain if an aggregate is encountered. */
  Feature_Context_Data *              Current_Context;


  /* Initialize our return value so that, if we abort, something appropriate will be returned. */
  LVM_Handle_Array.Count = 0;
  LVM_Handle_Array.Handles = NULL;

  API_ENTRY( "Get_Child_Handles" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Child_Handles" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return LVM_Handle_Array;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed.", "Error code", *Error_Code)

    API_EXIT( "Get_Child_Handles" )

    return LVM_Handle_Array;

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */
                              PartitionRecord = ( Partition_Data * ) Object;

                              /* Is this an aggregate? */
                              if ( PartitionRecord->Drive_Index != (CARDINAL32) -1L )
                              {

                                LOG_ERROR("Partitions do not have children!")

                                API_EXIT( "Get_Child_Handles" )

                                /* This is a partition, not an aggregate!  */
                                *Error_Code = LVM_ENGINE_NO_CHILDREN;

                                return LVM_Handle_Array;

                              }

                              /* Since this is an aggregate, we must allocate enough memory to return the handles
                                 of all of the partitions/aggregates in this aggregate's partitions list.          */

                              /* Now we must find the partitions list for the aggregate. */
                              Current_Context = PartitionRecord->Feature_Data;

                              /* The only feature's which will have a partitions list are Aggregators.  There can be only
                                 one aggregate class feature on an aggregate, and it will always be the one with a partitions list. */
                              while ( Current_Context->Partitions == NULL )
                              {

                                /* Get the next context in the feature context chain for this aggregate. */
                                Current_Context = Current_Context->Old_Context;

                              }

                              /* How many items are in the partitions list? */
                              LVM_Handle_Array.Count = GetListSize(Current_Context->Partitions, Error_Code);
                              if ( *Error_Code != DLIST_SUCCESS )
                              {

                                LOG_ERROR1("GetListSize failed!", "Error code", *Error_Code)

                                API_EXIT( "Get_Child_Handles" )

                                /* This should not happen!  Our data structures have been corrupted! */
                                *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                return LVM_Handle_Array;

                              }

                              /* Allocate memory for the handle array. */
                              LVM_Handle_Array.Handles = (ADDRESS *) malloc( LVM_Handle_Array.Count * sizeof(ADDRESS) );
                              if ( LVM_Handle_Array.Handles == NULL )
                              {

                                LOG_ERROR("Unable to allocate the handle array!  Out of memory!")

                                API_EXIT( "Get_Child_Handles" )

                                *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
                                LVM_Handle_Array.Count = 0;
                                return LVM_Handle_Array;

                              }

                              /* Now we must fill the handle array. */
                              LVM_Handle_Array.Count = 0;
                              ForEachItem(Current_Context->Partitions, &Fill_Handle_Array, &LVM_Handle_Array, TRUE, Error_Code);
                              if ( *Error_Code != DLIST_SUCCESS )
                              {

                                LOG_ERROR1("ForEachItem failed!", "Error code", *Error_Code)

                                free(LVM_Handle_Array.Handles);

                                API_EXIT( "Get_Child_Handles" )

                                LVM_Handle_Array.Count = 0;
                                LVM_Handle_Array.Handles = NULL;
                                *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                                return LVM_Handle_Array;

                              }

                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */
                           VolumeRecord = ( Volume_Data * ) Object;

                           /* We have only 1 handle to return.  Allocate memory for it. */
                           LVM_Handle_Array.Handles = (ADDRESS *) malloc( sizeof(ADDRESS) );

                           if ( LVM_Handle_Array.Handles == NULL )
                           {

                             LOG_ERROR("Unable to allocate the handle array!  Out of memory!")

                             API_EXIT( "Get_Child_Handles" )

                             *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
                             return LVM_Handle_Array;

                           }

                           /* Initialize the return values. */
                           LVM_Handle_Array.Count = 1;
                           LVM_Handle_Array.Handles[0] = VolumeRecord->Partition->External_Handle;

                           break;

    default :
              /* We have a bad handle! */
              *Error_Code = LVM_ENGINE_BAD_HANDLE;

              LOG_ERROR1("Unexpected ObjectTag returned from Translate_Handle!", "Object Tag", ObjectTag)

              API_EXIT( "Get_Child_Handles" )

              return LVM_Handle_Array;

              /* Keep the compiler happy. */
              break;

  }

  API_EXIT( "Get_Child_Handles" )

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return LVM_Handle_Array;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
ADDRESS _System Get_Parent_Handle( ADDRESS Handle, CARDINAL32 * Error_Code)
{

  ADDRESS                             Object;                  /* Used when translating Handle into something usable. */
  TAG                                 ObjectTag;               /* Used when translating Handle into something usabel. */
  Partition_Data *                    PartitionRecord;
  ADDRESS                             Parent_Handle = NULL;

  API_ENTRY( "Get_Parent_Handle" )

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Parent_Handle" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return NULL;

  }

  /* Determine what kind of a handle we have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed.", "Error code", *Error_Code)

    API_EXIT( "Get_Parent_Handle" )

    return NULL;

  }

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {

    case PARTITION_DATA_TAG : /* We have Partition_Data here. */
                              PartitionRecord = ( Partition_Data * ) Object;

                              /* If this partition is not part of a volume, then, by definition, it has no parent. */
                              if ( PartitionRecord->Volume_Handle == NULL )
                                *Error_Code = LVM_ENGINE_NO_PARENT;
                              else
                                Parent_Handle = PartitionRecord->Parent_Handle;

                              break;

    case VOLUME_DATA_TAG : /* We have Volume_Data here. */

                           /* Volumes do not have parents! */
                           *Error_Code = LVM_ENGINE_NO_PARENT;

                           break;

    default :
              /* We have a bad handle! */
              *Error_Code = LVM_ENGINE_BAD_HANDLE;

              LOG_ERROR1("Unexpected ObjectTag returned from Translate_Handle!", "Object Tag", ObjectTag)

              API_EXIT( "Get_Parent_Handle" )
              break;

  }

  API_EXIT( "Get_Parent_Handle" )

  return Parent_Handle;

}


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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
Feature_Information_Array _System Get_Features( ADDRESS Handle, CARDINAL32 * Error_Code )
{

  Feature_Information_Array     Feature_Information;
  ADDRESS                       Object;                          /* Used when translating the Volume_Handle into a Volume_Data structure. */
  TAG                           ObjectTag;                       /* Used when translating the Volume_Handle into a Volume_Data structure. */

  API_ENTRY("Get_Features")

  /* Initialize Feature_Information assuming failure. */
  Feature_Information.Count = 0;
  Feature_Information.Feature_Data = NULL;

  /* Has the LVM Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    LOG_ERROR("The LVM Engine is NOT open!")

    API_EXIT( "Get_Features" )

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return Feature_Information;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    LOG_ERROR1("Translate_Handle failed!", "Error code", *Error_Code)

    API_EXIT( "Get_Features" )

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    return Feature_Information;

  }

  /* From the ObjectTag we can tell what Object points to. */

  switch ( ObjectTag )
  {

    case VOLUME_DATA_TAG : /* Get the features on the volume from the Volume Manager. */

                           API_EXIT( "Get_Features" )

                           return Get_Volume_Features( (Volume_Data *) Object, Error_Code );
                           break;

    case PARTITION_DATA_TAG : /* Get the features on the aggregate/partition from the Partition Manager. */

                              API_EXIT( "Get_Features" )

                              return Get_Partition_Features( (Partition_Data *) Object, Error_Code );
                              break;
    default :
              /* We have a bad handle.  */
              *Error_Code = LVM_ENGINE_BAD_HANDLE;

              LOG_ERROR1("Unexpected ObjectTag returned from Translate_Handle!", "Object Tag", ObjectTag)

              break;
  }

  API_EXIT( "Get_Features" )

  /* We should only get here if there was an error! */
  return Feature_Information;

}




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
/*   Notes:  None.                                                                                  */
/*                                                                                                  */
/****************************************************************************************************/
void _Far16 _Pascal _loadds OPEN_LVM_ENGINE16( BOOLEAN Ignore_CHS, CARDINAL32 * _Seg16 Error_Code )
{

  Open_LVM_Engine( Ignore_CHS, Error_Code );

  return;

}



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
/*   Notes:  None.                                                                                  */
/*                                                                                                  */
/****************************************************************************************************/
void _Far16 _Pascal _loadds OPEN_LVM_ENGINE216( BOOLEAN Ignore_CHS, LVM_Interface_Types Interface_Type, CARDINAL32 * _Seg16 Error_Code )
{

  Open_LVM_Engine2( Ignore_CHS, Interface_Type, Error_Code );

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Commit_Changes16                                 */
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
BOOLEAN _Far16 _Pascal _loadds Commit_Changes16( CARDINAL32 * _Seg16 Error_Code )
{

  return Commit_Changes(Error_Code);

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Close_LVM_Engine16                               */
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
void _Far16 _Pascal _loadds Close_LVM_Engine16 ( void )
{

  Close_LVM_Engine();

  return;

}


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
void _Far16 _Pascal _loadds  REFRESH_LVM_ENGINE16( CARDINAL32 * _Seg16 Error_Code )
{

  Refresh_LVM_Engine( Error_Code );

  return;
}


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
                                                    )
{

  Drive_Control_Array        Control_Data;  /* Used to hold the value returned by the Get_Drive_Control_Data call. */

  API_ENTRY("GET_DRIVE_CONTROL_DATA16")

  Control_Data = Get_Drive_Control_Data( Error_Code );

  *Drive_Control_Data = Control_Data.Drive_Control_Data;
  *Count = Control_Data.Count;

  API_EXIT( "GET_DRIVE_CONTROL_DATA16" )

  return;

}


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
                                              )
{
  Drive_Information_Record       Drive_Information;

  API_ENTRY("GET_DRIVE_STATUS16")

  Drive_Information = Get_Drive_Status( (ADDRESS) Drive_Handle, Error_Code );

  *Drive_Status = Drive_Information;

  API_EXIT( "GET_DRIVE_STATUS16" )

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Partitions16                                 */
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
void _Far16 _Pascal _loadds GET_PARTITIONS16( CARDINAL32                                      Handle,
                                              Partition_Information_Record * _Seg16 * _Seg16  Partition_Array,
                                              CARDINAL32 *                            _Seg16  Count,
                                              CARDINAL32 *                            _Seg16  Error_Code
                                           )
{

  Partition_Information_Array Data;

  API_ENTRY("GET_PARTITIONS16")

  Data = Get_Partitions( (ADDRESS) Handle, Error_Code );

  *Partition_Array = Data.Partition_Array;
  *Count = Data.Count;

  API_EXIT( "GET_PARTITIONS16" )

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Set_Name16                                       */
/*                                                                   */
/*   Descriptive Name: Sets the name of a volume, drive, or partition*/
/*                                                                   */
/*   Input: CARDINAL32 Handle - The handle of the drive, partition,  */
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
void _Far16 _Pascal _loadds SET_NAME16 ( CARDINAL32          Handle,
                                         char       * _Seg16 New_Name,
                                         CARDINAL32 * _Seg16 Error_Code
                                       )
{

  Set_Name( (ADDRESS) Handle, New_Name, Error_Code );

  return;

}


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
                                             )
{

  Set_Startable( (ADDRESS) Handle, Error_Code);

  return;

}


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
                                                     )
{

  return Get_Valid_Options( (ADDRESS) Handle, Error_Code);

}


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
//void _Far16 _Pascal _loadds FREE_ENGINE_MEMORY16( ADDRESS _Seg16 Object )
void _Far16 _Pascal _loadds FREE_ENGINE_MEMORY16( void * _Seg16 Object )
{

  API_ENTRY("FREE_ENGINE_MEMORY16")

  Free_Engine_Memory( Object );

  API_EXIT( "FREE_ENGINE_MEMORY16" )

  return;

}

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
BOOLEAN _Far16 _Pascal _loadds REBOOT_REQUIRED16 ( void )
{

  return Reboot_Required();

}


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
BOOLEAN _Far16 _Pascal _loadds CHANGES_PENDING16 ( void )
{

  return Changes_Pending();

}



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
void _Far16 _Pascal _loadds SET_REBOOT_FLAG16( BOOLEAN Reboot, CARDINAL32 * _Seg16 Error_Code )
{

  Set_Reboot_Flag(Reboot, Error_Code );

  return;

}


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
BOOLEAN _Far16 _Pascal _loadds GET_REBOOT_FLAG16( CARDINAL32 * _Seg16 Error_Code )
{

  return Get_Reboot_Flag( Error_Code );

}


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
void _Far16 _Pascal _loadds SET_MIN_INSTALL_SIZE16 ( CARDINAL32  Min_Sectors )
{

  Set_Min_Install_Size( Min_Sectors );

  return;

}


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
void _Far16 _Pascal _loadds SET_FREE_SPACE_THRESHOLD16( CARDINAL32  Min_Sectors )
{

  Set_Free_Space_Threshold( Min_Sectors );

  return;

}


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
void _Far16 _Pascal _loadds READ_SECTORS16 ( CARDINAL32          Drive_Number,
                                             LBA                 Starting_Sector,
                                             CARDINAL32          Sectors_To_Read,
//                                             ADDRESS	  _Seg16 Buffer,
                                             void *	  _Seg16 Buffer,
                                             CARDINAL32 * _Seg16 Error)
{

  Read_Sectors(Drive_Number, Starting_Sector, Sectors_To_Read, Buffer, Error);

  return;

}

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

void _Far16 _Pascal _loadds WRITE_SECTORS16 ( CARDINAL32          Drive_Number,
                                              LBA                 Starting_Sector,
                                              CARDINAL32          Sectors_To_Write,
//                                              ADDRESS      _Seg16 Buffer,
                                              void *      _Seg16 Buffer,
                                              CARDINAL32 * _Seg16 Error)
{

  Write_Sectors(Drive_Number, Starting_Sector, Sectors_To_Write, Buffer, Error);

  return;

}

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
void _Far16 _Pascal _loadds REDISCOVER_PRMS16( CARDINAL32 * _Seg16 Error_Code )
{

  Rediscover_PRMs( Error_Code );

  return;

}


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
                                               BYTE *       _Seg16 UnitID)
{

  return Get_LVM_View( IFSM_Drive_Letter, Drive_Number, Partition_LBA, LVM_Drive_Letter, UnitID );

}


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
void _Far16 _Pascal _loadds START_LOGGING16( char * _Seg16 Filename, CARDINAL32 * _Seg16 Error_Code )
{

  Start_Logging( Filename, Error_Code);

  return;

}


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
void _Far16 _Pascal _loadds STOP_LOGGING16 ( CARDINAL32 * _Seg16 Error_Code )
{

  Stop_Logging( Error_Code );

  return;

}


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
void _Far16 _Pascal _loadds Set_Install_Flags16( CARDINAL32 Install_Flags, CARDINAL32 * _Seg16 Error_Code )
{

  Set_Install_Flags( Install_Flags, Error_Code );

  return;

}


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
void _Far16 _Pascal _loadds Get_Install_Flags16( CARDINAL32 * _Seg16 Install_Flags, CARDINAL32 * _Seg16 Error_Code )
{

  CARDINAL32      Temp;

  API_ENTRY("Get_Install_Flags16")

  Temp = Get_Install_Flags(Error_Code);

  *Install_Flags = Temp;

  API_EXIT( "Get_Install_Flags16" )

  return;

}


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
void _Far16 _Pascal _loadds Get_Partition_Handle16( CARDINAL32          Serial_Number,
                                                    CARDINAL32 * _Seg16 Handle,
                                                    CARDINAL32 * _Seg16 Error_Code )
{

  API_ENTRY("Get_Partition_Handle16")

  *Handle = (CARDINAL32) Get_Partition_Handle( Serial_Number, Error_Code );

  API_EXIT( "Get_Partition_Handle16" )

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
void _System Find_Feature_Given_ID(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Delcare a local variable to access the LVM Plugin Data Record so that we don't have to typecast each time. */
  LVM_Plugin_Data_Record *   Plugin_Data = (LVM_Plugin_Data_Record *) Object;

  /* Delcare a local variable to access our parameters. */
  Find_Feature_Data_Record *  Search_Data = (Find_Feature_Data_Record *) Parameters;

  /* Declare a local variable to make it easy to access the function table for the partition. */
  Plugin_Function_Table_V1 *      Function_Table;

  FUNCTION_ENTRY("Find_Feature_Given_ID")

#ifdef DEBUG

  if ( ( ObjectTag != LVM_PLUGIN_DATA_RECORD_TAG ) || ( ObjectSize != sizeof(LVM_Plugin_Data_Record) ) )
  {

    LOG_ERROR2("Bad Object Tag or Object Size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Find_Feature_Given_ID")

    /* Our list has been corrupted or this function is being used on the wrong list! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

  /* Assume Success. */
  *Error = DLIST_SUCCESS;


  /* Is the current feature the correct one? */
  Function_Table = Plugin_Data->Function_Table;
  if ( Function_Table->Feature_ID->ID == Search_Data->Feature_ID )
  {

    Search_Data->Function_Table = (ADDRESS) Function_Table;

    *Error = DLIST_SEARCH_COMPLETE;

  }
  else
    Search_Data->Function_Table = NULL;

  FUNCTION_EXIT("Find_Feature_Given_ID")

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
static void _System Check_For_Volume_Changes(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data object without having to typecast each time. */
  Volume_Data *    VolumeRecord = (Volume_Data *) Object;

  /* Delcare a local variable so that we can access our parameter without having to typecast every time. */
  BOOLEAN *        Changes_Found = (BOOLEAN *) Parameters;

  FUNCTION_ENTRY("Check_For_Volume_Changes")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Bad Object Tag or Object Size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Check_For_Volume_Changes")

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Does the current volume have changes pending against it? */
  if ( VolumeRecord->ChangesMade )
  {

    *Error = DLIST_SEARCH_COMPLETE;
    *Changes_Found = TRUE;

  }

  FUNCTION_EXIT("Check_For_Volume_Changes")

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
static void _System Update_Disk_Names_In_Signature_Sectors(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  FUNCTION_ENTRY("Update_Disk_Names_In_Signature_Sectors")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof(Partition_Data) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Bad Object Tag or Object Size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Update_Disk_Names_In_Signature_Sectors")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition an LVM partition?  If so, then we must make sure that the disk name in the LVM Signature Sector
       agrees with the current disk name.  The disk name in the LVM Signature Sector is used by the OS2LVM BBR code to
       identify drives with problems.                                                                                    */
    if ( PartitionRecord->Signature_Sector != NULL )
    {

      /* We have an LVM Signature Sector!  Do the disk names agree? */
      if ( strncmp(PartitionRecord->Signature_Sector->Disk_Name, DriveArray[PartitionRecord->Drive_Index].Drive_Name, DISK_NAME_SIZE) != 0 )
      {

        /* We must update the LVM Signature Sector. */
        strncpy(PartitionRecord->Signature_Sector->Disk_Name, DriveArray[PartitionRecord->Drive_Index].Drive_Name, DISK_NAME_SIZE);

        /* Mark the drive as having had changes made so that the updated LVM Signature Sector will be written to disk. */
        DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      }

    }

  }

  FUNCTION_EXIT("Update_Disk_Names_In_Signature_Sectors")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

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
static void APIENTRY Exit_Procedure( CARDINAL32 Error_Code )
{

  /* Now enable PRM Rediscovery. */
  PRM_Rediscovery_Control(TRUE);

  /* Call the next exit procedure in the list. */
  DosExitList(EXLST_EXIT, NULL);

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
static void _System Close_All_Features(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the LVM_Plugin_Data_Record for this feature without having to typecast each time. */
  LVM_Plugin_Data_Record *       Plugin_Data = (LVM_Plugin_Data_Record *) Object;

  /* Declare a local variable to make it easy to access the function table for the partition. */
  Plugin_Function_Table_V1 *      Function_Table;

  FUNCTION_ENTRY("Close_All_Features")

  /* Check the ObjectTag and ObjectSize. */
  if ( ( ObjectTag != LVM_PLUGIN_DATA_RECORD_TAG ) ||
       ( ObjectSize != sizeof(LVM_Plugin_Data_Record) )
     )
  {

    LOG_ERROR2("Bad Object Tag or Object Size!","Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Close_All_Features")

    *Error = DLIST_CORRUPTED;
    return;

  }

  /* We must call the Close_Feature entry point for this feature. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data->Function_Table;
  Function_Table->Close_Feature();

  /* Now we must unload the DLL unless it is a built-in feature like BBR or PassThru. */
  if ( ( Plugin_Data->Plugin_Handle != BBR_PLUGIN_HANDLE ) && ( Plugin_Data->Plugin_Handle != PASS_THRU_PLUGIN_HANDLE ) )
    DosFreeModule(Plugin_Data->Plugin_Handle);

  FUNCTION_EXIT("Close_All_Features")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

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
static BOOLEAN Load_Plugins(CARDINAL32 * Error_Code)
{

  DLIST                           Possible_Plugins;                               /* Used to hold the list of DLLs to load as possible plug-ins. */
  LVM_Plugin_Data_Record          Plugin_Data;                                    /* Used to put LVM Plug-in Feature data into the list of Available Features. */
  char                            DirectorySpecification[] = "c:\\os2\\dll\\lvm"; /* Used to obtain a list of DLLs which might be plug-ins. */
  char                            FileMask[] = "*.dll";                             /* Used to obtain a list of DLLs which might be plug-ins. */
  CARDINAL32                      Local_Error;                                    /* Used on error paths. */

  /* Declare a local variable to make it easy to access the function table. */
  Plugin_Function_Table_V1 *      Function_Table;

  /* Declare a local variable to make it easy to allocate and access the common services structure. */
  LVM_Common_Services_V1 *        Services;

  FUNCTION_ENTRY("Load_Plugins")

  /* Allocate memory for the common services. */
  Services = (LVM_Common_Services_V1 *) malloc( sizeof(LVM_Common_Services_V1) );

  /* We must build the structure containing the common services that will be used by plug-in modules. */
  Services->Volumes = Volumes;
  Services->Aggregates = Aggregates;
  Services->DriveArray = DriveArray;
  Services->DriveCount = DriveCount;
  Services->KillSector = KillSector;
  Services->Boot_Drive_Serial_Number = &Boot_Drive_Serial_Number;
  Services->Merlin_Mode = Merlin_Mode;
  Services->Log_Buffer = Log_Buffer;
  Services->Logging_Enabled = Logging_Enabled;
  Services->Initial_CRC = INITIAL_CRC;
  Services->Allocate = &Allocate_Engine_Memory;
  Services->Deallocate = &Free_Engine_Memory;
  Services->CalculateCRC = &CalculateCRC;
  Services->Create_Serial_Number = &Create_Serial_Number;
  Services->Adjust_Name = &Adjust_Name;
  Services->Valid_Signature_Sector = &Valid_Signature_Sector;
  Services->Create_Unique_Name = &Create_Unique_Name;
  Services->Compute_Reported_Volume_Size = &Compute_Reported_Volume_Size;
  Services->Create_Fake_EBR = &Create_Fake_EBR;
  Services->Create_Fake_Partition_Table_Entry = &Create_Fake_Partition_Table_Entry;
  Services->Convert_To_CHS = &Convert_To_CHS_With_No_Checking;
  Services->Convert_CHS_To_Partition_Table_Format = &Convert_CHS_To_Partition_Table_Format_With_Checking;
//  Services->Convert_Partition_Table_Format_To_CHS = &Convert_Partition_Table_Format_To_CHS_With_Checking;
  Services->Convert_CHS_To_LBA = &Convert_CHS_To_LBA;
  Services->Log_Current_Configuration = &Log_Current_Configuration;
  Services->Write_Log_Buffer = &Write_Log_Buffer;
  Services->Create_Handle = &Create_Handle;
  Services->Destroy_Handle = &Destroy_Handle;
  Services->Translate_Handle = &Translate_Handle;
  Services->CreateList = &CreateList;
  Services->InsertItem = &InsertItem;
  Services->InsertObject = &InsertObject;
  Services->DeleteItem = DeleteItem;
  Services->DeleteAllItems = &DeleteAllItems;
  Services->GetItem = &GetItem;
  Services->GetNextItem = &GetNextItem;
  Services->GetPreviousItem = &GetPreviousItem;
  Services->GetObject = &GetObject;
  Services->GetNextObject = &GetNextObject;
  Services->GetPreviousObject = &GetPreviousObject;
  Services->ExtractItem = &ExtractItem;
  Services->ExtractObject = &ExtractObject;
  Services->ReplaceItem = &ReplaceItem;
  Services->ReplaceObject = ReplaceObject;
  Services->GetTag = &GetTag;
  Services->GetHandle = &GetHandle;
  Services->GetListSize = &GetListSize;
  Services->ListEmpty = &ListEmpty;
  Services->AtEndOfList = &AtEndOfList;
  Services->AtStartOfList = &AtStartOfList;
  Services->DestroyList = &DestroyList;
  Services->NextItem = &NextItem;
  Services->PreviousItem = &PreviousItem;
  Services->GoToStartOfList = &GoToStartOfList;
  Services->GoToEndOfList = &GoToEndOfList;
  Services->GoToSpecifiedItem = &GoToSpecifiedItem;
  Services->SortList = &SortList;
  Services->ForEachItem = &ForEachItem;
  Services->PruneList = &PruneList;
  Services->AppendList = &AppendList;
  Services->TransferItem = &TransferItem;
#ifdef DEBUG
  Services->CheckListIntegrity = &CheckListIntegrity;
#endif

  Common_Services = (ADDRESS) Services;

#ifdef DEBUG

  {

    /* Pass Thru Version Check */
    CARDINAL32   Pass_Thru_Major = 0xFFFF;
    CARDINAL32   Pass_Thru_Minor = 0xFFFF;

    Pass_Thru_Get_Required_LVM_Version(&Pass_Thru_Major, &Pass_Thru_Minor);

    if ( ( Pass_Thru_Major > CURRENT_LVM_MAJOR_VERSION_NUMBER ) ||
         ( ( Pass_Thru_Major == CURRENT_LVM_MAJOR_VERSION_NUMBER ) &&
           ( Pass_Thru_Minor > CURRENT_LVM_MINOR_VERSION_NUMBER )
         )
       )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_VERSION_FAILURE;

      LOG_ERROR("The version number returned by Pass Thru is inappropriate for this release of LVM!")

      FUNCTION_EXIT("Load_Plugins")

      return FALSE;

    }

  }
#endif

  /* Pass Thru is built into LVM.DLL.  However, it looks and acts like a plug-in.  We need to add it to our list of plug-ins
     so that it will be treated as a plug-in.                                                                                 */
  Plugin_Data.Function_Table = (Plugin_Function_Table_V1 *) Pass_Thru_Exchange_Function_Tables(Common_Services);

  /* Open the "Pass Thru" feature. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data.Function_Table;
  Function_Table->Open_Feature(Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Open_Feature failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    /* We must have this plug-in to function successfully!  Abort! */
    return FALSE;

  }

  /* Now that we have the Pass Thru Function table, lets add it to the list of Available Services. */
  Plugin_Data.Plugin_Handle = PASS_THRU_PLUGIN_HANDLE;
  InsertItem(Available_Features, sizeof(LVM_Plugin_Data_Record), &Plugin_Data, LVM_PLUGIN_DATA_RECORD_TAG, NULL, AppendToList, FALSE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("InsertItem failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return FALSE;

  }

  /* Now save the Pass Thru function table for use by the Volume Manager. */
  PassThru_Function_Table = Plugin_Data.Function_Table;

#ifdef DEBUG

  {

    /* BBR Internal Version Check */
    CARDINAL32   BBR_Major = 0xFFFF;
    CARDINAL32   BBR_Minor = 0xFFFF;
    BBR_Get_Required_LVM_Version(&BBR_Major, &BBR_Minor);

    if ( ( BBR_Major > CURRENT_LVM_MAJOR_VERSION_NUMBER ) ||
         ( ( BBR_Major == CURRENT_LVM_MAJOR_VERSION_NUMBER ) &&
           ( BBR_Minor > CURRENT_LVM_MINOR_VERSION_NUMBER )
         )
       )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_VERSION_FAILURE;

      LOG_ERROR("The version number returned by BBR is inappropriate for this release of LVM!")

      FUNCTION_EXIT("Load_Plugins")

      return FALSE;

    }

  }

#endif

  /* BBR is built into LVM.DLL.  However, it looks and acts like a plug-in.  We need to add it to our list of plug-ins
     so that it will be treated as a plug-in.                                                                           */
  Plugin_Data.Function_Table = (Plugin_Function_Table_V1 *) BBR_Exchange_Function_Tables(Common_Services);

  /* Open the "BBR" feature. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data.Function_Table;
  Function_Table->Open_Feature(Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Open_Feature failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    /* We must have this plug-in to function successfully!  Abort! */
    return FALSE;

  }

  /* Now that we have the BBR Function table, lets add it to the list of Available Services. */
  Plugin_Data.Plugin_Handle = BBR_PLUGIN_HANDLE;
  InsertItem(Available_Features, sizeof(LVM_Plugin_Data_Record), &Plugin_Data, LVM_PLUGIN_DATA_RECORD_TAG, NULL, AppendToList, FALSE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("InsertItem failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return FALSE;

  }

  /* Save the BBR Function Table for use by the Volume Manager. */
  BBR_Function_Table = Plugin_Data.Function_Table;

  AddDriveLinking_Plugins(Error_Code); //load psevdo-plugin


  /* Get the list of DLL's that may contain LVM Plug-in modules. */
  DirectorySpecification[0] = GetBootDrive();
  Possible_Plugins = GetDirectoryList( DirectorySpecification, FileMask, Error_Code);
  if ( (Possible_Plugins == NULL ) || ( *Error_Code != DISKIO_NO_ERROR ) )
  {

    LOG_ERROR1("GetDirectoryList failed!", "Error code", *Error_Code)

    /* If the error is not memory related or internal, then we will proceed on the assumption that there are no plugins. */
    switch ( *Error_Code )
    {
      case DISKIO_INTERNAL_ERROR :
      case DISKIO_UNEXPECTED_OS_ERROR :
      case DISKIO_NOT_ENOUGH_HANDLES : /* Fatal error. */
                                       *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
                                       break;
      case DISKIO_OUT_OF_MEMORY : /* Fatal error. */
                                  *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
                                  break;
      case DISKIO_NOFILES_FOUND:
               *Error_Code = LVM_ENGINE_NO_ERROR;
               FUNCTION_EXIT("Load_Plugins")
                 return TRUE;
               break;

      default: /* Assume that there are no plugins. */
               *Error_Code = LVM_ENGINE_NO_ERROR;
               break;
    }

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      if ( Possible_Plugins != NULL )
        DestroyList(&Possible_Plugins, TRUE, &Local_Error);

      FUNCTION_EXIT("Load_Plugins")

      return FALSE;

    }

  }

  /* For each DLL in the list, load the DLL, see if it contains the required entry points, exchange function tables,
     and add it to the list of available features.  If a DLL does not contain the required entry points, or if an error
     occurs trying to exchange function tables, discard the DLL.                                                         */
  ForEachItem( Possible_Plugins, &Process_Plugins, NULL, TRUE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* Is the error a fatal error? */
    switch ( *Error_Code )
    {

      case DLIST_OUT_OF_MEMORY : /* Fatal Error! */
                                 *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

                                 LOG_ERROR("ForEachItem ran out of memory while processing possible plugins!")

                                 break;
      case DLIST_EMPTY : /* We can ignore this error. */
                         *Error_Code = LVM_ENGINE_NO_ERROR;
                         break;
      default: /* Fatal error! */
               *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

               LOG_ERROR1("ForEachItem failed!", "Error code", *Error_Code)

               break;
    }

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      DestroyList(&Possible_Plugins, TRUE, &Local_Error);

      FUNCTION_EXIT("Load_Plugins")

      return FALSE;

    }

  }

  /* Free the memory occupied by the Possible_Plugins list. */
  DestroyList(&Possible_Plugins, TRUE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("DestroyList failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return FALSE;

  }


  /* Now we will sort the list of plug-ins by what class they belong to.  Partition Class plug-ins will come first,
     followed by aggregate class plug-ins, with volume class plug-ins being last.                                    */
  SortList(Available_Features, &Sort_Plugins, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("SortList failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return FALSE;

  }

  FUNCTION_EXIT("Load_Plugins")

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return TRUE;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: AddDriveLinking_Plugins                          */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:  None.                                            */
/*   Notes:  Добавляет псевдоплугин из Drive_Linking_Feature.c       */
/*********************************************************************/
extern ADDRESS _System Exchange_Function_Tables( ADDRESS Common_Services );

static BOOLEAN  AddDriveLinking_Plugins( CARDINAL32 * Error_Code)
{
  LVM_Plugin_Data_Record     Plugin_Data;                       /* Used to put LVM Plug-in Feature data into the list of Available Features. */
  APIRET                     ReturnCode;                        /* Used with the OS/2 APIs to load and access a DLL. */
  CARDINAL32                 Required_LVM_Major_Version;        /* Used to determine if a plug-in module will work with this version of LVM. */
  CARDINAL32                 Required_LVM_Minor_Version;        /* Used to determine if a plug-in module will work with this version of LVM. */
  LVM_Plugin_DLL_Interface   EntryPoints;                       /* Used to hold the entry points to the DLL that was just loaded. */
  /* Declare a local variable to make it easy to access the function table for the partition. */
  Plugin_Function_Table_V1 * Function_Table;
/////////////////////////////////////////////////////////

  /* BBR is built into LVM.DLL.  However, it looks and acts like a plug-in.  We need to add it to our list of plug-ins
     so that it will be treated as a plug-in.                                                                           */
  Plugin_Data.Function_Table = (Plugin_Function_Table_V1 *) Exchange_Function_Tables(Common_Services);

  /* Open the  feature. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data.Function_Table;
  Function_Table->Open_Feature(Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {
    LOG_ERROR1("Open_Feature failed!", "Error code", *Error_Code)
    FUNCTION_EXIT("Load_Plugins")
    return FALSE;

  }

  /* Now that we have the BBR Function table, lets add it to the list of Available Services. */
  Plugin_Data.Plugin_Handle = BBR_PLUGIN_HANDLE;
  InsertItem(Available_Features, sizeof(LVM_Plugin_Data_Record), &Plugin_Data, LVM_PLUGIN_DATA_RECORD_TAG, NULL, AppendToList, FALSE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LOG_ERROR1("InsertItem failed!", "Error code", *Error_Code)

    FUNCTION_EXIT("Load_Plugins")

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return FALSE;

  }
  /* Indicate success and leave. */
  *Error_Code = DLIST_SUCCESS;

  return TRUE;
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
static void _System Process_Plugins(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

#define ERROR_MESSAGE_SIZE 128
  /* Establish access to the name of the DLL we need to load and process. */
  char *                     Plugin_Name = (char * ) Object;

  LVM_Plugin_Data_Record     Plugin_Data;                       /* Used to put LVM Plug-in Feature data into the list of Available Features. */
  APIRET                     ReturnCode;                        /* Used with the OS/2 APIs to load and access a DLL. */
  char                       ErrorMessage[ERROR_MESSAGE_SIZE];  /* Used with DosLoadModule to hold an error message if DosLoadModule fails. */
  CARDINAL32                 ExtraFileHandlesNeeded;            /* Used if we run out of file handles while trying to load a DLL. */
  CARDINAL32                 CurrentMaxFileHandles;             /* Used if we run out of file handles while trying to load a DLL. */
  LVM_Plugin_DLL_Interface   EntryPoints;                       /* Used to hold the entry points to the DLL that was just loaded. */
  CARDINAL32                 Required_LVM_Major_Version;        /* Used to determine if a plug-in module will work with this version of LVM. */
  CARDINAL32                 Required_LVM_Minor_Version;        /* Used to determine if a plug-in module will work with this version of LVM. */

  /* Declare a local variable to make it easy to access the function table for the partition. */
  Plugin_Function_Table_V1 * Function_Table;

  FUNCTION_ENTRY("Process_Plugins")

#ifdef DEBUG

#ifdef PARANOID

  assert( ObjectTag == FILENAME_TAG );

#else

  /* Is Object what we think it should be? */
  if ( ObjectTag != FILENAME_TAG )
  {

    LOG_ERROR1("Unexpected object tag!", "Object Tag", ObjectTag)

    FUNCTION_EXIT("Process_Plugins")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type char. */

  do
  {

    if ( Logging_Enabled )
    {

      sprintf(Log_Buffer,"Attempting to load plugin %s", Plugin_Name);
      Write_Log_Buffer();

    }

    /* Call DosLoadModule to load the DLL. */
    ReturnCode = DosLoadModule(&ErrorMessage[0], ERROR_MESSAGE_SIZE, Plugin_Name, &(Plugin_Data.Plugin_Handle) );

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

                                         LOG_ERROR("Unable to increase the number of filesystem handles available to this process!")

                                         FUNCTION_EXIT("Process_Plugins")

                                         /* Something is wrong.  We must be out of memory or something like that. */
                                         *Error = DLIST_OUT_OF_MEMORY;
                                         return;

                                       }
                                       else
                                         ReturnCode = ERROR_TOO_MANY_OPEN_FILES;  /* Restore the original value of ReturnCode. */

                                       break;
      case ERROR_NOT_ENOUGH_MEMORY : /* We are out of memory!  Abort. */

                                     LOG_ERROR("DosLoadModule failed due to a lack of available memory!")

                                     FUNCTION_EXIT("Process_Plugins")

                                     *Error = DLIST_OUT_OF_MEMORY;
                                     return;
                                     break;
      default: /* We can not use the current DLL.  Proceed to the next one. */

               LOG_ERROR1("The current plugin is not usable!","Error code", ReturnCode)

               FUNCTION_EXIT("Process_Plugins")

               *Error = DLIST_SUCCESS;
               return;

               break;  /* Keep the compiler happy. */

    }

  } while ( ReturnCode != NO_ERROR );


  LOG_EVENT("Getting the entry points for the plugin.")

  /* Get the entry points for the two functions we need. */
  ReturnCode = DosQueryProcAddr(Plugin_Data.Plugin_Handle, 0, "Get_Required_LVM_Version", ( PFN * ) &EntryPoints.Get_Required_LVM_Version );

  /* Did we succeed? */
  if ( ReturnCode != NO_ERROR )
  {

    LOG_EVENT("Entry points not found!  The plugin is not usable!")

    /* The entry point was not found.  We will assume that this DLL is not a plug-in module. */
    ReturnCode = DosFreeModule(Plugin_Data.Plugin_Handle);

    FUNCTION_EXIT("Process_Plugins")

    /* We will report success and move on to the next potential plug-in module. */
    *Error = DLIST_SUCCESS;
    return;

  }

  /* Get the next entry point. */
  ReturnCode = DosQueryProcAddr(Plugin_Data.Plugin_Handle, 0, "Exchange_Function_Tables", ( PFN* ) &EntryPoints.Exchange_Function_Tables );

  /* Did we succeed? */
  if ( ReturnCode != NO_ERROR )
  {

    LOG_EVENT("Entry points not found!  The plugin is not usable!")

    /* The entry point was not found.  We will assume that this DLL is not a plug-in module. */
    ReturnCode = DosFreeModule(Plugin_Data.Plugin_Handle);

    FUNCTION_EXIT("Process_Plugins")

    /* We will report success and move on to the next potential plug-in module. */
    *Error = DLIST_SUCCESS;
    return;

  }

  LOG_EVENT("Getting the required LVM Version for the plugin.")

  /* Get the version of LVM that the plug-in was made for. */
  EntryPoints.Get_Required_LVM_Version(&Required_LVM_Major_Version, &Required_LVM_Minor_Version);

  /* Can this version of LVM use this plug-in module? */
  if ( ( Required_LVM_Major_Version > CURRENT_LVM_MAJOR_VERSION_NUMBER ) ||
       ( ( Required_LVM_Major_Version == CURRENT_LVM_MAJOR_VERSION_NUMBER ) &&
         ( Required_LVM_Minor_Version > CURRENT_LVM_MINOR_VERSION_NUMBER )
       )
     )
  {

    LOG_EVENT("The plugin requires a newer version of LVM than this one, so we can not use it.")

    /* The plug-in requires a newer version of LVM, so we can't use it.  Unload the DLL and proceed to the next one. */
    ReturnCode = DosFreeModule(Plugin_Data.Plugin_Handle);

    FUNCTION_EXIT("Process_Plugins")

    /* We will report success and move on to the next potential plug-in module. */
    *Error = DLIST_SUCCESS;
    return;

  }

  LOG_EVENT("Exchanging function tables with the plugin.")

  /* Since the version numbers indicate that we can use this plug-in, lets exchange function tables. */
  Plugin_Data.Function_Table = EntryPoints.Exchange_Function_Tables(Common_Services);

  LOG_EVENT("Attempting to open the plugin.")

  /* Now call the Open_Feature function for the plug-in.  This must complete successfully in order for the plug-in to be used. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data.Function_Table;
  Function_Table->Open_Feature(Error);

  /* Did we succeed? */
  if ( *Error != LVM_ENGINE_NO_ERROR )
  {

    LOG_ERROR1("Open_Feature failed!", "Error code", *Error)

    /* The plug-in did not "open" successfully so we can not use it. */
    ReturnCode = DosFreeModule(Plugin_Data.Plugin_Handle);

    FUNCTION_EXIT("Process_Plugins")

    /* We will report success and move on to the next potential plug-in module. */
    *Error = DLIST_SUCCESS;
    return;

  }

  LOG_EVENT("Adding the plugin to the list of available features.")

  /* Now that we have exchanged function tables with the plug-in, we can add it to the list of Available_Features. */
  InsertItem(Available_Features, sizeof(LVM_Plugin_Data_Record), &Plugin_Data, LVM_PLUGIN_DATA_RECORD_TAG, NULL, AppendToList, FALSE, Error);

  /* Did we succeed? */
  if ( *Error != DLIST_SUCCESS )
  {

    LOG_ERROR1("InsertItem failed!", "Error code", *Error)

    /* Since this operation failed, we can not continue.  Abort.  *Error_Code was already set by InsertItem, so we don't need to set it here. */

    /* Free the module since we can't use it and we can't put it into the Available_Features list. */
    ReturnCode = DosFreeModule(Plugin_Data.Plugin_Handle);

    FUNCTION_EXIT("Process_Plugins")

    return;

  }

  LOG_EVENT("Plugin processed successfully.")

  FUNCTION_EXIT("Process_Plugins")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  return;

#undef ERROR_MESSAGE_SIZE

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
void _System Load_Feature_Information_Array(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  LVM_Plugin_Data_Record  *        Plugin_Data = (LVM_Plugin_Data_Record *) Object;
  Feature_Information_Array *      Feature_Information = (Feature_Information_Array *) Parameters;
  Plugin_Function_Table_V1 *       Function_Table;

  FUNCTION_ENTRY("Load_Feature_Information_Array")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == LVM_PLUGIN_DATA_RECORD_TAG ) && ( ObjectSize == sizeof( LVM_Plugin_Data_Record ) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != LVM_PLUGIN_DATA_RECORD_TAG ) || ( ObjectSize != sizeof( LVM_Plugin_Data_Record ) ) )
  {

    LOG_ERROR2("Invalid Object Tag or Object Size encountered", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Load_Feature_Information_Array")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type LVM_Plugin_Data_Record. */

  /* Extract the Function_Table. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data->Function_Table;

  /* Is this record for PassThru or BBR?  If so, we will ignore it.  We will also ignore
     any features which do not support the current interface.                             */
  if ( (Function_Table->Feature_ID->ID != PASS_THRU_FEATURE_ID ) &&
       (Function_Table->Feature_ID->ID != BBR_FEATURE_ID ) &&
       (Function_Table->Feature_ID->Interface_Support[Interface_In_Use].Interface_Supported )
     )
  {

    Feature_Information->Feature_Data[Feature_Information->Count] = *(Function_Table->Feature_ID);
    Feature_Information->Count++;

  }

  FUNCTION_EXIT("Load_Feature_Information_Array")

  /* Indicate success */
  *Error = DLIST_SUCCESS;

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
static void _System Fill_Handle_Array(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  Partition_Data *                 PartitionRecord = (Partition_Data *) Object;
  LVM_Handle_Array_Record *        LVM_Handle_Array = (LVM_Handle_Array_Record *) Parameters;

  FUNCTION_ENTRY("Fill_Handle_Array")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == PARTITION_DATA_TAG ) && ( ObjectSize == sizeof( Partition_Data ) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof( Partition_Data ) ) )
  {

    LOG_ERROR2("Invalid Object Tag or Object Size encountered", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Fill_Handle_Array")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Extract the external handle for the partition/aggregate and return it. */
  LVM_Handle_Array->Handles[LVM_Handle_Array->Count] = PartitionRecord->External_Handle;

  /* Update the number of handles in the array. */
  LVM_Handle_Array->Count += 1;

  FUNCTION_EXIT("Fill_Handle_Array")

  /* Indicate success */
  *Error = DLIST_SUCCESS;

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
static INTEGER32 _System Sort_Plugins(ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag,CARDINAL32 * Error_Code)
{

  LVM_Plugin_Data_Record  *        Plugin_Data = NULL;
  CARDINAL32                       Object1_Score = 0;
  CARDINAL32                       Object2_Score = 0;
  CARDINAL32                       Score;
  Plugin_Function_Table_V1 *       Function_Table;
  Feature_ID_Data *                Feature_Data;
  BOOLEAN                          Scoring_Complete = FALSE;

  FUNCTION_ENTRY("Sort_Plugins")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( Object1Tag == LVM_PLUGIN_DATA_RECORD_TAG ) && ( Object2Tag == LVM_PLUGIN_DATA_RECORD_TAG ) );

#else

  /* Is Object what we think it should be? */
  if ( ( Object1Tag != LVM_PLUGIN_DATA_RECORD_TAG ) || ( Object2Tag != LVM_PLUGIN_DATA_RECORD_TAG ) )
  {

    LOG_ERROR2("Invalid Object Tag detected!", "Object Tag 1", Object1Tag, "Object Tag 2", Object2Tag)

    FUNCTION_EXIT("Sort_Plugins")

    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    return 0;

  }

#endif

#endif

  /* Well, Object1 and Object2 have the correct TAG so we will assume that they point to an item of type LVM_Plugin_Data_Record. */

  while ( ! Scoring_Complete )
  {

    /* Get the plug-in data for the object we are going to process.   The first time through the loop
       Plugin_Data will be NULL, so we will load it with Object1 and process Object1.  The second time
       through the while loop, Plugin_Data will not be NULL, so we will load it with Object2 and
       process Object2.  We should exit the while loop after processing Object2.                        */
    if ( Plugin_Data == NULL )
    {

      Plugin_Data = (LVM_Plugin_Data_Record *) Object1;

    }
    else
    {

      Plugin_Data = (LVM_Plugin_Data_Record *) Object2;

    }

    /* Extract the Function_Table for the current object. */
    Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data->Function_Table;

    /* Extract the Feature_ID_Data. */
    Feature_Data = Function_Table->Feature_ID;

    /* Calculate the "score" for the current object. */
    Score = 0;

    switch ( Feature_Data->Preferred_Class )
    {

      case Partition_Class : Score += 1000;
                             Score += Feature_Data->ClassData[Partition_Class].Weight_Factor;
                             break;
      case Aggregate_Class : Score += 2000;
                             Score += Feature_Data->ClassData[Aggregate_Class].Weight_Factor;
                             break;
      case Volume_Class : Score += 3000;
                          Score += Feature_Data->ClassData[Volume_Class].Weight_Factor;
                          break;
      default :
                LOG_ERROR1("Invalid Feature Class Encountered!", "Feature Class", Feature_Data->Preferred_Class)

                FUNCTION_EXIT("Sort_Plugins")

                *Error_Code = DLIST_CORRUPTED;

                return 0;

                break; /* Keep the compiler happy. */
    }

    /* Save the score for the object. */
    if ( (ADDRESS) Plugin_Data == Object1 )
    {

      Object1_Score = Score;

    }
    else
    {

      Object2_Score = Score;
      Scoring_Complete = TRUE;

    }

  }

  FUNCTION_EXIT("Sort_Plugins")

  /* Indicate success */
  *Error_Code = DLIST_SUCCESS;

  /* Set the return value. */
  if ( Object1_Score == Object2_Score )
    return 0;

  if ( Object1_Score < Object2_Score )
    return -1;

  return 1;

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
INTEGER32 _System Sort_By_Feature_Sequence_Number( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error)
{

  /* Declare local variables so that we can access the Feature_Application_Data objects without having to typecast each time. */
  Feature_Application_Data *    Feature1 = (Feature_Application_Data *) Object1;
  Feature_Application_Data *    Feature2 = (Feature_Application_Data *) Object2;

  FUNCTION_ENTRY("Sort_By_Feature_Sequence_Number")

#ifdef DEBUG

  /* Are Object1 and Object2 what we think it should be? */
  if ( ( Object1Tag != FEATURE_APPLICATION_DATA_TAG ) || ( Object2Tag != FEATURE_APPLICATION_DATA_TAG ) )
  {


#ifdef PARANOID

    assert(0);

#endif

    LOG_ERROR2("Invalid Object Tag detected!", "Object Tag1", Object1Tag, "Object Tag2", Object2Tag)

    FUNCTION_EXIT("Sort_By_Feature_Sequence_Number")

    /* We have a TAG that is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return 0;

  }

#endif

  /* Assume success. */
  *Error = DLIST_SUCCESS;

  /* Well, the Object Tags look OK, so lets get to it. */

  /* Is Feature1 < Feature2? */
  if ( Feature1->Feature_Sequence_Number < Feature2->Feature_Sequence_Number )
  {

    FUNCTION_EXIT("Sort_By_Feature_Sequence_Number")

    return -1;

  }

  /* Is Feature1 > Feature2? */
  if ( Feature1->Feature_Sequence_Number > Feature2->Feature_Sequence_Number )
  {

    FUNCTION_EXIT("Sort_By_Feature_Sequence_Number")

    return 1;

  }

  /* They must be equal! */

  FUNCTION_EXIT("Sort_By_Feature_Sequence_Number")

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
void _System Load_Feature_Information_Array2(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  Feature_Application_Data   *     Plugin_Data = (Feature_Application_Data *) Object;
  Feature_Information_Array *      Feature_Information = (Feature_Information_Array *) Parameters;
  Plugin_Function_Table_V1 *       Function_Table;

  FUNCTION_ENTRY("Load_Feature_Information_Array2")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == FEATURE_APPLICATION_DATA_TAG ) && ( ObjectSize == sizeof( Feature_Application_Data ) ) );

#else

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != FEATURE_APPLICATION_DATA_TAG ) || ( ObjectSize != sizeof( Feature_Application_Data ) ) )
  {

    LOG_ERROR2("Invalid Object Tag or Object Size encountered", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Load_Feature_Information_Array2")

    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type LVM_Plugin_Data_Record. */

  /* Extract the Function_Table. */
  Function_Table = (Plugin_Function_Table_V1 *) Plugin_Data->Function_Table;

  /* Is this record for PassThru or BBR?  If so, we will ignore it. */
  if ( (Function_Table->Feature_ID->ID != PASS_THRU_FEATURE_ID ) &&
       (Function_Table->Feature_ID->ID != BBR_FEATURE_ID )
     )
  {

    Feature_Information->Feature_Data[Feature_Information->Count] = *(Function_Table->Feature_ID);
    Feature_Information->Count++;

  }

  FUNCTION_EXIT("Load_Feature_Information_Array2")

  /* Indicate success */
  *Error = DLIST_SUCCESS;

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
void _System Find_Existing_Feature(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare local variables to access our Object and Parameters without having to typecast each time. */
  Feature_Application_Data *   Feature_App_Data = (Feature_Application_Data *) Object;
  Feature_Search_Data *        Search_Data = (Feature_Search_Data *) Parameters;

  FUNCTION_ENTRY("Find_Existing_Feature")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == FEATURE_APPLICATION_DATA_TAG ) && ( ObjectSize == sizeof( Feature_Application_Data ) ) );

#else

  if ( ( ObjectTag != FEATURE_APPLICATION_DATA_TAG ) || ( ObjectSize != sizeof( Feature_Application_Data ) ) )
  {

    LOG_ERROR2("Invalid Object Tag or Object Size encountered", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Find_Existing_Feature")

    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* It appears that we have a valid Feature_Application_Data record.  Lets see if it is what we are looking for. */
  if ( Feature_App_Data->Function_Table->Feature_ID->ID == Search_Data->Feature_ID )
  {

    /* We found a match!  */
    Search_Data->Found = TRUE;

    *Error = DLIST_SEARCH_COMPLETE;

  }
  else
    *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Find_Existing_Feature")

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
void  _System Create_Fake_EBR( Extended_Boot_Record ** New_EBR, CARDINAL32 * New_EBR_Size, LVM_Signature_Sector * Signature_Sector, CARDINAL32 * Error_Code)
{

  Extended_Boot_Record *  EBR_Data;

  FUNCTION_ENTRY("Create_Fake_EBR")

  /* Set the size of the Fake EBR.  This includes the entire EBR track! */
  *New_EBR_Size = SYNTHETIC_SECTORS_PER_TRACK;

  /* Allocate the memory for the track containing the fake EBR. */
  EBR_Data = (Extended_Boot_Record *) malloc(SYNTHETIC_SECTORS_PER_TRACK * BYTES_PER_SECTOR);
  if ( EBR_Data == NULL )
  {

    LOG_ERROR("Unable to allocate the memory to hold an EBR!  Out of memory!")

    FUNCTION_EXIT("Create_Fake_EBR")

    /* We are out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    return;

  }

  *New_EBR = EBR_Data;

  /* Clear New_EBR. */
  memset(EBR_Data, 0, SYNTHETIC_SECTORS_PER_TRACK * BYTES_PER_SECTOR);

  /* Allocate space for the fake EBR track within the LVM Data Area. */
  Signature_Sector->LVM_Reserved_Sector_Count += SYNTHETIC_SECTORS_PER_TRACK;
  Signature_Sector->Partition_Size_To_Report_To_User = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;
  Signature_Sector->Fake_EBR_Location = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;
  Signature_Sector->Fake_EBR_Allocated = TRUE;

  /* Now create the partition table entry for the volume. */
  Create_Fake_Partition_Table_Entry( &( EBR_Data->Partition_Table[0] ), Signature_Sector->Partition_Size_To_Report_To_User );

  /* Now set the EBR signature. */
  EBR_Data->Signature = MBR_EBR_SIGNATURE;

  FUNCTION_EXIT("Create_Fake_EBR")

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

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
void  _System Create_Fake_Partition_Table_Entry( Partition_Record * Partition_Table_Entry, CARDINAL32 Partition_Size)
{

  FUNCTION_ENTRY("Create_Fake_Partition_Table_Entry")

  /* Clear partition table entry. */
  memset(Partition_Table_Entry, 0, sizeof(Partition_Record) );

  /* Ensure that the sector offset is correct. */
  Partition_Table_Entry->Sector_Offset = SYNTHETIC_SECTORS_PER_TRACK;

  /* Set the sector count based upon what will be reported through the GetDevParms IOCTLs. */
  Partition_Table_Entry->Sector_Count = Compute_Reported_Volume_Size( Partition_Size );

  /* Set the CHS values to their maximum. */
  Partition_Table_Entry->Starting_Head = 254;
  Partition_Table_Entry->Starting_Sector = 255;
  Partition_Table_Entry->Starting_Cylinder = 255;
  Partition_Table_Entry->Ending_Head = 254;
  Partition_Table_Entry->Ending_Sector = 255;
  Partition_Table_Entry->Ending_Cylinder = 255;

  /* Ensure that the "active" flag is set. */
  Partition_Table_Entry->Boot_Indicator = 0x80;

  /* Set the Format Indicator to 0x07 (IFS) */
  Partition_Table_Entry->Format_Indicator = IFS_INDICATOR;

  FUNCTION_EXIT("Create_Fake_Partition_Table_Entry")

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
CARDINAL32 _System Compute_Reported_Volume_Size( CARDINAL32 Volume_Size )
{

  CARDINAL32   SectorsPerTrack;
  CARDINAL32   Heads;
  CARDINAL32   Cylinders;
  BOOLEAN      done;

  FUNCTION_ENTRY("Compute_Reported_Volume_Size")

  /* Initialize SectorsPerTrack to value we use when computing geometries for LVM Volumes. */
  SectorsPerTrack = SYNTHETIC_SECTORS_PER_TRACK;

  /* The number of hidden sectors on an LVM volume is always SYNTHETIC_SECTORS_PER_TRACK.  This is due to the
     fake EBR used.  When calculating the number of cylinders that will be reported by the GetDevParms IOCTL,
     we must add in the hidden sectors to the volume size as the reported geometry covers both the volume and
     the EBR of the first partition in the volume.                                                             */
  Volume_Size += SYNTHETIC_SECTORS_PER_TRACK;

  /* Compute initial Heads value */
  Heads = Volume_Size / SectorsPerTrack;

  /* We would like to keep the # of heads to 255, if possible. */
  if ( Heads > 255 )
  {
    Heads = 255;
  }

  /* The number of cylinders must be below 65535.  We will increase the number of
     heads until the number of cylinders falls below 65535.                        */
  done = FALSE;
  while ( !done )
  {

    /* Compute Cylinders */
    Cylinders = Volume_Size / ( Heads * SectorsPerTrack );

    /* Is Cylinders acceptable? */
    if ( Cylinders <= 65535 )
    {
      done = TRUE;
    }
    else
    {
      Heads = Heads << 1;
    }

  }

  FUNCTION_EXIT("Compute_Reported_Volume_Size")

  /* Return the number of sectors that corresponds to our calculated geometry, less the number of hidden sectors. */
  return (Cylinders * Heads * SectorsPerTrack) - SYNTHETIC_SECTORS_PER_TRACK;

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
static void _System Find_Feature_And_Parse(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare local variables to access our Object and Parameters without having to typecast each time. */
  LVM_Plugin_Data_Record *     Feature_App_Data = (LVM_Plugin_Data_Record *) Object;
  Find_And_Parse_Record *      Parse_Data = (Find_And_Parse_Record *) Parameters;
  Plugin_Function_Table_V1 *   Function_Table;

  FUNCTION_ENTRY("Find_Feature_And_Parse")

#ifdef DEBUG

#ifdef PARANOID

  assert( ( ObjectTag == LVM_PLUGIN_DATA_RECORD_TAG ) && ( ObjectSize == sizeof( LVM_Plugin_Data_Record ) ) );

#else

  if ( ( ObjectTag != LVM_PLUGIN_DATA_RECORD_TAG ) || ( ObjectSize != sizeof( LVM_Plugin_Data_Record ) ) )
  {

    LOG_ERROR2("Invalid Object Tag or Object Size encountered", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Find_Feature_And_Parse")

    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

#endif

  /* It appears that we have a valid Feature_Application_Data record.  Lets see if it is what we are looking for. */
  Function_Table = Feature_App_Data->Function_Table;
  if ( Function_Table->Feature_ID->ID == Parse_Data->Feature_Data->Feature_ID )
  {

    /* We found a match!  */
    Parse_Data->Feature_Found = TRUE;
    *Error = DLIST_SEARCH_COMPLETE;

    /* Now we must call the parsing function associated with this feature. */

    /* Call the feature's parsing function. */
    Function_Table->ParseCommandLineArguments(Parse_Data->Tokens, &(Parse_Data->Feature_Data->Actual_Class), &(Parse_Data->Feature_Data->Init_Data), Parse_Data->Error_Message, Parse_Data->Error_Code );

  }
  else
    *Error = DLIST_SUCCESS;

  FUNCTION_EXIT("Find_Feature_And_Parse")

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
static void _System Free_Expansion_DLLs(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the LVM_Plugin_Data_Record for this feature without having to typecast each time. */
  LVM_Expansion_DLL_Record *       Expansion_DLL_Data = (LVM_Expansion_DLL_Record *) Object;


  FUNCTION_ENTRY("Free_Expansion_DLLs")

  /* Check the ObjectTag and ObjectSize. */
  if ( ( ObjectTag != LVM_EXPANSION_DLL_TAG ) ||
       ( ObjectSize != sizeof(LVM_Expansion_DLL_Record) )
     )
  {

    LOG_ERROR2("Invalid Object Tag or Object Size encountered", "Object Tag", ObjectTag, "Object Size", ObjectSize)

    FUNCTION_EXIT("Free_Expansion_DLLs")

    *Error = DLIST_CORRUPTED;
    return;

  }

  /* Since the ObjectTag and ObjectSize check out, we will proceed. */

  /* Now we must unload the DLL. */
  DosFreeModule(Expansion_DLL_Data->Expansion_DLL_Handle);

  FUNCTION_EXIT("Free_Expansion_DLLs")

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  return;

}

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
                                                    CARDINAL32 * _Seg16 Error_Code )
{
  Export_Configuration(Filename, Error_Code );

  return ;
}

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
void _System Export_Configuration( char * Filename, CARDINAL32 * Error_Code )
{
}
