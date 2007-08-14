/***********************************************************************/
/* COMM1.C - Commands A-C                                              */
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

static char RCSid[] = "$Id: comm1.c,v 1.13 2002/01/27 10:06:27 mark Exp $";

#include <the.h>
#include <proto.h>

/*#define DEBUG 1*/

/*man-start*********************************************************************


========================================================================
COMMAND REFERENCE
========================================================================
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     add - add blank line

SYNTAX
     Add [n]

DESCRIPTION
     The ADD command inserts 'n' blank lines after the <current line>,
     if issued from the <command line> or after the <focus line>, 
     if issued from the <filearea> or <prefix area>.

     If <SET NEWLINES> is set to ALIGNED, the cursor is positioned in
     the column corresponding to the first column not containing a 
     space in the line above.

     If <SET NEWLINES> is set to LEFT, the cursor is positioned in the
     first column.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1

SEE ALSO
     <SOS ADDLINE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Add(CHARTYPE *params)
#else
short Add(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define ADD_PARAMS  1
   CHARTYPE *word[ADD_PARAMS+1];
   CHARTYPE strip[ADD_PARAMS];
   unsigned short num_params=0;
   LINETYPE num_lines=0L;

   TRACE_FUNCTION("comm1.c:   Add");
   /*
    * Validate the parameters that have been supplied. The one and only
    * parameter should be a positive integer greater than zero.
    * If no parameter is supplied, 1 is assumed.
    */
   strip[0]=STRIP_BOTH;
   num_params = param_split(params,word,ADD_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      num_params = 1;
      word[0] = (CHARTYPE *)"1";
   }
   if (num_params != 1)
   {
      display_error(1,word[1],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (!valid_positive_integer(word[0]))
   {
      display_error(4,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   num_lines = atol((DEFCHAR *)word[0]);
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   insert_new_line((CHARTYPE *)"",0,num_lines,get_true_line(TRUE),FALSE,FALSE,TRUE,CURRENT_VIEW->display_low,TRUE,FALSE);
   if (curses_started
   && CURRENT_VIEW->current_window == WINDOW_COMMAND)
      THEcursor_home(TRUE);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     alert - display a user configurable dialog box with notification

SYNTAX
     ALERT /prompt/ [EDITfield [/val/]] [TITLE /title/] [OK|OKCANCEL|YESNO|YESNOCANCEL] [DEFBUTTON n]

DESCRIPTION
     The ALERT command is identical to the <DIALOG> command except that
     if <SET BEEP> is on, a beep is played.

     On exit from the ALERT command, the following Rexx variables are set:

          ALERT.0 - 2
          ALERT.1 - value of 'EDITfield'
          ALERT.2 - button selected as specified in the call to the command.

     The colours for the alert box are the same as for a dialog box, except
     the prompt area which uses the colour set by <SET COLOR> ALERT.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. Does not support bitmap icons or font options.

SEE ALSO
     <POPUP>, <DIALOG>, <READV>, <SET COLOR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Alert(CHARTYPE *params)
#else
short Alert(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Alert");
   /*
    * If we have a beep() functiond and its ON, ring it..
    */
#ifdef HAVE_BEEP
   if ( BEEPx )
      beep();
#endif
   rc = prepare_dialog( params, TRUE, (CHARTYPE *)"ALERT", FALSE );
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     all - select and display restricted set of lines

SYNTAX
     ALL [rtarget]

DESCRIPTION
     The ALL command allows for the selective display, and editting
     (subject to <SET SCOPE>) of lines that match the specified target.
     This target consists of any number of individual targets
     seperated by '&' (logical and) or '|' (logical or). 

     For example, to display all lines in a file that contain the 
     strings 'ball' and 'cat' on the same line or the named lines 
     .fred or .bill, use the following command

     ALL /ball/ & /cat/ | .fred | .bill

     Logical operators act left to right, with no precedence for &.

     <rtarget> can also be specified as a regular expression. The syntax of
     this is "Regexp /re/". eg ALL R /[0-9].*$/

     ALL without any arguments, is the equivalent of setting the
     selection level of all lines in your file to 0 and running
     <SET DISPLAY> 0 0.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SET SCOPE>, <SET DISPLAY>, <SET SELECT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short All(CHARTYPE *params)
#else
short All(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   LINE *curr=NULL;
   bool target_found=FALSE;
   short status=RC_OK;
   short target_type=TARGET_NORMAL|TARGET_REGEXP;
   TARGET target;
   LINETYPE line_number=0L;
   unsigned short x=0,y=0;
   bool save_scope=FALSE;
   LINETYPE num_lines=0L;

   TRACE_FUNCTION("comm1.c:   All");
   if (strlen((DEFCHAR *)params) == 0)
   {
      if (CURRENT_FILE->number_lines == 0L)
      {
         TRACE_RETURN();
         return(rc);
      }
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
      curr = CURRENT_FILE->first_line->next;
      while(1)
      {
         curr->select = 0;
         curr = curr->next;
         if (curr->next == NULL)
            break;
      }
      CURRENT_VIEW->display_low = 0;
      CURRENT_VIEW->display_high = 0;
      build_screen(current_screen); 
      display_screen(current_screen);
      TRACE_RETURN();
      return(rc);
   }
   if (CURRENT_FILE->number_lines == 0L)
   {
      display_error(17,params,FALSE);
      TRACE_RETURN();
      return(RC_TARGET_NOT_FOUND);
   }
   /*
    * Validate the parameters as valid targets...
    */
   initialise_target(&target);
   rc = parse_target(params,get_true_line(TRUE),&target,target_type,TRUE,TRUE,FALSE);
   if (rc != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Save the select levels for all lines in case no target is found.
    */
   curr = CURRENT_FILE->first_line->next;
   while(1)
   {
      curr->save_select = curr->select;
      curr = curr->next;
      if (curr->next == NULL)
         break;
   }
   /*
    * Find all lines for the supplied target...
    */
   curr = CURRENT_FILE->first_line;
   status = FALSE;
   save_scope = CURRENT_VIEW->scope_all;
   CURRENT_VIEW->scope_all = TRUE;
   for (line_number=0L;curr->next != NULL;line_number++)
   {
      status = find_rtarget_target(curr,&target,0L,line_number,&num_lines);
      if (status == RC_OK) /* target found */
      {
         target_found = TRUE;
         curr->select = 1;
      }
      else if (status == RC_TARGET_NOT_FOUND) /* target not found */
      {
         curr->select = 0;
      }
      else  /* error */
         break;
      curr = curr->next;
   }
   /*
    * If at least one line matches the target, set DISPLAY to 1 1,
    * otherwise reset the select levels as they were before the command.
    */
   if (target_found)
   {
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
      CURRENT_VIEW->display_low = 1;
      CURRENT_VIEW->display_high = 1;
      CURRENT_VIEW->scope_all = FALSE;
      CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,CURRENT_FILE->first_line->next,1L,DIRECTION_FORWARD);
      build_screen(current_screen); 
      display_screen(current_screen);
      CURRENT_VIEW->focus_line = calculate_focus_line(CURRENT_VIEW->focus_line,
                                                    CURRENT_VIEW->current_line);
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
      {
         getyx(CURRENT_WINDOW,y,x);
         y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                                  CURRENT_VIEW->current_row);
         wmove(CURRENT_WINDOW,y,x);
      }
   }
   else
   {
      CURRENT_VIEW->scope_all = save_scope;
      curr = CURRENT_FILE->first_line->next;
      while(1)
      {
         curr->select = curr->save_select;
         curr = curr->next;
         if (curr->next == NULL)
            break;
      }
      if (status == RC_TARGET_NOT_FOUND)
      {
         display_error(17,params,FALSE);
         rc = RC_TARGET_NOT_FOUND;
      }
      else
         rc = status;
   }
   free_target(&target);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     backward - scroll backwards [n] screens

SYNTAX
     BAckward [n|*]

DESCRIPTION
     The BACKWARD command scrolls the file contents backwards through
     the file 'n' or '*' screens.

     If 0 is specified as the number of screens to scroll, the last
     line of the file becomes the <current line>.

     If the BACKWARD command is issued while the current line is
     the <Top-of-File line>, the last line of the file becomes the
     <current line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Does not support HALF or Lines options.

DEFAULT
     1

SEE ALSO
     <FORWARD>, <TOP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Backward(CHARTYPE *params)
#else
short Backward(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define BAC_PARAMS  1
   CHARTYPE *word[BAC_PARAMS+1];
   CHARTYPE strip[BAC_PARAMS];
   unsigned short num_params=0;
   LINETYPE num_pages=0L;
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Backward");
   /*
    * Validate parameters...
    */
   strip[0]=STRIP_BOTH;
   num_params = param_split(params,word,BAC_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      num_params = 1;
      word[0] = (CHARTYPE *)"1";
   }
   if (num_params != 1)
   {
      display_error(1,(CHARTYPE *)word[1],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If parameter is '*', set current line equal to "Top of File".
    */
   if (strcmp((DEFCHAR *)word[0],"*") == 0)
   {
      rc = Top((CHARTYPE *)"");
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If the parameter is not a valid integer, error.
    */
   if (!valid_integer(word[0]))
   {
      display_error(1,(CHARTYPE *)word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Number of screens to scroll is set here.
    */
   num_pages = atol((DEFCHAR *)word[0]);
   /*
    * If the number specified is < 0, error...
    */
   if (num_pages < 0L)
   {
      display_error(5,(CHARTYPE *)word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the current line is already on "Top of File" or the parameter is
    * 0, go to the bottom of the file.
    */
   if ( num_pages == 0
   || ( CURRENT_TOF && PAGEWRAPx ) )
   {
      rc = Bottom((CHARTYPE *)"");
      TRACE_RETURN();
      return(rc);
   }
   /*
    * Scroll the screen num_pages...
    */
   rc = scroll_page(DIRECTION_BACKWARD,num_pages,FALSE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     bottom - move to the bottom of the file

SYNTAX
     Bottom

DESCRIPTION
     The BOTTOM command moves to the very end of the current file.
     The last line of the file is set to the <current line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FORWARD>, <TOP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Bottom(CHARTYPE *params)
#else
short Bottom(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   unsigned short x=0,y=0;

   TRACE_FUNCTION("comm1.c:   Bottom");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (CURRENT_VIEW->scope_all)
      CURRENT_VIEW->current_line = CURRENT_FILE->number_lines;
   else
      CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,CURRENT_FILE->last_line->prev,CURRENT_FILE->number_lines,DIRECTION_BACKWARD);
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   build_screen(current_screen); 
   if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   if (curses_started)
   {
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
         getyx(CURRENT_PREV_WINDOW,y,x);
      else
         getyx(CURRENT_WINDOW,y,x);
      display_screen(current_screen);
      y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                               CURRENT_VIEW->current_row);
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
         wmove(CURRENT_PREV_WINDOW,y,x);
      else
         wmove(CURRENT_WINDOW,y,x);
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     cancel - quit from all unaltered files in the ring

SYNTAX
     CANcel

DESCRIPTION
     The CANCEL command exits from THE quickly by executing a <QQUIT>
     command for every file in the ring that does not have any 
     outstanding alterations.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <CCANCEL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cancel(CHARTYPE *params)
#else
short Cancel(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   VIEW_DETAILS *save_current_view=(VIEW_DETAILS *)NULL;
   LINETYPE save_number_of_files=number_of_files;
   register int i=0;

   TRACE_FUNCTION("comm1.c:   Cancel");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   for (i=0;i<save_number_of_files;i++)
   {
      if (CURRENT_FILE->save_alt == 0)
         free_view_memory(TRUE,FALSE);
      else
      {
         save_current_view = CURRENT_VIEW;
         CURRENT_VIEW = CURRENT_VIEW->next;
         if (CURRENT_VIEW == NULL)
            CURRENT_VIEW = vd_first;
      }
   }
   if (save_current_view != (VIEW_DETAILS *)NULL)
   {
      CURRENT_VIEW = save_current_view;
      CURRENT_SCREEN.screen_view = CURRENT_VIEW;
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      build_screen(current_screen); 
      display_screen(current_screen);
      if (curses_started)
      {
         if (CURRENT_WINDOW_PREFIX != NULL)
            touchwin(CURRENT_WINDOW_PREFIX);
         if (CURRENT_WINDOW_COMMAND != NULL)
            touchwin(CURRENT_WINDOW_COMMAND);
         touchwin(CURRENT_WINDOW_FILEAREA);
         touchwin(CURRENT_WINDOW);
      }
   }
   if (number_of_files > 0)
   {
      sprintf((DEFCHAR *)temp_cmd,"%ld file(s) remain with outstanding changes",number_of_files);
      display_error(0,(CHARTYPE *)temp_cmd,TRUE);
   }
   TRACE_RETURN();
   return(QUIT);
}
/*man-start*********************************************************************
COMMAND
     cappend - append text after column pointer

SYNTAX
     CAppend [text]

DESCRIPTION
     The CAPPEND command moves the column pointer to the end of the
     focus line and appends the specified 'text'.

     If no 'text' is specified, the column pointer moves to the first
     trailing space.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <CLAST>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cappend(CHARTYPE *params)
#else
short Cappend(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Cappend");
   rc = column_command(params,COLUMN_CAPPEND);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     ccancel - qquit from all files in the ring

SYNTAX
     CCancel

DESCRIPTION
     The CCANCEL command exits from THE quickly by executing the <QQUIT>
     command for every file in the ring. Any changes made to any of 
     the files since the last <SAVE> will be lost.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <CANCEL>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Ccancel(CHARTYPE *params)
#else
short Ccancel(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("comm1.c:   Ccancel");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   CURRENT_VIEW = vd_first;
   while (CURRENT_VIEW != (VIEW_DETAILS *)NULL)
   {
      free_view_memory(TRUE,FALSE);
   }
   TRACE_RETURN();
   return(QUIT);
}
/*man-start*********************************************************************
COMMAND
     cdelete - delete text starting at column pointer

SYNTAX
     CDelete [column target]

DESCRIPTION
     The CDELETE command deletes characters starting from the current
     column pointer for the specified <'column target'>.

     If no <'column target'> is specified, the character at the column
     pointer is deleted.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Incomplete. No string targets.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cdelete(CHARTYPE *params)
#else
short Cdelete(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   short target_type=TARGET_ABSOLUTE|TARGET_RELATIVE|TARGET_STRING|TARGET_BLANK;
   TARGET target;
   LENGTHTYPE start_col=0,del_start=0;
   unsigned int y=0,x=0;

   TRACE_FUNCTION("comm1.c:   Cdelete");
   /*
    * Validate the cursor position...
    */
   if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
   {
      getyx(CURRENT_WINDOW,y,x);
      rc = processable_line(CURRENT_VIEW,CURRENT_SCREEN.sl[y].line_number,CURRENT_SCREEN.sl[y].current);
      switch(rc)
      {
         case LINE_SHADOW:
            display_error(87,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
            break;
/*       case LINE_TOF_EOF: MH12 */
         case LINE_TOF:
         case LINE_EOF:
            display_error(36,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
            break;
         default:
            break;
      }
   }
   /*
    * Determine at which column to start the search...
    */
   switch (CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         start_col = (CURRENT_VIEW->verify_col) + x;
         if (start_col > CURRENT_VIEW->zone_end)
            start_col = min(max_line_length,CURRENT_VIEW->zone_end+1);
         if (start_col < CURRENT_VIEW->zone_start)
            start_col = max(1,CURRENT_VIEW->zone_start-1);
         break;
      case WINDOW_PREFIX:
         start_col = max(CURRENT_VIEW->current_column,max(1,CURRENT_VIEW->zone_start));
         break;
      case WINDOW_COMMAND:
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->current_line,(LINE *)NULL);
         start_col = CURRENT_VIEW->current_column;
         break;
   }
   /*
    * Validate the parameters as valid targets...
    */
   initialise_target(&target);
   rc = parse_target(params,(LINETYPE)start_col,&target,target_type,TRUE,TRUE,TRUE);
   if (rc != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Find the valid column target. If found process the command...
    */
   if ((find_column_target(rec,rec_len,&target,start_col,TRUE,TRUE)) == RC_OK)
   {
      CURRENT_VIEW->current_column = start_col;
      if (target.num_lines < 0)
      {
         if (start_col > CURRENT_VIEW->zone_end)
         {
            start_col--;
            target.num_lines++;
         }
         del_start = start_col+target.num_lines;
         (void)memdeln(rec,del_start,rec_len,-target.num_lines);
         rec_len = calculate_rec_len(ADJUST_DELETE,rec_len,del_start,-target.num_lines);
/*     rec_len += (rec_len>del_start)?target.num_lines:0;*/
      }
      else
      {
         if (start_col < CURRENT_VIEW->zone_start)
         {
            start_col++;
            target.num_lines--;
         }
         del_start = start_col-1;
         (void)memdeln(rec,del_start,rec_len,target.num_lines);
         rec_len = calculate_rec_len(ADJUST_DELETE,rec_len,del_start,target.num_lines);
/*       rec_len -= (rec_len>del_start)?target.num_lines:0;*/
      }
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      {
         post_process_line(CURRENT_VIEW,CURRENT_VIEW->current_line,(LINE *)NULL,TRUE);
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      }
      else
      {
         rc = THEcursor_column();
      }
      build_screen(current_screen);
      display_screen(current_screen);
   }
   else
   {
      if (CURRENT_VIEW->current_window == WINDOW_COMMAND)
      {
         post_process_line(CURRENT_VIEW,CURRENT_VIEW->current_line,(LINE *)NULL,TRUE);
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      }
   }
   free_target(&target);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     cfirst - move column pointer to beginning of zone

SYNTAX
     CFirst

DESCRIPTION
     The CFIRST command moves the column pointer to the beginning of
     the zone.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SET ZONE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cfirst(CHARTYPE *params)
#else
short Cfirst(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   bool need_to_redisplay=FALSE;

   TRACE_FUNCTION("comm1.c:   Cfirst");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (column_in_view(current_screen,CURRENT_VIEW->current_column-1))
      need_to_redisplay = TRUE;
   CURRENT_VIEW->current_column = CURRENT_VIEW->zone_start;
   if (column_in_view(current_screen,CURRENT_VIEW->current_column-1))
      need_to_redisplay = TRUE;
   if (need_to_redisplay)
      display_screen(current_screen);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     change - change one string to another

SYNTAX
     Change [/string1/string2/ [target] [n] [m]]

DESCRIPTION
     The CHANGE command changes one string of text to another.

     The first parameter to the change command is the old and new
     string values, seperated by delimiters.
     The first non alphabetic character after the 'change' command 
     is the delimiter.

     <'target'> specifies how many lines are to be searched for 
     occurrences of 'string1' to be changed.

     'n' determines how many occurrences of 'string1' are to be 
     changed to 'string2' on each line. 'n' may be specified as
     '*' which will result in all occurrences of 'string1' will
     be changed.  '*' is equivalent to the current WIDTH of the
     line.

     'm' determines from which occurrence of 'string1' on the line 
     changes are to commence.

     If no arguments are supplied to the CHANGE command, the last
     change command, if any, is re-executed.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1 1 1

SEE ALSO
     <SCHANGE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Change(CHARTYPE *params)
#else
short Change(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Change");
   rc = execute_change_command(params,FALSE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     cinsert - insert text starting at the column pointer

SYNTAX
     CInsert text

DESCRIPTION
     The CINSERT command inserts 'text' starting at the column position.

     'text' can include leading or trailing space characters. Thus
     CINSERT immediatley followed by 5 spaces, will insert 4 space 
     characters. The first space character is the command seperator.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cinsert(CHARTYPE *params)
#else
short Cinsert(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Cinsert");
   rc = column_command(params,COLUMN_CINSERT);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     clast - move the column pointer to end of zone

SYNTAX
     CLAst

DESCRIPTION
     The CLAST command moves the column pointer to the end of the
     zone.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SET ZONE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Clast(CHARTYPE *params)
#else
short Clast(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   bool need_to_redisplay=FALSE;

   TRACE_FUNCTION("comm1.c:   Clast");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (column_in_view(current_screen,CURRENT_VIEW->current_column-1))
      need_to_redisplay = TRUE;
   CURRENT_VIEW->current_column = CURRENT_VIEW->zone_end;
   if (column_in_view(current_screen,CURRENT_VIEW->current_column-1))
      need_to_redisplay = TRUE;
   if (need_to_redisplay)
      display_screen(current_screen);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     clocate - move the column pointer

SYNTAX
     CLocate column target

DESCRIPTION
     The CLOCATE command scans the file for the specified <'column target'>
     beginning with the column following (or preceding) the column pointer.

     Column targets can be specified as absolute targets, relative
     targets or string targets.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Incomplete. No string targets.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Clocate(CHARTYPE *params)
#else
short Clocate(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   short target_type=TARGET_ABSOLUTE|TARGET_RELATIVE|TARGET_STRING|TARGET_BLANK;
   TARGET target;
   CHARTYPE *line=NULL;
   LINE *curr=NULL;
   LENGTHTYPE len=0,start_col=0;
   unsigned int y=0,x=0;

   TRACE_FUNCTION("comm1.c:   Clocate");
   /*
    * Determine at which column to start the search...
    */
   switch (CURRENT_VIEW->current_window)
   {
      case WINDOW_FILEAREA:
         if (compatible_feel == COMPAT_XEDIT)
         {
            curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->current_line,CURRENT_FILE->number_lines);
            line = curr->line;
            len = curr->length;
            start_col = CURRENT_VIEW->current_column;
         }
         else
         {
            line = rec;
            len = rec_len;
            getyx(CURRENT_WINDOW,y,x);
            start_col = (CURRENT_VIEW->verify_col) + x;
            if (start_col > CURRENT_VIEW->zone_end)
               start_col = min(max_line_length,CURRENT_VIEW->zone_end+1);
            if (start_col < CURRENT_VIEW->zone_start)
               start_col = max(1,CURRENT_VIEW->zone_start-1);
         }
         break;
      case WINDOW_PREFIX:
         line = rec;
         len = rec_len;
         start_col = max(CURRENT_VIEW->current_column,max(1,CURRENT_VIEW->zone_start));
         break;
      case WINDOW_COMMAND:
         curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,CURRENT_VIEW->current_line,CURRENT_FILE->number_lines);
         line = curr->line;
         len = curr->length;
         start_col = CURRENT_VIEW->current_column;
         break;
   }
   /*
    * Validate the parameters as valid targets...
    */
   initialise_target(&target);
   rc = parse_target(params,(LINETYPE)start_col,&target,target_type,TRUE,TRUE,TRUE);
   if (rc != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Find the valid column target. If found process the command...
    */
   if ((find_column_target(line,len,&target,start_col,TRUE,FALSE)) == RC_OK)
   {
      CURRENT_VIEW->current_column = start_col + target.num_lines;
      if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
         rc = THEcursor_column();
      build_screen(current_screen);
      display_screen(current_screen);
   }
   free_target(&target);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     cmatch - find matching bracket character

SYNTAX
     CMATCH

DESCRIPTION
     The CMATCH command searches for the matching bracket character to
     the character under the cursor.

     It handles nested sets of matching pairs.
     The matching character pairs are []{}<>().

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cmatch(CHARTYPE *params)
#else
short Cmatch(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   static CHARTYPE *match = (CHARTYPE *)"[]{}<>()";
   unsigned short x=0,y=0,current_y=0;
   CHARTYPE ch=0,match_ch=0;
   register short i=0;
   short direction_backward=0;
   short matches=1,match_col=(-1),start_col=0,focus_column=0;
   LINETYPE offset=0L;
   LINE *curr=NULL;
   LINETYPE focus_line=0L;
   bool use_current=TRUE;

   TRACE_FUNCTION("comm1.c:   Cmatch");
   if ( curses_started )
      getyx(CURRENT_WINDOW,y,x);
   /*
    * Determine variables based on which window we are in
    */
   if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
      current_y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                            CURRENT_VIEW->current_row);
      focus_line = CURRENT_VIEW->focus_line;
      focus_column = CURRENT_VIEW->verify_col + x - 1;
      ch = rec[focus_column];
      use_current = FALSE;
   }
   else
   {
      y = current_y = CURRENT_VIEW->current_row;
      focus_line = CURRENT_VIEW->current_line;
      focus_column = CURRENT_VIEW->current_column - 1;
      ch = CURRENT_SCREEN.sl[current_y].contents[focus_column];
      use_current = TRUE;
   }
   /*
    * This command cannot be issued on TOF or BOF.
    */
   if (TOF(focus_line)
   ||  BOF(focus_line))
   {
      display_error(66,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_TOF_EOF_REACHED);
   }
   /*
    * This command cannot be entered on a shadow line.
    */
   if (CURRENT_SCREEN.sl[y].line_type == LINE_SHADOW)
   {
      display_error(87,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_TARGET_NOT_FOUND);
   }
   /*
    * Check if the character under the cursor is a valid match character.
    */
   match_ch = 0;
   for (i=0;i<strlen((DEFCHAR *)match);i++)
   {
      if (ch == *(match+i))
      {
         direction_backward = (i % 2);
         match_ch = (direction_backward) ? *(match+i-1) : *(match+i+1);
         break;
      }
   }
   if (match_ch == 0)
   {
      display_error(67,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Calculate the actual position of the character in the LINE.
    */
   start_col = focus_column + ((direction_backward) ? (-1) : 1);
   /*
    * Find the focus line linked list entry.
    */
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,focus_line,CURRENT_FILE->number_lines);
   while (curr->next != NULL && curr->prev != NULL)
   {
      if (direction_backward)
      {
         for (i=start_col;i>(-1);i--)
         {
            if (*(curr->line+i) == ch)
               matches++;
            else
            {
               if (*(curr->line+i) == match_ch)
               {
                  matches--;
               }
            }
            if (matches == 0)       /* found matching one */
            {
               match_col = i;
               break;
            }
         }
         if (match_col != (-1))
            break;
         curr = curr->prev;
         offset--;
         start_col = curr->length;
      }
      else
      {
         for (i=start_col;i<curr->length;i++)
         {
            if (*(curr->line+i) == ch)
               matches++;
            else
            {
               if (*(curr->line+i) == match_ch)
               {
                  matches--;
               }
            }
            if (matches == 0)       /* found matching one */
            {
               match_col = i;
               break;
            }
         }
         if (match_col != (-1))
            break;
         curr = curr->next;
         offset++;
         start_col = 0;
      }
   }
   /*
    * If no match found, return with error.
    */
   if (match_col == (-1))  /* no match found */
   {
      display_error(68,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_TARGET_NOT_FOUND);
   }
   /*
    * If we get here, we have found the matching character, so we have to
    * move the cursor to the new column and/or line.
    */
   if (offset == 0L)
   {
      if (use_current)
         CURRENT_VIEW->current_column = match_col+1;
      if (match_col >= CURRENT_VIEW->verify_col-1
      &&  match_col <= (CURRENT_SCREEN.cols[WINDOW_FILEAREA]+(CURRENT_VIEW->verify_col-1))-1)
      {
         /*
          * If the new cursor position is in the same panel and on the same line
          * just move the cursor there and get out.
          */
         if (use_current)
         {
            build_screen(current_screen); 
            if ( curses_started )
            {
               display_screen(current_screen);
               wmove(CURRENT_WINDOW,y,x);
            }
         }
         else
         {
            if ( curses_started )
            {
               wmove(CURRENT_WINDOW,y,match_col-(CURRENT_VIEW->verify_col-1));
            }
         }
         TRACE_RETURN();
         return(RC_OK);
      }
      else
      {
         x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
         CURRENT_VIEW->verify_col = max(1,match_col-(short)x);
         build_screen(current_screen); 
         if ( curses_started )
         {
            display_screen(current_screen);
            wmove(CURRENT_WINDOW,y,(match_col-(CURRENT_VIEW->verify_col-1)));
         }
         TRACE_RETURN();
         return(RC_OK);
      }
   }
   /*
    * If a match IS found on a different line, further checks are required
    * for SCOPE.
    */
   if (IN_SCOPE(CURRENT_VIEW,curr))
   {
      /*
       * Set the cursor position for the matching character.
       */
      if (use_current)
      {
         CURRENT_VIEW->current_column = match_col+1;
         CURRENT_VIEW->current_line += offset;
         if (line_in_view(current_screen,CURRENT_VIEW->focus_line))
         {
            y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                                  CURRENT_VIEW->current_row);
         }
         else
         {
            CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
            y = CURRENT_VIEW->current_row;
         }
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
      }
      else
      {
         CURRENT_VIEW->focus_line += offset;
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         if (line_in_view(current_screen,CURRENT_VIEW->focus_line))
         {
            y = get_row_for_focus_line(current_screen,CURRENT_VIEW->focus_line,
                                  CURRENT_VIEW->current_row);
         }
         else
         {
            CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
            y = CURRENT_VIEW->current_row;
         }
      }
   }
   else
   {
      if (CURRENT_VIEW->scope_all)
      {
         curr->select = CURRENT_VIEW->display_low;
         if (use_current)
         {
            CURRENT_VIEW->current_column = match_col+1;
            CURRENT_VIEW->current_line += offset;
            CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
         }
         else
         {
            CURRENT_VIEW->focus_line += offset;
            CURRENT_VIEW->current_line = CURRENT_VIEW->focus_line;
         }
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
         y = CURRENT_VIEW->current_row;
      }
      else
      {
         display_error(68,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_TARGET_NOT_FOUND);
      }
   }
   if (match_col >= CURRENT_VIEW->verify_col-1
   &&  match_col <= (CURRENT_SCREEN.cols[WINDOW_FILEAREA]+(CURRENT_VIEW->verify_col-1))-1)
      x = match_col-(CURRENT_VIEW->verify_col-1);
   else
   {
      x = CURRENT_SCREEN.cols[WINDOW_FILEAREA] / 2;
      CURRENT_VIEW->verify_col = max(1,match_col-(short)x);
      x = (match_col-(CURRENT_VIEW->verify_col-1));
   }

   build_screen(current_screen); 
   if ( curses_started )
   {
      display_screen(current_screen);
      wmove(CURRENT_WINDOW,y,x);
   }
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     cmsg - display text on command line

SYNTAX
     CMSG [text]

DESCRIPTION
     The CMSG command, primarily used in macros, displays 'text' on the
     command line.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <EMSG>, <MSG>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cmsg(CHARTYPE *params)
#else
short Cmsg(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("comm1.c:   Cmsg");
   memset(cmd_rec,' ',max_line_length);
   cmd_rec_len = strlen((DEFCHAR *)params);
   memcpy(cmd_rec,params,cmd_rec_len);
   if (curses_started
   &&  CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
   {
      wmove(CURRENT_WINDOW_COMMAND,0,0);
      my_wclrtoeol(CURRENT_WINDOW_COMMAND);
      put_string(CURRENT_WINDOW_COMMAND,0,0,cmd_rec,cmd_rec_len);
      clear_command = FALSE;
   }
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     command - execute a command without translation

SYNTAX
     COMMAND command [options]

DESCRIPTION
     The COMMAND command executes the specified 'command' without
     synonym or macro translation. THE does not attempt to execute 
     the command as a <macro> even if <SET IMPMACRO> is ON. The 
     command will be passed to the operating system if <SET IMPOS> 
     is ON.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THECommand(CHARTYPE *params)
#else
short THECommand(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   THECommand");
   rc = command_line(params,COMMAND_ONLY_TRUE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     compress - reduce spaces to tabs

SYNTAX
     COMPress [target]

DESCRIPTION
     The COMPRESS command reduces multiple occurrences of spaces and
     replaces them with tab characters in the <'target'> lines.  
     The current tab columns (set by <SET TABS>) are used in 
     determining where tab characters will replaces spaces.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <EXPAND>, <SET TABS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Compress(CHARTYPE *params)
#else
short Compress(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Compress");
   rc = execute_expand_compress(params,FALSE,TRUE,TRUE,TRUE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     controlchar - allow control characters to be entered

SYNTAX
     CONTROLChar

DESCRIPTION
     The CONTROLCHAR command prompts the user to enter a control 
     character; an ASCII character between 1 and 31 inclusive.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short ControlChar(CHARTYPE *params)
#else
short ControlChar(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   unsigned short y=0,x=0;
   int key=0;

   TRACE_FUNCTION("comm1.c:   ControlChar");
   getyx(CURRENT_WINDOW,y,x);
   /*
    * If in the MAIN window, this command can only be issued on a real
    * line.
    */
   if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
      if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
      {
         display_error(38,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
      }
   }
   display_prompt((CHARTYPE *)"Press the character you require.");
   wmove(CURRENT_WINDOW,y,x);
   wrefresh(CURRENT_WINDOW);
   for ( ; ; )
   {
      key = my_getch(CURRENT_WINDOW);
      break;
   }
   clear_msgline(-1);
   if (islower(key))
      key = toupper(key);
   if (key >= (int)'A'    /* was '@' for ASCII 0, but Text() command fails */
   &&  key <= (int)'_')
   {
      TRACE_RETURN();
      return((RAW_KEY*2)+(short)key-(short)'@');
   }
   display_error(69,(CHARTYPE *)"- must be between 'A' and '_'",FALSE);
   TRACE_RETURN();
   return(RC_INVALID_OPERAND);
}
/*man-start*********************************************************************
COMMAND
     copy - copies text from one position to another

SYNTAX
     COPY target1 target2
     COPY BLOCK [RESET]

DESCRIPTION
     With the first form of the COPY command, text is copied from
     'target1' to the line specified by 'target2'. Text can
     only be copied within the same view of the file.

     The second form of the COPY command copies text within the
     currently marked block to the current cursor position.
     The text can be in the same file or a different file.

COMPATIBILITY
     XEDIT: COPY BLOCK not available.
     KEDIT: Adds extra functionality with [RESET] option.
            With the cursor in the marked block this command in KEDIT
            acts like <DUPLICATE> BLOCK.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Copy(CHARTYPE *params)
#else
short Copy(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   CHARTYPE reset_block=SOURCE_UNKNOWN;
   short rc=RC_OK;
   LINETYPE start_line=0L,end_line=0L,true_line=0L,lines_affected=0L;
   VIEW_DETAILS *source_view=NULL,*dest_view=NULL;
   TARGET target1,target2;
   short target_type1=TARGET_NORMAL|TARGET_BLOCK_ANY|TARGET_ALL|TARGET_SPARE;
   short target_type2=TARGET_NORMAL;
   bool lines_based_on_scope=FALSE;

   TRACE_FUNCTION("comm1.c:   Copy");
   initialise_target(&target1);
   initialise_target(&target2);
   if ((rc = validate_target(params,&target1,target_type1,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
      free_target(&target1);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If there is no second argument, the only valid target type for the
    * first argument then is BLOCK.
    */
   if (target1.spare == (-1))
   {
      if (target1.rt[0].target_type != TARGET_BLOCK_ANY
      &&  target1.rt[0].target_type != TARGET_BLOCK_CURRENT)
      {
         free_target(&target1);
         display_error(3,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      else
         reset_block = SOURCE_BLOCK;
   }
   else
   {
      if (equal((CHARTYPE *)"reset",strtrunc(target1.rt[target1.spare].string),5))
         reset_block = SOURCE_BLOCK_RESET;
      else
         reset_block = SOURCE_COMMAND;
   }
   /*
    * Validate the arguments following the target...
    */
   switch(reset_block)
   {
      case SOURCE_BLOCK:
      case SOURCE_BLOCK_RESET:
         /*
          * For box blocks, call the appropriate function...
          */
         if (MARK_VIEW->mark_type != M_LINE)
         {
            free_target(&target1);
            box_operations(BOX_C,reset_block,FALSE,' ');
            TRACE_RETURN();
            return(RC_OK);
         }
         source_view = MARK_VIEW;
         dest_view = CURRENT_VIEW;
         start_line = MARK_VIEW->mark_start_line;
         end_line = MARK_VIEW->mark_end_line;
         true_line = get_true_line(FALSE);
         lines_based_on_scope = FALSE;
         break;
      default:
         if ((rc = validate_target(target1.rt[target1.spare].string,&target2,target_type2,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
         {
            free_target(&target2);
            TRACE_RETURN();
            return(rc);
         }
         source_view = CURRENT_VIEW;
         dest_view = CURRENT_VIEW;
#if 0
         if (TOF(target1.true_line))
         {
            target1.true_line = 1L;
            target1.num_lines--;
         }
#endif
         start_line = target1.true_line;
         if (target1.num_lines < 0)
            end_line = (target1.true_line + target1.num_lines) + 1L;
         else
            end_line = (target1.true_line + target1.num_lines) - 1L;
         true_line = target2.true_line + target2.num_lines;
         lines_based_on_scope = TRUE;
         break;
   }
   free_target(&target1);
   free_target(&target2);
   /*
    * If the destination line for the copy is the *** Bottom of File ***
    * line, then subtract 1 to ensure lines don't get copied below the
    * *** Bottom of File *** line.
    */
   if (BOF(true_line))
      true_line--;
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
   rc = rearrange_line_blocks(COMMAND_COPY,(CHARTYPE)reset_block,start_line,
                            end_line,true_line,1,source_view,dest_view,lines_based_on_scope,
                            &lines_affected);

   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     coverlay - overlay text starting at the column pointer

SYNTAX
     COVerlay text

DESCRIPTION
     The COVERLAY command overlays the supplied 'text' onto the
     characters following the column position.

     Spaces in the 'text' do not destroy the existing characters.

     An underscore character "_" in the 'text' places a space in the
     corresponding character position. Therefore you cannot use the
     COVERLAY command to place underscores in a line.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Coverlay(CHARTYPE *params)
#else
short Coverlay(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Coverlay");
   rc = column_command(params,COLUMN_COVERLAY);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     creplace - replace text starting at the column pointer

SYNTAX
     CReplace text

DESCRIPTION
     The CREPLACE command replaces the current characters after the
     column pointer with the supplied 'text'.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Creplace(CHARTYPE *params)
#else
short Creplace(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm1.c:   Creplace");
   rc = column_command(params,COLUMN_CREPLACE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     cursor - move cursor to specified position

SYNTAX
     CURsor Column
     CURsor Screen UP|DOWN|LEFT|RIGHT
     CURsor Screen row [col]
     CURsor [Escreen] UP|DOWN
     CURsor [Escreen|Kedit] LEFT|RIGHT
     CURsor [Escreen] row [col]
     CURsor CUA UP|DOWN|LEFT|RIGHT
     CURsor CMdline [n]
     CURsor HOME [SAVE]
     CURsor File line [col]
     CURsor GOTO line col
     CURsor Mouse

DESCRIPTION
     The CURSOR command allows the user to specify where the cursor
     is to be positioned.

     CURSOR 'Column' moves the cursor to the current column of the
     <focus line>.

     CURSOR 'Screen' 'UP'|'DOWN'|'LEFT'|'RIGHT' moves the cursor in the
     indicated direction one line or column. If the cursor is
     positioned on the first or last line of the screen, the cursor
     wraps to the first or last enterable line. If the cursor is
     positioned on the left or right edges of the screen, the cursor
     moves to the left or right edge of the screen on the same line.

     CURSOR 'Screen' 'row' ['col'] is similar to CURSOR 'Escreen'
     'row' ['col'], but all coordinates are relative the the top left 
     corner of the screen, not the top left corner of the
     <filearea>. Hence, 1,1 would be an invalid cursor position because 
     it would result in the cursor being moved to the <idline>.
     Specification of 'row' and/or 'col' outside the boundaries of the
     logical window is regarded as an error.

     CURSOR ['Escreen'] 'UP'|'DOWN'|'LEFT'|'RIGHT' is similar to CURSOR 
     'Screen' 'UP'|'DOWN'|'LEFT'|'RIGHT', except that where scrolling 
     of the window is possible, then scrolling will take place.

     CURSOR ['Escreen'] 'row' ['col'] moves the cursor to the specified
     'row'/'col' position within the <filearea>. The top left corner of
     the <filearea> is 1,1.
     'row' and 'col' may be specified as '=', which will default to the
     current row and/or column position.
     If 'row' or 'col' are greater than the maximum number of rows or
     columns in the <filearea>, the cursor will move to the last
     row/column available.
     If the specified 'row' is a <reserved line>, <scale line> or <tab line>
     an error will be displayed.
     If the 'row' specified is above the <Top-of-File line> or below the
     <Bottom-of-File line> the cursor will be placed on the closest
     one of these lines.

     CURSOR 'Kedit' 'LEFT'|'RIGHT' mimics the default behaviour of 
     CURL and CURR in KEDIT.

     CURSOR 'CUA' 'UP'|'DOWN'|'LEFT'|'RIGHT' moves the cursor in the
     indicated direction one line or column. The behaviour of the
     cursor at the the end of a line and at the start of a line is
     consistent with the Common User Access (CUA) definition. 

     CURSOR 'CMdline' moves the cursor to the indicated column of the
     <command line>.

     CURSOR 'HOME' moves the cursor to the first column of the <command line> 
     (if not on the command line), or to the last row/column of
     the <filearea> if on the command line. With the ['SAVE'] option,
     the cursor will move to the last row/column of the <filearea> or
     <prefix area> (which ever was the last position) if on the
     <command line>.

     CURSOR 'File' moves the cursor to the line and column of the file.
     If the line and/or column are not currently displayed, an error
     message is displayed.

     CURSOR 'GOTO' moves the cursor to the specified line and column
     of the file, whether the row and column are currently displayed or 
     not.  If the 'line' and 'col' are currently displayed, then this
     command behaves just like CURSOR 'FIle'. If not, then the 
     <current line> will be changed to the specified <line>.

     CURSOR 'Mouse' moves the cursor to the position where a mouse button
     was last activated.  This command is specific to THE.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible. Added GOTO option.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Cursor(CHARTYPE *params)
#else
short Cursor(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   register short idx=0;
#define CUR_PARAMS  4
   CHARTYPE *word[CUR_PARAMS+1];
   CHARTYPE strip[CUR_PARAMS];
   unsigned short num_params=0;
   bool time_to_leave=FALSE;
   short error_number=1;
   CHARTYPE *error_message=(CHARTYPE *)"";
   short colno=1;
   short rc=RC_OK;
   short state=CURSOR_START;
   short row=0,col=0;
   LINETYPE line=0L;

   TRACE_FUNCTION("comm1.c:   Cursor");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(params,word,CUR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params ==0)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   error_message = word[0];
   state = CURSOR_START;
   idx = 0;
   while(1)
   {
      switch(state)
      {
         case CURSOR_START:
            if (equal((CHARTYPE *)"escreen",word[idx],1))
            {
               state = CURSOR_ESCREEN;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"screen",word[idx],1))
            {
               state = CURSOR_SCREEN;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"cua",word[idx],3))
            {
               state = CURSOR_CUA;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"left",word[idx],4))
            {
               if (num_params > 1)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_left( CURSOR_ESCREEN, FALSE );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"right",word[idx],5))
            {
               if (num_params > 1)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_right( CURSOR_ESCREEN, FALSE );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"up",word[idx],2))
            {
               if (num_params > 1)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_up( CURSOR_ESCREEN );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"down",word[idx],4))
            {
               if (num_params > 1)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_down( CURSOR_ESCREEN );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"home",word[idx],4))
            {
               state = CURSOR_HOME;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"column",word[idx],1))
            {
               state = CURSOR_COLUMN;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"cmdline",word[idx],2))
            {
               state = CURSOR_CMDLINE;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"file",word[idx],1))
            {
               state = CURSOR_FILE;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"goto",word[idx],4))
            {
               state = CURSOR_GOTO;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"kedit",word[idx],1))
            {
               state = CURSOR_KEDIT;
               idx++;
               break;
            }
            if (equal((CHARTYPE *)"mouse",word[idx],1))
            {
               state = CURSOR_MOUSE;
               idx++;
               break;
            }
            state = CURSOR_ESCREEN;
            break;
         case CURSOR_HOME:
            if (num_params > 2)
            {
               state = CURSOR_ERROR;
               error_message = word[idx];
               error_number = 1;
               break;
            }
            if (num_params == 2
            &&  !equal((CHARTYPE *)"save",word[1],4))
            {
               state = CURSOR_ERROR;
               error_number = 1;
               error_message = word[idx];
               break;
            }
            if (num_params == 2)
               rc = THEcursor_home(TRUE);
            else
               rc = THEcursor_home(FALSE);
            time_to_leave = TRUE;
            break;
         case CURSOR_COLUMN:
            if (num_params != 1)
            {
               state = CURSOR_ERROR;
               error_message = word[idx];
               error_number = 1;
               break;
            }
            rc = THEcursor_column();
            time_to_leave = TRUE;
            break;
         case CURSOR_FILE:
            if (num_params > 3)
            {
               state = CURSOR_ERROR;
               error_message = (CHARTYPE *)"";
               error_number = 2;
               break;
            }
            if (num_params < 2)
            {
               state = CURSOR_ERROR;
               error_message = (CHARTYPE *)"";
               error_number = 3;
               break;
            }
            if (!valid_positive_integer(word[1]))
              {
               state = CURSOR_ERROR;
               error_message = word[1];
               error_number = 4;
               break;
              }
            line = atol((DEFCHAR *)word[1]);
            if (num_params == 3)
            {
               if (!valid_positive_integer(word[2]))
               {
                  state = CURSOR_ERROR;
                  error_message = word[2];
                  error_number = 4;
                  break;
               }
               col = atoi((DEFCHAR *)word[2]);
            }
            else
               col = 0;
            rc = THEcursor_file(TRUE,line,col);
            time_to_leave = TRUE;
            break;
         case CURSOR_GOTO:
            if (num_params > 3)
            {
               state = CURSOR_ERROR;
               error_message = (CHARTYPE *)"";
               error_number = 2;
               break;
            }
            if (num_params < 3)
            {
               state = CURSOR_ERROR;
               error_message = (CHARTYPE *)"";
               error_number = 3;
               break;
            }
            if (!valid_positive_integer(word[1]))
              {
               state = CURSOR_ERROR;
               error_message = word[1];
               error_number = 4;
               break;
              }
            line = atol((DEFCHAR *)word[1]);
            if (num_params == 3)
            {
               if (!valid_positive_integer(word[2]))
               {
                  state = CURSOR_ERROR;
                  error_message = word[2];
                  error_number = 4;
                  break;
               }
               col = atoi((DEFCHAR *)word[2]);
            }
            else
               col = 0;
            rc = THEcursor_goto(line,col);
            time_to_leave = TRUE;
            break;
         case CURSOR_MOUSE:
            if (num_params != 1)
            {
               state = CURSOR_ERROR;
               error_message = word[idx];
               error_number = 1;
               break;
            }
            rc = THEcursor_mouse();
            time_to_leave = TRUE;
            break;
         case CURSOR_CMDLINE:
            if (num_params > 2)
            {
               state = CURSOR_ERROR;
               error_message = word[idx];
               error_number = 1;
               break;
            }
            if (num_params == 2)
            {
               colno = atoi((DEFCHAR *)word[idx]);
               if (colno < 1)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
            }
            rc = THEcursor_cmdline(colno);
            time_to_leave = TRUE;
            break;
         case CURSOR_SCREEN:
         case CURSOR_ESCREEN:
         case CURSOR_CUA:
            if (equal((CHARTYPE *)"left",word[idx],4))
            {
               if (num_params > 2)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_left( state , FALSE );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"right",word[idx],5))
            {
               if (num_params > 2)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_right( state, FALSE );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"up",word[idx],2))
            {
               if (num_params > 2)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_up( state );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"down",word[idx],4))
            {
               if (num_params > 2)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_down( state );
               time_to_leave = TRUE;
               break;
            }
            /*
             * Only CURSOR SCREEN and CURSOR ESCREEN is valid here.
             * If CURSOR CUA, report an error
             */
            if ( state == CURSOR_CUA )
            {
               state = CURSOR_ERROR;
               error_message = word[idx];
               error_number = 1;
               break;
            }
            if ( strcmp( (DEFCHAR *)word[idx], (DEFCHAR *)EQUIVCHARstr) == 0 )
               row = 0;
            else
            {
               if (!valid_positive_integer(word[idx]))
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 4;
                  break;
               }
               row = atoi((DEFCHAR *)word[idx]);
               if (row == 0)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 5;
                  break;
               }
            }
            idx++;
            if (strcmp((DEFCHAR *)word[idx],"") == 0)
            {
               if ( state == CURSOR_ESCREEN )
                  col = 1;
               else
                  col = (CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK) == PREFIX_LEFT ? CURRENT_VIEW->prefix_width + 1 : 1;
            }
            else
            {
               if (strcmp((DEFCHAR *)word[idx],EQUIVCHARstr) == 0)
                  col = 0;
               else
               {
                  if (!valid_positive_integer(word[idx]))
                  {
                     state = CURSOR_ERROR;
                     error_message = word[idx];
                     error_number = 4;
                     break;
                  }
                  col = atoi((DEFCHAR *)word[idx]);
                  if (col == 0)
                  {
                     state = CURSOR_ERROR;
                     error_message = word[idx];
                     error_number = 5;
                     break;
                  }
               }
            }
            rc = THEcursor_move(TRUE,(state==CURSOR_ESCREEN)?TRUE:FALSE,row,col);
            time_to_leave = TRUE;
            break;
         case CURSOR_KEDIT:
            if (equal((CHARTYPE *)"left",word[idx],4))
            {
               if (num_params > 2)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_left( CURSOR_ESCREEN, TRUE );
               time_to_leave = TRUE;
               break;
            }
            if (equal((CHARTYPE *)"right",word[idx],5))
            {
               if (num_params > 2)
               {
                  state = CURSOR_ERROR;
                  error_message = word[idx];
                  error_number = 1;
                  break;
               }
               rc = THEcursor_right( CURSOR_ESCREEN, TRUE );
               time_to_leave = TRUE;
               break;
            }
            state = CURSOR_ERROR;
            error_message = word[idx];
            error_number = 1;
            break;
         case CURSOR_ERROR:
            display_error(error_number,error_message,FALSE);
            rc = RC_INVALID_OPERAND;
            time_to_leave = TRUE;
            break;
      }
      if (time_to_leave)
         break;
   }
   TRACE_RETURN();
   return(rc);
}
