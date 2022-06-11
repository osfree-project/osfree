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
 * Module: DiskIO.c
 */

/*
 * Change History:
 *
 */

/*
 * Functions: BOOLEAN    OpenDrives
 *            void       CloseDrives
 *            CARDINAL32 GetDriveCount
 *            void       GetDriveGeometry
 *            void       ReadSectors
 *            void       WriteSectors
 *
 * Description: This module provides an LBA based means of reading and writing
 *              to the various disk drives in the system.
 *
 * Notes: This module is single threaded and is not reentrant.
 *
 */

#define INCL_32
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSPROCESS
#include <os2.h>             /* DosSetRelMaxFH, DosPhysicalDisk, PTRACKLAYOUT, Category 9 IOCTLs, DosQuerySysInfo */
#include <bsedev.h>          /* OEMHLP_QUERYDISKINFO, OEMHLPDISKINFO */
#include <stdio.h>           /* sprintf */
#include <stdlib.h>          /* malloc, free */
#include <string.h>          /* strlen */
#include <ctype.h>           /* toupper */
#include "lvm_gbls.h"        /* BOOLEAN, CARDINAL16, CARDINAL32 */
#include "lvm_type.h"       /* LBA */
#include "lvm_cons.h"        /* BYTES_PER_SECTOR */
#include "lock.h"            /* SEMAPHORE, CreateSemaphore, Lock, Unlock, DestroySemaphore. */
#include "lvm_intr.h"        /* Get_LVM_View */
#include "dlist.h"           /* DLIST, CreateList, InsertItem */
#include "diskio.h"          /* Prototypes for functions in this file. */
#include "logging.h"

#ifdef DEBUG

#include <assert.h>

#endif




/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define PRM_INDICATOR              0x08
#define OEMHLPDD                   "OEMHLP$"
#define IOCTL_LOGICAL_DRIVE        0x08
#define GET_OS2LVM_VIEW            0x0D
#define SET_PRM_REDISCOVERY_STATE  0x0E
#define GENERIC_FEATURE_COMMAND    0x0F
#define LVM_IOCTL_FUNCTION         0x69
#define REDISCOVER_DRIVE_IOCTL     0x6A


/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/
typedef struct _DiskDriveData {
                                 CARDINAL16     Cylinders;               /* The number of cylinders on the drive. */
                                 CARDINAL16     Heads;                   /* The number of heads on the drive. */
                                 CARDINAL16     SectorsPerTrack;         /* The number of sectors per track on the drive. */
                                 CARDINAL16     DriveHandle;             /* The handle used to access this drive with Category 9 IOCTL commands. */
                                 CARDINAL32     TrackLayoutSize;         /* The size, in bytes, of the TrackLayoutTable pointed to by TrackLayout. */
                                 TRACKLAYOUT *  TrackLayout;             /* The track layout table for this drive for use with the Category 9 IOCTL commands. */
                                 BOOLEAN        Initialized;             /* If TRUE, then this structure has been initialized. */
                                 BOOLEAN        Is_PRM;                  /* If TRUE, then the drive represented by this structure is a removable media device. */
                                 BOOLEAN        Cylinder_Limit_Applies;  /* Set to TRUE if the 1024 cylinder limit applies to this drive. */
                                 SEMAPHORE      Drive_Lock;              /* Band-aid to fix locking problems with FT.  It seems that the FT people
                                                                            missed the part in the LVM SDD1 which states that LVM.DLL is single threaded
                                                                            and expects any multi-threaded programs using it to do their own locking.
                                                                            Unfortunately, it is now too late to make all of the required changes to the
                                                                            FT code, so LVM.DLL must provide a band-aid to keep FT working.  LVM.DLL will
                                                                            now do its own locking on the I/O paths.                                              */
                              } DiskDriveData;

typedef struct _DDI_OS2LVMVIEW_data
{
  UCHAR       DriveNumber;                  /* 1 based drive number */
  UCHAR       DriveLetter;                  /* OS2LVM's drive letter */
  UCHAR       UnitID;                       /* IFSM's unit ID */
  ULONG       Starting_LBA;                 /* LBA for this volume */
} DDI_OS2LVMVIEW_data, FAR *PDDI_OS2LVMVIEW_data;

typedef struct _DDI_OS2LVM_param
{
  UCHAR       Command;
  UCHAR       DriveUnit;
  USHORT      TableNumber;
  ULONG       LSN;
  ULONG       Feature_ID;
  ULONG       Flags;
  ULONG       CurrentInstance;
  ULONG       StartingLSN;
  ULONG       EndingLSN;
} DDI_OS2LVM_param, FAR *PDDI_OS2LVM_param;

typedef struct _DDI_OS2LVM_data
{
  UCHAR        ReturnData;
  void*        UserBuffer;
} DDI_OS2LVM_data, FAR *PDDI_OS2LVM_data;

typedef struct _DDI_SETPRMSTATE_data
{
                                            /* 0 = enabled, 1 = disabled */
  UCHAR       NewState;                     /* New PRM rediscovery state */
  UCHAR       OldState;                     /* Old PRM rediscovery state */
  UCHAR       PRM_Rediscovery_In_Progress;  /* Cnt of In progress PRM rediscovories */
} DDI_SETPRMSTATE_data, FAR *PDDI_SETPRMSTATE_data;


/*--------------------------------------------------
 Private global variables.
--------------------------------------------------*/
static CARDINAL16       DriveCount = 0;       /* The number of hard drives in the system. */
static DiskDriveData *  DriveTable = NULL;    /* Points to an array of DiskDriveData structures - 1 per physical drive in the system. */


/*--------------------------------------------------
 Private functions.
--------------------------------------------------*/
static void Do_IO ( CARDINAL32   Drive_Number,
                    LBA          Starting_Sector,
                    CARDINAL32   SectorCount,
                    ADDRESS      Buffer,
                    BOOLEAN      Write,
                    CARDINAL32 * Error);


/*--------------------------------------------------
 There are no public global variables.
--------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/


/*********************************************************************/
/*                                                                   */
/*   Function Name: Rediscover                                       */
/*                                                                   */
/*   Descriptive Name:  This function causes the I/O subsystem to    */
/*                      re-examine the specified physical drives and */
/*                      discover any changes that may have been made */
/*                      to them.                                     */
/*                                                                   */
/*   Input:  PDDI_Rediscover_param Rediscovery_Parameters : This is  */
/*                      the parameter packet used by this function to*/
/*                      call the Rediscover IOCTL.  It consists of   */
/*                      the number of drives to perform rediscovery  */
/*                      on and an array of the OS/2 drive numbers of */
/*                      those drives.                                */
/*           PDDI_Rediscover_data Rediscovery_Data : This is the     */
/*                      parameter packet used by this functin to call*/
/*                      the Rediscover IOCTL.  It consists of a field*/
/*                      which is set by OS2LVM upon return           */
/*                      (NewIFSMUnits), a field to indicate how many */
/*                      volumes are being extended as part of this   */
/*                      rediscover operation (DDI_TotalExtends), and */
/*                      an array of the volume serial numbers and    */
/*                      unit IDs of the volumes to be extended.      */
/*                                                                   */
/*   Output:  The function return value will be NO_ERROR (see OS/2   */
/*            error defines ) if the function completed successfully.*/
/*            If the function did not complete successfully, the     */
/*            error code returned by OS/2 will be returned by this   */
/*            function.                                              */
/*                                                                   */
/*   Error Handling: If this function fails, the state of the system */
/*                   is unknown and the system should be rebooted.   */
/*                                                                   */
/*   Side Effects: OS2DASD, OS2LVM, and the IFSM may alter their     */
/*                 internal data structures as a result of this call.*/
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 Rediscover( PDDI_Rediscover_param  Rediscovery_Parameters, PDDI_Rediscover_data Rediscovery_Data)
{

  CARDINAL32           Parameter_Size;     /* Used for the DosDevIOCtl API. */
  CARDINAL32           Data_Size;          /* Used for the DosDevIOCtl API. */
  APIRET               Return_Code;        /* Used for the DosDevIOCtl API. */

  /* Initialize the IOCTL data. */
  Data_Size =  ( Rediscovery_Data->DDI_TotalExtends * sizeof(DDI_ExtendRecord) ) + sizeof(DDI_Rediscover_data) - sizeof(DDI_ExtendRecord);
  Rediscovery_Data->NewIFSMUnits = 0;

  /* Initialize the IOCTL Parameters. */
  Parameter_Size = ( Rediscovery_Parameters->DDI_TotalDrives * sizeof(UCHAR) ) + sizeof(DDI_Rediscover_param) - sizeof(BYTE);

  /* Do the IOCTL call. */
  Return_Code = DosDevIOCtl(DriveTable[0].DriveHandle,
                            IOCTL_PHYSICALDISK,
                            REDISCOVER_DRIVE_IOCTL,
                            Rediscovery_Parameters,
                            Parameter_Size,
                            &Parameter_Size,
                            Rediscovery_Data,
                            Data_Size,
                            &Data_Size);

/*   printf("\n\n%d\n\n",Return_Code);  */

 return Return_Code;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Get_LVM_View                                     */
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
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN Get_LVM_View( char IFSM_Drive_Letter, CARDINAL32 * Drive_Number, CARDINAL32 * Partition_LBA, char * LVM_Drive_Letter, BYTE * UnitID)
{

  DDI_OS2LVM_param      LVM_Parameters;     /* Used to call the OS2LVM View IOCTL. */
  DDI_OS2LVMVIEW_data   LVM_View;           /* Returned by the OS2LVM View IOCTL. */
  DDI_OS2LVM_data       LVM_Data;           /* Returned by the OS2LVM View IOCTL. */
  CARDINAL32            Parameter_Size;     /* Used for the DosDevIOCtl API. */
  CARDINAL32            Data_Size;          /* Used for the DosDevIOCtl API. */
  APIRET                Return_Code;        /* Used for the DosDevIOCtl API. */

  /* Initialize the return values in case of IOCTL failure. */
  *Drive_Number = 0;
  *Partition_LBA = 0;
  *LVM_Drive_Letter = 0;
  *UnitID = 0xFF;

  /* Initialize the IOCTL Parameters. */
  LVM_Parameters.Command = GET_OS2LVM_VIEW;
  LVM_Parameters.DriveUnit = toupper( IFSM_Drive_Letter ) - 'A';
  LVM_Parameters.TableNumber = 0;
  LVM_Parameters.LSN = 0;

  /* Initialize the IOCTL data. */
  LVM_Data.ReturnData = 0;
  LVM_Data.UserBuffer = &LVM_View;

  /* Initialize the Parameter size and the Data size. */
  Parameter_Size = sizeof(DDI_OS2LVM_param);
  Data_Size = sizeof(DDI_OS2LVM_data);

  /* Do the IOCTL call. */
  Return_Code = DosDevIOCtl(-1,
                            IOCTL_LOGICAL_DRIVE,
                            LVM_IOCTL_FUNCTION,
                            &LVM_Parameters,
                            Parameter_Size,
                            &Parameter_Size,
                            &LVM_Data,
                            Data_Size,
                            &Data_Size);


  /* Return TRUE if the IOCTL was successful, FALSE otherwise. */
  if ( Return_Code == NO_ERROR )
  {

    /* Since the IOCTL succeeded, set the return values. */
    *Drive_Number = LVM_View.DriveNumber;
    *Partition_LBA = LVM_View.Starting_LBA;
    *LVM_Drive_Letter = (char) toupper(LVM_View.DriveLetter);
    *UnitID = (BYTE) LVM_View.UnitID;

    /* Indicate success */
    return TRUE;

  }
  else
    return FALSE;    /* Indicate failure. */

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: PRM_Rediscovery_Control                          */
/*                                                                   */
/*   Descriptive Name:  This function enables or disables the PRM    */
/*                      rediscovery process in OS2LVM.               */
/*                                                                   */
/*   Input:  BOOLEAN Enable : If TRUE, then PRM Rediscovery will be  */
/*                            enabled.  If FALSE, then PRM           */
/*                            Rediscovery will be disabled.          */
/*                                                                   */
/*   Output:  None.                                                  */
/*                                                                   */
/*   Error Handling: This function should never fail.                */
/*                                                                   */
/*   Side Effects: OS2DASD, OS2LVM, and the IFSM may alter their     */
/*                 internal data structures as a result of this call.*/
/*                                                                   */
/*   Notes:  None.                                                   */
/*********************************************************************/
void PRM_Rediscovery_Control( BOOLEAN  Enable )
{

  DDI_OS2LVM_param      LVM_Parameters;     /* Used to call OS2LVM. */
  DDI_OS2LVM_data       LVM_Data;           /* Used to call OS2LVM. */
  DDI_SETPRMSTATE_data  PRM_State_Data;     /* Used to call OS2LVM. */
  CARDINAL32            Parameter_Size;     /* Used for the DosDevIOCtl API. */
  CARDINAL32            Data_Size;          /* Used for the DosDevIOCtl API. */
  APIRET                Return_Code;        /* Used for the DosDevIOCtl API. */

  do
  {

    /* Initialize the IOCTL Parameters. */
    LVM_Parameters.Command = SET_PRM_REDISCOVERY_STATE;
    LVM_Parameters.DriveUnit = 0;
    LVM_Parameters.TableNumber = 0;
    LVM_Parameters.LSN = 0;

    /* Initialize the IOCTL data. */
    LVM_Data.ReturnData = 0;
    LVM_Data.UserBuffer = &PRM_State_Data;

    /* Initialize the PRM_State_Data. */
    PRM_State_Data.OldState = 0;
    PRM_State_Data.PRM_Rediscovery_In_Progress = 0;
    if (Enable)
      PRM_State_Data.NewState = 0;
    else
      PRM_State_Data.NewState = 1;

    /* Initialize the Parameter size and the Data size. */
    Parameter_Size = sizeof(DDI_OS2LVM_param);
    Data_Size = sizeof(DDI_OS2LVM_data);

    /* Do the IOCTL call. */
    Return_Code = DosDevIOCtl(-1,
                              IOCTL_LOGICAL_DRIVE,
                              LVM_IOCTL_FUNCTION,
                              &LVM_Parameters,
                              Parameter_Size,
                              &Parameter_Size,
                              &LVM_Data,
                              Data_Size,
                              &Data_Size);

    if ( ( !Enable ) && ( Return_Code == NO_ERROR ) && ( PRM_State_Data.PRM_Rediscovery_In_Progress != 0 ) )
    {

      /* We must wait for pending PRM Rediscoveries to complete. */
      DosSleep(1000L);

    }

  } while ( ( Return_Code == NO_ERROR ) && ( !Enable ) && ( PRM_State_Data.PRM_Rediscovery_In_Progress != 0 ) );


  /* Return to caller. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: OpenDrives                                       */
/*                                                                   */
/*   Descriptive Name:  This function prepares the DiskIo module for */
/*                      for use.  It determines the number of        */
/*                      OS2DASD controlled physical disks in the     */
/*                      system, gathers some basic information about */
/*                      them, and obtains a handle for each one.     */
/*                                                                   */
/*   Input:  CARDINAL32 *  Error : The address of a variable to hold */
/*                                 the error return code.            */
/*                                                                   */
/*   Output:  *Error will be 0 if this function completed            */
/*            successfully.  *Error will be > 0 if this function     */
/*            failed.                                                */
/*                                                                   */
/*   Error Handling: If this function fails, it will attempt to      */
/*                   restore the system to the state it was in prior */
/*                   to this function being called.                  */
/*                                                                   */
/*   Side Effects: A physical device handle will be opened for each  */
/*                 OS2DASD controlled drive in the system.           */
/*   Notes:  None.                                                   */
/*********************************************************************/
BOOLEAN OpenDrives( CARDINAL32 * Error )
{

  APIRET                 ReturnCode;             /* Used to hold the return code from OS/2 API calls. */
  CARDINAL32             I;                      /* Used to traverse the DriveTable. */
  CARDINAL32             J;                      /* Used to initialize the track layout tables for each drive. */
  INTEGER32              ExtraFileHandlesNeeded; /* Used to adjust the number of file handles for this process. */
  CARDINAL32             CurrentMaxFileHandles;  /* Used to hold the number of file handles currently allocated for this process. */
  char                   DriveName[5];           /* Used when obtaining a drive handle for each drive. */

  UCHAR                  CommandInformation;     /* Used for DosDevIOCtl. */
  CARDINAL32             CommandSize;            /* Used for DosDevIOCtl. */
  DEVICEPARAMETERBLOCK   DriveParameters;        /* Used for DosDevIOCtl. */
  CARDINAL32             ParameterSize;          /* Used for DosDevIOCtl. */

  OEMHLPDISKINFO         Drive_Data;             /* Used with the OEMHLP_QUERYDISKINFO IOCtl. */
  BYTE                 * Drive_Data_Check;       /* Used to check the Drive_Data. */

  HFILE                  FileHandle;             /* Used when trying to access the OEMHLP device driver to determine if a 1024 cylinder limit exists. */
  CARDINAL32             Action;                 /* Used when getting a handle to the OEMHLP device driver. */
  CARDINAL32             TotalSectors;           /* Used to check the data returned from the OEMHLP device driver. */


  /* If the array of DiskDriveData structures already exists, then this function has already been called and
     the DriveTable has already been established!  Return an error!                                           */
  if ( DriveTable != NULL )
  {
     *Error = DISKIO_DRIVES_ALREADY_OPEN;
     return FALSE;
  }

  /* We must now establish our drive table. */

  /* Find out how many drives there are so that we can properly size the drive table. */
  ReturnCode = DosPhysicalDisk(1L,(PVOID) &DriveCount, sizeof(DriveCount),NULL,0);

  if ( ReturnCode != NO_ERROR )
  {

     /* Since there was an error, we will have to assume that there are no drives. */
     DriveCount = 0;
     *Error = DISKIO_NO_DRIVES_FOUND;
     return FALSE;

  }

  /* Now that we know how many drives there are, we can allocate memory for the drive table. */

  /* Allocate memory for the array to hold that many DiskDriveData structures. */
  DriveTable = ( DiskDriveData * ) malloc( DriveCount * sizeof( DiskDriveData ) );

  /* Did we get the memory? */
  if ( DriveTable == NULL )
  {

    /* We are out of memory!  Abort! */
    DriveCount = 0;
    *Error = DISKIO_OUT_OF_MEMORY;
    return FALSE;

  }

  /* Initialize the memory we just got. */
  memset( DriveTable, 0, sizeof( DiskDriveData ) );

  /* We will need one file handle per drive in the system.  This could easily exceed the default number
     of file handles allocated to this process.  As a result, we will need to increate the number of
     file handles allocated for this process by the number of drives in the system.  This will ensure
     that we have enough file handles to do what we want.                                                 */

  /* Increase the number of handles allocated to this process by 20 plus the # of drives found. */
  ExtraFileHandlesNeeded = DriveCount;
  ReturnCode = DosSetRelMaxFH(&ExtraFileHandlesNeeded,&CurrentMaxFileHandles);

  /* Did we succeed in increasing the number of file handles allocated to this process? */
  if ( ReturnCode != NO_ERROR )
  {

    /* We could not increase the number of file handles allocated to this process!  We must
       abort since we can not guarantee that we have enough handles to complete our work.    */

    /* Free the memory we allocated for the DriveTable. */
    free(DriveTable);

    /* Reset the drive count so things look like we were never called. */
    DriveCount = 0;

    /* Report the error. */
    *Error = DISKIO_NOT_ENOUGH_HANDLES;

    return FALSE;

  }

  /* Get a handle to the OEMHLP device driver.  We will use it later to determine if there is a 1024 cylinder limit in effect. */
  ReturnCode = DosOpen(OEMHLPDD,&FileHandle,&Action,0L,0,1,0xC2,0L);
  if ( ReturnCode != NO_ERROR )
  {

    /* The device driver is not loaded!  Set the handle to 0. */
    FileHandle = 0;

  }

  /* Initialize the array of DiskDriveData structures. */
  for ( I = 0, ReturnCode = NO_ERROR; (I < DriveCount) && (ReturnCode == NO_ERROR); I++ )
  {

    /* Mark this DiskDriveData structure as being uninitialized. */
    DriveTable[I].Initialized = FALSE;


    /* To initialize this entry in the DriveTable, we must get a handle for this drive and get its geometry. */

    /* Construct the drive name. */
    sprintf(DriveName,"%lu:",I + 1);

    /* Open the drive. */
    ReturnCode = DosPhysicalDisk( 02L, &(DriveTable[I].DriveHandle), sizeof(DriveTable[I].DriveHandle),DriveName, strlen(DriveName) + 1 );

    if ( ReturnCode != NO_ERROR )
    {

      /* Indicate the error.  Since ReturnCode is not NO_ERROR, we will automatically leave the for loop. */
      *Error = DISKIO_FAILED_TO_GET_HANDLE;

    }
    else
    {

      /* Now lets get the drive's geometry. */
      CommandInformation = 0;
      CommandSize = sizeof(CommandInformation);
      ParameterSize = sizeof(DriveParameters);
      ReturnCode = DosDevIOCtl(DriveTable[I].DriveHandle,
                               IOCTL_PHYSICALDISK,
                               PDSK_GETPHYSDEVICEPARAMS,
                               &CommandInformation,
                               sizeof(CommandInformation),
                               &CommandSize,
                               &DriveParameters,
                               sizeof(DriveParameters),
                               &ParameterSize);

      /* Did we get the geometry successfully? */
      if ( ReturnCode != NO_ERROR )
      {

        /* Indicate the error.  */
        *Error = DISKIO_BAD_GEOMETRY;

        /* Close the handle for this drive. */
        ReturnCode = DosPhysicalDisk(3L,NULL,0,&(DriveTable[I].DriveHandle),sizeof(DriveTable[I].DriveHandle) );

        /* Set ReturnCode so that we will abort the for loop. */
        ReturnCode = NO_ERROR + 1;

      }
      else
      {

        /* Now that we have the drive's geometry, let's save it. */
        DriveTable[I].Cylinders = DriveParameters.cCylinders;
        DriveTable[I].Heads = DriveParameters.cHeads;
        DriveTable[I].SectorsPerTrack = DriveParameters.cSectorsPerTrack;

        /* Is the drive a PRM? */
        if ( ( DriveParameters.reserved2 & PRM_INDICATOR ) != 0 )
          DriveTable[I].Is_PRM = TRUE;
        else
          DriveTable[I].Is_PRM = FALSE;

        /* Now we must see if the 1024 cylinder limit is in effect for this drive. */

        /* We will assume that the BIOS does not support more than 1024 cylinders per drive. */
        DriveTable[I].Cylinder_Limit_Applies = TRUE;

        /* Do we have a handle to talk to the OEMHLP device driver? */
        if ( FileHandle != 0 )
        {

          /* Call the OEMHLP driver to see if the 1024 cylinder limit is in effect. */
          CommandInformation = 0x80 + I;                        /* BIOS Drive Number.  0x80 is the first drive in the system. */
          CommandSize = sizeof(CommandInformation);
          ParameterSize = sizeof(OEMHLPDISKINFO);
          memset(&Drive_Data, 0, sizeof(OEMHLPDISKINFO) );
          ReturnCode = DosDevIOCtl(FileHandle,
                                   IOCTL_OEMHLP,
                                   OEMHLP_QUERYDISKINFO,
                                   &CommandInformation,
                                   CommandSize,
                                   &CommandSize,
                                   &Drive_Data,
                                   ParameterSize,
                                   &ParameterSize);

          /* Does the BIOS support more than 1024 cylinders?  If the IOCTL returned an error, then the BIOS does not
             support more than 1024 cylinders.  If no error was returned, then we must check what was returned in Drive_Data. */
          if ( ReturnCode == NO_ERROR )
          {

            /* If the BIOS can't handle more than 1024 cylinders on this drive, then Drive_Data will be all zeros. */

            /* Get a pointer to the Drive_Data.  We will walk Drive_Data byte by byte checking for a non-zero value. */
            Drive_Data_Check = (BYTE *) &Drive_Data;

            /* Look for a non-zero value. */
            for ( J = 0; J < sizeof(OEMHLPDISKINFO); J++ )
            {

              /* Is the current value 0? */
              if ( Drive_Data_Check[J] != 0 )
              {

                /* Drive_Data is not all 0s!  This BIOS claims to support more than 1024 cylinders!  Now we must check
                   to see if the BIOS is functioning correctly.  Some BIOS claim to support more than 1024 cylinders, but
                   they don't actually do it correctly.  We will compare the total number of sectors based upon the geometry
                   we got from OS2DASD against that we got from the BIOS.  If the BIOS value is less than the OS2DASD value,
                   then we will assume that the BIOS is broken, otherwise we will assume that the BIOS is correct.            */
                TotalSectors = (CARDINAL32) DriveTable[I].Cylinders * (CARDINAL32) DriveTable[I].Heads * (CARDINAL32) DriveTable[I].SectorsPerTrack;
                if ( ( TotalSectors <= Drive_Data.ODI_PhysSectors_low ) && ( Drive_Data.ODI_PhysSectors_high == 0 ) )
                {

                  /* The BIOS appears to be working.  There is no 1024 cylinder limit on this drive! */
                   DriveTable[I].Cylinder_Limit_Applies = FALSE;

                }

                /* Exit the loop. */
                break;

              }

            }
//TODO как определить, что этот диск - USB ?
//EK  временно: всегда НЕ использовать ограничение

           DriveTable[I].Cylinder_Limit_Applies = FALSE;


          }
          else
            ReturnCode = NO_ERROR;

        }


        /* Lets allocate the semaphore that controls access to this drive. */
        DriveTable[I].Drive_Lock = CreateSemaphore();

        /* Did we get the semaphore? */
        if ( DriveTable[I].Drive_Lock == (SEMAPHORE)NULL )
        {

          /* We did not get the memory!  We must abort. */

          /* Close the handle for this drive. */
          ReturnCode = DosPhysicalDisk(3L,NULL,0,&(DriveTable[I].DriveHandle),sizeof(DriveTable[I].DriveHandle) );

          /* Set ReturnCode so that we will abort the for loop. */
          ReturnCode = NO_ERROR + 1;

          /* When we abort the for loop, CloseDrives will be called because ReturnCode is not NO_ERROR.  CloseDrives will
             cleanup the rest of the DriveTable.                                                                             */
          continue;

        }

        /* The last thing to do to complete the initialization of this record is to allocate and fill in the track layout table. */

        /* Allocate the track layout table. */
        DriveTable[I].TrackLayoutSize = sizeof(TRACKLAYOUT) + ( ( DriveTable[I].SectorsPerTrack - 1 ) * ( sizeof( USHORT ) * 2 ) );
        DriveTable[I].TrackLayout = (TRACKLAYOUT *) malloc( DriveTable[I].TrackLayoutSize );

        /* Did we get the memory? */
        if ( DriveTable[I].TrackLayout == NULL )
        {

          /* We did not get the memory!  We must abort. */

          /* Release the semaphore for this drive. */
          DestroySemaphore(DriveTable[I].Drive_Lock);

          /* Close the handle for this drive. */
          ReturnCode = DosPhysicalDisk(3L,NULL,0,&(DriveTable[I].DriveHandle),sizeof(DriveTable[I].DriveHandle) );

          /* Set ReturnCode so that we will abort the for loop. */
          ReturnCode = NO_ERROR + 1;

          /* When we abort the for loop, CloseDrives will be called because ReturnCode is not NO_ERROR.  CloseDrives will
             cleanup the rest of the DriveTable.                                                                             */

        }
        else
        {
          /* We got the memory!  Now we must initialize it. */

          /* Indicate that the TrackTable starts with sector 1 and contains only consecutively numbered sectors. */
          DriveTable[I].TrackLayout->bCommand = 1;

          /* Now initialize the TrackTable. */
          for ( J = 0; J < DriveTable[I].SectorsPerTrack; J++ )
          {

             DriveTable[I].TrackLayout->TrackTable[J].usSectorNumber = J + 1;
             DriveTable[I].TrackLayout->TrackTable[J].usSectorSize = BYTES_PER_SECTOR;

          }

          /* Now that we have completed initializing the record for this drive, mark it as being initialized. */
          DriveTable[I].Initialized = TRUE;

        } /* End of else clause for successfully getting the memory for the track layout table for the drive. */

      } /* End of else clause for successfully getting the drive geometry. */

    } /* End of else clause for successfully getting a drive handle. */

  }  /* End of for loop. */

  /* Are we holding a handle to the OEMHLP device driver? */
  if ( FileHandle != 0 )
  {

    /* We must release this handle. */
    DosClose(FileHandle);

  }

  if ( ReturnCode != NO_ERROR )
  {

    /* There was an error of some sort.  *Error has already been set, so we don't need to do it here.  We just need to call CloseDrives to have
       it clean up the DriveTable for us, and then return to caller indicating failure.                                                           */
    CloseDrives();

    return FALSE;

  }


  /* All done!  Signal success! */
  *Error = DISKIO_NO_ERROR;

  return TRUE;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: CloseDrives                                      */
/*                                                                   */
/*   Descriptive Name: This function closes any and all drive handles*/
/*                     that this module may have, as well as freeing */
/*                     all memory allocated by this module.          */
/*                                                                   */
/*   Input: None.                                                    */
/*   Output: None.                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects: All drive handles opened by OpenDrives are closed.*/
/*                 All memory allocated by OpenDrives is released.   */
/*   Notes:  None.                                                   */
/*********************************************************************/
void CloseDrives( void )
{
  APIRET                 ReturnCode;             /* Used to hold the return code from OS/2 API calls. */
  CARDINAL32             I;                      /* Used to traverse the DriveTable. */

  /* If the DriveTable is NULL, or if there are no drives, then we have nothing to do! */
  if ( ( DriveTable == NULL ) || ( DriveCount == 0 ) )
  {

    return;

  }

  /* Loop through the drive table.  For each initialized entry, close the drive handle and free the track layout table. */
  for ( I = 0; ( I < DriveCount ) && ( DriveTable[I].Initialized ) ; I++)
  {
    /* Close the drive handle. */
    ReturnCode = DosPhysicalDisk(3L,NULL,0,&(DriveTable[I].DriveHandle),sizeof(DriveTable[I].DriveHandle) );

    /* Free the track layout table. */
    free( DriveTable[I].TrackLayout );

    /* Release the semaphore for this drive. */
    DestroySemaphore(DriveTable[I].Drive_Lock);

    /* Clear out the fields in this drive table entry. */
    DriveTable[I].TrackLayout = NULL;
    DriveTable[I].Initialized = FALSE;
    DriveTable[I].DriveHandle = 0;

  }

  /* Free the memory occupied by the drive table. */
  free( DriveTable );

  /* Set DriveTable to NULL and DiskCount to 0 so that we don't accidentally try to use them again. */
  DriveTable = NULL;
  DriveCount = 0;

  /* Return to caller. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GetDriveCount                                    */
/*                                                                   */
/*   Descriptive Name: This function returns the number of hard      */
/*                     drives in the system.                         */
/*                                                                   */
/*   Input:  CARDINAL32 *  Error : The address of a variable to hold */
/*                                 the error return code.            */
/*                                                                   */
/*   Output: The function return value will be the number of hard    */
/*           drives in the system if no errors occur.  If an error   */
/*           occurs, then the function return value will be 0 and    */
/*           *Error will be > 0.                                     */
/*                                                                   */
/*   Error Handling: If no errors occur, *Error will be 0.  If an    */
/*                   error occurs, then *Error will be > 0.          */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
CARDINAL32 GetDriveCount( CARDINAL32 * Error )
{

  /* If DriveTable is NULL, then we have not been initialized yet by a call to OpenDrives, or CloseDrives has been called.
     Return an error.                                                                                                       */
  if ( DriveTable == NULL )
  {

    *Error = DISKIO_DRIVES_NOT_OPEN;
    return 0;

  }

  /* Indicate success and return DriveCount. */
  *Error = DISKIO_NO_ERROR;

  return DriveCount;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GetDriveGeometry                                 */
/*                                                                   */
/*   Descriptive Name: This function returns the geometry values for */
/*                     the specified drive.                          */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    get the geometry for.  The     */
/*                                    drives in the system are       */
/*                                    numbered from 1 to n, where n  */
/*                                    is the total number of hard    */
/*                                    drives in the system.          */
/*           Drive_Geometry_Record * Geometry : The location of a    */
/*                                              buffer to hold the   */
/*                                              drive geometry record*/
/*                                              for the specified    */
/*                                              drive.               */
/*           BOOLEAN * Is_PRM : The address of a variable which will */
/*                              be set to TRUE if the specified drive*/
/*                              is a PRM.                            */
/*           BOOLEAN * Cylinder_Limit_Applies : The address of a     */
/*                                              variable which will  */
/*                                              be set to TRUE if the*/
/*                                              1024 cylinder limit  */
/*                                              applies to this drive*/
/*           CARDINAL32 *  Error : The address of a variable to hold */
/*                                 the error return code.            */
/*                                                                   */
/*   Output: If there are no errors, *Error will be 0 and *Geometry  */
/*           will contain a valid Drive_Geometry_Record for the      */
/*           specified drive.  If an error occurs, *Error will be >0 */
/*           and *Geometry will contain all 0s.                      */
/*                                                                   */
/*   Error Handling: *Error will be 0 unless an error occurs.        */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void GetDriveGeometry( CARDINAL32              Drive_Number,
                       Drive_Geometry_Record * Geometry,
                       BOOLEAN *               Is_PRM,
                       BOOLEAN *               Cylinder_Limit_Applies,
                       CARDINAL32 *            Error )
{

  /* If DriveTable is NULL, then we have not been initialized yet by a call to OpenDrives, or CloseDrives has been called.
     Return an error.                                                                                                       */
  if ( DriveTable == NULL )
  {

    *Error = DISKIO_DRIVES_NOT_OPEN;
    return;

  }

  /* Is the Drive_Number requested valid? */
  if ( ( Drive_Number == 0 ) || ( Drive_Number > DriveCount ) )
  {

    /* Drive_Number is invalid!  Return an error. */
    *Error = DISKIO_REQUEST_OUT_OF_RANGE;

    /* Zero out the Geometry. */
    Geometry->Cylinders = 0;
    Geometry->Heads = 0;
    Geometry->Sectors = 0;

    /* Set *Is_PRM to FALSE. */
    *Is_PRM = FALSE;

    /* Set Cylinder_Limit_Applies to TRUE. */
    *Cylinder_Limit_Applies = TRUE;

    /* Return to caller. */
    return;

  }

  /* copy the geometry data for the drive from its entry in the DriveTable to *Geometry. */
  Geometry->Cylinders = DriveTable[Drive_Number - 1].Cylinders;
  Geometry->Heads = DriveTable[Drive_Number -1 ].Heads;
  Geometry->Sectors = DriveTable[Drive_Number - 1].SectorsPerTrack;
  *Is_PRM = DriveTable[Drive_Number - 1 ].Is_PRM;
  *Cylinder_Limit_Applies = DriveTable[Drive_Number - 1].Cylinder_Limit_Applies;

  /* Indicate success and return to caller. */
  *Error = DISKIO_NO_ERROR;

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GetBootDrive                                     */
/*                                                                   */
/*   Descriptive Name: This function returns the drive letter of the */
/*                     drive that the system booted off of.          */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: The drive letter that the system booted from.           */
/*                                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects: None.                                             */
/*                                                                   */
/*   Notes:  This function uses the DosQuerySysInfo API.             */
/*                                                                   */
/*********************************************************************/
char GetBootDrive(void)
{

  CARDINAL32    Encoded_Drive_Letter;
  APIRET        ReturnCode;

  /* Ask the operating system which drive it booted from. */
  ReturnCode = DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE, &Encoded_Drive_Letter, sizeof(CARDINAL32) );

  /* Did we succeed? */
  if ( ReturnCode != NO_ERROR )
  {

    /* Unexpected error!  Return NULL. */
    return 0x0;

  }

  /* Convert the drive letter and return it. */

  return (char) ( (Encoded_Drive_Letter - 1) + 'A' );

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: GetDirectoryList                                 */
/*                                                                   */
/*   Descriptive Name: This function returns a DLIST containing      */
/*                     one entry for each file/directory which       */
/*                     matched the FileSpecification passed in.      */
/*                     Each entry in the DLIST is the NULL terminated*/
/*                     name of a file which matched the              */
/*                     FileSpecification provided by the caller.     */
/*                                                                   */
/*   Input: char * DirectorySpecification - The fully qualified name */
/*                                          of the directory to      */
/*                                          search.                  */
/*          char * FileMask - A search mask, which may include the   */
/*                            standard * and ? wildcards, to control */
/*                            which items in the specified directory */
/*                            are selected and returned.             */
/*          CARDINAL32 * Error_Code : The address of a variable to   */
/*                                    hold the error return code.    */
/*                                                                   */
/*   Output: If successful, then the function return value will be   */
/*           non-NULL and *Error_Code will be set to DISKIO_NO_ERROR.*/
/*           Each entry in the DLIST returned by this function will  */
/*           be the NULL terminated name of a file which matched the */
/*           FileSpecification.  The size of each entry will be the  */
/*           length of the filename in that entry plus 1.  The TAG   */
/*           value for each entry will be FILENAME_TAG, defined      */
/*           previously.                                             */
/*                                                                   */
/*           If unsuccessful, then NULL is returned and *Error_Code  */
/*           will be set to a non-zero error code.                   */
/*                                                                   */
/*   Error Handling: If an error occurs, then all memory allocated by*/
/*                   this function is freed, NULL is returned, and   */
/*                   *Error_Code will be non-zero.                   */
/*                                                                   */
/*   Side Effects: Memory is allocated for the DLIST being returned. */
/*                                                                   */
/*   Notes:  This function uses the DosFindFirst, DosFindNext, and   */
/*           DosFindClose APIs.                                      */
/*                                                                   */
/*********************************************************************/
DLIST GetDirectoryList(char * DirectorySpecification, char * FileMask, CARDINAL32 * Error_Code)
{

#define DIRECTORY_ENTRIES_PER_QUERY 4

  ADDRESS        Buffer;      /* Used to hold the data returned by DosFindFirst and DosFindNext. */
  CARDINAL32     FilesFound;  /* Used to hold the number of files whose information is being returned in Buffer. */
  CARDINAL32     Handle;      /* The handle created by DosFindFirst and used by DosFindNext. */
  DLIST          FileList;    /* The list of filenames that we are going to build. */
  APIRET         ReturnCode;  /* Used to hold the error return code from DosFindFirst, DosFindNext, and DosFindClose. */
  FILEFINDBUF3 * FileData;    /* Used to extract data from Buffer after a call to DosFindFirst or DosFindNext. */
  CARDINAL32     Error;       /* Used to hold error codes from functions called on an error path. */
  char           FileSpecification[256];  /* Used to hold the fully qualified search specification. */
  CARDINAL32     Length;      /* Used when computing string lengths. */

  /* Allocate memory for the buffer. */
  Buffer = malloc( DIRECTORY_ENTRIES_PER_QUERY * sizeof(FILEFINDBUF3) );
  if ( Buffer == NULL )
  {

    /* Since we could not allocate the buffer, we must be out of memory. */
    *Error_Code = DISKIO_OUT_OF_MEMORY;
    return (DLIST) NULL;

  }

  /* Create the FileList. */
  FileList = CreateList();

  /* Did we succeed? */
  if (FileList == NULL)
  {

    /* Since we could not create the FileList, we must be out of memory. */
    *Error_Code = DISKIO_OUT_OF_MEMORY;
    free(Buffer);
    return (DLIST) NULL;

  }

  if ( Logging_Enabled )
  {

    sprintf(Log_Buffer,"GetDirectoryList is about to search the path %s", DirectorySpecification);
    Write_Log_Buffer();

  }

  /* Build the FileSpecification. */
  FileSpecification[0] = 0x0;

  /* Did the user specify a directory? */
  Length = strlen(DirectorySpecification);
  if ( Length > 0 )
  {
    strcat(FileSpecification, DirectorySpecification);

    /* Does the FileSpecification end in a '\' character? */
    if ( FileSpecification[Length - 1] != '\\' )
      strcat(FileSpecification,"\\");

  }

  strcat(FileSpecification, FileMask);

  /* Lets start our search. */
  Handle = HDIR_CREATE;                       /* Cause DosFindFirst to create a search handle. */
  FilesFound = DIRECTORY_ENTRIES_PER_QUERY;   /* Ask for the maximum number of entries which will fit in the buffer. */
  ReturnCode = DosFindFirst( FileSpecification,
                             &Handle,
                             FILE_NORMAL,
                             Buffer,
                             DIRECTORY_ENTRIES_PER_QUERY * sizeof(FILEFINDBUF3),
                             &FilesFound,
                             FIL_STANDARD);

  while ( ( (ReturnCode == NO_ERROR) ||
            (ReturnCode == ERROR_NO_MORE_FILES)
          ) &&
          (FilesFound > 0) )
  {

    /* Move the entries found from the buffer to the FileList. */
    FileData = (FILEFINDBUF3 *) Buffer;

    while ( FilesFound > 0 )
    {

      if ( Logging_Enabled )
      {

        sprintf(Log_Buffer,"Adding %s to the FileList.",FileData->achName);
        Write_Log_Buffer();

      }

      /* Add the filename to the FileList. */

      /* Build the FileSpecification. */
      FileSpecification[0] = 0x0;

      /* Did the user specify a directory? */
      Length = strlen(DirectorySpecification);
      if ( Length > 0 )
      {
        strcat(FileSpecification, DirectorySpecification);

        /* Does the FileSpecification end in a '\' character? */
        if ( FileSpecification[Length - 1] != '\\' )
          strcat(FileSpecification,"\\");

      }

      /* Add the name of the file we found to the path to produce the fully qualified pathname of the file. */
      strcat(FileSpecification, FileData->achName);

      /* Add the filename to the FileList being returned to the caller. */
      InsertItem( FileList, strlen(FileSpecification) + 1, FileSpecification, FILENAME_TAG, NULL, AppendToList, FALSE, Error_Code);

      /* Did we succeed? */
      if ( *Error_Code != DLIST_SUCCESS )
      {

        /* We could not add the item to the list.  Are we out of memory? */
        if ( *Error_Code == DLIST_OUT_OF_MEMORY )
          *Error_Code = DISKIO_OUT_OF_MEMORY;
        else
          *Error_Code = DISKIO_INTERNAL_ERROR;

        /* Free memory. */
        free(Buffer);
        DestroyList(&FileList,TRUE,&Error);

        /* Free the search handle. */
        ReturnCode = DosFindClose(Handle);

        /* Return NULL. */
        return (DLIST) NULL;

      }

      /* Move to the next entry. */
      FileData = (FILEFINDBUF3 *) ( (CARDINAL32) FileData + FileData->oNextEntryOffset);

      /* Adjust the counter. */
      FilesFound--;

    }

    /* Get the next group of matching files. */
    FilesFound = DIRECTORY_ENTRIES_PER_QUERY;
    ReturnCode = DosFindNext(Handle,
                             Buffer,
                             DIRECTORY_ENTRIES_PER_QUERY * sizeof(FILEFINDBUF3),
                             &FilesFound);

  }

  /* Is ReturnCode set to ERROR_NO_MORE_FILES as we expect?  If not, we have a problem. */
  if ( ( ReturnCode != ERROR_NO_MORE_FILES ) &&
       ( ReturnCode != NO_ERROR )
     )
  {  APIRET rc;

    /* Something unexpected happened!  Abort. */

    /* Clean up!  Free the memory. */
    free(Buffer);
    DestroyList(&FileList,TRUE, Error_Code);

    /* Free the search handle. */
    rc = DosFindClose(Handle);

    /* Indicate the error. */
    if(ReturnCode == ERROR_PATH_NOT_FOUND || ReturnCode == ERROR_FILE_NOT_FOUND)
    {  *Error_Code = DISKIO_NOFILES_FOUND;

    } else
      *Error_Code = DISKIO_UNEXPECTED_OS_ERROR;

    return (DLIST) NULL;

  }

  /* Free the buffer as we don't need it anymore. */
  free(Buffer);

  /* Free the handle as we don't need than anymore either. */
  ReturnCode = DosFindClose(Handle);

  /* Indicate success. */
  *Error_Code = DISKIO_NO_ERROR;

  return FileList;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: ReadSectors                                      */
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
void ReadSectors ( CARDINAL32   Drive_Number,
                   LBA          Starting_Sector,
                   CARDINAL32   Sectors_To_Read,
                   ADDRESS      Buffer,
                   CARDINAL32 * Error)
{

  /* Do_IO is our common routine for reading or writing.  Call it here and indicate that we want to Read, not write. */
  Do_IO( Drive_Number, Starting_Sector, Sectors_To_Read, Buffer, FALSE, Error);

  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: WriteSectors                                     */
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
/*              *Error will be 0.                                    */
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
void WriteSectors ( CARDINAL32   Drive_Number,
                    LBA          Starting_Sector,
                    CARDINAL32   Sectors_To_Write,
                    ADDRESS      Buffer,
                    CARDINAL32 * Error)
{

  /* Do_IO is our common routine for reading or writing.  Call it here and indicate that we want to Write, not read. */
  Do_IO( Drive_Number, Starting_Sector, Sectors_To_Write, Buffer, TRUE, Error);

  return;


}


int LastErrorIOCTL = 0;
int LastError      = 0;

/*--------------------------------------------------
 * Private Functions Available
 --------------------------------------------------*/



/*********************************************************************/
/*                                                                   */
/*   Function Name: Do_IO                                            */
/*                                                                   */
/*   Descriptive Name: This function reads or writes one or more     */
/*                     sectors from the specified drive.  For reads, */
/*                     the data read is placed in Buffer.  For       */
/*                     writes, the data is taken from Buffer and     */
/*                     written to disk.                              */
/*                                                                   */
/*   Input: CARDINAL32 Drive_Number : The number of the hard drive to*/
/*                                    use.  The drives in the        */
/*                                    system are numbered from 1 to  */
/*                                    n, where n is the total number */
/*                                    of hard drives in the system.  */
/*          LBA Starting_Sector : The first sector to read from/write*/
/*                                to.                                */
/*          CARDINAL32 Sectors_To_Read : The number of sectors to    */
/*                                       read/write.                 */
/*          ADDRESS Buffer : The location to get/put the data.       */
/*          BOOLEAN Write : If TRUE, Do_IO will perform a write.  If */
/*                          FALSE, then Do_IO will perform a read.   */
/*          CARDINAL32 * Error : The address of a variable to hold   */
/*                               the error return code.              */
/*                                                                   */
/*   Output: If Successful, *Error will be 0.                        */
/*           If Unsuccessful, then *Error will be > 0.               */
/*                                                                   */
/*   Error Handling: *Error will be > 0 if an error occurs.          */
/*                                                                   */
/*   Side Effects: Data may be read into memory starting at Buffer,  */
/*                 or data may be written from Buffer to disk.       */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
static void Do_IO ( CARDINAL32   Drive_Number,
                    LBA          Starting_Sector,
                    CARDINAL32   SectorCount,
                    ADDRESS      Buffer,
                    BOOLEAN      Write,
                    CARDINAL32 * Error)
{

  APIRET         ReturnCode;                     /* Used to hold the return code from OS/2 API calls. */

  CARDINAL32     DriveIndex;                     /* Used to convert the Drive_Number in an index into the DriveTable. */

  CARDINAL32     ParameterSize;                  /* Used by DosDevIOCtl. */
  CARDINAL32     DataSize;                       /* Used by DosDevIOCtl. */
  CARDINAL32     FunctionNumber;                 /* Used by DosDevIOCtl. */

  CARDINAL32     StartingCylinder;               /* Used to convert an LBA address into a CHS address for use with the CAT 9 Read IOCTL. */
  CARDINAL32     StartingHead;                   /* Used to convert an LBA address into a CHS address for use with the CAT 9 Read IOCTL. */
  CARDINAL32     StartingSector;                 /* Used to convert an LBA address into a CHS address for use with the CAT 9 Read IOCTL. */
  CARDINAL32     SectorsToReadOrWrite;           /* Used to handle the case where an I/O request crosses a track boundary. */
  ADDRESS        CurrentBufferLocation;          /* Used to handle the case where an I/O request crosses a track boundary. */

//DEBUG EK  
//FILE *fp;
//int i,j;
//if(Drive_Number == 2)
//{  if(Write) fp = fopen("aW","a");
//   else      fp = fopen("aR","a");
//}
  
/* If DriveTable is NULL, then we have not been initialized yet by a call to OpenDrives, or CloseDrives has been called.
     Return an error.                                                                                                       */
  if ( DriveTable == NULL )
  {

    *Error = DISKIO_DRIVES_NOT_OPEN;
    return;

  }

  /* Is the Drive_Number requested valid? */
  if ( ( Drive_Number > DriveCount ) || ( Drive_Number == 0 ) )
  {

    /* Drive_Number is invalid!  Return an error. */
    *Error = DISKIO_REQUEST_OUT_OF_RANGE;

    /* Return to caller. */
    return;

  }

#ifdef DEBUG

#ifdef PARANOID

  assert(DriveTable[Drive_Number - 1].Initialized);

#endif

#endif

  /* If SectorCount is 0, we have nothing to do! */
  if ( SectorCount == 0 )
  {

    /* That was an easy request!  */
    *Error = DISKIO_NO_ERROR;
    return;

  }

  /* What function are we performing? */
  if ( Write )
  {

    /* Save the IOCTL function number for a write operation. */
    FunctionNumber = PDSK_WRITEPHYSTRACK;

  }
  else
  {

    /* Save the IOCTL function number for a read operation. */
    FunctionNumber = PDSK_READPHYSTRACK;

  }

  /* Convert Drive_Number into an index into the DriveTable. */
  DriveIndex = Drive_Number - 1;

  /* CHS addresses can be a little strange.  The cylinder and head parts of a CHS address are 0 based, while the sector
     part of a CHS address is 1 based.  Thus, for a drive with 619 cylinders, 128 heads, and 63 sectors per track, the
     cylinder portion of a CHS address can range from 0 to 618, the head portion can range from 0 to 127, and the
     sector portion can range from 1 to 63.

     CHS addresses are converted into LBA addresses by the following formula:

         LBA = (Sector - 1) + (Head * Sectors Per Track) + (Cylinder * Heads Per Cylinder * Sectors Per Track)

     To convert from LBA to CHS, use the following formulas:

         Sector = ( LBA MOD Sectors Per Track ) + 1
         Head = ( LBA DIV Sectors Per Track ) MOD Heads Per Cylinder
         Cylinder = ( LBA DIV Sectors Per Track ) DIV Heads Per Cylinder

     In the above formulas, DIV represents interger division (throw away the remainder) and MOD represents modulo
     division (keep the remainder, throw away everything else).                                                            */

  /* Translate the Starting_Sector LBA address into a CHS address.  */
  StartingSector = ( Starting_Sector % ( CARDINAL32 ) DriveTable[DriveIndex].SectorsPerTrack ) + 1;
  StartingHead = (Starting_Sector / (CARDINAL32) DriveTable[DriveIndex].SectorsPerTrack);
  StartingCylinder = StartingHead / (CARDINAL32) DriveTable[DriveIndex].Heads;
  StartingHead = StartingHead % (CARDINAL32) DriveTable[DriveIndex].Heads;

  /* We now have a CHS address.  However, the IOCTL calls to do I/O require StartingSector to be an index into a Track
     Layout Table.  The Track Layout Table is 0 based instead of being 1 based, so we must adjust StartingSector to
     make it an index into the Track Layout Table.  This is easily done by subtracting 1.  Of course, we could have done
     this in our initial calculation of StartingSector above, but I think its easier to understand this code if it is
     kept as a separate step.                                                                                              */
  StartingSector--;

  /* Now that we have our starting point on the disk, we are almost ready to do the I/O.  We need to calculate how many
     sectors to read or write in this I/O, as the IOCTL will not read or write across a track boundary.                                      */
  if ( ( StartingSector + SectorCount ) <= (CARDINAL32) DriveTable[DriveIndex].SectorsPerTrack )
  {

    /* We can do the entire request in a single operation! */
    SectorsToReadOrWrite = SectorCount;

  }
  else
  {

    /* The request will have to be broken up.  Calculate how many sectors can be read or written in our first I/O. */
    SectorsToReadOrWrite = (CARDINAL32) DriveTable[DriveIndex].SectorsPerTrack - StartingSector;

  }

  /* Set our location to read from/write to. */
  CurrentBufferLocation = Buffer;

  /* Get the drive lock for this drive. */
  Lock(DriveTable[DriveIndex].Drive_Lock);

  /* Now lets do some I/O! */
  do
  {

    /* Set up the TrackLayout for the request. */
    DriveTable[DriveIndex].TrackLayout->usHead = StartingHead;
    DriveTable[DriveIndex].TrackLayout->usCylinder = StartingCylinder;
    DriveTable[DriveIndex].TrackLayout->usFirstSector = StartingSector;
    DriveTable[DriveIndex].TrackLayout->cSectors = SectorsToReadOrWrite;

    /* Set up the parameters required by DosDevIOCtl. */
    ParameterSize = DriveTable[DriveIndex].TrackLayoutSize;
    DataSize = SectorsToReadOrWrite * BYTES_PER_SECTOR;
//DEBUG EK  
#if 0
if(Drive_Number == 2)
{  fprintf(fp,"SH%i SC%i SS%i nS%i\n", 
         StartingHead,StartingCylinder,StartingSector,SectorsToReadOrWrite);
   if(Write)
   {  char *ptr, ch;
      int l;
      ptr = (char *) CurrentBufferLocation;
      for(i=l=0; i<DataSize; i+=16)
      {
        for(j=0; j<16; j++)
        { ch = *ptr;
          ptr++;
          fprintf(fp,"%02x ",ch);
          l++;
          if(l >DataSize) break;
        }
        fprintf(fp,"\n");
        if(l >DataSize) break;
      }
   }
}
#endif //0

    /* Now do the I/O. */
    ReturnCode = DosDevIOCtl( DriveTable[DriveIndex].DriveHandle,
                              IOCTL_PHYSICALDISK,
                              FunctionNumber,
                              DriveTable[DriveIndex].TrackLayout,
                              ParameterSize,
                              &ParameterSize,
                              CurrentBufferLocation,
                              DataSize,
                              &DataSize);
   LastErrorIOCTL = ReturnCode; //EK
//DEBUG EK  
#if 0
if(Drive_Number == 2)   
{
 fprintf(fp, "rc=%i ds=%i\n",ReturnCode,DataSize);      
}

if(Drive_Number == 2 && ( ReturnCode == NO_ERROR ))
{
   if(!Write)
   {  char *ptr, ch;
      int l;
      ptr = (char *) CurrentBufferLocation;

      for(i=l=0; i<DataSize; i+=16)
      {
        for(j=0; j<16; j++)
        { ch = *ptr;
          ptr++;
          fprintf(fp,"%02x ",ch);
          l++;
          if(l >DataSize) break;
        }
        fprintf(fp,"\n");
        if(l >DataSize) break;
      }
   }
}
#endif //0
    /* Update our counters. */
    SectorCount -= SectorsToReadOrWrite;

    /* Do we need to read or write more sectors? */
    if ( ( SectorCount > 0 ) && ( ReturnCode == NO_ERROR ) )
    {

      /* We need to read or write some more!  Update our starting location to the first sector on the next track. */
      StartingSector = 0;  /* Remember, this is an index into the TrackTable.  Index 0 corresponds to sector 1. */
      StartingHead++;

      /* Do we need to switch cylinders? */
      if ( StartingHead >= DriveTable[DriveIndex].Heads )
      {

        /* Switch to the first head on the next cylinder. */
        StartingHead = 0;
        StartingCylinder++;

        /* Are we off the end of the drive? */
        if ( StartingCylinder >= DriveTable[DriveIndex].Cylinders )
        {

          /* We are trying to read or write past the end of the disk! */

          /* Indicate the error! */
          *Error = DISKIO_REQUEST_OUT_OF_RANGE;

          /* Release the drive lock. */
          Unlock(DriveTable[DriveIndex].Drive_Lock);

          /* Abort the operation and return to caller. */
          return;

        }

      }

      /* We must adjust where the data is going to/coming from in memory. */
      CurrentBufferLocation = ( ADDRESS ) ( ( CARDINAL32 ) CurrentBufferLocation + ( SectorsToReadOrWrite * BYTES_PER_SECTOR ) );

      /* Calculate the number of sectors to read or write on our next I/O request. */
      if ( SectorCount > DriveTable[DriveIndex].SectorsPerTrack )
      {

        /* The number of sectors left to read or write is greater than the number of sectors on a track.  Since the most sectors
           we can read or write in a single I/O request is the number of sectors on a track, we will have to settle for that.     */
        SectorsToReadOrWrite = DriveTable[DriveIndex].SectorsPerTrack;

      }
      else
      {

        /* The number of sectors left to read or write is less than the number of sectors on a track.  We can read or write
           all of the remaining sectors in a single I/O operation.                                                                */
        SectorsToReadOrWrite = SectorCount;

      }

      /* We are now ready for our next I/O request. */

    }


  } while ( ( SectorCount > 0 ) && ( ReturnCode == NO_ERROR ) );

  /* All done. */

  /* Release the drive lock. */
  Unlock(DriveTable[DriveIndex].Drive_Lock);

  /* Was there an error? */
  if ( ReturnCode != NO_ERROR )
  {

    *Error = DISKIO_READ_FAILED;

  }
  else
  {

     *Error = DISKIO_NO_ERROR;

  }

//DEBUG EK  
//if(Drive_Number == 2)
//  fclose(fp);

  LastError = *Error;

  /* Return to caller. */
  return;

}


/*********************************************************************/
/*                                                                   */
/*   Function Name: Issue_Ring0_Feature_Command                      */
/*                                                                   */
/*   Descriptive Name: Issues a feature specific command to the      */
/*                     Ring 0 portion of a feature.                  */
/*                                                                   */
/*   Input: CARDINAL32 Feature_ID - The numeric ID assigned to the   */
/*                                  feature which is to receive the  */
/*                                  command being issued.            */
/*          char Drive_Letter - The drive letter of the volume whose */
/*                              feature is to receive the command.   */
/*          ADDRESS InputBuffer - A buffer containing the command.   */
/*          CARDINAL32 * Error_Code - The address of a CARDINAL32 in */
/*                                   in which to store an error code */
/*                                   should an error occur.          */
/*                                                                   */
/*   Output: If successful, then *Error_Code will be set to          */
/*           DISKIO_NO_ERROR.  If unsuccessful, then *Error_Code     */
/*           will be set to a non-zero error code.                   */
/*                                                                   */
/*   Error Handling: If the specified feature does not exist, then   */
/*                   *Error_Code will contain a non-zero error code. */
/*                                                                   */
/*   Side Effects:  Side effects are feature dependent.              */
/*                                                                   */
/*   Notes:  This function does not do any parameter checking.       */
/*                                                                   */
/*********************************************************************/
void Issue_Ring0_Feature_Command( CARDINAL32   Feature_ID,
                                  char         Drive_Letter,
                                  ADDRESS      Buffer,
                                  CARDINAL32 * Error_Code )
{

  DDI_OS2LVM_param      LVM_Parameters;     /* Used to call the OS2LVM Generic Command IOCTL. */
  DDI_OS2LVM_data       LVM_Data;           /* Returned by the OS2LVM IOCTL. */
  CARDINAL32            Parameter_Size;     /* Used for the DosDevIOCtl API. */
  CARDINAL32            Data_Size;          /* Used for the DosDevIOCtl API. */
  APIRET                Return_Code;        /* Used for the DosDevIOCtl API. */

  /* Initialize the return values in case of IOCTL failure. */
  *Error_Code = DISKIO_NO_ERROR;

  /* Initialize the IOCTL Parameters. */
  memset(&LVM_Parameters, 0, sizeof(DDI_OS2LVM_param) );
  LVM_Parameters.Command = GENERIC_FEATURE_COMMAND;
  LVM_Parameters.DriveUnit = toupper( Drive_Letter ) - 'A';
  LVM_Parameters.Feature_ID = Feature_ID;


  /* Initialize the IOCTL data. */
  LVM_Data.ReturnData = 0;
  LVM_Data.UserBuffer = Buffer;

  /* Initialize the Parameter size and the Data size. */
  Parameter_Size = sizeof(DDI_OS2LVM_param);
  Data_Size = sizeof(DDI_OS2LVM_data);

  /* Do the IOCTL call. */
  Return_Code = DosDevIOCtl(-1,
                            IOCTL_LOGICAL_DRIVE,
                            LVM_IOCTL_FUNCTION,
                            &LVM_Parameters,
                            Parameter_Size,
                            &Parameter_Size,
                            &LVM_Data,
                            Data_Size,
                            &Data_Size);


  /* Was the IOCTL was successful? */
  if ( Return_Code == NO_ERROR )
  {

    /* Since the IOCTL succeeded, set Error_Code accordingly. */
    *Error_Code = DISKIO_NO_ERROR;

  }
  else
  {

    *Error_Code = DISKIO_UNEXPECTED_OS_ERROR;

  }

  return;    /* Indicate failure. */

}


