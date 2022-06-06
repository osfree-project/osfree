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
 * Module: help.c
 */

/*
 * Change History:
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define INCL_DOSMISC
#include <os2.h>         //DosGetMessage + TRUE/FALSE
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "strings.h"
#include "help.h"
#include "lvm2.h"

#define MAX_HELP_TEXT_LINES                 128
#define HELP_MESSAGE_FILE                   "LVMH.MSG"
#define MAX_HELP_MESSAGE_SIZE               (1024 * 12)

/*
 * Volume_options_help_array
 * Must match Volume_options_text entries.
 *
 */

PUBLIC
uint    Volume_options_help_array [] = {
    NAVIGATE_FEATURES_HELP_MESSAGE,
    EXPAND_VOLUME_HELP_MESSAGE,
    DELETE_VOLUME_HELP_MESSAGE,
    HIDE_VOLUME_HELP_MESSAGE,
    CHANGE_VOLUME_NAME_HELP_MESSAGE,
    CHANGE_DRIVE_LETTER_HELP_MESSAGE,
    0,
    SET_INSTALLABLE_HELP_MESSAGE,
    SET_STARTABLE_HELP_MESSAGE,
    ADD_TO_BOOTMAN_HELP_MESSAGE,
    REMOVE_FROM_BOOTMAN_HELP_MESSAGE,
    0,
    CREATE_VOLUME_HELP_MESSAGE,
    0,
    INSTALL_BOOTMAN_HELP_MESSAGE,
    REMOVE_BOOTMAN_HELP_MESSAGE,
    SET_BOOTMAN_VALUES_HELP_MESSAGE,
    SET_BOOTMAN_STARTABLE_HELP_MESSAGE
};

PUBLIC
uint    Volume_options_help_array_size = sizeof ( Volume_options_help_array ) /
                                         sizeof ( uint );


/*
 * Bootable_help_array
 * Must match Bootable_text entries.
 *
 */

PUBLIC
uint    Bootable_help_array [] = {
    NOT_BOOTABLE_VOLUME_HELP_MESSAGE,
    BOOTABLE_VOLUME_HELP_MESSAGE
};

PUBLIC
uint    Bootable_help_array_size = sizeof ( Bootable_help_array ) /
                                   sizeof ( uint );


/*
 * Disk_options_help_array
 * Must match Disk_options_text entries.
 *
 */

PUBLIC
uint    Disk_options_help_array [] = {
    USE_EXISTING_MESSAGE,
    USE_FREE_SPACE_MESSAGE,
    0,
    DISK_NAME_MESSAGE
};

PUBLIC
uint    Disk_options_help_array_size = sizeof ( Disk_options_help_array ) /
                                       sizeof ( uint );


/*
 * Help_panel declarations
 *
 */

PRIVATE
panel_t     Help_panel;                   /* base level panels */



/*
 * help text_line array
 *
 */

PRIVATE
char        *Help_panel_text [ MAX_HELP_TEXT_LINES + 1 ];
                                                  /* *** could be dynamic *** */


/*
 * build the help text_line array from the help message file entry.
 * caller frees memory.
 * displays a default help panel if error.
 *
 */

PRIVATE
void *
get_help_message ( uint    message_number )
{
    register
    char    *line;
    char    *memory,
            *text_line;
    uint    rc,
            index,
            length;
    bool    no_help = TRUE;

    memory = malloc ( MAX_HELP_MESSAGE_SIZE );

    if ( memory ) {
        rc = DosGetMessage ( NULL, 0, memory, MAX_HELP_MESSAGE_SIZE - 1,
                             message_number, HELP_MESSAGE_FILE, (ULONG *)&length );

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
                        Help_panel_text [ index ] = text_line;
                        ++index;
                        text_line = line + 1;
                    }
                }
            }
            Help_panel_text [ index ] = NULL;
        }
    }

    if ( no_help ) {                                     /* no help available */
        Help_panel_text [ 0 ] = " ";
        Help_panel_text [ 1 ] = No_help_available;
        Help_panel_text [ 2 ] = " ";
        Help_panel_text [ 3 ] = 0;
    }

    return  memory;
}


/*
 * DoHelpPanel shows the panel and acts on input events
 *
 */

PUBLIC
void _System
DoHelpPanel ( register
              panel_t  *panel )
{
    uint    message_number,
            save_control;
    void    *memory;

    message_number = panel->help.default_message;

    if ( panel->help.array ) {
        if ( panel->choice < panel->help.array_size ) {
            message_number = panel->help.array [ panel->choice ];
        }
    }

    memory = get_help_message ( message_number );

    Help_panel.control |= NOT_SIZED;
    Help_panel.box.row_length = 0;
    SetPanelSize ( &Help_panel );
    Help_panel.box.column = SCREEN_WIDTH - min( Help_panel.box.row_length, SCREEN_WIDTH );

    save_control = Message_bar_panel.control;
    Message_bar_panel.control |= SAVE_RESTORE;

    MESSAGE_BAR_PANEL ( Help_message_line );
    ScrollingPanel ( &Help_panel );

    ErasePanel ( &Help_panel );
    ErasePanel ( &Message_bar_panel );
    Message_bar_panel.control = save_control;

    if ( memory ) {
        free ( memory );
    }

}


/*
 * ConstructHelpPanels sets the characteristics of the panels
 *
 */

PUBLIC
void
ConstructHelpPanels ( uint  top_row,
                      uint  bottom_row )
{
    /* Help_panel */

    Help_panel.control = HELP_CONTROL;
    Help_panel.box.outline = Single_line_outline;
    Help_panel.box.scroll_bar = Line_scroll_bar;
    Help_panel.box.row = top_row;
    Help_panel.box.column = 0;
    Help_panel.box.row_length = 0;
    Help_panel.box.number_of_rows = bottom_row - top_row + 1;
    Help_panel.box.attribute = Help_attribute;
    Help_panel.box.border_attribute = Help_border_attribute;
    Help_panel.text_line = Help_panel_text;

}
