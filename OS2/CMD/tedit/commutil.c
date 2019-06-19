/***********************************************************************/
/* COMMUTIL.C -                                                        */
/* This file contains all utility functions used when processing       */
/* commands.                                                           */
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

static char RCSid[] = "$Id: commutil.c,v 1.25 2002/07/11 10:23:21 mark Exp $";

#include <the.h>
#include <proto.h>
#include <key.h>
#include <command.h>

#ifdef HAVE_PROTO
static CHARTYPE *build_defined_key_definition(int, CHARTYPE *,DEFINE *,bool);
static void save_last_command(CHARTYPE *,CHARTYPE *);
void AdjustThighlight( int );
static bool save_target( TARGET * );
#else
static CHARTYPE *build_defined_key_definition();
static void save_last_command();
void AdjustThighlight();
static bool save_target();
#endif

#define HEXVAL(c) (((c)>'9')?(tolower(c)-'a'+10):((c)-'0'))

 static CHARTYPE _THE_FAR cmd_history[MAX_SAVED_COMMANDS][MAX_COMMAND_LENGTH];
 static short last_cmd=(-1),current_cmd=0,number_cmds=0,offset_cmd=0;
 CHARTYPE _THE_FAR last_command_for_reexecute[MAX_COMMAND_LENGTH];
 CHARTYPE _THE_FAR last_command_for_repeat[MAX_COMMAND_LENGTH];

#define KEY_REDEF "/* Key re-definitions */"
#define KEY_DEFAULT "/* Default key definitions */"
#define KEY_MOUSE_REDEF "/* Mouse definitions */"
/*
 * The following two static variables are for reserving space for the
 * parameters of a command. Space for temp_params is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_params is > length_temp_params, reallocate a larger area and
 * set the value of length_temp_params to reflect the new size.
 */
 static CHARTYPE *temp_params=NULL;
 static unsigned short length_temp_params=0;
/*
 * The following two static variables are for reserving space for the
 * directories in a macro path. Space for temp_macros is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_macros is > length_temp_macros  reallocate a larger area and
 * set the value of length_temp_macros to reflect the new size.
 */
 static CHARTYPE *temp_macros=NULL;
 static unsigned short length_temp_macros=0;
/*
 * The following two static variables are for reserving space for the
 * contents of   a command. Space for tmp_cmd     is allocated and
 * freed in the.c. If the size of the string to be placed into
 * tmp_cmd     is > length_tmp_cmd    , reallocate a larger area and
 * set the value of length_tmp_cmd     to reflect the new size.
 */
 static CHARTYPE *tmp_cmd=NULL;
 static unsigned short length_tmp_cmd=0;
/*
 * The following two        variables are for reserving space for the
 * contents of   a command. Space for temp_cmd    is allocated and
 * freed in the.c. If the size of the string to be placed into
 * temp_cmd    is > length_temp_cmd   , reallocate a larger area and
 * set the value of length_temp_cmd    to reflect the new size.
 */
 CHARTYPE *temp_cmd=NULL;
 static unsigned short length_temp_cmd=0;
/*
 * The following two are to specify the first and last items in the
 * linked list for key definitions.
 */
DEFINE *first_define=NULL;
DEFINE *last_define=NULL;
/*
 * The following two are to specify the first and last items in the
 * linked list for command synonyms.
 */
DEFINE *first_synonym=NULL;
DEFINE *last_synonym=NULL;
/*
 * The following two are to specify the first and last items in the
 * linked list for mouse button defintions.
 */
DEFINE *first_mouse_define=NULL;
DEFINE *last_mouse_define=NULL;

bool clear_command=TRUE;

/*
 * Key re-definition pseudo files.
 */
_LINE *key_first_line=NULL;
_LINE *key_last_line=NULL;
LINETYPE key_number_lines=0L;

AREAS _THE_FAR valid_areas[ATTR_MAX]=
{
    {(CHARTYPE *)"FILEAREA"    ,1,WINDOW_FILEAREA   ,TRUE },
    {(CHARTYPE *)"CURLINE"     ,2,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"BLOCK"       ,1,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"CBLOCK"      ,2,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"CMDLINE"     ,1,WINDOW_COMMAND    ,TRUE },
    {(CHARTYPE *)"IDLINE"      ,1,WINDOW_IDLINE     ,TRUE },
    {(CHARTYPE *)"MSGLINE"     ,1,WINDOW_ERROR      ,FALSE},
    {(CHARTYPE *)"ARROW"       ,1,WINDOW_ARROW      ,TRUE },
    {(CHARTYPE *)"PREFIX"      ,2,WINDOW_PREFIX     ,TRUE },
    {(CHARTYPE *)"CPREFIX"     ,3,WINDOW_PREFIX     ,TRUE },
    {(CHARTYPE *)"PENDING"     ,1,WINDOW_PREFIX     ,FALSE},
    {(CHARTYPE *)"SCALE"       ,1,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"TOFEOF"      ,2,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"CTOFEOF"     ,2,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"TABLINE"     ,1,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"SHADOW"      ,2,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"STATAREA"    ,2,WINDOW_STATAREA   ,TRUE },
    {(CHARTYPE *)"DIVIDER"     ,1,WINDOW_DIVIDER    ,TRUE },
    {(CHARTYPE *)"RESERVED"    ,1,WINDOW_RESERVED   ,FALSE},
    {(CHARTYPE *)"NONDISP"     ,1,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"HIGHLIGHT"   ,2,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"CHIGHLIGHT"  ,3,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"THIGHLIGHT"  ,5,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"SLK"         ,3,WINDOW_SLK        ,FALSE},
    {(CHARTYPE *)"GAP"         ,3,WINDOW_PREFIX     ,FALSE},
    {(CHARTYPE *)"CGAP"        ,4,WINDOW_PREFIX     ,FALSE},
    {(CHARTYPE *)"ALERT"       ,5,WINDOW_DIVIDER    ,FALSE},
    {(CHARTYPE *)"DIALOG"      ,6,WINDOW_DIVIDER    ,FALSE},
    {(CHARTYPE *)"BOUNDMARKER" ,5,WINDOW_FILEAREA   ,FALSE},
    {(CHARTYPE *)"FILETABS"    ,8,WINDOW_FILETABS   ,TRUE },
    {(CHARTYPE *)"FILETABSDIV" ,11,WINDOW_FILETABS   ,TRUE },
 };

/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *get_key_name(int key, int *shift)
#else
CHARTYPE *get_key_name(key,shift)
int key;
int *shift;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 CHARTYPE *keyname=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:get_key_name");
/*---------------------------------------------------------------------*/
/* Get name of key...                                                  */
/*---------------------------------------------------------------------*/
 *shift = 0;
 for (i=0;key_table[i].mnemonic!=NULL;i++)
    {
     if (key == key_table[i].key_value)
       {
        keyname = key_table[i].mnemonic;
        *shift = key_table[i].shift;
        break;
       }
    }
 TRACE_RETURN();
 return(keyname);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *get_key_definition(int key,bool define_format,bool default_keys,bool mouse_key)
#else
CHARTYPE *get_key_definition(key,define_format,default_keys,mouse_key)
int key;
bool define_format,default_keys,mouse_key;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 DEFINE *curr=NULL;
 bool check_redefined=TRUE;
 bool check_default=TRUE;
 CHARTYPE *keyname=NULL;
 CHARTYPE _THE_FAR key_buf[50];
 int dummy=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:get_key_definition");
/*---------------------------------------------------------------------*/
/* First determine if the key is a named key.                          */
/*---------------------------------------------------------------------*/
 if (mouse_key)
    keyname = mouse_key_number_to_name(key,key_buf);
 else
    keyname = get_key_name(key,&dummy);
/*---------------------------------------------------------------------*/
/* If key is invalid,  show it as a character and decimal; provided it */
/* is an ASCII or extended character.                                  */
/*---------------------------------------------------------------------*/
 if (keyname == NULL)
   {
    if (define_format)
       sprintf((DEFCHAR *)temp_cmd,"\"define \\%d ",key);
    else
       sprintf((DEFCHAR *)temp_cmd,"Key: \\%d",key);
   }
 else
   {
    if (define_format)
       strcpy((DEFCHAR *)temp_cmd,"\"define ");
    else
       strcpy((DEFCHAR *)temp_cmd,"Key: ");
    strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)keyname);
   }

 if (mouse_key)
    check_default = check_redefined = FALSE;
 else
   {
    if (define_format)
      {
       check_default = (default_keys) ? TRUE : FALSE;
       check_redefined = (default_keys) ? FALSE : TRUE;
      }
   }
/*---------------------------------------------------------------------*/
/* If we want to first check redefined keys...                         */
/*---------------------------------------------------------------------*/
 if (check_redefined)
   {
/*---------------------------------------------------------------------*/
/* Next check to see if the key has been "defined".                    */
/*---------------------------------------------------------------------*/
    curr = first_define;
    if (build_defined_key_definition(key,temp_cmd,curr,define_format) != (CHARTYPE *)NULL)
      {
       strcat((DEFCHAR *)temp_cmd,"\"");
       TRACE_RETURN();
       return(temp_cmd);
      }
   }
/*---------------------------------------------------------------------*/
/* If not, check for the default function key values.                  */
/*---------------------------------------------------------------------*/
 if (check_default)
   {
    for (i=0;command[i].text != NULL;i++)
       {
         if (key == command[i].funkey)
           {
            if (define_format)
               strcat((DEFCHAR *)temp_cmd," ");
            else
               strcat((DEFCHAR *)temp_cmd," - assigned to \"");
            build_default_key_definition(i,temp_cmd);
            strcat((DEFCHAR *)temp_cmd,"\"");
            TRACE_RETURN();
            return(temp_cmd);
           }
       }
   }
/*---------------------------------------------------------------------*/
/* If we want to check mouse key definitions.                          */
/*---------------------------------------------------------------------*/
 if (mouse_key)
   {
/*---------------------------------------------------------------------*/
/* Next check to see if the key has been "defined".                    */
/*---------------------------------------------------------------------*/
    curr = first_mouse_define;
    if (build_defined_key_definition(key,temp_cmd,curr,define_format) != (CHARTYPE *)NULL)
      {
       strcat((DEFCHAR *)temp_cmd,"\"");
       TRACE_RETURN();
       return(temp_cmd);
      }
   }
/*---------------------------------------------------------------------*/
/* If none of the above, it is unassigned. We should never get here if */
/* define_format is TRUE.                                              */
/*---------------------------------------------------------------------*/
 strcat((DEFCHAR *)temp_cmd," - unassigned");
 TRACE_RETURN();
 return(temp_cmd);
}

/***********************************************************************/
#ifdef HAVE_PROTO
static short execute_synonym( CHARTYPE *synonym )
#else
static short execute_synonym( synonym )
CHARTYPE *synonym;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   DEFINE *curr=(DEFINE *)NULL;
   short rc=RC_FILE_NOT_FOUND;
   CHARTYPE *key_cmd=NULL;
   short macrorc=0;
   int tokenised=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:execute_synonym");
/*---------------------------------------------------------------------*/
/* First check to see if the function key has been redefined and save  */
/* all key redefinitions.  This is because we may be redefining a      */
/* function key in the middle of its redefinition. eg ENTER key        */
/*---------------------------------------------------------------------*/
   curr = first_synonym;
   while(curr != (DEFINE *)NULL)
   {
      if ( equal( curr->synonym,synonym,curr->def_funkey ) )
      {
/*---------------------------------------------------------------------*/
/* If there are no more files in the ring, and the command is not a    */
/* command to edit a new file, then ignore the command.                */
/*---------------------------------------------------------------------*/
         if (number_of_files == 0
         &&  curr->def_command != (-1)
         &&  !command[curr->def_command].edit_command)
         {
            rc = RC_OK;
            curr = NULL;
            break;
         }
/*---------------------------------------------------------------------*/
/* If running in read-only mode and the function selected is not valid */
/* display an error.                                                   */
/*---------------------------------------------------------------------*/
         if (number_of_files != 0
         &&  ISREADONLY(CURRENT_FILE)
         &&  curr->def_command != (-1)
         &&  !command[curr->def_command].valid_in_readonly)
         {
            display_error(56,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_ENVIRON;
            curr = NULL;
            break;
         }
         if ((key_cmd = (CHARTYPE *)my_strdup(curr->def_params)) == NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            rc = RC_OUT_OF_MEMORY;
            curr = NULL;
            break;
         }
         if (curr->def_command == (-1))
         {
            rc = execute_macro_instore(key_cmd,&macrorc,&curr->pcode,&curr->pcode_len,&tokenised,curr->def_funkey);
            if (tokenised)
            {
               tokenised = 0;
            }
            if (number_of_files > 0)
            {
               if (display_screens > 1)
               {
                  build_screen(other_screen);
                  display_screen(other_screen);
               }
               build_screen(current_screen);
               display_screen(current_screen);
            }
         }
         else
         {
            /*
             * If operating in CUA mode, and a CUA block exists, check
             * if the block should be reset or deleted before executing
             * the command.
             */
            if ( INTERFACEx == INTERFACE_CUA
            &&  MARK_VIEW == CURRENT_VIEW
            &&  MARK_VIEW->mark_type == M_CUA )
            {
               ResetOrDeleteCUABlock( command[curr->def_command].cua_behaviour );
            }
            /*
             * Possibly reset the THIGHLIGHT area
             */
            if ( CURRENT_VIEW->thighlight_on
            &&   CURRENT_VIEW->thighlight_active )
            {
               AdjustThighlight( command[curr->def_command].thighlight_behaviour );
            }
            rc = (*command[curr->def_command].function)((CHARTYPE *)key_cmd);
         }
         (*the_free)(key_cmd);
      }
      curr = curr->next;
   }
   TRACE_RETURN();
   return rc;
}

/***********************************************************************/
#ifdef HAVE_PROTO
short function_key(int key,int option,bool mouse_details_present)
#else
short function_key(key,option,mouse_details_present)
int key;
int option;
bool mouse_details_present;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
   DEFINE *curr=(DEFINE *)NULL;
   DEFINE *tcurr=(DEFINE *)NULL;
   DEFINE *first_save=(DEFINE *)NULL,*last_save=(DEFINE *)NULL;
   short rc=RC_OK;
   short len=0,num_cmds=0;
   CHARTYPE *key_cmd=NULL;
   CHARTYPE tmpnum[5];
   short macrorc=0;
   int tokenised=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:function_key");
   if (mouse_details_present)
   {
      rc = execute_mouse_commands(key);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * First check to see if the function key has been redefined and save
    * all key redefinitions.  This is because we may be redefining a
    * function key in the middle of its redefinition. eg ENTER key
    */
   curr = first_define;
   while(curr != (DEFINE *)NULL)
   {
      if (key == curr->def_funkey)
      {
         rc = append_define(&first_save,&last_save,key,curr->def_command,curr->def_params,curr->pcode,curr->pcode_len,NULL,0);
         if (rc != RC_OK)
         {
            TRACE_RETURN();
            return(rc);
         }
      }
      curr = curr->next;
   }
   /*
    * Now we have saved any current definition of the function key, use
    * these definitions to drive the following...
    */
   curr = first_save;
   while(curr != (DEFINE *)NULL)
   {
      switch(option)
      {
         case OPTION_NORMAL:
            /*
             * If there are no more files in the ring, and the command is not a
             * command to edit a new file, then ignore the command.
             */
            if (number_of_files == 0
            &&  curr->def_command != (-1)
            &&  !command[curr->def_command].edit_command)
            {
               rc = RC_OK;
               curr = NULL;
               break;
            }
            /*
             * If running in read-only mode and the function selected is not valid
             * display an error.
             */
            if (number_of_files != 0
            &&  ISREADONLY(CURRENT_FILE)
            &&  curr->def_command != (-1)
            &&  !command[curr->def_command].valid_in_readonly)
            {
               display_error(56,(CHARTYPE *)"",FALSE);
               rc = RC_INVALID_ENVIRON;
               curr = NULL;
               break;
            }
            if ((key_cmd = (CHARTYPE *)my_strdup(curr->def_params)) == NULL)
            {
               display_error(30,(CHARTYPE *)"",FALSE);
               rc = RC_OUT_OF_MEMORY;
               curr = NULL;
               break;
            }
            if (curr->def_command == (-1))
            {
               rc = execute_macro_instore(key_cmd,&macrorc,&curr->pcode,&curr->pcode_len,&tokenised,curr->def_funkey);
               if (tokenised)
               {
                  tokenised = 0;
                  tcurr = first_define;
                  while(tcurr != (DEFINE *)NULL)
                  {
                     if (key == tcurr->def_funkey)
                     {
                        /*
                         * The pcode returned from the interpreter needs
                         * to be transfered from the copy of the key
                         * definition (curr and first_save to the "real"
                         * key definition found here.  Setting the
                         * pcode pointer in tcurr to the pcode in curr
                         * and then setting pcode in curr to NULL stops
                         * the later call to dll_free() from freeing
                         * the memory associated with pcode.
                         */
                        tcurr->pcode = curr->pcode;
                        tcurr->pcode_len = curr->pcode_len;
                        curr->pcode = NULL;
                        break;
                     }
                     tcurr = tcurr->next;
                  }
               }
               if (number_of_files > 0)
               {
                  if (display_screens > 1)
                  {
                     build_screen(other_screen);
                     display_screen(other_screen);
                  }
                  build_screen(current_screen);
                  display_screen(current_screen);
               }
            }
            else
            {
               /*
                * If operating in CUA mode, and a CUA block exists, check
                * if the block should be reset or deleted before executing
                * the command.
                */
               if ( INTERFACEx == INTERFACE_CUA
               &&  MARK_VIEW == CURRENT_VIEW
               &&  MARK_VIEW->mark_type == M_CUA )
               {
                  ResetOrDeleteCUABlock( command[curr->def_command].cua_behaviour );
               }
               /*
                * Possibly reset the THIGHLIGHT area
                */
               if ( CURRENT_VIEW->thighlight_on
               &&   CURRENT_VIEW->thighlight_active )
               {
                  AdjustThighlight( command[curr->def_command].thighlight_behaviour );
               }
               rc = (*command[curr->def_command].function)((CHARTYPE *)key_cmd);
            }
            (*the_free)(key_cmd);
            if (rc != RC_OK
            &&  rc != RC_TOF_EOF_REACHED
            &&  rc != RC_NO_LINES_CHANGED
            &&  rc != RC_TARGET_NOT_FOUND)
            {
               curr = NULL;
               break;
            }
            break;
         case OPTION_EXTRACT:
            /*
             * If the request is to extract a keys commands, set a REXX variable
             * for each command associated with the function key.
             */
            if (curr->def_command != (-1))
            {
               len = strlen((DEFCHAR *)command[curr->def_command].text) +
                     strlen((DEFCHAR *)curr->def_params) + 2;
            }
            else
            {
               len = strlen((DEFCHAR *)curr->def_params) + 1;
            }
            if ((key_cmd = (CHARTYPE *)(*the_malloc)(len)) == NULL)
            {
               display_error(30,(CHARTYPE *)"",FALSE);
               curr = NULL;
               rc = RC_OUT_OF_MEMORY;
               break;
            }
            if (curr->def_command != (-1))
            {
               strcpy((DEFCHAR *)key_cmd,(DEFCHAR *)command[curr->def_command].text);
               strcat((DEFCHAR *)key_cmd," ");
               strcat((DEFCHAR *)key_cmd,(DEFCHAR *)curr->def_params);
            }
            else
               strcpy((DEFCHAR *)key_cmd,(DEFCHAR *)curr->def_params);
            rc = set_rexx_variable((CHARTYPE *)"SHOWKEY",key_cmd,strlen((DEFCHAR *)key_cmd),++num_cmds);
            (*the_free)(key_cmd);
            break;
         case OPTION_READV:
            /*
             * If the key hit is KEY_ENTER, KEY_RETURN or KEY_NUMENTER, terminate
             * the READV CMDLINE command.
             */
            if (key == KEY_RETURN
            ||  key == KEY_ENTER
            ||  key == KEY_C_m
            ||  key == KEY_NUMENTER)
            {
               rc = RC_READV_TERM;
               curr = NULL;
               break;
            }
            /*
             * If the command is not allowed in READV CMDLINE, or a REXX macro is
             * assigned to the key, return with an error.
             */
            if (curr->def_command == (-1))
            {
               rc = RC_INVALID_ENVIRON;
               curr = NULL;
               break;
            }
            if (!command[curr->def_command].valid_readv)
            {
               rc = RC_INVALID_ENVIRON;
               curr = NULL;
               break;
            }
            /*
             * To get here, a valid READV CMDLINE command is present; execute it.
             */
            if ((key_cmd = (CHARTYPE *)my_strdup(curr->def_params)) == NULL)
            {
               display_error(30,(CHARTYPE *)"",FALSE);
               rc = RC_OUT_OF_MEMORY;
               curr = NULL;
               break;
            }
            rc = (*command[curr->def_command].function)((CHARTYPE *)key_cmd);
            (*the_free)(key_cmd);
            if (rc != RC_OK
            &&  rc != RC_TOF_EOF_REACHED
            &&  rc != RC_NO_LINES_CHANGED
            &&  rc != RC_TARGET_NOT_FOUND)
            {
               curr = NULL;
               break;
            }
            break;
         default:
           break;
      }
      if (curr == NULL)
         break;
      curr = curr->next;
   }
   /*
    * If the key has been redefined, exit here...
    */
   if (first_save)
   {
      if (option == OPTION_EXTRACT)
      {
         sprintf((DEFCHAR *)tmpnum,"%d",num_cmds);
         rc = set_rexx_variable((CHARTYPE *)"SHOWKEY",tmpnum,strlen((DEFCHAR *)tmpnum),0);
         rc = num_cmds;
      }
      dll_free(first_save);
      TRACE_RETURN();
      return(rc);
   }
   /*
    * If not, check for the default function key values.
    */
   for (i=0;command[i].text != NULL;i++)
   {
      if (key == command[i].funkey)
      {
         switch(option)
         {
            case OPTION_NORMAL:
               /*
                * If running in read-only mode and the function selected is not valid
                * display an error.
                */
               if (number_of_files != 0
               &&  ISREADONLY(CURRENT_FILE)
               && !command[i].valid_in_readonly)
               {
                  display_error(56,(CHARTYPE *)"",FALSE);
                  rc = RC_INVALID_ENVIRON;
                  break;
               }
               if ((key_cmd = (CHARTYPE *)my_strdup(command[i].params)) == NULL)
               {
                  display_error(30,(CHARTYPE *)"",FALSE);
                  rc = RC_OUT_OF_MEMORY;
                  break;
               }
               /*
                * If operating in CUA mode, and a CUA block exists, check
                * if the block should be reset or deleted before executing
                * the command.
                */
               if ( INTERFACEx == INTERFACE_CUA
               &&  MARK_VIEW == CURRENT_VIEW
               &&  MARK_VIEW->mark_type == M_CUA )
               {
                  ResetOrDeleteCUABlock( command[i].cua_behaviour );
               }
               /*
                * Possibly reset the THIGHLIGHT area
                */
               if ( CURRENT_VIEW->thighlight_on
               &&   CURRENT_VIEW->thighlight_active )
               {
                  AdjustThighlight( command[i].thighlight_behaviour );
               }
               rc = (*command[i].function)((CHARTYPE *)key_cmd);
               (*the_free)(key_cmd);
               break;
            case OPTION_EXTRACT:
               len = strlen((DEFCHAR *)command[i].text) +
                     strlen((DEFCHAR *)command[i].params) + 10;
               if ((key_cmd = (CHARTYPE *)(*the_malloc)(len)) == NULL)
               {
                  display_error(30,(CHARTYPE *)"",FALSE);
                  rc = RC_OUT_OF_MEMORY;
                  break;
               }
               strcpy((DEFCHAR *)key_cmd,"");
               key_cmd = build_default_key_definition(i,key_cmd);
               rc = set_rexx_variable((CHARTYPE *)"SHOWKEY",key_cmd,strlen((DEFCHAR *)key_cmd),1);
               (*the_free)(key_cmd);
               rc = set_rexx_variable((CHARTYPE *)"SHOWKEY",(CHARTYPE *)"1",1,0);
               break;
            case OPTION_READV:
                  /*
                   * If the key hit is KEY_ENTER, KEY_RETURN or KEY_NUMENTER, terminate
                   * the READV CMDLINE command.
                   */
                  if (key == KEY_RETURN
                  ||  key == KEY_ENTER
                  ||  key == KEY_C_m
                  ||  key == KEY_NUMENTER)
                  {
                     rc = RC_READV_TERM;
                     break;
                  }
                  /*
                   * If the command is not allowed in READV CMDLINE, return with an error
                   */
                  if (!command[i].valid_readv)
                  {
                     rc = RC_INVALID_ENVIRON;
                     break;
                  }
                  /*
                   * To get here, a valid READV CMDLINE command is present; execute it.
                   */
                  if ((key_cmd = (CHARTYPE *)my_strdup(command[i].params)) == NULL)
                  {
                     display_error(30,(CHARTYPE *)"",FALSE);
                     rc = RC_OUT_OF_MEMORY;
                     break;
                  }
                  rc = (*command[i].function)((CHARTYPE *)key_cmd);
                  (*the_free)(key_cmd);
                  break;
         }
         TRACE_RETURN();
         return(rc);
      }
   }
   if (option == OPTION_EXTRACT)
      rc = set_rexx_variable((CHARTYPE *)"SHOWKEY",(CHARTYPE *)"0",1,0);
   TRACE_RETURN();
   return(RAW_KEY);
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool is_modifier_key(int key)
#else
bool is_modifier_key(key)
int key;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:is_modifier_key");
/*---------------------------------------------------------------------*/
/* Get name of key...                                                  */
/*---------------------------------------------------------------------*/
   for ( i = 0; key_table[i].mnemonic != NULL; i++ )
   {
      if ( key == key_table[i].key_value
      &&   key_table[i].shift == SHIFT_MODIFIER_ONLY )
      {
         TRACE_RETURN();
         return TRUE;
      }
   }
   TRACE_RETURN();
   return(FALSE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *build_default_key_definition(int key, CHARTYPE *buf)
#else
CHARTYPE *build_default_key_definition(key, buf)
int key;
CHARTYPE *buf;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:build_default_key_definition");
/*---------------------------------------------------------------------*/
/* The argument, buf, MUST be long enough to to accept the full command*/
/* and arguments and MUST have be nul terminated before this function  */
/* is called.                                                          */
/*---------------------------------------------------------------------*/
/* If a SET command, prefix with 'set'                                 */
/*---------------------------------------------------------------------*/
 if (command[key].set_command)
    strcat((DEFCHAR *)buf,"set ");
/*---------------------------------------------------------------------*/
/* If a SOS command, prefix with 'sos'                                 */
/*---------------------------------------------------------------------*/
 if (command[key].sos_command)
    strcat((DEFCHAR *)buf,"sos ");
/*---------------------------------------------------------------------*/
/* Append the command name.                                            */
/*---------------------------------------------------------------------*/
 strcat((DEFCHAR *)buf,(DEFCHAR *)command[key].text);
/*---------------------------------------------------------------------*/
/* Append any parameters.                                              */
/*---------------------------------------------------------------------*/
 if (strcmp((DEFCHAR *)command[key].params,"") != 0)
   {
    strcat((DEFCHAR *)buf," ");
    strcat((DEFCHAR *)buf,(DEFCHAR *)command[key].params);
   }
 TRACE_RETURN();
 return(buf);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static CHARTYPE *build_defined_key_definition(int key, CHARTYPE *buf,DEFINE *curr,bool define_format)
#else
static CHARTYPE *build_defined_key_definition(key, buf,curr,define_format)
int key;
CHARTYPE *buf;
DEFINE *curr;
bool define_format;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 bool key_defined=FALSE;
 bool first_time=TRUE;
 CHARTYPE delim[2];
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:build_defined_key_definition");
/*---------------------------------------------------------------------*/
/* The argument, buf, MUST be long enough to to accept the full command*/
/* and arguments and MUST have be nul terminated before this function  */
/* is called.                                                          */
/*---------------------------------------------------------------------*/
 delim[1] = '\0';
 delim[0] = CURRENT_VIEW->linend_value;
 while(curr != NULL)
  {
   if (key == curr->def_funkey)
     {
      key_defined = TRUE;
      if (first_time)
        {
         if (define_format)
            strcat((DEFCHAR *)buf," ");
         else
            strcat((DEFCHAR *)buf," - assigned to \"");
        }
      else
        {
         strcat((DEFCHAR *)buf,(DEFCHAR *)delim);
        }
/*---------------------------------------------------------------------*/
/* Append the command to the string.                                   */
/*---------------------------------------------------------------------*/
      if (curr->def_command == (-1))  /* definition is REXX instore */
         strcat((DEFCHAR *)buf,(DEFCHAR *)"REXX");
      else
         strcat((DEFCHAR *)buf,(DEFCHAR *)command[curr->def_command].text);
/*---------------------------------------------------------------------*/
/* Append any parameters.                                              */
/*---------------------------------------------------------------------*/
      if (strcmp((DEFCHAR *)curr->def_params,"") != 0)
        {
         strcat((DEFCHAR *)buf," ");
         strcat((DEFCHAR *)buf,(DEFCHAR *)curr->def_params);
        }
      first_time = FALSE;
     }
   curr = curr->next;
  }
 TRACE_RETURN();
 return((key_defined)?buf:(CHARTYPE *)NULL);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *build_synonym_definition( CHARTYPE *synonym, CHARTYPE *buf, DEFINE *curr )
#else
CHARTYPE *build_synonym_definition( synonym, buf, curr )
CHARTYPE *synonym,*buf;
DEFINE *curr;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   bool synonym_defined=FALSE;
   bool first_time=TRUE;
   CHARTYPE delim[2];
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:build_synonym_definition");
/*---------------------------------------------------------------------*/
/* The argument, buf, MUST be long enough to to accept the full command*/
/* and arguments and MUST have be nul terminated before this function  */
/* is called.                                                          */
/*---------------------------------------------------------------------*/
   delim[1] = '\0';
   while( curr != NULL )
   {
      if ( equal( curr->synonym, synonym,curr->def_funkey ) )
      {
         synonym_defined = TRUE;
         if ( first_time )
         {
            if ( curr->linend )
               delim[0] = curr->linend;
            else
               delim[0] = CURRENT_VIEW->linend_value;
         }
         else
         {
            strcat( (DEFCHAR *)buf, (DEFCHAR *)delim );
         }
         /*
          * Append the command to the string.
          */
         if ( curr->def_command == (-1) ) /* definition is REXX instore */
            strcat( (DEFCHAR *)buf, (DEFCHAR *)"REXX" );
         else
            strcat( (DEFCHAR *)buf, (DEFCHAR *)command[curr->def_command].text );
         /*
          * Append any parameters.
          */
         if (strcmp( (DEFCHAR *)curr->def_params, "" ) != 0 )
         {
            strcat( (DEFCHAR *)buf, " " );
            strcat( (DEFCHAR *)buf, (DEFCHAR *)curr->def_params );
         }
         first_time = FALSE;
      }
      curr = curr->next;
   }
   TRACE_RETURN();
   return( (synonym_defined) ? buf : (CHARTYPE *)NULL );
}
/***********************************************************************/
#ifdef HAVE_PROTO
short display_all_keys(void)
#else
short display_all_keys()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 _LINE *curr=NULL;
 DEFINE *curr_define=NULL;
 int key=0,save_funkey=0;
 short rc=RC_OK;
 register int i=0;
 CHARTYPE *keydef=NULL;
 VIEW_DETAILS *found_view=NULL;
 static bool first=TRUE;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:display_all_keys");
/*---------------------------------------------------------------------*/
/* If this is the first time we've called this, create the pseudo file */
/* name.                                                               */
/*---------------------------------------------------------------------*/
 if (first)
 {
    strcpy((DEFCHAR *)key_pathname,(DEFCHAR *)dir_pathname);
    strcat((DEFCHAR *)key_pathname,(DEFCHAR *)keyfilename);
    if ((rc = splitpath(key_pathname)) != RC_OK)
    {
       TRACE_RETURN();
       return(rc);
    }
    strcpy((DEFCHAR *)key_pathname,(DEFCHAR *)sp_path);
    strcpy((DEFCHAR *)key_filename,(DEFCHAR *)sp_fname);
    first = FALSE;
 }
#if !defined(MULTIPLE_PSEUDO_FILES)
 key_first_line = key_last_line = lll_free(key_first_line);
 key_number_lines = 0L;
 if ((found_view = find_file(key_pathname,key_filename)) != (VIEW_DETAILS *)NULL)
 {
    found_view->file_for_view->first_line = found_view->file_for_view->last_line = NULL;
    found_view->file_for_view->number_lines = 0L;
 }
#endif
/*---------------------------------------------------------------------*/
/* first_line is set to "Top of File"                                  */
/*---------------------------------------------------------------------*/
 if ((key_first_line = add_LINE(key_first_line,NULL,TOP_OF_FILE,
     strlen((DEFCHAR *)TOP_OF_FILE),0,FALSE)) == NULL)
 {
    TRACE_RETURN();
    return(RC_OUT_OF_MEMORY);
 }
/*---------------------------------------------------------------------*/
/* last line is set to "Bottom of File"                                */
/*---------------------------------------------------------------------*/
 if ((key_last_line = add_LINE(key_first_line,key_first_line,BOTTOM_OF_FILE,
    strlen((DEFCHAR *)BOTTOM_OF_FILE),0,FALSE)) == NULL)
 {
    TRACE_RETURN();
    return(RC_OUT_OF_MEMORY);
 }
 curr = key_first_line;
/*---------------------------------------------------------------------*/
/* First display default key mappings...                               */
/*---------------------------------------------------------------------*/
 if ((curr = add_LINE(key_first_line,curr,(CHARTYPE *)KEY_DEFAULT,strlen(KEY_DEFAULT),0,FALSE)) == NULL)
 {
    TRACE_RETURN();
    return(RC_OUT_OF_MEMORY);
 }
 key_number_lines++;
 save_funkey = (-1);
 for (i=0;command[i].text != NULL;i++)
 {
    if (command[i].funkey != (-1)
    &&  save_funkey != command[i].funkey)
    {
       save_funkey = command[i].funkey;
       keydef = get_key_definition(command[i].funkey,TRUE,TRUE,FALSE);
       if ((curr = add_LINE(key_first_line,curr,keydef,strlen((DEFCHAR *)keydef),0,FALSE)) == NULL)
       {
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
       }
       key_number_lines++;
    }
 }
/*---------------------------------------------------------------------*/
/* ...next, display any key redefinitions.                             */
/*---------------------------------------------------------------------*/
 if ((curr = add_LINE(key_first_line,curr,(CHARTYPE *)KEY_REDEF,strlen(KEY_REDEF),0,FALSE)) == NULL)
 {
    TRACE_RETURN();
    return(RC_OUT_OF_MEMORY);
 }
 key_number_lines++;
 curr_define = first_define;
 while(curr_define != NULL)
 {
    if (key != curr_define->def_funkey)
    {
       keydef = get_key_definition(curr_define->def_funkey,TRUE,FALSE,FALSE);
       if ((curr = add_LINE(key_first_line,curr,keydef,strlen((DEFCHAR *)keydef),0,FALSE)) == NULL)
       {
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
       }
       key_number_lines++;
    }
    key = curr_define->def_funkey;
    curr_define = curr_define->next;
 }
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
/*---------------------------------------------------------------------*/
/* ...last, display any mouse key definitions.                         */
/*---------------------------------------------------------------------*/
 if ((curr = add_LINE(key_first_line,curr,(CHARTYPE *)KEY_MOUSE_REDEF,strlen(KEY_MOUSE_REDEF),0,FALSE)) == NULL)
 {
    TRACE_RETURN();
    return(RC_OUT_OF_MEMORY);
 }
 key_number_lines++;
 curr_define = first_mouse_define;
 while(curr_define != NULL)
 {
    if (key != curr_define->def_funkey)
    {
       keydef = get_key_definition(curr_define->def_funkey,TRUE,FALSE,TRUE);
       if ((curr = add_LINE(key_first_line,curr,keydef,strlen((DEFCHAR *)keydef),0,FALSE)) == NULL)
       {
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
       }
       key_number_lines++;
    }
    key = curr_define->def_funkey;
    curr_define = curr_define->next;
 }
#endif
#if defined(MULTIPLE_PSEUDO_FILES)
 Xedit((CHARTYPE *)"***KEY***");
#else
 strcpy((DEFCHAR *)temp_cmd,(DEFCHAR *)key_pathname);
 strcat((DEFCHAR *)temp_cmd,(DEFCHAR *)key_filename);
 Xedit(temp_cmd);
#endif
 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short command_line(CHARTYPE *cmd_line,bool command_only)
#else
short command_line(cmd_line,command_only)
CHARTYPE *cmd_line;
bool command_only;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   bool valid_command=FALSE;
   bool linend_status=(number_of_files) ? CURRENT_VIEW->linend_status : LINEND_STATUSx;
   CHARTYPE linend_value=0;
   register short i=0,j=0;
   short rc=RC_OK;
   CHARTYPE *cmd[MAX_COMMANDS+1];
   unsigned short num_commands=0;
   CHARTYPE command_delim[2];
   CHARTYPE *command_entered=NULL;
   CHARTYPE *cl_cmd=NULL;
   CHARTYPE *cl_param=NULL;
   short macrorc=0;
   bool display_parse_error;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:command_line");
#ifdef THE_TRACE
   trace_string( "cmd_line: [%s] command_only: %d\n", cmd_line, command_only );
#endif
   /*
    * If the command line is blank, just return.
    */
   if (blank_field(cmd_line))
   {
      if (curses_started)
         wmove(CURRENT_WINDOW_COMMAND,0,0);
      TRACE_RETURN();
      return(RC_OK);
   }
   /*
    * Set up values for LINEND for later processing...
    */
   if (number_of_files == 0)
   {
      linend_status = LINEND_STATUSx;
      linend_value = LINEND_VALUEx;
   }
   else
   {
      linend_status = CURRENT_VIEW->linend_status;
      linend_value = CURRENT_VIEW->linend_value;
   }
   /*
    * If the command is to be kept displayed on the command line...
    */
   if (*(cmd_line) == '&')
   {
      cmd_line++;
      clear_command = FALSE;
   }
   else
   {
      if (!(in_macro && !clear_command))
         clear_command = TRUE;
   }
   /*
    * Copy the incoming cmd_line, so we can play with it.
    */
   if ((command_entered = (CHARTYPE *)my_strdup(cmd_line)) == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   /*
    * Allocate some space to cl_cmd and cl_param for the a command when
    * it is split into a command and its parameters.
    */
   if ((cl_cmd = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)cmd_line)+1)*sizeof(CHARTYPE))) == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if ((cl_param = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)cmd_line)+1)*sizeof(CHARTYPE))) == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   /*
    * If [SET] LINENd is set to ON, split the line up into a number of
    * individual commands.
    */
   if (linend_status
   && !define_command(cmd_line))
   {
      command_delim[0] = linend_value;
      command_delim[1] = '\0';
      num_commands = command_split(cmd_line,cmd,MAX_COMMANDS,command_delim,command_entered);
   }
   else
   {
      cmd[0] = command_entered;
      num_commands = 1;
   }
   /*
    * For each command entered, split it up into command and params, and
    * process it...
    */
   for (j=0;j<num_commands;j++)
   {
      valid_command = FALSE;
      split_command(cmd[j],cl_cmd,cl_param);
      cl_cmd = MyStrip(cl_cmd,STRIP_BOTH,' ');
      /*
       * Check for synonyms first.
       */
      if ( !command_only
      &&   CURRENT_VIEW
      &&   CURRENT_VIEW->synonym )
      {
         rc = execute_synonym(cl_cmd);
         if (rc != RC_FILE_NOT_FOUND)
         {
            lastrc = (rc==RC_SYSTEM_ERROR)?rc:macrorc;
            save_last_command(cmd[j],cl_cmd);
            continue;
         }
      }
      /*
       * Before we try the command array, if SET MACRO is ON and IMPMACRO is
       * ON and we have not reached here via COMMAND command, try for a
       * macro...
       */
      if (number_of_files > 0
      &&  CURRENT_VIEW->macro
      &&  CURRENT_VIEW->imp_macro
      && !command_only)
      {
         strcpy((DEFCHAR *)command_entered,(DEFCHAR *)cmd[j]);
         rc = execute_macro(command_entered,FALSE,&macrorc);
         if (rc != RC_FILE_NOT_FOUND)
         {
            lastrc = (rc==RC_SYSTEM_ERROR)?rc:macrorc;
            save_last_command(cmd[j],cl_cmd);
            continue;
         }
      }
      /*
       * Look up the command in the command array in command.h
       */
      for (i=0;command[i].text != NULL;i++)
      {
         /*
          * If no command text, continue.
          */
         if (strcmp((DEFCHAR *)command[i].text,"") == 0)
            continue;
         rc = RC_OK;
         /*
          * Check that the supplied command matches the command for the length
          * of the command and that the length is at least as long as the
          * necessary significance.
          */
         if (equal(command[i].text,cl_cmd,command[i].min_len)
         && command[i].min_len != 0
         && !command[i].sos_command)
         {
            if (batch_only
            && !command[i].valid_batch_command)
            {
               display_error(24,command[i].text,FALSE);
               lastrc = rc = RC_INVALID_ENVIRON;
               break;
            }
            valid_command = TRUE;
            /*
             * Here is a big kludge. Because only a few commands need leading
             * spaces to be present in temp_params and all other commands barf at
             * leading spaces, we need to left truncate temp_params for most
             * commands.
             */
            if (command[i].strip_param)
               cl_param = MyStrip(cl_param,command[i].strip_param,' ');
            /*
             * If we are currently processing the profile file as a result of
             * reprofile, ignore those commands that are invalid.
             */
            if (profile_file_executions > 1
            &&  in_reprofile
            &&  !command[i].valid_for_reprofile)
            {
               rc = RC_OK;
               break;
            }
            /*
             * If there are no more files in the ring, and the command is not a
             * command to edit a new file, then ignore the command.
             */
            if (number_of_files == 0
            &&  !command[i].edit_command)
            {
               rc = RC_OK;
               break;
            }
            /*
             * If running in read-only mode and the function selected is not valid
             * display an error.
             */
            if (number_of_files != 0
            &&  ISREADONLY(CURRENT_FILE)
            && !command[i].valid_in_readonly)
            {
               display_error(56,(CHARTYPE *)"",FALSE);
               rc = RC_INVALID_ENVIRON;
               break;
            }
            /*
             * If operating in CUA mode, and a CUA block exists, check
             * if the block should be reset or deleted before executing
             * the command.
             */
            if ( INTERFACEx == INTERFACE_CUA
            &&  MARK_VIEW == CURRENT_VIEW
            &&  MARK_VIEW->mark_type == M_CUA )
            {
               ResetOrDeleteCUABlock( command[i].cua_behaviour );
            }
            /*
             * Possibly reset the THIGHLIGHT area
             */
            if ( CURRENT_VIEW
            &&   CURRENT_VIEW->thighlight_on
            &&   CURRENT_VIEW->thighlight_active )
            {
               AdjustThighlight( command[i].thighlight_behaviour );
            }
            /*
             * Now call the function associated with the supplied command string
             * and the possibly stripped parameters.
             */
            lastrc = rc = (*command[i].function)(cl_param);
            break;
         }
      }
      /*
       * If an error occurred while executing a command above, break.
       */
      if (rc != RC_OK
      &&  rc != RC_TOF_EOF_REACHED)
         break;
      /*
       * If we found and successfully executed a command above, process the
       * next command.
       */
      if (valid_command)
      {
         save_last_command(cmd[j],cl_cmd);
         continue;
      }
      /*
       * If there are no more files in the ring, then ignore the command.
       */
      if (number_of_files == 0)
      {
         save_last_command(cmd[j],cl_cmd);
         continue;
      }
      /*
       * To get here the command was not a 'command'; check if a valid target
       */
      if (!CURRENT_VIEW->imp_macro
      &&  !CURRENT_VIEW->imp_os)
         display_parse_error = TRUE;
      else
         display_parse_error = FALSE;
      rc = execute_locate( cmd[j], display_parse_error );
      if ( rc == RC_OK
      ||   rc == RC_TOF_EOF_REACHED
      ||   rc == RC_TARGET_NOT_FOUND )
      {
         lastrc = rc;
         save_last_command( cmd[j], cl_cmd );
         continue;
      }



      /*
       * If return is RC_INVALID_OPERAND, check if command is OS command...
       */
      if (cmd[j][0] == '!')
      {
         strcpy((DEFCHAR *)command_entered,(DEFCHAR *)cmd[j]);
         lastrc = rc = Os(command_entered+1);
         save_last_command(cmd[j],cl_cmd);
         continue;
      }
      /*
       * ...or if command is a macro command (as long as IMPMACRO is ON) and
       * command_only is FALSE...
       */
      if (CURRENT_VIEW->imp_macro
      && !command_only)
      {
#ifdef __BOUNDS_CHECKING_ON
         memmove(command_entered,cmd[j],strlen(cmd[j]) + 1);
#else
         strcpy((DEFCHAR *)command_entered,(DEFCHAR *)cmd[j]);
#endif
         if (CURRENT_VIEW->imp_os)
         {
            rc = execute_macro(command_entered,FALSE,&macrorc);
            if (rc != RC_FILE_NOT_FOUND)
            {
               lastrc = (rc==RC_SYSTEM_ERROR)?rc:macrorc;
               save_last_command(cmd[j],cl_cmd);
               continue;
            }
         }
         else
         {
            rc = execute_macro(command_entered,TRUE,&macrorc);
            if (rc == RC_FILE_NOT_FOUND)
            {
               lastrc = rc = RC_NOT_COMMAND;
               break;
            }
            else
            {
               lastrc = (rc==RC_SYSTEM_ERROR)?rc:macrorc;
               save_last_command(cmd[j],cl_cmd);
               continue;
            }
         }
      }
      /*
       * ...or if command is an OS command (as long as IMPOS is ON).
       */
      if (CURRENT_VIEW->imp_os)
      {
         error_on_screen = FALSE;
         strcpy((DEFCHAR *)command_entered,(DEFCHAR *)cmd[j]);
         rc = Os(command_entered);
      }
      else
      {
         display_error(21,cmd[j],FALSE);
         rc = RC_NOT_COMMAND;
      }
      /*
       * If the 'command' is not a command then do not process any more.
       */
      lastrc = rc;
      if (rc == RC_NOT_COMMAND)
         break;
      save_last_command(cmd[j],cl_cmd);
   }
   cleanup_command_line();
   (*the_free)(command_entered);
   (*the_free)(cl_cmd);
   (*the_free)(cl_param);

   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void cleanup_command_line(void)
#else
void cleanup_command_line()
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("commutil.c:cleanup_command_line");
   if (!curses_started || in_macro || number_of_views == 0)
   {
      TRACE_RETURN();
      return;
   }
   if (clear_command)
   {
      if (CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
      {
         wmove(CURRENT_WINDOW_COMMAND,0,0);
         my_wclrtoeol(CURRENT_WINDOW_COMMAND);
      }
      memset(cmd_rec,' ',max_line_length);
      cmd_rec_len = 0;
   }
   if (CURRENT_WINDOW_COMMAND != (WINDOW *)NULL)
   {
      if (CURRENT_VIEW->cmdline_col == (-1))
         wmove(CURRENT_WINDOW_COMMAND,0,cmd_rec_len);
      else
         wmove(CURRENT_WINDOW_COMMAND,0,CURRENT_VIEW->cmdline_col);
   }
   CURRENT_VIEW->cmdline_col = (-1);
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void split_command(CHARTYPE *cmd_line,CHARTYPE *cmd,CHARTYPE *param)
#else
void split_command(cmd_line,cmd,param)
CHARTYPE *cmd_line,*cmd,*param;
#endif
/***********************************************************************/
{
   short pos=0;
   CHARTYPE *param_ptr=NULL;

   TRACE_FUNCTION("commutil.c:split_command");
   strcpy((DEFCHAR *)cmd,(DEFCHAR *)cmd_line);
   MyStrip(cmd,STRIP_LEADING,' ');
   /*
    * Special test here for ? and = command...
    */
   if (*cmd == '?')
   {
      strcpy((DEFCHAR *)param,(DEFCHAR *)(cmd+1));
      strcpy((DEFCHAR *)cmd,"?");
      TRACE_RETURN();
      return;
   }
   if (*cmd == '=')
   {
      strcpy((DEFCHAR *)param,(DEFCHAR *)(cmd+1));
      strcpy((DEFCHAR *)cmd,"=");
      TRACE_RETURN();
      return;
   }
   for (param_ptr=cmd;*param_ptr!='\0';param_ptr++)
   {
      if (!isalpha(*param_ptr))
         break;
   }
   if (!param_ptr)
   {
      strcpy((DEFCHAR *)param,"");
      TRACE_RETURN();
      return;
   }
   if (param_ptr == cmd
   ||  *param_ptr == '\0')
   {
      strcpy((DEFCHAR *)param,"");
      TRACE_RETURN();
      return;
   }
   pos = strzne(param_ptr,' ');
   if (pos == (-1))   /* parameters are all spaces */
   {
      strcpy((DEFCHAR *)param,(DEFCHAR *)param_ptr+1);
      TRACE_RETURN();
      return;
   }
   strcpy((DEFCHAR *)param,(DEFCHAR *)param_ptr+(*(param_ptr) == ' ' ? 1 : 0));
   *(param_ptr) = '\0';
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short param_split(CHARTYPE *params,CHARTYPE *word[],short words,
                CHARTYPE *delims,CHARTYPE param_type,CHARTYPE *strip, bool trailing_spaces_is_arg)
#else
short param_split(params,word,words,delims,param_type,strip,trailing_spaces_is_arg)
CHARTYPE *params;
CHARTYPE *word[];
short words;
CHARTYPE *delims;
CHARTYPE param_type;
CHARTYPE *strip;
bool trailing_spaces_is_arg;
#endif
/***********************************************************************/
{
#define STATE_START    0
#define STATE_WORD     1
#define STATE_DELIM    2
/*--------------------------- local data ------------------------------*/
 register short i=0,k=0,delims_len=strlen((DEFCHAR *)delims);
 unsigned short len=0;
 CHARTYPE j=0;
#if 0
 bool end_of_string=FALSE,end_of_word=FALSE;
#endif
 CHARTYPE *param_ptr=NULL;
 CHARTYPE *space_ptr=NULL;
 CHARTYPE state=STATE_START;
 short str_start=0,str_end=(-1);
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:param_split");
/*---------------------------------------------------------------------*/
/* Allocate some memory to the temporary area.                         */
/*---------------------------------------------------------------------*/
 if (params != NULL)
   {
    if (allocate_temp_space(strlen((DEFCHAR *)params),param_type) != RC_OK)
      {
      TRACE_RETURN();
      return(-1);
      }
   }
/*---------------------------------------------------------------------*/
/* Based on param_type, point param_ptr to appropriate buffer.         */
/*---------------------------------------------------------------------*/
 switch(param_type)
   {
    case TEMP_PARAM:
         param_ptr = temp_params;
         break;
    case TEMP_MACRO:
         param_ptr = temp_macros;
         break;
    case TEMP_TEMP_CMD:
         param_ptr = temp_cmd;
         break;
    case TEMP_TMP_CMD:
         param_ptr = tmp_cmd;
         break;
    default:
         TRACE_RETURN();
         return(-1);
         break;
   }
/*---------------------------------------------------------------------*/
/* In case params is NULL, copy an empty string into param_ptr...      */
/*---------------------------------------------------------------------*/
 if (params == NULL)
    strcpy((DEFCHAR *)param_ptr,"");
 else
#ifdef __BOUNDS_CHECKING_ON
    memmove((DEFCHAR *)param_ptr,(DEFCHAR *)params,strlen(params) + 1);
#else
    strcpy((DEFCHAR *)param_ptr,(DEFCHAR *)params);
#endif

 for (i=0;i<words;i++)
     word[i] = (CHARTYPE *)"";
 word[0] = param_ptr;
 len = strlen((DEFCHAR *)param_ptr);
 if (trailing_spaces_is_arg)
   {
    i = strzrevne(param_ptr,' ');
    if (i != (-1)
    &&  (len - i) > 2)
      {
       space_ptr = param_ptr+(i+2);
       param_ptr[i+1] = '\0';
      }
   }
#if 0
 if (words > 1)
   {
#endif
    j = 0;
    str_start = 0;
    for (i=0;i<len && j<words;i++)
      {
       switch(state)
         {
          case STATE_START:
               for (k=0;k<delims_len;k++)
                 {
                  if (*(param_ptr+i) == *(delims+k))
                    {
                     state = STATE_DELIM;
                     break;
                    }
                 }
               if (state == STATE_DELIM)
                  break;
               word[j++] = param_ptr+str_start;
               if (str_end != (-1))
                 {
                  *(param_ptr+str_end) = '\0';
                 }
               state = STATE_WORD;
               break;
          case STATE_WORD:
               for (k=0;k<delims_len;k++)
                 {
                  if (*(param_ptr+i) == *(delims+k))
                    {
                     state = STATE_DELIM;
                     break;
                    }
                 }
               if (state == STATE_DELIM)
                 {
                  str_end = i;
                  str_start = str_end + 1;
                  break;
                 }
               break;
          case STATE_DELIM:
               state = STATE_WORD;
               for (k=0;k<delims_len;k++)
                 {
                  if (*(param_ptr+i) == *(delims+k))
                    {
                     state = STATE_DELIM;
                     break;
                    }
                 }
               if (state == STATE_WORD)
                 {
                  word[j++] = param_ptr+str_start;
                  if (str_end != (-1))
                    {
                     *(param_ptr+str_end) = '\0';
                    }
                 }
               break;
         }
      }
#if 0
   }
 else
   j = words;
#endif
 for (i=0;i<words;i++)
   {
    if (*(strip+i))
       word[i] = MyStrip(word[i],*(strip+i),' ');
   }
 if (space_ptr)
   {
    word[j] = space_ptr;
    j++;
   }
 TRACE_RETURN();
 return(j);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short command_split(CHARTYPE *params,CHARTYPE *word[],short words,
                CHARTYPE *delims,CHARTYPE *buffer)
#else
short command_split(params,word,words,delims,buffer)
CHARTYPE *params;
CHARTYPE *word[];
short words;
CHARTYPE *delims;
CHARTYPE *buffer;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0,k=0,delims_len=strlen((DEFCHAR *)delims);
 unsigned short len=0;
 CHARTYPE j=0;
 bool end_of_string=FALSE,end_of_word=FALSE;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:command_split");
/*---------------------------------------------------------------------*/
/* In case params is NULL, copy an empty string into buffer...         */
/*---------------------------------------------------------------------*/
 if (params == NULL)
    strcpy((DEFCHAR *)buffer,"");
 else
    strcpy((DEFCHAR *)buffer,(DEFCHAR *)params);

 for (i=0;i<words;i++)
     word[i] = (CHARTYPE *)"";
 j = 0;
 end_of_string = TRUE;
 len = strlen((DEFCHAR *)buffer);
 for (i=0;i<len && j<words;i++)
   {
    end_of_word = FALSE;
    for (k=0;k<delims_len;k++)
      {
       if (*(buffer+i) == *(delims+k))
          end_of_word = TRUE;
      }
    if (end_of_word)
      {
       *(buffer+i) = '\0';
       end_of_string = TRUE;
      }
    else
       if (end_of_string)
         {
          word[j++] = buffer+i;
          end_of_string = FALSE;
         }
   }
 TRACE_RETURN();
 return(j);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LINETYPE get_true_line(bool respect_compat)
#else
LINETYPE get_true_line(respect_compat)
bool respect_compat;
#endif
/***********************************************************************/
{
   LINETYPE true_line=0L;

   TRACE_FUNCTION("commutil.c:get_true_line");
   /*
    * Determine 'true_line'.
    */
   if (CURRENT_VIEW->current_window == WINDOW_COMMAND
   ||  (compatible_feel == COMPAT_XEDIT && respect_compat)
   ||  batch_only)
      true_line = CURRENT_VIEW->current_line;
   else
      true_line = CURRENT_VIEW->focus_line;
   TRACE_RETURN();
   return(true_line);
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE next_char(_LINE *curr,long *off,LENGTHTYPE end_col)
#else
CHARTYPE next_char(curr,off,end_col)
_LINE *curr;
long *off;
LENGTHTYPE end_col;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("commutil.c:next_char");
   if (*(off) < (long)min(curr->length,end_col))
   {
      (*(off))++;
      TRACE_RETURN();
      return(*(curr->line+((*(off))-1L)));
   }
   *(off) = (-1L);
   TRACE_RETURN();
   return(0);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short add_define(DEFINE **first,DEFINE **last,int key_value,CHARTYPE *commands,bool instore,CHARTYPE *synonym,CHARTYPE linend)
#else
short add_define(first,last,key_value,commands,instore,synonym,linend)
DEFINE **first,**last;
int key_value;
CHARTYPE *commands;
bool instore;
CHARTYPE *synonym;
CHARTYPE linend;
#endif
/***********************************************************************/
/* Parameters:                                                         */
/*  key_value: numeric representation of function key                  */
/*   commands: commands and parameters                                 */
/***********************************************************************/
{
   register short j=0;
   short cmd_nr=0;
   CHARTYPE *word[MAX_COMMANDS+1];
   unsigned short num_commands=0;
   CHARTYPE command_delim[2];
   short rc=RC_OK;
   CHARTYPE *command_entered=NULL,*cl_cmd=NULL,*cl_param=NULL;

   TRACE_FUNCTION("commutil.c:add_define");
   /*
    * If the commands argument is empty, delete the definition of the key
    * definitions for the key, so just return.
    */
   if (strcmp((DEFCHAR *)commands,"") == 0)
   {
      remove_define(first,last,key_value,synonym);
      TRACE_RETURN();
      return(RC_OK);
   }
   if (instore)
   {
      if (rexx_support)
      {
         remove_define(first,last,key_value,synonym);
         rc = append_define(first,last,key_value,(-1),commands,NULL,0,synonym,linend);
         TRACE_RETURN();
         return(rc);
      }
      else
      {
         display_error(58,(CHARTYPE *)"instore macros",FALSE);
         TRACE_RETURN();
         return(RC_INVALID_OPERAND);
      }
   }
   /*
    * To have reached here we are dealing with "plain" key definitions,
    * rather than instore macros...
    * Copy the incoming commands, so we can play with it.
    */
   if ((command_entered = (CHARTYPE *)my_strdup(commands)) == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   /*
    * Allocate some space to cl_cmd and cl_param for the a command when
    * it is split into a command and its parameters.
    */
   if ((cl_cmd = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)commands)+1)*sizeof(CHARTYPE))) == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if ((cl_param = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)commands)+1)*sizeof(CHARTYPE))) == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   if ( synonym )
   {
      /*
       * If the is called from a SET SYNONYM call, then use the passed value
       * for LINEND...
       */
      if ( linend )
      {
         command_delim[0] = linend;
         command_delim[1] = '\0';
         num_commands = command_split(commands,word,MAX_COMMANDS,command_delim,command_entered);
      }
      else
      {
         if ( CURRENT_VIEW->linend_status )
         {
            command_delim[0] = CURRENT_VIEW->linend_value;
            command_delim[0] = '\0';
            num_commands = command_split(commands,word,MAX_COMMANDS,command_delim,command_entered);
         }
         else
         {
            word[0] = command_entered;
            num_commands = 1;
         }
      }
   }
   else
   {
      /*
       * If [SET] LINENd is set to ON, split the args up into a number of
       * individual commands.
       */
      if (CURRENT_VIEW)
      {
         if (CURRENT_VIEW->linend_status)
         {
            command_delim[0] = CURRENT_VIEW->linend_value;
            command_delim[1] = '\0';
            num_commands = command_split(commands,word,MAX_COMMANDS,command_delim,command_entered);
         }
         else
         {
            word[0] = command_entered;
            num_commands = 1;
         }
      }
      else
      {
         command_delim[0] = '#';
         command_delim[1] = '\0';
         num_commands = command_split(commands,word,MAX_COMMANDS,command_delim,command_entered);
      }
   }
   /*
    * For each command entered, split it up into command and params, and
    * validate that each command is valid.  The cmd_nr is discarded here.
    */
   for (j=0;j<num_commands;j++)
   {
      split_command(word[j],cl_cmd,cl_param);
      if ((cmd_nr = find_command(cl_cmd,FALSE)) == (-1))
      {
         display_error(21,cl_cmd,FALSE);
         rc = RC_INVALID_OPERAND;
         break;
      }
   }
   /*
    * Now we know each command is valid, we can remove any prior
    * definition and assign the new one.
    */
   if (rc == RC_OK)
   {
      remove_define(first,last,key_value,synonym);
      for (j=0;j<num_commands;j++)
      {
         split_command(word[j],cl_cmd,cl_param);
         if ((cmd_nr = find_command(cl_cmd,FALSE)) == (-1))
         {
            display_error(21,cl_cmd,FALSE);   /* this should not be reached */
            rc = RC_INVALID_OPERAND;
            break;
         }
         rc = append_define(first,last,key_value,cmd_nr,cl_param,NULL,0,synonym,linend);
         if (rc != RC_OK)
            break;
      }
   }
   (*the_free)(command_entered);
   (*the_free)(cl_cmd);
   (*the_free)(cl_param);
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short remove_define(DEFINE **first,DEFINE **last,int key_value,CHARTYPE *synonym)
#else
short remove_define(first,last,key_value,synonym)
DEFINE **first,**last;
int key_value;
CHARTYPE *synonym;
#endif
/***********************************************************************/
/* Parameters:                                                         */
/*  key_value: numeric representation of function key                  */
/***********************************************************************/
{
   DEFINE *curr=NULL;

   TRACE_FUNCTION("commutil.c:remove_define");
   /*
    * Find all items in the linked list for the key_value and remove them
    * from the list.
    */
   curr = *first;
   if ( synonym )
   {
      while(curr != NULL)
      {
         if ( my_stricmp( curr->synonym, synonym ) == 0 )
         {
            if (curr->def_params != NULL)
               (*the_free)(curr->def_params);
            if (curr->pcode != NULL)
               (*the_free)(curr->pcode);
            if (curr->synonym != NULL)
               (*the_free)(curr->synonym);
            curr = dll_del(first,last,curr,DIRECTION_FORWARD);
         }
         else
            curr = curr->next;
      }
   }
   else
   {
      while(curr != NULL)
      {
         if (curr->def_funkey == key_value)
         {
            if (curr->def_params != NULL)
               (*the_free)(curr->def_params);
            if (curr->pcode != NULL)
               (*the_free)(curr->pcode);
            curr = dll_del(first,last,curr,DIRECTION_FORWARD);
         }
         else
            curr = curr->next;
      }
   }

   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short append_define(DEFINE **first,DEFINE **last,int key_value,short cmd,CHARTYPE *prm,CHARTYPE *pcode,int pcode_len,CHARTYPE *synonym,CHARTYPE linend)
#else
short append_define(first,last,key_value,cmd,prm,pcode,pcode_len,synonym,linend)
DEFINE **first,**last;
int key_value;
short cmd;
CHARTYPE *prm;
CHARTYPE *pcode;
int pcode_len;
CHARTYPE *synonym;
CHARTYPE linend;
#endif
/***********************************************************************/
/* Parameters:                                                         */
/*  key_value: numeric representation of function key                  */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   DEFINE *curr=NULL;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:append_define");
/*---------------------------------------------------------------------*/
/* Add the new key definition to the end of the linked list...         */
/*---------------------------------------------------------------------*/
   curr = dll_add(*first,*last,sizeof(DEFINE));
   if (curr == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   curr->def_params = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)prm)+1)*sizeof(CHARTYPE));
   if (curr->def_params == NULL)
   {
      display_error(30,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_OUT_OF_MEMORY);
   }
   strcpy((DEFCHAR *)curr->def_params,(DEFCHAR *)prm);

   if ( synonym )
   {
      curr->synonym = (CHARTYPE *)(*the_malloc)((strlen((DEFCHAR *)synonym)+1)*sizeof(CHARTYPE));
      if (curr->synonym == NULL)
      {
         display_error(30,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
      strcpy((DEFCHAR *)curr->synonym,(DEFCHAR *)synonym);
   }
   else
      curr->synonym = NULL;

   curr->def_funkey = key_value;
   curr->def_command = cmd;
   curr->linend = linend;
   if (pcode && pcode_len)
   {
      curr->pcode = (CHARTYPE *)(*the_malloc)(pcode_len*sizeof(CHARTYPE));
      if (curr->pcode == NULL)
      {
         display_error(30,(CHARTYPE *)"",FALSE);
         TRACE_RETURN();
         return(RC_OUT_OF_MEMORY);
      }
      memcpy(curr->pcode,pcode,pcode_len);
      curr->pcode_len = pcode_len;
   }
   *last = curr;
   if (*first == NULL)
      *first = *last;
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
int find_key_value(CHARTYPE *mnemonic)
#else
int find_key_value(mnemonic)
CHARTYPE *mnemonic;
#endif
/***********************************************************************/
/*   Function: find the matching key value for the supplied key name   */
/* Parameters:                                                         */
/*   mnemonic: the key name to be matched                              */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:find_key_value");
 for (i=0;key_table[i].mnemonic!=NULL;i++)
 {
    if (equal(key_table[i].mnemonic,mnemonic,strlen((DEFCHAR *)key_table[i].mnemonic)))
    {
       TRACE_RETURN();
       return(key_table[i].key_value);
    }
 }
 TRACE_RETURN();
 return(-1);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short find_command(CHARTYPE *cmd,bool search_for_target)
#else
short find_command(cmd,search_for_target)
CHARTYPE *cmd;
bool search_for_target;
#endif
/***********************************************************************/
/*   Function: determine if the string supplied is a valid abbrev for  */
/*             a command.                                              */
/* Parameters:                                                         */
/*        cmd:               the string to be checked                  */
/*        search_for_target: determine if command is a valid target    */
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 short rc=RC_OK;
 TARGET target;
 short target_type=TARGET_NORMAL|TARGET_BLOCK|TARGET_ALL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:find_command");
 for (i=0;command[i].text != NULL;i++)
 {
    if (equal(command[i].text,cmd,(command[i].min_len == 0) ? strlen((DEFCHAR *)command[i].text) : command[i].min_len)
    &&  !command[i].sos_command)
    {
       TRACE_RETURN();
       return(i);
    }
 }
/*---------------------------------------------------------------------*/
/* To get here the command was not a 'command'. If we don't want to    */
/* search for targets, exit with (-1).                                 */
/*---------------------------------------------------------------------*/
 if (!search_for_target)
 {
    TRACE_RETURN();
    return(-1);
 }
/*---------------------------------------------------------------------*/
/* Find if it is a valid target...                                     */
/*---------------------------------------------------------------------*/
 initialise_target(&target);
 rc = validate_target(cmd,&target,target_type,get_true_line(TRUE),TRUE,TRUE);
 if (rc != RC_OK
 &&  rc != RC_TARGET_NOT_FOUND)
 {
    free_target(&target);
    TRACE_RETURN();
    return(-1);
 }
 free_target(&target);
/*---------------------------------------------------------------------*/
/* If a valid target, find 'LOCATE' command and return the index.      */
/*---------------------------------------------------------------------*/
 strcpy((DEFCHAR *)temp_params,(DEFCHAR *)cmd);
 for (i=0;command[i].text != NULL;i++)
 {
    if (strcmp((DEFCHAR *)command[i].text,"locate") == 0)
       break;
 }
 TRACE_RETURN();
  return(i);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void init_command(void)
#else
void init_command()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:init_command");
 strcpy((DEFCHAR *)last_command_for_reexecute,"");
 strcpy((DEFCHAR *)last_command_for_repeat,"");
 for (i=0;i<MAX_SAVED_COMMANDS;i++)
     strcpy((DEFCHAR *)cmd_history[i],"");
 TRACE_RETURN();
 return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
void add_command(CHARTYPE *new_cmd)
#else
void add_command(new_cmd)
CHARTYPE *new_cmd;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:add_command");
   /*
    * Do not save commands if the commands are issued from a macro.
    */
   if (in_macro)
   {
      TRACE_RETURN();
      return;
   }
   /*
    * If the command to be added is the same as the current command or if
    * the command line is empty or if the command is "=" or "?", return
    * without adding command to array.
    */
   if (!valid_command_to_save(new_cmd))
   {
      TRACE_RETURN();
      return;
   }
   offset_cmd = 0;
   if (strcmp((DEFCHAR *)new_cmd,(DEFCHAR *)cmd_history[current_cmd]) == 0)
   {
      TRACE_RETURN();
      return;
   }
   if (number_cmds == MAX_SAVED_COMMANDS)
      current_cmd = last_cmd = (last_cmd == MAX_SAVED_COMMANDS-1) ? 0 : ++last_cmd;
   else
      current_cmd = ++last_cmd;
   strcpy((DEFCHAR *)cmd_history[current_cmd],(DEFCHAR *)new_cmd);
   number_cmds++;
   if (number_cmds > MAX_SAVED_COMMANDS)
      number_cmds = MAX_SAVED_COMMANDS;
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE *get_next_command( short direction, short num)
#else
CHARTYPE *get_next_command(direction,num)
short direction,num;
#endif
/***********************************************************************/
{
   CHARTYPE *ret_cmd=NULL;

   TRACE_FUNCTION("commutil.c:get_next_command");
   if (number_cmds == 0)
   {
      TRACE_RETURN();
      return((CHARTYPE *)NULL);
   }
   while(num--)
   {
      switch(direction)
      {
         case DIRECTION_BACKWARD:
            if (current_cmd+1 == number_cmds)
            {
               current_cmd = 0;
               ret_cmd = cmd_history[current_cmd];
            }
            else
               ret_cmd = cmd_history[++current_cmd];
            break;
         case DIRECTION_FORWARD:
            if (current_cmd+offset_cmd < 0)
            {
               current_cmd = number_cmds-1;
               ret_cmd = cmd_history[current_cmd];
            }
            else
            {
               current_cmd = current_cmd+offset_cmd;
               ret_cmd = cmd_history[current_cmd];
            }
            offset_cmd = (-1);
            break;
         case DIRECTION_NONE:
            ret_cmd = cmd_history[current_cmd];
            break;
      }
   }
   TRACE_RETURN();
   return(ret_cmd);
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool valid_command_to_save(CHARTYPE *save_cmd)
#else
bool valid_command_to_save(save_cmd)
CHARTYPE *save_cmd;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("commutil.c:valid_command_to_save");
   /*
    * If the command to be added is empty or is "=" or starts with "?",
    * return FALSE, otherwise return TRUE.
    */
   if (save_cmd == NULL
   ||  strcmp((DEFCHAR *)save_cmd,"") == 0
   ||  strcmp((DEFCHAR *)save_cmd,"=") == 0
   ||  save_cmd[0] == '?')
   {
      TRACE_RETURN();
      return(FALSE);
   }
   TRACE_RETURN();
   return(TRUE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
static void save_last_command(CHARTYPE *last_cmd,CHARTYPE *cmnd)
#else
static void save_last_command(last_cmd,cmnd)
CHARTYPE *last_cmd,*cmnd;
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("commutil.c:save_last_command");
   /*
    * If the command to be added is the same as the current command or if
    * the command line is empty or if the command is "=" or "?", return
    * without adding command to array.
    * Save commands for repeat but NOT for execute if the command is
    * run from a macro...
    */
   if (valid_command_to_save(last_cmd))
   {
      if (!in_macro)
      {
         if ( strlen( (DEFCHAR *)last_cmd ) < sizeof(last_command_for_reexecute) )
         {
            strcpy((DEFCHAR *)last_command_for_reexecute,(DEFCHAR *)last_cmd);
         }
      }
      if (!equal((CHARTYPE *)"repeat",cmnd,4))
      {
         if ( strlen( (DEFCHAR *)last_cmd ) < sizeof(last_command_for_repeat) )
         {
            strcpy((DEFCHAR *)last_command_for_repeat,(DEFCHAR *)last_cmd);
         }
      }
   }
   TRACE_RETURN();
   return;
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool is_tab_col(LENGTHTYPE x)
#else
bool is_tab_col(x)
LENGTHTYPE x;
#endif
/***********************************************************************/
{
   register short i=0;
   bool rc=FALSE;

   TRACE_FUNCTION("commutil.c:is_tab_col");
   for (i=0;i<CURRENT_VIEW->numtabs;i++)
   {
      if (CURRENT_VIEW->tabs[i] == x)
      {
         rc = TRUE;
         break;
      }
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LENGTHTYPE find_next_tab_col(LENGTHTYPE x)
#else
LENGTHTYPE find_next_tab_col(x)
LENGTHTYPE x;
#endif
/***********************************************************************/
{
   register short i=0;
   LENGTHTYPE next_tab_col=0;

   TRACE_FUNCTION("commutil.c:find_next_tab_col");
   for (i=0;i<CURRENT_VIEW->numtabs;i++)
   {
      if (CURRENT_VIEW->tabs[i] > x)
      {
         next_tab_col = CURRENT_VIEW->tabs[i];
         break;
      }
   }
   TRACE_RETURN();
   return(next_tab_col);
}
/***********************************************************************/
#ifdef HAVE_PROTO
LENGTHTYPE find_prev_tab_col(LENGTHTYPE x)
#else
LENGTHTYPE find_prev_tab_col(x)
LENGTHTYPE x;
#endif
/***********************************************************************/
{
   register short i=0;
   LENGTHTYPE next_tab_col=0;

   TRACE_FUNCTION("commutil.c:find_prev_tab_col");
   for (i=CURRENT_VIEW->numtabs-1;i>-1;i--)
   {
      if (CURRENT_VIEW->tabs[i] < x)
      {
         next_tab_col = CURRENT_VIEW->tabs[i];
         break;
      }
   }
   TRACE_RETURN();
   return(next_tab_col);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short tabs_convert(_LINE *curr,bool expand_tabs,bool use_tabs,
                   bool add_to_recovery)
#else
short tabs_convert(curr,expand_tabs,use_tabs,add_to_recovery)
_LINE *curr;
bool expand_tabs,use_tabs,add_to_recovery;
#endif
/***********************************************************************/
{
#define STATE_NORMAL 0
#define STATE_TAB    1
/*--------------------------- local data ------------------------------*/
 register short i=0,j=0;
 bool expanded=FALSE;
 bool tabs_exhausted=FALSE;
 bool state=FALSE;
 LENGTHTYPE tabcol=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:tabs_convert");
/*---------------------------------------------------------------------*/
/* If we are expanding tabs to spaces, do the following...             */
/*---------------------------------------------------------------------*/
 if (expand_tabs)
   {
    for (i=0,j=0;i<curr->length;i++)
      {
       if (curr->line[i] == '\t')
         {
          if (use_tabs)
            {
             if (tabs_exhausted)
               {
                trec[j++] = ' ';
                if (j >= max_line_length)
                   break;
               }
             else
               {
                tabcol = find_next_tab_col(j+1);
                if (tabcol == 0)
                   tabs_exhausted = TRUE;
                else
                  {
                   tabcol--;
                   do
                     {
                      trec[j++] = ' ';
                      if (j >= max_line_length)
                         break;
                     }
                   while (j<tabcol);
                  }
               }
            }
          else
            {
             do
               {
                trec[j++] = ' ';
                if (j >= max_line_length)
                   break;
               }
             while ((j % TABI_Nx) != 0);
            }
          expanded = TRUE;
         }
       else
         {
          trec[j++] = curr->line[i];
          if (j >= max_line_length)
            break;
         }
      }
/*---------------------------------------------------------------------*/
/* If we expanded tabs, we need to reallocate memory for the line.     */
/*---------------------------------------------------------------------*/
    if (expanded)
      {
       if (add_to_recovery)
          add_to_recovery_list(curr->line,curr->length);
       curr->line = (CHARTYPE *)(*the_realloc)((void *)curr->line,(j+1)*sizeof(CHARTYPE));
       if (curr->line == (CHARTYPE *)NULL)
         {
          display_error(30,(CHARTYPE *)"",FALSE);
          TRACE_RETURN();
          return(RC_OUT_OF_MEMORY);
         }
/*---------------------------------------------------------------------*/
/* Copy the contents of rec into the line.                             */
/*---------------------------------------------------------------------*/
       memcpy(curr->line,trec,j);
       curr->length = j;
       *(curr->line+j) = '\0';
      }
   }
 else
   {
    for (i=(curr->length)-1,j=0;i>(-1);i--)
      {
       switch(state)
         {
          case STATE_NORMAL:
               trec[j++] = *(curr->line+i);
               if (is_tab_col(i+1)
               &&  i != 0)
                 {
                  if (*(curr->line+(i-1)) == ' ')
                    {
                     trec[j++] = '\t';
                     state = STATE_TAB;
                     expanded = TRUE;
                    }
                 }
               break;
          case STATE_TAB:
#if 0
               if (*(curr->line+i) == ' ')
                 {
                  if (is_tab_col(i+1)
                  &&  i != 0)
                     trec[j++] = '\t';
                 }
               else
                 {
                  trec[j++] = *(curr->line+i);
                  state = STATE_NORMAL;
                 }
#else
               if (is_tab_col(i+1)
               &&  i != 0)
                 {
                  if (*(curr->line+i) == ' ')
                    {
                     if (*(curr->line+(i-1)) == ' ')
                        trec[j++] = '\t';
                    }
                  else
                    {
                     trec[j++] = *(curr->line+i);
                     state = STATE_NORMAL;
                    }
                 }
               else
                 {
                  if (*(curr->line+i) != ' ')
                    {
                     trec[j++] = *(curr->line+i);
                     state = STATE_NORMAL;
                    }
                 }
#endif
               break;
         }
      }
    if (expanded)
      {
       trec[j] = '\0';
       curr->length = j;
       for (i=0,j--;j>(-1);i++,j--)
          *(curr->line+i) = trec[j];
       *(curr->line+curr->length) = '\0';
      }
   }
 TRACE_RETURN();
 return((expanded)?RC_FILE_CHANGED:RC_OK);
}

/* -----------------------------------------------------------------
 * This code borrowed heavily from Regina!!
 * Input is a hex string, which is converted to a char string
 * representing the same information and returned.
 *
 * We have to concider the optional grouping of hex digits by spaces at
 * byte boundaries, and the possibility of having to pad first group
 * with a zero.
 *
 * There is one performance problem with this. If the hex string is
 * normalized and long, we have to loop through the string twice, while
 * once would suffice. To determine whether or not to pad first group
 * with a zero, all of first group must be scanned, which is identical
 * to the whole string if it is normalized.
 */
static int pack_hex( char *string, char *out )
{
   char *ptr=NULL ;     /* current digit in input hex string */
   char *end_ptr=NULL ; /* ptr to end+1 in input hex string */
   char *res_ptr=NULL ;       /* ptr to current char in output string */
   int byte_boundary=0 ;      /* boolean, are we at at byte bounary? */
   int count;                 /* used to count positions */
   int last_blank=0;          /* used to report last byte errors */

   res_ptr = out ;
   /*
    * Initiate pointers to current char in intput string, and to
    * end+1 in input string.
    */
   ptr = string ;
   end_ptr = string + strlen( string ) ;
   /*
    * Explicitly check for space at start or end. Illegal space within
    * the hex string is checked for during the loop.
    */
   if ((ptr<end_ptr) && ((isspace(*ptr)) || (isspace(*(end_ptr-1)))))
   {
      TRACE_RETURN();
      return((-1));
   }
   /*
    * Find the number of hex digits in the first group of hex digits.
    * Let the variable 'byte_boundary' be a boolean, indicating if
    * current char might be a byte boundary. I.e if byte_boundary is
    * set, spaces are legal.
    *
    * Also, set the first byte in the output string. That is not
    * necessary if the first group of hex digits has an even number of
    * digits, but it is cheaper to do it always that check for it.
    */
   for (; (ptr<end_ptr) && (isxdigit(*ptr)); ptr++ )
      ;
   byte_boundary = !((ptr-string)%2) ;
   /*
    * Set output to null string
    */
   *res_ptr = 0x00 ;
   /*
    * Loop through the elements of the input string. Skip over spaces.
    * Stuff hex digits into the output string, and report error
    * for any other type of data.
    */
   for (count=1,ptr=string; ptr<end_ptr; ptr++, count++)
   {
      if (isspace(*ptr))
      {
         /*
          * Just make sure that this space occurs at a byte boundary,
          * except from that, ignore it.
          */
         last_blank = count;
         if (!byte_boundary)
         {
            TRACE_RETURN();
            return((-1));
         }
      }
      else if (isxdigit(*ptr))
      {
         /*
          * Stuff it into the output array, either as upper or lower
          * part of a byte, depending on the value of 'byte_boundary'.
          * Then toggle the value of 'byte_boundary'.
          */
         if (byte_boundary)
            *res_ptr = (char)( HEXVAL(*ptr) << 4 ) ;
         else
            {
               /* Damn'ed MSVC: */
               *res_ptr = (char) (*res_ptr + (char) (HEXVAL(*ptr))) ;
               res_ptr++;
            }
         byte_boundary = !byte_boundary ;
      }
      else
      {
         TRACE_RETURN();
         return((-1));
      }
   }
   /*
    * Set the length and do 'redundant' check for problems. In
    * particular, check 'byte_boundary' to verify that the last group
    * of hex digits ended at a byte boundary; report error if not.
    */
   if (!byte_boundary)
   {
      TRACE_RETURN();
      return((-1));
   }

   return res_ptr - out ;
}
/***********************************************************************/
#ifdef HAVE_PROTO
short convert_hex_strings(CHARTYPE *str)
#else
short convert_hex_strings(str)
CHARTYPE *str;
#endif
/***********************************************************************/
{
   register short i=0;
   CHARTYPE *p=NULL;
   bool dec_char=FALSE;
   CHARTYPE temp_str[MAX_COMMAND_LENGTH];
   CHARTYPE *ptr, *end_ptr, *first_non_blank=NULL, *last_non_blank=NULL;
   short num=0;
   int str_len;
#if 0
   CHARTYPE ch1=0,ch2=0;
#endif

   TRACE_FUNCTION("commutil.c:convert_hex_strings");
   /*
    * If the string is less than 4 chars; d'3', then it can't be a hex/dec
    * value.
    */
   str_len = strlen( (DEFCHAR*)str );
   if ( str_len < 4 )
   {
      TRACE_RETURN();
      return( str_len );
   }
   ptr = str;
   end_ptr = str + strlen( str ) - 1;
   /*
    * Determine the first and last non-blank characters...
    */
   for ( ptr = str; ptr < end_ptr; ptr++ )
   {
      if ( *ptr != (CHARTYPE)' ' )
      {
         first_non_blank = ptr;
         break;
      }
   }
   for ( ptr = str; ptr < end_ptr; end_ptr-- )
   {
      if ( *end_ptr != (CHARTYPE)' ' )
      {
         last_non_blank = end_ptr;
         break;
      }
   }
   /*
    * If the number of non-blank characters is less than 4 it can't be
    * a hex/dec value.
    */
   if ( last_non_blank - first_non_blank < 3 )
   {
      TRACE_RETURN();
      return( str_len );
   }
   /*
    * Check if the first non-blank character is d,D,x or X and it has '
    * as its 2nd non-blank character and as its last non-blank character
    * If not, then return with string unchanged.
    */
   if ( *last_non_blank != (CHARTYPE)'\''
   ||   *(first_non_blank+1) != (CHARTYPE)'\'' )
   {
      TRACE_RETURN();
      return( str_len );
   }
   temp_str[0] = toupper( *first_non_blank );
   if ( temp_str[0] == (CHARTYPE)'D' )
      dec_char = TRUE;
   else if ( temp_str[0] == (CHARTYPE)'X' )
      dec_char = FALSE;
   else
   {
      TRACE_RETURN();
      return( str_len );
   }
   /*
    * If we got here we can validate (and change) the contents of the string.
    */
   *(last_non_blank) = (CHARTYPE)'\0';
   if ( dec_char == FALSE )
   {
      i = pack_hex( first_non_blank+2, temp_str );
      if ( i != (-1) )
         memcpy(str,temp_str,i);
   }
   else
   {
      p = (CHARTYPE *)strtok((DEFCHAR *)first_non_blank+2," ");
      while(p != NULL)
      {
         if (equal((CHARTYPE *)"000000",p,1))
            temp_str[i++] = (CHARTYPE)0;
         else
         {
            num = atoi((DEFCHAR *)p);
            if (num < 1 || num > 255)
            {
               TRACE_RETURN();
               return((-1));
            }
            temp_str[i++] = (CHARTYPE)num;
         }
         p = (CHARTYPE *)strtok(NULL," ");
      }
      memcpy(str,temp_str,i);
   }
   TRACE_RETURN();
   return(i);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short marked_block(bool in_current_view)
#else
short marked_block(in_current_view)
bool in_current_view;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:marked_block");
 if (batch_only)                    /* block commands invalid in batch */
   {
    display_error(24,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 if (MARK_VIEW == (VIEW_DETAILS *)NULL)             /* no marked block */
   {
    display_error(44,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 if (MARK_VIEW != CURRENT_VIEW     /* marked block not in current view */
 && in_current_view)
   {
    display_error(45,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_INVALID_ENVIRON);
   }
 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short suspend_curses(void)
#else
short suspend_curses()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:suspend_curses");

#ifdef UNIX
# if defined(USE_EXTCURSES)
 csavetty(FALSE);
 reset_shell_mode();
# else
 endwin();
# endif
#endif

#ifdef OS2
   reset_shell_mode();
#endif

#if WAS_HAVE_BSD_CURSES
 noraw();
 nl();
 echo();
 nocbreak();
#endif

 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short resume_curses(void)
#else
short resume_curses()
#endif
/***********************************************************************/
{
   TRACE_FUNCTION("commutil.c:resume_curses");
#if defined(WIN32) || defined(OS2)
   reset_prog_mode();
#endif
#ifdef UNIX
# if defined(USE_EXTCURSES)
   cresetty(FALSE);
# else
   reset_prog_mode();
#  ifdef HAVE_BSD_CURSES
   raw();
   nonl();
   noecho();
   cbreak();
#  endif
# endif
#endif

   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short restore_THE(void)
#else
short restore_THE()
#endif
/***********************************************************************/
{
   unsigned short y=0,x=0;

   TRACE_FUNCTION("commutil.c:restore_THE");
   /*
    * If curses hasn't started, no point in doing anything...
    */
   if (!curses_started)
   {
      TRACE_RETURN();
      return(RC_OK);
   }
   getyx(CURRENT_WINDOW,y,x);

#if 0
   wclear(stdscr);
   refresh();
#endif

   if (display_screens > 1)
   {
      touch_screen(other_screen);
      refresh_screen(other_screen);
      if (!horizontal)
      {
         touchwin(divider);
         wnoutrefresh(divider);
      }
   }
   touch_screen(current_screen);
   if (statarea != (WINDOW *)NULL)
      touchwin(statarea);
   if ( filetabs != (WINDOW *)NULL )
      touchwin( filetabs );
  #if defined(HAVE_SLK_INIT)
   if (SLKx)
   {
      slk_touch();
      slk_noutrefresh();
   }
  #endif
   wmove(CURRENT_WINDOW,y,x);
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_set_sos_command(bool set_command,CHARTYPE *params)
#else
short execute_set_sos_command(set_command,params)
bool set_command;
CHARTYPE *params;
#endif
/***********************************************************************/
{
#define SETSOS_PARAMS  2
   CHARTYPE *word[SETSOS_PARAMS+1];
   CHARTYPE strip[SETSOS_PARAMS];
   unsigned short num_params=0;
   short rc=RC_OK,command_index=0;

   TRACE_FUNCTION("commutil.c:execute_set_sos_command");
   strip[0]=STRIP_BOTH;
   strip[1]=STRIP_NONE;
   num_params = param_split(params,word,SETSOS_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
   if (num_params < 1)
   {
      display_error(1,(CHARTYPE *)"",FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   if ((command_index = valid_command_type(set_command,word[0])) == RC_NOT_COMMAND)
   {
      display_error(set_command ? 42 : 41,word[0],FALSE);
      TRACE_RETURN();
      return(RC_INVALID_OPERAND);
   }
   /*
    * If the SOS command is being executed while in READV CMDLINE, only
    * execute those commands that are allowed...
    */
   if (in_readv)
   {
      if (command[command_index].valid_readv)
         rc = (*command[command_index].function)(word[1]);
   }
   else
   {
      /*
       * If operating in CUA mode, and a CUA block exists, check
       * if the block should be reset or deleted before executing
       * the command.
       */
      if ( INTERFACEx == INTERFACE_CUA
      &&  MARK_VIEW == CURRENT_VIEW
      &&  MARK_VIEW->mark_type == M_CUA )
      {
         ResetOrDeleteCUABlock( command[command_index].cua_behaviour );
      }
      /*
       * Possibly reset the THIGHLIGHT area
       */
      if ( CURRENT_VIEW->thighlight_on
      &&   CURRENT_VIEW->thighlight_active )
      {
         AdjustThighlight( command[command_index].thighlight_behaviour );
      }
      rc = (*command[command_index].function)(word[1]);
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short valid_command_type(bool set_command,CHARTYPE *cmd_line)
#else
short valid_command_type(set_command,cmd_line)
bool set_command;
CHARTYPE *cmd_line;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   register short i;
   short rc=RC_NOT_COMMAND;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:valid_command_type");
   for (i=0;command[i].text != NULL;i++)
   {
      /*
       * If no command text, continue.
       */
      if (strcmp((DEFCHAR *)command[i].text,"") == 0)
         continue;
      /*
       * Check that the supplied command matches the command for the length
       * of the command and that the length is at least as long as the
       * necessary significance.
       */
      if (equal(command[i].text,cmd_line,command[i].min_len)
      && command[i].min_len != 0)
      {
         TRACE_RETURN();
         if (set_command && command[i].set_command)
         {
            rc = i;
            break;
         }
         if (!set_command && command[i].sos_command)
         {
            rc = i;
            break;
         }
      }
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short allocate_temp_space(unsigned short length,CHARTYPE param_type)
#else
short allocate_temp_space(length,param_type)
unsigned short length;
CHARTYPE param_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE *temp_ptr=NULL;
 unsigned short *temp_length=NULL;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:allocate_temp_space");
/*---------------------------------------------------------------------*/
/* Based on param_type, point param_ptr to appropriate buffer.         */
/*---------------------------------------------------------------------*/
 switch(param_type)
   {
    case TEMP_PARAM:
         temp_ptr = temp_params;
         temp_length = &length_temp_params;
         break;
    case TEMP_MACRO:
         temp_ptr = temp_macros;
         temp_length = &length_temp_macros;
         break;
    case TEMP_TMP_CMD:
         temp_ptr = tmp_cmd;
         temp_length = &length_tmp_cmd;
         break;
    case TEMP_TEMP_CMD:
         temp_ptr = temp_cmd;
         temp_length = &length_temp_cmd;
         break;
    default:
         TRACE_RETURN();
         return(-1);
         break;
   }
 if (*temp_length >= length)
   {
    TRACE_RETURN();
    return(RC_OK);
   }
 if (temp_ptr == NULL)
    temp_ptr = (CHARTYPE *)(*the_malloc)(sizeof(CHARTYPE)*(length+1));
 else
    temp_ptr = (CHARTYPE *)(*the_realloc)(temp_ptr,sizeof(CHARTYPE)*(length+1));
 if (temp_ptr == NULL)
   {
    display_error(30,(CHARTYPE *)"",FALSE);
    TRACE_RETURN();
    return(RC_OUT_OF_MEMORY);
   }
/*---------------------------------------------------------------------*/
/* Based on param_type, point param_ptr to appropriate buffer.         */
/*---------------------------------------------------------------------*/
 switch(param_type)
   {
    case TEMP_PARAM:
         temp_params = temp_ptr;
         break;
    case TEMP_MACRO:
         temp_macros = temp_ptr;
         break;
    case TEMP_TMP_CMD:
         tmp_cmd = temp_ptr;
         break;
    case TEMP_TEMP_CMD:
         temp_cmd = temp_ptr;
         break;
    default:
         TRACE_RETURN();
         return(-1);
         break;
   }
 *temp_length = length;
 TRACE_RETURN();
 return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
void free_temp_space(CHARTYPE param_type)
#else
void free_temp_space(param_type)
CHARTYPE param_type;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 CHARTYPE *temp_ptr=NULL;
 unsigned short *temp_length=0;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:free_temp_space");
/*---------------------------------------------------------------------*/
/* Based on param_type, point param_ptr to appropriate buffer.         */
/*---------------------------------------------------------------------*/
 switch(param_type)
   {
    case TEMP_PARAM:
         temp_ptr    = temp_params;
         temp_params = NULL;
         temp_length = &length_temp_params;
         break;
    case TEMP_MACRO:
         temp_ptr    = temp_macros;
         temp_macros = NULL;
         temp_length = &length_temp_macros;
         break;
    case TEMP_TMP_CMD:
         temp_ptr = tmp_cmd;
         tmp_cmd  = NULL;
         temp_length = &length_tmp_cmd;
         break;
    case TEMP_TEMP_CMD:
         temp_ptr    = temp_cmd;
         temp_cmd    = NULL;
         temp_length = &length_temp_cmd;
         break;
    default:
         TRACE_RETURN();
         return;
         break;
   }
 (*the_free)(temp_ptr);
 *temp_length = 0;
 TRACE_RETURN();
 return;
}
#ifdef MSWIN
/***********************************************************************/
#ifdef HAVE_PROTO
void init_temp_space(void)
#else
void init_temp_space()
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:init_temp_space");
 temp_params = temp_macros = tmp_cmd = temp_cmd = NULL;
 TRACE_RETURN();
 return;
}
#endif
/***********************************************************************/
#ifdef HAVE_PROTO
CHARTYPE calculate_actual_row(CHARTYPE base,short off,CHARTYPE rows,bool force_in_view)
#else
CHARTYPE calculate_actual_row(base,off,rows,force_in_view)
CHARTYPE base,rows;
short off;
bool force_in_view;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   short row=0;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:calculate_actual_row");
   switch(base)
   {
      case POSITION_TOP:
         row = off;
         break;
      case POSITION_MIDDLE:
         row = (rows /2 ) + off;
         break;
      case POSITION_BOTTOM:
         row = rows+off+1;
         break;
   }
   /*
    * If the calculated row is outside the screen size, default to middle.
    */
   if ((row < 0 || row > rows)
   && force_in_view)
      row = rows / 2;
   TRACE_RETURN();
   return((CHARTYPE)row-1);
}
/*man***************************************************************************
NAME
     get_valid_macro_file_name

SYNOPSIS
     short get_valid_macro_file_name(macroname,filename,errnum)
     CHARTYPE *macroname;
     CHARTYPE *filename;
     CHARTYPE *macro_ext;
     short *errnum;

DESCRIPTION
     The get_valid_macro_file_name function determines the fully qualified
     file name for the supplied macroname.

     This routine is also used to find THE Language Definition files.

     If the macroname contains any path specifiers, then the macro name
     is used as the filename and a check is made to ensure that the file
     exists and is readable.

     If the macroname does not contain any path specifiers, each
     directory in the MACROPATH variable is searched for a file that
     consists of the macroname appended with the current value for
     macro_ext. If a file is found, it is checked to ensure it is
     readable.

RETURN VALUE
     If a file is found based on the above matching process, the fully
     qualified file name is copied into filename, errnum is set to 0
     and the function returns with RC_OK.

     If a file is not found, the macroname is copied into filename, the
     error number of the error message is copied into errnum and the
     function returns with RC_FILE_NOT_FOUND.

     If a file is found but the file is not readable, the macroname is
     copied into filename, the error number of the error message is
     copied into errnum and the function returns with RC_ACCESS_DENIED.
*******************************************************************************/
#ifdef HAVE_PROTO
short get_valid_macro_file_name(CHARTYPE *inmacroname,CHARTYPE *filename,CHARTYPE *macro_ext,short *errnum)
#else
short get_valid_macro_file_name(inmacroname,filename,macro_ext,errnum)
CHARTYPE *inmacroname,*filename,*macro_ext;
short *errnum;
#endif
/***********************************************************************/
{
   register short i=0;
   CHARTYPE delims[3];
   bool file_found=FALSE;
   CHARTYPE _THE_FAR macroname[MAX_FILE_NAME+1] ;
   int len_macroname=strlen((DEFCHAR*)inmacroname);
   int len_macro_suffix=strlen((DEFCHAR*)macro_ext);
   bool append_suffix=TRUE;

   TRACE_FUNCTION("commutil.c:get_valid_macro_file_name");
   /*
    * Create the full name of the macro file by prepending the default
    * macropath provided the filename does not already contain a path.
    */
   strcpy( (DEFCHAR*)macroname, (DEFCHAR*)inmacroname );
   (void *)strrmdup(strtrans(macroname,OSLASH,ISLASH),ISLASH,TRUE);
#ifdef UNIX
   strcpy((DEFCHAR *)delims,(DEFCHAR *)ISTR_SLASH);
   if (strpbrk((DEFCHAR *)macroname,(DEFCHAR *)delims) == NULL
   && *(macroname) != '~')
#endif
#if defined(DOS) || defined(OS2) || defined(WIN32) || defined(AMIGA)
   strcpy((DEFCHAR *)delims,ISTR_SLASH);
   strcat((DEFCHAR *)delims,":");
   if (strpbrk((DEFCHAR *)macroname,(DEFCHAR *)delims) == NULL)
#endif
   {
      /*
       * The supplied macro file name does not contain a path...so for each
       * directory in the_macro_path, try to find the supplied file in that
       * directory.
       */
      if (len_macroname > len_macro_suffix)
      {
         if (strcmp((DEFCHAR*)macroname+(len_macroname-len_macro_suffix),(DEFCHAR*)macro_ext) == 0)
            append_suffix = FALSE;
         else
            append_suffix = TRUE;
      }
      file_found = FALSE;
      for (i=0;i<max_macro_dirs;i++)
      {
         strcpy((DEFCHAR *)filename,(DEFCHAR *)the_macro_dir[i]);
         if (strlen((DEFCHAR *)filename) == 0)
            continue;
         if (*(filename+strlen((DEFCHAR *)filename)-1) != ISLASH)
            strcat((DEFCHAR *)filename,(DEFCHAR *)ISTR_SLASH);
         strcat((DEFCHAR *)filename,(DEFCHAR *)macroname);     /* append the file name */
         if (append_suffix)
            strcat((DEFCHAR *)filename,(DEFCHAR *)macro_ext); /* append default suffix */
         if (file_exists(filename))           /* check if file exists... */
         {
            file_found = TRUE;
            break;
         }
      }
      if (!file_found)
      {
         strcpy((DEFCHAR *)filename,(DEFCHAR *)macroname);
         if (append_suffix)
            strcat((DEFCHAR *)filename,(DEFCHAR *)macro_ext);
         *errnum = 11;
         TRACE_RETURN();
         return(RC_FILE_NOT_FOUND);
      }
   }
   else                                /* file contains a path specifier */
   {
      /*
       * The supplied macro file name does contain a path...so just check to
       * ensure that the file exists.
       */
      if (splitpath(macroname) != RC_OK)
      {
         *errnum = 9;
         TRACE_RETURN();
         return(RC_FILE_NOT_FOUND);
      }
      strcpy((DEFCHAR *)filename,(DEFCHAR *)sp_path);
      strcat((DEFCHAR *)filename,(DEFCHAR *)sp_fname);
      if (!file_exists(filename)
      ||  strcmp((DEFCHAR *)sp_fname,"") == 0)
      {
         *errnum = 9;
         TRACE_RETURN();
         return(RC_FILE_NOT_FOUND);
      }
   }
   /*
    * If the file is not readable, error.
    */
   if (!file_readable(filename))
   {
      *errnum = 8;
      TRACE_RETURN();
      return(RC_ACCESS_DENIED);
   }
   *errnum = 0;
   TRACE_RETURN();
   return(RC_OK);
}
/***********************************************************************/
#ifdef HAVE_PROTO
bool define_command(CHARTYPE *cmd_line)
#else
bool define_command(cmd_line)
CHARTYPE *cmd_line;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register short i=0;
 CHARTYPE buf[7];
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:define_command");
/*---------------------------------------------------------------------*/
/* First check if the command is a synonym, and use the real name to   */
/* search the command array.                                           */
/*---------------------------------------------------------------------*/

 memset(buf,'\0',7);
 memcpy(buf,cmd_line,min(6,strlen((DEFCHAR *)cmd_line)));
 for (i=0;i<7;i++)
   {
    if (buf[i] == ' ')
       buf[i] = '\0';
   }
 if ((i = find_command(buf,FALSE)) == (-1))
   {
    TRACE_RETURN();
    return(FALSE);
   }
 if (strcmp("define",(DEFCHAR *)command[i].text) == 0)
   {
    TRACE_RETURN();
    return(TRUE);
   }
 TRACE_RETURN();
 return(FALSE);
}
/***********************************************************************/
#ifdef HAVE_PROTO
int find_key_name(CHARTYPE *keyname)
#else
int find_key_name(keyname)
CHARTYPE *keyname;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
 register int i=0;
 int key=(-1);
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:find_key_name");
 for (i=0;key_table[i].mnemonic != NULL;i++)
   {
    if (memcmpi(keyname,key_table[i].mnemonic,strlen((DEFCHAR *)keyname)) == 0)
      {
       key = key_table[i].key_value;
       break;
      }
   }
 TRACE_RETURN();
 return(key);
}
/***********************************************************************/
#ifdef HAVE_PROTO
int readv_cmdline(CHARTYPE *initial, WINDOW *dw, int start_col)
#else
int readv_cmdline(initial,dw,start_col)
CHARTYPE *initial;
WINDOW *dw;
int start_col;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int key=0;
   short rc=RC_OK;
   CHARTYPE buf[3];
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:readv_cmdline");
   if ( CURRENT_WINDOW_COMMAND == (WINDOW *)NULL )
   {
      display_error( 86, (CHARTYPE *)"", FALSE );
      TRACE_RETURN();
      return( RC_INVALID_OPERAND );
   }
   buf[1] = '\0';
   Cmsg(initial);
   if ( start_col == -1 )
      THEcursor_cmdline( strlen( (DEFCHAR *)initial ) + 1 );
   else
      THEcursor_cmdline( start_col + 1 );
   in_readv = TRUE; /* this MUST go after THEcursor_cmdline() to work */
   /*
    * If we were called from execute_dialog, refresh the dialog window
    */
   if ( dw )
      wnoutrefresh( dw );
   wrefresh( CURRENT_WINDOW_COMMAND );
   while( 1 )
   {
      key = my_getch( stdscr );
#if defined(XCURSES)
      if ( key == KEY_SF || key == KEY_SR )
         continue;
#endif
#if defined(PDCURSES_MOUSE_ENABLED) || defined(NCURSES_MOUSE_VERSION)
      if (key == KEY_MOUSE)
      {
         int b,ba,bm,y,x;
         if (get_mouse_info(&b,&ba,&bm) != RC_OK)
            continue;
         if (b != 1
         ||  ba == BUTTON_PRESSED)
            continue;
         wmouse_position(CURRENT_WINDOW_COMMAND, &y, &x);
         if (y == -1
         &&  x == -1)
         {
            /*
             * Button 1 clicked or released outside of window.
             */
            if ( dw != NULL )
            {
               rc = RC_READV_TERM_MOUSE;
               break;
            }
            continue;

         }
         /*
          * Got a valid button. Check if its a click or press
          */
         if (ba == BUTTON_CLICKED
         ||  ba == BUTTON_RELEASED)
         {
            /*
             * Got a mouse event
             */
            wmove(CURRENT_WINDOW_COMMAND, 0, x );
         }
         else
            continue;
      }
      else
#endif
      {
         rc = function_key( key, OPTION_READV, FALSE );
         switch( rc )
         {
            case RC_READV_TERM:
               break;
            case RAW_KEY:
               if (rc >= RAW_KEY)
               {
                  if ( rc > RAW_KEY )
                     key = rc - ( RAW_KEY*2 );
                  if ( key < 256 && key >= 0 )
                  {
                     buf[0] = (CHARTYPE)key;
                     rc = Text( buf );
                  }
               }
               break;
            default:
               break;
         }
      }
      show_statarea();
      /*
       * If we were called from execute_dialog, refresh the dialog window
       */
      if ( dw )
         wnoutrefresh( dw );
      wrefresh( CURRENT_WINDOW_COMMAND );
      if ( rc == RC_READV_TERM
      ||   rc == RC_READV_TERM_MOUSE )
         break;
   }
   /*
    * If we were NOT on the command line, go back to where we were.
    */
   in_readv = FALSE; /* this MUST go here to allow THEcursor_home() to work */
   TRACE_RETURN();
   return( rc );
}
/***********************************************************************/
#ifdef HAVE_PROTO
short execute_mouse_commands(int key)
#else
short execute_mouse_commands(key)
int key;
#endif
/***********************************************************************/
{
   DEFINE *curr=(DEFINE *)NULL;
   CHARTYPE *key_cmd=NULL;
   short rc=RC_OK;
   short macrorc=0;

   TRACE_FUNCTION("commutil.c:execute_mouse_commands");
   curr = first_mouse_define;
   while(curr != (DEFINE *)NULL)
   {
      if (key == curr->def_funkey)
      {
         /*
          * If running in read-only mode and the function selected is not valid
          * display an error.
          */
         if (curr->def_command != (-1)
         && ISREADONLY(CURRENT_FILE)
         && !command[curr->def_command].valid_in_readonly)
         {
            display_error(56,(CHARTYPE *)"",FALSE);
            rc = RC_INVALID_ENVIRON;
            curr = NULL;
            break;
         }
         /*
          * If there are no more files in the ring, and the command is not a
          * command to edit a new file, then ignore the command.
          */
         if (curr->def_command != (-1)
         &&  number_of_files == 0
         &&  !command[curr->def_command].edit_command)
         {
            rc = RC_OK;
            curr = NULL;
            break;
         }
         if ((key_cmd = (CHARTYPE *)my_strdup(curr->def_params)) == NULL)
         {
            display_error(30,(CHARTYPE *)"",FALSE);
            rc = RC_OUT_OF_MEMORY;
            curr = NULL;
            break;
         }
         if (curr->def_command == (-1))
            rc = execute_macro_instore(key_cmd,&macrorc,&curr->pcode,&curr->pcode_len,NULL,curr->def_funkey);
         else
         {
            /*
             * If operating in CUA mode, and a CUA block exists, check
             * if the block should be reset or deleted before executing
             * the command.
             */
            if ( INTERFACEx == INTERFACE_CUA
            &&  MARK_VIEW == CURRENT_VIEW
            &&  MARK_VIEW->mark_type == M_CUA )
            {
               ResetOrDeleteCUABlock( command[curr->def_command].cua_behaviour );
            }
            /*
             * Possibly reset the THIGHLIGHT area
             */
            if ( CURRENT_VIEW->thighlight_on
            &&   CURRENT_VIEW->thighlight_active )
            {
               AdjustThighlight( command[curr->def_command].thighlight_behaviour );
            }
            rc = (*command[curr->def_command].function)((CHARTYPE *)key_cmd);
         }
         (*the_free)(key_cmd);
         if (rc != RC_OK
         &&  rc != RC_TOF_EOF_REACHED
         &&  rc != RC_NO_LINES_CHANGED
         &&  rc != RC_TARGET_NOT_FOUND)
         {
            curr = NULL;
            break;
         }
      }
      if (curr == NULL)
         break;
      curr = curr->next;
   }
   TRACE_RETURN();
   return(rc);
}
/***********************************************************************/
#ifdef HAVE_PROTO
short validate_n_m(CHARTYPE *params,short *col1,short *col2)
#else
short validate_n_m(params,col1,col2)
CHARTYPE *params;
short *col1,*col2;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
#define NM_PARAMS  2
 CHARTYPE *word[NM_PARAMS+1];
 CHARTYPE strip[NM_PARAMS];
 unsigned short num_params=0;
 short rc=RC_OK;
/*--------------------------- processing ------------------------------*/
 TRACE_FUNCTION("commutil.c:validate_n_m");
/*---------------------------------------------------------------------*/
/* Validate the parameters that have been supplied. One only           */
/* parameter MUST be supplied. The first parameter MUST be a positive  */
/* integer. The second can be a positive integer or '*'. If no second  */
/* parameter is supplied, defaults to p1. The second parameter MUST be */
/* >= first parameter. '*' is regarded as the biggest number and is    */
/* literally 255.                                                      */
/*---------------------------------------------------------------------*/
 strip[0]=STRIP_BOTH;
 strip[1]=STRIP_BOTH;
 num_params = param_split(params,word,NM_PARAMS,WORD_DELIMS,TEMP_PARAM,strip,FALSE);
 if (num_params < 1)
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
 if (!valid_positive_integer(word[0]))
   {
    display_error(4,word[0],FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 *col1 = atoi((DEFCHAR *)word[0]);
 if (strcmp((DEFCHAR *)word[1],"*") == 0)
    *col2 = 255;
 else
    if (num_params == 1)      /* no second parameter, default to first */
       *col2 = *col1;
    else
       if (!valid_positive_integer(word[1]))
         {
          display_error(4,word[1],FALSE);
          TRACE_RETURN();
          return(RC_INVALID_OPERAND);
         }
       else
          *col2 = atoi((DEFCHAR *)word[1]);

 if (*col2 > 255)
    *col2 = 255;
 if (*col1 > *col2)
   {
    display_error(6,word[0],FALSE);
    TRACE_RETURN();
    return(RC_INVALID_OPERAND);
   }
 TRACE_RETURN();
 return(rc);
}

/***********************************************************************/
#ifdef HAVE_PROTO
void ResetOrDeleteCUABlock( int cua_behaviour )
#else
void ResetOrDeleteCUABlock( cua_behaviour )
int cua_behaviour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   LENGTHTYPE save_col;
   LINETYPE save_line;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:ResetOrDeleteCUABlock");

   if ( cua_behaviour & CUA_RESET_BLOCK )
   {
      Reset( (CHARTYPE *)"BLOCK" );
   }
   else if ( cua_behaviour & CUA_DELETE_BLOCK )
   {
      /*
       * Determine start of block, and save this position so
       * we can move the cursor there after deleting the
       * marked block.
       */
      if ( (CURRENT_VIEW->mark_start_line * max_line_length ) + CURRENT_VIEW->mark_start_col < (CURRENT_VIEW->mark_end_line * max_line_length ) + CURRENT_VIEW->mark_end_col )
      {
         save_line = CURRENT_VIEW->mark_start_line;
         save_col = CURRENT_VIEW->mark_start_col;
      }
      else
      {
         save_line = CURRENT_VIEW->mark_end_line;
         save_col = CURRENT_VIEW->mark_end_col;
      }
      box_operations(BOX_D,SOURCE_BLOCK_RESET,FALSE,' ');
      THEcursor_goto( save_line, save_col );
   }
   TRACE_RETURN();
   return;
}

/***********************************************************************/
#ifdef HAVE_PROTO
void AdjustThighlight( int thighlight_behaviour )
#else
void AdjustThighlight( thighlight_behaviour )
int cua_behaviour;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:AdjustThighlight");
   if ( thighlight_behaviour ==  THIGHLIGHT_RESET_ALL )
   {
      CURRENT_VIEW->thighlight_active = FALSE;
   }
   else if ( thighlight_behaviour == THIGHLIGHT_RESET_FOCUS )
   {
      if ( CURRENT_VIEW->focus_line == CURRENT_VIEW->thighlight_target.true_line )
      {
         CURRENT_VIEW->thighlight_active = FALSE;
      }
   }
   TRACE_RETURN();
   return;
}

/***********************************************************************/
#ifdef HAVE_PROTO
bool save_target( TARGET *target )
#else
bool save_target( target )
TARGET *target;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   int i;
   bool string_target=FALSE;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:save_target");

   if ( TARGETSAVEx == TARGET_ALL )
   {
      TRACE_RETURN();
      return TRUE;
   }

   if ( TARGETSAVEx == TARGET_UNFOUND )
   {
      TRACE_RETURN();
      return FALSE;
   }

   for ( i = 0; i < target->num_targets; i++ )
   {
      if ( TARGETSAVEx & target->rt[i].target_type )
      {
         string_target = TRUE;
         break;
      }
   }
   TRACE_RETURN();
   return string_target;
}

/***********************************************************************/
#ifdef HAVE_PROTO
short execute_locate( CHARTYPE *cmd, bool display_parse_error )
#else
short execute_locate( cmd, display_parse_error )
CHARTYPE *cmd;
bool display_parse_error;
#endif
/***********************************************************************/
{
/*--------------------------- local data ------------------------------*/
   LINETYPE save_focus_line=0L;
   LINETYPE save_current_line=0L;
   TARGET target;
   short target_type=TARGET_NORMAL|TARGET_SPARE|TARGET_REGEXP;
   bool negative=FALSE;
   short rc;
   bool wrapped=FALSE;
   LINETYPE true_line=0L;
/*--------------------------- processing ------------------------------*/
   TRACE_FUNCTION("commutil.c:execute_locate");

   save_focus_line=CURRENT_VIEW->focus_line;
   save_current_line=CURRENT_VIEW->current_line;
   CURRENT_VIEW->thighlight_active = FALSE;
   initialise_target( &target );
   rc = validate_target( cmd, &target, target_type, get_true_line(TRUE), display_parse_error, (CURRENT_VIEW->wrap)?FALSE:TRUE );
   /*
    * If a valid target, but target not found, continue...
    */
   if (rc == RC_TARGET_NOT_FOUND)
   {
      if (CURRENT_VIEW->wrap)
      {
         wrapped = TRUE;
         negative = target.rt[0].negative;
         free_target( &target );
         initialise_target( &target );
         post_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (_LINE *)NULL, TRUE );
         true_line = (negative ? CURRENT_FILE->number_lines+1 : 0L);
         CURRENT_VIEW->focus_line = CURRENT_VIEW->current_line = true_line;
         pre_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (_LINE *)NULL );
         rc = validate_target( cmd, &target, target_type, true_line, display_parse_error, TRUE );
         if (rc != RC_OK)
         {
            CURRENT_VIEW->focus_line = save_focus_line;
            CURRENT_VIEW->current_line = save_current_line;
            pre_process_line( CURRENT_VIEW, CURRENT_VIEW->focus_line, (_LINE *)NULL );
         }
      }
      if (rc == RC_TARGET_NOT_FOUND)
      {
         if ( !in_macro
         &&   save_target( &target ) )
            strcpy( (DEFCHAR *)lastop[LASTOP_LOCATE].value, (DEFCHAR *)target.string );
         /*
          * Don't free the target if THIGHLIGHT is on
          */
         if ( !CURRENT_VIEW->thighlight_on )
            free_target( &target );
         rc = RC_TARGET_NOT_FOUND;
         TRACE_RETURN();
         return rc;
      }
   }
   /*
    * If a valid target and found, go there and execute any following
    * command.
    */
   if (rc == RC_OK)
   {
      if (wrapped)
      {
         display_error(0,(CHARTYPE*)"Wrapped...",FALSE);
         CURRENT_VIEW->focus_line = save_focus_line;
         CURRENT_VIEW->current_line = save_current_line;
         build_screen(current_screen);
         if (CURRENT_VIEW->current_window == WINDOW_COMMAND
         ||  compatible_feel == COMPAT_XEDIT)
            CURRENT_VIEW->current_line = true_line;
         else
            CURRENT_VIEW->focus_line = true_line;
         pre_process_line(CURRENT_VIEW,CURRENT_VIEW->focus_line,(_LINE *)NULL);
      }
      if ( !in_macro
      &&   save_target( &target ) )
         strcpy((DEFCHAR *)lastop[LASTOP_LOCATE].value,(DEFCHAR *)target.string);
      rc = advance_current_or_focus_line(target.num_lines);

      /*
       * Set the THIGHLIGHT location if supported and the target was a string...
       */
      if ( CURRENT_VIEW->thighlight_on )
      {
         free_target( &CURRENT_VIEW->thighlight_target );
         CURRENT_VIEW->thighlight_target = target;
         CURRENT_VIEW->thighlight_target.true_line = get_true_line( TRUE );
         CURRENT_VIEW->thighlight_active = TRUE;
         display_screen(current_screen);
      }
      if ((rc == RC_OK
        || rc == RC_TOF_EOF_REACHED)
      &&  target.spare != (-1))
         rc = command_line(MyStrip(target.rt[target.spare].string,STRIP_LEADING,' '),FALSE);
      /*
       * Don't free the target if THIGHLIGHT is on
       */
      if ( !CURRENT_VIEW->thighlight_on )
         free_target(&target);
      TRACE_RETURN();
      return rc;
   }
   free_target( &target );

   TRACE_RETURN();
   return rc;
}
