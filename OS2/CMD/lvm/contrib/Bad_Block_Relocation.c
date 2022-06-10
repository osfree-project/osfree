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
 * Module: Bad_Block_Relocation.c
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
 * Notes: None.
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
#include "gbltypes.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */

#include "LVM_Constants.h"   /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */
#include "LVM_Interface.h"

#include "Bad_Block_Relocation.h"

#ifdef DEBUG

#ifdef PARANOID

#include <assert.h>   /* assert */

#endif

#endif


/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
/* The following define is calculated as follows:

         MAX_SECTORS_IN_BBR_TABLE = (BBR_LIMIT / BBR_TABLE_ENTRIES_PER_SECTOR)
         if ( (BBR_LIMIT % BBR_TABLE_ENTRIES_PER_SECTOR) > 0)
           MAX_SECTORS_IN_BBR_TABLE += 1;

   Now that we know the number of sectors required to hold the BBR Table, add 1 for the BBR First Sector.

         MAX_SECTORS_IN_BBR_TABLE += 1;

*/
#define MAX_SECTORS_IN_BBR_TABLE   68

/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/
typedef struct _BBR_Data_Record {
                                  CARDINAL32        BBR_Table_Size;
                                  CARDINAL32        Entries_In_Use;
                                  CARDINAL32        Sequence_Number;
                                  CARDINAL32        Feature_Sequence_Number;
                                  BBR_Table_Entry * BBR_Table;
                                  LVM_Classes       Actual_Class;
                                  BOOLEAN           ChangesMade;
                                  BOOLEAN           Top_Of_Class;
                                } BBR_Data_Record;

typedef struct {
                 LVM_Classes    Actual_Class;
                 CARDINAL32     Feature_Sequence_Number;
                 BOOLEAN        Top_Of_Class;
               } Create_BBR_Data_Record;


/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static BYTE                       Feature_Data_Buffer1[BYTES_PER_SECTOR * MAX_SECTORS_IN_BBR_TABLE ];
static BYTE                       Feature_Data_Buffer2[BYTES_PER_SECTOR * MAX_SECTORS_IN_BBR_TABLE ];
static Plugin_Function_Table_V1   Function_Table;
static LVM_Common_Services_V1  *  LVM_Common_Services;
static Feature_ID_Data            Feature_ID_Record;
static BOOLEAN                    Feature_Is_Open = FALSE;


/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
static void    _System Create_BBR_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static void    _System Remove_BBR_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static BOOLEAN _System Find_BBR_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code);
static BOOLEAN Feature_Data_Is_Valid( BOOLEAN  Use_Primary_Buffer, CARDINAL32  Sectors_To_Validate );
static void    Extract_BBR_Table(Partition_Data * PartitionRecord, BOOLEAN Use_Primary_Buffer, BOOLEAN Old_BBR_Format);
static void    _System Discover_BBR_Partitions( DLIST  Partition_List, CARDINAL32 * Error_Code );
static void    _System Create_BBR_Volume( DLIST Partition_List,
                                          ADDRESS VData,
                                          ADDRESS Init_Data,
                                          void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                          LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                          BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                          CARDINAL32   Sequence_Number,
                                          CARDINAL32 * Error_Code );
static void    _System Open_BBR( CARDINAL32 * Error_Code );
static void    _System Close_BBR( void );
static BOOLEAN _System Can_Expand_BBR_Volume( ADDRESS AData, CARDINAL32 * Feature_ID, CARDINAL32 * Error_Code );
static void    _System Add_BBR_Partition ( ADDRESS AData, ADDRESS PData, CARDINAL32 * Error_Code );
static void    _System Delete_BBR_Partition( ADDRESS AData, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code);
static void    _System Commit_BBR_Changes( ADDRESS VolumeRecord, ADDRESS PData, CARDINAL32 * Error_Code );
static void    _System BBR_Write( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code);
static void    _System BBR_Read( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code);
static void    _System Remove_Features(ADDRESS Aggregate, CARDINAL32 * Error_Code);
static void    _System PassThru( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code );
static void    _System ReturnCurrentClass( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number );
static BOOLEAN _System BBR_ChangesPending(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code);
static void    _System BBR_ParseCommandLineArguments(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code );


/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

void BBR_Get_Required_LVM_Version( CARDINAL32 * Major_Version_Number, CARDINAL32 * Minor_Version_Number)
{

  *Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
  *Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;

  return;

}

ADDRESS BBR_Exchange_Function_Tables( ADDRESS Common_Services )
{

  /* Initialize the Feature_ID_Record. */
  strcpy(Feature_ID_Record.Name, "Bad Block Relocation");
  strcpy(Feature_ID_Record.Short_Name, "BBR");
  strcpy(Feature_ID_Record.OEM_Info, "IBM");
  Feature_ID_Record.ID = BBR_FEATURE_ID;
  Feature_ID_Record.Major_Version_Number = BBR_FEATURE_MAJOR_VERSION;
  Feature_ID_Record.Minor_Version_Number = BBR_FEATURE_MINOR_VERSION;
  Feature_ID_Record.LVM_Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
  Feature_ID_Record.LVM_Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;
  Feature_ID_Record.Preferred_Class = Partition_Class;
  Feature_ID_Record.ClassData[Partition_Class].ClassMember = TRUE;
  Feature_ID_Record.ClassData[Partition_Class].GlobalExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].TopExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].BottomExclusive = TRUE;
  Feature_ID_Record.ClassData[Partition_Class].ClassExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].Weight_Factor = 1;
  Feature_ID_Record.ClassData[Aggregate_Class].ClassMember = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].GlobalExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].TopExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].BottomExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].ClassExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].Weight_Factor = 1;
  Feature_ID_Record.ClassData[Volume_Class].ClassMember = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].GlobalExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].TopExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].BottomExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].ClassExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].Weight_Factor = 1;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Create_and_Configure = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Create_and_Configure = NULL;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Display_Status = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Display_Status = NULL;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Control_Panel = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Control_Panel = NULL;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Help_Panel = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Help_Panel = NULL;
  Feature_ID_Record.Interface_Support[Java_Interface].Java_Interface_Class = "";
  Feature_ID_Record.Interface_Support[PM_Interface].Interface_Supported = TRUE;
  Feature_ID_Record.Interface_Support[VIO_Interface].Interface_Supported = TRUE;
  Feature_ID_Record.Interface_Support[Java_Interface].Interface_Supported = TRUE;

  /* Initialize the Function Table for this feature. */
  Function_Table.Feature_ID = &Feature_ID_Record;
  Function_Table.Open_Feature = &Open_BBR;
  Function_Table.Close_Feature = &Close_BBR;
  Function_Table.Can_Expand = &Can_Expand_BBR_Volume;
  Function_Table.Add_Partition = &Add_BBR_Partition;
  Function_Table.Delete = &Delete_BBR_Partition;
  Function_Table.Discover = &Discover_BBR_Partitions;
  Function_Table.Remove_Features = &Remove_Features;
  Function_Table.Create = &Create_BBR_Volume;
  Function_Table.Commit = &Commit_BBR_Changes;
  Function_Table.Write = &BBR_Write;
  Function_Table.Read = &BBR_Read;
  Function_Table.ReturnCurrentClass = &ReturnCurrentClass;
  Function_Table.PassThru = &PassThru;
  Function_Table.ChangesPending = &BBR_ChangesPending;
  Function_Table.ParseCommandLineArguments = &BBR_ParseCommandLineArguments;

  /* Save the table of common services provided by LVM. */
  LVM_Common_Services = ( LVM_Common_Services_V1 * ) Common_Services;

  /* Return our table of services. */
  return (ADDRESS) &Function_Table;

}



/*--------------------------------------------------
 * Private Functions Available
 --------------------------------------------------*/


static void _System Discover_BBR_Partitions( DLIST  Partition_List, CARDINAL32 * Error_Code )
{

  FEATURE_FUNCTION_ENTRY("Discover_BBR_Partitions")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( Partition_List == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Discover_BBR_Partitions has been invoked with one or more NULL pointers!\n     Partition_List is %X (hex)\n     Error_Code is %X (hex)", Partition_List, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* Process each partition in the Partition_List. */
  LVM_Common_Services->PruneList(Partition_List, &Find_BBR_Partitions, NULL, Error_Code );

  if ( LVM_Common_Services->Logging_Enabled && ( *Error_Code != DLIST_SUCCESS ) )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"PruneList failed with DLIST error %X (hex)", *Error_Code );
    LVM_Common_Services->Write_Log_Buffer();

  }

  if ( *Error_Code != DLIST_SUCCESS )
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"*Error_Code has been set to %X (hex)", *Error_Code);
    LVM_Common_Services->Write_Log_Buffer();

  }

  FEATURE_FUNCTION_EXIT("Discover_BBR_Partitions")

  return;

}


static void _System Commit_BBR_Changes( ADDRESS VData, ADDRESS PData, CARDINAL32 * Error_Code )
{

  Partition_Data *             PartitionRecord = (Partition_Data *) PData;
  Volume_Data *                VolumeRecord = (Volume_Data *) VData;
  Feature_Context_Data *       Current_Context;
  BBR_Data_Record *            BBR_Data = (BBR_Data_Record *) PartitionRecord->Feature_Data->Data;
  LVM_Signature_Sector *       Signature_Sector = PartitionRecord->Signature_Sector;
  LVM_Feature_Data     *       Feature_Data;
  CARDINAL32                   FeatureIndex;
  LVM_BBR_Table_First_Sector * BBR_First_Sector = ( LVM_BBR_Table_First_Sector *) &Feature_Data_Buffer1;
  LVM_BBR_Table_Sector *       BBR_Sector = (LVM_BBR_Table_Sector *) &Feature_Data_Buffer2;
  CARDINAL32                   Sector_Count;
  CARDINAL32                   BBR_Table_Index;
  CARDINAL32                   BBR_Entries_Moved = 0;
  CARDINAL32                   Offset;
  Plugin_Function_Table_V1 *   Old_Function_Table;
  BOOLEAN                      Feature_Found = FALSE;


  FEATURE_FUNCTION_ENTRY("Commit_BBR_Changes")

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( PData == NULL ) || ( VData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Commit_BBR_Changes has been invoked with one or more NULL pointers!\n     VData is %X (hex)\n     PData is %X (hex)\n     Error_Code is %X (hex)", VData, PData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Commit_BBR_Changes has been invoked with the following parameters.\n     The Volume specified has handle %X (hex)\n     The partition specified has handle %X (hex)\n     Error_Code is at address %X (hex)",
              VolumeRecord->External_Handle,
              PartitionRecord->External_Handle,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* Save the current feature data so that we can restore it later. */
  Current_Context = PartitionRecord->Feature_Data;

  /* Extract the function table for the feature following BBR. */
  Old_Function_Table = Current_Context->Old_Context->Function_Table;

  /* Find the entry for BBR in the Feature Array in the LVM Signature Sector. */
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME ; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == BBR_FEATURE_ID )
    {

      /* We have found the BBR entry!  Lets save it. */
      Feature_Data = &(Signature_Sector->LVM_Feature_Array[FeatureIndex]);

      /* Indicate that the feature was found. */
      Feature_Found = TRUE;

      /* Exit the loop. */
      break;

    }

  }

  /* Was the feature data found? */
  if ( ! Feature_Found )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Error: LVM_ENGINE_INTERNAL_ERROR\n     Feature data for BBR not found for this partition!\n");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Commit_BBR_Changes")

    return;

  }

  /* Was the BBR data changed?  If not, then we don't need to write it to disk. */
  if ( ( ! BBR_Data->ChangesMade ) && ( ! VolumeRecord->Convert_To_LVM_V1 ) )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR data has not changed.  Calling the commit function for the next layer.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* We need to call the commit function for any other features which may be in effect on this partition.
       If there are no other features on this partition, then the commit function will go to the Pass Thru layer. */

    /* Restore the context of the next layer. */
    PartitionRecord->Feature_Data = PartitionRecord->Feature_Data->Old_Context;

    /* Now call the commit function of the next layer. */
    Old_Function_Table->Commit( VData, PData, Error_Code);

    /* Restore our context. */
    Current_Context->Old_Context = PartitionRecord->Feature_Data;
    PartitionRecord->Feature_Data = Current_Context;

    FEATURE_FUNCTION_EXIT("Commit_BBR_Changes")

    return;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"BBR Data changed.  Building image to write to disk.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* We need to build the first sector of the BBR Data in Feature_Data_Buffer1. */
  memset(&Feature_Data_Buffer1,0,BYTES_PER_SECTOR);

  /* Initialize the first sector of the BBR Data. */
  BBR_First_Sector->Signature = BBR_TABLE_MASTER_SIGNATURE;
  BBR_First_Sector->CRC = 0;
  BBR_First_Sector->Sequence_Number = BBR_Data->Sequence_Number + 1;
  BBR_First_Sector->Table_Size = BBR_Data->BBR_Table_Size;
  BBR_First_Sector->Table_Entries_In_Use = BBR_Data->Entries_In_Use;
  BBR_First_Sector->Sectors_Per_Table = Feature_Data->Feature_Data_Size - 1;
  BBR_First_Sector->First_Replacement_Sector = Feature_Data->Location_Of_Primary_Feature_Data + Feature_Data->Feature_Data_Size;
  BBR_First_Sector->Last_Replacement_Sector = (BBR_First_Sector->First_Replacement_Sector + BBR_Data->BBR_Table_Size) - 1;
  BBR_First_Sector->Replacement_Sector_Count = BBR_Data->BBR_Table_Size;

  /* Do we need to convert to LVM Version 1 format?  This could occur if the current LVM was installed
     as part of a fixpak and the fixpak is now being backed out for some reason.                       */
  if ( VolumeRecord->Convert_To_LVM_V1 )
  {

    /* We must change the version numbers for BBR so that LVM Version 1 will accept them. */
    Feature_Data->Feature_Major_Version_Number = 1;
    Feature_Data->Feature_Minor_Version_Number = 0;

    /* We must zero out the fields that LVM Version 1 will not understand. */
    BBR_First_Sector->Actual_Class = 0;
    BBR_First_Sector->Top_Of_Class = 0;
    BBR_First_Sector->Feature_Sequence_Number = 0;

  }
  else
  {

    BBR_First_Sector->Actual_Class = BBR_Data->Actual_Class;
    BBR_First_Sector->Top_Of_Class = BBR_Data->Top_Of_Class;
    BBR_First_Sector->Feature_Sequence_Number = BBR_Data->Feature_Sequence_Number;

  }

  /* Calculate the CRC. */
  BBR_First_Sector->CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, BBR_First_Sector, BYTES_PER_SECTOR);

  /* Now we will use Feature_Data_Buffer2 to build the sectors holding the BBR Table. */
  memset(&Feature_Data_Buffer2,0, BYTES_PER_SECTOR * MAX_SECTORS_IN_BBR_TABLE);

  /* Setup each sector of data contained in the buffer. */
  for ( Sector_Count = 1; Sector_Count <= BBR_First_Sector->Sectors_Per_Table; Sector_Count++)
  {

    /* Locate the next sector within the buffer. */
    Offset = BYTES_PER_SECTOR * ( Sector_Count - 1 );
    BBR_Sector = (LVM_BBR_Table_Sector *) &Feature_Data_Buffer2;
    BBR_Sector = (LVM_BBR_Table_Sector *) ( (CARDINAL32) BBR_Sector + Offset );

    /* Set the signature. */
    BBR_Sector->Signature = BBR_TABLE_SIGNATURE;

    /* Set the sequence number. */
    BBR_Sector->Sequence_Number = BBR_Data->Sequence_Number + 1;

    /* Transfer Link Entries to the Sector. */
    for ( BBR_Table_Index = 0; (BBR_Table_Index < BBR_TABLE_ENTRIES_PER_SECTOR) && (BBR_Entries_Moved < BBR_Data->BBR_Table_Size); BBR_Table_Index++ )
    {

      BBR_Sector->BBR_Table[BBR_Table_Index] = BBR_Data->BBR_Table[BBR_Entries_Moved];
      BBR_Entries_Moved++;

    }

    /* Calculate the CRC. */
    BBR_Sector->CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, BBR_Sector, BYTES_PER_SECTOR);

  }


  /* Now that the buffers have been filled in, lets write them to disk. */


  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Writing BBR feature data to disk.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Write the Primary Copy of the Feature Data. */

  /* Write the first sector. If the write fails, set the I/O error flag in the corresponding entry in the DriveArray. */
  Function_Table.Write( PartitionRecord, Feature_Data->Location_Of_Primary_Feature_Data + Signature_Sector->Partition_Start, 1, &Feature_Data_Buffer1, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

  /* Write the sectors containing the BBR Table. If the write fails, set the I/O error flag in the corresponding entry in the DriveArray. */
  Function_Table.Write( PartitionRecord, Feature_Data->Location_Of_Primary_Feature_Data + 1 + Signature_Sector->Partition_Start, BBR_First_Sector->Sectors_Per_Table, &Feature_Data_Buffer2, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

  /* Write the Secondary Copy of the Feature Data. */

  /* Write the first sector. If the write fails, set the I/O error flag in the corresponding entry in the DriveArray. */
  Function_Table.Write( PartitionRecord, Feature_Data->Location_Of_Secondary_Feature_Data + Signature_Sector->Partition_Start, 1, &Feature_Data_Buffer1, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

  /* Write the sectors containing the BBR Table. If the write fails, set the I/O error flag in the corresponding entry in the DriveArray. */
  Function_Table.Write( PartitionRecord, Feature_Data->Location_Of_Secondary_Feature_Data + 1 + Signature_Sector->Partition_Start, BBR_First_Sector->Sectors_Per_Table, &Feature_Data_Buffer2, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Calling the commit function for the next layer.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* We need to call the commit function for any other features which may be in effect on this partition.
     If there are no other features on this partition, then the commit function will go to the Pass Thru layer. */

  /* Restore the context of the next layer. */
  PartitionRecord->Feature_Data = Current_Context->Old_Context;

  /* Now call the commit function of the next layer. */
  Old_Function_Table->Commit( VData, PData, Error_Code);

  /* Restore our context. */
  Current_Context->Old_Context = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = Current_Context;

  FEATURE_FUNCTION_EXIT("Commit_BBR_Changes")

  return;

}


static void    _System Create_BBR_Volume( DLIST Partition_List,
                                         ADDRESS VData,
                                         ADDRESS Init_Data,
                                         void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                         LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                         BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                         CARDINAL32   Sequence_Number,
                                         CARDINAL32 * Error_Code )
{

  Create_BBR_Data_Record  BBR_Creation_Data;

  FEATURE_FUNCTION_ENTRY("Create_BBR_Volume")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( Partition_List == NULL ) || ( VData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Create_BBR_Volume has been invoked with one or more NULL pointers!\n     Partition_List is %X (hex)\n     VData is %X (hex)\n     Error_Code is %X (hex)", Partition_List, VData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( ( Actual_Class != Partition_Class) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Create_BBR_Volume has been invoked with the wrong class!\n ");
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* We must put BBR Data on each partition in the Partition_List. */
  BBR_Creation_Data.Actual_Class = Actual_Class;
  BBR_Creation_Data.Top_Of_Class = Top_Of_Class;
  BBR_Creation_Data.Feature_Sequence_Number = Sequence_Number;
  LVM_Common_Services->ForEachItem(Partition_List, &Create_BBR_Data, &BBR_Creation_Data, TRUE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Error: ForEachItem failed with error %X (hex)", *Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* We did not succeed!  Did we run out of memory? */
    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
    {

      /* We must remove the BBR Data from the partitions in the Partition_List. */

      /* We must remove any BBR Data from each partition in the Partition_List. */
      LVM_Common_Services->ForEachItem(Partition_List, &Remove_BBR_Data, NULL, TRUE, Error_Code);

      /* Set the proper error code. */
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    }
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Create_BBR_Volume")

    return;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,FUNCTION_EXIT_BORDER);
    LVM_Common_Services->Write_Log_Buffer();
    sprintf(LVM_Common_Services->Log_Buffer, "                                    Create_BBR_Volume");
    LVM_Common_Services->Write_Log_Buffer();
    sprintf(LVM_Common_Services->Log_Buffer,FUNCTION_EXIT_BORDER);
    LVM_Common_Services->Write_Log_Buffer();

  }

  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Create_BBR_Volume")

  return;

}


static void _System Open_BBR( CARDINAL32 * Error_Code )
{

  FEATURE_FUNCTION_ENTRY("Open_BBR")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( Feature_Is_Open )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Open_BBR has been invoked, but BBR is already open! ");
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( Error_Code == NULL )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Open_BBR has been invoked with a NULL pointer!\n     Error_Code is %X (hex)", Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }


  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Open_BBR")

  return;

}


static void _System Close_BBR( void )
{

  FEATURE_FUNCTION_ENTRY("Close_BBR")

  /* BBR has nothing to do here.  */

  FEATURE_FUNCTION_EXIT("Close_BBR")

  return;

}


static BOOLEAN _System Can_Expand_BBR_Volume( ADDRESS AData, CARDINAL32 * Feature_ID, CARDINAL32 * Error_Code )
{

  Partition_Data * Aggregate = (Partition_Data *) AData;

  FEATURE_FUNCTION_ENTRY("Can_Expand_BBR_Volume")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( AData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Can_Expand_BBR_Volume has been invoked with one or more NULL pointers!\n     AData is %X (hex)\n     Error_Code is %X (hex)", AData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Can_Expand_BBR_Volume has been invoked with the following parameters.\n     The partition specified has handle %X (hex)\n      Error_Code is at address %X (hex)",
              Aggregate->External_Handle,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }


  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Returning FALSE as BBR does not support Aggregates");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* BBR does not do aggregates, and it only works on partitions.  Furthermore, BBR can not tolerate having a
     partition underneath it change size.  Thus, you can not "expand" a BBR partition.                          */
  *Feature_ID = 0;
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Can_Expand_BBR_Volume")

  return FALSE;

}


static void _System Add_BBR_Partition ( ADDRESS AData, ADDRESS PData, CARDINAL32 * Error_Code )
{

  Partition_Data * Aggregate = (Partition_Data *) AData;
  Partition_Data * New_Partition = (Partition_Data *) PData;

  FEATURE_FUNCTION_ENTRY("Add_BBR_Partition")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( Aggregate == NULL ) || ( New_Partition == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Add_BBR_Partition has been invoked with one or more NULL pointers!\n     AData is %X (hex)\n     PData is %X (hex)\n     Error_Code is %X (hex)", AData, PData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Add_BBR_Partition has been invoked with an aggregate and a partition.\n     The aggregate belongs to the volume with handle %X (hex)\n     The partition has handle %X (hex)\n     Error_Code is at address %X (hex)",
              Aggregate->Volume_Handle,
              New_Partition->External_Handle,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  Create_BBR_Data(PData, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, NULL, Error_Code);
  switch ( *Error_Code )
  {

    case DLIST_SUCCESS :
                         *Error_Code = LVM_ENGINE_NO_ERROR;
                         break;
    case DLIST_OUT_OF_MEMORY :
                               *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

                               if ( LVM_Common_Services->Logging_Enabled )
                               {

                                 sprintf(LVM_Common_Services->Log_Buffer,"Create_BBR_Data failed due to a lack of memory!");
                                 LVM_Common_Services->Write_Log_Buffer();

                               }

                               break;
    default :
              *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Create_BBR_Data failed with DLIST error code %d (decimal)", *Error_Code);
                LVM_Common_Services->Write_Log_Buffer();

              }

              break;
  }

  FEATURE_FUNCTION_EXIT("Add_BBR_Partition")

  return;

}


static void _System Delete_BBR_Partition( ADDRESS AData, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code)
{

  Partition_Data *           PartitionRecord = (Partition_Data *) AData;
  Feature_Context_Data *     Current_Feature_Data = PartitionRecord->Feature_Data; /* Save the current feature data. */
  BBR_Data_Record *          BBR_Data;
  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("Delete_BBR_Partition")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( AData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Delete_BBR_Partition has been invoked with one or more NULL pointers!\n     AData is %X (hex)\n     Error_Code is %X (hex)", AData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Delete_BBR_Partition has been invoked with the following parameters.\n     The Volume specified has handle %X (hex)\n     Kill_Partitions is %X (hex)\n     Error_Code is at address %X (hex)",
              PartitionRecord->Volume_Handle,
              Kill_Partitions,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* We must pass this Delete request along to the layer below us. */

  /* Place the feature data for the layer below us into the Feature_Data field of the Partition Record. */
  PartitionRecord->Feature_Data = Current_Feature_Data->Old_Context;
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Calling the Delete function for the next layer.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now call the Delete function of the next layer.  The bottom layer will actually
     delete the Partition and free its associated PartitionRecord.                    */
  Old_Function_Table->Delete(AData, Kill_Partitions, Error_Code);

#ifdef DEBUG

#ifdef PARANOID

  assert( Current_Feature_Data->Partitions == NULL );

#else

  if ( Current_Feature_Data->Partitions != NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Error: LVM_ENGINE_INTERNAL_ERROR\n     The feature data for BBR has a partitions list!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Delete_BBR_Partition")

    return;

  }

#endif

#endif

  /* Free the BBR Data for the partition. */
  BBR_Data = (BBR_Data_Record *) Current_Feature_Data->Data;
  LVM_Common_Services->Deallocate(BBR_Data->BBR_Table);
  LVM_Common_Services->Deallocate(Current_Feature_Data->Data);

  /* Free the feature data for the partition. */
  LVM_Common_Services->Deallocate(Current_Feature_Data);

  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Delete_BBR_Partition")

  return;

}


static void _System BBR_Write( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code)
{

  Partition_Data *           PartitionRecord = (Partition_Data *) PData;
  Feature_Context_Data *     Current_Feature_Data;
  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("BBR_Write")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( PData == NULL ) || ( Buffer == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR_Write has been invoked with one or more NULL pointers!\n     PData is %X (hex)\n     Buffer is %X (hex)\n     Error_Code is %X (hex)", PData, Buffer, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "BBR_Write has been invoked with the following parameters.\n     The partition specified has handle %X (hex)\n     The LBA of the sector to read is %X (hex)\n     The number of sectors to write is %d (decimal)\n     The location of the buffer to read into is %X (hex)\n      Error_Code is at address %X (hex)",
              PartitionRecord->External_Handle,
              Starting_Sector,
              Sectors_To_Write,
              Buffer,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* We must pass this write request along to the layer below us. */

  /* Is the feature data for the next layer available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR_Write has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_Write")

    return;

  }

  /* Place the feature data for the layer below us into the Feature_Data field of the Partition Record. */
  Current_Feature_Data = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = Current_Feature_Data->Old_Context;
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  /* Is the function table for the next layer available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR_Write has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_Write")

    return;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Passing the write request to the next layer down.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now call the write function of the next layer. */
  Old_Function_Table->Write(PData, Starting_Sector, Sectors_To_Write, Buffer, Error_Code);

  /* Now restore the feature data to what it was before we called the write function and return. */
  Current_Feature_Data->Old_Context = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = Current_Feature_Data;

  FEATURE_FUNCTION_EXIT("BBR_Write")

  return;

}


static void _System BBR_Read( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code)
{

  Partition_Data *           PartitionRecord = (Partition_Data *) PData;
  Feature_Context_Data *     Current_Feature_Data = PartitionRecord->Feature_Data; /* Save the feature data for the PartitionRecord. */
  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("BBR_Read")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( PData == NULL ) || ( Buffer == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR_Read has been invoked with one or more NULL pointers!\n     PData is %X (hex)\n     Buffer is %X (hex)\n     Error_Code is %X (hex)", PData, Buffer, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "BBR_Read has been invoked with the following parameters.\n     The partition specified has handle %X (hex)\n     The LBA of the sector to read is %X (hex)\n     The number of sectors to read is %d (decimal)\n     The location of the buffer to read into is %X (hex)\n      Error_Code is at address %X (hex)",
              PartitionRecord->External_Handle,
              Starting_Sector,
              Sectors_To_Read,
              Buffer,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* We must pass this read request along to the layer below us. */

  /* Is the feature data for the next layer available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_Read")

    return;

  }

  /* Place the feature data for the layer below us into the Feature_Data field of the Partition Record. */
  PartitionRecord->Feature_Data = Current_Feature_Data->Old_Context;
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  /* Is the function table for the next layer available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR_Read has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_Read")

    return;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Passing the request to the next layer.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now call the read function of the next layer. */
  Old_Function_Table->Read(PData, Starting_Sector, Sectors_To_Read, Buffer, Error_Code);

  /* Now restore the feature data to what it was before we called the read function and return. */
  Current_Feature_Data->Old_Context = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = Current_Feature_Data;

  FEATURE_FUNCTION_EXIT("BBR_Read")

  return;

}




static void _System Create_BBR_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  Partition_Data *         PartitionRecord = (Partition_Data *) Object;
  Create_BBR_Data_Record * BBR_Creation_Data = (Create_BBR_Data_Record *) Parameters;
  Feature_Context_Data *   BBR_Context_Data;
  LVM_Signature_Sector *   Signature_Sector;                            /* Used when accessing the LVM Signature Sector of the partition. */
  CARDINAL32               Table_Size;
  CARDINAL32               Table_Index;
  CARDINAL32               Replacement_Sector_LSN;
  CARDINAL32               Sectors_Per_Table;
  BBR_Data_Record *        BBR_Data;
  CARDINAL32               FeatureIndex;
  BOOLEAN                  FeatureAdded = FALSE;

  FEATURE_FUNCTION_ENTRY("Create_BBR_Data")

#ifdef DEBUG

#ifdef PARANOID

  assert(Object != NULL);
  assert(ObjectSize == sizeof(Partition_Data) );
  assert(Parameters == NULL);
  assert(ObjectTag == PARTITION_DATA_TAG);

#else

  if ( ( Object == NULL ) ||
       ( ObjectSize != sizeof( Partition_Data ) ) ||
       ( Parameters == NULL ) ||
       ( ObjectTag != PARTITION_DATA_TAG )
     )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Create_BBR_Data called with invalid parameters!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Create_BBR_Data")

    return;

  }

#endif

#endif


  /* Allocate the memory for the BBR Feature Data. */
  BBR_Context_Data = (Feature_Context_Data *) LVM_Common_Services->Allocate(sizeof(Feature_Context_Data) );
  if ( BBR_Context_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Could not allocate memory for the BBR Context Data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = DLIST_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Create_BBR_Data")

    return;

  }

  /* Initialize the feature data for BBR. */
  BBR_Context_Data->Feature_ID = &Feature_ID_Record;
  BBR_Context_Data->Function_Table = &Function_Table;
  BBR_Context_Data->Data = NULL;
  BBR_Context_Data->Partitions = NULL;
  BBR_Context_Data->Old_Context = PartitionRecord->Feature_Data;

  /* Calculate the size of the BBR Table. */
  Table_Size = PartitionRecord->Partition_Size >> 10;
  if ( Table_Size < BBR_FLOOR )
    Table_Size = BBR_FLOOR;

  if ( Table_Size > BBR_LIMIT )
    Table_Size = BBR_LIMIT;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"BBR Table size set to %d (decimal)",Table_Size);
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Calculate the number of sectors required to hold the BBR Table. */
  Sectors_Per_Table = Table_Size / BBR_TABLE_ENTRIES_PER_SECTOR;
  if ( (Table_Size % BBR_TABLE_ENTRIES_PER_SECTOR) != 0 )
    Sectors_Per_Table += 1;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Sectors per table is set to %d (decimal)", Sectors_Per_Table);
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Allocate the BBR Data Record. */
  BBR_Context_Data->Data = (BBR_Data_Record *) LVM_Common_Services->Allocate( sizeof(BBR_Data_Record) );
  if ( BBR_Context_Data->Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Could not allocate memory for the BBR Data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    LVM_Common_Services->Deallocate(BBR_Context_Data);

    *Error_Code = DLIST_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Create_BBR_Data")

    return;

  }

  /* Initialize the BBR Data Record. */
  BBR_Data = BBR_Context_Data->Data;
  BBR_Data->BBR_Table_Size = Table_Size;
  BBR_Data->Entries_In_Use = 0;
  BBR_Data->Sequence_Number = 0;
  BBR_Data->ChangesMade = TRUE;
  BBR_Data->Actual_Class = BBR_Creation_Data->Actual_Class;
  BBR_Data->Top_Of_Class = BBR_Creation_Data->Top_Of_Class;
  BBR_Data->Feature_Sequence_Number = BBR_Creation_Data->Feature_Sequence_Number;
  BBR_Data->BBR_Table = NULL;

  /* Allocate the BBR Table. */
  BBR_Data->BBR_Table = (BBR_Table_Entry *) LVM_Common_Services->Allocate( Table_Size * sizeof(BBR_Table_Entry) );
  if ( BBR_Data->BBR_Table == NULL)
  {

    LVM_Common_Services->Deallocate(BBR_Context_Data->Data);
    LVM_Common_Services->Deallocate(BBR_Context_Data);

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Could not allocate memory for the BBR Table!  %d (decimal) bytes are needed.", Table_Size * sizeof(BBR_Table_Entry) );
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = DLIST_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Create_BBR_Data")

    return;

  }

  /* Initialize the BBR Table. */
  memset(BBR_Data->BBR_Table,0, Table_Size * sizeof(BBR_Table_Entry) );

  /* Add our feature data to the partition. */
  PartitionRecord->Feature_Data = BBR_Context_Data;

  /* Now we must add BBR to the LVM Signature Sector. */
  Signature_Sector = PartitionRecord->Signature_Sector;
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME ; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == 0)
    {

      /* We have found an open entry!  Lets fill it in. */
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID = BBR_FEATURE_ID;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Active = TRUE;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Data_Size = 1 + Sectors_Per_Table;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number = BBR_FEATURE_MAJOR_VERSION;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Minor_Version_Number = BBR_FEATURE_MINOR_VERSION;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"The original value of LVM_Reserved_Sector_Count is %d (decimal)", Signature_Sector->LVM_Reserved_Sector_Count );
        LVM_Common_Services->Write_Log_Buffer();

        sprintf(LVM_Common_Services->Log_Buffer,"The original size of the partition as seen by the user was %d (decimal) ", Signature_Sector->Partition_Size_To_Report_To_User);
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Reserve the replacement sectors. */
      Signature_Sector->LVM_Reserved_Sector_Count += Table_Size;

      /* Fill in the LSNs of the replacement sectors in the BBR Table. */

      /* Calculate the LSN of the first replacement sector. */
      Replacement_Sector_LSN = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count ) + 1;
      Replacement_Sector_LSN -= Signature_Sector->Partition_Start;

      /* Now fill in the BBR Table using the LSNs of the replacement sectors. */
      for ( Table_Index = 0; Table_Index < Table_Size; Table_Index++ )
      {

        BBR_Data->BBR_Table[Table_Index].ReplacementSector = Replacement_Sector_LSN++;
        BBR_Data->BBR_Table[Table_Index].BadSector = (CARDINAL32) -1;                     /* Indicates that this entry is unused. */

      }

      /* Reserve space for the primary copy of the BBR Feature Data. */
      Signature_Sector->LVM_Reserved_Sector_Count += (1 + Sectors_Per_Table);
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"The primary copy of the BBR feature data starts at sector %X (hex, LBA)", Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data);
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Convert from LBA to LSN. */
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data -= Signature_Sector->Partition_Start;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"The primary copy of the BBR feature data starts at sector %X (hex, LSN)", Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data);
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Reserve space for the secondary copy of the BBR Feature Data. */
      Signature_Sector->LVM_Reserved_Sector_Count += (1 + Sectors_Per_Table);
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"The secondary copy of the BBR feature data starts at sector %X (hex, LBA)", Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data);
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Convert from LBA to LSN. */
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data -= Signature_Sector->Partition_Start;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"The secondary copy of the BBR feature data starts at sector %X (hex, LSN)", Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data);
        LVM_Common_Services->Write_Log_Buffer();

      }


      /* Calculate the size of the partition as seen by the user. */
      Signature_Sector->Partition_Size_To_Report_To_User = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;

      /* Indicate that the feature was added. */
      FeatureAdded = TRUE;

      /* Adjust the partitions usable size to account for the sectors we have reserved for BBR. */
      PartitionRecord->Usable_Size = PartitionRecord->Partition_Size - Signature_Sector->LVM_Reserved_Sector_Count;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"The new value of LVM_Reserved_Sector_Count is %d (decimal)", Signature_Sector->LVM_Reserved_Sector_Count);
        LVM_Common_Services->Write_Log_Buffer();

        sprintf(LVM_Common_Services->Log_Buffer,"The new size of the partition as seen by the user is %d (decimal) ", Signature_Sector->Partition_Size_To_Report_To_User);
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Exit the loop. */
      break;

    }

  }

  /* Was the BBR Added successfully? */
  if ( ! FeatureAdded )
  {

    /* There are too many features on this partition! */
    *Error_Code = DLIST_OUT_OF_MEMORY;

    /* Undo the changes we have made. */
    PartitionRecord->Feature_Data = BBR_Context_Data->Old_Context;

    /* Free memory. */
    LVM_Common_Services->Deallocate(BBR_Data->BBR_Table);
    LVM_Common_Services->Deallocate(BBR_Context_Data->Data);
    LVM_Common_Services->Deallocate(BBR_Context_Data);

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Could not find an open entry in the feature table of the LVM Signature Sector!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    FEATURE_FUNCTION_EXIT("Create_BBR_Data")

    return;

  }

  /* Indicate success. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Create_BBR_Data")

  return;

}


static BOOLEAN _System Find_BBR_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code)
{

  Partition_Data *              PartitionRecord = (Partition_Data *) Object;
  Feature_Context_Data *        BBR_Context_Data = NULL;
  LVM_Signature_Sector *        Signature_Sector = NULL;                            /* Used when accessing the LVM Signature Sector of the partition. */
  LVM_Feature_Data *            BBR_Feature_Data = NULL;
  BBR_Data_Record *             BBR_Data = NULL;
  LVM_BBR_Table_First_Sector *  BBR_Primary_First_Sector = ( LVM_BBR_Table_First_Sector * ) &Feature_Data_Buffer1;
  LVM_BBR_Table_First_Sector *  BBR_Secondary_First_Sector = ( LVM_BBR_Table_First_Sector * ) &Feature_Data_Buffer2;
  Plugin_Function_Table_V1 *    Old_Function_Table;
  CARDINAL32                    FeatureIndex;
  BOOLEAN                       Primary_Data_Valid = FALSE;
  BOOLEAN                       Secondary_Data_Valid = FALSE;
  BOOLEAN                       Old_BBR_Format = FALSE;

  FEATURE_FUNCTION_ENTRY("Find_BBR_Partitions")

#ifdef DEBUG

#ifdef PARANOID

  assert(Object != NULL);
  assert(ObjectSize == sizeof(Partition_Data) );
  assert(Parameters == NULL);
  assert(ObjectTag == PARTITION_DATA_TAG);

#else

  if ( ( Object == NULL ) ||
       ( ObjectSize != sizeof( Partition_Data ) ) ||
       ( Parameters != NULL ) ||
       ( ObjectTag != PARTITION_DATA_TAG )
     )
  {

    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

    return TRUE;

  }

#endif

#endif

  /* Assume success. */
  *Error_Code = DLIST_SUCCESS;

  /* We don't want to actually delete any partition records, just remove them from the list being processed. */
  *FreeMemory = FALSE;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Examining partition with handle %X (hex) to see if it contains BBR data.", PartitionRecord->External_Handle);
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Does the partition have an LVM Signature Sector? */
  if ( PartitionRecord->Signature_Sector == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"The partition does not have an LVM Signature Sector, and therefore has no BBR data.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

    /* This partition does not have an LVM Signature Sector.  It can't have BBR on it. */
    return FALSE;

  }

  /* Remember the location of the LVM Signature Sector. */
  Signature_Sector = PartitionRecord->Signature_Sector;

  /* Is BBR in the Feature Array for this partition? */
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME; FeatureIndex++)
  {

    /* Does the current entry contain our BBR Feature ID? */
    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == BBR_FEATURE_ID )
    {

      /* Are we using BBR Data written by version 1 of BBR? */
      if ( ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number == 1 ) &&
           ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Minor_Version_Number == 0 )
         )
        Old_BBR_Format = TRUE;

      /* Establish access to the Feature Data in the LVM Signature Sector. */
      BBR_Feature_Data = (LVM_Feature_Data *) &(Signature_Sector->LVM_Feature_Array[FeatureIndex]);

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Attempting to get and validate the primary copy of the feature data.");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Get the function table associated with the topmost feature on the partition. */
      Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

      /* Get the first copy of the BBR Feature Data.   Make sure all of the sequence numbers match, and check all CRCs. */
      Old_Function_Table->Read(PartitionRecord, BBR_Feature_Data->Location_Of_Primary_Feature_Data + Signature_Sector->Partition_Start, BBR_Feature_Data->Feature_Data_Size, &Feature_Data_Buffer1, Error_Code);
      if ( ( *Error_Code == LVM_ENGINE_NO_ERROR ) && Feature_Data_Is_Valid(TRUE, BBR_Feature_Data->Feature_Data_Size) )
        Primary_Data_Valid = TRUE;

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Attempting to get and validate the secondary copy of the feature data.");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Get the second copy of the BBR Feature Data.  Make sure all of the sequence numbers match, and check all CRCs. */
      Old_Function_Table->Read(PartitionRecord, BBR_Feature_Data->Location_Of_Secondary_Feature_Data + Signature_Sector->Partition_Start, BBR_Feature_Data->Feature_Data_Size, &Feature_Data_Buffer2, Error_Code);
      if ( ( *Error_Code == LVM_ENGINE_NO_ERROR ) && Feature_Data_Is_Valid(FALSE, BBR_Feature_Data->Feature_Data_Size) )
        Secondary_Data_Valid = TRUE;

      /* Do we have a valid copy of the data to use? */
      if ( Primary_Data_Valid || Secondary_Data_Valid )
      {

        /* Allocate memory to hold the BBR Context Data that is to be associated with this PartitionRecord. */
        BBR_Context_Data = ( Feature_Context_Data * ) LVM_Common_Services->Allocate( sizeof(Feature_Context_Data) );

        if ( BBR_Context_Data == NULL )
        {

          if ( LVM_Common_Services->Logging_Enabled )
          {

            sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
            LVM_Common_Services->Write_Log_Buffer();

          }

          *Error_Code = DLIST_OUT_OF_MEMORY;

          FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

          return FALSE;

        }

        /* Initialize the BBR context data for this PartitionRecord. */
        BBR_Context_Data->Feature_ID = &Feature_ID_Record;
        BBR_Context_Data->Function_Table = &Function_Table;
        BBR_Context_Data->Data = NULL;
        BBR_Context_Data->Partitions = NULL;
        BBR_Context_Data->Old_Context = PartitionRecord->Feature_Data;

        /* Allocate memory to hold the BBR Data Record. */
        BBR_Context_Data->Data = LVM_Common_Services->Allocate( sizeof(BBR_Data_Record) );
        BBR_Data = (BBR_Data_Record *) BBR_Context_Data->Data;

        if ( BBR_Data == NULL )
        {

          *Error_Code = DLIST_OUT_OF_MEMORY;
          LVM_Common_Services->Deallocate(BBR_Context_Data);

          if ( LVM_Common_Services->Logging_Enabled )
          {

            sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
            LVM_Common_Services->Write_Log_Buffer();

          }

          FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

          return FALSE;

        }


        /* Are both copies valid? */
        if ( Primary_Data_Valid && Secondary_Data_Valid )
        {

          if ( LVM_Common_Services->Logging_Enabled )
          {

            sprintf(LVM_Common_Services->Log_Buffer,"Both copies of the feature data appear to be valid.  Investigating further.");
            LVM_Common_Services->Write_Log_Buffer();

          }

          /* Do the two copies of the feature data match?  If not, use the one with the higher sequence number. */
          if ( memcmp(&Feature_Data_Buffer1, &Feature_Data_Buffer2, BBR_Feature_Data->Feature_Data_Size * BYTES_PER_SECTOR ) != 0 )
          {

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"The two copies of the feature data do not match!");
              LVM_Common_Services->Write_Log_Buffer();

            }

            if ( BBR_Primary_First_Sector->Sequence_Number > BBR_Secondary_First_Sector->Sequence_Number )
            {

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Using the primary copy as it has the highest sequence number.");
                LVM_Common_Services->Write_Log_Buffer();

              }

              /* Allocate memory for the actual BBR Table. */
              BBR_Data->BBR_Table = (BBR_Table_Entry *) LVM_Common_Services->Allocate( sizeof( BBR_Table_Entry ) * BBR_Primary_First_Sector->Table_Size );

              if ( BBR_Data->BBR_Table == NULL )
              {

                *Error_Code = DLIST_OUT_OF_MEMORY;
                LVM_Common_Services->Deallocate(BBR_Data);
                LVM_Common_Services->Deallocate(BBR_Context_Data);

                if ( LVM_Common_Services->Logging_Enabled )
                {

                  sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
                  LVM_Common_Services->Write_Log_Buffer();

                }

                FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

                return FALSE;

              }

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Setting the ChangesMade flag in the BBR data.");
                LVM_Common_Services->Write_Log_Buffer();

              }

              /* Since the copies do not match, we need to get them back in sync.  Setting ChangesMade to TRUE will
                 cause the copy of the table which is used to be written back to both locations on disk when the user
                 does a commit, thereby bringing both on disk copies back into sync.                                   */
              BBR_Data->ChangesMade = TRUE;

              /* Since we have all of the memory we need, lets update the PartitionRecord and fill in the actual BBR Table. */
              PartitionRecord->Feature_Data = BBR_Context_Data;

              /* Update the Feature_Index to indicate that this feature has been successfully activated for this partition. */
              PartitionRecord->Feature_Index += 1;

              /* Get the BBR Table. */
              Extract_BBR_Table( PartitionRecord, TRUE, Old_BBR_Format);

            }
            else
            {

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Using the secondary copy as it has the highest sequence number.");
                LVM_Common_Services->Write_Log_Buffer();

              }

              /* Allocate memory for the actual BBR Table. */
              BBR_Data->BBR_Table = (BBR_Table_Entry *) LVM_Common_Services->Allocate( sizeof( BBR_Table_Entry ) * BBR_Primary_First_Sector->Table_Size );

              if ( BBR_Data->BBR_Table == NULL )
              {

                *Error_Code = DLIST_OUT_OF_MEMORY;
                LVM_Common_Services->Deallocate(BBR_Data);
                LVM_Common_Services->Deallocate(BBR_Context_Data);

                if ( LVM_Common_Services->Logging_Enabled )
                {

                  sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
                  LVM_Common_Services->Write_Log_Buffer();

                }

                FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

                return FALSE;

              }

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Setting the ChangesMade flag in the BBR Data.");
                LVM_Common_Services->Write_Log_Buffer();

              }

              /* Since the copies do not match, we need to get them back in sync.  Setting ChangesMade to TRUE will
                 cause the copy of the table which is used to be written back to both locations on disk when the user
                 does a commit, thereby bringing both on disk copies back into sync.                                   */
              BBR_Data->ChangesMade = TRUE;

              /* Since we have all of the memory we need, lets update the PartitionRecord and fill in the actual BBR Table. */
              PartitionRecord->Feature_Data = BBR_Context_Data;

              /* Update the Feature_Index to indicate that this feature has been successfully activated for this partition. */
              PartitionRecord->Feature_Index += 1;

              /* Get the BBR Table. */
              Extract_BBR_Table( PartitionRecord, FALSE, Old_BBR_Format);

            }

          }
          else
          {

            /* Since they match, we will use the Primary copy. */

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Both copies of the feature data match.  Using the primary copy.");
              LVM_Common_Services->Write_Log_Buffer();

            }


            /* Allocate memory for the actual BBR Table. */
            BBR_Data->BBR_Table = (BBR_Table_Entry *) LVM_Common_Services->Allocate( sizeof( BBR_Table_Entry ) * BBR_Primary_First_Sector->Table_Size );

            if ( BBR_Data->BBR_Table == NULL )
            {

              *Error_Code = DLIST_OUT_OF_MEMORY;
              LVM_Common_Services->Deallocate(BBR_Data);
              LVM_Common_Services->Deallocate(BBR_Context_Data);

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
                LVM_Common_Services->Write_Log_Buffer();

              }

              FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

              return FALSE;

            }

            /* Since both copies of the BBR Feature Data match, we can set ChangesMade to false. */
            BBR_Data->ChangesMade = FALSE;

            /* Since we have all of the memory we need, lets update the PartitionRecord and fill in the actual BBR Table. */
            PartitionRecord->Feature_Data = BBR_Context_Data;

            /* Update the Feature_Index to indicate that this feature has been successfully activated for this partition. */
            PartitionRecord->Feature_Index += 1;

            /* Get the BBR Table. */
            Extract_BBR_Table( PartitionRecord, TRUE, Old_BBR_Format);

          }

        }
        else
        {

          /* We must use the copy of the data which is valid. */
          if ( Primary_Data_Valid )
          {

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Only the primary copy of the feature data was valid.  Using the primary copy.");
              LVM_Common_Services->Write_Log_Buffer();

            }

            /* Allocate memory for the actual BBR Table. */
            BBR_Data->BBR_Table = (BBR_Table_Entry *) LVM_Common_Services->Allocate( sizeof( BBR_Table_Entry ) * BBR_Primary_First_Sector->Table_Size );

            if ( BBR_Data->BBR_Table == NULL )
            {

              *Error_Code = DLIST_OUT_OF_MEMORY;
              LVM_Common_Services->Deallocate(BBR_Data);
              LVM_Common_Services->Deallocate(BBR_Context_Data);

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
                LVM_Common_Services->Write_Log_Buffer();

              }

              FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

              return FALSE;

            }


            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Setting the ChangesMade flag in the BBR data.");
              LVM_Common_Services->Write_Log_Buffer();

            }

            /* Since the copies do not match, we need to get them back in sync.  Setting ChangesMade to TRUE will
               cause the copy of the table which is used to be written back to both locations on disk when the user
               does a commit, thereby bringing both on disk copies back into sync.                                   */
            BBR_Data->ChangesMade = TRUE;

            /* Since we have all of the memory we need, lets update the PartitionRecord and fill in the actual BBR Table. */
            PartitionRecord->Feature_Data = BBR_Context_Data;

            /* Update the Feature_Index to indicate that this feature has been successfully activated for this partition. */
            PartitionRecord->Feature_Index += 1;

            /* Get the BBR Table. */
            Extract_BBR_Table( PartitionRecord, TRUE, Old_BBR_Format);

          }
          else
          {

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Only the secondary copy of the feature data was valid.  Using the secondary copy.");
              LVM_Common_Services->Write_Log_Buffer();

            }

            /* Allocate memory for the actual BBR Table. */
            BBR_Data->BBR_Table = (BBR_Table_Entry *) LVM_Common_Services->Allocate( sizeof( BBR_Table_Entry ) * BBR_Primary_First_Sector->Table_Size );

            if ( BBR_Data->BBR_Table == NULL )
            {

              *Error_Code = DLIST_OUT_OF_MEMORY;
              LVM_Common_Services->Deallocate(BBR_Data);
              LVM_Common_Services->Deallocate(BBR_Context_Data);

              if ( LVM_Common_Services->Logging_Enabled )
              {

                sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory to hold the BBR data found!");
                LVM_Common_Services->Write_Log_Buffer();

              }

              FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

              return FALSE;

            }

            /* Since the copies do not match, we need to get them back in sync.  Setting ChangesMade to TRUE will
               cause the copy of the table which is used to be written back to both locations on disk when the user
               does a commit, thereby bringing both on disk copies back into sync.                                   */
            BBR_Data->ChangesMade = TRUE;

            /* Since we have all of the memory we need, lets update the PartitionRecord and fill in the actual BBR Table. */
            PartitionRecord->Feature_Data = BBR_Context_Data;

            /* Update the Feature_Index to indicate that this feature has been successfully activated for this partition. */
            PartitionRecord->Feature_Index += 1;

            /* Get the BBR Table. */
            Extract_BBR_Table( PartitionRecord, FALSE, Old_BBR_Format);

          }

        }

      }
      else
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"No valid BBR data was found.  Both copies of the feature data were invalid!");
          LVM_Common_Services->Write_Log_Buffer();

        }

        /* Since this partition specifies BBR in its LVM Signature Sector and the BBR data is not valid, we must prevent it from
           being used.  We can do this by removing it from the Partition_List.                                                     */

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Calling the delete function of the next layer down to free memory.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        /* Call the layers below us so that they can free any memory that they may have associated with this partition. */
        Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;
        Old_Function_Table->Delete(PartitionRecord,FALSE,Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          *Error_Code = DLIST_CORRUPTED;

        }

        FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

        return TRUE;

      }

    }

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Partition accepted as a BBR partition.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  FEATURE_FUNCTION_EXIT("Find_BBR_Partitions")

  return FALSE;

}


static BOOLEAN Feature_Data_Is_Valid( BOOLEAN  Use_Primary_Buffer, CARDINAL32  Sectors_To_Validate )
{

  LVM_BBR_Table_First_Sector *     First_Sector;
  LVM_BBR_Table_Sector *           Next_Sector;
  CARDINAL32                       Sector_Count;
  ADDRESS                          Buffer;
  CARDINAL32                       Offset;
  CARDINAL32                       Old_CRC;
  CARDINAL32                       Calculated_CRC;
  CARDINAL32                       Sequence_Count;

  FEATURE_FUNCTION_ENTRY("Feature_Data_Is_Valid")

  /* Select which buffer to validate. */
  if ( Use_Primary_Buffer )
    Buffer = &Feature_Data_Buffer1;
  else
    Buffer = &Feature_Data_Buffer2;

  /* Validate each sector of data contained in the buffer. */
  for ( Sector_Count = 1; Sector_Count < Sectors_To_Validate; Sector_Count++)
  {

    /* The first sector is different than the ones that follow it.  Handle it separately. */
    if ( Sector_Count == 1)
    {

      /* Find the location of the first sector in the buffer. */
      First_Sector = (LVM_BBR_Table_First_Sector *) Buffer;

      /* Check the signature. */
      if ( First_Sector->Signature != BBR_TABLE_MASTER_SIGNATURE )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Signature does not match.  Feature Data is invalid.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the Table_Size, Replacement_Sector_Count, Sectors_Per_Table, and Table_Entries_In_Use */
      if ( ( First_Sector->Table_Size == 0 ) ||
           ( First_Sector->Table_Size < First_Sector->Table_Entries_In_Use ) ||
           ( First_Sector->Table_Size != First_Sector->Replacement_Sector_Count ) ||
           ( First_Sector->Table_Size > ( First_Sector->Sectors_Per_Table * BBR_TABLE_ENTRIES_PER_SECTOR ) )
         )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"BBR Table characteristics are inconsistent.  Feature Data is invalid.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the CRC. */
      Old_CRC = First_Sector->CRC;
      First_Sector->CRC = 0;

      Calculated_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, First_Sector, BYTES_PER_SECTOR);

      if ( Calculated_CRC != Old_CRC )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"CRC failure.  Feature Data is invalid.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Save the Sequence_Count.  All of the remaining sectors must have the same sequence count! */
      Sequence_Count = First_Sector->Sequence_Number;

    }
    else
    {

      /* Locate the next sector within the buffer. */
      Offset = BYTES_PER_SECTOR * ( Sector_Count - 1 );
      Next_Sector = (LVM_BBR_Table_Sector *) Buffer;
      Next_Sector = (LVM_BBR_Table_Sector *) ( (CARDINAL32) Next_Sector + Offset );

      /* Check the signature. */
      if ( Next_Sector->Signature != BBR_TABLE_SIGNATURE )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Signature does not match.  Feature Data is invalid.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the CRC. */
      Old_CRC = Next_Sector->CRC;
      Next_Sector->CRC = 0;

      Calculated_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, Next_Sector, BYTES_PER_SECTOR);

      if ( Calculated_CRC != Old_CRC )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"CRC failure.  Feature Data is invalid.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the Sequence Number. */
      if ( Next_Sector->Sequence_Number != Sequence_Count )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Sequence number mis-match.  Feature Data is invalid.");
          LVM_Common_Services->Write_Log_Buffer();

        }

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

    }

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Feature Data appears valid.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

  return TRUE;

}


static void Extract_BBR_Table(Partition_Data * PartitionRecord, BOOLEAN Use_Primary_Buffer, BOOLEAN Old_BBR_Format)
{

  LVM_BBR_Table_First_Sector *     First_Sector;
  LVM_BBR_Table_Sector *           Next_Sector;
  BBR_Data_Record *                BBR_Data = (BBR_Data_Record *) PartitionRecord->Feature_Data->Data;
  CARDINAL32                       Sector_Count;
  ADDRESS                          Buffer;
  CARDINAL32                       Offset;
  CARDINAL32                       CurrentEntry;
  CARDINAL32                       EntriesMoved = 0;

  FEATURE_FUNCTION_ENTRY("Extract_BBR_Table")

  /* Select which buffer to use. */
  if ( Use_Primary_Buffer )
    Buffer = &Feature_Data_Buffer1;
  else
    Buffer = &Feature_Data_Buffer2;

  /* Establish access to the first BBR Feature Data Sector. */
  First_Sector = (LVM_BBR_Table_First_Sector *) Buffer;

  /* Fill in some of the fields in the BBR_Data. */
  BBR_Data->BBR_Table_Size = First_Sector->Table_Size;
  BBR_Data->Entries_In_Use = First_Sector->Table_Entries_In_Use;
  BBR_Data->Sequence_Number = First_Sector->Sequence_Number;
  if ( Old_BBR_Format )
  {

    BBR_Data->Actual_Class = Partition_Class;
    BBR_Data->Top_Of_Class = TRUE;
    BBR_Data->Feature_Sequence_Number = 1;

  }
  else
  {

    BBR_Data->Actual_Class = First_Sector->Actual_Class;
    BBR_Data->Top_Of_Class = First_Sector->Top_Of_Class;
    BBR_Data->Feature_Sequence_Number = First_Sector->Feature_Sequence_Number;

  }

  /* Copy the BBR Table entries from each sector in the buffer to the BBR Table in memory. */
  for ( Sector_Count = 2; Sector_Count < First_Sector->Sectors_Per_Table; Sector_Count++)
  {

    /* Locate the next sector within the buffer. */
    Offset = BYTES_PER_SECTOR * ( Sector_Count - 1 );
    Next_Sector = (LVM_BBR_Table_Sector *) Buffer;
    Next_Sector = (LVM_BBR_Table_Sector *) ( (CARDINAL32) Next_Sector + Offset );

    /* Now we must extract the BBR Table entries from this sector and place them in the BBR Table. */
    for ( CurrentEntry = 0; CurrentEntry < BBR_TABLE_ENTRIES_PER_SECTOR; CurrentEntry++ )
    {

      BBR_Data->BBR_Table[EntriesMoved] = Next_Sector->BBR_Table[CurrentEntry];

      EntriesMoved++;

      if ( EntriesMoved == BBR_Data->BBR_Table_Size )
        break;

    }

  }

  FEATURE_FUNCTION_EXIT("Extract_BBR_Table")

  return;

}


static void _System Remove_BBR_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  Partition_Data *       PartitionRecord = (Partition_Data *) Object;
  Feature_Context_Data * BBR_Context_Data;
  LVM_Signature_Sector * Signature_Sector;                            /* Used when accessing the LVM Signature Sector of the partition. */
  CARDINAL32             Sectors_Per_Table;
  BBR_Data_Record *      BBR_Data;
  CARDINAL32             FeatureIndex;

  FEATURE_FUNCTION_ENTRY("Remove_BBR_Data")

#ifdef DEBUG

#ifdef PARANOID

  assert(Object != NULL);
  assert(ObjectSize == sizeof(Partition_Data) );
  assert(Parameters == NULL);
  assert(ObjectTag == PARTITION_DATA_TAG);

#else

  if ( ( Object == NULL ) ||
       ( ObjectSize != sizeof( Partition_Data ) ) ||
       ( Parameters != NULL ) ||
       ( ObjectTag != PARTITION_DATA_TAG )
     )
  {

    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Remove_BBR_Data")

    return;

  }

#endif

#endif

  /* Assume success. */
  *Error_Code = DLIST_SUCCESS;

  /* Get BBR Feature Data. */
  BBR_Context_Data = PartitionRecord->Feature_Data;
  if ( BBR_Context_Data == NULL )
  {


    FEATURE_FUNCTION_EXIT("Remove_BBR_Data")

    /* All done!  No BBR Feature Data to remove. */
    return;

  }

  /* Is this feature data for BBR? */
  if ( BBR_Context_Data->Function_Table != &Function_Table )
  {

    FEATURE_FUNCTION_EXIT("Remove_BBR_Data")

    /* All done!  This context data is not for BBR! */
    return;

  }

  /* Restore the previous context data for the partition. */
  PartitionRecord->Feature_Data = BBR_Context_Data->Old_Context;

  /* Begin to free the components of the BBR Context Data. */
  BBR_Data = BBR_Context_Data->Data;
  if ( BBR_Data->BBR_Table != NULL )
    LVM_Common_Services->Deallocate(BBR_Data->BBR_Table);

  /* Calculate the number of sectors required to hold the BBR Table. */
  Sectors_Per_Table = BBR_Data->BBR_Table_Size / BBR_TABLE_ENTRIES_PER_SECTOR;
  if ( (BBR_Data->BBR_Table_Size % BBR_TABLE_ENTRIES_PER_SECTOR) != 0 )
    Sectors_Per_Table += 1;

  Sectors_Per_Table++;

  /* Now we must remove BBR from the LVM Signature Sector. */
  Signature_Sector = PartitionRecord->Signature_Sector;
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME ; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == BBR_FEATURE_ID )
    {

      /* We have found the BBR entry!  Lets remove it. */
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID = 0;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Active = FALSE;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Data_Size = 0;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number = 0;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Minor_Version_Number = 0;

      /* Return the reserved replacement sectors. */
      Signature_Sector->LVM_Reserved_Sector_Count -= BBR_Data->BBR_Table_Size;

      /* Return the reserved space for the primary copy of the BBR Feature Data. */
      Signature_Sector->LVM_Reserved_Sector_Count -= (1 + Sectors_Per_Table);
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data = 0;

      /* Return the reserved space for the secondary copy of the BBR Feature Data. */
      Signature_Sector->LVM_Reserved_Sector_Count -= (1 + Sectors_Per_Table);
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data = 0;

      /* Calculate the size of the partition as seen by the user. */
      Signature_Sector->Partition_Size_To_Report_To_User = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;

      /* Adjust the partitions usable size to account for the sectors we have freed. */
      PartitionRecord->Usable_Size = PartitionRecord->Partition_Size - Signature_Sector->LVM_Reserved_Sector_Count;

      /* Exit the loop. */
      break;

    }

  }

  /* Free the remaining memory. */
  LVM_Common_Services->Deallocate(BBR_Context_Data->Data);
  LVM_Common_Services->Deallocate(BBR_Context_Data);

  /* Indicate success. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Remove_BBR_Data")

  return;

}


static void _System Remove_Features(ADDRESS Aggregate, CARDINAL32 * Error_Code)
{

  Partition_Data *              PartitionRecord = (Partition_Data *) Aggregate;
  Plugin_Function_Table_V1 *    Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("Remove_Features")

  /* Remove the BBR data from the Aggregate/Partition. */
  Remove_BBR_Data(Aggregate, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, NULL, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    FEATURE_FUNCTION_EXIT("Remove_Features")

    return;

  }

  /* Now we must call the next layer down so that it can remove itself. */
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;
  Old_Function_Table->Remove_Features(PartitionRecord, Error_Code);

  FEATURE_FUNCTION_EXIT("Remove_Features")

  return;

}


static void _System PassThru( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code )
{

  Partition_Data *           PartitionRecord = (Partition_Data *) Aggregate;
  Feature_Context_Data  *    CurrentFeature;
  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("PassThru  (BBR)")

  /* BBR has no PassThru commands that it accepts, so this call should not be directed to BBR. */
  if ( Feature_ID == BBR_FEATURE_ID )
  {

    *Error_Code = LVM_ENGINE_BAD_FEATURE_ID;

    FEATURE_FUNCTION_EXIT("PassThru  (BBR)")

    return;

  }

  /* Is the feature data for the next layer available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR PassThru has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("PassThru  (BBR)")

    return;

  }

  /* Since this is not for us, pass it to the next feature. */
  CurrentFeature = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = CurrentFeature->Old_Context;
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  /* Is the feature data for the next layer available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR PassThru has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("PassThru  (BBR)")

    return;

  }

  Old_Function_Table->PassThru(Feature_ID, Aggregate, InputBuffer, InputSize, OutputBuffer, OutputSize, Error_Code);

  /* Put our context back! */
  CurrentFeature->Old_Context = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = CurrentFeature;

  FEATURE_FUNCTION_EXIT("PassThru  (BBR)")

  /* *Error_Code was set by the call to PassThru, so leave it alone. */
  return;

}


static void _System ReturnCurrentClass( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number )
{

  Partition_Data *     Partition_Record = (Partition_Data *) PartitionRecord;
  BBR_Data_Record *    BBR_Data;


  FEATURE_FUNCTION_ENTRY("ReturnCurrentClass")

  /* This function should only be called for LVM Volumes. */

  /* Get the BBR Feature data. */
  BBR_Data = (BBR_Data_Record *) Partition_Record->Feature_Data->Data;

  /* Return the requested values. */
  *Actual_Class = BBR_Data->Actual_Class;
  *Top_Of_Class = BBR_Data->Top_Of_Class;
  *Sequence_Number = BBR_Data->Feature_Sequence_Number;

  FEATURE_FUNCTION_EXIT("ReturnCurrentClass")

  return;

}


static BOOLEAN _System BBR_ChangesPending(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code)
{

  Feature_Context_Data  *    CurrentFeature;
  Plugin_Function_Table_V1 * Old_Function_Table;
  BBR_Data_Record *          BBR_Data;
  BOOLEAN                    ReturnValue = FALSE;

  FEATURE_FUNCTION_ENTRY("BBR_ChangesPending")

  /* Is our feature data available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_ChangesPending")

    return FALSE;

  }

  /* Get our feature data. */
  CurrentFeature = PartitionRecord->Feature_Data;

  /* Does our BBR Data exist? */
  if ( CurrentFeature->Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_ChangesPending")

    return FALSE;

  }

  /* Get our BBR Data. */
  BBR_Data = (BBR_Data_Record *) CurrentFeature->Data;

  /* Do we have any changes pending? */
  if ( BBR_Data->ChangesMade )
  {

    *Error_Code = LVM_ENGINE_NO_ERROR;

    FEATURE_FUNCTION_EXIT("BBR_ChangesPending")

    return TRUE;

  }

  /* Since we don't have any changes pending, we must find out if the next layer below us does.
     Therefore, we must pass this request to the next feature. */
  PartitionRecord->Feature_Data = CurrentFeature->Old_Context;
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  /* Is the feature data for the next layer available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"BBR ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("BBR_ChangesPending")

    return FALSE;

  }

  ReturnValue = Old_Function_Table->ChangesPending(PartitionRecord, Error_Code);

  /* Put our context back! */
  CurrentFeature->Old_Context = PartitionRecord->Feature_Data;
  PartitionRecord->Feature_Data = CurrentFeature;

  FEATURE_FUNCTION_EXIT("BBR_ChangesPending")

  /* *Error_Code was set by the call to ChangesPending, so leave it alone. */
  return ReturnValue;

}


static void _System BBR_ParseCommandLineArguments(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code )
{


  FEATURE_FUNCTION_ENTRY("BBR_ParseCommandLineArguments")

  *Init_Data = NULL;
  *Error_Message = NULL;

  *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

  FEATURE_FUNCTION_EXIT("BBR_ParseCommandLineArguments")

  return;

}
