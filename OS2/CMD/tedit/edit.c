/***********************************************************************/
/* EDIT.C - The body of the program.                                   */
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

static char RCSid[] = "$Id: edit.c,v 1.12 2002/06/09 09:26:16 mark Exp $";

#include <the.h>
#include <proto.h>

bool prefix_changed=FALSE;

/***********************************************************************/
#ifdef HAVE_PROTO
void editor(void)
#else
void editor()
#endif
/***********************************************************************/
{
   short y=0,x=0;

   TRACE_FUNCTION("edit.c:    editor");
   /*
    * Reset any command line positioning parameters so only those files
    * edited from the command line take on the line/col, command line
    * position.
    */
   startup_line = startup_column = 0;

   if (display_screens > 1)
      display_screen(other_screen);

   getyx(CURRENT_WINDOW,y,x);
   wmove(CURRENT_WINDOW,y,x);
   wrefresh(CURRENT_WINDOW);
   if (error_on_screen)
   {
      if (error_window != NULL)
      {
         touchwin(error_window);
         wrefresh(error_window);
         wmove(CURRENT_WINDOW,y,x);
         wrefresh(CURRENT_WINDOW);
      }
   }
#ifdef MSWIN
   draw_cursor(TRUE);
#endif

   for ( ; ; )
   {
      if ( process_key( -1, FALSE ) != RC_OK )
         break;
   }
   TRACE_RETURN();
   return;
}

/***********************************************************************/
#ifdef HAVE_PROTO
int process_key(int key, bool mouse_details_present)
#else
int process_key(key, mouse_details_present)
int key;
bool mouse_details_present;
#endif
/***********************************************************************/
{
   unsigned short x=0,y=0;
   short rc=RC_OK;
   CHARTYPE string_key[2];

   TRACE_FUNCTION("edit.c:    process_key");
#if defined(USE_EXTCURSES)
   getyx(CURRENT_WINDOW,y,x);
   wmove(CURRENT_WINDOW,y,x);
   wrefresh(CURRENT_WINDOW);
#endif
   string_key[1] = '\0';

#ifdef CAN_RESIZE
   if (is_termresized())
   {
      (void)THE_Resize(0,0);
      (void)THERefresh((CHARTYPE *)"");
   }
#endif
#ifdef THE_SINGLE_INSTANCE_ENABLED
   if ( single_instance_server )
      key = process_fifo_input( key );
#endif
   if (key == (-1))
   {
      key = my_getch(stdscr);
   }
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
   if (key == KEY_MOUSE)
   {
      int b,ba,bm;
      if (get_mouse_info(&b,&ba,&bm) != RC_OK)
      {
         TRACE_RETURN();
         return(RC_OK);
      }
   }
   else
   {
      if (!mouse_details_present)
         reset_saved_mouse_pos();
   }
#endif

#ifdef CAN_RESIZE
   if (is_termresized())
   {
      TRACE_RETURN();
      return(RC_OK);
   }
#endif

   initial = FALSE;                 /* set first time a key is requested */
   if (error_on_screen)
      clear_msgline(key);
   if (current_key == -1)
      current_key = 0;
   else
      current_key = (current_key == 7) ? 0 : current_key++;
   lastkeys[current_key] = key;
   rc = function_key(key,OPTION_NORMAL,mouse_details_present);
   if (number_of_files == 0)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }

   if (rc >= RAW_KEY)
   {
      if (rc > RAW_KEY)
         key = rc - (RAW_KEY*2);
      if (key < 256 && key >= 0)
      {
         string_key[0] = (CHARTYPE)key;
         /*
          * If operating in CUA mode, and a CUA block exists, check
          * if the block should be reset or deleted before executing
          * the command.
          */
         if ( INTERFACEx == INTERFACE_CUA
         &&  CURRENT_VIEW->mark_type == M_CUA )
         {
            ResetOrDeleteCUABlock( CUA_DELETE_BLOCK );
         }
         (void)Text(string_key);
      }
   }

   show_statarea();

   if ( FILETABSx )
      display_filetabs( NULL );

   if (display_screens > 1
   &&  SCREEN_FILE(0) == SCREEN_FILE(1))
   {
      build_screen(other_screen);
      display_screen(other_screen);
/*    refresh_screen(other_screen);*/
      show_heading(other_screen);
   }
   refresh_screen(current_screen);
   if (error_on_screen)
   {
      getyx(CURRENT_WINDOW,y,x);
      if (error_window != NULL)
      {
         touchwin(error_window);
         wnoutrefresh(error_window);
      }
      wmove(CURRENT_WINDOW,y,x);
      wnoutrefresh(CURRENT_WINDOW);
   }

#ifdef HAVE_BROKEN_SYSVR4_CURSES
   getyx(CURRENT_WINDOW,y,x);
   wmove(CURRENT_WINDOW,y,x);
   wrefresh(CURRENT_WINDOW);
#else
   doupdate();
#endif
   TRACE_RETURN();
   return(RC_OK);
}

/***********************************************************************/
#ifdef HAVE_PROTO
short EditFile(CHARTYPE *fn, bool external_command_line)
#else
short EditFile(fn, external_command_line)
CHARTYPE *fn;
bool external_command_line;
#endif
/***********************************************************************/
{
   short rc=RC_OK,y=0,x=0;
   VIEW_DETAILS *save_current_view=NULL;
   VIEW_DETAILS *previous_current_view=NULL;
   CHARTYPE save_prefix=0;
   short save_gap=0;
   ROWTYPE save_cmd_line=0;
   bool save_id_line=0;

   TRACE_FUNCTION("edit.c:    EditFile");
   /*
    * With no arguments, edit the next file in the ring...
    */
   if (strcmp((DEFCHAR *)fn,"") == 0)
   {
      rc = advance_view(NULL,DIRECTION_FORWARD);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * With "-" as argument, edit the previous file in the ring...
    */
   if (strcmp((DEFCHAR *)fn,"-") == 0)
   {
      rc = advance_view(NULL,DIRECTION_BACKWARD);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If there are still file(s) in the ring, clear the command line and
    * save any changes to the focus line.
    */
   if (number_of_files > 0)
   {
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL,TRUE);
      memset(cmd_rec,' ',max_line_length);
      cmd_rec_len = 0;
   }
   previous_current_view = CURRENT_VIEW;
   /*
    * Save the position of the cursor for the current view before getting
    * the contents of the new file...
    */
   if (curses_started
   &&  number_of_files > 0)
   {
      if (CURRENT_WINDOW_COMMAND != NULL)
      {
         wmove(CURRENT_WINDOW_COMMAND,0,0);
         my_wclrtoeol(CURRENT_WINDOW_COMMAND);
      }
      getyx(CURRENT_WINDOW_FILEAREA,CURRENT_VIEW->y[WINDOW_FILEAREA],CURRENT_VIEW->x[WINDOW_FILEAREA]);
      if (CURRENT_WINDOW_PREFIX != NULL)
         getyx(CURRENT_WINDOW_PREFIX,CURRENT_VIEW->y[WINDOW_PREFIX],CURRENT_VIEW->x[WINDOW_PREFIX]);
   }
   if (number_of_files > 0)
   {
      save_prefix=CURRENT_VIEW->prefix;
      save_gap=CURRENT_VIEW->prefix_gap;
      save_cmd_line=CURRENT_VIEW->cmd_line;
      save_id_line=CURRENT_VIEW->id_line;
   }
   /*
    * Read the contents of the new file into memory...
    */
   if ((rc = get_file(strrmdup(strtrans(fn,OSLASH,ISLASH),ISLASH,TRUE))) != RC_OK)
   {
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If more than one screen is displayed, sort out which view is to be
    * displayed...
    */
   if (display_screens > 1)
   {
      save_current_view = CURRENT_VIEW;
      CURRENT_SCREEN.screen_view = CURRENT_VIEW = previous_current_view;
      advance_view(save_current_view,DIRECTION_FORWARD);
   }
   else
   {
      if (number_of_files > 0)
      {
         /*
          * If the position of the prefix or command line for the new view is
          * different from the previous view, rebuild the windows...
          */
         if ((save_prefix&PREFIX_LOCATION_MASK) != (CURRENT_VIEW->prefix&PREFIX_LOCATION_MASK)
         ||  save_gap != CURRENT_VIEW->prefix_gap
         ||  save_cmd_line != CURRENT_VIEW->cmd_line
         ||  save_id_line != CURRENT_VIEW->id_line)
         {
            set_screen_defaults();
            if (curses_started)
            {
               if (set_up_windows(current_screen) != RC_OK)
               {
                  TRACE_RETURN();
                  return(RC_OK);
               }
            }
         }
      }
      /*
       * Re-calculate CURLINE for the new view in case the CURLINE is no
       * longer in the display area.
       */
      prepare_view(current_screen);
   }
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);
   build_screen(current_screen);
   /*
    * Position the cursor in the main window depending on the type of file
    */
   if (curses_started)
   {
      if (CURRENT_VIEW->in_ring)
      {
         wmove(CURRENT_WINDOW_FILEAREA,CURRENT_VIEW->y[WINDOW_FILEAREA],CURRENT_VIEW->x[WINDOW_FILEAREA]);
         if (CURRENT_WINDOW_PREFIX != NULL)
            wmove(CURRENT_WINDOW_PREFIX,CURRENT_VIEW->y[WINDOW_PREFIX],CURRENT_VIEW->x[WINDOW_PREFIX]);
         getyx(CURRENT_WINDOW,y,x);
         wmove(CURRENT_WINDOW,y,x);
      }
      else
      {
         if (CURRENT_FILE->pseudo_file == PSEUDO_DIR)
            wmove(CURRENT_WINDOW_FILEAREA,CURRENT_VIEW->current_row,file_start-1);
         else
            wmove(CURRENT_WINDOW_FILEAREA,CURRENT_VIEW->current_row,0);
      }
   }
   /*
    * Execute any profile file...
    */
   if ((REPROFILEx && CURRENT_VIEW->in_ring == FALSE)
   ||  (in_profile && external_command_line))
   {
      profile_file_executions++;
      in_reprofile = TRUE;
      if (execute_profile)
      {
         if (local_prf != (CHARTYPE *)NULL)
            rc = get_profile(local_prf,prf_arg);
      }
      in_reprofile = FALSE;
   }
   /*
    * If the result of processing the profile file results in no files
    * in the ring, we need to get out NOW.
    */
   if (number_of_files == 0)
   {
      TRACE_RETURN();
      return(rc);
   }
/* pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(LINE *)NULL);*/
   build_screen(current_screen);
   /*
    * If startup values were specified on the command, line, move cursor
    * there...
    */
   if (startup_line != 0
   ||  startup_column != 0)
   {
      THEcursor_goto( startup_line, startup_column );
   }
   filetabs_start_view = NULL;
   /*
    * If curses hasn't started, don't try to use curses functions...
    */
   if (curses_started)
   {
      display_screen(current_screen);
      if (CURRENT_WINDOW_COMMAND != NULL)
         wmove(CURRENT_WINDOW_COMMAND,0,0);
      if (CURRENT_WINDOW_PREFIX != NULL)
         touchwin(CURRENT_WINDOW_PREFIX);
      if (CURRENT_WINDOW_GAP != NULL)
         touchwin(CURRENT_WINDOW_GAP);
      if (CURRENT_WINDOW_COMMAND != NULL)
         touchwin(CURRENT_WINDOW_COMMAND);
      if (CURRENT_WINDOW_IDLINE != NULL)
         touchwin(CURRENT_WINDOW_IDLINE);
      touchwin(CURRENT_WINDOW_FILEAREA);
      show_statarea();
/*      display_filetabs( NULL ); */
   }
   /*
    * If we have a Rexx interpreter, register a handler for ring.x where
    * x is the number of files in the ring.
    */
   if (rexx_support)
   {
      CHARTYPE tmp[20];
      sprintf( (DEFCHAR *)tmp, "ring.%d", number_of_files+(compatible_feel==COMPAT_XEDIT) ? 1 : 0 );
      MyRexxRegisterFunctionExe( tmp );
   }
   TRACE_RETURN();
   return(rc);
}
