/***********************************************************************/
/* COMM5.C - Commands T-Z                                              */
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

static char RCSid[] = "$Id: comm5.c,v 1.11 2002/08/13 12:49:22 mark Exp $";

#include <the.h>
#include <proto.h>

#ifdef HAVE_PROTO
static bool ispf_special_lines_entry( short line_type, int ch, CHARTYPE real_key )
#else
static bool ispf_special_lines_entry( line_type, ch, real_key )
short line_type;
int ch;
CHARTYPE real_key;
#endif
{
   bool need_to_build_screen=FALSE;
   /*
    * In ISPF mode, we need to allow certain characters to be
    * typed on the TABS line, and the BNDS line
    * Characters allowed are:
    * Char     TABS       BNDS
    *   <       -          Y
    *   >       -          Y
    *           Y          Y
    * etc... TBD
    *
    */
   switch(line_type)
   {
      case LINE_BOUNDS:
         switch( real_key )
         {
            case ' ':
               /*
                * If the existing character is < or >, then reset that
                * particular edge of the BNDS
                */
               if ( ch == '<' || ch == '>' )
               {
                  need_to_build_screen = TRUE;
               }
               break;
            case '<':
               need_to_build_screen = TRUE;
               break;
            case '>':
               need_to_build_screen = TRUE;
               break;
         }
         break;
      case LINE_TABLINE:
         break;
      default:
         break;
   }
   return need_to_build_screen;
}

/*#define DEBUG 1*/
/*man-start*********************************************************************
COMMAND
     tabfile - edit the file under the file tab or shift FILETABS view

SYNTAX
     TABFILE [+|-]

DESCRIPTION
     The TABFILE makes the file pointed to by the mouse in the FILETABS
     window the current file.

     If run from the command line, without a paramter, the leftmost file
     displayed in the FILETABS window will be made the current file.


     TABFILE -, shifts the files in the FILETABS window one file to the right.
     TABFILE +, shifts the files in the FILETABS window one file to the left.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SET FILETABS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabfile(CHARTYPE *params)
#else
short Tabfile(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   int x,y;
   CHARTYPE scrn;
#endif
   int w;
   VIEW_DETAILS *curr;
   CHARTYPE edit_fname[MAX_FILE_NAME];

   TRACE_FUNCTION("comm5.c:   Tabfile");
   /*
    * Optionally only 1 argument allowed; + or -
    */
   if ( strcmp( (DEFCHAR *)params, "" ) == 0 )
   {
      /*
       * If no parameter it is either called from a mouse click, or we assume we
       * are called from the command line.
       * If called from mouse click, find file under mouse (or arrows)
       * If called from command line, edit first file displayed
       */
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
      which_window_is_mouse_in( &scrn, &w );
#else
      w = -1;
#endif
      if ( w == (-1 ) )
      {
         /*
          * Edit first file displayed.
          */
         curr = find_filetab( -1 );
         if ( curr != NULL )
         {
            strcpy( (DEFCHAR *)edit_fname, (DEFCHAR *)curr->file_for_view->fpath );
            strcat( (DEFCHAR *)edit_fname, (DEFCHAR *)curr->file_for_view->fname );
            rc = EditFile( edit_fname, FALSE );
         }
         TRACE_RETURN();
         return(RC_OK);
      }
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
      /*
       * Get mouse position
       */
      wmouse_position( filetabs, &y, &x );
      curr = find_filetab( x );
      if ( curr != NULL )
      {
         strcpy( (DEFCHAR *)edit_fname, (DEFCHAR *)curr->file_for_view->fpath );
         strcat( (DEFCHAR *)edit_fname, (DEFCHAR *)curr->file_for_view->fname );
         rc = EditFile( edit_fname, FALSE );
      }
      /*
       * If curr returned as NULL, then we have either clicked in a blank
       * or on an arrow. If on an arrow, we call this function again with
       * a + or - parameter.
       */
#endif
   }
   else if ( equal( (CHARTYPE *)"+", params, 1 ) )
   {
      filetabs_start_view = find_next_file( filetabs_start_view ? filetabs_start_view : vd_current, DIRECTION_FORWARD );
   }
   else if ( equal( (CHARTYPE *)"-", params, 1 ) )
   {
      filetabs_start_view = find_next_file( filetabs_start_view ? filetabs_start_view : vd_current, DIRECTION_BACKWARD );
   }
   else
   {
      display_error( 1, (CHARTYPE *)params, FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     tabpre - switch between FILEAREA and PREFIX area

SYNTAX
     tabpre

DESCRIPTION
     The TABPRE command switches the focus of the editor from the
     <filearea> to the <prefix area> and vice versa, depending
     on which window is currently active.

     This command can only be used by assigning it to a function key.

     This command will be removed in a future version.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Equivalent of <SOS LEFTEDGE> and <SOS PREFIX>

SEE ALSO
     <SOS LEFTEDGE>, <SOS PREFIX>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tabpre(CHARTYPE *params)
#else
short Tabpre(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm5.c:   Tabpre");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the cursor is in the command line or there is no prefix on, exit.
    */
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND
   ||  !CURRENT_VIEW->prefix)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   if (CURRENT_VIEW->current_window == WINDOW_FILEAREA)
      rc = Sos_prefix((CHARTYPE *)"");
   else
      rc = Sos_leftedge((CHARTYPE *)"");
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     tag - displays lines matching target in different colour

SYNTAX
     TAG [More|Less] [rtarget]

DESCRIPTION
     The TAG command is similar to the <ALL> command, in that it allows
     lines that match the specified target to be displayed.  Where it
     differs to <ALL> is that the lines that don't match are still
     displayed, but the lines that do match are displayed in the colour
     specified by <SET COLOUR> HIGHLIGHT.
     This target consists of any number of individual targets
     seperated by '&' (logical and) or '|' (logical or).

     For example, to display all lines in a file that contain the
     strings 'ball' and 'cat' on the same line or the named lines
     .fred or .bill, use the following command

     TAG /ball/ & /cat/ | .fred | .bill

     Logical operators act left to right, with no precedence for &.

     TAG without any arguments displays all lines without any highlighting.

     If <SET HIGHLIGHT> is not set to TAGGED, then if the specified 'rtarget'
     is found, <SET HIGHLIGHT> is set to TAGGED.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. MORE and LESS options not implemented.

SEE ALSO
     <ALL>, <SET HIGHLIGHT>, <SET COLOUR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Tag(CHARTYPE *params)
#else
short Tag(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   _LINE *curr=NULL;
   bool target_found=FALSE;
   short status=RC_OK;
   short target_type=TARGET_NORMAL;
   TARGET target;
   LINETYPE line_number=0L;
   bool save_scope=FALSE;
   LINETYPE num_lines=0L;

   TRACE_FUNCTION("comm1.c:   Tag");
   if (strlen((DEFCHAR *)params) == 0)
   {
      if (CURRENT_FILE->number_lines == 0L)
      {
         TRACE_RETURN();
         return(rc);
      }
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
      curr = CURRENT_FILE->first_line->next;
      while(1)
      {
         curr->flags.tag_flag = 0;
         curr = curr->next;
         if (curr->next == NULL)
            break;
      }
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
      curr->flags.save_tag_flag = curr->flags.tag_flag;
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
         curr->flags.tag_flag = 1;
      }
      else if (status == RC_TARGET_NOT_FOUND) /* target not found */
         curr->flags.tag_flag = 0;
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
      CURRENT_VIEW->highlight = HIGHLIGHT_TAG;
      build_screen(current_screen);
      display_screen(current_screen);
   }
   else
   {
      curr = CURRENT_FILE->first_line->next;
      while(1)
      {
         curr->flags.tag_flag = curr->flags.save_tag_flag;
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
     text - simulate keyboard entry of characters

SYNTAX
     TEXT text

DESCRIPTION
     The TEXT command simulates the entry of 'text' from the
     keyboard. This command is actually called when you enter text
     from the keyboard.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.
            Does not allow trailing spaces in text.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Text(CHARTYPE *params)
#else
short Text(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   register short i=0;
   CHARTYPE real_key=0;
   chtype chtype_key=0;
   LENGTHTYPE x=0;
   unsigned short y=0;
   short len_params=0;
   short rc=RC_OK;
#if defined(HAVE_BROKEN_COLORS)
   int junky,newx;
#endif
#if defined(USE_EXTCURSES)
   ATTR attr=0;
#else
   chtype attr=0;
#endif
   bool need_to_build_screen=FALSE;
   bool save_in_macro=in_macro;

   TRACE_FUNCTION("comm5.c:   Text");
   /*
    * If running in read-only mode, do not allow any text to be entered
    * in the main window.
    */
   if (ISREADONLY(CURRENT_FILE)
   && CURRENT_VIEW->current_window == WINDOW_FILEAREA)
   {
      display_error(56,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * If HEX mode is on, convert the hex string...
    */
   if ( CURRENT_VIEW->hex
   &&   strlen( (DEFCHAR *)params ) > 3 )
   {
      if ((len_params = convert_hex_strings(params)) == (-1))
      {
         display_error(32,params,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   else
      len_params = strlen((DEFCHAR *)params);
   for (i=0;i<len_params;i++)
   {
      real_key = case_translate((CHARTYPE)*(params+i));
      chtype_key = (chtype)(real_key & A_CHARTEXT);
      getyx(CURRENT_WINDOW,y,x);
#if defined(USE_EXTCURSES)
      attr = CURRENT_WINDOW->_a[y][x];
      wattrset(CURRENT_WINDOW,attr);
      attr = 0;
#else
      attr = winch(CURRENT_WINDOW) & A_ATTRIBUTES;
#endif
      switch(CURRENT_VIEW->current_window)
      {
         case WINDOW_FILEAREA:
            if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
            {
               if ( compatible_feel == COMPAT_ISPF
               && ( CURRENT_SCREEN.sl[y].line_type == LINE_BOUNDS
                 || CURRENT_SCREEN.sl[y].line_type == LINE_TABLINE ) )
               {
                  need_to_build_screen = ispf_special_lines_entry( CURRENT_SCREEN.sl[y].line_type,
                                                                   winch(CURRENT_WINDOW) & A_CHARTEXT,
                                                                   real_key );
               }
               break;
            }
            if ((LENGTHTYPE)(x+CURRENT_VIEW->verify_start) > (LENGTHTYPE)(CURRENT_VIEW->verify_end))
               break;
            if (INSERTMODEx)
            {
               rec = meminschr(rec,real_key,CURRENT_VIEW->verify_col-1+x,
                               max_line_length,rec_len);
               put_char(CURRENT_WINDOW,chtype_key|attr,INSCHAR);
            }
            else
            {
               rec[CURRENT_VIEW->verify_col-1+x] = real_key;
               if (x == CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1)
                  put_char(CURRENT_WINDOW,chtype_key|attr,INSCHAR);
               else
                  put_char(CURRENT_WINDOW,chtype_key|attr,ADDCHAR);
            }
            rc = memrevne(rec,' ',max_line_length);
            if (rc == (-1))
               rec_len = 0;
            else
               rec_len = rc+1;
            /*
             * If THIGHLIGHT on focus line, reset it.
             */
            if ( CURRENT_VIEW->thighlight_on
            &&   CURRENT_VIEW->thighlight_active
            &&   CURRENT_VIEW->thighlight_target.true_line == CURRENT_SCREEN.sl[y].line_number )
            {
               CURRENT_VIEW->thighlight_active = FALSE;
               need_to_build_screen = TRUE;
            }
            /* check for the cursor moving past the right   */
            /* margin when WORDWRAP is ON. If true, then    */
            /* don't execute the THEcursor_right() function, as */
            /* this could cause a window scroll.            */
            if (CURRENT_VIEW->wordwrap
            &&  rec_len > CURRENT_VIEW->margin_right)
               execute_wrap_word(x+CURRENT_VIEW->verify_col);
            else
            {
               /* this is done here so that the show_page() in */
               /* THEcursor_right() is executed AFTER we get the   */
               /* new length of rec_len.                       */
#if defined(USE_EXTCURSES)
               if (x == CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1)
               {
                  wmove(CURRENT_WINDOW,y,x);
     /*           wrefresh(CURRENT_WINDOW); */
                  THEcursor_right(TRUE,FALSE);
               }
#else
               if (INSERTMODEx
               || x == CURRENT_SCREEN.cols[WINDOW_FILEAREA]-1)
               {
                  THEcursor_right(TRUE,FALSE);
# if defined(HAVE_BROKEN_COLORS)
                  /*
                   * AIX curses is broken. It moves the cursor down one
                   * line, probably as a result of put_char() above. We
                   * fix this by getting the new cursor position after
                   * scrolling the screen horizontally, and then position
                   * the cursor with the OLD y value, and the NEW x value;
                   */
                  getyx(CURRENT_WINDOW,junky,newx);
                  wmove(CURRENT_WINDOW,y,newx);
# endif
               }
#endif
            }
            /*
             * If HEXSHOW is on and we are on the current line, build screen...
             */
            if (CURRENT_VIEW->hexshow_on
            &&  CURRENT_VIEW->focus_line == CURRENT_VIEW->current_line)
               need_to_build_screen = TRUE;
            break;
         case WINDOW_COMMAND:
            if (INSERTMODEx)
            {
               cmd_rec = (CHARTYPE *)meminschr((CHARTYPE *)cmd_rec,
                               real_key,x,
                               COLS,cmd_rec_len);
               put_char(CURRENT_WINDOW,chtype_key,INSCHAR);
#if !defined(USE_EXTCURSES)
               THEcursor_right(TRUE,FALSE);
#endif
#ifndef OLD_CMD
               cmd_rec_len++;
#endif
            }
            else
            {
               cmd_rec[x] = real_key;
               put_char(CURRENT_WINDOW,chtype_key,ADDCHAR);
#ifndef OLD_CMD
               cmd_rec_len = max(x+1,cmd_rec_len);
#endif
            }
#ifdef OLD_CMD
            rc = memrevne(cmd_rec,' ',COLS);
            if (rc == (-1))
               cmd_rec_len = 0;
            else
               cmd_rec_len = rc+1;
#endif
            /*
             * Ensure that the command line is not cleared if text is
             * entered here
             */
            clear_command = FALSE;
            break;
         case WINDOW_PREFIX:
            prefix_changed = TRUE;
            if (pre_rec_len == 0)
            {
               x = 0;
               wmove(CURRENT_WINDOW,y,x);
               my_wclrtoeol(CURRENT_WINDOW);
               wrefresh(CURRENT_WINDOW);
            }
            if (INSERTMODEx)
            {
               if (pre_rec_len == (CURRENT_VIEW->prefix_width-CURRENT_VIEW->prefix_gap))
                  break;
               pre_rec = (CHARTYPE *)meminschr((CHARTYPE *)pre_rec,
                               real_key,x,CURRENT_VIEW->prefix_width-CURRENT_VIEW->prefix_gap,pre_rec_len);
               put_char(CURRENT_WINDOW,chtype_key,INSCHAR);
            }
            else
            {
               pre_rec[x] = real_key;
               put_char(CURRENT_WINDOW,chtype_key,ADDCHAR);
            }
            wmove(CURRENT_WINDOW,y,min(x+1,CURRENT_VIEW->prefix_width-CURRENT_VIEW->prefix_gap-1));
            rc = memrevne(pre_rec,' ',CURRENT_VIEW->prefix_width);
            if (rc == (-1))
               pre_rec_len = 0;
            else
               pre_rec_len = rc+1;
            break;
      }
   }
   if (in_macro)
      need_to_build_screen = TRUE;
   /*
    * Set in_macro to FALSE to allow for the refreshing of the screen.
    */
   in_macro = FALSE;
   /*
    * If text is being inserted on a line which is in the marked block,
    * build and redisplay the window.
    */
   if (CURRENT_VIEW == MARK_VIEW
   &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA
   &&  INSERTMODEx
   &&  CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line
   &&  CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)
      need_to_build_screen = TRUE;
   /*
    * If the current file has colouring on and not using the NULL parser,
    * build and redisplay the window.
    */
   if (CURRENT_FILE->colouring
   &&  CURRENT_FILE->parser
   &&  CURRENT_VIEW->current_window == WINDOW_FILEAREA)
      need_to_build_screen = TRUE;
   /*
    * If the current view has boundmarks on, build and redisplay the window.
    */
   if (CURRENT_VIEW->boundmark)
      need_to_build_screen = TRUE;
   /*
    * If we have determined we need to rebuild the screen, do it now.
    * MH: FIXME: Need the ability to rebuild and redisplay one line only
    */
   if (need_to_build_screen
   &&  !in_readv)
   {
      build_screen(current_screen);
      display_screen(current_screen);
   }
   /*
    * Set in_macro back to its original value...
    */
   in_macro = save_in_macro;
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     the - edit another file or switch to next file

SYNTAX
     THE [filename]

DESCRIPTION
     The THE command allows the user to edit another 'file'. The new file
     is placed in the file <ring>. The previous file being edited remains
     in memory and can be returned to by issuing a THE command without
     any parameters. Several files can be edited at once, and all files
     are arranged in a ring, with subsequent THE commands moving through
     the ring, one file at a time.

COMPATIBILITY
     XEDIT: Does not provide options switches.
     KEDIT: Does not provide options switches.

SEE ALSO
     <XEDIT>, <EDIT>, <KEDIT>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     toascii - convert the target from EBCDIC to ASCII

SYNTAX
     TOASCII [target]

DESCRIPTION
     The TOASCII command converts the characters in the target from
     EBCDIC encoding to ASCII coding.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Toascii(CHARTYPE *params)
#else
short Toascii(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   LINETYPE num_lines=0L,true_line=0L,num_actual_lines=0L,i=0L,num_file_lines=0L;
   short  direction=0;
   _LINE *curr=NULL;
   LENGTHTYPE start_col=0,end_col=0;
   short rc=RC_OK;
   TARGET target;
   short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
   bool lines_based_on_scope=TRUE;
   bool adjust_alt=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("comm5.c:   Toascii");
   if (strcmp("",(DEFCHAR *)params) == 0)
      params = (CHARTYPE *)"+1";
   initialise_target(&target);
   if ((rc = validate_target(params,&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(rc);
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   /*
    * Determine in which direction we are working.
    */
   if (target.num_lines < 0L)
   {
      direction = DIRECTION_BACKWARD;
      num_lines = target.num_lines * (-1L);
   }
   else
   {
      direction = DIRECTION_FORWARD;
      num_lines = target.num_lines;
   }
   true_line = target.true_line;
   /*
    * If the target is BLOCK set the left and right margins to be the
    * margins of the BOX BLOCK, otherwise use ZONE settings.
    */
   start_col = CURRENT_VIEW->zone_start-1;
   end_col = CURRENT_VIEW->zone_end-1;
   if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
   {
      num_lines = MARK_VIEW->mark_end_line-MARK_VIEW->mark_start_line+1L;
      true_line = MARK_VIEW->mark_start_line;
      direction = DIRECTION_FORWARD;
      lines_based_on_scope = FALSE;
      if (MARK_VIEW->mark_type != M_LINE)
      {
         start_col = MARK_VIEW->mark_start_col-1;
         end_col   = MARK_VIEW->mark_end_col-1;
      }
   }
   /*
    * Find the current LINE pointer for the true_line.
    * This is the first line to change.
    */
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   /*
    * Change the case for the target lines and columns...
    */
   for (i=0L,num_actual_lines=0L;;i++)
   {
      if (lines_based_on_scope)
      {
         if (num_actual_lines == num_lines)
            break;
      }
      else
      {
         if (num_lines == i)
            break;
      }
      rc = processable_line(CURRENT_VIEW,true_line+(LINETYPE)(i*direction),curr);
      switch(rc)
      {
         case LINE_SHADOW:
            break;
/*       case LINE_TOF_EOF: MH12 */
         case LINE_TOF:
         case LINE_EOF:
            num_actual_lines++;
            break;
         default:
            add_to_recovery_list(curr->line,curr->length);
            if ( MARK_VIEW
            &&  ( MARK_VIEW->mark_type == M_STREAM || MARK_VIEW->mark_type == M_CUA ) )
            {
               int mystart=0,myend=curr->length-1;
               if (true_line + i == MARK_VIEW->mark_start_line)
                  mystart = start_col;
               if (true_line + i == MARK_VIEW->mark_end_line)
                  myend = end_col;
               ebc2asc(curr->line,curr->length,mystart,myend);
            }
            else
               ebc2asc(curr->line,curr->length,start_col,end_col);
            if (rc)
            {
               adjust_alt = TRUE;
               curr->flags.changed_flag = TRUE;
            }
            num_actual_lines++;
            break;
      }
      /*
       * Proceed to the next record, even if the current record not in scope.
       */
      if (direction == DIRECTION_FORWARD)
         curr = curr->next;
      else
         curr = curr->prev;
      num_file_lines += (LINETYPE)direction;
      if (curr == NULL)
         break;
   }
   /*
    * Increment the alteration counts if any lines changed...
    */
   if (adjust_alt)
      increment_alt(CURRENT_FILE);
   /*
    * Display the new screen...
    */
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   resolve_current_and_focus_lines(CURRENT_VIEW,true_line,num_file_lines,direction,TRUE,FALSE);
   TRACE_RETURN();
   return(RC_OK);
}

/*man-start*********************************************************************
COMMAND
     top - move to the top of the file

SYNTAX
     TOP

DESCRIPTION
     The TOP command moves to the very start of the current file.
     The <Top-of-File line> is set to the <current line>.

     TOP is equivalent to <BACKWARD> *.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <BACKWARD>, <BOTTOM>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Top(CHARTYPE *params)
#else
short Top(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_TOF_EOF_REACHED;
 unsigned short x=0,y=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("comm5.c:   Top");
/*---------------------------------------------------------------------*/
/* No arguments are allowed; error if any are present.                 */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)params,"") != 0)
   {
    display_error(1,(CHARTYPE *)params,FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 CURRENT_VIEW->current_line = 0L;
 post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
 build_screen(current_screen);
 if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
    CURRENT_VIEW->focus_line = 0L;
 pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
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
     up - move backward in the file a number of lines

SYNTAX
     Up [relative target]

DESCRIPTION
     The UP command moves the <current line> backwards the number of
     lines specified by the <relative target>. This <relative target> can
     only be a positive integer or the character "*".

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1

SEE ALSO
     <NEXT>, <DOWN>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Up(CHARTYPE *params)
#else
short Up(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
 LINETYPE num_lines=0L,true_line=0L;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("comm5.c:   Up");
 params = MyStrip(params,STRIP_BOTH,' ');
 if (strcmp("",(DEFCHAR *)params) == 0)
    params = (CHARTYPE *)"1";
 true_line = get_true_line(TRUE);
 if (strcmp("*",(DEFCHAR *)params) == 0)
    num_lines = true_line + 1L;
 else
   {
    if (!valid_integer(params))
      {
       display_error(4,params,FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
      }
    num_lines = atol((DEFCHAR *)params);
    if (num_lines < 0L)
      {
       display_error(5,params,FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
      }
   }
 rc = advance_current_or_focus_line(-num_lines);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     uppercase - change lowercase characters to uppercase

SYNTAX
     UPPercase [target]

DESCRIPTION
     The UPPERCASE command changes all lowercase characters in all
     lines up to the <'target'> line to uppercase. All other characters
     remain untouched.

COMPATIBILITY
     XEDIT: Equivalent of UPPERCAS
     KEDIT: Compatible.

SEE ALSO
     <LOWERCASE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Uppercase(CHARTYPE *params)
#else
short Uppercase(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("comm5.c:   Uppercase");
 rc = execute_change_case(params,CASE_UPPER);
 TRACE_RETURN();
 return(rc);
}
/*man-start*********************************************************************
COMMAND
     xedit - edit another file or switch to next file

SYNTAX
     Xedit [file]

DESCRIPTION
     The XEDIT command allows the user to edit another 'file'. The new file
     is placed in the file <ring>. The previous file being edited remains
     in memory and can be returned to by issuing an XEDIT command without
     any parameters. Several files can be edited at once, and all files
     are arranged in a ring, with subsequent XEDIT commands moving through
     the ring, one file at a time.

COMPATIBILITY
     XEDIT: Does not provide options switches.
     KEDIT: Does not provide options switches.

SEE ALSO
     <EDIT>, <THE>, <KEDIT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Xedit(CHARTYPE *params)
#else
short Xedit(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION( "comm5.c:   Xedit" );
   /*
    * Parse any parameters...future work.
    */
   rc = EditFile( params, FALSE );
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     ? - retrieve - return the next/prior command on the command line

SYNTAX
     ?[+|?...]

DESCRIPTION
     The ? command returns the next or prior command from the command
     line ring and displays it on the command line.

     With the ['+'] argument, the next command in the command ring is
     retrieved.

     With no arguments, the previous command entered on the command
     line is retrieved.

     With multiple, concatenated ?s as argument, the pervious command
     entered on the command line is retrieved corresponding to the
     number of ?s entered.

     For Example:
     The command; ????? will retrieve the fifth last command entered.

COMPATIBILITY
     XEDIT: Compatible. Adds extra support for multiple ?s.
     KEDIT: See below..
     This command is bound to the up and down arrows when on the
     command line depending on the setting of <SET CMDARROWS>.

SEE ALSO
     <SET CMDARROWS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Retrieve(CHARTYPE *params)
#else
short Retrieve(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE *current_command=NULL;
 CHARTYPE *save_params=NULL;
 int param_len=0;
 short direction=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("comm5.c:   Retrieve");
/*
 *---------------------------------------------------------------------
 * No parameters, get the last command...
 *---------------------------------------------------------------------
 */
 if (strcmp((DEFCHAR *)params,"") == 0)
    current_command = get_next_command(DIRECTION_FORWARD,1);
 else
   {
    /*
     *---------------------------------------------------------------------
     * Get a copy of the parameters, because we want to manipulate them,
     * and also retain the orignal for error reporting.
     *---------------------------------------------------------------------
     */
    if ((save_params = (CHARTYPE *)my_strdup(params)) == NULL)
      {
       display_error(30,(CHARTYPE *)"",FALSE);
       TRACE_RETURN();
       return(RC_OUT_OF_MEMORY);
      }
    /*
     *---------------------------------------------------------------------
     * Strip all spaces from the parameters.  We want to be able to specify
     * ?  ? ? - as a valid set of arguments, equivalent to ???-
     *---------------------------------------------------------------------
     */
    save_params = MyStrip(save_params,STRIP_ALL,' ');
    param_len = strlen((DEFCHAR *)save_params);
    if (*(save_params+(param_len-1)) == (CHARTYPE)'+')
      {
       *(save_params+(param_len-1)) = '\0';
       direction = DIRECTION_BACKWARD;
      }
    else
      {
       if (*(save_params+(param_len-1)) == (CHARTYPE)'-')
         {
          *(save_params+(param_len-1)) = '\0';
          direction = DIRECTION_FORWARD;
         }
      }
    if (strzne(save_params,(CHARTYPE)'?') != (-1))
      {
       display_error(1,params,FALSE);
       TRACE_RETURN();
       return(RC_INVALID_OPERAND);
      }
    current_command = get_next_command(direction,strlen((DEFCHAR *)save_params)+1);
   }
 if (save_params)
    (*the_free)(save_params);
 wmove(CURRENT_WINDOW_COMMAND,0,0);
 my_wclrtoeol(CURRENT_WINDOW_COMMAND);
 if (current_command != (CHARTYPE *)NULL)
    Cmsg(current_command);
 TRACE_RETURN();
 return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     = - re-execute the last command issued on the command line

SYNTAX
     =

DESCRIPTION
     The = command retrieves the most recently issued command from
     the <command line> and re-executes it.

COMPATIBILITY
     XEDIT: Does not support optional [subcommand] option.
     KEDIT: Does not support optional [command] option.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Reexecute(CHARTYPE *params)
#else
short Reexecute(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm5.c:   Reexecute");
   if (strcmp((DEFCHAR *)params,""))
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Retrieve the last command and execute it.
    */
   rc = command_line(last_command_for_reexecute,COMMAND_ONLY_FALSE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     ! - execute an operating system command

SYNTAX
     ! [command]

DESCRIPTION
     The ! command executes the supplied operating system 'command'
     or runs an interactive shell if no 'command' is supplied.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Equivalent to DOS command.

SEE ALSO
     <DOS>, <OS>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     & - execute and re-display command

SYNTAX
     &[command]

DESCRIPTION
     The & command executes the supplied 'command' in the normal
     way, but when the command completes, instead of clearing
     the THE command line, the command, and the & are
     re-displayed.  This makes it easy to repeat the same
     command, or make changes to it.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
