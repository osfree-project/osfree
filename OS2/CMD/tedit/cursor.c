/***********************************************************************/
/* CURSOR.C - CURSOR commands                                          */
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

static char RCSid[] = "$Id: cursor.c,v 1.9 2002/07/09 10:53:10 mark Exp $";

#include <the.h>
#include <proto.h>

/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_cmdline(short col)
#else
short THEcursor_cmdline(col)
short col;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  THEcursor_cmdline");
   /*
    * If in READV CMDLINE, return without doing anything
    */
   if (in_readv)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * If CMDLINE is OFF return without doing anything.
    */
   if (CURRENT_WINDOW_COMMAND == (WINDOW *)NULL)
   {
      TRACE_RETURN();
      return(rc);
   }
   if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
      CURRENT_VIEW->previous_window = CURRENT_VIEW->current_window;
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
      CURRENT_VIEW->current_window = WINDOW_COMMAND;
   }
   wmove(CURRENT_WINDOW,0,col-1);
   CURRENT_VIEW->cmdline_col = col-1;
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_column(void)
#else
short THEcursor_column()
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   unsigned short x=0,y=0;

   TRACE_FUNCTION("cursor.c:  THEcursor_column");
   /*
    * If in READV CMDLINE, return without doing anything
    */
   if (in_readv)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_COMMAND:
         rc = THEcursor_home(FALSE);
         break;
      case WINDOW_PREFIX:
         getyx(CURRENT_WINDOW,y,x);
         CURRENT_VIEW->current_window = WINDOW_FILEAREA;
         wmove(CURRENT_WINDOW,y,0);
         break;
   }
   rc = execute_move_cursor(CURRENT_VIEW->current_column-1);
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_down(short escreen)
#else
short THEcursor_down(escreen)
short escreen;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   short x,y;

   TRACE_FUNCTION("cursor.c:  THEcursor_down");
   /*
    * If in READV CMDLINE, return without doing anything
    */
   if (in_readv)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_PREFIX:
      case WINDOW_FILEAREA:
         rc = scroll_line(DIRECTION_FORWARD,1L,FALSE,escreen);
         if ( rc == RC_OK
         &&   escreen == CURSOR_CUA )
         {
            getyx(CURRENT_WINDOW_FILEAREA,y,x);
            if ( x + CURRENT_VIEW->verify_col > min(rec_len,CURRENT_VIEW->verify_end) )
               rc = execute_move_cursor(rec_len);
         }
         break;
      case WINDOW_COMMAND:
         /*
          * Cycle forward  through the command list or tab to first line.
          */
         if (CMDARROWSTABCMDx)
            rc = Sos_topedge((CHARTYPE *)"");
         else
            rc = Retrieve((CHARTYPE *)"+");
         break;
      default:
         display_error(2,(CHARTYPE *)"",FALSE);
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_file(bool show_errors,LINETYPE line,LENGTHTYPE col)
#else
short THEcursor_file(show_errors,line,col)
bool show_errors;
LINETYPE line;
LENGTHTYPE col;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   unsigned short y=0,x=0;

   TRACE_FUNCTION("cursor.c:  THEcursor_file");
   /*
    * If in READV CMDLINE, return without doing anything.
    */
   if (in_readv)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * If line is not in display, error.
    */
   if (!line_in_view(current_screen,line))
   {
      if ( show_errors )
         display_error(63,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If column is not 0 and not in display, error.
    */
   if (col == 0)
      x = 1;
   else
   {
      if (!column_in_view(current_screen,col-1))
      {
         if ( show_errors )
            display_error(63,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      x = (LENGTHTYPE)((LINETYPE)col - (LINETYPE)CURRENT_VIEW->verify_col + 1);
   }
   y = get_row_for_focus_line(current_screen,line,CURRENT_VIEW->current_line);
   rc = THEcursor_move(show_errors,TRUE,y+1,x);
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_home(bool save)
#else
short THEcursor_home(save)
bool save;
#endif
/***********************************************************************/
{
   CHARTYPE last_win=0;
   unsigned short x=0,y=0;
   short rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  THEcursor_home");
   /*
    * If CMDLINE is OFF or in READV CMDLINE, return without doing anything
    */
   if (CURRENT_WINDOW_COMMAND == (WINDOW *)NULL
   ||  in_readv)
   {
      TRACE_RETURN();
      return(rc);
   }
   last_win = CURRENT_VIEW->previous_window;
   CURRENT_VIEW->previous_window =
              CURRENT_VIEW->current_window;
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
   {
      if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
         CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
      if (save)
         CURRENT_VIEW->current_window = last_win;
      else
         CURRENT_VIEW->current_window = WINDOW_FILEAREA;
      getyx(CURRENT_WINDOW,y,x);
      y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                               CURRENT_VIEW->current_row);
      wmove(CURRENT_WINDOW,y,x);
   }
   else
   {
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
      CURRENT_VIEW->current_window = WINDOW_COMMAND;
      wmove(CURRENT_WINDOW,0,0);
   }
   build_screen(current_screen);
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_left(short escreen,bool kedit_defaults)
#else
short THEcursor_left(escreen,kedit_defaults)
short escreen;
bool kedit_defaults;
#endif
/***********************************************************************/
{
   unsigned short x=0,y=0;
   short rc=RC_OK;

   /*
    * If escreen is CURSOR_ESCREEN or CURSOR_CUA, then scrolling of the
    * window will be done if possible.
    */
   TRACE_FUNCTION("cursor.c:  THEcursor_left");
   /*
    * The following should be a temporary fix for KEDIT compatability...
    */
   if (CURRENT_VIEW->prefix
   &&  kedit_defaults)
      escreen = CURSOR_SCREEN;

   getyx(CURRENT_WINDOW,y,x);
   /*
    * For all windows, if we are not at left column, move 1 pos to left.
    */
   if (x > 0)
   {
      wmove(CURRENT_WINDOW,y,x-1);
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * We are at the left edge of the FILEAREA!
    * For all windows, determine if CMDARROWSTABLRx is set for tabbing or
    * scrolling and act accordingly.
    */
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         /*
          * For CUA interface, at the left column of the file, move the cursor
          * to the last character of the previous line; that line
          * which is in scope
          */
         if ( escreen == CURSOR_CUA
         &&   CURRENT_VIEW->verify_col == 1 )
         {
            rc = scroll_line(DIRECTION_BACKWARD,1L,FALSE,escreen);
            rc = Sos_endchar( (CHARTYPE *)"" );
            break;
         }
         if ( escreen == CURSOR_SCREEN )
         {
            /*
             * Move the cursor into the prefix area, to the right-most
             * column, then move the cursor to the last column of the
             * window, either PREFIX (if ON) or FILEAREA
             */
            if (CURRENT_VIEW->prefix)
               rc = Sos_prefix((CHARTYPE *)"");
            rc = Sos_lastcol((CHARTYPE *)"");
         }
         else
         {
            if (CURRENT_VIEW->verify_col != 1
            &&  CURRENT_VIEW->autoscroll != 0)
            {
               LENGTHTYPE curr_col = x+CURRENT_VIEW->verify_col-1;
               LENGTHTYPE num_cols = (CURRENT_VIEW->autoscroll == (-1)) ? CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2 : CURRENT_VIEW->autoscroll;

               num_cols = min(num_cols,CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
               if ( num_cols >= CURRENT_VIEW->verify_col )
                  CURRENT_VIEW->verify_col = 1;
               else
                  CURRENT_VIEW->verify_col = CURRENT_VIEW->verify_col-num_cols;
               build_screen(current_screen);
               display_screen(current_screen);
               wmove(CURRENT_WINDOW,y,curr_col-CURRENT_VIEW->verify_col);
            }
            else
            {
               if (compatible_feel == COMPAT_KEDIT
               ||  compatible_feel == COMPAT_KEDITW)
               {
                  if (CURRENT_VIEW->prefix)
                     if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_LEFT)
                        rc = Sos_prefix((CHARTYPE *)"");
                  rc = Sos_lastcol((CHARTYPE *)"");
               }
            }
         }
         break;
      case WINDOW_PREFIX:
      case WINDOW_COMMAND:
         if ((escreen == CURSOR_ESCREEN
         &&  (CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_RIGHT)
         ||  escreen == CURSOR_SCREEN)
             rc = Sos_rightedge((CHARTYPE *)"");
         break;
      default:
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_right(short escreen,bool kedit_defaults)
#else
short THEcursor_right(escreen,kedit_defaults)
short escreen;
bool kedit_defaults;
#endif
/***********************************************************************/
{
   unsigned short x=0,y=0,tempx=0;
   short right_column=0;
   short rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  THEcursor_right");
   /*
    * The following should be a temporary fix for KEDIT compatability...
    */
   if (CURRENT_VIEW->prefix
   &&  kedit_defaults)
      escreen = CURSOR_SCREEN;
   getyx(CURRENT_WINDOW,y,x);
   /*
    * Check for going past end of line - max_line_length
    */
   if (CURRENT_VIEW->verify_col+x+1 > max_line_length)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   right_column = getmaxx(CURRENT_WINDOW)-1;
   /*
    * For CUA interface, if after the right-most character of the line,
    * move the cursor to the first column of the next line; that line
    * which is in scope
    */
   if ( escreen == CURSOR_CUA
   &&   x + CURRENT_VIEW->verify_col > min(rec_len,CURRENT_VIEW->verify_end) )
   {
      rc = scroll_line(DIRECTION_FORWARD,1L,FALSE,escreen);
      rc = Sos_firstcol( (CHARTYPE *)"" );
      TRACE_RETURN();
      return(rc);
   }
   /*
    * For all windows, if we are not at right column, move 1 pos to right.
    */
   if (x < right_column)
   {
      wmove(CURRENT_WINDOW,y,x+1);
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * For all windows, determine if CMDARROWSTABLRx is set for tabbing or
    * scrolling and act accordingly.
    */
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         if (escreen == CURSOR_SCREEN)
         {
            if (CURRENT_VIEW->prefix)
               rc = Sos_prefix((CHARTYPE *)"");
            else
               wmove(CURRENT_WINDOW,y,0); /* this should move down a line too */
         }
         else
         {
            tempx = getmaxx(CURRENT_WINDOW);
            if (x == tempx-1
            &&  CURRENT_VIEW->autoscroll != 0)
            {
               int curr_col=x+CURRENT_VIEW->verify_col-1;
               int num_cols = (CURRENT_VIEW->autoscroll == (-1)) ? CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2 : CURRENT_VIEW->autoscroll;

               num_cols = min(num_cols,CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
               CURRENT_VIEW->verify_col += num_cols;
               build_screen(current_screen);
               display_screen(current_screen);
               wmove(CURRENT_WINDOW,y,curr_col-CURRENT_VIEW->verify_col+2);
            }
         }
         break;
      case WINDOW_PREFIX:
      case WINDOW_COMMAND:
         if ( escreen == CURSOR_SCREEN )
            rc = Sos_leftedge((CHARTYPE *)"");
         else
         {
            if ((CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_LEFT)
               Tabpre((CHARTYPE *)"");
         }
         break;
      default:
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_up(short escreen)
#else
short THEcursor_up(escreen)
short escreen;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   short x,y;
   CHARTYPE *current_command=NULL;

   TRACE_FUNCTION("cursor.c:  THEcursor_up");
   /*
    * If in READV CMDLINE, return without doing anything
    */
   if (in_readv)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
      case WINDOW_PREFIX:
         rc = scroll_line(DIRECTION_BACKWARD,1L,FALSE,escreen);
         if ( rc == RC_OK
         &&   escreen == CURSOR_CUA )
         {
            getyx(CURRENT_WINDOW_FILEAREA,y,x);
            if ( x + CURRENT_VIEW->verify_col > min(rec_len,CURRENT_VIEW->verify_end) )
               rc = execute_move_cursor(rec_len);
         }
         break;
      case WINDOW_COMMAND:
         /*
          * Cycle backward through the command list or tab to last line.
          */
         if (CMDARROWSTABCMDx)
            rc = Sos_bottomedge((CHARTYPE *)"");
         else
         {
            current_command = get_next_command(DIRECTION_FORWARD,1);
            wmove(CURRENT_WINDOW_COMMAND,0,0);
            my_wclrtoeol(CURRENT_WINDOW_COMMAND);
            if (current_command != (CHARTYPE *)NULL)
               Cmsg(current_command);
         }
         break;
      default:
         display_error(2,(CHARTYPE *)"",FALSE);
         rc = RC_INVALID_OPERAND;
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_move(bool show_errors, bool escreen, short row, short col)
#else
short THEcursor_move(show_errors,escreen,row,col)
bool show_errors,escreen;
short row,col;
#endif
/***********************************************************************/
{
   register int i=0;
   short rc=RC_OK;
   unsigned short x=0,y=0;
   unsigned short max_row=0,min_row=0,max_col=0;
   short idx=(-1);

   TRACE_FUNCTION("cursor.c:  THEcursor_move");
   getyx(CURRENT_WINDOW_FILEAREA,y,x);
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   if (escreen)
   {
      /*
       * For CURSOR ESCREEN...
       */
      if (find_last_focus_line(&max_row) != RC_OK)
      {
         TRACE_RETURN();
         return(rc);
      }
      if (find_first_focus_line(&min_row) != RC_OK)
      {
         TRACE_RETURN();
         return(rc);
      }
      if (row == 0)
         row = y;
      else
      {
         if (row > max_row)
            row = max_row;
         else
         {
            if (row < min_row)
               row = min_row;
            else
            {
               if (CURRENT_SCREEN.sl[row-1].main_enterable)
                  row--;
               else
               {
                  if ( show_errors )
                     display_error(63,(CHARTYPE *)"",FALSE);
                  TRACE_RETURN();
                  return(RC_TOF_EOF_REACHED);/* this is a strange RC :-( */
               }
            }
         }
      }
      max_col = CURRENT_SCREEN.cols[WINDOW_FILEAREA];
      if (col == 0)
       col = x;
      else
      {
         if (col > max_col)
            col = max_col - 1;
         else
            col--;
      }
      switch(CURRENT_VIEW->current_window)
      {
         case WINDOW_COMMAND:
            rc = THEcursor_home(FALSE);
            break;
         case WINDOW_PREFIX:
            CURRENT_VIEW->current_window = WINDOW_FILEAREA;
            break;
      }
      wmove(CURRENT_WINDOW_FILEAREA,row,col);
      CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   }
   else
   {
      /*
       * Convert supplied row/col to 0 based offset...
       */
      if (row == 0)
         row = CURRENT_SCREEN.start_row[CURRENT_VIEW->current_window] + y;
      else
         row--;
      if (col == 0)
         col = CURRENT_SCREEN.start_col[CURRENT_VIEW->current_window] + x;
      else
         col--;
      max_row = CURRENT_SCREEN.screen_rows - 1;
      max_col = CURRENT_SCREEN.screen_cols - 1;
      /*
       * If row/col outside maximum screen size, exit...
       */
      if (row > max_row
      ||  col > max_col)
      {
         if ( show_errors )
            display_error(63,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_TOF_EOF_REACHED);/* this is a strange RC :-( */
      }
      /*
       * Determine which window the cursor will end up in...
       */
      for (i=0;i<VIEW_WINDOWS;i++)
      {
         int top_int_row = CURRENT_SCREEN.start_row[i] - CURRENT_SCREEN.screen_start_row;
         int bot_int_row = CURRENT_SCREEN.start_row[i] - CURRENT_SCREEN.screen_start_row + CURRENT_SCREEN.rows[i] - 1;
         if (row >= top_int_row
         &&  row <= bot_int_row
         &&  col >= (CURRENT_SCREEN.start_col[i]  - CURRENT_SCREEN.screen_start_col)
         &&  col <= (CURRENT_SCREEN.start_col[i] + CURRENT_SCREEN.cols[i] - 1 - CURRENT_SCREEN.screen_start_col))
         {
            idx = i;
            break;
         }
      }
      row = row - (CURRENT_SCREEN.start_row[idx] - CURRENT_SCREEN.screen_start_row);
      col = col - (CURRENT_SCREEN.start_col[idx] - CURRENT_SCREEN.screen_start_col);
      switch(idx)
      {
         case WINDOW_FILEAREA:
            row = get_row_for_tof_eof(row,current_screen);
            if (!CURRENT_SCREEN.sl[row].main_enterable)
            {
               if ( show_errors )
                  display_error(63,(CHARTYPE *)"",FALSE);
               TRACE_RETURN();
               return(RC_TOF_EOF_REACHED);/* this is a strange RC :-( */
            }
            rc = Sos_current((CHARTYPE *)"");
            wmove(CURRENT_WINDOW_FILEAREA,row,col);
            CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
            break;
         case WINDOW_PREFIX:
            row = get_row_for_tof_eof(row,current_screen);
            if (!CURRENT_SCREEN.sl[row].prefix_enterable)
            {
               if ( show_errors )
                  display_error(63,(CHARTYPE *)"",FALSE);
               TRACE_RETURN();
               return(RC_TOF_EOF_REACHED);/* this is a strange RC :-( */
            }
            rc = Sos_current((CHARTYPE *)"");
            rc = Sos_prefix((CHARTYPE *)"");
            wmove(CURRENT_WINDOW_PREFIX,row,col);
            CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[row].line_number;
            pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
            break;
         case WINDOW_COMMAND:
            rc = THEcursor_cmdline(col+1);
            break;
         default:
            if ( show_errors )
               display_error(63,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_TOF_EOF_REACHED);/* this is a strange RC :-( */
            break;
      }
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_goto(LINETYPE row,COLTYPE col)
#else
short THEcursor_goto(row,col)
LINETYPE row;
COLTYPE col;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  THEcursor_goto");
   if ( row > CURRENT_FILE->number_lines
   ||   row < 0
   ||  col > max_line_length
   || col < 0 )
   {
      display_error(63,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_TOF_EOF_REACHED);/* this is a strange RC :-( */
   }
   if ( col == 0 )
   {
      col = CURRENT_VIEW->verify_col;
   }
   if ( THEcursor_file( FALSE, row, col ) != RC_OK )
   {
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
      /*
       * Make specified line current and display specified column in the
       * middle of the screen if not already displayed.
       */
      CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line = row;
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
      if (!column_in_view(current_screen,col-1))
      {
         if ( col < CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2 )
            CURRENT_VIEW->verify_col = 0;
         else
            CURRENT_VIEW->verify_col = col - (CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2);
      }
      build_screen(current_screen);
      display_screen(current_screen);
      THEcursor_file( FALSE, row, col );
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short THEcursor_mouse(void)
#else
short THEcursor_mouse()
#endif
/***********************************************************************/
{
#if defined(PDCURSES_MOUSE_ENABLED)
#define MOUSE_Y (MOUSE_Y_POS+1-screen[scrn].screen_start_row)
#define MOUSE_X (MOUSE_X_POS+1-screen[scrn].screen_start_col)
#endif
#if defined(NCURSES_MOUSE_VERSION)
#define MOUSE_Y (ncurses_mouse_event.y+1-screen[scrn].screen_start_row)
#define MOUSE_X (ncurses_mouse_event.x+1-screen[scrn].screen_start_col)
#endif
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   int w=0;
   CHARTYPE scrn=0;
#endif
   short rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  THEcursor_mouse");

#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   /*
    * If in READV CMDLINE, return without doing anything
    */
   if (in_readv)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * First determine in which window the mouse is...
    */
   which_window_is_mouse_in(&scrn,&w);
   if (w == (-1)) /* shouldn't happen! */
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * If the mouse is in a different screen to the current one, move there
    */
   if (current_screen != scrn)
   {
      (void)Nextwindow((CHARTYPE *)"");
   }
   /*
    * Move the cursor to the correct screen coordinates...
    */
   rc = THEcursor_move(TRUE,FALSE,MOUSE_Y,MOUSE_X);
#endif

   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
long where_now(void)
#else
long where_now()
#endif
/***********************************************************************/
{
   long rc=0L;
   unsigned short x=0,y=0;

   TRACE_FUNCTION("cursor.c:  where_now");
   getyx(CURRENT_WINDOW,y,x);
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         rc |= WHERE_WINDOW_FILEAREA;
         break;
      case WINDOW_PREFIX:
         if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT)
            rc |= WHERE_WINDOW_PREFIX_LEFT;
         else
            rc |= WHERE_WINDOW_PREFIX_RIGHT;
         break;
      case WINDOW_COMMAND:
         if (CURRENT_VIEW->cmd_line == 'B')
            rc |= WHERE_WINDOW_CMDLINE_BOTTOM;
         else
            rc |= WHERE_WINDOW_CMDLINE_TOP;
         break;
   }
   if (display_screens == 1)
      rc |= WHERE_SCREEN_ONLY;
   else
   {
      if (current_screen == 0)
         rc |= WHERE_SCREEN_FIRST;
      else
         rc |= WHERE_SCREEN_LAST;
   }
   rc |= (long)y;
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
long what_current_now(void)
#else
long what_current_now()
#endif
/***********************************************************************/
{
   long rc=0;

   TRACE_FUNCTION("cursor.c:  what_current_now");
   if (CURRENT_WINDOW_PREFIX != NULL)
   {
      if ((CURRENT_VIEW->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT)
         rc |= WHERE_WINDOW_PREFIX_LEFT;
      else
         rc |= WHERE_WINDOW_PREFIX_RIGHT;
   }
   if (CURRENT_VIEW->cmd_line == 'B')
      rc |= WHERE_WINDOW_CMDLINE_BOTTOM;
   else
   {
      if (CURRENT_VIEW->cmd_line == 'T')
         rc |= WHERE_WINDOW_CMDLINE_TOP;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
long what_other_now(void)
#else
long what_other_now()
#endif
/***********************************************************************/
{
   long rc=0L;

   TRACE_FUNCTION("cursor.c:  what_other_now");
   if (display_screens == 1)
   {
      TRACE_RETURN();
      return(rc);
   }
   if (OTHER_SCREEN.win[WINDOW_PREFIX] != NULL)
   {
      if ((OTHER_SCREEN.screen_view->prefix & PREFIX_LOCATION_MASK) == PREFIX_LEFT)
         rc |= WHERE_WINDOW_PREFIX_LEFT;
      else
         rc |= WHERE_WINDOW_PREFIX_RIGHT;
   }
   if (OTHER_SCREEN.screen_view->cmd_line == 'B')
      rc |= WHERE_WINDOW_CMDLINE_BOTTOM;
   else
   {
      if (OTHER_SCREEN.screen_view->cmd_line == 'T')
         rc |= WHERE_WINDOW_CMDLINE_TOP;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
long where_next(long where,long what_current,long what_other)
#else
long where_next(where,what_current,what_other)
long where,what_current,what_other;
#endif
/***********************************************************************/
{
   long where_row=0L,where_window=0L,where_screen=0L;
   long what_current_window=0L;
   long what_other_window=0L;
   long rc=0L;
   unsigned short current_top_row=0,current_bottom_row=0;

   where_row = where & WHERE_ROW_MASK;
   where_window = where & WHERE_WINDOW_MASK;
   where_screen = where & WHERE_SCREEN_MASK;
   what_current_window = what_current & WHERE_WINDOW_MASK;
   what_other_window = what_other & WHERE_WINDOW_MASK;
   find_first_focus_line(&current_top_row);
   find_last_focus_line(&current_bottom_row);

   switch(where_window)
   {
      case WHERE_WINDOW_FILEAREA:
         /*
          * In filearea.
          */
         if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT)
         {
            /*
             * In filearea and there is prefix on right.
             * Result: same row,same screen,go to prefix.
             */
            return(where_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
         }
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In filearea and only screen.
                */
               if (where_row == (long)current_bottom_row)
               {
                  /*
                   * In filearea, prefix on left or off, on bottom line.
                   */
                  if (what_current_window & WHERE_WINDOW_CMDLINE_TOP)
                  {
                     /*
                      * In filearea,prefix on left or off, on bottom line.
                      * Result: row irrelevant,same screen,go to cmdline.
                      */
                     return(where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
                  }
                  if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM)
                  {
                     /*
                      * In filearea,prefix on left or off, on bottom line.
                      * Result: row irrelevant,same screen,go to cmdline.
                      */
                     return(where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
                  }
                  /*
                   * ****************** To get here, there is no cmdline. ****************
                   */
                  if (what_current_window & WHERE_WINDOW_PREFIX_LEFT)
                  {
                     /*
                      * In filearea,prefix on left or off, on bottom line.
                      * Result: first row,same screen,go to prefix.
                      */
                     return((long)current_top_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
                  }
               }
               /*
                * ****************** To get here, we are not on last row. *************
                */
               if (what_current_window & WHERE_WINDOW_PREFIX_LEFT)
               {
                  /*
                   * In filearea, prefix on left, not on bottom line.
                   * Result: next row,same screen,go to prefix.
                   */
                  return((where_row+1L) | where_screen | WHERE_WINDOW_PREFIX_LEFT);
               }
               /*
                * In filearea, no prefix, not on bottom line.
                * Result: next row,same screen,same window.
                */
               return((where_row+1L) | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
      case WHERE_WINDOW_PREFIX_LEFT:
         rc = where_row | where_screen | WHERE_WINDOW_FILEAREA;
         break;
      case WHERE_WINDOW_PREFIX_RIGHT:
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In right prefix and only screen.
                */
               if (where_row != (long)current_bottom_row)
               {
                  /*
                   * In right prefix and not on bottom line.
                   * Result: next row,same screen,go to filearea.
                   */
                  return((where_row+1L) | where_screen | WHERE_WINDOW_FILEAREA);
               }
               if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM)
               {
                  /*
                   * In right prefix, cmdline on bottom, on bottom line.
                   * Result: row irrelevant,same screen,go to cmdline.
                   */
                  return(where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
               }
               if (what_current_window & WHERE_WINDOW_CMDLINE_TOP)
               {
                  /*
                   * In right prefix, cmdline on top, on bottom line.
                   * Result: row irrelevant,same screen,go to cmdline.
                   */
                  return(where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
               }
               /*
                * In right prefix, no cmdline, on bottom line.
                * Result: first row,same screen,go to filearea.
                */
               return((long)current_top_row | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
      case WHERE_WINDOW_CMDLINE_TOP:
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In cmdline, and only screen.
                */
               if (what_current_window & WHERE_WINDOW_PREFIX_LEFT)
               {
                  /*
                   * In cmdline, and only screen and prefix on left.
                   * Result: first row, same screen, go to prefix.
                   */
                  return((long)current_top_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
               }
               /*
                * In cmdline, and prefix on right or none.
                * Result: first row, same screen, go to filearea.
                */
               return((long)current_top_row | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
      case WHERE_WINDOW_CMDLINE_BOTTOM:
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In cmdline, and only screen.
                */
               if (what_current_window & WHERE_WINDOW_PREFIX_LEFT)
               {
                  /*
                   * In cmdline, and only screen and prefix on left.
                   * Result: first row, same screen, go to prefix.
                   */
                  return((long)current_top_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
               }
               /*
                * In cmdline, and prefix on right or none.
                * Result: first row, same screen, go to filearea.
                */
               return((long)current_top_row | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
   }
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
long where_before(long where,long what_current,long what_other)
#else
long where_before(where,what_current,what_other)
long where,what_current,what_other;
#endif
/***********************************************************************/
{
   long where_row=0L,where_window=0L,where_screen=0L;
   long what_current_window=0L;
   long what_other_window=0L;
   long rc=0L;
   unsigned short current_top_row=0,current_bottom_row=0;

   where_row = where & WHERE_ROW_MASK;
   where_window = where & WHERE_WINDOW_MASK;
   where_screen = where & WHERE_SCREEN_MASK;
   what_current_window = what_current & WHERE_WINDOW_MASK;
   what_other_window = what_other & WHERE_WINDOW_MASK;
   find_first_focus_line(&current_top_row);
   find_last_focus_line(&current_bottom_row);

   switch(where_window)
   {
      case WHERE_WINDOW_FILEAREA:
         /*
          * In filearea.
          */
         if (what_current_window & WHERE_WINDOW_PREFIX_LEFT)
         {
            /*
             * In filearea and there is prefix on left.
             * Result: same row,same screen,go to prefix.
             */
            return(where_row | where_screen | WHERE_WINDOW_PREFIX_LEFT);
         }
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In filearea and only screen.
                */
               if (where_row == (long)current_top_row)
               {
                  /*
                   * In filearea, prefix on right or off, on top    line.
                   */
                  if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM)
                  {
                     /*
                      * In filearea,prefix on right or off, on top    line.
                      * Result: row irrelevant,same screen,go to cmdline.
                      */
                     return(where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
                  }
                  if (what_current_window & WHERE_WINDOW_CMDLINE_TOP)
                  {
                     /*
                      * In filearea,prefix on right or off, on top    line.
                      * Result: row irrelevant,same screen,go to cmdline.
                      */
                     return(where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
                  }
                  /*
                   * ****************** To get here, there is no cmdline. ****************
                   */
                  if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT)
                  {
                     /*
                      * In filearea,prefix on right or off, on top    line.
                      * Result: last  row,same screen,go to prefix.
                      */
                     return((long)current_bottom_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
                  }
               }
               /*
                * ****************** To get here, we are not on top  row. *************
                */
               if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT)
               {
                  /*
                   * In filearea, prefix on right, not on top    line.
                   * Result: prior row,same screen,go to prefix.
                   */
                  return((where_row-1L) | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
               }
               /*
                * In filearea, no prefix, not on top    line.
                * Result: prior row,same screen,same window.
                */
               return((where_row-1L) | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
      case WHERE_WINDOW_PREFIX_RIGHT:
         rc = where_row | where_screen | WHERE_WINDOW_FILEAREA;
         break;
      case WHERE_WINDOW_PREFIX_LEFT:
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In left  prefix and only screen.
                */
               if (where_row != (long)current_top_row)
               {
                  /*
                   * In left  prefix and not on top    line.
                   * Result: prior row,same screen,go to filearea.
                   */
                  return((where_row-1L) | where_screen | WHERE_WINDOW_FILEAREA);
               }
               if (what_current_window & WHERE_WINDOW_CMDLINE_BOTTOM)
               {
                  /*
                   * In left  prefix, cmdline on bottom, on top    line.
                   * Result: row irrelevant,same screen,go to cmdline.
                   */
                  return(where_row | where_screen | WHERE_WINDOW_CMDLINE_BOTTOM);
               }
               if (what_current_window & WHERE_WINDOW_CMDLINE_TOP)
               {
                  /*
                   * In left  prefix, cmdline on top, on top    line.
                   * Result: row irrelevant,same screen,go to cmdline.
                   */
                  return(where_row | where_screen | WHERE_WINDOW_CMDLINE_TOP);
               }
               /*
                * In left  prefix, no cmdline, on top    line.
                * Result: last  row,same screen,go to filearea.
                */
               return((long)current_bottom_row | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
      case WHERE_WINDOW_CMDLINE_TOP:
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In cmdline, and only screen.
                */
               if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT)
               {
                  /*
                   * In cmdline, and only screen and prefix on right.
                   * Result: last  row, same screen, go to prefix.
                   */
                  return((long)current_bottom_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
               }
               /*
                * In cmdline, and prefix on left  or none.
                * Result: last  row, same screen, go to filearea.
                */
               return((long)current_bottom_row | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
      case WHERE_WINDOW_CMDLINE_BOTTOM:
         switch(where_screen)
         {
            case WHERE_SCREEN_FIRST:
            case WHERE_SCREEN_LAST:
               /* the two cases above will be separate in future */
            case WHERE_SCREEN_ONLY:
               /*
                * In cmdline, and only screen.
                */
               if (what_current_window & WHERE_WINDOW_PREFIX_RIGHT)
               {
                  /*
                   * In cmdline, and only screen and prefix on right.
                   * Result: last  row, same screen, go to prefix.
                   */
                  return((long)current_bottom_row | where_screen | WHERE_WINDOW_PREFIX_RIGHT);
               }
               /*
                * In cmdline, and prefix on left  or none.
                * Result: last  row, same screen, go to filearea.
                */
               return((long)current_bottom_row | where_screen | WHERE_WINDOW_FILEAREA);
               break;
         }
         break;
   }
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool enterable_field(long where)
#else
bool enterable_field(where)
long where;
#endif
/***********************************************************************/
{
   bool rc=TRUE;
   ROWTYPE row=0;
   long where_screen=0L;
   CHARTYPE scrn=0;

   TRACE_FUNCTION("cursor.c:  enterable_field");
   where_screen = where & WHERE_SCREEN_MASK;
   row = (ROWTYPE)(where & WHERE_ROW_MASK);
   scrn = (where_screen == WHERE_SCREEN_LAST) ? 1 : 0;
   switch(where & WHERE_WINDOW_MASK)
   {
      case WHERE_WINDOW_FILEAREA:
         if (!screen[scrn].sl[row].main_enterable)
            rc = FALSE;
         break;
      case WHERE_WINDOW_PREFIX_LEFT:
      case WHERE_WINDOW_PREFIX_RIGHT:
         if (!screen[scrn].sl[row].prefix_enterable)
            rc = FALSE;
         break;
      case WHERE_WINDOW_CMDLINE_TOP:
      case WHERE_WINDOW_CMDLINE_BOTTOM:
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short go_to_new_field(long save_where,long where)
#else
short go_to_new_field(save_where,where)
long save_where,where;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   long save_where_screen=0L,where_screen=0L;
   long save_where_window=0L,where_window=0L;
   ROWTYPE where_row=0;

   TRACE_FUNCTION("cursor.c:  go_to_new_field");
   save_where_screen = save_where & WHERE_SCREEN_MASK;
   where_screen = where & WHERE_SCREEN_MASK;
   save_where_window = save_where & WHERE_WINDOW_MASK;
   where_window = where & WHERE_WINDOW_MASK;
   where_row = (ROWTYPE)(where & WHERE_ROW_MASK);
#if 0
   if (save_where_screen != where_screen)
   {
   }
#endif
   if (save_where_window == where_window)
   {
      /*
       * No change to screen or window...
       */
      CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
   }
   else
   {
      switch(save_where_window)
      {
         case WHERE_WINDOW_FILEAREA:
            switch(where_window)
            {
               case WHERE_WINDOW_PREFIX_LEFT:
               case WHERE_WINDOW_PREFIX_RIGHT:
                  CURRENT_VIEW->current_window = WINDOW_PREFIX;
                  CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
                  break;
               case WHERE_WINDOW_CMDLINE_TOP:
               case WHERE_WINDOW_CMDLINE_BOTTOM:
                  CURRENT_VIEW->previous_window = CURRENT_VIEW->current_window;
                  CURRENT_VIEW->current_window = WINDOW_COMMAND;
                  where_row = 0;
                  break;
            }
            break;
         case WHERE_WINDOW_PREFIX_LEFT:
         case WHERE_WINDOW_PREFIX_RIGHT:
            switch(where_window)
            {
               case WHERE_WINDOW_FILEAREA:
                  CURRENT_VIEW->current_window = WINDOW_FILEAREA;
                  CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
                  break;
               case WHERE_WINDOW_CMDLINE_TOP:
               case WHERE_WINDOW_CMDLINE_BOTTOM:
                  CURRENT_VIEW->previous_window = CURRENT_VIEW->current_window;
                  CURRENT_VIEW->current_window = WINDOW_COMMAND;
                  where_row = 0;
                  break;
            }
            break;
         case WHERE_WINDOW_CMDLINE_TOP:
         case WHERE_WINDOW_CMDLINE_BOTTOM:
            switch(where_window)
            {
               case WHERE_WINDOW_PREFIX_LEFT:
               case WHERE_WINDOW_PREFIX_RIGHT:
                  CURRENT_VIEW->current_window = WINDOW_PREFIX;
                  CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
                  break;
               case WHERE_WINDOW_FILEAREA:
                  CURRENT_VIEW->current_window = WINDOW_FILEAREA;
                  CURRENT_VIEW->focus_line = CURRENT_SCREEN.sl[where_row].line_number;
                  break;
            }
            break;
      }
   }
   wmove(CURRENT_WINDOW,where_row,0);
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void get_cursor_position(LINETYPE *screen_line,LINETYPE *screen_column,LINETYPE *file_line,LINETYPE *file_column)
#else
void get_cursor_position(screen_line,screen_column,file_line,file_column)
LINETYPE *screen_line,*screen_column,*file_line,*file_column;
#endif
/***********************************************************************/
{
   unsigned short y=0,x=0;
   unsigned short begy=0,begx=0;

   TRACE_FUNCTION("cursor.c:  get_cursor_position");
   if (curses_started)
   {
      getyx(CURRENT_WINDOW,y,x);
      getbegyx(CURRENT_WINDOW,begy,begx);
      *screen_line = (LINETYPE)(y + begy + 1L);
      *screen_column = (LINETYPE)(x + begx + 1L);
   }
   else
      *screen_line = *screen_column = (-1L);
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         *file_line = CURRENT_VIEW->focus_line;
         *file_column = (LINETYPE)x + (LINETYPE)CURRENT_VIEW->verify_col;
         break;
      case WINDOW_PREFIX:
         *file_line = CURRENT_VIEW->focus_line;
         *file_column = (-1L);
         break;
      default:   /* command line */
         *file_line = *file_column = (-1L);
         break;
   }
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short advance_focus_line(LINETYPE num_lines)
#else
short advance_focus_line(num_lines)
LINETYPE num_lines;
#endif
/***********************************************************************/
{
   unsigned short y=0,x=0;
   _LINE *curr=NULL;
   LINETYPE actual_lines=num_lines;
   short direction=DIRECTION_FORWARD,rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  advance_focus_line");
   if (num_lines < 0L)
   {
      actual_lines = -num_lines;
      direction = DIRECTION_BACKWARD;
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->focus_line,CURRENT_FILE->number_lines);
   while(actual_lines>0)
   {
      if (direction == DIRECTION_BACKWARD)
         curr = curr->prev;
      else
         curr = curr->next;
      if (curr == NULL)
         break;
      CURRENT_VIEW->focus_line += (LINETYPE)direction;
      if (CURRENT_VIEW->scope_all
      ||  IN_SCOPE(CURRENT_VIEW,curr))
         actual_lines--;
   }
   if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
      CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   build_screen(current_screen);
   display_screen(current_screen);
   if (curses_started)
   {
      getyx(CURRENT_WINDOW,y,x);
      y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                               CURRENT_VIEW->current_row);
      wmove(CURRENT_WINDOW,y,x);
   }
   if (FOCUS_TOF || FOCUS_BOF)
      rc = RC_TOF_EOF_REACHED;
   TRACE_RETURN();
   return rc;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short advance_current_line(LINETYPE num_lines)
#else
short advance_current_line(num_lines)
LINETYPE num_lines;
#endif
/***********************************************************************/
{
   _LINE *curr=NULL;
   LINETYPE actual_lines=num_lines;
   short direction=DIRECTION_FORWARD;
   short y=0,x=0,rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  advance_current_line");
   if (num_lines < 0L)
   {
      actual_lines = -num_lines;
      direction = DIRECTION_BACKWARD;
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->current_line,CURRENT_FILE->number_lines);
   while(actual_lines>0)
   {
      if (direction == DIRECTION_BACKWARD)
         curr = curr->prev;
      else
         curr = curr->next;
      if (curr == NULL)
         break;
      CURRENT_VIEW->current_line += (LINETYPE)direction;
      if (CURRENT_VIEW->scope_all
      ||  IN_SCOPE(CURRENT_VIEW,curr))
         actual_lines--;
   }
   build_screen(current_screen);
   if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
   {
      if (compatible_feel == COMPAT_XEDIT)
         THEcursor_cmdline(1);
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
   }
   if (CURRENT_VIEW->current_window != WINDOW_COMMAND
   &&  line_in_view(current_screen,CURRENT_VIEW->focus_line)
   &&  compatible_feel == COMPAT_XEDIT)
   {
      if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
         getyx(CURRENT_WINDOW,y,x);
      y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,CURRENT_VIEW->current_line);
      /* THEcursor_move(TRUE,FALSE,y+1,x+1); */
      THEcursor_move(TRUE,TRUE,y+1,x+1);
   }
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   build_screen(current_screen);
   display_screen(current_screen);
   if (CURRENT_TOF || CURRENT_BOF)
      rc = RC_TOF_EOF_REACHED;
   TRACE_RETURN();
   return rc;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short advance_current_or_focus_line(LINETYPE num_lines)
#else
short advance_current_or_focus_line(num_lines)
LINETYPE num_lines;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("cursor.c:  advance_current_or_focus_line");
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND
   ||  compatible_feel == COMPAT_XEDIT)
      rc = advance_current_line(num_lines);
   else
      rc = advance_focus_line(num_lines);
   TRACE_RETURN();
   return(rc);
}
/*man***************************************************************************
NAME
     resolve_current_and_focus_lines

SYNOPSIS
     void resolve_current_and_focus_lines(view,num_lines,direction,respect_stay)
     VIEW_DETAILS *view;
     LINETYPE true_line;
     LINETYPE num_lines;
     short direction;
     bool respect_stay;

DESCRIPTION
     This function determines the value of current_line and focus_line
     from the existing values of these variables, using the number of
     actual lines offset and direction passed as parameters.

     The num_lines parameter is the offset from either the focus or
     current line expressed in file lines (ie ignores scope - SCOPE ALL)
     The true_line parameter is the value of either focus or current lien.

     This function determines what the new values for focus_line and
     current_line are to be based on the status of STAY and of the
     current compatibility mode.  On return from this function, the
     screen will have been displayed with the correct positioning of
     the focus and current lines and the position of the cursor.

     If respect_stay is FALSE, then SET STAY is ignored (ie for PUT
     command).

     If sos is TRUE, then this function was called from a SOS command.
     All SOS commands should result in cursor movement as for THE
     compatibility mode.

RETURN VALUE
     void
*******************************************************************************/
#ifdef HAVE_PROTO
void resolve_current_and_focus_lines(VIEW_DETAILS *view, LINETYPE true_line, LINETYPE num_lines, short direction,bool respect_stay,bool sos)
#else
void resolve_current_and_focus_lines(view,true_line,num_lines,direction,respect_stay,sos)
VIEW_DETAILS *view;
LINETYPE true_line,num_lines;
short direction;
bool respect_stay;
bool sos;
#endif
/***********************************************************************/
{
   short y=0,x=0;
   short save_compatible_feel=compatible_feel;

   TRACE_FUNCTION("cursor.c:  resolve_current_and_focus_lines");
   /*
    * If no lines to move, don't do anything...
    */
   if (num_lines == 0)
   {
      build_screen(current_screen);
      display_screen(current_screen);
      TRACE_RETURN();
      return;
   }
   /*
    * Set the internal compatibility mode to THE for sos = TRUE.
    */
   if (sos)
      save_compatible_feel = COMPAT_THE;
   /*
    * If STAY is ON, and we are respecting it, don't do anything...
    */
   if (view->stay
   && (respect_stay || compatible_feel != COMPAT_XEDIT))
   {
      build_screen(current_screen);
      display_screen(current_screen);
      TRACE_RETURN();
      return;
   }
   /*
    * If we are on the command line, all actions are the same irrespective
    * of the compatibility mode in place.
    */
   if (view->current_window == WINDOW_COMMAND)
   {
      view->current_line = true_line+num_lines-(LINETYPE)direction;
      build_screen(current_screen);
      display_screen(current_screen);
      TRACE_RETURN();
      return;
   }
   /*
    * From here down is applicable to the cursor being in the FILEAREA or
    * PREFIX...
    */
   switch(save_compatible_feel)
   {
      case COMPAT_THE:
      case COMPAT_KEDIT:
      case COMPAT_KEDITW:
         view->focus_line = true_line+num_lines-(LINETYPE)direction;
         build_screen(current_screen);
         if (!line_in_view(current_screen,view->focus_line))
            view->current_line = view->focus_line;
         pre_process_line(view,view->focus_line,(_LINE *)NULL);
         build_screen(current_screen);
         display_screen(current_screen);
         if (curses_started)
         {
            getyx(SCREEN_WINDOW(current_screen),y,x);
            y = get_row_for_focus_line(current_screen,view->focus_line,
                               view->current_row);
            wmove(SCREEN_WINDOW(current_screen),y,x);
         }
         break;
      case COMPAT_XEDIT:
         view->current_line = true_line+num_lines-(LINETYPE)direction;
         pre_process_line(view,view->focus_line,(_LINE *)NULL);
         build_screen(current_screen);
         if (!line_in_view(current_screen,view->focus_line))
            THEcursor_cmdline(1);
         else
         {
            if (curses_started)
            {
               if (view->current_window == WINDOW_FILEAREA)
                  getyx(SCREEN_WINDOW(current_screen),y,x);
               y = get_row_for_focus_line(current_screen,view->focus_line,view->current_line);
               THEcursor_move(TRUE,FALSE,y+1,x+1);
            }
         }
         display_screen(current_screen);
         break;
   }
   TRACE_RETURN();
   return;
}
