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
 * Module: Engine.h
 */

/*
 * Change History:
 *
 */


/*
 * Functions: The following two functions are actually declared in
 *            LVM_INTERFACE.H but are implemented in ENGINE.C:
 *
 *            void Open_LVM_Engine
 *            void Close_LVM_Engine
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
 *    BootManager.C          Logging.c                       Handle_Manager.C
 *
 * Notes: LVM Drive Letter Assignment Tables (DLA_Tables) appear on the
 *        last sector of each track containing a valid MBR or EBR.  Since
 *        partitions must be track aligned, any track containing an MBR or
 *        EBR will be almost all empty sectors.  We will grab the last
 *        of these empty sectors for our DLT_Tables.
 *
 */

#ifndef LVM_ENGINE_DATA

#define LVM_ENGINE_DATA  1

#define INCL_DOSMODULEMGR
#include "os2.h"           /* DosLoadModule and related calls. */

#include "gbltypes.h"      /* CARDINAL32, CARDINAL16, ADDRESS, BOOLEAN */

#include "lvm_types.h"     /* LBA, QuadWord, DoubleWord */

#include "lvm_stru.h"      /* Drive_Geometry_Record */

#include "lvm_data.h"      /* Partition_Record, DLA_Entry */

#include "lvm_plug.h"

#include "dlist.h"         /* DLIST */

/*--------------------------------------------------
 * Type definitions
 --------------------------------------------------*/

/* The following structure is used to track LVM Plug-in modules. */
typedef struct _LVM_Plugin_Data_Record {
                                         HMODULE      Plugin_Handle;
                                         ADDRESS      Function_Table;
                                       } LVM_Plugin_Data_Record;

#define LVM_PLUGIN_DATA_RECORD_TAG  354685989
#define BBR_PLUGIN_HANDLE           0xFFFFFFFF
#define PASS_THRU_PLUGIN_HANDLE     0xFFFFFFFE

/* The following structure is used (with the Find_Feature_Given_ID function below) to find a specific
   feature given the Feature_ID of the feature.                                                       */
typedef struct _Find_Feature_Data_Record {
                                           CARDINAL32        Feature_ID;     /* Input */
                                           ADDRESS           Function_Table; /* Output.  NULL if no matching feature was found. */
                                         } Find_Feature_Data_Record;

/* The following structure is used to keep track of how features are applied to a volume/partition. */
typedef struct _Feature_Application_Data {
                                           Plugin_Function_Table_V1 * Function_Table;           /* The function table for the feature. */
                                           CARDINAL32                 Feature_Sequence_Number;  /* Used to indicate the order in which features should be applied. */
                                           ADDRESS                    Init_Data;                /* A pointer to a buffer containing initialization data for the feature.  NULL if no initialization data. */
                                           LVM_Classes                Actual_Class;             /* Which LVM Class supported by the feature is to be used. */
                                           BOOLEAN                    Top_Of_Class;             /* Set to TRUE if this feature is the topmost feature in its class on the volume being created. */
                                         } Feature_Application_Data;

#define FEATURE_APPLICATION_DATA_TAG 384731372


typedef struct {
                 CARDINAL32   Feature_ID;
                 BOOLEAN      Found;
               } Feature_Search_Data;



/*--------------------------------------------------
 * Utility functions for use within the LVM Engine.
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
void _System Find_Feature_Given_ID(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


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
CARDINAL32 _System Create_Serial_Number(void);


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
void _System Duplicate_Check( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


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
char * _System Adjust_Name( char * Name );


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
BOOLEAN _System Valid_Signature_Sector(Partition_Data * PartitionRecord, LVM_Signature_Sector * Signature_Sector);


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
void _System Load_Feature_Information_Array(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


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
void _System Load_Feature_Information_Array2(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


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
INTEGER32 _System Sort_By_Feature_Sequence_Number( ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag, CARDINAL32 * Error);


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
void _System Find_Existing_Feature(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);


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
CARDINAL32 _System Compute_Reported_Volume_Size( CARDINAL32 Volume_Size );


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
void  _System Create_Fake_EBR( Extended_Boot_Record ** New_EBR, CARDINAL32 * New_EBR_Size, LVM_Signature_Sector * Signature_Sector, CARDINAL32 * Error_Code);


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
void  _System Create_Fake_Partition_Table_Entry( Partition_Record * Partition_Table_Entry, CARDINAL32 Partition_Size);


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
BOOLEAN _System Create_Unique_Name( CARDINAL32  Name_Lists_To_Use, BOOLEAN Add_Brackets, char * BaseName, CARDINAL32 * Initial_Count, char * Buffer, CARDINAL32 BufferSize);

#define VOLUME_NAMES     1
#define DISK_NAMES       2
#define PARTITION_NAMES  4


/*--------------------------------------------------
 * Global Data
 --------------------------------------------------*/

#ifdef DECLARE_GLOBALS

DLIST                      Available_Features = (DLIST) NULL;         /* Used to keep a list of the LVM Features available. */
DLIST                      Filesystem_Expansion_DLLs = (DLIST) NULL;  /* Used to keep a list of the filesystem expansion DLLs required by the volumes which exist within the system. */
DLIST                      Volumes = (DLIST) NULL;                    /* Used to keep track of the volumes in the system. */
DLIST                      Aggregates = (DLIST) NULL;                 /* Used to keep track of the aggregates created by plug-ins. */
DLIST                      KillSector = (DLIST) NULL;                 /* Used to keep track of sectors that need to be "killed" when partitions are created or deleted. */
Disk_Drive_Data   *        DriveArray = NULL;                         /* Used to keep track of the drives in the system.  */
CARDINAL32                 DriveCount = 0;                            /* The number of partitionable disk drives in the system. */
CARDINAL32                 Min_Install_Size = 0;                      /* A partition must be at least this big before it can be marked installable. */
BOOLEAN                    RebootRequired = FALSE;                    /* If TRUE, then a reboot is required for any changes made to take effect. */
ADDRESS                    Install_Volume_Handle = NULL;              /* The handle ( in the Volumes list) of the Volume marked installable. */
CARDINAL32                 Boot_Drive_Serial_Number = 0;              /* The serial number of the drive the system booted from, or, if an active copy of Boot Manager is present,
                                                                         then this becomes the serial number of the drive containing the active copy of Boot Manager.             */
ADDRESS                    Boot_Manager_Handle = NULL;                /* Set to the External_Handle of the Partition containing Boot Manager. */
BOOLEAN                    Boot_Manager_Active = FALSE;               /* Set to TRUE if an active copy of Boot Manager is found. */
CARDINAL32                 Min_Free_Space_Size = 2048;                /* A block of free space must be at least this big in order for the engine to report it. */
BOOLEAN                    Merlin_Mode = FALSE;                       /* Used to track whether or not we are running under a release of OS/2 prior to Aurora (i.e. Merlin). */
CARDINAL32                 Reserved_Drive_Letters = 0;                /* Used to track the drive letters assigned to non-lvm devices. */
ADDRESS                    Common_Services;                           /* Used to hold the list of services provided by the LVM Engine to LVM Plug-in modules. */
ADDRESS                    PassThru_Function_Table = NULL;            /* Used to point to the function table for the Pass Thru layer.  Pass Thru is built in to LVM.DLL, but is treated as a plug-in module. */
ADDRESS                    BBR_Function_Table = NULL;                 /* Used to point to the function table for the BBR layer.  BBR is built in to LVM.DLL, but is treated as a plug-in module. */
LVM_Interface_Types        Interface_In_Use;                          /* Used to identify the type of interface being used by the caller. */

/* Create a variable to hold the Java call back function for use in executing a Java class
   during the creation of a volume while running under a Java user interface.                    */
void                       (* _System Java_Call_Back) ( char *       Class_Name,
                                                        ADDRESS      InputBuffer,
                                                        CARDINAL32   InputBufferSize,
                                                        ADDRESS    * OutputBuffer,
                                                        CARDINAL32 * OutputBufferSize,
                                                        CARDINAL32 * Error_Code);



#else

extern DLIST                      Available_Features;        /* Used to keep a list of the LVM Features available. */
extern DLIST                      Filesystem_Expansion_DLLs; /* Used to keep a list of the filesystem expansion DLLs required by the volumes which exist within the system. */
extern DLIST                      Volumes;                   /* Used to keep track of the volumes in the system. */
extern DLIST                      Aggregates;                /* Used to keep track of the aggregates created by plug-ins. */
extern DLIST                      KillSector;                /* Used to keep track of sectors that need to be "killed" when partitions are created or deleted. */
extern Disk_Drive_Data   *        DriveArray;                /* Used to keep track of the drives in the system.  */
extern CARDINAL32                 DriveCount;                /* The number of partitionable disk drives in the system. */
extern CARDINAL32                 Min_Install_Size;          /* A partition must be at least this big before it can be marked installable. */
extern BOOLEAN                    RebootRequired;            /* If TRUE, then a reboot is required for any changes made to take effect. */
extern ADDRESS                    Install_Volume_Handle;     /* The handle ( in the Volumes list) of the Volume marked installable. */
extern CARDINAL32                 Boot_Drive_Serial_Number;  /* The serial number of the drive the system booted from, or, if an active copy of Boot Manager is present,
                                                                then this becomes the serial number of the drive containing the active copy of Boot Manager.             */
extern ADDRESS                    Boot_Manager_Handle;       /* Set to the External_Handle of the Partition containing Boot Manager. */
extern BOOLEAN                    Boot_Manager_Active;       /* Set to TRUE if an active copy of Boot Manager is found. */
extern CARDINAL32                 Min_Free_Space_Size;       /* A block of free space must be at least this big in order for the engine to report it. */
extern BOOLEAN                    Merlin_Mode;               /* Used to track whether or not we are running under a release of OS/2 prior to Aurora (i.e. Merlin). */
extern CARDINAL32                 Reserved_Drive_Letters;    /* Used to track the drive letters assigned to non-lvm devices. */
extern ADDRESS                    Common_Services;           /* Used to hold the list of services provided by the LVM Engine to LVM Plug-in modules. */
extern ADDRESS                    PassThru_Function_Table;   /* Used to point to the function table for the Pass Thru layer.  Pass Thru is built in to LVM.DLL, but is treated as a plug-in module. */
extern ADDRESS                    BBR_Function_Table;        /* Used to point to the function table for the BBR layer.  BBR is built in to LVM.DLL, but is treated as a plug-in module. */
extern LVM_Interface_Types        Interface_In_Use;          /* Used to identify the type of interface being used by the caller. */
extern void                       (* _System Java_Call_Back) ( char *       Class_Name,
                                                               ADDRESS      InputBuffer,
                                                               CARDINAL32   InputBufferSize,
                                                               ADDRESS    * OutputBuffer,
                                                               CARDINAL32 * OutputBufferSize,
                                                               CARDINAL32 * Error_Code);




#endif


#endif
