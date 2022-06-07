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
 * Module: show.c
 */

/*
 * Change History:
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_DOS
#include <os2.h>
#include "constant.h"
#include "display.h"
#include "getkey.h"
#include "panels.h"
#include "user.h"
#include "edit.h"


/*
 * draw a scroll bar on the right edge of the box
 * assumes panel has a border.  (panel.control & HAS_BORDER)
 */

scroll_bar_t ScrollISO   = {  '^', 'v', ':', ' ' };
scroll_bar_t ScrollNobox = { 0x18,0x19, '|', ' ' };
scroll_bar_t ScrollDBCS  = { 0x1c,0x07,0x14, ' ' };

/*
 * Fix up box characters based on the codepage.  Non-PC codepages do
 * not have the characters used to form the scoll bar in PC codepages.
 */

//PUBLIC
scroll_bar_t *
FixScrollByCodepage ( scroll_bar_t * scrollout )
{
    ULONG   codepage, len;

    codepage = 0;
    DosQueryCp(4, &codepage, &len);
    switch (codepage) {
    case 932:   /* Japan        */
    case 942:   /* Japan 1978   */
    case 943:   /* Japan 1990   */
    case 949:   /* Korea KS     */
    case 950:   /* Taiwan Big5  */
    case 1381:  /* China GB     */
        scrollout = &ScrollDBCS;
        break;

    case 813:   /* ISO Greek    */
    case 864:   /* PC Arabic    */
    case 915:   /* ISO Cyrillic */
    case 921:   /* ISO Baltic   */
    case 922:   /* ISO Estonian */
    case 1386:  /* China GBK    */
        scrollout = &ScrollNobox;
        break;

    case 874:   /* Thai         */
    case 1004:  /* ISO Latin 1  */
        scrollout = &ScrollISO;
        break;
    }
    return scrollout;
}

//PRIVATE
void
ShowRightScrollBar ( register
                     box_t  *box )
{
    register
    uint    row;
    uint    last_row, column;
    uchar   up_arrow, down_arrow, hash_box;
    scroll_bar_t * scrollbar;

    scrollbar = FixScrollByCodepage(&(box->scroll_bar));

    up_arrow = scrollbar->up_arrow;
    down_arrow = scrollbar->down_arrow;
    hash_box = scrollbar->hash_box;

    row = box->row + 1;
    column = box->column + box->row_length - 1;
    last_row = box->row + box->number_of_rows - 2;

    DisplayChars ( up_arrow, row, column, 1, 1 );

    for ( ++row; row < last_row; ++row ) {
        DisplayChars ( hash_box, row, column, 1, 1 );
    }

    DisplayChars ( down_arrow, row, column, 1, 1 );

}


/*
 * Display a string in the top or bottom border line. Leading blanks are
 * skipped to make centering easier. Only the text is displayed so that
 * border line chars on each side are not erased. Does nothing if the
 * string pointer is NULL.
 */

//PRIVATE
void
ShowBorderString ( register
                   char   *string,
                   uint   row,
                   uint   column )

{
    if ( string ) {
        while ( *string == ' ' ) {
            ++string;
            ++column;
        }
        DisplayString ( string, row, column, strlen ( string ) );
    }
}


/*
 * display each string as a text line of the panel. The array of strings
 * is terminated by a 0 entry. Clears the rest of the box.
 */

//PRIVATE
void
ShowStrings ( register
              char   **strings,
              uint   row,
              uint   column,
              uint   max_number_of_rows,
              uint   max_length,
              uint   grayed_attribute )

{
    register
    char    *string;
    uint    row_count = 0;

    for ( ; *strings; ++strings, ++row ) {
        if ( ++row_count > max_number_of_rows ) {
            break;
        }
        string = *strings;
        if ( ! IS_SEPARATOR_LINE ( string ) ) {
            if ( IS_NOT_SELECTABLE_LINE ( string ) ) {
                DisplayStringWithAttribute ( string, row, column, max_length,
                                             grayed_attribute );
            } else {
                DisplayString ( string, row, column, max_length );
            }
        }
    }
    for ( ;  row_count < max_number_of_rows;  ++row, ++row_count ) {
        DisplayString ( " ", row, column, max_length );
    }

}

outline_t BoxISO   = { '+', '+', '+', '+', '|', '-', '+', '+' };
outline_t BoxDBCS  = {0x01,0x03,0x02,0x04,0x05,0x06,0x19,0x17 };
outline_t BoxDBCST = {0x19,0x03,0x17,0x04,0x05,0x06,0x19,0x17 };
outline_t BoxArab  = {0x8d,0x8e,0x8c,0x8f,0x86,0x85,0x8a,0x88 };
outline_t BoxArabT = {0x8a,0x8e,0x88,0x8f,0x86,0x85,0x8a,0x88 };

/*
 * Fix up box characters based on the codepage
 */

//PUBLIC
outline_t *
FixOutlineByCodepage ( outline_t * boxout )
{
    ULONG   codepage, len;

    if (boxout->upper_left == ' ')
        return boxout;

    codepage = 0;
    DosQueryCp(4, &codepage, &len);
    switch (codepage) {
    case 932:   /* Japan        */
    case 942:   /* Japan 1978   */
    case 943:   /* Japan 1990   */
    case 949:   /* Korea KS     */
    case 1381:  /* China GB     */
        if (boxout->upper_left == 'Ã')
            boxout = &BoxDBCST;
        else
            boxout = &BoxDBCS;
        break;

    case 813:   /* ISO Greek    */
    case 874:   /* Thai         */
    case 915:   /* ISO Cyrillic */
    case 921:   /* ISO Baltic   */
    case 922:   /* ISO Estonian */
    case 1004:  /* ISO Latin 1  */
    case 1386:  /* China GBK    */
    case 950:   /* Taiwan Big5  */ /* - Must use ISO. Codepage has bad box corner chars. */
        boxout = &BoxISO;
        break;

    case 864:   /* PC Arabic    */
        if (boxout->upper_left == 'Ã')
            boxout = &BoxArabT;
        else
            boxout = &BoxArab;
        break;
    }
    return boxout;
}


/*
 * draw an outline of a panel using box size and coordinates
 * assumes panel has a border.  (panel.control & HAS_BORDER)
 */

//PUBLIC
void
ShowOutline ( register
              box_t  *box )
{
    register
    uint    row = box->row;
    outline_t * boxout;
    uint    last_row = box->row + box->number_of_rows - 1,
            attribute = box->border_attribute;
    uchar   upper_left, lower_left, upper_right,
            lower_right, vertical, horizontal;

    boxout = FixOutlineByCodepage(&(box->outline));

    upper_left = boxout->upper_left;
    lower_left = boxout->lower_left;
    upper_right = boxout->upper_right;
    lower_right = boxout->lower_right;
    vertical = boxout->vertical;
    horizontal = boxout->horizontal;

    DisplayCharsWithAttribute ( horizontal, row, box->column, box->row_length,
                                1, attribute );
    DisplayCharsWithAttribute ( upper_left, row, box->column, 1, 1, attribute );
    DisplayCharsWithAttribute ( upper_right, row,
                                box->column + box->row_length - 1,
                                1, 1, attribute );

    for ( ++row; row < last_row; ++row ) {
        DisplayCharsWithAttribute ( vertical, row, box->column,
                                    1, 1, attribute );
        DisplayCharsWithAttribute ( vertical, row,
                                    box->column + box->row_length - 1,
                                    1, 1, attribute );
    }

    DisplayCharsWithAttribute ( horizontal, row, box->column, box->row_length,
                                1, attribute );
    DisplayCharsWithAttribute ( lower_left, row, box->column, 1, 1, attribute );
    DisplayCharsWithAttribute ( lower_right, row,
                                box->column + box->row_length - 1,
                                1, 1, attribute );

}


/*
 * draw the horizontal separator lines of a panel.
 * assumes panel has a border.  (panel.control & HAS_BORDER)
 */

//PUBLIC
void
ShowSeparatorLines ( panel_t  *panel )
{
    register
    char    **strings = panel->text_line;
    register
    box_t   *box = &panel->box;
    uint    row = box->row;
    uint    last_row = box->row + box->number_of_rows - 1;
    outline_t * boxout;

    boxout = FixOutlineByCodepage(&(box->outline));

    for ( ++row; *strings  &&  row < last_row; ++strings, ++row ) {
        if ( IS_SEPARATOR_LINE ( *strings ) ) {
            DisplayChars ( boxout->horizontal, row, box->column + 1,
                           box->row_length - 2, 1 );
        }
    }
}


/*
 * Set the size of the panel to fit snugly around the array of strings.
 * If input size is not 0, it is not changed.
 */

//PUBLIC
void
SetPanelSize ( register
               panel_t  *panel )
{
    register
    char    **strings;
    uint    length = 0,
            number_of_rows,
            number_of_text_rows = 0,
            total_number_of_text_rows,
            non_scrollable_rows = 0,
            max_rows,
            temp_length;

    panel->control &= ~(HAS_SEPARATOR_LINE | HAS_NON_SELECTABLE_LINE);

    for ( strings = panel->text_line; *strings; ++strings ) {
        if ( panel->box.row_length == 0 ) {               /* set row length */
            temp_length = strlen ( *strings );
            if ( temp_length > length ) {
                length = temp_length;
            }
        }
        if ( IS_SEPARATOR_LINE ( *strings ) ) {
            panel->control |= HAS_SEPARATOR_LINE;
        }
        if ( IS_NOT_SELECTABLE_LINE ( *strings ) ) {
            panel->control |= HAS_NON_SELECTABLE_LINE;
        }
        ++number_of_text_rows;                           /* count text rows */
    }

    if (( panel->box.row_length == 0 ) && (panel->title_line)) {
       temp_length = strlen ( panel->title_line );
       if ( temp_length > length )
            length = temp_length;
    }

    total_number_of_text_rows = number_of_text_rows;
    if ( panel->box.row_length == 0 ) {                 /* set only if not set */
        if ( panel->control & HAS_BORDER ) {
            length += 2;
        }
        panel->box.row_length = length;
    }

    if ( panel->control & HAS_BORDER ) {
        non_scrollable_rows += 2;
    }

    number_of_rows = panel->box.number_of_rows;
    if ( number_of_rows == 0 ) {
        number_of_rows = number_of_text_rows + non_scrollable_rows;
    } else if ( number_of_text_rows > number_of_rows - non_scrollable_rows ) {
        number_of_text_rows = number_of_rows - non_scrollable_rows;
    }

    max_rows = Screen_rows - panel->box.row;            /* check for too big */
    if ( number_of_rows > max_rows ) {
        number_of_rows = max_rows;
    }
    max_rows = panel->box.max_number_of_rows;           /* user specified max */
    if ( max_rows ) {
        if ( number_of_rows > max_rows ) {
            number_of_rows = max_rows;
        }
    }
    if ( number_of_text_rows > number_of_rows - non_scrollable_rows ) {
        number_of_text_rows = number_of_rows - non_scrollable_rows;
    }

    if ( total_number_of_text_rows > number_of_text_rows ) {
        panel->control |= IS_SCROLLABLE;
    } else {
        panel->control &= ~IS_SCROLLABLE;
    }
    panel->box.number_of_rows = number_of_rows;
    panel->box.number_of_text_rows = number_of_text_rows;
    panel->control &= ~NOT_SIZED;

}


/*
 * Center the panel in the middle of the screen.
 */

//PUBLIC
void
CenterPanel ( register
              panel_t  *panel )
{

    if ( panel->control & NOT_SIZED ) {
        SetPanelSize ( panel );
    }

    panel->box.row = ( Screen_rows - panel->box.number_of_rows ) / 2;
    panel->box.column = ( Screen_columns - panel->box.row_length ) / 2;

}


/*
 * Check to see that choice is within the index range of text_line.
 * If not, set it to 0 which is assumed to always be valid for
 * a non-empty panel. Then skip over non-selectable entries.
 * If all are non-selectable, choice will be the first or last entry
 * depending on the direction, up or down.
 * Returns the current choice.
 */

//PUBLIC
uint
ValidatePanelChoice ( panel_t  *panel,
                      uint     direction )
{
    register
    char    **line;
    register
    uint    choice;
    uint    number_of_text_rows;

    if ( panel->control & NOT_SIZED ) {
        SetPanelSize ( panel );
    }

    line = panel->text_line;
    choice = panel->choice;
    number_of_text_rows = panel->box.number_of_text_rows;

    if ( choice >= number_of_text_rows ) {              /* choice is zero-based */
        choice = 0;
    }

    if ( number_of_text_rows > 0 ) {
        switch ( direction ) {
            case UP_CURSOR:
                while ( line [choice][0] == NOT_SELECTABLE_CHAR ) {  /* scan up */
                    if ( choice > 0 ) {
                        --choice;
                    } else {
                        break;
                    }
                }
                while ( line [choice][0] == NOT_SELECTABLE_CHAR ) { /* scan down */
                    if ( choice < number_of_text_rows - 1 ) {
                        ++choice;
                    } else {
                        break;
                    }
                }
                break;
            case DOWN_CURSOR:
                while ( line [choice][0] == NOT_SELECTABLE_CHAR ) { /* scan down */
                    if ( choice < number_of_text_rows - 1 ) {
                        ++choice;
                    } else {
                        break;
                    }
                }
                while ( line [choice][0] == NOT_SELECTABLE_CHAR ) {  /* scan up */
                    if ( choice > 0 ) {
                        --choice;
                    } else {
                        break;
                    }
                }
                break;
        }
    }

    panel->choice = choice;                                  /* update it */

    return  choice;
}


/*
 * Generic panel as an outlined box of text strings.
 */

//PUBLIC
void
ShowPanel ( register
            panel_t  *panel )
{
    uint    row_length,
            number_of_rows,
            max_string_length,
            row,
            column,
            grayed_attribute;

    if ( panel->control & NOT_SIZED ) {
        SetPanelSize ( panel );
    }

    row_length = panel->box.row_length;
    number_of_rows = panel->box.number_of_rows;

    if ( panel->control & SAVE_RESTORE ) {
        if ( panel->box.save_background == NULL ) {
            panel->box.save_background = SaveDisplayBlock ( panel->box.row,
                                                            panel->box.column,
                                                            row_length,
                                                            number_of_rows );
        }
    }
    if ( panel->control & SHOW_ATTRIBUTES ) {
        row = panel->box.row;
        column =  panel->box.column;
        row_length = panel->box.row_length;
        number_of_rows = panel->box.number_of_rows;
        if ( panel->control & HAS_BORDER ) {
            ++row;
            ++column;
            row_length -= 2;
            number_of_rows -= 2;
        }
        DisplayAttributes ( row, column, row_length, number_of_rows,
                            panel->box.attribute );

    }
    if ( (panel->control & SHOW_OUTLINE)  &&  (panel->control & HAS_BORDER) ) {
        ShowOutline ( &panel->box );
        if ( panel->control & HAS_SEPARATOR_LINE ) {
            ShowSeparatorLines ( panel );
        }
    }
    if ( (panel->control & SHOW_TITLE)  &&  (panel->control & HAS_BORDER) ) {
        if ( panel->title_line ) {
            ShowBorderString ( panel->title_line, panel->box.row,
                               panel->box.column );
        }
    }
    if ( (panel->control & SHOW_MESSAGE)  &&  (panel->control & HAS_BORDER) ) {
        if ( panel->message_line ) {
            ShowBorderString ( panel->message_line,
                               panel->box.number_of_rows - 1,
                               panel->box.column );
        }
    }
    if ( (panel->control & SHOW_SCROLL_BAR) && (panel->control & HAS_BORDER) ) {
        ShowRightScrollBar ( &panel->box );
    }
    if ( panel->control & SHOW_TEXT ) {
        row = panel->box.row;
        column =  panel->box.column;
        max_string_length = panel->box.row_length;
        number_of_rows = panel->box.number_of_rows;
        if ( panel->control & HAS_BORDER ) {
            ++row;
            ++column;
            max_string_length -= 2;
            number_of_rows -= 2;
        }
        grayed_attribute = panel->box.attribute;
        if ( grayed_attribute & HIGHLIGHTED_FG ) {
            grayed_attribute &= ~HIGHLIGHTED_FG;            /* just dim it */
        } else {
            grayed_attribute &= 0xF0;                       /* zero foreground */
            grayed_attribute |= GRAY_FG;                    /* make it gray */
        }
        ShowStrings ( panel->text_line,
                      row,
                      column,
                      number_of_rows,
                      max_string_length,
                      grayed_attribute );
    }

}


/*
 * Reset a generic panel to the background attribute.
 */

//PUBLIC
void
ErasePanel ( panel_t  *panel )
{
    uint    row_length = panel->box.row_length,
            number_of_rows = panel->box.number_of_rows,
            row,
            column;

    if ( panel->control & SAVE_RESTORE ) {
        if ( panel->box.save_background ) {
            RestoreDisplayBlock ( panel->box.row,
                                  panel->box.column,
                                  row_length,
                                  number_of_rows,
                                  panel->box.save_background );
            panel->box.save_background = NULL;
        }
    } else if ( panel->control & ERASE_TEXT_ONLY ) {
        row = panel->box.row;
        column =  panel->box.column;
        row_length = panel->box.row_length;
        number_of_rows = panel->box.number_of_rows;
        if ( panel->control & HAS_BORDER ) {
            ++row;
            ++column;
            row_length -= 2;
            number_of_rows -= 2;
        }
        DisplayChars ( ' ', row, column, row_length, number_of_rows );
    } else {
        DisplayCharsWithAttribute ( ' ',
                                    panel->box.row,
                                    panel->box.column,
                                    row_length,
                                    number_of_rows,
                                    Background_attribute );
    }
}


#include "interface.h"

// PUBLIC
uint
DataEntryPanel ( panel_t  *panel )
{
    uint  row_length,xmle=1,l=2;
    int i, rc;
    char ch;

    if ( panel->control & NOT_SIZED )
    {
        SetPanelSize ( panel );
        if(panel->input_line)
        {
           l = strlen(*panel->input_line);
           panel->box.row_length += l;
        }
    }
    ShowPanel (panel);

    {
       PMLE mess;
//      MakeMessageScreen();
        l = strlen(*panel->input_line);
        xmle = panel->box.column  + panel->box.row_length - l-1;
        mess = MLE_new(l,1,246,NULL,NULL,NULL,NULL);
        strncpy(mess->buf,*panel->input_line,l);

        MLE_load(panel->box.row+1, xmle, mess,WM_BLACK,WM_PALEGRAY);
        while (1)
        {
          rc = MLE_edit(mess);
          switch (rc)
          {
            case CR:
//                FinishFlag=1;
            for(i=0; i<l; i++)
             {  ch = mess->buf[i];
                if(ch == 0) break;
                (*panel->input_line)[i] = ch;
             }

            if(mess->buf)
                 free(mess->buf);
              return ENTER;
            case ESC:
//               WhereIam=PAGER;
            if(mess->buf)
                   free(mess->buf);
              return ESCAPE;
           }
        }
    }

    return 0;

}
