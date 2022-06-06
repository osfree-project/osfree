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
 * Module: strings.c
 */

/*
 * Change History:
 *
 */


#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "panels.h"
#include "user.h"
#include "strings.h"
#include "lvm2.h"

char *SaveString(char *);
/*
 * General use characters
 *
 */


outline_t
    Blank_outline            = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },

    Tee_top_outline          = { 'Ã', 'À', '´', 'Ù', '³', 'Ä', 'Ã', '´' },

    Single_line_outline      = { 'Ú', 'À', '¿', 'Ù', '³', 'Ä', 'Ã', '´' };


scroll_bar_t    Default_scroll_bar = { 0x0/*''*/, 0x0/*''*/, '³', ' ' },

                Line_scroll_bar    = { 0x0/*''*/, 0x0/*''*/, '³', ' ' },

                Block_scroll_bar   = { 0x0/*''*/, 0x0/*''*/, '°', ' ' };

 char chars_less_than_greater_than[2] = {'<', '>'};

 char chars_right_arrow_left_arrow[2] = {0x0/*''*/, 0x0/*''*/};

/*
 * general use strings
 *
 */


char    Blank_line [] =
"                                                                              ";


char    *No_volumes_defined;


char    *No_disks_found;


char    *Free_space;


char    *None_string;


char    *Corrupt_disk;


char    *Unusable_disk;



/*
 * Volume panel
 *
 */

/*
 * Volume_options_menu strings
 * Must match Volume_options_help_array entries.
 *
 */


char    *Volume_options_text [] = {
    " ",       /* Expand the volume ", */
    " ",       /* Delete the volume ", */
    " ",       /* Hide the volume from OS/2 ", */
    " ",       /* Change the volume name ", */
    " ",       /* Change the volume drive letter ", */
    SEPARATOR_LINE,
    " ",       /* Set the volume installable ", */
    " ",       /* Set the volume startable ", */
    " ",       /* Add the volume to boot manager menu ", */
    " ",       /* Remove the volume from boot manager menu ", */
    SEPARATOR_LINE,
    " ",       /* Create a new volume ", */
    SEPARATOR_LINE,
    " ",       /* Install boot manager ", */
    " ",       /* Remove boot manager */
    " ",       /* Set Boot Manager startup values ", */
    " ",       /* Set Boot Manager startable", */
    0
};


char    *Volume_options_text2 [] = {
    " ",       /* Show Volume Tree */
    " ",       /* Expand the volume ", */
    " ",       /* Delete the volume ", */
    " ",       /* Hide the volume from OS/2 ", */
    " ",       /* Change the volume name ", */
    " ",       /* Change the volume drive letter ", */
    SEPARATOR_LINE,
    " ",       /* Set the volume installable ", */
    " ",       /* Set the volume startable ", */
    " ",       /* Add the volume to boot manager menu ", */
    " ",       /* Remove the volume from boot manager menu ", */
    SEPARATOR_LINE,
    " ",       /* Create a new volume ", */
    SEPARATOR_LINE,
    " ",       /* Install boot manager ", */
    " ",       /* Remove boot manager */
    " ",       /* Set Boot Manager startup values ", */
    " ",       /* Set Boot Manager startable", */
    0
};

/*
 * Volume_name_panel strings
 *
 */


char    *Volume_name_text [] = {
    " ",       /* Enter a name for the volume: ", */
    0
};


/*
 * Drive_letter_panel strings
 *
 */


char    *Drive_letter_text [] = {                       /* not translated */
    " ",           /* A: ", */
    " ",           /* B: ", */
    " ",           /* C: ", */
    " ",           /* D: ", */
    " ",           /* E: ", */
    " ",           /* F: ", */
    " ",           /* G: ", */
    " ",           /* H: ", */
    " ",           /* I: ", */
    " ",           /* J: ", */
    " ",           /* K: ", */
    " ",           /* L: ", */
    " ",           /* M: ", */
    " ",           /* N: ", */
    " ",           /* O: ", */
    " ",           /* P: ", */
    " ",           /* Q: ", */
    " ",           /* R: ", */
    " ",           /* S: ", */
    " ",           /* T: ", */
    " ",           /* U: ", */
    " ",           /* V: ", */
    " ",           /* W: ", */
    " ",           /* X: ", */
    " ",           /* Y: ", */
    " ",           /* Z: ", */
    " ",           /* *  ", */
    0
};

/* Floating drive letter string for the Drive_letter_text array. */
/* Can remove this when LVM.TXT can be updated to contain this letter. */
PRIVATE
char Float_drive_letter[] = { ' ', ' ', '*', ' ', ' '};

/*
 * Delete_volume_panel strings
 *
 */


char    *Show_volume_text [] = {
    " ",
    0
};


char    *Delete_volume_text [] = {
    " ",            /* Do not delete the volume */
    " ",            /* Delete the volume */
    0
};


/*
 * Bootable_panel strings
 *
 */


char    *Bootable_text [] = {
    " ",       /* Create a volume that is not bootable ", */
    " ",       /* Create a volume that is bootable ", */
    0
};


char    *Bootable_strings [] = {
    " ",                /* No ", */
    " ",                /* Yes ", */
    0
};


/*
 * Volume status strings
 *
 */


char    *Volume_status_strings [] = {
    " ",
    " ",            /* Bootable */
    " ",            /* Startable */
    " ",            /* Installable */
    0
};


/*
 * Volume_type_panel strings
 *
 */


char    *Volume_type_text [] = {
    " ",       /* Create a compatibility volume ", */
    " ",       /* Create an LVM volume ", */
    0
};


char    *Volume_type_strings [] = {
    " ",        /* Compatibility", */
    " ",        /* LVM", */
    0
};


/*
 * Volume_panel strings
 */
/*
|        (20)        | 3 |     (16)       |       (14)   |   (8)  |    (11)    |
 Logical Volume             Type           Status     File System   Size (MB)
*/



char    *Volume_header_text [] = {
    " ",
    0
};


/*
 * Partitions_panel strings
 */
/*
|        (20)        |   (12)     |       (17)      |        (20)        |     |
 Disk Partition         Size (MB)                    Disk Name
*/



char    *Partitions_header_text [] = {
    " ",
    0
};


/*
 * Partitions_options_menu strings
 *
 */


char    *Partitions_options_text [] = {
    " ",       /* Change the partition name ", */
    " ",       /* Change the disk name ", */
    0
};



/*
 * Disk panel
 *
 */

/*
 * Disk_panel strings
 */
/*
|(4) |        (20)        |   (12)     |      (21)           |       (16)     |
 Physical Disk               Size (MB)    Free Space:  Total          Largest
*/



char    *Disk_header_text [] = {
    " ",
    0
};



/*
 * Disk_options_menu strings
 *
 */


char    *Disk_options_text [] = {
    " ",       /* Use existing partition ", */
    " ",       /* Allocate from free space ", */
    SEPARATOR_LINE,
    " ",       /* Change the disk name ", */
    0
};


/*
 * Partition_panel strings
 */
/*
|        (20)        |   (12)     |   (10)   |     (12)   |         (20)       |
 Disk Partition         Size (MB)    Type     Status       Logical Volume
*/



char    *Partition_header_text [] = {
    " ",
    0
};


/*
 * Partition_options_menu strings
 */


char    *Partition_options_text [] = {
    " ",       /* Create a new partition ", */
    " ",       /* Delete the partition ", */
    " ",       /* Change the partition name ", */
    " ",       /* Change the volume name ", */
    0
};


/*
 * Delete_partition_panel strings
 *
 */


char    *Show_partition_text [] = {
    " ",
    0
};


char    *Delete_partition_text [] = {
    " ",            /* Do not delete the partition */
    " ",            /* Delete the partition */
    0
};


/*
 * Partition_type_panel strings
 */


char    *Partition_type_text [] = {
    " ",       /* Logical partition ", */
    " ",       /* Primary partition ", */
    0
};


char    *Partition_type_strings [] = {
    " ",           /* Logical", */
    " ",           /* Primary", */
    0
};


/*
 * Partition status strings
 */


char    *Partition_status_strings [] = {
    " ",
    " ",            /* In use", */
    " ",            /* Available", */
    0
};


/*
 * Partition position strings
 */


char    *Partition_position_text [] = {
    " ",          /* Create at the end of free space */
    " ",          /* Create at the beginning of free space */
    0
};


/*
 * Partition_size_panel strings
 */


char    *Partition_size_text [] = {
    " ",       /* Enter a size for the partition: ", */
    0
};


/*
 * Partition_name_panel strings
 */


char    *Partition_name_text [] = {
    " ",       /* Enter a name for the partition: ", */
    0
};


/*
 * Disk_name_panel strings
 */


char    *Disk_name_text [] = {
    " ",       /* Enter a name for the disk: ", */
    0
};


/*
 * Bootman_timeout_panel strings
 */


char    *Bootman_timeout_text [] = {
    " ",       /* Enter the time-out value in seconds: */
    0
};


/*
 * Bootman panel
 */

/*
 * Bootman_options_menu strings
 * Must match Bootman_options_help_array entries.
 */

char    *Bootman_options_text [] = {
    " ",            /* Default boot volume      (                    )  */
    " ",            /*  Timer active             (                    ) */
    " ",            /*  Time-out value           (                    ) */
    " ",            /*  Display mode             (                    ) */
    SEPARATOR_LINE,
    " ",            /*  Save the changes */
    0
};

/*
 * Bootman_options_menu replaceable strings
 */

char    *Timer_active_No;
char    *Timer_active_Yes;
char    *Display_mode_Normal;
char    *Display_mode_Advanced;


/*
 * Informational messages
 */


/*
 * Program_title_text
 */


char    *Program_title_text [] = {
" ",       /*            Logical Volume Management Tool - Logical View */
    0
};


char    *Program_title_physical;
         /*            Logical Volume Management Tool - Physical View */


/*
 * Engine_error_text strings
 */


char    *Engine_error_text [] = {
    " ",
    " ",            /* The requested action was not successful: ", */
    " ",
    " ",
    " ",
    0
};


/*
 * Information_panel strings
 */


char    *Information_text [] = {
    Blank_line,
    0
};


/*
 * Exit_options_menu strings
 */


char    *Exit_options_text [] = {
    " ",         /* Return to the program ",        */
    " ",         /* Discard the changes and exit ", */
    " ",         /* Save the changes and exit ",    */
    0
};


/*
 * Reboot_required_panel strings
 */


char    *Reboot_required_text [] = {
    " ",
    " ",    /*      Changes have been made that require a system reboot. */
    " ",
    " ",    /* Press and hold the Ctrl, Alt, and Del keys to reboot the system. */
    " ",
    0
};


char    *Install_reboot_required_text [] = {
    " ",
    " ",    /* The disk partitioning is complete. The system must be     */
    " ",    /* restarted so that the partitions can be recognized during */
    " ",    /* system installation.                                      */
    " ",
    " ",    /* Remove the diskette from drive A. */
    " ",
    " ",    /* Insert the installation diskette into drive A. */
    " ",
    " ",    /* Press and hold the Ctrl, Alt, and Del keys to reboot the system. */
    " ",
    0
};


/*
 * Min_install_size_panel strings
 */


char    *Min_install_size_text [] = {
    " ",
    " ",    /* A volume of the following minimum size must be set installable: */
    " ",
    " ",    /*            megabytes */
    " ",
    0
};


/*
 * Choose_a_disk_panel strings
 */


char    *Choose_a_disk_text [] = {
    " ",
    " ",         /* Choose a disk to be used to create the volume. */
    " ",
    " ",         /*     Instructions are displayed below. */
    " ",
    0
};


char    *Choose_a_disk_text_F6 [] = {
    " ",
    " ",         /* Choose a disk for the volume. Press F6 to complete */
    " ",         /*          creation of the volume.                   */
    " ",
    " ",         /*     Instructions are displayed below.              */
    " ",
    0
};


/*
 * Message_bar_panel strings
 */


char    *Message_bar_text [] = {
    " ",
    0
};


char    *Volume_aggregates_header_text [] = {
    " ",
    0
};
char    *Aggregate_features_header_text [] = {
    " ",
    0
};


char    *Aggregate_feature_options_text [] = {
    " ",       /* Feature status  */
    " ",       /* Feature control */
    " ",       /* Feature help    */
    0
};

/*
 * Empty_text strings for panels with no strings defined
 */


char    *Empty_text [] = {
    " ",
    0
};


/*
 * No_help_text strings for panels with no strings defined
 */


char    *No_help_available;



/*
 * Message line strings
 */

char    *Volume_message_line;
char    *Disk_message_line;
char    *Menu_options_message_line;
char    *Bootable_message_line;
char    *Volume_type_message_line;
char    *Drive_letter_message_line;
char    *Enter_name_message_line;
char    *Add_partition_message_line;
char    *Add_disk_message_line;
char    *Add_disk_message_line_F6;
char    *Partition_size_message_line;
char    *Partition_type_message_line;
char    *Warning_message_line;
char    *Help_message_line;
char    *Continue_message_line;
char    *Partition_position_message_line;
char    *Committing_changes_message_line;
char    *Volume_select_feature_title;
char    *Volume_order_feature_title;
char    *Volume_select_feature_message_line;
char    *Volume_order_feature_message_line;
char    *Volume_aggregates_message_line;
char    *Aggregate_features_message_line;


/*
 * Warning messages
 */

char    *Drive_change_text [] = {
    " ",
    " ",       /* Changing the drive letter assigned to a volume can ", */
    " ",       /* have unforeseen effects. As a minimum, the drive ",   */
    " ",       /* letter currently assigned to the volume must not ",   */
    " ",       /* appear in any path, dpath, or libpath statements ",   */
    " ",       /* in your config.sys file.  Furthermore, the drive ",   */
    " ",       /* letter should not be referenced in any of the .INI ", */
    " ",       /* files on the system. If these conditions are not ",   */
    " ",       /* met, your system may not boot properly anymore, ",    */
    " ",       /* or some programs may not run correctly anymore. ",    */
    " ",
    " ",       /* Press Enter to change the volume drive letter. ",     */
    " ",       /* Press Esc to cancel the change request. ",            */
    " ",
    0
};


char    *None_startable_text [] = {
    " ",
    " ",        /* Boot manager is not installed and a partition     */
    " ",        /* or volume has not been set startable. This means  */
    " ",        /* that the system will not be able to boot.         */
    " ",
    " ",        /* Press Enter to exit the program. */
    " ",        /* Press Esc to return to the program. */
    " ",
    0
};


char    *Disk_warning_text [] = {
    " ",
    " ",        /* Physical Disk (001): */
    " ",
    " ",        /* << corrupt message or unusable message here >> */
    " ",
    0
};




/*
 * Input string arrays - no messages needed
 */

char    *Volume_name_input [] = {
    " ",
    0
};

char    *Disk_name_input [] = {
    " ",
    0
};

char    *Partition_name_input [] = {
    " ",
    0
};

char    *Partition_size_input [] = {
    " ",
    0
};

char    *Bootman_timeout_input [] = {
    " ",
    0
};



/*
 * Error messages
 */

char    *Out_of_memory = "Not enough memory is available.";        /* keep here */
char    *Cannot_open_engine;
char    *Cannot_get_disk_data;
char    *Cannot_get_volume_data;
char    *LVM_engine_errors [] = {
    " ",           /* LVM engine error occurred ",       reusing this number */
    " ",           /* LVM engine out of memory ",                            */
    " ",           /* LVM engine io error ",                                 */
    " ",           /* LVM engine bad handle ",                               */
    " ",           /* LVM engine internal error ",                           */
    " ",           /* LVM engine already open ",                             */
    " ",           /* LVM engine not open ",                                 */
    " ",           /* LVM engine name too big ",                             */
    " ",           /* LVM engine operation not allowed ",                    */
    " ",           /* LVM engine drive open failure ",                       */
    " ",           /* LVM engine bad partition ",                            */
    " ",           /* LVM engine can not make primary partition ",           */
    " ",           /* LVM engine too many primary partitions ",              */
    " ",           /* LVM engine can not make logical drive ",               */
    " ",           /* LVM engine requested size too big ",                   */
    " ",           /* LVM engine 1024 cylinder limit ",                      */
    " ",           /* LVM engine partition alignment error ",                */
    " ",           /* LVM engine requested size too small ",                 */
    " ",           /* LVM engine not enough free space ",                    */
    " ",           /* LVM engine bad allocation algorithm ",                 */
    " ",           /* LVM engine duplicate name ",                           */
    " ",           /* LVM engine bad name                                    */
    " ",           /* LVM engine bad drive letter preference                 */
    " ",           /* LVM engine no drives found                             */
    " ",           /* LVM engine wrong volume type                           */
    " ",           /* LVM engine volume too small                            */
    " ",           /* LVM engine boot manager already installed              */
    " ",           /* LVM engine boot manager not found                      */
    " ",           /* LVM engine invalid parameter                           */
    " ",           /* LVM engine bad feature set                             */
    " ",           /* LVM engine too many partitions specified               */
    " ",           /* LVM engine LVM partitions not bootable                 */
    " ",           /* LVM engine partition already in use                    */
    " ",           /* LVM engine selected partition not bootable             */
    " ",           /* LVM engine:  volume not found                          */
    " ",           /* LVM engine:  drive not found                           */
    " ",           /* LVM engine:  partition not found                       */
    " ",           /* Internal error: too many features are active           */
    " ",           /* The selected partition is too small                    */
    " ",           /* The maximum number of partitions are already in use    */
    " ",           /* The read/write request is out of range                 */
    " ",           /* The selected partition is not startable                */
    " ",           /* The selected volume is not startable                   */
    " ",           /* The system was not able to extend the volume           */
    " ",           /* The system must be rebooted                            */
    0
};

uint    Max_LVM_engine_error = sizeof ( LVM_engine_errors ) /
                               sizeof ( char * ) - 2;


/*
 * Routines to build the strings from the message file
 */

/*
 * Message numbers for the message file
 */

/*
 * General use characters
 */

#define TEE_TOP_OUTLINE                 1
#define SINGLE_LINE_OUTLINE             2
#define DEFAULT_SCROLL_BAR              3
#define LINE_SCROLL_BAR                 4
#define BLOCK_SCROLL_BAR                5
#define LESS_THAN_GREATER_THAN          6
#define RIGHT_ARROW_LEFT_ARROW          7

/*
 * 2D arrays
 */

#define VOLUME_HEADER_TEXT              10
#define PARTITIONS_HEADER_TEXT          11
#define VOLUME_OPTIONS_TEXT             12
#define VOLUME_NAME_TEXT                13
#define DRIVE_LETTER_TEXT               14
#define BOOTABLE_TEXT                   15
#define VOLUME_TYPE_TEXT                16
#define DISK_HEADER_TEXT                17
#define PARTITION_HEADER_TEXT           18
#define DISK_OPTIONS_TEXT               19
#define PARTITION_OPTIONS_TEXT          20
#define PARTITION_TYPE_TEXT             21
#define PARTITION_SIZE_TEXT             22
#define PARTITION_NAME_TEXT             23
#define DISK_NAME_TEXT                  24

#define DRIVE_CHANGE_TEXT               26
#define BOOTABLE_STRINGS                27
#define VOLUME_TYPE_STRINGS             28

#define PARTITION_TYPE_STRINGS          49
#define PARTITION_STATUS_STRINGS        50
#define EXIT_OPTIONS_TEXT               51
#define LVM_ENGINE_ERRORS               56
#define ENGINE_ERROR_TEXT               57
#define BOOTMAN_TIMEOUT_TEXT            58
#define BOOTMAN_OPTIONS_TEXT            59
#define MIN_INSTALL_SIZE_TEXT           64
#define REBOOT_REQUIRED_TEXT            65
#define INSTALL_REBOOT_REQUIRED_TEXT    66
#define PROGRAM_TITLE_TEXT              67
#define CHOOSE_A_DISK_TEXT_F6           69
#define PARTITIONS_OPTIONS_TEXT         70
#define CHOOSE_A_DISK_TEXT              71
#define PARTITION_POSITION_TEXT         73
#define DELETE_VOLUME_TEXT              75
#define SHOW_VOLUME_TEXT                76
#define DELETE_PARTITION_TEXT           77
#define SHOW_PARTITION_TEXT             SHOW_VOLUME_TEXT
#define VOLUME_STATUS_STRINGS           78
#define NONE_STARTABLE_TEXT             79
#define DISK_WARNING_TEXT               81

/*
 * 1D arrays
 */

#define VOLUME_MESSAGE_LINE             30
#define DISK_MESSAGE_LINE               31
#define MENU_OPTIONS_MESSAGE_LINE       32
#define BOOTABLE_MESSAGE_LINE           33
#define VOLUME_TYPE_MESSAGE_LINE        34
#define DRIVE_LETTER_MESSAGE_LINE       35
#define ENTER_NAME_MESSAGE_LINE         36
#define ADD_PARTITION_MESSAGE_LINE      37
#define ADD_DISK_MESSAGE_LINE           38
#define ADD_DISK_MESSAGE_LINE_F6        39
#define PARTITION_SIZE_MESSAGE_LINE     40
#define PARTITION_TYPE_MESSAGE_LINE     41
#define WARNING_MESSAGE_LINE            42
#define HELP_MESSAGE_LINE               43

#define NO_HELP_AVAILABLE               44
#define OUT_OF_MEMORY                   45
#define NO_VOLUMES_DEFINED              46
#define NO_DISKS_FOUND                  47
#define FREE_SPACE                      48

#define CONTINUE_MESSAGE_LINE           52
#define CANNOT_OPEN_ENGINE              53
#define CANNOT_GET_DISK_DATA            54
#define CANNOT_GET_VOLUME_DATA          55

#define TIMER_ACTIVE_NO                 60
#define TIMER_ACTIVE_YES                61
#define DISPLAY_MODE_NORMAL             62
#define DISPLAY_MODE_ADVANCED           63
#define PROGRAM_TITLE_PHYSICAL          68
#define NONE_STRING                     72
#define PARTITION_POSITION_MESSAGE_LINE 74
#define COMMITTING_CHANGES_MESSAGE_LINE 80
#define CORRUPT_DISK                    82
#define UNUSABLE_DISK                   83
#define VOLUME_SELECT_FEATURE_TITLE     84
#define VOLUME_ORDER_FEATURE_TITLE      85
#define VOLUME_SELECT_FEATURE_MESSAGE_LINE   86
#define VOLUME_ORDER_FEATURE_MESSAGE_LINE    87
#define VOLUME_AGGREGATES_HEADER_TEXT        88
#define AGGREGATE_FEATURES_HEADER_TEXT       89
#define VOLUME_AGGREGATES_MESSAGE_LINE       90
#define AGGREGATE_FEATURES_MESSAGE_LINE      91
#define AGGREGATE_FEATURE_OPTIONS_TEXT       92
/* message 93 & 94 are for Drive Linking VIO panels */
#define VOLUME_OPTIONS_TEXT2                 95   /*pcr1442 */


#define STRING_MESSAGE_FILE             "LVM.MSG"
#define MAX_STRING_MESSAGE_SIZE         (1024 * 12)

uint _System DosGetMessage ( char **, uint, char *, uint, uint, char *, uint *);


typedef struct _line_message {                          /* 1D arrays */
    char    **line;
    uint    number;
} line_message_t;

typedef struct _text_message {                          /* 2D arrays */
    char    **text_line;
    uint    number;
} text_message_t;


PRIVATE
text_message_t  text_message [] = {

    Volume_header_text,             VOLUME_HEADER_TEXT,
    Partitions_header_text,         PARTITIONS_HEADER_TEXT,
    Volume_options_text,            VOLUME_OPTIONS_TEXT,
    Volume_name_text,               VOLUME_NAME_TEXT,
    Drive_letter_text,              DRIVE_LETTER_TEXT,      /* not translated */
    Bootable_text,                  BOOTABLE_TEXT,
    Volume_type_text,               VOLUME_TYPE_TEXT,
    Disk_header_text,               DISK_HEADER_TEXT,
    Partition_header_text,          PARTITION_HEADER_TEXT,
    Disk_options_text,              DISK_OPTIONS_TEXT,
    Partition_options_text,         PARTITION_OPTIONS_TEXT,
    Partition_type_text,            PARTITION_TYPE_TEXT,
    Partition_size_text,            PARTITION_SIZE_TEXT,
    Partition_name_text,            PARTITION_NAME_TEXT,
    Disk_name_text,                 DISK_NAME_TEXT,
    Drive_change_text,              DRIVE_CHANGE_TEXT,
    Bootable_strings,               BOOTABLE_STRINGS,
    Volume_type_strings,            VOLUME_TYPE_STRINGS,
    Partition_type_strings,         PARTITION_TYPE_STRINGS,
    Partition_status_strings,       PARTITION_STATUS_STRINGS,
    Exit_options_text,              EXIT_OPTIONS_TEXT,
    LVM_engine_errors,              LVM_ENGINE_ERRORS,
    Engine_error_text,              ENGINE_ERROR_TEXT,
    Bootman_timeout_text,           BOOTMAN_TIMEOUT_TEXT,
    Bootman_options_text,           BOOTMAN_OPTIONS_TEXT,
    Min_install_size_text,          MIN_INSTALL_SIZE_TEXT,
    Reboot_required_text,           REBOOT_REQUIRED_TEXT,
    Install_reboot_required_text,   INSTALL_REBOOT_REQUIRED_TEXT,
    Program_title_text,             PROGRAM_TITLE_TEXT,
    Choose_a_disk_text_F6,          CHOOSE_A_DISK_TEXT_F6,
    Partitions_options_text,        PARTITIONS_OPTIONS_TEXT,
    Choose_a_disk_text,             CHOOSE_A_DISK_TEXT,
    Partition_position_text,        PARTITION_POSITION_TEXT,
    Delete_volume_text,             DELETE_VOLUME_TEXT,
    Show_volume_text,               SHOW_VOLUME_TEXT,
    Delete_partition_text,          DELETE_PARTITION_TEXT,
    Show_partition_text,            SHOW_PARTITION_TEXT,
    Volume_status_strings,          VOLUME_STATUS_STRINGS,
    None_startable_text,            NONE_STARTABLE_TEXT,
    Disk_warning_text,              DISK_WARNING_TEXT,
    Aggregate_feature_options_text,    AGGREGATE_FEATURE_OPTIONS_TEXT,
    Volume_options_text2,            VOLUME_OPTIONS_TEXT2,
    NULL, 0
};


PRIVATE
line_message_t  line_message [] = {

    &No_help_available,                 NO_HELP_AVAILABLE,
    &Volume_message_line,               VOLUME_MESSAGE_LINE,
    &Disk_message_line,                 DISK_MESSAGE_LINE,
    &Menu_options_message_line,         MENU_OPTIONS_MESSAGE_LINE,
    &Bootable_message_line,             BOOTABLE_MESSAGE_LINE,
    &Volume_type_message_line,          VOLUME_TYPE_MESSAGE_LINE,
    &Drive_letter_message_line,         DRIVE_LETTER_MESSAGE_LINE,
    &Enter_name_message_line,           ENTER_NAME_MESSAGE_LINE,
    &Add_partition_message_line,        ADD_PARTITION_MESSAGE_LINE,
    &Add_disk_message_line,             ADD_DISK_MESSAGE_LINE,
    &Add_disk_message_line_F6,          ADD_DISK_MESSAGE_LINE_F6,
    &Partition_size_message_line,       PARTITION_SIZE_MESSAGE_LINE,
    &Partition_type_message_line,       PARTITION_TYPE_MESSAGE_LINE,
    &Warning_message_line,              WARNING_MESSAGE_LINE,
    &Help_message_line,                 HELP_MESSAGE_LINE,
    &No_volumes_defined,                NO_VOLUMES_DEFINED,
    &No_disks_found,                    NO_DISKS_FOUND,
    &Free_space,                        FREE_SPACE,
    &Continue_message_line,             CONTINUE_MESSAGE_LINE,
    &Cannot_open_engine,                CANNOT_OPEN_ENGINE,
    &Cannot_get_disk_data,              CANNOT_GET_DISK_DATA,
    &Cannot_get_volume_data,            CANNOT_GET_VOLUME_DATA,
    &Timer_active_No,                   TIMER_ACTIVE_NO,
    &Timer_active_Yes,                  TIMER_ACTIVE_YES,
    &Display_mode_Normal,               DISPLAY_MODE_NORMAL,
    &Display_mode_Advanced,             DISPLAY_MODE_ADVANCED,
    &Program_title_physical,            PROGRAM_TITLE_PHYSICAL,
    &None_string,                       NONE_STRING,
    &Partition_position_message_line,   PARTITION_POSITION_MESSAGE_LINE,
    &Committing_changes_message_line,   COMMITTING_CHANGES_MESSAGE_LINE,
    &Corrupt_disk,                      CORRUPT_DISK,
    &Unusable_disk,                     UNUSABLE_DISK,
    &Volume_select_feature_title,       VOLUME_SELECT_FEATURE_TITLE,
    &Volume_order_feature_title,        VOLUME_ORDER_FEATURE_TITLE,
    &Volume_select_feature_message_line,VOLUME_SELECT_FEATURE_MESSAGE_LINE,
    &Volume_order_feature_message_line, VOLUME_ORDER_FEATURE_MESSAGE_LINE,
    &Volume_aggregates_header_text,     VOLUME_AGGREGATES_HEADER_TEXT,
    &Aggregate_features_header_text,    AGGREGATE_FEATURES_HEADER_TEXT,
    &Volume_aggregates_message_line,    VOLUME_AGGREGATES_MESSAGE_LINE,
    &Aggregate_features_message_line,   AGGREGATE_FEATURES_MESSAGE_LINE,
    NULL, 0
};


/*
 * Build the text_line array from the message file entry.
 * Sets the size of the array of strings by the size of the declaration
 * in this file. This must match the number of lines in the message file.
 */

PRIVATE
uint
get_text_message ( char    *memory )
{
    register
    text_message_t  *message = text_message;
    register
    char    *line;
    char    **array;
    char    *text_line;
    uint    rc = -1,
            index,
            array_size,
            length;

    for ( ;  message->text_line != NULL;  ++message ) {
        array = message->text_line;
        for ( array_size = 0;  array [array_size] != 0;  ++array_size ) {
            /* sets the size of the array of strings, not including the end 0 */
        }
        rc = DosGetMessage ( NULL, 0, memory, MAX_STRING_MESSAGE_SIZE - 1,
                             message->number, STRING_MESSAGE_FILE, &length );
        if ( rc == 0 ) {
            memory [ length ] = '\0';
            text_line = memory + 2;                         /* skip first \r\n */
            index = 0;
            for ( line = text_line;  *line;  ++line ) {
                if ( *line == '\r' ) {
                    *line = ' ';
                } else if ( *line == '\n' ) {
                    *line = '\0';
                    if ( index < array_size ) {
                        if ( text_line [1] == '\\'  &&  text_line [2] == '\\' ){
                            array [ index ] = SEPARATOR_LINE;
                        } else {
                            array [ index ] = SaveString ( text_line );
                        }
                        ++index;
                        text_line = line + 1;                   /* skip over \0 */
                    }
                }
            }
            /* If there are less lines in the message file than expected,
             * set remaining text lines to NULL.  Needed when NLS changes
             * are made for English, but not yet available in the translated
             * message files.
             */
            while (index < array_size) {
               array[index++] = NULL;
            }

        } else {
            break;                                             /* exit if error */
        }
    }

    /* Text string post-processing.
     */

    /* Set up floating drive letter. */
    /* Can remove this when LVM.TXT can be updated to contain this letter. */

    Drive_letter_text[ FLOAT_DRIVE_LETTER_INDEX ] = Float_drive_letter;

    return  rc;
}


/*
 * build the strings from the message file entries.
 */

PRIVATE
uint
get_line_message ( char    *memory )
{
    register
    line_message_t  *message = line_message;
    register
    char    *line;
    char    *text_line;
    char    **string;
    uint    rc = -1,
            length;

    for ( ;  message->line != NULL;  ++message ) {
        string = message->line;

        rc = DosGetMessage ( NULL, 0, memory, MAX_STRING_MESSAGE_SIZE - 1,
                             message->number, STRING_MESSAGE_FILE, &length );
        if ( rc == 0 ) {
            memory [ length ] = '\0';
            text_line = memory + 2;                         /* skip first \r\n */
            for ( line = text_line;  *line;  ++line ) {
                if ( *line == '\r' ) {
                    *line = ' ';
                } else if ( *line == '\n' ) {
                    *line = '\0';
                    *string = SaveString ( text_line );
                }
            }
        } else {
            break;                                             /* exit if error */
        }
    }

    return  rc;
}


/*
 * build the outlines and scroll bars from the message file entries.
 */

PRIVATE
void
get_char_message ( char    *memory,
                   uint    message_number,
                   char    *array,
                   uint    array_length )
{
    register
    char    *line;
    uint    rc,
            index = 0,
            length;

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


/*
 * ConstructTextStrings is the  constructor for strings.c
 * Reads the strings in from the message file, and saves them.
 * Returns 0 if no errors, Quits otherwise.
 *
 */


void
ConstructTextStrings ( void )
{
    uint    rc;
    char    *memory,
            *line;


    /* allocate input strings */

    line = AllocateOrQuit ( VOLUME_NAME_WIDTH + 1, sizeof (char) );
    strncpy ( line, Blank_line, VOLUME_NAME_WIDTH );
    Volume_name_input [ 0 ] = line;

    line = AllocateOrQuit ( DISK_NAME_WIDTH + 1, sizeof (char) );
    strncpy ( line, Blank_line, DISK_NAME_WIDTH );
    Disk_name_input [ 0 ] = line;

    line = AllocateOrQuit ( PARTITION_NAME_WIDTH + 1, sizeof (char) );
    strncpy ( line, Blank_line, PARTITION_NAME_WIDTH );
    Partition_name_input [ 0 ] = line;

    line = AllocateOrQuit ( PARTITION_SIZE_WIDTH + 1, sizeof (char) );
    strncpy ( line, Blank_line, PARTITION_SIZE_WIDTH );
    Partition_size_input [ 0 ] = line;

    line = AllocateOrQuit ( 3 + 1, sizeof (char) );     /* TIMEOUT_WIDTH *** */
    strncpy ( line, Blank_line, 3 );
    Bootman_timeout_input [ 0 ] = line;

    /* get messages */

    memory = malloc ( MAX_STRING_MESSAGE_SIZE );

    if ( memory ) {

        get_char_message ( memory, TEE_TOP_OUTLINE,
                           (char *) &Tee_top_outline,
                           sizeof ( outline_t ) );

        get_char_message ( memory, SINGLE_LINE_OUTLINE,
                           (char *) &Single_line_outline,
                           sizeof ( outline_t ) );

        get_char_message ( memory, DEFAULT_SCROLL_BAR,
                           (char *) &Default_scroll_bar,
                           sizeof ( scroll_bar_t ) );

        get_char_message ( memory, LINE_SCROLL_BAR,
                           (char *) &Line_scroll_bar,
                           sizeof ( scroll_bar_t ) );

        get_char_message ( memory, BLOCK_SCROLL_BAR,
                           (char *) &Block_scroll_bar,
                           sizeof ( scroll_bar_t ) );

        get_char_message ( memory, LESS_THAN_GREATER_THAN,
                           chars_less_than_greater_than,
                           2);
        get_char_message ( memory, RIGHT_ARROW_LEFT_ARROW,
                           chars_right_arrow_left_arrow,
                           2);

        rc = get_text_message ( memory );
        if ( rc ) {
            Quit ( "Cannot get messages." );
        }
        rc = get_line_message ( memory );
        if ( rc ) {
            Quit ( "Cannot get messages." );
        }

        free ( memory );

    } else {
        Quit ( Out_of_memory );
    }

}

