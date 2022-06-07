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
 * Module: help.h
 */

/*
 * Change History:
 *
 */


/*
 * Help definitions
 *
 */

#define HELP_MESSAGE_BASE                   5000

// --------------------------------------------------------------------
#define VOLUME_HELP_MESSAGE                     (HELP_MESSAGE_BASE + 1)

#define EXPAND_VOLUME_HELP_MESSAGE              (HELP_MESSAGE_BASE + 2)
#define DELETE_VOLUME_HELP_MESSAGE              (HELP_MESSAGE_BASE + 3)
#define HIDE_VOLUME_HELP_MESSAGE                (HELP_MESSAGE_BASE + 4)
#define CHANGE_VOLUME_NAME_HELP_MESSAGE         (HELP_MESSAGE_BASE + 5)
#define CHANGE_DRIVE_LETTER_HELP_MESSAGE        (HELP_MESSAGE_BASE + 6)
#define SET_INSTALLABLE_HELP_MESSAGE            (HELP_MESSAGE_BASE + 7)
#define ADD_TO_BOOTMAN_HELP_MESSAGE             (HELP_MESSAGE_BASE + 8)
#define REMOVE_FROM_BOOTMAN_HELP_MESSAGE        (HELP_MESSAGE_BASE + 9)
#define CREATE_VOLUME_HELP_MESSAGE              (HELP_MESSAGE_BASE + 10)
#define INSTALL_BOOTMAN_HELP_MESSAGE            (HELP_MESSAGE_BASE + 11)
#define SET_BOOTMAN_VALUES_HELP_MESSAGE         (HELP_MESSAGE_BASE + 12)
#define REMOVE_BOOTMAN_HELP_MESSAGE             (HELP_MESSAGE_BASE + 13)
#define SET_STARTABLE_HELP_MESSAGE              (HELP_MESSAGE_BASE + 14)
#define NOT_BOOTABLE_VOLUME_HELP_MESSAGE        (HELP_MESSAGE_BASE + 15)
#define BOOTABLE_VOLUME_HELP_MESSAGE            (HELP_MESSAGE_BASE + 16)


// --------------------------------------------------------------------

#define DISK_HELP_MESSAGE                       (HELP_MESSAGE_BASE + 17)

#define USE_EXISTING_MESSAGE                    (HELP_MESSAGE_BASE + 18)
#define USE_FREE_SPACE_MESSAGE                  (HELP_MESSAGE_BASE + 19)
#define DISK_NAME_MESSAGE                       (HELP_MESSAGE_BASE + 20)

#define DRIVE_LETTER_HELP_MESSAGE               (HELP_MESSAGE_BASE + 21)
#define SET_BOOTMAN_STARTABLE_HELP_MESSAGE      (HELP_MESSAGE_BASE + 22)
#define SELECT_FEATURES_HELP_MESSAGE            (HELP_MESSAGE_BASE + 23) //pcr1442
#define ORDER_FEATURES_HELP_MESSAGE             (HELP_MESSAGE_BASE + 24) //pcr1442
#define NAVIGATE_FEATURES_HELP_MESSAGE          (HELP_MESSAGE_BASE + 25) //pcr1442



// ====================================================================
// publics from help.c
// ====================================================================

// --------------------------------------------------------------------
// Volume_options_help_array
// --------------------------------------------------------------------

extern
uint    Volume_options_help_array [];

extern
uint    Volume_options_help_array_size;


// --------------------------------------------------------------------
// Bootable_help_array
// --------------------------------------------------------------------

extern
uint    Bootable_help_array [] ;

extern
uint    Bootable_help_array_size ;


// --------------------------------------------------------------------
// Disk_options_help_array
// --------------------------------------------------------------------

extern
uint    Disk_options_help_array [] ;

extern
uint    Disk_options_help_array_size ;


// --------------------------------------------------------------------
// DoHelpPanel shows the panel and acts on input events
// --------------------------------------------------------------------

PUBLIC
void _System
DoHelpPanel ( register
              panel_t  *panel );

// --------------------------------------------------------------------
// ConstructHelpPanels sets the characteristics of the panels
// --------------------------------------------------------------------

PUBLIC
void
ConstructHelpPanels ( uint  top_row,
                      uint  bottom_row );
