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
 * Module: lvm_plug.h
 */ 

/*
 * Change History:
 * 
 */

/*
 * Description: This header file defines the structures passed to and from an
 *              LVM plug-in.
 *
 */

#ifndef LVM_ENGINE_PLUGIN_INTERFACE

#define LVM_ENGINE_PLUGIN_INTERFACE  1

#include "lvm_gbls.h"      /* CARDINAL32, CARDINAL16, ADDRESS, BOOLEAN */

#include "lvm_type.h"

#include "LVM_LIST.H"

#include "LVM_HAND.h"

#include "lvm_stru.h"      /* LVM Engine internal structures. */

#include <stdio.h>     /* sprintf */

/*--------------------------------------------------
 * Logging Macros
 --------------------------------------------------*/

#define API_ENTRY_BORDER   "*****API ENTRY*****\n"
#define API_EXIT_BORDER    "*****API EXIT******\n"
#define ERROR_BORDER       "\n!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!ERROR!!!!!!!\n"
#define FUNCTION_ENTRY_BORDER   "*****FUNCTION ENTRY*****\n"
#define FUNCTION_EXIT_BORDER    "*****FUNCTION EXIT******\n"
 

#define LOG_FEATURE_EVENT( Event_Text )  if ( LVM_Common_Services->Logging_Enabled )                         \
                                         {                                                                   \
                                                                                                             \
                                           sprintf(LVM_Common_Services->Log_Buffer, "     %s", Event_Text);  \
                                           LVM_Common_Services->Write_Log_Buffer();                          \
                                                                                                             \
                                         }                                                                   \


#define LOG_FEATURE_EVENT1( Event_Text, Event_Code1_Text, Event_Code1 )  if ( LVM_Common_Services->Logging_Enabled )       \
                                                                         {                                                 \
                                                                                                                           \
                                                                           sprintf(LVM_Common_Services->Log_Buffer,        \
                                                                                   "     %s\n       %s = %x",              \
                                                                                   Event_Text,                             \
                                                                                   Event_Code1_Text,                       \
                                                                                   (CARDINAL32) Event_Code1);              \
                                                                           LVM_Common_Services->Write_Log_Buffer();        \
                                                                                                                           \
                                                                         }                                                 \


#define LOG_FEATURE_EVENT2( Event_Text, Event_Code1_Text, Event_Code1, Event_Code2_Text, Event_Code2 )                       \
                                                                    if (  LVM_Common_Services->Logging_Enabled )             \
                                                                    {                                                        \
                                                                                                                             \
                                                                      sprintf(LVM_Common_Services->Log_Buffer,               \
                                                                              "     %s\n       %s = %x\n       %s = %x",     \
                                                                              Event_Text,                                    \
                                                                              Event_Code1_Text,                              \
                                                                              (CARDINAL32) Event_Code1,                      \
                                                                              Event_Code2_Text,                              \
                                                                              (CARDINAL32) Event_Code2);                     \
                                                                      LVM_Common_Services->Write_Log_Buffer();               \
                                                                                                                             \
                                                                    }                                                        \



#define LOG_FEATURE_EVENT3( Event_Text, Event_Code1_Text, Event_Code1, Event_Code2_Text, Event_Code2, Event_Code3_Text, Event_Code3 )       \
                                                                   if (  LVM_Common_Services->Logging_Enabled )                             \
                                                                   {                                                                        \
                                                                                                                                            \
                                                                     sprintf(LVM_Common_Services->Log_Buffer,                               \
                                                                             "     %s\n       %s = %x\n       %s = %x\n       %s = %x",     \
                                                                             Event_Text,                                                    \
                                                                             Event_Code1_Text,                                              \
                                                                             (CARDINAL32) Event_Code1,                                      \
                                                                             Event_Code2_Text,                                              \
                                                                             (CARDINAL32 ) Event_Code2,                                     \
                                                                             Event_Code3_Text,                                              \
                                                                             (CARDINAL32) Event_Code3);                                     \
                                                                     LVM_Common_Services->Write_Log_Buffer();                               \
                                                                                                                                            \
                                                                   }                                                                        \


#define LOG_FEATURE_ERROR( Error_Text )  if (  LVM_Common_Services->Logging_Enabled )                        \
                                         {                                                                   \
                                                                                                             \
                                           sprintf( LVM_Common_Services->Log_Buffer,ERROR_BORDER);           \
                                           LVM_Common_Services->Write_Log_Buffer();                          \
                                           sprintf( LVM_Common_Services->Log_Buffer, "     %s", Error_Text); \
                                           LVM_Common_Services->Write_Log_Buffer();                          \
                                           sprintf( LVM_Common_Services->Log_Buffer,ERROR_BORDER);           \
                                           LVM_Common_Services->Write_Log_Buffer();                          \
                                                                                                             \
                                         }                                                                   \

#define LOG_FEATURE_ERROR1( Error_Text, Error1_Text, Error_Code )  if ( LVM_Common_Services->Logging_Enabled )      \                                                                   
                                                                      {                                             \
                                                                                                                     \
                                                                     sprintf(LVM_Common_Services->Log_Buffer,        \
                                                                             "     %s\n       %s = %x",              \
                                                                             Error_Text,                             \
                                                                             Error1_Text,                            \
                                                                             (CARDINAL32) Error_Code);               \
                                                                     LVM_Common_Services->Write_Log_Buffer();        \
                                                                                                                     \
                                                                   }                                                 \

#define LOG_FEATURE_ERROR2( Error_Text, Error_Code1_Text, Error_Code1, Error_Code2_Text, Error_Code2 )               \
                                                            if (  LVM_Common_Services->Logging_Enabled )             \
                                                            {                                                        \
                                                                                                                     \
                                                              sprintf(LVM_Common_Services->Log_Buffer,               \
                                                                      "     %s\n       %s = %x\n       %s = %x",     \
                                                                      Error_Text,                                    \
                                                                      Error_Code1_Text,                              \
                                                                      (CARDINAL32) Error_Code1,                      \
                                                                      Error_Code2_Text,                              \
                                                                      (CARDINAL32) Error_Code2);                     \
                                                              LVM_Common_Services->Write_Log_Buffer();               \
                                                                                                                     \
                                                            }                                                        \

#define FEATURE_FUNCTION_ENTRY( FunctionName )  if (  LVM_Common_Services->Logging_Enabled > 1)                       \
                                                {                                                                     \
                                                                                                                      \
                                                  sprintf( LVM_Common_Services->Log_Buffer,FUNCTION_ENTRY_BORDER);    \
                                                  LVM_Common_Services->Write_Log_Buffer();                            \
                                                  sprintf( LVM_Common_Services->Log_Buffer, "     %s", FunctionName); \
                                                  LVM_Common_Services->Write_Log_Buffer();                            \
                                                  sprintf( LVM_Common_Services->Log_Buffer,FUNCTION_ENTRY_BORDER);    \
                                                  LVM_Common_Services->Write_Log_Buffer();                            \
                                                                                                                      \
                                                }                                                                     \

#define FEATURE_FUNCTION_EXIT( FunctionName )  if (  LVM_Common_Services->Logging_Enabled > 1)                      \
                                               {                                                                    \
                                                                                                                    \
                                                 sprintf(LVM_Common_Services->Log_Buffer,FUNCTION_EXIT_BORDER);     \
                                                 LVM_Common_Services->Write_Log_Buffer();                           \
                                                 sprintf(LVM_Common_Services->Log_Buffer, "     %s", FunctionName); \
                                                 LVM_Common_Services->Write_Log_Buffer();                           \
                                                 sprintf(LVM_Common_Services->Log_Buffer,FUNCTION_EXIT_BORDER);     \
                                                 LVM_Common_Services->Write_Log_Buffer();                           \
                                                                                                                    \
                                               }                                                                    \


/*--------------------------------------------------
 * Type definitions
 --------------------------------------------------*/

/* The following structure defines the functions which a plugin module must provide in order to be useable by LVM. */
typedef struct _Plugin_Function_Table_V1{
                                          Feature_ID_Data *               Feature_ID;
                                          void                            (* _System Open_Feature ) ( CARDINAL32 * Error_Code );
                                          void                            (* _System Close_Feature) ( void );
                                          BOOLEAN                         (* _System Can_Expand ) ( ADDRESS          Aggregate,                  /* Input - The aggregate to expand. */
                                                                                                    CARDINAL32 *     Feature_ID,                 /* Output - The ID of the feature which will perform the expansion. */
                                                                                                    CARDINAL32 *     Error_Code );
                                          void                            (* _System Add_Partition) ( ADDRESS Aggregate, DLIST New_Partitions, CARDINAL32 * Error_Code );
                                          void                            (* _System Delete ) ( ADDRESS Aggregate, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code );
                                          void                            (* _System Discover) ( DLIST Partition_List, CARDINAL32 * Error_Code );
                                          void                            (* _System Remove_Features) (ADDRESS Aggregate, CARDINAL32 * Error_Code);
                                          void                            (* _System Create) ( DLIST Partition_List,
                                                                                               ADDRESS VData,
                                                                                               ADDRESS Init_Data,
                                                                                               void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                                                                               LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                                                                               BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                                                                               CARDINAL32   Sequence_Number, /* Assigned by LVM during volume creation, used during Volume Expansion. */
                                                                                               CARDINAL32 * Error_Code );
                                          void                            (* _System Commit) ( ADDRESS VolumeRecord, ADDRESS PartitionRecord, CARDINAL32 * Error_Code );
                                          void                            (* _System Write ) ( ADDRESS PartitionRecord, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code);
                                          void                            (* _System Read ) ( ADDRESS PartitionRecord, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code);
                                          void                            (* _System ReturnCurrentClass) ( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number );
                                          void                            (* _System PassThru) ( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code );
                                          BOOLEAN                         (* _System ChangesPending)(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code);
                                          void                            (* _System ParseCommandLineArguments)(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code );
                                        } Plugin_Function_Table_V1;

/* The following defines the TAG value used to identify items of type Plugin_Function_Table_V1 in DLISTs. */
#define PLUGIN_FUNCTION_TABLE_V1_TAG  354385987

/* The following structure defines the services provided by the LVM Engine to plugin modules.  The memory management services
   must be used by any plugins since, if the plugin has its own memory allocated, bad things could happen if memory is allocated
   by the plugin and freed by the LVM Engine!                                                                                  
   */
typedef struct _LVM_Common_Services_V1 {
                                         /* Internal LVM Engine structures available to plugins. */
                                         DLIST               Volumes;                  /* List of all volumes found in the system. */
                                         DLIST               Aggregates;               /* List used to track aggregates created by plug-ins. */
                                         Disk_Drive_Data   * DriveArray;               /* List of all physical drives found in the system. */
                                         CARDINAL32          DriveCount;               /* The number of physical drives in the system.  This gives the number of entries in DriveArray. */
                                         DLIST               KillSector;               /* A list of the sectors which must be overwritten with 0xF6's.  See Kill_Sector_Data in LVM_STRU.H. */
                                         CARDINAL32          Initial_CRC;              /* Used with the Calculate_CRC function. */
                                         CARDINAL32        * Boot_Drive_Serial_Number; /* The serial number of the drive that the system booted off of. */
                                         char *              Log_Buffer;               /* A buffer used to hold text which will be written to the log file when Write_Log_Buffer is executed. */
                                         BOOLEAN             Merlin_Mode;              /* Used to track whether or not we are running under a release of OS/2 prior to Aurora (i.e. Merlin).  If TRUE, then we are not running on Aurora! */
                                         BOOLEAN             Logging_Enabled;          /* Set to TRUE if logging is currently enabled, FALSE otherwise. */

                                         /* Functions provided by the LVM Engine for use by Plugins. */

                                         /* Memory Management Functions */
                                         ADDRESS    (* _System Allocate)( CARDINAL32 Size);
                                         void       (* _System Deallocate)(ADDRESS Pointer);
                                         /* General use functions. */
                                         CARDINAL32 (* _System CalculateCRC)( CARDINAL32 CRC, ADDRESS Buffer, CARDINAL32 BufferSize);
                                         CARDINAL32 (* _System Create_Serial_Number)(void);
                                         char *     (* _System Adjust_Name)( char * Name );
                                         BOOLEAN    (* _System Valid_Signature_Sector)(Partition_Data * PartitionRecord, LVM_Signature_Sector * Signature_Sector);
                                         BOOLEAN    (* _System Create_Unique_Name)( CARDINAL32  Name_Lists_To_Use, BOOLEAN Add_Brackets, char * BaseName, CARDINAL32 * Initial_Count, char * Buffer, CARDINAL32 BufferSize);
                                         CARDINAL32 (* _System Compute_Reported_Volume_Size)( CARDINAL32 Volume_Size );
                                         void       (* _System Create_Fake_EBR)( Extended_Boot_Record ** New_EBR, CARDINAL32 * New_EBR_Size, LVM_Signature_Sector * Signature_Sector, CARDINAL32 * Error_Code);
                                         void       (* _System Create_Fake_Partition_Table_Entry)( Partition_Record * Partition_Table_Entry, CARDINAL32 Partition_Size);
/* Functions for converting between LBA and CHS, and CHS and Partition Table Format. */
void       (* _System Convert_To_CHS)(LBA Starting_Sector, CARDINAL32 Drive_Index, CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code );
void       (* _System Convert_CHS_To_Partition_Table_Format)( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code );
//void       (* _System Convert_Partition_Table_Format_To_CHS)( CARDINAL32 * Cylinder, CARDINAL32 * Head, CARDINAL32 * Sector, CARDINAL32 * Error_Code );
void       (* _System Convert_CHS_To_LBA)(CARDINAL32 Drive_Index, CARDINAL32 Cylinder, CARDINAL32 Head, CARDINAL32 Sector, LBA * Starting_Sector, CARDINAL32 * Error_Code);
                                         /* Logging Functions - Allows a plugin to write to the LVM Log File, or dump the current LVM configuration to the LVM Log File. */
                                         void       (* _System Log_Current_Configuration) ( void );
                                         void       (* _System Write_Log_Buffer)( void );
                                         /* Handle Functions - Allows a plugin to create, destroy, and translate EXTERNAL handles into their corresponding Volume, Partition, or Disk. */
                                         ADDRESS    (* _System Create_Handle)( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, CARDINAL32 * Error_Code );
                                         void       (* _System Destroy_Handle)( ADDRESS Handle, CARDINAL32 * Error_Code );
                                         void       (* _System Translate_Handle)( ADDRESS Handle, ADDRESS * Object, TAG * ObjectTag, CARDINAL32 * Error_Code );
                                         /* DLIST Functions - Allows DLISTs to be created, destroyed, and manipulated. */
                                         DLIST      (* _System CreateList)( void );
                                         ADDRESS    (* _System InsertItem) ( DLIST            ListToAddTo,
                                                                             CARDINAL32       ItemSize,
                                                                             ADDRESS          ItemLocation,
                                                                             TAG              ItemTag,
                                                                             ADDRESS          TargetHandle,
                                                                             Insertion_Modes  Insert_Mode,
                                                                             BOOLEAN          MakeCurrent,
                                                                             CARDINAL32 *     Error);
                                         ADDRESS (* _System InsertObject) ( DLIST           ListToAddTo,
                                                                            CARDINAL32      ItemSize,
                                                                            ADDRESS         ItemLocation,
                                                                            TAG             ItemTag,
                                                                            ADDRESS         TargetHandle,
                                                                            Insertion_Modes Insert_Mode,
                                                                            BOOLEAN         MakeCurrent,
                                                                            CARDINAL32 *    Error);
                                         void (* _System DeleteItem) (DLIST        ListToDeleteFrom,
                                                                      BOOLEAN      FreeMemory,
                                                                      ADDRESS      Handle,
                                                                      CARDINAL32 * Error);
                                         void (* _System DeleteAllItems) (DLIST        ListToDeleteFrom,
                                                                          BOOLEAN      FreeMemory,
                                                                          CARDINAL32 * Error);
                                         void (* _System GetItem) (DLIST          ListToGetItemFrom,
                                                                   CARDINAL32     ItemSize,
                                                                   ADDRESS        ItemLocation,
                                                                   TAG            ItemTag,
                                                                   ADDRESS        Handle,
                                                                   BOOLEAN        MakeCurrent,
                                                                   CARDINAL32 *   Error);
                                         void (* _System GetNextItem)( DLIST          ListToGetItemFrom,
                                                                       CARDINAL32     ItemSize,
                                                                       ADDRESS        ItemLocation,
                                                                       TAG            ItemTag,
                                                                       CARDINAL32 *   Error);
                                         void (* _System GetPreviousItem)( DLIST          ListToGetItemFrom,
                                                                           CARDINAL32     ItemSize,
                                                                           ADDRESS        ItemLocation,
                                                                           TAG            ItemTag,
                                                                           CARDINAL32 *   Error);
                                         ADDRESS (* _System GetObject)( DLIST          ListToGetItemFrom,
                                                                        CARDINAL32     ItemSize,
                                                                        TAG            ItemTag,
                                                                        ADDRESS        Handle,
                                                                        BOOLEAN        MakeCurrent,
                                                                        CARDINAL32 *   Error);
                                         ADDRESS (* _System GetNextObject)( DLIST          ListToGetItemFrom,
                                                                            CARDINAL32     ItemSize,
                                                                            TAG            ItemTag,
                                                                            CARDINAL32 *   Error);
                                         ADDRESS (* _System GetPreviousObject)( DLIST          ListToGetItemFrom,
                                                                                CARDINAL32     ItemSize,
                                                                                TAG            ItemTag,
                                                                                CARDINAL32 *   Error);
                                         void (* _System ExtractItem)( DLIST          ListToGetItemFrom,
                                                                       CARDINAL32     ItemSize,
                                                                       ADDRESS        ItemLocation,
                                                                       TAG            ItemTag,
                                                                       ADDRESS        Handle,
                                                                       CARDINAL32 *   Error);
                                         ADDRESS (* _System ExtractObject)( DLIST          ListToGetItemFrom,
                                                                            CARDINAL32     ItemSize,
                                                                            TAG            ItemTag,
                                                                            ADDRESS        Handle,
                                                                            CARDINAL32 *   Error);
                                         void (* _System ReplaceItem)( DLIST         ListToReplaceItemIn,
                                                                       CARDINAL32    ItemSize,
                                                                       ADDRESS       ItemLocation,
                                                                       TAG           ItemTag,
                                                                       ADDRESS       Handle,
                                                                       BOOLEAN       MakeCurrent,
                                                                       CARDINAL32 *  Error);
                                         ADDRESS (* _System ReplaceObject)( DLIST         ListToReplaceItemIn,
                                                                            CARDINAL32 *  ItemSize,             /* On input - size of new object.  On return = size of old object. */
                                                                            ADDRESS       ItemLocation,
                                                                            TAG        *  ItemTag,              /* On input - TAG of new object.  On return = TAG of old object. */
                                                                            ADDRESS       Handle,
                                                                            BOOLEAN       MakeCurrent,
                                                                            CARDINAL32 *  Error);
                                         TAG (* _System GetTag)( DLIST        ListToGetTagFrom,
                                                                 ADDRESS      Handle,
                                                                 CARDINAL32 * ItemSize,
                                                                 CARDINAL32 * Error);
                                         ADDRESS (* _System GetHandle) ( DLIST ListToGetHandleFrom,
                                                                         CARDINAL32 * Error);
                                         CARDINAL32 (* _System GetListSize)( DLIST ListToGetSizeOf,
                                                                             CARDINAL32 * Error);
                                         BOOLEAN (* _System ListEmpty)( DLIST ListToCheck,
                                                                        CARDINAL32 * Error);
                                         BOOLEAN (* _System AtEndOfList)( DLIST ListToCheck,
                                                                          CARDINAL32 * Error);
                                         BOOLEAN (* _System AtStartOfList)( DLIST ListToCheck,
                                                                            CARDINAL32 * Error);
                                         void (* _System DestroyList)( DLIST *  ListToDestroy,
                                                                       BOOLEAN FreeItemMemory,
                                                                       CARDINAL32 * Error);
                                         void (* _System NextItem)( DLIST  ListToAdvance,
                                                                    CARDINAL32 * Error);
                                         void (* _System PreviousItem)( DLIST  ListToChange,
                                                                        CARDINAL32 * Error);
                                         void (* _System GoToStartOfList)( DLIST ListToReset,
                                                                           CARDINAL32 * Error);
                                         void (* _System GoToEndOfList)( DLIST ListToSet,
                                                                         CARDINAL32 * Error);
                                         void (* _System GoToSpecifiedItem)( DLIST ListToReposition,
                                                                             ADDRESS Handle,
                                                                             CARDINAL32 * Error);
                                         void (* _System SortList)(DLIST       ListToSort,
                                                                   INTEGER32 (* _System Compare) (ADDRESS Object1, TAG Object1Tag, ADDRESS Object2, TAG Object2Tag,CARDINAL32 * Error),
                                                                   CARDINAL32 * Error);
                                         void (* _System ForEachItem)(DLIST        ListToProcess,
                                                                      void (* _System ProcessItem) (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error),
                                                                      ADDRESS      Parameters,
                                                                      BOOLEAN      Forward,
                                                                      CARDINAL32 * Error);
                                         void (* _System PruneList)(DLIST        ListToProcess,
                                                                    BOOLEAN (* _System KillItem) (ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error),
                                                                    ADDRESS      Parameters,
                                                                    CARDINAL32 * Error);
                                         void (* _System AppendList)(DLIST        TargetList,
                                                                     DLIST        SourceList,
                                                                     CARDINAL32 * Error);
                                         void (* _System TransferItem)(DLIST            SourceList,
                                                                       ADDRESS          SourceHandle,
                                                                       DLIST            TargetList,
                                                                       ADDRESS          TargetHandle,
                                                                       Insertion_Modes  TransferCode,
                                                                       BOOLEAN          MakeCurrent,
                                                                       CARDINAL32 *     Error);
#ifdef DEBUG
                                         BOOLEAN (* _System CheckListIntegrity)(DLIST ListToCheck);
#endif
                                       } LVM_Common_Services_V1;

typedef struct _LVM_Plugin_DLL_Interface{
                                          void (* _System Get_Required_LVM_Version)(CARDINAL32 * Major_Version_Number,
                                                                            CARDINAL32 * Minor_Version_Number);
                                          ADDRESS (* _System Exchange_Function_Tables)(ADDRESS Common_Services);
                                        } LVM_Plugin_DLL_Interface;

#endif
