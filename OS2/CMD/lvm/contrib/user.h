/* user.h */
#ifndef _USER_H
#define _USER_H

#define SECTORS_PER_MEG 2048
#define LINE_LENGTH     180
#define PARTITION_NAME_WIDTH 20
#define PARTITION_SIZE_WIDTH 16
#define PARTITION_TYPE_WIDTH 10
#define PARTITION_STATUS_WIDTH 10

#define PARTITIONS_NAME_WIDTH 20
#define PARTITIONS_SIZE_WIDTH 16
#define PARTITIONS_NULL_WIDTH 16
#define PARTITION_NAME_OFFSET   1
#define PARTITION_STATUS_OFFSET (PARTITIONS_NAME_WIDTH + PARTITION_SIZE_WIDTH +PARTITION_TYPE_WIDTH + 3)
#define PARTITION_SIZE_OFFSET PARTITIONS_NAME_WIDTH +1 

#define VOLUME_NAME_WIDTH   20
#define VOLUME_DRIVE_WIDTH  6
#define VOLUME_TYPE_WIDTH   14
#define VOLUME_STATUS_WIDTH 14
#define VOLUME_FS_WIDTH     12
#define VOLUME_NAME_OFFSET  0
#define VOLUME_SIZE_WIDTH   6
#define LVM_VOLUME 3
#define COMPATIBILITY_VOLUME 4

#define PANEL_CONTROL   DEFAULT_CONTROL
#define CHOICE_CONTROL  DEFAULT_CONTROL
//SHOW_TEXT|SHOW_ATTRIBUTES
#define MENU_CONTROL    DEFAULT_CONTROL|SINGLE_LINE_BOX

#define WARNING_CONTROL SHOW_TEXT|HAS_BORDER|SHOW_OUTLINE|SHOW_ATTRIBUTES|SAVE_RESTORE|NOT_SIZED

#define ENTRY_CONTROL   DEFAULT_CONTROL
#define HELP_CONTROL    DEFAULT_CONTROL
#define MESSAGE_BAR_CONTROL SHOW_TEXT|SHOW_ATTRIBUTES
#define ERROR_CONTROL   SHOW_TEXT|HAS_BORDER|SHOW_OUTLINE|SHOW_ATTRIBUTES|SAVE_RESTORE|NOT_SIZED    
#define INFO_CONTROL    SHOW_TEXT|HAS_BORDER|SHOW_OUTLINE|SHOW_ATTRIBUTES|SAVE_RESTORE|NOT_SIZED


#define SCREEN_WIDTH 80
#define DISK_NUMBER_WIDTH 4
#define DISK_NAME_WIDTH 20
#define DISK_SIZE_WIDTH 12
#define DISK_FREE_WIDTH 10
#define DISK_LARGEST_WIDTH 10
#define FLOAT_DRIVE_LETTER_INDEX 26 /* ?? */

#define LEFT_DRIVE_LETTER_ACCENT_CHAR '>'
#define LEFT_DRIVE_LETTER_ACCENT_OFFSET 1 /* ??? */
#define RIGHT_DRIVE_LETTER_ACCENT_OFFSET 4 /* ??? */
#define RIGHT_DRIVE_LETTER_ACCENT_CHAR '<'

#define INSTALLABLE_VOLUME 'I'
#define NO_STATUS  0

typedef unsigned int bool, *pbool;
#define PUBLIC extern
#define PRIVATE static

#define loop for(;;)


extern
uint    Message_bar_attribute, // = MESSAGE_BAR_ATTRIBUTE,
        Panel_attribute, // = PANEL_ATTRIBUTE,
        Panel_border_attribute, // = PANEL_BORDER_ATTRIBUTE,
        Menu_attribute, // = MENU_ATTRIBUTE,
        Menu_border_attribute, // = MENU_BORDER_ATTRIBUTE,
        Entry_attribute, // = ENTRY_ATTRIBUTE,
        Entry_border_attribute, // = ENTRY_BORDER_ATTRIBUTE,
        Choice_attribute, // = CHOICE_ATTRIBUTE,
        Choice_border_attribute, // = CHOICE_BORDER_ATTRIBUTE,
        Info_attribute, // = INFO_ATTRIBUTE,
        Info_border_attribute, // = INFO_BORDER_ATTRIBUTE,
        Warning_attribute, // = WARNING_ATTRIBUTE,
        Warning_border_attribute, // = WARNING_BORDER_ATTRIBUTE,
        Error_attribute, // = ERROR_ATTRIBUTE,
        Error_border_attribute, // = ERROR_BORDER_ATTRIBUTE,
        Help_attribute, // = HELP_ATTRIBUTE,
        Help_border_attribute; // = HELP_BORDER_ATTRIBUTE;


extern
panel_t     Program_title_panel,
            Message_bar_panel,
            Engine_error_panel,
            Information_panel,
            None_startable_panel;

//#define  MESSAGE_BAR_PANEL(str) Message_bar_panel.text_line = &str; 
//                                MessageBarPanel(&Message_bar_panel);

extern void  MESSAGE_BAR_PANEL(char *str) ;

#endif //_USER_H
