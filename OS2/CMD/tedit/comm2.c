/***********************************************************************/
/* COMM2.C - Commands D-J                                              */
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

static char RCSid[] = "$Id: comm2.c,v 1.14 2002/08/13 09:11:47 mark Exp $";

#include <the.h>
#include <proto.h>

/*#define DEBUG 1*/

/*man-start*********************************************************************
COMMAND
     define - assign one or many commands to a key or mouse event

SYNTAX
     DEFine key-name [REXX] [command [args] [[#command [args]...]]]
     DEFine mouse-key-definition IN window [REXX] [command [args] [[#command [args]...]]]

DESCRIPTION
     The DEFINE command allows the user to assign one or many
     commands and optional parameter(s) to a key or a mouse button
     specification.

     Commands may be abbreviated.

     If multiple commands are assigned, then the LINEND setting
     must be ON and the LINEND character must match the character
     that delimits the commands at the time that the DEFINE command
     is executed. LINEND can be OFF at the time the key is pressed.

     With no arguments, any existing definition for that key is
     removed and the key reverts back to its default assignation (if
     it had any).

     'key-name' corresponds to the key name shown with the <SHOWKEY> command.

     If the optional keyword; 'REXX', is supplied, the remainder of the
     command line is treated as a Rexx macro and is passed onto the
     Rexx interpreter (if you have one) for execution.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.
            KEDIT does not allow multiple commands except as KEXX
            macros.

SEE ALSO
     <SHOWKEY>, <SET LINEND>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Define(CHARTYPE *params)
#else
short Define(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define DEF_PARAMS  2
#define DEF_MOUSE_PARAMS  4
   CHARTYPE *word[DEF_MOUSE_PARAMS+1];
   CHARTYPE strip[DEF_MOUSE_PARAMS];
   CHARTYPE *ptr=NULL;
   unsigned short num_params=0;
   int key_value=0;
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Define");
   strip[0] = STRIP_BOTH;
   strip[1] = STRIP_LEADING;
   num_params = param_split(params,word,DEF_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * The first parameter is the key name mnemonic , the next is one or
    * more commands and/or parameters.
    * First check the mnemonic for decimal string value. ie begins with \
    */
   if (word[0][0] == '\\')
   {
       if ((key_value = atoi((DEFCHAR *)word[0]+1)) == 0)
          rc = RC_INVALID_OPERAND;
   }
   else
   {
       if ((key_value = find_key_value(word[0])) == (-1))
          rc = RC_INVALID_OPERAND;
   }
   if (rc == RC_OK)
   {
      /*
       * Determine if the first word of the supplied command is REXX (either
       * case)...
       */
      if ( strlen( (DEFCHAR *)word[1] ) > 5
      &&   memcmpi( word[1], (CHARTYPE *)"REXX ", 5 ) == 0 )
      {
         ptr = word[1];
         rc = add_define(&first_define,&last_define,key_value,ptr+5,TRUE,FALSE,0);
      }
      else
         rc = add_define(&first_define,&last_define,key_value,word[1],FALSE,FALSE,0);
      /*
       * Return after processing a KEY definition...
       */
      TRACE_RETURN();
      return(rc);
   }
   /*
    * To get here, either it is an invalid KEY definition, or it is a
    * MOUSE key definition waiting to be validated.
    */
   strip[0] = STRIP_BOTH;
   strip[1] = STRIP_BOTH;
   strip[2] = STRIP_BOTH;
   strip[3] = STRIP_NONE;
   num_params = param_split(params,word,DEF_MOUSE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 3)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (!equal((CHARTYPE *)"in",word[1],2))
   {
      display_error(1,word[1],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if ((key_value = find_mouse_key_value(word[0],word[2])) == (-1))
   {
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Determine if the first word of the supplied command is REXX (either
    * case)...
    */
   if ( strlen( (DEFCHAR *)word[1] ) > 5
   &&   memcmpi( word[3], (CHARTYPE *)"REXX ", 5) == 0 )
   {
      ptr = word[3];
      rc = add_define(&first_mouse_define,&last_mouse_define,key_value,ptr+5,TRUE,FALSE,0);
   }
   else
      rc = add_define(&first_mouse_define,&last_mouse_define,key_value,word[3],FALSE,FALSE,0);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     delete - delete lines from a file

SYNTAX
     DELete [target]

DESCRIPTION
     The DELETE command removes lines from the current file.
     The number of lines removed depends on the <'target'> specified.
     Lines are removed starting with the <focus line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1

SEE ALSO
     <SOS DELLINE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short DeleteLine(CHARTYPE *params)
#else
short DeleteLine(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   LINETYPE start_line=0L,end_line=0L,dest_line=0L,lines_affected=0L;
   short rc=RC_OK;
   CHARTYPE *args=NULL;
   TARGET target;
   short target_type=TARGET_NORMAL|TARGET_ALL|TARGET_BLOCK_CURRENT;
   bool lines_based_on_scope=FALSE;

   TRACE_FUNCTION("comm2.c:   DeleteLine");
   /*
    * If no parameter is supplied, 1 is assumed.
    */
   if (blank_field(params))
   {
      args = (CHARTYPE *)"+1";
   }
   else
      args = params;
   initialise_target(&target);
   if ((rc = validate_target(args,&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If the target is BLOCK and the marked block is a box block, call
    * box_operations(), otherwise delete specified lines.
    */
   if (target.rt[0].target_type == TARGET_BLOCK_CURRENT)
   {
      /*
       * For box blocks, call the appropriate function...
       */
      if (MARK_VIEW->mark_type != M_LINE)
      {
         free_target(&target);
         box_operations(BOX_D,SOURCE_BLOCK_RESET,FALSE,' ');
         TRACE_RETURN();
         return(RC_OK);
      }
      start_line = MARK_VIEW->mark_start_line;
      end_line = MARK_VIEW->mark_end_line;
      dest_line = MARK_VIEW->mark_start_line;
      lines_based_on_scope = FALSE;
   }
   else
   {
      start_line = target.true_line;
      if (target.num_lines < 0L)
      {
         end_line = (target.true_line + target.num_lines) + 1L;
         dest_line = end_line;
      }
      else
      {
         end_line = (target.true_line + target.num_lines) - 1L;
         dest_line = start_line;
      }
      lines_based_on_scope = TRUE;
   }
   free_target(&target);
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   if (target.num_lines != 0L)
   {
      rc = rearrange_line_blocks(COMMAND_DELETE,SOURCE_COMMAND,start_line,
                               end_line,dest_line,1,CURRENT_VIEW,CURRENT_VIEW,
                               lines_based_on_scope,&lines_affected);
   }
   CURRENT_VIEW->current_line = find_next_in_scope(CURRENT_VIEW,NULL,CURRENT_VIEW->current_line,DIRECTION_FORWARD);
   start_line = find_next_in_scope(CURRENT_VIEW,NULL,CURRENT_VIEW->focus_line,DIRECTION_FORWARD);
   if (CURRENT_VIEW->focus_line != start_line)
   {
      CURRENT_VIEW->focus_line = start_line;
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   }
   if (rc == RC_OK)
   {
      if (CURRENT_BOF || CURRENT_TOF)
         rc = RC_TOF_EOF_REACHED;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     dialog - display a user configurable dialog box

SYNTAX
     DIALOG /prompt/ [EDITfield [/val/]] [TITLE /title/] [OK|OKCANCEL|YESNO|YESNOCANCEL] [DEFBUTTON n]

DESCRIPTION
     The DIALOG command displays a dialog box in the middle of the screen
     with user-configurable settings.

     The mandatory 'prompt' parameter, is the text of a prompt displayed
     near the top of the dialog window. Up to 10 lines can be displayed
     by separating lines with a character (decimal 10).

     'EDITfield' creates a user enterable field, with a default value
     of 'val', if supplied. While the cursor is in the editfield, "normal"
     edit keys are in effect. See <READV> for more details on keys that are
     useable in the editfield.  The same keys that exit from the <READV>
     command also exit the editfield. On exit from the editfield, the
     first button becomes active.

     'title' specifies optional text to be displayed on the border of
     the dialog box.

     The type of button combination can be specifed as one of the following:

          OK - just an OK button is displayed
          OKCANCEL - an OK and a CANCEL button are displayed
          YESNO - a YES and a NO button are displayed
          YESNOCANCEL - a YES, a NO and a CANCEL button are displayed

     If no button combination is selected, an OK button is displayed.

     If 'DEFBUTTON' is specified, it indicates which of the buttons is to
     be set as the active button. This is a number between 1 and the
     number of buttons displayed. By default, button 1 is active. If
     'EDITfield' is specified, no active button is set.

     The active button can be selected by pressing the TAB key; to exit
     from the DIALOG, press the RETURN or ENTER key, or click the first
     mouse button on the required button.

     On exit from the DIALOG command, the following Rexx variables are set:

          DIALOG.0 - 2
          DIALOG.1 - value of 'EDITfield'
          DIALOG.2 - button selected as specified in the call to the command.

     The colours used for the dialog box are:

          Border          -  <SET COLOR> DIVIDER
          Prompt area     -  <SET COLOR> DIALOG
          Editfield       -  <SET COLOR> CMDLINE
          Inactive button -  <SET COLOR> BLOCK
          Active button   -  <SET COLOR> CBLOCK

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible. Does not support bitmap icons or font options.

SEE ALSO
     <POPUP>, <ALERT>, <READV>, <SET COLOR>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Dialog(CHARTYPE *params)
#else
short Dialog(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION( "comm2.c:   Dialog" );
if ( BIRTHDAYx == FALSE )
{
   rc = prepare_dialog( params, FALSE, (CHARTYPE *)"DIALOG", FALSE );
}
else
{
   rc = prepare_dialog( params, FALSE, (CHARTYPE *)"DIALOG", TRUE );
BIRTHDAYx = FALSE;
}
   TRACE_RETURN();
 return(rc);
}

/*man-start*********************************************************************
COMMAND
     directory - list the specified directory as an editable file

SYNTAX
     DIRectory [file specification]

DESCRIPTION
     The DIRECTORY command displays all files matching the specified
     'file specification'.

     When no parameter is supplied, all files in the current directory
     are displayed subject to any <SET DIRINCLUDE> restrictions.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <LS>, <SET DIRINCLUDE>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Directory(CHARTYPE *params)
#else
short Directory(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#if !defined(MULTIPLE_PSEUDO_FILES)
#endif
#define DIR_PARAMS  1
   CHARTYPE *word[DIR_PARAMS+1];
   CHARTYPE strip[DIR_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK;

   TRACE_FUNCTION( "comm2.c:   Directory" );
   /*
    * Validate the parameters that have been supplied. The one and only
    * parameter should be the directory to display.
    */
   strip[0] = STRIP_BOTH;
   num_params = param_split( params, word, DIR_PARAMS, WORD_DELIMS, TEMP_PARAM, strip, FALSE );
   if ( num_params > 1 )
   {
      display_error( 1, (CHARTYPE *)word[1], FALSE );
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Validate that the supplied directory is valid.
    */
   if ( ( rc = splitpath( strrmdup( strtrans( word[0], OSLASH, ISLASH), ISLASH, TRUE ) ) ) != RC_OK )
   {
      display_error( 10, (CHARTYPE *)word[0], FALSE );
      TRACE_RETURN();
      return(rc);
   }
   if ( ( rc = read_directory() ) != RC_OK )
   {
      if ( strcmp( (DEFCHAR *)sp_fname, "" ) == 0 )
         display_error( 10, (CHARTYPE *)word[0], FALSE );
      else
         display_error( 9, (CHARTYPE *)word[0], FALSE );
      TRACE_RETURN();
      return(rc);
   }
#if 0
 if (CURRENT_VIEW != NULL)
    pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
#endif

#if defined(MULTIPLE_PSEUDO_FILES)
   strcpy( (DEFCHAR *)temp_cmd, (DEFCHAR *)dir_path );
   strcat( (DEFCHAR *)temp_cmd, (DEFCHAR *)dir_files );
#else
   strcpy( (DEFCHAR *)temp_cmd, (DEFCHAR *)dir_pathname );
   strcat( (DEFCHAR *)temp_cmd, (DEFCHAR *)dir_filename );
#endif
   /*
    * Edit the DIR.DIR file
    */
   Xedit( temp_cmd );

   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     dos - execute an operating system command

SYNTAX
     DOS [command]

DESCRIPTION
     The DOS command executes the supplied operating system 'command'
     or runs an interactive shell if no 'command' is supplied.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <OS>, <!>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     dosnowait - execute an operating system command - no prompt

SYNTAX
     DOSNowait command

DESCRIPTION
     The DOSNOWAIT command executes the supplied operating system
     command not waiting for the user to be prompted once the
     command has completed.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <OSNOWAIT>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     dosquiet - execute an operating system command quietly

SYNTAX
     DOSQuiet command

DESCRIPTION
     The DOSQUIET command executes the supplied operating system 'command'
     as quietly as possible.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

SEE ALSO
     <OSQUIET>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     down - move forward in the file a number of lines

SYNTAX
     Down [relative target]

DESCRIPTION
     The DOWN command moves the <current line> forwards the number of
     lines specified by the <'relative target'>. This <'relative target'>
     can only be a positive integer or the character "*".

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     1

SEE ALSO
     <NEXT>, <UP>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     duplicate - duplicate lines

SYNTAX
     DUPlicate [n [target|BLOCK]]

DESCRIPTION
     The DUPLICATE command copies the number of lines extrapolated from
     <'target'> or the marked 'BLOCK', 'n' times.

COMPATIBILITY
     XEDIT: Equivalent of DUPLICAT command.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Duplicate(CHARTYPE *params)
#else
short Duplicate(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define DUP_PARAMS  2
   CHARTYPE *word[DUP_PARAMS+1];
   CHARTYPE strip[DUP_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK,num_occ=0;
   LINETYPE start_line=0L,end_line=0L,dest_line=0L,lines_affected=0L;
   CHARTYPE command_source=0;
   TARGET target;
   short target_type=TARGET_NORMAL|TARGET_BLOCK_CURRENT|TARGET_ALL;
   bool lines_based_on_scope=FALSE;

   TRACE_FUNCTION("comm2.c:   Duplicate");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_LEADING;
   num_params = param_split(params,word,DUP_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   /*
    * If no parameters, default to 1 1
    */
   if (num_params == 0)
   {
      word[0] = (CHARTYPE *)"1";
      word[1] = (CHARTYPE *)"1";
   }
   /*
    * If 1 parameter, default 2nd parameter to 1
    */
   if (num_params == 1)
      word[1] = (CHARTYPE *)"1";
   /*
    * If first parameter is not an integer, error.
    */
   if (!valid_integer(word[0]))
   {
      display_error(4,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   num_occ = atoi((DEFCHAR *)word[0]);
   /*
    * Validate second parameter is a valid target...
    */
   initialise_target(&target);
   if ((rc = validate_target(word[1],&target,target_type,get_true_line(TRUE),TRUE,TRUE)) != RC_OK)
   {
      free_target(&target);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * Duplicate lines depending on target type...
    */
   switch(target.rt[0].target_type)
   {
      case TARGET_BLOCK_CURRENT:
         /*
          * This function not valid for box  blocks.
          */
         if (MARK_VIEW->mark_type == M_BOX)
         {
            display_error(48,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         command_source = SOURCE_BLOCK;
         start_line = MARK_VIEW->mark_start_line;
         end_line = dest_line = MARK_VIEW->mark_end_line;
         lines_based_on_scope = FALSE;
         break;
      default:
         command_source = SOURCE_COMMAND;
         if (target.num_lines < 0L)
         {
            start_line = target.true_line + target.num_lines + 1L;
            end_line = dest_line = target.true_line;
         }
         else
         {
            start_line = target.true_line;
            end_line = dest_line = (target.true_line + target.num_lines) - 1L;
         }
         lines_based_on_scope = TRUE;
         break;
   }
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   rc = rearrange_line_blocks(COMMAND_DUPLICATE,command_source,
                            start_line,end_line,dest_line,num_occ,
                            CURRENT_VIEW,CURRENT_VIEW,lines_based_on_scope,
                            &lines_affected);
   free_target(&target);
   TRACE_RETURN();
   return(rc);
}

/*man-start*********************************************************************
COMMAND
     edit - edit another file or switch to next file

SYNTAX
     Edit [file]

DESCRIPTION
     The EDIT command allows the user to edit another 'file'. The new file
     is placed in the file <ring>. The previous file being edited remains
     in memory and can be returned to by issuing an EDIT command without
     any parameters. Several files can be edited at once, and all files
     are arranged in a ring, with subsequent EDIT commands moving through
     the ring, one file at a time.

COMPATIBILITY
     XEDIT: Does not provide options switches.
     KEDIT: Does not provide options switches.

SEE ALSO
     <THE>, <XEDIT>, <KEDIT>

STATUS
     Complete.
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     editv - set and retrieve persistent macro variables

SYNTAX
     EDITV GET|PUT|GETF|PUTF var1 [var2 ...]
     EDITV SET|SETF var1 value1 [var2 value2 ...]
     EDITV SETL|SETLF|SETFL var1 value1
     EDITV LIST|LISTF [var1 ...]

DESCRIPTION
     The EDITV command manipulates variables for the lifetime of the
     edit session or the file, depending on the subcommand used.

     Edit variables are useful for maintaining variable values from
     one execution of a macro to another.

     EDITV GET, PUT, GETF and PUTF are only valid from within a macro
     as they reference Rexx variables.  All other subcommands are valid
     from within a macro or from the command line.

     EDITV GET sets a Rexx macro variable, with the same name as the
     edit variable, to the value of the edit variable.

     EDITV PUT stores the value of a Rexx macro variable as an edit
     variable.

     EDITV SET stores an edit variable with a value.

     EDITV SET can only work with variable values comprising a single
     space-seperated word.  To specify a variable value that contains
     spaces, use EDITV SETL.

     EDITV LIST displays the values of the specified edit variables, or
     all variables if no edit variables are specified.

     EDITV GETF, PUTF, SETF, SETLF, SETFL, and LISTF all work the same
     way as their counterparts without the F, but the variables are
     only available while the particular file is the current file.  This
     enables you to use the same edit variable name but with different
     values for different files.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short THEEditv(CHARTYPE *params)
#else
short THEEditv(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define EDITV_PARAMS  2
   CHARTYPE *word[EDITV_PARAMS+1];
   CHARTYPE strip[EDITV_PARAMS];
   unsigned short num_params=0;
   short editv_type=0;
   short rc=RC_OK;
   bool editv_file = FALSE;

   TRACE_FUNCTION("comm2.c:   THEEditv");
   strip[0] = STRIP_BOTH;
   strip[1] = STRIP_LEADING;
   num_params = param_split(params,word,EDITV_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Determine the subcommand...
    */
   if (equal((CHARTYPE *)"get",word[0],3))
   {
      editv_type = EDITV_GET;
   }
   else if (equal((CHARTYPE *)"put",word[0],3))
   {
      editv_type = EDITV_PUT;
   }
   else if (equal((CHARTYPE *)"set",word[0],3))
   {
      editv_type = EDITV_SET;
   }
   else if (equal((CHARTYPE *)"setl",word[0],4))
   {
      editv_type = EDITV_SETL;
   }
   else if (equal((CHARTYPE *)"list",word[0],4))
   {
      editv_type = EDITV_LIST;
   }
   else if (equal((CHARTYPE *)"getf",word[0],4))
   {
      editv_type = EDITV_GET;
      editv_file = TRUE;
   }
   else if (equal((CHARTYPE *)"putf",word[0],4))
   {
      editv_type = EDITV_PUT;
      editv_file = TRUE;
   }
   else if (equal((CHARTYPE *)"setf",word[0],4))
   {
      editv_type = EDITV_SET;
      editv_file = TRUE;
   }
   else if (equal((CHARTYPE *)"setlf",word[0],5))
   {
      editv_type = EDITV_SETL;
      editv_file = TRUE;
   }
   else if (equal((CHARTYPE *)"setfl",word[0],5))
   {
      editv_type = EDITV_SETL;
      editv_file = TRUE;
   }
   else if (equal((CHARTYPE *)"listf",word[0],5))
   {
      editv_type = EDITV_LIST;
      editv_file = TRUE;
   }
   else
   {
      display_error(1,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Only LIST and LISTF are allowed no parameters...
    */
   if (editv_type != EDITV_LIST
   &&  num_params == 1)
   {
      display_error(3,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * GET, PUT, GETF and PUTF only allowed in a macro...
    */
   if (editv_type == EDITV_GET
   ||  editv_type == EDITV_PUT)
   {
      if (!in_macro)
      {
         display_error(53,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_ENVIRON);
      }
   }
   rc = execute_editv(editv_type,editv_file,word[1]);
   TRACE_RETURN();
   return(RC_OK);
}

/*man-start*********************************************************************
COMMAND
     emsg - display message

SYNTAX
     EMSG [message]

DESCRIPTION
     The EMSG command displays an 'message' on the <message line>.
     This command is usually issued from a macro file.

COMPATIBILITY
     XEDIT: Does not support [mmmnnns text] option
     KEDIT: Compatible

SEE ALSO
     <CMSG>, <MSG>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Emsg(CHARTYPE *params)
#else
short Emsg(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("comm2.c:   Emsg");
   display_error(0,params,FALSE);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     enter - execute a command

SYNTAX
     enter [CUA]

DESCRIPTION
     The ENTER command executes the command currently displayed on the
     command line, if the cursor is currently displayed there.
     If the key associated with ENTER is pressed while in the <filearea>,
     then the cursor will move to the first column of the
     next line. If the cursor is in the <prefix area>, any pending
     prefix commands will be executed. If the mode is currently in
     'insert', then a new line is added and the cursor placed on the
     next line depending on the value of <SET NEWLINES>.

     This command can only be used by assigning it to a function key
     with the <DEFINE> command.

     With the optional 'CUA' argument, when in the <FILEAREA>, the enter
     command acts like the <SPLIT> command.

     This command will be removed in a future version.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: N/A

SEE ALSO
     <SOS EXECUTE>, <ENTER>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Enter(CHARTYPE *params)
#else
short Enter(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   unsigned short x=0,y=0;
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Enter");
   switch(CURRENT_VIEW->current_window)
   {
      case WINDOW_COMMAND:
         rc = Sos_execute((CHARTYPE *)"");
         break;
      case WINDOW_PREFIX:
         post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
         if (CURRENT_FILE->first_ppc == NULL)/* no pending prefix cmds */
         {
            THEcursor_down(TRUE);
            rc = Sos_firstcol((CHARTYPE *)"");
         }
         else
            Sos_doprefix((CHARTYPE *)"");
         break;
      case WINDOW_FILEAREA:
         /*
          * If in readonly mode, ignore new line addition...
          */
         if (!ISREADONLY(CURRENT_FILE))
         {
            if ( equal((CHARTYPE *)"cua", params, 3 ) )
            {
               /*
                * Split the line at the cursor position
                * move the cursor to the first character of the FILEAREA
                */
               if ( CURRENT_VIEW->newline_aligned )
               {
                  rc = execute_split_join(SPLTJOIN_SPLIT,TRUE,TRUE);
                  rc = Sos_firstchar((CHARTYPE *)"");
               }
               else
               {
                  rc = execute_split_join(SPLTJOIN_SPLIT,FALSE,TRUE);
                  rc = Sos_firstcol((CHARTYPE *)"");
               }
               THEcursor_down(TRUE);
               TRACE_RETURN();
               return(rc);
            }
            else
            {
               if (CURRENT_VIEW->inputmode == INPUTMODE_LINE)
               {
                  post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
                  insert_new_line((CHARTYPE *)"",0,1,get_true_line(FALSE),FALSE,FALSE,TRUE,CURRENT_VIEW->display_low,TRUE,TRUE);
                  break;
               }
            }
         }
         THEcursor_down(TRUE);
         getyx(CURRENT_WINDOW,y,x);
         wmove(CURRENT_WINDOW,y,0);
         break;
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     expand - expand tab characters to spaces

SYNTAX
     EXPand [target]

DESCRIPTION
     The EXPAND command converts all tab characters to spaces in the
     <'target'> depending on the size of a tab determined by the
     <SET TABS> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <COMPRESS>, <SET TABS>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Expand(CHARTYPE *params)
#else
short Expand(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Expand");
   rc = execute_expand_compress(params,TRUE,TRUE,TRUE,TRUE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     extract - obtain various internal information about THE

SYNTAX
     EXTract /item/[...]

DESCRIPTION
     The EXTRACT command is used to relay information about settings
     within THE from within a Rexx macro. EXTRACT is only valid within
     a Rexx macro.

     The '/' in the syntax clause represents any delimiter character.

     For a complete list of 'item's that can be extracted, see the section;
     <QUERY, EXTRACT and STATUS>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Extract(CHARTYPE *params)
#else
short Extract(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   register short i=0;
   short rc=RC_OK,itemno=0,num_items=0,len=0,num_values=0;
   short pos=0,arglen=0;
   CHARTYPE *args=NULL;
   CHARTYPE delim;
   bool invalid_item=FALSE;
   CHARTYPE item_type=0;

   TRACE_FUNCTION("comm2.c:   Extract");
   if (!in_macro
   ||  !rexx_support)
   {
      display_error(53,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * The first character is saved as the delimiter...
    */
   delim = *(params);
   params++;                             /* throw away first delimiter */
   strtrunc(params);
   len = strlen((DEFCHAR *)params);
   /*
    * Check that we have an item to extract...
    */
   if (len == 0)
      invalid_item = TRUE;
   else
   {
      if (len == 1 && (*(params) == delim))
         invalid_item = TRUE;
   }
   if (invalid_item)
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * Allow for no trailing delimiter...
    */
   if ((*(params+len-1) == delim))
      num_items = 0;
   else
      num_items = 1;
   /*
    * Replace all / with nul character to give us seperate strings.
    */
   for (i=0;i<len;i++)
   {
      if (*(params+i) == delim)
      {
         *(params+i) = '\0';
         num_items++;
      }
   }
   /*
    * For each item, extract its variables...
    */
   for (i=0;i<num_items;i++)
   {
      /*
       * First check if the item has any arguments with it.
       */
      arglen = strlen((DEFCHAR *)params);
      pos = strzeq(params,' ');
      if (pos == (-1))
         args = (CHARTYPE *)"";
      else
      {
         *(params+pos) = '\0';
         args = strtrunc(params+pos+1);
      }
      /*
       * Find the item in the list of valid extract options...
       */
      if ((itemno = find_query_item(params,strlen((DEFCHAR*)params),&item_type)) == (-1)
      ||  !(item_type & QUERY_EXTRACT))
      {
         display_error(1,params,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * Get the current settings for the valid item...
       */
      num_values = get_item_values(1,itemno,args,QUERY_EXTRACT,0L,NULL,0L);
      /*
       * If the arguments to the item are invalid, return with an error.
       */
      if (num_values == EXTRACT_ARG_ERROR)
      {
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
      /*
       * If the REXX variables have already been set, don't try to set them.
       */
      if (num_values != EXTRACT_VARIABLES_SET)
      {
         rc = set_extract_variables(itemno);
         if (rc == RC_SYSTEM_ERROR)
            break;
      }
      params += arglen+1;
   }

   TRACE_RETURN();
   return(rc);
}

/*man-start*********************************************************************
COMMAND
     ffile - force a FILE of the current file to disk

SYNTAX
     FFile  [filename]

DESCRIPTION
     The FFILE command writes the current file to disk to the current
     file name or to the supplied 'filename'.
     Unlike the <FILE> command, if the optional 'filename' exists, this
     command will overwrite the file.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

DEFAULT
     With no parameters, the current file is written.

SEE ALSO
     <FILE>, <SAVE>, <SSAVE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Ffile(CHARTYPE *params)
#else
short Ffile(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Ffile");
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   if ((rc = save_file(CURRENT_FILE,params,TRUE,CURRENT_FILE->number_lines,1L,NULL,FALSE,0,max_line_length,TRUE,FALSE,FALSE)) != RC_OK)
   {
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If autosave is on at the time of FFiling, remove the .aus file...
    */
   if (CURRENT_FILE->autosave > 0)
      rc = remove_aus_file(CURRENT_FILE);
   free_view_memory(TRUE,TRUE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     file - write the current file to disk and remove from ring

SYNTAX
     FILE  [filename]

DESCRIPTION
     The FILE command writes the current file to disk to the current
     file name or to the supplied 'filename'.
     Unlike the <FFILE> command, if the optional 'filename' exists, this
     command will not overwrite the file.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

DEFAULT
     With no parameters, the current file is written.

SEE ALSO
     <FFILE>, <SAVE>, <SSAVE>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short File(CHARTYPE *params)
#else
short File(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   File");
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   /*
    * If we are filing the current file with the same name AND the number
    * of alterations is zero, then quit the file.
    * Removed to be consistant with XEDIT/KEDIT.
    */
/*
   if (CURRENT_FILE->save_alt == 0 && strcmp(params,"") == 0)
      Quit((CHARTYPE *)"");
   else
*/
   {
      if ((rc = save_file(CURRENT_FILE,params,FALSE,CURRENT_FILE->number_lines,1L,NULL,FALSE,0,max_line_length,TRUE,FALSE,FALSE)) != RC_OK)
      {
         TRACE_RETURN();
         return(rc);
      }
      /*
       * If autosave is on at the time of Filing, remove the .aus file...
       */
      if (CURRENT_FILE->autosave > 0)
         rc = remove_aus_file(CURRENT_FILE);
      free_view_memory(TRUE,TRUE);
   }
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     fillbox - fill the marked block with a character

SYNTAX
     FILLbox [c]

DESCRIPTION
     The FILLBOX command fills the marked block with the specified
     character, 'c'. If no parameters are supplied and the command is run
     from the command line, then the block will be filled with spaces.
     If the command is not run from the command line, the user is
     prompted for a character to fill the box.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Compatible.

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Fillbox(CHARTYPE *params)
#else
short Fillbox(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   int key=0;
   short len_params=0;
   short y=0,x=0;

   TRACE_FUNCTION("comm2.c:   Fillbox");
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   /*
    * Validate the marked block.
    */
   if (marked_block(TRUE) != RC_OK)
   {
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   /*
    * Check if hex on in effect and translate hex char if required...
    */
   if (CURRENT_VIEW->hex)
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
   /*
    * Whew, now do something...
    */
   if (len_params > 1)
   {
      display_error(1,params,FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if (len_params == 0)
      key = (int)' ';
   else
      key = (int)*(params);
   if (CURRENT_VIEW->current_window != WINDOW_COMMAND
   && len_params != 1)
   {
      getyx(CURRENT_WINDOW,y,x);
      display_prompt((CHARTYPE *)"Enter fill character...");
      wmove(CURRENT_WINDOW_FILEAREA,y,x);
      wrefresh(CURRENT_WINDOW_FILEAREA);
      while(1)
      {
         key = my_getch(stdscr);
         if ( !is_modifier_key( key ) )
            break;
      }
      clear_msgline(-1);
   }
   box_operations(BOX_F,SOURCE_BLOCK,TRUE,(CHARTYPE)key);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     find - locate forwards the line which begins with the supplied string

SYNTAX
     Find [string]

DESCRIPTION
     The FIND command attempts to locate a line towards the end of
     the file that begins with 'string'.
     If the optional 'string' is not supplied the last 'string' used
     in any of the family of find commands is used.

     'string' can contain two special characters:

          space - this will match any single character in the target line
          underscore - this will match any single space in the target line

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FINDUP>, <NFIND>, <NFINDUP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Find(CHARTYPE *params)
#else
short Find(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Find");
   rc = execute_find_command(params,TARGET_FIND);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     findup - locate backwards the line which begins with the supplied string

SYNTAX
     FINDUp [string]

DESCRIPTION
     The FINDUP command attempts to locate a line towards the start of
     the file that begins with 'string'.
     If the optional 'string' is not supplied the last 'string' used
     in any of the family of find commands is used.

     'string' can contain two special characters:

          space - this will match any single character in the target line
          underscore - this will match any single space in the target line

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FIND>, <NFIND>, <NFINDUP>, <FUP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Findup(CHARTYPE *params)
#else
short Findup(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Findup");
   rc = execute_find_command(params,TARGET_FINDUP);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     forward - scroll forwards [n] screens

SYNTAX
     FOrward [n]

DESCRIPTION
     The FORWARD command scrolls the file contents forwards 'n' screens.

     If 0 is specified as the number of screens to scroll, the
     <Top-of-File line> becomes the <current line>.

     If the FORWARD command is issued while the <current line> is the
     <Bottom-of-File line>, the <Top-of-File line> becomes the
     <current line>.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Does not support HALF or Lines options.

DEFAULT
     1

SEE ALSO
     <BACKWARD>, <TOP>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Forward(CHARTYPE *params)
#else
short Forward(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define FOR_PARAMS  1
   CHARTYPE *word[FOR_PARAMS+1];
   CHARTYPE strip[FOR_PARAMS];
   unsigned short num_params=0;
   LINETYPE num_pages=0L;
   short rc=RC_OK;

   TRACE_FUNCTION("comm2.c:   Forward");
   /*
    * Validate parameters...
    */
   strip[0]=STRIP_BOTH;
   num_params = param_split(params,word,FOR_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
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
    * If parameter is '*', set current line equal to last line in file...
    */
   if (strcmp((DEFCHAR *)word[0],"*") == 0)
   {
      rc = Bottom((CHARTYPE *)"");
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
    * If the current line is already on "Bottom of File" or the parameter
    * is 0, go to the top of the file.
    */
   if ( num_pages == 0
   || ( CURRENT_BOF && PAGEWRAPx ) )
   {
      rc = Top((CHARTYPE *)"");
      TRACE_RETURN();
      return(rc);
   }
   /*
    * Scroll the screen num_pages...
    */
   rc = scroll_page(DIRECTION_FORWARD,num_pages,FALSE);
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     fup - locate backwards the line which begins with the supplied string

SYNTAX
     FUp [string]

DESCRIPTION
     The FUP command is a synonym for the <FINDUP> command.

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <FIND>, <NFIND>, <NFINDUP>, <FINDUP>

STATUS
     Complete
**man-end**********************************************************************/

/*man-start*********************************************************************
COMMAND
     get - insert into file the contents of specified file

SYNTAX
     GET [filename] [fromline] [numlines]
     GET CLIP: [STREAM|BOX|LINE]

DESCRIPTION
     The GET command reads a file into the current file, inserting
     lines after the current line.

     When no 'filename' is supplied the temporary file generated by the
     last <PUT> or <PUTD> command is used.

     When 'fromline' is specified, reading of the file begins at the
     line number specified.
     If 'fromline' is not specifed, reading begins at line 1.

     When 'numlines' is specified, reading of the file ends when the
     specified number of lines has been read.
     If 'numlines' is not specified, or 'numlines' is specified as '*',
     all files from the 'fromline' to the end of file are read.

     The second form implements interaction with the system clipboard.
     The optional parameter indicates how the contents of the
     clipboard is to be inserted into the file. If not supplied the
     contents of the clipboard is treated as a LINE block.
     This option only available for X11, OS/2 and Win32 ports of THE.
     (Incomplete)

COMPATIBILITY
     XEDIT: Compatible.
     KEDIT: Compatible.

SEE ALSO
     <PUT>, <PUTD>

STATUS
     Complete
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Get(CHARTYPE *params)
#else
short Get(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define GET_PARAMS  3
   CHARTYPE *word[GET_PARAMS+1];
   CHARTYPE strip[GET_PARAMS];
   unsigned short num_params=0;
   CHARTYPE *filename=NULL;
   FILE *fp=NULL;
   _LINE *curr=NULL;
   _LINE *save_curr=NULL;
   _LINE *save_next=NULL;
   LINETYPE old_number_lines=0L,true_line=0L;
   short rc=RC_OK;
   LINETYPE fromline=1L,numlines=0L;
   bool clip=FALSE;
   int clip_type;

   TRACE_FUNCTION("comm2.c:   Get");
   /*
    * Validate the parameters that have been supplied.
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   strip[2]=STRIP_BOTH;
   num_params = param_split(params,word,GET_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params > 3)
   {
      display_error(2,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_ENVIRON);
   }
   if (num_params == 0)
      filename = tempfilename;
   else
   {
      if (equal((CHARTYPE *)"clip:",word[0],5))
      {
         clip = TRUE;
         if ( num_params > 2 )
         {
            display_error(2,(CHARTYPE *)"",FALSE);
            TRACE_RETURN();
            return(RC_INVALID_ENVIRON);
         }
         else if ( num_params == 1 )
         {
            clip_type = M_LINE;
         }
         else
         {
            if (equal( (CHARTYPE *)"line", word[1], 4) )
               clip_type = M_LINE;
            else if (equal( (CHARTYPE *)"stream", word[1], 6) )
               clip_type = M_STREAM;
            else if (equal( (CHARTYPE *)"box", word[1], 3) )
               clip_type = M_BOX;
            else
            {
               display_error(1,word[1],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_ENVIRON);
            }
         }
      }
      else
      {
         if ((rc = splitpath(strrmdup(strtrans(word[0],OSLASH,ISLASH),ISLASH,TRUE))) != RC_OK)
         {
            display_error(10,word[0],FALSE);
            TRACE_RETURN();
            return(rc);
         }
         strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)sp_path);
         strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)sp_fname);
         filename = temp_cmd;
         if (num_params == 2
         ||  num_params == 3)
         {
            if (!valid_positive_integer(word[1]))
            {
               display_error(4,word[1],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
            fromline = atol((DEFCHAR *)word[1]);
            if (fromline == 0L)
            {
               display_error(4,word[1],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_OPERAND);
            }
         }
         if (num_params == 3)
         {
            if (strcmp((DEFCHAR *)word[2],"*") == 0)
               numlines = 0L;
            else
            {
               if (!valid_positive_integer(word[2]))
               {
                  display_error(4,word[2],FALSE);
                  TRACE_RETURN();
                  return(RC_INVALID_OPERAND);
               }
               else
                  numlines = atol((DEFCHAR *)word[2]);
            }
         }
      }
   }

   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);

   if (!clip)
   {
      if (!file_readable(filename))
      {
         display_error(8,filename,FALSE);
         TRACE_RETURN();
         return(RC_ACCESS_DENIED);
      }

      if ((fp = fopen((DEFCHAR *)filename,"r")) == NULL)
      {
         display_error(9,params,FALSE);
         TRACE_RETURN();
         return(RC_ACCESS_DENIED);
      }
   }
   true_line = get_true_line(TRUE);
   curr = lll_find(CURRENT_FILE->first_line,CURRENT_FILE->last_line,true_line,CURRENT_FILE->number_lines);
   if (curr->next == NULL)   /* on bottom of file */
      curr = curr->prev;
   old_number_lines = CURRENT_FILE->number_lines;
   save_curr = curr;
   save_next = curr->next;
   if (clip)
      curr = getclipboard(curr);
   else
      curr = read_file(fp,curr,filename,fromline,numlines,TRUE);
   if (curr == NULL)
   {
      for (curr=save_curr;curr!=save_next;)
      {
         if (curr != save_curr)
            curr = lll_del(&CURRENT_FILE->first_line,&CURRENT_FILE->last_line,curr,DIRECTION_FORWARD);
         else
            curr = curr->next;
      }
      pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
      if (!clip)
         fclose(fp);
      TRACE_RETURN();
      return(RC_ACCESS_DENIED);
   }

   if (!clip)
      fclose(fp);
   pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
   /*
    * Fix the positioning of the marked block (if there is one and it is
    * in the current view).
    */
   adjust_marked_lines(TRUE,true_line,CURRENT_FILE->number_lines - old_number_lines);
   adjust_pending_prefix(CURRENT_VIEW,TRUE,true_line,CURRENT_FILE->number_lines - old_number_lines);
   /*
    * Increment the number of lines counter for the current file and the
    * number of alterations.
    */
   increment_alt(CURRENT_FILE);

   build_screen(current_screen);
   display_screen(current_screen);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     help - edit help file for THE

SYNTAX
     HELP

DESCRIPTION
     The HELP command displays help for the editor.
     Uses THE_HELP_FILE environment variable to point to the help file.
     See Appendix 1 for details on this and other environemnt variables.

COMPATIBILITY
     XEDIT: Similar in concept.
     KEDIT: Similar in concept.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Help(CHARTYPE *params)
#else
short Help(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   static bool first=TRUE;
   char *envptr=NULL;

   TRACE_FUNCTION("comm2.c:   Help");
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
    * Set up help file name.
    */
   if (first)
   {
      if ((envptr = getenv("THE_HELP_FILE")) != NULL)
         strcpy((DEFCHAR *)the_help_file,envptr);
      else
      {
         strcpy((DEFCHAR *)the_help_file,(DEFCHAR *)the_home_dir);
         strcat((DEFCHAR *)the_help_file,"tedit.hlp");
      }
      (void *)strrmdup(strtrans(the_help_file,OSLASH,ISLASH),ISLASH,TRUE);
      first = FALSE;
   }
   if (!file_exists(the_help_file))
   {
      display_error(23,(CHARTYPE *)the_help_file,FALSE);
      TRACE_RETURN();
      return(RC_FILE_NOT_FOUND);
   }
   Xedit(the_help_file);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     hit - simulate hitting of the named key

SYNTAX
     HIT key

DESCRIPTION
     The HIT command enables the simulation of hitting the named 'key'.
     This is most useful from within a macro.

     Be very careful when using the HIT command with the <DEFINE> command.
     If you assign the HIT command to a key, DO NOT use the same key
     name. eg. DEFINE F1 HIT F1
     This will result in an infinite processing loop.

COMPATIBILITY
     XEDIT: N/A
     KEDIT: Similar, but more like the <MACRO> command.

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Hit(CHARTYPE *params)
#else
short Hit(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define HIT_MOUSE_PARAMS  4
   CHARTYPE *word[HIT_MOUSE_PARAMS+1];
   CHARTYPE strip[HIT_MOUSE_PARAMS];
   unsigned short num_params=0;
   int key=0;
   short rc=RC_OK;
   bool save_in_macro=in_macro;
   bool mouse_details_present=FALSE;

   TRACE_FUNCTION("comm2.c:   Hit");
   /*
    * Prase the parameters into multiple words. If only one word then it
    * must be a key.  If 3 words its a mouse key, otherwise asn error.
    */
   strip[0] = STRIP_BOTH;
   strip[1] = STRIP_BOTH;
   strip[2] = STRIP_BOTH;
   strip[3] = STRIP_NONE;
   num_params = param_split(params,word,HIT_MOUSE_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   switch(num_params)
   {
      case 1:  /* key definition */
         key = find_key_name(params);
         if (key == (-1))
         {
            display_error(13,params,FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
         break;
      case 3:  /* mouse definition */
         if (!equal((CHARTYPE *)"in",word[1],2))
         {
            display_error(1,word[1],FALSE);
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
         if ((key = find_mouse_key_value(word[0],word[2])) == (-1))
         {
            TRACE_RETURN();
            return(RC_INVALID_OPERAND);
         }
         mouse_details_present = TRUE;
         break;
      default: /* error */
         break;
   }
   /*
    * Only argument is the name of a valid key.
    */
   in_macro = FALSE;
   rc = process_key(key, mouse_details_present);
   in_macro = save_in_macro;
   if (number_of_files == 0)
      rc = RC_INVALID_ENVIRON;
   /* how to exit ???? */
   TRACE_RETURN();
   return(rc);
}
/*man-start*********************************************************************
COMMAND
     input - insert the command line contents into the file

SYNTAX
     Input [string]

DESCRIPTION
     The INPUT command inserts the 'string' specified on the <command line>
     into the current file after the <current line>.

     If <SET INPUTMODE> FULL is in effect, and the INPUT command is
     entered on the command line with no arguments, THE is put into
     full input mode.  If the <prefix area> is on, it is turned off,
     the cursor moved to the <filearea> and blank lines inserted
     into the file from the <current line> to the end of the screen.

     To get out of full input mode, press the key assigned to the
     <CURSOR> HOME [SAVE] command.

COMPATIBILITY
     XEDIT: Does not provide full input mode option.
     KEDIT: Does not provide full input mode option.

STATUS
     Complete. Except for full input mode capability.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Input(CHARTYPE *params)
#else
short Input(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
   LINETYPE len_params=0;

   TRACE_FUNCTION("comm2.c:   Input");
   post_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL,TRUE);
   if (CURRENT_VIEW->hex)
   {
      if ((len_params = (LINETYPE)convert_hex_strings(params)) == (-1))
      {
         display_error(32,params,FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   else
      len_params = (LINETYPE)strlen((DEFCHAR *)params);
   if (len_params > (LINETYPE)max_line_length)
   {
      display_error(0,(CHARTYPE *)"Truncated",FALSE);
      len_params = (LINETYPE)max_line_length;
   }
   insert_new_line(params,(unsigned short)len_params,1L,get_true_line(TRUE),TRUE,TRUE,TRUE,CURRENT_VIEW->display_low,TRUE,FALSE);
   TRACE_RETURN();
   return(RC_OK);
}
/*man-start*********************************************************************
COMMAND
     join - join a line with the line following

SYNTAX
     Join [ALigned] [Column|CURSOR]

DESCRIPTION
     The JOIN command makes one line out of the focus line and the
     line following.

     If 'Aligned' is specified, any leading spaces in the following line
     are ignored. If 'Aligned' is not specified, all characters, including
     spaces are added.

     If 'Column' (the default) is specified, the current line is joined
     at the current column location.

     If 'CURSOR' is specified, the focus line is joined at the cursor
     position.

COMPATIBILITY
     XEDIT: Compatible.
            Does not support Colno option
     KEDIT: Compatible.

SEE ALSO
     <SPLIT>, <SPLTJOIN>

STATUS
     Complete.
**man-end**********************************************************************/
#ifdef HAVE_PROTO
short Join(CHARTYPE *params)
#else
short Join(params)
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define JOI_PARAMS  2
   CHARTYPE *word[JOI_PARAMS+1];
   CHARTYPE strip[JOI_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK;
   bool aligned=FALSE;
   bool cursorarg=FALSE;

   TRACE_FUNCTION("comm2.c:   Join");
   /*
    * Split parameters up...
    */
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_BOTH;
   num_params = param_split(params,word,JOI_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params == 0)
   {
      aligned = FALSE;
      cursorarg = FALSE;
   }
   else
   {
      if (equal((CHARTYPE *)"aligned",word[0],2))
      {
         aligned = TRUE;
         if (equal((CHARTYPE *)"cursor",word[1],6))
         {
            cursorarg = TRUE;
         }
         else
         {
            if (equal((CHARTYPE *)"column",word[1],1))
            {
               cursorarg = FALSE;
            }
            else
            {
               display_error(1,(CHARTYPE *)word[1],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_ENVIRON);
            }
         }
      }
      else
      {
         if (equal((CHARTYPE *)"cursor",word[0],6))
         {
            aligned = FALSE;
            cursorarg = TRUE;
         }
         else
         {
            if (equal((CHARTYPE *)"column",word[0],1))
            {
               aligned = FALSE;
               cursorarg = FALSE;
            }
            else
            {
               display_error(1,(CHARTYPE *)word[0],FALSE);
               TRACE_RETURN();
               return(RC_INVALID_ENVIRON);
            }
         }
      }
   }
   rc = execute_split_join(SPLTJOIN_JOIN,aligned,cursorarg);
   TRACE_RETURN();
   return(rc);
}
