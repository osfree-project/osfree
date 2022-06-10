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
 * Module: Drive_Linking.c
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

#include <stdlib.h>   /* malloc, free */
#include <stdio.h>    /* sprintf */
#include <string.h>   /* strlen */

#include "engine.h"   /* Included for access to the global types and variables. */
#include "diskio.h"   /*  */

#define NEED_BYTE_DEFINED
#include "gbltypes.h" /* CARDINAL32, BYTE, BOOLEAN, ADDRESS */

#include "lvm_data.h"

#include "LVM_Interface.h"

#include "LVM_Constants.h"   /* PARTITION_NAME_SIZE, VOLUME_NAME_SIZE, DISK_NAME_SIZE, BYTES_PER_SECTOR */

#include "lvm_plug.h"

#include "Drive_Linking_Feature.h"

#include "Drive_Linking.h"

#include "constant.h"
#include "panels.h"
#include "display.h"
#include "getkey.h"


#ifdef DEBUG

#ifdef PARANOID

#include <assert.h>   /* assert */

#endif

#endif

typedef unsigned int bool, *pbool;

/*--------------------------------------------------
 * Private Constants
 --------------------------------------------------*/
#define UNEXPECTED_END_OF_INPUT "Unexpected end of input!  Expecting 'ALL' or the name of an aggregate."
#define UNEXPECTED_END_OF_INPUT2 "Unexpected end of input!  Expecting ')'."
#define AGGREGATE_NAME_EXPECTED  "Error!  An aggregate name or the word 'All' is expected here."
#define AGGREGATE_NAME_NOT_UNIQUE "Error!  Duplicate aggregate name specified.  Aggregate names must be unique!"
#define EQUAL_SIGN_EXPECTED "Error!  An equal sign is expected after an aggregate name."
#define BRACE_EXPECTED " Error!  A '{' character is expected!"
#define DRIVE_ID_EXPECTED "Error!  A drive name or number is expected here!"
#define NEW_COMMAND_EXPECTED "Error!  A ']' or an aggregate name is expected here!"
#define NEW_PARTITION_EXPECTED "Error!  A comma or a '}' is expected here!"
#define INVALID_DRIVE_NUMBER_SPECIFIED "Error!  The drive number specified is invalid!"
#define COMMA_EXPECTED "Error!  A comma followed by a partition name is expected here!"
#define PARTITION_NAME_EXPECTED "Error!  A partition name is expected here!"
#define PARTITION_NAME_TOO_LONG "Error!  The partition name specified is too long!"
#define MISSING_AGGREGATE_NAME  "Error!  An aggregate name is expected after this comma!"
#define OLD_DRIVE_LINKING_MAJOR_VERSION  1
#define OLD_DRIVE_LINKING_MINOR_VERSION  0
#define  OLD_CURRENT_LVM_MAJOR_VERSION_NUMBER   1        /* Define as appropriate. */
#define  OLD_CURRENT_LVM_MINOR_VERSION_NUMBER   0        /* Define as appropriate. */

/*--------------------------------------------------
 * Private Type definitions
 --------------------------------------------------*/
typedef struct _Extended_Drive_Link {
                                      Drive_Link       Link_Data;
                                      Partition_Data * PartitionRecord;
                                    } Extended_Drive_Link;

typedef struct _Drive_Link_Array {
                                   CARDINAL32           Links_In_Use;
                                   Extended_Drive_Link  LinkArray[MAXIMUM_LINKS];
                                   CARDINAL32           Sequence_Number;
                                   BOOLEAN              ChangesMade;
                                   CARDINAL32           Aggregate_Serial_Number;
                                   LVM_Classes          Actual_Class;
                                   CARDINAL32           Feature_Sequence_Number;
                                   BOOLEAN              Top_Of_Class;
                                   CARDINAL32           Fake_EBR_Size;
                                   ADDRESS              Fake_EBR_Buffer;
                                   BOOLEAN              Aggregate_Signature_Sector_Expected;
                                 } Drive_Link_Array;

typedef struct _Aggregate_Search_Record {
                                          Partition_Data *   PartitionRecord;
                                          CARDINAL32         Aggregate_Serial_Number;
                                          Drive_Link_Array * LinkTable;
                                          BOOLEAN            Aggregate_Found;
                                        } Aggregate_Search_Record;

typedef struct _Aggregate_Validation_Record {
                                              DLIST        Partition_List;
                                              BOOLEAN      Volume_Created;
                                              CARDINAL32 * Error_Code;
                                            } Aggregate_Validation_Record;

typedef struct _LSN_Translation_Record {
                                         Partition_Data  *  Aggregate;
                                         CARDINAL32         Sector_LSN;
                                         Partition_Data  *  PartitionRecord;     /* The PartitionRecord for the partition containing the specified sector. */
                                         CARDINAL32         Sector_PSN;          /* The sector's address relative to the partition containing it. */
                                       } LSN_Translation_Record;

typedef struct _Partition_Deletion_Record {
                                            CARDINAL32      LVM_Error;
                                            BOOLEAN         Kill_Partitions;
                                          } Partition_Deletion_Record;

typedef struct _Commit_Changes_Record {

                                        BOOLEAN  Commit_Drive_Linking_Changes;  /* If TRUE, then drive linking data was changed and needs to be written to disk. */
                                        ADDRESS  VData;                         /* The address of the volume data.  This will be passed through to the next layer needing to do a commit. */
                                      } Commit_Changes_Record;

typedef struct _PassThru_Data_Record {
                                       CARDINAL32   Feature_ID;
                                       ADDRESS      Aggregate;
                                       ADDRESS      InputBuffer;
                                       CARDINAL32   InputSize;
                                       ADDRESS *    OutputBuffer;
                                       CARDINAL32 * OutputSize;
                                       CARDINAL32 * Error_Code;
                                     } PassThru_Data_Record;

typedef struct {
                 char *   Aggregate_Name;
                 BOOLEAN  Name_Found;
               } Aggregate_Name_Search_Record;

/*--------------------------------------------------
 * Private Global Variables.
 --------------------------------------------------*/
static BYTE                       Feature_Data_Buffer1[BYTES_PER_SECTOR * DRIVE_LINKING_RESERVED_SECTOR_COUNT ];
static BYTE                       Feature_Data_Buffer2[BYTES_PER_SECTOR * DRIVE_LINKING_RESERVED_SECTOR_COUNT ];
static BYTE                       Fake_EBR_Buffer[BYTES_PER_SECTOR];
static DLIST                      Aggregate_List = NULL;
static Plugin_Function_Table_V1   Function_Table;
static LVM_Common_Services_V1  *  LVM_Common_Services;
static Feature_ID_Data            Feature_ID_Record;

static panel_t DL_help_panel = {0};
static char ** DL_help_panel_text;
static panel_t DL_group_panel = {0};
static char ** DL_group_panel_text;
static panel_t message_bar_panel = {0};
static char *message_bar_panel_text[2] = {NULL, NULL};
static unsigned int DL_group_panel_cursor = 0;
static unsigned int DL_group_partition_count;
static DLink_Comm_Struct *pDLink;
static int DL_cur_aggregate_number;
static unsigned int *DL_group_panel_state;


/*--------------------------------------------------
 * Private functions.
 --------------------------------------------------*/
static void     _System Create_Aggregates( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);
static BOOLEAN  _System Eliminate_Bad_Aggregates( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code);
static void     _System Remove_Features_From_Aggregate_Partitions( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static BOOLEAN  _System Delete_Partitions( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code);
static void     _System Find_Existing_Aggregate(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static void     _System Write_Feature_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static BOOLEAN  Feature_Data_Is_Valid( BOOLEAN Primary );
static BOOLEAN  Link_Tables_Match(Drive_Link_Array * LinkTable, BOOLEAN Copy_Instead, BOOLEAN Use_Primary_Buffer);
static void     _System Initialize_DL_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static void     _System Calculate_Aggregate_Size(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void     _System Set_Spanned_Volume_Flag(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error);
static void     _System Discover_Drive_Links( DLIST  Partition_List, CARDINAL32 * Error_Code );
static void     _System Create_DL_Volume( DLIST Partition_List,
                                          ADDRESS VData,
                                          ADDRESS Init_Data,
                                          void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                          LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                          BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                          CARDINAL32   Sequence_Number,
                                          CARDINAL32 * Error_Code );
static void     _System Open_Feature( CARDINAL32 * Error_Code );
static void     _System Close_Feature( void );
static BOOLEAN  _System Can_Expand_DL_Volume( ADDRESS AData, CARDINAL32 * Feature_ID, CARDINAL32 * Error_Code );
static void     _System Add_DL_Partition ( ADDRESS AData, ADDRESS PData, CARDINAL32 * Error_Code );
static void     _System Delete_DL_Partition( ADDRESS AData, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code);
static void     _System Commit_Drive_Linking_Changes( ADDRESS VData, ADDRESS PData, CARDINAL32 * Error_Code );
static void     _System DL_Write( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code);
static void     _System DL_Read( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code);
static void     _System Remove_Features(ADDRESS Aggregate, CARDINAL32 * Error_Code);
static void     _System ReturnCurrentClass( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number );
static void     _System PassThru( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code );
static BOOLEAN  _System Load_Feature_Data(Partition_Data * PartitionRecord, Drive_Link_Array * LinkTable, CARDINAL32 * Error_Code);
static void     _System Continue_PassThru(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static void     _System Continue_Changes_Pending(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code);
static BOOLEAN  _System Claim_Aggregate_Partitions( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code);
static void     _System Add_Partitions_To_Aggregate( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);
static void     _System Fill_In_Comm_Buffer( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);
static BOOLEAN  _System DL_ChangesPending(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code);
static void     _System DL_ParseCommandLineArguments(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code );
static void     _System Find_Duplicate_Aggregate_Names( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);
static void             Parse_Partition_List( DLIST  Token_List, DLIST  Potential_Aggregates, CARDINAL32 Current_Aggregate, LVM_Token * Aggregate_Name_Token, char **  Error_Message, CARDINAL32 *  Error_Code);
static void     _System Transfer_To_Comm_Buffer( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);
static LVM_Token *      GetToken(DLIST Token_List, BOOLEAN CurrentToken, CARDINAL32 * Error_Code);
static LVM_Token *      LookAhead(DLIST Token_List, CARDINAL32  Count, CARDINAL32 * Error_Code);
static void     _System Find_Partition_By_Name( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);
static void     _System Find_Duplicate_ANames( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code);


void _System VIO_Help_Panel (CARDINAL32 Help_Index, CARDINAL32 * Error_Code);
void _System VIO_Create_and_Configure ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code);
unsigned int _System Create_and_Configure_callback (panel_t *panel);
void Init_VIO_Help_Panel(void);
unsigned long VIO_ShowMessageBar(unsigned long message_number);
void VIO_ClearMessageBar(void);
void get_char_message ( char    *memory,           /* user supplied memory */
                        uint    message_number,
                        char    *array,
                        uint    array_length );
char * get_line_message ( char *memory,            /* user supplied memory */
                          uint message_number);

void * get_help_message ( uint    message_number );

bool MoveMenuCursor( unsigned int cur_position,
                     unsigned int *new_position,
                     unsigned int *menu_state,
                     int  count,
                     bool bForward);



/*--------------------------------------------------
 * There are no additional public global variables
 * beyond those declared in "engine.h".
 --------------------------------------------------*/



/*--------------------------------------------------
 * Public Functions Available
 --------------------------------------------------*/

static void _System Discover_Drive_Links( DLIST  Partition_List, CARDINAL32 * Error_Code )
{

  CARDINAL32                    Dlist_Error;
  Aggregate_Validation_Record   Validation_Record;
  BOOLEAN                       Aggregates_Created = FALSE;

  FEATURE_FUNCTION_ENTRY("Discover_Drive_Links")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( Partition_List == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Discover_Drive_Links has been invoked with one or more NULL pointers!\n     Partition_List is %X (hex)\n     Error_Code is %X (hex)", Partition_List, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* Assume success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  /* We must look at each item in the partitions list.  This includes reading in and validating the feature data. */
  LVM_Common_Services->ForEachItem( Partition_List, &Create_Aggregates, Error_Code, TRUE, &Dlist_Error);

  /* Did we succeed? */
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"PruneList failed while creating aggregates.\n     LVM error code %d (decimal)", *Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

    FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

    return;

  }

  if ( Dlist_Error != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"PruneList failed while creating aggregates.\n     DLIST error code %d (decimal)", Dlist_Error);
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( Dlist_Error != DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    else
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

    return;

  }

  /* Now we must eliminate any incomplete Aggregates. */
  Validation_Record.Partition_List = Partition_List;
  Validation_Record.Volume_Created = FALSE;
  Validation_Record.Error_Code = Error_Code;

  /* Find and eliminate incomplete aggregates. */
  LVM_Common_Services->PruneList( Aggregate_List, &Eliminate_Bad_Aggregates, &Validation_Record, &Dlist_Error);

  /* Did we succeed? */
  if ( ( *Error_Code != LVM_ENGINE_NO_ERROR ) && ( *Error_Code != LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE ) )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"PruneList failed while eliminating bad aggregates.\n     LVM error code %d (decimal)", *Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

    FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

    return;

  }


  if ( Dlist_Error != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"PruneList failed while eliminating bad aggregates.\n     DLIST error code %d (decimal)", Dlist_Error);
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( Dlist_Error != DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    else
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

    return;

  }

  /* Are there any complete aggregates in the Aggregate_List? */
  if ( LVM_Common_Services->GetListSize(Aggregate_List, &Dlist_Error) == 0 )
  {

    if ( Dlist_Error != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

      return;

    }

  }
  else
    Aggregates_Created = TRUE;

  /* Now we must add the list of Aggregates to the List of Partitions. */
  LVM_Common_Services->AppendList(Partition_List, Aggregate_List, &Dlist_Error);

  /* Did we succeed? */
  if ( Dlist_Error != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"AppendList failed with DLIST error code %d (decimal)", Dlist_Error);
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( Dlist_Error != DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    else
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

    return;

  }

  /* Indicate success and return. */

  /* Were any aggregates created? */
  if ( Aggregates_Created )
    *Error_Code = LVM_ENGINE_NO_ERROR;                    /* At least 1 aggregate was created! */
  else
    *Error_Code = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE; /* No aggregates could be created! */

  FEATURE_FUNCTION_EXIT("Discover_Drive_Links")

  return;

}


static void _System Open_Feature( CARDINAL32 * Error_Code )
{

  FEATURE_FUNCTION_ENTRY("Open_Feature")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( Aggregate_List != NULL )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Open_Feature has been invoked on drive linking, \n     but drive linking is already open!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( Error_Code == NULL )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Open_Feature for drive linking has been invoked with a NULL pointer!\n     Error_Code is %X (hex)", Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  if ( Aggregate_List == NULL )
  {

    /* Initialize the Aggregate_List. */
    Aggregate_List = LVM_Common_Services->CreateList();

    if ( Aggregate_List == NULL )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Unable to create the Aggregate_List.  Out of memory!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    }
    else
      *Error_Code = LVM_ENGINE_NO_ERROR;

  }
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Open_Feature")

  return;

}


static void _System Close_Feature( void )
{

  CARDINAL32  LocalError;

  FEATURE_FUNCTION_ENTRY("Close_Feature")

  /* Free up the memory associated with the global variable Aggregate_List. */
  if ( Aggregate_List != NULL )
    LVM_Common_Services->DestroyList(&Aggregate_List,TRUE, &LocalError);

  FEATURE_FUNCTION_EXIT("Close_Feature")

  return;

}


static BOOLEAN _System Can_Expand_DL_Volume( ADDRESS AData, CARDINAL32 * Feature_ID, CARDINAL32 * Error_Code )
{

  Partition_Data *   Aggregate = (Partition_Data *) AData;
  Drive_Link_Array * LinkTable;
  BOOLEAN            ReturnValue = FALSE;

  FEATURE_FUNCTION_ENTRY("Can_Expand_DL_Volume")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( AData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Can_Expand_DL_Volume has been invoked with one or more NULL pointers!\n     AData is %X (hex)\n     Error_Code is %X (hex)", AData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Can_Expand_DL_Volume has been invoked with the following parameters.\n     The partition specified has handle %X (hex)\n      Error_Code is at address %X (hex)",
              Aggregate->External_Handle,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

#ifdef DEBUG

#ifdef PARANOID

  assert(Aggregate != NULL);
  assert(Aggregate->Feature_Data != NULL);
  assert(Aggregate->Feature_Data->Data != NULL);

#else

  if ( ( Aggregate == NULL) ||
       ( Aggregate->Feature_Data == NULL) ||
       ( Aggregate->Feature_Data->Data == NULL)
     )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Invalid Aggregate specified!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Can_Expand_DL_Volume")

    return FALSE;

  }

#endif

#endif

  /* Set the Feature_ID. */
  *Feature_ID = DRIVE_LINKING_FEATURE_ID;

  /* Get the drive linking table. */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;

  /* Check the Links_In_Use field of the link table.  If it is less than MAXIMUM_LINKS - 1, then we can expand. */
  if ( LinkTable->Links_In_Use < ( MAXIMUM_LINKS - 1 ) )
    ReturnValue = TRUE;
  else
    ReturnValue = FALSE;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ReturnValue )
      sprintf(LVM_Common_Services->Log_Buffer,"The volume can be expanded!");
    else
      sprintf(LVM_Common_Services->Log_Buffer,"The volume can NOT be expanded!");

    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Can_Expand_DL_Volume")

  return ReturnValue;

}

static void _System Delete_DL_Partition( ADDRESS AData, BOOLEAN Kill_Partitions, CARDINAL32 * Error_Code)
{

  Partition_Data *           Aggregate = (Partition_Data *) AData;
  CARDINAL32                 Dlist_Error;
  Partition_Deletion_Record  Deletion_Data;
  Drive_Link_Array *         LinkTable;

  FEATURE_FUNCTION_ENTRY("Delete_DL_Partition")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( AData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Delete_DL_Partition has been invoked with one or more NULL pointers!\n     AData is %X (hex)\n     Error_Code is %X (hex)", AData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Delete_DL_Partition has been invoked with the following parameters.\n     The Volume specified has handle %X (hex)\n     Kill_Partitions is %X (hex)\n     Error_Code is at address %X (hex)",
              Aggregate->Volume_Handle,
              Kill_Partitions,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* Does this partition have an LVM Signature Sector? */
  if ( Aggregate->Signature_Sector != NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Killing the LVM Signature Sector of the Aggregate.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* Now free the memory. */
    memset(Aggregate->Signature_Sector,0, BYTES_PER_SECTOR);
    LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);

  }

  /* Remove the Aggregate from the list of Aggregates. */
  LVM_Common_Services->DeleteItem(LVM_Common_Services->Aggregates, FALSE, Aggregate->Drive_Partition_Handle, &Dlist_Error);
  if ( Dlist_Error != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DeleteItem failed while removing an aggregate from the list of aggregates.\n     The DLIST Error code is %d (decimal)", Dlist_Error);
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Delete_DL_Partition")

    return;

  }

  /* Since this is an aggregate, we must walk the Partitions list and delete each of the Partitions which comprise this aggregate. */
  Deletion_Data.LVM_Error = LVM_ENGINE_NO_ERROR;
  Deletion_Data.Kill_Partitions = Kill_Partitions;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Deleting the partitions which formed the Aggregate.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  LVM_Common_Services->PruneList(Aggregate->Feature_Data->Partitions, &Delete_Partitions, &Deletion_Data, &Dlist_Error);

  if ( ( Dlist_Error != DLIST_SUCCESS ) && ( Deletion_Data.LVM_Error != LVM_ENGINE_NO_ERROR ) )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"PruneList failed while deleting partitions.\n     The LVM Error code is %d (decimal)\n     The DLIST Error code is %d (decimal)", *Error_Code, Dlist_Error);
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Delete_DL_Partition")

    return;

  }

  /* Now free the memory. */
  LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions), FALSE, &Dlist_Error);
  if ( Dlist_Error != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DestroyList failed with DLIST error %d (decimal)", Dlist_Error);
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Delete_DL_Partition")

    return;

  }

  /* We must see if there is anything in the LinkTable to delete. */

  /* Get the LinkTable. */
  LinkTable = Aggregate->Feature_Data->Data;

  /* Is there a Fake EBR associated with this aggregate? */
  if ( LinkTable->Fake_EBR_Buffer != NULL )
  {

    /* Free the fake EBR. */
    LVM_Common_Services->Deallocate(LinkTable->Fake_EBR_Buffer);

  }

  /* Now free the Link Table. */
  LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);

  /* Now free the feature data. */
  LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

  /* Free the aggregate's external handle. */
  LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle, Error_Code);
  if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Delete_DL_Partition")

    return;

  }

  /* Now we can free the aggregate. */
  LVM_Common_Services->Deallocate(Aggregate);

  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Delete_DL_Partition")

  return;

}


static void _System Create_DL_Volume( DLIST Partition_List,
                                      ADDRESS VData,
                                      ADDRESS Init_Data,
                                      void (* _System Create_and_Configure) ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code),
                                      LVM_Classes  Actual_Class,  /* Tells the feature what class it is being used in on this volume.  Useful for features that can be in multiple classes. */
                                      BOOLEAN      Top_Of_Class,  /* TRUE if this feature is the topmost feature in its class.  The topmost aggregator must only turn out 1 aggregate! */
                                      CARDINAL32   Sequence_Number,
                                      CARDINAL32 * Error_Code )
{

  Partition_Data *              Aggregate;
  Volume_Data *                 VolumeRecord = (Volume_Data*) VData;
  LVM_Signature_Sector *        Signature_Sector;
  Drive_Link_Array *            LinkTable;
  CARDINAL32                    Ignore_Error;
  CARDINAL32                    Comm_Buffer_Size;
  DLink_Comm_Struct *           Comm_Buffer = NULL;                    /* Used to communicate with the JAVA and VIO interface portions of this plug-in. */
  DLIST                         Partitions_To_Use = (DLIST) NULL;
  CARDINAL32                    Aggregate_Count = 0;
  CARDINAL32                    Partition_Count;
  CARDINAL32                    Index;
  Aggregate_Name_Search_Record  Duplicate_Name_Data;            /* Used to check for a duplicate aggregate name in the Partitions_List. */
  char *                        Current_Aggregate_Name;                /* Used to hold the name of the aggregate being created. */
  BOOLEAN                       Process_Aggregate = TRUE;
  BOOLEAN                       Make_One_Aggregate = FALSE;            /* Set to TRUE if we are to put all partitions into one aggregate. */



  FEATURE_FUNCTION_ENTRY("Create_DL_Volume")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( Partition_List == NULL ) || ( VData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Create_DL_Volume has been invoked with one or more NULL pointers!\n     Partition_List is %X (hex)\n     VData is %X (hex)\n     Error_Code is %X (hex)", Partition_List, VData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

    if ( Actual_Class != Aggregate_Class )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Create_DL_Volume has been invoked with an invalid class!");
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  if ( Actual_Class != Aggregate_Class )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"The wrong LVM Class was specified for this feature!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_WRONG_CLASS_FOR_FEATURE;

    FEATURE_FUNCTION_EXIT("Create_DL_Volume")

    return;

  }

  /* How many partitions are in the Partition_List? */
  Partition_Count = LVM_Common_Services->GetListSize(Partition_List, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Unable to get the size of the Partition_List!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Create_DL_Volume")

    return;

  }

  /* If we are not the topmost aggregator, then we must interface with the user to see what the user wants unless we have been given an initialization buffer. */
  if ( ( ! Top_Of_Class ) &&
       ( Partition_Count > 1 )
     )
  {


    /* Were we given an initialization buffer? */
    if ( Init_Data != NULL )
    {

      /* We were given an initialization buffer.  This initialization buffer is in the same format as the communication
         buffer used with the Create_and_Configure function.  We will set Comm_Buffer to the same address as Init_Data
         and pretend we received the contents of Comm_Buffer from the Create_and_Configure function.                     */
      Comm_Buffer = (DLink_Comm_Struct *) Init_Data;

    }
    else
    {

      /* We must build the Comm_Buffer and call the Create_and_Configure function. */

      /* Allocate memory for the Comm_Buffer. */
      Comm_Buffer_Size =  sizeof(DLink_Comm_Struct) + ( ( Partition_Count - 1 ) * sizeof( DLink_Partition_Array_Entry ) );
      Comm_Buffer = (DLink_Comm_Struct *) LVM_Common_Services->Allocate( Comm_Buffer_Size );
      if ( Comm_Buffer == NULL )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory for the Comm_Buffer!");
          LVM_Common_Services->Write_Log_Buffer();

        }

        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        FEATURE_FUNCTION_EXIT("Create_DL_Volume")

        return;

      }

      /* Now clear the Comm_Buffer. */
      memset(Comm_Buffer,0,Comm_Buffer_Size);

      /* Now fill in the Comm_Buffer. */
      Comm_Buffer->Count = 0;
      Comm_Buffer->Sequence_Number = VolumeRecord->Next_Aggregate_Number;
      LVM_Common_Services->ForEachItem(Partition_List, &Fill_In_Comm_Buffer, Comm_Buffer, TRUE, Error_Code);
      if ( *Error_Code != DLIST_SUCCESS )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Error filling in the Comm_Buffer!");
          LVM_Common_Services->Write_Log_Buffer();

        }

        LVM_Common_Services->Deallocate(Comm_Buffer);

        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FEATURE_FUNCTION_EXIT("Create_DL_Volume")

        return;

      }

      /* Now call the create and configure function. */
      Create_and_Configure( DRIVE_LINKING_FEATURE_ID, Comm_Buffer, Comm_Buffer_Size, NULL, NULL, Error_Code );
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Create_and_Configure returned error: %d!", *Error_Code);
          LVM_Common_Services->Write_Log_Buffer();

        }

        LVM_Common_Services->Deallocate(Comm_Buffer);

        FEATURE_FUNCTION_EXIT("Create_DL_Volume")

        return;

      }

    }

  }

  /* Are we to make a single aggregate, or multiple aggregates? */
  if ( Top_Of_Class ||
       ( Partition_Count == 1 ) ||
       (
         ( Init_Data != NULL ) &&
         ( Comm_Buffer->Count == (CARDINAL32) -1L )
       )
     )
    Make_One_Aggregate = TRUE;

  while ( Process_Aggregate )
  {

    /* Clear the aggregate name. */
    Current_Aggregate_Name = NULL;

    /* Are we the topmost aggregator? */
    if ( Make_One_Aggregate )
    {

      /* Since we are the topmost aggregator, we must aggregate all partitions/aggregates in the Partitions_List
         into a single aggregate.  As such, we can set Partitions_To_Use to Partitions_List. */
      Partitions_To_Use = Partition_List;

      /* Since all of the partitions/aggregates in the Partition_List must be aggregated into a single entity,
         we will only be producing 1 aggregate.  Set Process_Aggregate to FALSE so that we will only pass through
         the while loop once.                                                                                       */
      Process_Aggregate = FALSE;

    }
    else
    {

      /* Has Partitions_To_Use been allocated yet? */
      if ( Partitions_To_Use == NULL )
      {

        /* We must allocate this list as it is used by the aggregate creation code. */
        Partitions_To_Use = LVM_Common_Services->CreateList();

        if ( Partitions_To_Use == NULL )
        {

          if ( LVM_Common_Services->Logging_Enabled )
          {

            sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory for the Partitions_To_Use list!");
            LVM_Common_Services->Write_Log_Buffer();

          }

          if ( Init_Data == NULL )
            LVM_Common_Services->Deallocate(Comm_Buffer);

          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          FEATURE_FUNCTION_EXIT("Create_DL_Volume")

          return;

        }

      }

      /* Find the partitions that belong to the current aggregate and place those into the Partitions_To_Use list. */
      Process_Aggregate = FALSE;
      for ( Index = 0; Index < Partition_Count; Index++ )
      {

        /* Is the current partition part of the aggregate we are preparing to build? */
        if ( Comm_Buffer->Partition_Array[Index].Aggregate_Number == Aggregate_Count )
        {

          /* Is the current partition referenced by name or handle?  If it is referenced by handle, then
             we can simply transfer its entry from the Partition_List to the Partitions_To_Use list.  If
             it is referenced by name, then we must find the partition in the Partition_List which corresponds
             to the name given and then transfer that partition to the Partitions_To_Use list.                   */
          if ( Comm_Buffer->Partition_Array[Index].Use_Partition_Name )
          {

            /* We must find the partition in the Partitions_List which has the same name and exists on the specified drive. */
            LVM_Common_Services->ForEachItem(Partition_List, &Find_Partition_By_Name, &Comm_Buffer->Partition_Array[Index], TRUE, Error_Code);

            /* Was the current partition found?  If it was, then it will have a partition handle and its Use_Partition_Name flag will be set to FALSE. */
            if ( Comm_Buffer->Partition_Array[Index].Use_Partition_Name || (Comm_Buffer->Partition_Array[Index].Partition_Handle == NULL) )
            {

              /* We have a problem as the partition was not found! */

              LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

              *Error_Code = LVM_ENGINE_PARTITION_NOT_FOUND;

              FEATURE_FUNCTION_EXIT("Create_DL_Volume")

              return;

            }

          }

          /* The current partition is part of the aggregate we are trying to build.  Move it into the Partitions_To_Use list. */
          LVM_Common_Services->TransferItem(Partition_List, Comm_Buffer->Partition_Array[Index].Reserved, Partitions_To_Use, NULL, AppendToList, FALSE, Error_Code);
          if ( *Error_Code != DLIST_SUCCESS )
          {

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Error transferring an item from the Partition_List to the Partitions_To_Use list!");
              LVM_Common_Services->Write_Log_Buffer();

            }

            /* If we allocated the Comm_Buffer, then free it. */
            if ( Init_Data == NULL )
              LVM_Common_Services->Deallocate(Comm_Buffer);

            LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

            FEATURE_FUNCTION_EXIT("Create_DL_Volume")

            return;


          }

          /* We need to create this aggregate, so make sure that Process_Aggregate is TRUE. */
          Process_Aggregate = TRUE;

          /* Save the name of the aggregate, if there is one. */
          if ( ( Current_Aggregate_Name == NULL) && ( Comm_Buffer->Partition_Array[Index].Aggregate_Name[0] != 0 ) )
          {

            /* Save the aggregate name. */
            Current_Aggregate_Name = (char *) &(Comm_Buffer->Partition_Array[Index].Aggregate_Name);

          }

        }

      }

      /* If no partitions were found, then we are done. */
      if ( ! Process_Aggregate )
      {

        /* Were any aggregates defined? */
        if ( Aggregate_Count == 0 )
        {

          /* The Create_and_Configure function did not define any aggregates! */
          *Error_Code = LVM_ENGINE_OPERATION_NOT_ALLOWED;

          FEATURE_FUNCTION_EXIT("Create_DL_Volume")

          return;

        }

        break;

      }
      else
      {
        Aggregate_Count += 1;

        /* If this aggregate has a name, is the name already in use in the Partition_List? */
        if ( Current_Aggregate_Name != NULL )
        {

          /* Set up the duplicate name check. */
          Duplicate_Name_Data.Name_Found = FALSE;
          Duplicate_Name_Data.Aggregate_Name = Current_Aggregate_Name;

          /* Look for the name in the Partition_List. */
          LVM_Common_Services->ForEachItem(Partition_List, &Find_Duplicate_ANames, &Duplicate_Name_Data, TRUE, Error_Code);
          if ( *Error_Code != DLIST_SUCCESS )
          {

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Error transferring an item from the Partition_List to the Partitions_To_Use list!");
              LVM_Common_Services->Write_Log_Buffer();

            }

            /* If we allocated the Comm_Buffer, then free it. */
            if ( Init_Data == NULL )
              LVM_Common_Services->Deallocate(Comm_Buffer);

            LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

            FEATURE_FUNCTION_EXIT("Create_DL_Volume")

            return;

          }

          /* Was the name found? */
          if ( Duplicate_Name_Data.Name_Found )
          {

            if ( LVM_Common_Services->Logging_Enabled )
            {

              sprintf(LVM_Common_Services->Log_Buffer,"Error! Duplicate aggregate name detected in the Partition_List!");
              LVM_Common_Services->Write_Log_Buffer();

            }

            /* If we allocated the Comm_Buffer, then free it. */
            if ( Init_Data == NULL )
              LVM_Common_Services->Deallocate(Comm_Buffer);

            LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

            *Error_Code = LVM_ENGINE_DUPLICATE_NAME;

            FEATURE_FUNCTION_EXIT("Create_DL_Volume")

            return;

          }

        }

      }

    }

    /* Create the new Aggregate. */
    Aggregate = ( Partition_Data * ) LVM_Common_Services->Allocate( sizeof( Partition_Data ) );

    if ( Aggregate == NULL )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory for a new Aggreate!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    memset(Aggregate,0,sizeof(Partition_Data));


    /* Allocate memory for Feature_Data. */
    Aggregate->Feature_Data = ( Feature_Context_Data * ) LVM_Common_Services->Allocate ( sizeof( Feature_Context_Data ) );

    if ( Aggregate->Feature_Data == NULL )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory for the feature data of the new Aggregate!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* Initialize the Feature_Data. */
    memset( Aggregate->Feature_Data, 0, sizeof (Feature_Context_Data) );
    Aggregate->Feature_Data->Feature_ID = &Feature_ID_Record;
    Aggregate->Feature_Data->Function_Table = &Function_Table;
    Aggregate->Feature_Data->Data = NULL;
    Aggregate->Feature_Data->Partitions = NULL;
    Aggregate->Feature_Data->Old_Context = NULL;

    /* Allocate the Drive Link Array for the Aggregate. */
    Aggregate->Feature_Data->Data = LVM_Common_Services->Allocate( sizeof( Drive_Link_Array ) );
    if ( Aggregate->Feature_Data->Data == NULL )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory for a new drive link array!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    memset( Aggregate->Feature_Data->Data, 0, sizeof( Drive_Link_Array ) );
    LinkTable = Aggregate->Feature_Data->Data;

    /* Allocate the Partitions list which is part of the Feature Data. */
    Aggregate->Feature_Data->Partitions = LVM_Common_Services->CreateList();
    if ( Aggregate->Feature_Data->Partitions == NULL )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Unable to allocate memory for a new partition list!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* Move all of the Partitions in the Partitions_To_Use List to Aggregate->Feature_Data->Partitions. */
    LVM_Common_Services->AppendList( Aggregate->Feature_Data->Partitions, Partitions_To_Use, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Insufficient memory to process the Partitions_To_Use!");
          LVM_Common_Services->Write_Log_Buffer();

        }

        LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
        LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
        LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
        LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
        if ( ! Make_One_Aggregate )
        {

          if ( Init_Data == NULL )
            LVM_Common_Services->Deallocate(Comm_Buffer);

          LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

        }

        *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

        FEATURE_FUNCTION_EXIT("Create_DL_Volume")

        return;

      }

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"AppendList failed with DLIST error %d (decimal)", *Error_Code);
        LVM_Common_Services->Write_Log_Buffer();

      }

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* We must allocate an LVM Signature Sector for the Aggregate. */
    Aggregate->Signature_Sector = ( LVM_Signature_Sector * ) LVM_Common_Services->Allocate( BYTES_PER_SECTOR );
    if ( Aggregate->Signature_Sector == NULL )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Insufficient memory to create an LVM Signature Sector!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if  ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* Initialize the LVM Signature Sector to all 0's. */
    memset( Aggregate->Signature_Sector, 0, sizeof( LVM_Signature_Sector ) );
    Signature_Sector = Aggregate->Signature_Sector;

    /* We now have all of the memory we need, and all of the partitions have been moved out of the Partitions_To_Use List.  Now we
       can initialize our Aggregate and each of the Partitions that are a part of it.                                                 */
    Aggregate->Partition_Type = Partition;
    Aggregate->New_Partition = TRUE;
    Aggregate->Drive_Index = 0xFFFFFFFFL;/* We don't need the drive index as this is not a real partition. */
    Aggregate->External_Volume_Handle = VolumeRecord->External_Handle;
    Aggregate->Volume_Handle = VolumeRecord->Volume_Handle;
    Aggregate->Starting_Sector = 0;
    Aggregate->DLA_Table_Entry.On_Boot_Manager_Menu = FALSE;
    Aggregate->DLA_Table_Entry.Installable = FALSE;
    Aggregate->DLA_Table_Entry.Partition_Serial_Number = LVM_Common_Services->Create_Serial_Number();
    Aggregate->DLA_Table_Entry.Partition_Start = 0;
    Aggregate->DLA_Table_Entry.Partition_Size = 0;
    Aggregate->DLA_Table_Entry.Volume_Serial_Number = VolumeRecord->Volume_Serial_Number;
    Aggregate->DLA_Table_Entry.Drive_Letter = VolumeRecord->Drive_Letter_Preference;
    strncpy(Aggregate->DLA_Table_Entry.Volume_Name, VolumeRecord->Volume_Name, VOLUME_NAME_SIZE);
    if ( Current_Aggregate_Name != NULL )
      strncpy(Aggregate->Partition_Name, Current_Aggregate_Name, PARTITION_NAME_SIZE);


    /* Save the serial number of the aggregate in the feature data. */
    LinkTable->Aggregate_Serial_Number = Aggregate->DLA_Table_Entry.Partition_Serial_Number;
    LinkTable->Actual_Class = Actual_Class;
    LinkTable->Feature_Sequence_Number = Sequence_Number;
    LinkTable->Top_Of_Class = Top_Of_Class;
    LinkTable->Aggregate_Signature_Sector_Expected = TRUE;

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Initializing the new partitions.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    LVM_Common_Services->ForEachItem(Aggregate->Feature_Data->Partitions, &Initialize_DL_Partitions, Aggregate, TRUE, Error_Code);

  #ifdef DEBUG

  #ifdef PARANOID

    assert( ( *Error_Code == DLIST_SUCCESS ) || ( *Error_Code == DLIST_OUT_OF_MEMORY ) );

  #else

    if ( ( *Error_Code != DLIST_SUCCESS ) && ( *Error_Code != DLIST_OUT_OF_MEMORY ) )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"ForEachItem failed while initializing the new partitions!\n     DLIST Error %d (decimal)", *Error_Code);
        LVM_Common_Services->Write_Log_Buffer();

      }

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

  #endif

  #endif

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Insufficient memory to initialize the new partitions!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Now free everything we can. */
      LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* The Aggregate's partition size has been initialized.  At the moment, the usable size is the same as the ( partition size - 1)
       since there is currently only 1 sector reserved for use by LVM - the LVM Signature Sector.                                     */

    /* Since this is an aggregate, it can not be a primary partition. */
    Aggregate->Primary_Partition = FALSE;

    /* Initialize the LVM Signature Sector for this aggregate. */
    Signature_Sector->LVM_Signature1 = LVM_PRIMARY_SIGNATURE;
    Signature_Sector->LVM_Signature2 = LVM_SECONDARY_SIGNATURE;
    Signature_Sector->Signature_Sector_CRC = 0;
    Signature_Sector->Partition_Serial_Number = Aggregate->DLA_Table_Entry.Partition_Serial_Number;
    Signature_Sector->Partition_Start = 0;
    Signature_Sector->Partition_End = Aggregate->Partition_Size - 1;
    Signature_Sector->Partition_Sector_Count = Aggregate->Partition_Size;
    Signature_Sector->Partition_Size_To_Report_To_User = Aggregate->Partition_Size - 1;                 /* One sector reserved for LVM Signature Sector. */
    Signature_Sector->LVM_Reserved_Sector_Count = 1;
    Signature_Sector->Fake_EBR_Location = 0;
    Signature_Sector->Fake_EBR_Allocated = FALSE;
    Signature_Sector->Boot_Disk_Serial_Number = *(LVM_Common_Services->Boot_Drive_Serial_Number);
    Signature_Sector->Volume_Serial_Number = VolumeRecord->Volume_Serial_Number;
    Signature_Sector->LVM_Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
    Signature_Sector->LVM_Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;
    strncpy(Signature_Sector->Partition_Name, Aggregate->Partition_Name, PARTITION_NAME_SIZE);
    Signature_Sector->Drive_Letter = VolumeRecord->Drive_Letter_Preference;
    strncpy(Signature_Sector->Volume_Name, VolumeRecord->Volume_Name, VOLUME_NAME_SIZE);

    /* If Drive Linking is the topmost aggregator, then it is responsible for allocating the "fake" EBR track. */
    if ( Top_Of_Class )
    {

      /* Create a fake partition record for the aggregate. */
      LVM_Common_Services->Create_Fake_Partition_Table_Entry( &(Aggregate->Partition_Table_Entry), Aggregate->Usable_Size);

      /* Allocate the "fake" EBR. */
      LVM_Common_Services->Create_Fake_EBR( (Extended_Boot_Record **) &(LinkTable->Fake_EBR_Buffer), &(LinkTable->Fake_EBR_Size), Signature_Sector, Error_Code);
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        if ( LVM_Common_Services->Logging_Enabled )
        {

          if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
          {

            sprintf(LVM_Common_Services->Log_Buffer,"Insufficient memory to create the fake EBR!");
            LVM_Common_Services->Write_Log_Buffer();

            *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
          }
          else
          {

            sprintf(LVM_Common_Services->Log_Buffer,"Internal Error attempting to allocate the fake EBR!");
            LVM_Common_Services->Write_Log_Buffer();

            *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          }

        }

        /* Now free everything we can. */
        LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
        if ( LinkTable->Fake_EBR_Buffer != NULL )
          LVM_Common_Services->Deallocate(LinkTable->Fake_EBR_Buffer);
        LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
        LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
        LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
        LVM_Common_Services->Deallocate(Aggregate);

        /* If we allocated the Partitions_To_Use list, LVM_Common_Services->Deallocate it.  Free the Comm_Buffer also.  We will have allocated these
           if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
        if ( ! Make_One_Aggregate )
        {

          if ( Init_Data == NULL )
            LVM_Common_Services->Deallocate(Comm_Buffer);

          LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

        }

        FEATURE_FUNCTION_EXIT("Create_DL_Volume")

        return;

      }

    }
    else
    {

      /* Since there is no fake EBR required here, initialize the fake EBR fields in the LinkTable to NULL. */
      LinkTable->Fake_EBR_Size = 0;
      LinkTable->Fake_EBR_Buffer = NULL;

    }

    /* We must add the aggregate to the list of Aggregates maintained by the LVM Engine. */
    Aggregate->Drive_Partition_Handle = LVM_Common_Services->InsertObject(LVM_Common_Services->Aggregates, sizeof(Partition_Data), Aggregate, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        if ( *Error_Code == DLIST_OUT_OF_MEMORY )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Insufficient memory to add the aggregate to the list of Aggregates!");
          LVM_Common_Services->Write_Log_Buffer();

          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
        }
        else
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Internal Error attempting to add an aggregate to the list of Aggregates!");
          LVM_Common_Services->Write_Log_Buffer();

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        }

      }

      /* Now free everything we can. */
      LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
      if ( LinkTable->Fake_EBR_Buffer != NULL )
        LVM_Common_Services->Deallocate(LinkTable->Fake_EBR_Buffer);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* We must get an external handle for this aggregate. */
    Aggregate->External_Handle = LVM_Common_Services->Create_Handle( Aggregate, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code );
    if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        if ( *Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY )
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Insufficient memory to initialize the new partitions!");
          LVM_Common_Services->Write_Log_Buffer();

          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
        }
        else
        {

          sprintf(LVM_Common_Services->Log_Buffer,"Internal Error attempting to get a handle from the handle manager!");
          LVM_Common_Services->Write_Log_Buffer();

          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        }

      }

      /* Now free everything we can. */
      LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
      LVM_Common_Services->DeleteItem(LVM_Common_Services->Aggregates, FALSE, Aggregate->Drive_Partition_Handle, &Ignore_Error);
      if ( LinkTable->Fake_EBR_Buffer != NULL )
        LVM_Common_Services->Deallocate(LinkTable->Fake_EBR_Buffer);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }


    /* Set the usable size of the aggregate. */
    Aggregate->Usable_Size = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Aggregate is complete!  Adding it to the Partitions_List list.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* Now put the Aggregate back into the Partitions List so that our caller will find it. */
    LVM_Common_Services->InsertObject(Partition_List, sizeof(Partition_Data), Aggregate, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

  #ifdef DEBUG

  #ifdef PARANOID

    assert( ( *Error_Code == DLIST_SUCCESS ) || ( *Error_Code == DLIST_OUT_OF_MEMORY ) );

  #else

    if ( ( *Error_Code != DLIST_SUCCESS ) && ( *Error_Code != DLIST_OUT_OF_MEMORY ) )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"AppendObject failed with DLIST error %d (decimal)", *Error_Code);
        LVM_Common_Services->Write_Log_Buffer();

      }

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

  #endif

  #endif

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"AppendObject failed due to lack of memory!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Now free everything we can. */
      LVM_Common_Services->DestroyList(&(Aggregate->Feature_Data->Partitions),FALSE, &Ignore_Error );
      LVM_Common_Services->DeleteItem(LVM_Common_Services->Aggregates, FALSE, Aggregate->Drive_Partition_Handle, &Ignore_Error);
      LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle, &Ignore_Error);
      if ( LinkTable->Fake_EBR_Buffer != NULL )
        LVM_Common_Services->Deallocate(LinkTable->Fake_EBR_Buffer);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->Deallocate(Aggregate);

      /* If we allocated the Partitions_To_Use list, free it.  Free the Comm_Buffer also.  We will have allocated these
         if this plug-in is not the topmost in its class, and there is more than one partition being aggregated.            */
      if ( ! Make_One_Aggregate )
      {

        if ( Init_Data == NULL )
          LVM_Common_Services->Deallocate(Comm_Buffer);

        LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, &Ignore_Error );

      }

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }


  }


  /* Since we got here, we created all of the aggregates successfully! */

  if ( ! Make_One_Aggregate )
  {

    /* Free the Partitions_To_Use list. */
    LVM_Common_Services->DestroyList(&(Partitions_To_Use),FALSE, Error_Code );
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"List Error %d while trying to destroy the Partitions_To_Use list!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Create_DL_Volume")

      return;

    }

    /* Free the Comm_Buffer. */
    LVM_Common_Services->Deallocate(Comm_Buffer);

  }

  /* Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Create_DL_Volume")

  return;

}

static void _System DL_Write( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Write, ADDRESS Buffer, CARDINAL32 * Error_Code)
{

  Partition_Data *           Aggregate = (Partition_Data *) PData;
  Plugin_Function_Table_V1 * Old_Function_Table;
  Drive_Link_Array *         LinkTable;
  Partition_Data *           PartitionRecord;
  CARDINAL32                 Sector_PSN;
  CARDINAL32                 Index;

  FEATURE_FUNCTION_ENTRY("DL_Write")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( PData == NULL ) || ( Buffer == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Write has been invoked with one or more NULL pointers!\n     PData is %X (hex)\n     Buffer is %X (hex)\n     Error_Code is %X (hex)", PData, Buffer, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "DL_Write has been invoked with the following parameters.\n     The partition specified has handle %X (hex)\n     The LBA of the sector to read is %X (hex)\n     The number of sectors to write is %d (decimal)\n     The location of the buffer to read into is %X (hex)\n      Error_Code is at address %X (hex)",
              Aggregate->External_Handle,
              Starting_Sector,
              Sectors_To_Write,
              Buffer,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Translating Sector.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* We must find out where Sector really is. */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;

  for (Index = 0; Index < LinkTable->Links_In_Use; Index++)
  {

    /* Get the partition for this link in the array. */
    PartitionRecord = LinkTable->LinkArray[Index].PartitionRecord;
    /* Does the requested sector lie in this partition? */
    if ( Starting_Sector < PartitionRecord->Usable_Size )
    {

      /* The requested sector does lie on this partition!  Translate the requested sector to a partition sector number. */
      Sector_PSN = Starting_Sector + PartitionRecord->Starting_Sector;
      break;

    }
    else
      Starting_Sector -= PartitionRecord->Usable_Size;

  }

  if ( Index >= LinkTable->Links_In_Use )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Write failed to find the partition record for the\n     partition where Sector resides!");

      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("DL_Write")

    return;

  }

  /* Is the feature data for the specified partition available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Write has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("DL_Write")

    return;

  }


  /* Now call the Write function of the PartitionRecord to complete the write operation. */
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  /* Is the function table for the next layer available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Write has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("DL_Write")

    return;

  }

  /* Call the write function of the next layer. */
  Old_Function_Table->Write(PartitionRecord, Sector_PSN, Sectors_To_Write, Buffer, Error_Code );

  FEATURE_FUNCTION_EXIT("DL_Write")

  return;

}

static void _System DL_Read( ADDRESS PData, LBA Starting_Sector, CARDINAL32 Sectors_To_Read, ADDRESS Buffer, CARDINAL32 * Error_Code)
{

  Partition_Data *           Aggregate = (Partition_Data *) PData;
  Plugin_Function_Table_V1 * Old_Function_Table;
  Drive_Link_Array *         LinkTable;
  Partition_Data *           PartitionRecord;
  CARDINAL32                 Sector_PSN;
  CARDINAL32                 Index;

  FEATURE_FUNCTION_ENTRY("DL_Read")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( PData == NULL ) || ( Buffer == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Read has been invoked with one or more NULL pointers!\n     PData is %X (hex)\n     Buffer is %X (hex)\n     Error_Code is %X (hex)", PData, Buffer, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "DL_Read has been invoked with the following parameters.\n     The partition specified has handle %X (hex)\n     The LBA of the sector to read is %X (hex)\n     The number of sectors to read is %d (decimal)\n     The location of the buffer to read into is %X (hex)\n      Error_Code is at address %X (hex)",
              Aggregate->External_Handle,
              Starting_Sector,
              Sectors_To_Read,
              Buffer,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Translating Sector.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* We must find out where Sector really is. */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;

  for (Index = 0; Index < LinkTable->Links_In_Use; Index++)
  {

    /* Get the partition for this link in the array. */
    PartitionRecord = LinkTable->LinkArray[Index].PartitionRecord;
    /* Does the requested sector lie in this partition? */
    if ( Starting_Sector < PartitionRecord->Usable_Size )
    {

      /* The requested sector does lie on this partition!  Translate the requested sector to a partition sector number. */
      Sector_PSN = Starting_Sector + PartitionRecord->Starting_Sector;
      break;

    }
    else
      Starting_Sector -= PartitionRecord->Usable_Size;

  }

  if ( Index >= LinkTable->Links_In_Use )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Read failed to find the partition record for the\n     partition where Sector resides!");

      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("DL_Read")

    return;

  }

  /* Is the feature data for the next layer available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Read has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("DL_Read")

    return;

  }

  /* Is the function table for the next layer on the partition available? */
  Old_Function_Table =  PartitionRecord->Feature_Data->Function_Table;
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL_Read has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("DL_Read")

    return;

  }

  /* Now call the Read function of the PartitionRecord to complete the read operation. */
  Old_Function_Table->Read(PartitionRecord, Sector_PSN, Sectors_To_Read, Buffer, Error_Code );

  FEATURE_FUNCTION_EXIT("DL_Read")

  return;

}


static void _System Add_DL_Partition ( ADDRESS AData, DLIST New_Partitions, CARDINAL32 * Error_Code )
{

  Partition_Data *        Aggregate = (Partition_Data *) AData;
  Drive_Link_Array *      LinkTable;
  LVM_Signature_Sector *  Signature_Sector;
  CARDINAL32              Partition_Count;

  FEATURE_FUNCTION_ENTRY("Add_DL_Partition")

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( Aggregate == NULL ) || ( New_Partitions == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Add_DL_Partition has been invoked with one or more NULL pointers!\n     AData is %X (hex)\n     PData is %X (hex)\n     Error_Code is %X (hex)", AData, New_Partitions, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Add_DL_Partition has been invoked with an aggregate and a list of partitions.\n     The aggregate belongs to the volume with handle %X (hex)\n     The list of partitions is at address %X (hex)\n     Error_Code is at address %X (hex)",
              Aggregate->Volume_Handle,
              New_Partitions,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* We must add the new partitions to Aggregate. */

  /* How many new partitions are there. */
  Partition_Count = LVM_Common_Services->GetListSize(New_Partitions, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* The GetListSize call should not have failed!  Abort! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Add_DL_Partition")

    return;

  }

  /* Can we add any more partitions to Aggregate? */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;
  if ( ( LinkTable->Links_In_Use + Partition_Count) >= ( MAXIMUM_LINKS - 1) )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Error: LVM_ENGINE_MAX_PARTITIONS_ALREADY_IN_USE");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_MAX_PARTITIONS_ALREADY_IN_USE;

    FEATURE_FUNCTION_EXIT("Add_DL_Partition")

    return;

  }

  /* Now add each partition to the Aggregate. */
  LVM_Common_Services->ForEachItem(New_Partitions, &Add_Partitions_To_Aggregate, Aggregate, TRUE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    /* We need to undo what we have done so far. */

    /* Or do we leave things as they are?  We may have expanded the volume to some degree, just not
       as much as the user wanted.  Is it better to give them as much of the requested expansion as
       possible, or is it better to use an "all or none" approach?                                    */

    /* ??????????? */

    FEATURE_FUNCTION_EXIT("Add_DL_Partition")

    return;

  }

  /* If this volume is an old style LVM Volume (ver 1.x), then it will not have a signature
     sector for the aggregate.  If it does have a signature sector, then it is a new style
     volume and the signature sector must be updated.                                           */
  if ( Aggregate->Signature_Sector != NULL )
  {

    /* Now we need to update the LVM Signature Sector of the Aggregate.  What we will do is essentially recreate it
       at its new location (it must be the last sector of the Aggregate!).  After we recreate it, as well as any fake
       EBR which may exist, all of the features above Drive Linking must add themselves to the LVM Signature Sector's
       LVM_Feature_Array.  They must also allocate space for their feature data again, just as if a Create_Volume were
       being performed.  The advantage to doing this is it allows each feature to allocate additional space in the LVM
       Data Area (if needed) to compensate for the new, increased size of the volume.                                   */
    Signature_Sector = Aggregate->Signature_Sector;
    memset(&(Signature_Sector->LVM_Feature_Array[0]),0,sizeof(LVM_Feature_Data)*MAX_FEATURES_PER_VOLUME);
    Signature_Sector->Partition_End = Aggregate->Partition_Size - 1;
    Signature_Sector->Partition_Sector_Count = Aggregate->Partition_Size;
    Signature_Sector->LVM_Reserved_Sector_Count = 1;
    Signature_Sector->Partition_Size_To_Report_To_User = Aggregate->Partition_Size - 1;

    /* Is there a Fake EBR associated with this aggregate? */
    if ( Signature_Sector->Fake_EBR_Allocated )
    {

      /* The Fake EBR will be updated during the commit process, but only if it is already in memory.  If this is a
         new Volume that is being expanded, then the Fake EBR will already be loaded.  If this was an existing volume,
         then we must load the Fake EBR so that it can be updated during the commit process.                            */

      /* Do we need to read in the old fake EBR? */
      if ( LinkTable->Fake_EBR_Buffer == NULL )
      {

        /* We must allocate a fake EBR Buffer and read in the fake EBR. */

        /* Set the size of the fake EBR. */
        LinkTable->Fake_EBR_Size = SYNTHETIC_SECTORS_PER_TRACK;

        /* Allocate the memory for the track containing the fake EBR. */
        LinkTable->Fake_EBR_Buffer = LVM_Common_Services->Allocate(SYNTHETIC_SECTORS_PER_TRACK * BYTES_PER_SECTOR);
        if ( LinkTable->Fake_EBR_Buffer == NULL )
        {

          /* We are out of memory! */
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

          FEATURE_FUNCTION_EXIT("Add_DL_Partition")

          return;

        }

        /* Now read in the fake EBR. */
        DL_Read(Aggregate, Signature_Sector->Fake_EBR_Location, LinkTable->Fake_EBR_Size, LinkTable->Fake_EBR_Buffer, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          FEATURE_FUNCTION_EXIT("Add_DL_Partition")

          /* There was an unexpected error!  Abort! */
          return;

        }

      }

    }

  }

  /* Indicate that changes were made. */
  LinkTable->ChangesMade = TRUE;

  /* Indicate success and return. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Add_DL_Partition")

  return;

}


static void _System Commit_Drive_Linking_Changes( ADDRESS VData, ADDRESS PData, CARDINAL32 * Error_Code )
{

  Partition_Data *                 PartitionRecord = (Partition_Data *) PData;
  Volume_Data *                    VolumeRecord = (Volume_Data *) VData;
  CARDINAL32                       LinkIndex;
  CARDINAL32                       Links_Moved = 0;
  CARDINAL32                       Sector_Count;
  LVM_Link_Table_First_Sector *    First_Sector;
  LVM_Link_Table_Sector *          Next_Sector;
  Drive_Link_Array *               LinkTable;
  CARDINAL32                       Offset;
  Commit_Changes_Record            Commit_Data;
  LVM_Signature_Sector *           Signature_Sector;
  Extended_Boot_Record *           Fake_EBR;


  FEATURE_FUNCTION_ENTRY("Commit_DL_Changes")

  /* Get the drive linking table. */
  LinkTable = (Drive_Link_Array *) PartitionRecord->Feature_Data->Data;

  /* Is logging active? */
  if ( LVM_Common_Services->Logging_Enabled )
  {

    if ( ( PData == NULL ) || ( VData == NULL ) || ( Error_Code == NULL) )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Commit_DL_Changes has been invoked with one or more NULL pointers!\n     VData is %X (hex)\n     PData is %X (hex)\n     Error_Code is %X (hex)", VData, PData, Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }
    else
    {

      sprintf(LVM_Common_Services->Log_Buffer,
              "Commit_DL_Changes has been invoked with the following parameters.\n     The Volume specified has handle %X (hex)\n     The partition specified has handle %X (hex)\n     Error_Code is at address %X (hex)",
              VolumeRecord->External_Handle,
              PartitionRecord->External_Handle,
              Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

  }

  /* We must write out the LVM Signature Sector for the Aggregate.  Since we can not tell when it has been modified,
     we must always write it out.                                                                                      */

  /* Is there a signature sector? */
  if ( PartitionRecord->Signature_Sector != NULL )
  {

    /* We will use Feature_Data_Buffer1 to write out the Signature Sector.  Clear the buffer. */
    memset(Feature_Data_Buffer1, 0, BYTES_PER_SECTOR );

    /* Set Signature_Sector to point to Feature_Data_Buffer1 so that we can copy the LVM Signature Sector data into the buffer. */
    Signature_Sector = (LVM_Signature_Sector *) &Feature_Data_Buffer1;

    /* Copy the LVM Signature Sector into the buffer. */
    *Signature_Sector = *(PartitionRecord->Signature_Sector);

    /* Calculate the CRC of the LVM Signature Sector. */
    Signature_Sector->Signature_Sector_CRC = 0;
    Signature_Sector->Signature_Sector_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, Signature_Sector, BYTES_PER_SECTOR);

    /* Now write the Signature Sector to disk. */
    DL_Write(PartitionRecord, PartitionRecord->Starting_Sector + PartitionRecord->Partition_Size - 1, 1, Feature_Data_Buffer1, Error_Code);
    if ( ( *Error_Code != LVM_ENGINE_NO_ERROR ) &&
         ( *Error_Code != LVM_ENGINE_IO_ERROR )
       )
    {

      FEATURE_FUNCTION_EXIT("Commit_DL_Changes")

      /* There was an unexpected error!  Abort! */
      return;

    }

    /* Is there a fake EBR that needs to be written out? */
    if ( LinkTable->Fake_EBR_Buffer != NULL )
    {

      /* Update the sector count based upon what will be reported through the GetDevParms IOCTLs.  We do this here because
         at this point we know what the final size of all aggregates created by this feature are.  Furthermore, an aggregate
         will only have a fake EBR if it is the topmost aggregate, which means that the size of the aggregate is the size of
         Volume.                                                                                                               */
      Fake_EBR = ( Extended_Boot_Record * ) LinkTable->Fake_EBR_Buffer;
      Fake_EBR->Partition_Table[0].Sector_Count = LVM_Common_Services->Compute_Reported_Volume_Size( Signature_Sector->Partition_Size_To_Report_To_User );
      PartitionRecord->Partition_Table_Entry.Sector_Count = Fake_EBR->Partition_Table[0].Sector_Count;

      /* We have a fake EBR.  Let's write it to disk. */
      DL_Write(PartitionRecord, Signature_Sector->Fake_EBR_Location, LinkTable->Fake_EBR_Size, LinkTable->Fake_EBR_Buffer, Error_Code);
      if ( ( *Error_Code != LVM_ENGINE_NO_ERROR ) &&
           ( *Error_Code != LVM_ENGINE_IO_ERROR )
         )
      {

        FEATURE_FUNCTION_EXIT("Commit_DL_Changes")

        /* There was an unexpected error!  Abort! */
        return;

      }

    }

  }

  /* Was the drive linking data changed? */
  if ( LinkTable->ChangesMade || VolumeRecord->Convert_To_LVM_V1 )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Drive Linking data was changed.  Building disk image.");
      LVM_Common_Services->Write_Log_Buffer();

    }


    /* Clear the buffers. */
    memset(Feature_Data_Buffer1, 0, BYTES_PER_SECTOR * DRIVE_LINKING_RESERVED_SECTOR_COUNT );

    /* Setup each sector of data contained in the buffer. */
    for ( Sector_Count = 1; Sector_Count <= DRIVE_LINKING_RESERVED_SECTOR_COUNT; Sector_Count++)
    {

      /* The first sector is different than the ones that follow it.  Handle it separately. */
      if ( Sector_Count == 1)
      {

        /* Find the location of the first sector in the buffer. */
        First_Sector = (LVM_Link_Table_First_Sector *) &Feature_Data_Buffer1;

        /* Set the signature. */
        First_Sector->Link_Table_Signature = LINK_TABLE_MASTER_SIGNATURE;

        /* Set the Links_In_Use field. */
        First_Sector->Links_In_Use = LinkTable->Links_In_Use;

        /* Set the Sequence Number. */
        First_Sector->Sequence_Number = LinkTable->Sequence_Number + 1;
        First_Sector->Aggregate_Serial_Number = LinkTable->Aggregate_Serial_Number;

        /* If this aggregate was created by Version 1 of Drive Linking, then the fields added for
           Version 2 of Drive Linking must be zeroed out to maintain compatibility with Version 1 of Drive Linking. */
        if ( LinkTable->Aggregate_Signature_Sector_Expected || ( ! VolumeRecord->Convert_To_LVM_V1 ) )
        {

          /* We have an aggregate created by Version 2 or later of Drive Linking. */
          First_Sector->Actual_Class = LinkTable->Actual_Class;
          First_Sector->Top_Of_Class = LinkTable->Top_Of_Class;
          First_Sector->Feature_Sequence_Number = LinkTable->Feature_Sequence_Number;

        }
        else
        {

          /* We have an aggregate created by Version 1 of Drive Linking. */
          First_Sector->Actual_Class = 0;
          First_Sector->Top_Of_Class = 0;
          First_Sector->Feature_Sequence_Number = 0;
          First_Sector->Aggregate_Serial_Number = 0;

        }

        /* Fill in the Link_Table in the First_Sector. */
        for ( LinkIndex = 0; LinkIndex < LINKS_IN_FIRST_SECTOR; LinkIndex++ )
        {

          First_Sector->Link_Table[LinkIndex] = LinkTable->LinkArray[Links_Moved].Link_Data;
          Links_Moved++;

        }

        /* Calculate the CRC. */
        First_Sector->Link_Table_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, First_Sector, BYTES_PER_SECTOR);

      }
      else
      {

        /* Locate the next sector within the buffer. */
        Offset = BYTES_PER_SECTOR * ( Sector_Count - 1 );
        Next_Sector = (LVM_Link_Table_Sector *) &Feature_Data_Buffer1;
        Next_Sector = (LVM_Link_Table_Sector *) ( (CARDINAL32) Next_Sector + Offset );

        /* Set the signature. */
        Next_Sector->Link_Table_Signature = LINK_TABLE_SIGNATURE;

        /* Set the sequence number. */
        Next_Sector->Sequence_Number = LinkTable->Sequence_Number + 1;

        /* Transfer Link Entries to the Sector. */
        for ( LinkIndex = 0; LinkIndex < LINKS_IN_NEXT_SECTOR; LinkIndex++ )
        {

          Next_Sector->Link_Table[LinkIndex] = LinkTable->LinkArray[Links_Moved].Link_Data;
          Links_Moved++;

        }

        /* Calculate the CRC. */
        Next_Sector->Link_Table_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, Next_Sector, BYTES_PER_SECTOR);

      }

    }

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Disk Image completed.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Set up the Commit_Data.  This structure is used to tell Write_Feature_Data whether or not to write drive linking data back
     to disk, and to give Write_Feature_Data the address of the volume record (VData) so that it can pass it to the next feature
     on each of the partitions in the volume we are processing.                                                                   */
  Commit_Data.Commit_Drive_Linking_Changes = ( LinkTable->ChangesMade || VolumeRecord->Convert_To_LVM_V1 );
  Commit_Data.VData = VData;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Attempting to write the disk image to each partition in the Aggregate.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now that we have created the Feature Data, we need to write it to disk. */
  LVM_Common_Services->ForEachItem(PartitionRecord->Feature_Data->Partitions, &Write_Feature_Data, &Commit_Data, TRUE, Error_Code);

  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"ForEachItem failed with DLIST error %d (decimal)", *Error_Code);
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* This should not happen because I/O errors are not reported back.  The disk array entry for the failed drive is marked instead. */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Commit_DL_Changes")

    return;

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Commit_DL_Changes")

  return;

}


/*--------------------------------------------------
 * Private functions available
 --------------------------------------------------*/

static void _System Create_Aggregates( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  /* Declare a local variable to walk the LVM Signature Sector Feature Array. */
  CARDINAL32   FeatureIndex;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  CARDINAL32 *    LVM_Error = ( CARDINAL32 *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access the LVM Signature Sector without having to use a double indirection each time. */
  LVM_Signature_Sector * Signature_Sector;

  /* We may need to create a new Partition Record.  Declare a local variable we can use to point to it. */
  Partition_Data * New_PartitionRecord = NULL;

  /* Declare an Aggregate Search Record.  We will need it to search the Aggregate List looking for an aggregate that this PartitionRecord may belong to. */
  Aggregate_Search_Record  Search_Record;

  /* We will have to manipulate a Drive Link Array at some point, so declare a variable to let us do so. */
  Drive_Link_Array  *  LinkArray;

  /* Declare a variable for use on error paths. */
  CARDINAL32  Ignore_Error;

  FEATURE_FUNCTION_ENTRY("Create_Aggregates")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *LVM_Error = LVM_ENGINE_INTERNAL_ERROR;
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Create_Aggregates")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type != Partition )
  {

    /* We have a corrupt list!  This function should only be used with the Partitions list for a volume.  The
       partitions list for a volume only contains entries which are for partitions, so if we get anything else,
       then the list has been corrupted!                                                                        */


#ifdef PARANOID

    assert(0);

#endif

      *LVM_Error = LVM_ENGINE_INTERNAL_ERROR;
      *Error_Code = DLIST_CORRUPTED;

      FEATURE_FUNCTION_EXIT("Create_Aggregates")

      return;

  }

#endif

  /* Assume success. */
  *Error_Code = DLIST_SUCCESS;
  *LVM_Error = LVM_ENGINE_NO_ERROR;

  /* Is this partition of interest to us?  It must have an LVM Signature Sector for us to be interested. */
  if ( PartitionRecord->Signature_Sector == NULL )
  {

    FEATURE_FUNCTION_EXIT("Create_Aggregates")

    /* This partition is not part of any linked drives.  */
    return;

  }

  /* Get the signature sector. */
  Signature_Sector = PartitionRecord->Signature_Sector;

  /* Is Drive Linking listed in the LVM Signature Sector? */
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == DRIVE_LINKING_FEATURE_ID )
    {

      /* Drive Linking is on this drive. */

      /* Create a Drive Link Array to hold the Drive Linking data for this partition. */
      LinkArray = (Drive_Link_Array *) LVM_Common_Services->Allocate( sizeof(Drive_Link_Array) );
      if ( LinkArray == NULL )
      {

        *Error_Code = DLIST_OUT_OF_MEMORY;
        *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;

        FEATURE_FUNCTION_EXIT("Create_Aggregates")

        return;

      }

      /* Initialize the Drive Link Array. */
      memset(LinkArray,0, sizeof(Drive_Link_Array) );

      /* Load the drive linking feature data for this partition. */
      if ( !Load_Feature_Data( PartitionRecord, LinkArray, LVM_Error) )
      {

        switch ( *LVM_Error )
        {

          case LVM_ENGINE_NO_ERROR :
                                     *Error_Code = DLIST_SUCCESS;
                                     break;
          case LVM_ENGINE_OUT_OF_MEMORY :
                                          *Error_Code = DLIST_OUT_OF_MEMORY;
                                          break;
          default : *Error_Code = DLIST_CORRUPTED;
                    break;
        }

        LVM_Common_Services->Deallocate(LinkArray);

        FEATURE_FUNCTION_EXIT("Create_Aggregates")

        return;

      }

      /* Is this partition part of an aggregate, or do we need to create a new one for it? */

      /* Set up to search the Aggreate List. */
      Search_Record.Aggregate_Found = FALSE;
      Search_Record.PartitionRecord = PartitionRecord;
      Search_Record.LinkTable = LinkArray;

      /* We must set the Aggregate_Serial_Number to search for.  Prior to Version 2 of Drive Linking, this was the volume
         serial number and there could only be one drive linking aggregate per volume.  With version 2 of drive linking,
         there can be several drive linking aggregates per volume, so each aggregate had to be given a special serial number
         to avoid problems.                                                                                                   */
      if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number > 1 )
        Search_Record.Aggregate_Serial_Number = LinkArray->Aggregate_Serial_Number;
      else
      {

        Search_Record.Aggregate_Serial_Number = Signature_Sector->Volume_Serial_Number;
        LinkArray->Aggregate_Serial_Number = Signature_Sector->Volume_Serial_Number;

      }

      /* Do the search. */
      LVM_Common_Services->ForEachItem(Aggregate_List, &Find_Existing_Aggregate, &Search_Record, TRUE, Error_Code );

#ifdef DEBUG

      /* Was there an error? */
      if ( *Error_Code != DLIST_SUCCESS )
      {

#ifdef PARANOID

        assert(0);

#else

        *LVM_Error = LVM_ENGINE_INTERNAL_ERROR;
        *Error_Code = DLIST_CORRUPTED;

        FEATURE_FUNCTION_EXIT("Create_Aggregates")

        return;

#endif

      }

#endif

      /* Was a matching Aggregate found? If it was, then the Partition was added to the aggregate.  If not, then we must create a new Aggregate. */
      if ( ! Search_Record.Aggregate_Found )
      {

        /* Since there was no existing Aggregate that this partition was a part of, create one. */
        New_PartitionRecord = (Partition_Data *) LVM_Common_Services->Allocate ( sizeof( Partition_Data ) );
        if ( New_PartitionRecord == NULL )
        {

          *Error_Code = DLIST_OUT_OF_MEMORY;
          *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;

          LVM_Common_Services->Deallocate(LinkArray);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

        /* Initialize those fields in the new PartitionRecord which we may need. */
        memset(New_PartitionRecord, 0, sizeof(Partition_Data) );
        New_PartitionRecord->External_Handle = NULL;
        New_PartitionRecord->Drive_Index = 0xFFFFFFFFL;/* We don't need the drive index as this is not a real partition. */
        New_PartitionRecord->Drive_Partition_Handle = NULL; /* This is not in the Partitions list of any entry in the DriveArray. */
        New_PartitionRecord->External_Volume_Handle = NULL;
        New_PartitionRecord->Volume_Handle = NULL;
        New_PartitionRecord->DLA_Table_Entry.Volume_Serial_Number = PartitionRecord->DLA_Table_Entry.Volume_Serial_Number;
        New_PartitionRecord->DLA_Table_Entry.Partition_Serial_Number = LinkArray->Aggregate_Serial_Number;
        New_PartitionRecord->DLA_Table_Entry.Drive_Letter = PartitionRecord->DLA_Table_Entry.Drive_Letter;
        strncpy(New_PartitionRecord->DLA_Table_Entry.Volume_Name, PartitionRecord->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);
        New_PartitionRecord->Partition_Type = Partition;

        /* Allocate memory for an LVM Signature Sector. */
        New_PartitionRecord->Signature_Sector = ( LVM_Signature_Sector * ) LVM_Common_Services->Allocate( BYTES_PER_SECTOR );
        if ( New_PartitionRecord->Signature_Sector == NULL )
        {

          *Error_Code = DLIST_OUT_OF_MEMORY;
          *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;

          LVM_Common_Services->Deallocate(LinkArray);
          LVM_Common_Services->Deallocate(New_PartitionRecord);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

        /* Initialize the Aggregate's LVM Signature Sector. */
        *(New_PartitionRecord->Signature_Sector) = *Signature_Sector;
        New_PartitionRecord->Signature_Sector->Partition_Serial_Number = LinkArray->Aggregate_Serial_Number;
        New_PartitionRecord->Signature_Sector->Partition_Start = 0;
        New_PartitionRecord->Signature_Sector->Partition_End = 0;
        New_PartitionRecord->Signature_Sector->Partition_Sector_Count = 0;
        New_PartitionRecord->Signature_Sector->LVM_Reserved_Sector_Count = 0;
        New_PartitionRecord->Signature_Sector->Partition_Size_To_Report_To_User = 0;
        New_PartitionRecord->Signature_Sector->Partition_Name[0] = 0x00;

        /* Allocate memory to hold the Feature Data for this Aggregate. */
        New_PartitionRecord->Feature_Data = ( Feature_Context_Data *) LVM_Common_Services->Allocate ( sizeof (Feature_Context_Data) );
        if ( New_PartitionRecord->Feature_Data == NULL )
        {

          *Error_Code = DLIST_OUT_OF_MEMORY;
          *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;

          LVM_Common_Services->Deallocate(LinkArray);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Signature_Sector);
          LVM_Common_Services->Deallocate(New_PartitionRecord);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

        /* Initialize the Aggregate's Feature Data. */
        memset( New_PartitionRecord->Feature_Data, 0, sizeof (Feature_Context_Data) );
        New_PartitionRecord->Feature_Data->Feature_ID = &Feature_ID_Record;
        New_PartitionRecord->Feature_Data->Function_Table = &Function_Table;
        New_PartitionRecord->Feature_Data->Data = NULL;
        New_PartitionRecord->Feature_Data->Partitions = NULL;
        New_PartitionRecord->Feature_Data->Old_Context = NULL;


        New_PartitionRecord->Feature_Data->Partitions = LVM_Common_Services->CreateList();
        if ( New_PartitionRecord->Feature_Data->Partitions == NULL )
        {

          *Error_Code = DLIST_OUT_OF_MEMORY;
          *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;

          LVM_Common_Services->Deallocate(LinkArray);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Signature_Sector);
          LVM_Common_Services->Deallocate(New_PartitionRecord);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

        /* Use the LinkArray of the partition as the LinkArray of the Aggregate. */
        New_PartitionRecord->Feature_Data->Data = (ADDRESS) LinkArray;

        /* Add the current partition to the partitions list of the aggregate. */
        LVM_Common_Services->InsertObject(New_PartitionRecord->Feature_Data->Partitions, sizeof(Partition_Data), PartitionRecord, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

        if ( *Error_Code != DLIST_SUCCESS )
        {

          if (*Error_Code == DLIST_OUT_OF_MEMORY)
            *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;
          else
            *LVM_Error = LVM_ENGINE_INTERNAL_ERROR;

          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data->Data);
          LVM_Common_Services->DestroyList(&(New_PartitionRecord->Feature_Data->Partitions), FALSE, &Ignore_Error);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Signature_Sector);
          LVM_Common_Services->Deallocate(New_PartitionRecord);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

        /* We must get an external handle for the aggregate. */
        New_PartitionRecord->External_Handle = LVM_Common_Services->Create_Handle(New_PartitionRecord, PARTITION_DATA_TAG, sizeof(Partition_Data), Error_Code);
        if ( *Error_Code != HANDLE_MANAGER_NO_ERROR )
        {

          if (*Error_Code == HANDLE_MANAGER_OUT_OF_MEMORY)
          {

            *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;
            *Error_Code = DLIST_OUT_OF_MEMORY;

          }
          else
          {

            *LVM_Error = LVM_ENGINE_INTERNAL_ERROR;
            *Error_Code = DLIST_CORRUPTED;

          }

          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data->Data);
          LVM_Common_Services->DestroyList(&(New_PartitionRecord->Feature_Data->Partitions), FALSE, &Ignore_Error);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Signature_Sector);
          LVM_Common_Services->Deallocate(New_PartitionRecord);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

        /* Add the Aggregate to the list of Aggregates. */
        LVM_Common_Services->InsertObject(Aggregate_List, sizeof(Partition_Data), New_PartitionRecord, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

        if ( *Error_Code != DLIST_SUCCESS )
        {

          if (*Error_Code == DLIST_OUT_OF_MEMORY)
            *LVM_Error = LVM_ENGINE_OUT_OF_MEMORY;
          else
            *LVM_Error = LVM_ENGINE_INTERNAL_ERROR;

          LVM_Common_Services->Destroy_Handle(New_PartitionRecord->External_Handle, &Ignore_Error);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data->Data);
          LVM_Common_Services->DestroyList(&(New_PartitionRecord->Feature_Data->Partitions), FALSE, &Ignore_Error);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Feature_Data);
          LVM_Common_Services->Deallocate(New_PartitionRecord->Signature_Sector);
          LVM_Common_Services->Deallocate(New_PartitionRecord);

          FEATURE_FUNCTION_EXIT("Create_Aggregates")

          return;

        }

      }

      /* Leave the FOR loop as we found what we were looking for. */
      break;

    }

  }

  /* Indicate success and leave. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Create_Aggregates")

  return;

}


static void _System Find_Existing_Aggregate(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
   Aggregate_Search_Record *  Search_Record = ( Aggregate_Search_Record *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * Aggregate = (Partition_Data *) Object;

  /* Declare a local variable to access the PartitionRecord we are trying to match. */
  Partition_Data * PartitionRecord = Search_Record->PartitionRecord;

  /* We will have to manipulate a Drive Link Array at some point, so declare a variable to let us do so. */
  Drive_Link_Array  *  LinkArray;

  /* We will need to compare some of the fields in the LVM Signature Sector of the Aggregate with those in the PartitionRecord we are seeking to match. */
  LVM_Signature_Sector * Signature_Sector_A, * Signature_Sector_P;

  /* Declare a variable to walk the LinkArray when we are comparing the feature data of the partition against that of the aggregate. */
  CARDINAL32  Index;

  FEATURE_FUNCTION_ENTRY("Find_Existing_Aggregate")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

    return ;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

#ifdef DEBUG

  /* Does this partition record represent a partition? */
  if ( Aggregate->Partition_Type != Partition )
  {

    /* We have a corrupt list!  This function should only be used with the Partitions list for a volume.  The
       partitions list for a volume only contains entries which are for partitions, so if we get anything else,
       then the list has been corrupted!                                                                        */


#ifdef PARANOID

    assert(0);

#endif

      *Error_Code = DLIST_CORRUPTED;

      FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

      return ;

  }

#endif

  /* Does the Volume Serial Number of the Aggregate match the Volume Serial Number of the PartitionRecord we have been given? */
  if ( PartitionRecord->DLA_Table_Entry.Volume_Serial_Number == Aggregate->DLA_Table_Entry.Volume_Serial_Number )
  {

    /* Do the drive preferences match? */
    if ( PartitionRecord->DLA_Table_Entry.Drive_Letter == Aggregate->DLA_Table_Entry.Drive_Letter )
    {

      /* Get the LVM Signature Sectors for the Aggregate and the Partition. */
      Signature_Sector_A = Aggregate->Signature_Sector;
      Signature_Sector_P = PartitionRecord->Signature_Sector;

      /* Do the appropriate fields in the Signature Sectors match? */
      if ( Signature_Sector_A->Boot_Disk_Serial_Number == Signature_Sector_P->Boot_Disk_Serial_Number )
      {

        /* Get the Drive Linking Feature Data for the aggregate and compare it against that for the partition. */
        LinkArray = (Drive_Link_Array * ) Aggregate->Feature_Data->Data;

        /* Does the LinkTable for the partition match the LinkArray for the Aggregate? */
        if ( ( LinkArray->Links_In_Use != Search_Record->LinkTable->Links_In_Use ) ||
             ( LinkArray->Sequence_Number != Search_Record->LinkTable->Sequence_Number ) ||
             ( LinkArray->Aggregate_Serial_Number != Search_Record->Aggregate_Serial_Number )
           )
        {

          FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

          /* This partition does not belong to this aggregate. */
          return;

        }

        /* Compare the partitions being linked. */
        for ( Index = 0; Index < MAXIMUM_LINKS; Index++ )
        {

          if ( ( LinkArray->LinkArray[Index].Link_Data.Drive_Serial_Number != Search_Record->LinkTable->LinkArray[Index].Link_Data.Drive_Serial_Number ) ||
               ( LinkArray->LinkArray[Index].Link_Data.Partition_Serial_Number != Search_Record->LinkTable->LinkArray[Index].Link_Data.Partition_Serial_Number )
             )
          {

            FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

            /* This partition does not belong to this aggregate. */
            return;

          }

        }

        /* It looks like this PartitionRecord belongs to this Aggregate. */

        /* Add this PartitionRecord to the Aggregrate. */
        LVM_Common_Services->InsertObject(Aggregate->Feature_Data->Partitions, sizeof(Partition_Data), PartitionRecord, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code );

        /* Did we succeed? */
        if ( *Error_Code != DLIST_SUCCESS )
        {

          FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

          return;

        }

        /* Find the partition in the link data for the aggregate and set a pointer to it. */
        for ( Index = 0; Index < MAXIMUM_LINKS; Index++ )
        {

          /* The only entry in the LinkTable of the partition record that will have a pointer to a partition record is
             the entry that points to itself.  We need to find that entry and update the corresponding entry in the
             LinkArray of the Aggregate.                                                                                */
          if ( Search_Record->LinkTable->LinkArray[Index].PartitionRecord != NULL )
          {

            LinkArray->LinkArray[Index].PartitionRecord = Search_Record->LinkTable->LinkArray[Index].PartitionRecord;

          }

        }


        /* Now indicate that a match was found. */
        Search_Record->Aggregate_Found = TRUE;

        /* Indicate that we found what we were looking for and return. */
        *Error_Code = DLIST_SEARCH_COMPLETE;

        FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

        return;

      }

    }

  }

  /* Indicate success and return. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Find_Existing_Aggregate")

  return;

}


static BOOLEAN  _System Eliminate_Bad_Aggregates( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code)
{

  Partition_Data *               Aggregate = (Partition_Data *) Object;
  Partition_Data *               NextPartitionRecord;
  Aggregate_Validation_Record *  Validation_Record = ( Aggregate_Validation_Record *) Parameters;
  Partition_Data *               PartitionRecord;
  Drive_Link_Array *             LinkTable;
  Extended_Boot_Record *         Fake_EBR  = (Extended_Boot_Record *) &Fake_EBR_Buffer;
  CARDINAL32                     Ignore_Error;
  CARDINAL32                     Index;
  Plugin_Function_Table_V1 *     Old_Function_Table;


  FEATURE_FUNCTION_ENTRY("Eliminate_Bad_Aggregates")

  /* To see if an aggregate is incomplete, we just need to check its feature data.  The number of links in use should match
     the number of partitions in the Partitions list for the Aggregate.  If it does not, then something is missing.  Since
     what is missing could come from another aggregator, we must put the partitions associated with an incomplete aggregate
     back into the master list of partitions so that we may retry this operation later, at which time we hope that all of the
     pieces will be available.                                                                                                 */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;
  if ( LVM_Common_Services->GetListSize(Aggregate->Feature_Data->Partitions, Error_Code) != LinkTable->Links_In_Use )
  {

    /* Free any memory held by the Aggregate. */
    LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
    LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
    LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
    LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
    LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

    /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
    if ( ! Validation_Record->Volume_Created )
      *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

    /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
    *Error_Code = DLIST_SUCCESS;

    FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

    return TRUE;

  }

  /* We will be keeping this Aggregate. */

  /* We must build the partitions list for the aggregate with the partitions in the list appearing in the order that we
     use them.  This is to support the Sequence_Number feature of the LVM Signature Sector, which is necessary for the
     proper operation of OS2LVM.DMD under certain circumstances.  We will due this by first deleting all items in the
     partition list for the aggregate, and then adding the partitions back in the order they appear in the link table.
     At the same time, we will remove the partitions from the partitions list.                                            */
  LVM_Common_Services->DeleteAllItems( Aggregate->Feature_Data->Partitions, FALSE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have an internal error!  We have used this list before without error, so this function should not have failed! */
    *(Validation_Record->Error_Code) = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

    return TRUE;

  }

  /* Now walk the LinkArray and put the partitions back into the Partitions list of the aggregate. */
  for ( Index = 0; Index < LinkTable->Links_In_Use; Index++)
  {

    /* Insert the current partition into the Partitions list of the aggregate. */
    LVM_Common_Services->InsertObject(Aggregate->Feature_Data->Partitions, sizeof(Partition_Data), LinkTable->LinkArray[Index].PartitionRecord, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      {

        /* Free any memory held by the Aggregate. */
        LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
        LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
        LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
        LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
        LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

      }

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      return TRUE;

    }

    /* We need to find this partition in the partitions list and delete it. */
    LVM_Common_Services->PruneList(Validation_Record->Partition_List, &Claim_Aggregate_Partitions, LinkTable->LinkArray[Index].PartitionRecord, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      return TRUE;

    }

  }

  /* Lets calculate the size of the Aggregate. */
  LVM_Common_Services->ForEachItem(Aggregate->Feature_Data->Partitions, &Calculate_Aggregate_Size, &(Aggregate->Partition_Size), TRUE, Error_Code);

  /* Did we succeed? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have an internal error!  We have already successfully walked this list before, so it should not fail now! */
    *(Validation_Record->Error_Code) = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

    return FALSE;

  }

  /* Since this is a new aggregate, set its usable size to the partition size. */
  Aggregate->Usable_Size = Aggregate->Partition_Size;

  /* Get the Drive Link Array.  */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;

  /* Is this a spanned volume? */
  if ( LinkTable->Links_In_Use > 1 )
  {

    /* This is a spanned volume!  Set the Spanned_Volume flag on all of the partitions which are a part of this aggregate. */
    LVM_Common_Services->ForEachItem(Aggregate->Feature_Data->Partitions, &Set_Spanned_Volume_Flag, NULL, TRUE, Error_Code);

    /* Did we succeed? */
    if ( *Error_Code != DLIST_SUCCESS )
    {

      /* We have an internal error!  We have already successfully walked this list before, so it should not fail now! */
      *(Validation_Record->Error_Code) = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      return FALSE;

    }

  }

  /* If we find that an aggregate is good, then, if the aggregate is a drive linking version 2
     aggregate, we must try to read in an LVM signature sector from the end of the aggregate.
     Any LVM Features which come after us will be expecting this.                                 */
  if ( LinkTable->Aggregate_Signature_Sector_Expected )
  {

    /* Since this aggregate is good, lets see if it has a signature sector. */
    DL_Read(Aggregate, Aggregate->Partition_Size - 1, 1, Aggregate->Signature_Sector, Error_Code);

    /* Did we succeed? */
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      if ( *Error_Code != LVM_ENGINE_IO_ERROR )
      {

        *Error_Code = DLIST_CORRUPTED;

        FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

        return FALSE;

      }

      /* Free any memory held by the Aggregate. */
      LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      *Error_Code = DLIST_SUCCESS;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      return TRUE;

    }

    /* Since we were able to read the sector, lets validate it. */
    if ( ! LVM_Common_Services->Valid_Signature_Sector( Aggregate, Aggregate->Signature_Sector) )
    {

      /* Free any memory held by the Aggregate. */
      LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      *Error_Code = DLIST_SUCCESS;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      return TRUE;

    }

    /* Update the Usable Size based upon the number of reserved sectors in the LVM Signature Sector. */
    Aggregate->Usable_Size = Aggregate->Signature_Sector->Partition_Size_To_Report_To_User;

    /* Save the name of the aggregate, if there is one. */
    strncpy(Aggregate->Partition_Name, Aggregate->Signature_Sector->Partition_Name, PARTITION_NAME_SIZE);

    /* Is there a fake EBR associated with this aggregate? */
    if ( Aggregate->Signature_Sector->Fake_EBR_Allocated )
    {

      /* We must read in the fake EBR and extract the format indicator for the volume.  */
      DL_Read(Aggregate, Aggregate->Signature_Sector->Fake_EBR_Location, 1, &Fake_EBR_Buffer, Error_Code);

      /* Did we succeed? */
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        if ( *Error_Code != LVM_ENGINE_IO_ERROR )
        {

          *Error_Code = DLIST_CORRUPTED;

          FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

          return FALSE;

        }

        /* Since we could not read the Fake EBR, we will create a new partition table entry for this aggregate.  The only
           data in the partition table entry that may not be correct is the Format_Indicator.  The new partition table entry
           will have a format indicator of 7.  This will work with FAT, HPFS, and JFS.  Other filesystems may not like it, though. */
        LVM_Common_Services->Create_Fake_Partition_Table_Entry( &(Aggregate->Partition_Table_Entry), Aggregate->Usable_Size );

      }
      else
      {

        /* We must extract the partition table entry for this aggregate from the "fake" EBR. */
        Aggregate->Partition_Table_Entry = Fake_EBR->Partition_Table[0];

      }

    }

  }
  else
  {

    /* Since this was an LVM Version 1 aggregate, there is no signature sector associated with it.  Free the
       default signature sector that was created by the Create_Aggregates function.                           */
    LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
    Aggregate->Signature_Sector = NULL;

    /* The fake EBR for this aggregate (and the volume) is located in the LVM Data Area of the first partition in the
       aggregate.  We must load this fake EBR and extract the partition table entry for the aggregate/volume.          */

    /* Is the fake EBR there? */
    if ( ! ( LinkTable->LinkArray[0].PartitionRecord->Signature_Sector->Fake_EBR_Allocated ) )
    {

      /* Free any memory held by the Aggregate. */
      LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      *Error_Code = DLIST_SUCCESS;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      return TRUE;

    }

    /* Since we think the fake EBR exists, lets try to read it in. */

    /* To read in the fake EBR, we must use the Read function from the function table of the partition. */
    if ( LinkTable->LinkArray[0].PartitionRecord->Feature_Data == NULL )
    {

      /* Free any memory held by the Aggregate. */
      LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      *Error_Code = DLIST_SUCCESS;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      return TRUE;

    }

    /* Is the function table for the next layer on the partition available? */
    Old_Function_Table =  LinkTable->LinkArray[0].PartitionRecord->Feature_Data->Function_Table;
    if ( Old_Function_Table == NULL )
    {

      /* Free any memory held by the Aggregate. */
      LVM_Common_Services->Destroy_Handle(Aggregate->External_Handle,&Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data->Data);
      LVM_Common_Services->Deallocate(Aggregate->Signature_Sector);
      LVM_Common_Services->DestroyList( &(Aggregate->Feature_Data->Partitions), FALSE, &Ignore_Error);
      LVM_Common_Services->Deallocate(Aggregate->Feature_Data);

      /* If we have not already created a volume, then set *(Validation_Record.Error_Code) to LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE. */
      if ( ! Validation_Record->Volume_Created )
        *(Validation_Record->Error_Code) = LVM_ENGINE_PLUGIN_OPERATION_INCOMPLETE;

      /* Returning TRUE will cause Aggregate to be removed from the Aggregate_List. */
      *Error_Code = DLIST_SUCCESS;

      FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

      return TRUE;

    }

    /* Now call the Read function of the PartitionRecord to complete the read operation. */
    Old_Function_Table->Read(LinkTable->LinkArray[0].PartitionRecord, LinkTable->LinkArray[0].PartitionRecord->Signature_Sector->Fake_EBR_Location, 1, &Fake_EBR_Buffer , Error_Code );

    /* Did we succeed? */
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      if ( *Error_Code != LVM_ENGINE_IO_ERROR )
      {

        *Error_Code = DLIST_CORRUPTED;

        FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

        return FALSE;

      }

      /* Since we could not read the Fake EBR, we will create a new partition table entry for this aggregate.  The only
         data in the partition table entry that may not be correct is the Format_Indicator.  The new partition table entry
         will have a format indicator of 7.  This will work with FAT, HPFS, and JFS.  Other filesystems may not like it, though. */
      LVM_Common_Services->Create_Fake_Partition_Table_Entry( &(Aggregate->Partition_Table_Entry), Aggregate->Usable_Size );

    }
    else
    {

      /* We must extract the partition table entry for this aggregate from the "fake" EBR. */
      Aggregate->Partition_Table_Entry = Fake_EBR->Partition_Table[0];

    }

  }

  /* We need to get the filesystem name for the aggregate.  This should come from the first partition in the link array. */

  /* Get the Partition Record for the first link in the Volume. */
  PartitionRecord = LinkTable->LinkArray[0].PartitionRecord;

  /* Copy the filesystem name. */
  strncpy(Aggregate->File_System_Name, PartitionRecord->File_System_Name, FILESYSTEM_NAME_SIZE);

  /* If the Aggregate has more than 1 partition in it, we must set the filesystem name for each of the partitions after the
     first.  The filesystem name is only stored in the Boot Sector of the first partition in the linked volume.              */
  if ( LinkTable->Links_In_Use > 1 )
  {

    for ( Index = 1; Index < LinkTable->Links_In_Use; Index++ )
    {

      /* Get the partition record of this link. */
      NextPartitionRecord = LinkTable->LinkArray[Index].PartitionRecord;

      /* Copy the filesystem name. */
      strncpy(NextPartitionRecord->File_System_Name, PartitionRecord->File_System_Name, FILESYSTEM_NAME_SIZE);

      /* Increment the Feature_Index to indicate that we have completed the current feature on this partition. */
      NextPartitionRecord->Feature_Index += 1;

    }

  }

  /* Add the aggregate to the list of aggregates. */
  Aggregate->Drive_Partition_Handle = LVM_Common_Services->InsertObject(LVM_Common_Services->Aggregates, sizeof(Partition_Data), Aggregate, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

    /* We may be out of memory, or worse! */
    return FALSE;

  }


  /* Since we got here, we will be keeping this Aggregate.  */
  Validation_Record->Volume_Created = TRUE;
  *(Validation_Record->Error_Code) = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("Eliminate_Bad_Aggregates")

  return FALSE;

}


static BOOLEAN _System Load_Feature_Data(Partition_Data * PartitionRecord, Drive_Link_Array * LinkTable, CARDINAL32 * Error_Code)
{

  CARDINAL32                       TargetSector;
  CARDINAL32                       FeatureIndex;
  CARDINAL32                       LinkIndex;
  LVM_Signature_Sector *           Signature_Sector;
  LVM_Link_Table_First_Sector *    First_Copy;
  LVM_Link_Table_First_Sector *    Second_Copy;
  Plugin_Function_Table_V1 *       Old_Function_Table;
  BOOLEAN                          Primary_Copy_Valid;
  BOOLEAN                          Secondary_Copy_Valid;
  BOOLEAN                          DL_Version1_Found = FALSE;

  FEATURE_FUNCTION_ENTRY("Load_Feature_Data")

  /* Get the signature sector. */
  Signature_Sector = PartitionRecord->Signature_Sector;

  /* Find the Drive Linking entry in the LVM Signature Sector. */
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == DRIVE_LINKING_FEATURE_ID )
    {

      /* Is this feature data for version 1 of Drive Linking? */
      if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number < 2 )
      {

        LOG_FEATURE_EVENT("Drive Linking Version 1 Feature Data located.")

        DL_Version1_Found = TRUE;

      }
      else
      {

        LOG_FEATURE_EVENT("Drive Linking Version 2 Feature Data located.")

      }

      /* Find the location of the feature data for Drive Linking from the LVM_Signature_Sector. */
      TargetSector = Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data;

      /* Read in the feature data for drive linking. */
      Old_Function_Table = (Plugin_Function_Table_V1 *) PartitionRecord->Feature_Data->Function_Table;
      Old_Function_Table->Read(PartitionRecord, TargetSector, DRIVE_LINKING_RESERVED_SECTOR_COUNT, &Feature_Data_Buffer1, Error_Code);

      /* Did we succeed? */
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        LOG_FEATURE_EVENT1("Error reading the primary copy of the feature data!", "Error code", *Error_Code)

        if ( *Error_Code != LVM_ENGINE_IO_ERROR )
        {

          FEATURE_FUNCTION_EXIT("Load_Feature_Data")

          return FALSE;

        }
        else
        {

          Primary_Copy_Valid = FALSE;

        }

      }
      else
      {

        /* Validate each of the sectors read in. */
        if ( ! Feature_Data_Is_Valid( TRUE ) )
        {

          /* The data for this partition is not valid.  */
          Primary_Copy_Valid = FALSE;

        }
        else
          Primary_Copy_Valid = TRUE;

      }

      /* Now read in the second copy of the Feature Data. */
      /* Find the location of the feature data for Drive Linking from the LVM_Signature_Sector. */
      TargetSector = Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data;

      /* Read in the feature data for drive linking. */
      Old_Function_Table->Read(PartitionRecord, TargetSector, DRIVE_LINKING_RESERVED_SECTOR_COUNT, &Feature_Data_Buffer2, Error_Code);

      /* Did we succeed? */
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        LOG_FEATURE_EVENT1("Error reading the secondary copy of the feature data!","Error code", *Error_Code)

        if ( *Error_Code != LVM_ENGINE_IO_ERROR )
        {

          FEATURE_FUNCTION_EXIT("Load_Feature_Data")

          return FALSE;

        }
        else
        {

          Secondary_Copy_Valid = FALSE;

        }

      }
      else
      {

        /* Validate each of the sectors read in. */
        if ( ! Feature_Data_Is_Valid( FALSE ) )
        {

          /* The data for this partition is not valid. */
          Secondary_Copy_Valid = FALSE;

        }
        else
          Secondary_Copy_Valid = TRUE;

      }

      /* Do we have a valid copy? */
      if ( ( ! Primary_Copy_Valid ) && ( ! Secondary_Copy_Valid ) )
      {

        LOG_FEATURE_ERROR("No valid feature data found!")

        /* There is no valid copy of the feature data for this partition! */

        *Error_Code = LVM_ENGINE_NO_ERROR;

        FEATURE_FUNCTION_EXIT("Load_Feature_Data")

        return FALSE;

      }

      /* Are both copies valid? */
      if ( Primary_Copy_Valid && Secondary_Copy_Valid )
      {

        /* Since both copies are valid, do they agree? */
        if ( memcmp(&Feature_Data_Buffer1, &Feature_Data_Buffer2, BYTES_PER_SECTOR * DRIVE_LINKING_RESERVED_SECTOR_COUNT) != 0 )
        {

          LOG_FEATURE_EVENT("Both copies of the feature data appear to be valid, but they do not match!")

          /* They do not agree!  Which one should we use?  We will use the one with the higher Sequence Count. */

          /* Establish access to the buffers. */
          First_Copy = ( LVM_Link_Table_First_Sector *) &Feature_Data_Buffer1;
          Second_Copy = ( LVM_Link_Table_First_Sector *) &Feature_Data_Buffer2;

          /* Which has the higher sequence count? */
          if ( First_Copy->Sequence_Number > Second_Copy->Sequence_Number )
          {

            LOG_FEATURE_EVENT("Using the first copy of the feature data (higher sequence number).")

            /* We will use the first copy. */

            /* Does this copy match up with the Link Table we already have for the Aggregate? */
            if ( ! Link_Tables_Match( LinkTable, TRUE, TRUE) )
            {

              /* There is no valid copy of the feature data for this partition! */

              *Error_Code = LVM_ENGINE_NO_ERROR;

              FEATURE_FUNCTION_EXIT("Load_Feature_Data")

              return FALSE;

            }

          }
          else
          {

            if ( First_Copy->Sequence_Number < Second_Copy->Sequence_Number )
            {

              LOG_FEATURE_EVENT("Using the second copy of the feature data (higher sequence number).")

              /* We will use the second copy. */
              if ( ! Link_Tables_Match( LinkTable, TRUE, FALSE) )
              {

                /* There is no valid copy of the feature data for this partition! */
                *Error_Code = LVM_ENGINE_NO_ERROR;

                FEATURE_FUNCTION_EXIT("Load_Feature_Data")

                return FALSE;

              }

            }
            else
            {

              LOG_FEATURE_ERROR("The sequence numbers match!  Unable to choose which copy of the feature data to use!")

              /* This should never happen!  Something is screwed up royal! */
              *Error_Code = LVM_ENGINE_NO_ERROR;

              FEATURE_FUNCTION_EXIT("Load_Feature_Data")

              return FALSE;

            }

          }

        }
        else
        {

          /* Since both copies are the same, we will use the first copy. */

          /* Does this copy match up with the Link Table we already have for the Aggregate? */
          if ( ! Link_Tables_Match( LinkTable, TRUE, TRUE) )
          {

            /* There is no valid copy of the feature data for this partition! */
            *Error_Code = LVM_ENGINE_NO_ERROR;

            FEATURE_FUNCTION_EXIT("Load_Feature_Data")

            return FALSE;

          }

        }

      }
      else
      {

        /* Since both copies were not valid, use the one that is. */
        if ( Primary_Copy_Valid )
        {

          LOG_FEATURE_EVENT("Using the primary copy of the feature data.")

          /* We will use the first copy. */

          /* Does this copy match up with the Link Table we already have for the Aggregate? */
          if ( ! Link_Tables_Match( LinkTable, TRUE, TRUE) )
          {

            /* There is no valid copy of the feature data for this partition! */
            *Error_Code = LVM_ENGINE_NO_ERROR;

            FEATURE_FUNCTION_EXIT("Load_Feature_Data")

            return FALSE;

          }

        }
        else
        {

          LOG_FEATURE_EVENT("Using the secondary copy of the feature data.")

          /* We will use the second copy. */
          if ( ! Link_Tables_Match( LinkTable, TRUE, FALSE) )
          {

            /* There is no valid copy of the feature data for this partition! */
            *Error_Code = LVM_ENGINE_NO_ERROR;

            FEATURE_FUNCTION_EXIT("Load_Feature_Data")

            return FALSE;

          }

        }

        LOG_FEATURE_EVENT("The link table has been marked dirty so that both copies of the feature data may be synchronized.")

        /* Mark the LinkTable as having had changes made so that the two copies of the data will be synchronized if
           a commit operation is done.                                                                                      */
        LinkTable->ChangesMade = TRUE;

      }

      /* We must find this Partition Record in the Link Table and place a pointer to it there. */
      for ( LinkIndex = 0; LinkIndex < MAXIMUM_LINKS; LinkIndex++)
      {

        if ( ( LinkTable->LinkArray[LinkIndex].Link_Data.Drive_Serial_Number == LVM_Common_Services->DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number ) &&
             ( LinkTable->LinkArray[LinkIndex].Link_Data.Partition_Serial_Number == PartitionRecord->DLA_Table_Entry.Partition_Serial_Number )
           )
        {

          /* We have found our position in the Link Table.  Place a pointer to the PartitionRecord in the Link Table. */
          LinkTable->LinkArray[LinkIndex].PartitionRecord = PartitionRecord;

          /* We found what we were looking for.  Stop the search. */
          break;

        }

      }

      if ( LinkIndex >= MAXIMUM_LINKS )
      {

        LOG_FEATURE_ERROR("A partition claimed to be part of the current volume, but was not found in the link table!")

        /* This partition claims to be part of this aggregate, but it does not appear in the Link Table for this aggregate! */
        *Error_Code = LVM_ENGINE_NO_ERROR;

        FEATURE_FUNCTION_EXIT("Load_Feature_Data")

        return FALSE;

      }

      /* We found what we wanted.  End the search. */
      break;

    }

  }

  /* We must adjust the LinkTable if the data in this LinkTable was created by Version 1 of Drive Linking.
     Version 1 of Drive Linking did not store all of the information that Version 2 and later did.              */
  if ( DL_Version1_Found )
  {

    /* Version 1 of Drive Linking did not use LVM Signature Sectors for its aggregates. */
    LinkTable->Aggregate_Signature_Sector_Expected = FALSE;

    /* The following data is not stored or used by drive linking version 1, but is needed by this
       version of drive linking and LVM.  Since this data is fixed for drive linking version 1, we
       can just hard code it here as it will not change.                                             */
    LinkTable->Actual_Class = Aggregate_Class;
    LinkTable->Top_Of_Class = TRUE;
    LinkTable->Feature_Sequence_Number = 2;

  }
  else
    LinkTable->Aggregate_Signature_Sector_Expected = TRUE;  /* Since this aggregate was not created by version 1 of drive linking, an LVM Signature Sector is expected. */

  /* Indicate success and return. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Load_Feature_Data")

  return TRUE;

}


static BOOLEAN Feature_Data_Is_Valid( BOOLEAN Primary )
{

  CARDINAL32                       Sector_Count;
  LVM_Link_Table_First_Sector *    First_Sector;
  LVM_Link_Table_Sector *          Next_Sector;
  ADDRESS                          Buffer;
  CARDINAL32                       Offset;
  CARDINAL32                       Old_CRC;
  CARDINAL32                       Calculated_CRC;
  CARDINAL32                       Sequence_Count;

  FEATURE_FUNCTION_ENTRY("Feature_Data_Is_Valid")

  /* Select which buffer to validate. */
  if ( Primary )
    Buffer = &Feature_Data_Buffer1;
  else
    Buffer = &Feature_Data_Buffer2;

  /* Validate each sector of data contained in the buffer. */
  for ( Sector_Count = 1; Sector_Count < DRIVE_LINKING_RESERVED_SECTOR_COUNT; Sector_Count++)
  {

    /* The first sector is different than the ones that follow it.  Handle it separately. */
    if ( Sector_Count == 1)
    {

      /* Find the location of the first sector in the buffer. */
      First_Sector = (LVM_Link_Table_First_Sector *) Buffer;

      /* Check the signature. */
      if ( First_Sector->Link_Table_Signature != LINK_TABLE_MASTER_SIGNATURE )
      {

        LOG_FEATURE_ERROR("The link table master signature is invalid!  The feature data is not valid!")

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the Links_In_Use field.  It must be smaller than the maximum number of links allowed. */
      if ( First_Sector->Links_In_Use >= MAXIMUM_LINKS )
      {

        LOG_FEATURE_ERROR("The links in use count is invalid!  The feature data is not valid!")

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the CRC. */
      Old_CRC = First_Sector->Link_Table_CRC;
      First_Sector->Link_Table_CRC = 0;

      Calculated_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, First_Sector, BYTES_PER_SECTOR);

      if ( Calculated_CRC != Old_CRC )
      {

        LOG_FEATURE_ERROR("The CRC of the feature data is invalid!  The feature data is not valid!")

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
      Next_Sector = (LVM_Link_Table_Sector *) Buffer;
      Next_Sector = (LVM_Link_Table_Sector *) ( (CARDINAL32) Next_Sector + Offset );

      /* Check the signature. */
      if ( Next_Sector->Link_Table_Signature != LINK_TABLE_SIGNATURE )
      {

        LOG_FEATURE_ERROR("The link table signature is invalid!  The feature data is not valid!")

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the CRC. */
      Old_CRC = Next_Sector->Link_Table_CRC;
      Next_Sector->Link_Table_CRC = 0;

      Calculated_CRC = LVM_Common_Services->CalculateCRC( LVM_Common_Services->Initial_CRC, Next_Sector, BYTES_PER_SECTOR);

      if ( Calculated_CRC != Old_CRC )
      {

        LOG_FEATURE_ERROR("The link table CRC is invalid!  The feature data is not valid!")

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

      /* Check the Sequence Number. */
      if ( Next_Sector->Sequence_Number != Sequence_Count )
      {

        LOG_FEATURE_ERROR("The link table sequence count is invalid!  The feature data is not valid!")

        FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

        /* The data in the buffer is not valid! */
        return FALSE;

      }

    }

  }

  FEATURE_FUNCTION_EXIT("Feature_Data_Is_Valid")

  return TRUE;

}

static BOOLEAN  Link_Tables_Match(Drive_Link_Array * LinkTable, BOOLEAN Copy_Instead, BOOLEAN Use_Primary_Buffer)
{

  CARDINAL32                       Sector_Count;
  LVM_Link_Table_First_Sector *    First_Sector;
  LVM_Link_Table_Sector *          Next_Sector;
  CARDINAL32                       Index = 0;
  CARDINAL32                       Sector_Link_Table_Index;
  ADDRESS                          Buffer;
  CARDINAL32                       Offset;

  FEATURE_FUNCTION_ENTRY("Link_Tables_Match")

  /* Select which buffer to validate. */
  if ( Use_Primary_Buffer )
    Buffer = &Feature_Data_Buffer1;
  else
    Buffer = &Feature_Data_Buffer2;

  /* Compare the Drive Links contained in each sector of data contained in the buffer against our in memory copy. */
  for ( Sector_Count = 1; (Sector_Count <= DRIVE_LINKING_RESERVED_SECTOR_COUNT) && ( Index < MAXIMUM_LINKS ); Sector_Count++)
  {

    /* The first sector is different than the ones that follow it.  Handle it separately. */
    if ( Sector_Count == 1)
    {

      /* Find the location of the first sector in the buffer. */
      First_Sector = (LVM_Link_Table_First_Sector *) Buffer;

      if ( Copy_Instead )
      {

        /* Save the sequence number, the number of links in use, and the Aggregate Serial Number. */
        LinkTable->Sequence_Number = First_Sector->Sequence_Number;
        LinkTable->Aggregate_Serial_Number = First_Sector->Aggregate_Serial_Number;
        LinkTable->Actual_Class = First_Sector->Actual_Class;
        LinkTable->Top_Of_Class = First_Sector->Top_Of_Class;
        LinkTable->Feature_Sequence_Number = First_Sector->Feature_Sequence_Number;
        LinkTable->Links_In_Use = First_Sector->Links_In_Use;

      }
      else
      {

        if ( ( LinkTable->Sequence_Number != First_Sector->Sequence_Number) ||
             ( LinkTable->Aggregate_Serial_Number != First_Sector->Aggregate_Serial_Number ) ||
             ( LinkTable->Links_In_Use != First_Sector->Links_In_Use )
           )
        {

          LOG_FEATURE_ERROR("The link table metadata does not match!  The link tables do NOT match!")

          FEATURE_FUNCTION_EXIT("Link_Tables_Match")

          return FALSE;

        }

      }

      /* Compare/Copy the contents of the Link Table in the sector against the correct portion of the one in memory. */
      for ( Sector_Link_Table_Index = 0; Sector_Link_Table_Index < LINKS_IN_FIRST_SECTOR; Sector_Link_Table_Index++ )
      {

        if ( Copy_Instead )
        {

          /* Copy the links instead of comparing them. */
          LinkTable->LinkArray[Index].Link_Data.Drive_Serial_Number = First_Sector->Link_Table[Sector_Link_Table_Index].Drive_Serial_Number;
          LinkTable->LinkArray[Index].Link_Data.Partition_Serial_Number = First_Sector->Link_Table[Sector_Link_Table_Index].Partition_Serial_Number;

        }
        else
        {

          /* Compare the links in the sector with those in the table. */
          if ( ( First_Sector->Link_Table[Sector_Link_Table_Index].Drive_Serial_Number != LinkTable->LinkArray[Index].Link_Data.Drive_Serial_Number ) ||
               ( First_Sector->Link_Table[Sector_Link_Table_Index].Partition_Serial_Number != LinkTable->LinkArray[Index].Link_Data.Partition_Serial_Number )
             )
          {

            LOG_FEATURE_ERROR("The link table entries do not match!  The link tables do NOT match!")

            FEATURE_FUNCTION_EXIT("Link_Tables_Match")

            /* The link entries do not match! */
            return FALSE;

          }

        }

        Index++;

      }

    }
    else
    {

      /* Locate the next sector within the buffer. */
      Offset = BYTES_PER_SECTOR * ( Sector_Count - 1 );
      Next_Sector = (LVM_Link_Table_Sector *) Buffer;
      Next_Sector = (LVM_Link_Table_Sector *) ( (CARDINAL32) Next_Sector + Offset );

      /* Compare the contents of the Link Table in the sector against the correct portion of the one in memory. */
      for ( Sector_Link_Table_Index = 0; Sector_Link_Table_Index < LINKS_IN_NEXT_SECTOR; Sector_Link_Table_Index++ )
      {

        if ( Copy_Instead )
        {

          /* Copy the link instead of comparing the link. */
          LinkTable->LinkArray[Index].Link_Data.Drive_Serial_Number = Next_Sector->Link_Table[Sector_Link_Table_Index].Drive_Serial_Number;
          LinkTable->LinkArray[Index].Link_Data.Partition_Serial_Number = Next_Sector->Link_Table[Sector_Link_Table_Index].Partition_Serial_Number;

        }
        else
        {

          /* Compare the links in the sector with those in the table. */
          if ( ( Next_Sector->Link_Table[Sector_Link_Table_Index].Drive_Serial_Number != LinkTable->LinkArray[Index].Link_Data.Drive_Serial_Number ) ||
               ( Next_Sector->Link_Table[Sector_Link_Table_Index].Partition_Serial_Number != LinkTable->LinkArray[Index].Link_Data.Partition_Serial_Number )
             )
          {

            LOG_FEATURE_ERROR("The link table entries do not match!  The link tables do NOT match!")

            FEATURE_FUNCTION_EXIT("Link_Tables_Match")

            /* The link entries do not match! */
            return FALSE;

          }

        }

        Index++;

      }

    }

  }

  LOG_FEATURE_EVENT("The link tables match!")

  FEATURE_FUNCTION_EXIT("Link_Tables_Match")

  return TRUE;

}


static void _System Remove_Features_From_Aggregate_Partitions( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  /* Declare a local variable to walk the LVM Signature Sector Feature Array. */
  CARDINAL32                 FeatureIndex;

  /* Declare a local variable so that we can access the LVM Signature Sector without having to use a double indirection each time. */
  LVM_Signature_Sector *     Signature_Sector;

  Partition_Data *           PartitionRecord = (Partition_Data *) Object;
  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("Remove_Features_From_Aggregate_Partitions")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Remove_Features_From_Aggregate_Partitions")

    return;

  }

#endif

  /* Remove Drive Linking from the Feature Table in the LVM Signature Sector. */
  if ( PartitionRecord->Signature_Sector != NULL )
  {

    /* Get the signature sector. */
    Signature_Sector = PartitionRecord->Signature_Sector;

    /* Is Drive Linking listed in the LVM Signature Sector? */
    for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME; FeatureIndex++)
    {

      if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == DRIVE_LINKING_FEATURE_ID )
      {

        /* We have found the entry for Drive Linking.  Nuke it. */
        memset(&(Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID), 0, sizeof(LVM_Feature_Data) );

      }

    }

  }

  /* Remove the partition from whatever features may have hooked it before us. */
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;
  Old_Function_Table->Remove_Features( PartitionRecord, Error_Code);

  FEATURE_FUNCTION_EXIT("Remove_Features_From_Aggregate_Partitions")

  return;

}


static BOOLEAN  _System Delete_Partitions( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code)
{

  Partition_Data *            PartitionRecord = (Partition_Data *) Object;
  Partition_Deletion_Record * Deletion_Data = (Partition_Deletion_Record *) Parameters;
  Plugin_Function_Table_V1 *  Old_Function_Table;


  FEATURE_FUNCTION_ENTRY("Delete_Partitions")

  /* We don't want the partition record deleted by PruneList again, so set FreeMemory to false. */
  *FreeMemory = FALSE;

#ifdef DEBUG

#ifdef PARANOID

   assert(Object != NULL);
   assert(ObjectSize == sizeof(Partition_Data) );
   assert(Parameters != NULL);
   assert(PartitionRecord->Feature_Data != NULL );

#else

   if ( ( Object == NULL ) ||
        ( ObjectSize != sizeof( Partition_Data ) ) ||
        ( Parameters == NULL ) ||
        ( PartitionRecord->Feature_Data == NULL )
      )
   {

     *Error_Code = DLIST_CORRUPTED;

     FEATURE_FUNCTION_EXIT("Delete_Partitions")

     return FALSE;

   }

#endif

#endif

   /* Call the delete function for this partition. */
   Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;
   Old_Function_Table->Delete( PartitionRecord, Deletion_Data->Kill_Partitions, &(Deletion_Data->LVM_Error) );

   if ( Deletion_Data->LVM_Error != LVM_ENGINE_NO_ERROR )
     *Error_Code = DLIST_CORRUPTED;
   else
     *Error_Code = DLIST_SUCCESS;


   FEATURE_FUNCTION_EXIT("Delete_Partitions")

   return TRUE;

}


static void _System Initialize_DL_Partitions(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  Partition_Data *       Aggregate = (Partition_Data *) Parameters;
  Partition_Data *       PartitionRecord = (Partition_Data *) Object;
  LVM_Signature_Sector * Signature_Sector;
  CARDINAL32             FeatureIndex;
  Drive_Link_Array *     LinkTable;

  FEATURE_FUNCTION_ENTRY("Initialize_DL_Partitions")

#ifdef DEBUG

#ifdef PARANOID

  assert( ObjectTag == PARTITION_DATA_TAG );
  assert( ObjectSize == sizeof(Partition_Data) );

#else

  if ( ( ObjectTag != PARTITION_DATA_TAG ) ||
       ( ObjectSize != sizeof(Partition_Data) )
     )
  {

    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Initialize_DL_Partitions")

    return;

  }

#endif

#endif

  /* Get the drive linking table for this aggregate. */
  LinkTable = ( Drive_Link_Array *) Aggregate->Feature_Data->Data;

  /* Is this the first partition in the Partitions list? */
  if ( Aggregate->Partition_Size > 0 )
  {

     /* We are not the first partition.  Set the Spanned_Volume flag. */
    Aggregate->Spanned_Volume = TRUE;

  }

  /* Update the Partition Record. */

  /* Does the Partition Record have an LVM Signature Sector? */
  if ( PartitionRecord->Signature_Sector == NULL )
  {

    /* Since the Create_Volume code always provides a Signature Sector on the partitions it passes it,
       we must have an aggregate here.                                                                  */

    /* Allocate an LVM Signature Sector for this partition. */
    PartitionRecord->Signature_Sector = (LVM_Signature_Sector *) LVM_Common_Services->Allocate( sizeof ( BYTES_PER_SECTOR ) );

    if (PartitionRecord->Signature_Sector == NULL)
    {

      *Error_Code = DLIST_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Initialize_DL_Partitions")

      return;

    }

    /* Initialize the LVM Signature Sector. */
    Signature_Sector = PartitionRecord->Signature_Sector;
    memset(Signature_Sector,0, BYTES_PER_SECTOR);
    Signature_Sector->LVM_Signature1 = LVM_PRIMARY_SIGNATURE;
    Signature_Sector->LVM_Signature2 = LVM_SECONDARY_SIGNATURE;
    Signature_Sector->Signature_Sector_CRC = 0;
    Signature_Sector->Partition_Serial_Number = PartitionRecord->DLA_Table_Entry.Partition_Serial_Number;
    Signature_Sector->Partition_Start = PartitionRecord->DLA_Table_Entry.Partition_Start;
    Signature_Sector->Partition_End = PartitionRecord->DLA_Table_Entry.Partition_Start + PartitionRecord->DLA_Table_Entry.Partition_Size - 1;
    Signature_Sector->Partition_Sector_Count = PartitionRecord->DLA_Table_Entry.Partition_Size;
    Signature_Sector->Partition_Size_To_Report_To_User = PartitionRecord->DLA_Table_Entry.Partition_Size - 1;      /* One sector reserved for LVM Signature Sector, one sector reserved for fake EBR. */
    Signature_Sector->LVM_Reserved_Sector_Count = 1;
    Signature_Sector->Boot_Disk_Serial_Number = *(LVM_Common_Services->Boot_Drive_Serial_Number);
    Signature_Sector->Volume_Serial_Number = 0;
    Signature_Sector->LVM_Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
    Signature_Sector->LVM_Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;
    strncpy(Signature_Sector->Partition_Name, PartitionRecord->Partition_Name, PARTITION_NAME_SIZE);
    strncpy(Signature_Sector->Volume_Name, PartitionRecord->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);
    PartitionRecord->Usable_Size = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;

  }
  else
  {

    /* Establish access to the partitions LVM Signature Sector. */
    Signature_Sector = PartitionRecord->Signature_Sector;

  }


  /* Now add drive linking to the LVM Signature sector. */
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME ; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == 0)
    {

      /* We have found an open entry!  Lets fill it in. */
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID = DRIVE_LINKING_FEATURE_ID;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Active = TRUE;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Data_Size = DRIVE_LINKING_RESERVED_SECTOR_COUNT;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number = DRIVE_LINKING_MAJOR_VERSION;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Minor_Version_Number = DRIVE_LINKING_MINOR_VERSION;
      Signature_Sector->LVM_Reserved_Sector_Count += DRIVE_LINKING_RESERVED_SECTOR_COUNT;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;
      Signature_Sector->LVM_Reserved_Sector_Count += DRIVE_LINKING_RESERVED_SECTOR_COUNT;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;
      Signature_Sector->Partition_Size_To_Report_To_User = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;
      PartitionRecord->Usable_Size = Signature_Sector->Partition_Size_To_Report_To_User;

      /* Exit the loop. */
      break;
    }

  }

  /* Update the Aggregate. */
  Aggregate->Partition_Size += Signature_Sector->Partition_Size_To_Report_To_User;

  /* Add the partition to the LinkTable. */
  LinkTable->LinkArray[LinkTable->Links_In_Use].Link_Data.Drive_Serial_Number = LVM_Common_Services->DriveArray[PartitionRecord->Drive_Index].Drive_Serial_Number;
  LinkTable->LinkArray[LinkTable->Links_In_Use].Link_Data.Partition_Serial_Number = PartitionRecord->DLA_Table_Entry.Partition_Serial_Number;
  LinkTable->LinkArray[LinkTable->Links_In_Use].PartitionRecord = PartitionRecord;
  LinkTable->Links_In_Use++;
  LinkTable->ChangesMade = TRUE;

  /* Indicate success and return. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Initialize_DL_Partitions")

  return;

}


static void _System Write_Feature_Data(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  Partition_Data *           PartitionRecord = (Partition_Data *) Object;
  LVM_Signature_Sector *     Signature_Sector;
  LVM_Feature_Data     *     Feature_Data;
  CARDINAL32                 FeatureIndex;
  BOOLEAN                    Feature_Found = FALSE;
  Commit_Changes_Record *    Commit_Data = (Commit_Changes_Record *) Parameters;
  Volume_Data *              VolumeRecord = (Volume_Data *) Commit_Data->VData;
  Plugin_Function_Table_V1 * Old_Function_Table;


  FEATURE_FUNCTION_ENTRY("Write_Feature_Data")

#ifdef DEBUG

#ifdef PARANOID

  assert( ObjectTag == PARTITION_DATA_TAG );
  assert( ObjectSize == sizeof(Partition_Data) );

#else

  if ( ( ObjectTag != PARTITION_DATA_TAG ) ||
       ( ObjectSize != sizeof(Partition_Data) )
     )
  {

    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Write_Feature_Data")

    return;

  }

#endif

#endif

  /* Get the function table for the feature below drive linking on this partition. */
  Old_Function_Table = PartitionRecord->Feature_Data->Function_Table;

  /* Do we need to write drive linking data to disk? */
  if ( Commit_Data->Commit_Drive_Linking_Changes )
  {

    /* Get the LVM Signature Sector. */
    Signature_Sector = PartitionRecord->Signature_Sector;

    /* Find where Drive Linking feature data is stored on this partition. */
    for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME ; FeatureIndex++)
    {

      if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == DRIVE_LINKING_FEATURE_ID )
      {

        /* We have found the entry for Drive Linking.  */
        Feature_Found = TRUE;
        Feature_Data = &(Signature_Sector->LVM_Feature_Array[FeatureIndex]);

        /* Do we need to convert to LVM Version 1 format?  This could occur if the current LVM was installed
           as part of a fixpak and the fixpak is now being backed out for some reason.                       */
        if ( VolumeRecord->Convert_To_LVM_V1 )
        {

          /* We must change the version numbers for drive linking so that LVM Version 1 will accept them. */
          Feature_Data->Feature_Major_Version_Number = 1;
          Feature_Data->Feature_Minor_Version_Number = 0;

        }

        /* Now write the data to the specified locations. */
        Old_Function_Table->Write( PartitionRecord, Feature_Data->Location_Of_Primary_Feature_Data, DRIVE_LINKING_RESERVED_SECTOR_COUNT, &Feature_Data_Buffer1, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR)
        {

          if (*Error_Code != LVM_ENGINE_IO_ERROR)
          {

            *Error_Code = DLIST_CORRUPTED;

            FEATURE_FUNCTION_EXIT("Write_Feature_Data")

            return;

          }

        }

        Old_Function_Table->Write( PartitionRecord, Feature_Data->Location_Of_Secondary_Feature_Data, DRIVE_LINKING_RESERVED_SECTOR_COUNT, &Feature_Data_Buffer1, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR)
        {

          if (*Error_Code != LVM_ENGINE_IO_ERROR)
          {

            *Error_Code = DLIST_CORRUPTED;

            FEATURE_FUNCTION_EXIT("Write_Feature_Data")

            return;

          }

        }

        /* Exit the loop. */
        break;

      }

    }

    /* Did we find the entry for drive linking? */
    if ( ! Feature_Found )
      *Error_Code = DLIST_CORRUPTED;
    else
      *Error_Code = DLIST_SUCCESS;

  }

  /* Were there errors, or can we continue? */
  if ( *Error_Code == DLIST_SUCCESS )
  {

    /* We need to call the commit function for any other features which may be in effect on this partition.
       If there are no other features on this partition, then the commit function will go to the Pass Thru layer. */
    Old_Function_Table->Commit( Commit_Data->VData ,PartitionRecord, Error_Code);

    switch ( *Error_Code )
    {
      case LVM_ENGINE_NO_ERROR : *Error_Code = DLIST_SUCCESS;
                                 break;
      case LVM_ENGINE_OUT_OF_MEMORY : *Error_Code = DLIST_OUT_OF_MEMORY;
                                      break;
      default : *Error_Code = DLIST_CORRUPTED;
                break;
    }

  }

  FEATURE_FUNCTION_EXIT("Write_Feature_Data")

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
static void _System Calculate_Aggregate_Size(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access our Parameters without having to typecast each time. */
  CARDINAL32 * Aggregate_Size = (CARDINAL32 *) Parameters;

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  FEATURE_FUNCTION_ENTRY("Calculate_Aggregate_Size")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Calculate_Aggregate_Size")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Add the usable size of this partition to those we have seen already. */
    *Aggregate_Size += PartitionRecord->Usable_Size;

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Calculate_Aggregate_Size")

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
static void _System Set_Spanned_Volume_Flag(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data * PartitionRecord = (Partition_Data *) Object;

  FEATURE_FUNCTION_ENTRY("Set_Spanned_Volume_Flag")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Set_Spanned_Volume_Flag")

    return;

  }

#endif

  /* Well, Object has the correct TAG so we will assume that it points to an item of type Partition_Data. */

  /* Does this partition record represent a partition? */
  if ( PartitionRecord->Partition_Type == Partition )
  {

    /* Set the Spanned_Volume flag. */
    PartitionRecord->Spanned_Volume = TRUE;

  }

  /* Indicate success and leave. */
  *Error = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Set_Spanned_Volume_Flag")

  return;

}


static void _System Remove_Features(ADDRESS Aggregate, CARDINAL32 * Error_Code)
{

  Partition_Data *           PartitionRecord = (Partition_Data *) Aggregate;
  CARDINAL32                 Ignore_Error;

  FEATURE_FUNCTION_ENTRY("Remove_Features")

  /* Do we have a partition or an aggregate?  If we have an aggregate, then there should be Drive Linking Feature Data.
     If we have a partition, then Drive Linking may be listed in its LVM Signature Sector, but there will NOT be any
     Drive Linking Feature Data.                                                                                         */
  if ( PartitionRecord->Signature_Sector == NULL )
  {

    /* There is no LVM Signature Sector!  This is not correct! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Remove_Features")

    return;

  }

  if ( ( PartitionRecord->Feature_Data == NULL ) ||
       ( PartitionRecord->Feature_Data->Feature_ID->ID != DRIVE_LINKING_FEATURE_ID )
     )
  {

    /* We just need to make sure that drive linking is removed from the LVM Signature Sector. */
    Remove_Features_From_Aggregate_Partitions(PartitionRecord, PARTITION_DATA_TAG, sizeof(Partition_Data), NULL, NULL, Error_Code);

    if ( *Error_Code != DLIST_SUCCESS )
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }
  else
  {

    /* We have an aggregate here.  We need to delete the Aggregate and then process the partitions belonging to the aggregate. */

    /* Process the partitions belonging to the aggregate first. */
    LVM_Common_Services->ForEachItem(PartitionRecord->Feature_Data->Partitions, &Remove_Features_From_Aggregate_Partitions, NULL, TRUE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("Remove_Features")

      return;

    }

    /* Free any memory held by the Aggregate. */
    LVM_Common_Services->Deallocate(PartitionRecord->Feature_Data->Data);
    LVM_Common_Services->Deallocate(PartitionRecord->Signature_Sector);
    LVM_Common_Services->DestroyList( &(PartitionRecord->Feature_Data->Partitions), FALSE, &Ignore_Error);
    LVM_Common_Services->Deallocate(PartitionRecord->Feature_Data);

    /* Remove the Aggregate from the list of Aggregates. */
    LVM_Common_Services->DeleteItem(LVM_Common_Services->Aggregates, FALSE, PartitionRecord->Drive_Partition_Handle, &Ignore_Error);

    /* Now delete the aggregate. */
    LVM_Common_Services->Deallocate(Aggregate);

  }

  FEATURE_FUNCTION_EXIT("Remove_Features")

  /* All done. */
  return;

}


static void _System PassThru( CARDINAL32 Feature_ID, ADDRESS Aggregate, ADDRESS InputBuffer, CARDINAL32 InputSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputSize, CARDINAL32 * Error_Code )
{

  Partition_Data *           PartitionRecord = (Partition_Data *) Aggregate;
  PassThru_Data_Record       PassThru_Data;
  CARDINAL32                 LocalError;

  FEATURE_FUNCTION_ENTRY("PassThru")

  /* Save the PassThru parameters.  We will need to pass them to the first feature on each of the partitions in the Aggregate. */
  PassThru_Data.Feature_ID = Feature_ID;
  PassThru_Data.Aggregate = Aggregate;
  PassThru_Data.InputBuffer = InputBuffer;
  PassThru_Data.InputSize = InputSize;
  PassThru_Data.OutputBuffer = OutputBuffer;
  PassThru_Data.OutputSize = OutputSize;
  PassThru_Data.Error_Code = Error_Code;

  /* Drive Linking has no PassThru commands that it accepts, so this call should not be directed to Drive Linking. */
  if ( Feature_ID == DRIVE_LINKING_FEATURE_ID )
  {

    *Error_Code = LVM_ENGINE_BAD_FEATURE_ID;

    FEATURE_FUNCTION_EXIT("PassThru")

    return;

  }

  /* Since this is not for us or our aggregate, we must pass it to the next feature on each of the partitions in the Aggregate. */
  LVM_Common_Services->ForEachItem(PartitionRecord->Feature_Data->Partitions,&Continue_PassThru, &PassThru_Data,TRUE,&LocalError);

  if ( LocalError != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }

  FEATURE_FUNCTION_EXIT("PassThru")

  /* *Error_Code was set by the call to PassThru, so leave it alone. */
  return;

}


static void _System Continue_PassThru(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;
  PassThru_Data_Record   *   PassThru_Data = (PassThru_Data_Record *) Parameters;

  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("Continue_PassThru")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Continue_PassThru")

    return;

  }

#endif

  /* Is the feature data for this partition available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL PassThru has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *(PassThru_Data->Error_Code) = LVM_ENGINE_BAD_PARTITION;
    *Error_Code = DLIST_SEARCH_COMPLETE;

    FEATURE_FUNCTION_EXIT("Continue_PassThru")

    return;

  }


  /* Get the function table for the first feature on this partition. */
  Old_Function_Table = (Plugin_Function_Table_V1 *) PartitionRecord->Feature_Data->Function_Table;

  /* Is the function table for the next feature on this partition available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL PassThru has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *(PassThru_Data->Error_Code) = LVM_ENGINE_BAD_PARTITION;
    *Error_Code = DLIST_SEARCH_COMPLETE;

    FEATURE_FUNCTION_EXIT("Continue_PassThru")

    return;

  }

  /* Now call the PassThru function for this feature and partition. */
  Old_Function_Table->PassThru(PassThru_Data->Feature_ID,
                               PartitionRecord,
                               PassThru_Data->InputBuffer,
                               PassThru_Data->InputSize,
                               PassThru_Data->OutputBuffer,
                               PassThru_Data->OutputSize,
                               PassThru_Data->Error_Code);

  /* Since we successfully called the PassThru function of the first feature on the partition, indicate success and return. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Continue_PassThru")

  return;

}


static void _System Continue_Changes_Pending(ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters, CARDINAL32 * Error_Code)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access our parameters without having to typecast each time. */
  BOOLEAN *                  ChangesArePending = ( BOOLEAN * ) Parameters;

  Plugin_Function_Table_V1 * Old_Function_Table;

  FEATURE_FUNCTION_ENTRY("Continue_Changes_Pending")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Continue_Changes_Pending")

    return;

  }

#endif

  /* Is the feature data for this partition available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = DLIST_SEARCH_COMPLETE;

    FEATURE_FUNCTION_EXIT("Continue_Changes_Pending")

    return;

  }


  /* Get the function table for the first feature on this partition. */
  Old_Function_Table = (Plugin_Function_Table_V1 *) PartitionRecord->Feature_Data->Function_Table;

  /* Is the function table for the next feature on this partition available? */
  if ( Old_Function_Table == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = DLIST_SEARCH_COMPLETE;

    FEATURE_FUNCTION_EXIT("Continue_Changes_Pending")

    return;

  }

  /* Now call the ChangesPending function for this feature and partition. */
  *ChangesArePending = Old_Function_Table->ChangesPending(PartitionRecord, Error_Code);

  /* If changes are pending on one partition, we don't need to ask any of the others. */
  if ( *ChangesArePending )
    *Error_Code = DLIST_SEARCH_COMPLETE;
  else
    *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Continue_Changes_Pending")

  return;

}


static void _System ReturnCurrentClass( ADDRESS PartitionRecord, LVM_Classes * Actual_Class, BOOLEAN * Top_Of_Class, CARDINAL32 * Sequence_Number )
{

  Partition_Data *     Partition_Record = (Partition_Data *) PartitionRecord;
  Drive_Link_Array *   Link_Table;

  FEATURE_FUNCTION_ENTRY("ReturnCurrentClass")

  /* This function should only be called for LVM Volumes. */

  /* Get the link table for the aggregate. */
  Link_Table = (Drive_Link_Array *) Partition_Record->Feature_Data->Data;

  /* Return the requested values. */
  *Actual_Class = Link_Table->Actual_Class;
  *Top_Of_Class = Link_Table->Top_Of_Class;
  *Sequence_Number = Link_Table->Feature_Sequence_Number;

  FEATURE_FUNCTION_EXIT("ReturnCurrentClass")

  return;

}


void _System Get_Required_LVM_Version( CARDINAL32 * Major_Version_Number, CARDINAL32 * Minor_Version_Number)
{

  *Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
  *Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;

  return;

}



#define STRING_MESSAGE_FILE             "LVM.MSG"
#define SINGLE_LINE_OUTLINE             2
#define DEFAULT_SCROLL_BAR              3
#define LESS_THAN_GREATER_THAN          6
#define HELP_MESSAGE_LINE               43
#define GROUP_PANEL_TITLE_LINE          93
#define GROUP_PANEL_MESSAGE_LINE        94
#define MAX_STRING_MESSAGE_SIZE         (1024 * 12)

#define MAX_HELP_TEXT_LINES             128
#define HELP_MESSAGE_FILE               "LVMH.MSG"
#define MAX_HELP_MESSAGE_SIZE           (1024 * 12)
#define DRIVE_LINKING_HELP_MESSAGE      5026

uint _System DosGetMessage ( char **, uint, char *, uint, uint, char *, uint *);
char *help_message=NULL;
#ifdef __DLL__ 
char chars_less_than_greater_than[2] = {'<', '>'};
#else
extern char chars_less_than_greater_than[2];

#endif
/*
 * from lvmexe\strings.c
 * build the outlines and scroll bars from the message file entries.
 *
 */

void get_char_message ( char    *memory,
                        uint    message_number,
                        char    *array,
                        uint    array_length )
{
    register
    char    *line;
    uint    rc,
            index = 0,
            length;

    FEATURE_FUNCTION_ENTRY("get_char_message")

    rc = DosGetMessage ( NULL, 0, memory, MAX_STRING_MESSAGE_SIZE - 1,
                         message_number, STRING_MESSAGE_FILE, &length );
    if ( rc == 0 ) {
        memory [ length ] = '\0';
        for ( line = memory;  *line;  ++line ) {
            if ( *line == ' ' ) {                               /* skip spaces */
                continue;
            } else if ( *line == '\r' ) {                       /* EOL */
                break;
            } else {                                            /* save char */
                if ( index < array_length ) {
                    array [ index ] = *line;
                    ++index;
                }
            }
        }
    }
}

char * get_line_message ( char *memory,         /* user supplied memory */
                          uint message_number)
{
   unsigned long error;
   unsigned int  len;
   char *temp = NULL;

   FEATURE_FUNCTION_ENTRY("get_line_message")

   error = DosGetMessage ( NULL, 0, memory, MAX_STRING_MESSAGE_SIZE - 1,
                           message_number, STRING_MESSAGE_FILE, &len );

   if (!error) {
      memory[len] = '\0';
      temp = memory + 2;  /* skip first \r\n */
      while (*temp) {
         if ((*temp == '\n') || (*temp == '\r')) {
            *temp = '\0';
         }
         ++temp;
      }
      temp = memory + 2;
   }

   FEATURE_FUNCTION_EXIT("get_line_message")

   return (temp);
}

/*
 * from lvm\lvmexe\help.c
 * build the help DL_help_panel_text array from the help message file entry.
 * caller frees memory.
 *
 */


void * get_help_message ( uint    message_number )
{
    register
    char    *line;
    char    *memory,
            *text_line;
    uint    rc,
            index,
            length;
    bool    no_help = TRUE;

    FEATURE_FUNCTION_ENTRY("get_help_message")

    memory = LVM_Common_Services->Allocate ( MAX_HELP_MESSAGE_SIZE );

    if ( memory ) {
        rc = DosGetMessage ( NULL, 0, memory, MAX_HELP_MESSAGE_SIZE - 1,
                             message_number, HELP_MESSAGE_FILE, &length );

        if ( rc == 0 ) {
            no_help = FALSE;
            memory [ length ] = '\0';
            text_line = memory;
            index = 0;
            for ( line = memory;  *line;  ++line ) {
                if ( *line == '\r' ) {
                    *line = ' ';
                } else if ( *line == '\n' ) {
                    *line = '\0';
                    if ( index < MAX_HELP_TEXT_LINES ) {
                        DL_help_panel_text [ index ] = text_line;
                        ++index;
                        text_line = line + 1;
                    }
                }
            }
            DL_help_panel_text [ index ] = NULL;
        } else {
           LVM_Common_Services->Deallocate(memory);
           memory = 0;
        }
    }

    FEATURE_FUNCTION_EXIT("get_help_message")

    return  memory;
}

void Init_VIO_Help_Panel()
{
   char *memory;

   FEATURE_FUNCTION_ENTRY("Init_VIO_Help_Panel")

   if (DL_help_panel.control == 0) {
      memory = LVM_Common_Services->Allocate ( MAX_STRING_MESSAGE_SIZE );
      DL_help_panel_text = (char **) LVM_Common_Services->Allocate((MAX_HELP_TEXT_LINES + 1) * sizeof (char *));
      help_message = get_help_message(DRIVE_LINKING_HELP_MESSAGE);
      if (memory && DL_help_panel_text && help_message) {

         DL_help_panel.control = INITIALIZE_PANEL | NOT_SIZED |
                                 SHOW_TITLE | SAVE_RESTORE |
                                 SHOW_OUTLINE | HAS_BORDER |
                                 SHOW_ATTRIBUTES | SHOW_TEXT;


         get_char_message ( memory, SINGLE_LINE_OUTLINE,
                            (char *) &DL_help_panel.box.outline,
                            sizeof ( outline_t ) );

         get_char_message ( memory, DEFAULT_SCROLL_BAR,
                            (char *) &DL_help_panel.box.scroll_bar,
                            sizeof ( scroll_bar_t ) );

         DL_help_panel.box.row = 1;
         DL_help_panel.box.column = 0;
         DL_help_panel.box.row_length = 0;
         DL_help_panel.box.number_of_rows = GetScreenRows() - 2;
         DL_help_panel.box.attribute = BLACK_FG | LT_WHITE_BG;
         DL_help_panel.box.border_attribute = BLACK_FG | LT_WHITE_BG;

         DL_help_panel.text_line = DL_help_panel_text;

         SetPanelSize ( &DL_help_panel );

         LVM_Common_Services->Deallocate(memory);
       }
   }
}

unsigned long VIO_ShowMessageBar(unsigned long message_number)
{
   char *memory;
   unsigned long error = 0;

   FEATURE_FUNCTION_ENTRY("VIO_ShowMessageBar")

   memory = LVM_Common_Services->Allocate ( MAX_STRING_MESSAGE_SIZE );
   if (memory) {
      message_bar_panel_text[0] = get_line_message(memory, GROUP_PANEL_MESSAGE_LINE);
      message_bar_panel_text[1] = NULL;
      message_bar_panel.text_line = message_bar_panel_text;
      message_bar_panel.box.attribute = BLACK_BG | LT_WHITE_FG;
      MessageBarPanel(&message_bar_panel);
   } else {
      error = 1; /* no memory */
   }

   FEATURE_FUNCTION_EXIT("VIO_ShowMessageBar")

   return (error);
}

void VIO_ClearMessageBar()
{

  FEATURE_FUNCTION_ENTRY("VIO_ClearMessageBar")

   ErasePanel( &message_bar_panel);
   LVM_Common_Services->Deallocate(message_bar_panel_text[0] - 2);
   message_bar_panel_text[0] = NULL;
}

void _System VIO_Help_Panel (CARDINAL32 Help_Index, CARDINAL32 * Error_Code)
{

  FEATURE_FUNCTION_ENTRY("VIO_Help_Panel")

   Init_VIO_Help_Panel();
   if (!VIO_ShowMessageBar(HELP_MESSAGE_LINE)) {
      ScrollingPanel ( &DL_help_panel );
      ErasePanel ( &DL_help_panel );
      VIO_ClearMessageBar();
   }
}


bool MoveMenuCursor( unsigned int cur_position,
                     unsigned int *new_position,
                     unsigned int *menu_state,
                     int  count,
                     bool bForward)
{
   int i;
   bool success = FALSE;

   FEATURE_FUNCTION_ENTRY("MoveMenuCursor")

   if (bForward) {
      i = cur_position + 1;
      while (i < count) {
         if (menu_state[i] & STATE_SELECTABLE) {
            success = TRUE;
            break;
         }
         i++;
      }

   } else {
      i = cur_position - 1;
      while (i >= 0) {
         if (menu_state[i] & STATE_SELECTABLE) {
            success = TRUE;
            break;
         }
         i--;
      }
   }

   if (success == TRUE) {
      *new_position = i;
      menu_state[cur_position] &= ~ACTION_NONE;
      menu_state[cur_position] |= ACTION_UNHIGHLIGHT;
      menu_state[i] &= ~ACTION_NONE;
      menu_state[i] |= ACTION_HIGHLIGHT;
   }

   FEATURE_FUNCTION_EXIT("MoveMenuCursor")

   return (success);
}

unsigned int _System Create_and_Configure_callback (panel_t *panel)
{
   uint i;
   uint num_selected_items;


   FEATURE_FUNCTION_ENTRY("Create_and_Configure_callback")

   for (i=0; i<DL_group_partition_count; i++) {
      DL_group_panel_state[i] |= ACTION_NONE;       /* assume nothing to do */
      DL_group_panel_state[i] &= ACTION_CLEAR_BITS; /* clear all other action bits */
   }

   switch (panel->choice) {   /* on input, "choice" is the key was pressed by the user */

      case ENTER:  /* group selected items */
         if (DL_group_panel_cursor != DL_group_partition_count) {
            num_selected_items = 0;
            for (i=0; i<DL_group_partition_count; i++) {
               if (DL_group_panel_state[i] & STATE_SELECTED) {
                  ++num_selected_items;
                  if (num_selected_items == 2)
                     break;
               }
            }

            /* Are there at least 2 selected items? */
            if (num_selected_items == 2) {
               ++DL_cur_aggregate_number;
               for (i=0; i<DL_group_partition_count; i++) {
                  if (DL_group_panel_state[i] & STATE_SELECTED) {
                     DL_group_panel_state[i] &= ~(ACTION_NONE | STATE_SELECTED | STATE_SELECTABLE);
                     DL_group_panel_state[i] |= (ACTION_DESELECT | ACTION_SET_NOT_SELECTABLE);
                     pDLink->Partition_Array[i].Aggregate_Number = DL_cur_aggregate_number;
                  }
               }
               /* set the cursor to the first selectable item */
               for (i=0; i<DL_group_partition_count; i++) {
                  if (DL_group_panel_state[i] & STATE_SELECTABLE) {
                     DL_group_panel_cursor = i;
                     DL_group_panel_state[i] &= ~ACTION_NONE;
                     DL_group_panel_state[i] |= ACTION_HIGHLIGHT;
                     break;
                  }
               }
               /* exit if no more to select from (ie. return(1)) */
               if (i == DL_group_partition_count)
                  /* return(1); */
                  DL_group_panel_cursor = DL_group_partition_count; /* end of list */
            }
         }
         break;
      case ESCAPE: /* undo */
         if (DL_cur_aggregate_number >= 0) {
            for (i=0; i<DL_group_partition_count; i++) {
               if (pDLink->Partition_Array[i].Aggregate_Number == DL_cur_aggregate_number) {
                  DL_group_panel_state[i] &= ~ACTION_NONE;
                  DL_group_panel_state[i] |= (ACTION_SET_SELECTABLE | STATE_SELECTABLE);
                  pDLink->Partition_Array[i].Aggregate_Number = -1;
               }
            }
            --DL_cur_aggregate_number;

            if (DL_group_panel_cursor == DL_group_partition_count) {
               /* set the cursor to the first selectable item */
               for (i=0; i<DL_group_partition_count; i++) {
                  if (DL_group_panel_state[i] & STATE_SELECTABLE) {
                     DL_group_panel_cursor = i;
                     DL_group_panel_state[i] &= ~ACTION_NONE;
                     DL_group_panel_state[i] |= ACTION_HIGHLIGHT;
                     break;
                  }
               }
            }
         }
         break;
     case F6:

        FEATURE_FUNCTION_EXIT("Create_and_Configure_callback")

        return (1);  /* tell UserDefinedMenuPanel to exit */
         break;
      case UP_CURSOR:   /* move cursor */
      case DOWN_CURSOR:
         if (DL_group_panel_cursor != DL_group_partition_count) {
            MoveMenuCursor(DL_group_panel_cursor,
                           &DL_group_panel_cursor,
                           DL_group_panel_state,
                           DL_group_partition_count,
                           (panel->choice == DOWN_CURSOR) ? TRUE : FALSE);
         }
         break;
      case SPACE: /* select or deselect a menu item */
         if (DL_group_panel_cursor != DL_group_partition_count) {
            DL_group_panel_state[DL_group_panel_cursor] &= ~ACTION_NONE;
            if (DL_group_panel_state[DL_group_panel_cursor] & STATE_SELECTED) { /* currently selected? */
               DL_group_panel_state[DL_group_panel_cursor] |= ACTION_DESELECT;
               DL_group_panel_state[DL_group_panel_cursor] &= ~STATE_SELECTED; /* clear selected */
            } else {
               DL_group_panel_state[DL_group_panel_cursor] |= ACTION_SELECT;
               DL_group_panel_state[DL_group_panel_cursor] |= STATE_SELECTED; /* set selected */
            }
         }
         break;
      default:  /* ignore all other keys */
         break;
   }
   panel->choice = DL_group_panel_cursor; /* new choice */

   FEATURE_FUNCTION_EXIT("Create_and_Configure_callback")

   return (0);
}

void _System VIO_Create_and_Configure ( CARDINAL32 ID, ADDRESS InputBuffer, CARDINAL32 InputBufferSize, ADDRESS * OutputBuffer, CARDINAL32 * OutputBufferSize, CARDINAL32 * Error_Code)
{

   bool finished = FALSE;
   uint key;
   char *memory;
   int i, j;
   Partition_Information_Record partition_info;
   unsigned long error;

   FEATURE_FUNCTION_ENTRY("VIO_Create_and_Configure")

   pDLink = (DLink_Comm_Struct *)InputBuffer;
   DL_group_partition_count = pDLink->Count;
   memory = LVM_Common_Services->Allocate ( MAX_STRING_MESSAGE_SIZE );
   DL_group_panel_text = (char **) LVM_Common_Services->Allocate( ( DL_group_partition_count + 1 ) * sizeof(char*) );
   DL_group_panel_state = LVM_Common_Services->Allocate( ( DL_group_partition_count + 1 ) * sizeof(unsigned int) );

   if (DL_group_panel_text) {
      for (i=0; i<DL_group_partition_count+1; i++)
         DL_group_panel_text[i] = NULL;
   }

   if (memory && DL_group_panel_text && DL_group_panel_state)
   {
         DL_group_panel.control = INITIALIZE_PANEL | NOT_SIZED |
                                  SHOW_TITLE | SAVE_RESTORE |
                                  SHOW_OUTLINE | HAS_BORDER |
                                  SHOW_ATTRIBUTES | SHOW_TEXT;


         get_char_message ( memory, SINGLE_LINE_OUTLINE,
                            (char *) &DL_group_panel.box.outline,
                            sizeof ( outline_t ) );

         get_char_message ( memory, DEFAULT_SCROLL_BAR,
                            (char *) &DL_group_panel.box.scroll_bar,
                            sizeof ( scroll_bar_t ) );

         get_char_message ( memory, LESS_THAN_GREATER_THAN,
                            chars_less_than_greater_than,
                            2);

         DL_group_panel.box.row = 1;
         DL_group_panel.box.column = 0;
         DL_group_panel.box.row_length = GetScreenColumns();
         DL_group_panel.box.number_of_rows = GetScreenRows() - 2;
         DL_group_panel.box.attribute = BLUE_BG | LT_WHITE_FG;
         DL_group_panel.box.border_attribute = BLUE_BG | LT_CYAN_FG;

         DL_group_panel.text_line = DL_group_panel_text;
         DL_group_panel.title_line = get_line_message(memory, GROUP_PANEL_TITLE_LINE);


      for (i=0; i<DL_group_partition_count; i++) {
         /* get partition name */
         partition_info = Get_Partition_Information( pDLink->Partition_Array[i].Partition_Handle, &error);
         if (error)
            break;
         DL_group_panel_text[i] = LVM_Common_Services->Allocate (PARTITION_NAME_SIZE + 3);
         if (DL_group_panel_text[i]) {
            memset(DL_group_panel_text[i], ' ', PARTITION_NAME_SIZE + 3);
            strncpy(&DL_group_panel_text[i][1], partition_info.Partition_Name, PARTITION_NAME_SIZE);
            for ( j = PARTITION_NAME_SIZE; j > 0; j--)
            {

              if ( ( DL_group_panel_text[i][j] != ' ' ) && ( DL_group_panel_text[i][j] != 0 ) )
              {

                DL_group_panel_text[i][j + 1] = ' ' ;
                DL_group_panel_text[i][j + 2] = 0 ;
                break;

              }

            }

         } else {
            error = LVM_ENGINE_OUT_OF_MEMORY;
         }
         DL_group_panel_state[i] = STATE_SELECTABLE;
      }

      if (error)
      {

        FEATURE_FUNCTION_EXIT("VIO_Create_and_Configure")

        return;

      }

      DL_group_panel_state[0] |= ACTION_HIGHLIGHT;
      DL_group_panel_cursor = 0;   /* initialize the cursor every time. */


      DL_cur_aggregate_number = -1;

      error = VIO_ShowMessageBar(GROUP_PANEL_MESSAGE_LINE);
      if (!error) {
         DL_group_panel.callback = Create_and_Configure_callback;
         key = UserDefinedMenuPanel( &DL_group_panel,
                                      DL_group_partition_count,
                                      DL_group_panel_state,
                                      chars_less_than_greater_than[0],
                                      chars_less_than_greater_than[1]);

         ErasePanel( &DL_group_panel);
         VIO_ClearMessageBar();
      }
   }


   if (DL_group_panel_text) {

      /* for (i=0; i<DL_group_partition_count; i++) */
      for (i=DL_group_partition_count-1; i>=0; i--)
      {
         if (DL_group_panel_text[i])
            LVM_Common_Services->Deallocate(DL_group_panel_text[i]);
      }
      LVM_Common_Services->Deallocate(DL_group_panel_text);
   }



   if (DL_group_panel_state)
      LVM_Common_Services->Deallocate(DL_group_panel_state);

   if (memory)
      LVM_Common_Services->Deallocate(memory);

   FEATURE_FUNCTION_EXIT("VIO_Create_and_Configure")

   return;
}


ADDRESS _System Exchange_Function_Tables( ADDRESS Common_Services )
{

  /* Initialize the Feature_ID_Record. */
  strcpy(Feature_ID_Record.Name, "IBM Drive Linking");
  strcpy(Feature_ID_Record.Short_Name, "IBM_DL");
  strcpy(Feature_ID_Record.OEM_Info, "International Business Machines");
  Feature_ID_Record.ID = DRIVE_LINKING_FEATURE_ID;
  Feature_ID_Record.Major_Version_Number = DRIVE_LINKING_MAJOR_VERSION;
  Feature_ID_Record.Minor_Version_Number = DRIVE_LINKING_MINOR_VERSION;
  Feature_ID_Record.LVM_Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
  Feature_ID_Record.LVM_Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;
  Feature_ID_Record.Preferred_Class = Aggregate_Class;
  Feature_ID_Record.ClassData[Partition_Class].ClassMember = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].GlobalExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].TopExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].BottomExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].ClassExclusive = FALSE;
  Feature_ID_Record.ClassData[Partition_Class].Weight_Factor = 1;
  Feature_ID_Record.ClassData[Aggregate_Class].ClassMember = TRUE;
  Feature_ID_Record.ClassData[Aggregate_Class].GlobalExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].TopExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].BottomExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].ClassExclusive = FALSE;
  Feature_ID_Record.ClassData[Aggregate_Class].Weight_Factor = 95;
  Feature_ID_Record.ClassData[Volume_Class].ClassMember = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].GlobalExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].TopExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].BottomExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].ClassExclusive = FALSE;
  Feature_ID_Record.ClassData[Volume_Class].Weight_Factor = 1;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Create_and_Configure = NULL;
  Feature_ID_Record.Interface_Support[PM_Interface].Interface_Supported = FALSE;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Create_and_Configure = VIO_Create_and_Configure;
  Feature_ID_Record.Interface_Support[VIO_Interface].Interface_Supported = TRUE;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Display_Status = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Display_Status = NULL;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Control_Panel = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Control_Panel = NULL;
  Feature_ID_Record.Interface_Support[PM_Interface].VIO_PM_Calls.Help_Panel = NULL;
  Feature_ID_Record.Interface_Support[VIO_Interface].VIO_PM_Calls.Help_Panel = VIO_Help_Panel;
  Feature_ID_Record.Interface_Support[Java_Interface].Java_Interface_Class = "drivelinking";
  Feature_ID_Record.Interface_Support[Java_Interface].Interface_Supported = TRUE;


  /* Initialize the Function Table for this feature. */
  Function_Table.Feature_ID = &Feature_ID_Record;
  Function_Table.Open_Feature = &Open_Feature;
  Function_Table.Close_Feature = &Close_Feature;
  Function_Table.Can_Expand = &Can_Expand_DL_Volume;
  Function_Table.Add_Partition = &Add_DL_Partition;
  Function_Table.Delete = &Delete_DL_Partition;
  Function_Table.Discover = &Discover_Drive_Links;
  Function_Table.Remove_Features = &Remove_Features;
  Function_Table.Create = &Create_DL_Volume;
  Function_Table.Commit = &Commit_Drive_Linking_Changes;
  Function_Table.Write = &DL_Write;
  Function_Table.Read = &DL_Read;
  Function_Table.ReturnCurrentClass = &ReturnCurrentClass;
  Function_Table.PassThru = &PassThru;
  Function_Table.ChangesPending = &DL_ChangesPending;
  Function_Table.ParseCommandLineArguments = &DL_ParseCommandLineArguments;

  /* Save the common functions provided by LVM.DLL. */
  LVM_Common_Services = ( LVM_Common_Services_V1 * ) Common_Services;

  /* Return our table of functions to LVM.DLL. */
  return (ADDRESS) &Function_Table;

}


static BOOLEAN _System Claim_Aggregate_Partitions( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters, BOOLEAN * FreeMemory, CARDINAL32 * Error_Code)
{

  /* Declare a local variable so that we can access the Partition_Data without having to typecast each time. */
  Partition_Data *           PartitionRecord = (Partition_Data *) Object;

  /* Declare a local variable so that we can access the Partition_Data passed to us as a parameter without having to typecast each time. */
  Partition_Data *           PartitionRecord_To_Find = ( Partition_Data * ) Parameters;


  FEATURE_FUNCTION_ENTRY("Claim_Aggregate_Partitions")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Claim_Aggregate_Partitions")

    return FALSE;

  }

#endif

  /* Since the size and TAG of the object are correct, we will proceed. */

  /* Since we don't want the actual Partition Data being deleted, set *FreeMemory to FALSE. */
  *FreeMemory = FALSE;

  /* Is the current partition the one we are looking for? */
  if ( PartitionRecord->External_Handle == PartitionRecord_To_Find->External_Handle )
  {

    /* This is the partition we are looking for!  We will return TRUE so that it will be removed from the Partitions List. */

    /* Stop further searching. */
    *Error_Code = DLIST_SEARCH_COMPLETE;

    FEATURE_FUNCTION_EXIT("Claim_Aggregate_Partitions")

    return TRUE;

  }

  /* Indicate success.  Return FALSE so that the current item will not be removed from the Partitions list. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Claim_Aggregate_Partitions")

  return FALSE;

}


static void _System Add_Partitions_To_Aggregate( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code)
{

  Partition_Data *        Aggregate = (Partition_Data *) Parameters;
  Partition_Data *        New_Partition = (Partition_Data *) Object;
  Drive_Link_Array *      LinkTable;
  LVM_Signature_Sector *  Signature_Sector;
  CARDINAL32              FeatureIndex;
  BOOLEAN                 Entry_Found = FALSE;

  FEATURE_FUNCTION_ENTRY("Add_Partitions_To_Aggregate")

  /* We must add the new partition to the Aggregate. */

  /* Get the drive linking data for the aggregate. */
  LinkTable = (Drive_Link_Array *) Aggregate->Feature_Data->Data;

  /* Does New_Partition have an LVM Signature Sector? */
  if ( New_Partition->Signature_Sector == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"The partition does not have an LVM Signature Sector.\n     Attempting to allocate an LVM Signature Sector.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* Allocate an LVM Signature Sector for this partition. */
    New_Partition->Signature_Sector = (LVM_Signature_Sector *) LVM_Common_Services->Allocate( sizeof ( BYTES_PER_SECTOR ) );

    if (New_Partition->Signature_Sector == NULL)
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Error: Out of memory while attempting to allocate a new LVM Signature Sector");
        LVM_Common_Services->Write_Log_Buffer();

      }

      *Error_Code = DLIST_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Add_Partitions_To_Aggregate")

      return;

    }

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Initializing the new LVM Signature Sector.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    /* Initialize the LVM Signature Sector. */
    Signature_Sector = New_Partition->Signature_Sector;
    memset(Signature_Sector,0, BYTES_PER_SECTOR);
    Signature_Sector->LVM_Signature1 = LVM_PRIMARY_SIGNATURE;
    Signature_Sector->LVM_Signature2 = LVM_SECONDARY_SIGNATURE;
    Signature_Sector->Signature_Sector_CRC = 0;
    Signature_Sector->Partition_Serial_Number = New_Partition->DLA_Table_Entry.Partition_Serial_Number;
    Signature_Sector->Partition_Start = New_Partition->DLA_Table_Entry.Partition_Start;
    Signature_Sector->Partition_End = New_Partition->DLA_Table_Entry.Partition_Start + New_Partition->DLA_Table_Entry.Partition_Size - 1;
    Signature_Sector->Partition_Sector_Count = New_Partition->DLA_Table_Entry.Partition_Size;
    Signature_Sector->Partition_Size_To_Report_To_User = New_Partition->DLA_Table_Entry.Partition_Size - 1;      /* One sector reserved for LVM Signature Sector, one sector reserved for fake EBR. */
    Signature_Sector->LVM_Reserved_Sector_Count = 1;
    Signature_Sector->Boot_Disk_Serial_Number = *(LVM_Common_Services->Boot_Drive_Serial_Number);
    Signature_Sector->Volume_Serial_Number = 0;
    Signature_Sector->LVM_Major_Version_Number = CURRENT_LVM_MAJOR_VERSION_NUMBER;
    Signature_Sector->LVM_Minor_Version_Number = CURRENT_LVM_MINOR_VERSION_NUMBER;
    strncpy(Signature_Sector->Partition_Name, New_Partition->Partition_Name, PARTITION_NAME_SIZE);
    strncpy(Signature_Sector->Volume_Name, Aggregate->DLA_Table_Entry.Volume_Name, VOLUME_NAME_SIZE);

  }
  else
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"Using the partition's existing LVM Signature Sector.");
      LVM_Common_Services->Write_Log_Buffer();

    }

    Signature_Sector = New_Partition->Signature_Sector;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Attempting to add Drive Linking to the partition's LVM Signature Sector.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now add drive linking to the LVM Signature sector. */
  for ( FeatureIndex = 0; FeatureIndex < MAX_FEATURES_PER_VOLUME ; FeatureIndex++)
  {

    if ( Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID == 0)
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"Found an open entry in the Feature Array.  Adding Drive Linking.");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* We have found an open entry!  Lets fill it in. */
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_ID = DRIVE_LINKING_FEATURE_ID;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Active = TRUE;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Data_Size = DRIVE_LINKING_RESERVED_SECTOR_COUNT;
      Signature_Sector->LVM_Reserved_Sector_Count += DRIVE_LINKING_RESERVED_SECTOR_COUNT;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Primary_Feature_Data = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;
      Signature_Sector->LVM_Reserved_Sector_Count += DRIVE_LINKING_RESERVED_SECTOR_COUNT;
      Signature_Sector->LVM_Feature_Array[FeatureIndex].Location_Of_Secondary_Feature_Data = ( Signature_Sector->Partition_End - Signature_Sector->LVM_Reserved_Sector_Count) + 1;
      Signature_Sector->Partition_Size_To_Report_To_User = Signature_Sector->Partition_Sector_Count - Signature_Sector->LVM_Reserved_Sector_Count;
      New_Partition->Usable_Size -= (2 * DRIVE_LINKING_RESERVED_SECTOR_COUNT);

      /* If this aggregate is going to be part of an LVM Volume created using Version 1 of Drive Linking, then we must
         use the appropriate version numbers for drive linking.                                                         */
      if ( LinkTable->Aggregate_Signature_Sector_Expected )
      {

        /* Only Drive Linking Version 2 or later aggregates have LVM Signature Sectors associated with them! */
        Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number = DRIVE_LINKING_MAJOR_VERSION;
        Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Minor_Version_Number = DRIVE_LINKING_MINOR_VERSION;

      }
      else
      {

        /* We must have an aggregate created by drive linking version 1.  Use the appropriate version numbers. */
        Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Major_Version_Number = OLD_DRIVE_LINKING_MAJOR_VERSION;
        Signature_Sector->LVM_Feature_Array[FeatureIndex].Feature_Minor_Version_Number = OLD_DRIVE_LINKING_MINOR_VERSION;
        Signature_Sector->LVM_Major_Version_Number = OLD_CURRENT_LVM_MAJOR_VERSION_NUMBER;
        Signature_Sector->LVM_Minor_Version_Number = OLD_CURRENT_LVM_MINOR_VERSION_NUMBER;

      }

      /* If this item is not an aggregate, then mark the drive containing this partition as being dirty. */
      if ( New_Partition->Drive_Index < LVM_Common_Services->DriveCount )
        LVM_Common_Services->DriveArray[New_Partition->Drive_Index].ChangesMade = TRUE;

      Entry_Found = TRUE;

      /* Exit the loop. */
      break;
    }

  }

  /* Did we find an open entry? */
  if ( ! Entry_Found )
  {



    *Error_Code = DLIST_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("Add_Partitions_To_Aggregate")

    return;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"The Aggregate's original size was %d (decimal).\nThe Aggregate's original usable size was %d (decimal)", Aggregate->Partition_Size, Aggregate->Usable_Size);
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Update the Aggregate. */
  Aggregate->Partition_Size += Signature_Sector->Partition_Size_To_Report_To_User;
  Aggregate->Usable_Size += Signature_Sector->Partition_Size_To_Report_To_User;

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"The Aggregate's new size is %d (decimal).\nThe Aggregate's new usable size is %d (decimal)", Aggregate->Partition_Size, Aggregate->Usable_Size);
    LVM_Common_Services->Write_Log_Buffer();

    sprintf(LVM_Common_Services->Log_Buffer,"Adding Partition to the Partition_List for this Aggregate.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now add the new partition to the partition list for this aggregate. */
  LVM_Common_Services->InsertObject(Aggregate->Feature_Data->Partitions, sizeof(Partition_Data), New_Partition, PARTITION_DATA_TAG, NULL, AppendToList, FALSE, Error_Code);

  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
    {

      if ( LVM_Common_Services->Logging_Enabled )
      {

        sprintf(LVM_Common_Services->Log_Buffer,"AppendObject failed due to lack of memory!");
        LVM_Common_Services->Write_Log_Buffer();

      }

      /* Abort! */
      *Error_Code = DLIST_OUT_OF_MEMORY;

      FEATURE_FUNCTION_EXIT("Add_Partitions_To_Aggregate")

      return;

    }

    /* Abort! */
    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("Add_Partitions_To_Aggregate")

    return;

  }

  if ( LVM_Common_Services->Logging_Enabled )
  {

    sprintf(LVM_Common_Services->Log_Buffer,"Updating the Link Table to include this partition.");
    LVM_Common_Services->Write_Log_Buffer();

  }

  /* Now update the LinkTable of the Aggregate. */
  LinkTable->LinkArray[LinkTable->Links_In_Use].PartitionRecord = New_Partition;
  LinkTable->LinkArray[LinkTable->Links_In_Use].Link_Data.Drive_Serial_Number = LVM_Common_Services->DriveArray[New_Partition->Drive_Index].Drive_Serial_Number;
  LinkTable->LinkArray[LinkTable->Links_In_Use].Link_Data.Partition_Serial_Number = New_Partition->DLA_Table_Entry.Partition_Serial_Number;
  LinkTable->Links_In_Use += 1;

  /* Indicate that changes were made. */
  LinkTable->ChangesMade = TRUE;

  /* Indicate success and return. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Add_Partitions_To_Aggregate")

  return;

}


static void _System Fill_In_Comm_Buffer( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize, ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code)
{

  Partition_Data *       PartitionRecord = (Partition_Data *) Object;
  DLink_Comm_Struct *    Comm_Buffer = (DLink_Comm_Struct *) Parameters;

  FEATURE_FUNCTION_ENTRY("Fill_In_Comm_Buffer")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Fill_In_Comm_Buffer")

    return;

  }

#endif

  /* Since the size and TAG of the object are correct, we will proceed. */
  Comm_Buffer->Partition_Array[Comm_Buffer->Count].Partition_Handle = PartitionRecord->External_Handle;
  Comm_Buffer->Partition_Array[Comm_Buffer->Count].Reserved = ObjectHandle;
  Comm_Buffer->Partition_Array[Comm_Buffer->Count].Aggregate_Number = -1;
  Comm_Buffer->Partition_Array[Comm_Buffer->Count].Use_Partition_Name = FALSE;
  Comm_Buffer->Count += 1;

  /* Indicate success and return. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Fill_In_Comm_Buffer")

  return;

}

static BOOLEAN  _System DL_ChangesPending(Partition_Data * PartitionRecord, CARDINAL32 * Error_Code)
{

  Feature_Context_Data  *    CurrentFeature;
  Drive_Link_Array *         LinkTable;
  BOOLEAN                    ReturnValue = FALSE;

  FEATURE_FUNCTION_ENTRY("DL_Changes_Pending")

  /* Is our feature data available? */
  if ( PartitionRecord->Feature_Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("DL_Changes_Pending")

    return FALSE;

  }

  /* Get our feature data. */
  CurrentFeature = PartitionRecord->Feature_Data;

  /* Does our Drive Linking Data exist? */
  if ( CurrentFeature->Data == NULL )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"DL ChangesPending has encountered a partition with bad feature data!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_BAD_PARTITION;

    FEATURE_FUNCTION_EXIT("DL_Changes_Pending")

    return FALSE;

  }

  /* Get our Drive Linking Data. */
  LinkTable = (Drive_Link_Array *) CurrentFeature->Data;

  /* Do we have any changes pending? */
  if ( LinkTable->ChangesMade )
  {

    *Error_Code = LVM_ENGINE_NO_ERROR;

    FEATURE_FUNCTION_EXIT("DL_Changes_Pending")

    return TRUE;

  }

  /* Since we don't have any changes pending, we must find out if the next layer below us does.  This means traversing
     our list of partitions for this aggregate and, for each partition in the list, calling the ChangesPending function. */
  LVM_Common_Services->ForEachItem(CurrentFeature->Partitions,&Continue_Changes_Pending, &ReturnValue, TRUE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( LVM_Common_Services->Logging_Enabled )
    {

      sprintf(LVM_Common_Services->Log_Buffer,"ForEachItem has failed in DL ChangesPending!");
      LVM_Common_Services->Write_Log_Buffer();

    }

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("DL_Changes_Pending")

    return FALSE;

  }


  FEATURE_FUNCTION_EXIT("DL_Changes_Pending")

  /* *Error_Code was set by the call to ChangesPending, so leave it alone. */
  return ReturnValue;

}


static void _System DL_ParseCommandLineArguments(DLIST Token_List, LVM_Classes * Actual_Class, ADDRESS * Init_Data, char ** Error_Message, CARDINAL32 * Error_Code )
{
  DLink_Comm_Struct *           Comm_Buffer;
  DLIST                         Aggregate_Names;        /* Used to hold a list of the names specified for aggregates. */
  DLIST                         Potential_Aggregates;   /* Used to hold a list of the partitions and aggregates which have already been parsed. */
  CARDINAL32                    Aggregate_Count = 0;    /* Used to track how many aggregates have been parsed. */
  LVM_Token *                   Aggregate_Name_Token;   /* Used to hold the token containing the name of the aggregrate being constructed. */
  LVM_Token *                   Token;                  /* Used to point to the token being parsed. */
  LVM_Token *                   Lookahead_Token;        /* Used when look ahead is required to resolve a parsing decision. */
  Aggregate_Name_Search_Record  Duplicate_Name_Check;   /* Used to check if an aggregate name has been specified more than once. */
  BOOLEAN                       Partition_List_Complete;/* Used to determine the end of a partition list specification. */


  FEATURE_FUNCTION_ENTRY("DL_ParseCommandLineArguments")

  /* Set up default return values. */
  *Init_Data = NULL;
  *Error_Message = NULL;
  *Actual_Class = Aggregate_Class;

  /* Initialize the DLISTs we need. */
  Aggregate_Names = LVM_Common_Services->CreateList();
  Potential_Aggregates = LVM_Common_Services->CreateList();
  if ( ( Aggregate_Names == NULL ) ||
       ( Potential_Aggregates == NULL )
     )
  {

    if ( Aggregate_Names != NULL )
      LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
    if ( Potential_Aggregates != NULL )
      LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;

    FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

    return;

  }

  /* Get the current token. */
  Token = GetToken(Token_List, TRUE, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
    LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

    if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
      *Error_Message = UNEXPECTED_END_OF_INPUT;

    FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

    return;

  }

  /* Is the token "ALL"?  If it is, we must look ahead to see if an "=" sign follows it, in which case it becomes
     an LVM_String instead of the command "ALL".                                                                    */
  if ( Token->TokenType == LVM_All )
  {

    /* Lookahead one token to see if that token is an equal sign.  If so, then we will treat this "all" as a string instead of a command. */
    Lookahead_Token = LookAhead(Token_List, 1, Error_Code);
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

      return;

    }

    if ( Lookahead_Token == NULL )
    {

      /* Clean up. */
      LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
      LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

      /* We have an unexpected end of input problem! */
      *Error_Message = UNEXPECTED_END_OF_INPUT2;
      *Error_Code = LVM_ENGINE_PARSING_ERROR;

      FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

      return;

    }

    if ( Lookahead_Token->TokenType == LVM_EQ_Sign )
    {

      /* We have "ALL =" here.  This is the only time that "ALL" is considered an aggregate name instead of a keyword.
         We will recharacterize the token so that it will be treated as a name instead of a key word.                   */
      Token->TokenType = LVM_String;

    }

  }

  /* Is the token "ALL" or the end of Feature Specific Commands token? */
  if ( ( Token->TokenType == LVM_All ) ||
       ( Token->TokenType == LVM_Close_Paren )
     )
  {

    /* Allocate a buffer to hold the minimum initialization data. */
    Comm_Buffer = (DLink_Comm_Struct *) LVM_Common_Services->Allocate( sizeof(DLink_Comm_Struct) );

    if (Comm_Buffer == NULL )
    {

      LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
      LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
      *Error_Message = NULL;

      FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

      return;

    }

    /* Initialize the Count field to the maximum legal value.  This is well above the number of partitions
       that the system can handle, and will thus never be a legitimate value.  We will use this value to
       indicate that the user has instructed us to produce a single aggregate from all of the partitions/aggregates
       passed in to us during volume creation.                                                                       */
    Comm_Buffer->Count = (CARDINAL32) -1L;

    /* Free memory that we don't need anymore. */
    LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
    LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

    /* Advance the token list if the token is not ')'. */
    if ( Token->TokenType != LVM_Close_Paren )
      Token = GetToken(Token_List, FALSE, Error_Code);
    else
      *Error_Code = LVM_ENGINE_NO_ERROR;

    if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
      *Error_Message = UNEXPECTED_END_OF_INPUT2;

    /* Return the Comm_Buffer as the Init_Data buffer. */
    *Init_Data = (ADDRESS) Comm_Buffer;

    FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

    return;

  }

  /* Loop through the commands parsing each command. */
  for (;;)
  {

    /* Is the token a string?  We are looking for an aggregate name here. */
    if ( ( Token->TokenType == LVM_String ) ||
         ( Token->TokenType == LVM_AcceptableCharsStr ) ||
         ( Token->TokenType == LVM_FileNameStr )
       )
    {


      /* Check the aggregate name to see if it is a duplicate of one we have already seen. */
      Duplicate_Name_Check.Name_Found = FALSE;
      Duplicate_Name_Check.Aggregate_Name = Token->TokenText;
      LVM_Common_Services->ForEachItem(Aggregate_Names,&Find_Duplicate_Aggregate_Names,&Duplicate_Name_Check, TRUE, Error_Code);
      if ( *Error_Code != DLIST_SUCCESS )
      {

        /* This should not happen! */
        *Error_Code = LVM_ENGINE_INTERNAL_FEATURE_ERROR;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* If the aggregate name is a duplicate then */
      if ( Duplicate_Name_Check.Name_Found )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        *Error_Code = LVM_ENGINE_PARSING_ERROR;
        *Error_Message = AGGREGATE_NAME_NOT_UNIQUE;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* else add the aggregate name to the aggregate name list and save its handle. */
      LVM_Common_Services->InsertObject(Aggregate_Names, sizeof(LVM_Token), Token, LVM_TOKEN_TAG, NULL, AppendToList, FALSE, Error_Code);
      if ( *Error_Code != DLIST_SUCCESS )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        if ( *Error_Code == DLIST_OUT_OF_MEMORY )
          *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
        else
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* Save the name of the aggregate. */
      Aggregate_Name_Token = Token;

      /* Get the next token. */
      Token = GetToken(Token_List, FALSE, Error_Code);
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
          *Error_Message = EQUAL_SIGN_EXPECTED;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* Is this token an "=" ? */
      if ( Token->TokenType != LVM_EQ_Sign )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        /* We have a parsing error! */
        *Error_Code = LVM_ENGINE_PARSING_ERROR;
        *Error_Message = EQUAL_SIGN_EXPECTED;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* Get the next token. */
      Token = GetToken(Token_List, FALSE, Error_Code);
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
          *Error_Message = BRACE_EXPECTED;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* Is this token a "{" ? */
      if ( Token->TokenType != LVM_Open_Brace )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        /* We have a parsing error! */
        *Error_Code = LVM_ENGINE_PARSING_ERROR;
        *Error_Message = BRACE_EXPECTED;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* Get the next token. */
      Token = GetToken(Token_List, FALSE, Error_Code);
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
          *Error_Message = DRIVE_ID_EXPECTED;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* Now we must parse partition specifiers. */
      Partition_List_Complete = FALSE;
      do
      {

        /* Parse a Partition Specifier. */
        Parse_Partition_List(Token_List, Potential_Aggregates, Aggregate_Count, Aggregate_Name_Token, Error_Message, Error_Code);

        /* If we failed to parse a partition specifier, then we need to abort.  Error_Message and
           Error_Code will be set by Parse_Partition_List if an error is encountered.              */
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
          LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

          FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

          return;

        }

        /* Get the current token in the token list.  This should be the first token after the partition specification that was parsed. */
        Token = GetToken(Token_List, TRUE, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
          LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

          if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
            *Error_Message = NEW_PARTITION_EXPECTED;

          FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

          return;

        }

        /* Is the current token a comma or } ? */
        if ( Token->TokenType == LVM_Comma )
        {

          /* Another partition is expected to be specified.  Advance to the next token. */
          Token = GetToken(Token_List, FALSE, Error_Code);
          if ( *Error_Code != LVM_ENGINE_NO_ERROR )
          {

            LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
            LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

            if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
              *Error_Message = DRIVE_ID_EXPECTED;

            FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

            return;

          }

        }
        else
          if ( Token->TokenType == LVM_Close_Brace )
          {

            /* The end of the partition list specification is here! */
            Partition_List_Complete = TRUE;

          }
          else
          {

            /* We have an invalid token!  Output an error message! */
            LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
            LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

            *Error_Code = LVM_ENGINE_PARSING_ERROR;
            *Error_Message = NEW_PARTITION_EXPECTED;

            FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

            return;

          }

      } while ( Partition_List_Complete == FALSE );

      /* Increment Aggregate_Count. */
      Aggregate_Count += 1;

      /* Setup to process another aggregate. */
      Aggregate_Name_Token = NULL;

      /* Get the next token. */
      Token = GetToken(Token_List, FALSE, Error_Code);
      if ( *Error_Code != LVM_ENGINE_NO_ERROR )
      {

        LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
        LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

        if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
          *Error_Message = NEW_COMMAND_EXPECTED;

        FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

        return;

      }

      /* If there are more feature specific commands, then there will be a ',' here.  If not, then there
         should be a ')' here.  Anything else is an error.                                                  */
      if ( Token->TokenType == LVM_Comma )
      {

        /* Is the next token a ')'?  If so, we have an extraneous comma here and need to put out an error message. */
        Lookahead_Token = LookAhead(Token_List, 1, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

          return;

        }

        if ( Lookahead_Token == NULL )
        {

          /* Clean up. */
          LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
          LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

          /* We have an unexpected end of input problem! */
          *Error_Message = MISSING_AGGREGATE_NAME;
          *Error_Code = LVM_ENGINE_PARSING_ERROR;

          FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

          return;

        }

        /* Is the token "ALL" ?  If so, then we must convert it to a string as "ALL" is allowed here as an aggregate name, not a key word. */
        if ( Lookahead_Token->TokenType == LVM_All )
        {

          Lookahead_Token->TokenType = LVM_String;

        }

        /* Is the token a string?  We are looking for an aggregate name here. */
        if ( ( Lookahead_Token->TokenType != LVM_String ) &&
             ( Lookahead_Token->TokenType != LVM_AcceptableCharsStr ) &&
             ( Lookahead_Token->TokenType != LVM_FileNameStr )
           )
        {

          /* We should have had an aggregate name next.  Since we don't, we have an error! */
          /* Clean up. */
          LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
          LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

          /* We have an unexpected end of input problem! */
          *Error_Message = MISSING_AGGREGATE_NAME;
          *Error_Code = LVM_ENGINE_PARSING_ERROR;

          FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

          return;

        }

        /* Since we found what we expected, lets get the next token and continue parsing. */
        Token = GetToken(Token_List, FALSE, Error_Code);
        if ( *Error_Code != LVM_ENGINE_NO_ERROR )
        {

          /* This should not have failed since LookAhead successfully performed this operation just a short time ago! */
          *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

          FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

          return;

        }

      }

    }
    else
    {

      /* Is the current token the end of Feature Specific Commands token? */

      if ( Token->TokenType == LVM_Close_Paren )
      {

        /* Since this is the end of Feature Specific Commands, break out of the loop. */
        break;
      }
      else
      {

        /* else output an error message about needing an aggregate name. */
        *Error_Code = LVM_ENGINE_PARSING_ERROR;
        *Error_Message = AGGREGATE_NAME_EXPECTED;
        break;

      }

    }

  }

  /* Now we must convert the contents of the Potential_Aggregates list into a buffer of initialization data. */

  /* How many items are in the Potential_Aggregates list? */

  /* Get the number of items in the Potential_Aggregates list.  */
  Aggregate_Count = LVM_Common_Services->GetListSize(Potential_Aggregates, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    /* We have an internal error! */
    *Error_Code = LVM_ENGINE_INTERNAL_FEATURE_ERROR;
    *Error_Message = NULL;

    FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

    return;

  }

  /* Allocate memory for an init. buffer that can hold all of the entries in the Potential_Aggregates list. */
  Comm_Buffer = (DLink_Comm_Struct * ) LVM_Common_Services->Allocate( ( ( Aggregate_Count - 1 ) * sizeof(DLink_Partition_Array_Entry) ) + sizeof(DLink_Comm_Struct) );

  if ( Comm_Buffer == NULL )
  {

    /* We ran out of memory! */
    *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    *Error_Message = NULL;

    LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
    LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

    FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

    return;

  }

  /* Now we must copy the data from the Potential_Aggregates list into the Comm_Buffer. */
  Comm_Buffer->Count = 0;
  LVM_Common_Services->ForEachItem(Potential_Aggregates, &Transfer_To_Comm_Buffer, Comm_Buffer, TRUE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    LVM_Common_Services->DestroyList(Aggregate_Names, TRUE, Error_Code);
    LVM_Common_Services->DestroyList(Potential_Aggregates, TRUE, Error_Code);

    *Error_Code = LVM_ENGINE_INTERNAL_FEATURE_ERROR;

    FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

    return;

  }

  /* Return the Comm_Buffer as the Init_Data buffer. */
  *Init_Data = (ADDRESS) Comm_Buffer;

  /* Cleanup */

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("DL_ParseCommandLineArguments")

  return;

}


static void _System Find_Duplicate_Aggregate_Names( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code)
{

  LVM_Token *                       Token = (LVM_Token *) Object;
  Aggregate_Name_Search_Record *    Search_Data = (Aggregate_Name_Search_Record *) Parameters;

  FEATURE_FUNCTION_ENTRY("Find_Duplicate_Aggregate_Names")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != LVM_TOKEN_TAG ) || ( ObjectSize != sizeof(LVM_Token) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Find_Duplicate_Aggregate_Names")

    return;

  }

#endif

  /* Since the size and TAG of the object are correct, we will proceed. */

  /* Does the text in the token match that in the Search_Data? */
  if ( strncmp(Token->TokenText, Search_Data->Aggregate_Name, PARTITION_NAME_SIZE) == 0 )
  {

    /* We have a match!  Indicate the match and return. */
    Search_Data->Name_Found = TRUE;

    *Error_Code = DLIST_SEARCH_COMPLETE;

  }
  else
  {

    /* Indicate success and return. */
    *Error_Code = DLIST_SUCCESS;

  }

  FEATURE_FUNCTION_EXIT("Find_Duplicate_Aggregate_Names")

  return;

}


static void Parse_Partition_List( DLIST  Token_List, DLIST  Potential_Aggregates, CARDINAL32 Current_Aggregate, LVM_Token * Aggregate_Name_Token, char **  Error_Message, CARDINAL32 *  Error_Code)
{

  DLink_Partition_Array_Entry   Partition_Data;
  LVM_Token *                   Token;
  CARDINAL32                    Drive_Number;
  char *                        Not_Needed;       /* Used with strtoul. */

  FEATURE_FUNCTION_ENTRY("Parse_Partition_List")

  /* Do some initial setup on Partition_Data. */
  Partition_Data.Partition_Handle = NULL;
  Partition_Data.Reserved = NULL;
  Partition_Data.Aggregate_Number = Current_Aggregate;
  strncpy(Partition_Data.Aggregate_Name, Aggregate_Name_Token->TokenText, PARTITION_NAME_SIZE);


  /* We are expecting either a drive number or a drive name. */

  /* Get the current token. */
  Token = GetToken(Token_List, TRUE, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
      *Error_Message = DRIVE_ID_EXPECTED;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* Do we have a drive number or drive name? */
  switch ( Token->TokenType )
  {
    case LVM_FileNameStr :
    case LVM_AcceptableCharsStr :
    case LVM_String : /* We have a drive name. */

                      /* Begin constructing the partition data for this partition. */
                      Partition_Data.Use_Drive_Number = FALSE;
                      Partition_Data.Drive_Number = (CARDINAL32) -1L;
                      strncpy(Partition_Data.Drive_Name,Token->TokenText, DISK_NAME_SIZE);

                      break;

    case LVM_Number : /* We have a drive number. */

                      /* Convert the text of the token into a number. */
                      Drive_Number = strtoul(Token->TokenText, &Not_Needed, 10);

                      /* Is the drive number legal? */
                      if ( ( Drive_Number == 0 ) ||
                           ( Drive_Number > LVM_Common_Services->DriveCount )
                         )
                      {

                        /* The drive number is out of bounds! */
                        *Error_Message = INVALID_DRIVE_NUMBER_SPECIFIED;
                        *Error_Code = LVM_ENGINE_PARSING_ERROR;

                        FEATURE_FUNCTION_EXIT("Parse_Partition_List")

                        return;

                      }

                      /* Save the drive number. */
                      Partition_Data.Use_Drive_Number = TRUE;
                      Partition_Data.Drive_Number = Drive_Number - 1;
                      Partition_Data.Drive_Name[0] = 0;

                      break;

    default : /* We have a problem! */
              *Error_Message = DRIVE_ID_EXPECTED;
              *Error_Code = LVM_ENGINE_PARSING_ERROR;

              FEATURE_FUNCTION_EXIT("Parse_Partition_List")

              return;

  }

  /* Advance to the next token. */
  Token = GetToken(Token_List, FALSE, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
      *Error_Message = COMMA_EXPECTED;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* Do we have the comma that we expect? */
  if ( Token->TokenType != LVM_Comma )
  {

    *Error_Code = LVM_ENGINE_PARSING_ERROR;
    *Error_Message = COMMA_EXPECTED;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* Advance to the next token. */
  Token = GetToken(Token_List, FALSE, Error_Code);
  if ( *Error_Code != LVM_ENGINE_NO_ERROR )
  {

    if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
      *Error_Message = PARTITION_NAME_EXPECTED;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* Do we have a partition name? */
  if ( ( Token->TokenType != LVM_String ) &&
       ( Token->TokenType != LVM_AcceptableCharsStr ) &&
       ( Token->TokenType != LVM_FileNameStr )
     )
  {

    *Error_Code = LVM_ENGINE_PARSING_ERROR;
    *Error_Message = PARTITION_NAME_EXPECTED;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* We have a name.  Is the name too big? */
  if ( strlen(Token->TokenText) > PARTITION_NAME_SIZE )
  {

    *Error_Code = LVM_ENGINE_PARSING_ERROR;
    *Error_Message = PARTITION_NAME_TOO_LONG;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* We have a partition name.  Put it in the Partition_Data. */
  strncpy(Partition_Data.Partition_Name, Token->TokenText, PARTITION_NAME_SIZE);

  /* Indicate that the partition name should be used. */
  Partition_Data.Use_Partition_Name = TRUE;

  /* Now the Partition_Data structure is complete.  Add it to the list of potential aggregates. */
  LVM_Common_Services->InsertItem(Potential_Aggregates, sizeof(DLink_Partition_Array_Entry), &Partition_Data, DLINK_PARTITION_ARRAY_ENTRY_TAG, NULL, AppendToList, FALSE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( *Error_Code == DLIST_OUT_OF_MEMORY )
      *Error_Code = LVM_ENGINE_OUT_OF_MEMORY;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    *Error_Message = NULL;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* Advance to the next token. */
  Token = GetToken(Token_List, FALSE, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( *Error_Code == LVM_ENGINE_PARSING_ERROR )
      *Error_Message = NEW_PARTITION_EXPECTED;

    FEATURE_FUNCTION_EXIT("Parse_Partition_List")

    return;

  }

  /* Indicate success. */
  *Error_Code = LVM_ENGINE_NO_ERROR;

}


static void _System Transfer_To_Comm_Buffer( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code)
{

  DLink_Partition_Array_Entry *  Partition_Data = (DLink_Partition_Array_Entry *) Object;
  DLink_Comm_Struct *            Comm_Buffer = (DLink_Comm_Struct *) Parameters;

  FEATURE_FUNCTION_ENTRY("Transfer_To_Comm_Buffer")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != DLINK_PARTITION_ARRAY_ENTRY_TAG ) || ( ObjectSize != sizeof(DLink_Partition_Array_Entry) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Transfer_To_Comm_Buffer")

    return;

  }

#endif

  /* Since the size and TAG of the object are correct, we will proceed. */
  Comm_Buffer->Partition_Array[Comm_Buffer->Count] = *(Partition_Data);

  /* Increment the counter. */
  Comm_Buffer->Count += 1;

  /* Indicate success. */
  *Error_Code = DLIST_SUCCESS;

  FEATURE_FUNCTION_EXIT("Transfer_To_Comm_Buffer")

  return;

}


static LVM_Token * GetToken(DLIST Token_List, BOOLEAN CurrentToken, CARDINAL32 * Error_Code)
{

  LVM_Token *  Token;

  FEATURE_FUNCTION_ENTRY("GetToken")

  /* Does the user want to start with the current token or the next token? */
  if ( CurrentToken )
  {

    /* Get the current token. */
    Token = (LVM_Token *) LVM_Common_Services->GetObject(Token_List, sizeof(LVM_Token), LVM_TOKEN_TAG, NULL, FALSE, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( *Error_Code == DLIST_END_OF_LIST )
        *Error_Code = LVM_ENGINE_PARSING_ERROR;
      else
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("GetToken")

      return NULL;

    }

  }
  else
  {

    /* Get the next token. */
    Token = (LVM_Token *) LVM_Common_Services->GetNextObject(Token_List, sizeof(LVM_Token), LVM_TOKEN_TAG, Error_Code);
    if ( *Error_Code != DLIST_SUCCESS )
    {

      if ( *Error_Code == DLIST_END_OF_LIST )
        *Error_Code = LVM_ENGINE_PARSING_ERROR;
      else
        *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

      FEATURE_FUNCTION_EXIT("GetToken")

      return NULL;

    }

  }

  /* Reject whitespace. */
  while ( ( *Error_Code == DLIST_SUCCESS ) &&
          ( ( Token->TokenType == LVM_Tab ) ||
            ( Token->TokenType == LVM_MultiTab ) ||
            ( Token->TokenType == LVM_MultiSpace) ||
            ( Token->TokenType == LVM_Space )
          )
        )
  {

    /* Get the next token. */
    Token = (LVM_Token *) LVM_Common_Services->GetNextObject(Token_List, sizeof(LVM_Token), LVM_TOKEN_TAG, Error_Code);

  }

  /* Did we leave the while loop because of an error? */
  if ( *Error_Code != DLIST_SUCCESS )
  {

    if ( *Error_Code == DLIST_END_OF_LIST )
      *Error_Code = LVM_ENGINE_PARSING_ERROR;
    else
      *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

  }
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("GetToken")

  return Token;

}


static LVM_Token * LookAhead(DLIST Token_List, CARDINAL32  Count, CARDINAL32 * Error_Code)
{

  ADDRESS     Current_Position;      /* The handle of the current token in the Token_List. */
  LVM_Token * Current_Token = NULL;  /* The token to be returned. */

  FEATURE_FUNCTION_ENTRY("LookAhead")

  /* Get the handle of the current token in the token list. */
  Current_Position = LVM_Common_Services->GetHandle(Token_List, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;

    FEATURE_FUNCTION_EXIT("LookAhead")

    return NULL;

  }

  /* Now begin to look ahead! */
  do
  {

    Current_Token = GetToken(Token_List, FALSE, Error_Code);
    if ( *Error_Code != LVM_ENGINE_NO_ERROR )
    {

      FEATURE_FUNCTION_EXIT("LookAhead")

      return NULL;

    }

    Count -= 1;

  } while ( Count > 0  );

  /* We must reposition Token_List so that the current item in Token_List is what it was before this function was called. */
  LVM_Common_Services->GoToSpecifiedItem(Token_List, Current_Position, Error_Code);
  if ( *Error_Code != DLIST_SUCCESS )
  {

    *Error_Code = LVM_ENGINE_INTERNAL_ERROR;
    Current_Token = NULL;

  }
  else
    *Error_Code = LVM_ENGINE_NO_ERROR;

  FEATURE_FUNCTION_EXIT("LookAhead")

  return Current_Token;

}


static void _System Find_Partition_By_Name( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code)
{

  Partition_Data *               PartitionRecord = (Partition_Data *) Object;
  DLink_Partition_Array_Entry *  Partition_Array_Entry = (DLink_Partition_Array_Entry *) Parameters;

  FEATURE_FUNCTION_ENTRY("Find_Partition_By_Name")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Find_Partition_By_Name")

    return;

  }

#endif

  /* Since the size and TAG of the object are correct, we will proceed. */

  /* Assume success. */
  *Error_Code = DLIST_SUCCESS;

  /* Does drive on which this partition resides match that specified in the Partition_Array_Entry? */
  if ( Partition_Array_Entry->Use_Drive_Number && ( PartitionRecord->Drive_Index != Partition_Array_Entry->Drive_Number ) )
  {

    FEATURE_FUNCTION_EXIT("Find_Partition_By_Name")

    /* Since the drive number does not match, this one can't be a match. */
    return;

  }

  if ( ( ! Partition_Array_Entry->Use_Drive_Number ) &&
       ( strncmp(LVM_Common_Services->DriveArray[Partition_Array_Entry->Drive_Number].Drive_Name, Partition_Array_Entry->Drive_Name, DISK_NAME_SIZE) != 0 )
     )
  {

    FEATURE_FUNCTION_EXIT("Find_Partition_By_Name")

    /* Since the drive number does not match, this one can't be a match. */
    return;

  }

  /* If we get here, then the drive is a match.  Now we must check the partition name. */
  if ( strncmp(PartitionRecord->Partition_Name, Partition_Array_Entry->Partition_Name, PARTITION_NAME_SIZE) == 0 )
  {

    /* We have a match!  Lets keep some of the handles for this partition. */
    Partition_Array_Entry->Partition_Handle = PartitionRecord->External_Handle;
    Partition_Array_Entry->Reserved = ObjectHandle;
    Partition_Array_Entry->Use_Partition_Name = FALSE;

    /* Indicate that we have found what we want. */
    *Error_Code = DLIST_SEARCH_COMPLETE;

  }

  FEATURE_FUNCTION_EXIT("Find_Partition_By_Name")

  return;

}


static void _System Find_Duplicate_ANames( ADDRESS Object, TAG ObjectTag, CARDINAL32 ObjectSize,  ADDRESS ObjectHandle, ADDRESS Parameters,  CARDINAL32 * Error_Code)
{

  Partition_Data *               PartitionRecord = (Partition_Data *) Object;
  Aggregate_Name_Search_Record * Name_Data = (Aggregate_Name_Search_Record *) Parameters;

  FEATURE_FUNCTION_ENTRY("Find_Duplicate_ANames")

#ifdef DEBUG

  /* Is Object what we think it should be? */
  if ( ( ObjectTag != PARTITION_DATA_TAG ) || ( ObjectSize != sizeof(Partition_Data) ) )
  {


#ifdef PARANOID

    assert(0);

#endif


    /* Object's TAG is not what we expected!  Abort! */
    *Error_Code = DLIST_CORRUPTED;

    FEATURE_FUNCTION_EXIT("Find_Duplicate_ANames")

    return;

  }

#endif

  /* Since the size and TAG of the object are correct, we will proceed. */

  /* Assume success. */
  *Error_Code = DLIST_SUCCESS;

  /* Is this an aggregate?  If so, its Drive_Number will be (CARDINAL32) -1L. */
  if ( PartitionRecord->Drive_Index == (CARDINAL32) -1L )
  {

    /* Do the names match? */
    if ( strncmp(PartitionRecord->Partition_Name, Name_Data->Aggregate_Name, PARTITION_NAME_SIZE) == 0 )
    {

      Name_Data->Name_Found = TRUE;

      *Error_Code = DLIST_SEARCH_COMPLETE;

    }

  }

  FEATURE_FUNCTION_EXIT("Find_Duplicate_ANames")

  return;

}

