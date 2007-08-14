/***********************************************************************/
/* COMMSOS.C - sos commands.                                           */
/* This file contains all commands that can be assigned to function    */
/* keys or typed on the command line.                                  */
/***********************************************************************/
/*
 * THE - The Hessling Editor. A text editor similar to VM/CMS xedit.
 * Copyright (C) 1991-2001 Mark Hessling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 *
 *    The Free Software Foundation, Inc.
 *    675 Mass Ave,
 *    Cambridge, MA 02139 USA.
 *
 *
 * If you make modifications to this software that you feel increases
 * it usefulness for the rest of the community, please email the
 * changes, enhancements, bug fixes as well as any and all ideas to me.
 * This software is going to be maintained and enhanced as deemed
 * necessary by the community.
 *
 * Mark Hessling,  M.Hessling@qut.edu.au  http://www.lightlink.com/hessling/
 */

static char RCSid[] = "$Id: commsos.c,v 1.8 2002/06/09 09:26:16 mark Exp $";

#include <the.h>
#include <proto.h>

/*#define DEBUG 1*/

static short sosdelback Args(( bool ));
static short sosdelchar Args(( bool ));

/*man-start*********************************************************************


========================================================================
SOS COMMAND REFERENCE
========================================================================
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     sos addline - add blank line after focus line

SYNTAX
     SOS ADDline

DESCRIPTION
     The SOS ADDLINE command inserts a blank line in the file following
     the focus line. The cursor is placed in the column under the first
     non-blank in the focus line.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SOS LINEADD>, <SOS DELLINE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_addline(CHARTYPE *params)
#else
short Sos_addline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_addline");
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 insert_new_line((CHARTYPE *)"",0,1,get_true_line(FALSE),FALSE,FALSE,TRUE,CURRENT_VIEW->display_low,TRUE,TRUE);
 if (compatible_feel == COMPAT_XEDIT)
    advance_current_line(1L);
 if (curses_started
 && CURRENT_VIEW->current_window == WINDOW_COMMAND)
   {
    THEcursor_home(FALSE);
    rc = Sos_firstcol((CHARTYPE *)"");
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos blockend - move cursor to end of marked block

SYNTAX
     SOS BLOCKEnd

DESCRIPTION
     The SOS BLOCKEND command moves the cursor to the ending line
     and column of the marked block.  If the cursor is on the command 
     line, the last line of the marked block becomes the current line.

     If no marked block is in the current file, an error is displayed.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS BLOCKSTART>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_blockend(CHARTYPE *params)
#else
short Sos_blockend(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 short rc=RC_OK;
 LINE *curr=NULL;
 LENGTHTYPE col=0;
 LINETYPE line=0L;
 short save_compat=0;
 CHARTYPE cmd[20];
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_blockend");
 if (MARK_VIEW != CURRENT_VIEW)
   {
    display_error(45,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 switch(MARK_VIEW->mark_type)
   {
    case M_COLUMN:
         col = MARK_VIEW->mark_end_col;
         line = CURRENT_FILE->number_lines;
         break;
    case M_LINE:
         line = MARK_VIEW->mark_end_line;
         break;
    default:
         col = MARK_VIEW->mark_end_col;
         line = MARK_VIEW->mark_end_line;
         break;
   }
 /* work out if block boundary is not excluded */
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,line,CURRENT_FILE->number_lines);
 if (!IN_SCOPE(CURRENT_VIEW,curr))
   {
    display_error(46,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
    Sos_leftedge((CHARTYPE *)"");
 getyx(CURRENT_WINDOW,y,x);
 /*
  * Move to the line
  */
 save_compat = compatible_feel;
 compatible_feel = COMPAT_THE;
 sprintf((DEFCHAR*)cmd,":%ld",(long)line);
 rc = command_line(cmd,COMMAND_ONLY_FALSE);
 compatible_feel = save_compat;;
 /*
  * Move to the column, except if the block is a LINE block or
  * we are on command line.
  */
 if (MARK_VIEW->mark_type != M_LINE
 &&  CURRENT_VIEW->current_window != WINDOW_COMMAND)
    execute_move_cursor(col-1);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos blockstart - move cursor to start of marked block

SYNTAX
     SOS BLOCKStart

DESCRIPTION
     The SOS BLOCKSTART command moves the cursor to the starting line
     and column of the marked block.  If the cursor is on the command 
     line, the first line of the marked block becomes the current line.

     If no marked block is in the current file, an error is displayed.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS BLOCKEND>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_blockstart(CHARTYPE *params)
#else
short Sos_blockstart(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 short rc=RC_OK;
 LINE *curr=NULL;
 LENGTHTYPE col=0;
 LINETYPE line=0L;
 short save_compat=0;
 CHARTYPE cmd[20];
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_blockstart");
 if (MARK_VIEW != CURRENT_VIEW)
   {
    display_error(45,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 switch(MARK_VIEW->mark_type)
   {
    case M_COLUMN:
         col = MARK_VIEW->mark_start_col;
         line = 1;
         break;
    case M_LINE:
         line = MARK_VIEW->mark_start_line;
         break;
    default:
         col = MARK_VIEW->mark_start_col;
         line = MARK_VIEW->mark_start_line;
         break;
   }
 /* work out if block boundary is not excluded */
 curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,line,CURRENT_FILE->number_lines);
 if (!IN_SCOPE(CURRENT_VIEW,curr))
   {
    display_error(46,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
    Sos_leftedge((CHARTYPE *)"");
 getyx(CURRENT_WINDOW,y,x);
 /*
  * Move to the line first
  */
 save_compat = compatible_feel;
 compatible_feel = COMPAT_THE;
 sprintf((DEFCHAR*)cmd,":%ld",(long)line);
 rc = command_line(cmd,COMMAND_ONLY_FALSE);
 compatible_feel = save_compat;;
 /*
  * Move to the column, except if the block is a LINE block or
  * we are on command line.
  */
 if (MARK_VIEW->mark_type != M_LINE
 &&  CURRENT_VIEW->current_window != WINDOW_COMMAND)
    execute_move_cursor(col-1);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos bottomedge - move cursor to bottom edge of FILEAREA

SYNTAX
     SOS BOTTOMEdge

DESCRIPTION
     The SOS BOTTOMEDGE command moves the cursor to the last 
     enterable line in the <filearea> or <prefix area>. If the cursor
     is on the command line, the cursor moves to the first 
     enterable line of the <filearea>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Comaptible.

SEE ALSO
     <SOS TOPEDGE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_bottomedge(CHARTYPE *params)
#else
short Sos_bottomedge(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short y=0,x=0,row=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_bottomedge");
 getyx(CURRENT_WINDOW,y,x);
/*---------------------------------------------------------------------*/
/* Get the last enterable row. If an error, stay where we are...       */
/*---------------------------------------------------------------------*/
 if (find_last_focus_line(&row) != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* For each window determine what needs to be done...                  */
/*---------------------------------------------------------------------*/
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_COMMAND:
         if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) != PREFIX_LEFT)
            x += CURRENT_VIEW->prefix_width;
         CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         CURRENT_VIEW->current_window = WINDOW_FILEAREA;
         wmove(CURRENT_WINDOW,row,x);
         break;
    case WINDOW_FILEAREA:
    case WINDOW_PREFIX:
            if (row != y)                            /* different rows */
              {
               post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
               CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
               pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
               wmove(CURRENT_WINDOW,row,x);
              }
            break;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos cuadelback - delete the character to the left of the cursor

SYNTAX
     SOS CUADELBAck

DESCRIPTION
     The SOS CUADELBACK command deletes the character to the right of the
     current cursor position.  It differs from <SOS DELBACK> in the case
     when the cursor is in the first column of the file and in the 
     FILEAREA. Then, the cursor first moves to the last character of the 
     previous line, and deletes this character.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SOS DELBACK>, <SOS CUADELCHAR>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_cuadelback(CHARTYPE *params)
#else
short Sos_cuadelback(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_cuadelback");
   rc = sosdelback( TRUE );
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     sos cuadelchar - delete character under cursor

SYNTAX
     SOS CUADELChar

DESCRIPTION
     The SOS CUADELCHAR command deletes the character under the cursor.
     Text to the right is shifted to the left.
     It differs from <SOS DELCHAR> in the case when the cursor is after
     the last character of the line and in the FILEAREA. Then, the 
     next line is joined with the current line.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SOS CURDELBACK>, <SOS DELCHAR>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_cuadelchar(CHARTYPE *params)
#else
short Sos_cuadelchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_cuadelchar");
   rc = sosdelchar( TRUE );
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     sos current - move cursor to current line

SYNTAX
     SOS CURRent

DESCRIPTION
     The SOS CURRENT command moves the cursor to the current column
     of the cursor line from any window.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_current(CHARTYPE *params)
#else
short Sos_current(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short x=0,y=0;
 bool same_line=TRUE;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_current");
 getyx(CURRENT_WINDOW_FILEAREA,y,x);
 switch (CURRENT_VIEW->current_window)
   {
    case WINDOW_FILEAREA:
         if (CURRENT_VIEW->focus_line != CURRENT_VIEW->current_line)
           {
            post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
            CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
            same_line = FALSE;
           }
         y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                                    CURRENT_VIEW->current_row);
         wmove(CURRENT_WINDOW_FILEAREA,y,x);
         if (!same_line)
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         break;
    case WINDOW_PREFIX:
    case WINDOW_COMMAND:
         CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
         y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                                    CURRENT_VIEW->current_row);
         CURRENT_VIEW->current_window = WINDOW_FILEAREA;
         wmove(CURRENT_WINDOW_FILEAREA,y,x);
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         break;
    default:
         break;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos cursoradj - move first non-blank character to cursor

SYNTAX
     SOS CURSORAdj

DESCRIPTION
     The SOS CURSORADJ command moves text in the <focus line> so that
     the first non-blank character appears under the cursor position.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS CURSORSHIFT>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_cursoradj(CHARTYPE *params)
#else
short Sos_cursoradj(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short num_cols=0,first_non_blank_col=0,col=0,rc=RC_OK;
   unsigned short x=0,y=0;

   TRACE_FUNCTION("commsos.c: Sos_cursoradj");
   getyx(CURRENT_WINDOW,y,x);
   switch (CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         if (FOCUS_TOF || FOCUS_BOF)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         col = x + CURRENT_VIEW->verify_col - 1;
         first_non_blank_col = strzne(rec,' ');
         if (first_non_blank_col == (-1))
            first_non_blank_col = 0;
         num_cols = first_non_blank_col - col;
         if (num_cols < 0)
            rc = execute_shift_command(FALSE,-num_cols,CURRENT_VIEW->focus_line,1,FALSE,TARGET_UNFOUND,TRUE,FALSE);
         else
            if (num_cols > 0)
               rc = execute_shift_command(TRUE,num_cols,CURRENT_VIEW->focus_line,1,FALSE,TARGET_UNFOUND,TRUE,FALSE);
         break;
      default:
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos cursorshift - move text to right of cursor to cursor

SYNTAX
     SOS CURSORSHIFT

DESCRIPTION
     The SOS CURSORSHIFT command moves text in the <focus line> so that
     the first non-blank character to the right of the cursor is
     shifted to under the cursor position.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SOS CURSORADJ>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_cursorshift(CHARTYPE *params)
#else
short Sos_cursorshift(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short num_cols=0,first_non_blank_col=0,col=0,rc=RC_OK;
   unsigned short x=0,y=0;

   TRACE_FUNCTION("commsos.c: Sos_cursorshift");
   getyx(CURRENT_WINDOW,y,x);
   switch (CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         if (FOCUS_TOF || FOCUS_BOF)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         col = x + CURRENT_VIEW->verify_col - 1;
         first_non_blank_col = col + strzne(rec+col,' ');
         num_cols = first_non_blank_col - col;
         if ( num_cols > 0 )
         {
            memdeln(rec,col,rec_len,num_cols);
            rec_len -= num_cols;
         }
         break;
      default:
         break;
   }
   /*
    * If we changed anything, redisplay the screen.           
    */
   if (num_cols > 0)
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos delback - delete the character to the left of the cursor

SYNTAX
     SOS DELBAck

DESCRIPTION
     The SOS DELBACK command moves the cursor one character to the left
     and deletes the character now under the cursor.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS DELCHAR>, <SOS CUADELCHAR>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_delback(CHARTYPE *params)
#else
short Sos_delback(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_delback");
   rc = sosdelback( FALSE );
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     sos delchar - delete character under cursor

SYNTAX
     SOS DELChar

DESCRIPTION
     The SOS DELCHAR command deletes the character under the cursor.
     Text to the right is shifted to the left.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS DELBACK>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_delchar(CHARTYPE *params)
#else
short Sos_delchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_delchar");
   rc = sosdelchar( FALSE );
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     sos delend - delete to end of line

SYNTAX
     SOS DELEnd

DESCRIPTION
     The SOS DELEND command deletes all characters from the current
     column to the end of line.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_delend(CHARTYPE *params)
#else
short Sos_delend(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
   unsigned short col=0,x=0,y=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_delend");
   getyx(CURRENT_WINDOW,y,x);
   switch (CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
/*---------------------------------------------------------------------*/
/* If running in read-only mode and an attempt is made to execute this */
/* command in the MAIN window, then error...                           */
/*---------------------------------------------------------------------*/
         if (ISREADONLY(CURRENT_FILE))
         {
            display_error(56,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         col = x + CURRENT_VIEW->verify_col - 1;
         for (i=col;i<max_line_length;i++)
            rec[i] = ' ';
         if (rec_len > col)
            rec_len = col;
         my_wclrtoeol(CURRENT_WINDOW);
         break;
      case WINDOW_COMMAND:
         for (i=x;i<COLS;i++)
            cmd_rec[i] = ' ';
         if (cmd_rec_len > x)
            cmd_rec_len = x;
         my_wclrtoeol(CURRENT_WINDOW);
         break;
      case WINDOW_PREFIX:
         if (x < pre_rec_len)
         {
            prefix_changed = TRUE;
            for (i=x;i<CURRENT_VIEW->prefix_width-CURRENT_VIEW->prefix_gap;i++)
               pre_rec[i] = ' ';
            if (pre_rec_len > x)
               pre_rec_len = x;
            my_wclrtoeol(CURRENT_WINDOW);
         }
         break;
      default:
         break;
   }
/*---------------------------------------------------------------------*/
/* If the character being deleted is on a line which is in the marked  */
/* block, and we are in the filearea, redisplay the screen.            */
/*---------------------------------------------------------------------*/
   if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
      if ((CURRENT_VIEW == MARK_VIEW
      &&  CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line
      &&  CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)
      || (CURRENT_FILE->colouring && CURRENT_FILE->parser))
      {
         build_screen(current_screen);
         display_screen(current_screen);
      }
   }
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     sos delline - delete focus line

SYNTAX
     SOS DELLine

DESCRIPTION
     The SOS DELLINE command deletes the <focus line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SOS LINEDEL>, <SOS ADDLINE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_delline(CHARTYPE *params)
#else
short Sos_delline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short x=0,y=0;
 LINETYPE true_line=0L,lines_affected=0L;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_delline");
 if (CURRENT_VIEW->current_window == WINDOW_FILEAREA
 ||  CURRENT_VIEW->current_window == WINDOW_PREFIX)
   {
    getyx(CURRENT_WINDOW,y,x);
    if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE
    &&  !CURRENT_VIEW->scope_all)
      {
       display_error(38,(CHARTYPE *)"",FALSE);
       TRACE_RETURN();
       return(RC_INVALID_ENVIRON);
      }
   }
 true_line = get_true_line(FALSE);
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 rc = rearrange_line_blocks(COMMAND_DELETE,SOURCE_COMMAND,true_line,
                            true_line,true_line,1,CURRENT_VIEW,CURRENT_VIEW,FALSE,
                            &lines_affected);
#if 1
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
    if (curses_started)
       getyx(CURRENT_WINDOW,y,x);
    CURRENT_VIEW->focus_line = get_focus_line_in_view(current_screen,CURRENT_VIEW->focus_line,y);
    y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,CURRENT_VIEW->current_row);
    if (curses_started)
       wmove(CURRENT_WINDOW,y,x);
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   }
#endif
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos delword - delete word at or right of cursor

SYNTAX
     SOS DELWord

DESCRIPTION
     The SOS DELWORD command deletes the word at or to the right
     of the current cursor position and any spaces following the 
     word.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_delword(CHARTYPE *params)
#else
short Sos_delword(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short rc=RC_OK;
 LENGTHTYPE first_col=0,last_col=0;
 unsigned short x=0,y=0,temp_rec_len=0;
 short num_cols=0,left_col=0;
 CHARTYPE *temp_rec=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_delword");
/*---------------------------------------------------------------------*/
/* This function is not applicable to the PREFIX window.               */
/*---------------------------------------------------------------------*/
 getyx(CURRENT_WINDOW,y,x);
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_PREFIX:
         display_error(38,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
         break;
    case WINDOW_FILEAREA:
/*---------------------------------------------------------------------*/
/* If running in read-only mode and an attempt is made to execute this */
/* command in the MAIN window, then error...                           */
/*---------------------------------------------------------------------*/
         if (ISREADONLY(CURRENT_FILE))
           {
            display_error(56,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
           }
         if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
           {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
           }
         temp_rec = rec;
         temp_rec_len = rec_len;
         left_col = CURRENT_VIEW->verify_col-1;
         break;
    case WINDOW_COMMAND:
         temp_rec = (CHARTYPE *)cmd_rec;
         temp_rec_len = cmd_rec_len;
         left_col = 0;
         break;
   }
 if (get_word(temp_rec,temp_rec_len,x+left_col,&first_col,&last_col) == 0)
   {
    TRACE_RETURN();
    return(0);
   }
/*---------------------------------------------------------------------*/
/* Delete from the field the number of columns calculated above        */
/* and adjust the appropriate record length.                           */
/*---------------------------------------------------------------------*/
 num_cols = last_col-first_col+1;
 memdeln(temp_rec,first_col,temp_rec_len,num_cols);
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_FILEAREA:
         rec_len -= num_cols;
         rc = execute_move_cursor(first_col);
         build_screen(current_screen); 
         display_screen(current_screen);
         break;
    case WINDOW_COMMAND:
         cmd_rec_len -= num_cols;
         wmove(CURRENT_WINDOW,y,first_col);
         for (i=0;i<num_cols;i++)
            my_wdelch(CURRENT_WINDOW);
         break;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos doprefix - execute any pending prefix commands

SYNTAX
     SOS DOPREfix

DESCRIPTION
     The SOS DOPREFIX command executes any pending prefix commands.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_doprefix(CHARTYPE *params)
#else
short Sos_doprefix(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_doprefix");
/*---------------------------------------------------------------------*/
/*                                                                     */
/*---------------------------------------------------------------------*/
 rc = execute_prefix_commands();
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos edit - edit a file from directory list

SYNTAX
     SOS EDIT

DESCRIPTION
     The SOS EDIT command allows the user to edit a file, chosen from
     a directory list (the file DIR.DIR).

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible with default definition for Alt-X key.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_edit(CHARTYPE *params)
#else
short Sos_edit(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   LINE *curr=NULL;
   CHARTYPE edit_fname[MAX_FILE_NAME];
   unsigned short y=0,x=0;
   short rc=RC_OK;
   LINETYPE true_line=0L;
   CHARTYPE *lname=NULL,*fname=NULL;
  
   TRACE_FUNCTION("commsos.c: Sos_edit");
   /*
    * If the current file is not the special DIR.DIR file exit.
    */
   if (CURRENT_FILE->pseudo_file != PSEUDO_DIR)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * Determine which line contains a vaild file to edit. TOF and EOF are
    * invalid positions.
    */
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
   {
      if (CURRENT_TOF || CURRENT_BOF)
      {
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
      }
      true_line = CURRENT_VIEW->current_line;
   }
   else
   {
      getyx(CURRENT_WINDOW,y,x);
      if (FOCUS_TOF || FOCUS_BOF)
      {
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
      }
      true_line = CURRENT_VIEW->focus_line;
   }
   /*
    * Find the current LINE pointer for the focus_line.
    */
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   /*
    * Ensure that the line is long enough to have a filename on it...
    */
   if (rec_len <= file_start)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }

   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   /*
    * Validate that the supplied file is valid.
    */
#if defined(MULTIPLE_PSEUDO_FILES)
   strcpy((DEFCHAR *)edit_fname,(DEFCHAR *)CURRENT_FILE->fpath);
   strcat((DEFCHAR *)edit_fname,(DEFCHAR *)curr->line+file_start);
# ifdef FILENAME_LENGTH
   edit_fname[(strlen((DEFCHAR *)CURRENT_FILE->fpath)+curr->filename_length)-1] = '\0';
# else
   edit_fname[(strlen((DEFCHAR *)CURRENT_FILE->fpath)+curr->length)-1] = '\0';
# endif
#else
   strcpy((DEFCHAR *)edit_fname,(DEFCHAR *)dir_path);
   fname = curr->line+file_start;
   if (*(curr->line) == 'l')
   {
      /*
       * We have a symbolic link.  Get the "real" file if there is one
       * ie. the string AFTER "->" is the "real" file name.
       */
      lname = (CHARTYPE*)strstr((DEFCHAR*)fname," -> ");
      if (lname != NULL)
      {
         if (strlen((DEFCHAR*)lname) > 4)
         {
            fname = lname + 4*sizeof(CHARTYPE);
            if (*fname == '/')  /* symbolic link is absolute...*/
               strcpy((DEFCHAR *)edit_fname,"");
         }
      }
   }
   strcat((DEFCHAR *)edit_fname,(DEFCHAR *)fname);
#endif
  
#if !defined(MULTIPLE_PSEUDO_FILES)
   if ((rc = splitpath(edit_fname)) != RC_OK)
   {
      display_error(10,edit_fname,FALSE);
      TRACE_RETURN();
      return(rc);
   }
   strcpy((DEFCHAR *)edit_fname,(DEFCHAR *)sp_path);
   strcat((DEFCHAR *)edit_fname,(DEFCHAR *)sp_fname);
#endif
   /*
    * Edit the file.
    */
   rc = EditFile( edit_fname, FALSE );
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos endchar - move cursor to end of focus line

SYNTAX
     SOS ENDChar

DESCRIPTION
     The SOS ENDCHAR command moves the cursor to the position after
     the last character displayed in the current window.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS STARTENDCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_endchar(CHARTYPE *params)
#else
short Sos_endchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short rc=RC_OK;
   unsigned short x=0,y=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_endchar");
   getyx(CURRENT_WINDOW,y,x);
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_PREFIX:
         wmove(CURRENT_WINDOW,y,min(pre_rec_len,CURRENT_VIEW->prefix_width-CURRENT_VIEW->prefix_gap-1));
         rc = RC_OK;
         break;
      case WINDOW_COMMAND:
         wmove(CURRENT_WINDOW,y,cmd_rec_len);
         rc = RC_OK;
         break;
      case WINDOW_FILEAREA:
         rc = execute_move_cursor(rec_len);
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos execute - move cursor to command line and execute command

SYNTAX
     SOS EXecute

DESCRIPTION
     The SOS EXECUTE command moves the cursor to the <command line>
     and executes any command that is displayed there.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

STATUS
     Complete. 
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_execute(CHARTYPE *params)
#else
short Sos_execute(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
   short rc=RC_OK;
   bool save_in_macro;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commsos.c: Sos_execute");

   if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
      rc = THEcursor_cmdline(1);
   if (rc == RC_OK)
   {
      save_in_macro = in_macro;
      in_macro = FALSE;
      for (i=0;i<cmd_rec_len;i++)
         temp_cmd[i] = cmd_rec[i];
      temp_cmd[cmd_rec_len] = '\0';
      MyStrip(temp_cmd,'L',' ');
      add_command(temp_cmd);
      rc = command_line(temp_cmd,COMMAND_ONLY_FALSE);
      in_macro = save_in_macro;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos firstchar - move cursor to first non-blank of field

SYNTAX
     SOS FIRSTCHar

DESCRIPTION
     The SOS FIRSTCHAR command moves the cursor to the first
     non-blank character of the cursor field

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SOS FIRSTCOL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_firstchar(CHARTYPE *params)
#else
short Sos_firstchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK,new_col=0;
 unsigned short y=0,x=0;
 LINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_firstchar");
/*---------------------------------------------------------------------*/
/* For the command line and prefix area, just go to the first column...*/
/*---------------------------------------------------------------------*/
 getyx(CURRENT_WINDOW,y,x);
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND
 ||  CURRENT_VIEW->current_window == WINDOW_PREFIX)
   {
    wmove(CURRENT_WINDOW,y,0);
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* For the filearea, we have to do a bit more...                       */
/*---------------------------------------------------------------------*/
 curr = CURRENT_SCREEN.sl[y].current;
 new_col = memne(curr->line,' ',curr->length);
 if (new_col == (-1))
    new_col = 0;
 rc = execute_move_cursor(new_col);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos firstcol - move cursor to first column of field

SYNTAX
     SOS FIRSTCOl

DESCRIPTION
     The SOS FIRSTCOL command moves the cursor to the first
     column of the <cursor field>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SOS FIRSTCHAR>, <SOS LASTCOL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_firstcol(CHARTYPE *params)
#else
short Sos_firstcol(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_firstcol");
 getyx(CURRENT_WINDOW,y,x);
/*---------------------------------------------------------------------*/
/* For the command line, just go to the first column...                */
/*---------------------------------------------------------------------*/
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_COMMAND:
    case WINDOW_PREFIX:
         wmove(CURRENT_WINDOW,y,0);
         break;
    case WINDOW_FILEAREA:
         if (CURRENT_VIEW->verify_col != 1)
            rc = execute_move_cursor(0);
         wmove(CURRENT_WINDOW,y,0);
         break;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos instab - shift text to next tab column

SYNTAX
     SOS INSTAB

DESCRIPTION
     The SOS INSTAB command shifts all text from the current cursor
     position in the <filearea> to the next tab column.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SET TABS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_instab(CHARTYPE *params)
#else
short Sos_instab(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 short rc=RC_OK;
 LENGTHTYPE col=0,tabcol=0;
 register int i=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_instab");
 if (CURRENT_VIEW->current_window != WINDOW_FILEAREA)
   {
    TRACE_RETURN();
    return(rc);
   }
 getyx(CURRENT_WINDOW,y,x);
 col = x + CURRENT_VIEW->verify_col;
 for (i=0;i<CURRENT_VIEW->numtabs;i++)
   {
    if (col < CURRENT_VIEW->tabs[i])
      {
       tabcol = CURRENT_VIEW->tabs[i];
       break;
      }
   }
 if (tabcol == 0) /* after last tab column or on a tab column */
   {
    TRACE_RETURN();
    return(rc);
   }
 for (i=0;i<tabcol-col;i++)
    meminschr(rec,' ',col-1,max_line_length,rec_len++);
#if 1
 rec_len = min(max_line_length,rec_len);
#endif

 Sos_tabf((CHARTYPE *)"nochar");
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos lastcol - move cursor to last column of field

SYNTAX
     SOS LASTCOl

DESCRIPTION
     The SOS LASTCOL command moves the cursor to the last column
     of the <cursor field>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SOS FIRSTCOL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_lastcol(CHARTYPE *params)
#else
short Sos_lastcol(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_lastcol");
#if 0
/*---------------------------------------------------------------------*/
/* For the command line and filearea, just go to the last column...    */
/*---------------------------------------------------------------------*/
 getyx(CURRENT_WINDOW,y,x);
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND
 ||  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    x = getmaxx(CURRENT_WINDOW)-1;
    wmove(CURRENT_WINDOW,y,x);
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* For the prefix area we have to do a bit more...                     */
/*---------------------------------------------------------------------*/
 if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_RIGHT)
     x = CURRENT_VIEW->prefix_width - 1;
 else
     x = CURRENT_VIEW->prefix_width - CURRENT_VIEW->prefix_gap - 1;
 wmove(CURRENT_WINDOW,y,x);
#else

 getyx(CURRENT_WINDOW,y,x);
 x = getmaxx(CURRENT_WINDOW)-1;
 wmove(CURRENT_WINDOW,y,x);
#endif

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos leftedge - move cursor to left edge of window

SYNTAX
     SOS LEFTEdge

DESCRIPTION
     The SOS LEFTEDGE command moves the cursor to the leftmost edge
     of the <filearea> if not on the command line or to the leftmost
     edge of the command line if on the <command line>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SOS RIGHTEDGE>, <SOS PREFIX>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_leftedge(CHARTYPE *params)
#else
short Sos_leftedge(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_leftedge");
 getyx(CURRENT_WINDOW,y,x);
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
 wmove(CURRENT_WINDOW,y,0);
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     sos lineadd - add blank line after focus line

SYNTAX
     SOS LINEAdd

DESCRIPTION
     The SOS LINEADD command inserts a blank line in the file following
     the <focus line>. The cursor is placed in the column under the first
     non-blank in the <focus line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SOS ADDLINE>, <SOS LINEDEL>

STATUS
     Complete
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     sos linedel - delete focus line

SYNTAX
     SOS LINEDel

DESCRIPTION
     The SOS LINEDEL command deletes the <focus line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SOS DELLINE>, <SOS LINEADD>        

STATUS
     Complete
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     sos makecurr - make focus line the current line

SYNTAX
     SOS MAKECURR

DESCRIPTION
     The SOS MAKECURR command set the <current line> to the <focus line>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_makecurr(CHARTYPE *params)
#else
short Sos_makecurr(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_makecurr");
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
    rc = execute_makecurr(CURRENT_VIEW->focus_line);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos marginl - move cursor to the left margin column

SYNTAX
     SOS MARGINL

DESCRIPTION
     The SOS MARGINL command moves the cursor to the left margin
     column.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.
            Although, when issued from the command line, nothing
            happens.

SEE ALSO
     <SOS MARGINR>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_marginl(CHARTYPE *params)
#else
short Sos_marginl(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_marginl");
 if (Sos_leftedge((CHARTYPE *)"") == RC_OK)
    rc = execute_move_cursor(CURRENT_VIEW->margin_left-1);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos marginr - move cursor to the right margin column

SYNTAX
     SOS MARGINR

DESCRIPTION
     The SOS MARGINR command moves the cursor to the right margin
     column.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.
            Although, when issued from the command line, nothing
            happens.

SEE ALSO
     <SOS MARGINL>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_marginr(CHARTYPE *params)
#else
short Sos_marginr(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_marginr");
 if (Sos_leftedge((CHARTYPE *)"") == RC_OK)
    rc = execute_move_cursor(CURRENT_VIEW->margin_right-1);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos parindent - move cursor to the paragraph indent column

SYNTAX
     SOS PARINDent

DESCRIPTION
     The SOS PARINDENT command moves the cursor to the paragraph
     indent column.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.
            Although, when issued from the command line, nothing
            happens.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_parindent(CHARTYPE *params)
#else
short Sos_parindent(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 COLTYPE parindent=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_parindent");
 if (CURRENT_VIEW->margin_indent_offset_status)
    parindent = CURRENT_VIEW->margin_left + CURRENT_VIEW->margin_indent - 1;
 else
    parindent = CURRENT_VIEW->margin_indent - 1;
 if (Sos_leftedge((CHARTYPE *)"") == RC_OK)
    rc = execute_move_cursor(parindent);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos pastecmdline - copy contents of marked block to command line

SYNTAX
     SOS PASTECMDline

DESCRIPTION
     The SOS PASTECMDLINE command copies the contents of the marked
     block to the command line at the current cursor location.

     Marked blocks that span one line only are allowed to be pasted.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_pastecmdline(CHARTYPE *params)
#else
short Sos_pastecmdline(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short x=0,y=0;
 LINE *curr=NULL;
 LENGTHTYPE start_col=0,end_col=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_pastecmdline");
 if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
    display_error(38,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 if (MARK_VIEW == NULL)
   {
    display_error(44,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 if (MARK_VIEW->mark_start_line != MARK_VIEW->mark_end_line)
   {
    display_error(81,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 curr = lll_find(MARK_FILE->first_line,MARK_FILE->last_line,MARK_VIEW->mark_start_line,MARK_FILE->number_lines);
 if (MARK_VIEW->mark_type == M_LINE)
   {
    start_col = 0;
    end_col = curr->length-1;
   }
 else
   {
    start_col = MARK_VIEW->mark_start_col-1;
    end_col = MARK_VIEW->mark_end_col-1;
   }
 getyx(CURRENT_WINDOW,y,x);
 if (INSERTMODEx)
   {
    meminsmem(cmd_rec,curr->line+start_col,end_col-start_col+1,x,max_line_length,cmd_rec_len);
    cmd_rec_len = max(cmd_rec_len,x)+end_col-start_col+1;
   }
 else
   {
    memcpy(cmd_rec+x,curr->line+start_col,end_col-start_col+1);
    cmd_rec_len = max(x+end_col-start_col+1,cmd_rec_len);
   }

 if (curses_started
 &&  CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
   {
    wmove(CURRENT_WINDOW_COMMAND,0,0);
    my_wclrtoeol(CURRENT_WINDOW_COMMAND);
    put_string(CURRENT_WINDOW_COMMAND,0,0,cmd_rec,cmd_rec_len);
    wmove(CURRENT_WINDOW,y,x+end_col-start_col+1);
    clear_command = FALSE;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos prefix - move cursor to leftmost edge of prefix area

SYNTAX
     SOS PREfix

DESCRIPTION
     The SOS PREFIX command moves the cursor to the rightmost edge
     of the <prefix area>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SOS LEFTEDGE>, <SOS RIGHTEDGE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_prefix(CHARTYPE *params)
#else
short Sos_prefix(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_prefix");
/*---------------------------------------------------------------------*/
/* If the cursor is in the command line or there is no prefix on, exit.*/
/*---------------------------------------------------------------------*/
 if (CURRENT_VIEW->current_window == WINDOW_COMMAND
 ||  !CURRENT_VIEW->prefix)
   {
    TRACE_RETURN();
    return(RC_OK);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 getyx(CURRENT_WINDOW,y,x);
 if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
    CURRENT_VIEW->current_window = WINDOW_PREFIX;
#if 0
 if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_RIGHT)
    x = CURRENT_VIEW->prefix_gap;
 else
#endif
    x = 0;
 wmove(CURRENT_WINDOW,y,x);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos qcmnd - move cursor to command line and clear

SYNTAX
     SOS QCmnd

DESCRIPTION
     The SOS QCMND command moves the cursor to the first column of
     the <command line> and clears it.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SOS EXECUTE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_qcmnd(CHARTYPE *params)
#else
short Sos_qcmnd(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_qcmnd");
 if ((rc = THEcursor_cmdline(1)) == RC_OK)
   {
    if (CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
      {
       wmove(CURRENT_WINDOW_COMMAND,0,0);
       my_wclrtoeol(CURRENT_WINDOW_COMMAND);
      }
    memset(cmd_rec,' ',max_line_length);
    cmd_rec_len = 0;
  }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos rightedge - move cursor to right edge of window

SYNTAX
     SOS RIGHTEdge

DESCRIPTION
     The SOS RIGHTEDGE command moves the cursor to the rightmost edge
     of the <filearea> if not on the command line or to the rightmost
     edge of the command line if on the <command line>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

SEE ALSO
     <SOS LEFTEDGE>, <SOS PREFIX>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_rightedge(CHARTYPE *params)
#else
short Sos_rightedge(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short y=0,x=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_rightedge");
 getyx(CURRENT_WINDOW,y,x);
 if (CURRENT_VIEW->current_window == WINDOW_PREFIX)
    CURRENT_VIEW->current_window = WINDOW_FILEAREA;
 x = getmaxx(CURRENT_WINDOW)-1;
 wmove(CURRENT_WINDOW,y,x);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos settab - set a tab column at the cursor position

SYNTAX
     SOS SETTAB

DESCRIPTION
     The SOS SETTAB command sets a tab column at the position of the
     cursor in the <filearea>.  This command is ignored if issued 
     elsewhere.
     If a tab column is already set at the cursor position, the tab 
     column is cleared.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Kedit does not toggle the tab column, but only sets it.

SEE ALSO
     <SET TABS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_settab(CHARTYPE *params)
#else
short Sos_settab(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define SETTAB_INSERT 0
#define SETTAB_APPEND 1
#define SETTAB_REMOVE 2
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 short rc=RC_OK;
 LENGTHTYPE col=0;
 LENGTHTYPE max_tab_col=0;
 int action=SETTAB_INSERT;
 int i=0,j=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_settab");
 if (CURRENT_VIEW->current_window != WINDOW_FILEAREA)
   {
    TRACE_RETURN();
    return(rc);
   }
 getyx(CURRENT_WINDOW,y,x);
 col = x + CURRENT_VIEW->verify_col;
 for (i=0;i<CURRENT_VIEW->numtabs;i++)
   {
    if (CURRENT_VIEW->tabs[i] == col)
      {
       action = SETTAB_REMOVE;
       break;
      }
    if (max_tab_col < CURRENT_VIEW->tabs[i])
       max_tab_col = CURRENT_VIEW->tabs[i];
   }
 if (action != SETTAB_REMOVE
 &&  col > max_tab_col)
    action = SETTAB_APPEND;

 switch(action)
   {
    case SETTAB_REMOVE:
         for (i=0,j=0;i<CURRENT_VIEW->numtabs;i++,j++)
           {
            if (CURRENT_VIEW->tabs[i] == col)
               j++;
            if (j < CURRENT_VIEW->numtabs)
               CURRENT_VIEW->tabs[i] = CURRENT_VIEW->tabs[j];
           }
         CURRENT_VIEW->numtabs--;
         CURRENT_VIEW->tabsinc = 0;
         break;
    case SETTAB_APPEND:
         if (CURRENT_VIEW->numtabs < MAX_NUMTABS)
           {
            CURRENT_VIEW->tabs[CURRENT_VIEW->numtabs] = col;
            CURRENT_VIEW->numtabs++;
            CURRENT_VIEW->tabsinc = 0;
           }
         else
           {
            display_error(79,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_ENVIRON;
           }
         break;
    default:
         if (CURRENT_VIEW->numtabs < MAX_NUMTABS)
           {
            for (i=0;i<CURRENT_VIEW->numtabs;i++)
              {
               if (col < CURRENT_VIEW->tabs[i])
                 {
                  for (j=CURRENT_VIEW->numtabs-1;j>i;j--)
                     CURRENT_VIEW->tabs[j] = CURRENT_VIEW->tabs[j-1];
                  CURRENT_VIEW->tabs[i] = col;
                  break;
                 }
              }
            CURRENT_VIEW->numtabs++;
            CURRENT_VIEW->tabsinc = 0;
           }
         else
           {
            display_error(79,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_ENVIRON;
           }
          break;
   }
 build_screen(current_screen);
 display_screen(current_screen);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos startendchar - move cursor to end/start of focus line

SYNTAX
     SOS STARTENDChar

DESCRIPTION
     The SOS STARTENDCHAR command moves the cursor to the first character
     displayed in the <cursor field>, if the cursor is after the last
     character displayed in the <cursor field>, or to the position after
     the last character displayed in the <cursor field>, if the cursor is
     anywhere else.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SOS ENDCHAR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_startendchar(CHARTYPE *params)
#else
short Sos_startendchar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_startendchar");
 getyx(CURRENT_WINDOW,y,x);
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_PREFIX:
         if (x >= pre_rec_len)
            wmove(CURRENT_WINDOW,y,0);
         else
            wmove(CURRENT_WINDOW,y,pre_rec_len);
         break;
    case WINDOW_COMMAND:
         if (x >= cmd_rec_len)
            wmove(CURRENT_WINDOW,y,0);
         else
            wmove(CURRENT_WINDOW,y,cmd_rec_len);
         break;
    case WINDOW_FILEAREA:
         if (x + CURRENT_VIEW->verify_col > min(rec_len,CURRENT_VIEW->verify_end))
            rc = Sos_firstcol((CHARTYPE *)"");
         else
            rc = Sos_endchar((CHARTYPE *)"");
         break;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos tabb - move cursor to previous tab stop

SYNTAX
     SOS TABB

DESCRIPTION
     The SOS TABB command causes the cursor to move to the previous tab
     column as set by the <SET TABS> command.
     If the resulting column is beyond the left hand edge of the main
     window, the window will scroll half a window.

COMPATIBILITY
     XEDIT: Does not allow arguments.
     KEDIT: Compatible. See below.
     Does not line tab to next line if before the left hand tab column.

SEE ALSO
     <SET TABS>, <SOS TABF>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_tabb(CHARTYPE *params)
#else
short Sos_tabb(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 LENGTHTYPE prev_tab_col=0,current_col=0;
 COLTYPE new_screen_col=0;
 LENGTHTYPE new_verify_col=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_tabb");
 getyx(CURRENT_WINDOW,y,x);
/*---------------------------------------------------------------------*/
/* Determine action depending on current window...                     */
/*---------------------------------------------------------------------*/
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_PREFIX:
         TRACE_RETURN();
         return(RC_OK);
         break;
    case WINDOW_FILEAREA:
         current_col = x+CURRENT_VIEW->verify_col;
         break;
    case WINDOW_COMMAND:
         current_col = x+1;
         break;
   }
/*---------------------------------------------------------------------*/
/* First determine the next tab stop column...                         */
/*---------------------------------------------------------------------*/
 prev_tab_col = find_prev_tab_col(current_col);
/*---------------------------------------------------------------------*/
/* If no prev tab column, stay where we are and return...              */
/*---------------------------------------------------------------------*/
 if (prev_tab_col == 0)
   {
    TRACE_RETURN();
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* For all windows, if the new cursor position does not exceed the     */
/* right edge, move there.                                             */
/*---------------------------------------------------------------------*/
 prev_tab_col--;                               /* zero base the column */

#ifdef VERSHIFT
 rc = execute_move_cursor(prev_tab_col);
#else
 calculate_new_column(x,CURRENT_VIEW->verify_col,prev_tab_col,&new_screen_col,&new_verify_col);
 if (CURRENT_VIEW->verify_col != new_verify_col
 &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    CURRENT_VIEW->verify_col = new_verify_col;
    build_screen(current_screen); 
    display_screen(current_screen);
   }
 wmove(CURRENT_WINDOW,y,new_screen_col);
#endif

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos tabf - move cursor to next tab stop

SYNTAX
     SOS TABf

DESCRIPTION
     The SOS TABF command causes the cursor to move to the next tab column
     as set by the <SET TABS> command.
     If the resulting column is beyond the right hand edge of the main
     window, the window will scroll half a window.

COMPATIBILITY
     XEDIT: Does not allow arguments.
     KEDIT: Compatible. See below.
     Does not line tab to next line if after the right hand tab column.

SEE ALSO
     <SET TABS>, <SOS TABB>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_tabf(CHARTYPE *params)
#else
short Sos_tabf(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 LENGTHTYPE next_tab_col=0,current_col=0;
 COLTYPE new_screen_col=0;
 LENGTHTYPE new_verify_col=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_tabf");
/*---------------------------------------------------------------------*/
/* If the actual tab character is to be display then exit so that      */
/* editor() can process it as a raw key.                               */
/* Ignore this test if the parameter 'nochar' is passed.  The command  */
/* SOS INSTAB is the only way that 'nochar' can be passed; you cannot  */
/* "DEFINE akey SOS TABF NOCHAR"!!                                     */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR*)params,"nochar") != 0)
   {
    if (INSERTMODEx && tabkey_insert == 'C')
      {
       TRACE_RETURN();
       return(RAW_KEY);
      }
    if (!INSERTMODEx && tabkey_overwrite == 'C')
      {
       TRACE_RETURN();
       return(RAW_KEY);
      }
   }
 getyx(CURRENT_WINDOW,y,x);
/*---------------------------------------------------------------------*/
/* Determine action depending on current window...                     */
/*---------------------------------------------------------------------*/
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_PREFIX:
         TRACE_RETURN();
         return(RC_OK);
         break;
    case WINDOW_FILEAREA:
         current_col = x+CURRENT_VIEW->verify_col;
         break;
    case WINDOW_COMMAND:
         current_col = x+1;
         break;
   }
/*---------------------------------------------------------------------*/
/* First determine the next tab stop column...                         */
/*---------------------------------------------------------------------*/
 next_tab_col = find_next_tab_col(current_col);
/*---------------------------------------------------------------------*/
/* If no next tab column, stay where we are and return...              */
/*---------------------------------------------------------------------*/
 if (next_tab_col == 0)
   {
    TRACE_RETURN();
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* Check for going past end of line - max_line_length                  */
/*---------------------------------------------------------------------*/
 if (next_tab_col > max_line_length)
   {
    TRACE_RETURN();
    return(RC_TRUNCATED);
   }
/*---------------------------------------------------------------------*/
/* For all windows, if the new cursor position does not exceed the     */
/* right edge, move there.                                             */
/*---------------------------------------------------------------------*/
 next_tab_col--;                               /* zero base the column */

#ifdef VERSHIFT
 rc = execute_move_cursor(next_tab_col);
#else
 calculate_new_column(x,CURRENT_VIEW->verify_col,next_tab_col,&new_screen_col,&new_verify_col);
 if (CURRENT_VIEW->verify_col != new_verify_col
 &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    CURRENT_VIEW->verify_col = new_verify_col;
    build_screen(current_screen); 
    display_screen(current_screen);
   }
 wmove(CURRENT_WINDOW,y,new_screen_col);
#endif

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos tabfieldb - move cursor to previous enterable field

SYNTAX
     SOS TABFIELDB

DESCRIPTION
     The SOS TABFIELDB command causes the cursor to move to the first
     column of the current enterable field. If the cursor is already
     in the first column of the current field the cursor moves to the
     first column of the previous enterable field on the screen. 
     This command is intended to mimic the behaviour of the SHIFT-TAB 
     key on a 3270 terminal.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS TABFIELDF>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_tabfieldb(CHARTYPE *params)
#else
short Sos_tabfieldb(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 long save_where=0L,where=0L,what_current=0L,what_other=0L;
 unsigned short y=0,x=0,left_col=0;
 bool stay_in_current_field=FALSE;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_tabfieldb");
/*---------------------------------------------------------------------*/
/* Determine if the cursor is in the left-most column of the current   */
/* field...                                                            */
/*---------------------------------------------------------------------*/
 getyx(CURRENT_WINDOW,y,x);
#if 0
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_FILEAREA:
    case WINDOW_COMMAND:
         if (x != left_col)
            stay_in_current_field = TRUE;
         break;
    case WINDOW_PREFIX:
         if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_RIGHT)
            left_col = CURRENT_VIEW->prefix_gap;
         else
            left_col = 0;
         if (x != left_col)
            stay_in_current_field = TRUE;
         break;
   }
#else
 if (x != left_col)
    stay_in_current_field = TRUE;
#endif
/*---------------------------------------------------------------------*/
/* If the cursor was not in the left-most column of the current field, */
/* move it there now...                                                */
/*---------------------------------------------------------------------*/
 if (stay_in_current_field)
   {
    wmove(CURRENT_WINDOW,y,left_col);
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* ... otherwise determine which is the previous enterable filed and   */
/* move the cursor there.                                              */
/*---------------------------------------------------------------------*/
 save_where = where = where_now();
 what_current = what_current_now();
 what_other = what_other_now();
 while(1)
   {
    where = where_before(where,what_current,what_other);
    if (where == save_where)
       break;
    if (enterable_field(where))
       break;
   }
/*---------------------------------------------------------------------*/
/* If we can't go anywhere, stay where we are...                       */
/*---------------------------------------------------------------------*/
 if (where == save_where)
   {
    TRACE_RETURN();
    return(rc);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 rc = go_to_new_field(save_where,where);
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos tabfieldf - move cursor to next enterable field

SYNTAX
     SOS TABFIELDf

DESCRIPTION
     The SOS TABFIELDF command causes the cursor to move to the next 
     enterable field on the screen. This command is intended to
     mimic the behaviour of the TAB key on a 3270 terminal.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS TABFIELDB>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_tabfieldf(CHARTYPE *params)
#else
short Sos_tabfieldf(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 long save_where=0L,where=0L,what_current=0L,what_other=0L;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_tabfieldf");
 save_where = where = where_now();
 what_current = what_current_now();
 what_other = what_other_now();
 while(1)
   {
    where = where_next(where,what_current,what_other);
    if (where == save_where)
       break;
    if (enterable_field(where))
       break;
   }
/*---------------------------------------------------------------------*/
/* If we can't go anywhere, stay where we are...                       */
/*---------------------------------------------------------------------*/
 if (where == save_where)
   {
    TRACE_RETURN();
    return(rc);
   }
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
 rc = go_to_new_field(save_where,where);
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos tabwordb - move cursor to beginning of previous word

SYNTAX
     SOS TABWORDB

DESCRIPTION
     The SOS TABWORDB command causes the cursor to move to the first 
     character of the word to the left or to the start of the line if 
     no more words precede.
     If the resulting column is beyond the left hand edge of the 
     <filearea>, the window will scroll half a window.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS TABWORDF>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_tabwordb(CHARTYPE *params)
#else
short Sos_tabwordb(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
 short start_word_col=0;
 unsigned short word_break=0;
 CHARTYPE *temp_rec=NULL;
 register short i=0;
 bool blank_found=FALSE;
 short left_col=0;
 COLTYPE new_screen_col=0;
 LENGTHTYPE new_verify_col=0;
 short current_char_type=0;
 CHARTYPE this_char=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_tabwordb");
/*---------------------------------------------------------------------*/
/* This function is not applicable to the PREFIX window.               */
/*---------------------------------------------------------------------*/
 getyx(CURRENT_WINDOW,y,x);
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_PREFIX:
         display_error(38,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
         break;
    case WINDOW_FILEAREA:
         temp_rec = rec;
         left_col = CURRENT_VIEW->verify_col-1;
         break;
    case WINDOW_COMMAND:
         temp_rec = (CHARTYPE *)cmd_rec;
         left_col = 0;
         break;
   }
/*---------------------------------------------------------------------*/
/* Determine the start of the prior word, or go to the start of the    */
/* line if already at or before beginning of prior word.               */
/*---------------------------------------------------------------------*/
 word_break = 0;
 start_word_col = (-1);
 if (CURRENT_VIEW->word == 'N')
   {
    /*
     * Word break is non-blank
     */
    for (i=left_col+x;i>(-1);i--)
      {
       switch(word_break)
         {
          case 0:  /* still in current word */
               if (*(temp_rec+i) == ' ')
                  word_break++;
               break;
          case 1:  /* in first blank space */
               if (*(temp_rec+i) != ' ')
                  word_break++;
               break;
          case 2:  /* in previous word */
               if (*(temp_rec+i) == ' ')
                 {
                  start_word_col = i+1;
                  word_break++;
                 }
               break;
          default: /* should not get here */
               break;
         }
       if (word_break == 3)
          break;
      }
   }
 else
   {
    /*
     * Word break is non-blank
     */
    word_break = 0;
    this_char = *(temp_rec+left_col+x);
    current_char_type = my_isalphanum(this_char);
    for (i=left_col+x;i>(-1);i--)
      {
       switch(word_break)
         {
          case 0:  /* still in current word or blank */
               if (current_char_type == CHAR_SPACE
               &&  my_isalphanum(*(temp_rec+i)) != CHAR_SPACE)
                 {
                  word_break++;
                  current_char_type = my_isalphanum(*(temp_rec+i));
                  break;
                 }
               if (*(temp_rec+i) == ' ')
                 {
                  blank_found = TRUE;
                  break;
                 }
               if (blank_found)
                 {
                  word_break++;
                  current_char_type = my_isalphanum(*(temp_rec+i));
                  break;
                 }
               if (current_char_type == CHAR_ALPHANUM
               &&  my_isalphanum(*(temp_rec+i)) != CHAR_ALPHANUM)
                 {
                  word_break++;
                  current_char_type = my_isalphanum(*(temp_rec+i));
                  break;
                 }
               if (current_char_type == CHAR_OTHER
               &&  my_isalphanum(*(temp_rec+i)) != CHAR_OTHER)
                 {
                  word_break++;
                  current_char_type = my_isalphanum(*(temp_rec+i));
                  break;
                 }
               break;
          case 1:  /* now at end of previous word */
               if (current_char_type == CHAR_ALPHANUM
               &&  (my_isalphanum(*(temp_rec+i)) != CHAR_ALPHANUM
               ||  *(temp_rec+i) == ' '))
                 {
                  word_break++;
                  break;
                 }
               if (current_char_type == CHAR_OTHER
               &&  (my_isalphanum(*(temp_rec+i)) != CHAR_OTHER
               ||  *(temp_rec+i) == ' '))
                  {
                   word_break++;
                   break;
                  }
               break;
          default: /* should not get here */
               break;
         }
       if (word_break == 2)
         {
          start_word_col = i+1;
          break;
         }
      }
   }
 if (start_word_col == (-1))
    start_word_col = 0;

#ifdef VERSHIFT
 rc = execute_move_cursor(start_word_col);
#else
 calculate_new_column(x,CURRENT_VIEW->verify_col,start_word_col,&new_screen_col,&new_verify_col);
 if (CURRENT_VIEW->verify_col != new_verify_col
 &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    CURRENT_VIEW->verify_col = new_verify_col;
    build_screen(current_screen); 
    display_screen(current_screen);
   }
 wmove(CURRENT_WINDOW,y,new_screen_col);
#endif

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos tabwordf - move cursor to start of next word

SYNTAX
     SOS TABWORDf

DESCRIPTION
     The SOS TABWORDF command causes the cursor to move to the first 
     character of the next word to the right or to the end of the line 
     if no more words follow.
     If the resulting column is beyond the right hand edge of the
     <filearea>, the window will scroll half a window.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <SOS TABWORDB>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_tabwordf(CHARTYPE *params)
#else
short Sos_tabwordf(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0,temp_rec_len=0;
 short start_word_col=0,left_col=0;
 bool word_break=FALSE;
 short current_char_type=0;
 CHARTYPE *temp_rec=NULL;
 CHARTYPE this_char=0;
 register short i=0;
 COLTYPE new_screen_col=0;
 LENGTHTYPE new_verify_col=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_tabwordf");
/*---------------------------------------------------------------------*/
/* This function is not applicable to the PREFIX window.               */
/*---------------------------------------------------------------------*/
 getyx(CURRENT_WINDOW,y,x);
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_PREFIX:
         display_error(38,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
         break;
    case WINDOW_FILEAREA:
         temp_rec = rec;
         temp_rec_len = rec_len;
         left_col = CURRENT_VIEW->verify_col-1;
         break;
    case WINDOW_COMMAND:
         temp_rec = (CHARTYPE *)cmd_rec;
         temp_rec_len = cmd_rec_len;
         left_col = 0;
         break;
   }
/*---------------------------------------------------------------------*/
/* If we are after the last column of the line, then just ignore the   */
/* command and leave the cursor where it is.                           */
/*---------------------------------------------------------------------*/
 if ((x + left_col) > temp_rec_len)
   {
    TRACE_RETURN();
    return(RC_OK);
   }
/*---------------------------------------------------------------------*/
/* Determine the start of the next word, or go to the end of the line  */
/* if already at or past beginning of last word.                       */
/*---------------------------------------------------------------------*/
 word_break = FALSE;
 start_word_col = (-1);
 if (CURRENT_VIEW->word == 'N')
   {
    /*
     * Word break is non-blank
     */
    for (i=left_col+x;i<temp_rec_len;i++)
      {
       if (*(temp_rec+i) == ' ')
          word_break = TRUE;
       else
         {
          if (word_break)
            {
             start_word_col = i;
             break;
            }
         }
      }
   }
 else
   {
    /*
     * Word break is non-blank
     */
    this_char = *(temp_rec+left_col+x);
    current_char_type = my_isalphanum(this_char);
    for (i=left_col+x;i<temp_rec_len;i++)
      {
       switch(current_char_type)
         {
          case CHAR_SPACE:
               if (*(temp_rec+i) != ' ')
                  start_word_col = i;
               break;
          case CHAR_ALPHANUM:
               if (*(temp_rec+i) == ' ')
                 {
                  word_break = TRUE;
                  break;
                 }
               if (my_isalphanum(*(temp_rec+i)) != CHAR_ALPHANUM
               ||  word_break)
                  start_word_col = i;
               break;
          default:
               if (*(temp_rec+i) == ' ')
                 {
                  word_break = TRUE;
                  break;
                 }
               if (my_isalphanum(*(temp_rec+i)) == CHAR_ALPHANUM
               ||  word_break)
                  start_word_col = i;
               break;
         }
       if (start_word_col != (-1))
          break;
      }
   }
 if (start_word_col == (-1))
    start_word_col = temp_rec_len;

#ifdef VERSHIFT
 rc = execute_move_cursor(start_word_col);
#else
 calculate_new_column(x,CURRENT_VIEW->verify_col,start_word_col,&new_screen_col,&new_verify_col);
 if (CURRENT_VIEW->verify_col != new_verify_col
 &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
    CURRENT_VIEW->verify_col = new_verify_col;
    build_screen(current_screen); 
    display_screen(current_screen);
   }
 wmove(CURRENT_WINDOW,y,new_screen_col);
#endif

 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos topedge - move cursor to top edge of filearea

SYNTAX
     SOS TOPEdge

DESCRIPTION
     The SOS TOPEDGE command moves the cursor to the first enterable
     line in the <filearea> or <prefix area>. If the cursor is on the 
     <command line>, the cursor moves to the first enterable line of 
     the <filearea>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Comaptible.

SEE ALSO
     <SOS BOTTOMEDGE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_topedge(CHARTYPE *params)
#else
short Sos_topedge(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 unsigned short y=0,x=0,row=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_topedge");
 getyx(CURRENT_WINDOW,y,x);
/*---------------------------------------------------------------------*/
/* Get the last enterable row. If an error, stay where we are...       */
/*---------------------------------------------------------------------*/
 if (find_first_focus_line(&row) != RC_OK)
   {
    TRACE_RETURN();
    return(rc);
   }
/*---------------------------------------------------------------------*/
/* For each window determine what needs to be done...                  */
/*---------------------------------------------------------------------*/
 switch(CURRENT_VIEW->current_window)
   {
    case WINDOW_COMMAND:
         if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) != PREFIX_LEFT)
            x += CURRENT_VIEW->prefix_width;
         CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         CURRENT_VIEW->current_window = WINDOW_FILEAREA;
         wmove(CURRENT_WINDOW,row,x);
         break;
    case WINDOW_FILEAREA:
    case WINDOW_PREFIX:
            if (row != y)                            /* different rows */
              {
               post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
               CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
               pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
               wmove(CURRENT_WINDOW,row,x);
              }
            break;
   }
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     sos undo - undo changes to the current line

SYNTAX
     SOS UNDO

DESCRIPTION
     The SOS UNDO command causes the contents of the <focus line> (or the
     <command line>) to be reset to the contents before the cursor was
     positioned there.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. 

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Sos_undo(CHARTYPE *params)
#else
short Sos_undo(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 unsigned short x=0,y=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commsos.c: Sos_undo");
/*---------------------------------------------------------------------*/
/* No arguments are allowed; error if any are present.                 */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,(CHARTYPE *)params,FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 switch (CURRENT_VIEW->current_window)
   {
    case WINDOW_FILEAREA:
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         build_screen(current_screen); 
         display_screen(current_screen);
         break;
    case WINDOW_COMMAND:
         memset(cmd_rec,' ',max_line_length);
         cmd_rec_len = 0;
         wmove(CURRENT_WINDOW,0,0);
         my_wclrtoeol(CURRENT_WINDOW);
         break;
    case WINDOW_PREFIX:
         prefix_changed = TRUE;
         memset(pre_rec,' ',MAX_PREFIX_WIDTH);
         pre_rec_len = 0;
         getyx(CURRENT_WINDOW,y,x);
         wmove(CURRENT_WINDOW,y,0);
         my_wclrtoeol(CURRENT_WINDOW);
         break;
    default:
         break;
   }
 TRACE_RETURN();
 return(RC_OK);
}

/*
 * Internal functions...
 */
#ifdef HAVE_PROTO
static short sosdelback( bool cua )
#else
static short sosdelback( cua )
bool cua;
#endif
{
   unsigned short x=0,y=0;
   short rc=RC_OK;

   getyx( CURRENT_WINDOW, y, x );
   switch( CURRENT_VIEW->current_window )
   {
      case WINDOW_FILEAREA:
         /*
          * If running in read-only mode and an attempt is made to execute this
          * command in the MAIN window, then error...
          */
         if ( ISREADONLY(CURRENT_FILE) )
         {
            display_error(56,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         break;
      case WINDOW_COMMAND:
         if (x == 0)
         {
            TRACE_RETURN();
            return(RC_OK);
         }
         wmove(CURRENT_WINDOW,y,x-1);
         my_wdelch(CURRENT_WINDOW);
         if (x <= cmd_rec_len)
         {
            memdeln(cmd_rec,x-1,cmd_rec_len,1);
            cmd_rec_len--;
         }
         TRACE_RETURN();
         return(RC_OK);
         break;
      case WINDOW_PREFIX:
         if (x == 0)
         {
            TRACE_RETURN();
            return(RC_OK);
         }
         wmove(CURRENT_WINDOW,y,x-1);
         if ((x) <= pre_rec_len)
         {
            prefix_changed = TRUE;
            my_wdelch(CURRENT_WINDOW);
            memdeln(pre_rec,x-1,pre_rec_len,1);
            pre_rec_len --;
         }
         TRACE_RETURN();
         return(RC_OK);
         break;
      default:
         break;
   }

   /*
    * Remainder of processing is only for WINDOW_FILEAREA.
    */

   /*
    * If we have a marked CUA block, then move the cursor to the
    * start of the marked block, and delete the block. Don't attempt
    * to move the cursor left.
    */
   if ( INTERFACEx == INTERFACE_CUA
   &&  MARK_VIEW == CURRENT_VIEW
   &&  MARK_VIEW->mark_type == M_CUA )
   {
      ResetOrDeleteCUABlock( CUA_DELETE_BLOCK );
      TRACE_RETURN();
      return(RC_OK);
   }

   /*
    * If we are in the first column of the file...
    */
   if ( x == 0
   && CURRENT_VIEW->verify_start == CURRENT_VIEW->verify_col )
   {
      /*
       * Check if we have been passed "cua" as an optional parameter.
       * If so, and we are not on the first line of the
       * file, join with the line above
       */
      if ( cua
      &&   CURRENT_SCREEN.sl[y].line_number != 1L )
      {
         /*
          * advance to previous line
          * move to end of line
          * JOIN
          * only do this if the previous line is not excluded
          */
         if ( 1 )
         {
            advance_focus_line(-1);
            Sos_endchar( (CHARTYPE *)"");
            rc = execute_split_join( SPLTJOIN_JOIN, TRUE, TRUE );
            TRACE_RETURN();
            return(rc);
         }
         else
         {
            TRACE_RETURN();
            return(RC_OK);
         }
      }
      else
      {
         /*
          * NOT CUA, or on first line, exit.
          */
         TRACE_RETURN();
         return(RC_OK);
      }
   }
   THEcursor_left(TRUE,FALSE);
   /*
    * If we are after the last character of the line, exit.
    */
   if (x+CURRENT_VIEW->verify_col-1 > rec_len)
   {
      TRACE_RETURN();
      return(RC_OK);
   }

   getyx(CURRENT_WINDOW,y,x);
   my_wdelch(CURRENT_WINDOW);

   memdeln(rec,CURRENT_VIEW->verify_col-1+x,rec_len,1);
   rec_len--;
   /*
    * If there is a character off the right edge of the screen, display it
    * in the last character of the main window.
    */
   if (CURRENT_VIEW->verify_col-1+CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1 < rec_len)
   {
      wmove(CURRENT_WINDOW,y,CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1);
      put_char(CURRENT_WINDOW,rec[CURRENT_VIEW->verify_col-1+CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1],ADDCHAR);
      wmove(CURRENT_WINDOW,y,x);
   }
   /*
    * If the character being deleted is on a line which is in the marked
    * block, redisplay the window.
    */
   if ((CURRENT_VIEW == MARK_VIEW
      &&  CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line
      &&  CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)
   || (CURRENT_FILE->colouring && CURRENT_FILE->parser) )
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }
   return rc;
}

#ifdef HAVE_PROTO
static short sosdelchar( bool cua )
#else
static short sosdelchar( cua )
bool cua;
#endif
{
   unsigned short x=0,y=0;
   short rc;

   getyx(CURRENT_WINDOW,y,x);
   switch (CURRENT_VIEW->current_window)
   {
      case WINDOW_COMMAND:
         my_wdelch(CURRENT_WINDOW);
         if (x < cmd_rec_len)
         {
            memdeln(cmd_rec,x,cmd_rec_len,1);
            cmd_rec_len--;
         }
         TRACE_RETURN();
         return(RC_OK);
         break;
      case WINDOW_PREFIX:
         if (x < pre_rec_len)
         {
            my_wdelch(CURRENT_WINDOW);
            prefix_changed = TRUE;
            memdeln(pre_rec,x,pre_rec_len,1);
            pre_rec_len--;
         }
         TRACE_RETURN();
         return(RC_OK);
         break;
      case WINDOW_FILEAREA:
         /*
          * If running in read-only mode and an attempt is made to execute this
          * command in the MAIN window, then error...
          */
         if ( ISREADONLY(CURRENT_FILE) )
         {
            display_error(56,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         /*
          * Do not allow this command on the top or bottom of file lines or on
          * shadow lines.
          */
         if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         break;
      default:
         break;
   }

   /*
    * Remainder of processing is only for WINDOW_FILEAREA.
    */

   /*
    * If we have a marked CUA block, then move the cursor to the
    * start of the marked block, and delete the block. Don't attempt
    * to move the cursor left.
    */
   if ( INTERFACEx == INTERFACE_CUA
   &&  MARK_VIEW == CURRENT_VIEW
   &&  MARK_VIEW->mark_type == M_CUA )
   {
      ResetOrDeleteCUABlock( CUA_DELETE_BLOCK );
      TRACE_RETURN();
      return(RC_OK);
   }

   my_wdelch(CURRENT_WINDOW);
   /*
    * If we are not after the last character of the line...
    */
   if (x+CURRENT_VIEW->verify_col <= rec_len)
   {
      memdeln(rec,CURRENT_VIEW->verify_col-1+x,rec_len,1);
      rec_len--;
      /*
       * If there is a character off the right edge of the screen, display it
       * in the last character of the main window.
       */
      if (CURRENT_VIEW->verify_col-1+CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1 < rec_len)
      {
         wmove(CURRENT_WINDOW,y,CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1);
         put_char(CURRENT_WINDOW,rec[CURRENT_VIEW->verify_col-1+CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1],ADDCHAR);
         wmove(CURRENT_WINDOW,y,x);
      }
   }
   else
   {
      /*
       * Check if we have been passed "cua" as an optional parameter.
       * If so, SOS DELCHAR after the end of the line is
       * equivalent to a JOIN
       */
      if ( cua )
      {
         rc = execute_split_join( SPLTJOIN_JOIN, TRUE, TRUE );
         TRACE_RETURN();
         return(rc);
      }
   }
   /*
    * If the character being deleted is on a line which is in the marked
    * block...
    */
   if ((CURRENT_VIEW == MARK_VIEW
   &&  CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line
   &&  CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)
   || (CURRENT_FILE->colouring && CURRENT_FILE->parser))
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }

   TRACE_RETURN();
   return(RC_OK);
}
