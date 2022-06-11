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
 * Module: Pass_Thru.c
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

#define NEED_BYTE_DEFINED
#include "engine.h"   /* Included for access to the global types and variables. */
#include "lvm_gbls.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */

#include "lvm_plug.h"

#include "dlist.h"    /*  */
#include "diskio.h"   /*  */


#include "lvm_cons.h"   /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */
#define NEED_BYTE_DEFINED
#include "lvm_intr.h"   /* */

#include "crc.h"               /* INITIAL_CRC, CalculateCRC */

#include "Pass_Thru.h"

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


/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static Plugin_Function_Table_V1   Function_Table;
static LVM_Common_Services_V1  *  LVM_Common_Services;
static Feature_ID_Data            Feature_ID_Record;
static BYTE                       Fake_EBR_Buffer[BYTES_PER_SECTOR];   /* Used to manipulate "fake" EBRs on LVM partitions created by LVM Version 1. */

/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
static void _System Discover_Pass_Thru_Links( DLIST  Partition_List, CARDINAL32 * Error_Code );
static void _System Create_Pass_Thru_Volume( DLIST Partition_List,
                                             ADDRESS VData,
                                             ADDRESS Init_Data,
                                             void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                             LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                             BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                             CARDINAL32   Sequence_Number,
                                             CARDINAL32 * Error_Code );
static BOOLEAN _System Can_Expand_PT_Volume( ADDRESS Aggregate, CARDINAL32 * Feature_ID, CARDINAL32 * Error_Code );
static void    _System Add_PT_Partition ( ADDRESS Aggregate, ADDRESS New_Partition, CARDINAL32 * Error_Code );
static void    _System Delete_PT_Partition( ADDRESS AData, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code);
static void    _System Open_Feature( CARDINAL32 * Error_Code);
static void    _System Close_Feature( void );
static void    _System Commit_PT_Changes( ADDRESS VolumeRecord, ADDRESS PartitionRecord, CARDINAL32 * Error_Code );
static void    _System PT_Write( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code);
static void    _System PT_Read( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code);
static void    _System PT_PassThru( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code );
static void    _System PT_Remove_Features(ADDRESS Aggregate, CARDINAL32 * Error_Code);
static void    _System Add_Pass_Thru_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void    _System ReturnCurrentClass( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number );
static BOOLEAN _System PT_ChangesPending(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code);
static void    _System PT_ParseCommandLineArguments(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code );


/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

void Pass_Thru_Get_Required_LVM_Version( CARDINAL32 * Major_Version_Number, CARDINAL32 * Minor_Version_Number)
{

  *Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
  *Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;

  return;

}

ADDRESS Pass_Thru_Exchange_Function_Tables( ADDRESS PT_Common_Services )
{

  /* Initialize the Feature_ID_Record. */
  strcpy(Feature_ID_Record.Name, "Pass Through");
  Feature_ID_Record.Short_Name[0] = 0x0;
  Feature_ID_Record.OEM_Info[0] = 0x0;
  Feature_ID_Record.ID = PASS_THRU_FEATURE_ID;
  Feature_ID_Record.Major_Version_Number = PASS_THRU_MAJOR_VERSION;
  Feature_ID_Record.Minor_Version_Number = PASS_THRU_MINOR_VERSION;
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
  Function_Table.Open_Feature = &Open_Feature;
  Function_Table.Close_Feature = &Close_Feature;
  Function_Table.Can_Expand = &Can_Expand_PT_Volume;
  Function_Table.Add_Partition = &Add_PT_Partition;
  Function_Table.Delete = &Delete_PT_Partition;
  Function_Table.Discover = &Discover_Pass_Thru_Links;
  Function_Table.Remove_Features = &PT_Remove_Features;
  Function_Table.Create = &Create_Pass_Thru_Volume;
  Function_Table.Commit = &Commit_PT_Changes;
  Function_Table.Write = &PT_Write;
  Function_Table.Read = &PT_Read;
  Function_Table.ReturnCurrentClass = &ReturnCurrentClass;
  Function_Table.PassThru = &PT_PassThru;
  Function_Table.ChangesPending = &PT_ChangesPending;
  Function_Table.ParseCommandLineArguments = &PT_ParseCommandLineArguments;

  /* Save the common functions provided by LVM.DLL. */
  LVM_Common_Services = ( LVM_Common_Services_V1 * ) PT_Common_Services;

  /* Return our table of functions to LVM.DLL. */
  return (ADDRESS) &Function_Table;

}


/*--------------------------------------------------
 * Private Functions Available
 --------------------------------------------------*/
static void Discover_Pass_Thru_Links( DLIST  Partition_List, CARDINAL32 * Error_Code )
{

  /* Pass Thru does nothing here. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static void _System Create_Pass_Thru_Volume( DLIST Partition_List,
                                             ADDRESS VData,
                                             ADDRESS Init_Data,
                                             void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                             LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                             BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                             CARDINAL32   Sequence_Number,
                                             CARDINAL32 * Error_Code )
{

  /* We must add the Pass Through layer to each partition. */
  ForEachItem(Partition_List, &Add_Pass_Thru_Data, NULL, TRUE, Error_Code );

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( *Error_Code == DLIST_OUT_OF_MEMORY)
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static BOOLEAN Can_Expand_PT_Volume( ADDRESS Aggregate, CARDINAL32 * Feature_ID, CARDINAL32 * Error_Code )
{

  /* Pass Thru volumes can not be expanded!  */
  *Feature_ID = 0;
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return FALSE;

}

static void Add_PT_Partition ( ADDRESS Aggregate, ADDRESS New_Partition, CARDINAL32 * Error_Code )
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *    PartitionRecord = (Partition_Data *) New_Partition;

  /* Pass Through is the first feature on any LVM volume.  Is it the first here? */
  if ( PartitionRecord->Feature_Data != NULL )
  {

    /* We have a problem.  There may be a feature on this partition already!. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    return;

  }

  /* Now allocate space for Feature Data. */
  PartitionRecord->Feature_Data = (Feature_Context_Data *) malloc ( sizeof( Feature_Context_Data ) );

  /* Did we get the memory? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    return;

  }

  /* Now initialize the feature data. */
  PartitionRecord->Feature_Data->Feature_ID = &Feature_ID_Record;
  PartitionRecord->Feature_Data->Function_Table = &Function_Table;
  PartitionRecord->Feature_Data->Data = NULL;
  PartitionRecord->Feature_Data->Partitions = NULL;
  PartitionRecord->Feature_Data->Old_Context = NULL;

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static void Delete_PT_Partition( ADDRESS AData, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code)
{

  Partition_Data * Aggregate = (Partition_Data *) AData;
  CARDINAL32       Signature_Sector_Location = 0;
  Kill_Sector_Data Sector_To_Overwrite;           /* Used to place sectors in the KillSector list. */


  /* Free the LVM Signature Sector, if there is one. */
  if ( Aggregate->Signature_Sector )
  {

    /* Save the location of the signature sector so that we can overwrite it. */
    Signature_Sector_Location = Aggregate->Signature_Sector->Partition_End;

    /* Now clear the memory used to store the LVM Signature Sector and free it. */
    memset(Aggregate->Signature_Sector,0,sizeof(LVM_Signature_Sector) );
    free(Aggregate->Signature_Sector);
    Aggregate->Signature_Sector = NULL;

  }


#ifdef DEBUG

#ifdef PARANOID

  assert( Aggregate->Feature_Data != NULL );
  assert( Aggregate->Feature_Data->Data == NULL);
  assert( Aggregate->Feature_Data->Partitions == NULL);

#else

  /* There should be Feature Data, but not Partitions for the Pass Thru layer. */
  if ( ( Aggregate->Feature_Data == NULL ) ||
       ( Aggregate->Feature_Data->Data != NULL ) ||
       ( Aggregate->Feature_Data->Partitions != NULL )
     )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* Free the Feature_Data for this partition. */
  free( Aggregate->Feature_Data );

  /* Pass Thru is the ending point for a chain of Delete calls.  If Kill_Partitions is TRUE, then we will
     cause the Aggregate, which has been reduced to just a partition by this point, to be deleted.         */

  /* Reset some fields. */
  Aggregate->Spanned_Volume = FALSE;                   /* This is just a partition now. */
  Aggregate->Usable_Size = Aggregate->Partition_Size;  /* No reserved sectors for LVM data. */
  Aggregate->DLA_Table_Entry.Volume_Serial_Number = 0;
  Aggregate->DLA_Table_Entry.Drive_Letter = 0x00;
  Aggregate->DLA_Table_Entry.Volume_Name[0] = 0x00;
  Aggregate->DLA_Table_Entry.Installable = FALSE;
  Aggregate->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;
  Aggregate->Volume_Handle = NULL;
  Aggregate->External_Volume_Handle = NULL;
  Aggregate->Feature_Data = NULL;

  if ( Kill_Partitions )
  {

    if ( ( Signature_Sector_Location != 0 ) && ( !Aggregate->New_Partition ) )
    {

      /* This partition had an LVM Signature Sector.  We must add this sector to the Kill list. */
      Sector_To_Overwrite.Drive_Index = Aggregate->Drive_Index;
      Sector_To_Overwrite.Sector_ID = Signature_Sector_Location;

      InsertItem(KillSector, sizeof(Kill_Sector_Data), &Sector_To_Overwrite, KILL_SECTOR_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

      /* We won't check the error code here as it is not fatal if the sector does not get added to the KillSector list. */

    }

    /* Now we will actually delete the partition. */
    Delete_Partition(Aggregate->External_Handle, Error_Code);

    return;

  }

  /* Indicate no error and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static void Open_Feature(CARDINAL32 * Error_Code)
{

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static void _System Close_Feature( void )
{

  /* The pass through layer has nothing to do here! */
  return;

}

static void Commit_PT_Changes( ADDRESS VolumeRecord, ADDRESS PartitionRecord, CARDINAL32 * Error_Code )
{

  Volume_Data *           VData = (Volume_Data *) VolumeRecord;
  Partition_Data *        PData = (Partition_Data *) PartitionRecord;
  LVM_Signature_Sector *  Signature_Sector;
  Extended_Boot_Record *  Fake_EBR = ( Extended_Boot_Record * ) Fake_EBR_Buffer;


  /* Pass Thru does not change anything, so there are no changes to commit.  However, as Pass Thru is the bottom
     feature layer, it must write the LVM Signature Sector back to the partition.                                 */

#ifdef DEBUG

#ifdef PARANOID

  assert( PData->Signature_Sector != NULL );

#else

  if ( PData->Signature_Sector == NULL )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    return;

  }

#endif

#endif

  /* We must calculate the CRC of the LVM Signature Sector for this partition. */

  /* Establish access to the LVM Signature Sector. */
  Signature_Sector = PData->Signature_Sector;

  /* Do we need to convert to LVM Version 1 format?  This could occur if the current LVM was installed
     as part of a fixpak and the fixpak is now being backed out for some reason.                       */
  if ( VData->Convert_To_LVM_V1 )
  {

    /* We must change the version numbers for the signature sector so that LVM Version 1 will accept them. */
    Signature_Sector->LVM_Major_Version_Number = 1;
    Signature_Sector->LVM_Minor_Version_Number = 0;

  }

  /* For compatibility with LVM Version 1, any signature sectors created by LVM Version 1 must be
     "scrubbed" - i.e. those fields added for LVM Version 2 must be set to 0!                       */
  if ( Signature_Sector->LVM_Major_Version_Number < 2 )
  {

    Signature_Sector->Sequence_Number = 0;
    Signature_Sector->Next_Aggregate_Number = 0;

  }

  /* The Signature Sector CRC must be 0 when calculating the CRC of the LVM Signature Sector. */
  Signature_Sector->Signature_Sector_CRC = 0;

  /* Actually calculate the CRC. */
  Signature_Sector->Signature_Sector_CRC = CalculateCRC(INITIAL_CRC, Signature_Sector, BYTES_PER_SECTOR);

  /* Now write the signature sector to disk. */
  WriteSectors(PData->Drive_Index + 1, PData->Starting_Sector + PData->Partition_Size - 1, 1, Signature_Sector, Error_Code);

  /* Was there an error? */
  if ( *Error_Code != DISKIO_NO_ERROR )
  {

    if ( *Error_Code == DISKIO_WRITE_FAILED )
    {

      /* Mark the drive as having had an I/O failure. */
      DriveArray[PData->Drive_Index].IO_Error = TRUE;

    }
    else
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      return;

    }

  }

  /* Mark the drive as having had changes so that a Rediscover operation will be performed on it.
     Rediscover causes OS2DASD and OS2LVM to revisit drives on which there have been changes to see
     if the changes affect any volumes.                                                             */
  DriveArray[PData->Drive_Index].ChangesMade = TRUE;


  /* At this level we are dealing with partitions.  Partitions will not normally have a fake EBR associated with them,
     unless they were created by Version 1 of LVM.  In Version 1 of LVM, whenever an LVM Volume was created, a fake EBR
     would be created in the LVM Data area of the first partition in the LVM Volume.  In version 2 of LVM, the fake EBR
     is associated with the topmost aggregate of an LVM Volume, if there is one.  If there are no aggregators, then
     there is no need for a fake EBR and the corresponding LVM Volume will not have one.  Either way, the only time we
     expect to see an LVM Signature Sector for a partition indicate that there is a fake EBR associated with the
     partition is if the partition is part of an LVM Volume created by version 1 of LVM.  Whenever we see such a Signature
     Sector, we must ensure that the size of the partition in the fake EBR matches the size of the volume.  This means
     that we must read in the fake EBR, update Sector_Count field in the first partition table entry, and then save it
     to disk again.  This ensures that if a volume is expanded, the fake EBR is updated to reflect the new size of the volume. */

  if ( Signature_Sector->Fake_EBR_Allocated )
  {

    /* Now read the fake EBR from the disk. */
    ReadSectors(PData->Drive_Index + 1, Signature_Sector->Fake_EBR_Location, 1, Fake_EBR_Buffer, Error_Code);
    if ( *Error_Code != DISKIO_NO_ERROR )
    {

      if ( *Error_Code == DISKIO_READ_FAILED )
      {

        /* Mark the drive as having had an I/O failure. */
        DriveArray[PData->Drive_Index].IO_Error = TRUE;

      }
      else
      {

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        return;

      }

    }
    else
    {

      /* We will recreate the fake EBR. */
      memset(Fake_EBR_Buffer,0, BYTES_PER_SECTOR);

      /* Now create the partition table entry for this volume. */
      Fake_EBR->Partition_Table[0] = VData->Partition->Partition_Table_Entry;

      /* Now set the EBR Signature. */
      Fake_EBR->Signature = MBR_EBR_SIGNATURE;

      /* Now write the fake EBR to disk. */
      WriteSectors(PData->Drive_Index + 1, Signature_Sector->Fake_EBR_Location, 1, Fake_EBR_Buffer, Error_Code);
      if ( *Error_Code != DISKIO_NO_ERROR )
      {

        if ( *Error_Code == DISKIO_WRITE_FAILED )
        {

          /* Mark the drive as having had an I/O failure. */
          DriveArray[PData->Drive_Index].IO_Error = TRUE;

        }
        else
        {

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          return;

        }

      }

    }

  }

  /* Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static void PT_Write( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code)
{

  Partition_Data * PartitionRecord = (Partition_Data *) PData;

  /* Translate the call from PT_Write to WriteSectors. */
  WriteSectors(PartitionRecord->Drive_Index + 1, Starting_Sector, Sectors_To_Write, Buffer, Error_Code);

  /* Translate the error code to an LVM_ENGINE error code. */
  if ( *Error_Code != DISKIO_NO_ERROR)
  {

    if (*Error_Code != DISKIO_WRITE_FAILED)
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    else
      *Error_Code = LVM_ENGINE_IO_ERROR;

    DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

  }
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}

static void PT_Read( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code)
{

  Partition_Data * PartitionRecord = (Partition_Data *) PData;

  /* Translate the call from PT_Read to ReadSectors. */
  ReadSectors(PartitionRecord->Drive_Index + 1, Starting_Sector, Sectors_To_Read, Buffer, Error_Code);

  /* Translate the error code to an LVM_ENGINE error code. */
  if ( *Error_Code != DISKIO_NO_ERROR)
  {

    if (*Error_Code != DISKIO_WRITE_FAILED)
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    else
      *Error_Code = LVM_ENGINE_IO_ERROR;

    DriveArray[PartitionRecord->Drive_Index].IO_Error = TRUE;

  }
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


static void _System PT_PassThru( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code )
{

  /* The pass through layer is the last layer, and it accepts no Pass Thru commands.  There is nothing else to pass through to.  This must be an error! */
  *Error_Code = LVM_ENGINE_BAD_FEATURE_ID;

  return;

}


static void  _System PT_Remove_Features(ADDRESS Aggregate, CARDINAL32 * Error_Code)
{

  /* Declare a local variable for use in accessing Aggregate. */
  Partition_Data * PartitionRecord = (Partition_Data *) Aggregate;

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* Check the feature data to make sure that it is the Pass Through feature data. */
  if ( ( PartitionRecord->Feature_Data != NULL ) &&
       (PartitionRecord->Feature_Data->Function_Table == (ADDRESS) PassThru_Function_Table )
     )
  {

    free(PartitionRecord->Feature_Data);
    PartitionRecord->Feature_Data = NULL;

  }
  else
  {

    /* The feature data is not the Pass Through feature data!  Abort! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }

  return;

}


static void _System Add_Pass_Thru_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *                PartitionRecord = (Partition_Data *) Object;

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Pass Through is the first feature on any LVM volume.  Is it the first here? */
  if ( PartitionRecord->Feature_Data != NULL )
  {

    /* We have a problem.  There may be a feature on this partition already!. */
    *Error = DLIST_CORRUPTED;
    return;

  }

  /* Now allocate space for Feature Data. */
  PartitionRecord->Feature_Data = (Feature_Context_Data *) malloc ( sizeof( Feature_Context_Data ) );

  /* Did we get the memory? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    *Error = DLIST_OUT_OF_MEMORY;
    return;

  }

  /* Now initialize the feature data. */
  PartitionRecord->Feature_Data->Feature_ID = &Feature_ID_Record;
  PartitionRecord->Feature_Data->Function_Table = &Function_Table;
  PartitionRecord->Feature_Data->Data = NULL;
  PartitionRecord->Feature_Data->Partitions = NULL;
  PartitionRecord->Feature_Data->Old_Context = NULL;

  /* Indicate success. */
  *Error = DLIST_SUCCESS;

  return;

}


static void _System ReturnCurrentClass( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number )
{

  /* This function should only be called on LVM Volumes.  */

  /* PassThru is always the bottom feature on any LVM Volume. */
  *Actual_Class = Partition_Class;
  *Top_Of_Class = FALSE;
  *Sequence_Number = 0;

  return;

}


static BOOLEAN _System PT_ChangesPending(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code)
{

  /* The pass through layer is the last layer, and it has no changes pending, ever. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return FALSE;

}


static void _System PT_ParseCommandLineArguments(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code )
{

  /* The pass through layer is the last layer, and it accepts no command line arguments.  It should also never be called! */
  *Init_Data = NULL;
  *Error_Message = NULL;
  *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  return;

}

