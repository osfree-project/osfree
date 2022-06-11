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
 * Module: BootManager.c
 */

/*
 * Change History:
 *
 */

/*
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


/*--------------------------------------------------
 * Necessary include files
 --------------------------------------------------*/

#include <stdlib.h>   /* malloc, free */
#include <stdio.h>    /* sprintf */
#include <string.h>   /* strlen */

#include "engine.h"   /* Included for access to the global types and variables. */

#define NEED_BYTE_DEFINED
#include "lvm_gbls.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */

#include "dlist.h"    /*  */
#include "diskio.h"   /*  */

#include "lvm_cons.h"   /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */

#define NEED_BYTE_DEFINED
#include "lvm_intr.h"   /* */

#include "lvm_hand.h"    /* Create_Handle, Destroy_Handle, Translate_Handle */

#include "Partition_Manager.h" /* Get_Partition_Options */

#include "mbb.h"               /* Boot Manager. */

#include "extboot.h"

#ifdef DEBUG

#ifdef PARANOID

#include <assert.h>   /* assert */

#endif

#endif


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/


/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/
typedef struct _Find_Boot_Manager_Control_Data{
                                                Partition_Data *  Boot_Manager_Partition;
                                                BOOLEAN           Find_Active_Boot_Manager;
                                                BOOLEAN           Non_Current_Boot_Manager;
                                              } Find_Boot_Manager_Control_Data;

typedef Alias_Table_Entry Alias_Table[24][4];


/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static Boot_Path_Record * BM_Options = NULL;                              /* Used to point to the copy in memory of the Boot Manager options. */
static Alias_Table        BM_Alias_Table;                                 /* Used to migrate old Boot Manager Menu data to the new LVM format. */
static BOOLEAN            Boot_Manager_Found = FALSE;                     /* Set to TRUE if Boot Manager is found on drives 1 or 2. */
static BYTE               Sector_Buffer[BYTES_PER_SECTOR];                /* Used when looking for the Boot Manager Signature. */
static BOOLEAN            Upgrade_Boot_Manager = FALSE;                   /* Set to TRUE if the version of Boot Manager found is less than the current version. */
static BYTE               Boot_Manager_Options_Sector[BYTES_PER_SECTOR];  /* Used to hold the Boot Manager BootPath sector. */
static BOOLEAN            Boot_Manager_Options_Changed = FALSE;           /* Used to track whether or not there were changes to Boot Manager's startup options. */

/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
static void _System Find_Boot_Manager(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Migrate_Old_BM_Menu (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Build_Alias_Table (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Remove_From_Boot_Manager_Menu(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Remove_Boot_Manager_Menu_Flag(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Get_Menu_Items(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Make_Primaries_Inactive(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Find_Free_Space_For_BM(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Find_Matching_Partition_Name(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void _System Find_Matching_Volume_Name(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);

/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


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
BOOLEAN _System Boot_Manager_Is_Installed( BOOLEAN * Active, CARDINAL32 * Error_Code)
{

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return FALSE;

  }

  /* Indicate success */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Set *Active for return to caller. */
  *Active = Boot_Manager_Active;

  return Boot_Manager_Found;

}


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
void Add_To_Boot_Manager ( ADDRESS Handle, CARDINAL32 * Error_Code )
{

  Partition_Data   * PartitionRecord; /* Used to point to the Partition_Data structure we are manipulating. */
  Volume_Data      * VolumeRecord;    /* Used to point to the Volume_Data structure we are manipulating. */
  ADDRESS            Object;          /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                ObjectTag;       /* Used when translating the Partition_Handle into a Partition_Data structure. */


  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Indicate that Boot Manager has not been installed yet! */
    *Error_Code = LVM_ENGINE_BOOT_MANAGER_NOT_FOUND;

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {
    case PARTITION_DATA_TAG : /* We have a partition here.  If it is part of a volume, then this is incorrect. */

                              /* Establish access to the Partition_Data structure for the partition. */
                              PartitionRecord = (Partition_Data *) Object;

                              /* Does this Partition_Data structure represent a partition? */
                              if ( PartitionRecord->Partition_Type != Partition )
                              {

                                /* Since this does not represent a partition, we can not add it to the Boot Manager Menu. */
                                *Error_Code = LVM_ENGINE_BAD_HANDLE;

                                return;

                              }

                              /* Is this partition part of a volume? */
                              if ( PartitionRecord->Volume_Handle != NULL )
                              {

                                /* Since this is part of a volume, we can not add it to the Boot Manager Menu. */
                                *Error_Code = LVM_ENGINE_BAD_HANDLE;

                                return;

                              }

                              /* Is this partition below the 1024 cylinder limit, if the limit applies? */
                              if ( DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies &&
                                   ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) > DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
                                 )
                              {

                                /* This partition is not bootable so it can not be put onto the Boot Manager menu. */
                                *Error_Code = LVM_ENGINE_1024_CYLINDER_LIMIT;

                                return;

                              }

                              /* Since this partition is not part of a volume, we just need to adjust its DLA_Table_Entry. */
                              PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = TRUE;

                              /* Set the ChangesMade flag for the drive containing the partition. */
                              DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

                              break;
    case VOLUME_DATA_TAG : /* We must add the Volume from the Boot Manager Menu. */

                           /* Establish access to the data for the volume. */
                           VolumeRecord = (Volume_Data *) Object;

                           /* If the volume can not be changed, abort. */
                           if ( ! VolumeRecord->Can_Be_Altered )
                           {

                             /* We can not perform this operation on this volume. */
                             *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                             return;

                           }

                           /* Only compatibility volumes are bootable.  If this is not a compatibility volume, then it is not
                              bootable and can not be on the Boot Manager Menu.                                                */
                           if ( ! VolumeRecord->Compatibility_Volume )
                           {

                             /* This volume is not eligible to be on the Boot Manager menu. */
                             *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

                             return;

                           }

                           /* To add the volume to the Boot Manager Menu, we need to set the entry in the DLA Table
                              of the first partition in the volume which indicates that this volume is on the Boot Manager Menu. */
                           PartitionRecord = VolumeRecord->Partition;

                           /* Is this partition below the 1024 cylinder limit, if the limit applies? */
                           if ( DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies &&
                                ( ( PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size ) > DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
                              )
                           {

                             /* This partition is not bootable so it can not be put onto the Boot Manager menu. */
                             *Error_Code = LVM_ENGINE_1024_CYLINDER_LIMIT;

                             return;

                           }

                           /* Now adjust the partition's DLA_Table_Entry. */
                           PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = TRUE;

                           /* Now set the corresponding flag in the VolumeRecord. */
                           VolumeRecord->On_Boot_Manager_Menu = TRUE;

                           /* Set the ChangesMade flag for the drive containing the partition. */
                           DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

                           break;

    default: /* Bad Handle! */
             *Error_Code = LVM_ENGINE_BAD_HANDLE;
             break;
  }

  /* All done. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
void Remove_From_Boot_Manager ( ADDRESS Handle, CARDINAL32 * Error_Code )
{

  Partition_Data   *        PartitionRecord; /* Used to point to the Partition_Data structure we are manipulating. */
  Volume_Data      *        VolumeRecord;    /* Used to point to the Volume_Data structure we are manipulating. */
  ADDRESS                   Object;          /* Used when translating the Partition_Handle into a Partition_Data structure. */
  TAG                       ObjectTag;       /* Used when translating the Partition_Handle into a Partition_Data structure. */
  Partition_Data_To_Update  New_Values;      /* Used when clearing the On_Boot_Manager_Menu flags in the partitions associated with a volume. */


  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Indicate that Boot Manager has not been installed yet! */
    *Error_Code = LVM_ENGINE_BOOT_MANAGER_NOT_FOUND;

    return;

  }

  /* Determine what kind of a handle we really have. */
  Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_BAD_HANDLE;

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */
  switch ( ObjectTag )
  {
    case PARTITION_DATA_TAG : /* We have a partition here.  If it is part of a volume, then this is incorrect. */

                              /* Establish access to the Partition_Data structure for the partition. */
                              PartitionRecord = (Partition_Data *) Object;

                              /* Does this Partition_Data structure represent a partition? */
                              if ( PartitionRecord->Partition_Type != Partition )
                              {

                                /* Since this is part of a volume, we can not remove it from the Boot Manager Menu. */
                                *Error_Code = LVM_ENGINE_BAD_HANDLE;

                                return;

                              }

                              /* Is this partition part of a volume? */
                              if ( PartitionRecord->Volume_Handle != NULL )
                              {

                                /* Since this is part of a volume, we can not remove it from the Boot Manager Menu. */
                                *Error_Code = LVM_ENGINE_BAD_HANDLE;

                                return;

                              }

                              /* Since this partition is not part of a volume, we just need to adjust its DLA_Table_Entry. */
                              PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;

                              /* Since this is not part of a volume, make sure that its Installable flag is off. */
                              PartitionRecord->DLA_Table_Entry.Installable = FALSE;

                              /* Set the ChangesMade flag for the drive containing the partition. */
                              DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

                              break;
    case VOLUME_DATA_TAG : /* We must remove the Volume from the Boot Manager Menu. */

                           /* Establish access to the data for the volume. */
                           VolumeRecord = (Volume_Data *) Object;

                           /* If the volume can not be changed, abort. */
                           if ( ! VolumeRecord->Can_Be_Altered )
                           {

                             /* We can not perform this operation on this volume. */
                             *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

                             return;

                           }

                           /* To remove the volume from the Boot Manager Menu, we need to clear the entry in the DLA Table
                              of the first partition in the volume which indicates that this volume is on the Boot Manager Menu. */

                           /* Set up to clear the On_Boot_Manager_Menu flag. */
                           New_Values.Update_Drive_Letter = FALSE;
                           New_Values.New_Drive_Letter = 'B';
                           New_Values.Update_Volume_Name = FALSE;
                           New_Values.New_Name = NULL;
                           New_Values.Update_Boot_Manager_Status = TRUE;
                           New_Values.On_Menu = FALSE;
                           New_Values.Update_Volume_Spanning = FALSE;
                           New_Values.Spanned_Volume = FALSE;

                           /* Now update all of the partitions. */
                           Update_Partitions_Volume_Data( VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, &New_Values, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

                           assert( *Error_Code == DLIST_SUCCESS );

#else

                           if ( *Error_Code != DLIST_SUCCESS )
                           {

                             *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

                           }

#endif

#endif

                           /* Now update the VolumeRecord. */
                           VolumeRecord->On_Boot_Manager_Menu = FALSE;

                           /* If this volume was installable, it isn't anymore because it is no longer on the Boot Manager Menu! */
                           if ( Install_Volume_Handle == VolumeRecord->Volume_Handle)
                             Install_Volume_Handle = NULL;

                           break;

    default: /* Bad Handle! */
             *Error_Code = LVM_ENGINE_BAD_HANDLE;
             break;
  }

  /* All done. */

  return;

}


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
Boot_Manager_Menu  Get_Boot_Manager_Menu ( CARDINAL32 * Error_Code)
{

  CARDINAL32        Index;

  Boot_Manager_Menu   Menu;

  /* Initialize Menu assuming failure. */
  Menu.Count = 0;
  Menu.Menu_Items = NULL;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return Menu;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Indicate that Boot Manager has not been installed yet! */
    *Error_Code = LVM_ENGINE_BOOT_MANAGER_NOT_FOUND;

    return Menu;

  }

  /* We need to do two passes.  The first pass counts how many items are on the Boot Manager Menu.  This allows us to
     correctly size the array being returned to our caller and allocate memory accordingly.  The second pass actually
     fills in the array allocated after the first pass.                                                                    */

  /* Search the drives for partitions with DLA Table entries indicating that they are on the Boot Manager Menu and count them. */
  for ( Index = 0; Index < DriveCount; Index++)
  {

    ForEachItem(DriveArray[Index].Partitions, &Get_Menu_Items, &Menu, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return Menu;

    }

#endif

#endif

  }

  /* Allocate the Menu_Items array. */
  Menu.Menu_Items = ( Boot_Manager_Menu_Item *) malloc(Menu.Count * sizeof(Boot_Manager_Menu_Item));

  /* Now set up to search for Boot Manager Menu Items again. */
  Menu.Count = 0;

  /* Search the drives for partitions with DLA Table entries indicating that they are on the Boot Manager Menu and
     fill in the Menu_Items array.                                                                                   */
  for ( Index = 0; Index < DriveCount; Index++)
  {

    ForEachItem(DriveArray[Index].Partitions, &Get_Menu_Items, &Menu, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return Menu;

    }

#endif

#endif

  }

  /* Indicate that no errors were found. */
  *Error_Code  = LVM_ENGINE_NO_ERROR;

  return Menu;

}


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
void Install_Boot_Manager ( CARDINAL32   Drive_Number, CARDINAL32 * Error_Code )
{

  ADDRESS                         Partition_Handle;
  Partition_Data *                PartitionRecord;
  Volume_Data *                   VolumeRecord;
  CARDINAL32                      Index;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed and active? */
  if ( Boot_Manager_Found && Boot_Manager_Active )
  {

    /* Indicate that Boot Manager is already installed! */
    *Error_Code = LVM_ENGINE_BOOT_MANAGER_ALREADY_INSTALLED;

    return;

  }

  /* Is the drive number specified correct? */
  if ( ( Drive_Number < 1 ) || ( Drive_Number > 2 ) || ( Drive_Number > DriveCount ) )
  {

    /* Indicate the error. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    return;

  }

  /* If the drive is a PRM in big floppy mode, then we can not install Boot Manager to it. */
  if ( DriveArray[Drive_Number - 1].Is_Big_Floppy )
  {

    /* We can not perform this operation on this volume. */
    *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

    return;

  }

  /* Is there an inactive copy of Boot Manager already on this drive?  If so, then we will activate it and update it. */
  if ( Boot_Manager_Found && !Boot_Manager_Active )
  {

    /* Set the Boot Manager Partition Startable. */
    Set_Startable( Boot_Manager_Handle, Error_Code );

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      return;

  }
  else
  {

    /* There was no inactive copy of Boot Manager.  We will have to create a new one. */

    /* If we are installing Boot Manager on drive 2, then we must do a little preparation before calling Create_Partition. */
    if ( Drive_Number == 2 )
    {

      /* When Create_Partition is told to make a bootable partition, it checks several things.  The one that concerns us here
         is the check performed to see if a partition can be made bootable.  If the partition is not being created on the first
         drive, then, in order to pass this check, Boot Manager must be installed.  Since Boot Manager is not currently installed
         (we are in the process of installing it!), this check will fail and we will not be able to create a bootable partition.
         If we tell Create_Partition that we do not want a bootable partition, then the partition it creates may be above the
         1024 cylinder limit, in which case we may not be able to use it for Boot Manager.  So what we will do is pretend that
         Boot Manager is already installed by setting the global Boot Manager variables.  This will cause Create Partition to
         allow us to create a bootable primary partition (if possible) on drive 2.                                                  */
      Boot_Manager_Found = TRUE;
      Boot_Manager_Handle = (ADDRESS) 1;
      Boot_Manager_Active = TRUE;

    }

    /* Boot Manager needs to be on a bootable primary partition.  Allocate one. */
    Partition_Handle = Create_Partition(DriveArray[Drive_Number - 1].External_Handle, MBB_SECTOR_COUNT, "[ BOOT MANAGER ]", Automatic, TRUE, TRUE, TRUE, Error_Code);

    /* Ensure that the Boot Manager Control flags are off as we may have had to change them before calling Create_Partition.
       These should only be turned on after Boot Manager has been successfully installed.                                     */
    Boot_Manager_Found = FALSE;
    Boot_Manager_Handle = NULL;
    Boot_Manager_Active = FALSE;

    /* Did we succeed. */
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      return;

    /* Now make all the primary partitions on the drive inactive.  Only Boot Manager should be active. */
    ForEachItem(DriveArray[Drive_Number - 1].Partitions, &Make_Primaries_Inactive, NULL, TRUE, Error_Code);

  #ifdef DEBUG

  #ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

  #else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

  #endif

  #endif

    /* Now set our Boot Manager Partition to Active. */
    Set_Active_Flag( Partition_Handle, ACTIVE_PARTITION, Error_Code);
  #ifdef DEBUG

  #ifdef PARANOID

    assert( *Error_Code == LVM_ENGINE_NO_ERROR );

  #else

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

  #endif

  #endif


    /* Set our Boot Manager Partition's OS Flag to the Boot Manager code. */
    Set_OS_Flag( Partition_Handle, BOOT_MANAGER_INDICATOR, Error_Code);

  #ifdef DEBUG

  #ifdef PARANOID

    assert( *Error_Code == LVM_ENGINE_NO_ERROR );

  #else

    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

  #endif

  #endif

    /* Now set our global variables that indicate that a Boot Manager is installed and needs to be upgraded. */
    Boot_Manager_Found = TRUE;
    Upgrade_Boot_Manager = TRUE;
    Boot_Manager_Handle = Partition_Handle;
    Boot_Manager_Active = TRUE;

    /* Since this is a new Boot Manager, set up access to our default options. */
    BM_Options = (Boot_Path_Record *) &(PBOOT[ BOOT_MANAGER_PATH_SECTOR_OFFSET * BYTES_PER_SECTOR / 4 ]);
    Boot_Manager_Options_Changed = FALSE;

    /* Does the drive containing Boot Manager have a serial number? */
    if ( DriveArray[Drive_Number - 1].Drive_Serial_Number == 0 )
    {

      DriveArray[Drive_Number - 1].Drive_Serial_Number =  Create_Serial_Number();
      DriveArray[Drive_Number - 1].ChangesMade = TRUE;

    }

    /* Is the Serial Number of the drive containing Boot Manager different than what is in Boot_Drive_Serial_Number? */
    if ( Boot_Drive_Serial_Number != DriveArray[Drive_Number - 1].Drive_Serial_Number )
    {

      /* Update the Boot_Drive_Serial_Numbers for drives using the previous one. */
      for ( Index = 0; Index < DriveCount; Index++ )
      {

        if ( DriveArray[Index].Boot_Drive_Serial_Number == Boot_Drive_Serial_Number )
        {
          DriveArray[Index].Boot_Drive_Serial_Number = DriveArray[Drive_Number - 1].Drive_Serial_Number;
          DriveArray[Index].ChangesMade = TRUE;
        }

      }

      /* Save the new Boot Drive Serial Number. */
      Boot_Drive_Serial_Number = DriveArray[Drive_Number - 1].Drive_Serial_Number;

    }

  }

  /* Do we need to update the MBR? */

  /* If we are installing Boot Manager onto drive 2, then drive 1 must have the correct MBR.  We will upgrade the
     MBR on drive one.  If there is already an MBR on drive 1, all we need to do to upgrade it is to set the
     New_Partition flag for the MBR to TRUE.  If there is no MBR on drive 1, then we must create one.              */
  if ( Drive_Number == 2 )
  {

    /* Is there an MBR on drive 1?  If there is more than 1 partition in the Partitions list for drive 1 then there must be. */
    if ( GetListSize(DriveArray[0].Partitions, Error_Code) > 1 )
    {

      /* We have an MBR!  Lets find it and fix it. */

      /* The MBR should be the first item in the Partitions list since it occupies LBA 0. */
      GoToStartOfList(DriveArray[0].Partitions, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

#endif

#endif

      PartitionRecord = (Partition_Data *) GetObject( DriveArray[0].Partitions, sizeof(Partition_Data),PARTITION_DATA_TAG, NULL, FALSE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );
      assert( PartitionRecord->Partition_Type == MBR_EBR );
      assert( PartitionRecord->Starting_Sector == 0 );

#else

      if ( ( *Error_Code != DLIST_SUCCESS ) || ( PartitionRecord->Partition_Type != MBR_EBR ) || (PartitionRecord->Starting_Sector != 0) )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

#endif

#endif

      /* Set the New_Partition flag to TRUE to cause the MBR to be updated. */
      PartitionRecord->New_Partition = TRUE;

    }
    else
    {

      /* Did we get here because GetListSize failed? */
#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

#endif

#endif

      /* There is no MBR.  We must create one! */
      New_MBR( DriveArray[0].External_Handle, Error_Code);

      /* If we did not succeed, then return the error code to the caller. */
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        return;

    }

  }

  /* Is there an Installable Volume? */
  if ( Install_Volume_Handle != NULL )
  {

    /* We must add this volume to the Boot Manager Menu. */

    /* Get the Volume's data. */
    VolumeRecord = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, Install_Volume_Handle, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

#endif

#endif

    /* Get the partition associated with the volume. */
    PartitionRecord = VolumeRecord->Partition;

    /* Set the partition's On_Boot_Manager_Menu flag. */
    PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = TRUE;

    /* Set the Volume's On_Boot_Manager_Menu flag. */
    VolumeRecord->On_Boot_Manager_Menu = TRUE;

    /* Indicate that changes have been made to the drive containing the partition associated with the Volume marked Installable. */
    DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
void Remove_Boot_Manager( CARDINAL32 * Error_Code )
{

  CARDINAL32        Index;
  ADDRESS           Old_Boot_Manager_Handle;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Since Boot Manager has not been installed, we don't have to do anything! */
    *Error_Code = LVM_ENGINE_NO_ERROR;

    return;

  }

  /* Save the Boot Manager Handle.  We will need it to delete the Boot Manager Partition. */
  Old_Boot_Manager_Handle = Boot_Manager_Handle;

  /* Clear the Boot_Manager_Handle.  Delete_Partition keys on this value to determine if it is deleting Boot Manager or not.
     If it finds that the handle for the partition it is deleting is the same as the Boot_Manager_Handle, then it calls
     Remove_Boot_Manager.  Since we don't want a recursive disaster on our hands, we must set Boot_Manager_Handle to NULL so
     that Delete_Partition will not call us.  Then we can use Delete_Partition to delete the Boot Manager partition.            */
  Boot_Manager_Handle = NULL;

  /* Reset the other Boot Manager flags. */
  Boot_Manager_Found = FALSE;
  Upgrade_Boot_Manager = FALSE;
  Boot_Manager_Options_Changed = FALSE;
  BM_Options = NULL;
  Boot_Manager_Active = FALSE;

  /* Now it is safe to delete the Boot Manager partition.  */
  Delete_Partition(Old_Boot_Manager_Handle, Error_Code);

  /* Was there a problem? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == LVM_ENGINE_OUT_OF_MEMORY);

#else

    if ( *Error_Code != LVM_ENGINE_OUT_OF_MEMORY )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    }

#endif

#endif

    return;

  }

  /* Search the drives for partitions with DLA Table entries indicating that they are on the Boot Manager Menu.
     Remove them from the Boot Manager Menu.  Also, check any installable partitions to see if they are still
     installable without the presence of Boot Manager.                                                           */
  for ( Index = 0; Index < DriveCount; Index++)
  {

    /* Since there is no Boot Manager, the system must boot off of the first drive in the system.
       Update the Boot Drive Serial Number information for drives in the DriveArray.               */
    if ( ( Boot_Drive_Serial_Number != DriveArray[0].Boot_Drive_Serial_Number ) &&
         ( DriveArray[Index].Boot_Drive_Serial_Number == Boot_Drive_Serial_Number )
       )
    {

      DriveArray[Index].Boot_Drive_Serial_Number = DriveArray[0].Drive_Serial_Number;
      DriveArray[Index].ChangesMade = TRUE;

    }

    /* Check the partitions on the current drive and remove them from the Boot Manager Menu. */
    ForEachItem(DriveArray[Index].Partitions, &Remove_From_Boot_Manager_Menu, NULL, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

#endif

#endif

  }

  /* Since Boot Manager is no longer available, we must mark all volumes as NOT being on the Boot Manager Menu. */
  ForEachItem(Volumes,&Remove_Boot_Manager_Menu_Flag, NULL, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

#endif

#endif

  /* Save the new Boot Drive Serial Number. */
  Boot_Drive_Serial_Number = DriveArray[0].Boot_Drive_Serial_Number;

  /* Indicate that no errors were found. */
  *Error_Code  = LVM_ENGINE_NO_ERROR;

  return;

}


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
void Set_Boot_Manager_Options( ADDRESS      Handle,
                               BOOLEAN      Timer_Active,
                               CARDINAL32   Time_Out_Value,
                               BOOLEAN      Advanced_Mode,
                               CARDINAL32 * Error_Code
                             )
{

  Partition_Data   * PartitionRecord; /* Used to point to the Partition_Data structure we are manipulating. */
  Volume_Data      * VolumeRecord;    /* Used to point to the Volume_Data structure we are manipulating. */
  ADDRESS            Object;          /* Used when translating the Handle into a Partition or Volume Data structure. */
  TAG                ObjectTag;       /* Used when translating the Handle into a Partition or Volume Data structure. */


  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Indicate that Boot Manager has not been installed yet! */
    *Error_Code = LVM_ENGINE_BOOT_MANAGER_NOT_FOUND;

    return;

  }

  if ( Handle != NULL )
  {

    /* Determine what kind of a handle we really have. */
    Translate_Handle( Handle, &Object, &ObjectTag, Error_Code );

    /* Was the handle valid? */
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      *Error_Code = LVM_ENGINE_BAD_HANDLE;

      return;

    }

    /* From the ObjectTag we can tell what Object points to. */
    switch ( ObjectTag )
    {
      case PARTITION_DATA_TAG : /* We have a partition here. */

                                /* Establish access to the Partition_Data structure for the partition. */
                                PartitionRecord = (Partition_Data *) Object;

                                /* Does this Partition_Data structure represent a partition? */
                                if ( PartitionRecord->Partition_Type != Partition )
                                {

                                  /* Since this does not represent a partition, we can not use it with Boot Manager. */
                                  *Error_Code = LVM_ENGINE_BAD_HANDLE;

                                  return;

                                }

                                /* Is this partition on the Boot Manager Menu? */
                                if ( ! PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
                                {

                                  /* Since this partition is not on the Boot Manager Menu, we can not set it as the default boot partition! */
                                  *Error_Code = LVM_ENGINE_BAD_HANDLE;

                                  return;

                                }

                                /* prefill with blanks 206211 */
                                memset(BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name, ' ', ALIAS_NAME_LENGTH_2);
                                memset(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, ' ', ALIAS_NAME_LENGTH);

                                /* indicate that name was set by large name aware program 206211*/
                                BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Valid_Length = ALIAS_NAME_LENGTH_2;

                                /* If this partition is part of a volume, it will have a volume name.  The Volume Name is what Boot Manager will
                                   use when looking for the default boot volume.                                                                  */
                                if ( PartitionRecord->DLA_Table_Entry.Volume_Name[0] != 0x00 )
                                {

                                  /* We have a volume name.  Copy that into the correct location in the BM_Options. */
                                  memcpy(BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
                                         PartitionRecord->DLA_Table_Entry.Volume_Name,
                                         min(ALIAS_NAME_LENGTH_2,strlen(PartitionRecord->DLA_Table_Entry.Volume_Name)));

                                  memcpy(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
                                         PartitionRecord->DLA_Table_Entry.Volume_Name,
                                         min(ALIAS_NAME_LENGTH,strlen(PartitionRecord->DLA_Table_Entry.Volume_Name)));

                                }
                                else
                                {

                                  /* Since there was no volume name, use the partition name. */
                                  memcpy(BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
                                         PartitionRecord->Partition_Name,
                                         min(ALIAS_NAME_LENGTH_2,strlen(PartitionRecord->Partition_Name)));
                                  memcpy(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
                                         PartitionRecord->Partition_Name,
                                         min(ALIAS_NAME_LENGTH,strlen(PartitionRecord->Partition_Name)));

                                }

                                break;
      case VOLUME_DATA_TAG : /* We must make this volume the default boot volume. */

                             /* Establish access to the data for the volume. */
                             VolumeRecord = (Volume_Data *) Object;

                             /* Only compatibility volumes are bootable.  If this is not a compatibility volume, then it is not
                                bootable and can not be on the Boot Manager Menu.                                                */
                             if ( ! VolumeRecord->Compatibility_Volume )
                             {

                               /* This volume is not eligible to be on the Boot Manager menu. */
                               *Error_Code = LVM_ENGINE_WRONG_VOLUME_TYPE;

                               return;

                             }

                             /* Is this volume already on the Boot Manager Menu? */
                             if ( ! VolumeRecord->On_Boot_Manager_Menu )
                             {

                               /* Since this volume is not on the Boot Manager Menu, we can not make it the default boot volume. */
                               *Error_Code = LVM_ENGINE_BAD_HANDLE;

                               return;

                             }

                             /* prefill with blanks 206211*/
                             memset(BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name, ' ', ALIAS_NAME_LENGTH_2);
                             memset(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, ' ', ALIAS_NAME_LENGTH);

                             /* indicate that name was set by large name aware program 206211*/
                             BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Valid_Length = ALIAS_NAME_LENGTH_2;

                             /* Copy the Volume Name into the correct location in the BM_Options. */
                             memcpy(BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
                                    VolumeRecord->Volume_Name,
                                    min(ALIAS_NAME_LENGTH_2,strlen(VolumeRecord->Volume_Name)));
                             memcpy(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
                                    VolumeRecord->Volume_Name,
                                    min(ALIAS_NAME_LENGTH,strlen(VolumeRecord->Volume_Name)));
                             /*strncpy(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, VolumeRecord->Volume_Name, ALIAS_NAME_LENGTH ); */

                             break;

      default: /* Bad Handle! */
               *Error_Code = LVM_ENGINE_BAD_HANDLE;
               return;

               /* Keep the compiler happy. */
               break;

    }

  }
  else
  {

    /* A NULL Handle means to remove the default boot volume. */
    memset(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, 0x00, ALIAS_NAME_LENGTH );
    memset(BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name, 0x00, ALIAS_NAME_LENGTH_2 );

  }

  /* Now set the remaining options. */

  /* Is the Timer Active?  */
  if ( Timer_Active )
    BM_Options->TimeOut = Time_Out_Value * 18;  /* The time out value is specified in seconds, but Boot Manager needs it in 1/18 of a second increments.  Convert accordingly. */
  else
    BM_Options->TimeOut = 0xffff;               /* The timer is disabled.  */

  BM_Options->Advanced_Mode = Advanced_Mode;    /* Set the status of Boot Manager's Advanced Mode. */

  /* Set the global flag used to track changes to the Boot Manager Options. */
  Boot_Manager_Options_Changed = TRUE;

  /* All done. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


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
void Get_Boot_Manager_Options( ADDRESS    *  Handle,
                               BOOLEAN    *  Handle_Is_Volume,
                               BOOLEAN    *  Timer_Active,
                               CARDINAL32 *  Time_Out_Value,
                               BOOLEAN    *  Advanced_Mode,
                               CARDINAL32 * Error_Code
                             )
{

  CARDINAL32  Drive_Index;   /* Used to walk the DriveArray. */


  /* Assume failure. */
  *Handle = NULL;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Indicate that Boot Manager has not been installed yet! */
    *Error_Code = LVM_ENGINE_BOOT_MANAGER_NOT_FOUND;

    return;

  }

  /* We must find a volume which is on the Boot Manager Menu and which has the same name as what is listed
     in BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name.  If we can not find a volume that meets these
     requirements, we must then search all of the partitions in the system until we find a partition that
     matches.  If we can not find a partition which matches, then we give up and return NULL for the handle. */
  ForEachItem( Volumes, &Find_Matching_Volume_Name, Handle, TRUE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == DLIST_SUCCESS );

#else

  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Did we find a match?  If we did, *Handle will not be NULL. */
  if ( *Handle == NULL )
  {

    /* Since we did not find a match, then whatever the default boot partition may be, it is not a volume. */
    *Handle_Is_Volume = FALSE;

    /* We did not find a match.  We must now search all of the partitions in the system looking for a match. */
    for ( Drive_Index = 0; Drive_Index < DriveCount; Drive_Index++ )
    {

      /* Examine all of the partitions on the current drive. */
      ForEachItem( DriveArray[Drive_Index].Partitions, &Find_Matching_Partition_Name, Handle, TRUE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

#endif

#endif

    }

  }
  else
    *Handle_Is_Volume = TRUE;

  /* Now get the remaining options. */

  /* Is the Timer Active?  */
  if ( BM_Options->TimeOut != 0xffff )
  {

      *Timer_Active = TRUE;

      *Time_Out_Value = BM_Options->TimeOut / 18;

  }
  else
  {

    *Timer_Active = FALSE;

    *Time_Out_Value = 0;

  }

  *Advanced_Mode = BM_Options->Advanced_Mode;

  /* All done. */
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
void Close_Boot_Manager( void )
{

  /* Clear some of the global flags. */
  Boot_Manager_Found = FALSE;
  Boot_Manager_Active = FALSE;
  Boot_Manager_Handle = NULL;
  Upgrade_Boot_Manager = FALSE;
  Boot_Manager_Options_Changed = FALSE;
  BM_Options = NULL;

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
void Discover_Boot_Manager( CARDINAL32 * Error_Code )
{

  CARDINAL32                      Index;
  CARDINAL32                      Limit = 2;
  Find_Boot_Manager_Control_Data  Search_Control;
  Partition_Data *                PartitionRecord;

  /* Boot Manager can only be installed on Drives 1 or 2, so those are the only drives we need to check. */

  /* Are there two or more drives? */
  if ( Limit > DriveCount )
  {

    /* No, there is only 1 drive.  Set limit accordingly. */
    Limit = DriveCount;

  }

  /* Set up to search the drives for an active Boot Manager. */
  Search_Control.Find_Active_Boot_Manager = TRUE;
  Search_Control.Boot_Manager_Partition = NULL;
  Search_Control.Non_Current_Boot_Manager = FALSE;
  Boot_Manager_Found = FALSE;
  Boot_Manager_Active = FALSE;
  Boot_Manager_Options_Changed = FALSE;

  /* Search the drives. */
  for ( Index = 0; (Index < Limit) && ( Search_Control.Boot_Manager_Partition == NULL ); Index++)
  {

    ForEachItem(DriveArray[Index].Partitions, &Find_Boot_Manager, &Search_Control, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

    assert(*Error_Code == DLIST_SUCCESS);

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

#endif

#endif

  }

  /* Was a Boot Manager found? */
  if ( Search_Control.Boot_Manager_Partition != NULL )
  {

    /* We have found the active copy of Boot Manager. */
    Boot_Manager_Active = TRUE;
    Boot_Manager_Found = TRUE;

    /* Set up easy access to the partition data for the Boot Manager partition. */
    PartitionRecord = Search_Control.Boot_Manager_Partition;

    /* Save the partition's external handle.  This way, if the user deletes the Boot Manager partition using the
       Delete_Partition function instead of calling Remove_Boot_Manager, the Delete_Partition function can
       determine that it is Boot Manager being deleted and call the Remove_Boot_Manager function itself.            */
    Boot_Manager_Handle = PartitionRecord->External_Handle;

    /* Was the version of Boot Manager current?  If not, then we will automatically upgrade it. */
    if ( Search_Control.Non_Current_Boot_Manager )
    {

      Upgrade_Boot_Manager = TRUE;

    }

    /* Does the drive containing Boot Manager have a serial number? */
    if ( DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number == 0 )
    {

      DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number =  Create_Serial_Number();
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

    /* Is the Drive_Serial_Number of the drive containing Boot Manager different that what we have in Boot_Drive_Serial_Number? */
    if ( Boot_Drive_Serial_Number != DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number )
    {

      /* Update the Boot_Drive_Serial_Numbers for drives using the previous one. */
      for ( Index = 0; Index < DriveCount; Index++ )
      {

        if ( DriveArray[Index].Boot_Drive_Serial_Number == Boot_Drive_Serial_Number )
        {
           DriveArray[Index].Boot_Drive_Serial_Number = DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number;
           DriveArray[Index].ChangesMade = TRUE;
        }

      }

      /* Save the new Boot Drive Serial Number. */
      Boot_Drive_Serial_Number = DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number;

    }

  }
  else
  {

    /* We did not find an active copy of Boot Manager!  Is there an inactive copy of Boot Manager?  Certain other operating systems
       (i.e. Windoze and its variants) will disable Boot Manager when installing and then never re-activate it.                       */

    /* Set up to search the drives for an active Boot Manager. */
    Search_Control.Find_Active_Boot_Manager = FALSE;
    Search_Control.Boot_Manager_Partition = NULL;
    Search_Control.Non_Current_Boot_Manager = FALSE;

    /* Search the drives. */
    for ( Index = 0; (Index < Limit) && (  Search_Control.Boot_Manager_Partition == NULL ); Index++)
    {

      ForEachItem(DriveArray[Index].Partitions, &Find_Boot_Manager, &Search_Control, TRUE, Error_Code);

  #ifdef DEBUG

  #ifdef PARANOID

      assert(*Error_Code == DLIST_SUCCESS);

  #else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

  #endif

  #endif

    }

    if ( Search_Control.Boot_Manager_Partition != NULL )
    {

      /* We have found an inactive copy of Boot Manager. */
      Boot_Manager_Active = FALSE;
      Boot_Manager_Found = TRUE;

      /* Set up easy access to the partition data for the Boot Manager partition. */
      PartitionRecord = Search_Control.Boot_Manager_Partition;

      /* Save the partition's external handle.  This way, if the user deletes the Boot Manager partition using the
         Delete_Partition function instead of calling Remove_Boot_Manager, the Delete_Partition function can
         determine that it is Boot Manager being deleted and call the Remove_Boot_Manager function itself.            */
      Boot_Manager_Handle = PartitionRecord->External_Handle;

      /* Was the version of Boot Manager current?  If not, then we will automatically upgrade it. */
      if ( Search_Control.Non_Current_Boot_Manager )
      {

        Upgrade_Boot_Manager = TRUE;

      }

    }

  }

  /* Was a copy of Boot Manager found? */
  if ( Boot_Manager_Found )
  {

#if 0  //EK
//code to get bootmanager binary
/*--------------------------------------- */
 { FILE *fp;
   int i,j,bsize,n;
   char str[18];
  Boot_Manager_Boot_Record *  BM_Boot_Sector;      /* Used to manipulate the Boot Manager Boot Sector.     */
  Boot_Path_Record *          BM_Boot_Path_Sector; /* Used to access the "Boot Path" sector in Boot Manager. */

  n = sizeof(PBOOT)/512;
  memset(PBOOT,0,sizeof(PBOOT));
  ReadSectors( PartitionRecord->Drive_Index + 1,          /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
               PartitionRecord->Starting_Sector,
               n,
               &PBOOT,
               Error_Code);

   /* Establish access to the Boot Manager Boot Sector. */
   BM_Boot_Sector = ( Boot_Manager_Boot_Record * ) &PBOOT;
   BM_Boot_Path_Sector = ( Boot_Path_Record * ) ( (CARDINAL) &PBOOT + ( BOOT_MANAGER_PATH_SECTOR_OFFSET * BYTES_PER_SECTOR ) );

   fp=fopen("pboot._h","w");
   for(i=0; i<sizeof(PBOOT)/sizeof(long int); i++)
   {  if(PBOOT[i] != 0) bsize = i;
   }
   bsize = (bsize+4)/4 * 4;
   n =  (bsize*4 + 512)/512;
   
   fprintf(fp,"/* pboot.h */\n");
   fprintf(fp,"/* (%i+512)/512 */\n",bsize );
   fprintf(fp,"#define MBB_SECTOR_COUNT %i\n",n);

   fprintf(fp,"long PBOOT[%i] =\n{\n",sizeof(PBOOT)/sizeof(long int));

   for(i=0; i<bsize; i++)
   { if(i> 0 && !(i%4))
     {   memcpy(str, &PBOOT[i-4],16);
         str[16]=0;
         for(j=0;j<16;j++)
         { if(str[j]<32) str[j]='.';
         }
         fprintf(fp," /* %s */\n ",str);
     }
    fprintf(fp,"%#10x", PBOOT[i]);
    if(i < bsize-1)
            fprintf(fp,", ");
    }
   fprintf(fp,"\n};\n ");

   fclose(fp);
//   exit(0);
 }
/*--------------------------------------- */
#endif //0

    /* Now we must read in the Boot Manager Options. */
    ReadSectors( PartitionRecord->Drive_Index + 1,          /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                 PartitionRecord->Starting_Sector + BOOT_MANAGER_PATH_SECTOR_OFFSET,
                 1,                                          /* We only want the Boot Manager BootPath Sector. */
                 &Boot_Manager_Options_Sector,
                 Error_Code);

    if ( *Error_Code == DISKIO_NO_ERROR )
    {

      /* Set up our Boot Manager Options pointer to point to the buffer where we read in the Boot Manager Options. */
      BM_Options = (Boot_Path_Record *) &Boot_Manager_Options_Sector;

    }
    else
    {

      /* Was this an I/O error?  If so, mark the Drive Array accordingly. */
      if ( *Error_Code == DISKIO_READ_FAILED )
      {

        DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

        /* Since Boot Manager will not function if it can not read its options sector, set the Upgrade_Boot_Manager flag.
           That will cause us write Boot Manager to disk again, and possibly fix the write error.                          */
        Upgrade_Boot_Manager = TRUE;

        /* Set BM_Options to point to our built in copy of Boot Manager and its options sector. */
        BM_Options = (Boot_Path_Record *) &(PBOOT[ BOOT_MANAGER_PATH_SECTOR_OFFSET * BYTES_PER_SECTOR / 4]);

      }
      else
      {

#ifdef DEBUG

#ifdef PARANOID

        assert(0);

#else

        /* We have an internal error!  Indicate a corrupted list.  This will abort out list traversal. */
        *Error_Code = DLIST_CORRUPTED;

        return;
#endif

#endif

      }

    }

  }

  /* Indicate that no errors were found. */
  *Error_Code  = LVM_ENGINE_NO_ERROR;

  return;

}


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
ADDRESS _System Get_Boot_Manager_Handle( CARDINAL32 * Error_Code)
{

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return NULL;

  }

  return Boot_Manager_Handle;

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
void Migrate_Old_Boot_Manager_Menu_Items( CARDINAL32 * Error_Code )
{

  CARDINAL32                      Index;
  CARDINAL32                      Limit = 2;
  Partition_Data *                BM_PartitionRecord;
  ADDRESS                         BM_Object;         /* Used to get around a bug in the Visual Age compiler. */
  TAG                             BM_PartitionTag;

  /* Was Boot Manager found? */
  if ( Boot_Manager_Handle != NULL )
  {

    /* Now we must migrate the Boot Manager Menu from the old format to the new format.  Part of the migration is done
       here, and part of the migration is done during the discovery of partitions in the Partition Manager.  Specifically,
       we must migrate the Boot Manager Menu data for primary partitions here, and finish the migration of Logical Drives
       here.  The Migration of Logical Drives is begun in the Partition Manager when the Partition Manager is traversing
       all of the MBR and EBR's on the drives in the system.  The migration is handled this way due to the way in
       which Boot Manager stores its Menu data.  For logical drives, Boot Manager stores its Menu data in the EBR.  Since
       the EBR is read and processed by the Partition Manager, it was easy to have the Partition Manager perform some of the
       migration of Logical Drives.  If we were to do this here, we would have to read all of the EBRs again,
       which would mean a lot of extra code and I/O.  For primary partitions, Boot Manager stores its Menu data in the
       Alias Table inside of the Boot Manager partition.  The Alias Table supports 24 drives with up to 4 primary partitions
       each.  So what we must do here is read the Alias Table into memory and, for each primary partition on each of the
       first 24 drives, find the entry in the Alias Table for that primary partition, and, if the entry has its Bootable
       flag set, migrate the entry to the new LVM format.

       Migration to the new LVM format consists of checking to see if the partition is part of a compatibility volume.
       If it is, then we just ensure that the appropriate flag is set in the DLA Table entry for that partition/volume.
       If the partition is not part of a volume, the we make it a hidden compatibility volume and ensure that the
       appropriate flag is set in the DLA Table Entry for the new volume.                                                       */

    /* Get the Partition_Data for the Boot Manager Partition. */
    Translate_Handle( Boot_Manager_Handle, &BM_Object, &BM_PartitionTag, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

    assert( ( *Error_Code == HANDLE_MANAGER_NO_ERROR ) && ( BM_PartitionTag == PARTITION_DATA_TAG ) );

#else

    /* Was the operation successful? */
    if ( ( *Error_Code != HANDLE_MANAGER_NO_ERROR ) || ( BM_PartitionTag != PARTITION_DATA_TAG ) )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

#endif

#endif

    /* Establish access to the Partition Data for the Boot Manager Partition. */
    BM_PartitionRecord = (Partition_Data *) BM_Object;

    /* We need to read in the Alias Table. The size of the Alias Table is an exact multiple of the sector size. */
    ReadSectors( BM_PartitionRecord->Drive_Index + 1,                             /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                 BM_PartitionRecord->Starting_Sector + ALIAS_TABLE_SECTOR_OFFSET,
                 SECTORS_PER_ALIAS_TABLE,
                 &BM_Alias_Table,
                 Error_Code);

    if ( *Error_Code == DISKIO_NO_ERROR )
    {

      /* Now that we have the Alias Table, we can begin looking for partitions to migrate. */

      /* How many drives do we have to search? */
      if ( ALIAS_TABLE_DRIVE_LIMIT < DriveCount )
        Limit = ALIAS_TABLE_DRIVE_LIMIT;
      else
        Limit = DriveCount;

      /* Search the drives. */
      for ( Index = 0; Index < Limit; Index++)
      {

        ForEachItem(DriveArray[Index].Partitions, &Migrate_Old_BM_Menu, NULL, TRUE, Error_Code);

    #ifdef DEBUG

    #ifdef PARANOID

        assert(*Error_Code == DLIST_SUCCESS);

    #else

        if ( *Error_Code != DLIST_SUCCESS )
        {

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          return;

        }

    #endif

    #endif

      }

      /* Migration is now complete. */

    }
    else
    {

      /* There was an error!  Lets see what kind of error. */
      switch ( *Error_Code )
      {

        case DISKIO_OUT_OF_MEMORY : /* We are out of memory!  Convert the error code and return. */
                                    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
                                    return;

                                    break;  /* Keep the compiler happy. */


        case DISKIO_READ_FAILED :
                                  /* Mark the drive as having had an I/O error.  Because of this I/O error, we will not
                                     migrate any Boot Manager Menu items created by FDISK.  In fact, Boot Manager will
                                     probably not function correctly since the Alias Table is not readable.  Because of
                                     this, we will also set the flag which indicates that Boot Manager needs to be upgraded.
                                     This will result in a new copy of Boot Manager being written to the disk.               */
                                  DriveArray[BM_PartitionRecord->Drive_Index].IO_Error = TRUE;
                                  Upgrade_Boot_Manager = TRUE;

                                  break; /* Keep the compiler happy. */

        default :
                  /* We should only get here if there was an internal error! */

#ifdef DEBUG

#ifdef PARANOID

                  assert( 0 );

#else

                  *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

#endif

#endif

                  return;

                  break; /* Keep the compiler happy. */
      }

    }

  }

  /* Indicate that no errors were found. */
  *Error_Code  = LVM_ENGINE_NO_ERROR;

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
void Commit_Boot_Manager_Changes( CARDINAL32 * Error_Code )
{

  Partition_Data   *          PartitionRecord;     /* Used to point to the Partition_Data structure we are manipulating. */
  Boot_Manager_Boot_Record *  BM_Boot_Sector;      /* Used to manipulate the Boot Manager Boot Sector.     */
  Boot_Path_Record *          BM_Boot_Path_Sector; /* Used to access the "Boot Path" sector in Boot Manager. */
  CARDINAL32                  Cylinder;            /* Used to calculate the cylinder where Boot Manager will reside. */
  CARDINAL32                  Head;                /* Used to calculate the head where Boot Manager will reside. */
  CARDINAL32                  Sector;              /* Used to calculate the sector where Boot Manager will reside. */
  CARDINAL32                  Sectors_Remaining;   /* Used when calculating where Boot Manager lies on a disk. */
  CARDINAL32                  Current_LBA;         /* Used when calculating where Boot Manager lies on a disk. */
  CARDINAL32                  Table_Index;         /* Used when calculating where Boot Manager lies on a disk. */
  ADDRESS                     Object;              /* Used when translating the Boot_Manager_Handle into a Partition_Data structure. */
  TAG                         ObjectTag;           /* Used when translating the Boot_Manager_Handle into a Partition_Data structure. */
  CARDINAL32                  Drive_Index;         /* Used when walking the drive array. */

  /* Has the Engine been opened yet? */
  if ( DriveArray == NULL )
  {

    /* The Engine has not been opened yet!  Nothing has been initialized yet, so we can not perform the function asked of us.  Abort. */
    *Error_Code = LVM_ENGINE_NOT_OPEN;

    return;

  }

  /* Is Boot Manager installed? */
  if ( ! Boot_Manager_Found )
  {

    /* Since Boot Manager has not been installed, we don't have to do anything! */
    *Error_Code = LVM_ENGINE_NO_ERROR;

    return;

  }

  /* We need to get the partition data for the Boot Manager Partition. */
  Translate_Handle( Boot_Manager_Handle, &Object, &ObjectTag, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

  assert( *Error_Code == HANDLE_MANAGER_NO_ERROR );
  assert( ObjectTag == PARTITION_DATA_TAG );

#else

  /* Was the handle valid? */
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

  /* From the ObjectTag we can tell what Object points to. */

  /* Is the object what we want? */
  if ( ObjectTag != PARTITION_DATA_TAG )
  {

    /* We have a bad handle.  */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Establish access to the Partition_Data structure we want to manipulate. */
  PartitionRecord = ( Partition_Data * ) Object;

#ifdef DEBUG

#ifdef PARANOID

  assert( PartitionRecord->Partition_Type == Partition );

#else

  if ( PartitionRecord->Partition_Type != Partition )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Is the Upgrade_Boot_Manager flag set?  If so, then we must write out the Boot Manager code to the Boot Manager Partition. */
  if ( Upgrade_Boot_Manager )
  {

    /* We must update some values in Boot Manager before we can write it to disk. */

    /* Establish access to the Boot Manager Boot Sector. */
    BM_Boot_Sector = ( Boot_Manager_Boot_Record * ) &PBOOT;

    /* We must update the location of the Boot Path Sector.  */
    Convert_To_CHS(PartitionRecord->Starting_Sector + BOOT_MANAGER_PATH_SECTOR_OFFSET, PartitionRecord->Drive_Index, &Cylinder, &Head, &Sector);
    Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head, &Sector );

    BM_Boot_Sector->BootPathDrive = (BYTE) (PartitionRecord->Drive_Index + 0x80);
    BM_Boot_Sector->BootPathHead = (BYTE) Head;
    BM_Boot_Sector->BootPathSector = (BYTE) Sector;
    BM_Boot_Sector->BootPathCylinder = (BYTE) Cylinder;

    /* We must update the location of the Alias Table.  The location of the Alias Table is stored in the Boot Path Sector. */
    BM_Boot_Path_Sector = ( Boot_Path_Record * ) ( (CARDINAL) &PBOOT + ( BOOT_MANAGER_PATH_SECTOR_OFFSET * BYTES_PER_SECTOR ) );

    /* Calculate the location of the Alias Table. */
    Convert_To_CHS(PartitionRecord->Starting_Sector + ALIAS_TABLE_SECTOR_OFFSET, PartitionRecord->Drive_Index, &Cylinder, &Head, &Sector);
    Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head, &Sector );

    BM_Boot_Path_Sector->Drive = (BYTE) (PartitionRecord->Drive_Index + 0x80);
    BM_Boot_Path_Sector->Head = (BYTE) Head;
    BM_Boot_Path_Sector->Sector = (BYTE) Sector;
    BM_Boot_Path_Sector->Cylinder = (BYTE) Cylinder;

/* Now we must tell Boot Manager where to find himself.  Boot Manager will
   normally occupy only part of a single track on the disk.  However, for
   drives with small tracks ( less than 32 sectors per track ), Boot Manager
   may not fit on a single track.  In this case, Boot Manager will spill
   over into the following tracks.

   Boot Manager keeps track of himself through a table embedded in the
   Boot Manager Boot Sector.  In the normal case,  only the first entry
   is used as Boot Manager is contained in a single track.  But for drives
   with small tracks, additional table entries are used to point to the
   extra tracks that Boot Manager spills into.  We must calculate
   all of the entries that may be used in this table and update the table
   before we write Boot Manager to disk.
*/

    /* Calculate the table entries. */
    Sectors_Remaining = MBB_SECTOR_COUNT;
    Current_LBA = PartitionRecord->Starting_Sector;
    Table_Index = 0;

    while ( Sectors_Remaining > 0 )
    {

      Convert_To_CHS(Current_LBA , PartitionRecord->Drive_Index, &Cylinder, &Head, &Sector);
      Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head, &Sector );

      BM_Boot_Sector->INT13_Table[Table_Index].CX = (unsigned short) ( ( Cylinder << 8 ) + Sector );
      BM_Boot_Sector->INT13_Table[Table_Index].DX = (unsigned short) ( ( Head << 8 ) + 0x80 + PartitionRecord->Drive_Index );

      BM_Boot_Sector->INT13_Table[Table_Index].AX = 0x0200;

      if ( Sectors_Remaining > DriveArray[PartitionRecord->Drive_Index].Geometry.Sectors )
      {

        BM_Boot_Sector->INT13_Table[Table_Index].AX += (BYTE) DriveArray[PartitionRecord->Drive_Index].Geometry.Sectors;
        Sectors_Remaining -= DriveArray[PartitionRecord->Drive_Index].Geometry.Sectors;
        Current_LBA += DriveArray[PartitionRecord->Drive_Index].Geometry.Sectors;

      }
      else
      {

        BM_Boot_Sector->INT13_Table[Table_Index].AX += (BYTE) Sectors_Remaining;
        Sectors_Remaining = 0;

      }

      Table_Index++;

    }

    /* Now that all of the table entries have been set, we can write Boot Manager to disk. */
    WriteSectors( PartitionRecord->Drive_Index + 1, PartitionRecord->Starting_Sector, MBB_SECTOR_COUNT, &PBOOT, Error_Code);

    if ( *Error_Code != DISKIO_NO_ERROR )
    {

      if ( *Error_Code == DISKIO_WRITE_FAILED )
      {

        /* Mark the drive as having an I/O Error. */
        DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

        *Error_Code = LVM_ENGINE_IO_ERROR;

        return;

      }
      else
      {

#ifdef DEBUG

#ifdef PARANOID

        assert(0);

#else

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;
#endif

#endif

      }

    }

  }

  /* We need to construct the Boot Manager Alias Table and write it to disk. */

  /* Zero out the Boot Manager Alias Table. */
  memset(BM_Alias_Table,0,sizeof(Alias_Table));

  /* To construct the Boot Manager Alias table, we must scan the first 24 drives in the system for primary partitions
     and then create an entry in the Alias table for each primary partition found.                                      */
  for ( Drive_Index = 0; Drive_Index < DriveCount; Drive_Index++ )
  {

    /* The Boot Manager Alias Table has an entry for each of the first 24 disk drives in the system.  Each entry consists
       of 4 records.  Each record is used to hold the information about a primary partition on the corresponding disk.
       We will use Table_Index as an index indicating which of the four records to use for the next primary partition found
       on the corresponding drive.                                                                                           */
    Table_Index = 0;

    /* Scan the drive for primary partitions. */
    ForEachItem(DriveArray[Drive_Index].Partitions, &Build_Alias_Table, &Table_Index, TRUE, Error_Code );

#ifdef DEBUG

#ifdef PARANOID

    assert( *Error_Code == DLIST_SUCCESS );

#else

    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

#endif

#endif

  }

  /* We need to write the Alias Table to disk. The size of the Alias Table is an exact multiple of the sector size. */
  WriteSectors( PartitionRecord->Drive_Index + 1,                             /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                PartitionRecord->Starting_Sector + ALIAS_TABLE_SECTOR_OFFSET,
                SECTORS_PER_ALIAS_TABLE,
                &BM_Alias_Table,
                Error_Code);

  /* Did the write succeed? */
  if ( *Error_Code != DISKIO_NO_ERROR )
  {

    if ( *Error_Code == DISKIO_WRITE_FAILED )
    {

      /* Mark the drive as having an I/O Error. */
      DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

      *Error_Code = LVM_ENGINE_IO_ERROR;

      return;

    }
    else
    {

#ifdef DEBUG

#ifdef PARANOID

      assert(0);

#else

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

#endif

#endif

    }

  }

  /* Do we need to update the Boot Manager options? */

  /* Boot_Manager_Options_Changed will be TRUE anytime that the Boot Manager Options have changed.  However, if we are installing
     a new Boot Manager, then BM_Options will point into our built in copy of Boot Manager, which means that any option changes
     made will have been already written to disk.  Thus, we only have to write the new options to disk if BM_Options points to
     Boot_Manager_Options_Sector, which means that we already have a Boot Manager installed on the disk and we read in its options
     and manipulated them.                                                                                                          */
  if ( ( Boot_Manager_Options_Changed || Upgrade_Boot_Manager ) &&
       ( (ADDRESS) BM_Options == (ADDRESS) &Boot_Manager_Options_Sector )
     )
  {

    WriteSectors( PartitionRecord->Drive_Index + 1, PartitionRecord->Starting_Sector + BOOT_MANAGER_PATH_SECTOR_OFFSET, 1, &Boot_Manager_Options_Sector, Error_Code);

    if ( *Error_Code != DISKIO_NO_ERROR )
    {

      if ( *Error_Code == DISKIO_WRITE_FAILED )
      {

        /* Mark the drive as having an I/O Error. */
        DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

        *Error_Code = LVM_ENGINE_IO_ERROR;

        return;

      }
      else
      {

#ifdef DEBUG

#ifdef PARANOID

        assert(0);

#else

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

#endif

#endif

      }

    }

  }

  /* If we get here, we succeeded! */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Boot_Manager_Is_Installed16                      */
/*                                                                   */
/*   Descriptive Name: Indicates whether or not Boot Manager is      */
/*                     installed on the first or second hard drives  */
/*                     in the system.                                */
/*                                                                   */
/*   Input: CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                    in which to store an error code*/
/*                                    should an error occur.         */
/*                                                                   */
/*   Output: TRUE is returned if Boot Manager is found.              */
/*           FALSE is returned if Boot Manager is not found or if an */
/*           error occurs.                                           */
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
BOOLEAN _Far16 _Pascal _loadds BOOT_MANAGER_IS_INSTALLED16( BOOLEAN * _Seg16 Active, CARDINAL32 * _Seg16 Error_Code)
{

  return Boot_Manager_Is_Installed(Active, Error_Code);

}


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
ADDRESS _Far16 _Pascal _loadds GET_BOOT_MANAGER_HANDLE16( CARDINAL32 * _Seg16 Error_Code)
{

  return Get_Boot_Manager_Handle(Error_Code);

}


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
void _Far16 _Pascal _loadds ADD_TO_BOOT_MANAGER16 ( CARDINAL32 Handle, CARDINAL32 * _Seg16 Error_Code )
{

  Add_To_Boot_Manager( (ADDRESS) Handle, Error_Code);

  return;

}


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
void _Far16 _Pascal _loadds REMOVE_FROM_BOOT_MANAGER16 ( CARDINAL32 Handle, CARDINAL32 * _Seg16 Error_Code )
{

  Remove_From_Boot_Manager( (ADDRESS) Handle, Error_Code );

  return;

}


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
                                                    )
{

  Boot_Manager_Menu   Data;

  Data = Get_Boot_Manager_Menu( Error_Code );

  *Menu_Items = Data.Menu_Items;
  *Count = Data.Count;

  return;

}


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
void _Far16 _Pascal _loadds INSTALL_BOOT_MANAGER16 ( CARDINAL32   Drive_Number, CARDINAL32 * _Seg16 Error_Code )
{

  Install_Boot_Manager( Drive_Number, Error_Code );

  return;

}


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
void _Far16 _Pascal _loadds REMOVE_BOOT_MANAGER16( CARDINAL32 * _Seg16 Error_Code )
{

  Remove_Boot_Manager(Error_Code);

  return;

}


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
void _Far16 _Pascal _loadds SET_BOOT_MANAGER_OPTIONS16( CARDINAL32          Handle,
                                                        BOOLEAN             Timer_Active,
                                                        CARDINAL32          Time_Out_Value,
                                                        BOOLEAN             Advanced_Mode,
                                                        CARDINAL32 * _Seg16 Error_Code
                                                      )
{

  Set_Boot_Manager_Options( (ADDRESS) Handle, Timer_Active, Time_Out_Value, Advanced_Mode, Error_Code );

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_Boot_Manager_Options16                       */
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
void _Far16 _Pascal _loadds GET_BOOT_MANAGER_OPTIONS16( CARDINAL32 * _Seg16 Handle,
                                                        BOOLEAN    * _Seg16 Handle_Is_Volume,
                                                        BOOLEAN    * _Seg16 Timer_Active,
                                                        CARDINAL32 * _Seg16 Time_Out_Value,
                                                        BOOLEAN    * _Seg16 Advanced_Mode,
                                                        CARDINAL32 * _Seg16 Error_Code
                                                      )
{

  Get_Boot_Manager_Options( (ADDRESS) Handle, Handle_Is_Volume, Timer_Active, Time_Out_Value, Advanced_Mode, Error_Code );

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
BOOLEAN Can_Boot_Manager_Be_Installed( CARDINAL32 * Error_Code)
{

  BOOLEAN        Acceptable_Free_Space_Exists = FALSE;
  BOOLEAN        Can_Install = FALSE;
  CARDINAL32     Available_Slots;                      /* Used to calculate how many more primary partitions can be created on a drive. */

  /* Is Boot Manager installed? */
  if ( Boot_Manager_Handle != NULL )
  {

    /* Since Boot Manager is installed, we won't install it again. */
    Can_Install = FALSE;

  }
  else
  {

    /* Since Boot Manager is not installed, we may be able to install it. */

    /* Calculate the maximum number of primary partitions that can be made on Drive 0. */
    if ( DriveArray[0].Logical_Partition_Count > 0 )
      Available_Slots = 3;
    else
      Available_Slots = 4;

    /* Can we create a primary partition on physical disk 1? */
    if ( ( ! DriveArray[0].Corrupt || DriveArray[0].NonFatalCorrupt) &&
         ( DriveArray[0].Primary_Partition_Count < Available_Slots )
       )
    {

      /* We must find out if there is a block of free space on physical disk 1 that can be
         turned into a primary partition that is at least 1 MB in size.                      */
      ForEachItem(DriveArray[0].Partitions, &Find_Free_Space_For_BM, &Acceptable_Free_Space_Exists, TRUE, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

      assert( *Error_Code == DLIST_SUCCESS );

#else

      if ( *Error_Code != DLIST_SUCCESS )
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return FALSE;

      }

#endif

#endif

    }

    if ( Acceptable_Free_Space_Exists )
      Can_Install = TRUE;
    else
    {

      /* Boot Manager can also be installed on the second physical disk.  Lets look there. */

      /* Is there a second disk? */
      if ( DriveCount > 1 )
      {

        /* Calculate the maximum number of primary partitions that can be made on Drive 1. */
        if ( DriveArray[1].Logical_Partition_Count > 0 )
          Available_Slots = 3;
        else
          Available_Slots = 4;

        /* Can we create a primary partition on physical disk 1? */
        if ( ( ! DriveArray[1].Corrupt ) &&
             ( DriveArray[1].Primary_Partition_Count < Available_Slots )
           )
        {

          /* We must find out if there is a block of free space on physical disk 2 that can be
             turned into a primary partition that is at least 1 MB in size.                      */
          ForEachItem(DriveArray[1].Partitions, &Find_Free_Space_For_BM, &Acceptable_Free_Space_Exists, TRUE, Error_Code);

  #ifdef DEBUG

  #ifdef PARANOID

          assert( *Error_Code == DLIST_SUCCESS );

  #else

          if ( *Error_Code != DLIST_SUCCESS )
          {

            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

            return FALSE;

          }

  #endif

  #endif

          if ( Acceptable_Free_Space_Exists )
            Can_Install = TRUE;
        }

      }

    }

  }

  return Can_Install;

}



/*--------------------------------------------------
 * Private functions.
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
static void _System Find_Boot_Manager(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our parameters without having to typecase all the time. */
  Find_Boot_Manager_Control_Data  *   Control_Data = (Find_Boot_Manager_Control_Data *) Parameters;

  /* Declare a variable for use when checking for the Boot Manager signature in the Boot Sector. */
  struct Extended_Boot * Boot_Sector;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a primary partition?  Boot Manager is always a primary partition. */
  if ( ( PartitionRecord->Partition_Type == Partition ) &&
       ( PartitionRecord->Primary_Partition == TRUE )
     )
  {

    /* We have a primary partition.  Is this the Boot Manager Partition?
       Note:  On the first drive, Boot Manager must be in an active primary partition to be operational.  If
              Boot Manager is installed on the second drive, he does NOT have to be marked active to be operational.
              This is because the boot code in the MBR does not check the Boot_Indicator for a Boot Manager
              partition if that partition is not on the first drive.                                                  */
    if ( PartitionRecord->Partition_Table_Entry.Format_Indicator == BOOT_MANAGER_INDICATOR )
    {

      /* We must look for the magic signature. */

      /* Read in the Boot Sector for the partition. */
      ReadSectors( PartitionRecord->Drive_Index + 1,          /* OS/2's drive numbers are 1 based whereas our DriveArray is 0 based.  Add 1 to Index to translate. */
                   PartitionRecord->Starting_Sector,
                   1,                                          /* We only want the Boot Sector. */
                   &Sector_Buffer,
                   Error);

      if ( *Error == DISKIO_NO_ERROR )
      {

        /* We need to view the BPB in the Boot Sector in order to see the Boot Manager magic signature.  Set up to do so. */
        Boot_Sector = (struct Extended_Boot *) &Sector_Buffer;

        /* Do we have the magic signature? */
        if ( strncmp(Boot_Sector->Boot_OEM, BOOT_MANAGER_SIGNATURE, strlen(BOOT_MANAGER_SIGNATURE) ) == 0 )
        {

          /* Check the version of Boot Manager against our copy. */
          if ( Boot_Sector->Boot_OEM[6] < MBB_VERSION )
            Control_Data->Non_Current_Boot_Manager = TRUE;
          else
            Control_Data->Non_Current_Boot_Manager = FALSE;

          /* Are we looking for the active Boot Manager? */
          if ( Control_Data->Find_Active_Boot_Manager )
          {

            if ( ( PartitionRecord->Partition_Table_Entry.Boot_Indicator == ACTIVE_PARTITION) || ( PartitionRecord->Drive_Index > 0 ) )
            {

              /* We have an active Boot Manager!  Save a pointer to this Partition_Data record. */
              Control_Data->Boot_Manager_Partition = PartitionRecord;

              /* We have found what we were looking for, so abort the search and return success. */
              *Error = DLIST_SEARCH_COMPLETE;

              return;

            }

          }
          else
          {

            /* Save a pointer to this Partition_Data record. */
            Control_Data->Boot_Manager_Partition = PartitionRecord;

            /* We have found what we were looking for, so abort the search and return success. */
            *Error = DLIST_SEARCH_COMPLETE;

            return;

          }

        }

      }
      else
      {

        /* Was this an I/O error?  If so, mark the Drive Array accordingly. */
        if ( *Error == DISKIO_READ_FAILED )
        {

          DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

        }
        else
        {

#ifdef DEBUG

#ifdef PARANOID

          assert(0);

#else

          /* We have an internal error!  Indicate a corrupted list.  This will abort out list traversal. */
          *Error = DLIST_CORRUPTED;

          return;
#endif

#endif

        }

      }

    }

  }

  /* Indicate success. */
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
static void _System Remove_From_Boot_Manager_Menu(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition on the Boot Manager Menu? */
    if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
    {

      /* Take this partition off of the Boot Manager Menu. */
      PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;

      /* Indicate that changes were made to the drive. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

    /* Is this partition marked installable? */
    if ( PartitionRecord->DLA_Table_Entry.Installable )
    {

      /* To remain installable without Boot Manager, the partition must be a primary partition
         on the first drive in the system and have a drive letter preference of 'C'.  If it meets
         these requirements, we will mark it startable and leave it marked installable.  If it does
         not meet these requirements, we will clear the Installable flag.                               */
      if ( PartitionRecord->Primary_Partition &&
           ( PartitionRecord->Drive_Index == 0 ) &&
           ( PartitionRecord->DLA_Table_Entry.Drive_Letter == 'C' ) &&
           ( PartitionRecord->Volume_Handle != NULL )
         )
      {

        /* This partition is still installable!  Lets make it startable. */
        Set_Startable( PartitionRecord->External_Volume_Handle, Error);

        if ( *Error != LVM_ENGINE_NO_ERROR )
        {

          /* We could not mark it startable!  It may be above the 1024 cylinder limit.  Either way, this partition is no longer installable! */
          PartitionRecord->DLA_Table_Entry.Installable = FALSE;

          /* This partition should have been part of the volume marked installable.  Since
             it is nolonger installable, update the Install_Volume_Handle.                     */
          if ( PartitionRecord->Volume_Handle == Install_Volume_Handle )
            Install_Volume_Handle = NULL;

          /* Indicate that changes were made to the drive. */
          DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

        }

      }
      else
      {

        /* This partition is no longer installable!  Update its fields! */
        PartitionRecord->DLA_Table_Entry.Installable = FALSE;

        /* This partition should have been part of the volume marked installable.  Since
           it is nolonger installable, update the Install_Volume_Handle.                     */
        if ( PartitionRecord->Volume_Handle == Install_Volume_Handle )
          Install_Volume_Handle = NULL;

        /* Indicate that changes were made to the drive. */
        DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      }

    }

  }

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
static void _System Get_Menu_Items(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  Boot_Manager_Menu * Menu = (Boot_Manager_Menu *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition on the Boot Manager Menu? */
    if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
    {

      /* If Menu->Menu_Items is NULL, then we are just counting items on the Boot Manger Menu. */
      if ( Menu->Menu_Items != NULL )
      {

        /* Add this partition to the Menu_Items array. */

        /* Is this partition part of a Volume?  If the Volume_Handle is not NULL, then this is part of a volume. */
        if ( PartitionRecord->Volume_Handle != NULL )
        {

          /* Since this is a volume, use the volume's external handle. */
          Menu->Menu_Items[Menu->Count].Handle = PartitionRecord->External_Volume_Handle;

          /* Indicate that we have a volume. */
          Menu->Menu_Items[Menu->Count].Volume = TRUE;
        }
        else
        {

          /* Since this is a partition, use the partition's external handle. */
          Menu->Menu_Items[Menu->Count].Handle = PartitionRecord->External_Handle;

          /* Indicate that this is a partition. */
          Menu->Menu_Items[Menu->Count].Volume = FALSE;

        }

      }

      /* Count the current partition since it is on the Boot Manager Menu. */
      Menu->Count++;

    }

  }

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
static void _System Make_Primaries_Inactive(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this a primary partition? */
    if ( PartitionRecord->Primary_Partition )
    {

      /* Make it inactive by setting the Active bit to 0 in the Boot_Indicator. */
      PartitionRecord->Partition_Table_Entry.Boot_Indicator = PartitionRecord->Partition_Table_Entry.Boot_Indicator & (~ACTIVE_PARTITION);

      /* Mark the drive containing this partition as having had something change. */
      DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

    }

  }

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
static void _System Find_Free_Space_For_BM(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  BOOLEAN *  Acceptable_Free_Space_Found = (BOOLEAN *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  CARDINAL32       PartitionOptions;   /* Used to hold the allowable options for the partition being investigated. */

  CARDINAL32       Min_Partition_Size; /* Used to determine how big a partition Boot Manager must reside in. */

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent Free Space? */
  if ( PartitionRecord->Partition_Type == FreeSpace )
  {

    /* Calculate the minimum partition size for Boot Manager. */
    Min_Partition_Size = DriveArray[PartitionRecord->Drive_Index].Sectors_Per_Cylinder - DriveArray[PartitionRecord->Drive_Index].Geometry.Sectors;
    if (Min_Partition_Size < BOOT_MANAGER_SIZE)
      Min_Partition_Size = BOOT_MANAGER_SIZE;

    /* Is this block of free space large enough to hold Boot Manager? */
    if ( PartitionRecord->Partition_Size >= Min_Partition_Size )
    {

      /* Would a Boot Manager Partition created at the start of the block of free space be below the 1024 cylinder limit, if that
         limit applies?                                                                                                            */
      if ( ( ! DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit_Applies ) ||
           ( ( PartitionRecord->Starting_Sector + Min_Partition_Size ) <= DriveArray[PartitionRecord->Drive_Index].Cylinder_Limit )
         )
      {

        /* Can this block of free space be turned into a primary partition? */
        PartitionOptions = Get_Partition_Options(PartitionRecord, Error);

        /* Did we succeed? */
        if ( *Error != LVM_ENGINE_NO_ERROR )
          return;

        /* Can we create a primary partition from this block of free space? */
        if ( ( PartitionOptions & CREATE_PRIMARY_PARTITION ) != 0 )
        {

          /* We have found an acceptable block of free space for Boot Manager to be installed into. */
          *Acceptable_Free_Space_Found = TRUE;

          /* Indicate that we found what we were looking for and return. */
          *Error = DLIST_SEARCH_COMPLETE;

          return;

        }

      }

    }

  }

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
static void _System Find_Matching_Partition_Name(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  char temp_name[ALIAS_NAME_LENGTH_2];   /* temporary name var for comparing padded names */
  BOOLEAN match;                         /* boolean for match found bool */
  int i;                                 /* loop counter */

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  ADDRESS *  Handle = (ADDRESS *) Handle;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;


#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a real partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Is this partition on the Boot Manager menu? */
    if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
    {
/*
 * #if ALIAS_NAME_LENGTH < PARTITION_NAME_SIZE
 *
 *      if ( strncmp(PartitionRecord->Partition_Name, BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, ALIAS_NAME_LENGTH ) == 0 )
 * #else
 *      if ( strncmp(PartitionRecord->Partition_Name, BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, PARTITION_NAME_SIZE ) == 0 )
 * #endif
 */

      match = TRUE;     /* init match found to TRUE */

      memcpy(temp_name, PartitionRecord->Partition_Name,
             ALIAS_NAME_LENGTH_2);
      for (i=0;i<ALIAS_NAME_LENGTH_2;i++)
      {
        if (temp_name[i] == 0)
          break;
      }
      memset(&temp_name[i], ' ',ALIAS_NAME_LENGTH_2 - i );  /* padd with blanks */

      /* compare the old 8 char default alias with the 1st 8 chars of the new */
      /* 20 char default alias.  If they are the same, then use the new alias */
      /* field.  If not, we assume that the default has been modified by an   */
      /* old utility and we will use the short alias.                  slp    */
      if (strncmp(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
                  BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
                  ALIAS_NAME_LENGTH))
      {
        /* names do not match, use short name */
        if (strncmp(temp_name,BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
                ALIAS_NAME_LENGTH))
        {
          match = FALSE;
        }
      }
      else      /* use the new 20 char alias field */
      {
        if (strncmp(temp_name,BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
                ALIAS_NAME_LENGTH_2))
        {
          match = FALSE;
        }
      }

      if (match)
      {

        /* We have a match!  Save this partition's external handle. */
        *Handle = PartitionRecord->External_Handle;

        /* Indicate that we found what we were looking for and return. */
        *Error = DLIST_SEARCH_COMPLETE;

      }

    }

  }

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
static void _System Find_Matching_Volume_Name(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  char temp_name[ALIAS_NAME_LENGTH_2];   /* temporary name var for comparing padded names */
  BOOLEAN match;                         /* boolean for match found bool */
  int i;                                 /* loop counter */

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  ADDRESS * Handle = (ADDRESS *) Parameters;

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data * VolumeRecord = (Volume_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Is this volume on the Boot Manager Menu? */
  if ( VolumeRecord->On_Boot_Manager_Menu )
  {

/*
 * #if ALIAS_NAME_LENGTH < VOLUME_NAME_SIZE
 *
 *    if ( strncmp(VolumeRecord->Volume_Name, BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, ALIAS_NAME_LENGTH ) == 0 )
 *
 * #else
 *
 *   if ( strncmp(VolumeRecord->Volume_Name, BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name, VOLUME_NAME_SIZE ) == 0 )
 *
 * #endif
 */

    match = TRUE;     /* init match found to TRUE */

    memcpy(temp_name, VolumeRecord->Volume_Name,
           ALIAS_NAME_LENGTH_2);
    for (i=0;i<ALIAS_NAME_LENGTH_2;i++)
    {
      if (temp_name[i] == 0)
        break;
    }
    memset(&temp_name[i], ' ',ALIAS_NAME_LENGTH_2 - i );  /* padd with blanks */

    /* compare the old 8 char default alias with the 1st 8 chars of the new */
    /* 20 char default alias.  If they are the same, then use the new alias */
    /* field.  If not, we assume that the default has been modified by an   */
    /* old utility and we will use the short alias.                  slp    */
    if (strncmp(BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
                BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
                ALIAS_NAME_LENGTH))
    {
      /* names do not match, use short name */
      if (strncmp(temp_name,BM_Options->Alias_Array[DEFAULT_ALIAS_ENTRY].Name,
              ALIAS_NAME_LENGTH))
      {
        match = FALSE;
      }
    }
    else      /* use the new 20 char alias field */
    {
      if (strncmp(temp_name,BM_Options->Alias_Array_2[DEFAULT_ALIAS_ENTRY].Name,
              ALIAS_NAME_LENGTH_2))
      {
        match = FALSE;
      }
    }


    if (match)
    {

      /* We have a match!  Save this partition's external handle. */
      *Handle = VolumeRecord->External_Handle;

      /* Indicate that we found what we were looking for and return. */
      *Error = DLIST_SEARCH_COMPLETE;

      return;

    }

  }

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
static void _System Remove_Boot_Manager_Menu_Flag(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data * VolumeRecord = (Volume_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != VOLUME_DATA_TAG ) || ( ObjectSize != sizeof(Volume_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Volume_Data. */

  /* Make sure this volume is NOT on the boot manager menu. */
  VolumeRecord->On_Boot_Manager_Menu = FALSE;

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
static void _System Migrate_Old_BM_Menu (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

#define MAX_COUNT_CHARACTERS  10

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  Volume_Data *    VolumeRecord;    /* Used when migrating old Boot Manager Menu items to the new LVM format when
                                       the partition specified in the Old Boot Manager Menu data is an existing
                                       LVM Compatibility volume.                                                   */
  char             New_Volume_Name[VOLUME_NAME_SIZE];       /* Used when creating a new LVM Compatibility Volume. */
  char             New_Partition_Name[PARTITION_NAME_SIZE]; /* Used when assigning a name to a partition as part of creating a new LVM Compatibility Volume. */
  char             Name_Count_Buffer[MAX_COUNT_CHARACTERS]; /* Used when converting Name_Count to an ASCII string. */
  CARDINAL32       Name_Count;                              /* Used for resolving duplicate name problems. */
  CARDINAL32       Partition_Index; /* Used to walk the Alias Table. */
  CARDINAL32       Cylinder;        /* Used to calculate the cylinder value of the CHS address of the partition. */
  CARDINAL32       Head;            /* Used to calculate the head value of the CHS address of the partition. */
  CARDINAL32       Sector;          /* Used to calculate the sector value of the CHS address of the partition. */
  BOOLEAN          Continue_Migration; /* Used to indicate whether or not the migration of the current partition should continue. */

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a primary partition?  */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Assume that the partition we are examining will not be migrated. */
    Continue_Migration = FALSE;

    /* What kind of a partition do we have? */
    if ( ! PartitionRecord->Primary_Partition )
    {

      /* We have a logical drive here.  Should we continue the migration process? */
      Continue_Migration = PartitionRecord->Migration_Needed;

    }
    else
    {

      /* We have a primary partition.  Does it appear in the Boot Manager Alias Table? */

      /* In the Alias Table, the CHS version of the starting address of the partition is stored.  We must convert the
         starting address of the partition into CHS format so that we can find it in the Alias Table.                    */
      Convert_To_CHS(PartitionRecord->Starting_Sector, PartitionRecord->Drive_Index, &Cylinder, &Head, &Sector);
      Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head, &Sector );

      /* Now check the Alias Table for a match. */
      for ( Partition_Index = 0; Partition_Index < ALIAS_TABLE_PARTITION_LIMIT; Partition_Index++ )
      {

        if ( BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Bootable &&
             ( BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Head == Head ) &&
             ( BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Sector == Sector ) &&
             ( BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Cylinder == Cylinder ) &&
             ( strncmp(BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name, ALIAS_TABLE_ENTRY_MIGRATION_TEXT, ALIAS_NAME_SIZE) != 0 )   /* Has it been migrated already? */
           )
        {

          /* We have a match! */
          Continue_Migration = TRUE;

          break;

        }

      }

    }

    /* Should we continue the migration?  This means that, for a primary, an Alias Table entry was found.  For a logical drive,
       it means that the Partition Manager flagged the partition as requiring migration.                                         */
    if ( Continue_Migration )
    {

      /* Is the partition already part of a volume? */
      if ( PartitionRecord->Volume_Handle != NULL )
      {

        /* This partition is part of a volume.  We must check to see if the volume is a compatibility volume.  If it is,
           then we must mark it as being on the boot manager menu.  If the partition is a primary partition, then
           we must change its entry in the Alias Table to "--> LVM" and turn off its Bootable flag in the Alias Table.
           If it is not a compatibility volume, then we must remove its name from the Alias Table and change the Bootable
           flag to FALSE.  For Logical Drives which are not compatibility volumes, we must ensure that they are removed
           from the Boot Manager Menu.                                                                                     */

        VolumeRecord = (Volume_Data *) GetObject(Volumes, sizeof(Volume_Data), VOLUME_DATA_TAG, PartitionRecord->Volume_Handle, TRUE, Error);

#ifdef DEBUG

#ifdef PARANOID

        assert( *Error == DLIST_SUCCESS );

#else

        if ( *Error != DLIST_SUCCESS )
        {

          *Error = DLIST_CORRUPTED;

          return;

        }

#endif

#endif

        /* Is this volume a compatibility volume? */
        if ( VolumeRecord->Compatibility_Volume )
        {

          /* Is this volume already on the Boot Manager Menu? */
          if ( ! VolumeRecord->On_Boot_Manager_Menu )
          {

            Add_To_Boot_Manager(VolumeRecord->External_Handle, Error);

          }

        }

        /* Mark the drive as having had changes made.  This will cause the EBR's to be recreated as well as any Boot Manager Aliases they may contain. */
        DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

      }
      else
      {

        /* The partition does not belong to a volume! */

        /* We must set the partition's name.  The partition's name can not be a duplicate of another partition which resides on the
           same physical disk.  We will use the Set_Name function to set the partition's name as this function will also check
           for any duplicate names which may exist.                                                                                   */

        /* Set up the new partition name. */
        memset(New_Partition_Name, 0, PARTITION_NAME_SIZE);

        if ( PartitionRecord->Primary_Partition )
          strncpy( New_Partition_Name, BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name, ALIAS_NAME_LENGTH);
        else
          strncpy( New_Partition_Name, PartitionRecord->DLA_Table_Entry.Partition_Name, PARTITION_NAME_SIZE);

        /* Set up the Name_Count in case we must create a unique name from a duplicate name. */
        Name_Count = 1;

        do
        {

          /* We must set the name of this partition. */
          Set_Name(PartitionRecord->External_Handle, New_Partition_Name, Error);

          if ( *Error != LVM_ENGINE_NO_ERROR )
          {

            /* We can not set the partition's name!  The name we have must be either invalid or a duplicate of an existing partition name. */
            switch ( *Error )
            {

              case LVM_ENGINE_DUPLICATE_NAME : /* We already have a partition with the same name!  */

                                               /* Set up the new partition name again. */
                                               memset(New_Partition_Name, 0, PARTITION_NAME_SIZE);

                                               if ( PartitionRecord->Primary_Partition )
                                                 strncpy( New_Partition_Name, BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name, ALIAS_NAME_LENGTH);
                                               else
                                                 strncpy( New_Partition_Name, PartitionRecord->DLA_Table_Entry.Partition_Name, PARTITION_NAME_SIZE);

                                               /* We must create a unique name.  We will take the Name_Count and append it to the
                                                  original partition name.                                                            */
                                               sprintf(Name_Count_Buffer,"%d",Name_Count);

                                               /* Adjust the length of New_Partition_Name so that we can safely append Name_Count_Buffer. */
                                               if ( ( strlen(New_Partition_Name) + strlen(Name_Count_Buffer) ) >= PARTITION_NAME_SIZE )
                                               {

                                                 /* We must truncate New_Partition_Name to make room for Name_Count_Buffer. */
                                                 New_Partition_Name[PARTITION_NAME_SIZE - strlen(Name_Count_Buffer) - 1] = 0;

                                               }

                                               /* Now add Name_Count_Buffer to New_Partition_Name. */
                                               strcat(New_Partition_Name,Name_Count_Buffer);

                                               /* Increment the Name_Count. */
                                               Name_Count++;

                                               /* Ensure that *Error does not equal DLIST_SUCCESS so that we will not
                                                  exit the do-while loop.  This will cause the Set_Name call to be
                                                  repeated, hopefully with better results this time.                     */
                                               *Error = DLIST_CORRUPTED;

                                               break;

              case LVM_ENGINE_BAD_NAME : /* The name was incorrect in some way.  We must create a name for this partition. */

                                         /* Following the KISS principal (Keep It Simple, Stupid! ), we will use the letter P as
                                            the name of the new partition.                                                           */

                                         /* Set up the new partition name. */
                                         memset(New_Partition_Name, 0, PARTITION_NAME_SIZE);
                                         New_Partition_Name[0] = 'P';

                                         /* If this is a primary partition, get rid of its bogus Boot Manager Alias Table Entry. */
                                         if ( PartitionRecord->Primary_Partition )
                                         {

                                           memset(BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name, 0, ALIAS_NAME_LENGTH );
                                           BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name[0] = 'P';

                                         }
                                         else
                                         {

                                           memset(PartitionRecord->DLA_Table_Entry.Partition_Name, 0, PARTITION_NAME_SIZE );
                                           PartitionRecord->DLA_Table_Entry.Partition_Name[0] = 'P';

                                         }

                                         /* Ensure that *Error does not equal DLIST_SUCCESS so that we will not
                                            exit the do-while loop.  This will cause the Set_Name call to be
                                            repeated, hopefully with better results this time.                     */
                                         *Error = DLIST_CORRUPTED;

                                         break;

              default : /* Something's not right! */
                        *Error = LVM_ENGINE_INTERNAL_ERROR;
                        return;
                        break;   /* Keep the compiler happy. */

            }

          }
          else
          {

            /* Set *Error to DLIST_SUCCESS so that we will exit the do-while loop. */
            *Error = DLIST_SUCCESS;

          }

        } while ( *Error != DLIST_SUCCESS );

        /* Now we must create the new, hidden, compatibility volume. */

        /* Set up the new volume name. */
        memset(New_Volume_Name, 0, VOLUME_NAME_SIZE);

        if ( PartitionRecord->Primary_Partition )
          strncpy( New_Volume_Name, BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name, ALIAS_NAME_LENGTH);
        else
          strncpy( New_Volume_Name, PartitionRecord->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);

        /* Set up the Name_Count in case we must create a unique name from a duplicate name. */
        Name_Count = 1;

        do
        {

          /* We must turn this partition into a hidden compatibility volume. */
          Create_Volume2(New_Volume_Name,
                         FALSE,
                         TRUE,
                         0,
                         0,
                         NULL,
                         1,
                         &(PartitionRecord->External_Handle),
                         Error);

          if ( *Error != LVM_ENGINE_NO_ERROR )
          {

            /* We can not create a bootable volume!  This partition must not be bootable or we are out of memory or worse! */
            switch ( *Error )
            {

              case LVM_ENGINE_MAX_PARTITIONS_ALREADY_IN_USE :
              case LVM_ENGINE_OUT_OF_MEMORY                 : *Error = DLIST_OUT_OF_MEMORY;
                                                              return;
                                                              break;      /* Keep the compiler happy. */

              case LVM_ENGINE_1024_CYLINDER_LIMIT :
              case LVM_ENGINE_SELECTED_PARTITION_NOT_BOOTABLE : /* This partition is not Bootable! */

                                                                /* Mark the drive as having had changes made.  This will cause the EBR's to be recreated as well as any Boot Manager Aliases they may contain. */
                                                                DriveArray[PartitionRecord->Drive_Index].ChangesMade = TRUE;

                                                                /* Set *Error to DLIST_SUCCESS in order to exit the while loop. */
                                                                *Error = DLIST_SUCCESS;
                                                                break;

              case LVM_ENGINE_DUPLICATE_NAME : /* We already have a volume with the same name!  */

                                               /* Set up the new volume name again. */
                                               memset(New_Volume_Name, 0, VOLUME_NAME_SIZE);

                                               if ( PartitionRecord->Primary_Partition )
                                                 strncpy( New_Volume_Name, BM_Alias_Table[PartitionRecord->Drive_Index][Partition_Index].Name, ALIAS_NAME_LENGTH);
                                               else
                                                 strncpy( New_Volume_Name, PartitionRecord->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);

                                               /* We must create a unique name.  We will take the Name_Count and append it to the
                                                  original volume name.                                                            */
                                               sprintf(Name_Count_Buffer,"%d",Name_Count);

                                               /* Adjust the length of New_Volume_Name so that we can safely append Name_Count_Buffer. */
                                               if ( ( strlen(New_Volume_Name) + strlen(Name_Count_Buffer) ) >= VOLUME_NAME_SIZE )
                                               {

                                                 /* We must truncate New_Volume_Name to make room for Name_Count_Buffer. */
                                                 New_Volume_Name[VOLUME_NAME_SIZE - strlen(Name_Count_Buffer) - 1] = 0;

                                               }

                                               /* Now add Name_Count_Buffer to New_Volume_Name. */
                                               strcat(New_Volume_Name,Name_Count_Buffer);

                                               /* Increment the Name_Count. */
                                               Name_Count++;

                                               /* Ensure that *Error does not equal DLIST_SUCCESS so that we will not
                                                  exit the do-while loop.  This will cause the Create_Volume call to be
                                                  repeated, hopefully with better results this time.                     */
                                               *Error = DLIST_CORRUPTED;

                                               break;

              case LVM_ENGINE_BAD_NAME : /* The name was incorrect in some way.  We must create a name for this volume. */

                                         /* Following the KISS principal (Keep It Simple, Stupid! ), we will use the letter V as
                                            the name of the new volume.                                                           */

                                         /* Set up the new volume name. */
                                         memset(New_Volume_Name, 0, VOLUME_NAME_SIZE);
                                         New_Volume_Name[0] = 'V';

                                         /* Ensure that *Error does not equal DLIST_SUCCESS so that we will not
                                            exit the do-while loop.  This will cause the Create_Volume call to be
                                            repeated, hopefully with better results this time.                     */
                                         *Error = DLIST_CORRUPTED;

                                         break;

              default : /* Something's not right! */
                        *Error = LVM_ENGINE_INTERNAL_ERROR;
                        return;
                        break;   /* Keep the compiler happy. */

            }

          }
          else
          {

            /* Set *Error to DLIST_SUCCESS so that we will exit the do-while loop. */
            *Error = DLIST_SUCCESS;

          }

        } while ( *Error != DLIST_SUCCESS );

      }

    }

  }

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
static void _System Build_Alias_Table (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Delcare a local variable so that we can access our parameters without having to typecast each time. */
  CARDINAL32 *     Partition_Index = (CARDINAL32 *) Parameters;

  CARDINAL32       Cylinder;        /* Used to calculate the cylinder value of the CHS address of the partition. */
  CARDINAL32       Head;            /* Used to calculate the head value of the CHS address of the partition. */
  CARDINAL32       Sector;          /* Used to calculate the sector value of the CHS address of the partition. */

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a primary partition? */
  if ( ( PartitionRecord->Partition_Type == Partition ) && ( PartitionRecord->Primary_Partition ) )
  {

    /* We have a primary partition!  We must make an entry in the Alias Table for it. */

    /* In the Alias Table, the CHS version of the starting address of the partition is stored.  We must convert the
       starting address of the partition into CHS format so that we can store it in the Alias Table.                    */
    Convert_To_CHS(PartitionRecord->Starting_Sector, PartitionRecord->Drive_Index, &Cylinder, &Head, &Sector);
    Convert_CHS_To_Partition_Table_Format( &Cylinder, &Head, &Sector );

    /* Store the CHS values in the Boot Manager Alias Table. */
    BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Head = Head;
    BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Sector = Sector;
    BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Cylinder = Cylinder;

    /* Store the BIOS drive number in the Boot Manager Alias Table. */
    BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Drive = PartitionRecord->Drive_Index + 0x80;

    /* Is this primary partition on the Boot Manager Menu? */
    if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
    {

      /* Set the Bootable flag in the alias table entry for this partition. */
      BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Bootable = TRUE;

      /* Now we must set the name field.  */
      if ( PartitionRecord->DLA_Table_Entry.Installable )
        strncpy(BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Name, ALIAS_TABLE_ENTRY_MIGRATION_TEXT2, ALIAS_NAME_SIZE);
      else
        strncpy(BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Name, ALIAS_TABLE_ENTRY_MIGRATION_TEXT, ALIAS_NAME_SIZE);

    }
    else
      BM_Alias_Table[PartitionRecord->Drive_Index][*Partition_Index].Bootable = FALSE;



    /* Increment the Partition_Index so that the next primary partition we find does not try to use the same alias table
       entry as this primary partition.                                                                                   */
    *Partition_Index = *Partition_Index + 1;

  }


}
