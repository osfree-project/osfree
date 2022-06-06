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
 * Module: disk.c
 */

/*
 * Change History:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
#include "Logging.h"

#define API_ENTRY_BORDER   "***?!?** ENTRY *****\n"
#define API_EXIT_BORDER    "***?!?** return ******\n"


/*
 * LVM Engine structures
 *
 */

PRIVATE
Drive_Control_Array         Disk;


PUBLIC
panel_t     Program_title_panel;

/*
 * Disk panel declarations
 *
 */

PRIVATE
panel_t     Disk_panel,
            Disk_header_panel;

PRIVATE
panel_t     Disk_options_menu,                          /* menu panels */
            Choose_partition_menu,
            Free_space_menu,
            Disk_name_panel,
            Disk_warning_panel;



/*
 * Partition_panel declarations
 *
 */

PRIVATE
panel_t     Partition_panel,
            Partition_header_panel,
            Choose_a_disk_panel;

PRIVATE
panel_t     Partition_options_menu,                          /* menu panels */
            Show_partition_panel,
            Delete_partition_panel,
            Partition_type_panel,
            Partition_name_panel,
            Partition_position_panel,
            Partition_size_panel;


/*
 * display text_line control and arrays
 *
 */

PRIVATE
char        **Disk_panel_text;              /* disk display lines */

PRIVATE
char       ***Partition_panel_text;         /* partition display lines arrays */

PRIVATE
uint        Total_disks = 0;                /* currently defined */



/*
 * Partition handles declarations for creating and extending a volume
 *
 */

#define MAX_PARTITION_HANDLES   132

PRIVATE
uint *  Partition_handles [ MAX_PARTITION_HANDLES ],
        Partition_count;

PRIVATE
bool    Create_bootable_partition,
        Create_primary_partition;



/*
 * Macros
 *
 */

#define IS_FREE_SPACE(d,p) ( ! strncmp ( Partition_panel_text[d][p] + \
PARTITION_STATUS_OFFSET, Blank_line, PARTITION_STATUS_WIDTH ) )

#define IS_AVAILABLE_PARTITION(d,p) ( ! strncmp ( Partition_panel_text [d][p] +\
PARTITION_STATUS_OFFSET, Partition_status_strings [PARTITION_IS_AVAILABLE], \
min( strlen ( Partition_status_strings [PARTITION_IS_AVAILABLE] ), PARTITION_STATUS_WIDTH) ) )

#define SET_NOT_AVAILABLE(d,p) \
    *(Partition_panel_text [d][p] + PARTITION_STATUS_OFFSET) = '\0'



/*
 * display text_line routines
 *
 */



/*
 * allocate_text_arrays allocates Disk_panel_text and Partition_panel_text
 * after first initializing Total_disks.
 *
 */

PRIVATE
void
allocate_text_arrays ( void )
{
    CARDINAL32      error;

    Disk = Get_Drive_Control_Data ( &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }

    Total_disks = Disk.Count;
    if ( Total_disks == 0 ) {
        Quit ( No_disks_found );
    }

    Disk_panel_text = AllocateOrQuit ( Total_disks + 1, sizeof (char *) );
    Partition_panel_text = AllocateOrQuit ( Total_disks + 1, sizeof (char *) );

}


/*
 * make_partition_line sprintfs a text line for the Partition_panel_text
 * array.
 * If the current line is NULL, allocate a new one.
 *
 */

PRIVATE
char *
make_partition_line ( char    *line,
                      char    *partition_name,
                      uint    size,
                      char    *partition_type,
                      char    *partition_status,
                      char    *volume_name )
{
    if ( line == NULL ) {
        line = AllocateOrQuit ( LINE_LENGTH, sizeof (char) );
    }

    sprintf ( line, "%-*.*s %*u %-*.*s %-*.*s %-*.*s",
              PARTITION_NAME_WIDTH, PARTITION_NAME_WIDTH, partition_name,
              PARTITION_SIZE_WIDTH, size,
              PARTITION_TYPE_WIDTH, PARTITION_TYPE_WIDTH, partition_type,
              PARTITION_STATUS_WIDTH, PARTITION_STATUS_WIDTH, partition_status,
              VOLUME_NAME_WIDTH, VOLUME_NAME_WIDTH, volume_name );

    return  line;
}


/*
 * update_partition_lines updates the Partition_panel_text array entry.
 * All lines must be deleted because the volume may have been deleted.
 * partition name is one of:
 *      1. [ free space n ]
 *      2. name if present in info struct
 *      3. [ n ]
 *
 */

PRIVATE
void
update_partition_lines ( uint    disk )
{
    register
    char    **line;
    uint    index,
            free_space_number = 1;
    uint    size;
    char    *partition_name,
            *partition_type,
            *partition_status,
            *volume_name,
            buffer [PARTITION_NAME_WIDTH + 4];
    register
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error;
    Partition_Information_Array     info;

    info = Get_Partitions ( Disk.Drive_Control_Data [disk].Drive_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

    line = Partition_panel_text [ disk ];
    if(line)
      for (;  *line;  ++line ) {
          free ( *line );     /* erase it */
          *line = NULL;
       }

    line = Partition_panel_text [ disk ];
    line = ReallocStringArray ( line, info.Count );
    Partition_panel_text [ disk ] = line;


    line = Partition_panel_text [ disk ];

    for ( index = 0;  index < info.Count;  ++index, ++line, ++record ) {
        size = record->Usable_Partition_Size / SECTORS_PER_MEG;
        if ( record->Partition_Type == 0 ) {
            sprintf ( buffer, "[%-.*s%u ]", PARTITION_NAME_WIDTH-6, Free_space, free_space_number );
            ++free_space_number;
            partition_name = buffer;
            partition_type = Blank_line;
            partition_status = Blank_line;
            volume_name = Blank_line;
        } else {
            partition_name = record->Partition_Name;
            if ( *partition_name == '\0'  ||  *partition_name == ' ' ) {
                sprintf ( partition_name, "[ %u ]", index + 1 );
            }
            if ( record->Primary_Partition ) {
                partition_type = Partition_type_strings [ 1 ];
            } else {
                partition_type = Partition_type_strings [ 0 ];
            }
            partition_status =
                           Partition_status_strings [record->Partition_Status];
            volume_name = record->Volume_Name;
        }
        *line = make_partition_line ( *line, partition_name, size,
                                      partition_type, partition_status,
                                      volume_name );
    }

    /* Set Partition panel to use new partition lines */
    Partition_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
    Partition_panel.text_line = Partition_panel_text [ disk ];

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

}


/*
 * make_disk_line allocates and sprintfs a text line for the
 * Disk_panel_text array
 *
 */

PRIVATE
char *
make_disk_line ( uint    disk_number,
                 char    *disk_name,
                 uint    size,
                 uint    total_free_space,
                 uint    largest_free_space )
{
    char    *line;

    line = AllocateOrQuit ( LINE_LENGTH, sizeof (char) );

    sprintf ( line, "%-*d %-*.*s %*u %*u %*u",
              DISK_NUMBER_WIDTH, disk_number,
              DISK_NAME_WIDTH, DISK_NAME_WIDTH, disk_name,
              DISK_SIZE_WIDTH, size,
              DISK_FREE_WIDTH, total_free_space,
              DISK_LARGEST_WIDTH, largest_free_space );

    return  line;
}


/*
 * update_disk_line updates one line in the Disk_panel_text array.
 * Does not update the partition lines for the disk because this
 * undoes the effect of SET_NOT_AVAILABLE.
 *
 */

PRIVATE
void
update_disk_line ( uint    disk )
{
    uint    number;
    char    *disk_name;
    uint    size,
            total,
            largest;
    Drive_Information_Record    info;
    CARDINAL32                  error;

    info = Get_Drive_Status ( Disk.Drive_Control_Data [disk].Drive_Handle,
                              &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    number = Disk.Drive_Control_Data [disk].Drive_Number;
    size = Disk.Drive_Control_Data [disk].Drive_Size / SECTORS_PER_MEG;
    total = info.Total_Available_Sectors / SECTORS_PER_MEG;
    largest = info.Largest_Free_Block_Of_Sectors / SECTORS_PER_MEG;
    disk_name = info.Drive_Name;

    Disk_panel_text [ disk ] = make_disk_line ( number, disk_name,
                                                size, total, largest );
}


/*
 * update_disk_lines updates all lines in the Disk_panel_text array.
 * Also updates the partition lines for the disks.
 *
 */

PRIVATE
void
update_disk_lines ( void )
{
    register
    char    *string;
    uint    disk,
            key,
            number;
    char    *disk_name;
    bool    looping;
    uint    size,
            total,
            largest;
    static
    bool    first_call = TRUE;
    Drive_Information_Record    info;
    CARDINAL32                  error;

    for ( disk = 0;  disk < Total_disks;  ++disk ) {
        info = Get_Drive_Status ( Disk.Drive_Control_Data [disk].Drive_Handle,
                                  &error );
        if ( error ) {
            Quit ( Cannot_get_disk_data );
        }
        number = Disk.Drive_Control_Data [disk].Drive_Number;
        size = Disk.Drive_Control_Data [disk].Drive_Size / SECTORS_PER_MEG;
        total = info.Total_Available_Sectors / SECTORS_PER_MEG;
        largest = info.Largest_Free_Block_Of_Sectors / SECTORS_PER_MEG;
        disk_name = info.Drive_Name;

        Disk_panel_text [ disk ] = make_disk_line ( number, disk_name,
                                                    size, total, largest );
        update_partition_lines ( disk );

        if ( first_call ) {
            if ( info.Corrupt_Partition_Table ) {               /* corrupt disk */
               static char str[128];
               sprintf(str,"( error codes: %i, %i )",info.LastErrorIOCTL, info.LastError);
                Disk_warning_text [ 2 ] = str;
                Disk_warning_text [ 3 ] = Corrupt_disk;
                string = Disk_warning_text [ 1 ];
                while ( *string  &&  *string != ':' ) {
                    ++string;                              /* find number field */
                }
                string -= 4;
                sprintf ( string, "%3u):", number );

                MESSAGE_BAR_PANEL ( Continue_message_line );
                Disk_warning_panel.control |= NOT_SIZED;
                Disk_warning_panel.box.number_of_rows = 0;
                Disk_warning_panel.box.row_length = 0;
                CenterPanel ( &Disk_warning_panel );
                ShowPanel ( &Disk_warning_panel );
                for ( looping = TRUE; looping; ) {
                    key = GetKeystroke ();
                    switch ( key ) {
                        case '\r':
                            looping = FALSE;
                            break;
                    }
                }
                ErasePanel ( &Disk_warning_panel );
                str[0] = ' '; str[1] = 0;
            }

            if ( info.Unusable ) {                             /* unusable disk */
                Disk_warning_text [ 3 ] = Unusable_disk;
                string = Disk_warning_text [ 1 ];
                while ( *string  &&  *string != ':' ) {
                    ++string;                              /* find number field */
                }
                string -= 4;
                sprintf ( string, "%3u):", number );

                MESSAGE_BAR_PANEL ( Continue_message_line );
                Disk_warning_panel.control |= NOT_SIZED;
                Disk_warning_panel.box.number_of_rows = 0;
                Disk_warning_panel.box.row_length = 0;
                CenterPanel ( &Disk_warning_panel );
                ShowPanel ( &Disk_warning_panel );
                for ( looping = TRUE; looping; ) {
                    key = GetKeystroke ();
                    switch ( key ) {
                        case '\r':
                            looping = FALSE;
                            break;
                    }
                }
                ErasePanel ( &Disk_warning_panel );
            }
        }
    }

    first_call = FALSE;
}



/*
 * partition routines
 *
 */


/*
 * can_create_partition:
 * Is it possible to create a partition from the given free_space
 * partition?
 *
 */

PRIVATE
bool
can_create_partition ( uint   disk,
                       uint   partition,
                       bool   bootable )
{
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error,
                                    bit_map = 0,
                                    can_create;
    Partition_Information_Array     info;

    info = Get_Partitions ( Disk.Drive_Control_Data [disk].Drive_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

    bit_map = Get_Valid_Options ( record [ partition ].Partition_Handle,
                                  &error );
    if ( error ) {
        DoEngineErrorPanel ( error );
    }

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

    if (bootable)
        can_create = CAN_BOOT_PRIMARY | CAN_BOOT_LOGICAL;
    else
        can_create = CREATE_PRIMARY_PARTITION | CREATE_LOGICAL_DRIVE;

    return ( bit_map & can_create );
}

/*
 * can_boot_partition
 * Is this a bootable partition?
 *
 */

//PRIVATE
bool
can_boot_partition ( uint   disk,
                     uint   partition )
{
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error,
                                    bit_map = 0;
    Partition_Information_Array     info;
  API_ENTRY( "can_boot_partition" )
printf("can_boot_partition %i %i\n", disk, partition );

    info = Get_Partitions ( Disk.Drive_Control_Data [disk].Drive_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

  if ( Logging_Enabled )
  {
    sprintf(Log_Buffer,"+-+ can_boot_partition: disk %i handle=%x\n",disk,Disk.Drive_Control_Data [disk].Drive_Handle);
    Write_Log_Buffer();
    sprintf(Log_Buffer,"+-+ can_boot_partition: partition %i handle=%x\n",partition, record [ partition ].Partition_Handle);
    Write_Log_Buffer();
  }

    bit_map = Get_Valid_Options ( record [ partition ].Partition_Handle,
                                  &error );
    if ( error ) {
        DoEngineErrorPanel ( error );
    }

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }
  if ( Logging_Enabled )
  {
    sprintf(Log_Buffer,"!!!!can_boot_partition bit_map=%x, CAN_BOOT_PRIMARY=%x, CAN_BOOT_LOGICAL=%x\n",
     bit_map, CAN_BOOT_PRIMARY , CAN_BOOT_LOGICAL);
    Write_Log_Buffer();
  }
    API_EXIT( "can_boot_partition" )
     return ( bit_map & (CAN_BOOT_PRIMARY | CAN_BOOT_LOGICAL) );
}


/*
 * generate_partition_name generates a unique system name for a partition.
 * The name buffer is static, so the name must be copied out before
 * calling for another name.
 *
 */

PRIVATE
char *
generate_partition_name ( uint    disk )
{
    uint    index,
            letter = 'A';
    bool    finished = FALSE,
            duplicate;
    static
    char    buffer [PARTITION_NAME_WIDTH + 2];
    register
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error;
    Partition_Information_Array     info;

    info = Get_Partitions ( Disk.Drive_Control_Data [disk].Drive_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

    while ( ! finished ) {
        sprintf ( buffer, "[ %c ]", letter );
        for ( index = 0;  index < info.Count;  ++index, ++record ) {
            duplicate = ! strncmp ( buffer, record->Partition_Name, 5 );
            if ( duplicate ) {
                break;
            }
        }
        record = info.Partition_Array;
        ++letter;
        if ( letter == 'Z' + 1 ) {
            letter = 'a';                                    /* skip non-alphas */
        }
        if ( ! duplicate ) {
            finished = TRUE;
        }
    }

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

    return  buffer;

}


/*
 * create_legacy_partition leads the user through the create partition steps.
 * Can only be used to create a legacy partition from free space.
 * Allocates primary partitions from the start, logicals from the end
 * of the free space, if user agrees.
 * Never asks for bootable because there is no way to know what the user
 * intends to do with the partition.
 *
 */

PRIVATE
uint
create_legacy_partition ( uint  disk,
                          uint  partition,
                          uint  bit_map,
                          Partition_Information_Record    *record )
{
    uint    menu = 0,
            key = ESCAPE;
    bool    finished = FALSE,
            bootable = FALSE,
            primary,
            allocate_from_start;
    char    *partition_name;
    uint    size,
            max_size;
    CARDINAL32      error;

    if ( bit_map & CREATE_LOGICAL_DRIVE ) {
        Partition_type_text [ 0 ] [ 0 ] = ' ';
    } else {
        Partition_type_text [ 0 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }
    if ( bit_map & CREATE_PRIMARY_PARTITION ) {
        Partition_type_text [ 1 ] [ 0 ] = ' ';
    } else {
        Partition_type_text [ 1 ] [ 0 ] = NOT_SELECTABLE_CHAR;
    }

    while ( ! finished ) {
        switch ( menu ) {
            case 0:                                     /* Partition_type_panel */
                MESSAGE_BAR_PANEL ( Partition_type_message_line );
                key = MenuPanel ( &Partition_type_panel );
                switch ( key ) {
                    case '\r':
                        menu = 1;
                        primary = Partition_type_panel.choice;
                        break;
                    case ESCAPE:
                        finished = TRUE;
                        ErasePanel ( &Partition_type_panel );
                        break;
                }
                break;
            case 1:                                 /* Partition_position_panel */
                MESSAGE_BAR_PANEL ( Partition_position_message_line );
                Partition_position_panel.choice = primary;
                key = MenuPanel ( &Partition_position_panel );
                switch ( key ) {
                    case '\r':
                        menu = 2;
                        allocate_from_start = Partition_position_panel.choice;
                        break;
                    case ESCAPE:
                        menu = 0;
                        ErasePanel ( &Partition_position_panel );
                        break;
                }
                break;
            case 2:                                     /* Partition_name_panel */
                partition_name = generate_partition_name ( disk );
                key = InputName ( &Partition_name_panel,
                                  Partition_name_input [0],
                                  PARTITION_NAME_WIDTH,
                                  partition_name );
                switch ( key ) {
                    case '\r':                              /* change the name */
                        menu = 3;
                        partition_name = Partition_name_input [0];
                        break;
                    case ESCAPE:                                    /* cancel */
                        menu = 1;
                        ErasePanel ( &Partition_name_panel );
                        break;
                }
                break;
            case 3:                                     /* Partition_size_panel */
                MESSAGE_BAR_PANEL ( Partition_size_message_line );
                max_size = record [ partition ].Usable_Partition_Size /
                                                SECTORS_PER_MEG;
                sprintf ( Partition_size_input [0], "%-*u",
                          PARTITION_SIZE_WIDTH, max_size );
                key = DataEntryPanel ( &Partition_size_panel );
                switch ( key ) {
                    case '\r':                              /* change the size */
                        menu = 4;
                        size = atoi ( Partition_size_input [0] );
                        if ( size == 0  ||  size > max_size ) {
                            menu = 3;
                        }
                        size *= SECTORS_PER_MEG;
                        break;
                    case ESCAPE:                            /* cancel */
                        menu = 2;
                        ErasePanel ( &Partition_size_panel );
                        break;
                    default:
                        break;
                }
                break;
            case 4:                                     /* create the partition */
                finished = TRUE;
                ErasePanel ( &Partition_size_panel );
                ErasePanel ( &Partition_name_panel );
                ErasePanel ( &Partition_position_panel );
                ErasePanel ( &Partition_type_panel );

                Create_Partition ( record [ partition ].Partition_Handle,
                                   size,
                                   partition_name,
                                   0,                           /* algorithm */
                                   bootable,
                                   primary,
                                   allocate_from_start,
                                   &error );
                if ( error ) {
                    DoEngineErrorPanel ( error );
                }
                update_disk_line ( disk );
                update_partition_lines ( disk );
                break;
            default:
                break;
        }
    }

    return  key;
}


/*
 * delete_partition leads the user through the delete partition steps.
 * The lines will be erased and recreated by update_partition_lines.
 *
 */

PRIVATE
uint
delete_partition ( uint    disk,
                   uint    partition,
                   Partition_Information_Record    *record )
{
    char        *line,
                *partition_name;
    uint        menu = 0,
                key;
    bool        delete = FALSE,
                finished = FALSE;
    CARDINAL32  error;

    line = Show_partition_text [ 0 ];
    partition_name = Partition_panel_text [ disk ] [ partition ] +
                                                     PARTITION_NAME_OFFSET;

    sprintf ( line, " %-*.*s ",
              PARTITION_NAME_WIDTH, PARTITION_NAME_WIDTH, partition_name );

    while ( ! finished ) {
        switch ( menu ) {
            case 0:
                key = MenuPanel ( &Show_partition_panel );
                switch ( key ) {
                    case '\r':
                        menu = 1;
                        break;
                    case ESCAPE:
                        finished = TRUE;
                        ErasePanel ( &Show_partition_panel );
                        break;
                }
                break;
            case 1:
                Delete_partition_panel.choice = 0;                /* not delete */
                key = MenuPanel ( &Delete_partition_panel );
                switch ( key ) {
                    case '\r':
                        delete = Delete_partition_panel.choice;
                        break;
                    case ESCAPE:
                        break;
                }
                finished = TRUE;
                ErasePanel ( &Delete_partition_panel );
                ErasePanel ( &Show_partition_panel );
                break;
            default:
                break;
        }
    }

    if ( delete ) {
        Delete_Partition ( record [ partition ].Partition_Handle, &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        }

        update_disk_line ( disk );
        update_partition_lines ( disk );
    }

    return  key;
}


/*
 * do_Partition_options_menu shows the menu and acts on input events
 *
 */

PRIVATE
uint
do_Partition_options_menu ( uint    disk,
                            uint    partition )
{
    uint    key;
    bool    finished = FALSE;
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error,
                                    bit_map,
                                    can_create_partition;
    Partition_Information_Array     info;

    info = Get_Partitions ( Disk.Drive_Control_Data [disk].Drive_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

    bit_map = Get_Valid_Options ( record [ partition ].Partition_Handle,
                                  &error );
    if ( error ) {
        DoEngineErrorPanel ( error );
    }

    if ( IS_FREE_SPACE ( disk, partition ) ) {
        Partition_options_text [0] [0] = ' ';
        Partition_options_text [1] [0] = NOT_SELECTABLE_CHAR;
        Partition_options_text [2] [0] = NOT_SELECTABLE_CHAR;
        Partition_options_text [3] [0] = NOT_SELECTABLE_CHAR;
    } else {
        Partition_options_text [0] [0] = NOT_SELECTABLE_CHAR;
        Partition_options_text [1] [0] = NOT_SELECTABLE_CHAR;
        Partition_options_text [2] [0] = ' ';
        Partition_options_text [3] [0] = ' ';
    }
    if ( IS_AVAILABLE_PARTITION ( disk, partition ) ) {
        Partition_options_text [1] [0] = ' ';
        Partition_options_text [3] [0] = NOT_SELECTABLE_CHAR;
    }
    if ( record [ partition ].Volume_Handle == NULL  &&  ( !IS_FREE_SPACE ( disk, partition ) )) {
        Partition_options_text [3] [0] = NOT_SELECTABLE_CHAR;
        Partition_options_text [1] [0] = ' '; //можно удалять ? EK
    }

    can_create_partition = CREATE_PRIMARY_PARTITION | CREATE_LOGICAL_DRIVE;
    if ( ! (bit_map & can_create_partition) ) {
        Partition_options_text [0] [0] = NOT_SELECTABLE_CHAR;
    }
    if ( ! (bit_map & CAN_SET_NAME ) ) {
        Partition_options_text [2] [0] = NOT_SELECTABLE_CHAR;
    }

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Menu_options_message_line );
        key = MenuPanel ( &Partition_options_menu );
        switch ( key ) {
            case '\r':
                switch ( Partition_options_menu.choice ) {
                    case 0:
                        key = create_legacy_partition ( disk, partition,
                                                        bit_map, record );
                        break;
                    case 1:
                        key = delete_partition ( disk, partition, record );
                        break;
                    case 2:
                        key = ChangePartitionName (
                                        Partition_name_panel.box.row,
                                        Partition_name_panel.box.column,
                                        record [ partition ].Partition_Name,
                                        record [ partition ].Partition_Handle );
                        if ( key == '\r' ) {
                            update_partition_lines ( disk );
                        }
                        break;
                    case 3:
                        key = ChangeVolumeName (
                                          Partition_name_panel.box.row,
                                          Partition_name_panel.box.column,
                                          record [ partition ].Volume_Name,
                                          record [ partition ].Volume_Handle );
                        if ( key == '\r' ) {
                            update_disk_lines ();                /* must do all */
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

    ErasePanel ( &Partition_options_menu );

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }

    return  key;
}


/*
 * ChangePartitionName gets a disk name from the user using InputName, and
 * then changes it by calling Set_Name.
 * The main purpose is to keep the entry panel private, and to handle
 * repositioning the panel.
 * Must erase the panel here because it is repositioned.
 * It returns the key from InputName.
 *
 */

PUBLIC
uint
ChangePartitionName ( uint   row,
                      uint   column,
                      char   *old_name,
                      void   *handle )
{
    uint        key,
                save_row = Partition_name_panel.box.row,
                save_column = Partition_name_panel.box.column;
    CARDINAL32  error;

    Partition_name_panel.box.row = row;
    Partition_name_panel.box.column = column;

    key = InputName ( &Partition_name_panel,
                      Partition_name_input [0],
                      PARTITION_NAME_WIDTH,
                      old_name );

    if ( key == '\r' ) {
        Set_Name ( handle, Partition_name_input [0], &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        }
    }

    ErasePanel ( &Partition_name_panel );

    Partition_name_panel.box.row = save_row;
    Partition_name_panel.box.column = save_column;

    return  key;
}


/*
 * disk routines
 *
 */


/*
 * disk_panel_callback updates the partitions panel in the background
 *
 */

PRIVATE
uint _System disk_panel_callback ( panel_t *panel )
{
    uint    disk;

    disk = panel->choice;
    Partition_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
    Partition_panel.text_line = Partition_panel_text [ disk ];
    ShowPanel ( &Partition_panel );
    return(0);
}


/*
 * use_free_space:
 * Create and add a partition to the Partition_handles array.
 * Always creates a logical at the end of free space, primary at beginning.
 *
 */

PRIVATE
uint
use_free_space ( uint   disk )
{
    uint    menu = 0,
            free_space = 0,
            partition,
            index,
            number,
            i, j,
            key = ESCAPE,
            max_size,
            size;
    char    *partition_name,
            *partition_size;
    bool    finished = FALSE,
            bootable = Create_bootable_partition,
            primary = Create_primary_partition,
            allocate_from_start = Create_primary_partition;
    uint    save_row = Partition_name_panel.box.row,
            save_column = Partition_name_panel.box.column,
            save_row2 = Partition_size_panel.box.row,
            save_column2 = Partition_size_panel.box.column;
    char    **partition_text = Partition_panel_text [ disk ],
            **free_space_text;
    register
    Partition_Information_Record    *record;
    void                            *memory;
    ADDRESS                         partition_handle,
                                    free_space_handle;
    CARDINAL32                      error,
                                    bit_map,
                                    free_space_size,
                                    boot_limit;
    Partition_Information_Array     info;

    Partition_name_panel.box.row = Choose_partition_menu.box.row + 2;
    Partition_name_panel.box.column = Choose_partition_menu.box.column + 2;
    Partition_size_panel.box.row = Partition_name_panel.box.row + 2;
    Partition_size_panel.box.column = Partition_name_panel.box.column + 2;

    for ( number = 0;  partition_text [ number ];  ++number ) {
    }                                                          /* count them up */
    ++number;
    free_space_text = AllocateOrQuit ( number, sizeof ( char ** ) );

    for ( partition = 0; partition_text [ partition ]; ++partition ) {
        if ( IS_FREE_SPACE ( disk, partition ) && can_create_partition ( disk, partition, bootable ) ) {
            free_space_text [ free_space ] = partition_text [ partition ];
            ++free_space;
        }
    }
    free_space_text [ free_space ] = NULL;

    Choose_partition_menu.control |= (INITIALIZE_PANEL | NOT_SIZED);
    Choose_partition_menu.box.number_of_rows = 0;
    Choose_partition_menu.text_line = free_space_text;

    while ( ! finished ) {
        switch ( menu ) {
            case 0:                                    /* Choose_partition_menu */
                MESSAGE_BAR_PANEL ( Add_partition_message_line );
                key = ScrollingMenuPanel ( &Choose_partition_menu );
                switch ( key ) {
                    case '\r':
                        menu = 1;
                        free_space = Choose_partition_menu.choice;
                        break;
                    case ESCAPE:                                      /* cancel */
                        finished = TRUE;
                        ErasePanel ( &Choose_partition_menu );
                        break;
                }
                break;
            case 1:                                     /* Partition_name_panel */
                partition_name = generate_partition_name ( disk );
                key = InputName ( &Partition_name_panel,
                                  Partition_name_input [0],
                                  PARTITION_NAME_WIDTH,
                                  partition_name );
                switch ( key ) {
                    case '\r':                              /* change the name */
                        menu = 2;
                        partition_name = Partition_name_input [0];
                        break;
                    case ESCAPE:                                    /* cancel */
                        menu = 0;
                        ErasePanel ( &Partition_name_panel );
                        break;
                }
                break;
            case 2:                                     /* Partition_size_panel */
                MESSAGE_BAR_PANEL ( Partition_size_message_line );
                partition_size =
                     &free_space_text [ free_space ] [ PARTITION_SIZE_OFFSET ];
                max_size = atoi ( partition_size );
                sprintf ( Partition_size_input [0], "%-*u",
                          PARTITION_SIZE_WIDTH, max_size );
                key = DataEntryPanel ( &Partition_size_panel );
                switch ( key ) {
                    case '\r':                               /* change the size */
                        menu = 3;
                        size = atoi ( Partition_size_input [0] );
                        if ( size == 0  ||  size > max_size ) {
                            menu = 2;
                        }
                        size *= SECTORS_PER_MEG;
                        break;
                    case ESCAPE:                                      /* cancel */
                        menu = 1;
                        ErasePanel ( &Partition_size_panel );
                        break;
                    default:
                        break;
                }
                break;
            case 3:                                     /* create the partition */
                finished = TRUE;
                ErasePanel ( &Partition_size_panel );
                ErasePanel ( &Partition_name_panel );
                ErasePanel ( &Choose_partition_menu );

                index = 0;                   /* convert free_space to partition */

                for ( partition = 0; partition_text [partition]; ++partition ) {
                    if ( IS_FREE_SPACE ( disk, partition ) &&
                         can_create_partition ( disk, partition, bootable ) ) {
                        if ( index == free_space ) {
                            break;                          /* partition is set */
                        }
                        ++index;
                    }
                }
                info = Get_Partitions (
                                   Disk.Drive_Control_Data [disk].Drive_Handle,
                                   &error );
                if ( error ) {
                    Quit ( Cannot_get_disk_data );
                }
                record = info.Partition_Array;         /* get free space handle */
                memory = record;
                free_space_handle = record [ partition ].Partition_Handle;

                if ( bootable ) {                           /* check boot_limit */
                    boot_limit = record [partition].Boot_Limit;
                    free_space_size = record [partition].Usable_Partition_Size;
                    if ( free_space_size > boot_limit ) {
                        uint primaryCount;

                        allocate_from_start = TRUE;
                        bit_map = Get_Valid_Options ( free_space_handle,
                                                      &error );
                        if ( error ) {
                            DoEngineErrorPanel ( error );
                        }

                        /* Count # primary partitions currently on drive.
                         * Once we hit 3 primaries, our preference for type of
                         * partition switches from primary to extended.  Using
                         * this last slot for extended will enable additional
                         * extendeds on this drive. (defect 219899)
                         */
                        for ( i = 0, primaryCount = 0; i < info.Count; ++i ) {
                            if ( record[i].Primary_Partition )
                                ++primaryCount;
                        }

                        if ( bit_map & CAN_BOOT_PRIMARY  &&  primaryCount < 3 ) {
                            primary = TRUE;
                        } else {
                            primary = FALSE;
                        }

                     /* if ( size > boot_limit ) {              */
                     /*     size = boot_limit;                  */
                     /* }                                       */

                    }
                }

                partition_handle = Create_Partition ( free_space_handle,
                                                      size,
                                                      partition_name,
                                                      0,           /* algorithm */
                                                      bootable,
                                                      primary,
                                                      allocate_from_start,
                                                      &error );

                if ( error ) {                                       /* 215671 */
                    /* May fail attempting to create a logical (or primary)
                     * in situation where it can't be done.
                     * Retry, switching primary / logical choice.
                     * This is OK, since user doesn't specify this.
                     */
                    primary = !primary;
                    partition_handle = Create_Partition ( free_space_handle,
                                                          size,
                                                          partition_name,
                                                          0,           /* algorithm */
                                                          bootable,
                                                          primary,
                                                          allocate_from_start,
                                                          &error );
                }
                if ( error ) {
                    DoEngineErrorPanel ( error );
                }
                if ( memory ) {
                    Free_Engine_Memory ( memory );
                }
                if ( partition_handle ) {
                    Partition_handles [ Partition_count ] = partition_handle;
                    ++Partition_count;
                }

                info = Get_Partitions (
                                   Disk.Drive_Control_Data [disk].Drive_Handle,
                                   &error );
                if ( error ) {
                    Quit ( Cannot_get_disk_data );
                }
                update_partition_lines ( disk );

                record = info.Partition_Array;                /* get new handle */
                memory = record;
                                                /* must redo all not-availables */

                for ( i = 0;  i < Partition_count; ++i ) {
                    record = info.Partition_Array;                  /* reset it */
                    partition_handle = Partition_handles [ i ];
                    for ( j = 0;  j < info.Count;  ++j, ++record ) {
                        if ( record->Partition_Handle == partition_handle ) {
                            SET_NOT_AVAILABLE ( disk, j );
                        }
                    }
                }
                if ( memory ) {
                    Free_Engine_Memory ( memory );
                }
                break;
            default:
                break;
        }
    }

    Partition_name_panel.box.row = save_row;
    Partition_name_panel.box.column = save_column;
    Partition_size_panel.box.row = save_row2;
    Partition_size_panel.box.column = save_column2;

    if ( free_space_text ) {
        free ( free_space_text );
    }

    return  key;
}


/*
 * use_existing_partition:
 * Add an existing partition to the Partition_handles array.
 *
 */

PRIVATE
uint
use_existing_partition ( uint   disk )
{
    uint    menu = 0,
            existing = 0,
            number,
            partition,
            index,
            key = ESCAPE;
    char    *partition_name;
    bool    finished = FALSE,
            bootable = Create_bootable_partition;
    uint    save_row = Partition_name_panel.box.row,
            save_column = Partition_name_panel.box.column;
    char    **partition_text = Partition_panel_text [ disk ],
            **existing_text;
    Partition_Information_Record    *record;
    void                            *memory;
    ADDRESS                         partition_handle;
    Partition_Information_Array     info;
    CARDINAL32                      error;

    Partition_name_panel.box.row = Choose_partition_menu.box.row + 2;
    Partition_name_panel.box.column = Choose_partition_menu.box.column + 2;

    for ( number = 0;  partition_text [ number ];  ++number ) {
    }                                                          /* count them up */
    ++number;
    existing_text = AllocateOrQuit ( number, sizeof ( char ** ) );

    for ( partition = 0; partition_text [ partition ]; ++partition ) {
        if ( IS_AVAILABLE_PARTITION ( disk, partition ) ) {
            if ( !bootable || can_boot_partition( disk, partition ) ) {
                existing_text [ existing ] = partition_text [ partition ];
                ++existing;
            }
        }
    }
    existing_text [ existing ] = NULL;

    Choose_partition_menu.control |= (INITIALIZE_PANEL | NOT_SIZED);
    Choose_partition_menu.box.number_of_rows = 0;
    Choose_partition_menu.text_line = existing_text;

    info = Get_Partitions ( Disk.Drive_Control_Data [disk].Drive_Handle,
                            &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    record = info.Partition_Array;
    memory = record;

    while ( ! finished ) {
        switch ( menu ) {
            case 0:                                    /* Choose_partition_menu */
                MESSAGE_BAR_PANEL ( Add_partition_message_line );
                key = ScrollingMenuPanel ( &Choose_partition_menu );
                existing = Choose_partition_menu.choice;
                switch ( key ) {
                    case '\r':                              /* select partition */
                        menu = 1;
                        partition_name = existing_text [ existing ];
                        break;
                    case ESCAPE:                                      /* cancel */
                        finished = TRUE;
                        ErasePanel ( &Choose_partition_menu );
                        break;
                }
                break;
            case 1:                                     /* Partition_name_panel */
                key = InputName ( &Partition_name_panel,
                                  Partition_name_input [0],
                                  PARTITION_NAME_WIDTH,
                                  partition_name );
                switch ( key ) {
                    case '\r':                               /* change the name */
                        menu = 2;
                        partition_name = Partition_name_input [0];
                        while ( *partition_name == ' ' ) {
                            ++partition_name;
                        }
                        break;
                    case ESCAPE:                                      /* cancel */
                        menu = 0;
                        ErasePanel ( &Partition_name_panel );
                        break;
                }
                break;
            case 2:                                        /* add the partition */
                finished = TRUE;
                ErasePanel ( &Partition_name_panel );
                ErasePanel ( &Choose_partition_menu );

                index = 0;                     /* convert existing to partition */

                for ( partition = 0; partition_text [partition]; ++partition ) {
                    if ( IS_AVAILABLE_PARTITION ( disk, partition ) ) {
                        if ( !bootable || can_boot_partition( disk, partition ) ) {
                            if ( index == existing ) {
                                break;                          /* partition is set */
                            }
                            ++index;
                        }
                    }
                }
                partition_handle = record [ partition ].Partition_Handle;
                Set_Name ( partition_handle, partition_name, &error );
                if ( error ) {
                    DoEngineErrorPanel ( error );
                } else {
                    Partition_handles [ Partition_count ] = partition_handle;
                    ++Partition_count;
                    SET_NOT_AVAILABLE ( disk, partition );
                }
                break;
            default:
                break;
        }
    }

    Partition_name_panel.box.row = save_row;
    Partition_name_panel.box.column = save_column;

    if ( memory ) {
        Free_Engine_Memory ( memory );
    }
    if ( existing_text ) {
        free ( existing_text );
    }

    return  key;
}


/*
 * do_Disk_options_menu shows the menu and acts on input events
 *
 */

PRIVATE
uint
do_Disk_options_menu ( uint disk )
{
    uint    key;
    bool    finished = FALSE;
    char    *disk_name;
    Drive_Information_Record    info;
    CARDINAL32                  error;

    info = Get_Drive_Status ( Disk.Drive_Control_Data [disk].Drive_Handle,
                              &error );
    if ( error ) {
        Quit ( Cannot_get_disk_data );
    }
    disk_name = info.Drive_Name;

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Menu_options_message_line );
        key = MenuPanel ( &Disk_options_menu );
        switch ( key ) {
            case '\r':
                switch ( Disk_options_menu.choice ) {
                    case 0:                                     /* use existing */
                        key = use_existing_partition ( disk );
                        break;
                    case 1:                                  /* from free space */
                        key = use_free_space ( disk );
                        break;
                    case 2:                                        /* separator */
                        break;
                    case 3:
                        key = ChangeDiskName ( Disk_name_panel.box.row,
                                               Disk_name_panel.box.column,
                                               disk_name,
                                 Disk.Drive_Control_Data [disk].Drive_Handle );
                        break;
                    default:
                        break;
                }
                if ( key == '\r' ) {                        /* action completed */
                    finished = TRUE;
                    update_disk_line ( disk );
                }
                break;
            case ESCAPE:
                finished = TRUE;
                break;
            default:
                break;
        }
    }

    ErasePanel ( &Disk_options_menu );

    return  key;
}


/*
 * NoStartablePartition returns true if bootmanager is not installed
 * and there is not a startable partition on disk 1.
 *
 */

PUBLIC
bool
NoStartablePartition ( void )
{
    bool    none_startable = TRUE;
    uint    index;
    register
    Partition_Information_Record    *record;
    void                            *memory;
    CARDINAL32                      error;
    BOOLEAN                         bootman_installed;
    BOOLEAN                         bootman_active;
    Partition_Information_Array     info;

    bootman_installed = Boot_Manager_Is_Installed ( &bootman_active, &error );
    if ( error ) {
        DoEngineErrorPanel ( error );
    } else {
        if ( bootman_installed && bootman_active ) {
            none_startable = FALSE;
        }
    }

    if ( none_startable ) {                                  /* check for startable */

        info = Get_Partitions ( Disk.Drive_Control_Data [ 0 ].Drive_Handle,
                                &error );
        if ( error ) {
            Quit ( Cannot_get_disk_data );
        }
        record = info.Partition_Array;
        memory = record;

        for ( index = 0;  index < info.Count;  ++index, ++record ) {
            if ( record->Primary_Partition  &&  record->Active_Flag ) {
                none_startable = FALSE;
                break;                                       /* found startable */
            }
        }

        if ( memory ) {
            Free_Engine_Memory ( memory );
        }
    }

    return  none_startable;
}


/*
 * ChangeDiskName gets a disk name from the user using InputName, and
 * then changes it by calling Set_Name.
 * The main purpose is to keep the entry panel private, and to handle
 * repositioning the panel.
 * Must erase the panel here because it is repositioned.
 * It returns the key from InputName.
 *
 */

PUBLIC
uint
ChangeDiskName ( uint   row,
                 uint   column,
                 char   *old_name,
                 void   *handle )
{
    uint        key,
                save_row = Disk_name_panel.box.row,
                save_column = Disk_name_panel.box.column;
    CARDINAL32  error;

    Disk_name_panel.box.row = row;
    Disk_name_panel.box.column = column;

    key = InputName ( &Disk_name_panel,
                      Disk_name_input [0],
                      DISK_NAME_WIDTH,
                      old_name );

    if ( key == '\r' ) {
        Set_Name ( handle, Disk_name_input [0], &error );
        if ( error ) {
            DoEngineErrorPanel ( error );
        }
    }

    ErasePanel ( &Disk_name_panel );

    Disk_name_panel.box.row = save_row;
    Disk_name_panel.box.column = save_column;

    return  key;
}


/*
 * GetDriveNumber returns the disk number for a drive handle.
 *
 */

PUBLIC
uint
GetDriveNumber ( ADDRESS    drive_handle )
{
    uint    drive_number = 0,
            disk;

    for ( disk = 0;  disk < Total_disks;  ++disk ) {
        if ( Disk.Drive_Control_Data [disk].Drive_Handle == drive_handle ) {
            drive_number = Disk.Drive_Control_Data [disk].Drive_Number;
            break;
        }
    }

    return  drive_number;
}


/*
 * GetPartitionHandles shows the disk panels, gets partition choices
 * and returns the partition handle array and count.
 * Returns keys Esc for cancel, \r for complete the process.
 *
 */

PUBLIC
uint
GetPartitionHandles ( uint    ***partition_handles,
                      uint    *partition_count,
                      uint    max_partitions,
                      bool    bootable,
                      bool    primary,
                      bool    PRM_only )
{
    uint    key,
            disk = 0,
            partition = 0;
    bool    finished = FALSE;
    char    **partition_text,
            *message_line;

    update_disk_lines ();                               /* pick up logical info */

    Create_bootable_partition = bootable;               /* set private globals */
    Create_primary_partition = primary;

    Partition_count = 0;
    if ( max_partitions > MAX_PARTITION_HANDLES ) {
        max_partitions = MAX_PARTITION_HANDLES;
    }
    if ( max_partitions > 1 ) {
        message_line = Add_disk_message_line_F6;
        Choose_a_disk_panel.text_line = Choose_a_disk_text_F6;
    } else {
        message_line = Add_disk_message_line;
        Choose_a_disk_panel.text_line = Choose_a_disk_text;
    }
    Choose_a_disk_panel.control |= NOT_SIZED;
    Choose_a_disk_panel.box.number_of_rows = 0;
    Choose_a_disk_panel.box.row_length = 0;
    CenterPanel ( &Choose_a_disk_panel );

    Disk_header_panel.control |= SAVE_RESTORE;
    Disk_panel.control |= SAVE_RESTORE;
    Partition_header_panel.control |= SAVE_RESTORE;
    Partition_panel.control |= SAVE_RESTORE;

    ShowPanel ( &Program_title_panel );
    ShowPanel ( &Disk_header_panel );
    ShowPanel ( &Disk_panel );
    ShowPanel ( &Partition_header_panel );
    ShowPanel ( &Partition_panel );

    MESSAGE_BAR_PANEL ( Continue_message_line );
    ShowPanel ( &Choose_a_disk_panel );
    loop {
        key = GetKeystroke ();
        if ( key == '\r' ) {
            break;
        }
    }
    ErasePanel ( &Choose_a_disk_panel );

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( message_line );
        key = ScrollingMenuPanel ( &Disk_panel );
        disk = Disk_panel.choice;

        switch ( key ) {
            case '\r':
                Disk_options_text [0] [0] = NOT_SELECTABLE_CHAR;
                Disk_options_text [1] [0] = NOT_SELECTABLE_CHAR;
                Disk_options_text [3] [0] = NOT_SELECTABLE_CHAR;
                partition_text = Partition_panel_text [ disk ];

                /* Enable Partition and Free Space options if suitable partitions
                 * and free space exist on the drive.
                 * Don't enable any options if PRM is required (PRM_only) and drive
                 * is not a PRM (defect 219338).
                 */
                if ( !PRM_only  ||  Disk.Drive_Control_Data [disk].Drive_Is_PRM ) {
                    for ( partition = 0; partition_text [partition]; ++partition ) {

//#define IS_AVAILABLE_PARTITION(d,p) ( ! strncmp ( Partition_panel_text [d][p] +\
//PARTITION_STATUS_OFFSET, Partition_status_strings [PARTITION_IS_AVAILABLE], \
//min( strlen ( Partition_status_strings [PARTITION_IS_AVAILABLE] ), PARTITION_STATUS_WIDTH) ) )
  if ( Logging_Enabled )
  {
    sprintf(Log_Buffer,"PARTITION_STATUS_OFFSET=%i, %i\n",PARTITION_STATUS_OFFSET, PARTITION_STATUS_WIDTH);
    Write_Log_Buffer();
    sprintf(Log_Buffer,"Partition_status_strings [%i]=%s\n",PARTITION_IS_AVAILABLE,Partition_status_strings [PARTITION_IS_AVAILABLE]);
    Write_Log_Buffer();
    sprintf(Log_Buffer,"%s\n",Partition_panel_text [disk][partition]+PARTITION_STATUS_OFFSET);
    Write_Log_Buffer();
  }

printf("PARTITION_STATUS_OFFSET=%i, %i\n",PARTITION_STATUS_OFFSET, PARTITION_STATUS_WIDTH);
printf("Partition_status_strings [%i]=%s\n",PARTITION_IS_AVAILABLE,Partition_status_strings [PARTITION_IS_AVAILABLE]);
printf("%s\n",Partition_panel_text [disk][partition]+PARTITION_STATUS_OFFSET);

  if ( Logging_Enabled )
  {
    sprintf(Log_Buffer,"IS_AVAILABLE_PARTITION ( %i, %i)=%i\n", disk, partition, IS_AVAILABLE_PARTITION ( disk, partition ));
    Write_Log_Buffer();
    sprintf(Log_Buffer,"bootable=%i,can_boot_partition( %i, %i )= %i\n",
               bootable, disk, partition, can_boot_partition( disk, partition ) );
    Write_Log_Buffer();
  }
                        if ( IS_AVAILABLE_PARTITION ( disk, partition ) ) {
                            if ( !bootable || can_boot_partition( disk, partition ) ) {
                                Disk_options_text [0] [0] = ' ';
                            }
                        }
                        if ( IS_FREE_SPACE ( disk, partition ) ) {
                            if ( can_create_partition ( disk, partition, bootable ) ) {
                                Disk_options_text [1] [0] = ' ';
                            }
                        }
                    }
                }
                key = do_Disk_options_menu ( disk );
                if ( Partition_count >= max_partitions ) {
                    finished = TRUE;
                    key = '\r';                            /* complete creation */
                }
                break;
            case ESCAPE:
                finished = TRUE;
                break;
            case F6:
                finished = TRUE;
                key = '\r';                                /* complete creation */
                break;
            default:
                ;
        }
    }

    ErasePanel ( &Partition_panel );
    ErasePanel ( &Partition_header_panel );
    ErasePanel ( &Disk_panel );
    ErasePanel ( &Disk_header_panel );
    ErasePanel ( &Program_title_panel );

    Disk_header_panel.control &= ~SAVE_RESTORE;
    Disk_panel.control &= ~SAVE_RESTORE;
    Partition_header_panel.control &= ~SAVE_RESTORE;
    Partition_panel.control &= ~SAVE_RESTORE;

    *partition_count = Partition_count;
    *partition_handles = Partition_handles;

    return  key;
}


/*
 * DoDiskPanel shows the panel and acts on input events
 *
 */

PUBLIC
uint
DoDiskPanel ( void )
{
    uint    key,
            disk = 0,
            partition = 0;
    bool    scrolling,
            finished = FALSE;

    update_disk_lines ();                               /* pick up logical info */

    ShowPanel ( &Program_title_panel );
    ShowPanel ( &Disk_header_panel );
    ShowPanel ( &Disk_panel );
    ShowPanel ( &Partition_header_panel );
    ShowPanel ( &Partition_panel );

    while ( ! finished ) {
        MESSAGE_BAR_PANEL ( Disk_message_line );
        key = ScrollingMenuPanel ( &Disk_panel );
        disk = Disk_panel.choice;

        switch ( key ) {
            case '\t':
                scrolling = TRUE;
                while ( scrolling ) {
                    MESSAGE_BAR_PANEL ( Disk_message_line );
                    key = ScrollingMenuPanel ( &Partition_panel );
                    partition = Partition_panel.choice;

                    switch ( key ) {
                        case '\t':
                        case ESCAPE:
                            scrolling = FALSE;
                            break;
                        case '\r':
                            key = do_Partition_options_menu ( disk, partition );
                            Partition_panel.control |= (INITIALIZE_PANEL |
                                                        NOT_SIZED);
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
                Disk_options_text [0] [0] = NOT_SELECTABLE_CHAR;
                Disk_options_text [1] [0] = NOT_SELECTABLE_CHAR;
                Disk_options_text [3] [0] = ' ';
                key = do_Disk_options_menu ( disk );
                break;
            case F3:
            case F5:
                finished = TRUE;
                break;
            default:
                ;
        }
    }

    ErasePanel ( &Program_title_panel );

    return  key;
}


/*
 * ConstructDiskPanels sets the characteristics of the panel
 *
 */

PUBLIC
void
ConstructDiskPanels ( uint  top_row,
                      uint  bottom_row )
{
    uint    usable_rows;

    allocate_text_arrays ();                                   /* must be first */

    /* Disk_warning_panel       << used by update_disk_lines >> */

    Disk_warning_panel.control = WARNING_CONTROL;
    Disk_warning_panel.box.outline = Blank_outline;
    Disk_warning_panel.box.scroll_bar = Default_scroll_bar;
    Disk_warning_panel.box.row = 0;
    Disk_warning_panel.box.column = 0;
    Disk_warning_panel.box.number_of_rows = 0;
    Disk_warning_panel.box.attribute = Warning_attribute;
    Disk_warning_panel.box.border_attribute = Warning_border_attribute;
    Disk_warning_panel.text_line = Disk_warning_text;



    update_disk_lines ();                       /* initialize the text arrays */


    /* Disk_header_panel */

    Disk_header_panel.control = PANEL_CONTROL;
    Disk_header_panel.box.outline = Single_line_outline;
    Disk_header_panel.box.scroll_bar = Line_scroll_bar;
    Disk_header_panel.box.row = top_row;
    Disk_header_panel.box.column = 0;
    Disk_header_panel.box.row_length = SCREEN_WIDTH;
    Disk_header_panel.box.number_of_rows = 3;
    Disk_header_panel.box.attribute = Panel_attribute;
    Disk_header_panel.box.border_attribute = Panel_border_attribute;
    Disk_header_panel.text_line = Disk_header_text;

    /* Disk_panel */

    Disk_panel.control = PANEL_CONTROL;
    Disk_panel.box.outline = Tee_top_outline;
    Disk_panel.box.scroll_bar = Line_scroll_bar;
    Disk_panel.box.row = top_row + Disk_header_panel.box.number_of_rows - 1;
    Disk_panel.box.column = 0;
    Disk_panel.box.row_length = SCREEN_WIDTH;
    usable_rows = bottom_row - top_row - 4;
    Disk_panel.box.number_of_rows = ( usable_rows * 2 ) / 3;
    Disk_panel.box.max_number_of_rows = 26;             /* match logical window */
    Disk_panel.box.attribute = Panel_attribute;
    Disk_panel.box.border_attribute = Panel_border_attribute;
    Disk_panel.text_line = Disk_panel_text;
    Disk_panel.callback = disk_panel_callback;
    Disk_panel.help.routine = DoHelpPanel;
    Disk_panel.help.default_message = DISK_HELP_MESSAGE;

    /* Disk_options_menu */

    Disk_options_menu.control = MENU_CONTROL;
    Disk_options_menu.box.outline = Single_line_outline;
    Disk_options_menu.box.row = Disk_header_panel.box.row + 1;
    Disk_options_menu.box.column = Disk_panel.box.column + 1;
    Disk_options_menu.box.attribute = Menu_attribute;
    Disk_options_menu.box.border_attribute = Menu_border_attribute;
    Disk_options_menu.text_line = Disk_options_text;
    Disk_options_menu.help.routine = DoHelpPanel;
    Disk_options_menu.help.array = Disk_options_help_array;
    Disk_options_menu.help.array_size = Disk_options_help_array_size;

    /* Choose_partition_menu */

    Choose_partition_menu.control = MENU_CONTROL;
    Choose_partition_menu.box.outline = Single_line_outline;
    Choose_partition_menu.box.scroll_bar = Line_scroll_bar;
    Choose_partition_menu.box.row = Disk_options_menu.box.row + 4;
    Choose_partition_menu.box.column = Disk_options_menu.box.column + 4;
    Choose_partition_menu.box.row_length = PARTITION_NAME_WIDTH + 3 +
                                           PARTITION_SIZE_WIDTH;
    Choose_partition_menu.box.max_number_of_rows = bottom_row -
                                                 Choose_partition_menu.box.row;
    Choose_partition_menu.box.attribute = Menu_attribute;
    Choose_partition_menu.box.border_attribute = Menu_border_attribute;
    Choose_partition_menu.text_line = Disk_options_text;

    /* Disk_name_panel */

    Disk_name_panel.control = ENTRY_CONTROL;
    Disk_name_panel.box.outline = Single_line_outline;
    Disk_name_panel.box.row = Disk_options_menu.box.row + 4;
    Disk_name_panel.box.column = Disk_options_menu.box.column + 4;
    Disk_name_panel.box.attribute = Entry_attribute;
    Disk_name_panel.box.border_attribute = Entry_border_attribute;
    Disk_name_panel.text_line = Disk_name_text;
    Disk_name_panel.input_line = Disk_name_input;

    /* Partition_header_panel */

    Partition_header_panel.control = PANEL_CONTROL;
    Partition_header_panel.box.outline = Single_line_outline;
    Partition_header_panel.box.scroll_bar = Line_scroll_bar;
    Partition_header_panel.box.row = Disk_panel.box.row +
                                     Disk_panel.box.number_of_rows;
    Partition_header_panel.box.column = 0;
    Partition_header_panel.box.row_length = SCREEN_WIDTH;
    Partition_header_panel.box.number_of_rows = 3;
    Partition_header_panel.box.attribute = Panel_attribute;
    Partition_header_panel.box.border_attribute = Panel_border_attribute;
    Partition_header_panel.text_line = Partition_header_text;

    /* Partition_panel */

    Partition_panel.control = PANEL_CONTROL;
    Partition_panel.box.outline = Tee_top_outline;
    Partition_panel.box.scroll_bar = Line_scroll_bar;
    Partition_panel.box.row = Partition_header_panel.box.row +
                              Partition_header_panel.box.number_of_rows - 1;
    Partition_panel.box.column = 0;
    Partition_panel.box.row_length = SCREEN_WIDTH;
    Partition_panel.box.number_of_rows = bottom_row -
                                         Partition_panel.box.row + 1;
    Partition_panel.box.attribute = Panel_attribute;
    Partition_panel.box.border_attribute = Panel_border_attribute;
    Partition_panel.text_line = Partition_panel_text [ 0 ];
    Partition_panel.help.routine = DoHelpPanel;
    Partition_panel.help.default_message = DISK_HELP_MESSAGE;

    /* Partition_options_menu */

    Partition_options_menu.control = MENU_CONTROL;
    Partition_options_menu.box.outline = Single_line_outline;
    Partition_options_menu.box.row = Partition_header_panel.box.row - 2;
    Partition_options_menu.box.column = Partition_panel.box.column + 1;
    Partition_options_menu.box.attribute = Menu_attribute;
    Partition_options_menu.box.border_attribute = Menu_border_attribute;
    Partition_options_menu.text_line = Partition_options_text;

    /* Show_partition_panel */

    Show_partition_panel.control = CHOICE_CONTROL;
    Show_partition_panel.box.outline = Single_line_outline;
    Show_partition_panel.box.row = Partition_options_menu.box.row + 3;
    Show_partition_panel.box.column = Partition_options_menu.box.column + 4;
    Show_partition_panel.box.attribute = Choice_attribute;
    Show_partition_panel.box.border_attribute = Choice_border_attribute;
    Show_partition_panel.text_line = Show_partition_text;

    /* Delete_partition_panel */

    Delete_partition_panel.control = CHOICE_CONTROL;
    Delete_partition_panel.box.outline = Single_line_outline;
    Delete_partition_panel.box.row = Show_partition_panel.box.row + 2;
    Delete_partition_panel.box.column = Show_partition_panel.box.column + 2;
    Delete_partition_panel.box.attribute = Choice_attribute;
    Delete_partition_panel.box.border_attribute = Choice_border_attribute;
    Delete_partition_panel.text_line = Delete_partition_text;

    /* Partition_type_panel */

    Partition_type_panel.control = CHOICE_CONTROL;
    Partition_type_panel.box.outline = Single_line_outline;
    Partition_type_panel.box.row = Partition_options_menu.box.row + 2;
    Partition_type_panel.box.column = Partition_options_menu.box.column + 2;
    Partition_type_panel.box.attribute = Choice_attribute;
    Partition_type_panel.box.border_attribute = Choice_border_attribute;
    Partition_type_panel.text_line = Partition_type_text;

    /* Partition_position_panel */

    Partition_position_panel.control = CHOICE_CONTROL;
    Partition_position_panel.box.outline = Single_line_outline;
    Partition_position_panel.box.row = Partition_type_panel.box.row + 2;
    Partition_position_panel.box.column = Partition_type_panel.box.column + 2;
    Partition_position_panel.box.attribute = Choice_attribute;
    Partition_position_panel.box.border_attribute = Choice_border_attribute;
    Partition_position_panel.text_line = Partition_position_text;

    /* Partition_name_panel */

    Partition_name_panel.control = ENTRY_CONTROL;
    Partition_name_panel.box.outline = Single_line_outline;
    Partition_name_panel.box.row = Partition_position_panel.box.row + 2;
    Partition_name_panel.box.column = Partition_position_panel.box.column + 2;
    Partition_name_panel.box.attribute = Entry_attribute;
    Partition_name_panel.box.border_attribute = Entry_border_attribute;
    Partition_name_panel.text_line = Partition_name_text;
    Partition_name_panel.input_line = Partition_name_input;

    /* Partition_size_panel */

    Partition_size_panel.control = ENTRY_CONTROL | NUMERIC_DATA_ONLY;
    Partition_size_panel.box.outline = Single_line_outline;
    Partition_size_panel.box.row = Partition_name_panel.box.row + 2;
    Partition_size_panel.box.column = Partition_name_panel.box.column + 2;
    Partition_size_panel.box.attribute = Entry_attribute;
    Partition_size_panel.box.border_attribute = Entry_border_attribute;
    Partition_size_panel.text_line = Partition_size_text;
    Partition_size_panel.input_line = Partition_size_input;

    /* Choose_a_disk_panel */

    Choose_a_disk_panel.control = MENU_CONTROL;
    Choose_a_disk_panel.box.outline = Single_line_outline;
    Choose_a_disk_panel.box.scroll_bar = Default_scroll_bar;
    Choose_a_disk_panel.box.number_of_rows = 0;
    Choose_a_disk_panel.box.row_length = 0;
    Choose_a_disk_panel.box.attribute = Menu_attribute;
    Choose_a_disk_panel.box.border_attribute = Menu_border_attribute;
    Choose_a_disk_panel.text_line = Choose_a_disk_text;
    CenterPanel ( &Choose_a_disk_panel );

}

