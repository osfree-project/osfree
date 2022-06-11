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
 * Module: logging.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: The following functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in Logging.C:
 *
 *              Start_Logging
 *              Stop_Logging
 *
 *
 * Description: Since the LVM Engine Interface is very
 *              large, it became undesirable to put all of the code
 *              to implement it in a single file.  Instead, the
 *              implementation of the LVM Engine Interface was divided
 *              among several C files which would have access to a
 *              core set of data.  Engine.H and Engine.C define,
 *              allocate, and initialize that core set of data.
 *              Logging.H and Logging.C implement the logging
 *              functions described in the LVM Engine Interface.
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
 *    BootManager.C          Logging.H                       Handle_Manager.C
 *
 * Notes: This module is used to maintain a copy of the original partitioning
 *        information for inclusion in a log file if logging is active.  It
 *        can also be used to restore the original configuration.  If a
 *        Commit_Changes operation is performed, and it completes successfully,
 *        then the new configuration will be saved as the original configuration
 *        by this module.
 *
 */

#define NEED_BYTE_DEFINED
#include "engine.h"
#include "lvm_gbls.h"  /* CARDINAL32 */
#define NEED_BYTE_DEFINED
#include "lvm_intr.h"   /* */

#define DECLARE_LOGGING_GLOBALS
#include "logging.h"
#include "alvm.h"

#include <stdio.h>         /* file I/O functions. */
#include <time.h>          /* time, ctime */
#include <assert.h>

/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/



/*--------------------------------------------------
 * There are no private Type definitions
 --------------------------------------------------*/



/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static FILE   * Log_File = (FILE *) NULL;       /* Handle of the log file. */

/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
void _System Log_Volumes_And_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);


/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: Log_Current_Configuration                        */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Log_Current_Configuration( void )
{

  CARDINAL32  Index;      /* Used to walk the DriveArray. */
  CARDINAL32  Error_Code; /* Used when calling the ForEachItem function. */

  /* Is logging enabled? */
  if ( Logging_Enabled )
  {

    /* Logging is enabled.  Log the current configuration. */

    /* Is the LVM Engine open? */
    if ( DriveArray != NULL )
    {

      fprintf(Log_File,"\n\n*****************************************************************************\n");
      fprintf(Log_File,"*****************************************************************************\n\n");

      fprintf(Log_File,"                         LVM Engine Status Report\n\n");

      fprintf(Log_File,"\n\n*****************************************************************************\n");
      fprintf(Log_File,"*****************************************************************************\n\n");

      /* Now output the volume information for the system. */
      fprintf(Log_File,"\n\n============================  Volume Status Report  ============================\n\n");

      /* Traverse the list of volumes and log each volume's data. */
      ForEachItem(Volumes,&Log_Volumes_And_Partitions,NULL,TRUE,&Error_Code);

      /* Now we will output the data on the physical drives in the system. */
      fprintf(Log_File,"\n\n============================  Drive Status Report  =============================\n\n");

      /* For each drive in the drive array, output all the information that has been gathered so far. */
      for (Index = 0; Index < DriveCount; Index++ )
      {

        fprintf(Log_File,"\n\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n");

        fprintf(Log_File,"                          Drive Information for drive %d.\n\n",Index + 1);

        fprintf(Log_File,"Drive Array Index: %lu\n",DriveArray[Index].DriveArrayIndex);
        fprintf(Log_File,"External Handle: %lx\n",(CARDINAL32) DriveArray[Index].External_Handle);
        fprintf(Log_File,"Drive Size ( in sectors ): %lu\n",DriveArray[Index].Drive_Size);
        fprintf(Log_File,"Sectors per Cylinder: %lu\n",DriveArray[Index].Sectors_Per_Cylinder);
        fprintf(Log_File,"1024 Cylinder Limit LBA: %lu\n",DriveArray[Index].Cylinder_Limit);
        fprintf(Log_File,"Drive Serial Number assigned by LVM: %lu\n",DriveArray[Index].Drive_Serial_Number);
        fprintf(Log_File,"Boot Drive Serial Number: %lu\n",DriveArray[Index].Boot_Drive_Serial_Number);
        fprintf(Log_File,"Installation Flags ( only valid for drive 1 ): %lx\n",DriveArray[Index].Install_Flags);
        fprintf(Log_File,"Number of primary partitions on this drive: %lu\n",DriveArray[Index].Primary_Partition_Count);
        fprintf(Log_File,"Number of logical partitions on this drive: %lu\n",DriveArray[Index].Logical_Partition_Count);
        fprintf(Log_File,"Drive Geometry: %u Cylinders, %u Heads, %u Sectors per track.\n", DriveArray[Index].Geometry.Cylinders, DriveArray[Index].Geometry.Heads, DriveArray[Index].Geometry.Sectors);
        fprintf(Log_File,"Drive Name: %#20.20s\n",DriveArray[Index].Drive_Name);

        fprintf(Log_File,"\nDrive Attributes:\n");

        if ( DriveArray[Index].Cylinder_Limit_Applies )
          fprintf(Log_File,"     The 1024 Cylinder Limit applies.\n");
        else
          fprintf(Log_File,"     The 1024 Cylinder Limit does NOT apply.\n");

        if ( DriveArray[Index].ChangesMade )
          fprintf(Log_File,"     Changes have been made to this drive.\n");
        else
          fprintf(Log_File,"     This drive has not been altered yet.\n");

        if ( DriveArray[Index].IO_Error )
          fprintf(Log_File,"     There have been I/O errors on this drive!\n");
        else
          fprintf(Log_File,"     This drive has had NO I/O errors.\n");

        if ( DriveArray[Index].Corrupt )
        {
           if( DriveArray[Index].NonFatalCorrupt)
           {   fprintf(Log_File,"     The partition tables on this drive seems nonfatal corrupt!\n");
           } else {
               fprintf(Log_File,"     The partition tables on this drive are corrupt!\n");
           }
        }
        else
          fprintf(Log_File,"     The partition tables on this drive are valid.\n");

        if ( DriveArray[Index].Record_Initialized )
          fprintf(Log_File,"     This drive array entry has been fully initialized.\n");
        else
          fprintf(Log_File,"     This drive array entry has NOT been fully initialized.\n");

        if ( DriveArray[Index].Is_PRM )
          fprintf(Log_File,"     This drive is a PRM.\n");
        else
          fprintf(Log_File,"     This drive is NOT a PRM.\n");

        if ( DriveArray[Index].Is_Big_Floppy )
          fprintf(Log_File,"     This drive is formatted as a big floppy.\n");
        else
          fprintf(Log_File,"     This drive is NOT formatted as a big floppy.\n");

        if ( DriveArray[Index].Reboot_Flag )
          fprintf(Log_File,"     The Reboot_Flag used by Install is set.\n");
        else
          fprintf(Log_File,"     The Reboot_Flag used by Install is clear.\n");

        if ( DriveArray[Index].Unusable )
          fprintf(Log_File,"     This drive is UNUSABLE due to I/O errors on the MBR!\n");
        else
          fprintf(Log_File,"     This drive is usable.\n");

        fprintf(Log_File,"\nThe partitions on this drive are as follows: \n");

        /* Now output the partition information for this drive. */
        if ( DriveArray[Index].Partitions != NULL)
          ForEachItem(DriveArray[Index].Partitions, &Log_Volumes_And_Partitions, NULL, TRUE, &Error_Code);

      }

      /* Now indicate that our LVM Engine Status Report is complete. */
      fprintf(Log_File,"\n\n***********************************************************\n");
      fprintf(Log_File,"###############################################################\n\n");

      fprintf(Log_File,"         End of LVM Engine Status Report\n\n");

      fprintf(Log_File,"\n\n###########################################################\n");
      fprintf(Log_File,"***************************************************************\n\n");

      fflush(Log_File);

    }
    else
      fprintf(Log_File,"\n\nThe LVM Engine is currently CLOSED.\n\n");

  }

  return;

}

/*********************************************************************/
/*                                                                   */
/*   Function Name: Write_Log_Buffer                                 */
/*                                                                   */
/*   Descriptive Name: Appends the contents of the log buffer to the */
/*                     log file.                                     */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: None.                                           */
/*                                                                   */
/*   Side Effects: If disk space is available, then the contents of  */
/*                 the log buffer are appended to the log file.      */
/*                                                                   */
/*   Notes:  This function assumes that the contents of the log      */
/*           buffer is a single, null terminated string.  This       */
/*           function will add a CF/LF to the end of the string.     */
/*                                                                   */
/*********************************************************************/
void _System Write_Log_Buffer( void )
{

  /* Is logging enabled? */
  if ( Logging_Enabled )
  {

    /* Logging is enabled.  Write the contents of the log file to disk. */
    fprintf(Log_File,"%s\n", Log_Buffer);

    /* fflush(Log_File); */   /* fflush was not flushing the file buffers! */

  }

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Start_Logging                                    */
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
void Start_Logging( char * Filename, CARDINAL32 * Error_Code)
{

  time_t  Current_Time;     /* Used to get the current time. */
  int     IO_Result;        /* Used to hold the return value from fprintf. */

  /* Try to open the log file. */
  Log_File = fopen(Filename, "a");

  /* Did we succeed? */
  if ( Log_File == (FILE *) NULL )
  {

    /* We can not open the log file!  Indicate the error and abort. */
    *Error_Code = LVM_ENGINE_CAN_NOT_OPEN_LOG_FILE;

    return;

  }

  /* Disable buffering of the log file. */
  setbuf(Log_File,NULL);

  /* Since the log file was created successfully, indicate that logging is enabled and return to caller. */
  Logging_Enabled =  1;


  /* Now write out a message to mark the beginning of a new section in the log file.  This is to cover the
     case where we are appending to an existing log file.  This allows us to distinguish where one log
     ends and another begins.                                                                                */


  if ( ( fprintf(Log_File, "\n\n****************************************************************\n") < 0 ) ||
       ( fprintf(Log_File, "*         ALVM  vers %s Log File                               *\n", ALVM_VERSION) < 0 ) ||
       ( fprintf(Log_File, "*****************************************************************\n\n") < 0 ) ||
       ( fprintf(Log_File, "This log file was created on ") < 0 )
     )
  {

    /* We had an error writing to the output file!  Close the output file and abort! */
    fclose(Log_File);

    /* Disable logging. */
    Logging_Enabled = 0;

    /* Set an error code. */
    *Error_Code = LVM_ENGINE_CAN_NOT_WRITE_TO_LOG_FILE;

    return;

  }

  /* Now output the date and time at which the log file was created. */

  if ( time( &Current_Time ) != (time_t) -1 )
  {

    IO_Result = fprintf(Log_File,"%s\n\n", ctime(&Current_Time) );

  }
  else
  {

    IO_Result = fprintf(Log_File,"< Date and Time not available! >\n\n");

  }

  /* Did we succeed in writing the date and time to the log file? */
  if ( IO_Result < 0 )
  {

    /* We had an error writing to the output file!  Close the output file and abort! */
    fclose(Log_File);

    /* Disable logging. */
    Logging_Enabled = FALSE;

    /* Set an error code. */
    *Error_Code = LVM_ENGINE_CAN_NOT_WRITE_TO_LOG_FILE;

    return;

  }

  /* If the LVM Engine is already open, then log its current state.  */
  if ( DriveArray != NULL )
  {

    /* Indicate in the log file that what we are going to be putting in the log is not the initial LVM Engine state but
       the state of the LVM Engine as we found it since it was already open when logging started.                        */
    if ( fprintf(Log_File,"The LVM Engine was opened prior to the start of logging.\n\nThe current state of the LVM Engine follows: \n\n") >= 0 )
      Log_Current_Configuration();
    else
    {

      /* We had an error writing to the output file!  Close the output file and abort! */
      fclose(Log_File);

      /* Disable logging. */
      Logging_Enabled = 0;

      /* Set an error code. */
      *Error_Code = LVM_ENGINE_CAN_NOT_WRITE_TO_LOG_FILE;

  return;

    }

  }

  /* All done.  Indicate success and return to caller. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Stop_Logging                                     */
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
void Stop_Logging ( CARDINAL32 * Error_Code )
{

  /* Is logging enabled? */
  if ( Logging_Enabled )
  {

    /* Indicate that logging has been disabled. */
    Logging_Enabled = 0;

    /* Close the log file. */
    *Error_Code = fclose(Log_File);

    /* We don't care about whether or not the close was successful, so indicate success and exit. */
    *Error_Code = LVM_ENGINE_NO_ERROR;

  }

  return;

}



/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: Log_Volumes_And_Partitions                       */
/*   Descriptive Name:                                               */
/*   Input:                                                          */
/*   Output:                                                         */
/*   Error Handling:                                                 */
/*   Side Effects:                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void _System Log_Volumes_And_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access the Volume_Data without having to typecast each time. */
  Volume_Data * VolumeRecord = ( Volume_Data *) Object;

  /* Assume success. */
  *Error_Code = DLIST_SUCCESS;

  /* Lets see what we have to work with! */
  switch ( ObjectTag )
  {
    case PARTITION_DATA_TAG : /* We must log the contents of this partition record. */
                              fprintf(Log_File,"\n\n----------------------------------PARTITION----------------------------------\n\n");

                              fprintf(Log_File,"This Partition's External Handle: %lx (HEX)\n",PartitionRecord->External_Handle);
                              fprintf(Log_File,"This Partition's Drive Index: %lu (DEC)\n",PartitionRecord->Drive_Index);
                              fprintf(Log_File,"This Partition's Internal Handle: %lx (HEX)\n",PartitionRecord->Drive_Partition_Handle);
                              fprintf(Log_File,"The External Handle of the Volume this partition belongs to: %lx (HEX)\n",PartitionRecord->External_Volume_Handle);
                              fprintf(Log_File,"The Internal Handle of the Volume this partition belongs to: %lx (HEX)\n",PartitionRecord->Volume_Handle);
                              fprintf(Log_File,"The LBA of the first sector of this partition: %lx (HEX)\n",PartitionRecord->Starting_Sector);
                              fprintf(Log_File,"The size of this partition, in sectors: %lu (DEC)\n",PartitionRecord->Partition_Size);
                              fprintf(Log_File,"The usable size of this partition, in sectors: %lu (DEC)\n",PartitionRecord->Usable_Size);
                              fprintf(Log_File,"The name of this partition is: %#20.20s\n",PartitionRecord->Partition_Name);
                              fprintf(Log_File,"The name of the filesystem used on this partition is: %#20.20s\n",PartitionRecord->File_System_Name);

                              fprintf(Log_File,"\nThe partition table entry for this partition contains the following:\n");
                              fprintf(Log_File,"     The Boot Indicator is: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Boot_Indicator);
                              fprintf(Log_File,"     The Starting Head field contains: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Starting_Head);
                              fprintf(Log_File,"     The Starting Sector field contains: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Starting_Sector);
                              fprintf(Log_File,"     The Starting Cylinder field contains: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Starting_Cylinder);
                              fprintf(Log_File,"     The Format Indicator is: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Format_Indicator);
                              fprintf(Log_File,"     The Ending Head field contains: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Ending_Head);
                              fprintf(Log_File,"     The Ending Sector field contains: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Ending_Sector);
                              fprintf(Log_File,"     The Ending Cylinder field contains: %lx (HEX)\n", (CARDINAL32) PartitionRecord->Partition_Table_Entry.Ending_Cylinder);
                              fprintf(Log_File,"     The Sector Offset is: %lu (DEC)\n", PartitionRecord->Partition_Table_Entry.Sector_Offset);
                              fprintf(Log_File,"     The Sector Count is: %lu (DEC)\n", PartitionRecord->Partition_Table_Entry.Sector_Count);

                              fprintf(Log_File,"\nThe DLA Table Entry for this partition contains the following:\n");
                              fprintf(Log_File,"     Volume Serial Number = %lu (DEC)\n",PartitionRecord->DLA_Table_Entry.Volume_Serial_Number);
                              fprintf(Log_File,"     Partition Serial Number = %lu (DEC)\n",PartitionRecord->DLA_Table_Entry.Partition_Serial_Number);
                              fprintf(Log_File,"     Partition Size = %lu (DEC)\n",PartitionRecord->DLA_Table_Entry.Partition_Size);
                              fprintf(Log_File,"     The LBA of the first sector of the partition is %lu (DEC)\n",PartitionRecord->DLA_Table_Entry.Partition_Start);
                              fprintf(Log_File,"     The drive letter assigned to this partition: %c\n",PartitionRecord->DLA_Table_Entry.Drive_Letter);
                              fprintf(Log_File,"     Volume Name = %#20.20s\n",PartitionRecord->DLA_Table_Entry.Volume_Name);
                              fprintf(Log_File,"     Partition Name = %#20.20s\n",PartitionRecord->DLA_Table_Entry.Partition_Name);

                              if ( PartitionRecord->DLA_Table_Entry.On_Boot_Manager_Menu )
                                fprintf(Log_File,"     On Boot Manager Menu is set to TRUE.\n");
                              else
                                fprintf(Log_File,"     On Boot Manager Menu is set to FALSE.\n");

                              if ( PartitionRecord->DLA_Table_Entry.Installable )
                                fprintf(Log_File,"     Installable is set to TRUE.\n");
                              else
                                fprintf(Log_File,"     Installable set to FALSE.\n");


                              fprintf(Log_File,"\nThis partition has the following attributes:\n");

                              if ( PartitionRecord->Partition_Type == FreeSpace )
                                fprintf(Log_File,"     This partition record represents free space on the drive.\n");
                              else
                                if ( PartitionRecord->Partition_Type == Partition )
                                  fprintf(Log_File,"     This partition record represents a partition on the drive.\n");
                                else
                                  if ( PartitionRecord->Partition_Type == MBR_EBR )
                                    fprintf(Log_File,"     This partition record represents an MBR or EBR.\n");
                                  else
                                    fprintf(Log_File,"     THE PARTITION_TYPE FIELD FOR THIS PARTITION RECORD IS INVALID!\n");

                              if ( PartitionRecord->New_Partition )
                                fprintf(Log_File,"     This partition record represents a new partition.\n");
                              else
                                fprintf(Log_File,"     This partition record represents a pre-existing partition.\n");

                              if ( PartitionRecord->Primary_Partition )
                                fprintf(Log_File,"     This partition record represents a primary partition.\n");
                              else
                                fprintf(Log_File,"     This partition record represents a logical drive.\n");

                              if ( PartitionRecord->Spanned_Volume)
                                fprintf(Log_File,"     This partition is part of a spanned volume.\n");
                              else
                                fprintf(Log_File,"     This partition is NOT part of a spanned volume.\n");

                              if ( PartitionRecord->Migration_Needed )
                                fprintf(Log_File,"     This partition was placed on the Boot Manager menu by FDISK and needed to be migrated.\n");
                              else
                                fprintf(Log_File,"     This partition does NOT need to be migrated by LVM.\n");

                              /* Is this partition record represent a real partition, or is it a composite? */
                              if ( ( PartitionRecord->Feature_Data != NULL ) &&
                                   ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
                                 )
                              {

                                fprintf(Log_File,"\n\nThis partition record is a composite.  It represents the following partitions:\n\n");

                                ForEachItem(PartitionRecord->Feature_Data->Partitions,&Log_Volumes_And_Partitions,NULL,TRUE,Error_Code);

                              }

                              break; /* Keep the compiler happy. */

    case VOLUME_DATA_TAG : /* We must log the contents of this volume record. */
                           fprintf(Log_File,"\n\n-------------------------------------VOLUME----------------------------------\n\n");

                           fprintf(Log_File,"The Serial Number of this Volume is: %lu (DEC)\n",VolumeRecord->Volume_Serial_Number);
                           fprintf(Log_File,"The external handle of this Volume is: %lx (HEX)\n",VolumeRecord->External_Handle);
                           fprintf(Log_File,"The internal handle of this volume is: %lx (HEX)\n",VolumeRecord->Volume_Handle);
                           fprintf(Log_File,"The size of this volume (in sectors) is: %lu (DEC)\n",VolumeRecord->Volume_Size);
                           fprintf(Log_File,"This volume is comprised of %lu (DEC) partitions.\n",VolumeRecord->Partition_Count);
                           fprintf(Log_File,"The Drive Letter Conflict Code for this volume is: %lu (DEC)\n",VolumeRecord->Drive_Letter_Conflict);
                           fprintf(Log_File,"The name of this volume is: %.20s\n",VolumeRecord->Volume_Name);
                           fprintf(Log_File,"The filesystem used on this volume is: %.20s\n",VolumeRecord->File_System_Name);
                           fprintf(Log_File,"The drive letter preference for this volume is: %c\n",VolumeRecord->Drive_Letter_Preference);
                           fprintf(Log_File,"The current drive letter for this volume is: %c\n",VolumeRecord->Current_Drive_Letter);
                           fprintf(Log_File,"The initial drive letter for this volume is: %c\n",VolumeRecord->Initial_Drive_Letter);
                           fprintf(Log_File,"The device type for this volume is %u (DEC)\n", VolumeRecord->Device_Type);
                           fprintf(Log_File,"\nThis Volume has the following attributes:\n");

                           if ( VolumeRecord->New_Volume)
                             fprintf(Log_File,"     This is a newly created volume.\n");
                           else
                             fprintf(Log_File,"     This is a prior existing volume.\n");

                           if ( VolumeRecord->Can_Be_Altered )
                             fprintf(Log_File,"     This volume can be altered.\n");
                           else
                             fprintf(Log_File,"     This volume can NOT be altered.\n");

                           if ( VolumeRecord->Compatibility_Volume )
                             fprintf(Log_File,"     This is a Compatibility Volume.\n");
                           else
                             fprintf(Log_File,"     This is an LVM Volume.\n");

                           if (VolumeRecord->Foreign_Volume)
                             fprintf(Log_File,"     This is a foreign volume.\n");
                           else
                             fprintf(Log_File,"     This is NOT a foreign volume.\n");

                           if ( VolumeRecord->On_Boot_Manager_Menu )
                             fprintf(Log_File,"     This Volume is on the Boot Manager Menu.\n");
                           else
                             fprintf(Log_File,"     This Volume is NOT on the Boot Manager Menu.\n");

                           if ( VolumeRecord->Expand_Volume)
                             fprintf(Log_File,"     This volume has been expanded during this LVM Engine session.\n");
                           else
                             fprintf(Log_File,"     This volume has NOT been expanded during this LVM Engine session.\n");

                           if ( VolumeRecord->ChangesMade)
                             fprintf(Log_File,"     There have been changes made to this Volume during this LVM Engine session.\n");
                           else
                             fprintf(Log_File,"     This volume has not been altered during the current LVM Engine session.\n");

                           fprintf(Log_File,"\n\nThe partitions associated with this volume follow:\n\n");

                           if ( VolumeRecord->Partition != NULL )
                             Log_Volumes_And_Partitions( VolumeRecord->Partition, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, NULL, Error_Code);

                           break;   /* Keep the compiler happy. */

    default :
              /* We have an unknown object in the list!  Abort! */
              *Error_Code = DLIST_CORRUPTED;

              fprintf(Log_File,"\n\nBad object in Partition/Volume list!\n\n");

              break;  /* Keep the compiler happy. */

  }

  /* Make sure our output has reached the disk, if possible. */
  fflush(Log_File);

  return;

}

