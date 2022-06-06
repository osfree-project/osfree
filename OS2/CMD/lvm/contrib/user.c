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
 * Module: user.c
 */

/*
 * Change History:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <signal.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "strings.h"
#include "help.h"
#include "LVM_Interface.h"
#include "lvm2.h"

#define PCR1442 1

/*
 * panel declarations
 */

PRIVATE
panel_t     Exit_options_menu,
            Reboot_required_panel,
            Min_install_size_panel;

//PUBLIC
panel_t     Program_title_panel,
            Message_bar_panel,
            Engine_error_panel,
            Information_panel,
            None_startable_panel;


/*
 * attribute declarations
 */


//PUBLIC
uint    Message_bar_attribute = MESSAGE_BAR_ATTRIBUTE,
        Panel_attribute = PANEL_ATTRIBUTE,
        Panel_border_attribute = PANEL_BORDER_ATTRIBUTE,
        Menu_attribute = MENU_ATTRIBUTE,
        Menu_border_attribute = MENU_BORDER_ATTRIBUTE,
        Entry_attribute = ENTRY_ATTRIBUTE,
        Entry_border_attribute = ENTRY_BORDER_ATTRIBUTE,
        Choice_attribute = CHOICE_ATTRIBUTE,
        Choice_border_attribute = CHOICE_BORDER_ATTRIBUTE,
        Info_attribute = INFO_ATTRIBUTE,
        Info_border_attribute = INFO_BORDER_ATTRIBUTE,
        Warning_attribute = WARNING_ATTRIBUTE,
        Warning_border_attribute = WARNING_BORDER_ATTRIBUTE,
        Error_attribute = ERROR_ATTRIBUTE,
        Error_border_attribute = ERROR_BORDER_ATTRIBUTE,
        Help_attribute = HELP_ATTRIBUTE,
        Help_border_attribute = HELP_BORDER_ATTRIBUTE;




/*
 * InitializeUserInterface initializes the panels subsystem first.
 * Then it constructs the LVM user interface panels.
 */

PUBLIC
void
InitializeUserInterface ( uint default_attribute,
                          uint exit_attribute )
{
    uint    total_rows,
            top_row,
            attribute_mask = ~(HIGHLIGHTED_FG | HIGHLIGHTED_BG),
            rc;
    CARDINAL32      error;

    rc = InitializePanels ( default_attribute, exit_attribute );

    if ( rc ) {
        Message_bar_attribute &= attribute_mask;
        Panel_attribute &= attribute_mask;
        Menu_attribute &= attribute_mask;
        Entry_attribute &= attribute_mask;
        Choice_attribute &= attribute_mask;
        Info_attribute &= attribute_mask;
        Warning_attribute &= attribute_mask;
        Warning_border_attribute &= attribute_mask;
        Error_attribute &= attribute_mask;
        Error_border_attribute &= attribute_mask;
        Help_attribute &= attribute_mask;
        Help_border_attribute &= attribute_mask;
    }

    total_rows = GetScreenRows ();
    top_row = 1;

    ConstructTextStrings ();                           /* do this first */

#ifdef PCR1442
    Open_LVM_Engine2 ( FALSE, VIO_Interface, &error);
#else
    Open_LVM_Engine ( FALSE, &error );
#endif

    if ( error ) {
        Quit ( Cannot_open_engine );
    }

    /* construct the global panels */

    /* Program_title_panel */

    Program_title_panel.control = MESSAGE_BAR_CONTROL;
    Program_title_panel.box.row = 0;
    Program_title_panel.box.column = 0;
    Program_title_panel.box.row_length = SCREEN_WIDTH;
    Program_title_panel.box.number_of_rows = 1;
    Program_title_panel.box.attribute = Message_bar_attribute;
    Program_title_panel.text_line = Program_title_text;

    /* Message_bar_panel */

    Message_bar_panel.control = MESSAGE_BAR_CONTROL;
    Message_bar_panel.box.row = total_rows - 1;
    Message_bar_panel.box.column = 0;
    Message_bar_panel.box.row_length = SCREEN_WIDTH;
    Message_bar_panel.box.number_of_rows = 1;
    Message_bar_panel.box.attribute = Message_bar_attribute;
    Message_bar_panel.text_line = Message_bar_text;

    /* Exit_options_menu */

    Exit_options_menu.control = MENU_CONTROL;
    Exit_options_menu.box.outline = Single_line_outline;
    Exit_options_menu.box.scroll_bar = Default_scroll_bar;
    Exit_options_menu.box.number_of_rows = 0;
    Exit_options_menu.box.row = 0;
    Exit_options_menu.box.column = 0;
    Exit_options_menu.box.row_length = 0;
    Exit_options_menu.box.attribute = Menu_attribute;
    Exit_options_menu.box.border_attribute = Menu_border_attribute;
    Exit_options_menu.text_line = Exit_options_text;
    CenterPanel ( &Exit_options_menu );

    /* Reboot_required_panel */

    Reboot_required_panel.control = MENU_CONTROL;
    Reboot_required_panel.box.outline = Single_line_outline;
    Reboot_required_panel.box.scroll_bar = Default_scroll_bar;
    Reboot_required_panel.box.number_of_rows = 0;
    Reboot_required_panel.box.row_length = 0;
    Reboot_required_panel.box.attribute = Menu_attribute;
    Reboot_required_panel.box.border_attribute = Menu_border_attribute;
    Reboot_required_panel.text_line = Reboot_required_text;

    /* Min_install_size_panel */

    Min_install_size_panel.control = MENU_CONTROL;
    Min_install_size_panel.box.outline = Single_line_outline;
    Min_install_size_panel.box.scroll_bar = Default_scroll_bar;
    Min_install_size_panel.box.number_of_rows = 0;
    Min_install_size_panel.box.row_length = 0;
    Min_install_size_panel.box.attribute = Menu_attribute;
    Min_install_size_panel.box.border_attribute = Menu_border_attribute;
    Min_install_size_panel.text_line = Min_install_size_text;
    CenterPanel ( &Min_install_size_panel );

    /* Engine_error_panel */

    Engine_error_panel.control = ERROR_CONTROL;
    Engine_error_panel.box.outline = Single_line_outline;
    Engine_error_panel.box.scroll_bar = Default_scroll_bar;
    Engine_error_panel.box.number_of_rows = 7;
    Engine_error_panel.box.row = total_rows / 2;
    Engine_error_panel.box.column = 10;
    Engine_error_panel.box.row_length = 0;
    Engine_error_panel.box.attribute = Error_attribute;
    Engine_error_panel.box.border_attribute = Error_border_attribute;
    Engine_error_panel.text_line = Engine_error_text;

    /* Information_panel */

    Information_panel.control = INFO_CONTROL;
    Information_panel.box.outline = Single_line_outline;
    Information_panel.box.scroll_bar = Default_scroll_bar;
    Information_panel.box.number_of_rows = 3;
    Information_panel.box.row = total_rows - 2 -
                                Information_panel.box.number_of_rows -
                                Message_bar_panel.box.number_of_rows;
    Information_panel.box.column = 1;
    Information_panel.box.row_length = 78;
    Information_panel.box.attribute = Info_attribute;
    Information_panel.box.border_attribute = Info_border_attribute;
    Information_panel.text_line = Information_text;

    /* None_startable_panel */

    None_startable_panel.control = WARNING_CONTROL;
    None_startable_panel.box.outline = Blank_outline;
    None_startable_panel.box.scroll_bar = Default_scroll_bar;
    None_startable_panel.box.row = 0;
    None_startable_panel.box.column = 0;
    None_startable_panel.box.number_of_rows = 0;
    None_startable_panel.box.attribute = Warning_attribute;
    None_startable_panel.box.border_attribute = Warning_border_attribute;
    None_startable_panel.text_line = None_startable_text;


    /* call the other panel constructors */

    ConstructDiskPanels ( top_row, Message_bar_panel.box.row - 1 );
    ConstructVolumePanels ( top_row, Message_bar_panel.box.row - 1 );
    ConstructHelpPanels ( top_row, Message_bar_panel.box.row - 1 );
}


/*
 * RunUserInterface is the main user interface driver.
 * Returns the last error code that was received from the engine on
 * Commit_Changes. The interface forces the user to continue, or
 * to exit discarding changes if there is an engine error.
 */

PUBLIC
uint
RunUserInterface ( uint     min_install_size )
{
    uint    key = '\r',
            current = 0;
    bool    finished = FALSE,
            looping,
            changes_made = TRUE;                    /* *** need engine function */
    CARDINAL32      error = 0;
    static
    uint    ( *do_panel [2] ) (void) = { DoVolumePanel, DoDiskPanel };

    ShowPanel ( &Program_title_panel );
    Program_title_panel.control |= SAVE_RESTORE;
    Program_title_panel.control &= ~SHOW_ATTRIBUTES;
    Program_title_text [ 0 ] = Program_title_physical;

    Message_bar_panel.text_line [0] = Volume_message_line;
    ShowPanel ( &Message_bar_panel );
    Message_bar_panel.control &= ~SHOW_ATTRIBUTES;
    ParkCursor ();

    if ( min_install_size ) {
        sprintf ( Min_install_size_text [3],
                  " %*u", VOLUME_SIZE_WIDTH, min_install_size );
        Min_install_size_text [ 3 ][ VOLUME_SIZE_WIDTH + 1 ] = ' ';
        MESSAGE_BAR_PANEL ( Continue_message_line );
        ShowPanel ( &Min_install_size_panel );
        for(;;) {
            key = GetKeystroke ();
            if ( key == '\r' ) {
                break;
            }
        }
        Reboot_required_panel.text_line = Install_reboot_required_text;
        ErasePanel ( &Min_install_size_panel );
    }

    while ( ! finished ) {
        switch ( key ) {
            case F3:
                if ( ! changes_made ) {
                    finished = TRUE;
                    break;                                      /* quit */
                }
                MESSAGE_BAR_PANEL ( Menu_options_message_line );
                Exit_options_menu.choice = 0;                   /* continue */
                key = MenuPanel ( &Exit_options_menu );
                switch ( key ) {
                    case '\r':
                        switch ( Exit_options_menu.choice ) {
                            case 0:                             /* continue */
                                break;
                            case 1:                             /* discard */
                                finished = TRUE;
                                break;
/* SAVE AND EXIT ===================================================================== */
//debug EK
#if 0
#endif //0
                            case 2:                             /* save, exit */
                                finished = TRUE;
                                MESSAGE_BAR_PANEL (
                                             Committing_changes_message_line );
                                Commit_Changes ( &error );
                                if ( error ) {
                                    finished = FALSE;
                                    DoEngineErrorPanel ( error );
                                }
                                if ( finished  &&  NoStartablePartition () ) {
                                    MESSAGE_BAR_PANEL ( Warning_message_line );
                                    CenterPanel ( &None_startable_panel );
                                    ShowPanel ( &None_startable_panel );
                                    for ( looping = TRUE; looping; ) {
                                        key = GetKeystroke ();
                                        switch ( key ) {
                                            case '\r':
                                                looping = FALSE;
                                                break;
                                            case ESCAPE:
                                                looping = FALSE;
                                                finished = FALSE;
                                                break;
                                        }
                                    }
                                    ErasePanel ( &None_startable_panel );
                                }
/* endof SAVE AND EXIT ===================================================================== */
                                break;
#if 0                                
#endif //0 debug

                            default:
                                key = F3;
                        }
                        break;
                    case ESCAPE:
                        break;
                    default:
                        key = F3;
                }
                ErasePanel ( &Exit_options_menu );
                break;
            case F5:
                current = ! current;
                key = do_panel [current] ();
                break;
            default:
                key = do_panel [current] ();
                break;
        }
    }

    if ( Reboot_Required () ) {
        MESSAGE_BAR_PANEL ( Blank_line );
        CenterPanel ( &Reboot_required_panel );
        ShowPanel ( &Reboot_required_panel );
        loop {
            key = GetKeystroke ();                              /* loop forever */
        }
    }

    Close_LVM_Engine ();

    return  error;
}


/*
 * DoEngineErrorPanel displays an LVM engine error panel
 */

PUBLIC
void
DoEngineErrorPanel ( uint   error )
{
    uint    key;

    Engine_error_panel.control |= (INITIALIZE_PANEL | NOT_SIZED);
    Engine_error_panel.box.row_length = 0;
    if ( error > Max_LVM_engine_error ) {
        error = 0;                                      /* non-specific message */
    }
    Engine_error_panel.text_line [3] = LVM_engine_errors [ error ];
    CenterPanel ( &Engine_error_panel );

    MESSAGE_BAR_PANEL ( Continue_message_line );
    ShowPanel ( &Engine_error_panel );
    loop {
        key = GetKeystroke ();
        if ( key == '\r' ) {
            break;
        }
    }
    ErasePanel ( &Engine_error_panel );

}

PUBLIC
void  MESSAGE_BAR_PANEL(char *str)
{ // Message_bar_panel.text_line = &str;
   Message_bar_text[0] = str; //??
   MessageBarPanel(&Message_bar_panel);
}

