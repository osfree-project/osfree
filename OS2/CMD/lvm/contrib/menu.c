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
 * Module: menu.c
 */

/*
 * Change History:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "lvm2.h"
#include "edit.h"
#define TRUE  1
#define FALSE 0


/*
 * Size and display a simple informational panel. No input.
 */


void
MessagePanel ( panel_t  *panel )
{

    panel->box.row_length = 0;                  /* force resizing */
    panel->box.number_of_rows = 0;
    SetPanelSize ( panel );
    ShowPanel ( panel );

}


/*
 * Message bar panel is a single line bar across the bottom of the screen.
 * Hides the cursor again because the background attribute is changed.
 */

void
MessageBarPanel ( panel_t  *panel )
{

    panel->box.row = Screen_rows - 1;
    panel->box.column = 0;
    panel->box.row_length = Screen_columns;
    panel->box.number_of_rows = 1;
    panel->control = 0;
    panel->control |= SHOW_ATTRIBUTES;
    panel->control |= SAVE_RESTORE;
    panel->control |= SHOW_TEXT;

    SetPanelSize ( panel );
    ShowPanel ( panel );
    ParkCursor();

}


/*
 * Display a generic panel as a menu of strings. Current choice is reverse
 * video. Use arrow keys to move between choices, enter to select a choice.
 * Escape for no choice. Updates the choice made in the panel struct.
 * Returns the key pressed. Does not erase the panel.
 */


uint
MenuPanel ( register
            panel_t  *panel )
{
    int     key = ESCAPE,
            choice;
    uint    row = panel->box.row,
            column = panel->box.column,
            max_string_length;
    char    **line = panel->text_line;
    bool    finished = FALSE;

    ShowPanel ( panel );

    if ( panel->box.number_of_text_rows == 0 ) {
        return key;                                             /* early exit */
    }

    max_string_length = panel->box.row_length;
    if ( panel->control & HAS_BORDER ) {
        ++row;
        ++column;
        max_string_length -= 2;
    }

    choice = ValidatePanelChoice ( panel, DOWN_CURSOR );
    ReverseAttributes ( row + choice, column, max_string_length, 1 );

    if ( line [choice][0] == NOT_SELECTABLE_CHAR ) {         /* none selectable */
        finished = TRUE;
    }

    while ( ! finished ) {
        key = GetKeystroke ();
        switch ( key ) {
            case UP_CURSOR:
                if ( choice > 0 ) {
                    ReverseAttributes ( row + choice, column,
                                        max_string_length, 1 );
                    --panel->choice;
                    choice = ValidatePanelChoice ( panel, UP_CURSOR );
                    ReverseAttributes ( row + choice, column,
                                        max_string_length, 1 );
                }
                break;
            case DOWN_CURSOR:
                if ( line [choice + 1] ) {
                    ReverseAttributes ( row + choice, column,
                                        max_string_length, 1 );
                    ++panel->choice;
                    choice = ValidatePanelChoice ( panel, DOWN_CURSOR );
                    ReverseAttributes ( row + choice, column,
                                        max_string_length, 1 );
                }
                break;
            case HELP:
                if ( panel->help.routine ) {
                    panel->help.routine ( panel );
                }
                break;
            default:
                finished = TRUE;
        }
    }

    panel->choice = choice;                                 /* update it */

    return  key;
}

/*
 *
 * This is a special menu panel.  It allows the caller to manipulate the
 * menu items.
 *
 *
 *   Use arrow keys to move between choices.
 *   Use space bar to select or deselect a menu item, use up/down to move
 *   the selected item to the desired position then space bar to de-select item
 *   Use ENTER/ESCAPE to exit the panel and save the order.
 * Returns the key pressed. Does not erase the panel.
 *
 */


uint
UserDefinedMenuPanel ( register panel_t  *panel,
                       uint     count_menu_items,    /* number of menu items */
                       uint    *menu_item_action,    /* pointer to array of menu item action */
                       char     char_for_selection1, /* character to show selected menu item */
                       char     char_for_selection2) /* character to show selected menu item */
{
    int     key = 0, i;
    uint    row = panel->box.row,
            column = panel->box.column,
            max_string_length;
    uint    *action;
    char    **line = panel->text_line;
    //uint    (* _System callback) (panel_t *panel) = panel->callback;
    uint    _System (* callback) (panel_t *panel) = panel->callback;
    bool    finished = FALSE;
    uint    grayed_attribute, normal_attribute;



    ShowPanel ( panel );

    if ( panel->box.number_of_text_rows == 0 ) {
        return key;                                             /* early exit */
    }

    max_string_length = panel->box.row_length;
    if ( panel->control & HAS_BORDER ) {
        ++row;
        ++column;
        max_string_length -= 2;
    }

    if (callback == NULL) /* must have callback routine */
       finished = TRUE;

    grayed_attribute = normal_attribute = panel->box.attribute;
    if ( grayed_attribute & HIGHLIGHTED_FG ) {
        grayed_attribute &= ~HIGHLIGHTED_FG;            /* just dim it */
    } else {
        grayed_attribute &= 0xF0;                       /* zero foreground */
        grayed_attribute |= GRAY_FG;                    /* make it gray */
    }

    while ( ! finished ) {

        switch ( key ) {
            case HELP:
                if ( panel->help.routine ) {
                    panel->help.routine ( panel );
                }
                break;

            /* case UP_CURSOR: */
            /* case DOWN_CURSOR: */
            /* case SPACE: */
            default:
               /* call back */
               if (key) {
                  panel->choice = (uint)key;   /* notify caller a key was pressed */
                  finished = (bool)callback(panel);  /* callback routine must update "choice" */
               }

               /* the caller must set the state for each of the menu items */

               for (i=0, action=menu_item_action; i<count_menu_items; i++, action++) {
                  if (*action & ACTION_NONE)
                     continue;                 /* leave the menu item as is */

                  if (*action & ACTION_REDRAW) /* redraw this menu item */
                     DisplayString ( line[i], row+i, column, max_string_length );

                  if (*action & ACTION_DESELECT) {
                     DisplayChars(' ', row+i, column, 1, 1);
                     if (char_for_selection2)
                        DisplayChars(' ', row+i, column + strlen(line[i])-1, 1, 1);
                  }

                  if (*action & ACTION_SELECT) {
                     DisplayChars(char_for_selection1, row+i, column, 1, 1);
                     if (char_for_selection2)
                        DisplayChars(char_for_selection2, row+i, column + strlen(line[i])-1, 1, 1);
                  }

                  if (*action & ACTION_SET_NOT_SELECTABLE)
                     DisplayAttributes (row+i, column, max_string_length, 1, grayed_attribute );

                  if (*action & ACTION_SET_SELECTABLE)
                     DisplayAttributes (row+i, column, max_string_length, 1, normal_attribute );

                  if (*action & (ACTION_HIGHLIGHT | ACTION_UNHIGHLIGHT))
                     ReverseAttributes ( row+i, column, max_string_length, 1);


               }
               break;
        }

        if (! finished)
           key = GetKeystroke ();
    }

    return  key;
}




/*
 * A panel that can have more text lines than the size of the panel. Can
 * scroll using arrow and page keys. Returns key pressed. The choice is
 * updated in the panel structure.
 * A callback routine is called after each key stroke so the caller can
 * update a background panel to match the current light bar selection.
 * Does not erase the panel.
 */


int
ScrollingMenuPanel ( register
                     panel_t  *panel )
{
    int     key = ESCAPE,
            direction;
    uint    row,
            row_count,
            max_string_length,
            number_of_text_rows,
            save_control,
            save_border_attribute = panel->box.border_attribute,
            column;
    char    **original_text_line = panel->text_line,
            **top_line,
            **current_line,
            **bottom_line;
    bool    finished = FALSE;
    //uint    (* _System callback) (panel_t *panel) = panel->callback;
    uint    _System (* callback) (panel_t *panel) = panel->callback;

    if ( panel->control & INITIALIZE_PANEL ) {
        panel->control &= ~INITIALIZE_PANEL;
        top_line = original_text_line;
        while ( *top_line  &&  **top_line == NOT_SELECTABLE_CHAR ) {
            if ( *++top_line == NULL ) {
                --top_line;
                break;
            }
        }
        panel->top_line = top_line;
        panel->current_line = top_line;
    }

    SetPanelSize ( panel );
    number_of_text_rows = panel->box.number_of_text_rows;
    if ( number_of_text_rows == 0 ) {
        return key;                                             /* early exit */
    }

    if ( panel->control & HIGHLIGHT_BORDER_LINE ) {
        panel->box.border_attribute |= HIGHLIGHTED_FG;
    }

    save_control = panel->control;

    if ( panel->control & IS_SCROLLABLE ) {
        panel->control |= SHOW_SCROLL_BAR;
    } else {
        panel->control &= ~SHOW_SCROLL_BAR;
    }

    row = panel->box.row;
    column = panel->box.column;
    top_line = panel->top_line;
    current_line = panel->current_line;

    max_string_length = panel->box.row_length;
    if ( panel->control & HAS_BORDER ) {
        ++row;
        ++column;
        max_string_length -= 2;
    }

    panel->text_line = top_line;
    bottom_line = top_line;

    for ( row_count = 1;  row_count < number_of_text_rows;  ++row_count ) {
        if ( *(bottom_line + 1) ) {
            ++bottom_line;
        } else {
            break;
        }
    }

    ShowPanel ( panel );

    panel->control &= ~SHOW_NON_TEXT_ITEMS;           /* just need to show text */
    if ( panel->control & HAS_NON_SELECTABLE_LINE ) {
        panel->control |= SHOW_ATTRIBUTES;
    }

    if ( callback ) {
        panel->text_line = original_text_line;                 /* restore value */
        panel->choice = current_line - original_text_line;
        callback ( panel );
    }

    row += current_line - top_line;
    ReverseAttributes ( row, column, max_string_length, 1 );            /* on */

    while ( ! finished ) {
        key = GetKeystroke ();
        ReverseAttributes ( row, column, max_string_length, 1 );        /* off */
        switch ( key ) {
            case UP_CURSOR:
                if ( current_line > original_text_line ) {
                    if ( current_line == top_line ) {
                        --top_line;
                        --bottom_line;
                    } else {
                        --row;
                    }
                    --current_line;
                }
                direction = UP_CURSOR;
                break;
            case DOWN_CURSOR:
                if ( *(current_line + 1) ) {
                    if ( current_line == bottom_line ) {
                        ++top_line;
                        ++bottom_line;
                    } else {
                        ++row;
                    }
                    ++current_line;
                }
                direction = DOWN_CURSOR;
                break;
            case DOWNPAGE:
                row_count = 1;
                for ( ; row_count < number_of_text_rows; ++row_count ) {
                    if ( ! *(bottom_line + 1) ) {           /* no more lines */
                        break;
                    }
                    ++top_line;
                    ++bottom_line;
                    ++current_line;
                }
                direction = DOWN_CURSOR;
                break;
            case UPPAGE:
                row_count = 0;
                while ( top_line > original_text_line ) {
                    if ( ++row_count >= number_of_text_rows ) {
                        break;                              /* 1 page of lines */
                    }
                    --top_line;
                    --bottom_line;
                    --current_line;
                }
                direction = UP_CURSOR;
                break;
            case TOP_OF_FILE:
                while ( top_line > original_text_line ) {
                    --top_line;
                    --bottom_line;
                    --current_line;
                }
                while ( current_line > top_line ) {
                    --current_line;
                    --row;
                }
                direction = UP_CURSOR;
                break;
            case END_OF_FILE:
                while ( *(bottom_line + 1) ) {
                    ++top_line;
                    ++bottom_line;
                    ++current_line;
                }
                while ( current_line < bottom_line ) {
                    ++current_line;
                    ++row;
                }
                direction = DOWN_CURSOR;
                break;
            case HELP:
                if ( panel->help.routine ) {
                    panel->help.routine ( panel );
                }
                direction = 0;
                break;
            default:
                direction = 0;
                finished = TRUE;
        }

        switch ( direction ) {                /* skip over non-selectable lines */
            case UP_CURSOR:
                while ( **current_line == NOT_SELECTABLE_CHAR ) {
                    if ( current_line > original_text_line ) {
                        if ( current_line == top_line ) {
                            --top_line;
                            --bottom_line;
                        } else {
                            --row;
                        }
                        --current_line;
                    } else {
                        break;
                    }
                }
                while ( **current_line == NOT_SELECTABLE_CHAR ) {
                    if ( *(current_line + 1) ) {
                        if ( current_line == bottom_line ) {
                            ++top_line;
                            ++bottom_line;
                        } else {
                            ++row;
                        }
                        ++current_line;
                    } else {
                        break;
                    }
                }
                break;
            case DOWN_CURSOR:
                while ( **current_line == NOT_SELECTABLE_CHAR ) {
                    if ( *(current_line + 1) ) {
                        if ( current_line == bottom_line ) {
                            ++top_line;
                            ++bottom_line;
                        } else {
                            ++row;
                        }
                        ++current_line;
                    } else {
                        break;
                    }
                }
                while ( **current_line == NOT_SELECTABLE_CHAR ) {
                    if ( current_line > original_text_line ) {
                        if ( current_line == top_line ) {
                            --top_line;
                            --bottom_line;
                        } else {
                            --row;
                        }
                        --current_line;
                    } else {
                        break;
                    }
                }
                break;
            default:
                ;
        }
        if ( panel->top_line != top_line ) {
            panel->top_line = top_line;
            panel->text_line = top_line;
            ShowPanel ( panel );
        }

        ReverseAttributes ( row, column, max_string_length, 1 );      /* on */

        if ( ! finished ) {
            if ( callback ) {
                panel->text_line = original_text_line;      /* restore values */
                panel->top_line = top_line;                 /* update values */
                panel->current_line = current_line;
                panel->choice = current_line - original_text_line;
                callback ( panel );
            }
        }
    }

    panel->top_line = top_line;                             /* update values */
    panel->current_line = current_line;
    panel->choice = current_line - original_text_line;

    panel->control = save_control;                          /* restore values */
    panel->box.border_attribute = save_border_attribute;
    panel->text_line = original_text_line;

    if ( panel->control & TURN_OFF_LIGHT_BAR ) {
        ReverseAttributes ( row, column, max_string_length, 1 );      /* off */
    }
    if ( panel->control & HIGHLIGHT_BORDER_LINE ) {
        ShowOutline ( &panel->box );
    }

    return key;
}


/*
 * A panel that can have more text lines than the size of the panel. Can
 * scroll using arrow and page keys. Display only, no edit capability.
 * Does not erase the panel. Esc to return.
 */


uint
ScrollingPanel ( panel_t  *panel )
{
    int     key = ESCAPE;
    uint    row = panel->box.row,
            row_count,
            number_of_text_rows,
            save_control,
            column = panel->box.column;
    char    **original_text_line = panel->text_line,
            **top_line = panel->text_line,
            **bottom_line;
    bool    finished = FALSE;

    SetPanelSize ( panel );
    number_of_text_rows = panel->box.number_of_text_rows;
    if ( number_of_text_rows == 0 ) {
        return key;                                             /* early exit */
    }

    save_control = panel->control;

    if ( panel->control & HAS_BORDER ) {
        ++row;
        ++column;
    }
    if ( panel->control & IS_SCROLLABLE ) {
        panel->control |= SHOW_SCROLL_BAR;
    } else {
        panel->control &= ~SHOW_SCROLL_BAR;
    }

    panel->text_line = top_line;
    bottom_line = top_line;

    for ( row_count = 1;  row_count < number_of_text_rows;  ++row_count ) {
        if ( *(bottom_line + 1) ) {
            ++bottom_line;
        } else {
            break;
        }
    }

    ShowPanel ( panel );

    panel->control &= ~SHOW_NON_TEXT_ITEMS;           /* just need to show text */

    while ( ! finished ) {
        key = GetKeystroke ();
        switch ( key ) {
            case UP_CURSOR:
                if ( top_line > original_text_line ) {
                    --top_line;
                    --bottom_line;
                }
                break;
            case DOWN_CURSOR:
                if ( *(bottom_line + 1) ) {
                    ++top_line;
                    ++bottom_line;
                }
                break;
            case DOWNPAGE:
                row_count = 0;
                for ( ; *bottom_line; ++bottom_line, ++top_line ) {
                    if ( ++row_count >= number_of_text_rows ) {
                        break;                              /* last window line */
                    }
                    if ( ! *(bottom_line + 1) ) {           /* no more lines */
                        break;
                    }
                }
                break;
            case UPPAGE:
                row_count = 0;
                while ( top_line > original_text_line ) {
                    if ( ++row_count >= number_of_text_rows ) {
                        break;                              /* 1 page of lines */
                    }
                    --top_line;
                    --bottom_line;
                }
                break;
            case TOP_OF_FILE:
                while ( top_line > original_text_line ) {
                    --top_line;
                    --bottom_line;
                }
                break;
            case END_OF_FILE:
                while ( *(bottom_line + 1) ) {
                    ++top_line;
                    ++bottom_line;
                }
                break;
            case ESCAPE:
                finished = TRUE;
                break;
            case HELP:
                if ( panel->help.routine ) {
                    panel->help.routine ( panel );
                }
                break;
        }
        panel->text_line = top_line;
        ShowPanel ( panel );
    }

    panel->text_line = original_text_line;                  /* restore values */
    panel->control = save_control;

    return key;
}


