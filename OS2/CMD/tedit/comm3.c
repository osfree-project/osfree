/***********************************************************************/
/* COMM3.C - Commands K-O                                              */
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

static char RCSid[] = "$Id: comm3.c,v 1.12 2002/08/04 06:03:59 mark Exp $";

#include <the.h>
#include <proto.h>

/*man-start*********************************************************************
COMMAND
     kedit - edit another file or switch to next file

SYNTAX
     Kedit [file]

DESCRIPTION
     The KEDIT command allows the user to edit another 'file'. The new file
     is placed in the file <ring>. The previous file being edited remains
     in memory and can be returned to by issuing a KEDIT command without
     any parameters. Several files can be edited at once, and all files
     are arranged in a ring, with subsequent KEDIT commands moving through
     the ring, one file at a time.

COMPATIBILITY
     XEDIT: Does not provide options switches.
     KEDIT: Does not provide options switches.

SEE ALSO
     <EDIT>, <THE>, <XEDIT>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     left - scroll the screen to the left

SYNTAX
     LEft [n|HALF]

DESCRIPTION
     The LEFT command scrolls the screen to the left.

     If 'n' is supplied, the screen scrolls by that many columns.

     If 'HALF' is specified the screen is scrolled by half the number
     of columns in the <filearea>.

     If 'FULL' is specified the screen is scrolled by the number
     of columns in the <filearea>.

     If no parameter is supplied, the screen is scrolled by one
     column.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <RIGHT>, <RGTLEFT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Left(CHARTYPE *params)
#else
short Left(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   LINETYPE shift_val=1L;

   TRACE_FUNCTION("comm3.c:   Left");
   /*
    * Validate only parameter, HALF or positive integer. 1 if no argument.
    */
   if (equal((CHARTYPE *)"half",params,4))
      shift_val = -(CURRENT_SCREEN.cols[WINDOW_FILEAREA]/2);
   if (equal((CHARTYPE *)"full",params,4))
      shift_val = -(CURRENT_SCREEN.cols[WINDOW_FILEAREA]);
   if (blank_field(params))
      shift_val = (-1L);
   if (shift_val == 1)                 /* argument not HALF or empty ... */
   {
      if (valid_positive_integer(params))
      {
         shift_val = atol((DEFCHAR *)params);
         if (shift_val != 0)
            shift_val = -shift_val;
      }
   }
   if (shift_val == 1)                               /* invalid argument */
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the argument is 0, restore the original verify columns display.
    */
   if (shift_val == 0L)
      CURRENT_VIEW->verify_col = CURRENT_VIEW->verify_start;
   else
      CURRENT_VIEW->verify_col = max(1,CURRENT_VIEW->verify_col+shift_val);
#ifdef MSWIN
   Win31HScroll(CURRENT_VIEW->verify_col);
#endif
   build_screen(current_screen);
   display_screen(current_screen);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     locate - search for a target

SYNTAX
     [Locate] target [command]

DESCRIPTION
     The LOCATE command searches for the next or previous occurrence
     of the specified <'target'>.  If no parameter is supplied, LOCATE
     uses the the last target specified. If no prior target has been
     specified, an error message is displayed.

     <target> can also be specified as a regular expression. The syntax of
     this is "Regexp /re/". eg LOCATE RE /[0-9].*$/

     With an optional 'command', this command is executed after finding
     the <'target'>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Locate(CHARTYPE *params)
#else
short Locate(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Locate");
   /*
    * If no parameter is specified, use the last_target. If that doesn't
    * exist, error.
    */
   if (blank_field(params))
   {
      if (blank_field(lastop[LASTOP_LOCATE].value))
      {
         display_error(39,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      rc = execute_locate( lastop[LASTOP_LOCATE].value, TRUE );
      TRACE_RETURN();
      return(rc);
   }
   /*
    * Here we have some parameters.
    */
   rc = execute_locate( params, TRUE );
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     lowercase - change uppercase characters to lowercase

SYNTAX
     LOWercase [target]

DESCRIPTION
     The LOWERCASE command changes all uppercase characters in all
     lines up to the <'target'> line to lowercase. All other characters
     remain untouched.

COMPATIBILITY
     XEDIT: Equivalent of LOWERCAS command.
     KEDIT: Compatible.

SEE ALSO
     <UPPERCASE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Lowercase(CHARTYPE *params)
#else
short Lowercase(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Lowercase");
   rc = execute_change_case(params,CASE_LOWER);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     ls - list the specified directory as an editable file

SYNTAX
     LS [file specification]

DESCRIPTION
     The LS command displays all files matching the specified
     'file specification'.

     When no parameter is supplied, all files in the current directory
     are displayed subject to any <SET DIRINCLUDE> restrictions.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <DIRECTORY>, <SET DIRINCLUDE>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     macro - execute a macro command file

SYNTAX
     MACRO [?] filename [arguments ...]

DESCRIPTION
     The MACRO command executes the contents of the specified 'filename'
     as command line commands. The 'filename' can contain either a series
     of THE commands, or can be a Rexx program. The 'filename' is considered
     a <macro>.

     Rexx macros can be passed optional 'arguments'.

     With the optional '?' parameter, interactive tracing of the Rexx
     macro is possible, but this does not set interactive tracing on;

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Macro(CHARTYPE *params)
#else
short Macro(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   short macrorc=0;

   TRACE_FUNCTION("comm3.c:   Macro");
   rc = execute_macro(params,TRUE,&macrorc);
   TRACE_RETURN();
   return((rc==RC_SYSTEM_ERROR)?rc:macrorc);
}
/*man-start*********************************************************************
COMMAND
     mark - mark a portion of text

SYNTAX
     MARK Box [line1 col1 line2 col2]
     MARK Line [line1 line2]
     MARK Stream [line1 col1 line2 col2]
     MARK Column [col1 col2]
     MARK Word [line1 col1]
     MARK CUA [LEFT|RIGHT|UP|DOWN|START|END|FOrward|BAckward|TOP|Bottom|MOUSE]

DESCRIPTION
     The MARK command marks a portion of text for later processing
     by a <COPY>, <MOVE> or <DELETE> command. This marked area is
     known as a <block>.

     When the MARK command is executed with the optional line/column
     arguments, these values are used to specify the position of the
     marked <block>.  Without the optional arguments, the position of
     the cursor is used to determine which portion of text is marked.

     'line1' and 'line2' specify the first or last line of the
     marked block.

     'col1' and 'col2' specify the first or last column of the
     marked block.

     Any existing marked block will be replaced with the block specified
     in this command.

     When marking a <word block>, 'line1' and 'col1' refer to any position
     within the word.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Adds CUA, WORD, and COLUMN options and position specifiers.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Mark(CHARTYPE *params)
#else
short Mark(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define MAR_PARAMS    5
#define CUA_NONE      0
#define CUA_LEFT      1
#define CUA_RIGHT     2
#define CUA_UP        3
#define CUA_DOWN      4
#define CUA_START     5
#define CUA_END       6
#define CUA_TOP       7
#define CUA_BOTTOM    8
#define CUA_FORWARD   9
#define CUA_BACKWARD 10
#define CUA_MOUSE    11
   LINETYPE true_line=0L;
   unsigned short y=0,x=0;
   unsigned short real_col=0;
   CHARTYPE *word[MAR_PARAMS+1];
   CHARTYPE strip[MAR_PARAMS];
   register short i=0;
   short num_params=0;
   short mark_type=0;
   short cua_type=CUA_NONE;
   short rc;
   LINETYPE tmp_line;
   LENGTHTYPE tmp_col;
   int numparms[7];
   LINETYPE nummax[7][5];
   LENGTHTYPE first_col=0,last_col=0;
   _LINE *curr=NULL;
   CHARTYPE *cont=NULL;
   LENGTHTYPE cont_len=0;
   int num[5]; /* must be at least as big as miximum number of args */

   TRACE_FUNCTION("comm3.c:   Mark");
   /*
    * Do this rather than define numparams[6] = {0,3,5,5,3,3} so that
    * non-ansi compilers won't barf.
    */
   numparms[0] = 0;
   numparms[1] = 3;
   numparms[2] = 5;
   numparms[3] = 5;
   numparms[4] = 3;
   numparms[5] = 3;
   numparms[6] = 2;
   /*
    * Marking text sets the following variables:
    * LINE:
    *         CURRENT_VIEW->marked_line:       TRUE
    *         CURRENT_VIEW->marked_start_line: line number of first line
    *         CURRENT_VIEW->marked_end_line:   line number of last line
    *         CURRENT_VIEW->marked_col:        FALSE
    *         CURRENT_VIEW->marked_start_col:  1 (ignored)
    *         CURRENT_VIEW->marked_end_col:    max width of line(ignored)
    * BOX:
    * STREAM:
    * WORD:
    *         CURRENT_VIEW->marked_line:       TRUE
    *         CURRENT_VIEW->marked_start_line: line number of first line
    *         CURRENT_VIEW->marked_end_line:   line number of last line
    *         CURRENT_VIEW->marked_col:        TRUE
    *         CURRENT_VIEW->marked_start_col:  first column
    *         CURRENT_VIEW->marked_end_col:    last column
    * CUA:
    *         CURRENT_VIEW->marked_line:       TRUE
    *         CURRENT_VIEW->marked_start_line: line number of anchor line
    *         CURRENT_VIEW->marked_end_line:   line number of last line
    *         CURRENT_VIEW->marked_col:        TRUE
    *         CURRENT_VIEW->marked_start_col:  anchor column
    *         CURRENT_VIEW->marked_end_col:    last column
    * COLUMN:
    *         CURRENT_VIEW->marked_line:       FALSE
    *         CURRENT_VIEW->marked_start_line: (ignored)
    *         CURRENT_VIEW->marked_end_line:   (ignored)
    *         CURRENT_VIEW->marked_col:        TRUE
    *         CURRENT_VIEW->marked_start_col:  first column
    *         CURRENT_VIEW->marked_end_col:    last column
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   strip[3]=STRIP_BOTH;
   num_params = param_split(params,word,MAR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   /*
    * Validate the first parameter: must be Box, Line, Stream, Column, Word
    */
   if (equal((CHARTYPE *)"box",word[0],1))
      mark_type = M_BOX;
   else if (equal((CHARTYPE *)"line",word[0],1))
      mark_type = M_LINE;
   else if (equal((CHARTYPE *)"stream",word[0],1))
      mark_type = M_STREAM;
   else if (equal((CHARTYPE *)"column",word[0],1))
      mark_type = M_COLUMN;
   else if (equal((CHARTYPE *)"word",word[0],1))
      mark_type = M_WORD;
   else if (equal((CHARTYPE *)"cua",word[0],3))
      mark_type = M_CUA;
   else
   {
      display_error(1,(CHARTYPE *)word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * For CUA, validate the optional parameter...
    */
   if ( mark_type == M_CUA
   &&  num_params == 2 )
   {
      if (equal((CHARTYPE *)"left",word[1],4))
         cua_type = CUA_LEFT;
      else if (equal((CHARTYPE *)"right",word[1],5))
         cua_type = CUA_RIGHT;
      else if (equal((CHARTYPE *)"up",word[1],2))
         cua_type = CUA_UP;
      else if (equal((CHARTYPE *)"down",word[1],4))
         cua_type = CUA_DOWN;
      else if (equal((CHARTYPE *)"top",word[1],3))
         cua_type = CUA_TOP;
      else if (equal((CHARTYPE *)"bottom",word[1],1))
         cua_type = CUA_BOTTOM;
      else if (equal((CHARTYPE *)"forward",word[1],2))
         cua_type = CUA_FORWARD;
      else if (equal((CHARTYPE *)"backward",word[1],2))
         cua_type = CUA_BACKWARD;
      else if (equal((CHARTYPE *)"start",word[1],5))
         cua_type = CUA_START;
      else if (equal((CHARTYPE *)"end",word[1],3))
         cua_type = CUA_END;
      else if (equal((CHARTYPE *)"mouse",word[1],5))
         cua_type = CUA_MOUSE;
      else
      {
         display_error(1,(CHARTYPE *)word[1],FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Reset the previous marked view if its not the current view...
       */
      if (MARK_VIEW != (VIEW_DETAILS *)NULL
      &&  MARK_VIEW != CURRENT_VIEW)
      {
         MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
         if (display_screens > 1
         &&  MARK_VIEW == OTHER_VIEW)
         {
            MARK_VIEW = (VIEW_DETAILS *)NULL;
            build_screen(other_screen);
            display_screen(other_screen);
         }
      }
      MARK_VIEW = CURRENT_VIEW;
      /*
       * If we don't have a CUA marked block already, call ourselves
       * as MARK CUA, so that the anchor point is set.  Then we can call
       * the appropriate cursor movement function, and then mark the
       * block at the newly positioned place.
       */
      if ( CURRENT_VIEW->mark_type != M_CUA )
      {
         MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
         Mark( (CHARTYPE *)"CUA" );
      }
      /*
       * We can now move the cursor (if required) and then mark the end
       * point of the block
       */
      switch( cua_type )
      {
         case CUA_LEFT:
            rc = THEcursor_left( CURSOR_CUA, FALSE );
            break;
         case CUA_RIGHT:
            rc = THEcursor_right( CURSOR_CUA, FALSE );
            break;
         case CUA_UP:
            rc = THEcursor_up( CURSOR_CUA );
            break;
         case CUA_DOWN:
            rc = THEcursor_down( CURSOR_CUA );
            break;
         case CUA_FORWARD:
            rc = scroll_page(DIRECTION_FORWARD,1,FALSE);
            break;
         case CUA_BACKWARD:
            rc = scroll_page(DIRECTION_BACKWARD,1,FALSE);
            break;
         case CUA_TOP:
            rc = Top((CHARTYPE *)"");
            break;
         case CUA_BOTTOM:
            rc = Bottom((CHARTYPE *)"");
            break;
         case CUA_END:
            rc = Sos_endchar((CHARTYPE *)"");
            break;
         case CUA_START:
            rc = Sos_firstchar((CHARTYPE *)"");
            break;
         case CUA_MOUSE:
            rc = THEcursor_mouse();
            break;
         default:
            break;
      }
      true_line = get_true_line(FALSE);
      /*
       * If we are on 'Top of File' or 'Bottom of File' lines, error.
       */
      if (TOF(true_line) || BOF(true_line))
      {
         display_error(38,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
      }
      /*
       * If we are in the file area or prefix area and the focus line is not
       * a real line, error.
       */
      getyx(CURRENT_WINDOW,y,x);
      if (CURRENT_VIEW->current_window == WINDOW_FILEAREA
      ||  CURRENT_VIEW->current_window == WINDOW_PREFIX)
      {
         if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
      }
      real_col = x + CURRENT_VIEW->verify_col;

      CURRENT_VIEW->mark_type = mark_type;
      CURRENT_VIEW->mark_end_line = true_line;
      CURRENT_VIEW->mark_end_col = real_col;
      build_screen(current_screen);
      display_screen(current_screen);
      wmove(CURRENT_WINDOW,y,x);
   }
   /*
    * With one parameter determine position of block...
    */
   else if ( num_params == 1 )
   {
      true_line = get_true_line(FALSE);
      /*
       * If we are on 'Top of File' or 'Bottom of File' lines, error.
       */
      if (TOF(true_line) || BOF(true_line))
      {
         display_error(38,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
      }
      /*
       * If we are in the file area or prefix area and the focus line is not
       * a real line, error.
       */
      getyx(CURRENT_WINDOW,y,x);
      if (CURRENT_VIEW->current_window == WINDOW_FILEAREA
      ||  CURRENT_VIEW->current_window == WINDOW_PREFIX)
      {
         if (CURRENT_SCREEN.sl[y].line_type != LINE_LINE)
         {
            display_error(38,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
      }
      /*
       * Reset the previous marked view if its not the current view...
       */
      if (MARK_VIEW != (VIEW_DETAILS *)NULL
      &&  MARK_VIEW != CURRENT_VIEW)
      {
         MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
         if (display_screens > 1
         &&  MARK_VIEW == OTHER_VIEW)
         {
            MARK_VIEW = (VIEW_DETAILS *)NULL;
            build_screen(other_screen);
            display_screen(other_screen);
         }
      }
      MARK_VIEW = CURRENT_VIEW;
      CURRENT_VIEW->mark_type = mark_type;
      /*
       * Set the new values for top and bottom lines marked.
       * For all marked blocks, other than CUA, the block will
       * extend either side of the first position.
       * For CUA blocks, the first position is an anchor position
       * and will not change.
       */
      if (CURRENT_VIEW->marked_line)
      {
         if ( mark_type == M_CUA )
         {
            CURRENT_VIEW->mark_end_line = true_line;
         }
         else
         {
            if (true_line > CURRENT_VIEW->mark_end_line)
               CURRENT_VIEW->mark_end_line = true_line;
            if (true_line < CURRENT_VIEW->mark_start_line)
               CURRENT_VIEW->mark_start_line = true_line;
            if (true_line < CURRENT_VIEW->mark_end_line
            &&  true_line > CURRENT_VIEW->mark_start_line)
            {
               if (true_line-CURRENT_VIEW->mark_end_line >
                   CURRENT_VIEW->mark_start_line-true_line)
                  CURRENT_VIEW->mark_end_line = true_line;
               else
                  CURRENT_VIEW->mark_start_line = true_line;
            }
         }
      }
      else
      {
         CURRENT_VIEW->mark_start_line = CURRENT_VIEW->mark_end_line = true_line;
      }
      /*
       * Set the new values for first and last columns marked.
       */
      real_col = x + CURRENT_VIEW->verify_col;
      if (CURRENT_VIEW->marked_col)
      {
         if ( mark_type == M_CUA )
         {
            CURRENT_VIEW->mark_end_col = real_col;
         }
         else
         {
            if (mark_type == M_STREAM
            &&  CURRENT_VIEW->mark_start_line != CURRENT_VIEW->mark_end_line)
            {
               if (CURRENT_VIEW->mark_end_line == true_line)
                  CURRENT_VIEW->mark_end_col = (real_col);
               if (CURRENT_VIEW->mark_start_line == true_line)
                  CURRENT_VIEW->mark_start_col = (real_col);
            }
            else
            {
               if ((real_col) > CURRENT_VIEW->mark_end_col)
                  CURRENT_VIEW->mark_end_col = (real_col);
               if ((real_col) < CURRENT_VIEW->mark_start_col)
                  CURRENT_VIEW->mark_start_col = (real_col);
               if ((real_col) < CURRENT_VIEW->mark_end_col
               &&  (real_col) > CURRENT_VIEW->mark_start_col)
               {
                  if ((real_col)-CURRENT_VIEW->mark_end_col > CURRENT_VIEW->mark_start_col-(real_col))
                     CURRENT_VIEW->mark_end_col = (real_col);
                  else
                     CURRENT_VIEW->mark_start_col = (real_col);
               }
            }
         }
      }
      else
      {
         CURRENT_VIEW->mark_start_col = CURRENT_VIEW->mark_end_col = real_col;
      }

      /*
       * Set flags for various marked text types...
       */
      switch(mark_type)
      {
         case M_LINE:
            CURRENT_VIEW->marked_col = FALSE;
            CURRENT_VIEW->marked_line = TRUE;
            CURRENT_VIEW->mark_start_col = 1;
            CURRENT_VIEW->mark_end_col = max_line_length;
            break;
         case M_BOX:
            CURRENT_VIEW->marked_col = TRUE;
            CURRENT_VIEW->marked_line = TRUE;
            break;
         case M_WORD:
            if (get_word(rec,rec_len,real_col-1,&first_col,&last_col) == 0)
              {
               CURRENT_VIEW->marked_line = CURRENT_VIEW->marked_col = FALSE;
               MARK_VIEW = (VIEW_DETAILS *)NULL;
               break;
              }
            CURRENT_VIEW->marked_col = TRUE;
            CURRENT_VIEW->marked_line = TRUE;
            CURRENT_VIEW->mark_start_line = CURRENT_VIEW->mark_end_line = true_line;
            CURRENT_VIEW->mark_start_col = first_col+1;
            CURRENT_VIEW->mark_end_col = last_col+1;
            break;
         case M_COLUMN:
            CURRENT_VIEW->marked_line = FALSE;
            CURRENT_VIEW->marked_col = TRUE;
            CURRENT_VIEW->mark_start_line = 1L;
            CURRENT_VIEW->mark_end_line = MAX_LONG;
            break;
         case M_STREAM:
         case M_CUA:
            CURRENT_VIEW->marked_col = TRUE;
            CURRENT_VIEW->marked_line = TRUE;
            break;
      }
      build_screen(current_screen);
      display_screen(current_screen);
      wmove(CURRENT_WINDOW,y,x);
   }
   else
   {
      nummax[M_LINE][1] = nummax[M_LINE][2] = nummax[M_BOX][1] =
         nummax[M_BOX][3] = nummax[M_STREAM][1] = nummax[M_STREAM][3] =
         nummax[M_WORD][1] = CURRENT_FILE->number_lines;
      nummax[M_BOX][2] = nummax[M_BOX][4] = nummax[M_COLUMN][1] =
         nummax[M_COLUMN][2] = nummax[M_STREAM][2] = nummax[M_STREAM][4] =
         nummax[M_WORD][2] = max_line_length;
      /*
       * Validate the number of parameters...
       */
      if (num_params < numparms[mark_type])
      {
         display_error(3,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      if (num_params > numparms[mark_type])
      {
         display_error(2,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Validate the arguments...
       */
      for (i=1;i<numparms[mark_type];i++)
      {
         if (valid_positive_integer(word[i]))
         {
            num[i] = atol((DEFCHAR *)word[i]);
            if (num[i] > nummax[mark_type][i])
            {
               display_error(6,word[i],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
            if (num[i] == 0)
            {
               display_error(5,word[i],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
         else
         {
            display_error(1,word[i],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
      }
      /*
       * Reset the previous marked view...
       */
      if (MARK_VIEW != (VIEW_DETAILS *)NULL
      &&  MARK_VIEW != CURRENT_VIEW)
      {
         MARK_VIEW->marked_line = MARK_VIEW->marked_col = FALSE;
         if (display_screens > 1
         &&  MARK_VIEW == OTHER_VIEW)
         {
            MARK_VIEW = (VIEW_DETAILS *)NULL;
            build_screen(other_screen);
            display_screen(other_screen);
         }
      }
      /*
       * Set the new values of the marked block.
       */
      MARK_VIEW = CURRENT_VIEW;
      CURRENT_VIEW->mark_type = mark_type;
      switch(mark_type)
      {
         case M_BOX:
         case M_STREAM:
            CURRENT_VIEW->mark_start_line = num[1];
            CURRENT_VIEW->mark_end_line = num[3];
            CURRENT_VIEW->mark_start_col = (LENGTHTYPE)num[2];
            CURRENT_VIEW->mark_end_col = (LENGTHTYPE)num[4];
            CURRENT_VIEW->marked_line = TRUE;
            CURRENT_VIEW->marked_col = TRUE;
            if (CURRENT_VIEW->mark_start_line > CURRENT_VIEW->mark_end_line)
            {
               tmp_line = CURRENT_VIEW->mark_end_line;
               CURRENT_VIEW->mark_end_line = CURRENT_VIEW->mark_start_line;
               CURRENT_VIEW->mark_start_line = tmp_line;
               tmp_col = CURRENT_VIEW->mark_end_col;
               CURRENT_VIEW->mark_end_col = CURRENT_VIEW->mark_start_col;
               CURRENT_VIEW->mark_start_col = tmp_col;
            }
            else if (CURRENT_VIEW->mark_start_line == CURRENT_VIEW->mark_end_line
                 &&  CURRENT_VIEW->mark_start_col > CURRENT_VIEW->mark_end_col)
            {
               tmp_col = CURRENT_VIEW->mark_end_col;
               CURRENT_VIEW->mark_end_col = CURRENT_VIEW->mark_start_col;
               CURRENT_VIEW->mark_start_col = tmp_col;
            }
            break;
         case M_LINE:
            CURRENT_VIEW->mark_start_line = num[1];
            CURRENT_VIEW->mark_end_line = num[2];
            CURRENT_VIEW->marked_col = FALSE;
            CURRENT_VIEW->mark_start_col = 1;
            CURRENT_VIEW->mark_end_col = max_line_length;
            if (CURRENT_VIEW->mark_start_line > CURRENT_VIEW->mark_end_line)
            {
               tmp_line = CURRENT_VIEW->mark_end_line;
               CURRENT_VIEW->mark_end_line = CURRENT_VIEW->mark_start_line;
               CURRENT_VIEW->mark_start_line = tmp_line;
            }
            break;
         case M_COLUMN:
            CURRENT_VIEW->mark_start_col = (LENGTHTYPE)num[1];
            CURRENT_VIEW->mark_end_col = (LENGTHTYPE)num[2];
            CURRENT_VIEW->marked_line = FALSE;
            CURRENT_VIEW->marked_col = TRUE;
            CURRENT_VIEW->mark_start_line = 1;
            CURRENT_VIEW->mark_end_line = MAX_LONG;
            if (CURRENT_VIEW->mark_start_col > CURRENT_VIEW->mark_end_col)
            {
               tmp_col = CURRENT_VIEW->mark_end_col;
               CURRENT_VIEW->mark_end_col = CURRENT_VIEW->mark_start_col;
               CURRENT_VIEW->mark_start_col = tmp_col;
            }
            break;
         case M_WORD:
            if (CURRENT_VIEW->current_window == WINDOW_FILEAREA
            &&  CURRENT_VIEW->focus_line == num[1])
            {
               cont = rec;
               cont_len = rec_len;
            }
            else
            {
               curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,num[1],CURRENT_FILE->number_lines);
               cont = curr->line;
               cont_len = curr->length;
            }
            if (get_word(cont,cont_len,num[2],&first_col,&last_col) == 0)
            {
               CURRENT_VIEW->marked_line = CURRENT_VIEW->marked_col = FALSE;
               MARK_VIEW = (VIEW_DETAILS *)NULL;
               break;
            }
            CURRENT_VIEW->marked_col = TRUE;
            CURRENT_VIEW->marked_line = TRUE;
            CURRENT_VIEW->mark_start_line = CURRENT_VIEW->mark_end_line = num[1];
            CURRENT_VIEW->mark_start_col = first_col+1;
            CURRENT_VIEW->mark_end_col = last_col+1;
            break;
         default:
            break;
      }
      build_screen(current_screen);
      display_screen(current_screen);
   }
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     modify - display current SET command for alteration

SYNTAX
     MODify set-command

DESCRIPTION
     The MODIFY command displays the current setting of a <SET> command
     on the command line enabling the user to change that setting.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <SET>, <QUERY>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Modify(CHARTYPE *params)
#else
short Modify(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Modify");
   if ((rc = execute_modify_command(params)) == RC_OK)
      Cmsg(temp_cmd);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     move - move a portion of text

SYNTAX
     MOVE target1 target2
     MOVE BLOCK [RESET]

DESCRIPTION
     The MOVE command copies the contents of a portion of the file to
     the same or a different file, and deletes the marked portion from
     the original file.

     The first form of the MOVE command, moves the portion of the file
     specified by 'target1' to the line spcified by 'target2' in the
     same file.

     The second form of the MOVE command moves the contents of the marked
     <block> to the current cursor position. If the optional ['RESET']
     argument is supplied, the marked block is reset as though a
     <RESET> BLOCK command had been issued.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Adds extra functionality with [RESET] option.

STATUS
     Incomplete. First form is not supported.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEMove(CHARTYPE *params)
#else
short THEMove(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define MOV_PARAMS 2
   CHARTYPE *word[MOV_PARAMS+1];
   CHARTYPE strip[MOV_PARAMS];
   unsigned short num_params=0;
   unsigned short y=0,x=0;
   LINETYPE true_line=0L;
   CHARTYPE reset_block=SOURCE_UNKNOWN;
   CHARTYPE copy_command=0,delete_command=0;
   short rc=RC_OK;
   LINETYPE start_line=0L,end_line=0L,num_lines=0L,dest_line=0L,lines_affected=0L;
   VIEW_DETAILS *old_mark_view=NULL;

   TRACE_FUNCTION("comm3.c:   THEMove");
   /*
    * This command invalid if source file is readonly...
    */
   if (!MARK_VIEW)
   {
      display_error(44,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   if (ISREADONLY(MARK_FILE))
   {
      display_error(56,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,MOV_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (num_params > 2)
   {
      display_error(2,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Test for valid parameters...
    */
   if (num_params == 1
   &&  equal((CHARTYPE *)"block",word[0],5))
      reset_block = SOURCE_BLOCK;
   if (num_params == 2
   &&  equal((CHARTYPE *)"block",word[0],5)
   &&  equal((CHARTYPE *)"reset",word[1],5))
      reset_block = SOURCE_BLOCK_RESET;
   if (reset_block == SOURCE_UNKNOWN)
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Validate marked block, can be in any view.
    */
   if (marked_block(FALSE) != RC_OK)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * If the cursor is in the marked block...error.
    */
   if (MARK_VIEW == CURRENT_VIEW)
   {
      getyx(CURRENT_WINDOW_FILEAREA,y,x);
      switch(MARK_VIEW->mark_type)
      {
         case M_LINE:
         case M_WORD:
         case M_COLUMN:
         case M_STREAM:
         case M_BOX:
            if ((CURRENT_VIEW->focus_line >= MARK_VIEW->mark_start_line)
            &&  (CURRENT_VIEW->focus_line <= MARK_VIEW->mark_end_line)
            &&  (x + CURRENT_VIEW->verify_col >= MARK_VIEW->mark_start_col)
            &&  (x + CURRENT_VIEW->verify_col <= MARK_VIEW->mark_end_col))
            {
               display_error(50,(CHARTYPE *)"",FALSE);
               TRACE_RETURN();
               return(RC_INVALID_ENVIRON);
            }
            break;
         default:
            break;
      }
   }
   /*
    * If block is a box, call its function.
    */
   if (MARK_VIEW->mark_type == M_BOX
   ||  MARK_VIEW->mark_type == M_WORD
   ||  MARK_VIEW->mark_type == M_STREAM
   ||  MARK_VIEW->mark_type == M_COLUMN)
   {
      box_operations(BOX_M,reset_block,FALSE,' ');/* don't reset and don't overlay */
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Determine the target line. If on the command line, target is current
    * line, else target line is focus line.
    */
   true_line = get_true_line(FALSE);
   /*
    * If the true  line is the bottom of file line, subtract 1 from it.
    */
   if (BOF(true_line))
      true_line--;

   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   start_line = MARK_VIEW->mark_start_line;
   end_line = MARK_VIEW->mark_end_line;
   num_lines = end_line - start_line + 1L;
   dest_line = true_line;
   old_mark_view = MARK_VIEW;
   if (MARK_VIEW == CURRENT_VIEW)
   {
      copy_command = COMMAND_MOVE_COPY_SAME;
      delete_command = COMMAND_MOVE_DELETE_SAME;
   }
   else
   {
      copy_command = COMMAND_MOVE_COPY_DIFF;
      delete_command = COMMAND_MOVE_DELETE_DIFF;
   }

   rc = rearrange_line_blocks(copy_command,reset_block,start_line,
                              end_line,dest_line,1,MARK_VIEW,CURRENT_VIEW,FALSE,
                              &lines_affected);
   if (rc == RC_OK)
   {
      if (old_mark_view == CURRENT_VIEW)
      {
         if (dest_line < start_line)
         {
            start_line += num_lines;
            end_line += num_lines;
            dest_line += num_lines;
         }
      }
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
      rc = rearrange_line_blocks(delete_command,reset_block,start_line,
                              end_line,start_line,1,old_mark_view,old_mark_view,FALSE,
                              &lines_affected);
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     msg - display message on error line

SYNTAX
     MSG [message]

DESCRIPTION
     The MSG command displays a 'message' on the <message line>.
     This command is usually issued from a macro file.
     This is similar to <EMSG>, but MSG does not sound the bell if
     <SET BEEP> is on.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <CMSG>, <EMSG>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Msg(CHARTYPE *params)
#else
short Msg(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("comm3.c:   Msg");
   display_error(0,params,TRUE);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     next - move forward in the file a number of lines

SYNTAX
     Next [relative target]

DESCRIPTION
     The NEXT command moves the <current line> forwards the number of
     lines specified by the <relative target>. This <relative target> can
     only be a positive integer or the character "*".

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1

SEE ALSO
     <DOWN>, <UP>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THENext(CHARTYPE *params)
#else
short THENext(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;
   LINETYPE num_lines=0L,true_line=0L;

   TRACE_FUNCTION("comm3.c:   THENext");
   params = MyStrip(params,STRIP_BOTH,' ');
   if (strcmp("",(DEFCHAR *)params) == 0)
      params = (CHARTYPE *)"1";
   true_line = get_true_line(TRUE);
   if (strcmp("*",(DEFCHAR *)params) == 0)
      num_lines = CURRENT_FILE->number_lines - true_line + 1L;
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
   rc = advance_current_or_focus_line(num_lines);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     nextwindow - switch focus of editing session to another file

SYNTAX
     NEXTWindow

DESCRIPTION
     The NEXTWINDOW command moves the focus of the editing session to
     the other screen (if <SET SCREEN> 2 is in effect) or to the next
     file in the <ring>.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <PREVWINDOW>, <EDIT>, <SET SCREEN>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Nextwindow(CHARTYPE *params)
#else
short Nextwindow(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Nextwindow");
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (display_screens == 1)
   {
      rc = Xedit((CHARTYPE *)"");
      TRACE_RETURN();
      return(rc);
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   current_screen = (current_screen == 0) ? 1 : 0;
   CURRENT_VIEW = CURRENT_SCREEN.screen_view;
   if (curses_started)
   {
      if (CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
      {
         wattrset(CURRENT_WINDOW_COMMAND,set_colour(CURRENT_FILE->attr+ATTR_CMDLINE));
         touchwin(CURRENT_WINDOW_COMMAND);
         wnoutrefresh(CURRENT_WINDOW_COMMAND);
      }
      if (CURRENT_WINDOW_ARROW != (WINDOW *)NULL)
      {
         wattrset(CURRENT_WINDOW_ARROW,set_colour(CURRENT_FILE->attr+ATTR_ARROW));
         redraw_window(CURRENT_WINDOW_ARROW);
         wnoutrefresh(CURRENT_WINDOW_ARROW);
      }
      if (statarea != (WINDOW *)NULL)
      {
         wattrset(statarea,set_colour(CURRENT_FILE->attr+ATTR_STATAREA));
         redraw_window(statarea);
      }
      if (CURRENT_WINDOW_IDLINE != (WINDOW *)NULL)
      {
         wattrset(CURRENT_WINDOW_IDLINE,set_colour(CURRENT_FILE->attr+ATTR_IDLINE));
         redraw_window(CURRENT_WINDOW_IDLINE);
      }
      if (display_screens > 1
      &&  !horizontal)
      {
         wattrset(divider,set_colour(CURRENT_FILE->attr+ATTR_DIVIDER));
         draw_divider();
         wnoutrefresh(divider);
      }
   }
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   build_screen(current_screen);
   if (!line_in_view(current_screen,CURRENT_VIEW->focus_line))
   {
      CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line;
      build_screen(current_screen);
   }
   display_screen(current_screen);

   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     nfind - locate forwards the line which does NOT begin with the supplied string

SYNTAX
     NFind [string]

DESCRIPTION
     The NFIND command attempts to locate a line towards the end of
     the file that does NOT begin with 'string'.
     If the optional 'string' is not supplied the last 'string' used
     in any of the family of find commands is used.

     'string' can contain two special characters:

          space - this will match any single character in the target line
          underscore - this will match any single space in the target line

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FIND>, <FINDUP>, <NFINDUP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Nfind(CHARTYPE *params)
#else
short Nfind(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Nfind");
   rc = execute_find_command(params,TARGET_NFIND);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     nfindup - locate backwards the line which does NOT begin with the supplied string

SYNTAX
     NFINDUp [string]

DESCRIPTION
     The NFINDUP command attempts to locate a line towards the start of
     the file that does NOT begin with 'string'.
     If the optional 'string' is not supplied the last 'string' used
     in any of the family of find commands is used.

     'string' can contain two special characters:

          space - this will match any single character in the target line
          underscore - this will match any single space in the target line

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FIND>, <FINDUP>, <NFIND>, <NFUP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Nfindup(CHARTYPE *params)
#else
short Nfindup(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Nfindup");
   rc = execute_find_command(params,TARGET_NFINDUP);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     nfup - locate backwards the line which does NOT begin with the supplied string

SYNTAX
     NFUp [string]

DESCRIPTION
     The NFUP command is a synonym for the <NFINDUP> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FIND>, <FINDUP>, <NFIND>, <NFINDUP>

STATUS
     Complete
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     nomsg - execute a command suppressing any messages

SYNTAX
     NOMSG command [arguments]

DESCRIPTION
     The NOMSG command executes the supplied 'command' but suppresses
     messages that would normally be displayed as a result of the
     command.

     Optional 'arguments' may be passed to the 'command'.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Nomsg(CHARTYPE *params)
#else
short Nomsg(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Nomsg");
   in_nomsg = TRUE;
   rc = command_line(params,COMMAND_ONLY_FALSE);
   in_nomsg = FALSE;
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     nop - no operation command

SYNTAX
     NOP

DESCRIPTION
     The NOP command does nothing. It is used as a means of turning
     off an assignment to a key.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <DEFINE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Nop(CHARTYPE *params)
#else
short Nop(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("comm3.c:   Nop");
   /*
    * No arguments are allowed; error if any are present.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,(CHARTYPE *)params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     os - execute an operating system command

SYNTAX
     OS [command]

DESCRIPTION
     The OS command executes the supplied operating system 'command'
     or runs an interactive shell if no 'command' is supplied.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Equivalent to DOS command.

SEE ALSO
     <DOS>, <!>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Os(CHARTYPE *params)
#else
short Os(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Os");
   /*
    * Execute the supplied parameters as OS commands. Run with output
    * displayed and pause before redrawing the windows.
    */
   rc = execute_os_command(params,FALSE,TRUE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     osnowait - execute an operating system command - no prompt

SYNTAX
     OSNowait command

DESCRIPTION
     The OSNOWAIT command executes the supplied operating system
     'command' not waiting for the user to be prompted once the
     command has completed.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Equivalent of <DOSNOWAIT> command.

SEE ALSO
     <DOSNOWAIT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Osnowait(CHARTYPE *params)
#else
short Osnowait(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Osnowait");
   /*
    * Execute the supplied parameters as OS commands. Run with output
    * displayed but no pause before redrawing the windows.
    */
   if (strcmp((DEFCHAR *)params,"") == 0)          /* no params....error */
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   rc = execute_os_command(params,FALSE,FALSE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     osquiet - execute an operating system command quietly

SYNTAX
     OSQuiet command

DESCRIPTION
     The OSQUIET command executes the supplied operating system 'command'
     as quietly as possible.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Equivalent of <DOSQUIET> command.

SEE ALSO
     <DOSQUIET>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Osquiet(CHARTYPE *params)
#else
short Osquiet(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm3.c:   Osquiet");
   /*
    * Execute the supplied parameters as OS commands. Run with no output
    * displayed and no pause before redrawing the windows.
    */
   if (strcmp((DEFCHAR *)params,"") == 0)          /* no params....error */
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   rc = execute_os_command(params,TRUE,FALSE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     osredir - execute an operating system command and capture output

SYNTAX
     OSRedir filename command [arguments ...]

DESCRIPTION
     The OSREDIR command executes the supplied operating system 'command'
     and redirects output destined for STDOUT and STDERR to the specified
     'filename'.

     Optional 'arguments' may be supplied to the 'command'.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <OS>, <OSQUIET>, <OSNOWAIT>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Osredir(CHARTYPE *params)
#else
short Osredir(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK,rrc=0;
#define OSR_PARAMS 2
   CHARTYPE err[1000];
   CHARTYPE strip[OSR_PARAMS];
   CHARTYPE *word[OSR_PARAMS+1];
   unsigned short num_params=0;
   int save_stdout=0,save_stderr=0;
#ifdef UNIX
   int save_stdin=0;
#endif
   int fd=0;

   TRACE_FUNCTION( "comm3.c:   Osredir" );
   /*
    * Execute the supplied parameters as OS commands. Run with no output
    * displayed and no pause before redrawing the windows.
    */
   err[0] = '\0';
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split( params, word, OSR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );
   if ( num_params == 0 )
   {
      display_error( 3, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if ( num_params > 2 )
   {
      display_error( 2, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Delete the file to be on the safe side
    */
   remove_file( word[0] );
   /*
    * Save the "real" stdout and stderr handles so we can restore them later
    */
   save_stdout = dup( fileno( stdout ) );
   save_stderr = dup( fileno( stderr ) );
#ifdef UNIX1
   save_stdin  = dup( fileno( stdin ) );
#endif

#if defined(S_IWRITE) && defined(S_IREAD)
   fd = open( (DEFCHAR *)word[0], O_WRONLY|O_CREAT, S_IWRITE|S_IREAD );
#else
   fd = open( (DEFCHAR *)word[0], O_WRONLY|O_CREAT );
#endif
   if ( fd == (-1) )
   {
      display_error( 8, word[0], FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
#ifdef UNIX
   chmod( (DEFCHAR *)word[0], S_IRUSR|S_IWUSR );
#endif
   /*
    * Redirect stdout and stderr to the supplied file handle
    */
   if ( dup2( fd, fileno( stdout ) ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error dup2() on stdout: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
   if ( dup2( fd, fileno( stderr ) ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error dup2() on stderr: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
#ifdef UNIX1
   if ( rc == RC_OK )
   &&   dup2( fd, fileno( stdin ) ) == (-1) )
   {
      sprintf( err, "Error dup2() on stdin: %s", strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
#endif
   /*
    * Run the supplied OS command with stdout and stderr going to the
    * supplied redirection file
    */
   if ( rc == RC_OK )
      rrc = system( (DEFCHAR *)word[1] );
   /*
    * Close the redirected file
    */
   if ( close( fd ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error closing fd: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
   /*
    * Reset the stdout and stderr handles to the "real" handles.
    */
   if ( dup2( save_stdout, fileno( stdout ) ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error resetting stdout: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
   if ( dup2( save_stderr, fileno( stderr ) ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error resetting stderr: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
#ifdef UNIX1
   dup2( save_stdin, fileno( stdin ) );
   close( save_stdin );
#endif

#if 1
   if ( close( save_stdout ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error closing save_stdout: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
   if ( close( save_stderr ) == (-1) )
   {
      strcat( (DEFCHAR *)err, "Error closing save_stderr: " );
      strcat( (DEFCHAR *)err, strerror( errno ) );
      rc = RC_INVALID_OPERAND;
   }
#endif

   TRACE_RETURN();
   if ( rc == RC_OK )
      return(rrc);
   else
   {
      display_error( 8, err, FALSE );
      return(rc);
   }
}
/*man-start*********************************************************************
COMMAND
     overlaybox - overlay marked block on current cursor position

SYNTAX
     OVERLAYBox

DESCRIPTION
     The OVERLAYBOX command overlays the contents of the marked <block>;
     <box block> or <line block>, over the characters or lines at the
     <focus line> and <focus column>.

     This command implies that only <box block>s are handled.  This used
     to be the case, and for compatibility reasons the name remains.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <MOVE>, <COPY>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Overlaybox(CHARTYPE *params)
#else
short Overlaybox(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   unsigned short y=0,x=0;
   LINETYPE true_line=0L,start_line=0L,end_line=0L,num_lines=0L,dest_line=0L,lines_affected=0L;
   VIEW_DETAILS *old_mark_view=NULL;
   short rc=RC_OK;
   _LINE *curr=NULL;
   LINETYPE save_current_line=CURRENT_VIEW->current_line;

   TRACE_FUNCTION("comm3.c:   Overlaybox");
   /*
    * Ensure there are no parameters.
    */
   if (strcmp((DEFCHAR *)params,"") != 0)
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
   /*
    * Validate marked block, can be in any view.
    */
   if (marked_block(FALSE) != RC_OK)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * Don't allow for multi-line STREAM blocks.
    */
   if (MARK_VIEW->mark_type == M_STREAM
   &&  MARK_VIEW->mark_start_line != MARK_VIEW->mark_end_line)
   {
      display_error(62,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * If the command is executed from the filearea, the focus line must be
    * in scope...
    */
   if (curses_started)
   {
      if (CURRENT_VIEW->current_window != WINDOW_COMMAND)
      {
         getyx(CURRENT_WINDOW,y,x);
         if (!IN_SCOPE(CURRENT_VIEW,CURRENT_SCREEN.sl[y].current)
         && !CURRENT_VIEW->scope_all)
         {
            display_error(87,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
      }
   }
   /*
    * If block is a box, call its function.
    */
   if (MARK_VIEW->mark_type != M_LINE)
   {
      box_operations(BOX_C,SOURCE_BLOCK,TRUE,' ');  /* no reset, overlay */
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Determine the target line. If on the command line, target is current
    * line, else target line is focus line.
    */
   true_line = get_true_line(TRUE);

   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   start_line = MARK_VIEW->mark_start_line;
   end_line = MARK_VIEW->mark_end_line;
   num_lines = end_line - start_line + 1L;
   dest_line = true_line-1L;
   old_mark_view = MARK_VIEW;

   rc = rearrange_line_blocks(COMMAND_OVERLAY_COPY,SOURCE_BLOCK,start_line,
                            end_line,dest_line,1,MARK_VIEW,CURRENT_VIEW,FALSE,
                            &lines_affected);
   if (rc == RC_OK
   &&  lines_affected != 0)
   {
      start_line = end_line = true_line + lines_affected;
      curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,start_line,CURRENT_FILE->number_lines);
      for ( ; ; )
      {
         if (CURRENT_VIEW->scope_all
         ||  IN_SCOPE(CURRENT_VIEW,curr))
            lines_affected--;
         curr = curr->next;
         if (curr == NULL
         ||  lines_affected == 0L
         ||  end_line == CURRENT_FILE->number_lines)
            break;
         end_line++;
      }
      dest_line = true_line;
      post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
      rc = rearrange_line_blocks(COMMAND_OVERLAY_DELETE,SOURCE_BLOCK,start_line,
                            end_line,dest_line,1,CURRENT_VIEW,CURRENT_VIEW,FALSE,
                            &lines_affected);
      if (old_mark_view != MARK_VIEW)
         old_mark_view->marked_line = old_mark_view->marked_col = FALSE;
   }
   CURRENT_VIEW->current_line = save_current_line;
   build_screen(current_screen);
   display_screen(current_screen);
   if (curses_started
   && CURRENT_VIEW->current_window != WINDOW_COMMAND)
      wmove(CURRENT_WINDOW,y,x);

   TRACE_RETURN();
   return(rc);
}
