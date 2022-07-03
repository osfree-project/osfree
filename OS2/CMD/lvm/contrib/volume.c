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
 * Module: volume.c
 */

/*
 * Change History:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "strings.h"
#include "help.h"

#include "LVM_Interface.h"
#include "lvm2.h"

#define AN_UNLIMITED_NUMBER            -1
#define INVALID_HANDLE                  NULL

uint GetDriveNumber ( ADDRESS    drive_handle );
uint GetPartitionHandles ( uint    ***partition_handles,
                      uint    *partition_count,
                      uint    max_partitions,
                      bool    bootable,
                      bool    primary,
                      bool    PRM_only );

int SetBootmanOptions(ADDRESS  volume_handle);
extern bool  Install_time;

int GetBytesPerSector(char DriveLetter);
/*
 * LVM Engine structures
 */

PRIVATE
Volume_Control_Array        Volume;



/*
 * Volume_panel declarations
 */

PRIVATE
panel_t     Volume_panel,                   /* base level panels */
            Volume_header_panel;

PRIVATE
panel_t     Volume_options_menu,            /* menu panels */
            Show_volume_panel,
            Delete_volume_panel,
            Drive_change_panel,
            Drive_letter_panel,
            Bootable_panel,
            Volume_type_panel,
            Volume_name_panel;

PRIVATE
panel_t     Volume_select_feature_menu, Volume_order_feature_menu,
            Volume_aggregates_header_panel, Volume_aggregates_panel,
            Aggregate_features_header_panel, Aggregate_features_panel,
            Aggregate_feature_options_menu;



/*
 * Partitions_panel declarations
 */

PRIVATE
panel_t     Partitions_panel,
            Partitions_header_panel;

PRIVATE
panel_t     Partitions_options_menu;                         /* menu panels */


/*
 * display text_line control and arrays
 */

#define MAX_VOLUMES             24

PRIVATE
char        *Volume_panel_text [ MAX_VOLUMES + 1 ];

PRIVATE
char        **Partitions_panel_text [ MAX_VOLUMES + 1 ];

PRIVATE
uint        Total_volumes = 0;                          /* currently defined */

#ifndef MAX_FEATURES_PER_VOLUME
#define MAX_FEATURES_PER_VOLUME 10
#endif
PRIVATE
char        *Volume_feature_text [MAX_FEATURES_PER_VOLUME];
uint        Volume_feature_state [MAX_FEATURES_PER_VOLUME];
uint        Volume_feature_menu_cursor;

CARDINAL32  Total_features = 0;
CARDINAL32  Total_selected_features = 0;
/*uint        original_index[MAX_FEATURES_PER_VOLUME]; */
Feature_Information_Array feature_info = {0};
LVM_Feature_Specification_Record spec_selected_features[MAX_FEATURES_PER_VOLUME];

#define MAX_AGGREGATES 128 /* == max_partitions */
PRIVATE
char        *Volume_aggregates_panel_text [MAX_AGGREGATES];
PRIVATE
char        *Aggregate_features_panel_text [MAX_AGGREGATES][MAX_FEATURES_PER_VOLUME];

LVM_Handle_Array_Record record_of_aggregate_handles = {0};
Feature_Information_Array record_of_aggregate_features[MAX_AGGREGATES] = {0};

typedef struct _Class_Index_Boundary {
   int count;
   int low_index;
   int high_index;
} Class_Index_Boundary;

Class_Index_Boundary Order_features_class_boundary[MAXIMUM_LVM_CLASSES];


typedef struct _Selected_Feature {  /* this structure combines LVM_Feature_Specification_Record & Feature_ID_Data */
   LVM_Feature_Specification_Record  spec;  /* needed to call LVM_Create */
   Feature_ID_Data                  *pData; /* Pointer to the feature specific data */
} Selected_Feature;

Selected_Feature vio_selected_features[MAX_FEATURES_PER_VOLUME];

/*
 * display text_line routines
 */


/*
 * make_partitions_line sprintfs a text line for the Partitions_panel_text
 * array.
 * Reuse the current line, or allocate one if it is NULL.
 */

PRIVATE
char *
make_partitions_line ( char    *line,
                       char    *partitions_name,
                       uint    size,
                       char    *disk_name )
{
    if ( line == NULL ) {
        line = AllocateOrQuit ( LINE_LENGTH, sizeof (char) );
    }

    sprintf ( line, "%-*.*s %*u %-*.*s %-*.*s",
              PARTITIONS_NAME_WIDTH, PARTITIONS_NAME_WIDTH, partitions_name,
              PARTITIONS_SIZE_WIDTH, size,
              PARTITIONS_NULL_WIDTH, PARTITIONS_NULL_WIDTH, Blank_line,
              DISK_NAME_WIDTH, DISK_NAME_WIDTH, disk_name );

    return  line;
}


/*
 * update_partitions_lines updates the Partitions_panel_text array entry
 * for the volume.
 */

PRIVATE
void
update_partitions_lines ( uint    volume )
{
    register
    char    **line;
    uint    index;
    char    *partition_name;
    uint    drive_number,
            size;
    char    *disk_name;
    register
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error;
    Partition_Information_Array     info;

    if ( Total_volumes == 0 ) {
        return;                                                   /* early exit */
    }

    info = Get_Partitions ( Volume.Volume_Control_Data [volume].Volume_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_volume_data );
    }
    record = info.Partition_Array;
    memory = record;

    line = Partitions_panel_text [ volume ];
    if(line)
      for ( ;  *line;  ++line ) {
           free ( *line );                                     /* erase it */
          *line = NULL;
      }

    line = Partitions_panel_text [ volume ];
    line = ReallocStringArray ( line, info.Count );
    Partitions_panel_text [ volume ] = line;
    Partitions_panel.text_line = Partitions_panel_text [ volume ];


    line = Partitions_panel_text [ volume ];

    for ( index = 0;  index < info.Count;  ++index, ++line, ++record ) {
        size = record->Usable_Partition_Size / SECTORS_PER_MEG;
        partition_name = record->Partition_Name;
        disk_name = record->Drive_Name;
        if ( *disk_name == '\0' ) {
            drive_number = GetDriveNumber ( record->Drive_Handle );
            sprintf ( disk_name, "[ %u ]", drive_number );
        }

        *line = make_partitions_line ( *line, partition_name, size, disk_name );
    }

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

}


/*
 * make_volume_line sprintfs a text line for the Volume_panel_text array.
 * Reuse the current line, or allocate one if it is NULL.
 */

PRIVATE
char *
make_volume_line ( char    *line,
                   char    *volume_name,
                   char    *drive_string,
                   char    *volume_type,
                   char    *volume_status,
                   char    *file_system,
                   uint    size )
{
    if ( line == NULL ) {
        line = AllocateOrQuit ( LINE_LENGTH, sizeof (char) );
    }

    sprintf ( line, "%-*.*s %-*.*s %-*.*s %-*.*s %-*.*s %*u",
              VOLUME_NAME_WIDTH, VOLUME_NAME_WIDTH, volume_name,
              VOLUME_DRIVE_WIDTH, VOLUME_DRIVE_WIDTH, drive_string,
              VOLUME_TYPE_WIDTH, VOLUME_TYPE_WIDTH, volume_type,
              VOLUME_STATUS_WIDTH, VOLUME_STATUS_WIDTH, volume_status,
              VOLUME_FS_WIDTH, VOLUME_FS_WIDTH, file_system,
              VOLUME_SIZE_WIDTH, size );

    return  line;
}



PRIVATE
void
get_features (void)
{

   int i;
   int len;
   CARDINAL32 error=0;

   if (Total_features == 0) /* first time in this function? */
   {
#ifdef TEST
      feature_info.Count = 5;
      feature_info.Feature_Data = AllocateOrQuit(5, sizeof(Feature_ID_Data));

      /*name */
      strcpy(feature_info.Feature_Data[4].Name, "Global Exclusive Plug-in");
      strcpy(feature_info.Feature_Data[3].Name, "Vinca Vol. or Part. Mirroring");
      strcpy(feature_info.Feature_Data[2].Name, "IBM Drive Linking");
      strcpy(feature_info.Feature_Data[1].Name, "Vinca Partition Encryption");
      strcpy(feature_info.Feature_Data[0].Name, "Class Exclusive Plug-in");

      /*ids */
      feature_info.Feature_Data[4].ID = 1004;
      feature_info.Feature_Data[3].ID = 1003;
      feature_info.Feature_Data[2].ID = 1002;
      feature_info.Feature_Data[1].ID = 1001;
      feature_info.Feature_Data[0].ID = 1000;

      /* preferred class */
      feature_info.Feature_Data[4].Preferred_Class = Volume_Class;
      feature_info.Feature_Data[3].Preferred_Class = Partition_Class;
      feature_info.Feature_Data[2].Preferred_Class = Aggregate_Class;
      feature_info.Feature_Data[1].Preferred_Class = Partition_Class;
      feature_info.Feature_Data[0].Preferred_Class = Partition_Class;


      /* Class data */
      feature_info.Feature_Data[4].ClassData[Partition_Class].ClassMember = FALSE;
      feature_info.Feature_Data[4].ClassData[Aggregate_Class].ClassMember = FALSE;
      feature_info.Feature_Data[4].ClassData[Volume_Class].ClassMember = TRUE;
      feature_info.Feature_Data[4].ClassData[Volume_Class].GlobalExclusive = TRUE;
      feature_info.Feature_Data[4].ClassData[Volume_Class].ClassExclusive = FALSE;
      feature_info.Feature_Data[4].ClassData[Volume_Class].TopExclusive = FALSE;
      feature_info.Feature_Data[4].ClassData[Volume_Class].BottomExclusive = FALSE;
      feature_info.Feature_Data[4].ClassData[Volume_Class].Weight_Factor = 0;

      feature_info.Feature_Data[3].ClassData[Partition_Class].ClassMember = TRUE;
      feature_info.Feature_Data[3].ClassData[Partition_Class].GlobalExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Partition_Class].ClassExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Partition_Class].TopExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Partition_Class].BottomExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Partition_Class].Weight_Factor = 50;
      feature_info.Feature_Data[3].ClassData[Aggregate_Class].ClassMember = FALSE;
      feature_info.Feature_Data[3].ClassData[Volume_Class].ClassMember = TRUE;
      feature_info.Feature_Data[3].ClassData[Volume_Class].GlobalExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Volume_Class].ClassExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Volume_Class].TopExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Volume_Class].BottomExclusive = FALSE;
      feature_info.Feature_Data[3].ClassData[Volume_Class].Weight_Factor = 50;

      feature_info.Feature_Data[2].ClassData[Partition_Class].ClassMember = FALSE;
      feature_info.Feature_Data[2].ClassData[Aggregate_Class].ClassMember = TRUE;
      feature_info.Feature_Data[2].ClassData[Aggregate_Class].GlobalExclusive = FALSE;
      feature_info.Feature_Data[2].ClassData[Aggregate_Class].ClassExclusive = FALSE;
      feature_info.Feature_Data[2].ClassData[Aggregate_Class].TopExclusive = TRUE;
      feature_info.Feature_Data[2].ClassData[Aggregate_Class].BottomExclusive= FALSE;
      feature_info.Feature_Data[2].ClassData[Aggregate_Class].Weight_Factor = 0;
      feature_info.Feature_Data[2].ClassData[Volume_Class].ClassMember = FALSE;

      feature_info.Feature_Data[1].ClassData[Partition_Class].ClassMember = TRUE;
      feature_info.Feature_Data[1].ClassData[Partition_Class].GlobalExclusive = FALSE;
      feature_info.Feature_Data[1].ClassData[Partition_Class].ClassExclusive = FALSE;
      feature_info.Feature_Data[1].ClassData[Partition_Class].TopExclusive = FALSE;
      feature_info.Feature_Data[1].ClassData[Partition_Class].BottomExclusive = TRUE;
      feature_info.Feature_Data[1].ClassData[Partition_Class].Weight_Factor = 0;
      feature_info.Feature_Data[1].ClassData[Aggregate_Class].ClassMember = FALSE;
      feature_info.Feature_Data[1].ClassData[Volume_Class].ClassMember = FALSE;

      feature_info.Feature_Data[0].ClassData[Partition_Class].ClassMember = TRUE;
      feature_info.Feature_Data[0].ClassData[Partition_Class].GlobalExclusive = FALSE;
      feature_info.Feature_Data[0].ClassData[Partition_Class].ClassExclusive = TRUE;
      feature_info.Feature_Data[0].ClassData[Partition_Class].TopExclusive = FALSE;
      feature_info.Feature_Data[0].ClassData[Partition_Class].BottomExclusive = TRUE;
      feature_info.Feature_Data[0].ClassData[Partition_Class].Weight_Factor = 0;
      feature_info.Feature_Data[0].ClassData[Aggregate_Class].ClassMember = FALSE;
      feature_info.Feature_Data[0].ClassData[Volume_Class].ClassMember = FALSE;

#else
      feature_info = Get_Available_Features(&error);
      if ( error ) {
         DoEngineErrorPanel ( error );
      }
#endif


      Total_features = feature_info.Count;
      for (i=0; i<Total_features; i++)
      {
         len = strlen(feature_info.Feature_Data[i].Name);
         Volume_feature_text[i] = AllocateOrQuit( len+3, sizeof(char));
         Volume_feature_text[i][0] = ' ';
         strcpy(&Volume_feature_text[i][1],feature_info.Feature_Data[i].Name);
         Volume_feature_text[i][len+1] = ' ';
         Volume_feature_text[i][len+2] = 0;   /* new eol */
      }

   } else {
      /* set all entries for selectable */
      for (i=0; i<Total_features; i++)
      {
         Volume_feature_text[i][0] = ' '; /*selectable */
        /*must show that <drive linking> is selected by default */
      }
   }

   Volume_feature_state[0] = (ACTION_HIGHLIGHT | STATE_SELECTABLE);
   Volume_feature_menu_cursor = 0;
   for (i=1; i<Total_features; i++) {
      Volume_feature_state[i] = STATE_SELECTABLE;
   }

}

PRIVATE
BOOLEAN
MoveMenuCursor( uint cur_position,
                uint *new_position,
                int  *menu_state,
                int  count,
                BOOLEAN bForward)
{
   int i;
   BOOLEAN success = FALSE;

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

   return (success);
}

/*
 * volume_select_feature_callback manipulates the menu items of the
 * "Select Features for Volume" menu.  This function only handles three
 * keys:  up arrow, down arrow, and space bar.  Other keys are ignored.
 */

PRIVATE
uint _System Volume_select_feature_callback ( panel_t *panel )
{
   uint i;
   BOOLEAN bMultipleClasses;
   LVM_Classes class, ExclusiveClass;

   for (i=0; i<Total_features; i++) {
      Volume_feature_state[i] |= ACTION_NONE;       /* assume nothing to do */
      Volume_feature_state[i] &= ACTION_CLEAR_BITS; /* clear all other action bits */
   }

   switch (panel->choice) {   /* on input, "choice" is the key was pressed by the user */
      case ENTER:
      case ESCAPE:
         return (1);  /* tell UserDefinedMenuPanel to exit */
         break;
      case UP_CURSOR:
      case DOWN_CURSOR:
           MoveMenuCursor(Volume_feature_menu_cursor,
                         &Volume_feature_menu_cursor,
                          (int *)Volume_feature_state,
                          Total_features,
                          (panel->choice == DOWN_CURSOR) ? TRUE : FALSE);
         break;

      case SPACE: /* select or deselect a menu item */
         Volume_feature_state[Volume_feature_menu_cursor] &= ~ACTION_NONE;
         if (Volume_feature_state[Volume_feature_menu_cursor] & STATE_SELECTED) { /* currently selected? */
            Volume_feature_state[Volume_feature_menu_cursor] |= ACTION_DESELECT;
            Volume_feature_state[Volume_feature_menu_cursor] &= ~STATE_SELECTED; /* clear selected */
         } else {
            Volume_feature_state[Volume_feature_menu_cursor] |= ACTION_SELECT;
            Volume_feature_state[Volume_feature_menu_cursor] |= STATE_SELECTED; /* set selected */
         }

         /* Is it global exclusive? */
         if (feature_info.Feature_Data[Volume_feature_menu_cursor].ClassData[Partition_Class].GlobalExclusive ||
             feature_info.Feature_Data[Volume_feature_menu_cursor].ClassData[Aggregate_Class].GlobalExclusive ||
             feature_info.Feature_Data[Volume_feature_menu_cursor].ClassData[Volume_Class].GlobalExclusive )
         {
            if (Volume_feature_state[Volume_feature_menu_cursor] & STATE_SELECTED) {
               /* The user had just selected the global exclusive feature, disable all other features */
               for (i=0; i<Total_features; i++) {
                  if (i != Volume_feature_menu_cursor) {
                     Volume_feature_state[i] &= ~ACTION_NONE;
                     if (Volume_feature_state[i] & STATE_SELECTED) { /* currently selected? */
                        Volume_feature_state[i] |= ACTION_DESELECT;
                        Volume_feature_state[i] &= ~STATE_SELECTED; /* clear selected */
                     }
                     Volume_feature_state[i] |= ACTION_SET_NOT_SELECTABLE;
                     Volume_feature_state[i] &= ~STATE_SELECTABLE;
                  }
               }
            } else {
               /* The user had just deselected the global exclusive feature, re-enable all other features */
               for (i=0; i<Total_features; i++) {
                  if (i != Volume_feature_menu_cursor) {
                     Volume_feature_state[i] &= ~ACTION_NONE;
                     Volume_feature_state[i] |= (ACTION_SET_SELECTABLE | STATE_SELECTABLE);
                  }
               }
            }
         } /* end of global exclusive */
         else {
           /* Is it class exclusive? */
           for (ExclusiveClass = Partition_Class; ExclusiveClass <= Volume_Class; ExclusiveClass++) {
              if (feature_info.Feature_Data[Volume_feature_menu_cursor].ClassData[ExclusiveClass].ClassExclusive) {
                 for (i=0; i<Total_features; i++) {
                    if (i != Volume_feature_menu_cursor) {
                       if (feature_info.Feature_Data[i].ClassData[ExclusiveClass].ClassMember) {
                          /* Found the feature that in the same class as the exclusive class */
                          /* Can this feature be in another class? */
                          bMultipleClasses = FALSE;
                          for (class=Partition_Class; class <= Volume_Class; class++) {
                             if ( class != ExclusiveClass ) {
                                if (feature_info.Feature_Data[i].ClassData[class].ClassMember) {
                                   bMultipleClasses = TRUE;
                                   break;
                                }
                             }
                          }
                          if ( !bMultipleClasses) {
                             Volume_feature_state[i] &= ~ACTION_NONE;
                             if (Volume_feature_state[Volume_feature_menu_cursor] & STATE_SELECTED) {
                                /* selection of an exclusive class */
                                if (Volume_feature_state[i] & STATE_SELECTED) { /* currently selected? */
                                   Volume_feature_state[i] |= ACTION_DESELECT;
                                   Volume_feature_state[i] &= ~STATE_SELECTED; /* clear selected */
                                }
                                Volume_feature_state[i] |= ACTION_SET_NOT_SELECTABLE;
                                Volume_feature_state[i] &= ~STATE_SELECTABLE;
                             } else {
                                /*  de-selection of an exclusive class */
                                Volume_feature_state[i] |= (ACTION_SET_SELECTABLE | STATE_SELECTABLE);
                             }
                          }
                       }
                    }
                 }
                 break;
              }
           }
         }

         break;
      default:
         break;
   }
   panel->choice = Volume_feature_menu_cursor; /* new choice */
   return (0);
}


/*
 * Scan through Volume_feature_text array, find out which feature has been
 * selected to set up the following arrays:
 *    Partition_class_selected_feature_text
 *    Aggregate_class_selected_feature_text
 *    Volume_class_selected_feature_text
 * Show order features panel for each of the class
 * Build spec_selected_features array
 * Return:  0    - to go back to previous panel
 *          else - the count of selelected features
 */

PRIVATE
CARDINAL32
verify_selected_features(void)
{
   uint key;
   uint *state;
   CARDINAL32 i;

   char *Selected_feature_text[MAX_FEATURES_PER_VOLUME] = {0};

   /* initialize total selected features */
   Total_selected_features = 0;
   memset(Order_features_class_boundary,0,sizeof(Order_features_class_boundary));

   /* Get selected features */
   for (i=0, state=Volume_feature_state; i<Total_features; i++, state++) {
      if (*state & STATE_SELECTED) {
         Selected_feature_text[Total_selected_features] = Volume_feature_text[i];
         /* original_index[Total_selected_features] = i; */
         vio_selected_features[Total_selected_features].pData = &(feature_info.Feature_Data[i]);
         vio_selected_features[Total_selected_features].spec.Feature_ID = vio_selected_features[Total_selected_features].pData->ID;
         vio_selected_features[Total_selected_features].spec.Actual_Class = vio_selected_features[Total_selected_features].pData->Preferred_Class;
         ++Total_selected_features;
      }
   }


   if (Total_selected_features > 1) {

      Volume_feature_state[0] = (ACTION_HIGHLIGHT | STATE_SELECTABLE);  /* need work */
      Volume_feature_menu_cursor = 0;                                   /* need work */
      for (i=1; i<Total_selected_features; i++) {                       /* need work */
         Volume_feature_state[i] = STATE_SELECTABLE;                    /* need work */
      }                                                                 /* need work */

      Volume_order_feature_menu.text_line = Selected_feature_text;
      Volume_order_feature_menu.control |= NOT_SIZED; /* re-size every time */
      MESSAGE_BAR_PANEL ( Volume_order_feature_message_line );
      key = UserDefinedMenuPanel( &Volume_order_feature_menu,
                         Total_selected_features,
                         Volume_feature_state,
                         chars_right_arrow_left_arrow[0],  /* use right arrow to mark text */
                         0);
      switch ( key ) {
         case '\r':
            ErasePanel ( &Volume_order_feature_menu );
            break;
         case ESCAPE:
            ErasePanel ( &Volume_order_feature_menu );
            return(0);
            break;
      }
   }

   for (i=0; i<Total_selected_features; i++)
   {
      spec_selected_features[i] = vio_selected_features[i].spec;
   }

   return Total_selected_features;
}

PRIVATE
uint _System Volume_order_feature_callback ( panel_t *panel )
{
   uint i;
   char *temp;
   uint temp_index;
   Selected_Feature temp_feature;
   bool move;
   int  old_cursor;

   for (i=0; i<Total_selected_features; i++) {
      Volume_feature_state[i] |= ACTION_NONE;       /* assume nothing to do */
      Volume_feature_state[i] &= ACTION_CLEAR_BITS; /* clear all other action bits */
   }

   switch (panel->choice) {   /* on input, "choice" is either up arrow or down arrow key */
      case ENTER:
      case ESCAPE:
         return (1);  /* tell UserDefinedMenuPanel to exit */
         break;
      case UP_CURSOR:
      case DOWN_CURSOR:
         old_cursor = Volume_feature_menu_cursor;
         if (MoveMenuCursor(Volume_feature_menu_cursor,
                         &Volume_feature_menu_cursor,
                          (int *)Volume_feature_state,
                          Total_selected_features,
                          (panel->choice == DOWN_CURSOR) ? TRUE : FALSE)) {
            if (Volume_feature_state[old_cursor] & STATE_SELECTED) {
               /* switch 2 text lines */
               temp = panel->text_line[old_cursor];
               panel->text_line[old_cursor] = panel->text_line[Volume_feature_menu_cursor];
               panel->text_line[Volume_feature_menu_cursor] = temp;

               /* switch real feature data */
               temp_feature = vio_selected_features[old_cursor];
               vio_selected_features[old_cursor] = vio_selected_features[Volume_feature_menu_cursor];
               vio_selected_features[Volume_feature_menu_cursor] = temp_feature;

               Volume_feature_state[old_cursor] |= (ACTION_REDRAW | ACTION_DESELECT);
               Volume_feature_state[old_cursor] &= ~STATE_SELECTED;
               Volume_feature_state[Volume_feature_menu_cursor] |= (ACTION_REDRAW | ACTION_SELECT);
               Volume_feature_state[Volume_feature_menu_cursor] |= STATE_SELECTED;
            }
         }

         break;

      case SPACE: /* select or deselect a menu item */
         Volume_feature_state[Volume_feature_menu_cursor] &= ~ACTION_NONE;
         if (Volume_feature_state[Volume_feature_menu_cursor] & STATE_SELECTED) { /* currently selected? */
            Volume_feature_state[Volume_feature_menu_cursor] |= ACTION_DESELECT;
            Volume_feature_state[Volume_feature_menu_cursor] &= ~STATE_SELECTED; /* clear selected */
         } else {
            Volume_feature_state[Volume_feature_menu_cursor] |= ACTION_SELECT;
            Volume_feature_state[Volume_feature_menu_cursor] |= STATE_SELECTED; /* set selected */
         }

      default:
         break;
   }
   panel->choice = Volume_feature_menu_cursor; /* new choice */
   return (0);

}



/*
 * set_available_drive_letters sets the selectable drive letters in
 * the Drive_letter_text array.
 */

PRIVATE
void
set_available_drive_letters ( void )
{
    register
    char    **text_line;
    register
    CARDINAL32      available_map = -1;
    CARDINAL32      reserved_map  =  0;
    CARDINAL32      error;
    int             i;

    Refresh_LVM_Engine( &error );

    if (!error)
        available_map = Get_Available_Drive_Letters ( &error );
    if (!error)
        reserved_map = Get_Reserved_Drive_Letters ( &error );

    if ( error ) {
        DoEngineErrorPanel ( error );
    }
//printf("FLOAT_DRIVE_LETTER_INDEX=%i\n",FLOAT_DRIVE_LETTER_INDEX);

    for ( i = 0, text_line = Drive_letter_text;
          i < FLOAT_DRIVE_LETTER_INDEX;
          ++i, ++text_line ) {

        (*text_line)[0] = available_map & 1  ?  ' ' : NOT_SELECTABLE_CHAR;

        (*text_line)[LEFT_DRIVE_LETTER_ACCENT_OFFSET] = reserved_map & 1  ?
                        LEFT_DRIVE_LETTER_ACCENT_CHAR : ' ';

        (*text_line)[RIGHT_DRIVE_LETTER_ACCENT_OFFSET] = reserved_map & 1 ?
                        RIGHT_DRIVE_LETTER_ACCENT_CHAR : ' ';

        available_map >>= 1;
        reserved_map >>= 1;
    }

    Drive_letter_panel.control |= INITIALIZE_PANEL;

}

/*
 * make_drive_letter formats the proper drive letter information into
 * a given drive string from information in a given Volume_Information_Record.
 */

PRIVATE
void
make_drive_letter( char                        *drive_string,
                   Volume_Information_Record   *pinfo )

{
    memset(drive_string, ' ', VOLUME_DRIVE_WIDTH);

    if (pinfo->Current_Drive_Letter != 0) {
        /* Current drive letter is valid */
        if (pinfo->Current_Drive_Letter != pinfo->Drive_Letter_Preference) {
            drive_string[0] = pinfo->Drive_Letter_Preference;
            drive_string[1] = '-';
            drive_string[2] = '>';
        }
        drive_string[3] = pinfo->Current_Drive_Letter;
        drive_string[4] = ':';
    }
    else {
        /* Current drive letter is not valid */
        drive_string[0] = pinfo->Drive_Letter_Preference;
        drive_string[1] = ':';
    }
}

/*
 * update_volume_line updates the Volume_panel_text array line.
 * Also updates partition lines.
 */

PRIVATE
void
update_volume_line ( uint    volume )
{
    register
    char    *line;
    uint    volume_type = 0,
            drive_number = 0;
    char    *volume_name,
            *file_system;
    char    drive_string[VOLUME_DRIVE_WIDTH];
    uint    size;
    Volume_Control_Record       *memory;
    Volume_Information_Record   info;
    CARDINAL32                  error;

    memory = Volume.Volume_Control_Data;
    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

    Refresh_LVM_Engine( &error );
    if ( error )
        DoEngineErrorPanel ( error );

    Volume = Get_Volume_Control_Data ( &error );
    if ( error ) {
        Quit ( Cannot_get_volume_data );
    }

    Total_volumes = Volume.Count;
    if ( Total_volumes == 0 ) {
        strncpy ( Volume_panel_text [ 0 ], No_volumes_defined, SCREEN_WIDTH );
    } else {
        info = Get_Volume_Information (
                            Volume.Volume_Control_Data [volume].Volume_Handle,
                            &error );
        if ( error ) {
            Quit ( Cannot_get_volume_data );
        }
        volume_name = info.Volume_Name;
        volume_type = ! info.Compatibility_Volume;

        make_drive_letter(drive_string, &info);

        if ( ! strncmp ( info.File_System_Name, "unformatted", 11 ) ) {
            sprintf ( info.File_System_Name, "%-*.*s",
                      VOLUME_FS_WIDTH, VOLUME_FS_WIDTH, None_string );
        }
        file_system = info.File_System_Name;
        size = info.Volume_Size / SECTORS_PER_MEG;
        if ( info.Status == INSTALLABLE_VOLUME  &&  ! Install_time ) {
            info.Status = NO_STATUS;
        }

        line = Volume_panel_text [ volume ];
        line = make_volume_line ( line, volume_name,
                                  drive_string,
                                  Volume_type_strings [ volume_type ],
                                  Volume_status_strings [ info.Status ],
                                  file_system, size );

        Volume_panel_text [ volume ] = line;

        update_partitions_lines ( volume );
    }

    set_available_drive_letters ();

}


/*
 * update_volume_lines updates the full Volume_panel_text array.
 * Also updates partition lines.
 */

PRIVATE
void
update_volume_lines ( void )
{
    register
    char    *line;
    uint    volume,
            volume_type = 0,
            drive_number = 0;
    char    *volume_name,
            *file_system;
    char    drive_string[VOLUME_DRIVE_WIDTH];
    uint    size;
    Volume_Control_Record       *memory;
    Volume_Information_Record   info;
    CARDINAL32                  error;
    int rc, cbSector;
//    Drive_Control_Array        dca;

    memory = Volume.Volume_Control_Data;
    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

    Refresh_LVM_Engine( &error );
    if ( error )
        DoEngineErrorPanel ( error );

    Volume = Get_Volume_Control_Data ( &error );
    if ( error ) {
        Quit ( Cannot_get_volume_data );
    }

    Total_volumes = Volume.Count;
    if ( Total_volumes == 0 ) {
        strncpy ( Volume_panel_text [ 0 ], No_volumes_defined, SCREEN_WIDTH );
    }
//  dca = Get_Drive_Control_Data(&error);

    for ( volume = 0;  volume < Total_volumes;  ++volume ) {
        info = Get_Volume_Information (
                            Volume.Volume_Control_Data [volume].Volume_Handle,
                            &error );
        if ( error ) {
            Quit ( Cannot_get_volume_data );
        }
        volume_name = info.Volume_Name;
        volume_type = ! info.Compatibility_Volume;

        make_drive_letter(drive_string, &info);

        if ( ! strncmp ( info.File_System_Name, "unformatted", 11 ) ) {
            sprintf ( info.File_System_Name, "%-*.*s",
                      VOLUME_FS_WIDTH, VOLUME_FS_WIDTH, None_string );
        }
        file_system = info.File_System_Name;
        cbSector =  GetBytesPerSector(info.Current_Drive_Letter -'A'+1);

/* находим число байт в секторе */
        if(cbSector != 0)
        {
           size = info.Volume_Size / (1024*1024/cbSector);
        } else {
           if(!strcmp(info.File_System_Name,"CDFS"))
                   size = info.Volume_Size / (1024*1024/2048);
           else
                   size = info.Volume_Size / SECTORS_PER_MEG;
        }
        if ( info.Status == INSTALLABLE_VOLUME  &&  ! Install_time ) {
            info.Status = NO_STATUS;
        }

        line = Volume_panel_text [ volume ];
        line = make_volume_line ( line, volume_name,
                                  drive_string,
                                  Volume_type_strings [ volume_type ],
                                  Volume_status_strings [ info.Status ],
                                  file_system, size );

        Volume_panel_text [ volume ] = line;

        update_partitions_lines ( volume );
    }

    set_available_drive_letters ();

}


/*
 * partitions routines
 */


/*
 * do_Partitions_options_menu shows the menu and acts on input events
 */

PRIVATE
uint
do_Partitions_options_menu ( uint   volume,
                             uint   partition )
{
    uint    key;
    bool    finished = FALSE;
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error;
    Partition_Information_Array     info;

    info = Get_Partitions ( Volume.Volume_Control_Data [volume].Volume_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

    Partition_options_text [0] [0] = ' ';
    Partition_options_text [1] [0] = ' ';

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Menu_options_message_line );
        key = MenuPanel ( &Partitions_options_menu );
        switch ( key ) {
            case '\r':
                switch ( Partitions_options_menu.choice ) {
                    case 0:
                        key = ChangePartitionName (
                                        Partitions_options_menu.box.row + 2,
                                        Partitions_options_menu.box.column + 2,
                                        record [ partition ].Partition_Name,
                                        record [ partition ].Partition_Handle );
                        if ( key == '\r' ) {
                            update_partitions_lines ( volume );
                        }
                        break;
                    case 1:
                        key = ChangeDiskName (
                                         Partitions_options_menu.box.row + 2,
                                         Partitions_options_menu.box.column + 2,
                                         record [ partition ].Drive_Name,
                                         record [ partition ].Drive_Handle );
                        if ( key == '\r' ) {
                            update_volume_lines ();              /* must do all */
                        }
                        break;
                    default:
                        break;
                }
                if ( key == '\r' ) {                        /* action completed */
                    finished = TRUE;
                }
                break;
            case ESCAPE:
                finished = TRUE;
                break;
            default:
                break;
        }
    }

    ErasePanel ( &Partitions_options_menu );

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

    return  key;
}


/*
 * volume routines
 */


/*
 * change_drive_letter leads the user through the steps to change the
 * drive letter for a volume
 */

PRIVATE
uint
change_drive_letter ( uint    volume )
{
    uint    menu = 0,
            drive_number = 0,
            key = ESCAPE;
    char    drive_char;
    bool    finished = FALSE,
            looping;
    CARDINAL32      error;

    while ( ! finished ) {
        switch ( menu ) {
            case 0:                                       /* Drive_change_panel */
                MESSAGE_BAR_PANEL ( Warning_message_line );
                ShowPanel ( &Drive_change_panel );
                for ( looping = TRUE; looping; ) {
                    key = GetKeystroke ();
                    switch ( key ) {
                        case '\r':
                            looping = FALSE;
                            menu = 1;
                            break;
                        case ESCAPE:
                            looping = FALSE;
                            finished = TRUE;
                            ErasePanel ( &Drive_change_panel );
                            break;
                    }
                }
                break;
            case 1:                                       /* Drive_letter_panel */
                MESSAGE_BAR_PANEL ( Drive_letter_message_line );

                /* Allow '*' drive letter only for PRM  */
                if ( Volume.Volume_Control_Data[volume].Device_Type == LVM_PRM )
                    Drive_letter_text[FLOAT_DRIVE_LETTER_INDEX][0] = ' ';
                else
                    Drive_letter_text[FLOAT_DRIVE_LETTER_INDEX][0] = NOT_SELECTABLE_CHAR;

                key = ScrollingMenuPanel ( &Drive_letter_panel );
                switch ( key ) {
                    case '\r':
                        menu = 2;
                        drive_number = Drive_letter_panel.choice;
                        break;
                    case ESCAPE:
                        menu = 0;
                        ErasePanel ( &Drive_letter_panel );
                        break;
                    default:
                        ;
                }
                break;
            case 2:                                        /* change the letter */
                finished = TRUE;
                ErasePanel ( &Drive_letter_panel );
                ErasePanel ( &Drive_change_panel );

                if (drive_number == FLOAT_DRIVE_LETTER_INDEX)
                    drive_char = '*';
                else
                    drive_char = drive_number + 'A';

                Assign_Drive_Letter (
                             Volume.Volume_Control_Data [volume].Volume_Handle,
                             drive_char, &error );
                if ( error ) {
                    DoEngineErrorPanel ( error );
                }
                update_volume_lines ();
                break;
        }
    }

    return  key;
}


/*
 * delete_volume leads the user through the delete volume steps.
 * Always erase the last volume to decrease the number of text_lines
 * by one. The lines will be recreated by update_volume_lines.
 * Erase all of the partitions lines. The physical partitions are
 * erased and recreated in update_partition_lines in the disk object.
 * Never erase the last volume line because it is needed for the
 * "no volumes" line.
 */

PRIVATE
uint
delete_volume ( uint    volume )
{
    register
    char        **text_line;
    char        *line,
                *volume_name;
    uint        menu = 0,
                key;
    bool        delete = FALSE,
                finished = FALSE;
    ADDRESS     volume_handle;
    CARDINAL32  error;

    line = Show_volume_text [ 0 ];
    volume_name = Volume_panel_text [ volume ] + VOLUME_NAME_OFFSET;

    sprintf ( line, " %-*.*s ",
              VOLUME_NAME_WIDTH, VOLUME_NAME_WIDTH, volume_name );

    while ( ! finished ) {
        switch ( menu ) {
            case 0:
                key = MenuPanel ( &Show_volume_panel );
                switch ( key ) {
                    case '\r':
                        menu = 1;
                        break;
                    case ESCAPE:
                        finished = TRUE;
                        ErasePanel ( &Show_volume_panel );
                        break;
                }
                break;
            case 1:
                Delete_volume_panel.choice = 0;                   /* not delete */
                key = MenuPanel ( &Delete_volume_panel );
                switch ( key ) {
                    case '\r':
                        delete = Delete_volume_panel.choice;
                        break;
                    case ESCAPE:
                        break;
                }
                finished = TRUE;
                ErasePanel ( &Delete_volume_panel );
                ErasePanel ( &Show_volume_panel );
                break;
            default:
                break;
        }
    }

    if ( delete ) {
        volume_handle = Volume.Volume_Control_Data [volume].Volume_Handle;

        Delete_Volume ( volume_handle, &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        }

        text_line = Partitions_panel_text [ volume ];

        for ( ;  *text_line;  ++text_line ) {
            free ( *text_line );                            /* erase it */
            *text_line = NULL;
        }

        if ( Total_volumes > 1 ) {                          /* keep the last one */
            line = Volume_panel_text [ Total_volumes - 1 ];
            if ( line ) {
                free ( line );                              /* erase it */
            }
            Volume_panel_text [ Total_volumes - 1 ] = NULL;
        }

        update_volume_lines ();
    }

    return  key;
}



PRIVATE
CARDINAL32
update_aggregate_features_lines (CARDINAL32 agg_idx, ADDRESS aggregate_handle)
{
   CARDINAL32     i, Total_aggregate_features;
   CARDINAL32     error = 0;
   Feature_ID_Data *memory;
#ifdef UNIT_TEST
   char *names[10];
#endif


#ifdef UNIT_TEST
   record_of_aggregate_features[agg_idx].Count = 0;
   switch (aggregate_handle)
   {
      case 1:
         Total_aggregate_features = 0;
         break;
      case 2:
         Total_aggregate_features = 1;
         names[0] = "Feature 1";
         break;
      case 3:
         Total_aggregate_features = 0;
         break;
      case 4:
         Total_aggregate_features = 2;
         names[0] = "Feature 1";
         names[1] = "Feature 2";
         break;
      case 5:
         Total_aggregate_features = 0;
         break;
      case 6:
         Total_aggregate_features = 0;
         break;
      case 7:
         Total_aggregate_features = 0;
         break;
      case 8:
         Total_aggregate_features = 0;
         break;
      default:
         break;
   }

   if (Total_aggregate_features == 0)
      return error;

   record_of_aggregate_features[agg_idx].Count = Total_aggregate_features;

   for (i=0; i< Total_aggregate_features; i++) {
      if (Aggregate_features_panel_text[agg_idx][i] == NULL)
         Aggregate_features_panel_text[agg_idx][i] = AllocateOrQuit(MAX_FEATURE_NAME_LENGTH+1, sizeof(char));

      strcpy(Aggregate_features_panel_text[agg_idx][i],names[i]);
   }
#else

   memory = record_of_aggregate_features[agg_idx].Feature_Data;
   record_of_aggregate_features[agg_idx].Count = 0;
   if ( memory ) {
      Free_Engine_Memory ( memory );
   }

   Refresh_LVM_Engine( &error );
   if ( error )
      DoEngineErrorPanel ( error );

   record_of_aggregate_features[agg_idx] = Get_Features( aggregate_handle, &error);
   Total_aggregate_features = record_of_aggregate_features[agg_idx].Count;

   if ( error || (Total_aggregate_features == 0) ) {
       return error;
   }

   for (i=0; i< Total_aggregate_features; i++) {
      if (Aggregate_features_panel_text[agg_idx][i] == NULL)
         Aggregate_features_panel_text[agg_idx][i] = AllocateOrQuit(MAX_FEATURE_NAME_LENGTH+1, sizeof(char));

      strcpy(Aggregate_features_panel_text[agg_idx][i],
             record_of_aggregate_features[agg_idx].Feature_Data[i].Name);
   }
#endif

   for (i=Total_aggregate_features; i<MAX_FEATURES_PER_VOLUME; i++) {
      if (Aggregate_features_panel_text[agg_idx][i])
         Aggregate_features_panel_text[agg_idx][i][0] = 0;
      else
         break;
   }

   return error;
}

/*
 * update_volume_aggregates_lines updates the
 * full Volume_aggregates_panel_text array.
 * Also updates the Aggregate_features_panel_text array
 */

PRIVATE
CARDINAL32
update_volume_aggregates_lines ( ADDRESS volume_handle )
{
   register char *line;
   CARDINAL32     i, Total_elements;
   CARDINAL32     error = 0;
   Partition_Information_Record aggregate_info;
   LVM_Handle_Array_Record record = {0};
   ADDRESS  *memory;

   /* get children */
   record = Get_Child_Handles( volume_handle, &error);

   Total_elements = record.Count;
   if ( error || (Total_elements == 0) ) {
       return error; /* error = LVM_ENGINE_NO_CHILDREN */
   }

   memory = record_of_aggregate_handles.Handles;
   if ( memory ) {
      Free_Engine_Memory ( memory );
   }

   Refresh_LVM_Engine( &error );
   if ( error )
      DoEngineErrorPanel ( error );

   record_of_aggregate_handles = record; /* new handles */



   for ( i=0; i < Total_elements; i++ ) {


      aggregate_info = Get_Partition_Information( record_of_aggregate_handles.Handles[i], &error);
      if (error)
         return error;

      if (Volume_aggregates_panel_text[i] == NULL)
         Volume_aggregates_panel_text[i] = AllocateOrQuit (PARTITION_NAME_SIZE+1,sizeof(char));

      strcpy(Volume_aggregates_panel_text[i], aggregate_info.Partition_Name);
      if (Volume_aggregates_panel_text[i][0] == 0)
         strcpy(Volume_aggregates_panel_text[i], aggregate_info.Volume_Name);

      update_aggregate_features_lines ( i, record_of_aggregate_handles.Handles[i] );


   }

   for (i=Total_elements; i<MAX_AGGREGATES; i++) {
      if (Volume_aggregates_panel_text[i]) {
         free(Volume_aggregates_panel_text[i]);
         Volume_aggregates_panel_text[i] = NULL;
      } else
         break;
   }

   return error;
}

PRIVATE
uint
do_aggregate_feature_options_menu (ADDRESS volume_handle, Feature_ID_Data *feature_data)
{
    uint    key;
    uint    i;
    bool    finished = FALSE;
    CARDINAL32 error;
    void (* _System fnDisplayStatus) (ADDRESS vol_handle, CARDINAL32 * error) = 0;
    void (* _System fnControlPanel) (ADDRESS vol_handle, CARDINAL32 * error) = 0;
    void (* _System fnHelpPanel) (CARDINAL32 help_idx, CARDINAL32 * error) = 0;



    if (feature_data->Interface_Support[VIO_Interface].Interface_Supported) {
       fnDisplayStatus = feature_data->Interface_Support[VIO_Interface].VIO_PM_Calls.Display_Status;
       fnControlPanel  = feature_data->Interface_Support[VIO_Interface].VIO_PM_Calls.Control_Panel;
       fnHelpPanel     = feature_data->Interface_Support[VIO_Interface].VIO_PM_Calls.Help_Panel;

    }


    if (fnDisplayStatus)
       Aggregate_feature_options_text [0] [0] = ' ';
    else
       Aggregate_feature_options_text [0] [0] = NOT_SELECTABLE_CHAR;

    if (fnControlPanel)
       Aggregate_feature_options_text [1] [0] = ' ';
    else
       Aggregate_feature_options_text [1] [0] = NOT_SELECTABLE_CHAR;

    if (fnHelpPanel)
       Aggregate_feature_options_text [2] [0] = ' ';
    else
       Aggregate_feature_options_text [2] [0] = NOT_SELECTABLE_CHAR;


    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Menu_options_message_line );
        key = MenuPanel ( &Aggregate_feature_options_menu );
        switch ( key ) {
            case '\r':
                switch ( Aggregate_feature_options_menu.choice ) {
                    case 0:
                       fnDisplayStatus(volume_handle, &error);
                       break;
                    case 1:
                       fnControlPanel(volume_handle, &error);
                       break;
                    case 2: /* need work */
                       fnHelpPanel(0, &error);
                       break;
                    default:
                        break;
                }
                finished = TRUE;
                break;
            case ESCAPE:
                finished = TRUE;
                break;
            default:
                break;
        }
    }

    ErasePanel ( &Aggregate_feature_options_menu );

    return  key;
}



/*
 * volume_aggregates_panel_callback updates the aggregate features panel
 * in the background
 */

PRIVATE
uint _System volume_aggregates_panel_callback ( panel_t *panel )
{
    uint    aggregate;

    aggregate = panel->choice;
    Aggregate_features_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
    if (record_of_aggregate_features[aggregate].Count) {
       Aggregate_features_panel.text_line = Aggregate_features_panel_text [ aggregate ];
    } else {
       Aggregate_features_panel.text_line = Empty_text;
    }
    ShowPanel ( &Aggregate_features_panel );
    return (0);
}

PRIVATE
uint
DoVolumeAggregatesPanel( ADDRESS volume_handle )
{
   CARDINAL32 error;
   uint    key,
           feature = 0,
           aggregate = 0;
   bool    scrolling,
           finished = FALSE;
   uint    level;
   ADDRESS *saved_handles;
   ADDRESS current_handle=0;


   saved_handles = AllocateOrQuit(100, sizeof(ADDRESS));
   saved_handles[0] = volume_handle;

#ifdef UNIT_TEST
   error = update_volume_aggregates_lines(1);
#else
   error = update_volume_aggregates_lines(volume_handle); /* get aggregates information for the volume */
#endif
   if (!error) {

      level=1;
      ShowPanel ( &Volume_aggregates_header_panel );
      ShowPanel ( &Volume_aggregates_panel );
      ShowPanel ( &Aggregate_features_header_panel );
      ShowPanel ( &Aggregate_features_panel );

         while ( ! finished ) {
            MESSAGE_BAR_PANEL ( Volume_aggregates_message_line );
            key = ScrollingMenuPanel ( &Volume_aggregates_panel );
            aggregate = Volume_aggregates_panel.choice;

            switch ( key ) {
                case '\t':
                   if (record_of_aggregate_features[aggregate].Count) {
                      /* allow TAB when there are features associated with the aggregate */
                      scrolling = TRUE;
                      while ( scrolling ) {
                         MESSAGE_BAR_PANEL ( Aggregate_features_message_line );
                         key = ScrollingMenuPanel ( &Aggregate_features_panel );
                         feature = Aggregate_features_panel.choice;

                         switch ( key ) {
                            case '\t':
                            case ESCAPE:
                                scrolling = FALSE;
                                break;
                            case '\r':
                                /* do_aggregate_feature_options_menu(aggregate, feature); */
                                do_aggregate_feature_options_menu(volume_handle,
                                            &record_of_aggregate_features[aggregate].Feature_Data[feature]);
                                break;
                            case F3:
                                scrolling = FALSE;
                                finished = TRUE;
                                break;
                            default:
                                ;
                         }
                      }
                   }
                   break;
                case '\r':
                   /* show children if any */
                   current_handle = record_of_aggregate_handles.Handles[aggregate];
                   error = update_volume_aggregates_lines(current_handle);
                   if (!error)
                   {
                      saved_handles[level] = current_handle;
                      ++level;
                      ShowPanel ( &Volume_aggregates_header_panel );
                      ShowPanel ( &Aggregate_features_header_panel );
                      Volume_aggregates_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
                      Aggregate_features_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
                      ShowPanel ( &Volume_aggregates_panel );
                      ShowPanel ( &Aggregate_features_panel );
                   }
                   break;
                case ESCAPE:
                   if (level==1)
                      finished = TRUE;
                   else {
                      /* show parent */
                      --level;
                      error = update_volume_aggregates_lines(saved_handles[level-1]);
                      if (!error) {
                         ShowPanel ( &Volume_aggregates_header_panel );
                         ShowPanel ( &Aggregate_features_header_panel );
                         Volume_aggregates_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
                         Aggregate_features_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
                         ShowPanel ( &Volume_aggregates_panel );
                         ShowPanel ( &Aggregate_features_panel );
                      } else {
                         DoEngineErrorPanel ( error );
                         finished = TRUE;
                      }

                   }
                   break;
                case F3:
                   finished = TRUE;
                   break;
                default:
                   ;
          }
       }
    }

    free(saved_handles);
    return  key;
}



/*
 * expand_volume leads the user through the expand volume steps
 */

PRIVATE
uint
expand_volume ( uint    volume )
{
    uint        key;
    uint        partition_count;
    bool        bootable = FALSE,
                primary = FALSE;
    uint        **partition_handles;
    ADDRESS     volume_handle;
    CARDINAL32  error;

    volume_handle = Volume.Volume_Control_Data [volume].Volume_Handle;

    key = GetPartitionHandles ( &partition_handles,
                                &partition_count,
                                 AN_UNLIMITED_NUMBER,
                                 bootable,
                                 primary,
                                 FALSE );
    switch ( key ) {
        case '\r':
            if ( partition_count > 0 ) {
                Expand_Volume ( volume_handle, partition_count,
                                (ADDRESS *) partition_handles,
                                &error );
                if ( error ) {
                    DoEngineErrorPanel ( error );
                }
                update_volume_line ( volume );
            }
            break;
        default:
            break;
    }

    return  key;
}


/*
 * create_new_volume leads the user through the create volume steps
 * If bootable and C:, create primary, else logical.
 */

PRIVATE
uint
create_new_volume ( void )
{
    uint    menu = 0,
            bootable = 0,
            volume_type = 0,
            drive_letter = 0,
            key = ESCAPE;
    CARDINAL32 i, selected_features_count = 0;
    bool    finished = FALSE,
            primary;
    char    *volume_name;
    char    drive_char;
    uint    partition_count,
            max_partitions=0;
    uint    **partition_handles;
    CARDINAL32      error;

    strncpy ( Volume_name_input [0], Blank_line, VOLUME_NAME_WIDTH);

    while ( ! finished ) {
        switch ( menu ) {
            case 0:                                        /* Bootable_panel */
                MESSAGE_BAR_PANEL ( Bootable_message_line );
                key = MenuPanel ( &Bootable_panel );
                switch ( key ) {
                    case '\r':
                        bootable = Bootable_panel.choice;
                        if ( bootable ) {
                            menu = 2;
                            volume_type = 0;                /* compatibility */
                        } else {
                            menu = 1;
                        }
                        break;
                    case ESCAPE:
                        finished = TRUE;
                        ErasePanel ( &Bootable_panel );
                        break;
                }
                break;
            case 1:                                        /* Volume_type_panel */
                MESSAGE_BAR_PANEL ( Volume_type_message_line );
                key = MenuPanel ( &Volume_type_panel );
                switch ( key ) {
                    case '\r':
                        menu = 2;
                        volume_type = Volume_type_panel.choice;
                        break;
                    case ESCAPE:
                        menu = 0;
                        ErasePanel ( &Volume_type_panel );
                        break;
                }
                break;
            case 2:                                       /* Drive_letter_panel */
                MESSAGE_BAR_PANEL ( Drive_letter_message_line );
                Drive_letter_text[FLOAT_DRIVE_LETTER_INDEX][0] = ' ';
                key = ScrollingMenuPanel ( &Drive_letter_panel );
                switch ( key ) {
                    case '\r':
                        menu = 3;
                        drive_letter = Drive_letter_panel.choice;
                        if (drive_letter == FLOAT_DRIVE_LETTER_INDEX)
                            drive_char = '*';
                        else
                            drive_char = drive_letter + 'A';

                        primary = ( bootable  &&  ( drive_letter == 2 ) );
                        break;
                    case ESCAPE:
                        if ( bootable ) {
                            menu = 0;
                        } else {
                            menu = 1;
                        }
                        ErasePanel ( &Drive_letter_panel );
                        break;
                    default:
                        ;
                }
                break;
            case 3:                                        /* Volume_name_panel */
                MESSAGE_BAR_PANEL ( Enter_name_message_line );
                key = DataEntryPanel ( &Volume_name_panel );
                switch ( key ) {
                    case '\r':
                        menu = 4;
                        volume_name = Volume_name_input [0];
                        while ( *volume_name == ' ' ) {
                            ++volume_name;
                        }
                        if ( *volume_name == '\0' ) {                /* no name */
                            menu = 3;
                        }
                        break;
                    case ESCAPE:
                        menu = 2;
                        ErasePanel ( &Volume_name_panel );
                        break;
                }
                break;

            case 4: /* select features */
               if ( volume_type == LVM_VOLUME)
               {
                  get_features();
                  MESSAGE_BAR_PANEL ( Volume_select_feature_message_line );
                  key = UserDefinedMenuPanel(&Volume_select_feature_menu,
                                   Total_features,
                                   Volume_feature_state,
                                   chars_less_than_greater_than[0], /* use '<' and '>' to mark selected text */
                                   chars_less_than_greater_than[1]);
                  switch ( key ) {
                     case '\r':
                        /* The user just selected some features, find out how many & what they are */
                        selected_features_count = verify_selected_features();
                        menu = 5;

                        break;
                     case ESCAPE:
                        menu = 3;
                        ErasePanel ( &Volume_select_feature_menu );
                        break;
                  }
               } else {
                  menu = 5;
               }
               break;

            case 5:
                if ( volume_type == COMPATIBILITY_VOLUME ) {
                    max_partitions = 1;
                } else {
                    for (i=0; i<selected_features_count; i++) {
                       if (spec_selected_features[i].Actual_Class == Aggregate_Class) {
                          max_partitions = AN_UNLIMITED_NUMBER;
                          break;
                       }
                    }

                    if (max_partitions != AN_UNLIMITED_NUMBER)
                       max_partitions = 1;
                }
                key = GetPartitionHandles ( &partition_handles,
                                            &partition_count,
                                             max_partitions,
                                             bootable,
                                             primary,
                                             drive_char == '*' );
                switch ( key ) {
                    case '\r':
                        finished = TRUE;
                        ErasePanel ( &Volume_select_feature_menu);
                        ErasePanel ( &Volume_name_panel );
                        ErasePanel ( &Drive_letter_panel );
                        if ( ! bootable ) {
                            ErasePanel ( &Volume_type_panel );
                        }
                        ErasePanel ( &Bootable_panel );
                        if ( partition_count > 0 ) {
                            Create_Volume2 ( volume_name, volume_type, bootable,
                                             drive_char,
                                             selected_features_count,
                                             spec_selected_features,
                                             partition_count,
                                             (ADDRESS *) partition_handles,
                                             &error );
                            if ( error ) {
                                DoEngineErrorPanel ( error );
                            }
                            update_volume_lines ();
                        }
                        break;
                    case ESCAPE:
                        menu = 3;
                        break;
                }
                break;
        }
    }

    return  key;
}


/*
 * set_valid_volume_options sets the selectable options in the
 * Volume_options_text2 array.
 * Choice numbers need to match Volume_options_text2.
 */

PRIVATE
void
set_valid_volume_options ( ADDRESS  volume_handle )
{
    register
    char    **text_line = Volume_options_text2;
    register
    CARDINAL32      bit_map = 0;                                /* no volumes */
    CARDINAL32      error = 0;
    BOOLEAN         bootman_installed;
    BOOLEAN         bootman_active;
    Feature_Information_Array this_volume_feature_info;

    if ( volume_handle ) {                                      /* volumes > 0 */
        bit_map = Get_Valid_Options ( volume_handle, &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        }
    } else {
        bootman_installed = Boot_Manager_Is_Installed ( &bootman_active, &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        } else {
            if ( bootman_installed ) {
                bit_map |= SET_BOOT_MANAGER_DEFAULTS;
                bit_map |= REMOVE_BOOT_MANAGER;
            } else {
                bit_map |= INSTALL_BOOT_MANAGER;
            }
        }
    }


    if (volume_handle) {
       this_volume_feature_info = Get_Features( volume_handle, &error);
       if ((error == LVM_ENGINE_NO_ERROR) && (this_volume_feature_info.Count != 0)) {
          Free_Engine_Memory(this_volume_feature_info.Feature_Data);
          text_line [ 0 ] [ 0 ] = ' ';
       } else {
          text_line [ 0 ] [ 0 ] = NOT_SELECTABLE_CHAR;
       }
    } else {
          text_line [ 0 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }


    if ( bit_map & EXPAND_VOLUME ) {
        text_line [ 1 ] [ 0 ] = ' ';
    } else {
        text_line [ 1 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & DELETE_VOLUME ) {
        text_line [ 2 ] [ 0 ] = ' ';
    } else {
        text_line [ 2 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & HIDE_VOLUME ) {
        text_line [ 3 ] [ 0 ] = ' ';
    } else {
        text_line [ 3 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }

    if ( bit_map & CAN_SET_NAME ) {
        text_line [ 4 ] [ 0 ] = ' ';                  /* Change the volume name = 3 */
    } else {
        text_line [ 4 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }

    if ( bit_map & ASSIGN_DRIVE_LETTER ) {
        text_line [ 5 ] [ 0 ] = ' ';
    } else {
        text_line [ 5 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
                                                            /* SEPARATOR_LINE, */
    if ( bit_map & SET_VOLUME_INSTALLABLE ) {
        text_line [ 7 ] [ 0 ] = ' ';
    } else {
        text_line [ 7 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & SET_STARTABLE ) {
        text_line [ 8 ] [ 0 ] = ' ';
    } else {
        text_line [ 8 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & ADD_TO_BOOT_MANAGER_MENU ) {
        text_line [ 9 ] [ 0 ] = ' ';
    } else {
        text_line [ 9 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & REMOVE_FROM_BOOT_MANAGER_MENU ) {
        text_line [ 10 ] [ 0 ] = ' ';
    } else {
        text_line [ 10 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
                                                            /* SEPARATOR_LINE, */

    if ( Total_volumes < MAX_VOLUMES ) {            /* Create a new volume = 10 */
        text_line [ 12 ] [ 0 ] = ' ';
    } else {
        text_line [ 12 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
                                                            /* SEPARATOR_LINE, */
    if ( bit_map & INSTALL_BOOT_MANAGER ) {
        text_line [ 14 ] [ 0 ] = ' ';
    } else {
        text_line [ 14 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & REMOVE_BOOT_MANAGER ) {
        text_line [ 15 ] [ 0 ] = ' ';
    } else {
        text_line [ 15 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & SET_BOOT_MANAGER_DEFAULTS ) {
        text_line [ 16 ] [ 0 ] = ' ';
    } else {
        text_line [ 16 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if (text_line[17]) {     /* this option may not be present in all NLS msg files yet. */
        if ( bit_map & SET_BOOT_MANAGER_STARTABLE ) {
            text_line [ 17 ] [ 0 ] = ' ';
        } else {
            text_line [ 17 ] [ 0 ] = NOT_SELECTABLE_CHAR;
        }
    }

}


/*
 * do_Volume_options_menu shows the menu and acts on input events.
 * Choice numbers need to match Volume_options_text2.
 */

PRIVATE
uint
do_Volume_options_menu ( uint   volume )
{
    uint    key;
    bool    finished = FALSE,
            on_bm_menu;
    char    *volume_name = Blank_line;
    Volume_Information_Record   info;
    CARDINAL32                  error;
    ADDRESS                     volume_handle = NULL;
    ADDRESS                     bootmgr_handle = NULL;

    if ( Volume.Count > 0 ) {
        volume_handle = Volume.Volume_Control_Data [volume].Volume_Handle;
        info = Get_Volume_Information ( volume_handle, &error );
        if ( error ) {
            Quit ( Cannot_get_volume_data );
        }
        volume_name = info.Volume_Name;
    }
    set_valid_volume_options ( volume_handle );

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Menu_options_message_line );
        key = MenuPanel ( &Volume_options_menu );
        switch ( key ) {
            case '\r':
                switch ( Volume_options_menu.choice ) {
                    case 0:
                        key = DoVolumeAggregatesPanel ( volume_handle );   /* Show volume tree */
                        break;
                    case 1:                                     /* expand */
                        key = expand_volume ( volume );
                        break;
                    case 2:                                     /* delete */
                        key = delete_volume ( volume );
                        break;
                    case 3:                                     /* hide */
                        Hide_Volume ( volume_handle, &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        update_volume_lines ();
                        break;
                    case 4:                                    /* change name */
                        key = ChangeVolumeName (
                                          Volume_name_panel.box.row,
                                          Volume_name_panel.box.column,
                                          volume_name,
                                          volume_handle );
                        if ( key == '\r' ) {
                            update_volume_line ( volume );
                        }
                        break;
                    case 5:                                    /* change letter */
                        key = change_drive_letter ( volume );
                        break;
                    case 6:                                     /* separator */
                        break;
                    case 7:                                     /* installable */
                        Set_Installable ( volume_handle, &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        update_volume_lines ();
                        break;
                    case 8:                                     /* startable */
                        Set_Startable ( volume_handle, &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        update_volume_lines ();
                        break;
                    case 9:                                     /* add to BM */
                        Add_To_Boot_Manager ( volume_handle, &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        update_volume_line ( volume );
                        break;
                    case 10:                                   /* remove from BM */
                        Remove_From_Boot_Manager ( volume_handle, &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        update_volume_line ( volume );
                        break;
                    case 11:                                    /* separator */
                        break;
                    case 12:                                   /* create volume */
                        key = create_new_volume ();
                        break;
                    case 13:                                    /* separator */
                        break;
                    case 14:                                    /* install BM */
                        Install_Boot_Manager ( 1, &error );
                        if ( error ) {
                            Install_Boot_Manager ( 2, &error );
                            if ( error ) {
                                DoEngineErrorPanel ( error );
                            }
                        }
                        update_volume_lines ();
                        break;
                    case 15:                                    /* remove BM */
                        Remove_Boot_Manager ( &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        update_volume_lines ();
                        break;
                    case 16:                                    /* BM values */
                        /* on_bm_menu = ( Volume_options_text2 [ 9 ] [ 0 ] == ' ' );  */
                        on_bm_menu = ( Volume_options_text2 [ 10 ] [ 0 ] == ' ' );
                        if ( on_bm_menu ) {
                            SetBootmanOptions ( volume_handle );
                        } else {
                            SetBootmanOptions ( NULL );
                        }
                        update_volume_lines ();
                        break;
                    case 17:                                    /* BM startable */
                        bootmgr_handle = Get_Boot_Manager_Handle ( &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }
                        else if (bootmgr_handle) {
                            Set_Startable ( bootmgr_handle, &error );
                            if ( error ) {
                                DoEngineErrorPanel ( error );
                            }
                        }
                        update_volume_lines ();
                        break;
                    default:
                        break;
                }
                if ( key == '\r' ) {                        /* action completed */
                    finished = TRUE;
                }
                break;
            case ESCAPE:
                finished = TRUE;
                break;
            default:
                break;
        }
    }

    ErasePanel ( &Volume_options_menu );

    return  key;
}


/*
 * volume_panel_callback updates the partitions panel in the background
 */

PRIVATE
uint _System volume_panel_callback ( panel_t *panel )
{
    uint    volume;

    volume = panel->choice;
    Partitions_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
    Partitions_panel.text_line = Partitions_panel_text [ volume ];
    ShowPanel ( &Partitions_panel );
    return(0);

}


/*
 * ChangeVolumeName gets a volume name from the user using InputName, and
 * then changes it by calling Set_Name.
 * The main purpose is to keep the entry panel private, and to handle
 * repositioning the panel.
 * Must erase the panel here because it is repositioned.
 * It returns the key from InputName.
 */

PUBLIC
uint
ChangeVolumeName ( uint   row,
                   uint   column,
                   char   *old_name,
                   void   *handle )
{
    uint        key,
                save_row = Volume_name_panel.box.row,
                save_column = Volume_name_panel.box.column;
    CARDINAL32  error;

    Volume_name_panel.box.row = row;
    Volume_name_panel.box.column = column;

    key = InputName ( &Volume_name_panel,
                      Volume_name_input [0],
                      VOLUME_NAME_WIDTH,
                      old_name );

    if ( key == '\r' ) {
        Set_Name ( handle, Volume_name_input [0], &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        }
    }

    ErasePanel ( &Volume_name_panel );

    Volume_name_panel.box.row = save_row;
    Volume_name_panel.box.column = save_column;

    return  key;
}


/*
 * DoVolumePanel shows the panel and acts on input events
 */

PUBLIC
uint
DoVolumePanel ( void )
{
    uint    key,
            partition = 0,
            volume = 0;
    bool    scrolling,
            finished = FALSE;

    update_volume_lines ();                            /* pick up physical info */

    ShowPanel ( &Volume_header_panel );
    ShowPanel ( &Volume_panel );
    ShowPanel ( &Partitions_header_panel );
    ShowPanel ( &Partitions_panel );

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Volume_message_line );
        key = ScrollingMenuPanel ( &Volume_panel );
        volume = Volume_panel.choice;

        switch ( key ) {
            case '\t':
                scrolling = TRUE;
                while ( scrolling ) {
                    MESSAGE_BAR_PANEL ( Volume_message_line );
                    key = ScrollingMenuPanel ( &Partitions_panel );
                    partition = Partitions_panel.choice;

                    switch ( key ) {
                        case '\t':
                        case ESCAPE:
                            scrolling = FALSE;
                            break;
                        case '\r':
                            key = do_Partitions_options_menu ( volume,
                                                               partition );
                            break;
                        case F3:
                        case F5:
                            scrolling = FALSE;
                            finished = TRUE;
                            break;
                        default:
                            ;
                    }
                }
                break;
            case '\r':
                key = do_Volume_options_menu ( volume );
                ShowPanel ( &Volume_header_panel );
                ShowPanel ( &Partitions_header_panel );
                Volume_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
                break;
            case F3:
            case F5:
                finished = TRUE;
                break;
            default:
                ;
        }
    }

    return  key;
}


/*
 * ConstructVolumePanels sets the characteristics of the panels
 */

PUBLIC
void
ConstructVolumePanels ( uint  top_row,
                        uint  bottom_row )
{
    char    *line;
    uint    usable_rows;


                                                  /* initialize for no volumes */

    line = AllocateOrQuit ( LINE_LENGTH, sizeof (char) );
    strncpy ( line, No_volumes_defined, SCREEN_WIDTH );
    Volume_panel_text [ 0 ] = line;

    Partitions_panel_text [ 0 ] = ReallocStringArray ( Partitions_panel_text [ 0 ] , 0 );

    update_volume_lines ();                       /* initialize the text arrays */



    /* Volume_header_panel */

    Volume_header_panel.control = PANEL_CONTROL;
    Volume_header_panel.box.outline = Single_line_outline;
    Volume_header_panel.box.row = top_row;
    Volume_header_panel.box.column = 0;
    Volume_header_panel.box.row_length = SCREEN_WIDTH;
    Volume_header_panel.box.number_of_rows = 3;
    Volume_header_panel.box.attribute = Panel_attribute;
    Volume_header_panel.box.border_attribute = Panel_border_attribute;
    Volume_header_panel.text_line = Volume_header_text;

    /* Volume_panel */

    Volume_panel.control = PANEL_CONTROL;
    Volume_panel.box.outline = Tee_top_outline;
    Volume_panel.box.scroll_bar = Line_scroll_bar;
    Volume_panel.box.row = top_row + Volume_header_panel.box.number_of_rows - 1;
    Volume_panel.box.column = 0;
    Volume_panel.box.row_length = SCREEN_WIDTH;
    usable_rows = bottom_row - top_row - 4;
    Volume_panel.box.number_of_rows = ( usable_rows * 2 ) / 3;
    Volume_panel.box.max_number_of_rows = 26;               /* 24 volumes + 2 */
    Volume_panel.box.attribute = Panel_attribute;
    Volume_panel.box.border_attribute = Panel_border_attribute;
    Volume_panel.text_line = Volume_panel_text;
    Volume_panel.callback = volume_panel_callback;
    Volume_panel.help.routine = (void *)DoHelpPanel;
    Volume_panel.help.default_message = VOLUME_HELP_MESSAGE;

    /* Volume_options_menu */

    Volume_options_menu.control = MENU_CONTROL;
    Volume_options_menu.box.outline = Single_line_outline;
    Volume_options_menu.box.row = Volume_header_panel.box.row + 1;
    Volume_options_menu.box.column = Volume_panel.box.column + 1;
    Volume_options_menu.box.attribute = Menu_attribute;
    Volume_options_menu.box.border_attribute = Menu_border_attribute;
    Volume_options_menu.text_line = Volume_options_text2;
    Volume_options_menu.help.routine = (void *)DoHelpPanel;
    Volume_options_menu.help.array = Volume_options_help_array;
    Volume_options_menu.help.array_size = Volume_options_help_array_size;

    /* Show_volume_panel */

    Show_volume_panel.control = CHOICE_CONTROL;
    Show_volume_panel.box.outline = Single_line_outline;
    Show_volume_panel.box.row = Volume_options_menu.box.row + 3;
    Show_volume_panel.box.column = Volume_options_menu.box.column + 4;
    Show_volume_panel.box.attribute = Choice_attribute;
    Show_volume_panel.box.border_attribute = Choice_border_attribute;
    Show_volume_panel.text_line = Show_volume_text;

    /* Delete_volume_panel */

    Delete_volume_panel.control = CHOICE_CONTROL;
    Delete_volume_panel.box.outline = Single_line_outline;
    Delete_volume_panel.box.row = Show_volume_panel.box.row + 2;
    Delete_volume_panel.box.column = Show_volume_panel.box.column + 2;
    Delete_volume_panel.box.attribute = Choice_attribute;
    Delete_volume_panel.box.border_attribute = Choice_border_attribute;
    Delete_volume_panel.text_line = Delete_volume_text;

    /* Bootable_panel */

    Bootable_panel.control = CHOICE_CONTROL;
    Bootable_panel.box.outline = Single_line_outline;
    Bootable_panel.box.row = Volume_options_menu.box.row + 6;
    Bootable_panel.box.column = Volume_options_menu.box.column + 2;
    Bootable_panel.box.attribute = Choice_attribute;
    Bootable_panel.box.border_attribute = Choice_border_attribute;
    Bootable_panel.text_line = Bootable_text;
    Bootable_panel.help.routine = (void *)DoHelpPanel;
    Bootable_panel.help.array = Bootable_help_array;
    Bootable_panel.help.array_size = Bootable_help_array_size;

    /* Volume_type_panel */

    Volume_type_panel.control = CHOICE_CONTROL;
    Volume_type_panel.box.outline = Single_line_outline;
    Volume_type_panel.box.row = Bootable_panel.box.row + 2;
    Volume_type_panel.box.column = Bootable_panel.box.column + 2;
    Volume_type_panel.box.attribute = Choice_attribute;
    Volume_type_panel.box.border_attribute = Choice_border_attribute;
    Volume_type_panel.text_line = Volume_type_text;

    /* Drive_letter_panel */

    Drive_letter_panel.control = CHOICE_CONTROL;
    Drive_letter_panel.box.outline = Single_line_outline;
    Drive_letter_panel.box.scroll_bar = Default_scroll_bar;
    Drive_letter_panel.box.row = Volume_type_panel.box.row + 2;
    Drive_letter_panel.box.column = Volume_type_panel.box.column + 2;
    Drive_letter_panel.box.number_of_rows = bottom_row -
                                            Drive_letter_panel.box.row;
    Drive_letter_panel.box.max_number_of_rows = 26;         /* 24 letters + 2 */
    Drive_letter_panel.box.attribute = Choice_attribute;
    Drive_letter_panel.box.border_attribute = Choice_border_attribute;
    Drive_letter_panel.text_line = Drive_letter_text;
    Drive_letter_panel.help.routine = (void *)DoHelpPanel;
    Drive_letter_panel.help.default_message = DRIVE_LETTER_HELP_MESSAGE;

    /* Volume_name_panel */

    Volume_name_panel.control = ENTRY_CONTROL;
    Volume_name_panel.box.outline = Single_line_outline;
    Volume_name_panel.box.row = Drive_letter_panel.box.row + 2;
    Volume_name_panel.box.column = Drive_letter_panel.box.column + 2;
    Volume_name_panel.box.attribute = Entry_attribute;
    Volume_name_panel.box.border_attribute = Entry_border_attribute;
    Volume_name_panel.text_line = Volume_name_text;
    Volume_name_panel.input_line = Volume_name_input;

    /* Drive_change_panel */

    Drive_change_panel.control = WARNING_CONTROL;
    Drive_change_panel.box.outline = Blank_outline;
    Drive_change_panel.box.scroll_bar = Default_scroll_bar;
    Drive_change_panel.box.row = Volume_options_menu.box.row + 2;
    Drive_change_panel.box.column = Volume_options_menu.box.column + 2;
    Drive_change_panel.box.number_of_rows = 0;
    Drive_change_panel.box.attribute = Warning_attribute;
    Drive_change_panel.box.border_attribute = Warning_border_attribute;
    Drive_change_panel.text_line = Drive_change_text;

    /* Partitions_header_panel */

    Partitions_header_panel.control = PANEL_CONTROL;
    Partitions_header_panel.box.outline = Single_line_outline;
    Partitions_header_panel.box.row = Volume_panel.box.row +
                                      Volume_panel.box.number_of_rows;
    Partitions_header_panel.box.column = 0;
    Partitions_header_panel.box.row_length = SCREEN_WIDTH;
    Partitions_header_panel.box.number_of_rows = 3;
    Partitions_header_panel.box.attribute = Panel_attribute;
    Partitions_header_panel.box.border_attribute =
                                                Panel_border_attribute;
    Partitions_header_panel.text_line = Partitions_header_text;

    /* Partitions_panel */

    Partitions_panel.control = PANEL_CONTROL;
    Partitions_panel.box.outline = Tee_top_outline;
    Partitions_panel.box.scroll_bar = Line_scroll_bar;
    Partitions_panel.box.row = Partitions_header_panel.box.row +
                               Partitions_header_panel.box.number_of_rows - 1;
    Partitions_panel.box.column = 0;
    Partitions_panel.box.row_length = SCREEN_WIDTH;
    Partitions_panel.box.number_of_rows = bottom_row -
                                          Partitions_panel.box.row + 1;
    Partitions_panel.box.attribute = Panel_attribute;
    Partitions_panel.box.border_attribute = Panel_border_attribute;
    Partitions_panel.text_line = Empty_text;
    Partitions_panel.help.routine = (void *)DoHelpPanel;
    Partitions_panel.help.default_message = VOLUME_HELP_MESSAGE;

    /* Partitions_options_menu */

    Partitions_options_menu.control = MENU_CONTROL;
    Partitions_options_menu.box.outline = Single_line_outline;
    Partitions_options_menu.box.row = Partitions_header_panel.box.row + 1;
    Partitions_options_menu.box.column = Partitions_panel.box.column + 1;
    Partitions_options_menu.box.attribute = Menu_attribute;
    Partitions_options_menu.box.border_attribute = Menu_border_attribute;
    Partitions_options_menu.text_line = Partitions_options_text;

    /* Volume_select_feature_menu */
    Volume_select_feature_menu.control = MENU_CONTROL;
    Volume_select_feature_menu.box.outline = Single_line_outline;
    /*Volume_select_feature_menu.box.scroll_bar = Line_scroll_bar; */
    Volume_select_feature_menu.box.row = Volume_type_panel.box.row+2;
    Volume_select_feature_menu.box.column = 0;
    Volume_select_feature_menu.box.attribute = Menu_attribute;
    Volume_select_feature_menu.box.border_attribute = Menu_border_attribute;
    Volume_select_feature_menu.title_line = Volume_select_feature_title;
    Volume_select_feature_menu.text_line = Volume_feature_text;
    Volume_select_feature_menu.help.routine = (void *)DoHelpPanel;
    Volume_select_feature_menu.help.default_message = SELECT_FEATURES_HELP_MESSAGE;
    Volume_select_feature_menu.callback = Volume_select_feature_callback;

    Volume_order_feature_menu.control = MENU_CONTROL;
    Volume_order_feature_menu.box.outline = Single_line_outline;
    /*Volume_select_feature_menu.box.scroll_bar = Line_scroll_bar; */
    Volume_order_feature_menu.box.row = Volume_select_feature_menu.box.row;
    Volume_order_feature_menu.box.column = 2;
    Volume_order_feature_menu.box.attribute = Menu_attribute;
    Volume_order_feature_menu.box.border_attribute = Menu_border_attribute;
    Volume_order_feature_menu.title_line = Volume_order_feature_title;
    Volume_order_feature_menu.text_line = 0;
    Volume_order_feature_menu.help.routine = (void *)DoHelpPanel;
    Volume_order_feature_menu.help.default_message = ORDER_FEATURES_HELP_MESSAGE;
    Volume_order_feature_menu.callback = Volume_order_feature_callback;


    /* Volume_aggregates_header_panel */

    Volume_aggregates_header_panel.control = PANEL_CONTROL;
    Volume_aggregates_header_panel.box.outline = Single_line_outline;
    Volume_aggregates_header_panel.box.row = top_row;
    Volume_aggregates_header_panel.box.column = 0;
    Volume_aggregates_header_panel.box.row_length = SCREEN_WIDTH;
    Volume_aggregates_header_panel.box.number_of_rows = 3;
    Volume_aggregates_header_panel.box.attribute = Panel_attribute;
    Volume_aggregates_header_panel.box.border_attribute = Panel_border_attribute;
    Volume_aggregates_header_panel.text_line = Volume_aggregates_header_text;

    /* Volume_aggregates_panel */

    Volume_aggregates_panel.control = PANEL_CONTROL;
    Volume_aggregates_panel.box.outline = Tee_top_outline;
    Volume_aggregates_panel.box.scroll_bar = Line_scroll_bar;
    Volume_aggregates_panel.box.row = top_row + Volume_aggregates_header_panel.box.number_of_rows - 1;
    Volume_aggregates_panel.box.column = 0;
    Volume_aggregates_panel.box.row_length = SCREEN_WIDTH;
    Volume_aggregates_panel.box.number_of_rows = ( usable_rows * 2 ) / 3;
    Volume_aggregates_panel.box.max_number_of_rows = 26;               /* 24 volumes + 2 */
    Volume_aggregates_panel.box.attribute = Panel_attribute;
    Volume_aggregates_panel.box.border_attribute = Panel_border_attribute;
    Volume_aggregates_panel.text_line = Volume_aggregates_panel_text;
    Volume_aggregates_panel.callback = volume_aggregates_panel_callback;
    Volume_aggregates_panel.help.routine = (void *)DoHelpPanel;
    Volume_aggregates_panel.help.default_message = NAVIGATE_FEATURES_HELP_MESSAGE;


    /* Aggregate_features_header_panel */

    Aggregate_features_header_panel.control = PANEL_CONTROL;
    Aggregate_features_header_panel.box.outline = Single_line_outline;
    Aggregate_features_header_panel.box.row = Volume_aggregates_panel.box.row +
                                      Volume_aggregates_panel.box.number_of_rows;
    Aggregate_features_header_panel.box.column = 0;
    Aggregate_features_header_panel.box.row_length = SCREEN_WIDTH;
    Aggregate_features_header_panel.box.number_of_rows = 3;
    Aggregate_features_header_panel.box.attribute = Panel_attribute;
    Aggregate_features_header_panel.box.border_attribute =
                                                Panel_border_attribute;
    Aggregate_features_header_panel.text_line = Aggregate_features_header_text;

    /* Aggregate_features_panel */

    Aggregate_features_panel.control = PANEL_CONTROL;
    Aggregate_features_panel.box.outline = Tee_top_outline;
    Aggregate_features_panel.box.scroll_bar = Line_scroll_bar;
    Aggregate_features_panel.box.row = Aggregate_features_header_panel.box.row +
                                       Aggregate_features_header_panel.box.number_of_rows - 1;
    Aggregate_features_panel.box.column = 0;
    Aggregate_features_panel.box.row_length = SCREEN_WIDTH;
    Aggregate_features_panel.box.number_of_rows = bottom_row -
                                                  Aggregate_features_panel.box.row + 1;
    Aggregate_features_panel.box.attribute = Panel_attribute;
    Aggregate_features_panel.box.border_attribute = Panel_border_attribute;
    Aggregate_features_panel.text_line = Empty_text;
    Aggregate_features_panel.help.routine = (void *)DoHelpPanel;
    Aggregate_features_panel.help.default_message = NAVIGATE_FEATURES_HELP_MESSAGE;

    /* Aggregate_feature_options_menu */

    Aggregate_feature_options_menu.control = MENU_CONTROL;
    Aggregate_feature_options_menu.box.outline = Single_line_outline;
    Aggregate_feature_options_menu.box.row = Aggregate_features_header_panel.box.row + 1;
    Aggregate_feature_options_menu.box.column = Aggregate_features_panel.box.column + 1;
    Aggregate_feature_options_menu.box.attribute = Menu_attribute;
    Aggregate_feature_options_menu.box.border_attribute = Menu_border_attribute;
    Aggregate_feature_options_menu.text_line = Aggregate_feature_options_text;
    Aggregate_feature_options_menu.help.routine = (void *)DoHelpPanel;
    Aggregate_feature_options_menu.help.default_message = NAVIGATE_FEATURES_HELP_MESSAGE;



    ConstructBootmanPanels ( Bootable_panel.box.row,
                             Bootable_panel.box.column );

}

