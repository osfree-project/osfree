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
 * Module: strings.h
 */

/*
 * Change History:
 *
 */


/*
 * General use characters
 */

extern outline_t Blank_outline ;
extern outline_t Tee_top_outline ;
extern outline_t Single_line_outline ;
extern scroll_bar_t Default_scroll_bar ;
extern scroll_bar_t Line_scroll_bar ;
extern scroll_bar_t Block_scroll_bar ;
extern char chars_less_than_greater_than[];
extern char chars_right_arrow_left_arrow[];


/*
 * General use strings
 */

extern char    Blank_line [] ;
extern char    *No_volumes_defined;
extern char    *No_disks_found;
extern char    *Free_space ;
extern char    *None_string;
extern char    *Corrupt_disk;
extern char    *Unusable_disk;



/*
 * Volume_panel strings
 */

extern char    *Volume_header_text [] ;
extern char    *Partitions_header_text [] ;
extern char    *Partitions_options_text [] ;

extern char    *Volume_options_text [] ;
extern char    *Volume_options_text2 [] ;

extern char    *Volume_name_text [] ;
extern char    *Drive_letter_text [] ;
extern char    *Show_volume_text [] ;
extern char    *Delete_volume_text [] ;
extern char    *Bootable_text [] ;
extern char    *Bootable_strings [] ;
extern char    *Volume_status_strings [] ;
extern char    *Volume_type_text [] ;
extern char    *Volume_type_strings [] ;
extern char    *Volume_aggregates_header_text[];
extern char    *Aggregate_features_header_text[];
extern char    *Aggregate_feature_options_text[];
extern char    *Volume_select_feature_title;
extern char    *Volume_order_feature_title;
extern char    *Volume_select_feature_message_line;
extern char    *Volume_order_feature_message_line;
extern char    *Volume_aggregates_message_line;
extern char    *Aggregate_features_message_line;


/*
 * Disk_panel strings
 */

extern char    *Disk_header_text [] ;
extern char    *Partition_header_text [] ;

extern char    *Disk_options_text [] ;

extern char    *Partition_options_text [] ;
extern char    *Show_partition_text [] ;
extern char    *Delete_partition_text [] ;
extern char    *Partition_type_text [] ;
extern char    *Partition_type_strings [] ;
extern char    *Partition_status_strings [] ;
extern char    *Partition_position_text [] ;
extern char    *Partition_size_text [] ;
extern char    *Partition_name_text [] ;
extern char    *Disk_name_text [] ;
extern char    *Bootman_timeout_text [] ;


/*
 * Bootman panel
 */

extern char    *Bootman_options_text [] ;
extern char    *Timer_active_No;
extern char    *Timer_active_Yes;
extern char    *Display_mode_Normal;
extern char    *Display_mode_Advanced;


/*
 * Informational messages
 */

extern char    *Program_title_text [] ;
extern char    *Program_title_physical;
extern char    *Engine_error_text [] ;
extern char    *Information_text [] ;
extern char    *Exit_options_text [] ;
extern char    *Reboot_required_text [] ;
extern char    *Install_reboot_required_text [] ;
extern char    *Min_install_size_text [] ;
extern char    *Choose_a_disk_text_F6 [] ;
extern char    *Choose_a_disk_text [] ;
extern char    *Message_bar_text [] ;
extern char    *Empty_text [] ;

extern char    *No_help_available ;

extern char    *Volume_message_line ;
extern char    *Disk_message_line ;
extern char    *Menu_options_message_line ;
extern char    *Bootable_message_line ;
extern char    *Volume_type_message_line ;
extern char    *Drive_letter_message_line ;
extern char    *Enter_name_message_line ;
extern char    *Add_partition_message_line ;
extern char    *Add_disk_message_line ;
extern char    *Add_disk_message_line_F6 ;
extern char    *Partition_size_message_line ;
extern char    *Partition_type_message_line ;
extern char    *Warning_message_line ;
extern char    *Help_message_line ;
extern char    *Continue_message_line ;
extern char    *Partition_position_message_line;
extern char    *Committing_changes_message_line;


/*
 * Warning messages
 */

extern char    *Drive_change_text [] ;
extern char    *None_startable_text [] ;
extern char    *Disk_warning_text [] ;


/*
 * Input string arrays - no message
 */

extern char    *Volume_name_input [] ;
extern char    *Disk_name_input [] ;
extern char    *Partition_name_input [] ;
extern char    *Partition_size_input [] ;
extern char    *Bootman_timeout_input [] ;

/*
 * Error messages
 */

extern char    *Out_of_memory ;
extern char    *Cannot_open_engine ;
extern char    *Cannot_get_disk_data ;
extern char    *Cannot_get_volume_data ;

extern char    *LVM_engine_errors [] ;
extern uint    Max_LVM_engine_error ;               /* number of errors */


/*
 * Routines to build the strings from the message file
 */

/*
 * ConstructTextStrings is the public constructor for strings.c
 * Returns 0 if no errors.
 */

// PUBLIC
void
ConstructTextStrings ( void );

