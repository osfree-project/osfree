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
 * Module: diskio.h
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


#ifndef LBA_BASED_DISKIO

#define LBA_BASED_DISKIO 1

#include "lvm_gbls.h"        /* ADDRESS, BOOLEAN, CARDINAL16, CARDINAL32 */
#include "lvm_type.h"        /* LBA */
#include "lvm_stru.h"        /* Drive_Geometry_Record */
#include "dlist.h"           /* DLIST */

/*----------------------------------------------------------*/
/* Category 9, function 6AH, Rediscover Partitions          */
/*----------------------------------------------------------*/

typedef struct _DDI_Rediscover_param
{
  BYTE          DDI_TotalDrives;       /* # of drives to rediscover.  Use 0 for PRM only rediscover. */
  BYTE          DDI_aDriveNums[1];     /* array of drive number to rediscover */
} DDI_Rediscover_param, *PDDI_Rediscover_param;

typedef struct _DDI_ExtendRecord
{
  BYTE          DDI_Volume_UnitID;     /* Unit ID of volume being extended */
  CARDINAL32    DDI_Volume_SerialNumber; /* Serial # of volume being extended */
} DDI_ExtendRecord;

typedef struct _DDI_Rediscover_data
{
  BYTE             NewIFSMUnits;          /* # of new units reported to IFSM  */
  BYTE             DDI_TotalExtends;      /* # of volumes being extended */
  DDI_ExtendRecord DDI_aExtendRecords[1]; /* array of extendfs volume records */
} DDI_Rediscover_data, *PDDI_Rediscover_data;

/* The following define is used as the TAG value for entries in the DLIST returned by
   the GetDirectoryList function below.                                                 */
#define FILENAME_TAG  9871625

/* Defines for error codes used by this module. */
#define DISKIO_NO_ERROR              0
#define DISKIO_DRIVES_ALREADY_OPEN   1
#define DISKIO_DRIVES_NOT_OPEN       2
#define DISKIO_OUT_OF_MEMORY         3
#define DISKIO_REQUEST_OUT_OF_RANGE  4
#define DISKIO_READ_FAILED           5
#define DISKIO_WRITE_FAILED          6
#define DISKIO_NO_DRIVES_FOUND       7
#define DISKIO_NOT_ENOUGH_HANDLES    8
#define DISKIO_FAILED_TO_GET_HANDLE  9
#define DISKIO_BAD_GEOMETRY         10
#define DISKIO_UNEXPECTED_OS_ERROR  11
#define DISKIO_INTERNAL_ERROR       12
#define DISKIO_NOFILES_FOUND        13


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
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
BOOLEAN OpenDrives( CARDINAL32 * Error );


/*********************************************************************/
/*                                                                   */
/*   Function Name: CloseDrives                                      */
/*                                                                   */
/*   Descriptive Name: This function closes any and all drive handles*/
/*                     that this module may have, as well as freeing */
/*                     all memory allocated by this module.          */
/*                                                                   */
/*   Input: None.                                                    */
/*                                                                   */
/*   Output: None.                                                   */
/*                                                                   */
/*   Error Handling: N/A                                             */
/*                                                                   */
/*   Side Effects: All drive handles opened by OpenDrives are closed.*/
/*                 All memory allocated by OpenDrives is released.   */
/*                                                                   */
/*   Notes:  None.                                                   */
/*                                                                   */
/*********************************************************************/
void CloseDrives( void );


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
CARDINAL32 GetDriveCount( CARDINAL32 * Error );


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
                       CARDINAL32 *            Error );


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
char GetBootDrive(void);


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
DLIST GetDirectoryList(char * DirectorySpecification, char * FileMask, CARDINAL32 * Error_Code);


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
                   CARDINAL32 * Error);


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
                    CARDINAL32 * Error);


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
CARDINAL32 Rediscover( PDDI_Rediscover_param  Rediscovery_Parameters, PDDI_Rediscover_data Rediscovery_Data);


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
/*                                                                   */
/*********************************************************************/
void PRM_Rediscovery_Control( BOOLEAN  Enable );


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
                                  CARDINAL32 * Error_Code );


#endif
