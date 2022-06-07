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
 * Module: lvm.c
 */

/*
 * Change History:
 *
 */
typedef unsigned char uchar;
typedef unsigned int  uint;

struct _outline1
{
    unsigned char   upper_left;
    uchar   lower_left;
    uchar   upper_right;
    uchar   lower_right;
    uchar   vertical;
    uchar   horizontal;
    uchar   left_tee;
    uchar   right_tee;
};
typedef struct _outline1 outline1_t;

/*
 * public defines and structures
 */

#define NO_CHOICE_MADE          -1
#define NOT_SELECTABLE_CHAR     '\xFF'
#define NOT_SELECTABLE_STRING   "\xFF"
#define NOT_SELECTABLE_LINE     "\xFF "
#define SEPARATOR_LINE          "\xFF"
#define IS_SEPARATOR_LINE(s) ( (s)[0] == NOT_SELECTABLE_CHAR && (s)[1] == '\0' )
#define IS_NOT_SELECTABLE_LINE(s) ( (s)[0] == NOT_SELECTABLE_CHAR && \
                                    (s)[1] != '\0' )

/*
 * menu item states and actions
 */
#define STATE_SELECTABLE       0x0001
#define STATE_SELECTED         0x0002


#define ACTION_NONE                0x00010000
#define ACTION_REDRAW              0x00020000
#define ACTION_HIGHLIGHT           0x00040000
#define ACTION_UNHIGHLIGHT         0x00080000
#define ACTION_SELECT              0x00100000
#define ACTION_DESELECT            0x00200000
#define ACTION_SET_SELECTABLE      0x00400000
#define ACTION_SET_NOT_SELECTABLE  0x00800000

#define ACTION_CLEAR_BITS          0x0001FFFF  /* when AND with this, the state is unchanged */
                                               /* but action is set to NONE. */
/*
 * panel control and structures
 */

#define INITIALIZE_PANEL        0x0001
#define NOT_SIZED               0x0002
#define SAVE_RESTORE            0x0004
#define ERASE_TEXT_ONLY         0x0008

#define SHOW_TEXT               0x0010
#define SHOW_ATTRIBUTES         0x0020
#define SHOW_OUTLINE            0x0040

#define SHOW_TITLE              0x0100
#define SHOW_MESSAGE            0x0200
#define SHOW_SCROLL_BAR         0x0400
#define IS_SCROLLABLE           0x0800

#define HAS_NON_SELECTABLE_LINE 0x1000
#define HAS_BORDER              0x2000
#define HAS_SEPARATOR_LINE      0x4000

#define NUMERIC_DATA_ONLY       0x8000

#define TURN_OFF_LIGHT_BAR      0x00010000
#define HIGHLIGHT_BORDER_LINE   0x00020000

#define SHOW_NON_TEXT_ITEMS     (SHOW_ATTRIBUTES | SHOW_OUTLINE | \
                                 SHOW_SCROLL_BAR)

#define BLANK_LINE_BOX          ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
#define SINGLE_LINE_BOX         'Ú', 'À', '¿', 'Ù', '³', 'Ä', 'Ã', '´'
#define SCROLL_BAR              '', '', '°', ' '
#define DEFAULT_CONTROL         ( (0x03FF | HAS_BORDER) & ~ERASE_TEXT_ONLY )
#define DEFAULT_PANEL_INIT      DEFAULT_CONTROL, SINGLE_LINE_BOX, SCROLL_BAR

typedef struct _outline
{
    uchar   upper_left;
    uchar   lower_left;
    uchar   upper_right;
    uchar   lower_right;
    uchar   vertical;
    uchar   horizontal;
    uchar   left_tee;
    uchar   right_tee;
} outline_t;

typedef struct _scroll_bar {
    uchar   up_arrow;
    uchar   down_arrow;
    uchar   hash_box;
    uchar   pad1;
} scroll_bar_t;

typedef struct _box {
    outline_t       outline;                    /* outline chars                 */
    scroll_bar_t    scroll_bar;                 /* scroll bar chars              */
    uint            row;                        /* top row of the box            */
    uint            column;                     /* left column of the box        */
    uint            row_length;                 /* width of the box              */
    uint            number_of_rows;             /* total box rows                */
    uint            number_of_text_rows;        /* visible strings in the box    */
    uint            max_number_of_rows;         /* not to be exceeded            */
    uint            attribute;                  /* foreground and background     */
    uint            border_attribute;           /* foreground and background     */
    void            *save_background;           /* storage pointer or NULL       */
} box_t;

struct _panel;

typedef struct _help {
    uint    *array;                             /* indices or message numbers     */
    uint    array_size;                         /* total array entries            */
    uint    default_message;                    /* default message number         */
    void    (* _System routine) (struct _panel *panel);  /* help routine                   */
} help_t;

typedef struct _panel {
    uint    control;                            /* describes the panel          */
    box_t   box;                                /* from structs above           */
    help_t  help;                               /* from struct above            */
    uint    choice;                             /* index into text_line array   */
    char    **text_line,                        /* strings in the box           */
            **input_line,                       /* editable strings             */
            **top_line,                         /* current string at top row    */
            **current_line;                     /* current selected string      */
    char    *title_line,                        /* top border text              */
            *message_line;                      /* bottom border text           */
    uint    (* _System callback) (struct _panel *panel); /* call back routine   */
} panel_t;


/*
 * global variables
 */

extern
uint    Background_attribute,                      /* background screen         */
        Screen_rows,                               /* number of screen rows     */
        Screen_columns;                            /* number of screen columns  */



/*
 * public function prototypes from panels.c, show.c, entry.c, menu.c
 */


/*
 * Must be called before any panel routines. Sets up the attributes,
 * initializes the Display routines, sets the exit routine, and clears
 * the display.
 * Returns the code from InitializeDisplay.
 */

// PUBLIC
uint
InitializePanels ( uint default_attribute,
                   uint exit_attribute );


/*
 * Can be called at any time to reset the attributes and clear the display.
 */

// PUBLIC
void
ReInitializePanels ( uint default_attribute,
                     uint exit_attribute );


/*
 * Return the total number of screen rows
 */

// PUBLIC
uint
GetScreenRows ( void );


/*
 * Return the total number of screen columns
 */

// PUBLIC
uint
GetScreenColumns ( void );


/*
 * draw an outline of a panel using box size and coordinates
 * assumes panel.has_border
 */

// PUBLIC
void
ShowOutline ( register
              box_t  *box );


/*
 * draw the horizontal separator lines of a panel.
 * assumes panel.control & HAS_BORDER
 */

// PUBLIC
void
ShowSeparatorLines ( panel_t  *panel );


/*
 * Set the size of the panel to fit snugly around the array of strings.
 * If input size is not 0, it is not changed.
 */

// PUBLIC
void
SetPanelSize ( register
               panel_t  *panel );


/*
 * Center the panel in the middle of the screen.
 */

// PUBLIC
void
CenterPanel ( register
              panel_t  *panel );


/*
 * Check to see that choice is within the index range of text_line.
 * If not, set it to 0 which is assumed to always be valid for
 * a non-empty panel. Then skip over non-selectable entries.
 * If all are non-selectable, index will be the first or last entry
 * depending on the direction, up or down.
 */

// PUBLIC
uint
ValidatePanelChoice ( panel_t  *panel,
                      uint     direction );


/*
 * Generic panel as an outlined box of text strings.
 */

// PUBLIC
void
ShowPanel ( panel_t  *panel );


/*
 * Reset a generic panel to the background attribute.
 */

// PUBLIC
void
ErasePanel ( panel_t  *panel );



/*
 * Size and display a simple informational panel. No input.
 */

// PUBLIC
void
MessagePanel ( panel_t  *panel );


/*
 * Message bar panel is a single line bar across the bottom of the screen.
 * Hides the cursor again because the background attribute is changed.
 */

// PUBLIC
void
MessageBarPanel ( panel_t  *panel );


/*
 * Display a generic panel as a menu of strings. Current choice is reverse
 * video. Use arrow keys to move between choices, enter to select a choice.
 * Escape for no choice. Updates the choice made in the panel struct.
 * Returns the key pressed. Does not erase the panel.
 */

// PUBLIC
uint
MenuPanel ( register
            panel_t  *panel );

/*
 * This is a special menu panel.  It allows the caller to manipulate the
 * menu items.
 *
 * A callback routine must be provided by the caller. Upon every key pressed
 * by the user, the callback will be invoked.  The pressed key is passed via
 * panel->choice.
 */

// PUBLIC
uint
UserDefinedMenuPanel ( register panel_t  *panel,
                       uint     count_menu_items,     /* number of menu items                  */
                       uint    *menu_item_action,     /* pointer to array of menu item action  */
                       char     char_for_selection1,  /* character to show selected menu item  */
                       char     char_for_selection2); /* character to show selected menu item  */


/*
 * A panel that can have more text lines than the size of the panel. Can
 * scroll using arrow and page keys. Returns key pressed. The choice is
 * updated in the panel structure.
 * A callback routine is called after each key stroke so the caller can
 * update a background panel to match the current light bar selection.
 * Does not erase the panel.
 */

// PUBLIC
int
ScrollingMenuPanel ( register
                     panel_t  *panel );


/*
 * A panel that can have more text lines than the size of the panel. Can
 * scroll using arrow and page keys. Display only, no edit capability.
 * Does not erase the panel. Esc to return.
 */

// PUBLIC
uint
ScrollingPanel ( panel_t  *panel );


/*
 * Display a data entry panel as strings followed by data entry fields.
 * Put entry field on next line by making text_line not selectable and
 * adding a blank text_line after and a blank input_line before the lines.
 * The current edit field is in reverse video. Uses EditLine on each line.
 * Use arrow keys to move between choices, any other key to quit.
 * Returns the key from EditLine. Does not erase the panel.
 * Number of text_lines must equal number of input_lines to work.
 */

// PUBLIC
uint
DataEntryPanel ( panel_t  *panel );

